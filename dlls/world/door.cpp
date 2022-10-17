#include "world.h"
#include "nodelist.h"
#include "GrphPath.h"
//#include "ai_func.h"// SCG[1/23/00]: not used
#include "common.h"

// exports
DllExport	void	func_door(userEntity_t *self);
DllExport	void	func_door_rotate (userEntity_t *self);
DllExport	void	func_plat (userEntity_t *self);
DllExport	void	func_rotate(userEntity_t *self);
DllExport	void	func_door_rotating (userEntity_t *self);
DllExport	void	func_train (userEntity_t *self);
DllExport	void	path_corner_train (userEntity_t *self);
DllExport	void	func_button (userEntity_t *self);
DllExport	void	func_door_secret (userEntity_t *self);
DllExport	void	func_wall (userEntity_t *self);
DllExport	void	func_multi_button (userEntity_t *self);
DllExport	void	func_areaportal (userEntity_t *self);
// SCG[2/13/00]: DllExport	void	func_nodetrain (userEntity_t *self);

/////////////////////////////////////////////////////////////////////
// defines
/////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// prototypes
///////////////////////////////////////////////////////////////////////////////

void door_init (userEntity_t *self);
void door_close (userEntity_t *self);
void door_open (userEntity_t *self);
void door_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);
void plat_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);
void door_continuous_refire (userEntity_t *self);
void door_fully_open( userEntity_t *self );
void door_fully_closed( userEntity_t *self );
void train_find( userEntity_t *self );

void door_fire (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void door_reverse (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void door_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void door_parse_sounds (userEntity_t *self);
void train_next		(userEntity_t *self);
void func_rotate_blocked (userEntity_t *self, userEntity_t *other);
void areaportal_toggle (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void areaportal_set (userEntity_t *self, int state);
// SCG[2/13/00]: void	nodetrain_next (userEntity_t *self);
userEntity_t *SpawnField (userEntity_t *self, CVector &fmins, CVector &fmaxs);

void train_next( userEntity_t *self );
void train_die_next( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point );
void train_find( userEntity_t *self );
void train_wait( userEntity_t *self );
void train_rotate_think (userEntity_t *self);

#define FUNC_DOOR_STRUGGLE			0x00000100
#define FUNC_DOOR_ROTATE_STRUGGLE	0x00000400
#define FUNC_BUTTON_STRUGGLE		0x00000002


// SCG[11/19/99]: Save game stuff
#define	DOORHOOK(x) (int)&(((doorHook_t *)0)->x)
field_t door_hook_fields[] = 
{
	{"state",				DOORHOOK(state),				F_INT},
	{"count",				DOORHOOK(count),				F_INT},
	{"max_count",			DOORHOOK(max_count),			F_INT},
	{"already_linked",		DOORHOOK(already_linked),		F_INT},
	{"continuous_mode",		DOORHOOK(continuous_mode),		F_INT},
	{"wait",				DOORHOOK(wait),					F_FLOAT},
	{"base_health",			DOORHOOK(base_health),			F_FLOAT},
	{"speed",				DOORHOOK(speed),				F_FLOAT},
	{"distance",			DOORHOOK(distance),				F_FLOAT},
	{"move_finished",		DOORHOOK(move_finished),		F_FLOAT},
	{"damage",				DOORHOOK(damage),				F_FLOAT},
	{"ignore_time",			DOORHOOK(ignore_time),			F_FLOAT},
	{"x_speed",				DOORHOOK(x_speed),				F_FLOAT},
	{"y_speed",				DOORHOOK(y_speed),				F_FLOAT},
	{"z_speed",				DOORHOOK(z_speed),				F_FLOAT},
	{"x_distance",			DOORHOOK(x_distance),			F_FLOAT},
	{"y_distance",			DOORHOOK(y_distance),			F_FLOAT},
	{"z_distance",			DOORHOOK(z_distance),			F_FLOAT},
	{"end_x_rotation_time",	DOORHOOK(end_x_rotation_time),	F_FLOAT},
	{"end_y_rotation_time",	DOORHOOK(end_y_rotation_time),	F_FLOAT},
	{"end_z_rotation_time",	DOORHOOK(end_z_rotation_time),	F_FLOAT},
	{"end_x_angle",			DOORHOOK(end_x_angle),			F_FLOAT},
	{"end_y_angle",			DOORHOOK(end_y_angle),			F_FLOAT},
	{"end_z_angle",			DOORHOOK(end_z_angle),			F_FLOAT},
	{"end_move_time",		DOORHOOK(end_move_time),		F_FLOAT},
	{"t_width",				DOORHOOK(t_width),				F_FLOAT},
	{"t_length",			DOORHOOK(t_length),				F_FLOAT},
	{"lip",					DOORHOOK(lip),					F_FLOAT},
	{"pos1",				DOORHOOK(pos1),					F_VECTOR},
	{"pos2",				DOORHOOK(pos2),					F_VECTOR},
	{"final_dest",			DOORHOOK(final_dest),			F_VECTOR},
	{"size",				DOORHOOK(size),					F_VECTOR},
	{"old_origin",			DOORHOOK(old_origin),			F_VECTOR},
	{"mangle",				DOORHOOK(mangle),				F_VECTOR},
	{"pos1_offs",			DOORHOOK(pos1_offs),			F_VECTOR},
	{"pos2_offs",			DOORHOOK(pos2_offs),			F_VECTOR},
	{"destangle",			DOORHOOK(destangle),			F_VECTOR},
	{"velocityInc",			DOORHOOK(velocityInc),			F_VECTOR},
	{"done_think",			DOORHOOK(done_think),			F_FUNC},
	{"linked_door",			DOORHOOK(linked_door),			F_EDICT},
	{"trigger_field",		DOORHOOK(trigger_field),		F_EDICT},
	{"activator",			DOORHOOK(activator),			F_EDICT},
	{"portal",				DOORHOOK(portal),				F_EDICT},
	{"nodetarget",			DOORHOOK(nodetarget),			F_LSTRING},
	{"pAiScript",			DOORHOOK(pAiScript),			F_LSTRING},
	{"pCineScript",			DOORHOOK(pCineScript),			F_LSTRING},
	{"sound_top",			DOORHOOK(sound_top),			F_INT},
	{"sound_bottom",		DOORHOOK(sound_bottom),			F_INT},
	{"sound_up",			DOORHOOK(sound_up),				F_INT},
	{"sound_down",			DOORHOOK(sound_down),			F_INT},
	{"bitflags",			DOORHOOK(bitflags),				F_INT},
	{"steps",				DOORHOOK(steps),				F_INT},
	{"bounceSteps",			DOORHOOK(bounceSteps),			F_INT},
	{"no_more_move",		DOORHOOK(no_more_move),			F_INT},
	{"bStruggle",			DOORHOOK(bStruggle),			F_INT},
	{"misctarget",			DOORHOOK(misctarget),			F_LSTRING},
	{NULL, 0, F_INT}
};

void areaportal_save( FILE *f, edict_t *ent )
{
}

void areaportal_load( FILE *f, edict_t *ent )
{
	gstate->SetAreaPortalState( ent->hacks_int, ent->light_level );
}


#define TRAIN_STATE_NOT_MOVING	0
#define TRAIN_STATE_MOVING		1

void func_train_init_from_save( userEntity_t *self )
{
	userEntity_t	*pTarget;
	doorHook_t		*pHook = ( doorHook_t * ) self->userHook;
	char			*szTarget;

	if( ( self->goalentity != NULL ) && ( self->goalentity->targetname != NULL ) )
	{
		szTarget = self->goalentity->targetname;
	}
	else if( self->target != NULL )
	{
		szTarget = self->target;
	}
	else if( self->target == NULL )
	{
		return;
	}

// SCG[6/23/00]: not needed anymore
/*
	if( pHook->state == TRAIN_STATE_MOVING )
	{
		// SCG[3/6/00]: We need to get the movedir and back the object off a bit
		// SCG[3/6/00]: Get the movedir
		CVector vOffset = self->velocity;
		// SCG[3/6/00]: Get the average speed ( taken from length )
		float	fAverageSpeed = vOffset.Length();
		// SCG[3/6/00]: rate is the number of units moved in one frame assuming the entity frame time is one tenth of a second ( normal server rate )
		float fUnitsPerFrame = fAverageSpeed * 0.1;
		// SCG[3/6/00]: normalize the direction.
		vOffset.Normalize();
		// SCG[3/6/00]: subtract 30 frames worth of distance
		vOffset.Multiply( fUnitsPerFrame * 20 );
		// SCG[3/6/00]: Subtract it from the origin
		self->s.origin -= vOffset;
		// SCG[3/6/00]: link entity
		gstate->LinkEntity( self );
	}
	else
	{
		self->think = NULL;
		self->nextthink = -1;
		return;
	}
*/
// SCG[6/23/00]: use this instead
	if( pHook->state != TRAIN_STATE_MOVING )
	{
		self->think = NULL;
		self->nextthink = -1;
		return;
	}

	pTarget = gstate->FirstEntity();

	while( pTarget != NULL )
	{
		if( pTarget->targetname != NULL )
		{
			if( stricmp( szTarget, pTarget->targetname ) == 0 )
			{
				if( pTarget->className )
				{
					if( stricmp( pTarget->className, "path_corner_train" ) == 0 )
					{
						break;
					}
				}
			}
		}

		pTarget = gstate->NextEntity( pTarget );
	}

	if( pTarget == NULL )
	{
		gstate->Con_Printf( "func_train at %s has no first target\n", com->vtos( self->s.origin ) );
		self->remove( self );
	}

	if( pTarget != NULL )
	{
		if( self->goalentity->spawnflags & TELEPORT )
		{
			trace_t	tr;

			pHook->done_think = train_wait;
			pHook->final_dest = pTarget->s.origin;
			self->s.event = EV_ENTITY_FORCEMOVE;
			com_CalcAbsolutePosition( self );
			tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_PLAYERSOLID );
			if( tr.allsolid && tr.startsolid && tr.ent && tr.ent->solid != SOLID_BSP && tr.ent != &gstate->g_edicts[0] )
			{
				com->Damage( tr.ent, self, self, self->s.origin, zero_vector, tr.ent->health + 100, DAMAGE_TELEFRAG );
			}
		}
		else
		{
			com_CalcMove( self, pTarget->s.origin, pHook->speed, train_wait );
		}
	}
}

void door_init_from_save( userEntity_t *self )
{
	doorHook_t	*pHook = ( doorHook_t * ) self->userHook;

	if( strcmp( self->className, "func_door_rotate" ) == 0 )
	{
		if( self->spawnflags & ROTATE_AUTO_OPEN )
		{
			pHook->trigger_field = SpawnField( self, self->absmin, self->absmax );
		}

		if( pHook->state == STATE_UP )
		{
			CVector doorDir;
			doorDir = pHook->pos2;

			if( self->spawnflags & ROTATE_SWING )
			{
				float dotProduct;
				CVector rightDir, doorVector;
				CVector f, u;
				
				doorVector = self->s.origin - pHook->activator->s.origin;
				AngleToVectors( pHook->activator->s.angles , f, rightDir, u );
				
				dotProduct = DotProduct( doorVector, rightDir );

				// if it's positive, we need to reverse the door's path
				if (dotProduct > 0)
				{
					doorDir.Negate( pHook->pos2 );
				}
			}

			com_CalcAngleMove( self, doorDir, pHook->speed, door_fully_open );
		}
		else if( pHook->state == STATE_DOWN )
		{
			com_CalcAngleMove( self, pHook->pos1, pHook->speed, door_fully_closed );
		}
		else if( self->spawnflags & DOOR_TOGGLE )
		{
			self->nextthink = -1;
		}
		else if( !( self->spawnflags & DOOR_START_OPEN ) && pHook->wait >= 0 )
		{
			self->think = door_close;
			self->nextthink = gstate->time + pHook->wait;
		}
	}
	else if( strcmp( self->className, "func_door" ) == 0 )
	{
		if( self->spawnflags & DOOR_AUTO_OPEN )
		{
			pHook->trigger_field = SpawnField( self, self->absmin, self->absmax );
			self->think = NULL;
			self->nextthink = -1;
		}

		if( pHook->state == STATE_UP )
		{
			if (self->teammaster)
			{
				CVector new_org;
				CVector adjOrigin;  // in case the door has collided or something
				adjOrigin = self->teammaster->s.origin + self->transformedOffset;

				pHook->pos1 = adjOrigin;

				com->TransformChildOffset( adjOrigin, pHook->pos2_offs, self->teammaster->s.angles, TRUE, new_org );
				pHook->pos2 = new_org;
			}

			com_CalcMove (self, pHook->pos2, pHook->speed, door_fully_open );
		}
		else if( pHook->state == STATE_DOWN )
		{
			com_CalcMove( self, pHook->pos1, pHook->speed, door_fully_closed );
		}
		else if( pHook->state == STATE_TOP )
		{
			if( ( pHook->continuous_mode == CONTINUOUS_TRIGGERED ) || ( pHook->continuous_mode == CONTINUOUS_ON ) )
			{
				self->think = door_continuous_refire;
				if( pHook->wait > 0 )
				{
					self->nextthink = gstate->time + pHook->wait;
				}
				else
				{
					self->nextthink = gstate->time + 0.1;
				}
			}
			else
			{
				if( pHook->done_think != NULL )
				{
					self->think = pHook->done_think;
				}
				if( pHook->wait > 0 )
				{
					self->nextthink = gstate->time + pHook->wait;
				}
				else 
				{
					self->nextthink = gstate->time + 0.1;
				}
			}
		}
		else if( pHook->state == STATE_BOTTOM )
		{
			if( ( pHook->continuous_mode == CONTINUOUS_TRIGGERED ) || ( pHook->continuous_mode == CONTINUOUS_ON ) )
			{
				self->think = door_continuous_refire;
				if( pHook->wait > 0 )
				{
					self->nextthink = gstate->time + pHook->wait;
				}
				else
				{
					self->nextthink = gstate->time + 0.1;
				}
			}
			else
			{
				if( pHook->done_think != NULL )
				{
					self->think = pHook->done_think;
				}
				if( pHook->wait > 0 )
				{
					self->nextthink = gstate->time + pHook->wait;
				}
				else 
				{
					self->nextthink = gstate->time + 0.1;
				}
			}
		}

		if( ( pHook->state != STATE_BOTTOM ) && ( pHook->state != STATE_TOP ) )
		{
			if( self->spawnflags & DOOR_TOGGLE )
			{
				self->nextthink = -1;
			}
			
/*
			if( !( self->spawnflags & DOOR_START_OPEN ) && pHook->wait >= 0 )
			{
				self->think = door_close;
				self->nextthink = gstate->time + pHook->wait;
			}
*/
		}
	}
}

void door_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, door_hook_fields, sizeof( doorHook_t ) );
}

