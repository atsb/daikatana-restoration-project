// ==========================================================================
//
//  File:RockGat "Show up and Shoot Down!"
//  Contents:Uhhg... target_turret no' mo'
//  Author:N.S.S.
//
// ==========================================================================
#include "world.h"
#include "ai.h"
//#include "ai_utils.h"// SCG[1/23/00]: not used
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
//#include "MonsterSound.h"// SCG[1/23/00]: not used
#include "actorlist.h"
#include "ai_common.h"
//#include "ai_think.h"// SCG[1/23/00]: not used

/* ***************************** Local Variables *************************** */
// --- target_monster_spawn
#define	USE_ONCE	0x0001

// --- target_turret
#define	TURRET_TOGGLE	0x0001
#define	TURRET_ON		0x0001

enum turretState
{
	TS_DISABLED		= 0,
	TS_SCANNING		= 1,
	TS_RAISING		= 2,
	TS_LOWERING		= 3,
	TS_PASSIVESCAN	= 4

};

typedef struct	turretHook_s
{
	playerHook_t	phook;
	int				modelindex;		// default gun model
	int				height;			// default popup height
	int				soundmax;		//#sounds
	int				sound_up;		// sound when raising
	int				sound_down;		// sound when lowering
	int				hit_sound;		// sound when destroyed
	int				ammo_type;		// default ammo type (laser bolts)
	float			fire_rate;		// fire rate (time between firings)
	float			range;			// range of turret
	int				state;			// current turret state
	float			nextAttack;		// next time the turret can fire
	float			nextSndAtak;	// next time a sound can play for the attack
	float			basedmg;		// default base damage 
	float			rnddmg;			// default random damage
//	void			*userHook;      // userHook information
	int				sound[6];		// sound played when firing
} turretHook_t;

#define	TURRETHOOK(x) (int)&(((turretHook_t *)0)->x)
field_t turret_hook_fields[] = 
{
	{"modelindex",	TURRETHOOK(modelindex),		F_INT},
	{"height",		TURRETHOOK(height),			F_INT},
	{"soundmax",	TURRETHOOK(soundmax),		F_INT},
	{"sound_up",	TURRETHOOK(sound_up),		F_INT},
	{"sound_down",	TURRETHOOK(sound_down),		F_INT},
	{"hit_sound",	TURRETHOOK(hit_sound),		F_INT},
	{"ammo_type",	TURRETHOOK(ammo_type),		F_INT},
	{"fire_rate",	TURRETHOOK(fire_rate),		F_FLOAT},
	{"range",		TURRETHOOK(range),			F_FLOAT},
	{"state",		TURRETHOOK(state),			F_INT},
	{"nextAttack",	TURRETHOOK(nextAttack),		F_FLOAT},
	{"nextSndAtak",	TURRETHOOK(nextSndAtak),	F_FLOAT},
	{"basedmg",		TURRETHOOK(basedmg),		F_FLOAT},
	{"rnddmg",		TURRETHOOK(rnddmg),			F_FLOAT},
//	{"userHook",	TURRETHOOK(userHook),		F_RESTORE_AS_NULL},
	{"sound",		TURRETHOOK(sound),			F_INT},
	{NULL, 0, F_INT}
};

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_rockgat(userEntity_t *self);




///////////////////////////////////////////////////////////////////////////////
//
//	target_turret_findtarget
//
///////////////////////////////////////////////////////////////////////////////

