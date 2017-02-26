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

/*
=======================================================================================================================================

	ADD BOTS MENU

=======================================================================================================================================
*/

#include "ui_local.h"

#define ART_BACK0 "menu/art/back_0"
#define ART_BACK1 "menu/art/back_1"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"
#define ART_BACKGROUND "menu/art/addbotframe"
#define ART_ARROWS "menu/art/arrows_vert_0"
#define ART_ARROWUP "menu/art/arrows_vert_top"
#define ART_ARROWDOWN "menu/art/arrows_vert_bot"
#define ART_SELECT "menu/art/opponents_select"

enum {
	ID_BACK,
	ID_GO,
	ID_LIST,
	ID_UP,
	ID_DOWN,
	ID_SKILL,
	ID_TEAM,
	ID_BOTNAME0,
	ID_BOTNAME1,
	ID_BOTNAME2,
	ID_BOTNAME3,
	ID_BOTNAME4,
	ID_BOTNAME5,
	ID_BOTNAME6,
	ID_SKILLSLIDER
};

typedef struct {
	menuframework_s menu;
	menubitmap_s arrows;
	menubitmap_s up;
	menubitmap_s down;
	menutext_s bots[7];
	menulist_s skill;
	menuslider_s skill_slider;
	menulist_s team;
	menubitmap_s icon;
	menubitmap_s icon_hilite;
	menubitmap_s go;
	menubitmap_s back;
	int numBots;
	int delay;
	int baseBotNum;
	int selectedBotNum;
	int sortedBotNums[MAX_BOTS];
	char botnames[7][32];
	char boticon[MAX_QPATH];
	int gametype;
} addBotsMenuInfo_t;

static addBotsMenuInfo_t addBotsMenuInfo;

static const char *skillNames[] = {
	"I Can Win",
	"Bring It On",
	"Hurt Me Plenty",
	"Hardcore",
	"Nightmare!",
	NULL
};

static const char *teamNames1[] = {
	"Free",
	NULL
};

static const char *teamNames2[] = {
	"Red",
	"Blue",
	NULL
};

/*
=======================================================================================================================================
AddBots_SetBotIcon
=======================================================================================================================================
*/
static void AddBots_SetBotIcon(void) {
	char *info;

	info = UI_GetBotInfoByNumber(addBotsMenuInfo.sortedBotNums[addBotsMenuInfo.baseBotNum + addBotsMenuInfo.selectedBotNum]);

	UI_ServerPlayerIcon(Info_ValueForKey(info, "model"), addBotsMenuInfo.boticon, MAX_QPATH);

	addBotsMenuInfo.icon.shader = 0;
}

