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
 Both games misc functions, all completely stateless.
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "bg_public.h"

int numSplinePaths;
splinePath_t splinePaths[MAX_SPLINE_PATHS];

int numPathCorners;
pathCorner_t pathCorners[MAX_PATH_CORNERS];

/*QUAKED item_***** (0 0 0) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up. If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait" override the default wait before respawning. -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

/**************************************************************************************************************************************
	LEAVE INDEX 0 ALONE!
**************************************************************************************************************************************/

gitem_t bg_itemlist[] = {
	{
		NULL,
		NULL,
		{NULL, NULL, NULL, NULL},
/* icon */		NULL,
/* pickup */	NULL,
		0,
		0,
		0,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	HEALTH
**************************************************************************************************************************************/

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_small",
		"snd/i/puhs.wav",
		{"models/powerups/health/small_cross.md3", "models/powerups/health/small_sphere.md3", NULL, NULL},
/* icon */		"icons/iconh_green",
/* pickup */	"5 Health",
		5,
		IT_HEALTH,
		0,
/* sounds */ ""
	},

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health",
		"snd/i/puhn.wav",
		{"models/powerups/health/medium_cross.md3", "models/powerups/health/medium_sphere.md3", NULL, NULL},
/* icon */		"icons/iconh_yellow",
/* pickup */	"25 Health",
		25,
		IT_HEALTH,
		0,
/* sounds */ ""
	},

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_large",
		"snd/i/puhl.wav",
		{"models/powerups/health/large_cross.md3", "models/powerups/health/large_sphere.md3", NULL, NULL},
/* icon */		"icons/iconh_red",
/* pickup */	"50 Health",
		50,
		IT_HEALTH,
		0,
/* sounds */ ""
	},

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_mega",
		"snd/i/puhm.wav",
		{"models/powerups/health/mega_cross.md3", "models/powerups/health/mega_sphere.md3", NULL, NULL},
/* icon */		"icons/iconh_mega",
/* pickup */	"Mega Health",
		100,
		IT_HEALTH,
		0,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	ARMOR
**************************************************************************************************************************************/

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_shard",
		"snd/i/puag.wav",
		{"models/powerups/armor/shard.md3", NULL, NULL, NULL},
/* icon */		"icons/iconr_shard",
/* pickup */	"Armor Shard",
		5,
		IT_ARMOR,
		0,
/* sounds */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_combat",
		"snd/i/puay.wav",
		{"models/powerups/armor/armor_yel.md3", NULL, NULL, NULL},
/* icon */		"icons/iconr_yellow",
/* pickup */	"Armor",
		50,
		IT_ARMOR,
		0,
/* sounds */ ""
	},

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_body",
		"snd/i/puar.wav",
		{"models/powerups/armor/armor_red.md3", NULL, NULL, NULL},
/* icon */		"icons/iconr_red",
/* pickup */	"Heavy Armor",
		100,
		IT_ARMOR,
		0,
/* sounds */ ""
	},

/*QUAKED item_armor_full (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_full",
		"sound/misc/ar4_pkup.wav",
		{"models/powerups/armor/armor_blue.md3", NULL, NULL, NULL},
/* icon */		"icons/iconr_blue",
/* pickup */	"Full Armor",
		200,
		IT_ARMOR,
		0,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	WEAPONS
**************************************************************************************************************************************/

/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_gauntlet",
		"snd/i/puw.wav",
		{"models/weapons2/gauntlet/gauntlet.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_gauntlet",
/* pickup */	"Gauntlet",
		0,
		IT_WEAPON,
		WP_GAUNTLET,
/* sounds */ ""
	},

/*QUAKED weapon_handgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_handgun",
		"snd/i/puw.wav",
		{"models/weapons2/handgun/handgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_handgun",
/* pickup */	"Trusty .44",
		50,
		IT_WEAPON,
		WP_HANDGUN,
/* sounds */ ""
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_machinegun",
		"snd/i/puw.wav",
		{"models/weapons2/machinegun/machinegun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_machinegun",
/* pickup */	"Machine Gun",
		40,
		IT_WEAPON,
		WP_MACHINEGUN,
/* sounds */ ""
	},

/*QUAKED weapon_heavy_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_heavy_machinegun",
		"snd/i/puw.wav",
		{"models/weapons2/heavy_machinegun/heavymgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_hmgun",
/* pickup */	"Heavy Machine Gun",
		100,
		IT_WEAPON,
		WP_HEAVY_MACHINEGUN,
/* sounds */ "sound/weapons/hmg/hmgwind.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_chaingun",
		"snd/i/puw.wav",
		{"models/weapons/vulcan/vulcan.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_chaingun",
/* pickup */	"Chain Gun",
		80,
		IT_WEAPON,
		WP_CHAINGUN,
/* sounds */ "sound/weapons/vulcan/wvulwind.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_shotgun",
		"snd/i/puw.wav",
		{"models/weapons2/shotgun/shotgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_shotgun",
/* pickup */	"Shotgun",
		10,
		IT_WEAPON,
		WP_SHOTGUN,
/* sounds */ ""
	},

/*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_nailgun",
		"snd/i/puw.wav",
		{"models/weapons/nailgun/nailgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_nailgun",
/* pickup */	"Nail Gun",
		10,
		IT_WEAPON,
		WP_NAILGUN,
/* sounds */ ""
	},

/*QUAKED weapon_phosphorgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_phosphorgun",
		"snd/i/puw.wav",
		{"models/weapons/phosphorgun/phosphorgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_phosphorgun",
/* pickup */	"Phosphor Gun",
		50,
		IT_WEAPON,
		WP_PHOSPHORGUN,
/* sounds */ ""
	},

/*QUAKED weapon_prox_launcher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_prox_launcher",
		"snd/i/puw.wav",
		{"models/weapons/proxmine/proxmine.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_proxlauncher",
/* pickup */	"Proximity Launcher",
		5,
		IT_WEAPON,
		WP_PROXLAUNCHER,
/* sounds */ "sound/weapons/proxmine/wstbtick.wav sound/weapons/proxmine/wstbactv.wav sound/weapons/proxmine/wstbimpd.wav sound/weapons/proxmine/wstbactv.wav"
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_grenadelauncher",
		"snd/i/puw.wav",
		{"models/weapons2/grenadel/grenadel.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_grenade",
/* pickup */	"Grenade Launcher",
		10,
		IT_WEAPON,
		WP_GRENADELAUNCHER,
/* sounds */ "sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav"
	},

/*QUAKED weapon_napalmlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_napalmlauncher",
		"snd/i/puw.wav",
		{"models/weapons2/napalml/napalml.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_napalm",
/* pickup */	"Napalm Launcher",
		10,
		IT_WEAPON,
		WP_NAPALMLAUNCHER,
/* sounds */ ""
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_rocketlauncher",
		"snd/i/puw.wav",
		{"models/weapons2/rocketl/rocketl.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_rocket",
/* pickup */	"Rocket Launcher",
		10,
		IT_WEAPON,
		WP_ROCKETLAUNCHER,
/* sounds */ ""
	},

/*QUAKED weapon_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_lightning",
		"snd/i/puw.wav",
		{"models/weapons2/lightning/lightning.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_lightning",
/* pickup */	"Lightning Gun",
		100,
		IT_WEAPON,
		WP_LIGHTNING,
