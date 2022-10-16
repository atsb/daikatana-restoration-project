//---------------------------------------------------------------------------
// SIDEWINDER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "sidewinder.h"

enum
{
    // model defs
    MDL_START,
    MDL_MISSILE,
    MDL_AMBER,
    MDL_MUZZLEFLASH,

    // sound defs
    SND_START,
    SND_EXPLODE,
    SND_FLYBY,
	SND_UWEXPLODE1,
	SND_UWEXPLODE2,
	SND_UWEXPLODE3,
	SND_UWEXPLODE4,
	SND_UWFIRE,
//   SND_WATERFLYBY,
//   SND_SPLASH,
//   SND_WATERFIRE,

    // frame defs
    ANIM_START,
    ANIM_RELOAD
};

// weapon info def
weaponInfo_t sidewinderWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SIDEWINDER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SIDEWINDER_AMMO),
	// names, net names, icon
	"weapon_sidewinder",
	tongue_weapons[T_WEAPON_SIDEWINDER],
	"ammo_rockets",
	tongue_weapons[T_WEAPON_SIDEWINDER_AMMO],
	"iconname",

	// model files
	{
		"models/e1/w_sidewinder.dkm",
		"models/e1/a_swindr.dkm",
		"models/e1/wa_swindr.dkm",
		"models/e1/we_swrocket.dkm",
		"models/global/we_flareamber.sp2",
		"models/e1/we_mfswinder.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e1/we_sidewinderready.wav",
		"e1/we_sidewinderaway.wav",
		"e1/we_sidewindershoota.wav",
		"e1/we_sidewinderamba.wav",
		"e1/we_sidewinderamba.wav",
		SND_NOSOUND,
		"e1/we_sidewinderexp.wav",
		"e1/we_sidewinderfly.wav",
		"global/e_uwexplodea.wav",
		"global/e_uwexplodeb.wav",
		"global/e_uwexplodec.wav",
		"global/e_uwexploded.wav",
		"e1/we_sideunfire.wav",
//       "e1/we_sidewinderunderwaterflyby.wav",
//       "e1/we_sidewindersplash.wav",
//       "e1/we_sidewinderunderwaterfire.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoot",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,
		ANIM_NONE,//doesn't exist-1,"reload",0,0,
		0,NULL
	},

	// commands
	{
		wpcmds[WPCMDS_SIDEWINDER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_5,
		wpcmds[WPCMDS_SIDEWINDER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_5,
		NULL
	},

	weapon_give_5,
	weapon_select_5,
	sidewinder_command,
	sidewinder_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(weapon_t),

	500,
	50,     //	ammo_max
	2,      //	ammo_per_use
	1,      //	ammo_display_divisor
	20,     //	initial ammo
	950.0,  //	speed
	2000.0, //	range
	90.0,       //	damage
	6.0,        //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_PLAYER_ONLY,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{ 
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_SIDEWINDER],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_SIDEWINDER),
	VICTIM_ATTACKER,

	WEAPON_SIDEWINDER,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&sidewinderWeaponInfo;

#define	SW_THINK_TIME	0.1

typedef struct
{
    float   remove_time;
	CVector start;
	short	inWater;
} sidewinderHook_t;

//---------------------------------------------------------------------------
// sidewinder_give()
//---------------------------------------------------------------------------
userInventory_t *sidewinder_give (userEntity_t *self, int ammoCount)
{
    return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// sidewinder_select()
//---------------------------------------------------------------------------
short   sidewinder_select (userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// sidewinder_command()
//---------------------------------------------------------------------------
void *sidewinder_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

void sidewinder_diethink(userEntity_t *self)
{
	weaponUntrackProjectile(self);
//	gstate->RemoveEntity(self);
}

void rocketTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags,bool full)
{
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_SIDEWINDER,projflags,full);
	if (full)
	{
		tinfo.lightSize = 200;
		tinfo.lightColor.Set(0.8,0.4,0.2);
		tinfo.scale = 1.5;
		tinfo.flags |= TEF_SCALE|TEF_LIGHTSIZE|TEF_LIGHTCOLOR;
	}
}