typedef	(*func_t)(edict_t *);
void door_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, door_hook_fields, sizeof( doorHook_t ) );
	
	doorHook_t	*pHook = ( doorHook_t * ) ent->userHook;

	if( ( pHook->state == STATE_BOTTOM ) )
	{
		ent->s.sound = 0;
	}

	if( strcmp( ent->className, "func_train" ) == 0 )
	{
		ent->think = func_train_init_from_save;
		ent->nextthink = gstate->time + 0.1f;
	}
	else if( strstr( ent->className, "func_door" ) )
	{
		pHook->already_linked = 0;
		ent->think = door_init_from_save;
		ent->nextthink = gstate->time + 0.1f;

	}
	else if( strcmp( ent->className, "path_corner_train" ) == 0 )
	{
	}
	else if( strcmp( ent->className, "func_button" ) == 0 )
	{
//		ent->think = NULL;
//		ent->nextthink = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	areaportal_toggle
//
//	toggles a func_areaportal on and off
//
//	self->frags = portal number
//	self->light_level = state, TRUE = open, FALSE = closed
///////////////////////////////////////////////////////////////////////////////

void	areaportal_toggle (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
//	self->light_level ^= 1;		// toggle state

	gstate->SetAreaPortalState (self->hacks_int, self->light_level);
/*
	if (self->light_level == AREAPORTAL_OPEN)
		gstate->Con_Printf ("area portal #%i opened.\n", self->frags);
	else
		gstate->Con_Printf ("area portal #%i closed.\n", self->frags);
*/
}

///////////////////////////////////////////////////////////////////////////////
//	areaportal_set
//
//	sets a func_areaportal to the specified state
//
//	self->frags = portal number
//	self->light_level = state, TRUE = open, FALSE = closed
///////////////////////////////////////////////////////////////////////////////

void	areaportal_set (userEntity_t *self, int state)
{
	self->light_level = state;

	gstate->SetAreaPortalState (self->hacks_int, state);
}

///////////////////////////////////////////////////////////////////////////////
//	func_areaportal
//
//	self->frags = portal number
//	self->light_level = state, TRUE = close, FALSE = open
///////////////////////////////////////////////////////////////////////////////

void	func_areaportal (userEntity_t *self)
{
	int	i;

	self->save = areaportal_save;
	self->load = areaportal_load;

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "style"))
			self->hacks_int = atoi (self->epair [i].value);

		//	get mins/maxs for epairs for areaportals
		else 
		if (!stricmp (self->epair [i].key, "minsx"))
			self->absmin.x = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "minsy"))
			self->absmin.y = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "minsz"))
			self->absmin.z = atof (self->epair [i].value);

		else 
		if (!stricmp (self->epair [i].key, "maxsx"))
			self->absmax.x = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "maxsy"))
			self->absmax.y = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "maxsz"))
			self->absmax.z = atof (self->epair [i].value);
	}

	self->use = areaportal_toggle;
	self->light_level = AREAPORTAL_CLOSED;
}

///////////////////////////////////////////////////////////////////////////////
//	door_find_areaportal
//
//	looks for an area portal inside of a door and links them if found
///////////////////////////////////////////////////////////////////////////////

void	door_find_areaportal (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	userEntity_t	*head;

	for (head = gstate->FirstEntity (); head; head = gstate->NextEntity (head))
	{
		if (head->className && !stricmp (head->className, "func_areaportal"))
		{
			//	check if this areaportal is already linked
			if (head->owner != NULL)
				continue;
			else if (head->absmin.x > self->absmax.x ||
					head->absmin.y > self->absmax.y ||
					head->absmin.z > self->absmax.z ||
					head->absmax.x < self->absmin.x ||
					head->absmax.y < self->absmin.y ||
					head->absmax.z < self->absmin.z)
				continue;

			hook->portal = head;
			head->owner = self;

			//	toggle portal off if door starts open
			if (self->spawnflags & DOOR_START_OPEN)
				areaportal_set (head, AREAPORTAL_OPEN);

			return;
		}
	}
}

//////////////////////////////
//	door_blocked
//
//////////////////////////////

void	door_blocked (userEntity_t *self, userEntity_t *other)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// FIXME: Call damage routine to squish

	client_deathtype = CRUSHED;

// SCG[2/15/00]: THis whole thing is silly...
/*

	if( !( self->spawnflags & DOOR_FORCEMOVE ) )
	{
		if(!(other->flags & FL_BOT))
		{
			// NSS[1/14/00]:Do damage in the case that we are not a bot and we are not supposed to do a forcemove.
			if(hook->damage)
				com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
		}
	}
	// SCG[2/15/00]: FL_FORCEMOVE is for self->flags!!!
	else if( self->spawnflags & FL_FORCEMOVE)
	{
		// NSS[1/6/00]:Always do damage to anything blocking if we have damage applied.
		if(hook->damage)
			com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);

	}
*/
	// SCG[2/15/00]: If this has forcemove on it, kill whatever is blocking the door
	//if( ( self->spawnflags & DOOR_FORCEMOVE ) || ( self->flags & FL_FORCEMOVE ) )
	if(  self->flags & FL_FORCEMOVE )
	{
		if( ( hook != NULL ) && ( hook->damage ) )
		{
			com->Damage( other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD );
		}
		if(self->hacks < gstate->time)
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/m_gibbonecrke.wav"),1.0f, 256.0f, 512.0f);
			self->hacks = gstate->time + 4.0f;
		}
	}
	else
	{
		// SCG[2/15/00]: Do not hurt bots (sidekicks)
		if( !( other->flags & FL_BOT ) )
		{
			if( ( hook != NULL ) && ( hook->damage ) )
			{
				com->Damage( other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD );
			}
		}

		// NSS[2/9/00]:This makes more sense.
		if (self->hacks < gstate->time)
		{
			door_reverse(self, other, other);
			self->hacks = gstate->time + 5.0f;
		}
	}
}

//////////////////////////////
//	plat_restore_touch
//
//////////////////////////////

void	plat_restore_touch (userEntity_t *self)
{
	self->touch = plat_touch;

	self->think = NULL;
	self->nextthink = -1;
}

///////////////////////////////////////////////////////////////////////////////
//	door_continuous_refire
//
///////////////////////////////////////////////////////////////////////////////

void	door_continuous_refire (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	door_fire (self, self, hook->activator);
}

void door_stop_looping_sound( userEntity_t *self, int nSoundIndex )
{
//	gstate->StartEntitySound( self, CHAN_LOOP, nSoundIndex, 0, 0, 0 );
	self->s.sound = 0;
}

void door_handle_opening_sounds( userEntity_t *self )
{
	doorHook_t	*hook = ( doorHook_t * ) self->userHook;

	if( hook->sound_up )
	{
		if( ( strcmp( self->className, "func_door_rotate" ) == 0 ) && ( self->spawnflags & ROTATE_LOOP_SOUNDS ) )
		{
			self->s.sound = hook->sound_up;
		}
		else if( self->spawnflags & DOOR_LOOP_SOUNDS )
		{
			self->s.sound = hook->sound_up;
		}
		else if( strcmp( self->className, "func_plat" ) == 0 )
		{
			self->s.sound = hook->sound_up;
		}
		else
		{
			gstate->StartEntitySound( self, CHAN_OVERRIDE, hook->sound_up, self->s.volume, self->s.dist_min, self->s.dist_max );
		}
	}
}

void door_handle_opened_sounds( userEntity_t *self )
{
	doorHook_t	*hook = ( doorHook_t * ) self->userHook;

	door_stop_looping_sound( self, hook->sound_up );
	if( hook->sound_top )
	{
		gstate->StartEntitySound( self, CHAN_OVERRIDE, hook->sound_top, self->s.volume, self->s.dist_min, self->s.dist_max );
	}
}

void door_handle_closing_sounds( userEntity_t *self )
{
	doorHook_t	*hook = ( doorHook_t * ) self->userHook;

	if( hook->sound_down )
	{
		if( ( strcmp( self->className, "func_door_rotate" ) == 0 ) && ( self->spawnflags & ROTATE_LOOP_SOUNDS ) )
		{
			self->s.sound = hook->sound_down;
		}
		else if( self->spawnflags & DOOR_LOOP_SOUNDS )
		{
			self->s.sound = hook->sound_down;
		}
		else if( strcmp( self->className, "func_plat" ) == 0 )
		{
			self->s.sound = hook->sound_down;
		}
		else
		{
			gstate->StartEntitySound( self, CHAN_OVERRIDE, hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max );
		}
	}
}

void door_handle_closed_sounds( userEntity_t *self )
{
	doorHook_t	*hook = ( doorHook_t * ) self->userHook;

	door_stop_looping_sound( self, hook->sound_down );
	if( hook->sound_bottom )
	{
		gstate->StartEntitySound( self, CHAN_OVERRIDE, hook->sound_bottom, self->s.volume, self->s.dist_min, self->s.dist_max );
	}
}

//////////////////////////////
//	door_fully_open
//
//////////////////////////////

void door_fully_open(userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	door_handle_opened_sounds( self );
/*
	//NSS[12/3/99]:Turn off looped sounds for func_door_rotate
	if (stricmp (self->className, "func_door_rotate") == 0)
	{
		if(self->spawnflags & ROTATE_LOOP_SOUNDS)
		{
			gstate->StartEntitySound(self, CHAN_LOOP, hook->sound_up, 0.0f, 0.0, 0.0);
		}	
	}
	// mdm 98.01.22 - this is correct
	gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top, self->s.volume, self->s.dist_min, self->s.dist_max);
	// kill the looping sound
	self->s.sound = 0;
*/
	hook->state = STATE_TOP;

	if (hook->wait == -1)
		hook->no_more_move = TRUE;

	if (!stricmp (self->className, "func_door"))
	{
		if (self->spawnflags & DOOR_USE_TO_CLOSE)
		{
			self->use = door_use;
			return;
		}
	}
	else if (!stricmp (self->className, "func_door_rotate"))
	{
		if (self->spawnflags & ROTATE_USE_TO_CLOSE)
		{
			self->use = door_use;
			return;
		}
	}
	else if (!stricmp (self->className, "func_plat"))
	{
		if (self->spawnflags & PLAT_START_UP)
		{
			self->think = plat_restore_touch;
			self->nextthink = gstate->time + 1.0;
		}
		else
			self->touch = plat_touch;
	}

	if (hook->continuous_mode == CONTINUOUS_TRIGGERED || hook->continuous_mode == CONTINUOUS_ON)
	{
		self->think = door_continuous_refire;
		if (hook->wait > 0)
			self->nextthink = gstate->time + hook->wait;
		else
		{
			hook->wait = 0.1;
			self->nextthink = gstate->time + 0.1;
		}
		return;
	}
	else if (self->spawnflags & DOOR_TOGGLE)
	{
		self->nextthink = -1;
		return;
	}
	else if (!(self->spawnflags & DOOR_START_OPEN) && hook->wait >= 0)
	{
		self->think = door_close;
		self->nextthink = gstate->time + hook->wait;

	}
	else
		self->nextthink = -1;

	gstate->LinkEntity( self );
}

//////////////////////////////
//	door_fully_closed
//
//////////////////////////////

void door_fully_closed(userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	hook->state = STATE_BOTTOM;

	door_handle_closed_sounds( self );
/*
	//NSS[12/3/99]:Turn off looped sounds for func_door_rotate
	if (stricmp (self->className, "func_door_rotate") == 0)
	{
		if(self->spawnflags & ROTATE_LOOP_SOUNDS)
		{
			gstate->StartEntitySound(self, CHAN_LOOP, hook->sound_down, 0.0f, 0.0, 0.0);
		}
	}
	gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_bottom, self->s.volume, self->s.dist_min, self->s.dist_max);

	// kill the looping sound
	self->s.sound = 0;
*/
	if (hook->wait == -1)
		hook->no_more_move = TRUE;

	// removed touch function
	if (!stricmp (self->className, "func_door"))
	{
		if (self->spawnflags & DOOR_AUTO_OPEN)
			self->touch = door_touch;
		if (self->spawnflags & DOOR_USE_TO_CLOSE)
			self->use = door_use;
	}
	else if (!stricmp (self->className, "func_door_rotate"))
	{
		if (self->spawnflags & ROTATE_AUTO_OPEN)
			self->touch = door_touch;
		if (self->spawnflags & ROTATE_USE_TO_CLOSE)
			self->use = door_use;
	}
	else if (stricmp (self->className, "func_button") == 0)
	{
		if (self->spawnflags & PUSH_TOUCH)
			self->touch = door_touch;
	}
	else if (!stricmp (self->className, "func_plat"))
	{
		self->touch = plat_touch;
	}
	else
		self->touch = door_touch;

	if (hook->continuous_mode == CONTINUOUS_TRIGGERED || hook->continuous_mode == CONTINUOUS_ON)
	{
		//	in continuous mode, fire again

		self->think = door_continuous_refire;
		if (hook->wait > 0)
			self->nextthink = gstate->time + hook->wait;
		else
			self->nextthink = gstate->time + 0.1;
		return;
	}
	else if (self->spawnflags & DOOR_TOGGLE)
	{
		self->nextthink = -1;
		return;
	}
	else if (!(self->spawnflags & DOOR_START_OPEN) || hook->wait < 0)
	{
		self->think = NULL;
		self->nextthink = -1;
	}
	else
	{
		self->think = door_open;
		self->nextthink = gstate->time + hook->wait;
	}

	//	toggle area portal if one is associated with this door
	// SCG[2/21/00]: we only want to use area portals if the door is closed and if it's links are closed...
/*
	qboolean bSafeToClose = TRUE;
	doorHook_t *link_hook = ( doorHook_t * ) hook;
	if( hook->portal )
	{
		while( link_hook->linked_door )
		{
			if( link_hook->state != STATE_BOTTOM )
			{
				bSafeToClose = FALSE;
			}
			link_hook = ( doorHook_t * ) link_hook->linked_door->userHook;
		}

		if( bSafeToClose == TRUE )
		{
			hook->portal->use( hook->portal, self, self );
		}
	}
*/

//	if (hook->portal)
//		hook->portal->use (hook->portal, self, self);

	gstate->LinkEntity( self );
}

//////////////////////////////
//	door_close
//
//////////////////////////////

void	door_close (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// SCG[2/25/00]: if the door is already closing, early out.
 	if( hook->state == STATE_DOWN )
	{
		return;
	}

	if (hook->base_health)
	{
		self->takedamage = DAMAGE_YES;
		self->health = hook->base_health;
	}

	hook->state = STATE_DOWN;

	door_handle_closing_sounds( self );

	if (stricmp (self->className, "func_door_rotate") == 0)
	{
		// loop sound down
		//self->s.sound = hook->sound_down;
/*
		if(self->spawnflags & ROTATE_LOOP_SOUNDS)
		{
			gstate->StartEntitySound(self, CHAN_LOOP, hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
		else
		{
			gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
*/
		com_CalcAngleMove (self, hook->pos1, hook->speed, door_fully_closed);
	}
	else
	{
/*
		// amw: 5.11.99 - this totally shits in the bed..
		if (self->spawnflags & DOOR_LOOP_SOUNDS)
		{
			// loop sound down
			self->s.sound = hook->sound_down;
		}
		else
		{
			// play down sound
			gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
*/
		com_CalcMove (self, hook->pos1, hook->speed, door_fully_closed);
	}
}

//////////////////////////////
//	door_open
//
//////////////////////////////

