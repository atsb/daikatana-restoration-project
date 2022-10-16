//---------------------------------------------------------------------------
// SLUGGER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "slugger.h"

enum
{
    // model defs
    MDL_START,
    MDL_GRENADE,
//    MDL_CORDITE,
    MDL_MUZZLEFLASH,
    MDL_CORDITE_RING,

    // sound defs
    SND_START,
    SND_CORDITE_LAUNCH,
    SND_CORDITE_HIT1,
    SND_CORDITE_HIT2,
    SND_CORDITE_HIT3,
    SND_CORDITE_HIT4,
    SND_CORDITE_HIT5,
    SND_CORDITE_HIT6,
    SND_RIPGUN_MODE1,
    SND_RIPGUN_MODE2,
	SND_CLICK,

    // frame defs
    ANIM_START,
    ANIM_READY2,
    ANIM_SHOOT2,
    ANIM_CORDITE_RING,
	ANIM_CORD_COCK
};

// weapon info def
weaponInfo_t sluggerWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SLUGGER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SLUGGER_AMMO),
	// names, net names, icon
	"weapon_slugger",
	tongue_weapons[T_WEAPON_SLUGGER],
	"ammo_slugger",
	tongue_weapons[T_WEAPON_SLUGGER_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_slugger.dkm",
		"models/e4/a_slug.dkm",
		"models/e4/wa_rip.dkm",
		"models/e4/we_ripgren.dkm",
		//        "models/e4/wa_rip2.dkm",
		"models/e4/we_mfripg.sp2",
		"models/global/we_shotring.sp2",
		"models/e4/a_cslug.dkm",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_ripgunready.wav",
		"e4/we_ripgunaway.wav",
		"e4/we_ripgunshootc.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e4/we_ripgunshootb.wav",
		"e4/we_ripgunhita.wav",
		"e4/we_ripgunhitb.wav",
		"e4/we_ripgunhitc.wav",
		"e4/we_ripgunhitd.wav",
		"e4/we_ripgunhite.wav",
		"e4/we_ripgunhitf.wav",
		"e4/we_ripgchangea.wav",
		"e4/we_ripgunshoota.wav",
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
		-1,"ready",0,0,
		-1,"shootb",0,0,
		MDL_CORDITE_RING, "sp2", 0, 0,
		MDL_WEAPON_STD,"ambb",0,0,
		0,
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_SLUGGER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_2,
		wpcmds[WPCMDS_SLUGGER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_2,
		NULL
	},

	weapon_give_2,
	weapon_select_2,
	weapon_command,
	slugger_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(weaponPulse_t),

	200,
	200,      //	ammo_max (laser)
	1,        //	ammo_per_use 
	1,        //	ammo_display_divisor
	40,       //	initial ammo
	1000.0,   //	speed
	1000.0,   //	range
	80.0,     //	damage
	1.25,     //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_SAME_WEAPON_MASK,        //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_SLUGGER],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_SLUGGER),
	ATTACKER_VICTIM,

	WEAPON_SLUGGER,
	0,
	NULL
};