/*
=======================================================================================================================================
UI_AddBotsMenu_SkillChangeEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_SkillChangeEvent(void *ptr, int event) {
	menucommon_s *common;

	if (event != QM_ACTIVATED) {
		return;
	}
	// ties the skill controls together
	common = (menucommon_s *)ptr;

	switch (common->id) {
		case ID_SKILL: {
			menulist_s *skill = (menulist_s *)ptr;
			addBotsMenuInfo.skill_slider.curvalue = skill->curvalue;
			break;
		}

		case ID_SKILLSLIDER: {
			menuslider_s *skillslider = (menuslider_s *)ptr;
			addBotsMenuInfo.skill.curvalue = (int)skillslider->curvalue;
			break;
		}
	}
}

/*
=======================================================================================================================================
UI_AddBotsMenu_FightEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_FightEvent(void *ptr, int event) {
	const char *team;
	float skill;

	if (event != QM_ACTIVATED) {
		return;
	}

	team = addBotsMenuInfo.team.itemnames[addBotsMenuInfo.team.curvalue];
	skill = addBotsMenuInfo.skill_slider.curvalue + 1;

	trap_Cmd_ExecuteText(EXEC_APPEND, va("addbot %s %4.2f %s %i\n", addBotsMenuInfo.botnames[addBotsMenuInfo.selectedBotNum], skill, team, addBotsMenuInfo.delay));

	addBotsMenuInfo.delay += 1500;
}

/*
=======================================================================================================================================
UI_AddBotsMenu_BotEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_BotEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	addBotsMenuInfo.bots[addBotsMenuInfo.selectedBotNum].color = color_orange;
	addBotsMenuInfo.selectedBotNum = ((menucommon_s *)ptr)->id - ID_BOTNAME0;
	addBotsMenuInfo.bots[addBotsMenuInfo.selectedBotNum].color = color_white;

	AddBots_SetBotIcon();
}

/*
=======================================================================================================================================
UI_AddBotsMenu_TeamEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_TeamEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (addBotsMenuInfo.gametype < GT_TEAM) {
		return;
	}

	if (addBotsMenuInfo.team.curvalue == 1) {
		addBotsMenuInfo.icon_hilite.focuscolor = color_blue;
	} else {
		addBotsMenuInfo.icon_hilite.focuscolor = color_red;
	}
}

/*
=======================================================================================================================================
UI_AddBotsMenu_BackEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_BackEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	UI_PopMenu();
}

/*
=======================================================================================================================================
UI_AddBotsMenu_SetBotNames
=======================================================================================================================================
*/
static void UI_AddBotsMenu_SetBotNames(void) {
	int n;
	const char *info;

	for (n = 0; n < 7; n++) {
		info = UI_GetBotInfoByNumber(addBotsMenuInfo.sortedBotNums[addBotsMenuInfo.baseBotNum + n]);
		Q_strncpyz(addBotsMenuInfo.botnames[n], Info_ValueForKey(info, "name"), sizeof(addBotsMenuInfo.botnames[n]));
	}
}

/*
=======================================================================================================================================
UI_AddBotsMenu_UpEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_UpEvent(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	if (addBotsMenuInfo.baseBotNum > 0) {
		addBotsMenuInfo.baseBotNum -= 7;

		if (addBotsMenuInfo.baseBotNum < 0) {
			addBotsMenuInfo.baseBotNum = 0;
		}

		UI_AddBotsMenu_SetBotNames();
		AddBots_SetBotIcon();
	}
}

/*
=======================================================================================================================================
UI_AddBotsMenu_DownEvent
=======================================================================================================================================
*/
static void UI_AddBotsMenu_DownEvent(void *ptr, int event) {
	int bot;

	if (event != QM_ACTIVATED) {
		return;
	}

	bot = addBotsMenuInfo.baseBotNum;

	if (bot + 7 < addBotsMenuInfo.numBots) {
		bot += 7;
	} else {
		return;
	}

	if (bot + 7 >= addBotsMenuInfo.numBots) {
		bot = addBotsMenuInfo.numBots - 7;

		if (bot < 0) {
			bot = 0;
		}
	}

	addBotsMenuInfo.baseBotNum = bot;

	UI_AddBotsMenu_SetBotNames();
	AddBots_SetBotIcon();
}

/*
=======================================================================================================================================
UI_AddBotsMenu_SortCompare
=======================================================================================================================================
*/
static int QDECL UI_AddBotsMenu_SortCompare(const void *arg1, const void *arg2) {
	int num1, num2;
	const char *info1, *info2;
	const char *name1, *name2;

	num1 = *(int *)arg1;
	num2 = *(int *)arg2;

	info1 = UI_GetBotInfoByNumber(num1);
	info2 = UI_GetBotInfoByNumber(num2);

	name1 = Info_ValueForKey(info1, "name");
	name2 = Info_ValueForKey(info2, "name");
	// put random option first
	if (Q_stricmp(name1, "Random") == 0) {
		return -1;
	}

	if (Q_stricmp(name2, "Random") == 0) {
		return 1;
	}

	return Q_stricmp(name1, name2);
}

