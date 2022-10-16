//---------------------------------------------------------------------------
// KINETICORE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "kineticore.h"

enum
{
    // model defs
    MDL_START,
    MDL_SHOT,
    MDL_ICE,

    // sound defs
    SND_START,
    SND_HIT,
    SND_EXPLODE,
    SND_FLYBY1,
	SND_FLYBY2,
	SND_FLYBY3,
	SND_CLICK,

    // frame defs
    ANIM_START,
    ANIM_RELOAD,
    ANIM_SHOT
};

// weapon info def
weaponInfo_t kineticoreWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_KINETICORE),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_KINETICORE_AMMO),
	// names, net names, icon
	"weapon_kineticore",
	tongue_weapons[T_WEAPON_KINETICORE],
	"ammo_kineticore",
	tongue_weapons[T_WEAPON_KINETICORE_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_kcore.dkm",
		"models/e4/a_kcore.dkm",
		"models/e4/wa_kcore.dkm",
		"models/e4/we_kcoreshot.sp2",
		"models/e4/we_ice.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_kcoreready.wav",
		"e4/we_kcoreaway.wav",
		"e4/we_kcoreshoota.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e4/we_kcorehita.wav",
		"e4/we_kcoreexp.wav",
		"e4/we_kcoreflybya.wav",
		"e4/we_kcoreflybyb.wav",
		"e4/we_kcoreflybyc.wav",
		"e4/we_glockclick.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		ANIM_NONE,
		ANIM_NONE,
		-1,"reload",0,0,
		MDL_SHOT,"sp2",0,0,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_KINETICORE][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_3,
		wpcmds[WPCMDS_KINETICORE][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_3,
		NULL
	},

	weapon_give_3,
	weapon_select_3,
	kineticore_command,
	kineticore_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(weapon_t),

	400,
	200,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	50,         //	initial ammo
	1250.0,     //	speed
	1875.0,     //	range
	13.0,       //	damage
	1.5,        //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_PLAYER_ONLY,      //	flags

	WEAPONTYPE_SHOULDER,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_KINETICORE],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_KINETICORE),
	VICTIM_ATTACKER,

	WEAPON_KINETICORE,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&kineticoreWeaponInfo;
static short iceCount = 0;
#define MAX_ICE_COUNT 10

//---------------------------------------------------------------------------
// kineticore_give()
//---------------------------------------------------------------------------
userInventory_t *kineticore_give (userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// kineticore_select()
//---------------------------------------------------------------------------
short   kineticore_select (userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// kineticore_command()
//---------------------------------------------------------------------------
void *kineticore_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

void kcoreTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags,bool full)
{
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_KCORE,projflags,full);
	if (full)
	{
		tinfo.scale = 0.8;
		tinfo.flags |= TEF_SCALE;
	}
}

void kcoreRemoveThink(userEntity_t *ent)
{
	weaponUntrackProjectile(ent);
}

//---------------------------------------------------------------------------
// kcoreRemoveEnt()
//---------------------------------------------------------------------------
void kcoreRemoveEnt(userEntity_t *ent)
{
	winfoStopLooped(ent);

    kcoreHook_t *hook = (kcoreHook_t *) ent->userHook;
	if (hook)
	{
		trackInfo_t tinfo;
		kcoreTrack(tinfo,ent,hook->owner,TEF_PROJ_DIE,false);
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}

	ent->think = kcoreRemoveThink;
	ent->nextthink = gstate->time + 0.1;

	if (rand() & 0x01)
		winfoPlay(ent,SND_EXPLODE,1.0f);
}

