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
=======================================================================================================================================
*/

/*
=======================================================================================================================================

	MAP SELECT MENU

=======================================================================================================================================
*/

#include "ui_local.h"
#include "ui_createserver.h"

#define MAX_MAPSLIST 2048

#define MAP_ERRORPIC "menu/art/unknownmap"

#define MAPTYPE_MASTER_BEGIN	0
#define MAPTYPE_MASTER_END		1
#define MAPTYPE_CUSTOM_BEGIN	2
#define MAPTYPE_CUSTOM_END		3
#define MAPTYPE_ICONX 20
#define MAPTYPE_ICONY 20

#define TMP_BUFSIZE 64
#define GROUP_INDEX "[Index]"

const char *mapfilter_items[MAPFILTER_MAX + MAX_MAPTYPES + 1] = {
	"Off",		// MAPFILTER_OFF
	"Id",		// MAPFILTER_ID
	"Others",	// MAPFILTER_NONID
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0
};

typedef struct mapTypeList_s {
	int num_maptypes;
	int num_maps;
	int noBotsIndex;
	int type_offset[MAX_MAPTYPES][4];
	char mapTypeName[MAX_MAPTYPES][MAPNAME_BUFFER + 2];
	char mapGraphic[MAX_MAPTYPES][MAX_QPATH];
	char mapName[MAX_MAPSLIST][MAPNAME_BUFFER];
} mapTypeList_t;

static mapTypeList_t s_mapList;
static qboolean maplist_loaded = qfalse;
static vec4_t faded_color = {1.0, 1.0, 1.0, 1.0};
static vec4_t shadow_color = {0.0, 0.0, 0.0, 1.0};

static const char *maptype_icon[NUM_GAMETYPES] = {
	"menu/art/gt1",	// GT_FFA
	"menu/art/gt2",	// GT_TOURNAMENT
	"menu/art/gt3",	// GT_TEAM
	"menu/art/gt4",	// GT_CTF
	"menu/art/gt5",	// GT_1FCTF
	"menu/art/gt6",	// GT_OBELISK
	"menu/art/gt7"	// GT_HARVESTER
};

/*
=======================================================================================================================================
UI_BuildMapListByType

If list is NULL then we're just counting the number of maps that match gametype.

gametype == -1

Can be used externally to the ui_createserver.c subsystem.
=======================================================================================================================================
*/
int UI_BuildMapListByType(int *list, int listmax, int gametype, callbackMapList callback) {
	int count, i, nummaps, matchbits, gamebits;
	const char *info;

	count = 0;

	if (gametype == -1) {
		matchbits = (1 << GT_MAX_GAME_TYPE) - 1;
	} else {
		matchbits = 1 << gametype;
	}

	nummaps = UI_GetNumArenas();

	for (i = 0; i < nummaps; i++) {
		info = UI_GetArenaInfoByNumber(i);
		gamebits = GametypeBits(Info_ValueForKey(info, "type"));

		if (!(gamebits & matchbits)) {
			continue;
		}

		if (callback && !callback(info)) {
			continue;
		}
		// add to list, terminate if full
		count++;

		if (!list) {
			continue;
		}

		list[count - 1] = i;

		if (count == listmax) {
			break;
		}
	}

	return count;
}

/*
=======================================================================================================================================
UI_DefaultIconFromGameType
=======================================================================================================================================
*/
const char *UI_DefaultIconFromGameType(int gametype) {

	if (gametype < 0 || gametype > NUM_GAMETYPES) {
		trap_Print(va(S_COLOR_RED "Unknown gametype icon: %i\n", gametype));
		return NULL;
	}

	return maptype_icon[gametype_remap2[gametype]];
}

/*
=======================================================================================================================================
CreateServer_SetIconFromGameType

gametype < 0 clears the bitmap so nothing is drawn. Modifies path to icon to get high quality Id version if needed.
Note: these Id icons don't have the proper transparency behaviour.
=======================================================================================================================================
*/
void CreateServer_SetIconFromGameType(menubitmap_s *b, int gametype, qboolean custom) {
	const char *new_icon;

	if (!b) {
		return;
	}

	if (gametype < 0) {
		b->generic.name = 0;
		b->shader = 0;
		return;
	}

	new_icon = CreateServer_MapIconFromType(gametype, custom);

	if (new_icon != b->generic.name) {
		b->generic.name = new_icon;
		b->shader = 0;
	}
}

