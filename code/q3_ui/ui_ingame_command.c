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

	INGAME MENU

=======================================================================================================================================
*/

#include "ui_local.h"
#include "ui_dynamicmenu.h"

#define INGAME_FRAME "menu/art/addbotframe"
#define INGAME_SCROLL "menu/ui_art/separator"

enum {
	ID_SETUP,
	ID_TEAM,
	ID_LEAVEARENA,
	ID_RESTART,
	ID_QUIT,
	ID_SERVERINFO,
	ID_ADDBOTS,
	ID_REMOVEBOTS,
	ID_TEAMORDERS,
	ID_RESUME,
	ID_NEXTMAP
};

/*
=======================================================================================================================================
Dynamic_InGame_EventHandler

May be used by dynamic menu system also.
=======================================================================================================================================
*/
static void Dynamic_InGame_EventHandler(int id) {

	switch (id) {
		case ID_TEAM:
			break;
		case ID_SETUP:
			break;
		case ID_LEAVEARENA:
			break;
		case ID_RESTART:
			break;
		case ID_QUIT:
			break;
		case ID_SERVERINFO:
			break;
		case ID_ADDBOTS:
			break;
		case ID_REMOVEBOTS:
			break;
		case ID_TEAMORDERS:
			UI_BotCommandMenu_f();
			break;
		case ID_RESUME:
			UI_PopMenu();
			break;
		case ID_NEXTMAP:
			break;
	}
}

/*
=======================================================================================================================================
Dynamic_InGame_Event
=======================================================================================================================================
*/
static void Dynamic_InGame_Event(void *ptr, int notification) {

	if (notification != QM_ACTIVATED) {
		return;
	}

	Dynamic_InGame_EventHandler(((menucommon_s *)ptr)->id);
}

/*
=======================================================================================================================================

	INGAME ESCAPE MENU, USING DYNAMIC MENU SYSTEM

=======================================================================================================================================
*/

typedef struct {
	int gametype;
	char *menu;
} gametypeMenu;

static gametypeMenu gametypeMenu_data[] = {
	{GT_FFA, "Free For All"},
	{GT_TOURNAMENT, "Tournament"},
	{GT_TEAM, "Team Deathmatch"},
	{GT_CTF, "Capture The Flag"},
	{GT_1FCTF, "One Flag CTF"},
	{GT_OBELISK, "Overload"},
	{GT_HARVESTER, "Harvester"}
};

static int gametypeMenu_size = sizeof(gametypeMenu_data) / sizeof(gametypeMenu_data[0]);
// main dynamic in game menu
enum {
	IGM_START,
	IGM_CALLVOTE,
	IGM_VOTE,
	IGM_TEAMORDERS,
	IGM_SETUP,
	IGM_BOTS,
	IGM_MAP,
	IGM_DEMOS,
	IGM_EXIT,
	IGM_CLOSE
};
// callvote map options
enum {
	CVM_SELECTMAP,
	CVM_NEXTMAP,
	CVM_MAPRESTART
};
// callvote options
enum {
	IGCV_MAP,
	IGCV_GAMETYPE,
	IGCV_MISC,
	IGCV_KICK,
	IGCV_TEAMBALANCE,
	IGCV_LEADER
};
// vote options
enum {
	IGV_YES,
	IGV_NO,
	IGV_TEAMYES,
	IGV_TEAMNO
};
// record options
enum {
	IGD_RECORD,
	IGD_STOP
};
// setup options
enum {
	IGS_PLAYER,
	IGS_MODEL,
	IGS_CONTROLS,
	IGS_OPTIONS,
	IGS_GRAPHICS,
	IGS_DISPLAY,
	IGS_SOUND,
	IGS_NETWORK,
	IGS_LOADSAVE
};
// join team options
enum {
	DM_START_SPECTATOR,
	DM_START_GAME,
	DM_START_RED,
	DM_START_BLUE,
	DM_START_AUTO,
	DM_READY,
	DM_START_MAX
};

static char *jointeam_cmd[DM_START_MAX] = {
	"spectator",	// DM_START_SPECTATOR
	"free",			// DM_START_GAME
	"red",			// DM_START_RED
	"blue",			// DM_START_BLUE
	"auto",			// DM_START_AUTO
	"ready"			// DM_READY
};

/*
=======================================================================================================================================
InGameDynamic_Close
=======================================================================================================================================
*/
static void InGameDynamic_Close(void) {
//	UI_PopMenu();
}

/*
=======================================================================================================================================
IG_Unlagged_Event
=======================================================================================================================================
*/
static void IG_Unlagged_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	switch (id) {
		case 0:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote unlagged off\n"));
			break;
		case 1:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote unlagged on\n"));
			break;
	}
}

/*
=======================================================================================================================================
IG_FragLimit_Event
=======================================================================================================================================
*/
static void IG_FragLimit_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote fraglimit %i\n", id));
}

