//---------------------------------------------------------------------------
// DAIKATANA - Barrier
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_barrier.h"

enum {
   // model defs
   MDL_START,

   // sound defs
   SND_START,

   // special frame defs
   ANIM_START,
};

// weapon info def
weaponInfo_t dk_barrierWeaponInfo = {
      // names, net names, icon
      "weapon_dk_barrier",
      tongue_weapons[T_WEAPON_DAIKATANA_BARRIER],
      "ammo_none",
      tongue_weapons[T_WEAPON_NO_AMMO],
      "iconname",

      // model files
      {"models/global/w_daikatana.dkm",
       "models/e4/a_glock.dkm",
	    "models/e4/wa_glock.dkm",
       NULL},

      // sound files
      {SND_NOSOUND,
       "null.wav", // ready 
       "null.wav", // away
       "null.wav", // fire
       "null.wav", // ambient
   	 SND_NOSOUND,
	    SND_NOSOUND,
       NULL},

      // frame names
      {MDL_WEAPON_STD,"ready",0,0,
               -1,"away",0,0,
               -1,"shoota",0,0,
               -1,"amba",0,0,
               ANIM_NONE,
               ANIM_NONE,

               
               0,NULL},

      // commands
      { 
         wpcmds[WPCMDS_DAIKATANA_BARRIER][WPCMDS_GIVE] ,(void (*)(struct edict_s *))d4_give,
         wpcmds[WPCMDS_DAIKATANA_BARRIER][WPCMDS_SELC] ,(void (*)(struct edict_s *))d4_select,
         NULL
      },

      d4_give,
      d4_select,
      d4_command,
      d4_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
      sizeof(dk_barrier_t),

      200,
      1000,		//	ammo_max
      1,		   //	ammo_per_use
      10,		//	ammo_display_divisor
	  0,		//	initial ammo
      0,		//	speed
      0,		//	range
      0,		//	damage
	  0,		//	lifetime

      WFL_SPECIAL | WFL_DK_POWER,		//	flags

	  WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

      {NULL,
       NULL,
       NULL},
    WEAPON_DKBARRIER
};

// local weapon info access
static weaponInfo_t *winfo=&dk_barrierWeaponInfo;

//---------------------------------------------------------------------------
// d4_give()
//---------------------------------------------------------------------------
userInventory_t *d4_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// d4_select()
//---------------------------------------------------------------------------
short d4_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d4_command()
//---------------------------------------------------------------------------
void *d4_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d4_use()
//---------------------------------------------------------------------------
void d4_use(userInventory_s *ptr, userEntity_t *self)
{
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void d4_think(userEntity_t *self)
{
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_dk_barrier_register_func()
{
	gstate->RegisterFunc("dk_barrierThink",d4_think);
	gstate->RegisterFunc("dk_barrieruse",d4_use);
}