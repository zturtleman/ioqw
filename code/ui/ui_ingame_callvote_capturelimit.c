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

static char *callvotemenu_capturelimit_artlist[] = {
	VOTEMENU_BACK0,
	VOTEMENU_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

enum {
	ID_BACK,
	ID_GO,
	ID_25,
	ID_1,
	ID_5,
	ID_10,
	ID_15,
	ID_20,
	ID_INF
};

#define CAPTURELIMIT_MENU_VERTICAL_SPACING 28

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s back;
	menubitmap_s go;
	// buttons
	menutext_s bLimit25;
	menutext_s bLimit1;
	menutext_s bLimit5;
	menutext_s bLimit10;
	menutext_s bLimit15;
	menutext_s bLimit20;
	menutext_s bLimitInf;
	int min, max; // values restricted by server
	int selection;
} callvotemenu_t;

static callvotemenu_t s_callvotemenu_capturelimit;

void UI_VoteCapturelimitMenuInternal(void);

/*
=======================================================================================================================================
VoteMenu_Capturelimit_Event
=======================================================================================================================================
*/
static void VoteMenu_Capturelimit_Event(void *ptr, int event) {

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
		case ID_1:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 1");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_5:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 5");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_10:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 10");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_15:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 15");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_20:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 20");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_25:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 25");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_INF:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote capturelimit 0");
			UI_PopMenu();
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
SetCapturelimitMenuText
=======================================================================================================================================
*/
static void SetCapturelimitMenuText(menutext_s *menu, int y, int id, int value, char *text) {

	menu->generic.type = MTYPE_PTEXT;
	menu->color = color_red;
	menu->generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;

	if ((s_callvotemenu_capturelimit.min > value && value != 0) || (s_callvotemenu_capturelimit.max < value && s_callvotemenu_capturelimit.max != 0) || (s_callvotemenu_capturelimit.max != 0 && value == 0)) {
		menu->generic.flags |= QMF_INACTIVE|QMF_GRAYED;
	} else if (s_callvotemenu_capturelimit.selection == id) {
		menu->color = color_orange;
	}

	menu->generic.x = 320;
	menu->generic.y = y;
	menu->generic.id = id;
	menu->generic.callback = VoteMenu_Capturelimit_Event;
	menu->string = text;
	menu->style = UI_CENTER|UI_SMALLFONT;
}

/*
=======================================================================================================================================
VoteMenu_Capturelimit_Cache
=======================================================================================================================================
*/
static void VoteMenu_Capturelimit_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0;; i++) {
		if (!callvotemenu_capturelimit_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(callvotemenu_capturelimit_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_VoteMenu_Capturelimit_Draw
=======================================================================================================================================
*/
static void UI_VoteMenu_Capturelimit_Draw(void) {

	UI_DrawBannerString(320, 16, "CALL VOTE - CAPTURELIMIT", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_callvotemenu_capturelimit.menu);
}

/*
=======================================================================================================================================
UI_VoteCapturelimitMenuInternal
=======================================================================================================================================
*/
void UI_VoteCapturelimitMenuInternal(void) {
	int y;

	VoteMenu_Capturelimit_Cache();

	s_callvotemenu_capturelimit.menu.wrapAround = qtrue;
	s_callvotemenu_capturelimit.menu.fullscreen = qfalse;
	s_callvotemenu_capturelimit.menu.draw = UI_VoteMenu_Capturelimit_Draw;

	s_callvotemenu_capturelimit.banner.generic.type = MTYPE_BTEXT;
	s_callvotemenu_capturelimit.banner.generic.x = 320;
	s_callvotemenu_capturelimit.banner.generic.y = 16;
	s_callvotemenu_capturelimit.banner.string = "CALL VOTE - CAPTURELIMIT";
	s_callvotemenu_capturelimit.banner.color = color_white;
	s_callvotemenu_capturelimit.banner.style = UI_CENTER;

	y = 98;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit1, y, ID_1, 1, "1");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit5, y, ID_5, 5, "5");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit10, y, ID_10, 10, "10");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit15, y, ID_15, 15, "15");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit20, y, ID_20, 20, "20");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimit25, y, ID_25, 25, "25");
	y += CAPTURELIMIT_MENU_VERTICAL_SPACING;
	SetCapturelimitMenuText(&s_callvotemenu_capturelimit.bLimitInf, y, ID_INF, 0, "No limit");

	s_callvotemenu_capturelimit.back.generic.type = MTYPE_BITMAP;
	s_callvotemenu_capturelimit.back.generic.name = VOTEMENU_BACK0;
	s_callvotemenu_capturelimit.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_capturelimit.back.generic.callback = VoteMenu_Capturelimit_Event;
	s_callvotemenu_capturelimit.back.generic.id = ID_BACK;
	s_callvotemenu_capturelimit.back.generic.x = 320 - 128;
	s_callvotemenu_capturelimit.back.generic.y = 256 + 128 - 64;
	s_callvotemenu_capturelimit.back.width = 128;
	s_callvotemenu_capturelimit.back.height = 64;
	s_callvotemenu_capturelimit.back.focuspic = VOTEMENU_BACK1;

	s_callvotemenu_capturelimit.go.generic.type = MTYPE_BITMAP;
	s_callvotemenu_capturelimit.go.generic.name = ART_FIGHT0;
	s_callvotemenu_capturelimit.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_capturelimit.go.generic.callback = VoteMenu_Capturelimit_Event;
	s_callvotemenu_capturelimit.go.generic.id = ID_GO;
	s_callvotemenu_capturelimit.go.generic.x = 320;
	s_callvotemenu_capturelimit.go.generic.y = 256 + 128 - 64;
	s_callvotemenu_capturelimit.go.width = 128;
	s_callvotemenu_capturelimit.go.height = 64;
	s_callvotemenu_capturelimit.go.focuspic = ART_FIGHT1;
}

/*
=======================================================================================================================================
UI_VoteCapturelimitMenu
=======================================================================================================================================
*/
void UI_VoteCapturelimitMenu(void) {
	char serverinfo[MAX_INFO_STRING];

	// zero set all our globals
	memset(&s_callvotemenu_capturelimit, 0, sizeof(callvotemenu_t));

	trap_GetConfigString(CS_SERVERINFO, serverinfo, MAX_INFO_STRING);

	s_callvotemenu_capturelimit.min = atoi(Info_ValueForKey(serverinfo, "g_voteMinCapturelimit"));
	s_callvotemenu_capturelimit.max = atoi(Info_ValueForKey(serverinfo, "g_voteMaxCapturelimit"));

	UI_VoteCapturelimitMenuInternal();

	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.banner);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.back);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit1);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit5);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit10);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit15);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit20);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimit25);
	Menu_AddItem(&s_callvotemenu_capturelimit.menu, (void *)&s_callvotemenu_capturelimit.bLimitInf);

	UI_PushMenu(&s_callvotemenu_capturelimit.menu);
}
