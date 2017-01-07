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

#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "ui_public.h"
#include "../client/keycodes.h"
#include "../game/bg_public.h"
#include "ui_shared.h"

// global display context
extern vmCvar_t ui_ffa_fraglimit;
extern vmCvar_t ui_ffa_timelimit;
extern vmCvar_t ui_tourney_fraglimit;
extern vmCvar_t ui_tourney_timelimit;
extern vmCvar_t ui_team_fraglimit;
extern vmCvar_t ui_team_timelimit;
extern vmCvar_t ui_team_friendly;
extern vmCvar_t ui_ctf_capturelimit;
extern vmCvar_t ui_ctf_timelimit;
extern vmCvar_t ui_ctf_friendly;
extern vmCvar_t ui_arenasFile;
extern vmCvar_t ui_botsFile;
extern vmCvar_t ui_spScores1;
extern vmCvar_t ui_spScores2;
extern vmCvar_t ui_spScores3;
extern vmCvar_t ui_spScores4;
extern vmCvar_t ui_spScores5;
extern vmCvar_t ui_spAwards;
extern vmCvar_t ui_spVideos;
extern vmCvar_t ui_spSkill;
extern vmCvar_t ui_spSelection;
extern vmCvar_t ui_browserMaster;
extern vmCvar_t ui_browserGameType;
extern vmCvar_t ui_browserShowFull;
extern vmCvar_t ui_browserShowEmpty;
extern vmCvar_t ui_brassTime;
extern vmCvar_t ui_drawCrosshair;
extern vmCvar_t ui_drawCrosshairNames;
extern vmCvar_t ui_marks;
extern vmCvar_t ui_server1;
extern vmCvar_t ui_server2;
extern vmCvar_t ui_server3;
extern vmCvar_t ui_server4;
extern vmCvar_t ui_server5;
extern vmCvar_t ui_server6;
extern vmCvar_t ui_server7;
extern vmCvar_t ui_server8;
extern vmCvar_t ui_server9;
extern vmCvar_t ui_server10;
extern vmCvar_t ui_server11;
extern vmCvar_t ui_server12;
extern vmCvar_t ui_server13;
extern vmCvar_t ui_server14;
extern vmCvar_t ui_server15;
extern vmCvar_t ui_server16;
extern vmCvar_t ui_captureLimit;
extern vmCvar_t ui_fragLimit;
extern vmCvar_t ui_gameType;
extern vmCvar_t ui_netGameType;
extern vmCvar_t ui_actualNetGameType;
extern vmCvar_t ui_joinGameType;
extern vmCvar_t ui_netSource;
extern vmCvar_t ui_serverFilterType;
extern vmCvar_t ui_dedicated;
extern vmCvar_t ui_opponentName;
extern vmCvar_t ui_menuFiles;
extern vmCvar_t ui_currentTier;
extern vmCvar_t ui_currentMap;
extern vmCvar_t ui_currentNetMap;
extern vmCvar_t ui_mapIndex;
extern vmCvar_t ui_currentOpponent;
extern vmCvar_t ui_selectedPlayer;
extern vmCvar_t ui_selectedPlayerName;
extern vmCvar_t ui_lastServerRefresh_0;
extern vmCvar_t ui_lastServerRefresh_1;
extern vmCvar_t ui_lastServerRefresh_2;
extern vmCvar_t ui_lastServerRefresh_3;
extern vmCvar_t ui_singlePlayerActive;
extern vmCvar_t ui_scoreAccuracy;
extern vmCvar_t ui_scoreImpressives;
extern vmCvar_t ui_scoreExcellents;
extern vmCvar_t ui_scoreDefends;
extern vmCvar_t ui_scoreAssists;
extern vmCvar_t ui_scoreGauntlets;
extern vmCvar_t ui_scoreScore;
extern vmCvar_t ui_scorePerfect;
extern vmCvar_t ui_scoreTeam;
extern vmCvar_t ui_scoreBase;
extern vmCvar_t ui_scoreTimeBonus;
extern vmCvar_t ui_scoreSkillBonus;
extern vmCvar_t ui_scoreShutoutBonus;
extern vmCvar_t ui_scoreTime;
extern vmCvar_t ui_smallFont;
extern vmCvar_t ui_bigFont;
extern vmCvar_t ui_serverStatusTimeOut;
// ui_qmenu.c
#define MAX_EDIT_LINE 256
#define MAX_MENUITEMS 96

