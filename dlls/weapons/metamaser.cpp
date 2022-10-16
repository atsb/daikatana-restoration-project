//---------------------------------------------------------------------------
// METAMASER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "metamaser.h"

enum
{
    // model defs
    MDL_START,
    MDL_METACUBE,
    MDL_SHOCKRING,

    // sound defs
    SND_START,
    SND_MASERFIRE,
    SND_ARMBEEP,
	SND_SMFIRE1,
	SND_SMFIRE2,
	SND_LGFIRE1,
	SND_LGFIRE2,

    // special frame defs
    ANIM_START,
	ANIM_CUBE_SHOOT,
	ANIM_CUBE_WALK,
    ANIM_SPHEREEXPLODE
};

// weapon info def
weaponInfo_t metamaserWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_METAMASER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_METAMASER_AMMO),
	// names, net names, icon
	"weapon_metamaser",
	tongue_weapons[T_WEAPON_METAMASER],
	"ammo_metamaser",
	tongue_weapons[T_WEAPON_METAMASER_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_mmaser.dkm",
		"models/e4/a_mmaser.dkm",
		"models/e4/a_mmaser.dkm",
		"models/e4/we_mmprj.dkm",
		"models/e1/we_shockring.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_metaready.wav", 
		"e4/we_metaaway.wav", 
		"e2/we_sflareshoota.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"global/we_zapa.wav", 
		"e1/we_c4beepa.wav",
		"e4/we_metamaszapa.wav",
		"e4/we_metamaszapb.wav",
		"e4/we_metamalzapa.wav",
		"e4/we_metamalzapb.wav",
		SND_NOSOUND, 
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
		MDL_METACUBE,"cubeshoot",13,16,
		MDL_METACUBE,"cubewalk",18,37,
		ANIM_NONE,
		0,
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_METAMASER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_6,
		wpcmds[WPCMDS_METAMASER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_6,
		NULL
	},

	weapon_give_6,
	weapon_select_6,
	metamaser_command,
	metamaser_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(metamaser_t),

	900,
	20,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	5,      //	initial ammo
	0,      //	speed
	0,      //	range
	0,      //	damage
	0.0,        //	lifetime

	WFL_SPECIAL|WFL_CROSSHAIR|WFL_FORCE_SWITCH|WFL_PLAYER_ONLY,        //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_METAMASER],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_METAMASER),
	VICTIM_ATTACKER,

	WEAPON_METAMASER,
	0,
	NULL
};
static short metaCount = 0;			// limit to 3 deployed metamasers
static weaponInfo_t *winfo=&metamaserWeaponInfo;
static unsigned short bits[16] = {0x0001,
								0x0002,
								0x0004,
								0x0008,
								0x0010,
								0x0020,
								0x0040,
								0x0080,
								0x0100,
								0x0200,
								0x0400,
								0x0800,
								0x1000,
								0x2000,
								0x4000,
								0x8000};

/// 111b << x 
#define SET(x)	((unsigned short) 7 << (3*(x)))
#define INDEX(x,y) ((x & SET(y)) >> (3*(y)))


