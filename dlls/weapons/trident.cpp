//---------------------------------------------------------------------------
// POSEIDONS TRIDENT
//---------------------------------------------------------------------------
#include "weapons.h"
#include "trident.h"

enum
{
    // model defs
    MDL_START,
    MDL_TIP,
    MDL_MUZZLEFLASH,

    // sound defs
    SND_START,
    SND_TIPBOOST,
	SND_LIGHTNING1,
	SND_LIGHTNING2,
	SND_EXPLODE,

    // frame defs
    ANIM_START,
    ANIM_MUZZLEFLASH
};

// weapon info def
weaponInfo_t tridentWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_TRIDENT),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_TRIDENT_AMMO),
	// names, net names, icon
	"weapon_trident",
	tongue_weapons[T_WEAPON_TRIDENT],
	"ammo_tritips",
	tongue_weapons[T_WEAPON_TRIDENT_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_trident.dkm",
		"models/e2/a_tri.dkm",
		"models/e2/wa_trident.dkm",
		"models/e2/we_tritip.dkm",
		"models/e2/we_mftrdnt.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e2/we_tridentready.wav",
		"e2/we_tridentaway.wav",
		"e2/we_tridentfirea.wav",
		"e2/we_tridentamba.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"global/e_lightningb.wav",
		"global/e_lightningc.wav",
		"global/e_wexplodee.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoot",0,0,
		-1,"amba",0,0,
		ANIM_NONE,      
		ANIM_NONE,      
		MDL_MUZZLEFLASH,"sp2",0,0,
		0,
		NULL
	},

	// commands
	{
		wpcmds[WPCMDS_TRIDENT][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_5,
		wpcmds[WPCMDS_TRIDENT][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_5,
		NULL
	},

	weapon_give_5,
	weapon_select_5,
	tridentCommand,
	tridentUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(trident_t),

	500,
	100,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	66,         //	initial ammo
	1200.0,     //	speed
	2000.0,     //	range
	20.0,       //	damage
	0.0,        //	lifetime

	WFL_LOOPAMBIENT_ANIM|WFL_FORCE_SWITCH|WFL_CROSSHAIR,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_TRIDENT],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_TRIDENT),
	ATTACKER_VICTIM,

	WEAPON_TRIDENT,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&tridentWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *tridentGive(userEntity_t *self, int ammoCount)
{
	// SCG[12/7/99]: 
//self is the player	self->s.renderfx |= RF_ENVMAP;
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// trident_select()
//---------------------------------------------------------------------------
short tridentSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *tridentCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    return winfoGenericCommand(winfo,inv,commandStr,data);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void tridentUse(userInventory_s *ptr, userEntity_t *self)
{
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used

    if ( !weaponHasAmmo(self,true) )
        return;
     
    winfoPlay(self,SND_SHOOT_STD,1.0f);

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,0.05);

    tridentShootFunc(self);
}

void tridentTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags, bool full)
{
    tridentHook_t *hook=(tridentHook_t *)self->userHook;
//	RELIABLE_UNTRACK(self);
	
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_TRIDENT,projflags,full);
	tinfo.scale = TRI_TIP_SIZE;
	if (projflags & TEF_PROJ_WATER)
		tinfo.scale *= 2;
	if (hook && hook->supercharged)
		tinfo.scale *= 2;
	tinfo.flags |= TEF_SCALE;
	
	if (full)
	{
		tinfo.scale2 = 0.55;
		tinfo.lightSize = 100;
		tinfo.lightColor.Set(0.4,0.4,0.9);
		tinfo.flags |= TEF_SCALE2|TEF_LIGHTSIZE|TEF_LIGHTCOLOR;
	}
}

