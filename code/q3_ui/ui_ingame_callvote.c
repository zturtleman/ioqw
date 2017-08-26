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

#define VOTEMENU_BACK0 "menu/art/back_0"
#define VOTEMENU_BACK1 "menu/art/back_1"
#define ART_FIGHT0 "menu/art/accept_0"
#define ART_FIGHT1 "menu/art/accept_1"
#define ART_BACKGROUND "menu/art/addbotframe"

static char *callvotemenu_artlist[] = {
	VOTEMENU_BACK0,
	VOTEMENU_BACK1,
	ART_FIGHT0,
	ART_FIGHT1,
	NULL
};

enum {
	ID_BACK,
	ID_GO,
	ID_RESTART,
	ID_NEXTMAP,
	ID_CHANGEMAP,
	ID_GAMETYPE,
	ID_LIMITS,
	ID_TIMELIMIT,
	ID_DOWARMUP,
	ID_KICK
};
// this sorta dependend on number of vote options
#define VOTEMENU_MENU_VERTICAL_SPACING 22

typedef struct {
	menuframework_s menu;
	menutext_s banner;
	menubitmap_s back;
	menubitmap_s go;
	qboolean g_doWarmupEnabled;
	// buttons
	menutext_s bMapRestart;
	menutext_s bNextmap;
	menutext_s bMap;
	menutext_s bGametype;
	menutext_s bFraglimit;
	menutext_s bCapturelimit;
	menutext_s bTimelimit;
	menutext_s bDoWarmup;
	menutext_s bKick;
} callvotemenu_t;

static callvotemenu_t s_callvotemenu;

void UI_CallVoteMenuInternal(void);

/*
=======================================================================================================================================
CallVoteMenu_Event
=======================================================================================================================================
*/
static void CallVoteMenu_Event(void *ptr, int event) {
	int gametype;

	gametype = UI_ServerGametype();

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
		case ID_BACK:
			if (event != QM_ACTIVATED) {
				return;
			}

			UI_PopMenu();
			break;
		case ID_RESTART:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote map_restart");
			UI_PopMenu();
			break;
		case ID_NEXTMAP:
			trap_Cmd_ExecuteText(EXEC_APPEND, "callvote nextmap");
			UI_PopMenu();
			break;
		case ID_CHANGEMAP:
			UI_VoteMapMenu();
			// Don't pop! It will do a double pop if successfull
			break;
		case ID_GAMETYPE:
			UI_VoteGametypeMenu();
			// Don't pop! It will do a double pop if successfull
			break;
		case ID_LIMITS:
			if (gametype < GT_CTF) {
				UI_VoteFraglimitMenu();
				// Don't pop! It will do a double pop if successfull
			} else {
				UI_VoteCapturelimitMenu();
				// Don't pop! It will do a double pop if successfull
			}

			break;
		case ID_TIMELIMIT:
			UI_VoteTimelimitMenu();
			// Don't pop! It will do a double pop if successfull
			break;
		case ID_DOWARMUP:
			if (s_callvotemenu.g_doWarmupEnabled) {
				trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_doWarmup 0");
			} else {
				trap_Cmd_ExecuteText(EXEC_APPEND, "callvote g_doWarmup 1");
			}

			UI_PopMenu();
			break;
		case ID_KICK:
			UI_VoteKickMenu();
			// Don't pop! It will do a double pop if successfull
			break;
	}
}

/*
=======================================================================================================================================
CallVoteMenu_Cache
=======================================================================================================================================
*/
static void CallVoteMenu_Cache(void) {
	int i;

	// touch all our pics
	for (i = 0;; i++) {
		if (!callvotemenu_artlist[i]) {
			break;
		}

		trap_R_RegisterShaderNoMip(callvotemenu_artlist[i]);
	}
}

/*
=======================================================================================================================================
UI_CallVoteMenu_Draw
=======================================================================================================================================
*/
static void UI_CallVoteMenu_Draw(void) {

	UI_DrawBannerString(320, 16, "CALL VOTE", UI_CENTER, color_white);
	UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
	// standard menu drawing
	Menu_Draw(&s_callvotemenu.menu);
}

