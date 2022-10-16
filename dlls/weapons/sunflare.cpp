//---------------------------------------------------------------------------
// SUNFLARE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "sunflare.h"

enum
{
    // model defs
    MDL_START,
    MDL_POT,
    MDL_FIRE,
    MDL_POTFIRE,

    // sound defs
    SND_START,
    SND_EXPLODE,
    SND_BURNING,

    // special frame defs
    ANIM_START,
    ANIM_FIRE
};

typedef struct
{
	weapon_t		weapon;			// make it compatible with weapon_t
	short			flameActive;	// is the flame turned on?
	short			flameStarted;	// had the flame started?
} sunflare_weap_t;

// weapon info def
weaponInfo_t sunflareWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SUNFLARE),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SUNFLARE_AMMO),
	// names, net names, icon
	"weapon_sunflare",
	tongue_weapons[T_WEAPON_SUNFLARE],
	"ammo_sunflare",
	tongue_weapons[T_WEAPON_SUNFLARE_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_sflare.dkm",
		"models/e2/a_sflare.dkm",
		MDL_NOMODEL,
		"models/e2/we_sunprj.dkm",
		"models/global/e2_firea.sp2",
		"models/global/e_fireb.sp2",
		NULL
	},

	// sound files
	{
		"e2/we_sflareamba.wav",
		"e2/we_sflareready.wav",
		"e2/we_sflareaway.wav",
		"e2/we_sflareshoota.wav",
		"e2/we_sflareamba.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		"e2/we_sflareexplodea.wav",
		"e2/we_sflareexploded.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,
		MDL_FIRE,"sp2",0,0,
		0,
		NULL
	},

	// commands
	{
		wpcmds[WPCMDS_SUNFLARE][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_3,
		wpcmds[WPCMDS_SUNFLARE][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_3,
		NULL
	},

	weapon_give_3,
	weapon_select_3,
	sunflareCommand,
	sunflareUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(sunflare_weap_t),

	200,
	20,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	1,      //	initial ammo
	600.0,  //	speed
	512.0,  //	range
	1.0,        //	damage
	0.0,        //	lifetime

	WFL_FIRE|WFL_FORCE_SWITCH|WFL_PLAYER_ONLY,       //	flags

	WEAPONTYPE_GLOVE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_SUNFLARE],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_SUNFLARE),
	ATTACKER_VICTIM,

	WEAPON_SUNFLARE,
	0,
	weapThink
};

// local weapon info access
static weaponInfo_t *winfo=&sunflareWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *sunflareGive(userEntity_t *self, int ammoCount)
{
	// see how many we have
	if (!winfoCheckMaxCount(self, winfo))
	{
		return NULL;
	}

    return(weaponGive(self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// sunflare_select()
//---------------------------------------------------------------------------
short sunflareSelect(userEntity_t *self)
{
    short rt; // have to select the weapon before you do nasty stuff to it.
    rt = weaponSelect(self,winfo);

    // weapon selected, ok to fire
	sunflare_weap_t *weap;
    if ( weap=(sunflare_weap_t *)gstate->InventoryFindItem(self->inventory,winfo->weaponName) )
	{
		weap->flameActive = 0;
		weap->flameStarted = 0;
	}

	return rt;
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *sunflareCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
	if (!stricmp(commandStr,"change"))
	{
		userEntity_t *self = (userEntity_t *)data;
		if (self && (self->flags & FL_CLIENT))
		{
			userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
			if (weap)
				weap->s.effects2 &= ~EF2_SUNFLARE_FLAME;
		}
	}

	return winfoGenericCommand(winfo,inv,commandStr,data);
}

void sunflareRemoveFlame(userEntity_t *self)
{
	com->untrackEntity(self,NULL,MULTICAST_ALL);
}


//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void sunflareUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *went=(userEntity_t *)self->client->ps.weapon;
	sunflare_weap_t *weap = (sunflare_weap_t *)ptr;

    if ( !weaponHasAmmo(self,true) )
        return;

    weaponUseAmmo (self, true);

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

	if (went)
	{
		went->s.effects2 &= ~EF2_SUNFLARE_FLAME;
		gstate->LinkEntity(went);
	}
//		trackRemoveEffect(went,TEF_FIRE);
//		sunflareRemoveFlame(went);

    phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD) + winfoNumFrames(ANIM_SHOOT_STD)*0.5;
    phook->fxFrameFunc=sunflareShootFunc;
}

//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void sunflareShootFunc(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    weapon_t *weapon=(weapon_t *)self->curWeapon;
    userEntity_t *ent/*,*went=(userEntity_t *)self->client->ps.weapon*/;// SCG[1/24/00]: not used

    phook->fxFrameNum = -1;
    phook->fxFrameFunc=sunflareRestartEffect;

    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1+13.0f, winfo->fWeaponOffsetZ1-5 );
    ent=weapon_spawn_projectile(self,offset,winfo->speed,-22.5,"sunflare_pot", FALSE);
	offset = ent->velocity;
	offset.Normalize();
	ent->s.old_origin = ent->s.origin - 5*offset;
    ent->movetype=MOVETYPE_TOSS;
    ent->think=sunflareThink;
    ent->nextthink=gstate->time+.1;
    ent->touch=sunflareTouch;
    ent->curWeapon=self->curWeapon;
    ent->s.effects=0;//EF_SMOKETRAIL;
    ent->avelocity.x=90+rnd()*60;
    ent->avelocity.y=90+rnd()*60;
    ent->avelocity.z=90+rnd()*60;

    winfoSetModel(ent,MDL_POT);
    winfoPlay(self,SND_SHOOT_STD,1.0f);
    gstate->SetSize(ent,-12,-12,-18,12,12,18);
    ent->s.render_scale.Set(2,2,2);

    weapon->lastFired=gstate->time;
	WEAPON_FIRED(self);
}

