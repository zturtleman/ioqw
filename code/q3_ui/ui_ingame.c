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

#define INGAME_FRAME "menu/art/addbotframe"

enum {
	ID_TEAM,
	ID_ADDBOTS,
	ID_REMOVEBOTS,
	ID_TEAMORDERS,
	ID_SERVERINFO,
	ID_SETUP,
	ID_RESTART,
	ID_CREATE,
	ID_LEAVEARENA,
	ID_QUIT
};

static ingamemenu_t s_ingame;

/*
=======================================================================================================================================
InGame_QuitAction
=======================================================================================================================================
*/
static void InGame_QuitAction(qboolean result) {

	if (!result) {
		return;
	}

	UI_PopMenu();
	UI_CreditMenu();
}

/*
=======================================================================================================================================
InGame_LeaveAction
=======================================================================================================================================
*/
static void InGame_LeaveAction(qboolean result) {

	if (!result) {
		return;
	}

	UI_PopMenu();
	trap_Cmd_ExecuteText(EXEC_APPEND, "disconnect\n");
}

/*
=======================================================================================================================================
InGame_RestartAction
=======================================================================================================================================
*/
static void InGame_RestartAction(qboolean result) {

	if (!result) {
		return;
	}

	UI_PopMenu();
	trap_Cmd_ExecuteText(EXEC_APPEND, "map_restart 0\n");
}

/*
=======================================================================================================================================
InGame_Event
=======================================================================================================================================
*/
void InGame_Event(void *ptr, int notification) {

	if (notification != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_TEAM:
			UI_TeamMainMenu();
			break;
		case ID_ADDBOTS:
			UI_AddBotsMenu();
			break;
		case ID_REMOVEBOTS:
			UI_RemoveBotsMenu(RBM_KICKBOT);
			break;
		case ID_TEAMORDERS:
			UI_TeamOrdersMenu();
			break;
		case ID_SERVERINFO:
			UI_ServerInfoMenu();
			break;
		case ID_SETUP:
			UI_SetupMenu();
			break;
		case ID_RESTART:
			UI_ConfirmMenu("Restart Arena?", 0, InGame_RestartAction);
			break;
		case ID_CREATE:
			UI_StartServerMenu(qtrue); // Tobias: UI_StartServerIngameMenu
			break;
		case ID_LEAVEARENA:
			UI_ConfirmMenu("Leave Arena?", 0, InGame_LeaveAction);
			break;
		case ID_QUIT:
			UI_ConfirmMenu("Exit Game?", 0, InGame_QuitAction);
			break;
	}
}