//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *metamaser_give(userEntity_t *self, int ammoCount)
{
	// see how many we have
	if (!winfoCheckMaxCount(self, winfo))
	{
		return NULL;
	}

    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// metamaser_select()
//---------------------------------------------------------------------------
short metamaser_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *metamaser_command(struct userInventory_s *inv, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void metamaser_use(struct userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;

    if ( !weaponHasAmmo(self,true) )
        return;

    weaponUseAmmo (self, true);
	winfoPlay(self,SND_SHOOT_STD, 1.0);

//   winfoPlay(self,SND_SHOOT_STD,1.0f);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

    phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+6;
    phook->fxFrameFunc=metamaser_shoot_func;
}

//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void metamaser_shoot_func(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *metacube;
    CVector forward,right,up,temp;
    metamaserHook_t *hook;

    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    metacube=weapon_spawn_projectile(self,offset,800,0,"metacube");

	metacube->owner = self;					// so the player can kill it
    metacube->solid=SOLID_BBOX;
    metacube->movetype=MOVETYPE_TOSS;
	metacube->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
	metacube->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	metacube->fragtype          |= FRAGTYPE_ALWAYSGIB; // boom, bitchie
	metacube->clipmask			= MASK_SOLID;
	metacube->flags			|= FL_EXPLOSIVE;

//    metacube->touch=metamaser_touch;
    metacube->nextthink=gstate->time + 0.1;
    metacube->think=metamaser_think;
    metacube->health = (winfo->fWeaponOffsetY2) ? winfo->fWeaponOffsetY2 : META_MAX_HEALTH;
    metacube->die=metamaser_die;
    metacube->pain=metamaser_pain;
    metacube->takedamage=DAMAGE_YES;
	metacube->s.render_scale.Set(8,8,8);
    winfoSetModel(metacube,MDL_METACUBE);
	metacube->userHook = gstate->X_Malloc(sizeof(metamaserHook_t),MEM_TAG_HOOK);
	gstate->SetSize(metacube,-6,-6,0,6,6,12);
	gstate->LinkEntity(metacube);

    // used to scale rotational velocity
    hook=(metamaserHook_t *)metacube->userHook;
    hook->rotation_scale=.8;
    hook->mass_scale=1.1;
    hook->charges= (winfo->fWeaponOffsetX2) ? winfo->fWeaponOffsetX2 : META_MAX_CHARGES;
    hook->remove_time=gstate->time + ((winfo->fWeaponOffsetZ2) ? winfo->fWeaponOffsetZ2 : 60);
    hook->last_water=CONTENTS_EMPTY;
    hook->hit_water_pos=metacube->s.origin;
	hook->owner = self;


    phook->fxFrameFunc=NULL;
	metaCount++;

    invWeapon->lastFired=gstate->time;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void metamaser_think(userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

//    metamaser_waterFriction(self);
	if (self->velocity.Length() < 10)
	{
		self->s.angles.Set(0,0,0);
		self->spawn_origin = self->s.origin;
		self->s.old_origin = self->s.origin;	
		self->owner = NULL;
		self->movetype = MOVETYPE_NONE;
		self->clipmask = MASK_SOLID;
		self->svflags = SVF_MONSTER;
		self->solid = SOLID_BBOX;
		gstate->LinkEntity(self);

		metamaser_initTargets(self);

		self->think=metamaser_wait;
		self->hacks=gstate->time+3.0;
		hook->pain_end_time=gstate->time+0.5;		// use this for beep time
		self->nextthink=gstate->time+0.1;			// wait 3 seconds before opening the can..
	}

    if (gstate->time >= hook->remove_time)
        metamaser_diethink(self);
    else
        self->nextthink=gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// wait()
//---------------------------------------------------------------------------
void metamaser_wait(userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    if ( gstate->time >= hook->remove_time || !hook->charges || (metaCount >= 3))
	{
        metamaser_diethink(self);
		return;
	}

	if (gstate->time > hook->pain_end_time)
	{
		winfoPlay(self,SND_ARMBEEP,1.0f);
		hook->pain_end_time=gstate->time + 0.5;
	}

	if (gstate->time > self->hacks)
	{
		hook->pain_end_time = 0;
		self->think = metamaser_track;
		self->prethink = metamaser_search;
	}

	self->nextthink=gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// search()
//---------------------------------------------------------------------------
void metamaser_search(userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    userEntity_t *head;

	if (((hook->numTargets == META_MAX_TARGETS) && (hook->numAcquired == hook->maxAcquired)) || (gstate->time < hook->pain_end_time) )
		return;

	float range = (winfo->range) ? winfo->range : 512;
    // search entity list
    head=gstate->FirstEntity();
    while ( (head != NULL)  && (hook->numTargets < META_MAX_TARGETS))
    {
        if ( !hook->charges )
            break;

		if (validateTarget(self,head,0,TARGET_ALWAYS_IF_ALIVE,TARGET_ALWAYS_IF_ALIVE,TARGET_ALWAYS_IF_ALIVE,TARGET_NEVER) && 
		   com->Visible(self, head) && ((self->s.origin - head->s.origin).Length() <= range))
		{
	        metamaser_addTarget(self,head);
		}

        head=gstate->NextEntity(head);
    }

	if (hook->numAcquired < hook->maxAcquired)
		metamaser_acquire(self);

    if ( gstate->time >= hook->remove_time || !hook->charges || (metaCount >= 3))
		metamaser_diethink(self);
    else
    {
		if (hook->numTargets < META_MAX_ATTACKS)
			hook->maxAcquired = hook->numTargets;
		else
			hook->maxAcquired = META_MAX_ATTACKS;

		metamaser_update_flags(self, false);
    }
}

void metamaser_acquire(userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	int i,j,num;
	userEntity_t *target;
	j = hook->numAcquired;

	for (; j <= hook->maxAcquired; j++)
	{
		i = 0;
		while (i < 50)
		{
			num = frand() * META_MAX_TARGETS;
			i++;
			if (target=hook->targets[num].target)
			{
				metamaser_acquire_target(self,target);						// bust em up!
				break;
			}
		}
	}
}

void metamaser_acquire_target(userEntity_t *self, userEntity_t *target)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	int i;

    // target already in list?
    for ( i = 0; i < META_MAX_ATTACKS; i++ )
        if ( hook->acquired[i].target == target )
            return;

	for (i = 0; i < META_MAX_ATTACKS; i++)
	{
		if (hook->acquired[i].target == NULL)
		{
			hook->acquired[i].target = target;
			hook->acquired[i].end_track_time = gstate->time + META_FIRE_RATE + META_FIRE_RATE*frand();//META_ATTACK_TIME + META_ATTACK_TIME*frand();
			hook->acquired[i].next_sound_time = gstate->time;
			hook->acquired[i].next_damage_time = gstate->time;
			hook->numAcquired++;
			metamaser_set_target(i,target->s.number,&hook->tinfo);
			return;
		}
	}
}

void metamaser_unacquire_target(userEntity_t *self, userEntity_t *target)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    int i;

    for ( i = 0; i < META_MAX_ATTACKS; i++ )
    {
        if ( hook->acquired[i].target == target )
        {
            hook->acquired[i].target = NULL;
            hook->acquired[i].end_track_time = 0.0;
            hook->acquired[i].next_sound_time = 0.0;
            hook->acquired[i].next_damage_time = 0.0;  
			hook->numAcquired--;
			metamaser_set_target(i,0,&hook->tinfo);
            return;
        }
    }
}

//---------------------------------------------------------------------------
// track()
//---------------------------------------------------------------------------
void metamaser_meta_remove (userEntity_t *self);
void metamaser_track (userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    int i;
	userEntity_t *target;

	if (gstate->bCinematicPlaying)
	{
		metamaser_meta_remove(self);
		self->think = self->prethink = NULL;
		return;
	}

	if (gstate->time < hook->pain_end_time)
	{
		self->think = metamaser_track;
		self->prethink = metamaser_search;
		self->nextthink=gstate->time + META_SEARCH_TIME;
		return;
	}
	
	if (hook->tinfo.numframes & bits[13])
	{
		hook->tinfo.numframes &= ~bits[13];
		metamaser_update_flags(self,true);
	}
	hook->pain_end_time = 0;

	// do our attacks
	qboolean attacked = FALSE;
	for (i = 0; i < META_MAX_ATTACKS; i++)
	{
		if (target=hook->acquired[i].target)
		{
			if (hook->acquired[i].end_track_time > gstate->time)// && com->Visible(self,target))
			{
				trace_t tr = gstate->TraceLine_q2(self->s.origin, target->s.origin, self, MASK_SHOT);
				if (tr.ent == target)
				{
					float nextTime = gstate->time + 0.5*META_FIRE_RATE + 0.5*META_FIRE_RATE*frand();
					if (hook->acquired[i].next_sound_time <= gstate->time)
					{
						winfoPlay(self,SND_SMFIRE1 + (rand() & 1),1.0);
						hook->acquired[i].next_sound_time = gstate->time + nextTime;
					}
					if ((hook->acquired[i].next_damage_time <= gstate->time))// && (hook->targets[i].target))META_LASER_DAMAGE*
					{
						attacked = TRUE;
						hook->charges--;
						// cek[1-24-00]: set the attackweapon
						winfoAttackWeap;

						com->Damage(hook->acquired[i].target,self,hook->owner,zero_vector,zero_vector,winfo->damage,DAMAGE_NONE);
						if (!EntIsAlive(hook->acquired[i].target))
							com->Damage(hook->acquired[i].target,self,hook->owner,zero_vector,zero_vector,1000,DAMAGE_NONE);

						hook->acquired[i].next_damage_time=gstate->time + nextTime;
						if (( hook->charges) < 0 || (metaCount >= 3))
						{
							metamaser_diethink(self);
							return;
						}
					}
				}
			}
			else
			{
				metamaser_unacquire_target(self,target);
				metamaser_removeTarget(self,target);
			}
		}
	}
	if (attacked)
		metamaser_update_flags(self,false);

	// check our target list
	for (i = 0; i < META_MAX_TARGETS; i++)
	{
        if ( target=hook->targets[i].target )
        {
            // still a valid target?
            if ( hook->targets[i].end_track_time > gstate->time && com->Visible(self,target) )
            {

			}
			else
			{
				metamaser_unacquire_target(self,target);
				metamaser_removeTarget(self,target);
			}
		}
	}
	self->think = metamaser_track;
	self->prethink = metamaser_search;
    self->nextthink=gstate->time + META_SEARCH_TIME;
}

//---------------------------------------------------------------------------
// waterFriction()
//---------------------------------------------------------------------------
void metamaser_waterFriction (userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    CVector forward,right,up,dir,ang;
    float pc,wobble_speed,wander_speed;

    pc=gstate->PointContents(self->s.origin);
    if ( pc != CONTENTS_LAVA && pc != CONTENTS_SLIME && pc != CONTENTS_WATER )
    {
        if ( hook->last_water == CONTENTS_LAVA || hook->last_water == CONTENTS_SLIME ||
             hook->last_water == CONTENTS_WATER )
        {
            // just left water.  if this is a self-propelled MOVETYPE_FLYMISSILE then 
            // scale its velocity by 2
            if ( self->movetype == MOVETYPE_FLYMISSILE )
            {
                dir=self->velocity * 2;
                self->velocity = dir;
            }

            // make sure gravity goes back to normal
            self->gravity=1.0;
        }

        hook->last_water=pc;
        return;
    }

    if ( hook->last_water != pc )
    {
        // just hit water, so slow down
        hook->hit_water_pos=self->s.origin;
        dir=self->velocity * 0.5 * hook->mass_scale;
        self->velocity = dir;
        hook->change_time=gstate->time;

        // decrease effect of gravity
        self->gravity = 0.5 * hook->mass_scale;       

        //gstate->sound (self, CHAN_AUTO, "bloop.wav", 255, ATTN_NORM);
    }
    else if ( (self->movetype == MOVETYPE_BOUNCE) || (self->movetype == MOVETYPE_TOSS) )
    {
        if ( hook->change_time <= gstate->time )
        {
            hook->hit_water_pos.x=self->s.origin[2] + (rnd() - 0.2) * (rnd() * 256.0 + 32.0);

            dir=hook->hit_water_pos - self->s.origin;
            dir.Normalize();

            VectorToAngles(dir, ang);
            ang.AngleToVectors(forward, right, up);

            // randomly choose left or right
            if ( rnd() < 0.5 )
                right=right * -1.0;

            // randomly choose towards or away
            if ( rnd () < 0.5 )
                right=right * -1.0;

            wobble_speed=rnd() * 240;
            wander_speed=rnd() * 180;

            dir=self->velocity + dir * wander_speed;
            self->velocity = dir;

            right=right * wobble_speed;
            self->velocity = right;

            hook->change_time=gstate->time + rnd() * 1.0 + 0.1;

            // change angular velocity because grenade just bounced
            self->avelocity.Set(hook->rotation_scale * (rnd () - 0.5) * 700.0, hook->rotation_scale * (rnd () - 0.5) * 700.0, hook->rotation_scale * (rnd () - 0.5) * 700.0);
        }

        self->velocity.x=self->velocity.x * 0.5 * hook->mass_scale;   
        self->velocity.y=self->velocity.y * 0.5 * hook->mass_scale;   
    }

    hook->last_water=pc;
}

//---------------------------------------------------------------------------
// initTargets()
//---------------------------------------------------------------------------
void metamaser_initTargets(userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    int i;

    for ( i = 0; i < META_MAX_TARGETS; i++ )
    {
        hook->targets [i].target=NULL;
        hook->targets [i].end_track_time=0;
        hook->targets [i].next_sound_time=0;
        hook->targets [i].next_damage_time=0;
    }
	for (i = 0; i < META_MAX_ATTACKS; i++)
	{
        hook->acquired [i].target=NULL;
        hook->acquired [i].end_track_time=0;
        hook->acquired [i].next_sound_time=0;
        hook->acquired [i].next_damage_time=0;
	}
	hook->numAcquired = 0;
	hook->maxAcquired = 0;
	hook->numTargets = 0;
	hook->pain_end_time = 0;
	hook->next_pain_level = META_MAX_HEALTH - META_HEALTH_LEVEL;

	// set up the static tracker stuff (only sent one time .. now)
	hook->tinfo.ent = self;
	hook->tinfo.srcent = self;
//	hook->tinfo.lightSize = 150;			// size of the dlight
//	hook->tinfo.lightColor.Set(0,0,1);		// color of the beam/dlight
//	hook->tinfo.scale = 4;					// radius of the beam
//	hook->tinfo.scale2 = 0.4;				// alpha
	hook->tinfo.frametime = 1.0;			// used by the trackent internally
	hook->tinfo.modelindex = gstate->ModelIndex( "models/global/e_flblue.sp2" );

	// dynamic data
	hook->tinfo.fxflags = TEF_METABEAMS;		// not dynamic but needs sent over and over...
	hook->tinfo.altpos.Set(0,0,0);				// 0 1  2
	hook->tinfo.altpos2.Set(0,0,0);				// 3 4  5
	hook->tinfo.altangle.Set(0,0,0);			// 6 7  8
	hook->tinfo.dstpos.Set(0,0,0);				// 9 10 11
	hook->tinfo.numframes = bits[12];			// bitflags 0 - 11 for the targets, 12 to toggle the light

	hook->tinfo.flags = TEF_SRCINDEX|TEF_FRAMETIME|TEF_MODELINDEX|TEF_FXFLAGS|TEF_ALTPOS|TEF_ALTPOS2|TEF_ALTANGLE|TEF_DSTPOS|TEF_NUMFRAMES;//|TEF_SCALE2|TEF_LIGHTSIZE|TEF_LIGHTCOLOR|TEF_SCALE;
    com->trackEntity(&hook->tinfo,MULTICAST_ALL);
}

void metamaser_set_target(int index, long value, trackInfo_t *tinfo)
{
	if (value == 0)
		tinfo->numframes &= ~bits[index];			// clear its 'on' bit
	else
		tinfo->numframes |= bits[index];				// set its 'on' bit

	CVector *vec;
	if (index < 3)
	{
		vec = &tinfo->altpos;
	}
	else if (index < 6)
	{
		vec = &tinfo->altpos2;
	}
	else if (index < 9)
	{
		vec = &tinfo->dstpos;
	}
	else if (index < 12)
	{
		vec = &tinfo->altangle;
	}
	else
	{
		vec = &tinfo->altpos;							// shouldn't happen
	}
	index=index % 3;

	if (index == 0)												// x
		vec->x = value;
	else if (index == 1)										// y
		vec->y = value;
	else if (index == 2)										// z
		vec->z = value;
}

//---------------------------------------------------------------------------
// addTarget()
//---------------------------------------------------------------------------
void metamaser_addTarget (userEntity_t *self, userEntity_t *other)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    int i;

    // target already in list?
    for ( i = 0; i < META_MAX_TARGETS; i++ )
        if ( hook->targets [i].target == other )
            return;

        // find slot for target, and add to list
    for ( i = 0; i < META_MAX_TARGETS; i++ )
    {
        if ( hook->targets [i].target == NULL )
        {
            hook->targets [i].target = other;
            hook->targets [i].end_track_time = gstate->time + META_TRACK_TIME;
            hook->targets [i].next_sound_time = gstate->time;
            hook->targets [i].next_damage_time = gstate->time;
			hook->numTargets++;
            return;
        }
    }
}

//---------------------------------------------------------------------------
// removeTarget()
//---------------------------------------------------------------------------
void metamaser_removeTarget (userEntity_t *self, userEntity_t *other)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

    int i;

    for ( i = 0; i < META_MAX_TARGETS; i++ )
    {
        if ( hook->targets [i].target == other )
        {
            hook->targets [i].target = NULL;
            hook->targets [i].end_track_time = 0.0;
            hook->targets [i].next_sound_time = 0.0;
            hook->targets [i].next_damage_time = 0.0;          
			hook->numTargets--;
            return;
        }
    }
}


