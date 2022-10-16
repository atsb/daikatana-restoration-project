#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"
#include "client.h"

#include "l__language.h"

#include "csv.h"
#include "dk_io.h"
#include "server.h"

//#include "mp_launch.h"

//==============================
// GAMESPY GAME NAME
#define SERVER_NAME	"daikatana"
//#define SERVER_NAME "unreal" 
//==============================


// compile defines

#define REMOVE_SKILLS
#define NO_EPISODE_SELECTOR

// cek[12-10-99] moved these to the localiztion files...
/*
static enum temp_tongue_menu_enum
{
	T_MENU_CTF,
	T_MENU_MULTIPLAYER_BACK,
	TEMP_T_MENU_CONNECTSPEED_3,
	T_MENU_AUTO,
	T_MENU_TEAM_1,
	T_MENU_TEAM_2,
};

static char *temp_tongue_menu[] = 
{
	"Capture the Flag",
	"Back",
	"56.6k",
	"Auto",
	"Team 1",
	"Team 2",
};
*/

///////////////////////////////////////////////////////////////////////////////////
//
//  Multiplayer Menu
//
///////////////////////////////////////////////////////////////////////////////////

enum Multiplayer_Modes
{
	MPM_DEATHMATCH,
	MPM_CTF,
	MPM_DEATHTAG,
	MPM_COOP,
	MPM_ASSIGN
};

#define CHARACTER_MODEL_ANIM_DELAY  (50)     // animation delay
#define CHARACTER_MODEL_ROTATION    (1.0)	//(0.35f)  // speed of rotation (1.0)
#define CHARACTER_MODEL_START_ANGLE (165.0f) // facing direction
                
#define MULTI_LEFT (100)
#define MULTI_TOP_JOIN_LEFT (110)
#define MULTI_TOP_JOIN_WIDTH (90)
#define MULTI_TOP_START_LEFT (MULTI_TOP_JOIN_LEFT + MULTI_TOP_JOIN_WIDTH + 30)
#define MULTI_TOP_START_WIDTH (90)
#define MULTI_TOP_CHARACTER_LEFT (MULTI_TOP_START_LEFT + MULTI_TOP_START_WIDTH + 30)
#define MULTI_TOP_CHARACTER_WIDTH (90)

#define MULTI_LAN_BUTTON_LEFT       (110)
#define MULTI_LAN_BUTTON_WIDTH      (90)
#define MULTI_INTERNET_BUTTON_LEFT  (230)
#define MULTI_INTERNET_BUTTON_WIDTH (90)

// SCG[10/29/99]: #define MULTI_TOP (125)
#define MULTI_TOP (105)

#define MULTI_TOP_MPLAYER_TEXT  (112)
#define MULTI_LEFT_MPLAYER_TEXT (200)
#define MULTI_TOP_MPLAYER_ICON  ( 90)
#define MULTI_LEFT_MPLAYER_ICON (120)

#define MULTI_BUTTON_HEIGHT (20)

#define SPACE (14)

//#define MULTI_SETUP_LEFT (MULTI_LEFT + 10)

//#define MULTI_SETUP_HANDEDNESS_TOP (MULTI_SETUP_TOP + SPACE + 20 + 4)

#define MULTI_SETUP_HANDEDNESS_TOP (MULTI_SETUP_TOP + SPACE + 40 + 4)
#define multi_setup_handedness_line(i) (MULTI_SETUP_HANDEDNESS_TOP + i * SPACE)

#define SPEEDPICKER_WIDTH 160
//#define multi_setup_speed_line(i) (MULTI_SETUP_SPEED_TOP + i * SPACE)

//#define MULTI_START_EPISODE_WIDTH (245)

//#define MULTI_START_RIGHT (MULTI_START_LEFT + 250)
#define MULTI_START_RIGHT (318)
#define MULTI_START_RIGHT_WIDTH (155)

#define MULTI_START_GO_LEFT (360)
#define MULTI_START_GO_TOP (380)//(365)

#define MULTI_JOIN_NAME_LEFT (90)
#define MULTI_JOIN_NAME_TOP (MULTI_JOIN_TOP + 15)

#define MULTI_JOIN_PLAY_LEFT (MULTI_JOIN_NAME_LEFT + 165)
#define MULTI_JOIN_PLAY_TOP (MULTI_JOIN_NAME_TOP)

//#define MULTI_JOIN_ADD_LEFT (MULTI_JOIN_PLAY_LEFT + 70)
//#define MULTI_JOIN_ADD_TOP (MULTI_JOIN_PLAY_TOP)

//#define MULTI_JOIN_REFRESH_LEFT (MULTI_JOIN_LEFT + 20)
//#define MULTI_JOIN_REFRESH_TOP (MULTI_JOIN_NAME_TOP + 30)

#define MULTI_JOIN_REFRESH_LEFT (MULTI_JOIN_PLAY_LEFT + 110)  // moved over to old 'add' button
#define MULTI_JOIN_REFRESH_TOP (MULTI_JOIN_NAME_TOP)


#define MULTI_JOIN_LOCAL_LIST_LEFT (MULTI_JOIN_LEFT - 10)
#define MULTI_JOIN_LOCAL_LIST_TOP (MULTI_JOIN_REFRESH_TOP + 40) // 30
#define MULTI_JOIN_LOCAL_LIST_WIDTH (376)
#define MULTI_JOIN_LOCAL_LIST_RIGHT (MULTI_JOIN_LOCAL_LIST_LEFT + MULTI_JOIN_LOCAL_LIST_WIDTH)

#define MULTI_JOIN_LOCAL_LIST_LINE_SPACING 14
#define multi_join_local_list_line_y(i) (MULTI_JOIN_LOCAL_LIST_TOP + (i) * MULTI_JOIN_LOCAL_LIST_LINE_SPACING)

#define MULTI_JOIN_LOCAL_LIST_NUM_LINES (12)
#define MULTI_JOIN_LOCAL_LIST_BOTTOM (multi_join_local_list_line_y(MULTI_JOIN_LOCAL_LIST_NUM_LINES))


// < ServerName       > < map > <type> xx/xx ppppp
// 01234567890123456789012345678901234567890123456
//	         1         2         3         4
#define MM_PLACEMENT(c)				(MULTI_JOIN_LOCAL_LIST_LEFT + (8*c))
#define MM_WIDTH(c)					(8*c)
#define INTERNETJOIN_HOST			MM_PLACEMENT(0)
#define INTERNETJOIN_MAP			MM_PLACEMENT(21)
#define INTERNETJOIN_GAME			MM_PLACEMENT(29)
#define INTERNETJOIN_PLAYERS		MM_PLACEMENT(36)
#define INTERNETJOIN_PING			MM_PLACEMENT(42)

#define INTERNETJOIN_HOST_CHARS		19
#define INTERNETJOIN_HOST_WIDTH		MM_WIDTH(INTERNETJOIN_HOST_CHARS)
#define INTERNETJOIN_MAP_WIDTH		MM_WIDTH(7)
#define INTERNETJOIN_GAME_WIDTH		MM_WIDTH(8)
#define INTERNETJOIN_PLAYERS_WIDTH	MM_WIDTH(5)
#define INTERNETJOIN_PING_WIDTH		MM_WIDTH(5)

#define MULTI_JOIN_LOCAL_LIST_UP_LEFT	(MULTI_JOIN_LOCAL_LIST_RIGHT - 8 * 12)
#define MULTI_JOIN_LOCAL_LIST_UP_TOP	(MULTI_JOIN_LOCAL_LIST_BOTTOM + 9)

#define MULTI_JOIN_LOCAL_LIST_DOWN_LEFT	(MULTI_JOIN_LOCAL_LIST_RIGHT - 4 * 12)
#define MULTI_JOIN_LOCAL_LIST_DOWN_TOP	(MULTI_JOIN_LOCAL_LIST_BOTTOM + 9)


#define JOIN_TEAM_CHOICE_PICKER_LEFT	(MULTI_JOIN_LEFT)
#define JOIN_TEAM_CHOICE_PICKER_TOP		(MULTI_JOIN_LOCAL_LIST_BOTTOM + 9)
#define JOIN_MULTI_START_LEVEL_LIMIT_WIDTH		( MULTI_START_SKILL_WIDTH )


#define JOINMOVE_START				700
#define JOINMOVE_REPEAT				100


// WAW[12/3/99]: Join back button.
// ===============================
#define MULTI_JOINBACK_LEFT		100
#define MULTI_JOINBACK_TOP		105
#define MULTI_HOSTBACK_LEFT		100
#define MULTI_HOSTBACK_TOP		105

// SCG[12/1/99]: ==============================================================================================
// SCG[12/1/99]: ==============================================================================================

// SCG[12/1/99]: Begin positional defines for main multiplayer menu
#define MULTI_BUTTON_V_SPACING				( 30 )
#define MULTI_FIND_INTERNET_GAME_TOP		( MULTI_START_SERVERNAME_TOP + MULTI_BUTTON_V_SPACING )
#define MULTI_FIND_INTERNET_GAME_LEFT		( MULTI_LEFT )
#define MULTI_FIND_LOCAL_GAME_TOP			( MULTI_FIND_INTERNET_GAME_TOP + MULTI_BUTTON_V_SPACING )
#define MULTI_FIND_LOCAL_GAME_LEFT			( MULTI_LEFT )
#define MULTI_START_MULTIPLAYER_GAME_TOP	( MULTI_FIND_LOCAL_GAME_TOP + MULTI_BUTTON_V_SPACING )
#define MULTI_START_MULTIPLAYER_GAME_LEFT	( MULTI_LEFT )

#define MULTI_SETUP_NAME_LABEL_LEFT			( 300 )
#define MULTI_SETUP_NAME_LABEL_TOP			( MULTI_TOP + 5 )
#define MULTI_SETUP_NAME_FIELD_TOP			( MULTI_SETUP_NAME_LABEL_TOP + 20 )
#define MULTI_START_SERVERNAME_LEFT			( MULTI_SETUP_LEFT )
#define MULTI_START_SERVERNAME_LABEL_TOP	( MULTI_SETUP_NAME_LABEL_TOP )
#define MULTI_START_SERVERNAME_TOP			( MULTI_SETUP_NAME_FIELD_TOP )
#define MULTI_SETUP_SPEED_TOP				( 360 )

#define MULTI_MPLAYER_LOGO_LEFT				( MULTI_SETUP_LEFT )
#define MULTI_MPLAYER_LOGO_TOP				( MULTI_START_MULTIPLAYER_GAME_TOP + MULTI_BUTTON_V_SPACING + 10 )
#define MULTI_MPLAYER_LOGO_WIDTH			( 64 )
#define MULTI_MPLAYER_LOGO_HEIGHT			( 64 )
#define MULTI_MPLAYER_LOGO_IMGHEIGHT		( 64 )
#define MULTI_MPLAYER_LOGO_IMGWIDTH			( 64 )

#define MULTI_START_MPLAYER_LEFT			( MULTI_MPLAYER_LOGO_LEFT + MULTI_MPLAYER_LOGO_WIDTH + 31 )
#define MULTI_START_MPLAYER_TOP				( MULTI_MPLAYER_LOGO_TOP + 8)
#define MULTI_MPLAYER_COMMUNITY_LEFT		( MULTI_START_MPLAYER_LEFT )
#define MULTI_MPLAYER_COMMUNITY_TOP			( MULTI_START_MPLAYER_TOP + MULTI_BUTTON_V_SPACING )
#define MPLAYER_EXECUTE_GAME				( 1 )
#define MPLAYER_EXECUTE_WEB					( 2 )

// confirm dlg
// the buttons
#define BUTTON_HEIGHT										(25)
#define BUTTON_SHORT_WIDTH									(63)
#define BUTTON_MED_WIDTH									(115)

// some cool stuff for centering buttons
#define BUTTON_SPACE(width,eawidth,max)						((width - (max * eawidth))/(max + 1))
#define BUTTON_L(left,width,num,max,eawidth)				(left + (num * BUTTON_SPACE(width,eawidth,max)) + ((num-1)*eawidth))
#define BUTTON_MED_LEFT(num,max)							(BUTTON_L(CONFIG_LEFT,FILE_LIST_WIDTH,num,max,BUTTON_MED_WIDTH))
#define CENTERED(left,width,cwidth)							(left + (0.5*width) - (0.5*cwidth))
#define CONFIRM_WIDTH										(350)
#define CONFIRM_HEIGHT										(110)
#define CONFIRM_BUTTON_TOP									(CONFIRM_BOTTOM - (BUTTON_HEIGHT + 10))
#define CONFIRM_BUTTON_LEFT(idx)							( BUTTON_L(CONFIRM_LEFT,(CONFIRM_RIGHT-CONFIRM_LEFT),idx,2,BUTTON_SHORT_WIDTH) )

#define CONFIG_LEFT											110
#define CONFIG_TOP											115
#define CONFIG_WIDTH										375
#define CONFIG_HEIGHT										300
#define CONFIRM_LEFT										(CENTERED(CONFIG_LEFT - 5,CONFIG_WIDTH,CONFIRM_WIDTH))
#define CONFIRM_RIGHT										(CONFIRM_LEFT + CONFIRM_WIDTH)
#define CONFIRM_TOP											(CENTERED(CONFIG_TOP,CONFIG_HEIGHT,CONFIRM_HEIGHT))
#define CONFIRM_BOTTOM										(CONFIRM_TOP + CONFIRM_HEIGHT)

/*
#define MULTI_FIND_INTERNET_GAME_TOP		( MULTI_TOP + 20 )
#define MULTI_FIND_INTERNET_GAME_LEFT		( MULTI_LEFT )
#define MULTI_FIND_LOCAL_GAME_TOP			( MULTI_FIND_INTERNET_GAME_TOP + 30 )
#define MULTI_FIND_LOCAL_GAME_LEFT			( MULTI_LEFT )
#define MULTI_START_MULTIPLAYER_GAME_TOP	( MULTI_FIND_LOCAL_GAME_TOP + 30 )
#define MULTI_START_MULTIPLAYER_GAME_LEFT	( MULTI_LEFT )
#define MULTI_SETUP_NAME_LABEL_TOP			( 240 )
#define MULTI_SETUP_NAME_FIELD_TOP			( 260 )
#define MULTI_START_SERVERNAME_LEFT			( MULTI_SETUP_LEFT )
#define MULTI_START_SERVERNAME_TOP			( 320 )
#define MULTI_SETUP_SPEED_TOP				( 360 )
*/

// model picker placement
#define MODELNAME_X							( 340 )
#define MODELNAME_Y							( 310 )
#define MODELNAME_WIDTH						( 120 )

// skin color picker placement
#define SKINCOLOR_X							( MODELNAME_X )
#define SKINCOLOR_Y							( MODELNAME_Y + 50 )
#define SKINCOLOR_WIDTH						( MODELNAME_WIDTH )
// SCG[12/1/99]: End positional defines for main multiplayer menu

// SCG[12/1/99]: Begin positional defines for start multiplayer game menu
#define MULTI_START_TIME_LEFT				( MULTI_START_LEFT )
#define MULTI_START_TIME_TOP				( MULTI_START_MODE_TOP + 110 )
#define MULTI_START_TIME_WIDTH				( 104 )

#define MULTI_START_SKILL_LEFT				( MULTI_START_LEFT )
#define MULTI_START_SKILL_TOP				( MULTI_START_TIME_TOP + 40 )
#define MULTI_START_SKILL_WIDTH				( 104 )

#define MULTI_START_MODE_LEFT				( 170 )
#define MULTI_START_MODE_TOP				( 100 )
#define MULTI_START_MODE_WIDTH				( MULTI_START_LEVEL_WIDTH )

#define MULTI_START_EPISODE_LEFT			( MULTI_START_LEFT )
#define MULTI_START_EPISODE_TOP				( MULTI_START_TOP + 45 )
#define MULTI_START_EPISODE_WIDTH			( 214 )

#define MULTI_START_LEVEL_LEFT				( MULTI_START_LEFT )
#define MULTI_START_LEVEL_TOP				( MULTI_START_TOP + 5 )
#define MULTI_START_LEVEL_WIDTH				( MULTI_START_EPISODE_WIDTH )

#define MULTI_SEPARATOR_LEFT				( 100 )
#define MULTI_SEPARATOR_RIGHT				( 470 )
#define MULTI_SEPARATOR_TOP					( MULTI_TOP + 17 )

#define MULTI_BUTTON_START_TOP				( MULTI_TOP + 30)

#define MULTI_GSPY_LOGO_LEFT				( 200 )
#define MULTI_GSPY_LOGO_TOP					( MULTI_TOP - 5 )
#define MULTI_GSPY_LOGO_WIDTH				( 280 )
#define MULTI_GSPY_LOGO_HEIGHT				( 80 )
#define MULTI_GSPY_LOGO_IMGWIDTH			( 256 )
#define MULTI_GSPY_LOGO_IMGHEIGHT			( 64 )

#define MULTI_JOIN_LEFT						( MULTI_LEFT )
#define MULTI_JOIN_TOP						( MULTI_BUTTON_START_TOP + 10)
											
#define MULTI_SETUP_LEFT					( MULTI_LEFT )
#define MULTI_SETUP_TOP						( MULTI_TOP + 30 )
											
#define MULTI_START_LEFT					( 90 )
#define MULTI_START_TOP						( MULTI_BUTTON_START_TOP )

#define MULTI_ARRAY_RIGHT					( MULTI_START_RIGHT )
#define MULTI_ARRAY_TOP						( MULTI_START_TOP - 5 )
#define MULTI_ARRAY_RIGHT_WIDTH				( MULTI_START_RIGHT_WIDTH )

#define MULTI_START_PLAYERS_LEFT			( MULTI_START_LEFT + MULTI_START_TIME_WIDTH + 6 )
#define MULTI_START_PLAYERS_TOP				( MULTI_START_TIME_TOP )
#define MULTI_START_PLAYERS_WIDTH			( MULTI_START_TIME_WIDTH )

#define MULTI_START_FRAG_LEFT				( MULTI_START_LEFT )
#define MULTI_START_FRAG_TOP				( MULTI_START_SKILL_TOP )
#define MULTI_START_FRAG_WIDTH				( MULTI_START_SKILL_WIDTH )

// configure weapons button
#define MULTI_WC_LEFT						( MULTI_START_LEFT + 8)
#define MULTI_WC_TOP						( MULTI_START_FRAG_TOP + 50 )

#define MULTI_FRIENDLY_FIRE_RADIO_LEFT		( MULTI_SETUP_LEFT + 20 )
#define MULTI_FRIENDLY_FIRE_RADIO_TOP		( 390 )

#define MULTI_TEAMPLAY_RADIO_LEFT			( MULTI_FRIENDLY_FIRE_RADIO_LEFT )
#define MULTI_TEAMPLAY_RADIO_TOP			( 350 )

// instagib radio
#define MULTI_INSTAGIB_RADIO_LEFT			( MULTI_FRIENDLY_FIRE_RADIO_LEFT )
#define MULTI_INSTAGIB_RADIO_TOP			( 370 )

#define MULTI_START_LEVEL_LIMIT_LEFT		( MULTI_START_LEFT + MULTI_START_TIME_WIDTH + 6  )
#define MULTI_START_LEVEL_LIMIT_TOP			( MULTI_START_FRAG_TOP )
#define MULTI_START_LEVEL_LIMIT_WIDTH		( MULTI_START_SKILL_WIDTH )

#define MULTI_SETUP_TEAM_LABEL_TOP			( MULTI_START_TOP + 175 )
#define MULTI_SETUP_TEAM_FIELD_TOP			( MULTI_SETUP_TEAM_LABEL_TOP + SPACE )

#define TEAM_CHOICE_PICKER_LEFT				( MULTI_START_LEVEL_LIMIT_LEFT )
#define TEAM_CHOICE_PICKER_TOP				( MULTI_START_LEVEL_LIMIT_TOP )

#define TEAM1_COLOR_PICKER_LEFT				( MULTI_START_LEFT + 40 )
#define TEAM1_COLOR_PICKER_TOP				( TEAM_CHOICE_PICKER_TOP + 45 )

#define TEAM2_COLOR_PICKER_LEFT				( MULTI_START_LEFT + 40 )
#define TEAM2_COLOR_PICKER_TOP				( TEAM1_COLOR_PICKER_TOP + 45 )

#define TEAM_CTF_LIMIT_LEFT					( MULTI_START_LEFT )
#define TEAM_CTF_LIMIT_TOP					( MULTI_START_SKILL_TOP )

#define COOP_EPISODE_PICKER_LEFT			( newgame_button_x(1) )//MULTI_START_LEFT + 70)
#define COOP_EPISODE_PICKER_TOP				( MULTI_START_MODE_TOP + 35 )
#define COOP_EPISODE_PICKER_WIDTH			( 100 )

#define COOP_CHARACTER_COLOR_PICKER_LEFT	( MULTI_START_LEFT + 110 )
#define COOP_CHARACTER_COLOR_PICKER_TOP		( COOP_EPISODE_PICKER_TOP )
#define COOP_CHARACTER_COLOR_PICKER_WIDTH	( COOP_EPISODE_PICKER_WIDTH )

#define COOP_TIME_LIMIT_PICKER_LEFT			( MULTI_START_LEFT + 220 )
#define COOP_TIME_LIMIT_PICKER_TOP			( COOP_CHARACTER_COLOR_PICKER_TOP )
#define COOP_TIME_LIMIT_PICKER_WIDTH		( COOP_CHARACTER_COLOR_PICKER_WIDTH )

// stuff to create a new-game style coop menu
#define NUM_NEWGAME_BUTTONS 3

#define NEWGAME_HEADER_X 220
#define NEWGAME_HEADER_Y 120

#define newgame_button_x(i)    (100 + (i) * NEWGAME_BUTTON_SPACING)
#define NEWGAME_BUTTON_Y       (340)
#define NEWGAME_BUTTON_SPACING (130)

