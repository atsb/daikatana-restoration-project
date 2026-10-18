//---------------------------------------------------------------------------
// DAIKATANA - Posession
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_posession.h"

enum {
   // model defs
   MDL_START,

   // sound defs
   SND_START,

   // special frame defs
   ANIM_START,
};

// weapon info def
weaponInfo_t dk_posessionWeaponInfo = {
	// names, net names, icon
	"weapon_dk_posession",
	tongue_weapons[T_WEAPON_DAIKATANA_POSSESSION],
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
      wpcmds[WPCMDS_DAIKATANA_POSSESSION][WPCMDS_GIVE], (void (*)(struct edict_s *))d3_give,
	  wpcmds[WPCMDS_DAIKATANA_POSSESSION][WPCMDS_SELC], (void (*)(struct edict_s *))d3_select,
	  NULL
    },

	d3_give,
	d3_select,
	d3_command,
	d3_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(dk_posession_t),

	200,
	1000,	//	ammo_max
	1,		//	ammo_per_use
	10,		//	ammo_display_divisor
	0,		//	initial ammo
	0,		//	speed
	0,		//	range
	0,		//	damage
	0.0,	//	lifetime

	WFL_SPECIAL | WFL_DK_POWER,		//	flags
	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	{NULL,
	NULL,
	NULL},
    WEAPON_DKPOSESSION
};

// local weapon info access
static weaponInfo_t *winfo=&dk_posessionWeaponInfo;

//---------------------------------------------------------------------------
// d3_give()
//---------------------------------------------------------------------------
userInventory_t *d3_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// d3_select()
//---------------------------------------------------------------------------
short d3_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d3_command()
//---------------------------------------------------------------------------
void *d3_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d3_use()
//---------------------------------------------------------------------------
void d3_use(userInventory_s *ptr, userEntity_t *self)
{
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void d3_think(userEntity_t *self)
{
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_dk_posession_register_func()
{
	gstate->RegisterFunc("dk_posessionThink",d3_think);
	gstate->RegisterFunc("dk_posessionuse",d3_use);
}