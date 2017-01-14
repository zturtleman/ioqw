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
#include "ui_dynamicmenu.h"

/*
=======================================================================================================================================

	MAP VOTE SELECT MENU

=======================================================================================================================================
*/

#define BUTTON_WIDTH 96
#define BUTTON_HEIGHT 48

#define ART_BUTTONSBACKGROUND "textures/base_wall/redmet"
#define ART_BLACKBORDER "menu/art/blackborder"

#define MAPSELECT_HARROWS "menu/art/arrows_horz_0"
#define MAPSELECT_NEXT "menu/art/arrows_horz_right"
#define MAPSELECT_PREVIOUS "menu/art/arrows_horz_left"
#define MAPSELECT_CANCEL0 "menu/art/back_0"
#define MAPSELECT_CANCEL1 "menu/art/back_1"
#define MAPSELECT_VOTE0 "menu/ui_art/vote_0"
#define MAPSELECT_VOTE1 "menu/ui_art/vote_1"
#define MAPSELECT_MAPFOCUS "menu/ui_art/mapfocus"
#define MAPSELECT_MAPSELECTED "menu/art/maps_selected"

#define MAPNAME_LONGBUFFER 64

#define MAPSELECT_ERRORPIC "menu/art/unknownmap"

#define MAPPIC_WIDTH 120
#define MAPPIC_HEIGHT 80

enum {
	ID_MAPSELECT_CANCEL,
	ID_MAPSELECT_NEXT,
	ID_MAPSELECT_PREV,
	ID_MAPSELECT_OK,
	ID_MAPSELECT_ALLMAPS,
	ID_MAPSELECT_FILTERMAPS,
	ID_MAPSELECT_MAPICONS,
	ID_MAPSELECT_MULTISEL,
	ID_MAPSELECT_LISTVIEW
};

#define MAPGRID_ROWS 4
#define MAPGRID_COLUMNS 5
#define MAX_GRIDMAPSPERPAGE (MAPGRID_ROWS * MAPGRID_COLUMNS)

#define MAPLIST_ROWS 18
#define MAPLIST_COLUMNS 3
#define MAX_LISTMAPSPERPAGE (MAPLIST_ROWS * MAPLIST_COLUMNS)

#if (MAX_LISTMAPSPERPAGE > MAX_GRIDMAPSPERPAGE)
#define MAX_MAPSPERPAGE MAX_LISTMAPSPERPAGE
#else
#define MAX_MAPSPERPAGE MAX_GRIDMAPSPERPAGE
#endif

const char *mapicons_items2[] = {
	"All",		// MAPICONS_ALL
	"Custom",	// MAPICONS_CUSTOM
	"None",		// MAPICONS_NONE
	0
};

static int ms_lastgametype2 = -1;
static int ms_allmaps2 = 0;
static int ms_filter2 = MAPFILTER_OFF;
static vec4_t color_nomap2 = {0.75, 0.0, 0.0, 1.0};
int gametype, index;
const char *mapname;

typedef struct mapselect_s {
	menubitmap_s blackborder;
	menubitmap_s blackborder2;
	menubitmap_s buttonsbackground;
	menubitmap_s buttonsbackground2;
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s mappics[MAX_GRIDMAPSPERPAGE];
	menubitmap_s icona;
	menubitmap_s iconb;
	menubitmap_s arrows;
	menubitmap_s next;
	menubitmap_s previous;
	menubitmap_s cancel;
	menubitmap_s accept;
	menutext_s maptype;
	menuradiobutton_s allmaps;
	menulist_s filter;
	menulist_s mapicons;
	menuradiobutton_s listview;
	menulist_s maplist;
	int gametype;	// GT_* format
	int nummaps;
	int maxpages;
	int page;
	// index into index_maplist[], -1 => no selection, otherwise >= 0 and < nummaps
	int currentmap;
	int index;		// index of map to change
	qboolean nomaps;
	int maxMapsPerPage;
	mappic_t mapinfo[MAX_MAPSPERPAGE];
	const char *maplist_alias[MAX_LISTMAPSPERPAGE];
	float *maptext_color[MAX_MAPSPERPAGE];
	int index_maplist[MAX_MAPS_LIST];
	char maplongname[MAX_MAPS_LIST][MAPNAME_LONGBUFFER];
	char mapdrawname[MAX_MAPS_LIST][MAPNAME_LONGBUFFER];
	int mapsecondline[MAX_MAPS_LIST];
	int bottomrow_y;
} mapselect_t;

static mapselect_t s_mapselect2;

/*
=======================================================================================================================================
VoteMenu_Map_CellSize
=======================================================================================================================================
*/
static void VoteMenu_Map_CellSize(int *colh, int *colw) {

	// screen height - 2 buttons
	// colh rounded to multiple of 2 to reduce drawing "artifacts"
	*colw = 640 / MAPGRID_COLUMNS;
	*colh = ((480 - 2 * 64) / MAPGRID_ROWS) & 0xFE;
}

