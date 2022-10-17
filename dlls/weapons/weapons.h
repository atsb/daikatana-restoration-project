//#include <windows.h>
#include <memory.h>
//#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dk_std.h"
#include "p_user.h"
#include "hooks.h"
#include "common.h"
#include "weapon_funcs.h"
#include "libclient.h"

#include "l__language.h"
#include "../world/ai_frames.h"
/* ATSB
#include "..\world\ai_frames.h"

^^

I am going to leave this here.  Whoever did that is no programmer, but instead, is a 
giant *&^&$*($&&%*$&%(% (censored)
*/

#define TRUE      1
#define FALSE     0

/////////////////////////////////////////////////////////////////////
// defines                                                         //
/////////////////////////////////////////////////////////////////////

#define VICTIM_ATTACKER	false
#define ATTACKER_VICTIM	true

#define WPCMDS_GIVE 0
#define WPCMDS_SELC 1

#define SPAWNFLAG_SUPERFLY	1
#define SPAWNFLAG_MIKIKO	2
// weapon commands
static char *wpcmds[][2] = { 
//       give command                 select command
/*
     {  "w_ballista",                "s_ballista"              },
     {  "w_bolter",                  "s_bolter"                },
     {  "w_c4",                      "s_c4"                    },
     {  "w_celestril",               "s_celestril"             },
     {  "w_daikatana",               "s_daikatana"             },
     {  "w_discus",                  "s_discus"                },
     {  "w_disruptor",               "s_disruptor"             },
     {  "w_flashlight",              "s_flashlight"            },
     {  "w_glock",                   "s_glock"                 },
     {  "w_hammer",                  "s_hammer"                },
     {  "w_ionblaster",              "s_ionblaster"            },
     {  "w_kineticore",              "s_kineticore"            },
     {  "w_metamaser",               "s_metamaser"             },
     {  "w_midas",                   "s_midas"                 },
     {  "w_nightmare",               "s_nightmare"             },
     {  "w_novabeam",                "s_novabeam"              },
     {  "w_ripgun",                  "s_ripgun"                },
     {  "w_shockwave",               "s_shockwave"             },
     {  "w_shotcycler",              "s_shotcycler"            },
     {  "w_sidewinder",              "s_sidewinder"            },
     {  "w_silverclaw",              "s_silverclaw"            },
     {  "w_slugger",                 "s_slugger"               },
     {  "w_stavros",                 "s_stavros"               },
     {  "w_sunflare",                "s_sunflare"              },
     {  "w_tazerhook",               "s_tazerhook"             },
     {  "w_trident",                 "s_trident"               },
     {  "w_venomous",                "s_venomous"              },
     {  "w_wyndrax",                 "s_wyndrax"               },
     {  "w_zeus",                    "s_zeus"                  },
     {  "w_gashands",                "s_gashands"              },
*/
	{	"weapon_give_4",	"weapon_select_4"	},
	{	"weapon_give_2",	"weapon_select_2"	},
	{	"weapon_give_3",	"weapon_select_3"	},
	{	"weapon_give_7",	"weapon_select_7"	},
	{	"weapon_give_1",	"weapon_select_1"	},
	{	"weapon_give_1",	"weapon_select_1"	},
	{	"weapon_give_1",	"weapon_select_1"	},
	{	"weapon_give_4",	"weapon_select_4"	},
	{	"weapon_give_2",	"weapon_select_2"	},
	{	"weapon_give_3",	"weapon_select_3"	},
	{	"weapon_give_6",	"weapon_select_6"	},
	{	"weapon_give_6",	"weapon_select_6"	},
	{	"weapon_give_5",	"weapon_select_5"	},
	{	"weapon_give_4",	"weapon_select_4"	},
	{	"weapon_give_6",	"weapon_select_6"	},
	{	"weapon_give_4",	"weapon_select_4"	},
	{	"weapon_give_5",	"weapon_select_5"	},
	{	"weapon_give_1",	"weapon_select_1"	},
	{	"weapon_give_2",	"weapon_select_2"	},
	{	"weapon_give_3",	"weapon_select_3"	},
	{	"weapon_give_3",	"weapon_select_3"	},
	{	"weapon_give_5",	"weapon_select_5"	},
	{	"weapon_give_2",	"weapon_select_2"	},
	{	"weapon_give_5",	"weapon_select_5"	},
	{	"weapon_give_6",	"weapon_select_6"	},
	{	"w_gashands",		"s_gashands"		},
};

