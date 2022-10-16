#include	"world.h"

//#include	"triggers.h"// SCG[1/23/00]: not used
//#include	"actorlist.h"// SCG[1/23/00]: not used
#include	"misc.h"
//#include	<windows.h>  		// temp *jas* just used to call GetVersionE-()
#include	"dk_system.h"
#include "ai_utils.h"

//DllExport	void	func_floater (userEntity_t *self);
DllExport	void	func_debris (userEntity_t *self);
DllExport	void	func_debris_visible (userEntity_t *self);
DllExport	void	info_null (userEntity_t *self);
DllExport	void	info_not_null (userEntity_t *self);
// SCG[2/13/00]: DllExport	void	func_particlefield (userEntity_t *self);
DllExport	void	misc_lavaball_drop (userEntity_t *self);
DllExport	void	misc_lavaball_toss (userEntity_t *self);
DllExport	void	func_explosive (userEntity_t *self);
// SCG[12/5/99]: DllExport	void	func_talking_head (userEntity_t *self);
DllExport	void	sound_ambient (userEntity_t *self);

DllExport	void	func_event_generator( userEntity_t *self );

////////////////////////////////////////////////////////////////////////////////
// decorations
////////////////////////////////////////////////////////////////////////////////

DllExport	void	deco_e1		(userEntity_t *self);
DllExport	void	deco_e2		(userEntity_t *self);
DllExport	void	deco_e3		(userEntity_t *self);
DllExport	void	deco_e4		(userEntity_t *self);


#define MAX_DEBRIS 40.0f
////////////////////////////////////////////////////////////////////////////////
// EXPLOSION DEFINES
////////////////////////////////////////////////////////////////////////////////

#define	EXPL_TRIGGER_SPAWN		0x0001
#define	EXPL_ANIM				0x0002
#define	EXPL_ANIMFAST			0x0004
#define	EXPL_STONE				0x0008
#define	EXPL_WOOD				0x0010
#define	EXPL_METAL				0x0020
//#define	EXPL_RANDOM_FALL		0x0040
#define EXPL_NO_CHUNKS			0x0040
#define EXPL_NO_SOUND			0x0080
#define EXPL_NO_EXPLOSION		0x0100
#define EXPL_TRIGGER_NOT_SOLID	0x0200

userEntity_t *throw_debris (char *modelname, float speed, CVector &origin, int renderfx, CVector &scale, CVector &velocity, float delay);
void com_FindParent (userEntity_t *self);
void com_PlayEntityScript(userEntity_t *pActivator, char *pCineScript, char *pAiScript);
float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type);

static	int	temp_count;
int			func_explosive_count;

// func_floater defines
#define		RANDOM_WANDER	1
#define		ROTATE_YAW		2

// func_debris defines
#define		GO_TO_ACTIVATOR		1
#define		NO_ROTATE			2
#define		MOMENTUM_DAMAGE		4
#define		NO_ROTATION_ADJUST	8
#define		DROP_ONLY			16
#define		QUARTER_SIZE		32

//	deco_ defines
#define		DECO_EXPLODE		0x0001
#define		DECO_NO_BREAK		0x0002
#define		DECO_PUSHABLE		0x0004
#define		DECO_WOOD     		0x0008
#define		DECO_METAL    		0x0010
#define		DECO_GLASS    		0x0020
#define		DECO_GIB    		0x0040
#define		DECO_ROTATE			0x0080
#define     DECO_TRANSLUCENT    0x0100      // 1.19 dsn


/*
enum TALKING_HEAD_SPAWNFLAGS 
{
	HEAD_START_ON			=	0x0001,		// the head will start on
	HEAD_ON					=   0x0001,		// the head is currently on
	HEAD_TRACK_CLIENT		=	0x0002,		// head will look towards the client
	HEAD_HOLOGRAPH			=	0x0004,		// head will be displayed with translucency
	HEAD_USE_TOGGLE			=	0x0008,		// using or targeting will do specific actions, default
};											// is to turn it on and off
*/

// SCG[11/19/99]: 
#define	DEBRISHOOK(x) (int)&(((debrisHook_t *)0)->x)
field_t debris_hook_fields[] = 
{
	{"modelName",		DEBRISHOOK(modelName),		F_LSTRING},
	{"fly_sound",		DEBRISHOOK(fly_sound),		F_LSTRING},
	{"hit_sound",		DEBRISHOOK(hit_sound),		F_LSTRING},
	{"has_rotated",		DEBRISHOOK(has_rotated),	F_INT},
	{"damage",			DEBRISHOOK(damage),			F_FLOAT},
	{"last_water",		DEBRISHOOK(last_water),		F_FLOAT},
	{"change_time",		DEBRISHOOK(change_time),	F_FLOAT},
	{"rotation_scale",	DEBRISHOOK(rotation_scale),	F_FLOAT},
	{"mass_scale",		DEBRISHOOK(mass_scale),		F_FLOAT},
	{"floor_distance",	DEBRISHOOK(floor_distance),	F_FLOAT},
	{"last_pos",		DEBRISHOOK(last_pos),		F_VECTOR},
	{"hit_water_pos",	DEBRISHOOK(hit_water_pos),	F_VECTOR},
	{"dest",			DEBRISHOOK(dest),			F_VECTOR},
	{"avel",			DEBRISHOOK(avel),			F_VECTOR},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	PARTICLEHOOK(x) (int)&(((particleHook_t *)0)->x)
field_t particle_hook_fields[] = 
{
	{"dest",			PARTICLEHOOK(dest),				F_VECTOR},
	{"dest1",			PARTICLEHOOK(dest1),			F_VECTOR},
	{"dest2",			PARTICLEHOOK(dest2),			F_VECTOR},
	{"impact_vel",		PARTICLEHOOK(impact_vel),		F_VECTOR},
	{"touch_sound",		PARTICLEHOOK(touch_sound),		F_LSTRING},
	{"trigger_sound",	PARTICLEHOOK(trigger_sound),	F_LSTRING},
	{"amb_sound",		PARTICLEHOOK(amb_sound),		F_LSTRING},
	{"damage",			PARTICLEHOOK(damage),			F_FLOAT},
	{"attack_finished",	PARTICLEHOOK(attack_finished),	F_FLOAT},
	{"hold_time",		PARTICLEHOOK(hold_time),		F_FLOAT},
	{"count",			PARTICLEHOOK(count),			F_INT},
	{"color",			PARTICLEHOOK(color),			F_VECTOR},
	{"last_color",		PARTICLEHOOK(last_color),		F_VECTOR},
	{"other",			PARTICLEHOOK(other),			F_EDICT},
	{"think_use",		PARTICLEHOOK(think_use),		F_FUNC},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	LAVABALLHOOK(x) (int)&(((lavaballHook_t *)0)->x)
field_t lavaball_hook_fields[] = 
{
	{"targ_origin",	LAVABALLHOOK(targ_origin),	F_VECTOR},
	{"time_min",	LAVABALLHOOK(time_min),		F_FLOAT},
	{"time_rng",	LAVABALLHOOK(time_rng),		F_FLOAT},
	{"z_min",		LAVABALLHOOK(z_min),		F_FLOAT},
	{"z_rng",		LAVABALLHOOK(z_rng),		F_FLOAT},
	{"damage",		LAVABALLHOOK(damage),		F_FLOAT},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	FUNCEXPLOHOOK(x) (int)&(((funcExploHook_t *)0)->x)
field_t func_explode_hook_fields[] = 
{
	{"damage",		FUNCEXPLOHOOK(damage),		F_INT},
	{"count",		FUNCEXPLOHOOK(count),		F_INT},
	{"rndcount",	FUNCEXPLOHOOK(rndcount),	F_INT},
	{"Sounds",		FUNCEXPLOHOOK(Sounds),		F_INT},
	{"gravity",		FUNCEXPLOHOOK(gravity),		F_FLOAT},
	{"scale",		FUNCEXPLOHOOK(scale),		F_FLOAT},
	{"speed",		FUNCEXPLOHOOK(speed),		F_FLOAT},
	{"personality",	FUNCEXPLOHOOK(personality),	F_FLOAT},
	{"alpha",		FUNCEXPLOHOOK(alpha),		F_FLOAT},
	{"pCineScript",	FUNCEXPLOHOOK(pCineScript),	F_LSTRING},
	{"pAiScript",	FUNCEXPLOHOOK(pAiScript),	F_LSTRING},
	{"target",		FUNCEXPLOHOOK(target),		F_EDICT},
	{"nextTarget",	FUNCEXPLOHOOK(nextTarget),	F_EDICT},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: 
#define	DECOHOOK(x) (int)&(((decoHook_t *)0)->x)
field_t deco_hook_fields[] = 
{
	{"damage",		DECOHOOK(damage),		F_INT},
	{"inflictor",	DECOHOOK(inflictor),	F_EDICT},
	{"attacker",	DECOHOOK(attacker),		F_EDICT},
	{"damage_done",	DECOHOOK(damage_done),	F_INT},
	{"point",		DECOHOOK(point),		F_VECTOR},
	{"explo_count",	DECOHOOK(explo_count),	F_INT},
	{"explo_max",	DECOHOOK(explo_max),	F_INT},
	{"frame",		DECOHOOK(frame),		F_INT},
	{"animseq",		DECOHOOK(animseq),		F_INT},
	{"seq",			DECOHOOK(seq),			F_INT},
	{NULL, 0, F_INT}
};

#define MAX_TARGET_SPEAKER_SNDS		6
typedef struct	speakerHook_s
{
	int			sound[MAX_TARGET_SPEAKER_SNDS];		
	int			count;
	int			delay;
	int			minDelay;
	float		volume;
} speakerHook_t;

// SCG[11/19/99]: 
#define	SPEAKERHOOK(x) (int)&(((speakerHook_t *)0)->x)
field_t speaker_hook_fields[] = 
{
	{"sound",		SPEAKERHOOK(sound),		F_INT},
	{"count",		SPEAKERHOOK(count),		F_INT},
	{"delay",		SPEAKERHOOK(delay),		F_INT},
	{"minDelay",	SPEAKERHOOK(minDelay),	F_INT},
	{"volume",		SPEAKERHOOK(volume),	F_FLOAT},
	{NULL, 0, F_INT}
};

void debris_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, debris_hook_fields, sizeof( debrisHook_t ) );
}

void debris_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, debris_hook_fields, sizeof( debrisHook_t ) );
}

void particle_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, particle_hook_fields, sizeof( particleHook_t ) );
}

void particle_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, particle_hook_fields, sizeof( particleHook_t ) );
}

void lavaball_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, lavaball_hook_fields, sizeof( lavaballHook_t ) );
}

void lavaball_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, lavaball_hook_fields, sizeof( lavaballHook_t ) );
}

void func_explode_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, func_explode_hook_fields, sizeof( funcExploHook_t ) );
}

void func_explode_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, func_explode_hook_fields, sizeof( funcExploHook_t ) );
}

void deco_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, deco_hook_fields, sizeof( decoHook_t ) );
}

void deco_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, deco_hook_fields, sizeof( decoHook_t ) );
}

void speaker_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, speaker_hook_fields, sizeof( speakerHook_t ) );
}

void speaker_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, speaker_hook_fields, sizeof( speakerHook_t ) );
}

// ----------------------------------------------------------------------------
// <nss>
// Name:FindEntityFromName
// Description:Will find the entity from its targetname
// Input:char *name (name to look for)
// Output:userEntity_t * (entity it found) or NULL for no Entity
// Note:
// ----------------------------------------------------------------------------

userEntity_t * findEntityFromName(char *name)
{
	userEntity_t *ent = gstate->FirstEntity();
	
	//return com->FindEntity (name);
	int found = 0;
	//Process through all the entities on the map
	while(ent)
	{
		//make sure it has a targetname
		if(ent->targetname)
		{
			//Check for TargetName
			if(_stricmp(ent->targetname, name)==0)
			{
				//Check its distance and make sure it has some active wisps
				found = 1;
				break;
			}
		}
		ent = gstate->NextEntity(ent);
	}
	if(found == 0 )
		ent = NULL;
	return ent;
}



///////////////////////////////////////////////////////////////////////////////
//	spawn_explosion
//
//	now done with a temporary entity
///////////////////////////////////////////////////////////////////////////////

void	spawn_sprite_explosion (userEntity_t *self, CVector &org, int play_sound)
{
	gstate->WriteByte (SVC_TEMP_ENTITY);

	//	FIXME:	make a separate te_explosion type for no sound	
	if (play_sound)
		gstate->WriteByte (TE_EXPLOSION1);
	else
		gstate->WriteByte (TE_EXPLOSION2);

	gstate->WritePosition (org);
	gstate->MultiCast (org, MULTICAST_PVS);
}

///////////////////////////////////////////////////////////////////////////////
//	spawn_small_sprite_explosion
//
//	now done with a temporary entity
///////////////////////////////////////////////////////////////////////////////

void	spawn_small_sprite_explosion (userEntity_t *self, CVector &org, int play_sound)
{
	gstate->WriteByte (SVC_TEMP_ENTITY);

	//	FIXME:	make a separate te_explosion type for no sound	
	if (play_sound)
		gstate->WriteByte (TE_EXPLOSION_SMALL1);
	else
		gstate->WriteByte (TE_EXPLOSION_SMALL2);

	gstate->WritePosition (org);
	gstate->MultiCast (org, MULTICAST_PVS);
}


/*
===============
func_debris_water_friction

adds friction and random motion to an object that is 
travelling through water
===============
*/

void	func_debris_water_friction (userEntity_t *self)
{
	debrisHook_t	*hook = (debrisHook_t *) self->userHook;
	CVector			dir, ang, org;
	float			pc, wobble_speed, wander_speed;

	org = (self->absmin + self->absmax) * 0.5;
	pc = gstate->PointContents( org );

	if (pc != CONTENTS_LAVA && pc != CONTENTS_SLIME && pc != CONTENTS_WATER)
	{
		if (hook->last_water == CONTENTS_LAVA || hook->last_water == CONTENTS_SLIME ||
			hook->last_water == CONTENTS_WATER)
		{
			// just left water.  if this is a self-propelled MOVETYPE_FLYMISSILE then 
			// scale its velocity by 2
			if (self->movetype == MOVETYPE_FLYMISSILE)
			{
				dir = self->velocity * 2;
				self->velocity = dir;
			}
			
			// make sure gravity goes back to normal
			self->gravity = 1.0;
		}

		hook->last_water = pc;

		return;
	}

	if (hook->last_water != pc)
	{
		// just entered the water, so scale velocity by half
		hook->hit_water_pos = org;
		dir = self->velocity * 0.5 * hook->mass_scale;
		self->velocity = dir;
		hook->change_time = gstate->time;

		// make it fall slower
		self->gravity = 0.5 * hook->mass_scale;

//		gstate->sound (self, CHAN_AUTO, "bloop.wav", 255, ATTN_NORM);
	}
	else if (!self->targetname)
	{
		if ((self->movetype == MOVETYPE_BOUNCE) || (self->movetype == MOVETYPE_TOSS))
		{
			if (hook->change_time <= gstate->time)
			{
				hook->hit_water_pos.x = org.z + (rnd () - 0.2) * (rnd () * 256.0 + 32.0);
	
				dir = hook->hit_water_pos - org;
				dir.Normalize ();
	
				VectorToAngles( dir, ang );
				ang.AngleToVectors(forward, right, up);
			
				// randomly choose left or right
				if (rnd () < 0.5)
					right = right * -1.0;
	
				// randomly choose towards or away
				if (rnd () < 0.5)
					right = right * -1.0;
					
				wobble_speed = rnd () * 240;
				wander_speed = rnd () * 180;

				dir = self->velocity + dir * wander_speed;
				self->velocity = dir;

				right = right * wobble_speed;
				self->velocity = right;

				hook->change_time = gstate->time + rnd() * 1.0 + 0.1;

				// change angular velocity 
				self->avelocity.Set( hook->rotation_scale * (rnd() - 0.5) * 700.0, hook->rotation_scale * (rnd() - 0.5) * 700.0, hook->rotation_scale * (rnd() - 0.5) * 700.0);
			}

			self->velocity.x = self->velocity.x * 0.5 * hook->mass_scale;
			self->velocity.y = self->velocity.y * 0.5 * hook->mass_scale;
		}
	}

	hook->last_water = pc;
}

/*
==================
func_debris_think

Nelno says "I da man!"
==================
*/

