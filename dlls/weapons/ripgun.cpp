//---------------------------------------------------------------------------
// RIPGUN (CHAINGUN)
//---------------------------------------------------------------------------
#include "weapons.h"
#include "ripgun.h"

enum
{
    // model defs
    MDL_START,
    MDL_MUZZLEFLASH,

    // sound defs
    SND_START,
	SND_LOCK,
	SND_CLICK,
	SND_SPIN,

    // special frame defs
    ANIM_START,
    ANIM_SPINDOWN
};

// weapon info def
weaponInfo_t ripgunWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_RIPGUN),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SLUGGER_AMMO),
	// names, net names, icon
	"weapon_ripgun",
	tongue_weapons[T_WEAPON_RIPGUN],
	"ammo_ripgun",
	tongue_weapons[T_WEAPON_SLUGGER_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_ripgun.dkm",
		"models/e4/a_ripgun.dkm",			
		"models/e4/wa_slug.dkm",
		"models/e4/we_mfslugr.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_sluggerready.wav",
		"e4/we_sluggeraway.wav",
		"e4/we_sluggershoota.wav",
		"e4/we_sluggeramba.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		"e4/we_sluggerlock.wav",
		"e4/we_glockclick.wav",
		"e4/we_sluggerspin.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		ANIM_NONE,//doesn't exist-1,"ambb",0,0,
		ANIM_NONE,
		-1,"spdn",0,0,
		0,
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_RIPGUN][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_4,
		wpcmds[WPCMDS_RIPGUN][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_4,
		NULL
	},

	weapon_give_4,
	weapon_select_4,
	ripgun_command,
	ripgun_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(ripgun_t),

	600,
	250,    //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	50,     //	initial ammo
	0,      //	speed
	2000.0, //	range
	100.0,   //	damage
	0.3,    //	lifetime

	WFL_AI_TRACK_TARG|WFL_FORCE_SWITCH|WFL_CROSSHAIR,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_RIPGUN],
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_CORDITE],
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_RIPGUN),
	VICTIM_ATTACKER,

	WEAPON_RIPGUN,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&ripgunWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *ripgun_give(userEntity_t *self, int ammoCount)
{
    return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// slugger_select()
//---------------------------------------------------------------------------
short ripgun_select(userEntity_t *self)
{
    ripgun_t *ripgun;
    short rt;

    rt=weaponSelect(self,winfo);

    // weapon selected, ok to fire
    if ( ripgun=(ripgun_t *)gstate->InventoryFindItem(self->inventory,winfo->weaponName) )
	{
        ripgun->stopfiring=false;
		ripgun->controller = NULL;
	}

    return(rt);
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *ripgun_command(struct userInventory_s *inv, char *commandStr, void *data)
{
    userEntity_t *self=(userEntity_t *)data;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used

    if ( !stricmp(commandStr, "change") )
    {
        ripgun_t *ripgun=(ripgun_t *)self->curWeapon;

        // weapon deselected, stop firing
        ripgun->killtime=gstate->time;
        ripgun->stopfiring=true;

        winfoGenericCommand(winfo,inv,commandStr,data);
    }
    else
        winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void ripgun_use(userInventory_s *ptr, userEntity_t *self)
{
	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook=(playerHook_t *)self->userHook;
		ripgun_t *ripgun=(ripgun_t *)self->curWeapon;
//		userEntity_t *went=(userEntity_t *)self->client->ps.weapon;// SCG[1/24/00]: not used

		short noAmmo = weaponHasAmmo(self,true) ? 0 : 1;
		if ( noAmmo || ripgun->stopfiring )
		{
			if (noAmmo)
				winfoPlay(self,SND_CLICK,1.0);
			ripgun->stopfiring = false;
			return;
		}

		// spawn ent that controls shooting while attack is pressed
		if ( !ripgun->controller )
		{
			winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_LOOP|FRAME_WEAPON|FRAME_NODELAY,.05);

			ripgun->controller=gstate->SpawnEntity();
			ripgun->controller->flags |= FL_NOSAVE;

			phook->fxFrameFunc=ripgun_shoot_func;
			phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+winfoNumFrames(ANIM_SHOOT_STD)*.75;
		}

		// when attack is pressed, keeps setting killtime to .25 secs from now
		ripgun->killtime=gstate->time + winfo->lifetime;
		// delay things so that the weapon can't switch while firing!
		phook->attack_finished += 0.1;
	}
	else if (self->flags & FL_BOT)
	{
	    ripgun_shootSlug(self);
/*		weaponMuzzleFlash(	self,self,winfo,					// thing tracked, player, weapon info
							"models/global/genflash.dkm",		// flash model (modelindex)
							1, 0.05,							// model frame count, frametime
							8,									// scale
							175 + 75*frand(),					// light value (may be 0)
							CVector(0.8,0.4,0.2),				// light color (ignored if no lightsize)
							TEF_ANIM2_THIRD|TEF_ANIM2_FIRST,					// any of TEF_AMIN2_* animation flags
							0,						// additional fxflags (TEF_NOROTATE)
							0.6,
							CVector(-2,0,0));*/
		weaponMuzzleFlash(	self,self,winfo,FLASH_RIPGUN);
	}
}

//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void ripgun_shoot_func(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    ripgun_t *ripgun=(ripgun_t *)self->curWeapon;
    userEntity_t *ent,*went=(userEntity_t *)self->client->ps.weapon;

    phook->fxFrameFunc=NULL;
	winfoPlayLooped(self,SND_SPIN,1.0);

    ent=ripgun->controller;
    ent->className="ripgun_controller";
    ent->owner=self;
    ent->s.renderfx |= RF_TRACKENT;
    ent->s.origin = self->s.origin;
    ent->think=ripgun_shootThink;
    ent->remove=ripgun_removeController;
	ent->s.modelindex = gstate->ModelIndex( "models/e4/we_mfripg.sp2" );
	ent->s.render_scale.Set(0.001,0.001,0.001);
    gstate->LinkEntity(ent);

    self->nextthink=gstate->time+.1;

	if (went)
		went->s.renderfx |= RF_SHAKE;

    ripgun->controller=ent;

	trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));

    ripgun_shootThink(ent);
}

