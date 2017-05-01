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

/**************************************************************************************************************************************
 Initialization and primary entry point for cgame.
**************************************************************************************************************************************/

#include "cg_local.h"

int forceModelModificationCount = -1;
int redTeamNameModificationCount = -1;
int blueTeamNameModificationCount = -1;
void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum);
void CG_Shutdown(void);
static char *CG_VoIPString(void);

/*
=======================================================================================================================================
vmMain

This is the only way control passes into the module. This must be the very first function compiled into the .q3vm file.
=======================================================================================================================================
*/
Q_EXPORT intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {

	switch (command) {
		case CG_INIT:
			CG_Init(arg0, arg1, arg2);
			return 0;
		case CG_SHUTDOWN:
			CG_Shutdown();
			return 0;
		case CG_CONSOLE_COMMAND:
			return CG_ConsoleCommand();
		case CG_DRAW_ACTIVE_FRAME:
			CG_DrawActiveFrame(arg0, arg1, arg2);
			return 0;
		case CG_CROSSHAIR_PLAYER:
			return CG_CrosshairPlayer();
		case CG_LAST_ATTACKER:
			return CG_LastAttacker();
		case CG_VOIP_STRING:
			return (intptr_t)CG_VoIPString();
		case CG_KEY_EVENT:
			CG_KeyEvent(arg0, arg1);
			return 0;
		case CG_MOUSE_EVENT:
			CG_MouseEvent(arg0, arg1);
			return 0;
		case CG_EVENT_HANDLING:
			CG_EventHandling(arg0);
			return 0;
		default:
			CG_Error("vmMain: unknown command %i", command);
			break;
	}

	return -1;
}

cg_t cg;
cgs_t cgs;
centity_t cg_entities[MAX_GENTITIES];
weaponInfo_t cg_weapons[MAX_WEAPONS];
itemInfo_t cg_items[MAX_ITEMS];

