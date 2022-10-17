//---------------------------------------------------------------------------
// DISCUS OF DAEDALUS
//---------------------------------------------------------------------------
#include "weapons.h"
#include "discus.h"

enum
{
    // model defs
    MDL_START,
    MDL_DISC,
    MDL_TRAIL,

    // sound defs
    SND_START,
    SND_HIT1,
    SND_HIT2,
    SND_CATCH,
    SND_BLADESIN,
    SND_BLADESOUT,
    SND_DISCUSLOOP,
    SND_AMB2,
    SND_AMB3,
    SND_AMB4,

    // frame defs
    ANIM_START,
    ANIM_MELEE,
    ANIM_MELEE2
};

// weapon info def
weaponInfo_t discusWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_DISCUS),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_DISCUS_AMMO),
	// names, net names, icon
	"weapon_discus",
	tongue_weapons[T_WEAPON_DISCUS],
	"ammo_discus",
	tongue_weapons[T_WEAPON_DISCUS_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_discus.dkm",
		"models/e2/a_discus.dkm",
		MDL_NOMODEL,
		"models/e2/we_discus.dkm",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e2/we_discreadya.wav",
		"e2/we_discawaya.wav",
		"e2/we_discfire.wav",
		"e2/we_discawaya.wav",
		"e2/we_discambd.wav",
		SND_NOSOUND,
		"e2/we_dischit1.wav",
		"global/m_knifehitb.wav",//e2/we_dischit2.wav",
		"e2/we_disccatch.wav",
		"e2/we_discbladesin.wav",
		"e2/we_discbladesout.wav",
		"e2/we_discshoota.wav",
		"e2/we_discambb.wav",
		"e2/we_discambc.wav",
		"e2/we_discambd.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"readya",0,0,
		-1,"awaya",0,0,
		-1,"shootb", 0, 0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,
		-1,"shootc", 0, 0,
		-1,"shootd", 0, 0,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_DISCUS][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_1,
		wpcmds[WPCMDS_DISCUS][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_1,
		NULL
	},

	weapon_give_1,
	weapon_select_1,
	discusCommand,
	discusUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(discus_t),

	100,
	10,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	1,      //	initial ammo
	1000.0, //	speed
	2000.0, //	range
	35.0,   //	damage
	10.0,       //	lifetime

	WFL_CROSSHAIR,      //	flags

	WEAPONTYPE_GLOVE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_DISCUSS],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_DISCUSS),
	ATTACKER_VICTIM,

	WEAPON_DISCUS,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&discusWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *discusGive(userEntity_t *self, int ammoCount)
{
	// see how many we have
	if (!winfoCheckMaxCount(self, winfo))
	{
		return NULL;
	}

    discus_t *discus;

    discus=(discus_t *)weaponGive (self, winfo, ammoCount);

    if ( discus )
        discus->bladesout=true;

    return((userInventory_t *)discus);
}

//---------------------------------------------------------------------------
// discus_select()
//---------------------------------------------------------------------------
short discusSelect(userEntity_t *self)
{
	return weaponSelect(self,winfo);

    playerHook_t *phook=(playerHook_t *)self->userHook;
    userInventory_t *invItem;
    short rt;

    if ( !(invItem=gstate->InventoryFindItem(self->inventory,winfo->weaponName)) )
        return(WEAPON_UNAVAILABLE);

    if ( invItem==self->curWeapon )
    {
        return WEAPON_ACTIVE;
    }
/*
    else
    {
        rt=weaponSelect(self,winfo);
        if ( rt==WEAPON_ACTIVE )
            phook->weapon_next=discusSelectFinish;
    }
*/
    return(rt);
}

