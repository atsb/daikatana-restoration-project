//---------------------------------------------------------------------------
// NIGHTMARE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "nightmare.h"

enum
{
    // model defs
    MDL_START,
    MDL_REAPER,
    MDL_PENTAGRAM,
    MDL_FIRE,

    // sound defs
    SND_START,
    SND_ATTACK,
    SND_CHANT,
    SND_WIND,
	SND_RUMBLE,
	SND_BOOM,

    // frame defs
    ANIM_START,
    ANIM_REAPER,
    ANIM_PENTAGRAM,
    ANIM_FIRE
};

// weapon info def
weaponInfo_t nightmareWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NIGHTMARE),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NIGHTMARE_AMMO),
    // names, net names, icon
    "weapon_nightmare",
    tongue_weapons[T_WEAPON_NIGHTMARE],
    "ammo_gibs",
    tongue_weapons[T_WEAPON_NIGHTMARE_AMMO],
    "iconname",

	// model files
	{"models/e3/w_nmare.dkm",
	 "models/e3/a_nmare.dkm",
	 MDL_NOMODEL,
	 "models/e3/we_nnreaper.dkm",
	 "models/e3/we_nnpent.dkm",
     "models/global/e2_firea.sp2",
	 NULL},

    // sound files
    {SND_NOSOUND,
	 "e3/we_nharreready.wav",
	 "e3/we_nharreaway.wav",
	 "e3/we_reaperappear2.wav",
	 SND_NOSOUND,
	 SND_NOSOUND,
	 SND_NOSOUND,
	 "e3/we_reaperattack2.wav",
	 "e3/we_chant5.wav",
	 "e3/we_nharrewind.wav",
	 "global/earthquake_b.wav",
	 "global/e_explodec.wav",
	 NULL},

    // frame names
    {MDL_WEAPON_STD,"ready",0,0,
	 -1,"away",0,0,
	 -1,"shoot",0,0,
	 -1,"amba",0,0,
	 -1,"ambb",0,0,
	 -1,"ambc",0,0,
	 MDL_REAPER,"ataka",0,0,
	 MDL_PENTAGRAM,"pent",0,0,
     MDL_FIRE,"sp2",0,0,
	 0,NULL},

    // commands
    {
		wpcmds[WPCMDS_NIGHTMARE][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_6,
		wpcmds[WPCMDS_NIGHTMARE][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_6,
		NULL
    },

    weapon_give_6,
    weapon_select_6,
    nightmareCommand,
    nightmareUse,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(weapon_t),

    600,
    100,      //	ammo_max
    20,       //	ammo_per_use
    1,        //	ammo_display_divisor
    60,       //	initial ammo
    0,        //	speed
    0,        //	range
    0,        //	damage
    0.0,      //	lifetime

    WFL_SPECIAL|WFL_PLAYER_ONLY,

	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_NHARRE],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_NHARRE),
	VICTIM_ATTACKER,

    WEAPON_NIGHTMARE,
    0,
    NULL
};

// local weapon info access
static weaponInfo_t *winfo=&nightmareWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *nightmareGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// nightmare_select()
//---------------------------------------------------------------------------
short nightmareSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *nightmareCommand(struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void nightmareUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;

    if ( !weaponHasAmmo(self, true) )
        return;

	if (phook->attack_finished > gstate->time)
		return;

	phook->attack_finished = gstate->time + 90;		// make sure we have time to do stuff.
    weaponUseAmmo (self, false);

	WEAPON_FIRED(self);
	weapon_PowerSound(self);
	nightmareShootFunc(self);
}

//---------------------------------------------------------------------------
// nightmareShootFunc()
//---------------------------------------------------------------------------
void nightmareShootFunc(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;

	// the Nightmare controller does all the work
	nmControllerHook_t *hook;
	userEntity_t *nmController;
    nmController = gstate->SpawnEntity();
	nmController->flags |= FL_NOSAVE;
	nmController->className = "nightmare_controller";
    nmController->owner = self;
    nmController->s.renderfx |= RF_TRACKENT;
    nmController->s.origin = self->s.origin;
	nmController->think = nightmareControllerTracePent;
	nmController->remove = nightmareControllerRemove;
	nmController->nextthink = gstate->time + 0.1;

    nmController->userHook = gstate->X_Malloc(sizeof(nmControllerHook_t),MEM_TAG_HOOK);
    hook = (nmControllerHook_t *) nmController->userHook;
	memset(hook,0,sizeof(hook));

	gstate->LinkEntity(nmController);
	
	// trace out a pentagram in the shooter's view
	tracePentagram(&hook->shooter,self, self, nmController);
	hook->curDeath = 0;
	hook->reaped = 0;

	// set up the animation and start the attack
	winfoPlay(self, SND_CHANT, 1.0);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
	phook->attack_finished = gstate->time + 90;		// make sure we have time to do stuff.
    phook->fxFrameNum=-1;
    phook->fxFrameFunc=NULL;
}