//---------------------------------------------------------------------------
// sidewinder_explode()
//---------------------------------------------------------------------------
void    sidewinder_explode (userEntity_t *self, CVector &normal)
{
	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;

	int num = com->RadiusDamage (self, self->owner, self, winfo->damage, DAMAGE_SIDEWINDER|DAMAGE_INERTIAL|DAMAGE_EXPLOSION, 128.0);
	WEAPON_HIT(self->owner,num);
	
	// play explosion
//	gstate->StartEntitySound(self,CHAN_WEAPON,gstate->SoundIndex(winfo->sounds[(short)(SND_EXPLODE)]),1.0f,512.0f,4096.0f);
    sidewinderHook_t *hook = (sidewinderHook_t *) self->userHook;
	if (hook && hook->inWater)
		winfoPlayAttn(self,SND_UWEXPLODE1+(int)(3.95*frand()),1.0,1000,2000);
	else
		winfoPlayAttn(self,SND_EXPLODE,1.0,1000,2000);

	// tell the trackent to enter die
	CVector temp;
	VectorToAngles(normal,temp);

	trackInfo_t tinfo;
	rocketTrack(tinfo,self,self->owner,TEF_PROJ_DIE,false);
	tinfo.altpos = temp;
	tinfo.flags |= TEF_ALTPOS;   
	com->trackEntity(&tinfo,MULTICAST_ALL);
    
	self->velocity.Set(0,0,0);
    CVector color(1,.2,.2);
//    flashClients(self, .9, .9, .9, .3, .1);

	self->touch = NULL;
	self->think = sidewinder_diethink;
	self->nextthink = gstate->time + 0.5;
}

//---------------------------------------------------------------------------
// sidewinder_touch()
//---------------------------------------------------------------------------
void sidewinder_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
    if ( self->owner == other || other->flags & FL_EXPLOSIVE )
        return;

    if ( surf && (surf->flags & SURF_SKY) )
    {
		weaponUntrackProjectile(self);
//        gstate->RemoveEntity(self);
		return;
    }
    else
    {
		CVector pln;
		if (plane && plane->normal) // WAW[11/16/99]: Plane might be NULL.
			pln = plane->normal;
		else
			pln = zero_vector;

		sidewinder_explode(self,pln);
		if (plane)	// WAW[11/16/99]: Plane might be NULL.
			weaponScorchMark2(self,other,plane);
	}
}

//---------------------------------------------------------------------------
// sidewinder_think()
//---------------------------------------------------------------------------
void sidewinder_think (userEntity_t *self)
{
    sidewinderHook_t        *hook = (sidewinderHook_t *) self->userHook;

	if( hook == NULL )
	{
		return;
	}

    if ( gstate->time >= hook->remove_time )
    {
		AngleToVectors(self->s.angles,forward,right,up);
        sidewinder_explode(self,zero_vector);
    }
    else
    {
		short inWater;
		if (gstate->PointContents(self->s.origin) & MASK_WATER)
			inWater = 1;
		else
			inWater = 0;

		if (hook->inWater != inWater)
		{
			trackInfo_t tinfo;
//			memset(&tinfo, 0, sizeof(tinfo));
			short flags = TEF_PROJ_FLY;
			if (inWater)
				flags |= TEF_PROJ_WATER;
//			weaponTrackProjectile(tinfo,self,self->owner,winfo,TEF_PROJ_ID_SIDEWINDER,flags);
			rocketTrack(tinfo,self,self->owner,flags,false);
			com->trackEntity(&tinfo,MULTICAST_ALL);
		}

		hook->inWater = inWater;
		CVector distance = hook->start - self->s.origin;
        if(distance.Length() > 400 && self->velocity.Length() == winfo->speed)
        {
            self->velocity = self->velocity * 2;
        }
		self->nextthink = gstate->time + SW_THINK_TIME;
    }

}