/*
=======================================================================================================================================
UI_AddBotsMenu_GetSortedBotNums
=======================================================================================================================================
*/
static void UI_AddBotsMenu_GetSortedBotNums(void) {
	int n;

	// initialize the array
	for (n = 0; n < addBotsMenuInfo.numBots; n++) {
		addBotsMenuInfo.sortedBotNums[n] = n;
	}

	qsort(addBotsMenuInfo.sortedBotNums, addBotsMenuInfo.numBots, sizeof(addBotsMenuInfo.sortedBotNums[0]), UI_AddBotsMenu_SortCompare);
}

/*
=======================================================================================================================================
UI_AddBotsMenu_Draw
=======================================================================================================================================
*/
static void UI_AddBotsMenu_Draw(void) {
	int x, y, w, h, skill;
	float f_skill;
	qhandle_t hpic;
	menubitmap_s *b;

	UI_DrawBannerString(320, 16, "Add Bots", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&addBotsMenuInfo.menu);

	if (addBotsMenuInfo.boticon[0]) {
		skill = addBotsMenuInfo.skill.curvalue + 1;
		hpic = trap_R_RegisterShaderNoMip(va("menu/art/skill%i", skill));

		if (!hpic) {
			return;
		}
		// put icon in bottom right corner of pic
		b = &addBotsMenuInfo.icon;
		w = b->width;
		h = b->height;
		x = b->generic.x + w;
		y = b->generic.y + h;

		w /= 3;
		h /= 3;
		x -= w;
		y -= h;

		trap_R_SetColor(color_black);

		UI_DrawHandlePic(x, y, w, h, hpic);

		trap_R_SetColor(NULL);

		UI_DrawHandlePic(x - 2, y - 2, w, h, hpic);
		// write bot skill as float
		x = b->generic.x + b->width - 4 * SMALLCHAR_WIDTH;
		y = b->generic.y + b->height + 2;
		f_skill = addBotsMenuInfo.skill_slider.curvalue + 1;

		UI_DrawString(x, y, va("%4.2f", f_skill), UI_SMALLFONT, color_orange);
	}
}

