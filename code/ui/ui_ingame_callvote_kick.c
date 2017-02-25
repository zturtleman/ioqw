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

#define ART_BACK0 "menu/art/back_0"
#define ART_BACK1 "menu/art/back_1"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"
#define ART_BACKGROUND "menu/art/addbotframe"
#define ART_ARROWS "menu/art/arrows_vert_0"
#define ART_ARROWUP "menu/art/arrows_vert_top"
#define ART_ARROWDOWN "menu/art/arrows_vert_bot"

enum {
	ID_BACK,
	ID_GO,
	ID_LIST,
	ID_UP,
	ID_DOWN,
	ID_PLAYERNAME0,
	ID_PLAYERNAME1,
	ID_PLAYERNAME2,
	ID_PLAYERNAME3,
	ID_PLAYERNAME4,
	ID_PLAYERNAME5,
	ID_PLAYERNAME6,
	ID_PLAYERNAME7,
	ID_PLAYERNAME8,
	ID_PLAYERNAME9
};

#define SIZE_OF_LIST 10
#define SIZE_OF_NAME 32
#define VOTEKICK_MENU_VERTICAL_SPACING 20

typedef struct {
	int id;
	char name[SIZE_OF_NAME];
} player;

typedef struct {
	menuframework_s menu;
	menubitmap_s arrows;
	menutext_s banner;
	menubitmap_s up;
	menubitmap_s down;
	menutext_s players[SIZE_OF_LIST];
	menubitmap_s go;
	menubitmap_s back;
	int numPlayers;
	int selected;
	int startIndex;
	player players_profiles[MAX_CLIENTS];
} votemenu_kick_t;

static votemenu_kick_t s_callvotemenu_kick;

void UI_VoteKickMenuInternal(void);

/*
=======================================================================================================================================
PopulatePlayerList
=======================================================================================================================================
*/
static void PopulatePlayerList(void) {
	int i;
	char playerinfo[MAX_INFO_STRING];
	s_callvotemenu_kick.numPlayers = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		trap_GetConfigString(CS_PLAYERS + i, playerinfo, MAX_INFO_STRING);

		if (strlen(playerinfo)) {
			s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.numPlayers].id = i;
			Q_strncpyz(s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.numPlayers].name, Info_ValueForKey(playerinfo, "n"), SIZE_OF_NAME - 2);
			s_callvotemenu_kick.numPlayers++;
		}
	}
}

/*
=======================================================================================================================================
VoteMenu_Kick_EventEvent
=======================================================================================================================================
*/
static void VoteMenu_Kick_EventEvent(void *ptr, int event) {

	switch (((menucommon_s *)ptr)->id) {
		case ID_BACK:
			if (event != QM_ACTIVATED) {
				return;
			}

			UI_PopMenu();
			break;
		default:
			if (event != QM_ACTIVATED) {
				return;
			}

			if (s_callvotemenu_kick.selected != ((menucommon_s *)ptr)->id) {
				s_callvotemenu_kick.selected = ((menucommon_s *)ptr)->id;
				UI_VoteKickMenuInternal();
			}

			if (event != QM_ACTIVATED) {
				return;
			}

			if (!s_callvotemenu_kick.selected) {
				return;
			}

			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote clientkick %d", s_callvotemenu_kick.players_profiles[(s_callvotemenu_kick.selected - 20) + s_callvotemenu_kick.startIndex].id));

			UI_PopMenu();
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
UI_VoteMenu_Kick_EventUpEvent
=======================================================================================================================================
*/
static void UI_VoteMenu_Kick_EventUpEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (s_callvotemenu_kick.startIndex > 0) {
		s_callvotemenu_kick.startIndex--;
	}

	UI_VoteKickMenuInternal();
}

/*
=======================================================================================================================================
UI_VoteMenu_Kick_EventDownEvent
=======================================================================================================================================
*/
static void UI_VoteMenu_Kick_EventDownEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (s_callvotemenu_kick.startIndex + SIZE_OF_LIST < s_callvotemenu_kick.numPlayers) {
		s_callvotemenu_kick.startIndex++;
	}

	UI_VoteKickMenuInternal();
}

