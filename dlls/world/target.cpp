///////////////////////////////////////////////////////////////////////////////
//	target.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "world.h"
#include "light.h"
#include "ai_frames.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
//#include "ai_utils.h"// SCG[1/23/00]: not used
//#include "ai_func.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "common.h"
#include "spawn.h"

// char *strlwr (char *s);
///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	target_laser (userEntity_t *self);
DllExport	void	target_earthquake (userEntity_t *self);
DllExport	void	target_spotlight (userEntity_t *self);
DllExport	void	target_lightramp (userEntity_t *self);
// SCG[12/5/99]: DllExport	void	target_framechange (userEntity_t *self);
DllExport	void	target_effect (userEntity_t *self);
DllExport	void	target_speaker (userEntity_t *self);
DllExport	void	target_attractor (userEntity_t *self);
DllExport	void	target_monster_spawn (userEntity_t *self);
// SCG[12/5/99]: DllExport	void	target_lightray (userEntity_t *self);
// SCG[12/5/99]: DllExport	void	target_throw (userEntity_t *self);
// SCG[12/5/99]: DllExport	void	target_turret (userEntity_t *self);

// SCG[11/19/99]: 
#define	MONSTERSPAWNHOOK(x) (int)&(((monsterSpawnHook_t *)0)->x)
field_t monster_spawn_hook_fields[] = 
{
	{"sound",			MONSTERSPAWNHOOK(sound),		F_INT},
	{"monsterClass",	MONSTERSPAWNHOOK(monsterClass),	F_LSTRING},
	{"aiStateStr",		MONSTERSPAWNHOOK(aiStateStr),	F_LSTRING},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: 
#define	LIGHTRAMPHOOK(x) (int)&(((lightrampHook_t *)0)->x)
field_t lightramp_hook_fields[] = 
{
	{"timestamp",	LIGHTRAMPHOOK(timestamp),	F_FLOAT},
	{"speed",		LIGHTRAMPHOOK(speed),		F_FLOAT},
	{"message",		LIGHTRAMPHOOK(message),		F_LSTRING},
	{NULL, 0, F_INT}
};

///////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////

#define	DIST_EPSILON	(0.03125)            

// --- target_monster_spawn
#define	USE_ONCE	0x0001

// --- target_turret
#define	TURRET_TOGGLE	0x0001
#define	TURRET_ON		0x0001

#define MAX_TARGET_SPEAKER_SNDS		6
#define	LOOPED_ON					0x01
#define LOOPED_OFF					0x02
#define RELIABLE					0x04
#define NON_DIRECTIONAL				0x08
#define START_OFF					0x10

///////////////////////////////////////////////////////////////////////////////
//	External functions
///////////////////////////////////////////////////////////////////////////////
void UNIQUEID_Add( const char *szUniqueID, userEntity_t *pEntity );



///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	laserHook_s
{
	int			damage;
	int			soundIndex;
} laserHook_t;

#define	LASERHOOK(x) (int)&(((laserHook_t *)0)->x)
field_t laser_hook_fields[] = 
{
	{"damage",	LASERHOOK(damage),			F_EDICT},
	{"soundIndex",	LASERHOOK(soundIndex),	F_EDICT},
	{NULL, 0, F_INT}
};

typedef	struct	quakeHook_s
{
	int			severity;
	int			sndIndex;
	float		mins;				//NSS[11/13/99]:Added mins and maxs sound values
	float		maxs;
	float		last_move_time;
	float		radius;
	float		damage;
	float		duration;
	float		end_time;
} quakeHook_t;

#define	QUAKEHOOK(x) (int)&(((quakeHook_t *)0)->x)
field_t quake_hook_fields[] = 
{
	{"severity",		QUAKEHOOK(severity),		F_INT},
	{"sndIndex",		QUAKEHOOK(sndIndex),		F_INT},
	{"mins",			QUAKEHOOK(mins),			F_INT},
	{"maxs",			QUAKEHOOK(maxs),			F_INT},
	{"last_move_time",	QUAKEHOOK(last_move_time),	F_INT},
	{"radius",			QUAKEHOOK(radius),			F_INT},
	{"damage",			QUAKEHOOK(damage),			F_INT},
	{"duration",		QUAKEHOOK(duration),		F_INT},
	{"end_time",		QUAKEHOOK(end_time),		F_INT},
	{NULL, 0, F_INT}
};

typedef struct	frameChangeHook_s
{
	ANIMSEQ		seq;
	float		frametime;
	char		*sound;
} frameChangeHook_t;

#define	FRAMECHANGEHOOK(x) (int)&(((frameChangeHook_t *)0)->x)
field_t frame_change_hook_fields[] = 
{
	{"seq",			FRAMECHANGEHOOK(seq),			F_INT},
	{"frametime",	FRAMECHANGEHOOK(frametime),		F_FLOAT},
	{"sound",		FRAMECHANGEHOOK(sound),			F_LSTRING},
	{NULL, 0, F_INT}
};

typedef struct	speakerHook_s
{
	int			sound[MAX_TARGET_SPEAKER_SNDS];		
	int			count;
	int			delay;
	int			minDelay;
//	float		attenuation;
	float		volume;
} speakerHook_t;

/*
typedef struct	turretHook_s
{
	int			modelindex;		// default gun model
	int			height;			// default popup height
	int			sound;			// sound played when firing
	int			sound_up;		// sound when raising
	int			sound_down;		// sound when lowering
	int			hit_sound;		// sound when destroyed
	int			ammo_type;		// default ammo type (laser bolts)
	float		fire_rate;		// fire rate (time between firings)
	float		range;			// range of turret
	int			state;			// current turret state
	float		nextAttack;		// next time the turret can fire
	float		basedmg;		// default base damage 
	float		rnddmg;			// default random damage
} turretHook_t;
*/
/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/
// SCG[12/2/99]: Great, thanks for making two fucking turretHook_t 
// SCG[12/2/99]: structures and making them ALMOST exactly the same..
/*
#define	TURRETHOOK(x) (int)&(((turretHook_t *)0)->x)
field_t turret_hook_fields2[] = 
{
	{"modelindex",	TURRETHOOK(modelindex),		F_INT},
	{"height",		TURRETHOOK(height),			F_INT},
	{"sound",		TURRETHOOK(sound),			F_INT},
	{"sound_up",	TURRETHOOK(sound_up),		F_INT},
	{"sound_down",	TURRETHOOK(sound_down),		F_INT},
	{"hit_sound",	TURRETHOOK(hit_sound),		F_INT},
	{"ammo_type",	TURRETHOOK(ammo_type),		F_INT},
	{"fire_rate",	TURRETHOOK(fire_rate),		F_FLOAT},
	{"range",		TURRETHOOK(range),			F_FLOAT},
	{"state",		TURRETHOOK(state),			F_INT},
	{"nextAttack",	TURRETHOOK(nextAttack),		F_FLOAT},
	{"basedmg",		TURRETHOOK(basedmg),		F_FLOAT},
	{"rnddmg",		TURRETHOOK(rnddmg),			F_FLOAT},
	{NULL, 0, F_INT}
};

void turret_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, turret_hook_fields2, sizeof( turretHook_t ) );
}

void turret_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, turret_hook_fields2, sizeof( turretHook_t ) );
}
*/
void monster_spawn_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, monster_spawn_hook_fields, sizeof( monsterSpawnHook_t ) );
}

void monster_spawn_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, monster_spawn_hook_fields, sizeof( monsterSpawnHook_t ) );
}

void light_ramp_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, lightramp_hook_fields, sizeof( lightrampHook_t ) );
}

void target_lightramp_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator );
void target_lightramp_think( userEntity_t *self );
void light_ramp_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, lightramp_hook_fields, sizeof( lightrampHook_t ) );

	// SCG[2/23/00]: restore lightramp stuff
	if( ent->think == target_lightramp_think )
	{
		target_lightramp_use( ent, ent, ent );
	}
}

void laser_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, laser_hook_fields, sizeof( laserHook_t ) );
}

void laser_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, laser_hook_fields, sizeof( laserHook_t ) );
}

void frame_change_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, frame_change_hook_fields, sizeof( frameChangeHook_t ) );
}

void frame_change_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, frame_change_hook_fields, sizeof( frameChangeHook_t ) );
}

