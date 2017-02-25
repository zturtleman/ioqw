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

/*
=======================================================================================================================================

	REMOVE BOTS MENU

=======================================================================================================================================
*/

#include "ui_local.h"

#define ART_BACKGROUND "menu/art/addbotframe"
#define ART_BACK0 "menu/art/back_0"
#define ART_BACK1 "menu/art/back_1"
#define ART_DELETE0 "menu/art/delete_0"
#define ART_DELETE1 "menu/art/delete_1"
#define ART_ARROWS "menu/art/arrows_vert_0"
#define ART_ARROWUP "menu/art/arrows_vert_top"
#define ART_ARROWDOWN "menu/art/arrows_vert_bot"
#define ART_SELECT "menu/art/opponents_select"

#define ART_KICK0 "menu/ui_art/kick_0"
#define ART_KICK1 "menu/ui_art/kick_1"
#define ART_VOTE0 "menu/ui_art/vote_0"
#define ART_VOTE1 "menu/ui_art/vote_1"

enum {
	ID_UP,
	ID_DOWN,
	ID_DELETE,
	ID_BACK,
	ID_BOTNAME0,
	ID_BOTNAME1,
	ID_BOTNAME2,
	ID_BOTNAME3,
	ID_BOTNAME4,
	ID_BOTNAME5,
	ID_BOTNAME6
};

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s background;
	menubitmap_s arrows;
	menubitmap_s up;
	menubitmap_s down;
	menutext_s bots[7];
	menubitmap_s icon;
	menubitmap_s icon_hilite;
	menubitmap_s deleteBot;
	menubitmap_s back;
	int numBots;
	int baseBotNum;
	int selectedBotNum;
	char botnames[7][32];
	int botClientNums[MAX_BOTS];
	char boticon[MAX_QPATH];
	int gametype;
	int action; // RBM_* type of menu
} removeBotsMenuInfo_t;

static removeBotsMenuInfo_t removeBotsMenuInfo;