void	door_open (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// is door already going up?
//	if (hook->state == STATE_UP)
//		return;

	// SCG[2/25/00]: if the door is moving, early out.
	if( hook->state == STATE_UP )
	{
		return;
	}
	if (hook->state == STATE_TOP)
	{
		self->nextthink = gstate->time + hook->wait;
		return;
	}

	//	toggle area portal if one is associated with this door and door is closed
	if( ( hook->portal != NULL ) && ( hook->state == STATE_BOTTOM ) )
	{
		areaportal_set( hook->portal, AREAPORTAL_OPEN );
	}
	
	door_handle_opening_sounds( self );

	hook->state = STATE_UP;

	if (!stricmp (self->className, "func_door_rotate"))
	{
		// where are we going to rotate to?
		CVector doorDir;
		doorDir = hook->pos2;

		//NSS[11/27/99]:
		// loop going up sound
		//self->s.sound = hook->sound_up;
/*
		if(self->spawnflags & ROTATE_LOOP_SOUNDS)
		{
			gstate->StartEntitySound(self, CHAN_LOOP, hook->sound_up, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
		else
		{
			gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_up, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
*/
		if (self->spawnflags & ROTATE_SWING)
		{
			float dotProduct;
			CVector rightDir, doorVector;
			CVector f, u;
			
			doorVector = self->s.origin - hook->activator->s.origin;
			AngleToVectors(hook->activator->s.angles , f, rightDir, u );

			
			dotProduct = DotProduct(doorVector, rightDir);

			// if it's positive, we need to reverse the door's path
			if (dotProduct > 0)
			{
				doorDir.Negate( hook->pos2 );
			}
		}

		com_CalcAngleMove( self, doorDir, hook->speed, door_fully_open );
	}
	else
	{
/*
		// amw: 5.11.99 - this totally shits in the bed..
		if (self->spawnflags & DOOR_LOOP_SOUNDS)
		{
			// loop going up sound
			self->s.sound = hook->sound_up;
		}
		else
		{
			// play up sound
			gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_up, self->s.volume, self->s.dist_min, self->s.dist_max);
		}
*/
		//&&& AMW 6.25.98 - recalculate the door offsets if the parent has rotated etc
		if (self->teammaster)
		{
			CVector new_org;

			CVector adjOrigin;  // in case the door has collided or something
			adjOrigin = self->teammaster->s.origin + self->transformedOffset;

			// reset pos1 for door_close()  .. it's always the current origin of the door before it opens
			hook->pos1 = adjOrigin;

//			com->TransformChildOffset( adjOrigin.vec(), hook->pos2_offs.vec(), self->s.angles, TRUE, new_org );
			com->TransformChildOffset( adjOrigin, hook->pos2_offs, self->teammaster->s.angles, TRUE, new_org );

			// update the child origin
			hook->pos2 = new_org;
		}

		com_CalcMove (self, hook->pos2, hook->speed, door_fully_open);
	}
}

/////////////////////////////
//	door_fire
//
/////////////////////////////

void	door_fire (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*head;
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if( ( self->owner != NULL ) && ( self->owner != self ) )
	{
		// make sure we're pointing to a master door
		//	so that we can fire all linked doors
		self = self->owner;
		hook = (doorHook_t *) self->userHook;
	}

	// SCG[2/19/00]: Make sure hook is not null so doors that get killed
	// SCG[2/19/00]: do not get called
	if( ( hook == NULL ) || ( hook->no_more_move ) )
	{
		return;
	}

	// FIXME: play use key sound

	self->message = "";

	// trigger all paired doors
	// basically traverse the linked_door field until it comes
	// up NULL
	head = self->owner;

	do
	{
		hook = (doorHook_t *) head->userHook;

		// make sure linked doors know who activated them
		hook->activator = activator;

		if (hook->state == STATE_TOP)
		{

			if (head->delay != 0.0)
			{
				// still need to delay using head if it has a delay
				head->think = door_close;
				head->nextthink = gstate->time + head->delay;
			}
			else
				door_close (head);

			//	com->UseTargets will handle delays for using targets
			//	Q2FIXME:	activator...
			com->UseTargets (head, other, activator);
		}
		else
		if (hook->state == STATE_BOTTOM)
		{

			if (head->delay != 0.0)
			{
				// still need to delay using head if it has a delay
				head->think = door_open;
				head->nextthink = gstate->time + head->delay;
			}
			else
				door_open (head);

			// amw: moved here because of dabug's fucked up maps...
			//	com->UseTargets will handle delays for using targets
			com->UseTargets (head, other, activator);
		}

		head = hook->linked_door;
	} while (head != NULL);
}

/////////////////////////////
//	door_reverse
//
/////////////////////////////

void door_reverse (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*head;
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (self->owner != self)
	{
		// make sure we're pointing to a master door
		//	so that we can fire all linked doors
		self = self->owner;
		hook = (doorHook_t *) self->userHook;
	}

	self->message = "";

	// trigger all paired doors
	// basically traverse the linked_door field until it comes
	// up NULL
	head = self->owner;

	do
	{
		hook = (doorHook_t *) head->userHook;

		// make sure linked doors know who activated them
		hook->activator = activator;

		if (hook->state == STATE_TOP || hook->state == STATE_UP)
		{
			if( strcmp( self->className, "func_plat" ) == 0 )
			{
				door_open (head);
			}
			else
			{
				if (head->delay != 0.0)
				{
					// still need to delay using head if it has a delay
					head->think = door_close;
					head->nextthink = gstate->time + head->delay;
				}
				else
					door_close (head);
			}

			//	com->UseTargets will handle delays for using targets
			com->UseTargets (head, other, activator);
		}
		else
		if (hook->state == STATE_BOTTOM || hook->state == STATE_DOWN)
		{
			if( strcmp( self->className, "func_plat" ) == 0 )
			{
				door_open (head);
			}
			else
			{
				if (head->delay != 0.0)
				{
					// still need to delay using head if it has a delay
					head->think = door_open;
					head->nextthink = gstate->time + head->delay;
				}
				else
					door_open (head);
			}

			//	com->UseTargets will handle delays for using targets
			com->UseTargets (head, other, activator);
		}

		head = hook->linked_door;
	} while (head != NULL);
}


/////////////////////////////
//	door_use
//
/////////////////////////////

void	door_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if( hook == NULL )
	{
		return;
	}

	if (hook->no_more_move)
		return;

	//&&& AMW 6.12.98 - temporarily disable this button if it's a child and it's moving
	if (self->flags & FL_TEAMSLAVE)
	{
		userEntity_t *parent = self->teammaster;
		if (parent &&
			parent->velocity.x  || parent->velocity.y  || parent->velocity.z ||
			parent->avelocity.x || parent->avelocity.y || parent->avelocity.z)
		{
			return;
		}
	}

	// FIXME - is gstate->activator the same as activator here?
//	hook->activator = gstate->activator;
	hook->activator = gstate->activator = activator;

	if (self->spawnflags & DOOR_CONTINUOUS)
	{
		if (hook->continuous_mode != CONTINUOUS_OFF)
		{
			if (hook->continuous_mode == CONTINUOUS_TRIGGERED)
				//	only stop in starting position
				hook->continuous_mode = CONTINUOUS_STOP_NEXT;
			else
				hook->continuous_mode = CONTINUOUS_OFF;
		}
		else
			hook->continuous_mode = CONTINUOUS_ON;
	}
	
	if (self->message && activator->flags & FL_CLIENT)
		gstate->centerprintf (activator, self->message);

#if 0 // not needed.. delete soon
	// jack up this door's damage when it is triggered
	if (hook->trigger_damage > 0)
		hook->damage += hook->trigger_damage;
#endif
	//	fire master door
	if( self->owner )
	{
		door_fire (self->owner, other, activator);
	}

	// fire off and cine or ai scripts attached to this entity
	if( hook->pCineScript!= NULL && hook->pAiScript != NULL )
	{
		com_PlayEntityScript(activator, hook->pCineScript, hook->pAiScript);
	}

}

///////////////////////////////////////////////////////////////////////////////
//	door_trigger_touch
//
///////////////////////////////////////////////////////////////////////////////

void	door_trigger_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (!com->ValidTouch (self, other) || gstate->time < hook->move_finished)
		return;

	gstate->activator = other;

	//	Q2FIXME: why are we passing other and activator?!?!?
	door_use (self, other, other);
}

///////////////////////////////////////////////////////////////////////////////
//	door_killed
//
///////////////////////////////////////////////////////////////////////////////

void	door_killed (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	doorHook_t		*hook = (doorHook_t *) self->owner->userHook;

	self = self->owner;
	self->health = hook->base_health;
	self->takedamage = DAMAGE_NO;

	//	Q2FIXME: why are we passing other and activator?!?!?
	door_use (self, attacker, attacker);
}

///////////////////////////////////////////////////////////////////////////////
//	door_touch
//
//	Prints messages and opens key doors
//
//	Used in Quake only for key doors, apparently
///////////////////////////////////////////////////////////////////////////////

void	door_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*owner_hook = (doorHook_t *) self->owner->userHook;
	userEntity_t	*owner = self->owner;

	if (!com->ValidTouch (self, other))
		return;

	if (stricmp (self->className, "func_plat") == 0)
	{
		if (hook->ignore_time > gstate->time)
			return;
	}

	if (owner_hook->move_finished > gstate->time)
		return;

	owner_hook->move_finished = gstate->time + 2;

	if (owner->message && other->flags & FL_CLIENT)
		gstate->centerprintf (other, owner->message);

	self->touch = NULL;

	if (hook->linked_door)
		hook->linked_door->touch = NULL;

	door_use (self, other, other);
}

////////////////////////////////
//	plat_wait_move
//
////////////////////////////////

void	plat_wait_move (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*owner_hook = (doorHook_t *) self->owner->userHook;
	userEntity_t	*owner = self->owner;

	owner_hook->move_finished = gstate->time + 2;

	self->touch = NULL;

	if (hook->linked_door)
		hook->linked_door->touch = NULL;

	door_use (self, self, self);
}

///////////////////////////////////////////////////////////////////////////////
//	plat_touch
//
///////////////////////////////////////////////////////////////////////////////

void	plat_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*owner_hook = (doorHook_t *) self->owner->userHook;
	userEntity_t	*owner = self->owner;

//	gstate->Con_Printf ("plat_touch\n");

	//	don't start based on touch if we're at the top and initial position is down, or
	//	if we're at the bottom and initial position is up
	if (hook->state == STATE_TOP && !(self->spawnflags & PLAT_START_UP))
		return;
	if (hook->state == STATE_BOTTOM && self->spawnflags & PLAT_START_UP)
		return;

	//	don't start if toucher is not moving since this probably means they just rode
	//	the plat down and are simply standing on it
	if (other->velocity.x == 0 && other->velocity.y == 0)
		return;

	if (!com->ValidTouch (self, other))
		return;

	if (hook->ignore_time > gstate->time)
		return;

	if (owner_hook->move_finished > gstate->time)
		return;

	owner_hook->move_finished = gstate->time + 2;

	if (owner->message && other->flags & FL_CLIENT)
		gstate->centerprintf (other, owner->message);

	self->touch = NULL;

	if (hook->linked_door)
		hook->linked_door->touch = NULL;

	//	Q2FIXME: why are we passing other and activator?!?!?
	door_use (self, other, other);
}

/////////////////////////////
//	plat_trigger_touch
//
//	trigger spawns over the plat's down position
//	for plats that start down
/////////////////////////////

void	plat_trigger_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t		*owner_hook = (doorHook_t *) self->owner->userHook;

	if (!com->ValidTouch (self, other))
		return;

	if (owner_hook->state == STATE_UP)
		return;

	if (owner_hook->ignore_time >= gstate->time && other->velocity.z == 0.0)
		return;

//	gstate->Con_Printf ("plat_trigger_touch\n");

	owner_hook->ignore_time = gstate->time + 0.5;
}