void sunflareAddFlame(userEntity_t *self)
{
	if (!self || !self->owner)
		return;

    trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));
	weaponTrackMuzzle(tinfo,self,self->owner,winfo,true);

    tinfo.modelindex=gstate->ModelIndex("models/global/e_fireb.sp2");
    tinfo.flags |= TEF_FXFLAGS|TEF_MODELINDEX;
	tinfo.fxflags |= TEF_FIRE|TEF_FX_ONLY;
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// restartEffect()
//---------------------------------------------------------------------------
void sunflareRestartEffect(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *went=(userEntity_t *)self->client->ps.weapon;
	if (!went || !phook)
		return;

	sunflare_weap_t *weap = (sunflare_weap_t *)self->curWeapon;
	if (!weap)
		return;

    phook->fxFrameFunc=NULL;
    if ( ! (gstate->PointContents(self->s.origin) & MASK_WATER) )
    {
        weap->flameActive = 0;
		weap->flameStarted = 0;
//		sunflareAddFlame(went);
    }

}

void sunflareExplode(userEntity_t *entSelf,userEntity_t *other)
{
    winfoPlay(entSelf,SND_EXPLODE,1.0f);

    gstate->SetSize(entSelf,0,0,-18,0,0,18);
	entSelf->hacks = gstate->time + 2;
	entSelf->think = flame_think2;
    entSelf->nextthink=gstate->time+.1;
	entSelf->mass = 1.85f;
    entSelf->s.render_scale.Set(0.001,0.001,0.001);
	entSelf->velocity.Zero();
	if (!(other && other->takedamage))
	{
		entSelf->velocity.x = rnd() > 0.5 ? rnd()*entSelf->velocity.x : -rnd()*entSelf->velocity.x;
		entSelf->velocity.y = rnd() > 0.5 ? rnd()*entSelf->velocity.y : -rnd()*entSelf->velocity.y;
	}
	entSelf->velocity.z = -rnd()*entSelf->velocity.z*2;
	entSelf->movetype=MOVETYPE_FLYMISSILE;
    entSelf->solid=SOLID_NOT;
    entSelf->flags=FL_EXPLOSIVE;
	entSelf->s.effects = 0;

	// set up the hook
    entSelf->userHook = gstate->X_Malloc(sizeof(sfHook_t),MEM_TAG_HOOK);
    sfHook_t *hook=(sfHook_t *)entSelf->userHook;
	hook->flames = 0;

    winfoSetModel(entSelf,MDL_FIRE);
    winfoAnimate(winfo,entSelf,ANIM_FIRE,FRAME_LOOP,.05);
    entSelf->s.frame=rnd()*winfoNumFrames(ANIM_FIRE);       // start at random frame

	gstate->LinkEntity(entSelf);

	if ((other) && (other->takedamage))
	{
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;
		
		sfHook_t *hook=(sfHook_t *)entSelf->userHook;
		if (hook)
			hook->hits.AddHit(entSelf->owner,other);

		e2_com_Damage(other,entSelf,entSelf->owner,other->s.origin,zero_vector,5 * winfo->damage,DAMAGE_NONE);
	}

    clientSmoke(entSelf->s.origin,6,6,95,20);
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void sunflareThink(userEntity_t *self)
{
    CVector vecDistance;
    int     iPointContents;

	if (!(self->s.effects & EF_SMOKETRAIL))
	{
		self->s.effects |= EF_SMOKETRAIL;
	}

    self->nextthink=gstate->time+.1;
    vecDistance = self->s.origin - self->owner->s.origin;

    if( (iPointContents = gstate->PointContents(self->s.origin)) & MASK_WATER )
    {
        self->velocity.z = 0.0f;
        sunflareExplode(self,NULL);
    }

}

//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void sunflareTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    if ( plane )
	{
		spawn_sprite_explosion(self,self->s.origin,0);
        entBackstep(self, plane->normal, 4);
	}
	self->touch = NULL;
    sunflareExplode(self,other);
}


