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

	MAIN MENU

=======================================================================================================================================
*/

#include "ui_local.h"

#define MAIN_BANNER_MODEL "models/mapobjects/banner/banner5.md3"
#define MAIN_MENU_VERTICAL_SPACING 34

enum {
	ID_SINGLEPLAYER,
	ID_MULTIPLAYER,
	ID_SETUP,
	ID_DEMOS,
	ID_CINEMATICS,
	ID_MODS,
	ID_EXIT
};

typedef struct {
	menuframework_s menu;
	menutext_s singleplayer;
	menutext_s multiplayer;
	menutext_s setup;
	menutext_s demos;
	menutext_s cinematics;
	menutext_s mods;
	menutext_s exit;
	qhandle_t bannerModel;
} mainmenu_t;

static mainmenu_t s_main;

typedef struct {
	menuframework_s menu;
	char errorMessage[4096];
} errorMessage_t;

static errorMessage_t s_errorMessage;

/*
=======================================================================================================================================
MainMenu_ExitAction
=======================================================================================================================================
*/
static void MainMenu_ExitAction(qboolean result) {

	if (!result) {
		return;
	}

	UI_PopMenu();
	UI_CreditMenu();
}

/*
=======================================================================================================================================
Main_MenuEvent
=======================================================================================================================================
*/
void Main_MenuEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_SINGLEPLAYER:
			UI_SPLevelMenu();
			break;
		case ID_MULTIPLAYER:
			UI_ArenaServersMenu();
			break;
		case ID_SETUP:
			UI_SetupMenu();
			break;
		case ID_DEMOS:
			UI_DemosMenu();
			break;
		case ID_CINEMATICS:
			UI_CinematicsMenu();
			break;
		case ID_MODS:
			UI_ModsMenu();
			break;
		case ID_EXIT:
			UI_ConfirmMenu("Exit Game?", 0, MainMenu_ExitAction);
			break;
	}
}

/*
=======================================================================================================================================
MainMenu_Cache
=======================================================================================================================================
*/
void MainMenu_Cache(void) {
	s_main.bannerModel = trap_R_RegisterModel(MAIN_BANNER_MODEL);
}

/*
=======================================================================================================================================
ErrorMessage_Key
=======================================================================================================================================
*/
sfxHandle_t ErrorMessage_Key(int key) {

	trap_Cvar_Set("com_errorMessage", "");
	UI_MainMenu();
	return (menu_null_sound);
}

/*
=======================================================================================================================================
Main_MenuDraw

This function is common to the main menu and errorMessage menu.
=======================================================================================================================================
*/
static void Main_MenuDraw(void) {
	refdef_t refdef;
	refEntity_t ent;
	vec3_t origin;
	vec3_t angles;
	float adjust;
	float x, y, w, h;
	vec4_t color = {0.5, 0, 0, 1};

	// setup the refdef
	memset(&refdef, 0, sizeof(refdef));

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	x = 0;
	y = 0;
	w = 640;
	h = 120;

	UI_AdjustFrom640(&x, &y, &w, &h);

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	adjust = 0; // JDC: Kenneth asked me to stop this 1.0 * sin((float)uis.realtime / 1000);

	refdef.fov_x = 60 + adjust;
	refdef.fov_y = 19.6875 + adjust;
	refdef.time = uis.realtime;

	origin[0] = 300;
	origin[1] = 0;
	origin[2] = -32;

	trap_R_ClearScene();
	// add the model
	memset(&ent, 0, sizeof(ent));

	adjust = 5.0 * sin((float)uis.realtime / 5000);

	VectorSet(angles, 0, 180 + adjust, 0);
	AnglesToAxis(angles, ent.axis);

	ent.hModel = s_main.bannerModel;

	VectorCopy(origin, ent.origin);
	VectorCopy(origin, ent.lightingOrigin);

	ent.renderfx = RF_LIGHTING_ORIGIN|RF_NOSHADOW;

	VectorCopy(ent.origin, ent.oldorigin);

	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);

	if (strlen(s_errorMessage.errorMessage)) {
		UI_DrawProportionalString_AutoWrapped(320, 192, 600, 20, s_errorMessage.errorMessage, UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color);
	} else {
		// standard menu drawing
		Menu_Draw(&s_main.menu);
	}

	UI_DrawString(320, 450, "Quake III Arena(c) 1999-2000, Id Software, Inc. All Rights Reserved", UI_CENTER|UI_SMALLFONT, color);
}