/*
=======================================================================================================================================
UI_VoteMenu_Kick_EventDraw
=======================================================================================================================================
*/
static void UI_VoteMenu_Kick_EventDraw(void) {

	UI_DrawBannerString(320, 16, "CALL VOTE KICK", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_callvotemenu_kick.menu);
}

/*
=======================================================================================================================================
VoteMenu_Kick_EventCache
=======================================================================================================================================
*/
static void VoteMenu_Kick_EventCache(void) {

	trap_R_RegisterShaderNoMip(ART_BACK0);
	trap_R_RegisterShaderNoMip(ART_BACK1);
	trap_R_RegisterShaderNoMip(ART_FIGHT0);
	trap_R_RegisterShaderNoMip(ART_FIGHT1);
	trap_R_RegisterShaderNoMip(ART_BACKGROUND);
	trap_R_RegisterShaderNoMip(ART_ARROWS);
	trap_R_RegisterShaderNoMip(ART_ARROWUP);
	trap_R_RegisterShaderNoMip(ART_ARROWDOWN);
}

/*
=======================================================================================================================================
SetKickMenuText
=======================================================================================================================================
*/
static void SetKickMenuText(menutext_s *menu, int y, int id, qboolean active, char *text) {

	menu->generic.type = MTYPE_PTEXT;
	menu->color = color_red;
	menu->generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;

	if (!active) {
		menu->generic.flags |= QMF_INACTIVE|QMF_GRAYED;
	} else if (s_callvotemenu_kick.selected == id) {
		menu->color = color_orange;
	}

	menu->generic.x = 320;
	menu->generic.y = y;
	menu->generic.id = id;
	menu->generic.callback = VoteMenu_Kick_EventEvent;
	menu->string = text;
	menu->style = UI_CENTER|UI_SMALLFONT;
}

