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

#include "g_local.h"

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience. They are turned into normal brushes by the utilities.
*/

/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc.), but removed during gameplay.
*/
void SP_info_camp(gentity_t *self) {
	G_SetOrigin(self, self->s.origin);
}

/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc.), but removed during gameplay.
*/
void SP_info_null(gentity_t *self) {
	G_FreeEntity(self);
}

/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull(gentity_t *self) {
	G_SetOrigin(self, self->s.origin);
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LINEAR
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light(gentity_t *self) {
	G_FreeEntity(self);
}

/*
=======================================================================================================================================

	TELEPORTERS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
TeleportPlayer
=======================================================================================================================================
*/
void TeleportPlayer(gentity_t *player, vec3_t origin, vec3_t angles) {
	gentity_t *tent;
	qboolean noAngles;
	playerState_t *ps;
	int clientNum;

	ps = G_GetEntityPlayerState(player);

	if (!ps) {
		return;
	}

	noAngles = (angles[0] > 999999.0);
	// use temp events at source and destination to prevent the effect from getting dropped by a second player event
	if (ps->pm_type != PM_SPECTATOR) {
		tent = G_TempEntity(ps->origin, EV_PLAYER_TELEPORT_OUT);
		tent->s.clientNum = player->s.clientNum;
		tent = G_TempEntity(origin, EV_PLAYER_TELEPORT_IN);
		tent->s.clientNum = player->s.clientNum;
	}
	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity(player);
	VectorCopy(origin, ps->origin);

	ps->origin[2] += 1;

	if (!noAngles) {
		// spit the player out
		AngleVectorsForward(angles, ps->velocity);
		VectorScale(ps->velocity, 400, ps->velocity);

		ps->pm_time = 160; // hold time
		ps->pm_flags |= PMF_TIME_KNOCKBACK;
		// set angles
		SetClientViewAngle(player, angles);
	}
	// toggle the teleport bit so the client knows to not lerp
	ps->eFlags ^= EF_TELEPORT_BIT;
	// kill anything at the destination
	if (ps->pm_type != PM_SPECTATOR) {
		G_KillBox(player);
	}
	// save results of pmove
	clientNum = player->s.clientNum;

	BG_PlayerStateToEntityState(ps, &player->s, qtrue);

	player->s.clientNum = clientNum;
	// use the precise origin for linking
	VectorCopy(ps->origin, player->r.currentOrigin);

	if (ps->pm_type != PM_SPECTATOR) {
		trap_LinkEntity(player);
	}
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest(gentity_t *ent) {

}

/*
=======================================================================================================================================

	MODELS

=======================================================================================================================================
*/

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model" arbitrary .md3 file to display
*/
void SP_misc_model(gentity_t *ent) {
#if 0
	ent->s.modelindex = G_ModelIndex(ent->model);

	VectorSet(ent->mins, -16, -16, -16);
	VectorSet(ent->maxs, 16, 16, 16);
	trap_LinkEntity(ent);
	G_SetOrigin(ent, ent->s.origin);
	VectorCopy(ent->s.angles, ent->s.apos.trBase);
#else
	G_FreeEntity(ent);
#endif
}

/*
=======================================================================================================================================
LocateMaster
=======================================================================================================================================
*/
void LocateMaster(gentity_t *ent) {

	ent->target_ent = G_PickTarget(ent->target);

	if (ent->target_ent) {
		ent->r.visDummyNum = ent->target_ent->s.number;
	} else {
		G_Printf("Couldn't find target (%s) for misc_vis_dummy at %s\n", ent->target, vtos(ent->r.currentOrigin));
		G_FreeEntity(ent);
	}
}

/*QUAKED misc_vis_dummy (1 .5 0) (-8 -8 -8) (8 8 8)
If this entity is "visible" (in player's PVS) then it's target is forced to be active whether it is in the player's PVS or not.
This entity itself is never visible or transmitted to clients.
For safety, you should have each dummy only point at one entity (however, it's okay to have many dummies pointing at one entity)
*/
void SP_misc_vis_dummy(gentity_t *ent) {

	if (!ent->target) {
		G_Printf("No target specified for misc_vis_dummy at %s\n", vtos(ent->r.currentOrigin));
		G_FreeEntity(ent);
		return;
	}

	ent->r.svFlags |= SVF_VISDUMMY;

	G_SetOrigin(ent, ent->s.origin);
	trap_LinkEntity(ent);

	ent->think = LocateMaster;
	ent->nextthink = level.time + 1000;
}

/*QUAKED misc_vis_dummy_multiple (1 .5 0) (-8 -8 -8) (8 8 8)
If this entity is "visible" (in player's PVS) then it's target is forced to be active whether it is in the player's PVS or not.
This entity itself is never visible or transmitted to clients.
This entity was created to have multiple speakers targeting it
*/
void SP_misc_vis_dummy_multiple(gentity_t *ent) {

	if (!ent->targetname) {
		G_Printf("misc_vis_dummy_multiple needs a targetname at %s\n", vtos(ent->r.currentOrigin));
		G_FreeEntity(ent);
		return;
	}

	ent->r.svFlags |= SVF_VISDUMMY_MULTIPLE;

	G_SetOrigin(ent, ent->s.origin);
	trap_LinkEntity(ent);
}

/*
=======================================================================================================================================

	PORTALS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
LocateCamera
=======================================================================================================================================
*/
void LocateCamera(gentity_t *ent) {
	vec3_t dir;
	gentity_t *target;
	gentity_t *owner;

	owner = G_PickTarget(ent->target);

	if (!owner) {
		G_Printf("Couldn't find target for misc_portal_surface\n");
		G_FreeEntity(ent);
		return;
	}

	ent->r.ownerNum = owner->s.number;
	// frame holds the rotate speed
	if (owner->spawnflags & 1) {
		ent->s.frame = 25;
	} else if (owner->spawnflags & 2) {
		ent->s.frame = 75;
	}
	// swing camera ?
	if (owner->spawnflags & 4) {
		ent->s.powerups = 1;
	} else {
		// set to 0 for no rotation at all
		ent->s.powerups = 0;
	}
	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy(owner->s.origin, ent->s.origin2);
	// see if the portal_camera has a target
	target = G_PickTarget(owner->target);

	if (target) {
		VectorSubtract(target->s.origin, owner->s.origin, dir);
		VectorNormalize(dir);
	} else {
		G_SetMovedir(owner->s.angles, dir);
	}

	ent->s.eventParm = DirToByte(dir);
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) {

	VectorClear(ent->r.mins);
	VectorClear(ent->r.maxs);
	trap_LinkEntity(ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if (!ent->target) {
		VectorCopy(ent->s.origin, ent->s.origin2);
	} else {
		ent->think = LocateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) SLOWROTATE FASTROTATE NOSWING
The target for a misc_portal_director. You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float roll;

	VectorClear(ent->r.mins);
	VectorClear(ent->r.maxs);
	trap_LinkEntity(ent);
	G_SpawnFloat("roll", "0", &roll);

	ent->s.clientNum = roll / 360.0 * 256;
}

/*
=======================================================================================================================================

	SHOOTERS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
Use_Shooter
=======================================================================================================================================
*/
void Use_Shooter(gentity_t *ent, gentity_t *other, gentity_t *activator) {
	vec3_t dir;
	float deg;
	vec3_t up, right;

	// see if we have a target
	if (ent->enemy) {
		VectorSubtract(ent->enemy->r.currentOrigin, ent->s.origin, dir);
		VectorNormalize(dir);
	} else {
		VectorCopy(ent->movedir, dir);
	}
	// randomize a bit
	PerpendicularVector(up, dir);
	CrossProduct(up, dir, right);

	deg = crandom() * ent->random;

	VectorMA(dir, deg, up, dir);

	deg = crandom() * ent->random;

	VectorMA(dir, deg, right, dir);
	VectorNormalize(dir);

	switch (ent->s.weapon) {
		case WP_GRENADELAUNCHER:
			fire_grenade(ent, ent->s.origin, dir);
			break;
		case WP_ROCKETLAUNCHER:
			fire_rocket(ent, ent->s.origin, dir);
			break;
		case WP_PLASMAGUN:
			fire_plasma(ent, ent->s.origin, dir);
			break;
	}

	G_AddEvent(ent, EV_FIRE_WEAPON, 0);
}

/*
=======================================================================================================================================
InitShooter_Finish
=======================================================================================================================================
*/
static void InitShooter_Finish(gentity_t *ent) {

	ent->enemy = G_PickTarget(ent->target);
	ent->think = 0;
	ent->nextthink = 0;
}

/*
=======================================================================================================================================
InitShooter
=======================================================================================================================================
*/
void InitShooter(gentity_t *ent, int weapon) {

	ent->use = Use_Shooter;
	ent->s.weapon = weapon;

	RegisterItem(BG_FindItemForWeapon(weapon));
	G_SetMovedir(ent->s.angles, ent->movedir);

	if (!ent->random) {
		ent->random = 1.0;
	}

	ent->random = sin(M_PI * ent->random / 180);
	// target might be a moving object, so we can't set movedir for it
	if (ent->target) {
		ent->think = InitShooter_Finish;
		ent->nextthink = level.time + 500;
	}

	trap_LinkEntity(ent);
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the target (1.0 default).
*/
void SP_shooter_rocket(gentity_t *ent) {
	InitShooter(ent, WP_ROCKETLAUNCHER);
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target (1.0 default).
*/
void SP_shooter_plasma(gentity_t *ent) {
	InitShooter(ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the target (1.0 default).
*/
void SP_shooter_grenade(gentity_t *ent) {
	InitShooter(ent, WP_GRENADELAUNCHER);
}