#define skill_pic_x(i) (newgame_button_x(i) - 12)
#define SKILL_PIC_Y    (NEWGAME_BUTTON_Y - 260)


// pictures of Hiro icon for each skill level
static char * skill_pics[NUM_NEWGAME_BUTTONS] =
{
	"pics/menu/skill_0.tga",   // easy
	"pics/menu/skill_1.tga",   // medium
	"pics/menu/skill_2.tga"    // hard
};


// SCG[12/1/99]: End positional defines for start multiplayer game menu

// SCG[12/1/99]: ==============================================================================================
// SCG[12/1/99]: ==============================================================================================

//cek[12-14-99] check visibility of the radio button then run func if it is.
#define FLAG_RADIO_FUNC(radio,func)			(flag_control_array.ControlVisible(& radio) ? (radio.func) : (false))

#define	VALID_GAME_MODE(mode)				( (mode > 0) && (mode < 5) )
char *flag_cvars[][5] =
{
	//	deathmatch					ctf							 deathtag						coop						assign to
	{"m_dm_weapons_stay"		,"ctf_weapons_stay"			,"dt_weapons_stay"				,NULL						,"dm_weapons_stay"},
	{"m_dm_item_respawn"		,"ctf_item_respawn"			,"dt_item_respawn"				,NULL						,"dm_item_respawn"},
	{"m_dm_spawn_farthest"		,NULL						,"dt_spawn_farthest"			,NULL						,"dm_spawn_farthest"},
	{"m_dm_allow_exiting"		,NULL						,"dt_allow_exiting"				,NULL						,"dm_allow_exiting"},
	{"m_dm_same_map"			,"ctf_same_map"				,"dt_same_map"					,NULL						,"dm_same_map"},
	{"m_dm_force_respawn"		,"ctf_force_respawn"		,"dt_force_respawn"				,NULL						,"dm_force_respawn"},
	{"m_dm_falling_damage"		,"ctf_falling_damage"		,"dt_falling_damage"			,NULL						,"dm_falling_damage"},
	{"m_dm_allow_powerups"		,"ctf_allow_powerups"		,"dt_allow_powerups"			,NULL						,"dm_allow_powerups"},
	{"m_dm_allow_health"		,"ctf_allow_health"			,"dt_allow_health"				,NULL						,"dm_allow_health"},
	{"m_dm_allow_armor"			,"ctf_allow_armor"			,"dt_allow_armor"				,NULL						,"dm_allow_armor"},
	{"m_dm_unlimited_ammo"		,"ctf_unlimited_ammo"		,"dt_unlimited_ammo"			,NULL						,"dm_infinite_ammo"},
	{"m_dm_teamplay"			,NULL						,NULL							,NULL						,"dm_teamplay"},
	{"m_dm_friendly_fire"		,"ctf_friendly_fire"		,"dt_friendly_fire"				,"coop_friendly_fire"		,"dm_friendly_fire"},
	{"m_dm_fast_switch"			,"ctf_fast_switch"			,"dt_fast_switch"				,NULL						,"dm_fast_switch"},
	{"m_dm_footsteps"			,"ctf_footsteps"			,"dt_footsteps"					,NULL						,"dm_footsteps"},
	{"m_dm_shotshells"			,"ctf_shotshells"			,"dt_shotshells"				,NULL						,"dm_shotshells"},
	{"m_dm_levellimit"			,"ctf_levellimit"			,"dt_levellimit"				,NULL						,"dm_levellimit"},
	{"m_dm_use_skill_system"	,"ctf_use_skill_system"		,"dt_use_skill_system"			,NULL						,"dm_use_skill_system"},

	{"m_dm_timelimit"			,"ctf_timelimit"			,"dt_timelimit"					,"coop_timelimit"			,"timelimit"},
	{"m_dm_maxclients"			,"ctf_maxclients"			,NULL							,NULL						,NULL},//"maxclients"},
	{"m_dm_fraglimit"			,NULL						,NULL							,NULL						,"fraglimit"},
	{NULL						,NULL						,NULL							,NULL						,NULL},
	{NULL						,"ctf_limit"				,NULL							,NULL						,"fraglimit"},
	{"m_dm_team"				,"ctf_team"					,"dt_team"						,NULL						,"team"},
	{NULL						,NULL						,NULL							,NULL						,NULL},
	{NULL						,NULL						,NULL							,NULL						,NULL},
	{NULL						,NULL						,"dt_limit"						,NULL						,"fraglimit"},

	{"m_dm_weap_noallow"		,"ctf_weap_noallow"			,"dt_weap_noallow"				,NULL						,"dm_weap_noallow"},
	{"m_dm_instagib"			,"ctf_instagib"				,"dt_instagib"					,NULL						,"dm_instagib"}
};
/*
	{NULL						,"ctf_team_color_1"			,"dt_team_color_1"				,NULL						,NULL},
	{NULL						,"ctf_team_color_2"			,"dt_team_color_2"				,NULL						,NULL},
*/

#define CVAR_NAME_WEAPON_STAY(mode)					(flag_cvars[0][mode])
#define CVAR_NAME_ITEM_RESPAWN(mode)				(flag_cvars[1][mode])
#define CVAR_NAME_SPAWN_FARTHEST(mode)				(flag_cvars[2][mode])
#define CVAR_NAME_ALLOW_EXITING(mode)				(flag_cvars[3][mode])
#define CVAR_NAME_SAME_MAP(mode)					(flag_cvars[4][mode])
#define CVAR_NAME_FORCE_RESPAWN(mode)				(flag_cvars[5][mode])
#define CVAR_NAME_FALLING_DAMAGE(mode)				(flag_cvars[6][mode])
#define CVAR_NAME_ALLOW_POWERUPS(mode)				(flag_cvars[7][mode])
#define CVAR_NAME_ALLOW_HEALTH(mode)				(flag_cvars[8][mode])
#define CVAR_NAME_ALLOW_ARMOR(mode)					(flag_cvars[9][mode])
#define CVAR_NAME_INFINITE_AMMO(mode)				(flag_cvars[10][mode])
#define CVAR_NAME_TEAMPLAY(mode)					(flag_cvars[11][mode])
#define CVAR_NAME_FRIENDLY_FIRE(mode)				(flag_cvars[12][mode])
#define CVAR_NAME_FAST_SWITCH(mode)					(flag_cvars[13][mode])
#define CVAR_NAME_FOOTSTEPS(mode)					(flag_cvars[14][mode])
#define CVAR_NAME_SHOTSHELLS(mode)					(flag_cvars[15][mode])
#define CVAR_NAME_LEVELLIMIT(mode)					(flag_cvars[16][mode])
#define CVAR_NAME_SKILL_SYSTEM(mode)				(flag_cvars[17][mode])

#define CVAR_NAME_TIMELIMIT(mode)					(flag_cvars[18][mode])
#define CVAR_NAME_MAXCLIENTS(mode)					(flag_cvars[19][mode])
#define CVAR_NAME_FRAGLIMIT(mode)					(flag_cvars[20][mode])
#define CVAR_NAME_TEAM(mode)						(flag_cvars[21][mode])
#define CVAR_NAME_CAPTURELIMIT(mode)				(flag_cvars[22][mode])
#define CVAR_NAME_TEAMCHOICE(mode)					(flag_cvars[23][mode])
#define CVAR_NAME_TEAMCOLOR1(mode)					(flag_cvars[24][mode])
#define CVAR_NAME_TEAMCOLOR2(mode)					(flag_cvars[25][mode])
#define CVAR_NAME_TAGLIMIT(mode)					(flag_cvars[26][mode])

#define CVAR_NAME_WEAPONALLOW(mode)					(flag_cvars[27][mode])
#define CVAR_NAME_INSTAGIB(mode)					(flag_cvars[28][mode])

#define FLAG_WEAPON_1								0X00000001
#define FLAG_WEAPON_2								0X00000002
#define FLAG_WEAPON_3								0X00000004
#define FLAG_WEAPON_4								0X00000008
#define FLAG_WEAPON_5								0X00000010
#define FLAG_WEAPON_6								0X00000020

/*
"dm_weapons_stay"
"dm_item_respawn"
"dm_spawn_farthest"
"dm_allow_exiting"
"dm_same_map"
"dm_force_respawn"
"dm_falling_damage"
"dm_allow_powerups"
"dm_allow_health"
"dm_allow_armor"
"g_unlimited_ammo"
"dm_teamplay"
"dm_friendly_fire"
"dm_fast_switch"
"dm_footsteps"
"dm_shotshells"
"dm_levellimit"
"dm_use_skill_system"
*/
#define CS_566		CS_IDSN

/*
int rate_settings[] = {   
                          2500,  // 28.8k		    CS_288,
                          3200,  // 33.3k			CS_336,
                          5000,  // 56.0k			CS_ISDN,
                         10000,  // 128.0k			CS_DUALISDN,
                         25000,  // T1				CS_T1,
                             0   // custom			CS_CUSTOM
                      };
*/
int rate_settings[] = {   
                          2500,  // 28.8k		    CS_288,
                          5000,  // 56.0k			CS_ISDN,
                         10000,  // 128.0k			CS_DUALISDN,
						 15000,
                         25000,  // T1				CS_T1,
						 0
                      };

int maxplayer_settings[] = 
{
	2,3,4,6,8,12,16,24,32,0
};

static int team_color_convert[] = 
{
	4,5,2,3,6,7,8,1,2,8,1,5
};

// mdm 98.01.29 - we have fraglimit and timelimit, why do we need these?
//static cvar_t *dm_frag_limit = NULL;
//static cvar_t *dm_time_limit = NULL;
static cvar_t *dm_skill_level = NULL;
static cvar_t *dm_max_players = NULL;
static cvar_t *dm_weapons_stay = NULL;
static cvar_t *dm_item_respawn = NULL;
static cvar_t *dm_spawn_farthest = NULL;
static cvar_t *dm_allow_exiting = NULL;
static cvar_t *dm_same_map = NULL;
static cvar_t *dm_force_respawn = NULL;
static cvar_t *dm_falling_damage = NULL;
//static cvar_t *dm_instant_powerups = NULL;
static cvar_t *dm_allow_powerups = NULL;
static cvar_t *dm_allow_health = NULL;
static cvar_t *dm_allow_armor = NULL;
static cvar_t *g_unlimited_ammo = NULL;
static cvar_t *dm_fixed_fov = NULL;
static cvar_t *dm_teamplay = NULL;
static cvar_t *dm_friendly_fire = NULL;
static cvar_t *dm_fast_switch = NULL;
static cvar_t *dm_footsteps = NULL;
static cvar_t *dm_allow_hook = NULL;
static cvar_t *dm_shotshells = NULL;
static cvar_t *dm_levellimit = NULL;
static cvar_t *dm_use_skill_system = NULL;

static cvar_t *ctf_limit = NULL;
static cvar_t *ctf_team = NULL;

//a structure to hold the level name and bsp name.
typedef struct {
    char *print_name;
    char *map_name;
} episode_level;

//the maps for each episode.
static episode_level episode_1_levels[] = {
    //{"Level 1", "E1M1"},
    {"Level 1", "E1DM2"},  // e1dm2
    {"Level 2", "E1M2"},
    {"Level 3", "E1M3"},
    {"Level 4", "E1M4"},
    {"Level 5", "E1M5"},
    {"Level 6", "E1M6"},
    {"Level 7", "E1M7"},
    {"Level 8", "E1M8"},
};

static episode_level episode_2_levels[] = {
    {"Level 1", "E2M1"},
    {"Level 2", "E2M2"},
    {"Level 3", "E2M3"},
    {"Level 4", "E2M4"},
    {"Level 5", "E2M5"},
    {"Level 6", "E2M6"},
    {"Level 7", "E2M7"},
    {"Level 8", "E2M8"},
};

static episode_level episode_3_levels[] = {
    {"Level 1", "E3M1"},
    {"Level 2", "E3M2"},
    {"Level 3", "E3M3"},
    {"Level 4", "E3M4"},
    {"Level 5", "E3M5"},
    {"Level 6", "E3M6"},
    {"Level 7", "E3M7"},
    {"Level 8", "E3M8"},
};

static episode_level episode_4_levels[] = {
    {"Level 1", "E4M1"},
    {"Level 2", "E4M2"},
    {"Level 3", "E4M3"},
    {"Level 4", "E4M4"},
    {"Level 5", "E4M5"},
    {"Level 6", "E4M6"},
    {"Level 7", "E4M7"},
    {"Level 8", "E4M8"},
};

static episode_level deathmatch_levels[] = {
    {"Level 1", "E1DM1"},
    {"Level 2", "E1DM2"},
//    {"Level 3", "E2DM1"},
//    {"Level 4", "E2DM2"},
//    {"Level 5", "E3DM1"},
//    {"Level 6", "E3DM2"},
//    {"Level 7", "E4DM1"},
//    {"Level 8", "E4DM2"},
};

//the number of maps for each episode.
static int32 number_episode_maps[] = {
    sizeof(deathmatch_levels) / sizeof(episode_level),
    sizeof(episode_1_levels) / sizeof(episode_level),
    sizeof(episode_2_levels) / sizeof(episode_level),
    sizeof(episode_3_levels) / sizeof(episode_level),
    sizeof(episode_4_levels) / sizeof(episode_level),
};

//an array of the level arrays.
static episode_level *episode_levels[] = {
    deathmatch_levels,
    episode_1_levels,
    episode_2_levels,
    episode_3_levels,
    episode_4_levels,
};

int CMenuSubMultiplayer::GameModeIndex()
{
#ifdef DAIKATANA_DEMO
	int val = game_mode_picker.CurrentIndex();
	if (val == MPM_DEATHMATCH)
		return val;
	else
		return MPM_COOP;
#else
	return game_mode_picker.CurrentIndex();
#endif
}

#define MAX_LOCAL_GAMES 32

/*
//the daikatana server names of the local games that are displayed in the join menu.
static buffer256 local_game_server_names[MAX_LOCAL_GAMES];
//the ip addresses of the local games that are displayed in the join menu.
static netadr_t local_game_ips[MAX_LOCAL_GAMES];
*/

//the number of local games we have information on.
static int32 num_local_games = 0;

// local server info
typedef struct local_game_server_info_s
{
  char server_name[32];
  char map_name[32];
  char players[32];
  char gametype[32];
  netadr_t  address;
} local_game_server_info_t; 

local_game_server_info_t local_game_server_info[MAX_LOCAL_GAMES];


// WAW[11/23/99]: Added for GAMESPY
// Just call the member function.
static void multiplayerListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2)
{
	((CMenuSubMultiplayer *)instance)->ServerListCallBack( msg, param1, param2 );
}
// =============


//resets the list
void DKM_ResetLocalServerList() 
{
    num_local_games = 0;
	memset(&local_game_server_info,0,sizeof(local_game_server_info));
}

char *ExtractString(char *dst, char *src, char separator, int dstSize, int separatorCount = 1, char *def = "-")
{
	if (!dstSize || !separatorCount)
		return src;

	Com_sprintf(dst,dstSize,def);

	if (!src)
		return src;

	int idx = 0;
	while (*src && (*src != '\n') && (separatorCount > 0) && (idx < dstSize - 1))
	{
		dst[idx] = *src;
		idx++;
		src++;  // advance to next char

		if (*src == separator)
			separatorCount--;
	}
	
	dst[idx] = '\0';

	if (src)
		src++;
	return src;
}

#define SERVERNAME_WIDTH 200
qboolean DKM_CleanupServerInfo(char *server_name, netadr_t addr)
{	
	char buf[256];
	int idx;

	// rip out server name
	strcpy(buf,"");
	idx = 0;
/*	
	while (*server_name && *server_name != '\n' && *server_name != '/')
	{
		buf[idx] = *server_name;
		idx++;
		server_name++;  // advance to next char
	}
	
	buf[idx] = '\0';
*/	
	server_name = ExtractString(buf,server_name,'/',256);
	buffer256 print; //we dont use operator= here because it will call the constructor and interpret text as the format string.
	print = buf;
	
	//check if the text is too long for the field width
	int32 text_width = re.StringWidth(buf, menu_font, -1);
	if (text_width > SERVERNAME_WIDTH)
	{
		//get the width of the "..." we will put at the end.
		int32 dots_width = re.StringWidth("...", menu_font, -1);
		
		//the text is too wide, find out how many characters will fit in the field.
		int32 num_printable = re.FontNumCharsInWidth( print, menu_font, SERVERNAME_WIDTH - dots_width, NULL );
		
		//put the "..." at that position in the string.
		print.NewEnding(num_printable, "...");
	}
	
	strncpy(local_game_server_info[num_local_games].server_name,print,31);
	local_game_server_info[num_local_games].server_name[31] = '\0';
	
	// rip out map name
//	if (server_name)
//		server_name++;
/*	
	strcpy(buf,"");
	idx = 0;
	
	while (*server_name && *server_name != '\n' && *server_name != '/')
	{
		buf[idx] = *server_name;
		idx++;
		server_name++;
	}
	buf[idx] = '\0';
	
	if (server_name)
		server_name++;
*/
	server_name = ExtractString(buf,server_name,'/',256);
	strncpy(local_game_server_info[num_local_games].map_name,buf,31);
	local_game_server_info[num_local_games].map_name[31] = '\0';

/*	
	// WAW[12/7/99]: Copy the player out
	int cnt = 0;
	idx = 0;
	while (*server_name && *server_name != '\n')
	{
		if (*server_name == '/')
		{
			if (cnt >= 1)
				break;
			cnt++;
		}
		buf[idx] = *server_name;
		idx++;
		server_name++;
	}
	buf[idx] = '\0';
	if (server_name)
		server_name++;
*/
	server_name = ExtractString(buf,server_name,'/',256,2);
	strncpy(local_game_server_info[num_local_games].players,buf,31); // copy remains
	local_game_server_info[num_local_games].players[31] = '\0';
	
	// WAW[12/7/99]: Get the game type.
//	strncpy(local_game_server_info[num_local_games].gametype,(*server_name ? server_name:"-"),31); // copy remains
//	local_game_server_info[num_local_games].gametype[31] = '\0';
/*
	strcpy(buf,"");
	idx = 0;
	
	while (*server_name && *server_name != '\n' && *server_name != '/')
	{
		buf[idx] = *server_name;
		idx++;
		server_name++;
	}
	buf[idx] = '\0';
	
	if (server_name)
		server_name++;
*/	
	server_name = ExtractString(buf,server_name,'/',256);
	strncpy(local_game_server_info[num_local_games].gametype,buf,31);//(*buf ? buf:"-"),31);
	local_game_server_info[num_local_games].gametype[31] = '\0';
	
	server_name = ExtractString(buf,server_name,'/',256);
	netadr_t from;
	if (NET_StringToAdr(buf,&from))
	{
		if (NET_CompareBaseAdr(addr,from))
			return FALSE;
	}

	return TRUE;
}


//adds a local server name and ip address to our list of ip addresses.
void DKM_AddLocalServer(char *server_name, netadr_t addr)
{
    if (server_name == NULL) 
      return;

	// WAW[11/24/99]: Check for duplicate addresses.
	int i;
	for(i=0;i<num_local_games;i++)
	{
		if (NET_CompareAdr(local_game_server_info[i].address,addr))	// Address the same, just update the information.
		{
			int save = num_local_games;
			num_local_games = i;
			DKM_CleanupServerInfo( server_name, addr );
			num_local_games = save;
			return;
		}
	}
    //make sure we have room for this server name.
    if (num_local_games >= MAX_LOCAL_GAMES) 
      return;

    // clean up the format
    if (DKM_CleanupServerInfo(server_name, addr))
	{
		local_game_server_info[num_local_games].address = addr;

		//increment our count of local games.
		num_local_games++;
	}
}


static int32 number_episodes = sizeof(episode_levels) / sizeof(episode_level *);

// old_val of -1 will just make sure they aren't the same
bool CMenuSubMultiplayer::VerifyTeamColorPickers(CInterfacePicker &changing, CInterfacePicker &other,int old_val)
{
	int c = changing.CurrentIndex();
	int o = other.CurrentIndex();

	if (c == o)
	{
		if (old_val == -1)
		{
			c++;
		}
		else
		{
			if (c > old_val)
				c++;
			else
				c--;
		}

		int count = changing.StringCount();
		if (c >= count)
			c = 0;
		else if (c < 0)
			c = count-1;

		changing.SetCurrentString(c);
		return true;
	}
	return false;
}

CMenuSubMultiplayer::CMenuSubMultiplayer():
weapons()
{
	memset(&manual_server,0,sizeof(manual_server));
	// register all of the cool cvars as archivable
	char *temp_str;
	int count = sizeof(flag_cvars)/sizeof(flag_cvars[0]);
	for (int j = 0; j < count; j++)
	{
		for (int k = MPM_DEATHMATCH; k <= MPM_COOP; k++)
		{
			temp_str = flag_cvars[j][k];
			if (temp_str == NULL || temp_str[0] == '\0') continue;

			Cvar_Get(temp_str,"0",CVAR_ARCHIVE);
		}
	}

    SetCurrentMode(MM_JOIN);

    //place the separator line.
    separator.Init( MULTI_SEPARATOR_LEFT, MULTI_SEPARATOR_TOP, MULTI_SEPARATOR_RIGHT , 2);

    // 3.17.dsn  NO!  set initial connection speed by reading cvar
    //current_connection_speed = CS_T1;

    //place the local games list border.
    local_games_border.Init(MULTI_JOIN_LOCAL_LIST_LEFT - 2, MULTI_JOIN_LOCAL_LIST_TOP - 5, 
        MULTI_JOIN_LOCAL_LIST_RIGHT + 5, MULTI_JOIN_LOCAL_LIST_BOTTOM + 5, 2);

    local_list_top_index = 0;
    local_list_hilight_index = -1;
    net_list_top_index = 0;
    net_list_hilight_index = -1;

    refresh_state = MJRS_NONE;

    

    //fill in the strings for each picker.
    // TTD: replace with reading in a text file, or read in the maps directory with 
    //      a certain key that signals deathmatch maps

	fillEpisodePicker( "deathmatch" );
    //level_picker.AddString("Gibbler on the Roof");
    //level_picker.AddString("Storm Sector 7");

    frag_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);  // "none"
    frag_limit_picker.AddString("5");
    frag_limit_picker.AddString("10");
    frag_limit_picker.AddString("15");
    frag_limit_picker.AddString("20");
    frag_limit_picker.AddString("30");
    frag_limit_picker.AddString("40");
    frag_limit_picker.AddString("50");
    frag_limit_picker.SetCurrentString(0);  // default is None

  	game_mode_picker.AddString(tongue_menu[T_MENU_DEATHMATCH]);