typedef struct _tag_menuframework {
	int cursor;
	int cursor_prev;
	int nitems;
	void *items[MAX_MENUITEMS];
	void (*draw)(void);
	sfxHandle_t (*key)(int key);
	qboolean wrapAround;
	qboolean fullscreen;
	qboolean showlogo;
} menuframework_s;

typedef struct {
	int type;
	const char *name;
	int id;
	int x, y;
	int left;
	int top;
	int right;
	int bottom;
	menuframework_s *parent;
	int menuPosition;
	unsigned flags;
	void (*callback)(void *self, int event);
	void (*statusbar)(void *self);
	void (*ownerdraw)(void *self);
} menucommon_s;

typedef struct {
	int cursor;
	int scroll;
	int widthInChars;
	char buffer[MAX_EDIT_LINE];
	int maxchars;
} mfield_t;

typedef struct {
	menucommon_s generic;
	mfield_t field;
} menufield_s;

typedef struct {
	menucommon_s generic;
	int oldvalue;
	int curvalue;
	int numitems;
	int top;
	const char **itemnames;
	int width;
	int height;
	int columns;
	int seperation;
} menulist_s;

typedef struct {
	menucommon_s generic;
} menuaction_s;

typedef struct {
	menucommon_s generic;
	char *focuspic;
	char *errorpic;
	qhandle_t shader;
	qhandle_t focusshader;
	int width;
	int height;
	float *focuscolor;
} menubitmap_s;

extern sfxHandle_t menu_in_sound;
extern sfxHandle_t menu_move_sound;
extern sfxHandle_t menu_out_sound;
extern sfxHandle_t menu_buzz_sound;
extern sfxHandle_t menu_null_sound;
extern sfxHandle_t weaponChangeSound;

extern vec4_t menu_text_color;
extern vec4_t menu_grayed_color;
extern vec4_t menu_dark_color;
extern vec4_t menu_highlight_color;
extern vec4_t menu_red_color;
extern vec4_t menu_black_color;
extern vec4_t menu_dim_color;
extern vec4_t color_black;
extern vec4_t color_white;
extern vec4_t color_yellow;
extern vec4_t color_blue;
extern vec4_t color_orange;
extern vec4_t color_red;
extern vec4_t color_dim;
extern vec4_t name_color;
extern vec4_t list_color;
extern vec4_t listbar_color;
extern vec4_t text_color_disabled;
extern vec4_t text_color_normal;
extern vec4_t text_color_highlight;
extern char *ui_medalNames[];
extern char *ui_medalPicNames[];
extern char *ui_medalSounds[];
// ui_main.c
void UI_Report(void);
void UI_Load(void);
void UI_LoadMenus(const char *menuFile, qboolean reset);
void _UI_SetActiveMenu(uiMenuCommand_t menu);
int UI_AdjustTimeByGame(int time);
void UI_ShowPostGame(qboolean newHigh);
void UI_ClearScores(void);
void UI_LoadArenas(void);

