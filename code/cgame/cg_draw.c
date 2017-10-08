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
 Draw all of the graphical elements during active (after loading) gameplay.
**************************************************************************************************************************************/

#include "cg_local.h"
#ifdef MISSIONPACK
#include "../ui/ui_shared.h"
// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
#else
int drawTeamOverlayModificationCount = -1;
#endif
int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];
#ifndef MISSIONPACK
/*
=======================================================================================================================================
CG_DrawField

Draws large numbers for status bar and powerups.
=======================================================================================================================================
*/
static void CG_DrawField(int x, int y, int width, int value, float *color) {
	char num[16], *ptr;
	int l;
	int frame;

	if (width < 1) {
		return;
	}
	// draw number string
	if (width > 5) {
		width = 5;
	}

	switch (width) {
		case 1:
			value = value > 9 ? 9 : value;
			value = value < 0 ? 0 : value;
			break;
		case 2:
			value = value > 99 ? 99 : value;
			value = value < -9 ? -9 : value;
			break;
		case 3:
			value = value > 999 ? 999 : value;
			value = value < -99 ? -99 : value;
			break;
		case 4:
			value = value > 9999 ? 9999 : value;
			value = value < -999 ? -999 : value;
			break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);

	l = strlen(num);

	if (l > width) {
		l = width;
	}

	x += 2 + CHAR_WIDTH * (width - l);
	// center x, move y to bottom
	x += (1.0f - cg_statusScale.value) * CHAR_WIDTH * 0.5f;
	y += (1.0f - cg_statusScale.value) * CHAR_HEIGHT;

	trap_R_SetColor(color);

	ptr = num;

	while (*ptr && l) {
		if (*ptr == '-') {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		CG_DrawPic(x, y, CHAR_WIDTH * cg_statusScale.value, CHAR_HEIGHT * cg_statusScale.value, cgs.media.numberShaders[frame]);
		x += CHAR_WIDTH * cg_statusScale.value;
		ptr++;
		l--;
	}

	trap_R_SetColor(NULL);
}
#endif
/*
=======================================================================================================================================
CG_Draw3DModel
=======================================================================================================================================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles) {
	refdef_t refdef;
	refEntity_t ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer) {
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));
	memset(&ent, 0, sizeof(ent));

	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);

	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW; // no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;
	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);
}

/*
=======================================================================================================================================
CG_DrawHealthModel
=======================================================================================================================================
*/
void CG_DrawHealthModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, qhandle_t model2, vec3_t origin, vec3_t angles, float yaw2) {
	refdef_t refdef;
	refEntity_t ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer) {
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));
	memset(&ent, 0, sizeof(ent));

	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);

	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW; // no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;
	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);

	if (model2) {
		ent.hModel = model2;
		angles[YAW] = yaw2;
		AnglesToAxis(angles, ent.axis);
		trap_R_AddRefEntityToScene(&ent);
	}

	trap_R_RenderScene(&refdef);
}

/*
=======================================================================================================================================
CG_DrawHead

Used for both the status bar and the scoreboard.
=======================================================================================================================================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles) {
	clipHandle_t cm;
	clientInfo_t *ci;
	float len;
	vec3_t origin;
	vec3_t mins, maxs;

	ci = &cgs.clientinfo[clientNum];

	if (cg_draw3dIcons.integer) {
		cm = ci->headModel;

		if (!cm) {
			return;
		}
		// offset the origin y and z to center the head
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);
		// calculate distance so the head nearly fills the box, assume heads are taller than wide
		len = 0.7 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268; // len / tan(fov / 2)
		// allow per-model tweaking
		VectorAdd(origin, ci->headOffset, origin);

		CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);
	} else if (cg_drawIcons.integer) {
		CG_DrawPic(x, y, w, h, ci->modelIcon);
	}
	// if they are deferred, draw a cross out
	if (ci->deferred) {
		CG_DrawPic(x, y, w, h, cgs.media.deferShader);
	}
}

/*
=======================================================================================================================================
CG_DrawFlagModel

Used for both the status bar and the scoreboard.
=======================================================================================================================================
*/
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D) {
	qhandle_t cm;
	float len;
	vec3_t origin, angles;
	vec3_t mins, maxs;
	qhandle_t handle;

	if (!force2D && cg_draw3dIcons.integer) {
		VectorClear(angles);

		cm = cgs.media.redFlagModel;
		// offset the origin y and z to center the flag
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);
		// calculate distance so the flag nearly fills the box, assume heads are taller than wide
		len = 0.5 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268; // len / tan(fov / 2)
		angles[YAW] = 60 * sin(cg.time / 2000.0);

		if (team == TEAM_RED) {
			handle = cgs.media.redFlagModel;
		} else if (team == TEAM_BLUE) {
			handle = cgs.media.blueFlagModel;
		} else if (team == TEAM_FREE) {
			handle = cgs.media.neutralFlagModel;
		} else {
			return;
		}

		CG_Draw3DModel(x, y, w, h, handle, 0, origin, angles);
	} else if (cg_drawIcons.integer) {
		gitem_t *item;

		if (team == TEAM_RED) {
			item = BG_FindItemForPowerup(PW_REDFLAG);
		} else if (team == TEAM_BLUE) {
			item = BG_FindItemForPowerup(PW_BLUEFLAG);
		} else if (team == TEAM_FREE) {
			item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
		} else {
			return;
		}

		if (item) {
			CG_DrawPic(x, y, w, h, cg_items[ITEM_INDEX(item)].icon);
		}
	}
}
#ifndef MISSIONPACK
/*
=======================================================================================================================================
CG_DrawStatusBarHead
=======================================================================================================================================
*/
static void CG_DrawStatusBarHead(float x) {
	vec3_t angles;
	float size, stretch;
	float frac;

	VectorClear(angles);

	if (cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME) {
		frac = (float)(cg.time - cg.damageTime) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * (1.5 - frac * 0.5);
		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;
		cg.headEndYaw = 180 + 20 * cos(crandom() * M_PI);
		cg.headEndPitch = 5 * cos(crandom() * M_PI);
		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if (cg.time >= cg.headEndTime) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;
			cg.headEndYaw = 180 + 20 * cos(crandom() * M_PI);
			cg.headEndPitch = 5 * cos(crandom() * M_PI);
		}

		size = ICON_SIZE * 1.25;
	}
	// if the server was frozen for a while we may have a bad head start time
	if (cg.headStartTime > cg.time) {
		cg.headStartTime = cg.time;
	}

	frac = (cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
	frac = frac * frac * (3 - 2 * frac);
	angles[YAW] = cg.headStartYaw + (cg.headEndYaw - cg.headStartYaw) * frac;
	angles[PITCH] = cg.headStartPitch + (cg.headEndPitch - cg.headStartPitch) * frac;

	CG_DrawHead(x + (1.0f - cg_statusScale.value) * size * 0.5f, 480 - size * cg_statusScale.value, size * cg_statusScale.value, size * cg_statusScale.value, cg.snap->ps.clientNum, angles);
}