//---------------------------------------------------------------------------
// update_frames()
//---------------------------------------------------------------------------
void metamaser_update_flags(userEntity_t *self, bool bNoTracks)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	trackInfo_t *tinfo = &hook->tinfo;

	// this will happen if the thing gets whooped before it hits something.
	if (!tinfo->ent)
		return;

	// basic stuff that has to get repeated
	tinfo->flags = TEF_FXFLAGS|TEF_NUMFRAMES|TEF_MODELINDEX;

	// stuff that depends on what flags are valid were made
	if (!bNoTracks)
	{
		if (tinfo->numframes & SET(0))
			tinfo->flags |= TEF_ALTPOS;
		if (tinfo->numframes & SET(1))
			tinfo->flags |= TEF_ALTPOS2;
		if (tinfo->numframes & SET(2))
			tinfo->flags |= TEF_DSTPOS;
		if (tinfo->numframes & SET(3))
			tinfo->flags |= TEF_ALTANGLE;
	}

	if (tinfo->numframes & bits[14])
		tinfo->flags |= TEF_SCALE;

	com->trackEntity(tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// meta_remove()
//---------------------------------------------------------------------------
void metamaser_meta_remove (userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	laserTarget_t *lhook;
   // add some cool effect here for when the metamaser grenade is removed
	for (int i = 0; i < META_MAX_LASERS; i++)
	{
		if (hook->lasers[i])
		{
			if (lhook = (laserTarget_t *)(hook->lasers[i]->userHook))
			{
				lhook->live = false;
				hook->lasers[i] = NULL;
			}
		}
	}

	metaCount--;
	RELIABLE_UNTRACK(self);
    self->remove (self);
}

//---------------------------------------------------------------------------
// diethink()
// series of animations and effects for destruction of the cube.
//---------------------------------------------------------------------------
void metamaser_diethink (userEntity_t *self)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	if (!(hook->tinfo.numframes & bits[14]))		// first time in die
	{
		hook->tinfo.numframes = bits[14];			// shut off everything else
		hook->remove_time = gstate->time + META_DIE_TIME;
		hook->tinfo.scale = 6;
		metamaser_update_flags(self,true);
		hook->pain_end_time = gstate->time;			// make an explosion now!
		hook->next_pain_level = 0;					// how big is the explosion?
	}
	
	if ((gstate->time > hook->remove_time) || (self->health < -20000))
	{
		// big friggin bang
		metamaser_meta_remove(self);
		return;
	}

	if (gstate->time >= hook->pain_end_time)		// generate an explosion/shockring
	{
		hook->tinfo.numframes |= bits[15];
//		float scale = 0.5 + 0.05*hook->next_pain_level;
		hook->tinfo.scale = 6 + 2*hook->next_pain_level;

		hook->next_pain_level += 1;

		metamaser_spawn_metaring(self);
		metamaser_spawn_laser_targets(self,META_MAX_LASERS);
		hook->pain_end_time = gstate->time + (.25 + 0.75*frand());;	// set up to make another
	}
	metamaser_update_flags(self,false);
	hook->tinfo.numframes &= ~bits[15];								// shut this off now..

	self->think = metamaser_diethink;
	self->prethink = NULL;
	self->nextthink = gstate->time + 0.1;
	self->pain = NULL;
}