/*
===========================================================
Spawning functions
===========================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	SpawnPlatTrigger
//
//	spawns a trigger on top of a platform's down position, when this trigger
//	is hit, the associated platform will be be stopped from triggering for
//	an amount of time after that
//
//	stops plats from going up immediately when someone falls down on them
///////////////////////////////////////////////////////////////////////////////

userEntity_t *SpawnPlatTrigger (userEntity_t *self)
{
	userEntity_t	*trigger;
	CVector			min, max, temp, org, t_org;

	if (self->spawnflags & PLAT_START_UP)
		return	NULL;

	trigger = gstate->SpawnEntity ();
	trigger->movetype = MOVETYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->owner = self;
	trigger->touch = plat_trigger_touch;
	trigger->className = "plat_trigger";

	trigger->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);

	com->GetMinsMaxs (self, org, min, max);

	//	spawn trigger above plat
	t_org= org + CVector(0, 0, max.z + 72.0);

	gstate->SetOrigin (trigger, t_org.x, t_org.y, t_org.z);

	// make 8 smaller than plat on each edge
	temp.Set(8.0, 8.0, 0.0);
	min = min + temp;
	max = max - temp;
	max.z = 8.0;
	min.z = -8.0;

//	gstate->SetSize (trigger, min.x, min.y, min.z, max.x, max.y, max.z);
//	com->SpawnMarker (self, t_org, debug_models [DEBUG_MARKER], 1000);

	return trigger;
}

///////////////////////////////////////////////////////////////////////////////
//	SpawnField
//
//	spawns a trigger field around a door so that it can be opened before the
//	door is actually touched
///////////////////////////////////////////////////////////////////////////////

userEntity_t *SpawnField (userEntity_t *self, CVector &fmins, CVector &fmaxs)
{
	userEntity_t	*trigger;
	CVector			t1, t2, temp;

	trigger = gstate->SpawnEntity ();
	trigger->movetype = MOVETYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->owner = self;
	trigger->touch = door_trigger_touch;
	trigger->className = "door_trigger";
	trigger->flags |= FL_NOSAVE;

	trigger->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	trigger->save = door_hook_save;
	trigger->load = door_hook_load;

	t1 = fmins;
	t2 = fmins;

	if (stricmp (self->className, "func_door_rotate") == 0)
	{
		temp.Set(96.0, 96.0, 8.0);
	}
	else
	{
		temp.Set(60.0, 60.0, 8.0);
	}

	t1 = t1 - temp;
	t2 = t2 + temp;

	gstate->SetSize (trigger, t1.x, t1.y, t1.z, t2.x, t2.y, t2.z);

	return trigger;
}

///////////////////////////////////////////////////////////////////////////////
//	EntitiesTouching
//
///////////////////////////////////////////////////////////////////////////////

int	EntitiesTouching (userEntity_t *e1, userEntity_t *e2)
{
	CVector	mins1, maxs1, mins2, maxs2;

	mins1 = e1->absmin;
	maxs1 = e1->absmax;

	mins2 = e2->absmin;
	maxs2 = e2->absmax;

	if (mins1.x > maxs2.x)
		return FALSE;
	if (mins1.y > maxs2.y)
		return FALSE;
	if (mins1.z > maxs2.z)
		return FALSE;
	if (maxs1.x < mins2.x)
		return FALSE;
	if (maxs1.y < mins2.y)
		return FALSE;
	if (maxs1.z < mins2.z)
		return FALSE;

	return TRUE;
}

///////////////////////////
//	TouchesDoorInList
//
///////////////////////////

int	TouchesDoorInList (userEntity_t *start_door, userEntity_t *door)
{
	doorHook_t		*cur_hook;
	userEntity_t	*cur_door;

	cur_door = start_door;

	while (cur_door)
	{
		if (EntitiesTouching (cur_door, door))
			return	TRUE;

		cur_hook = (doorHook_t *) cur_door->userHook;
		cur_door = cur_hook->linked_door;
	}

	return	FALSE;
}

///////////////////////////
//	TouchingDoor
//
//	searches through the entire entity list
//	and returns the first unlinked door that
//	touches self
///////////////////////////

userEntity_t	*TouchingDoor (userEntity_t *self)
{
	userEntity_t	*door;
	doorHook_t		*dhook;

	for (door = gstate->FirstEntity (); door; door = gstate->NextEntity (door))
	{
		if (!door->className || door == self)
			continue;
		if (stricmp (door->className, "func_door") && stricmp (door->className, "func_door_rotate"))
			continue;

		dhook = (doorHook_t *) door->userHook;
		if (dhook->already_linked)
			continue;

		if (EntitiesTouching (self, door))
			return	door;
	}

	return	NULL;
}

///////////////////////////
//	AddDoorToList
//
//	adds a door to the end of a master
//	door's linked_door list
//
//	master doors have self->owner == self
//
//	this function will accept a non-master
//	door being passed as master, as long
//	as master->owner is the real master door
///////////////////////////

void	AddDoorToList (userEntity_t *master, userEntity_t *new_door)
{
	doorHook_t		*hook;

	//	just in case a non-master door is passed
	master = master->owner;
	hook = (doorHook_t *) master->userHook;

	//	go to last door in list
	while (hook->linked_door)
		hook = (doorHook_t *) hook->linked_door->userHook;

	hook->linked_door = new_door;

	hook = (doorHook_t *) new_door->userHook;
	hook->linked_door = NULL;
	hook->already_linked = TRUE;
}

///////////////////////////
//	DoorFindTouching
//
//	recursively finds all doors touching
//	self, or doors touching those doors...
///////////////////////////

void	DoorFindTouching (userEntity_t *door, CVector &trig_min, CVector &trig_max)
{
	userEntity_t	*new_door;

	for (new_door = TouchingDoor (door); new_door; new_door = TouchingDoor (door))
	{
		if (new_door->absmin.x < trig_min.x)
			trig_min.x = new_door->absmin.x;
		if (new_door->absmin.y < trig_min.y)
			trig_min.y = new_door->absmin.y;
		if (new_door->absmin.z < trig_min.z)
			trig_min.z = new_door->absmin.z;
		if (new_door->absmax.x < trig_max.x)
			trig_max.x = new_door->absmax.x;
		if (new_door->absmax.y < trig_max.y)
			trig_max.y = new_door->absmax.y;
		if (new_door->absmax.z < trig_max.z)
			trig_max.z = new_door->absmax.z;

		AddDoorToList (door, new_door);
		new_door->owner = door->owner;
		DoorFindTouching (new_door, trig_min, trig_max);
	}
}

///////////////////////////
//	DoorTargetted
//
///////////////////////////

int	DoorTargetted (userEntity_t *master)
{
	doorHook_t		*hook;
	userEntity_t	*door;

	//	just in case the real master isn't passed
	door = master->owner;

	while (door)
	{
		if (door->targetname)
			return	TRUE;

		hook = (doorHook_t *) door->userHook;
		door = hook->linked_door;
	}

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//	door_parse_sounds
//
///////////////////////////////////////////////////////////////////////////////

void	door_parse_sounds (userEntity_t *self)
{
	int				i;
	char			*name;
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	for (i = 0; self->epair [i].key; i++)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (!stricmp (self->epair [i].key, "sound_open_finish"))
		{
			name = self->epair [i].value;
			hook->sound_top = gstate->SoundIndex (name);
		}
		else if (!stricmp (self->epair [i].key, "sound_close_finish"))
		{
			name = self->epair [i].value;
			hook->sound_bottom = gstate->SoundIndex (name);
		}
		else if (!stricmp (self->epair [i].key, "sound_opening"))
		{
			name = self->epair [i].value;
			hook->sound_up = gstate->SoundIndex (name);
		}
		else if (!stricmp (self->epair [i].key, "sound_closing"))
		{
			name = self->epair [i].value;
			hook->sound_down = gstate->SoundIndex (name);
		}
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			self->s.volume = atof(self->epair[i].value);
	}

	// set default volume to full
	//NSS[11/27/99]:Reduced over-all sound to be played at about 0.65 instead of maxing out sounds
	if (!self->s.volume) self->s.volume = 0.65f;
	// set default distances
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;
}


///////////////////////////////////////////////////////////////////////////////
//	door_init
//
//	sets up a door entity
//
//	finds associated func_areaportals
//	links touching doors
//	creates triggers
///////////////////////////////////////////////////////////////////////////////


void	door_init (userEntity_t *self)
{
	CVector			real_org, trig_min, trig_max, size;
	doorHook_t		*cur_hook, *self_hook;
	userEntity_t	*target;

	self_hook = (doorHook_t *) self->userHook;

	///////////////////////////////////////////////////////////////////////////
	//	find an area portal for this door, if possible
	///////////////////////////////////////////////////////////////////////////

	if (self->target)
	{
		//	if door has a target, determine if it targets a func_areaportal Q2 style
		//	targetted func_areaportals ALWAYS override touching ones
		target = com->FindTarget (self->target);
		if (target)
		{
			if (!stricmp (target->className, "func_areaportal"))
			{
				if (target->owner)
				{
					//	if this area portal is already linked to a door, unlink it
					cur_hook = (doorHook_t *) target->owner->userHook;
					cur_hook->portal = NULL;
				}

				self_hook->portal = target;
				target->owner = self;

				//	clear door's target so UseTargets doesn't re-use the portal
				self->target = NULL;

				//	toggle portal off if door starts open
				if (self->spawnflags & DOOR_START_OPEN)
					areaportal_set (target, AREAPORTAL_OPEN);
			}
/*
			else
			{
				gstate->Con_Dprintf ("Bad target on %s.\n", self->className);
				self->target = "";
			}
*/
		}
		else
			//	not targeting a func_areaportal, so search for one touching
			door_find_areaportal (self);
	}
	else
		door_find_areaportal (self);

	//	Nelno:	6/10/98, make sure func_trains and all other doors have
	//			done their setup before linking to parent, this may not
	//			really do the trick in all cases, especially with parents
	//			that have parents...
	if (self->parentname)
	{
//		self->think = door_find_parent;
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.5;
	}
	else
	{
		self->think = NULL;
		self->nextthink = -1;
	}

	if (self_hook->already_linked)
		return;

	//	extents of trigger
	trig_min = self->absmin;
	trig_max = self->absmax;

	self_hook->already_linked = TRUE;

	if (!(self->spawnflags & DOOR_DONT_LINK))
		DoorFindTouching (self, trig_min, trig_max);

	// if secret door or if door has a target, don't make a trigger field
	if (self->health || DoorTargetted (self))
		return;

	if (stricmp (self->className, "func_door_rotate") == 0)
	{
		if (!(self->spawnflags & ROTATE_AUTO_OPEN))
			return;
	}
	else if (stricmp (self->className, "func_door") == 0)
	{
		if (!(self->spawnflags & DOOR_AUTO_OPEN))
			return;
	}

	// if door requires keys, return
/*
	if (self->items)
		return;
*/

	self_hook->trigger_field = SpawnField (self, trig_min, trig_max);
}

///////////////////////////////////////////////////////////////////////////////
//	func_door
///////////////////////////////////////////////////////////////////////////////

void	func_door (userEntity_t *self)
{
	CVector		temp;
	CVector		min_size, trig_max;
	doorHook_t	*hook;
	int			i;
    int         boing = 0, dust = 0, accel = 0, quake = 0;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;
	hook->bStruggle = false;

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

//	memset (hook, 0x00, sizeof (doorHook_t));
	hook->lip = -1;
	hook->wait = 32767;

	self->targetname = self->target = self->killtarget = NULL;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "dmg"))
			hook->damage = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lip"))
			hook->lip = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "speed"))
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "message"))
			self->message = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "boing"))		// simulate bouncing physics if this is set
			boing = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "dust"))		// create a dust cloud when the door stops
			dust = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "accelerate"))	// accelerate the rotation of the door
			accel = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "spawnquake"))	// spawn a small earth quake
			quake = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mass"))		// mass of door.. dust and quakes are based on it
			self->mass = atof (self->epair [i].value);

		i++;
	}

	door_parse_sounds (self);

	com->SetMovedir (self);

	if (boing > 0)
	{
		hook->bitflags |= DF_BOUNCE;
		hook->bounceSteps = 2;
	}

	if (dust > 0)
		hook->bitflags |= DF_DUST;

	if (accel > 0)
		hook->bitflags |= DF_ACCEL;

	if (quake > 0)
		hook->bitflags |= DF_QUAKE;

	hook->base_health = self->health;
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);
	gstate->SetModel (self, self->modelName);

	// doors with a bounce/boing flag set should not call the door_blocked function
	if (!boing)
		self->blocked = door_blocked;
	
    self->use = door_use;

	if (!hook->speed)
		hook->speed = 100.0;
	if (hook->wait == 32767)
		hook->wait = 3;
	if (hook->lip == -1) // amw: now allows a lip of 0 to be valid
		hook->lip = 8;
	if (!hook->damage)
		hook->damage = 20;

	// start position
	hook->pos1 = self->s.origin;

	// find ending position
	temp.x = fabs(self->movedir.x);
	temp.y = fabs(self->movedir.y);
	temp.z = fabs(self->movedir.z);
	hook->pos2 = hook->pos1 + self->movedir * (DotProduct(temp, self->size) - hook->lip);

	// save the original offsets from the origin of the entity

	hook->pos1_offs = hook->pos1 - self->s.origin;
	hook->pos2_offs = hook->pos2 - self->s.origin;


	if (self->spawnflags & DOOR_START_OPEN)
	{
		gstate->SetOrigin (self, hook->pos2.x, hook->pos2.y, hook->pos2.z);
		hook->state = STATE_TOP;
	}
	else
		hook->state = STATE_BOTTOM;

	hook->move_finished = 0.0;
	hook->no_more_move = FALSE;

	// if the door has health, we don't want to be able to use it
	if (self->health)
	{
		self->takedamage = DAMAGE_YES;
		self->die = door_killed;
		self->use = NULL;	
	}

	//NSS[11/28/99]:Forcemove
	if(self->spawnflags & DOOR_FORCEMOVE)
	{
		self->flags |= FL_FORCEMOVE;
		self->blocked = door_blocked;
	}

	//	Q2FIXME:	just for testing doors on Q2 maps
	//	if (!self->targetname)
	//		self->spawnflags |= DOOR_AUTO_OPEN;

	if ((self->targetname) || (!(self->spawnflags & DOOR_AUTO_OPEN)))
		self->touch = NULL;
	else
		self->touch = door_touch;

	// this must be set up if LinkDoors is not working
	self->owner = self;
	hook->linked_door = NULL;

	// door linking can't be done until all of the doors have
	// been spawned so the sizes can be detected properly
	self->think = door_init;
	self->nextthink = gstate->time + 0.1;

	self->s.origin_hack = (self->s.mins + self->s.maxs) * 0.5;

	// SCG[11/5/99]: added flag for struggle sound
	if( self->spawnflags & FUNC_DOOR_STRUGGLE )
	{
		hook->bStruggle = true;
	}
}

/*
================
func_plat

a func_plat is essentially a door, except the lip field is now the height field
and the START_OPEN flag is now the START_UP flag
================
*/

void	func_plat (userEntity_t *self)
{
	CVector		temp;
	doorHook_t	*hook;
	float		height;
	int			i;
	char*		name;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;
	hook->lip = 8;
	height = 0.0;
	hook->wait = 32767;

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (stricmp (self->epair [i].key, "dmg") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "height") == 0)
			height = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "lip") == 0)
			hook->lip = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound_top"))
		{
			name = self->epair [i].value;
			hook->sound_top = gstate->SoundIndex (name);
			if (!hook->sound_top) hook->sound_top = gstate->SoundIndex ("null.wav");
		}
		else if (!stricmp (self->epair [i].key, "sound_bottom"))
		{
			name = self->epair [i].value;
			hook->sound_bottom = gstate->SoundIndex (name);
			if (!hook->sound_bottom) hook->sound_bottom = gstate->SoundIndex ("null.wav");
		}
		else if (!stricmp (self->epair [i].key, "sound_up"))
		{
			name = self->epair [i].value;
			hook->sound_up = gstate->SoundIndex (name);
			if (!hook->sound_up) hook->sound_up = gstate->SoundIndex ("null.wav");
		}
		else if (!stricmp (self->epair [i].key, "sound_down"))
		{
			name = self->epair [i].value;
			hook->sound_down = gstate->SoundIndex (name);
			if (!hook->sound_down) hook->sound_down = gstate->SoundIndex ("null.wav");
		}
		else if (!stricmp (self->epair [i].key, "min"))
			self->s.dist_min = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "max"))
			self->s.dist_max = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "volume"))
			self->s.volume = atof(self->epair[i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;
		i++;
	}

	// set default volume to full
	if (!self->s.volume) self->s.volume = 1.0f;
	// set default distances
	if (!self->s.dist_min) self->s.dist_min = ATTN_NORM_MIN;
	if (!self->s.dist_max) self->s.dist_max = ATTN_NORM_MAX;

	// wrong epair names
	//door_parse_sounds (self);

	hook->base_health = self->health;
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);
	gstate->SetModel (self, self->modelName);

	com->SetMovedir (self);

	self->blocked = door_blocked;
	self->use = door_use;

	// get size of entity
	hook->size = self->absmax - self->absmin;

	// SCG[3/16/00]: ah. The mystery is solved.  It seems this is used to specify the origin of the entity for sound.
	self->s.origin_hack = (self->s.mins + self->s.maxs) * 0.5;

	if (!hook->speed)
		hook->speed = 100.0;
	if (hook->wait == 32767)
		hook->wait = 3;
	if (!hook->damage)
		hook->damage = 2;

	// start position
	hook->pos2 = self->s.origin;

	// find ending position
	temp = self->movedir;

	// if height is set then movement position is calculated based on the height value
	if (height != 0.0)
	{
		temp = temp * height;
		hook->pos1 = hook->pos2 + temp;
	}
	else
	{
		temp = temp * (self->size [2] - hook->lip);
		// find ending position
		hook->pos1 = hook->pos2 + temp;
	}

	// now flip the start/end positions unless the plat should start in the down position
	if (!(self->spawnflags & PLAT_START_UP))
	{
		gstate->SetOrigin (self, hook->pos1.x, hook->pos1.y, hook->pos1.z);
		hook->state = STATE_BOTTOM;
	}
	else
		hook->state = STATE_TOP;

	hook->move_finished = 0.0;

	// if the door has health, we don't want to be able to use it
	if (self->health)
	{
		self->takedamage = DAMAGE_YES;
		self->die = door_killed;
		self->use = NULL;	
	}
/*
	if (self->items)
		hook->wait = -1;
*/
	if (self->targetname)
		self->touch = NULL;
	else
		self->touch = plat_touch;

	// this must be set up if LinkDoors is not working
	self->owner = self;
	hook->linked_door = NULL;

	if (!self->target && !(self->spawnflags & PLAT_START_UP))
		hook->trigger_field = SpawnPlatTrigger (self);

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;
}

///////////////////////////////////////////////////////////////////////////////
//	func_door_rotate
///////////////////////////////////////////////////////////////////////////////