/*
=======================================================================================================================================
CG_DrawStatusBarFlag
=======================================================================================================================================
*/
static void CG_DrawStatusBarFlag(float x, int team) {
	int iconSize = ICON_SIZE * cg_statusScale.value;

	CG_DrawFlagModel(x + (1.0f - cg_statusScale.value) * ICON_SIZE * 0.5f, 480 - iconSize, iconSize, iconSize, team, qfalse);
}
#endif
/*
=======================================================================================================================================
CG_DrawTeamBackground
=======================================================================================================================================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team) {
	vec4_t hcolor;

	hcolor[3] = alpha;

	if (team == TEAM_RED) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if (team == TEAM_BLUE) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}

	trap_R_SetColor(hcolor);
	CG_SetScreenPlacement(PLACE_STRETCH, CG_GetScreenVerticalPlacement());
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	CG_PopScreenPlacement();
	trap_R_SetColor(NULL);
}
#ifndef MISSIONPACK
/*
=======================================================================================================================================
CG_DrawStatusBar
=======================================================================================================================================
*/
static void CG_DrawStatusBar(void) {
	int color;
	char *s;
	centity_t *cent;
	playerState_t *ps;
	int value;
	vec3_t angles;
	vec3_t origin;
	float scale, iconSize;

	static float colors[4][4] = {
//		{0.2, 1.0, 0.2, 1.0}, {1.0, 0.2, 0.2, 1.0}, {0.5, 0.5, 0.5, 1}};
		{1.0f, 0.69f, 0.0f, 1.0f}, // normal
		{1.0f, 0.2f, 0.2f, 1.0f}, // low health
		{0.5f, 0.5f, 0.5f, 1.0f}, // weapon firing
		{1.0f, 1.0f, 1.0f, 1.0f}}; // health > 100

	if (cg_drawStatus.integer == 0) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);

	scale = Com_Clamp(0.1f, 2, cg_statusScale.value);
	iconSize = scale * ICON_SIZE;
	// draw the team background
	CG_DrawTeamBackground(0, 480 - 60 * scale, 640, 60 * scale, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear(angles);
	// draw any 3D icons first, so the changes back to 2D are minimized

	// ammo
	if (cent->currentState.weapon && cg_weapons[cent->currentState.weapon].ammoModel) {
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin(cg.time / 1000.0);

		CG_Draw3DModel(480 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, cg_weapons[cent->currentState.weapon].ammoModel, 0, origin, angles);
	}
	// health
	if (cg_drawStatusHead.integer == 2) {
		origin[0] = 60;
		origin[1] = 0;
		origin[2] = -5;
		angles[YAW] = (cg.time & 2047) * 360 / 4096.0;

		CG_DrawHealthModel(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, cg_items[3].models[0], 0, cg_items[3].models[1], origin, angles, 0);
		// if we didn't draw a 3D icon, draw a 2D icon for health
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
			CG_DrawPic(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, cg_items[3].icon);
		}
	} else if (cg_drawStatusHead.integer == 1) {
		CG_DrawStatusBarHead(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE);
	}
	// armor
	if (ps->stats[STAT_ARMOR]) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = (cg.time & 2047) * 360 / 2048.0;

		CG_Draw3DModel(CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, cgs.media.armorModel, 0, origin, angles);
	}
	// flags
	if (cg.predictedPlayerState.powerups[PW_REDFLAG]) {
		CG_DrawStatusBarFlag(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED);
	} else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG]) {
		CG_DrawStatusBarFlag(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE);
	} else if (cg.predictedPlayerState.powerups[PW_NEUTRALFLAG]) {
		CG_DrawStatusBarFlag(240 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE);
	}
	// skulls
	if (cgs.gametype == GT_HARVESTER) {
		value = ps->tokens;
		s = va("%i", value);

		if (value > 0) {
			// if we didn't draw a 3D icon, draw a 2D icon for the skull
			origin[0] = 80;
			origin[1] = 0;
			origin[2] = +10;
			angles[YAW] = (cg.time & 2047) * 360 / 2048.0;

			if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
				CG_Draw3DModel(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, 480 - iconSize, iconSize, iconSize, cgs.media.redCubeModel, 0, origin, angles);
			} else {
				CG_Draw3DModel(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, 480 - iconSize, iconSize, iconSize, cgs.media.blueCubeModel, 0, origin, angles);
			}
			// if we didn't draw a 3D icon, draw a 2D icon for health
			if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
				qhandle_t icon;

				if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
					icon = cgs.media.redCubeIcon;
				} else {
					icon = cgs.media.blueCubeIcon;
				}

				CG_DrawPic(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE + 12, 480 - iconSize, iconSize * 0.5, iconSize * 0.5, icon);
			}

			trap_R_SetColor(colors[0]);
			CG_DrawBigString(200 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, 462, s, 1.0F);
			trap_R_SetColor(NULL);
		}
	}
	// ammo
	if (cent->currentState.weapon) {
		value = ps->ammo[cent->currentState.weapon];

		if (value > -1) {
			if (cg.predictedPlayerState.weaponstate == WEAPON_FIRING && cg.predictedPlayerState.weaponTime > 100) {
				// draw as dark grey when reloading
				color = 2; // dark grey
			} else {
				if (value >= 0) {
					color = 0; // green
				} else {
					color = 1; // red
				}
			}

			CG_DrawField(480, 432, 3, value, colors[color]);
			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
				qhandle_t icon;

				icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;

				if (icon) {
					CG_DrawPic(480 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, icon);
				}
			}
		}
	}
	// health
	value = ps->stats[STAT_HEALTH];

	if (value > 100) {
		color = 3; // white
	} else if (value > 25) {
		color = 0; // green
	} else if (value > 0) {
		color = (cg.time >> 8) & 1; // flash
	} else {
		color = 1; // red
	}
	// stretch the health up when taking damage
	CG_DrawField(260, 432, 3, value, colors[color]);
	// armor
	value = ps->stats[STAT_ARMOR];

	if (value > 0) {
		CG_DrawField(0, 432, 3, value, colors[color]);
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer) {
			CG_DrawPic(CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 480 - iconSize, iconSize, iconSize, cgs.media.armorIcon);
		}
	}
}
#endif
/*
=======================================================================================================================================

	UPPER RIGHT CORNER

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_DrawAttacker
=======================================================================================================================================
*/
static float CG_DrawAttacker(float y) {
	int t;
	float size;
	vec3_t angles;
	vec4_t color;
	const char *info;
	const char *name;
	int clientNum;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) {
		return y;
	}

	if (!cg.attackerTime) {
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];

	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum) {
		return y;
	}

	if (!cgs.clientinfo[clientNum].infoValid) {
		cg.attackerTime = 0;
		return y;
	}

	t = cg.time - cg.attackerTime;

	if (t > ATTACKER_HEAD_TIME) {
		cg.attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;

	CG_DrawHead(640 - size, y, size, size, clientNum, angles);

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	name = Info_ValueForKey(info, "n");

	y += size;

	color[0] = color[1] = color[2] = 1;
	color[3] = 0.5f;

	CG_DrawString(635, y + 2, name, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, color);

	return y + 2 + CG_DrawStringLineHeight(UI_BIGFONT);
}