void	func_debris_think (userEntity_t *self)
{
	debrisHook_t	*hook = (debrisHook_t *) self->userHook;
	CVector temp;
//	func_debris_water_friction (self);

	if(hook->change_time < gstate->time)
	{
		
		if(hook->change_time != 0.0f)
		{
			temp = (self->absmax - self->s.origin) * 2.25;
			self->s.maxs = temp;
		
			temp = (self->absmin - self->s.origin) * 2.25;
			self->s.mins = temp;
			hook->change_time = 0.0f;
		}

		// if it is not on the ground and it's velocity magnitude is > 80 set think and leave
		// <nss> Hmmm.  Why not determine what our velocity is and if it is something less than say 100 let's zero out our velocity 
//		float distance = VectorDistance( self->s.origin, hook->last_pos);// SCG[1/23/00]: not used
		if((self->delay+6.0f < gstate->time)||(self->delay + 2.0f < gstate->time && self->velocity.Length() == 0.0f))
		{
			self->solid = SOLID_BBOX;
			self->movetype = MOVETYPE_NONE;
			self->think = NULL;
			self->touch = NULL;
			self->use   = NULL;
			return;
		}
	}
	self->nextthink = gstate->time + 0.1;
	self->think = func_debris_think;

}

/*
==================
func_debris_touch

damages the entity it touches
==================
*/

void	func_debris_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	debrisHook_t	*hook = (debrisHook_t *) self->userHook;
	float			damage;

	if (other->takedamage)
	{
		// calls with the activator as the attacker, so you can get a frag for
		// killing with debris
		if (self->spawnflags & MOMENTUM_DAMAGE)
			damage = self->velocity.Length() / hook->damage;
		else
			damage = hook->damage;
		
		// kick ass hack
		if (!gstate->activator)
			gstate->activator = self;

		com->Damage (other, self, gstate->activator, zero_vector, zero_vector, damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
	}
	if (hook->hit_sound)
	{

		gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex(hook->hit_sound), self->s.volume, self->s.dist_min, self->s.dist_max);
	}
}


/*
==================
debris_vel

sets the correct forward velocity need to fly distxy units forward and
land distz units below current position
==================
*/

float	debris_vel	(userEntity_t *self, float distxy, float z, float up_vel)
{
	float			t, end_z, g, vt;

	if (self->gravity != 0.0)
		g = (p_gravity->value * self->gravity) / 2;
	else
		g = p_gravity->value / 2;
	t = 0.1;

	do
	{
		t = t + 0.1;
		
		// get velocity at time 1
		vt = up_vel - g * 2.0 * t;	
		if (vt < 0.0)
		{
			// get z position at time t
			end_z = self->s.origin [2] + up_vel * t - g * t * t;

			if (end_z <= z) break;
		}
	} while (1);

	return	distxy / t;
};

/*
==========
func_debris_use
==========
*/

void	func_debris_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	debrisHook_t	*hook = (debrisHook_t *) self->userHook;
	CVector			temp, org;
	float			distxy, forward_vel, up_vel;

	self->movetype = MOVETYPE_BOUNCE;
	self->solid = SOLID_BBOX;

	self->s.modelindex = gstate->ModelIndex (hook->modelName);

	//fix up origin
	org = (self->absmin + self->absmax) * 0.5;

	//Time to start checking if we need to stop thinking
	self->delay = gstate->time + 1.5f;

	//Time to change our mins and maxs
	hook->change_time = gstate->time + 0.65f;

	gstate->SetOrigin (self, org.x, org.y, org.z);

	temp = (self->absmax - self->s.origin) * 0.05;
	self->s.maxs = temp;
	
	temp = (self->absmin - self->s.origin) * 0.05;
	self->s.mins = temp;
	
	self->gravity = 1.0f;
	

	hook->last_pos = self->s.origin;
	hook->floor_distance = AI_Determine_Room_Height( self, 2000, 1);

	if (self->spawnflags & GO_TO_ACTIVATOR)// && activator)
		hook->dest = activator->s.origin;
	//func_debris_water_friction (self);

	// if no target, then give random velocities
	if (!self->target && !(self->spawnflags & GO_TO_ACTIVATOR))
	{
		if (!(self->spawnflags & DROP_ONLY))
		{
			//<nss> Let's try this again... how about find a normalized random direction and then find a random velocity??
			temp = CVector(crand(),crand(),crand());
			temp = temp * (rnd()*1000 + 350);
			self->velocity = temp;
		}
		else
		{
			temp = CVector(0.0f,0.0f,-1.0f);
			temp = temp * (rnd()*200 + 100);
			self->velocity = temp;
		}
	}
	else
	{
		// has a target, so find velocity required to reach it
		temp = CVector(hook->dest.x, hook->dest.y, hook->dest.z) - CVector(org.x, org.y, org.z);
		
		//We have the XY distance to the point and are assuming gravity will do the rest of the job(my assumption that this is what the programmer who wrote
		//this was thinking)<nss>
		distxy = temp.Length();
		
		//<nss> Now, if we are in water everything moves around differently, if the Drop flag is not set make it hall ass if so drop to ground.
		if(hook->last_water == CONTENTS_LAVA || hook->last_water == CONTENTS_WATER || hook->last_water == CONTENTS_SLIME)
			up_vel = rnd () * 150 + 150;
		else if(!(self->spawnflags & DROP_ONLY))
			up_vel = (rnd()*300) + 400;		//This might be too much... 
		else
			up_vel = (rnd()*-100) + -100;

		if (self->targetname)
			if (!stricmp (self->targetname, "fanboom"))
				up_vel = 50;
		
		forward_vel = debris_vel (self, distxy, hook->dest.z, up_vel);

		temp = hook->dest - org;
		temp.Normalize ();

		temp = temp * forward_vel;
		temp.z = up_vel;

		self->velocity = temp;
	}

	if (!(self->spawnflags & NO_ROTATE))
	{
		temp = CVector(rnd() * 200, rnd() * 200, rnd() * 200);
		self->avelocity = temp;
		hook->avel = temp;
	}
	
	self->think = func_debris_think;
	self->nextthink = gstate->time + 0.1;
	self->touch = func_debris_touch;

	if (hook->fly_sound)
	{
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->fly_sound), self->s.volume, self->s.dist_min, self->s.dist_max);
	}
}

/*
==========
func_debris_find_target
==========
*/

void	func_debris_find_target (userEntity_t *self)
{
	debrisHook_t	*hook = (debrisHook_t *) self->userHook;
	userEntity_t	*head;

	if (self->target)
	{
		// find target
		head = gstate->FirstEntity ();
		while (head)
		{
			if (head->targetname)
			{
				if (stricmp (self->target, head->targetname) == 0)
				{
					hook->dest = head->s.origin;
				}
			}
			// set up owners so debris won't clip against each other
			else if (head->target && self->targetname)
			{
				if (stricmp (head->target, self->targetname) == 0)
				{
					self->owner = head;
				}
			}

			head = gstate->NextEntity (head);
		}
	}		
}

/*
==========
func_debris
==========
*/

void	func_debris (userEntity_t *self)
{
	debrisHook_t	*hook;
	int				i;
	CVector			org;

	// parse epairs
	self->userHook = gstate->X_Malloc(sizeof(debrisHook_t),MEM_TAG_HOOK);
	hook = (debrisHook_t *) self->userHook;
	hook->damage = -1;
	hook->has_rotated = FALSE;
	hook->mass_scale = 1.2;
	hook->rotation_scale = 0.9;

	// SCG[11/24/99]: Save game stuff
	self->save = debris_hook_save;
	self->load = debris_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "fly_sound") == 0)
		{
			hook->fly_sound = self->epair [i].value;
			if (hook->fly_sound)
				gstate->SoundIndex (hook->fly_sound);
		}
		else if (stricmp (self->epair [i].key, "hit_sound") == 0)
		{
			hook->hit_sound = self->epair [i].value;
			if (hook->hit_sound)
				gstate->SoundIndex (hook->hit_sound);
		}
		else if (stricmp (self->epair [i].key, "damage") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "mass_scale") == 0)
			hook->mass_scale = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "rotation_scale") == 0)
			hook->rotation_scale = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "flag") == 0)
		{
			if (stricmp (self->epair [i].value, "GO_TO_ACTIVATOR") == 0)
				self->spawnflags = self->spawnflags | GO_TO_ACTIVATOR;
			else if (stricmp (self->epair [i].value, "NO_ROTATE") == 0)
				self->spawnflags = self->spawnflags | NO_ROTATE;
			else if (stricmp (self->epair [i].value, "MOMENTUM_DAMAGE") == 0)
				self->spawnflags = self->spawnflags | MOMENTUM_DAMAGE;
			else if (stricmp (self->epair [i].value, "NO_ROTATION_ADJUST") == 0)
				self->spawnflags = self->spawnflags | NO_ROTATION_ADJUST;
			else if (stricmp (self->epair [i].value, "DROP_ONLY") == 0)
				self->spawnflags = self->spawnflags | DROP_ONLY;
			else if (stricmp (self->epair [i].value, "QUARTER_SIZE") == 0)
				self->spawnflags = self->spawnflags | QUARTER_SIZE;
		}
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			self->s.volume = atof(self->epair[i].value);

		i++;
	}

	// set default volume to 60%
	if (!self->s.volume) self->s.volume = 0.60f;
	// set default distances
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;

	if (hook->damage = -1)
	{
		hook->damage = 3;
		self->spawnflags = self->spawnflags | MOMENTUM_DAMAGE;
	}

//	self->spawnflags = self->spawnflags | QUARTER_SIZE;

	if (self->spawnflags & NO_ROTATE)
		hook->has_rotated = TRUE;

	self->movetype = MOVETYPE_NONE;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	hook->modelName = strdup (self->modelName);

	org = (self->absmin + self->absmax) * 0.5;
	hook->last_water = gstate->PointContents( org );
	hook->dest = CVector(0, 0, 0);

	self->use = func_debris_use;

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);
	self->s.modelindex = 0;

	temp_count = 0;
	//<nss> let's try linking this entity!
	gstate->LinkEntity(self);
	self->think = func_debris_find_target;
	self->nextthink = gstate->time + 0.1;
}

///////////////////////////////
//	func_debris_visible
//
//
///////////////////////////////

void	func_debris_visible (userEntity_t *self)
{
	debrisHook_t	*hook;
	int				i;
	CVector			org;

	// parse epairs
	self->userHook = gstate->X_Malloc(sizeof(debrisHook_t), MEM_TAG_HOOK);
	hook = (debrisHook_t *) self->userHook;
	hook->damage = -1;
	hook->has_rotated = FALSE;
	hook->mass_scale = 1.2;
	hook->rotation_scale = 0.9;

	// SCG[11/24/99]: Save game stuff
	self->save = debris_hook_save;
	self->load = debris_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "fly_sound") == 0)
		{
			hook->fly_sound = self->epair [i].value;
			if (hook->fly_sound)
				gstate->SoundIndex (hook->fly_sound);
		}
		else if (stricmp (self->epair [i].key, "hit_sound") == 0)
		{
			hook->hit_sound = self->epair [i].value;
			if (hook->hit_sound)
				gstate->SoundIndex (hook->hit_sound);
		}
		else if (stricmp (self->epair [i].key, "damage") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "mass_scale") == 0)
			hook->mass_scale = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "rotation_scale") == 0)
			hook->rotation_scale = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "flag") == 0)
		{
			if (stricmp (self->epair [i].value, "GO_TO_ACTIVATOR") == 0)
				self->spawnflags = self->spawnflags | GO_TO_ACTIVATOR;
			else if (stricmp (self->epair [i].value, "NO_ROTATE") == 0)
				self->spawnflags = self->spawnflags | NO_ROTATE;
			else if (stricmp (self->epair [i].value, "MOMENTUM_DAMAGE") == 0)
				self->spawnflags = self->spawnflags | MOMENTUM_DAMAGE;
			else if (stricmp (self->epair [i].value, "NO_ROTATION_ADJUST") == 0)
				self->spawnflags = self->spawnflags | NO_ROTATION_ADJUST;
			else if (stricmp (self->epair [i].value, "DROP_ONLY") == 0)
				self->spawnflags = self->spawnflags | DROP_ONLY;
			else if (stricmp (self->epair [i].value, "QUARTER_SIZE") == 0)
				self->spawnflags = self->spawnflags | QUARTER_SIZE;
		}
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			self->s.volume = atof(self->epair[i].value);
		i++;
	}

	// set default volume to full
	if (!self->s.volume) self->s.volume = 1.0f;
	// set default distances
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;

	if (hook->damage = -1)
	{
		hook->damage = 3;
		self->spawnflags = self->spawnflags | MOMENTUM_DAMAGE;
	}

	if (self->spawnflags & NO_ROTATE)
		hook->has_rotated = TRUE;

	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;

	self->use = func_debris_use;
	self->touch = NULL;

	hook->modelName = strdup (self->modelName);

	org = (self->absmin + self->absmax) * 0.5;
	hook->last_water = gstate->PointContents (org);
	hook->dest = CVector(0, 0, 0);

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	self->think = func_debris_find_target;
	self->nextthink = gstate->time + 0.2;
}

/*
==========
info_null
==========
*/

void	info_null (userEntity_t *self)
{
	int		i;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		i++;
	}


	self->className = "info_null";
	self->nextthink = gstate->time + 0.2;
	self->think = self->remove;
}

///////////////////////////////////////////////////////////////////////////////
//	info_not_null
///////////////////////////////////////////////////////////////////////////////

void	info_not_null (userEntity_t *self)
{
	int		i;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		i++;
	}
	
	self->solid			= SOLID_NOT;
	self->className		= "info_not_null";
	self->s.modelindex	= 0;
	self->svflags		&= SVF_NOCLIENT;
	self->use			= NULL;
	self->think			= NULL;
	gstate->LinkEntity (self);
}

///////////////////////////////////////////////////////////////////////////////
//	sound_ambient_toggle
///////////////////////////////////////////////////////////////////////////////

//typedef struct
//{
//	int	soundIndex;
//} ambientHook_t;
//
//void	sound_ambient_toggle (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
//{
//	ambientHook_t	*hook = (ambientHook_t *) self->userHook;
//
//	if (self->s.sound != 0)
//		self->s.sound = 0;
//	else
//		self->s.sound = hook->soundIndex;
//}

///////////////////////////////////////////////////////////////////////////////
//	sound_ambient
///////////////////////////////////////////////////////////////////////////////

//#define	AMBIENT_START_OFF	0x0001
#define	AMBIENT_LOOPED_ON	0x0001

DllExport	void	target_speaker (userEntity_t *self);

void	sound_ambient (userEntity_t *self)
{
	self->spawnflags = AMBIENT_LOOPED_ON;

	self->userHook = gstate->X_Malloc(sizeof(speakerHook_t), MEM_TAG_HOOK);
	speakerHook_t* hook = (speakerHook_t*) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = speaker_hook_save;
	self->load = speaker_hook_load;

	for (int i = 0; self->epair [i].key; i++)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (!stricmp (self->epair [i].key, "volume"))
			hook->volume = atof(self->epair[i].value);
	}

	if( !hook->volume )
	{
		hook->volume = 0.5;
	}

	target_speaker(self);
}


void Deco_Sound(userEntity_t *SoundEnt, unsigned long SpawnFlags)
{
	char buffer[32];
	char sound[64];

	if(SoundEnt)
	{

		//Get root for sounds of explosion debris<nss> and set our alpha level
		if(SpawnFlags & DECO_METAL)
		{
//			sprintf(buffer,"global/e_metalbreaks");
			Com_sprintf(buffer,sizeof(buffer),"global/e_metalbreaks");
		}
		else if (SpawnFlags & DECO_GLASS)
		{
			Com_sprintf(buffer,sizeof(buffer),"global/e_Glassbreaks");
		}
		else//(SpawnFlags & DECO_WOOD)
		{
//			sprintf(buffer,"global/e_woodbreaks");
			Com_sprintf(buffer,sizeof(buffer),"global/e_woodbreaks");
		}
		//else
		//{
		//	sprintf(gib_sound,"global/m_gibslop%c.wav", 97+(int)(rnd()*4.0));
		//	gstate->StartEntitySound(SoundEnt, CHAN_AUTO,gstate->SoundIndex(sound),0.65f, 128.0f, 768.0f);
		//	return;
		//}

//		sprintf(sound,"%s%c.wav",buffer,((int)(rnd()*5))+97);
		Com_sprintf(sound,sizeof(sound),"%s%c.wav",buffer,((int)(rnd()*5))+97);
	
		gstate->StartEntitySound(SoundEnt, CHAN_AUTO,gstate->SoundIndex(sound),0.65f, 256.0f, 768.0f);
	}
}