bool rockgat_find_target (userEntity_t *self, bool bRotateAndAttack)
{
	userEntity_t	*pClient	= NULL, *pTarget = self->enemy;
	turretHook_t	*hook		= (turretHook_t*) self->userHook;
//	playerHook_t	*phook		=  (playerHook_t*) hook->userHook;// SCG[1/23/00]: not used

	bool			bInRange = FALSE;
//	trace_t			tr;// SCG[1/23/00]: not used

	// do we already have a target?
	if (pTarget && !(pTarget->flags & FL_NOTARGET))
	{
		// check and see if it's still in range
		if (VectorDistance(pTarget->s.origin, self->s.origin) > hook->range)
		{
			// don't target that one any more
			self->enemy = pTarget = NULL;
		}
	}

	if (!pTarget)
	{
		// find the closest client
		pClient = alist_FirstEntity (client_list);

		for (int i = 0; pClient != NULL; i++)
		{
			if (VectorDistance(pClient->s.origin, self->s.origin) < hook->range)
			{
				bInRange = TRUE;
				break;
			}
			// try the next one
			pClient = alist_NextEntity (client_list);
		}
		// we found one
		if (bInRange && !(pClient->flags & FL_NOTARGET))
			pTarget = pClient;
	}

	// no targets
	if (!pTarget)
	{
		return FALSE;
	}
	else if (bRotateAndAttack)
	{
		CVector angles;

		// lock on to this enemy
		self->enemy = pTarget;

		// turn the turret so it faces pTarget
		CVector targetPt = pTarget->absmin + pTarget->size * 0.5;
		CVector dir = targetPt - self->s.origin;
		dir.Normalize();

		// we don't want any pitch on the model.. just rotation
		float pitch = dir.z;
		float maxPitch = 0.35f;
		

		dir.z = 0.0f;

		VectorToAngles( dir, angles );
		self->s.angles = angles;

		// is it time for us to attack again?
		if (gstate->time < hook->nextAttack)
			return TRUE;

		// now do a trace and see if we can fire
//		userEntity_t *pIgnore = self;// SCG[1/23/00]: not used
		// if we can hit the target, fire
		if( AI_IsLineOfSight( self, self->enemy ) && !(self->enemy->deadflag & DEAD_PUNK) && pitch < maxPitch)
		{
			// play the attack sound
			//
			if(hook->nextSndAtak < gstate->time)
			{
				int n = rnd()* hook->soundmax;
				gstate->StartEntitySound(self, CHAN_AUTO, hook->sound[n],0.85f, 1000.0f, 1000.0f);
				hook->nextSndAtak = gstate->time + 0.22f;
			}

			//if (hook->sound)
			//	gstate->StartEntitySound(self, CHAN_AUTO, hook->sound, 1.0f, 1000.0f, 1024.0f);
			
			// actually fire the projectile
			ai_fire_curWeapon( self );
			// set the time for the next attack
			hook->nextAttack = gstate->time + hook->fire_rate + rnd()*0.03;
		}
		else
		{
			return FALSE;
		}
	}
	if(pTarget->deadflag & DEAD_PUNK)
		return FALSE;
	return TRUE;
}

