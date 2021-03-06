/*
=======================================================================================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

#include "g_local.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
#include "../botlib/be_ea.h"
#include "../botlib/be_ai_char.h"
#include "../botlib/be_ai_chat.h"
#include "../botlib/be_ai_gen.h"
#include "../botlib/be_ai_goal.h"
#include "../botlib/be_ai_move.h"
#include "../botlib/be_ai_weap.h"
#include "../botlib/be_ai_weight.h"
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
#include "ai_vcmd.h"
#include "chars.h" // characteristics
#include "inv.h" // indexes into the inventory
#include "syn.h" // synonyms
#include "match.h" // string matching types and vars
#include "../../ui/menudef.h" // for the voice chats
// ctf task preferences for a client
typedef struct bot_ctftaskpreference_s {
	char name[36];
	int preference;
} bot_ctftaskpreference_t;

bot_ctftaskpreference_t ctftaskpreferences[MAX_CLIENTS];

/*
=======================================================================================================================================
BotValidTeamLeader
=======================================================================================================================================
*/
int BotValidTeamLeader(bot_state_t *bs) {

	if (!strlen(bs->teamleader)) {
		return qfalse;
	}

	if (ClientFromName(bs->teamleader) == -1) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotNumTeamMates
=======================================================================================================================================
*/
int BotNumTeamMates(bot_state_t *bs) {
	int i, numteammates;
	char buf[MAX_INFO_STRING];

	numteammates = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			numteammates++;
		}
	}

	return numteammates;
}

/*
=======================================================================================================================================
BotGetNextPlayer

Returns -1 if there are no more players. Does not return the bot itself (if bs != NULL). Use lastPlayer = -1 for first call.
=======================================================================================================================================
*/
int BotGetNextPlayer(bot_state_t *bs, int lastPlayer, playerState_t *ps) {
	int currentClient;

	for (currentClient = lastPlayer + 1; currentClient < level.maxclients; currentClient++) {
		if (bs && bs->entitynum == currentClient) {
			continue;
		}

		if (!g_entities[currentClient].inuse) {
			continue;
		}

		if (!g_entities[currentClient].client) {
			continue;
		}

		if (g_entities[currentClient].client->pers.connected != CON_CONNECTED) {
			continue;
		}

		if (g_entities[currentClient].client->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}

		if (!BotAI_GetClientState(currentClient, ps)) {
			continue;
		}

		return currentClient;
	}

	return -1;
}

/*
=======================================================================================================================================
BotGetNextPlayerOrMonster

Returns -1 if there are no more players. Does not return the bot itself (if bs != NULL). Use lastPlayer = -1 for first call.
=======================================================================================================================================
*/
/*
int BotGetNextPlayerOrMonster(bot_state_t *bs, int lastPlayer, playerState_t *ps) {
	int currentClient;

	if (lastPlayer < MAX_CLIENTS) {
		lastPlayer = BotGetNextPlayer(bs, lastPlayer, ps);

		if (lastPlayer >= 0) {
			return lastPlayer;
		}

		lastPlayer = MAX_CLIENTS - 1;
	}

	for (currentClient = lastPlayer + 1; currentClient < level.num_entities; currentClient++) {
		if (bs && bs->entitynum == currentClient) {
			continue;
		}

		if (!BotAI_GetClientState(currentClient, ps)) {
			continue;
		}

		return currentClient;
	}

	return -1;
}
*/
/*
=======================================================================================================================================
BotGetNextTeamMate

Returns -1 if there are no more team mates. Does not return the bot itself. Use lastTeamMate = -1 for first call.
=======================================================================================================================================
*/
int BotGetNextTeamMate(bot_state_t *bs, int lastTeamMate, playerState_t *ps) {
	int player;

	if (gametype < GT_TEAM) {
		return -1;
	}

	for (player = lastTeamMate; (player = BotGetNextPlayer(bs, player, ps)) >= 0;) {
		if (bs->cur_ps.persistant[PERS_TEAM] == ps->persistant[PERS_TEAM]) {
			return player;
		}
	}

	return -1;
}

/*
=======================================================================================================================================
BotDetermineVisibleTeammates
=======================================================================================================================================
*/
void BotDetermineVisibleTeammates(bot_state_t *bs) {
	int teammate;
	playerState_t ps;

	if (gametype < GT_TEAM) {
		bs->numvisteammates = 0;
		return;
	}

	if (FloatTime() < bs->visteammates_time) {
		return;
	}

	bs->visteammates_time = FloatTime() + 1 + random();
	bs->numvisteammates = 0;

	for (teammate = -1; (teammate = BotGetNextTeamMate(bs, teammate, &ps)) >= 0;) {
		if (ps.stats[STAT_HEALTH] <= 0) {
			continue;
		}

		if (DistanceSquared(bs->origin, ps.origin) > 4096.0 * 4096.0) {
			continue;
		}

		if (BotEntityVisible(&bs->cur_ps, 360, teammate)) {
			bs->visteammates[bs->numvisteammates++] = teammate;
		}
	}
}

