//---------------------------------------------------------------------------
// VENOMOUS
//---------------------------------------------------------------------------
#include "weapons.h"
#include "venomous.h"

enum
{
    // model defs
    MDL_START,
    MDL_SHOT,
	MDL_SPLAT,

    // sound defs
    SND_START,
    SND_SHOOT2,
    SND_SHOOT3,
    SND_VENOMHIT,
    SND_VENOMDRIP1,
    SND_VENOMDRIP2,
	SND_HIT_FLESH,
    // frame defs
    ANIM_START,
    ANIM_SHOOT2,
    ANIM_MELEE
};

// weapon info def
weaponInfo_t venomousWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_VENOMOUS),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_VENOMOUS_AMMO),
	// names, net names, icon
	"weapon_venomous",
	tongue_weapons[T_WEAPON_VENOMOUS],
	"ammo_venomous",
	tongue_weapons[T_WEAPON_VENOMOUS_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_venomous.dkm",
		"models/e2/a_venom.dkm",
		"models/e2/wa_venom.dkm",
		"models/e2/we_3dvenom.dkm",
		"models/e2/we_venstand.sp2",
		//       "models/e2/we_venappear.sp2",
		//       "models/e2/we_venstand.sp2",
		//       "models/e2/we_vendis.sp2",
		NULL
	},

	// sound files
	{
		"e2/we_venomamba.wav",
		"e2/we_venomready.wav",
		"e2/we_venomaway.wav",
		"e2/we_venomshoota.wav",
		"e2/we_venomamba.wav",
		"e2/we_venomambb.wav",
		"e2/we_venomambc.wav",
		"e2/we_venomshootb.wav",
		"e2/we_venomshootc.wav",
		"e2/we_venomhit.wav",
		"e2/we_venomdripa.wav",
		"e2/we_venomdripb.wav",
		"global/m_knifehitb.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoot",0,0,
		-1,"amba",0,0,
		-1,"amba",0,0,
		ANIM_NONE,      
		-1,"shoota",0,0,
		-1, "melee",0,0,
		ANIM_NONE,
		0,
		NULL
	},

	// commands
	{
		wpcmds[WPCMDS_VENOMOUS][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_2,
		wpcmds[WPCMDS_VENOMOUS][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_2,
		NULL
	},

	weapon_give_2,
	weapon_select_2,
	venomousCommand,
	venomousUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(venomous_t),

	400,
	75,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	10,     //	initial ammo
	600.0,  //	speed
	512.0,  //	range
	30.0,       //	damage
	5.0,        //	lifetime

	//WFL_LOOPAMBIENT_ANIM|WFL_CROSSHAIR,//|WFL_SELECT_EMPTY,            //|WFL_LOOPAMBIENT_SND,			// special flag
	WFL_LOOPAMBIENT_ANIM|WFL_CROSSHAIR|WFL_SELECT_EMPTY,

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_VENOMOUS],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_VENOMOUS),
	ATTACKER_VICTIM,


	WEAPON_VENEMOUS,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&venomousWeaponInfo;