void quake_hook_save( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, quake_hook_fields, sizeof( quakeHook_t ) );
}

void quake_hook_load( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, quake_hook_fields, sizeof( quakeHook_t ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser_think (userEntity_t *self)
{
	laserHook_t		*hook = (laserHook_t *) self->userHook;
	userEntity_t	*ignore;
	trace_t			tr;
	CVector			start, end;
	CVector			point, dir;
	CVector			last_movedir;
	int				count;

	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	if (self->enemy)
	{
		last_movedir = self->movedir;

		point = self->enemy->absmin + self->enemy->size * 0.5;
		dir = point - self->s.origin;
		dir.Normalize();
		self->movedir = dir;

		if (dir != last_movedir)
		{
			VectorToAngles( dir, end );
			self->s.angles = end;
			self->spawnflags |= 0x80000000;
		}
	}
	else
	{
		dir = self->movedir;
	}

	ignore = self;

	start = self->s.origin;
	end = start + dir * 2048;

	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
		if (!tr.ent)
		{
			break;
		}

		// hurt it if we can
		if ((tr.ent->takedamage) && (tr.ent != self->owner))
		{
			com->Damage(tr.ent, self, self->owner, tr.endpos, dir, hook->damage, DAMAGE_NONE);
		}

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		//	Your mom is done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gstate->WriteByte (SVC_TEMP_ENTITY);
				gstate->WriteByte (TE_LASER_SPARKS);
				gstate->WriteByte (count);
				gstate->WritePosition (tr.endpos);
				gstate->WriteDir (tr.plane.normal);
				gstate->WriteByte (self->s.skinnum);
				gstate->MultiCast (tr.endpos, MULTICAST_PVS);
			}
			break;
		}

		ignore = tr.ent;
		start = tr.endpos;
	}

	self->s.old_origin = tr.endpos;

	self->nextthink = gstate->time + THINK_TIME;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser_on (userEntity_t *self)
{
	laserHook_t		*hook = (laserHook_t *) self->userHook;
	CVector			dir, ang;

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;

	if (!self->enemy)
	{
		//	set up angles here for renderer to use when drawing
		//	this way it doesn't have to be done every think
		dir = self->movedir;
		VectorToAngles( dir, ang );
		self->s.angles = ang;
	}
	
	// play the looping sound
	if (hook->soundIndex)
	{
		self->s.sound = hook->soundIndex;
	}

	target_laser_think (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser_off (userEntity_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;

	// turn off the sound
	self->s.sound = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (self->spawnflags & 1)
		target_laser_off (self);
	else
		target_laser_on (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser_start (userEntity_t *self)
{
	userEntity_t	*ent;
	CVector			ang;
	laserHook_t		*hook = (laserHook_t *) self->userHook;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero
	self->s.alpha = 0.5;

	//	set the beam diameter - starting radius is encoded in upper 8 bits
	//	ending radius in the lower 8 bits -- this is a hack to make the network
	//	messages be a little faster
	if (self->spawnflags & 64)
		self->s.frame = 16;
	else
		self->s.frame = 4;
	self->s.frame = (self->s.frame << 8) + self->s.frame;

	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	// set the color
	if (!self->owner)
		self->owner = self;

	if (!self->enemy)
	{
		if (self->target)
		{
			ent = com->FindTarget (self->target);
			self->enemy = ent;
		}
		else
			com->SetMovedir (self);
	}
	self->use = target_laser_use;
	self->think = target_laser_think;

	if (!hook->damage)
		hook->damage = 1;

	self->s.mins.Set(-8, -8, -8);
	self->s.maxs.Set(8, 8, 8);
	gstate->LinkEntity (self);

	if (self->spawnflags & 1)
	{
		target_laser_on (self);
	}
	else
	{
		target_laser_off (self);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_laser (userEntity_t *self)
{
	int			i;
	laserHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(laserHook_t),MEM_TAG_HOOK);
	hook = (laserHook_t *) self->userHook;

	self->save = laser_hook_save;
	self->load = laser_hook_load;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "dmg") || !stricmp (self->epair [i].key, "damage"))
			hook->damage = atoi (self->epair [i].value);
		else if (!stricmp (self->epair[i].key, "sound"))	
			hook->soundIndex = gstate->SoundIndex (self->epair [i].value);
	}

	// let everything else get spawned before we start firing
	self->think = target_laser_start;
	self->nextthink = gstate->time + 0.1;
}

/* ------------------------- target_earthquake ----------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	target_earthquake_think
//
///////////////////////////////////////////////////////////////////////////////

#if 0
void SpawnEarthquake( userEntity_t *self, float fRadius, float fScale )
{
	userEntity_t	*ent;
	CVector			vDist, vKick;
	float			fLength, fIntensity, fIntensityScale;
	
	fIntensityScale = fScale * 0.05f;
    for( ent = gstate->FirstEntity(); ent; ent = gstate->NextEntity( ent ) )
	{
        if ( ( !ent->inuse ) || ( ent == self ) || ( ent->solid != SOLID_BBOX ) || ( ent->s.effects & EF_GIB ) )
		{
            continue;
		}

		vDist = ent->s.origin - self->s.origin;
		fLength = vDist.Length();

		if( fLength > fRadius )
		{
			continue;
		}

		fIntensity = ( fRadius - fLength );

        if( ( ent->flags & FL_CLIENT ) && ( ent->client ) )
        {
			vKick.Set( ( crand() * 0.5 ), ( crand() * 0.5 ), ( crand() * 0.5 ) );
			vKick = -vKick;
            vKick.Multiply( ( fIntensity * fIntensityScale ) );
            com->KickView( ent, vKick, 0, 50, 100 );
        }
/*
        else
		{
            fIntensity *= 4;
		}

        if( ent->groundEntity )
        {
            ent->groundEntity = NULL;
            ent->velocity.x += ( rnd() - 0.5 ) * (fIntensity * 1.25);
            ent->velocity.y += ( rnd() - 0.5 ) * (fIntensity * 1.25);
            ent->velocity.z += ( rnd() - 0.5 ) * (fIntensity * 1.25);
        }
*/
	}
}
#endif /* 0 */

void target_earthquake_think (userEntity_t *self)
{
	quakeHook_t		*hook = (quakeHook_t *) self->userHook;

	
	com_SpawnEarthQuake( self, hook->radius, hook->severity, hook->damage );
	//NSS[10/25/99]:Added a code to remove the earthquake entity once it is done.
	if (gstate->time < hook->end_time)
	{
		self->nextthink = gstate->time + THINK_TIME;
	}
	else
	{
		//remove the sound
		gstate->StartEntitySound(self, CHAN_LOOP,hook->sndIndex, 0.0f, 0.0f, 0.0f);
		gstate->RemoveEntity(self);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	target_earthquake_use
//
///////////////////////////////////////////////////////////////////////////////

void target_earthquake_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	quakeHook_t		*hook = (quakeHook_t *) self->userHook;

	
	hook->end_time	= gstate->time + hook->duration;
	self->nextthink = gstate->time + THINK_TIME;
	self->think		= target_earthquake_think;
//	self->activator = activator;
	hook->last_move_time = 0;
	//NSS[10/25/99]:Added sound for Earthquake
	char buffer[30];
	int RaN = (int)(rnd()*4);

//	sprintf(buffer,"global/earthquake_%c.wav",97 + RaN);
	Com_sprintf(buffer,sizeof(buffer),"global/earthquake_%c.wav",97 + RaN);
	hook->sndIndex = gstate->SoundIndex (buffer);
	gstate->StartEntitySound(self, CHAN_LOOP,hook->sndIndex, 0.85f, hook->mins, hook->maxs);

}

///////////////////////////////////////////////////////////////////////////////
//	target_earthquake
//
///////////////////////////////////////////////////////////////////////////////

void target_earthquake (userEntity_t *self)
{
	int			i;
	quakeHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(quakeHook_t),MEM_TAG_HOOK);
	hook = (quakeHook_t *) self->userHook;

	self->save = quake_hook_save;
	self->load = quake_hook_load;

	hook->mins = 2000.0f;
	hook->maxs = 2024.0f;
	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "severity") || !stricmp (self->epair [i].key, "speed"))
			hook->severity = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "duration") || !stricmp (self->epair [i].key, "count"))
			hook->duration = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "radius"))
			hook->radius = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "damage"))
			hook->damage = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mins"))
			hook->mins = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "maxs"))
			hook->maxs = atof (self->epair [i].value);
	}

	if (!self->targetname)
		gstate->Con_Dprintf ("untargeted %s at %s\n", self->className, com->vtos (self->s.origin));

	if (!hook->duration)
		hook->duration = 5.0;

	if (!hook->severity)
		hook->severity = 200.0;

	if( !hook->radius )
		hook->radius = 200.0;

	self->svflags |= SVF_NOCLIENT;
	// NSS[2/21/00]:Doesn't get set until used.
	//self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	self->save = quake_hook_save;
	self->load = quake_hook_load;

	//self->noise_index = gstate->SoundIndex ("world/quake.wav");
}