//---------------------------------------------------------------------------
// spawn_metaring()
// Spawns an entity to track the rings and deal damage accordingly
//---------------------------------------------------------------------------
void metamaser_spawn_metaring(userEntity_t *self)
{
	userEntity_t *ent;
	metaring_t *hook;
	CVector angle;
    metamaserHook_t *mhook=(metamaserHook_t *)self->userHook;
	if (!mhook)
		return;

	self->s.angles.AngleToVectors(forward,right,up);
	VectorToAngles(up,angle);
	winfoClientScaleAlphaRotate(winfo,self->s.origin+5*up,angle,CVector(0,0,5),1.1,META_RING_MAX_SCALE + 0.5,MDL_SHOCKRING,2, META_RING_TIME);
	winfoClientScaleAlphaRotate(winfo,self->s.origin+5*up,angle,CVector(0,0,0),1,META_RING_MAX_SCALE,MDL_SHOCKRING,2, META_RING_TIME);
	winfoClientScaleAlphaRotate(winfo,self->s.origin+5*up,angle,CVector(0,0,-5),.9,META_RING_MAX_SCALE - 0.5,MDL_SHOCKRING,2, META_RING_TIME);

	ent = gstate->SpawnEntity ();	
	ent->flags |= FL_NOSAVE;
	ent->owner = self;
	ent->movetype = MOVETYPE_HOVER;
	ent->solid = SOLID_NOT;
	ent->s.render_scale.Set(0.001,0.001,0.001);
	ent->think = metaring_think;
	ent->s.origin = self->s.origin;
	ent->className = "metaring";
	winfoSetModel(ent,MDL_METACUBE);

	ent->userHook = gstate->X_Malloc(sizeof(metamaserHook_t),MEM_TAG_HOOK);
	hook=(metaring_t *)ent->userHook;
	hook->rmin = 0;
	hook->rmax = 15;
	hook->start_time = gstate->time;
	hook->owner = mhook->owner;

	gstate->LinkEntity(ent);
	ent->nextthink = gstate->time + 0.1;
}