//---------------------------------------------------------------------------
// kineticore_think()
//---------------------------------------------------------------------------
void    kineticore_think (userEntity_t *self)
{
    kcoreHook_t *hook = (kcoreHook_t *) self->userHook;
	if (!hook)
		return;

	if (self->velocity.Length() < winfo->speed)
	{
		self->velocity.Multiply(2);
		if (self->velocity.Length() > winfo->speed)
		{
			self->velocity.Normalize();
			self->velocity.Multiply(winfo->speed);
		}
	}
    if ( self->groundEntity || (gstate->time >= hook->expire_time) )
    {
		self->touch = NULL;
        kcoreRemoveEnt(self);
        return;
    }

    self->think = kineticore_think;
    self->nextthink = gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// kineticore_touch()
//---------------------------------------------------------------------------
void    kineticore_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
    kcoreHook_t *hook = (kcoreHook_t *) self->userHook;
    float       damage;

	if ( !hook || (surf && (surf->flags & SURF_SKY)) )
    {
		self->touch = NULL;
		winfoStopLooped(self);
		weaponUntrackProjectile(self);
//		gstate->RemoveEntity(self);
        return;
    }

    winfoPlay(self,SND_HIT,1.0f);

    if ( (other->takedamage == DAMAGE_NO))
    {
		if ( plane && !deathmatch->value && !coop->value)
		{
			CVector temp;
			VectorToAngles(plane->normal,temp);

			trackInfo_t tinfo;
			kcoreTrack(tinfo,self,hook->owner,TEF_PROJ_SPECIAL|TEF_PROJ_FLY,false);
			tinfo.altpos = temp;
			tinfo.modelindex = plane->planeIndex;
			tinfo.flags |= TEF_ALTPOS|TEF_MODELINDEX;
			com->trackEntity(&tinfo,MULTICAST_ALL);
		}

		hook->bounce_count++;
		self->owner = self;
        return;
    }

	// don't kick player's butt till we bounce
	if ((hook->bounce_count == 0) && (other == hook->owner))
		return;

    //	calculate damage based on number of bounces
//	damage = 15.0 - (BOUNCE_DAMAGE * (float) hook->bounce_count);
    damage = 2 + winfo->damage * (hook->expire_time-gstate->time) / winfo->lifetime;
	if (hook->owner == other)
		damage *= 0.5;

	if( damage < 5 )
		damage = 5;

	WEAPON_HIT(hook->owner,1);
	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;
	com->Damage (other, self, hook->owner, self->s.origin, zero_vector, damage, DAMAGE_INERTIAL | DAMAGE_INFLICTOR_VEL);

	if ( (other->flags & (FL_CLIENT|FL_MONSTER)) && (other->userHook) && (gstate->damage_inflicted > 0))
	{
		playerHook_t *ohook = (playerHook_t *)other->userHook;
		ohook->freezeLevel += 0.2;
	}

    kcoreRemoveEnt(self);
}

