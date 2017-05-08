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

/**************************************************************************************************************************************
 This is the menu displayed the first time the player selects "Multiplayer". It only has the must set or know about options!
 Options: Name, Network Rate, Auto Download.
**************************************************************************************************************************************/

#define ART_BACK0 "menu/art/back_0"
#define ART_BACK1 "menu/art/back_1"
#define ART_FRAMEL "menu/art/frame2_l"
#define ART_FRAMER "menu/art/frame1_r"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"

enum {
	ID_BACK,
	ID_GO,
	ID_NAME,
	ID_RATE,
	ID_ALLOWDOWNLOAD
};

#define MAX_NAMELENGTH 20

static const char *rate_items2[] = {
	"<= 28.8K",
	"33.6K",
	"56K",
	"ISDN",
	"LAN/Cable/xDSL",
	NULL
};

static char *art_artlist[] = {
	ART_FRAMEL,
	ART_FRAMER,
	ART_BACK0,
	ART_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s framel;
	menubitmap_s framer;
	menubitmap_s go;
	menubitmap_s back;
	menutext_s info;
	menutext_s info2;
	// important options
	menufield_s name;
	menulist_s rate;
	// optional options
	menuradiobutton_s allowdownload;
} firstconnect_t;

static firstconnect_t s_firstconnect;

/*
=======================================================================================================================================
PlayerSettings_SetName
=======================================================================================================================================
*/
static void PlayerSettings_SetName(void *self) {
	menufield_s *f;
	qboolean focus;
	int style;
	char *txt;
	char c;
	float *color;
	int n;
	int basex, x, y;
	char name[32];

	f = (menufield_s *)self;
	basex = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);
	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;

	if (focus) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString(basex, y, "Name:", style, color);
	// draw the actual name
	basex += 64 + 10;
	y += 4;
	txt = f->field.buffer;
	color = g_color_table[ColorIndex(COLOR_WHITE)];
	x = basex;

	while ((c = *txt) != 0) {
		if (!focus && Q_IsColorString(txt)) {
			n = ColorIndex(*(txt + 1));

			if (n == 0) {
				n = 7;
			}

			color = g_color_table[n];
			txt += 2;
			continue;
		}

		UI_DrawChar(x, y, c, style, color);

		txt++;
		x += SMALLCHAR_WIDTH;
	}
	// draw cursor if we have focus
	if (focus) {
		if (trap_Key_GetOverstrikeMode()) {
			c = 11;
		} else {
			c = 10;
		}

		style &= ~UI_PULSE;
		style |= UI_BLINK;

		UI_DrawChar(basex + f->field.cursor * SMALLCHAR_WIDTH, y, c, style, color_white);
	}
	// draw at bottom also using proportional font
	Q_strncpyz(name, f->field.buffer, sizeof(name));
	Q_CleanStr(name);
}

