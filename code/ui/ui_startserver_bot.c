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

#include "ui_local.h"
#include "ui_startserver_q3.h"

enum {
	ID_BOT_TYPE,
	ID_BOT_BOTNUMBER,
	ID_BOT_CHANGE,
	ID_BOT_SLOTS,
	ID_BOT_SKILL,
//	ID_BOT_SKILLRANGE,
	ID_BOT_SKILLBIAS,
	ID_BOT_JOINAS,
	ID_BOT_DELETESLOT,
	ID_BOT_LEFTTEAM,
	ID_BOT_RIGHTTEAM,
	ID_BOT_GAMETYPE,
	ID_BOT_SWAPARROWS,
	ID_BOT_ACTION
};

#define BOTSELECT_SWAPARROWS0 "menu/art/swaparrows_0"
#define BOTSELECT_SWAPARROWS1 "menu/art/swaparrows_1"
#define BOTSELECT_DEL0 "menu/art/del_0"
#define BOTSELECT_DEL1 "menu/art/del_1"
#define BOTSELECT_SELECT "menu/art/opponents_select"

#define BOT_LEFTCTRL COLUMN_LEFT
#define BOT_RIGHTCTRL COLUMN_RIGHT
#define BOTCOL_LEFT (2 * SMALLCHAR_WIDTH)
#define BOTCOL_RIGHT (320 + 6 * SMALLCHAR_WIDTH)

#define BOT_ICONX BOT_ICONY
#define BOT_ICONY SMALLCHAR_HEIGHT

#define BOTNAME_LENGTH 16 // chars displayed
#define BOTNAME_DX (8 * SMALLCHAR_WIDTH)

#define BOTSKILL_DX (25 * SMALLCHAR_WIDTH)
#define BOT_FADETIME 1000
/*
	new control for skill input

	override generic.ownerdraw and generic.callback
	use QMF_NODEFAULTINIT
	generic.type ignored
*/
typedef struct {
	menucommon_s generic;
	botskill_t *data;
} menuskill_s;
// enumerator for testing cursor position within control
enum {
	MSKILL_NONE,
	MSKILL_LEFT,
	MSKILL_RIGHT
};

typedef struct botcontrols_s {
	menuframework_s menu;
	commoncontrols_t common;
	menulist_s botGameType;
	menubitmap_s botGameTypeIcon;
	menulist_s botTypeSelect;
	menufield_s numberBots;
	menulist_s changeBots;
	menufield_s numberOpen;
	menulist_s skillType;
	menuskill_s skillValue;
	menulist_s skillBias;
	menuradiobutton_s joinAs;
	menutext_s teamLeft;
	menutext_s teamRight;
	menubitmap_s swapArrows;
	menubitmap_s delBot;
	menulist_s slotType[PLAYER_SLOTS];
	menutext_s slotName[PLAYER_SLOTS];
	menuskill_s slotSkill[PLAYER_SLOTS];
	menubitmap_s slotSelected[PLAYER_SLOTS];
	menubitmap_s moveSlot;
	menubitmap_s deleteSlot;
	menulist_s actionDest;
	menubitmap_s actionActivate;
	// local, used by the interface
	int selected;
	int fadetime;
	int statusbar_height;
	int savetime;
	char statusbar_message[MAX_STATUSBAR_TEXT];
	int hotspot; // emulates cursor over skill sfx
	char boticon[MAX_QPATH];
	char playername[MAX_NAME_LENGTH];
} botcontrols_t;
// controls
static botcontrols_t s_botcontrols;

static const char *skill_items[] = {
	"I Can Win!",
	"Bring it On",
	"Hurt Me Plenty",
	"Hardcore",
	"Nightmare!",
	0
};

static const char *botTypeSel_list[BOTTYPE_MAX + 1] = {
	"Hand selected",			// BOTTYPE_SELECT
	"Random",					// BOTTYPE_RANDOM
	"Random, list excluded",	// BOTTYPE_RANDOMEXCLUDE
	"Random, script size",		// BOTTYPE_RANDOMARENASCRIPT
	"From arena script",		// BOTTYPE_ARENASCRIPT
	"Selection, arena size",	// BOTTYPE_SELECTARENASCRIPT
	0
};

static const char *botSkill_list[BOTSKILL_COUNT + 1] = {
	"Identical",		// BOTSKILL_SAME
	"Ranged",			// BOTSKILL_RANGE
	"Custom, single",	// BOTSKILL_CUSTOMSAME
	"Custom, range",	// BOTSKILL_CUSTOMRANGE
	0
};

static const char *botSlotType_list[SLOTTYPE_COUNT + 1] = {
	"----",		// SLOTTYPE_EMPTY
	"Human",	// SLOTTYPE_HUMAN
	"Bot",		// SLOTTYPE_BOT
	"Open",		// SLOTTYPE_OPEN
	0
};

static const char *botSkillBias_list[SKILLBIAS_COUNT + 1] = {
	"None",				// SKILLBIAS_NONE
	"Very low",			// SKILLBIAS_VLOW
	"Low",				// SKILLBIAS_LOW
	"High",				// SKILLBIAS_HIGH
	"Very high",		// SKILLBIAS_VHIGH
	"Fractional",		// SKILLBIAS_FRACTIONAL
	"Frac, Very low",	// SKILLBIAS_FRAC_VLOW
	"Frac, Low",		// SKILLBIAS_FRAC_LOW
	"Frac, High",		// SKILLBIAS_FRAC_HIGH
	"Frac, Very high",	// SKILLBIAS_FRAC_VHIGH
	0
};

static const char *botChange_list[BOTCHANGE_COUNT + 1] = {
	"Never",			// BOTCHANGE_NEVER
	"Every map",		// BOTCHANGE_EVERYMAP
	"Every 2nd map",	// BOTCHANGE_MAP2
	"Every 3rd map",	// BOTCHANGE_MAP3
	"Every 4th map",	// BOTCHANGE_MAP4
	0
};

const char *botCopyTo_items[BOT_CT_COUNT + 1] = {
	"Clear all bot and open slots",				// BOT_CT_CLEARALL
	"Copy individual skill to selected bot",	// BOT_CT_INDIV_SELECTED
	"Copy ranged skill to selected bot",		// BOT_CT_RANGE_SELECTED
	"Copy individual skill to all bots",		// BOT_CT_INDIV_ALL
	"Copy ranged skill to all bots",			// BOT_CT_RANGE_ALL
	"Neaten list of bots",						// BOT_CT_NEATEN
	0
};

