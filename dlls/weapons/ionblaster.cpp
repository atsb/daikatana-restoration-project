//---------------------------------------------------------------------------
// ION BLASTER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "ionblaster.h"


enum
{
    // model defs
    MDL_START,
    MDL_SHOT,
    MDL_DISSIPATE,
    MDL_WALLEXP,
//    MDL_MUZZLEFLASH,
	MDL_FLARE,

    // sound defs
    SND_START,
    SND_EXPLODE1,
    SND_EXPLODE2,
    SND_EXPLODE3,
    SND_FLYBY,
    SND_HIT,
	SND_WALLHIT1,
	SND_WALLHIT2,
	SND_WALLHIT3,
	SND_WALLHIT4,

    // frame defs
    ANIM_START,
    ANIM_WALLEXP,
    ANIM_SHOT,
    ANIM_DISSIPATE
//    ANIM_MUZZLEFLASH,
};

// weapon info def
weaponInfo_t ionblasterWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_IONBLASTER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_IONBLASTER_AMMO),
	// names, net names, icon
	"weapon_ionblaster",
	tongue_weapons[T_WEAPON_IONBLASTER],
	"ammo_ionpack",
	tongue_weapons[T_WEAPON_IONBLASTER_AMMO],
	"iconname",

		// model files
	{
		"models/e1/w_ionblaster.dkm",
		"models/e1/a_ion.dkm",
		"models/e1/wa_ion.dkm",
		"models/e1/we_ionbl.dkm",
		"models/e1/we_iondis.sp2",
		"models/e1/we_ionexpl.sp2",
		"models/e1/we_mfion.sp2",
		"models/e1/we_ionbf.sp2",
		NULL
	},

	// sound files
	{
		"e1/we_ionamba.wav",
		"e1/we_ionready.wav",
		"e1/we_ionaway.wav",
		"e1/we_ionshootb.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e1/we_ionexplodea.wav",
		"e1/we_ionexplodeb.wav",
		"e1/we_ionexplodec.wav",
		"e1/we_ionflyby.wav",
		"e1/we_ionhit.wav",
		"global/e_electronsprka.wav",
		"global/e_electronsprke.wav",
		"global/e_electronsprkg.wav",
		"global/e_electronsprkh.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,//doesn't exist-1,"ambc",0,0,
		MDL_WALLEXP,"sp2",0,0,
		MDL_SHOT,"sp2",0,0,
		MDL_DISSIPATE,"sp2",0,0,
		0,
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_IONBLASTER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_2,
		wpcmds[WPCMDS_IONBLASTER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_2,
		NULL
	},

	ionblaster_give,
	ionblaster_select,
	ionblaster_command,
	ionblaster_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(weapon_t),

	200,
	100,        //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	40,     //	initial ammo
	1840.0,  //	speed
	2000.0, //	range
	30.0,       //	damage
	0.0,        //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR,       //WFL_LOOPAMBIENT_SND,		//	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{ 
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_IONBLASTER],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_IONBLASTER),
	VICTIM_ATTACKER,

	WEAPON_IONBLASTER,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&ionblasterWeaponInfo;

//---------------------------------------------------------------------------
// ionblaster_give()
//---------------------------------------------------------------------------
userInventory_t *ionblaster_give (userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// ionblaster_select()
//---------------------------------------------------------------------------
short ionblaster_select (userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// ionblaster_command()
//---------------------------------------------------------------------------
void *ionblaster_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

void blastTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags,bool full)
{
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_IONBLASTER,projflags,full);
	if (full)
	{
		tinfo.lightSize = 300;
		tinfo.lightColor.Set(0.0,0.8,0.0);
		tinfo.scale = 3;
		tinfo.scale2 = 0.75;
		tinfo.length = gstate->ModelIndex("models/e1/we_ionbf.sp2");
		tinfo.flags |= TEF_SCALE|TEF_SCALE2|TEF_LIGHTSIZE|TEF_LIGHTCOLOR|TEF_LENGTH;
	}
}

