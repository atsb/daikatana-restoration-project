//---------------------------------------------------------------------------
// DAIKATANA - Proteus
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_proteus.h"

enum {
   // model defs
   MDL_START,

   // sound defs
   SND_START,

   // special frame defs
   ANIM_START,
};

// weapon info def
weaponInfo_t dk_proteusWeaponInfo = {
	// names, net names, icon
	"weapon_dk_proteus",
	tongue_weapons[T_WEAPON_DAIKATANA_PROTEUS],
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
      wpcmds[WPCMDS_DAIKATANA_PROTEUS][WPCMDS_GIVE], (void (*)(struct edict_s *))d2_give,
	  wpcmds[WPCMDS_DAIKATANA_PROTEUS][WPCMDS_SELC], (void (*)(struct edict_s *))d2_select,
	  NULL
    },

	d2_give,
	d2_select,
	d2_command,
	d2_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(dk_proteus_t),

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
    WEAPON_DKPROTEUS
};

// local weapon info access
static weaponInfo_t *winfo=&dk_proteusWeaponInfo;

//---------------------------------------------------------------------------
// d2_give()
//---------------------------------------------------------------------------
userInventory_t *d2_give (userEntity_t *self, int ammoCount)
{
   return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// d2_select()
//---------------------------------------------------------------------------
short d2_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d2_command()
//---------------------------------------------------------------------------
void *d2_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d2_use()
//---------------------------------------------------------------------------
void d2_use(userInventory_s *ptr, userEntity_t *self)
{
   playerHook_t *phook=(playerHook_t *)self->userHook;
   dk_proteus_t *dk_proteus=(dk_proteus_t *)self->curWeapon;
   dk_proteusHook_t *hook;
   userEntity_t *ent;

   if (dk_proteus->active)
      return;

   winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

   // spawn controller
   ent=gstate->SpawnEntity();
   ent->owner=self;
   ent->nextthink=gstate->time+.1;
   ent->think=think_phaseIn;
   ent->remove=controllerRemove;
   ent->s.origin = self->s.origin;
   gstate->LinkEntity(ent);

//   ent->userHook=new dk_proteusHook_t;
	ent->userHook = gstate->X_Malloc(sizeof(dk_proteusHook_t),MEM_TAG_HOOK);
   hook=(dk_proteusHook_t *)ent->userHook;
   hook->killtime=gstate->time + 6;
   hook->prevCameraMode=0;
   hook->dk_proteus=dk_proteus;

   dk_proteus->active=true;

   phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+1;
   phook->fxFrameFunc=think_phaseIn;
}

//---------------------------------------------------------------------------
// think_phaseIn()
//---------------------------------------------------------------------------
void think_phaseIn(userEntity_t *self)
{
   playerHook_t *phook=(playerHook_t *)self->owner->userHook;
   dk_proteusHook_t *hook=(dk_proteusHook_t *)self->userHook;

   phook->fxFrameFunc=NULL;

//   camera_start(self, false);
   self->s.origin = self->owner->s.origin;

   self->think=think_lurking;
   self->nextthink=gstate->time+DK_PROTEUS_PHASEIN_TIME;
}

//---------------------------------------------------------------------------
// think_lurking()
//---------------------------------------------------------------------------
void think_lurking(userEntity_t *self)
{
   dk_proteusHook_t *hook=(dk_proteusHook_t *)self->userHook;

   self->touch = touch_lurking;
   self->s.origin = self->owner->s.origin;

   if (gstate->time >= hook->killtime)
   {
      self->think=self->remove;
      self->nextthink=gstate->time+DK_PROTEUS_PHASEOUT_TIME;
      return;
   }

   self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// touch_lurking()
//---------------------------------------------------------------------------
void touch_lurking(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
}

//---------------------------------------------------------------------------
// controllerRemove()
//---------------------------------------------------------------------------
void controllerRemove(userEntity_t *self)
{
   dk_proteusHook_t *hook=(dk_proteusHook_t *)self->userHook;

//   camera_stop(self->owner, false);
   hook->dk_proteus->active=false;
   gstate->RemoveEntity(self);
}