/*
=======================================================================================================================================
VoteMenu_Map_SetViewType
=======================================================================================================================================
*/
static void VoteMenu_Map_SetViewType(void) {

	if (s_mapselect2.listview.curvalue) {
		s_mapselect2.maxMapsPerPage = MAX_LISTMAPSPERPAGE;
	} else {
		s_mapselect2.maxMapsPerPage = MAX_GRIDMAPSPERPAGE;
	}

	s_mapselect2.maxpages = s_mapselect2.nummaps / s_mapselect2.maxMapsPerPage;

	if (s_mapselect2.nummaps % s_mapselect2.maxMapsPerPage) {
		s_mapselect2.maxpages++;
	}

	s_mapselect2.page = 0;

	if (s_mapselect2.currentmap >= 0) {
		s_mapselect2.page = s_mapselect2.currentmap / s_mapselect2.maxMapsPerPage;
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_Index
=======================================================================================================================================
*/
static int VoteMenu_Map_Index(const char *mapname) {
	int i;
	const char *info;

	// check for valid mapname
	if (!mapname || *mapname == '\0') {
		return -1;
	}
	// find the map
	for (i = 0; i < s_mapselect2.nummaps; i++) {
		info = UI_GetArenaInfoByNumber(s_mapselect2.index_maplist[i]);

		if (Q_stricmp(mapname, Info_ValueForKey(info, "map")) == 0) {
			return i;
		}
	}

	return -1;
}

/*
=======================================================================================================================================
VoteMenu_Map_SupportsGametype
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_SupportsGametype(const char *mapname) {
	int count, matchbits, gamebits, i;
	const char *info;
	char *arena_mapname;

	if (!mapname || !mapname[0]) {
		return qtrue;
	}

	count = UI_GetNumArenas();

	if (count > MAX_MAPS_LIST) {
		count = MAX_MAPS_LIST;
	}

	matchbits = 1 << s_mapselect2.gametype;

	if (s_mapselect2.gametype == GT_FFA) {
		matchbits |= (1 << GT_SINGLE_PLAYER);
	}

	for (i = 0; i < count; i++) {
		info = UI_GetArenaInfoByNumber(i);

		if (!info) {
			continue;
		}

		arena_mapname = Info_ValueForKey(info, "map");

		if (!arena_mapname || arena_mapname[0] == '\0') {
			continue;
		}

		gamebits = GametypeBits(Info_ValueForKey(info, "type"));

		if (!(gamebits & matchbits)) {
			continue;
		}

		if (Q_stricmp(mapname, arena_mapname) == 0) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
VoteMenu_Map_FilteredMap
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_FilteredMap(const char *mapname) {
	qboolean idmap;
	int type;

	if (s_mapselect2.filter.curvalue == MAPFILTER_OFF) {
		return qtrue;
	}
	// handle request for Id or non-Id map type
	if (s_mapselect2.filter.curvalue < MAPFILTER_MAX) {
		idmap = StartServer_IsIdMap(mapname);

		if (s_mapselect2.filter.curvalue == MAPFILTER_NONID) {
			if (idmap) {
				return qfalse;
			}

			return qtrue;
		}

		return idmap;
	}
	// check for specific map list
	type = s_mapselect2.filter.curvalue - MAPFILTER_MAX;
	return StartServer_IsCustomMapType(mapname, type);
}

/*
=======================================================================================================================================
VoteMenu_Map_SetMapTypeIcons
=======================================================================================================================================
*/
static void VoteMenu_Map_SetMapTypeIcons(void) {
	int gametype, customtype;
	menubitmap_s *icon_type, *icon_custom;

	icon_type = &s_mapselect2.icona;
	icon_custom = &s_mapselect2.iconb;
	// using all maps, so don't set gametype icon
	gametype = s_mapselect2.gametype;

	if (s_mapselect2.allmaps.curvalue) {
		gametype = -1;
	}
	// check for custom map icon, bump gametype icon to left if so
	customtype = s_mapselect2.filter.curvalue - MAPFILTER_MAX;

	if (customtype >= 0) {
		icon_custom = icon_type;
		icon_type = &s_mapselect2.iconb;
	}

	StartServer_SetIconFromGameType(icon_type, gametype, qfalse);
	StartServer_SetIconFromGameType(icon_custom, customtype, qtrue);
}

/*
=======================================================================================================================================
VoteMenu_Map_ValidateMapForLoad
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_ValidateMapForLoad(const char *info, int matchbits, qboolean cache) {
	int gamebits;
	const char *arena_mapname;

	// error check the map
	arena_mapname = Info_ValueForKey(info, "map");

	if (!arena_mapname || arena_mapname[0] == '\0') {
		return qfalse;
	}

	gamebits = GametypeBits(Info_ValueForKey(info, "type"));

	if (!(gamebits & matchbits) && !s_mapselect2.allmaps.curvalue) {
		return qfalse;
	}

	if (!VoteMenu_Map_FilteredMap(arena_mapname)) {
		return qfalse;
	}
	// cache map image
	if (cache) {
		trap_R_RegisterShaderNoMip(va("levelshots/%s.tga", arena_mapname));
	}

	return qtrue;
}

/*
=======================================================================================================================================
VoteMenu_Map_LoadMaps
=======================================================================================================================================
*/
static void VoteMenu_Map_LoadMaps(const char *mapname, qboolean cache) {
	int count, matchbits, i, j, nchars, lastspace, secondline, count2;
	const char *info;
	char *buf;

	count = UI_GetNumArenas();

	if (count > MAX_MAPS_LIST) {
		count = MAX_MAPS_LIST;
	}

	s_mapselect2.nummaps = 0;
	matchbits = 1 << s_mapselect2.gametype;

	if (s_mapselect2.gametype == GT_FFA) {
		matchbits |= (1 << GT_SINGLE_PLAYER);
	}

	for (i = 0; i < count; i++) {
		info = UI_GetArenaInfoByNumber(i);

		if (!info || !VoteMenu_Map_ValidateMapForLoad(info, matchbits, cache)) {
			if (!info) {
				trap_Print(va("Load Map error(%i)\n", i));
			}

			continue;
		}

		s_mapselect2.index_maplist[s_mapselect2.nummaps] = i;
		/*
		Q_strncpyz(s_mapselect2.maplongname[s_mapselect2.nummaps], Info_ValueForKey(info, "longname"), MAPNAME_LONGBUFFER);
		// convert the map long name into a name that spans(at most) 2 rows
		Q_strncpyz(s_mapselect2.mapdrawname[s_mapselect2.nummaps], s_mapselect2.maplongname[s_mapselect2.nummaps], MAPNAME_LONGBUFFER);
		*/
		buf = s_mapselect2.mapdrawname[s_mapselect2.nummaps];
		buf[MAPNAME_LONGBUFFER - 1] = '\0';
		nchars = (640 / (SMALLCHAR_WIDTH * MAPGRID_COLUMNS)) - 1;
		lastspace = 0;
		count2 = 0;
		secondline = 0;

		for (j = 0; j < MAPNAME_LONGBUFFER; j++) {
			if (buf[j] == '\0') {
				break;
			}

			if (buf[j] == ' ') {
				lastspace = j;
			}

			count2++;

			if ((count2 % nchars) == 0) {
				if (lastspace) {
					buf[lastspace] = '\0';
					count2 = j - lastspace;

					if (!secondline) {
						secondline = lastspace + 1;
					}

					lastspace = 0;
					continue;
				}
				// move always preserves buf[MAPNAME_LONGBUFFER - 1]
				memcpy(&buf[j + 1], &buf[j], MAPNAME_LONGBUFFER - j - 2);

				buf[j] = '\0';
				count2 = 0;

				if (!secondline) {
					secondline = j + 1;
				}
			}
		}

		s_mapselect2.mapsecondline[s_mapselect2.nummaps] = secondline;
		s_mapselect2.nummaps++;
	}
	// set up the correct map page
	s_mapselect2.currentmap = VoteMenu_Map_Index(mapname);

	VoteMenu_Map_SetViewType();
}

/*
=======================================================================================================================================
VoteMenu_Map_HighlightIfOnPage

Only used for grid display of maps.
=======================================================================================================================================
*/
static void VoteMenu_Map_HighlightIfOnPage(int index) {
	int i;

	i = index - s_mapselect2.page * MAX_GRIDMAPSPERPAGE;

	if (i >= 0 && i < MAX_GRIDMAPSPERPAGE) {
		s_mapselect2.mappics[i].generic.flags |= QMF_HIGHLIGHT;
		s_mapselect2.mappics[i].generic.flags &= ~QMF_PULSEIFFOCUS;
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_OnCurrentPage
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_OnCurrentPage(int index) {
	int base;

	base = s_mapselect2.page * s_mapselect2.maxMapsPerPage;

	if (index < base) {
		return qfalse;
	}

	if (index >= base + s_mapselect2.maxMapsPerPage) {
		return qfalse;
	}

	if (index >= s_mapselect2.nummaps) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
VoteMenu_Map_UpdateAcceptInterface
=======================================================================================================================================
*/
static void VoteMenu_Map_UpdateAcceptInterface(void) {

	if (s_mapselect2.currentmap == -1) {
		s_mapselect2.accept.generic.flags |= (QMF_GRAYED|QMF_INACTIVE);
	} else {
		s_mapselect2.accept.generic.flags &= ~(QMF_GRAYED|QMF_INACTIVE);
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_UpdateInterface
=======================================================================================================================================
*/
static void VoteMenu_Map_UpdateInterface(void) {
	int top, i;

	if (s_mapselect2.listview.curvalue) {
		for (i = 0; i < MAX_GRIDMAPSPERPAGE; i++) {
			s_mapselect2.mappics[i].generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
		}

		s_mapselect2.maplist.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	} else {
		// set attributes of buttons
		top = s_mapselect2.page * MAX_GRIDMAPSPERPAGE;

		for (i = 0; i < MAX_GRIDMAPSPERPAGE; i++) {
			if ((top + i) >= s_mapselect2.nummaps) {
				break;
			}

			s_mapselect2.mappics[i].generic.flags &= ~(QMF_HIGHLIGHT|QMF_INACTIVE|QMF_HIDDEN);
			s_mapselect2.mappics[i].generic.flags |= QMF_PULSEIFFOCUS;
		}

		for (; i < MAX_GRIDMAPSPERPAGE; i++) {
			s_mapselect2.mappics[i].generic.flags &= ~(QMF_HIGHLIGHT|QMF_PULSEIFFOCUS);
			s_mapselect2.mappics[i].generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
		}

		s_mapselect2.maplist.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);

		VoteMenu_Map_HighlightIfOnPage(s_mapselect2.currentmap);
	}

	VoteMenu_Map_UpdateAcceptInterface();
}

/*
=======================================================================================================================================
VoteMenu_Map_SetNewMapPics
=======================================================================================================================================
*/
static void VoteMenu_Map_SetNewMapPics(void) {
	int top, i, j;
	const char *info;

	// set map names and start with clear buttons
	top = s_mapselect2.page * s_mapselect2.maxMapsPerPage;

	for (i = 0; i < s_mapselect2.maxMapsPerPage; i++) {
		if ((top + i) >= s_mapselect2.nummaps) {
			break;
		}

		StartServer_InitMapPictureFromIndex(&s_mapselect2.mapinfo[i], s_mapselect2.index_maplist[top + i]);

		s_mapselect2.maptext_color[i] = color_orange;
		// check if map has been used before
		if (s_mapselect2.index < 0 || s_mapselect2.index >= MAX_NUMMAPS) {
			continue;
		}

		for (j = 0; j < s_scriptdata.map.num_maps; j++) {
			info = UI_GetArenaInfoByNumber(s_mapselect2.index_maplist[top + i]);

			if (Q_stricmp(Info_ValueForKey(info, "map"), s_scriptdata.map.data[j].shortName) == 0) {
				s_mapselect2.maptext_color[i] = color_red;
				break;
			}
		}
	}
	// clear any left-over grid buttons
	if (!s_mapselect2.listview.curvalue) {
		for (; i < MAX_GRIDMAPSPERPAGE; i++) {
			s_mapselect2.mappics[i].generic.name = 0;
			s_mapselect2.mappics[i].shader = 0;
		}
	}
	// no maps found
	if (!s_mapselect2.nummaps) {
		s_mapselect2.nomaps = qtrue;
		s_mapselect2.accept.generic.flags |= QMF_INACTIVE;
	} else {
		s_mapselect2.nomaps = qfalse;
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_SetNewListNames
=======================================================================================================================================
*/
static void VoteMenu_Map_SetNewListNames(void) {
	int base, i;

	base = s_mapselect2.page * s_mapselect2.maxMapsPerPage;
	s_mapselect2.maplist.numitems = 0;

	for (i = 0; i < s_mapselect2.maxMapsPerPage; i++) {
		if (base + i == s_mapselect2.nummaps) {
			break;
		}

		s_mapselect2.maplist_alias[i] = s_mapselect2.mapinfo[i].mapname;
		s_mapselect2.maplist.numitems++;
	}

	if (VoteMenu_Map_OnCurrentPage(s_mapselect2.currentmap)) {
		s_mapselect2.maplist.curvalue = s_mapselect2.currentmap % s_mapselect2.maxMapsPerPage;
	} else {
		s_mapselect2.maplist.curvalue = -1;
	}

	s_mapselect2.maplist.oldvalue = s_mapselect2.maplist.curvalue;
}

/*
=======================================================================================================================================
VoteMenu_Map_SetNewMapPage
=======================================================================================================================================
*/
static void VoteMenu_Map_SetNewMapPage(void) {

	VoteMenu_Map_SetNewMapPics();

	if (s_mapselect2.listview.curvalue) {
		VoteMenu_Map_SetNewListNames();
	}

	VoteMenu_Map_UpdateInterface();
}

/*
=======================================================================================================================================
VoteMenu_Map_FilterChanged
=======================================================================================================================================
*/
static void VoteMenu_Map_FilterChanged(void) {
	char mapname[MAPNAME_BUFFER];
	const char *info;

	if (s_mapselect2.currentmap >= 0) {
		info = UI_GetArenaInfoByNumber(s_mapselect2.index_maplist[s_mapselect2.currentmap]);
		Q_strncpyz(mapname, Info_ValueForKey(info, "map"), MAPNAME_BUFFER);
	} else {
		mapname[0] = '\0';
	}
	// handle muliple selections
	// try and keep as many as possible across filter changes
	// convert to arena index
	VoteMenu_Map_LoadMaps(mapname, qfalse);
	VoteMenu_Map_SetNewMapPage();
	VoteMenu_Map_SetMapTypeIcons();
}

/*
=======================================================================================================================================
VoteMenu_Map_CommitSelection
=======================================================================================================================================
*/
static void VoteMenu_Map_CommitSelection(void) {
	const char *info;

	/*
	index = s_mapselect2.page * LISTMAPS_PERPAGE + s_mapselect2.maps.curvalue;
	info = UI_GetArenaInfoByNumber(s_mapselect2.index[index]);
	*/
	info = UI_GetArenaInfoByNumber(s_mapselect2.index_maplist[s_mapselect2.currentmap]);

	UI_ForceMenuOff();
	trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote map %s\n", Info_ValueForKey(info, "map")));
}

/*
=======================================================================================================================================
VoteMenu_Map_Event
=======================================================================================================================================
*/
static void VoteMenu_Map_Event(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_MAPSELECT_CANCEL:
			UI_PopMenu();
			break;
		case ID_MAPSELECT_OK:
			ms_allmaps2 = s_mapselect2.allmaps.curvalue;
			ms_filter2 = s_mapselect2.filter.curvalue;
			VoteMenu_Map_CommitSelection();
			break;
		case ID_MAPSELECT_PREV:
			if (s_mapselect2.page > 0) {
				s_mapselect2.page--;
				VoteMenu_Map_SetNewMapPage();
			}

			break;
		case ID_MAPSELECT_NEXT:
			if (s_mapselect2.page < s_mapselect2.maxpages - 1) {
				s_mapselect2.page++;
				VoteMenu_Map_SetNewMapPage();
			}

			break;
		case ID_MAPSELECT_FILTERMAPS:
		case ID_MAPSELECT_ALLMAPS:
			VoteMenu_Map_FilterChanged();
			// very ugly but works, I couldn't do an infinite bucle
			if ((s_mapselect2.nomaps & s_mapselect2.filter.numitems) > s_mapselect2.filter.curvalue) {
				s_mapselect2.filter.curvalue++;
				VoteMenu_Map_FilterChanged();
			}

			if ((s_mapselect2.nomaps & s_mapselect2.filter.numitems) > s_mapselect2.filter.curvalue) {
				s_mapselect2.filter.curvalue++;
				VoteMenu_Map_FilterChanged();
			}

			if ((s_mapselect2.nomaps & s_mapselect2.filter.numitems) > s_mapselect2.filter.curvalue) {
				s_mapselect2.filter.curvalue++;
				VoteMenu_Map_FilterChanged();
			}

			if ((s_mapselect2.nomaps & s_mapselect2.filter.numitems) > s_mapselect2.filter.curvalue) {
				s_mapselect2.filter.curvalue++;
				VoteMenu_Map_FilterChanged();
			}

			if ((s_mapselect2.nomaps & s_mapselect2.filter.numitems) == s_mapselect2.filter.curvalue) {
				s_mapselect2.filter.curvalue = 0;
				VoteMenu_Map_FilterChanged();
			}

			break;
		case ID_MAPSELECT_LISTVIEW:
			trap_Cvar_SetValue("ui_map_list", s_mapselect2.listview.curvalue);
			VoteMenu_Map_SetViewType();
			VoteMenu_Map_SetNewMapPage();
			VoteMenu_Map_UpdateInterface();
			break;
		case ID_MAPSELECT_MAPICONS:
			trap_Cvar_SetValue("ui_mapicons", s_mapselect2.mapicons.curvalue);
			break;
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_CallVoteEvent
=======================================================================================================================================
*/
static void VoteMenu_Map_CallVoteEvent(void *ptr, int event) {
	int index;

	if (event != QM_ACTIVATED) {
		return;
	}

	index = (s_mapselect2.page * MAX_GRIDMAPSPERPAGE) + ((menucommon_s *)ptr)->id;
	s_mapselect2.currentmap = index;

	VoteMenu_Map_UpdateInterface();
}

/*
=======================================================================================================================================
VoteMenu_Map_DrawMapPic
=======================================================================================================================================
*/
static void VoteMenu_Map_DrawMapPic(void *self) {
	menubitmap_s *b;
	int hasfocus, n, x, y, w, secondline, offset, id, h;
	vec4_t tempcolor;
	float *color;

	b = (menubitmap_s *)self;
	n = 0;
	hasfocus = 0;

	if (b->focuspic && !b->focusshader) {
		b->focusshader = trap_R_RegisterShaderNoMip(b->focuspic);
	}
	// draw focus/highlight
	if (!(b->generic.flags & QMF_INACTIVE)) {
		x = b->generic.left;
		y = b->generic.top;
		w = b->generic.right - b->generic.left;
		h = b->generic.bottom - b->generic.top;
		hasfocus = ((b->generic.flags & QMF_PULSE) || (b->generic.flags & QMF_PULSEIFFOCUS && (Menu_ItemAtCursor(b->generic.parent) == b)));

		if (hasfocus) {
			if (b->focuscolor) {
				tempcolor[0] = b->focuscolor[0];
				tempcolor[1] = b->focuscolor[1];
				tempcolor[2] = b->focuscolor[2];
				color = tempcolor;
			} else {
				color = pulsecolor;
			}

			color[3] = 0.7 + 0.3 * sin(uis.realtime / PULSE_DIVISOR);

			trap_R_SetColor(color);
			UI_DrawHandlePic(x, y, w, h, b->focusshader);
			trap_R_SetColor(NULL);
		}
	}
	// draw image/text
	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h = b->height;
	id = b->generic.id;

	StartServer_DrawMapPicture(x, y, w, h, &s_mapselect2.mapinfo[id], NULL);

	if (b->generic.flags & QMF_HIGHLIGHT) {
		UI_DrawNamedPic(x, y, w, h, MAPSELECT_MAPSELECTED);
	}

	x = b->generic.x + b->width / 2;
	y = b->generic.y + b->height + 2;

	if (hasfocus) {
		tempcolor[0] = s_mapselect2.maptext_color[id][0];
		tempcolor[1] = s_mapselect2.maptext_color[id][1];
		tempcolor[2] = s_mapselect2.maptext_color[id][2];
		color = tempcolor;
		color[3] = 0.7 + 0.3 * sin(uis.realtime / PULSE_DIVISOR);
	} else {
		color = s_mapselect2.maptext_color[id];
	}

	secondline = s_mapselect2.mapsecondline[n];
	offset = LINE_HEIGHT;

	if (secondline) {
		offset += LINE_HEIGHT;
		UI_DrawString(x, y - LINE_HEIGHT, &s_mapselect2.mapdrawname[n][secondline], UI_CENTER|UI_SMALLFONT, color);
	}

	UI_DrawString(x, y - offset, s_mapselect2.mapdrawname[n], UI_CENTER|UI_SMALLFONT, color);
	// mapname
	UI_DrawString(x, y - 18, s_mapselect2.mapinfo[id].mapname, UI_CENTER|UI_SMALLFONT, color_white);
}

/*
=======================================================================================================================================
VoteMenu_Map_Cache
=======================================================================================================================================
*/
void VoteMenu_Map_Cache(void) {

	trap_R_RegisterShaderNoMip(MAPSELECT_HARROWS);
	trap_R_RegisterShaderNoMip(MAPSELECT_NEXT);
	trap_R_RegisterShaderNoMip(MAPSELECT_PREVIOUS);
	trap_R_RegisterShaderNoMip(MAPSELECT_CANCEL0);
	trap_R_RegisterShaderNoMip(MAPSELECT_CANCEL1);
	trap_R_RegisterShaderNoMip(MAPSELECT_VOTE0);
	trap_R_RegisterShaderNoMip(MAPSELECT_VOTE1);
	trap_R_RegisterShaderNoMip(MAPSELECT_ERRORPIC);
	trap_R_RegisterShaderNoMip(MAPSELECT_MAPFOCUS);
	trap_R_RegisterShaderNoMip(MAPSELECT_MAPSELECTED);
}

/*
=======================================================================================================================================
VoteMenu_Map_ScrollCharParams
=======================================================================================================================================
*/
static void VoteMenu_Map_ScrollCharParams(int *height, int *width, int *line) {

	*height = SMALLCHAR_HEIGHT;
	*width = SMALLCHAR_WIDTH;
	*line = SMALLCHAR_HEIGHT + 2;
}

/*
=======================================================================================================================================
VoteMenu_Map_ScrollListDraw
=======================================================================================================================================
*/
static void VoteMenu_Map_ScrollListDraw(void *ptr) {
	float *color;
	qboolean hasfocus;
	menulist_s *l;
	int charwidth, charheight, lineheight, column, index, base, i, x, y, u, map, offset, style;

	l = (menulist_s *)ptr;
	hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);

	VoteMenu_Map_ScrollCharParams(&charheight, &charwidth, &lineheight);

	x = l->generic.x;

	for (column = 0; column < l->columns; column++) {
		y = l->generic.y;
		base = l->top + column * l->height;

		for (i = base; i < base + l->height; i++) {
			if (i >= l->numitems) {
				break;
			}

			style = UI_SMALLFONT;
			color = s_mapselect2.maptext_color[i];

			if (i == l->curvalue) {
				u = x - 2;

				if (l->generic.flags & QMF_CENTER_JUSTIFY) {
					u -= (l->width * charwidth) / 2 + 1;
				}

				UI_FillRect(u, y, l->width * charwidth, lineheight, listbar_color);

				if (color != color_red) {
					color = text_color_highlight;
				}

				if (hasfocus) {
					style |= (UI_PULSE|UI_LEFT);
				} else {
					style |= UI_LEFT;
				}
			} else {
				style |= UI_LEFT|UI_INVERSE;
			}

			if (l->generic.flags & QMF_CENTER_JUSTIFY) {
				style |= UI_CENTER;
			}

			index = 0;
			map = i + s_mapselect2.page * s_mapselect2.maxMapsPerPage;

			if (index > 0) {
				offset = 0;

				if (index < 10) {
					offset += charwidth;
				}

				UI_DrawString(x + offset, y + (lineheight - charheight) / 2, va("%i", index), style, color_white);
			}

			UI_DrawString(x + 3 * charwidth, y + (lineheight - charheight) / 2, l->itemnames[i], style, color);

			y += lineheight;
		}

		x += (l->width + l->seperation) * charwidth;
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_ListIndexFromCursor
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_ListIndexFromCursor(int *pos) {
	menulist_s *l;
	int charheight, charwidth, lineheight, x, y, w, cursorx, cursory, column;

	l = &s_mapselect2.maplist;

	VoteMenu_Map_ScrollCharParams(&charheight, &charwidth, &lineheight);

	*pos = -1;
	// check scroll region
	x = l->generic.x;
	y = l->generic.y;
	w = ((l->width + l->seperation) * l->columns - l->seperation) * charwidth;

	if (l->generic.flags & QMF_CENTER_JUSTIFY) {
		x -= w / 2;
	}

	if (!UI_CursorInRect(x, y, w, l->height * lineheight - 1)) {
		return qfalse;
	}

	cursorx = (uis.cursorx - x) / charwidth;
	column = cursorx / (l->width + l->seperation);
	cursory = (uis.cursory - y) / lineheight;
	*pos = column * l->height + cursory;
	return qtrue;
}

/*
=======================================================================================================================================
VoteMenu_Map_DrawListMapPic

Draws the picture under cursor in the map selection listbox.
=======================================================================================================================================
*/
static void VoteMenu_Map_DrawListMapPic(void) {
	static int oldindex = 0;
	static int maptime = 0;
	int base, index, delta, x, y, colw, colh;
	vec4_t colour;

	colour[0] = 1.0;
	colour[1] = 1.0;
	colour[2] = 1.0;
	colour[3] = 1.0;

	base = s_mapselect2.page * s_mapselect2.maxMapsPerPage;
	// cursor is outside list, fade map
	if (VoteMenu_Map_ListIndexFromCursor(&index) && VoteMenu_Map_OnCurrentPage(base + index)) {
		maptime = uis.realtime;
		oldindex = index;
	} else {
		index = oldindex;
		delta = uis.realtime - maptime;

		if (delta >= MAP_FADETIME) {
			return;
		}

		colour[3] = 1.0 - (float)(delta) / MAP_FADETIME;
	}

	fading_red[3] = colour[3];
	// preview in list
	x = 640 - MAPPIC_WIDTH;
	y = s_mapselect2.bottomrow_y + 20 + 90;

	VoteMenu_Map_CellSize(&colh, &colw);

	trap_R_SetColor(fading_red);
	UI_DrawNamedPic(x - (colw - MAPPIC_WIDTH) / 2 - 4, y - 7, colw + 6, colh + 12, MAPSELECT_SELECT);
	trap_R_SetColor(NULL);

	StartServer_DrawMapPicture(x, y, MAPPIC_WIDTH, MAPPIC_HEIGHT, &s_mapselect2.mapinfo[index], colour);

	UI_DrawString(320, y + MAPPIC_HEIGHT + 8, s_mapselect2.maplongname[base + index], UI_CENTER|UI_SMALLFONT, colour);
}

/*
=======================================================================================================================================
VoteMenu_Map_Draw
=======================================================================================================================================
*/
static void VoteMenu_Map_Draw(void) {

	StartServer_BackgroundDraw(qfalse);
	// draw the controls
	Menu_Draw(&s_mapselect2.menu);

	if (s_mapselect2.nomaps) {
		UI_DrawProportionalString(320, 240 - 32, "NO MAPS FOUND", UI_CENTER, color_nomap2);
		return;
	}

	if (s_mapselect2.listview.curvalue) {
		VoteMenu_Map_DrawListMapPic();
	}
}

/*
=======================================================================================================================================
VoteMenu_Map_HandleListKey

Returns true if the list box accepts that key input.
=======================================================================================================================================
*/
static qboolean VoteMenu_Map_HandleListKey(int key, sfxHandle_t *psfx) {
	menulist_s *l;
	int index, sel;

	l = &s_mapselect2.maplist;

	switch (key) {
		case K_MOUSE1:
			if (l->generic.flags & QMF_HASMOUSEFOCUS) {
				// absorbed, silent sound effect
				*psfx = (menu_null_sound);

				if (!VoteMenu_Map_ListIndexFromCursor(&index)) {
					return qtrue;
				}

				if (l->top + index < l->numitems) {
					l->oldvalue = l->curvalue;
					l->curvalue = l->top + index;

					if (l->oldvalue != l->curvalue && l->generic.callback) {
						l->generic.callback(l, QM_GOTFOCUS);
					}

					sel = s_mapselect2.page * s_mapselect2.maxMapsPerPage;
					sel += l->curvalue;
					s_mapselect2.currentmap = sel;

					VoteMenu_Map_UpdateAcceptInterface();
					*psfx = (menu_move_sound);
				}
			}

			return qtrue;
		// keys that have the default action
		case K_ESCAPE:
			*psfx = Menu_DefaultKey(&s_mapselect2.menu, key);
			return qtrue;
		}

		return qfalse;
}

/*
=======================================================================================================================================
VoteMenu_Map_Key
=======================================================================================================================================
*/
static sfxHandle_t VoteMenu_Map_Key(int key) {
	menulist_s *l;
	sfxHandle_t sfx;

	l = (menulist_s *)Menu_ItemAtCursor(&s_mapselect2.menu);
	sfx = menu_null_sound;

	if (l == &s_mapselect2.maplist) {
		if (!VoteMenu_Map_HandleListKey(key, &sfx)) {
			return menu_buzz_sound;
		}
	} else {
		sfx = Menu_DefaultKey(&s_mapselect2.menu, key);
	}

	return sfx;
}

/*
=======================================================================================================================================
VoteMenu_Map_ScrollListInit
=======================================================================================================================================
*/
static void VoteMenu_Map_ScrollListInit(menulist_s *l) {
	int charheight, charwidth, lineheight, w;

	l->oldvalue = 0;
	l->curvalue = 0;
	l->top = 0;

	VoteMenu_Map_ScrollCharParams(&charheight, &charwidth, &lineheight);

	if (!l->columns) {
		l->columns = 1;
		l->seperation = 0;
	} else if (!l->seperation) {
		l->seperation = 3;
	}

	w = ((l->width + l->seperation) * l->columns - l->seperation) * charwidth;

	l->generic.left = l->generic.x;
	l->generic.top = l->generic.y;
	l->generic.right = l->generic.x + w;
	l->generic.bottom = l->generic.y + l->height * lineheight;

	if (l->generic.flags & QMF_CENTER_JUSTIFY) {
		l->generic.left -= w / 2;
		l->generic.right -= w / 2;
	}
}

/*
=======================================================================================================================================
UI_VoteMapMenuInternal
=======================================================================================================================================
*/
static void UI_VoteMapMenuInternal(int gametype, int index, const char *mapname) {
	int lastpage, i, x, y, top, colw, colh;

	lastpage = -1;

	if (ms_lastgametype2 == gametype) {
		lastpage = s_mapselect2.page;
	} else {
		ms_lastgametype2 = gametype;
	}

	memset(&s_mapselect2, 0, sizeof(s_mapselect2));

	s_mapselect2.gametype = gametype;
	s_mapselect2.menu.key = VoteMenu_Map_Key;

	VoteMenu_Map_Cache();

	if (gametype < GT_CTF && VoteMenu_Map_SupportsGametype(mapname)) {
		s_mapselect2.allmaps.curvalue = ms_allmaps2;
	} else {
		s_mapselect2.allmaps.curvalue = 0;
	}
	// change map filter if needed
	if (mapname && mapname[0]) {
		if (ms_filter2 < MAPFILTER_MAX) {
			if (StartServer_IsIdMap(mapname)) {
				if (ms_filter2 == MAPFILTER_NONID) {
					ms_filter2 = MAPFILTER_OFF;
				}
			} else {
				if (ms_filter2 == MAPFILTER_ID) {
					ms_filter2 = MAPFILTER_OFF;
				}
			}
		} else if (!StartServer_IsCustomMapType(mapname, ms_filter2 - MAPFILTER_MAX)) {
			ms_filter2 = MAPFILTER_OFF;
		}
	}

	s_mapselect2.filter.curvalue = ms_filter2;
	// remember previous map page
	if (s_mapselect2.currentmap == -1 && lastpage != -1) {
		s_mapselect2.page = lastpage;
	}

	s_mapselect2.index = index;
	s_mapselect2.menu.wrapAround = qtrue;
	s_mapselect2.menu.fullscreen = qtrue;
	s_mapselect2.menu.draw = VoteMenu_Map_Draw;

	s_mapselect2.buttonsbackground.generic.type = MTYPE_BITMAP;
	s_mapselect2.buttonsbackground.generic.name = ART_BUTTONSBACKGROUND;
	s_mapselect2.buttonsbackground.generic.flags = QMF_INACTIVE;
	s_mapselect2.buttonsbackground.generic.x = -150;
	s_mapselect2.buttonsbackground.generic.y = 0;
	s_mapselect2.buttonsbackground.width = 900;
	s_mapselect2.buttonsbackground.height = BUTTON_HEIGHT;

	s_mapselect2.buttonsbackground2.generic.type = MTYPE_BITMAP;
	s_mapselect2.buttonsbackground2.generic.name = ART_BUTTONSBACKGROUND;
	s_mapselect2.buttonsbackground2.generic.flags = QMF_INACTIVE;
	s_mapselect2.buttonsbackground2.generic.x = -150;
	s_mapselect2.buttonsbackground2.generic.y = 440;
	s_mapselect2.buttonsbackground2.width = 900;
	s_mapselect2.buttonsbackground2.height = BUTTON_HEIGHT;

	s_mapselect2.banner.generic.type = MTYPE_BTEXT;
	s_mapselect2.banner.generic.x = 160;
	s_mapselect2.banner.generic.y = 6;
	s_mapselect2.banner.string = "Map Select";
	s_mapselect2.banner.color = color_white;
	s_mapselect2.banner.style = UI_CENTER|UI_GIANTFONT;

	s_mapselect2.maptype.generic.type = MTYPE_TEXT;
	s_mapselect2.maptype.generic.x = 160;
	s_mapselect2.maptype.generic.y = 14 + 36;
	s_mapselect2.maptype.string = (char *)gametype_items[gametype_remap2[gametype]];
	s_mapselect2.maptype.color = color_white;
	s_mapselect2.maptype.style = UI_CENTER;

	s_mapselect2.icona.generic.type = MTYPE_BITMAP;
	s_mapselect2.icona.generic.flags = QMF_INACTIVE;
//	s_mapselect2.icona.generic.name = "menu/medals/medal_excellent";
	s_mapselect2.icona.generic.x = 420 - 32 - SMALLCHAR_WIDTH;
	s_mapselect2.icona.generic.y = 18 + (SMALLCHAR_HEIGHT - 32) / 2;
	s_mapselect2.icona.width = 32;
	s_mapselect2.icona.height = 32;

	s_mapselect2.iconb.generic.type = MTYPE_BITMAP;
	s_mapselect2.iconb.generic.flags = QMF_INACTIVE;
//	s_mapselect2.iconb.generic.name = "menu/medals/medal_victory";
	s_mapselect2.iconb.generic.x = 420 - 64 - SMALLCHAR_WIDTH;
	s_mapselect2.iconb.generic.y = 18 + (SMALLCHAR_HEIGHT - 32) / 2;
	s_mapselect2.iconb.width = 32;
	s_mapselect2.iconb.height = 32;

	s_mapselect2.filter.generic.type = MTYPE_SPINCONTROL;
	s_mapselect2.filter.generic.x = 420 + 8 * SMALLCHAR_WIDTH;
	s_mapselect2.filter.generic.y = 18;
	s_mapselect2.filter.generic.name = "Filter:";
	s_mapselect2.filter.generic.id = ID_MAPSELECT_FILTERMAPS;
	s_mapselect2.filter.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.filter.itemnames = mapfilter_items;

	s_mapselect2.allmaps.generic.type = MTYPE_RADIOBUTTON;
	s_mapselect2.allmaps.generic.x = 480 - 8 * SMALLCHAR_WIDTH;
	s_mapselect2.allmaps.generic.y = 28 + LINE_HEIGHT + 8;
	s_mapselect2.allmaps.generic.name = "All maps:";
	s_mapselect2.allmaps.generic.id = ID_MAPSELECT_ALLMAPS;
	s_mapselect2.allmaps.generic.callback = VoteMenu_Map_Event;

	s_mapselect2.mapicons.generic.type = MTYPE_SPINCONTROL;
	s_mapselect2.mapicons.generic.x = 480 + 8 * SMALLCHAR_WIDTH;
	s_mapselect2.mapicons.generic.y = 28 + LINE_HEIGHT + 8;
	s_mapselect2.mapicons.generic.name = "Icons:";
	s_mapselect2.mapicons.generic.id = ID_MAPSELECT_MAPICONS;
	s_mapselect2.mapicons.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.mapicons.itemnames = mapicons_items2;
	s_mapselect2.mapicons.curvalue = (int)Com_Clamp(0, MAPICONS_MAX - 1, ui_mapicons.integer);

	s_mapselect2.arrows.generic.type = MTYPE_BITMAP;
	s_mapselect2.arrows.generic.name = MAPSELECT_HARROWS;
	s_mapselect2.arrows.generic.flags = QMF_INACTIVE;
	s_mapselect2.arrows.generic.x = 320;
	s_mapselect2.arrows.generic.y = 435;
	s_mapselect2.arrows.width = 192;
	s_mapselect2.arrows.height = 64;

	s_mapselect2.previous.generic.type = MTYPE_BITMAP;
	s_mapselect2.previous.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_mapselect2.previous.generic.id = ID_MAPSELECT_PREV;
	s_mapselect2.previous.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.previous.generic.x = 320;
	s_mapselect2.previous.generic.y = 435;
	s_mapselect2.previous.width = 96;
	s_mapselect2.previous.height = 64;
	s_mapselect2.previous.focuspic = MAPSELECT_PREVIOUS;

	s_mapselect2.next.generic.type = MTYPE_BITMAP;
	s_mapselect2.next.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_mapselect2.next.generic.id = ID_MAPSELECT_NEXT;
	s_mapselect2.next.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.next.generic.x = 320 + 96;
	s_mapselect2.next.generic.y = 435;
	s_mapselect2.next.width = 96;
	s_mapselect2.next.height = 64;
	s_mapselect2.next.focuspic = MAPSELECT_NEXT;

	s_mapselect2.cancel.generic.type = MTYPE_BITMAP;
	s_mapselect2.cancel.generic.name = MAPSELECT_CANCEL0;
	s_mapselect2.cancel.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_mapselect2.cancel.generic.id = ID_MAPSELECT_CANCEL;
	s_mapselect2.cancel.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.cancel.generic.x = 0;
	s_mapselect2.cancel.generic.y = 435;
	s_mapselect2.cancel.width = BUTTON_WIDTH;
	s_mapselect2.cancel.height = BUTTON_HEIGHT;
	s_mapselect2.cancel.focuspic = MAPSELECT_CANCEL1;

	s_mapselect2.accept.generic.type = MTYPE_BITMAP;
	s_mapselect2.accept.generic.name = MAPSELECT_VOTE0;
	s_mapselect2.accept.generic.flags = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_mapselect2.accept.generic.id = ID_MAPSELECT_OK;
	s_mapselect2.accept.generic.callback = VoteMenu_Map_Event;
	s_mapselect2.accept.generic.x = 640;
	s_mapselect2.accept.generic.y = 435;
	s_mapselect2.accept.width = BUTTON_WIDTH;
	s_mapselect2.accept.height = BUTTON_HEIGHT;
	s_mapselect2.accept.focuspic = MAPSELECT_VOTE1;

	s_mapselect2.maplist.generic.type = MTYPE_SCROLLLIST;
	s_mapselect2.maplist.generic.flags = QMF_PULSEIFFOCUS|QMF_NODEFAULTINIT;
	s_mapselect2.maplist.generic.x = 3 * SMALLCHAR_WIDTH;
	s_mapselect2.maplist.generic.y = 84;
	s_mapselect2.maplist.generic.ownerdraw = VoteMenu_Map_ScrollListDraw;
	s_mapselect2.maplist.columns = MAPLIST_COLUMNS;
	s_mapselect2.maplist.seperation = 2;
	s_mapselect2.maplist.height = MAPLIST_ROWS;
	s_mapselect2.maplist.width = 22;
	s_mapselect2.maplist.itemnames = s_mapselect2.maplist_alias;

	VoteMenu_Map_ScrollListInit(&s_mapselect2.maplist);

	top = s_mapselect2.page * s_mapselect2.maxMapsPerPage;

	VoteMenu_Map_CellSize(&colh, &colw);

	s_mapselect2.bottomrow_y = 64 + 2 * colh;

	for (i = 0; i < MAX_GRIDMAPSPERPAGE; i++) {
		x = colw * (i % MAPGRID_COLUMNS) + (colw - MAPPIC_WIDTH) / 2;
		y = 84 + (i / MAPGRID_COLUMNS) * colh; // offset by one button

		s_mapselect2.mappics[i].generic.type = MTYPE_BITMAP;
		s_mapselect2.mappics[i].generic.name = 0;
		s_mapselect2.mappics[i].generic.flags = QMF_NODEFAULTINIT;
		s_mapselect2.mappics[i].generic.ownerdraw = VoteMenu_Map_DrawMapPic;
		s_mapselect2.mappics[i].generic.callback = VoteMenu_Map_CallVoteEvent;
		s_mapselect2.mappics[i].generic.id = i;
		s_mapselect2.mappics[i].generic.x = x;
		s_mapselect2.mappics[i].generic.y = y;
		s_mapselect2.mappics[i].width = MAPPIC_WIDTH;
		s_mapselect2.mappics[i].height = MAPPIC_HEIGHT;
		s_mapselect2.mappics[i].focuspic = MAPSELECT_MAPSELECTED;
		s_mapselect2.mappics[i].errorpic = MAPSELECT_ERRORPIC;
		s_mapselect2.mappics[i].generic.left = x - (colw - MAPPIC_WIDTH) / 2 - 5;
		s_mapselect2.mappics[i].generic.top = y - 7;
		s_mapselect2.mappics[i].generic.right = x + (colw + MAPPIC_WIDTH) / 2 + 6;
		s_mapselect2.mappics[i].generic.bottom = y + colh - 8 + 12;
		s_mapselect2.mappics[i].focuspic = MAPSELECT_MAPFOCUS;
	}

	s_mapselect2.listview.generic.type = MTYPE_RADIOBUTTON;
	s_mapselect2.listview.generic.x = 128 + 15 * SMALLCHAR_WIDTH;
	s_mapselect2.listview.generic.y = 485 - 2 * SMALLCHAR_HEIGHT;
	s_mapselect2.listview.generic.name = "List view:";
	s_mapselect2.listview.generic.id = ID_MAPSELECT_LISTVIEW;
	s_mapselect2.listview.generic.callback = VoteMenu_Map_Event;

	s_mapselect2.listview.curvalue = (int)Com_Clamp(0, 1, trap_Cvar_VariableValue("ui_map_list"));
	// register for display
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.buttonsbackground);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.buttonsbackground2);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.banner);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.arrows);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.previous);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.next);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.cancel);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.accept);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.maptype);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.filter);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.mapicons);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.icona);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.iconb);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.listview);
	Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.maplist);

	if (gametype < GT_CTF) {
		Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.allmaps);
	}

	for (i = 0; i < MAX_GRIDMAPSPERPAGE; i++) {
		Menu_AddItem(&s_mapselect2.menu, &s_mapselect2.mappics[i]);
	}

	VoteMenu_Map_LoadMaps(mapname, qfalse);
	VoteMenu_Map_SetNewMapPage();
	VoteMenu_Map_SetMapTypeIcons();

	UI_PushMenu(&s_mapselect2.menu);
}

/*
=======================================================================================================================================
UI_VoteMapMenu
=======================================================================================================================================
*/
void UI_VoteMapMenu(void) {

	gametype = UI_ServerGametype();

	UI_LoadMapTypeInfo();

	UI_VoteMapMenuInternal(gametype, index, mapname);
}