//---------------------------------------------------------------------------
// sidewinder_fire()
//---------------------------------------------------------------------------
void sidewinder_fire (userEntity_t *self)
{
	WEAPON_FIRED(self->owner);

    userEntity_t    *rocket;
    CVector         temp, kick_angles;
    sidewinderHook_t        *hook;
    int             pc;
    float           forward_vel, kick_velocity=-90;

    pc = gstate->PointContents (self->s.origin);
	short inWater;
    if ( pc & MASK_WATER )
	{
        forward_vel = winfo->speed / 3.0;
		inWater = 1;
	}
    else
	{
        forward_vel = winfo->speed;
		inWater = 0;
	}

	CVector offset;
	if (self->team == 2)
		offset.Set(winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1);
	else
		offset.Set(winfo->fWeaponOffsetX2, winfo->fWeaponOffsetY2, winfo->fWeaponOffsetZ2);

    rocket = weapon_spawn_projectile (self->owner, offset, forward_vel, 0, "projectile_missile");
    rocket->movetype = MOVETYPE_FLYMISSILE;
    rocket->flags = FL_EXPLOSIVE;
	rocket->s.render_scale.Set(0.001,0.001,0.001);
    gstate->SetSize(rocket, -2, -2, -2, 2, 2, 2);
    rocket->s.renderfx = SPR_ALPHACHANNEL;
    winfoSetModel(rocket,MDL_MISSILE);
	rocket->s.angles.roll += 90*(self->team - 1);

    rocket->userHook = gstate->X_Malloc(sizeof(sidewinderHook_t),MEM_TAG_HOOK);
    hook = (sidewinderHook_t *) rocket->userHook;

    rocket->touch = sidewinder_touch;
    rocket->nextthink = gstate->time + 0.1;
    rocket->think = sidewinder_think;

	hook->inWater = inWater;

    hook->remove_time = gstate->time + winfo->lifetime;
	hook->start = rocket->s.origin;

    gstate->LinkEntity (rocket);
	kick_angles.Set(-1,0,0);
	weapon_kick(self->owner, kick_angles, kick_velocity, 200, 200);

	trackInfo_t tinfo;
	rocketTrack(tinfo,rocket,self->owner,TEF_PROJ_FLY|TEF_PROJ_LAUNCH,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// sidewinder_firethink()
//---------------------------------------------------------------------------
void sidewinder_firethink (userEntity_t *self)
{
    if ( !weaponHasAmmo (self->owner, true) )
    {
        gstate->RemoveEntity (self);
        return;
    }

    weaponUseAmmo (self->owner, true);
    sidewinder_fire (self);

    self->team--;
    if ( self->team > 0 )
    {
        self->nextthink = gstate->time + 0.01;	// fire the next rocket really soon...
        self->think = sidewinder_firethink;
    }
    else
        gstate->RemoveEntity (self);
}

//---------------------------------------------------------------------------
// sidewinder_use()
//---------------------------------------------------------------------------
void    sidewinder_use (userInventory_s *ptr, userEntity_t *self)
{
    userEntity_t    *fire_ent;

    //	just check amount of ammo available
    if ( !weaponHasAmmo(self,true) )
        return;

    fire_ent = gstate->SpawnEntity ();
    fire_ent->owner = self;
	fire_ent->flags |= FL_NOSAVE;
    fire_ent->modelName = NULL;
    fire_ent->movetype = MOVETYPE_NONE;
    fire_ent->solid = SOLID_NOT;
    fire_ent->elasticity = 1.0;
    fire_ent->team = 2;				// shoot two projectiles!
    gstate->LinkEntity(fire_ent);

    sidewinder_firethink (fire_ent);


    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

	if (gstate->PointContents (self->s.origin) & MASK_WATER)
		winfoPlay(self,SND_UWFIRE,0.7f);
	else
		winfoPlay(self,SND_SHOOT_STD,1.0f);
}

//-----------------------------------------------------------------------------
//	weapon_sidewinder
//-----------------------------------------------------------------------------
void weapon_sidewinder(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_rockets
//-----------------------------------------------------------------------------
void ammo_rockets (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 18, 30.0, NULL);
	ammo_set_sound(self, gstate->SoundIndex("global/i_swinderammo.wav"));
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_sidewinder_register_func()
{
	gstate->RegisterFunc("sidewinder_think",sidewinder_think);
	gstate->RegisterFunc("sidewinder_use",sidewinder_use);
	gstate->RegisterFunc("sidewinder_touch",sidewinder_touch);
	gstate->RegisterFunc("sidewinder_give",sidewinder_give);
	gstate->RegisterFunc("sidewinder_select",sidewinder_select);
}