/*
=======================================================================================================================================
CG_DrawSnapshot
=======================================================================================================================================
*/
static float CG_DrawSnapshot(float y) {
	char *s;

	s = va("Time:%i Snap:%i Cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);

	CG_DrawString(635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_SMALLFONT, NULL);

	return y + 2 + CG_DrawStringLineHeight(UI_SMALLFONT);
}

#define FPS_FRAMES 4
/*
=======================================================================================================================================
CG_DrawFPS
=======================================================================================================================================
*/
static float CG_DrawFPS(float y) {
	char *s;
	static int previousTimes[FPS_FRAMES];
	static int index;
	int i, total;
	int fps;
	static int previous;
	int t, frameTime;

	// don't use serverTime, because that will be drifting to correct for internet lag changes, timescales, timedemos, etc.
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;
	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;

	if (index > FPS_FRAMES) {
		// average multiple frames together to smooth changes out a bit
		total = 0;

		for (i = 0; i < FPS_FRAMES; i++) {
			total += previousTimes[i];
		}

		if (!total) {
			total = 1;
		}

		fps = 1000 * FPS_FRAMES / (float)total;
		s = va("%iFps", fps);

		CG_DrawString(635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_SMALLFONT, NULL);
	}

	return y + 2 + CG_DrawStringLineHeight(UI_SMALLFONT);
}

/*
=======================================================================================================================================
CG_DrawTimer
=======================================================================================================================================
*/
static float CG_DrawTimer(float y) {
	char *s;
	int hours, mins, seconds;
	int msec;

	msec = cg.time - cgs.levelStartTime;
	seconds = (msec / 1000) % 60;
	mins = (msec / (1000 * 60)) % 60;
	hours = (msec / (1000 * 60 * 60));

	if (hours > 0) {
		s = va("%i:%s%i:%s%i", hours, mins < 10 ? "0" : "", mins, seconds < 10 ? "0" : "", seconds);
	} else {
		s = va("%i:%s%i", mins, seconds < 10 ? "0" : "", seconds);
	}

	CG_DrawString(635, y + 2, s, UI_RIGHT|UI_DROPSHADOW|UI_BIGFONT, NULL);

	return y + 2 + CG_DrawStringLineHeight(UI_BIGFONT);
}

/*
=======================================================================================================================================
CG_DrawTeamOverlay
=======================================================================================================================================
*/
static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper) {
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t *item;
	int ret_y, count;
	int team;
	int lineHeight;
	int iconWidth, iconHeight;
	int healthWidth, armorWidth;

	if (!cg_drawTeamOverlay.integer) {
		return y;
	}

	team = cg.snap->ps.persistant[PERS_TEAM];

	if (team != TEAM_RED && team != TEAM_BLUE) {
		return y; // not on any team
	}

	lineHeight = CG_DrawStringLineHeight(UI_TINYFONT);
	healthWidth = CG_DrawStrlen("000", UI_TINYFONT);
	iconWidth = iconHeight = lineHeight;
	armorWidth = healthWidth;

	plyrs = 0;
	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];

		if (ci->infoValid && ci->team == team) {
			plyrs++;
			len = CG_DrawStrlen(ci->name, UI_TINYFONT);

			if (len > pwidth) {
				pwidth = len;
			}
		}
	}

	if (!plyrs) {
		return y;
	}

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH * TINYCHAR_WIDTH) {
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH * TINYCHAR_WIDTH;
	}
	// max location name width
	lwidth = 0;

	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);

		if (p && *p) {
			len = CG_DrawStrlen(p, UI_TINYFONT);

			if (len > lwidth) {
				lwidth = len;
			}
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH * TINYCHAR_WIDTH) {
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH * TINYCHAR_WIDTH;
	}

	w = pwidth + lwidth + healthWidth + armorWidth + iconWidth * 5;

	if (right) {
		x = 640 - w;
	} else {
		x = 0;
	}

	h = plyrs * lineHeight;

	if (upper) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if (team == TEAM_RED) {
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	} else {
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}

	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];

		if (ci->infoValid && ci->team == team) {
			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;
			xx = x + iconWidth;

			CG_DrawStringExt(xx, y, ci->name, UI_DROPSHADOW|UI_TINYFONT, NULL, 0, TEAM_OVERLAY_MAXNAME_WIDTH, 0);

			xx += pwidth;

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);

				if (!p || !*p) {
					p = "Unknown";
				}

				xx += iconWidth; // not icon related
				CG_DrawStringExt(xx, y, p, UI_DROPSHADOW|UI_TINYFONT, NULL, 0, TEAM_OVERLAY_MAXLOCATION_WIDTH, 0);
				xx += lwidth;
			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor);
			// draw health
			xx += iconWidth; // not icon related

			Com_sprintf(st, sizeof(st), "%3i", ci->health);
			CG_DrawString(xx, y, st, UI_DROPSHADOW|UI_TINYFONT, hcolor);
			// draw weapon icon
			xx += healthWidth;

			if (cg_weapons[ci->curWeapon].weaponIcon) {
				CG_DrawPic(xx, y, iconWidth, iconHeight, cg_weapons[ci->curWeapon].weaponIcon);
			} else {
				CG_DrawPic(xx, y, iconWidth, iconHeight, cgs.media.deferShader);
			}
			// draw armor
			xx += iconWidth;

			Com_sprintf(st, sizeof(st), "%3i", ci->armor);
			CG_DrawString(xx, y, st, UI_DROPSHADOW|UI_TINYFONT, hcolor);
			// draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - iconWidth;
			}

			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
				if (ci->powerups & (1 << j)) {
					item = BG_FindItemForPowerup(j);

					if (item) {
						CG_DrawPic(xx, y, iconWidth, iconHeight, trap_R_RegisterShader(item->icon));

						if (right) {
							xx -= iconWidth;
						} else {
							xx += iconWidth;
						}
					}
				}
			}

			y += lineHeight;
		}
	}

	return ret_y;
}

