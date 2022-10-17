#include "world.h"
#include "actorlist.h"

///////////////////////////////////////////////////////////////////////////////
// exports
///////////////////////////////////////////////////////////////////////////////

//DllExport	void	effect_fog	(userEntity_t *self);// SCG[1/23/00]: not used
//DllExport	void	effect_fire (userEntity_t *self);
DllExport	void	effect_snow	(userEntity_t *self);
DllExport	void	effect_rain (userEntity_t *self);
// SCG[2/13/00]: DllExport	void	effect_drip (userEntity_t *self);
DllExport	void	effect_lightning (userEntity_t *self);

DllExport   void    effect_steam (userEntity_t *self);

// SCG[11/19/99]: Save game stuff
#define	LIGHTNINGEFFECTHOOK(x) (int)&(((lightningEffectHook_t *)0)->x)
field_t lightning_effect_hook_fields[] = 
{
	{"attractorHead",	LIGHTNINGEFFECTHOOK(attractorHead),		F_EDICT},
	{"curAttractor",	LIGHTNINGEFFECTHOOK(curAttractor),		F_EDICT},
	{"color",			LIGHTNINGEFFECTHOOK(color),				F_VECTOR},
	{"totalAttractors",	LIGHTNINGEFFECTHOOK(totalAttractors),	F_INT},
	{"sound",			LIGHTNINGEFFECTHOOK(sound),				F_INT},
	{"delay",			LIGHTNINGEFFECTHOOK(delay),				F_FLOAT},
	{"duration",		LIGHTNINGEFFECTHOOK(duration),			F_FLOAT},
	{"dmg",				LIGHTNINGEFFECTHOOK(dmg),				F_FLOAT},
	{"scale",			LIGHTNINGEFFECTHOOK(scale),				F_FLOAT},
	{"chance",			LIGHTNINGEFFECTHOOK(chance),			F_FLOAT},
	{"gndchance",		LIGHTNINGEFFECTHOOK(gndchance),			F_FLOAT},
	{"modulation",		LIGHTNINGEFFECTHOOK(modulation),		F_FLOAT},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	EFFECTENTITYHOOK(x) (int)&(((effectEntityHook_t *)0)->x)
field_t effect_entity_hook_fields[] = 
{
	{"emitter",				EFFECTENTITYHOOK(emitter),				F_EDICT},
	{"dstEnt",				EFFECTENTITYHOOK(dstEnt),				F_EDICT},
	{"dstPos",				EFFECTENTITYHOOK(dstPos),				F_VECTOR},
	{"killtime",			EFFECTENTITYHOOK(killtime),				F_FLOAT},
	{"dmg",					EFFECTENTITYHOOK(dmg),					F_FLOAT},
	{"bCheckTraceDamage",	EFFECTENTITYHOOK(bCheckTraceDamage),	F_INT},
	{NULL, 0, F_INT}
};

///////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////

// for effect_lightning

enum
{
//	START_ON			=	0x0001,	// effect is toggled on and off
	LIGHT_ON			=	0x0001,	//
	CYCLE				=	0x0002,	// cycle through the attractors in 'triggerindex' order
	GROUND_STRIKES		=	0x0004,	// randomly strike the ground (see gndchance)
	RANDOM_DELAY		=	0x0008,	// randomize the delay between strikes
	SPAWN_LIGHT			=	0x0010,	// spawn a dynamic like at the pt of impact
	NO_CLIENTS			=	0x0020,	// do not strike the players
	TRACE_DMG			=	0x0040,	// electrocute the fucker if he walks through the bolt
	STRIKE_ONCE			=	0x0080,	// strike once when triggered
	NO_SPARKS			=	0x0100,	// don't put the little sparkles on the lightning
	CONSTANT_BOLT		=	0x0200,	// constant stream.. don't keep spawning entities
	LEAVE_SCORCH		=   0x0400	// leave a scorch mark on the ground or a bmodel
};

const int MAX_STRIKE_TRIES	= 6;	// number of tries to find a spot to strike

// for effect_drip

enum
{
	DRIP_BLOOD			=	0x0001,	// drip type is blood
	DRIP_SLUDGE			=	0x0002	// drip type is sludge
};

// for effect_rain

enum
{
	RAIN_NORTH			=	0x0001,	// it will rain to the north
	RAIN_SOUTH			=	0x0002,	// it will rain to the south
	RAIN_EAST			=	0x0004,	// it will rain to the east
	RAIN_WEST			=	0x0008,	// it will rain to the west
};

///////////////////////////////////////////////////////////////////////////////
// prototypes
///////////////////////////////////////////////////////////////////////////////

void com_FindParent (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
// effect_fire
///////////////////////////////////////////////////////////////////////////////

/*
void effect_fire (userEntity_t *self)
{
	int		i;
	float	height = 0;

	for (i = 0; self->epair[i].key != NULL; i++)
		if (!stricmp (self->epair[i].key, "height"))
			height = atof (self->epair[i].value);

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_FIRE;

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

//	self->s.mins [2] = self->s.maxs [2] - height;
}
*/

void lightning_effect_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, lightning_effect_hook_fields, sizeof( lightningEffectHook_t ) );
}