/*
=======================================================================================================================================
UI_RemoveBotsMenu_SetBotNames
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_SetBotNames(void) {
	int n;
	char info[MAX_INFO_STRING], namebuf[64];

	for (n = 0; (n < 7) && (removeBotsMenuInfo.baseBotNum + n < removeBotsMenuInfo.numBots); n++) {
		trap_GetConfigString(CS_PLAYERS + removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.baseBotNum + n], info, MAX_INFO_STRING);
		Q_strncpyz(namebuf, Info_ValueForKey(info, "n"), 64);
		Q_CleanStr(namebuf);
		Q_strncpyz(removeBotsMenuInfo.botnames[n], namebuf, sizeof(removeBotsMenuInfo.botnames[n]));
	}
}

/*
=======================================================================================================================================
RemoveBots_SetBotIcon
=======================================================================================================================================
*/
static void RemoveBots_SetBotIcon(void) {
	char info[MAX_INFO_STRING];
	int index, team;

	index = CS_PLAYERS + removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.baseBotNum + removeBotsMenuInfo.selectedBotNum];

	trap_GetConfigString(index, info, MAX_INFO_STRING);

	UI_ServerPlayerIcon(Info_ValueForKey(info, "model"), removeBotsMenuInfo.boticon, MAX_QPATH);

	removeBotsMenuInfo.icon.shader = 0;

	if (removeBotsMenuInfo.gametype < GT_TEAM) {
		return;
	}

	team = atoi(Info_ValueForKey(info, "t"));

	if (team == TEAM_RED) {
		removeBotsMenuInfo.icon_hilite.focuscolor = color_red;
	} else if (team == TEAM_BLUE) {
		removeBotsMenuInfo.icon_hilite.focuscolor = color_blue;
	} else {
		removeBotsMenuInfo.icon_hilite.focuscolor = color_white;
	}
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_DeleteEvent
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_DeleteEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (removeBotsMenuInfo.action) {
		case RBM_KICKBOT:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("clientkick %i\n", removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.baseBotNum + removeBotsMenuInfo.selectedBotNum]));
			break;
		case RBM_CALLVOTEKICK:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote clientkick %i\n", removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.baseBotNum + removeBotsMenuInfo.selectedBotNum]));
			break;
		case RBM_CALLVOTELEADER:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callteamvote leader %i\n", removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.baseBotNum + removeBotsMenuInfo.selectedBotNum]));
			break;
	}
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_BotEvent
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_BotEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	removeBotsMenuInfo.bots[removeBotsMenuInfo.selectedBotNum].color = color_orange;
	removeBotsMenuInfo.selectedBotNum = ((menucommon_s *)ptr)->id - ID_BOTNAME0;
	removeBotsMenuInfo.bots[removeBotsMenuInfo.selectedBotNum].color = color_white;

	RemoveBots_SetBotIcon();
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_BackEvent
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_BackEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	UI_PopMenu();
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_UpEvent
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_UpEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (removeBotsMenuInfo.baseBotNum > 0) {
		removeBotsMenuInfo.baseBotNum--;
		UI_RemoveBotsMenu_SetBotNames();
		RemoveBots_SetBotIcon();
	}
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_DownEvent
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_DownEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (removeBotsMenuInfo.baseBotNum + 7 < removeBotsMenuInfo.numBots) {
		removeBotsMenuInfo.baseBotNum++;
		UI_RemoveBotsMenu_SetBotNames();
		RemoveBots_SetBotIcon();
	}
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_GetBots
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_GetBots(void) {
	int numPlayers;
	int isBot;
	int playerTeam;
	int team;
	int n;
	char info[MAX_INFO_STRING];

	trap_GetConfigString(CS_SERVERINFO, info, sizeof(info));

	numPlayers = atoi(Info_ValueForKey(info, "sv_maxclients"));
	removeBotsMenuInfo.numBots = 0;

	playerTeam = UI_CurrentPlayerTeam();

	for (n = 0; n < numPlayers; n++) {
		trap_GetConfigString(CS_PLAYERS + n, info, MAX_INFO_STRING);

		if (info[0] == '\0') {
			continue;
		}

		isBot = atoi(Info_ValueForKey(info, "skill"));
		team = atoi(Info_ValueForKey(info, "t"));

		if (removeBotsMenuInfo.action == RBM_KICKBOT) {
			if (!isBot) {
				continue;
			}
		} else if (removeBotsMenuInfo.action == RBM_CALLVOTEKICK) {
			if (isBot) {
				continue;
			}
		} else if (removeBotsMenuInfo.action == RBM_CALLVOTELEADER) {
			if (team != playerTeam) {
				continue;
			}
		}

		removeBotsMenuInfo.botClientNums[removeBotsMenuInfo.numBots] = n;
		removeBotsMenuInfo.numBots++;
	}
}

/*
=======================================================================================================================================
UI_RemoveBots_Cache
=======================================================================================================================================
*/
void UI_RemoveBots_Cache(void) {

	trap_R_RegisterShaderNoMip(ART_BACKGROUND);
	trap_R_RegisterShaderNoMip(ART_BACK0);
	trap_R_RegisterShaderNoMip(ART_BACK1);
	trap_R_RegisterShaderNoMip(ART_DELETE0);
	trap_R_RegisterShaderNoMip(ART_DELETE1);
}