void	func_door_rotate (userEntity_t *self)
{
	doorHook_t	*hook;
	CVector		vec;
	float		distance = 90;
	int			i, boing = 0, dust = 0, accel = 0, quake = 0;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	hook->bStruggle = false;

	hook->wait = 32767;
	self->movedir.Zero();

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	// NSS[2/21/00]:Generic Door blocked code.
	self->blocked = door_blocked;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "damage") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "distance") == 0)
			distance = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "boing"))		// simulate bouncing physics if this is set
			boing = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "dust"))		// create a dust cloud when the door stops
			dust = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "accelerate"))	// accelerate the rotation of the door
			accel = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "spawnquake"))	// spawn a small earth quake
			quake = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mass"))		// mass of door.. dust and quakes are based on it
			self->mass = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key,"forcemove"))	//NSS[12/3/99]:ForceMove (We ran out of flags for this bad boy)
		{
			self->flags |= FL_FORCEMOVE;
		}
		i++;
	}

	self->blocked = door_blocked;

	door_parse_sounds (self);

	hook->base_health = self->health;
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);
	gstate->SetModel (self, self->modelName);
	self->className = "func_door_rotate";

	if (!hook->speed)
		hook->speed = 100.0;
	if (hook->wait == 32767)
		hook->wait = 3;
	if (!hook->damage)
		hook->damage = 2;
	
	// set the entity's hook bitflags 
	if (boing > 0)
	{
		hook->bitflags |= DF_BOUNCE;
		hook->bounceSteps = 2;
	}

	if (dust > 0)
		hook->bitflags |= DF_DUST;

	if (accel > 0)
		hook->bitflags |= DF_ACCEL;

	if (quake > 0)
		hook->bitflags |= DF_QUAKE;

	if ((int)self->spawnflags & ROTATE_X_AXIS)
		self->movedir.z = 1.0;
	else 
	if ((int)self->spawnflags & ROTATE_Y_AXIS)
		self->movedir.x = 1.0;
	else
		self->movedir.y = 1.0;

	if ((int)self->spawnflags & ROTATE_REVERSE)
	{
		vec = self->movedir * -1.0;
		self->movedir = vec;
	}
	
	hook->pos1 = self->s.angles;
	hook->pos2 = hook->pos1 + self->movedir * distance;

#if 0	//&&& AMW 6.26.98 - this is not valid for rotating doors
	// save the original offsets from the origin of the entity
	hook->pos1_offs = hook->pos1 - self->s.origin;
	hook->pos2_offs = hook->pos2 - self->s.origin;
#endif

	hook->no_more_move = FALSE;

	// FIXME: set key types

	if (self->spawnflags & ROTATE_START_OPEN)
	{
		self->s.angles = hook->pos2;

		vec = self->movedir * -1.0;
		self->movedir = vec;
		hook->state = STATE_TOP;
	}
	else
	{
		hook->state = STATE_BOTTOM;
	}

/*
	if (self->items)
		hook->wait = -1;
*/

	if (!self->targetname && (self->spawnflags & ROTATE_AUTO_OPEN))
		self->touch = door_touch;
	else
		self->touch = NULL;

	// doors with a bounce/boing flag set should not call the door_blocked function
	if (!boing)
		self->blocked = door_blocked;

	// if the door has health, we don't want to be able to use it
	if (self->health)
	{
		self->takedamage = DAMAGE_YES;
		self->die = door_killed;
		self->use = NULL;	
	}
	else
	{
		self->use = door_use;
	}
	// NSS[2/19/00]:
	if(!hook->damage)
		hook->damage = 20.0f;

	hook->move_finished = 0.0;

	// this must be set up if LinkDoors is not working
	self->owner = self;
	hook->linked_door = NULL;

	self->think = door_init;
	self->nextthink = gstate->time + 0.1;

	// SCG[3/16/00]: ah. The mystery is solved.  It seems this is used to specify the origin of the entity for sound.
	self->s.origin_hack = (self->s.mins + self->s.maxs) * 0.5;

	// SCG[11/5/99]: added flag for struggle sound
	if( self->spawnflags & FUNC_DOOR_ROTATE_STRUGGLE )
	{
		hook->bStruggle = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	func_door_rotating
//
//	for compatibility with Q2 rotating doors
///////////////////////////////////////////////////////////////////////////////

#define	Q2_FDR_START_OPEN	1
#define	Q2_FDR_REVERSE		2
#define	Q2_FDR_TOGGLE		32
#define	Q2_FDR_X_AXIS		64
#define	Q2_FDR_Y_AXIS		128

void	func_door_rotating (userEntity_t *self)
{
	unsigned	long	sf = 0;

	if (self->spawnflags & Q2_FDR_START_OPEN)
		sf |= ROTATE_START_OPEN;
	if (self->spawnflags & Q2_FDR_REVERSE)
		sf |= ROTATE_REVERSE;
	if (self->spawnflags & Q2_FDR_TOGGLE)
		sf |= ROTATE_TOGGLE;
	if (self->spawnflags & Q2_FDR_X_AXIS)
		sf |= ROTATE_X_AXIS;
	if (self->spawnflags & Q2_FDR_Y_AXIS)
		sf |= ROTATE_Y_AXIS;

	self->spawnflags = sf;

	func_door_rotate (self);
}

/*
============
func_rotate_blocked

damages the blocking entity
============
*/

void func_rotate_blocked (userEntity_t *self, userEntity_t *other)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (!com->ValidTouch (self, other))
		return;

	float fDamage;

	// NSS[1/6/00]:Always do damage to anything blocking if we have damage applied.
	if(hook->damage)
		fDamage = hook->damage;
	else
	{
		fDamage = 5.0f;
	}

	com->Damage (other, self, self, zero_vector, zero_vector, fDamage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
}

/*
============
func_rotate_use

toggles the rotation the brush on and off
============
*/

#define	ROTATE_ON	0x0001

void func_rotate_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// if it's rotating, turn it off
	if (self->spawnflags & ROTATE_ON)
	{
		self->spawnflags &= ~ROTATE_ON;
		self->avelocity.Zero();		
	}
	else
	{
		// it's off.. start it rotating
		self->spawnflags |= ROTATE_ON;
		
		// calculate rotation speeds
		CVector angularVelocity = self->movedir * hook->speed;
		self->avelocity = angularVelocity;
	}
}

/*
============
func_rotate

spawns a rotating brush

============
*/

void func_rotate(userEntity_t *self)
{
	float       speed = 100.0;
	int			i;
	doorHook_t	*hook;
	CVector		temp;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	self->movedir.Zero();

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "dmg") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			self->s.sound = gstate->SoundIndex(self->epair[i].value);
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

	// set the speed
	hook->speed = speed;

	// set axis of rotation
 	if (self->spawnflags & 4)
	{
		self->movedir.z = 1.0;
	}
	else 
	if (self->spawnflags & 8)
	{
		self->movedir.x = 1.0;
	}
	else
	{
		self->movedir.y = 1.0;
	}

	// check for reverse rotation
	if (self->spawnflags & 2)
	{
		temp = self->movedir * -1.0;
		self->movedir = temp;
	}

	self->solid    = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	self->className = "func_rotate";

    // these two are required
	gstate->SetModel(self,  self->modelName);
	gstate->SetOrigin(self, self->s.origin.x, self->s.origin.y, self->s.origin.z );

	// is the func_rotate starting on?
	if (self->spawnflags & ROTATE_ON)
	{
		// start rotating
		temp = self->movedir * hook->speed;
		self->avelocity = temp;
		self->use = NULL;
	}
	else
	{
		self->use = func_rotate_use;
	}

	self->think = NULL;
	self->blocked = func_rotate_blocked;

	// is this func_rotate attached to a moving entity?
	if (self->parentname)
	{
		// com_FindParent links the two entities together and when the parent moves, so will the child
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.10;
	}


}


///////////////////////////////////////////////////////////////////////////////
//	func_wall_use
///////////////////////////////////////////////////////////////////////////////

void	func_wall_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// if it's not solid, make it solid
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
	}
	else  
	{
		// make it go away
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	com->UseTargets (self, other, activator);

	// fire off and cine or ai scripts attached to this entity
	if( hook->pCineScript || hook->pAiScript )
	{
		com_PlayEntityScript(self, hook->pCineScript, hook->pAiScript);
	}

	// if we've used it and it's not toggleable.. nuke the use function
	if (!(self->spawnflags & FUNCWALL_TOGGLE))
	{
		self->use = NULL;
		// FIXME: if it's currently off, should we remove it from the world???
/*
		if (self->solid == SOLID_NOT)
		{
			// remove this entity from the world now
			self->remove (self);
		}
*/
	}
	gstate->LinkEntity( self );
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_die
///////////////////////////////////////////////////////////////////////////////

void	func_wall_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	
	// use it since it's going away
	func_wall_use (self, attacker, attacker);

	// remove this entity from the world now
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall
///////////////////////////////////////////////////////////////////////////////

void	func_wall (userEntity_t *self)
{
	int			i;
	doorHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	self->movedir.Zero();

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "health") == 0)
			self->health = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;

		i++;
	}

	// animate the textures different if these flags are set
	if (self->spawnflags & FUNCWALL_ANIM_ALL)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & FUNCWALL_ANIM_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->s.angles.Zero();

	// set up the physics
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;

	if (self->spawnflags & FUNCWALL_NOT_SOLID)
		self->solid = SOLID_NOT;

	// just a wall
	if ((self->spawnflags & (FUNCWALL_TRIGGER_SPAWN | FUNCWALL_TOGGLE | FUNCWALL_START_ON)) == 0)
	{
		// set up the model and put it in the world
		gstate->SetModel (self, self->modelName);
		gstate->LinkEntity (self);
		return;
	}

	//NSS[11/28/99]:If this wall is CTF specific but we are not in CTF mode
	//Then free this wall.
	if( (self->spawnflags & FUNCWALL_CTF) &&!((int)gstate->GetCvar("ctf")))
	{
		gstate->RemoveEntity(self);
		return;
	}
	
	
	// yell if the spawnflags are odd
	if (self->spawnflags & FUNCWALL_START_ON)
	{
		if (!(self->spawnflags & FUNCWALL_TOGGLE))
		{
			gstate->Con_Dprintf ("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= FUNCWALL_TOGGLE;
		}
	}

	// if the wall isn't starting ON, make it SOLID_NOT
	if (!(self->spawnflags & FUNCWALL_START_ON))
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	// set up the use functions
	self->use = func_wall_use;
	self->die = func_wall_die;

	// set up the model and put it in the world
	gstate->SetModel (self, self->modelName);

	// if this has a parent, initialize the parent/child link
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.3;
	}

}

