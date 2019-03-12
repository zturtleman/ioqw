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

/*
=======================================================================================================================================
ScorePlum
=======================================================================================================================================
*/
void ScorePlum(gentity_t *ent, vec3_t origin, int score) {
	gentity_t *plum;

	plum = G_TempEntity(origin, EV_SCOREPLUM);
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
=======================================================================================================================================
AddScore

Adds score to both the client and his team.
=======================================================================================================================================
*/
void AddScore(gentity_t *ent, vec3_t origin, int score) {

	if (!ent->client) {
		return;
	}
	// no scoring during pre-match warmup
	if (level.warmupTime) {
		return;
	}
	// show score plum
	ScorePlum(ent, origin, score);

	ent->client->ps.persistant[PERS_SCORE] += score;

	if (g_gametype.integer == GT_TEAM) {
		AddTeamScore(origin, ent->client->ps.persistant[PERS_TEAM], score);
	}

	CalculateRanks();
}

/*
=======================================================================================================================================
TossClientItems

Toss the weapon and powerups for the killed player.
=======================================================================================================================================
*/
void TossClientItems(gentity_t *self) {
	gitem_t *item;
	int weapon;
	float angle;
	int i;
	gentity_t *drop;

	// drop the weapon if not a gauntlet or machine gun
	weapon = self->s.weapon;
	// make a special check to see if they are changing to a new weapon that isn't the mg or gauntlet. Without this, a client can pick
	// up a weapon, be killed, and not drop the weapon because their weapon change hasn't completed yet and they are still holding the MG.
	if (weapon == WP_MACHINEGUN || weapon == WP_GAUNTLET) { // Tobias CHECK: wtf?
		if (self->client->ps.weaponstate == WEAPON_DROPPING) {
			weapon = self->client->pers.cmd.weapon;
		}

		if (!(self->client->ps.stats[STAT_WEAPONS] & (1 << weapon))) {
			weapon = WP_NONE;
		}
	}

	if (weapon > WP_MACHINEGUN && self->client->ps.ammo[weapon]) { // Tobias NOTE: be careful here!
		// find the item type for this weapon
		item = BG_FindItemForWeapon(weapon);
		// spawn the item
		Drop_Item(self, item, 0);
	}
	// drop all the powerups if dead
	for (i = 1; i < PW_NUM_POWERUPS; i++) {
		if (self->client->ps.powerups[i] > level.time) {
			item = BG_FindItemForPowerup(i);

			if (!item) {
				continue;
			}

			angle = 45;
			drop = Drop_Item(self, item, angle);
			// decide how many seconds it has left
			drop->count = (self->client->ps.powerups[i] - level.time) / 1000;

			if (drop->count < 1) {
				drop->count = 1;
			}

			angle += 45;
		}
	}
}

/*
=======================================================================================================================================
TossClientCubes

Spawn cube at neutral obelisk.
=======================================================================================================================================
*/
extern gentity_t *neutralObelisk;

void TossClientCubes(gentity_t *self) {
	gitem_t *item;
	gentity_t *drop;
	vec3_t velocity;
	vec3_t angles;
	vec3_t origin;

	self->client->ps.tokens = 0;
	// this should never happen but we should never get the server to crash due to skull being spawned in
	if (!G_EntitiesFree()) {
		return;
	}

	if (self->client->sess.sessionTeam == TEAM_RED) {
		item = BG_FindItem("Red Cube");
	} else {
		item = BG_FindItem("Blue Cube");
	}

	angles[YAW] = (float)(level.time % 360);
	angles[PITCH] = 0; // always forward
	angles[ROLL] = 0;

	AngleVectorsForward(angles, velocity);
	VectorScale(velocity, 150, velocity);

	velocity[2] += 200 + crandom() * 50;

	if (neutralObelisk) {
		VectorCopy(neutralObelisk->s.pos.trBase, origin);
		origin[2] += 44;
	} else {
		VectorClear(origin);
	}

	drop = Launch_Item(item, origin, velocity);
	drop->nextthink = level.time + g_cubeTimeout.integer * 1000;
	drop->think = G_FreeEntity;
	drop->s.team = self->client->sess.sessionTeam;
}

/*
=======================================================================================================================================
TossClientPersistantPowerups
=======================================================================================================================================
*/
void TossClientPersistantPowerups(gentity_t *ent) {
	gentity_t *powerup;

	if (!ent->client) {
		return;
	}

	if (!ent->client->persistantPowerup) {
		return;
	}

	powerup = ent->client->persistantPowerup;
	powerup->r.svFlags &= ~SVF_NOCLIENT;
	powerup->s.eFlags &= ~EF_NODRAW;
	powerup->r.contents = CONTENTS_TRIGGER;

	trap_LinkEntity(powerup);

	ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
	ent->client->persistantPowerup = NULL;
}

/*
=======================================================================================================================================
LookAtKiller
=======================================================================================================================================
*/
void LookAtKiller(gentity_t *self, gentity_t *inflictor, gentity_t *attacker) {
	vec3_t dir;

	if (attacker && attacker != self) {
		VectorSubtract(attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if (inflictor && inflictor != self) {
		VectorSubtract(inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = VectorToYaw(dir);
}

/*
=======================================================================================================================================
GibEntity
=======================================================================================================================================
*/
void GibEntity(gentity_t *self, int killer) {
	gentity_t *ent;
	int i;

	// if this entity still has kamikaze
	if (self->s.eFlags & EF_KAMIKAZE) {
		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < level.num_entities; i++) {
			ent = &g_entities[i];

			if (!ent->inuse) {
				continue;
			}

			if (ent->activator != self) {
				continue;
			}

			if (strcmp(ent->classname, "kamikaze timer")) {
				continue;
			}

			G_FreeEntity(ent);
			break;
		}
	}

	G_AddEvent(self, EV_GIB_PLAYER, killer);

	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
=======================================================================================================================================
BodyDie
=======================================================================================================================================
*/
void BodyDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath) {

	if (self->health > GIB_HEALTH) {
		return;
	}

	if (!g_blood.integer) {
		self->health = GIB_HEALTH + 1;
		return;
	}

	GibEntity(self, 0);
}

// these are just for logging, the client prints its own messages
char *modNames[] = {
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_CHAINGUN",
	"MOD_SHOTGUN",
	"MOD_NAIL",
	"MOD_PROXIMITY_MINE",
	"MOD_GRENADE_SPLASH",
	"MOD_NAPALM",
	"MOD_NAPALM_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_BEAMGUN",
	"MOD_RAILGUN",
	"MOD_PLASMA",
	"MOD_PLASMA_SPLASH",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_KAMIKAZE",
	"MOD_TELEFRAG",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_FALLING",
	"MOD_TRIGGER_HURT",
	"MOD_CRUSH",
	"MOD_TARGET_LASER",
	"MOD_SUICIDE",
	"MOD_SUICIDE_TEAM_CHANGE",
	"MOD_UNKNOWN"
};

/*
=======================================================================================================================================
Kamikaze_DeathActivate
=======================================================================================================================================
*/
void Kamikaze_DeathActivate(gentity_t *ent) {

	G_StartKamikaze(ent);
	G_FreeEntity(ent);
}

/*
=======================================================================================================================================
Kamikaze_DeathTimer
=======================================================================================================================================
*/
void Kamikaze_DeathTimer(gentity_t *self) {
	gentity_t *ent;

	ent = G_Spawn();
	ent->classname = "kamikaze timer";

	VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);

	ent->r.svFlags |= SVF_NOCLIENT;
	ent->think = Kamikaze_DeathActivate;
	ent->nextthink = level.time + 5 * 1000;
	ent->activator = self;
}

/*
=======================================================================================================================================
CheckAlmostCapture
=======================================================================================================================================
*/
void CheckAlmostCapture(gentity_t *self, gentity_t *attacker) {
	gentity_t *ent;
	vec3_t dir;
	char *classname;

	// if this player was carrying a flag
	if (self->client->ps.powerups[PW_REDFLAG] || self->client->ps.powerups[PW_BLUEFLAG] || self->client->ps.powerups[PW_NEUTRALFLAG]) {
		// get the goal flag this player should have been going for
		if (g_gametype.integer == GT_CTF) {
			if (self->client->sess.sessionTeam == TEAM_BLUE) {
				classname = "team_CTF_blueflag";
			} else {
				classname = "team_CTF_redflag";
			}
		} else {
			if (self->client->sess.sessionTeam == TEAM_BLUE) {
				classname = "team_CTF_redflag";
			} else {
				classname = "team_CTF_blueflag";
			}
		}

		ent = NULL;

		do {
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT)) {
			// if the player was *very* close
			VectorSubtract(self->client->ps.origin, ent->s.origin, dir);

			if (VectorLength(dir) < 200) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;

				if (attacker->client) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
=======================================================================================================================================
CheckAlmostScored
=======================================================================================================================================
*/
void CheckAlmostScored(gentity_t *self, gentity_t *attacker) {
	gentity_t *ent;
	vec3_t dir;
	char *classname;

	// if the player was carrying cubes
	if (self->client->ps.tokens) {
		if (self->client->sess.sessionTeam == TEAM_BLUE) {
			classname = "team_redobelisk";
		} else {
			classname = "team_blueobelisk";
		}

		ent = G_Find(NULL, FOFS(classname), classname);
		// if we found the destination obelisk
		if (ent) {
			// if the player was *very* close
			VectorSubtract(self->client->ps.origin, ent->s.origin, dir);

			if (VectorLength(dir) < 200) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;

				if (attacker->client) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
=======================================================================================================================================
PlayerDie
=======================================================================================================================================
*/
void PlayerDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath) {
	gentity_t *ent;
	int anim;
	int contents;
	int killer;
	int i;
	char *killerName, *obit;

	if (self->client->ps.pm_type == PM_DEAD) {
		return;
	}

	if (level.intermissiontime) {
		return;
	}
	// check for an almost capture
	CheckAlmostCapture(self, attacker);
	// check for a player that almost brought in cubes
	CheckAlmostScored(self, attacker);

	if ((self->client->ps.eFlags & EF_TICKING) && self->activator) {
		self->client->ps.eFlags &= ~EF_TICKING;
		self->activator->think = G_FreeEntity;
		self->activator->nextthink = level.time;
	}

	self->client->ps.pm_type = PM_DEAD;

	if (attacker) {
		killer = attacker->s.number;

		if (attacker->client) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if (killer < 0 || killer >= MAX_CLIENTS) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if (meansOfDeath < 0 || meansOfDeath >= ARRAY_LEN(modNames)) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[meansOfDeath];
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", killer, self->s.number, meansOfDeath, killerName, self->client->pers.netname, obit);
	// don't send death obituary when swiching teams
	if (meansOfDeath != MOD_SUICIDE_TEAM_CHANGE) {
		// broadcast the death event to everyone
		ent = G_TempEntity(self->r.currentOrigin, EV_OBITUARY);
		ent->s.eventParm = meansOfDeath;
		ent->s.otherEntityNum = self->s.number;
		ent->s.otherEntityNum2 = killer;
		ent->r.svFlags = SVF_BROADCAST; // send to everyone
	}

	self->enemy = attacker;
	self->client->ps.persistant[PERS_KILLED]++;

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if (attacker == self || OnSameTeam(self, attacker)) {
			AddScore(attacker, self->r.currentOrigin, -1);
		} else {
			AddScore(attacker, self->r.currentOrigin, 1);

			if (meansOfDeath == MOD_GAUNTLET) {
				// play humiliation on player
				attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
				attacker->client->rewardTime = level.time + REWARD_TIME;
				// also play humiliation on target
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}
			// check for two kills in a short amount of time, if this is close enough to the last kill, give a reward sound
			if (level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME) {
				// play excellent on player
				attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
				attacker->client->rewardTime = level.time + REWARD_TIME;
			}

			attacker->client->lastKillTime = level.time;
		}
	} else {
		AddScore(self, self->r.currentOrigin, -1);
	}
	// add team bonuses
	Team_FragBonuses(self, inflictor, attacker);
	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE || meansOfDeath == MOD_SUICIDE_TEAM_CHANGE) {
		if (self->client->ps.powerups[PW_REDFLAG]) { // only happens in standard CTF
			Team_ReturnFlag(TEAM_RED);
			self->client->ps.powerups[PW_REDFLAG] = 0;
		} else if (self->client->ps.powerups[PW_BLUEFLAG]) { // only happens in standard CTF
			Team_ReturnFlag(TEAM_BLUE);
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		} else if (self->client->ps.powerups[PW_NEUTRALFLAG]) { // only happens in One Flag CTF
			Team_ReturnFlag(TEAM_FREE);
			self->client->ps.powerups[PW_NEUTRALFLAG] = 0;
		}
	}

	TossClientItems(self);
	TossClientPersistantPowerups(self);

	if (g_gametype.integer == GT_HARVESTER) {
		TossClientCubes(self);
	}

	Cmd_Score_f(self); // show scores
	// send updated scores to any clients that are following this one, or they would get stale scoreboards
	for (i = 0; i < level.maxclients; i++) {
		gclient_t *client;

		client = &level.clients[i];

		if (client->pers.connected != CON_CONNECTED) {
			continue;
		}

		if (client->sess.sessionTeam != TEAM_SPECTATOR) {
			continue;
		}

		if (client->sess.spectatorClient == self->s.number) {
			Cmd_Score_f(g_entities + i);
		}
	}

	self->takedamage = qtrue; // can still be gibbed
	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;
	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	LookAtKiller(self, inflictor, attacker);
	VectorCopy(self->s.angles, self->client->ps.viewangles);

	self->s.loopSound = 0;
	self->r.maxs[2] = -8;
	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 2000;
	// remove powerups
	memset(self->client->ps.powerups, 0, sizeof(self->client->ps.powerups));
	// never gib in a nodrop
	contents = trap_PointContents(self->r.currentOrigin, -1);

	if ((self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity(self, killer);
	} else {
		// normal death
		static int i;

		switch (i) {
			case 0:
				anim = BOTH_DEATH1;
				break;
			case 1:
				anim = BOTH_DEATH2;
				break;
			case 2:
			default:
				anim = BOTH_DEATH3;
				break;
		}
		// for the no-blood option, we need to prevent the health from going to gib level
		if (self->health <= GIB_HEALTH) {
			self->health = GIB_HEALTH + 1;
		}

		self->client->ps.legsAnim = ((self->client->ps.legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)|anim;
		self->client->ps.torsoAnim = ((self->client->ps.torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)|anim;

		G_AddEvent(self, EV_DEATH1 + i, killer);
		// the body can still be gibbed
		self->die = BodyDie;
		// globally cycle through the different death animations
		i = (i + 1) % 3;

		if (self->s.eFlags & EF_KAMIKAZE) {
			Kamikaze_DeathTimer(self);
		}
	}

	trap_LinkEntity(self);
}

/*
=======================================================================================================================================
CheckArmor
=======================================================================================================================================
*/
int CheckArmor(gentity_t *ent, int damage, int dflags) {
	gclient_t *client;
	int save;
	int count;

	if (!damage) {
		return 0;
	}

	client = ent->client;

	if (!client) {
		return 0;
	}

	if (dflags & DAMAGE_NO_ARMOR) {
		return 0;
	}
	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil(damage * ARMOR_PROTECTION);

	if (save >= count) {
		save = count;
	}

	if (!save) {
		return 0;
	}

	client->ps.stats[STAT_ARMOR] -= save;
	return save;
}

/*
=======================================================================================================================================
G_Damage

targ:		Entity that is being damaged.
inflictor:	Entity that is causing the damage.
attacker:	Entity that caused the inflictor to damage targ.

example: targ = monster, inflictor = rocket, attacker = player.

dir	:		Direction of the attack for knockback.
point:		Point at which the damage is being inflicted, used for headshots.
damage:		Amount of damage being inflicted.
knockback:	Force to be applied against targ as a result of the damage.

inflictor, attacker, dir, and point can be NULL for environmental effects.

dflags		These flags are used to control how G_Damage works.

 DAMAGE_RADIUS			Damage was indirect (from a nearby explosion).
 DAMAGE_NO_ARMOR		Armor does not protect from this damage.
 DAMAGE_NO_KNOCKBACK	Do not affect velocity, just view angles.
 DAMAGE_NO_PROTECTION	Kills godmode, armor, everything.
=======================================================================================================================================
*/
void G_Damage(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod) {
	gclient_t *client;
	int take;
	int asave;
	int knockback;

	if (!targ->takedamage) {
		return;
	}
	// the intermission has already been qualified for, so don't allow any extra scoring
	if (level.intermissionQueued) {
		return;
	}

	if (!inflictor) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}

	if (!attacker) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}
	// shootable doors / buttons don't actually have any health
	if (targ->s.eType == ET_MOVER) {
		if (targ->use && targ->moverState == MOVER_POS1) {
			targ->use(targ, inflictor, attacker);
		}

		return;
	}

	if (g_gametype.integer == GT_OBELISK && CheckObeliskAttack(targ, attacker)) {
		return;
	}

	client = targ->client;

	if (client) {
		if (client->noclip) {
			return;
		}
	}

	if (!dir) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;

	if (knockback > 200) {
		knockback = 200;
	}

	if (targ->flags & FL_NO_KNOCKBACK) {
		knockback = 0;
	}

	if (dflags & DAMAGE_NO_KNOCKBACK) {
		knockback = 0;
	}
	// figure momentum add, even if the damage won't be taken
	if (knockback && targ->client) {
		vec3_t kvel;
		float mass;

		mass = 200;

		VectorScale(dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd(targ->client->ps.velocity, kvel, targ->client->ps.velocity);
		// set the timer so that the other client can't cancel out the movement immediately
		if (!targ->client->ps.pm_time) {
			int t;

			t = knockback * 2;

			if (t < 50) {
				t = 50;
			}

			if (t > 200) {
				t = 200;
			}

			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}
	// check for completely getting out of the damage
	if (!(dflags & DAMAGE_NO_PROTECTION)) {
		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target if the attacker was on the same team
		if (targ != attacker && !(dflags & DAMAGE_NO_TEAM_PROTECTION) && OnSameTeam(targ, attacker)) {
			if (!g_friendlyFire.integer) {
				return;
			}
		}

		if (mod == MOD_PROXIMITY_MINE) {
			if (inflictor && inflictor->parent && OnSameTeam(targ, inflictor->parent)) {
				if (!g_friendlyFire.integer) {
					return;
				}
			}
		}
		// check for godmode
		if (targ->flags & FL_GODMODE) {
			return;
		}
	}
	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if (attacker->client && client && targ != attacker && targ->health > 0 && targ->s.eType != ET_MISSILE && targ->s.eType != ET_GENERAL) {
		if (OnSameTeam(targ, attacker)) {
			attacker->client->ps.persistant[PERS_HITS]--;
		} else {
			attacker->client->ps.persistant[PERS_HITS]++;
		}

		attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health << 8)|(client->ps.stats[STAT_ARMOR]);
	}

	if (damage < 1) {
		damage = 1;
	}

	take = damage;
	// save some from armor
	asave = CheckArmor(targ, take, dflags);
	take -= asave;

	if (g_debugDamage.integer) {
		G_Printf("%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number, targ->health, take, asave);
	}
	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks at the end of the frame
	if (client) {
		client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;

		if (dir) {
			VectorCopy(dir, client->damage_from);
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy(targ->r.currentOrigin, client->damage_from);
			client->damage_fromWorld = qtrue;
		}
	}
	// see if it's the player hurting the emeny flag carrier
	if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF) {
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_time = level.time;
		targ->client->lasthurt_mod = mod;
	}
	// do the damage
	if (take) {
		targ->health = targ->health - take;

		if (targ->client) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

		if (targ->health <= 0) {
			if (client) {
				targ->flags |= FL_NO_KNOCKBACK;
			}

			if (targ->health < -999) {
				targ->health = -999;
			}

			targ->enemy = attacker;
			targ->die(targ, inflictor, attacker, take, mod);
			return;
		} else if (targ->pain) {
			targ->pain(targ, attacker, take);
		}
	}
}

/*
=======================================================================================================================================
CanDamage

Returns qtrue if the inflictor can directly damage the target. Used for explosions and melee attacks.
=======================================================================================================================================
*/
qboolean CanDamage(gentity_t *targ, vec3_t origin) {
	vec3_t dest;
	trace_t tr;
	vec3_t midpoint;
	vec3_t offsetmins = {-15, -15, -15};
	vec3_t offsetmaxs = {15, 15, 15};

	// use the midpoint of the bounds instead of the origin, because bmodels may have their origin is 0, 0, 0
	VectorAdd(targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale(midpoint, 0.5, midpoint);
	VectorCopy(midpoint, dest);
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number) {
		return qtrue;
	}
	// this should probably check in the plane of projection, rather than in world coordinate
	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
G_RadiusDamage
=======================================================================================================================================
*/
qboolean G_RadiusDamage(vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod) {
	float points, dist;
	gentity_t *ent;
	int entityList[MAX_GENTITIES];
	int numListedEntities;
	vec3_t mins, maxs;
	vec3_t v;
	vec3_t dir;
	int i, e;
	qboolean hitClient = qfalse;

	if (radius < 1) {
		radius = 1;
	}

	for (i = 0; i < 3; i++) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for (e = 0; e < numListedEntities; e++) {
		ent = &g_entities[entityList[e]];

		if (ent == ignore) {
			continue;
		}

		if (!ent->takedamage) {
			continue;
		}
		// find the distance from the edge of the bounding box
		for (i = 0; i < 3; i++) {
			if (origin[i] < ent->r.absmin[i]) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if (origin[i] > ent->r.absmax[i]) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength(v);

		if (dist >= radius) {
			continue;
		}

		points = damage * (1.0 - dist / radius);

		if (CanDamage(ent, origin)) {
			if (LogAccuracyHit(ent, attacker)) {
				hitClient = qtrue;
			}

			VectorSubtract(ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players get knocked into the air more
			dir[2] += 24;

			G_Damage(ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}