//---------------------------------------------------------------------------
// shootFunc()
//---------------------------------------------------------------------------
void tridentShootFunc(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    userEntity_t /**weapon=(userEntity_t *)self->client->ps.weapon,*/*middle=NULL,*left=NULL,*right=NULL;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    tridentHook_t *hook=(tridentHook_t *)self->userHook;

    // spawn 3 tips
    middle=tipShoot(self,TRITIP_MIDDLE);
    if ( weaponHasAmmo(self, true) )
    {
        left=tipShoot(self,TRITIP_LEFT);
        if ( weaponHasAmmo(self, true) )
        {
            right=tipShoot(self,TRITIP_RIGHT);
        }
    }

    // clear pointer to self
    if ( middle )
    {
        hook=(tridentHook_t *)middle->userHook;
        hook->middle=NULL;
        hook->left=left;
        hook->rt=right;
    }

    // set pointer to middle tip
    if ( left )
    {
        hook=(tridentHook_t *)left->userHook;
        hook->middle=middle;
    }

    // set pointer to middle tip
    if ( right )
    {
        hook=(tridentHook_t *)right->userHook;
        hook->middle=middle;
    }

//    if ( weapon != NULL ) //	bots have no 1st person weapons!
//        trackSetAnim(weapon,gstate->ModelIndex(winfoModel(MDL_MUZZLEFLASH)),winfoNumFrames(ANIM_MUZZLEFLASH),.15,TEF_ANIMATE|TEF_3POINT_ANIM,.05);

	trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));

	weaponTrackMuzzle(tinfo,self,self,winfo,true);
	tinfo.modelindex = gstate->ModelIndex("models/e2/we_mftrdnt.sp2");			
	tinfo.numframes = winfoNumFrames(ANIM_MUZZLEFLASH)|TEF_ANIM2_FIRST|TEF_ANIM2_THIRD;
	tinfo.frametime = 0.05;
	tinfo.scale = 0.15;
	tinfo.fxflags = TEF_ANIMATE2|TEF_LIGHT|TEF_FX_ONLY;
	tinfo.flags |= TEF_MODELINDEX|TEF_NUMFRAMES|TEF_FRAMETIME|TEF_FXFLAGS|TEF_SCALE|TEF_LONG2;
	tinfo.Long2 = TEF_3POINT_ANIM;
	tinfo.lightSize = 150;
	tinfo.lightColor = CVector(.8,.8,1.0);
	tinfo.flags |= TEF_LIGHTSIZE|TEF_LIGHTCOLOR;

	com->trackEntity(&tinfo,MULTICAST_PVS);

    invWeapon->lastFired=gstate->time;
}

