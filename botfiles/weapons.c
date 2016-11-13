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

#include "inv.h"

#define VEC_ORIGIN {0, 0, 0}
// projectile flags
#define PFL_WINDOWDAMAGE		1 // projectile damages through window
#define PFL_RETURN				2 // set when projectile returns to owner
// weapon flags
#define WFL_FIRERELEASED		1 // set when projectile is fired with key-up event
// damage types
#define DAMAGETYPE_IMPACT		1 // damage on impact
#define DAMAGETYPE_RADIAL		2 // radial damage
#define DAMAGETYPE_VISIBLE		4 // damage to all entities visible to the projectile
#define DAMAGETYPE_IGNOREARMOR	8 // projectile goes right through armor

//===========================================================================
// Gauntlet
//===========================================================================
projectileinfo
{
	name			"gauntletdamage"
	gravity			0.0
	damage			50
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Gauntlet"
	number			WEAPONINDEX_GAUNTLET
	projectile		"gauntletdamage"
	numprojectiles	0
	hspread			0
	vspread			0
	speed			0
}

//===========================================================================
// Machinegun
//===========================================================================
projectileinfo
{
	name			"machinegunbullet"
	gravity			0.0
	damage			7
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Machinegun"
	number			WEAPONINDEX_MACHINEGUN
	projectile		"machinegunbullet"
	numprojectiles	1
	hspread			1
	vspread			1
	speed			0
}

//===========================================================================
// Chaingun
//===========================================================================
projectileinfo
{
	name			"chaingunbullet"
	gravity			0.0
	damage			7
	damagetype		$evalint(DAMAGETYPE_IMPACT)
}

weaponinfo
{
	name			"Chaingun"
	number			WEAPONINDEX_CHAINGUN
	projectile		"chaingunbullet"
	numprojectiles	1
	hspread			1
	vspread			1
	speed			0
}

//===========================================================================
// Shotgun
//===========================================================================
projectileinfo
{
	name			"shotgunbullet"
	gravity			0.0
	damage			10
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Shotgun"
	number			WEAPONINDEX_SHOTGUN
	projectile		"shotgunbullet"
	numprojectiles	11
	hspread			1
	vspread			1
	speed			0
}

//===========================================================================
// Nailgun
//===========================================================================
projectileinfo
{
	name			"nail"
	gravity			0.0
	damage			20
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Nailgun"
	number			WEAPONINDEX_NAILGUN
	projectile		"nail"
	numprojectiles	15
	hspread			1
	vspread			1
	speed			1600
}

//===========================================================================
// Prox Launcher
//===========================================================================
projectileinfo
{
	name			"mine"
	gravity			0.45
	damage			0
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Prox Launcher"
	number			WEAPONINDEX_PROXLAUNCHER
	projectile		"mine"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			700
}

//===========================================================================
// Grenade Launcher
//===========================================================================
projectileinfo
{
	name			"grenade"
	gravity			0.45 //0.35
	damage			120
	radius			160
	damagetype		$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo
{
	name			"Grenade Launcher"
	number			WEAPONINDEX_GRENADE_LAUNCHER
	projectile		"grenade"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			700 //1300
}

//===========================================================================
// Napalm Launcher
//===========================================================================
//projectileinfo
//{
//	name			"napalm"
//	gravity			0.04 // speed: 10000 !!!!!!!!!!!!
//	damage			120
//	radius			160
//	damagetype		$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
//}

//weaponinfo
//{
//	name			"Napalm Launcher"
//	number			WEAPONINDEX_NAPALM_LAUNCHER
//	projectile		"napalm"
//	numprojectiles	1
//	hspread			0
//	vspread			0
//	speed			10000 // gravity: 0.04 !!!!!!!!!!!!
//}

//===========================================================================
// Rocket Launcher
//===========================================================================
projectileinfo
{
	name			"rocket"
	gravity			0.0
	damage			100
	radius			120
	damagetype		$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo
{
	name			"Rocket Launcher"
	number			WEAPONINDEX_ROCKET_LAUNCHER
	projectile		"rocket"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			900
}

//===========================================================================
// Lightning
//===========================================================================
projectileinfo
{
	name			"lightning"
	gravity			0.0
	damage			8
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Lightning Gun"
	number			WEAPONINDEX_LIGHTNING
	projectile		"lightning"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			0
}

//===========================================================================
// Railgun
//===========================================================================
projectileinfo
{
	name			"rail"
	gravity			0.0
	damage			100
	damagetype		DAMAGETYPE_IMPACT
}

weaponinfo
{
	name			"Railgun"
	number			WEAPONINDEX_RAILGUN
	projectile		"rail"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			0
}

//===========================================================================
// Plasma Gun
//===========================================================================
projectileinfo
{
	name			"plasma"
	gravity			0.0
	damage			20
	radius			20
	damagetype		$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo
{
	name			"Plasma Gun"
	number			WEAPONINDEX_PLASMAGUN
	projectile		"plasma"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			2000
}

//===========================================================================
// BFG10K
//===========================================================================
projectileinfo
{
	name			"bfgexplosion"
	gravity			0.0
	damage			100
	radius			100
	damagetype		$evalint(DAMAGETYPE_IMPACT|DAMAGETYPE_RADIAL)
}

weaponinfo
{
	name			"BFG10K"
	number			WEAPONINDEX_BFG
	projectile		"bfgexplosion"
	numprojectiles	1
	hspread			0
	vspread			0
	speed			2000
}