//---------------------------------------------------------------------------
// metaring_think()
// Finds entities to damage.  Positions were eyeballed but look pretty good
//---------------------------------------------------------------------------
void metaring_think(userEntity_t *self)
{
	metaring_t *hook=(metaring_t *)self->userHook;

	if (!hook || (gstate->time > hook->start_time + META_RING_TIME))
	{
		gstate->RemoveEntity(self);
		return;
	}
	// update the position
    float perc=(gstate->time-hook->start_time)/META_RING_TIME;
    hook->rmax=perc*META_RING_MAX_SCALE * META_RING_INIT_RAD;
	hook->rmin=hook->rmax - 25;

    userEntity_t *head = gstate->FirstEntity ();
	float min = hook->rmin;
	float max = hook->rmax;
	float dist;
	float damage;

	// deal any applicable damage
    while ( head )
    {
        if ( head != self->owner && head->takedamage && (fabs(self->owner->s.origin.z - head->s.origin.z) < 64 ))
        {
			dist = (self->owner->s.origin - head->s.origin).Length();
			if ((dist > min) && (dist < max))
			{
                if ( com->Visible(self->owner, head) )
				{
					damage = META_RING_DAMAGE * winfo->damage * (META_RING_MAXDIST - dist)/META_RING_MAXDIST;
					if (damage < 0)
						damage = -damage;
					if (head == hook->owner)
						damage *= 0.5;
					
					CVector pushdir=head->s.origin-self->owner->s.origin;
					pushdir.Normalize();
					if ( pushdir.z < .4 && pushdir.z > -0.1 )
						pushdir.z = .4;

					if (!stricmp(head->className,"metacube"))			// busting up another metacube..gib it 
						damage = 32000;

					// cek[1-24-00]: set the attackweapon
					winfoAttackWeap;

					com->Damage(head,self->owner,hook->owner,head->s.origin,pushdir,damage,DAMAGE_EXPLOSION|DAMAGE_DIR_TO_INFLICTOR);

					metamaser_push_entity(head,pushdir,50*damage);
				}

			}
		}
        head = gstate->NextEntity (head);
	}

	self->nextthink = gstate->time + 0.05;
}