/*
=======================================================================================================================================
IG_CaptureLimit_Event
=======================================================================================================================================
*/
static void IG_CaptureLimit_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote capturelimit %i\n", id));
}

/*
=======================================================================================================================================
IG_TimeLimit_Event
=======================================================================================================================================
*/
static void IG_TimeLimit_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote timelimit %i\n", id));
}

/*
=======================================================================================================================================
IG_UseOldInGame_Event
=======================================================================================================================================
*/
static void IG_UseOldInGame_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	InGameDynamic_Close();

	switch (id) {
		case ID_ADDBOTS:
		case ID_REMOVEBOTS:
		case ID_RESTART:
		case ID_LEAVEARENA:
		case ID_QUIT:
		case ID_NEXTMAP:
			break;
		default:
			Com_Printf("IG_UseOldInGame_Event: unknown id (%i)", id);
			return;
	}

	Dynamic_InGame_EventHandler(id);
}

/*
=======================================================================================================================================
IG_Setup_Event
=======================================================================================================================================
*/
static void IG_Setup_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	InGameDynamic_Close();

	switch (id) {
		case IGS_PLAYER:
			UI_PlayerSettingsMenu();
			break;
		case IGS_MODEL:
			UI_PlayerModelMenu();
			break;
		case IGS_CONTROLS:
			UI_ControlsMenu();
			break;
		case IGS_OPTIONS:
			UI_GraphicsOptionsMenu();
			break;
		case IGS_GRAPHICS:
			UI_GraphicsOptionsMenu();
			break;
		case IGS_DISPLAY:
			UI_DisplayOptionsMenu();
			break;
		case IGS_SOUND:
			UI_SoundOptionsMenu();
			break;
		case IGS_NETWORK:
			UI_NetworkOptionsMenu();
			break;
		case IGS_LOADSAVE:
			UI_SaveConfigMenu();
			break;
		default:
			Com_Printf("IG_Setup_Event: unknown id (%i)", id);
			return;
	}
}

/*
=======================================================================================================================================
IG_CallVoteGameType_Event
=======================================================================================================================================
*/
static void IG_CallVoteGameType_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	switch (id) {
		case GT_FFA:
			break;
		case GT_TOURNAMENT:
			break;
		case GT_TEAM:
			break;
		case GT_CTF:
			break;
		case GT_1FCTF:
			break;
		case GT_OBELISK:
			break;
		case GT_HARVESTER:
			break;
		default:
			Com_Printf("IG_CallVoteGameType_Event: unknown game - id (%i)", id);
			return;
	}

	trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote g_gametype %i\n", id));
}

/*
=======================================================================================================================================
IG_CallVoteMaps_Event
=======================================================================================================================================
*/
static void IG_CallVoteMaps_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	switch (id) {
		case CVM_SELECTMAP:
			UI_VoteMapMenu();
			break;
		case CVM_NEXTMAP:
			UI_ForceMenuOff();
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote nextmap\n"));
			break;
		case CVM_MAPRESTART:
			UI_ForceMenuOff();
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote map_restart\n"));
			break;
		default:
			UI_ForceMenuOff();
			Com_Printf("IG_CallVoteMaps_Event: unknown id (%i)", id);
			return;
	}
}

/*
=======================================================================================================================================
IG_CallVote_Event
=======================================================================================================================================
*/
static void IG_CallVote_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	InGameDynamic_Close();

	switch (id) {
		case IGCV_KICK:
			UI_RemoveBotsMenu(RBM_CALLVOTEKICK);
			break;
		case IGCV_TEAMBALANCE:
			UI_ForceMenuOff();
			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote teambalance\n"));
			break;
		case IGCV_LEADER:
			UI_RemoveBotsMenu(RBM_CALLVOTELEADER);
			break;
		default:
			Com_Printf("IG_CallVote_Event: unknown id (%i)", id);
			return;
	}
}

/*
=======================================================================================================================================
IG_Vote_Event
=======================================================================================================================================
*/
static void IG_Vote_Event(int index) {
	int id;
	char *s;

	id = DynamicMenu_IdAtIndex(index);

	InGameDynamic_Close();

	switch (id) {
		case IGV_YES:
			s = "vote yes\n";
			break;
		case IGV_NO:
			s = "vote no\n";
			break;
		case IGV_TEAMYES:
			s = "teamvote yes\n";
			break;
		case IGV_TEAMNO:
			s = "teamvote yes\n";
			break;
		default:
			Com_Printf("IG_Vote_Event: unknown id (%i)", id);
			return;
	}

	trap_Cmd_ExecuteText(EXEC_APPEND, s);
}