/*
=======================================================================================================================================
StartServer_BotPage_UpdateButtonInterface
=======================================================================================================================================
*/
static void StartServer_BotPage_UpdateButtonInterface(void) {

	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		s_botcontrols.swapArrows.generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
	} else {
		s_botcontrols.swapArrows.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	}

	if (s_botcontrols.selected != -1) {
		s_botcontrols.swapArrows.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.delBot.generic.flags &= ~QMF_GRAYED;
	} else {
		s_botcontrols.swapArrows.generic.flags |= QMF_GRAYED;
		s_botcontrols.delBot.generic.flags |= QMF_GRAYED;
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_SelectItem
=======================================================================================================================================
*/
static void StartServer_BotPage_SelectItem(int index) {

	if (index >= PLAYER_SLOTS) {
		return;
	}

	if (s_botcontrols.selected >= 0) {
		s_botcontrols.slotSelected[s_botcontrols.selected].generic.flags &= ~QMF_HIGHLIGHT;

		if (s_botcontrols.selected == index) {
			index = -1;
		}
	}

	if (index != -1) {
		s_botcontrols.slotSelected[index].generic.flags |= QMF_HIGHLIGHT;
	}

	s_botcontrols.selected = index;
}

/*
=======================================================================================================================================
StartServer_BotPage_SetSkillRangeType
=======================================================================================================================================
*/
static void StartServer_BotPage_SetSkillRangeType(void) {

	StartServer_SetBotSkillRangeType(s_botcontrols.skillType.curvalue);
	s_botcontrols.skillType.curvalue = s_scriptdata.bot.skillType;
}

/*
=======================================================================================================================================
StartServer_BotPage_UpdateInterface
=======================================================================================================================================
*/
static void StartServer_BotPage_UpdateInterface(void) {
	int i, value, skill, last;
	qboolean grayBots, hideTeam, hideBotType;
	qboolean grayBotNumber, grayBotChange;

	StartServer_SetIconFromGameType(&s_botcontrols.botGameTypeIcon, s_scriptdata.gametype, qfalse);

	hideTeam = qtrue;

	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		hideTeam = qfalse;
	}

	grayBots = qfalse;
	hideBotType = qfalse;
	grayBotNumber = qtrue;
	grayBotChange = qfalse;

	switch (s_botcontrols.botTypeSelect.curvalue) {
		default:
		case BOTTYPE_SELECT:
			grayBotChange = qtrue;
			break;
		case BOTTYPE_RANDOM:
			grayBotNumber = qfalse;
			grayBots = qtrue;
			break;
		case BOTTYPE_RANDOMEXCLUDE:
			hideBotType = qtrue;
			hideTeam = qtrue;
			grayBotNumber = qfalse;
			break;
		case BOTTYPE_RANDOMARENASCRIPT:
			hideBotType = qtrue;
			hideTeam = qtrue;
			grayBotNumber = qfalse;
			break;
		case BOTTYPE_ARENASCRIPT:
			grayBotNumber = qfalse;
			grayBots = qtrue;
			grayBotChange = qtrue;
			break;
		case BOTTYPE_SELECTARENASCRIPT:
			grayBotChange = qtrue;
			grayBotNumber = qfalse;
			break;
	}
	// bot selection method
	if (grayBotNumber) {
		s_botcontrols.numberBots.generic.flags |= QMF_GRAYED;
		s_botcontrols.changeBots.generic.flags |= QMF_GRAYED;
		s_botcontrols.numberOpen.generic.flags |= QMF_GRAYED;
	} else {
		s_botcontrols.numberBots.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.changeBots.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.numberOpen.generic.flags &= ~QMF_GRAYED;
	}

	if (grayBots) {
		for (i = 0; i < PLAYER_SLOTS; i++) {
			s_botcontrols.slotType[i].generic.flags |= QMF_GRAYED;
			s_botcontrols.slotName[i].generic.flags |= QMF_GRAYED;
			s_botcontrols.slotSkill[i].generic.flags |= QMF_GRAYED;
			s_botcontrols.slotSelected[i].generic.flags |= QMF_GRAYED;
		}

		s_botcontrols.teamLeft.generic.flags |= QMF_GRAYED;
		s_botcontrols.teamRight.generic.flags |= QMF_GRAYED;

		StartServer_BotPage_SelectItem(-1);

		s_botcontrols.actionDest.generic.flags |= QMF_GRAYED;
		s_botcontrols.actionActivate.generic.flags |= QMF_GRAYED;
	} else {
		for (i = 0; i < PLAYER_SLOTS; i++) {
			s_botcontrols.slotType[i].generic.flags &= ~QMF_GRAYED;
			s_botcontrols.slotName[i].generic.flags &= ~QMF_GRAYED;
			s_botcontrols.slotSkill[i].generic.flags &= ~QMF_GRAYED;
			s_botcontrols.slotSelected[i].generic.flags &= ~QMF_GRAYED;
		}

		s_botcontrols.teamLeft.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.teamRight.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.actionDest.generic.flags &= ~QMF_GRAYED;
		s_botcontrols.actionActivate.generic.flags &= ~QMF_GRAYED;
	}

	if (grayBotChange) {
		s_botcontrols.changeBots.generic.flags |= QMF_GRAYED;
	} else {
		s_botcontrols.changeBots.generic.flags &= ~QMF_GRAYED;
	}
	// left/right arrow and del controls
	StartServer_BotPage_UpdateButtonInterface();
	// slot skill type
	skill = s_botcontrols.skillType.curvalue;

	if (skill == BOTSKILL_SAME || skill == BOTSKILL_CUSTOMSINGLE) {
		s_botcontrols.skillBias.generic.flags |= QMF_GRAYED;
	} else {
		s_botcontrols.skillBias.generic.flags &= ~QMF_GRAYED;
	}
	// skill value
	if (skill < BOTSKILL_CUSTOMSINGLE) {
		s_botcontrols.skillValue.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	} else {
		s_botcontrols.skillValue.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
	}
	// team game type
	if (hideTeam) {
		s_botcontrols.teamLeft.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.teamRight.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	} else {
		s_botcontrols.teamLeft.generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.teamRight.generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
	}
	// bot slots
	for (i = 0; i < PLAYER_SLOTS; i++) {
		value = s_scriptdata.bot.slotType[i];

		if (skill >= BOTSKILL_CUSTOMSINGLE && value == SLOTTYPE_BOT && s_scriptdata.bot.name[i][0]) {
			s_botcontrols.slotSkill[i].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		} else {
			s_botcontrols.slotSkill[i].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		}

		if (value == SLOTTYPE_BOT || StartServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
			s_botcontrols.slotName[i].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
			StartServer_BotNameDrawn(i, qtrue);
		} else {
			s_botcontrols.slotName[i].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
			StartServer_BotNameDrawn(i, qfalse);
		}

		if (hideBotType) {
			s_botcontrols.slotType[i].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		} else {
			s_botcontrols.slotType[i].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		}

		s_botcontrols.slotSelected[i].generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	// enable fight button if possible
	StartServer_CheckFightReady(&s_botcontrols.common);
	// deal with player when name is hidden by spectator enabled
	if (StartServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		return;
	}

	last = PLAYER_SLOTS - 1;

	if (!hideTeam) {
		last = PLAYER_SLOTS_PERCOL - 1;
	}

	if (s_botcontrols.joinAs.curvalue) {
		// player as spectator
		s_botcontrols.slotType[last].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.slotName[last].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.slotSkill[last].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.slotSelected[last].generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
	} else {
		// player in game
		s_botcontrols.slotType[0].generic.flags &= ~QMF_HIDDEN;
		s_botcontrols.slotName[0].generic.flags &= ~QMF_HIDDEN;
		s_botcontrols.slotType[0].generic.flags |= QMF_INACTIVE;
		s_botcontrols.slotName[0].generic.flags |= QMF_INACTIVE;
		s_botcontrols.slotSkill[0].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		s_botcontrols.slotSelected[0].generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_CopySlotTypeToInterface
=======================================================================================================================================
*/
static void StartServer_BotPage_CopySlotTypeToInterface(void) {
	int i;

	// update slot type from script data
	for (i = 0; i < PLAYER_SLOTS; i++) {
		s_botcontrols.slotType[i].curvalue = s_scriptdata.bot.slotType[i];
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_MoveToOtherTeam
=======================================================================================================================================
*/
static void StartServer_BotPage_MoveToOtherTeam(void) {

	StartServer_MoveBotToOtherTeam(s_botcontrols.selected);
	StartServer_BotPage_CopySlotTypeToInterface();
}

/*
=======================================================================================================================================
StartServer_BotPage_DeleteSlot
=======================================================================================================================================
*/
static void StartServer_BotPage_DeleteSlot(int index) {

	if (!StartServer_DeleteBotSlot(index)) {
		return;
	}
	// update slot type from script data
	StartServer_BotPage_CopySlotTypeToInterface();

	if (s_botcontrols.selected > index) {
		StartServer_BotPage_SelectItem(s_botcontrols.selected - 1);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_InsertSlot
=======================================================================================================================================
*/
static void StartServer_BotPage_InsertSlot(int index) {

	if (!StartServer_InsertBotSlot(index)) {
		return;
	}
	// update slot type from script data
	StartServer_BotPage_CopySlotTypeToInterface();

	if (index <= s_botcontrols.selected) {
		StartServer_BotPage_SelectItem(s_botcontrols.selected + 1);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_SetPlayerNamedSlot
=======================================================================================================================================
*/
static void StartServer_BotPage_SetPlayerNamedSlot(void) {

	if (StartServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		return;
	}

	if (s_scriptdata.bot.joinAs && s_scriptdata.bot.slotType[0] == SLOTTYPE_HUMAN) {
		StartServer_BotPage_DeleteSlot(0);
		StartServer_BotPage_UpdateInterface();
		return;
	}

	if (s_scriptdata.bot.slotType[0] != SLOTTYPE_HUMAN) {
		StartServer_BotPage_InsertSlot(0);
		s_scriptdata.bot.slotType[0] = SLOTTYPE_HUMAN;
		s_botcontrols.slotType[0].curvalue = s_scriptdata.bot.slotType[0];
		StartServer_BotPage_UpdateInterface();
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_ValidateSlotCount
=======================================================================================================================================
*/
static void StartServer_BotPage_ValidateSlotCount(void) {
	int bots, open;

	bots = atoi(s_botcontrols.numberBots.field.buffer);
	open = atoi(s_botcontrols.numberOpen.field.buffer);

	StartServer_ValidateBotSlotCount(bots, open);

	Q_strncpyz(s_botcontrols.numberBots.field.buffer, va("%i", s_scriptdata.bot.numberBots), 3);
	Q_strncpyz(s_botcontrols.numberOpen.field.buffer, va("%i", s_scriptdata.bot.numberOpen), 3);
}

/*
=======================================================================================================================================
StartServer_BotPage_HandleGoAction
=======================================================================================================================================
*/
static void StartServer_BotPage_HandleGoAction(void) {

	StartServer_DoBotAction(s_botcontrols.actionDest.curvalue, s_botcontrols.selected);
	StartServer_BotPage_CopySlotTypeToInterface();
	StartServer_BotPage_UpdateInterface();
}

/*
=======================================================================================================================================
StartServer_BotPage_Cache
=======================================================================================================================================
*/
void StartServer_BotPage_Cache(void) {

	trap_R_RegisterShaderNoMip(BOTSELECT_SWAPARROWS0);
	trap_R_RegisterShaderNoMip(BOTSELECT_SWAPARROWS1);
	trap_R_RegisterShaderNoMip(BOTSELECT_DEL0);
	trap_R_RegisterShaderNoMip(BOTSELECT_DEL1);
	trap_R_RegisterShaderNoMip(BOTSELECT_SELECT);
	trap_R_RegisterShaderNoMip(GAMESERVER_SELECTED0);
	trap_R_RegisterShaderNoMip(GAMESERVER_SELECTED1);
	trap_R_RegisterShaderNoMip(GAMESERVER_ACTION0);
	trap_R_RegisterShaderNoMip(GAMESERVER_ACTION1);
}

/*
=======================================================================================================================================
StartServer_BotPage_SetTeamTitle
=======================================================================================================================================
*/
static void StartServer_BotPage_SetTeamTitle(int swapped) {
	int x, w;
	menutext_s *red, *blue;
	float sizeScale;

	if (swapped == -1) {
		// toggle values
		if (s_scriptdata.bot.teamSwapped) {
			swapped = 0;
		} else {
			swapped = 1;
		}
	}

	if (swapped == 0) {
		// default
		red = &s_botcontrols.teamLeft;
		blue = &s_botcontrols.teamRight;
		s_scriptdata.bot.teamSwapped = qfalse;
	} else {
		// swapped
		blue = &s_botcontrols.teamLeft;
		red = &s_botcontrols.teamRight;
		s_scriptdata.bot.teamSwapped = qtrue;
	}

	red->string = "Team Red";
	red->color = color_red;
	blue->string = "Team Blue";
	blue->color = color_blue;

	sizeScale = UI_ProportionalSizeScale(red->style);

	x = red->generic.x;
	w = UI_ProportionalStringWidth(red->string) * sizeScale;
	x -= w / 2;

	red->generic.left = x - PROP_GAP_WIDTH * sizeScale;
	red->generic.right = x + w + PROP_GAP_WIDTH * sizeScale;

	x = blue->generic.x;
	w = UI_ProportionalStringWidth(blue->string) * sizeScale;
	x -= w / 2;

	blue->generic.left = x - PROP_GAP_WIDTH * sizeScale;
	blue->generic.right = x + w + PROP_GAP_WIDTH * sizeScale;
}

/*
=======================================================================================================================================
StartServer_BotPage_InitControlsFromScript
=======================================================================================================================================
*/
static void StartServer_BotPage_InitControlsFromScript(void) {

	// method of selecting bots
	s_botcontrols.botTypeSelect.curvalue = s_scriptdata.bot.typeSelect;
	// number of bots if randomly generated
	Q_strncpyz(s_botcontrols.numberBots.field.buffer, va("%i", s_scriptdata.bot.numberBots), 3);
	// frequency of bot changing on maps
	s_botcontrols.changeBots.curvalue = s_scriptdata.bot.changeBots;
	// number of open slots if bots are randomly selected
	Q_strncpyz(s_botcontrols.numberOpen.field.buffer, va("%i", s_scriptdata.bot.numberOpen), 3);
	// skill selection method for bots
	s_botcontrols.skillType.curvalue = s_scriptdata.bot.skillType;
	// skill bias
	s_botcontrols.skillBias.curvalue = s_scriptdata.bot.skillBias;
	// join game as
	s_botcontrols.joinAs.curvalue = s_scriptdata.bot.joinAs;
	// nothing selected
	StartServer_BotPage_SelectItem(-1);
	// swap teams
	if (s_scriptdata.gametype > GT_TOURNAMENT) {
		StartServer_BotPage_SetTeamTitle(s_scriptdata.bot.teamSwapped);
	}

	StartServer_BotPage_CopySlotTypeToInterface();
}

/*
=======================================================================================================================================
StartServer_BotPage_InitSkillControl
=======================================================================================================================================
*/
static void StartServer_BotPage_InitSkillControl(menuskill_s *s) {
	int x, y, h;

	x = s->generic.x;
	y = s->generic.y;
	h = BOT_ICONY;

	if (h < SMALLCHAR_HEIGHT) {
		h = SMALLCHAR_HEIGHT;
	}

	s->generic.left = x;
	s->generic.top = y;
	s->generic.right = x + (2 * BOT_ICONX) + (3 * SMALLCHAR_WIDTH);
	s->generic.bottom = y + h;
}

/*
=======================================================================================================================================
StartServer_BotPage_LoadBotNameList

Specialist interface init, used when user has toggled type of bot selection.
=======================================================================================================================================
*/
static void StartServer_BotPage_LoadBotNameList(int type) {

	StartServer_LoadBotNameList(type);
	// it is possible for StartServer_LoadBotNameList() to change the selection type, so we recover that change here
	s_botcontrols.botTypeSelect.curvalue = s_scriptdata.bot.typeSelect;

	StartServer_BotPage_CopySlotTypeToInterface();
}

/*
=======================================================================================================================================
StartServer_BotPage_Load
=======================================================================================================================================
*/
static void StartServer_BotPage_Load(void) {

	s_botcontrols.botGameType.curvalue = gametype_remap2[s_scriptdata.gametype];

	Q_strncpyz(s_botcontrols.playername, UI_Cvar_VariableString("name"), MAX_NAME_LENGTH);

	StartServer_BotPage_InitControlsFromScript();
	StartServer_BotPage_SetSkillRangeType();
}

/*
=======================================================================================================================================
StartServer_BotPage_Save
=======================================================================================================================================
*/
static void StartServer_BotPage_Save(void) {
	StartServer_SaveScriptData();
}

/*
=======================================================================================================================================
StartServer_BotPage_DrawStatusBarText
=======================================================================================================================================
*/
static void StartServer_BotPage_DrawStatusBarText(void) {
	vec4_t color;
	int fade;
	float fadecol;

	if (uis.realtime > s_botcontrols.savetime) {
		return;
	}

	if (!s_botcontrols.statusbar_message[0]) {
		return;
	}

	fade = s_botcontrols.savetime - uis.realtime;
	fadecol = (float)fade / STATUSBAR_FADETIME;

	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	color[3] = fadecol;

	UI_DrawString(320, s_botcontrols.statusbar_height, s_botcontrols.statusbar_message, UI_CENTER|UI_SMALLFONT, color);
}

/*
=======================================================================================================================================
StartServer_BotPage_SetStatusBar
=======================================================================================================================================
*/
static void StartServer_BotPage_SetStatusBar(const char *text) {

	s_botcontrols.savetime = uis.realtime + STATUSBAR_FADETIME;

	if (text) {
		Q_strncpyz(s_botcontrols.statusbar_message, text, MAX_STATUSBAR_TEXT);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_StatusBar
=======================================================================================================================================
*/
static void StartServer_BotPage_StatusBar(void *ptr) {

	switch (((menucommon_s *)ptr)->id) {
		case ID_BOT_TYPE:
			switch (s_scriptdata.bot.typeSelect) {
				case BOTTYPE_SELECT:
					StartServer_BotPage_SetStatusBar("Same bots for all maps");
					break;
				case BOTTYPE_RANDOM:
					StartServer_BotPage_SetStatusBar("Randomly picked bots for all maps");
					break;
				case BOTTYPE_RANDOMEXCLUDE:
					StartServer_BotPage_SetStatusBar("Randomly picked bots, but exclude least favourite");
					break;
				case BOTTYPE_RANDOMARENASCRIPT:
					StartServer_BotPage_SetStatusBar("Replace map recommended bots with random ones");
					break;
				case BOTTYPE_ARENASCRIPT:
					StartServer_BotPage_SetStatusBar("Use map recommended bots");
					break;
			}

			break;
		case ID_BOT_BOTNUMBER:
			if (StartServer_IsBotArenaScript(s_scriptdata.bot.typeSelect)) {
				StartServer_BotPage_SetStatusBar("Number of bots if arena script has none");
			}

			break;
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_SkillOffset
=======================================================================================================================================
*/
static int StartServer_BotPage_SkillOffset(qboolean range) {

	if (range) {
		return (BOT_ICONX + 2 * SMALLCHAR_WIDTH);
	}

	return (BOT_ICONX + 2 * SMALLCHAR_WIDTH) / 2;
}

/*
=======================================================================================================================================
StartServer_BotPage_OverSkillHotspot
=======================================================================================================================================
*/
static int StartServer_BotPage_OverSkillHotspot(menuskill_s *s) {
	int x, y, h;

	x = s->generic.left;
	y = s->generic.top;
	h = s->generic.bottom - y;

	if (s->data->range && UI_CursorInRect(x, y, BOT_ICONX + SMALLCHAR_WIDTH, h)) {
		return MSKILL_LEFT;
	}

	x += StartServer_BotPage_SkillOffset(s->data->range);

	if (UI_CursorInRect(x, y, BOT_ICONX + SMALLCHAR_WIDTH, h)) {
		return MSKILL_RIGHT;
	}

	return MSKILL_NONE;
}

/*
=======================================================================================================================================
StartServer_BotPage_SkillStatusBar
=======================================================================================================================================
*/
static void StartServer_BotPage_SkillStatusBar(void *self) {
	menuskill_s *s;
	int index, cursor;

	s = (menuskill_s *)self;
	cursor = StartServer_BotPage_OverSkillHotspot(s);

	if (cursor == MSKILL_NONE) {
		return;
	}

	if (s->data->range) {
		if (cursor == MSKILL_LEFT) {
			index = s->data->low;
		} else {
			index = s->data->high;
		}
	} else {
		index = s->data->value;
	}

	StartServer_BotPage_SetStatusBar(skill_items[index]);
//	UI_DrawString(140, s_botcontrols.statusbar_height, skill_items[index], UI_CENTER|UI_SMALLFONT, color_white);
}

/*
=======================================================================================================================================
StartServer_BotPage_SkillDraw
=======================================================================================================================================
*/
static void StartServer_BotPage_SkillDraw(void *self) {
	int skill;
	menuskill_s *s;
	int x, y, h;
	int shader_y, cursor;
	qhandle_t shader;
	vec4_t temp_bkcolor, tempcolor;
	float *color;

	s = (menuskill_s *)self;
	x = s->generic.left;
	y = s->generic.top;
	h = s->generic.bottom - y;

	temp_bkcolor[0] = listbar_color[0];
	temp_bkcolor[1] = listbar_color[1];
	temp_bkcolor[2] = listbar_color[2];
	temp_bkcolor[3] = 0.3 + 0.3 * sin(uis.realtime / PULSE_DIVISOR);

	tempcolor[0] = text_color_highlight[0];
	tempcolor[1] = text_color_highlight[1];
	tempcolor[2] = text_color_highlight[2];
	tempcolor[3] = 0.7 + 0.3 * sin(uis.realtime / PULSE_DIVISOR);

	pulsecolor[3] = tempcolor[3];

	cursor = StartServer_BotPage_OverSkillHotspot(s);
	// draw left side of control
	shader_y = y + (h - BOT_ICONY) / 2;

	if (s->data->range) {
		skill = s->data->low + 1;
		shader = trap_R_RegisterShaderNoMip(va("menu/art/skill%i", skill));

		if (s->generic.flags & QMF_GRAYED) {
			color = text_color_disabled;

			trap_R_SetColor(menu_dim_color);
			UI_DrawHandlePic(x, shader_y, BOT_ICONX, BOT_ICONY, shader);
			trap_R_SetColor(NULL);
		} else if (cursor == MSKILL_LEFT) {
			// mouse over control, "pulsing"
			UI_FillRect(x, y, BOT_ICONX + SMALLCHAR_WIDTH + 1, h + 1, temp_bkcolor);
			color = tempcolor;

			trap_R_SetColor(pulsecolor);
			UI_DrawHandlePic(x, shader_y, BOT_ICONX, BOT_ICONY, shader);
			trap_R_SetColor(NULL);
		} else {
			color = text_color_normal;
			UI_DrawHandlePic(x, shader_y, BOT_ICONX, BOT_ICONY, shader);
		}

		UI_DrawString(x + BOT_ICONX, y, va("%i", skill), UI_LEFT|UI_SMALLFONT, color);
		UI_DrawString(x + BOT_ICONX + SMALLCHAR_WIDTH, y, "-", UI_LEFT|UI_SMALLFONT, text_color_normal);
		// setup for second part of skill control
		skill = s->data->high + 1;
	} else {
		skill = s->data->value + 1;
	}

	if (cursor != MSKILL_NONE && cursor != s_botcontrols.hotspot) {
		trap_S_StartLocalSound(menu_move_sound, CHAN_LOCAL_SOUND);
		s_botcontrols.hotspot = cursor;
	}

	x += StartServer_BotPage_SkillOffset(s->data->range);
	// draw right side of control (always drawn)
	shader = trap_R_RegisterShaderNoMip(va("menu/art/skill%i", skill));

	if (s->generic.flags & QMF_GRAYED) {
		color = text_color_disabled;

		trap_R_SetColor(menu_dim_color);
		UI_DrawHandlePic(x, shader_y, BOT_ICONX, BOT_ICONY, shader);
		trap_R_SetColor(NULL);
	} else if (cursor == MSKILL_RIGHT) {
		// mouse over control, "pulsing"
		UI_FillRect(x, y, BOT_ICONX + SMALLCHAR_WIDTH + 1, h + 1, temp_bkcolor);
		color = tempcolor;

		trap_R_SetColor(pulsecolor);
		UI_DrawHandlePic(x + SMALLCHAR_WIDTH, shader_y, BOT_ICONX, BOT_ICONY, shader);
		trap_R_SetColor(NULL);
	} else {
		color = text_color_normal;
		UI_DrawHandlePic(x + SMALLCHAR_WIDTH, shader_y, BOT_ICONX, BOT_ICONY, shader);
	}

	UI_DrawString(x, y, va("%i", skill), UI_LEFT|UI_SMALLFONT, color);
}

/*
=======================================================================================================================================
StartServer_BotPage_SkillEvent
=======================================================================================================================================
*/
static void StartServer_BotPage_SkillEvent(void *ptr, int event) {
	int hit;
	menuskill_s *s;
	botskill_t *data;

	if (event == QM_GOTFOCUS) {
		trap_S_StartLocalSound(menu_move_sound, CHAN_LOCAL_SOUND);
		s_botcontrols.hotspot = MSKILL_NONE;
		return;
	}

	if (event != QM_ACTIVATED) {
		return;
	}

	s = (menuskill_s *)ptr;
	data = s->data;
	hit = StartServer_BotPage_OverSkillHotspot(s);

	if (hit == MSKILL_LEFT) {
		s->data->low++;

		if (data->low > data->high) {
			data->low = 0;
		}
	} else if (hit == MSKILL_RIGHT) {
		if (data->range) {
			data->high++;

			if (data->high == MAX_SKILL) {
				data->high = data->low;
			}
		} else {
			data->value++;

			if (data->value == MAX_SKILL) {
				data->value = 0;
			}
		}
	}

	if (hit != MSKILL_NONE) {
		trap_S_StartLocalSound(menu_move_sound, CHAN_LOCAL_SOUND);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_CommonEvent
=======================================================================================================================================
*/
static void StartServer_BotPage_CommonEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	StartServer_BotPage_Save();

	switch (((menucommon_s *)ptr)->id) {
		case ID_SERVERCOMMON_SERVER:
			StartServer_ServerPage_MenuInit();
			break;
		case ID_SERVERCOMMON_MAPS:
			UI_PopMenu();
			break;
		case ID_SERVERCOMMON_BACK:
			UI_PopMenu();
			UI_PopMenu();
			break;
		case ID_SERVERCOMMON_FIGHT:
			StartServer_CreateServer(NULL);
			break;
		case ID_SINGLEPLAYER:
			UI_PopMenu();
			UI_SPLevelMenu();
			break;
		case ID_SERVERS:
			UI_PopMenu();
			UI_ArenaServersMenu();
			break;
		case ID_SPECIFY:
			UI_PopMenu();
			UI_SpecifyServerMenu();
			break;
		case ID_CREATE:
			break;
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_SelectionEvent
=======================================================================================================================================
*/
static void StartServer_BotPage_SelectionEvent(void *ptr, int event) {

	if (event == QM_ACTIVATED) {
		StartServer_BotPage_SelectItem(((menucommon_s *)ptr)->id);
		StartServer_BotPage_UpdateInterface();
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_NameDraw
=======================================================================================================================================
*/
static void StartServer_BotPage_NameDraw(void *self) {
	int x, y, pulse, style;
	vec4_t tempcolor;
	float *color;
	menutext_s *t;
	char *string, buffer[32];

	t = (menutext_s *)self;
	x = t->generic.x;
	y = t->generic.y;
	style = UI_SMALLFONT;

	pulse = ((t->generic.flags & QMF_PULSE) || (Menu_ItemAtCursor(t->generic.parent) == t));

	if (t->generic.flags & QMF_GRAYED) {
		color = text_color_disabled;
	} else if (pulse) {
		tempcolor[0] = text_color_highlight[0];
		tempcolor[1] = text_color_highlight[1];
		tempcolor[2] = text_color_highlight[2];
		tempcolor[3] = 0.7 + 0.3 * sin(uis.realtime / PULSE_DIVISOR);

		color = tempcolor;
		style |= UI_PULSE;

		UI_FillRect(x, y, t->generic.right - x + 1, t->generic.bottom - y + 1, listbar_color);
		UI_DrawChar(x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	} else {
		color = text_color_normal;
	}

	string = NULL;

	if (s_botcontrols.slotType[t->generic.id].curvalue == SLOTTYPE_HUMAN) {
		string = s_botcontrols.playername;

		if (t->generic.flags & QMF_GRAYED) {
			Q_strncpyz(buffer, string, 32);
			Q_CleanStr(buffer);
			string = buffer;
		}
	} else if (s_scriptdata.bot.drawName[t->generic.id]) {
		string = t->string;

		if (string[0] == '\0') {
			string = "----";
		}
	}

	if (string) {
		UI_DrawString(x, y, string, style, color);
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_NameEvent
=======================================================================================================================================
*/
static void StartServer_BotPage_NameEvent(void *ptr, int event) {
	int index, i, botnum;
	char botname[MAX_NAME_LENGTH], *bot;

	index = ((menucommon_s *)ptr)->id;

	switch (event) {
		case QM_ACTIVATED:
			UI_BotSelect_Index(s_scriptdata.bot.name[index], index);
			break;
		case QM_GOTFOCUS:
			botnum = UI_GetNumBots();
			s_botcontrols.boticon[0] = '\0';

			for (i = 0; i < botnum; i++) {
				bot = UI_GetBotInfoByNumber(i);
				strncpy(botname, Info_ValueForKey(bot, "name"), MAX_NAME_LENGTH);
				Q_CleanStr(botname);

				if (Q_stricmp(botname, s_scriptdata.bot.name[index])) {
					continue;
				}

				UI_ServerPlayerIcon(Info_ValueForKey(bot, "model"), s_botcontrols.boticon, MAX_QPATH);
				break;
			}

			break;
		case QM_LOSTFOCUS:
			break;
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_TypeEvent
=======================================================================================================================================
*/
static void StartServer_BotPage_TypeEvent(void *ptr, int event) {
	int index;
	menulist_s *s;

	if (event != QM_ACTIVATED) {
		return;
	}

	s = (menulist_s *)ptr;
	index = s->generic.id;

	if (s->curvalue == SLOTTYPE_HUMAN) {
		// prevent human index, and wrap if necessary
		s->curvalue++;

		if (s->curvalue == s->numitems) {
			s->curvalue = 0;
		}
	}

	s_scriptdata.bot.slotType[index] = s->curvalue;

	StartServer_BotPage_UpdateInterface();
}

/*
=======================================================================================================================================
StartServer_BotPage_MenuKey
=======================================================================================================================================
*/
static sfxHandle_t StartServer_BotPage_MenuKey(int key) {

	switch (key) {
		case K_MOUSE2:
		case K_ESCAPE:
			StartServer_BotPage_Save();
			UI_PopMenu();
			break;
	}

	return (Menu_DefaultKey(&s_botcontrols.menu, key));
}

/*
=======================================================================================================================================
StartServer_BotPage_Event
=======================================================================================================================================
*/
static void StartServer_BotPage_Event(void *ptr, int event) {

	switch (((menucommon_s *)ptr)->id) {
		// controls that change script data
		case ID_BOT_GAMETYPE:
			if (event != QM_ACTIVATED) {
				return;
			}
			// make all changes before updating control page
			StartServer_SaveScriptData();
			StartServer_BotPage_SelectItem(-1);
			StartServer_LoadScriptDataFromType(gametype_remap[s_botcontrols.botGameType.curvalue]);
			StartServer_BotPage_InitControlsFromScript();
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_TYPE:
			if (event != QM_ACTIVATED) {
				return;
			}

			StartServer_SaveBotNameList();
			StartServer_BotPage_LoadBotNameList(s_botcontrols.botTypeSelect.curvalue);
			StartServer_BotPage_SelectItem(-1);
			StartServer_BotPage_SetSkillRangeType();
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_LEFTTEAM:
		case ID_BOT_RIGHTTEAM:
			if (event != QM_ACTIVATED) {
				return;
			}

			StartServer_BotPage_SetTeamTitle(-1);
			break;
		case ID_BOT_DELETESLOT:
			if (event != QM_ACTIVATED) {
				return;
			}

			StartServer_BotPage_DeleteSlot(s_botcontrols.selected);
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_SWAPARROWS:
			if (event != QM_ACTIVATED) {
				return;
			}

			StartServer_BotPage_MoveToOtherTeam();
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_JOINAS:
			if (event != QM_ACTIVATED) {
				return;
			}

			s_scriptdata.bot.joinAs = s_botcontrols.joinAs.curvalue;

			StartServer_BotPage_SetPlayerNamedSlot();
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_BOTNUMBER:
		case ID_BOT_SLOTS:
			if (event == QM_LOSTFOCUS) {
				StartServer_BotPage_ValidateSlotCount();
			}

			break;
		case ID_BOT_ACTION:
			if (event != QM_ACTIVATED) {
				break;
			}

			StartServer_BotPage_HandleGoAction();
			break;
		case ID_BOT_SKILL:
			if (event != QM_ACTIVATED) {
				return;
			}

			s_scriptdata.bot.skillType = s_botcontrols.skillType.curvalue;

			StartServer_BotPage_SetSkillRangeType();
			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_CHANGE:
			if (event != QM_ACTIVATED) {
				return;
			}

			s_scriptdata.bot.changeBots = s_botcontrols.changeBots.curvalue;

			StartServer_BotPage_UpdateInterface();
			break;
		case ID_BOT_SKILLBIAS:
			if (event != QM_ACTIVATED) {
				return;
			}

			s_scriptdata.bot.skillBias = s_botcontrols.skillBias.curvalue;

			StartServer_BotPage_UpdateInterface();
			break;
			// controls that just change something on the interface
	}
}

/*
=======================================================================================================================================
StartServer_BotPage_MenuDraw
=======================================================================================================================================
*/
static void StartServer_BotPage_MenuDraw(void) {
	qboolean excluded;
	menulist_s *b;
	menucommon_s *g;
	qhandle_t pic;
	int i;
	float f;

	if (uis.firstdraw) {
		StartServer_BotPage_Load();
		StartServer_BotPage_UpdateInterface();
	}

	excluded = qfalse;

	if (StartServer_IsRandomBotExclude(s_scriptdata.bot.typeSelect)) {
		excluded = qtrue;
	}

	StartServer_BackgroundDraw(excluded);
	// draw bot icon
	for (i = 0; i < PLAYER_SLOTS; i++) {
		b = &s_botcontrols.slotType[i];

		if (b->curvalue != SLOTTYPE_BOT) {
			continue;
		}

		g = &s_botcontrols.slotName[i].generic;

		if (g->flags &(QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN)) {
			continue;
		}

		if (UI_CursorInRect(g->left, g->top, g->right - g->left, g->bottom - g->top)) {
			s_botcontrols.fadetime = uis.realtime + BOT_FADETIME;
			break;
		}
	}

	if (uis.realtime < s_botcontrols.fadetime) {
		f = (float)(s_botcontrols.fadetime - uis.realtime) / BOT_FADETIME;
		pulsecolor[3] = f;
		fading_red[3] = f;
		pic = trap_R_RegisterShaderNoMip(s_botcontrols.boticon);

		if (pic) {
			trap_R_SetColor(pulsecolor);
			UI_DrawHandlePic(640 - 64 - 64, 48, 64, 64, pic);
			trap_R_SetColor(fading_red);
			UI_DrawNamedPic(640 - 64 - 64 - 15, 48 - 16, 128, 128, BOTSELECT_SELECT);
			trap_R_SetColor(NULL);
		}
	}
	// draw the controls
	Menu_Draw(&s_botcontrols.menu);

	StartServer_BotPage_DrawStatusBarText();
}

/*
=======================================================================================================================================
StartServer_BotPage_MenuInit
=======================================================================================================================================
*/
void StartServer_BotPage_MenuInit(void) {
	menuframework_s *menuptr;
	int y, i, list_y, colx, sel_colx;

	memset(&s_botcontrols, 0, sizeof(botcontrols_t));

	StartServer_BotPage_Cache();

	menuptr = &s_botcontrols.menu;
	menuptr->key = StartServer_BotPage_MenuKey;
	menuptr->wrapAround = qtrue;
	menuptr->fullscreen = qtrue;
	menuptr->draw = StartServer_BotPage_MenuDraw;

	StartServer_CommonControls_Init(menuptr, &s_botcontrols.common, StartServer_BotPage_CommonEvent, COMMONCTRL_BOTS);
	// the user selected bots
	y = GAMETYPEROW_Y;
	s_botcontrols.botGameType.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.botGameType.generic.id = ID_BOT_GAMETYPE;
	s_botcontrols.botGameType.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.botGameType.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.botGameType.generic.x = GAMETYPECOLUMN_X;
	s_botcontrols.botGameType.generic.y = y;
	s_botcontrols.botGameType.generic.name = "Game Type:";
	s_botcontrols.botGameType.itemnames = gametype_items;

	s_botcontrols.botGameTypeIcon.generic.type = MTYPE_BITMAP;
	s_botcontrols.botGameTypeIcon.generic.flags = QMF_INACTIVE;
	s_botcontrols.botGameTypeIcon.generic.x = GAMETYPEICON_X;
	s_botcontrols.botGameTypeIcon.generic.y = y;
	s_botcontrols.botGameTypeIcon.width = 32;
	s_botcontrols.botGameTypeIcon.height = 32;

	y += LINE_HEIGHT;
	s_botcontrols.botTypeSelect.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.botTypeSelect.generic.id = ID_BOT_TYPE;
	s_botcontrols.botTypeSelect.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.botTypeSelect.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.botTypeSelect.generic.statusbar = StartServer_BotPage_StatusBar;
	s_botcontrols.botTypeSelect.generic.x = GAMETYPECOLUMN_X;
	s_botcontrols.botTypeSelect.generic.y = y;
	s_botcontrols.botTypeSelect.generic.name = "Bot selection:";
	s_botcontrols.botTypeSelect.itemnames = botTypeSel_list;

	y += 2 * LINE_HEIGHT;
	s_botcontrols.numberBots.generic.type = MTYPE_FIELD;
	s_botcontrols.numberBots.generic.name = "Number of bots:";
	s_botcontrols.numberBots.generic.id = ID_BOT_BOTNUMBER;
	s_botcontrols.numberBots.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.numberBots.generic.statusbar = StartServer_BotPage_StatusBar;
	s_botcontrols.numberBots.generic.flags = QMF_SMALLFONT|QMF_PULSEIFFOCUS;
	s_botcontrols.numberBots.generic.x = BOT_RIGHTCTRL;
	s_botcontrols.numberBots.generic.y = y;
	s_botcontrols.numberBots.field.widthInChars = 2;
	s_botcontrols.numberBots.field.maxchars = 2;

	s_botcontrols.skillType.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.skillType.generic.id = ID_BOT_SKILL;
	s_botcontrols.skillType.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.skillType.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.skillType.generic.x = BOT_LEFTCTRL;
	s_botcontrols.skillType.generic.y = y;
	s_botcontrols.skillType.generic.name = "Skill:";
	s_botcontrols.skillType.itemnames = botSkill_list;
	// custom skill control
	s_botcontrols.skillValue.generic.type = MTYPE_NULL;
	s_botcontrols.skillValue.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NODEFAULTINIT|QMF_SILENT;
	s_botcontrols.skillValue.generic.ownerdraw = StartServer_BotPage_SkillDraw;
	s_botcontrols.skillValue.generic.statusbar = StartServer_BotPage_SkillStatusBar;
	s_botcontrols.skillValue.generic.callback = StartServer_BotPage_SkillEvent;
	s_botcontrols.skillValue.generic.x = BOT_LEFTCTRL + 13 * SMALLCHAR_WIDTH + BOT_ICONX;
	s_botcontrols.skillValue.generic.y = y;
	s_botcontrols.skillValue.data = &s_scriptdata.bot.globalSkill;
	StartServer_BotPage_InitSkillControl(&s_botcontrols.skillValue);

	y += LINE_HEIGHT;
	s_botcontrols.changeBots.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.changeBots.generic.id = ID_BOT_CHANGE;
	s_botcontrols.changeBots.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.changeBots.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.changeBots.generic.x = BOT_RIGHTCTRL;
	s_botcontrols.changeBots.generic.y = y;
	s_botcontrols.changeBots.generic.name = "Change bots:";
	s_botcontrols.changeBots.itemnames = botChange_list;

	s_botcontrols.skillBias.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.skillBias.generic.id = ID_BOT_SKILLBIAS;
	s_botcontrols.skillBias.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.skillBias.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.skillBias.generic.x = BOT_LEFTCTRL;
	s_botcontrols.skillBias.generic.y = y;
	s_botcontrols.skillBias.generic.name = "Skill bias:";
	s_botcontrols.skillBias.itemnames = botSkillBias_list;

	y += LINE_HEIGHT;
	s_botcontrols.numberOpen.generic.type = MTYPE_FIELD;
	s_botcontrols.numberOpen.generic.name = "Open slots:";
	s_botcontrols.numberOpen.generic.id = ID_BOT_SLOTS;
	s_botcontrols.numberOpen.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.numberOpen.generic.flags = QMF_SMALLFONT|QMF_PULSEIFFOCUS;
	s_botcontrols.numberOpen.generic.x = BOT_RIGHTCTRL;
	s_botcontrols.numberOpen.generic.y = y;
	s_botcontrols.numberOpen.field.widthInChars = 2;
	s_botcontrols.numberOpen.field.maxchars = 2;

	s_botcontrols.joinAs.generic.type = MTYPE_RADIOBUTTON;
	s_botcontrols.joinAs.generic.id = ID_BOT_JOINAS;
	s_botcontrols.joinAs.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.joinAs.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.joinAs.generic.x = BOT_LEFTCTRL;
	s_botcontrols.joinAs.generic.y = y;
	s_botcontrols.joinAs.generic.name = "Spectator:";
	// re-initialized in StartServer_BotPage_SetTeamTitle()
	y += (3 * LINE_HEIGHT) / 2;
	s_botcontrols.teamLeft.generic.type = MTYPE_PTEXT;
	s_botcontrols.teamLeft.generic.id = ID_BOT_LEFTTEAM;
	s_botcontrols.teamLeft.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_CENTER_JUSTIFY;
	s_botcontrols.teamLeft.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.teamLeft.generic.x = BOT_RIGHTCTRL;
	s_botcontrols.teamLeft.generic.y = y;
	s_botcontrols.teamLeft.string = "";
	s_botcontrols.teamLeft.style = UI_CENTER;
	s_botcontrols.teamLeft.color = 0;

	s_botcontrols.teamRight.generic.type = MTYPE_PTEXT;
	s_botcontrols.teamRight.generic.id = ID_BOT_RIGHTTEAM;
	s_botcontrols.teamRight.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_CENTER_JUSTIFY;
	s_botcontrols.teamRight.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.teamRight.generic.x = BOT_LEFTCTRL;
	s_botcontrols.teamRight.generic.y = y;
	s_botcontrols.teamRight.string = "";
	s_botcontrols.teamRight.style = UI_CENTER;
	s_botcontrols.teamRight.color = 0;

	y += 2 + PROP_HEIGHT * UI_ProportionalSizeScale(s_botcontrols.teamLeft.style);
	s_botcontrols.swapArrows.generic.type = MTYPE_BITMAP;
	s_botcontrols.swapArrows.generic.name = BOTSELECT_SWAPARROWS0;
	s_botcontrols.swapArrows.generic.flags = QMF_PULSEIFFOCUS;
	s_botcontrols.swapArrows.generic.x = 320 - 18;
	s_botcontrols.swapArrows.generic.y = y;
	s_botcontrols.swapArrows.generic.id = ID_BOT_SWAPARROWS;
	s_botcontrols.swapArrows.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.swapArrows.width = 36;
	s_botcontrols.swapArrows.height = 36;
	s_botcontrols.swapArrows.focuspic = BOTSELECT_SWAPARROWS1;

	s_botcontrols.delBot.generic.type = MTYPE_BITMAP;
	s_botcontrols.delBot.generic.name = BOTSELECT_DEL0;
	s_botcontrols.delBot.generic.flags = QMF_PULSEIFFOCUS;
	s_botcontrols.delBot.generic.x = 320 - 18;
	s_botcontrols.delBot.generic.y = y + 48;
	s_botcontrols.delBot.generic.id = ID_BOT_DELETESLOT;
	s_botcontrols.delBot.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.delBot.width = 36;
	s_botcontrols.delBot.height = 72;
	s_botcontrols.delBot.focuspic = BOTSELECT_DEL1;

	colx = BOTCOL_LEFT;
	sel_colx = 320 - 40;

	for (i = 0; i < PLAYER_SLOTS; i++) {
		list_y = y + (i % PLAYER_SLOTS_PERCOL) * (LINE_HEIGHT + 4);

		if (i == PLAYER_SLOTS_PERCOL) {
			colx = BOTCOL_RIGHT;
			sel_colx = 320 + 24;
		}

		s_botcontrols.slotType[i].generic.type = MTYPE_SPINCONTROL;
		s_botcontrols.slotType[i].generic.id = i;
		s_botcontrols.slotType[i].generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_botcontrols.slotType[i].generic.callback = StartServer_BotPage_TypeEvent;
		s_botcontrols.slotType[i].generic.x = colx;
		s_botcontrols.slotType[i].generic.y = list_y;
		s_botcontrols.slotType[i].generic.name = 0;
		s_botcontrols.slotType[i].itemnames = botSlotType_list;

		s_botcontrols.slotName[i].generic.type = MTYPE_TEXT;
		s_botcontrols.slotName[i].generic.id = i;
		s_botcontrols.slotName[i].generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NODEFAULTINIT;
		s_botcontrols.slotName[i].generic.callback = StartServer_BotPage_NameEvent;
		s_botcontrols.slotName[i].generic.ownerdraw = StartServer_BotPage_NameDraw;
		s_botcontrols.slotName[i].generic.x = colx + BOTNAME_DX;
		s_botcontrols.slotName[i].generic.y = list_y;
		s_botcontrols.slotName[i].generic.left = s_botcontrols.slotName[i].generic.x;
		s_botcontrols.slotName[i].generic.top = list_y;
		s_botcontrols.slotName[i].generic.right = s_botcontrols.slotName[i].generic.x + BOTNAME_LENGTH * SMALLCHAR_WIDTH;
		s_botcontrols.slotName[i].generic.bottom = list_y + SMALLCHAR_HEIGHT;
		s_botcontrols.slotName[i].string = s_scriptdata.bot.name[i];

		s_botcontrols.slotSelected[i].generic.type = MTYPE_BITMAP;
		s_botcontrols.slotSelected[i].generic.name = GAMESERVER_SELECTED0;
		s_botcontrols.slotSelected[i].generic.flags = QMF_PULSEIFFOCUS;
		s_botcontrols.slotSelected[i].generic.x = sel_colx;
		s_botcontrols.slotSelected[i].generic.y = list_y;
		s_botcontrols.slotSelected[i].generic.id = i;
		s_botcontrols.slotSelected[i].generic.callback = StartServer_BotPage_SelectionEvent;
		s_botcontrols.slotSelected[i].generic.ownerdraw = StartServer_SelectionDraw;
		s_botcontrols.slotSelected[i].width = 16;
		s_botcontrols.slotSelected[i].height = 16;
		s_botcontrols.slotSelected[i].focuspic = GAMESERVER_SELECTED1;
		// custom skill control
		s_botcontrols.slotSkill[i].generic.type = MTYPE_NULL;
		s_botcontrols.slotSkill[i].generic.id = i;
		s_botcontrols.slotSkill[i].generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NODEFAULTINIT|QMF_SILENT;
		s_botcontrols.slotSkill[i].generic.ownerdraw = StartServer_BotPage_SkillDraw;
		s_botcontrols.slotSkill[i].generic.statusbar = StartServer_BotPage_SkillStatusBar;
		s_botcontrols.slotSkill[i].generic.callback = StartServer_BotPage_SkillEvent;
		s_botcontrols.slotSkill[i].generic.x = colx + BOTSKILL_DX;
		s_botcontrols.slotSkill[i].generic.y = list_y;
		s_botcontrols.slotSkill[i].data = &s_scriptdata.bot.skill[i];
		StartServer_BotPage_InitSkillControl(&s_botcontrols.slotSkill[i]);
	}

	y += 8 * (LINE_HEIGHT + 4) + LINE_HEIGHT;
	s_botcontrols.actionDest.generic.type = MTYPE_SPINCONTROL;
	s_botcontrols.actionDest.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_botcontrols.actionDest.generic.x = 240;
	s_botcontrols.actionDest.generic.y = y;
	s_botcontrols.actionDest.itemnames = botCopyTo_items;

	s_botcontrols.actionActivate.generic.type = MTYPE_BITMAP;
	s_botcontrols.actionActivate.generic.name = GAMESERVER_ACTION0;
	s_botcontrols.actionActivate.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_botcontrols.actionActivate.generic.callback = StartServer_BotPage_Event;
	s_botcontrols.actionActivate.generic.id = ID_BOT_ACTION;
	s_botcontrols.actionActivate.generic.x = 240 - 64 - SMALLCHAR_WIDTH;
	s_botcontrols.actionActivate.generic.y = y - (32 - SMALLCHAR_HEIGHT) / 2;
	s_botcontrols.actionActivate.width = 64;
	s_botcontrols.actionActivate.height = 32;
	s_botcontrols.actionActivate.focuspic = GAMESERVER_ACTION1;
	s_botcontrols.statusbar_height = y - 20;
	// register page controls
	Menu_AddItem(menuptr, &s_botcontrols.botGameType);
	Menu_AddItem(menuptr, &s_botcontrols.botGameTypeIcon);
	Menu_AddItem(menuptr, &s_botcontrols.botTypeSelect);
	Menu_AddItem(menuptr, &s_botcontrols.numberBots);
	Menu_AddItem(menuptr, &s_botcontrols.changeBots);
	Menu_AddItem(menuptr, &s_botcontrols.numberOpen);
	Menu_AddItem(menuptr, &s_botcontrols.skillType);
	Menu_AddItem(menuptr, &s_botcontrols.skillValue);
	Menu_AddItem(menuptr, &s_botcontrols.skillBias);
	Menu_AddItem(menuptr, &s_botcontrols.joinAs);
	Menu_AddItem(menuptr, &s_botcontrols.teamLeft);
	Menu_AddItem(menuptr, &s_botcontrols.teamRight);
	Menu_AddItem(menuptr, &s_botcontrols.swapArrows);
	Menu_AddItem(menuptr, &s_botcontrols.delBot);

	for (i = 0; i < PLAYER_SLOTS; i++) {
		Menu_AddItem(menuptr, &s_botcontrols.slotName[i]);
		Menu_AddItem(menuptr, &s_botcontrols.slotSelected[i]);
		Menu_AddItem(menuptr, &s_botcontrols.slotType[i]);
		Menu_AddItem(menuptr, &s_botcontrols.slotSkill[i]);
	}

	Menu_AddItem(menuptr, &s_botcontrols.actionDest);
	Menu_AddItem(menuptr, &s_botcontrols.actionActivate);

	UI_PushMenu(&s_botcontrols.menu);
}