///////////////////////////////////////////////////////////////////////////////
//NSS[11/28/99]:Added sounds
///////////////////////////////////////////////////////////////////////////////

userEntity_t *deco_debris (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, const CVector &point)
{
	decoHook_t		*hook = (decoHook_t *) self->userHook;
	userEntity_t	*ent;
	CVector			origin, size;
	CVector			chunkorigin, vel, scale, foreard,target_angles;
	int				count;
	int				mass;
	unsigned long	spawnflags;

	size = self->size * 0.5;
	origin = self->s.origin;

	self->takedamage = DAMAGE_NO;

	// start chunks towards the center
	size = size;// * 0.5;

	//NSS[11/28/99]:
	ent = NULL;

	mass = self->mass;
	spawnflags = self->spawnflags;
	com->UseTargets (self, attacker, attacker);

	// if this entity has 'spawnname' defined, try to spawn an entity using 'spawnname' 
	// as the classname
    AI_DoSpawnName( self );

	self->remove (self);

	if (!mass)
		mass = 50;

	// big chunks
	if (mass >= 400)
	{
		count = mass / 100;
		if (count > 7)
			count = 7;

		while(count--)
		{
			scale.x = 1.0 + rnd () * 2.0;
			scale.y = 1.0 + rnd () * 2.0;
			scale.z = 1.0 + rnd () * 2.0;

			chunkorigin.x  = origin.x + (rnd () - 0.5) * size.x;
			chunkorigin.y  = origin.y + (rnd () - 0.5) * size.y;
			chunkorigin.z = origin.z + (rnd () - 0.5) * size.z;

			vel.Set(crand(),crand(),crand());
			
			VectorToAngles(vel,target_angles);

			target_angles.yaw += crand()*20;
			target_angles.pitch += crand()*20;

			target_angles.AngleToForwardVector(forward);
						
			vel = (forward * 250);

			self->velocity = vel;

			if (spawnflags & DECO_WOOD)
				ent = throw_debris (gib_models [GIB_WOOD1], 1, chunkorigin, 0, scale, vel, 0.0);
			else if (spawnflags & DECO_METAL)
				ent = throw_debris (gib_models [GIB_METAL1], 1, chunkorigin, 0, scale, vel, 0.0);
			else if (spawnflags & DECO_GLASS)
				ent = throw_debris (gib_models [GIB_GLASS1], 1, chunkorigin, RF_TRANSLUCENT, scale, vel, 0.0);
			//else if (spawnflags & DECO_GIB)
			//	ent = throw_debris (gib_models [GIB_MISC], 1, chunkorigin, 0, scale, vel, 0.0);
		}
	}

	// small chunks
	count = mass / 30;
	if (count > 16)
		count = 16;
	while(count--)
	{
		scale.x = 0.50 + rnd () * 1.0;
		scale.y = 0.50 + rnd () * 1.0;
		scale.z = 0.50 + rnd () * 1.0;

		chunkorigin.x  = origin.x + (rnd () - 0.5) * size.x;
		chunkorigin.y  = origin.y + (rnd () - 0.5) * size.y;
		chunkorigin.z = origin.z + (rnd () - 0.5) * size.z;

		vel.Set(crand(),crand(),crand());
		
		VectorToAngles(vel,target_angles);

		target_angles.yaw += crand()*20;
		target_angles.pitch += crand()*20;

		target_angles.AngleToForwardVector(forward);
					
		vel = (forward * 350);

		self->velocity = vel;

		if (spawnflags & DECO_WOOD)
			ent = throw_debris (gib_models [GIB_WOOD2], 1, chunkorigin, 0, scale, vel, 0.0);
		else if (spawnflags & DECO_METAL)
			ent = throw_debris (gib_models [GIB_METAL2], 1, chunkorigin, 0, scale, vel, 0.0);
		else if (spawnflags & DECO_GLASS) 
			ent = throw_debris (gib_models [GIB_GLASS2], 1, chunkorigin, RF_TRANSLUCENT, scale, vel, 0.0);
		//else if (spawnflags & DECO_GIB)
		//	ent = throw_debris (gib_models [GIB_MISC], 1, chunkorigin, 0, scale, vel, 0.0);
	}
	return ent;
}

///////////////////////////
//	deco_explode
//
///////////////////////////

void	deco_explode (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, const CVector &point)
{
	
	decoHook_t	*hook = (decoHook_t *) self->userHook;

	self->takedamage = DAMAGE_NO;
	
	if(self->spawnflags & DECO_EXPLODE)
	{
		// blow up
		spawn_sprite_explosion (self, self->s.origin, TRUE);

		//NSS[11/28/99]:Play the dumb explosion sound thingy
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/e_explode1.wav"),0.65f, 128.0f, 768.0f);

		//NSS[11/30/99]:Only do damage if it is of type explosion.
		if (hook->damage)
		{
			com->RadiusDamage (self, self, self, hook->damage, DAMAGE_EXPLOSION, 100.0 );
		}

	}
	
	//NSS[11/28/99]:Added pieces sounds
	unsigned long SpawnFlags = self->spawnflags;
	Deco_Sound(deco_debris (self, inflictor, attacker, 0, point), SpawnFlags);	
}

///////////////////////////////////////////////////////////////////////////////
//	deco_explode_think
//
///////////////////////////////////////////////////////////////////////////////
//userEntity_t *ai_throw_gib( userEntity_t *self, char *model, CVector &offset, byte gib_type, CVector Dir );// SCG[1/23/00]: not used
void	deco_explode_think (userEntity_t *self)
{
	decoHook_t	*hook = (decoHook_t *) self->userHook;
	CVector		org, rand_org;

	hook->explo_count++;

	if (self->spawnflags & DECO_GIB)
	{
		int i,max_gibs;
		max_gibs = self->mass/100;
		if(max_gibs > 6)
			max_gibs = 6;
		ai_gib_sound( self);
		for(i=0;i < max_gibs ; i++)
		{
			ai_throw_gib(self, gib_models[GIB_TORSO+i], CVector(rnd() * 16 - 8, rnd() * 16 - 8, rnd() * 16 - 8),CVector(crand(),crand(),crand()) , GIB_TORSO+i );
		}
		self->think = NULL;
		gstate->RemoveEntity(self);
		return;
	}
	else if (hook->explo_count > hook->explo_max)
	{
		deco_explode (self, hook->inflictor, hook->attacker, hook->damage_done, hook->point);
	}
	else
	{
		self->groundEntity = NULL;
		org = CVector( rnd() * 40 - 20, rnd () * 40 - 20, rnd () * 40 - 20);
		self->velocity = org;

		rand_org.x = self->size[0] * rnd();
		rand_org.y = self->size[1] * rnd();
		rand_org.z = self->size[2] * rnd();


		// small explosions before big boom
		org = self->s.origin + self->s.mins + rand_org;
//		spawn_small_sprite_explosion( self, org, TRUE );
		self->nextthink = gstate->time + THINK_TIME;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	deco_explode_die
//
///////////////////////////////////////////////////////////////////////////////

void	deco_explode_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	decoHook_t	*hook = (decoHook_t *) self->userHook;

	hook->explo_max = rand() & 3;
	hook->attacker = attacker;
	hook->inflictor = inflictor;
	hook->damage_done = damage;
	hook->point = point;

	self->think = deco_explode_think;
	self->nextthink = gstate->time + 0.1;
}

///////////////////////////
//	deco_die
//
//	FIXME: throw gibs depending on type of model
///////////////////////////

void	deco_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	self->takedamage = DAMAGE_NO;

	self->size = self->s.maxs - self->s.mins;
	deco_debris (self, inflictor, attacker, 0, point);

}


/////////////////////
//  deco info class.
//////////////////////

//	Nelno:	commented this out to compile

class deco_info {
public:
    deco_info();

    //the name of the model
    char name[256];
    //the path of the md2 file.
    char path[256];

    //the mass of the object
    float mass;

    //the movement type of the object.
    movetype_t move_type;

    //the solid type of the the object.
    solid_t solid_type;

    //TRUE if we can explode, FALSE otherwise.
    int exploding;

    //the amound of hit points the deco has.
    float hit_points;

    //the size of the object.
    int min[3];
    int max[3];

	// gib types for this deco
	int gib_type;

	// the number of animation sequences for this deco.. default is 0
	int animSequences;

	// the parsed representation of the animation sequences
	ANIMSEQ seq[MAX_DECO_ANIM_SEQS];

    //render fx: 1 = translucent, 0 = default
    int renderFx;

    // render fx alpha value: 0.0f - 1.0f
    float renderFxAlpha; 


    //takes the file pointer and deco name and fills in the data for the deco.
    //returns FALSE if the info for the named deco is not found in the file.
    //file_start is the position in the file that marks the beginning of the data, used
    //when this file is part of a .pak file and the first byte is somewhere in the middle of the
    //file.
    int ReadInfo(char *deco_name, FILE *file, int file_start, int file_len);
};

deco_info::deco_info() {
    //set default parameters.
    strcpy(name, "2shield1");
    strcpy(path, "models/e2/d2_shield1.dkm");

    mass = 1.0f;

    move_type = MOVETYPE_TOSS;
    solid_type = SOLID_BBOX;

    exploding = TRUE;
    hit_points = 20.0f;

	animSequences = 0;
	gib_type = DECO_NO_BREAK;

    renderFx = 0;
    renderFxAlpha = 1.0;

	memset(seq, 0, sizeof(seq));
}

//strips leading and trailing spaces from a string.
void StripSpaces(char *str) {
    if (str == NULL) return;

    //the src and dest positions we read/write to.
    int dest = 0;
    int src = 0;
    
    //the length of the string without the spaces.
    int len = 0;

    //skip past any leading spaces.
    while (str[src] == ' ') {
        src++;
    }

    //copy the string 
    while (str[src] != '\0') {
        //copy the character.
        str[dest] = str[src];

        //increment src and dest.
        dest++;
        src++;

        //increment the length
        len++;
    }

    //null terminate the string.
    str[len] = '\0';

    //remove any spaces at the end of the string.
    while (str[len - 1] == ' ') {
        //get rid of the space.
        str[len - 1] = '\0';
        //decrement the length.
        len--;
    }
}

//parses the hit points out of a string.
float GetHitPoints(char *str);
//parses the exploding flag out of a string.
int	GetExploding(char *str);
//parses the mass out of a string.
float GetMass(char *str);
//parses the solid type out of a string.
solid_t GetSolidType(char *str);
//parses the move type out of a string.
movetype_t GetMoveType(char *str);
//parses a size parameter out of a string.
int GetSize(char *str);
// parses the animation sequences value out of a string
int GetAnimationSequences(char *str);
// parses the gib type value out of a string
int GetGibType(char *str);
//parses the animated frame numbers out of a string.
void ParseAnimationSequences(char *str, LPANIMSEQ lpSeq);


//////////////////////////////////////////////////////////////////////////////////////////////
//  reads through a decoinfo.txt file and finds the data for the deco with the given name.
//////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_DECO_LINE_LEN 255

//#define NUM_DECO_FIELDS 20
#define NUM_DECO_FIELDS 22 // expanded to 22   (1.19 dsn)


int deco_info::ReadInfo(char *deco_name, FILE *file, int file_start, int file_len)
{
    if (file == NULL) return FALSE;
    if (deco_name == NULL) return FALSE;

    //seek to the start of the file
    gstate->FS_Seek(file, file_start, SEEK_SET);

    //the text fields that we will read out of each line of the file
    char fields[NUM_DECO_FIELDS][MAX_DECO_LINE_LEN];

    //the current position we are writing in the line
    int line_pos = 0;

    //the current field we are reading
    int cur_field = 0;

    //reset the first field.
    fields[0][0] = '\0';

    //flag to tell if we are reading through a comment.
    int in_comment = FALSE;

    while (file_len > 0)
    {
        //read in a character.
        char lastchar = gstate->FS_Getc(file);
        //decrement the file_len var.
        file_len--;

        //check if this line is a comment.
        if (line_pos == 0) {
            if (lastchar == ';') {
                in_comment = TRUE;
            }
            else {
                in_comment = FALSE;
            }
        }

        //check if we are reading through a comment.
        if (in_comment == TRUE) 
		{
            line_pos++;
            //check what character we read in last, if it was a \n, we are on the next line.
            if (lastchar == '\n') 
			{
                //we are leaving the comment.
                in_comment = FALSE;

                //reset our line position.
                line_pos = 0;
            }
            //read the next character, dont do any processing on this character.
            continue;
        }

        //put the character in the line.
        fields[cur_field][line_pos] = lastchar;

        //check which character we read.
        //check if we hit the end of the line or file.
        if (lastchar == '\r' || file_len < 1) {
            //the end of the line/file.
            //null terminate the current field.
            fields[cur_field][line_pos] = '\0';

            //reset our line position
            line_pos = 0;

            //null terminate the remainder of the fields.
            for (cur_field++; cur_field < NUM_DECO_FIELDS; cur_field++) {
                fields[cur_field][0] = '\0';
            }

            //read past the \n in the file.
            gstate->FS_Getc(file);
            file_len--;
        }
        //check if we are done with the current field.
        //either we find a comma or the field was too long.
        else if (lastchar == ',' || line_pos >= MAX_DECO_LINE_LEN - 1) {
            //we found the end of the current field.
            //null terminate the field string
            fields[cur_field][line_pos] = '\0';
            //go to the next field.
            cur_field++;
            //reset our line position.
            line_pos = 0;
        }
        else {
            //we didnt end our current field or our current line,
            //advance to the next line position
            line_pos++;
        }

        //check if we have read in all the fields.
        if (cur_field >= NUM_DECO_FIELDS) {
            //got all the fields.  

            //remove spaces from the fields.
            for (int i = 0; i < NUM_DECO_FIELDS; i++) {
                StripSpaces(&fields[i][0]);
            }

            //check the name of the model we read from the file.
            if (stricmp(&fields[0][0], deco_name) == 0) {
                //this is the deco we are looking for.
                //parse the fields.
                //get the deco name.
                strcpy(&name[0], &fields[0][0]);
                //get the md2 file path name.
                strcpy(&path[0], &fields[1][0]);
                //get the move type.
                move_type = GetMoveType(&fields[2][0]);
                //get the solid type
                solid_type = GetSolidType(&fields[3][0]);
                //get the mass.
                mass = GetMass(&fields[4][0]);
                //get our exploding flag.
                exploding = GetExploding(&fields[5][0]);
                //get our hit points
                hit_points = GetHitPoints(&fields[6][0]);
                //get the sizes.
                min[0] = GetSize(&fields[7][0]);
                min[1] = GetSize(&fields[8][0]);
                min[2] = GetSize(&fields[9][0]);
                max[0] = GetSize(&fields[10][0]);
                max[1] = GetSize(&fields[11][0]);
                max[2] = GetSize(&fields[12][0]);
				// get the gib type
				gib_type = GetGibType(&fields[13][0]);
				// get the number of animation sequences.. 0 means only 1 frame
				animSequences = GetAnimationSequences(&fields[14][0]);
				if (animSequences > MAX_DECO_ANIM_SEQS)
					animSequences = MAX_DECO_ANIM_SEQS;
				if (animSequences < 0)
					animSequences = 0;
				// parse the animation sequences
				for (int j = 0; j < animSequences; j++)
				{
					ParseAnimationSequences (&fields[15+j][0], &seq[j]);
				}

                //we're done.
                return TRUE;
            }

            //reset cur_field
            cur_field = 0;

            //reset our line position
            line_pos = 0;

            //read the next character.
            continue;
        }

        //read another character
    }

    //went through the whole file and couldnt find the line.
    return FALSE;
}


//parses the animated frame numbers out of a string.
void ParseAnimationSequences(char *str, LPANIMSEQ lpSeq) 
{
	char delimiters[] = "-\n\r;";
	char *token;

	if ((lpSeq == NULL) || (str == NULL)) return;

	// tilde ~ means to loop from the start frame to the end frame
	if (strchr(str, '~'))
	{
		lpSeq->flags = FRAME_LOOP;
		strcpy(delimiters, "~\n\r;");
	}
	else 
	{
		lpSeq->flags = FRAME_ONCE;
	}
	token = strtok(str, delimiters);
	if (!token)
	{
		lpSeq->start = lpSeq->end = 0;
		return;
	}
	lpSeq->start = atoi(token);

	token = strtok(NULL, delimiters);
	if (!token)
	{
		// only one param in that string
		lpSeq->end = lpSeq->start;
	}
	else
		lpSeq->end = atoi(token);
	
	return;
}