#ifndef DAIKATANA_DEMO
	game_mode_picker.AddString(tongue_menu[ T_MENU_CTF ] );	// WAW[11/19/99]: CTF Menu.
    game_mode_picker.AddString(tongue_menu[T_MENU_DEATHTAG]);
#endif
    game_mode_picker.AddString(tongue_menu[T_MENU_COOPERATIVE]);
	

#ifndef REMOVE_SKILLS
    skill_level_picker.AddString(tongue_menu[T_MENU_EASY]);
    skill_level_picker.AddString(tongue_menu[T_MENU_MEDIUM]);
    skill_level_picker.AddString(tongue_menu[T_MENU_HARD]);
#endif

//	int curnum = 0;
    level_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);  // "none"
	char	temp[3];
	for( int i = 2; i <= 25; i++ )
	{
		temp[0] = temp[1] = temp[2] = 0x00;
// SCG[1/16/00]: 		sprintf( temp, itoa( i, temp, 10 ) );
		Com_sprintf( temp, sizeof(temp), itoa( i, temp, 10 ) );
		level_limit_picker.AddString( temp );
	}
    level_limit_picker.SetCurrentString(0);  // default is None

    score_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);  // "none"
	for( i = 5; i <= 20; i++ )
	{
		temp[0] = temp[1] = temp[2] = 0x00;
// SCG[1/16/00]: 		sprintf( temp, itoa( i, temp, 10 ) );
		Com_sprintf( temp, sizeof( temp ), itoa( i, temp, 10 ) );
		score_limit_picker.AddString( temp );
	}
    score_limit_picker.SetCurrentString(0);  // default is 5

	team_choice_picker.AddString(tongue_menu[T_MENU_AUTO]);
	team_choice_picker.AddString(tongue_ctf[T_CTF_RED_TEAM_NAME]);
	team_choice_picker.AddString(tongue_ctf[T_CTF_BLUE_TEAM_NAME]);
//	team_choice_picker.AddString(tongue_menu[T_MENU_TEAM_1]);
//	team_choice_picker.AddString(tongue_menu[T_MENU_TEAM_2]);
	team_choice_picker.SetCurrentString(0);

	join_team_choice_picker.AddString(tongue_menu[T_MENU_AUTO]);
	join_team_choice_picker.AddString(tongue_ctf[T_CTF_RED_TEAM_NAME]);
	join_team_choice_picker.AddString(tongue_ctf[T_CTF_BLUE_TEAM_NAME]);
//	join_team_choice_picker.AddString(tongue_menu[T_MENU_TEAM_1]);
//	join_team_choice_picker.AddString(tongue_menu[T_MENU_TEAM_2]);
	join_team_choice_picker.SetCurrentString(0);

    ctf_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);
    ctf_limit_picker.AddString("1");
    ctf_limit_picker.AddString("2");
    ctf_limit_picker.AddString("5");
    ctf_limit_picker.AddString("10");
    ctf_limit_picker.AddString("15");
    ctf_limit_picker.AddString("20");
    ctf_limit_picker.AddString("30");
    ctf_limit_picker.AddString("40");
	ctf_limit_picker.SetCurrentString(0);

    time_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);  // none
    time_limit_picker.AddString("5");
    time_limit_picker.AddString("10");
    time_limit_picker.AddString("15");
    time_limit_picker.AddString("20");
    time_limit_picker.AddString("30");
    time_limit_picker.SetCurrentString(0); // default is none

#ifndef DAIKATANA_DEMO
	// set up stuff for coop
    coop_episode_picker.AddString("1");
    coop_episode_picker.AddString("2");
    coop_episode_picker.AddString("3");
    coop_episode_picker.AddString("4");
    coop_episode_picker.SetCurrentString(0); // default is none
#endif
/*
    coop_time_limit_picker.AddString(tongue_menu[T_MENU_NONE_STRING]);  // none
    coop_time_limit_picker.AddString("5");
    coop_time_limit_picker.AddString("10");
    coop_time_limit_picker.AddString("15");
    coop_time_limit_picker.AddString("20");
    coop_time_limit_picker.AddString("30");
    coop_time_limit_picker.AddString("60");
    coop_time_limit_picker.AddString("90");
    coop_time_limit_picker.AddString("120");
    coop_time_limit_picker.AddString("180");
    coop_time_limit_picker.SetCurrentString(0); // default is none
*/	
	for(i=0;maxplayer_settings[i] != 0;i++)
	{
		char buf[32];
// SCG[1/16/00]: 		sprintf(buf,"%d",maxplayer_settings[i]);
		Com_sprintf(buf,sizeof(buf), "%d",maxplayer_settings[i]);
		max_players_picker.AddString( buf );
	}
    max_players_picker.SetCurrentString(4);

    //add the radio buttons to the control array.
    flag_control_array.AddControl(&weapon_stay_radio);
    flag_control_array.AddControl(&item_respawn_radio);
    flag_control_array.AddControl(&spawn_farthest_radio);
    flag_control_array.AddControl(&allow_exiting_radio);
    flag_control_array.AddControl(&same_map_radio);
    flag_control_array.AddControl(&force_respawn_radio);
    flag_control_array.AddControl(&falling_damage_radio);
    flag_control_array.AddControl(&allow_powerups_radio);
    flag_control_array.AddControl(&allow_health_radio);
    flag_control_array.AddControl(&allow_armor_radio);
    flag_control_array.AddControl(&infinite_ammo_radio);
//    flag_control_array.AddControl(&teamplay_radio);
//    flag_control_array.AddControl(&friendly_fire_radio);
    flag_control_array.AddControl(&fast_switch_radio);
    flag_control_array.AddControl(&footsteps_radio);
    flag_control_array.AddControl(&shotshells_radio);
	flag_control_array.AddControl(&skill_system_radio);
/*
	flag_control_array.AddControl(&weapon_allow[0]);
	flag_control_array.AddControl(&weapon_allow[1]);
	flag_control_array.AddControl(&weapon_allow[2]);
	flag_control_array.AddControl(&weapon_allow[3]);
	flag_control_array.AddControl(&weapon_allow[4]);
	flag_control_array.AddControl(&weapon_allow[5]);
*/
    SetJoinString("");

    modelname_picker.AddString("Hiro");
    modelname_picker.AddString("Mikiko");
    modelname_picker.AddString("Superfly");

	skincolor_picker.AddString(tongue_ctf[T_CTF_METAL_TEAM_NAME]);	// metal... default skin
	skincolor_picker.AddString(tongue_ctf[T_CTF_GREEN_TEAM_NAME]);	
	skincolor_picker.AddString(tongue_ctf[T_CTF_BLUE_TEAM_NAME]);		
	skincolor_picker.AddString(tongue_ctf[T_CTF_CHROME_TEAM_NAME]);		
	skincolor_picker.AddString(tongue_ctf[T_CTF_ORANGE_TEAM_NAME]);		
	skincolor_picker.AddString(tongue_ctf[T_CTF_PURPLE_TEAM_NAME]);		
	skincolor_picker.AddString(tongue_ctf[T_CTF_YELLOW_TEAM_NAME]);		
	skincolor_picker.AddString(tongue_ctf[T_CTF_RED_TEAM_NAME]);

#ifdef TONGUE_ENGLISH
	skincolor_picker.AddString("Dark Blue");
	skincolor_picker.AddString(tongue_skincolors[T_MENU_SKINCOLOR_4]);	// gold
	skincolor_picker.AddString("Maroon");
	skincolor_picker.AddString("Olive");
#else
	buffer64 colBuf("%s 2",tongue_ctf[T_CTF_BLUE_TEAM_NAME]);
	skincolor_picker.AddString(colBuf);
	colBuf.Set("%s",tongue_skincolors[T_MENU_SKINCOLOR_4]);
	skincolor_picker.AddString(colBuf);
	colBuf.Set("%s 2",tongue_ctf[T_CTF_RED_TEAM_NAME]);
	skincolor_picker.AddString(colBuf);
	colBuf.Set("%s 2",tongue_ctf[T_CTF_GREEN_TEAM_NAME]);
	skincolor_picker.AddString(colBuf);
#endif
    connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_1]);
    connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_2]);
    connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_3]);
    connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_4]);
    connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_5]);
    // connect_speed_picker.AddString(tongue_menu[T_MENU_CONNECTSPEED_6]);

	// WAW[11/23/99]: Added for gamespy.
	serverlist_state = SERVERLISTSTATE_NONE;
	// cek[1-11-00]: serverlist is actually a pointer...gotta set it to null
	serverlist = NULL;
	if (!serverlist)
	{
		char goa_secret_key[16];
		goa_secret_key[0] = 'f';
		goa_secret_key[1] = 'l';
		goa_secret_key[2] = '8';
		goa_secret_key[3] = 'a';
		goa_secret_key[4] = 'Y';
		goa_secret_key[5] = '7';
		goa_secret_key[6] = '\0';
		serverlist = ServerListNew(SERVER_NAME,"daikatana",goa_secret_key,10,
									multiplayerListCallBack,GCALLBACK_FUNCTION,(void *)this);
	}

	ctf_selected = false;

    //grab our cvars.
    dm_skill_level		= Cvar_Get("dm_skill_level", "0", CVAR_ARCHIVE);
    dm_max_players		= Cvar_Get("dm_max_players", "8", CVAR_ARCHIVE);
    dm_weapons_stay		= Cvar_Get("dm_weapons_stay", "1", CVAR_ARCHIVE);
    dm_item_respawn		= Cvar_Get("dm_item_respawn", "1", CVAR_ARCHIVE);
    dm_spawn_farthest	= Cvar_Get("dm_spawn_farthest", "1", CVAR_ARCHIVE);
    dm_allow_exiting	= Cvar_Get("dm_allow_exiting", "0", CVAR_ARCHIVE);
    dm_same_map			= Cvar_Get("dm_same_map", "0", CVAR_ARCHIVE);
    dm_force_respawn	= Cvar_Get("dm_force_respawn", "0", CVAR_ARCHIVE);
    dm_falling_damage	= Cvar_Get("dm_falling_damage", "1", CVAR_ARCHIVE);
    dm_allow_powerups	= Cvar_Get("dm_allow_powerups", "1", CVAR_ARCHIVE);
    dm_allow_health		= Cvar_Get("dm_allow_health", "1", CVAR_ARCHIVE);
    dm_allow_armor		= Cvar_Get("dm_allow_armor", "1", CVAR_ARCHIVE);
    g_unlimited_ammo	= Cvar_Get("g_unlimited_ammo", "0", CVAR_ARCHIVE);
    dm_teamplay			= Cvar_Get("dm_teamplay", "0", CVAR_ARCHIVE);
    dm_friendly_fire	= Cvar_Get("dm_friendly_fire", "0", CVAR_ARCHIVE);
    dm_fast_switch		= Cvar_Get("dm_fast_switch", "0", CVAR_ARCHIVE);
    dm_footsteps		= Cvar_Get("dm_footsteps", "1", CVAR_ARCHIVE);
    dm_allow_hook		= Cvar_Get("dm_allow_hook", "0", CVAR_ARCHIVE);
    dm_shotshells		= Cvar_Get("dm_shotshells", "1", CVAR_ARCHIVE);
	dm_levellimit		= Cvar_Get("dm_levellimit", "5", CVAR_ARCHIVE);//Cvar_Get(CVAR_NAME_LEVELLIMIT, "5", CVAR_ARCHIVE); 
	ctf_limit			= Cvar_Get("ctf_limit","0",CVAR_ARCHIVE);
    dm_use_skill_system = Cvar_Get("dm_use_skill_system", "1", CVAR_ARCHIVE);
	
// SCG[12/1/99]: Begin interface objects for main screen
    modelname = Cvar_Get ("modelname", DEFAULT_CHARACTER_MODEL, CVAR_USERINFO | CVAR_ARCHIVE | CVAR_NOSET );
    skinname  = Cvar_Get ("skinname",  "", CVAR_USERINFO | CVAR_ARCHIVE);
    skincolor = Cvar_Get ("skincolor", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    character = Cvar_Get ("character", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    ctf_team  = Cvar_Get ("ctf_team", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_Get ("dt_team", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_Get ("dm_weap_noallow","0",CVAR_SERVERINFO|CVAR_LATCH);
}


CMenuSubMultiplayer::~CMenuSubMultiplayer()
{
	if (serverlist && ( (int)serverlist != 0xcdcdcdcd ) )
	{	
		ServerListFree(serverlist);
		serverlist = NULL;
	}
}

void CMenuSubMultiplayer::FillLevelPicker(int32 episode_num)
{
    //reset the picker first.

#ifndef NO_EPISODE_SELECTOR
    episode_picker.Reset();

    if (episode_num < 0 || episode_num >= number_episodes) return;

    for (int32 i = 0; i < number_episode_maps[episode_num]; i++) {
        episode_picker.AddString(episode_levels[episode_num][i].print_name);
    }

    //make sure a valid level is selected.

    episode_picker.CheckCurrentString();
#endif

}


//sets the current mode.
void CMenuSubMultiplayer::SetCurrentMode(multiplayer_mode mode)
{
	current_mode = mode;
	if (current_mode != MM_SETUP)
		selected_mode = mode;

	//enable/disable the 3 main buttons.
// SCG[12/2/99]: 	join_server_button.Enable(current_mode != MM_JOIN);
// SCG[12/2/99]: 	host_server_button.Enable(current_mode != MM_HOST);
// SCG[12/2/99]: 	character_setup_button.Enable(current_mode != MM_SETUP);

}

// WAW[12/9/99]: Added so that it saves the changes when leaving.
void CL_WriteConfiguration (char *name);

void CMenuSubMultiplayer::Exit()
{
	Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_ASSIGN), team_color_convert[skincolor_picker.CurrentIndex()]);
//	Commit();
	CL_WriteConfiguration("current");
//	CL_WriteConfiguration(Cvar_VariableString("currentconfig"));
}

void CMenuSubMultiplayer::Enter()
{
	// set up the weapon configure button
	BtnConfigureWeapons.Init( MULTI_WC_LEFT, MULTI_WC_TOP, menu_font, menu_font_bright, button_font );
	BtnConfigureWeapons.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	BtnConfigureWeapons.SetText( tongue_menu[T_MENU_CONFIGURE_WEAPONS], true );

//	weapons.Activate("m_dm_weap_noallow");
//	return;

    SetJoinString("");
	// set up the mplayer logo thinggy
	float xScale = (viddef.width / 640.0);
	float yScale = (viddef.height / 480.0);
	picDrawStruct.nFlags = DSFLAG_SCALE|DSFLAG_BLEND|DSFLAG_ALPHA;
	picDrawStruct.fScaleX = (MULTI_MPLAYER_LOGO_WIDTH * xScale) / MULTI_MPLAYER_LOGO_IMGWIDTH;
	picDrawStruct.fScaleY = (MULTI_MPLAYER_LOGO_HEIGHT * yScale) / MULTI_MPLAYER_LOGO_IMGHEIGHT;
	picDrawStruct.nXPos = MULTI_MPLAYER_LOGO_LEFT * xScale;
	picDrawStruct.nYPos = MULTI_MPLAYER_LOGO_TOP * yScale;
	picDrawStruct.fAlpha = 1.0;

	// set up the gamespy logo thinggy
	picGamespy.nFlags = DSFLAG_SCALE|DSFLAG_ALPHA|DSFLAG_BLEND;
	picGamespy.fScaleX = (MULTI_GSPY_LOGO_WIDTH * xScale) / MULTI_GSPY_LOGO_IMGWIDTH;
	picGamespy.fScaleY = (MULTI_GSPY_LOGO_HEIGHT * yScale) / MULTI_GSPY_LOGO_IMGHEIGHT;
	picGamespy.nXPos = MULTI_GSPY_LOGO_LEFT * xScale;
	picGamespy.nYPos = MULTI_GSPY_LOGO_TOP * yScale;
	picGamespy.fAlpha = 1.0;


	int num;

    gameconnect  = GAMECONNECT_NONE;  // no game initially selected
    current_mode = MM_SETUP;			// WAW[11/24/99]: Go into setup first.
	selected_mode = MM_SETUP;

    if (character->value < 0 || character->value > 2) // boundary check
      Cvar_Set("character", "0");

#ifdef JPN
	button_find_local.Init( MULTI_FIND_LOCAL_GAME_LEFT, MULTI_FIND_LOCAL_GAME_TOP, menu_font, menu_font_bright, button_font );
	button_find_local.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	button_find_local.SetText( tongue_menu[T_MENU_FIND_LOCAL], true );
	button_start_multiplayer.Init( MULTI_START_MULTIPLAYER_GAME_LEFT, MULTI_START_MULTIPLAYER_GAME_TOP, menu_font, menu_font_bright, button_font );
	button_start_multiplayer.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	button_start_multiplayer.SetText( tongue_menu[T_MENU_START_MULTIPLAYER], true );
#else
	button_find_internet.Init( MULTI_FIND_INTERNET_GAME_LEFT, MULTI_FIND_INTERNET_GAME_TOP ,menu_font, menu_font_bright, button_font );
	button_find_internet.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	button_find_internet.SetText( tongue_menu[T_MENU_FIND_INTERNET], true );
	button_find_local.Init( MULTI_FIND_LOCAL_GAME_LEFT, MULTI_FIND_LOCAL_GAME_TOP, menu_font, menu_font_bright, button_font );
	button_find_local.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	button_find_local.SetText( tongue_menu[T_MENU_FIND_LOCAL], true );
	button_start_multiplayer.Init( MULTI_START_MULTIPLAYER_GAME_LEFT, MULTI_START_MULTIPLAYER_GAME_TOP, menu_font, menu_font_bright, button_font );
	button_start_multiplayer.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	button_start_multiplayer.SetText( tongue_menu[T_MENU_START_MULTIPLAYER], true );

	// mplayer buttons
	confirm_dlg.Init(CONFIRM_LEFT,CONFIRM_TOP,CONFIRM_RIGHT,CONFIRM_BOTTOM,2,CONFIRM_BUTTON_TOP,CONFIRM_BUTTON_LEFT(1),CONFIRM_BUTTON_LEFT(2));
	mplayer_exec_mode = -1;

    mplayer_start_button.Init(MULTI_START_MPLAYER_LEFT,MULTI_START_MPLAYER_TOP,menu_font, menu_font_bright, button_font);     
	mplayer_start_button.InitGraphics( BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING );
	mplayer_start_button.SetText( tongue_menu[T_MENU_MPLAYER_GAME], true );
    mplayer_community_button.Init(MULTI_MPLAYER_COMMUNITY_LEFT,MULTI_MPLAYER_COMMUNITY_TOP,menu_font, menu_font_bright, button_font);
	mplayer_community_button.InitGraphics( BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING );
	mplayer_community_button.SetText( tongue_menu[T_MENU_MPLAYER_COMM], true );
#endif

    //get the current player name setting.
    player_name = Cvar_VariableString("name");
    player_name.StripInvalidChars(16);
	
    //set up the player name field.
    player_name_field.Init( MULTI_SETUP_NAME_LABEL_LEFT, MULTI_SETUP_NAME_FIELD_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR );
    player_name_field.SetText( player_name );

    // 3.17.99 dsn  read connection rate from config file
    cvar_t * temp_cvar;
    temp_cvar = Cvar_Get("rate", "25000", CVAR_ARCHIVE);

	int i;
	for(i=0;i<4;i++)
	{
		if ((int)temp_cvar->value <= rate_settings[i])
			break;
	}
    connect_speed_picker.Init( MULTI_SETUP_LEFT, MULTI_SETUP_SPEED_TOP, SPEEDPICKER_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_CONNECTSPEED] );
    connect_speed_picker.SetCurrentString( i ); // current_connection_speed );

	// WAW[12/9/99]: Added default frag limit.
	cvar_t *fraglimit = Cvar_Get("fraglimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	if (fraglimit->value < 0.0f) num = 0;
	else if (fraglimit->value < 7.5f) num = 1;
	else if (fraglimit->value < 12.5f) num = 2;
	else if (fraglimit->value < 17.5f) num = 3;
	else if (fraglimit->value < 25.0f) num = 4;
	else if (fraglimit->value < 35.0f) num = 5;
	else if (fraglimit->value < 12.5f) num = 6;
	else						  num = 7;
    frag_limit_picker.SetCurrentString( num );

	// WAW[12/9/99]: Added default level limit 
	level_limit_picker.SetCurrentString((dm_levellimit->value >= 2 ? (int)(dm_levellimit->value-1):0)); 

	// WAW[12/9/99]: Added default time limit.
	cvar_t *timelimit = Cvar_Get("timelimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	if (timelimit->value < 2.5f) num = 0;
	else if (timelimit->value < 7.5f) num = 1;
	else if (timelimit->value < 12.5f) num = 2;
	else if (timelimit->value < 17.5f) num = 3;
	else if (timelimit->value < 25.0f) num = 4;
	else num = 5;
	time_limit_picker.SetCurrentString( num );

	// WAW[12/9/99]: Added default max players.
	for(i = 0;maxplayer_settings[i+1] != 0;i++)
	{
		if ((int)dm_max_players->value <= maxplayer_settings[i])
			break;
	}
    max_players_picker.SetCurrentString(i);
		
    // set selector based on modelname
    modelname_picker.Init( MODELNAME_X, MODELNAME_Y, MODELNAME_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_MODELNAME] );
    modelname_picker.SetCurrentString( character->value );
	
    skincolor_picker.Init(SKINCOLOR_X, SKINCOLOR_Y, SKINCOLOR_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_SKINCOLOR] );
    skincolor_picker.SetCurrentString( skincolor->value );

    //get the current server name.
	server_name = Cvar_VariableString("hostname");
	server_name.StripInvalidChars(32);

    //set up the server name input field.
    server_name_field.Init(MULTI_START_SERVERNAME_LEFT, MULTI_START_SERVERNAME_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR);
    server_name_field.SetText(server_name);

    register_character_model(true); // make sure initial model/skins are registered 
    character_model_rotation = CHARACTER_MODEL_START_ANGLE;  // set initial angle for model ONCE
// SCG[12/1/99]: End interface objects for main screen

	
	
// SCG[12/1/99]: Begin multiplayer menu
	go_button.Init(MULTI_START_GO_LEFT, MULTI_START_GO_TOP, menu_font, menu_font_bright, button_font);
	go_button.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
//	go_button.InitGraphics(BUTTONFAT_UP_STRING, BUTTONFAT_DOWN_STRING, BUTTONFAT_DISABLE_STRING);
	go_button.SetText(tongue_menu[T_MENU_MULTIPLAYER_GO], true);

	// WAW[12/3/99]: Back gutton.
/*	hostback_button.Init(MULTI_HOSTBACK_LEFT, MULTI_HOSTBACK_TOP, menu_font, menu_font_bright, button_font);
	hostback_button.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
	hostback_button.SetText(tongue_menu[T_MENU_MULTIPLAYER_BACK], true);

	joinback_button.Init(MULTI_JOINBACK_LEFT, MULTI_JOINBACK_TOP, menu_font, menu_font_bright, button_font);
	joinback_button.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
	joinback_button.SetText(tongue_menu[T_MENU_MULTIPLAYER_BACK], true);
*/
	hostback_button.Init(MULTI_HOSTBACK_LEFT, MULTI_HOSTBACK_TOP, menu_font, menu_font_bright, button_font);
	hostback_button.InitGraphics(BUTTONBACK_UP_STRING, BUTTONBACK_DOWN_STRING, BUTTONBACK_DISABLE_STRING);
	hostback_button.SetText("", true);

	joinback_button.Init(MULTI_JOINBACK_LEFT, MULTI_JOINBACK_TOP, menu_font, menu_font_bright, button_font);
	joinback_button.InitGraphics(BUTTONBACK_UP_STRING, BUTTONBACK_DOWN_STRING, BUTTONBACK_DISABLE_STRING);
	joinback_button.SetText("", true);
	
    //check what mode is selected.
	const char *game_mode_string = game_mode_picker.CurrentString();
	if (game_mode_string == NULL || game_mode_string[0] == '\0') return;
	if (stricmp(game_mode_string, tongue_menu[T_MENU_DEATHMATCH]) == 0)
		go_button.Enable(true);
	else
		go_button.Enable(false);


    //set up the start game option radio buttons.  we give 0, 0 for the position, because
    //the flag array control will position them.
    weapon_stay_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_WEAPONS_STAY] );
    item_respawn_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_ITEMS_RESPAWN] );
    spawn_farthest_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_SPAWN_FARTHEST] );
    allow_exiting_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_ALLOW_EXITING] );
    same_map_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_SAME_MAP] );
    force_respawn_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_FORCE_RESPAWN] );
    falling_damage_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_FALLING_DAMAGE] );
    allow_powerups_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_ALLOW_POWERUPS] );
    allow_health_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_ALLOW_HEALTH] );
    allow_armor_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_ALLOW_ARMOR] );
    infinite_ammo_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_INFINITE_AMMO] );
    fast_switch_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_FAST_SWITCH] );
    footsteps_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_FOOTSTEPS] );
    shotshells_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_SHOTSHELLS] );
    skill_system_radio.Init( 0, 0, menu_font, menu_font_bright, tongue_menu[T_MENU_SKILL_SYSTEM] );