/* sounds */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_railgun",
		"snd/i/puw.wav",
		{"models/weapons2/railgun/railgun.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_railgun",
/* pickup */	"Railgun",
		10,
		IT_WEAPON,
		WP_RAILGUN,
/* sounds */ ""
	},

/*QUAKED weapon_plasmagun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_plasmagun",
		"snd/i/puw.wav",
		{"models/weapons2/plasma/plasma.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_plasma",
/* pickup */	"Plasma Gun",
		50,
		IT_WEAPON,
		WP_PLASMAGUN,
/* sounds */ ""
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_bfg",
		"snd/i/puw.wav",
		{"models/weapons2/bfg/bfg.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_bfg",
/* pickup */	"BFG 10K",
		20,
		IT_WEAPON,
		WP_BFG,
/* sounds */ ""
	},

/*QUAKED weapon_missilelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_missilelauncher",
		"snd/i/puw.wav",
		{"models/weapons2/missile/missilel.md3", NULL, NULL, NULL},
/* icon */		"icons/iconw_missile",
/* pickup */	"Missile Launcher",
		5,
		IT_WEAPON,
		WP_MISSILELAUNCHER,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	AMMO
**************************************************************************************************************************************/

/*QUAKED ammo_clip (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_clip",
		"snd/i/pum.wav",
		{"models/powerups/ammo/handgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_handgun",
/* pickup */	"Trusty .44 Ammo",
		50,
		IT_AMMO,
		WP_HANDGUN,
/* sounds */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_bullets",
		"snd/i/pum.wav",
		{"models/powerups/ammo/machinegunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_machinegun",
/* pickup */	"Bullets",
		40,
		IT_AMMO,
		WP_MACHINEGUN,
/* sounds */ ""
	},

/*QUAKED ammo_hmg_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_hmg_bullets",
		"snd/i/pum.wav",
		{"models/powerups/ammo/hmgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_hmgun",
/* pickup */	"HMG Bullets",
		80,
		IT_AMMO,
		WP_HEAVY_MACHINEGUN,
/* sounds */ ""
	},

/*QUAKED ammo_belt (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_belt",
		"snd/i/pum.wav",
		{"models/powerups/ammo/chaingunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_chaingun",
/* pickup */	"Chain Gun Belt",
		80,
		IT_AMMO,
		WP_CHAINGUN,
/* sounds */ ""
	},

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_shells",
		"snd/i/pum.wav",
		{"models/powerups/ammo/shotgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_shotgun",
/* pickup */	"Shells",
		10,
		IT_AMMO,
		WP_SHOTGUN,
/* sounds */ ""
	},

/*QUAKED ammo_nails (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_nails",
		"snd/i/pum.wav",
		{"models/powerups/ammo/nailgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_nailgun",
/* pickup */	"Nails",
		5,
		IT_AMMO,
		WP_NAILGUN,
/* sounds */ ""
	},

/*QUAKED ammo_capsules (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_capsules",
		"snd/i/pum.wav",
		{"models/powerups/ammo/phosphorgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_phosphorgun",
/* pickup */	"Capsules",
		50,
		IT_AMMO,
		WP_PHOSPHORGUN,
/* sounds */ ""
	},

/*QUAKED ammo_mines (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_mines",
		"snd/i/pum.wav",
		{"models/powerups/ammo/proxmineam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_proxlauncher",
/* pickup */	"Proximity Mines",
		5,
		IT_AMMO,
		WP_PROXLAUNCHER,
/* sounds */ ""
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_grenades",
		"snd/i/pum.wav",
		{"models/powerups/ammo/grenadeam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_grenade",
/* pickup */	"Grenades",
		5,
		IT_AMMO,
		WP_GRENADELAUNCHER,
/* sounds */ ""
	},

/*QUAKED ammo_canisters (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_canisters",
		"snd/i/pum.wav",
		{"models/powerups/ammo/napalmam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_napalm",
/* pickup */	"Canisters",
		5,
		IT_AMMO,
		WP_NAPALMLAUNCHER,
/* sounds */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_rockets",
		"snd/i/pum.wav",
		{"models/powerups/ammo/rocketam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_rocket",
/* pickup */	"Rockets",
		5,
		IT_AMMO,
		WP_ROCKETLAUNCHER,
/* sounds */ ""
	},

/*QUAKED ammo_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_lightning",
		"snd/i/pum.wav",
		{"models/powerups/ammo/lightningam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_lightning",
/* pickup */	"Lightning Ammo",
		50,
		IT_AMMO,
		WP_LIGHTNING,
/* sounds */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_slugs",
		"snd/i/pum.wav",
		{"models/powerups/ammo/railgunam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_railgun",
/* pickup */	"Slugs",
		10,
		IT_AMMO,
		WP_RAILGUN,
/* sounds */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_cells",
		"snd/i/pum.wav",
		{"models/powerups/ammo/plasmaam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_plasma",
/* pickup */	"Cells",
		30,
		IT_AMMO,
		WP_PLASMAGUN,
/* sounds */ ""
	},

/*QUAKED ammo_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_bfg",
		"snd/i/pum.wav",
		{"models/powerups/ammo/bfgam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_bfg",
/* pickup */	"Bfg Ammo",
		15,
		IT_AMMO,
		WP_BFG,
/* sounds */ ""
	},

/*QUAKED ammo_missiles (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_missiles",
		"snd/i/pum.wav",
		{"models/powerups/ammo/missileam.md3", NULL, NULL, NULL},
/* icon */		"icons/icona_missile",
/* pickup */	"Missiles",
		3,
		IT_AMMO,
		WP_MISSILELAUNCHER,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	HOLDABLE ITEMS
**************************************************************************************************************************************/

/*QUAKED holdable_kamikaze (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"holdable_kamikaze",
		"snd/i/puh.wav",
		{"models/powerups/kamikazi.md3", NULL, NULL, NULL},
/* icon */		"icons/kamikaze",
/* pickup */	"Kamikaze",
		60,
		IT_HOLDABLE,
		HI_KAMIKAZE,
/* sounds */ "snd/i/kam_sp.wav"
	},

/**************************************************************************************************************************************
	POWERUP ITEMS
**************************************************************************************************************************************/

/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_quad",
		"snd/v/voc_quad.wav",
		{"models/powerups/instant/quad.md3", "models/powerups/instant/quad_ring.md3", NULL, NULL},
/* icon */		"icons/quad",
/* pickup */	"Quad Damage",
		30,
		IT_POWERUP,
		PW_QUAD,
/* sounds */ "snd/i/q.wav"
	},

/*QUAKED item_invis (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_invis",
		"snd/v/voc_invis.wav",
		{"models/powerups/instant/invis.md3", "models/powerups/instant/invis_ring.md3", NULL, NULL},
/* icon */		"icons/invis",
/* pickup */	"Invisibility",
		30,
		IT_POWERUP,
		PW_INVIS,
/* sounds */ ""
	},

/*QUAKED item_regen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_regen",
		"snd/v/voc_regen.wav",
		{"models/powerups/instant/regen.md3", "models/powerups/instant/regen_ring.md3", NULL, NULL},
/* icon */		"icons/regen",
/* pickup */	"Regeneration",
		30,
		IT_POWERUP,
		PW_REGEN,
/* sounds */ "snd/i/r.wav"
	},

/**************************************************************************************************************************************
	PERSISTANT POWERUP ITEMS
**************************************************************************************************************************************/

