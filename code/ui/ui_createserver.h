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

	CREATE SERVER MENU HEADER (Q3 INTERFACE INDEPENDENT DEFINTIONS/DATA)

=======================================================================================================================================
*/

#define MAX_GAME_TYPE GT_MAX_GAME_TYPE

#define NUM_GAMETYPES 7

/*
=======================================================================================================================================

	MAP DATA

=======================================================================================================================================
*/

#define MAX_MAPPAGES 2
#define NUMMAPS_PERPAGE 16
#define MAX_NUMMAPS (MAX_MAPPAGES * NUMMAPS_PERPAGE)
#define MAX_LIMIT_BUF 4
#define SHORTMAP_BUFFER 16
#define LONGMAP_BUFFER 32
#define MAPNAME_BUFFER 16
#define MAX_MAPTYPES 32
#define MAX_MAPS_LIST 512
// indicies for counting map types
enum {
	MAP_GROUP_ID,
	MAP_GROUP_NONID,
	MAX_MAP_GROUP
};

enum mapSource {
	MAP_MS_ORDER,
	MAP_MS_RANDOMLIST,
	MAP_MS_RANDOM,
	MAP_MS_RANDOMEXCLUDE,
	MAP_MS_MAX
};

enum randomMapType {
	MAP_RND_ANY,
	MAP_RND_ID,
	MAP_RND_NONID,
	MAP_RND_BIASID,
	MAP_RND_BIASNONID,
	MAP_RND_MAX
};

enum enumLimitType {
	MAP_LT_DEFAULT,
	MAP_LT_NONE,
	MAP_LT_CUSTOM,
	MAP_LT_COUNT
};

enum mapCopyFrom {
	MAP_CF_ARENASCRIPT,
	MAP_CF_TIME,
	MAP_CF_FRAG,
	MAP_CF_BOTH,
	MAP_CF_CLEARALL,
	MAP_CF_CLEARPAGE,
	MAP_CF_COUNT
};

enum mapCopyTo {
	MAP_CT_SELECTED,
	MAP_CT_PAGE,
	MAP_CT_ALL,
	MAP_CT_COUNT
};
// this is used on the map chooser, but is referenced here so that data on the map page can be initialized
enum {
	MAPFILTER_OFF,
	MAPFILTER_ID,
	MAPFILTER_NONID,
	MAPFILTER_MAX
};

typedef struct mapdata_s {
	char shortName[SHORTMAP_BUFFER];
	char longName[LONGMAP_BUFFER];
	char fragLimit[MAX_LIMIT_BUF];
	char timeLimit[MAX_LIMIT_BUF];
} mapdata_t;
// data used by the map page
typedef struct mapparameters_s {
	int num_maps;
	mapdata_t data[MAX_NUMMAPS];
	int listSource;
	// values applied to all maps
	int fragLimit; // also doubles as caplimit
	int timeLimit;
	int fragLimitType;
	int timeLimitType;
	qboolean Repeat;
	int SourceCount;
	int SourceType;
	int TypeCount[MAX_GAME_TYPE][MAX_MAP_GROUP];
} mapparameters_t;

// map data manipulation functions

// map data based on list/exclude type
void CreateServer_LoadMapList(void);
void CreateServer_SaveMapList(void);
void CreateServer_RefreshMapNames(void);
// map list manipulation
void CreateServer_StoreMap(int pos, int arena);
void CreateServer_InsertMap(int pos, int arena);
void CreateServer_DeleteMap(int index);
qboolean CreateServer_SwapMaps(int from, int to);
void CreateServer_MapDoAction(int src, int dest, int page, int selected);
// map info
int CreateServer_MapPageCount(void);
int CreateServer_RangeClipMapIndex(int index);
qboolean CreateServer_IsRandomGeneratedMapList(int type);
qboolean CreateServer_IsIdMap(const char *mapname);

/*
=======================================================================================================================================

	BOT DATA

=======================================================================================================================================
*/

#define PLAYER_SLOTS 16
#define PLAYER_SLOTS_PERCOL (PLAYER_SLOTS / 2)