vmCvar_t cg_railTrailTime;
vmCvar_t cg_centertime;
vmCvar_t cg_runpitch;
vmCvar_t cg_runroll;
vmCvar_t cg_bobup;
vmCvar_t cg_bobpitch;
vmCvar_t cg_bobroll;
vmCvar_t cg_swingSpeed;
vmCvar_t cg_shadows;
vmCvar_t cg_statusScale;
vmCvar_t cg_stretch;
vmCvar_t cg_gibs;
vmCvar_t cg_drawTimer;
vmCvar_t cg_drawFPS;
vmCvar_t cg_drawSnapshot;
vmCvar_t cg_draw3dIcons;
vmCvar_t cg_drawIcons;
vmCvar_t cg_drawAmmoWarning;
vmCvar_t cg_hitSounds;
vmCvar_t cg_drawScores;
vmCvar_t cg_drawPickups;
vmCvar_t cg_drawStatusHead;
vmCvar_t cg_drawCrosshair;
vmCvar_t cg_drawCrosshairNames;
vmCvar_t cg_crosshairSize;
vmCvar_t cg_crosshairX;
vmCvar_t cg_crosshairY;
vmCvar_t cg_crosshairHealth;
vmCvar_t cg_draw2D;
vmCvar_t cg_drawStatus;
vmCvar_t cg_animSpeed;
vmCvar_t cg_debugAnim;
vmCvar_t cg_debugPosition;
vmCvar_t cg_debugEvents;
vmCvar_t cg_errorDecay;
vmCvar_t cg_nopredict;
vmCvar_t cg_noPlayerAnims;
vmCvar_t cg_showmiss;
vmCvar_t cg_footsteps;
vmCvar_t cg_addMarks;
vmCvar_t cg_brassTime;
vmCvar_t cg_viewsize;
vmCvar_t cg_drawGun;
vmCvar_t cg_gun_frame;
vmCvar_t cg_gun_x;
vmCvar_t cg_gun_y;
vmCvar_t cg_gun_z;
vmCvar_t cg_tracerChance;
vmCvar_t cg_tracerWidth;
vmCvar_t cg_tracerLength;
vmCvar_t cg_autoswitch;
vmCvar_t cg_ignore;
vmCvar_t cg_simpleItems;
vmCvar_t cg_fov;
vmCvar_t cg_fovAspectAdjust;
vmCvar_t cg_zoomFov;
vmCvar_t cg_fovGunAdjust;
vmCvar_t cg_thirdPerson;
vmCvar_t cg_thirdPersonRange;
vmCvar_t cg_thirdPersonAngle;
vmCvar_t cg_drawLagometer;
vmCvar_t cg_drawAttacker;
vmCvar_t cg_synchronousClients;
vmCvar_t cg_singlePlayer;
vmCvar_t cg_teamChatTime;
vmCvar_t cg_teamChatHeight;
vmCvar_t cg_stats;
vmCvar_t cg_buildScript;
vmCvar_t cg_forceModel;
vmCvar_t cg_paused;
vmCvar_t cg_blood;
vmCvar_t cg_predictItems;
vmCvar_t cg_deferPlayers;
vmCvar_t cg_drawTeamOverlay;
vmCvar_t cg_teamOverlayUserinfo;
vmCvar_t cg_drawFriend;
vmCvar_t cg_teamChatsOnly;
vmCvar_t cg_noVoiceChats;
vmCvar_t cg_noVoiceText;
vmCvar_t cg_scorePlum;
vmCvar_t cg_smoothClients;
vmCvar_t pmove_fixed;
//vmCvar_t cg_pmove_fixed;
vmCvar_t pmove_msec;
vmCvar_t cg_pmove_msec;
vmCvar_t cg_cameraMode;
vmCvar_t cg_cameraOrbit;
vmCvar_t cg_cameraOrbitDelay;
vmCvar_t cg_timescaleFadeEnd;
vmCvar_t cg_timescaleFadeSpeed;
vmCvar_t cg_timescale;
vmCvar_t cg_noTaunt;
vmCvar_t cg_noProjectileTrail;
vmCvar_t cg_oldRail;
vmCvar_t cg_oldRocket;
vmCvar_t cg_oldPlasma;
vmCvar_t cg_trueLightning;
vmCvar_t cg_enableDust;
vmCvar_t cg_enableBreath;
vmCvar_t cg_obeliskRespawnDelay;
#ifdef MISSIONPACK
vmCvar_t cg_redTeamName;
vmCvar_t cg_blueTeamName;
vmCvar_t cg_currentSelectedPlayer;
vmCvar_t cg_currentSelectedPlayerName;
vmCvar_t cg_recordSPDemo;
vmCvar_t cg_recordSPDemoName;
#endif
typedef struct {
	vmCvar_t *vmCvar;
	char *cvarName;
	char *defaultString;
	int cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
	{&cg_ignore, "cg_ignore", "0", 0}, // used for debugging
	{&cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE},
	{&cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE},
	{&cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE},
	{&cg_fov, "cg_fov", "80", CVAR_ARCHIVE},
	{&cg_fovGunAdjust, "cg_fovGunAdjust", "0", CVAR_ARCHIVE},
	{&cg_fovAspectAdjust, "cg_fovAspectAdjust", "0", CVAR_ARCHIVE},
	{&cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE},
	{&cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE},
	{&cg_statusScale, "cg_statusScale", "0.5", CVAR_ARCHIVE},
	{&cg_stretch, "cg_stretch", "0", CVAR_ARCHIVE},
	{&cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE},
	{&cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE},
	{&cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE},
	{&cg_drawTimer, "cg_drawTimer", "1", CVAR_ARCHIVE},
	{&cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE},
	{&cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE},
	{&cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE},
	{&cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE},
	{&cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE},
	{&cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE},
	{&cg_drawLagometer, "cg_drawLagometer", "0", CVAR_ARCHIVE},
	{&cg_hitSounds, "cg_hitSounds", "0", CVAR_ARCHIVE},
	{&cg_drawScores, "cg_drawScores", "1", CVAR_ARCHIVE},
	{&cg_drawPickups, "cg_drawPickups", "1", CVAR_ARCHIVE},
	{&cg_drawStatusHead, "cg_drawStatusHead", "1", CVAR_ARCHIVE},
	{&cg_drawCrosshair, "cg_drawCrosshair", "2", CVAR_ARCHIVE},
	{&cg_drawCrosshairNames, "cg_drawCrosshairNames", "0", CVAR_ARCHIVE},
	{&cg_crosshairSize, "cg_crosshairSize", "11", CVAR_ARCHIVE},
	{&cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE},
	{&cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE},
	{&cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE},
	{&cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE},
	{&cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE},
	{&cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE},
	{&cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE},
	{&cg_gun_x, "cg_gunX", "0", CVAR_CHEAT},
	{&cg_gun_y, "cg_gunY", "0", CVAR_CHEAT},
	{&cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT},
	{&cg_centertime, "cg_centertime", "3", CVAR_CHEAT},
	{&cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{&cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE},
	{&cg_bobup, "cg_bobup", "0.005", CVAR_CHEAT},
	{&cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE},
	{&cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE},
	{&cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT},
	{&cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT},
	{&cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT},
	{&cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT},
	{&cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT},
	{&cg_errorDecay, "cg_errordecay", "100", 0},
	{&cg_nopredict, "cg_nopredict", "0", 0},
	{&cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT},
	{&cg_showmiss, "cg_showmiss", "0", 0},
	{&cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT},
	{&cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT},
	{&cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT},
	{&cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT},
	{&cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT},
	{&cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT},
	{&cg_thirdPerson, "cg_thirdPerson", "0", 0},
	{&cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE},
	{&cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE},
	{&cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE},
	{&cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE},
	{&cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE},
	{&cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE},
	{&cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM|CVAR_USERINFO},
	{&cg_stats, "cg_stats", "0", 0},
	{&cg_drawFriend, "cg_drawFriend", "0", CVAR_ARCHIVE},
	{&cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE},
	{&cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE},
	{&cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE},
	// the following variables are created in other parts of the system, but we also reference them here
	{&cg_buildScript, "com_buildScript", "0", 0}, // force loading of all possible data amd error on failures
	{&cg_paused, "cl_paused", "0", CVAR_ROM},
	{&cg_blood, "com_blood", "1", CVAR_ARCHIVE},
	{&cg_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO},
	{&cg_enableDust, "cg_enableDust", "0", 0},
	{&cg_enableBreath, "cg_enableBreath", "0", 0},
	{&cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SYSTEMINFO},
#ifdef MISSIONPACK
	{&cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE|CVAR_SYSTEMINFO},
	{&cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE|CVAR_SYSTEMINFO},
	{&cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{&cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{&cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{&cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE},
#endif
	{&cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_SYSTEMINFO|CVAR_ROM},
	{&cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{&cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{&cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{&cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{&cg_timescale, "timescale", "1", 0},
	{&cg_scorePlum, "cg_scorePlums", "0", CVAR_USERINFO|CVAR_ARCHIVE},
	{&cg_smoothClients, "cg_smoothClients", "1", CVAR_USERINFO|CVAR_ARCHIVE},
	{&cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},
	{&pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO},
	{&pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO},
	{&cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{&cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
	{&cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE},
	{&cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE},
	{&cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE},
	{&cg_trueLightning, "cg_trueLightning", "1.0", CVAR_ARCHIVE}
//	{&cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO|CVAR_ARCHIVE}
};

static int cvarTableSize = ARRAY_LEN(cvarTable);

/*
=======================================================================================================================================
CG_RegisterCvars
=======================================================================================================================================
*/
void CG_RegisterCvars(void) {
	int i;
	cvarTable_t *cv;
	char var[MAX_TOKEN_CHARS];

	for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
		trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
	}
	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer("sv_running", var, sizeof(var));
	cgs.localServer = atoi(var);

	forceModelModificationCount = cg_forceModel.modificationCount;
#ifdef MISSIONPACK
	redTeamNameModificationCount = cg_redTeamName.modificationCount;
	blueTeamNameModificationCount = cg_blueTeamName.modificationCount;
#endif
	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO|CVAR_ARCHIVE);
	trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO|CVAR_ARCHIVE);
	trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO|CVAR_ARCHIVE);
	trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO|CVAR_ARCHIVE);
}

/*
=======================================================================================================================================
CG_ForceModelChange
=======================================================================================================================================
*/
static void CG_ForceModelChange(void) {
	int i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		const char *clientInfo;

		clientInfo = CG_ConfigString(CS_PLAYERS + i);

		if (!clientInfo[0]) {
			continue;
		}

		CG_NewClientInfo(i);
	}
}

/*
=======================================================================================================================================
CG_UpdateCvars
=======================================================================================================================================
*/
void CG_UpdateCvars(void) {
	int i;
	cvarTable_t *cv;

	for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
		trap_Cvar_Update(cv->vmCvar);
	}
	// check for modications here

	// If team overlay is on, ask for updates from the server. If it's off, let the server know so we don't receive it
	if (drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if (cg_drawTeamOverlay.integer > 0) {
			trap_Cvar_SetValue("teamoverlay", 1);
		} else {
			trap_Cvar_SetValue("teamoverlay", 0);
		}
	}
	// if force model or a team name changed
	if (forceModelModificationCount != cg_forceModel.modificationCount
#ifdef MISSIONPACK
		|| redTeamNameModificationCount != cg_redTeamName.modificationCount || blueTeamNameModificationCount != cg_blueTeamName.modificationCount
#endif
		) {

		forceModelModificationCount = cg_forceModel.modificationCount;
#ifdef MISSIONPACK
		redTeamNameModificationCount = cg_redTeamName.modificationCount;
		blueTeamNameModificationCount = cg_blueTeamName.modificationCount;
#endif
		CG_ForceModelChange();
	}
}

/*
=======================================================================================================================================
CG_CrosshairPlayer
=======================================================================================================================================
*/
int CG_CrosshairPlayer(void) {

	if (cg.time > (cg.crosshairClientTime + 1000)) {
		return -1;
	}

	return cg.crosshairClientNum;
}

/*
=======================================================================================================================================
CG_LastAttacker
=======================================================================================================================================
*/
int CG_LastAttacker(void) {

	if (!cg.attackerTime) {
		return -1;
	}

	return cg.snap->ps.persistant[PERS_ATTACKER];
}

/*
=======================================================================================================================================
CG_Printf
=======================================================================================================================================
*/
void QDECL CG_Printf(const char *msg, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	trap_Print(text);
}

/*
=======================================================================================================================================
CG_Error
=======================================================================================================================================
*/
void QDECL CG_Error(const char *msg, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	trap_Error(text);
}

/*
=======================================================================================================================================
Com_Error
=======================================================================================================================================
*/
void QDECL Com_Error(int level, const char *error, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	Q_vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	trap_Error(text);
}

/*
=======================================================================================================================================
Com_Printf
=======================================================================================================================================
*/
void QDECL Com_Printf(const char *msg, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	trap_Print(text);
}

/*
=======================================================================================================================================
CG_Argv
=======================================================================================================================================
*/
const char *CG_Argv(int arg) {
	static char buffer[MAX_STRING_CHARS];

	trap_Argv(arg, buffer, sizeof(buffer));
	return buffer;
}

/*
=======================================================================================================================================
CG_RegisterItemSounds

The server says this item is used on this level.
=======================================================================================================================================
*/
static void CG_RegisterItemSounds(int itemNum) {
	gitem_t *item;
	char data[MAX_QPATH];
	char *s, *start;
	int len;

	item = &bg_itemlist[itemNum];

	if (item->pickup_sound) {
		cgs.media.itemPickupSounds[itemNum] = trap_S_RegisterSound(item->pickup_sound, qfalse);
	}
	// parse the space separated precache string for other media
	s = item->sounds;

	if (!s || !s[0]) {
		return;
	}

	while (*s) {
		start = s;

		while (*s && *s != ' ') {
			s++;
		}

		len = s - start;

		if (len >= MAX_QPATH || len < 5) {
			CG_Error("PrecacheItem: %s has bad precache string", item->classname);
			return;
		}

		memcpy(data, start, len);

		data[len] = 0;

		if (*s) {
			s++;
		}

		if (!strcmp(data + len - 3, "wav")) {
			trap_S_RegisterSound(data, qfalse);
		}
	}
}

/*
=======================================================================================================================================
CG_RegisterSounds

Called during a precache command.
=======================================================================================================================================
*/
static void CG_RegisterSounds(void) {
	int i;
	char items[MAX_ITEMS + 1];
	char name[MAX_QPATH];
	const char *soundName;

	// voice commands
	CG_LoadVoiceChats();

	cgs.media.oneMinuteSound = trap_S_RegisterSound("sound/feedback/1_minute.wav", qtrue);
	cgs.media.fiveMinuteSound = trap_S_RegisterSound("sound/feedback/5_minute.wav", qtrue);
	cgs.media.suddenDeathSound = trap_S_RegisterSound("sound/feedback/sudden_death.wav", qtrue);
	cgs.media.oneFragSound = trap_S_RegisterSound("sound/feedback/1_frag.wav", qtrue);
	cgs.media.twoFragSound = trap_S_RegisterSound("sound/feedback/2_frags.wav", qtrue);
	cgs.media.threeFragSound = trap_S_RegisterSound("sound/feedback/3_frags.wav", qtrue);
	cgs.media.count3Sound = trap_S_RegisterSound("sound/feedback/three.wav", qtrue);
	cgs.media.count2Sound = trap_S_RegisterSound("sound/feedback/two.wav", qtrue);
	cgs.media.count1Sound = trap_S_RegisterSound("sound/feedback/one.wav", qtrue);
	cgs.media.countFightSound = trap_S_RegisterSound("sound/feedback/fight.wav", qtrue);
	cgs.media.countPrepareSound = trap_S_RegisterSound("sound/feedback/prepare.wav", qtrue);
	cgs.media.countPrepareTeamSound = trap_S_RegisterSound("sound/feedback/prepare_team.wav", qtrue);

	if (cgs.gametype > GT_TOURNAMENT || cg_buildScript.integer) {
		cgs.media.captureAwardSound = trap_S_RegisterSound("sound/teamplay/flagcapture_yourteam.wav", qtrue);
		cgs.media.redLeadsSound = trap_S_RegisterSound("sound/feedback/redleads.wav", qtrue);
		cgs.media.blueLeadsSound = trap_S_RegisterSound("sound/feedback/blueleads.wav", qtrue);
		cgs.media.teamsTiedSound = trap_S_RegisterSound("sound/feedback/teamstied.wav", qtrue);
		cgs.media.hitTeamSound = trap_S_RegisterSound("sound/feedback/hit_teammate.wav", qtrue);
		cgs.media.redScoredSound = trap_S_RegisterSound("sound/teamplay/voc_red_scores.wav", qtrue);
		cgs.media.blueScoredSound = trap_S_RegisterSound("sound/teamplay/voc_blue_scores.wav", qtrue);
		cgs.media.captureYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagcapture_yourteam.wav", qtrue);
		cgs.media.captureOpponentSound = trap_S_RegisterSound("sound/teamplay/flagcapture_opponent.wav", qtrue);
		cgs.media.returnYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagreturn_yourteam.wav", qtrue);
		cgs.media.returnOpponentSound = trap_S_RegisterSound("sound/teamplay/flagreturn_opponent.wav", qtrue);
		cgs.media.takenYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagtaken_yourteam.wav", qtrue);
		cgs.media.takenOpponentSound = trap_S_RegisterSound("sound/teamplay/flagtaken_opponent.wav", qtrue);

		if (cgs.gametype == GT_CTF || cg_buildScript.integer) {
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/voc_red_returned.wav", qtrue);
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/voc_blue_returned.wav", qtrue);
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound("sound/teamplay/voc_enemy_flag.wav", qtrue);
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_flag.wav", qtrue);
		}

		if (cgs.gametype == GT_1FCTF || cg_buildScript.integer) {
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/flagreturn_opponent.wav", qtrue);
			cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_1flag.wav", qtrue);
			cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_enemy_1flag.wav", qtrue);
		}

		if (cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cg_buildScript.integer) {
			cgs.media.youHaveFlagSound = trap_S_RegisterSound("sound/teamplay/voc_you_flag.wav", qtrue);
			cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qtrue);
		}

		if (cgs.gametype == GT_OBELISK || cg_buildScript.integer) {
			cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound("sound/teamplay/voc_base_attack.wav", qtrue);
		}
	}

	cgs.media.tracerSound = trap_S_RegisterSound("sound/weapons/machinegun/buletby1.wav", qfalse);
	cgs.media.selectSound = trap_S_RegisterSound("sound/weapons/change.wav", qfalse);
	cgs.media.wearOffSound = trap_S_RegisterSound("sound/items/wearoff.wav", qfalse);
	cgs.media.useNothingSound = trap_S_RegisterSound("sound/items/use_nothing.wav", qfalse);
	cgs.media.gibSound = trap_S_RegisterSound("sound/player/gibsplt1.wav", qfalse);
	cgs.media.gibBounce1Sound = trap_S_RegisterSound("sound/player/gibimp1.wav", qfalse);
	cgs.media.gibBounce2Sound = trap_S_RegisterSound("sound/player/gibimp2.wav", qfalse);
	cgs.media.gibBounce3Sound = trap_S_RegisterSound("sound/player/gibimp3.wav", qfalse);
	cgs.media.obeliskHitSound1 = trap_S_RegisterSound("sound/items/obelisk_hit_01.wav", qfalse);
	cgs.media.obeliskHitSound2 = trap_S_RegisterSound("sound/items/obelisk_hit_02.wav", qfalse);
	cgs.media.obeliskHitSound3 = trap_S_RegisterSound("sound/items/obelisk_hit_03.wav", qfalse);
	cgs.media.obeliskRespawnSound = trap_S_RegisterSound("sound/items/obelisk_respawn.wav", qfalse);
	cgs.media.ammoregenSound = trap_S_RegisterSound("sound/items/cl_ammoregen.wav", qfalse);
	cgs.media.doublerSound = trap_S_RegisterSound("sound/items/cl_doubler.wav", qfalse);
	cgs.media.guardSound = trap_S_RegisterSound("sound/items/cl_guard.wav", qfalse);
	cgs.media.scoutSound = trap_S_RegisterSound("sound/items/cl_scout.wav", qfalse);
	cgs.media.teleInSound = trap_S_RegisterSound("sound/world/telein.wav", qfalse);
	cgs.media.teleOutSound = trap_S_RegisterSound("sound/world/teleout.wav", qfalse);
	cgs.media.respawnSound = trap_S_RegisterSound("sound/items/respawn1.wav", qfalse);
	cgs.media.noAmmoSound = trap_S_RegisterSound("sound/weapons/noammo.wav", qfalse);
	cgs.media.talkSound = trap_S_RegisterSound("sound/player/talk.wav", qfalse);
	cgs.media.landSound = trap_S_RegisterSound("sound/player/land1.wav", qfalse);
	cgs.media.hitSound = trap_S_RegisterSound("sound/feedback/hit.wav", qfalse);
	cgs.media.hitSoundHighArmor = trap_S_RegisterSound("sound/feedback/hithi.wav", qfalse);
	cgs.media.hitSoundLowArmor = trap_S_RegisterSound("sound/feedback/hitlo.wav", qfalse);
	cgs.media.impressiveSound = trap_S_RegisterSound("sound/feedback/impressive.wav", qtrue);
	cgs.media.excellentSound = trap_S_RegisterSound("sound/feedback/excellent.wav", qtrue);
	cgs.media.deniedSound = trap_S_RegisterSound("sound/feedback/denied.wav", qtrue);
	cgs.media.humiliationSound = trap_S_RegisterSound("sound/feedback/humiliation.wav", qtrue);
	cgs.media.assistSound = trap_S_RegisterSound("sound/feedback/assist.wav", qtrue);
	cgs.media.defendSound = trap_S_RegisterSound("sound/feedback/defense.wav", qtrue);
	cgs.media.firstImpressiveSound = trap_S_RegisterSound("sound/feedback/first_impressive.wav", qtrue);
	cgs.media.firstExcellentSound = trap_S_RegisterSound("sound/feedback/first_excellent.wav", qtrue);
	cgs.media.firstHumiliationSound = trap_S_RegisterSound("sound/feedback/first_gauntlet.wav", qtrue);
	cgs.media.takenLeadSound = trap_S_RegisterSound("sound/feedback/takenlead.wav", qtrue);
	cgs.media.tiedLeadSound = trap_S_RegisterSound("sound/feedback/tiedlead.wav", qtrue);
	cgs.media.lostLeadSound = trap_S_RegisterSound("sound/feedback/lostlead.wav", qtrue);
	cgs.media.voteNow = trap_S_RegisterSound("sound/feedback/vote_now.wav", qtrue);
	cgs.media.votePassed = trap_S_RegisterSound("sound/feedback/vote_passed.wav", qtrue);
	cgs.media.voteFailed = trap_S_RegisterSound("sound/feedback/vote_failed.wav", qtrue);
	cgs.media.watrInSound = trap_S_RegisterSound("sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound("sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound("sound/player/watr_un.wav", qfalse);
	cgs.media.jumpPadSound = trap_S_RegisterSound("sound/world/jumppad.wav", qfalse);

	for (i = 0; i < 4; i++) {
		Com_sprintf(name, sizeof(name), "sound/player/footsteps/step%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/boot%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/mech%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/energy%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/splash%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound(name, qfalse);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/clank%i.wav", i + 1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound(name, qfalse);
	}
	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for (i = 1; i < bg_numItems; i++) {
//		if (items[i] == '1' || cg_buildScript.integer) {
			CG_RegisterItemSounds(i);
//		}
	}

	for (i = 1; i < MAX_SOUNDS; i++) {
		soundName = CG_ConfigString(CS_SOUNDS + i);

		if (!soundName[0]) {
			break;
		}

		if (soundName[0] == '*') {
			continue; // custom sound
		}

		cgs.gameSounds[i] = trap_S_RegisterSound(soundName, qfalse);
	}
	// FIXME: only needed with item
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav", qfalse);
	cgs.media.sfx_ric1 = trap_S_RegisterSound("sound/weapons/machinegun/ric1.wav", qfalse);
	cgs.media.sfx_ric2 = trap_S_RegisterSound("sound/weapons/machinegun/ric2.wav", qfalse);
	cgs.media.sfx_ric3 = trap_S_RegisterSound("sound/weapons/machinegun/ric3.wav", qfalse);
	//cgs.media.sfx_railg = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.wav", qfalse);
	cgs.media.sfx_rockexp = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.wav", qfalse);
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound("sound/weapons/plasma/plasmx1a.wav", qfalse);
	cgs.media.sfx_proxexp = trap_S_RegisterSound("sound/weapons/proxmine/wstbexpl.wav", qfalse);
	cgs.media.sfx_nghit = trap_S_RegisterSound("sound/weapons/nailgun/wnalimpd.wav", qfalse);
	cgs.media.sfx_chghit = trap_S_RegisterSound("sound/weapons/vulcan/wvulimpd.wav", qfalse);
	cgs.media.sfx_chgstop = trap_S_RegisterSound("sound/weapons/vulcan/wvulwind.wav", qfalse);
	cgs.media.sfx_hmgstop = trap_S_RegisterSound("sound/weapons/hmg/hmgwind.wav", qfalse);
	cgs.media.kamikazeExplodeSound = trap_S_RegisterSound("sound/items/kam_explode.wav", qfalse);
	cgs.media.kamikazeImplodeSound = trap_S_RegisterSound("sound/items/kam_implode.wav", qfalse);
	cgs.media.kamikazeFarSound = trap_S_RegisterSound("sound/items/kam_explode_far.wav", qfalse);
	cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
	cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);
#ifdef MISSIONPACK
	cgs.media.winnerSound = trap_S_RegisterSound("sound/feedback/voc_youwin.wav", qfalse);
	cgs.media.loserSound = trap_S_RegisterSound("sound/feedback/voc_youlose.wav", qfalse);
#endif
	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav", qfalse);
	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse);
	cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav", qfalse);
	cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav", qfalse);
#ifdef MISSIONPACK
	trap_S_RegisterSound("sound/player/james/death1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/death2.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/death3.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/jump1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/pain25_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/pain75_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/pain100_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/falling1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/gasp.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/drown.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/fall1.wav", qfalse);
	trap_S_RegisterSound("sound/player/james/taunt.wav", qfalse);

	trap_S_RegisterSound("sound/player/janet/death1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/death2.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/death3.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/jump1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/pain25_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/pain75_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/pain100_1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/falling1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/gasp.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/drown.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/fall1.wav", qfalse);
	trap_S_RegisterSound("sound/player/janet/taunt.wav", qfalse);
#endif
}

/*
=======================================================================================================================================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=======================================================================================================================================
*/
static void CG_RegisterGraphics(void) {
	int i;
	char items[MAX_ITEMS + 1];
	static char *sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};
	// clear any references to old media
	memset(&cg.refdef, 0, sizeof(cg.refdef));

	trap_R_ClearScene();

	CG_LoadingString(cgs.mapname);

	trap_R_LoadWorldMap(cgs.mapname);
	// precache status bar pics
	CG_LoadingString("game media");

	for (i = 0; i < 11; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader(sb_nums[i]);
	}

	cgs.media.botSkillShaders[0] = trap_R_RegisterShader("menu/art/skill1.tga");
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader("menu/art/skill2.tga");
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader("menu/art/skill3.tga");
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader("menu/art/skill4.tga");
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader("menu/art/skill5.tga");
	cgs.media.viewBloodShader = trap_R_RegisterShader("viewBloodBlend");
	cgs.media.deferShader = trap_R_RegisterShaderNoMip("gfx/2d/defer.tga");
	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip("menu/tab/name.tga");
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip("menu/tab/ping.tga");
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip("menu/tab/score.tga");
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip("menu/tab/time.tga");
	cgs.media.smokePuffShader = trap_R_RegisterShader("smokePuff");
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader("smokePuffRagePro");
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader("shotgunSmokePuff");
	cgs.media.nailPuffShader = trap_R_RegisterShader("nailtrail");
	cgs.media.blueProxMine = trap_R_RegisterModel("models/weaphits/proxmineb.md3");
	cgs.media.plasmaBallShader = trap_R_RegisterShader("sprites/plasma1");
	cgs.media.bloodTrailShader = trap_R_RegisterShader("bloodTrail");
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer");
	cgs.media.connectionShader = trap_R_RegisterShader("disconnected");
	cgs.media.waterBubbleShader = trap_R_RegisterShader("waterBubble");
	cgs.media.tracerShader = trap_R_RegisterShader("gfx/misc/tracer");
	cgs.media.selectShader = trap_R_RegisterShader("gfx/2d/select");

	for (i = 0; i < NUM_CROSSHAIRS; i++) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader(va("gfx/2d/crosshair%c", 'a' + i));
	}

	cgs.media.backTileShader = trap_R_RegisterShader("gfx/2d/backtile");
	cgs.media.noammoShader = trap_R_RegisterShader("icons/noammo");
	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad");
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon");
	cgs.media.invisRedShader = trap_R_RegisterShader("powerups/invisibilityRed");
	cgs.media.invisBlueShader = trap_R_RegisterShader("powerups/invisibilityBlue");
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility");
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen");

	if (cgs.gametype == GT_HARVESTER || cg_buildScript.integer) {
		cgs.media.redCubeModel = trap_R_RegisterModel("models/powerups/orb/r_orb.md3");
		cgs.media.blueCubeModel = trap_R_RegisterModel("models/powerups/orb/b_orb.md3");
		cgs.media.redCubeIcon = trap_R_RegisterShader("icons/skull_red");
		cgs.media.blueCubeIcon = trap_R_RegisterShader("icons/skull_blue");
	}

	if (cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer) {
		cgs.media.redFlagModel = trap_R_RegisterModel("models/flags/r_flag.md3");
		cgs.media.blueFlagModel = trap_R_RegisterModel("models/flags/b_flag.md3");
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_red1");
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_red2");
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_red3");
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_blu1");
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_blu2");
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_blu3");
		cgs.media.flagPoleModel = trap_R_RegisterModel("models/flag2/flagpole.md3");
		cgs.media.flagFlapModel = trap_R_RegisterModel("models/flag2/flagflap3.md3");
		cgs.media.redFlagFlapSkin = trap_R_RegisterSkin("models/flag2/red.skin");
		cgs.media.blueFlagFlapSkin = trap_R_RegisterSkin("models/flag2/blue.skin");
		cgs.media.neutralFlagFlapSkin = trap_R_RegisterSkin("models/flag2/white.skin");
		cgs.media.redFlagBaseModel = trap_R_RegisterModel("models/mapobjects/flagbase/red_base.md3");
		cgs.media.blueFlagBaseModel = trap_R_RegisterModel("models/mapobjects/flagbase/blue_base.md3");
		cgs.media.neutralFlagBaseModel = trap_R_RegisterModel("models/mapobjects/flagbase/ntrl_base.md3");
	}

	if (cgs.gametype == GT_1FCTF || cg_buildScript.integer) {
		cgs.media.neutralFlagModel = trap_R_RegisterModel("models/flags/n_flag.md3");
		cgs.media.flagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_neutral1");
		cgs.media.flagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_red2");
		cgs.media.flagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_blu2");
		cgs.media.flagShader[3] = trap_R_RegisterShaderNoMip("icons/iconf_neutral3");
	}

	if (cgs.gametype == GT_OBELISK || cg_buildScript.integer) {
		cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");
		cgs.media.overloadBaseModel = trap_R_RegisterModel("models/powerups/overload_base.md3");
		cgs.media.overloadTargetModel = trap_R_RegisterModel("models/powerups/overload_target.md3");
		cgs.media.overloadLightsModel = trap_R_RegisterModel("models/powerups/overload_lights.md3");
		cgs.media.overloadEnergyModel = trap_R_RegisterModel("models/powerups/overload_energy.md3");
	}

	if (cgs.gametype == GT_HARVESTER || cg_buildScript.integer) {
		cgs.media.harvesterModel = trap_R_RegisterModel("models/powerups/harvester/harvester.md3");
		cgs.media.harvesterRedSkin = trap_R_RegisterSkin("models/powerups/harvester/red.skin");
		cgs.media.harvesterBlueSkin = trap_R_RegisterSkin("models/powerups/harvester/blue.skin");
		cgs.media.harvesterNeutralModel = trap_R_RegisterModel("models/powerups/obelisk/obelisk.md3");
	}

	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff");

	if (cgs.gametype > GT_TOURNAMENT || cg_buildScript.integer) {
		cgs.media.redFriendShader = trap_R_RegisterShader("sprites/team_red");
		cgs.media.blueFriendShader = trap_R_RegisterShader("sprites/team_blue");
		cgs.media.teamStatusBar = trap_R_RegisterShader("gfx/2d/colorbar.tga");
	}

	cgs.media.armorModel = trap_R_RegisterModel("models/powerups/armor/armor_yel.md3");
	cgs.media.armorIcon = trap_R_RegisterShaderNoMip("icons/iconr_yellow");
	cgs.media.machinegunBrassModel = trap_R_RegisterModel("models/weapons2/shells/m_shell.md3");
	cgs.media.shotgunBrassModel = trap_R_RegisterModel("models/weapons2/shells/s_shell.md3");
	cgs.media.gibAbdomen = trap_R_RegisterModel("models/gibs/abdomen.md3");
	cgs.media.gibArm = trap_R_RegisterModel("models/gibs/arm.md3");
	cgs.media.gibChest = trap_R_RegisterModel("models/gibs/chest.md3");
	cgs.media.gibFist = trap_R_RegisterModel("models/gibs/fist.md3");
	cgs.media.gibFoot = trap_R_RegisterModel("models/gibs/foot.md3");
	cgs.media.gibForearm = trap_R_RegisterModel("models/gibs/forearm.md3");
	cgs.media.gibIntestine = trap_R_RegisterModel("models/gibs/intestine.md3");
	cgs.media.gibLeg = trap_R_RegisterModel("models/gibs/leg.md3");
	cgs.media.gibSkull = trap_R_RegisterModel("models/gibs/skull.md3");
	cgs.media.gibBrain = trap_R_RegisterModel("models/gibs/brain.md3");
	cgs.media.smoke2 = trap_R_RegisterModel("models/weapons2/shells/s_shell.md3");
	cgs.media.balloonShader = trap_R_RegisterShader("sprites/balloon3");
	cgs.media.bloodExplosionShader = trap_R_RegisterShader("bloodExplosion");
	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
	cgs.media.teleportEffectModel = trap_R_RegisterModel("models/misc/telep.md3");
	cgs.media.teleportEffectShader = trap_R_RegisterShader("teleportEffect");
	cgs.media.kamikazeEffectModel = trap_R_RegisterModel("models/weaphits/kamboom2.md3");
	cgs.media.kamikazeShockWave = trap_R_RegisterModel("models/weaphits/kamwave.md3");
	cgs.media.kamikazeHeadModel = trap_R_RegisterModel("models/powerups/kamikazi.md3");
	cgs.media.kamikazeHeadTrail = trap_R_RegisterModel("models/powerups/trailtest.md3");
	cgs.media.guardPowerupModel = trap_R_RegisterModel("models/powerups/guard_player.md3");
	cgs.media.scoutPowerupModel = trap_R_RegisterModel("models/powerups/scout_player.md3");
	cgs.media.doublerPowerupModel = trap_R_RegisterModel("models/powerups/doubler_player.md3");
	cgs.media.ammoRegenPowerupModel = trap_R_RegisterModel("models/powerups/ammo_player.md3");
	cgs.media.medalAccuracy = trap_R_RegisterShaderNoMip("medal_accuracy");
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip("medal_excellent");
	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip("medal_impressive");
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip("medal_gauntlet");
	cgs.media.medalCapture = trap_R_RegisterShaderNoMip("medal_capture");
	cgs.media.medalDefend = trap_R_RegisterShaderNoMip("medal_defend");
	cgs.media.medalAssist = trap_R_RegisterShaderNoMip("medal_assist");

	memset(cg_items, 0, sizeof(cg_items));
	memset(cg_weapons, 0, sizeof(cg_weapons));
	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for (i = 1; i < bg_numItems; i++) {
		if (items[i] == '1' || cg_buildScript.integer) {
			CG_LoadingItem(i);
			CG_RegisterItemVisuals(i);
		}
	}
	// can be used by HUD so always load it
	CG_RegisterItemVisuals(3);
	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader("gfx/damage/bullet_mrk");
	cgs.media.burnMarkShader = trap_R_RegisterShader("gfx/damage/burn_med_mrk");
	cgs.media.holeMarkShader = trap_R_RegisterShader("gfx/damage/hole_lg_mrk");
	cgs.media.energyMarkShader = trap_R_RegisterShader("gfx/damage/plasma_mrk");
	cgs.media.shadowMarkShader = trap_R_RegisterShader("markShadow");
	cgs.media.wakeMarkShader = trap_R_RegisterShader("wake");
	cgs.media.bloodMarkShader = trap_R_RegisterShader("bloodMark");
	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();

	if (cgs.numInlineModels > MAX_SUBMODELS) {
		CG_Error("MAX_SUBMODELS(%d) exceeded by %d", MAX_SUBMODELS, cgs.numInlineModels - MAX_SUBMODELS);
	}

	for (i = 1; i < cgs.numInlineModels; i++) {
		char name[10];
		vec3_t mins, maxs;
		int j;

		Com_sprintf(name, sizeof(name), "*%i", i);
		cgs.inlineDrawModel[i] = trap_R_RegisterModel(name);
		trap_R_ModelBounds(cgs.inlineDrawModel[i], mins, maxs);

		for (j = 0; j < 3; j++) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * (maxs[j] - mins[j]);
		}
	}
	// register all the server specified models
	for (i = 1; i < MAX_MODELS; i++) {
		const char *modelName;

		modelName = CG_ConfigString(CS_MODELS + i);

		if (!modelName[0]) {
			break;
		}

		cgs.gameModels[i] = trap_R_RegisterModel(modelName);
	}

	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");
	// task shaders
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
#ifdef MISSIONPACK
	trap_R_RegisterModel("models/players/james/lower.md3");
	trap_R_RegisterModel("models/players/james/upper.md3");
	trap_R_RegisterModel("models/players/heads/james/james.md3");

	trap_R_RegisterModel("models/players/janet/lower.md3");
	trap_R_RegisterModel("models/players/janet/upper.md3");
	trap_R_RegisterModel("models/players/heads/janet/janet.md3");