//parses the animated flag out of a string.
int GetAnimationSequences(char *str) {
    if (str == NULL) return FALSE;

    //get the integer value of the string.
    int sequences = atoi(str);

    // clip it to a valid setting
	if (sequences < 0 || sequences > 5)
		sequences = 0;
	
	return (sequences);
}

//parses the gib type out of a string.
int GetGibType(char *str) {
    if (str == NULL) return DECO_NO_BREAK;

    if (stricmp(str, "WOOD") == 0) {
        return DECO_WOOD;
    }
    if (stricmp(str, "METAL") == 0) {
        return DECO_METAL;
    }
    if (stricmp(str, "GLASS") == 0) {
        return DECO_GLASS;
    }
    if (stricmp(str, "GIBS") == 0) {
        return DECO_GIB;
    }

    return DECO_NO_BREAK;
}

//parses a size parameter out of a string.
int GetSize(char *str) {
    if (str == NULL) return -16;

    //get the integer value of the string.
    int val = atoi(str);

    //dont allow a size of zero.
    if (val == 0) {
        return -16;
    }

    return val;
}

//parses the hit points out of a string.
float GetHitPoints(char *str) {
    if (str == NULL) return 20.0f;

    float val = atof(str);

    if (val < 1.0f) {
        val = 20.0f;
    }

    return val;
}

//parses the exploding flag out of a string.
int GetExploding(char *str) {
    if (str == NULL) return TRUE;

    //get the integer value of the string.
    int val = atoi(str);

    //check if it is 0.
    if (val == 0) return FALSE;

    //otherwise, it is exploding.
    return TRUE;
}

//parses the mass out of a string.
float GetMass(char *str) {
    if (str == NULL) return 1.0f;

    return atof(str);
}

//parses the solid type out of a string.
solid_t GetSolidType(char *str) {
    if (str == NULL) return SOLID_BBOX;

    if (stricmp(str, "bbox") == 0) {
        return SOLID_BBOX;
    }

    if (stricmp(str, "not") == 0) {
        return SOLID_NOT;
    }

    return SOLID_BBOX;
}

//parses the move type out of a string.
movetype_t GetMoveType(char *str) {
    if (str == NULL) return MOVETYPE_TOSS;

    if (stricmp(str, "toss") == 0) {
        return MOVETYPE_TOSS;
    }

    if (stricmp(str, "none") == 0) {
        return MOVETYPE_NONE;
    }

    if (stricmp(str, "bounce") == 0) {
        return MOVETYPE_BOUNCE;
    }

    if (stricmp(str, "float") == 0) {
        return MOVETYPE_FLOAT;
    }

    return MOVETYPE_TOSS;
}


#define DEFAULT_OVERRIDE_MASS	-666.0
void entAnimate(userEntity_t *ent,short first,short last,unsigned short frameflags,float frametime);

//generic function that will spawn a deco in any of the episodes.
void SpawnDeco(userEntity_t *self, char *decoinfo_file_name) 
{
    char				*model = NULL;
    float				scale = 1.0f, mass = DEFAULT_OVERRIDE_MASS, speed;
	decoHook_t			*hook;

	self->userHook = gstate->X_Malloc(sizeof(decoHook_t), MEM_TAG_HOOK);
	hook = (decoHook_t *) self->userHook;
	
	// SCG[11/24/99]: Save game stuff
	self->save = deco_hook_save;
	self->load = deco_hook_load;

	hook->damage = 25;
	self->health = 0;
	hook->frame = 0;
	hook->animseq = 0;		//&&& if animation sequences are defined for this deco, automatically
							// run the first squence in the list

    for (int i = 0; self->epair[i].key != NULL; i++)
    {
        if (stricmp (self->epair[i].key, "target") == 0) {
            self->target = self->epair[i].value;
		}
        else if (stricmp (self->epair[i].key, "targetname") == 0) {
            self->targetname = self->epair[i].value;
		}
        else if (stricmp (self->epair[i].key, "killtarget") == 0) {
            self->killtarget = self->epair[i].value;
		}
        else if (stricmp (self->epair[i].key, "model") == 0) {
            model = self->epair[i].value;
        }
        else if (stricmp(self->epair[i].key, "frame") == 0) {
            hook->frame = atoi(self->epair[i].value);
            if (hook->frame < 0) //error check
				hook->frame = 0;
        }
        else if (stricmp(self->epair[i].key, "animseq") == 0) {
            hook->animseq = atoi(self->epair[i].value);
            if (hook->animseq < 0 || hook->animseq > (MAX_DECO_ANIM_SEQS-1))	//error check
				hook->animseq = 0;
        }
        else if (stricmp(self->epair[i].key, "scale") == 0) {
            scale = atof(self->epair[i].value);
            //make sure the scale is a reasonable value.
            if (scale < 0.01f) scale = 1.0f;
        }
        else if (stricmp(self->epair[i].key, "damage") == 0) 
		{
            hook->damage = atof(self->epair[i].value);
        }
        else if (!stricmp(self->epair[i].key, "health")) 
		{
            self->health = atof(self->epair[i].value);
        }
        else if (!stricmp(self->epair[i].key, "mass")) 
		{
			//	this is the mass override key - allows them to override the value in decoinfo.txt
            mass = atof(self->epair[i].value);
        }
		else if (!stricmp(self->epair[i].key, "x_speed")) 
		{
			self->s.effects |= EF_ROTATE;
			//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the x axis
			speed = atof(self->epair[i].value);
			speed = speed / 20;
			if (speed < 0)
			{
				if (speed < -179)
					speed = -179;
				self->s.angle_delta.z = 180 - abs(speed);  // encoded..negatives are 0-179 positives are 180-360
			}
			else
			{
				if (speed > 179)
					speed = 179;
				self->s.angle_delta.z = 180 + speed;
			}

		}
		else 
		if (!stricmp(self->epair[i].key, "y_speed")) 
		{
			self->s.effects |= EF_ROTATE;
			//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the y axis
			speed = atof(self->epair[i].value);
			speed = speed / 20;
			if (speed < 0)
			{
				if (speed < -179)
					speed = -179;
				self->s.angle_delta.x = 180 - abs(speed);	// encoded..negatives are 0-179 positives are 180-360
			}
			else
			{
				if (speed > 179)
					speed = 179;
				self->s.angle_delta.x = 180 + speed;
			}
		}
		else 
		if (!stricmp(self->epair[i].key, "z_speed")) 
		{
			self->s.effects |= EF_ROTATE;
			//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the z axis
			speed = atof(self->epair[i].value);
			speed = speed / 20;
			if (speed < 0)
			{
				if (speed < -179)
					speed = -179;
				self->s.angle_delta.y = 180 - abs(speed);	// encoded..negatives are 0-179 positives are 180-360
			}
			else
			{
				if (speed > 179)
					speed = 179;
				self->s.angle_delta.y = 180 + speed;
			}
		}
        else 
		if (!stricmp(self->epair[i].key, "alpha"))  // 1.19 dsn
		{
          self->s.alpha = atof(self->epair[i].value); // set specified alpha value
          
          // boundary check
          if (self->s.alpha < 0.0f)
            self->s.alpha = 0.0f;
          else
            if (self->s.alpha > 1.0f)
              self->s.alpha = 1.0f;
        }

	}

	if (model == NULL) 
	{
		//there was no model name in the entity
        gstate->Con_Printf ("No model name specified in the deco entity at %s\n", com->vtos(self->s.origin));
		return;
	}

	//open the file with our model 
    int file_len;
    int file_start;
	FILE *file;

    //for now, we cannot use the .pak file code.
    file_len = gstate->FS_Open(decoinfo_file_name, &file);

	//check if we got the file.
	if (file == NULL) 
	{
        gstate->Con_Printf ("[%s] <- deco info file not found!!!\n", decoinfo_file_name);
		return;
	}

    gstate->FS_Seek(file, 0, SEEK_SET);
    //use ftell to get the start of the file
    file_start = gstate->FS_Tell(file);

    //make a deco info struct.
    deco_info info;
    //read the info from the file
    if (info.ReadInfo(model, file, file_start, file_len) == FALSE)
    {
        //couldn't read the deco info from the file.
        gstate->Con_Printf ("couldn't read the [%s] deco info from the deco info file!!\n", model);
        return;
    }

    //close the file
    gstate->FS_Close(file);   

// SCG[8/13/99]: What a waste.  This should have just allocated enough for the string.
// SCG[8/13/99]: It really doesn't need to be done at all since info is a local variable
// SCG[8/13/99]: and info.path can be passed to gstate->ModelIndex() instead of name
/*
    //allocate memory for the name of the object 
    //name = new char[256];
	name = (char*)gstate->X_Malloc(sizeof(char)*256,MEM_TAG_LEVEL);

    //read the data out of the deco_info.
    //get the path of the md2 file.
	strcpy(name, info.path);
*/
	//NSS[11/30/99]:Check the Gib type
	if(!(self->spawnflags & DECO_WOOD) && !(self->spawnflags & DECO_METAL) && !(self->spawnflags & DECO_GLASS) && !(self->spawnflags & DECO_GIB))
		self->spawnflags |= info.gib_type;

	// do we want this deco to rotate? (we check here just in case they want auto-rotate)
	if (self->spawnflags & DECO_ROTATE)
		self->s.effects |= EF_ROTATE;

	if (!self->health)
		self->health = info.hit_points;

	self->die = deco_die;

    if(( (self->spawnflags & (DECO_WOOD | DECO_METAL | DECO_GLASS | DECO_GIB)) || info.exploding == TRUE || self->spawnflags & DECO_EXPLODE)&&!(self->spawnflags & DECO_NO_BREAK))//NSS[11/26/99]:Preset the CSV setting.  If we are supposed to explode set it this way unless flags have been set.
	{
		//	DAMAGE_YES so that damage can be done
		self->takedamage = DAMAGE_YES;
		self->use = NULL;
		self->die = deco_explode_die;
		if (!hook->damage)
			hook->damage = 25;
	}
	
	//check if the object should explode or not
	if( self->spawnflags & DECO_NO_BREAK)
	{
		self->takedamage = DAMAGE_NO;
		self->die = NULL;
		self->use = NULL;
		hook->damage = 0;
	}

    //set the move type
	self->movetype = info.move_type;

    //set the solid type.
    self->solid = info.solid_type;

	self->clipmask = MASK_MONSTERSOLID;
	//self->clipmask = MASK_SOLID;

	//set the mass
	// amw - check for the mass override
	if (mass > DEFAULT_OVERRIDE_MASS)
		self->mass = mass;
	else
		self->mass = info.mass;

	//	must be FL_PUSHABLE or damage won't apply velocity
	self->flags = FL_PUSHABLE;

	//	SVF_MONSTER so shots clip against it
	self->svflags |= SVF_MONSTER;

	//  set iflags to prevent players from bouncing up and down on deco's due
	//  to the prediction bullshit
	// SCG[11/11/99]: WHAT THE FUCK IS THIS!?!?!?!!?!
	// SCG[11/11/99]: THIS FLAG FUCKS THE PREDICTION HARDCORE
	//NSS[11/22/99]:This is "ok" there is another problem elsewhere.
	self->s.iflags |= IF_SV_SENDBBOX;

    //get the model index.
    self->s.modelindex = gstate->ModelIndex(info.path);

    //set the size of the object.
    gstate->SetSize (self, info.min[0], info.min[1], info.min[2], 
                     info.max[0], info.max[1], info.max[2]);

    //set the origin.
    gstate->SetOrigin2(self, self->s.origin);


	// if the deco model has been rotated, rotate the bounding box also
	if (self->s.angles[0] || self->s.angles[1] || self->s.angles[2])
		com->RotateBoundingBox (self);

	// set up the deco's frame animation
	if ((info.animSequences > 0) && (hook->animseq != -1))
	{
		if (hook->animseq > (info.animSequences - 1))
		{
	        gstate->Con_Printf ("animseq specified for deco at %s is invalid!!\n", com->vtos(self->s.origin));
			self->s.frame = 0;
		}
		else
		{
			LPANIMSEQ lpSeq = &info.seq[hook->animseq];
			com->AnimateEntity(self, lpSeq->start, lpSeq->end, lpSeq->flags, 0.10);
		}
	}
	else
	{
		//set the frame
		//self->s.frame = hook->frame;
		entAnimate(self,0,hook->frame,FRAME_ONCE,0.05);
	}

    // check render fx / translucency   1.19 dsn
    if (self->spawnflags & DECO_TRANSLUCENT)
    {
      self->s.renderfx |= RF_TRANSLUCENT;
    }
    else
      self->s.alpha = 1.0f; // default alpha is 1.0f


	// record the deco animation sequences in the decoHook_t record
	size_t bytes = sizeof(info.seq);
	
	memcpy (&hook->seq, &info.seq, bytes);

    //set the scale
    self->s.render_scale.Set( scale, scale, scale );

	self->size = self->s.mins - self->s.maxs;

	// if this has a parent, initialize the parent/child link
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.3;
	}
	
	//NSS[11/22/99]:Let's link the entity at the very end of everything dealing with initialization.
	gstate->LinkEntity( self );
}

///////////////////////////
// deco_e1
///////////////////////////

void deco_e1(userEntity_t *self) {
    SpawnDeco(self, "models/e1/e1decoinfo.csv");
}

///////////////////////////
// deco_e2
///////////////////////////

void deco_e2 (userEntity_t *self) {
    SpawnDeco(self, "models/e2/e2decoinfo.csv");
}

///////////////////////////
// deco_e3
///////////////////////////

void deco_e3(userEntity_t *self) {
    SpawnDeco(self, "models/e3/e3decoinfo.csv");
}

///////////////////////////
//	deco_e4
///////////////////////////

void deco_e4(userEntity_t *self) {
    SpawnDeco(self, "models/e4/e4decoinfo.csv");
}

///////////////////////////
//  crate_pain
//
///////////////////////////
// SCG[1/23/00]: not used
/*
void	crate_pain (userEntity_t *self, userEntity_t *other, float kick, int damage)
{
	if (self->takedamage)
	{
		if (self->hacks - self->health > 10.0)
		{
			self->hacks = self->health;
//			self->s.frame;MAX_DECO_LINE_LEN
		}

		if (self->s.frame >= 8)
		{
			self->takedamage = DAMAGE_NO;
			self->deadflag = DEAD_DEAD;
//			self->s.frame = 8;
			entAnimate(self,8,8,FRAME_ONCE,0.05);
		}
	}
}
*/
///////////////////////////////////////
//  particlefield_XZ
///////////////////////////////////////

#define	USE_COUNT	1

void	particlefield_XZ (userEntity_t *self)
{
	CVector			pos, start, end, temp;
	particleHook_t	*hook = (particleHook_t *) self->userHook;
	userEntity_t	*ent;

	ent = com->ClientVisible (self);
	if (!ent)
		return;

	start = hook->dest1 + self->s.origin;
	end   = hook->dest2 + self->s.origin;

	//	Q2FIXME:	change to Q2 network functions
/*
	gstate->WriteByte (self, MSG_BROADCAST, SVC_TEMPENTITY);
	gstate->WriteByte (self, MSG_BROADCAST, TE_FORCEFIELDXZ);

	gstate->WriteCoord (self, MSG_BROADCAST, start.x);
	gstate->WriteCoord (self, MSG_BROADCAST, start.y);
	gstate->WriteCoord (self, MSG_BROADCAST, start.z);

	gstate->WriteCoord (self, MSG_BROADCAST, end.x);
	gstate->WriteCoord (self, MSG_BROADCAST, end.y);
	gstate->WriteCoord (self, MSG_BROADCAST, end.z);

	gstate->WriteByte (self, MSG_BROADCAST, hook->color);	
*/
/*	
	pos.y = start.y;
	pos.z = start.z;

	while (pos.z <= end.z)
	{
		pos.x = start.x;
		while (pos.x <= end.x)
		{
			temp = CVector (0, 0, 0);
			gstate->particle (pos.vec (), temp.vec (), hook->color, 1);
			pos.x = pos.x + 16;
		}

		pos.z = pos.z + 16;
	}
*/
}

///////////////////////////////////////
//  particlefield_YZ
///////////////////////////////////////