//---------------------------------------------------------------------------
// selectFinish()
//---------------------------------------------------------------------------
void discusSelectFinish(userEntity_t *self)
{
    weaponSelectFinish(self);

    if ( !weaponHasAmmo(self,false) )
    {
        short frame=winfoLastFrame(ANIM_SHOOT_STD);

        weapon_animate(self,frame,frame,.01);
    }
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *discusCommand (struct userInventory_s *ptr,char *commandStr,void *data)
{
	if (!stricmp(commandStr,"ambient"))
	{
		userEntity_t *self=(userEntity_t *)data;
		if (!self || !self->curWeapon || !weaponHasAmmo(self,false))
			return NULL;
	}

    return winfoGenericCommand(winfo,ptr,commandStr,data);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void discusUse (userInventory_t *ptr, userEntity_t *self)
{
    playerHook_t *phook;
//    discus_t *discus=(discus_t *)self->curWeapon;// SCG[1/24/00]: not used
    CVector         vecDir, vecForward, vecRight, vecUp, vecEnd;
    trace_t         tr;
    short           sAnim;

	if (!weaponHasAmmo(self,false))
		return;

    phook = (playerHook_t *) self->userHook;
	if (!phook)
		return;

	CVector vecOrigin = self->s.origin;
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
	vecOrigin += offset;
	if( self->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		vecOrigin.z -= 25;
	}

    vecDir = self->client->v_angle;
    vecDir.AngleToVectors(vecForward, vecRight, vecUp);
    vecEnd = vecOrigin;
    vecEnd += vecForward * 100;
    tr = gstate->TraceLine_q2(vecOrigin, vecEnd, self, MASK_SHOT);
	WEAPON_FIRED(self);
    if(tr.fraction < 1.0)
    {
        sAnim = ANIM_MELEE + (rand()&1);
        winfoAnimate(winfo, self, sAnim, FRAME_ONCE|FRAME_WEAPON, 0.05);
        phook->fxFrameNum = -2;//winfoLastFrame(sAnim)-2;
        winfoPlay(self, SND_AMB2, 1.0f);
        phook->fxFrameFunc = discusMelee;
    } 
    else
    {
        winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
//        phook->fired++;
        phook->fxFrameNum= -2;//winfoLastFrame(ANIM_SHOOT_STD)-2;
        phook->fxFrameFunc=discusShoot;
    }
}

void discusMelee(userEntity_t *self)
{
    CVector         vecDir, vecForward, vecRight, vecUp, vecEnd, vecOrigin;
    trace_t         tr;
    playerHook_t *phook;

	vecOrigin = self->s.origin;
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
	if( self->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		vecOrigin.z -= 25;
	}

	vecOrigin += offset;
    vecDir = self->client->v_angle;

    vecDir.AngleToVectors(vecForward, vecRight, vecUp);
    vecEnd = vecOrigin;
    vecEnd += vecForward * 120;//45;

    tr = gstate->TraceLine_q2(vecOrigin, vecEnd, self, MASK_SHOT);

	if ((self->flags & FL_BOT) && (self->enemy))
	{
		tr.fraction = 0.8;
		tr.endpos = self->enemy->s.origin;
		tr.ent = self->enemy;
	}

    if(tr.fraction < 1.0)
    {
        if(tr.ent && tr.ent->takedamage)
        {
            
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

			WEAPON_HIT(self,1);
			e2_com_Damage(tr.ent,self,self,vecOrigin,zero_vector,winfo->damage,DAMAGE_INERTIAL);

			if ((tr.ent->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)) && (Weapon_is_fleshy(tr.ent) & IS_FLESHY))
				winfoPlayAttn(self, SND_HIT2, 1.0f, 32.0f, 1024.0f);
			else
				winfoPlayAttn(self, SND_HIT1, 1.0f, 32.0f, 1024.0f);

			CVector Color;
			if(tr.ent->fragtype & FRAGTYPE_NOBLOOD)
			{
				Color.Set(1.0,1.0,1.0);
			}
			else
			{
				Color.Set(0.2,0.4,0.8);
			}
			vecDir.Negate();
			clientSparks(tr.endpos,vecDir,4, Color);
        }
		else
		{
			CVector Color;
			Color.Set(0.65,0.65,0.1);
			winfoPlayAttn(self, SND_HIT1, 1.0f, 32.0f, 1024.0f);
			vecDir.Negate();
			clientSparks(tr.endpos,vecDir,10, Color);
		}
    }
    if( (phook = (playerHook_t *) self->userHook) != NULL)
        phook->fxFrameFunc = NULL;


}
///////////////////////////////////////////////////////////////////////
//  void discusReady(userEntity_t *self)
//
//  Description:
//      Plays the "ready" animation
//
//  Parameters:
//      userEntity_t *self      player entity
//  
//  Return:
//      void
//
void discusReady(userEntity_t *self)
{
    playerHook_t *phook;

    if( (phook=(playerHook_t *)self->userHook))
    {
        phook->attack_finished=gstate->time+.1;
        phook->fxFrameFunc=NULL;
        if(weaponHasAmmo(self,false))
        {
            winfoAnimate(winfo,self,ANIM_AMBIENT_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
        }
    }
}
///////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void discusShoot(userEntity_t *self)
{
    discus_t *curWeap=(discus_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!curWeap || !phook)
		return;

    userEntity_t *discus;
    discusHook_t *hook;
    CVector temp;

    weaponUseAmmo(self,true);

	if (!phook)
		return;

    phook->fxFrameNum= -1;
    phook->fxFrameFunc=discusReady;

    // spawn discus
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    discus=weapon_spawn_projectile(self,offset,winfo->speed,0,"discus");
    discus->movetype=MOVETYPE_FLYMISSILE;
    discus->s.frame=!curWeap->bladesout;
    //discus->userHook = new discusHook_t;
	discus->userHook = gstate->X_Malloc(sizeof(discusHook_t),MEM_TAG_HOOK);
    hook=(discusHook_t *)discus->userHook;

    // setup forward vector
    temp = discus->velocity;
    temp.Normalize();
    hook->forward=temp;
    temp.VectorToAngles(discus->s.angles);
    
    // set funcs
    discus->touch=discusTouch;
    discus->think=discusThink;
    discus->nextthink=gstate->time+.1;
    discus->remove=discusRemove;
    discus->goalentity=phook->autoAim.ent;
    discus->s.renderfx = RF_INITTRAILS;
    discus->s.effects = EF_BEAMTRAIL;
    winfoSetModel(discus,MDL_DISC);
    gstate->SetSize(discus,-8,-8,-4,8,8,4);
    gstate->LinkEntity(discus);

    // set hook shit
    hook->owner=self;
    hook->droptime=gstate->time+5;
    hook->speed=winfo->speed;
    hook->target = discus->goalentity;
    hook->sSeekTarget = 1;
	hook->reflected = FALSE;

    discus->s.dist_min = 128.0f;
	discus->s.dist_max = 1024.0f;
	
	winfoPlayLooped(discus,SND_DISCUSLOOP,1.0f);
    
    curWeap->weapon.lastFired=gstate->time;
    winfoPlay(self,SND_SHOOT_STD,1.0f);

}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void discusThink(userEntity_t *self)
{
    discusHook_t    *hook=(discusHook_t *)self->userHook;
	if (!hook)
	{
		gstate->RemoveEntity(self);
		return;
	}
	
    CVector         vec, vecGoalDir, vecSelfDir, vecChangeDir, vecDelta;
    userEntity_t    *entNear = NULL;
    float           fChangeFrac;
	
    if(self->s.renderfx & RF_INITTRAILS)
        self->s.renderfx &= ~RF_INITTRAILS;
	
	if (--hook->clear < 0) hook->clear = 0;
    // in water?
	if ( gstate->PointContents(self->s.origin) & MASK_WATER )
    {
        hook->speed *= .75;
        if ( hook->speed < winfo->speed/8 )
        {
            dropDiscus(self);
            return;
        }
    } else if (hook->speed < winfo->speed) {
        hook->speed *= 1.25;
        if(hook->speed > winfo->speed)
            hook->speed = winfo->speed;
    }
    // time to drop?
    if ( (gstate->time >= hook->droptime) || (!EntIsAlive(self->owner)) )
    {
        dropDiscus(self);
        return;
    }
	
    vec=hook->forward*hook->speed;
    self->velocity = vec;
	
	if (hook->reflected)
	{
		CVector dir = (self->s.origin - self->owner->s.origin);
		if (dir.Length() < 100)
		{
			// turn looped sound off
			winfoPlayLooped(self,-1,0.0f);
            discusPickup(self, self->owner);
            return;
		}
	}

    if(hook->target && hook->sSeekTarget)
    {
		if ((sv_violence->value > 0) && !EntIsAlive(hook->target))
			hook->target = self->owner;//selectTarget(self, CVector(0.25, 0.25, 2000), SELECT_TARGET_PATH, NULL);
		
        if(!self->s.sound)
            winfoPlayLooped(self,SND_DISCUSLOOP,1.0f);
        if(! (self->s.effects & EF_BEAMTRAIL))
		{
			self->s.renderfx |= RF_INITTRAILS;
            self->s.effects |= EF_BEAMTRAIL;
		}
        if(hook->target == self->owner)
        {
            fChangeFrac = 1.0;
            vecChangeDir = hook->target->s.origin - self->s.origin;
            if(vecChangeDir.Length() < 100)
            {
				// turn looped sound off
				winfoPlayLooped(self,-1,0.0f);
                discusPickup(self, self->owner);
                return;
            }
        } 
		else 
		{
            fChangeFrac = 0.65;
        }
        vecGoalDir = turnToTarget(self, hook->target, fChangeFrac);
        self->velocity = vecGoalDir;
        vecDelta = self->velocity - vecGoalDir;
        vecGoalDir.Normalize();
        hook->forward = vecGoalDir;
        vecGoalDir.VectorToAngles(vecGoalDir);
        self->s.angles = vecGoalDir;
        self->s.angles.roll = (int)( atan2( vecDelta.z, vecDelta.y ) * ONEEIGHTY_OVER_PI );
        gstate->LinkEntity(self);
		
    } 
	else if (!hook->target && hook->sSeekTarget)
	{
        hook->target = selectTarget(self, CVector(0.25, 0.25, 2000), SELECT_TARGET_PATH, NULL);
		if ((hook->target != self->owner) && !validateTarget(self->owner,hook->target,0,TARGET_DM,TARGET_ALWAYS,TARGET_DM,TARGET_NEVER))
			hook->target = NULL;
    }
    self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void discusTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    // ISP:8-25-99
    if ( !plane || !surf )
    {
        return;
    }

    discusHook_t *hook=(discusHook_t *)self->userHook;
	if (!hook)
	{
		gstate->RemoveEntity(self);
		return;
	}

	if (other == self->owner)
	{
		// turn looped sound off
		winfoPlayLooped(self,-1,0.0f);
        discusPickup(self, self->owner);
        return;
	}

    CVector normal,vec;
    float dp;

    if(hook->target && hook->sSeekTarget)
        hook->sSeekTarget = 0;  // bounce without seeking
    else
        hook->sSeekTarget = 1;

    hook->target = self->owner;
    
	short backToPlayer = 0;
    // do damage
    if ( other->takedamage && !hook->clear)
    {
		hook->clear = 3;

		if ((other->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)) && (Weapon_is_fleshy(other) & IS_FLESHY) )
			winfoPlayAttn(self, SND_HIT2, 1.0f, 32.0f, 1024.0f);
		else
			winfoPlayAttn(self, SND_HIT1, 1.0f, 32.0f, 1024.0f);
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		short doDamage = TRUE;
		if (!deathmatch->value && hook->reflected && (other->flags & (FL_CLIENT|FL_BOT)))
		{
			doDamage = FALSE;
		}

		if (doDamage)
		{
			WEAPON_HIT(hook->owner,1);
			e2_com_Damage(other,self,hook->owner,self->s.origin,zero_vector,winfo->damage,DAMAGE_INERTIAL);
		}

		if ((other->solid == SOLID_BSP) || (other->deadflag != DEAD_NO))  // target died...send the discus back
		{
			backToPlayer = 1;
		}	
    }
    else
    {
        winfoPlayAttn(self, SND_HIT1, 1.0f, 32.0f, 1024.0f);
        clientSparks(self->s.origin,plane->normal, 1, CVector(1.0f, 1.0f, 0.4f));
    }

	if (backToPlayer)
	{
		// calc reflection vector
		dp = hook->forward.Length();
		vec = - self->s.origin + self->owner->s.origin;
		vec.Normalize();
		hook->sSeekTarget = 1;
		hook->target = self->owner;
		self->s.origin += 15*vec;
		gstate->LinkEntity(self);
	}
	else
	{
		// calc reflection vector
		if (other->flags & (FL_CLIENT|FL_MONSTER|FL_BOT))
		{
			// see if we have to go through this target to get to the owner
			trace_t tr;
			tr = gstate->TraceBox_q2 (self->s.origin,self->s.mins,self->s.maxs, self->owner->s.origin, NULL, MASK_SHOT);
			if (tr.ent && (tr.ent == other))
			{
				hook->sSeekTarget = 0;
				hook->target = NULL;
				// randomize reflection a bit so the back and forth thing goes away
				CVector n = plane->normal;
				n.x += (rand() & 1 ? 1 : -1) * (0.1 + 0.2*frand());
				n.y += (rand() & 1 ? 1 : -1) * (0.1 + 0.2*frand());
				n.Normalize();

				dp=hook->forward.DotProduct(n);//hook->forward.x*n.x + forward.y*n.y + forward.z*n.z;
				vec=hook->forward-2*n*dp;
			}
			else
			{
				vec = - self->s.origin + self->owner->s.origin;
				vec.Normalize();
			}
		}
		else
		{
			dp=hook->forward.x*plane->normal[0] + hook->forward.y*plane->normal[1] + hook->forward.z*plane->normal[2];
			normal=(dp)*plane->normal;
			vec=hook->forward-2*normal;
		}
	}

    // save new forward and change velocity
    hook->forward=vec;
	hook->reflected = TRUE;

    // face direction of movement
    VectorToAngles(hook->forward, vec);
    self->s.angles = vec;
}

void dropDiscusPickup(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	discusPickup(self,other);
}
//---------------------------------------------------------------------------
// dropDiscus()
//---------------------------------------------------------------------------
void dropDiscus(userEntity_t *self)
{
	if (EntIsAlive(self->owner) && (self->owner->flags & FL_BOT))
	{
		catchDiscus(self,self->owner);
		self->remove(self);
		return;
	}	

    if ( unlimitedAmmo() )
	{
		self->remove(self);
		return;
	}

    discusHook_t *hook=(discusHook_t *)self->userHook;
	if (!hook)
		return;

    hook->droptime=gstate->time+5;
    hook->target = self->owner;
    self->movetype=MOVETYPE_TOSS;
	self->velocity.z = 60;
    self->touch=(touch_t)dropDiscusPickup;
    self->clipmask=MASK_SOLID;
    self->elasticity=.6;
    self->s.effects &= ~EF_BEAMTRAIL;
    self->netname=winfo->weaponNetName;//"Discus of Daedalus";
    self->className=winfo->weaponName;"weapon_discus";
//    self->client->ps.gunindex=gstate->ModelIndex("models/e2/w_discus.dkm");

    winfoSetModel(self,MDL_DISC);
    entAnimate(self,1,1,FRAME_ONCE,.05);

    self->flags|=FL_ITEM;
	self->svflags |= SVF_ITEM;
    self->think=spinDiscus;
    self->nextthink=gstate->time+.1;
    self->s.sound = 0;
    self->s.volume = 0;

    gstate->LinkEntity(self);
}

//---------------------------------------------------------------------------
// discus_pickup()
//---------------------------------------------------------------------------
void discusPickup(userEntity_t *self, userEntity_t *other)
{
//    playerHook_t *ohook=(playerHook_t *)other->userHook;// SCG[1/24/00]: not used
//    discus_t *weapon=(discus_t *)other->curWeapon;// SCG[1/24/00]: not used

    if ( !com->ValidTouch(self, other) )
        return;
    self->s.effects &= ~EF_BEAMTRAIL;
    gstate->LinkEntity(self);

    catchDiscus(self,other);

    self->remove(self);
}

//---------------------------------------------------------------------------
// spinDiscus()
//---------------------------------------------------------------------------
void spinDiscus(userEntity_t *self)
{
    discusHook_t    *hook=(discusHook_t *)self->userHook;
    trace_t         tr;
    CVector         vecGoal, vecForward, vecRight, vecUp;
    // ok, owner is probably out of site. wait here until the owner is
    // in sight again, then head that way!
    if(hook)
    {
        if(hook->target != self->owner)
        {
            hook->target = self->owner;
        }
		if (!EntIsAlive(hook->target))
		{
			self->remove(self);
			return;
		}

        hook->sSeekTarget = 1;
		vecGoal = hook->target->s.origin - self->s.origin;
		vecGoal.Multiply(2);
        tr = gstate->TraceLine_q2( self->s.origin, self->s.origin + vecGoal, self, MASK_SOLID );
//        tr = gstate->TraceLine_q2( self->s.origin, hook->target->s.origin, self, MASK_SOLID );
//        if ( tr.ent && (tr.fraction >= 1.0) && !tr.startsolid && !tr.allsolid )
		if (com->Visible(self,hook->owner))
        {

            if(self->flags)
            {
                // master! I see you!!
                 vecGoal = hook->target->s.origin - self->s.origin;
                vecGoal.Normalize();
                hook->forward=vecGoal;
                hook->speed = winfo->speed;
                self->flags = 0;
                self->movetype = MOVETYPE_FLYMISSILE;
                self->clipmask = MASK_SHOT;
                self->svflags = SVF_SHOT;
                self->solid = SOLID_BBOX;
                self->s.angles = vecGoal;
                // vecGoal should reflect new velocity
                vecGoal = vecGoal * winfo->speed;
                self->velocity = vecGoal;
                self->think = discusThink;
                self->nextthink = gstate->time + 0.1;
                // set funcs
                gstate->LinkEntity(self);
            }
        }

		if ( hook->droptime && gstate->time >= hook->droptime )
		{
			hook->droptime=0;
		}
    }

// SCG[1/24/00]: moved inside above if statment to make sure hook is valid...
//    if ( hook->droptime && gstate->time >= hook->droptime )
//    {
//        hook->droptime=0;
//    }
    self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// catchDiscus()
//---------------------------------------------------------------------------
void catchDiscus(userEntity_t *self, userEntity_t *other)
{
    playerHook_t *phook=(playerHook_t *)other->userHook;
    discus_t *discus=(discus_t *)other->curWeapon;
    int amount;

    winfoPlay(other,SND_CATCH,1.0f);

    self->s.effects &= ~EF_BEAMTRAIL;
    gstate->LinkEntity(self);

    amount=1;
	// don't give more more discusseses if unlimited ammo is on and we're a client
	if (!((other->flags & FL_CLIENT) && unlimitedAmmo()))
		weaponGive (other, winfo, amount);

	if ( (other->flags & FL_CLIENT) && (!discus || (discus && !stricmp(discus->weapon.name,"weapon_discus"))))
		discusSelect(other);
	else
		return;

    winfoAnimate(winfo,other,ANIM_AMBIENT_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
	if (phook)
		phook->attack_finished=gstate->time+.1;
}

//-----------------------------------------------------------------------------
//	discusRemove
//-----------------------------------------------------------------------------
void discusRemove(userEntity_t *self)
{
    // give trail a chance to fade out
    self->s.effects &= ~EF_BEAMTRAIL;
    self->touch=NULL;
    self->s.render_scale.Set(.01,.01,.01);
    self->velocity = zero_vector;
    self->solid=SOLID_NOT;
    gstate->RemoveEntity (self);

}

//-----------------------------------------------------------------------------
//	weapon_discus
//-----------------------------------------------------------------------------
void weapon_discus(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_discus_register_func()
{
	gstate->RegisterFunc("discusThink",discusThink);
	gstate->RegisterFunc("spinDiscus",spinDiscus);
	gstate->RegisterFunc("discusUse",discusUse);
	gstate->RegisterFunc("discusRemove",discusRemove);
	gstate->RegisterFunc("discusTouch",discusTouch);
	gstate->RegisterFunc("discusPickup",discusPickup);
	gstate->RegisterFunc("discusGive",discusGive);
	gstate->RegisterFunc("discusSelect",discusSelect);
	gstate->RegisterFunc("dropDiscusPickup",dropDiscusPickup);
}