/*
=======================================================================================================================================
UI_MainMenu

The main menu only comes up when not in a game, so make sure that the attract loop server is down and that local cinematics are killed.
=======================================================================================================================================
*/
void UI_MainMenu(void) {
	int y;
	int style = UI_CENTER|UI_DROPSHADOW;

	trap_Cvar_SetValue("sv_killserver", 1);

	if (ui_firstrun.integer) {
		UI_FirstRunMenu();
		return;
	}

	memset(&s_main, 0, sizeof(mainmenu_t));
	memset(&s_errorMessage, 0, sizeof(errorMessage_t));
	// com_errorMessage would need that too
	MainMenu_Cache();

	trap_Cvar_VariableStringBuffer("com_errorMessage", s_errorMessage.errorMessage, sizeof(s_errorMessage.errorMessage));

	if (strlen(s_errorMessage.errorMessage)) {
		s_errorMessage.menu.draw = Main_MenuDraw;
		s_errorMessage.menu.key = ErrorMessage_Key;
		s_errorMessage.menu.fullscreen = qtrue;
		s_errorMessage.menu.wrapAround = qtrue;
		s_errorMessage.menu.showlogo = qtrue;

		trap_Key_SetCatcher(KEYCATCH_UI);

		uis.menusp = 0;

		UI_PushMenu(&s_errorMessage.menu);
		return;
	}

	s_main.menu.draw = Main_MenuDraw;
	s_main.menu.fullscreen = qtrue;
	s_main.menu.wrapAround = qtrue;
	s_main.menu.showlogo = qtrue;

	y = 134;
	s_main.singleplayer.generic.type = MTYPE_PTEXT;
	s_main.singleplayer.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.singleplayer.generic.x = 320;
	s_main.singleplayer.generic.y = y;
	s_main.singleplayer.generic.id = ID_SINGLEPLAYER;
	s_main.singleplayer.generic.callback = Main_MenuEvent;
	s_main.singleplayer.string = "Singleplayer";
	s_main.singleplayer.color = color_red;
	s_main.singleplayer.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.multiplayer.generic.type = MTYPE_PTEXT;
	s_main.multiplayer.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.multiplayer.generic.x = 320;
	s_main.multiplayer.generic.y = y;
	s_main.multiplayer.generic.id = ID_MULTIPLAYER;
	s_main.multiplayer.generic.callback = Main_MenuEvent;
	s_main.multiplayer.string = "Multiplayer";
	s_main.multiplayer.color = color_red;
	s_main.multiplayer.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.cinematics.generic.type = MTYPE_PTEXT;
	s_main.cinematics.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.cinematics.generic.x = 320;
	s_main.cinematics.generic.y = y;
	s_main.cinematics.generic.id = ID_CINEMATICS;
	s_main.cinematics.generic.callback = Main_MenuEvent;
	s_main.cinematics.string = "Cinematics";
	s_main.cinematics.color = color_red;
	s_main.cinematics.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.demos.generic.type = MTYPE_PTEXT;
	s_main.demos.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.demos.generic.x = 320;
	s_main.demos.generic.y = y;
	s_main.demos.generic.id = ID_DEMOS;
	s_main.demos.generic.callback = Main_MenuEvent;
	s_main.demos.string = "Demos";
	s_main.demos.color = color_red;
	s_main.demos.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.setup.generic.type = MTYPE_PTEXT;
	s_main.setup.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.setup.generic.x = 320;
	s_main.setup.generic.y = y;
	s_main.setup.generic.id = ID_SETUP;
	s_main.setup.generic.callback = Main_MenuEvent;
	s_main.setup.string = "Setup";
	s_main.setup.color = color_red;
	s_main.setup.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.mods.generic.type = MTYPE_PTEXT;
	s_main.mods.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.mods.generic.x = 320;
	s_main.mods.generic.y = y;
	s_main.mods.generic.id = ID_MODS;
	s_main.mods.generic.callback = Main_MenuEvent;
	s_main.mods.string = "Mods";
	s_main.mods.color = color_red;
	s_main.mods.style = style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.exit.generic.type = MTYPE_PTEXT;
	s_main.exit.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.exit.generic.x = 320;
	s_main.exit.generic.y = y;
	s_main.exit.generic.id = ID_EXIT;
	s_main.exit.generic.callback = Main_MenuEvent;
	s_main.exit.string = "Exit";
	s_main.exit.color = color_red;
	s_main.exit.style = style;

	Menu_AddItem(&s_main.menu, &s_main.singleplayer);
	Menu_AddItem(&s_main.menu, &s_main.multiplayer);
	Menu_AddItem(&s_main.menu, &s_main.cinematics);
	Menu_AddItem(&s_main.menu, &s_main.demos);
	Menu_AddItem(&s_main.menu, &s_main.setup);
	Menu_AddItem(&s_main.menu, &s_main.mods);
	Menu_AddItem(&s_main.menu, &s_main.exit);

	trap_Key_SetCatcher(KEYCATCH_UI);

	uis.menusp = 0;

	UI_PushMenu(&s_main.menu);
}