//---------------------------------------------------------------------------
// flame_think2()
//---------------------------------------------------------------------------
void flame_think2(userEntity_t *self)
{
/*    if(self->s.render_scale.x < 5.0)
    {
        self->s.render_scale.x += 0.51;
        self->s.render_scale.y += 0.51;
        self->s.render_scale.z += 0.51;
    }*/

	if ( (gstate->PointContents(self->s.origin) & MASK_WATER ))
	{
		self->movetype = MOVETYPE_FLOAT;
		self->gravity  = 1.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
	}

	flame_damage(self);

	if (self->velocity.Length() < 10)
	{
		sfHook_t *hook=(sfHook_t *)self->userHook;

		winfoPlayAttn(self,SND_BURNING,1.0f,600,1200);
		self->s.render_scale.Set(0.001,0.001,0.001);
		self->hacks = gstate->time + 5;
		trackInfo_t tinfo;
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent=self;
		tinfo.srcent=self;

		hook->flames = 5 + 4.9*frand();
		tinfo.Long2 = hook->flames;			// number of flames to spawn
		tinfo.numframes = winfoNumFrames(ANIM_FIRE);
		tinfo.length = 50 + 10*(hook->flames - 5);	// radius
		tinfo.modelindex = gstate->ModelIndex("models/global/e2_firea.sp2");
		tinfo.modelindex2 = gstate->ModelIndex("models/global/e_sflorange.sp2");
		tinfo.scale = 2.0;
		tinfo.scale2 = 0.6;								// alpha
		tinfo.frametime = 0.05;
		tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX|TEF_NUMFRAMES|TEF_LONG2|TEF_LENGTH|TEF_MODELINDEX|TEF_MODELINDEX2|TEF_SCALE|TEF_SCALE2|TEF_FRAMETIME;
		if (gstate->PointContents(self->s.origin) & MASK_WATER )
		{
			tinfo.lightSize = 2;
			tinfo.flags |= TEF_LIGHTSIZE;
		}

		tinfo.fxflags = TEF_SUNFLARE_FX|TEF_FX_ONLY;
		com->trackEntity(&tinfo,MULTICAST_PVS);

		self->think = flame_doflames;
	}
	self->nextthink=gstate->time+.1;
}

void flame_doflames(userEntity_t *self)
{
	if (gstate->time > self->hacks)
	{
		self->think = flame_die;
		self->nextthink = gstate->time + 5;
		return;
	}

	// do some radius damage!
	flame_damage(self);
	self->nextthink = gstate->time + 0.3;
}

void flame_die(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}


void flame_damage(userEntity_t *self)
{
	sfHook_t *hook=(sfHook_t *)self->userHook;
	if (!hook)
		return;

	float range = 60 + 10*(hook->flames);
	float damage = 3 * (winfo->damage + 0.25*hook->flames);

	userEntity_t *ent = NULL;
	while ( (ent = com->FindRadius(ent, self->s.origin, range)) != NULL )
	{
        if ( ent->takedamage )
		{
			hook->hits.AddHit(self,ent);
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;
            e2_com_Damage(ent,self,self->owner,self->s.origin,zero_vector,damage,DAMAGE_NONE);
		}
	}
}

//-----------------------------------------------------------------------------
//	weapon_sunflare
//-----------------------------------------------------------------------------
void weapon_sunflare(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////
//  weapThink()
//
//  Description:
//      Custom think function for the sunflare weapon entity. This 
//      is mostly used to determine when the flame in the pot should
//      be active and when it shouldn't.
//
void weapThink(userEntity_t *entWeapon)
{
	userEntity_t *player = entWeapon->owner;
	if (!player)
		return;

	sunflare_weap_t *weap = (sunflare_weap_t *)player->curWeapon;
	if (!weap)
		return;

	if (stricmp(weap->weapon.name,"weapon_sunflare"))
		return;

	int iPointContents = gstate->PointContents(entWeapon->s.origin);

	if((weap->flameActive == 1)&& (iPointContents & MASK_WATER))
	{
		// deactivate the flame
		entWeapon->s.effects2 &= ~EF2_SUNFLARE_FLAME;
		gstate->LinkEntity(entWeapon);
		weap->flameActive = 0;
		weap->flameStarted = 0;
	} 
	else if (weap->flameActive == 0 && !(iPointContents & MASK_WATER))
	{
		// activate the flame
		if(weap->flameStarted)
		{
			entWeapon->s.effects2 |= EF2_SUNFLARE_FLAME;
			gstate->LinkEntity(entWeapon);
			weap->flameActive = 1;
		}
	}

	if(!weap->flameStarted)
	{
		weap->flameStarted = 1;
		if ( !(iPointContents & MASK_WATER) )
		{
			entWeapon->s.effects2 |= EF2_SUNFLARE_FLAME;
			gstate->LinkEntity(entWeapon);
			weap->flameActive = 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_sunflare_register_func()
{
	gstate->RegisterFunc("sunflareThink",sunflareThink);
	gstate->RegisterFunc("sunflareflame_think2",flame_think2);
	gstate->RegisterFunc("sunflareflame_doflames",flame_doflames);
	gstate->RegisterFunc("sunflareUse",sunflareUse);
	gstate->RegisterFunc("sunflareflame_die",flame_die);
	gstate->RegisterFunc("sunflareTouch",sunflareTouch);
	gstate->RegisterFunc("sunflareGive",sunflareGive);
	gstate->RegisterFunc("sunflareSelect",sunflareSelect);
}
