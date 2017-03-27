/*
=======================================================================================================================================
The work contained within this file is software written by various copyright holders. The initial contributor, Id Software holds all
copyright over their software. However, software used and written by and for UI Enhanced has copyrights held by the initial author of
the software.

The changes written by and for UI Enhanced are contained alongside the original work from Id Software for convenience and ease of
interoperability.

For the code contained herein that was written by Id Software, see the license agreement on their original archive for restrictions and
limitations.

The UI Enhanced copyright owner permit free reuse of his code contained herein, as long as the following terms are met:
---------------------------------------------------------------------------------------------------------------------------------------
1) Credit is given in a place where users of the mod may read it. (Title screen, credit screen or README will do). The recommended
   format is: "First, Last, alias, email"

2) There are no attempts to misrepresent the public as to who made the alterations. The UI Enhanced copyright owner does not give
   permission for others to release software under the UI Enhanced name.
---------------------------------------------------------------------------------------------------------------------------------------
Ian Jefferies - HypoThermia (uie@planetquake.com)
http://www.planetquake.com/uie

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
=======================================================================================================================================
*/

#include "ui_local.h"

#define VOTEMENU_BACK0 "menu/art/back_0"
#define VOTEMENU_BACK1 "menu/art/back_1"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"
#define ART_BACKGROUND "menu/art/addbotframe"

static char *callvotemenu_timelmit_artlist[] = {
	VOTEMENU_BACK0,
	VOTEMENU_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

enum {
	ID_BACK,
	ID_GO,
	ID_50,
	ID_10,
	ID_15,
	ID_20,
	ID_30,
	ID_40,
	ID_INF
};

#define TimeLIMIT_MENU_VERTICAL_SPACING 28

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s back;
	menubitmap_s go;
	// buttons
	menutext_s bLimit50;
	menutext_s bLimit10;
	menutext_s bLimit15;
	menutext_s bLimit20;
	menutext_s bLimit30;
	menutext_s bLimit40;
	menutext_s bLimitInf;
	int min, max; // values restricted by server
	int selection;
} callvotemenu_t;

static callvotemenu_t s_callvotemenu_timelmit;

void UI_VoteTimelimitMenuInternal(void);

/*
=======================================================================================================================================
VoteMenu_Timelimit_Event
=======================================================================================================================================
*/
static void VoteMenu_Timelimit_Event(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_BACK:
			if (event != QM_ACTIVATED) {
				break;
			}

			UI_PopMenu();
			break;
		case ID_10:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 10");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_15:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 15");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_20:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 20");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_30:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 30");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_40:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 40");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_50:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 50");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_INF:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote timelimit 0");
			UI_PopMenu();
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
SetTimeMenuText
=======================================================================================================================================
*/
static void SetTimeMenuText(menutext_s *menu, int y, int id, int value, char *text) {

	menu->generic.type = MTYPE_PTEXT;
	menu->color = color_red;
	menu->generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;

	if ((s_callvotemenu_timelmit.min > value && value != 0) || (s_callvotemenu_timelmit.max < value && s_callvotemenu_timelmit.max != 0) || (s_callvotemenu_timelmit.max != 0 && value == 0)) {
		menu->generic.flags |= QMF_INACTIVE|QMF_GRAYED;
	} else if (s_callvotemenu_timelmit.selection == id) {
		menu->color = color_orange;
	}

	menu->generic.x = 320;
	menu->generic.y = y;
	menu->generic.id = id;
	menu->generic.callback = VoteMenu_Timelimit_Event;
	menu->string = text;
	menu->style = UI_CENTER|UI_SMALLFONT;
}