/*QUAKED item_ammoregen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
		"item_ammoregen",
		"snd/i/pu.wav",
		{"models/powerups/ammo.md3", NULL, NULL, NULL},
/* icon */		"icons/ammo_regen",
/* pickup */	"Ammo Regen",
		30,
		IT_PERSISTANT_POWERUP,
		PW_AMMOREGEN,
/* sounds */ ""
	},

/*QUAKED item_guard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
		"item_guard",
		"snd/i/pu.wav",
		{"models/powerups/guard.md3", NULL, NULL, NULL},
/* icon */		"icons/guard",
/* pickup */	"Guard",
		30,
		IT_PERSISTANT_POWERUP,
		PW_GUARD,
/* sounds */ ""
	},

/*QUAKED item_doubler (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
		"item_doubler",
		"snd/i/pu.wav",
		{"models/powerups/doubler.md3", NULL, NULL, NULL},
/* icon */		"icons/doubler",
/* pickup */	"Doubler",
		30,
		IT_PERSISTANT_POWERUP,
		PW_DOUBLER,
/* sounds */ ""
	},

/*QUAKED item_scout (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
		"item_scout",
		"snd/i/pu.wav",
		{"models/powerups/scout.md3", NULL, NULL, NULL},
/* icon */		"icons/scout",
/* pickup */	"Scout",
		30,
		IT_PERSISTANT_POWERUP,
		PW_SCOUT,
/* sounds */ ""
	},

/**************************************************************************************************************************************
	TEAM ITEMS
**************************************************************************************************************************************/

/*QUAKED team_CTF_redflag (1 0 0) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
		"team_CTF_redflag",
		NULL,
		{"models/flags/r_flag.md3", NULL, NULL, NULL},
/* icon */		"icons/iconf_red1",
/* pickup */	"Red Flag",
		0,
		IT_TEAM,
		PW_REDFLAG,
/* sounds */ ""
	},

/*QUAKED team_CTF_blueflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
		"team_CTF_blueflag",
		NULL,
		{"models/flags/b_flag.md3", NULL, NULL, NULL},
/* icon */		"icons/iconf_blu1",
/* pickup */	"Blue Flag",
		0,
		IT_TEAM,
		PW_BLUEFLAG,
/* sounds */ ""
	},

/*QUAKED team_CTF_neutralflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in One Flag CTF games
*/
	{
		"team_CTF_neutralflag",
		NULL,
		{"models/flags/n_flag.md3", NULL, NULL, NULL},
/* icon */		"icons/iconf_neutral1",
/* pickup */	"Neutral Flag",
		0,
		IT_TEAM,
		PW_NEUTRALFLAG,
/* sounds */ ""
	},

/*QUAKED item_redcube (0 0 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_redcube",
		"snd/i/pu.wav",
		{"models/powerups/orb/r_orb.md3", NULL, NULL, NULL},
/* icon */		"icons/iconh_rorb",
/* pickup */	"Red Cube",
		0,
		IT_TEAM,
		0,
/* sounds */ ""
	},

/*QUAKED item_bluecube (0 0 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bluecube",
		"snd/i/pu.wav",
		{"models/powerups/orb/b_orb.md3", NULL, NULL, NULL},
/* icon */		"icons/iconh_borb",
/* pickup */	"Blue Cube",
		0,
		IT_TEAM,
		0,
/* sounds */ ""
	},
	// end of list marker
	{NULL}
};

int bg_numItems = ARRAY_LEN(bg_itemlist) - 1;
// may not contain spaces, dpmaster will reject the server
const char *bg_netGametypeNames[GT_MAX_GAME_TYPE] = {
	"SP",
	"FFA",
	"Tournament",
	"TeamDM",
	"CTF",
	"1FCTF",
	"Overload",
	"Harvester"
};

const char *bg_displayGametypeNames[GT_MAX_GAME_TYPE] = {
	"Single Player",
	"Free For All",
	"Tournament",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
};