// weapon info def
weaponInfo_t corditeWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SLUGGER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_RIPGUN_AMMO),
    // names, net names, icon
    "weapon_slugger",
    tongue_weapons[T_WEAPON_SLUGGER],
    "ammo_cordite",
    tongue_weapons[T_WEAPON_RIPGUN_AMMO],		// cordite grenades...
    "iconname",

    // model files
    {"models/e4/w_slugger.dkm",
        "models/e4/a_cslug.dkm",
        "models/e4/wa_rip2.dkm",
        "models/e4/we_ripgren.dkm",
//        "models/e4/wa_rip2.dkm",
        "models/e4/we_mfripg.sp2",
        "models/global/we_shotring.sp2",
        NULL},

    // sound files
    {
		SND_NOSOUND,
        "e4/we_ripgunready.wav",
        "e4/we_ripgunaway.wav",
        "e4/we_ripgunshootc.wav",
        SND_NOSOUND,
        SND_NOSOUND,
        SND_NOSOUND,
        "e4/we_ripgunshootb.wav",
        "e4/we_ripgunhita.wav",
        "e4/we_ripgunhitb.wav",
        "e4/we_ripgunhitc.wav",
        "e4/we_ripgunhitd.wav",
        "e4/we_ripgunhite.wav",
        "e4/we_ripgunhitf.wav",
        "e4/we_ripgchangea.wav",
        "e4/we_ripgchangeb.wav",
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
        -1,"ready",0,0,
        -1,"shootb",0,0,
        MDL_CORDITE_RING, "sp2", 0, 0,
		MDL_WEAPON_STD,"ambb",0,0,
        0,
		NULL
		},

    // commands
    {  
		wpcmds[WPCMDS_SLUGGER][WPCMDS_GIVE], (void (*)(struct edict_s *))slugger_give,
		wpcmds[WPCMDS_SLUGGER][WPCMDS_SELC], (void (*)(struct edict_s *))slugger_select,
		NULL
    },

    slugger_give,
    slugger_select,
    weapon_command,
    slugger_use,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(weaponPulse_t),

    200,
    15,      //	ammo_max (laser)
    1,        //	ammo_per_use 
    1,        //	ammo_display_divisor
    2,       //	initial ammo
    700.0,   //	speed
    150.0,   //	range
    150.0,     //	damage
    3,     //	lifetime

    WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_SAME_WEAPON_MASK,        //	flags

    WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_SLUGGER],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_SLUGGER),
	ATTACKER_VICTIM,

    WEAPON_SLUGGER,
    1,
    NULL
};

// local weapon info access
static weaponInfo_t *swinfo=&sluggerWeaponInfo;		// uhh, slugs?
static weaponInfo_t *cwinfo = &corditeWeaponInfo;	// cordite!
static weaponInfo_t *winfo = swinfo;				// just for anims and sounds...this data is the same.

static   float flash_distance;
static   float flash_delta_yaw;  // difference between entities view angle and angle to explosion

//---------------------------------------------------------------------------
// slugger_give()
// 00000000 00000000 00000000 00000000
// mccccccc cccccccc ssssssss ssssssss
//---------------------------------------------------------------------------
userInventory_t *slugger_give (userEntity_t *self, int ammoCount)
{
	unsigned int cordite = (((unsigned int)ammoCount) >> 16) & 0X0000FFFF;
	unsigned int slugs = ((unsigned int)ammoCount) & 0x0000ffff;

    weaponPulse_t *slugger;

	if (!(deathmatch->value && dm_instagib->value))
	{
		weaponGive(self,cwinfo,cordite);
	}

    slugger=(weaponPulse_t *)weaponGive (self, swinfo, slugs);

    if ( slugger && slugger->mode!=0 && slugger->mode!=1 )
        slugger->mode = 0;

    return((userInventory_t *)slugger);
}


short try_select_cordite(userEntity_t *self, ammo_t *ammo, short force, bool doubleCock)
{
	if (!ammo && !force)
		return WEAPON_UNAVAILABLE;

	if (ammo && ammo->count == 0)
		return WEAPON_UNAVAILABLE;

	weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(self->inventory,cwinfo->weaponName);	
	if (sluggerInv)
	{
		sluggerInv->weapon.winfo = cwinfo;
		sluggerInv->weapon.ammo = ammo;
		sluggerInv->weapon.modelIndex = gstate->ModelIndex(cwinfo->models[MDL_ARTIFACT_STD]);
		if (self->flags & FL_CLIENT)
			self->client->needsUpdate |= HUD_UPDATE_WEAP;
	}
	else
		return WEAPON_UNAVAILABLE;

	short return_value = weaponSelect(self,cwinfo);

	if (return_value == WEAPON_CHANGED)
	{
		// gotta get this info after the weapon changes!!!
		if (doubleCock)
			winfoAnimate(swinfo,self,ANIM_CORD_COCK,FRAME_ONCE|FRAME_WEAPON,.05);

		winfoPlay(self,SND_RIPGUN_MODE2,1.0);
		sluggerInv->mode = sluggerInv->mode = 1;
	}

	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook = (playerHook_t *)self->userHook;
		if (phook)
			phook->attack_finished = gstate->time + 0.3;
	}

	return return_value;
}