/*
=======================================================================================================================================
CG_DrawUpperRight
=======================================================================================================================================
*/
static void CG_DrawUpperRight(stereoFrame_t stereoFrame) {
	float y;

	y = 0;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_TOP);

	if (cgs.gametype > GT_TOURNAMENT && cg_drawTeamOverlay.integer == 1) {
		y = CG_DrawTeamOverlay(y, qtrue, qtrue);
	}

	if (cg_drawSnapshot.integer) {
		y = CG_DrawSnapshot(y);
	}

	if (cg_drawFPS.integer && (stereoFrame == STEREO_CENTER || stereoFrame == STEREO_RIGHT)) {
		y = CG_DrawFPS(y);
	}

	if (cg_drawTimer.integer) {
		y = CG_DrawTimer(y);
	}

	if (cg_drawAttacker.integer) {
		CG_DrawAttacker(y);
	}
}

/*
=======================================================================================================================================

	LOWER RIGHT CORNER

=======================================================================================================================================
*/
#ifndef MISSIONPACK
/*
=======================================================================================================================================
CG_DrawScores

Draw the small two score display.
=======================================================================================================================================
*/
static float CG_DrawScores(float y) {
	const char *s;
	int s1, s2, score;
	int x, w;
	int v;
	vec4_t color;
	float y1;
	gitem_t *item;
	int scoreHeight;

	if (!cg_drawScores.integer) {
		return y;
	}

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	scoreHeight = CG_DrawStringLineHeight(UI_BIGFONT) + 6;
	y -= scoreHeight - 20;
	y1 = y;
	// draw from the right side to left
	if (cgs.gametype > GT_TOURNAMENT) {
		x = 640;

		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		color[3] = 0.33f;

		s = va("%2i", s2);
		w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
		x -= w;

		CG_FillRect(x, y - 4, w, scoreHeight, color);

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
			CG_DrawPic(x, y - 4, w, scoreHeight, cgs.media.selectShader);
		}

		CG_DrawBigString(x + 4, y, s, 1.0f);

		if (cgs.gametype == GT_CTF) {
			// display flag status
			item = BG_FindItemForPowerup(PW_BLUEFLAG);

			if (item) {
				y1 = y - scoreHeight;

				if (cgs.blueflag >= 0 && cgs.blueflag <= 2) {
					CG_DrawPic(x, y1 - 4, w, scoreHeight, cgs.media.blueFlagShader[cgs.blueflag]);
				}
			}
		}

		if (cgs.gametype == GT_1FCTF) {
			// display flag status
			item = BG_FindItemForPowerup(PW_NEUTRALFLAG);

			if (item) {
				y1 = y - scoreHeight;

				if (cgs.flagStatus >= 0 && cgs.flagStatus <= 4) {
					vec4_t color = {1, 1, 1, 1};
					int index = 0;

					if (cgs.flagStatus == FLAG_TAKEN_RED) {
						color[1] = color[2] = 0;
						index = 1;
					} else if (cgs.flagStatus == FLAG_TAKEN_BLUE) {
						color[0] = color[1] = 0;
						index = 1;
					} else if (cgs.flagStatus == FLAG_DROPPED) {
						index = 2;
					}

					trap_R_SetColor(color);
					CG_DrawPic(x, y1 - 4, w, scoreHeight, cgs.media.flagShaders[index]);
					trap_R_SetColor(NULL);
				}
			}
		}

		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.33f;

		s = va("%2i", s1);
		w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
		x -= w;

		CG_FillRect(x, y - 4, w, scoreHeight, color);

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
			CG_DrawPic(x, y - 4, w, scoreHeight, cgs.media.selectShader);
		}

		CG_DrawBigString(x + 4, y, s, 1.0f);

		if (cgs.gametype == GT_CTF) {
			// Display flag status
			item = BG_FindItemForPowerup(PW_REDFLAG);

			if (item) {
				y1 = y - scoreHeight;

				if (cgs.redflag >= 0 && cgs.redflag <= 2) {
					CG_DrawPic(x, y1 - 4, w, scoreHeight, cgs.media.redFlagShader[cgs.redflag]);
				}
			}
		}

		if (cgs.gametype > GT_TEAM) {
			v = cgs.capturelimit;
		} else {
			v = cgs.fraglimit;
		}

		if (v) {
			s = va("%2i", v);
			w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
			x -= w;

			CG_DrawBigString(x + 4, y, s, 1.0f);
		} else {
			qboolean spectator;

			score = cg.snap->ps.persistant[PERS_SCORE];
			spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);

			s = va("%2i", score);
			w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
			x -= w;

			if (!spectator) {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4, w, BIGCHAR_HEIGHT + 8, color);
			}

			CG_DrawBigString(x + 4, y, s, 1.0f);
		}
	} else {
		qboolean spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);
		// always show your score in the second box if not in first place
		if (s1 != score) {
			s2 = score;
		}

		if (s2 != SCORE_NOT_PRESENT) {
			s = va("%2i", s2);
			w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
			x -= w;

			if (!spectator && score == s2 && score != s1) {
				color[0] = 1.0f;
				color[1] = 0.0f;
				color[2] = 0.0f;
				color[3] = 0.33f;

				CG_FillRect(x, y - 4, w, scoreHeight, color);
				CG_DrawPic(x, y - 4, w, scoreHeight, cgs.media.selectShader);
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;

				CG_FillRect(x, y - 4, w, scoreHeight, color);
			}

			CG_DrawBigString(x + 4, y, s, 1.0f);
		}
		// first place
		if (s1 != SCORE_NOT_PRESENT) {
			s = va("%2i", s1);
			w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
			x -= w;

			if (!spectator && score == s1) {
				color[0] = 0.0f;
				color[1] = 0.0f;
				color[2] = 1.0f;
				color[3] = 0.33f;

				CG_FillRect(x, y - 4, w, scoreHeight, color);
				CG_DrawPic(x, y - 4, w, scoreHeight, cgs.media.selectShader);
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;

				CG_FillRect(x, y - 4, w, scoreHeight, color);
			}

			CG_DrawBigString(x + 4, y, s, 1.0f);
		}

		if (cgs.fraglimit) {
			s = va("%2i", cgs.fraglimit);
			w = CG_DrawStrlen(s, UI_BIGFONT) + 8;
			x -= w;
			CG_DrawBigString(x + 4, y, s, 1.0f);
		}
	}

	return y1 - 8;
}

