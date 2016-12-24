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
 Local definitions for the bg (both games) files.
**************************************************************************************************************************************/

#define MIN_WALK_NORMAL 0.7f // can't walk on very steep slopes
#define STEPSIZE 18
#define JUMP_VELOCITY 270
#define TIMER_LAND 130
#define TIMER_GESTURE 2294
#define OVERCLIP 1.001f

#define VOICECHAT_GETFLAG			"getflag"			// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"			// command someone to go on offense
#define VOICECHAT_DEFENDFLAG		"defendflag"		// command someone to defend the flag
#define VOICECHAT_DEFEND			"defend"			// command someone to go on defense
#define VOICECHAT_RETURNFLAG		"returnflag"		// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"	// command someone to follow the flag carrier
#define VOICECHAT_FOLLOWME			"followme"			// command someone to follow you
#define VOICECHAT_CAMP				"camp"				// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_PATROL			"patrol"			// command someone to go on patrol (roam)
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"		// I want to be on offense
#define VOICECHAT_WANTONDEFENSE		"wantondefense"		// I want to be on defense
#define VOICECHAT_WHOISLEADER		"whoisleader"		// who is the team leader
#define VOICECHAT_STOPLEADER		"stopleader"		// I resign leadership
#define VOICECHAT_STARTLEADER		"startleader"		// I'm the leader
#define VOICECHAT_YES				"yes"				// yes, affirmative, etc.
#define VOICECHAT_NO				"no"				// no, negative, etc.
#define VOICECHAT_ONGETFLAG			"ongetflag"			// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"			// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"			// I'm on defense
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"		// I'm returning our flag
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"	// I'm following the flag carrier
#define VOICECHAT_ONFOLLOW			"onfollow"			// I'm following
#define VOICECHAT_ONCAMPING			"oncamp"			// I'm camping somewhere
#define VOICECHAT_ONPATROL			"onpatrol"			// I'm on patrol (roaming)
#define VOICECHAT_IHAVEFLAG			"ihaveflag"			// I have the flag
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"		// the enemy has our flag (CTF)
#define VOICECHAT_BASEATTACK		"baseattack"		// the base is under attack
#define VOICECHAT_INPOSITION		"inposition"		// I'm in position
#define VOICECHAT_PRAISE			"praise"			// you did something good
#define VOICECHAT_KILLINSULT		"kill_insult"		// I just killed you
#define VOICECHAT_TAUNT				"taunt"				// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"		// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"		// I just killed you with the gauntlet
#define VOICECHAT_TRASH				"trash"				// lots of trash talk

/**************************************************************************************************************************************
 All of the locals will be zeroed before each pmove, just to make damn sure we don't have any differences when running on client or
 server.
**************************************************************************************************************************************/

typedef struct {
	vec3_t forward, right, up;
	float frametime;
	int msec;
	qboolean walking;
	qboolean groundPlane;
	trace_t groundTrace;
	float impactSpeed;
	vec3_t previous_origin;
	vec3_t previous_velocity;
	int previous_waterlevel;
} pml_t;

extern pmove_t *pm;
extern pml_t pml;
// movement parameters
extern float pm_stopspeed;
extern float pm_duckScale;
extern float pm_swimScale;
extern float pm_accelerate;
extern float pm_airaccelerate;
extern float pm_wateraccelerate;
extern float pm_flyaccelerate;
extern float pm_friction;
extern float pm_waterfriction;
extern int c_pmove;

void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void PM_AddTouchEnt(int entityNum);
void PM_AddEvent(int newEvent);
qboolean PM_SlideMove(qboolean gravity);
void PM_StepSlideMove(qboolean gravity);