void lightning_effect_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, lightning_effect_hook_fields, sizeof( lightningEffectHook_t ) );
}

void effect_entity_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, effect_entity_hook_fields, sizeof( effectEntityHook_t ) );
}

void effect_entity_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, effect_entity_hook_fields, sizeof( effectEntityHook_t ) );
}

///////////////////////////////////////////////////////////////////////////////
// effect_fog
///////////////////////////////////////////////////////////////////////////////

/*
void effect_fog (userEntity_t *self)
{
	int		i;
	float	height = 0;

	for (i = 0; self->epair[i].key != NULL; i++)
	{
//		if (!stricmp (self->epair[i].key, "height"))
//		{
//			height = atof (self->epair[i].value);
//		}
	}

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_FOG;

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

}
*/


///////////////////////////////////////////////////////////////////////////////
// effect_snow
///////////////////////////////////////////////////////////////////////////////
// cek: win2k -- rain and snow volumes set to nosave
void effect_snow (userEntity_t *self)
{
	int		i;
	float	height = 0;

	for (i = 0; self->epair[i].key != NULL; i++)
	{
		if (!stricmp (self->epair[i].key, "height"))
		{
			height = atof (self->epair[i].value);
		}
	}

	// what direction will the snow be falling...
	self->s.skinnum = self->spawnflags;		// default is straight down (spawnflags == 0)

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->flags |= FL_NOSAVE;

	self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_SNOW;

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	self->s.mins.z = self->s.maxs.z - height;
}

///////////////////////////////////////////////////////////////////////////////
// effect_rain
///////////////////////////////////////////////////////////////////////////////

void effect_rain (userEntity_t *self)
{
	int		i;
	float	height = 0;

	for (i = 0; self->epair[i].key != NULL; i++)
	{
		if (!stricmp (self->epair[i].key, "height"))
		{
			height = atof (self->epair[i].value);
		}
	}

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->flags |= FL_NOSAVE;

	self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_RAIN;

	// what direction will the rain be falling...
	self->s.skinnum = self->spawnflags;		// default is straight down (spawnflags == 0)

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	self->s.mins.z = self->s.maxs.z - height;
}

///////////////////////////////////////////////////////////////////////////////
// effect_drip
///////////////////////////////////////////////////////////////////////////////
/*
void effect_drip (userEntity_t *self)
{
	int		i;
	float	height = 0;

	for (i = 0; self->epair[i].key != NULL; i++)
		if (!stricmp (self->epair[i].key, "height"))
			height = atof (self->epair[i].value);

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_DRIP;

	// what kind of drip type is this...
	if (self->spawnflags & DRIP_BLOOD)
		self->s.skinnum =	PARTICLE_DRIP_BLOOD;
	else 
	if (self->spawnflags & DRIP_SLUDGE)
		self->s.skinnum =	PARTICLE_DRIP_SLUDGE;
	else
		self->s.skinnum =	PARTICLE_DRIP_WATER;	// default is water drip

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	self->s.mins.z = self->s.maxs.z - height;
}
*/