/*
=======================================================================================================================================
BotClientTravelTimeToGoal
=======================================================================================================================================
*/
int BotClientTravelTimeToGoal(int client, bot_goal_t *goal) {
	playerState_t ps;
	int areanum;

	if (BotAI_GetClientState(client, &ps)) {
		areanum = BotPointAreaNum(ps.origin);
	} else {
		areanum = 0;
	}

	if (!areanum) {
		return 1;
	}

	return trap_AAS_AreaTravelTimeToGoalArea(areanum, ps.origin, goal->areanum, TFL_DEFAULT);
}

/*
=======================================================================================================================================
BotSortTeamMatesByBaseTravelTime
=======================================================================================================================================
*/
int BotSortTeamMatesByBaseTravelTime(bot_state_t *bs, int *teammates, int maxteammates) {
	int i, j, k, numteammates, traveltime;
	char buf[MAX_INFO_STRING];
	int traveltimes[MAX_CLIENTS];
	bot_goal_t *goal = NULL;

	if (gametype == GT_CTF || gametype == GT_1FCTF) {
		if (BotTeam(bs) == TEAM_RED) {
			goal = &ctf_redflag;
		} else {
			goal = &ctf_blueflag;
		}
	} else if (gametype == GT_OBELISK || gametype == GT_HARVESTER) {
		if (BotTeam(bs) == TEAM_RED) {
			goal = &redobelisk;
		} else {
			goal = &blueobelisk;
		}
	}

	numteammates = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i) && goal) {
			traveltime = BotClientTravelTimeToGoal(i, goal);

			for (j = 0; j < numteammates; j++) {
				if (traveltime < traveltimes[j]) {
					for (k = numteammates; k > j; k--) {
						traveltimes[k] = traveltimes[k - 1];
						teammates[k] = teammates[k - 1];
					}

					break;
				}
			}

			traveltimes[j] = traveltime;
			teammates[j] = i;
			numteammates++;

			if (numteammates >= maxteammates) {
				break;
			}
		}
	}

	return numteammates;
}

/*
=======================================================================================================================================
BotSetTeamMateTaskPreference
=======================================================================================================================================
*/
void BotSetTeamMateTaskPreference(bot_state_t *bs, int teammate, int preference) {
	char teammatename[MAX_NETNAME];

	ctftaskpreferences[teammate].preference = preference;
	ClientName(teammate, teammatename, sizeof(teammatename));
	strcpy(ctftaskpreferences[teammate].name, teammatename);
}

/*
=======================================================================================================================================
BotGetTeamMateTaskPreference
=======================================================================================================================================
*/
int BotGetTeamMateTaskPreference(bot_state_t *bs, int teammate) {
	char teammatename[MAX_NETNAME];

	if (!ctftaskpreferences[teammate].preference) {
		return 0;
	}

	ClientName(teammate, teammatename, sizeof(teammatename));

	if (Q_stricmp(teammatename, ctftaskpreferences[teammate].name)) {
		return 0;
	}

	return ctftaskpreferences[teammate].preference;
}

/*
=======================================================================================================================================
BotSortTeamMatesByTaskPreference
=======================================================================================================================================
*/
int BotSortTeamMatesByTaskPreference(bot_state_t *bs, int *teammates, int numteammates) {
	int defenders[MAX_CLIENTS], numdefenders;
	int attackers[MAX_CLIENTS], numattackers;
	int roamers[MAX_CLIENTS], numroamers;
	int i, preference;

	numdefenders = numattackers = numroamers = 0;

	for (i = 0; i < numteammates; i++) {
		preference = BotGetTeamMateTaskPreference(bs, teammates[i]);

		if (preference & TEAMTP_DEFENDER) {
			defenders[numdefenders++] = teammates[i];
		} else if (preference & TEAMTP_ATTACKER) {
			attackers[numattackers++] = teammates[i];
		} else {
			roamers[numroamers++] = teammates[i];
		}
	}

	numteammates = 0;
	// defenders at the front of the list
	memcpy(&teammates[numteammates], defenders, numdefenders * sizeof(int));
	numteammates += numdefenders;
	// roamers in the middle
	memcpy(&teammates[numteammates], roamers, numroamers * sizeof(int));
	numteammates += numroamers;
	// attacker in the back of the list
	memcpy(&teammates[numteammates], attackers, numattackers * sizeof(int));
	numteammates += numattackers;
	return numteammates;
}

/*
=======================================================================================================================================
BotSayTeamOrderAlways
=======================================================================================================================================
*/
void BotSayTeamOrderAlways(bot_state_t *bs, int toclient) {
	char teamchat[MAX_MESSAGE_SIZE];
	char buf[MAX_MESSAGE_SIZE];
	char name[MAX_NETNAME];

	// if the bot is talking to itself
	if (bs->client == toclient) {
		// don't show the message just put it in the console message queue
		trap_BotGetChatMessage(bs->cs, buf, sizeof(buf));
		ClientName(bs->client, name, sizeof(name));
		Com_sprintf(teamchat, sizeof(teamchat), EC"(%s"EC")"EC": %s", name, buf);
		trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, teamchat);
	} else {
		trap_BotEnterChat(bs->cs, toclient, CHAT_TELL);
	}
}

