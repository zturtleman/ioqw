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

	DYANMIC MENU HEADER

=======================================================================================================================================
*/

#define PT_FRIENDLY				1
#define PT_ENEMY				2
#define PT_BOTONLY				4
#define PT_PLAYERONLY			8
#define PT_EXCLUDEPARENT		16
#define PT_EXCLUDEGRANDPARENT	32

typedef void (*createHandler)(void);
typedef void (*eventHandler)(int index);
// manipulate menu
qboolean DynamicMenu_AddItem(const char *string, int id, createHandler crh, eventHandler evh);
qboolean DynamicMenu_AddIconItem(const char *string, int id, const char *icon, createHandler crh, eventHandler evh);
void DynamicMenu_AddBackground(const char *background);
qboolean DynamicMenu_SubMenuInit(void);
void DynamicMenu_FinishSubMenuInit(void);
void DynamicMenu_MenuInit(qboolean fullscreen, qboolean wraparound);
void DynamicMenu_ClearFocus(int pos);
void DynamicMenu_SetFocus(int pos);
void DynamicMenu_SetFlags(int depth, int id, int flags);
void DynamicMenu_RemoveFlags(int depth, int id, int flags);
// information about the menu structure

// general
int DynamicMenu_Depth(void);
// takes an index and returns data
qboolean DynamicMenu_OnActiveList(int index);
const char *DynamicMenu_StringAtIndex(int index);
int DynamicMenu_DepthOfIndex(int index);
int DynamicMenu_IdAtIndex(int index);
// takes a depth and returns data
int DynamicMenu_ActiveIdAtDepth(int depth);
int DynamicMenu_ActiveIndexAtDepth(int depth);
// returns data about items
const char *DynamicMenu_ItemShortname(int index);
// core services
void DynamicMenu_AddListOfPlayers(int type, createHandler crh, eventHandler evh);
void DynamicMenu_AddListOfItems(int exclude, createHandler crh, eventHandler evh);
