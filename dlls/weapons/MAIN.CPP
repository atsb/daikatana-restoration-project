//#include <windows.h>
#include "dk_system.h"
#include <stdio.h>

#include	"p_user.h"
#include	"hooks.h"
#include	"common.h"
#include	"weapon_funcs.h"

///////////////////////////////////////////////
// include files for each weapon
///////////////////////////////////////////////

//#include	"celestril.h"// SCG[1/24/00]: not used
#include	"discus.h"
#include	"glock.h"
#include	"ionblaster.h"
#include	"metamaser.h"
//#include	"tazerhook.h"// SCG[1/24/00]: not used
#include	"wyndrax.h"
#include	"c4.h"
#include	"bolter.h"
#include	"sidewinder.h"
//#include	"greekfire.h"// SCG[1/24/00]: not used
#include	"kineticore.h"
#include "shockwave.h"
#include "silverclaw.h"
#include "ripgun.h"
#include "zeus.h"
#include "novabeam.h"
#include "nightmare.h"
#include "stavros.h"
#include "trident.h"
#include "venomous.h"
#include "disruptor.h"
#include "shotcycler.h"
#include "slugger.h"
#include "sunflare.h"
// SCG[11/29/99]: #include "flashlight.h"
#include "ballista.h"
#include "hammer.h"
#include "daikatana.h"
//#include	"testweapon.h"// SCG[1/24/00]: not used
#include "gashands.h"

//unix - dk_dll.h / dk_so.h
#if _MSC_VER
#include	"dk_dll.h"
#else
#include	"dk_so.h"
#endif

#include "w_attribute.h"
#include "dk_io.h"

/////////////////////////////////////////////////////////////////////
//	exports
/////////////////////////////////////////////////////////////////////

DllExport   void    *dll_Query(void);
DllExport   BOOL    dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);
DllExport   BOOL WINAPI DllEntryPoint(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved);

///////////////////////////////////////////////////////////////////////////////
//
//  local imports
//
///////////////////////////////////////////////////////////////////////////////

void weapon_funcs_register_func();

/////////////////////////////////////////////////////////////////////
//	globals
/////////////////////////////////////////////////////////////////////

void giveAmmo(userEntity_t *self);
void giveAllWeapons(userEntity_t *self);
void toggleWeaponMode(userEntity_t *self);

common_export_t *com;
common_export_t *memory_com;
memory_import_t memmgr;

serverState_t   *gstate;
CVector         forward, up, right;
userTrace_t     trace;
CVector         zero_vector(0, 0, 0);
CVector         zeroVector(0,0,0);

cvar_t  *maxclients;
cvar_t  *deathmatch;
cvar_t  *coop;
cvar_t  *skill;
cvar_t  *p_gravity;
cvar_t  *sv_violence;
cvar_t *cv_autoaim;
cvar_t  *developer;
// multiplayer options
//cvar_t  *dm_frag_limit;
//cvar_t  *dm_time_limit;
cvar_t  *dm_skill_level;
cvar_t  *dm_max_players;

// deathmatch flags
cvar_t  *dm_weapons_stay;
cvar_t  *dm_spawn_farthest;
cvar_t  *dm_allow_exiting;
cvar_t  *dm_same_map;
cvar_t  *dm_force_respawn;
cvar_t  *dm_falling_damage;
//cvar_t  *dm_instant_powerups;
cvar_t  *dm_allow_powerups;
cvar_t  *dm_allow_health;
cvar_t  *dm_allow_armor;
cvar_t  *dm_infinite_ammo;
cvar_t  *dm_fixed_fov;
cvar_t  *dm_teamplay;
cvar_t  *dm_friendly_fire;
cvar_t  *dm_fast_switch;
cvar_t  *dm_footsteps;
cvar_t  *dm_allow_hook;
cvar_t  *dm_item_respawn;
cvar_t  *dm_shotshells;
cvar_t	*dm_instagib;
cvar_t  *p_cheats;

cvar_t	*dedicated;

#ifdef _DEBUG
cvar_t	*dk_level;
#endif

char    *dll_Description = "WEAPONS.DLL.  Client weapons.\n";