/*
=======================================================================================================================================
BotSayTeamOrder
=======================================================================================================================================
*/
void BotSayTeamOrder(bot_state_t *bs, int toclient) {
	// voice chats only
	char buf[MAX_MESSAGE_SIZE];

	trap_BotGetChatMessage(bs->cs, buf, sizeof(buf));
}

/*
=======================================================================================================================================
BotVoiceChat
=======================================================================================================================================
*/
void BotVoiceChat(bot_state_t *bs, int toclient, char *voicechat) {

	if (toclient == -1) {
		// voice only say team
		trap_EA_Command(bs->client, va("vsay_team %s", voicechat));
	} else {
		// voice only tell single player
		trap_EA_Command(bs->client, va("vtell %d %s", toclient, voicechat));
	}
}

/*
=======================================================================================================================================
BotVoiceChatOnly
=======================================================================================================================================
*/
void BotVoiceChatOnly(bot_state_t *bs, int toclient, char *voicechat) {

	if (toclient == -1) {
		// voice only say team
		trap_EA_Command(bs->client, va("vosay_team %s", voicechat));
	} else {
		// voice only tell single player
		trap_EA_Command(bs->client, va("votell %d %s", toclient, voicechat));
	}
}

/*
=======================================================================================================================================
BotSayVoiceTeamOrder
=======================================================================================================================================
*/
void BotSayVoiceTeamOrder(bot_state_t *bs, int toclient, char *voicechat) {
	BotVoiceChat(bs, toclient, voicechat);
}

/*
=======================================================================================================================================

	HARVESTER

=======================================================================================================================================
*/

/*
=======================================================================================================================================
BotHarvesterOrders

X% defend the base, Y% harvest.
=======================================================================================================================================
*/
void BotHarvesterOrders(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
				{
					break;
				}
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will harvest
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the one second closest to the base also defends the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other one goes harvesting
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 70% defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% goes harvesting
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will harvest
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the one second closest to the base also defends the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other one goes harvesting
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 50% defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% goes harvesting
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will harvest
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others go harvesting
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 30% defend the base
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}
					// 60% go harvesting
					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will harvest
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others go harvesting
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 20% defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% go harvesting
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_harvest", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================

	OBELISK

=======================================================================================================================================
*/

/*
=======================================================================================================================================
BotObeliskOrders

X% in defence Y% in offence.
=======================================================================================================================================
*/
void BotObeliskOrders(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the one second closest to the base also defends the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other one attacks the enemy base
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 60% defend the base
					defenders = (int)(float)numteammates * 0.6 + 0.5;

					if (defenders > 6) {
						defenders = 6;
					}
					// 30% attack the enemy base
					attackers = (int)(float)numteammates * 0.3 + 0.5;

					if (attackers > 3) {
						attackers = 3;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the one second closest to the base also defends the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other one attacks the enemy base
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 50% defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% attack the enemy base
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 30% defend the base
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}
					// 60% attack the enemy base
					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others attack the enemy base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_OFFENSE);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_OFFENSE);
					break;
				}
				default:
				{
					// 20% defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% attack the enemy base
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_OFFENSE);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================

	ONEFLAG CTF

=======================================================================================================================================
*/

