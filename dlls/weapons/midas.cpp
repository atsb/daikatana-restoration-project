//---------------------------------------------------------------------------
// MIDAS' GOLDEN GRASP
//---------------------------------------------------------------------------
#include "weapons.h"
#include "midas.h"

enum
{
    // model defs
    MDL_START,

    // sound defs
    SND_START,

    // special frame defs
    ANIM_START,
    ANIM_CATCH,
};

// weapon info def
weaponInfo_t midasWeaponInfo = {
    // names, net names, icon
    "weapon_midas",
    tongue_weapons[T_WEAPON_MIDAS],
    "ammo_pyrite",
    tongue_weapons[T_WEAPON_MIDAS_AMMO],
    "iconname",

    // model files
    {"models/e2/w_midas.dkm",
        "models/e2/a_midas.dkm",
        "models/e2/wa_midas.dkm",
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
        -1,"shoot",0,0,
        -1,"amba",0,0,
        -1,"ambb",0,0,
        -1,"ambc",0,0,
        -1,"catch",0,0,
        0,NULL},

    // commands
    {
      wpcmds[WPCMDS_MIDAS][WPCMDS_GIVE], (void (*)(struct edict_s *))give,
      wpcmds[WPCMDS_MIDAS][WPCMDS_SELC], (void (*)(struct edict_s *))midas_select,
      NULL
    },

    give,
    midas_select,
    command,
    use,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(midas_t),

    300,
    25,       //	ammo_max
    1,        //	ammo_per_use
    1,        //	ammo_display_divisor
    100,        //	initial ammo
    0,        //	speed
    0,        //	range
    0,        //	damage
    0.0,      //	lifetime

    WFL_SPECIAL,

    WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	{NULL,
        NULL,
        NULL},
};

// local weapon info access
static weaponInfo_t *winfo=&midasWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
static userInventory_t *give(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// midas_select()
//---------------------------------------------------------------------------
static short midas_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
static void *command(struct userInventory_s *inv, char *commandStr, void *data)
{
/*
   userEntity_t	*self=(userEntity_t *)data;
   playerHook_t	*phook=(playerHook_t *)self->userHook;

   if (!stricmp(commandStr,"ambient"))
   {
      if (weaponHasAmmo(self,false))
        {
         short animlist1[]={ANIM_AMBIENT_STD,ANIM_AMBIENT2,ANIM_AMBIENT3};
         short animnum;

         animnum=animlist1[(short)(rnd()*2.9)];

         winfoAnimate(winfo,self,animnum,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
         phook->ambient_time=gstate->time+winfoNumFrames(animnum)*.05+2.0+(rnd()*6);
//         gstate->sound (self, CHAN_WEAPON, winfoSound(SND_DISCUSLOOP), 255, ATTN_NORM);
      }
   }
   else
*/
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
static void use(userInventory_s *ptr, userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

    invWeapon->lastFired=gstate->time;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
static void think(userEntity_t *self)
{
}

//-----------------------------------------------------------------------------
//	weapon_midas
//-----------------------------------------------------------------------------
void weapon_midas(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_pyrite
//-----------------------------------------------------------------------------
void ammo_pyrite (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 10, 30.0, NULL);
}