///////////////////////////////////////////////////////////////////////////////
// check_trace_damage
///////////////////////////////////////////////////////////////////////////////

void check_trace_damage (userEntity_t *self)
{
	effectEntityHook_t	*hook = (effectEntityHook_t *) self->userHook;
	CVector dir, destPt;

	// get the destination point for the trace
	if (hook->dstEnt)
	{
		destPt = hook->dstEnt->s.origin;
	}
	else
	{
		destPt = hook->dstPos;
	}

	// perform the trace
	tr = gstate->TraceLine_q2(self->s.origin, destPt, self, MASK_SHOT);

	// if this is a player....or a sidekick
	if (tr.ent && (tr.ent->flags & (FL_CLIENT | FL_BOT)))
	{
		// damage the dstEnt
		dir = destPt - self->s.origin;
		//VectorSubtract(destPt, self->s.origin, dir );

		// hurt it if we can
		if ((tr.ent->takedamage) && (tr.ent != self->owner))
			com->Damage (tr.ent, self, self->owner, tr.endpos, dir, hook->dmg, DAMAGE_INERTIAL | DAMAGE_SPARKLES);
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
// check_scorch_mark
///////////////////////////////////////////////////////////////////////////////

void check_scorch_mark (userEntity_t *self)
{
	trace_t				scorch_trace;
	CVector				destPt, destIntoWorldPt;
	CVector				temp;
	effectEntityHook_t	*hook = (effectEntityHook_t *) self->userHook;

	// get the destination point for the trace
	if (hook->dstEnt)
	{
		destPt = hook->dstEnt->s.origin;
	}
	else
	{
		destPt = hook->dstPos;
	}

	// extend it somewhat
	temp = destPt - self->s.origin;
	temp = temp * 1.10;
	destIntoWorldPt = destPt + temp;

	// perform the trace
	scorch_trace = gstate->TraceLine_q2(destPt, destIntoWorldPt, hook->emitter, MASK_SOLID);

	// did we hit the world?
	if (scorch_trace.ent == gstate->FirstEntity())
	{
		gstate->WriteByte (SVC_TEMP_ENTITY);
		gstate->WriteByte (TE_SCORCHMARK);
		gstate->WritePosition (scorch_trace.endpos);
		gstate->WriteShort (scorch_trace.plane.planeIndex);
		gstate->WriteShort (scorch_trace.ent->s.modelindex);	//	write model index so that we can stick to
																//	bmodels correctly
		gstate->WriteShort (scorch_trace.ent->s.number);		//	entity number for bmodels

		gstate->MultiCast(zero_vector,MULTICAST_PVS);
	}
	return;
}


void effect_bolt_track(userEntity_t *self)
{
	effectEntityHook_t *hook = (effectEntityHook_t *) self->userHook;
	if (!hook || !hook->emitter)
		return;

	lightningEffectHook_t *ehook = (lightningEffectHook_t *) hook->emitter->userHook;
	if (!ehook)
		return;

	trackInfo_t				tinfo;
	memset(&tinfo,0,sizeof(tinfo));

	// msg client to track this entity
	tinfo.renderfx	= RF_LIGHTNING|RF_TRANSLUCENT;
	tinfo.ent		= self;
	tinfo.srcent	= self;
	tinfo.altangle	= ehook->color;
	tinfo.altpos2.x	= ehook->scale;
	tinfo.altpos2.y = ehook->modulation;
	tinfo.flags = TEF_SRCINDEX|TEF_FXFLAGS|TEF_ALTANGLE|TEF_LENGTH|TEF_LIGHTSIZE|TEF_ALTPOS2;

	// we're either using a destination entity or a position w/ the track entity
	if (hook->dstEnt)
	{
		tinfo.dstent = hook->dstEnt;
		tinfo.flags |= TEF_DSTINDEX|TEF_FRU;
   		tinfo.fru.Zero();
	}
	else
	{
		tinfo.flags |= TEF_DSTPOS;
		tinfo.dstpos = hook->dstPos;
	}

	// does dabug want sparks?
	if (hook->emitter->spawnflags & NO_SPARKS)
	{
		tinfo.fxflags |= TEF_NOPARTICLES;
	}

   	// send it down....
//	gstate->Con_Dprintf("effect_bolt_track\n");
	self->hacks = 0;
	com->trackEntity(&tinfo, MULTICAST_ALL);
}
///////////////////////////////////////////////////////////////////////////////
// effect_bolt_think
///////////////////////////////////////////////////////////////////////////////

void effect_bolt_think (userEntity_t *self)
{
	effectEntityHook_t	*hook = (effectEntityHook_t *) self->userHook;
	CVector dir;

	if (gstate->level->players < self->hacks_int)
	{
		self->hacks_int = gstate->level->players;		// s'ok.  someone left...just decrement the counter
	}
	else if (gstate->level->players > self->hacks_int)
	{
		int count = 0;
		userEntity_t *ent = alist_FirstEntity(client_list);
		while (ent)
		{
			if ((ent->flags & FL_CLIENT) && (ent->inuse))
				count++;
			ent = alist_NextEntity(client_list);
		}

		if (count == gstate->level->players)
		{
			self->hacks_int = count;
			effect_bolt_track(self);					// someone joined...track it!
		}
	}

	// make sure it sends the scale down
	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	// reached killtime?
	if ( ( (gstate->time >= hook->killtime) && !(hook->emitter->spawnflags & CONSTANT_BOLT) ) || ((hook->emitter->spawnflags & CONSTANT_BOLT) && !(hook->emitter->spawnflags & LIGHT_ON)))
	{
		// check trace damage when deleting
		if (hook->dmg && (hook->emitter->spawnflags & TRACE_DMG))
			check_trace_damage (self);

		// do we want to leave a scorch mark?
		if (hook->emitter->spawnflags & LEAVE_SCORCH)
			check_scorch_mark (self);

		com->untrackEntity(self, NULL, MULTICAST_ALL);
		gstate->RemoveEntity(self);
	}
	else
	{
	   	// is this bolt going to do damage
		if (hook->dmg && hook->dstEnt)
		{
			// damage the dstEnt
			dir = hook->dstEnt->s.origin - self->s.origin;
			//VectorSubtract( hook->dstEnt->s.origin, self->s.origin, dir );

			// do damage
			com->Damage( hook->dstEnt, self, self->owner, hook->dstEnt->s.origin, dir, hook->dmg, DAMAGE_INERTIAL | DAMAGE_SPARKLES);
		}
		else if (hook->dmg && hook->bCheckTraceDamage && (hook->emitter->spawnflags & TRACE_DMG))
		{
			// don't check the trace damage again until it is being removed
			if (!(hook->emitter->spawnflags & CONSTANT_BOLT))
				hook->bCheckTraceDamage = FALSE;

			check_trace_damage (self);
		}

		// if it's  constant bolt, only check damage every half second
		if (hook->emitter->spawnflags & CONSTANT_BOLT)
			self->nextthink = gstate->time + 0.5f;
		else
			self->nextthink = gstate->time + 0.1f;
	}
}

///////////////////////////////////////////////////////////////////////////////
// effect_bolt_spawn
///////////////////////////////////////////////////////////////////////////////

void effect_bolt_spawn (userEntity_t *pEmitter, userEntity_t *dstEnt, CVector &dstPos)
{
	lightningEffectHook_t	*ehook = (lightningEffectHook_t *) pEmitter->userHook;
	effectEntityHook_t		*hook = NULL;
	userEntity_t			*bolt = NULL, *client = NULL;
//	trackInfo_t				tinfo;
	qboolean				bInRange = FALSE;

	// clear this variable
//	memset(&tinfo, 0, sizeof(tinfo));

	// let's make sure this bolt is going to be in the PHS before
	// we spawn it for optimization purposes
	client = alist_FirstEntity (client_list);

	for (int i = 0; client != NULL; i++)
	{
		if (gstate->inPHS(client->s.origin, pEmitter->s.origin))
		{
			bInRange = TRUE;
			break;
		}
		// try the next one
		client = alist_NextEntity (client_list);
	}
	// get out of here without respawn if we're not close enough to any of the players
	if (!bInRange && (gstate->time > 3.0) && !(pEmitter->spawnflags & CONSTANT_BOLT))  // hack because client's aren't loaded when entities are
		return;

	// spawn bolt and set up the data fields
	bolt = gstate->SpawnEntity();
	bolt->className = "effect_lightning_bolt";
	bolt->movetype = MOVETYPE_NONE;
	bolt->solid = SOLID_NOT;
	bolt->s.renderfx = RF_TRACKENT|RF_LIGHTNING;    // drawn by client
	bolt->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	bolt->s.frame = ehook->scale;					// beam diameter
	bolt->s.skinnum = 0xa1a2a3a4;					// beam color
	bolt->s.alpha = 0.6;
	bolt->owner = pEmitter;
	bolt->s.origin = pEmitter->s.origin;

	// link it into the world
	gstate->LinkEntity(bolt);

	bolt->userHook = gstate->X_Malloc(sizeof(effectEntityHook_t), MEM_TAG_HOOK);
	hook = (effectEntityHook_t *) bolt->userHook;

	bolt->save = effect_entity_hook_save;
	bolt->load = effect_entity_hook_load;

	hook->killtime = gstate->time + ehook->duration;
	hook->emitter = pEmitter;
	hook->bCheckTraceDamage = TRUE;					// check it once when spawning and once when deleted
	hook->dmg = ehook->dmg;							// set the damage values


	// set up the functions to update the lightning bolt
	bolt->think = effect_bolt_think;
	bolt->nextthink = gstate->time + 0.1;
/*
	// msg client to track this entity
	tinfo.renderfx	= RF_LIGHTNING|RF_TRANSLUCENT;
	tinfo.ent		= bolt;
	tinfo.srcent	= bolt;
	tinfo.altangle	= ehook->color;
	tinfo.altpos2.x	= ehook->scale;
	tinfo.altpos2.y = ehook->modulation;
	tinfo.flags = TEF_SRCINDEX|TEF_FXFLAGS|TEF_ALTANGLE|TEF_LENGTH|TEF_LIGHTSIZE|TEF_ALTPOS2;

	// we're either using a destination entity or a position w/ the track entity
	if (dstEnt)
	{
		hook->dstEnt = dstEnt;
		tinfo.dstent = dstEnt;
		tinfo.flags |= TEF_DSTINDEX|TEF_FRU;
   		tinfo.fru.Zero();
	}
	else
	{
		hook->dstPos = dstPos;
		tinfo.flags |= TEF_DSTPOS;
		tinfo.dstpos = dstPos;
	}

	// does dabug want sparks?
	if (pEmitter->spawnflags & NO_SPARKS)
	{
		tinfo.fxflags |= TEF_NOPARTICLES;
	}

   	// send it down....
	com->trackEntity(&tinfo, MULTICAST_ALL);
*/

	if (dstEnt)
		hook->dstEnt = dstEnt;
	else
		hook->dstPos = dstPos;

	bolt->hacks_int = gstate->level->players;	// hacky hacky to let new clients see this bolt!
	effect_bolt_track(bolt);

	// get a random sound
	int snd = (frand() * 2.9);
	if (!ehook->sound[snd]) snd = 0;

	// if sound is okay play it
	if (ehook->sound[snd])
	{
		if (pEmitter->spawnflags & CONSTANT_BOLT)
		{
			// if the bolt is constant, loop the sound
			pEmitter->s.sound = ehook->sound[0];
		}
		else
		{
			// play emitter sound once
			gstate->StartEntitySound(bolt, CHAN_OVERRIDE, ehook->sound[snd], 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// effect_lightning_think
///////////////////////////////////////////////////////////////////////////////

void effect_lightning_think (userEntity_t *self)
{
	lightningEffectHook_t	*hook = (lightningEffectHook_t *) self->userHook;
	attractorHook_t			*ahook = NULL;
	userEntity_t			*ent = NULL;
	userEntity_t			*bestEnt = NULL;
	float					ClosestRange = 2000.0f;
	float					hitChance = rnd();

	/////////////////////////////////////////////////////////////////////////
	// are we looking for players? calc a random number to see if we hit...
	/////////////////////////////////////////////////////////////////////////
	if (!(self->spawnflags & NO_CLIENTS) && (hitChance < hook->chance))
	{
		CVector dir, start;

		ent = alist_FirstEntity (client_list);

		for (int i = 0; ent != NULL; i++)
		{
			// if this client is visible and within range.. break out
			if (VectorDistance( self->s.origin, ent->s.origin ) < ClosestRange)
			{
				// get the vector from the entity to the emitter
				dir = ent->s.origin - self->s.origin;
				dir.Normalize();

				start = self->s.origin;
				start = start + dir * 128;	// the emitter might be inside a bmodel.. move it out some

   				tr=gstate->TraceLine_q2( start, ent->s.origin, self, MASK_SHOT);

				if (tr.ent == ent || ( (tr.ent == NULL) && (tr.fraction == 1.0) ) )
				{
					bestEnt			= ent;
					ClosestRange	= VectorDistance( self->s.origin, ent->s.origin );
				}
			}	
			// try the next one
			ent = alist_NextEntity (client_list);

		}
		// if we found one...zap the shit out of him
		if (bestEnt)
		{
// amw : I like the lightning to track players
#if 0
			start = bestEnt->s.origin;
			start.z += bestEnt->viewheight;
			effect_bolt_spawn ( self, NULL, start );
#else
			effect_bolt_spawn ( self, bestEnt, zero_vector );
#endif
		}
	}
	else if ((self->spawnflags & GROUND_STRIKES) && (hitChance < hook->gndchance))
	{
		qboolean bFound = FALSE;
		int numTries = 0;

		//////////////////////////////////////////////////////
		// randomly spawn lightning to points in the world
		//////////////////////////////////////////////////////

		CVector vec, ang, start, end;
   		trace_t tr;

		while (!bFound && (numTries < MAX_STRIKE_TRIES))
		{
			vec = CVector(rnd() - 0.5, rnd() - 0.5, rnd() - 0.5);
			VectorToAngles( vec, ang );
   			AngleToVectors( ang, forward );

			start = self->s.origin;
			start = start + forward * 128;	// the emitter might be inside a bmodel.. move it out some
			end   = start + forward * 2000;

   			tr=gstate->TraceLine_q2( start, end, self, MASK_SHOT );
			if (tr.ent == gstate->FirstEntity())
			{
				bFound = TRUE;
				effect_bolt_spawn ( self, NULL, tr.endpos );
			}
			numTries++;
		}

	}
	else
	{
		////////////////////////////////////
		// find an appropriate attractor
		////////////////////////////////////

		if (self->spawnflags & CYCLE)
		{
			if (!hook->curAttractor)
			{
				hook->curAttractor = hook->attractorHead;
			}

			ent = hook->curAttractor;
			
			if (ent)
			{
				// use the attractor's targets
				com->UseTargets (ent, NULL, NULL);

				// spawn the lightning bolt
				effect_bolt_spawn ( self, NULL, ent->s.origin );
				
				// get the hook for the attractor
				ahook = (attractorHook_t *) ent->userHook;
				// set up next attractor
				hook->curAttractor = ahook->nextAttractor;
			}
		}
		else
		{
			// find a random attractor to strike
			int attractorIdx = (int)(rnd() * hook->totalAttractors);

			// get the first attractor
			ent = hook->attractorHead;

			for (int i = 0; ent && i < attractorIdx; i++)
			{
				// get the hook for the attractor
				ahook = (attractorHook_t *) ent->userHook;
				ent = ahook->nextAttractor;
			}

			// if we found one...zap the shit out of him
			if (ent)
			{
				hook->curAttractor = ent;

				// use the attractor's targets
				com->UseTargets (ent, NULL, NULL);

				// spawn the lightning bolt
				effect_bolt_spawn ( self, NULL, ent->s.origin );
			}
		}
	}

	// strike once and then get out
	if (self->spawnflags & (STRIKE_ONCE|CONSTANT_BOLT))
		return;

	self->think = effect_lightning_think;

	if (self->spawnflags & RANDOM_DELAY)
		self->nextthink = gstate->time + (rnd() * hook->delay);
	else
		self->nextthink = gstate->time + hook->delay;
}


///////////////////////////////////////////////////////////////////////////////
// effect_lightning_on
///////////////////////////////////////////////////////////////////////////////

void effect_lightning_on (userEntity_t *self)
{
	self->spawnflags |= LIGHT_ON;
	self->svflags &= ~SVF_NOCLIENT;	

	// call the think function immediately.. it will set up nextthink times etc
	effect_lightning_think (self);
}

///////////////////////////////////////////////////////////////////////////////
// effect_lightning_off
///////////////////////////////////////////////////////////////////////////////

void effect_lightning_off (userEntity_t *self)
{
	self->spawnflags &= ~LIGHT_ON;
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
	self->nextthink = 0;
}

///////////////////////////////////////////////////////////////////////////////
// effect_lightning_use
///////////////////////////////////////////////////////////////////////////////

void effect_lightning_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	// strike once and then get out
	if (self->spawnflags & STRIKE_ONCE)
	{
		effect_lightning_think (self);
		return;
	}

	// normal mode.. just toggle it on and off
	if (self->spawnflags & LIGHT_ON)
		effect_lightning_off (self);
	else
		effect_lightning_on (self);
}

///////////////////////////////////////////////////////////////////////////////
// effect_lightning_start
///////////////////////////////////////////////////////////////////////////////

void effect_lightning_start (userEntity_t *self)
{

	if (self->spawnflags & LIGHT_ON)
		effect_lightning_on (self);
	else
		effect_lightning_off (self);

	// if this has a parent, initialize the parent/child link
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.3;
	}
	else if (self->nextthink > 0)
	{
		// give the attractors time to link up
		self->nextthink = gstate->time + 3.0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// effect_lightning
///////////////////////////////////////////////////////////////////////////////

void effect_lightning (userEntity_t *self)
{
	lightningEffectHook_t		*hook = NULL;
	char						*soundStr = NULL;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(lightningEffectHook_t), MEM_TAG_HOOK);
	hook = (lightningEffectHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = lightning_effect_hook_save;
	self->load = lightning_effect_hook_load;

	//Set the default color
	hook->color.Set(0.45f,0.45f,0.75);
	hook->modulation = 1.0f;
	
	for (int i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair[i].key, "sound"))	
			hook->sound[0] = gstate->SoundIndex(self->epair [i].value);

		else if (!stricmp (self->epair[i].key, "sound1"))	
			hook->sound[0] = gstate->SoundIndex(self->epair [i].value);

		else if (!stricmp (self->epair[i].key, "sound2"))	
			hook->sound[1] = gstate->SoundIndex(self->epair [i].value);

		else if (!stricmp (self->epair[i].key, "sound3"))	
			hook->sound[2] = gstate->SoundIndex(self->epair [i].value);

		else if (!stricmp (self->epair[i].key, "delay"))
			hook->delay = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "duration"))
			hook->duration = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "dmg"))
			hook->dmg = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "scale"))
			hook->scale = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "chance"))
			hook->chance = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "gndchance"))
			hook->gndchance = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "modulation"))
			hook->modulation = atof (self->epair[i].value);

		else if ( _stricmp(self->epair[i].key, "_color") == 0 )
		{
			sscanf(self->epair[i].value,"%f%f%f",&hook->color.x,&hook->color.y,&hook->color.z);
		}
	}

	// set the lightning scale value
	if (hook->scale)
		self->s.frame = hook->scale;
	else
		self->s.frame = hook->scale = 10.0;

	// set the strike chances
	if (!hook->chance)
		hook->chance = 0.10;

	if (!hook->gndchance)
		hook->gndchance = 0.20;

	// set the delay between strikes
	if (!hook->delay)
		hook->delay = 2.0;
	
	// duration of the lightning strike
	if (!hook->duration)
		hook->duration = 0.3;

	if (!self->owner)
		self->owner = self;

	self->use = effect_lightning_use;
	self->think = effect_lightning_think;

	// let everything else get spawned before we start setting up
	self->think = effect_lightning_start;
	self->nextthink = gstate->time + 0.1;