void	particlefield_YZ (userEntity_t *self)
{
	CVector			pos, start, end, temp;
	particleHook_t	*hook = (particleHook_t *) self->userHook;
	userEntity_t	*ent;

	ent = com->ClientVisible (self);
	if (!ent)
		return;
	start = hook->dest1 + self->s.origin;
	end   = hook->dest2 + self->s.origin;

	//	Q2FIXME:	change to Q2 network functions
	/*
	gstate->WriteByte (self, MSG_BROADCAST, SVC_TEMPENTITY);
	gstate->WriteByte (self, MSG_BROADCAST, TE_FORCEFIELDYZ);

	gstate->WriteCoord (self, MSG_BROADCAST, start.x);
	gstate->WriteCoord (self, MSG_BROADCAST, start.y);
	gstate->WriteCoord (self, MSG_BROADCAST, start.z);

	gstate->WriteCoord (self, MSG_BROADCAST, end.x);
	gstate->WriteCoord (self, MSG_BROADCAST, end.y);
	gstate->WriteCoord (self, MSG_BROADCAST, end.z);

	gstate->WriteByte (self, MSG_BROADCAST, hook->color);	
	*/

/*
	pos.x = start.x;
	pos.z = start.z;
   
	while(pos.z < end.z)
	{
		pos.y = start.y;
		while (pos.y < end.y)
		{
			temp = CVector (0, 0, 0);
			gstate->particle (pos.vec (), temp.vec (), hook->color, hook->count);
			pos.y = pos.y + 16;
		}
		
		pos.z = pos.z + 16;
	}
*/
}

///////////////////////////////////////
//  particlefield_XY
///////////////////////////////////////

void	particlefield_XY (userEntity_t *self)
{
	CVector			pos, start, end, temp;
	particleHook_t	*hook = (particleHook_t *) self->userHook;

	// Only show particles if client is visible.
	// This helps to keep network traffic down to a minimum.
	if (!com->ClientVisible (self))
		return;

	start = hook->dest1 + self->s.origin;
	end   = hook->dest2 + self->s.origin;

	//	Q2FIXME:	change to Q2 network functions
	/*
	gstate->WriteByte (self, MSG_BROADCAST, SVC_TEMPENTITY);
	gstate->WriteByte (self, MSG_BROADCAST, TE_FORCEFIELDXY);

	gstate->WriteCoord (self, MSG_BROADCAST, start.x);
	gstate->WriteCoord (self, MSG_BROADCAST, start.y);
	gstate->WriteCoord (self, MSG_BROADCAST, start.z);

	gstate->WriteCoord (self, MSG_BROADCAST, end.x);
	gstate->WriteCoord (self, MSG_BROADCAST, end.y);
	gstate->WriteCoord (self, MSG_BROADCAST, end.z);
	
	gstate->WriteByte (self, MSG_BROADCAST, hook->color);	
	*/
/*
	pos.x = start.x;
	pos.z = start.z;

	while(pos.x < end.x)
	{
		pos.y = start.y;
		
		while (pos.y < end.y)
		{
			temp = CVector (0, 0, 0);
			gstate->particle (pos.vec (), temp.vec (), hook->color, hook->count);
			pos.y = pos.y + 16;
		}
		
		pos.x = pos.x + 16;
	}
*/
}

///////////////////////////////////////
//  particlefield_touch
///////////////////////////////////////

void particlefield_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	particleHook_t	*hook = (particleHook_t *) self->userHook;
	playerHook_t	*ohook;
	CVector			temp;

	if (!(other->flags & FL_CLIENT) && !(other->flags & FL_BOT) &&
		!(other->flags & FL_MONSTER))
	{
		if (other->touch)
		{
			//	blow up projectiles

			//	Q2FIXME:	activator?!?!?
			other->touch (other, self, NULL, NULL);
		}
		else 
		{
			//	remove other stuff
			spawn_sprite_explosion (other, other->s.origin, TRUE);
			other->remove (other);
		}
		
		return;
	}
	else if (other->teleport_time > gstate->time)
		return;
	else
		ohook = AI_GetPlayerHook( other );

	if (!hook->damage)
		return;
	
	if (hook->other == other && hook->hold_time > gstate->time)
	{
		other->velocity.Zero();
		return;
	}

	// not holding anyone yet
	if (!hook->other)
	{
		if (other->flags & FL_DEBUG)
			gstate->bprint ("starting to hold %s\n", other->netname);
		
		hook->other = other;
		hook->last_color = hook->color;

		hook->color.Set( 1, 1, 1 );
		self->use (self, other, other);

		// ignore client's input so they can't run through the field

		//	Q2FIXME:	change to Q2 network functions
		/*
		if (hook->other->flags & FL_CLIENT)
			gstate->WriteByte (other, MSG_ONE, SVC_IGNOREINPUT);
		else if (hook->other->flags & FL_POSSESSED)
			if (ohook->camera->owner)
				gstate->WriteByte (ohook->camera->owner, MSG_ONE, SVC_IGNOREINPUT);
		*/
			
		hook->impact_vel = other->velocity;
		hook->impact_vel.Normalize();

		other->velocity.Zero();

		hook->hold_time = gstate->time + 0.5;
		hook->attack_finished = THINK_TIME;

		other->s.renderfx |= RF_MUZZLEFLASH;

		if (hook->trigger_sound)
			gstate->StartEntitySound(other, CHAN_ITEM, gstate->SoundIndex(hook->trigger_sound), 
                                     1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);

		//////////////////////////////////////////////////////////////
		//	if a monster, stop if from moving forward
		//////////////////////////////////////////////////////////////

		if (other->flags & (FL_BOT + FL_MONSTER))
		{
			self->die = (die_t) other->think;
			other->think = NULL;
		}
	}
	// already holding one person, so just throw this other entity off
	else
	{
		if (other == hook->other)
			return;

		if (other->flags & FL_DEBUG)
			gstate->bprint ("throwing %s\n", other->netname);
		
		temp = other->velocity;
		temp.Normalize ();

		temp = temp * -1 * 800.0;
		other->velocity = temp;
		other->velocity.z = 10.0;
		other->groundEntity = NULL;

		other->s.renderfx |= RF_MUZZLEFLASH;
		other->teleport_time = gstate->time + 0.3;

		if (hook->touch_sound)
			gstate->StartEntitySound(other, CHAN_ITEM, gstate->SoundIndex(hook->touch_sound), 
                                    1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	if (other->flags & FL_CLIENT)
		com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_NONE | DAMAGE_NO_BLOOD);
};

///////////////////////////////////////
//  particlefield_think
///////////////////////////////////////