/*
=======================================================================================================================================
Bot1FCTFOrders_FlagAtCenter

X% defend the base, Y% get the flag.
=======================================================================================================================================
*/
void Bot1FCTFOrders_FlagAtCenter(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 70% defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% get the flag
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 50% defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% get the flag
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 40% defend the base
					defenders = (int)(float)numteammates * 0.4 + 0.5;

					if (defenders > 4) {
						defenders = 4;
					}
					// 50% get the flag
					attackers = (int)(float)numteammates * 0.5 + 0.5;

					if (attackers > 5) {
						attackers = 5;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 20% defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% get the flag
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
Bot1FCTFOrders_TeamHasFlag

X% towards neutral flag, Y% go towards enemy base and accompany flag carrier if visible.
=======================================================================================================================================
*/
void Bot1FCTFOrders_TeamHasFlag(bot_state_t *bs) {
	int numteammates, defenders, attackers, i, other;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME], carriername[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the one furthest from the base not carrying the flag to accompany the flag carrier
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					// 50% will defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the one furthest from the base not carrying the flag to accompany the flag carrier
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					// 30% will defend the base
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}
					// 60% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));
						// tell the one furthest from the base not carrying the flag to accompany the flag carrier
						if (teammates[2] != bs->flagcarrier) {
							other = teammates[2];
						} else {
							other = teammates[1];
						}

						ClientName(other, name, sizeof(name));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					// 20% will defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_attackenemybase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_OFFENSE);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));
						// tell the one furthest from the base not carrying the flag to accompany the flag carrier
						if (teammates[2] != bs->flagcarrier) {
							other = teammates[2];
						} else {
							other = teammates[1];
						}

						ClientName(other, name, sizeof(name));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					// 10% will defend the base
					defenders = (int)(float)numteammates * 0.1 + 0.5;

					if (defenders > 1) {
						defenders = 1;
					}
					// 80% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.8 + 0.5;

					if (attackers > 8) {
						attackers = 8;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
Bot1FCTFOrders_EnemyHasFlag

X% defend the base, Y% towards neutral flag.
=======================================================================================================================================
*/
void Bot1FCTFOrders_EnemyHasFlag(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// both defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);

					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other will also defend the base
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_DEFEND);
					break;
				}
				default:
				{
					// 80% will defend the base
					defenders = (int)(float)numteammates * 0.8 + 0.5;

					if (defenders > 8) {
						defenders = 8;
					}
					// 10% will try to return the flag
					attackers = (int)(float)numteammates * 0.1 + 0.5;

					if (attackers > 1) {
						attackers = 1;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 70% defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% try to return the flag
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
Bot1FCTFOrders_EnemyDroppedFlag

X% defend the base, Y% get the flag.
=======================================================================================================================================
*/
void Bot1FCTFOrders_EnemyDroppedFlag(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 70% defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% get the flag
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 50% defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% get the flag
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 30% defend the base
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}
					// 60% get the flag
					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 20% defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% get the flag
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
Bot1FCTFOrders
=======================================================================================================================================
*/
void Bot1FCTFOrders(bot_state_t *bs) {

	switch (bs->neutralflagstatus) {
		case 0:
			Bot1FCTFOrders_FlagAtCenter(bs);
			break;
		case 1:
			Bot1FCTFOrders_TeamHasFlag(bs);
			break;
		case 2:
			Bot1FCTFOrders_EnemyHasFlag(bs);
			break;
		case 3:
			Bot1FCTFOrders_EnemyDroppedFlag(bs);
			break;
	}
}

/*
=======================================================================================================================================

	CAPTURE THE FLAG

=======================================================================================================================================
*/

/*
=======================================================================================================================================
BotCTFOrders_BothFlagsNotAtBase
=======================================================================================================================================
*/
void BotCTFOrders_BothFlagsNotAtBase(bot_state_t *bs) {
	int numteammates, defenders, attackers, i, other;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME], carriername[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the one furthest from the the base not carrying the flag to accompany the flag carrier
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_RETURNFLAG);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					defenders = (int)(float)numteammates * 0.6 + 0.5;

					if (defenders > 6) {
						defenders = 6;
					}

					attackers = (int)(float)numteammates * 0.3 + 0.5;

					if (attackers > 3) {
						attackers = 3;
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[i]);
						}
					} else {
						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
							BotSayTeamOrder(bs, teammates[i]);
							BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
						}
					}

					for (i = 0; i < attackers; i++) {
						if (teammates[numteammates - i - 1] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_RETURNFLAG);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to accompany the flag carrier
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					}

					BotSayTeamOrder(bs, other);
					// tell the one furthest from the the base not carrying the flag to get the enemy flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_returnflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_RETURNFLAG);
					break;
				}
				default:
				{
					defenders = (int)(float)numteammates * 0.4 + 0.5;

					if (defenders > 4) {
						defenders = 4;
					}

					attackers = (int)(float)numteammates * 0.5 + 0.5;

					if (attackers > 5) {
						attackers = 5;
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[i]);
						}
					} else {
						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
							BotSayTeamOrder(bs, teammates[i]);
							BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
						}
					}

					for (i = 0; i < attackers; i++) {
						if (teammates[numteammates - i - 1] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the one furthest from the the base not carrying the flag to get the enemy flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}

					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[i]);
						}
					} else {
						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[i]);
							BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_GETFLAG);
						}
					}

					for (i = 0; i < attackers; i++) {
						if (teammates[numteammates - i - 1] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to attack the enemy base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the one furthest from the the base not carrying the flag to get the enemy flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						if (bs->flagcarrier == bs->client) {
							BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWME);
						} else {
							BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
							BotSayVoiceTeamOrder(bs, other, VOICECHAT_FOLLOWFLAGCARRIER);
						}
					} else {
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					}

					BotSayTeamOrder(bs, other);
					break;
				}
				default:
				{
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}

					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[i]);
						}
					} else {
						for (i = 0; i < defenders; i++) {
							if (teammates[i] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[i], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[i]);
							BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_GETFLAG);
						}
					}

					for (i = 0; i < attackers; i++) {
						if (teammates[numteammates - i - 1] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
BotCTFOrders_TeamFlagNotAtBase
=======================================================================================================================================
*/
void BotCTFOrders_TeamFlagNotAtBase(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		default:
		// very passive strate
		case CTFS_MAX_DEFENSIVE:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// keep one near the base for when the flag is returned
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// keep one near the base for when the flag is returned
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other two get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// keep some people near the base for when the flag is returned
					defenders = (int)(float)numteammates * 0.3 + 0.5;

					if (defenders > 3) {
						defenders = 3;
					}

					attackers = (int)(float)numteammates * 0.6 + 0.5;

					if (attackers > 6) {
						attackers = 6;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// both will go for the enemy flag
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_GETFLAG);

					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// everyone go for the flag
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_GETFLAG);

					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// keep some people near the base for when the flag is returned
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}

					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
BotCTFOrders_EnemyFlagNotAtBase
=======================================================================================================================================
*/
void BotCTFOrders_EnemyFlagNotAtBase(bot_state_t *bs) {
	int numteammates, defenders, attackers, i, other;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME], carriername[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the other also to defend the base
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				default:
				{
					// 70% will defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the other to get the flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 60% will defend the base
					defenders = (int)(float)numteammates * 0.6 + 0.5;

					if (defenders > 6) {
						defenders = 6;
					}
					// 30% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.3 + 0.5;

					if (attackers > 3) {
						attackers = 3;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_accompany", name, carriername, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWFLAGCARRIER);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the other to get the flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 40% will defend the base
					defenders = (int)(float)numteammates * 0.4 + 0.5;

					if (defenders > 4) {
						defenders = 4;
					}
					// 50% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.5 + 0.5;

					if (attackers > 5) {
						attackers = 5;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}
					// if we have a flag carrier
					if (bs->flagcarrier != -1) {
						ClientName(bs->flagcarrier, carriername, sizeof(carriername));

						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));

							if (bs->flagcarrier == bs->client) {
								BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_FOLLOWME);
							} else {
								BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
								BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
							}

							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						}
					} else {
						for (i = 0; i < attackers; i++) {
							if (teammates[numteammates - i - 1] == bs->flagcarrier) {
								continue;
							}

							ClientName(teammates[numteammates - i - 1], name, sizeof(name));
							BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
							BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
							BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
						}
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// tell the one not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					break;
				}
				case 3:
				{
					// tell the one closest to the base not carrying the flag to defend the base
					if (teammates[0] != bs->flagcarrier) {
						other = teammates[0];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_DEFEND);
					// tell the other to get the flag
					if (teammates[2] != bs->flagcarrier) {
						other = teammates[2];
					} else {
						other = teammates[1];
					}

					ClientName(other, name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, other);
					BotSayVoiceTeamOrder(bs, other, VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 20% will defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% accompanies the flag carrier
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						if (teammates[i] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						if (teammates[numteammates - i - 1] == bs->flagcarrier) {
							continue;
						}

						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
BotCTFOrders_BothFlagsAtBase
=======================================================================================================================================
*/
void BotCTFOrders_BothFlagsAtBase(bot_state_t *bs) {
	int numteammates, defenders, attackers, i;
	int teammates[MAX_CLIENTS] = {0};
	char name[MAX_NETNAME];

	// sort team mates by travel time to base
	numteammates = BotSortTeamMatesByBaseTravelTime(bs, teammates, sizeof(teammates));
	// sort team mates by CTF preference
	BotSortTeamMatesByTaskPreference(bs, teammates, numteammates);

	switch (bs->ctfstrategy) {
		// very passive strategy
		case CTFS_MAX_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 70% will defend the base
					defenders = (int)(float)numteammates * 0.7 + 0.5;

					if (defenders > 7) {
						defenders = 7;
					}
					// 20% get the flag
					attackers = (int)(float)numteammates * 0.2 + 0.5;

					if (attackers > 2) {
						attackers = 2;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		default:
		// passive strategy
		case CTFS_DEFENSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the second one closest to the base will defend the base
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 50% will defend the base
					defenders = (int)(float)numteammates * 0.5 + 0.5;

					if (defenders > 5) {
						defenders = 5;
					}
					// 40% get the flag
					attackers = (int)(float)numteammates * 0.4 + 0.5;

					if (attackers > 4) {
						attackers = 4;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// aggressive strategy
		case CTFS_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 40% will defend the base
					defenders = (int)(float)numteammates * 0.4 + 0.5;

					if (defenders > 4) {
						defenders = 4;
					}
					// 50% get the flag
					attackers = (int)(float)numteammates * 0.5 + 0.5;

					if (attackers > 5) {
						attackers = 5;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
		// very aggressive strategy
		case CTFS_MAX_AGGRESSIVE:
		{
			// different orders based on the number of team mates
			switch (numteammates) {
				case 1:
					break;
				case 2:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the other will get the flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);
					break;
				}
				case 3:
				{
					// the one closest to the base will defend the base
					ClientName(teammates[0], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
					BotSayTeamOrder(bs, teammates[0]);
					BotSayVoiceTeamOrder(bs, teammates[0], VOICECHAT_DEFEND);
					// the others should go for the enemy flag
					ClientName(teammates[1], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[1]);
					BotSayVoiceTeamOrder(bs, teammates[1], VOICECHAT_GETFLAG);

					ClientName(teammates[2], name, sizeof(name));
					BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
					BotSayTeamOrder(bs, teammates[2]);
					BotSayVoiceTeamOrder(bs, teammates[2], VOICECHAT_GETFLAG);
					break;
				}
				default:
				{
					// 20% will defend the base
					defenders = (int)(float)numteammates * 0.2 + 0.5;

					if (defenders > 2) {
						defenders = 2;
					}
					// 70% get the flag
					attackers = (int)(float)numteammates * 0.7 + 0.5;

					if (attackers > 7) {
						attackers = 7;
					}

					for (i = 0; i < defenders; i++) {
						ClientName(teammates[i], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_defendbase", name, NULL);
						BotSayTeamOrder(bs, teammates[i]);
						BotSayVoiceTeamOrder(bs, teammates[i], VOICECHAT_DEFEND);
					}

					for (i = 0; i < attackers; i++) {
						ClientName(teammates[numteammates - i - 1], name, sizeof(name));
						BotAI_BotInitialChat(bs, "cmd_getflag", name, NULL);
						BotSayTeamOrder(bs, teammates[numteammates - i - 1]);
						BotSayVoiceTeamOrder(bs, teammates[numteammates - i - 1], VOICECHAT_GETFLAG);
					}

					break;
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================
BotCTFOrders
=======================================================================================================================================
*/
void BotCTFOrders(bot_state_t *bs) {
	int flagstatus;

	if (BotTeam(bs) != TEAM_RED) {
		flagstatus = bs->blueflagstatus * 2 + bs->redflagstatus;
	} else {
		flagstatus = bs->redflagstatus * 2 + bs->blueflagstatus;
	}

	switch (flagstatus) {
		case 0:
			BotCTFOrders_BothFlagsAtBase(bs);
			break;
		case 1:
			BotCTFOrders_EnemyFlagNotAtBase(bs);
			break;
		case 2:
			BotCTFOrders_TeamFlagNotAtBase(bs);
			break;
		case 3:
			BotCTFOrders_BothFlagsNotAtBase(bs);
			break;
	}
}

/*
=======================================================================================================================================

	TEAM DEATHMATCH

=======================================================================================================================================
*/

/*
=======================================================================================================================================
BotCreateGroup
=======================================================================================================================================
*/
void BotCreateGroup(bot_state_t *bs, int *teammates, int groupsize) {
	char name[MAX_NETNAME], leadername[MAX_NETNAME];
	int i;

	// the others in the group will follow the teammates[0]
	ClientName(teammates[0], leadername, sizeof(leadername));

	for (i = 1; i < groupsize; i++) {
		ClientName(teammates[i], name, sizeof(name));

		if (teammates[0] != bs->client) {
			BotAI_BotInitialChat(bs, "cmd_accompany", name, leadername, NULL);
		} else {
			BotAI_BotInitialChat(bs, "cmd_accompanyme", name, NULL);
		}

		BotSayTeamOrderAlways(bs, teammates[i]);
	}
}

/*
=======================================================================================================================================
BotTeamOrders

FIXME: defend key areas?
=======================================================================================================================================
*/
void BotTeamOrders(bot_state_t *bs) {
	int teammates[MAX_CLIENTS] = {0};
	int numteammates, i;
	char buf[MAX_INFO_STRING];

	numteammates = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			teammates[numteammates] = i;
			numteammates++;
		}
	}

	switch (numteammates) {
		case 1:
			break;
		case 2:
		{
			// nothing special
			break;
		}
		case 3:
		{
			// have one follow another and one free roaming
			BotCreateGroup(bs, teammates, 2);
			break;
		}
		case 4:
		{
			BotCreateGroup(bs, teammates, 2); // a group of 2
			BotCreateGroup(bs, &teammates[2], 2); // a group of 2
			break;
		}
		case 5:
		{
			BotCreateGroup(bs, teammates, 2); // a group of 2
			BotCreateGroup(bs, &teammates[2], 3); // a group of 3
			break;
		}
		default:
		{
			if (numteammates <= 10) {
				for (i = 0; i < numteammates / 2; i++) {
					BotCreateGroup(bs, &teammates[i * 2], 2); // groups of 2
				}
			}

			break;
		}
	}
}

/*
=======================================================================================================================================

	TEAM AI

=======================================================================================================================================
*/

/*
=======================================================================================================================================
FindHumanTeamLeader
=======================================================================================================================================
*/
int FindHumanTeamLeader(bot_state_t *bs) {
	int i;

	for (i = 0; i < level.maxclients; i++) {
		if (g_entities[i].inuse) {
			// if this player is not a bot
			if (!(g_entities[i].r.svFlags & SVF_BOT)) {
				// if this player is ok with being the leader
				if (!notleader[i]) {
					// if this player is on the same team
					if (BotSameTeam(bs, i)) {
						ClientName(i, bs->teamleader, sizeof(bs->teamleader));
						// if not yet ordered to do anything
						if (!BotSetLastOrderedTask(bs)) {
							// go on defense by default
							BotVoiceChat_Defend(bs, i, SAY_TELL);
						}

						return qtrue;
					}
				}
			}
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotTeamAI
=======================================================================================================================================
*/
void BotTeamAI(bot_state_t *bs) {
	int numteammates;
	char netname[MAX_NETNAME];

	if (gametype < GT_TEAM) {
		return;
	}
	// make sure we've got a valid team leader
	if (!BotValidTeamLeader(bs)) {
		if (!FindHumanTeamLeader(bs)) {
			if (!bs->askteamleader_time && !bs->becometeamleader_time) {
				if (bs->entergame_time + 10 > FloatTime()) {
					bs->askteamleader_time = FloatTime() + 5 + random() * 10;
				} else {
					bs->becometeamleader_time = FloatTime() + 5 + random() * 10;
				}
			}

			if (bs->askteamleader_time && bs->askteamleader_time < FloatTime()) {
				// if asked for a team leader and no response
				BotAI_BotInitialChat(bs, "whoisteamleader", NULL);
				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
				bs->askteamleader_time = 0;
				bs->becometeamleader_time = FloatTime() + 8 + random() * 10;
			}

			if (bs->becometeamleader_time && bs->becometeamleader_time < FloatTime()) {
				BotAI_BotInitialChat(bs, "iamteamleader", NULL);
				trap_BotEnterChat(bs->cs, 0, CHAT_TEAM);
				BotSayVoiceTeamOrder(bs, -1, VOICECHAT_STARTLEADER);
				ClientName(bs->client, netname, sizeof(netname));
				strncpy(bs->teamleader, netname, sizeof(bs->teamleader));
				bs->teamleader[sizeof(bs->teamleader) - 1] = '\0';
				bs->becometeamleader_time = 0;
			}

			return;
		}
	}

	bs->askteamleader_time = 0;
	bs->becometeamleader_time = 0;

	ClientName(bs->client, netname, sizeof(netname));
	// return if this bot is NOT the team leader
	if (Q_stricmp(netname, bs->teamleader) != 0) {
		// set default strategy
		bs->ctfstrategy = CTFS_DEFENSIVE;
		return;
	}
	// if this bot IS the team leader
	if (!Q_stricmp(netname, bs->teamleader)) {
		bs->ctfstrategy = trap_Characteristic_BInteger(bs->character, CHARACTERISTIC_LEADER_STRATEGY, 1, 4);
	}

	numteammates = BotNumTeamMates(bs);
	// give orders
	switch (gametype) {
		case GT_TEAM:
		{
			// if someone wants to know what to do or if the number of teammates changed
			if (bs->forceorders || bs->numteammates != numteammates) {
				bs->teamgiveorders_time = FloatTime();
				bs->numteammates = numteammates;
				bs->forceorders = qfalse;
			}
			// if it's time to give orders
			if (bs->teamgiveorders_time && bs->teamgiveorders_time < FloatTime() - 5) {
				BotTeamOrders(bs);
				// give orders again after 120 seconds
				bs->teamgiveorders_time = FloatTime() + 120;
			}

			break;
		}
		case GT_CTF:
		{
			// if the enemy team is leading by more than 1 point, or if the enemy team leads and time limit has expired to 60%, choose the most aggressive strategy
			if (bs->ownteamscore + 1 < bs->enemyteamscore || (bs->ownteamscore < bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.6f)) {
				bs->ctfstrategy = CTFS_MAX_AGGRESSIVE;
			// if the own team is leading by more than 1 point, or if the own team leads and time limit has expired to 60%, choose the most defensive strategy
			} else if (bs->ownteamscore - 1 > bs->enemyteamscore || (bs->ownteamscore > bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.6f)) {
				bs->ctfstrategy = CTFS_MAX_DEFENSIVE;
			} else {
				// if there were no flag captures the last 4 minutes
				if (bs->lastflagcapture_time < FloatTime() - 240) {
					bs->lastflagcapture_time = FloatTime();
					// randomly change the CTF strategy
					if (random() < 0.4) {
						bs->ctfstrategy = CTFS_AGGRESSIVE;
					} else {
						bs->ctfstrategy = CTFS_DEFENSIVE;
					}
					// give orders and eventually change the strategy
					bs->teamgiveorders_time = FloatTime();
				}
			}
			// if the flag status changed or someone wants to know what to do or if the number of teammates changed
			if (bs->flagstatuschanged || bs->forceorders || bs->numteammates != numteammates) {
				bs->teamgiveorders_time = FloatTime();
				bs->numteammates = numteammates;
				bs->flagstatuschanged = qfalse;
				bs->forceorders = qfalse;
			}
			// if it's time to give orders
			if (bs->teamgiveorders_time && bs->teamgiveorders_time < FloatTime() - 5) {
				BotCTFOrders(bs);
				// give orders again after 30 seconds
				bs->teamgiveorders_time = FloatTime() + 30;
			}

			break;
		}
		case GT_1FCTF:
		{
			// if the enemy team is leading by more than 2 points, or if the enemy team leads and time limit has expired to 70%, choose the most aggressive strategy
			if (bs->ownteamscore + 2 < bs->enemyteamscore || (bs->ownteamscore < bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.7f)) {
				bs->ctfstrategy = CTFS_MAX_AGGRESSIVE;
			// if the own team is leading by more than 2 points, or if the own team leads and time limit has expired to 70%, choose the most defensive strategy
			} else if (bs->ownteamscore - 2 > bs->enemyteamscore || (bs->ownteamscore > bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.7f)) {
				bs->ctfstrategy = CTFS_MAX_DEFENSIVE;
			} else {
				// if there were no flag captures the last 2 minutes
				if (bs->lastflagcapture_time < FloatTime() - 120) {
					bs->lastflagcapture_time = FloatTime();
					// randomly change the CTF strategy
					if (random() < 0.4) {
						bs->ctfstrategy = CTFS_AGGRESSIVE;
					} else {
						bs->ctfstrategy = CTFS_DEFENSIVE;
					}
					// give orders and eventually change the strategy
					bs->teamgiveorders_time = FloatTime();
				}
			}
			// if the flag status changed or someone wants to know what to do or if the number of teammates changed
			if (bs->flagstatuschanged || bs->forceorders || bs->numteammates != numteammates) {
				bs->teamgiveorders_time = FloatTime();
				bs->numteammates = numteammates;
				bs->flagstatuschanged = qfalse;
				bs->forceorders = qfalse;
			}
			// if it's time to give orders
			if (bs->teamgiveorders_time && bs->teamgiveorders_time < FloatTime() - 5) {
				Bot1FCTFOrders(bs);
				// give orders again after 30 seconds
				bs->teamgiveorders_time = FloatTime() + 30;
			}

			break;
		}
		case GT_OBELISK:
		{
			// if the enemy team is leading by more than 1 point, or if the enemy team leads and time limit has expired to 50%, choose the most aggressive strategy
			if (bs->ownteamscore + 1 < bs->enemyteamscore || (bs->ownteamscore < bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.5f)) {
				bs->ctfstrategy = CTFS_MAX_AGGRESSIVE;
			// if the own team is leading by more than 1 point, or if the own team leads and time limit has expired to 50%, choose the most defensive strategy
			} else if (bs->ownteamscore - 1 > bs->enemyteamscore || (bs->ownteamscore > bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.5f)) {
				bs->ctfstrategy = CTFS_MAX_DEFENSIVE;
			} else {
				// if none of the teams scored during the last 4 minutes
				if (bs->lastteamscore_time < FloatTime() - 240) {
					bs->lastteamscore_time = FloatTime();
					// randomly change the CTF strategy
					if (random() < 0.4) {
						bs->ctfstrategy = CTFS_AGGRESSIVE;
					} else {
						bs->ctfstrategy = CTFS_DEFENSIVE;
					}
					// give orders and eventually change the strategy
					bs->teamgiveorders_time = FloatTime();
				}
			}
			// if someone wants to know what to do or if the number of teammates changed
			if (bs->forceorders || bs->numteammates != numteammates) {
				bs->teamgiveorders_time = FloatTime();
				bs->numteammates = numteammates;
				bs->forceorders = qfalse;
			}
			// if it's time to give orders
			if (bs->teamgiveorders_time && bs->teamgiveorders_time < FloatTime() - 5) {
				BotObeliskOrders(bs);
				// give orders again after 30 seconds
				bs->teamgiveorders_time = FloatTime() + 30;
			}

			break;
		}
		case GT_HARVESTER:
		{
			// if the enemy team is leading by more than the half of the capturelimit points and time limit has expired to 60%, choose the most aggressive strategy
			if (bs->ownteamscore + (g_capturelimit.integer * 0.5f) < bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.6f) {
				bs->ctfstrategy = CTFS_MAX_AGGRESSIVE;
			// if the own team is leading by more than the half of the capturelimit points and time limit has expired to 60%, choose the most defensive strategy
			} else if (bs->ownteamscore - (g_capturelimit.integer * 0.5f) > bs->enemyteamscore && level.time - level.startTime > (g_timelimit.integer * 60000) * 0.6f) {
				bs->ctfstrategy = CTFS_MAX_DEFENSIVE;
			} else {
				// if none of the teams scored during the last 3 minutes
				if (bs->lastteamscore_time < FloatTime() - 180) {
					bs->lastteamscore_time = FloatTime();
					// randomly change the CTF strategy
					if (random() < 0.4) {
						bs->ctfstrategy = CTFS_AGGRESSIVE;
					} else {
						bs->ctfstrategy = CTFS_DEFENSIVE;
					}
					// give orders and eventually change the strategy
					bs->teamgiveorders_time = FloatTime();
				}
			}
			// if someone wants to know what to do or if the number of teammates changed
			if (bs->forceorders || bs->numteammates != numteammates) {
				bs->teamgiveorders_time = FloatTime();
				bs->numteammates = numteammates;
				bs->forceorders = qfalse;
			}
			// if it's time to give orders
			if (bs->teamgiveorders_time && bs->teamgiveorders_time < FloatTime() - 5) {
				BotHarvesterOrders(bs);
				// give orders again after 30 seconds
				bs->teamgiveorders_time = FloatTime() + 30;
			}

			break;
		}
	}
}
