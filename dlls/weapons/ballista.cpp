//---------------------------------------------------------------------------
// BALLISTA
//---------------------------------------------------------------------------
#include "weapons.h"
#include "ballista.h"

enum
{
    // model defs
    MDL_START,
    MDL_BALLISTA,

    // sound defs
    SND_START,
	SND_FIRE1,
	SND_FIRE2,
	SND_FIRE3,
	SND_COCK,

    // special frame defs
    ANIM_START
};

// weapon info def
weaponInfo_t ballistaWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_BALLISTA),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_BALLISTA_AMMO),
	// names, net names, icon
	"weapon_ballista",
	tongue_weapons[T_WEAPON_BALLISTA],
	"ammo_ballista",
	tongue_weapons[T_WEAPON_BALLISTA_AMMO],
	"iconname",

    // model files
	{
		"models/e3/w_bal.dkm",
		"models/e3/a_bal.dkm",
		"models/e3/wa_bal.dkm",
		"models/e3/we_balprj.dkm",
		NULL
	},

    // sound files
	{
		SND_NOSOUND,
		"e3/we_ballistaready.wav", // ready
		"e3/we_ballistaaway.wav", // away
		"e3/we_ballistafirea.wav",//"null.wav", // fire
		"null.wav", // ambient
		SND_NOSOUND,
		SND_NOSOUND,
		"e3/we_ballistaflybya.wav",
		"e3/we_ballistaflybyb.wav",
		"e3/we_ballistaflybyc.wav",
		"e3/we_ballistacocka.wav",
		SND_NOSOUND
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		ANIM_NONE,
		ANIM_NONE,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_BALLISTA][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_4,
		wpcmds[WPCMDS_BALLISTA][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_4,
		NULL
	},

	weapon_give_4,
	weapon_select_4,
	ballistaCommand,
	ballistaUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(ballista_t),

	400,
	10,       //	ammo_max
	1,        //	ammo_per_use
	1,        //	ammo_display_divisor
	100,      //	initial ammo
	1600.0,   //	speed
	16000.0,  //	range
	40,       //	damage - direct hit
	10.0,     //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR,        //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_BALLISTA],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_BALLISTA),
	VICTIM_ATTACKER,

    WEAPON_BALLISTA,
    0,
    NULL
};

// local weapon info access
static  weaponInfo_t    *winfo=&ballistaWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *ballistaGive(userEntity_t *self, int ammoCount)
{
    return	weaponGive (self, winfo, ammoCount);
}

//---------------------------------------------------------------------------
// ballista_select()
//---------------------------------------------------------------------------
short ballistaSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *ballistaCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void ballistaUse(userInventory_s *ptr, userEntity_t *self)
{
    if ( !weaponHasAmmo(self,true) )
        return;

	weaponUseAmmo(self,true);

    playerHook_t *phook=(playerHook_t *)self->userHook;

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

    phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+3;
    phook->fxFrameFunc=ballistaShoot;
}

#define BAL_COCK_FRAME 46
void BallistaReload(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (phook)
		phook->fxFrameFunc = NULL;

	winfoPlay(self,SND_COCK,0.8);
}

void ballistaTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags,bool full)
{
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_BALLISTA,projflags,full);
	if (full)
	{
		tinfo.lightSize = 150;
		tinfo.lightColor.Set(0.8,0.4,0.2);
		tinfo.scale = 6.0;
		tinfo.flags |= TEF_SCALE|TEF_LIGHTSIZE|TEF_LIGHTCOLOR;
	}
}
//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void ballistaShoot(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    ballistaHook_t *hook;
    userEntity_t *ent;
    CVector dir,vel, vecKick;
    float speed;

    phook->fxFrameFunc=BallistaReload;
	phook->fxFrameNum = BAL_COCK_FRAME;

    speed=winfo->speed;
    if ( gstate->PointContents(self->s.origin) & MASK_WATER )
        speed *= .5;

    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
	ent=weapon_spawn_projectile(self, offset,speed,0,"projectile_ballista");
    ent->movetype    = MOVETYPE_FLYMISSILE;
    ent->touch       = ballistaTouch;
    ent->think       = ballistaThink;
    ent->nextthink   = gstate->time + .1;
    ent->flags       |= FL_NOPUSH;

    //ent->svflags &= ~SVF_SHOT;
    ent->clipmask=MASK_SHOT;//MASK_MONSTERSOLID;

	ent->userHook = gstate->X_Malloc(sizeof(ballistaHook_t),MEM_TAG_HOOK);
    hook=(ballistaHook_t *)ent->userHook;
    hook->killtime = gstate->time + winfo->lifetime;

    hook->velocity = ent->velocity;
	hook->angles = ent->s.angles;
	hook->skewered.ent = NULL;
	hook->skewered.old_flags = 0;
	hook->skewered.release_time = 0;
	hook->stuck = false;
	hook->unskeweredCount = 0;
	hook->lastSkewered = NULL;

	winfoPlay(self,SND_SHOOT_STD,1.0);
    winfoSetModel(ent,MDL_BALLISTA);