/*	char temp[16];
	for (i = 0; i < 6; i++)
	{
		Com_sprintf(temp,sizeof(temp),tongue_menu[T_MENU_WEAPON],i+1);
		weapon_allow[i].Init( 0, 0, menu_font, menu_font_bright, temp );
	}

	weapon_allow[0].Init( 0, 0, menu_font, menu_font_bright, "Weapon 1" );
	weapon_allow[1].Init( 0, 0, menu_font, menu_font_bright, "Weapon 2" );
	weapon_allow[2].Init( 0, 0, menu_font, menu_font_bright, "Weapon 3" );
	weapon_allow[3].Init( 0, 0, menu_font, menu_font_bright, "Weapon 4" );
	weapon_allow[4].Init( 0, 0, menu_font, menu_font_bright, "Weapon 5" );
	weapon_allow[5].Init( 0, 0, menu_font, menu_font_bright, "Weapon 6" );
*/
    // SCG[12/1/99]: Init the control array

	flag_control_array.Init( MULTI_ARRAY_RIGHT, MULTI_ARRAY_TOP, MULTI_ARRAY_RIGHT_WIDTH, 10, 20 );
    flag_control_array.InitScrollButtons( menu_font, menu_font_bright, button_font );

	time_limit_picker.Init( MULTI_START_TIME_LEFT, MULTI_START_TIME_TOP, MULTI_START_TIME_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_TIME_LIMIT] );
	
	score_limit_picker.Init( MULTI_START_PLAYERS_LEFT, MULTI_START_PLAYERS_TOP, MULTI_START_PLAYERS_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_SCORE_LIMIT] );

	max_players_picker.Init( MULTI_START_PLAYERS_LEFT, MULTI_START_PLAYERS_TOP, MULTI_START_PLAYERS_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_MAX_PLAYERS] );
	
	frag_limit_picker.Init( MULTI_START_FRAG_LEFT, MULTI_START_FRAG_TOP, MULTI_START_FRAG_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_FRAG_LIMIT] );
	
	game_mode_picker.Init( MULTI_START_MODE_LEFT, MULTI_START_MODE_TOP, MULTI_START_MODE_WIDTH, menu_font, menu_font_bright, button_font, NULL );
    
	teamplay_radio.Init( MULTI_TEAMPLAY_RADIO_LEFT, MULTI_TEAMPLAY_RADIO_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_TEAMPLAY] );
	friendly_fire_radio.Init( MULTI_FRIENDLY_FIRE_RADIO_LEFT, MULTI_FRIENDLY_FIRE_RADIO_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_MPO_FRIENDLY_FIRE] );
    instagib_radio.Init( MULTI_INSTAGIB_RADIO_LEFT, MULTI_INSTAGIB_RADIO_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_INSTAGIB] );
    
	
#ifndef NO_EPISODE_SELECTOR
	episode_picker.Init(MULTI_START_EPISODE_LEFT, MULTI_START_EPISODE_TOP, MULTI_START_EPISODE_WIDTH, menu_font, menu_font_bright, button_font, NULL);
#endif 

	level_limit_picker.Init( MULTI_START_LEVEL_LIMIT_LEFT, MULTI_START_LEVEL_LIMIT_TOP, MULTI_START_LEVEL_LIMIT_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_LEVEL_LIMIT] );

//    team_name = Cvar_VariableString("team");
//    team_name.StripInvalidChars( 16 );
//    team_name_field.Init( MULTI_SETUP_LEFT + 20, MULTI_SETUP_TEAM_FIELD_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR );
//    team_name_field.SetText( team_name );

//	team1_color_picker.Init( TEAM1_COLOR_PICKER_LEFT, TEAM1_COLOR_PICKER_TOP, MULTI_START_LEVEL_LIMIT_WIDTH * 1.25, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_TEAM1_COLOR] );
//	team2_color_picker.Init( TEAM2_COLOR_PICKER_LEFT, TEAM2_COLOR_PICKER_TOP, MULTI_START_LEVEL_LIMIT_WIDTH * 1.25, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_TEAM2_COLOR] );
	team_choice_picker.Init( TEAM_CHOICE_PICKER_LEFT, TEAM_CHOICE_PICKER_TOP, MULTI_START_LEVEL_LIMIT_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_TEAM_CHOICE] );
	join_team_choice_picker.Init( JOIN_TEAM_CHOICE_PICKER_LEFT, JOIN_TEAM_CHOICE_PICKER_TOP, JOIN_MULTI_START_LEVEL_LIMIT_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_TEAM_CHOICE] );

	if (ctf_limit->value < 0.5f) num = 0;
	else if (ctf_limit->value <= 1.5f) num = 1;
	else if (ctf_limit->value <  3.5f) num = 2;
	else if (ctf_limit->value <  7.5f) num = 3;
	else if (ctf_limit->value < 12.5f) num = 4;
	else if (ctf_limit->value < 17.5f) num = 5;
	else if (ctf_limit->value < 25.0f) num = 6;
	else if (ctf_limit->value < 35.0f) num = 7;
	else num = 8;
	ctf_limit_picker.SetCurrentString(num);
    ctf_limit_picker.Init( TEAM_CTF_LIMIT_LEFT, TEAM_CTF_LIMIT_TOP, MULTI_START_LEVEL_LIMIT_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_CTF_LIMIT] );
// SCG[12/1/99]: End multiplayer menu
	
//    coop_time_limit_picker.Init( COOP_TIME_LIMIT_PICKER_LEFT, COOP_TIME_LIMIT_PICKER_TOP, COOP_TIME_LIMIT_PICKER_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_COOP_TIME_LIMIT] );
//    coop_character_color_picker.Init( COOP_CHARACTER_COLOR_PICKER_LEFT, COOP_CHARACTER_COLOR_PICKER_TOP, COOP_CHARACTER_COLOR_PICKER_WIDTH,   menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_COOP_COLOR] );
#ifndef DAIKATANA_DEMO
    coop_episode_picker.Init( COOP_EPISODE_PICKER_LEFT, COOP_EPISODE_PICKER_TOP, COOP_EPISODE_PICKER_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_COOP_EPISODE] );
#endif
// SCG[12/2/99]: 	character_setup_button.Init(MULTI_TOP_CHARACTER_LEFT, MULTI_TOP, menu_font, menu_font_bright, button_font);
// SCG[12/2/99]: 	character_setup_button.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
// SCG[12/2/99]: 	character_setup_button.SetText(tongue_menu[T_MENU_SETUP_PLAYER], true);
	
    //set up controls for start game screen.
	// SCG[12/2/99]: episode picker for coop mode
	level_picker.Init(MULTI_START_LEVEL_LEFT, MULTI_START_LEVEL_TOP, MULTI_START_LEVEL_WIDTH, menu_font, menu_font_bright, button_font, NULL);
 	FillLevelPicker(level_picker.CurrentIndex());
    
#ifndef REMOVE_SKILLS
	skill_level_picker.Init(MULTI_START_SKILL_LEFT,   MULTI_START_SKILL_TOP,   MULTI_START_SKILL_WIDTH,   menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_SKILL_LEVEL]);
#endif
    // set up controls in the join game screen.
    join_name_field.Init(MULTI_JOIN_NAME_LEFT, MULTI_JOIN_NAME_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR);
	
    play_button.Init(MULTI_JOIN_PLAY_LEFT, MULTI_JOIN_PLAY_TOP, menu_font, menu_font_bright, button_font);
    play_button.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    play_button.SetText(tongue_menu[T_MENU_JOIN_SERVER], true);
	
    /*
    add_button.Init(MULTI_JOIN_ADD_LEFT, MULTI_JOIN_ADD_TOP, menu_font, menu_font_bright, button_font);
    add_button.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
	add_button.SetText(tongue_menu[T_MENU_MP_ADD], true);
    */
	
    refresh_button.Init(MULTI_JOIN_REFRESH_LEFT, MULTI_JOIN_REFRESH_TOP, menu_font, menu_font_bright, button_font);
    refresh_button.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
	refresh_button.SetText(tongue_menu[T_MENU_MP_REFRESH], true);
	
	
    //setting the mode here will allow the 3 main buttons to be enabled/disabled correctly.
    SetCurrentMode( current_mode );
	
	// WAW[11/24/99]: Scroll stuff.
	local_games_up_button.Init( MULTI_JOIN_LOCAL_LIST_UP_LEFT, MULTI_JOIN_LOCAL_LIST_UP_TOP,
								menu_font, menu_font_bright, button_font);
    local_games_up_button.InitGraphics(TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING);
	local_games_up_button.SetText(ARROW_UP_STRING, true);
	
	
	local_games_down_button.Init( MULTI_JOIN_LOCAL_LIST_DOWN_LEFT, MULTI_JOIN_LOCAL_LIST_DOWN_TOP,
								menu_font, menu_font_bright, button_font);
    local_games_down_button.InitGraphics(TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING);
	local_games_down_button.SetText(ARROW_DOWN_STRING, true);								

	// set up the coop skill selector
    for (i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        start[i].Init(newgame_button_x(i), NEWGAME_BUTTON_Y, menu_font, menu_font_bright, button_font);
        start[i].InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
        start[i].SetText(tongue_menu_options[T_MENU_NEWGAME_SKILL_EASY + i], true);
	}

//	SetTeamChoicePicker(); // WAW[12/9/99]
	setGameMode();
}

void CMenuSubMultiplayer::Animate(int32 elapsed_time)
{
	if (weapons.Active())
	{
		weapons.Animate(elapsed_time);
		return;
	}

	if (confirm_dlg.Showing())
	{
		confirm_dlg.Animate(elapsed_time);
		return;
	}
	// WAW[11/24/99]: Auto move the thing down if nessary.
	if (local_games_up_button.IsDown())
	{
		buttonmove_timer -= elapsed_time;
		while (buttonmove_timer <= 0)
		{
			MoveJoinUp();
			buttonmove_timer += JOINMOVE_REPEAT;
		}
	}
	if (local_games_down_button.IsDown())
	{
		buttonmove_timer -= elapsed_time;
		while (buttonmove_timer <= 0)
		{
			MoveJoinDown();
			buttonmove_timer += JOINMOVE_REPEAT;
		}
	}
	
	
	if (serverlist_state > SERVERLISTSTATE_IDLE)
	{
		if (ServerListThink(serverlist) == GE_NOCONNECT)
		{
			serverlist_state		= SERVERLISTSTATE_NONE;
			ServerListHalt( serverlist );
			ServerListClear( serverlist );
		}
	}
    character_model_timer += elapsed_time;
	
    //check if the setup subsubmenu is active
    if (current_mode == MM_SETUP &&   
        character_model_timer > CHARACTER_MODEL_ANIM_DELAY) 
    {
		character_model_timer = 0;
		character_model_frame++;
		if (character_model_frame >= character_model_frame_end)
			character_model_frame = character_model_frame_start;
		
		// rotate model
		character_model_rotation += CHARACTER_MODEL_ROTATION;
    }

	if ((current_mode == MM_HOST) && (GameModeIndex() == MPM_COOP) )//(game_mode_picker.CurrentIndex() == MPM_COOP))
	{
		for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
		{
			start[i].Animate(elapsed_time);
		}
	}
}

void CMenuSubMultiplayer::DrawBackground()
{
	if (weapons.Active())
	{
		weapons.DrawBackground();
		return;
	}

	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawBackground();
		return;
	}

	if( current_mode == MM_SETUP )
	{
		mplayer_start_button.DrawBackground();
		mplayer_community_button.DrawBackground();
		button_find_internet.DrawBackground();
		button_find_local.DrawBackground();
		button_start_multiplayer.DrawBackground();
		player_name_field.DrawBackground();
		server_name_field.DrawBackground();
		modelname_picker.DrawBackground();
		skincolor_picker.DrawBackground();
		connect_speed_picker.DrawBackground();
	}
	else if( current_mode == MM_HOST )
	{
		// SCG[12/2/99]: Common to all menus
		game_mode_picker.DrawBackground();
		hostback_button.DrawBackground(); // WAW[12/3/99]: Added

		switch( GameModeIndex() )//game_mode_picker.CurrentIndex() )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			instagib_radio.DrawBackground();
			go_button.DrawBackground();
			BtnConfigureWeapons.DrawBackground();
//			team_name_field.DrawBackground();
			score_limit_picker.DrawBackground();
			team_choice_picker.DrawBackground();
//			team1_color_picker.DrawBackground();
//			team2_color_picker.DrawBackground();
			level_picker.DrawBackground();
			flag_control_array.DrawBackground();
			break;
		case 1:		// WAW[11/19/99]: CTF
			instagib_radio.DrawBackground();
			BtnConfigureWeapons.DrawBackground();
			go_button.DrawBackground();
			score_limit_picker.DrawBackground();
			team_choice_picker.DrawBackground();
//			team1_color_picker.DrawBackground();
//			team2_color_picker.DrawBackground();
			ctf_limit_picker.DrawBackground();
			level_picker.DrawBackground();
			flag_control_array.DrawBackground();
			break;
		case 2:		// SCG[10/29/99]: Deathtag
			instagib_radio.DrawBackground();
			BtnConfigureWeapons.DrawBackground();
			go_button.DrawBackground();
			flag_control_array.DrawBackground();
			break;			
		case 3:		// SCG[10/29/99]: Cooperative
			for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++) 
			{
				start[i].DrawBackground();
			}
//			coop_time_limit_picker.DrawBackground();
//			coop_character_color_picker.DrawBackground();
#ifndef DAIKATANA_DEMO
			coop_episode_picker.DrawBackground();
#endif
			break;
		}

	}
	else if( current_mode == MM_JOIN )
	{
		join_name_field.DrawBackground();
		joinback_button.DrawBackground();	// WAW[12/3/99]: Added
		
		play_button.DrawBackground();
		//add_button.DrawBackground();
		
		refresh_button.DrawBackground();
		
		local_games_up_button.DrawBackground();
		local_games_down_button.DrawBackground();
		if (ctf_selected)
			join_team_choice_picker.DrawBackground();
	}
}

