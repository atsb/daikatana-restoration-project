#include "weapons.h"
#include "c4.h"

enum
{
    // model defs
    MDL_START,
    MDL_GLOB,

    // sound defs
    SND_START,
    SND_EXPLODE,
    SND_BEEP,
    SND_BEEP_WARNING,
    SND_STICK,
    SND_STICK_METAL,
    SND_STICK_WOOD,
    SND_STICK_STONE,
    SND_STICK_GLASS,
    SND_STICK_ICE,
    SND_STICK_SNOW,
    SND_JAM,

    // special frame defs
    ANIM_START,
    ANIM_PUSHBUTTON
};

// weapon info def
weaponInfo_t c4WeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_C4),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_C4_AMMO),
	// names, net names, icon
	"weapon_c4",
	tongue_weapons[T_WEAPON_C4],
	"ammo_c4",
	tongue_weapons[T_WEAPON_C4_AMMO],
	"iconname",

    // model files
    {
		"models/e1/w_c4.dkm",
		"models/e1/a_c4.dkm",
		"models/e1/wa_c4.dkm",
		"models/e1/we_c4prj.dkm",
		NULL
	},

    // sound files
    {
		SND_NOSOUND,
		"e1/we_c4ready.wav",		// ready
		"e1/we_c4away.wav",		// away 
		"e1/we_c4shoota.wav",     // fire
		SND_NOSOUND, //"e1/we_c4amba.wav",		// ambient 1 -- cek - this particular anim  doesn't do anything
		"e1/we_c4ambb.wav",		// ambient 2
		SND_NOSOUND,
		"null.wav",
		"e1/we_c4beepa.wav",		// away 
		"e1/we_c4beepb.wav",
		"e1/we_c4cona.wav",		// fire (2?)
		"e1/we_c4metala.wav",
		"e1/we_c4wooda.wav",
		"e1/we_c4stonea.wav",
		"e1/we_c4glassa.wav",
		"e1/we_c4icea.wav",
		SND_NOSOUND, //"e1/we_c4snowa.wav",
		"e1/we_c4jam.wav",
		NULL
	},

    // frame names
    {
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0, // doesn't do anything sept beep
		-1,"ambb",0,0,
		ANIM_NONE,
		-1,"btnpsh",0,0,
		0,
		NULL
	},

    // commands
    { 
		wpcmds[WPCMDS_C4][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_3,
		wpcmds[WPCMDS_C4][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_3,
		"c4_detonate",(void (*)(struct edict_s *))c4Detonate,
		NULL
    },

	c4Give,
	c4Select,
	c4Command,
	c4Use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(c4_t),

	300,
	6,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	10,     //	initial ammo
	1800.0, //	speed
	512.0,  //	range
	250.0,       //	damage - radius
	10.0,       //	lifetime

	WFL_CROSSHAIR|WFL_SELECT_EMPTY|WFL_PLAYER_ONLY,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
/*
	{
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_C4],
		NULL,
		ATTACKER_VICTIM
	},*/
	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_C4),
	ATTACKER_VICTIM,

	WEAPON_C4,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&c4WeaponInfo;
static const float fProximity = 150;   // How close someone can wander before exploding
static const float fC4Proximity = 200; // How close another C4 nodule can be for a chain reaction;
static float touchradius = 300;
void c4Die(userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector & origin);
listHeader_t *c4_list;  

#define MAX_C4_DEPLOYMENT 4

void C4ClearList()
{
	if( c4_list != NULL )
	{
		com->list_kill( c4_list );
		c4_list = NULL;
	}
}

int C4Count(userEntity_t *owner)
{
    int i;
    listNode_t *cur_node;
    userEntity_t *cur_ent;

    if ( !c4_list )
        return 0;

    cur_node=c4_list->head;

    i=0;
    while ( cur_node )
    {
        cur_ent=(userEntity_t *)com->list_ptr(cur_node);
	    moduleHook_t *hook = (moduleHook_t*)cur_ent->userHook;
		if (!hook)
		{
	        cur_node=com->list_node_next(cur_node);
			continue;
		}

        // only detonate if owner detonates me
        if (hook->owner == owner)
            i++;
        // go to the next node
        cur_node=com->list_node_next(cur_node);
    }
    return i;
}

///////////////////////////////////////////////////////////////////////
// RadiusKick  (CVector &epicenter, 
//             int severity, 
//             float radius)
//
// Kick the shit out of clients within a radius. As client's distance 
// from the epicenter increases, the kick effect decreases. 
///////////////////////////////////////////////////////////////////////
void RadiusKick (CVector &veEpicenter, int iSeverity, float fRadius)
{
    float fDistance, fKickGradient;
    userEntity_t *ent_shaken;
    CVector veDistance, veKickAngles;

    fDistance = 0;
    fKickGradient = 0;

    // PreCalc kick angle randoms (don't bother repeating for everyone)
    veKickAngles.x = rnd() - 0.5;
    veKickAngles.y = rnd() - 0.5;
    veKickAngles.z = rnd() - 0.5;

    // Loop through ents, shake what can be shaken

//    for ( ent_shaken = gstate->FirstEntity();ent_shaken;ent_shaken = gstate->NextEntity(ent_shaken) )
    for( int i = 1; i <= gstate->game->maxclients; i++ )
    {
		ent_shaken = &gstate->g_edicts[i];

        // Skip these
        if ( !ent_shaken->inuse )
            continue;

        // Is it a client?
        if ( !(ent_shaken->flags & FL_CINEMATIC) && ent_shaken->flags & FL_CLIENT && ent_shaken->client )
        {
            // get distance info, perhaps no quaking necessary (bah!)
            veDistance = ent_shaken->s.origin - veEpicenter;
            fDistance = veDistance.Length();

            if ( fDistance > fRadius )
                continue;

            // OK, client shakes, how bad?
            fKickGradient = (fRadius - fDistance) / fRadius;
            // Shake the client
            veKickAngles = veKickAngles * ((iSeverity / 50) * fKickGradient);
            weapon_kick(ent_shaken, veKickAngles, veKickAngles.x * 500 - 250, 200, 200);
        }
    }
}

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *c4Give(userEntity_t *self, int ammoCount)
{
    // init the c4 list the first time someone picks up the weapon
	if (!c4_list)
        c4_list=com->list_init(c4_list);

    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// c4_select()
//---------------------------------------------------------------------------
short c4Select(userEntity_t *self)
{
    short rt=weaponSelect(self,winfo);

    if ( rt==WEAPON_ACTIVE )
    {
        // play button press animation
        winfoAnimate(winfo,self,ANIM_PUSHBUTTON,FRAME_ONCE|FRAME_WEAPON,.05);
        c4Detonate(self,0);
    }

    return(rt);
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *c4Command(struct userInventory_s *inv, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
//	c4_remove
//
//	cleanup function for c4 module
//---------------------------------------------------------------------------

void    c4Remove (userEntity_t *self)
{
	moduleHook_t *hook=(moduleHook_t *)self->userHook;
	if (hook)
	{
		// kill my trigger
		if (hook->trigger)
		{
			hook->trigger->remove(hook->trigger);
			hook->trigger = NULL;
		}

		// get rid of the friction hook memory if it is there
		com->friction_remove (hook->fhook);

		// remove this c4 module from the list of active c4 modules
		if(c4_list->head && c4_list->tail)
		{
			com->list_remove_node (self, c4_list);
		}
	}

    //	free the userHook and edict
    gstate->RemoveEntity (self);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void c4Use(struct userInventory_s *ptr, userEntity_t *self)
{
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    if ( !weaponHasAmmo(self,true) )
        return;
    // OK launch 'em
    weaponUseAmmo (self, true);

    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
    winfoPlay(self,SND_SHOOT_STD,1.0f);

    c4Launch(self);
}


//---------------------------------------------------------------------------
// launch()
//---------------------------------------------------------------------------
void c4Launch(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    moduleHook_t *hook;
    userEntity_t *module;
    CVector vel,origin;
    phook->fxFrameFunc = NULL;

    // spawn a new c4 module
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    module=weapon_spawn_projectile(self,offset, winfo->speed, 0,"projectile_c4", FALSE);
    
    module->avelocity.z = 1440;

    module->die = c4Die;
    // set up the userHook for the c4 module
    module->userHook = gstate->X_Malloc(sizeof(moduleHook_t),MEM_TAG_HOOK);
    hook = (moduleHook_t*)module->userHook;

    // set up frictionHook
    hook->fhook = com->friction_init(module, hook->fhook);
    com->friction_set_physics(module,hook->fhook,.5,1.5,1000,FR_SPIRAL);

    // make the model visible
    winfoSetModel(module,MDL_GLOB);
    gstate->SetSize(module,-8,-8,-8,8,8,8);

    module->movetype = MOVETYPE_BOUNCE;
    module->size = module->absmax - module->absmin;
	module->flags |= FL_EXPLOSIVE;

    // set max detonation time and beep time
    hook->time_to_die = gstate->time + winfo->lifetime;
    hook->beep_time = gstate->time+C4_BEEP_DELAY;
    hook->warning_beep_frequency = 0;
    hook->last_warning_beep = gstate->time;

    module->think = c4Think;
    module->nextthink = gstate->time+0.05;
    module->touch = (touch_t)c4Touch;
    module->remove = c4Remove;
    module->takedamage = DAMAGE_YES;
    module->health = 5;
	module->owner = self;
	hook->owner = self;

    // add this module to the global list of c4 modules
	com->list_add_node(module,c4_list,0);
    invWeapon->lastFired=gstate->time;
    gstate->LinkEntity(module);

	WEAPON_FIRED(self);
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void c4Think(userEntity_t *self)
{
    moduleHook_t *hook=(moduleHook_t *)self->userHook;
	if (!hook)
	{
		gstate->RemoveEntity(self);
		return;
	}

    userEntity_t *ent_Proximity;
    int i;
    float f;

    // slight random error in aim
    if(self->velocity.x)
    {
	    self->velocity.x+=(rnd()-0.5) * 80;
        self->velocity.y+=(rnd()-0.5) * 80;
        self->velocity.z+=(rnd()-0.5) * 20;
    }

	//[CEK-11/2/99] make sure it dies after its lifetime is over
    if( (i=C4Count(hook->owner) > MAX_C4_DEPLOYMENT) || (gstate->time > hook->time_to_die))
    {
        // Destabalized, a chance it will go BOOM
        f=frand();
        if(((gstate->time > hook->time_to_die)) || (f <= 0.05))
        {
            c4Detonate(hook->owner,1);
			self->nextthink = gstate->time + 0.1;
			return;
        } 
		else if (f > 0.05) 
		{
            winfoPlay(self,SND_BEEP,1.0f);
        }
    }
    ent_Proximity = NULL;
	// [CEK 11-18-99] the trigger was a good idea but monsters weren't triggering it... do it the hard way... :/
	if (hook->onWorld)
	{
		CVector dir;
		float dist=4096,temp;
		while ((ent_Proximity = com->FindRadius(ent_Proximity, self->s.origin, touchradius)) != NULL )
		{
			if (ent_Proximity->flags & (FL_CLIENT|FL_MONSTER|FL_BOT) && 
				com->Visible(self,ent_Proximity))
			{
				dir = self->s.origin - ent_Proximity->s.origin;
				temp = dir.Length();
				if (temp < dist)
				{
					dist = temp;
					if (temp < fProximity)		// kaboom!
						break;
				}
			}
		}

		if (dist < touchradius)
		{
			if (dist < fProximity)
			{
				c4Explode(self);
				return;
			}
			else
				hook->warning_beep_frequency = dist * 0.002;

		}
		else
			hook->warning_beep_frequency = 0;
	}

    if(hook->warning_beep_frequency && hook->last_warning_beep + hook->warning_beep_frequency < gstate->time)
    {
//		trackAddEffect(self,0,TEF_ALPHA_SPRITE|TEF_LIGHT|TEF_FX_ONLY);
		self->s.effects2 |= EF2_C4_BEEP;
        if(hook->warning_beep_frequency > 0.15)
            winfoPlay(self,SND_BEEP,1.0f);
        else
            winfoPlay(self,SND_BEEP_WARNING,1.0f);
        hook->last_warning_beep = gstate->time;
    }
	else
		self->s.effects2 &= ~EF2_C4_BEEP;
//		trackRemoveEffect(self,TEF_ALPHA_SPRITE|TEF_LIGHT);


    self->think=c4Think;
    self->nextthink=gstate->time+.1;
}

void c4TouchStuck(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
	if (!stricmp(other->className,"projectile_missile") || !stricmp(other->className,"projectile_c4"))
	{
		self->touch = NULL;
		c4Explode(self);
	}
}

//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void c4Touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
	bool bStuck = FALSE;

	moduleHook_t *hook=(moduleHook_t *)self->userHook;
	if (!hook)
		return;

    // don't stick to self or sky
    if ((hook->owner == other) || (surf && (surf->flags & SURF_SKY)))
        return;

    // Kaboom mobs
    if( other->takedamage || (other->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)))
    {
        if(surf && surf->flags & SURF_GLASS)
        {
            com->Damage(other, self, hook->owner, self->s.origin, other->s.origin - self->s.origin, 32000, DAMAGE_INERTIAL);
            return;
        }
        c4Explode(self);
        return;
    }

	// play the stick sound
	if (surf)
	{
		if(surf->flags & SURF_METAL)
			winfoPlay(self,SND_STICK_METAL,1.0f);
		else if (surf->flags & SURF_WOOD)
			winfoPlay(self,SND_STICK_WOOD,1.0f);
		else if (surf->flags & SURF_GLASS)
			winfoPlay(self,SND_STICK_GLASS,1.0f);
		else if (surf->flags & SURF_ICE)
			winfoPlay(self, SND_STICK_ICE, 1.0f);
		else if (surf->flags & SURF_SNOW)
			winfoPlay(self, SND_STICK_SNOW, 1.0f);
		else
			winfoPlay(self, SND_STICK, 1.0f);
	}

	// we're stuck to wall, or entity ... just sit there
	// (needs to be done before we link to a parent entity)
	CVector lastdir = self->velocity;
	lastdir.Normalize();
	self->velocity.Zero();
	self->avelocity.Zero();

	// attach c4 to a bmodel as a child entity
	if (other && other->s.solid == 31)  // 19990311 JAR - please explain this
	{
		// make sure a child can not take on it's own list of entities
		if (other->teammaster)
		{
			// let it fall to the ground
			self->movetype = MOVETYPE_TOSS;
		}
		else
		{
			userEntity_t *pCur = other, *pLast = NULL;
			// step through the teamlist and attach 'self' to the end
			while (pCur)
			{
				pLast = pCur;
				pCur = pCur->teamchain;		// go to next entity
			}
			pLast->teamchain = self;		// link the end to the current entity
			self->teamchain = NULL;			// just to make sure
			self->teammaster = other;		// point back to the parent
			self->flags |= FL_TEAMSLAVE;	// let it be known this is a child entity

			self->spawn_origin = self->s.origin;
			self->s.old_origin = self->s.origin;
			// initialize all the offsets from the parent
			self->childOffset = self->s.origin - other->s.origin;
			self->transformedOffset = self->childOffset;
			self->s.angle_delta = self->s.angles - other->s.angles;
		
			// we're stuck to a bmodel now...
			bStuck = TRUE;
		}
	}
	else
	{
		// stuck to a wall
		bStuck = TRUE;
	}
	
	// only change the touch functions etc if we're stuck to a wall or bmodel
	if (bStuck)	
	{
		// minimal physics... also allows parent child link to operate unhithered
		int i = 20;
		while (i-- && (gstate->PointContents(self->s.origin) & CONTENTS_SOLID))
			entBackstep(self,lastdir,4.0f);

		self->movetype = MOVETYPE_NOCLIP;//MOVETYPE_PUSH;

		// set the clip mask
		self->clipmask = CONTENTS_MONSTER;
        //self->clipmask = MASK_PLAYERSOLID;
        self->svflags = SVF_MONSTER;//SVF_DEADMONSTER;
		hook->onWorld = true;
		self->solid = SOLID_BBOX;
		self->touch = c4TouchStuck;		// kill the touch function so we don't spawn this shit all over
		self->nextthink = gstate->time + 1.0;	// slight delay
		self->owner = NULL;

		gstate->LinkEntity(self);

		// MIKE: this code really has no effect here...
		if (plane) 
            self->movedir = plane->normal;
	}
}


//---------------------------------------------------------------------------
// explode()
//---------------------------------------------------------------------------
void c4Explode(userEntity_t *self)
{
    moduleHook_t *hook=(moduleHook_t *)self->userHook;
	if (!hook)
	{
//		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}

    userEntity_t *ent_C4Near;
    int iCount;

    iCount = 0;
    ent_C4Near = NULL;

	hook->warning_beep_frequency = 0;
    // look for nearby entities, start a chain reaction!
    
    while ( (ent_C4Near = com->FindRadius(ent_C4Near, self->s.origin, fC4Proximity)) != NULL )
    {
        // if it is a C4 grenade, and it's still in good shape (ent data)
        if ( !stricmp(ent_C4Near->className, "projectile_c4") )
        {
            if ( ent_C4Near->userHook )
            {
                // blow it up too!
                // other_hook = (moduleHook_t *)ent_C4Near->userHook;
                ent_C4Near->think = c4Explode;
                ent_C4Near->nextthink = gstate->time + (iCount * 0.1);
                iCount++;
            }
        }
    }
    
    // do the damage, if there's a chain reaction, to a LOT of damage
    // com->RadiusDamage(self, self, self, winfo->damage + ( float(iCount * winfo->damage) * 0.5), DAMAGE_EXPLOSION);                        //Damage!

    // JAR can't team check if the owner isn't passed to com->Damage!
	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;
    int num = com->RadiusDamage(self, hook->owner, self, winfo->damage + ( float(iCount * winfo->damage) * 0.1), DAMAGE_EXPLOSION, fProximity * 2 );                        //Damage!
	WEAPON_HIT(hook->owner,num);
    winfoPlay(self,SND_EXPLODE,1.0f);

    CVector color( 1,.2,.2);
    spawnPolyExplosion(self->s.origin,self->movedir,1+iCount,250,color,PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );

    if ( iCount > 3 )
    {
        RadiusKick(self->s.origin, winfo->damage + (0.1 * winfo->damage * iCount), fProximity * 2);
    }

    // clean up, when the others explode, this one won't be counted in the chain reaction
    self->remove (self);
}

void c4Die(userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector & origin)
{
    if(!self->inuse || inflictor->className == self->className)
        return;
    self->takedamage = DAMAGE_NO;    
    c4Explode(self);
    self->die = NULL;
}


//---------------------------------------------------------------------------
// detonate()
//---------------------------------------------------------------------------
void c4Detonate(userEntity_t *owner, short staggered)
{
    float i;
    listNode_t *cur_node;
    userEntity_t *cur_ent;
    moduleHook_t *cur_entHook;

    if ( !c4_list )
        return;

    cur_node=c4_list->head;

    i=1;
    while ( cur_node )
    {
        cur_ent=(userEntity_t *)com->list_ptr(cur_node);
        cur_entHook=(moduleHook_t *)cur_ent->userHook;// SCG[1/24/00]: not used cek[1-28-00] is now.  :)
		if (!cur_entHook)
			continue;

        // only detonate if owner detonates me
        if (cur_entHook->owner == owner)
        {
            cur_ent->think = c4Explode; //time_to_die=gstate->time+((i * 0.05));      // + 0.05);
            cur_ent->nextthink  = gstate->time + 0.2*(i*staggered);
            i++;
        }

        // go to the next node
        cur_node=com->list_node_next(cur_node);
    }
}

/////////////////////////////////////
// c4_exit_level
//
// clean-up routines called when a
// level ends to reclaim all malloc'd
// memory
/////////////////////////////////////

void c4ExitLevel(userEntity_t *self)
{
    listNode_t      *cur_node;
//    listNode_t      *last_node;
    userEntity_t    *module;
//    moduleHook_t    *mhook;// SCG[1/24/00]: not used

    // traverse the list of c4 modules and remove all of their userhooks,
    // friction hooks and sticky hooks

    cur_node = c4_list->head;

    while ( cur_node )
    {
        module = (userEntity_t *) cur_node->ptr;
//        mhook = (moduleHook_t *) module->userHook;// SCG[1/24/00]: not used

        //	go to next node
        cur_node = com->list_node_next (cur_node);

        //	remove from list and free userHook and edict
        self->remove (self);
    }
}

//-----------------------------------------------------------------------------
//	weapon_c4viz
//-----------------------------------------------------------------------------
void weapon_c4viz (userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_c4
//-----------------------------------------------------------------------------
void ammo_c4 (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 8, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_c4_register_func()
{
	gstate->RegisterFunc("c4Think",c4Think);
	gstate->RegisterFunc("c4Explode",c4Explode);
	gstate->RegisterFunc("c4Touch",c4Touch);
//	gstate->RegisterFunc("c4Radiustouch",c4Radiustouch);
	gstate->RegisterFunc("c4Use",c4Use);
	gstate->RegisterFunc("c4Die",c4Die);
	gstate->RegisterFunc("c4Remove",c4Remove);
	gstate->RegisterFunc("c4TouchStuck",c4TouchStuck);

	gstate->RegisterFunc("c4Give",c4Give);
	gstate->RegisterFunc("c4Select",c4Select);
}