//const int iStrikeDamage = 2;// SCG[1/24/00]: not used
//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *venomousGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// venomous_select()
//---------------------------------------------------------------------------
short venomousSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *venomousCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    userEntity_t    *venomous;
    playerHook_t    *pHook;
    venomous_t		*pVenomousInv;
	userEntity_t	*player;

    // Logic [7/15/99]: Venomous ambients are special, amba 
    // has the snake "feathers" waving gently all the time
    // but ambb (snakes poking each other) is played only once, then back to amba 
	// cek[11-26-99] Hey, I got a neat idea...lets actually use the right hooks!
    if(!stricmp(commandStr, "ambient"))
    {
		if ( (player = (userEntity_t *)data) && (player->client) &&
			 (venomous = (userEntity_t *)player->client->ps.weapon) &&
			 (pHook = (playerHook_t *)player->userHook) &&
			 (pVenomousInv = (venomous_t *)player->curWeapon) )
        {
            // if it's not playing the snake poking ambient and we make a 50% chance to try it, play it
            if((pVenomousInv->iAmbAnim != ANIM_AMBIENT2_STD) && (frand()>0.55))
            {
                // play ambb then amba
                winfoAnimate(winfo,player,ANIM_AMBIENT2_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
                pHook->ambient_time = gstate->time + winfoNumFrames(ANIM_AMBIENT2_STD)*.05;	// cek[11-26-99] add current time mebbe?
                pVenomousInv->iAmbAnim = ANIM_AMBIENT2_STD;
                
                winfoPlay(venomous, SND_AMBIENT_STD+(int)(frand()*3), 1.0f);
            }
			else 
			{
                winfoAnimate(winfo,player,ANIM_AMBIENT_STD,FRAME_LOOP|FRAME_WEAPON|FRAME_NODELAY,.05);
                pHook->ambient_time = gstate->time + winfoNumFrames(ANIM_AMBIENT_STD)*.05;
                pVenomousInv->iAmbAnim = ANIM_AMBIENT_STD;
            }
        }
    } 
	else
	{
        winfoGenericCommand(winfo,inv,commandStr,data);
    }

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void venomousUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    CVector start,end,dir;
    trace_t tr;
    venomousHook_t *hVenomous;
    short       sAltFire = 0;



    // hit anything?
    dir=self->client->v_angle;
    dir.AngleToVectors(forward,right,up);
    start=self->s.origin+up*4;
    end=start+forward*VEN_BITE_DIST;
    tr=gstate->TraceBox_q2(start,self->s.mins,self->s.maxs,end,self,MASK_SHOT);
    end=tr.endpos;

    if (!(gstate->PointContents(self->s.origin) & MASK_WATER) && (!tr.ent || tr.ent==gstate->FirstEntity()) && weaponHasAmmo(self,false) )
    {
        if((hVenomous = (venomousHook_t *)self->userHook) != NULL)
            sAltFire = hVenomous->sAltFire;

        weaponUseAmmo (self, false);

        if(sAltFire)
        {
            winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,0.05);
            phook->fxFrameNum=-1;//winfoLastFrame(ANIM_SHOOT_STD)-1;
        } else {
            winfoAnimate(winfo,self,ANIM_SHOOT2,FRAME_ONCE|FRAME_WEAPON,0.05);
            phook->fxFrameNum=-1;//winfoLastFrame(ANIM_SHOOT2)-1;
        }
        phook->fxFrameFunc=venomousShoot;
    }
    else                                               // short-range attack
    {
        winfoPlay(self,SND_SHOOT2 + rand() & 1,1.0f);
        winfoAnimate(winfo,self,ANIM_MELEE,FRAME_ONCE|FRAME_WEAPON,0.05);
        phook->fxFrameNum=-2;//winfoLastFrame(ANIM_MELEE)-1;
        phook->fxFrameFunc=venomousBite;
    }
}

//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void venomousShoot(userEntity_t *self)
{
    venomousHook_t *hVenomous;
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    CVector offset;
	if (phook)
	{
		phook->ambient_time = gstate->time + 0.5;
		phook->fxFrameFunc=NULL;
	}

    winfoPlay(self,SND_SHOOT_STD,1.0f);
    if(hVenomous=(venomousHook_t *)self->userHook)
    {
        if(hVenomous->sAltFire & 1)
        {
            hVenomous->sAltFire = 0;
            offset.Set( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1 + 10, winfo->fWeaponOffsetZ1 );
            venomousSpitPoison(self, offset,1,winfo->speed);
        } else {
            hVenomous->sAltFire = 1;

	        offset.Set( winfo->fWeaponOffsetX2, winfo->fWeaponOffsetY2 + 10, winfo->fWeaponOffsetZ2 );
            venomousSpitPoison(self, offset,2,winfo->speed);
        }
        invWeapon->lastFired=gstate->time;
    }
}

