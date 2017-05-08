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
#include "ui_startserver_q3.h"

// misc data
// scratchpad colours, [3] may be any value
vec4_t pulsecolor = {1.0, 1.0, 1.0, 0.0};
vec4_t fading_red = {1.0, 0.0, 0.0, 0.0};

/*
=======================================================================================================================================
StartServer_CheckFightReady
=======================================================================================================================================
*/
qboolean StartServer_CheckFightReady(commoncontrols_t *c) {

	if (StartServer_CanFight()) {
		c->fight.generic.flags &= ~QMF_GRAYED;
		return qtrue;
	}

	c->fight.generic.flags |= QMF_GRAYED;
	return qfalse;
}

/*
=======================================================================================================================================
StartServer_BackgroundDraw
=======================================================================================================================================
*/
void StartServer_BackgroundDraw(qboolean excluded) {
	static vec4_t dim = {1.0, 1.0, 1.0, 0.5};

	trap_R_SetColor(dim);
	// UI_DrawNamedPic(0, y, w, h, FRAME_LEFT);
	UI_DrawNamedPic(-80, 40, 800, 400, FRAME_RIGHT);

	if (excluded) {
		dim[3] = 0.25;
		UI_DrawNamedPic(320 - 256, 240 - 64 - 32, 512, 256, FRAME_EXCLUDED);
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
StartServer_SelectionDraw
=======================================================================================================================================
*/
void StartServer_SelectionDraw(void *self) {
	float x, y, w, h, offset;
	qhandle_t shader;
	menubitmap_s *b;

	b = (menubitmap_s *)self;
	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h = b->height;
	// used to refresh shader
	if (b->generic.name && !b->shader) {
		b->shader = trap_R_RegisterShaderNoMip(b->generic.name);

		if (!b->shader && b->errorpic) {
			b->shader = trap_R_RegisterShaderNoMip(b->errorpic);
		}
	}

	if (b->focuspic && !b->focusshader) {
		b->focusshader = trap_R_RegisterShaderNoMip(b->focuspic);
	}

	if (b->generic.flags & QMF_HIGHLIGHT) {
		shader = b->focusshader;
	} else {
		shader = b->shader;
	}

	if (b->generic.flags & QMF_GRAYED) {
		if (shader) {
			trap_R_SetColor(colorMdGrey);
			UI_DrawHandlePic(x, y, w, h, shader);
			trap_R_SetColor(NULL);
		}
	} else {
		if ((b->generic.flags & QMF_PULSE) || ((b->generic.flags & QMF_PULSEIFFOCUS) && (UI_CursorInRect(x, y, w, h)))) {
			offset = 3 * sin(uis.realtime / PULSE_DIVISOR);
			UI_DrawHandlePic(x - offset, y - offset, w + 2 * offset, h + 2 * offset, shader);
		} else {
			UI_DrawHandlePic(x, y, w, h, shader);
		}
	}
}

/*
=======================================================================================================================================
StartServer_CommonControls_Cache
=======================================================================================================================================
*/
void StartServer_CommonControls_Cache(void) {

	trap_R_RegisterShaderNoMip(GAMESERVER_BACK0);
	trap_R_RegisterShaderNoMip(GAMESERVER_BACK1);
	trap_R_RegisterShaderNoMip(GAMESERVER_FIGHT0);
	trap_R_RegisterShaderNoMip(GAMESERVER_FIGHT1);
	trap_R_RegisterShaderNoMip(GAMESERVER_SERVER0);
	trap_R_RegisterShaderNoMip(GAMESERVER_SERVER1);
	trap_R_RegisterShaderNoMip(GAMESERVER_MAPS0);
	trap_R_RegisterShaderNoMip(GAMESERVER_MAPS1);
	trap_R_RegisterShaderNoMip(GAMESERVER_BOTS0);
	trap_R_RegisterShaderNoMip(GAMESERVER_BOTS1);
	//trap_R_RegisterShaderNoMip(FRAME_LEFT);
	trap_R_RegisterShaderNoMip(FRAME_RIGHT);
	trap_R_RegisterShaderNoMip(FRAME_EXCLUDED);
}

/*
=======================================================================================================================================
StartServer_CommonControls_Init
=======================================================================================================================================
*/
void StartServer_CommonControls_Init(menuframework_s *menuptr, commoncontrols_t *common, CtrlCallback_t callback, int ctrlpage) {

	StartServer_CommonControls_Cache();

	common->singleplayer.generic.type = MTYPE_BITMAP;
	common->singleplayer.generic.name = ART_SINGLEPLAYER0;
	common->singleplayer.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	common->singleplayer.generic.callback = callback;
	common->singleplayer.generic.id = ID_SINGLEPLAYER;
	common->singleplayer.generic.x = 0;
	common->singleplayer.generic.y = -5;
	common->singleplayer.width = BUTTON_WIDTH;
	common->singleplayer.height = BUTTON_HEIGHT;
	common->singleplayer.focuspic = ART_SINGLEPLAYER1;

	common->servers.generic.type = MTYPE_BITMAP;
	common->servers.generic.name = ART_SERVERS0;
	common->servers.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	common->servers.generic.callback = callback;
	common->servers.generic.id = ID_SERVERS;
	common->servers.generic.x = BUTTON_WIDTH + 4;
	common->servers.generic.y = -5;
	common->servers.width = BUTTON_WIDTH;
	common->servers.height = BUTTON_HEIGHT;
	common->servers.focuspic = ART_SERVERS1;

	common->specify.generic.type = MTYPE_BITMAP;
	common->specify.generic.name = ART_SPECIFY0;
	common->specify.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	common->specify.generic.callback = callback;
	common->specify.generic.id = ID_SPECIFY;
	common->specify.generic.x = BUTTON_WIDTH * 2 + 8;
	common->specify.generic.y = -5;
	common->specify.width = BUTTON_WIDTH;
	common->specify.height = BUTTON_HEIGHT;
	common->specify.focuspic = ART_SPECIFY1;

	common->create.generic.type = MTYPE_BITMAP;
	common->create.generic.name = ART_CREATE0;
	common->create.generic.flags = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT;
	common->create.generic.callback = callback;
	common->create.generic.id = ID_CREATE;
	common->create.generic.x = BUTTON_WIDTH * 5.5;
	common->create.generic.y = -5;
	common->create.width = BUTTON_WIDTH;
	common->create.height = BUTTON_HEIGHT;
	common->create.focuspic = ART_CREATE1;

	common->back.generic.type = MTYPE_BITMAP;
	common->back.generic.name = GAMESERVER_BACK0;
	common->back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	common->back.generic.callback = callback;
	common->back.generic.id = ID_SERVERCOMMON_BACK;
	common->back.generic.x = 0;
	common->back.generic.y = 435;
	common->back.width = BUTTON_WIDTH;
	common->back.height = BUTTON_HEIGHT;
	common->back.focuspic = GAMESERVER_BACK1;

	common->fight.generic.type = MTYPE_BITMAP;
	common->fight.generic.name = GAMESERVER_FIGHT0;
	common->fight.generic.flags = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	common->fight.generic.callback = callback;
	common->fight.generic.id = ID_SERVERCOMMON_FIGHT;
	common->fight.generic.x = 640;
	common->fight.generic.y = 435;
	common->fight.width = BUTTON_WIDTH;
	common->fight.height = BUTTON_HEIGHT;
	common->fight.focuspic = GAMESERVER_FIGHT1;

	common->maps.generic.type = MTYPE_BITMAP;
	common->maps.generic.name = GAMESERVER_MAPS0;
	common->maps.generic.callback = callback;
	common->maps.generic.id = ID_SERVERCOMMON_MAPS;
	common->maps.generic.x = 156;
	common->maps.generic.y = 435;
	common->maps.width = BUTTON_WIDTH;
	common->maps.height = BUTTON_HEIGHT;
	common->maps.focuspic = GAMESERVER_MAPS1;

	common->bots.generic.type = MTYPE_BITMAP;
	common->bots.generic.name = GAMESERVER_BOTS0;
	common->bots.generic.callback = callback;
	common->bots.generic.id = ID_SERVERCOMMON_BOTS;
	common->bots.generic.x = 252;
	common->bots.generic.y = 435;
	common->bots.width = BUTTON_WIDTH;
	common->bots.height = BUTTON_HEIGHT;
	common->bots.focuspic = GAMESERVER_BOTS1;

	common->server.generic.type = MTYPE_BITMAP;
	common->server.generic.name = GAMESERVER_SERVER0;
	common->server.generic.callback = callback;
	common->server.generic.id = ID_SERVERCOMMON_SERVER;
	common->server.generic.x = 348;
	common->server.generic.y = 435;
	common->server.width = BUTTON_WIDTH;
	common->server.height = BUTTON_HEIGHT;
	common->server.focuspic = GAMESERVER_SERVER1;
	// register controls
	Menu_AddItem(menuptr, &common->servers);
	Menu_AddItem(menuptr, &common->singleplayer);
	Menu_AddItem(menuptr, &common->specify);
	Menu_AddItem(menuptr, &common->create);
	Menu_AddItem(menuptr, &common->back);
	Menu_AddItem(menuptr, &common->fight);

	switch (ctrlpage) {
		case COMMONCTRL_BOTS:
			common->maps.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			common->bots.generic.flags = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT;
			common->server.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			break;
		case COMMONCTRL_MAPS:
			common->maps.generic.flags = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT;
			common->bots.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			common->server.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			break;
		case COMMONCTRL_SERVER:
			common->maps.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			common->bots.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
			common->server.generic.flags = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT;
		break;
	}

	Menu_AddItem(menuptr, &common->maps);
	Menu_AddItem(menuptr, &common->bots);
	Menu_AddItem(menuptr, &common->server);
}

/*
=======================================================================================================================================
StartServer_Cache
=======================================================================================================================================
*/
void StartServer_Cache(void) {

	StartServer_CommonControls_Cache();
	StartServer_ServerPage_Cache();
	StartServer_MapPage_Cache();
	StartServer_BotPage_Cache();
}

/*
=======================================================================================================================================
ServerOptions_Cache
=======================================================================================================================================
*/
void ServerOptions_Cache(void) {
}