/*
=======================================================================================================================================
CG_DrawPowerups
=======================================================================================================================================
*/
static float CG_DrawPowerups(float y) {
	int sorted[MAX_POWERUPS];
	int sortedTime[MAX_POWERUPS];
	int i, j, k;
	int active;
	playerState_t *ps;
	int t;
	gitem_t *item;
	int x;
	int color;
	float size;
	float f;
	static float colors[2][4] = {{0.2f, 1.0f, 0.2f, 1.0f}, {1.0f, 0.2f, 0.2f, 1.0f}};

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0) {
		return y;
	}
	// sort the list by time remaining
	active = 0;

	for (i = 0; i < MAX_POWERUPS; i++) {
		if (!ps->powerups[i]) {
			continue;
		}

		if (i == PW_AMMOREGEN || i == PW_GUARD || i == PW_DOUBLER || i == PW_SCOUT) {
			continue;
		}
		// don't draw if the power up has unlimited time, this is true of the CTF flags
		if (ps->powerups[i] == INT_MAX) {
			continue;
		}

		t = ps->powerups[i] - cg.time;

		if (t <= 0) {
			continue;
		}
		// insert into the list
		for (j = 0; j < active; j++) {
			if (sortedTime[j] >= t) {
				for (k = active - 1; k >= j; k--) {
					sorted[k + 1] = sorted[k];
					sortedTime[k + 1] = sortedTime[k];
				}

				break;
			}
		}

		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}
	// draw the icons and timers
	x = 640 - ICON_SIZE - CHAR_WIDTH * 2;

	for (i = 0; i < active; i++) {
		item = BG_FindItemForPowerup(sorted[i]);

		if (item) {
			color = 1;
			y -= ICON_SIZE;

			CG_DrawField(x, y, 2, sortedTime[i] / 1000, colors[color]);

			t = ps->powerups[sorted[i]];

			if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME) {

			} else {
				vec4_t modulate;

				f = (float)(t - cg.time) / POWERUP_BLINK_TIME;
				f -= (int)f;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;

				trap_R_SetColor(modulate);
			}

			if (cg.powerupActive == sorted[i] && cg.time - cg.powerupTime < PULSE_TIME) {
				f = 1.0 - (((float)cg.time - cg.powerupTime) / PULSE_TIME);
				size = ICON_SIZE * (1.0 + (PULSE_SCALE - 1.0) * f);
			} else {
				size = ICON_SIZE;
			}

			CG_DrawPic(640 - size, y + ICON_SIZE / 2 - size / 2, size, size, trap_R_RegisterShader(item->icon));
			trap_R_SetColor(NULL);
		}
	}

	return y;
}

/*
=======================================================================================================================================
CG_DrawLocalInfo
=======================================================================================================================================
*/
static float CG_DrawLocalInfo(float y) {
	char *s;
	qtime_t qtime;

	trap_RealTime(&qtime);

	s = va("%02d:%02d", qtime.tm_hour, qtime.tm_min);
	CG_DrawString(640, y + 32, s, UI_RIGHT|UI_DROPSHADOW|UI_TINYFONT, NULL);
	return y;
}

/*
=======================================================================================================================================
CG_DrawLowerRight
=======================================================================================================================================
*/
static void CG_DrawLowerRight(void) {
	float y;

	y = 480 - ICON_SIZE;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);

	if (cgs.gametype > GT_TOURNAMENT && cg_drawTeamOverlay.integer == 2) {
		y = CG_DrawTeamOverlay(y, qtrue, qfalse);
	}

	y = CG_DrawLocalInfo(y);
	y = CG_DrawScores(y);

	CG_DrawPowerups(y);
}

/*
=======================================================================================================================================
CG_DrawPickupItem
=======================================================================================================================================
*/
static int CG_DrawPickupItem(int y) {
	int value;
	float *fadeColor;

	if (cg.snap->ps.stats[STAT_HEALTH] <= 0) {
		return y;
	}

	y -= ICON_SIZE;
	value = cg.itemPickup;

	if (value) {
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);

		if (fadeColor) {
			CG_RegisterItemVisuals(value);
			trap_R_SetColor(fadeColor);
			CG_DrawPic(8, y, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
			trap_R_SetColor(NULL);
			CG_DrawString(ICON_SIZE + 16, y + (ICON_SIZE / 2), bg_itemlist[value].pickup_name, UI_VA_CENTER|UI_DROPSHADOW|UI_BIGFONT, fadeColor);
		}
	}

	return y;
}

/*
=======================================================================================================================================
CG_DrawLowerLeft
=======================================================================================================================================
*/
static void CG_DrawLowerLeft(void) {
	float y;

	y = 480 - (ICON_SIZE - 14);

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_BOTTOM);

	if (cgs.gametype > GT_TOURNAMENT && cg_drawTeamOverlay.integer == 3) {
		y = CG_DrawTeamOverlay(y, qfalse, qfalse);
	}

	CG_DrawPickupItem(y);
}

/*
=======================================================================================================================================
CG_DrawTeamInfo
=======================================================================================================================================
*/
static void CG_DrawTeamInfo(void) {
	int h;
	int i;
	vec4_t hcolor;
	int chatHeight;
	int lineHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT) {
		chatHeight = cg_teamChatHeight.integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if (chatHeight <= 0) {
		return; // disabled
	}

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_BOTTOM);

	lineHeight = CG_DrawStringLineHeight(UI_TINYFONT);

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * lineHeight;

		if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		} else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor(hcolor);
		CG_DrawPic(CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar);
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawString(CHATLOC_X + TINYCHAR_WIDTH, CHATLOC_Y - (cgs.teamChatPos - i) * lineHeight, cgs.teamChatMsgs[i % chatHeight], UI_DROPSHADOW|UI_TINYFONT, hcolor);
		}
	}
}

/*
=======================================================================================================================================
CG_DrawHoldableItem
=======================================================================================================================================
*/
static void CG_DrawHoldableItem(void) {
	int value;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_CENTER);

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];

	if (value) {
		CG_RegisterItemVisuals(value);
		CG_DrawPic(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
	}
}

/*
=======================================================================================================================================
CG_DrawPersistantPowerup
=======================================================================================================================================
*/
static void CG_DrawPersistantPowerup(void) {
	int value;

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_CENTER);

	value = cg.snap->ps.stats[STAT_PERSISTANT_POWERUP];

	if (value) {
		CG_RegisterItemVisuals(value);
		CG_DrawPic(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
	}
}
#endif // MISSIONPACK
/*
=======================================================================================================================================

	LAGOMETER

=======================================================================================================================================
*/

#define LAG_SAMPLES 128

typedef struct {
	int frameSamples[LAG_SAMPLES];
	int frameCount;
	int snapshotFlags[LAG_SAMPLES];
	int snapshotSamples[LAG_SAMPLES];
	int snapshotCount;
} lagometer_t;

lagometer_t lagometer;

/*
=======================================================================================================================================
CG_AddLagometerFrameInfo

Adds the current interpolate/extrapolate bar for this frame.
=======================================================================================================================================
*/
void CG_AddLagometerFrameInfo(void) {
	int offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[lagometer.frameCount &(LAG_SAMPLES - 1)] = offset;
	lagometer.frameCount++;
}