//---------------------------------------------------------------------------
// shootTip()
//---------------------------------------------------------------------------
userEntity_t *tipShoot(userEntity_t *self,tridentTip_t tiptype)
{
#define TRI_MIDX        1

    userEntity_t *ent;
    tridentHook_t *hook;
    int pc;
    float /*size,*/movedir[]={-1,0,1};
    CVector angle;
//    CVector pos[3]= { CVector(TRI_MIDX-14, 10, 12),
//					  CVector(TRI_MIDX,    10, 18),
//					  CVector(TRI_MIDX+14, 10, 16) };// SCG[1/24/00]: not used


    float           kick_velocity=-50;
    CVector         kick_angles;
	kick_angles.Set(-1,0,0);
	weapon_kick(self, kick_angles, kick_velocity, 200, 200);

	WEAPON_FIRED(self);
    weaponUseAmmo(self,false);

    // spawn tip
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    if ( tiptype == 1 )
	{
		offset.Set( winfo->fWeaponOffsetX2, winfo->fWeaponOffsetY2, winfo->fWeaponOffsetZ2 );
	}
	else
	if ( tiptype == 2 )
	{
		offset.Set( winfo->fWeaponOffsetX3, winfo->fWeaponOffsetY3, winfo->fWeaponOffsetZ3 );
	}

	ent=weapon_spawn_projectile(self, offset,winfo->speed,0,winfo->ammoName);
    ent->movetype=MOVETYPE_FLYMISSILE;
    ent->think=tipThink;
    ent->nextthink=gstate->time+.1;
    ent->touch=tipTouch;
//    ent->s.effects2=EF2_TRIDENTTRAIL;
    ent->avelocity.z = 720;
    ent->remove = tipRemove;
    winfoSetModel(ent,MDL_TIP);

    // setup hook
    ent->userHook = gstate->X_Malloc(sizeof(tridentHook_t),MEM_TAG_HOOK);
    hook=(tridentHook_t *)ent->userHook;
    hook->tiptype=tiptype;
    hook->movedir=movedir[tiptype];
    hook->changeDirTime=gstate->time+TRI_EXPAND_TIME;
    hook->boostTime=gstate->time+TRI_EXPAND_TIME*2;
    hook->supercharged=false;

    angle=ent->s.angles;
    angle.AngleToVectors(hook->forward,hook->right,hook->up);

    // set starting size
//    size=TRI_TIP_SIZE;// SCG[1/24/00]: not used
    pc=gstate->PointContents(self->s.origin);

	ent->s.render_scale.Set(0.001,0.001,0.001);
	gstate->LinkEntity(ent);

	trackInfo_t tinfo;
	if (pc & MASK_WATER)
	{
		tridentTrack(tinfo,ent,self,TEF_PROJ_LAUNCH|TEF_PROJ_FLY|TEF_PROJ_WATER,true);
	}
	else
	{
		tridentTrack(tinfo,ent,self,TEF_PROJ_LAUNCH|TEF_PROJ_FLY,true);
	}

	com->trackEntity(&tinfo,MULTICAST_ALL);
    return(ent);
}

//---------------------------------------------------------------------------
// think_tip()
//---------------------------------------------------------------------------
void tipThink(userEntity_t *self)
{
    tridentHook_t *hook=(tridentHook_t *)self->userHook;
	if (!hook)
	{
		self->remove(self);
		return;
	}

	// are we in water?
	short inWater;
	if (gstate->PointContents(self->s.origin) & MASK_WATER)
		inWater = 1;
	else
		inWater = 0;

    // spin tips
    self->s.angles.z += 60;
    if ( self->s.angles.z >= 360 )
        self->s.angles.z -= 360;

    // all three tips must exist for weapon to build power.
    // middle tip controls direction of outer tips, and determines (by boostTime) when it's time
    // to become a super projectile.
    if ( hook->tiptype==TRITIP_MIDDLE && hook->left && hook->rt )
    {
        tipSetOuterDirection(hook->left);
        tipSetOuterDirection(hook->rt);

        if ( gstate->time >= hook->boostTime )
        {
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

			int num = e2_com_RadiusDamage (self, self->owner, self, winfo->damage, DAMAGE_EXPLOSION, 128);
			WEAPON_HIT(self->owner,num);
			winfoPlayAttn(self,SND_LIGHTNING1+(rand() & 1), 1.0, 1000,2500);
            self->remove(hook->left);
            self->remove(hook->rt);
            hook->left=hook->rt=NULL;
            hook->supercharged=true;
        }
    }

	if ((hook->inWater != inWater) || (hook->supercharged && !hook->supermsgSent))
	{
		trackInfo_t tinfo;
		short flags = TEF_PROJ_FLY;

		if (inWater)
			flags |= TEF_PROJ_WATER;		

		if (hook->supercharged)
		{
			if (!hook->supermsgSent)
			{
				hook->supermsgSent = 1;
				flags |= TEF_PROJ_SPECIAL;
			}
		}

		tridentTrack(tinfo,self,self->owner,flags,false);
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}

	hook->inWater = inWater;

    self->nextthink=gstate->time+.1;
}