userEntity_t *AI_FindNearClientOrSidekick( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////
//
//	target_turret_think
//
///////////////////////////////////////////////////////////////////////////////

void rockgat_think (userEntity_t *self)
{
	turretHook_t* hook = (turretHook_t*) self->userHook;

	switch (hook->state)
	{
		case TS_RAISING:
		{
			if (hook->sound_up)
				gstate->StartEntitySound(self, CHAN_AUTO, hook->sound_up,0.65f,  300.0f, 700.0);
			if (hook->height)
				com->AnimateEntity(self, 0, hook->height, FRAME_ONCE, 0.10);
			// give it a delay so that the turret can raise
			hook->nextAttack = gstate->time + 1.0;

			hook->state = TS_SCANNING;
			break;
		}
		case TS_LOWERING:
		{
			if (hook->sound_down)
				gstate->StartEntitySound(self, CHAN_AUTO, hook->sound_down, 0.65f,  300.0f, 700.0f);

			if (hook->height)
				com->AnimateEntity(self, hook->height-1, 0, FRAME_ONCE, 0.10);
			// if this turret can be toggled on and off, disable it.. 
			if (self->use)
				hook->state = TS_DISABLED;
			else
				hook->state = TS_PASSIVESCAN;				// scan while lowered

			break;
		}
		case TS_SCANNING:
		{
			if (!rockgat_find_target (self, TRUE))	// find target and fire
			{
				// only lower if this is a passive scanning turret
				if (!self->use && hook->nextSndAtak < gstate->time)
					hook->state = TS_LOWERING;
			}
			break;
		}
		case TS_PASSIVESCAN:
		{
			if (rockgat_find_target (self, FALSE))	// find target and raise
				hook->state = TS_RAISING;
			break;
		}
		case TS_DISABLED:	
		default:
			break;
	}

	// if the turret is currently disabled, don't bother with the think function
	if (hook->state == TS_DISABLED)
		self->think = NULL;
	else
		self->nextthink = gstate->time + 0.1;

	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//	rockgat_use
//
///////////////////////////////////////////////////////////////////////////////

void rockgat_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	turretHook_t* hook = (turretHook_t*) self->userHook;

	// turret needs to be turned off
	if (self->spawnflags & TURRET_ON)
	{
		self->spawnflags &= ~TURRET_ON;
		if (hook->height)
			hook->state = TS_LOWERING;
		else
			hook->state = TS_DISABLED;
	}
	else // now on...
	{
		self->spawnflags |= TURRET_ON;
		if (hook->height)
			hook->state = TS_RAISING;
		else
			hook->state = TS_SCANNING;

	}
	// let the state engine handle the state changes
	self->think = rockgat_think;
	self->nextthink = gstate->time + 0.1;

}
void	spawn_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);
void AI_HolyShitIdied(userEntity_t *self, userEntity_t *attacker, userEntity_t *inflictor);
///////////////////////////////////////////////////////////////////////////////
//
//	rockgat_die
//
///////////////////////////////////////////////////////////////////////////////
void rockgat_die( userEntity_t *self )
{
	//	major damage, so gib
	self->s.renderfx = RF_NODRAW;
	//Make Noise
	ai_gib_sound( self );

	// NSS[1/4/00]:
	AI_StartGibFest( self, self );
	
	AI_HolyShitIdied(self, self, self);
	spawn_sprite_explosion( self, self->s.origin, true );

	self->deadflag	 = DEAD_DEAD;
	
	self->pain		= NULL;
	self->use		= NULL;
	self->die		= NULL;
	self->think		= NULL;
	self->prethink	= NULL;
	self->postthink = NULL;
	self->remove	= NULL;

	alist_remove( self );
	gstate->RemoveEntity( self );
}