short try_select_slugger(userEntity_t *self, ammo_t *ammo, short force)
{
	if (!ammo && !force)
		return WEAPON_UNAVAILABLE;

	if (ammo && ammo->count == 0)
		return WEAPON_UNAVAILABLE;

	weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(self->inventory,swinfo->weaponName);	
	if (sluggerInv)
	{
		sluggerInv->weapon.winfo = swinfo;
		sluggerInv->weapon.ammo = ammo;
		sluggerInv->weapon.modelIndex = gstate->ModelIndex(swinfo->models[MDL_ARTIFACT_STD]);
		if (self->flags & FL_CLIENT)
			self->client->needsUpdate |= HUD_UPDATE_WEAP;
	}
	else
		return WEAPON_UNAVAILABLE;

	short return_value = weaponSelect(self,swinfo);

	if (return_value == WEAPON_CHANGED)
	{
		// gotta get this info after the weapon changes!!!
		winfoPlay(self,SND_RIPGUN_MODE1,1.0);
		sluggerInv->mode = sluggerInv->mode = 0;
	}

	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook = (playerHook_t *)self->userHook;
		if (phook)
			phook->attack_finished = gstate->time + 0.3;
	}

	return return_value;
}
	
//---------------------------------------------------------------------------
// slugger_select()
//---------------------------------------------------------------------------
short slugger_select(userEntity_t *self)
{
	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook = (playerHook_t *)self->userHook;
		if (phook)
		{
			if (gstate->time < phook->attack_finished)
				return WEAPON_UNAVAILABLE;
		}

		weaponPulse_t	*slugger;
		short			return_value;
		ammo_t			*ammos,*ammoc;

		if (!self->curWeapon)
			return weaponSelect(self,&sluggerWeaponInfo);		// select the slugger first

		slugger = (weaponPulse_t *) self->curWeapon;
		weapon_t *weap = &slugger->weapon;

		// get the ammo from the inventory
		ammoc = (ammo_t *)gstate->InventoryFindItem (self->inventory, cwinfo->ammoName);
		ammos = (ammo_t *)gstate->InventoryFindItem (self->inventory, swinfo->ammoName);

		short isSlugger = (stricmp(weap->winfo->ammoName,swinfo->ammoName) == 0) ? 1 : 0;
		short isCordite = (stricmp(weap->winfo->ammoName,cwinfo->ammoName) == 0) ? 1 : 0;

		if (!isSlugger && !isCordite)	// neither are currently selected...
		{
			return_value = try_select_slugger(self,ammos,1);
			if ((return_value == WEAPON_CHANGED) || (return_value == WEAPON_ACTIVE))
				return return_value;

			return_value = try_select_cordite(self,ammoc,0,false);
			return return_value;
		}
		else if (isSlugger)				// slugger is currently selected
		{
			return_value = try_select_cordite(self,ammoc,0,true);
		}
		else							// cordite is currently selected
		{
			return_value = try_select_slugger(self,ammos,0);
		}

		return return_value;
	}
	else
		return weaponSelect(self,swinfo);
}

//---------------------------------------------------------------------------
// weapon_command()
//---------------------------------------------------------------------------
void *weapon_command(struct userInventory_s *ptr, char *commandStr, void *data)
{
    weaponPulse_t   *weapon = (weaponPulse_t *) ptr;
    userEntity_t *self=(userEntity_t *)data;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used

    if ( !stricmp (commandStr, "force_mode") )
    {
        //	force the mode of the ripgun
        weapon->mode = *(int *) data;
    }
    else if ( !stricmp (commandStr, "set_ammo") )
    {
        weapon->weapon.ammo = (ammo_t *) data;
    }
	else
	{
		if (weapon && (weapon->weapon.winfo == cwinfo))
			winfoGenericCommand(cwinfo,ptr,commandStr,data);
		else
			winfoGenericCommand(swinfo,ptr,commandStr,data);
	}

    return(NULL);
}