void particlefield_think (userEntity_t *self)
{
	particleHook_t	*hook = (particleHook_t *) self->userHook;
	playerHook_t	*ohook;
	CVector			temp;
	userEntity_t	*ent;
	int				percent;

	// if hook->other is set then we're hold a victim momentarily
	// on the field
	if (hook->other)
	{
		particleHook_t	*hook = (particleHook_t *) self->userHook;
		
		ohook = AI_GetPlayerHook( hook->other );

		if (hook->other->flags & FL_CLIENT && ohook->shake_time < gstate->time)
			ohook->shake_time = gstate->time + 0.75;

		if (hook->other->flags & FL_CLIENT || hook->other->flags & FL_POSSESSED)
			com->Damage (hook->other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_NONE | DAMAGE_NO_BLOOD);
		
		ent = hook->other;
		percent = (int)(rnd () * 128.0);

		if (hook->other->flags & FL_DEBUG)
			gstate->bprint ("holding %s\n", hook->other->netname);

		// finished holding, throw entity off
		if (hook->hold_time <= gstate->time)
		{
			/////////////////////////////////////////////////////////
			//	throw clients off based on their impact velocity
			//	throw bots/monsters off based on their facing direction
			/////////////////////////////////////////////////////////

			if (hook->other->flags & FL_CLIENT)	
				forward = hook->impact_vel;
			else
			{
				temp = hook->other->s.angles;
				temp.AngleToVectors(forward, right, up);
			}

			temp = hook->other->s.origin + forward * -16.0;
			temp.z = hook->other->s.origin[2];
			gstate->SetOrigin2 (hook->other, temp);

			temp = forward * -150.0;
			hook->other->velocity = temp;

			hook->other->teleport_time = gstate->time + 0.3;
			hook->other->velocity.z = 250.0;
			hook->other->groundEntity = NULL;
	
			hook->other->s.effects |= RF_MUZZLEFLASH;

			if (hook->touch_sound)
				gstate->StartEntitySound(hook->other, CHAN_ITEM, gstate->SoundIndex(hook->touch_sound), 
                                         1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		
//			self->max_speed = hook->attack_finished;
			if (hook->other->flags & FL_CLIENT)
			{
				gstate->StartEntitySound(hook->other, CHAN_VOICE, gstate->SoundIndex("player/playerpain4.wav"), 
                                         1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				//	Q2FIXME:	change to Q2 network functions
				/*
				if (ohook->camera)
					gstate->WriteByte (ohook->camera->owner, MSG_ONE, SVC_RESTOREINPUT);
				else
					gstate->WriteByte (hook->other, MSG_ONE, SVC_RESTOREINPUT);
				*/
			}
			else if (hook->other->flags & FL_POSSESSED)
			{
				//	Q2FIXME:	change to Q2 network functions
				/*
				if (ohook->camera->owner)
					gstate->WriteByte (ohook->camera->owner, MSG_ONE, SVC_RESTOREINPUT);
				*/
			}
			else if (hook->other->flags & (FL_BOT + FL_MONSTER))
			{
				//////////////////////////////////////////////////////////////
				//	if a monster, start it thinking again
				//////////////////////////////////////////////////////////////
				hook->other->nextthink = gstate->time + ohook->think_time;
				hook->other->think = (think_t) self->die;
				self->die = NULL;
			}

			if (hook->other->flags & FL_DEBUG)
				gstate->bprint ("threw %s\n", hook->other->netname);

			hook->other = NULL;
			hook->color = hook->last_color;
			hook->attack_finished = 0.1;
			
			percent = 255;
		}

		if (ent->flags & FL_CLIENT)
		{
			//	Q2FIXME:	change to Q2 network functions
			/*
			gstate->WriteByte (ent, MSG_ONE, SVC_TEMPENTITY);
			gstate->WriteByte (ent, MSG_ONE, TE_COLORFLASH);
			gstate->WriteByte (ent, MSG_ONE, 255);		//red component
			gstate->WriteByte (ent, MSG_ONE, 255);		//green component
			gstate->WriteByte (ent, MSG_ONE, 255);		//blue component
			gstate->WriteByte (ent, MSG_ONE, percent);	//percent
			gstate->WriteByte (ent, MSG_ONE, 1);			//wait
			gstate->WriteLong (ent, MSG_ONE, 128);		//decay
			*/
		}
	}

	//////////////////////////////////////////////////////////////
	//	send the message down to the client if client is visible
	//////////////////////////////////////////////////////////////
	hook->think_use (self);

	self->think = particlefield_think;
	self->nextthink = gstate->time + hook->attack_finished;
}

///////////////////////////////////////
//  particlefield_toggle
///////////////////////////////////////

void	particlefield_toggle (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	particleHook_t	*hook = (particleHook_t *) self->userHook;

	// if no one is on the field and it is active, toggle it off
	if (self->think && hook->other == NULL)
	{
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("ambience/forcefieldoff.wav"), 
                                 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		self->think = NULL;
		self->touch = NULL;
	}
	else if (!self->think)
	{
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("ambience/forcefieldon.wav"), 
                                 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		self->think = particlefield_think;
		self->nextthink = gstate->time + hook->attack_finished;
		self->touch = particlefield_touch;
	}
}

///////////////////////////////////////
//  func_particlefield
//
//	"cnt" is the count to activate on when USE_COUNT is set.
//	"color" is the color of the particles.  Default is 192 (yellow).
//	"count" is the density of the particles.  Default is 2.
//	"noise" is the sound to play when triggered.  Do not use a looping sound here.
//	"dmg" is the amount of damage to cause when touched.
///////////////////////////////////////
/*
void	func_particlefield (userEntity_t *self)
{
	int				i;
	particleHook_t	*hook;
	CVector			temp;

	return;

	self->userHook = gstate->X_Malloc(sizeof(particleHook_t), MEM_TAG_HOOK);
	hook = (particleHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = particle_hook_save;
	self->load = particle_hook_load;

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "color"))
		{
			if (!stricmp (self->epair [i].value, "black"))
				hook->color.Set( 0.0, 0.0, 0.0 );
			else if (!stricmp (self->epair [i].value, "blue"))
				hook->color.Set( 0.0, 0.0, 1.0 );
			else if (!stricmp (self->epair [i].value, "green"))
				hook->color.Set( 0.0, 1.0, 0.0 );
			else if (!stricmp (self->epair [i].value, "cyan"))
				hook->color.Set( 0.0, 1.0, 1.0 );
			else if (!stricmp (self->epair [i].value, "red"))
				hook->color.Set( 1.0, 0.0, 0.0 );
			else if (!stricmp (self->epair [i].value, "purple"))
				hook->color.Set( 0.5, 0.0, 0.5 );
			else if (!stricmp (self->epair [i].value, "brown"))
				hook->color.Set( 0.5, 0.25, 0.0 );
			else if (!stricmp (self->epair [i].value, "ltgray"))
				hook->color.Set( 0.5, 0.5, 0.5 );
			else if (!stricmp (self->epair [i].value, "dkgray"))
				hook->color.Set( 0.25, 0.25, 0.25 );
			else if (!stricmp (self->epair [i].value, "ltblue"))
				hook->color.Set( 0.25, 0.25, 1.0 );
			else if (!stricmp (self->epair [i].value, "ltgreen"))
				hook->color.Set( 0.25, 1.0, 0.25 );
			else if (!stricmp (self->epair [i].value, "ltcyan"))
				hook->color.Set( 0.25, 1.0, 1.0 );
			else if (!stricmp (self->epair [i].value, "ltpurple"))
				hook->color.Set( 1.0, 0.25, 1.0 );
			else if (!stricmp (self->epair [i].value, "yellow"))
				hook->color.Set( 1, 1, 0.25 );
			else if (!stricmp (self->epair [i].value, "white"))
				hook->color.Set( 1.0, 1.0, 1.0 );
		}
		else if (!stricmp (self->epair [i].key, "count"))
			hook->count = atoi (self->epair [i].value);
//		else if (!stricmp (self->epair [i].key, "touch_sound"))
//			hook->touch_sound = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "trigger_sound"))
			hook->trigger_sound = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "ambient_sound"))
			hook->amb_sound = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}

	if (hook->amb_sound)
		gstate->AmbientSound (self->s.origin, hook->amb_sound, 128, ATTN_STATIC, 1.0);

	if (!hook->touch_sound)
		hook->touch_sound = "doors/e1/forcerepel.wav";
	if (!hook->trigger_sound)
		hook->trigger_sound = "doors/e1/forcehold.wav";
	
	gstate->SoundIndex (hook->touch_sound);
	gstate->SoundIndex (hook->trigger_sound);

	gstate->SoundIndex ("ambience/forcefieldoff.wav");
	gstate->SoundIndex ("ambience/forcefieldon.wav");

	if (hook->color == 0)
		hook->color.Set( 0.25, 0.25, 1.0 );
//	if (hook->count == 0)
//		hook->count = 2;
	hook->count = temp_count++;

	self->className = "func_particlefield";
	self->solid = SOLID_TRIGGER;
	gstate->SetModel (self, self->modelName);
	self->movetype = MOVETYPE_NONE;
	self->modelName = NULL;

	temp = (self->s.mins + self->s.maxs) * 0.5;
	self->s.origin = temp;
//	gstate->SetOrigin2 (self, self->s.origin);
	hook->dest = self->s.maxs - self->s.mins - CVector(16, 16, 16);
	hook->dest1 = self->s.mins + CVector(8, 8, 8) - self->s.origin;
	hook->dest2 = self->s.maxs + CVector(7.9, 7.9, 7.9) - self->s.origin;
//	gstate->SetSize (self, self->mins [0], self->mins [1], self->mins [2], self->maxs [0], self->maxs [1], self->maxs [2]);
	self->touch = particlefield_touch;

	if (!hook->damage)
		hook->damage = 2.0;

	if (hook->dest.x > hook->dest.z)
	{
		if (hook->dest.y > hook->dest.z)
		{
			hook->think_use = particlefield_XY;
			hook->dest1.z = (hook->dest1.z + hook->dest2.z ) / 2;
		}
		else
		{
			hook->think_use = particlefield_XZ;
			hook->dest1.y = (hook->dest1.y + hook->dest2.y ) / 2;
		}
	}
	else
	{
		if (hook->dest.y > hook->dest.x)
		{
			hook->think_use = particlefield_YZ;
			hook->dest1.x = (hook->dest1.x + hook->dest2.x ) / 2;
		}
		else
		{
			hook->think_use = particlefield_XZ;
			hook->dest1.y = (hook->dest1.y + hook->dest2.y ) / 2;
		}
	}

	if (hook->trigger_sound)
		gstate->SoundIndex (hook->trigger_sound);

	self->use = particlefield_toggle;

	if (!(self->spawnflags & USE_COUNT))
	{
		self->think = particlefield_think;
		self->nextthink = gstate->time + 0.1;
	}

	hook->attack_finished = 0.1;
}
*/
/////////////////////////////
//	lavaball_glob_think
//
/////////////////////////////

#define	LAVABALL_MAXSCALE	15.0

void	lavaball_glob_think (userEntity_t *self)
{
	if (self->s.render_scale [0] > LAVABALL_MAXSCALE)
		self->delay = 0.92;
	else if (self->s.render_scale [0] < 2.0)
		self->delay = 1.08;

	if (self->s.render_scale [1] > LAVABALL_MAXSCALE)
		self->hacks = 0.95;
	else if (self->s.render_scale [1] < 2.0)
		self->hacks = 1.05;

	if (self->s.render_scale [2] > LAVABALL_MAXSCALE)
		self->team = 0.91;
	else if (self->s.render_scale [2] < 2.0)
		self->team = 1.09;

	self->s.render_scale.x *= self->delay;
	self->s.render_scale.y *= self->hacks;
	self->s.render_scale.z *= self->team;

	gstate->Con_Printf ("%.2f %.2f %.2f\n", self->s.render_scale [0], self->s.render_scale [1], self->s.render_scale [2]);
	gstate->Con_Printf ("%.2f %.2f %.2f\n", self->delay, self->hacks, self->team);

	self->think = lavaball_glob_think;
	self->nextthink = gstate->time + THINK_TIME;
}


///////////////////////////////////////
//	lavaball_shrink
//
///////////////////////////////////////

#define	SPLAT_MAXSCALE	10.0
#define	SPLAT_MINSCALE	1.0

void	lavaball_shrink (userEntity_t *self)
{
	int			pc_top, pc_mid, pc_bottom;
	CVector		org;

	self->velocity.z *= 1.0125;

	if (self->s.render_scale.z > SPLAT_MAXSCALE * 0.75)
	{
		self->delay *= self->hacks;
		self->team = 1.0 - self->delay;
	}
	else 
	if (self->s.render_scale.z < 3.0)
	{
		self->delay *= self->hacks;
		self->team = 1.0 + self->delay;
	}

	self->s.render_scale.z *= self->team;

	//	width is inversly proportional to height
	self->s.render_scale.x = (SPLAT_MAXSCALE * 3.0) / (self->s.render_scale [2]);
	self->s.render_scale.y = (SPLAT_MAXSCALE * 3.0) / (self->s.render_scale [2]);

	gstate->Con_Printf ("shrink: %.2f %.2f %.2f\n", self->s.render_scale [0], self->s.render_scale [1], self->s.render_scale [2]);

	org = self->s.origin + CVector(0, 0, 16.0);
	pc_top = gstate->PointContents( org );

	pc_mid = gstate->PointContents (self->s.origin);

	org = self->s.origin - CVector(0, 0, fabs(self->velocity.z) * 0.05);
	pc_bottom = gstate->PointContents( org );

	if (pc_top == CONTENTS_LAVA && pc_mid == CONTENTS_LAVA && pc_bottom == CONTENTS_LAVA)
		self->remove (self);
}

/////////////////////////////
//	lavaball_velocity_deform_think
//
/////////////////////////////

#define	LAVABALL_DEFORM_MAXSCALE	20.0
#define	LAVABALL_DEFORM_MINSCALE	2.0
#define	LAVABALL_DELTA_MID			15.0

void	lavaball_velocity_deform_think (userEntity_t *self)
{
	CVector		dir, org, speed, deform;
	int			pc, i;
	float		scale_factor;

	org = self->s.origin - CVector(0, 0, fabs(self->velocity.z) * 0.05);
	pc = gstate->PointContents (org);

	if (pc == CONTENTS_LAVA)
	{
		org = self->s.origin;

		for (i = 1; i <= 16; i++)
		{
			org.z -= 1;
			if (gstate->PointContents (org) != CONTENTS_LAVA)
				break;
		}

		org.z -= 4;
		gstate->SetOrigin2 (self, org);
		self->velocity.Zero();

		self->delay = 0.25;
		self->hacks = 0.85;
		self->team = 0.80;

		self->movetype = MOVETYPE_FLY;
		
		self->velocity.Set( 0, 0, -5 );

		lavaball_shrink (self);
		self->prethink = lavaball_shrink;

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//	do a pretend deform based on "air friction" (direction of travel)
	//////////////////////////////////////////////////////////////////////////

	speed = self->velocity;

	//	will reach max elongation at speed of 1000
	scale_factor = 1000 / (LAVABALL_DEFORM_MAXSCALE - LAVABALL_DEFORM_MINSCALE);

	deform.x = fabs (speed.x) / scale_factor + LAVABALL_DEFORM_MINSCALE;
	deform.y = fabs (speed.y) / scale_factor + LAVABALL_DEFORM_MINSCALE;;
	deform.z = fabs (speed.z) / scale_factor + LAVABALL_DEFORM_MINSCALE;;

	self->s.render_scale.Set( fabs(deform.x), fabs(deform.y), fabs(deform.z) );

//	gstate->Con_Printf ("vel_def: %.2f %.2f %.2f\n", self->s.render_scale [0], self->s.render_scale [1], self->s.render_scale [2]);
}

/////////////////////////////
//	lavaball_spawn
//
/////////////////////////////

void	lavaball_spawn (userEntity_t *self)
{
	userEntity_t	*ball;

	ball = gstate->SpawnEntity ();
	
	gstate->SetModel (ball, "models/e3/e_lavaball.dkm");
	gstate->SetSize (ball, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2 (ball, self->s.origin);

	ball->movetype = MOVETYPE_TOSS;
	ball->solid = SOLID_BBOX;
	ball->gravity = 0.75;
	
	ball->prethink = lavaball_velocity_deform_think;
//	ball->nextthink = gstate->time + THINK_TIME;

	ball->s.render_scale.Set(1.0, 1.0, 1.0);

	self->nextthink = gstate->time + 8.0;
	self->think = lavaball_spawn;
}

/////////////////////////////
//	lavaball_spawn_glob
//
/////////////////////////////

void	lavaball_spawn_glob (userEntity_t *self)
{
	userEntity_t	*ball;

	ball = gstate->SpawnEntity ();

	gstate->SetModel (ball, "models/e3/e_lavaball.dkm");
	gstate->SetSize (ball, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2 (ball, self->s.origin);

	ball->movetype = MOVETYPE_NONE;
	ball->solid = SOLID_NOT;
	
	ball->think = lavaball_glob_think;
	ball->nextthink = gstate->time + THINK_TIME;

	ball->delay = 1.08;
	ball->hacks = 1.05;
	ball->team = 1.09;
	
	ball->s.render_scale.Set(2.0, 2.0, 2.0);
}

/////////////////////////////
//	misc_lavaball_drop
//
/////////////////////////////

void	misc_lavaball_drop (userEntity_t *self)
{
	gstate->SetModel (self, "models/e3/e_lavaball.dkm");

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->modelName = NULL;

	self->think = lavaball_spawn;
	self->nextthink = gstate->time + rnd () * 5.0;
}

////////////////////////////////////////////////////////////////////////////
//	lavaball_xy_vel
//
//	sets the correct forward velocity need to go distxy forward and
//	land distz units below current position
/////////////////////////////////////////////////////////////////////////

float	lavaball_xy_vel (userEntity_t *self, float distxy, float distz, float up_vel)
{
	float			t, g, y0, y, root;
	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if (self->gravity != 0.0)
		g = p_gravity->value * self->gravity;
	else
		g = p_gravity->value;

	y0 = self->s.origin [2];
	y = self->s.origin [2] + distz;

	root = y - y0;
	root = (up_vel * up_vel) - (2 * g * (y - y0));

	// negative value means we're trying to jump to a higher ledge!
	if (root < 0)
		return	root;

	t = (up_vel + sqrt (root)) / g;
	
	return	distxy / t;
}

/////////////////////////////
//	lavaball_touch
//
/////////////////////////////

void	lavaball_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->takedamage)
		com->Damage (other, self, self, self->s.origin, zero_vector, self->health, DAMAGE_INERTIAL | DAMAGE_INFLICTOR_VEL);
}

/////////////////////////////
//	lavaball_velocity_deform
//
/////////////////////////////

#define	LAVABALL_VEL_MAXSCALE	10.0
#define	LAVABALL_VEL_MINSCALE	2.0

void	lavaball_velocity_deform (userEntity_t *self)
{
	CVector		dir, ang, speed, deform;
	int			pc;
	float		scale_factor;

	pc = gstate->PointContents (self->s.origin);

	if (pc == CONTENTS_LAVA || gstate->time > self->teleport_time)
	{
		self->remove (self);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//	orient towards direction of travel
	//////////////////////////////////////////////////////////////////////////
/*
	dir = self->velocity;
	dir.Normalize ();
	VectorToAngles( dir, ang );
	//self->s.angles = ang;
	self->s.angles.x = ang.x;
//	self->s.angles.y = ang.y;
	self->s.angles.z = ang.z;
*/
	//////////////////////////////////////////////////////////////////////////
	//	do a pretend deform based on "air friction" (direction of travel)
	//////////////////////////////////////////////////////////////////////////

	speed = self->velocity;

	//	will reach max elongation at speed of 1000
	scale_factor = 500 / (self->delay - self->hacks);

	//	deform x and y proportional to z

	deform.x = fabs (speed.x) / scale_factor + self->hacks;
	deform.y = fabs (speed.y) / scale_factor + self->hacks;
	deform.z = fabs (speed.z) / scale_factor + self->hacks;

	self->s.render_scale.Set( fabs(deform.x), fabs(deform.y), fabs(deform.z) );
}

/////////////////////////////
//	lavaball_random_toss
//
/////////////////////////////

void	lavaball_random_toss (userEntity_t *self)
{
	lavaballHook_t	*hook = (lavaballHook_t *) self->userHook;
	userEntity_t	*ball;

	ball = gstate->SpawnEntity ();
	
	ball->s.modelindex = gstate->ModelIndex ("models/e3/e_lavaball.dkm");
	gstate->SetSize (ball, -8, -8, -8, 8, 8, 8);
//	gstate->SetSize (ball, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2 (ball, self->s.origin);

	ball->movetype = MOVETYPE_BOUNCE;
	ball->solid = SOLID_BBOX;
	ball->gravity = rnd () * 0.5 + 0.25;
	ball->s.effects = 0;//EF_DIMLIGHT + EF_LAVATRAIL;	
	ball->teleport_time = gstate->time + 4.0;
	ball->health = hook->damage;
	ball->mass = 2.0;
	ball->flags = FL_PUSHABLE;

	ball->delay = rnd () * 10.0 + 10.0;
	ball->hacks = rnd () * 2.0 + 2.0;

	ball->prethink = lavaball_velocity_deform;
	if (hook->damage)
		ball->touch = lavaball_touch;

	ball->s.render_scale.Set(1.0, 1.0, 1.0);

	ball->velocity.x = rnd() * 300 - 150.0;
	ball->velocity.y = rnd() * 300 - 150.0;
	ball->velocity.z = rnd() * hook->z_rng + hook->z_min;

	ball->avelocity.x = rnd() * 300 - 150.0;
	ball->avelocity.y = rnd() * 300 - 150.0;
	ball->avelocity.z = rnd() * 300 - 150.0;

	self->nextthink = gstate->time + (rnd() * hook->time_rng) + hook->time_min;
	self->think = lavaball_random_toss;
}

/////////////////////////////
//	lavaball_targetted_toss
//
/////////////////////////////

void	lavaball_targetted_toss (userEntity_t *self)
{
	lavaballHook_t	*hook = (lavaballHook_t *) self->userHook;
	userEntity_t	*ball;
	float			dist, xy_vel, z_vel;
	CVector			dir, org, end;

	ball = gstate->SpawnEntity ();

	ball->s.modelindex = gstate->ModelIndex ("models/e3/e_lavaball.dkm");
	gstate->SetSize (ball, -8, -8, -8, 8, 8, 8);
	gstate->SetOrigin2 (ball, self->s.origin);

	ball->movetype = MOVETYPE_BOUNCE;
	ball->solid = SOLID_BBOX;
	ball->gravity = rnd () * 0.5 + 0.25;
	ball->s.effects = 0;// EF_DIMLIGHT + EF_LAVATRAIL;	
	ball->health = hook->damage;
	ball->mass = 2.0;
	ball->flags = FL_PUSHABLE;

	ball->delay = rnd () * 5.0 + 10.0;
	ball->hacks = rnd () * 1.0 + 2.0;

	ball->s.render_scale.Set(1.0, 1.0, 1.0);

	ball->avelocity.x = rnd() * 300 - 150.0;
	ball->avelocity.y = rnd() * 300 - 150.0;
	ball->avelocity.z = rnd() * 300 - 150.0;

	ball->teleport_time = gstate->time + 4.0;

	ball->prethink = lavaball_velocity_deform;
	if (hook->damage)
		ball->touch = lavaball_touch;

	/////////////////////////////////////////////////////////
	//	compute velocity needed to reach target
	//
	/////////////////////////////////////////////////////////

	org = self->s.origin;
	org.z = 0;
	end = hook->targ_origin;
	end.z = 0;

	dir = end - org;
	dist = dir.Length();
	dir.Normalize();

	z_vel = (rnd() * hook->z_rng + hook->z_min) * ball->gravity / 0.5;

	xy_vel = lavaball_xy_vel( ball, dist, hook->targ_origin[2] - self->s.origin[2], z_vel );
	dir = dir * xy_vel;
	dir.z = z_vel;
	ball->velocity = dir;

	if (!self->targetname)
	{
		self->nextthink = gstate->time + (rnd() * hook->time_rng) + hook->time_min;
		self->think = lavaball_targetted_toss;
	}
}

/////////////////////////////
//	lavaball_findtarget
//
/////////////////////////////

void	lavaball_find_target (userEntity_t *self)
{
	lavaballHook_t	*hook = (lavaballHook_t *) self->userHook;
	userEntity_t	*target;

	target = com->FindTarget (self->target);
	
	if (target)
	{
		hook->targ_origin = target->s.origin;
		
		//	if not targetted, start throwing at random time
		if (!self->targetname)
		{
			self->think = lavaball_targetted_toss;
			self->nextthink = gstate->time + hook->time_min + rnd () * hook->time_rng;
		}
	}
}

/////////////////////////////
//	misc_lavaball_toss
//
/////////////////////////////

void	misc_lavaball_toss (userEntity_t *self)
{
	lavaballHook_t	*hook;
	int				i;
	float			max_time = 12.0, min_time = 4.0;
	float			max_up = 400.0, min_up = 400.0;

	self->userHook = gstate->X_Malloc(sizeof(lavaballHook_t), MEM_TAG_HOOK);
	hook = (lavaballHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = lavaball_hook_save;
	self->load = lavaball_hook_load;

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "mintime"))
			min_time = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "maxtime"))
			max_time = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "damage"))
			hook->damage = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "upmax"))
			max_up = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "upmin"))
			min_up = atof (self->epair [i].value);
	}

	if (!hook->damage)
		hook->damage = 15.0;

	/////////////////////////////////////////////////////////
	//	toss timing
	/////////////////////////////////////////////////////////

	if (max_time >= min_time)
	{
		hook->time_rng = max_time - min_time;
		hook->time_min = min_time;
	}
	else
	{
		hook->time_rng = 8.0;
		hook->time_min = 4.0;
	}

	/////////////////////////////////////////////////////////
	//	z velocity
	/////////////////////////////////////////////////////////

	if (max_up >= min_up)
	{
		hook->z_rng = max_up - min_up;
		hook->z_min = min_up;
	}
	else
	{
		hook->z_rng = 0.0;
		hook->z_min = 400.0;
	}

	gstate->ModelIndex ("models/e3/e_lavaball.dkm");

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->modelName = NULL;

	if (!self->target)
	{
		self->think = lavaball_random_toss;
		self->nextthink = gstate->time + hook->time_min + rnd () * hook->time_rng;
	}
	else
	{
		self->think = lavaball_find_target;
		self->nextthink = gstate->time + 0.1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
//	Any brush that you want to explode or break apart.  If you want an
//	explosion, set dmg and it will do a radius explosion of that amount
//	at the center of the bursh.
//
//	If targeted it will not be shootable.
//
//	health defaults to 100.
//
//	mass defaults to 75.  This determines how much debris is emitted when
//	it explodes.  You get one large chunk per 100 of mass (up to 8) and
//	one small chunk per 25 of mass (up to 16).  So 800 gives the most.
///////////////////////////////////////////////////////////////////////////////

void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, const CVector &point)
{
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	debris_fall (userEntity_t *self)
{
	CVector	vel;

	self->movetype = MOVETYPE_BOUNCE;

	self->think = self->remove;
	self->nextthink = gstate->time + 5 + rnd () * 5;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

userEntity_t *throw_debris (char *modelname, float speed, CVector &origin, 
			int renderfx, CVector &scale, CVector &velocity, float delay)
{
	userEntity_t	*chunk;
	CVector			vel;

	chunk = gstate->SpawnEntity ();
	gstate->SetOrigin2 (chunk, origin);
	chunk->s.modelindex = gstate->ModelIndex (modelname);

	chunk->s.renderfx = renderfx;
	if (renderfx & RF_TRANSLUCENT)
		chunk->s.alpha = 0.75;

	chunk->solid = SOLID_NOT;
	chunk->clipmask = MASK_SOLID;

	vel.x = 100 * (rnd () - 0.5);
	vel.y = 100 * (rnd () - 0.5);
	vel.z = 100 + 100 * (rnd () - 0.5);

	vel = velocity + vel * speed;
	chunk->velocity = vel;

	chunk->avelocity.x = rnd() * 600.0f;
	chunk->avelocity.y = rnd() * 600.0f;
	chunk->avelocity.z = rnd() * 600.0f;

	if (delay >= 0.1)
	{
		chunk->movetype = MOVETYPE_NONE;

		chunk->max_speed = speed;

		chunk->think = debris_fall;
		chunk->nextthink = gstate->time + delay;
	}
	else
	{
		chunk->movetype = MOVETYPE_BOUNCE;

		chunk->think = chunk->remove;
		chunk->nextthink = gstate->time + 5 + rnd () * 5 + delay;
	}
	
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->className = "debris";
//	chunk->takedamage = DAMAGE_YES;
//	chunk->die = debris_die;

	chunk->s.render_scale = scale;

	gstate->LinkEntity (chunk);

	return	chunk;
}



///////////////////////////////////////////////////////////////////////////////
//NSS[11/14/99]:Maek sure we fade out faster when we need more debris
///////////////////////////////////////////////////////////////////////////////

void	debris_fall2(userEntity_t *self)
{
	CVector	vel;
	float Fade = 1.0f;
	if(	func_explosive_count > (MAX_DEBRIS * .90))
		Fade = 5.0f;

	self->s.alpha -= 0.05*Fade;

	if(self->s.alpha <= 0.01)
	{
		func_explosive_count--;
		self->remove(self);
	}
	self->nextthink = gstate->time + 0.05f;
}

void	debris_StartFall(userEntity_t *self)
{
	if(self->delay + 0.55f > gstate->time)
	{
		self->s.angles.roll		+= self->mass+(rnd()*10);
		self->s.angles.pitch	+= self->mass+(rnd()*10);
	}
	if(self->delay+5.0f < gstate->time)
	{
		self->think = debris_fall2;
	}
	self->nextthink = gstate->time + 0.01f;
}
// ----------------------------------------------------------------------------
// <nss>
// Name:throw_debris2
// Description:This actually spawns the debris
// Input:char *modelname, float speed, CVector &origin, int renderfx, CVector &scale, CVector &velocity, float delay, float Alpha, float gravity
// Output:NA
// Note:
// ----------------------------------------------------------------------------

userEntity_t *throw_debris2(char *modelname, float speed, CVector &origin, int renderfx, CVector &scale, CVector &velocity, float delay, float Alpha, float gravity)
{
	userEntity_t	*chunk;
	CVector			vel;

	if(	func_explosive_count >= MAX_DEBRIS)
		return NULL;
	
	
	func_explosive_count++;
	chunk = gstate->SpawnEntity ();
	gstate->SetOrigin2 (chunk, origin);
	chunk->s.modelindex = gstate->ModelIndex (modelname);

	chunk->gravity = gravity;
	
	chunk->s.renderfx = renderfx;

	chunk->s.alpha = Alpha;

	chunk->solid = SOLID_NOT;
	chunk->clipmask = MASK_SOLID;
	chunk->takedamage = DAMAGE_NO;

	vel = velocity * speed;
	chunk->velocity = vel;

	chunk->s.angles.roll = 180*rnd();
	chunk->max_speed = speed;

	chunk->movetype = MOVETYPE_BOUNCE;

	chunk->mass		= 25* ((scale.x+scale.y+scale.z)*0.33f);
	chunk->think = debris_StartFall;
	chunk->nextthink = gstate->time + delay;
	chunk->delay = gstate->time;
	
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->className = "debris";

	chunk->s.render_scale = scale;
	//chunk->mass = (rnd() * 30.0f)+5;

	gstate->LinkEntity (chunk);

	return	chunk;
}



// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive_get_model_enum
// Description:Determine what model we should use
// Input:userEntity_t * self,CVector Size
// Output:NA
// Note:
// ----------------------------------------------------------------------------
int func_explosive_get_model_enum(userEntity_t * self,CVector Size)
{
	funcExploHook_t	*hook = (funcExploHook_t *) self->userHook;
	
	float Stot = VectorDistance(Size,Size * 2);
	//Rotate through different models
	if(rnd()>0.50)
	{
		if(self->spawnflags & EXPL_WOOD)		//wood
		{
			return GIB_WOOD1;
		}
		else if(self->spawnflags & EXPL_METAL)	//metal
		{
			return GIB_METAL1;
		}
		else if(self->spawnflags & EXPL_STONE)	//stone
		{
			return GIB_ROCK1;
		}
		else									//<default> glass
		{
			return GIB_GLASS1;
		}
	}
	else	//We are small use the other model
	{
		if(self->spawnflags & EXPL_WOOD)		//wood
		{
			return GIB_WOOD2;
		}
		else if(self->spawnflags & EXPL_METAL)	//metal
		{
			return GIB_METAL2;
		}
		else if(self->spawnflags & EXPL_STONE)	//stone
		{
			return GIB_ROCK2;
		}
		else									//<default> glass
		{
			return GIB_GLASS2;
		}
		
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive_afterthought
// Description:We are really dead... wait for a bit then die.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void func_explosive_afterthought(userEntity_t *self)
{
	funcExploHook_t	*hook = (funcExploHook_t *) self->userHook;
	float Damage = hook->damage;
	if ( Damage )
	{
			com->RadiusDamage (self, self, self, Damage, DAMAGE_EXPLOSION, 128.0 );
	}
	//We are done with ourself... let's phuk ourselves up.
	gstate->RemoveEntity(self);//self->remove (self);
}

// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive_explode
// Description:The main debris throwing, explosion making, ass kicking, function
// Input:userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void func_explosive_explode (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	funcExploHook_t	*hook = (funcExploHook_t *) self->userHook;
	CVector			origin, size;
	CVector			chunkorigin, vel, scale, target_angles,forward,right,up;
	int				count,Model;
	int				PASSED = 0;
	float			delay,RN;
	userEntity_t	*soundEnt = NULL;

	//Hmmmmm interesting.
	size = self->size * 0.5;
	
	origin = size + self->absmin;

	//First kill the killtarget.
	com->UseTargets (self, attacker, attacker);
	
	//Now deal with the nexttarget
/*	if(self->target != NULL)
	{
		userEntity_t *ent = findEntityFromName(self->target);
		if(ent != NULL)
		{
			if(ent->use != NULL)
				ent->use(ent, self, self);
		}
		else
		{
			hook->target = NULL;
		}
	}*/

	//Get random count of debris chunks
	count = hook->count + (rnd()*hook->rndcount) ;

	//Get number of debris chunks
	if (count > 20)
		count = 20;
	else if (count < (hook->count * 0.5f))
		count = hook->count * 0.5f;
	self->s.origin = origin;


	if(!(self->spawnflags & EXPL_NO_CHUNKS))
	{
		
		for(int i=0;i<count;i++)
		{
			
			RN = rnd();
			if(RN < 0.10f)
			{
				RN = 0.10f;
			}
			//Generate random scale
			scale.x = (hook->scale * RN);
			scale.y = (hook->scale * RN);
			scale.z = (hook->scale * RN);
			
			if(hook->target)
			{
				
				//Need to setup random spread towards the object
				vel = hook->target->s.origin - origin;
				
				vel.Normalize();
				VectorToAngles(vel,target_angles);

				target_angles.yaw += crand()*50;
				target_angles.pitch += crand()*15;

				target_angles.AngleToForwardVector(forward);
				
				vel = (forward * 350)*hook->speed;
				chunkorigin.x	= origin.x + (RN * 0.2) * size.x;
				chunkorigin.y	= origin.y + (RN * 0.2) * size.y;
				chunkorigin.z	= origin.z + (RN * 0.2) * size.z;
			}
			else
			{
				//randomly setup directions for debris.
				vel.Set(crand(),crand(),1.0);
				vel = (vel * 350)*hook->speed;
				chunkorigin.x	= origin.x + (RN * 0.6) * size.x;
				chunkorigin.y	= origin.y + (RN * 0.6) * size.y;
				chunkorigin.z	= origin.z + (RN * 0.6) * size.z;				
			}
			delay = self->delay * rnd();
			if (delay < 0.1) 
				delay = 0.0;

			Model = func_explosive_get_model_enum(self,scale);
			if(PASSED == 1)
			{
				throw_debris2(gib_models [Model], 1, chunkorigin, RF_TRANSLUCENT, scale, vel, delay,hook->alpha, hook->gravity);
			}
			else
			{
				soundEnt = throw_debris2(gib_models [Model], 1, chunkorigin, RF_TRANSLUCENT, scale, vel, delay,hook->alpha, hook->gravity);
				PASSED = 1; //NSS[10/24/99]:Let's get the first object created
			}
			
			
		}
	}

	
	//NSS[10/24/99]:I put all of this shit down here for a VERY specific reason
	//There was a very strange recursive bug occuring with the radius damage.
	//If you change this make sure you talk with me first about this.

	//NSS[10/24/99]:At this point we should not be able to take any damage!
	self->takedamage = 0;
	
	int SpawnFlags,Damage;
	float Volume,vMin,vMax;
	Damage	= hook->damage;
	Volume	= self->s.volume;
	vMin	= self->s.dist_min;
	vMax	= self->s.dist_max;
	SpawnFlags = self->spawnflags;

	if(!(SpawnFlags & EXPL_NO_EXPLOSION))
	{
		if(soundEnt)
		{
			gstate->StartEntitySound(soundEnt, CHAN_AUTO, gstate->SoundIndex ("global/e_explodeb.wav"), Volume, vMin, vMax);
		}
		spawn_sprite_explosion( self, origin, true );
	}
	
	//This is the sound of the pieces falling all around.
	if(!(SpawnFlags & EXPL_NO_SOUND)&&!(self->spawnflags & EXPL_NO_CHUNKS))
	{
		int RaN;
		if(SpawnFlags & EXPL_STONE && soundEnt)
		{
			RaN = (int)(rnd()*4);
			gstate->StartEntitySound(soundEnt, CHAN_AUTO, hook->Sounds[RaN], Volume, vMin, vMax);
		}
		else if(soundEnt)
		{
			gstate->StartEntitySound(soundEnt, CHAN_AUTO, hook->Sounds[(int)(rnd()*5)], Volume, vMin,  vMax);
		}
	}	
	//Do dammage if we have dammage to apply
	// fire off and cine or ai scripts attached to this entity
	if( hook->pCineScript!= NULL && hook->pAiScript != NULL )
	{
		com_PlayEntityScript(inflictor, hook->pCineScript, hook->pAiScript);
	}
	self->nextthink = gstate->time + count*0.018;
	self->think = func_explosive_afterthought;
	
		
}

// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive_use
// Description:The default use function for func_explosive
// Input:edict_t *self, edict_t *other, edict_t *activator
// Output:NA
// Note:
// ----------------------------------------------------------------------------

void func_explosive_use (edict_t *self, edict_t *other, edict_t *activator)
{
	//Set this here because we know we are already gonna explode!
	func_explosive_explode (self, self, other, self->health, zero_vector);
}

// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive_spawn
// Description:Called when the use function for the entity is called if EXPL_TRIGGER_SPAWN is set in self->spawnflags
// Input:edict_t *self, edict_t *other, edict_t *activator
// Output:NA
// Note:Got rid of the KillBox(self); call right after self->use=NULL
// ----------------------------------------------------------------------------

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	gstate->LinkEntity (self);
}

// ----------------------------------------------------------------------------
// <nss>
// Name:func_explosive     2.0
// Description:Function to be called for assigned func_explosive 
// Input:userEntity_t *self
// Output:NA
// Note:Reference
/*	{"rem",			"damage - radius explosion of that amount, at center of brush"},
	{"rem",			"health - defaults to 100."},
	{"rem",			"delay	- defaults to 1.0 to randomize the emission of debris chunks. The higher the number the longer it will take some chunks to chunk out."},
	{"rem",			"count   - defaults to 10.  This determines how many chunks are emitted. 10 = maximum of 10 chunks but is random so could be 5-10, no less than 5."},
	{"rem",			"rndcount  - defaults to 0. Random number of debris entities plus the count."},
	{"rem",			"scale  - This is the scale factor of the chunks. 1.0 is current Size."},
	{"rem",			"target - Sets the vector for the initial direction.  It will be random in a 180 degree spread."},
	{"rem",			"speed  - is a scaled value. 1.0 is normal speed."},
	{"rem",			"gravity    - Gravity to apply to debris. Default is 1.0.(normal)"},
	{"rem",			"cinescript - name of the cinematic script to run"},
	{"rem",			"aiscript   - name of the AI script to run"},
	{"rem",			"nexttarget - Determines what gets triggered next."},*/
// ----------------------------------------------------------------------------
void func_explosive (userEntity_t *self)
{
	funcExploHook_t	*hook;
	int				i;
	char buffer[30];
	char sound[30];

	self->userHook = gstate->X_Malloc(sizeof(funcExploHook_t), MEM_TAG_HOOK);
	hook = (funcExploHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = func_explode_hook_save;
	self->load = func_explode_hook_load;

	//setup some default values
	hook->damage		= 0;
	hook->count			= 10;
	hook->scale			= 1.5f;
	hook->personality	= rnd();
	self->health		= 100;
	hook->speed			= 1.0f;
	hook->target		= NULL;
	hook->gravity		= 1.0f;
	hook->rndcount		= 0.0f;
	self->target		= NULL;
	for (i = 0; self->epair [i].key; i++)
	{
		if( !stricmp(self->epair [i].key, "vectortarget"))//find the target and get the vector to the target
		{
			userEntity_t *ent = findEntityFromName(self->epair[i].value);
			if(ent != NULL)
			{
				hook->target = ent;
			}
			else
			{
				hook->target = NULL;
			}
		}
		else if (!stricmp (self->epair [i].key, "target"))		//next target to 
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))	//id of object
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))	//id of object it destroys
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "damage"))		//Amount of dammage to do		
			hook->damage = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "count"))		//Amount of Debris
			hook->count = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "rndcount"))	//Random amount of Debris plus count
			hook->rndcount = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "gravity"))		//Amount of gravity to apply to Debris
			hook->gravity = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "health"))		//Get health
			self->health = atoi (self->epair [i].value);		
		else if (!stricmp (self->epair [i].key, "delay"))		//Delay for object to explode(destroy)
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))	//cinescript to trigger
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))	//aiscript to trigger
			hook->pAiScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "scale"))		//Average scale of debris
			hook->scale = 1.5f * (atof(self->epair [i].value));
		else if (!stricmp (self->epair [i].key, "speed"))		//Velociy scale of debris
			hook->speed = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			self->s.volume = atof(self->epair[i].value);

	}

	// set default volume to full
	if (!self->s.volume) self->s.volume = 1.0f;
	// set default distances
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;

	self->movetype = MOVETYPE_NONE;

	self->mass = 20*hook->scale;

	//dunno why this is fucked but raidus damage is not doing things right
	if(hook->damage > 0 && hook->damage < 90)
	{
		hook->damage = 100.0;
	}
	gstate->SetModel (self, self->modelName);

	if (self->spawnflags & EXPL_TRIGGER_SPAWN)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else if( self->spawnflags & EXPL_TRIGGER_NOT_SOLID )
	{
		self->solid = SOLID_NOT;
		if (self->targetname)
			self->use = func_explosive_use;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & EXPL_ANIM)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & EXPL_ANIMFAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	//link the entity
	gstate->LinkEntity (self);
	
	//Get root for sounds of explosion debris<nss> and set our alpha level
	if(self->spawnflags & EXPL_STONE)
	{
//		sprintf(buffer,"global/e_rocktumble");
		Com_sprintf(buffer,sizeof(buffer),"global/e_rocktumble");
		hook->alpha = 1.0f;
	}
	else if(self->spawnflags & EXPL_METAL)
	{
//		sprintf(buffer,"global/e_metalbreaks");
		Com_sprintf(buffer,sizeof(buffer),"global/e_metalbreaks");
		hook->alpha = 1.0f;
	}
	else if(self->spawnflags & EXPL_WOOD)
	{
//		sprintf(buffer,"global/e_woodbreaks");
		Com_sprintf(buffer,sizeof(buffer),"global/e_woodbreaks");
		hook->alpha = 1.0f;
	}
	else
	{
//		sprintf(buffer,"global/e_Glassbreaks");
		Com_sprintf(buffer,sizeof(buffer),"global/e_Glassbreaks");
		hook->alpha = 0.55f;
	}
	if(self->spawnflags & EXPL_STONE)
	{
		//Setup sounds for explosion
		for(i=0;i<5;i++)
		{
//			sprintf(sound,"%s%d.wav",buffer,i+1);
			Com_sprintf(sound,sizeof(sound),"%s%d.wav",buffer,i+1);
			hook->Sounds[i] = gstate->SoundIndex (sound);
			sound[0] = NULL;
		}	
	}
	else
	{
		//Setup sounds for explosion
		for(i=0;i<5;i++)
		{
//			sprintf(sound,"%s%c.wav",buffer,i+97);
			Com_sprintf(sound,sizeof(sound),"%s%c.wav",buffer,i+97);
			hook->Sounds[i] = gstate->SoundIndex (sound);
			sound[0] = NULL;
		}
	}
}