/*
=======================================================================================================================================
UI_VoteKickMenuInternal
=======================================================================================================================================
*/
void UI_VoteKickMenuInternal(void) {
	int y;

	s_callvotemenu_kick.menu.wrapAround = qtrue;
	s_callvotemenu_kick.menu.fullscreen = qfalse;
	s_callvotemenu_kick.menu.draw = UI_VoteMenu_Kick_EventDraw;

	s_callvotemenu_kick.banner.generic.type = MTYPE_BTEXT;
	s_callvotemenu_kick.banner.generic.x = 320;
	s_callvotemenu_kick.banner.generic.y = 16;
	s_callvotemenu_kick.banner.string = "CALL VOTE KICK";
	s_callvotemenu_kick.banner.color = color_white;
	s_callvotemenu_kick.banner.style = UI_CENTER;

	s_callvotemenu_kick.arrows.generic.type = MTYPE_BITMAP;
	s_callvotemenu_kick.arrows.generic.name = ART_ARROWS;
	s_callvotemenu_kick.arrows.generic.flags = QMF_INACTIVE;
	s_callvotemenu_kick.arrows.generic.x = 200;
	s_callvotemenu_kick.arrows.generic.y = 128;
	s_callvotemenu_kick.arrows.width = 64;
	s_callvotemenu_kick.arrows.height = 128;

	y = 98;
	SetKickMenuText(&s_callvotemenu_kick.players[0], y, ID_PLAYERNAME0, s_callvotemenu_kick.startIndex < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[1], y, ID_PLAYERNAME1, s_callvotemenu_kick.startIndex + 1 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 1].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[2], y, ID_PLAYERNAME2, s_callvotemenu_kick.startIndex + 2 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 2].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[3], y, ID_PLAYERNAME3, s_callvotemenu_kick.startIndex + 3 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 3].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[4], y, ID_PLAYERNAME4, s_callvotemenu_kick.startIndex + 4 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 4].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[5], y, ID_PLAYERNAME5, s_callvotemenu_kick.startIndex + 5 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 5].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[6], y, ID_PLAYERNAME6, s_callvotemenu_kick.startIndex + 6 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 6].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[7], y, ID_PLAYERNAME7, s_callvotemenu_kick.startIndex + 7 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 7].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[8], y, ID_PLAYERNAME8, s_callvotemenu_kick.startIndex + 8 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 8].name);
	y += VOTEKICK_MENU_VERTICAL_SPACING;
	SetKickMenuText(&s_callvotemenu_kick.players[9], y, ID_PLAYERNAME9, s_callvotemenu_kick.startIndex + 9 < s_callvotemenu_kick.numPlayers, s_callvotemenu_kick.players_profiles[s_callvotemenu_kick.startIndex + 9].name);

	s_callvotemenu_kick.up.generic.type = MTYPE_BITMAP;
	s_callvotemenu_kick.up.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_kick.up.generic.x = 200;
	s_callvotemenu_kick.up.generic.y = 128;
	s_callvotemenu_kick.up.generic.id = ID_UP;
	s_callvotemenu_kick.up.generic.callback = UI_VoteMenu_Kick_EventUpEvent;
	s_callvotemenu_kick.up.width = 64;
	s_callvotemenu_kick.up.height = 64;
	s_callvotemenu_kick.up.focuspic = ART_ARROWUP;

	s_callvotemenu_kick.down.generic.type = MTYPE_BITMAP;
	s_callvotemenu_kick.down.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_kick.down.generic.x = 200;
	s_callvotemenu_kick.down.generic.y = 128 + 64;
	s_callvotemenu_kick.down.generic.id = ID_DOWN;
	s_callvotemenu_kick.down.generic.callback = UI_VoteMenu_Kick_EventDownEvent;
	s_callvotemenu_kick.down.width = 64;
	s_callvotemenu_kick.down.height = 64;
	s_callvotemenu_kick.down.focuspic = ART_ARROWDOWN;

	s_callvotemenu_kick.go.generic.type = MTYPE_BITMAP;
	s_callvotemenu_kick.go.generic.name = ART_FIGHT0;
	s_callvotemenu_kick.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_kick.go.generic.id = ID_GO;
	s_callvotemenu_kick.go.generic.callback = VoteMenu_Kick_EventEvent;
	s_callvotemenu_kick.go.generic.x = 320 + 128 - 128;
	s_callvotemenu_kick.go.generic.y = 256 + 128 - 64;
	s_callvotemenu_kick.go.width = 128;
	s_callvotemenu_kick.go.height = 64;
	s_callvotemenu_kick.go.focuspic = ART_FIGHT1;

	s_callvotemenu_kick.back.generic.type = MTYPE_BITMAP;
	s_callvotemenu_kick.back.generic.name = ART_BACK0;
	s_callvotemenu_kick.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_kick.back.generic.id = ID_BACK;
	s_callvotemenu_kick.back.generic.callback = VoteMenu_Kick_EventEvent;
	s_callvotemenu_kick.back.generic.x = 320 - 128;
	s_callvotemenu_kick.back.generic.y = 256 + 128 - 64;
	s_callvotemenu_kick.back.width = 128;
	s_callvotemenu_kick.back.height = 64;
	s_callvotemenu_kick.back.focuspic = ART_BACK1;
}

/*
=======================================================================================================================================
UI_VoteKickMenu
=======================================================================================================================================
*/
void UI_VoteKickMenu(void) {
	int i;

	VoteMenu_Kick_EventCache();

	memset(&s_callvotemenu_kick, 0 , sizeof(votemenu_kick_t));

	PopulatePlayerList();

	UI_VoteKickMenuInternal();

	Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.banner);
	Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.back);
	Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.arrows);
	Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.down);
	Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.up);

	for (i = 0; i < SIZE_OF_LIST; i++) {
		Menu_AddItem(&s_callvotemenu_kick.menu, (void *)&s_callvotemenu_kick.players[i]);
	}

	UI_PushMenu(&s_callvotemenu_kick.menu);
}