//---------------------------------------------------------------------------
// entity_isin_fov()
//---------------------------------------------------------------------------
float entity_isin_fov(userEntity_t *self, userEntity_t *target)
{
    CVector     dir, temp;
    float       target_yaw, dir_yaw;
//    corditeHook_t   *hook = (corditeHook_t *) self->userHook;// SCG[1/24/00]: not used

    // can the entity see the grenade at all?
    gstate->TraceLine(self->s.origin, target->s.origin, TRUE, self, &trace);

    // can't see it, so leave
    if ( trace.fraction != 1.0 )
        return FALSE;

    dir = self->s.origin - target->s.origin;
    flash_distance = dir.Length();
    dir.Normalize ();

    // if 250 units or more away, no effect
    dir_yaw = VectorToYaw (dir);
    target_yaw = AngleMod (target->s.angles [1]);

    flash_delta_yaw = target_yaw - dir_yaw;

    // keep angles relative and within 180 degrees
    while ( flash_delta_yaw >= 180.0 )
        flash_delta_yaw -= 360.0;

    flash_delta_yaw = fabs (flash_delta_yaw);

    return TRUE;
}

//---------------------------------------------------------------------------
// cordite_remove ()
//
//	cleanup for cordite grenade entity
//---------------------------------------------------------------------------
void    cordite_remove (userEntity_t *self)
{
    corditeHook_t   *hook = (corditeHook_t *) self->userHook;

    // free the friction hook
    com->friction_remove (hook->fhook);

    gstate->RemoveEntity (self);
}

//---------------------------------------------------------------------------
// cordite_explode()
//---------------------------------------------------------------------------
void cordite_explode(userEntity_t *self)
{
//    playerHook_t    *phook = (playerHook_t *) self->owner->userHook;// SCG[1/24/00]: not used
//    corditeHook_t   *hook = (corditeHook_t *) self->userHook;// SCG[1/24/00]: not used

    //The cordite does hurt but not that much.
	// cek[1-24-00]: set the attackweapon
	gstate->attackWeap = cwinfo->weaponName;

    int num = com->RadiusDamage (self, self->owner, self, cwinfo->damage, DAMAGE_EXPLOSION, cwinfo->range);
	WEAPON_HIT(self->owner,num);

    spawn_sprite_explosion(self,self->s.origin,false);
    spawnPolyExplosion(self->s.origin, self->s.angles, 2, 400, CVector(1,1,0.5), PEF_SOUND|PEF_ANGLES);

//    flashClients(self,.8,.8,.5,.3);

    self->remove (self);
}

//---------------------------------------------------------------------------
// cordite_touch()
//---------------------------------------------------------------------------
void cordite_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
	corditeHook_t   *hook = (corditeHook_t *) self->userHook;
	CVector     temp, forward, right, up;

	if (!hook)
		return;

	if ( other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_MONSTER )
	{
		cordite_explode (self);
		return;
	}

	temp = hook->last_pos - self->s.origin;

	hook->last_pos = self->s.origin;    

	// we just touched less than 1/10 of a second ago, so don't play bounce noise again
	if ( gstate->time < hook->last_touch_time + 0.1 )
		return;

	if (!self->groundEntity)
		winfoPlayAttn(self, SND_CORDITE_HIT1 + (int)(frand() * 6), 1.0f, 256, 1000);

    if ( self->velocity.Length() != 0.0 )
	{
	    self->avelocity.x = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
	    self->avelocity.y = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
	    self->avelocity.z = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
	}
    

	hook->last_touch_time = gstate->time;
}

//---------------------------------------------------------------------------
// cordite_think()
//---------------------------------------------------------------------------
void cordite_think(userEntity_t *self)
{
    corditeHook_t       *hook = (corditeHook_t *) self->userHook;
    CVector             vecDistance, vecForwardAngles, vecColor(1.0f, 0.8f, 1.0f);
    int                 iPointContents;

	if (!hook)
		return;

    if( (iPointContents = gstate->PointContents(self->s.origin)) & MASK_WATER)
    {
        self->velocity.x = self->velocity.x * 0.5;
        self->velocity.y = self->velocity.y * 0.5;
        if(fabs(self->velocity.z) > 50)
            self->velocity.z = self->velocity.z * 0.5;
    }

    if ( gstate->time >= hook->remove_time )
        cordite_explode (self);
    else if ( gstate->time >= hook->remove_time - 0.5f && gstate->time < hook->remove_time)
    {
        self->s.effects &= ~EF_SMOKETRAIL;
        self->nextthink = gstate->time + 0.1;
    } 
    else
    {
        vecDistance = self->s.origin - self->owner->s.origin;
        if(hook && hook->iRingCount < 3)
        {
            self->velocity.VectorToAngles(vecForwardAngles);
            winfoClientScaleAlpha(winfo,self->s.origin,vecForwardAngles,.1,MDL_CORDITE_RING, ANIM_CORDITE_RING,.76,.01,55,vecColor);
            hook->iRingCount++;
        } 
        else if(self->movetype != MOVETYPE_BOUNCE)
        {
            self->movetype = MOVETYPE_BOUNCE;
        }
        self->nextthink = gstate->time + 0.1;
        self->think = cordite_think;
    }
}