//    ent->s.render_scale.Set(6,6,6);
	ent->s.render_scale.Set(0.001,0.001,0.001);
    gstate->SetSize(ent,-4,-4,-2,4,4,2);
	gstate->LinkEntity(ent);

    CVector angle=self->client->v_angle;
    angle.AngleToVectors(forward,right,up);
    forward=forward*-8;
    vecKick.Set(2.5f, 0.0f, 0.0f);
    weapon_kick(self, vecKick, -400, 200, 200);
    invWeapon->lastFired=gstate->time;
	WEAPON_FIRED(self);

	short snd;
	snd = (short)(frand() * 2.95) + (short)SND_FIRE1;
	winfoPlayLooped(ent,snd,1.0);

	// set up the effects track entity
	trackInfo_t tinfo;
	ballistaTrack(tinfo,ent,self,TEF_PROJ_FLY|TEF_PROJ_LAUNCH,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

void ballistaThink(userEntity_t *self)
{
    ballistaHook_t *hook=(ballistaHook_t *)self->userHook;
	if (!hook)
		return;

	ballistaSkewer_t *skewered = &hook->skewered;

    // remove ballista and skewered entities
    if ( (gstate->time > hook->killtime))
    {
        removeBallista(self,hook->stuck,zero_vector);
        return;
    }

	if (hook->unskeweredCount >= 2)
	{
        removeBallista(self,true,zero_vector);
        return;
	}

	if (!hook->stuck)
	{
		self->velocity = hook->velocity;
		self->s.angles = hook->angles;
	}
	
	if (hook->unskeweredCount)
	{
		float dist = (self->s.origin - self->s.old_origin).Length();
		if (dist < 20)
			hook->stuck = true;
	}

	// do stuff to skewered entity here
	if (skewered->ent)
	{
		if ((gstate->time > skewered->release_time) || (!EntIsAlive(skewered->ent)))
		{
			ballista_unskewer(self);
		}
		else
		{
			CVector dir = -0.5*(skewered->ent->absmax + skewered->ent->absmin);
			dir += (self->s.origin + 20*skewered->normal);
			if (hook->stuck)
			{
				self->clipmask = 0;
				float mult = 0;
				if (dir.Length() > 50)
					mult = 500;

				dir.Normalize();
				skewered->ent->velocity = -mult*skewered->normal + 200*dir;

				if (!deathmatch->value)
				{
					// cek[1-24-00]: set the attackweapon
					winfoAttackWeap;

					hook->hits.AddHit(self->owner,skewered->ent);
					com->Damage (skewered->ent, self, self->owner, skewered->ent->s.origin, zero_vector, 1, DAMAGE_NONE);
				}
				gstate->LinkEntity(skewered->ent);
			}
			else
			{
				// do a short trace to see if there's another ent in the way
				AngleToVectors(self->s.angles,forward);
				trace_t tr;
                tr = gstate->TraceLine_q2(self->s.origin, self->s.origin + 40*forward, skewered->ent, MASK_SHOT);
				if ( tr.ent && tr.ent->takedamage)
				{
					removeBallista(self,true,zero_vector);
					return;
				}

				self->clipmask = MASK_SHOT;//MASK_MONSTERSOLID;
				skewered->ent->velocity = self->velocity;
				gstate->LinkEntity(self);
				gstate->LinkEntity(skewered->ent);
			}
		}
	}

	if (hook->stuck && (!skewered->ent || (skewered->ent && skewered->ent->takedamage == DAMAGE_NO)))
	{
		removeBallista(self,true,zero_vector);
		return;
	}

	self->think = ballistaThink;
	self->nextthink = gstate->time + 0.1;
}

void ballistaTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    ballistaHook_t *hook=(ballistaHook_t *)self->userHook;
	ballistaSkewer_t *skewered = &hook->skewered;

	if ( !(other->flags & (FL_CLIENT|FL_BOT|FL_MONSTER)) )
	{
		if ( surf && (surf->flags & SURF_SKY))
			removeBallista(self,false,zero_vector);			// make it go away
		else
		{
			// SCG[1/24/00]: added check for plane since both conditions make use of it.
			if( plane != NULL )
			{
				//   a stuck ent?          
				if( skewered->ent )
				{
					// check our angles to see if we should stickem
					AngleToVectors(self->s.angles,forward);
					forward = -forward;
					float dp = DotProduct(forward,plane->normal);
					if ((fabs(plane->normal.z) > 0.1) || (dp < 0.85))
					{
						removeBallista(self,true,plane->normal);
						return;
					}

					// we're not flying anymore...
					trackInfo_t tinfo;
					ballistaTrack(tinfo,self,self->owner,0,false);
					com->trackEntity(&tinfo,MULTICAST_ALL);

					self->velocity.Set(0,0,0);
					skewered->ent->velocity.Set(0,0,0);
					hook->stuck = true;
					skewered->normal = plane->normal;
					skewered->release_time = gstate->time + 2;
					hook->killtime = gstate->time + 2;
					self->movetype = MOVETYPE_NONE;	
					self->clipmask = 0;
					gstate->LinkEntity(self);
					gstate->LinkEntity(skewered->ent);
				}
				else
					removeBallista(self,true, plane->normal);			// blow up!	TODO: stick to wall for a bit!
			}
		}
		return;
	}

	// we hit monster/player!  see if we can stick it!
	if (skewered->ent != other)	// we hit something else!
	{
		if (!skewered->ent && other != (hook->lastSkewered))		// we don't have anything stuck yet
		{
			ballista_skewer(self,other);
		}
		else
		{
			removeBallista(self,true,plane->normal);
		}
	}
}