extern void UI_RegisterCvars(void);
extern void UI_UpdateCvars(void);
// ui_connect.c
extern void UI_DrawConnectScreen(qboolean overlay);
// ui_video.c
extern void UI_GraphicsOptionsMenu(void);
extern void GraphicsOptions_Cache(void);
extern void DriverInfo_Cache(void);
// ui_players.c
//FIXME ripped from cg_local.h
typedef struct {
	int oldFrame;
	int oldFrameTime;		// time when->oldFrame was exactly on
	int frame;
	int frameTime;			// time when->frame will be exactly on
	float backlerp;
	float yawAngle;
	qboolean yawing;
	float pitchAngle;
	qboolean pitching;
	int animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t *animation;
	int animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;

typedef struct {
	// model info
	qhandle_t legsModel;
	qhandle_t legsSkin;
	lerpFrame_t legs;
	qhandle_t torsoModel;
	qhandle_t torsoSkin;
	lerpFrame_t torso;
	qhandle_t headModel;
	qhandle_t headSkin;
	animation_t animations[MAX_TOTALANIMATIONS];
	qhandle_t weaponModel;
	qhandle_t barrelModel;
	qhandle_t flashModel;
	vec3_t flashDlightColor;
	int muzzleFlashTime;
	vec3_t color1;
	byte c1RGBA[4];
	// currently in use drawing parms
	vec3_t viewAngles;
	vec3_t moveAngles;
	weapon_t currentWeapon;
	int legsAnim;
	int torsoAnim;
	// animation vars
	weapon_t weapon;
	weapon_t lastWeapon;
	weapon_t pendingWeapon;
	int weaponTimer;
	int pendingLegsAnim;
	int torsoAnimationTimer;
	int pendingTorsoAnim;
	int legsAnimationTimer;
	qboolean chat;
	qboolean newModel;
	qboolean barrelSpinning;
	float barrelAngle;
	int barrelTime;
	int realWeapon;
} playerInfo_t;

void UI_DrawPlayer(float x, float y, float w, float h, playerInfo_t *pi, int time);
void UI_PlayerInfo_SetModel(playerInfo_t *pi, const char *model, const char *headmodel, char *teamName);
void UI_PlayerInfo_SetInfo(playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNum, qboolean chat);
void UI_ColorFromIndex(int val, vec3_t color);
void UI_PlayerInfo_UpdateColor(playerInfo_t *pi);
qboolean UI_RegisterClientModelname(playerInfo_t *pi, const char *modelSkinName, const char *headName, const char *teamName);
// new ui stuff
#define UI_NUMFX 7
#define MAX_HEADS 64
#define MAX_ALIASES 64
#define MAX_HEADNAME 32
#define MAX_TEAMS 64
#define MAX_GAMETYPES 16
#define MAX_MAPS 128
#define MAX_SPMAPS 16
#define PLAYERS_PER_TEAM 5
#define MAX_PINGREQUESTS 32
#define MAX_ADDRESSLENGTH 64
#define MAX_HOSTNAMELENGTH 22
#define MAX_MAPNAMELENGTH 16
#define MAX_STATUSLENGTH 64
#define MAX_LISTBOXWIDTH 59
#define UI_FONT_THRESHOLD 0.1
#define MAX_DISPLAY_SERVERS 2048
#define MAX_SERVERSTATUS_LINES 128
#define MAX_SERVERSTATUS_TEXT 1024
#define MAX_FOUNDPLAYER_SERVERS 16
#define TEAM_MEMBERS 5

#define GAMES_ALL		0
#define GAMES_FFA		1
#define GAMES_TEAMPLAY	2
#define GAMES_TOURNEY	3
#define GAMES_CTF		4

#define MAPS_PER_TIER 3
#define MAX_TIERS 16
#define MAX_MODS 64
#define MAX_DEMOS 512
#define MAX_MOVIES 256
#define MAX_PLAYERMODELS 256

typedef struct {
	const char *name;
	const char *imageName;
	qhandle_t headImage;
	const char *base;
	qboolean active;
	int reference;
} characterInfo;

typedef struct {
	const char *name;
	const char *ai;
	const char *action;
} aliasInfo;

typedef struct {
	const char *teamName;
	const char *imageName;
	const char *teamMembers[TEAM_MEMBERS];
	qhandle_t teamIcon;
	qhandle_t teamIcon_Metal;
	qhandle_t teamIcon_Name;
	int cinematic;
} teamInfo;

typedef struct {
	const char *gameType;
	int gtEnum;
} gameTypeInfo;

typedef struct {
	const char *mapName;
	const char *mapLoadName;
	const char *imageName;
	const char *opponentName;
	int teamMembers;
	int typeBits;
	int cinematic;
	int timeToBeat[MAX_GAMETYPES];
	qhandle_t levelShot;
	qboolean active;
} mapInfo;

typedef struct {
	const char *tierName;
	const char *maps[MAPS_PER_TIER];
	int gameTypes[MAPS_PER_TIER];
	qhandle_t mapHandles[MAPS_PER_TIER];
} tierInfo;

typedef struct serverFilter_s {
	const char *description;
	const char *basedir;
} serverFilter_t;

typedef struct {
	char adrstr[MAX_ADDRESSLENGTH];
	int start;
} pinglist_t;

typedef struct serverStatus_s {
	pinglist_t pingList[MAX_PINGREQUESTS];
	int numqueriedservers;
	int currentping;
	int nextpingtime;
	int maxservers;
	int refreshtime;
	int numServers;
	int sortKey;
	int sortDir;
	int lastCount;
	qboolean refreshActive;
	int currentServer;
	int displayServers[MAX_DISPLAY_SERVERS];
	int numDisplayServers;
	int numPlayersOnServers;
	int nextDisplayRefresh;
	int nextSortTime;
	qhandle_t currentServerPreview;
	int currentServerCinematic;
	float motdOffset;
	int motdTime;
	char motd[MAX_STRING_CHARS];
} serverStatus_t;

typedef struct {
	char adrstr[MAX_ADDRESSLENGTH];
	char name[MAX_ADDRESSLENGTH];
	int startTime;
	int serverNum;
	qboolean valid;
} pendingServer_t;

typedef struct {
	int num;
	pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
} pendingServerStatus_t;

typedef struct {
	char address[MAX_ADDRESSLENGTH];
	char *lines[MAX_SERVERSTATUS_LINES][4];
	char text[MAX_SERVERSTATUS_TEXT];
	char pings[MAX_CLIENTS * 3];
	int numLines;
} serverStatusInfo_t;

typedef struct {
	const char *modName;
	const char *modDescr;
} modInfo_t;

typedef struct {
	displayContextDef_t uiDC;
	int newHighScoreTime;
	int newBestTime;
	int showPostGameTime;
	qboolean newHighScore;
	qboolean demoAvailable;
	qboolean soundHighScore;
	int characterCount;
	int botIndex;
	characterInfo characterList[MAX_HEADS];
	int aliasCount;
	aliasInfo aliasList[MAX_ALIASES];
	int teamCount;
	teamInfo teamList[MAX_TEAMS];
	int numGameTypes;
	gameTypeInfo gameTypes[MAX_GAMETYPES];
	int numJoinGameTypes;
	gameTypeInfo joinGameTypes[MAX_GAMETYPES];
	int redBlue;
	int playerCount;
	int myTeamCount;
	int teamIndex;
	int playerRefresh;
	int playerIndex;
	int playerNumber;
	qboolean teamLeader;
	char playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	char teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	int teamClientNums[MAX_CLIENTS];
	int mapCount;
	mapInfo mapList[MAX_MAPS];
	int tierCount;
	tierInfo tierList[MAX_TIERS];
	int skillIndex;
	modInfo_t modList[MAX_MODS];
	int modCount;
	int modIndex;
	const char *demoList[MAX_DEMOS];
	int demoCount;
	int demoIndex;
	const char *movieList[MAX_MOVIES];
	int movieCount;
	int movieIndex;
	int previewMovie;
	serverStatus_t serverStatus;
	// for the showing the status of a server
	char serverStatusAddress[MAX_ADDRESSLENGTH];
	serverStatusInfo_t serverStatusInfo;
	int nextServerStatusRefresh;
	// to retrieve the status of server to find a player
	pendingServerStatus_t pendingServerStatus;
	char findPlayerName[MAX_STRING_CHARS];
	char foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	char foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	int currentFoundPlayerServer;
	int numFoundPlayerServers;
	int nextFindPlayerRefresh;
	int currentCrosshair;
	int startPostGameTime;
	sfxHandle_t newHighScoreSound;
	int q3HeadCount;
	char q3HeadNames[MAX_PLAYERMODELS][64];
	qhandle_t q3HeadIcons[MAX_PLAYERMODELS];
	int q3SelectedHead;
	int effectsColor;
	qboolean inGameLoad;
} uiInfo_t;

extern uiInfo_t uiInfo;

extern void UI_Init(void);
extern void UI_Shutdown(void);
extern void UI_KeyEvent(int key);
extern void UI_MouseEvent(int dx, int dy);
extern void UI_Refresh(int realtime);
extern qboolean UI_ConsoleCommand(int realTime);
extern void UI_DrawNamedPic(float x, float y, float width, float height, const char *picname);
extern void UI_DrawHandlePic(float x, float y, float w, float h, qhandle_t hShader);
extern void UI_FillRect(float x, float y, float width, float height, const float *color);
extern void UI_DrawRect(float x, float y, float width, float height, const float *color);
extern void UI_DrawTopBottom(float x, float y, float w, float h);
extern void UI_DrawSides(float x, float y, float w, float h);
extern void UI_UpdateScreen(void);
extern void UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
extern void UI_DrawBannerString(int x, int y, const char *str, int style, vec4_t color);
extern float UI_ProportionalSizeScale(int style);
extern void UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color);
extern int UI_ProportionalStringWidth(const char *str);
extern void UI_DrawString(int x, int y, const char *str, int style, vec4_t color);
extern void UI_DrawChar(int x, int y, int ch, int style, vec4_t color);
extern qboolean UI_CursorInRect(int x, int y, int width, int height);
extern void UI_AdjustFrom640(float *x, float *y, float *w, float *h);
extern qboolean UI_IsFullscreen(void);
extern void UI_SetActiveMenu(uiMenuCommand_t menu);
extern void UI_ForceMenuOff(void);
extern char *UI_Argv(int arg);
extern char *UI_Cvar_VariableString(const char *var_name);
extern void UI_Refresh(int time);
extern void UI_KeyEvent(int key);
extern qboolean m_entersound;