//---------------------------------------------------------------------------
// spawn_laser_targets()
// Makes count 'target entities' for the tracker attached to the metacube.  
// basically a really nasty laser light show.
//---------------------------------------------------------------------------
void metamaser_spawn_laser_targets(userEntity_t *self, int count)
{
	userEntity_t *laserent;
	laserTarget_t *hook;
    metamaserHook_t *mhook=(metamaserHook_t *)self->userHook;
	if (!mhook)
		return;

	while (count--)
	{
		laserent = gstate->SpawnEntity ();	
		laserent->flags |= FL_NOSAVE;
		laserent->hacks = mhook->remove_time;				// remove time
		laserent->owner = self;
		laserent->movetype = MOVETYPE_HOVER;
		laserent->solid = SOLID_NOT;
		laserent->s.render_scale.Set(0.001,0.001,0.001);
		laserent->think = metamaser_laser_target_think;
		laserent->s.origin = self->s.origin;
		laserent->className = "laser target";
		winfoSetModel(laserent,MDL_METACUBE);

		// keep track of which target we are!
		laserent->userHook = gstate->X_Malloc(sizeof(metamaserHook_t),MEM_TAG_HOOK);
		hook=(laserTarget_t *)laserent->userHook;
		hook->index = count;
		hook->live = true;

		gstate->LinkEntity(laserent);
		laserent->nextthink = gstate->time + 0.1 + (0.1*count);
		mhook->lasers[count] = laserent;
	}
}