///////////////////////////////////////////////////////////////////////////////
//
//	rockgat_pain
//
///////////////////////////////////////////////////////////////////////////////
void rockgat_pain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
//	turretHook_t* hook = (turretHook_t*) self->userHook;// SCG[1/23/00]: not used
	self->health -= damage;
	if(self->health <= 0)
	{
		// SCG[1/31/00]: The commented out code below was moved to rockgat_die
/*
		//	major damage, so gib
		self->s.renderfx = RF_NODRAW;
		//Make Noise
		ai_gib_sound( self);

		// NSS[1/4/00]:
		AI_StartGibFest(self, attacker);
		
		spawn_sprite_explosion( self, self->s.origin, true );

		self->deadflag	 = DEAD_DEAD;
		
		self->pain		= NULL;
		self->use		= NULL;
		self->die		= NULL;
		self->think		= NULL;
		self->prethink	= NULL;
		self->postthink = NULL;

		alist_remove( self );
*/
		self->remove( self );
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//	monster_rockgat
//
///////////////////////////////////////////////////////////////////////////////
void rockgat_hook_save( FILE *f, edict_t *ent )
{
//	AI_SaveHook( f, ent, turret_hook_fields, sizeof( turretHook_t ) );

	// save the playerHook_t part
	AI_SavePlayerHook(f,ent);

	// save the turretHook_t part
	turretHook_t *tHook = (turretHook_t *)ent->userHook;
	com->AI_SaveHook(f,&tHook->modelindex, sizeof(turretHook_t) - sizeof(playerHook_t) );
}

//void SPAWN_CallInitFunction( userEntity_t *self, const char *szAIClassName );
void rockgat_hook_load( FILE *f, userEntity_t *ent )
{
	// load the playerhook part first
	AI_LoadPlayerHook(f,ent);

	// create the real turretHook.
	turretHook_t *tHook = (turretHook_t *)gstate->X_Malloc(sizeof(turretHook_t), MEM_TAG_HOOK);
	playerHook_t *pHook = (playerHook_t *)tHook;

	// copy the playerhook over to the new turrethook
	memcpy(pHook,ent->userHook,sizeof(playerHook_t));

	// free the ent's playerhook and replace it with the turrethook
	gstate->X_Free(ent->userHook);
	ent->userHook = tHook;

	// load the turrethook part
	com->AI_LoadHook(f,&tHook->modelindex, sizeof(turretHook_t) - sizeof(playerHook_t) );

	// do the rest of the init stuff.
	ent->s.modelindex3 = gstate->ModelIndex("models/global/me_mflash.dkm");//Muzzleflash model

	ent->inventory = gstate->InventoryNew (MEM_MALLOC);
	ent->curWeapon = ai_init_weapon (ent, tHook->basedmg, tHook->rnddmg, 0, 0, 950.0, tHook->range, CVector(0, 0, 0), "chaingun", chaingun_fire, ITF_BULLET);
	gstate->InventoryAddItem (ent, ent->inventory, ent->curWeapon);
	return;

	// free the entity's userhook
//	gstate->UnlinkEntity( ent );

//	memset (&ent->area, 0, sizeof(ent->area));

	// SCG[11/24/99]: Pre load tasks
	// SCG[11/26/99]: We need to just clear everything out and re-initialize the AI 
	// SCG[11/26/99]: for this entity.

	// SCG[11/24/99]: Load
	AI_LoadHook( f, ent, turret_hook_fields, sizeof( turretHook_t ) );
	turretHook_t* hook = (turretHook_t*) ent->userHook;

	ent->s.modelindex3 = gstate->ModelIndex("models/global/me_mflash.dkm");//Muzzleflash model

	ent->inventory = gstate->InventoryNew (MEM_MALLOC);
	ent->curWeapon = ai_init_weapon (ent, hook->basedmg, hook->rnddmg, 0, 0, 950.0, hook->range, CVector(0, 0, 0), "chaingun", chaingun_fire, ITF_BULLET);
	gstate->InventoryAddItem (ent, ent->inventory, ent->curWeapon);
/*
	userEntity_t	*pEnemy		= ent->enemy;
	float			fHealth		= ent->health;
	touch_t			touch		= ent->touch;
	think_t			think		= ent->think;
	blocked_t		blocked		= ent->blocked;
	use_t			use			= ent->use;
	pain_t			pain		= ent->pain;
	die_t			die			= ent->die;
	remove_t		remove		= ent->remove;
*/

	// cek[12-14-99] decrement the monster counter so these don't get double counted!
//	gstate->numMonsters--;
//	SPAWN_CallInitFunction( ent, ent->className );

/*
	ent->enemy			= pEnemy;
	ent->health			= fHealth;
	ent->touch			= touch;
	ent->think			= think;
	ent->blocked		= blocked;
	ent->use			= use;	
	ent->pain			= pain;	
	ent->die			= die;		
	ent->remove			= remove;
*/
}

void monster_rockgat (userEntity_t *self)
{
//	playerHook_t *phook;
	// set up the physics for this entity
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->netname = "monster_rockgat";
	self->health = 500;														// default health
	self->fragtype = FRAGTYPE_ROBOTIC|FRAGTYPE_NOBLOOD;						//Set it to be a Robotic fragtype
	self->className = "monster_rockgat";
	self->flags |= FL_MONSTER;

//	self->userHook = gstate->X_Malloc(sizeof(playerHook_t), MEM_TAG_HOOK);
	self->userHook = gstate->X_Malloc(sizeof(turretHook_t), MEM_TAG_HOOK);
	turretHook_t* hook = (turretHook_t*) self->userHook;
	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    
	_ASSERTE( szModelName );

	//Set the modelindex
	self->s.modelindex = gstate->ModelIndex( szModelName );

	AIATTRIBUTE_GetCSVFileName( self->className );

	//store the address of the playerHook
//	phook = (playerHook_t *) self->userHook;

	// alloc the original turret hook
//	self->userHook = gstate->X_Malloc(sizeof(turretHook_t), MEM_TAG_HOOK);
//	turretHook_t* hook = (turretHook_t*) self->userHook;

	self->save = rockgat_hook_save;
	self->load = rockgat_hook_load;

	//shuffle the new player hook around
//	hook->userHook = phook;

	hook->sound[0] = gstate->SoundIndex ("e1/e_rockgatshootmultia.wav");

	// init hook vals
	self->s.modelindex3 = gstate->ModelIndex("models/global/me_mflash.dkm");//Muzzleflash model
	hook->height		= 10;												// default popup height (frames)
	//hook->sound = gstate->SoundIndex ("e1/e_rockgatshootmultia.wav");		// sound when firing  (default)
	hook->sound_up		= gstate->SoundIndex ("doors/e1/lift3start.wav");	// sound when raising (default)
	hook->sound_down	= gstate->SoundIndex ("doors/e1/lift3stop.wav");	// sound when lowering (default)
	hook->hit_sound		= 0;												// sound when destroyed
	hook->ammo_type		= 0;												// default ammo type (laser bolts)
	hook->fire_rate		= 0.13f;											// default time between firings (def: 5 times per sec)
	hook->range			= 512.0f;											// default turret range
	hook->state			= TS_DISABLED;										// set default state
	hook->basedmg		= 1.0;												// default base damage value
	hook->rnddmg		=  1.0;												// default random damage value
	hook->soundmax		= 1;
	hook->nextSndAtak	= gstate->time;
	// loop through epairs
	if( self->epair )
	{
		for (int i = 0; self->epair[i].key; i++)
		{
			if (!stricmp (self->epair [i].key, "height"))
				hook->height = atoi(self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "frames"))
				hook->height = atoi(self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "sound"))
			{
				hook->sound[0] = gstate->SoundIndex (self->epair [i].value);
				hook->soundmax = 1;
			}
			else if (!stricmp (self->epair [i].key, "sound_up"))
				hook->sound_up = gstate->SoundIndex (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "sound_down"))
				hook->sound_down = gstate->SoundIndex (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "hit_sound"))
				hook->hit_sound = gstate->SoundIndex (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "ammo_type"))
				hook->ammo_type = atoi (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "fire_rate"))
				hook->fire_rate = atof (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "range"))
				hook->range = atof (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "basedmg"))
				hook->basedmg = atof (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "rnddmg"))
				hook->rnddmg = atof (self->epair [i].value);
			else if (!stricmp (self->epair [i].key, "health"))
				self->health = atof (self->epair [i].value);

		}
	}

	if (self->spawnflags & TURRET_TOGGLE)
	{
		hook->state = TS_DISABLED;
		self->spawnflags &= ~TURRET_ON;
		self->use = rockgat_use;
		self->think = NULL;
	}
	else
	{
		hook->state = TS_PASSIVESCAN;
		self->use = NULL;
		self->think = rockgat_think;
		self->nextthink = gstate->time + 0.1;
	}

	// initialize the model's frame
	self->s.frame = 0;
	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	// set up the clip mask
	self->clipmask = MASK_MONSTERSOLID;
    
	//set the mass
	self->mass = 1.0f;

	//	SVF_MONSTER so shots clip against it
	self->svflags |= SVF_MONSTER;

	//	DAMAGE_YES so that damage can be done
	self->takedamage = DAMAGE_YES;

	self->pain = rockgat_pain;

	self->remove = rockgat_die;

	// give this entity an inventory so he can use the monster weapons
	self->inventory = gstate->InventoryNew (MEM_MALLOC);
	self->curWeapon = ai_init_weapon (self, hook->basedmg, hook->rnddmg, 0, 0, 950.0, hook->range, CVector(0, 0, 0), "chaingun", chaingun_fire, ITF_BULLET);
	gstate->InventoryAddItem (self, self->inventory, self->curWeapon);

    //set the size of the object.
    gstate->SetSize (self, -16, -16, -16, 16, 16, 16);
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);



}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_rockgat_register_func()
{
	gstate->RegisterFunc("rockgat_think",rockgat_think);
	gstate->RegisterFunc("rockgat_use",rockgat_use);
	gstate->RegisterFunc("rockgat_pain",rockgat_pain);
	gstate->RegisterFunc("rockgat_hook_save",rockgat_hook_save);
	gstate->RegisterFunc("rockgat_hook_load",rockgat_hook_load);
	gstate->RegisterFunc("rockgat_die",rockgat_die);
}
