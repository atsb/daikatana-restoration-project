//---------------------------------------------------------------------------
// DAIKATANA - Banish
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_banish.h"

enum {
   // model defs
   MDL_START,

   // sound defs
   SND_START,

   // special frame defs
   ANIM_START,
};

// weapon info def
weaponInfo_t dk_banishWeaponInfo = {
	// names, net names, icon
	"weapon_dk_banish",
	tongue_weapons[T_WEAPON_DAIKATANA_BANISH],
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
      wpcmds[WPCMDS_DAIKATANA_BANISH][WPCMDS_GIVE], (void (*)(struct edict_s *))d6_give,
	  wpcmds[WPCMDS_DAIKATANA_BANISH][WPCMDS_SELC], (void (*)(struct edict_s *))d6_select,
	  NULL
    },

	d6_give,
	d6_select,
	d6_command,
	d6_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(dk_banish_t),

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
    WEAPON_DKBANISH
};

// local weapon info access
static weaponInfo_t *winfo=&dk_banishWeaponInfo;

//---------------------------------------------------------------------------
// d6_give()
//---------------------------------------------------------------------------
userInventory_t *d6_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// d6_select()
//---------------------------------------------------------------------------
short d6_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d6_command()
//---------------------------------------------------------------------------
void *d6_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d6_use()
//---------------------------------------------------------------------------
void d6_use(userInventory_s *ptr, userEntity_t *self)
{
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void think(userEntity_t *self)
{
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_dk_banish_register_func()
{
	gstate->RegisterFunc("dk_banishThink",think);
	gstate->RegisterFunc("dk_banishuse",d6_use);
}