/* ------------------------- target_earthquake ----------------------------- */

/*QUAKED target_spotlight (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
*/

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight_think (userEntity_t *self)
{
//	laserHook_t		*hook = (laserHook_t *) self->userHook;
	userEntity_t	*ignore;
	trace_t			tr;
	CVector			start, end;
	CVector			point, dir;
	CVector			last_movedir;

	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	if (self->enemy)
	{
		last_movedir = self->movedir;

		point = self->enemy->absmin + self->enemy->size * 0.5;
		dir = point - self->s.origin;
		dir.Normalize ();
		self->movedir = dir;

//		if (dir != last_movedir)
//			self->spawnflags |= 0x80000000;
	}
	else
	{
		dir = self->movedir;
	}

	start = self->s.origin;
//	end = start + dir * 2048;
	end = start + dir * self->s.maxs[1];		// hack: maxs[1] holds the length to project the spotlight

	ignore = self->enemy;

	while(1)
	{
		tr = gstate->TraceLine_q2( start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER );
		if (!tr.ent)
		{
			break;
		}

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		//	Your mom is done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
		{
			break;
		}

		ignore = tr.ent;
		start = tr.endpos;
	}

//	self->s.old_origin = tr.endpos;
	self->s.mins = tr.endpos;

	self->nextthink = gstate->time + THINK_TIME;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight_on (userEntity_t *self)
{
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	target_spotlight_think (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight_off (userEntity_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (self->spawnflags & 1)
		target_spotlight_off (self);
	else
		target_spotlight_on (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight_start (userEntity_t *self)
{
	userEntity_t	*ent;
	CVector			ang;
	int				i, radius = 0, length = 2048;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_SPOTLIGHT|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair[i].key, "radius"))
			 radius = atoi (self->epair[i].value);

		if (!stricmp (self->epair[i].key, "length"))
			 length = atoi (self->epair[i].value);
	}

	// set the beam diameter
	if (radius)
		self->s.frame = radius;
	else
		self->s.frame = 4;

	if (!self->owner)
		self->owner = self;

	if (!self->enemy)
	{
		if (self->target)
		{
			ent = com->FindTarget (self->target);
			self->enemy = ent;
		}
		else
		{
			com->SetMovedir (self);
		}
	}
	self->use = target_spotlight_use;
	self->think = target_spotlight_think;

	
	self->s.maxs.y = length;				// hack: using maxs[1] as the length value

	
	gstate->LinkEntity (self);

	if (self->spawnflags & 1)
	{
		target_spotlight_on (self);
	}
	else
	{
		target_spotlight_off (self);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void target_spotlight (userEntity_t *self)
{
	// let everything else get spawned before we start firing
	self->think = target_spotlight_start;
	self->nextthink = gstate->time + 0.1;
}


///////////////////////////////////////////////////////////////////////////////
//
//	target_lightramp_think
//
///////////////////////////////////////////////////////////////////////////////

void target_lightramp_think (userEntity_t *self)
{
	char	style[2];
	
	lightrampHook_t	*hook = (lightrampHook_t *) self->userHook;
	// NSS : For restored saved games when self->enemy cannot be saved or restored... find it again.
	// SCG[2/23/00]: We will just call lightramp_use when the entity is restored
	// SCG[2/23/00]: since this code is a duplicate.
/*
	if (!self->enemy)
	{
		userEntity_t	*target = NULL;

		// search for the entity with a targetName == self->target
		target = com->FindTarget (self->target);

		// this only supports one target
		if (target)
		{
			// is this a light?
			if (!stricmp (target->className, "light"))
			{
				self->enemy = target;
			}
			else
			{
				gstate->Con_Dprintf ("%s at %s ", self->className, com->vtos(self->s.origin));
				gstate->Con_Dprintf ("target %s (%s at %s) is not a light\n", self->target, target->className, com->vtos(target->s.origin));
			}
		}

		if (!self->enemy)
		{
			gstate->Con_Dprintf ("%s target %s not found at %s\n", self->className, self->target, com->vtos(self->s.origin));
			self->remove (self);
			return;
		}
	}
*/
	lightHook_t		*lhook = (lightHook_t *) self->enemy->userHook;
	
	if( lhook == NULL )
	{
		gstate->Con_Dprintf( "lightramp enemy has no hook!\n" );
		return;
	}

	style[0] = 'a' + self->movedir[0] + (gstate->time - hook->timestamp) / 0.1 * self->movedir[2];
	style[1] = 0;
	gstate->ConfigString (CS_LIGHTS+lhook->style, style);

	if ((gstate->time - hook->timestamp) < hook->speed)
	{
		self->nextthink = gstate->time + 0.1;
	}
	else if (self->spawnflags & 1)
	{
		char	temp;

		temp = self->movedir[0];
		self->movedir.x = self->movedir[1];
		self->movedir.y = temp;
		self->movedir.z *= -1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	target_lightramp_use
//
///////////////////////////////////////////////////////////////////////////////

void target_lightramp_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	lightrampHook_t	*hook = (lightrampHook_t *) self->userHook;

	if (!self->enemy)
	{
		userEntity_t	*target = NULL;

		// search for the entity with a targetName == self->target
		target = com->FindTarget (self->target);

		// this only supports one target
		if (target)
		{
			// is this a light?
			if (!stricmp (target->className, "light"))
			{
				self->enemy = target;
			}
			else
			{
				gstate->Con_Dprintf ("%s at %s ", self->className, com->vtos(self->s.origin));
				gstate->Con_Dprintf ("target %s (%s at %s) is not a light\n", self->target, target->className, com->vtos(target->s.origin));
			}
		}

		if (!self->enemy)
		{
			gstate->Con_Dprintf ("%s target %s not found at %s\n", self->className, self->target, com->vtos(self->s.origin));
			self->remove (self);
			return;
		}
	}

	hook->timestamp = gstate->time;
//	target_lightramp_think (self);
	self->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//
//	target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
//
//	speed		How many seconds the ramping will take
//	message		two letters; starting lightlevel and ending lightlevel
//
///////////////////////////////////////////////////////////////////////////////

void target_lightramp (userEntity_t *self)
{

	lightrampHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(lightrampHook_t),MEM_TAG_HOOK);
	hook = (lightrampHook_t *) self->userHook;

	self->save = light_ramp_hook_save;
	self->load = light_ramp_hook_load;
	
	for (int i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "speed"))
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "message"))
			hook->message = self->epair [i].value;
	}

	if (!self->targetname)
		gstate->Con_Dprintf ("untargeted %s at %s\n", self->className, com->vtos(self->s.origin));

	
	if (!hook->message || strlen(hook->message) != 2 || hook->message[0] < 'a' || hook->message[0] > 'z' || hook->message[1] < 'a' || hook->message[1] > 'z' || hook->message[0] == hook->message[1])
	{
		gstate->Con_Dprintf("target_lightramp has bad ramp (%s) at %s\n", hook->message, com->vtos(self->s.origin));
		self->remove (self);
		return;
	}

	if (!hook->speed)
		hook->speed = 1.0;

	// target_lightramps do not work in network play
	if (deathmatch->value)
	{
		self->remove (self);
		return;
	}

	if (!self->target)
	{
		gstate->Con_Dprintf("%s with no target at %s\n", self->className, com->vtos(self->s.origin));
		self->remove (self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = target_lightramp_use;
	self->think = target_lightramp_think;

	self->movedir.x = hook->message[0] - 'a';
	self->movedir.y = hook->message[1] - 'a';
	self->movedir.z = (self->movedir.y - self->movedir.x) / (hook->speed / 0.1);
}


///////////////////////////////////////////////////////////////////////////////
//	target_framechange_use
//
///////////////////////////////////////////////////////////////////////////////

/*
void target_framechange_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*target;
	frameChangeHook_t	*hook = (frameChangeHook_t *) self->userHook;

	// search for the entity with a targetName == self->target
	target = com->FindTarget (self->target);

	// this only supports one target and deco's at that!
	if (target)
	{
		com->AnimateEntity(target, hook->seq.start, hook->seq.end, hook->seq.flags, hook->frametime);

		if (hook->sound)
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
//	target_framechange
//
///////////////////////////////////////////////////////////////////////////////

void target_framechange (userEntity_t *self)
{
	int		i;
	char	*seqType = NULL, *seqString = NULL;
	short	animseq = -1;
	frameChangeHook_t	*hook = NULL;
	userEntity_t	*target = NULL;

	self->userHook = gstate->X_Malloc(sizeof(frameChangeHook_t),MEM_TAG_HOOK);
	hook = (frameChangeHook_t *) self->userHook;

	self->save = frame_change_hook_save;
	self->load = frame_change_hook_load;

	// initialize settings
	hook->seq.start = hook->seq.end = -1;
	hook->frametime = 0.10;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "start"))
			hook->seq.start = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "end"))
			hook->seq.end = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "frametime"))
			hook->frametime = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "seqtype"))
			seqType = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "animseq"))
			animseq = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "seqstring"))
			seqString = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "sound"))
		{
			hook->sound = self->epair [i].value;
			if (hook->sound)
				gstate->SoundIndex (hook->sound);
		}
	}

	if (!self->targetname)
	{
		gstate->Con_Dprintf("untargeted %s at %s\n", self->className, com->vtos(self->s.origin));

		self->remove (self);
		return;
	}

	if (hook->frametime == 0)
		hook->frametime = 0.10;

	// are we specifying an animation sequence from the decoinfo.txt file or a string sequence?
	if (animseq >= 0 || seqString)
	{
		// the string takes precedence over the animseq value
		if (seqString)
		{
			// parse the string sequence
			ParseAnimationSequences(seqString, &hook->seq);
		}
		else
		{
			// search for the entity with a targetName == self->target
			target = com->FindTarget (self->target);

			// this only supports one target and deco's at that!
			if (!target || !strstr (target->className, "deco_") || (animseq > (MAX_DECO_ANIM_SEQS-1)))
			{
				gstate->Con_Dprintf ("Invalid animseq or target defined on %s at %s - is the target a deco?\n", self->className, com->vtos(self->s.origin));

				self->remove (self);
				return;
			}
			else
			{
				decoHook_t	*thook;
				thook = (decoHook_t *) target->userHook;

				// get the animation sequence data from the deco hook
				memcpy(&hook->seq, &thook->seq[animseq], sizeof(ANIMSEQ));
			}
		}
	}
	else
	{
		// figure out what the sequence type is and validate start & end
		if (!stricmp (seqType, "FRAME_LOOP"))
			hook->seq.flags = FRAME_LOOP;
		else
			hook->seq.flags = FRAME_ONCE;

		if (hook->seq.end < 0)
		{
			// they've only defined one frame
			hook->seq.end = hook->seq.start;
			hook->seq.flags = FRAME_ONCE;
		}

		// FIXME: check for valid maximum frame numbers
		if (hook->seq.start < 0)
		{
			gstate->Con_Dprintf("%s with invalid frame information at %s\n", self->className, com->vtos(self->s.origin));

			self->remove (self);
			return;
		}
	}

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
	self->use = target_framechange_use;

}
*/

//	void (*particle)(int te_type, vec3_t org, vec3_t dir, float speed, int color, int count, int type);

///////////////////////////////////////////////////////////////////////////////
//
//	target_effect_think
//
///////////////////////////////////////////////////////////////////////////////

//#define	RANDOM_SPREAD		0x01	// randomly spread the particles

#define TARGET_EFFECT_START_ON				0x01	// SCG[8/13/99]: start on
#define TARGET_EFFECT_BEAMS					0x02	// SCG[8/13/99]: use beams instead of particles
#define TARGET_EFFECT_RANDOM				0x04	// SCG[8/13/99]: random triggering
#define TARGET_EFFECT_BEAMS_SMOKE			0x08	// SCG[8/31/99]: use particles too
float rand_table[16] = 
{
//	1.0, 0.5, 2.0, 0.1, 
//	0.1, 3.0, 0.6, 0.1, 
//	1.0, 1.5, 0.5, 1.0, 
//	0.5, 0.1, 0.1, 2.0
	0.2, 0.1, 0.2, 0.6, 
	0.1, 0.2, 0.7, 0.3, 
	0.1, 0.3, 0.4, 0.1, 
	0.5, 0.8, 0.6, 0.9
};
int rand_count;
// SCG[11/19/99]: Save game stuff
#define	EFFECTHOOK(x) (int)&(((effectHook_t *)0)->x)
field_t effect_hook_fields[] = 
{
	{"spawn_type",		EFFECTHOOK(spawn_type),		F_INT},
	{"dir",				EFFECTHOOK(dir),			F_VECTOR},
	{"speed",			EFFECTHOOK(speed),			F_FLOAT},
	{"color",			EFFECTHOOK(color),			F_VECTOR},
	{"count	",			EFFECTHOOK(count),			F_INT},
	{"particle_type	",	EFFECTHOOK(particle_type),	F_INT},
	{"length	",		EFFECTHOOK(length),			F_FLOAT},
	{"frametime	",		EFFECTHOOK(frametime),		F_FLOAT},
	{"endtime	",		EFFECTHOOK(endtime),		F_FLOAT},
	{"sound	",			EFFECTHOOK(sound),			F_LSTRING},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	ATTRACTORHOOK(x) (int)&(((attractorHook_t *)0)->x)
field_t attractor_hook_fields[] = 
{
	{"emitter",			ATTRACTORHOOK(emitter),			F_EDICT},
	{"prevAttractor",	ATTRACTORHOOK(prevAttractor),	F_EDICT},
	{"nextAttractor",	ATTRACTORHOOK(nextAttractor),	F_EDICT},
	{"sound",			ATTRACTORHOOK(sound),			F_INT},
	{"triggerIndex",	ATTRACTORHOOK(triggerIndex),	F_INT},
	{NULL, 0, F_INT}
};

void effect_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, effect_hook_fields, sizeof( effectHook_t ) );
}

void effect_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, effect_hook_fields, sizeof( effectHook_t ) );
}