/*
=======================================================================================================================================
InGame_MenuInit
=======================================================================================================================================
*/
void InGame_MenuInit(void) {
	int y;
	uiClientState_t cs;
	char info[MAX_INFO_STRING];
	int team;

	memset(&s_ingame, 0, sizeof(ingamemenu_t));

	InGame_Cache();

	s_ingame.menu.wrapAround = qtrue;
	s_ingame.menu.fullscreen = qfalse;

	s_ingame.frame.generic.type = MTYPE_BITMAP;
	s_ingame.frame.generic.flags = QMF_INACTIVE;
	s_ingame.frame.generic.name = INGAME_FRAME;
	s_ingame.frame.generic.x = 320 - 233; // 142;
	s_ingame.frame.generic.y = 240 - 166; // 118;
	s_ingame.frame.width = 466; // 359;
	s_ingame.frame.height = 332; // 256;

	y = 88; //y = 96;
	s_ingame.team.generic.type = MTYPE_PTEXT;
	s_ingame.team.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.team.generic.x = 320;
	s_ingame.team.generic.y = y;
	s_ingame.team.generic.id = ID_TEAM;
	s_ingame.team.generic.callback = InGame_Event;
	s_ingame.team.string = "START";
	s_ingame.team.color = color_red;
	s_ingame.team.style = UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.addbots.generic.type = MTYPE_PTEXT;
	s_ingame.addbots.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.addbots.generic.x = 320;
	s_ingame.addbots.generic.y = y;
	s_ingame.addbots.generic.id = ID_ADDBOTS;
	s_ingame.addbots.generic.callback = InGame_Event;
	s_ingame.addbots.string = "ADD BOTS";
	s_ingame.addbots.color = color_red;
	s_ingame.addbots.style = UI_CENTER|UI_SMALLFONT;

	if (!trap_Cvar_VariableValue("sv_running") || !trap_Cvar_VariableValue("bot_enable") || (trap_Cvar_VariableValue("g_gametype") == GT_SINGLE_PLAYER)) {
		s_ingame.addbots.generic.flags |= QMF_GRAYED;
	}

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.removebots.generic.type = MTYPE_PTEXT;
	s_ingame.removebots.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.removebots.generic.x = 320;
	s_ingame.removebots.generic.y = y;
	s_ingame.removebots.generic.id = ID_REMOVEBOTS;
	s_ingame.removebots.generic.callback = InGame_Event;
	s_ingame.removebots.string = "REMOVE BOTS";
	s_ingame.removebots.color = color_red;
	s_ingame.removebots.style = UI_CENTER|UI_SMALLFONT;

	if (!trap_Cvar_VariableValue("sv_running") || !trap_Cvar_VariableValue("bot_enable") || (trap_Cvar_VariableValue("g_gametype") == GT_SINGLE_PLAYER)) {
		s_ingame.removebots.generic.flags |= QMF_GRAYED;
	}

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.teamorders.generic.type = MTYPE_PTEXT;
	s_ingame.teamorders.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.teamorders.generic.x = 320;
	s_ingame.teamorders.generic.y = y;
	s_ingame.teamorders.generic.id = ID_TEAMORDERS;
	s_ingame.teamorders.generic.callback = InGame_Event;
	s_ingame.teamorders.string = "TEAM ORDERS";
	s_ingame.teamorders.color = color_red;
	s_ingame.teamorders.style = UI_CENTER|UI_SMALLFONT;

	if (!(trap_Cvar_VariableValue("g_gametype") > GT_TOURNAMENT)) {
		s_ingame.teamorders.generic.flags |= QMF_GRAYED;
	} else {
		trap_GetClientState(&cs);
		trap_GetConfigString(CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING);

		team = atoi(Info_ValueForKey(info, "t"));

		if (team == TEAM_SPECTATOR) {
			s_ingame.teamorders.generic.flags |= QMF_GRAYED;
		}
	}

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.server.generic.type = MTYPE_PTEXT;
	s_ingame.server.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.server.generic.x = 320;
	s_ingame.server.generic.y = y;
	s_ingame.server.generic.id = ID_SERVERINFO;
	s_ingame.server.generic.callback = InGame_Event;
	s_ingame.server.string = "SERVER INFO";
	s_ingame.server.color = color_red;
	s_ingame.server.style = UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.setup.generic.type = MTYPE_PTEXT;
	s_ingame.setup.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.setup.generic.x = 320;
	s_ingame.setup.generic.y = y;
	s_ingame.setup.generic.id = ID_SETUP;
	s_ingame.setup.generic.callback = InGame_Event;
	s_ingame.setup.string = "SETUP";
	s_ingame.setup.color = color_red;
	s_ingame.setup.style = UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.restart.generic.type = MTYPE_PTEXT;
	s_ingame.restart.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.restart.generic.x = 320;
	s_ingame.restart.generic.y = y;
	s_ingame.restart.generic.id = ID_RESTART;
	s_ingame.restart.generic.callback = InGame_Event;
	s_ingame.restart.string = "RESTART ARENA";
	s_ingame.restart.color = color_red;
	s_ingame.restart.style = UI_CENTER|UI_SMALLFONT;

	if (!trap_Cvar_VariableValue("sv_running")) {
		s_ingame.restart.generic.flags |= QMF_GRAYED;
	}

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.startnew.generic.type = MTYPE_PTEXT;
	s_ingame.startnew.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.startnew.generic.x = 320;
	s_ingame.startnew.generic.y = y;
	s_ingame.startnew.generic.id = ID_CREATE;
	s_ingame.startnew.generic.callback = InGame_Event;
	s_ingame.startnew.string = "START NEW ARENA";
	s_ingame.startnew.color = color_red;
	s_ingame.startnew.style = UI_CENTER|UI_SMALLFONT;

	if (!trap_Cvar_VariableValue("sv_running") || (trap_Cvar_VariableValue("g_gametype") == GT_SINGLE_PLAYER)) {
		s_ingame.startnew.generic.flags |= QMF_GRAYED;
	}

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.leave.generic.type = MTYPE_PTEXT;
	s_ingame.leave.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.leave.generic.x = 320;
	s_ingame.leave.generic.y = y;
	s_ingame.leave.generic.id = ID_LEAVEARENA;
	s_ingame.leave.generic.callback = InGame_Event;
	s_ingame.leave.string = "LEAVE ARENA";
	s_ingame.leave.color = color_red;
	s_ingame.leave.style = UI_CENTER|UI_SMALLFONT;

	y += INGAME_MENU_VERTICAL_SPACING;
	s_ingame.quit.generic.type = MTYPE_PTEXT;
	s_ingame.quit.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_ingame.quit.generic.x = 320;
	s_ingame.quit.generic.y = y;
	s_ingame.quit.generic.id = ID_QUIT;
	s_ingame.quit.generic.callback = InGame_Event;
	s_ingame.quit.string = "EXIT GAME";
	s_ingame.quit.color = color_red;
	s_ingame.quit.style = UI_CENTER|UI_SMALLFONT;

	Menu_AddItem(&s_ingame.menu, &s_ingame.frame);
	Menu_AddItem(&s_ingame.menu, &s_ingame.team);
	Menu_AddItem(&s_ingame.menu, &s_ingame.addbots);
	Menu_AddItem(&s_ingame.menu, &s_ingame.removebots);
	Menu_AddItem(&s_ingame.menu, &s_ingame.teamorders);
	Menu_AddItem(&s_ingame.menu, &s_ingame.server);
	Menu_AddItem(&s_ingame.menu, &s_ingame.setup);
	Menu_AddItem(&s_ingame.menu, &s_ingame.restart);
	Menu_AddItem(&s_ingame.menu, &s_ingame.startnew);
	Menu_AddItem(&s_ingame.menu, &s_ingame.leave);
	Menu_AddItem(&s_ingame.menu, &s_ingame.quit);
}

/*
=======================================================================================================================================
InGame_Cache
=======================================================================================================================================
*/
void InGame_Cache(void) {
	trap_R_RegisterShaderNoMip(INGAME_FRAME);
}

/*
=======================================================================================================================================
UI_InGameMenu
=======================================================================================================================================
*/
void UI_InGameMenu(void) {

	// force as top level menu
	uis.menusp = 0;
	// set menu cursor to a nice location
	uis.cursorx = 319;
	uis.cursory = 80;

	InGame_MenuInit();
	UI_PushMenu(&s_ingame.menu);
}