/*
=======================================================================================================================================
UI_CallVoteMenuInternal
=======================================================================================================================================
*/
void UI_CallVoteMenuInternal(void) {
	int y;
	int gametype;

	CallVoteMenu_Cache();

	gametype = UI_ServerGametype();

	s_callvotemenu.menu.wrapAround = qtrue;
	s_callvotemenu.menu.fullscreen = qfalse;
	s_callvotemenu.menu.draw = UI_CallVoteMenu_Draw;

	s_callvotemenu.banner.generic.type = MTYPE_BTEXT;
	s_callvotemenu.banner.generic.x = 320;
	s_callvotemenu.banner.generic.y = 16;
	s_callvotemenu.banner.string = "CALL VOTE";
	s_callvotemenu.banner.color = color_white;
	s_callvotemenu.banner.style = UI_CENTER;

	y = 98;
	s_callvotemenu.bMapRestart.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bMapRestart.color = color_red;
	s_callvotemenu.bMapRestart.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bMapRestart.generic.x = 320;
	s_callvotemenu.bMapRestart.generic.y = y;
	s_callvotemenu.bMapRestart.generic.id = ID_RESTART;
	s_callvotemenu.bMapRestart.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bMapRestart.string = "Restart match";
	s_callvotemenu.bMapRestart.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bNextmap.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bNextmap.color = color_red;
	s_callvotemenu.bNextmap.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bNextmap.generic.x = 320;
	s_callvotemenu.bNextmap.generic.y = y;
	s_callvotemenu.bNextmap.generic.id = ID_NEXTMAP;
	s_callvotemenu.bNextmap.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bNextmap.string = "Next map";
	s_callvotemenu.bNextmap.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bMap.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bMap.color = color_red;
	s_callvotemenu.bMap.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bMap.generic.x = 320;
	s_callvotemenu.bMap.generic.y = y;
	s_callvotemenu.bMap.generic.id = ID_CHANGEMAP;
	s_callvotemenu.bMap.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bMap.string = "Change map";
	s_callvotemenu.bMap.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bGametype.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bGametype.color = color_red;
	s_callvotemenu.bGametype.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bGametype.generic.x = 320;
	s_callvotemenu.bGametype.generic.y = y;
	s_callvotemenu.bGametype.generic.id = ID_GAMETYPE;
	s_callvotemenu.bGametype.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bGametype.string = "Change gametype";
	s_callvotemenu.bGametype.style = UI_CENTER|UI_SMALLFONT;

	if (gametype < GT_CTF) {
		y += VOTEMENU_MENU_VERTICAL_SPACING;
		s_callvotemenu.bFraglimit.generic.type = MTYPE_PTEXT;
		s_callvotemenu.bFraglimit.color = color_red;
		s_callvotemenu.bFraglimit.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		s_callvotemenu.bFraglimit.generic.x = 320;
		s_callvotemenu.bFraglimit.generic.y = y;
		s_callvotemenu.bFraglimit.generic.id = ID_LIMITS;
		s_callvotemenu.bFraglimit.generic.callback = CallVoteMenu_Event;
		s_callvotemenu.bFraglimit.string = "Change fraglimit";
		s_callvotemenu.bFraglimit.style = UI_CENTER|UI_SMALLFONT;
	} else {
		y += VOTEMENU_MENU_VERTICAL_SPACING;
		s_callvotemenu.bCapturelimit.generic.type = MTYPE_PTEXT;
		s_callvotemenu.bCapturelimit.color = color_red;
		s_callvotemenu.bCapturelimit.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		s_callvotemenu.bCapturelimit.generic.x = 320;
		s_callvotemenu.bCapturelimit.generic.y = y;
		s_callvotemenu.bCapturelimit.generic.id = ID_LIMITS;
		s_callvotemenu.bCapturelimit.generic.callback = CallVoteMenu_Event;
		s_callvotemenu.bCapturelimit.string = "Change capturelimit";
		s_callvotemenu.bCapturelimit.style = UI_CENTER|UI_SMALLFONT;
	}

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bTimelimit.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bTimelimit.color = color_red;
	s_callvotemenu.bTimelimit.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bTimelimit.generic.x = 320;
	s_callvotemenu.bTimelimit.generic.y = y;
	s_callvotemenu.bTimelimit.generic.id = ID_TIMELIMIT;
	s_callvotemenu.bTimelimit.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bTimelimit.string = "Change timelimit";
	s_callvotemenu.bTimelimit.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bDoWarmup.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bDoWarmup.color = color_red;
	s_callvotemenu.bDoWarmup.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bDoWarmup.generic.x = 320;
	s_callvotemenu.bDoWarmup.generic.y = y;
	s_callvotemenu.bDoWarmup.generic.id = ID_DOWARMUP;
	s_callvotemenu.bDoWarmup.generic.callback = CallVoteMenu_Event;

	if (s_callvotemenu.g_doWarmupEnabled) {
		s_callvotemenu.bDoWarmup.string = "Disable warmup";
	} else {
		s_callvotemenu.bDoWarmup.string = "Enable warmup";
	}

	s_callvotemenu.bDoWarmup.style = UI_CENTER|UI_SMALLFONT;

	y += VOTEMENU_MENU_VERTICAL_SPACING;
	s_callvotemenu.bKick.generic.type = MTYPE_PTEXT;
	s_callvotemenu.bKick.color = color_red;
	s_callvotemenu.bKick.generic.flags = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.bKick.generic.x = 320;
	s_callvotemenu.bKick.generic.y = y;
	s_callvotemenu.bKick.generic.id = ID_KICK;
	s_callvotemenu.bKick.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.bKick.string = "Kick player";
	s_callvotemenu.bKick.style = UI_CENTER|UI_SMALLFONT;

	s_callvotemenu.back.generic.type = MTYPE_BITMAP;
	s_callvotemenu.back.generic.name = VOTEMENU_BACK0;
	s_callvotemenu.back.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.back.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.back.generic.id = ID_BACK;
	s_callvotemenu.back.generic.x = 320 - 128;
	s_callvotemenu.back.generic.y = 256 + 128 - 64 + VOTEMENU_MENU_VERTICAL_SPACING * 2;
	s_callvotemenu.back.width = 128;
	s_callvotemenu.back.height = 64;
	s_callvotemenu.back.focuspic = VOTEMENU_BACK1;

	s_callvotemenu.go.generic.type = MTYPE_BITMAP;
	s_callvotemenu.go.generic.name = ART_FIGHT0;
	s_callvotemenu.go.generic.flags = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_callvotemenu.go.generic.callback = CallVoteMenu_Event;
	s_callvotemenu.go.generic.id = ID_GO;
	s_callvotemenu.go.generic.x = 320;
	s_callvotemenu.go.generic.y = 256 + 128 - 64 + VOTEMENU_MENU_VERTICAL_SPACING * 2;
	s_callvotemenu.go.width = 128;
	s_callvotemenu.go.height = 64;
	s_callvotemenu.go.focuspic = ART_FIGHT1;
}

/*
=======================================================================================================================================
UI_CallVoteMenu
=======================================================================================================================================
*/
void UI_CallVoteMenu(void) {
	char serverinfo[MAX_INFO_STRING];
	int gametype;

	// zero set all our globals
	memset(&s_callvotemenu, 0, sizeof(callvotemenu_t));

	trap_GetConfigString(CS_SERVERINFO, serverinfo, MAX_INFO_STRING);

	s_callvotemenu.g_doWarmupEnabled = atoi(Info_ValueForKey(serverinfo, "g_doWarmup"));

	UI_CallVoteMenuInternal();

	gametype = UI_ServerGametype();

	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.banner);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.back);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bMapRestart);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bNextmap);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bMap);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bGametype);

	if (gametype < GT_CTF) {
		Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bFraglimit);
	} else {
		Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bCapturelimit);
	}

	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bTimelimit);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bDoWarmup);
	Menu_AddItem(&s_callvotemenu.menu, (void *)&s_callvotemenu.bKick);

	UI_PushMenu(&s_callvotemenu.menu);
}
