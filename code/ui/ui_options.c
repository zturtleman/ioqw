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

	SYSTEM CONFIGURATION MENU

=======================================================================================================================================
*/

#include "ui_local.h"

#define ART_FRAMEL "menu/art/frame2_l"
#define ART_FRAMER "menu/art/frame1_r"
#define ART_BACK0 "menu/art/back_0"
#define ART_BACK1 "menu/art/back_1"

#define VERTICAL_SPACING 34

enum {
	ID_GRAPHICS,
	ID_DISPLAY,
	ID_SOUND,
	ID_NETWORK,
	ID_BACK
};

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s framel;
	menubitmap_s framer;
	menutext_s graphics;
	menutext_s display;
	menutext_s sound;
	menutext_s network;
	menubitmap_s back;
} optionsmenu_t;

static optionsmenu_t s_options;

/*
=======================================================================================================================================
Options_Event
=======================================================================================================================================
*/
static void Options_Event(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_GRAPHICS:
			UI_GraphicsOptionsMenu();
			break;
		case ID_DISPLAY:
			UI_DisplayOptionsMenu();
			break;
		case ID_SOUND:
			UI_SoundOptionsMenu();
			break;
		case ID_NETWORK:
			UI_NetworkOptionsMenu();
			break;
		case ID_BACK:
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
SystemConfig_Cache
=======================================================================================================================================
*/
void SystemConfig_Cache(void) {

	trap_R_RegisterShaderNoMip(ART_FRAMEL);
	trap_R_RegisterShaderNoMip(ART_FRAMER);
	trap_R_RegisterShaderNoMip(ART_BACK0);
	trap_R_RegisterShaderNoMip(ART_BACK1);
}

/*
=======================================================================================================================================
Options_MenuInit
=======================================================================================================================================
*/
void Options_MenuInit(void) {
	int y;
	uiClientState_t cstate;

	memset(&s_options, 0, sizeof(optionsmenu_t));

	SystemConfig_Cache();

	s_options.menu.wrapAround = qtrue;

	trap_GetClientState(&cstate);

	if (cstate.connState >= CA_CONNECTED) {
		s_options.menu.fullscreen = qfalse;
	} else {
		s_options.menu.fullscreen = qtrue;
	}

	s_options.banner.generic.type = MTYPE_BTEXT;
	s_options.banner.generic.flags = QMF_CENTER_JUSTIFY;
	s_options.banner.generic.x = 320;
	s_options.banner.generic.y = 16;
	s_options.banner.string = "System Setup";
	s_options.banner.color = color_white;
	s_options.banner.style = UI_CENTER;

	s_options.framel.generic.type = MTYPE_BITMAP;
	s_options.framel.generic.name = ART_FRAMEL;
	s_options.framel.generic.flags = QMF_INACTIVE;
	s_options.framel.generic.x = 8;
	s_options.framel.generic.y = 76;
	s_options.framel.width = 256;
	s_options.framel.height = 334;

	s_options.framer.generic.type = MTYPE_BITMAP;
	s_options.framer.generic.name = ART_FRAMER;
	s_options.framer.generic.flags = QMF_INACTIVE;
	s_options.framer.generic.x = 376;
	s_options.framer.generic.y = 76;
	s_options.framer.width = 256;
	s_options.framer.height = 334;

	y = 168;
	s_options.graphics.generic.type = MTYPE_PTEXT;
	s_options.graphics.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.graphics.generic.callback = Options_Event;
	s_options.graphics.generic.id = ID_GRAPHICS;
	s_options.graphics.generic.x = 320;
	s_options.graphics.generic.y = y;
	s_options.graphics.string = "Graphics";
	s_options.graphics.color = color_red;
	s_options.graphics.style = UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.display.generic.type = MTYPE_PTEXT;
	s_options.display.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.display.generic.callback = Options_Event;
	s_options.display.generic.id = ID_DISPLAY;
	s_options.display.generic.x = 320;
	s_options.display.generic.y = y;
	s_options.display.string = "Display";
	s_options.display.color = color_red;
	s_options.display.style = UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.sound.generic.type = MTYPE_PTEXT;
	s_options.sound.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.sound.generic.callback = Options_Event;
	s_options.sound.generic.id = ID_SOUND;
	s_options.sound.generic.x = 320;
	s_options.sound.generic.y = y;
	s_options.sound.string = "Sound";
	s_options.sound.color = color_red;
	s_options.sound.style = UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.network.generic.type = MTYPE_PTEXT;
	s_options.network.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.network.generic.callback = Options_Event;
	s_options.network.generic.id = ID_NETWORK;
	s_options.network.generic.x = 320;
	s_options.network.generic.y = y;
	s_options.network.string = "Network";
	s_options.network.color = color_red;
	s_options.network.style = UI_CENTER;

	s_options.back.generic.type = MTYPE_BITMAP;
	s_options.back.generic.name = ART_BACK0;
	s_options.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.back.generic.callback = Options_Event;
	s_options.back.generic.id = ID_BACK;
	s_options.back.generic.x = 0;
	s_options.back.generic.y = 480 - 64;
	s_options.back.width = 128;
	s_options.back.height = 64;
	s_options.back.focuspic = ART_BACK1;

	Menu_AddItem(&s_options.menu, (void *)&s_options.banner);
	Menu_AddItem(&s_options.menu, (void *)&s_options.framel);
	Menu_AddItem(&s_options.menu, (void *)&s_options.framer);
	Menu_AddItem(&s_options.menu, (void *)&s_options.graphics);
	Menu_AddItem(&s_options.menu, (void *)&s_options.display);
	Menu_AddItem(&s_options.menu, (void *)&s_options.sound);
	Menu_AddItem(&s_options.menu, (void *)&s_options.network);
	Menu_AddItem(&s_options.menu, (void *)&s_options.back);
}

/*
=======================================================================================================================================
UI_SystemConfigMenu
=======================================================================================================================================
*/
void UI_SystemConfigMenu(void) {

	Options_MenuInit();
	UI_PushMenu(&s_options.menu);
}