void ballista_skewer(userEntity_t *self, userEntity_t *other)
{
    ballistaHook_t *hook=(ballistaHook_t *)self->userHook;
	ballistaSkewer_t *skewered = &hook->skewered;

	// do some damage
	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;

	hook->hits.AddHit(self->owner,other);
	com->Damage (other, self, self->owner, self->s.origin, self->velocity, winfo->damage, DAMAGE_NONE);
	self->clipmask = 0;
	if (!hitMidsection(self,other))
		return;

	float del;
	// save the stuff
	skewered->ent = other;
	hook->lastSkewered = other;

	if (!stricmp(other->className,"monster_lycanthir") || !stricmp(other->className,"monster_buboid"))
	{
		del = 0.25;
	}
	else if (other->mass >= 200)
	{
		del = (300 / other->mass);
	}
	else
	{
		del = 1.0;
	}

	skewered->release_time = gstate->time + del;
	skewered->old_movetype = other->movetype;
	other->groundEntity = NULL;

	gstate->LinkEntity(self);
	gstate->LinkEntity(other);
}	

void ballista_unskewer(userEntity_t *self)
{
    ballistaHook_t *hook=(ballistaHook_t *)self->userHook;
	if (!hook)
		return;

	ballistaSkewer_t *skewered = &hook->skewered;
	hook->unskeweredCount++;

	// put the stuff back if necessary
	if (skewered->ent && EntIsAlive(skewered->ent))
	{
		userEntity_t *ent = skewered->ent;
		ent->velocity.Set(0,0,0);
		ent->movetype = skewered->old_movetype;
		gstate->LinkEntity(ent);
	}

	if (skewered->ent && IS_MONSTER_TYPE(skewered->ent,TYPE_LYCANTHIR))//!stricmp(skewered->ent->className,"monster_lycanthir"))			// friggin gross hack
		hook->unskeweredCount += 50;

	self->clipmask = MASK_SHOT;//MASK_MONSTERSOLID;
	skewered->ent = NULL;
	skewered->old_flags = 0;
	gstate->LinkEntity(self);
}