void attractor_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, attractor_hook_fields, sizeof( attractorHook_t ) );
}

void attractor_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, attractor_hook_fields, sizeof( attractorHook_t ) );
}

void target_effect_think (userEntity_t *self)
{
	
	effectHook_t	*hook = (effectHook_t *) self->userHook;
	CVector			origin;

/*
	if (hook->particle_type & PARTICLE_RANDOM_SPREAD)
	{
		origin.x = self->s.origin[0];// + ((rnd() - 0.5) * 8);
		origin.y = self->s.origin[1];// + ((rnd() - 0.5) * 8);
		origin.z = self->s.origin[2];// + ((rnd() - 0.5) * 8);
	}
	else
	{
		origin = self->s.origin;
	}
*/
	origin = self->s.origin;

	// display the particles
	if( self->spawnflags & TARGET_EFFECT_BEAMS )
	{
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SPARKS);
		gstate->WriteByte(hook->length);
		gstate->WritePosition(origin);
		gstate->WritePosition(hook->dir);
		gstate->WritePosition(hook->color);
		if( self->spawnflags & TARGET_EFFECT_BEAMS_SMOKE )
		{
			gstate->WriteByte(1);
		}
		else
		{
			gstate->WriteByte(0);
		}
		gstate->MultiCast(self->s.origin, MULTICAST_PVS);
	}
	else
	{
	
	//NSS[10/24/99]:un-commented this code, it is using multicast_pvs... just shouldn't be used everywhere.
	gstate->particle (	hook->spawn_type, 
							origin, 
							hook->dir, 
							hook->speed, 
							hook->color, 
							hook->count, 
							hook->particle_type);
	}
	
	// play the associated wave effect
	if (hook->sound)
		gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex(hook->sound), 0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX);


	if( self->spawnflags & TARGET_EFFECT_RANDOM )
	{
		if( rand_count > 15 )
		{
			rand_count = 0;
		}

		self->nextthink = gstate->time + rand_table[rand_count++];
	}
	else if( hook->endtime )
	{
		if( ( gstate->time + hook->frametime ) < hook->endtime )
		{
			self->nextthink = gstate->time + hook->frametime;
		}
		else 
		{
			self->think = NULL;
			self->nextthink = -1;
		}
	}
	else
	{
		self->nextthink = gstate->time + hook->frametime;
	}
}