/* ----------------------------- func_train ------------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	train_blocked
///////////////////////////////////////////////////////////////////////////////

void	train_blocked (userEntity_t *self, userEntity_t *other)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (gstate->time < hook->move_finished)
		return;

	hook->move_finished = gstate->time + 0.5;

	client_deathtype = CRUSHED;

	// NSS[1/6/00]:Always do damage to anything blocking if we have damage applied.
	if(hook->damage)
		com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
}

///////////////////////////////////////////////////////////////////////////////
//	train_use
///////////////////////////////////////////////////////////////////////////////

void	train_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if( hook->state == TRAIN_STATE_MOVING )
	{
		return;
	}
	if (com->CheckTargetForActiveChildren (self, other, activator) == TRUE)
		return;

	if (!self->goalentity) 
		return;

	if ((self->think != train_find) && !(self->goalentity->spawnflags & WAITFORTRIGGER) && !self->targetname)
		return;

	if ((self->goalentity->spawnflags & WAITFORTRIGGER) && self->think != NULL)
		return;

	if( hook->sound_bottom )
	{
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_bottom, self->s.volume, self->s.dist_min, self->s.dist_max);
	}

	train_next (self);
}

///////////////////////////////////////////////////////////////////////////////
//	train_check_rotation
///////////////////////////////////////////////////////////////////////////////

void	train_check_rotation (userEntity_t *train, userEntity_t *path_corner, int check_targetname)
{
	doorHook_t	*hook = (doorHook_t *) train->userHook;
	doorHook_t	*phook = (doorHook_t *) path_corner->userHook;
	int			go_to_think;
	float		sign, last_think_time, move_time, shortest_rotation;

	go_to_think = FALSE;
	last_think_time = train->nextthink;

	//////////////////////////////////////////////
	// start a continual rotation about the axis
	//////////////////////////////////////////////
	if (path_corner->spawnflags & X_AXIS)
		train->avelocity.z = phook->x_speed;
	else
		train->avelocity.z = 0;

	if (path_corner->spawnflags & Y_AXIS)
		train->avelocity.x = phook->y_speed;
	else
		train->avelocity.x = 0;

	if (path_corner->spawnflags & Z_AXIS)
		train->avelocity.y = phook->z_speed;
	else
		train->avelocity.y = 0;

	phook->end_x_rotation_time = phook->end_y_rotation_time = phook->end_z_rotation_time = 0;
	shortest_rotation = 0;

	///////////////////////////////////////////////////
	// check for a rotation around x axis
	//
	// x_distance is degrees to rotate on x axis
	// x_speed is speed degrees per second to rotate
	///////////////////////////////////////////////////

	if (phook->x_distance)
	{
		// if no x_speed is specified, and there is movement from one path to the next
		// then calculate the angular velocity to complete rotation when the move completes
		if (phook->x_speed == 0 && hook->end_move_time > gstate->time)
		{
			move_time = hook->end_move_time - gstate->time;
			phook->x_speed = phook->x_distance / move_time;
		}
		else if (phook->x_speed == 0) phook->x_speed = 90.0;

		// find time from start rotation to end of rotation (in bmodels local time)
		hook->end_x_rotation_time = gstate->time + (fabs (phook->x_distance) / phook->x_speed);
		last_think_time = hook->end_x_rotation_time;
		if (hook->end_x_rotation_time < shortest_rotation || shortest_rotation == 0)
			shortest_rotation = hook->end_x_rotation_time;

		if (phook->x_distance > 0) 
		{
			sign = 1; 
		}
		else 
		{
			sign = -1;
		}

		if (phook->x_speed > 0) 
		{
			sign *= 1; 
		}
		else 
		{
			sign *= -1;
		}
		train->avelocity.z = phook->x_speed * sign;

		// final angle after rotation
		hook->end_x_angle = AngleMod (train->s.angles.z + phook->x_distance);
		hook->x_distance = phook->x_distance;

		if ((!train->targetname && check_targetname) || (!check_targetname))
			go_to_think = TRUE;
	}
	else hook->x_distance = 0;

	///////////////////////////////////////////////////
	// check for a rotation around y axis
	//
	// y_distance is degrees to rotate on y axis
	// y_speed is speed degrees per second to rotate
	///////////////////////////////////////////////////

	if (phook->y_distance)
	{
		// if no y_speed is specified, and there is movement from one path to the next
		// then calculate the angular velocity to complete rotation when the move completes
		if (phook->y_speed == 0 && hook->end_move_time > gstate->time)
		{
			move_time = hook->end_move_time - gstate->time;
			phook->y_speed = phook->y_distance / move_time;
		}
		else if (phook->y_speed == 0) phook->y_speed = 90.0;

		hook->end_y_rotation_time = gstate->time + (fabs (phook->y_distance) / phook->y_speed);
		if (hook->end_y_rotation_time > last_think_time)
			last_think_time = hook->end_y_rotation_time;
		if (hook->end_y_rotation_time < shortest_rotation || shortest_rotation == 0)
			shortest_rotation = hook->end_y_rotation_time;

		if (phook->y_distance > 0) 
		{
			sign = 1; 
		}
		else 
		{
			sign = -1;
		}
		
		if (phook->y_speed > 0) 
		{
			sign *= 1; 
		}
		else 
		{
			sign *= -1;
		}
		train->avelocity.x = phook->y_speed * sign;

		hook->end_y_angle = AngleMod(train->s.angles.x + phook->y_distance);
		hook->y_distance = phook->y_distance;
		if ((!train->targetname && check_targetname) || (!check_targetname))
		{
			go_to_think = TRUE;
		}
	}
	else 
	{
		hook->y_distance = 0;
	}

	///////////////////////////////////////////////////
	// check for a rotation around z axis
	//
	// z_distance is degrees to rotate on z axis
	// z_speed is speed degrees per second to rotate
	///////////////////////////////////////////////////

	if (phook->z_distance)
	{
		// if no z_speed is specified, and there is movement from one path to the next
		// then calculate the angular velocity to complete rotation when the move completes
		if (phook->z_speed == 0 && hook->end_move_time > gstate->time)
		{
			move_time = hook->end_move_time - gstate->time;
			phook->z_speed = phook->z_distance / move_time;
		}
		else if (phook->z_speed == 0) phook->z_speed = 90.0;

		hook->end_z_rotation_time = gstate->time + (fabs (phook->z_distance) / fabs (phook->z_speed));
		if (hook->end_z_rotation_time > last_think_time)
			last_think_time = hook->end_z_rotation_time;
		if (hook->end_z_rotation_time < shortest_rotation || shortest_rotation == 0)
			shortest_rotation = hook->end_z_rotation_time;

		if (phook->z_distance > 0) 
		{
			sign = 1; 
		}
		else 
		{
			sign = -1;
		}

		if (phook->z_speed > 0) 
		{
			sign *= 1; 
		}
		else 
		{
			sign *= -1;
		}
		train->avelocity.y = phook->z_speed * sign;

		hook->end_z_angle = AngleMod (train->s.angles.y + phook->z_distance);

		hook->z_distance = phook->z_distance;
		if ((!train->targetname && check_targetname) || (!check_targetname))
		{
			go_to_think = TRUE;
		}
	}
	else 
	{
		hook->z_distance = 0;
	}

	//	make sure that if the end of movement is the first thing to occur that it
	//	gets done at the right time
	if (hook->end_move_time < shortest_rotation && hook->end_move_time > gstate->time)
	{
		shortest_rotation = hook->end_move_time;
	}

	if (go_to_think)
	{
		train->nextthink = shortest_rotation;
		train->think = train_rotate_think;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	train_die
///////////////////////////////////////////////////////////////////////////////

void	train_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	CVector	org;
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	//	Q2FIXME:	attacker == other???
	com->UseTargets (self->goalentity, attacker, attacker);

	org = self->goalentity->s.origin;
	com_CalcMove (self, org, hook->speed, train_wait);

	train_check_rotation (self, self->goalentity, FALSE);
}

///////////////////////////////////////////////////////////////////////////////
//	train_next
//
//	find the next path_corner for func_train self
//	Use the current path_corner's values for control, etc.
//	the next path_corner should only be used as a destination origin until it is reached
///////////////////////////////////////////////////////////////////////////////

void	train_next (userEntity_t *self)
{
	userEntity_t	*targ;
	userEntity_t	*cur_path = self->goalentity;
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*path_hook = (doorHook_t *) cur_path->userHook;
	doorHook_t		*targ_hook;
	CVector			org, temp;

	if( path_hook == NULL )
	{
		return;
	}

	hook->state = TRAIN_STATE_MOVING;

	// when hitting the end of the path, play the stop sound
	if (path_hook->sound_down)
	{
		gstate->StartEntitySound(self, CHAN_OVERRIDE, path_hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max);
		self->s.sound = 0;
	}

	// reached the end of the path, nothing else to do
	if (hook->wait == -1)
		return;

	if (cur_path->target)
	{
		targ = gstate->FirstEntity ();

		while (targ)
		{
			if (targ->targetname)
			{
				if (stricmp (cur_path->target, targ->targetname) == 0)
				{
					if (targ->className)
					{
						if (stricmp (targ->className, "path_corner_train") == 0)
							break;
					}
				}
			}

			targ = gstate->NextEntity (targ);
		}

		if (!targ)
		{
			return;
		}

		targ_hook = (doorHook_t *) targ->userHook;

		// store for future use
		self->goalentity = targ;
	}
	else
		targ = NULL;

	if (path_hook->wait)
		hook->wait = path_hook->wait;
	else
		hook->wait = 0;

	// play start sound from path corner
// SCG[2/28/00]: um, this is redundant
//	if (path_hook->sound_up)
//		gstate->StartEntitySound(self, CHAN_OVERRIDE, path_hook->sound_up, self->s.volume, self->s.dist_min, self->s.dist_max);

	// setup sound on train
	if( targ != NULL )
	{
		self->s.sound = hook->sound_up;
	}

	// set the func_train's speed to the current path_corner's speed
	if (path_hook->speed)
		hook->speed = path_hook->speed;

	// make func_train's delay == path_corner's
	self->delay = cur_path->delay;

	// clear rotation flags on func_train
	if (self->spawnflags & X_AXIS)
		self->spawnflags &= ~X_AXIS;
	if (self->spawnflags & Y_AXIS)
		self->spawnflags &= ~Y_AXIS;
	if (self->spawnflags & Z_AXIS)
		self->spawnflags &= ~Z_AXIS;

	// set func_train's rotation flags to path_corner's
	if (cur_path->spawnflags & X_AXIS)
		self->spawnflags |= X_AXIS;
	if (cur_path->spawnflags & Y_AXIS)
		self->spawnflags |= Y_AXIS;
	if (cur_path->spawnflags & Z_AXIS)
		self->spawnflags |= Z_AXIS;

	// the the current path_corner has a health, then set it up
	// so that it must be killed to activate
	if (cur_path->health)
	{
		self->health = targ->health;
		self->die = train_die;
		self->think = NULL;
		self->takedamage = DAMAGE_YES;
	}
	// otherwise, start the train moving
	else
	{
		// can't kill a train while it's moving
		self->takedamage = DAMAGE_NO;

		// use the current path_corner's targets (kill targets)
		//	Q2FIXME:	activator == self?
		com->UseTargets (cur_path, self, self);

		if (!self->inuse)
			//	path might have killtargeted this train
			return;

		// if current path_corner has a target, then move to it
		if (targ)
		{
			// if we're teleporting the train to the next path_corner_train, do so now
			if (cur_path->spawnflags & TELEPORT)
			{
				trace_t	tr;

				hook->done_think = train_wait;		// for compatibility
				hook->final_dest = targ->s.origin;
				// flag this train as transporting
				self->s.event = EV_ENTITY_FORCEMOVE;
				// reposition the train at the new target
				com_CalcAbsolutePosition (self);	
				// make sure there are no players standing in the way
				tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_PLAYERSOLID);
				if (tr.allsolid && tr.startsolid && tr.ent && tr.ent->solid != SOLID_BSP && tr.ent != &gstate->g_edicts [0])
					com->Damage (tr.ent, self, self, self->s.origin, zero_vector, tr.ent->health+100, DAMAGE_TELEFRAG);
			}
			else
			{
				org = targ->s.origin;
				com_CalcMove (self, org, hook->speed, train_wait);  //&&& FIXME - does this train_wait cause the pause?????
			}
		}

		// check rotation even if there is no target because rotation
		// doesn't require a target path_corner
		train_check_rotation (self, cur_path, FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	train_die_next
///////////////////////////////////////////////////////////////////////////////

void	train_die_next (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	train_next (self);
}

///////////////////////////////////////////////////////////////////////////////
//	train_wait
///////////////////////////////////////////////////////////////////////////////

void	train_wait (userEntity_t *self)
{
	userEntity_t	*cur_path = self->goalentity;
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*path_hook = (doorHook_t *) cur_path->userHook;

	// fire off and cine or ai scripts attached to this entity
	if( hook->pCineScript!= NULL && hook->pAiScript != NULL )
	{
		com_PlayEntityScript(self->goalentity, hook->pCineScript, hook->pAiScript);
	}

	// if the path_corner the train is coming from has a wait
	// on it, then wait that amount of time before going to
	// the next path_corner
	if (hook->wait > 0)
	{
		self->avelocity.Zero();
		self->nextthink = gstate->time + hook->wait;
		self->think = train_next;
		// NSS[12/13/99]:Stop playing the freakin' sound
		self->s.sound = 0;
		hook->state = TRAIN_STATE_NOT_MOVING;
		return;
	}
	else if (self->goalentity->spawnflags & WAITFORTRIGGER)
	{
		// reassign the train's target to next path_corner
		self->target = self->goalentity->target;
		self->think = NULL;
		// the train has stopped, so stop the sound
		self->s.sound = 0;

		// when hitting the end of the path, play the stop sound
		if (path_hook->sound_down)
			gstate->StartEntitySound(self, CHAN_OVERRIDE, path_hook->sound_down, self->s.volume, self->s.dist_min, self->s.dist_max);
		hook->state = TRAIN_STATE_NOT_MOVING;
		return;
	}
	else if (hook->wait == -1)
	{
		self->nextthink = -1;
		self->velocity.Zero();
		self->avelocity.Zero();
		// the train has stopped, so stop the sound
	
		self->s.sound = 0;
		hook->state = TRAIN_STATE_NOT_MOVING;
		return;
	}

	// SCG[8/3/99]: If misctarget is set, call it's use function...
	if( path_hook != NULL )
	{
		if( path_hook->misctarget != NULL )
		{
			userEntity_t	*target_entity = gstate->FirstEntity ();

			while( target_entity )
			{
				if( target_entity->targetname )
				{
					if( stricmp( path_hook->misctarget, target_entity->targetname ) == 0 )
					{
						if( target_entity->use != NULL )
						{
							target_entity->use( target_entity, self, self );
						}
						break;
					}
				}

				target_entity = gstate->NextEntity( target_entity );
			}
		}
	}

	train_next (self);
}

///////////////////////////////////////////////////////////////////////////////
//	train_rotate_think
///////////////////////////////////////////////////////////////////////////////

void	train_rotate_think (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	float			next_think;
	if( hook == NULL )
	{
		self->think = NULL;
		return;
	}
/*
	if (gstate->time >= hook->end_x_rotation_time && !(self->spawnflags & X_AXIS) &&
		hook->x_distance != 0)
	{
		self->avelocity.z = 0;
		self->s.angles.z = hook->end_x_angle;
		hook->end_x_rotation_time = 0;
	}

	if (gstate->time >= hook->end_y_rotation_time && !(self->spawnflags & Y_AXIS) &&
		hook->y_distance != 0)
	{
		self->avelocity.x = 0;
		self->s.angles.x = hook->end_y_angle;
		hook->end_y_rotation_time = 0;
	}

	if (gstate->time >= hook->end_z_rotation_time && !(self->spawnflags & Z_AXIS) &&
		hook->z_distance != 0)
	{
		self->avelocity.y = 0;
		self->s.angles.y = hook->end_z_angle;
		hook->end_z_rotation_time = 0;
	}
*/
	if (gstate->time >= hook->end_x_rotation_time && hook->x_distance != 0 )
	{
		self->avelocity.z = 0;
		self->s.angles.z = hook->end_x_angle;
		hook->end_z_rotation_time = 0;
	}

	if (gstate->time >= hook->end_y_rotation_time && hook->y_distance != 0 )
	{
		self->avelocity.x = 0;
		self->s.angles.x = hook->end_y_angle;
		hook->end_z_rotation_time = 0;
	}

	if (gstate->time >= hook->end_z_rotation_time && hook->z_distance != 0 )
	{
		self->avelocity.y = 0;
		self->s.angles.y = hook->end_z_angle;
		hook->end_z_rotation_time = 0;
	}

	if (gstate->time >= hook->end_move_time)
	{
		gstate->SetOrigin (self, hook->final_dest.x, hook->final_dest.y, hook->final_dest.z);
		self->velocity.Zero();

		//&&& AMW 6.26.98 - re-orient the linked entities also so they end up in
		// the right place
		if (self->teamchain && !(self->flags & FL_TEAMSLAVE))
		{
			// update the positions of the child entities
			com_CalcChildPositions (self);
		}
	}

	next_think = hook->end_x_rotation_time;

	if (next_think < hook->end_y_rotation_time)
		next_think = hook->end_y_rotation_time;
	if (next_think < hook->end_z_rotation_time)
		next_think = hook->end_z_rotation_time;
	if (next_think < hook->end_move_time)
		next_think = hook->end_move_time;

	self->nextthink = next_think;
	self->think = train_rotate_think;

	// if all movement and rotation is complete, go to train_wait
	if (gstate->time >= hook->end_move_time && gstate->time >= hook->end_x_rotation_time &&
		gstate->time >= hook->end_y_rotation_time && gstate->time >= hook->end_z_rotation_time)
		train_wait (self);
}

///////////////////////////////////////////////////////////////////////////////
//	train_find
//
//	find the first path corner that the train points to and
//	move the train's origin to it.  Point self->goalentity to
//	the current path_corner
//
//	if the train is not targeted, then go ahead and call train_next
//	to use the current path_corner and move to the next
///////////////////////////////////////////////////////////////////////////////

void train_find (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*thook;
	userEntity_t	*targ;
	CVector			org;

	if (!self->target)
		return;

	targ = gstate->FirstEntity ();

	while (targ)
	{
		if (targ->targetname)
		{
			if (stricmp (self->target, targ->targetname) == 0)
			{
				if (targ->className)
				{
					if (stricmp (targ->className, "path_corner_train") == 0)
						break;
				}
			}
		}

		targ = gstate->NextEntity (targ);
	}

	if (!targ)
	{
		gstate->Con_Printf ("func_train at %s has no first target\n", com->vtos(self->s.origin));
		self->remove(self);
		return;
	}
	// save the original spawn origin of the train because all child entities calculate their
	// offsets from it


	// point to path_corner for future reference
	self->goalentity = targ;
	gstate->SetOrigin2 (self, targ->s.origin);

	thook = (doorHook_t *) targ->userHook;

	self->delay = targ->delay;

	if (targ->health)
	{
		self->health = targ->health;
		self->die = train_die_next;
		self->takedamage = DAMAGE_YES;
		self->think = NULL;
	}

	if( (!self->targetname && !( targ->spawnflags & WAITFORTRIGGER ) ) || 
		self->targetname && self->health || 
		( self->spawnflags & FUNC_TRAIN_START_ON ) )
	{
		train_next (self);

		if( hook->sound_bottom )
		{
			gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_bottom, self->s.volume, self->s.dist_min, self->s.dist_max);
		}

		train_check_rotation (self, targ, TRUE);
	}
	else
	{
		self->think = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	func_train
///////////////////////////////////////////////////////////////////////////////

void	func_train (userEntity_t *self)
{
	int			i;
	doorHook_t	*hook;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	self->movedir.Zero();
	self->health = 0;

	hook->sound_up = NULL;
	hook->sound_down = NULL;
	hook->sound_top = NULL;
	hook->sound_bottom = NULL;

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		//gstate->Con_Printf ("key = %s, value = %s\n", self->epair [i].key, self->epair [i].value);
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (!stricmp (self->epair [i].key, "dmg"))
			hook->damage = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "speed"))
			hook->speed = atof (self->epair [i].value);
//		else if (!stricmp (self->epair [i].key, "targetname"))
//			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "x_distance"))
			hook->x_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "y_distance"))
			hook->y_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "z_distance"))
			hook->z_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "x_speed"))
			hook->x_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "y_speed"))
			hook->y_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "z_speed"))
			hook->z_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			hook->sound_up = gstate->SoundIndex (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound_move"))
			hook->sound_bottom = gstate->SoundIndex( self->epair [i].value );
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

	// if this train must move regardless of what is blocking it.. i.e. an escalator
	// then let the physics system do special checks for it...
	if (self->spawnflags & FORCEMOVE)
	{
		self->flags |= FL_FORCEMOVE;
		self->blocked = train_blocked;
	}

	if (!hook->speed)
		hook->speed = 100;
	if (!self->target)
		com->Warning ("func_train %s has no target\n", self->targetname);
	if (!hook->damage)
		hook->damage = 10;

	hook->count = 1;
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	self->blocked = train_blocked;
	self->takedamage = DAMAGE_NO;
	self->use = train_use;

	gstate->SetModel (self, self->modelName);
	gstate->SetOrigin2 (self, self->s.origin);

	// this is a special case where it's an "invisible" train without a model
	// usually the case if it's just an origin brush
	if (self->s.modelindex == 0)
	{
		// if this is the case then we don't want it to collide with something
		self->solid = SOLID_NOT;
	}

	//&&& AMW 6.16.98 - save the original position of the train before it moves to it's first target
	self->spawn_origin = self->s.origin;

	// start trains on the second frame, to make sure their targets
	// have been spawned

	self->nextthink = gstate->time + 0.1;
	self->think = train_find;
}