void removeBallista(userEntity_t *self, bool bExplode, CVector normal)
{
    ballistaHook_t *hook=(ballistaHook_t *)self->userHook;
	if (!hook)
	{
		weaponUntrackProjectile(self);
		return;
	}

	ballista_unskewer(self);
	hook->stuck = false;

	if (!bExplode)
	{
		weaponUntrackProjectile(self);
		return;
	}

	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;
    counted_com_RadiusDamage(self,self->owner,self,&hook->hits,winfo->damage * 0.5,DAMAGE_EXPLOSION|DAMAGE_INERTIAL, 128.0);

	if (normal.Length() < 0.01)
	{
		VectorToAngles(self->s.angles,normal);
		normal = -1.0*normal;
	}
	CVector scale;
	scale.Set(3.0+0.7*frand(),0.5+0.3*frand(),0.5+0.3*frand());
	int count = 10+frand()*10;


	if (!(deathmatch->value || coop->value))
	{
		debrisInfo_t di;
		memset(&di,0,sizeof(di));
		di.surf				= NULL;
		di.type				= DEBRIS_WOOD;	
		di.bRandom			= 1;			
		di.count			= count;
		di.org				= self->s.origin;
		di.dir				= normal;
		di.scale			= scale;
		di.scale_delta		= 0.25;					
		di.speed			= 650;
		di.spread			= 180;
		di.alpha			= 1.0;
		di.alpha_decay		= 0.25;
		di.gravity			= 1.0;
		di.delay			= 5;
		di.damage			= 0;
		di.owner			= NULL;

		di.bSound			= 1;						
		di.soundSource		= self;
		di.minAttn			= 256;
		di.maxAttn			= 1024;

		di.particles		= -1;		// no particles here!

		spawn_surface_debris(di);
	}

	// track the death effect!
	trackInfo_t tinfo;
	ballistaTrack(tinfo,self,self->owner,TEF_PROJ_DIE,false);
	com->trackEntity(&tinfo,MULTICAST_ALL);

	self->touch = NULL;
	self->think = ballistaDie;
	self->nextthink = gstate->time + 0.2;
}	

void ballistaDie(userEntity_t *self)
{
	weaponUntrackProjectile(self);
//	gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
// hitMidsection()
//---------------------------------------------------------------------------
short hitMidsection(userEntity_t *self,userEntity_t *other)
{
    CVector ofs = self->s.origin - other->s.origin;
    if ( ofs.x < 0 )
    {
        ofs.x = -ofs.x;
    }
    if ( ofs.y < 0 )
    {
        ofs.y = -ofs.y;
    }
    if ( ofs.z < 0 )
    {
        ofs.z = -ofs.z;
    }

    short hitmid = 0;
    if ( ofs.x <= (other->s.maxs.x-other->s.mins.x)*.35 )
    {
        hitmid++;
    }
    if ( ofs.y <= (other->s.maxs.y-other->s.mins.y)*.35 )
    {
        hitmid++;
    }
    if ( ofs.z <= (other->s.maxs.z-other->s.mins.z)*.35 )
    {
        hitmid++;
    }

    return(hitmid >= 2);
}

//-----------------------------------------------------------------------------
//	weapon_ballista
//-----------------------------------------------------------------------------
void weapon_ballista(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_ballista
//-----------------------------------------------------------------------------
void ammo_ballista (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 10, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_ballista_register_func()
{
	gstate->RegisterFunc("ballistaThink",ballistaThink);
	gstate->RegisterFunc("ballistaUse",ballistaUse);
	gstate->RegisterFunc("ballistaTouch",ballistaTouch);
	gstate->RegisterFunc("ballistaDie",ballistaDie);
	gstate->RegisterFunc("ballistaSelect",ballistaSelect);
	gstate->RegisterFunc("ballistaGive",ballistaGive);
}