qboolean IsNT4 ()
{
#if _MSC_VER
	OSVERSIONINFO osVer ;
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx (&osVer);
	return (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT && osVer.dwMajorVersion < 5) ;
#else
	return false;
#endif
}


// lip sync animation data..  first value is the number of frames of data at 10 fps
// SCG[1/23/00]: not used
//char mouthFrame[2][300] = {	{ 32, 0, 4, 2, 1, 4, 7, 2, 1, 1, 4, 4, 5, 5, 0, 0, 5, 5, 4, 4, 2, 0, 0, 4, 2, 0, 1, 4, 4, 0, 2, 2, 0 },
//							{ 34, 0, 5, 3, 1, 4, 2, 1, 7, 0, 5, 2, 4, 2, 6, 3, 0, 0, 0, 2, 0, 1, 2, 5, 1, 2, 4, 7, 4, 2, 6, 3, 0, 0, 0 }};


///////////////////////////////////////////////////////////////////////////////
// func_talking_head_think()
///////////////////////////////////////////////////////////////////////////////

/*
void func_talking_head_think (userEntity_t *self)
{
	CVector last_movedir, dirVec, angles;
	talkingHeadHook_t *hook = (talkingHeadHook_t *) self->userHook;

	// make sure we're pointed towards our target entity
	if (self->enemy)
	{
		last_movedir = self->movedir;

		dirVec = self->enemy->s.origin - self->s.origin;
		dirVec.Normalize ();
		self->movedir = dirVec;

		if (dirVec != last_movedir)
		{
			VectorToAngles( dirVec, angles );
			self->s.angles = angles;

			// zero out pitch
			self->s.angles.x = 0;
		}
	}
	else
	{
		if (self->target)
		{
			// if we've specified a target, set self->enemy so it will track that entity
			self->enemy = com->FindTarget (self->target);
			if (!self->enemy)
				self->target = NULL;
		}
		else if (self->spawnflags & HEAD_TRACK_CLIENT)
		{
			// if we've specified the head to track a client, then set self->enemy to the client entity
			self->enemy = alist_FirstEntity(client_list);
		}

		// update the lips
		if (hook->bPlaying)
		{
//			float curFrame = (gstate->time - hook->startTime) * 10;
			DWORD	curTime = timeGetTime();
			
			DWORD curFrame = (curTime - hook->startTime) / 100;			

//			curFrame = ceil(curFrame);
			
			// this is necessary because of NT delay
			if ((curFrame >= 0) && (curTime > hook->startTime))
			{
				if (curFrame >= hook->frames)
				{
					hook->bPlaying = FALSE;
					hook->frames = 0;
					self->s.frame = 0;
					self->s.frameInfo.startFrame = self->s.frameInfo.endFrame = self->s.frame;
					self->s.frameInfo.frameFlags |= (FRAME_FORCEINDEX| FRAME_FPS30);
					self->s.frameInfo.frameTime = FRAMETIME_FPS30;
				}
				else
				{
					// is this a key frame?  keyframes are > zero
//					if (mouthFrame1[(int)curFrame+1] >= 0)
					{
						self->s.frame = mouthFrame[(hook->bRandom) ? 0 : 1][(int)curFrame+1];
						self->s.frameInfo.startFrame = self->s.frameInfo.endFrame = self->s.frame;
						self->s.frameInfo.frameFlags |= (FRAME_FORCEINDEX| FRAME_FPS30);
						self->s.frameInfo.frameTime = FRAMETIME_FPS30;
						hook->lastIndex = curFrame;
					}
				}
				gstate->Con_Printf ("curFrame: %d  Time: %f  \tFrame: %3.3d  \n", curFrame, (float)((curTime - hook->startTime)/1000), self->s.frame);
			}

		}
	}
	// update the think
	self->nextthink = gstate->time + 0.1;

}

///////////////////////////////////////////////////////////////////////////////
// func_talking_head_use()
///////////////////////////////////////////////////////////////////////////////

void func_talking_head_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	talkingHeadHook_t *hook = (talkingHeadHook_t *) self->userHook;

	// when used, the head will do a specific action
	if (self->spawnflags & HEAD_USE_TOGGLE)
	{
#if 0
		self->s.frame++;
		if (self->s.frame > 7)
			self->s.frame = 0;
		self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
        gstate->Con_Printf ("Frame: %d\n", self->s.frame);
#else
		if (hook->bRandom)
		{
			hook->bRandom = FALSE;
			//gstate->PositionedSound (zero_vector, self, CHAN_OVERRIDE, hook->waveIndex[0], 1.0, ATTN_NORM, 0.0);
			gstate->StreamedSound("cin/e1m3_intro/line1-mp3.wav",1.0);
		}
		else
		{
			hook->bRandom = TRUE;
			//gstate->PositionedSound (zero_vector, self, CHAN_OVERRIDE, hook->waveIndex[1], 1.0, ATTN_NORM, 0.0);
			gstate->StreamedSound("cin/e1m3_intro/line2-mp3.wav",1.0);
		}
		hook->bPlaying = TRUE;

		hook->startTime = timeGetTime();

		hook->frames = mouthFrame[(hook->bRandom) ? 0 : 1][0];
		hook->lastIndex = -1;
        gstate->Con_Printf ("\n*** Talk started: %f ***\n", gstate->time);

#endif

	}
	else	// just toggle the head on and off
	{
		// toggle the entity on and off
		if (self->spawnflags & HEAD_ON)
		{
			self->svflags |= SVF_NOCLIENT;
			self->spawnflags &= ~HEAD_ON;
		}
		else
		{
			self->svflags &= ~SVF_NOCLIENT;
			self->spawnflags |= HEAD_ON;
			// put it in the world
//			gstate->LinkEntity (self);	// is this necessary?
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// func_talking_head
///////////////////////////////////////////////////////////////////////////////


void func_talking_head (userEntity_t *self)
{
	talkingHeadHook_t	*hook;
	char *modelName = NULL;
	int	i = 0;
	
	self->userHook = gstate->X_Malloc(sizeof(talkingHeadHook_t), MEM_TAG_HOOK);
	hook = (talkingHeadHook_t *) self->userHook;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
        else if (!stricmp (self->epair[i].key, "modelname")) 
            modelName = self->epair[i].value;

	}

	// was there a model name specified for the entity?
	if (!modelName)
	{
        gstate->Con_Printf ("No model name specified for the func_talking_head entity at %s\n", com->vtos(self->s.origin));
		self->remove(self);
		return;
	}

	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_BBOX;

	// do we want the head to start on?
	if (self->spawnflags & HEAD_START_ON)
		self->svflags &= ~SVF_NOCLIENT;
	else
		self->svflags |= SVF_NOCLIENT;

	self->clipmask = MASK_PLAYERSOLID;
//    self->flags = FL_PUSHABLE;
	self->svflags |= SVF_MONSTER;
	// SCG[11/11/99]: THIS FLAG FUCKS THE PREDICTION HARDCORE
	self->s.iflags |= IF_SV_SENDBBOX;
	self->takedamage = DAMAGE_NO; //DAMAGE_YES;

	// is the head supposed to be a holograph?
	if (self->spawnflags & HEAD_HOLOGRAPH)
	{
		self->s.alpha = 0.7;
		self->s.renderfx |= RF_TRANSLUCENT;
	}

	// make it bigger HACK
	self->s.render_scale.Set( 2.0, 2.0, 2.0 );

	// set up the model name 
	self->s.modelindex = gstate->ModelIndex (modelName);

	self->s.frame = 0;
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;// | FRAME_FPS30;

//	self->s.frameInfo.startFrame = 0;
//	self->s.frameInfo.endFrame = 0;
//	self->s.frameInfo.frameInc = 1;
//	self->s.frameInfo.frameTime = FRAMETIME_FPS20;

	//	set mins/maxs/size
	self->s.mins.Set(-8, -8, -8);
	self->s.maxs.Set(8, 8, 8);

	// track entities?
	if (self->target)
	{
		// if we've specified a target, set self->enemy so it will track that entity
		self->enemy = com->FindTarget (self->target);
	}
	else if (self->spawnflags & HEAD_TRACK_CLIENT)
	{
		// if we've specified the head to track a client, then set self->enemy to the client entity
		self->enemy = alist_FirstEntity(client_list);
	}

	// set up think functions
	self->use = func_talking_head_use;
	self->think = func_talking_head_think;
	self->nextthink = gstate->time + 0.1;

	// put it in the world
	gstate->LinkEntity (self);

	hook->waveIndex [0] = gstate->SoundIndex ("cin/e1m3_intro/line1.wav");
	hook->waveIndex [1] = gstate->SoundIndex ("cin/e1m3_intro/line2.wav");
}
*/