void cordite_reload(userEntity_t *self)
{
    playerHook_t    *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

	phook->fxFrameFunc = NULL;
	phook->fxFrameNum = -1;

    weaponPulse_t *weapon = (weaponPulse_t *) self->curWeapon;
    ammo_t *ammo = (ammo_t *)weapon->weapon.ammo;

	if (!stricmp(ammo->name,cwinfo->ammoName))
	{
		winfoAnimate(swinfo,self,ANIM_CORD_COCK,FRAME_ONCE|FRAME_WEAPON,.05);
//		winfoPlay(self,SND_RIPGUN_MODE2,1.0);
	}
}

//---------------------------------------------------------------------------
// cordite_fire_grenade()
//---------------------------------------------------------------------------
void cordite_fire_grenade(userEntity_t *self)
{
    userEntity_t    *grenade;
    corditeHook_t      *hook;
    playerHook_t    *phook=(playerHook_t *)self->userHook;
    weaponPulse_t       *grenadep;
//    ammo_t          *ammo;// SCG[1/24/00]: not used

    grenadep = (weaponPulse_t *) self->curWeapon;
//    ammo = (ammo_t *)grenadep->weapon.ammo;// SCG[1/24/00]: not used

    CVector offset( 10, 5, 15);

    grenade = weapon_spawn_projectile (self, offset, cwinfo->speed, -5/(phook->attack_boost+1), "projectile_cordite");
    grenade->movetype = MOVETYPE_BOUNCEMISSILE;
	grenade->flags |= FL_EXPLOSIVE;

    // set up userHook
	grenade->userHook = gstate->X_Malloc(sizeof(corditeHook_t),MEM_TAG_HOOK);
    hook = (corditeHook_t *) grenade->userHook;

    // set up frictionHook
    hook->fhook = com->friction_init (self, hook->fhook);
    hook->iRingCount = 0;
    com->friction_set_physics (self, hook->fhook, 1.0, 1.0, 500.0, 0);

    grenade->s.angles.x = self->s.angles.x;
    grenade->elasticity = 0.6;
    grenade->s.effects |= EF_SMOKETRAIL;
    winfoSetModel(grenade,MDL_GRENADE);
    gstate->SetSize (grenade,-4,-4,-4,4,4,12);
    //gstate->SetOrigin (grenade, grenade->s.origin [0], grenade->s.origin [1], grenade->s.origin [2]);

    grenade->touch = (touch_t)cordite_touch;
    grenade->nextthink = gstate->time + 0.08;
    grenade->think = cordite_think;
    grenade->remove = cordite_remove;

    grenade->avelocity.x = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
    grenade->avelocity.y = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
    grenade->avelocity.z = rnd() > 0.5 ? rnd()*100 : -rnd()*100;

    hook->remove_time = gstate->time + cwinfo->lifetime;

	phook->fxFrameFunc = cordite_reload;
	phook->fxFrameNum = winfoLastFrame(ANIM_SHOOT2) - 9;
}

