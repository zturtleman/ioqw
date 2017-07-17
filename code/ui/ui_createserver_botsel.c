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
=======================================================================================================================================
*/

#include "ui_local.h"
#include "ui_startserver_q3.h"

/*
=======================================================================================================================================

	BOT SELECT MENU

=======================================================================================================================================
*/
#define BUTTON_WIDTH 96
#define BUTTON_HEIGHT 48

#define BOTSELECT_BACK0 "menu/art/back_0"
#define BOTSELECT_BACK1 "menu/art/back_1"
#define BOTSELECT_ACCEPT0 "menu/art/accept_0"
#define BOTSELECT_ACCEPT1 "menu/art/accept_1"
#define BOTSELECT_SELECT "menu/art/opponents_select"
#define BOTSELECT_SELECTED "menu/art/opponents_selected"
#define BOTSELECT_ARROWS "menu/art/arrows_horz_0"
#define BOTSELECT_ARROWSL "menu/art/arrows_horz_left"
#define BOTSELECT_ARROWSR "menu/art/arrows_horz_right"

enum {
	ID_BOTSELECT_VIEWLIST,
	ID_BOTSELECT_LEFT,
	ID_BOTSELECT_RIGHT,
	ID_BOTSELECT_BACK,
	ID_BOTSELECT_ACCEPT,
	ID_BOTSELECT_MULTISEL
};

#define BOTNAME_LENGTH 16

#define BOTGRID_COLS 4
#define BOTGRID_ROWS 4

#define MAX_GRIDMODELSPERPAGE (BOTGRID_ROWS * BOTGRID_COLS)

#define BOTLIST_COLS 3
#define BOTLIST_ROWS 12

#define MAX_LISTMODELSPERPAGE (BOTLIST_COLS * BOTLIST_ROWS)

#define BOTLIST_ICONSIZE 28
#define MAX_MULTISELECTED 15

#if (MAX_LISTMODELSPERPAGE > MAX_GRIDMODELSPERPAGE)
#define MAX_MODELSPERPAGE MAX_LISTMODELSPERPAGE
#else
#define MAX_MODELSPERPAGE MAX_GRIDMODELSPERPAGE
#endif
// holdover viewing method
//static qboolean botselRevisit = qfalse;
//static qboolean lastVisitViewList = qfalse;
//static qboolean useMultiSel = qfalse;

static vec4_t transparent_color = {1.0, 1.0, 1.0, 0.7};

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s picbuttons[MAX_GRIDMODELSPERPAGE];
	menubitmap_s arrows;
	menubitmap_s left;
	menubitmap_s right;
	menubitmap_s go;
	menubitmap_s back;
	menulist_s botlist; // only ever contains a screen full of information
	menuradiobutton_s viewlist;
	menuradiobutton_s multisel;
	int maxBotsPerPage;
	int index;
	int numBots;
	int page;
	int numpages;
	// most recently selected bot
	int selectedbot;
	int sortedBotNums[MAX_BOTS];
	char boticons[MAX_MODELSPERPAGE][MAX_QPATH];
	char botnames[MAX_MODELSPERPAGE][BOTNAME_LENGTH];
	int numMultiSel;
	int multiSel[MAX_MULTISELECTED];
	float *botcolor[MAX_MODELSPERPAGE];
	const char *botalias[MAX_MODELSPERPAGE];
} botMultiSelectInfo_t;

static botMultiSelectInfo_t botMultiSelectInfo;

/*
=======================================================================================================================================
UI_BotSelect_SortCompare
=======================================================================================================================================
*/
static int QDECL UI_BotSelect_SortCompare(const void *arg1, const void *arg2) {
	int num1, num2;
	const char *info1, *info2, *name1, *name2;

	num1 = *(int *)arg1;
	num2 = *(int *)arg2;

	info1 = UI_GetBotInfoByNumber(num1);
	info2 = UI_GetBotInfoByNumber(num2);

	name1 = Info_ValueForKey(info1, "name");
	name2 = Info_ValueForKey(info2, "name");

	return Q_stricmp(name1, name2);
}