/*
=======================================================================================================================================
FirstConnect_StatusBar_Name
=======================================================================================================================================
*/
static void FirstConnect_StatusBar_Name(void *ptr) {
	UI_DrawString(320, 440, "Your nickname", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstConnect_StatusBar_Rate
=======================================================================================================================================
*/
static void FirstConnect_StatusBar_Rate(void *ptr) {
	UI_DrawString(320, 440, "Your connection speed", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstConnect_StatusBar_Download
=======================================================================================================================================
*/
static void FirstConnect_StatusBar_Download(void *ptr) {
	UI_DrawString(320, 440, "Auto download missing maps and mods", UI_CENTER|UI_SMALLFONT, colorWhite);
}

/*
=======================================================================================================================================
FirstConnect_SaveChanges
=======================================================================================================================================
*/
static void FirstConnect_SaveChanges(void) {

	// name
	trap_Cvar_Set("name", s_firstconnect.name.field.buffer);
}

/*
=======================================================================================================================================
FirstConnect_Event
=======================================================================================================================================
*/
static void FirstConnect_Event(void *ptr, int event) {

	switch (((menucommon_s *)ptr)->id) {
		case ID_GO:
			if (event != QM_ACTIVATED) {
				break;
			}

			FirstConnect_SaveChanges();

			UI_PopMenu();

			trap_Cvar_SetValue("ui_firststart", 0);

			UI_ArenaServersMenu();
			break;
		case ID_BACK:
			if (event != QM_ACTIVATED) {
				break;
			}

			UI_PopMenu();
			break;
		case ID_RATE:
			if (s_firstconnect.rate.curvalue == 0) {
				trap_Cvar_SetValue("rate", 2500);
			} else if (s_firstconnect.rate.curvalue == 1) {
				trap_Cvar_SetValue("rate", 3000);
			} else if (s_firstconnect.rate.curvalue == 2) {
				trap_Cvar_SetValue("rate", 4000);
			} else if (s_firstconnect.rate.curvalue == 3) {
				trap_Cvar_SetValue("rate", 5000);
			} else if (s_firstconnect.rate.curvalue == 4) {
				trap_Cvar_SetValue("rate", 25000);
			}

			break;
		case ID_ALLOWDOWNLOAD:
			trap_Cvar_SetValue("cl_allowDownload", s_firstconnect.allowdownload.curvalue);
			trap_Cvar_SetValue("sv_allowDownload", s_firstconnect.allowdownload.curvalue);
			break;
	}
}

/*
=======================================================================================================================================
FirstConnect_SetMenuItems
=======================================================================================================================================
*/
static void FirstConnect_SetMenuItems(void) {
	int rate;

	// name
	Q_strncpyz(s_firstconnect.name.field.buffer, UI_Cvar_VariableString("name"), sizeof(s_firstconnect.name.field.buffer));

	rate = trap_Cvar_VariableValue("rate");

	if (rate <= 2500) {
		s_firstconnect.rate.curvalue = 0;
	} else if (rate <= 3000) {
		s_firstconnect.rate.curvalue = 1;
	} else if (rate <= 4000) {
		s_firstconnect.rate.curvalue = 2;
	} else if (rate <= 5000) {
		s_firstconnect.rate.curvalue = 3;
	} else {
		s_firstconnect.rate.curvalue = 4;
	}

	s_firstconnect.allowdownload.curvalue = trap_Cvar_VariableValue("cl_allowDownload") != 0;
}

/*
=======================================================================================================================================
FirstConnect_MenuInit
=======================================================================================================================================
*/
void FirstConnect_MenuInit(void) {
	int y;

	// zero set all our globals
	memset(&s_firstconnect, 0, sizeof(firstconnect_t));

	FirstConnect_Cache();

	s_firstconnect.menu.wrapAround = qtrue;
	s_firstconnect.menu.fullscreen = qtrue;

	s_firstconnect.banner.generic.type = MTYPE_BTEXT;
	s_firstconnect.banner.generic.x = 320;
	s_firstconnect.banner.generic.y = 16;
	s_firstconnect.banner.string = "FIRST CONNECT";
	s_firstconnect.banner.color = color_white;
	s_firstconnect.banner.style = UI_CENTER;

	s_firstconnect.framel.generic.type = MTYPE_BITMAP;
	s_firstconnect.framel.generic.name = ART_FRAMEL;
	s_firstconnect.framel.generic.flags = QMF_INACTIVE;
	s_firstconnect.framel.generic.x = 0;
	s_firstconnect.framel.generic.y = 78;
	s_firstconnect.framel.width = 256;
	s_firstconnect.framel.height = 329;

	s_firstconnect.framer.generic.type = MTYPE_BITMAP;
	s_firstconnect.framer.generic.name = ART_FRAMER;
	s_firstconnect.framer.generic.flags = QMF_INACTIVE;
	s_firstconnect.framer.generic.x = 376;
	s_firstconnect.framer.generic.y = 76;
	s_firstconnect.framer.width = 256;
	s_firstconnect.framer.height = 334;

	s_firstconnect.go.generic.type = MTYPE_BITMAP;
	s_firstconnect.go.generic.name = ART_FIGHT0;
	s_firstconnect.go.generic.flags = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_firstconnect.go.generic.callback = FirstConnect_Event;
	s_firstconnect.go.generic.id = ID_GO;
	s_firstconnect.go.generic.x = 640;
	s_firstconnect.go.generic.y = 480 - 64;
	s_firstconnect.go.width = 128;
	s_firstconnect.go.height = 64;
	s_firstconnect.go.focuspic = ART_FIGHT1;

	s_firstconnect.back.generic.type = MTYPE_BITMAP;
	s_firstconnect.back.generic.name = ART_BACK0;
	s_firstconnect.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_firstconnect.back.generic.callback = FirstConnect_Event;
	s_firstconnect.back.generic.id = ID_BACK;
	s_firstconnect.back.generic.x = 0;
	s_firstconnect.back.generic.y = 480 - 64;
	s_firstconnect.back.width = 128;
	s_firstconnect.back.height = 64;
	s_firstconnect.back.focuspic = ART_BACK1;

	y = 144;
	s_firstconnect.name.generic.type = MTYPE_FIELD;
	s_firstconnect.name.generic.flags = QMF_NODEFAULTINIT;
	s_firstconnect.name.generic.ownerdraw = PlayerSettings_SetName;
	s_firstconnect.name.field.widthInChars = MAX_NAMELENGTH;
	s_firstconnect.name.field.maxchars = MAX_NAMELENGTH;
	s_firstconnect.name.generic.x = 192;
	s_firstconnect.name.generic.y = y;
	s_firstconnect.name.generic.left = 192 - 8;
	s_firstconnect.name.generic.top = y - 8;
	s_firstconnect.name.generic.right = 192 + 200;
	s_firstconnect.name.generic.bottom = y + 2 * PROP_HEIGHT;
	s_firstconnect.name.generic.statusbar = FirstConnect_StatusBar_Name;

	y += 4 * PROP_HEIGHT;
	s_firstconnect.rate.generic.type = MTYPE_SPINCONTROL;
	s_firstconnect.rate.generic.name = "Data Rate:";
	s_firstconnect.rate.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_firstconnect.rate.generic.callback = FirstConnect_Event;
	s_firstconnect.rate.generic.id = ID_RATE;
	s_firstconnect.rate.generic.x = 320;
	s_firstconnect.rate.generic.y = y;
	s_firstconnect.rate.itemnames = rate_items2;
	s_firstconnect.rate.generic.statusbar = FirstConnect_StatusBar_Rate;

	y += BIGCHAR_HEIGHT + 2;
	s_firstconnect.allowdownload.generic.type = MTYPE_RADIOBUTTON;
	s_firstconnect.allowdownload.generic.name = "Automatic Downloading:";
	s_firstconnect.allowdownload.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_firstconnect.allowdownload.generic.callback = FirstConnect_Event;
	s_firstconnect.allowdownload.generic.id = ID_ALLOWDOWNLOAD;
	s_firstconnect.allowdownload.generic.x = 320;
	s_firstconnect.allowdownload.generic.y = y;
	s_firstconnect.allowdownload.generic.statusbar = FirstConnect_StatusBar_Download;

	s_firstconnect.info.generic.type = MTYPE_TEXT;
	s_firstconnect.info.generic.x = 320;
	s_firstconnect.info.generic.y = 400;
	s_firstconnect.info.color = color_white;
	s_firstconnect.info.style = UI_CENTER|UI_SMALLFONT;
	s_firstconnect.info.string = "Note: All settings can be changed later in SETUP";

	s_firstconnect.info2.generic.type = MTYPE_TEXT;
	s_firstconnect.info2.generic.x = 320;
	s_firstconnect.info2.generic.y = 80;
	s_firstconnect.info2.color = color_white;
	s_firstconnect.info2.style = UI_CENTER|UI_SMALLFONT;
	s_firstconnect.info2.string = "Please verify these settings";

	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.banner);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.framel);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.framer);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.go);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.back);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.name);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.rate);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.allowdownload);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.info);
	Menu_AddItem(&s_firstconnect.menu, &s_firstconnect.info2);

	FirstConnect_SetMenuItems();
}

/*
=======================================================================================================================================
FirstConnect_Cache
=======================================================================================================================================
*/
void FirstConnect_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0;; i++) {
		if (!art_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(art_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_FirstConnectMenu
=======================================================================================================================================
*/
void UI_FirstConnectMenu(void) {

	FirstConnect_MenuInit();
	UI_PushMenu(&s_firstconnect.menu);
}