void nightmareRemoveTarget(markedEnt_t *me, freezeEntity_t *frozen)
{
	if (!me || !frozen)
		return;

	if (me->ent && (me->ent->flags & FL_FORCEANGLES))
	{
		unfreezeEntity(me->ent,frozen);
	}
	me->ent = NULL;
	me->pent = NULL;
}
//---------------------------------------------------------------------------
// nightmareControllerRemove()
//---------------------------------------------------------------------------
void nightmareControllerRemove(userEntity_t *self)
{
	// remove all of the spawned pentagrams
    nmControllerHook_t *hook = (nmControllerHook_t *) self->userHook;
	if (!hook)
		return;

	markedEnt_t *me;

	me = &hook->shooter;
//	me->ent = NULL;
//	me->pent = NULL;
	nightmareRemoveTarget(me, &hook->frozen);

	for (int i = 0; i < NIGHTMARE_MAX_ATTACKS; i++)
	{
		me = &hook->marked[i];
		nightmareRemoveTarget(me,&hook->frozen);
//		me->ent = NULL;
//		me->pent = NULL;
	}
	userEntity_t *player = self->owner;
	playerHook_t *plHook = (playerHook_t *)player->userHook;
	plHook->attack_finished = gstate->time;
	gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
// nightmareControllerTracePent()
//---------------------------------------------------------------------------
void nightmareControllerTracePent(userEntity_t *self)
{
	// basically wait here until the shooter dies or the animation is done
	userEntity_t *player = self->owner;
	userEntity_t *went=(userEntity_t *)player->client->ps.weapon;


    if (player->deadflag != DEAD_NO) 
	{
		gstate->RemoveEntity(self);
		return;
	}
	if (went->s.frame < (winfoLastFrame(ANIM_SHOOT_STD) - 2))
    {
		self->nextthink = gstate->time + 0.1;
		return;
	}

	self->think = nightmareControllerSearch;
	self->nextthink = gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// nightmareControllerSearch()
//---------------------------------------------------------------------------
void nightmareControllerSearch(userEntity_t *self)
{
	userEntity_t *player = self->owner;
    nmControllerHook_t *hook = (nmControllerHook_t *) self->userHook;
	pentHook_t *phook;

	if (!hook)
		return;

	// populate the death list
	int count(0);
	short attackPlayer = 1;
	userEntity_t *ent=gstate->FirstEntity();
	while ((ent = com->FindRadius(ent, self->s.origin, winfo->range)) != NULL )
	{
		if (validateTarget(player,ent,1,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_NEVER))
		{	
			attackPlayer = 0;
			if(stricmp(ent->className,"monster_garroth"))		// ignore garroth!
			{
				tracePentagram(&hook->marked[count++],ent,player,self);
				if (hook->marked && hook->marked->pent && hook->marked->pent->userHook)
				{
					phook = (pentHook_t *)hook->marked->pent->userHook;
					phook->blood = 1;
				}
			}
		}
	}

	// if you don't see anyone, YOU die!
	if (attackPlayer)
	{
		// add player to the death list (keep the pentagram around... do blood or something
		hook->marked[0].ent = player;
		hook->marked[0].pent = hook->shooter.pent;
		hook->shooter.ent = NULL;
		hook->shooter.pent = NULL;
		if (hook->marked[0].pent && hook->marked[0].pent->userHook)
		{
			phook = (pentHook_t *)hook->marked[0].pent->userHook;
			phook->blood = 1;
			phook->fadeTime = gstate->time + 3.0;
		}
	}
	else if (hook->shooter.pent && hook->shooter.pent->userHook)	// glow!
	{
		phook = (pentHook_t *)hook->shooter.pent->userHook;
		phook->glow = 1;
		phook->fadeTime = gstate->time + 1.0;
	}

	// stop everything if the player's dead or the reaper has done his badness!
    if (player->deadflag != DEAD_NO) 
	{
		self->remove(self);
		return;
	}

	self->think = nightmareControllerBeatDown;
	float delay;
	if (deathmatch->value)
		delay = 1.5;
	else
		delay = 0.5;
	self->nextthink = gstate->time + delay;		// give some time to kill the player before mayhem starts
}

//---------------------------------------------------------------------------
// nightmareControllerBeatDown()
//---------------------------------------------------------------------------
void nightmareControllerBeatDown(userEntity_t *self)
{
	timeToDie(self);
	self->think = nightmareControllerUpkeep;
	self->nextthink = gstate->time + 0.05;
}

//---------------------------------------------------------------------------
// nightmareControllerUpkeep()
//---------------------------------------------------------------------------
void nightmareControllerUpkeep(userEntity_t *self)
{
	userEntity_t *player = self->owner;
    nmControllerHook_t *hook = (nmControllerHook_t *) self->userHook;
	if (!player || !hook)
		return;

	markedEnt_t *me;
	// service the death list
	if (self->owner->deadflag != DEAD_NO)
	{
		self->remove(self);
		return;
	}

	for (int i = hook->curDeath; i < NIGHTMARE_MAX_ATTACKS; i++)
	{
		me = &hook->marked[i];
		if ((me->ent == NULL) || (me->ent->deadflag != DEAD_NO))
		{
			nightmareRemoveTarget(me,&hook->frozen);
/*			if (me->ent && (me->ent->flags & FL_FORCEANGLES))
			{
				unfreezeEntity(me->ent,&hook->frozen);
			}
			me->ent = NULL;
			me->pent = NULL;*/
		}
	}

	self->nextthink = gstate->time + 0.05;
}

//---------------------------------------------------------------------------
// tracePentagram()
//---------------------------------------------------------------------------
void tracePentagram(markedEnt_t *markedEnt, userEntity_t *self, userEntity_t *shooter, userEntity_t *controller)
{
	markedEnt->ent = self;
	markedEnt->pent = NULL;
	if (self->flags & FL_CLIENT)
	{
		userEntity_t *pent;
		pentHook_t *phook;
		// setup weapon entity
		pent=gstate->SpawnEntity();
		pent->flags |= FL_NOSAVE;
		pent->owner=self;
		pent->movetype=MOVETYPE_FLY;
		pent->solid=SOLID_NOT;
		pent->className="nightmare_pentagram";
		pent->s.effects=256;
		pent->s.renderfx=RF_TRANSLUCENT|RF_NODRAW|RF_TRACK_REFDEF;
		pent->s.alpha=.8;
		pent->curWeapon=self->curWeapon;
		pent->s.origin = self->s.origin;

		// set up the hook
		pent->userHook = gstate->X_Malloc(sizeof(pentHook_t),MEM_TAG_HOOK);
		phook = (pentHook_t *) pent->userHook;
		phook->shooter = shooter;
		phook->controller = controller;
		phook->killPent = 0;
		phook->glow = 0;
		phook->blood = 0;
		phook->fadeTime = gstate->time + 10;
		phook->roll = 0;

		winfoSetModel(pent,MDL_PENTAGRAM);
		float speed;
		if (self == shooter)
		{
			speed = 0.05;
			playerHook_t *hook;
			if (hook = (playerHook_t *)self->userHook)
			{
				if (hook->attack_boost)
				{
					float add = (hook->attack_boost == 1) ? 1.5 : 1.0;	// make it so one boost has an effect...
					speed /= ((hook->attack_boost+add) * 0.5);
				}
			}
		}
		else
			speed = 0.015;
		winfoAnimate(winfo,pent,ANIM_PENTAGRAM,FRAME_ONCE,speed);

		pent->think=doPentagram;
		pent->nextthink=gstate->time+.1;
		pent->s.render_scale.Set(1.6,1.6,1.6);
		pent->s.angles.Zero();
		gstate->LinkEntity(pent);
		markedEnt->pent = pent;
	}
}

#define SHOOTER_INVALID(ent)	(!ent || (ent->deadflag |= DEAD_NO) || (!ent->curWeapon) || stricmp(ent->curWeapon->name,"weapon_nightmare"))
//---------------------------------------------------------------------------
// doPentagram()
//---------------------------------------------------------------------------
void doPentagram(userEntity_t *self)
{
	userEntity_t *player = self->owner;
    pentHook_t *phook = (pentHook_t *) self->userHook;
	if (!player || !phook)
		return;

	// shut it all down if the player dies or if we just need to kill the pent or if the shooter dies
//    if ( (!player) || (!phook)  || (!phook->shooter)|| (player->deadflag != DEAD_NO) || (phook->killPent) || (phook->shooter->deadflag |= DEAD_NO) || (!CHECK_WEAPON(phook->shooter)) || (self->s.alpha < 0.1))
	if ( (!player) || (!phook) || (self->s.alpha < 0.1) || SHOOTER_INVALID(phook->shooter))
    {
		RELIABLE_UNTRACK(self);
		if (phook && phook->controller)
		{
			// have to clear this entry out of the 
			nmControllerHook_t *nmhook = (nmControllerHook_t *)phook->controller->userHook;
			if (nmhook)
			{
				for (int i = 0; i < NIGHTMARE_MAX_ATTACKS; i++)
				{
					if (nmhook->marked[i].pent == self)
					{
						nmhook->marked[i].pent = NULL;
						break;
					}
				}
			}
		}
        gstate->RemoveEntity(self);
        return;
    }

	if (gstate->time > phook->fadeTime)
	{
		self->s.alpha -= 0.05;
	}

	if (self->s.renderfx & RF_NODRAW)
	{
        trackInfo_t tinfo;

        // clear this variable
        memset(&tinfo, 0, sizeof(tinfo));

        self->s.renderfx &= ~RF_NODRAW;
        self->s.renderfx |= RF_TRACKENT;

        // send 'track entity' msg to client
        tinfo.ent=self;
        tinfo.srcent=player;
        tinfo.fru.Set( 13, 0, 0 );
        tinfo.dstent=0;
        tinfo.flags=TEF_SRCINDEX|TEF_FRU;
        tinfo.renderfx=RF_MINLIGHT|RF_WEAPONMODEL|RF_3P_NODRAW;
        com->trackEntity(&tinfo,MULTICAST_ALL);
		self->nextthink = gstate->time + 0.1;
		return;
	}

	// pentagram is done animating... apply specials to it (glow, blood, etc.)
    if ((phook->glow) || (phook->blood))
    {
        trackInfo_t tinfo;
        // clear this variable
        memset(&tinfo, 0, sizeof(tinfo));

		// set up the default stuff
        tinfo.ent=self;
        tinfo.srcent=player;
        tinfo.fru.Set( 13, 0, 0 );
        tinfo.dstent=0;
        tinfo.flags=TEF_SRCINDEX|TEF_FRU|TEF_FXFLAGS;
        tinfo.renderfx=RF_MINLIGHT|RF_WEAPONMODEL|RF_3P_NODRAW;

		if (phook->blood)
		{
			phook->blood = 0;
            com->FlashClient( player, 0.6, 0.9, 0.6, 0.6, 0.1);
			tinfo.lightColor.Set(-0.9,-1,-1);
			tinfo.lightSize = 150;
		}

		if (phook->glow)
		{
			tinfo.lightColor.Set(0.8,0.4,0.2);
			tinfo.lightSize = 200;
		}

		tinfo.fxflags |= TEF_LIGHT;
		tinfo.flags |= TEF_LIGHTSIZE|TEF_LIGHTCOLOR;
        com->trackEntity(&tinfo,MULTICAST_ALL);
	}

	self->nextthink = gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// timeToDie()  :)
//---------------------------------------------------------------------------
void timeToDie(userEntity_t *nmController)
{
    // spawn grim reaper
	userEntity_t *ent;
    ent=gstate->SpawnEntity();
	ent->flags |= FL_NOSAVE;
    ent->owner=nmController;
    ent->movetype=MOVETYPE_NONE;
    ent->solid=SOLID_BBOX;
    ent->className="reaper";
    ent->s.modelindex=gstate->ModelIndex(winfoModel(MDL_REAPER));
	ent->s.render_scale.Set(0.001,0.001,0.001); // make him hide till we want him
    ent->s.renderfx=0;//RF_MINLIGHT|RF_FULLBRIGHT;
    ent->s.alpha=1.0;
    ent->remove=removeReaper;
	gstate->LinkEntity(ent);

    initReaperAttack(ent);
}


//---------------------------------------------------------------------------
// initReaperAttack()
//---------------------------------------------------------------------------
void initReaperAttack(userEntity_t *reaper)
{
	userEntity_t *nmController = reaper->owner;
	userEntity_t *player = nmController->owner;
	nmControllerHook_t *nmHook = (nmControllerHook_t *)nmController->userHook;
    CVector pos,angle;

	if (!nmController || !player || !nmHook)
		return;

	// find a victim!
	markedEnt_t *meVictim = NULL;
	userEntity_t *victim = NULL;
	
	reaper->s.render_scale.Set(0.001,0.001,0.001); // put the reaper away!

	while (!victim && nmHook->curDeath < NIGHTMARE_MAX_ATTACKS)
	{
		meVictim = &nmHook->marked[nmHook->curDeath++];
		victim = meVictim->ent;
		if (victim && ((victim->flags & FL_INWARP) || (victim->deadflag != DEAD_NO)) )
			victim = NULL;
	}

	// dead shooter or no victim to reap...we're done.  clean up the controller and the reaper
	if ((player->deadflag != DEAD_NO) || (!victim) )
	{
		nmController->remove(nmController);
		reaper->remove(reaper);
		return;
	}

	if( meVictim == NULL )
	{
		return;
	}

	// get the pentagram hook.  make some blood or something...
//	pentHook_t *pHook = NULL;// SCG[1/24/00]: not used
//	if (meVictim->pent)
//		pHook = (pentHook_t *)meVictim->pent->userHook;

    // get vector towards an open area of the map
    findUnobstructedDirection(victim,forward);

	reaper->think = nightmareAlmostDeath;
	reaper->nextthink = gstate->time + 0.5;
    // set reaper position
    pos=victim->s.origin+forward*100;
    reaper->s.origin = pos;

    // set reaper direction
    angle=-forward;
    VectorToAngles(angle,angle);
    reaper->s.angles = angle;

    reaper->s.renderfx=0;
    // link 'n lower
    gstate->LinkEntity(reaper);

    // freeze victim
	nightmareScaredStiff(victim);
    freezeEntity(victim,&nmHook->frozen);

	nmHook->frozen.old_owner = victim->owner;
    victim->owner=reaper->owner;

    winfoPlayAttn(reaper,SND_SHOOT_STD,1.0f, 512, 1024);
	winfoPlayAttn(reaper,SND_WIND,1.0f,512,1024);
    // client gets extra attention
    if ( victim->flags & FL_CLIENT )
    {
        playerHook_t *phook=(playerHook_t *)victim->userHook;
        float delta,diff;

        pos=reaper->s.origin;
        angle=pos-victim->s.origin;
        VectorToAngles(angle,angle);
        angle.x = 360-25;

        phook->force_angles=angle;

        BestDelta(victim->s.angles[0],angle.x,&delta,&diff);
        phook->force_rate.x=200*delta;

        BestDelta(victim->s.angles[1],angle.y,&delta,&diff);
        phook->force_rate.y=200*delta;

        phook->force_rate.z=200;

        gstate->SetClientAngles(victim,angle);
		gstate->LinkEntity(victim);
        nmHook->vangle=forward;
    }
    else
    {
        // setup current anim state to show pain
        victim->s.frameInfo.frameState |= FRSTATE_STOPPED;
    }
	nmHook->curDeath--;	// it shoots past! duh
	nmHook->reaped = 0;	// nuthin reaped yet!

	// hide the flame
    trackInfo_t tinfo;
	memset(&tinfo,0,sizeof(tinfo));

	tinfo.ent=reaper;
    tinfo.srcent=reaper;

	// for the flame
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.modelindex = gstate->ModelIndex ("models/global/we_nharref.sp2");
	tinfo.scale = 0.01;	// this needs updated as stuff happens!

    tinfo.flags=TEF_SRCINDEX|TEF_FXFLAGS|TEF_LIGHTSIZE|TEF_SCALE|TEF_MODELINDEX;
	tinfo.fxflags |= TEF_ALPHA_SPRITE|TEF_REAPER;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// death()
//---------------------------------------------------------------------------
void nightmareAlmostDeath(userEntity_t *reaper)
{
	flameSpawn(reaper);
	// rocks!
	CVector scale;
	scale.x = 1.5 + 1*frand();
	scale.y = 1.5 + 1*frand();
	scale.z = 1.5 + 1*frand();

	int count = (deathmatch->value || coop->value) ? 0.0 : (5+5*frand());
	debrisInfo_t di;
	memset(&di,0,sizeof(di));
	di.surf				= NULL;
	di.type				= DEBRIS_ROCK1;	
	di.bRandom			= 1;			
	di.count			= count;
	di.org				= reaper->s.origin;
	di.dir				= CVector(0,0,1);
	di.scale			= scale;
	di.scale_delta		= 0.0;					
	di.speed			= 750;
	di.spread			= 120;
	di.alpha			= 1.0;
	di.alpha_decay		= 0.25;
	di.gravity			= 1.0;
	di.delay			= 5;
	di.damage			= 0;
	di.owner			= NULL;

	di.bSound			= 1;						
	di.soundSource		= reaper;					
	di.minAttn			= ATTN_NORM_MAX;
	di.maxAttn			= ATTN_NORM_MAX	* 3; // these are big rox!

	di.particles		= PARTICLE_FIRE;
	di.pColor			= CVector(0.6,0,0);
	di.pScale			= 2.0;
	spawn_surface_debris(di);

    trackInfo_t tinfo;
	memset(&tinfo,0,sizeof(tinfo));

	tinfo.ent=reaper;
    tinfo.srcent=reaper;

	// for the flame
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 5.0;	// this needs updated as stuff happens!
	tinfo.scale2 = 0.8;	// alpha!
	tinfo.modelindex = gstate->ModelIndex ("models/global/we_nharref.sp2");

	// negative light technology
	tinfo.lightColor.Set(1,-1,-1);
	tinfo.lightSize = 250;
	tinfo.length = 3.0;
	
	// init the reaper!
	tinfo.Long1 |= TEF_REAPER_INIT;

    tinfo.flags=TEF_SRCINDEX|TEF_FXFLAGS|TEF_LIGHTSIZE|TEF_LIGHTCOLOR|TEF_LENGTH|TEF_MODELINDEX|TEF_SCALE|TEF_LONG1|TEF_SCALE2;
	tinfo.fxflags |= TEF_LIGHT_SHRINK|TEF_ALPHA_SPRITE|TEF_REAPER;
    com->trackEntity(&tinfo,MULTICAST_ALL);

	reaper->s.render_scale.Set(1,1,1); // make him hide till we want him
    winfoAnimate(winfo,reaper,ANIM_REAPER,FRAME_ONCE, FRAMETIME_FPS10);
    reaper->s.frameInfo.sound1Frame = winfoLastFrame (ANIM_REAPER) - 1;
    reaper->s.frameInfo.sound2Frame = winfoLastFrame (ANIM_REAPER);
    reaper->think=nightmareDeath;
    reaper->nextthink=gstate->time+0.1;
}

//---------------------------------------------------------------------------
// death()
//---------------------------------------------------------------------------
void nightmareDeath(userEntity_t *reaper)
{
	userEntity_t *nmController = reaper->owner;
	userEntity_t *player = nmController->owner;
	nmControllerHook_t *nmHook = (nmControllerHook_t *)nmController->userHook;
	if (!stricmp(nmController->className,"freed") || !player || !player->inuse || !stricmp(player->className,"freed") || !nmHook)	// the player died or something...bottom line is that the controller is gone
	{
		if (nmHook)
		{
			userEntity_t *vic = nmHook->marked[nmHook->curDeath].ent;
			if (vic)
				unfreezeEntity(vic,&nmHook->frozen);
		}
		reaper->think = NULL;
		reaper->remove(reaper);
		return;
	}

	// all good, keep going!
	markedEnt_t *meVictim=&nmHook->marked[nmHook->curDeath];
	userEntity_t *victim = meVictim->ent;


//    short lastframe=winfoLastFrame(ANIM_REAPER);// SCG[1/24/00]: not used
    CVector dir;

	// update our anim stuff and set the sound playing flags
    com->FrameUpdate(reaper);

    if ( player->deadflag != DEAD_NO )
    {
        // that breaks the spell
        if ( victim )
		{
            unfreezeEntity(victim,&nmHook->frozen);
			victim->owner = nmHook->frozen.old_owner;
		}
		reaper->remove(reaper);
        return;
    }
	
	// make monster show pain
	if (victim && victim->deadflag==DEAD_NO && !(victim->flags & FL_CLIENT) )
	{
//		playerHook_t *hook=(playerHook_t *)victim->userHook;// SCG[1/24/00]: not used

		com->FrameUpdate(victim);
		if ( victim->s.frameInfo.frameState & FRSTATE_STOPPED && rnd()>.75 )
		{
			victim->s.frameInfo.frameState=0;
			//AI_StartSequence( victim, "paina" );
			victim->s.frameInfo.frameFlags|=FRAME_ONCE;
		}
	}

	if (victim && (victim->flags & FL_CLIENT) && (victim->deadflag != DEAD_NO))
	{
		meVictim->ent = NULL;
        unfreezeEntity(victim,&nmHook->frozen);
		victim->owner = nmHook->frozen.old_owner;
		reaper->remove(reaper);
        return;
	}

	// update the flame sprite!
    trackInfo_t tinfo;
	memset(&tinfo,0,sizeof(tinfo));
	tinfo.ent=reaper;
    tinfo.srcent=reaper;

	// for the flame
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 2 + 4.0*( fabs(REAPER_TOP_FRAME - 0.5*reaper->s.frame + 1) / REAPER_TOP_FRAME );
    tinfo.flags=TEF_SRCINDEX|TEF_FXFLAGS|TEF_SCALE|TEF_LONG1;
	tinfo.Long1 |= TEF_REAPER_SMOKE;
	tinfo.fxflags |= TEF_LIGHT_SHRINK|TEF_ALPHA_SPRITE|TEF_REAPER;
    com->trackEntity(&tinfo,MULTICAST_ALL);

	// do damage near end of attack
	if ( (nmHook->reaped >= 3) || !victim )
	{
		meVictim->ent = NULL;
		meVictim->pent = NULL;
		initReaperAttack(reaper);
		return;
	}
	else if (nmHook->reaped >= 1)
	{
		nmHook->reaped++;
	}
	else if ( (reaper->s.frame >= (winfoLastFrame (ANIM_REAPER) - 1)) && victim)//reaper->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 && victim )
	{
		nmHook->reaped = 1;
		reaper->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;

		winfoPlayAttn(reaper,SND_ATTACK,1.0f, 256, 1024);
		unfreezeEntity(victim,&nmHook->frozen);
		victim->owner = nmHook->frozen.old_owner;
		if ( victim->flags & FL_CLIENT )
		{
			CVector pushdir,vel;

			VectorToAngles(nmHook->vangle,nmHook->vangle);

			pushdir=victim->s.origin-reaper->s.origin;
			pushdir.Normalize();

			vel=victim->velocity;
			vel=pushdir*1500;
			victim->velocity = vel;

			victim->groundEntity = NULL;
		}

		//	Nelno:	calculate vector from reaper to victim for good gibbage spread
		//	0, 0, -24 moves origin of reaper down a bit so gibs fly upwards
		dir = victim->s.origin - (reaper->s.origin + CVector(0, 0, -24));
		dir.Normalize ();
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;
		WEAPON_HIT(player,1);
		com->Damage(victim,reaper,player,victim->s.origin,dir,winfo->damage,DAMAGE_INERTIAL);
	}

    reaper->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// removeReaper()
//---------------------------------------------------------------------------
void removeReaper(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
    gstate->RemoveEntity(self);
}


//---------------------------------------------------------------------------
// findUnobstructedDirection()
//---------------------------------------------------------------------------
void findUnobstructedDirection(userEntity_t *ent,CVector &forward)
{
    lookInfo_t look[(int)REAP_MAX_LOOKS];

    CVector pos,end,angles,vec;
    short i,next,prev,bestangle;
    float rot,bestdist;

    // get distance from each angle
    rot=0;
    pos=ent->s.origin;
    pos.z += 10;

    angles=ent->s.angles;
    angles.x=0;

    for ( i=0; i<REAP_MAX_LOOKS; i++ )
    {
        angles.AngleToVectors(forward,right,up);
        look[i].forward=forward;

        end=pos+forward*300;
        gstate->TraceLine(pos,end,true,ent,&trace);
        vec=pos-trace.endpos;
        look[i].dist=vec.Length();

        rot += 360.0/REAP_MAX_LOOKS;
        angles.y += rot;
        if ( angles.y >= 360 )
            angles.y -= 360;
    }   

    // find best unobstructed angle
    bestangle=-1;
    bestdist=-1;
    for ( i=0; i<REAP_MAX_LOOKS; i++ )
    {
        prev=i-1;
        if ( prev < 0 )
            prev=REAP_MAX_LOOKS-1;

        next=i+1;
        if ( next==REAP_MAX_LOOKS )
            next=0;

        look[i].dist3=look[i].dist;      //(look[prev].dist+look[i].dist+look[next].dist)/3;
        if ( look[prev].dist >= REAP_MIN_DIST && look[i].dist >= REAP_MIN_DIST && look[next].dist >= REAP_MIN_DIST )
        {
            // best angle?
            if ( look[i].dist3 > bestdist )
            {
                bestangle=i;
                bestdist=look[i].dist3;
            }
        }
    }

    // hmm... reaper's getting pretty claustrophobic right about now...
    // find the largest avg distance, period!
    if ( bestangle==-1 )
        for ( i=0; i<REAP_MAX_LOOKS; i++ )
        {
            prev=i-1;
            if ( prev < 0 )
                prev=REAP_MAX_LOOKS-1;

            next=i+1;
            if ( next==REAP_MAX_LOOKS )
                next=0;

            // best angle?
            if ( look[i].dist > bestdist )
            {
                bestangle=i;
                bestdist=look[i].dist3;
            }
        }

	// SCG[1/24/00]: Added check to make sure index -1 is not accesed
	if( bestangle >= 0 )
	{
	    forward=look[bestangle].forward;
	}
}

//-----------------------------------------------------------------------------
//	weapon_nightmare
//-----------------------------------------------------------------------------
void weapon_nightmare(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	light_think
//-----------------------------------------------------------------------------
void light_think(userEntity_t *self)
{
	if ( !self->owner || !self->owner->inuse || !stricmp(self->owner->className,"freed") || (self->owner->s.frame >= (REAPER_TOP_FRAME + 5)))//winfoLastFrame(ANIM_REAPER)))
	{
		gstate->RemoveEntity(self);
		return;
	}

	self->s.render_scale.x += 15*crand();
	self->s.render_scale.y += 15*crand();
	self->nextthink = gstate->time + 0.2;
}

//-----------------------------------------------------------------------------
//	flamespawn
//-----------------------------------------------------------------------------
void flameSpawn(userEntity_t *self)
{
	userEntity_t *light;
	// spawn light 

	light = gstate->SpawnEntity();
	// setup the light data
	trace_t tr;
	CVector start = self->absmin;
	CVector end = start;
	end.z -= 2000;

	// find the start position
	userEntity_t *ignore = self;
	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
 			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}
	light->s.origin = tr.endpos;

	// now, find the end position
	ignore = self->owner;
	start = self->absmin;
	end = start;
	end.z += 2000;
	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
 			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}

	light->s.render_scale = tr.endpos;

	light->movetype			= MOVETYPE_NONE;
	light->solid			= SOLID_NOT;
	light->s.modelindex		= gstate->ModelIndex("models/e1/me_cambotf.sp2");		// must be non-zero
	light->s.alpha			= 0.01;
	// set the position
	light->view_ofs			= self->view_ofs;
	// add a spotlight to this thing 
	light->s.renderfx		|= (RF_SPOTLIGHT|RF_TRANSLUCENT);//|RF_LIGHTFLARE);
	// set the spotlight color
	light->s.color.Set( 0.90f,0.20f ,0.10f );

	VectorToAngles(CVector(0,0,1),light->s.angles);
	
	// set the spotlight diameter
	light->s.frame	= 10;
	// set owner and enemy
	light->owner	= self;
	// setup thinking
	light->think	 = light_think;
	light->nextthink = gstate->time + 0.1;
	// put this entity in the world
	gstate->LinkEntity(light);

	// make a cool scorch mark
	CVector dst = self->absmin;
	dst.z -= 2000;
	tr = gstate->TraceLine_q2( self->s.origin, dst, self, MASK_MONSTERSOLID);
	if (tr.ent)
	{
		gstate->WriteByte( SVC_TEMP_ENTITY );
		gstate->WriteByte( TE_SCORCHMARK );
		gstate->WritePosition( tr.endpos );
		gstate->WriteShort( tr.plane.planeIndex );
		gstate->WriteShort( tr.ent->s.modelindex );
		gstate->WriteShort( tr.ent->s.number);
		gstate->WriteFloat( 2 );
		gstate->WriteFloat( 0 );
		gstate->WriteByte( SM_EARTH );
		gstate->MultiCast( zero_vector, MULTICAST_ALL );
	}
}

//-----------------------------------------------------------------------------
//  play a really cool scream sound!
//-----------------------------------------------------------------------------
void nightmareScaredStiff(userEntity_t *self)
{
    char path[256]={"\0"};
    char modelname[64]={"\0"};

	if( self->client == NULL )
	{
		return;
	}

//    strncpy(modelname, Info_ValueForKey(self->client->pers.userinfo, "modelname"), 64);
	if (deathmatch->value || coop->value)
		strncpy(modelname, self->client->pers.body_info.modelname, 64);
	else
		strncpy(modelname, Info_ValueForKey(self->client->pers.userinfo, "modelname"), 64);

    if(strstr(modelname, "mikiko"))
	{
        sprintf(path, "mikiko/death8.wav");
	}
    else if(strstr(modelname, "hiro"))
	{
        sprintf(path, "hiro/death8.wav");
	}
    else if(strstr(modelname, "superfly"))
	{
        sprintf(path, "superfly/death4.wav");
	}

    if(self->input_entity)
        gstate->StartEntitySound(self->input_entity, CHAN_BODY, gstate->SoundIndex(path), 1.0f, 256,1024);
    else
        gstate->StartEntitySound(self, CHAN_BODY, gstate->SoundIndex(path), 1.0f, 256,1024);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_nightmare_register_func()
{
	gstate->RegisterFunc("nightmareUse",nightmareUse);
	gstate->RegisterFunc("nightmaredoPentagram",doPentagram);
	gstate->RegisterFunc("nightmareDeath",nightmareDeath);
	gstate->RegisterFunc("nightmarelight_think",light_think);
	gstate->RegisterFunc("removeremoveReaper",removeReaper);
	gstate->RegisterFunc("nightmareAlmostDeath",nightmareAlmostDeath);
	gstate->RegisterFunc("initReaperAttack",initReaperAttack);
	gstate->RegisterFunc("nightmareControllerUpkeep",nightmareControllerUpkeep);
	gstate->RegisterFunc("nightmareControllerBeatDown",nightmareControllerBeatDown);
	gstate->RegisterFunc("nightmareControllerSearch",nightmareControllerSearch);
	gstate->RegisterFunc("nightmareControllerTracePent",nightmareControllerTracePent);
	gstate->RegisterFunc("nightmareControllerRemove",nightmareControllerRemove);
	gstate->RegisterFunc("nightmareGive",nightmareGive);
	gstate->RegisterFunc("nightmareSelect",nightmareSelect);
}