void ripgun_lock(userEntity_t *self)
{
	winfoPlay(self,SND_LOCK,1.0);
	playerHook_t *phook = (playerHook_t *)self->userHook;
	if (phook)
	{
		phook->fxFrameNum = -1;
		phook->fxFrameFunc = NULL;
	}
}
//---------------------------------------------------------------------------
// shootThink()
//---------------------------------------------------------------------------
void ripgun_shootThink(userEntity_t *self)
{
    ripgun_t *ripgun=(ripgun_t *)self->owner->curWeapon;
    userEntity_t *went=(userEntity_t *)self->owner->client->ps.weapon;
    userEntity_t *owner=self->owner;

	if (self->owner->flags & FL_CLIENT)
	{
		// kickback from shooting
		owner->client->kick_origin.x = rnd() * 0.65;       // .35
		owner->client->kick_origin.y = rnd() * 0.65;       // .35
		owner->client->kick_origin.z = rnd() * 0.65;       // .35

		owner->client->kick_angles.x = rnd() * 1.3;        // .7
		owner->client->kick_angles.y = rnd() * 1.3;        // .7
		owner->client->kick_angles.z = rnd() * 1.3;        // .7

		owner->client->kick_angles.x = 3.5 + rnd() * -2.4;           // -.5
	}

   //	Nelno:	remove self if owner died
    if ( self->owner->deadflag != DEAD_NO || ripgun== NULL)
    {
		if (ripgun)
			ripgun->controller = NULL;
        self->remove(self);
        return;
    }

    self->s.origin = self->owner->s.origin;
    gstate->LinkEntity(self);

    ripgun_shootSlug(self->owner);

	weaponMuzzleFlash(	self,owner,winfo,FLASH_RIPGUN);
    //	if last shot used up all our ammo, or if time is up, then remove entity
    if ( gstate->time > ripgun->killtime || !weaponHasAmmo(self->owner,false) )
    {
        if ( !ripgun->stopfiring )
		{
			winfoStopLooped(self->owner);
            winfoAnimate(winfo,went,ANIM_SPINDOWN,FRAME_ONCE|FRAME_NODELAY,.05);
			playerHook_t *phook = (playerHook_t *)self->owner->userHook;
			if (phook)
			{
				phook->fxFrameNum = winfoLastFrame(ANIM_SPINDOWN);
				phook->fxFrameFunc = ripgun_lock;
			}
		}
		if (went)
			went->s.renderfx &= ~RF_SHAKE;

        ripgun->controller=NULL;
        self->remove(self);
        return;
    }

    self->nextthink=gstate->time+.1;

    ripgun->weapon.lastFired=gstate->time;

}

