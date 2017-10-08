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
 Draw the scoreboard on top of the game screen.
**************************************************************************************************************************************/

#include "cg_local.h"

static qboolean localClient; // true if local client has been displayed
static qboolean drawBackgroundTeamScores;

/*
=======================================================================================================================================
CG_DrawClientScore
=======================================================================================================================================
*/
static void CG_DrawClientScore(int y, score_t *score, float *color, float fade, qboolean interLeaved) {
	clientInfo_t *ci;
	vec3_t headAngles;
	float hcolor[4];
	const char *s;
	char string[1024];
	int x, w;

	if (score->client < 0 || score->client >= cgs.maxclients) {
		Com_Printf("Bad score->client: %i\n", score->client);
		return;
	}
	// highlight your position
	if (score->client == cg.snap->ps.clientNum) {
		localClient = qtrue;
		drawBackgroundTeamScores = qtrue;

		if (cgs.gametype > GT_TOURNAMENT) {
			if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
				VectorSet(hcolor, 0.8f, 0.35f, 0.35f);
			} else {
				VectorSet(hcolor, 0.2f, 0.6f, 0.8f);
			}

			hcolor[3] = fade * 0.8;
		} else {
			VectorSet(hcolor, 0.9f, 1.0f, 0.9f);
			hcolor[3] = fade * 0.5;
		}

		if (!interLeaved) {
			CG_FillRect(1, y - 1, 638, 18 + 0.33f, hcolor);
			CG_DrawRect(1, y - 1, 638, 18 + 0.66f, 0.75f, colorBlack);
			CG_DrawRect(1, y - 1, 638, 18 + 0.66f, 0.33f, colorLtGrey);
		} else {
			CG_FillRect(178, 443, 157, 28, hcolor);
		}
	}
	// draw the big two score display after the highlighted position, but before any text is drawn
	hcolor[3] = fade * 0.45f;

	if (drawBackgroundTeamScores) {
		if (cgs.gametype > GT_TOURNAMENT) {
			if (cg.teamScores[0] <= cg.teamScores[1]) {
				VectorSet(hcolor, 0.2f, 0.6f, 1.0f);
				s = va("%i", cg.teamScores[1]);
				CG_DrawString(320, 160, s, UI_CENTER|UI_TITANFONT, hcolor);

				VectorSet(hcolor, 1.0f, 0.0f, 0.0f);
				s = va("%i", cg.teamScores[0]);
				CG_DrawString(320, 330, s, UI_CENTER|UI_TITANFONT, hcolor);
			} else {
				VectorSet(hcolor, 1.0f, 0.0f, 0.0f);
				s = va("%i", cg.teamScores[0]);
				CG_DrawString(320, 160, s, UI_CENTER|UI_TITANFONT, hcolor);

				VectorSet(hcolor, 0.2f, 0.6f, 1.0f);
				s = va("%i", cg.teamScores[1]);
				CG_DrawString(320, 330, s, UI_CENTER|UI_TITANFONT, hcolor);
			}
		}

		drawBackgroundTeamScores = qfalse;
	}

	ci = &cgs.clientinfo[score->client];
	// draw the face
	VectorClear(headAngles);

	headAngles[YAW] = 180;

	if (interLeaved) {
		CG_DrawHead(180, 453, 16, 16, score->client, headAngles);
		// draw the score
		Com_sprintf(string, sizeof(string), "Score: %i", score->score);
		CG_DrawString(183, 445, string, UI_LEFT|UI_DROPSHADOW|UI_SMALLFONT, color);
		// draw the name
		CG_DrawString(197, 455, ci->name, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
		return;
	}
	// draw the country flag (for humans only) or the bot skill marker
	if (ci->botSkill > 0 && ci->botSkill <= 5) {
		CG_DrawPic(8, y, 16, 16, cgs.media.botSkillShaders[ci->botSkill - 1]);
	} else {
		// Tobias FIXME: GeoIP countryflags
	}

	CG_DrawHead(30, y, 16, 16, score->client, headAngles);

	x = 50;

	if (score->ping != -1) {
		if (cgs.gametype > GT_TOURNAMENT) {
			// draw the team task
			if (ci->teamTask != TEAMTASK_NONE) {
				switch (ci->teamTask) {
					case TEAMTASK_OFFENSE:
						CG_DrawPic(x, y, 16, 16, cgs.media.assaultShader);
						break;
					case TEAMTASK_DEFENSE:
						CG_DrawPic(x, y, 16, 16, cgs.media.defendShader);
						break;
					case TEAMTASK_RETRIEVE:
						CG_DrawPic(x, y, 16, 16, cgs.media.retrieveShader);
						break;
					case TEAMTASK_ESCORT:
						CG_DrawPic(x, y, 16, 16, cgs.media.escortShader);
						break;
					case TEAMTASK_FOLLOW:
						CG_DrawPic(x, y, 16, 16, cgs.media.followShader);
						break;
					case TEAMTASK_CAMP:
						CG_DrawPic(x, y, 16, 16, cgs.media.campShader);
						break;
					case TEAMTASK_PATROL:
						CG_DrawPic(x, y, 16, 16, cgs.media.patrolShader);
						break;
					default:
						break;
				}

				x += 19;
			}
			// draw the fireteam
			// Tobias FIXME: fireteam
			// draw the class
			// Tobias FIXME: player class (medic etc.)
			// draw the flag
			if (ci->powerups &(1 << PW_REDFLAG)) {
				CG_DrawFlagModel(x, y, 16, 16, TEAM_RED, qfalse);
				x += 19;
			} else if (ci->powerups &(1 << PW_BLUEFLAG)) {
				CG_DrawFlagModel(x, y, 16, 16, TEAM_BLUE, qfalse);
				x += 19;
			} else if (ci->powerups &(1 << PW_NEUTRALFLAG)) {
				CG_DrawFlagModel(x, y, 16, 16, TEAM_FREE, qfalse);
				x += 19;
			}
		} else {
			// draw the wins/losses
			if (cgs.gametype == GT_TOURNAMENT) {
				Com_sprintf(string, sizeof(string), "%i/%i", ci->wins, ci->losses);
				w = CG_DrawStrlen(string, UI_DEFAULTFONT) + 4;
				CG_DrawString(x, y + 2, string, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
				x += w;
			}
		}
		// draw the medals/rank
		if (score->client == cg.snap->ps.clientNum) {
			Vector4Set(hcolor, 0.1f, 0.1f, 0.1f, 0.5f);
			CG_FillRect(178, 443, 157, 28, hcolor);

			CG_DrawPic(180, 445, 16, 16, cgs.media.medalAccuracy);
			Com_sprintf(string, sizeof(string), "%i %%", score->accuracy);
			CG_DrawString(188, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(203, 445, 16, 16, cgs.media.medalExcellent);
			Com_sprintf(string, sizeof(string), "%i", score->excellentCount);
			CG_DrawString(211, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(226, 445, 16, 16, cgs.media.medalImpressive);
			Com_sprintf(string, sizeof(string), "%i", score->impressiveCount);
			CG_DrawString(234, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(249, 445, 16, 16, cgs.media.medalGauntlet);
			Com_sprintf(string, sizeof(string), "%i", score->gauntletCount);
			CG_DrawString(257, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(272, 445, 16, 16, cgs.media.medalCapture);
			Com_sprintf(string, sizeof(string), "%i", score->captures);
			CG_DrawString(280, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(295, 445, 16, 16, cgs.media.medalDefend);
			Com_sprintf(string, sizeof(string), "%i", score->defendCount);
			CG_DrawString(303, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);

			CG_DrawPic(318, 445, 16, 16, cgs.media.medalAssist);
			Com_sprintf(string, sizeof(string), "%i", score->assistCount);
			CG_DrawString(326, 461, string, UI_CENTER|UI_DROPSHADOW|UI_SMALLFONT, color);
		}
	}

	y += 2;
	// 1st column: draw the status line
	if (score->ping == -1) {
		Com_sprintf(string, sizeof(string), "Connecting");
	// add the "ready" marker for intermission exiting
	} else if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client)) {
		Com_sprintf(string, sizeof(string), "Ready");
	} else if (ci->teamLeader) {
		Com_sprintf(string, sizeof(string), "Leader");
	} else {
		Com_sprintf(string, sizeof(string), NULL);
	}

	CG_DrawString(x, y, string, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	// 2nd column: draw the name
	CG_DrawString(183, y, ci->name, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	// 3rd column: draw the score
	Com_sprintf(string, sizeof(string), "%i", score->score);
	CG_DrawString(361, y, string, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	// 4th column: draw the time
	Com_sprintf(string, sizeof(string), "%i", score->time);
	CG_DrawString(521, y, string, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	// 5th column: draw the ping
	Com_sprintf(string, sizeof(string), "%i", score->ping);
	CG_DrawString(597, y, string, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
}

/*
=======================================================================================================================================
CG_SetClientScore
=======================================================================================================================================
*/
static int CG_SetClientScore(int y, team_t team, float fade, int maxClients) {
	int i;
	score_t *score;
	float color[4];
	int count;
	clientInfo_t *ci;

	color[0] = color[1] = color[2] = 1.0;
	color[3] = fade;

	count = 0;

	for (i = 0; i < cg.numScores && count < maxClients; i++) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[score->client];

		if (team != ci->team) {
			continue;
		}

		CG_DrawClientScore(y + 18 * count, score, color, fade, qfalse);

		count++;
	}

	return count;
}

/*
=======================================================================================================================================
CG_DrawSpectators
=======================================================================================================================================
*/
static void CG_DrawSpectators(float x, int y, int w, int h, vec4_t color) {
	char *text = cg.spectatorList;
	float textWidth = MAX(w, Text_Width(text, &cgs.media.giantFont, 1, 0));
	int now = trap_Milliseconds();
	int delta = now - cg.spectatorTime;

	CG_SetClipRegion(x, y, w, h);
	CG_DrawFloatString(x - cg.spectatorOffset, y, text, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	CG_DrawFloatString(x + textWidth - cg.spectatorOffset, y, text, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, color);
	CG_ClearClipRegion();

	cg.spectatorOffset += (delta / 1000.0f) * 30.0f;

	while (cg.spectatorOffset > textWidth) {
		cg.spectatorOffset -= textWidth;
	}

	cg.spectatorTime = now;
}

/*
=======================================================================================================================================
CG_GetKilledByText
=======================================================================================================================================
*/
static const char *CG_GetKilledByText(void) {
	const char *s = "";

	if (cg.killerName[0]) {
		s = va("Fragged by %s - ", cg.killerName);
	}

	return s;
}

/*
=======================================================================================================================================
CG_DrawOldScoreboard

Draw the normal in-game scoreboard.
=======================================================================================================================================
*/
qboolean CG_DrawOldScoreboard(void) {
	int y, i, min, tens, ones;
	float *fadeColor, fade, tempy;
	vec4_t hcolor;
	char *s;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_CENTER);
	// don't draw amuthing if the menu or console is up
	if (cg_paused.integer) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}
	// don't draw scoreboard during death while warmup up
	if (cg.warmup && !cg.showScores) {
		return qfalse;
	}

	if (cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);

		if (!fadeColor) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			return qfalse;
		}
		// ZTM: FIXME?: to actually fade, should be fade = fadeColor[3] and later CG_DrawString should use fadeColor
		fade = *fadeColor;
	}
	// draw the colored scoreboard header
	Vector4Set(hcolor, 0.25f, 0.30f, 0.32f, 0.75f);
	CG_FillRect(1, 42, 638, 28, hcolor);
	// the scoreboard header string
	CG_DrawString(5, 44, "SCOREBOARD", UI_LEFT|UI_DROPSHADOW|UI_GIANTFONT, NULL);
	// draw the colored rank/fragged by ... line
	Vector4Set(hcolor, 0.1f, 0.1f, 0.1f, 0.75f);
	CG_FillRect(1, 70, 638, 20, hcolor);
	// the current rank and 'fragged by ...' string
	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
		if (cgs.gametype > GT_TOURNAMENT) {
			if (cg.teamScores[0] == cg.teamScores[1]) {
				s = va("%sTeams are tied at %i", CG_GetKilledByText(), cg.teamScores[0]);
			} else if (cg.teamScores[0] >= cg.teamScores[1]) {
				s = va("%sRed leads %i to %i", CG_GetKilledByText(), cg.teamScores[0], cg.teamScores[1]);
			} else {
				s = va("%sBlue leads %i to %i", CG_GetKilledByText(), cg.teamScores[1], cg.teamScores[0]);
			}
		} else {
			s = va("%s%s place with %i", CG_GetKilledByText(), CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1), cg.snap->ps.persistant[PERS_SCORE]);
		}
	} else {
		if (cgs.gametype != GT_TOURNAMENT) {
			s = "Press ESC and use the JOIN menu to play";
		} else {
			s = "Waiting to play";
		}
	}

	CG_DrawString(320, 73, s, UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	// draw the colored status line
	Vector4Set(hcolor, 0.84f, 0.75f, 0.65f, 0.60f);
	CG_FillRect(1, 90, 638, 18, hcolor);
	// the status string
	CG_DrawString(90, 91, "STATUS", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	CG_DrawString(268, 91, "NAME", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	CG_DrawString(438, 91, "SCORE", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	CG_DrawString(556, 91, "TIME", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	CG_DrawString(616, 91, "PING", UI_CENTER|UI_DROPSHADOW|UI_BIGFONT, NULL);
	// the spectator line at the bottom
	Vector4Set(hcolor, 0.5f, 0.5f, 0.5f, 0.33f);
	CG_FillRect(1, 443, 177, 28, hcolor);
	CG_FillRect(335, 443, 304, 28, hcolor);

	CG_DrawString(340, 445, "Spectators:", UI_LEFT|UI_DROPSHADOW|UI_SMALLFONT, NULL);
	CG_DrawSpectators(335, 455, 304, 18, NULL);
	// print server time
	CG_DrawString(6, 445, "Server time:", UI_LEFT|UI_DROPSHADOW|UI_SMALLFONT, NULL);

	ones = cg.time / 1000;
	min = ones / 60;
	ones %= 60;
	tens = ones / 10;
	ones %= 10;
	s = va("%i:%i%i", min, tens, ones);

	CG_DrawString(6, 455, s, UI_LEFT|UI_DROPSHADOW|UI_DEFAULTFONT, NULL);
	// draw the rows!
	localClient = qfalse;

	y = 108;
	// save off y val
	tempy = y;
	// teamplay scoreboard
	if (cgs.gametype > GT_TOURNAMENT) {
		hcolor[3] = fade * 0.4f;

		if (cg.teamScores[0] <= cg.teamScores[1]) {
			// draw the upper blue color bands (0-9)
			for (i = 0; i < 9; i++) {
				if (i % 2 == 0) {
					VectorSet(hcolor, 0.35f, 0.75f, 0.95f); // light blue
				} else {
					VectorSet(hcolor, 0.05f, 0.45f, 0.65f); // dark blue
				}

				CG_FillRect(1, y, 638, 18, hcolor);

				trap_R_SetColor(colorBlack);
				CG_DrawPic(1, y, 638, 0.33f, cgs.media.whiteShader);
				trap_R_SetColor(NULL);

				y += 18;
			}
			// fill in the scores, do it now to draw the scores on top of the rows!
			CG_SetClientScore(109, TEAM_BLUE, fade, 9);
			// draw the lower red color bands (10-18)
			y = 278;

			for (i = 0; i < 9; i++) {
				if (i % 2 == 0) {
					VectorSet(hcolor, 0.95f, 0.4f, 0.4f); // light red
				} else {
					VectorSet(hcolor, 0.65f, 0.1f, 0.1f); // dark red
				}

				CG_FillRect(1, y, 638, 18, hcolor);

				trap_R_SetColor(colorBlack);
				CG_DrawPic(1, y, 638, 0.33f, cgs.media.whiteShader);
				trap_R_SetColor(NULL);

				y += 18;
			}
			// fill in the scores, do it now to draw the scores on top of the rows!
			CG_SetClientScore(279, TEAM_RED, fade, 9);
		} else {
			// draw the upper red color bands (0-9)
			for (i = 0; i < 9; i++) {
				if (i % 2 == 0) {
					VectorSet(hcolor, 0.95f, 0.4f, 0.4f); // light red
				} else {
					VectorSet(hcolor, 0.65f, 0.1f, 0.1f); // dark red
				}

				CG_FillRect(1, y, 638, 18, hcolor);

				trap_R_SetColor(colorBlack);
				CG_DrawPic(1, y, 638, 0.33f, cgs.media.whiteShader);
				trap_R_SetColor(NULL);

				y += 18;
			}
			// fill in the scores, do it now to draw the scores on top of the rows!
			CG_SetClientScore(109, TEAM_RED, fade, 9);
			// draw the lower blue color bands (10-18)
			y = 278;

			for (i = 0; i < 9; i++) {
				if (i % 2 == 0) {
					VectorSet(hcolor, 0.35f, 0.75f, 0.95f); // light blue
				} else {
					VectorSet(hcolor, 0.05f, 0.45f, 0.65f); // dark blue
				}

				CG_FillRect(1, y, 638, 18, hcolor);

				trap_R_SetColor(colorBlack);
				CG_DrawPic(1, y, 638, 0.33f, cgs.media.whiteShader);
				trap_R_SetColor(NULL);

				y += 18;
			}
			// fill in the scores, do it now to draw the scores on top of the rows!
			CG_SetClientScore(279, TEAM_BLUE, fade, 9);
		}
	// free for all scoreboard
	} else {
		hcolor[3] = fade * 0.6f;

		for (i = 0; i < 18; i++) {
			if (i % 2 == 0) {
				VectorSet(hcolor, 0.3f, 0.3f, 0.3f); // light
			} else {
				VectorSet(hcolor, 0.f, 0.f, 0.f); // dark
			}

			CG_FillRect(1, y, 638, 18, hcolor);

			trap_R_SetColor(colorBlack);
			CG_DrawPic(1, y, 638, 0.33f, cgs.media.whiteShader);
			trap_R_SetColor(NULL);

			y += 18;
		}
		// fill in the scores, do it now to draw the scores on top of the rows!
		CG_SetClientScore(109, TEAM_FREE, fade, 18);
	}

	hcolor[3] = 1;

	if (!localClient) {
		drawBackgroundTeamScores = qtrue;

		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
			// draw local client at the bottom
			for (i = 0; i < cg.numScores; i++) {
				if (cg.scores[i].client == cg.snap->ps.clientNum) {
					CG_DrawClientScore(445, &cg.scores[i], fadeColor, fade, qtrue);
					break;
				}
			}
		}
	}

	y = (int)tempy;

	trap_R_SetColor(colorBlack);

	if (cgs.gametype > GT_TOURNAMENT) {
		// draw the columns (the columns will overdraw the highlighted position on the scoreboard)
		CG_DrawPic(180, 90, 0.33f, 180, cgs.media.whiteShader);
		CG_DrawPic(358, 90, 0.33f, 180, cgs.media.whiteShader);
		CG_DrawPic(518, 90, 0.33f, 180, cgs.media.whiteShader);
		CG_DrawPic(594, 90, 0.33f, 180, cgs.media.whiteShader);

		CG_DrawPic(180, 278, 0.33f, 162, cgs.media.whiteShader);
		CG_DrawPic(358, 278, 0.33f, 162, cgs.media.whiteShader);
		CG_DrawPic(518, 278, 0.33f, 162, cgs.media.whiteShader);
		CG_DrawPic(594, 278, 0.33f, 162, cgs.media.whiteShader);
		// draw the window frame on top of everything
		CG_DrawRect(1, 42, 638, 229, 0.75f, colorBlack);
		CG_DrawRect(1, 42, 638, 229, 0.33f, colorLtGrey);

		CG_DrawRect(1, 278, 638, 163, 0.75f, colorBlack);
		CG_DrawRect(1, 278, 638, 163, 0.33f, colorLtGrey);
	} else {
		// draw the columns (the columns will overdraw the highlighted position on the scoreboard)
		CG_DrawPic(180, 90, 0.45f, 342, cgs.media.whiteShader);
		CG_DrawPic(358, 90, 0.45f, 342, cgs.media.whiteShader);
		CG_DrawPic(518, 90, 0.45f, 342, cgs.media.whiteShader);
		CG_DrawPic(594, 90, 0.45f, 342, cgs.media.whiteShader);
		// draw the window frame on top of everything
		CG_DrawRect(1, 42, 638, 391, 0.75f, colorBlack);
		CG_DrawRect(1, 42, 638, 391, 0.33f, colorLtGrey);
	}

	trap_R_SetColor(NULL);
	// 1st main frame
	CG_DrawRect(1, 42, 638, 28, 0.75f, colorBlack);
	CG_DrawRect(1, 42, 638, 28, 0.33f, colorLtGrey);
	// 2nd main frame
	CG_DrawRect(1, 90, 638, 18, 0.75f, colorBlack);
	CG_DrawRect(1, 90, 638, 18, 0.33f, colorLtGrey);
	// 3rd main frame(s)
	CG_DrawRect(1, 443, 177, 28, 0.75f, colorBlack);
	CG_DrawRect(1, 443, 177, 28, 0.33f, colorLtGrey);

	CG_DrawRect(178, 443, 157, 28, 0.75f, colorBlack);
	CG_DrawRect(178, 443, 157, 28, 0.33f, colorLtGrey);

	CG_DrawRect(335, 443, 304, 28, 0.75f, colorBlack);
	CG_DrawRect(335, 443, 304, 28, 0.33f, colorLtGrey);
	// load any models that have been deferred
	if (++cg.deferredPlayerLoading > 10) {
		CG_LoadDeferredPlayers();
	}

	return qtrue;
}