///////////////////////////////////////////////////////////////////////////////
//	target_effect_use
//
///////////////////////////////////////////////////////////////////////////////

void target_effect_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	effectHook_t	*hook = (effectHook_t *) self->userHook;

	hook->endtime = gstate->time + hook->length;
	
	self->think = target_effect_think;
	self->nextthink = gstate->time + hook->frametime;

	target_effect_think(self);
}

///////////////////////////////////////////////////////////////////////////////
//	target_effect
//
///////////////////////////////////////////////////////////////////////////////

void target_effect (userEntity_t *self)
{
	int		i, spawntype;
	effectHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(effectHook_t), MEM_TAG_HOOK);
	hook = (effectHook_t *) self->userHook;

	// initialize settings
	hook->frametime = 0.10;
	hook->particle_type = -1;
	hook->count = -1;
	hook->endtime = 0;
	spawntype = 1;

	rand_count = 0;

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "gravity"))
			spawntype = atoi (self->epair [i].value);

		if (!stricmp (self->epair [i].key, "speed"))
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair[i].key, "_color"))
		{
			sscanf (self->epair[i].value, "%f %f %f", &hook->color.x, &hook->color.y, &hook->color.z);
		}
		else if (!stricmp (self->epair [i].key, "count"))
			hook->count = atoi (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "type"))
			hook->particle_type = atoi (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "sound"))
			hook->sound = self->epair [i].value;

		else if (!stricmp (self->epair [i].key, "length"))
			hook->length = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "frametime"))
			hook->frametime = atof (self->epair [i].value);
	    
		else if (!stricmp(self->epair [i].key, "dir"))
	    {
		    sscanf(self->epair [i].value, "%f%f%f", 
			    &hook->dir.x,
			    &hook->dir.y,
			    &hook->dir.z);
	    }

	}

	self->load = effect_hook_load;
	self->save = effect_hook_save;

	// check the spawn type
	switch (spawntype)
	{
		case 0:
			hook->spawn_type = TE_PARTICLE_FALL;
			break;
		case 1:
			hook->spawn_type = TE_PARTICLE_RISE;
			break;
		case 2:
			hook->spawn_type = TE_PARTICLE_FLOAT;
			break;
		default:
			hook->spawn_type = TE_PARTICLE_RISE;
			break;
	}

	// check the particle type 
	if ((hook->particle_type < PARTICLE_SIMPLE) || (hook->particle_type > NUM_PARTICLETYPES))
	{	
		gstate->Con_Dprintf ("Invalid particle type defined on %s at %s\n", self->className, com->vtos(self->s.origin));
		hook->particle_type = PARTICLE_SMOKE;
	}

	// do we want to randomly spread the particles?
