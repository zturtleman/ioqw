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

/*
=======================================================================================================================================

	START SERVER MENU HEADER (Q3 UI INTERFACE SPECIFIC)

=======================================================================================================================================
*/

/*
	This file should only contain info and data that is used to draw/update or interact with the Q3 user interface.
	Put UI INDEPENDENT stuff into ui_createserver.h
*/

#include "ui_createserver.h"

#define GAMESERVER_BACK0 "menu/art/back_0"
#define GAMESERVER_BACK1 "menu/art/back_1"
#define GAMESERVER_FIGHT0 "menu/art/fight_0"
#define GAMESERVER_FIGHT1 "menu/art/fight_1"
#define GAMESERVER_SERVER0 "menu/art/server_0"
#define GAMESERVER_SERVER1 "menu/art/server_1"
#define GAMESERVER_ITEMS0 "menu/art/items_0"
#define GAMESERVER_ITEMS1 "menu/art/items_1"
#define GAMESERVER_BOTS0 "menu/art/bots_0"
#define GAMESERVER_BOTS1 "menu/art/bots_1"
#define GAMESERVER_MAPS0 "menu/art/maps_0"
#define GAMESERVER_MAPS1 "menu/art/maps_1"
#define GAMESERVER_VARROWS "menu/art/arrows_vert_0"
#define GAMESERVER_UP "menu/art/arrows_vert_top"
#define GAMESERVER_DOWN "menu/art/arrows_vert_bot"
#define GAMESERVER_DEL0 "menu/art/del_0"
#define GAMESERVER_DEL1 "menu/art/del_1"
#define GAMESERVER_SELECTED0 "menu/art/switch_off"
#define GAMESERVER_SELECTED1 "menu/art/switch_on"
#define GAMESERVER_ACTION0 "menu/art/action0"
#define GAMESERVER_ACTION1 "menu/art/action1"

#define BUTTON_WIDTH 96
#define BUTTON_HEIGHT 48

#define ART_CREATE0 "menu/art/create_0"
#define ART_CREATE1 "menu/art/create_1"
#define ART_SPECIFY0 "menu/art/specify0"
#define ART_SPECIFY1 "menu/art/specify1"

#define ID_SINGLEPLAYER	26
#define ID_SERVERS		27
#define ID_CREATE		28
#define ID_SPECIFY		29

#define FRAME_LEFT "menu/art/frame2_l"
#define FRAME_RIGHT "menu/art/frame1_r"
#define FRAME_SPLASH "menu/art/cut_frame"
#define FRAME_EXCLUDED "menu/art/excluded"
// control id's are grouped for ease of allocation
// < 100 are common to all pages
#define ID_SERVERCOMMON_BACK	10
#define ID_SERVERCOMMON_FIGHT	11
#define ID_SERVERCOMMON_SERVER	12
#define ID_SERVERCOMMON_BOTS	13
#define ID_SERVERCOMMON_MAPS	14

#define COLUMN_LEFT (SMALLCHAR_WIDTH * 18)
#define COLUMN_RIGHT (320 + (SMALLCHAR_WIDTH * 20))

#define LINE_HEIGHT (SMALLCHAR_HEIGHT + 2)

#define GAMETYPECOLUMN_X 320
#define GAMETYPEICON_X (GAMETYPECOLUMN_X - (20 * SMALLCHAR_WIDTH))
#define GAMETYPEROW_Y (64 + (LINE_HEIGHT / 2))

#define MAPPAGE_TEXT 16
#define MAPSELECT_SELECT "menu/art/maps_select"
#define MAP_FADETIME 1000

#define TABCONTROLCENTER_Y (240 + LINE_HEIGHT)

typedef void (*CtrlCallback_t)(void *self, int event);
// all the controls common to menus
typedef struct commoncontrols_s {
	menubitmap_s singleplayer;
	menubitmap_s servers;
	menubitmap_s specify;
	menubitmap_s create;
	menubitmap_s back;
	menubitmap_s fight;
	menubitmap_s server;
	menubitmap_s items;
	menubitmap_s maps;
	menubitmap_s bots;
	menutext_s banner;
	menutext_s servertext;
	menutext_s itemtext;
	menutext_s maptext;
	menutext_s bottext;
} commoncontrols_t;

enum {
	COMMONCTRL_BOTS,
	COMMONCTRL_MAPS,
	COMMONCTRL_SERVER,
	COMMONCTRL_ITEMS
};
// global data
extern vec4_t pulsecolor;
extern vec4_t fading_red;
// declarations

// ui_createserver_botsel.c
void UI_BotSelect_Index(char *bot, int index);
// ui_createserver_mapsel.c
void UI_StartMapMenu(int gametype, int index, const char *mapname);
// ui_createserver_common.c
qboolean CreateServer_CheckFightReady(commoncontrols_t *c);
void CreateServer_CommonControls_Init(menuframework_s *, commoncontrols_t *, CtrlCallback_t, int);
//void CreateServer_BackgroundDraw(qboolean excluded);
void CreateServer_SelectionDraw(void *self);
// ui_createserver_map.c
void CreateServer_MapPage_MenuInit(void);
void CreateServer_MapPage_Cache(void);
void CreateServer_MapPage_CopyCustomLimitsToControls(void);
// ui_createserver_bot.c
void CreateServer_BotPage_MenuInit(void);
void CreateServer_BotPage_Cache(void);
// ui_createserver_server.c
void CreateServer_ServerPage_MenuInit(void);
void CreateServer_ServerPage_Cache(void);
// ui_createserver_item.c
void CreateServer_BothItemMenus_Cache(void);
void CreateServer_ItemPage_MenuInit(void);
// ui_createserver_item_old.c
void CreateServer_ItemPage_MenuInit_OldMenu(qboolean ingame);