///////////////////////////////////////////////////////////////////////////////
//	path_corner_train
///////////////////////////////////////////////////////////////////////////////

void	path_corner_train (userEntity_t *self)
{
	doorHook_t		*hook;
	int				i;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;
	
	memset( hook, 0, sizeof( doorHook_t ) );

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "delay") == 0)
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
		{
			hook->sound_up = gstate->SoundIndex (self->epair [i].value);
		}
		else if (!stricmp (self->epair [i].key, "sound_start"))
		{
			hook->sound_up = gstate->SoundIndex (self->epair [i].value);
		}
		else if (!stricmp (self->epair [i].key, "sound_stop"))
		{
			hook->sound_down = gstate->SoundIndex (self->epair [i].value);
		}
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "health") == 0)
			self->health = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "x_distance"))
			hook->x_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "y_distance"))
			hook->y_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "z_distance"))
			hook->z_distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "x_speed"))
			hook->x_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "y_speed"))
			hook->y_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "z_speed"))
			hook->z_speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "pathtarget"))
			hook->misctarget = self->epair [i].value;

		i++;
	}

	self->flags = self->flags & FL_IGNOREUSETARGET;
}

///////////////////////////////////////////////////////////////
//	func_button routines
///////////////////////////////////////////////////////////////

void	button_return (userEntity_t *self);

///////////////////////
//	button_wait
///////////////////////

void	button_wait (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	// amw: wait of -1 means button does not return
	if (hook->wait == -1)
	{
		self->think = NULL;
		// if this button has a parent, we need to reset the childOffset variables 
		// because the button is staying here
		if (self->teammaster)
		{
			userEntity_t *parent = self->teammaster;
			
			// re-initialize all the offsets from the parent
			self->childOffset = self->s.origin - parent->s.origin;
			self->transformedOffset = self->childOffset;
			self->s.angle_delta = self->s.angles - parent->s.angles;
		}

		return;
	}

	hook->state = STATE_TOP;
	self->nextthink = gstate->time + hook->wait;
	self->think = button_return;

	//	Q2: now done in button_fire
	//	com->UseTargets (self, other, activator);

	self->s.frame = 1;
}

///////////////////////
//	button_done
///////////////////////

void	button_done (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	hook->state = STATE_BOTTOM;
}

///////////////////////
//	button_return
///////////////////////

void	button_return (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	com_CalcMove (self, hook->pos1, hook->speed, button_done);

	if (hook->sound_bottom)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_bottom, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	self->s.frame = 0;

	if (self->health)
		self->takedamage = DAMAGE_YES;
}

///////////////////////
//	button_blocked
///////////////////////

void	button_blocked (userEntity_t *self, userEntity_t *other)
{
}

///////////////////////
//	button_fire
///////////////////////

void	button_fire (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if  (hook->state == STATE_UP || hook->state == STATE_TOP)
		return;

	if (hook->sound_top)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top,0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	hook->state = STATE_UP;

	//&&& AMW 6.25.98 - recalculate the button offsets if the parent has rotated etc
	if (self->teammaster)
	{
		CVector new_org;

		CVector adjOrigin;  // in case the door has collided or something
		adjOrigin = self->teammaster->s.origin + self->transformedOffset;

		// reset pos1 for button_return()  .. it's always the current origin of the button
		hook->pos1 = adjOrigin;

		com->TransformChildOffset( adjOrigin, hook->pos2_offs, self->teammaster->s.angles, TRUE, new_org );

		// update the child origin
		hook->pos2 = new_org;

	}
	com_CalcMove (self, hook->pos2, hook->speed, button_wait);

	//&&& AMW 7/7/98 - moved here so that the button has velocity etc before it triggers it's target
	// helps to prevent trains from moving before their children have finished moving

	//	Q2:	use targets on button activation, not button wait
	com->UseTargets (self, other, activator);

	// fire off and cine or ai scripts attached to this entity
	if( hook->pCineScript!= NULL && hook->pAiScript != NULL )
	{
		com_PlayEntityScript(activator, hook->pCineScript, hook->pAiScript);
	}
}



///////////////////////
//	button_use
///////////////////////

//	Q2FIXME:	activator?!?!?

void	button_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	hook->activator = gstate->activator;
	if( hook->activator == NULL )
	{
		hook->activator = activator;
	}

	//&&& AMW 6.12.98 - temporarily disable this button if it's a child and it's moving
	if (self->flags & FL_TEAMSLAVE)
	{
		userEntity_t *parent = self->teammaster;
		if (parent &&
			parent->velocity.x  || parent->velocity.y  || parent->velocity.z ||
			parent->avelocity.x || parent->avelocity.y || parent->avelocity.z)
		{
			return;
		}
	}
	
	button_fire (self, other, activator);
}

///////////////////////
//	button_touch
///////////////////////

void	button_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (!com->ValidTouch (self, other))
		return;

	hook->activator = other;

	if( com_DoKeyFunctions( self, other, tongue_world[T_MUST_HAVE_KEY] ) )
	{
		button_fire (self, other, other);
	}
}

///////////////////////
//	button_killed
///////////////////////

void	button_killed (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;
		
	hook->activator = gstate->attacker;
	self->health = hook->base_health;
	self->takedamage = DAMAGE_NO;

	//	Q2FIXME:	other == attacker ?
	button_fire (self, attacker, attacker);
}

///////////////////////
//	func_button
///////////////////////

void	func_button (userEntity_t *self)
{
	doorHook_t	*hook;
	CVector			temp;
	int				i;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	hook->bStruggle = false;

	temp.Set(0.0, 1.0, 0.0);
	self->movedir = temp;
	hook->lip = 4;

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	hook->speed = 40;
	while (self->epair [i].key != NULL)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (stricmp (self->epair [i].key, "health") == 0)
			self->health = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "nodetarget") == 0) // this button is linked to a node
			hook->nodetarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lip"))	//&&& AMW 5.25.98 - lip value didn't work
			hook->lip = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "sound_use") == 0)
		{
			hook->sound_top = gstate->SoundIndex (self->epair [i].value);
		}	
		else if (stricmp (self->epair [i].key, "sound_return") == 0)
		{
			hook->sound_bottom = gstate->SoundIndex (self->epair [i].value);
		}
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;


		i++;
	}

	com->SetMovedir (self);

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);
	gstate->SetModel (self, self->modelName);
	self->className = "func_button";

	self->blocked = button_blocked;
	self->use = button_use;

	// if the button has health, we don't want to be able to use it
	if (self->health)
	{
		hook->base_health = self->health;
		self->die = button_killed;
		self->takedamage = DAMAGE_YES;
		self->use = NULL;
	}

	//	FIXME: touching buttons no longer activates them, they must be used
	else if ((int)self->spawnflags & PUSH_TOUCH)
		self->touch = button_touch;
	else
		self->touch = NULL;

	if (!hook->speed)
		hook->speed = 40;
	if (!hook->wait)
		hook->wait = 1;

	hook->state = STATE_BOTTOM;

	// start position
	hook->pos1 = self->s.origin;

	// find ending position
	hook->pos2 = hook->pos1 + self->movedir * (fabs(DotProduct( self->movedir, self->size)) - hook->lip);

	// save the original offsets from the origin of the entity
	hook->pos1_offs = hook->pos1 - self->s.origin;
	hook->pos2_offs = hook->pos2 - self->s.origin;

	//&&& AMW 6.12.98 - if this button has a parent, it must be initialized after all the other entities have also
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.5;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	// SCG[11/5/99]: added flag for struggle sound
	if( self->spawnflags & FUNC_BUTTON_STRUGGLE )
	{
		hook->bStruggle = true;
	}
}


/////////////////////////////////////////////////////////
/// Secret X@!%^%!@$ doors
/////////////////////////////////////////////////////////

#define	SECRET_OPEN_ONCE	1	// stays open
#define	SECRET_1ST_LEFT		2	// 1st move is left of arrow
#define	SECRET_1ST_DOWN		4	// 1st move is down from arrow
#define	SECRET_NO_SHOOT		8	// only opened by trigger
#define	SECRET_YES_SHOOT	16	// shootable even if targeted

void	secret_move1 (	userEntity_t *self);
void	secret_move2 (	userEntity_t *self);
void	secret_move3 (	userEntity_t *self);
void	secret_move4 (	userEntity_t *self);
void	secret_move5 (	userEntity_t *self);
void	secret_move6 (	userEntity_t *self);
void	secret_done (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	secret_use
///////////////////////////////////////////////////////////////////////////////

//	Q2FIXME: activator?!?!?

void	secret_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	float			temp;
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	self->health = 10000;

	if (self->s.origin != hook->old_origin)
		return;

	self->message = NULL;

	com->UseTargets (self, other, activator);

	if (!((int)self->spawnflags & SECRET_NO_SHOOT))
	{
		self->pain = NULL;
		self->takedamage = DAMAGE_NO;
	}

	self->velocity.Zero();

	if (hook->sound_up)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_up, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	
	self->nextthink = gstate->time + 0.1;

	temp = 1 - ((int)self->spawnflags & SECRET_1ST_LEFT);
	hook->mangle.AngleToVectors(forward, right, up);
	
	if (!hook->t_width)
	{
		if ((int)self->spawnflags & SECRET_1ST_DOWN)
			hook->t_width = fabs( DotProduct( up, self->size ) );
		else
			hook->t_width = fabs( DotProduct( right, self->size ) );
	}

	if (!hook->t_length)
		hook->t_length = fabs( DotProduct( forward, self->size ) );

	if ((int)self->spawnflags & SECRET_1ST_DOWN)
		hook->pos1 = self->s.origin - up * hook->t_width;
	else
		hook->pos1 = self->s.origin + right * (hook->t_width * temp);

	hook->pos2 = hook->pos1 + forward * hook->t_length;

	com_CalcMove (self, hook->pos1, hook->speed, secret_move1);

	if (hook->sound_top)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top,0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_pain
///////////////////////////////////////////////////////////////////////////////

void	secret_pain (userEntity_t *self, userEntity_t *other, float kick, int damage)
{
	secret_use (self, other, other);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_die
///////////////////////////////////////////////////////////////////////////////

void	secret_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	secret_use (self, attacker, attacker);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move1
//
//	wait after first movement
///////////////////////////////////////////////////////////////////////////////

void	secret_move1 (	userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	self->nextthink = gstate->time + 1.0;
	self->think = secret_move2;

	if (hook->sound_down)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down,0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move2
//
//	start moving sideways w/sound  
///////////////////////////////////////////////////////////////////////////////

void	secret_move2 (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	
	if (hook->sound_top)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	com_CalcMove (self, hook->pos2, hook->speed, secret_move3);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move3
//
//	wait here until time to go back
///////////////////////////////////////////////////////////////////////////////

void	secret_move3 (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (hook->sound_down)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	if (!((int)self->spawnflags & SECRET_OPEN_ONCE) && hook->wait >= 0)
	{
		self->nextthink = gstate->time + hook->wait;
		self->think = secret_move4;
	}
	else
	{
		self->nextthink = -1;
		//wait until hell freezes over
	}
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move4
//
//	move backwards
///////////////////////////////////////////////////////////////////////////////

void	secret_move4 (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (hook->sound_top)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	com_CalcMove (self, hook->pos1, hook->speed, secret_move5);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move5
//
//	wait 1 second, please
///////////////////////////////////////////////////////////////////////////////

void	secret_move5 (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (hook->sound_down)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down,0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	self->nextthink = gstate->time + 1.0;
	self->think = secret_move6;
}

///////////////////////////////////////////////////////////////////////////////
//	secret_move6
//
//	ach, ah, bleh, ugh, ich, SMACK!
///////////////////////////////////////////////////////////////////////////////

void	secret_move6 (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (hook->sound_top)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_top, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	com_CalcMove (self, hook->old_origin, hook->speed, secret_done);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_done
//
//	please let this end  
///////////////////////////////////////////////////////////////////////////////

void	secret_done (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (!self->targetname || (int)self->spawnflags & SECRET_YES_SHOOT)
	{
		self->health = 10000;
		self->takedamage = DAMAGE_YES;
		self->pain = secret_pain;
	}

	if (hook->sound_down)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->sound_down, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_blocked
///////////////////////////////////////////////////////////////////////////////

void	secret_blocked (userEntity_t *self, userEntity_t *other)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	
	if (gstate->time < hook->move_finished)
		return;

	hook->move_finished = gstate->time + 0.5;

	com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
}

///////////////////////////////////////////////////////////////////////////////
//	secret_touch
///////////////////////////////////////////////////////////////////////////////

void	secret_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;

	if (!com->ValidTouch (self, other))
		return;
	if (hook->move_finished > gstate->time)
		return;

	hook->move_finished = gstate->time + 2.0;

	if (self->message && other->flags & FL_CLIENT)
		gstate->centerprintf (other, self->message);
}

///////////////////////////////////////////////////////////////////////////////
//	func_door_secret
///////////////////////////////////////////////////////////////////////////////

void	func_door_secret (userEntity_t *self)
{
	CVector		temp;
	doorHook_t	*hook;
	int			i;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;
	
	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "dmg") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		i++;
	}

	if (!hook->damage)
		hook->damage = 2;

	door_parse_sounds (self);

	hook->mangle = self->s.angles;
	self->s.angles.Zero();
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	self->className = "func_door";
	gstate->SetModel (self, self->modelName);
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);

	self->touch = secret_touch;
	self->blocked = secret_blocked;
	hook->speed = 50;
	self->use = secret_use;

	if (!self->targetname || (int)self->spawnflags & SECRET_YES_SHOOT)
	{
		self->health = 10000;
		self->takedamage = DAMAGE_YES;
		self->pain = secret_pain;
		self->die = secret_die;
	}

	hook->old_origin = self->s.origin;

	if (!hook->wait)
		hook->wait = 5;

	//&&& AMW 6.12.98 - if this button has a parent, it must be initialized after all the other entities have also
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.5;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}
}

///////////////////////////////////////////////////////////////
//	func_multi_button routines
///////////////////////////////////////////////////////////////

void	multi_button_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator);

///////////////////////
//	multi_button_wait
///////////////////////

void	multi_button_wait (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (hook->count == hook->max_count - 1)
		hook->state = STATE_BOTTOM;
	else if (hook->count == 0)
		hook->state = STATE_TOP;

	//	Q2:	now done in multi_button_fire
	//	com->UseTargets (self, other, activator);

	self->s.frame = 1;
	self->use = multi_button_use;
}

///////////////////////
//	multi_button_blocked
///////////////////////

void	multi_button_blocked (userEntity_t *self, userEntity_t *other)
{
}

///////////////////////
//	multi_button_fire
///////////////////////

void	multi_button_fire (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (hook->state == STATE_DOWN || hook->state == STATE_TOP)
		hook->count++;
	else
		hook->count--;

	gstate->Con_Printf ("multi_button count = %i\n", hook->count);

	hook->pos2 = hook->pos1 + self->movedir * hook->distance * hook->count;

	//	Q2: was done in multi_button_wait
	com->UseTargets (self, other, activator);

	com_CalcMove (self, hook->pos2, hook->speed, multi_button_wait);
}

///////////////////////
//	multi_button_use
///////////////////////

void	multi_button_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	hook->activator = gstate->activator;
	self->use = NULL;

	multi_button_fire (self, other, activator);
}

