//---------------------------------------------------------------------------
// DAIKATANA - Dancing Blade
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_dance.h"

enum {
   // model defs
   MDL_START,

   // sound defs
   SND_START,

   // special frame defs
   ANIM_START,
};

// weapon info def
weaponInfo_t dk_danceWeaponInfo = {
      // names, net names, icon
      "weapon_dk_dance",
      tongue_weapons[T_WEAPON_DAIKATANA_DANCING],
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
       SND_NOSOUND, //"weapons/disruptorrod/disruptorready.wav",
       SND_NOSOUND, //"weapons/disruptorrod/disruptoraway.wav",
       SND_NOSOUND, //"weapons/shotcycler/shotcyclerfire.wav",
       SND_NOSOUND, //"weapons/disruptorrod/disruptorambient1.wav",
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
         wpcmds[WPCMDS_DAIKATANA_DANCING][WPCMDS_GIVE], (void (*)(struct edict_s *))d5_give,
         wpcmds[WPCMDS_DAIKATANA_DANCING][WPCMDS_SELC] ,(void (*)(struct edict_s *))d5_select,
         NULL
      },

      d5_give,
      d5_select,
      d5_command,
      d5_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
      sizeof(dk_dance_t),

      200,
      1000,		//	ammo_max
      1,		   //	ammo_per_use
      10,		//	ammo_display_divisor
	  0,		//	initial ammo
      0,		//	speed
      0,		//	range
      0,		//	damage
	  0.0,		//	lifetime

      WFL_SPECIAL | WFL_DK_POWER,		//	flags
	  WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

      {NULL,
       NULL,
       NULL},
    WEAPON_DKDANCE
};

// local weapon info access
static weaponInfo_t *winfo=&dk_danceWeaponInfo;

//---------------------------------------------------------------------------
// d5_give()
//---------------------------------------------------------------------------
userInventory_t *d5_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// d5_select()
//---------------------------------------------------------------------------
short d5_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d5_command()
//---------------------------------------------------------------------------
void *d5_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d5_use()
//---------------------------------------------------------------------------
void d5_use(userInventory_s *ptr, userEntity_t *self)
{
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void d5_think(userEntity_t *self)
{
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_dk_dance_register_func()
{
	gstate->RegisterFunc("dk_danceThink",d5_think);
	gstate->RegisterFunc("dk_danceuse",d5_use);
}