#endif
	CG_ClearParticles();
/*
	for (i = 1; i < MAX_PARTICLES_AREAS; i++) {
		int rval;

		rval = CG_NewParticleArea(CS_PARTICLES + i);

		if (!rval) {
			break;
		}
	}
*/
}

/*
=======================================================================================================================================
CG_BuildSpectatorString
=======================================================================================================================================
*/
void CG_BuildSpectatorString(void) {
	int i;

	cg.spectatorList[0] = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
}

/*
=======================================================================================================================================
CG_RegisterClients
=======================================================================================================================================
*/
static void CG_RegisterClients(void) {
	int i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i = 0; i < MAX_CLIENTS; i++) {
		const char *clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString(CS_PLAYERS + i);

		if (!clientInfo[0]) {
			continue;
		}

		CG_LoadingClient(i);
		CG_NewClientInfo(i);
	}

	CG_BuildSpectatorString();
}

/*
=======================================================================================================================================
CG_ConfigString
=======================================================================================================================================
*/
const char *CG_ConfigString(int index) {

	if (index < 0 || index >= MAX_CONFIGSTRINGS) {
		CG_Error("CG_ConfigString: bad index: %i", index);
	}

	return cgs.gameState.stringData + cgs.gameState.stringOffsets[index];
}

/*
=======================================================================================================================================
CG_StartMusic
=======================================================================================================================================
*/
void CG_StartMusic(void) {
	char *s;
	char parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString(CS_MUSIC);
	Q_strncpyz(parm1, COM_Parse(&s), sizeof(parm1));
	Q_strncpyz(parm2, COM_Parse(&s), sizeof(parm2));

	trap_S_StartBackgroundTrack(parm1, parm2);
}
#ifdef MISSIONPACK
/*
=======================================================================================================================================
CG_PlayCinematic
=======================================================================================================================================
*/
static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
	return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