/*
=======================================================================================================================================
UI_AddBotsMenu_Init
=======================================================================================================================================
*/
static void UI_AddBotsMenu_Init(void) {
	int n;
	int y;
	int count;
	char info[MAX_INFO_STRING];

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);

	memset(&addBotsMenuInfo, 0, sizeof(addBotsMenuInfo));

	addBotsMenuInfo.menu.draw = UI_AddBotsMenu_Draw;
	addBotsMenuInfo.menu.fullscreen = qfalse;
	addBotsMenuInfo.menu.wrapAround = qtrue;
	addBotsMenuInfo.delay = 1000;
	addBotsMenuInfo.gametype = atoi(Info_ValueForKey(info, "g_gametype"));

	UI_AddBots_Cache();

	addBotsMenuInfo.numBots = UI_GetNumBots();
	count = addBotsMenuInfo.numBots < 7 ? addBotsMenuInfo.numBots : 7;

	addBotsMenuInfo.arrows.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.arrows.generic.name = ART_ARROWS;
	addBotsMenuInfo.arrows.generic.flags = QMF_INACTIVE;
	addBotsMenuInfo.arrows.generic.x = 200;
	addBotsMenuInfo.arrows.generic.y = 128;
	addBotsMenuInfo.arrows.width = 64;
	addBotsMenuInfo.arrows.height = 128;

	addBotsMenuInfo.up.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.up.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	addBotsMenuInfo.up.generic.x = 200;
	addBotsMenuInfo.up.generic.y = 128;
	addBotsMenuInfo.up.generic.id = ID_UP;
	addBotsMenuInfo.up.generic.callback = UI_AddBotsMenu_UpEvent;
	addBotsMenuInfo.up.width = 64;
	addBotsMenuInfo.up.height = 64;
	addBotsMenuInfo.up.focuspic = ART_ARROWUP;

	addBotsMenuInfo.down.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.down.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	addBotsMenuInfo.down.generic.x = 200;
	addBotsMenuInfo.down.generic.y = 128 + 64;
	addBotsMenuInfo.down.generic.id = ID_DOWN;
	addBotsMenuInfo.down.generic.callback = UI_AddBotsMenu_DownEvent;
	addBotsMenuInfo.down.width = 64;
	addBotsMenuInfo.down.height = 64;
	addBotsMenuInfo.down.focuspic = ART_ARROWDOWN;

	for (n = 0, y = 120; n < count; n++, y += 20) {
		addBotsMenuInfo.bots[n].generic.type = MTYPE_PTEXT;
		addBotsMenuInfo.bots[n].generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		addBotsMenuInfo.bots[n].generic.id = ID_BOTNAME0 + n;
		addBotsMenuInfo.bots[n].generic.x = 320 - 56;
		addBotsMenuInfo.bots[n].generic.y = y;
		addBotsMenuInfo.bots[n].generic.callback = UI_AddBotsMenu_BotEvent;
		addBotsMenuInfo.bots[n].string = addBotsMenuInfo.botnames[n];
		addBotsMenuInfo.bots[n].color = color_orange;
		addBotsMenuInfo.bots[n].style = UI_LEFT|UI_SMALLFONT;
	}

	y += 12;
	addBotsMenuInfo.skill_slider.generic.type = MTYPE_SLIDER;
	addBotsMenuInfo.skill_slider.generic.x = 320;
	addBotsMenuInfo.skill_slider.generic.y = y;
	addBotsMenuInfo.skill_slider.generic.name = "Skill:";
	addBotsMenuInfo.skill_slider.generic.callback = UI_AddBotsMenu_SkillChangeEvent;
	addBotsMenuInfo.skill_slider.generic.id = ID_SKILLSLIDER;
	addBotsMenuInfo.skill_slider.minvalue = 0.0;
	addBotsMenuInfo.skill_slider.maxvalue = 4.0;
	addBotsMenuInfo.skill_slider.curvalue = Com_Clamp(0, 4, (int)trap_Cvar_VariableValue("g_spSkill") - 1);

	y += SMALLCHAR_HEIGHT;
	addBotsMenuInfo.skill.generic.type = MTYPE_SPINCONTROL;
	addBotsMenuInfo.skill.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	addBotsMenuInfo.skill.generic.callback = UI_AddBotsMenu_SkillChangeEvent;
	addBotsMenuInfo.skill.generic.x = 320;
	addBotsMenuInfo.skill.generic.y = y;
	addBotsMenuInfo.skill.generic.id = ID_SKILL;
	addBotsMenuInfo.skill.itemnames = skillNames;
	addBotsMenuInfo.skill.curvalue = (int)addBotsMenuInfo.skill_slider.curvalue;

	y += SMALLCHAR_HEIGHT;
	addBotsMenuInfo.team.generic.type = MTYPE_SPINCONTROL;
	addBotsMenuInfo.team.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	addBotsMenuInfo.team.generic.callback = UI_AddBotsMenu_TeamEvent;
	addBotsMenuInfo.team.generic.x = 320;
	addBotsMenuInfo.team.generic.y = y;
	addBotsMenuInfo.team.generic.name = "Team:";
	addBotsMenuInfo.team.generic.id = ID_TEAM;

	if (addBotsMenuInfo.gametype > GT_TOURNAMENT) {
		addBotsMenuInfo.team.itemnames = teamNames2;
	} else {
		addBotsMenuInfo.team.itemnames = teamNames1;
		addBotsMenuInfo.team.generic.flags = QMF_GRAYED;
	}

	addBotsMenuInfo.go.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.go.generic.name = ART_FIGHT0;
	addBotsMenuInfo.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	addBotsMenuInfo.go.generic.id = ID_GO;
	addBotsMenuInfo.go.generic.callback = UI_AddBotsMenu_FightEvent;
	addBotsMenuInfo.go.generic.x = 320 + 128 - 128;
	addBotsMenuInfo.go.generic.y = 256 + 128 - 64;
	addBotsMenuInfo.go.width = 128;
	addBotsMenuInfo.go.height = 64;
	addBotsMenuInfo.go.focuspic = ART_FIGHT1;

	addBotsMenuInfo.back.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.back.generic.name = ART_BACK0;
	addBotsMenuInfo.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	addBotsMenuInfo.back.generic.id = ID_BACK;
	addBotsMenuInfo.back.generic.callback = UI_AddBotsMenu_BackEvent;
	addBotsMenuInfo.back.generic.x = 320 - 128;
	addBotsMenuInfo.back.generic.y = 256 + 128 - 64;
	addBotsMenuInfo.back.width = 128;
	addBotsMenuInfo.back.height = 64;
	addBotsMenuInfo.back.focuspic = ART_BACK1;

	addBotsMenuInfo.icon_hilite.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.icon_hilite.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_HIGHLIGHT;
	addBotsMenuInfo.icon_hilite.generic.x = 190 - 64 - 15; // 320 + 128 - 15;
	addBotsMenuInfo.icon_hilite.generic.y = 240 - 32 - 16;
	addBotsMenuInfo.icon_hilite.width = 128;
	addBotsMenuInfo.icon_hilite.height = 128;
	addBotsMenuInfo.icon_hilite.focuspic = ART_SELECT;
	addBotsMenuInfo.icon_hilite.focuscolor = color_red;

	addBotsMenuInfo.icon.generic.type = MTYPE_BITMAP;
	addBotsMenuInfo.icon.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	addBotsMenuInfo.icon.generic.name = addBotsMenuInfo.boticon;
	addBotsMenuInfo.icon.generic.x = 190 - 64; // 320 + 128;
	addBotsMenuInfo.icon.generic.y = 240 - 32;
	addBotsMenuInfo.icon.width = 64;
	addBotsMenuInfo.icon.height = 64;

	addBotsMenuInfo.baseBotNum = 0;
	addBotsMenuInfo.selectedBotNum = 0;
	addBotsMenuInfo.bots[0].color = color_white;

	UI_AddBotsMenu_GetSortedBotNums();
	UI_AddBotsMenu_SetBotNames();

	AddBots_SetBotIcon();

	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.arrows);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.up);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.down);

	for (n = 0; n < count; n++) {
		Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.bots[n]);
	}

	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.skill);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.skill_slider);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.team);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.go);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.back);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.icon_hilite);
	Menu_AddItem(&addBotsMenuInfo.menu, &addBotsMenuInfo.icon);
}

/*
=======================================================================================================================================
UI_AddBots_Cache
=======================================================================================================================================
*/
void UI_AddBots_Cache(void) {

	trap_R_RegisterShaderNoMip(ART_BACK0);
	trap_R_RegisterShaderNoMip(ART_BACK1);
	trap_R_RegisterShaderNoMip(ART_FIGHT0);
	trap_R_RegisterShaderNoMip(ART_FIGHT1);
	trap_R_RegisterShaderNoMip(ART_BACKGROUND);
	trap_R_RegisterShaderNoMip(ART_ARROWS);
	trap_R_RegisterShaderNoMip(ART_ARROWUP);
	trap_R_RegisterShaderNoMip(ART_ARROWDOWN);
	trap_R_RegisterShaderNoMip(ART_SELECT);
}

/*
=======================================================================================================================================
UI_AddBotsMenu
=======================================================================================================================================
*/
void UI_AddBotsMenu(void) {

	UI_AddBotsMenu_Init();
	UI_PushMenu(&addBotsMenuInfo.menu);
}
