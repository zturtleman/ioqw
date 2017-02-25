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

static char *callvotemenu_gametype_artlist[] = {
	VOTEMENU_BACK0,
	VOTEMENU_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

enum {
	ID_BACK,
	ID_GO,
	ID_FFA,
	ID_TOURNEY,
	ID_TDM,
	ID_CTF,
	ID_1FCTF,
	ID_OVERLOAD,
	ID_HARVESTER
};

#define Gametype_MENU_VERTICAL_SPACING 19

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s back;
	menubitmap_s go;
	// buttons
	menutext_s bFFA;
	menutext_s bTourney;
	menutext_s bTDM;
	menutext_s bCTF;
	menutext_s b1FCTF;
	menutext_s bOverload;
	menutext_s bHarvester;
	// allowed
	qboolean FFA;
	qboolean Tourney;
	qboolean TDM;
	qboolean CTF;
	qboolean Oneflag;
	qboolean Overload;
	qboolean Harvester;
	int selection;
} callvotemenu_t;

static callvotemenu_t s_callvotemenu_gametype;

void UI_VoteGametypeMenuInternal(void);

/*
=======================================================================================================================================
VoteMenu_Gametype_Event
=======================================================================================================================================
*/
static void VoteMenu_Gametype_Event(void *ptr, int event) {

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
		case ID_FFA:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 1");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_TOURNEY:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 2");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_TDM:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 3");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_CTF:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 4");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_1FCTF:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 5");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_OVERLOAD:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 6");
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_HARVESTER:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_gametype 7");
			UI_PopMenu();
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
SetGametypeMenuText
=======================================================================================================================================
*/
static void SetGametypeMenuText(menutext_s *menu, int y, int id, qboolean active, char *text) {

	menu->generic.type = MTYPE_PTEXT;
	menu->color = color_red;
	menu->generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;

	if (!active) {
		menu->generic.flags |= QMF_INACTIVE|QMF_GRAYED;
	} else if (s_callvotemenu_gametype.selection == id) {
		menu->color = color_orange;
	}

	menu->generic.x = 320;
	menu->generic.y = y;
	menu->generic.id = id;
	menu->generic.callback = VoteMenu_Gametype_Event;
	menu->string = text;
	menu->style = UI_CENTER|UI_SMALLFONT;
}