//---------------------------------------------------------------------------
// laser_target_think()
// will either attach itself to the laser tracker or find something to fire a
// laser at.  Does lots of damage and knockaround
//---------------------------------------------------------------------------
void metamaser_laser_target_think(userEntity_t *self)
{
    laserTarget_t *hook=(laserTarget_t *)self->userHook;
    metamaserHook_t *mhook=(metamaserHook_t *)self->owner->userHook;
	trace_t tr;

	if ((gstate->time > self->hacks) || !mhook || !hook || !hook->live)
	{
		gstate->RemoveEntity(self);
		return;
	}

	winfoPlayAttn(self,SND_LGFIRE1 + (rand() & 1),1.0,512,2048);
	bool bFoundTarget = false;
	float r = frand();
	if (r < 0.025)		// be really sneaky and try to target something
	{
		userEntity_t *head = gstate->FirstEntity ();
		while (head != NULL)
		{
			if ( head != self->owner && head->takedamage && com->Visible(head,self->owner))
			{
				CVector start = self->owner->s.origin;
				CVector end = head->s.origin;
				CVector dir = end - start;
				tr = gstate->TraceLine_q2(start, end + dir, self, MASK_SHOT);
				if ( tr.ent && ( tr.ent->takedamage ))
				{
					bFoundTarget = true;
					dir = (end-start);
					dir.Normalize();
					metamaser_set_target(hook->index,tr.ent->s.number,&mhook->tinfo);
					// cek[1-24-00]: set the attackweapon
					winfoAttackWeap;

					com->Damage(tr.ent,self->owner,mhook->owner,zero_vector,zero_vector,META_END_LASER_DAMAGE*winfo->damage,DAMAGE_NONE);
					metamaser_push_entity(tr.ent,dir,750);	// slam him!
					break;
				}

			}
			head=gstate->NextEntity(head);
		}
	}

	if (!bFoundTarget)
	{
		CVector start(self->owner->s.origin), ang,end;
		ang = self->owner->s.angles;
		AngleToVectors(ang,forward,right,up);
		VectorToAngles(up,ang);
		ang.x += (180*(frand() - 0.5));
		ang.y += (360*(frand() - 0.5));
		ang.AngleToForwardVector(forward);
		end = start + 4000 * forward;
		tr = gstate->TraceLine_q2(start, end, self, MASK_SHOT);
		self->s.origin = tr.endpos;
		self->s.old_origin = self->s.origin; 
		
		// hit anything?
		if ( tr.ent && ( tr.ent->takedamage ))
		{
			// do some damage
			CVector dir = (end-start);
			dir.Normalize();
			metamaser_set_target(hook->index,tr.ent->s.number,&mhook->tinfo);
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

			com->Damage(tr.ent,self->owner,mhook->owner,zero_vector,zero_vector,META_END_LASER_DAMAGE*winfo->damage,DAMAGE_NONE);
			metamaser_push_entity(tr.ent,dir,750);	// slam him!
		}
		else
		{
			metamaser_set_target(hook->index,self->s.number,&mhook->tinfo);
		}
	}
//	update_flags(self->owner,false);

	// do a trace and see if we hit something that takes damage
	// if we did, set the tracker to that entity, 
	// otherwise, set the tracker to the laserent
	self->nextthink = gstate->time + 5;
}