/*
=======================================================================================================================================
CG_StopCinematic
=======================================================================================================================================
*/
static void CG_StopCinematic(int handle) {
	trap_CIN_StopCinematic(handle);
}

/*
=======================================================================================================================================
CG_DrawCinematic
=======================================================================================================================================
*/
static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
	trap_CIN_SetExtents(handle, x, y, w, h);
	trap_CIN_DrawCinematic(handle);
}

/*
=======================================================================================================================================
CG_RunCinematicFrame
=======================================================================================================================================
*/
static void CG_RunCinematicFrame(int handle) {
	trap_CIN_RunCinematic(handle);
}
#endif
/*
=======================================================================================================================================
CG_Init

Called after every level change or subsystem restart. Will perform callbacks to make the loading info screen update.
=======================================================================================================================================
*/
void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum) {
	const char *s;

	// clear everything
	memset(&cgs, 0, sizeof(cgs));
	memset(&cg, 0, sizeof(cg));
	memset(cg_entities, 0, sizeof(cg_entities));
	memset(cg_weapons, 0, sizeof(cg_weapons));
	memset(cg_items, 0, sizeof(cg_items));

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;
	// load a few needed things before we do any screen updates
	cgs.media.charsetShader = trap_R_RegisterShader("gfx/2d/bigchars");
	cgs.media.whiteShader = trap_R_RegisterShader("white");
	cgs.media.charsetProp = trap_R_RegisterShaderNoMip("menu/art/font1_prop.tga");
	cgs.media.charsetPropGlow = trap_R_RegisterShaderNoMip("menu/art/font1_prop_glo.tga");
	cgs.media.charsetPropB = trap_R_RegisterShaderNoMip("menu/art/font2_prop.tga");
	// register fonts here, otherwise CG_LoadingString wont work
	trap_R_RegisterFont("fonts/tinyFont.ttf", TINYCHAR_HEIGHT, 0, qtrue, &cgs.media.tinyFont);
	trap_R_RegisterFont("fonts/smallFont.ttf", SMALLCHAR_HEIGHT, 0, qtrue, &cgs.media.smallFont);
	trap_R_RegisterFont("fonts/defaultFont.ttf", DEFAULTCHAR_HEIGHT, 0, qtrue, &cgs.media.defaultFont);
	trap_R_RegisterFont("fonts/bigFont.ttf", BIGCHAR_HEIGHT, 0, qtrue, &cgs.media.bigFont);
	trap_R_RegisterFont("fonts/bigFont.ttf", GIANTCHAR_HEIGHT, 0, qtrue, &cgs.media.giantFont);
	trap_R_RegisterFont("fonts/bigFont.ttf", TITANCHAR_HEIGHT, 0, qtrue, &cgs.media.titanFont);

	CG_RegisterCvars();
	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig(&cgs.glconfig);

	cgs.screenXScaleStretch = cgs.glconfig.vidWidth * (1.0 / 640.0);
	cgs.screenYScaleStretch = cgs.glconfig.vidHeight * (1.0 / 480.0);

	if (cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640) {
		cgs.screenXScale = cgs.glconfig.vidWidth * (1.0 / 640.0);
		cgs.screenYScale = cgs.glconfig.vidHeight * (1.0 / 480.0);
		// wide screen
		cgs.screenXBias = 0.5 * (cgs.glconfig.vidWidth - (cgs.glconfig.vidHeight * (640.0 / 480.0)));
		cgs.screenXScale = cgs.screenYScale;
		// no narrow screen
		cgs.screenYBias = 0;
	} else {
		cgs.screenXScale = cgs.glconfig.vidWidth * (1.0 / 640.0);
		cgs.screenYScale = cgs.glconfig.vidHeight * (1.0 / 480.0);
		// narrow screen
		cgs.screenYBias = 0.5 * (cgs.glconfig.vidHeight - (cgs.glconfig.vidWidth * (480.0 / 640.0)));
		cgs.screenYScale = cgs.screenXScale;
		// no wide screen
		cgs.screenXBias = 0;
	}
	// get the gamestate from the client system
	trap_GetGameState(&cgs.gameState);
	// check version
	s = CG_ConfigString(CS_GAME_VERSION);

	if (strcmp(s, GAME_VERSION)) {
		CG_Error("Client/Server game mismatch: %s/%s", GAME_VERSION, s);
	}

	s = CG_ConfigString(CS_LEVEL_START_TIME);
	cgs.levelStartTime = atoi(s);

	trap_SetMapTitle(CG_ConfigString(CS_MESSAGE));

	CG_ParseServerinfo();
	// load the new map
	CG_LoadingString("collision map");

	trap_CM_LoadMap(cgs.mapname);

	cg.loading = qtrue; // force players to load instead of defer

	CG_LoadingString("sounds");
	CG_RegisterSounds();
	CG_LoadingString("graphics");
	CG_RegisterGraphics();
	CG_LoadingString("clients");
	CG_RegisterClients(); // if low on memory, some clients will be deferred

	cg.loading = qfalse; // future players will be deferred

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	// remove the last loading update
	cg.infoScreenText[0] = 0;
	// Make sure we have update values (scores)
	CG_SetConfigValues();
	CG_StartMusic();
	CG_LoadingString("");
#ifdef MISSIONPACK
	CG_InitTeamChat();
#endif
	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds(qtrue);
}