/*	if (self->spawnflags & RANDOM_SPREAD)
	{
		hook->particle_type |= PARTICLE_RANDOM_SPREAD;
	}
*/

	// check the speed
	if (hook->speed == 0)
		hook->speed = 5.0;

	// set the default particle count
	if ((hook->count <= 0) || (hook->count > 64))
	{
		gstate->Con_Dprintf ("Invalid particle count defined on %s at %s\n", self->className, com->vtos(self->s.origin));
		hook->count = 10;
	}

	// pre-cache the sound if it is defined
	if (hook->sound)
		gstate->SoundIndex (hook->sound);

	// set the default caller to light gray if it wasn't specified
	if( ( hook->color.x == 0 ) && ( hook->color.y == 0 ) && ( hook->color.z == 0 ) )
		hook->color.Set( 0.5, 0.5, 0.5 );

	// set the default time between frames
	if (hook->frametime == 0)
		hook->frametime = 0.10;

	// set the default length of time for this effect to last (defaults to 1 frame)
	if (hook->length == 0)
	{
		if( self->spawnflags & TARGET_EFFECT_BEAMS )
		{
			hook->length = 3;
		}
		else
		{
			hook->length = hook->frametime;
		}
	}


	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->flags |= FL_NOSAVE;

	if( self->spawnflags & TARGET_EFFECT_START_ON )
	{
		self->think = target_effect_think;
		self->nextthink = gstate->time + hook->frametime;
	}
	else
	{
		self->think = NULL;
		self->use = target_effect_use;
	}

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	if( sv_violence->value != 0)
#endif
	{
		if( ( hook->particle_type >= PARTICLE_BLOOD ) && ( hook->particle_type <= PARTICLE_BLOOD5 ) )
		{
			self->remove( self );
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//	emitter_remove_attractor
///////////////////////////////////////////////////////////////////////////////

void emitter_remove_attractor (userEntity_t *pAttractor)
{
	attractorHook_t	*ahook = NULL, *hook = (attractorHook_t *) pAttractor->userHook;
	userEntity_t	*pEmitter = hook->emitter;

	// if there is an attractor in the list before us
	if (hook->prevAttractor)
	{
		// set the previous attractor's fields
		ahook = (attractorHook_t *) hook->prevAttractor;
		ahook->nextAttractor = hook->nextAttractor;				// could be NULL
	}
	// if there is an attractor in the list after us
	if (hook->nextAttractor)
	{
		// set the next attractor's fields
		ahook = (attractorHook_t *) hook->nextAttractor;
		ahook->prevAttractor = hook->prevAttractor;
	}

	// if there's no emitter, things are kinda fucked so just delete yourself
	if (pEmitter && pEmitter->inuse)
	{
		// set the emitter fields
		if (!stricmp(pEmitter->className, "effect_lightning"))
		{
			lightningEffectHook_t	*lhook = (lightningEffectHook_t *) pEmitter->userHook;
			lhook->totalAttractors--;
			
			if (lhook->attractorHead == pAttractor)
				lhook->attractorHead = hook->nextAttractor;		// could be NULL
			
			if (lhook->curAttractor == pAttractor)
				lhook->curAttractor = lhook->attractorHead;		// could be NULL
		}
	}

	//	userHook gets freed in RemoveEntity
	gstate->RemoveEntity (pAttractor);
}
///////////////////////////////////////////////////////////////////////////////
//	emitter_add_attractor
///////////////////////////////////////////////////////////////////////////////

qboolean emitter_add_attractor (userEntity_t *pAttractor, userEntity_t *pEmitter)
{
	attractorHook_t	*ahook = NULL, *hook = (attractorHook_t *) pAttractor->userHook;
	userEntity_t	*cur = NULL, *last = NULL;

	// build the attractor linked list
	if (!stricmp(pEmitter->className, "effect_lightning"))
	{
		lightningEffectHook_t	*lhook = (lightningEffectHook_t *) pEmitter->userHook;

		//
		// NOTE: if we ever have more than one attractor type, create a function
		//       for the code below
		//

		// get the first attractor
		cur = lhook->attractorHead;
		
		// is this the first attractor?
		if (!cur)
		{
			// set up the emitter fields
			lhook->attractorHead = pAttractor;
			lhook->curAttractor = pAttractor;
			lhook->totalAttractors++;
			// set the attractor fields
			hook->emitter = pEmitter;
			hook->prevAttractor = NULL;
			hook->nextAttractor = NULL;
		}
		else
		{
			// skip through to the last attractor in the list
			while (cur)
			{
				// get the hook for the attractor
				ahook = (attractorHook_t *) cur->userHook;
				last = cur;
				cur = ahook->nextAttractor;
			}
			// set up the emitter fields
			lhook->totalAttractors++;
			// set up the last attractor fields
			if( ahook != NULL )
				ahook->nextAttractor = pAttractor;
			// set up the attractor fields
			hook->emitter = pEmitter;
			hook->prevAttractor = last;
			hook->nextAttractor = NULL;
		}
	}
	else
	{
		gstate->Con_Dprintf ("undefined emitter type on target_attractor at %s\n", com->vtos (pAttractor->s.origin));
		return (FALSE);
	}

	return (TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//	emitter_find
///////////////////////////////////////////////////////////////////////////////

void emitter_find (userEntity_t *pAttractor)
{
	userEntity_t	*ent;
//	attractorHook_t	*hook = (attractorHook_t *) pAttractor->userHook;// SCG[1/23/00]: not used
	qboolean		bFound = FALSE;


	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->inuse || !ent->className || ent == pAttractor)
			continue;

		if (ent->target && !stricmp (ent->target, pAttractor->targetname))
		{
			//build the attractor linked list
			bFound = emitter_add_attractor(pAttractor, ent);
		}
	}
	
	// did we find the emitter?
	if (!bFound)
		gstate->Con_Dprintf ("target_attractor at %s could not find the emitter\n", com->vtos (pAttractor->s.origin));

	// attractors don't think past this point
	pAttractor->think = NULL;
	pAttractor->nextthink = -1;
}



///////////////////////////////////////////////////////////////////////////////
//	target_attractor
///////////////////////////////////////////////////////////////////////////////

void target_attractor (userEntity_t *self)
{
	attractorHook_t	*hook = NULL;
//	char			*soundStr = NULL;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
//	self->svflags |= SVF_NOCLIENT;

	self->s.effects = EF_GIB;
	self->takedamage = DAMAGE_NO;

	self->use = NULL;
	self->remove = emitter_remove_attractor;

	self->userHook = gstate->X_Malloc(sizeof(attractorHook_t), MEM_TAG_HOOK);
	hook = (attractorHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = attractor_hook_save;
	self->load = attractor_hook_load;
	
	for (int i = 0; self->epair [i].key; i++)
	{
//#if 0	// sounds play from the emitters
//		if (!stricmp (self->epair[i].key, "sound"))	
//			soundStr = self->epair [i].value;
//		else 
//#endif 
		if (!stricmp (self->epair [i].key, "triggerindex"))
			hook->triggerIndex = atoi (self->epair [i].value);
	}

	// if this is a valid attractor...
	if (self->targetname)
	{
#if 0	// sounds play from the emitters
		// set up the sound to play when striking
		if (soundStr)
			hook->sound = gstate->SoundIndex (soundStr);
#endif 
		// set it up to find its matching emitter
		self->think = emitter_find;
		self->nextthink = gstate->time + 0.2 + ((float)hook->triggerIndex/10.0);
	}
	else
	{
		gstate->Con_Dprintf ("target_attractor does not have a targetname at %s\n", com->vtos (self->s.origin));
		self->think = NULL;
	}

	// put this entity in the world
	gstate->LinkEntity (self);

}

///////////////////////////////////////////////////////////////////////////////
//	target_monster_spawn_use
///////////////////////////////////////////////////////////////////////////////

void target_monster_spawn_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	monsterSpawnHook_t	*hook = NULL;

	hook = (monsterSpawnHook_t *) self->userHook;
	bool bIsMonster = bool(strstr(hook->monsterClass, "monster") != NULL);

	// spawn a monster of class hook->monsterclass
	userEntity_t *ent = NULL;

	ent = com->SpawnDynamicEntity (self, hook->monsterClass, bIsMonster);

	if (ent == NULL)
	{
		return;
	}
	else
	{
		// if we spawned a monster, there is special stuff to do
		if (bIsMonster)
		{
			IncrementMonsterCount();
			// if the entity that triggered this spawn was a monster
			// then we should copy it's spawnflags to the new monster
			if (other && (other->flags & FL_MONSTER))
			{
				ent->spawnflags = other->spawnflags;
			}
			else
				ent->spawnflags = self->spawnflags;//NSS[11/20/99]:
	        
			//NSS[11/20/99]:This MUST be here for unique IDs
			int i = 0;
			while(ent->epair[i].key)
			{
				if(_stricmp(ent->epair[i].key,"uniqueid")==0)
				{
					UNIQUEID_Add( ent->epair[i].value, ent );
				}
				i++;
			}
			SPAWN_CallInitFunction( ent, hook->monsterClass );
			//NSS[11/16/99]:Face the direction the target_monster_spawn is facing
			ent->s.angles		= self->s.angles;			
		}
	}

	// play the spawn sound if it is specified
	if (hook->sound)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound, 0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
// NSS[2/21/00]:
// Name:		CacheMonsterSounds
// Description:We need to cache any monster sounds in case this monster doesn't get spawned
// upon level load.
// Input: char *className 
// Output:TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
void CacheMonsterSounds(userEntity_t *self, char *className )
{
	userEntity_t *ent;
	ent = com->SpawnDynamicEntity (self, className, TRUE);
	if(ent)
	{
		SPAWN_CallInitFunction( ent, className );
		gstate->RemoveEntity(ent);
	}
}


///////////////////////////////////////////////////////////////////////////////
//	target_monster_spawn
///////////////////////////////////////////////////////////////////////////////

void target_monster_spawn (userEntity_t *self)
{
	monsterSpawnHook_t	*hook = NULL;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;

	self->takedamage = DAMAGE_NO;

	self->use = target_monster_spawn_use;

	self->userHook = gstate->X_Malloc(sizeof(monsterSpawnHook_t), MEM_TAG_HOOK);
	hook = (monsterSpawnHook_t *) self->userHook;

	self->save = monster_spawn_hook_save;
	self->load = monster_spawn_hook_load;
	
//	self->flags |= FL_NOSAVE;

	for (int i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair[i].key, "sound"))	
			hook->sound = gstate->SoundIndex (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "monsterclass"))
			hook->monsterClass = self->epair [i].value;

		else if (!stricmp (self->epair [i].key, "aistate"))	
			hook->aiStateStr = self->epair [i].value;
	}

	// is this valid?
	if (!hook->monsterClass)
	{
		gstate->Con_Dprintf ("target_monster_spawn does not have a monsterclass key at %s\n", com->vtos (self->s.origin));
		self->remove(self);
		return;
	}
	else
	{
		// pre-cache the monster so there is no hitch when it's first loaded
		CacheMonsterModel  ( hook->monsterClass );
		CacheMonsterSounds ( self, hook->monsterClass );
	}
}

//==========================================================

/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"		wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"	0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/

///////////////////////////////////////////////////////////////////////////////
//	target_speaker_use
//
///////////////////////////////////////////////////////////////////////////////

void target_speaker_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	int		chan, soundIdx;

	speakerHook_t* hook = (speakerHook_t*)self->userHook;
	
	//trap to set the thinking back on if we were originally off
	if(self->nextthink == 0.0f)
		self->nextthink = gstate->time + 0.1;

	if (self->spawnflags & LOOPED_ON || self->spawnflags & LOOPED_OFF)
	{	// looping sound toggles
		if (self->s.sound)
			self->s.sound = 0;				// turn it off
		else
			self->s.sound = hook->sound[0];	// start it
	}
	else
	{	// normal sound
		if (self->spawnflags & RELIABLE)
			chan = CHAN_VOICE|CHAN_RELIABLE;
		else
			chan = CHAN_VOICE;

		if (hook->count)
			soundIdx = rand() % hook->count;
		else
			soundIdx = 0;

		// use a positioned_sound, because this entity won't normally be
		// sent to any clients because it is invisible
		// mdm 99.07.13 - doesn't matter sound engine knows this already and does it correctly
		gstate->StartEntitySound(self, chan, hook->sound[soundIdx], hook->volume, self->s.dist_min, self->s.dist_max);
	}
}


///////////////////////////////////////////////////////////////////////////////
//	target_speaker_use
//
///////////////////////////////////////////////////////////////////////////////

void target_speaker_think (userEntity_t *self)
{
	int		chan, soundIdx;

	speakerHook_t* hook = (speakerHook_t*)self->userHook;
	

	
	// normal sound
	if (self->spawnflags & RELIABLE)
		chan = CHAN_VOICE|CHAN_RELIABLE;
	else
		chan = CHAN_VOICE;

	if (hook->count)
		soundIdx = rand() % hook->count;
	else
		soundIdx = 0;
	
	// use a positioned_sound, because this entity won't normally be
	// sent to any clients because it is invisible
	gstate->StartEntitySound(self, chan, hook->sound[soundIdx], hook->volume, self->s.dist_min, self->s.dist_max);

	// set up the next time to play a sound
	int randomSecs = rand() % hook->delay;
	if (randomSecs < hook->minDelay)
		randomSecs = hook->minDelay;

	self->nextthink = gstate->time + randomSecs;

}