//---------------------------------------------------------------------------
// bite_func()
//---------------------------------------------------------------------------
void venomousBite(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    CVector start,end,dir;
    trace_t tr;

	if (phook)
	{
		phook->ambient_time = gstate->time;
		phook->fxFrameFunc=NULL;
	}

	WEAPON_FIRED(self);
    // hit anything?
    winfoPlay(self,SND_SHOOT_STD,1.0f);
    dir=self->client->v_angle;
    dir.AngleToVectors(forward,right,up);
    start=self->s.origin+up*4;
    end=start+forward*VEN_BITE_DIST;
    tr=gstate->TraceBox_q2(start,self->s.mins,self->s.maxs,end,self,MASK_SHOT);

    if ( tr.ent && tr.ent->takedamage != DAMAGE_NO )
    {
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		WEAPON_HIT(self,1);
        e2_com_Damage(tr.ent,self,self,tr.endpos,dir,1.3*winfo->damage,DAMAGE_INERTIAL);
		winfoPlay(self,SND_HIT_FLESH,1.0);
		// don't poison stuff that's not alive!
		if (tr.ent->deadflag == DEAD_NO)
		{
			// no damage?  no poison!
			if (gstate->damage_inflicted > 0)
				com->Poison(tr.ent, 0.1*winfo->damage, winfo->lifetime, 1);
		}
    }
}

void poisonTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags, bool full)
{
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_VENOMOUS,projflags,full);
	if (full)
	{
		tinfo.scale = 5;
		tinfo.scale2 = 0.55;
		tinfo.renderfx |= RF_TRANSLUCENT;
		tinfo.flags |= TEF_SCALE|TEF_SCALE2;
	}
}