//---------------------------------------------------------------------------
// ripgun_use()
//---------------------------------------------------------------------------
void    slugger_use (struct userInventory_s *ptr, userEntity_t *self)
{
    CVector         temp, end;
    weaponPulse_t       *slugger;


    if ( !weaponHasAmmo(self,true) )
	{
        winfoPlay(self,SND_CLICK,1.0f);
        return;
	}

	if (self->flags & FL_BOT)
	{
		weaponUseAmmo (self, true);
		slugger_shootShrapnel(self);
		return;
	}	

	weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(self->inventory,cwinfo->weaponName);
	if (sluggerInv)
	{
		if (sluggerInv->weapon.winfo == swinfo)//(sluggerInv->mode == 0)
		{
			if (sluggerInv->mode != 0)//(sluggerInv->weapon.winfo != swinfo)
				sluggerInv->mode = 0;
		}
		else
		{
			if (sluggerInv->mode != 1)
				sluggerInv->mode = 1;
		}
	}

    weaponUseAmmo (self, true);

	WEAPON_FIRED(self);
//    phook = (playerHook_t *) self->userHook;// SCG[1/24/00]: not used
    slugger = (weaponPulse_t *) self->curWeapon;

    if ( slugger->mode != 0 )
    {
        winfoPlay(self, SND_CORDITE_LAUNCH, 1.0f);
        winfoAnimate(winfo,self,ANIM_SHOOT2,FRAME_ONCE|FRAME_WEAPON,.05);
        cordite_fire_grenade(self);

		if (!weaponHasAmmo(self,false))		// try to set up the slugger ammo
		{
			// get the ammo from the inventory
			ammo_t *ammos = (ammo_t *)gstate->InventoryFindItem (self->inventory, swinfo->ammoName);
			if (ammos && ammos->count)
			{
				weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(self->inventory,swinfo->weaponName);	
				if (sluggerInv)
				{
					sluggerInv->weapon.winfo = swinfo;
					sluggerInv->weapon.ammo = ammos;
					sluggerInv->mode = 0;
					slugger->weapon.modelIndex = gstate->ModelIndex(swinfo->models[MDL_ARTIFACT_STD]);
					if (self->flags & FL_CLIENT)
						self->client->needsUpdate |= HUD_UPDATE_WEAP;
				}
			}

		}
    }
    else
    {
        winfoAnimate(winfo,self,ANIM_SHOOT2,FRAME_ONCE|FRAME_WEAPON,.05);
        winfoPlay(self, SND_SHOOT_STD, 1.0f);
        slugger_shootShrapnel(self);
		if (!weaponHasAmmo(self,false))		// try to set up the cordite ammo
		{
			// get the ammo from the inventory
			ammo_t *ammoc = (ammo_t *)gstate->InventoryFindItem (self->inventory, cwinfo->ammoName);
			if (ammoc && ammoc->count)
			{
				weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(self->inventory,swinfo->weaponName);	
				if (sluggerInv)
				{
					sluggerInv->weapon.winfo = cwinfo;
					sluggerInv->weapon.ammo = ammoc;
					sluggerInv->mode = 1;
					slugger->weapon.modelIndex = gstate->ModelIndex(cwinfo->models[MDL_ARTIFACT_STD]);
					if (self->flags & FL_CLIENT)
						self->client->needsUpdate |= HUD_UPDATE_WEAP;
				}
			}

		}
    }
}   