//	// this needs an added delay to allow the attractors to link up
	if (self->spawnflags & CONSTANT_BOLT)
		self->nextthink = gstate->time + 3.75;

	// put this entity in the world
	gstate->LinkEntity (self);

	return;
}

///////////////////////////////////////////////////////////////////////////////
// effect_steam
///////////////////////////////////////////////////////////////////////////////

// SCG[11/19/99]: Save game stuff
#define	STEAMEFFECTHOOK(x) (int)&(((steamEffectHook_t *)0)->x)
field_t steam_effect_hook_fields[] = 
{
	{"sound",		STEAMEFFECTHOOK(sound),		F_INT},
	{"vec",			STEAMEFFECTHOOK(vec),		F_VECTOR},
	{"delay",		STEAMEFFECTHOOK(delay),		F_FLOAT},
	{"duration",	STEAMEFFECTHOOK(duration),	F_FLOAT},
	{"dmg",			STEAMEFFECTHOOK(dmg),		F_FLOAT},
	{"scale",		STEAMEFFECTHOOK(scale),		F_FLOAT},
	{"flags",		STEAMEFFECTHOOK(flags),		F_INT},
	{NULL, 0, F_INT}
};

void steam_effect_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, steam_effect_hook_fields, sizeof( steamEffectHook_t ) );
}