//---------------------------------------------------------------------------
// blowPoison()
//---------------------------------------------------------------------------
userEntity_t *venomousSpitPoison(userEntity_t *self,CVector &pos,short num,float speed)
{
    venomousHook_t *hook;
    userEntity_t *ent;
    CVector angles,vel;

    WEAPON_FIRED(self);
    ent=weapon_spawn_projectile(self,pos,winfo->speed,-5,winfo->ammoName, FALSE);

	// dr. hacknslice hard at work.  You can't see the projectile if you fire it downwards becaue
	ent->gravity = 0.2;
    ent->movetype=MOVETYPE_BOUNCE;
    ent->clipmask=MASK_SHOT;
    ent->think=poisonThink;
    ent->nextthink=gstate->time+.1;
    ent->touch=poisonTouch;
    ent->s.effects=EF_POISON;
    ent->s.alpha=1;

//    ent->s.render_scale.Set(5.0f,5.0f,5.0f);
    ent->s.render_scale.Set(0.001f,0.001f,0.001f);
	ent->avelocity.Set(700*frand(),700*frand(),700*frand());
    winfoSetModel(ent,MDL_SHOT);
	ent->userHook = gstate->X_Malloc(sizeof(venomousHook_t),MEM_TAG_HOOK);
    hook=(venomousHook_t *)ent->userHook;
    hook->killtime=gstate->time+2*winfo->lifetime;
    hook->forward=forward;
    hook->mode=VEN_MODE_STAND;
    hook->count=num;
	hook->onGround = 0;
	hook->owner = self;
	gstate->SetSize( ent, -8, -8, -2, 8, 8, 14 );
	ent->elasticity = 0.6f;
	gstate->LinkEntity(ent);

	trackInfo_t tinfo;
	poisonTrack(tinfo,ent,hook->owner,TEF_PROJ_LAUNCH|TEF_PROJ_FLY,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
    
    return(ent);
}

void poison_water_die(userEntity_t *self)
{
	weaponUntrackProjectile(self);
}
//---------------------------------------------------------------------------
// think_poison()
//---------------------------------------------------------------------------
void poisonThink(userEntity_t *self)
{
    venomousHook_t *hook=(venomousHook_t *)self->userHook;
	if (!hook)
		return;
//    venomous_t *venomous=(venomous_t *)hook->owner->curWeapon;// SCG[1/24/00]: not used
    CVector pos,vel;

	
	if (gstate->PointContents(self->s.origin) & MASK_WATER)
	{
		self->touch = NULL;
		self->think = poison_water_die;
		self->nextthink = gstate->time + 0.1;

		trackInfo_t tinfo;
		poisonTrack(tinfo,self,hook->owner,TEF_PROJ_DIE|TEF_PROJ_WATER,false);
		com->trackEntity(&tinfo,MULTICAST_ALL);

        winfoPlay(self, SND_VENOMDRIP1 + rand() & 1, 1.0f);
		return;
	}

	if (!hook || (gstate->time > hook->killtime))
	{
		self->touch = NULL;
		weaponUntrackProjectile(self);
		return;
	}
	else if ((!hook->onGround) && ((self->velocity.Length() < 10) || self->groundEntity))
	{
		self->flags |= FL_EXPLOSIVE;
		hook->killtime = gstate->time + winfo->lifetime;
		hook->onGround = 1;
		self->owner = NULL;
		self->solid = SOLID_TRIGGER;
		gstate->LinkEntity(self);

		trackInfo_t tinfo;
		poisonTrack(tinfo,self,hook->owner,TEF_PROJ_FLY,false);
		tinfo.Long2 = 1;
		tinfo.flags |= TEF_LONG2;
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}

    self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// touch_poison()
//---------------------------------------------------------------------------
void poisonTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    venomousHook_t *hook=(venomousHook_t *)self->userHook;
	if (!hook)
		return;
    playerHook_t *ohook=(playerHook_t *)other->userHook;

    if (surf && (surf->flags & SURF_SKY))		
    {
		weaponUntrackProjectile(self);
        return;
    }
    // damage?

    if ( other->takedamage != DAMAGE_NO )
    {
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		WEAPON_HIT(hook->owner,1);
        e2_com_Damage(other,self,hook->owner,other->s.origin,zero_vector,winfo->damage,DAMAGE_INERTIAL);

        if	(ohook && (ohook->poison_time <= 0 ) && (other->deadflag == DEAD_NO))
        {
			// no damage?  no poison!
			if (gstate->damage_inflicted > 0)
				com->Poison(other, 0.1*winfo->damage, winfo->lifetime, 1);
        }

		self->touch = NULL;
        winfoPlay(self, SND_VENOMDRIP1 + rand() & 1, 1.0f);
		weaponUntrackProjectile(self);
//        self->remove(self);
    }
    else if ( plane && !hook->onGround)
    {
		trackInfo_t tinfo;
		poisonTrack(tinfo,self,hook->owner,TEF_PROJ_SPECIAL|TEF_PROJ_FLY,false);
		tinfo.modelindex = gstate->ModelIndex("models/e2/we_venstand.sp2");
		VectorToAngles(plane->normal,tinfo.altpos);
		tinfo.flags |= TEF_MODELINDEX|TEF_ALTPOS;

		com->trackEntity(&tinfo,MULTICAST_ALL);
        winfoPlay(self, SND_VENOMHIT, 1.0f);
        entBackstep(self, plane->normal, 1);
    }
}

//-----------------------------------------------------------------------------
//	weapon_venomous
//-----------------------------------------------------------------------------
void weapon_venomous(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_venomous
//-----------------------------------------------------------------------------
void ammo_venomous(userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 25, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_venomous_register_func()
{
	gstate->RegisterFunc("venomouspoisonThink",poisonThink);
	gstate->RegisterFunc("venomousUse",venomousUse);
	gstate->RegisterFunc("venomouspoisonTouch",poisonTouch);
	gstate->RegisterFunc("venomousGive",venomousGive);
	gstate->RegisterFunc("venomousSelect",venomousSelect);
}