///////////////////////////////////////////////////////////////////////////////
//	target_speaker
//
//	"noise"	- wav file to play
//	"attenuation"
//	-1 = none, send to whole level
//	 1 = normal fighting sounds
//	 2 = idle sound level
//	 3 = ambient sound level
//	"volume"	0.0 to 1.0
//
//  Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.
//
//	Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
//	Multiple identical looping sounds will just increase volume without any speed cost.
//
//
///////////////////////////////////////////////////////////////////////////////

// SCG[11/24/99]: Defined in misc.cpp
void speaker_hook_save( FILE *f, edict_t *ent );
void speaker_hook_load( FILE *f, edict_t *ent );

void target_speaker (userEntity_t *self)
{
	// set up the physics for this entity
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;		// this entity is never sent to the client
	
	// alloc a hook
	// SCG[8/16/99]: sound_ambient, which calls target_speaker, has to allocate it's
	// SCG[8/16/99]: own user hook to get the volume ot set a default if one does not exist
	if( self->userHook == NULL )
	{
		self->userHook = gstate->X_Malloc(sizeof(speakerHook_t), MEM_TAG_HOOK);
	}
	speakerHook_t* hook = (speakerHook_t*) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = speaker_hook_save;
	self->load = speaker_hook_load;

	// loop through epairs
	for (int i = 0; self->epair [i].key; i++)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		// amw: check "sound" to maintain compatibility
		// Logic[5/26/99]: if (strstr (strlwr(self->epair [i].key), "sound"))
        if (strstr (self->epair [i].key, "sound"))
		{
			if (hook->count < MAX_TARGET_SPEAKER_SNDS)
				hook->sound[hook->count++] = gstate->SoundIndex (self->epair [i].value);
		}
//		else if (!stricmp (self->epair [i].key, "attenuation"))
//			hook->attenuation = atof(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			hook->volume = atof(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			hook->delay = atoi(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "mindelay"))
			hook->minDelay = atoi(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atoi(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atoi(self->epair[i].value);
	}

	// setup defaults for min/max if necessary
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;

	if (self->s.dist_min >= self->s.dist_max) self->s.dist_min = 0.0f;
	
	if (!hook->volume)
		hook->volume = 1.0;

	// check for prestarted looping sound
	if ((hook->count <= 1) && (self->spawnflags & LOOPED_ON))
	{
		self->s.sound = hook->sound[0];	// use the default index 0 sound
	}
	else
	{
		// set the default sound delay if there are multiple waves, no delay
		// and no targetname (i.e. you're not trying to trigger a random wave)
		if ((hook->count > 1) && (hook->delay == 0.0f) && !self->targetname)
				hook->delay = 3.0f;

		// turn off the looping flag, it isn't eligible
		self->spawnflags &= ~LOOPED_ON;

		if (hook->delay > 0)
		{
			self->think = target_speaker_think;
			if( ( self->spawnflags & START_OFF ) || ( self->spawnflags & LOOPED_OFF ) )
			{
				self->nextthink = 0;
			}
			else
			{
				self->nextthink = gstate->time + 1.0;
			}
		}
	}

	self->use = target_speaker_use;

	// setup entity volume
	self->s.volume = hook->volume;

	if (self->spawnflags & NON_DIRECTIONAL)
		self->s.snd_flags = SND_NONDIRECTIONAL;

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	
	if (self->spawnflags & LOOPED_ON || self->spawnflags & LOOPED_OFF)
	{
		self->svflags &= ~SVF_NOCLIENT;		// this entity is should be to the client
		gstate->LinkEntity(self);	//amw: 4.29.99 - only needed for looping entities
	}
}

/* --------------------------- target_lightray ----------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	target_lightray_think
//
///////////////////////////////////////////////////////////////////////////////

/*
void	target_lightray_think (userEntity_t *self)
{
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
	self->s.old_origin = self->enemy->s.origin;

	self->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//	target_lightray_on
//
///////////////////////////////////////////////////////////////////////////////

void	target_lightray_on (userEntity_t *self)
{
	CVector			dir, ang;

	self->spawnflags |= 1;			//	flag as on
	self->svflags &= ~SVF_NOCLIENT;	//	send to client

	if (!self->enemy)
	{
		//	set up angles here for renderer to use when drawing
		//	this way it doesn't have to be done every think
		dir = self->movedir;
		VectorToAngles( dir, ang );
		self->s.angles = ang;
	}
	else
	{
		self->s.old_origin = self->enemy->s.origin;
	}

	self->think = target_lightray_think;
	self->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//	target_lightray_off
//
///////////////////////////////////////////////////////////////////////////////

void	target_lightray_off (userEntity_t *self)
{
	self->spawnflags &= ~1;			//	flag as off
	self->svflags |= SVF_NOCLIENT;	//	send to client
}

///////////////////////////////////////////////////////////////////////////////
//	target_lightray_use
//
///////////////////////////////////////////////////////////////////////////////

void	target_lightray_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (self->spawnflags & 1)
		target_lightray_off (self);
	else
		target_lightray_on (self);
}

///////////////////////////////////////////////////////////////////////////////
//	target_lightray_start
//
///////////////////////////////////////////////////////////////////////////////

void	target_lightray_start (userEntity_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_LIGHTRAY | RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero or entity won't get sent
									// to client
	self->s.alpha = 0.1;

	//	set the beam diameter - starting radius is encoded in upper 8 bits
	//	ending radius in the lower 8 bits -- this is a hack to make the network
	//	messages be a little faster
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	if (!self->owner)
		self->owner = self;

	if (!self->enemy)
	{
		if (self->target)
			self->enemy = com->FindTarget (self->target);
		else
			com->SetMovedir (self);
	}

	self->use = target_lightray_use;

	gstate->LinkEntity (self);

	if (self->spawnflags & 1)
		target_lightray_on (self);
	else
		target_lightray_off (self);

}

///////////////////////////////////////////////////////////////////////////////
//	target_lightray
//
///////////////////////////////////////////////////////////////////////////////

void	target_lightray (userEntity_t *self)
{
	int			i, startRadius = 4, endRadius = 16;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "startradius"))
			startRadius = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "endradius"))
			endRadius = atoi (self->epair [i].value);
	}

	//	encode end radius in upper 8 bits, start radius in lower 8
	self->s.frame = (endRadius << 8) | startRadius;

	// let everything else get spawned before we start firing
	self->think = target_lightray_start;
	self->nextthink = gstate->time + 0.1;
}
*/
#define	THROW_CLIENT_ONLY	1

///////////////////////////////////////////////////////////////////////////////
//	target_throw_use
//
//	throws the player at specified velocity in current direction of movement
///////////////////////////////////////////////////////////////////////////////

/*
void	target_throw_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (self->spawnflags & THROW_CLIENT_ONLY && !(other->flags & FL_CLIENT))
		return;

	CVector dir = other->velocity;
	dir.z = 0.0f;
	dir.Normalize();

	dir.x = dir.x * self->health;
	dir.x = dir.y * self->health;
	dir.z = self->hacks;

	other->velocity = dir;
	other->groundEntity = NULL;
	other->groundSurface = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	target_throw
//
//	throws the player at specified velocity in current direction of movement
///////////////////////////////////////////////////////////////////////////////

void	target_throw (userEntity_t *self)
{
	int			i, startRadius = 4, endRadius = 16;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "xy_vel"))
			self->health = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "z_vel"))
			self->hacks = atoi (self->epair [i].value);
	}

	self->svflags |= SVF_NOCLIENT;

	self->use = target_throw_use;
}
*/
/*
///////////////////////////////////////////////////////////////////////////////
//
//	target_turret_findtarget
//
///////////////////////////////////////////////////////////////////////////////

bool target_turret_find_target (userEntity_t *self, bool bRotateAndAttack)
{
	userEntity_t	*pClient = NULL, *pTarget = self->enemy;
	turretHook_t	*hook = (turretHook_t*) self->userHook;
	bool			bInRange = FALSE;
	trace_t			tr;

	// do we already have a target?
	if (pTarget)
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
		if (bInRange)
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
		float maxPitch = 0.2f;

		if( fabs(pitch) < maxPitch )
		{
			//dir.z = 0.0f;

			VectorToAngles( dir, angles );
			self->s.angles = angles;

			// is it time for us to attack again?
			if (gstate->time < hook->nextAttack)
				return TRUE;

			// now do a trace and see if we can fire
			userEntity_t *pIgnore = self;
			// calculate an end point for the projectile
			CVector endPt = self->s.origin + dir * hook->range;

			// trace from the gun out to the target
			tr = gstate->TraceLine_q2 (self->s.origin, endPt, pIgnore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			// if we can hit the target, fire
			if( tr.ent == pTarget && fabs(pitch) < maxPitch )
			{
				// play the attack sound
				if (hook->sound)
					gstate->StartEntitySound(self, CHAN_WEAPON, hook->sound, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				
				// actually fire the projectile
				ai_fire_curWeapon( self );
				// set the time for the next attack
				hook->nextAttack = gstate->time + hook->fire_rate;
			}
		}
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//	target_turret_think
//
///////////////////////////////////////////////////////////////////////////////

void target_turret_think (userEntity_t *self)
{
	turretHook_t* hook = (turretHook_t*) self->userHook;

	switch (hook->state)
	{
		case TS_RAISING:
		{
			if (hook->sound_up)
				gstate->StartEntitySound(self, CHAN_WEAPON, hook->sound_up, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			if (hook->height)
				com->AnimateEntity(self, 0, hook->height-1, FRAME_ONCE, 0.10);
			// give it a delay so that the turret can raise
			hook->nextAttack = gstate->time + 1.0;

			hook->state = TS_SCANNING;
			break;
		}
		case TS_LOWERING:
		{
			// zero out the rotation
			self->s.angles.Set( 0.0, 0.0, 0.0 );

			if (hook->sound_down)
				gstate->StartEntitySound(self, CHAN_WEAPON, hook->sound_down, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);

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
			if (!target_turret_find_target (self, TRUE))	// find target and fire
			{
				// only lower if this is a passive scanning turret
				if (!self->use)
					hook->state = TS_LOWERING;
			}
			break;
		}
		case TS_PASSIVESCAN:
		{
			if (target_turret_find_target (self, FALSE))	// find target and raise
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
//	target_turret_use
//
///////////////////////////////////////////////////////////////////////////////

void target_turret_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
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
	self->think = target_turret_think;
	self->nextthink = gstate->time + 0.1;

}


///////////////////////////////////////////////////////////////////////////////
//
//	target_turret
//
///////////////////////////////////////////////////////////////////////////////

void target_turret (userEntity_t *self)
{
	// set default model name
	char *pModelName = "models/e1/a1_rockgun.dkm";

	// set up the physics for this entity
	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_BBOX;
	self->netname = "Auto Turret";
	self->health = 100;									// default health
	
	// alloc a hook
	self->userHook = gstate->X_Malloc(sizeof(turretHook_t), MEM_TAG_HOOK);
	turretHook_t* hook = (turretHook_t*) self->userHook;

	self->save = turret_hook_save;
	self->load = turret_hook_load;

	// init hook vals
	self->s.modelindex = gstate->ModelIndex (pModelName);	// default gun model
	hook->height = 10;									// default popup height (frames)
	hook->sound = 0;									// sound when firing
	hook->sound_up = 0;									// sound when raising
	hook->sound_down = 0;								// sound when lowering
	hook->hit_sound = 0;								// sound when destroyed
	hook->ammo_type	= 0;								// default ammo type (laser bolts)
	hook->fire_rate = 0.20f;							// default time between firings (def: 5 times per sec)
	hook->range		= 512.0;							// default turret range
	hook->state = TS_DISABLED;							// set default state
	//hook->basedmg = 15.0;								// default base damage value
	//hook->rnddmg = 10.0;								// default random damage value
	hook->basedmg = 5.0;								// default base damage value
	hook->rnddmg =  2.0;								// default random damage value


	// loop through epairs
	for (int i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "model"))
			self->s.modelindex = gstate->ModelIndex (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "height"))
			hook->height = atoi(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "frames"))
			hook->height = atoi(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			hook->sound = gstate->SoundIndex (self->epair [i].value);
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

	if (self->spawnflags & TURRET_TOGGLE)
	{
		hook->state = TS_DISABLED;
		self->spawnflags &= ~TURRET_ON;
		self->use = target_turret_use;
		self->think = NULL;
	}
	else
	{
		hook->state = TS_PASSIVESCAN;
		self->use = NULL;
		self->think = target_turret_think;
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

    //set the size of the object.
    gstate->SetSize (self, -16, -16, -24, 16, 16, -8);

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	gstate->LinkEntity(self);

	self->save = rockgat_hook_save;
	self->load = rockgat_hook_load;

	// give this entity an inventory so he can use the monster weapons
	self->inventory = gstate->InventoryNew (MEM_MALLOC);
	self->curWeapon = ai_init_weapon (self, hook->basedmg, hook->rnddmg, 0, 0, 950.0, hook->range, CVector(0, 0, 0), "laser", laser_fire, ITF_PROJECTILE);
	gstate->InventoryAddItem (self, self->inventory, self->curWeapon);

}
*/
void target_register_func()
{
	gstate->RegisterFunc("target_laser_use",target_laser_use);
	gstate->RegisterFunc("target_earthquake_use",target_earthquake_use);
	gstate->RegisterFunc("target_spotlight_use",target_spotlight_use);
	gstate->RegisterFunc("target_lightramp_use",target_lightramp_use);
// SCG[12/5/99]: 	gstate->RegisterFunc("target_framechange_use",target_framechange_use);
	gstate->RegisterFunc("target_effect_use",target_effect_use);
	gstate->RegisterFunc("target_monster_spawn_use",target_monster_spawn_use);
	gstate->RegisterFunc("target_speaker_use",target_speaker_use);
// SCG[12/5/99]: 	gstate->RegisterFunc("target_lightray_use",target_lightray_use);
//	gstate->RegisterFunc("target_throw_use",target_throw_use);
//	gstate->RegisterFunc("target_turret_use",target_turret_use);

	gstate->RegisterFunc("target_laser_think",target_laser_think);
	gstate->RegisterFunc("target_laser_start",target_laser_start);
	gstate->RegisterFunc("target_earthquake_think",target_earthquake_think);
	gstate->RegisterFunc("target_spotlight_think",target_spotlight_think);
	gstate->RegisterFunc("target_spotlight_start",target_spotlight_start);
	gstate->RegisterFunc("target_lightramp_think",target_lightramp_think);
	gstate->RegisterFunc("target_effect_think",target_effect_think);
	gstate->RegisterFunc("emitter_find",emitter_find);
	gstate->RegisterFunc("target_speaker_think",target_speaker_think);
// SCG[12/5/99]: 	gstate->RegisterFunc("target_lightray_think",target_lightray_think);
// SCG[12/5/99]: 	gstate->RegisterFunc("target_lightray_start",target_lightray_start);
//	gstate->RegisterFunc("target_turret_think",target_turret_think);
	gstate->RegisterFunc("emitter_remove_attractor",emitter_remove_attractor);

	gstate->RegisterFunc( "effect_hook_save", effect_hook_save );
	gstate->RegisterFunc( "effect_hook_load", effect_hook_load );
	gstate->RegisterFunc( "attractor_hook_save", attractor_hook_save );
	gstate->RegisterFunc( "attractor_hook_load", attractor_hook_load );

	gstate->RegisterFunc( "monster_spawn_hook_save", monster_spawn_hook_save );
	gstate->RegisterFunc( "monster_spawn_hook_load", monster_spawn_hook_load );
	gstate->RegisterFunc( "light_ramp_hook_save", light_ramp_hook_save );
	gstate->RegisterFunc( "light_ramp_hook_load", light_ramp_hook_load );
	gstate->RegisterFunc( "laser_hook_save", laser_hook_save );
	gstate->RegisterFunc( "laser_hook_load", laser_hook_load );
	gstate->RegisterFunc( "frame_change_hook_load", frame_change_hook_load );
	gstate->RegisterFunc( "frame_change_hook_save", frame_change_hook_save );
	gstate->RegisterFunc( "quake_hook_save", quake_hook_save );
	gstate->RegisterFunc( "quake_hook_load", quake_hook_load );
}














