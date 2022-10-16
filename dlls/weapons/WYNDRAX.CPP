//---------------------------------------------------------------------------
// WYNDRAX'S WISP
//---------------------------------------------------------------------------
#include "weapons.h"
#include "wyndrax.h"

enum
{
    // model defs
    MDL_START,

    // sound defs
    SND_START,
    SND_ZAP,
	SND_PICKUP,

    // special frame defs
    ANIM_START
};

// weapon info def
weaponInfo_t wyndraxWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_WYNDRAX),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_WYNDRAX_AMMO),
	// names, net names, icon
	"weapon_wyndrax",
	tongue_weapons[T_WEAPON_WYNDRAX],
	"ammo_wisp",
	tongue_weapons[T_WEAPON_WYNDRAX_AMMO],
	"iconname",

	// model files
	{
		"models/e3/w_wisp.dkm",
		"models/e3/a_wyndrx.dkm",
		"models/e3/we_wisp.dkm",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e3/we_wwispready.wav",
		"e3/we_wwispaway.wav",
		"e3/we_wwispshoota.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e3/we_wwispcorditea.wav",
		"e3/we_wwispcorditec.wav",
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
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_WYNDRAX][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_5,
		wpcmds[WPCMDS_WYNDRAX][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_5,
		NULL
	},

	weapon_give_5,
	weapon_select_5,
	wyndraxCommand,
	wyndraxUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(wyndrax_t),

	500,
	100,        //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	100,        //	initial ammo
	200.0,  //	speed
	1000.0,     //	range
	10.0,       //	damage
	4.0,    //	lifetime

	WFL_SPECIAL|WFL_FORCE_SWITCH|WFL_PLAYER_ONLY,        //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_WYNDRAX],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_WYNDRAX),
	VICTIM_ATTACKER,

	WEAPON_WYNDRAX,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&wyndraxWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *wyndraxGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// wyndrax_select()
//---------------------------------------------------------------------------
short wyndraxSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *wyndraxCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

void WyndraxWisp_Attack(userEntity_t *self );
//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void wyndraxUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;

    if ( !weaponUseAmmo(self,true) )
        return;

    winfoPlay(self,SND_SHOOT_STD,1.0f);

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

    phook->fxFrameFunc=WyndraxWisp_Attack;
    phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+10;

//	WyndraxWisp_Attack(ptr, self);
}
// ----------------------------------------------------------------------------
//
// Name:		Ent
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	weapon_wyndrax
//-----------------------------------------------------------------------------
void weapon_wyndrax(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

#define	WISP_GROUND_DIST	512
#define	WISP_TRACE_DIST		1024
#define	WISP_ANGLE_INC		45.0
#define	WISP_BASE_SPEED		40.0
#define	WISP_RANDOM_SPEED	80.0
#define	WISP_SINE_INC		0.25

typedef struct
{
    weaponInfo_t    *winfo;     //	pointer to winfo for this weapon
    int             count;      //	how much ammo this ammo pack holds

    CVector         base_origin;        //	where wisp started
    CVector         spin_origin;        //	origin wisp is spinning around

    float           spin_distance;      //	distance to switch spin_sign
    float           spin_sign;          //	negative to spin out, positive for in
//    float           spin_pitch;
    float           sin_count;
    float           sin_count_add;

    int             spin_count;
    int             spin_time;

    int             movePitch;      //	range -1.0 - 1.0, postive = up, negative = down
} wispTouchHook_t;

void    ammoWispThink (userEntity_t *self);
void	ammoWispSetup (userEntity_t *self)
{
    wispTouchHook_t *hook;
    hook = (wispTouchHook_t *) self->userHook;

    hook->count = 5;
    hook->winfo = winfo;

    hook->spin_count = 0;
    hook->spin_time = 0;
    hook->sin_count_add = WISP_SINE_INC + rnd () * 1.0;
    hook->sin_count = rnd () * 3.14159265359 * 2.0;
	hook->movePitch = 1.0f;

    self->s.effects |= EF_WISP_TRAIL;
    self->s.renderfx = SPR_ALPHACHANNEL;
    self->movetype = MOVETYPE_FLY;
    self->solid = SOLID_TRIGGER;
    self->clipmask = MASK_PLAYERSOLID;
	self->s.render_scale.Set(2,2,2);

    self->s.mins.Set(-8, -8, -8);
    self->s.maxs.Set(8, 8, 8);

    self->max_speed = 50.0;
	self->velocity.Set(0,0,self->max_speed);

    gstate->LinkEntity (self);

    self->think = ammoWispThink;
    self->nextthink = gstate->time + 0.1;
}

//-----------------------------------------------------------------------------
//	ammoWispRespawn
//
//-----------------------------------------------------------------------------

void    ammoWispRespawn (userEntity_t *self)
{
    if ( deathmatch->value )//FIXME: no respawn unless deathmatch
    {
        self->s.modelindex = gstate->ModelIndex (self->modelName);

        self->solid = SOLID_TRIGGER;
        self->nextthink = -1;

        self->s.effects |= EF_RESPAWN;  //	tell client to start respawn effect
        self->s.renderfx |= RF_GLOW | RF_TRANSLUCENT;

        gstate->LinkEntity (self);

        //	Q2FIXME:	re-implement ambient volume changes
        // aiment is used to hold the ambient volume between respawns
        if ( self->soundAmbientIndex )
            self->s.sound = self->soundAmbientIndex;

		wispTouchHook_t *hook;
		hook = (wispTouchHook_t *) self->userHook;
		self->s.origin = hook->base_origin;

		ammoWispSetup(self);
    }
}

//-----------------------------------------------------------------------------
//	ammoTouchDefault
//
//	called when a weapon pickup item is touched
//	sets item's modelindex to 0 so that it disappears
//	sets item's solid type to SOLID_NOT so that it cannot be touched
//	calls item's give function
//	sets weapon's think to respawn function
//-----------------------------------------------------------------------------

void    ammoWispTouch (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf)
{
    ammoTouchHook_t *hook = (ammoTouchHook_t *) self->userHook;
	if (!hook)
		return;

    weapon_t        *new_weapon;
    ammo_t          *ammo = NULL;
    int             ammo_addcount = hook->count;
    weaponInfo_t    *winfo;

    if ( !(other->flags & FL_CLIENT) )
        return;

	// get the ammo from the inventory
    ammo = (ammo_t *) gstate->InventoryFindItem (other->inventory, self->className);
    if (ammo && (winfo = (weaponInfo_t *)ammo->winfo))
    {
		ammo_addcount = (ammo->count + ammo_addcount <= winfo->ammo_max) ? (ammo_addcount) : (winfo->ammo_max - ammo->count);
    }

    //	add the ammo
	if (!ammo_addcount)
		return;

    ammo = ammoAdd (other, hook->count, hook->winfo);

	if (!ammo)
		return;

	if (ammo->winfo && (other->flags & FL_CLIENT))
	{
	//	gstate->centerprint (other, 1.0, "%s %s.\n", tongue[T_PICKUP_WEAPON], ammo->winfo->ammoNetName);
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s %s.\n",2);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue,T_PICKUP_WEAPON));
		gstate->cs.SendStringID(ammo->winfo->ammoNameIndex);
		gstate->cs.Unicast(other, CS_print_center, 1.0);
		gstate->cs.EndSendString();
	}

	gstate->StartEntitySound( other, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispcorditec.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
//	winfoPlay(other,SND_PICKUP,1.0);
    if ( ammo_addcount == 0 )
    {
        //	player didn't have any ammo of this type, so check to see
        //	if they already have a matching weapon of this type
        new_weapon = (weapon_t *) gstate->InventoryFindItem (other->inventory, hook->winfo->weaponName);
        if ( new_weapon )
        {
            //	weaponShouldSwitch returns true player should switch to the new weapon
            if ( weaponShouldSwitch ((weapon_t *) new_weapon, (weapon_t *) other->curWeapon) )
            {
                winfo = (weaponInfo_t *) new_weapon->winfo;
                winfo->select_func (other);
            }
        }
    }


    //	make ammo invisible
    self->s.modelindex = 0;

    //	set up wisp to respawn
    if ( deathmatch->value )
    {
        self->s.modelindex = 0; //	set model to 0, modelName should still be valid, though
        self->solid = SOLID_NOT;

        //	shut off looping ambient sounds
        if ( self->s.sound > 0 )
        {
            self->soundAmbientIndex = self->s.sound;
            self->s.sound = 0;
        }
        else
            self->soundAmbientIndex = 0;

        self->nextthink = gstate->time + self->delay;
        self->think = ammoWispRespawn;
    }
    else
        self->remove (self);

    //	flash screen if this is a client
    if ( other->flags & FL_CLIENT )
        other->client->bonus_alpha = 0.25;

	WEAPON_DISPLAY_UPDATE(other);
}

//-----------------------------------------------------------------------------
//	ammoWispSpin
//
//	
//-----------------------------------------------------------------------------

void    ammoWispSpin (userEntity_t *self)
{
    wispTouchHook_t *hook = (wispTouchHook_t *) self->userHook;
    CVector         dir_to_origin, dir_right, end, dir, normal;
    trace_t         tr;
    float           outward_speed, dist;

    hook->sin_count += WISP_SINE_INC;
    if ( hook->sin_count > 3.14159265359 * 2.0 )
        hook->sin_count = 0.0;

    hook->spin_time--;
    if ( hook->spin_time <= 0 )
    {
        hook->spin_count = 0;
        ammoWispThink (self);

        self->max_speed = WISP_BASE_SPEED + rnd () * WISP_RANDOM_SPEED;

        return;
    }

    //	find direction to origin without z
    dir_to_origin.x = hook->spin_origin [0] - self->s.origin [0];
    dir_to_origin.y = hook->spin_origin [1] - self->s.origin [1];
    dir_to_origin.z = 0;

    //	normalize it
    dist = dir_to_origin.Length();
    dir_to_origin.Normalize ();

    //	get direction to the right of this
    dir_right.x = dir_to_origin.y;
    dir_right.y = - dir_to_origin.x;
    dir_right.z = dir_to_origin.z;
    dir_right = dir_right * hook->spin_sign;

    outward_speed = self->max_speed * 0.25;
    self->velocity.x = dir_right.x * self->max_speed + dir_to_origin.x * outward_speed;
    self->velocity.y = dir_right.y * self->max_speed + dir_to_origin.y * outward_speed;
    self->velocity.z = dir_right.z * self->max_speed + dir_to_origin.z * outward_speed;
    self->velocity.z += hook->spin_sign * 20;

    //	check direction of flight for obstructions
    end = self->s.origin + self->velocity * 0.2;
    tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
    if ( tr.fraction < 1.0 )
    {
        hook->spin_count = 0;
        ammoWispThink (self);

        self->max_speed = WISP_BASE_SPEED + rnd () * 160;

        return;
    }

    self->max_speed *= 1.01;

    self->think = ammoWispSpin;
    self->nextthink = gstate->time + 0.1;
}

//-----------------------------------------------------------------------------
//	ammoWispThink
//
//	ai function for wisp ammo
//-----------------------------------------------------------------------------
#define MAX_WISP_HEIGHT 48
void    ammoWispThink (userEntity_t *self)
{
    wispTouchHook_t *hook = (wispTouchHook_t *) self->userHook;
	if (!hook)
		return;

	self->s.angles.yaw += 15;
	if (self->s.angles.yaw > 360)
		self->s.angles.yaw -= 360;

	float height = self->s.origin.z - hook->base_origin.z;
	if (hook->movePitch >= 0)	// moving up
	{
		if (height > MAX_WISP_HEIGHT)
		{
			hook->movePitch = -1;
		}
		else if (height < 0)
		{
			hook->movePitch = 1;
		}
	}
	else						// moving down
	{
		if (height < 0)
		{
			hook->movePitch = 1;
		}
		else if (height > MAX_WISP_HEIGHT)
		{
			hook->movePitch = -1;
		}
	}

	if (hook->movePitch >= 0)
	{
		if (self->velocity.z < 0)
			self->velocity.Set(0,0,self->max_speed);
	}
	else
	{
		if (self->velocity.z >= 0)
			self->velocity.Set(0,0,-self->max_speed);
	}

    self->think = ammoWispThink;
    self->nextthink = gstate->time + 0.1;
}

/*
//-----------------------------------------------------------------------------
//	ammoWispThink
//
//	ai function for wisp ammo
//-----------------------------------------------------------------------------

void    ammoWispThink (userEntity_t *self)
{
    wispTouchHook_t *hook = (wispTouchHook_t *) self->userHook;
    float   groundDist, addYaw, addPitch;
    int     moveDown = false, retryCount = 0;
    int     moveUp = false;
    CVector end, ang, vel;
    trace_t tr;

	if( hook == NULL )
	{
		return;
	}

    hook->sin_count += WISP_SINE_INC;
    if ( hook->sin_count > 3.14159265359 * 2.0 )
        hook->sin_count = 0.0;

    hook->spin_count++;
    if ( hook->spin_count >= 50 )
    {
        ang = self->velocity;
        ang.Normalize ();
        end = self->s.origin + ang * 7.0;
        hook->spin_origin = end;
        hook->spin_time = rand () % 75 + 25;
        hook->spin_distance = rnd () * 64 + 32;
        if ( rand () % 100 < 50 )
            hook->spin_sign = -1;
        else
            hook->spin_sign = 1;

        self->max_speed = WISP_BASE_SPEED + rnd () * WISP_RANDOM_SPEED;

        ammoWispSpin (self);

        return;
    }

    //	check distance from the ground
    end = self->s.origin;
    end.z -= WISP_GROUND_DIST;
    tr = gstate->TraceLine_q2 (self->s.origin, end, self, self->clipmask);
	CVector dir = tr.endpos - self->s.origin;
    groundDist = dir.Length();//tr.fraction * WISP_GROUND_DIST;
    if ( groundDist > 96 )
        //	wisp needs to go towards ground
        moveDown = true;
    else if ( groundDist < 24 )
        //	wisp is too low to ground
        moveUp = true;

    if ( rand () % 100 < 25 )
        addYaw = rnd () * 60 - 30;
    else
        addYaw = 0.0;

    //	determine which direction to fly
    ammo_wisp_think_retry:
    ang = self->s.angles;
    ang.y += addYaw;

    if ( moveDown || moveUp )
    {
        if ( moveDown )
            hook->movePitch = 1.0;
        else if ( moveUp )
            hook->movePitch = -1.0;
        addPitch = 5.0;
        ang.x += addPitch * hook->movePitch;

        if ( ang.x > 30 )
            ang.x = 30;
        else if ( ang.x < -30 )
            ang.x = -30;
    }
    else

        ang.x = sin (hook->sin_count) * 30;

    ang.AngleToVectors (forward, right, up);
    end = self->s.origin + forward * (self->max_speed * 0.1 + 16.0);
    tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
    if ( tr.fraction < 1.0 )
    {
        self->s.angles.y += WISP_ANGLE_INC;
        retryCount++;
        if ( retryCount < (360 / WISP_ANGLE_INC) )
        {
            addPitch = 0;
            addYaw = 0;
            goto    ammo_wisp_think_retry;
        }
        else
        {
            self->nextthink = gstate->time + 0.1;
        }
    }

    self->s.angles.x = ang.x;
    self->s.angles.y = ang.y;

    forward = forward * self->max_speed;
    self->velocity = forward;

    self->think = ammoWispThink;
    self->nextthink = gstate->time + 0.1;
}
*/
//-----------------------------------------------------------------------------
//	ammo_wisp
//-----------------------------------------------------------------------------
void ammo_wisp (userEntity_t *self)
{
    wispTouchHook_t *hook;

    winfoAmmoSetSpawn (winfo, self, 5, 30.0, ammoWispTouch);

    //delete (self->userHook);
	if (self->userHook)
		gstate->X_Free(self->userHook);

    //self->userHook = new wispTouchHook_t;
	self->userHook = gstate->X_Malloc(sizeof(wispTouchHook_t),MEM_TAG_HOOK);
    hook = (wispTouchHook_t *) self->userHook;

    hook->base_origin = self->s.origin;
    hook->spin_origin = self->s.origin;

/*    hook->count = 5;
    hook->winfo = winfo;

    hook->base_origin = self->absmin;
    hook->spin_origin = self->s.origin;

    hook->spin_count = 0;
    hook->spin_time = 0;
    hook->sin_count_add = WISP_SINE_INC + rnd () * 1.0;
    hook->sin_count = rnd () * 3.14159265359 * 2.0;
	hook->movePitch = 1.0f;

    self->s.effects |= EF_WISP_TRAIL;
    self->s.renderfx = SPR_ALPHACHANNEL;
    self->movetype = MOVETYPE_FLY;
    self->solid = SOLID_TRIGGER;
    self->clipmask = MASK_PLAYERSOLID;
	self->s.render_scale.Set(2,2,2);

    self->s.mins.Set(-8, -8, -8);
    self->s.maxs.Set(8, 8, 8);

    self->max_speed = 50.0;
	self->velocity.Set(0,0,self->max_speed);

    gstate->LinkEntity (self);

    self->think = ammoWispThink;
    self->nextthink = gstate->time + 0.1;*/
	ammoWispSetup(self);
}


/* ************************************ Wyndrax's Wisps ************************************** */

//---------------------------------------------------------------------------
// directLine()  Something that might not need to be here <check later> <nss>
//---------------------------------------------------------------------------
int directLine(userEntity_t *src,userEntity_t *dst)
{
    CVector         org, end;

	if (!src || !dst)
		return (0);

    end = ((dst->absmin + dst->absmax) * 0.5) + dst->view_ofs;
    org = src->s.origin + src->view_ofs;

    gstate->TraceLine (org, end, TRUE, src, &trace);

    if ( trace.fraction == 1.0 || trace.ent == dst )
    {
        org = org - trace.endpos;

        // return the distance to target
        return	org.Length();
    }

    return(0);
}

// ----------------------------------------------------------------------------
//
// Name:        Wisp_Personality_Generator
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float Wisp_Personality_Generator(float P)
{
	float New;

	//These buggers are pretty happy who they are...but sometimes they like changing.
	if(rnd() > 0.55f)
	{
		//But when they do they get a new personality...
		New = rnd();
	}
	else
	{
		New = P;
	}
	
	return New;
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		sineMove 
// Description:A modified version of the previous sineMove
// Input:	userEntity_t *self  --> what we want to move around crazy like
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
#define WOBBLE				75
void sineMove(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (!hook)
		return;

    CVector vec,Dir;
	int count;
	float fDistance;
	if (self->enemy)
	{
		fDistance= VectorDistance(self->enemy->s.origin,self->s.origin);
		Dir = self->enemy->absmax - self->s.origin;
	}
	else
	{
		fDistance = 200;
		Dir = self->velocity;
	}
	

//	Dir.z += 10 + (rnd()*50);
	
	Dir.Normalize();

	Dir = Dir * 150;

	count = hook->sinofs;
	if(hook->Personality < 0.0)
	{
		count = 11 - count;
	}
	
	if(abs((int)Dir.x) > abs((int)Dir.y))
	{
		Dir.y += cos_tbl[count] * (WOBBLE*hook->Personality);
		
	}
	else
	{
		Dir.x += cos_tbl[count] * (-WOBBLE*hook->Personality);
	}
	
	
	Dir.z += sin_tbl [count] * (WOBBLE*hook->Personality);
   
    //This code prevents the wisps from getting over the player's head.
	if(fDistance < 64.0f)
	{
		Dir.x = Dir.x * -1;
		Dir.y = Dir.y * -1;
	}	
	else if(fDistance < 100.0)
	{
		Dir.x = 0.0f;
		Dir.y = 0.0f;
	}
		
	self->velocity = Dir;

    hook->sinofs++;
    if ( hook->sinofs > 11 )
	{
        hook->sinofs=0;
		hook->Personality = Wisp_Personality_Generator(hook->Personality);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		removeWisp
// Description:Make sure we remove all attached lightning and then the wisp
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void removeWisp(userEntity_t *self)
{
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispaway.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	weaponUntrackProjectile(self);
}

void wispTrack(trackInfo_t &tinfo, userEntity_t *self, userEntity_t *player, short projflags,bool full)
{
	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackProjectile(tinfo,self,player,winfo,TEF_PROJ_ID_WISP,projflags,full);

    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (hook)
	{
		CVector c1(0.25,0.45,0.85);
//		CVector c2(0.85,0.45,0.25);
		tinfo.lightColor = c1;//romero wants themm blue.Interpolate(c1,c2,hook->Personality);
		tinfo.flags |= TEF_LIGHTCOLOR;
	}

	if (full)
	{
		tinfo.modelindex = gstate->ModelIndex ("models/global/e_flare4+o.sp2");
		tinfo.flags |= TEF_MODELINDEX;
	}
}

void sendTargetList(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (!hook)
		return;

	trackInfo_t tinfo;
	wispTrack(tinfo,self,self->owner,TEF_PROJ_FLY,false);

	tinfo.flags |= TEF_LONG2|TEF_DSTPOS;
	int count = 0;
	for (int i = 0; i < WYNDRAX_MAX_LIGHTNING; i++)
	{
		if (hook->lightningList[i])
		{
			switch(count)
			{
			case 0:
				tinfo.dstpos.x = hook->lightningList[i]->s.number;
				break;

			case 1:
				tinfo.dstpos.y = hook->lightningList[i]->s.number;
				break;

			case 2:
				tinfo.dstpos.z = hook->lightningList[i]->s.number;
				break;

			case 3:
				tinfo.Long2 = hook->lightningList[i]->s.number;
				break;
			}
			count++;
		}
	}

	com->trackEntity(&tinfo,MULTICAST_ALL);
}

void wispDamage(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (!hook)
		return;

	CVector dir;

	char buffer[30];
	int Letter_Offset;

	// dig through our lightning list and zap stuff.
	bool changed = false;
	bool zap = (gstate->time > self->hacks);
	for (int i = 0; i < WYNDRAX_MAX_LIGHTNING; i++)
	{
		if (hook->lightningList[i])
		{
			zap = true;
			CVector dir = hook->lightningList[i]->s.origin - self->s.origin;
			if (dir.Length() > 250)
				continue;

			dir.Normalize();
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

			hook->hits.AddHit(self->owner,hook->lightningList[i]);
			com->Damage(hook->lightningList[i],self,self->owner,hook->lightningList[i]->s.origin,dir,0.5 * winfo->damage,DAMAGE_INERTIAL | DAMAGE_SPARKLES);

			if (!EntIsAlive(hook->lightningList[i]) || (frand() > 0.9))
			{
				changed = true;
				hook->lightningList[i] = NULL;
				hook->lightningCount--;
			}
		}

	}

	if (zap)
	{
		self->hacks = gstate->time + 0.1 + 0.2 * frand();
		//Generate random noise
		Letter_Offset = (int)((rnd()*3.0f)+97.0f);
		sprintf(buffer,"e3/we_wwispcordite%c.wav",Letter_Offset);
		//Make noise
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(buffer),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}

	if (changed)
		sendTargetList(self);
}

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		wisFade
// Description:Process to make the wisp fade out into nothing
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wispFade(userEntity_t *self)
{
	wispDamage(self);

	self->s.alpha -= 0.05;

	self->s.render_scale.x -= 0.10f;
	self->s.render_scale.y -= 0.10f;
	//For weird fade out into 'dense' space effect.
	if(self->s.alpha < 0.50)
	{
		self->s.render_scale.z -= 0.20f;
	}
	else
	{
		self->s.render_scale.z += 0.10f;
	}
	sineMove(self);
	if((self->s.alpha < 0.001f) || (self->s.render_scale.x <= 0.1) || (self->s.render_scale.x <= 0.1))
	{
		removeWisp(self);
	}

	self->nextthink=gstate->time+.1;
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		WyndraxWispTouch
// Description:Touch function for the wisps
// Input:userEntity_t *self -->The wisp
//		userEntity_t *other -->The thing we touched
//		cplane_t *plane     -->The plane
//      csurface_t *surf    -->The surface of the plane
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxWispTouch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (!hook)
		return;

    CVector vec;

    // calc reflection vector
    if(plane)
	{
		float dp=hook->forward.x*plane->normal[0] + hook->forward.y*plane->normal[1] + hook->forward.z*plane->normal[2];
	    CVector normal(plane->normal);
	    normal=normal*dp;
	   vec=hook->forward-2*normal;

		// put this in angles
		VectorToAngles(vec,vec);
		self->s.angles = vec;

		// calc new forward / up vectors
		vec=self->s.angles;
		vec.AngleToVectors(forward,right,up);
		hook->forward=forward;
		hook->up=up;
		hook->sinetime=gstate->time+.2;
	}
}

// ----------------------------------------------------------------------------
// CEK[10/29/99]:
// Name:		wyndraxWispPrethink
// Description:  Picks and zaps targets, sets an enemy for tracking...
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxWispPrethink(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if (!hook)
		return;

	userEntity_t *oldEnemy=NULL;
	if (!directLine(self,self->enemy) || !EntIsAlive(self->enemy))
	{
		if (EntIsAlive(self->enemy))
			oldEnemy = self->enemy;
		self->enemy = NULL;
	}

	// only 4!!!!!
    if ( rnd() > .02 && hook->lightningCount < WYNDRAX_MAX_LIGHTNING )
    {
		bool entFound = false;
        userEntity_t *ent;
//        short entCount=0,num;
		int i;

        for ( ent=gstate->FirstEntity(); ent; ent=gstate->NextEntity(ent) )
        {
            // exclude some ents
			float fDistance = VectorDistance( self->s.origin, ent->s.origin );

			if ((fDistance > 300) || !validateTarget(self,ent,1,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_NEVER))
				continue;

			// is it in the zap list already?
			long entIndex = ent->s.number;
			for (i = 0; i < WYNDRAX_MAX_LIGHTNING; i++)
			{
				if (!hook->lightningList[i])	// this one's empty..drop the new one in!
				{
					entFound = true;
					hook->lightningList[i] = ent;
					hook->lightningCount++;
					break;
				}
				else if (entIndex == hook->lightningList[i]->s.number)
				{
					break;
				}
			}
		}

		if (entFound)
		{
			sendTargetList(self);
		}
	}
	// if we don't have an enemy, find one, dammit!
	if (!self->enemy)
	{
		userEntity_t *ent,*closestEnt=NULL;

        for ( ent=gstate->FirstEntity(); ent; ent=gstate->NextEntity(ent) )
        {
			if (!validateTarget(self,ent,1,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_NEVER))
				continue;

			if (!closestEnt)
				closestEnt = ent;
			else
			{
				float fDistance = VectorDistance( self->s.origin, ent->s.origin );
				if (fDistance < VectorDistance(self->s.origin, closestEnt->s.origin))
					closestEnt = ent;
			}
		}
		self->enemy = closestEnt ? closestEnt : oldEnemy;
	}
}

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		wyndraxWispThink
// Description:Think function for the wisps themselves
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxWispThink(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	if(!hook)
		return;

	wispDamage(self);

//    short randomZap=false;// SCG[1/24/00]: not used
	CVector dir;	
    CVector Angles;
	if (self->enemy)
		dir = self->enemy->s.origin - self->s.origin;
	else
		dir = self->velocity;

	dir.Normalize();
	
	VectorToAngles(dir,Angles);
	self->s.angles = Angles;	
	
	// time to die?
    if ( (gstate->time >= hook->killtime) || !(EntIsAlive(self->owner)))
    {
 		//change our think to the wispFade function
		self->think = wispFade;
		self->prethink = NULL;
		//In case we take anymore damage let's reset our damage related shit
		self->pain = NULL;
		self->takedamage = DAMAGE_NO;

		self->nextthink=gstate->time+.1;
        return;
    }

    // move in sine wave
    if ( gstate->time >= hook->sinetime )
        sineMove(self);

    self->nextthink=gstate->time+.1;
}




// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		WyndraxWisp_Attack
// Description:Initiating Wyndrax Launch Wisp!
// Input:userInventory_s *ptr, userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void WyndraxWisp_Attack(userEntity_t *self )
{
	playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

	WEAPON_FIRED(self);
	phook->fxFrameFunc = NULL;
	phook->fxFrameNum = -1;

    userEntity_t *wisp;
    wyndraxHook_t *hook;
    CVector vec;

    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    wisp=weapon_spawn_projectile(self,offset,winfo->speed,0,"proj_monster_wisp");
	wisp->s.modelindex	= gstate->ModelIndex ("models/e3/we_wisp.dkm");
	wisp->s.effects		= EF_WISP_TRAIL;
	wisp->enemy			= self->enemy;
	wisp->s.frame		= 0;
    wisp->think			= wyndraxWispThink;
	wisp->prethink		= wyndraxWispPrethink;
	wisp->touch			= wyndraxWispTouch;
    wisp->nextthink		= gstate->time+.1;
    wisp->remove		= removeWisp;
    wisp->movetype		= MOVETYPE_BOUNCEMISSILE;
    wisp->solid			= SOLID_TRIGGER;
	wisp->owner			= self;
	long flag = 0;//MASK_SOLID & ~CONTENTS_SOLID;
	wisp->clipmask		= flag;
	wisp->hacks			= gstate->time + 0.1 + 0.2 * frand();

	gstate->LinkEntity(wisp);
	//Setup the size and scale
    gstate->SetSize(wisp,-16,-16,-16,16,16,16);
	wisp->s.render_scale.Set(2.0f, 2.0f, 2.0f);
//	wisp->s.render_scale.Set(0.001,0.001,0.001);

	//Set alpha, scale, and bounding region
	wisp->s.alpha = 1.0f;
 	
	//Setup initial velocity towards the player	
	AngleToVectors(self->s.angles,forward,right,up);
	wisp->velocity = forward;
	wisp->velocity.Normalize();
	wisp->velocity = wisp->velocity *(500 + rnd() * 500);
	VectorToAngles(forward,wisp->s.angles);

	wisp->userHook = gstate->X_Malloc(sizeof(wyndraxHook_t),MEM_TAG_HOOK);
    hook=(wyndraxHook_t *)wisp->userHook;
    
	
	//Let's setup a lifetime for the Wisps... say like 20 seconds
	hook->killtime=gstate->time + winfo->lifetime;
    
	//Setup static information pertaining to wisp's movement etc..
	hook->sinofs=0;
    hook->sinetime=0;
	hook->Personality = rnd();

    vec=self->s.angles;
    vec.AngleToVectors(forward,right,up);
    hook->forward=forward;
    hook->up=up;
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispshoota.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	
	//Attach the Sprite Light&Glow stuff (add track entity)
    trackInfo_t tinfo;
	wispTrack(tinfo,wisp,self,TEF_PROJ_FLY|TEF_PROJ_LAUNCH,true);
	com->trackEntity(&tinfo,MULTICAST_ALL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_wyndrax_register_func()
{
	gstate->RegisterFunc("wyndraxammoWispThink",ammoWispThink);
	gstate->RegisterFunc("wyndraxammoWispRespawn",ammoWispRespawn);
	gstate->RegisterFunc("wyndraxammoWispSpin",ammoWispSpin);
//	gstate->RegisterFunc("wyndraxlightningThink",lightningThink);
	gstate->RegisterFunc("wyndraxwispFade",wispFade);
	gstate->RegisterFunc("wyndraxwyndraxWispThink",wyndraxWispThink);
	gstate->RegisterFunc("wyndraxwyndraxWispTouch",wyndraxWispTouch);
	gstate->RegisterFunc("wyndraxremoveWisp",removeWisp);
//	gstate->RegisterFunc("wyndraxremoveLightning",removeLightning);
	gstate->RegisterFunc("wyndraxWispPrethink",wyndraxWispPrethink);
	gstate->RegisterFunc("wyndraxGive",wyndraxGive);
	gstate->RegisterFunc("wyndraxSelect",wyndraxSelect);
}