enum wpcmd_enums
{
	WPCMDS_BALLISTA,
    WPCMDS_BOLTER,
	WPCMDS_C4,
	WPCMDS_DAIKATANA,
	WPCMDS_DISCUS,
	WPCMDS_DISRUPTOR,
	WPCMDS_GLOCK,
	WPCMDS_HAMMER,
	WPCMDS_IONBLASTER,
	WPCMDS_KINETICORE,
	WPCMDS_METAMASER,
	WPCMDS_NIGHTMARE,
	WPCMDS_NOVABEAM,
	WPCMDS_RIPGUN,
	WPCMDS_SHOCKWAVE,
	WPCMDS_SHOTCYCLER,
	WPCMDS_SIDEWINDER,
	WPCMDS_SILVERCLAW,
	WPCMDS_SLUGGER,
	WPCMDS_STAVROS,
	WPCMDS_SUNFLARE,
	WPCMDS_TRIDENT,
	WPCMDS_VENOMOUS,
	WPCMDS_WYNDRAX,
	WPCMDS_ZEUS,
    WPCMDS_GASHANDS
};

// SCG[10/8/99]: for surface marks
enum
{
	SM_BLOOD,
	SM_BULLET,
	SM_SCORCH,
	SM_FIST,
	SM_EARTH,
	SM_CLAW,
	SM_NOVA,
	SM_META,
	SM_MULTIBULLET,
	SM_DK_SLASH,
	SM_COUNT
};

/////////////////////////////////////////////////////////////////////
//	globals
/////////////////////////////////////////////////////////////////////

extern  serverState_t   *gstate;
extern  CVector         forward, up, right;
extern  userTrace_t     trace;
extern  CVector         zero_vector;
extern  CVector         zeroVector;
extern  common_export_t *com;
extern  common_export_t *memory_com;

extern  cvar_t  *maxclients;
extern  cvar_t  *deathmatch;
extern  cvar_t  *coop;
extern  cvar_t  *skill;
extern  cvar_t  *p_gravity;
extern  cvar_t  *sv_violence;
extern  cvar_t  *cv_autoaim;

// multiplayer options
extern  cvar_t  *dm_frag_limit;
extern  cvar_t  *dm_time_limit;
extern  cvar_t  *dm_skill_level;
extern  cvar_t  *dm_max_players;

// deathmatch flags
extern  cvar_t  *dm_weapons_stay;
extern  cvar_t  *dm_spawn_farthest;
extern  cvar_t  *dm_allow_exiting;
extern  cvar_t  *dm_same_map;
extern  cvar_t  *dm_force_respawn;
extern  cvar_t  *dm_falling_damage;
//extern  cvar_t  *dm_instant_powerups;
extern  cvar_t  *dm_allow_powerups;
extern  cvar_t  *dm_allow_health;
extern  cvar_t  *dm_allow_armor;
extern  cvar_t  *dm_infinite_ammo;
extern  cvar_t  *dm_fixed_fov;
extern  cvar_t  *dm_teamplay;
extern  cvar_t  *dm_friendly_fire;
extern  cvar_t  *dm_fast_switch;
extern  cvar_t  *dm_footsteps;
extern  cvar_t  *dm_allow_hook;
extern  cvar_t  *dm_item_respawn;
extern  cvar_t  *dm_shotshells;
extern	cvar_t	*dm_instagib;

extern	cvar_t	*dedicated;
#ifdef _DEBUG
	extern	cvar_t	*dk_level;
#endif

// defined in zeus.cpp
int ai_visible(userEntity_t *self, userEntity_t *targ);
void ai_setfov (userEntity_t *self, float fov_degrees);
int ai_infront(userEntity_t *self, userEntity_t *target);
void Cmd_Unholster(userEntity_t *self);

short weapon_select_1( userEntity_t *self );
short weapon_select_2( userEntity_t *self );
short weapon_select_3( userEntity_t *self );
short weapon_select_4( userEntity_t *self );
short weapon_select_5( userEntity_t *self );
short weapon_select_6( userEntity_t *self );
short weapon_select_7( userEntity_t *self );

userInventory_t *weapon_give_1( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_2( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_3( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_4( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_5( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_6( userEntity_t *self, int ammoCount );
userInventory_t *weapon_give_7( userEntity_t *self, int ammoCount );