/*
=======================================================================================================================================
CreateServer_CreateMapType
=======================================================================================================================================
*/
static qboolean CreateServer_CreateMapType(const char *name, const char *graphic) {
	int i;
	qboolean duplicated;
	char *ptr;

	ptr = va("[%s]", name);
	duplicated = qfalse;

	for (i = 0; i < s_mapList.num_maptypes; i++) {
		if (!Q_stricmp(ptr, s_mapList.mapTypeName[i])) {
			duplicated = qtrue;
			break;
		}
	}

	if (duplicated || !Q_stricmp(ptr, GROUP_INDEX)) {
		return qfalse;
	}

	strcpy(s_mapList.mapTypeName[s_mapList.num_maptypes], ptr);
	strcpy(s_mapList.mapGraphic[s_mapList.num_maptypes], graphic);
	s_mapList.num_maptypes++;

	return qtrue;
}

/*
=======================================================================================================================================
CreateServer_LoadCustomMapData
=======================================================================================================================================
*/
static void CreateServer_LoadCustomMapData(const char *filename, qboolean merge) {
	qboolean indexgroup, groupfound, indexdone, groupused[MAX_MAPTYPES];
	char buf[TMP_BUFSIZE], *text_p, *token, *ptr;
	static char data[65000];
	int len, i, index, first, last;
	fileHandle_t file;

	// setup parameters
	for (i = 0; i < MAX_MAPTYPES; i++) {
		groupused[i] = qfalse;
	}

	if (merge) {
		first = MAPTYPE_CUSTOM_BEGIN;
		last = MAPTYPE_CUSTOM_END;
	} else {
		first = MAPTYPE_MASTER_BEGIN;
		last = MAPTYPE_MASTER_END;
	}
	// read the file
	len = trap_FS_FOpenFile(filename, &file, FS_READ);

	if (len <= 0) {
		return;
	}

	if (len >= (sizeof(data) - 1)) {
		Com_Printf("UI_LoadCustomMapData: %s larger than buffer\n", filename);
		return;
	}

	trap_FS_Read(data, len, file);
	trap_FS_FCloseFile(file);

	data[len] = 0;
	// parse the data file
	groupfound = qfalse;
	indexgroup = qfalse;
	indexdone = qfalse;
	index = -1;
	text_p = data;

	while (1) {
		token = COM_Parse(&text_p);

		if (!text_p) {
			break;
		}

		if (token[0] == '[') {
			// close previous group
			if (groupfound) {
				s_mapList.type_offset[index][last] = s_mapList.num_maps;
				groupused[index] = qtrue;
				groupfound = qfalse;
			}
			// check for "[name]" format
			ptr = strchr(token, ']');

			if (strchr(&token[1],'[') || !ptr || ptr[1]) {
				Com_Printf("(%s): has malformed group (%s)\n", filename, token);
				break;
			}
			// we have a valid "[name]" tokem
			if (indexgroup) {
				indexgroup = qfalse;
				indexdone = qtrue;
			}

			if (!indexgroup && !indexdone) {
				if (Q_stricmp(token, GROUP_INDEX)) {
					Com_Printf("(%s): must have %s group first\n", filename, GROUP_INDEX);
					break;
				}

				indexgroup = qtrue;
				continue;
			}
			// locate
			index = -1;

			for (i = 0; i < MAX_MAPTYPES; i++) {
				if (!Q_stricmp(token, s_mapList.mapTypeName[i])) {
					index = i;
					break;
				}
			}
			// failed to locate, or duplicated
			if (index == -1 || groupused[index] || !Q_stricmp(token, GROUP_INDEX)) {
				Com_Printf("(%s): %s ignored\n", filename, token);
				continue;
			}
			// have a valid value of index at this point
			groupfound = qtrue;
			s_mapList.type_offset[index][first] = s_mapList.num_maps;
			continue;
		}
		// not a new index header, so we treat token as a map name or description of index
		if (indexgroup) {
			// "name = graphic file" format
			ptr = strchr(token, '=');
			buf[0] = 0;

			if (ptr) {
				Q_strncpyz(buf, token, ptr - token + 1);

				if (!ptr[1]) {
					token = COM_Parse(&text_p);

					if (!text_p) {
						break;
					}
				} else {
					token = &ptr[1];
				}
			} else { // token is separated from equals by whitespace
				Q_strncpyz(buf, token, TMP_BUFSIZE);
				token = COM_Parse(&text_p);

				if (!text_p) {
					break;
				}

				if (token[0] != '=') {
					Com_Printf("(%s):(%s) requires = separator\n", filename, buf);
					break;
				}

				if (!token[1]) {
					token = COM_Parse(&text_p);

					if (!text_p) {
						break;
					}
				} else {
					token = &token[1];
				}
			}
			// found a type of map, with associated graphic
			if (!CreateServer_CreateMapType(buf, token)) {
				Com_Printf("(%s):(%s) duplicated, ignored\n", filename, buf);
			}

			continue;
		} else {
			if (!groupfound) {
				continue;
			}
			// map name, mustn't be too long
			if (strlen(token) >= MAPNAME_BUFFER - 1) {
				Com_Printf("(%s): mapname too long (%s)\n", filename, token);
				break;
			}

			strcpy(s_mapList.mapName[s_mapList.num_maps++], token);
		}
	}

	if (index != -1) {
		s_mapList.type_offset[index][last] = s_mapList.num_maps;
	}
}