//---------------------------------------------------------------------------
// die()
//---------------------------------------------------------------------------
void metamaser_die(struct edict_s *self, struct edict_s *inflictor, struct edict_s *attacker, int damage, CVector &point)
{
	metamaser_diethink(self);
}

//---------------------------------------------------------------------------
// pain()
//---------------------------------------------------------------------------
void metamaser_pain(struct edict_s *self, struct edict_s *other, float kick, int damage)
{
    metamaserHook_t *hook=(metamaserHook_t *)self->userHook;
	if (!hook)
		return;

	// if we get whooped, shut down for a bit

    if ( gstate->time >= hook->remove_time || !hook->charges )
        metamaser_diethink(self);

	if (self->health > hook->next_pain_level)
		return;
	
	// turn on the pain bit (13)
	hook->next_pain_level = self->health - META_HEALTH_LEVEL;
	hook->tinfo.numframes |= bits[13];
	metamaser_update_flags(self,true);			// just the flags, maam
	hook->pain_end_time = gstate->time + META_PAIN_TIME;
	// play some cool sparky sound here
}

//---------------------------------------------------------------------------
// push_entity()
// knock the crap out of the given entity
//---------------------------------------------------------------------------
void metamaser_push_entity(userEntity_t *ent, CVector &push, float magnitude)
{
	if ((push.z < 0.4) && (push.z > -0.1))
		push.z = 0.4;
	CVector vel=ent->velocity;
	vel = vel + magnitude*push;
	ent->velocity = vel;

	ent->groundEntity = NULL;
}

//-----------------------------------------------------------------------------
//	weapon_metamaser
//-----------------------------------------------------------------------------
void weapon_metamaser(userEntity_t *self)
{
	if (deathmatch->value)
		winfo->initialAmmo = 1;

    winfoSetSpawn(winfo,self,60.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_metamaser
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_metamaser (userEntity_t *self)
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
void weapon_metamaser_register_func()
{
	gstate->RegisterFunc("metamaserthink",metamaser_think);
	gstate->RegisterFunc("metamaserwait",metamaser_wait);
	gstate->RegisterFunc("metamasertrack",metamaser_track);
	gstate->RegisterFunc("metamaserdiethink",metamaser_diethink);
	gstate->RegisterFunc("metamasermetaring_think",metaring_think);
	gstate->RegisterFunc("metamaserlaser_t_think",metamaser_laser_target_think);
	gstate->RegisterFunc("metamasersearch",metamaser_search);
	gstate->RegisterFunc("metamaseruse",metamaser_use);
	gstate->RegisterFunc("metamaserdie",metamaser_die);
	gstate->RegisterFunc("metamaserpain",metamaser_pain);
//	gstate->RegisterFunc("metamasertouch",metamaser_touch);
	gstate->RegisterFunc("metamaser_give",metamaser_give);
	gstate->RegisterFunc("metamaser_select",metamaser_select);
}