void UI_LoadBestScores(const char *map, int game);
// ui_syscalls.c
void trap_Print(const char *string);
void trap_Error(const char *string) __attribute__((noreturn));
int trap_Milliseconds(void);
void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags);
void trap_Cvar_Update(vmCvar_t *vmCvar);
void trap_Cvar_Set(const char *var_name, const char *value);
float trap_Cvar_VariableValue(const char *var_name);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);
void trap_Cvar_SetValue(const char *var_name, float value);
void trap_Cvar_Reset(const char *name);
void trap_Cvar_InfoStringBuffer(int bit, char *buffer, int bufsize);
int trap_Argc(void);
void trap_Argv(int n, char *buffer, int bufferLength);
int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode);
int trap_FS_Read(void *buffer, int len, fileHandle_t f);
int trap_FS_Write(const void *buffer, int len, fileHandle_t f);
void trap_FS_FCloseFile(fileHandle_t f);
int trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize);
int trap_FS_Seek(fileHandle_t f, long offset, int origin); // fsOrigin_t
qhandle_t trap_R_RegisterModel(const char *name);
qhandle_t trap_R_RegisterSkin(const char *name);
qhandle_t trap_R_RegisterShaderNoMip(const char *name);
void trap_R_ClearScene(void);
void trap_R_AddRefEntityToScene(const refEntity_t *re);
void trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts);
void trap_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b);
void trap_R_RenderScene(const refdef_t *fd);
void trap_R_SetColor(const float *rgba);
void trap_R_SetClipRegion(const float *region);
void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);
void trap_UpdateScreen(void);
int trap_R_LerpTag(orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName);
void trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed);
int trap_S_SoundDuration(sfxHandle_t handle);
void trap_Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
void trap_Key_GetBindingBuf(int keynum, char *buf, int buflen);
void trap_Key_SetBinding(int keynum, const char *binding);
qboolean trap_Key_IsDown(int keynum);
qboolean trap_Key_GetOverstrikeMode(void);
void trap_Key_SetOverstrikeMode(qboolean state);
void trap_Key_ClearStates(void);
int trap_Key_GetCatcher(void);
void trap_Key_SetCatcher(int catcher);
int trap_Key_GetKey(const char *binding, int startKey);
void trap_GetClipboardData(char *buf, int bufsize);
void trap_GetClientState(uiClientState_t *state);
void trap_GetGlconfig(glconfig_t *glconfig);
int trap_GetConfigString(int index, char *buff, int buffsize);
int trap_LAN_GetServerCount(int source);
void trap_LAN_GetServerAddressString(int source, int n, char *buf, int buflen);
void trap_LAN_GetServerInfo(int source, int n, char *buf, int buflen);
int trap_LAN_GetServerPing(int source, int n);
int trap_LAN_GetPingQueueCount(void);
void trap_LAN_ClearPing(int n);
void trap_LAN_GetPing(int n, char *buf, int buflen, int *pingtime);
void trap_LAN_GetPingInfo(int n, char *buf, int buflen);
void trap_LAN_LoadCachedServers(void);
void trap_LAN_SaveCachedServers(void);
void trap_LAN_MarkServerVisible(int source, int n, qboolean visible);
int trap_LAN_ServerIsVisible(int source, int n);
qboolean trap_LAN_UpdateVisiblePings(int source);
int trap_LAN_AddServer(int source, const char *name, const char *addr);
void trap_LAN_RemoveServer(int source, const char *addr);
void trap_LAN_ResetPings(int n);
int trap_LAN_ServerStatus(const char *serverAddress, char *serverStatus, int maxLen);
int trap_LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2);
int trap_MemoryRemaining(void);
void trap_R_RegisterFont(const char *pFontname, int pointSize, fontInfo_t *font);
void trap_S_StopBackgroundTrack(void);
void trap_S_StartBackgroundTrack(const char *intro, const char *loop);
int trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic(int handle);
void trap_CIN_DrawCinematic(int handle);
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h);
int trap_RealTime(qtime_t *qtime);
void trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);
// ui_gameinfo.c
char *UI_GetBotInfoByNumber(int num);
char *UI_GetBotInfoByName(const char *name);
int UI_GetNumBots(void);
void UI_LoadBots(void);
char *UI_GetBotNameByNumber(int num);
// new ui
#define ASSET_BACKGROUND "uiBackground"
// for tracking sp game info in Team Arena
typedef struct postGameInfo_s {
	int score;
	int redScore;
	int blueScore;
	int perfects;
	int accuracy;
	int impressives;
	int excellents;
	int defends;
	int assists;
	int gauntlets;
	int captures;
	int time;
	int timeBonus;
	int shutoutBonus;
	int skillBonus;
	int baseScore;
} postGameInfo_t;
#endif