/*
=======================================================================================================================================
IG_Demos_Event
=======================================================================================================================================
*/
static void IG_Demos_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	InGameDynamic_Close();

	switch (id) {
		case IGD_RECORD:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("record"));
			break;
		case IGD_STOP:
			trap_Cmd_ExecuteText(EXEC_APPEND, va("stoprecord\n"));
			break;
		default:
			Com_Printf("IG_Demo_Event: unknown id (%i)", id);
			return;
	}
}

/*
=======================================================================================================================================
IG_TeamOrders_Event
=======================================================================================================================================
*/
static void IG_TeamOrders_Event(int index) {

	UI_PopMenu();
	UI_BotCommandMenu_f();
}

/*
=======================================================================================================================================
IG_Start_Event
=======================================================================================================================================
*/
static void IG_Start_Event(int index) {
	int id;

	id = DynamicMenu_IdAtIndex(index);

	UI_ForceMenuOff();

	if (id < 0 || id >= DM_START_MAX) {
		Com_Printf("IG_Start_Event: unknown id (%i)", id);
		return;
	}

	trap_Cmd_ExecuteText(EXEC_APPEND, va("team %s\n", jointeam_cmd[id]));
}

/*
=======================================================================================================================================
IG_TimeLimit_SubMenu
=======================================================================================================================================
*/
static void IG_TimeLimit_SubMenu(void) {
	int tmp;
	int depth;

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Unlimited", 0, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("5", 5, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("10", 10, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("15", 15, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("20", 20, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("25", 25, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("30", 30, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("45", 45, NULL, IG_TimeLimit_Event);
	DynamicMenu_AddItem("60", 60, NULL, IG_TimeLimit_Event);

	depth = DynamicMenu_Depth();
	tmp = UI_ServerTimelimit();

	if ((tmp == 0) || (tmp == 5) || (tmp == 10) || (tmp == 15) || (tmp == 20) || (tmp == 25) || (tmp == 30) || (tmp == 45) || (tmp == 60)) {
		DynamicMenu_SetFlags(depth, tmp, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Unlagged_SubMenu
=======================================================================================================================================
*/
static void IG_Unlagged_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Yes", 1, NULL, IG_Unlagged_Event);
	DynamicMenu_AddItem("No", 0, NULL, IG_Unlagged_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_FragLimit_SubMenu
=======================================================================================================================================
*/
static void IG_FragLimit_SubMenu(void) {
	int tmp;
	int depth;

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Unlimited", 0, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("10", 10, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("15", 15, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("20", 20, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("30", 30, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("40", 40, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("50", 50, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("75", 75, NULL, IG_FragLimit_Event);
	DynamicMenu_AddItem("100", 100, NULL, IG_FragLimit_Event);

	depth = DynamicMenu_Depth();
	tmp = UI_ServerFraglimit();

	if ((tmp == 0) || (tmp == 10) || (tmp == 15) || (tmp == 20) || (tmp == 30) || (tmp == 40) || (tmp == 50) || (tmp == 75) || (tmp == 100)) {
		DynamicMenu_SetFlags(depth, tmp, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_CaptureLimit_SubMenu
=======================================================================================================================================
*/
static void IG_CaptureLimit_SubMenu(void) {
	int tmp;
	int depth;

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Unlimited", 0, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("1", 1, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("2", 2, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("4", 4, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("6", 6, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("8", 8, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("10", 10, NULL, IG_CaptureLimit_Event);
	DynamicMenu_AddItem("12", 12, NULL, IG_CaptureLimit_Event);

	depth = DynamicMenu_Depth();
	tmp = UI_ServerCapturelimit();

	if ((tmp == 0) || (tmp == 1) || (tmp == 2) || (tmp == 4) || (tmp == 6) || (tmp == 8) || (tmp == 10) || (tmp == 12)) {
		DynamicMenu_SetFlags(depth, tmp, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Map_SubMenu
=======================================================================================================================================
*/
static void IG_Map_SubMenu(void) {
	int depth;

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Restart map", ID_RESTART, NULL, IG_UseOldInGame_Event);
	DynamicMenu_AddItem("Next map", ID_NEXTMAP, NULL, IG_UseOldInGame_Event);

	depth = DynamicMenu_Depth();

	if (UI_ServerGametype() == GT_SINGLE_PLAYER) {
		DynamicMenu_SetFlags(depth, ID_NEXTMAP, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_CallVoteMisc_SubMenu
=======================================================================================================================================
*/
static void IG_CallVoteMisc_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Timelimit", 0, IG_TimeLimit_SubMenu, NULL);

	if (UI_ServerGametype() > GT_TEAM) {
		DynamicMenu_AddItem("CaptureLimit", 0, IG_CaptureLimit_SubMenu, NULL);
	} else {
		DynamicMenu_AddItem("FragLimit", 0, IG_FragLimit_SubMenu, NULL);
	}

	DynamicMenu_AddItem("Unlagged", 0, IG_Unlagged_SubMenu, NULL);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_CallVoteMaps_SubMenu
=======================================================================================================================================
*/
static void IG_CallVoteMaps_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Select map", CVM_SELECTMAP, NULL, IG_CallVoteMaps_Event);
	DynamicMenu_AddItem("Next map", CVM_NEXTMAP, NULL, IG_CallVoteMaps_Event);
	DynamicMenu_AddItem("Map restart", CVM_MAPRESTART, NULL, IG_CallVoteMaps_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_CallVoteMisc_SubMenu
=======================================================================================================================================
*/
static void IG_CallVoteGameType_SubMenu(void) {
	int gametype;
	int i;
	int depth;
	const char *icon;

	gametype = UI_ServerGametype();

	DynamicMenu_SubMenuInit();

	depth = DynamicMenu_Depth();

	for (i = 0; i < gametypeMenu_size; i++) {
		icon = UI_DefaultIconFromGameType(gametypeMenu_data[i].gametype);

		DynamicMenu_AddIconItem(gametypeMenu_data[i].menu, gametypeMenu_data[i].gametype, icon, NULL, IG_CallVoteGameType_Event);

		if (gametypeMenu_data[i].gametype == gametype) {
			DynamicMenu_SetFlags(depth, gametype, QMF_GRAYED);
		}
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_CallVote_SubMenu
=======================================================================================================================================
*/
static void IG_CallVote_SubMenu(void) {
	int team;
	int depth;

	team = UI_CurrentPlayerTeam();

	DynamicMenu_SubMenuInit();

	depth = DynamicMenu_Depth();

	DynamicMenu_AddItem("Gametype", IGCV_GAMETYPE, IG_CallVoteGameType_SubMenu, NULL);
	DynamicMenu_AddItem("Maps", 0, IG_CallVoteMaps_SubMenu, NULL);
	DynamicMenu_AddItem("Misc", 0, IG_CallVoteMisc_SubMenu, NULL);
	// DynamicMenu_AddItem("Map", IGCV_MAP, NULL, IG_CallVote_Event);
	// DynamicMenu_AddItem("Kick", IGCV_KICK, NULL, IG_CallVote_Event);
	DynamicMenu_AddItem("Team Balance", IGCV_TEAMBALANCE, NULL, IG_CallVote_Event);
	DynamicMenu_AddItem("Leader", IGCV_LEADER, NULL, IG_CallVote_Event);

	if (team == TEAM_SPECTATOR || team == TEAM_FREE) {
		DynamicMenu_SetFlags(depth, IGCV_LEADER, QMF_GRAYED);
		DynamicMenu_SetFlags(depth, IGCV_TEAMBALANCE, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Vote_SubMenu
=======================================================================================================================================
*/
static void IG_Vote_SubMenu(void) {
	char buf[MAX_INFO_STRING];

	DynamicMenu_SubMenuInit();

	trap_GetConfigString(CS_VOTE_TIME, buf, MAX_INFO_STRING);

	if (atoi(buf) != 0) {
		DynamicMenu_AddItem("Yes", IGV_YES, NULL, IG_Vote_Event);
		DynamicMenu_AddItem("No", IGV_NO, NULL, IG_Vote_Event);
	}

	trap_GetConfigString(CS_TEAMVOTE_TIME, buf, MAX_INFO_STRING);

	if (atoi(buf) != 0) {
		DynamicMenu_AddItem("Team Yes", IGV_TEAMYES, NULL, IG_Vote_Event);
		DynamicMenu_AddItem("Team No", IGV_TEAMNO, NULL, IG_Vote_Event);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Demos_SubMenu
=======================================================================================================================================
*/
static void IG_Demos_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddIconItem("Record ", IGD_RECORD, "menu/ui_art/rotate_record1", NULL, IG_Demos_Event);
	DynamicMenu_AddIconItem("Stop", IGD_STOP, "menu/ui_art/rotate_stop1", NULL, IG_Demos_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_AddBot_SubMenu
=======================================================================================================================================
*/
static void IG_AddBot_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Add bot", ID_ADDBOTS, NULL, IG_UseOldInGame_Event);
	DynamicMenu_AddItem("Remove bot", ID_REMOVEBOTS, NULL, IG_UseOldInGame_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Setup_SubMenu
=======================================================================================================================================
*/
static void IG_Setup_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Player", IGS_PLAYER, NULL, IG_Setup_Event);
	DynamicMenu_AddItem("Model", IGS_MODEL, NULL, IG_Setup_Event);
	DynamicMenu_AddItem("Controls", IGS_CONTROLS, NULL, IG_Setup_Event);
	DynamicMenu_AddItem("System", IGS_OPTIONS, NULL, IG_Setup_Event);
	DynamicMenu_AddItem("Load/Save", IGS_LOADSAVE, NULL, IG_Setup_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Exit_SubMenu
=======================================================================================================================================
*/
static void IG_Exit_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Main Menu", ID_LEAVEARENA, NULL, IG_UseOldInGame_Event);
	DynamicMenu_AddItem("Quit", ID_QUIT, NULL, IG_UseOldInGame_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
IG_Start_SubMenu
=======================================================================================================================================
*/
static void IG_Start_SubMenu(void) {
	int depth;
	int gametype;

	gametype = UI_ServerGametype();

	DynamicMenu_SubMenuInit();

	depth = DynamicMenu_Depth();

	if (gametype < GT_TEAM) {
		DynamicMenu_AddIconItem("Join Game", DM_START_GAME, "menu/medals/medal_gauntlet", NULL, IG_Start_Event);
	} else {
		DynamicMenu_AddIconItem("Auto Join", DM_START_AUTO, "menu/medals/medal_capture", NULL, IG_Start_Event);
		DynamicMenu_AddIconItem("Join Red", DM_START_RED, "ui_icons/iconf_red", NULL, IG_Start_Event);
		DynamicMenu_AddIconItem("Join Blue", DM_START_BLUE, "ui_icons/iconf_blu", NULL, IG_Start_Event);
	}

	DynamicMenu_AddItem("Spectate", DM_START_SPECTATOR, NULL, IG_Start_Event);

	if (UI_CurrentPlayerTeam() == TEAM_SPECTATOR) {
		DynamicMenu_SetFlags(depth, DM_START_SPECTATOR, QMF_GRAYED);
		DynamicMenu_SetFlags(depth, DM_READY, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
InGameDynamic_InitPrimaryMenu
=======================================================================================================================================
*/
static void InGameDynamic_InitPrimaryMenu(void) {
	int depth;
	int gametype;
	int team;
	qboolean localserver;
	qboolean voting;
	char buf[MAX_INFO_STRING];

	team = UI_CurrentPlayerTeam();
	gametype = UI_ServerGametype();
	localserver = trap_Cvar_VariableValue("sv_running");

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Start", IGM_START, IG_Start_SubMenu, NULL);

	if (gametype != GT_SINGLE_PLAYER) {
		DynamicMenu_AddItem("Call Vote", IGM_CALLVOTE, IG_CallVote_SubMenu, NULL);
		DynamicMenu_AddItem("Vote", IGM_VOTE, IG_Vote_SubMenu, NULL);
	}

	DynamicMenu_AddItem("Team Orders", IGM_TEAMORDERS, NULL, IG_TeamOrders_Event);
	DynamicMenu_AddItem("Setup", IGM_SETUP, IG_Setup_SubMenu, NULL);
	// disable map commands if non-local server
	if (localserver) {
		DynamicMenu_AddItem("Map", IGM_MAP, IG_Map_SubMenu, NULL);
	}
	// bot manipulation
	if (!(!localserver || !trap_Cvar_VariableValue("bot_enable") || (gametype == GT_SINGLE_PLAYER))) {
		DynamicMenu_AddItem("Bots", IGM_BOTS, IG_AddBot_SubMenu, NULL);
	}

	DynamicMenu_AddItem("Demos", IGM_DEMOS, IG_Demos_SubMenu, NULL);
	DynamicMenu_AddItem("Exit!", IGM_EXIT, IG_Exit_SubMenu, NULL);

	depth = DynamicMenu_Depth();
	gametype = trap_Cvar_VariableValue("g_gametype");

	if (gametype < GT_TEAM || team == TEAM_SPECTATOR) {
		DynamicMenu_SetFlags(depth, IGM_TEAMORDERS, QMF_GRAYED);
	}
	// spec protects voting menu (otherwise it could be used to cheat)
	if (team == TEAM_SPECTATOR) {
		DynamicMenu_SetFlags(depth, IGM_CALLVOTE, QMF_GRAYED);
		DynamicMenu_SetFlags(depth, IGM_VOTE, QMF_GRAYED);
	}
	// disable vote menu if no voting
	voting = qfalse;
	trap_GetConfigString(CS_VOTE_TIME, buf, MAX_INFO_STRING);

	if (atoi(buf) != 0) {
		voting = qtrue;
	}

	trap_GetConfigString(CS_TEAMVOTE_TIME, buf, MAX_INFO_STRING);

	if (atoi(buf) != 0) {
		voting = qtrue;
	}

	if (!voting) {
		DynamicMenu_SetFlags(depth, IGM_VOTE, QMF_GRAYED);
	}

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
UI_InGameDynamic
=======================================================================================================================================
*/
void UI_InGameDynamic(void) {

	DynamicMenu_MenuInit(qfalse, qtrue);
	InGameDynamic_InitPrimaryMenu();
}
// Tobias: TODO Entscheidung ob Dynamic oder nicht
/*
=======================================================================================================================================
UI_InGameMenu
=======================================================================================================================================
*//*
void UI_InGameMenu(void) {

	if (ui_ingame_dynamicmenu.integer) {
		UI_InGameDynamic();
	} else {
		// force as top level menu
		uis.menusp = 0;
		// set menu cursor to a nice location
		uis.cursorx = 319;
		uis.cursory = 80;

		InGame_MenuInit();
		UI_PushMenu(&s_ingame.menu);
	}
}
*/
/*
=======================================================================================================================================

	INGAME DYNAMIC BOT COMMAND MENU

=======================================================================================================================================
*/

// stores current gametype for fast access by menus
static int botcommandmenu_gametype = 0;

enum {
	COM_WHOLEADER,
	COM_IAMLEADER,
	COM_QUITLEADER,
	COM_MYTASK
} commandId;

static char *commandString[] = {
	"Who is the leader",		// COM_WHOLEADER
	"I am the leader",			// COM_IAMLEADER
	"I quit being the leader",	// COM_QUITLEADER
	"What is my job",			// COM_MYTASK
	0
};

enum {
	BC_NULL,
	BC_FOLLOW,
	BC_HELP,
	BC_GET,
	BC_PATROL,
	BC_CAMP,
	BC_HUNT,
	BC_DISMISS,
	BC_REPORT,
	BC_POINT,
	BC_GETFLAG,
	BC_DEFENDBASE
} botCommandId;

static char *botCommandStrings[] = {
	"",							// BC_NULL
	"%s follow %s",				// BC_FOLLOW
	"%s help %s",				// BC_HELP
	"%s get %s",				// BC_GET
	"%s patrol from %s to %s",	// BC_PATROL
	"%s camp %s",				// BC_CAMP
	"%s kill %s",				// BC_HUNT
	"%s dismissed",				// BC_DISMISS
	"%s report",				// BC_REPORT
	"%s lead the way",			// BC_POINT
	"%s get the flag",			// BC_GETFLAG
	"%s defend the base",		// BC_DEFENDBASE
	0
};

/*
=======================================================================================================================================
BotCommand_MenuClose
=======================================================================================================================================
*/
void BotCommand_MenuClose(void) {
// Tobias: TODO
/*
	if (ui_autoclosebotmenu.integer) {
		UI_PopMenu();
	}
*/
}

/*
=======================================================================================================================================
DM_BotPlayerTarget_Event

Issues a command to a bot that needs a target. Assumes index is the object, parent is the command, and parent of parent is the bot.
=======================================================================================================================================
*/
static void DM_BotPlayerTarget_Event(int index) {
	int depth;
	int bot, cmd;
	const char *s;

	depth = DynamicMenu_DepthOfIndex(index);

	if (depth != DynamicMenu_Depth() || depth < 3) {
		Com_Printf("BotPlayerTarget_Event: index %i at wrong depth (%i)\n", index, depth);
		BotCommand_MenuClose();
		return;
	}
	// validate command
	cmd = DynamicMenu_ActiveIdAtDepth(depth - 1);

	switch (cmd) {
		case BC_FOLLOW:
		case BC_HELP:
		case BC_HUNT:
			break;
		default:
			Com_Printf("BotPlayerTarget_Event: unknown command id %i\n", cmd);
			BotCommand_MenuClose();
			return;
	}
	// get the parent bot, insert it and item into command string
	bot = DynamicMenu_ActiveIndexAtDepth(depth - 2);
	s = va(botCommandStrings[cmd], DynamicMenu_StringAtIndex(bot), DynamicMenu_StringAtIndex(index));
	// issue the command
	BotCommand_MenuClose();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("say_team \"%s\"\n", s));
}

/*
=======================================================================================================================================
DM_BotItemItemTarget_Event

Issues a command to a bot that needs two targets. Assumes index and parent are the objects, grandparent is the command, and great -
grandparent is the bot.
=======================================================================================================================================
*/
static void DM_BotItemItemTarget_Event(int index) {
	int depth;
	int bot, cmd, item, item2;
	const char *s;

	depth = DynamicMenu_DepthOfIndex(index);

	if (depth != DynamicMenu_Depth() || depth < 4) {
		Com_Printf("BotItemItemTarget_Event: index %i at wrong depth (%i)\n", index, depth);
		BotCommand_MenuClose();
		return;
	}
	// validate command
	cmd = DynamicMenu_ActiveIdAtDepth(depth - 2);

	switch (cmd) {
		case BC_PATROL:
			break;
		default:
			Com_Printf("BotItemItemTarget_Event: unknown command id %i\n", cmd);
			BotCommand_MenuClose();
			return;
	}
	// get the parent bot, insert it and item into command string
	bot = DynamicMenu_ActiveIndexAtDepth(depth - 3);
	item = DynamicMenu_ActiveIdAtDepth(depth - 1);
	item2 = DynamicMenu_IdAtIndex(index);
	s = va(botCommandStrings[cmd], DynamicMenu_StringAtIndex(bot), DynamicMenu_ItemShortname(item), DynamicMenu_ItemShortname(item2));
	// issue the command
	BotCommand_MenuClose();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("say_team \"%s\"\n", s));
}

/*
=======================================================================================================================================
DM_BotItemTarget_Event

Issues a command to a bot that needs a target. Assumes index is the object, parent is the command, and parent of parent is the bot.
=======================================================================================================================================
*/
static void DM_BotItemTarget_Event(int index) {
	int depth;
	int bot, cmd, item;
	const char *s;
	const char *item_str;

	depth = DynamicMenu_DepthOfIndex(index);

	if (depth != DynamicMenu_Depth() || depth < 3) {
		Com_Printf("BotItemTarget_Event: index %i at wrong depth (%i)\n", index, depth);
		BotCommand_MenuClose();
		return;
	}
	// validate command
	cmd = DynamicMenu_ActiveIdAtDepth(depth - 1);

	switch (cmd) {
		case BC_GET:
		case BC_CAMP:
			break;
		default:
			Com_Printf("BotItemTarget_Event: unknown command id %i\n", cmd);
			BotCommand_MenuClose();
			return;
	}
	// get the parent bot, insert it and item into command string
	bot = DynamicMenu_ActiveIndexAtDepth(depth - 2);
	item = DynamicMenu_IdAtIndex(index);

	if (item < 0) {
		item_str = DynamicMenu_StringAtIndex(index);
	} else {
		item_str = DynamicMenu_ItemShortname(item);
	}

	s = va(botCommandStrings[cmd], DynamicMenu_StringAtIndex(bot), item_str);
	// issue the command
	BotCommand_MenuClose();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("say_team \"%s\"\n", s));
}

/*
=======================================================================================================================================
DM_BotCommand_Event

Issues a command to a bot
=======================================================================================================================================
*/
static void DM_BotCommand_Event(int index) {
	int depth;
	int bot, cmd;
	const char *s;

	depth = DynamicMenu_DepthOfIndex(index);

	if (depth != DynamicMenu_Depth() || depth < 2) {
		Com_Printf("BotCommand_Event: index %i at wrong depth (%i)\n", index, depth);
		BotCommand_MenuClose();
		return;
	}
	// validate command
	cmd = DynamicMenu_IdAtIndex(index);

	switch (cmd) {
		case BC_DISMISS:
		case BC_REPORT:
		case BC_POINT:
		case BC_GETFLAG:
		case BC_DEFENDBASE:
			break;
		default:
			Com_Printf("BotCommand_Event: unknown command (%i)\n", cmd);
			BotCommand_MenuClose();
			return;
	}
	// get the parent bot name, insert into command string
	bot = DynamicMenu_ActiveIndexAtDepth(depth - 1);
	s = va(botCommandStrings[cmd], DynamicMenu_StringAtIndex(bot));
	// issue the command
	BotCommand_MenuClose();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("say_team \"%s\"\n", s));
}

/*
=======================================================================================================================================
DM_Command_Event

Issues a command without target
=======================================================================================================================================
*/
static void DM_Command_Event(int index) {
	int depth;
	int cmd;

	depth = DynamicMenu_DepthOfIndex(index);

	if (depth != DynamicMenu_Depth()) {
		Com_Printf("Command_Event: index %i at wrong depth (%i)\n", index, depth);
		BotCommand_MenuClose();
		return;
	}
	// validate command
	cmd = DynamicMenu_IdAtIndex(index);

	switch (cmd) {
		case COM_WHOLEADER:
		case COM_IAMLEADER:
		case COM_QUITLEADER:
		case COM_MYTASK:
			break;
		default:
			Com_Printf("Command_Event: unknown command (%i)\n", cmd);
			BotCommand_MenuClose();
			return;
	}
	// issue the command
	BotCommand_MenuClose();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("say_team \"%s\"\n", commandString[cmd]));
}

/*
=======================================================================================================================================
DM_Close_Event
=======================================================================================================================================
*/
static void DM_Close_Event(int index) {
	UI_PopMenu();
}

/*
=======================================================================================================================================
DM_TeamList_SubMenu
=======================================================================================================================================
*/
static void DM_TeamList_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("me", 0, NULL, DM_BotPlayerTarget_Event);
	DynamicMenu_AddListOfPlayers(PT_FRIENDLY|PT_EXCLUDEGRANDPARENT, NULL, DM_BotPlayerTarget_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_ItemPatrol2_SubMenu
=======================================================================================================================================
*/
static void DM_ItemPatrol2_SubMenu(void) {
	int exclude, depth;

	DynamicMenu_SubMenuInit();

	depth = DynamicMenu_Depth() - 1;
	exclude = DynamicMenu_ActiveIdAtDepth(depth);
//	index = s_dynamic.active[depth - 1]; // previous menu level
//	exclude = s_dynamic.data[index].id;
	DynamicMenu_AddListOfItems(exclude, NULL, DM_BotItemItemTarget_Event);
	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_ItemPatrol_SubMenu
=======================================================================================================================================
*/
static void DM_ItemPatrol_SubMenu(void) {

	DynamicMenu_SubMenuInit();
	DynamicMenu_AddListOfItems(-1, DM_ItemPatrol2_SubMenu, NULL);
	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_CampItemList_SubMenu
=======================================================================================================================================
*/
static void DM_CampItemList_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("here", -1, NULL, DM_BotItemTarget_Event);
	DynamicMenu_AddItem("there", -1, NULL, DM_BotItemTarget_Event);

	DynamicMenu_AddListOfItems(-1, NULL, DM_BotItemTarget_Event);
	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_ItemList_SubMenu
=======================================================================================================================================
*/
void DM_ItemList_SubMenu(void) {

	DynamicMenu_SubMenuInit();
	DynamicMenu_AddListOfItems(-1, NULL, DM_BotItemTarget_Event);
	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_EnemyList_SubMenu
=======================================================================================================================================
*/
static void DM_EnemyList_SubMenu(void) {

	DynamicMenu_SubMenuInit();
	DynamicMenu_AddListOfPlayers(PT_ENEMY, NULL, DM_BotPlayerTarget_Event);
	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
DM_CommandList_SubMenu
=======================================================================================================================================
*/
static void DM_CommandList_SubMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Report", BC_REPORT, (createHandler)NULL, DM_BotCommand_Event);
	DynamicMenu_AddItem("Help", BC_HELP, DM_TeamList_SubMenu, (eventHandler)NULL);

	if (botcommandmenu_gametype == GT_CTF) {
		DynamicMenu_AddItem("Capture Flag", BC_GETFLAG, (createHandler)NULL, DM_BotCommand_Event);
		DynamicMenu_AddItem("Defend Base", BC_DEFENDBASE, (createHandler)NULL, DM_BotCommand_Event);
	}

	DynamicMenu_AddItem("Follow", BC_FOLLOW, DM_TeamList_SubMenu, (eventHandler)NULL);
	DynamicMenu_AddItem("Get", BC_GET, DM_ItemList_SubMenu, (eventHandler)NULL);
	DynamicMenu_AddItem("Patrol", BC_PATROL, DM_ItemPatrol_SubMenu, (eventHandler)NULL);
	DynamicMenu_AddItem("Camp", BC_CAMP, DM_CampItemList_SubMenu, (eventHandler)NULL);
	DynamicMenu_AddItem("Hunt", BC_HUNT, DM_EnemyList_SubMenu, (eventHandler)NULL);
	DynamicMenu_AddItem("Point + ", BC_POINT, (createHandler)NULL, DM_BotCommand_Event);
	DynamicMenu_AddItem("Dismiss", BC_DISMISS, (createHandler)NULL, DM_BotCommand_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
BotCommand_InitPrimaryMenu
=======================================================================================================================================
*/
static void BotCommand_InitPrimaryMenu(void) {

	DynamicMenu_SubMenuInit();

	DynamicMenu_AddItem("Close!", 0, NULL, DM_Close_Event);
	DynamicMenu_AddItem("Everyone", 0, DM_CommandList_SubMenu, NULL);
	DynamicMenu_AddListOfPlayers(PT_FRIENDLY|PT_BOTONLY, DM_CommandList_SubMenu, NULL);
	DynamicMenu_AddItem("Leader?", COM_WHOLEADER, NULL, DM_Command_Event);

	if (botcommandmenu_gametype > GT_TEAM) {
		DynamicMenu_AddItem("My task?", COM_MYTASK, NULL, DM_Command_Event);
	}

	DynamicMenu_AddItem("Lead", COM_IAMLEADER, (createHandler)NULL, DM_Command_Event);
	DynamicMenu_AddItem("Resign", COM_QUITLEADER, (createHandler)NULL, DM_Command_Event);

	DynamicMenu_AddBackground(INGAME_FRAME);
	DynamicMenu_FinishSubMenuInit();
}

/*
=======================================================================================================================================
UI_BotCommand_Cache
=======================================================================================================================================
*/
void UI_BotCommand_Cache(void) {
}

/*
=======================================================================================================================================
UI_BotCommandMenu
=======================================================================================================================================
*/
void UI_BotCommandMenu(void) {

	if (UI_CurrentPlayerTeam() == TEAM_SPECTATOR) {
		return;
	}

	botcommandmenu_gametype = UI_ServerGametype();

	if (botcommandmenu_gametype < GT_TEAM) {
		return;
	}

	DynamicMenu_MenuInit(qfalse, qtrue);
	BotCommand_InitPrimaryMenu();
}

/*
=======================================================================================================================================
UI_BotCommandMenu_f
=======================================================================================================================================
*/
void UI_BotCommandMenu_f(void) {
	UI_BotCommandMenu();
}