#define MAX_SKILL 5
// note restrictions on ordering below
// see CreateServer_BotPage_Event(), case ID_BOT_TYPE
enum {
	BOTSKILL_SAME,			// must be first non-custom value
	BOTSKILL_RANGE,			// must be last non-custom value
	BOTSKILL_CUSTOMSINGLE,	// display bot skill controls at this value and higher. Must be first CUSTOM value
	BOTSKILL_CUSTOMRANGE,
	BOTSKILL_COUNT
};

enum {
	BOTTYPE_SELECT,
	BOTTYPE_RANDOM,
	BOTTYPE_RANDOMEXCLUDE,
	BOTTYPE_RANDOMARENASCRIPT,
	BOTTYPE_ARENASCRIPT,
	BOTTYPE_SELECTARENASCRIPT,
	BOTTYPE_MAX
};

enum {
	SKILLBIAS_NONE,
	SKILLBIAS_VLOW,
	SKILLBIAS_LOW,
	SKILLBIAS_HIGH,
	SKILLBIAS_VHIGH,
	SKILLBIAS_FRACTIONAL,
	SKILLBIAS_FRAC_VLOW,
	SKILLBIAS_FRAC_LOW,
	SKILLBIAS_FRAC_HIGH,
	SKILLBIAS_FRAC_VHIGH,
	SKILLBIAS_COUNT
};

enum {
	SLOTTYPE_EMPTY,
	SLOTTYPE_HUMAN,
	SLOTTYPE_BOT,
	SLOTTYPE_OPEN,
	SLOTTYPE_COUNT
};

enum {
	SLOTTEAM_INVALID,
	SLOTTEAM_NONE,
	SLOTTEAM_ONE,
	SLOTTEAM_TWO
};

enum {
	BOTCHANGE_NEVER,
	BOTCHANGE_EVERYMAP,
	BOTCHANGE_MAP2,
	BOTCHANGE_MAP3,
	BOTCHANGE_MAP4,
	BOTCHANGE_COUNT
};

enum botCopyTo {
	BOT_CT_CLEARALL,
	BOT_CT_INDIV_SELECTED,
	BOT_CT_RANGE_SELECTED,
	BOT_CT_INDIV_ALL,
	BOT_CT_RANGE_ALL,
	BOT_CT_NEATEN,
	BOT_CT_COUNT
};
// bot skill control data
typedef struct botskill_s {
	int low;
	int high;
	int value;
	qboolean range;
} botskill_t;
// data used by the bot page
typedef struct botparameters_s {
	char name[PLAYER_SLOTS][MAX_NAME_LENGTH];
	botskill_t skill[PLAYER_SLOTS];
	qboolean drawName[PLAYER_SLOTS];
	int slotType[PLAYER_SLOTS];
	// way in which bots are selected
	int typeSelect;
	int numberOpen;
	int joinAs;
	int numberBots;
	int changeBots;
	int skillType;
	int skillBias;
	botskill_t globalSkill;
	qboolean teamSwapped;
} botparameters_t;
// bot data manipulation functions

// bot list based on list/exclude type
void CreateServer_LoadBotNameList(int type);
void CreateServer_SaveBotNameList(void);
// bot list manipulation
qboolean CreateServer_InsertBotSlot(int index);
qboolean CreateServer_DeleteBotSlot(int index);
void CreateServer_MoveBotToOtherTeam(int selected);
void CreateServer_DoBotAction(int action, int selected);
void CreateServer_SetBotSkillRangeType(int skill);
void CreateServer_SetNamedBot(int index, char *name);
void CreateServer_InsertNamedBot(int index, char *name);
void CreateServer_BotNameDrawn(int index, qboolean drawn);
// bot info
int CreateServer_ValidBotCount(void);
qboolean CreateServer_BotOnSelectionList(const char *checkName);
void CreateServer_ValidateBotSlotCount(int bots, int open);
int CreateServer_SlotTeam(int index); // returns SLOTTEAM_* values
// list type info
qboolean CreateServer_IsRandomBotList(int type);
qboolean CreateServer_IsBotArenaScript(int type);
qboolean CreateServer_IsRandomBotExclude(int type);