//---------------------------------------------------------------------------
// setOuterTipDirection()
//---------------------------------------------------------------------------
void tipSetOuterDirection(userEntity_t *self)
{
    tridentHook_t *hook=(tridentHook_t *)self->userHook;
	if (!hook)
		return;

    // move in current 'movedir' direction
    CVector vec=hook->forward*winfo->speed + hook->right*hook->movedir*200;
    self->velocity = vec;

    // set angle to direction of travel
    VectorToAngles(vec,vec);
    self->s.angles = vec;

    // time to move in other direction?
    if ( gstate->time >= hook->changeDirTime )
    {
        hook->movedir = -hook->movedir;
        hook->changeDirTime=gstate->time+9999;
    }
}

//---------------------------------------------------------------------------
// touch_tip()
//---------------------------------------------------------------------------
void tipTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    tridentHook_t *hook=(tridentHook_t *)self->userHook,*mhook;
	if (!hook)
		return;
    short damage;

    // cleanup pointers held by other tips
    switch ( hook->tiptype )
    {
        case TRITIP_MIDDLE:
            if ( hook->left )
            {
                mhook=(tridentHook_t *)hook->left->userHook;
                mhook->middle=NULL;
            }

            if ( hook->rt )
            {
                mhook=(tridentHook_t *)hook->rt->userHook;
                mhook->middle=NULL;
            }
            break;

        default:
            // i shouldn't need to check for middle->userHook ... if there's a middle, there should be a userHook
            // BUT, userHook is NULL sometimes...
            if ( hook->middle && hook->middle->userHook )
            {
                // clear left and right tip entity pointers -- this aborts all special movement.
                mhook=(tridentHook_t *)hook->middle->userHook;
                mhook->left=mhook->rt=NULL;
            }
            break;
    }

	self->touch = NULL;
	if (!surf || (surf && (surf->flags & SURF_SKY)))
	{
        self->remove (self);
        return;
	}
    else if ( other->takedamage==DAMAGE_NO )
    {
//		winfoPlayAttn(self,SND_EXPLODE,1.0,500,1000);
		if ( plane )
		{
			CVector temp;
			VectorToAngles(plane->normal,temp);

			trackInfo_t tinfo;
			tridentTrack(tinfo,self,self->owner,TEF_PROJ_DIE,false);
			tinfo.altpos = temp;
			tinfo.flags |= TEF_ALTPOS;
			com->trackEntity(&tinfo,MULTICAST_ALL);
			self->touch = NULL;
		}
		else
			self->remove(self);

//        return;
    }

	winfoPlayAttn(self,SND_EXPLODE,1.0,500,1000);
    // do damage
    damage = winfo->damage;
    if ( gstate->PointContents(self->s.origin) & MASK_WATER )
    {
        damage *= 2;
    }

    if ( hook->supercharged )
        damage *= 10;

	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;

//    e2_com_Damage(other,self,self->owner,other->s.origin,other->s.angles,damage,DAMAGE_INERTIAL);
    e2_com_RadiusDamage(self,self->owner,NULL,damage,DAMAGE_SIDEWINDER, 100.0);
	self->think = tipDie;
	self->nextthink = gstate->time + 0.1;
}

void tipDie(userEntity_t *self)
{
	self->remove(self);
	return;
}

void tipRemove(userEntity_t *entTip)
{
	weaponUntrackProjectile(entTip);
    winfoStopLooped(entTip);
//    gstate->RemoveEntity(entTip);
}

//-----------------------------------------------------------------------------
//	weapon_trident
//-----------------------------------------------------------------------------
void weapon_trident(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_tritips
//-----------------------------------------------------------------------------
void ammo_tritips (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 30, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_trident_register_func()
{
	gstate->RegisterFunc("tridenttipThink",tipThink);
	gstate->RegisterFunc("tridentUse",tridentUse);
	gstate->RegisterFunc("tridenttipRemove",tipRemove);
	gstate->RegisterFunc("tridenttipTouch",tipTouch);
	gstate->RegisterFunc("tridentGive",tridentGive);
	gstate->RegisterFunc("tridentSelect",tridentSelect);
}