/*
=======================================================================================================================================
CreateServer_MapSupportsBots
=======================================================================================================================================
*/
qboolean CreateServer_MapSupportsBots(const char *mapname) {
	int i, index, start, end;

	index = s_mapList.noBotsIndex;

	if (index < 0) {
		return qtrue;
	}

	start = s_mapList.type_offset[index][MAPTYPE_MASTER_BEGIN];
	end = s_mapList.type_offset[index][MAPTYPE_MASTER_END];

	for (i = start; i < end; i++) {
		if (!Q_stricmp(mapname, s_mapList.mapName[i])) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
CreateServer_AddMapType
=======================================================================================================================================
*/
static void CreateServer_AddMapType(mappic_t *mappic, int type) {
	int i;

	if (mappic->num_types == MAX_MAPTYPES) {
		return;
	}
	// prevent duplication
	for (i = 0; i < mappic->num_types; i++) {
		if (mappic->type[i] == type) {
			return;
		}
	}

	mappic->type[mappic->num_types++] = type;
}

/*
=======================================================================================================================================
CreateServer_InitMapPictureFromIndex
=======================================================================================================================================
*/
void CreateServer_InitMapPictureFromIndex(mappic_t *mappic, int index) {
	int i, j, tmp;
	const char *info;
	char mapname[MAPNAME_BUFFER];

	memset(mappic, 0, sizeof(mappic_t));

	if (!maplist_loaded) {
		UI_LoadMapTypeInfo();
	}

	info = UI_GetArenaInfoByNumber(index);
	mappic->gamebits = GametypeBits(Info_ValueForKey(info, "type"));

	Q_strncpyz(mapname, Info_ValueForKey(info, "map"), MAPNAME_BUFFER);
	// find map on master and custom lists
	for (i = 0; i < s_mapList.num_maps; i++) {
		if (Q_stricmp(mapname, s_mapList.mapName[i])) {
			continue;
		}
		// find map in list
		for (j = 0; j < s_mapList.num_maptypes; j++) {
			if (i >= s_mapList.type_offset[j][MAPTYPE_MASTER_BEGIN] && i < s_mapList.type_offset[j][MAPTYPE_MASTER_END]) {
				CreateServer_AddMapType(mappic, j);
				break;
			}

			if (i >= s_mapList.type_offset[j][MAPTYPE_CUSTOM_BEGIN] && i < s_mapList.type_offset[j][MAPTYPE_CUSTOM_END]) {
				CreateServer_AddMapType(mappic, j);
				break;
			}
		}
	}
	// sort the icons, so they're always displayed in the same order
	for (i = 0; i < mappic->num_types - 1; i++) {
		for (j = i + 1; j < mappic->num_types; j++) {
			if (mappic->type[j] < mappic->type[i]) {
				tmp = mappic->type[j];
				mappic->type[j] = mappic->type[i];
				mappic->type[i] = tmp;
			}
		}
	}
	// set the picture name, cache it
	strcpy(mappic->mapname, mapname);
	trap_R_RegisterShaderNoMip(va("levelshots/%s.tga", mapname));
}

/*
=======================================================================================================================================
CreateServer_InitMapPicture
=======================================================================================================================================
*/
void CreateServer_InitMapPicture(mappic_t *mappic, const char *mapname) {
	int i, nummaps;
	const char *info;

	if (!mapname || !*mapname) {
		return;
	}
	// verify existance of map
	nummaps = UI_GetNumArenas();

	for (i = 0; i < nummaps; i++) {
		info = UI_GetArenaInfoByNumber(i);

		if (!info) {
			continue;
		}

		if (Q_stricmp(Info_ValueForKey(info, "map"), mapname)) {
			continue;
		}

		CreateServer_InitMapPictureFromIndex(mappic, i);
		return;
	}
}

/*
=======================================================================================================================================
CreateServer_DrawMapPicture
=======================================================================================================================================
*/
void CreateServer_DrawMapPicture(int x, int y, int w, int h, mappic_t *mappic, vec4_t color) {
	qhandle_t hpic;
	int i, mapbits, colx, coly, icons;

	// load and draw map picture
	hpic = trap_R_RegisterShaderNoMip(va("levelshots/%s.tga", mappic->mapname));

	if (!hpic) {
		hpic = trap_R_RegisterShaderNoMip(MAP_ERRORPIC);
	}

	if (color) {
		trap_R_SetColor(color);
	}

	UI_DrawHandlePic(x, y, w, h, hpic);

	if (color) {
		faded_color[0] = color[0];
		faded_color[1] = color[1];
		faded_color[2] = color[2];
		faded_color[3] = color[3] * 0.75; // slight transparency
	} else {
		faded_color[0] = 1.0;
		faded_color[1] = 1.0;
		faded_color[2] = 1.0;
		faded_color[3] = 0.75; // slight transparency
	}

	shadow_color[3] = faded_color[3];
	// overlay the icons
	colx = 0;
	coly = 0;
	// built in game types first
	icons = ui_mapicons.integer;

	if (icons == MAPICONS_ALL) {
		for (i = 0; i < NUM_GAMETYPES; i++) { // only main icon
			if (i < GT_CTF) {
				mapbits = 1 << gametype_remap[i];

				if (!(mapbits & mappic->gamebits)) {
					continue;
				}

				hpic = trap_R_RegisterShaderNoMip(maptype_icon[i]);

				if (!hpic) {
					continue;
				}

				colx += MAPTYPE_ICONX;

				if (colx > w) {
					colx = MAPTYPE_ICONX;
					coly += MAPTYPE_ICONY;
				}

				trap_R_SetColor(shadow_color);
				UI_DrawHandlePic(x + w - colx + 1, y + coly + 1, MAPTYPE_ICONX, MAPTYPE_ICONY, hpic);

				trap_R_SetColor(faded_color);
				UI_DrawHandlePic(x + w - colx, y + coly, MAPTYPE_ICONX, MAPTYPE_ICONY, hpic);
			}
		}
	}

	if (icons != MAPICONS_NONE) {
		for (i = 0; i < mappic->num_types; i++) {
			hpic = trap_R_RegisterShaderNoMip(s_mapList.mapGraphic[mappic->type[i]]);

			if (!hpic) {
				continue;
			}

			colx += MAPTYPE_ICONX;

			if (colx > w) {
				colx = MAPTYPE_ICONX;
				coly += MAPTYPE_ICONY;
			}

			trap_R_SetColor(shadow_color);
			UI_DrawHandlePic(x + w - colx + 1, y + coly + 1, MAPTYPE_ICONX, MAPTYPE_ICONY, hpic);

			trap_R_SetColor(faded_color);
			UI_DrawHandlePic(x + w - colx, y + coly, MAPTYPE_ICONX, MAPTYPE_ICONY, hpic);
		}
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
CreateServer_LoadBotlessMaps

Creates a special group of maps that don't have .aas bot support files.
=======================================================================================================================================
*/
void CreateServer_LoadBotlessMaps(void) {
	int i, nummaps, len, index;
	const char *info;
	char mapname[MAPNAME_BUFFER];
	fileHandle_t file;

	CreateServer_CreateMapType("NoBots", "icons/noammo");

	index = s_mapList.num_maptypes - 1;
	s_mapList.noBotsIndex = index;
	s_mapList.type_offset[index][MAPTYPE_MASTER_BEGIN] = s_mapList.num_maps;
	nummaps = UI_GetNumArenas();

	for (i = 0; i < nummaps; i++) {
		info = UI_GetArenaInfoByNumber(i);

		if (!info) {
			continue;
		}

		Q_strncpyz(mapname, Info_ValueForKey(info, "map"), MAPNAME_BUFFER);
		len = trap_FS_FOpenFile(va("maps/%s.aas", mapname), &file, FS_READ);

		if (len >= 0) {
			trap_FS_FCloseFile(file);
			continue;
		}
		// no .aas file, so we can't play bots on this map
		strcpy(s_mapList.mapName[s_mapList.num_maps], mapname);
		s_mapList.num_maps++;
	}
	// update last map index
	s_mapList.type_offset[index][MAPTYPE_MASTER_END] = s_mapList.num_maps;
}

/*
=======================================================================================================================================
UI_LoadMapTypeInfo
=======================================================================================================================================
*/
void UI_LoadMapTypeInfo(void) {
	int i;

	if (maplist_loaded) {
		return;
	}
	// reset data
	memset(&s_mapList, 0, sizeof(mapTypeList_t));

	s_mapList.num_maptypes = 0;
	s_mapList.num_maps = 0;
	s_mapList.noBotsIndex = -1;
	// load all the maps that don't have bot route files
	//CreateServer_LoadBotlessMaps();
	// load data files
	CreateServer_LoadCustomMapData("mapdata.txt", qfalse);
	CreateServer_LoadCustomMapData("usermaps.txt", qtrue);
	// update map selection list so we can put custom maps on screen
	for (i = 0; i < s_mapList.num_maptypes; i++) {
		mapfilter_items[MAPFILTER_MAX + i] = s_mapList.mapTypeName[i];
		randommaptype_items[MAP_RND_MAX + i] = s_mapList.mapTypeName[i];
	}

	maplist_loaded = qtrue;
}

/*
=======================================================================================================================================
CreateServer_NumCustomMapTypes
=======================================================================================================================================
*/
int CreateServer_NumCustomMapTypes(void) {

	if (!maplist_loaded) {
		UI_LoadMapTypeInfo();
	}

	return s_mapList.num_maptypes;
}

/*
=======================================================================================================================================
CreateServer_MapIconFromType
=======================================================================================================================================
*/
const char *CreateServer_MapIconFromType(int gametype, qboolean isCustomMap) {

	if (!maplist_loaded) {
		UI_LoadMapTypeInfo();
	}

	if (isCustomMap) {
		if (gametype >= s_mapList.num_maptypes || gametype < 0) {
			return NULL;
		}

		return s_mapList.mapGraphic[gametype];
	} else {
		return UI_DefaultIconFromGameType(gametype);
	}
}

/*
=======================================================================================================================================
CreateServer_IsCustomMapType
=======================================================================================================================================
*/
qboolean CreateServer_IsCustomMapType(const char *mapname, int type) {
	int i, end;

	if (!maplist_loaded) {
		UI_LoadMapTypeInfo();
	}

	if (type >= s_mapList.num_maptypes || type < 0) {
		return qfalse;
	}

	end = s_mapList.type_offset[type][MAPTYPE_MASTER_END];

	for (i = s_mapList.type_offset[type][MAPTYPE_MASTER_BEGIN]; i < end; i++) {
		if (!Q_stricmp(mapname, s_mapList.mapName[i])) {
			return qtrue;
		}
	}

	end = s_mapList.type_offset[type][MAPTYPE_CUSTOM_END];

	for (i = s_mapList.type_offset[type][MAPTYPE_CUSTOM_BEGIN]; i < end; i++) {
		if (!Q_stricmp(mapname, s_mapList.mapName[i])) {
			return qtrue;
		}
	}

	return qfalse;
}