/*
=======================================================================================================================================

	ITEM DATA

=======================================================================================================================================
*/

// must match order in server_itemlist[]
enum {
	ITEM_GAUNTLET,
	ITEM_HANDGUN,
	ITEM_MACHINEGUN,
	ITEM_HEAVY_MACHINEGUN,
	ITEM_CHAINGUN,
	ITEM_SHOTGUN,
	ITEM_NAILGUN,
	ITEM_PHOSPHORGUN,
	ITEM_PROXLAUNCHER,
	ITEM_GRENADELAUNCHER,
	ITEM_NAPALMLAUNCHER,
	ITEM_ROCKETLAUNCHER,
	ITEM_LIGHTNING,
	ITEM_RAILGUN,
	ITEM_PLASMAGUN,
	ITEM_BFG,
	ITEM_MISSILELAUNCHER,
	ITEM_HANDGUN_AMMO,
	ITEM_BULLETS,
	ITEM_HMG_BULLETS,
	ITEM_BELT,
	ITEM_SHELLS,
	ITEM_NAILS,
	ITEM_CAPSULES,
	ITEM_MINES,
	ITEM_GRENADES,
	ITEM_CANISTERS,
	ITEM_ROCKETS,
	ITEM_LIGHTNING_AMMO,
	ITEM_SLUGS,
	ITEM_CELLS,
	ITEM_BFG_AMMO,
	ITEM_MISSILES,
	ITEM_HEALTHSMALL,
	ITEM_HEALTH,
	ITEM_HEALTHLARGE,
	ITEM_HEALTHMEGA,
	ITEM_ARMORSHARD,
	ITEM_ARMORCOMBAT,
	ITEM_ARMORBODY,
	ITEM_ARMORFULL,
	ITEM_KAMIKAZE,
	ITEM_QUAD,
	ITEM_INVIS,
	ITEM_REGEN,
	ITEM_AMMOREGEN,
	ITEM_GUARD,
	ITEM_DOUBLER,
	ITEM_SCOUT,
	ITEM_COUNT // total number of items
};
// item group types, each has a master override that forces all on, all off, or custom
enum {
	ITEMGROUP_WEAPON,
	ITEMGROUP_AMMO,
	ITEMGROUP_HEALTH,
	ITEMGROUP_ARMOR,
	ITEMGROUP_HOLDABLE,
	ITEMGROUP_POWERUPS,
	ITEMGROUP_PERSISTANT_POWERUPS,
	ITEMGROUP_COUNT
};
// possible values group master control
enum {
	ALLGROUPS_ENABLED,
	ALLGROUPS_CUSTOM,
	ALLGROUPS_HIDDEN
};

typedef struct itemnode_s {
	int groupid;
	int ident; // index of item in server_itemlist[]
	const char *mapitem;
	const char *shortitem;
} itemnode_t;

extern itemnode_t server_itemlist[ITEM_COUNT];

typedef struct itemparameters_s {
	int groupstate[ITEMGROUP_COUNT];
	qboolean enabled[ITEM_COUNT];
} itemparameters_t;
// item data manipulation functions

// item list based on gametype
void CreateServer_LoadDisabledItems(void);

/*
=======================================================================================================================================

	SERVER DATA

=======================================================================================================================================
*/

enum {
	SRVDED_OFF,
	SRVDED_LAN,
	SRVDED_INTERNET
};

#define MAX_HOSTNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 24