/*
=======================================================================================================================================
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and the number of snapshots that were dropped before it.
Pass NULL for a dropped packet.
=======================================================================================================================================
*/
void CG_AddLagometerSnapshotInfo(snapshot_t *snap) {

	// dropped packet
	if (!snap) {
		lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
		lagometer.snapshotCount++;
		return;
	}
	// add this snapshot's info
	lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
	lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
=======================================================================================================================================
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
=======================================================================================================================================
*/
static void CG_DrawDisconnect(void) {
	float x, y;
	int cmdNum;
	usercmd_t cmd;
	const char *s;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);

	if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time) { // special check for map_restart
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);
	// also add text in center of screen
	s = "Connection Interrupted!";

	CG_DrawString(SCREEN_WIDTH / 2, 100, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	// blink the icon
	if ((cg.time >> 9) & 1) {
		return;
	}

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif
	CG_DrawPic(x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga"));
}

#define MAX_LAGOMETER_PING 900
#define MAX_LAGOMETER_RANGE 300
/*
=======================================================================================================================================
CG_DrawLagometer
=======================================================================================================================================
*/
static void CG_DrawLagometer(void) {
	int a, x, y, i;
	float v;
	float ax, ay, aw, ah, mid, range;
	int color;
	float vscale;

	if (!cg_drawLagometer.integer || cgs.localServer) {
		CG_DrawDisconnect();
		return;
	}

	CG_SetScreenPlacement(PLACE_RIGHT, PLACE_BOTTOM);
	// draw the graph
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif
	CG_DrawPic(x, y, 48, 48, cgs.media.lagometerShader);

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;

	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	color = -1;
	range = ah / 3;
	mid = ay + range;
	vscale = range / MAX_LAGOMETER_RANGE;
	// draw the frame interpolate/extrapolate graph
	for (a = 0; a < aw; a++) {
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;

		if (v > 0) {
			if (color != 1) {
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}

			if (v > range) {
				v = range;
			}

			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 2) {
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}

			v = -v;

			if (v > range) {
				v = range;
			}

			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}
	// draw the snapshot latency/drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for (a = 0; a < aw; a++) {
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];

		if (v > 0) {
			if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED) {
				if (color != 5) {
					color = 5; // YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			} else {
				if (color != 3) {
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}

			v = v * vscale;

			if (v > range) {
				v = range;
			}

			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 4) {
				color = 4; // RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}

			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if (cg_nopredict.integer || cg_synchronousClients.integer) {
		CG_DrawBigString(x, y, "snc", 1.0);
	}

	CG_DrawDisconnect();
}

/*
=======================================================================================================================================

	CENTER PRINTING

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_CenterPrint

Called for important messages that should stay in the center of the screen for a few moments.
=======================================================================================================================================
*/
void CG_CenterPrint(const char *str, int y, int style, int priority) {

	// don't draw if this print message is less important
	if (cg.centerPrintTime && priority < cg.centerPrintPriority) {
		return;
	}

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintPriority = priority;
	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintStyle = style;
}

/*
=======================================================================================================================================
CG_DrawCenterString
=======================================================================================================================================
*/
static void CG_DrawCenterString(void) {
	float *color;

	if (!cg.centerPrintTime) {
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);

	if (!color) {
		cg.centerPrintTime = 0;
		cg.centerPrintPriority = 0;
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);
	CG_DrawStringAutoWrap(SCREEN_WIDTH / 2, cg.centerPrintY, cg.centerPrint, cg.centerPrintStyle, color, 0, 0, cgs.screenFakeWidth - 64);
}

/*
=======================================================================================================================================

	CROSSHAIR

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_DrawCrosshair
=======================================================================================================================================
*/
static void CG_DrawCrosshair(void) {
	float w, h, x, y;
	qhandle_t hShader;
	int ca;

	if (!cg_drawCrosshair.integer) {
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if (cg.renderingThirdPerson) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);
	// set color based on health
	if (cg_crosshairHealth.integer) {
		vec4_t hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}

	w = h = cg_crosshairSize.value;
	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	ca = cg_drawCrosshair.integer;

	if (ca < 0) {
		ca = 0;
	}

	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];

	CG_DrawPic(((SCREEN_WIDTH - w) * 0.5f) + x, ((SCREEN_HEIGHT - h) * 0.5f) + y, w, h, hShader);
	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
CG_DrawCrosshair3D
=======================================================================================================================================
*/
static void CG_DrawCrosshair3D(void) {
	float w, stereoSep, zProj, maxdist, xmax;
	qhandle_t hShader;
	int ca;
	trace_t trace;
	vec3_t endpos;
	char rendererinfos[128];
	refEntity_t ent;

	if (!cg_drawCrosshair.integer) {
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if (cg.renderingThirdPerson) {
		return;
	}

	w = cg_crosshairSize.value;
	ca = cg_drawCrosshair.integer;

	if (ca < 0) {
		ca = 0;
	}

	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];
	// use a different method rendering the crosshair so players don't see two of them when focusing their eyes at distant objects with
	// high stereo separation. We are going to trace to the next shootable object and place the crosshair in front of it.
	// first get all the important renderer information
	trap_Cvar_VariableStringBuffer("r_zProj", rendererinfos, sizeof(rendererinfos));
	zProj = atof(rendererinfos);

	trap_Cvar_VariableStringBuffer("r_stereoSeparation", rendererinfos, sizeof(rendererinfos));

	stereoSep = zProj / atof(rendererinfos);
	xmax = zProj * tan(cg.refdef.fov_x * M_PI / 360.0f);
	// let the trace run through until a change in stereo separation of the crosshair becomes less than one pixel.
	maxdist = cgs.glconfig.vidWidth * stereoSep * zProj / (2 * xmax);

	VectorMA(cg.refdef.vieworg, maxdist, cg.refdef.viewaxis[0], endpos);
	CG_Trace(&trace, cg.refdef.vieworg, NULL, NULL, endpos, 0, MASK_SHOT);

	memset(&ent, 0, sizeof(ent));

	ent.reType = RT_SPRITE;
	ent.renderfx = RF_DEPTHHACK|RF_CROSSHAIR;

	VectorCopy(trace.endpos, ent.origin);
	// scale the crosshair so it appears the same size for all distances
	ent.radius = w / 640 * xmax * trace.fraction * maxdist / zProj;
	ent.customShader = hShader;

	trap_R_AddRefEntityToScene(&ent);
}

/*
=======================================================================================================================================
CG_ScanForCrosshairEntity
=======================================================================================================================================
*/
static void CG_ScanForCrosshairEntity(void) {
	trace_t trace;
	vec3_t start, end;
	int content;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY);

	if (trace.entityNum >= MAX_CLIENTS) {
		return;
	}
	// if the player is in fog, don't show it
	content = CG_PointContents(trace.endpos, 0);

	if (content & CONTENTS_FOG) {
		return;
	}
	// if the player is invisible, don't show it
	if (cg_entities[trace.entityNum].currentState.powerups & (1 << PW_INVIS)) {
		return;
	}
	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}

/*
=======================================================================================================================================
CG_DrawCrosshairNames
=======================================================================================================================================
*/
static void CG_DrawCrosshairNames(void) {
	float *color;
	char *name;
	int crosshairSize, lineHeight, y;

	if (!cg_drawCrosshair.integer) {
		return;
	}

	if (!cg_drawCrosshairNames.integer) {
		return;
	}

	if (cg.renderingThirdPerson) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);
	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();
	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);

	if (!color) {
		return;
	}

	crosshairSize = cg_crosshairSize.value;
	// space for two lines above crosshair
	lineHeight = CG_DrawStringLineHeight(UI_BIGFONT);
	y = (SCREEN_HEIGHT - crosshairSize) / 2 - lineHeight * 2;
	name = cgs.clientinfo[cg.crosshairClientNum].name;
	color[3] *= 0.5f;

	CG_DrawStringExt(SCREEN_WIDTH / 2, y, name, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, color, 0, 0, 1);
}