/////////////////////////////////////////////////////////////////////
// Epairs for Katana Ed
//
// Katana Ed loads this DLL and executes dll_Query, which returns
// a pointer to the epair structure which describes entity types
// which can be included in Daikatana maps
/////////////////////////////////////////////////////////////////////

userEpair_t eclassStr[] =
{
    {"classname",    "weapon_daikatana"},
	{"modelname",	 "models/e1/a_tazer"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 1 weapons
    //---------------------------------
    {"classname",    "weapon_disruptor"},
	{"modelname",	 "models/e1/a_tazer"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_ionblaster"},
	{"modelname",	 "models/e1/a_ion"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
    {"rem",          "ION blaster thingy"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_shotcycler"},
	{"modelname",	 "models/e1/a_shot"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_c4viz"},
	{"modelname",	 "models/e1/a_c4"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_sidewinder"},
	{"modelname",	 "models/e1/a_swindr"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_shockwave"},
	{"modelname",	 "models/e1/a_shokwv"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_gashands"},
	{"modelname",	 "models/e1/a_tazer"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},
    //---------------------------------
    //	episode 2 weapons
    //---------------------------------
    {"classname",    "weapon_discus"},
	{"modelname",	 "models/e2/a_disk"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
    {"rem",          "Discus of Daedalus"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_sunflare"},
	{"modelname",	 "models/e2/a_sflare"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_hammer"},
	{"modelname",	 "models/e2/a_hammer"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_zeus"},
	{"modelname",	 "models/e2/a_zeus"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_trident"},
	{"modelname",	 "models/e2/a_tri"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_venomous"},
	{"modelname",	 "models/e2/a_venom"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 3 weapons
    //---------------------------------
    {"classname",    "weapon_silverclaw"},
	{"modelname",	 "models/e3/a_claw"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_bolter"},
	{"modelname",	 "models/e3/a_bolter"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_ballista"},
	{"modelname",	 "models/e3/a_bal"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

	{"classname",    "weapon_wyndrax"},
	{"modelname",	 "models/e3/a_wyndrx"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_stavros"},
	{"modelname",	 "models/e3/a_stav"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_nightmare"},
	{"modelname",	 "models/e3/a_nmare"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 4 weapons
    //---------------------------------
    {"classname",    "weapon_glock"},
	{"modelname",	 "models/e4/a_glock"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
    {"rem",          "Glock Pistol of Love"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_kineticore"},
	{"modelname",	 "models/e4/a_kcore"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_ripgun"},
	{"modelname",	 "models/e4/a_ripgun"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
    {"rem",          "Pulse Rifle"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_slugger"},
	{"modelname",	 "models/e4/a_slugger"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_novabeam"},
	{"modelname",	 "models/e4/a_nova"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_metamaser"},
	{"modelname",	 "models/e4/a_mmaser"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-20 -20 0 20 20 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 1 ammo
    //---------------------------------
    {"classname",    "ammo_ionpack"},
	{"modelname",	 "models/e1/wa_ion"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "50 ion cells."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_rockets"},
	{"modelname",	 "models/e1/wa_swindr"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "18 rockets."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_shells"},
	{"modelname",	 "models/e1/wa_shot6"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "24 shotcycler shells."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_c4"},
	{"modelname",	 "models/e1/wa_c4"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "8 C4 Modules."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_shocksphere"},
	{"modelname",	 "models/e1/wa_shokwv"},
	{"episode",		 "1"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "2 ShockSpheres."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 2 ammo
    //---------------------------------

    {"classname",    "ammo_tritips"},
	{"modelname",	 "models/e2/wa_trident"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "30 tritips."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_venomous"},
	{"modelname",	 "models/e2/wa_venom"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "25 ammo for Venomous."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_zeus"},
	{"modelname",	 "models/e2/wa_zeus"},
	{"episode",		 "2"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "40 ammo for Zeus."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 3 ammo
    //---------------------------------

    {"classname",    "ammo_bolts"},
	{"modelname",	 "models/e3/wa_bolt"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "crossbow bolts "},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_ballista"},
	{"modelname",	 "models/e3/wa_bal"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "Ballista trunks."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_stavros"},
	{"modelname",	 "models/e3/wa_stav"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "Meteor clips."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_wisp"},
	{"modelname",	 "models/e3/wa_wisp"},
	{"episode",		 "3"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "Wyndrax ammo."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    //---------------------------------
    //	episode 4 ammo
    //---------------------------------
    {"classname",    "ammo_bullets"},
	{"modelname",	 "models/e4/wa_glock"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "20 bullets."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_kineticore"},
	{"modelname",	 "models/e4/wa_kcore"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "25 things."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_novabeam"},
	{"modelname",	 "models/e4/wa_nova"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "25 nova paks."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_cordite"},
	{"modelname",	 "models/e4/wa_cordite"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "5 cordite grenades."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_ripgun"},
	{"modelname",	 "models/e4/wa_rip"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "50 Rip-Ups (tm)."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "ammo_slugger"},
	{"modelname",	 "models/e4/wa_slug"},
	{"episode",		 "4"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
    {"rem",          "50 slugs."},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},


    //	global weapons/items
/*
    {"classname",    "weapon_dk_melee"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_dk_proteus"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_dk_posession"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_dk_barrier"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_dk_dance"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},

    {"classname",    "weapon_dk_banish"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},
*/
/*
    {"classname",    "weapon_flashlight"},
    {"color",        "0.0 0.5 0.8"},
    {"size",         "-16 -16 0 16 16 40"},
	{"flag",		 "SUPERFLY"},
   	{"flag",		 "MIKIKO"},
*/
    {NULL,          NULL},
};

weaponInfo_t *weaponList[WP_NUM_EPISODES+1][WP_WEAPONS_PER_EP] = {
    // episode 1 weapons
    {&disruptorWeaponInfo,
        &ionblasterWeaponInfo,
        &c4WeaponInfo,
        &shotcyclerWeaponInfo,
        &sidewinderWeaponInfo,
        &shockwaveWeaponInfo,
        &gashandsWeaponInfo,
        NULL,
    },

    // episode 2 weapons
    {&discusWeaponInfo,
        &sunflareWeaponInfo,
        &venomousWeaponInfo,
        &hammerWeaponInfo,
        &tridentWeaponInfo,
        &zeusWeaponInfo,
        NULL,
    },

    // episode 3 weapons
    {&silverclawWeaponInfo,
        &bolterWeaponInfo,
        &stavrosWeaponInfo,
        &ballistaWeaponInfo,
        &wyndraxWeaponInfo,
        &nightmareWeaponInfo,
		NULL,
        NULL,
    },

    // episode 4 weapons
    {&glockWeaponInfo,
        &ripgunWeaponInfo,
        &sluggerWeaponInfo,
        &kineticoreWeaponInfo,
        &novabeamWeaponInfo,
        &metamaserWeaponInfo,
		NULL,
        NULL,
    },

    // special weapons (all episodes)
    {&daikatana_WeaponInfo,
//        &flashlightWeaponInfo,
        NULL,                                    // end list with NULL
    },
};

/////////////////////////////////
// dll_Query
/////////////////////////////////

void *dll_Query(void)
{
    return(eclassStr);
}


///////////////////////////////////////////////////////////////////////////////
//	weapons_InitCVars
///////////////////////////////////////////////////////////////////////////////

void    weapons_InitCVars (void)
{
    deathmatch = gstate->cvar ("deathmatch", "0", CVAR_SERVERINFO|CVAR_LATCH);
    coop = gstate->cvar ("coop", "0", CVAR_SERVERINFO|CVAR_LATCH);
    skill = gstate->cvar ("skill", "100", CVAR_SERVERINFO|CVAR_LATCH);
    maxclients = gstate->cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    p_gravity = gstate->cvar ("p_gravity", "4", CVAR_SERVERINFO | CVAR_LATCH);
    sv_violence = gstate->cvar ("sv_violence", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE);
    developer = gstate->cvar("developer", "0", CVAR_ARCHIVE);
//	cv_autoaim = gstate->cvar ("autoaim", "50", CVAR_SERVERINFO | CVAR_LATCH);

//    dm_frag_limit       = gstate->cvar ("dm_frag_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
//    dm_time_limit       = gstate->cvar ("dm_time_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_skill_level      = gstate->cvar ("dm_skill_level", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_max_players      = gstate->cvar ("dm_max_players", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_weapons_stay     = gstate->cvar ("dm_weapons_stay", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_spawn_farthest   = gstate->cvar ("dm_spawn_farthest", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_exiting    = gstate->cvar ("dm_allow_exiting", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_same_map         = gstate->cvar ("dm_same_map", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_force_respawn    = gstate->cvar ("dm_force_respawn", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_falling_damage   = gstate->cvar ("dm_falling_damage", "1", CVAR_SERVERINFO|CVAR_LATCH);
//    dm_instant_powerups = gstate->cvar ("dm_instant_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_powerups   = gstate->cvar ("dm_allow_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_health     = gstate->cvar ("dm_allow_health", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_armor      = gstate->cvar ("dm_allow_armor", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_infinite_ammo    = gstate->cvar ("dm_infinite_ammo", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_fixed_fov        = gstate->cvar ("dm_fixed_fov", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_teamplay         = gstate->cvar ("dm_teamplay", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_friendly_fire    = gstate->cvar ("dm_friendly_fire", "0", 0);
    dm_fast_switch      = gstate->cvar ("dm_fast_switch", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_footsteps        = gstate->cvar ("dm_footsteps", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_hook       = gstate->cvar ("dm_allow_hook", "1", CVAR_SERVERINFO | CVAR_LATCH);
    dm_item_respawn     = gstate->cvar ("dm_item_respawn", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_shotshells       = gstate->cvar ("dm_shotshells", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_instagib		    = gstate->cvar ("dm_instagib", "0", CVAR_SERVERINFO|CVAR_LATCH);
    p_cheats            = gstate->cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);

	dedicated			= gstate->cvar ("dedicated", "0", CVAR_NOSET);
//    g_cvarLogServer = gstate->cvar("log_server", "", CVAR_ARCHIVE);

#ifdef _DEBUG
	dk_level			= gstate->cvar ("dk_level","0",0);
#endif
}

/////////////////////////////////
// dll_ServerLoad
//
// initialization functions called
// when the DLL is loaded
/////////////////////////////////
void do_weapon_give(userEntity_t *self, int num)
{
	weaponInfo_t **winfo;
	short wp;

	if ((gstate->episode < 1) || (gstate->episode > 4))
		return;

    winfo=weaponList[gstate->episode-1];
    for ( wp=0; wp<WP_WEAPONS_PER_EP; wp++,winfo++ )
    {
        if ( *winfo && ((*winfo)->display_order+1 == num) )
		{
			weaponInfo_t *gWinfo = *winfo;
			if (!stricmp(gWinfo->weaponName,"weapon_slugger"))
			{
				int count = gWinfo->initialAmmo;
				count += 2 << 16;
				gWinfo->give_func(self,count);
			}
			else
				gWinfo->give_func(self,gWinfo->initialAmmo);
			return;
		}
    }
}

void weapon_give_1_f(userEntity_t *self)
{
	do_weapon_give(self,1);
}
void weapon_give_2_f(userEntity_t *self)
{
	do_weapon_give(self,2);
}
void weapon_give_3_f(userEntity_t *self)
{
	do_weapon_give(self,3);
}
void weapon_give_4_f(userEntity_t *self)
{
	do_weapon_give(self,4);
}
void weapon_give_5_f(userEntity_t *self)
{
	do_weapon_give(self,5);
}
void weapon_give_6_f(userEntity_t *self)
{
	do_weapon_give(self,6);
}

void weapon_give_gashands_f(userEntity_t *self)
{
	if (gstate->episode == 1)
		gashandsGive(self,0);
}

void ManualNextWeapon(userEntity_t *self, int dir);
void    dll_ServerLoad (serverState_t *state)
{
//
// this really doesn't do anything for the weapons ...
// dllInit code just returns size of userEntity + stateEntity
//

    short first,last,ep,wp;
    weaponInfo_t **winfo;

    gstate = state;

    gstate->Con_Dprintf ("* Loaded WEAPONS.DLL, %i\n", sizeof (playerHook_t));

    first=0;
    last=WP_NUM_EPISODES-1;
    for ( ep=first; ep<=last; ep++ )
    {
        winfo=weaponList[ep];
        for ( wp=0; wp<WP_WEAPONS_PER_EP; wp++ )
        {
            if ( *winfo )
                weaponDllInit(*winfo);
            winfo++;
        }
    }

    // handle special weapons
    winfo=weaponList[WP_NUM_EPISODES];
    while ( *winfo )
    {
        weaponDllInit(*winfo);
        winfo++;
    }

	// set up the serverstate function to allow control of weapon selection from world.
	gstate->WeaponSwitch = ManualNextWeapon;

#ifndef _DEBUG    
	if( gstate->GetCvar( "cheats" ) )
#endif
	{
		gstate->AddCommand("weapon_give_1",weapon_give_1_f);
		gstate->AddCommand("weapon_give_2",weapon_give_2_f);
		gstate->AddCommand("weapon_give_3",weapon_give_3_f);
		gstate->AddCommand("weapon_give_4",weapon_give_4_f);
		gstate->AddCommand("weapon_give_5",weapon_give_5_f);
		gstate->AddCommand("weapon_give_6",weapon_give_6_f);
		gstate->AddCommand("weapon_give_gashands",weapon_give_gashands_f);
	}
}

/////////////////////////////////
// dll_LevelLoad
//
// initialization functions called
// when a level starts
/////////////////////////////////

void dll_LevelLoad (void)
{
    weaponInfo_t **winfo;
    short first,last,wp,ep;

    weapons_InitCVars ();

    // cache weapons for which episodes?
    //   first=last=atoi(gstate->mapName+1)-1;
    first = last = gstate->GetCvar ("sv_episode") - 1;
    if ( first<0 || first>3 )
    {
        first=0;
        last=WP_NUM_EPISODES-1;
    }

    //	reset common weapon list
    com->ClearRegisteredWeapons ();

    // handle episode weapons
    for ( ep=first; ep<=last; ep++ )
    {
        winfo=weaponList[ep];
        for ( wp=0; wp<WP_WEAPONS_PER_EP; wp++ )
        {
            if ( *winfo )
            {
                weaponLevelInit(*winfo);
                //	register weapons with physics.dll for use by AI
                com->RegisterWeapon (*winfo);
            }
            winfo++;
        }
    }

    // handle special weapons
    winfo=weaponList[WP_NUM_EPISODES];
    while ( *winfo )
    {
        weaponLevelInit(*winfo);
		com->RegisterWeapon (*winfo);
        winfo++;
    }


    // "give_all_weapons" cheat
    if(p_cheats->value) // && deathmatch->value)
    {
        gstate->AddCommand("giveall",giveAllWeapons);
        gstate->AddCommand("give_all_weapons",giveAllWeapons);
        gstate->AddCommand("give_ammo",giveAmmo);
    }
//	gstate->AddCommand("toggle_weapon_mode",toggleWeaponMode);
}

//---------------------------------------------------------------------------
// giveAllWeapons()
//---------------------------------------------------------------------------
void giveAllWeapons(userEntity_t *self)
{
    weaponInfo_t **winfo;
    short wp;
    if(developer->value)
    {
        // episode specific weapons
        winfo=weaponList[(short)(gstate->GetCvar("sv_episode")-1)];
        for ( wp=0; wp<WP_WEAPONS_PER_EP; wp++ )
        {
            if ( *winfo )
                (*winfo)->give_func(self,(*winfo)->ammo_max);
            winfo++;
        }

        // special weapons
        winfo=weaponList[WP_NUM_EPISODES];
        while ( *winfo )
        {
            weaponLevelInit(*winfo);
            winfo++;
        }
    }
}

//---------------------------------------------------------------------------
// giveAmmo()
//---------------------------------------------------------------------------
void giveAmmo(userEntity_t *self)
{
    short count;

    if ( gstate->GetArgc() != 2 )
        count=10;
    else
        count=atoi(gstate->GetArgv(1));

    if ( self->winfo )
        ammoAdd(self,count,(weaponInfo_t *)self->winfo);
}

//---------------------------------------------------------------------------
// toggleWeaponMode()
//---------------------------------------------------------------------------
void toggleWeaponMode(userEntity_t *self)
{
    userEntity_t *weapon=(userEntity_t *)self->client->ps.weapon;
    weaponInfo_t **winfo;
    short numEntries;

    // no weapon ... get outta' here!
    if ( !weapon )
        return;

    // find index of current weapon in weaponList
    winfo=(weaponInfo_t **)weaponList;
    numEntries=sizeof(weaponList)/sizeof(weaponList[0][0]);
    for ( int i=0; i<numEntries; i++,winfo++ )
        if ( *winfo && !strcmp(weapon->className,(*winfo)->weaponName) )
            break;

    if ( i==numEntries )
        return;

    // toggle modes (daikatana power <--> standard weapons)
    if ( weapon->team )
    {
        // CURRENT MODE IS DK POWER, CHANGE TO STANDARD WEAPON

        // save current daikatana power
        weapon->health=i;

        // change to standard weapons
        winfo=(weaponInfo_t **)weaponList+((short)weapon->hacks_int);
        if ( (*winfo)->select_func(self)==WEAPON_CHANGED )
            weapon->team=0;
    }
    else
    {
        // CURRENT MODE IS STANDARD WEAPON, CHANGE TO DK POWER

        // save current standard weapon
        weapon->hacks_int = i;

        // change to daikatana power
        winfo=(weaponInfo_t **)weaponList+((short)weapon->health);
        if ( (*winfo)->select_func(self)==WEAPON_CHANGED )
            weapon->team=1;
    }
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Version
//
//	returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version (int size)
{
    if ( size == IONSTORM_DLL_INTERFACE_VERSION )
        return	TRUE;
    else
        return	FALSE;
}

/////////////////////////////////
//	dll_ServerKill
//
/////////////////////////////////

void    dll_ServerKill (void)
{
    //DKLOG_Close();
}

/////////////////////////////////
//	dll_ServerInit
//
/////////////////////////////////

void    dll_ServerInit (void)
{
    memory_com = com = (common_export_t *)gstate->common_exports;

	memmgr.X_Malloc = com->X_Malloc;
	memmgr.X_Free = com->X_Free;

    weapons_InitCVars ();

//	g_cvarLogOutput = gstate->cvar( "LogOutput", "1", CVAR_ARCHIVE );

//	cvar_t* cvarLogFilePath;
//	cvarLogFilePath = gstate->cvar( "LogFilePath", "", CVAR_ARCHIVE );
//	DKLOG_Initialize( cvarLogFilePath->string );

//	DKLOG_Write( LOGTYPE_WEAPONS, 0.0, "dll_ServerInit()" );

	DKIO_Init( gstate->FS_Open,
			   gstate->FS_Close,
			   gstate->FS_FileLength,
			   gstate->FS_Read,
			   gstate->FS_Seek,
			   gstate->FS_Tell,
			   gstate->FS_Getc,
			   gstate->FS_Gets );

	// read the weapon attribute files
	WEAPONATTRIBUTE_ReadFile( "Weapons.csv" );
	//WEAPONATTRIBUTE_ReadFile( "Weapons.cs2" );

	// register functions
	weapon_funcs_register_func();
}

/////////////////////////////////
//	dll_LevelExit
//
/////////////////////////////////

void C4ClearList();
void    dll_LevelExit (void)
{
	C4ClearList();
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Entry
//
//
/////////////////////////////////////////////////////////////////////////////////

BOOL dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
    int         size;

    switch ( dwReasonForCall )
    {
        case    QDLL_VERSION:
            size = *(int *) pvData;
            return	dll_Version (size);
            break;

        case    QDLL_QUERY:
            pvData = eclassStr;
            return(TRUE);
            break;

        case    QDLL_DESCRIPTION:
            pvData = dll_Description;
            return(TRUE);
            break;

        case    QDLL_SERVER_INIT:
            dll_ServerInit ();
            return(TRUE);
            break;

        case    QDLL_SERVER_KILL:
            dll_ServerKill ();
            return(TRUE);
            break;

        case    QDLL_SERVER_LOAD:
            dll_ServerLoad ((serverState_t *) pvData);
            return(TRUE);
            break;

        case    QDLL_LEVEL_LOAD:
            dll_LevelLoad ();
            return(TRUE);
            break;

        case    QDLL_LEVEL_EXIT:
            dll_LevelExit ();
            return(TRUE);
            break;
    }

    return	FALSE;
}