//---------------------------------------------------------------------------
// shootShrapnel()
//---------------------------------------------------------------------------
void slugger_shootShrapnel(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    userEntity_t *weapon=(userEntity_t *)self->client->ps.weapon;
    trace_t     tr, trShot;
    CVector     vecMuzzle, vecDir;
    int         iPellet;

    vecMuzzle.Set(swinfo->fWeaponOffsetX1, swinfo->fWeaponOffsetZ1, swinfo->fWeaponOffsetY1);
    tr = weaponTraceShot(self, vecMuzzle, 0);
    vecMuzzle += self->s.origin;
    vecDir = tr.endpos - vecMuzzle;
    vecDir.Normalize();
//	weaponScorchMark( tr, SM_BULLET );
    // trace pellets from the ripgun

/*	weaponMuzzleFlash(	self,self,winfo,					// thing tracked, player, weapon info
						"models/global/genflash.dkm",		// flash model (modelindex)
						1, 0.05,							// model frame count, frame time
						2.3,								// scale
						175 + 75*frand(),					// light value (may be 0)
						CVector(0.8,0.4,0.2),				// light color (ignored if no lightsize)
						TEF_ANIM2_THIRD|TEF_ANIM2_FIRST|TEF_ANIM2_SMOKE,					// any of TEF_AMIN2_* animation flags
						0);						// additional fxflags (TEF_NOROTATE)*/
	weaponMuzzleFlash(	self,self,winfo,FLASH_SLUGGER);


	float totalDamage = 0;
	userEntity_t *damagedEnt = NULL;
    for(iPellet = 0; iPellet < 12; iPellet++)
    {
        vecDir = tr.endpos - vecMuzzle;
        vecDir.Normalize();
        vecDir.Set(vecDir.x + (crand() * 0.07), vecDir.y + (crand() * 0.07), vecDir.z + (crand() * 0.07));
        vecDir = vecMuzzle + vecDir * 4000;
        trShot = gstate->TraceLine_q2(vecMuzzle, vecDir, self, MASK_SHOT);
        if(trShot.fraction < 1.0f)
        {
            if(trShot.ent && trShot.ent->takedamage)
            {
				damagedEnt = trShot.ent;
                vecDir = trShot.endpos - vecMuzzle;
                vecDir.Normalize();
				totalDamage += swinfo->damage *  0.083;
                vecDir = vecMuzzle - trShot.endpos;
            } 
            else if(trShot.ent && trShot.ent->solid == SOLID_BSP)
            {
                vecDir.Normalize();
            }
        }
        
    }

	if (trShot.ent)
	{
		if(trShot.ent->takedamage)
		{
			vecDir = trShot.endpos - vecMuzzle;
			vecDir.Normalize();
			vecDir = vecMuzzle - trShot.endpos;
			weaponBloodSplat(trShot.ent, vecMuzzle, trShot.endpos, trShot.endpos, vecDir.Length());
		} 
		else if((trShot.ent->solid == SOLID_BSP) && !(trShot.contents & CONTENTS_CLEAR))
		{
			gstate->WriteByte( SVC_TEMP_ENTITY );	// we're makins less, so make them bigger
			gstate->WriteByte( TE_SCORCHMARK );
			gstate->WritePosition( trShot.endpos );
			gstate->WriteShort( trShot.plane.planeIndex );
			gstate->WriteShort( trShot.ent->s.modelindex );
			gstate->WriteShort( trShot.ent->s.number);
			gstate->WriteFloat( 4 );
			gstate->WriteFloat( 0 );
			gstate->WriteByte( SM_BULLET );
			gstate->WritePosition(trShot.plane.normal);
			gstate->MultiCast( zero_vector, MULTICAST_ALL );
//			weaponScorchMark( tr, SM_BULLET );
		}
	}

	if (!weapon)
		weapon = self;
	// Do all accumulated damage at once
	if( totalDamage && damagedEnt)
	{
		vecDir.Multiply(-1);	// it isn't a tractor beam.
		winfoAttackWeap;
		com->Damage(damagedEnt, weapon, self, trShot.endpos, vecDir, totalDamage, DAMAGE_INERTIAL);
		WEAPON_HIT(self,1);
	}

    invWeapon->lastFired=gstate->time;
}