/*  demo talking head data..

Frame   Timecode   Key   MouthName     Comments
    0  00:00:00.00    X   talk000       
    1  00:00:00.01    X   talk004       
    2  00:00:00.02    X   talk002       
    3  00:00:00.03    X   talk001       
    4  00:00:00.04    X   talk004       
    5  00:00:00.05    X   talk007       
    6  00:00:00.06    X   talk002       
    7  00:00:00.07    X   talk001       
    8  00:00:00.08    X   talk001       
    9  00:00:00.09    X   talk004       
   10  00:00:01.00        talk004       
   11  00:00:01.01    X   talk005       
   12  00:00:01.02        talk005       
   13  00:00:01.03    X   talk000       
   14  00:00:01.04        talk000       
   15  00:00:01.05    X   talk005       
   16  00:00:01.06        talk005       
   17  00:00:01.07    X   talk004       
   18  00:00:01.08        talk004       
   19  00:00:01.09    X   talk002       
   20  00:00:02.00    X   talk000       
   21  00:00:02.01        talk000       
   22  00:00:02.02    X   talk004       
   23  00:00:02.03    X   talk002       
   24  00:00:02.04    X   talk000       
   25  00:00:02.05    X   talk001       
   26  00:00:02.06    X   talk004       
   27  00:00:02.07        talk004       
   28  00:00:02.08    X   talk000       
   29  00:00:02.09    X   talk002       
   30  00:00:03.00        talk002       
   31  00:00:03.01    X   talk000       

*/

void func_event_generator_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );
void func_event_generator_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator );
void func_event_generator_think( userEntity_t *self );
void func_event_generator( userEntity_t *self );

void misc_register_func()
{
	gstate->RegisterFunc("func_explosive_spawn",func_explosive_spawn);
	gstate->RegisterFunc("func_explosive_use",func_explosive_use);
	gstate->RegisterFunc("func_explosive_explode",func_explosive_explode);

	gstate->RegisterFunc("deco_die",deco_die);
	gstate->RegisterFunc("deco_explode_die",deco_explode_die);

	gstate->RegisterFunc("deco_explode_think",deco_explode_think);

	gstate->RegisterFunc("particlefield_toggle",particlefield_toggle);
	gstate->RegisterFunc("particlefield_think",particlefield_think);
	gstate->RegisterFunc("particlefield_touch",particlefield_touch);
	gstate->RegisterFunc("particlefield_XY",particlefield_XY);
	gstate->RegisterFunc("particlefield_XZ",particlefield_XZ);
	gstate->RegisterFunc("particlefield_YZ",particlefield_YZ);

	gstate->RegisterFunc("lavaball_glob_think",lavaball_glob_think);
	gstate->RegisterFunc("lavaball_spawn",lavaball_spawn);
	gstate->RegisterFunc("lavaball_random_toss",lavaball_random_toss);
	gstate->RegisterFunc("lavaball_targetted_toss",lavaball_targetted_toss);
	gstate->RegisterFunc("lavaball_find_target",lavaball_find_target);
	gstate->RegisterFunc("lavaball_touch",lavaball_touch);
	gstate->RegisterFunc("lavaball_shrink",lavaball_shrink);
	gstate->RegisterFunc("lavaball_velocity_deform_think",lavaball_velocity_deform_think);
	gstate->RegisterFunc("lavaball_velocity_deform",lavaball_velocity_deform);

	gstate->RegisterFunc("func_debris_use",func_debris_use);
	gstate->RegisterFunc("func_debris_think",func_debris_think);
	gstate->RegisterFunc("func_debris_find_target",func_debris_find_target);
	gstate->RegisterFunc("debris_fall",debris_fall);
	gstate->RegisterFunc("debris_fall2",debris_fall2);
	gstate->RegisterFunc("debris_die",debris_die);
	gstate->RegisterFunc("debris_StartFall",debris_StartFall);
	gstate->RegisterFunc("func_debris_touch",func_debris_touch);

// SCG[12/5/99]: 	gstate->RegisterFunc("func_talking_head_use",func_talking_head_use);
// SCG[12/5/99]: 	gstate->RegisterFunc("func_talking_head_think",func_talking_head_think);

	gstate->RegisterFunc( "func_event_generator_think", func_event_generator_think );
	gstate->RegisterFunc( "func_event_generator_touch", func_event_generator_touch );
	gstate->RegisterFunc( "func_event_generator_use", func_event_generator_use );
	gstate->RegisterFunc( "func_event_generator", func_event_generator );

	gstate->RegisterFunc( "debris_hook_save", debris_hook_save );
	gstate->RegisterFunc( "debris_hook_load", debris_hook_load );
	gstate->RegisterFunc( "particle_hook_save", particle_hook_save );
	gstate->RegisterFunc( "particle_hook_load", particle_hook_load );
	gstate->RegisterFunc( "lavaball_hook_save", lavaball_hook_save );
	gstate->RegisterFunc( "lavaball_hook_load", lavaball_hook_load );
	gstate->RegisterFunc( "func_explode_hook_save", func_explode_hook_save );
	gstate->RegisterFunc( "func_explode_hook_load", func_explode_hook_load );
	gstate->RegisterFunc( "deco_hook_save", deco_hook_save );
	gstate->RegisterFunc( "deco_hook_load", deco_hook_load );
	gstate->RegisterFunc( "speaker_hook_save", speaker_hook_save );
	gstate->RegisterFunc( "speaker_hook_load", speaker_hook_load );
}