//---------------------------------------------------------------------------
// kineticore_fire()
//---------------------------------------------------------------------------
void    kineticore_fire (userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    kcoreHook_t     *hook;
    userEntity_t    *proj;      //,*weaponEnt;
    CVector         org,temp;
    CVector v;

	CVector kick_angles(-1,0,0);
	float kick_velocity=-90;
	weapon_kick(self, kick_angles, kick_velocity, 200, 200);

    // weapon kickback
    temp=self->client->v_angle;
    temp.AngleToVectors(forward,right,up);
    forward=-forward;
    v = forward;

    self->client->kick_origin.x += v.x*4;
    self->client->kick_origin.y += v.y; 
    self->client->kick_origin.z += v.z*4;

    winfoPlay(self,SND_SHOOT_STD,1.0f);
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    proj=weapon_spawn_projectile(self, offset, 0.25*winfo->speed, 0.0, "projectile_kineticore");

	winfoPlayLooped(proj,SND_FLYBY1 + (rand() % 3),1.0);
    proj->movetype=MOVETYPE_BOUNCEMISSILE;
    proj->elasticity=1;

    winfoSetModel(proj,MDL_SHOT);
    winfoAnimate(winfo,proj,ANIM_SHOT,FRAME_LOOP,.05);
    proj->s.renderfx = SPR_ALPHACHANNEL;
//    proj->s.render_scale.Set( 0.8, 0.8, 0.8 );
    proj->s.render_scale.Set( 0.001, 0.001, 0.001 );

    //	com->SetVector(proj->s.render_scale,5,5,5);
    gstate->SetSize(proj, -8, -8, -8, 8, 8, 8);

    //	set up hook
//    proj->userHook = new kcoreHook_t;
	proj->userHook = gstate->X_Malloc(sizeof(kcoreHook_t),MEM_TAG_HOOK);
    hook=(kcoreHook_t *)proj->userHook;

    hook->bounce_count=0;
    hook->expire_time=gstate->time + winfo->lifetime;
	hook->owner = self;
	proj->owner = self;

    proj->touch = (touch_t)kineticore_touch;
    proj->think = kineticore_think;
    proj->nextthink = gstate->time + 0.1;

	gstate->LinkEntity(proj);

    org=proj->s.origin;

    forward=-forward;

    invWeapon->lastFired=gstate->time;
	WEAPON_FIRED(self);
	// let it snow...let it snow...let it snow...
	trackInfo_t tinfo;
	kcoreTrack(tinfo,proj,self,TEF_PROJ_LAUNCH|TEF_PROJ_FLY,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// kineticore_usethink()
//---------------------------------------------------------------------------
void    kineticore_usethink (userEntity_t *self)
{
    if ( !weaponHasAmmo (self->owner, true) )
    {
        //	don't fire any longer, should be auto-changing
        gstate->RemoveEntity (self);
        return;
    }

    weaponUseAmmo (self->owner, true);
    kineticore_fire (self->owner);

    self->team--;
    if ( self->team > 0 )
    {
        self->nextthink = gstate->time + 0.1;
        self->think = kineticore_usethink;
    }
    else
        gstate->RemoveEntity (self);
}

//---------------------------------------------------------------------------
// kineticore_use()
//---------------------------------------------------------------------------
void    kineticore_use (userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t    *phook;
    userEntity_t    *fire_ent;

    //	just check amount of ammo available
    if ( !weaponHasAmmo(self,true) )
	{
        winfoPlay(self,SND_CLICK,1.0f);
        return;
	}

    phook = (playerHook_t *) self->userHook;
	if (!phook)
		return;

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

	float delay = 1.0;
	if (phook->attack_boost)
		delay /= phook->attack_boost;

    phook->attack_finished += delay;

    fire_ent = gstate->SpawnEntity ();
	fire_ent->flags |= FL_NOSAVE;
    fire_ent->owner = self;
    fire_ent->modelName = NULL;
    fire_ent->movetype = MOVETYPE_NONE;
    fire_ent->solid = SOLID_NOT;
    fire_ent->elasticity = 1.0;

    fire_ent->team = 5;

    kineticore_usethink (fire_ent);

//    winfoPlay(self,SND_SHOOT_STD,1.0f);

    gstate->LinkEntity(fire_ent);
}

#define MAX_ICE_VEL  150

//---------------------------------------------------------------------------
// spawnIce()
//---------------------------------------------------------------------------
void spawnIce(userEntity_t *self)
{
    kcoreHook_t *hook = (kcoreHook_t *) self->userHook;
	if (!hook)
	{
		weaponUntrackProjectile(self);
//		gstate->RemoveEntity(self);
		return;
	}

	self->touch = NULL;
    float dif,size,sizex,sizey,sizez;
	size = 1.0;
	dif = 0.5;

	iceCount++;
	sizex = size+(rnd()-0.5)*dif;
	sizey = size+(rnd()-0.5)*dif;
	sizez = size+(rnd()-0.5)*dif;

    gstate->SetSize(self,-sizex,-sizey,-sizez,sizex,sizey,sizez);
	self->hacks = gstate->time + 2;
	self->think = iceThink;
    self->nextthink=gstate->time+.1;
	self->mass = 1.85f;
    self->s.render_scale.Set(0.01,0.01,0.01);
	self->velocity.x = rnd() > 0.5 ? rnd()*MAX_ICE_VEL : -rnd()*MAX_ICE_VEL;
	self->velocity.y = rnd() > 0.5 ? rnd()*MAX_ICE_VEL : -rnd()*MAX_ICE_VEL;
	self->velocity.z = rnd() > 0.5 ? rnd()*MAX_ICE_VEL*2 : -rnd()*MAX_ICE_VEL*2;
	self->movetype=MOVETYPE_FLYMISSILE;
    self->solid=SOLID_NOT;
	gstate->LinkEntity(self);

	RELIABLE_UNTRACK(self);
	trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent=self;
    tinfo.srcent=hook->owner;
	tinfo.fxflags		= TEF_PROJ_FX|TEF_FX_ONLY;
	tinfo.numframes		= TEF_ID_OF(TEF_PROJ_ID_KCORE)|TEF_PROJ_DIE;
	tinfo.modelindex	= gstate->ModelIndex( "models/e4/we_ice.sp2" );// particle type
	tinfo.modelindex2   = 0;
	tinfo.lightSize		= 1.0;							// alpha
	tinfo.altpos		= CVector(sizex,sizey,sizez);	// current size
	tinfo.altpos2.Zero();								// size dec...we use fractional
	tinfo.scale			= 0.005;						// fractional scale value
	tinfo.dstpos		= CVector(0.2,0.2,0.2);			// minimum size
	tinfo.altangle.Zero();								// angle
	tinfo.flags |= TEF_SRCINDEX|TEF_FXFLAGS|TEF_NUMFRAMES|TEF_LIGHTSIZE|TEF_SCALE|TEF_ALTPOS|TEF_MODELINDEX|TEF_MODELINDEX2|TEF_DSTPOS|TEF_ALTPOS2|TEF_ALTANGLE;
    
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// iceMelt()
//---------------------------------------------------------------------------
void iceMelt(userEntity_t *self)
{
	if ((self->hacks < gstate->time) || (iceCount > MAX_ICE_COUNT))
	{
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		iceCount--;
		return;
	}

	self->think = iceMelt;
	self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// iceThink()
//---------------------------------------------------------------------------
void iceThink(userEntity_t *self)
{
//    kcoreHook_t *hook = (kcoreHook_t *) self->userHook;// SCG[1/24/00]: not used
	if ( (gstate->PointContents(self->s.origin) & MASK_WATER ))
	{
		self->movetype = MOVETYPE_FLOAT;
		self->gravity  = 0.55f;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
	}

    float speed=self->velocity.Length();
	if ((speed < 10) || (gstate->time > self->hacks))
	{
		self->think = iceMelt;
		self->hacks = gstate->time + 2;

		trackInfo_t tinfo;
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent=self;
//		tinfo.srcent = hook->owner;
															// the ent is no longer a projectile tracker!!!!
		tinfo.modelindex2 = 1;								// start the melting
		tinfo.fxflags = TEF_SHRINKINGMODEL|TEF_FX_ONLY;
		tinfo.flags = TEF_FXFLAGS|TEF_MODELINDEX2;
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}
	self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
//	weapon_kineticore
//---------------------------------------------------------------------------
void weapon_kineticore(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_kineticore
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_kineticore (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 50, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_kineticore_register_func()
{
	gstate->RegisterFunc("kineticore_think",kineticore_think);
	gstate->RegisterFunc("kineticore_usethink",kineticore_usethink);
	gstate->RegisterFunc("kineticore_use",kineticore_use);
	gstate->RegisterFunc("kineticore_iceMelt",iceMelt);
	gstate->RegisterFunc("kineticore_iceThink",iceThink);
	gstate->RegisterFunc("kineticore_give",kineticore_give);
	gstate->RegisterFunc("kineticore_select",kineticore_select);
	gstate->RegisterFunc("kcoreRemoveThink",kcoreRemoveThink);
}