void CMenuSubMultiplayer::DrawGameModeDeathmatch()
{
	BtnConfigureWeapons.DrawForeground();
	level_picker.DrawForeground();
    time_limit_picker.DrawForeground();
    max_players_picker.DrawForeground();
    frag_limit_picker.DrawForeground();
    level_limit_picker.DrawForeground();

    teamplay_radio.DrawForeground();
    SetRadioState(teamplay_radio, CVAR_NAME_TEAMPLAY(MPM_DEATHMATCH));

//	DKM_DrawString( MULTI_SETUP_LEFT + 20, MULTI_SETUP_TEAM_LABEL_TOP - 2, tongue_menu[T_MENU_TEAM_NAME], NULL, false, false );
//	team_name_field.DrawForeground();

    SetRadioState( instagib_radio		, CVAR_NAME_INSTAGIB(MPM_DEATHMATCH) );
    //set the value of the radio button controls.
    SetRadioState( weapon_stay_radio		, CVAR_NAME_WEAPON_STAY(MPM_DEATHMATCH) );
    SetRadioState( item_respawn_radio		, CVAR_NAME_ITEM_RESPAWN(MPM_DEATHMATCH) );
    SetRadioState( spawn_farthest_radio		, CVAR_NAME_SPAWN_FARTHEST(MPM_DEATHMATCH) );
    SetRadioState( allow_exiting_radio		, CVAR_NAME_ALLOW_EXITING(MPM_DEATHMATCH) );
    SetRadioState( same_map_radio			, CVAR_NAME_SAME_MAP(MPM_DEATHMATCH) );
    SetRadioState( force_respawn_radio		, CVAR_NAME_FORCE_RESPAWN(MPM_DEATHMATCH) );
    SetRadioState( falling_damage_radio		, CVAR_NAME_FALLING_DAMAGE(MPM_DEATHMATCH) );
    SetRadioState( allow_powerups_radio		, CVAR_NAME_ALLOW_POWERUPS(MPM_DEATHMATCH) );
    SetRadioState( allow_health_radio		, CVAR_NAME_ALLOW_HEALTH(MPM_DEATHMATCH) );
    SetRadioState( allow_armor_radio		, CVAR_NAME_ALLOW_ARMOR(MPM_DEATHMATCH) );
    SetRadioState( infinite_ammo_radio		, CVAR_NAME_INFINITE_AMMO(MPM_DEATHMATCH) );
    SetRadioState( fast_switch_radio		, CVAR_NAME_FAST_SWITCH(MPM_DEATHMATCH) );
    SetRadioState( footsteps_radio			, CVAR_NAME_FOOTSTEPS(MPM_DEATHMATCH) );
    SetRadioState( shotshells_radio			, CVAR_NAME_SHOTSHELLS(MPM_DEATHMATCH) );
    SetRadioState( skill_system_radio		, CVAR_NAME_SKILL_SYSTEM(MPM_DEATHMATCH) );
/*
	unsigned long flag;
	int i;
	for (i = 0, flag = 1; i < 6; i++,flag <<= 1)
	{
	    SetRadioStateFlag( weapon_allow[i], CVAR_NAME_WEAPONALLOW(MPM_DEATHMATCH), flag ,true);
	}
*/
	instagib_radio.DrawForeground();
    //draw the option radio buttons.
	flag_control_array.DrawForeground();

	FLAG_RADIO_FUNC(weapon_stay_radio		,DrawForeground());
	FLAG_RADIO_FUNC(item_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(spawn_farthest_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_exiting_radio		,DrawForeground());
	FLAG_RADIO_FUNC(same_map_radio			,DrawForeground());
	FLAG_RADIO_FUNC(force_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(falling_damage_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_powerups_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_health_radio		,DrawForeground());
	FLAG_RADIO_FUNC(allow_armor_radio		,DrawForeground());
	FLAG_RADIO_FUNC(infinite_ammo_radio		,DrawForeground());
	FLAG_RADIO_FUNC(fast_switch_radio		,DrawForeground());
	FLAG_RADIO_FUNC(footsteps_radio			,DrawForeground());
	FLAG_RADIO_FUNC(shotshells_radio		,DrawForeground());
	FLAG_RADIO_FUNC(skill_system_radio		,DrawForeground());
/*
	for (i = 0; i < 6; i++)
	{
		FLAG_RADIO_FUNC(weapon_allow[i]		,DrawForeground());
	}
*/
}

void CMenuSubMultiplayer::DrawGameModeCTF()
{
	BtnConfigureWeapons.DrawForeground();
	level_picker.DrawForeground();
    time_limit_picker.DrawForeground();
    max_players_picker.DrawForeground();
	ctf_limit_picker.DrawForeground();
    team_choice_picker.DrawForeground();
//	team1_color_picker.DrawForeground();
//	team2_color_picker.DrawForeground();
	team_choice_picker.DrawForeground();

    SetRadioState( instagib_radio		, CVAR_NAME_INSTAGIB(MPM_CTF) );
    //set the value of the radio button controls.
    SetRadioState( weapon_stay_radio		, CVAR_NAME_WEAPON_STAY(MPM_CTF) );
    SetRadioState( item_respawn_radio		, CVAR_NAME_ITEM_RESPAWN(MPM_CTF) );
    SetRadioState( same_map_radio			, CVAR_NAME_SAME_MAP(MPM_CTF) );
    SetRadioState( force_respawn_radio		, CVAR_NAME_FORCE_RESPAWN(MPM_CTF) );
    SetRadioState( falling_damage_radio		, CVAR_NAME_FALLING_DAMAGE(MPM_CTF) );
    SetRadioState( allow_powerups_radio		, CVAR_NAME_ALLOW_POWERUPS(MPM_CTF) );
    SetRadioState( allow_health_radio		, CVAR_NAME_ALLOW_HEALTH(MPM_CTF) );
    SetRadioState( allow_armor_radio		, CVAR_NAME_ALLOW_ARMOR(MPM_CTF) );
    SetRadioState( infinite_ammo_radio		, CVAR_NAME_INFINITE_AMMO(MPM_CTF) );
    SetRadioState( fast_switch_radio		, CVAR_NAME_FAST_SWITCH(MPM_CTF) );
    SetRadioState( footsteps_radio			, CVAR_NAME_FOOTSTEPS(MPM_CTF) );
    SetRadioState( shotshells_radio			, CVAR_NAME_SHOTSHELLS(MPM_CTF) );
    SetRadioState( skill_system_radio		, CVAR_NAME_SKILL_SYSTEM(MPM_CTF) );
/*
	unsigned long flag;
	int i;
	for (i = 0, flag = 1; i < 6; i++,flag <<= 1)
	{
	    SetRadioStateFlag( weapon_allow[i], CVAR_NAME_WEAPONALLOW(MPM_CTF), flag, true );
	}
*/
	instagib_radio.DrawForeground();
    //draw the option radio buttons.
	flag_control_array.DrawForeground();

	FLAG_RADIO_FUNC(weapon_stay_radio		,DrawForeground());
	FLAG_RADIO_FUNC(item_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(same_map_radio			,DrawForeground());
	FLAG_RADIO_FUNC(force_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(falling_damage_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_powerups_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_health_radio		,DrawForeground());
	FLAG_RADIO_FUNC(allow_armor_radio		,DrawForeground());
	FLAG_RADIO_FUNC(infinite_ammo_radio		,DrawForeground());
	FLAG_RADIO_FUNC(fast_switch_radio		,DrawForeground());
	FLAG_RADIO_FUNC(footsteps_radio			,DrawForeground());
	FLAG_RADIO_FUNC(shotshells_radio		,DrawForeground());
	FLAG_RADIO_FUNC(skill_system_radio		,DrawForeground());
/*
	for (i = 0; i < 6; i++)
	{
		FLAG_RADIO_FUNC(weapon_allow[i]		,DrawForeground());
	}
*/
}

void CMenuSubMultiplayer::DrawGameModeDeathtag()
{
	BtnConfigureWeapons.DrawForeground();
	level_picker.DrawForeground();
    time_limit_picker.DrawForeground();
    score_limit_picker.DrawForeground();
	team_choice_picker.DrawForeground();
//	team1_color_picker.DrawForeground();
//	team2_color_picker.DrawForeground();

    SetRadioState( instagib_radio		, CVAR_NAME_INSTAGIB(MPM_DEATHTAG) );
    //set the value of the radio button controls.
    SetRadioState( weapon_stay_radio		, CVAR_NAME_WEAPON_STAY(MPM_DEATHTAG) );
    SetRadioState( item_respawn_radio		, CVAR_NAME_ITEM_RESPAWN(MPM_DEATHTAG) );
    SetRadioState( spawn_farthest_radio		, CVAR_NAME_SPAWN_FARTHEST(MPM_DEATHTAG) );
    SetRadioState( allow_exiting_radio		, CVAR_NAME_ALLOW_EXITING(MPM_DEATHTAG) );
    SetRadioState( same_map_radio			, CVAR_NAME_SAME_MAP(MPM_DEATHTAG) );
    SetRadioState( force_respawn_radio		, CVAR_NAME_FORCE_RESPAWN(MPM_DEATHTAG) );
    SetRadioState( falling_damage_radio		, CVAR_NAME_FALLING_DAMAGE(MPM_DEATHTAG) );
    SetRadioState( allow_powerups_radio		, CVAR_NAME_ALLOW_POWERUPS(MPM_DEATHTAG) );
    SetRadioState( allow_health_radio		, CVAR_NAME_ALLOW_HEALTH(MPM_DEATHTAG) );
    SetRadioState( allow_armor_radio		, CVAR_NAME_ALLOW_ARMOR(MPM_DEATHTAG) );
    SetRadioState( infinite_ammo_radio		, CVAR_NAME_INFINITE_AMMO(MPM_DEATHTAG) );
    SetRadioState( fast_switch_radio		, CVAR_NAME_FAST_SWITCH(MPM_DEATHTAG) );
    SetRadioState( footsteps_radio			, CVAR_NAME_FOOTSTEPS(MPM_DEATHTAG) );
    SetRadioState( shotshells_radio			, CVAR_NAME_SHOTSHELLS(MPM_DEATHTAG) );
    SetRadioState( skill_system_radio		, CVAR_NAME_SKILL_SYSTEM(MPM_DEATHTAG) );
/*
	unsigned long flag;
	int i;
	for (i = 0, flag = 1; i < 6; i++,flag <<= 1)
	{
	    SetRadioStateFlag( weapon_allow[i], CVAR_NAME_WEAPONALLOW(MPM_DEATHTAG), flag, true );
	}
*/
	instagib_radio.DrawForeground();
    //draw the option radio buttons.
	flag_control_array.DrawForeground();

	FLAG_RADIO_FUNC(weapon_stay_radio		,DrawForeground());
	FLAG_RADIO_FUNC(item_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(same_map_radio			,DrawForeground());
	FLAG_RADIO_FUNC(force_respawn_radio		,DrawForeground());
	FLAG_RADIO_FUNC(falling_damage_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_powerups_radio	,DrawForeground());
	FLAG_RADIO_FUNC(allow_health_radio		,DrawForeground());
	FLAG_RADIO_FUNC(allow_armor_radio		,DrawForeground());
	FLAG_RADIO_FUNC(infinite_ammo_radio		,DrawForeground());
	FLAG_RADIO_FUNC(fast_switch_radio		,DrawForeground());
	FLAG_RADIO_FUNC(footsteps_radio			,DrawForeground());
	FLAG_RADIO_FUNC(shotshells_radio		,DrawForeground());
	FLAG_RADIO_FUNC(skill_system_radio		,DrawForeground());
/*
	for (i = 0; i < 6; i++)
	{
		FLAG_RADIO_FUNC(weapon_allow[i]		,DrawForeground());
	}
*/
}

void CMenuSubMultiplayer::DrawGameModeCooperative()
{
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
      DrawSkillPic(skill_pics[i],skill_pic_x(i), SKILL_PIC_Y);  // draw picture of Hiro representing skill levels
      start[i].DrawForeground();
	}

//    coop_time_limit_picker.DrawForeground();
//    coop_character_color_picker.DrawForeground();
#ifndef DAIKATANA_DEMO
    coop_episode_picker.DrawForeground();
#endif
}

void CMenuSubMultiplayer::DrawForeground()
{
	if (weapons.Active())
	{
		weapons.DrawForeground();
		return;
	}

	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawForeground();
		return;
	}

	if( current_mode == MM_SETUP )
	{
// SCG[6/15/00]: this was commecnted in the japanese version
// changed by yokoyama for Japanese version
#ifndef JPN 
		picDrawStruct.pImage = re.RegisterPic( "pics/interface/mplayer.tga", NULL, NULL, RESOURCE_INTERFACE );
		re.DrawPic( picDrawStruct );
#endif
// for cut the 37play
		mplayer_start_button.DrawForeground();
		mplayer_community_button.DrawForeground();
		button_find_internet.DrawForeground();
		button_find_local.DrawForeground();
		button_start_multiplayer.DrawForeground();
		player_name_field.DrawForeground();
		DKM_DrawString( MULTI_SETUP_NAME_LABEL_LEFT, MULTI_SETUP_NAME_LABEL_TOP, tongue_menu[T_MENU_PLAYER_NAME], NULL, false, false );

		server_name_field.DrawForeground();
		DKM_DrawString( MULTI_START_SERVERNAME_LEFT, MULTI_START_SERVERNAME_LABEL_TOP, tongue_menu[T_MENU_MP_SERVER_NAME], NULL, false, false );

		modelname_picker.DrawForeground();
		skincolor_picker.DrawForeground();
		connect_speed_picker.DrawForeground(); 
	}
	else if( current_mode == MM_HOST )
	{
		// SCG[12/2/99]: Common to all menus
		game_mode_picker.DrawForeground();
#ifndef NO_EPISODE_SELECTOR
	    episode_picker.DrawForeground();
#endif
		hostback_button.DrawForeground();	// WAW[12/3/99]: Added

		int gamemode = GameModeIndex();//game_mode_picker.CurrentIndex();
//		if (VALID_GAME_MODE(gamemode))
		SetRadioState(friendly_fire_radio, CVAR_NAME_FRIENDLY_FIRE(gamemode));
		friendly_fire_radio.DrawForeground();

		switch( gamemode )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			go_button.DrawForeground();
			DrawGameModeDeathmatch();
			break;
		case 1:		// WAW[11/19/99]: CTF
			go_button.DrawForeground();
			DrawGameModeCTF();
			break;
		case 2:		// SCG[10/29/99]: Deathtag
			go_button.DrawForeground();
			DrawGameModeDeathtag();
			break;			
		case 3:		// SCG[10/29/99]: Cooperative
			DrawGameModeCooperative();
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
		DrawModeJoin();
	}
}

inline cstrncpy( char *tostr, const char *fromstr, int len )
{
	len--;
	strncpy( tostr, fromstr, len );
	tostr[len] = '\0';
}


int32 DKM_StringWidth(const char *string, int32 max_chars_counted);

//#define SCREENSHOT
#ifdef SCREENSHOT
typedef struct
{
	char	*hostname;
	char	*mapname;
	char	*game;
	char	*players;
} bogusdata_t;

bogusdata_t  screenshot_data[4] = 
{
	"SmackUDown",			"e3m4b",	"DM",		"5/8",
	"SlamDownMyHammer",		"e2dm1",	"DM",		"3/8",
	"My Server",			"e1dm1",	"DeathTag",	"9/16"
};

static void emptyLine(int i,bool ping);

static void screenshotLine(int i,bool ping = true)
{
	if( i > 2 )
	{
		emptyLine( i, ping );
	}

	// server name
	DKM_DrawString(INTERNETJOIN_HOST, multi_join_local_list_line_y(i), 
					screenshot_data[i].hostname,INTERNETJOIN_HOST_WIDTH, false);

	// MapName
	DKM_DrawString(INTERNETJOIN_MAP, multi_join_local_list_line_y(i), 
					screenshot_data[i].mapname,INTERNETJOIN_MAP_WIDTH, false);

	// Game
	DKM_DrawString(INTERNETJOIN_GAME, multi_join_local_list_line_y(i), 
					screenshot_data[i].game,INTERNETJOIN_GAME_WIDTH, false);

	// Players
	DKM_DrawString(INTERNETJOIN_PLAYERS + INTERNETJOIN_PLAYERS_WIDTH - 16, multi_join_local_list_line_y(i), 
					screenshot_data[i].players,MULTI_JOIN_LOCAL_LIST_WIDTH, false);

	if (ping)
		DKM_DrawString(INTERNETJOIN_PING + INTERNETJOIN_PING_WIDTH - 16, multi_join_local_list_line_y(i), 
					"--",MULTI_JOIN_LOCAL_LIST_WIDTH, false);
}
#endif

static void emptyLine(int i,bool ping = true)
{
	// server name
	DKM_DrawString(INTERNETJOIN_HOST, multi_join_local_list_line_y(i), 
					"--",INTERNETJOIN_HOST_WIDTH, false);

	// MapName
	DKM_DrawString(INTERNETJOIN_MAP, multi_join_local_list_line_y(i), 
					"--",INTERNETJOIN_MAP_WIDTH, false);

	// Game
	DKM_DrawString(INTERNETJOIN_GAME, multi_join_local_list_line_y(i), 
					"--",INTERNETJOIN_GAME_WIDTH, false);

	// Players
	DKM_DrawString(INTERNETJOIN_PLAYERS + INTERNETJOIN_PLAYERS_WIDTH - 16, multi_join_local_list_line_y(i), 
					"--",MULTI_JOIN_LOCAL_LIST_WIDTH, false);

	if (ping)
		DKM_DrawString(INTERNETJOIN_PING + INTERNETJOIN_PING_WIDTH - 16, multi_join_local_list_line_y(i), 
					"--",MULTI_JOIN_LOCAL_LIST_WIDTH, false);
}

static void serverLine(int i, bool hilighted, const char *hostname, const char *mapname, 
						const char *game, const char *players,int ping )
{
	// Servername
	DKM_DrawString(INTERNETJOIN_HOST, multi_join_local_list_line_y(i), 
					hostname,INTERNETJOIN_HOST_WIDTH, hilighted);

	// MapName
	DKM_DrawString(INTERNETJOIN_MAP, multi_join_local_list_line_y(i), 
					mapname,INTERNETJOIN_MAP_WIDTH, hilighted);

	// Game
	DKM_DrawString(INTERNETJOIN_GAME, multi_join_local_list_line_y(i), 
					game,INTERNETJOIN_GAME_WIDTH, hilighted);

	// Players
	DKM_DrawString(INTERNETJOIN_PLAYERS + INTERNETJOIN_PLAYERS_WIDTH - DKM_StringWidth(players,8), multi_join_local_list_line_y(i), 
					players,MULTI_JOIN_LOCAL_LIST_WIDTH, hilighted);

	// Ping
	if (ping >= 0)
	{
		char buf[32];
// SCG[1/16/00]: 		sprintf(buf,"%d",ping);
		Com_sprintf(buf,sizeof(buf), "%d",ping);
		DKM_DrawString(INTERNETJOIN_PING + INTERNETJOIN_PING_WIDTH - DKM_StringWidth(buf,8), multi_join_local_list_line_y(i), 
					   buf,MULTI_JOIN_LOCAL_LIST_WIDTH, hilighted);
	}
}



void CMenuSubMultiplayer::DrawModeJoin()
{
	int32 i;
	int32 maxnum;
	// SERVER_IP_ADDRESS
	DKM_DrawString(MULTI_JOIN_NAME_LEFT, MULTI_JOIN_TOP, tongue_menu[T_MENU_MP_SERVER_IP_ADDRESS], NULL, false, false);
	join_name_field.DrawForeground();

	play_button.DrawForeground();
//	refresh_button.DrawForeground();
	joinback_button.DrawForeground(); // WAW[12/3/99]: Added

	if (ctf_selected)
		join_team_choice_picker.DrawForeground();

	// OK Here is the fun part.
	if (gameconnect == GAMECONNECT_INTERNET)
	{
		if (serverlist_state <= SERVERLISTSTATE_IDLE)
			refresh_button.SetText(tongue_menu[T_MENU_MP_REFRESH], true);
		else
			refresh_button.SetText(tongue_menu[T_MENU_CANCEL], true);

		refresh_button.DrawForeground();

		picGamespy.pImage = re.RegisterPic( "pics/interface/gamespy_logo.tga", NULL, NULL, RESOURCE_INTERFACE );
		re.DrawPic( picGamespy );
		switch( serverlist_state )
		{
		case SERVERLISTSTATE_NONE:
		case SERVERLISTSTATE_IDLE:
		case SERVERLISTSTATE_STARTED:
			break;
		case SERVERLISTSTATE_LISTXFER:
			DKM_DrawString(MULTI_JOIN_LOCAL_LIST_LEFT, MULTI_JOIN_LOCAL_LIST_BOTTOM+4,
						   "Downloading server list", NULL, false, false);
			break;
		case SERVERLISTSTATE_LANLIST:
			DKM_DrawString(MULTI_JOIN_LOCAL_LIST_LEFT, MULTI_JOIN_LOCAL_LIST_BOTTOM+4,
						   "Downloading lan list", NULL, false, false);
			break;
		case SERVERLISTSTATE_QUERYING:
			{
				char buf[256];
// SCG[1/16/00]: 				sprintf(buf, "Querying Servers: %d%%", serverlist_progress );
				Com_sprintf(buf,sizeof(buf), "Querying Servers: %d%%", serverlist_progress );
		
				DKM_DrawString(MULTI_JOIN_LOCAL_LIST_LEFT, MULTI_JOIN_LOCAL_LIST_BOTTOM+4,
								buf, NULL, false, false);
			}
			break;
		}
		// Draw the actual game list.

		maxnum = ServerListCount( serverlist );
		int index;
		char buf[64];
		char players[64];

		bound_max(net_list_top_index, maxnum - MULTI_JOIN_LOCAL_LIST_NUM_LINES);
		bound_min(net_list_top_index, 0);

		
		//draw as many local games as we can fit in the list.
		for (i = 0; i < MULTI_JOIN_LOCAL_LIST_NUM_LINES; i++)
		{
			index = i + net_list_top_index;
			if (i < 0)
				continue;

			bool hilighted = (index == local_list_hilight_index);
			
			if (index >= maxnum) 
			{
#ifdef SCREENSHOT
				screenshotLine(i);
#else
				emptyLine(i);
#endif
			}
			else
			{
				GServer server = ServerListGetServer( serverlist, index );
				if (server)
				{
					// Servername
					cstrncpy( buf,ServerGetStringValue( server, "hostname", "" ), 32 );
					if (buf[0] == '\0')
						_snprintf(buf,32,"%s:%d",ServerGetAddress(server),ServerGetIntValue( server, "hostport", 0 )); //ServerGetQueryPort(server) );
					_snprintf(players,32,"%d/%d",ServerGetIntValue(server,"numplayers",0),ServerGetIntValue(server,"maxplayers",0));
					serverLine( i, hilighted,
								buf, 
								ServerGetStringValue( server, "mapname", "(NONE)" ),
								ServerGetStringValue( server, "gametype", " " ),	
								players,
								ServerGetPing(server));
				}
				else
				{
					emptyLine(i);
				}
			}
		}
	}
	else if (num_local_games < 1)
	{
		refresh_button.SetText(tongue_menu[T_MENU_MP_REFRESH], true);
		refresh_button.DrawForeground();
		DKM_DrawString(MULTI_JOIN_LOCAL_LIST_LEFT, multi_join_local_list_line_y(0), tongue_menu[T_MENU_MP_NO_LOCAL_GAMES], NULL, false, false);
	}
	else
	{
		refresh_button.SetText(tongue_menu[T_MENU_MP_REFRESH], true);
		refresh_button.DrawForeground();
		bound_max(local_list_top_index, num_local_games - MULTI_JOIN_LOCAL_LIST_NUM_LINES);
		bound_min(local_list_top_index, 0);
		maxnum = num_local_games;
		for (i = 0; i < MULTI_JOIN_LOCAL_LIST_NUM_LINES; i++)
		{
			int32 index = i + local_list_top_index;	
			if (index < 0 || index >= 32) 
				continue;
					
			//decide if we want to draw the server info hilighted.
			bool hilighted = (index == local_list_hilight_index);
			if (!hilighted && (strlen(manual_server) > 0))
			{
				netadr_t	adr;
				if ( NET_StringToAdr(manual_server, &adr))
				{
					hilighted = NET_CompareBaseAdr( local_game_server_info[index].address,adr) == TRUE;
				}
			}
			
			if (index >= num_local_games)
				emptyLine(i,false);
			else
			{
				serverLine(i,hilighted,
							local_game_server_info[index].server_name,
							local_game_server_info[index].map_name, 
							local_game_server_info[index].gametype,
							local_game_server_info[index].players,-1 );
			}
		}
	}
}


void CMenuSubMultiplayer::SetJoinListButtons()
{
	int curline = GetListTopIndex();
	int maxlines = MaxJoinLines();

	local_games_up_button.Enable( (curline > 0) );
	local_games_down_button.Enable( (maxlines - curline >= MULTI_JOIN_LOCAL_LIST_NUM_LINES) );
}

int CMenuSubMultiplayer::MaxJoinLines()
{
	if (gameconnect == GAMECONNECT_INTERNET)
		return ServerListCount( serverlist );
	return num_local_games;
}

void CMenuSubMultiplayer::PlaceEntitiesDeathmatch()
{
	BtnConfigureWeapons.PlaceEntities();
	level_picker.PlaceEntities();
	DKM_DrawString( MULTI_START_EPISODE_LEFT + 70, MULTI_START_LEVEL_TOP - 17, tongue_menu[T_MENU_GAME_MAP], NULL, false, false );
	time_limit_picker.PlaceEntities();
	max_players_picker.PlaceEntities();
	frag_limit_picker.PlaceEntities();
	level_limit_picker.PlaceEntities();
    flag_control_array.PlaceEntities();
}

void CMenuSubMultiplayer::PlaceEntitiesCTF()
{
	BtnConfigureWeapons.PlaceEntities();
	level_picker.PlaceEntities();
	DKM_DrawString( MULTI_START_EPISODE_LEFT + 70, MULTI_START_LEVEL_TOP - 17, tongue_menu[T_MENU_GAME_MAP], NULL, false, false );
	time_limit_picker.PlaceEntities();
	max_players_picker.PlaceEntities();
	ctf_limit_picker.PlaceEntities();
	team_choice_picker.PlaceEntities();
//	team1_color_picker.PlaceEntities();
//	team2_color_picker.PlaceEntities();
    flag_control_array.PlaceEntities();
}

void CMenuSubMultiplayer::PlaceEntitiesDeathtag()
{
	BtnConfigureWeapons.PlaceEntities();
	level_picker.PlaceEntities();
	DKM_DrawString( MULTI_START_EPISODE_LEFT + 70, MULTI_START_LEVEL_TOP - 17, tongue_menu[T_MENU_GAME_MAP], NULL, false, false );
	time_limit_picker.PlaceEntities();
	score_limit_picker.PlaceEntities();
	team_choice_picker.PlaceEntities();
//	team1_color_picker.PlaceEntities();
//	team2_color_picker.PlaceEntities();
    flag_control_array.PlaceEntities();
}

void CMenuSubMultiplayer::PlaceEntitiesCooperative()
{
//    coop_time_limit_picker.PlaceEntities();
//    coop_character_color_picker.PlaceEntities();
#ifndef DAIKATANA_DEMO
    coop_episode_picker.PlaceEntities();
#endif
}

void CMenuSubMultiplayer::PlaceEntities()
{
	if (weapons.Active())
	{
		weapons.PlaceEntities();
		return;
	}

	if (confirm_dlg.Showing())
	{
		confirm_dlg.PlaceEntities();
		return;
	}

	if( current_mode == MM_SETUP )
	{
		connect_speed_picker.PlaceEntities();
		modelname_picker.PlaceEntities();
		skincolor_picker.PlaceEntities();
		load_character_model(); // update character model animation here
	}
	else if( current_mode == MM_HOST )
	{
		// SCG[12/2/99]: Common to all menus
		game_mode_picker.PlaceEntities();
		separator.PlaceEntities();
#ifndef NO_EPISODE_SELECTOR
		DKM_DrawString( MULTI_START_EPISODE_LEFT + 47, MULTI_START_TOP - 17, tongue_menu[T_MENU_GAME_EPISODE], NULL, false, false );
		episode_picker.PlaceEntities();
#endif

		switch( GameModeIndex() )//game_mode_picker.CurrentIndex() )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			PlaceEntitiesDeathmatch();
			break;
		case 1:		// WAW[11/19/99]: CTF
			PlaceEntitiesCTF();
			break;
		case 2:		// SCG[10/29/99]: Deathtag
			PlaceEntitiesDeathtag();
			break;
		case 3:		// SCG[10/29/99]: Cooperative
			PlaceEntitiesCooperative();
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
		local_games_border.PlaceEntities();
		if (ctf_selected)
			join_team_choice_picker.PlaceEntities();
	}
}


bool AbortEditingKey(int32 key) {
    return key == K_ESCAPE;
}

bool DoneEditingKey(int32 key) {
    return key == K_ENTER || key == K_TAB || key == K_KP_ENTER;
}


bool CMenuSubMultiplayer::Keydown(int32 key)
{
	if (weapons.Active())
	{
		return weapons.Keydown(key);
	}

	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.Keydown(key)) 
		{
			ConfirmFinish();
			return true;
		}
		return false;
	}
	
	if( current_mode == MM_SETUP )
	{
		if( player_name_field.Keydown( key ) == true )
		{
			SetPlayerName(player_name_field.GetText());
			if (AbortEditingKey(key) || DoneEditingKey(key))
			{
				player_name_field.StopEditing();

//				if (DoneEditingKey(key))
//					SetPlayerName(player_name_field.GetText());
				player_name_field.SetText(player_name);
			}
			return true;
		}

		if( server_name_field.Keydown( key ) == true )
		{
			SetServerName(server_name_field.GetText());
			if (AbortEditingKey(key) || DoneEditingKey(key))
			{
				server_name_field.StopEditing();

//				if (DoneEditingKey(key))
//					SetServerName(server_name_field.GetText());
				server_name_field.SetText(server_name);
			}
			return true;
		}
	}
	else if( current_mode == MM_HOST )
	{
		switch( GameModeIndex() )//game_mode_picker.CurrentIndex() )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			break;
		case 1:		// WAW[11/19/99]: CTF
			break;
		case 2:		// SCG[10/29/99]: Deathtag
			break;
		case 3:		// SCG[10/29/99]: Cooperative
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
		if( join_name_field.Keydown( key ) == true )
		{
			if (AbortEditingKey(key) || DoneEditingKey(key))
			{
				join_name_field.StopEditing();

				if (DoneEditingKey(key))
				{
					SetJoinString(join_name_field.GetText());
					
					// save it in manual_server so it gets pinged!
					gameconnect = GAMECONNECT_LAN;
					Com_sprintf(manual_server,sizeof(manual_server),"%s",(const char *)join_name_string);
					DoRefresh();
				}
				join_name_field.SetText(join_name_string);
			}
			return true;
		}
	}
	
	if (gameconnect == GAMECONNECT_NONE)
	{
		switch (key)
		{
		case K_ESCAPE:
			
			if (gameconnect != GAMECONNECT_NONE) // return back to lan vs. internet selection
			{
				gameconnect = GAMECONNECT_NONE;  
			}
			else
			{
				if( Cvar_VariableValue( "console" ) == 0 )
				{
					return false;
				}
				DKM_Shutdown(); // exit entire interface
			}
			return (true);
		}			
	}

	return false;
}

bool CMenuSubMultiplayer::MousePosDeathmatch( int32 norm_x, int32 norm_y )
{
	bool ans = false;	// WAW[12/12/99]: Old way was keeping stuff highlighted.

    if( BtnConfigureWeapons.MousePos(norm_x, norm_y) == true ) ans = true;
    if( level_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( time_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( max_players_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( frag_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( level_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;

    if( teamplay_radio.MousePos(norm_x, norm_y) == true ) ans = true;

//    if( team_name_field.MousePos( norm_x, norm_y ) == true ) ans = true;
	if (instagib_radio.MousePos(norm_x, norm_y)) ans = true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(spawn_farthest_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_exiting_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MousePos( norm_x, norm_y))) ans = true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MousePos( norm_x, norm_y))) ans = true;
	}
*/
	return ans;
}

bool CMenuSubMultiplayer::MousePosCTF( int32 norm_x, int32 norm_y )
{
	bool ans = false;	// WAW[12/12/99]: Old way was keeping stuff highlighted.

    if( BtnConfigureWeapons.MousePos(norm_x, norm_y) == true ) ans = true;
    if( level_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( time_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( max_players_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( ctf_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( team_choice_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
//    if( team1_color_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
//    if( team2_color_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
	if (instagib_radio.MousePos(norm_x, norm_y)) ans = true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MousePos( norm_x, norm_y))) ans = true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MousePos( norm_x, norm_y))) ans = true;
	}
*/
	return ans;
}

bool CMenuSubMultiplayer::MousePosDeathtag(int32 norm_x, int32 norm_y)
{
	bool ans = false;	// WAW[12/12/99]: Old way was keeping stuff highlighted.

    if( BtnConfigureWeapons.MousePos(norm_x, norm_y) == true ) ans = true;
    if( level_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( time_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( score_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
    if( team_choice_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
//    if( team1_color_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
//    if( team2_color_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
	if (instagib_radio.MousePos(norm_x, norm_y)) ans = true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MousePos( norm_x, norm_y))) ans = true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MousePos( norm_x, norm_y))) ans = true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MousePos( norm_x, norm_y))) ans = true;
	}