void steam_effect_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, steam_effect_hook_fields, sizeof( steamEffectHook_t ) );
}

void effect_steam (userEntity_t *self)
{
	int		i;
	//float	height = 0;
    steamEffectHook_t *hook;

//   	self->userHook = new steamEffectHook_t;
	self->userHook = gstate->X_Malloc(sizeof(steamEffectHook_t),MEM_TAG_HOOK);
	hook = (steamEffectHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = steam_effect_hook_save;
	self->load = steam_effect_hook_load;

    // set up defaults
    hook->vec.Set(0,0,0);
    hook->delay    = 6.0f;
    hook->duration = 2.0f;
    hook->dmg      = 0.0f;
	hook->scale    = 1.0f;
    hook->flags    = 0;

/*
	int				sound;			    // sound index
    CVector         vec;                // vector distance/direction
	float			delay;				// time between jets
	float			duration;			// duration of steam jet
	float			dmg;				// damage if a PLAYER gets hit
	float			scale;				// scale of the particle model
    int             flags;              // bit flags
*/


    for (i = 0; self->epair[i].key != NULL; i++)
    {
	  if (!stricmp (self->epair[i].key, "delay"))
	  {
	    hook->delay = atof (self->epair[i].value);
	  }



	}
   

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	////self->s.renderfx	= RF_PARTICLEVOLUME;
	self->s.effects		= 0;
	self->s.effects2	= EF2_STEAM;

	// what direction will the rain be falling...
	//self->s.skinnum = self->spawnflags;		// default is straight down (spawnflags == 0)

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	//self->s.mins.z = self->s.maxs.z - height;

	
	gstate->LinkEntity (self);  // put this entity in the world
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_effects_register_func()
{
	gstate->RegisterFunc("effect_bolt_think",effect_bolt_think);
	gstate->RegisterFunc("effect_lightning_think",effect_lightning_think);
	gstate->RegisterFunc("effect_lightning_start",effect_lightning_start);
	gstate->RegisterFunc("effect_lightning_use",effect_lightning_use);
	gstate->RegisterFunc( "lightning_effect_hook_save", lightning_effect_hook_save );
	gstate->RegisterFunc( "lightning_effect_hook_load", lightning_effect_hook_load );
	gstate->RegisterFunc( "effect_entity_hook_save", effect_entity_hook_save );
	gstate->RegisterFunc( "effect_entity_hook_load", effect_entity_hook_load );
	gstate->RegisterFunc( "steam_effect_hook_save", steam_effect_hook_save );
	gstate->RegisterFunc( "steam_effect_hook_load", steam_effect_hook_load );
}

