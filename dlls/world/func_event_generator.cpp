#include "world.h"

DllExport	void func_event_generator( userEntity_t *self );
void func_event_generator_think( userEntity_t *self );

#define TRIGGER_ONCE		0x00000001
#define TRIGGER_TOUCHABLE	0x00000002
#define ALLOW_MONSTERS		0x00000004

typedef struct event_s
{
	char			*target_name;
	float			delay;
	int				active;
	struct event_s	*next_event;
} event_t;

typedef struct event_generator_hook_s
{
	int		total_events;
	int		active_events;
	float	start_time;
	int		thinking;
	event_t	*events;

	int		sound;
} event_generator_hook_t;

#define	EVENT_GENERATOR_HOOK(x) (int)&(((event_generator_hook_t *)0)->x)
field_t event_generator_hook_fields[] = 
{
	{"total_events",	EVENT_GENERATOR_HOOK(total_events),		F_INT},
	{"active_events",	EVENT_GENERATOR_HOOK(active_events),	F_INT},
	{"start_time",		EVENT_GENERATOR_HOOK(start_time),		F_FLOAT},
	{"events",			EVENT_GENERATOR_HOOK(events),			F_RESTORE_AS_NULL},
	{"sound",			EVENT_GENERATOR_HOOK(sound),			F_INT},
	{NULL, 0, F_INT}
};

#define	EVENT_GENERATOR_EVENT(x) (int)&(((event_t *)0)->x)
field_t event_generator_event_fields[] = 
{
	{"target_name",	EVENT_GENERATOR_EVENT(target_name),	F_LSTRING},
	{"delay",		EVENT_GENERATOR_EVENT(delay),		F_FLOAT},
	{"active",		EVENT_GENERATOR_EVENT(active),		F_INT},
	{"next_event",	EVENT_GENERATOR_EVENT(next_event),	F_RESTORE_AS_NULL},
	{NULL, 0, F_INT}
};

event_t	first_event;

void event_generator_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, event_generator_hook_fields, sizeof( event_generator_hook_t ) );

	event_generator_hook_t	*hook = ( event_generator_hook_t * ) ent->userHook;

	// SCG[12/22/99]: Restore all of the events
	field_t	*field;
	event_t	event, **ppEvent;
	event_t	*current_event;
	
	ppEvent = &hook->events;

	if( hook->thinking == TRUE )
	{
		ent->think = func_event_generator_think;
	}

	for( int i = 0; i < hook->total_events; i++ )
	{
		com->AI_LoadHook( f, &event, sizeof( event ) );

		// change the pointers to lengths or indexes
		for( field = event_generator_event_fields; field->name; field++ )
		{
			com->AI_ReadField( f, field, ( byte * ) &event );
		}

		*ppEvent = ( event_t *) gstate->X_Malloc( sizeof( event_t ), MEM_TAG_ENTITY );
		current_event = *ppEvent;
		current_event->target_name = event.target_name;
		current_event->delay = event.delay;
		current_event->active = event.active;
		ppEvent = &current_event->next_event;
	}
}

void event_generator_hook_save( FILE *f, edict_t *ent )
{
	event_generator_hook_t	*hook = ( event_generator_hook_t * ) ent->userHook;

	AI_SaveHook( f, ent, event_generator_hook_fields, sizeof( event_generator_hook_t ) );

	// SCG[12/22/99]: Save all of the events
	field_t	*field;
	event_t	event, *pEvent;
	
	pEvent = hook->events;

	for( int i = 0; pEvent != NULL; i++ )
	{
		event = *pEvent;

		// change the pointers to lengths or indexes
		for( field = event_generator_event_fields; field->name; field++ )
		{
			com->AI_WriteSaveField1( f, field, ( byte * ) &event );
		}

		// write the block
		com->AI_SaveHook( f, &event, sizeof( event_t ) );

		// now write any allocated data following the edict
		for( field = event_generator_event_fields; field->name; field++ )
		{
			com->AI_WriteSaveField2( f, field, ( byte * ) pEvent );
		}

		pEvent = pEvent->next_event;
	}
}

void trigger_event_delay_think( userEntity_t *self )
{
	if( self->enemy )
	{
		if( self->enemy->use )
		{
			self->enemy->use( self->enemy, self->groundEntity, self->owner );
		}
	}

	gstate->RemoveEntity( self );
}

void SpawnDelay( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	userEntity_t	*temp;

	temp = gstate->SpawnEntity();
	temp->className = "DelayedUse";
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->owner = activator;
	temp->enemy = self;
	temp->groundEntity = other;

	temp->nextthink = gstate->time + self->delay;

	temp->think = trigger_event_delay_think;
	temp->message = self->message;
	temp->target = self->target;
	temp->killtarget = self->killtarget;
}