/*
=======================================================================================================================================
CG_DrawSpectator
=======================================================================================================================================
*/
static void CG_DrawSpectator(void) {

	if (cg.scoreBoardShowing) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if (cgs.gametype == GT_TOURNAMENT) {
		CG_DrawString(320, 73, "Waiting to play", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	} else {
		CG_DrawString(320, 73, "Press ESC and use the JOIN menu to play", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	}
}

/*
=======================================================================================================================================
CG_DrawVote
=======================================================================================================================================
*/
static void CG_DrawVote(void) {
	char *s;
	int sec;
	char yesKeys[64];
	char noKeys[64];
	int lineHeight;

	if (!cgs.voteTime) {
		return;
	}

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_TOP);
	// play a talk beep whenever it is modified
	if (cgs.voteModified) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;

	if (sec < 0) {
		sec = 0;
	}

	CG_KeysStringForBinding("vote yes", yesKeys, sizeof(yesKeys));
	CG_KeysStringForBinding("vote no", noKeys, sizeof(noKeys));

	lineHeight = CG_DrawStringLineHeight(UI_SMALLFONT);

	s = va("Vote (%i): %s. Yes: %i, No: %i. Press %s to vote for Yes or %s to vote for No (or press ESC then click Vote).", sec, cgs.voteString, cgs.voteYes, cgs.voteNo, yesKeys, noKeys);
	CG_DrawSmallString(0, 58 + lineHeight, s, 1.0f);
}

/*
=======================================================================================================================================
CG_DrawTeamVote
=======================================================================================================================================
*/
static void CG_DrawTeamVote(void) {
	char *s;
	int sec, cs_offset;
	char yesKeys[64];
	char noKeys[64];
	int lineHeight;

	if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
		cs_offset = 0;
	} else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
		cs_offset = 1;
	} else {
		return;
	}

	if (!cgs.teamVoteTime[cs_offset]) {
		return;
	}

	CG_SetScreenPlacement(PLACE_LEFT, PLACE_TOP);
	// play a talk beep whenever it is modified
	if (cgs.teamVoteModified[cs_offset]) {
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;

	if (sec < 0) {
		sec = 0;
	}

	CG_KeysStringForBinding("vote yes", yesKeys, sizeof(yesKeys));
	CG_KeysStringForBinding("vote no", noKeys, sizeof(noKeys));

	lineHeight = CG_DrawStringLineHeight(UI_SMALLFONT);

	s = va("TEAMVOTE (%i): %s. Yes: %i, No: %i. Press %s to vote for Yes or %s to vote for No (or press ESC then click Vote).", sec, cgs.teamVoteString[cs_offset], cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset], yesKeys, noKeys);
	CG_DrawSmallString(0, 90 + lineHeight, s, 1.0f);
}

/*
=======================================================================================================================================
CG_DrawScoreboard
=======================================================================================================================================
*/
static qboolean CG_DrawScoreboard(void) {
#ifdef MISSIONPACK
	static qboolean firstTime = qtrue;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);

	if (menuScoreboard) {
		menuScoreboard->window.flags &= ~WINDOW_FORCED;
	}

	if (cg_paused.integer) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}
	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}
	// don't draw scoreboard during death while warmup up
	if (cg.warmup && !cg.showScores) {
		return qfalse;
	}

	if (cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION) {

	} else {
		if (!CG_FadeColor(cg.scoreFadeTime, FADE_TIME)) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			firstTime = qtrue;
			return qfalse;
		}
	}

	if (menuScoreboard == NULL) {
		if (cgs.gametype > GT_TOURNAMENT) {
			menuScoreboard = Menus_FindByName("teamscore_menu");
		} else {
			menuScoreboard = Menus_FindByName("score_menu");
		}
	}

	if (menuScoreboard) {
		if (firstTime) {
			CG_SetScoreSelection(menuScoreboard);
			firstTime = qfalse;
		}

		Menu_Paint(menuScoreboard, qtrue);
	}
	// load any models that have been deferred
	if (++cg.deferredPlayerLoading > 10) {
		CG_LoadDeferredPlayers();
	}

	return qtrue;
#else
	return CG_DrawOldScoreboard();
#endif
}