/*
=======================================================================================================================================
VoteMenu_Timelimit_Cache
=======================================================================================================================================
*/
static void VoteMenu_Timelimit_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0;; i++) {
		if (!callvotemenu_timelmit_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(callvotemenu_timelmit_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_VoteMenu_Timelimit_Draw
=======================================================================================================================================
*/
static void UI_VoteMenu_Timelimit_Draw(void) {

	UI_DrawBannerString(320, 16, "CALL VOTE - TIMELIMIT", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_callvotemenu_timelmit.menu);
}

/*
=======================================================================================================================================
UI_VoteTimelimitMenuInternal
=======================================================================================================================================
*/
void UI_VoteTimelimitMenuInternal(void) {
	int y;

	VoteMenu_Timelimit_Cache();

	s_callvotemenu_timelmit.menu.wrapAround = qtrue;
	s_callvotemenu_timelmit.menu.fullscreen = qfalse;
	s_callvotemenu_timelmit.menu.draw = UI_VoteMenu_Timelimit_Draw;

	s_callvotemenu_timelmit.banner.generic.type = MTYPE_BTEXT;
	s_callvotemenu_timelmit.banner.generic.x = 320;
	s_callvotemenu_timelmit.banner.generic.y = 16;
	s_callvotemenu_timelmit.banner.string = "CALL VOTE - TIMELIMIT";
	s_callvotemenu_timelmit.banner.color = color_white;
	s_callvotemenu_timelmit.banner.style = UI_CENTER;

	y = 98;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit10, y, ID_10, 10, "10 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit15, y, ID_15, 15, "15 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit20, y, ID_20, 20, "20 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit30, y, ID_30, 30, "30 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit40, y, ID_40, 40, "40 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimit50, y, ID_50, 50, "50 minutes");
	y += TimeLIMIT_MENU_VERTICAL_SPACING;
	SetTimeMenuText(&s_callvotemenu_timelmit.bLimitInf, y, ID_INF, 0, "No limit");

	s_callvotemenu_timelmit.back.generic.type = MTYPE_BITMAP;
	s_callvotemenu_timelmit.back.generic.name = VOTEMENU_BACK0;
	s_callvotemenu_timelmit.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_timelmit.back.generic.callback = VoteMenu_Timelimit_Event;
	s_callvotemenu_timelmit.back.generic.id = ID_BACK;
	s_callvotemenu_timelmit.back.generic.x = 320 - 128;
	s_callvotemenu_timelmit.back.generic.y = 256 + 128 - 64;
	s_callvotemenu_timelmit.back.width = 128;
	s_callvotemenu_timelmit.back.height = 64;
	s_callvotemenu_timelmit.back.focuspic = VOTEMENU_BACK1;

	s_callvotemenu_timelmit.go.generic.type = MTYPE_BITMAP;
	s_callvotemenu_timelmit.go.generic.name = ART_FIGHT0;
	s_callvotemenu_timelmit.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_timelmit.go.generic.callback = VoteMenu_Timelimit_Event;
	s_callvotemenu_timelmit.go.generic.id = ID_GO;
	s_callvotemenu_timelmit.go.generic.x = 320;
	s_callvotemenu_timelmit.go.generic.y = 256 + 128 - 64;
	s_callvotemenu_timelmit.go.width = 128;
	s_callvotemenu_timelmit.go.height = 64;
	s_callvotemenu_timelmit.go.focuspic = ART_FIGHT1;
}

/*
=======================================================================================================================================
UI_VoteTimelimitMenu
=======================================================================================================================================
*/
void UI_VoteTimelimitMenu(void) {
	char serverinfo[MAX_INFO_STRING];

	// zero set all our globals
	memset(&s_callvotemenu_timelmit, 0, sizeof(callvotemenu_t));

	trap_GetConfigString(CS_SERVERINFO, serverinfo, MAX_INFO_STRING);

	s_callvotemenu_timelmit.min = atoi(Info_ValueForKey(serverinfo, "g_voteMinTimelimit"));
	s_callvotemenu_timelmit.max = atoi(Info_ValueForKey(serverinfo, "g_voteMaxTimelimit"));

	UI_VoteTimelimitMenuInternal();

	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.banner);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.back);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit10);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit15);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit20);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit30);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit40);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimit50);
	Menu_AddItem(&s_callvotemenu_timelmit.menu, (void *)&s_callvotemenu_timelmit.bLimitInf);

	UI_PushMenu(&s_callvotemenu_timelmit.menu);
}