/*
=======================================================================================================================================
BG_CheckSpawnEntity
=======================================================================================================================================
*/
qboolean BG_CheckSpawnEntity(const bgEntitySpawnInfo_t *info) {
	int i, gametype;
	char *s, *value, *gametypeName;
	static char *gametypeNames[GT_MAX_GAME_TYPE] = {"single", "ffa", "tournament", "team", "ctf", "oneflag", "obelisk", "harvester"};

	gametype = info->gametype;
	// check for "notsingle" flag
	if (gametype == GT_SINGLE_PLAYER) {
		info->spawnInt("notsingle", "0", &i);

		if (i) {
			return qfalse;
		}
	}
	// check for "notteam" flag (GT_SINGLE_PLAYER, GT_FFA, GT_TOURNAMENT)
	if (gametype > GT_TOURNAMENT) {
		info->spawnInt("notteam", "0", &i);

		if (i) {
			return qfalse;
		}
	} else {
		info->spawnInt("notfree", "0", &i);

		if (i) {
			return qfalse;
		}
	}

	if (info->spawnString("!gametype", NULL, &value)) {
		if (gametype >= 0 && gametype < GT_MAX_GAME_TYPE) {
			gametypeName = gametypeNames[gametype];
			s = strstr(value, gametypeName);

			if (s) {
				return qfalse;
			}
		}
	}

	if (info->spawnString("gametype", NULL, &value)) {
		if (gametype >= 0 && gametype < GT_MAX_GAME_TYPE) {
			gametypeName = gametypeNames[gametype];
			s = strstr(value, gametypeName);

			if (!s) {
				return qfalse;
			}
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
BG_FindItemForPowerup
=======================================================================================================================================
*/
gitem_t *BG_FindItemForPowerup(powerup_t pw) {
	int i;

	for (i = 0; i < bg_numItems; i++) {
		if ((bg_itemlist[i].giType == IT_POWERUP || bg_itemlist[i].giType == IT_TEAM || bg_itemlist[i].giType == IT_PERSISTANT_POWERUP) && bg_itemlist[i].giTag == pw) {
			return &bg_itemlist[i];
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
BG_FindItemForHoldable
=======================================================================================================================================
*/
gitem_t *BG_FindItemForHoldable(holdable_t pw) {
	int i;

	for (i = 0; i < bg_numItems; i++) {
		if (bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw) {
			return &bg_itemlist[i];
		}
	}

	Com_Error(ERR_DROP, "HoldableItem not found");
	return NULL;
}

/*
=======================================================================================================================================
BG_FindItemForWeapon
=======================================================================================================================================
*/
gitem_t *BG_FindItemForWeapon(weapon_t weapon) {
	gitem_t *it;

	for (it = bg_itemlist + 1; it->classname; it++) {
		if (it->giType == IT_WEAPON && it->giTag == weapon) {
			return it;
		}
	}

	Com_Error(ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
=======================================================================================================================================
BG_FindItem
=======================================================================================================================================
*/
gitem_t *BG_FindItem(const char *pickupName) {
	gitem_t *it;

	for (it = bg_itemlist + 1; it->classname; it++) {
		if (!Q_stricmp(it->pickup_name, pickupName)) {
			return it;
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make grabbing them easier.
=======================================================================================================================================
*/
qboolean BG_PlayerTouchesItem(playerState_t *ps, entityState_t *item, int atTime) {
	vec3_t origin;

	BG_EvaluateTrajectory(&item->pos, atTime, origin, qfalse, item->effect2Time);
	// we are ignoring ducked differences here
	if (ps->origin[0] - origin[0] > 36 || ps->origin[0] - origin[0] < -36 || ps->origin[1] - origin[1] > 36 || ps->origin[1] - origin[1] < -36 || ps->origin[2] - origin[2] > 36 || ps->origin[2] - origin[2] < -36) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up. This needs to be the same for client side prediction and server use.
=======================================================================================================================================
*/
qboolean BG_CanItemBeGrabbed(int gametype, const entityState_t *ent, const playerState_t *ps) {
	gitem_t *item;
	int max;

	if (ent->modelindex < 1 || ent->modelindex >= bg_numItems) {
		Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: index out of range");
	}

	item = &bg_itemlist[ent->modelindex];

	switch (item->giType) {
		case IT_HEALTH:
			// don't pick up if already at max
			if (bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD) {
				max = ps->stats[STAT_MAX_HEALTH] / 2;
			} else {
				max = ps->stats[STAT_MAX_HEALTH];
			}

			if (ps->stats[STAT_HEALTH] >= max) {
				return qfalse;
			}

			return qtrue;
		case IT_ARMOR:
			// we also clamp armor to the maxhealth for handicapping
			if (bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD) {
				max = ps->stats[STAT_MAX_HEALTH];
			} else {
				max = ps->stats[STAT_MAX_HEALTH] * 2;
			}

			if (ps->stats[STAT_ARMOR] >= max) {
				return qfalse;
			}

			return qtrue;
		case IT_WEAPON:
			return qtrue; // weapons are always picked up
		case IT_AMMO:
			if (ps->ammo[item->giTag] >= 200) {
				return qfalse; // can't hold any more
			}

			return qtrue;
		case IT_HOLDABLE:
			// can only hold one item at a time
			if (ps->stats[STAT_HOLDABLE_ITEM]) {
				return qfalse;
			}

			return qtrue;
		case IT_POWERUP:
			return qtrue; // powerups are always picked up
		case IT_PERSISTANT_POWERUP:
			// can only hold one item at a time
			if (ps->stats[STAT_PERSISTANT_POWERUP]) {
				return qfalse;
			}
			// check team only
			if (ent->team != 255 && (ps->persistant[PERS_TEAM] != ent->team)) {
				return qfalse;
			}

			return qtrue;
		case IT_TEAM: // team items, such as flags
			if (gametype == GT_CTF) {
				// ent->modelindex2 is non-zero on items if they are dropped
				// we need to know this because we can pick up our dropped flag (and return it) but we can't pick up our flag at base
				if (ps->persistant[PERS_TEAM] == TEAM_RED) {
					if (item->giTag == PW_BLUEFLAG || (item->giTag == PW_REDFLAG && ent->modelindex2) || (item->giTag == PW_REDFLAG && ps->powerups[PW_BLUEFLAG])) {
						return qtrue;
					}
				} else if (ps->persistant[PERS_TEAM] == TEAM_BLUE) {
					if (item->giTag == PW_REDFLAG || (item->giTag == PW_BLUEFLAG && ent->modelindex2) || (item->giTag == PW_BLUEFLAG && ps->powerups[PW_REDFLAG])) {
						return qtrue;
					}
				}
			}

			if (gametype == GT_1FCTF) {
				// neutral flag can always be picked up
				if (item->giTag == PW_NEUTRALFLAG) {
					return qtrue;
				}

				if (ps->persistant[PERS_TEAM] == TEAM_RED) {
					if (item->giTag == PW_BLUEFLAG && ps->powerups[PW_NEUTRALFLAG]) {
						return qtrue;
					}
				} else if (ps->persistant[PERS_TEAM] == TEAM_BLUE) {
					if (item->giTag == PW_REDFLAG && ps->powerups[PW_NEUTRALFLAG]) {
						return qtrue;
					}
				}
			}

			if (gametype == GT_HARVESTER) {
				return qtrue;
			}

			return qfalse;
		case IT_BAD:
			Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD");
		default:
#ifndef Q3_VM
#ifndef NDEBUG
			Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType);
#endif
#endif
			break;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BG_CalculateSpline_r
=======================================================================================================================================
*/
void BG_CalculateSpline_r(splinePath_t *spline, vec3_t out1, vec3_t out2, float tension) {
	vec3_t points[18];
	int i;
	int count = spline->numControls + 2;
	vec3_t dist;

	VectorCopy(spline->point.origin, points[0]);

	for (i = 0; i < spline->numControls; i++) {
		VectorCopy(spline->controls[i].origin, points[i + 1]);
	}

	if (!spline->next) {
		return;
	}

	VectorCopy(spline->next->point.origin, points[i + 1]);

	while (count > 2) {
		for (i = 0; i < count - 1; i++) {
			VectorSubtract(points[i + 1], points[i], dist);
			VectorMA(points[i], tension, dist, points[i]);
		}

		count--;
	}

	VectorCopy(points[0], out1);
	VectorCopy(points[1], out2);
}

/*
=======================================================================================================================================
BG_TraverseSpline
=======================================================================================================================================
*/
qboolean BG_TraverseSpline(float *deltaTime, splinePath_t **pSpline) {
	float dist;

	while ((*deltaTime) > 1) {
		(*deltaTime) -= 1;
		dist = (*pSpline)->length * (*deltaTime);

		if (!(*pSpline)->next || (*pSpline)->next->length == 0.f) {
			return qfalse;
		}

		(*pSpline) = (*pSpline)->next;
		*deltaTime = dist / (*pSpline)->length;
	}

	while ((*deltaTime) < 0) {
		dist = -((*pSpline)->length * (*deltaTime));

		if (!(*pSpline)->prev || (*pSpline)->prev->length == 0.f) {
			return qfalse;
		}

		(*pSpline) = (*pSpline)->prev;
		(*deltaTime) = 1 - (dist / (*pSpline)->length);
	}

	return qtrue;
}

/*
=======================================================================================================================================
BG_RaySphereIntersection
=======================================================================================================================================
*/
qboolean BG_RaySphereIntersection(float radius, vec3_t origin, splineSegment_t *path, float *t0, float *t1) {
	vec3_t v;
	float b, c, d;

	VectorSubtract(path->start, origin, v);

	b = 2 * DotProduct(v, path->v_norm);
	c = DotProduct(v, v) - (radius * radius);
	d = (b * b) - (4 * c);

	if (d < 0) {
		return qfalse;
	}

	d = sqrt(d);

	*t0 = (-b + d) * 0.5f;
	*t1 = (-b - d) * 0.5f;

	return qtrue;
}

/*
=======================================================================================================================================
BG_LinearPathOrigin2
=======================================================================================================================================
*/
void BG_LinearPathOrigin2(float radius, splinePath_t **pSpline, float *deltaTime, vec3_t result, qboolean backwards) {
	qboolean first = qtrue;
	float t = 0.f;
	int i = floor((*deltaTime) * (MAX_SPLINE_SEGMENTS));
	float frac;

	if (i >= MAX_SPLINE_SEGMENTS) {
		i = MAX_SPLINE_SEGMENTS - 1;
		frac = 1.f;
	} else {
		frac = (((*deltaTime) * (MAX_SPLINE_SEGMENTS)) - i);
	}

	while (qtrue) {
		float t0, t1;

		while (qtrue) {
			if (BG_RaySphereIntersection(radius, result, &(*pSpline)->segments[i], &t0, &t1)) {
				qboolean found = qfalse;

				t0 /= (*pSpline)->segments[i].length;
				t1 /= (*pSpline)->segments[i].length;

				if (first) {
					if (radius < 0) {
						if (t0 < frac && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						} else if (t1 < frac) {
							t = t1;
							found = qtrue;
						}
					} else {
						if (t0 > frac && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						} else if (t1 > frac) {
							t = t1;
							found = qtrue;
						}
					}
				} else {
					if (radius < 0) {
						if (t0 < t1 && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						} else {
							t = t1;
							found = qtrue;
						}
					} else {
						if (t0 > t1 && (t0 >= 0.f && t0 <= 1.f)) {
							t = t0;
							found = qtrue;
						} else {
							t = t1;
							found = qtrue;
						}
					}
				}

				if (found) {
					if (t >= 0.f && t <= 1.f) {
						*deltaTime = (i / (float)(MAX_SPLINE_SEGMENTS)) + (t / (float)(MAX_SPLINE_SEGMENTS));
						VectorMA((*pSpline)->segments[i].start, t * (*pSpline)->segments[i].length, (*pSpline)->segments[i].v_norm, result);
						return;
					}
				}

				found = qfalse;
			}

			first = qfalse;

			if (radius < 0) {
				i--;

				if (i < 0) {
					i = MAX_SPLINE_SEGMENTS - 1;
					break;
				}
			} else {
				i++;

				if (i >= MAX_SPLINE_SEGMENTS) {
					i = 0;
					break;
				}
			}
		}

		if (radius < 0) {
			if (!(*pSpline)->prev) {
				return;
			}

			*pSpline = (*pSpline)->prev;
		} else {
			if (!(*pSpline)->next) {
				return;
			}

			*pSpline = (*pSpline)->next;
		}
	}
}

/*
=======================================================================================================================================
BG_ComputeSegments
=======================================================================================================================================
*/
void BG_ComputeSegments(splinePath_t *pSpline) {
	int i;
	float granularity = 1 / ((float)(MAX_SPLINE_SEGMENTS));
	vec3_t vec[4];

	for (i = 0; i < MAX_SPLINE_SEGMENTS; i++) {
		BG_CalculateSpline_r(pSpline, vec[0], vec[1], i * granularity);
		VectorSubtract(vec[1], vec[0], pSpline->segments[i].start);
		VectorMA(vec[0], i * granularity, pSpline->segments[i].start, pSpline->segments[i].start);

		BG_CalculateSpline_r(pSpline, vec[2], vec[3], (i + 1) * granularity);
		VectorSubtract(vec[3], vec[2], vec[0]);
		VectorMA(vec[2], (i + 1) * granularity, vec[0], vec[0]);

		VectorSubtract(vec[0], pSpline->segments[i].start, pSpline->segments[i].v_norm);
		pSpline->segments[i].length = VectorLength(pSpline->segments[i].v_norm);
		VectorNormalize(pSpline->segments[i].v_norm);
	}
}

/*
=======================================================================================================================================
BG_Find_Spline
=======================================================================================================================================
*/
splinePath_t *BG_Find_Spline(const char *match) {
	int i;

	for (i = 0; i < numSplinePaths; i++) {
		if (!Q_stricmp(splinePaths[i].point.name, match)) {
			return &splinePaths[i];
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
BG_AddSplinePath
=======================================================================================================================================
*/
splinePath_t *BG_AddSplinePath(const char *name, const char *target, vec3_t origin) {
	splinePath_t *spline;

	if (numSplinePaths >= MAX_SPLINE_PATHS) {
		Com_Error(ERR_DROP, "MAX SPLINES (%i) hit", MAX_SPLINE_PATHS);
	}

	spline = &splinePaths[numSplinePaths];

	memset(spline, 0, sizeof(splinePath_t));

	VectorCopy(origin, spline->point.origin);

	Q_strncpyz(spline->point.name, name, 64);
	Q_strncpyz(spline->strTarget, target ? target : "", 64);

	spline->numControls = 0;

	numSplinePaths++;
	return spline;
}

/*
=======================================================================================================================================
BG_AddSplineControl
=======================================================================================================================================
*/
void BG_AddSplineControl(splinePath_t *spline, const char *name) {

	if (spline->numControls >= MAX_SPLINE_CONTROLS) {
		Com_Error(ERR_DROP, "MAX SPLINE CONTROLS (%i) hit", MAX_SPLINE_CONTROLS);
	}

	Q_strncpyz(spline->controls[spline->numControls].name, name, 64);

	spline->numControls++;
}

/*
=======================================================================================================================================
BG_SplineLength
=======================================================================================================================================
*/
float BG_SplineLength(splinePath_t *pSpline) {
	float i;
	float granularity = 0.01f;
	float dist = 0.f;
	vec3_t vec[2];
	vec3_t lastPoint = {0};
	vec3_t result;

	for (i = 0.f; i <= 1.f; i += granularity) {
		BG_CalculateSpline_r(pSpline, vec[0], vec[1], i);

		VectorSubtract(vec[1], vec[0], result);
		VectorMA(vec[0], i, result, result);

		if (i != 0.f) {
			VectorSubtract(result, lastPoint, vec[0]);
			dist += VectorLength(vec[0]);
		}

		VectorCopy(result, lastPoint);
	}

	return dist;
}

/*
=======================================================================================================================================
BG_BuildSplinePaths
=======================================================================================================================================
*/
void BG_BuildSplinePaths(void) {
	int i, j;
	pathCorner_t *pnt;
	splinePath_t *spline, *st;

	for (i = 0; i < numSplinePaths; i++) {
		spline = &splinePaths[i];

		if (*spline->strTarget) {
			for (j = 0; j < spline->numControls; j++) {
				pnt = BG_Find_PathCorner(spline->controls[j].name);

				if (!pnt) {
					Com_Printf("^1Cant find control point (%s) for spline (%s)\n", spline->controls[j].name, spline->point.name);
					// Just changing to a warning for now, easier for region compiles...
					continue;
				} else {
					VectorCopy(pnt->origin, spline->controls[j].origin);
				}
			}

			st = BG_Find_Spline(spline->strTarget);

			if (!st) {
				Com_Printf("^1Cant find target point (%s) for spline (%s)\n", spline->strTarget, spline->point.name);
				// Just changing to a warning for now, easier for region compiles...
				continue;
			}

			spline->next = st;
			spline->length = BG_SplineLength(spline);

			BG_ComputeSegments(spline);
		}
	}

	for (i = 0; i < numSplinePaths; i++) {
		spline = &splinePaths[i];

		if (spline->next) {
			spline->next->prev = spline;
		}
	}
}

/*
=======================================================================================================================================
BG_Find_PathCorner
=======================================================================================================================================
*/
pathCorner_t *BG_Find_PathCorner(const char *match) {
	int i;

	for (i = 0; i < numPathCorners; i++) {
		if (!Q_stricmp(pathCorners[i].name, match)) {
			return &pathCorners[i];
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
BG_AddPathCorner
=======================================================================================================================================
*/
void BG_AddPathCorner(const char *name, vec3_t origin) {

	if (numPathCorners >= MAX_PATH_CORNERS) {
		Com_Error(ERR_DROP, "MAX PATH CORNERS (%i) hit", MAX_PATH_CORNERS);
	}

	VectorCopy(origin, pathCorners[numPathCorners].origin);

	Q_strncpyz(pathCorners[numPathCorners].name, name, 64);

	numPathCorners++;
}

/*
=======================================================================================================================================
BG_GetSplineData
=======================================================================================================================================
*/
splinePath_t *BG_GetSplineData(int number, qboolean *backwards) {

	if (number < 0) {
		*backwards = qtrue;
		number = -number;
	} else {
		*backwards = qfalse;
	}

	number--;

	if (number < 0 || number >= numSplinePaths) {
		return NULL;
	}

	return &splinePaths[number];
}

/*
=======================================================================================================================================
BG_EvaluateTrajectory
=======================================================================================================================================
*/
void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splinePath) {
	float deltaTime;
	float phase;
	vec3_t dir, v, vec[2];
	splinePath_t *pSpline;
	qboolean backwards;
	float deltaTime2, angle;
	int gravity;
	char var[128];

	trap_Cvar_VariableStringBuffer("g_gravity", var, sizeof(var));
	gravity = atoi(var);
	backwards = qfalse;

	switch (tr->trType) {
		case TR_STATIONARY:
		case TR_INTERPOLATE:
		case TR_GRAVITY_PAUSED:
			VectorCopy(tr->trBase, result);
			break;
		case TR_ROTATING:
			if (tr->trTime > 0) {
				deltaTime = tr->trTime * 0.001f; // milliseconds to seconds
			} else if (tr->trTime < 0) {
				deltaTime = (atTime + tr->trTime) * 0.001f;
			} else {
				deltaTime = (atTime - tr->trTime) * 0.001f;
			}

			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_LINEAR_BOB:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			phase = sin(atTime / 100.0f);
			result[2] += phase * 5.0f;
			break;
		case TR_LINEAR:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_LINEAR_STOP:
			if (atTime > tr->trTime + tr->trDuration) {
				atTime = tr->trTime + tr->trDuration;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds

			if (deltaTime < 0) {
				deltaTime = 0;
			}

			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_NONLINEAR_STOP:
			if (atTime > tr->trTime + tr->trDuration) {
				atTime = tr->trTime + tr->trDuration;
			}
			// new slow-down at end
			if (atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0) {
				deltaTime = 0;
			} else { // FIXME: maybe scale this somehow? So that it starts out faster and stops faster?
				deltaTime = tr->trDuration * 0.001f * ((float)cos(DEG2RAD(90.0f - (90.0f * ((float)atTime - tr->trTime) / (float)tr->trDuration))));
			}

			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_GRAVITY:
			if (atTime < tr->trTime) {
				atTime = tr->trTime;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5f * gravity * deltaTime * deltaTime;
			break;
		case TR_GRAVITY_FLOAT:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5f * (gravity * 0.2f) * deltaTime;
			break;
		case TR_GRAVITY_SMALL:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			VectorScale(result, 0.02f * deltaTime * deltaTime, dir);
			VectorSubtract(result, dir, result);
			result[2] -= 0.5f * gravity * deltaTime * deltaTime;
			break;
		case TR_GRAVITY_LOW:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5f * 0.5f * gravity * deltaTime * deltaTime;
			break;
		case TR_GRAVITY_WATER:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorNormalize2(tr->trDelta, dir);
			VectorMA(tr->trBase, Q_ln(0.02f * VectorLength(tr->trDelta) * deltaTime + 1) / 0.02f, dir, result);
			result[2] -= 30.0f * deltaTime;
			break;
		case TR_GRAVITY_LOCAL: //add local gravity, trDuration has been co-opted to hold "gravity"
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5f * tr->trDuration * deltaTime * deltaTime;
			break;
		case TR_BUOYANCY:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] += 0.5f * gravity * deltaTime * deltaTime;
			break;
		case TR_ACCELERATE: // trDelta is the ultimate speed
			if (atTime > tr->trTime + tr->trDuration) {
				atTime = tr->trTime + tr->trDuration;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			// phase is the acceleration constant
			phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001f);
			// trDelta at least gives us the acceleration direction
			VectorNormalize2(tr->trDelta, result);
			// get distance travelled at current time
			VectorMA(tr->trBase, phase * 0.5f * deltaTime * deltaTime, result, result);
			break;
		case TR_DECCELERATE: // trDelta is the starting speed
			if (atTime > tr->trTime + tr->trDuration) {
				atTime = tr->trTime + tr->trDuration;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			// phase is the breaking constant
			phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001f);
			// trDelta at least gives us the acceleration direction
			VectorNormalize2(tr->trDelta, result);
			// get distance travelled at current time (without breaking)
			VectorMA(tr->trBase, deltaTime, tr->trDelta, v);
			// subtract breaking force
			VectorMA(v, -phase * 0.5f * deltaTime * deltaTime, result, result);
			break;
		case TR_SPLINE:
			if (!(pSpline = BG_GetSplineData(splinePath, &backwards))) {
				return;
			}

			deltaTime = tr->trDuration ? (atTime - tr->trTime) / ((float)tr->trDuration) : 0;

			if (deltaTime < 0.f) {
				deltaTime = 0.f;
			} else if (deltaTime > 1.f) {
				deltaTime = 1.f;
			}

			if (backwards) {
				deltaTime = 1 - deltaTime;
			}

			deltaTime2 = deltaTime;

			BG_CalculateSpline_r(pSpline, vec[0], vec[1], deltaTime);

			if (isAngle) {
				qboolean dampin = qfalse;
				qboolean dampout = qfalse;
				float base1;

				if (tr->trBase[0] != 0.f) {
					vec3_t result2;
					splinePath_t *pSp2 = pSpline;

					deltaTime2 += tr->trBase[0] / pSpline->length;

					if (BG_TraverseSpline(&deltaTime2, &pSp2)) {
						VectorSubtract(vec[1], vec[0], result);
						VectorMA(vec[0], deltaTime, result, result);

						BG_CalculateSpline_r(pSp2, vec[0], vec[1], deltaTime2);

						VectorSubtract(vec[1], vec[0], result2);
						VectorMA(vec[0], deltaTime2, result2, result2);

						if (tr->trBase[0] < 0) {
							VectorSubtract(result, result2, result);
						} else {
							VectorSubtract(result2, result, result);
						}
					} else {
						VectorSubtract(vec[1], vec[0], result);
					}
				} else {
					VectorSubtract(vec[1], vec[0], result);
				}

				vectoangles(result, result);

				base1 = tr->trBase[1];
				if (base1 >= 10000 || base1 < -10000) {
					dampin = qtrue;
					if (base1 < 0) {
						base1 += 10000;
					} else {
						base1 -= 10000;
					}
				}

				if (base1 >= 1000 || base1 < -1000) {
					dampout = qtrue;
					if (base1 < 0) {
						base1 += 1000;
					} else {
						base1 -= 1000;
					}
				}

				if (dampin && dampout) {
					result[ROLL] = base1 + ((sin(((deltaTime * 2) - 1) * M_PI * 0.5f) + 1) * 0.5 * tr->trBase[2]);
				} else if (dampin) {
					result[ROLL] = base1 + (sin(deltaTime * M_PI * 0.5) * tr->trBase[2]);
				} else if (dampout) {
					result[ROLL] = base1 + ((1 - sin((1 - deltaTime) * M_PI * 0.5)) * tr->trBase[2]);
				} else {
					result[ROLL] = base1 + (deltaTime * tr->trBase[2]);
				}
			} else {
				VectorSubtract(vec[1], vec[0], result);
				VectorMA(vec[0], deltaTime, result, result);
			}

			break;
		case TR_LINEAR_PATH:
			if (!(pSpline = BG_GetSplineData(splinePath, &backwards))) {
				return;
			}

			deltaTime = tr->trDuration ? (atTime - tr->trTime) / ((float)tr->trDuration) : 0;

			if (deltaTime < 0.f) {
				deltaTime = 0.f;
			} else if (deltaTime > 1.f) {
				deltaTime = 1.f;
			}

			if (backwards) {
				deltaTime = 1 - deltaTime;
			}

			if (isAngle) {
				int pos = floor(deltaTime * (MAX_SPLINE_SEGMENTS));
				float frac;

				if (pos >= MAX_SPLINE_SEGMENTS) {
					pos = MAX_SPLINE_SEGMENTS - 1;
					frac = pSpline->segments[pos].length;
				} else {
					frac = ((deltaTime * (MAX_SPLINE_SEGMENTS)) - pos) * pSpline->segments[pos].length;
				}

				if (tr->trBase[0] != 0.f) {
					VectorMA(pSpline->segments[pos].start, frac, pSpline->segments[pos].v_norm, result);
					VectorCopy(result, v);

					BG_LinearPathOrigin2(tr->trBase[0], &pSpline, &deltaTime, v, backwards);
					if (tr->trBase[0] < 0) {
						VectorSubtract(v, result, result);
					} else {
						VectorSubtract(result, v, result);
					}

					vectoangles(result, result);
				} else {
					vectoangles(pSpline->segments[pos].v_norm, result);
				}
			} else {
				int pos = floor(deltaTime * (MAX_SPLINE_SEGMENTS));
				float frac;

				if (pos >= MAX_SPLINE_SEGMENTS) {
					pos = MAX_SPLINE_SEGMENTS - 1;
					frac = pSpline->segments[pos].length;
				} else {
					frac = ((deltaTime * (MAX_SPLINE_SEGMENTS)) - pos) * pSpline->segments[pos].length;
				}

				VectorMA(pSpline->segments[pos].start, frac, pSpline->segments[pos].v_norm, result);
			}

			break;
		case TR_WATER:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] += 0.5f * 20 * deltaTime * deltaTime;
			break;
		case TR_SINE:
			deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
			phase = sin(deltaTime * M_PI * 2);
			VectorMA(tr->trBase, phase, tr->trDelta, result);
			break;
		case TR_ORBITAL:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);

			angle = (float)((atTime + tr->trDuration) % 360) * M_PI / 180;
			result[0] += cos(angle) * 48;
			result[1] += sin(angle) * 48;

			angle = (float)((atTime / 6 + tr->trDuration) % 360) * M_PI / 180;
			result[2] += sin(angle) * 32;
			break;
		default:
			Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trType);
			break;
	}
}

/*
=======================================================================================================================================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time.
=======================================================================================================================================
*/
void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splineData) {
	float deltaTime;
	float phase;
	int gravity;
	char var[128];

	trap_Cvar_VariableStringBuffer("g_gravity", var, sizeof(var));
	gravity = atoi(var);

	switch (tr->trType) {
		case TR_STATIONARY:
		case TR_INTERPOLATE:
			VectorClear(result);
			break;
		case TR_ROTATING:
		case TR_LINEAR:
		case TR_LINEAR_BOB:
			VectorCopy(tr->trDelta, result);
			break;
		case TR_LINEAR_STOP:
			if (atTime > tr->trTime + tr->trDuration) {
				VectorClear(result);
				return;
			}

			VectorCopy(tr->trDelta, result);
			break;
		case TR_NONLINEAR_STOP:
			if (atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0) {
				VectorClear(result);
				return;
			}

			deltaTime = tr->trDuration * 0.001f * ((float)cos(DEG2RAD(90.0f - (90.0f * ((float)atTime - tr->trTime) / (float)tr->trDuration))));
			VectorScale(tr->trDelta, deltaTime, result);
			break;
		case TR_GRAVITY:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= gravity * deltaTime;
			break;
		case TR_GRAVITY_FLOAT:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= (gravity * 0.2f) * deltaTime;
			break;
		case TR_GRAVITY_SMALL:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= gravity * deltaTime;
			break;
		case TR_GRAVITY_LOW:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= 0.5f * gravity * deltaTime;
			break;
		case TR_GRAVITY_WATER:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorScale(tr->trDelta, 1 / (0.02f * deltaTime * VectorLength(tr->trDelta) + 1), result);
			result[2] -= 30.0f;
			break;
		case TR_GRAVITY_LOCAL:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= tr->trDuration * deltaTime;
			break;
		case TR_BUOYANCY:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] += gravity * deltaTime;
			break;
		case TR_ACCELERATE: // trDelta is eventual speed
			if (atTime > tr->trTime + tr->trDuration) {
				VectorClear(result);
				return;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			phase = deltaTime / (float)tr->trDuration;
			VectorScale(tr->trDelta, deltaTime * deltaTime, result);
			break;
		case TR_DECCELERATE: // trDelta is breaking force
			if (atTime > tr->trTime + tr->trDuration) {
				VectorClear(result);
				return;
			}

			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorScale(tr->trDelta, deltaTime, result);
			break;
		case TR_SPLINE:
		case TR_LINEAR_PATH:
			VectorClear(result);
			break;
		case TR_WATER:
			deltaTime = (atTime - tr->trTime) * 0.001f; // milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] += 0.5f * 20 * deltaTime * deltaTime;
			break;
		case TR_SINE:
			deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
			phase = cos(deltaTime * M_PI * 2); // derivative of sin = cos
			phase *= 0.5f;
			VectorScale(tr->trDelta, phase, result);
			break;
		case TR_ORBITAL:
			VectorCopy(tr->trDelta, result);
			break;
		default:
			Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trType);
			break;
	}
}

char *eventnames[] = {
	"EV_NONE",
	"EV_FIRE_WEAPON",
	"EV_BULLET_HIT_FLESH",
	"EV_BULLET_HIT_WALL",
	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_PHOSPHORTRAIL",
	"EV_RAILTRAIL",
	"EV_SHOTGUN",
	"EV_PROXIMITY_MINE_STICK",
	"EV_PROXIMITY_MINE_TRIGGER",
	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex
	"EV_KAMIKAZE",				// kamikaze explodes
	"EV_OBELISKEXPLODE",		// obelisk explodes
	"EV_OBELISKPAIN",			// obelisk is in pain
	"EV_STOPLOOPINGSOUND",
	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",			// no attenuation
	"EV_GLOBAL_TEAM_SOUND",
	"EV_PAIN",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_OBITUARY",
	"EV_GIB_PLAYER",			// gib a previously living player
	"EV_FALL_SHORT",
	"EV_FALL_MEDIUM",
	"EV_FALL_FAR",
	"EV_JUMP",
	"EV_JUMP_PAD",				// boing sound at origin", jump sound on player
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",
	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_FOOTSTEP_HARD",
	"EV_FOOTSTEP_HARD_FROZEN",
	"EV_FOOTSTEP_HARD_SNOW",
	"EV_FOOTSTEP_HARD_SLUSH",
	"EV_FOOTSTEP_PUDDLE",
	"EV_FOOTSTEP_LEAVES",
	"EV_FOOTSTEP_BUSH",
	"EV_FOOTSTEP_GRASS",
	"EV_FOOTSTEP_LONGGRASS",
	"EV_FOOTSTEP_LONGGRASS_MUD",
	"EV_FOOTSTEP_SAND",
	"EV_FOOTSTEP_GRAVEL",
	"EV_FOOTSTEP_RUBBLE",
	"EV_FOOTSTEP_RUBBLE_WET",
	"EV_FOOTSTEP_SOIL",
	"EV_FOOTSTEP_MUD",
	"EV_FOOTSTEP_SNOW_DEEP",
	"EV_FOOTSTEP_ICE",
	"EV_FOOTSTEP_METAL_HOLLOW",
	"EV_FOOTSTEP_METAL_HOLLOW_FROZEN",
	"EV_FOOTSTEP_METAL_HOLLOW_SNOW",
	"EV_FOOTSTEP_METAL_HOLLOW_SLUSH",
	"EV_FOOTSTEP_METAL_HOLLOW_SPLASH",
	"EV_FOOTSTEP_GRATE_01",
	"EV_FOOTSTEP_GRATE_02",
	"EV_FOOTSTEP_DUCT",
	"EV_FOOTSTEP_PLATE",
	"EV_FOOTSTEP_FENCE",
	"EV_FOOTSTEP_WOOD_HOLLOW",
	"EV_FOOTSTEP_WOOD_HOLLOW_FROZEN",
	"EV_FOOTSTEP_WOOD_HOLLOW_SNOW",
	"EV_FOOTSTEP_WOOD_HOLLOW_SLUSH",
	"EV_FOOTSTEP_WOOD_HOLLOW_SPLASH",
	"EV_FOOTSTEP_WOOD_SOLID",
	"EV_FOOTSTEP_WOOD_CREAKING",
	"EV_FOOTSTEP_ROOF",
	"EV_FOOTSTEP_SHINGLES",
	"EV_FOOTSTEP_SOFT",
	"EV_FOOTSTEP_GLASS_SHARDS",
	"EV_FOOTSTEP_TRASH_GLASS",
	"EV_FOOTSTEP_TRASH_DEBRIS",
	"EV_FOOTSTEP_TRASH_WIRE",
	"EV_FOOTSTEP_TRASH_PACKING",
	"EV_FOOTSTEP_TRASH_PLASTIC",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",
	"EV_WATER_TOUCH",			// foot touches
	"EV_WATER_LEAVE",			// foot leaves
	"EV_WATER_UNDER",			// head touches
	"EV_WATER_CLEAR",			// head leaves
	"EV_TAUNT",
	"EV_TAUNT_YES",
	"EV_TAUNT_NO",
	"EV_TAUNT_FOLLOWME",
	"EV_TAUNT_GETFLAG",
	"EV_TAUNT_GUARDBASE",
	"EV_TAUNT_PATROL",
	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",
	"EV_ITEM_POP",
	"EV_ITEM_RESPAWN",
	"EV_ITEM_PICKUP",			// normal item pickups are predictable
	"EV_GLOBAL_ITEM_PICKUP",	// powerup/team sounds are broadcast to everyone
	"EV_POWERUP_QUAD",
	"EV_POWERUP_REGEN",
	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_SCOREPLUM",				// score plum
#ifdef MISSIONPACK
	"EV_LIGHTNINGBOLT",
#endif
	"EV_DEBUG_LINE"
};

/*
=======================================================================================================================================
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers.
=======================================================================================================================================
*/
void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm, playerState_t *ps) {
#ifdef _DEBUG
	{
		char buf[256];
		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));

		if (atof(buf) != 0) {
#ifdef QAGAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_PS_EVENTS - 1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS - 1)] = eventParm;
	ps->eventSequence++;
}

/*
=======================================================================================================================================
BG_TouchJumpPad
=======================================================================================================================================
*/
void BG_TouchJumpPad(playerState_t *ps, entityState_t *jumppad) {
	vec3_t angles;
	float p;
	int effectNum;

	// spectators don't use jump pads
	if (ps->pm_type != PM_NORMAL) {
		return;
	}
	// if we didn't hit this same jumppad the previous frame then don't play the event sound again if we are in a fat trigger
	if (ps->jumppad_ent != jumppad->number) {
		vectoangles(jumppad->origin2, angles);
		p = fabs(AngleNormalize180(angles[PITCH]));

		if (p < 45) {
			effectNum = 0;
		} else {
			effectNum = 1;
		}

		BG_AddPredictableEventToPlayerstate(EV_JUMP_PAD, effectNum, ps);
	}
	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	VectorCopy(jumppad->origin2, ps->velocity);
}

/*
=======================================================================================================================================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server, and after local prediction on the client.
=======================================================================================================================================
*/
void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap) {
	int i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR) {
		s->eType = ET_INVISIBLE;
	} else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;
	s->pos.trType = TR_INTERPOLATE;

	VectorCopy(ps->origin, s->pos.trBase);

	if (snap) {
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction
	VectorCopy(ps->velocity, s->pos.trDelta);

	s->apos.trType = TR_INTERPOLATE;

	VectorCopy(ps->viewangles, s->apos.trBase);

	if (snap) {
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum; // ET_PLAYER looks here instead of at number, so corpses can also reference the proper config
	s->eFlags = ps->eFlags;

	if (ps->stats[STAT_HEALTH] <= 0) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	if (ps->externalEvent) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if (ps->entityEventSequence < ps->eventSequence) {
		int seq;

		if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}

		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		s->event = ps->events[seq]|((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;
	s->powerups = 0;

	for (i = 0; i < MAX_POWERUPS; i++) {
		if (ps->powerups[i]) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->tokens = ps->tokens;
	s->team = ps->persistant[PERS_TEAM];
}

/*
=======================================================================================================================================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server, and after local prediction on the client.
=======================================================================================================================================
*/
void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s, int time, qboolean snap) {
	int i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR) {
		s->eType = ET_INVISIBLE;
	} else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;
	s->pos.trType = TR_LINEAR_STOP;

	VectorCopy(ps->origin, s->pos.trBase);

	if (snap) {
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy(ps->velocity, s->pos.trDelta);
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)
	s->apos.trType = TR_INTERPOLATE;

	VectorCopy(ps->viewangles, s->apos.trBase);

	if (snap) {
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum; // ET_PLAYER looks here instead of at number, so corpses can also reference the proper config
	s->eFlags = ps->eFlags;

	if (ps->stats[STAT_HEALTH] <= 0) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	if (ps->externalEvent) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if (ps->entityEventSequence < ps->eventSequence) {
		int seq;

		if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}

		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		s->event = ps->events[seq]|((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;
	s->powerups = 0;

	for (i = 0; i < MAX_POWERUPS; i++) {
		if (ps->powerups[i]) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->tokens = ps->tokens;
	s->team = ps->persistant[PERS_TEAM];
}

/*
=======================================================================================================================================
SnapVectorTowards

Round a vector to integers for more efficient network transmission, but make sure that it rounds towards a given point rather than
blindly truncating. This prevents it from truncating into a wall.
=======================================================================================================================================
*/
void SnapVectorTowards(vec3_t v, vec3_t to) {
	int i;

	for (i = 0; i < 3; i++) {
		if (to[i] <= v[i]) {
			v[i] = floor(v[i]);
		} else {
			v[i] = ceil(v[i]);
		}
	}
}

/*
=======================================================================================================================================
cmdcmp
=======================================================================================================================================
*/
int cmdcmp(const void *a, const void *b) {
	return Q_stricmp((const char *)a, ((dummyCmd_t *)b)->name);
}