/*
=======================================================================================================================================
CG_DrawIntermission
=======================================================================================================================================
*/
static void CG_DrawIntermission(void) {
//	int key;
#ifdef MISSIONPACK
	//if (cg_singlePlayer.integer) {
	//	CG_DrawCenterString();
	//	return;
	//}
#else
	if (cgs.gametype == GT_SINGLE_PLAYER) {
		CG_DrawCenterString();
		return;
	}
#endif
	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=======================================================================================================================================
CG_DrawFollow
=======================================================================================================================================
*/
static qboolean CG_DrawFollow(void) {
	const char *name;
	int y;

	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
		return qfalse;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	y = 24;

	CG_DrawString(SCREEN_WIDTH / 2, y, "Following", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	y += CG_DrawStringLineHeight(UI_BIGFONT);

	name = cgs.clientinfo[cg.snap->ps.clientNum].name;

	CG_DrawString(SCREEN_WIDTH / 2, y, name, UI_CENTER|UI_DROPSHADOW|UI_GIANTFONT, NULL);
	y += CG_DrawStringLineHeight(UI_GIANTFONT);

	return qtrue;
}

/*
=======================================================================================================================================
CG_DrawAmmoWarning
=======================================================================================================================================
*/
static void CG_DrawAmmoWarning(void) {
	const char *s;

	if (cg_drawAmmoWarning.integer == 0) {
		return;
	}

	if (!cg.lowAmmoWarning) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if (cg.lowAmmoWarning == 2) {
		s = "OUT OF AMMO!";
	} else {
		s = "LOW AMMO WARNING!";
	}

	CG_DrawString(SCREEN_WIDTH / 2, 64, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
}

/*
=======================================================================================================================================
CG_DrawProxWarning
=======================================================================================================================================
*/
static void CG_DrawProxWarning(void) {
	char s[32];
	static int proxTime;
	int proxTick;
	int lineHeight;

	if (!(cg.snap->ps.eFlags & EF_TICKING)) {
		proxTime = 0;
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if (proxTime == 0) {
		proxTime = cg.time;
	}

	proxTick = 10 - ((cg.time - proxTime) / 1000);

	if (proxTick > 0 && proxTick <= 5) {
		Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
	} else {
		Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED!");
	}
	// put proxy warning below where the out of ammo location
	lineHeight = CG_DrawStringLineHeight(UI_BIGFONT);

	CG_DrawString(SCREEN_WIDTH / 2, 64 + lineHeight, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, g_color_table[ColorIndex(COLOR_RED)]);
}

/*
=======================================================================================================================================
CG_DrawWarmup
=======================================================================================================================================
*/
static void CG_DrawWarmup(void) {
	int sec;
	int i;
	clientInfo_t *ci1, *ci2;
	const char *s1 = "", *s2;

	sec = cg.warmup;

	if (!sec) {
		return;
	}

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_TOP);

	if (sec < 0) {
		s1 = "Waiting for players";
		CG_CenterPrint(s1, CENTERPRINT_HEIGHT, UI_CENTER|UI_VA_CENTER|UI_DROPSHADOW|UI_GIANTFONT, 99999);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;

		for (i = 0; i < cgs.maxclients; i++) {
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE) {
				if (!ci1) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if (ci1 && ci2) {
			s1 = va("%s vs %s", ci1->name, ci2->name);
		}
	} else {
		s1 = cgs.gametypeName;
	}

	sec = (sec - cg.time) / 1000;

	if (sec < 0) {
		cg.warmup = 0;
		sec = 0;
	}

	s2 = va("%s\nStarts in: %i", s1, sec + 1);

	if (sec != cg.warmupCount) {
		cg.warmupCount = sec;

		switch (sec) {
			case 0:
				trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
				break;
			case 1:
				trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
				break;
			case 2:
				trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
				break;
			default:
				break;
		}
	}

	CG_CenterPrint(s2, CENTERPRINT_HEIGHT, UI_CENTER|UI_VA_CENTER|UI_DROPSHADOW|UI_TITANFONT, 99999);
}
#ifdef MISSIONPACK
/*
=======================================================================================================================================
CG_DrawTimedMenus
=======================================================================================================================================
*/
void CG_DrawTimedMenus(void) {

	if (cg.voiceTime) {
		int t = cg.time - cg.voiceTime;

		if (t > 2500) {
			Menus_CloseByName("voiceMenu");
			trap_Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}
#endif
/*
=======================================================================================================================================
CG_Draw2D
=======================================================================================================================================
*/
static void CG_Draw2D(stereoFrame_t stereoFrame) {
#ifdef MISSIONPACK
	if (cgs.orderPending && cg.time > cgs.orderTime) {
		CG_CheckOrderPending();
	}
#endif
	// if we are taking a levelshot for the menu, don't draw anything
	if (cg.levelShot) {
		return;
	}

	if (cg_draw2D.integer == 0) {
		return;
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION) {
		CG_DrawIntermission();
		return;
	}
/*
	if (cg.cameraMode) {
		return;
	}
*/
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		CG_DrawSpectator();

		if (stereoFrame == STEREO_CENTER) {
			CG_DrawCrosshair();
		}

		CG_DrawCrosshairNames();
	} else {
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if (!cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0) {
#ifdef MISSIONPACK
			if (cg_drawStatus.integer) {
				CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);
				Menu_PaintAll();
				CG_DrawTimedMenus();
			}
#else
			CG_DrawStatusBar();
#endif
			CG_DrawAmmoWarning();
			CG_DrawProxWarning();

			if (stereoFrame == STEREO_CENTER) {
				CG_DrawCrosshair();
			}

			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
#ifndef MISSIONPACK
			CG_DrawHoldableItem();
			CG_DrawPersistantPowerup();
#endif
		}
	}

	if (cgs.gametype > GT_TOURNAMENT) {
#ifndef MISSIONPACK
		CG_DrawTeamInfo();
#endif
	}

	CG_DrawVote();
	CG_DrawTeamVote();
	CG_DrawLagometer();
#ifdef MISSIONPACK
	if (!cg_paused.integer) {
		CG_DrawUpperRight(stereoFrame);
	}
#else
	CG_DrawUpperRight(stereoFrame);
#endif
#ifndef MISSIONPACK
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
#endif
	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();

	if (!cg.scoreBoardShowing) {
		if (!CG_DrawFollow()) {
			CG_DrawWarmup();
			CG_DrawCenterString();
		}
	}
}

/*
=======================================================================================================================================
CG_DrawMiscGamemodels
=======================================================================================================================================
*/
void CG_DrawMiscGamemodels(void) {
	int i, j;
	refEntity_t ent;
	int drawn = 0;

	memset(&ent, 0, sizeof(ent));

	ent.reType = RT_MODEL;
	ent.nonNormalizedAxes = qtrue;
	// static gamemodels don't project shadows
	ent.renderfx = RF_NOSHADOW;

	for (i = 0; i < cg.numMiscGameModels; i++) {
		if (cgs.miscGameModels[i].radius) {
			if (CG_CullPointAndRadius(cgs.miscGameModels[i].org, cgs.miscGameModels[i].radius)) {
				continue;
			}
		}

		if (!trap_R_inPVS(cg.refdef.vieworg, cgs.miscGameModels[i].org)) {
			continue;
		}

		VectorCopy(cgs.miscGameModels[i].org, ent.origin);
		VectorCopy(cgs.miscGameModels[i].org, ent.oldorigin);
		VectorCopy(cgs.miscGameModels[i].org, ent.lightingOrigin);

		for (j = 0; j < 3; j++) {
			VectorCopy(cgs.miscGameModels[i].axes[j], ent.axis[j]);
		}

		ent.hModel = cgs.miscGameModels[i].model;

		trap_R_AddRefEntityToScene(&ent);

		drawn++;
	}
}

/*
=======================================================================================================================================
CG_DrawActive

Perform all drawing needed to completely fill the screen.
=======================================================================================================================================
*/
void CG_DrawActive(stereoFrame_t stereoView) {

	// optionally draw the info screen instead
	if (!cg.snap) {
		CG_DrawInformation();
		return;
	}
	// clear around the rendered view if sized down
	CG_TileClear();

	if (stereoView != STEREO_CENTER) {
		CG_DrawCrosshair3D();
	}

	CG_DrawMiscGamemodels();
	// draw 3D view
	trap_R_RenderScene(&cg.refdef);
	// draw status bar and other floating elements
	CG_Draw2D(stereoView);
}