//-----------------------------------------------------------------------------
//	ionblaster_use
//-----------------------------------------------------------------------------
void ionblaster_use (userInventory_s *ptr, userEntity_t *self)
{
    if ( !weaponHasAmmo(self,true) )
        return;

	weaponUseAmmo(self,true);

    playerHook_t *phook = (playerHook_t *) self->userHook;
	if (!phook)
		return;

	userEntity_t *ionBlast;
	CVector offset(winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1);
    ionBlast = weapon_spawn_projectile (self, offset, winfo->speed/1, 0, "projectile_missile");
    ionBlast->movetype		= MOVETYPE_BOUNCEMISSILE;
    ionBlast->touch			= (touch_t)ionblaster_touch;
    ionBlast->nextthink		= gstate->time + 0.1;
    ionBlast->think			= (think_t)ionblaster_think;
	ionBlast->remove		= ionblaster_remove;
	ionBlast->flags			|= FL_IGNORE_ARMOR;
    ionBlast->s.render_scale.Set(0.001,0.001,0.001);

    winfoSetModel(ionBlast,MDL_SHOT);
    winfoAnimate(winfo,ionBlast,ANIM_SHOT,FRAME_LOOP,.05);
//    gstate->SetSize (ionBlast, -8, -8, -8, 8, 8, 8);
    gstate->SetSize (ionBlast, -2, -2, -2, 2, 2, 2);


	gstate->LinkEntity(ionBlast);
    // malloc the memory for the weapon hook
    ionBlast->userHook = gstate->X_Malloc(sizeof(ionHook_t),MEM_TAG_HOOK);
    ionHook_t *hook = (ionHook_t *) ionBlast->userHook;
	hook->owner = self;
	hook->lastImpactNorm.Set(0,0,0);
	
	weaponMuzzleFlash(	self,self,winfo,FLASH_ION);
	// track the projectile to do cool client effects
	winfoPlayLooped(ionBlast,SND_FLYBY,1.0f);

	trackInfo_t tinfo;
	blastTrack(tinfo,ionBlast,self,TEF_PROJ_FLY|TEF_PROJ_LAUNCH,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
    
    phook->fired++;

	weapon_face_velocity(ionBlast);	
	winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
    winfoPlay(self,SND_SHOOT_STD,1.0f);
}

//-----------------------------------------------------------------------------
//	ionblaster_think
//-----------------------------------------------------------------------------
void ionblaster_think (userEntity_t *self)
{
    ionHook_t *hook = (ionHook_t *) self->userHook;

    if (!hook || (self->velocity.Length() == 0) || self->groundEntity)
    {
		self->touch = NULL;
		self->think = ionblaster_diethink;
		self->nextthink = gstate->time + 0.1;
        return;
    }

    if( (gstate->PointContents(self->s.origin) & MASK_WATER) )
    {
		if (gstate->PointContents(hook->owner->s.origin) & MASK_WATER)
	        com->FlashClient( hook->owner, 0.6, 0, 1, 0, .2 );

		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

        int num = com->RadiusDamage(self, hook->owner, NULL, winfo->damage, DAMAGE_NONE, 64.0);
		WEAPON_HIT(hook->owner,num)

		trackInfo_t tinfo;
		blastTrack(tinfo,self,hook->owner,TEF_PROJ_DIE,false);
		tinfo.Long2 = gstate->ModelIndex("models/e1/we_ionexp.dkm");	// set the water explode model
		tinfo.flags |= TEF_LONG2;
		com->trackEntity(&tinfo,MULTICAST_ALL);

		self->velocity.Set(0,0,0);
		self->touch = NULL; // no spunk left!
		self->think = ionblaster_remove;
		self->nextthink = gstate->time + 0.2;
        return;
    }

	self->think = ionblaster_think;
	self->nextthink = gstate->time + 0.1;
}

//-----------------------------------------------------------------------------
//	ionblaster_touch
//-----------------------------------------------------------------------------
void ionblaster_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
	if (!plane || !surf)
		return;

    ionHook_t *hook = (ionHook_t *) self->userHook;


	if (!hook || (surf && (surf->flags & SURF_SKY)))
	{
		self->touch = NULL;
		winfoStopLooped(self);
		weaponUntrackProjectile(self);
//		gstate->RemoveEntity(self);
		return;
	}

    hook->bounce_count++;
	weapon_face_velocity(self);	
	if (plane)
		hook->lastImpactNorm = plane->normal;

	if (hook->bounce_count > 0)
		self->owner = self;		// let the player get some too!

    if ( other->takedamage != DAMAGE_NO )
    {
		float damage = winfo->damage;
		if (other == hook->owner)
			damage *= 0.5;

		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

        com->Damage(other, self, hook->owner, self->s.origin, zero_vector, damage, DAMAGE_NONE);
		WEAPON_HIT(hook->owner,1);

		winfoPlayAttn(self,SND_EXPLODE1+rnd()*2.5,1.0f,500,1000);
        winfoAnimate(winfo,self,ANIM_DISSIPATE,FRAME_ONCE,.05);

		self->touch = NULL;
		self->think = ionblaster_diethink;
		self->nextthink = gstate->time + 0.1;
        return;
    }

    if (hook->bounce_count >= 3)
    {
		self->touch = NULL;
		self->think = ionblaster_diethink;
		self->nextthink = gstate->time + 0.1;
        return;
    }

	// speed it up
	self->velocity.Multiply(1.25);
	// do a cool impact thingy 
	winfoPlayAttn(self,SND_WALLHIT1+rnd()*3.5,1.0f,256,525);
//	VectorToAngles(plane->normal,forward);
	VectorToAngles(hook->lastImpactNorm,forward);

	trackInfo_t tinfo;
	blastTrack(tinfo,self,hook->owner,TEF_PROJ_FLY|TEF_PROJ_SPECIAL,false);
	tinfo.altpos = forward;
	tinfo.modelindex = gstate->ModelIndex("models/global/we_ioexp.dkm");// use the cool impact explosion!
	tinfo.flags |= TEF_ALTPOS|TEF_MODELINDEX;
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

//-----------------------------------------------------------------------------
//	ionblaster_diethink
//-----------------------------------------------------------------------------
void ionblaster_diethink(userEntity_t *self)	// basically, makes it fade away nicely.  :o
{
    ionHook_t *hook = (ionHook_t *) self->userHook;
	if (!hook)
	{
		winfoStopLooped(self);	
		gstate->RemoveEntity(self);
		return;
	}

	VectorToAngles(hook->lastImpactNorm,forward);
	self->velocity.Set(0,0,0);
	self->touch = NULL; // no spunk left!

	// the final blammo
	winfoPlayAttn(self,SND_HIT,1.0f,500,1000);

	trackInfo_t tinfo;
	blastTrack(tinfo,self,hook->owner,TEF_PROJ_DIE,false);
	tinfo.altpos = -forward;
	tinfo.modelindex = gstate->ModelIndex("models/e1/we_ionexpl.sp2");		// kinda make it dissipate.
	tinfo.flags |= TEF_MODELINDEX|TEF_ALTPOS;
	com->trackEntity(&tinfo,MULTICAST_ALL);

	self->think = ionblaster_remove;
	self->nextthink = gstate->time + 0.1;	// give it time!
}

//-----------------------------------------------------------------------------
//	ionblaster_remove
//-----------------------------------------------------------------------------
void ionblaster_remove(userEntity_t *self)
{
	winfoStopLooped(self);	
	weaponUntrackProjectile(self);
//	gstate->RemoveEntity(self);
}

//-----------------------------------------------------------------------------
//	weapon_ionblaster
//-----------------------------------------------------------------------------
void weapon_ionblaster(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_ionpack
//-----------------------------------------------------------------------------
void ammo_ionpack (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 50, 30.0, NULL);
	ammo_set_sound(self, gstate->SoundIndex("global/i_ionammo.wav"));
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_ionblaster_register_func()
{
	gstate->RegisterFunc("ionblaster_diethink",ionblaster_diethink);
	gstate->RegisterFunc("ionblaster_touch",ionblaster_touch);
	gstate->RegisterFunc("ionblaster_remove",ionblaster_remove);
	gstate->RegisterFunc("ionblaster_think",ionblaster_think);
	gstate->RegisterFunc("ionblaster_use",ionblaster_use);
	gstate->RegisterFunc("ionblaster_give",ionblaster_give);
	gstate->RegisterFunc("ionblaster_select",ionblaster_select);
}