//---------------------------------------------------------------------------
// shootSlug()
//---------------------------------------------------------------------------
void ripgun_shootSlug(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *weapon=(userEntity_t *)self->client->ps.weapon;
    CVector angle,start,newstart,end,ofs,vec, vecDir;
    trace_t     tr;

    //	Nelno:	because of other checks, there should ALWAYS be ammo
    //	when we get to this point
    if (!weaponUseAmmo (self, true))
		return;

	WEAPON_FIRED(self);

    if ( tr.ent=phook->autoAim.ent )
    {
        tr.endpos = phook->autoAim.shootpos;
        tr.fraction=.5;
    }

    // fire shot
    phook->fired++;

    winfoPlay(self,SND_SHOOT_STD,1.0f);        // need looping sound

    angle=self->client->v_angle;
    ofs.x = rnd() > .5 ? rnd()*1 : -rnd()*1;
    ofs.y = rnd() > .5 ? rnd()*1 : -rnd()*1;
    ofs.z = rnd() > .5 ? rnd()*1 : -rnd()*1;
    angle=angle+ofs;
    // using auto-aim?
    if ( tr.ent )
    {
        angle=tr.endpos-self->s.origin;
        VectorToAngles(angle,angle);
    }
    angle.AngleToVectors(forward,right,up);

    start=self->s.origin+weaponHandVector(self,forward*10,right*3)+CVector(0,0,20);
    newstart=start+ofs;

	if (tr.ent)
		end = tr.endpos;
	else
		end=newstart+forward*winfo->range;

    // using auto-aim entity?
    tr = weaponTraceShot(self, weaponHandVector(self,forward*10,right*3)+CVector(0,0,20), 0);

	if ( tr.ent && (tr.fraction != 1) )
    {
        if ( tr.ent->takedamage != DAMAGE_NO )
        {
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

			WEAPON_HIT(self,1);
            com->Damage(tr.ent, self, self, tr.endpos, forward, winfo->damage, DAMAGE_INERTIAL);
            phook->hit++;

            weaponBloodSplat (tr.ent, newstart, end, tr.endpos, 128.0);
        }
		
        if(tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO)
		{
			if(tr.surface)//Let's make sure we have a surface before we try to check out the flags for it <nss>
			{
				if (!(tr.surface->flags & SURF_SKY))
				{
					weaponScorchMark( tr, SM_BULLET );

					clientSmoke(tr.endpos,1,20,5,20);
//					vecDir = tr.endpos - newstart;
//					vecDir.Normalize();
//					weaponRicochet(tr.endpos);
				}
			}
		}
    }

    // show tracer
    vec=start+weaponHandVector(self,-forward,right*4)-CVector(0,0,2);
	clientSmoke(vec,.01,5,1,4);
	
	if (weapon && ((rand() & 0x04) == 0x04))
	{
		ripgun_shootTracer(weapon,start,tr.endpos);
	}
}


//---------------------------------------------------------------------------
// slug_think()
//---------------------------------------------------------------------------
void ripgun_slug_think(userEntity_t *self)
{
	if ((++self->delay >= 5) || (gstate->time > self->hacks))
	{
		RELIABLE_UNTRACK(self);
		self->think = NULL;
		gstate->RemoveEntity(self);
		return;
	}
	self->nextthink = gstate->time + 0.01f;
}

//---------------------------------------------------------------------------
// shootTracer()
//---------------------------------------------------------------------------
void ripgun_shootTracer(userEntity_t *self,CVector &start,CVector &end)
{
	userEntity_t *slug = gstate->SpawnEntity();

	gstate->SetSize(slug, 0, 0, 0, 0, 0, 0);
	slug->movetype	= MOVETYPE_NONE;
	slug->s.origin = self->s.origin;
	
	slug->owner	= self;
	slug->s.frame	= 0;

	slug->s.modelindex = gstate->ModelIndex ("models/global/e_sflred.sp2");
	slug->s.render_scale.Set(0.001f, 0.001f, 0.001f);
	slug->solid		= SOLID_NOT;
	
	slug->think = ripgun_slug_think;
	
	slug->nextthink = gstate->time + 0.01f;
	
	slug->delay = 0;
	slug->hacks = gstate->time + 2;
	gstate->LinkEntity(slug);

	//Tracer Bullshit
    trackInfo_t tinfo;

	CVector Dir;
    VectorToAngles(end-start,Dir);
	// clear this variable
    memset(&tinfo, 0, sizeof(tinfo));
    tinfo.ent=slug;
    tinfo.srcent=slug;
	tinfo.extra = self;
    tinfo.fru.Zero();
 	tinfo.altangle = self->s.angles;
	tinfo.altpos = end;
	
	tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX|TEF_ALTANGLE|TEF_ALTPOS|TEF_EXTRAINDEX;
    tinfo.fxflags = TEF_TRACER;
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// removeController()
//---------------------------------------------------------------------------
void ripgun_removeController(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
    ripgun_t *ripgun=(ripgun_t *)self->owner->curWeapon;
	winfoStopLooped(self->owner);
	if (ripgun)
	{
		ripgun->controller = NULL;
		ripgun->stopfiring = false;
	}
    gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
//	weapon_slugger
//---------------------------------------------------------------------------
void weapon_ripgun(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_slugger
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_ripgun(userEntity_t *self)
{
    //	set className so we can use gstate->InventoryFindItem...
    self->className = winfo->ammoName;

    winfoAmmoSetSpawn (winfo, self, 50, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_ripgun_register_func()
{
	gstate->RegisterFunc("ripgunshootThink",ripgun_shootThink);
	gstate->RegisterFunc("ripgunuse",ripgun_use);
	gstate->RegisterFunc("ripgunremoveController",ripgun_removeController);
	gstate->RegisterFunc("slug_think",ripgun_slug_think);
	gstate->RegisterFunc("ripgun_give",ripgun_give);
	gstate->RegisterFunc("ripgun_select",ripgun_select);
}