/*
=======================================================================================================================================
RemoveBots_MenuDraw
=======================================================================================================================================
*/
static void RemoveBots_MenuDraw(void) {

	// draw the controls
	Menu_Draw(&removeBotsMenuInfo.menu);
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu_Init
=======================================================================================================================================
*/
static void UI_RemoveBotsMenu_Init(int action) {
	int n;
	int count;
	int y;
	char info[MAX_INFO_STRING];

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);

	memset(&removeBotsMenuInfo, 0, sizeof(removeBotsMenuInfo));

	removeBotsMenuInfo.menu.fullscreen = qfalse;
	removeBotsMenuInfo.menu.wrapAround = qtrue;
	removeBotsMenuInfo.menu.draw = RemoveBots_MenuDraw;
	removeBotsMenuInfo.gametype = atoi(Info_ValueForKey(info, "g_gametype"));
	removeBotsMenuInfo.action = action;

	UI_RemoveBots_Cache();
	UI_RemoveBotsMenu_GetBots();
	UI_RemoveBotsMenu_SetBotNames();

	count = removeBotsMenuInfo.numBots < 7 ? removeBotsMenuInfo.numBots : 7;

	removeBotsMenuInfo.banner.generic.type = MTYPE_BTEXT;
	removeBotsMenuInfo.banner.generic.x = 320;
	removeBotsMenuInfo.banner.generic.y = 16;
	removeBotsMenuInfo.banner.color = color_white;
	removeBotsMenuInfo.banner.style = UI_CENTER;

	if (action == RBM_CALLVOTEKICK) {
		removeBotsMenuInfo.banner.string = "Callvote Kick";
	} else if (action == RBM_CALLVOTELEADER) {
		removeBotsMenuInfo.banner.string = "Callvote Team Leader";
	} else {
		removeBotsMenuInfo.banner.string = "Remove Bots";
	}

	removeBotsMenuInfo.background.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.background.generic.name = ART_BACKGROUND;
	removeBotsMenuInfo.background.generic.flags = QMF_INACTIVE;
	removeBotsMenuInfo.background.generic.x = 320 - 233;
	removeBotsMenuInfo.background.generic.y = 240 - 166;
	removeBotsMenuInfo.background.width = 466;
	removeBotsMenuInfo.background.height = 332;

	removeBotsMenuInfo.arrows.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.arrows.generic.name = ART_ARROWS;
	removeBotsMenuInfo.arrows.generic.flags = QMF_INACTIVE;
	removeBotsMenuInfo.arrows.generic.x = 200;
	removeBotsMenuInfo.arrows.generic.y = 128;
	removeBotsMenuInfo.arrows.width = 64;
	removeBotsMenuInfo.arrows.height = 128;

	removeBotsMenuInfo.up.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.up.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	removeBotsMenuInfo.up.generic.x = 200;
	removeBotsMenuInfo.up.generic.y = 128;
	removeBotsMenuInfo.up.generic.id = ID_UP;
	removeBotsMenuInfo.up.generic.callback = UI_RemoveBotsMenu_UpEvent;
	removeBotsMenuInfo.up.width = 64;
	removeBotsMenuInfo.up.height = 64;
	removeBotsMenuInfo.up.focuspic = ART_ARROWUP;

	removeBotsMenuInfo.down.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.down.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	removeBotsMenuInfo.down.generic.x = 200;
	removeBotsMenuInfo.down.generic.y = 128 + 64;
	removeBotsMenuInfo.down.generic.id = ID_DOWN;
	removeBotsMenuInfo.down.generic.callback = UI_RemoveBotsMenu_DownEvent;
	removeBotsMenuInfo.down.width = 64;
	removeBotsMenuInfo.down.height = 64;
	removeBotsMenuInfo.down.focuspic = ART_ARROWDOWN;

	for (n = 0, y = 120; n < count; n++, y += 20) {
		removeBotsMenuInfo.bots[n].generic.type = MTYPE_PTEXT;
		removeBotsMenuInfo.bots[n].generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		removeBotsMenuInfo.bots[n].generic.id = ID_BOTNAME0 + n;
		removeBotsMenuInfo.bots[n].generic.x = 320 - 56;
		removeBotsMenuInfo.bots[n].generic.y = y;
		removeBotsMenuInfo.bots[n].generic.callback = UI_RemoveBotsMenu_BotEvent;
		removeBotsMenuInfo.bots[n].string = removeBotsMenuInfo.botnames[n];
		removeBotsMenuInfo.bots[n].color = color_orange;
		removeBotsMenuInfo.bots[n].style = UI_LEFT|UI_SMALLFONT;
	}

	removeBotsMenuInfo.deleteBot.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.deleteBot.generic.id = ID_DELETE;
	removeBotsMenuInfo.deleteBot.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	removeBotsMenuInfo.deleteBot.generic.callback = UI_RemoveBotsMenu_DeleteEvent;
	removeBotsMenuInfo.deleteBot.generic.x = 320 + 128 - 128;
	removeBotsMenuInfo.deleteBot.generic.y = 256 + 128 - 64;
	removeBotsMenuInfo.deleteBot.width = 128;
	removeBotsMenuInfo.deleteBot.height = 64;

	if (action == RBM_CALLVOTEKICK) {
		removeBotsMenuInfo.deleteBot.generic.name = ART_KICK0;
		removeBotsMenuInfo.deleteBot.focuspic = ART_KICK1;
	} else if (action == RBM_CALLVOTELEADER) {
		removeBotsMenuInfo.deleteBot.generic.name = ART_VOTE0;
		removeBotsMenuInfo.deleteBot.focuspic = ART_VOTE1;
	} else {
		removeBotsMenuInfo.deleteBot.generic.name = ART_DELETE0;
		removeBotsMenuInfo.deleteBot.focuspic = ART_DELETE1;
	}

	removeBotsMenuInfo.back.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.back.generic.name = ART_BACK0;
	removeBotsMenuInfo.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	removeBotsMenuInfo.back.generic.id = ID_BACK;
	removeBotsMenuInfo.back.generic.callback = UI_RemoveBotsMenu_BackEvent;
	removeBotsMenuInfo.back.generic.x = 320 - 128;
	removeBotsMenuInfo.back.generic.y = 256 + 128 - 64;
	removeBotsMenuInfo.back.width = 128;
	removeBotsMenuInfo.back.height = 64;
	removeBotsMenuInfo.back.focuspic = ART_BACK1;

	removeBotsMenuInfo.icon_hilite.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.icon_hilite.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_HIGHLIGHT;
	removeBotsMenuInfo.icon_hilite.generic.x = 190 - 64 - 15; // 320 + 128 - 15;
	removeBotsMenuInfo.icon_hilite.generic.y = 240 - 32 - 16;
	removeBotsMenuInfo.icon_hilite.width = 128;
	removeBotsMenuInfo.icon_hilite.height = 128;
	removeBotsMenuInfo.icon_hilite.focuspic = ART_SELECT;
	removeBotsMenuInfo.icon_hilite.focuscolor = color_red;

	removeBotsMenuInfo.icon.generic.type = MTYPE_BITMAP;
	removeBotsMenuInfo.icon.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	removeBotsMenuInfo.icon.generic.name = removeBotsMenuInfo.boticon;
	removeBotsMenuInfo.icon.generic.x = 190 - 64; // 320 + 128;
	removeBotsMenuInfo.icon.generic.y = 240 - 32;
	removeBotsMenuInfo.icon.width = 64;
	removeBotsMenuInfo.icon.height = 64;

	if (removeBotsMenuInfo.numBots == 0) {
		removeBotsMenuInfo.icon_hilite.generic.flags |= QMF_HIDDEN;
		removeBotsMenuInfo.icon.generic.flags |= QMF_HIDDEN;
	}

	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.background);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.banner);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.arrows);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.up);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.down);

	for (n = 0; n < count; n++) {
		Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.bots[n]);
	}

	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.deleteBot);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.back);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.icon_hilite);
	Menu_AddItem(&removeBotsMenuInfo.menu, &removeBotsMenuInfo.icon);

	removeBotsMenuInfo.baseBotNum = 0;
	removeBotsMenuInfo.selectedBotNum = 0;
	removeBotsMenuInfo.bots[0].color = color_white;

	RemoveBots_SetBotIcon();
}

/*
=======================================================================================================================================
UI_RemoveBotsMenu

Supports kicking a bot, kicking a player, or voting for a leader.
All are so similar that they're not worth writing and maintaining separate modules.
=======================================================================================================================================
*/
void UI_RemoveBotsMenu(int type) {

	UI_RemoveBotsMenu_Init(type);
	UI_PushMenu(&removeBotsMenuInfo.menu);
}
