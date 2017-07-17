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

	CREATE SERVER GLOBAL DATA USED BY UI AND SCRIPT

=======================================================================================================================================
*/

#include "ui_local.h"
#include "ui_createserver.h"

// global data
scriptdata_t s_scriptdata;

// order must mach that of gametype_remap[]
char *gametype_cvar_base[NUM_GAMETYPES] = {
	"ui_ffa_%s",
	"ui_tourney_%s",
	"ui_team_%s",
	"ui_ctf_%s",
	"ui_1flag_%s",
	"ui_obelisk_%s",
	"ui_harvester_%s"
};

const char *idmap_list[] = {
	// deathmatch
	"q3dm0",
	"q3dm1",
	"q3dm2",
	"q3dm3",
	"q3dm4",
	"q3dm5",
	"q3dm6",
	"q3dm7",
	"q3dm8",
	"q3dm9",
	"q3dm10",
	"q3dm11",
	"q3dm12",
	"q3dm13",
	"q3dm14",
	"q3dm15",
	"q3dm16",
	"q3dm17",
	"q3dm18",
	"q3dm19",
	// tourney
	"q3tourney1",
	"q3tourney2",
	"q3tourney3",
	"q3tourney4",
	"q3tourney5",
	"q3tourney6",
	// ctf
	"q3ctf1",
	"q3ctf2",
	"q3ctf3",
	"q3ctf4",
	"q3tourney6_ctf",
	// 3-wave ctf
	"q3wctf1",
	"q3wctf2",
	"q3wctf3",
	// pro-maps
	"pro-q3dm6",
	"pro-q3dm13",
	"pro-q3tourney2",
	"pro-q3tourney4",
	0
};
// individual items
// Must be in ITEM_* numerical order
itemnode_t server_itemlist[ITEM_COUNT] = {
	// weapon
	{ITEMGROUP_WEAPON, ITEM_GAUNTLET, "weapon_gauntlet", "GLT"},
	{ITEMGROUP_WEAPON, ITEM_HANDGUN, "weapon_handgun", "HG"},
	{ITEMGROUP_WEAPON, ITEM_MACHINEGUN, "weapon_machinegun", "MG"},
	{ITEMGROUP_WEAPON, ITEM_HEAVY_MACHINEGUN, "weapon_heavy_machinegun", "HMG"},
	{ITEMGROUP_WEAPON, ITEM_CHAINGUN, "weapon_chaingun", "CG"},
	{ITEMGROUP_WEAPON, ITEM_SHOTGUN, "weapon_shotgun", "SG"},
	{ITEMGROUP_WEAPON, ITEM_NAILGUN, "weapon_nailgun", "NG"},
	{ITEMGROUP_WEAPON, ITEM_PHOSPHORGUN, "weapon_phosphorgun", "PPG"},
	{ITEMGROUP_WEAPON, ITEM_PROXLAUNCHER, "weapon_prox_launcher", "Proxlauncher"},
	{ITEMGROUP_WEAPON, ITEM_GRENADELAUNCHER, "weapon_grenadelauncher", "GL"},
	{ITEMGROUP_WEAPON, ITEM_NAPALMLAUNCHER, "weapon_napalmlauncher", "NL"},
	{ITEMGROUP_WEAPON, ITEM_ROCKETLAUNCHER, "weapon_rocketlauncher", "RL"},
	{ITEMGROUP_WEAPON, ITEM_LIGHTNING, "weapon_lightning", "LG"},
	{ITEMGROUP_WEAPON, ITEM_RAILGUN, "weapon_railgun", "RG"},
	{ITEMGROUP_WEAPON, ITEM_PLASMAGUN, "weapon_plasmagun", "PG"},
	{ITEMGROUP_WEAPON, ITEM_BFG, "weapon_bfg", "BFG"},
	{ITEMGROUP_WEAPON, ITEM_MISSILELAUNCHER, "weapon_missilelauncher", "ML"},
	// ammo
	{ITEMGROUP_AMMO, ITEM_HANDGUN_AMMO, "ammo_clip", "HGA"},
	{ITEMGROUP_AMMO, ITEM_BULLETS, "ammo_bullets", "MGA"},
	{ITEMGROUP_AMMO, ITEM_HMG_BULLETS, "ammo_hmg_bullets", "HMGA"},
	{ITEMGROUP_AMMO, ITEM_BELT, "ammo_belt", "CGA"},
	{ITEMGROUP_AMMO, ITEM_SHELLS, "ammo_shells", "SGA"},
	{ITEMGROUP_AMMO, ITEM_NAILS, "ammo_nails", "NGA"},
	{ITEMGROUP_AMMO, ITEM_CAPSULES, "ammo_capsules", "PPGA"},
	{ITEMGROUP_AMMO, ITEM_MINES, "ammo_mines", "PMA"},
	{ITEMGROUP_AMMO, ITEM_GRENADES, "ammo_grenades", "GLA"},
	{ITEMGROUP_AMMO, ITEM_CANISTERS, "ammo_canisters", "NLA"},
	{ITEMGROUP_AMMO, ITEM_ROCKETS, "ammo_rockets", "RLA"},
	{ITEMGROUP_AMMO, ITEM_LIGHTNING_AMMO, "ammo_lightning", "LGA"},
	{ITEMGROUP_AMMO, ITEM_SLUGS, "ammo_slugs", "RGA"},
	{ITEMGROUP_AMMO, ITEM_CELLS, "ammo_cells", "PGA"},
	{ITEMGROUP_AMMO, ITEM_BFG_AMMO, "ammo_bfg", "BFGA"},
	{ITEMGROUP_AMMO, ITEM_MISSILES, "ammo_missiles", "MLA"},
	// health
	{ITEMGROUP_HEALTH, ITEM_HEALTHSMALL, "item_health_small", "SH"},
	{ITEMGROUP_HEALTH, ITEM_HEALTH, "item_health", "NH"},
	{ITEMGROUP_HEALTH, ITEM_HEALTHLARGE, "item_health_large", "HL"},
	{ITEMGROUP_HEALTH, ITEM_HEALTHMEGA, "item_health_mega", "MH"},
	// armor
	{ITEMGROUP_ARMOR, ITEM_ARMORSHARD, "item_armor_shard", "AS"},
	{ITEMGROUP_ARMOR, ITEM_ARMORCOMBAT, "item_armor_combat", "YA"},
	{ITEMGROUP_ARMOR, ITEM_ARMORBODY, "item_armor_body", "RA"},
	{ITEMGROUP_ARMOR, ITEM_ARMORFULL, "item_armor_full", "BA"},
	// holdable items
	{ITEMGROUP_HOLDABLE, ITEM_KAMIKAZE, "holdable_kamikaze", "Kamikazi"},
	// powerups
	{ITEMGROUP_POWERUPS, ITEM_QUAD, "item_quad", "Quad"},
	{ITEMGROUP_POWERUPS, ITEM_INVIS, "item_invis", "Invis"},
	{ITEMGROUP_POWERUPS, ITEM_REGEN, "item_regen", "Regen"},
	// persistant powerups
	{ITEMGROUP_PERSISTANT_POWERUPS, ITEM_AMMOREGEN, "item_ammoregen", "Ammoregen"},
	{ITEMGROUP_PERSISTANT_POWERUPS, ITEM_GUARD, "item_guard", "Guard"},
	{ITEMGROUP_PERSISTANT_POWERUPS, ITEM_DOUBLER, "item_doubler", "Doubler"},
	{ITEMGROUP_PERSISTANT_POWERUPS, ITEM_SCOUT, "item_scout", "Scout"}
};

/*
=======================================================================================================================================

	DATA ARCHIVE

=======================================================================================================================================
*/

// Some of these vars are legacy cvars that were previously stored in q3config.cfg.
// In order to load them into memory for porting over to the new data format we need to "register" them and load their current values.
// This porting process should be done only once, assuming we installed over a previous version of UIE.
// Default values for cvars that we haven't yet created.
// On startup we load from uiSkirmish.dat, check for new vars from this list, and add them into data storage.

typedef struct {
	char *cvarName;
	char *defaultString;
} ui_cvarTable_t;