/*
=======================================================================================================================================
CG_Shutdown

Called before every level change or subsystem restart.
=======================================================================================================================================
*/
void CG_Shutdown(void) {
	// some mods may need to do cleanup work here, like closing files or archiving session data
}

/*
=======================================================================================================================================
CG_EventHandling

 type 0 - no event handling
      1 - team menu
      2 - hud editor
=======================================================================================================================================
*/
void CG_EventHandling(int type) {

}

/*
=======================================================================================================================================
CG_KeyEvent
=======================================================================================================================================
*/
void CG_KeyEvent(int key, qboolean down) {

}

/*
=======================================================================================================================================
CG_MouseEvent
=======================================================================================================================================
*/
void CG_MouseEvent(int x, int y) {

}

/*
=======================================================================================================================================
CG_VoIPString
=======================================================================================================================================
*/
static char *CG_VoIPString(void) {
	// a generous overestimate of the space needed for 0, 1, 2...61, 62, 63
	static char voipString[MAX_CLIENTS * 4];
	char voipSendTarget[MAX_CVAR_VALUE_STRING];

	trap_Cvar_VariableStringBuffer("cl_voipSendTarget", voipSendTarget, sizeof(voipSendTarget));

	if (Q_stricmp(voipSendTarget, "team") == 0) {
		int i, slen, nlen;

		for (slen = i = 0; i < cgs.maxclients; i++) {
			if (!cgs.clientinfo[i].infoValid || i == cg.clientNum) {
				continue;
			}

			if (cgs.clientinfo[i].team != cgs.clientinfo[cg.clientNum].team) {
				continue;
			}

			nlen = Com_sprintf(&voipString[slen], sizeof(voipString) - slen, "%s%d", (slen > 0) ? "," : "", i);

			if (slen + nlen + 1 >= sizeof(voipString)) {
				CG_Printf(S_COLOR_YELLOW "WARNING: voipString overflowed\n");
				break;
			}

			slen += nlen;
		}
		// notice that if the Com_sprintf was truncated, slen was not updated, so this will remove any trailing commas or partially-completed numbers
		voipString[slen] = '\0';
	} else if (Q_stricmp(voipSendTarget, "crosshair") == 0) {
		Com_sprintf(voipString, sizeof(voipString), "%d", CG_CrosshairPlayer());
	} else if (Q_stricmp(voipSendTarget, "attacker") == 0) {
		Com_sprintf(voipString, sizeof(voipString), "%d", CG_LastAttacker());
	} else {
		return NULL;
	}

	return voipString;
}