void func_event_generator_think( userEntity_t *self )
{

	event_generator_hook_t	*hook = ( event_generator_hook_t * ) self->userHook;

	if( hook == NULL )
	{
		return;
	}

	// SCG[8/4/99]: Get the event list
	event_t	*current_event = hook->events;

	// SCG[8/4/99]: Go throught each event
	while( current_event )
	{
		qboolean bFoundTargetname = FALSE;

		// SCG[8/4/99]: Make sure we haven't already called this event
		if( current_event->active != 0 )
		{
			// SCG[8/4/99]: If the delay has passed, use the target
			if( gstate->time >= hook->start_time + current_event->delay )
			{
				// SCG[8/4/99]: Get a pointer to the first entity
				userEntity_t	*target_entity = gstate->FirstEntity ();

				// SCG[8/4/99]: Find the target entity
				while( target_entity )
				{
					if( target_entity->targetname )
					{
						if( stricmp( current_event->target_name, target_entity->targetname ) == 0 )
						{
							bFoundTargetname = TRUE;
							if( target_entity->use != NULL )
							{
								if( target_entity->delay )
								{
									SpawnDelay( target_entity, self, self->owner );
								}
								else
								{
									target_entity->use( target_entity, self, self->owner );
								}
								hook->active_events -= current_event->active;
								current_event->active = 0;
							}
						}
					}

					// SCG[8/4/99]: We didn't find the right entity so get the next one.
					target_entity = gstate->NextEntity( target_entity );
				}
			}
		}

		if( ( bFoundTargetname == FALSE ) && ( gstate->time >= hook->start_time + current_event->delay ) )
		{
			hook->active_events -= current_event->active;
			current_event->active = 0;
		}

		// SCG[8/4/99]: Set the current event to the next event
		current_event = current_event->next_event;
	}

	// SCG[8/4/99]: If everything has been activated, were done for this round
	if( hook->active_events <= 0 )
	{
		self->think = NULL;
		hook->thinking = FALSE;

		if( self->spawnflags & TRIGGER_ONCE )
		{
			hook->active_events = -1;
			gstate->RemoveEntity( self );
		}
	}
	else
	{
		self->nextthink = gstate->time + 0.1;
	}
}

void func_event_generator_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	event_generator_hook_t	*hook = ( event_generator_hook_t * ) self->userHook;

	if( self->spawnflags & ALLOW_MONSTERS )
	{
		if( !com->ValidTouch2( self, other ) )
			return;
	}
	else
	{
		if( !com->ValidTouch( self, other ) )
			return;
	}

	if( hook == NULL )
	{
		return;
	}

	if( hook->active_events != 0 )
	{
		return;
	}

	self->owner = other;

	if( hook->sound )
	{
		gstate->StartEntitySound( self, CHAN_AUTO, hook->sound, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}

	// SCG[8/4/99]: Go through the list of events and trigger each one
	event_t	*current_event = hook->events;

	// SCG[8/4/99]: Set the time when the hook was used.
	hook->start_time = gstate->time;

	while( current_event )
	{
		current_event->active = 1;
		current_event = current_event->next_event;
		hook->active_events++;
	}

	self->think = func_event_generator_think;
	self->nextthink = gstate->time + 0.01;

	hook->thinking = TRUE;

	if( self->spawnflags & TRIGGER_ONCE )
	{
		self->touch = NULL;
	}
}

void func_event_generator_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	if( other->flags & ( FL_CLIENT | FL_BOT | FL_MONSTER ) )
	{
		func_event_generator_touch( self, other, NULL, NULL );
	}
	else
	{
		func_event_generator_touch( self, activator, NULL, NULL );
	}
}

void func_event_generator( userEntity_t *self )
{
	int						i = 0, num_events = 0, targetname_index = 0;
	event_generator_hook_t	*hook;
	event_t					**event;
	event_t					*current_event;

	self->userHook = gstate->X_Malloc( sizeof( event_generator_hook_t ), MEM_TAG_HOOK );

	if( self->userHook == NULL )
	{
		return;
	}

	hook = ( event_generator_hook_t * ) self->userHook;

	hook->active_events = 0;
	hook->sound = 0;

	event = &hook->events;

	while (self->epair[i].key != NULL)
	{
		if( strcmp( self->epair[i].key, "model" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "origin" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "model" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "modelname" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "spawnflags" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "classname" ) == 0 )
		{
		}
		else if( strcmp( self->epair[i].key, "sound" ) == 0 )
		{
			hook->sound = gstate->SoundIndex( self->epair[i].value );
		}
		else if( strcmp( self->epair[i].key, "targetname" ) == 0 )
		{
			targetname_index = i;
		}
		else if( strcmp( self->epair[i].key, "target" ) == 0 )
		{
		}
		else
		{
			*event = ( event_t *) gstate->X_Malloc( sizeof( event_t ), MEM_TAG_ENTITY );

			current_event = *event;

			current_event->target_name = self->epair[i].key;
			current_event->delay = atof( self->epair[i].value );
			event = &current_event->next_event;
			num_events++;
		}

		i++;
	}

	hook->total_events = num_events;

	if( self->spawnflags & TRIGGER_TOUCHABLE )
	{
		self->solid = SOLID_TRIGGER;
		self->touch = func_event_generator_touch;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->use = func_event_generator_use;
	}

	self->movetype = MOVETYPE_NONE;
	gstate->SetModel (self, self->modelName);
	self->modelName = NULL;
	self->svflags |= SVF_NOCLIENT;

	self->save = event_generator_hook_save;
	self->load = event_generator_hook_load;
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_func_event_generator_register_func()
{
	gstate->RegisterFunc("func_event_generator_touch",func_event_generator_touch);
	gstate->RegisterFunc("func_event_generator_think",func_event_generator_think);
	gstate->RegisterFunc("func_event_generator_use",func_event_generator_use);
	gstate->RegisterFunc("func_event_generator", func_event_generator );

	gstate->RegisterFunc( "event_generator_hook_save", event_generator_hook_save );
	gstate->RegisterFunc( "event_generator_hook_load", event_generator_hook_load );
}