ui_cvarTable_t ui_cvarTable[] = {
	{"ui_gametype", "0"},
	{"ui_pureServer", "1"},
	{"ui_inactivity", "0"},
	{"ui_smoothclients", "1"},
	{"ui_syncclients", "0"},
	{"ui_allowmaxrate", "0"},
	{"ui_maxrate", "0"},
	{"ui_allowdownload", "1"},
	{"ui_allowpass", "0"},
	{"ui_password", ""},
	{"ui_allowvote", "1"},
	{"ui_minPing", "0"},
	{"ui_maxPing", "0"},
	{"ui_allowMinPing", "0"},
	{"ui_allowMaxPing", "0"},
	{"ui_preventConfigBug", "0"},
	{"ui_gravity", "800"},
	{"ui_knockback", "1000"},
	{"ui_quadfactor", "3"},
	{"ui_netport", "27960"},
	{"ui_svfps", "20"},
	{"ui_allowprivateclients", "0"},
	{"ui_privateclients", "0"},
	{"ui_privatepassword", ""},
	{"ui_strictAuth", "1"},
	{"ui_lanForceRate", "1"},
	// Free For All
	{"ui_ffa_fragtype", "0"},
	{"ui_ffa_timetype", "0"},
	{"ui_ffa_customfraglimits", ""},
	{"ui_ffa_customtimelimits", ""},
	{"ui_ffa_maplist", ""},
	{"ui_ffa_maplistexclude", ""},
	{"ui_ffa_MapRepeat", "1"},
	{"ui_ffa_MapSource", "0"},
	{"ui_ffa_RandomMapCount", "16"},
	{"ui_ffa_RandomMapType", "0"},
	{"ui_ffa_slottype", ""},
	{"ui_ffa_botname", ""},
	{"ui_ffa_botexclude", ""},
	{"ui_ffa_botskill", ""},
	{"ui_ffa_BotSelection", "0"},
	{"ui_ffa_BotCount", "6"},
	{"ui_ffa_BotChange", "0"},
	{"ui_ffa_OpenSlotCount", "0"},
	{"ui_ffa_BotSkillType", "0"},
	{"ui_ffa_BotSkillValue", "200"},
	{"ui_ffa_BotSkillBias", "0"},
	{"ui_ffa_PlayerJoinAs", "0"},
	{"ui_ffa_hostname", "Quake Wars Deathmatch Server"},
	{"ui_ffa_ForceRespawn", "20"},
	{"ui_ffa_itemGroups", ""},
	{"ui_ffa_itemsHidden", ""},
	{"ui_ffa_fraglimit", "50"},
	{"ui_ffa_timelimit", "15"},
	{"ui_ffa_weaponrespawn", "5"},
	// Tourney
	{"ui_tourney_fragtype", "0"},
	{"ui_tourney_timetype", "0"},
	{"ui_tourney_customfraglimits", ""},
	{"ui_tourney_customtimelimits", ""},
	{"ui_tourney_maplist", ""},
	{"ui_tourney_maplistexclude", ""},
	{"ui_tourney_MapRepeat", "1"},
	{"ui_tourney_MapSource", "0"},
	{"ui_tourney_RandomMapCount", "16"},
	{"ui_tourney_RandomMapType", "0"},
	{"ui_tourney_slottype", ""},
	{"ui_tourney_botname", ""},
	{"ui_tourney_botexclude", ""},
	{"ui_tourney_botskill", ""},
	{"ui_tourney_BotSelection", "0"},
	{"ui_tourney_BotCount", "4"},
	{"ui_tourney_BotChange", "0"},
	{"ui_tourney_OpenSlotCount", "0"},
	{"ui_tourney_BotSkillType", "0"},
	{"ui_tourney_BotSkillValue", "200"},
	{"ui_tourney_BotSkillBias", "0"},
	{"ui_tourney_PlayerJoinAs", "0"},
	{"ui_tourney_hostname", "Quake Wars Tournament Server"},
	{"ui_tourney_ForceRespawn", "20"},
	{"ui_tourney_itemGroups", ""},
	{"ui_tourney_itemsHidden", ""},
	{"ui_tourney_fraglimit", "0"},
	{"ui_tourney_timelimit", "15"},
	{"ui_tourney_weaponrespawn", "5"},
	// Team Deathmatch
	{"ui_team_fragtype", "0"},
	{"ui_team_timetype", "0"},
	{"ui_team_customfraglimits", ""},
	{"ui_team_customtimelimits", ""},
	{"ui_team_maplist", ""},
	{"ui_team_maplistexclude", ""},
	{"ui_team_MapRepeat", "1"},
	{"ui_team_MapSource", "0"},
	{"ui_team_RandomMapCount", "16"},
	{"ui_team_RandomMapType", "0"},
	{"ui_team_slottype", ""},
	{"ui_team_botname", ""},
	{"ui_team_botexclude", ""},
	{"ui_team_botskill", ""},
	{"ui_team_BotSelection", "0"},
	{"ui_team_BotCount", "8"},
	{"ui_team_BotChange", "0"},
	{"ui_team_OpenSlotCount", "0"},
	{"ui_team_BotSkillType", "0"},
	{"ui_team_BotSkillValue", "200"},
	{"ui_team_BotSkillBias", "0"},
	{"ui_team_PlayerJoinAs", "0"},
	{"ui_team_hostname", "Quake Wars TDM Server"},
	{"ui_team_ForceRespawn", "20"},
	{"ui_team_itemGroups", ""},
	{"ui_team_itemsHidden", ""},
	{"ui_team_fraglimit", "0"},
	{"ui_team_timelimit", "15"},
	{"ui_team_weaponrespawn", "15"},
	{"ui_team_friendly", "1"},
	{"ui_team_TeamSwapped", "0"},
	{"ui_team_AutoJoin", "1"},
	{"ui_team_TeamBalance", "1"},
	// Capture the Flag
	{"ui_ctf_capturetype", "0"},
	{"ui_ctf_timetype", "0"},
	{"ui_ctf_customcapturelimits", ""},
	{"ui_ctf_customtimelimits", ""},
	{"ui_ctf_maplist", ""},
	{"ui_ctf_maplistexclude", ""},
	{"ui_ctf_MapRepeat", "1"},
	{"ui_ctf_MapSource", "0"},
	{"ui_ctf_RandomMapCount", "16"},
	{"ui_ctf_RandomMapType", "0"},
	{"ui_ctf_slottype", ""},
	{"ui_ctf_botname", ""},
	{"ui_ctf_botexclude", ""},
	{"ui_ctf_botskill", ""},
	{"ui_ctf_BotSelection", "0"},
	{"ui_ctf_BotCount", "8"},
	{"ui_ctf_BotChange", "0"},
	{"ui_ctf_OpenSlotCount", "0"},
	{"ui_ctf_BotSkillType", "0"},
	{"ui_ctf_BotSkillValue", "200"},
	{"ui_ctf_BotSkillBias", "0"},
	{"ui_ctf_PlayerJoinAs", "0"},
	{"ui_ctf_hostname", "Quake Wars CTF Server"},
	{"ui_ctf_ForceRespawn", "20"},
	{"ui_ctf_itemGroups", ""},
	{"ui_ctf_itemsHidden", ""},
	{"ui_ctf_capturelimit", "8"},
	{"ui_ctf_timelimit", "15"},
	{"ui_ctf_weaponrespawn", "5"},
	{"ui_ctf_friendly", "1"},
	{"ui_ctf_TeamSwapped", "0"},
	{"ui_ctf_AutoJoin", "1"},
	{"ui_ctf_TeamBalance", "1"},
	// Oneflag
	{"ui_1flag_capturetype", "0"},
	{"ui_1flag_timetype", "0"},
	{"ui_1flag_customcapturelimits", ""},
	{"ui_1flag_customtimelimits", ""},
	{"ui_1flag_maplist", ""},
	{"ui_1flag_maplistexclude", ""},
	{"ui_1flag_MapRepeat", "1"},
	{"ui_1flag_MapSource", "0"},
	{"ui_1flag_RandomMapCount", "16"},
	{"ui_1flag_RandomMapType", "0"},
	{"ui_1flag_slottype", ""},
	{"ui_1flag_botname", ""},
	{"ui_1flag_botexclude", ""},
	{"ui_1flag_botskill", ""},
	{"ui_1flag_BotSelection", "0"},
	{"ui_1flag_BotCount", "8"},
	{"ui_1flag_BotChange", "0"},
	{"ui_1flag_OpenSlotCount", "0"},
	{"ui_1flag_BotSkillType", "0"},
	{"ui_1flag_BotSkillValue", "200"},
	{"ui_1flag_BotSkillBias", "0"},
	{"ui_1flag_PlayerJoinAs", "0"},
	{"ui_1flag_hostname", "Quake Wars 1CTF Server"},
	{"ui_1flag_ForceRespawn", "20"},
	{"ui_1flag_itemGroups", ""},
	{"ui_1flag_itemsHidden", ""},
	{"ui_1flag_capturelimit", "8"},
	{"ui_1flag_timelimit", "15"},
	{"ui_1flag_weaponrespawn", "5"},
	{"ui_1flag_friendly", "1"},
	{"ui_1flag_TeamSwapped", "0"},
	{"ui_1flag_AutoJoin", "1"},
	{"ui_1flag_TeamBalance", "1"},
	// Obelisk
	{"ui_obelisk_capturetype", "0"},
	{"ui_obelisk_timetype", "0"},
	{"ui_obelisk_customcapturelimits", ""},
	{"ui_obelisk_customtimelimits", ""},
	{"ui_obelisk_maplist", ""},
	{"ui_obelisk_maplistexclude", ""},
	{"ui_obelisk_MapRepeat", "1"},
	{"ui_obelisk_MapSource", "0"},
	{"ui_obelisk_RandomMapCount", "16"},
	{"ui_obelisk_RandomMapType", "0"},
	{"ui_obelisk_slottype", ""},
	{"ui_obelisk_botname", ""},
	{"ui_obelisk_botexclude", ""},
	{"ui_obelisk_botskill", ""},
	{"ui_obelisk_BotSelection", "0"},
	{"ui_obelisk_BotCount", "8"},
	{"ui_obelisk_BotChange", "0"},
	{"ui_obelisk_OpenSlotCount", "0"},
	{"ui_obelisk_BotSkillType", "0"},
	{"ui_obelisk_BotSkillValue", "200"},
	{"ui_obelisk_BotSkillBias", "0"},
	{"ui_obelisk_PlayerJoinAs", "0"},
	{"ui_obelisk_hostname", "Quake Wars Overload Server"},
	{"ui_obelisk_ForceRespawn", "20"},
	{"ui_obelisk_itemGroups", ""},
	{"ui_obelisk_itemsHidden", ""},
	{"ui_obelisk_capturelimit", "8"},
	{"ui_obelisk_timelimit", "15"},
	{"ui_obelisk_weaponrespawn", "5"},
	{"ui_obelisk_friendly", "1"},
	{"ui_obelisk_TeamSwapped", "0"},
	{"ui_obelisk_AutoJoin", "1"},
	{"ui_obelisk_TeamBalance", "1"},
	// Harvester
	{"ui_harvester_capturetype", "0"},
	{"ui_harvester_timetype", "0"},
	{"ui_harvester_customcapturelimits", ""},
	{"ui_harvester_customtimelimits", ""},
	{"ui_harvester_maplist", ""},
	{"ui_harvester_maplistexclude", ""},
	{"ui_harvester_MapRepeat", "1"},
	{"ui_harvester_MapSource", "0"},
	{"ui_harvester_RandomMapCount", "16"},
	{"ui_harvester_RandomMapType", "0"},
	{"ui_harvester_slottype", ""},
	{"ui_harvester_botname", ""},
	{"ui_harvester_botexclude", ""},
	{"ui_harvester_botskill", ""},
	{"ui_harvester_BotSelection", "0"},
	{"ui_harvester_BotCount", "8"},
	{"ui_harvester_BotChange", "0"},
	{"ui_harvester_OpenSlotCount", "0"},
	{"ui_harvester_BotSkillType", "0"},
	{"ui_harvester_BotSkillValue", "200"},
	{"ui_harvester_BotSkillBias", "0"},
	{"ui_harvester_PlayerJoinAs", "0"},
	{"ui_harvester_hostname", "Quake Wars Harvester Server"},
	{"ui_harvester_ForceRespawn", "20"},
	{"ui_harvester_itemGroups", ""},
	{"ui_harvester_itemsHidden", ""},
	{"ui_harvester_capturelimit", "10"},
	{"ui_harvester_timelimit", "15"},
	{"ui_harvester_weaponrespawn", "5"},
	{"ui_harvester_friendly", "1"},
	{"ui_harvester_TeamSwapped", "0"},
	{"ui_harvester_AutoJoin", "1"},
	{"ui_harvester_TeamBalance", "1"},
};

static const int ui_cvarTableSize = sizeof(ui_cvarTable) / sizeof(ui_cvarTable[0]);

#define MAX_CVAR_DATA (24 * 1024)
#define UI_SKIRMISH_DATAFILE "uiSkirmish.dat"

static qboolean skirmishCvarLoaded = qfalse;
static char skirmishCvarData[MAX_CVAR_DATA];