/*
=======================================================================================================================================
UI_BotSelect_SelectedOnPage
=======================================================================================================================================
*/
static qboolean UI_BotSelect_SelectedOnPage(void) {
	int page;

	if (botMultiSelectInfo.selectedbot == -1) {
		return qfalse;
	}

	page = botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

	if (botMultiSelectInfo.selectedbot < page) {
		return qfalse;
	}

	if (botMultiSelectInfo.selectedbot >= page + botMultiSelectInfo.maxBotsPerPage) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
UI_BotSelect_SetBotInfoInCaller
=======================================================================================================================================
*/
static void UI_BotSelect_SetBotInfoInCaller(void) {
	char *info, *name;
	int index, sel, type;

	if (botMultiSelectInfo.multisel.curvalue) {
		index = botMultiSelectInfo.index;
		type = CreateServer_SlotTeam(index);

		if (type == SLOTTEAM_INVALID) {
			return;
		}

		for (sel = 0; sel < botMultiSelectInfo.numMultiSel; sel++, index++) {
			if (CreateServer_SlotTeam(index) != type) {
				break;
			}

			info = UI_GetBotInfoByNumber(botMultiSelectInfo.sortedBotNums[botMultiSelectInfo.multiSel[sel]]);
			name = Info_ValueForKey(info, "name");

			if (sel == 0) {
				CreateServer_SetNamedBot(index, name);
			} else {
				CreateServer_InsertNamedBot(index, name);
			}
		}
	} else {
		if (botMultiSelectInfo.selectedbot == -1) {
			return;
		}

		info = UI_GetBotInfoByNumber(botMultiSelectInfo.sortedBotNums[botMultiSelectInfo.selectedbot]);
		name = Info_ValueForKey(info, "name");

		CreateServer_SetNamedBot(botMultiSelectInfo.index, name);
	}
}

/*
=======================================================================================================================================
UI_BotSelect_AddBotSelection
=======================================================================================================================================
*/
static void UI_BotSelect_AddBotSelection(int bot) {
	int i, j;

	// single selection only
	if (!botMultiSelectInfo.multisel.curvalue) {
		if (botMultiSelectInfo.selectedbot == bot) {
			// toggle current selection
			botMultiSelectInfo.selectedbot = -1;
		} else {
			botMultiSelectInfo.selectedbot = bot;
		}

		return;
	}
	// check for presence in list already, and remove if found
	for (i = 0; i < botMultiSelectInfo.numMultiSel; i++) {
		if (botMultiSelectInfo.multiSel[i] == bot) {
			for (j = i; j < botMultiSelectInfo.numMultiSel - 1; j++) {
				botMultiSelectInfo.multiSel[j] = botMultiSelectInfo.multiSel[j + 1];
			}

			botMultiSelectInfo.numMultiSel--;
			return;
		}
	}
	// add to list, if enough space
	if (botMultiSelectInfo.numMultiSel == MAX_MULTISELECTED) {
		return;
	}

	botMultiSelectInfo.multiSel[botMultiSelectInfo.numMultiSel] = bot;
	botMultiSelectInfo.numMultiSel++;
	botMultiSelectInfo.selectedbot = bot;
}

/*
=======================================================================================================================================
UI_BotSelect_ToggleMultiSelect
=======================================================================================================================================
*/
static void UI_BotSelect_ToggleMultiSelect(void) {

	trap_Cvar_SetValue("ui_bot_multisel", botMultiSelectInfo.multisel.curvalue);

	if (!botMultiSelectInfo.multisel.curvalue) {
		// change to single sel
		if (botMultiSelectInfo.numMultiSel) {
			botMultiSelectInfo.selectedbot = botMultiSelectInfo.multiSel[0];
		} else {
			botMultiSelectInfo.selectedbot = -1;
		}

		return;
	}
	// change to multiple sel
	if (botMultiSelectInfo.selectedbot == -1) {
		botMultiSelectInfo.numMultiSel = 0;
		return;
	}

	if (botMultiSelectInfo.numMultiSel == 0) {
		UI_BotSelect_AddBotSelection(botMultiSelectInfo.selectedbot);
		return;
	}

	if (botMultiSelectInfo.selectedbot != botMultiSelectInfo.multiSel[0]) {
		botMultiSelectInfo.multiSel[0] = botMultiSelectInfo.selectedbot;
		botMultiSelectInfo.numMultiSel = 1;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_SetViewType
=======================================================================================================================================
*/
static void UI_BotSelect_SetViewType(void) {

	if (botMultiSelectInfo.viewlist.curvalue) {
		botMultiSelectInfo.maxBotsPerPage = MAX_LISTMODELSPERPAGE;
	} else {
		botMultiSelectInfo.maxBotsPerPage = MAX_GRIDMODELSPERPAGE;
	}

	botMultiSelectInfo.numpages = botMultiSelectInfo.numBots / botMultiSelectInfo.maxBotsPerPage;

	if (botMultiSelectInfo.numBots % botMultiSelectInfo.maxBotsPerPage) {
		botMultiSelectInfo.numpages++;
	}

	trap_Cvar_SetValue("ui_bot_list", botMultiSelectInfo.viewlist.curvalue);
}

/*
=======================================================================================================================================
UI_BotSelect_BuildList
=======================================================================================================================================
*/
static void UI_BotSelect_BuildList(void) {
	int n;

	botMultiSelectInfo.numBots = UI_GetNumBots();
	// initialize the array
	for (n = 0; n < botMultiSelectInfo.numBots; n++) {
		botMultiSelectInfo.sortedBotNums[n] = n;
	}
	// now sort it
	qsort(botMultiSelectInfo.sortedBotNums, botMultiSelectInfo.numBots, sizeof(botMultiSelectInfo.sortedBotNums[0]), UI_BotSelect_SortCompare);
}

/*
=======================================================================================================================================
UI_ServerPlayerIcon
=======================================================================================================================================
*/
void UI_ServerPlayerIcon(const char *modelAndSkin, char *iconName, int iconNameMaxSize) {
	char *skin, model[MAX_QPATH];

	Q_strncpyz(model, modelAndSkin, sizeof(model));
	skin = strrchr(model, '/');

	if (skin) {
		*skin++ = '\0';
	} else {
		skin = "default";
	}

	Com_sprintf(iconName, iconNameMaxSize, "models/players/%s/icon_%s.tga", model, skin);

	if (!trap_R_RegisterShaderNoMip(iconName) && Q_stricmp(skin, "default") != 0) {
		Com_sprintf(iconName, iconNameMaxSize, "models/players/%s/icon_default.tga", model);
	}
}

/*
=======================================================================================================================================
UI_BotSelect_UpdateGridInterface
=======================================================================================================================================
*/
static void UI_BotSelect_UpdateGridInterface(void) {
	int i, j, page, sel;

	// clear out old values
	j = botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

	for (i = 0; i < MAX_GRIDMODELSPERPAGE; i++, j++) {
		botMultiSelectInfo.picbuttons[i].generic.flags &= ~QMF_HIDDEN;

		if (j < botMultiSelectInfo.numBots) {
			botMultiSelectInfo.picbuttons[i].generic.flags &= ~QMF_INACTIVE;
			botMultiSelectInfo.picbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
		} else {
			// dead control
			botMultiSelectInfo.picbuttons[i].generic.flags |= QMF_INACTIVE;
		}

		botMultiSelectInfo.picbuttons[i].generic.flags &= ~QMF_HIGHLIGHT;
	}
	// set selected model(s), if visible
	if (botMultiSelectInfo.multisel.curvalue) {
		for (i = 0; i < botMultiSelectInfo.numMultiSel; i++) {
			sel = botMultiSelectInfo.multiSel[i];
			page = sel / botMultiSelectInfo.maxBotsPerPage;

			if (botMultiSelectInfo.page != page) {
				continue;
			}

			sel %= botMultiSelectInfo.maxBotsPerPage;
			botMultiSelectInfo.picbuttons[sel].generic.flags |= QMF_HIGHLIGHT;
			botMultiSelectInfo.picbuttons[sel].generic.flags &= ~QMF_PULSEIFFOCUS;
		}
	} else {
		if (botMultiSelectInfo.selectedbot == -1) {
			return;
		}

		page = botMultiSelectInfo.selectedbot / botMultiSelectInfo.maxBotsPerPage;

		if (botMultiSelectInfo.page == page) {
			i = botMultiSelectInfo.selectedbot % botMultiSelectInfo.maxBotsPerPage;
			botMultiSelectInfo.picbuttons[i].generic.flags |= QMF_HIGHLIGHT;
			botMultiSelectInfo.picbuttons[i].generic.flags &= ~QMF_PULSEIFFOCUS;
		}
	}
}

/*
=======================================================================================================================================
UI_BotSelect_CheckAcceptButton
=======================================================================================================================================
*/
static void UI_BotSelect_CheckAcceptButton(void) {
	qboolean enable;

	enable = qfalse;

	if (botMultiSelectInfo.multisel.curvalue) {
		if (botMultiSelectInfo.numMultiSel > 0) {
			enable = qtrue;
		}
	} else {
		if (botMultiSelectInfo.selectedbot != -1) {
			enable = qtrue;
		}
	}

	if (enable) {
		botMultiSelectInfo.go.generic.flags &= ~(QMF_INACTIVE|QMF_GRAYED);
	} else {
		botMultiSelectInfo.go.generic.flags |= (QMF_INACTIVE|QMF_GRAYED);
	}
}

/*
=======================================================================================================================================
UI_BotSelect_UpdateView

The display acts as a "window" on the list of all bots, and only ever contains one screens worth of information.
=======================================================================================================================================
*/
static void UI_BotSelect_UpdateView(void) {
	const char *info;
	int i, j, pageBotCount;

	j = botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

	for (i = 0; i < botMultiSelectInfo.maxBotsPerPage; i++, j++) {
		if (j < botMultiSelectInfo.numBots) {
			info = UI_GetBotInfoByNumber(botMultiSelectInfo.sortedBotNums[j]);
			UI_ServerPlayerIcon(Info_ValueForKey(info, "model"), botMultiSelectInfo.boticons[i], MAX_QPATH);
			Q_strncpyz(botMultiSelectInfo.botnames[i], Info_ValueForKey(info, "name"), BOTNAME_LENGTH);
			Q_CleanStr(botMultiSelectInfo.botnames[i]);

			if (botMultiSelectInfo.index != -1 && CreateServer_BotOnSelectionList(botMultiSelectInfo.botnames[i])) {
				botMultiSelectInfo.botcolor[i] = color_red;
			} else {
				botMultiSelectInfo.botcolor[i] = color_orange;
			}
		} else {
			// dead slot
			botMultiSelectInfo.botnames[i][0] = 0;
		}
	}
	// update display details based on the view type
	pageBotCount = botMultiSelectInfo.numBots - botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;;

	if (pageBotCount > botMultiSelectInfo.maxBotsPerPage) {
		pageBotCount = botMultiSelectInfo.maxBotsPerPage;
	}

	if (!botMultiSelectInfo.viewlist.curvalue) {
		// grid display
		UI_BotSelect_UpdateGridInterface();
		botMultiSelectInfo.botlist.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	} else {
		// list display
		for (i = 0; i < MAX_GRIDMODELSPERPAGE; i++) {
			botMultiSelectInfo.picbuttons[i].generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
		}

		botMultiSelectInfo.botlist.generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);

		if (UI_BotSelect_SelectedOnPage()) {
			botMultiSelectInfo.botlist.curvalue = botMultiSelectInfo.selectedbot % botMultiSelectInfo.maxBotsPerPage;
		} else {
			botMultiSelectInfo.botlist.curvalue = -1;
		}

		botMultiSelectInfo.botlist.numitems = pageBotCount;
	}
	// left/right controls
	if (botMultiSelectInfo.numpages > 1) {
		if (botMultiSelectInfo.page > 0) {
			botMultiSelectInfo.left.generic.flags &= ~QMF_INACTIVE;
		} else {
			botMultiSelectInfo.left.generic.flags |= QMF_INACTIVE;
		}

		if (botMultiSelectInfo.page < (botMultiSelectInfo.numpages - 1)) {
			botMultiSelectInfo.right.generic.flags &= ~QMF_INACTIVE;
		} else {
			botMultiSelectInfo.right.generic.flags |= QMF_INACTIVE;
		}
	} else {
		// hide left/right markers
		botMultiSelectInfo.left.generic.flags |= QMF_INACTIVE;
		botMultiSelectInfo.right.generic.flags |= QMF_INACTIVE;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_SetPageFromSelected
=======================================================================================================================================
*/
static void UI_BotSelect_SetPageFromSelected(void) {

	if (botMultiSelectInfo.selectedbot == -1) {
		botMultiSelectInfo.page = 0;
		return;
	}

	botMultiSelectInfo.page = botMultiSelectInfo.selectedbot / botMultiSelectInfo.maxBotsPerPage;
}

/*
=======================================================================================================================================
UI_BotSelect_Default
=======================================================================================================================================
*/
static void UI_BotSelect_Default(char *bot) {
	const char *botInfo, *test;
	int n, i;

	botMultiSelectInfo.selectedbot = -1;
	botMultiSelectInfo.page = 0;
	botMultiSelectInfo.numMultiSel = 0;

	for (n = 0; n < botMultiSelectInfo.numBots; n++) {
		botInfo = UI_GetBotInfoByNumber(n);
		test = Info_ValueForKey(botInfo, "name");

		if (Q_stricmp(bot, test) == 0) {
			break;
		}
	}
	// bot name not in list
	if (n == botMultiSelectInfo.numBots) {
		return;
	}
	// find in sorted list
	for (i = 0; i < botMultiSelectInfo.numBots; i++) {
		if (botMultiSelectInfo.sortedBotNums[i] == n) {
			break;
		}
	}
	// not in sorted list
	if (i == botMultiSelectInfo.numBots) {
		return;
	}
	// found it!
	UI_BotSelect_AddBotSelection(i);
	UI_BotSelect_SetPageFromSelected();
}

/*
=======================================================================================================================================
UI_BotSelect_BotEvent
=======================================================================================================================================
*/
static void UI_BotSelect_BotEvent(void *ptr, int event) {
	int i;

	if (event != QM_ACTIVATED) {
		return;
	}

	i = ((menucommon_s *)ptr)->id;
	i += botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

	UI_BotSelect_AddBotSelection(i);
	UI_BotSelect_CheckAcceptButton();
	UI_BotSelect_UpdateGridInterface();
}

/*
=======================================================================================================================================
UI_BotSelect_Event
=======================================================================================================================================
*/
static void UI_BotSelect_Event(void *ptr, int event) {

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_BOTSELECT_VIEWLIST:
			UI_BotSelect_SetViewType();
			UI_BotSelect_SetPageFromSelected();
			UI_BotSelect_UpdateView();
			break;
		case ID_BOTSELECT_MULTISEL:
			UI_BotSelect_ToggleMultiSelect();
			UI_BotSelect_UpdateView();
			break;
		case ID_BOTSELECT_LEFT:
			if (botMultiSelectInfo.page > 0) {
				botMultiSelectInfo.page--;
				UI_BotSelect_UpdateView();
			}

			break;
		case ID_BOTSELECT_RIGHT:
			if (botMultiSelectInfo.page < botMultiSelectInfo.numpages - 1) {
				botMultiSelectInfo.page++;
				UI_BotSelect_UpdateView();
			}

			break;
		case ID_BOTSELECT_ACCEPT:
			UI_PopMenu();

			if (botMultiSelectInfo.index != -1) {
				UI_BotSelect_SetBotInfoInCaller();
			}

			break;
		case ID_BOTSELECT_BACK:
			UI_PopMenu();
			break;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_Cache
=======================================================================================================================================
*/
void UI_BotSelect_Cache(void) {

	trap_R_RegisterShaderNoMip(BOTSELECT_BACK0);
	trap_R_RegisterShaderNoMip(BOTSELECT_BACK1);
	trap_R_RegisterShaderNoMip(BOTSELECT_ACCEPT0);
	trap_R_RegisterShaderNoMip(BOTSELECT_ACCEPT1);
	trap_R_RegisterShaderNoMip(BOTSELECT_SELECT);
	trap_R_RegisterShaderNoMip(BOTSELECT_SELECTED);
	trap_R_RegisterShaderNoMip(BOTSELECT_ARROWS);
	trap_R_RegisterShaderNoMip(BOTSELECT_ARROWSL);
	trap_R_RegisterShaderNoMip(BOTSELECT_ARROWSR);
}

/*
=======================================================================================================================================
UI_BotSelect_ScrollList_LineSize
=======================================================================================================================================
*/
static void UI_BotSelect_ScrollList_LineSize(int *charheight, int *charwidth, int *lineheight) {
	float scale;

	scale = UI_ProportionalSizeScale(UI_SMALLFONT);

	*charwidth = scale * UI_ProportionalStringWidth("X");
	*charheight = scale * PROP_HEIGHT;

	if (*charheight > BOTLIST_ICONSIZE) {
		*lineheight = *charheight + 2;
	} else {
		*lineheight = BOTLIST_ICONSIZE + 2;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_HandleListKey

Returns true if the list box accepts that key input. Implements the list box input, but with larger proportional fonts.
=======================================================================================================================================
*/
static qboolean UI_BotSelect_HandleListKey(int key, sfxHandle_t *psfx) {
	int x, y, w, cursorx, cursory, column, index, charwidth, charheight, lineheight, sel;
	menulist_s *l;

	UI_BotSelect_ScrollList_LineSize(&charheight, &charwidth, &lineheight);

	l = &botMultiSelectInfo.botlist;

	switch (key) {
		case K_MOUSE1:
			if (l->generic.flags & QMF_HASMOUSEFOCUS) {
				// check scroll region
				x = l->generic.x;
				y = l->generic.y;
				w = ((l->width + l->seperation) * l->columns - l->seperation) * charwidth;

				if (l->generic.flags & QMF_CENTER_JUSTIFY) {
					x -= w / 2;
				}

				if (UI_CursorInRect(x, y, w, l->height * lineheight)) {
					cursorx = (uis.cursorx - x) / charwidth;
					column = cursorx / (l->width + l->seperation);
					cursory = (uis.cursory - y) / lineheight;
					index = column * l->height + cursory;

					if (l->top + index < l->numitems) {
						l->oldvalue = l->curvalue;
						l->curvalue = l->top + index;

						if (l->oldvalue != l->curvalue) {
							if (l->generic.callback) {
								l->generic.callback(l, QM_GOTFOCUS);
							}
						}

						sel = botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;
						sel += l->curvalue;

						UI_BotSelect_AddBotSelection(sel);
						UI_BotSelect_CheckAcceptButton();

						*psfx = (menu_move_sound);
					}
				} else {
					// absorbed, silent sound effect
					*psfx = (menu_null_sound);
				}
			}

			return qtrue;
		// keys that have the default action
		case K_ESCAPE:
			*psfx = Menu_DefaultKey(&botMultiSelectInfo.menu, key);
			return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
UI_BotSelect_Key
=======================================================================================================================================
*/
static sfxHandle_t UI_BotSelect_Key(int key) {
	menulist_s *l;
	sfxHandle_t sfx;

	l = (menulist_s *)Menu_ItemAtCursor(&botMultiSelectInfo.menu);
	sfx = menu_null_sound;

	if (l == &botMultiSelectInfo.botlist) {
		if (!UI_BotSelect_HandleListKey(key, &sfx)) {
			return menu_buzz_sound;
		}
	} else {
		sfx = Menu_DefaultKey(&botMultiSelectInfo.menu, key);
	}

	return sfx;
}

/*
=======================================================================================================================================
UI_BotSelect_ScrollList_Init
=======================================================================================================================================
*/
static void UI_BotSelect_ScrollList_Init(menulist_s *l) {
	int w, charwidth, charheight, lineheight;

	l->oldvalue = 0;
	l->curvalue = 0;
	l->top = 0;

	UI_BotSelect_ScrollList_LineSize(&charheight, &charwidth, &lineheight);

	if (!l->columns) {
		l->columns = 1;
		l->seperation = 0;
	} else if (!l->seperation) {
		l->seperation = 3;
	}

	w = ((l->width + l->seperation) * l->columns - l->seperation) * charwidth;

	l->generic.left = l->generic.x;
	l->generic.top = l->generic.y;
	l->generic.right = l->generic.x + w;
	l->generic.bottom = l->generic.y + l->height * lineheight;

	if (l->generic.flags & QMF_CENTER_JUSTIFY) {
		l->generic.left -= w / 2;
		l->generic.right -= w / 2;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_ScrollListDraw
=======================================================================================================================================
*/
static void UI_BotSelect_ScrollListDraw(void *ptr) {
	int x, y, i, j, base, column, style, charwidth, charheight, lineheight, index, bot;
	float *color;
	qboolean hasfocus, selected;
	menulist_s *l;

	UI_BotSelect_ScrollList_LineSize(&charheight, &charwidth, &lineheight);

	l = (menulist_s *)ptr;
	hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);
	x = l->generic.x;

	for (column = 0; column < l->columns; column++) {
		y = l->generic.y;
		base = l->top + column * l->height;

		for (i = base; i < base + l->height; i++) {
			if (i >= l->numitems) {
				break;
			}

			style = UI_SMALLFONT;
			color = botMultiSelectInfo.botcolor[i];

			if (i == l->curvalue) {
				UI_FillRect(x, y + (lineheight - BOTLIST_ICONSIZE) / 2, l->width * charwidth, BOTLIST_ICONSIZE, listbar_color);

				if (color != color_red) {
					color = text_color_highlight;
				}

				if (hasfocus) {
					style |= (UI_PULSE|UI_LEFT);
				} else {
					style |= UI_LEFT;
				}
			} else {
				style |= UI_LEFT|UI_INVERSE;
			}

			index = -1;
			selected = qfalse;
			bot = i + botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

			if (botMultiSelectInfo.multisel.curvalue) {
				for (j = 0; j < botMultiSelectInfo.numMultiSel; j++) {
					if (botMultiSelectInfo.multiSel[j] == bot) {
						index = j + 1;
						selected = qtrue;
						break;
					}
				}
			} else {
				if (botMultiSelectInfo.selectedbot == bot) {
					selected = qtrue;
				}
			}

			trap_R_SetColor(transparent_color);
			UI_DrawNamedPic(x, y + (lineheight - BOTLIST_ICONSIZE) / 2, BOTLIST_ICONSIZE, BOTLIST_ICONSIZE, botMultiSelectInfo.boticons[i]);
			trap_R_SetColor(NULL);

			if (selected) {
				trap_R_SetColor(colorRed);
				UI_DrawNamedPic(x, y + (lineheight - BOTLIST_ICONSIZE) / 2, BOTLIST_ICONSIZE, BOTLIST_ICONSIZE, BOTSELECT_SELECTED);
				trap_R_SetColor(NULL);
			}

			if (index != -1) {
				UI_DrawString(x + BOTLIST_ICONSIZE / 2, y + (lineheight - BIGCHAR_HEIGHT) / 2, va("%i", index), UI_CENTER|UI_DROPSHADOW, color_white);
			}

			UI_DrawProportionalString(x + BOTLIST_ICONSIZE + 2, y + (lineheight - charheight) / 2, l->itemnames[i], style, color);

			y += lineheight;
		}

		x += (l->width + l->seperation) * charwidth;
	}
}

/*
=======================================================================================================================================
UI_BotSelect_BotGridDraw
=======================================================================================================================================
*/
static void UI_BotSelect_BotGridDraw(void *ptr) {
	float x, y, w, h, *color;
	vec4_t tempcolor;
	int index, i, bot;
	menubitmap_s *b;

	b = (menubitmap_s *)ptr;
	// draw bot icon
	index = b->generic.id;
	x = b->generic.left;
	y = b->generic.top;
	w = b->generic.right - x;
	h = b->generic.bottom - y;

	if (botMultiSelectInfo.botnames[index][0]) {
		UI_DrawNamedPic(x, y, w, h, botMultiSelectInfo.boticons[index]);

		if (b->generic.flags & QMF_HIGHLIGHT) {
			trap_R_SetColor(color_red);
			UI_DrawNamedPic(x, y, w, h, BOTSELECT_SELECTED);
			trap_R_SetColor(NULL);
		}
	}
	// draw bot position in multi
	if (botMultiSelectInfo.multisel.curvalue) {
		bot = index + botMultiSelectInfo.page * botMultiSelectInfo.maxBotsPerPage;

		for (i = 0; i < botMultiSelectInfo.numMultiSel; i++) {
			if (botMultiSelectInfo.multiSel[i] != bot) {
				continue;
			}

			UI_DrawString(x + w / 2, y + (h - GIANTCHAR_HEIGHT) / 2, va("%i", i + 1), UI_CENTER|UI_GIANTFONT|UI_DROPSHADOW, color_white);
			break;
		}
	}
	// draw bot name text
	if (botMultiSelectInfo.botnames[index][0]) {
		UI_DrawString(x + 32, y + 64, botMultiSelectInfo.botnames[index], UI_CENTER|UI_SMALLFONT, botMultiSelectInfo.botcolor[index]);
	}
	// draws pulse shader showing mouse over

	// no pulse shader required
	if (b->generic.flags & QMF_HIGHLIGHT) {
		return;
	}

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h = b->height;

	if (b->generic.flags & QMF_RIGHT_JUSTIFY) {
		x = x - w;
	} else if (b->generic.flags & QMF_CENTER_JUSTIFY) {
		x = x - w / 2;
	}
	// used to refresh shader
	if (b->generic.name && !b->shader) {
		b->shader = trap_R_RegisterShaderNoMip(b->generic.name);

		if (!b->shader && b->errorpic) {
			b->shader = trap_R_RegisterShaderNoMip(b->errorpic);
		}
	}

	if (b->focuspic && !b->focusshader) {
		b->focusshader = trap_R_RegisterShaderNoMip(b->focuspic);
	}

	if (b->generic.flags & QMF_GRAYED) {
		if (b->shader) {
			trap_R_SetColor(colorMdGrey);
			UI_DrawHandlePic(x, y, w, h, b->shader);
			trap_R_SetColor(NULL);
		}
	} else {
		if (b->shader) {
			UI_DrawHandlePic(x, y, w, h, b->shader);
		}

		if ((b->generic.flags & QMF_PULSE) || ((b->generic.flags & QMF_PULSEIFFOCUS) && (Menu_ItemAtCursor(b->generic.parent) == b))) {
			if (b->focuscolor) {
				tempcolor[0] = b->focuscolor[0];
				tempcolor[1] = b->focuscolor[1];
				tempcolor[2] = b->focuscolor[2];
				color = tempcolor;
			} else {
				color = pulse_color;
			}

			color[3] = 0.5 + 0.5 * sin(uis.realtime / PULSE_DIVISOR);

			trap_R_SetColor(color);
			UI_DrawHandlePic(x, y, w, h, b->focusshader);
			trap_R_SetColor(NULL);
		} else if ((b->generic.flags & QMF_HIGHLIGHT) || ((b->generic.flags & QMF_HIGHLIGHT_IF_FOCUS) && (Menu_ItemAtCursor(b->generic.parent) == b))) {
			if (b->focuscolor) {
				trap_R_SetColor(b->focuscolor);
				UI_DrawHandlePic(x, y, w, h, b->focusshader);
				trap_R_SetColor(NULL);
			} else {
				UI_DrawHandlePic(x, y, w, h, b->focusshader);
			}
		}
	}
}

/*
=======================================================================================================================================
UI_BotSelect_MenuDraw
=======================================================================================================================================
*/
static void UI_BotSelect_MenuDraw(void) {

//	UI_DrawString(0, 0, va("%i", botMultiSelectInfo.selectedbot), UI_SMALLFONT, color_white);
	// draw the controls
	Menu_Draw(&botMultiSelectInfo.menu);
}

/*
=======================================================================================================================================
UI_BotMultiSelect_Init
=======================================================================================================================================
*/
static void UI_BotMultiSelect_Init(char *bot, int index) {
	int i, j, k, x, y;

	memset(&botMultiSelectInfo, 0, sizeof(botMultiSelectInfo));

	botMultiSelectInfo.menu.key = UI_BotSelect_Key;
	botMultiSelectInfo.menu.wrapAround = qtrue;
	botMultiSelectInfo.menu.fullscreen = qtrue;
	botMultiSelectInfo.menu.draw = UI_BotSelect_MenuDraw;

	UI_BotSelect_Cache();

	botMultiSelectInfo.index = index;
	botMultiSelectInfo.numMultiSel = 0;

	botMultiSelectInfo.banner.generic.type = MTYPE_BTEXT;
	botMultiSelectInfo.banner.generic.x = 320;
	botMultiSelectInfo.banner.generic.y = 16;
	botMultiSelectInfo.banner.string = "SELECT BOT";
	botMultiSelectInfo.banner.color = color_white;
	botMultiSelectInfo.banner.style = UI_CENTER;

	botMultiSelectInfo.viewlist.generic.type = MTYPE_RADIOBUTTON;
	botMultiSelectInfo.viewlist.generic.name = "View list:";
	botMultiSelectInfo.viewlist.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	botMultiSelectInfo.viewlist.generic.id = ID_BOTSELECT_VIEWLIST;
	botMultiSelectInfo.viewlist.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.viewlist.generic.x = 320 - 13 * SMALLCHAR_WIDTH;
	botMultiSelectInfo.viewlist.generic.y = 495 - 2 * LINE_HEIGHT;

	botMultiSelectInfo.multisel.generic.type = MTYPE_RADIOBUTTON;
	botMultiSelectInfo.multisel.generic.name = "Multi-sel:";
	botMultiSelectInfo.multisel.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	botMultiSelectInfo.multisel.generic.id = ID_BOTSELECT_MULTISEL;
	botMultiSelectInfo.multisel.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.multisel.generic.x = 320 - 13 * SMALLCHAR_WIDTH;
	botMultiSelectInfo.multisel.generic.y = 497 - 3 * LINE_HEIGHT;
	// init based on previous value
	botMultiSelectInfo.viewlist.curvalue = (int)Com_Clamp(0, 1, trap_Cvar_VariableValue("ui_bot_list"));
	botMultiSelectInfo.multisel.curvalue = (int)Com_Clamp(0, 1, trap_Cvar_VariableValue("ui_bot_multisel"));

	for (i = 0; i < MAX_MODELSPERPAGE; i++) {
		botMultiSelectInfo.botalias[i] = botMultiSelectInfo.botnames[i];
	}

	botMultiSelectInfo.botlist.generic.type = MTYPE_SCROLLLIST;
	botMultiSelectInfo.botlist.generic.flags = QMF_PULSEIFFOCUS|QMF_NODEFAULTINIT;
	botMultiSelectInfo.botlist.generic.x = 21;
	botMultiSelectInfo.botlist.generic.y = 60;
	botMultiSelectInfo.botlist.generic.ownerdraw = UI_BotSelect_ScrollListDraw;
	botMultiSelectInfo.botlist.columns = BOTLIST_COLS;
	botMultiSelectInfo.botlist.seperation = 2;
	botMultiSelectInfo.botlist.height = BOTLIST_ROWS;
	botMultiSelectInfo.botlist.width = 14;
	botMultiSelectInfo.botlist.itemnames = botMultiSelectInfo.botalias;

	UI_BotSelect_ScrollList_Init(&botMultiSelectInfo.botlist);

	y = 80;

	for (i = 0, k = 0; i < BOTGRID_ROWS; i++) {
		x = 180;

		for (j = 0; j < BOTGRID_COLS; j++, k++) {
			botMultiSelectInfo.picbuttons[k].generic.type = MTYPE_BITMAP;
			botMultiSelectInfo.picbuttons[k].generic.flags = QMF_LEFT_JUSTIFY|QMF_NODEFAULTINIT|QMF_PULSEIFFOCUS;
			botMultiSelectInfo.picbuttons[k].generic.callback = UI_BotSelect_BotEvent;
			botMultiSelectInfo.picbuttons[k].generic.ownerdraw = UI_BotSelect_BotGridDraw;
			botMultiSelectInfo.picbuttons[k].generic.id = k;
			botMultiSelectInfo.picbuttons[k].generic.x = x - 16;
			botMultiSelectInfo.picbuttons[k].generic.y = y - 16;
			botMultiSelectInfo.picbuttons[k].generic.left = x;
			botMultiSelectInfo.picbuttons[k].generic.top = y;
			botMultiSelectInfo.picbuttons[k].generic.right = x + 64;
			botMultiSelectInfo.picbuttons[k].generic.bottom = y + 64;
			botMultiSelectInfo.picbuttons[k].width = 128;
			botMultiSelectInfo.picbuttons[k].height = 128;
			botMultiSelectInfo.picbuttons[k].focuspic = BOTSELECT_SELECT;
			botMultiSelectInfo.picbuttons[k].focuscolor = colorRed;

			x += (64 + 6);
		}

		y += (64 + SMALLCHAR_HEIGHT + 6);
	}

	botMultiSelectInfo.arrows.generic.type = MTYPE_BITMAP;
	botMultiSelectInfo.arrows.generic.name = BOTSELECT_ARROWS;
	botMultiSelectInfo.arrows.generic.flags = QMF_INACTIVE;
	botMultiSelectInfo.arrows.generic.x = 320;
	botMultiSelectInfo.arrows.generic.y = 435;
	botMultiSelectInfo.arrows.width = 192;
	botMultiSelectInfo.arrows.height = 64;

	botMultiSelectInfo.left.generic.type = MTYPE_BITMAP;
	botMultiSelectInfo.left.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botMultiSelectInfo.left.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.left.generic.id = ID_BOTSELECT_LEFT;
	botMultiSelectInfo.left.generic.x = 320;
	botMultiSelectInfo.left.generic.y = 435;
	botMultiSelectInfo.left.width = 96;
	botMultiSelectInfo.left.height = 64;
	botMultiSelectInfo.left.focuspic = BOTSELECT_ARROWSL;

	botMultiSelectInfo.right.generic.type = MTYPE_BITMAP;
	botMultiSelectInfo.right.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botMultiSelectInfo.right.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.right.generic.id = ID_BOTSELECT_RIGHT;
	botMultiSelectInfo.right.generic.x = 320 + 96;
	botMultiSelectInfo.right.generic.y = 435;
	botMultiSelectInfo.right.width = 96;
	botMultiSelectInfo.right.height = 64;
	botMultiSelectInfo.right.focuspic = BOTSELECT_ARROWSR;

	botMultiSelectInfo.back.generic.type = MTYPE_BITMAP;
	botMultiSelectInfo.back.generic.name = BOTSELECT_BACK0;
	botMultiSelectInfo.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botMultiSelectInfo.back.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.back.generic.id = ID_BOTSELECT_BACK;
	botMultiSelectInfo.back.generic.x = 0;
	botMultiSelectInfo.back.generic.y = 435;
	botMultiSelectInfo.back.width = BUTTON_WIDTH;
	botMultiSelectInfo.back.height = BUTTON_HEIGHT;
	botMultiSelectInfo.back.focuspic = BOTSELECT_BACK1;

	botMultiSelectInfo.go.generic.type = MTYPE_BITMAP;
	botMultiSelectInfo.go.generic.name = BOTSELECT_ACCEPT0;
	botMultiSelectInfo.go.generic.flags = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	botMultiSelectInfo.go.generic.callback = UI_BotSelect_Event;
	botMultiSelectInfo.go.generic.id = ID_BOTSELECT_ACCEPT;
	botMultiSelectInfo.go.generic.x = 640;
	botMultiSelectInfo.go.generic.y = 435;
	botMultiSelectInfo.go.width = BUTTON_WIDTH;
	botMultiSelectInfo.go.height = BUTTON_HEIGHT;
	botMultiSelectInfo.go.focuspic = BOTSELECT_ACCEPT1;

	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.banner);

	for (i = 0; i < MAX_GRIDMODELSPERPAGE; i++) {
		Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.picbuttons[i]);
	}

	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.arrows);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.left);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.right);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.back);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.go);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.viewlist);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.multisel);
	Menu_AddItem(&botMultiSelectInfo.menu, &botMultiSelectInfo.botlist);

	UI_BotSelect_BuildList();
	UI_BotSelect_SetViewType();
	UI_BotSelect_Default(bot);
	UI_BotSelect_UpdateView();
	UI_BotSelect_CheckAcceptButton();
}

/*
=======================================================================================================================================
UI_BotSelect_Index
=======================================================================================================================================
*/
void UI_BotSelect_Index(char *bot, int index) {

	UI_BotMultiSelect_Init(bot, index);
	UI_PushMenu(&botMultiSelectInfo.menu);
}

/*
=======================================================================================================================================
UI_BotMultiSelectMenu
=======================================================================================================================================
*/
void UI_BotMultiSelectMenu(char *bot) {

	UI_BotMultiSelect_Init(bot, -1);
	UI_PushMenu(&botMultiSelectInfo.menu);
}