///////////////////////
//	multi_button_touch
///////////////////////

void	multi_button_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (!com->ValidTouch (self, other))
		return;

	hook->activator = other;

	multi_button_fire (self, other, other);
}

///////////////////////
//	multi_button_killed
///////////////////////

void	multi_button_killed (userEntity_t *self, userEntity_t *infictor, userEntity_t *attacker, int damage, CVector &point)
{
	//&&& FIXME: amw - this appears to be fucked up
	
	doorHook_t	*hook = (doorHook_t *) self->userHook;
		
	hook->activator = gstate->attacker;
	self->health = hook->base_health;
	self->takedamage = DAMAGE_NO;

	//	Q2FIXME:	other == attacker???
	button_fire (self, attacker, attacker);
}

///////////////////////
//	func_multi_button
///////////////////////

void	func_multi_button (userEntity_t *self)
{
	doorHook_t		*hook;
	CVector			temp;
	int				i;

	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	temp.Set(0.0, 1.0, 0.0);
	self->movedir = temp;
	hook->lip = 4;

	// SCG[11/24/99]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (stricmp (self->epair [i].key, "health") == 0)
			self->health = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "nodetarget") == 0) // this button is linked to a node
			hook->nodetarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "distance"))
			hook->distance = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "count"))
			hook->max_count = atoi (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lip"))	//&&& AMW 5.25.98 - lip value didn't work
			hook->lip = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "sound_use") == 0)
		{
			hook->sound_top = gstate->SoundIndex (self->epair [i].value);
		}	
		else if (stricmp (self->epair [i].key, "sound_return") == 0)
		{
			hook->sound_bottom = gstate->SoundIndex (self->epair [i].value);
		}

		i++;
	}

	com->SetMovedir (self);

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gstate->SetOrigin (self, self->s.origin.x, self->s.origin.y, self->s.origin.z);
	gstate->SetModel (self, self->modelName);
	self->className = "func_multi_button";

	self->blocked = multi_button_blocked;
	self->use = multi_button_use;

	// if the button has health, we don't want to be able to use it
	if (self->health)
	{
		hook->base_health = self->health;
		self->die = multi_button_killed;
		self->takedamage = DAMAGE_YES;
		self->use = NULL;
	}

	//	FIXME: touching buttons no longer activates them, they must be used
	else if ((int)self->spawnflags & PUSH_TOUCH)
		self->touch = multi_button_touch;
	else
		self->touch = NULL;

	if (!hook->speed)
		hook->speed = 40;
	if (!hook->wait)
		hook->wait = 1;

	hook->state = STATE_TOP;
	hook->count = 0;

	// start position
	hook->pos1 = self->s.origin;

	// find ending position
	hook->pos2 = hook->pos1 + self->movedir * hook->distance;

	//&&& AMW 6.12.98 - if this button has a parent, it must be initialized after all the other entities have also
	//&&& AMW 6.26.98 - support for these being child entities is disabled right now
#if 0	
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.5;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}
#else
	self->think = NULL;
	self->nextthink = 0;

#endif
}


///////////////////////////////////////////////////////////////////////////////
//	nodetrain_next
//
//	find the next node_node for func_nodetrain self
//	Use the current node's linked path_corner values for control, etc.
///////////////////////////////////////////////////////////////////////////////
/*
void nodetrain_next (userEntity_t *self)
{
	MAPNODE_PTR		pCurNode = NULL;
	userEntity_t	*cur_path = self->goalentity;
	nodeTrainHook_t	*hook = (nodeTrainHook_t *) self->userHook;
	userEntity_t	*target = NULL;
	doorHook_t		*path_hook = NULL;

	CVector			org, temp;

	// reached the end of the path, nothing else to do
	if (hook->dh.wait == -1)
		return;

	// if this train has a path, then let's continue on it...
	if (hook->path)
	{
		// this is the node we're traveling to
		if (hook->path->nNodeIndex >= 0)
		{
			// first node in path is the current node we're at.. skip it
			pCurNode = &(pTrackNodes->pNodes[hook->path->nNodeIndex]);
			org = pCurNode->position;
		}		
	}
	else
	{
		// no where else to move to
		self->nextthink = -1;
		return;
	}

	// does this node have a path_corner associated with it?
	if (pCurNode && pCurNode->target)
	{
		target = com->FindTarget (pCurNode->target);
		if (target)
		{
			// store for future use
			self->goalentity = target;

			path_hook = (doorHook_t *) target->userHook;
		}  
		// are we using advanced features of the node->path_corner links?
		if (path_hook)
		{
			if (path_hook->wait)
				hook->dh.wait = path_hook->wait;
			else
				hook->dh.wait = 0;

			// if there is a sound on the current path_corner, play it
			if (path_hook->sound_up)
			{
				gstate->StartEntitySound(self, CHAN_OVERRIDE, path_hook->sound_up,0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			}
			// set the func_nodetrain's speed to the current path_corner's speed
			if (path_hook->speed)
				hook->dh.speed = path_hook->speed;
		}
	}

	if (pCurNode)
	{
		hook->path = path_delete_first( hook->path );
		com_CalcMove (self, org, hook->dh.speed, nodetrain_next);  //&&& FIXME - does this train_wait cause the pause?????
	}
	else
	{
		self->nextthink = -1;
	}

}
*/

///////////////////////////////////////////////////////////////////////////////
//	nodetrain_blocked
///////////////////////////////////////////////////////////////////////////////
/*
void nodetrain_blocked (userEntity_t *self, userEntity_t *other)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (gstate->time < hook->move_finished)
		return;

	hook->move_finished = gstate->time + 0.5;

	com->Damage (other, self, self, zero_vector, zero_vector, hook->damage, DAMAGE_SQUISH | DAMAGE_NO_BLOOD);
}
*/
///////////////////////////////////////////////////////////////////////////////
//	nodetrain_use
///////////////////////////////////////////////////////////////////////////////
/*
void nodetrain_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	nodeTrainHook_t	*hook = (nodeTrainHook_t *) self->userHook;

	if (!gstate->targeter)
		return;

	if (com->CheckTargetForActiveChildren (self, other, activator) == TRUE)
		return;

	// find the node we're closest to
	hook->pCurNode = NODE_FindClosestTrackNode( self );

	// find the node the target is closest to and send the train to it
	doorHook_t	*bhook = (doorHook_t *) gstate->targeter->userHook;
	
	if (bhook->nodetarget)
		hook->pTargetNode = NODE_GetNodeFromTargetName( pTrackNodes, bhook->nodetarget );
	else
		hook->pTargetNode = NODE_FindClosestTrackNode( gstate->targeter );


	if (hook->pCurNode && hook->pTargetNode)
	{
		if (hook->pCurNode == hook->pTargetNode)
		{
			gstate->Con_Printf ("Path to current position : %d to %d\n", hook->pCurNode->nIndex, hook->pTargetNode->nIndex);
		}
		
		hook->path = PATH_ComputePath(self, pTrackNodes, hook->pCurNode, hook->pTargetNode, hook->path_length);

		if (hook->path)
		{
			nodetrain_next( self );
		}
	}
}
*/

///////////////////////////////////////////////////////////////////////////////
//	nodetrain_find
//
//	find the first node that the train points to and
//	move the train's origin to it.  Point self->goalentity to
//	the current node (or the one we're moving to)
//
///////////////////////////////////////////////////////////////////////////////
/*
void nodetrain_find (userEntity_t *self)
{
	nodeTrainHook_t	*hook = (nodeTrainHook_t *) self->userHook;
	userEntity_t	*target = NULL;
	CVector			org;


	// if we have a target defined, position on top of it.. otherwise find the closest node
	if (self->target)
	{
		// search through the nodes and find the one with the targetname of self->target
		hook->pCurNode = NODE_GetNodeFromTargetName( pTrackNodes, self->target );
	}
	else
	{
		// find the node we're closest to and snap to it
		hook->pCurNode = NODE_FindClosestTrackNode( self );
	}

	if (hook->pCurNode)
	{
		gstate->SetOrigin2 (self, hook->pCurNode->position);
		
		// see if this node had a link to a path_corner for the advanced features
		if (hook->pCurNode->target)
		{
			target = com->FindTarget (hook->pCurNode->target);
			// point to path_corner for future reference
			if (target)
				self->goalentity = target;
			else
				self->goalentity = NULL;
		}
	}

	// don't do anything right now - by default this train is called by a button
	self->nextthink = -1;
	self->think = NULL;
}
*/

///////////////////////////////////////////////////////////////////////////////
//	func_nodetrain
///////////////////////////////////////////////////////////////////////////////
/*
void	func_nodetrain (userEntity_t *self)
{
	int			i;
	nodeTrainHook_t	*hook;

//	self->userHook = new nodeTrainHook_t;
	self->userHook = gstate->X_Malloc(sizeof(nodeTrainHook_t),MEM_TAG_HOOK);
	hook = (nodeTrainHook_t *) self->userHook;

	self->movedir.Zero();
	self->health = 0;

	hook->dh.sound_up = NULL;
	hook->dh.sound_down = NULL;
	hook->dh.sound_top = NULL;
	hook->dh.sound_bottom = NULL;

	i = 0;
	while (self->epair [i].key != NULL)
	{
        swap_backslashes(self->epair[i].value); // 3.8 dsn  quick fix for MPlayer

		if (!stricmp (self->epair [i].key, "dmg"))
			hook->dh.damage = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "wait"))
			hook->dh.wait = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "speed"))
			hook->dh.speed = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;

		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;

		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;

		else if (!stricmp (self->epair [i].key, "x_distance"))
			hook->dh.x_distance = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "y_distance"))
			hook->dh.y_distance = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "z_distance"))
			hook->dh.z_distance = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "x_speed"))
			hook->dh.x_speed = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "y_speed"))
			hook->dh.y_speed = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "z_speed"))
			hook->dh.z_speed = atof (self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);

		i++;
	}

	// if this train must move regardless of what is blocking it.. i.e. an escalator
	// then let the physics system do special checks for it...
	if (self->spawnflags & FORCEMOVE)
	{
		self->flags |= FL_FORCEMOVE;
		self->blocked = door_blocked;
	}

	if (!hook->dh.speed)
		hook->dh.speed = 100;

	if (!self->target)
		com->Warning ("func_nodetrain %s has no target\n", self->targetname);

	if (!hook->dh.damage)
		hook->dh.damage = 2;

	hook->dh.count = 1;
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	self->blocked = nodetrain_blocked;
	self->takedamage = DAMAGE_NO;
	self->use = nodetrain_use;

	gstate->SetOrigin2 (self, self->s.origin);
	gstate->SetModel (self, self->modelName);

	// this is a special case where it's an "invisible" train without a model
	// usually the case if it's just an origin brush
	if (self->s.modelindex == 0)
	{
		// if this is the case then we don't want it to collide with something
		self->solid = SOLID_NOT;
	}

	//&&& AMW 6.16.98 - save the original position of the train before it moves to it's first target
	self->spawn_origin = self->s.origin;

	// start trains on the second frame, to make sure their targets
	// have been spawned

	self->nextthink = gstate->time + 0.1;
	self->think = nodetrain_find;

}
*/
void door_register_func()
{
	gstate->RegisterFunc("door_blocked",door_blocked);
	gstate->RegisterFunc("func_rotate_blocked",func_rotate_blocked);
	gstate->RegisterFunc("train_blocked",train_blocked);
	gstate->RegisterFunc("button_blocked",button_blocked);
	gstate->RegisterFunc("secret_blocked",secret_blocked);
	gstate->RegisterFunc("multi_button_blocked",multi_button_blocked);
//	gstate->RegisterFunc("nodetrain_blocked",nodetrain_blocked);

	gstate->RegisterFunc("areaportal_toggle",areaportal_toggle);
	gstate->RegisterFunc("door_use",door_use);
	gstate->RegisterFunc("func_rotate_use",func_rotate_use);
	gstate->RegisterFunc("func_wall_use",func_wall_use);
	gstate->RegisterFunc("train_use",train_use);
	gstate->RegisterFunc("secret_use",secret_use);
	gstate->RegisterFunc("multi_button_use",multi_button_use);
// SCG[2/13/00]: 	gstate->RegisterFunc("nodetrain_use",nodetrain_use);
	gstate->RegisterFunc("button_use",button_use);
	gstate->RegisterFunc("button_touch",button_touch);

	gstate->RegisterFunc("plat_restore_touch",plat_restore_touch);
	gstate->RegisterFunc("door_continuous_refire",door_continuous_refire);
	gstate->RegisterFunc("door_close",door_close);
	gstate->RegisterFunc("door_open",door_open);
	gstate->RegisterFunc("door_init",door_init);
	gstate->RegisterFunc("train_rotate_think",train_rotate_think);
	gstate->RegisterFunc("train_next",train_next);
	gstate->RegisterFunc("train_find",train_find);
	gstate->RegisterFunc("train_wait",train_wait);
	gstate->RegisterFunc("button_return",button_return);
	gstate->RegisterFunc("secret_move2",secret_move2);
	gstate->RegisterFunc("secret_move4",secret_move4);
	gstate->RegisterFunc("secret_move6",secret_move6);
// SCG[2/13/00]: 	gstate->RegisterFunc("nodetrain_find",nodetrain_find);

	gstate->RegisterFunc("plat_touch",plat_touch);
	gstate->RegisterFunc("door_touch",door_touch);
	gstate->RegisterFunc("plat_trigger_touch",plat_trigger_touch);
	gstate->RegisterFunc("door_trigger_touch",door_trigger_touch);
	gstate->RegisterFunc("secret_touch",secret_touch);
	gstate->RegisterFunc("multi_button_touch",multi_button_touch);

	gstate->RegisterFunc("secret_pain",secret_pain);
	gstate->RegisterFunc("secret_die",secret_die);
	gstate->RegisterFunc("door_killed",door_killed);
	gstate->RegisterFunc("func_wall_die",func_wall_die);
	gstate->RegisterFunc("train_die",train_die);
	gstate->RegisterFunc("train_die_next",train_die_next);
	gstate->RegisterFunc("button_killed",button_killed);
	gstate->RegisterFunc("multi_button_killed",multi_button_killed);

	gstate->RegisterFunc("door_hook_save", door_hook_save);
	gstate->RegisterFunc("door_hook_load", door_hook_load);
	gstate->RegisterFunc("areaportal_save", areaportal_save);
	gstate->RegisterFunc("areaportal_load", areaportal_load);

	gstate->RegisterFunc("button_done", button_done);
	gstate->RegisterFunc("button_wait", button_wait);
	gstate->RegisterFunc("door_fully_open", door_fully_open);
	gstate->RegisterFunc("door_fully_closed", door_fully_closed);

	gstate->RegisterFunc("secret_move1", secret_move1);
	gstate->RegisterFunc("secret_move3", secret_move3);
	gstate->RegisterFunc("secret_move5", secret_move5);
	gstate->RegisterFunc("secret_done", secret_done);

	gstate->RegisterFunc("multi_button_wait", multi_button_wait);

	gstate->RegisterFunc("func_train_init_from_save", func_train_init_from_save);
	gstate->RegisterFunc("door_init_from_save", door_init_from_save);
}
