//---------------------------------------------------------------------------
// NOVABEAM
//---------------------------------------------------------------------------
#include "weapons.h"
#include "novabeam.h"

enum
{
    // model defs
    MDL_START,
    MDL_MUZZLEFLASH,

    // sound defs
    SND_START,
    SND_BARRELIN,
    SND_CLICK,

    // frame defs
    ANIM_START,
    ANIM_UNSHOOT
};

// weapon info def
weaponInfo_t novabeamWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NOVABEAM),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NOVABEAM_AMMO),
	// names, net names, icon
	"weapon_novabeam",
	tongue_weapons[T_WEAPON_NOVABEAM],
	"ammo_novabeam",
	tongue_weapons[T_WEAPON_NOVABEAM_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_novabeam.dkm",
		"models/e4/a_nova.dkm",
		"models/e4/wa_nova.dkm",
		"models/e4/we_mfnbeam.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_novaready.wav",
		"e4/we_novaaway.wav",
		"e4/we_novafirea.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e4/we_novafireb.wav",
		"e4/we_glockclick.wav",
		SND_NOSOUND,
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
		-1,"shootb",0,0,
		0,	
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_NOVABEAM][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_5,
		wpcmds[WPCMDS_NOVABEAM][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_5,
		NULL
	},

	weapon_give_5,
	weapon_select_5,
	novabeam_command,
	novabeam_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(novabeam_t),

	750,
	200,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	0,          //	speed
	100,        //	initial ammo
	2000.0,     //	range
	8.0,        //	damage
	3.0,        //	lifetime

	WFL_AI_TRACK_TARG|WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_PLAYER_ONLY,          //	flags

	WEAPONTYPE_SHOULDER,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_NOVABEAM],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_NOVABEAM),
	ATTACKER_VICTIM,

	WEAPON_NOVABEAM,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&novabeamWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *novabeam_give(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// novabeam_select()
//---------------------------------------------------------------------------
short novabeam_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *novabeam_command(struct userInventory_s *inv, char *commandStr, void *data)
{
    userEntity_t *self=(userEntity_t *)data;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used

    if ( !stricmp (commandStr, "ambient") )
    {     // no ambient for this weapon... delete this section when it gets one
    }
    else if ( !stricmp(commandStr, "change") )
    {
        novabeam_t *novabeam=(novabeam_t *)self->curWeapon;

        winfoAnimate(winfo,(userEntity_t *)data,ANIM_AWAY_STD,FRAME_ONCE|FRAME_WEAPON,.05);

        if ( novabeam->changekill & 1 )
        {
			RELIABLE_UNTRACK(self);
            if ( novabeam->changekill & 2 )
                novabeam->beam->remove (novabeam->beam);
        }
    }
    else
        winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void novabeam_use(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    novabeam_t *novabeam=(novabeam_t *)self->curWeapon;
    CVector vec(0,0,0);

    if ( !weaponHasAmmo(self,true) )
	{
        winfoPlay(self,SND_CLICK,1.0f);
        return;
	}

    winfoPlay(self,SND_SHOOT_STD,1.0f);
//   winfoPlay(self,SND_BARRELIN,1.0f);
/*
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,0.05);
    novabeam->killbeam=gstate->time + winfo->lifetime;
    phook->attack_finished=novabeam->killbeam+.2;
*/
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,0.05);

	float lifetime = winfo->lifetime;
    if(phook->attack_boost)
		lifetime /= ((phook->attack_boost+1) * 0.5);
    novabeam->killbeam=gstate->time + lifetime;
    phook->attack_finished=novabeam->killbeam+.2;

    phook->fxFrameNum=winfoLastFrame(ANIM_SHOOT_STD);
    phook->fxFrameFunc=novabeam_startLaser;
	WEAPON_FIRED(self);
	weapon_PowerSound(self);
}

//---------------------------------------------------------------------------
// startLaser()
//---------------------------------------------------------------------------
void novabeam_startLaser(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    novabeam_t *novabeam=(novabeam_t *)self->curWeapon;
    userEntity_t *laser,*went;

    // spawn laser
    laser=gstate->SpawnEntity();
	laser->flags |= FL_NOSAVE;
	laser->className = "Novabeam";
    laser->prethink=novabeam_think;
    laser->owner=self;
    laser->flags=RF_TRACKENT;        // drawn by client
    laser->s.effects=EF_GIB;         // not really GIBS, makes server send to clients
    laser->s.renderfx=RF_TRANSLUCENT|RF_SHAKE;
	laser->userHook = gstate->X_Malloc(sizeof(novaHook_t),MEM_TAG_HOOK);
    novaHook_t *hook=(novaHook_t *)laser->userHook;

    //	set the beam diameter - starting radius is encoded in upper 8 bits
    //	ending radius in the lower 8 bits -- this is a hack to make the network
    //	messages be a little faster
    laser->s.frame = 2;
    laser->s.frame = (laser->s.frame << 8) + laser->s.frame;

    entAnimate(laser,laser->s.frame,laser->s.frame,FRAME_ONCE,.05);     // so it's actually sent to CL

    novabeam->endeffect=false;
    novabeam->changekill=3;
    novabeam->beam=laser;
	novabeam->burst = winfo->damage;
    novabeam_think(laser);

    if (went=(userEntity_t *)self->client->ps.weapon)
		went->s.renderfx |= RF_SHAKE;

    phook->fxFrameFunc=NULL;

	// set up the effects track entity
    trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));

	weaponTrackMuzzle(tinfo,laser,self,winfo,true);
	tinfo.modelindex = gstate->ModelIndex( "models/e4/we_mfnbeam.sp2" );		// muzzle flash sprite
	tinfo.scale2 = (float)gstate->ModelIndex("models/global/e_florange.sp2");	// flare for the end of the laser
	tinfo.numframes = 1 |TEF_ANIM2_THIRD|TEF_ANIM2_FIRST|TEF_ANIM2_LOOP|TEF_ANIM2_BILLBOARD;		// set up the tef_animate2 stuff
	tinfo.frametime = 0.05;
	tinfo.scale = 0.25;
    tinfo.renderfx=RF_NOVALASER;
	tinfo.flags |= TEF_MODELINDEX|TEF_SCALE2|TEF_NUMFRAMES|TEF_FRAMETIME|TEF_SCALE;
    com->trackEntity(&tinfo,MULTICAST_PVS);

    novabeam->weapon.lastFired=gstate->time;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void novabeam_think(userEntity_t *self)
{
    userEntity_t *owner=self->owner,*went=(userEntity_t *)owner->client->ps.weapon;;
    playerHook_t *phook=(playerHook_t *)owner->userHook;
    novabeam_t *novabeam=(novabeam_t *)owner->curWeapon;
//    weapon_t *weapont=(weapon_t *)owner->curWeapon;// SCG[1/24/00]: not used
    CVector start,end, dir, offset, ang;
    trace_t tr;
    float timeLeft;
    short ammo;

    // player dead?
    if ( self->owner->curWeapon == NULL )
    {
        RELIABLE_UNTRACK(self);
        self->remove (self);
        return;
    }

    tr.endpos = phook->autoAim.shootpos;
    tr.ent=phook->autoAim.ent;

	// gotta tweak with the damage a bit..attack boost causes the discharge to happen more quickly so it will do
	// less damage...pump it up a tad.

	float lifetime = winfo->lifetime;
    if(phook->attack_boost)
	{
		lifetime /= ((phook->attack_boost+1) * 0.5);
	}

    timeLeft=novabeam->killbeam-gstate->time + 0.1;
	timeLeft /= lifetime;
//	timeLeft *= winfo->lifetime;
/*
	float damage = (timeLeft)*winfo->damage;

    if(phook->attack_boost >= 1)
	{
		damage *= (1.0 + (phook->attack_boost - 1) * 0.25);
	}

	if (damage < 0) damage = 0;
*/

	// how much of the burst will we do?
	float frac = phook->attack_boost * 0.10;
	if (frac < 0.2) frac = 0.2;

	// find the damage and decrement the available burst
	float damage = novabeam->burst * frac;
	novabeam->burst -= damage;
	if (damage < 0) damage = 0;
	if (novabeam->burst < 0) novabeam->burst = 0;

//	gstate->Con_Dprintf ("time left:%f, damage:%f\n", timeLeft, damage);

    ammo = weaponUseAmmo(owner, false);

    // kill beam??
    if ( timeLeft <= .5 || !ammo )
    {
        if ( !novabeam->endeffect )
        {
            winfoStopLooped(owner);

            novabeam->endeffect=true;

            // stop shaking weapon
			if (went)
				went->s.renderfx &= ~RF_SHAKE;

            // stop tracking
            RELIABLE_UNTRACK(self);
            novabeam->changekill &= ~1;
            novabeam->killbeam=gstate->time;
        }
        else
        {
            if ( timeLeft <= 0 )
            {
                // kill entity
                winfoAnimate(winfo,self->owner,ANIM_UNSHOOT,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,0.05);
				winfoPlay(self->owner,SND_BARRELIN,1.0f);
                self->remove (self);
                novabeam->changekill &= ~2;
                novabeam->beam=NULL;
            }
        }
        return;
    }

    self->s.alpha=timeLeft + 0.1;
    if ( self->s.alpha > 1 )
        self->s.alpha=1;

    // set start/end positions
    dir=owner->client->v_angle;
    dir.AngleToVectors(forward,right,up);
    start=owner->s.origin+CVector(0,0,20.6)+weaponHandVector(owner,forward*14,right*6);

    // using auto-aim entity?
    if ( tr.ent && tr.ent->takedamage)
    {
        CVector dir2;

        dir=tr.ent->s.origin-owner->s.origin;
		dir.Normalize();
		forward = dir;
    }

    end=start+forward*2000;
//    else
//        tr=gstate->TraceBox_q2(start,self->s.mins,self->s.maxs,end,owner,MASK_SHOT);

	tr = gstate->TraceLine_q2 (start, end, self, MASK_SHOT);

    self->s.origin = start;
    end=tr.endpos;
    self->s.old_origin = end;
    self->movedir = dir;
    gstate->LinkEntity(self);

    // hit anything?
    if ( tr.ent && tr.ent->takedamage)
    {
		novaHook_t *hook = (novaHook_t *)self->userHook;
		if (hook)
			hook->hits.AddHit(self->owner,tr.ent);
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

        com->Damage (tr.ent, self, self->owner, tr.endpos, forward, damage, DAMAGE_NONE);
	}

	// make the cool beam effect and muzzle flash stuff happen!
    trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));
	weaponTrackMuzzle(tinfo,self,owner,NULL,true);

	tinfo.dstpos = end;
    tinfo.renderfx=RF_NOVALASER;
	tinfo.fxflags =	TEF_ADDFX|TEF_ANIMATE2|TEF_FX_ONLY;
	tinfo.flags |= TEF_DSTPOS|TEF_FXFLAGS;
    com->trackEntity(&tinfo,MULTICAST_PVS);

	self->nextthink = gstate->time + 0.1;
}

//---------------------------------------------------------------------------
//	weapon_novabeam
//---------------------------------------------------------------------------
void weapon_novabeam(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_novabeam
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_novabeam (userEntity_t *self)
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
void weapon_novabeam_register_func()
{
	gstate->RegisterFunc("novabeamuse",novabeam_use);
	gstate->RegisterFunc("novabeamthink",novabeam_think);
	gstate->RegisterFunc("novabeam_give",novabeam_give);
	gstate->RegisterFunc("novabeam_select",novabeam_select);
}