/*
=======================================================================================================================================
VoteMenu_Gametype_Cache
=======================================================================================================================================
*/
static void VoteMenu_Gametype_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0; ; i++) {
		if (!callvotemenu_gametype_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(callvotemenu_gametype_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_VoteMenu_Gametype_Draw
=======================================================================================================================================
*/
static void UI_VoteMenu_Gametype_Draw(void) {

	UI_DrawBannerString(320, 16, "CALL VOTE GAMETYPE", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_callvotemenu_gametype.menu);
}

/*
=======================================================================================================================================
UI_VoteGametypeMenuInternal
=======================================================================================================================================
*/
void UI_VoteGametypeMenuInternal(void) {
	int y;

	VoteMenu_Gametype_Cache();

	s_callvotemenu_gametype.menu.wrapAround = qtrue;
	s_callvotemenu_gametype.menu.fullscreen = qfalse;
	s_callvotemenu_gametype.menu.draw = UI_VoteMenu_Gametype_Draw;

	s_callvotemenu_gametype.banner.generic.type = MTYPE_BTEXT;
	s_callvotemenu_gametype.banner.generic.x = 320;
	s_callvotemenu_gametype.banner.generic.y = 16;
	s_callvotemenu_gametype.banner.string = "CALL VOTE GAMETYPE";
	s_callvotemenu_gametype.banner.color = color_white;
	s_callvotemenu_gametype.banner.style = UI_CENTER;

	y = 98;
	SetGametypeMenuText(&s_callvotemenu_gametype.bFFA, y, ID_FFA, s_callvotemenu_gametype.FFA, "Free For All");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.bTourney, y, ID_TOURNEY, s_callvotemenu_gametype.Tourney, "Tournament");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.bTDM, y, ID_TDM, s_callvotemenu_gametype.TDM, "Team Deathmatch");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.bCTF, y, ID_CTF, s_callvotemenu_gametype.CTF, "Capture the Flag");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.b1FCTF, y, ID_1FCTF, s_callvotemenu_gametype.Oneflag, "One Flag CTF");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.bOverload, y, ID_OVERLOAD, s_callvotemenu_gametype.Overload, "Overload");
	y += Gametype_MENU_VERTICAL_SPACING;
	SetGametypeMenuText(&s_callvotemenu_gametype.bHarvester, y, ID_HARVESTER, s_callvotemenu_gametype.Harvester, "Harvester");
	y += Gametype_MENU_VERTICAL_SPACING;

	s_callvotemenu_gametype.back.generic.type = MTYPE_BITMAP;
	s_callvotemenu_gametype.back.generic.name = VOTEMENU_BACK0;
	s_callvotemenu_gametype.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_gametype.back.generic.callback = VoteMenu_Gametype_Event;
	s_callvotemenu_gametype.back.generic.id = ID_BACK;
	s_callvotemenu_gametype.back.generic.x = 320 - 128;
	s_callvotemenu_gametype.back.generic.y = y;
	s_callvotemenu_gametype.back.width = 128;
	s_callvotemenu_gametype.back.height = 64;
	s_callvotemenu_gametype.back.focuspic = VOTEMENU_BACK1;

	s_callvotemenu_gametype.go.generic.type = MTYPE_BITMAP;
	s_callvotemenu_gametype.go.generic.name = ART_FIGHT0;
	s_callvotemenu_gametype.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu_gametype.go.generic.callback = VoteMenu_Gametype_Event;
	s_callvotemenu_gametype.go.generic.id = ID_GO;
	s_callvotemenu_gametype.go.generic.x = 320;
	s_callvotemenu_gametype.go.generic.y = 256 + 128 - 64;
	s_callvotemenu_gametype.go.width = 128;
	s_callvotemenu_gametype.go.height = 64;
	s_callvotemenu_gametype.go.focuspic = ART_FIGHT1;
}

/*
=======================================================================================================================================
UI_VoteGametypeMenu
=======================================================================================================================================
*/
void UI_VoteGametypeMenu(void) {
	char serverinfo[MAX_INFO_STRING], *gametypeinfo;

	// zero set all our globals
	memset(&s_callvotemenu_gametype, 0 , sizeof(callvotemenu_t));

	trap_GetConfigString(CS_SERVERINFO, serverinfo, MAX_INFO_STRING);

	gametypeinfo = Info_ValueForKey(serverinfo, "g_voteGametypes");

	if (Q_stricmp(gametypeinfo, "*")) {
		s_callvotemenu_gametype.FFA = qtrue;
		s_callvotemenu_gametype.Tourney = qtrue;
		s_callvotemenu_gametype.TDM = qtrue;
		s_callvotemenu_gametype.CTF = qtrue;
		s_callvotemenu_gametype.Oneflag = qtrue;
		s_callvotemenu_gametype.Overload = qtrue;
		s_callvotemenu_gametype.Harvester = qtrue;
	} else {
		s_callvotemenu_gametype.FFA = (qboolean)Q_stristr(gametypeinfo, "/1/");
		s_callvotemenu_gametype.Tourney = (qboolean)Q_stristr(gametypeinfo, "/2/");
		s_callvotemenu_gametype.TDM = (qboolean)Q_stristr(gametypeinfo, "/3/");
		s_callvotemenu_gametype.CTF = (qboolean)Q_stristr(gametypeinfo, "/4/");
		s_callvotemenu_gametype.Oneflag = (qboolean)Q_stristr(gametypeinfo, "/5/");
		s_callvotemenu_gametype.Overload = (qboolean)Q_stristr(gametypeinfo, "/6/");
		s_callvotemenu_gametype.Harvester = (qboolean)Q_stristr(gametypeinfo, "/7/");
	}

	UI_VoteGametypeMenuInternal();

	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.banner);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.back);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bFFA);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bTourney);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bTDM);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bCTF);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.b1FCTF);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bOverload);
	Menu_AddItem(&s_callvotemenu_gametype.menu, (void *)&s_callvotemenu_gametype.bHarvester);

	UI_PushMenu(&s_callvotemenu_gametype.menu);
}