typedef struct serverparameters_s {
	char hostname[MAX_HOSTNAME_LENGTH];
	int smoothclients;
	int allowmaxrate;
	int maxrate;
	int allowdownload;
	int allowvote;
	int allowpass;
	char password[MAX_PASSWORD_LENGTH];
	int forceRespawn;
	int friendlyFire;
	int teamBalance;
	int autoJoin;
	int syncClients;
	int allowMinPing;
	int minPing;
	int allowMaxPing;
	int maxPing;
	int pureServer;
	int preventConfigBug;
	int dedicatedServer;
	int inactivityTime;
	int gravity;
	int knockback;
	int quadfactor;
	int allowPrivateClients;
	int privateClients;
	char privatePassword[MAX_PASSWORD_LENGTH];
	int sv_fps;
	int weaponrespawn;
	int netport;
	int strictAuth;
	int lanForceRate;
} serverparameters_t;

/*
=======================================================================================================================================

	ALL SCRIPT DATA

=======================================================================================================================================
*/

typedef struct scriptdata_s {
	// general date, used by all pages
	int gametype; // stored as GT_*
	// controls the setting up some params in a server script
	qboolean multiplayer;
	// specific groups of related parameters
	mapparameters_t map;
	botparameters_t bot;
	itemparameters_t item;
	serverparameters_t server;
} scriptdata_t;

extern scriptdata_t s_scriptdata;
// global functions

// ui_createserver_data.c
qboolean CreateServer_CanFight(void);
void UI_SetSkirmishCvar(char *base, const char *var_name, const char *string);
void UI_SetSkirmishCvarInt(char *base, const char *name, int value);
qboolean UI_GetSkirmishCvar(char *base, const char *var_name, char *buffer, int buflen);
int UI_GetSkirmishCvarInt(char *base, const char *name);
int UI_GetSkirmishCvarIntClamp(int min, int max, char *base, const char *name);
void UI_CreateServer_SaveSkirmishCvars(void);
void UI_CreateServer_LoadSkirmishCvars(void);
qboolean UI_SkirmishCvarExists(char *base, const char *var_name);
// ui_createserver_script.c
void CreateServer_InitScriptData(qboolean multi);
void CreateServer_LoadScriptDataFromType(int gametype);
void CreateServer_SaveScriptData(void);
typedef char *(*String_Callback)(int index);
void UI_LoadMultiArray(char *base, const char *key, String_Callback callback, int count, int size, char newnull);
void UI_SaveMultiArray(char *base, const char *key, String_Callback callback, int count, int size, char newnull);
qboolean CreateServer_ServerScript(const char *scriptFile);
// ui_createserver_custommaps.c
enum {
	MAPICONS_ALL,
	MAPICONS_CUSTOM,
	MAPICONS_NONE,
	MAPICONS_MAX
};
// stores info on a picture that might be drawn on screen
typedef struct mappic_s {
	char mapname[SHORTMAP_BUFFER];
	int gamebits;
	int type[MAX_MAPTYPES];
	int num_types;
} mappic_t;

typedef qboolean (*callbackMapList)(const char *);

int GametypeBits(char *string);
int UI_BuildMapListByType(int *list, int listmax, int gametype, callbackMapList);
void CreateServer_SetIconFromGameType(menubitmap_s *b, int gametype, qboolean custom);
qboolean CreateServer_MapSupportsBots(const char *mapname);
int CreateServer_NumCustomMapTypes(void);
void CreateServer_InitMapPictureFromIndex(mappic_t *mappic, int index);
void CreateServer_InitMapPicture(mappic_t *mappic, const char *mapname);
void CreateServer_DrawMapPicture(int x, int y, int w, int h, mappic_t *mappic, vec4_t color);
qboolean CreateServer_IsCustomMapType(const char *mapname, int type);
const char *CreateServer_MapIconFromType(int gametype, qboolean isCustomMap);
void UI_LoadMapTypeInfo(void);
// global data
extern const char *idmap_list[];
extern int gametype_remap[NUM_GAMETYPES];
extern int gametype_remap2[MAX_GAME_TYPE];
extern const char *gametype_items[NUM_GAMETYPES + 1];
extern const char *randommaptype_items[MAP_RND_MAX + MAX_MAPTYPES + 1];
extern char *gametype_cvar_base[NUM_GAMETYPES];
extern const char *mapfilter_items[MAPFILTER_MAX + MAX_MAPTYPES + 1];
