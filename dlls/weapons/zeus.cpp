//---------------------------------------------------------------------------
// HAND OF ZEUS
//---------------------------------------------------------------------------
#include "weapons.h"
#include "zeus.h"

enum
{
    // model defs
    MDL_START,
    MDL_BOLT,

    // sound defs
    SND_START,
    SND_LIGHTNING1,
    SND_LIGHTNING2,
    SND_LIGHTNING3,

    // frame defs
    ANIM_START,
    ANIM_BOLT,
	ANIM_SHAKE,
	ANIM_EYE_CLOSE
};

// weapon info def
weaponInfo_t zeusWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_ZEUS),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_ZEUS_AMMO),
	// names, net names, icon
	"weapon_zeus",
	tongue_weapons[T_WEAPON_ZEUS],
	"ammo_zeus",
	tongue_weapons[T_WEAPON_ZEUS_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_zeuseye.dkm",
		"models/e2/a_zeus.dkm",
		"models/e2/wa_zeus.dkm",
		"models/e2/we_zeusbolt.dkm",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e2/we_zeusready.wav",
		"e2/we_zeusaway.wav",
		"e2/we_zeusshoot.wav",
		"e2/we_zeusamba.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		"global/e_lightninga.wav",
		"global/e_lightningb.wav",
		"global/e_lightningc.wav",
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
		ANIM_NONE,//MDL_BOLT,"bolta",0,0,
		MDL_WEAPON_STD, "shootb",0,0,
		MDL_WEAPON_STD, "shootc",0,0,
		0,
		NULL
	},

    // commands
	{ 
		wpcmds[WPCMDS_ZEUS][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_6,
		wpcmds[WPCMDS_ZEUS][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_6,
		NULL
	},

	weapon_give_6,
	weapon_select_6,
	command,
	use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(zeus_t),

	600,
	100,        //	ammo_max
	20,         //	ammo_per_use
	1,          //	ammo_display_divisor
	100,        //	initial ammo
	0,          //	speed
	2000.0,         //	range
	0,          //	damage
	0.25,           //	lifetime

	WFL_FORCE_SWITCH|WFL_PLAYER_ONLY,          //	flags

	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_ZEUS],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_ZEUS),
	ATTACKER_VICTIM,

	WEAPON_ZEUS,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&zeusWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *zeus_give(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// zeus_select()
//---------------------------------------------------------------------------
short zeus_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *command(struct userInventory_s *ptr, char *commandStr, void *data)
{
    if ( !stricmp(commandStr, "ambient"))         // if numBolts, don't do ambient
    {
    }
    else
        winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void use(userInventory_s *ptr, userEntity_t *self)
{
    if ( !weaponHasAmmo(self,true))
        return;
        
	playerHook_t *phook = (playerHook_t *)self->userHook;
	if (!phook)
		return;

//    winfoPlay(self, SND_SHOOT_STD, 1.0f);    
	winfoPlayAttn(self, SND_SHOOT_STD, 1.0f, 640, 1024);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,0.05);
    phook->fxFrameNum=-1;
    phook->fxFrameFunc=zeusStrike;

	// extend the animation finished time a tad to fix an exploit
	phook->attack_finished += 0.1;
}

short inTargetList(userEntity_t *entLeaderBolt, userEntity_t *target)
{
 	zHook_t	*zHook = (zHook_t *)entLeaderBolt->userHook;
	if (!entLeaderBolt || !entLeaderBolt->inuse || !zHook)
		return -1;		// don't want to add if something's broken.

	for (int i = 0; i < ZEUS_MAX_TARGETS; i++)
	{
		if (zHook->targets[i] == target)
			return 0;
	}
	
	return 1;
}

int EntIsInFOV( userEntity_t *self, userEntity_t *target )
{
	if (!self || !target || !self->userHook)
		return 0;

    playerHook_t *hook = (playerHook_t *)self->userHook;

	float fHalfFOV = hook->fov * 0.5f;
    
    CVector dir = target->s.origin - self->s.origin;
	dir.Normalize();
	CVector angleTowardClient;
	VectorToAngles( dir, angleTowardClient );
	
	float fYawDiff = fabs( self->s.angles.yaw - angleTowardClient.yaw );
	if( fYawDiff > 180 )
	{
		fYawDiff -= 360;
	}
	if( fYawDiff < -180 )
	{
		fYawDiff += 360;
	}
	if ( fYawDiff <= fHalfFOV || fYawDiff >= (360.0f - fHalfFOV) )
	{
		if (com->Visible(self,target))
			return TRUE;
		else
			return FALSE;
    }

	return	FALSE;
}

void boltRemove(userEntity_t *entBolt)
{
	RELIABLE_UNTRACK(entBolt);
//    com->untrackEntity(entBolt, NULL, MULTICAST_ALL);
    gstate->RemoveEntity(entBolt);
}

void boltZap(userEntity_t *entBolt)
{
	if (!entBolt || !EntIsAlive(entBolt->owner))
	{
		entBolt->remove(entBolt);
		return;
	}

	userEntity_t *entLeaderBolt = entBolt->teamchain;
	float zaps = 0;
	if (entLeaderBolt && entLeaderBolt->inuse && entLeaderBolt->userHook)
	{
 		zHook_t	*zHook = (zHook_t *)entLeaderBolt->userHook;
		zHook->bolts--;
		zaps = zHook->zaps++;
	}

	float damage = winfo->damage;
	if (zaps > 15)
		damage *= 0.25;
	else if (zaps > 10)
		damage *= 0.5;
	else if (zaps > 5)
		damage *= 0.75;

    CVector vecDir = entBolt->goalentity->s.origin - entBolt->teammaster->s.origin;
    vecDir.Normalize();

	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;

	WEAPON_HIT(entBolt->owner,1)
    com->Damage(entBolt->goalentity,
        entBolt,
        entBolt->owner,
        entBolt->goalentity->s.origin,
        vecDir,
        damage,
        DAMAGE_INERTIAL);

	entBolt->think = boltRemove;
	entBolt->nextthink = gstate->time + 0.5;
}

///////////////////////////////////////////////////////////////////////
//  void chainLightning(userEntity_t *entBolt)
//
//  Description:
//      After a bolt is spawned, it calls this as its think function.
//      Potential targets are considered, and where this chain hasn't
//      already touch them, it spawns new bolts. This chain reaction
//      continues until all possible targets have been struck, then
//      the whole chain is killed.
//
//  Parameters:
//      userEntity_t *entBolt       The bolt making the strike
//
//  Return:
//      Void
//
void chainLightning(userEntity_t *entBolt)
{
	if (!entBolt )
	{
		return;
	}

	if( !EntIsAlive(entBolt->owner))
	{
		entBolt->remove(entBolt);
		return;
	}

    userEntity_t    *entTarget;
	userEntity_t	*entLeaderBolt;
 	zHook_t			*zHook;
   
	entBolt->think = boltZap;
	entBolt->nextthink = gstate->time + 0.1;

	entLeaderBolt = entBolt->teamchain;
	if (entLeaderBolt && entLeaderBolt->userHook && entLeaderBolt->inuse)
	{
		zHook = (zHook_t *)entLeaderBolt->userHook;
		if (zHook->targetCount >= ZEUS_MAX_TARGETS)
		{
			return;
		}
	}
	else
	{
		return;
	}

    entTarget = NULL;
	userEntity_t *entNewBolt1 = NULL;
	userEntity_t *entNewBolt2 = NULL;
	float minDist = 8192;
	CVector dir;
	while ( (entTarget = com->FindRadius(entTarget, entBolt->s.origin, 0.25*winfo->range)) != NULL )
	{
		short inList = inTargetList(entLeaderBolt,entTarget);
		if ((inList == 0) || (inList == -1))
			continue;

		if (validateTarget(entBolt,entTarget,0,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_NEVER) && com->Visible(entBolt->goalentity,entTarget))
		{
			dir = entBolt->s.origin - entTarget->s.origin;
			if (dir.Length() < minDist)
			{
				entNewBolt2 = entNewBolt1;
				minDist = dir.Length();
				entNewBolt1 = entTarget;
			}
		}
	}
	if (rand() & 1)
	{
		if (entNewBolt1)
			spawnBolt(entBolt->goalentity, entNewBolt1, entBolt->owner, entLeaderBolt);
		if (entNewBolt2 && (entNewBolt2 != entNewBolt1))
			spawnBolt(entBolt->goalentity, entNewBolt2, entBolt->owner, entLeaderBolt);
	}
	else
	{
		if (entNewBolt1)
			spawnBolt(entBolt->goalentity, entNewBolt1, entBolt->owner, entLeaderBolt);
	}
}


///////////////////////////////////////////////////////////////////////
//  userEntity_t *spawnBolt(userEntity_t *entSource, 
//                          userEntity_t *entDest, 
//                          userEntity_t *entOwner)
//
//  Description:
//      Creates a lightning bolt, starting at entSource, ending at 
//      entDest. The think function is set to spawn more bolts to 
//      potential baddies in the circuit, until all potentiall baddies
//      in the chain path have been zapped. A track entity is created
//      for the actual bolt that renders on the client, and the server
//      creates a bolt entity to track the progress of the chain.
//
//  Return Value:
//      The bolt entity itself. If no bolt could be created (bad) then
//      NULL is returned.
userEntity_t *spawnBolt(userEntity_t *entSource, userEntity_t *entDest, userEntity_t *entOwner, userEntity_t *entLeaderBolt)
{
    userEntity_t    *entBolt;
    trackInfo_t     trackBolt;
    int             iBoltSound;
    CVector         vecFRU;

    entBolt = NULL;
    if(!entSource || !entDest || !entOwner || !entLeaderBolt->inuse)
        return NULL;

	zHook_t *zHook = (zHook_t *)entLeaderBolt->userHook;
	if (zHook)
	{
		zHook->targets[zHook->targetCount++] = entDest;
		zHook->bolts++;
	}
	else
	{
		return NULL;
	}

    if(entSource == entOwner)
        vecFRU.Set(8.0f, 2.5f, -0.5f);
    else
        vecFRU.Set(0,0,0);

    entBolt = gstate->SpawnEntity();
    entBolt->className = "zeus bolt";
	entBolt->flags |= FL_NOSAVE;
    entBolt->movetype = MOVETYPE_NONE;
    entBolt->solid = SOLID_NOT;
    entBolt->s.renderfx = RF_TRACKENT;//|RF_LIGHTNING;
    entBolt->s.effects = EF_GIB;
    entBolt->s.frame = 4;
    entBolt->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;
    entBolt->s.skinnum=0xa1a2a3a4;
    entBolt->s.alpha = 0.6;
    entBolt->s.origin = entSource->s.origin;
    entBolt->teammaster = entSource;    // teammaster is used to track the source entity for this hop in the chain
    entBolt->remove = boltRemove;
    entBolt->goalentity = entDest;
    entBolt->owner = entOwner;
    entBolt->EntityType = ZEUS_BOLT;
    entBolt->think = chainLightning;
    entBolt->nextthink = gstate->time + 0.1f;
	entBolt->s.modelindex = gstate->ModelIndex("models/global/e_flblue.sp2");
	entBolt->s.render_scale.Set(0.001,0.001,0.001);
    entBolt->teamchain = entLeaderBolt;

    gstate->LinkEntity(entBolt);

    // play the lightning bolt sound
    iBoltSound = (rand()%3);
    winfoPlay(entBolt, SND_LIGHTNING1 + iBoltSound, 1.0f);

    // msg client to track this entity
    trackBolt.flags=0;
    trackBolt.ent=entBolt;
    trackBolt.srcent=entSource;
    trackBolt.dstent=entDest;
    trackBolt.fru = vecFRU;
	trackBolt.altpos2.x = 4.0f;
	trackBolt.altpos2.y = 2.0f;
	trackBolt.altangle.Set(0.25,0.45,0.85); //Set color of bolt
	trackBolt.lightSize = 180;
	trackBolt.lightColor.Set(0,0,1);
    trackBolt.flags |= TEF_SRCINDEX|TEF_DSTINDEX|TEF_FRU|TEF_ALTPOS2|TEF_ALTANGLE|TEF_LIGHTSIZE|TEF_LIGHTCOLOR;
    trackBolt.renderfx = RF_LIGHTNING|RF_TRANSLUCENT;

	trackBolt.modelindex = gstate->ModelIndex("models/global/e_flblue.sp2");
	trackBolt.scale = 4;
	trackBolt.scale2 = 0.2;
	trackBolt.flags |= TEF_MODELINDEX|TEF_SCALE|TEF_SCALE2;

    com->trackEntity(&trackBolt,MULTICAST_ALL);



    return entBolt;
}

void zeusControllerThink(userEntity_t *self)
{
	zHook_t *zHook = (zHook_t *)self->userHook;
	if (zHook)
	{
		if (!zHook->bolts)
		{
			if (self->owner && EntIsAlive(self->owner))
				winfoAnimate(winfo,self->owner,ANIM_EYE_CLOSE,FRAME_ONCE|FRAME_WEAPON,0.05);
			gstate->RemoveEntity(self);
			return;
		}
	}
	else
	{
		if (self->owner && EntIsAlive(self->owner))
			winfoAnimate(winfo,self->owner,ANIM_EYE_CLOSE,FRAME_ONCE|FRAME_WEAPON,0.05);
		gstate->RemoveEntity(self);
		return;
	}
	self->nextthink = gstate->time + 0.1;
}

void zeusStrike(userEntity_t *entSelf)
{
    userEntity_t    *entTarget;
    userEntity_t    *entBolt;
    trackInfo_t     trackBolt;
    playerHook_t    *phook;
    CVector         vecDir;
    int             iPointContents;

	entTarget = NULL;
//    while(entTarget = selectTarget(entSelf, CVector(0.25, 0.25, 2000), SELECT_TARGET_PATH, entTarget))
	while ( (entTarget = com->FindRadius(entTarget, entSelf->s.origin, winfo->range)) != NULL )
	{
		if (EntIsInFOV(entSelf,entTarget) && 
			validateTarget(entSelf,entTarget,0,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_NEVER))
			break;
	}

	if (entTarget)
    {
		weaponUseAmmo (entSelf, true);
		WEAPON_FIRED(entSelf);
		weapon_PowerSound(entSelf);

 		// spawn controller
		userEntity_t *ent;
		ent = gstate->SpawnEntity();
		ent->flags |= FL_NOSAVE;
		ent->className = "zeus_controller";
		ent->s.modelindex=gstate->ModelIndex("models/e2/a_zeus.dkm");	
		ent->s.render_scale.Set(0.001,0.001,0.001);
		ent->owner = entSelf;
		ent->s.renderfx |= RF_TRACKENT;
		ent->s.origin = entSelf->s.origin;
		ent->think = zeusControllerThink;
		ent->nextthink = gstate->time + 0.1;
		gstate->LinkEntity(ent);
        ent->userHook = gstate->X_Malloc(sizeof(zHook_t), MEM_TAG_HOOK);
		zHook_t *zHook = (zHook_t *)ent->userHook;
		if (zHook)
		{
			memset(zHook,0,sizeof(zHook_t));
		}

		entBolt = spawnBolt(entSelf, entTarget, entSelf, ent);
        // zap the shit out of the first critter on the target list
        vecDir = entBolt->goalentity->s.origin - entBolt->teammaster->s.origin;
        vecDir.Normalize();
        if( (iPointContents = gstate->PointContents(entSelf->s.origin)) & MASK_WATER)
        {
            // zap the shit for radius damage
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

            int num = com->RadiusDamage(entBolt, entSelf, NULL, winfo->damage * 2, DAMAGE_EXPLOSION, 64.0);
			WEAPON_HIT(entSelf,num);
        }

		// delay the finish of this attack...
		playerHook_t *phook = (playerHook_t *)entSelf->userHook;
		phook->attack_finished = gstate->time + 5;
    }
	else if (deathmatch->value)
	{
		weaponUseAmmo (entSelf, true);
		WEAPON_FIRED(entSelf);
		weapon_PowerSound(entSelf);

        gstate->WriteByte(SVC_TEMP_ENTITY);
        gstate->WriteByte(TE_LIGHT);
        gstate->WritePosition(entSelf->s.origin);
        gstate->WritePosition(CVector(0,0,1));
        gstate->WriteFloat(350);
        gstate->MultiCast(entSelf->s.origin, MULTICAST_PVS);

		short iBoltSound = (rand()%3);
		winfoPlay(entSelf, SND_LIGHTNING1 + iBoltSound, 1.0f);
		CVector dir;
		AngleToVectors(entSelf->s.angles,dir);
		dir = -dir;
		// cek[1-24-00]: set the attackweaponANIM_EYE_CLOSE
		winfoAttackWeap;

		com->Damage(entSelf,entSelf,entSelf,zero_vector,dir,0.5 *winfo->damage,DAMAGE_INERTIAL);
		if (EntIsAlive(entSelf))
			winfoAnimate(winfo,entSelf,ANIM_EYE_CLOSE,FRAME_ONCE|FRAME_WEAPON,0.05);
	}
	else
	{
		if (EntIsAlive(entSelf))
			winfoAnimate(winfo,entSelf,ANIM_EYE_CLOSE,FRAME_ONCE|FRAME_WEAPON,0.05);
	}

    if( (phook=(playerHook_t *)entSelf->userHook) != NULL)
    {
        phook->fxFrameFunc=NULL;
    }
}

//-----------------------------------------------------------------------------
//	weapon_zeus
//-----------------------------------------------------------------------------
void weapon_zeus(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_zeus
//-----------------------------------------------------------------------------
void ammo_zeus(userEntity_t *self)
{
    winfoAmmoSetSpawn(winfo, self, 1, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_zeus_register_func()
{
	gstate->RegisterFunc("zeuschainLightning",chainLightning);
//	gstate->RegisterFunc("zeusmonitorChain",monitorChain);
	gstate->RegisterFunc("zeususe",use);
	gstate->RegisterFunc("zeusboltRemove",boltRemove);
	gstate->RegisterFunc("zeus_give",zeus_give);
	gstate->RegisterFunc("zeus_select",zeus_select);
}