//---------------------------------------------------------------------------
//	weapon_ripgun
//---------------------------------------------------------------------------
void weapon_slugger(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_ripgun
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_slugger(userEntity_t *self)
{
    //	set className so we can use gstate->InventoryFindItem...
    self->className = swinfo->ammoName;

    winfoAmmoSetSpawn (swinfo, self, 15, 30.0, ammoTouchSlugger);
}

//---------------------------------------------------------------------------
//	ammo_cordite
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_cordite (userEntity_t *self)
{
    //	set className so we can use gstate->InventoryFindItem...

    winfoAmmoSetSpawn (winfo, self, 4, 30.0, ammoTouchSlugger);
	if (!self->inuse || !stricmp(self->className,"freed"))
		return;

    self->className = cwinfo->ammoName;//"ammo_cordite";
    ammoTouchHook_t *hook=(ammoTouchHook_t *)self->userHook;
	if (!hook)
		return;

	// local copy so some crazy winfo macros will work
	weaponInfo_t *winfo = cwinfo;
    winfoSetModel(self,MDL_AMMO_STD);

    //	self->modelName stores the model name so it can be reset when
    //	the item respawns
    self->modelName=winfoModel(MDL_AMMO_STD);
	hook->winfo = cwinfo;
}

void    ammoTouchSlugger (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf)
{
    ammoTouchHook_t *hook = (ammoTouchHook_t *) self->userHook;
	if (!hook)
		return;

    ammo_t          *ammo = NULL;
    int             ammo_addcount = hook->count;
    weaponInfo_t    *winfo;

    if ( !(other->flags & (FL_CLIENT | FL_BOT)) )
        return;

	// get the right kind of winfo for this type of ammo...
	if (!stricmp(self->className,"ammo_cordite"))
		winfo = cwinfo;
	else
		winfo = swinfo;

	// get the ammo from the inventory
    ammo = (ammo_t *) gstate->InventoryFindItem (other->inventory, self->className);
    if (ammo)
    {
		ammo_addcount = (ammo->count + ammo_addcount <= winfo->ammo_max) ? (ammo_addcount) : (winfo->ammo_max - ammo->count);
    }

    //	add the ammo
	if (ammo_addcount)
		ammo = ammoAdd (other, ammo_addcount, hook->winfo);

	if (!ammo)
		return;

	// find the weapon...if it's currently a null ammo or out of ammo, fix that!
	weaponPulse_t *sluggerInv = (weaponPulse_t *)gstate->InventoryFindItem(other->inventory,cwinfo->weaponName);
	if (sluggerInv)
	{
		weapon_t *weapon = (weapon_t *)&sluggerInv->weapon;
		if (weapon && ammo)	// we SHOULD  have ammo here... just a sanity check...
		{
			// ammo not set		or	ammo set			but empty
			if ((!weapon->ammo) || (weapon->ammo && !weapon->ammo->count))
			{
				sluggerInv->weapon.winfo = winfo;
				sluggerInv->weapon.ammo = ammo;
				sluggerInv->mode = (winfo == swinfo) ? 0 : 1;
				sluggerInv->weapon.modelIndex = gstate->ModelIndex(winfo->models[MDL_ARTIFACT_STD]);
				if (self->flags & FL_CLIENT)
					self->client->needsUpdate |= HUD_UPDATE_WEAP;
			}
		}
	}
	
	// leave if we didn't add any ammo
    if (!ammo_addcount)
		return;

	if (ammo->winfo && (other->flags & FL_CLIENT))
	{
		//gstate->centerprint (other, 1.0, "%s %s.\n", tongue[T_PICKUP_WEAPON], ammo->winfo->ammoNetName);
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s %s.\n",2);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue,T_PICKUP_WEAPON));
		gstate->cs.SendStringID(ammo->winfo->ammoNameIndex);
		gstate->cs.Unicast(other, CS_print_center, 1.0);
		gstate->cs.EndSendString();
	}

	//	make ammo invisible
    self->s.modelindex = 0;

    //	com->SetRespawn does the following:
    //	In deathmatch:
    //	set weapon to SOLID_NOT, set think function to com->Respawn
    //	reset ambient volume if item has one (not working in Q2 yet)
    com->SetRespawn (self, self->delay);

    // play the ammo pickup sound
    gstate->StartEntitySound(other, CHAN_OVERRIDE, gstate->SoundIndex("global/i_c4ammo.wav"), 1.0, ATTN_NORM, 0.0);

    //	flash screen if this is a client
    if ( other->flags & FL_CLIENT )
        other->client->bonus_alpha = 0.25;

	if ( other->PickedUpItem )
		other->PickedUpItem( other, ammo->name );

	if (self->flags & FL_CLIENT)
		self->client->needsUpdate |= HUD_UPDATE_WEAP;

	WEAPON_DISPLAY_UPDATE(other);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_slugger_register_func()
{
	gstate->RegisterFunc("ripguncordite_think",cordite_think);
	gstate->RegisterFunc("ripgun_use",slugger_use);
	gstate->RegisterFunc("ripguncordite_remove",cordite_remove);
	gstate->RegisterFunc("ripguncordite_touch",cordite_touch);
	gstate->RegisterFunc("ammoTouchSlugger",ammoTouchSlugger);
	gstate->RegisterFunc("slugger_give",slugger_give);
	gstate->RegisterFunc("slugger_select",slugger_select);
}