/*
=======================================================================================================================================
UI_SkirmishCvarExists
=======================================================================================================================================
*/
qboolean UI_SkirmishCvarExists(char *base, const char *var_name) {
	int i;
	char name[64];

	if (!var_name || !*var_name) {
		return qfalse;
	}

	if (base) {
		Q_strncpyz(name, va(base, var_name), 64);
	} else {
		Q_strncpyz(name, var_name, 64);
	}

	for (i = 0; i < ui_cvarTableSize; i++) {
		if (!Q_stricmp(ui_cvarTable[i].cvarName, name)) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
UI_GetSkirmishCvar
=======================================================================================================================================
*/
qboolean UI_GetSkirmishCvar(char *base, const char *var_name, char *buffer, int buflen) {
	char *p, *pnext, name[64];

	if (!var_name || !*var_name || !buffer) {
		return qfalse;
	}

	if (!skirmishCvarLoaded) {
		UI_CreateServer_LoadSkirmishCvars();
	}

	if (base) {
		Q_strncpyz(name, va(base, var_name), 64);
	} else {
		Q_strncpyz(name, var_name, 64);
	}

	p = skirmishCvarData;

	while (*p) {
		pnext = p + strlen(p) + 1;

		if (!Q_stricmp(name, COM_Parse(&p))) {
			break;
		}

		p = pnext;
	}

	buffer[0] = '\0';

	if (!*p) {
//		Com_Printf("Cvar not found: %s\n", name);
		return qfalse;
	}
	// return string inside quotes
	Q_strncpyz(buffer, COM_Parse(&p), buflen);

	return qtrue;
}

/*
=======================================================================================================================================
UI_GetSkirmishCvarInt
=======================================================================================================================================
*/
int UI_GetSkirmishCvarInt(char *base, const char *name) {
	char buf[32];

	UI_GetSkirmishCvar(base, name, buf, 32);
	return atoi(buf);
}

/*
=======================================================================================================================================
UI_GetSkirmishCvarIntClamp
=======================================================================================================================================
*/
int UI_GetSkirmishCvarIntClamp(int min, int max, char *base, const char *name) {
	int value;
	char buf[32];

	UI_GetSkirmishCvar(base, name, buf, 32);

	value = atoi(buf);

	if (value < min) {
		value = min;
	}

	if (value > max) {
		value = max;
	}

	return value;
}

/*
=======================================================================================================================================
UI_SetSkirmishCvar
=======================================================================================================================================
*/
void UI_SetSkirmishCvar(char *base, const char *var_name, const char *string) {
	char *p, *old, *newstr, name[64];
	int len, next, lenmove, oldlen, addlen;

	if (!var_name || !*var_name || !string) {
		return;
	}

	if (base) {
		Q_strncpyz(name, va(base, var_name), 64);
	} else {
		Q_strncpyz(name, var_name, 64);
	}

	len = strlen(name) + 1;
	p = skirmishCvarData;
	old = NULL;
	// do search
	while (*p) {
		next = strlen(p) + 1;
		// prevent premature termination on longer name string
		if (!old && (next > len) && p[len - 1] == ' ' && Q_stricmpn(p, name, len - 1) == 0) {
			old = p;
		}

		p += next;
	}
	// delete old entry
	if (old) {
		oldlen = strlen(old) + 1;
		lenmove = p -(old + oldlen) + 1;

		memmove(old, old + oldlen, lenmove);

		p -= oldlen;
	}
	// check for overflow - bad news
	newstr = va("%s \"%s\"", name, string);
	addlen = strlen(newstr) + 1;

	if (p - skirmishCvarData + addlen >= MAX_CVAR_DATA) {
		Com_Printf("Skirmish Cvar overflow, cvar %s lost\n", name);
		return;
	}
	// add to end, keeping \0\0 integrity
	Q_strncpyz(p, newstr, addlen);
	p[addlen] = '\0';

//	Com_Printf("Cvar wrote: %s\n", newstr);
}

/*
=======================================================================================================================================
UI_SetSkirmishCvarInt
=======================================================================================================================================
*/
void UI_SetSkirmishCvarInt(char *base, const char *name, int value) {
	char buf[32];

	Q_strncpyz(buf, va("%i", value), 32);
	UI_SetSkirmishCvar(base, name, buf);
}

/*
=======================================================================================================================================
UI_CreateServer_MergeSkirmishCvars

Convert from text format to internal NULL buffer terminated.
May have been edited by a program that changes line terminator formats so we attempt to recover from this.
=======================================================================================================================================
*/
static void UI_CreateServer_MergeSkirmishCvars(char *cvarList) {
	char *p, varname[MAX_TOKEN_CHARS], *data;

	p = cvarList;

	while (*p) {
		Q_strncpyz(&varname[0], COM_Parse(&p), MAX_TOKEN_CHARS);

		if (!varname[0]) {
			break;
		}

		data = COM_Parse(&p);

		UI_SetSkirmishCvar(NULL, varname, data);
	}
}

/*
=======================================================================================================================================
UI_CreateServer_SaveSkirmishCvars
=======================================================================================================================================
*/
void UI_CreateServer_SaveSkirmishCvars(void) {
	char *p;
	fileHandle_t file;
	int len;

	p = skirmishCvarData;

	while (*p) {
		p += strlen(p);
		*p++ = '\r';
	}

	len = strlen(skirmishCvarData);

	trap_FS_FOpenFile(UI_SKIRMISH_DATAFILE, &file, FS_WRITE);
	trap_FS_Write(skirmishCvarData, len, file);
	trap_FS_FCloseFile(file);

	p = skirmishCvarData;

	while (*p) {
		if (*p == '\r') {
			*p = '\0';
		}

		p++;
	}

	// Com_Printf("Wrote %s, %i bytes\n", UI_SKIRMISH_DATAFILE, len);
}

/*
=======================================================================================================================================
UI_CreateServer_LoadSkirmishCvars
=======================================================================================================================================
*/
void UI_CreateServer_LoadSkirmishCvars(void) {
	int i, len;
	vmCvar_t cvar;
	fileHandle_t file;
	char newCvars[MAX_CVAR_DATA];

	memset(skirmishCvarData, 1, MAX_CVAR_DATA);

	skirmishCvarLoaded = qfalse;
	skirmishCvarData[0] = '\0';
	// load from cvars in memory or default values on list.
	// The flags marked here don't override the existing ARCHIVE flag, which is set when the cvar is loaded from qwconfig.cfg
	for (i = 0; i < ui_cvarTableSize; i++) {
		trap_Cvar_Register(&cvar, ui_cvarTable[i].cvarName, ui_cvarTable[i].defaultString, CVAR_TEMP|CVAR_USER_CREATED);
		UI_SetSkirmishCvar(NULL, ui_cvarTable[i].cvarName, cvar.string);
	}

	skirmishCvarLoaded = qtrue;
	// load cvars from file, and merge them with this prepared list
	len = trap_FS_FOpenFile(UI_SKIRMISH_DATAFILE, &file, FS_READ);

	if (!(len < MAX_CVAR_DATA - 1)) {
		Com_Printf(UI_SKIRMISH_DATAFILE" is too large, skirmish reset to default.\n");
		trap_FS_FCloseFile(file);
		return;
	} else if (len <= 0) {
		Com_Printf(UI_SKIRMISH_DATAFILE" doesn't exist, imported default cvars.\n");
		trap_FS_FCloseFile(file);
		return;
	}

	trap_FS_Read(newCvars, len, file);
	trap_FS_FCloseFile(file);

	newCvars[len] = '\0';
	skirmishCvarLoaded = qtrue;

	UI_CreateServer_MergeSkirmishCvars(newCvars);
	UI_CreateServer_SaveSkirmishCvars();
}

/*
=======================================================================================================================================

	MISC FUNCTIONS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
UI_CreateServer_RegisterDisableCvars
=======================================================================================================================================
*/
void UI_CreateServer_RegisterDisableCvars(qboolean init) {
	int i;
	char *disable;

	for (i = 0; i < ITEM_COUNT; i++) {
		disable = va("disable_%s", server_itemlist[i].mapitem);

		if (init) {
			trap_Cvar_Register(NULL, disable, "0", 0);
		} else {
			trap_Cvar_Set(disable, "0");
		}
	}
}

/*
=======================================================================================================================================
CreateServer_CanFight
=======================================================================================================================================
*/
qboolean CreateServer_CanFight(void) {

	// number of maps
	if (CreateServer_IsRandomGeneratedMapList(s_scriptdata.map.listSource)) {
		if (s_scriptdata.map.SourceCount == 0) {
			return qfalse;
		}
	} else {
		if (s_scriptdata.map.num_maps == 0) {
			return qfalse;
		}
	}
	// number of bots
	if (s_scriptdata.bot.typeSelect == BOTTYPE_SELECTARENASCRIPT) {
		if (CreateServer_ValidBotCount() == 0) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
UI_SaveMultiArray
=======================================================================================================================================
*/
void UI_SaveMultiArray(char *base, const char *key, String_Callback callback, int count, int size, char newnull) {
	char buf[MAX_STRING_CHARS], *arraychar;
	int i, last;

	if (size * count >= MAX_STRING_CHARS) {
		trap_Error("size * step >= MAX_STRING_CHARS");
		return;
	}

	last = 0;

	for (i = 0; i < count; i++) {
		arraychar = callback(i);
		Q_strncpyz(&buf[last], arraychar, size);
		last += strlen(arraychar);
		buf[last++] = newnull;
	}

	buf[last] = '\0';

	UI_SetSkirmishCvar(base, key, buf);
}

/*
=======================================================================================================================================
UI_LoadMultiArray
=======================================================================================================================================
*/
void UI_LoadMultiArray(char *base, const char *key, String_Callback callback, int count, int size, char newnull) {
	char buf[MAX_STRING_CHARS], *arraychar;
	int len, i, c;

	UI_GetSkirmishCvar(base, key, buf, MAX_STRING_CHARS);

	len = strlen(buf);

	for (i = 0; i < len; i++) {
		if (buf[i] == newnull) {
			buf[i] = '\0';
		}
	}

	i = 0;
	c = 0;

	while (i < len && c < count) {
		arraychar = callback(c);
		Q_strncpyz(arraychar, &buf[i], size);
		i += strlen(&buf[i]) + 1;
		c++;
	}
	// clear remaining elements
	while (c < count) {
		arraychar = callback(c);
		memset(arraychar, 0, size);
		c++;
	}
}

/*
=======================================================================================================================================

	LOADING AND SAVING OF MAP SCRIPT DATA

=======================================================================================================================================
*/

// additional map type strings are set in ui_createserver_custommaps.c
const char *randommaptype_items[MAP_RND_MAX + MAX_MAPTYPES + 1] = {
	"(Any)",		// MAP_RND_ANY
	"(Id only)",	// MAP_RND_ID
	"(NonId only)",	// MAP_RND_NONID
	"(Bias Id)",	// MAP_RND_BIASID
	"(Bias NonId)",	// MAP_RND_BIASNONID
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0
};

/*
=======================================================================================================================================
CreateServer_IsIdMap
=======================================================================================================================================
*/
qboolean CreateServer_IsIdMap(const char *mapname) {
	const char *const *ptr;

	// check list of idmaps
	ptr = idmap_list;

	while (*ptr) {
		if (!Q_stricmp(*ptr, mapname)) {
			return qtrue;
		}

		ptr++;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_IsRandomGeneratedMapList

Map generation will be from a list of randomly selected map names, not a user list of map name.
=======================================================================================================================================
*/
qboolean CreateServer_IsRandomGeneratedMapList(int type) {

	if (type == MAP_MS_RANDOM || type == MAP_MS_RANDOMEXCLUDE) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_MapPageCount
=======================================================================================================================================
*/
int CreateServer_MapPageCount(void) {
	int count;

	count = 1 + (s_scriptdata.map.num_maps / NUMMAPS_PERPAGE);

	if (count > MAX_MAPPAGES) {
		count = MAX_MAPPAGES;
	}

	return count;
}

/*
=======================================================================================================================================
CreateServer_RangeClipMapIndex
=======================================================================================================================================
*/
int CreateServer_RangeClipMapIndex(int index) {

	if (index < 0) {
		return 0;
	}

	if (index > s_scriptdata.map.num_maps) {
		index = s_scriptdata.map.num_maps;
	}

	if (index == MAX_NUMMAPS) {
		return MAX_NUMMAPS - 1;
	}

	return index;
}

/*
=======================================================================================================================================
SSMP_ShortName_Callback
=======================================================================================================================================
*/
static char *SSMP_ShortName_Callback(int index) {
	return s_scriptdata.map.data[index].shortName;
}

/*
=======================================================================================================================================
SSMP_FragLimit_Callback
=======================================================================================================================================
*/
static char *SSMP_FragLimit_Callback(int index) {
	return s_scriptdata.map.data[index].fragLimit;
}

/*
=======================================================================================================================================
SSMP_TimeLimit_Callback
=======================================================================================================================================
*/
static char *SSMP_TimeLimit_Callback(int index) {
	return s_scriptdata.map.data[index].timeLimit;
}

/*
=======================================================================================================================================
CreateServer_LoadMapList

Must be called after s_scriptdata.map.type has been set.
=======================================================================================================================================
*/
void CreateServer_LoadMapList(void) {
	char *s, *ml;

	ml = "maplist";

	if (s_scriptdata.map.listSource == MAP_MS_RANDOMEXCLUDE) {
		ml = "maplistexclude";
	}

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	UI_LoadMultiArray(s, ml, SSMP_ShortName_Callback, MAX_NUMMAPS, SHORTMAP_BUFFER, ';');
}

/*
=======================================================================================================================================
CreateServer_SwapMaps
=======================================================================================================================================
*/
qboolean CreateServer_SwapMaps(int from, int to) {
	static mapdata_t tmp;

	if (from < 0 || to < 0 || from == to) {
		return qfalse;
	}

	if (from >= s_scriptdata.map.num_maps || to >= s_scriptdata.map.num_maps) {
		return qfalse;
	}

	memcpy(&tmp, &s_scriptdata.map.data[from], sizeof(mapdata_t));
	memcpy(&s_scriptdata.map.data[from], &s_scriptdata.map.data[to], sizeof(mapdata_t));
	memcpy(&s_scriptdata.map.data[to], &tmp, sizeof(mapdata_t));

	return qtrue;
}

/*
=======================================================================================================================================
CreateServer_StoreMap

Places map data into the array, overwriting a previous entry.
=======================================================================================================================================
*/
void CreateServer_StoreMap(int pos, int arena) {
	const char *info;
	char *shortname, *longname;
	int len;

	pos = CreateServer_RangeClipMapIndex(pos);
	info = UI_GetArenaInfoByNumber(arena);
	shortname = Info_ValueForKey(info, "map");
	len = strlen(shortname) + 1;

	Q_strncpyz(s_scriptdata.map.data[pos].shortName, shortname, len);

	longname = Info_ValueForKey(info, "longname");

	if (!longname || !*longname) {
		longname = shortname;
	}

	len = strlen(longname) + 1;

	Q_strncpyz(s_scriptdata.map.data[pos].longName, longname, len);
	// increase map count if we put map into a previously empty slot
	// set frag/time limits too
	if (pos == s_scriptdata.map.num_maps) {
		s_scriptdata.map.num_maps++;
		Q_strncpyz(s_scriptdata.map.data[pos].fragLimit, va("%i", s_scriptdata.map.fragLimit), MAX_LIMIT_BUF);
		Q_strncpyz(s_scriptdata.map.data[pos].timeLimit, va("%i", s_scriptdata.map.timeLimit), MAX_LIMIT_BUF);
	}
}

/*
=======================================================================================================================================
CreateServer_InsertMap

Creates an empty slot for map data to be added.
=======================================================================================================================================
*/
void CreateServer_InsertMap(int pos, int arena) {
	int last, i;

	// drop any maps that try to overflow
	if (pos > MAX_NUMMAPS - 1) {
		return;
	}

	pos = CreateServer_RangeClipMapIndex(pos);
	// quietly drop last map on list if we are inserting earlier
	last = s_scriptdata.map.num_maps - 1;

	if (last == MAX_NUMMAPS - 1) {
		last--;
	} else {
		s_scriptdata.map.num_maps++;
	}
	// move maps up one slot
	for (i = last; i >= pos; i--) {
		memcpy(&s_scriptdata.map.data[i + 1], &s_scriptdata.map.data[i], sizeof(mapdata_t));
	}

	CreateServer_StoreMap(pos, arena);

	Q_strncpyz(s_scriptdata.map.data[pos].fragLimit, va("%i", s_scriptdata.map.fragLimit), MAX_LIMIT_BUF);
	Q_strncpyz(s_scriptdata.map.data[pos].timeLimit, va("%i", s_scriptdata.map.timeLimit), MAX_LIMIT_BUF);
}

/*
=======================================================================================================================================
CreateServer_DeleteMap
=======================================================================================================================================
*/
void CreateServer_DeleteMap(int index) {
	int lines, i;

	if (index < 0 || index >= MAX_NUMMAPS) {
		return;
	}

	lines = (MAX_NUMMAPS - index - 1);

	if (lines) {
		for (i = 0; i < lines; i++) {
			memcpy(&s_scriptdata.map.data[index + i], &s_scriptdata.map.data[index + i + 1], sizeof(mapdata_t));
		}
	}
	// zero final element only
	memset(&s_scriptdata.map.data[MAX_NUMMAPS - 1], 0, sizeof(mapdata_t));

	if (index < s_scriptdata.map.num_maps) {
		s_scriptdata.map.num_maps--;
	}
}

/*
=======================================================================================================================================
CreateServer_RefreshMapNames
=======================================================================================================================================
*/
void CreateServer_RefreshMapNames(void) {
	int i, j, count;
	const char *info;
	char *arena_mapname;

	i = 0;
	count = UI_GetNumArenas();

	while (i < MAX_NUMMAPS && s_scriptdata.map.data[i].shortName[0]) {
		for (j = 0; j < count; j++) {
			info = UI_GetArenaInfoByNumber(j);

			if (!info) {
				continue;
			}

			arena_mapname = Info_ValueForKey(info, "map");

			if (!arena_mapname || arena_mapname[0] == '\0') {
				continue;
			}

			if (!Q_stricmp(s_scriptdata.map.data[i].shortName, arena_mapname)) {
				Q_strncpyz(s_scriptdata.map.data[i].longName, Info_ValueForKey(info, "longname"), LONGMAP_BUFFER);
				break;
			}
		}
		// map not found, quietly delete it from list
		if (j == count) {
			CreateServer_DeleteMap(i);
			continue;
		}

		i++;
	}

	s_scriptdata.map.num_maps = i;
}

/*
=======================================================================================================================================
CreateServer_GetArenaFragLimit
=======================================================================================================================================
*/
static const char *CreateServer_GetArenaFragLimit(int map) {
	static char fraglimit[16];
	const char *info, *infofrag;

	info = UI_GetArenaInfoByMap(s_scriptdata.map.data[map].shortName);
	infofrag = Info_ValueForKey(info, "fraglimit");

	if (infofrag[0]) {
		Q_strncpyz(fraglimit, infofrag, 16);
	} else {
		Q_strncpyz(fraglimit, va("%i", s_scriptdata.map.fragLimit), 16);
	}

	return fraglimit;
}

/*
=======================================================================================================================================
CreateServer_MapDoAction
=======================================================================================================================================
*/
void CreateServer_MapDoAction(int src, int dest, int page, int selected) {
	int i, pageindex;
	const char *fragsrc, *timesrc;

	pageindex = page * NUMMAPS_PERPAGE;
	// actions using src
	if (src == MAP_CF_CLEARALL) {
		while (s_scriptdata.map.num_maps) {
			CreateServer_DeleteMap(0);
		}

		return;
	}

	if (src == MAP_CF_CLEARPAGE) {
		for (i = 0; i < NUMMAPS_PERPAGE; i++) {
			CreateServer_DeleteMap(pageindex);
		}

		return;
	}
	// actions that combine src and dest
	if (src == MAP_CF_ARENASCRIPT) {
		switch (dest) {
			case MAP_CT_SELECTED:
				if (selected >= 0) {
					fragsrc = CreateServer_GetArenaFragLimit(pageindex + selected);
					Q_strncpyz(s_scriptdata.map.data[pageindex + selected].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_PAGE:
				for (i = 0; i < NUMMAPS_PERPAGE; i++) {
					fragsrc = CreateServer_GetArenaFragLimit(pageindex + i);
					Q_strncpyz(s_scriptdata.map.data[pageindex + i].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_ALL:
				for (i = 0; i < s_scriptdata.map.num_maps; i++) {
					fragsrc = CreateServer_GetArenaFragLimit(i);
					Q_strncpyz(s_scriptdata.map.data[i].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
		}

		return;
	}
	// there's some overlap between the copying of time and frag limits
	// "flow through" from the time to the frag code is expected
	// ANY OTHER CUSTOM VALUES SHOULD APPEAR BEFORE THIS CODE

	// copy time
	timesrc = va("%i", s_scriptdata.map.timeLimit);

	if (src != MAP_CF_FRAG) {
		switch (dest) {
			case MAP_CT_SELECTED:
				if (selected >= 0) {
					Q_strncpyz(s_scriptdata.map.data[pageindex + selected].timeLimit, timesrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_PAGE:
				for (i = 0; i < NUMMAPS_PERPAGE; i++) {
					Q_strncpyz(s_scriptdata.map.data[pageindex + i].timeLimit, timesrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_ALL:
				for (i = 0; i < s_scriptdata.map.num_maps; i++) {
					Q_strncpyz(s_scriptdata.map.data[i].timeLimit, timesrc, MAX_LIMIT_BUF);
				}

				break;
		}
	}
	// copy frags/caps
	fragsrc = va("%i", s_scriptdata.map.fragLimit);

	if (src != MAP_CF_TIME) {
		switch (dest) {
			case MAP_CT_SELECTED:
				if (selected >= 0) {
					Q_strncpyz(s_scriptdata.map.data[pageindex + selected].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_PAGE:
				for (i = 0; i < NUMMAPS_PERPAGE; i++) {
					Q_strncpyz(s_scriptdata.map.data[pageindex + i].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
			case MAP_CT_ALL:
				for (i = 0; i < s_scriptdata.map.num_maps; i++) {
					Q_strncpyz(s_scriptdata.map.data[i].fragLimit, fragsrc, MAX_LIMIT_BUF);
				}

				break;
		}
	}
}

/*
=======================================================================================================================================
CreateServer_BuildMapDistribution
=======================================================================================================================================
*/
static void CreateServer_BuildMapDistribution(void) {
	int i, j, count, maptype, gametype, matchbits, gamebits;
	const char *info;
	char *arena_mapname;

	// set zero
	for (i = 0; i < MAX_GAME_TYPE; i++) {
		for (j = 0; j < MAX_MAP_GROUP; j++) {
			s_scriptdata.map.TypeCount[i][j] = 0;
		}
	}

	count = UI_GetNumArenas();

	for (i = 0; i < count; i++) {
		info = UI_GetArenaInfoByNumber(i);

		if (!info) {
			continue;
		}

		arena_mapname = Info_ValueForKey(info, "map");

		if (!arena_mapname || arena_mapname[0] == '\0') {
			continue;
		}

		if (CreateServer_IsIdMap(arena_mapname)) {
			maptype = MAP_GROUP_ID;
		} else {
			maptype = MAP_GROUP_NONID;
		}

		gamebits = GametypeBits(Info_ValueForKey(info, "type"));

		for (j = 0; j < NUM_GAMETYPES; j++) {
			gametype = gametype_remap[j];
			matchbits = 1 << gametype;

			if (gametype == GT_FFA) {
				matchbits |= (1 << GT_SINGLE_PLAYER);
			}

			if (matchbits & gamebits) {
				s_scriptdata.map.TypeCount[gametype_remap2[gametype]][maptype]++;
			}
		}
	}
}

/*
=======================================================================================================================================
CreateServer_SaveMapList
=======================================================================================================================================
*/
void CreateServer_SaveMapList(void) {
	char *s, *s1;

	s1 = "maplist";

	if (s_scriptdata.map.listSource == MAP_MS_RANDOMEXCLUDE) {
		s1 = "maplistexclude";
	}

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];

	UI_SaveMultiArray(s, s1, SSMP_ShortName_Callback, MAX_NUMMAPS, SHORTMAP_BUFFER, ';');
}

/*
=======================================================================================================================================
CreateServer_LoadMapScriptData

Loads map specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_LoadMapScriptData(void) {
	char buf[64], *s, *f, *f2;
	int i, max;

	f = "customfraglimits";
	f2 = "fragtype";
	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];

	if (s_scriptdata.gametype > GT_TEAM) {
		s_scriptdata.map.fragLimit = UI_GetSkirmishCvarIntClamp(0, 999, s, "capturelimit");
		f = "customcapturelimits";
		f2 = "capturetype";
	} else {
		s_scriptdata.map.fragLimit = UI_GetSkirmishCvarIntClamp(0, 99, s, "fraglimit");
	}

	s_scriptdata.map.timeLimit = UI_GetSkirmishCvarIntClamp(0, 999, s, "timelimit");
	// load custom frag/time values
	UI_LoadMultiArray(s, f, SSMP_FragLimit_Callback, MAX_NUMMAPS, MAX_LIMIT_BUF, ';');
	UI_LoadMultiArray(s, "customtimelimits", SSMP_TimeLimit_Callback, MAX_NUMMAPS, MAX_LIMIT_BUF, ';');
	// load type of frag/time value used to start game (none, default, custom)
	s_scriptdata.map.fragLimitType = UI_GetSkirmishCvarIntClamp(0, 2, s, f2);
	s_scriptdata.map.timeLimitType = UI_GetSkirmishCvarIntClamp(0, 2, s, "timetype");
	// load map source and repeat info
	s_scriptdata.map.Repeat = UI_GetSkirmishCvarIntClamp(0, 1, s, "MapRepeat");
	s_scriptdata.map.listSource = UI_GetSkirmishCvarIntClamp(0, MAP_MS_MAX - 1, s, "MapSource");
	// load maps
	// must be after s_scriptdata.map.type is set
	CreateServer_LoadMapList();

	s_scriptdata.map.Repeat = UI_GetSkirmishCvarIntClamp(0, 1, s, "MapRepeat");
	s_scriptdata.map.SourceCount = UI_GetSkirmishCvarIntClamp(2, 99, s, "RandomMapCount");

	UI_GetSkirmishCvar(s, "RandomMapType", buf, 64);

	s_scriptdata.map.SourceType = (int)Com_Clamp(0, MAP_RND_MAX - 1, atoi(buf)); // non-numerical values give zero

	max = CreateServer_NumCustomMapTypes();

	for (i = 0; i < max; i++) {
		if (!Q_stricmp(buf, randommaptype_items[MAP_RND_MAX + i])) {
			s_scriptdata.map.SourceType = MAP_RND_MAX + i;
			break;
		}
	}
	// validate each of the map names
	CreateServer_RefreshMapNames();
}

/*
=======================================================================================================================================
CreateServer_SaveMapScriptData

Saves map specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_SaveMapScriptData(void) {
	int type;
	char *s, *f, *f2;

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	f = "customfraglimits";
	f2 = "fragtype";

	UI_SetSkirmishCvarInt(s, "timelimit", s_scriptdata.map.timeLimit);

	if (s_scriptdata.gametype > GT_TEAM) {
		UI_SetSkirmishCvarInt(s, "capturelimit", s_scriptdata.map.fragLimit);
		f = "customcapturelimits";
		f2 = "capturetype";
	} else {
		UI_SetSkirmishCvarInt(s, "fraglimit", s_scriptdata.map.fragLimit);
	}

	CreateServer_SaveMapList();

	UI_SetSkirmishCvarInt(s, "MapSource", s_scriptdata.map.listSource);
	// save custom frag/time values
	UI_SaveMultiArray(s, f, SSMP_FragLimit_Callback, MAX_NUMMAPS, MAX_LIMIT_BUF, ';');
	UI_SaveMultiArray(s, "customtimelimits", SSMP_TimeLimit_Callback, MAX_NUMMAPS, MAX_LIMIT_BUF, ';');
	// save type of frag/time value used to start game (none, default, custom)
	UI_SetSkirmishCvarInt(s, f2, s_scriptdata.map.fragLimitType);
	UI_SetSkirmishCvarInt(s, "timetype", s_scriptdata.map.timeLimitType);
	// save map source and repeat info
	UI_SetSkirmishCvarInt(s, "MapRepeat", s_scriptdata.map.Repeat);
	UI_SetSkirmishCvarInt(s, "RandomMapCount", s_scriptdata.map.SourceCount);

	type = s_scriptdata.map.SourceType;

	if (type < MAP_RND_MAX) {
		UI_SetSkirmishCvarInt(s, "RandomMapType", type);
	} else {
		UI_SetSkirmishCvar(s, "RandomMapType", randommaptype_items[type]);
	}
}

/*
=======================================================================================================================================

	LOADING AND SAVING OF BOT SCRIPT DATA

=======================================================================================================================================
*/

#define BOT_TMPBUFFER 4

static char botskill_tmpbuffer[PLAYER_SLOTS][BOT_TMPBUFFER]; // tmp used to load/save bot skill values

/*
=======================================================================================================================================
CreateServer_IsBotArenaScript
=======================================================================================================================================
*/
qboolean CreateServer_IsBotArenaScript(int type) {

	if (type == BOTTYPE_ARENASCRIPT || type == BOTTYPE_RANDOMARENASCRIPT || type == BOTTYPE_SELECTARENASCRIPT) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_IsRandomBotExclude
=======================================================================================================================================
*/
qboolean CreateServer_IsRandomBotExclude(int type) {

	if (type == BOTTYPE_RANDOMEXCLUDE || type == BOTTYPE_RANDOMARENASCRIPT) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_IsRandomBotList
=======================================================================================================================================
*/
qboolean CreateServer_IsRandomBotList(int type) {

	if (type == BOTTYPE_RANDOM || type == BOTTYPE_RANDOMEXCLUDE || type == BOTTYPE_RANDOMARENASCRIPT) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_SetBotSkillValue
=======================================================================================================================================
*/
static void CreateServer_SetBotSkillValue(botskill_t *b, int value) {

	value = (int)Com_Clamp(0, 999, value);
	b->value = (int)Com_Clamp(0, 4, (value / 100) % 10);
	b->low = (int)Com_Clamp(0, 4, (value / 10) % 10);
	b->high = (int)Com_Clamp(0, 4, value % 10);
}

/*
=======================================================================================================================================
CreateServer_SetBotSkillRangeType

Assumes s_scriptdata.bot.typeSelect is already initialized.
=======================================================================================================================================
*/
void CreateServer_SetBotSkillRangeType(int skill) {
	int i;
	qboolean qbool;

	// wrap skill early if selecting from random
	if ((CreateServer_IsRandomBotList(s_scriptdata.bot.typeSelect) || CreateServer_IsBotArenaScript(s_scriptdata.bot.typeSelect)) && skill >= BOTSKILL_CUSTOMSINGLE) {
		skill = BOTSKILL_SAME;
	}

	s_scriptdata.bot.skillType = skill;

	if (skill == BOTSKILL_SAME || skill == BOTSKILL_CUSTOMSINGLE) {
		qbool = qfalse;
	} else {
		qbool = qtrue;
	}

	s_scriptdata.bot.globalSkill.range = qbool;

	for (i = 0; i < PLAYER_SLOTS; i++) {
		s_scriptdata.bot.skill[i].range = qbool;
	}
}

/*
=======================================================================================================================================
SSBP_BotName_Callback
=======================================================================================================================================
*/
static char *SSBP_BotName_Callback(int index) {
	return s_scriptdata.bot.name[index];
}

/*
=======================================================================================================================================
SSBP_BotBuffer_Callback
=======================================================================================================================================
*/
static char *SSBP_BotBuffer_Callback(int index) {
	return botskill_tmpbuffer[index];
}

/*
=======================================================================================================================================
CreateServer_ValidBotCount
=======================================================================================================================================
*/
int CreateServer_ValidBotCount(void) {
	int count = 0, i;

	for (i = 0; i < PLAYER_SLOTS; i++) {
		if (s_scriptdata.bot.slotType[i] != SLOTTYPE_BOT) {
			continue;
		}

		if (s_scriptdata.bot.name[i][0] == '\0') {
			continue;
		}

		count++;
	}

	return count;
}

/*
=======================================================================================================================================
CreateServer_BotOnSelectionList
=======================================================================================================================================
*/
qboolean CreateServer_BotOnSelectionList(const char *checkName) {
	int i;

	for (i = 0; i < PLAYER_SLOTS; i++) {
		if (s_scriptdata.bot.slotType[i] != SLOTTYPE_BOT) {
			continue;
		}

		if (Q_stricmp(checkName, s_scriptdata.bot.name[i]) == 0) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
CreateServer_DeleteBotSlot
=======================================================================================================================================
*/
qboolean CreateServer_DeleteBotSlot(int index) {
	int i, count, last;

	if (index < 0 || index >= PLAYER_SLOTS) {
		return qfalse;
	}
	// number of slots to move
	count = PLAYER_SLOTS - index - 1;

	if (s_scriptdata.gametype > GT_TOURNAMENT && !CreateServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect) && index < PLAYER_SLOTS_PERCOL) {
		count -= PLAYER_SLOTS_PERCOL;
	}

	if (count) {
		for (i = index; i < index + count; i++) {
			memcpy(s_scriptdata.bot.name[i], s_scriptdata.bot.name[i + 1], MAX_NAME_LENGTH);
			memcpy(&s_scriptdata.bot.skill[i], &s_scriptdata.bot.skill[i + 1], sizeof(botskill_t));

			s_scriptdata.bot.slotType[i] = s_scriptdata.bot.slotType[i + 1];
		}
	}
	// zero last slot
	last = index + count;

	memset(s_scriptdata.bot.name[last], 0, MAX_NAME_LENGTH);

	s_scriptdata.bot.skill[last].value = 0;
	s_scriptdata.bot.skill[last].low = 0;
	s_scriptdata.bot.skill[last].high = 0;
	s_scriptdata.bot.slotType[last] = SLOTTYPE_EMPTY;

	return qtrue;
}

/*
=======================================================================================================================================
CreateServer_InsertBotSlot
=======================================================================================================================================
*/
qboolean CreateServer_InsertBotSlot(int index) {
	int i, count;

	if (index < 0 || index >= PLAYER_SLOTS) {
		return qfalse;
	}
	// number of slots to move
	count = PLAYER_SLOTS - index - 1;

	if (s_scriptdata.gametype > GT_TOURNAMENT && index < PLAYER_SLOTS_PERCOL) {
		count -= PLAYER_SLOTS_PERCOL;
	}

	if (count) {
		for (i = index + count - 1; i >= index; i--) {
			// memcpy causes problems when copying "up", so move one entry at a time
			// write this before discovering memmove!
			memcpy(s_scriptdata.bot.name[i + 1], s_scriptdata.bot.name[i], MAX_NAME_LENGTH);
			memcpy(&s_scriptdata.bot.skill[i + 1], &s_scriptdata.bot.skill[i], sizeof(botskill_t));

			s_scriptdata.bot.slotType[i + 1] = s_scriptdata.bot.slotType[i];
		}
	}

	memset(s_scriptdata.bot.name[index], 0, MAX_NAME_LENGTH);

	s_scriptdata.bot.skill[index].value = 0;
	s_scriptdata.bot.skill[index].low = 0;
	s_scriptdata.bot.skill[index].high = 0;
	s_scriptdata.bot.slotType[index] = SLOTTYPE_EMPTY;

	return qtrue;
}

/*
=======================================================================================================================================
CreateServer_SetNamedBot
=======================================================================================================================================
*/
void CreateServer_SetNamedBot(int index, char *name) {

	if (index < 0 || index >= PLAYER_SLOTS || !name) {
		return;
	}

	if (s_scriptdata.bot.name[index][0] == '\0') {
		s_scriptdata.bot.skill[index] = s_scriptdata.bot.globalSkill;
	}

	Q_strncpyz(s_scriptdata.bot.name[index], name, MAX_NAME_LENGTH);
	s_scriptdata.bot.slotType[index] = SLOTTYPE_BOT;
}

/*
=======================================================================================================================================
CreateServer_InsertNamedBot
=======================================================================================================================================
*/
void CreateServer_InsertNamedBot(int index, char *name) {

	if (index < 0 || index >= PLAYER_SLOTS || !name) {
		return;
	}

	CreateServer_InsertBotSlot(index);
	CreateServer_SetNamedBot(index, name);
}

/*
=======================================================================================================================================
CreateServer_SlotTeam
=======================================================================================================================================
*/
int CreateServer_SlotTeam(int index) {

	if (index < 0) {
		return SLOTTEAM_INVALID;
	}

	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		if (index < PLAYER_SLOTS_PERCOL) {
			return SLOTTEAM_ONE;
		}

		if (index < PLAYER_SLOTS) {
			return SLOTTEAM_TWO;
		}
	} else {
		if (index < PLAYER_SLOTS) {
			return SLOTTEAM_NONE;
		}
	}

	return SLOTTEAM_INVALID;
}

/*
=======================================================================================================================================
CreateServer_MoveBotToOtherTeam
=======================================================================================================================================
*/
void CreateServer_MoveBotToOtherTeam(int selected) {
	int i, firstopen, dest, max, start;

	if (selected < 0 || selected >= PLAYER_SLOTS) {
		return;
	}

	if (s_scriptdata.bot.slotType[selected] != SLOTTYPE_BOT) {
		return;
	}
	// try to find an empty slot first
	firstopen = -1;
	max = PLAYER_SLOTS_PERCOL;
	start = 0;

	if (selected < PLAYER_SLOTS_PERCOL) {
		max = PLAYER_SLOTS;
		start = PLAYER_SLOTS_PERCOL;
	}

	dest = -1;

	for (i = start; i < max; i++) {
		if (firstopen == -1 && s_scriptdata.bot.slotType[i] == SLOTTYPE_OPEN) {
			firstopen = i;
		}

		if (s_scriptdata.bot.slotType[i] == SLOTTYPE_EMPTY) {
			dest = i;
			break;
		}
	}
	// use openslot if we have no free ones
	if (dest == -1) {
		if (firstopen == -1) {
			return;
		}

		dest = firstopen;
	}
	// copy over details
	Q_strncpyz(s_scriptdata.bot.name[dest], s_scriptdata.bot.name[selected], MAX_NAME_LENGTH);

	s_scriptdata.bot.slotType[dest] = s_scriptdata.bot.slotType[selected];
	s_scriptdata.bot.skill[dest].value = s_scriptdata.bot.skill[selected].value;
	s_scriptdata.bot.skill[dest].high = s_scriptdata.bot.skill[selected].high;
	s_scriptdata.bot.skill[dest].low = s_scriptdata.bot.skill[selected].low;

	CreateServer_DeleteBotSlot(selected);
}

/*
=======================================================================================================================================
CreateServer_BotNameDrawn
=======================================================================================================================================
*/
void CreateServer_BotNameDrawn(int index, qboolean drawn) {
	s_scriptdata.bot.drawName[index] = drawn;
}

/*
=======================================================================================================================================
CreateServer_DoBotAction
=======================================================================================================================================
*/
void CreateServer_DoBotAction(int action, int selected) {
	int i, count, index, open, slot, bots_done;

	switch (action) {
		case BOT_CT_CLEARALL:
			for (i = 0; i < PLAYER_SLOTS; i++) {
				if (s_scriptdata.bot.slotType[i] == SLOTTYPE_BOT || s_scriptdata.bot.slotType[i] == SLOTTYPE_OPEN || s_scriptdata.bot.slotType[i] == SLOTTYPE_EMPTY) {
					s_scriptdata.bot.slotType[i] = SLOTTYPE_EMPTY;
					CreateServer_BotNameDrawn(i, qfalse);
					memset(s_scriptdata.bot.name[i], 0, MAX_NAME_LENGTH);
				}
			}

			break;
		case BOT_CT_INDIV_SELECTED:
			if (selected >= 0) {
				s_scriptdata.bot.skill[selected].value = s_scriptdata.bot.globalSkill.value;
			}

			break;
		case BOT_CT_RANGE_SELECTED:
			if (selected >= 0) {
				s_scriptdata.bot.skill[selected].low = s_scriptdata.bot.globalSkill.low;
				s_scriptdata.bot.skill[selected].high = s_scriptdata.bot.globalSkill.high;
			}

			break;
		case BOT_CT_INDIV_ALL:
			for (i = 0; i < PLAYER_SLOTS; i++) {
				s_scriptdata.bot.skill[i].value = s_scriptdata.bot.globalSkill.value;
			}

			break;
		case BOT_CT_RANGE_ALL:
			for (i = 0; i < PLAYER_SLOTS; i++) {
				s_scriptdata.bot.skill[i].low = s_scriptdata.bot.globalSkill.low;
				s_scriptdata.bot.skill[i].high = s_scriptdata.bot.globalSkill.high;
			}

			break;
		case BOT_CT_NEATEN:
			// perform two passes when we have a team arrangement once on each column, treated separately
			bots_done = 0;

			do {
				open = 0;
				count = PLAYER_SLOTS;
				index = bots_done;

				if (s_scriptdata.gametype > GT_TOURNAMENT && !CreateServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
					count = PLAYER_SLOTS_PERCOL;
				}
				// compact all the bots, counting the open slots
				while (count > 0) {
					slot = s_scriptdata.bot.slotType[index];

					if (slot == SLOTTYPE_OPEN) {
						open++;
					}

					if (slot == SLOTTYPE_BOT || slot == SLOTTYPE_HUMAN) {
						index++;
					} else {
						CreateServer_DeleteBotSlot(index);
					}

					count--;
					bots_done++;
				}
				// place all the open slots
				for (i = 0; i < open; i++) {
					s_scriptdata.bot.slotType[index] = SLOTTYPE_OPEN;
					index++;
				}
			} while (bots_done < PLAYER_SLOTS);
			break;
	}
}

/*
=======================================================================================================================================
CreateServer_ValidateBotSlotCount
=======================================================================================================================================
*/
void CreateServer_ValidateBotSlotCount(int bots, int open) {

	if (bots < 0) {
		bots = 0;
	}

	if (open < 0) {
		open = 0;
	}

	if (bots > MAX_CLIENTS - 1) {
		bots = MAX_CLIENTS - 1;
	}

	if (open > MAX_CLIENTS - 1) {
		open = MAX_CLIENTS - 1;
	}
	// sacrifice open slots for bots
	if (bots + open > MAX_CLIENTS - 1) {
		open = MAX_CLIENTS - bots - 1;
	}

	s_scriptdata.bot.numberBots = bots;
	s_scriptdata.bot.numberOpen = open;
}

/*
=======================================================================================================================================
CreateServer_AdjustBotSelectionFromGametype

Wraps the bot.typeSelect safely, based on the current gametype.
=======================================================================================================================================
*/
static void CreateServer_AdjustBotSelectionFromGametype(void) {

	if (s_scriptdata.gametype > GT_TOURNAMENT && CreateServer_IsBotArenaScript(s_scriptdata.bot.typeSelect)) {
		s_scriptdata.bot.typeSelect = BOTTYPE_SELECT;
	}
}

/*
=======================================================================================================================================
CreateServer_LoadBotNameList
=======================================================================================================================================
*/
void CreateServer_LoadBotNameList(int type) {
	char *s, *s1;
	int i;

	s_scriptdata.bot.typeSelect = type;
	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];

	memset(&s_scriptdata.bot.name, 0, PLAYER_SLOTS * MAX_NAME_LENGTH);

	for (i = 0; i < PLAYER_SLOTS; i++) {
		s_scriptdata.bot.slotType[i] = SLOTTYPE_EMPTY;
		CreateServer_SetBotSkillValue(&s_scriptdata.bot.skill[i], 0);
	}

	CreateServer_AdjustBotSelectionFromGametype();
	// check if we need to load any bot data at all
	if (s_scriptdata.bot.typeSelect == BOTTYPE_ARENASCRIPT) {
		return;
	}
	// find the right type of bot data to load
	if (CreateServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		s1 = "botexclude";
	} else {
		s1 = "botname";
	}
	// do the load
	UI_LoadMultiArray(s, s1, SSBP_BotName_Callback, PLAYER_SLOTS, MAX_NAME_LENGTH, ';');

	if (CreateServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		for (i = 0; i < PLAYER_SLOTS; i++) {
			if (s_scriptdata.bot.name[i][0]) {
				s_scriptdata.bot.slotType[i] = SLOTTYPE_BOT;
			} else {
				s_scriptdata.bot.slotType[i] = SLOTTYPE_EMPTY;
			}

			CreateServer_SetBotSkillValue(&s_scriptdata.bot.skill[i], 0);
		}
	} else {
		UI_LoadMultiArray(s, "slottype", SSBP_BotBuffer_Callback, PLAYER_SLOTS, BOT_TMPBUFFER, ';');

		for (i = 0; i < PLAYER_SLOTS; i++) {
			s_scriptdata.bot.slotType[i] = (int)Com_Clamp(0, SLOTTYPE_COUNT, atoi(botskill_tmpbuffer[i]));
		}

		UI_LoadMultiArray(s, "botskill", SSBP_BotBuffer_Callback, PLAYER_SLOTS, BOT_TMPBUFFER, ';');

		for (i = 0; i < PLAYER_SLOTS; i++) {
			CreateServer_SetBotSkillValue(&s_scriptdata.bot.skill[i], atoi(botskill_tmpbuffer[i]));
		}

		if (!s_scriptdata.bot.joinAs) {
			s_scriptdata.bot.slotType[0] = SLOTTYPE_HUMAN;
		}
	}
	// set any slot other that first as non-human (only tampering should cause this)
	for (i = 1; i < PLAYER_SLOTS; i++) {
		if (s_scriptdata.bot.slotType[i] == SLOTTYPE_HUMAN) {
			s_scriptdata.bot.slotType[i] = SLOTTYPE_OPEN;
		}
	}
}

/*
=======================================================================================================================================
CreateServer_SaveBotNameList
=======================================================================================================================================
*/
void CreateServer_SaveBotNameList(void) {
	char *s, *s1;
	int i;
	botskill_t *b;
	qboolean exclude;

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];

	if (CreateServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		exclude = qtrue;
		s1 = "botexclude";
	} else {
		exclude = qfalse;
		s1 = "botname";
	}

	UI_SaveMultiArray(s, s1, SSBP_BotName_Callback, PLAYER_SLOTS, MAX_NAME_LENGTH, ';');

	if (!exclude) {
		for (i = 0; i < PLAYER_SLOTS; i++) {
			Q_strncpyz(botskill_tmpbuffer[i], va("%i", s_scriptdata.bot.slotType[i]), BOT_TMPBUFFER);
		}

		UI_SaveMultiArray(s, "slottype", SSBP_BotBuffer_Callback, PLAYER_SLOTS, BOT_TMPBUFFER, ';');

		for (i = 0; i < PLAYER_SLOTS; i++) {
			b = &s_scriptdata.bot.skill[i];
			Q_strncpyz(botskill_tmpbuffer[i], va("%i%i%i", b->value, b->low, b->high), BOT_TMPBUFFER);
		}

		UI_SaveMultiArray(s, "botskill", SSBP_BotBuffer_Callback, PLAYER_SLOTS, BOT_TMPBUFFER, ';');
	}
}

/*
=======================================================================================================================================
CreateServer_LoadBotScriptData

Loads bot specific gametype data.
=======================================================================================================================================
*/
void CreateServer_LoadBotScriptData(void) {
	char *s;

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	// load state values

	// join game as
	s_scriptdata.bot.joinAs = UI_GetSkirmishCvarIntClamp(0, 1, s, "PlayerJoinAs");
	// skill selection method for bots
	// assumes BotSelection method is already set in Start_Server_LoadBotNameList()
	CreateServer_SetBotSkillRangeType(UI_GetSkirmishCvarIntClamp(0, BOTSKILL_COUNT, s, "BotSkillType"));
	// number of bots if randomly generated
	s_scriptdata.bot.numberBots = UI_GetSkirmishCvarIntClamp(0, 99, s, "BotCount");
	// frequency of bot changing on maps
	s_scriptdata.bot.changeBots = UI_GetSkirmishCvarIntClamp(0, BOTCHANGE_COUNT, s, "BotChange");
	// number of open slots if bots are randomly selected
	s_scriptdata.bot.numberOpen = UI_GetSkirmishCvarIntClamp(0, 99, s, "OpenSlotCount");
	// skill range values
	CreateServer_SetBotSkillValue(&s_scriptdata.bot.globalSkill, UI_GetSkirmishCvarInt(s, "BotSkillValue"));
	// skill bias
	s_scriptdata.bot.skillBias = UI_GetSkirmishCvarIntClamp(0, SKILLBIAS_COUNT, s, "BotSkillBias");
	// swap teams
	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		s_scriptdata.bot.teamSwapped = UI_GetSkirmishCvarIntClamp(0, 1, s, "TeamSwapped");
	}
	// load bot stats
	// requires bot.joinAs to be set
	CreateServer_LoadBotNameList(UI_GetSkirmishCvarIntClamp(0, BOTTYPE_MAX, s, "BotSelection"));
}

/*
=======================================================================================================================================
CreateServer_SaveBotScriptData

Saves bot specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_SaveBotScriptData(void) {
	char *s;
	int value;
	botskill_t *b;

	// save state values
	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	// method of selecting bots
	UI_SetSkirmishCvarInt(s, "BotSelection", s_scriptdata.bot.typeSelect);
	// number of bots if randomly generated
	UI_SetSkirmishCvarInt(s, "BotCount", s_scriptdata.bot.numberBots);
	// frequency of bot changing on maps
	UI_SetSkirmishCvarInt(s, "BotChange", s_scriptdata.bot.changeBots);
	// number of open slots if bots are randomly selected
	UI_SetSkirmishCvarInt(s, "OpenSlotCount", s_scriptdata.bot.numberOpen);
	// skill selection method for bots
	UI_SetSkirmishCvarInt(s, "BotSkillType", s_scriptdata.bot.skillType);
	// skill range values
	b = &s_scriptdata.bot.globalSkill;
	value = (b->value * 100) + (b->low * 10) + b->high;

	UI_SetSkirmishCvarInt(s, "BotSkillValue", value);
	// skill bias
	UI_SetSkirmishCvarInt(s, "BotSkillBias", s_scriptdata.bot.skillBias);
	// join game as
	UI_SetSkirmishCvarInt(s, "PlayerJoinAs", s_scriptdata.bot.joinAs);
	// swap teams
	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		UI_SetSkirmishCvarInt(s, "TeamSwapped", s_scriptdata.bot.teamSwapped);
	}
	// bots

	// save bot stats
	CreateServer_SaveBotNameList();
}

/*
=======================================================================================================================================

	LOADING AND SAVING OF ITEM SCRIPT DATA

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CreateServer_LoadDisabledItems

Loads item specific gametype data.
=======================================================================================================================================
*/
void CreateServer_LoadDisabledItems(void) {
	char *s, buffer[256], *ptr, *ptr_last;
	int i;

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	// load the group on/off/custom values
	memset(buffer, 0, sizeof(buffer));

	UI_GetSkirmishCvar(s, "itemGroups", buffer, 256);

	ptr = buffer;

	for (i = 0; i < ITEMGROUP_COUNT; i++) {
		if (*ptr) {
			s_scriptdata.item.groupstate[i] = (int)Com_Clamp(0, ALLGROUPS_HIDDEN, *ptr - '0');
			ptr++;
		} else {
			s_scriptdata.item.groupstate[i] = ALLGROUPS_ENABLED;
		}
	}
	// load individual item values
	// we only load a list of items that are "off"
	// no assumption about order is made
	for (i = 0; i < ITEM_COUNT; i++) {
		s_scriptdata.item.enabled[i] = qtrue;
	}

	memset(buffer, 0, sizeof(buffer));

	UI_GetSkirmishCvar(s, "itemsHidden", buffer, 256);

	ptr = buffer;

	while (*ptr) {
		ptr_last = strchr(ptr, '\\');

		if (!ptr_last) {
			break;
		}

		*ptr_last = '\0';

		for (i = 0; i < ITEM_COUNT; i++) {
			if (!Q_stricmp(ptr, server_itemlist[i].shortitem)) {
				s_scriptdata.item.enabled[i] = qfalse;
				break;
			}
		}
		// move to next char
		ptr = ptr_last + 1;
	}
}

/*
=======================================================================================================================================
CreateServer_SaveItemScriptData

Saves item specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_SaveItemScriptData(void) {
	char *s, buffer[256];
	int i;

	s = gametype_cvar_base[gametype_remap2[s_scriptdata.gametype]];
	// save the group values
	for (i = 0; i < ITEMGROUP_COUNT; i++) {
		buffer[i] = '0' + s_scriptdata.item.groupstate[i];
	}

	buffer[ITEMGROUP_COUNT] = '\0';

	UI_SetSkirmishCvar(s, "itemGroups", buffer);
	// save individual item values
	// we only save a list of items that are "off"
	// always terminate with a slash
	buffer[0] = '\0';

	for (i = 0; i < ITEM_COUNT; i++) {
		if (!s_scriptdata.item.enabled[i]) {
			Q_strcat(buffer, 256, va("%s\\", server_itemlist[i].shortitem));
		}
	}

	UI_SetSkirmishCvar(s, "itemsHidden", buffer);
}

/*
=======================================================================================================================================

	LOADING AND SAVING OF SERVER SCRIPT DATA

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CreateServer_LoadServerScriptData

Loads server specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_LoadServerScriptData(void) {
	char *s, *t;
	int gametype;

	s_scriptdata.server.pureServer = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_pureServer");
	s_scriptdata.server.smoothclients = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_smoothclients");
	s_scriptdata.server.syncClients = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_syncclients");
	s_scriptdata.server.minPing = UI_GetSkirmishCvarIntClamp(0, 999, NULL, "ui_minping");
	s_scriptdata.server.maxPing = UI_GetSkirmishCvarIntClamp(0, 999, NULL, "ui_maxping");
	s_scriptdata.server.allowMinPing = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowMinPing");
	s_scriptdata.server.allowMaxPing = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowMaxPing");
	s_scriptdata.server.allowmaxrate = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowmaxrate");
	s_scriptdata.server.maxrate = UI_GetSkirmishCvarIntClamp(0, 99999, NULL, "ui_maxrate");
	s_scriptdata.server.allowdownload = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowdownload");
	s_scriptdata.server.allowvote = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowvote");
	s_scriptdata.server.allowpass = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_allowpass");
	s_scriptdata.server.gravity = UI_GetSkirmishCvarIntClamp(20, 10000, NULL, "ui_gravity");
	s_scriptdata.server.knockback = UI_GetSkirmishCvarIntClamp(0, 99999, NULL, "ui_knockback");
	s_scriptdata.server.quadfactor = UI_GetSkirmishCvarIntClamp(0, 15, NULL, "ui_quadfactor");
	s_scriptdata.server.netport = UI_GetSkirmishCvarIntClamp(1024, 65535, NULL, "ui_netport");
	s_scriptdata.server.sv_fps = UI_GetSkirmishCvarIntClamp(0, 160, NULL, "ui_svfps");

	UI_GetSkirmishCvar(NULL, "ui_password", s_scriptdata.server.password, MAX_PASSWORD_LENGTH);

	s_scriptdata.server.allowPrivateClients = UI_GetSkirmishCvarIntClamp(0, 32, NULL, "ui_allowprivateclients");
	s_scriptdata.server.privateClients = UI_GetSkirmishCvarIntClamp(0, 32, NULL, "ui_privateclients");

	UI_GetSkirmishCvar(NULL, "ui_privatepassword", s_scriptdata.server.privatePassword, MAX_PASSWORD_LENGTH);

	s_scriptdata.server.preventConfigBug = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_preventConfigBug");

	if (s_scriptdata.multiplayer) {
		s_scriptdata.server.inactivityTime = UI_GetSkirmishCvarIntClamp(0, 999, NULL, "ui_inactivity");
		s_scriptdata.server.strictAuth = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_strictAuth");
		s_scriptdata.server.lanForceRate = UI_GetSkirmishCvarIntClamp(0, 1, NULL, "ui_lanForceRate");
	} else {
		s_scriptdata.server.dedicatedServer = SRVDED_OFF;
		s_scriptdata.server.inactivityTime = 0;
		s_scriptdata.server.strictAuth = 0;
		s_scriptdata.server.lanForceRate = 0;
	}
	// gametype specific values
	gametype = s_scriptdata.gametype;
	s = gametype_cvar_base[gametype_remap2[gametype]];
	// reset server text control
	UI_GetSkirmishCvar(s, "hostname", s_scriptdata.server.hostname, MAX_HOSTNAME_LENGTH);

	s_scriptdata.server.forceRespawn = UI_GetSkirmishCvarIntClamp(0, 999, s, "ForceRespawn");
	s_scriptdata.server.weaponrespawn = UI_GetSkirmishCvarIntClamp(0, 9999, s, "weaponrespawn");

	if (gametype > GT_TOURNAMENT) {
		switch (gametype) {
			case GT_TEAM:
				t = "ui_team_friendly";
				break;
			case GT_CTF:
				t = "ui_ctf_friendly";
				break;
			case GT_1FCTF:
				t = "ui_1flag_friendly";
				break;
			case GT_OBELISK:
				t = "ui_obelisk_friendly";
				break;
			case GT_HARVESTER:
				t = "ui_harvester_friendly";
				break;
			default:
				t = 0;
				break;
		}

		if (t) {
			s_scriptdata.server.friendlyFire = (int)Com_Clamp(0, 1, trap_Cvar_VariableValue(t));
		}

		s_scriptdata.server.autoJoin = UI_GetSkirmishCvarIntClamp(0, 1, s, "AutoJoin");
		s_scriptdata.server.teamBalance = UI_GetSkirmishCvarIntClamp(0, 1, s, "TeamBalance");
	} else {
		s_scriptdata.server.autoJoin = 0;
		s_scriptdata.server.teamBalance = 0;
		s_scriptdata.server.friendlyFire = 0;
	}
}

/*
=======================================================================================================================================
CreateServer_SaveServerScriptData

Saves server specific gametype data.
=======================================================================================================================================
*/
static void CreateServer_SaveServerScriptData(void) {
	char *s;
	int friendly, gametype;

	UI_SetSkirmishCvarInt(NULL, "ui_pureServer", s_scriptdata.server.pureServer);
	UI_SetSkirmishCvarInt(NULL, "ui_smoothclients", s_scriptdata.server.smoothclients);
	UI_SetSkirmishCvarInt(NULL, "ui_syncclients", s_scriptdata.server.syncClients);
	UI_SetSkirmishCvarInt(NULL, "ui_minping", s_scriptdata.server.minPing);
	UI_SetSkirmishCvarInt(NULL, "ui_maxping", s_scriptdata.server.maxPing);
	UI_SetSkirmishCvarInt(NULL, "ui_allowMinPing", s_scriptdata.server.allowMinPing);
	UI_SetSkirmishCvarInt(NULL, "ui_allowMaxPing", s_scriptdata.server.allowMaxPing);
	UI_SetSkirmishCvarInt(NULL, "ui_maxrate", s_scriptdata.server.maxrate);
	UI_SetSkirmishCvarInt(NULL, "ui_allowmaxrate", s_scriptdata.server.allowmaxrate);
	UI_SetSkirmishCvarInt(NULL, "ui_allowdownload", s_scriptdata.server.allowdownload);
	UI_SetSkirmishCvarInt(NULL, "ui_allowvote", s_scriptdata.server.allowvote);
	UI_SetSkirmishCvarInt(NULL, "ui_allowpass", s_scriptdata.server.allowpass);
	UI_SetSkirmishCvarInt(NULL, "ui_gravity", s_scriptdata.server.gravity);
	UI_SetSkirmishCvarInt(NULL, "ui_knockback", s_scriptdata.server.knockback);
	UI_SetSkirmishCvarInt(NULL, "ui_quadfactor", s_scriptdata.server.quadfactor);
	UI_SetSkirmishCvarInt(NULL, "ui_netport", s_scriptdata.server.netport);
	UI_SetSkirmishCvarInt(NULL, "ui_svfps", s_scriptdata.server.sv_fps);
	UI_SetSkirmishCvar(NULL, "ui_password", s_scriptdata.server.password);
	UI_SetSkirmishCvarInt(NULL, "ui_allowprivateclients", s_scriptdata.server.allowPrivateClients);
	UI_SetSkirmishCvarInt(NULL, "ui_privateclients", s_scriptdata.server.privateClients);
	UI_SetSkirmishCvar(NULL, "ui_privatepassword", s_scriptdata.server.privatePassword);
	UI_SetSkirmishCvarInt(NULL, "ui_preventConfigBug", s_scriptdata.server.preventConfigBug);

	if (s_scriptdata.multiplayer) {
		UI_SetSkirmishCvarInt(NULL, "ui_inactivity", s_scriptdata.server.inactivityTime);
		UI_SetSkirmishCvarInt(NULL, "ui_strictAuth", s_scriptdata.server.strictAuth);
		UI_SetSkirmishCvarInt(NULL, "ui_lanForceRate", s_scriptdata.server.lanForceRate);
	}
	// save gametype specific data
	gametype = s_scriptdata.gametype;
	s = gametype_cvar_base[gametype_remap2[gametype]];
	// save state values
	UI_SetSkirmishCvar(s, "hostname", s_scriptdata.server.hostname);
	UI_SetSkirmishCvarInt(s, "ForceRespawn", s_scriptdata.server.forceRespawn);

	if (gametype > GT_TOURNAMENT) {
		// ff is an existing cvar, so we use the existing cvar
		friendly = s_scriptdata.server.friendlyFire;

		switch (gametype) {
			case GT_TEAM:
				trap_Cvar_SetValue("ui_team_friendly", friendly);
				break;
			case GT_CTF:
				trap_Cvar_SetValue("ui_ctf_friendly", friendly);
				break;
			case GT_1FCTF:
				trap_Cvar_SetValue("ui_1flag_friendly", friendly);
				break;
			case GT_OBELISK:
				trap_Cvar_SetValue("ui_obelisk_friendly", friendly);
				break;
			case GT_HARVESTER:
				trap_Cvar_SetValue("ui_harvester_friendly", friendly);
				break;
			default:
				break;
		}

		UI_SetSkirmishCvarInt(s, "AutoJoin", s_scriptdata.server.autoJoin);
		UI_SetSkirmishCvarInt(s, "TeamBalance", s_scriptdata.server.teamBalance);
	}
}

/*
=======================================================================================================================================

	INIT SCRIPT DATA

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CreateServer_LoadScriptDataFromType

Loads script data for the give game type.
=======================================================================================================================================
*/
void CreateServer_LoadScriptDataFromType(int gametype) {

	s_scriptdata.gametype = gametype;

	CreateServer_LoadMapScriptData();
	CreateServer_LoadBotScriptData();
	CreateServer_LoadServerScriptData();
}

/*
=======================================================================================================================================
CreateServer_InitScriptData

Loads all script data.
=======================================================================================================================================
*/
void CreateServer_InitScriptData(qboolean multiplayer) {

	memset(&s_scriptdata, 0, sizeof(scriptdata_t));

	UI_CreateServer_LoadSkirmishCvars();

	s_scriptdata.multiplayer = multiplayer;

	CreateServer_LoadScriptDataFromType((int)Com_Clamp(0, MAX_GAME_TYPE, UI_GetSkirmishCvarInt(NULL, "ui_gametype")));
	CreateServer_BuildMapDistribution();
}

/*
=======================================================================================================================================
CreateServer_SaveScriptData

Saves all script data.
=======================================================================================================================================
*/
void CreateServer_SaveScriptData(void) {

	UI_SetSkirmishCvarInt(NULL, "ui_gametype", s_scriptdata.gametype);

	CreateServer_SaveMapScriptData();
	CreateServer_SaveBotScriptData();
	CreateServer_SaveItemScriptData();
	CreateServer_SaveServerScriptData();

	UI_CreateServer_SaveSkirmishCvars();
}