*/
	return ans;
}

bool CMenuSubMultiplayer::MousePosCooperative(int32 norm_x, int32 norm_y)
{
	bool ans = false;	// WAW[12/12/99]: Old way was keeping stuff highlighted.

    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++) 
    {
        if (start[i].MousePos(norm_x, norm_y) == true) ans = true;
    }   

//	if( coop_time_limit_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
//	if( coop_character_color_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
#ifndef DAIKATANA_DEMO
	if( coop_episode_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
#endif
	return ans;
}

bool CMenuSubMultiplayer::MousePos(int32 norm_x, int32 norm_y)
{
	if (weapons.Active())
	{
		return weapons.MousePos(norm_x, norm_y);
	}

	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MousePos(norm_x,norm_y)) return true;
		return false;
	}

	bool ans = false;	// WAW[12/12/99]: Old way was keeping stuff highlighted.

	if( current_mode == MM_SETUP )
	{
		if( mplayer_start_button.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( mplayer_community_button.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( button_find_internet.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( button_find_local.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( button_start_multiplayer.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( player_name_field.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( server_name_field.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( modelname_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( skincolor_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
		if( connect_speed_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
	}
	else if( current_mode == MM_HOST )
	{
		// SCG[12/2/99]: Common to all menus
		if( game_mode_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
#ifndef NO_EPISODE_SELECTOR
	    if( episode_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
#endif
	    if( friendly_fire_radio.MousePos(norm_x, norm_y) == true ) ans = true;
//	    if( flag_control_array.MousePos( norm_x, norm_y ) == true ) ans = true;
//	    if( go_button.MousePos( norm_x, norm_y ) == true ) ans = true;
	    if( hostback_button.MousePos( norm_x, norm_y ) == true ) ans = true; // WAW[12/3/99]: Added

		switch( GameModeIndex() )//game_mode_picker.CurrentIndex() )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			if( flag_control_array.MousePos( norm_x, norm_y ) == true ) ans = true;
			if( go_button.MousePos( norm_x, norm_y ) == true ) ans = true;
			return MousePosDeathmatch( norm_x, norm_y );
			break;
		case 1:		// WAW[11/19/99]: CTF
			if( flag_control_array.MousePos( norm_x, norm_y ) == true ) ans = true;
		    if( go_button.MousePos( norm_x, norm_y ) == true ) ans = true;
			return MousePosCTF( norm_x, norm_y );
			break;
		case 2:		// SCG[10/29/99]: Deathtag
		    if( flag_control_array.MousePos( norm_x, norm_y ) == true ) ans = true;
		    if( go_button.MousePos( norm_x, norm_y ) == true ) ans = true;
			return MousePosDeathtag( norm_x, norm_y );
			break;
			
		case 3:		// SCG[10/29/99]: Cooperative
			return MousePosCooperative( norm_x, norm_y );
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
	    if( ctf_selected && join_team_choice_picker.MousePos( norm_x, norm_y ) == true ) ans = true;
		if (join_name_field.MousePos(norm_x, norm_y) == true) ans = true;
		if (play_button.MousePos(norm_x, norm_y) == true) ans = true;
		//if (add_button.MousePos(norm_x, norm_y) == true) ans = true;
		if (refresh_button.MousePos(norm_x, norm_y) == true) ans = true;
	    if( joinback_button.MousePos( norm_x, norm_y ) == true ) ans = true; // WAW[12/3/99]: Added
		
		//get the row in the local game list that the mouse touched.
		int32 local_list_hit_row = list_box_hit_test(MULTI_JOIN_LOCAL_LIST_LEFT, MULTI_JOIN_LOCAL_LIST_TOP, 
            MULTI_JOIN_LOCAL_LIST_RIGHT, MULTI_JOIN_LOCAL_LIST_BOTTOM, norm_x, norm_y, MULTI_JOIN_LOCAL_LIST_NUM_LINES);
		
		//check if we were on the list.
		if (local_list_hit_row != -1)
		{
			//mark this row to be hilighted.
			SetListHilightIndex( local_list_hit_row + GetListTopIndex() );
		}
		else
		{
			local_list_hilight_index = -1;
		}
	}
    return ans;
}

bool CMenuSubMultiplayer::MouseDownDeathmatch(int32 norm_x, int32 norm_y)
{
    if( BtnConfigureWeapons.MouseDown( norm_x, norm_y ) == true ) return true;
    if( level_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( max_players_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( frag_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( level_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;

	if( teamplay_radio.MouseDown(norm_x, norm_y) == true ) return true;

//	if( team_name_field.MouseDown( norm_x, norm_y ) == true ) return true;
	if (instagib_radio.MouseDown(norm_x, norm_y)) return true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(spawn_farthest_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_exiting_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseDown( norm_x, norm_y))) return true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseDown( norm_x, norm_y))) return true;
	}
*/
	return false;
}

bool CMenuSubMultiplayer::MouseDownCTF(int32 norm_x, int32 norm_y)
{
    if( BtnConfigureWeapons.MouseDown( norm_x, norm_y ) == true ) return true;
    if( level_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( max_players_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( ctf_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( team_choice_picker.MouseDown( norm_x, norm_y ) == true ) return true;
//	if( team1_color_picker.MouseDown( norm_x, norm_y ) == true ) return true;
//	if( team2_color_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if (instagib_radio.MouseDown(norm_x, norm_y)) return true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseDown( norm_x, norm_y))) return true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseDown( norm_x, norm_y))) return true;
	}
*/
	return false;
}

bool CMenuSubMultiplayer::MouseDownDeathtag(int32 norm_x, int32 norm_y)
{
    if( BtnConfigureWeapons.MouseDown( norm_x, norm_y ) == true ) return true;
    if( level_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( score_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if( team_choice_picker.MouseDown( norm_x, norm_y ) == true ) return true;
//	if( team1_color_picker.MouseDown( norm_x, norm_y ) == true ) return true;
//	if( team2_color_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	if (instagib_radio.MouseDown(norm_x, norm_y)) return true;

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseDown( norm_x, norm_y))) return true;
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseDown( norm_x, norm_y))) return true;
/*
	for (int i = 0; i < 6; i++)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseDown( norm_x, norm_y))) return true;
	}
*/
	return false;
}

bool CMenuSubMultiplayer::MouseDownCooperative(int32 norm_x, int32 norm_y)
{
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        if (start[i].MouseDown(norm_x, norm_y) == true) 
          return true;
    }
//	if( coop_time_limit_picker.MouseDown( norm_x, norm_y ) == true ) return true;
//	if( coop_character_color_picker	.MouseDown( norm_x, norm_y ) == true ) return true;
#ifndef DAIKATANA_DEMO
	if( coop_episode_picker	.MouseDown( norm_x, norm_y ) == true ) return true;
#endif
	return false;
}

bool CMenuSubMultiplayer::MouseDown(int32 norm_x, int32 norm_y)
{
	if (weapons.Active())
	{
		return weapons.MouseDown(norm_x, norm_y);
	}

	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseDown(norm_x,norm_y)) return true;
		return false;
	}

	if( current_mode == MM_SETUP )
	{
		if( mplayer_start_button.MouseDown( norm_x, norm_y ) == true ) return true;
		if( mplayer_community_button.MouseDown( norm_x, norm_y ) == true ) return true;
		if( button_find_internet.MouseDown( norm_x, norm_y ) == true ) return true;
		if( button_find_local.MouseDown( norm_x, norm_y ) == true ) return true;
		if( button_start_multiplayer.MouseDown( norm_x, norm_y ) == true ) return true;
		if( player_name_field.MouseDown( norm_x, norm_y ) == true ) return true;
		if( server_name_field.MouseDown( norm_x, norm_y ) == true ) return true;
		if( modelname_picker.MouseDown( norm_x, norm_y ) == true ) return true;
		if( skincolor_picker.MouseDown( norm_x, norm_y ) == true ) return true;
		if( connect_speed_picker.MouseDown( norm_x, norm_y ) == true ) return true;
	}
	else if( current_mode == MM_HOST )
	{
		// SCG[12/2/99]: Common to all menus
		if( game_mode_picker.MouseDown( norm_x, norm_y ) == true ) return true;
#ifndef NO_EPISODE_SELECTOR
		if( episode_picker.MouseDown( norm_x, norm_y ) == true ) return true;
#endif
		if( friendly_fire_radio.MouseDown(norm_x, norm_y) == true ) return true;
//		if( flag_control_array.MouseDown( norm_x, norm_y ) == true ) return true;
//		if( go_button.MouseDown( norm_x, norm_y ) == true ) return true;
	    if( hostback_button.MouseDown( norm_x, norm_y ) == true ) return true; // WAW[12/3/99]: Added

		switch( GameModeIndex() )//game_mode_picker.CurrentIndex() )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			if( flag_control_array.MouseDown( norm_x, norm_y ) == true ) return true;
			if( go_button.MouseDown( norm_x, norm_y ) == true ) return true;
			return MouseDownDeathmatch( norm_x, norm_y );

		case 1:		// WAW[11/19/99]: CTF
			if( flag_control_array.MouseDown( norm_x, norm_y ) == true ) return true;
			if( go_button.MouseDown( norm_x, norm_y ) == true ) return true;
			return MouseDownCTF( norm_x, norm_y );
			
		case 2:		// SCG[10/29/99]: Deathtag
			if( flag_control_array.MouseDown( norm_x, norm_y ) == true ) return true;
			if( go_button.MouseDown( norm_x, norm_y ) == true ) return true;
			return MouseDownDeathtag( norm_x, norm_y );
			
		case 3:		// SCG[10/29/99]: Cooperative
			return MouseDownCooperative( norm_x, norm_y );

		default:
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
		if( ctf_selected && join_team_choice_picker.MouseDown( norm_x, norm_y ) == true ) return true;
		if (join_name_field.MouseDown(norm_x, norm_y) == true) return true;
		if (play_button.MouseDown(norm_x, norm_y) == true) return true;
		//if (add_button.MouseDown(norm_x, norm_y) == true) return true;
		if (refresh_button.MouseDown(norm_x, norm_y) == true) return true;
	    if( joinback_button.MouseDown( norm_x, norm_y ) == true ) return true; // WAW[12/3/99]: Added
	
		//get the row in the local game list that the mouse touched.
		int32 local_list_hit_row = list_box_hit_test(MULTI_JOIN_LOCAL_LIST_LEFT, MULTI_JOIN_LOCAL_LIST_TOP, 
            MULTI_JOIN_LOCAL_LIST_RIGHT, MULTI_JOIN_LOCAL_LIST_BOTTOM, norm_x, norm_y, MULTI_JOIN_LOCAL_LIST_NUM_LINES);
		
		//check if we were on the list.
		if (local_list_hit_row != -1) 
		{
			// WAW[11/24/99]: Allow selecting of internet games.
			if (gameconnect == GAMECONNECT_INTERNET)
			{
				int32 local_game_index = net_list_top_index + local_list_hit_row;				
				if (local_game_index < 0 || local_game_index >= ServerListCount( serverlist )) 
				{
					return true;
				}

				GServer server = ServerListGetServer( serverlist, local_game_index );
				bool newServer = true;
				if (server)//breaks shit && stricmp(ServerGetStringValue( server, "hostname", "" ),Cvar_VariableString("hostname")))
				{
					char buf[64];
					_snprintf(buf,64,"%s:%d",ServerGetAddress(server),ServerGetIntValue( server, "hostport", 0 )); // ServerGetQueryPort( server ));
					newServer = stricmp(join_name_string,buf) != 0;
					SetJoinString( buf );
//					ctf_selected = (stricmp( "ctf", ServerGetStringValue( server, "gamemode", " " )) == 0);
					ctf_selected =	(strnicmp( "ctf", ServerGetStringValue( server, "gametype", " " ), 3) == 0) || 
									(strnicmp( "deathtag", ServerGetStringValue( server, "gametype", " " ), 8) == 0);
				    S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);
				}

				// check for a double click here.
				if (!newServer)
				{
					if (cl.time - lastClick <= 200)
						return DoJoin();
				}

				lastClick = cl.time;
			}
			else
			{
				//get the index of the server that was clicked.
				int32 local_game_index = local_list_top_index + local_list_hit_row;
				
				//check if we have a valid server at that index.
				if (local_game_index < 0 || local_game_index >= num_local_games)
					return true;

				// filter out your game...
//				if (!stricmp(local_game_server_info[local_game_index].server_name, Cvar_VariableString("hostname")))
//					return true;

				//take the ip address of the server and dump in into the join name field.
				bool newServer = stricmp(join_name_string,NET_AdrToString(local_game_server_info[local_game_index].address)) != 0;
				SetJoinString(NET_AdrToString(local_game_server_info[local_game_index].address));
//				ctf_selected = (_strnicmp( local_game_server_info[local_game_index].gametype,"CTF",3 ) == 0);
				ctf_selected =	(strnicmp("ctf",local_game_server_info[local_game_index].gametype,3) == 0) ||
								(strnicmp("deathtag", local_game_server_info[local_game_index].gametype,8) == 0);
			    S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);

				// check for a double click here.
				if (!newServer)
				{
					if (cl.time - lastClick <= 200)
						return DoJoin();
				}

				lastClick = cl.time;
			}				
			return true;
		}
		
		if (local_games_up_button.MouseDown(norm_x, norm_y) == true) 
		{
			MoveJoinUp();
			buttonmove_timer = JOINMOVE_START;
			return true;
		}
		if (local_games_down_button.MouseDown(norm_x, norm_y) == true)
		{
			MoveJoinDown();
			buttonmove_timer = JOINMOVE_START;
			return true;
		}
	}

    return false;
}

bool CMenuSubMultiplayer::MouseUpDeathmatch(int32 norm_x, int32 norm_y)
{
	if( BtnConfigureWeapons.MouseUp( norm_x, norm_y ) == true )
	{
		weapons.Activate(CVAR_NAME_WEAPONALLOW(MPM_DEATHMATCH));
		return true;
	}
	if( level_picker.MouseUp( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_DEATHMATCH), time_limit_picker.CurrentString());
		return true;
	}

	if( max_players_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_MAXCLIENTS(MPM_DEATHMATCH), max_players_picker.CurrentString());
		return true;
	}

	if( frag_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_FRAGLIMIT(MPM_DEATHMATCH), frag_limit_picker.CurrentString());
		return true;
	}

	if( level_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_LEVELLIMIT(MPM_DEATHMATCH), level_limit_picker.CurrentString());
		return true;
	}

	if( teamplay_radio.MouseUp(norm_x, norm_y) == true) { ToggleRadioState(CVAR_NAME_TEAMPLAY(MPM_DEATHMATCH)); return true; }

//	if( team_name_field.MouseUp( norm_x, norm_y ) == true ) return true;
	if (instagib_radio.MouseUp(norm_x, norm_y)) { ToggleRadioState( CVAR_NAME_INSTAGIB(MPM_DEATHMATCH) ); return true; }

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_WEAPON_STAY(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ITEM_RESPAWN(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(spawn_farthest_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState(	CVAR_NAME_SPAWN_FARTHEST(MPM_DEATHMATCH)); return true; }
	if (FLAG_RADIO_FUNC(allow_exiting_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState(	CVAR_NAME_ALLOW_EXITING(MPM_DEATHMATCH)); return true; }
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SAME_MAP(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FORCE_RESPAWN(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FALLING_DAMAGE(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_POWERUPS(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_HEALTH(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_ARMOR(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_INFINITE_AMMO(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FAST_SWITCH(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FOOTSTEPS(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SHOTSHELLS(MPM_DEATHMATCH) ); return true; }
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SKILL_SYSTEM(MPM_DEATHMATCH) ); return true; }
/*
	unsigned long flag = 1;
	for (int i = 0; i < 6; i++,flag <<= 1)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseUp( norm_x, norm_y))) 
		{ 
			ToggleCvarFlag( CVAR_NAME_WEAPONALLOW(MPM_DEATHMATCH), flag ); 
			return true; 
		}
	}
*/
	return false;
}

bool CMenuSubMultiplayer::MouseUpCTF(int32 norm_x, int32 norm_y)
{
	if( BtnConfigureWeapons.MouseUp( norm_x, norm_y ) == true )
	{
		weapons.Activate(CVAR_NAME_WEAPONALLOW(MPM_CTF));
		return true;
	}
	if( level_picker.MouseUp( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_CTF), time_limit_picker.CurrentString());
		return true;
	}

	if( max_players_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_MAXCLIENTS(MPM_CTF), max_players_picker.CurrentString());
		return true;
	}

	if( ctf_limit_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		Cvar_ForceSet(CVAR_NAME_CAPTURELIMIT(MPM_CTF), ctf_limit_picker.CurrentString());
		return true;
	}

	if( team_choice_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		SetTeamChoice( team_choice_picker.CurrentIndex() );
		return true;
	}
/*
	int old_val = team1_color_picker.CurrentIndex();
	if( team1_color_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		VerifyTeamColorPickers(team1_color_picker,team2_color_picker,old_val);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_CTF),team1_color_picker.CurrentIndex()+1);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_CTF),team2_color_picker.CurrentIndex()+1);
		return true;
	}

	old_val = team2_color_picker.CurrentIndex();
	if( team2_color_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		VerifyTeamColorPickers(team2_color_picker,team1_color_picker,old_val);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_CTF),team1_color_picker.CurrentIndex()+1);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_CTF),team2_color_picker.CurrentIndex()+1);
		return true;
	}
*/
	if (instagib_radio.MouseUp(norm_x, norm_y)) { ToggleRadioState( CVAR_NAME_INSTAGIB(MPM_CTF) ); return true; }

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_WEAPON_STAY(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ITEM_RESPAWN(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SAME_MAP(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FORCE_RESPAWN(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FALLING_DAMAGE(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_POWERUPS(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_HEALTH(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_ARMOR(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_INFINITE_AMMO(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FAST_SWITCH(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FOOTSTEPS(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SHOTSHELLS(MPM_CTF) ); return true; }
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SKILL_SYSTEM(MPM_CTF) ); return true; }
/*
	unsigned long flag = 1;
	for (int i = 0; i < 6; i++,flag <<= 1)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseUp( norm_x, norm_y))) 
		{ 
			ToggleCvarFlag( CVAR_NAME_WEAPONALLOW(MPM_CTF), flag ); 
			return true; 
		}
	}
*/
	return false;
}

// WAW[12/3/99]: Added for reusablity.
void CMenuSubMultiplayer::RefreshInternetList()
{
	if (serverlist_state > SERVERLISTSTATE_IDLE)
	{
		ServerListHalt( serverlist );
//		while( serverlist_state > SERVERLISTSTATE_IDLE )
//			ServerListThink(serverlist);
	}
	ServerListClear( serverlist );
	serverlist_state		= SERVERLISTSTATE_STARTED;
	serverlist_progress = 0;
	if (ServerListUpdate( serverlist, true ) == GE_NOCONNECT)
	{
		ServerListHalt( serverlist );
		ServerListClear( serverlist );
		serverlist_state		= SERVERLISTSTATE_NONE;
	}
}


bool CMenuSubMultiplayer::MouseUpDeathtag(int32 norm_x, int32 norm_y)
{
	if( BtnConfigureWeapons.MouseUp( norm_x, norm_y ) == true )
	{
		weapons.Activate(CVAR_NAME_WEAPONALLOW(MPM_DEATHTAG));
		return true;
	}
	if( level_picker.MouseUp( norm_x, norm_y ) == true ) return true;
	if( time_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_DEATHTAG), time_limit_picker.CurrentString());
		return true;
	}

	if( score_limit_picker.MouseUp( norm_x, norm_y ) == true )
	{
		Cvar_ForceSet(CVAR_NAME_TAGLIMIT(MPM_DEATHTAG), score_limit_picker.CurrentString());
		return true;
	}

	if( team_choice_picker.MouseUp( norm_x, norm_y ) == true )
	{
		SetTeamChoice( team_choice_picker.CurrentIndex() );
		return true;
	}
/*
	int old_val = team1_color_picker.CurrentIndex();
	if( team1_color_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		VerifyTeamColorPickers(team1_color_picker,team2_color_picker,old_val);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_DEATHTAG),team1_color_picker.CurrentIndex()+1);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_DEATHTAG),team2_color_picker.CurrentIndex()+1);
		return true;
	}

	old_val = team2_color_picker.CurrentIndex();
	if( team2_color_picker.MouseUp( norm_x, norm_y ) == true ) 
	{
		VerifyTeamColorPickers(team2_color_picker,team1_color_picker,old_val);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_DEATHTAG),team1_color_picker.CurrentIndex()+1);
		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_DEATHTAG),team2_color_picker.CurrentIndex()+1);
		return true;
	}
*/
	if (instagib_radio.MouseUp(norm_x, norm_y)) { ToggleRadioState( CVAR_NAME_INSTAGIB(MPM_DEATHTAG) ); return true; }

	if (FLAG_RADIO_FUNC(weapon_stay_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_WEAPON_STAY(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(item_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ITEM_RESPAWN(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(same_map_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SAME_MAP(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(force_respawn_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FORCE_RESPAWN(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(falling_damage_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FALLING_DAMAGE(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(allow_powerups_radio	,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_POWERUPS(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(allow_health_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_HEALTH(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(allow_armor_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_ALLOW_ARMOR(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(infinite_ammo_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_INFINITE_AMMO(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(fast_switch_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FAST_SWITCH(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(footsteps_radio			,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_FOOTSTEPS(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(shotshells_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SHOTSHELLS(MPM_DEATHTAG) ); return true; }
	if (FLAG_RADIO_FUNC(skill_system_radio		,MouseUp( norm_x, norm_y))) { ToggleRadioState( CVAR_NAME_SKILL_SYSTEM(MPM_DEATHTAG) ); return true; }
/*
	unsigned long flag = 1;
	for (int i = 0; i < 6; i++,flag <<= 1)
	{
		if (FLAG_RADIO_FUNC(weapon_allow[i]		,MouseUp( norm_x, norm_y))) 
		{ 
			ToggleCvarFlag( CVAR_NAME_WEAPONALLOW(MPM_DEATHTAG), flag ); 
			return true; 
		}
	}
*/
	return false;
}

bool CMenuSubMultiplayer::MouseUpCooperative(int32 norm_x, int32 norm_y)
{
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        if (start[i].MouseUp(norm_x, norm_y) == true)
        {
			Commit();
			FillCoopInfo(true);
            // set difficulty skill level
            Cvar_SetValue("skill", ( float ) i );
			Cvar_Set( "deathmatch", "0" );
			Cvar_Set( "deathtag", "0" );
			Cvar_Set( "ctf", "0" );
			Cvar_Set( "coop", "1" );

#ifdef DAIKATANA_DEMO
			DKM_ShutdownString("maxclients 3\nwait\nwait\nmap e1m1a\n");
#else
	int episode = coop_episode_picker.CurrentIndex() + 1;
	#ifdef DAIKATANA_OEM
			if(episode == 1)
			{
				DKM_ShutdownString("maxclients 3\nwait\nwait\nmap e%dm2a\n",episode);
			}
			else
			{
				DKM_ShutdownString("maxclients 3\nwait\nwait\nmap e%dm1a\n",episode);
			}
	#else
			DKM_ShutdownString("maxclients 3\nwait\nwait\nmap e%dm1a\n",episode);
	#endif
#endif
            DKM_Shutdown();

            return (true);
        }
    }

#ifndef DAIKATANA_DEMO
	if( coop_episode_picker.MouseUp( norm_x, norm_y ) == true )
	{
		return true;
	}
#endif

	return false;
}


int LaunchMplayer(void);
int LaunchBrowser(char *url);

bool CMenuSubMultiplayer::ConfirmFinish()
{
	if (confirm_dlg.Showing())
	{
		bool yes = false;
		if (confirm_dlg.Result(yes))
		{
			if ((mplayer_exec_mode != -1) && yes)
			{
				switch(mplayer_exec_mode)
				{
				default:
					break;
				case MPLAYER_EXECUTE_GAME:
					LaunchMplayer();
					DKM_ShutdownString("quit");
					DKM_Shutdown();
					break;
				case MPLAYER_EXECUTE_WEB:
					Sys_Minimize();
					LaunchBrowser("www.daikatananews.com");
//					DKM_ShutdownString("quit");
//					DKM_Shutdown();
					break;
				};
			}
			return true;
		}
	}

	return false;
}

bool CMenuSubMultiplayer::DoJoin()
{
	if (strlen(join_name_string)) // join name must exist
	{
		if (ctf_selected)
		{					
			Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_CTF),join_team_choice_picker.CurrentIndex());
			Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_DEATHTAG),join_team_choice_picker.CurrentIndex());
		}
		else
		{
			Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_ASSIGN), team_color_convert[skincolor_picker.CurrentIndex()]);
		}

		//make the command string to run the connect command.
		char command[256];
		Com_sprintf(command,sizeof(command),"connect %s\n",(const char *)join_name_string);
		
		//start the map. loading\nwait\nwait\nwait\nwait\nwait\nwait\n
//		DKM_ShutdownString("%s", (const char *)command);
		Cbuf_AddText(command);
		//shut down the menu.
		DKM_Shutdown();
		return true;
	}
	else
	{
		return false;
	}
}

bool CMenuSubMultiplayer::MouseUp(int32 norm_x, int32 norm_y)
{
	if (weapons.Active())
	{
		return weapons.MouseUp(norm_x, norm_y);
	}

	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseUp(norm_x,norm_y))
		{
			ConfirmFinish();
			return true;
		}
		return false;
	}

	if( current_mode == MM_SETUP )
	{
		if( mplayer_start_button.MouseUp( norm_x, norm_y ) == true )
		{
			char *msg = tongue_menu[T_MENU_DLG_MPLAYER];
			confirm_dlg.Show(msg,tongue_menu[T_MENU_CONTINUE]);			
			mplayer_exec_mode = MPLAYER_EXECUTE_GAME;
			return true;
		}

		if( mplayer_community_button.MouseUp( norm_x, norm_y ) == true )
		{
			char *msg = tongue_menu[T_MENU_DLG_COMM];
			confirm_dlg.Show(msg,tongue_menu[T_MENU_CONTINUE]);			
			mplayer_exec_mode = MPLAYER_EXECUTE_WEB;
			return true;
		}

		if( button_find_internet.MouseUp( norm_x, norm_y ) == true )
		{
			SetCurrentMode( MM_JOIN );
			gameconnect = GAMECONNECT_INTERNET;
			// WAW[12/3/99]: If no entries or first time in send a update command.
			if ((serverlist_state == SERVERLISTSTATE_NONE) || (ServerListCount( serverlist ) == 0))
				RefreshInternetList();
			return true;
		}
		if( button_find_local.MouseUp( norm_x, norm_y ) == true )
		{
			SetCurrentMode( MM_JOIN );
			gameconnect = GAMECONNECT_LAN;
			DoRefresh();
			return true;
		}
		if( button_start_multiplayer.MouseUp( norm_x, norm_y ) == true )
		{
			SetCurrentMode( MM_HOST );
			return true;
		}
		if( player_name_field.MouseUp( norm_x, norm_y ) == true ) return true;
		if( server_name_field.MouseUp( norm_x, norm_y ) == true ) return true;
		if( modelname_picker.MouseUp( norm_x, norm_y ) == true )
		{
			register_character_model(true);
			return true;   
		}
		if( skincolor_picker.MouseUp( norm_x, norm_y ) == true )
		{
			//set the maximum number of clients allowed.
			Cvar_SetValue("skincolor", skincolor_picker.CurrentIndex());
			register_character_model(false);
			return true;   
		}
		if( connect_speed_picker.MouseUp( norm_x, norm_y ) == true )
		{
			if (connect_speed_picker.CurrentIndex() != CS_CUSTOM) // don't apply rate for custom setting
			{
				Cvar_SetValue("rate", rate_settings[connect_speed_picker.CurrentIndex()] );
			}
		}

/*
		if( character_setup_button.MouseUp(norm_x, norm_y) == true )
		{
			SetCurrentMode(MM_SETUP);
			load_character_model();
			character_setup_button.MousePos(-1, -1);
			gameconnect = GAMECONNECT_NONE;
			return true;
		}
*/
	}
	else if( current_mode == MM_HOST )
	{
		if (game_mode_picker.MouseUp(norm_x, norm_y) == true) 
		{
			setGameMode(); // WAW[11/19/99]: Called once when the game mode gets changed.
			return true;
		}
		
#ifndef NO_EPISODE_SELECTOR
		if( episode_picker.MouseUp( norm_x, norm_y ) == true ) return true;
#endif
		int gamemode = GameModeIndex();//game_mode_picker.CurrentIndex();
//		if (VALID_GAME_MODE(gamemode))
		{
			if( friendly_fire_radio.MouseUp( norm_x, norm_y ) == true ) { ToggleRadioState( CVAR_NAME_FRIENDLY_FIRE( gamemode ) ); return true; }
		}

		if (gamemode != MPM_COOP)
		{
			if( flag_control_array.MouseUp( norm_x, norm_y ) == true ) return true;
		}		
//		if( go_button.MouseUp( norm_x, norm_y ) == true ) { StartServer(); return true; }
	    if( hostback_button.MouseUp( norm_x, norm_y ) == true ) // WAW[12/3/99]: Added
		{
			SetCurrentMode( MM_SETUP );
			return true;
		}



		switch( gamemode )
		{
		case 0:		// SCG[10/29/99]: Deathmatch
			if( go_button.MouseUp( norm_x, norm_y ) == true ) { StartServer(); return true; }
			return MouseUpDeathmatch( norm_x, norm_y );
			break;
		case 1:		// WAW[11/29/99]: CTF
			if( go_button.MouseUp( norm_x, norm_y ) == true ) { StartServer(); return true; }
			return MouseUpCTF( norm_x, norm_y );
			break;
		case 2:		// SCG[10/29/99]: Deathtag
			if( go_button.MouseUp( norm_x, norm_y ) == true ) { StartServer(); return true; }
			return MouseUpDeathtag( norm_x, norm_y );
			break;
			
		case 3:		// SCG[10/29/99]: Cooperative
			return MouseUpCooperative( norm_x, norm_y );
			break;
		}
	}
	else if( current_mode == MM_JOIN )
	{
		if( ctf_selected && join_team_choice_picker.MouseUp( norm_x, norm_y ) == true ) 
		{
			SetTeamChoice( join_team_choice_picker.CurrentIndex() );
			return true;
		}
		if (join_name_field.MouseUp(norm_x, norm_y) == true)
		{
			return true;
		}   
		
		if (play_button.MouseUp(norm_x, norm_y) == true)
		{
			return DoJoin();
/*
			if (strlen(join_name_string)) // join name must exist
			{
				if (ctf_selected)
				{					
					Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_CTF),join_team_choice_picker.CurrentIndex());
					Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_DEATHTAG),join_team_choice_picker.CurrentIndex());
				}
				else
				{
					Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_ASSIGN), team_color_convert[skincolor_picker.CurrentIndex()]);
				}

				//make the command string to run the connect command.
//				buffer256 command("connect %s\n", (const char *)join_name_string);
				char command[256];
				Com_sprintf(command,sizeof(command),"connect %s",(const char *)join_name_string);
				
				//start the map.
				DKM_ShutdownString("loading\nwait\nwait\nwait\nwait\nwait\nwait\n%s", (const char *)command);
				//shut down the menu.
				DKM_Shutdown();
				return true;
			}
			else
			{
				// TTD: beep as invalid selection
				return false;
			}
*/
		}
		
	    if( joinback_button.MouseUp( norm_x, norm_y ) == true ) // WAW[12/3/99]: Added
		{
			SetCurrentMode( MM_SETUP );
			return true;
		}
		//        if (add_button.MouseUp(norm_x, norm_y) == true)
		//          return true;
		
		if (refresh_button.MouseUp(norm_x, norm_y) == true)
		{
			if (gameconnect == GAMECONNECT_INTERNET)
			{
				if (serverlist_state > SERVERLISTSTATE_IDLE)
				{
					ServerListHalt( serverlist );
					serverlist_state = SERVERLISTSTATE_IDLE;
					return true;
				}		
			}

			DoRefresh();
			return true;
		}
		
		// WAW[11/23/99]: Scroll buttons for Join
		if (local_games_up_button.MouseUp(norm_x, norm_y) == true) return true;
		if (local_games_down_button.MouseUp(norm_x, norm_y) == true) return true;
	}
    return false;
}


void CMenuSubMultiplayer::SetPlayerName(const char *name)
{
    buffer256 playername(name);
#ifdef JPN
    playername.StripInvalidChars(15);
#else
    playername.StripInvalidChars(16);
#endif // JPN
	
    Cvar_Set("name", playername);
    player_name = Cvar_VariableString("name");
}


void CMenuSubMultiplayer::SetTeamName(const char *name)
{
	return;
    buffer256 teamname(name);
    teamname.StripInvalidChars(16);
	
    Cvar_Set("team", teamname);
    team_name = Cvar_VariableString("team");
}


void CMenuSubMultiplayer::SetServerName(const char *name)
{
    buffer256 hostname(name);
    hostname.StripInvalidChars(32);
	
    Cvar_Set("hostname", hostname);
    server_name = Cvar_VariableString("hostname");
}


void CMenuSubMultiplayer::SetJoinString(const char *str) 
{
    //save the string.
    join_name_string = str;
	
    //set the text in the control.
    join_name_field.SetText(join_name_string);
}


void CMenuSubMultiplayer::Commit()
{
/*	int val;
    //set the maximum number of clients allowed.
    const char *max_players_string = max_players_picker.CurrentString();
    if (max_players_string == NULL || max_players_string[0] == '\0') return;
	val = atoi(max_players_string);
	buffer256 max_players("%d", val);
	//Cvar_Set("maxclients", (const char *)max_players);
	Cvar_Set("dm_max_players", (const char *)max_players);


    //set the frag limit.
    const char *frag_limit_string = frag_limit_picker.CurrentString();
    if (frag_limit_string == NULL || frag_limit_string[0] == '\0') return;
	val = atoi(frag_limit_string);
	cvar_t *fraglimit = Cvar_Get("fraglimit", "0", CVAR_ARCHIVE);
    buffer256 frag_limit("%d", val );
	Cvar_Set("fraglimit", (const char *)frag_limit);

	// WAW[12/8/99]: Level limit.
    const char *level_limit_string = level_limit_picker.CurrentString();
	if (frag_limit_string == NULL || level_limit_string[0] == '\0') return;
	val = atoi(level_limit_string);
	buffer256 level_limit("%d", val);

	int gamemode = game_mode_picker.CurrentIndex();
	if (VALID_GAME_MODE(gamemode) && (gamemode != MPM_COOP))
		Cvar_Set(CVAR_NAME_LEVELLIMIT( gamemode ), (const char *)level_limit);
	
    //set the time limit.
    const char *time_limit_string = time_limit_picker.CurrentString();
    if (time_limit_string == NULL || time_limit_string[0] == '\0') return;
	val = atoi(time_limit_string);
	cvar_t *timelimit = Cvar_Get("timelimit", "0", CVAR_ARCHIVE);
	buffer256 time_limit("%d", val );
	Cvar_Set("timelimit", (const char *)time_limit);

    const char *ctf_limit_string = ctf_limit_picker.CurrentString();
    if (ctf_limit_string == NULL || ctf_limit_string[0] == '\0') return;
	val = atoi(ctf_limit_string);
	buffer256 ctf_limit("%d", val );
	Cvar_Set("ctf_limit", (const char *)ctf_limit);
*/
	SetPlayerName( player_name_field.GetText() );
//	SetTeamName( team_name_field.GetText() );
	SetServerName( server_name_field.GetText() );

	int mode = GameModeIndex();//game_mode_picker.CurrentIndex();
	switch(mode)
	{
	case 0:	// Deathmatch
		FillDeathmatchInfo(true);
		break;
	case 1: // CTF
		FillCTFInfo(true);
		break;
	case 2: // Deathtag
		FillDeathtagInfo(true);
		break;
	case 3: // Coop.
		FillCoopInfo(true);
		break;
	}

	// copy over all of the cvars to dm_ style vars
	char *from_cvar_str;
	char *to_cvar_str;
	int count = sizeof(flag_cvars)/sizeof(flag_cvars[0]);
	for (int j = 0; j < count; j++)
	{
		// verify the source cvar
		from_cvar_str = flag_cvars[j][mode];
		if (from_cvar_str == NULL || from_cvar_str[0] == '\0') continue;

		// verify the dest cvar
		to_cvar_str = flag_cvars[j][MPM_ASSIGN];
		if (to_cvar_str == NULL || to_cvar_str[0] == '\0') continue;

		// force the cvar's value
		Cvar_ForceSet(to_cvar_str,Cvar_VariableString(from_cvar_str));
	}
}

void CMenuSubMultiplayer::StartServer()
{
	int mode = GameModeIndex();//game_mode_picker.CurrentIndex();
    //get the map name that is selected.
    const char *map_name = GetCurrentMapName();
    if (map_name == NULL || map_name[0] == '\0' && (mode != MPM_COOP)) 
		return;
	
	Commit();

	int maxclients;
	switch (mode)
	{
	case MPM_DEATHMATCH:
		maxclients = Cvar_VariableValue("m_dm_maxclients");
		Cvar_ForceSet( "deathmatch", "1" );
		Cvar_ForceSet( "deathtag", "0" );
		Cvar_ForceSet( "ctf", "0" );
		Cvar_ForceSet( "coop", "0" );
		break;
	case MPM_CTF:
		maxclients = Cvar_VariableValue("ctf_maxclients");
		Cvar_ForceSet( "deathmatch", "1" );
		Cvar_ForceSet( "deathtag", "0" );
		Cvar_ForceSet( "ctf", "1" );
		Cvar_ForceSet( "coop", "0" );
		break;
	case MPM_DEATHTAG:
		maxclients = 4;
		Cvar_ForceSet( "deathmatch", "1" );
		Cvar_ForceSet( "deathtag", "1" );
		Cvar_ForceSet( "ctf", "0" );
		Cvar_ForceSet( "coop", "0" );
		break;
	case MPM_COOP:
		maxclients = 3;
		Cvar_ForceSet( "deathmatch", "0" );
		Cvar_ForceSet( "deathtag", "0" );
		Cvar_ForceSet( "ctf", "0" );
		Cvar_ForceSet( "coop", "1" );
		break;
	}

	DKM_ShutdownString("maxclients %d\nwait\nwait\nmap %s\n",maxclients,map_name);
    DKM_Shutdown();
}



const char *CMenuSubMultiplayer::GetCurrentMapName()
{
    const char *map_name = "";
    //get the currently selected episode.
    int32 episode_num = level_picker.CurrentIndex();
	if ((episode_num < 0) || (episode_num >= episode_map_names.Num()))
		return "";
	
	return *episode_map_names.Item(episode_num);
}



void CMenuSubMultiplayer::load_character_model(void)
// load player models to show within the interface
{
//	static bool first_in = true;
	entity_t *ent;
	float ent_scale = 0.37f;
	
	ent = NewEntity();  //get an entity structure we can fill in.
	
	ent->model = character_model;
	ent->skin  = character_skin;
	ent->flags = RF_FULLBRIGHT;   // make the button fullbright.
	
	ent->origin.Set(120, -10, 16); // set origin
	ent->angles.Set(0,0,0); // show your good side, baby
	
	ent->angles.y = anglemod(character_model_rotation);   // rotation
	ent->render_scale.Set(ent_scale,ent_scale,ent_scale); // squishysquish
	ent->frame = character_model_frame;
	
//	first_in = false;
}



void CMenuSubMultiplayer::register_character_model(bool bResetRotation)
// initialize the character model
{
	char *m_name[] = { "m_hiro.dkm", "m_mikiko.dkm", "m_superfly.dkm" };
	char *s_name[] = { "hiro", "miko", "sfly" };
	frameData_t frameData;
	char modelname_string[256];
	int model_idx;
	
	
	// set main character model
	model_idx = modelname_picker.CurrentIndex();
	Cvar_SetValue("character",model_idx); // 0=Hiro, 1=Mikiko, 2=Superfly
	
// SCG[1/16/00]: 	sprintf(modelname_string,"models/global/%s", m_name[model_idx]);
	Com_sprintf(modelname_string,sizeof(modelname_string), "models/global/%s", m_name[model_idx]);
	Cvar_ForceSet("modelname", (const char *)modelname_string);
	character_model = re.RegisterModel(modelname_string, RESOURCE_GLOBAL);
	
	
	buffer32 skinname_string("skins/%s_bod_%d.wal", s_name[(int)character->value], skincolor_picker.CurrentIndex() + 1);
	Cvar_Set("skinname", (const char *)skinname_string);
	character_skin  = re.RegisterSkin(skinname_string, RESOURCE_GLOBAL);
	
	if (bResetRotation)
	{
		// defaults
		character_model_frame_start = 0;
		character_model_frame_end   = 0;
		character_model_frame       = 0;
		
		// grab start/stop frames from frame data
		dk_GetSpecificAnimSequences(-1, modelname_string, &frameData, "oamba");
		
		character_model_frame_start = frameData.first;						// starting frame
		
		dk_GetSpecificAnimSequences(-1, modelname_string, &frameData, "oambb");
		
		character_model_frame_end   = frameData.last;            // ending frame
		
		character_model_frame = character_model_frame_start;
		character_model_timer = 0;
	}
	
}


void CMenuSubMultiplayer::fillEpisodePicker( const char *type )
{
	CCSVFile *pcsvfile = NULL;
	char szline[2048];
	char szmapname[64];
	char sztype[64];
	char szmaptitle[128];
	int cnt = 0;
	
	int curnum = level_picker.CurrentIndex();
	if (curnum < 0)
		curnum = 0;
	
	level_picker.ResetStrings();
	episode_map_names.Reset();
	
	int nretcode = CSV_OpenFile( "MultiplayerMaps.csv", &pcsvfile );
	if ( nretcode == DKIO_ERROR_NONE )
	{
		while (CSV_GetNextLine(pcsvfile,szline) != EOF)
		{
			// get level name
			CSV_GetFirstElement(pcsvfile, szline, szmapname);
			CSV_GetNextElement(pcsvfile, szline, sztype );
			CSV_GetNextElement(pcsvfile, szline, szmaptitle );
			if (!_stricmp(sztype, type ))
			{
				char *tm = szmaptitle;
				while((*tm == ' ') || (*tm == '\t'))
					tm++;
				level_picker.AddString((*tm == '\0' ? szmapname:tm));
				
				// Add it to our array.
				buffer64 *new_string = new buffer64;
				*new_string = szmapname;
				episode_map_names.Add( new_string );
				cnt++;
			}
		}
		CSV_CloseFile( pcsvfile );
	}
	
	if (curnum >= cnt)
	{
		curnum = cnt - 1;
	}
	level_picker.SetCurrentString( curnum );
}

void CMenuSubMultiplayer::setGameMode()
{
	int gm = GameModeIndex();//game_mode_picker.CurrentIndex();
	switch(gm)
	{
	case 0:	// Deathmatch
		fillEpisodePicker( "deathmatch" );
		FillDeathmatchInfo();
		go_button.Enable(true);
		break;
	case 1: // CTF
		fillEpisodePicker( "ctf" );
		FillCTFInfo();
		go_button.Enable(true);
		break;
	case 2: // Deathtag
		fillEpisodePicker( "deathtag" );
		FillDeathtagInfo();
		go_button.Enable(true);
		break;
	case 3: // Coop.
		fillEpisodePicker( "coop" );
		FillCoopInfo();
		go_button.Enable(true);
		break;
	}
}

void CMenuSubMultiplayer::FillDeathmatchInfo(bool bSetVars)
{
	if (bSetVars)
	{
		Cvar_ForceSet(CVAR_NAME_FRAGLIMIT(MPM_DEATHMATCH), frag_limit_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_LEVELLIMIT(MPM_DEATHMATCH), level_limit_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_DEATHMATCH), time_limit_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_MAXCLIENTS(MPM_DEATHMATCH), max_players_picker.CurrentString());
		Cvar_ForceSetValue(CVAR_NAME_TEAMCHOICE(MPM_DEATHMATCH), team_color_convert[skincolor_picker.CurrentIndex()]);
	}
	else
	{
		int i,num;

		// set up the frag limit
		cvar_t *fraglimit = Cvar_Get(CVAR_NAME_FRAGLIMIT(MPM_DEATHMATCH), "0", CVAR_ARCHIVE);
		if (fraglimit->value < 1.0f) num = 0;
		else if (fraglimit->value < 7.5f) num = 1;
		else if (fraglimit->value < 12.5f) num = 2;
		else if (fraglimit->value < 17.5f) num = 3;
		else if (fraglimit->value < 25.0f) num = 4;
		else if (fraglimit->value < 35.0f) num = 5;
		else if (fraglimit->value < 12.5f) num = 6;
		else						  num = 7;
		frag_limit_picker.SetCurrentString( num );

		// set up the player level limit
		cvar_t *levellimit = Cvar_Get(CVAR_NAME_LEVELLIMIT(MPM_DEATHMATCH), "0", CVAR_ARCHIVE);
		level_limit_picker.SetCurrentString((levellimit->value >= 2 ? (int)(levellimit->value-1):0)); 

		// set up the timelimit
		cvar_t *timelimit = Cvar_Get(CVAR_NAME_TIMELIMIT(MPM_DEATHMATCH), "0", CVAR_ARCHIVE);
		if (timelimit->value < 2.5f) num = 0;
		else if (timelimit->value < 7.5f) num = 1;
		else if (timelimit->value < 12.5f) num = 2;
		else if (timelimit->value < 17.5f) num = 3;
		else if (timelimit->value < 25.0f) num = 4;
		else num = 5;
		time_limit_picker.SetCurrentString( num );

		// set up the maxclients
		cvar_t *max_players = Cvar_Get(CVAR_NAME_MAXCLIENTS(MPM_DEATHMATCH), "0", CVAR_ARCHIVE);
		for(i = 0;maxplayer_settings[i+1] != 0;i++)
		{
			if ((int)max_players->value <= maxplayer_settings[i])
				break;
		}
		max_players_picker.SetCurrentString(i);

		// set up the team name
	//    team_name = Cvar_VariableString("team");
	//    team_name.StripInvalidChars( 16 );
	//    team_name_field.SetText( team_name );
	}
}

void CMenuSubMultiplayer::FillCTFInfo(bool bSetVars)
{
	if (bSetVars)
	{
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_CTF), time_limit_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_MAXCLIENTS(MPM_CTF), max_players_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_CAPTURELIMIT(MPM_CTF), ctf_limit_picker.CurrentString());
//		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_CTF),team1_color_picker.CurrentIndex()+1);
//		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_CTF),team2_color_picker.CurrentIndex()+1);
		SetTeamChoice( team_choice_picker.CurrentIndex() );
	}
	else
	{
		int i,num;

		// set up the timelimit
		cvar_t *timelimit = Cvar_Get(CVAR_NAME_TIMELIMIT(MPM_CTF), "0", CVAR_ARCHIVE);
		if (timelimit->value < 2.5f) num = 0;
		else if (timelimit->value < 7.5f) num = 1;
		else if (timelimit->value < 12.5f) num = 2;
		else if (timelimit->value < 17.5f) num = 3;
		else if (timelimit->value < 25.0f) num = 4;
		else num = 5;
		time_limit_picker.SetCurrentString( num );

		// set up the maxclients
		cvar_t *max_players = Cvar_Get(CVAR_NAME_MAXCLIENTS(MPM_CTF), "0", CVAR_ARCHIVE);
		for(i = 0;maxplayer_settings[i+1] != 0;i++)
		{
			if ((int)max_players->value <= maxplayer_settings[i])
				break;
		}
		max_players_picker.SetCurrentString(i);

		// set up the capture limit
		cvar_t *caplimit = Cvar_Get(CVAR_NAME_CAPTURELIMIT(MPM_CTF), "0", CVAR_ARCHIVE);
		if (caplimit->value < 0.5f) num = 0;
		else if (caplimit->value <= 1.5f) num = 1;
		else if (caplimit->value <  3.5f) num = 2;
		else if (caplimit->value <  7.5f) num = 3;
		else if (caplimit->value < 12.5f) num = 4;
		else if (caplimit->value < 17.5f) num = 5;
		else if (caplimit->value < 25.0f) num = 6;
		else if (caplimit->value < 35.0f) num = 7;
		else num = 8;
		ctf_limit_picker.SetCurrentString(num);

		// set up the team choice
		cvar_t *teamchoice = Cvar_Get(CVAR_NAME_TEAMCHOICE(MPM_CTF), "0", CVAR_ARCHIVE);
		num = (int)teamchoice->value;
		if (num > 2)
			num = 2;
		else if (num < 0)
			num = 0;
		team_choice_picker.SetCurrentString(num);
/*
		int colorcount = team1_color_picker.StringCount();
		int color;
		// set up the team color selections
		teamchoice = Cvar_Get(CVAR_NAME_TEAMCOLOR1(MPM_CTF), "0", CVAR_ARCHIVE);
		color = (int)teamchoice->value - 1;
		team1_color_picker.SetCurrentString( ((color > 0) && (color < colorcount-1)) ? color : 0);

		teamchoice = Cvar_Get(CVAR_NAME_TEAMCOLOR2(MPM_CTF), "0", CVAR_ARCHIVE);
		color = (int)teamchoice->value - 1;
		team2_color_picker.SetCurrentString( ((color > 0) && (color < colorcount-1)) ? color : 0);

		VerifyTeamColorPickers(team2_color_picker,team1_color_picker,-1);
*/
	}
}

void CMenuSubMultiplayer::FillDeathtagInfo(bool bSetVars)
{
	if (bSetVars)
	{
		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_DEATHTAG), time_limit_picker.CurrentString());
		Cvar_ForceSet(CVAR_NAME_TAGLIMIT(MPM_DEATHTAG), score_limit_picker.CurrentString());
//		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR1(MPM_DEATHTAG),team1_color_picker.CurrentIndex()+1);
//		Cvar_ForceSetValue(CVAR_NAME_TEAMCOLOR2(MPM_DEATHTAG),team2_color_picker.CurrentIndex()+1);
		SetTeamChoice( team_choice_picker.CurrentIndex() );
	}
	else
	{
		int num;

		// set up the timelimit
		cvar_t *timelimit = Cvar_Get(CVAR_NAME_TIMELIMIT(MPM_DEATHTAG), "0", CVAR_ARCHIVE);
		if (timelimit->value < 2.5f) num = 0;
		else if (timelimit->value < 7.5f) num = 1;
		else if (timelimit->value < 12.5f) num = 2;
		else if (timelimit->value < 17.5f) num = 3;
		else if (timelimit->value < 25.0f) num = 4;
		else num = 5;
		time_limit_picker.SetCurrentString( num );

		// set up the team choice
		cvar_t *teamchoice = Cvar_Get(CVAR_NAME_TEAMCHOICE(MPM_DEATHTAG), "0", CVAR_ARCHIVE);
		num = (int)teamchoice->value;
		if (num > 2)
			num = 2;
		else if (num < 0)
			num = 0;
		team_choice_picker.SetCurrentString(num);
/*
		int colorcount = team1_color_picker.StringCount();
		int color;
		// set up the team color selections
		teamchoice = Cvar_Get(CVAR_NAME_TEAMCOLOR1(MPM_DEATHTAG), "0", CVAR_ARCHIVE);
		color = (int)teamchoice->value - 1;
		team1_color_picker.SetCurrentString( ((color > 0) && (color < colorcount-1)) ? color : 0);

		teamchoice = Cvar_Get(CVAR_NAME_TEAMCOLOR2(MPM_DEATHTAG), "0", CVAR_ARCHIVE);
		color = (int)teamchoice->value - 1;
		team2_color_picker.SetCurrentString( ((color > 0) && (color < colorcount-1)) ? color : 0);

		VerifyTeamColorPickers(team2_color_picker,team1_color_picker,-1);
*/
	}
}

void CMenuSubMultiplayer::FillCoopInfo(bool bSetVars)
{
	if (bSetVars)
	{
//		Cvar_ForceSet(CVAR_NAME_TIMELIMIT(MPM_COOP), coop_time_limit_picker.CurrentString());
	}
	else
	{
/*		int num;

		// set up the timelimit
		cvar_t *timelimit = Cvar_Get(CVAR_NAME_TIMELIMIT(MPM_COOP), "0", CVAR_ARCHIVE);
		if (timelimit->value < 2.5f) num = 0;			// none
		else if (timelimit->value < 7.5f) num = 1;		// 5
		else if (timelimit->value < 12.5f) num = 2;		// 10
		else if (timelimit->value < 17.5f) num = 3;		// 15
		else if (timelimit->value < 25.0f) num = 4;		// 20
		else if (timelimit->value < 45.0f) num = 5;		// 30
		else if (timelimit->value < 75.0f) num = 6;		// 60
		else if (timelimit->value < 105.0f) num = 7;	// 90
		else if (timelimit->value < 150.0f) num = 8;	// 120
		else num = 9;									// 180
		coop_time_limit_picker.SetCurrentString( num );*/
	}
}

void CMenuSubMultiplayer::ServerListCallBack( int msg, void *param1, void *param2 )
{
	//GServer server;
	SetJoinListButtons();
	if (msg == LIST_PROGRESS)
	{
		serverlist_progress = (int)param2;
		//server = (GServer)param1;
		//Com_DPrintf ("%s:%d [%d] %s %d/%d %s\n",ServerGetAddress(server),ServerGetQueryPort(server), ServerGetPing(server),ServerGetStringValue(server, "hostname","(NONE)"), ServerGetIntValue(server,"numplayers",0), ServerGetIntValue(server,"maxplayers",0), ServerGetStringValue(server,"mapname","(NO MAP)"));
	}
	else if (msg == LIST_STATECHANGED)
	{
		switch( ServerListState( serverlist ) )
		{
		case sl_idle:	// Finished
			serverlist_state = SERVERLISTSTATE_IDLE;
			ServerListSort( serverlist, true, "ping", cm_int );
			break;
		case sl_listxfer:
			serverlist_state = SERVERLISTSTATE_LISTXFER;
			break;
		case sl_lanlist:
			serverlist_state = SERVERLISTSTATE_LANLIST;
			break;
		case sl_querying:
			serverlist_state = SERVERLISTSTATE_QUERYING;
			break;
		}
	}
}


void CMenuSubMultiplayer::MoveJoinUp()
{
	int idx = GetListTopIndex();
	idx--;
	if (idx < 0)
		idx = 0;
	SetListTopIndex( idx );
	SetJoinListButtons();
}

void CMenuSubMultiplayer::MoveJoinDown()
{
	int idx = GetListTopIndex();
	idx++;
	if (idx + MULTI_JOIN_LOCAL_LIST_NUM_LINES > MaxJoinLines())
		idx = MaxJoinLines() - MULTI_JOIN_LOCAL_LIST_NUM_LINES;
	if (idx < 0)
		idx = 0;
	SetListTopIndex( idx );
	SetJoinListButtons();
}


void CMenuSubMultiplayer::SetTeamChoicePicker()
{
	// Figure the number based on the current team settings.
	SetTeamChoice( (int)ctf_team->value );
}

void CMenuSubMultiplayer::SetTeamChoice( int val )
{
	if (val > 2)
		val = 2;
	else if (val < 0)
		val = 0;
	char buf[32];
// SCG[1/16/00]: 	sprintf(buf,"%d",val);
	Com_sprintf(buf,sizeof(buf), "%d",val);
//    Cvar_Set("ctf_team", buf);

	int gamemode = GameModeIndex();//game_mode_picker.CurrentIndex();
	if (VALID_GAME_MODE(gamemode))
		Cvar_Set(CVAR_NAME_TEAMCHOICE(gamemode), buf);

	join_team_choice_picker.SetCurrentString(val);
	team_choice_picker.SetCurrentString(val);
}


void CMenuSubMultiplayer::DrawSkillPic(char * filename, int x, int y)
{

  float	norm_to_screen_x = viddef.width  / 640.0f;
  float	norm_to_screen_y = viddef.height / 480.0f;

  DRAWSTRUCT drawStruct;

  drawStruct.pImage  = re.RegisterPic( filename, NULL, NULL, RESOURCE_INTERFACE );
  drawStruct.nFlags  = DSFLAG_PALETTE | DSFLAG_ALPHA | DSFLAG_SCALE | DSFLAG_BLEND;
  drawStruct.nXPos   = x * norm_to_screen_x;
  drawStruct.nYPos   = y * norm_to_screen_y;
  drawStruct.fScaleX = norm_to_screen_x;
  drawStruct.fScaleY = norm_to_screen_y;
  drawStruct.fAlpha  = 0.90f;

  re.DrawPic( drawStruct );
}

void CL_PingServer(char *adrstring);
void CMenuSubMultiplayer::DoRefresh()
{
	ctf_selected = 0;
	SetJoinString("");
	DKM_ResetLocalServerList();

	if (gameconnect == GAMECONNECT_INTERNET)	// WAW[11/24/99]: Added for GameSpy.
	{
		RefreshInternetList();
	}
	else
	{
		CL_PingServers_f();
		CL_PingServer(manual_server);
	}
}
