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

static char *votemenu_artlist[] = {
	VOTEMENU_BACK0,
	VOTEMENU_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

enum {
	ID_BACK,
	ID_GO,
	ID_YES,
	ID_NO
};
// this sorta dependend on number of vote options
#define VOTEMENU_MENU_VERTICAL_SPACING 22

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s back;
	menubitmap_s go;
	menutext_s bYes;
	menutext_s bNo;
} votemenu_t;

static votemenu_t s_votemenu;

void UI_VoteMenuInternal(void);

/*
=======================================================================================================================================
VoteMenu_Event
=======================================================================================================================================
*/
static void VoteMenu_Event(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_BACK:
			if (event != QM_ACTIVATED) {
				return;
			}

			UI_PopMenu();
			break;
		case ID_YES:
			trap_Cmd_ExecuteText(EXEC_APPEND, "vote yes");
			UI_PopMenu();
			break;
		case ID_NO:
			trap_Cmd_ExecuteText(EXEC_APPEND, "vote no");
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
VoteMenu_Cache
=======================================================================================================================================
*/
static void VoteMenu_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0; ; i++) {
		if (!votemenu_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(votemenu_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_VoteMenu_Draw
=======================================================================================================================================
*/
static void UI_VoteMenu_Draw(void) {

	UI_DrawBannerString(320, 16, "VOTE", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_votemenu.menu);
}

/*
=======================================================================================================================================
UI_VoteMenuInternal
=======================================================================================================================================
*/
void UI_VoteMenuInternal(void) {
	int y;

	VoteMenu_Cache();

	s_votemenu.menu.wrapAround = qtrue;
	s_votemenu.menu.fullscreen = qfalse;
	s_votemenu.menu.draw = UI_VoteMenu_Draw;

	s_votemenu.banner.generic.type = MTYPE_BTEXT;
	s_votemenu.banner.generic.x = 320;
	s_votemenu.banner.generic.y = 16;
	s_votemenu.banner.string = "VOTE";
	s_votemenu.banner.color = color_white;
	s_votemenu.banner.style = UI_CENTER;

	y = 98;
	s_votemenu.bYes.generic.type = MTYPE_PTEXT;
	s_votemenu.bYes.color = color_red;
	s_votemenu.bYes.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_votemenu.bYes.generic.x = 320;
	s_votemenu.bYes.generic.y = y;
	s_votemenu.bYes.generic.id = ID_YES;
	s_votemenu.bYes.generic.callback = VoteMenu_Event;
	s_votemenu.bYes.string = "Vote yes";
	s_votemenu.bYes.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_votemenu.bNo.generic.type = MTYPE_PTEXT;
	s_votemenu.bNo.color = color_red;
	s_votemenu.bNo.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_votemenu.bNo.generic.x = 320;
	s_votemenu.bNo.generic.y = y;
	s_votemenu.bNo.generic.id = ID_NO;
	s_votemenu.bNo.generic.callback = VoteMenu_Event;
	s_votemenu.bNo.string = "Vote no";
	s_votemenu.bNo.style = UI_CENTER|UI_SMALLFONT;

	s_votemenu.back.generic.type = MTYPE_BITMAP;
	s_votemenu.back.generic.name = VOTEMENU_BACK0;
	s_votemenu.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_votemenu.back.generic.callback = VoteMenu_Event;
	s_votemenu.back.generic.id = ID_BACK;
	s_votemenu.back.generic.x = 320 - 128;
	s_votemenu.back.generic.y = 256 + 128 - 64 + VOTEMENU_MENU_VERTICAL_SPACING * 2;
	s_votemenu.back.width = 128;
	s_votemenu.back.height = 64;
	s_votemenu.back.focuspic = VOTEMENU_BACK1;

	s_votemenu.go.generic.type = MTYPE_BITMAP;
	s_votemenu.go.generic.name = ART_FIGHT0;
	s_votemenu.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_votemenu.go.generic.callback = VoteMenu_Event;
	s_votemenu.go.generic.id = ID_GO;
	s_votemenu.go.generic.x = 320;
	s_votemenu.go.generic.y = 256 + 128 - 64 + VOTEMENU_MENU_VERTICAL_SPACING * 2;
	s_votemenu.go.width = 128;
	s_votemenu.go.height = 64;
	s_votemenu.go.focuspic = ART_FIGHT1;
}

/*
=======================================================================================================================================
UI_VoteMenu
=======================================================================================================================================
*/
void UI_VoteMenu(void) {

	// zero set all our globals
	memset(&s_votemenu, 0 , sizeof(votemenu_t));

	UI_VoteMenuInternal();

	Menu_AddItem(&s_votemenu.menu, (void *)&s_votemenu.bYes);
	Menu_AddItem(&s_votemenu.menu, (void *)&s_votemenu.bNo);

	UI_PushMenu(&s_votemenu.menu);
}
