// Triggers
// FIXME: default sounds for each trigger type

#include "world.h"
#include "triggers.h"
#include "misc.h"
#include "nodelist.h"
#include "ai_move.h"
#include "chasecam.h"
#include "client.h"
#include "ai.h"
#include "ai_func.h"
#include "ai_utils.h"
#include "sidekick.h"
#include "ai_info.h"
#include "actorlist.h"
#include "coop.h"
// for monster IDs
#include "ai_frames.h"

///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////
#define Trigger_Reject( ent, flag ) if( ent->flags & flag ) return;


///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport void trigger_relay( userEntity_t *self );
DllExport void trigger_once( userEntity_t *self );
DllExport void trigger_console( userEntity_t *self );
DllExport void trigger_multiple( userEntity_t *self );
DllExport void trigger_setskill( userEntity_t *self );
DllExport void info_teleport_destination( userEntity_t *self );
DllExport void trigger_teleport( userEntity_t *self );
DllExport void trigger_counter( userEntity_t *self );
DllExport void trigger_push( userEntity_t *self );
DllExport void trigger_hurt( userEntity_t *self );
DllExport void trigger_changelevel( userEntity_t *self );
DllExport void func_wall_explode( userEntity_t *self );
//DllExport void trigger_fog_value( userEntity_t *self );
//DllExport void trigger_warp( userEntity_t *self );
//DllExport void info_warp_destination( userEntity_t *self );
DllExport void warp_cylinder( userEntity_t *self );
DllExport void trigger_change_sfx( userEntity_t *self );
DllExport void trigger_secret(userEntity_t *self );
//DllExport void trigger_monsterjump( userEntity_t *self );
DllExport void trigger_cinematic( userEntity_t *self );	//amw INVALID... remove soon
DllExport void trigger_script( userEntity_t *self );
DllExport void trigger_changetarget( userEntity_t *self );
DllExport void trigger_sidekick( userEntity_t *self ); 
DllExport void trigger_sidekick_stop( userEntity_t *self ); 
DllExport void trigger_sidekick_teleport( userEntity_t *self );
DllExport void trigger_toggle( userEntity_t *self );
DllExport void trigger_changemusic( userEntity_t *self );
DllExport void target_crosslevel_trigger( userEntity_t  *self );
DllExport void target_crosslevel_target( userEntity_t  *self );
DllExport void trigger_remove_inventory_item( userEntity_t  *self );

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

int			func_wall_explode_count;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

// trigger flags
#define		SPAWNFLAG_NOMESSAGE				0x01

#define		SPAWNFLAG_NOTOUCH				0x01
#define		TRIGGER_TOGGLE_USE_ON_EXIT		0x01
#define		TRIGGER_INCLUDE_MONSTERS		0x02
#define		TRIGGER_INCLUDE_SIDEKICKS		0x04
#define		TRIGGER_TOGGLE_SIDEKICK_ONLY	0x08

// trigger_hurt flags
#define		ALLOW_TOGGLE				0x01
#define		START_DISABLED				0x02
#define		FLOWTHRU_DMG				0x04

// func_wall_explode
#define		ROCK_CHUNKS			1
#define		WOOD_CHUNKS			2
#define		EXTRA_CHUNKS		4
#define		EXTRA_VELOCITY		8
#define		NO_CHUNKS			16
#define		NO_SOUND			32
#define		METAL_CHUNKS		64
#define		NO_EXPLOSIONS		128

//	teleporter/warp spawnflags
#define		PLAYER_ONLY			1

//	teleporter spawnflags
#define		NO_FLASH			2
#define		NO_ANGLE_ADJUST		4

//	info_warp_destination spawnflags
//#define		INFOWARP_SKIPTO		1
//#define		INFOWARP_GLOW		2

// trigger_change_sfx
#define		SFX_NORMAL			0x01
#define		SFX_PRESET1			0x02
#define		SFX_PRESET2			0x04
#define		SFX_PRESET3			0x08
#define		SFX_PRESET4			0x10

// trigger_sript
#define		SCRIPT_MULTIPLE		0x01
#define		SCRIPT_NOTOUCH		0x02

// trigger_changetarget & trigger_console
#define		USE_ONCE			0x01


///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	warpHook_s
{
	think_t		think;
	float		nextthink;

	movetype_t	movetype;
	solid_t		solid;

	unsigned	long	effects;

	CVector		velocity;
	CVector		render_scale;
} warpHook_t;

#define	WARPHOOK(x) (int)&(((warpHook_t *)0)->x)
field_t warp_hook_fields[] = 
{
	{"think",			WARPHOOK(think),		F_FUNC},
	{"nextthink",		WARPHOOK(nextthink),	F_FLOAT},
	{"movetype",		WARPHOOK(movetype),		F_INT},
	{"solid",			WARPHOOK(solid),		F_INT},
	{"effects",			WARPHOOK(effects),		F_INT},
	{"velocity",		WARPHOOK(velocity),		F_VECTOR},
	{"render_scale",	WARPHOOK(render_scale),	F_VECTOR},
	{NULL, 0, F_INT}
};

typedef struct sfxTrigHook_s
{
	float	volume;
	float	reverb;
	int		fxStyle;
} sfxTrigHook_t;

#define	SFXTRIGHOOK(x) (int)&(((sfxTrigHook_t *)0)->x)
field_t sfx_trig_hook_fields[] = 
{
	{"volume",	SFXTRIGHOOK(volume),	F_FLOAT},
	{"reverb",	SFXTRIGHOOK(reverb),	F_FLOAT},
	{"fxStyle",	SFXTRIGHOOK(fxStyle),	F_INT},
	{NULL, 0, F_INT}
};

typedef struct scriptTrigHook_s
{
	char	*pAiScript;
	char	*pCineScript;
	float	wait;
	float	nextTriggerTime;
} scriptTrigHook_t;

#define	SCRIPTTRIGHOOK(x) (int)&(((scriptTrigHook_t *)0)->x)
field_t script_trig_hook_fields[] = 
{
	{"pAiScript",		SCRIPTTRIGHOOK(pAiScript),			F_LSTRING},
	{"pCineScript",		SCRIPTTRIGHOOK(pCineScript),		F_LSTRING},
	{"wait",			SCRIPTTRIGHOOK(wait),				F_FLOAT},
	{"nextTriggerTime",	SCRIPTTRIGHOOK(nextTriggerTime),	F_FLOAT},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	TRIGHOOK(x) (int)&(((trigHook_t *)0)->x)
field_t trig_hook_fields[] = 
{
	{"wait",				TRIGHOOK(wait),					F_FLOAT},
	{"base_health",			TRIGHOOK(base_health),			F_FLOAT},
	{"next_trigger_time",	TRIGHOOK(next_trigger_time),	F_FLOAT},
	{"damage",				TRIGHOOK(damage),				F_FLOAT},
	{"speed",				TRIGHOOK(speed),				F_FLOAT},
	{"bEnabled",			TRIGHOOK(bEnabled),				F_INT},
	{"count",				TRIGHOOK(count),				F_INT},
	{"triggerSound",		TRIGHOOK(triggerSound),			F_INT},
	{"console_msg",			TRIGHOOK(console_msg),			F_LSTRING},
	{"model_1",				TRIGHOOK(model_1),				F_LSTRING},
	{"model_2",				TRIGHOOK(model_2),				F_LSTRING},
	{"model_3",				TRIGHOOK(model_3),				F_LSTRING},
	{"activator",			TRIGHOOK(activator),			F_EDICT},
	{"mangle",				TRIGHOOK(mangle),				F_VECTOR},
	{"v_angle",				TRIGHOOK(v_angle),				F_VECTOR},
	{"pCineScript",			TRIGHOOK(pCineScript),			F_LSTRING},
	{"pAiScript",			TRIGHOOK(pAiScript),			F_LSTRING},
	{"done_think",			TRIGHOOK(done_think),			F_FUNC},
	{"mp3file",				TRIGHOOK(mp3file),				F_LSTRING},
	{NULL, 0, F_INT}
};

// SCG[11/19/99]: Save game stuff
#define	ROCKHOOK(x) (int)&(((rockHook_t *)0)->x)
field_t rock_hook_fields[] = 
{
	{"remove_time",		ROCKHOOK(remove_time),		F_FLOAT},
	{"last_touch_time",	ROCKHOOK(last_touch_time),	F_FLOAT},
	{"last_water",		ROCKHOOK(last_water),		F_FLOAT},
	{"change_time",		ROCKHOOK(change_time),		F_FLOAT},
	{"rotation_scale",	ROCKHOOK(rotation_scale),	F_FLOAT},
	{"mass_scale",		ROCKHOOK(mass_scale),		F_FLOAT},
	{"last_pos",		ROCKHOOK(last_pos),			F_VECTOR},
	{"hit_water_pos",	ROCKHOOK(hit_water_pos),	F_VECTOR},
	{NULL, 0, F_INT}
};

void warp_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, warp_hook_fields, sizeof( warpHook_t ) );
}

void warp_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, warp_hook_fields, sizeof( warpHook_t ) );
}

void sfx_trig_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, sfx_trig_hook_fields, sizeof( sfxTrigHook_t ) );
}

void sfx_trig_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, sfx_trig_hook_fields, sizeof( sfxTrigHook_t ) );
}

void script_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, script_trig_hook_fields, sizeof( scriptTrigHook_t ) );
}

void script_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, script_trig_hook_fields, sizeof( scriptTrigHook_t ) );
}

void trig_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, trig_hook_fields, sizeof( trigHook_t ) );
}

void trig_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, trig_hook_fields, sizeof( trigHook_t ) );
}

void rock_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, rock_hook_fields, sizeof( rockHook_t ) );
}

void rock_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, rock_hook_fields, sizeof( rockHook_t ) );
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_init
///////////////////////////////////////////////////////////////////////////////

void trigger_init( userEntity_t *self )
{
// trigger angles are used for one-way touches.  An angle of 0 is assumed
// to mean no restrictions, so use a yaw of 360 instead.

	if (self->s.angles.Length() == 0.0 )
		com->SetMovedir (self);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	gstate->SetModel (self, self->modelName);
	gstate->LinkEntity (self);

	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	memset( self->userHook, 0, sizeof( trigHook_t ) );

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;
	
};

///////////////////////////////////////////////////////////////////////////////
//	trigger_remove
//
//	doesn't bother to deallocate the userHook hunk since there
//	should be very few triggers on a level and that memory will
//	be reclaimed each time the level ends
///////////////////////////////////////////////////////////////////////////////

void trigger_remove( userEntity_t *self )
{
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_skill_touch
///////////////////////////////////////////////////////////////////////////////

void trigger_skill_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	gstate->SetCvar ("skill", self->message);
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_setskill
///////////////////////////////////////////////////////////////////////////////

void trigger_setskill( userEntity_t *self )
{
	int		i;

	trigger_init (self);

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "message"))
			self->message = self->epair [i].value;
	}

	self->touch = trigger_skill_touch;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_reactivate
///////////////////////////////////////////////////////////////////////////////

void trigger_reactivate( userEntity_t *self )
{
	self->solid = SOLID_TRIGGER;
}

///////////////////////////////////////////////////////////////////////////////
//	multi_wait
//
//	reset trigger to touch again
///////////////////////////////////////////////////////////////////////////////

void multi_wait( userEntity_t *self )
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	if (hook->base_health)
	{
		self->health = hook->base_health;
		self->takedamage = DAMAGE_YES;
		self->solid = SOLID_BBOX;
	}
	else
		trigger_reactivate (self);
}

// NSS[2/14/00]:
void PlaySidekickMP3(userEntity_t *self,char *sound);
///////////////////////////////////////////////////////////////////////////////
//	multi_trigger
///////////////////////////////////////////////////////////////////////////////

void multi_trigger( userEntity_t *self, userEntity_t *other, userEntity_t *activator, qboolean bTriggeredByUse )
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	unsigned int	nRejectFlag;

	if (self->nextthink > gstate->time)
		return;

	nRejectFlag = 0;

	// SCG[12/11/99]: Get reject flags
/*
	if( bTriggeredByUse == TRUE )
	{
		nRejectFlag = 0;
	}
*/
	if( !( self->spawnflags & TRIGGER_INCLUDE_SIDEKICKS ) )
	{
		nRejectFlag = FL_BOT;
	}

	if( !( self->spawnflags & TRIGGER_INCLUDE_MONSTERS ) )
	{
		nRejectFlag |= FL_MONSTER;
	}

	// SCG[12/11/99]: Reject test
	Trigger_Reject( activator, ( nRejectFlag ) );

	// SCG[12/11/99]: Do vald touch tests based of reject flags...
	if( nRejectFlag & FL_MONSTER )
	{
		if( !com->ValidTouch( self, activator ) )
		{
			return;
		}
	}
	else
	{
		if( !com->ValidTouch2( self, activator ) )
		{
			return;
		}
	}

	if ( (self->className != NULL ) && !stricmp (self->className, "trigger_secret"))
	{
		//  increment number of secrets found in this map
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_world,T_FOUND_SECRET),0);

		if (activator->flags & FL_CLIENT)
			gstate->cs.Unicast(activator,CS_print_center,2.0);
		else if (other->flags & FL_CLIENT)
			gstate->cs.Unicast(other,CS_print_center,2.0);

		gstate->cs.EndSendString();

		gstate->numSecretsFound++;	
	}

	if( hook->triggerSound )
	{
		gstate->StartEntitySound(other, CHAN_OVERRIDE, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	// NSS[3/1/00]:Do not play this stuff in CooP or death match mode.
	if( ( hook->mp3file != NULL ) && ( hook->mp3file[0] != NULL ) && ( coop->value == 0 ) && ( deathmatch->value == 0 ) )
	{
		// NSS[2/14/00]:Cheese hack requested by John
		if(strstr(hook->mp3file,"superfly")|| strstr(hook->mp3file,"mikiko"))
		{
			PlaySidekickMP3(activator,hook->mp3file);
		}
		else
		{
			gstate->StartMP3( hook->mp3file, CHAN_CINE_1, 1.0, 1 );
		}
	}

	// don't trigger again until reset
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;

	gstate->activator = activator;	//&&&&&  ?????????????  is this right?

	// fixes race condition.. UseTargets adds another delay...
	float oldDelay = self->delay;
	self->delay = 0.0f;

	com->UseTargets (self, other, activator);
	
	self->delay = oldDelay;

	if (hook->wait >= 0)
	{
		self->think = multi_wait;
		self->nextthink = gstate->time + hook->wait;
	}
	else
	{
		// trigger_onces remove themselves
		self->touch = NULL;
		self->nextthink = gstate->time + self->delay + 0.1;
		self->think = trigger_remove;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	multi_killed
///////////////////////////////////////////////////////////////////////////////

void multi_killed( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damgae, CVector &point)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	hook->activator = gstate->attacker;
	
	//	Q2FIXME:	other == attacker???
	multi_trigger (self, attacker, attacker, FALSE);
}

///////////////////////////////////////////////////////////////////////////////
//	multi_pain
///////////////////////////////////////////////////////////////////////////////

void multi_pain( userEntity_t *self, userEntity_t *other, float kick, int damage)
{
	//NSS[11/30/99]:changed so triggers with health will now wait until they have
	//no health left before they trigger.
	self->health -= damage;
	if(self->health <= 0)
		multi_killed (self, other, other, damage, self->s.origin);
}

///////////////////////////////////////////////////////////////////////////////
//	multi_use
///////////////////////////////////////////////////////////////////////////////

//	Q2FIXME:	activator?!?!?

void multi_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	
	hook->activator = activator;
	
	// NSS[12/15/99]:Changed activator to self and made self possessed to pas the validtouch function.
	self->flags |= FL_POSSESSED;

	multi_trigger (self, other , self, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//	multi_touch
///////////////////////////////////////////////////////////////////////////////

void multi_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	//NSS[11/10/99]:
	Trigger_Reject( other, ( FL_MONSTER | FL_BOT ) );
	// is this a valid touch?  are we including monsters for this trigger?
 	if (self->spawnflags & TRIGGER_INCLUDE_MONSTERS)
	{
		if (!com->ValidTouch2 (self, other))
			return;
	}
	else
	{
		if (!com->ValidTouch (self, other))
			return;
	}

	// FIXME!!!!
/*
	if (!com->CmpVector (self->movedir, 0.0, 0.0, 0.0))
	{
		other->s.angles.AngleToVectors(forward, right, up);
		
		if (com->DotProduct (forward, self->movedir) < 0)
		{
"			gstate->Con_Printf ("angle failed\n");
			return;
		}
	}
*/

	hook->activator = other;
	multi_trigger (self, other, other, FALSE);

	if (hook->triggerSound)
	{
		gstate->StartEntitySound(other, CHAN_VOICE, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_setup
//
//	shared code for setting up a trigger
///////////////////////////////////////////////////////////////////////////////

void	trigger_setup( userEntity_t *self )
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	char		*soundStr = NULL;
	int			i;

	// parse epairs
	// SCG[1/13/00]: Can you say redundant?
//	hook = (trigHook_t *) self->userHook;

	self->movedir.Zero();

	memset( hook, 0, sizeof( trigHook_t ) );

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "health"))
			self->health = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			soundStr = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "message"))
			self->message = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;
	}

	if (soundStr)
		hook->triggerSound = gstate->SoundIndex (soundStr);

	if (!hook->wait)
		hook->wait = 0.2;
	
	self->use = multi_use;

	if (self->health)
	{
		if (self->spawnflags & SPAWNFLAG_NOTOUCH)
			gstate->Con_Dprintf ("health and NOTOUCH don't make sense\n");

		hook->base_health = self->health;
		self->die = multi_killed;
		self->pain = multi_pain;
		self->takedamage = DAMAGE_YES;
		self->solid = SOLID_BBOX;
		gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
		gstate->LinkEntity(self);
	}
	else
	{
		if (!(self->spawnflags & SPAWNFLAG_NOTOUCH))
			self->touch = multi_touch;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_console_touch
//
//	FIXME: test this out
//	sends message as a console command  
///////////////////////////////////////////////////////////////////////////////

void trigger_console_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	char buf[256];
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);
	
	if (!com->ValidTouch (self, other))
		return;

//	sprintf(buf, "%s\n",self->message);
	Com_sprintf(buf, sizeof(buf),"%s\n",self->message);
	
	// stuff the keyboard handler
	gstate->CBuf_AddText(buf);

	if (hook->triggerSound)
	{
		gstate->StartEntitySound(other, CHAN_VOICE, hook->triggerSound,0.80f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	if (self->spawnflags & USE_ONCE)
	{
		self->remove(self);
	}

}

///////////////////////////////////////////////////////////////////////////////
//	trigger_console
//
//	Basically you only want a trigger to happen once in most cases
//	key "command"
//	self->message -- text to send to console
///////////////////////////////////////////////////////////////////////////////

void trigger_console( userEntity_t *self )
{
	trigger_init (self);
//	trigHook_t	*hook = (trigHook_t *) self->userHook;// SCG[1/23/00]: not used

	// set defaults from keys
	trigger_setup (self);

	self->touch = trigger_console_touch;

	if( self->message == NULL )
	{

		for( int i = 0; self->epair [i].key != NULL; i++ )
		{
			if (!stricmp (self->epair [i].key, "command"))
				self->message = self->epair [i].value;
		}

		if( self->message == NULL )
		{
			gstate->Con_Dprintf ("WARNING: invalid trigger_console at %s\n",com->vtos(self->s.origin));
			self->remove(self);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
//	trigger_once
///////////////////////////////////////////////////////////////////////////////

void	trigger_once( userEntity_t *self )
{
	trigHook_t	*hook;

	trigger_init (self);
	hook = (trigHook_t *) self->userHook;

	// set defaults from keys
	trigger_setup (self);
		
	// never retriggers 
	hook->wait = -1.0;

	self->className = "trigger_once";
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_multiple
//
//	see world.c, I don't feel like writing it again
///////////////////////////////////////////////////////////////////////////////

void	trigger_multiple( userEntity_t *self )
{
	trigger_init (self);

	trigger_setup (self);
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_relay_use
///////////////////////////////////////////////////////////////////////////////
void trigger_relay_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	
	// implement the wait key
	if (hook->next_trigger_time > gstate->time)
	{		
		return;
	}

	hook->activator = activator;
	// play the sound if this trigger has one assigned
	if (hook->triggerSound)
	{
		gstate->StartEntitySound(self, CHAN_OVERRIDE, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	// fixes race condition
	float oldDelay = self->delay;
	self->delay = 0.0f;

	// go ahead and use the targets
	com->UseTargets( self, other, activator );

	self->delay = oldDelay;

	// wait before we're able to use this trigger again
	hook->next_trigger_time = gstate->time + hook->wait;
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_relay
//
//	these are only entities.. no associated brushes.. do not link 
//
///////////////////////////////////////////////////////////////////////////////

void trigger_relay( userEntity_t *self )
{
	trigHook_t	*hook = NULL;

	// set up the userhook
	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	hook = (trigHook_t *) self->userHook;
	memset( hook, 0, sizeof( trigHook_t ) );

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;

	// set the default wait and delay values
	for (int i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			hook->triggerSound = gstate->SoundIndex (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "message"))
			self->message = self->epair [i].value;
	}
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	// trigger relays can only be activated by targeting them
	self->touch = NULL;
	self->use = trigger_relay_use;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_secret
///////////////////////////////////////////////////////////////////////////////

void trigger_secret( userEntity_t *self )
{
	trigHook_t	*hook;

	// increment total secrets variable...
	gstate->numSecrets++;
	
	trigger_init (self);
	hook = (trigHook_t *) self->userHook;

	trigger_setup (self);

	// amw: moved here because it can be set in the epairs
	hook->wait = -1.0;

	// 5.11 dsn  silly..
    //if (!self->message)
	//	self->message = "You found a secret!";


	if( gstate->episode == 1 )
	{
		hook->triggerSound = gstate->SoundIndex ("e1/e1_secret.wav");
	}
	else if( gstate->episode == 2 )
	{
		hook->triggerSound = gstate->SoundIndex ("e2/e2_secret.wav");
	}
	else if( gstate->episode == 3 )
	{
		hook->triggerSound = gstate->SoundIndex ("e3/e3_secret.wav");
	}
	else if( gstate->episode == 4 )
	{
		hook->triggerSound = gstate->SoundIndex ("e4/e4_secret.wav");
	}

}

/* ---------------------------- Teleport Triggers ---------------------------- */


///////////////////////////////////////////////////////////////////////////////
//	spawn_tele_sound
///////////////////////////////////////////////////////////////////////////////

void	spawn_tele_sound( userEntity_t *self )
{
	trigHook_t	*owner_hook = (trigHook_t *) self->owner->userHook;

	if (owner_hook->triggerSound)
		gstate->StartEntitySound(self, CHAN_AUTO, owner_hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	spawn_tele_fog
///////////////////////////////////////////////////////////////////////////////

void	spawn_tele_fog( userEntity_t *self, CVector &org)
{
	userEntity_t	*temp;

	temp = gstate->SpawnEntity ();
	
	temp->s.origin = org;
	temp->nextthink = gstate->time + 0.2;
	temp->owner = self;
	
	temp->think = spawn_tele_sound;

	//	Q2FIXME:	change to Q2 network functions
	/*
	gstate->WriteByte (self, MSG_BROADCAST, SVC_TEMPENTITY);
	gstate->WriteByte (self, MSG_BROADCAST, TE_TELEPORT);
	gstate->WriteCoord (self, MSG_BROADCAST, org.x);
	gstate->WriteCoord (self, MSG_BROADCAST, org.y);
	gstate->WriteCoord (self, MSG_BROADCAST, org.z);
	*/
}

///////////////////////////////////////////////////////////////////////////////
//	tele_gib_touch
///////////////////////////////////////////////////////////////////////////////

void	tele_gib_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userEntity_t	*head;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	if (other == self->owner)
		return;

	if (other->className)
	{
		if (stricmp (other->className, "player") == 0)
		{
			//FIXME: check for invincible

			// Check the touching player's tele_gib time against this
			// one.  If it's more recent, then the touching player doesn't
			// get gibbed because he came through last
			head = gstate->FirstEntity ();

			while (head)
			{
				if (head->className)
				{
					if ((head->owner == other) && (VectorDistance (head->s.origin, self->s.origin) < 32) &&
						(stricmp (head->className, "tele_gib") == 0))
					{
						if (head->nextthink > self->nextthink)
						{
							//gstate->Con_Printf ("%s came through tele last\n", other->className);
							return;
						}
					}
				}
				
				head = gstate->NextEntity (head);
			}

			if (stricmp (self->owner->className, "player") != 0)
			{
				// explode monsters teleporting onto players
				com->Damage (self->owner, self, self, zero_vector, zero_vector, 50000.0, DAMAGE_TELEFRAG | DAMAGE_NO_BLOOD);
				return;
			}
		}
	}
	else return;

	if (other->health)
	{
		client_deathtype = TELEFRAG;
		com->Damage (other, self, self, zero_vector, zero_vector, 50000.0, DAMAGE_TELEFRAG | DAMAGE_NO_BLOOD);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	tele_gib_remove
///////////////////////////////////////////////////////////////////////////////

void	tele_gib_remove( userEntity_t *self )
{
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	spawn_tele_gib
///////////////////////////////////////////////////////////////////////////////

void	spawn_tele_gib( CVector &org, userEntity_t *gib_owner)
{
	userEntity_t	*death;
	CVector			origin, mins, maxs, temp;

	death = gstate->SpawnEntity ();
	death->className = "tele_gib";
	death->movetype = MOVETYPE_NONE;
	death->solid = SOLID_TRIGGER;
	death->modelName = "";
	death->owner = gib_owner;

	death->s.angles.Zero();
	
	com->GetMinsMaxs (gib_owner, origin, mins, maxs);
	temp = CVector(1.0, 1.0, 1.0);

	mins = mins - temp;
	maxs = maxs + temp;

	gstate->SetSize (death, mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);
	gstate->SetOrigin (death, org.x, org.y, org.z);

	death->touch = tele_gib_touch;
	death->nextthink = gstate->time + 0.2;
	death->think = tele_gib_remove;
	
	// FIXME: no global for this...
	//force_retouch = 2;
}

///////////////////////////////////////////////////////////////////////////////
//	info_teleport_destination
///////////////////////////////////////////////////////////////////////////////

void	info_teleport_destination( userEntity_t *self )
{
	CVector		temp;
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	hook = (trigHook_t *) self->userHook;
	memset( hook, 0, sizeof( trigHook_t ) );

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;
		i++;
	}

	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

	self->className = "info_teleport_destination";

	//self->mangle = self->s.angles;
	
	hook->mangle = self->s.angles;
	self->s.angles.Zero();
	self->modelName = "";
	
	temp = CVector(0.0, 0.0, 27.0) + self->s.origin;
	self->s.origin = temp;

	if (!self->targetname)
		gstate->Con_Dprintf("No targetname for teleport_dest\n");
}	

///////////////////////////////////////////////////////////////////////////////
//	teleport_touch
///////////////////////////////////////////////////////////////////////////////

void	teleport_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userEntity_t	*dest;
	CVector			org, ang_diff, enter_dir, facing_dir;
	int				found;
	trigHook_t		*hook, *hook2 = (trigHook_t *) self->userHook;
	playerHook_t	*ohook;
	teleportNode_t	*node_data;	

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER );
	
	if( com->ValidTouch2( self, other ) == 0 )
	{
		return;
	}

	if (self->targetname)
	{
		if (self->nextthink < gstate->time)
			return;
	}

	if ((int)self->spawnflags & PLAYER_ONLY)
	{
		if (!com->ValidTouch (self, other))
			return;
	}

	com->UseTargets (self, other, other);

	// put a teleport fog where the player was
	if (!((int)self->spawnflags & NO_FLASH))
		spawn_tele_fog (self, other->s.origin);

	// find the teleporters target
	dest = gstate->FirstEntity ();

	found = FALSE;
	while (dest && !found)
	{
		if (dest->targetname)
		{
			if (stricmp (dest->targetname, self->target) == 0)
				found = TRUE;
			else
				dest = gstate->NextEntity (dest);
		}
		else
	 		dest = gstate->NextEntity (dest);
	}

	// SCG[1/26/00]: 
	if( dest == NULL )
	{
		return;
	}

	hook = (trigHook_t *) dest->userHook;

	// spawn a flash in front of the destination

	hook->mangle.AngleToVectors(forward, right, up);	
	//com->GetMinsMaxs (dest, org, mins, maxs);
	//org = org + dest->s.origin;
	org = dest->s.origin + forward * 32.0;
	
	if (!((int)self->spawnflags & NO_FLASH))
		spawn_tele_fog (self, org);
	spawn_tele_gib (dest->s.origin, other);

	if (!other->health)
	{
		org = dest->s.origin;
		other->s.origin = org;

		org = forward * other->velocity[0] + forward * other->velocity[1];
		other->velocity = org;

		return;
	}

	// set fog_value if set for teleport
	// FIXME: local commands to a specific client!!!

/*
	if (other->flags & FL_CLIENT)
	{
		if (hook2->model_1)
		{
			gstate->Con_Dprintf ("setting fog\n");

			if (!stricmp (hook2->model_1, "0"))
			{
				gstate->WriteByte (self, MSG_ONE, SVC_FOGVALUE);
				gstate->WriteByte (self, MSG_ONE, 0);
			}
			else
			{
				if (!gstate->foglevel)
				{
					gstate->WriteByte (self, MSG_ONE, SVC_FOGVALUE);
					gstate->WriteByte (self, MSG_ONE, 1);
				}
				gstate->WriteByte (self, MSG_ONE, SVC_FOGVALUE);
				gstate->WriteByte (self, MSG_ONE, atoi (hook2->model_1));
			}
		}
	}
*/
	gstate->SetOrigin (other, dest->s.origin [0], dest->s.origin [1], dest->s.origin [2]);

	if (other->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		other->s.angles.z = 0.0;
		other->fixangle = 1.0;
		other->teleport_time = gstate->time + 0.7;
		other->groundEntity = NULL;

		// set velocity of teleported player to the direction
		// of the teleporter's mangle, at the same speed as they
		// were traveling when they entered the teleporter
		org = other->velocity;
		forward = forward * org.Length();
		if ( !((int)self->spawnflags & NO_ANGLE_ADJUST) )
		{
			other->velocity = forward;
		}
	}

//	gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/e_forceoff.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
//	gstate->StartEntitySound(other, CHAN_AUTO, gstate->SoundIndex("global/e_forceoff.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/new_teleport1.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	gstate->StartEntitySound(other, CHAN_AUTO, gstate->SoundIndex("global/new_teleport1.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);



	if (other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_MONSTER)
	{
		///////////////////////////////////////////////////////////////////////
		//	force a teleport node to be placed at the location of this teleport
		///////////////////////////////////////////////////////////////////////

		ohook = AI_GetPlayerHook( other );

		other->s.angles = hook->mangle;
		if(other->flags &FL_CLIENT)
		{
			if(other->client)
			{
				// NSS[1/7/00]:Hack to force view angles
				other->client->fix_angles.yaw		= hook->mangle.yaw;
				other->client->v_angle.yaw			= hook->mangle.yaw;
				other->client->oldviewangles.yaw	= hook->mangle.yaw;
				other->client->ps.viewangles.yaw	= hook->mangle.yaw;
				ohook->force_rate.x = 0;
				other->flags						|= FL_FIXANGLES;
				gstate->LinkEntity(other);
			}
		}

		hook = (trigHook_t *) self->userHook;

		org = (self->absmin + self->absmax) * 0.5;

		//	create the necessary node data for this teleport
		node_data = (teleportNode_t*)gstate->X_Malloc(sizeof(teleportNode_t), MEM_TAG_HOOK);
		node_data->start_origin = org;
		node_data->dest_origin = dest->s.origin;

		//node_force_node (other, ohook->pNodeList, node_data->start_origin, NODE_TELEPORTER, node_data);

		//	so exit node from teleporter/warp gets placed at actual exit!
		ai_teleported (other);
	}



}

///////////////////////////////////////////////////////////////////////////////
//	teleport_use// NSS[12/13/99]:Added the ability to warp players for cinematics request per John Romero
///////////////////////////////////////////////////////////////////////////////

void	teleport_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*dest;
	userEntity_t	*ent;
	int				found;
	int				i = 0;

	// NSS[12/13/99]:First find the destination
	dest = gstate->FirstEntity ();

	found = FALSE;
	while (dest && !found)
	{
		if (dest->targetname)
		{
			if (stricmp (dest->targetname, self->target) == 0)
				found = TRUE;
			else
				dest = gstate->NextEntity (dest);
		}
		else
	 		dest = gstate->NextEntity (dest);
	}
	
	// NSS[12/13/99]:If we found our destination then let's go.
	if(found == TRUE)
	{
		// NSS[12/13/99]:Look for our information
		while (self->epair [i].key != NULL)
		{
			// NSS[12/13/99]:find the epair value teleport
			if (!stricmp (self->epair [i].key, "teleport"))
			{
				if(!stricmp(self->epair[i].value,"player"))
				{
					//We are looking for the player... for speeds reasons use this method.
					ent = &gstate->g_edicts[1];
					gstate->SetOrigin (ent, dest->s.origin [0], dest->s.origin [1], dest->s.origin [2]);

					if(ent->flags & FL_CLIENT)
					{
						if(ent->client)
						{
							playerHook_t	*ohook = AI_GetPlayerHook( ent );
							trigHook_t		*hook = (trigHook_t *) dest->userHook;

							// NSS[1/7/00]:Hack to force view angles
							ent->client->fix_angles.yaw		= hook->mangle.yaw;
							ent->client->v_angle.yaw		= hook->mangle.yaw;
							ent->client->oldviewangles.yaw	= hook->mangle.yaw;
							ent->client->ps.viewangles.yaw	= hook->mangle.yaw;
							ohook->force_rate.x = 0;
							ent->flags						|= FL_FIXANGLES;
							gstate->LinkEntity(other);
						}
					}
				}
				else
				{
					ent = gstate->FirstEntity();
					while(ent)
					{
						//Make sure the entity has the right targetname
						if(_stricmp(ent->targetname, self->epair[i].value)==0)
						{
							// NSS[12/13/99]:If so let's warp.
							gstate->SetOrigin (ent, dest->s.origin [0], dest->s.origin [1], dest->s.origin [2]);
							ent->s.angles = dest->s.angles;
						}
						ent = gstate->NextEntity(ent);
					}
				}
			}
			i++;
		}
	}	
	
	self->think = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_teleport
///////////////////////////////////////////////////////////////////////////////

void	trigger_teleport( userEntity_t *self )
{
	trigHook_t	*hook;
	int			i;
	int			Special = 0;				// NSS[12/13/99]:This lets us know we are a special teleporter and should not have a touch function
	char		*soundStr = NULL;

	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "killtarget") == 0)
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "fog_value"))
		{
			gstate->Con_Dprintf("fog_value = %s\n", self->epair [i].value);
			hook->model_1 = self->epair [i].value;
		}
		else if (!stricmp (self->epair [i].key, "mp3"))
		{
			hook->mp3file = self->epair[i].value;
		}
		else if (!stricmp (self->epair [i].key, "teleport"))
		{
			if(strlen(self->epair[i].value)>2)
			{
				Special = 1;
			}
		}
		

		i++;
	}

	if (soundStr)	
		hook->triggerSound = gstate->SoundIndex (soundStr);
	
	// NSS[12/13/99]:
	if(!Special)
	{
		self->touch = teleport_touch;
	}
	
	self->className = "trigger_teleport";

	if (!self->target)
	{
		gstate->Con_Dprintf("teleporter has no target!\n");
		gstate->RemoveEntity(self);
		return;
	}

//cek[2-1-00]: john wants all teleporters to have this sound	if (!hook->triggerSound)
	{
		hook->triggerSound = gstate->SoundIndex ("global/new_teleport1.wav");//"misc/teleportstart.wav");

	}

	self->use = teleport_use;
}

///////////////////////////////////////////////////////////////////////////////
//	onlyreg_touch
///////////////////////////////////////////////////////////////////////////////

void	onlyreg_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);
	
	if (!com->ValidTouch (self, other))
		return;

	if (hook->next_trigger_time > gstate->time)
		return;

	hook->next_trigger_time = gstate->time + 2.0;

	if (gstate->GetCvar ("registered"))
	{
		self->message = NULL;
		com->UseTargets (self, other, other);
		self->remove (self);
	}
	else
	{
		if (self->message && other->flags & FL_CLIENT)
		{
			if (hook->triggerSound)
				gstate->StartEntitySound(self, CHAN_AUTO, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_onlyregistered
///////////////////////////////////////////////////////////////////////////////

/*
void	trigger_onlyregistered( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	// FIXME: precache only reg. sound
	
	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;

		i++;
	}
	
	if (soundStr)
		hook->triggerSound = gstate->SoundIndex (soundStr);

	self->touch = onlyreg_touch;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	trigger_hurt_reset
///////////////////////////////////////////////////////////////////////////////

void	trigger_hurt_reset( userEntity_t *self )
{
	self->solid = SOLID_TRIGGER;
	self->nextthink = -1;
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_hurt_use
///////////////////////////////////////////////////////////////////////////////

void	trigger_hurt_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	// toggle the hurt field on and off
	if (hook->bEnabled)
		hook->bEnabled = FALSE;
	else
		hook->bEnabled = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_hurt_touch
///////////////////////////////////////////////////////////////////////////////

void	trigger_hurt_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	CVector		damageVec(0.0f, 0.0f, 1.0f);

	if (!com->ValidTouch2(self, other))
		return;

	// the trigger_hurt is not turned on right now
	if (!hook->bEnabled)
		return;

	// play the associated sound
	if (hook->triggerSound)
		gstate->StartEntitySound(other, CHAN_OVERRIDE, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	if (other->takedamage)
	{
		self->solid = SOLID_NOT;
		if (self->spawnflags & FLOWTHRU_DMG)
		{
			damageVec = other->velocity;	// continue moving in direction of travel
			damageVec.Normalize();
			client_deathtype = KILLBRUSH;			
			com->Damage (other, self, self, damageVec, damageVec, hook->damage, DAMAGE_INERTIAL | DAMAGE_NO_BLOOD);
		}
		else
		{
			client_deathtype = KILLBRUSH;			
			com->Damage (other, self, self, damageVec, damageVec, hook->damage, DAMAGE_TRIGGER | DAMAGE_NO_BLOOD);
		}
	}

	self->think = trigger_hurt_reset;
	self->nextthink = gstate->time + hook->wait;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_hurt
///////////////////////////////////////////////////////////////////////////////

void	trigger_hurt( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	// FIXME: precache only reg. sound
	
	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	// by default, a trigger_hurt is enabled... you can target it (use it)
	// and it will switch this toggle on and off
	hook->bEnabled = TRUE;

	if (self->spawnflags & ALLOW_TOGGLE)
	{
		// this allows it to toggle on and off
		self->use = trigger_hurt_use;

		// disable it at the beginning
		if (self->spawnflags & START_DISABLED)
			hook->bEnabled = FALSE;
	}
	else
	{
		// you can not toggle this trigger on and off
		self->use = NULL;
	}

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "dmg") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "wait") == 0)
			hook->wait = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}

	if (soundStr)
		hook->triggerSound = gstate->SoundIndex (soundStr);

	self->touch = trigger_hurt_touch;

	if (!hook->damage)
		hook->damage = 5;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_push_use
///////////////////////////////////////////////////////////////////////////////

#define	PUSH_ONCE			0x00000001
#define PUSH_TOGGLE			0x00000002
#define	PUSH_START_DISABLED	0x00000004

// SCG[6/24/99]: 
void	trigger_push_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	// toggle the trigger_push
	if (hook->bEnabled)
		hook->bEnabled = FALSE;
	else
		hook->bEnabled = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_push_touch
///////////////////////////////////////////////////////////////////////////////

void	trigger_push_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	CVector		pushVelocity;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	if( hook->bEnabled == FALSE )
	{
		return;
	}

	if (other && other->className)
	{
		// TODO: add pushes for certain weapon types
		pushVelocity = hook->speed * self->movedir * 10.0;

		// push other
		other->velocity = pushVelocity;

		if (other->health)
		{
			if (stricmp (other->className, "player") == 0)
			{
				if (hook->triggerSound)
					gstate->StartEntitySound(other, CHAN_AUTO, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			}
		}

		if ((int)self->spawnflags & PUSH_ONCE)
			self->remove (self);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_push
//
///////////////////////////////////////////////////////////////////////////////

void	trigger_push( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	trigger_init (self);

	// this makes sure movedir is assigned 
	if ( self->s.angles.y == 0 )
	{
		self->s.angles.y = 360;
	}

	com->SetMovedir (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}
	
	if (soundStr)
		hook->triggerSound = gstate->SoundIndex (soundStr);

	self->touch = trigger_push_touch;

	// SCG[6/24/99]: 
	// disable it at the beginning
	if( self->spawnflags & PUSH_START_DISABLED )
	{
		hook->bEnabled = FALSE;
	}
	else
	{
		hook->bEnabled = TRUE;
	}

	if( self->spawnflags & PUSH_TOGGLE )
	{
		// this allows it to toggle on and off
		self->use = trigger_push_use;
	}
	else
	{
		// you can not toggle this trigger on and off
		self->use = NULL;
	}

	if (!hook->speed)
		hook->speed = 1000.0;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_counter_use
///////////////////////////////////////////////////////////////////////////////


void trigger_counter_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;
	
	if (hook->count == 0)
		return;
	
	hook->count--;

	if (hook->count)
	{
		if (! (self->spawnflags & SPAWNFLAG_NOMESSAGE))
		{
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierStr("%i %s...\n",2);
			gstate->cs.SendInteger(hook->count);
			gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_world,T_TRIGGERS_COUNTER));
			gstate->cs.Unicast(activator, CS_print_center,2.0);
			gstate->cs.EndSendString();
		}
		return;
	}
	
	if ( !(self->spawnflags & SPAWNFLAG_NOMESSAGE) && (activator->flags & FL_CLIENT))
	{
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s!\n",1);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_world,T_TRIGGERS_SEQUENCE_COMPLETE));
		gstate->cs.Unicast(activator, CS_print_center,2.0);
		gstate->cs.EndSendString();
	}

	self->enemy = activator;  //&&& this was self->activator ... 
	self->flags |= FL_POSSESSED;
	//multi_trigger (self, self, self, TRUE);
	multi_trigger (self, self, activator, TRUE);
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_counter
///////////////////////////////////////////////////////////////////////////////

void	trigger_counter( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			hook->triggerSound = gstate->SoundIndex( self->epair[i].value );
		else if (stricmp (self->epair [i].key, "message") == 0)
			self->message = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "count") == 0)
			hook->count = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}

	if (soundStr)
		hook->triggerSound = gstate->SoundIndex (soundStr);

	hook->wait = -1.0;

	if (!hook->count)
		hook->count = 2;

	self->use = trigger_counter_use;
}

///////////////////////////////////////////////////////////////////////////////
//	changelevel_execute
///////////////////////////////////////////////////////////////////////////////
#define PROTOPOD_HATCHED	0x0001
void changelevel_add_transients( userEntity_t *player )
{
	int				i, count = 0;
	float			fLength;
	CVector			vOrigin;
	userEntity_t	*pEnt, *pSuperfly, *pMikiko;
	playerHook_t	*hook;
	trace_t			trace;

	pSuperfly = AIINFO_GetSuperfly();
	pMikiko = AIINFO_GetMikiko();

	memset( gstate->game->transients, 0, 8 * sizeof( transient_t ) );
	for( i = 1, count = 0; i < gstate->game->maxentities; i++ )
	{
		pEnt = &gstate->g_edicts[i];
		if( !( pEnt->flags & FL_MONSTER ))
		{
			continue;
		}

		if( pEnt->enemy == NULL )
		{
			continue;
		}

		if( ( pEnt->enemy != player ) && ( pEnt->enemy != pSuperfly ) && ( pEnt->enemy != pMikiko ) )
		{
			continue;
		}

		if (pEnt->deadflag != DEAD_NO)
		{
			continue;
		}

		// check for monster-specific stuff...
		hook = AI_GetPlayerHook(pEnt);
		if (hook)
		{
			switch(hook->type)
			{
			case TYPE_PROTOPOD:
				if ( hook->nFlags & PROTOPOD_HATCHED ) continue;
				break;
			default:
				break;
			};
		}
		
		vOrigin = player->s.origin - pEnt->s.origin;
		fLength = vOrigin.Length();

		trace = gstate->TraceLine_q2( player->s.origin, pEnt->s.origin, 0, MASK_SOLID );
		// SCG[12/12/99]: Add entity to the transient list and remove it from the world
		if( fLength <= 256.0 || trace.fraction == 1.0 )
		{
			gstate->game->transients[count].vOffset = pEnt->s.origin - player->s.origin;
			gstate->game->transients[count].spawnflags = pEnt->spawnflags;
			gstate->game->transients[count].health = pEnt->health;
			strcpy( gstate->game->transients[count].className, pEnt->className );
			gstate->RemoveEntity( pEnt );
			count++;
		}

		if( count >= 8 )
		{
			break;
		}
	}
}

#define CHANGELEVEL_INTERMISSION		0x0001
#define CHANGELEVEL_REQUIRES_SUPERFLY	0x0002
#define CHANGELEVEL_REQUIRES_MIKIKO		0x0004
#define CHANGELEVEL_THE_END				0x0008

void Client_BeginIntermission( char *nextMap, qboolean bShowStats );

void changelevel_execute( userEntity_t *self )
{
	char	msg [256];
	trigHook_t *pTrigHook = ( trigHook_t * ) self->userHook;

/*	if( ( self->spawnflags & CHANGELEVEL_THE_END ) && 
		( pTrigHook->activator != NULL ) &&
		( pTrigHook->activator->client != NULL )
		)
	{
		gstate->WriteByte( SVC_END_OF_GAME );
		gstate->UniCast( pTrigHook->activator, true );

		// cek[2-5-00] so we can prevent icon/hud drawing
		if (pTrigHook->activator->flags & FL_CLIENT)
			pTrigHook->activator->client->ps.rdflags |= RDF_LETTERBOX;
	}
*/
	userEntity_t *pSuperfly = AIINFO_GetSuperfly();
	if( pSuperfly == NULL )
	{
		pSuperfly = AIINFO_GetMikikofly();
	}

	userEntity_t *pMikiko = AIINFO_GetMikiko();
	// SCG[2/6/00]: oh god this sucks.  Sidekicks should not save during changelevel...
	if( pSuperfly != NULL && ( self->spawnflags & CHANGELEVEL_REQUIRES_SUPERFLY ) )
	{
		pSuperfly->flags |= FL_NOSAVE;
	}
	if( pMikiko != NULL && ( self->spawnflags & CHANGELEVEL_REQUIRES_MIKIKO ) )
	{
		pMikiko->flags |= FL_NOSAVE;
	}

	int length = strlen( self->message );
	char test = self->message[length - 1];
	int type = com->TransitionType(gstate->level->mapname,self->message,false);
	test = tolower(test);

	char nextMap[64];
	if ( (type == 2) || (type == 0) )
	{
		Com_sprintf (nextMap, sizeof(nextMap),"*%s", self->message);
	}
	else
	{
		Com_sprintf (nextMap, sizeof(nextMap),"%s", self->message);
	}

	if( !coop->value && 
		( deathmatch->value || ( ( test == 'a' ) && ( type != 1 ) ) ) )
	{
		Com_sprintf (msg, sizeof(msg),"changelevel %s\n", nextMap);
	}
	else if( ( coop->value ) || ( self->spawnflags & CHANGELEVEL_THE_END ) )
	{
		Client_BeginIntermission( self->message, !( self->spawnflags & CHANGELEVEL_THE_END ) );
	}
	else
	{
		Com_sprintf (msg, sizeof(msg),"changelevel %s noplaque\n", nextMap);
	}

	gstate->CBuf_AddText (msg);

//	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_changelevel_use
///////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        AI_DetermineAttributeModification
// Description: Determines the modification for monsters based off of the player's skill
// Input:NA
// Output:NA
// ----------------------------------------------------------------------------
void AI_DetermineAttributeModification( void )
{
	playerHook_t *hook;
	float SpeedM,VitalityM,PowerM;
	int		num_clients = 0;
	
	// NSS[3/6/00]:Parse through all of the clients
	userEntity_t *head = alist_FirstEntity( client_list );
	while ( head != NULL )
	{
		if(head->flags & FL_CLIENT)
		{
			hook = AI_GetPlayerHook(head);
			if(hook)
			{
				SpeedM		+= hook->base_speed;
				VitalityM	+= hook->base_health;
				PowerM		+= hook->base_power;
				num_clients ++;
			}
		}
		head = alist_NextEntity( client_list );
	}
	// NSS[3/6/00]:if we have more than 1 client(i.e. COOP) then we should average the value out.
	if(num_clients > 1)
	{
		SpeedM		= SpeedM/num_clients;
		VitalityM	= VitalityM/num_clients;
		PowerM		= PowerM/num_clients;
	}
	// NSS[3/6/00]:Make sure we are at least 1.0f
	if(SpeedM < 1.0f)
		SpeedM		= 1.0f;
	if(VitalityM < 1.0f)
		VitalityM	= 1.0f;
	if(PowerM < 1.0f)
		PowerM		= 1.0f;
	
	// NSS[3/6/00]:Make sure it is not too high
	if(SpeedM > 5.0f)
		SpeedM		= 5.0f;
	if(VitalityM > 5.0f)
		VitalityM	= 5.0f;
	if(PowerM > 5.0f)
		PowerM		= 5.0f;
	
	// NSS[3/6/00]:
	gstate->game->SpeedM		= SpeedM;
	gstate->game->VitalityM		= VitalityM;
	gstate->game->PowerM		= PowerM;
}


void trigger_changelevel_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator);

void trigger_changelevel_cinematic_think( userEntity_t *self )
{
	if( gstate->bCinematicPlaying == TRUE )
	{
		self->nextthink = gstate->time + 1;
	}
	else
	{
		trigHook_t *pTrigHook = ( trigHook_t * ) self->userHook;
		_ASSERTE( pTrigHook->activator );
		trigger_changelevel_use( self, pTrigHook->activator, pTrigHook->activator );
	}
}

void trigger_changelevel_delay_think( userEntity_t *self )
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

void SpawnDelay2HackShitFuckThisSucksAss( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	userEntity_t	*temp;

	temp = gstate->SpawnEntity();
	temp->className = "DelayedUse";
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->owner = activator;
	temp->enemy = self;
	temp->groundEntity = other;

	temp->nextthink = gstate->time + 1.0;

	temp->think = trigger_changelevel_delay_think;
	temp->message = self->message;
	temp->target = self->target;
	temp->killtarget = self->killtarget;
}

extern void CIN_RemoveAllEntities();
void trigger_changelevel_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigHook_t *pTrigHook = ( trigHook_t * ) self->userHook;
	int i;

	// SCG[3/23/00]: make sure there are no dead clients when changing a level...
	for( i = 0; i < gstate->game->maxclients; i++ )
	{
		if( gstate->g_edicts[i + 1].deadflag != DEAD_NO )
		{
			if( coop->value != 0 )
			{
				SpawnDelay2HackShitFuckThisSucksAss( other, self, activator );
			}
			return;
		}
	}
	// SCG[12/22/99]: Save the time
	for( i = 0; i < gstate->game->maxclients; i++ )
	{
		gstate->game->clients[i].pers.nTime[gstate->subMap] = gstate->level->time;
	}

	if( ( self->spawnflags & CHANGELEVEL_THE_END ) )
	{
		userEntity_t *pClient;

		for( int i = 0; i < maxclients->value; i++ )
		{
			pClient = &gstate->g_edicts[i + 1];

			if( !pClient->inuse )
			{
				continue;
			}

			gstate->WriteByte( SVC_END_OF_GAME );
			gstate->UniCast( pClient, true );

			// cek[2-5-00] so we can prevent icon/hud drawing
			if( pClient->flags & FL_CLIENT )
			{
				pClient->client->ps.rdflags |= RDF_LETTERBOX;
			}
		}
	}

	// SCG[11/18/99]: check for cinematic script
	if( ( sv_cinematics->value != 0 ) && 
		( coop->value == 0 ) && 
		( deathmatch->value == 0 ) && 
		( pTrigHook != NULL ) && 
		( pTrigHook->pCineScript != NULL ) ) 
	{
		char buf[128];
		if( other->flags & FL_CLIENT )
		{
			pTrigHook->activator = other;
		}
		else if( activator->flags & FL_CLIENT )
		{
			pTrigHook->activator = activator;
		}
		else
		{
			// NSS[2/17/00]:Activator Cannot be NULL
			pTrigHook->activator = self;
			//pTrigHook->activator = NULL;
		}
		self->think = trigger_changelevel_cinematic_think;
		self->nextthink = gstate->time + 1;
		Com_sprintf( buf, sizeof(buf),"cin_load_and_play %s\n", pTrigHook->pCineScript );
		gstate->CBuf_AddText( buf );
		pTrigHook->pCineScript = NULL;
		return;
	}
	else if(  sv_cinematics->value == 0 )
	{
		if( pTrigHook->pCineScript == NULL )
		{
			gstate->Con_Dprintf("pTrigHook->pCineScript == NULL\n");
		}
		else
		{
			strncpy( gstate->szCinematicName, pTrigHook->pCineScript, 32 );
			CIN_RemoveAllEntities();
		}
	}

	// SCG[10/18/99]: make sure the map name is valid
	if( strcmp( self->message, gstate->mapName ) == 0 )
	{
		return;
	}

	if( strncmp( gstate->mapName, self->message, 4 ) )
	{
		gstate->game->serverflags &= ~(SFL_CROSS_TRIGGER_MASK);
	}

	if (self->target)
	{
		strcpy( gstate->game->spawnpoint, self->target );
	}
	else
	{
		strcpy( gstate->game->spawnpoint, "" );
	}

	if (!gstate->GetCvar ("dm_allow_exiting") && deathmatch->value)
	{
		com->Damage (other, self, self, zero_vector, zero_vector, 50000, DAMAGE_EXPLOSION | DAMAGE_NO_BLOOD);
		return;
	}

	if (deathmatch->value || coop->value)
		gstate->bprint ("%s %s\n", com->ProperNameOf(other), tongue_world[T_PLAYER_EXITED_LEVEL]);

	com->UseTargets (self, other, other);

	// SCG[12/12/99]: Add transient entities
	int length = strlen( self->message );
	char test = self->message[length - 1];
	int type = com->TransitionType(gstate->level->mapname,self->message,false);
	test = tolower(test);
	if( ( ( test != 'a' ) || ( ( test == 'a' ) && ( type == 1 ) ) ) && ( deathmatch->value == 0 ) )
	{
		changelevel_add_transients( other );
	}

	// SCG[2/2/00]: Save changelevel trigger origin;
	if( other != NULL )
	{
		if( ( other->flags & FL_CLIENT ) && ( other->client != NULL ) )
		{
			CVector vOrigin = self->absmax - self->absmin;
			vOrigin.Multiply( 0.5 );
			vOrigin += self->absmin;
			other->client->pers.vOffset = other->s.origin - vOrigin;
		}
	}

//	if ((self->spawnflags & NO_INTERMISSION) && deathmatch->value == 0)
	if( ( !( self->spawnflags & CHANGELEVEL_INTERMISSION ) || self->target ) && deathmatch->value == 0 )
	{
		changelevel_execute( self );
		return;
	}
	// SCG[8/25/99]: What kind of fucking logic is this?
//	else if (deathmatch->value || deathmatch->value == 0)
	else if (deathmatch->value != 0 || ( self->spawnflags & CHANGELEVEL_INTERMISSION ) )
	{
		Client_BeginIntermission( self->message, TRUE );
	}
	else
	{
		self->think = changelevel_execute;
		self->nextthink = gstate->time + 0.1;
	}
	// NSS[3/6/00]:Will setup the attribute modifications for the next level.
	AI_DetermineAttributeModification();
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_sidekick_detect 
//  0 Everything is ok go ahead and exit.
//  1 Need Mikiko
//  2 Need Superfly
//  3 Need Both
///////////////////////////////////////////////////////////////////////////////
void trigger_changelevel_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);
void trigger_script_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);


void trigger_changelevel_message_reset( userEntity_t *self )
{
	if(_stricmp(self->className,"trigger_changelevel")==0)
	{
		self->touch = trigger_changelevel_touch;
	}
	else if(_stricmp(self->className,"trigger_script")==0)
	{
		self->touch = trigger_script_touch;
	}
	else
	{
		AI_Dprintf("Trigger is not of type changelevel or script!\n");
		self->touch = NULL;
	}
	self->think = NULL;
}

int trigger_sidekick_detect(userEntity_t *self, userEntity_t *other, int Mikiko_Flag, int SuperFly_Flag, int tongue_message)
{
	userEntity_t	*pSuperflyOwner, *pMikikoOwner;
	playerHook_t	*pHook;

	char			message[128];
	int				Value = 0;

	if( coop->value || deathmatch->value )
	{
		return 0;
	}
	
	// SCG[10/18/99]: make sure we do not leave without the sidekicks
	// SCG[11/17/99]: These are specified in spawnflags
	qboolean	bSuperflyVisible, bMikikoVisible;
	qboolean	bRequiresSuperfly = FALSE, bRequiresMikiko = FALSE;
	qboolean	bDisplaySuperflyMessage = FALSE, bDisplayMikikoMessage = FALSE;
	bSuperflyVisible = bMikikoVisible = FALSE;
	pSuperflyOwner = pMikikoOwner = NULL;

	userEntity_t *pSidekick;

	float fDistance;

	// SCG[11/18/99]: If we need Superfly, make sure he is in range
	if( self->spawnflags & SuperFly_Flag )
	{
		bRequiresSuperfly = TRUE;
		pSidekick = AIINFO_GetSuperfly();
		
		// SCG[2/6/00]: If we didnt get Superfly, lets try to get Mikokofly in case we are on one of those levels.
		if( pSidekick == NULL )
		{
			pSidekick = AIINFO_GetMikikofly();
		}

		if( pSidekick )
		{
			pHook = AI_GetPlayerHook( pSidekick );
			pSuperflyOwner = pHook->owner;
			if( pSuperflyOwner )
			{
//				bSuperflyVisible = SIDEKICK_IsOwnerClose( pSidekick );
			    fDistance = VectorDistance( pSidekick->s.origin, other->s.origin );
				bSuperflyVisible = ( fabs( fDistance ) < 150.0 ) ? TRUE : FALSE;
			}
		}
	}

	// SCG[11/18/99]: If we need Mikiko, make sure she is in range
	if( self->spawnflags & Mikiko_Flag )
	{
		bRequiresMikiko = TRUE;
		pSidekick = AIINFO_GetMikiko();
		if( pSidekick )
		{
			pHook = AI_GetPlayerHook( pSidekick );
			pMikikoOwner = pHook->owner;
			if( pMikikoOwner )
			{
//				bMikikoVisible = SIDEKICK_IsOwnerClose( pSidekick );
			    fDistance = VectorDistance( pSidekick->s.origin, other->s.origin );
				bMikikoVisible = ( fabs( fDistance ) < 150.0 ) ? TRUE : FALSE;
			}
		}
	}

	if( bRequiresSuperfly == TRUE && bSuperflyVisible == FALSE ) bDisplaySuperflyMessage = TRUE;
	if( bRequiresMikiko == TRUE && bMikikoVisible == FALSE ) bDisplayMikikoMessage = TRUE;

	if( ( bRequiresSuperfly && !bSuperflyVisible ) || ( bRequiresMikiko && !bMikikoVisible ) )
	{
// changed by yokoyama for Japanese version
#ifndef JPN  // Original
		if( bDisplaySuperflyMessage && bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s and %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 3;
		}
		else if( bDisplaySuperflyMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON] );
			Value = 2;
		}
		else if( bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 1;

		}

#else // for JPN

#ifdef MAN // for MAN
		if( bDisplaySuperflyMessage && bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s and %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 3;
		}
		else if( bDisplaySuperflyMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON] );
			Value = 2;
		}
		else if( bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 1;

		}
#else
#ifdef TIW // for TIW
		if( bDisplaySuperflyMessage && bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s and %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 3;
		}
		else if( bDisplaySuperflyMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_SUPERFLY_JOHNSON] );
			Value = 2;
		}
		else if( bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[tongue_message], 
				tongue_world[T_MIKIKO_IBIHARA] );
			Value = 1;

		}
#else // for JPN
		if( bDisplaySuperflyMessage && bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s , %s\n%s\n", 
				tongue_world[T_SUPERFLY_JOHNSON], 
				tongue_world[T_MIKIKO_IBIHARA],
				tongue_world[tongue_message]);
			Value = 3;
		}
		else if( bDisplaySuperflyMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[T_SUPERFLY_JOHNSON],
				tongue_world[tongue_message]);
			Value = 2;
		}
		else if( bDisplayMikikoMessage )
		{
			Com_sprintf( message, sizeof(message),"%s %s\n", 
				tongue_world[T_MIKIKO_IBIHARA],
				tongue_world[tongue_message]);
			Value = 1;

		}
#endif // TIW
#endif // MAN
#endif // JPN

		gstate->centerprint( other, 2.0, message );
		self->touch = NULL;
		self->think = trigger_changelevel_message_reset;
		self->nextthink = gstate->time + 2.5;
	}
	return Value;
}


///////////////////////////////////////////////////////////////////////////////
//	AI_SIDEKICK_ALEXWANTSYOURASSTHERE
///////////////////////////////////////////////////////////////////////////////
void AI_SIDEKICK_ALEXWANTSYOURASSTHERE(userEntity_t *self)
{
	playerHook_t *pHook			= AI_GetPlayerHook( self );
	userEntity_t *pSidekickOwner= pHook->owner;
	if( pSidekickOwner )
	{
		if(!SIDEKICK_HasTaskInQue(self,TASKTYPE_FOLLOWRUNNING))
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_FOLLOWRUNNING, pSidekickOwner );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_changelevel_touch
///////////////////////////////////////////////////////////////////////////////
void trigger_changelevel_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	char			message[128];

	//NSS[11/10/99]:
	Trigger_Reject( other, ( FL_MONSTER | FL_BOT ) );
	
	if (!com->ValidTouch (self, other))
		return;

	if( !AI_IsAlive( other ) )
	{
		return;
	}

	// SCG[11/9/99]: See if we need a key
	if( com_DoKeyFunctions( self, other, tongue_world[T_NO_EXIT_WITHOUT_KEY] ) == FALSE )
	{
		return;
	}

	userEntity_t *pSuperfly, *pMikiko;
	pSuperfly = AIINFO_GetSuperfly();
	pMikiko = AIINFO_GetMikiko();

	if (coop->value)
	{
		char *name1,*name2;
		if (coop->value && !CoopIsAllOthersClose( other, 128.0f, &name1, &name2 ))
		{
			// only send the message once every second...
			if (gstate->time > self->nextthink)
			{
#ifdef JPN // added by yokoyama BUGFIX:NO.2
 #ifdef MAN // for MAN
				if (!name2)
					Com_sprintf(message, sizeof(message), "%s %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1 );
				else
					Com_sprintf( message, sizeof(message),"%s %s and %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1, name2 );


 #else
  #ifdef TIW // for TIW
				if (!name2)
					Com_sprintf(message, sizeof(message), "%s %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1 );
				else
					Com_sprintf( message, sizeof(message),"%s %s and %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1, name2 );

  #else
			
				if (!name2)
					Com_sprintf(message, sizeof(message), "%s %s\n", name1,tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS] );
				else
					Com_sprintf( message, sizeof(message),"%s ,%s %s\n", name1, name2 ,tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS]);
  #endif // TIW
 #endif // MAN
#else // for Original
				if (!name2)
					Com_sprintf(message, sizeof(message), "%s %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1 );
				else
					Com_sprintf( message, sizeof(message),"%s %s and %s\n",tongue_world[T_NO_EXIT_WITHOUT_SIDEKICKS], name1, name2 );
#endif // JPN

				gstate->centerprint( other, 2.0, message );
				self->nextthink = gstate->time + 1.0;
			}
			return;
		}
	}
	else
	{
	
		int Value = trigger_sidekick_detect(self, other, CHANGELEVEL_REQUIRES_MIKIKO, CHANGELEVEL_REQUIRES_SUPERFLY, T_NO_EXIT_WITHOUT_SIDEKICKS);
		if(Value)
		{
			// NSS[12/10/99]:Play sound when you can't leave without superfly or mikiko or both.
			switch(Value)
			{
			case 1:
				{
					gstate->StartMP3("sounds/voices/hiro/sid_h_01b.mp3", 2, 0.75, 1, FALSE );
					if( pMikiko )
					{
						AI_SIDEKICK_ALEXWANTSYOURASSTHERE( pMikiko );
					}
					break;
				}
			case 2:
				{
					gstate->StartMP3("sounds/voices/hiro/sid_h_02c.mp3", 2, 0.75, 1, FALSE );
					if( pSuperfly )
					{
						AI_SIDEKICK_ALEXWANTSYOURASSTHERE(pSuperfly);
					}
					break;
				}
			case 3:
				{
					gstate->StartMP3("sounds/voices/hiro/sid_h_03b.mp3", 2, 0.75, 1, FALSE );
					if( pSuperfly )
					{
						AI_SIDEKICK_ALEXWANTSYOURASSTHERE(pSuperfly);
					}
					if( pMikiko )
					{
						AI_SIDEKICK_ALEXWANTSYOURASSTHERE( pMikiko );
					}
					break;
				}

			}
			return;
		}
	}

	trigHook_t *pTrigHook = ( trigHook_t * ) self->userHook;

	gstate->game->serverflags &= ~SFL_SIDEKICK_MASK;

	if( self->spawnflags & CHANGELEVEL_REQUIRES_SUPERFLY )
	{
		playerHook_t *pSuperflyHook;
		if( pSuperfly != NULL )
		{
			pSuperflyHook = ( playerHook_t * ) pSuperfly->userHook;
		}
		if( AIINFO_GetMikikofly() || ( ( pSuperflyHook != NULL ) && ( pSuperflyHook->bCarryingMikiko ) ) )
		{
			gstate->game->serverflags |= SFL_SPAWN_MIKIKOFLY;
		}
		else
		{
			gstate->game->serverflags |= SFL_SPAWN_SUPERFLY;
		}
	}

	if( self->spawnflags & CHANGELEVEL_REQUIRES_MIKIKO )
	{
		gstate->game->serverflags |= SFL_SPAWN_MIKIKO;
	}

	trigger_changelevel_use (self, other, other);
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_changelevel
///////////////////////////////////////////////////////////////////////////////

void trigger_changelevel( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	if( hook == NULL )
	{
		return;
	}

	memset( hook, 0, sizeof( trigHook_t ) );
	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "sound"))
			soundStr = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "map"))
			self->message = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "keyname"))
			self->keyname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "cinematic"))
			hook->pCineScript = self->epair[i].value;
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}
	
	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

	if (!self->message)
		com->Error ("trigger_changelevel has no map.\n");

	self->touch = trigger_changelevel_touch;
	self->use = trigger_changelevel_use;

	self->save = trig_hook_save;
	self->load = trig_hook_load;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_change_sfx_touch
//
//	we've walked into a trigger to change the sound systems' playback effects
//
///////////////////////////////////////////////////////////////////////////////

void trigger_change_sfx_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	sfxTrigHook_t	*hook = (sfxTrigHook_t *) self->userHook;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	// check and make sure it's only the player or a possessed entity triggering this
	if (!other || !(other->flags & FL_CLIENT || other->flags & FL_POSSESSED))
		return;

	// okay.. now change the sound system's settings
#if 0
	if (self->spawnflags & SFX_NORMAL)
	{
		gstate->Con_Printf ("Resetting sound system parameters\n");
	}
	else if (self->spawnflags & SFX_PRESET1)
	{
		gstate->Con_Printf ("Setting sound system parameters to PRESET1\n");
	}
	else if (self->spawnflags & SFX_PRESET2)
	{
		gstate->Con_Printf ("Setting sound system parameters to PRESET2\n");
	}
	else if (self->spawnflags & SFX_PRESET3)
	{
		gstate->Con_Printf ("Setting sound system parameters to PRESET3\n");
	}
	else if (self->spawnflags & SFX_PRESET4)
	{
		gstate->Con_Printf ("Setting sound system parameters to PRESET4\n");
	}
	else
	{
		// we're not using any presets.. so process the custom settings here
		gstate->Con_Printf ("Setting CUSTOM sound system parameters\n");
	}
#endif
	// mdm99.06.02 - 0 is a valid reverb preset	
//    if (hook->fxStyle)
//    {
        gstate->SetReverbPreset (hook->fxStyle) ;
//    }
}



///////////////////////////////////////////////////////////////////////////////
//	trigger_change_sfx
//
//	changes effects used by the sound system when playing sounds
//
///////////////////////////////////////////////////////////////////////////////

void trigger_change_sfx( userEntity_t *self )
{
	int			i;
	sfxTrigHook_t	*hook;

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	gstate->SetModel (self, self->modelName);
	gstate->LinkEntity (self);

	self->userHook = gstate->X_Malloc(sizeof(sfxTrigHook_t), MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	self->save = sfx_trig_hook_save;
	self->load = sfx_trig_hook_load;

	// parse epairs
	hook = (sfxTrigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "volume"))
			hook->volume = atof(self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "reverb") || !stricmp (self->epair [i].key, "fxstyle"))
			hook->fxStyle = atoi(self->epair [i].value);
		i++;
	}

	self->touch = trigger_change_sfx_touch;
}

///////////////////////////////////////////////////////////////////////////////
//
//	trigger_changetarget_use
//
//	we've targetted a trigger to change the target of another entity
//
///////////////////////////////////////////////////////////////////////////////

void trigger_changetarget_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*pTarget = NULL;
	trigHook_t		*hook = (trigHook_t *) self->userHook;

// 	if (!com->ValidTouch (self, other))
//		return;

	pTarget = com->FindTarget(self->target);
	if (pTarget)
	{
		pTarget->target = hook->model_1;		// model_1 holds the new target name
	}

	if (!pTarget || (self->spawnflags & USE_ONCE))
	{
		self->remove(self);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_changetarget
//
//	changes target of another entity
//
///////////////////////////////////////////////////////////////////////////////

void trigger_changetarget( userEntity_t *self )
{
	int			i = 0;
	trigHook_t	*hook = NULL;

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	hook = (trigHook_t*) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;

	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "newtarget"))
			hook->model_1 = self->epair [i].value;			// hack but why waste memory
		i++;
	}

	// if we don't have a target or a new target name, remove the entity
	if (!self->target || !hook->model_1)
	{
		gstate->Con_Dprintf ("WARNING: invalid trigger_changetarget at %s\n",com->vtos(self->s.origin));
		self->remove(self);
	}

	self->touch = NULL;
	self->use = trigger_changetarget_use;
}



///////////////////////////////////////////////////////////////////////////////
//	trigger_script_use
//
///////////////////////////////////////////////////////////////////////////////
#define TRIGGER_SCRIPT_SUPERFLY_NEEDED	0x04
#define TRIGGER_SCRIPT_MIKIKO_NEEDED	0x08

void trigger_script_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	scriptTrigHook_t	*hook = (scriptTrigHook_t *) self->userHook;
	char	buf[256];

	// see below.. there is a delay/wait between CINE_MULTIPLE re-triggers
	if (gstate->time < hook->nextTriggerTime)
		return;

	qboolean bOtherCanTrigger = true;
	qboolean bActivatorCanTrigger = true;
	// check and make sure it's only the player or a possessed entity triggering this
	if (!other || !(other->flags & (FL_CLIENT|FL_MONSTER|FL_BOT) || other->flags & FL_POSSESSED))
	{		
		bOtherCanTrigger = false;
	}

	if (!activator || !(activator->flags & FL_CLIENT || activator->flags & FL_POSSESSED))
	{		
		bActivatorCanTrigger = false;
	}

	if( ( bOtherCanTrigger == false ) && ( bActivatorCanTrigger = false ) )
	{
		return;
	}

	// SCG[11/9/99]: See if we need a key
	if( com_DoKeyFunctions( self, other, NULL ) == FALSE )
	{
		return;
	}

	// SCG[2/15/00]: this was moved here to prevent multiple messages
	// SCG[2/15/00]: First, we early out with the key check if need be, 
	// SCG[2/15/00]: otherwise we check for sidekicks ( if need be )
	int Value = trigger_sidekick_detect(self, other, TRIGGER_SCRIPT_MIKIKO_NEEDED, TRIGGER_SCRIPT_SUPERFLY_NEEDED, T_MUST_HAVE_KEY);
	if(Value)
	{

		// NSS[12/10/99]:Play sound when you can't trigger a script without mikiko or superfly or both
		switch(Value)
		{
		case 1:
			{
				PlaySidekickMP3(self,"sounds/voices/mikiko/sid_m_27.mp3");
				//gstate->StartMP3("sounds/voices/mikiko/sid_m_27.mp3", 2, 0.75, 1 );
				break;
			}
		case 2:
			{
				PlaySidekickMP3(self,"sounds/voices/superfly/sid_s_27.mp3");
				//gstate->StartMP3("sounds/voices/superfly/sid_s_27.mp3", 2, 0.75, 1 );
				break;
			}
		case 3:
			{
				gstate->StartMP3("sounds/voices/hiro/sid_h_03b.mp3", 2, 0.75, 1 );
				break;
			}

		}
		
		return;
	}

/*
	if( self->keyname )
	{
		userEntity_t *pKey = com->FindEntity( self->keyname );
		if( other->inventory )
		{
			userInventory_t *pKeyItem = gstate->InventoryFindItem( other->inventory, self->keyname );

			// did the player have the key?
			if( pKeyItem == NULL )
			{
				// NSS[12/8/99]:To pause and keep the message from blinking like a fag.
				if( pKey != NULL && (self->hacks < gstate->time) )
				{
					self->hacks = gstate->time + 4.50f;
				}

				return;
			}
			else if( pKeyItem->flags & ITF_USEONCE )
			{
				gstate->InventoryDeleteItem( other, other->inventory, pKeyItem );
			}
		}
	}
*/

	// run the cinematic
	if (hook->pCineScript)
	{
		if( ( sv_cinematics->value != 0 ) && ( coop->value == 0 ) && ( deathmatch->value == 0 ) )
		{
			gstate->CBuf_AddText("cin_cancel_shots\n");
//			sprintf(buf, "cin_load_and_play %s\n",hook->pCineScript);
			Com_sprintf(buf, sizeof(buf),"cin_load_and_play %s\n",hook->pCineScript);
			gstate->CBuf_AddText(buf);
		}
		else
		{
			strncpy( gstate->szCinematicName, hook->pCineScript, 32 );
			CIN_RemoveAllEntities();
		}
	}
	else if (hook->pAiScript)
	{
		// NSS[2/3/00]:Add the new script to the entity
		AI_AddNewScriptActionGoal( self, hook->pAiScript);
	}

	// delete it unless this is a trigger that can spawn cinematics multiple times
	if (!(self->spawnflags & SCRIPT_MULTIPLE))
	{
		self->remove(self);
	}
	else
	{
		// make sure we don't trigger for at least hook->wait (hook->wait defaults to 2 secs)
		hook->nextTriggerTime = gstate->time + hook->wait;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_script_touch
//
///////////////////////////////////////////////////////////////////////////////

void trigger_script_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
//	scriptTrigHook_t	*hook = (scriptTrigHook_t *) self->userHook;// SCG[1/23/00]: not used

	//NSS[11/10/99]:
	// SCG[12/6/99]: Sidekicks should not trigger cinematics
	Trigger_Reject( other, ( FL_MONSTER | FL_BOT ) );

	// SCG[12/3/99]: Brushes should not trigger cinematics
	if (!com->ValidTouch (self, other))
		return;

	// if this trigger was touched and SCRIPT_NOTOUCH is set, blow it off
	if (self->spawnflags & SCRIPT_NOTOUCH)
		return;
	
	trigger_script_use (self, other, other);
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_script
//
//	triggers an ingame cinematic or AI script
//
///////////////////////////////////////////////////////////////////////////////

void trigger_script( userEntity_t *self )
{
	int				i;
	scriptTrigHook_t	*hook;

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	gstate->SetModel (self, self->modelName);
	gstate->LinkEntity (self);

	self->userHook = gstate->X_Malloc(sizeof(scriptTrigHook_t), MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	self->save = script_hook_save;
	self->load = script_hook_load;

	// parse epairs
	hook = (scriptTrigHook_t *) self->userHook;

	// set some defaults
	hook->wait = -1;
	hook->nextTriggerTime = 0;
	self->delay = 0;

	self->keyname = NULL;
	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof(self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cinescript"))
			hook->pCineScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "aiscript"))
			hook->pAiScript = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "keyname"))
			self->keyname = self->epair [i].value;
		i++;
	}

	// delay defaults to 2 seconds
	if (hook->wait == -1)
		hook->wait = 2.0;

	// set up the touch function
	self->touch = trigger_script_touch;

	// set up the use function so this trigger can be "used" as well
	self->use = trigger_script_use;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_cinematic
//
//	this entity is now obsolete
//
///////////////////////////////////////////////////////////////////////////////

void trigger_cinematic( userEntity_t *self )
{
	gstate->Con_Dprintf ("WARNING: obsolete trigger_cinematic at %s\n",com->vtos(self->s.origin));
	self->remove(self);
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_monstrjump_touch
///////////////////////////////////////////////////////////////////////////////
/*
#ifdef	OLD
void	trigger_monsterjump_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigHook_t	*hook = (trigHook_t *) self->userHook;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	other->velocity [0] = self->movedir [0] * hook->speed;
	other->velocity [0] = self->movedir [0] * hook->speed;

	if (other->groundEntity == NULL)
		return;
	
	other->groundEntity = NULL;

	other->velocity [2]  = hook->damage;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_monstrjump
///////////////////////////////////////////////////////////////////////////////

void	trigger_monsterjump( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;


	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "height") == 0)
			hook->damage = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "flag") == 0)
		{
			if (stricmp (self->epair [i].value, "NO_MESSAGE") == 0)
				self->spawnflags = self->spawnflags | SPAWNFLAG_NOMESSAGE;
		}
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}
	
	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

	if (!hook->speed)
		hook->speed = 200;
	if (!hook->damage)
		hook->damage = 200;

	if ( self->s.angles.Length() == 0.0 )
	{
		ZeroVector (self->s.angles);
		self->s.angles.x = 360.0;
	}

	self->touch = trigger_monsterjump_touch;
}
#endif
*/
///////////////////////////////////////////////////////////////////////////////
//	rock_water_friction
//
//	adds friction and random motion to an object that is 
//	travelling through water
///////////////////////////////////////////////////////////////////////////////

void	rock_water_friction( userEntity_t *self )
{
	rockHook_t	*hook = (rockHook_t *) self->userHook;
	CVector			dir, ang;
	float			pc, wobble_speed, wander_speed;

	pc = gstate->PointContents (self->s.origin);
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
		// just hit water, so slow down
		hook->hit_water_pos = self->s.origin;
		dir = self->velocity * 0.5 * hook->mass_scale;
		self->velocity = dir;
		hook->change_time = gstate->time;

		// decrease effect of gravity
		self->gravity = 0.5 * hook->mass_scale;		
		gstate->Con_Dprintf ("gravity decreased to %f\n", self->gravity);

//		gstate->sound (self, CHAN_AUTO, "bloop.wav", 255, ATTN_NORM);
	}
	else if ((self->movetype == MOVETYPE_BOUNCE) || (self->movetype == MOVETYPE_TOSS))
	{
		if (hook->change_time <= gstate->time)
		{
			hook->hit_water_pos.x = self->s.origin [2] + (rnd () - 0.2) * (rnd () * 256.0 + 32.0);

			dir = hook->hit_water_pos - self->s.origin;
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

			// change angular velocity because grenade just bounced
			self->avelocity.Set( hook->rotation_scale * (rnd() - 0.5) * 700.0, hook->rotation_scale * (rnd() - 0.5) * 700.0, hook->rotation_scale * (rnd() - 0.5) * 700.0);
		}

		self->velocity.x = self->velocity.x * 0.5 * hook->mass_scale;	
		self->velocity.y = self->velocity.y * 0.5 * hook->mass_scale;	
	}

	hook->last_water = pc;
}

///////////////////////////////////////////////////////////////////////////////
//	rock_touch
///////////////////////////////////////////////////////////////////////////////

void	rock_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	rockHook_t	*hook = (rockHook_t *) self->userHook;
	CVector		temp;

	
	if (!com->ValidTouch (self, other))
		return;

	temp = hook->last_pos - self->s.origin;

	hook->last_pos = self->s.origin;

	if ( temp.Length() < 1.0 )
	{
		self->velocity.Zero();
		self->avelocity.Zero();
		self->s.angles.x = 0.0;

	//	gstate->SetSize (self, -8.0, -8.0, -8.0, 8.0, 8.0, 8.0);

		if (other == self->owner)
		{
			temp = other->s.angles;
			temp.AngleToVectors(forward, right, up);

			temp = forward * 32.0 + other->s.origin;
	
			gstate->SetOrigin (self, temp.x, temp.y, temp.z);

			temp = forward * other->velocity.Length() * 2.2;
			self->velocity = temp;
			self->velocity.z = self->velocity.z + other->velocity.Length() / 2 + 100.0;
		
			return;
		}
	}

	if (gstate->time < hook->last_touch_time + 0.1)
		return;

	if ( self->velocity.Length() != 0.0)
	{
		// change angular velocity because grenade just bounced
		self->avelocity.Set( hook->rotation_scale * (rnd () - 0.5) * 1400.0, hook->rotation_scale * (rnd () - 0.5) * 1400.0, hook->rotation_scale * (rnd () - 0.5) * 1400.0);

		temp = self->avelocity;
		temp = self->size;
		
//		gstate->sound (self, CHAN_AUTO, "bounce.wav", 255, ATTN_NORM);
	}

	hook->last_touch_time = gstate->time;
}

///////////////////////////////////////////////////////////////////////////////
//	rock_think
///////////////////////////////////////////////////////////////////////////////

void	rock_think( userEntity_t *self )
{
	if (gstate->time >= self->delay)
	{
		self->remove (self);
		return;
	}

	rock_water_friction (self);

	self->nextthink = gstate->time + 0.1;
	self->think = rock_think;
}

///////////////////////////////////////////////////////////////////////////////
//	spawn_rock
///////////////////////////////////////////////////////////////////////////////

void	spawn_rock( userEntity_t *self, CVector &org)
{
	trigHook_t		*ohook = (trigHook_t *) self->userHook;
	rockHook_t		*hook;
	userEntity_t	*temp;
	CVector			mins, maxs;
	int				r;
	float			scale;

	///////////////////////////////////////////////////////////////////////////
	//	get size and determine if we're spawning inside a solid
	///////////////////////////////////////////////////////////////////////////

	scale = rnd () * 1.2 + 0.8;
	
	mins = CVector(-8, -8, -8) * scale;
	maxs = CVector(8, 8, 8) * scale;

	tr = gstate->TraceBox_q2(org, mins, maxs, org, self, MASK_SOLID);
	if (tr.startsolid || tr.allsolid)
	{
		//	spawning in a solid, so just abort
		return;
	}

	temp = gstate->SpawnEntity ();

	temp->s.render_scale.Set( scale, scale, scale );
	temp->s.mins = mins;
	temp->s.maxs = maxs;

	temp->solid = SOLID_NOT;
	temp->takedamage = DAMAGE_NO;
	temp->movetype = MOVETYPE_BOUNCE;
	temp->svflags |= SVF_MONSTER | SVF_SHOT;
	temp->clipmask = MASK_SHOT;
/*
	org.x = org.x + (rnd () * 40) - 20;
	org.y = org.y + (rnd () * 40) - 20;
	org.z = org.z + (rnd () * 40) - 20;
*/
	gstate->SetOrigin (temp, org.x, org.y, org.z);

 	r = rand () & 3;

	if (r == 0)
		temp->s.modelindex = gstate->ModelIndex (ohook->model_1);
	else if (r == 1)
		temp->s.modelindex = gstate->ModelIndex (ohook->model_2);
	else
		temp->s.modelindex = gstate->ModelIndex (ohook->model_3);

	temp->avelocity.x = rnd() * 1400 - 700;
	temp->avelocity.y = rnd() * 1400 - 700;
	temp->avelocity.z = rnd() * 1400 - 700;

	temp->velocity.x = rnd() * 500 - 250;
	temp->velocity.y = rnd() * 500 - 250;
	temp->velocity.z = 270;

	temp->delay = gstate->time + 10.0 + rnd () * 10.0;

	if ((self->spawnflags & EXTRA_VELOCITY)) 
	{
		temp->velocity.x = temp->velocity.x * 2.0;
		temp->velocity.y = temp->velocity.y * 2.0;
		temp->velocity.z = temp->velocity.z * 2.0;
	}

	temp->userHook = gstate->X_Malloc(sizeof(rockHook_t), MEM_TAG_HOOK);
	hook = (rockHook_t *) temp->userHook;

	temp->save = rock_hook_save;
	temp->load = rock_hook_load;

	// used to scale rotational velocity
	hook->rotation_scale = 0.9;
	hook->mass_scale = 1.0;
	hook->last_water = gstate->PointContents (temp->s.origin);
	hook->hit_water_pos = temp->s.origin;

	temp->think = rock_think;
	temp->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//	blow_up
///////////////////////////////////////////////////////////////////////////////

void	blow_up( userEntity_t *self )
{
	int				i, r;
	CVector			org;
	CVector			rock_org;
	float			x_width, y_width, z_width;

	// throw rock gibbage
	if (self->spawnflags & EXTRA_CHUNKS)
		r = rand () & 3;
	else
		r = 0;

	//	get width, height and depth of func_wall_explode
	x_width = self->s.maxs.x - self->s.mins.x;
	y_width = self->s.maxs.y - self->s.mins.y;	
	z_width = self->s.maxs.z - self->s.mins.z;

	for (i = 0; i <= r; i++)
	{
		rock_org.Set( self->s.mins.x + x_width * rnd(), 
					  self->s.mins.y + y_width * rnd(),
					  self->s.mins.z + z_width * rnd() );
		spawn_rock (self, rock_org);
	}

	self->nextthink = -1;

	org = self->s.origin;

	if (!(self->spawnflags & NO_EXPLOSIONS))
		spawn_sprite_explosion (self, org, (int)self->delay);

	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	spawn_explosion
///////////////////////////////////////////////////////////////////////////////

float	spawn_explosion( userEntity_t *wall, CVector &org, int play_sound)
{
	userEntity_t	*temp;
	float			time;

	temp = gstate->SpawnEntity ();

	//	copy func_wall_explode's user hook
	temp->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	memcpy (temp->userHook, wall->userHook, sizeof (trigHook_t));

	temp->save = trig_hook_save;
	temp->load = trig_hook_load;

	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->modelName = NULL;
	temp->owner = NULL;
	temp->s.modelindex = 0;
	temp->spawnflags = wall->spawnflags;

	temp->delay = (float)play_sound;

	//	just set origin, but don't link this entity in for clipping
	temp->s.origin = org;
	
	//	save mins and maxs for debris origin calculation. Because the 
	//	func_wall_explode is a bmodel, these are absolute world coords
	temp->s.mins = wall->s.mins;
	temp->s.maxs = wall->s.maxs;

	time = rnd () * 0.25;

	temp->nextthink = gstate->time + time;
	temp->think = blow_up;

	return	time;
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_explode_remove2
///////////////////////////////////////////////////////////////////////////////

void	func_wall_explode_remove2( userEntity_t * self)
{
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_explode_remove2
///////////////////////////////////////////////////////////////////////////////

void	func_wall_explode_remove( userEntity_t * self)
{
	self->solid = SOLID_NOT;
	self->modelName = NULL;

	//	self->nextthink = self->ltime + self->frags;
	self->nextthink = gstate->time + self->hacks;
	self->think = func_wall_explode_remove2;
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_explode_die
///////////////////////////////////////////////////////////////////////////////

const int MAX_DUPLICATES = 16;

void	func_wall_explode_die(userEntity_t * self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	// get the wall hook
	trigHook_t* hook = (trigHook_t*)self->userHook;

	//&&& AMW 6.12.98 - if this entity is part of a group.. make sure we shouldn't be triggering something
	// else first
	if (self->groupname)
	{
		userEntity_t	*cur, *target = self, *dupeList[MAX_DUPLICATES];
		int duplicates = 0;

		// find the first member in the group of the same class
		cur = com->FindFirstGroupMember (self->groupname, self->className);

		while (cur)
		{
			// look for duplicate triggerIndex levels first
			if ((cur != self) && (cur->triggerIndex == target->triggerIndex) && (duplicates < (MAX_DUPLICATES-1)))
			{
				dupeList[duplicates] = cur;	// this is a list of duplicates to 'target'
				duplicates++;
			}

			// find the lowest triggerIndex we can
			else if (cur->triggerIndex < target->triggerIndex)
			{
				duplicates = 0;	// reset this.. we found one lower
				target = cur;
			}
			// find the next member in the group of the same class
			cur = com->FindNextGroupMember(self->groupname, cur, self->className);
		}

		// if we found one with a lower triggerIndex
		if (target && (target != self))
		{
			// see if we have duplicates
			if (duplicates > 0)
			{
				// add in the original target we found into the list
				dupeList[duplicates++] = target;
				// get a random entry
				int randIndex = rand() % duplicates;	
				// this is the "new" target
				target = dupeList[randIndex];
			}
			
			// restore helath for self and set health for 'target' according to gstate->damage_inflicted
			self->health += gstate->damage_inflicted;
			target->health -= gstate->damage_inflicted;
			
			// trigger 'target' instead if it's dead
			if (target->health <= 0.0)
			{
				if (target->die)
				{
					if (target->flags & FL_CLIENT && target != attacker)
						//	make sure we'll follow killer
						target->enemy = attacker;
					else
						target->enemy = NULL;

					//make sure we don't do another recursion level.. this guy is really dead
					target->groupname = NULL;

					target->die (target, inflictor, attacker, damage, target->s.origin);
				}
			}
			return;
		}
	}

	CVector	temp, center;
	float	width, x_width, y_width, z_width, explo_count, new_time, max_time;
	int		i;

	// calc the number of explosions this wall will need based on it's size

	temp = self->size;
	width = temp.Length();

	// set the activator to whoever shot the wall
	gstate->activator = gstate->attacker;

	center = self->absmin + temp * 0.5;

	x_width = self->absmax[0] - self->absmin[0];
	y_width = self->absmax[1] - self->absmin[1];	
	z_width = self->absmax[2] - self->absmin[2];

	// figure number of explosions based on size of wall
	explo_count = ((x_width / 60) + 1) * ((y_width / 60) + 1) * ((z_width / 60) + 1) - 2;
	if (explo_count < 1) explo_count = 1;

	if (self->spawnflags & NO_CHUNKS)
		explo_count = -1;

	// amw: let's cap this at 10 in case the func_wall is huge
	if (explo_count > 10)
	{
		explo_count = 10;
	}


	if (!(self->spawnflags & NO_SOUND) && hook->triggerSound)
	{
		gstate->StartEntitySound(self, CHAN_AUTO, hook->triggerSound, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	gstate->StartEntitySound( attacker, CHAN_AUTO, gstate->SoundIndex("global/e_explodec.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	max_time = 0;
	for (i = 0; i <= explo_count; i++)
	{
		temp = self->absmax;
		
		temp.x = temp.x - rnd() * x_width;
		temp.y = temp.y - rnd() * y_width;
		temp.z = temp.z - rnd() * z_width;
		
		new_time = spawn_explosion (self, temp, (self->spawnflags & NO_SOUND));
		if (new_time > max_time)
			max_time = new_time;
	}

	//	don't remove immediately, because blow_up still refers back to this entity 
	//	and may do so for another couple of thinks
//	self->nextthink = gstate->time + 0.1;
//	self->think = func_wall_explode_remove;

	//	Q2FIXME:	other == attacker??? 
	com->UseTargets (self, attacker, attacker);

	self->takedamage = DAMAGE_NO;

	self->hacks = max_time;

	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_explode_use
///////////////////////////////////////////////////////////////////////////////

void	func_wall_explode_use( userEntity_t * self, userEntity_t *other, userEntity_t *activator)
{
	func_wall_explode_die (self, activator, activator, 32768, self->s.origin);
}

///////////////////////////////////////////////////////////////////////////////
//	func_wall_explode
///////////////////////////////////////////////////////////////////////////////

void	func_wall_explode( userEntity_t *self )
{
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	func_wall_explode_count++;

	// parse epairs
	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	hook = (trigHook_t *) self->userHook;
	hook->base_health = 0;

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;

	self->targetname = self->target = self->killtarget = NULL;

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "sound"))
			soundStr = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "health"))
			self->health = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "model_1"))
			hook->model_1 = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "model_2"))
			hook->model_2 = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "model_3"))
			hook->model_3 = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "message"))
			self->message = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

	}

	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

	hook->base_health = self->health;

	if (!hook->model_1)
	{
		if (self->spawnflags & WOOD_CHUNKS)
			hook->model_1 = gib_models [GIB_WOOD1];
		else if (self->spawnflags & ROCK_CHUNKS)
			hook->model_1 = gib_models [GIB_ROCK1];
		else
			hook->model_1 = gib_models [GIB_ROCK1];
	}

	if (!hook->model_2)
	{
		if (self->spawnflags & WOOD_CHUNKS)
			hook->model_2 = gib_models [GIB_WOOD2];
		else if (self->spawnflags & ROCK_CHUNKS)
			hook->model_2 = gib_models [GIB_ROCK2];
		else
			hook->model_2 = gib_models [GIB_ROCK2];
	}

	if (!hook->model_3)
	{
		if (self->spawnflags & WOOD_CHUNKS)
			hook->model_3 = gib_models [GIB_WOOD2];
		else if (self->spawnflags & ROCK_CHUNKS)
			hook->model_3 = gib_models [GIB_ROCK3];
		else
			hook->model_3 = gib_models [GIB_ROCK3];
	}

	gstate->SetOrigin2 (self, self->s.origin);

	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	self->takedamage = DAMAGE_YES;
	gstate->SetModel (self, self->modelName);

	if (self->health)
		self->die = func_wall_explode_die;
	else
		self->use = func_wall_explode_use;
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_fog_value_touch
///////////////////////////////////////////////////////////////////////////////

/*
#define ONEDIV255 0.003921
void	trigger_fog_value_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
//void	trigger_fog_value_touch( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	int	i;
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

//	if (activator->flags & FL_CLIENT)
	if (other->flags & FL_CLIENT)
	{
		for (i = 0; self->epair[i].key != NULL; i++)
		{
			if (!stricmp (self->epair[i].key, "on"))
				gstate->fogactive = atoi (self->epair[i].value);
			else if (!stricmp (self->epair[i].key, "dec"))
				gstate->foginc = atoi (self->epair[i].value);
			else if (!stricmp (self->epair[i].key, "inc"))
				gstate->foginc = -atoi (self->epair[i].value);
			else if (!stricmp (self->epair[i].key, "_color"))
				sscanf (self->epair[i].value, "%f %f %f", &gstate->fogcolor[0], &gstate->fogcolor[1], &gstate->fogcolor[2]);
			else if (!stricmp (self->epair[i].key, "r"))
				gstate->fogcolor[0] = atof (self->epair[i].value) * ONEDIV255;
			else if (!stricmp (self->epair[i].key, "g"))
				gstate->fogcolor[1] = atof (self->epair[i].value) * ONEDIV255;
			else if (!stricmp (self->epair[i].key, "b"))
				gstate->fogcolor[2] = atof (self->epair[i].value) * ONEDIV255;
			else if (!stricmp (self->epair[i].key, "start"))
				gstate->fogstart = atof (self->epair[i].value);
			else if (!stricmp (self->epair[i].key, "end"))
				gstate->fogend = atof (self->epair[i].value);
			else if (!stricmp (self->epair[i].key, "skyend"))
				gstate->fogskyend = atof (self->epair[i].value);
		}

//		if( gstate->fogend > 4096 )
//			gstate->fogend = 4096;

		gstate->WriteByte (SVC_EFFECT_NOTIFY);
		gstate->WriteLong (ET_FOG);
		gstate->WriteByte (gstate->fogactive);
		gstate->WriteFloat (gstate->fogcolor[0]);
		gstate->WriteFloat (gstate->fogcolor[1]);
		gstate->WriteFloat (gstate->fogcolor[2]);
		gstate->WriteFloat (gstate->fogstart);
		gstate->WriteFloat (gstate->fogend);
		gstate->WriteFloat (gstate->fogskyend);
		gstate->WriteByte (gstate->foginc);
		gstate->UniCast (other, true);
	}
}
void	trigger_fog_value_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	trigger_fog_value_touch( self, other, NULL, NULL );
}

#define	TRIGGER_FOG_TOUCHABLE	0x0001

///////////////////////////////////////////////////////////////////////////////
//	trigger_fog_value
///////////////////////////////////////////////////////////////////////////////

void	trigger_fog_value( userEntity_t *self )
{
	if (self->spawnflags & TRIGGER_FOG_TOUCHABLE)
	{
		self->solid = SOLID_TRIGGER;
		self->movetype = MOVETYPE_NONE;
		gstate->SetModel (self, self->modelName);
		self->modelName = NULL;

		self->touch = trigger_fog_value_touch;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_NONE;
		gstate->SetModel (self, self->modelName);
		self->modelName = NULL;

		self->use = trigger_fog_value_use;
	}
}
*/
///////////////////////////////////////////////////////////////////////////////
//	warp_continue
//
//	think function for an object other than a monster or client
//	that has gone through a warp
///////////////////////////////////////////////////////////////////////////////

void	warp_continue( userEntity_t *self )
{
	warpHook_t		*whook = (warpHook_t *) self->ptr1;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	self->think = whook->think;
	self->nextthink = gstate->time + whook->nextthink;

	self->solid = whook->solid;
	self->movetype = whook->movetype;

	self->s.effects = whook->effects;

	gstate->SetOrigin2 (self, self->s.origin);
	self->velocity = whook->velocity;
	self->s.render_scale = whook->render_scale;

//	delete whook;
	gstate->X_Free( whook );

//	self->flags -= FL_INWARP;
	self->flags &= ~FL_INWARP;

	if (self->flags & (FL_MONSTER + FL_BOT))
	{
		//	so exit node from teleporter/warp gets placed at actual exit!
		ai_teleported (self);

		//	set render scale of all parts
/*
		if (hook->dflags & DFL_HIERARCHICAL)
		{
			if (hook->legs)
			{
				hook->legs->s.render_scale = self->s.render_scale;

				if (hook->torso)
				{
					hook->torso->s.render_scale = self->s.render_scale;

					if (hook->head)
						hook->head->s.render_scale = self->s.render_scale;
					if (hook->weapon)
						hook->weapon->s.render_scale = self->s.render_scale;
				}
			}
		}
*/
	}
}

///////////////////////////////////////////////////////////////////////////////
//	warp_appear_think
//
//	Makes object stretch out of nothing when appearing from	a teleporter
//
//	client is immobile until reaching full size
///////////////////////////////////////////////////////////////////////////////

#define	SCALE_UP	1.2
#define	SCALE_DOWN	0.8
/*
void	warp_appear_think( userEntity_t *self )
{
//	playerHook_t	*hook;
	warpHook_t		*whook = (warpHook_t *) self->ptr1;

	self->s.render_scale.x *= SCALE_UP;
	self->s.render_scale.y *= SCALE_UP;
	self->s.render_scale.z *= SCALE_DOWN;

	if ( self->s.render_scale.x > whook->render_scale.x )
	{
		warp_continue (self);

		return;
	}

	self->think = warp_appear_think;
	self->nextthink = gstate->time + THINK_TIME;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	warp_appear
//
//	Makes teleporting entity stretch out of nothing when appearing from
//	a teleporter
//
//	entity is immobile until reaching full size
///////////////////////////////////////////////////////////////////////////////
/*
void	warp_appear( userEntity_t *self )
{
//	playerHook_t	*hook;

	self->s.render_scale.Set( 0.1, 0.1, 7.4);
	self->s.renderfx -= RF_NODRAW;

	///////////////////////////////////////////////////////////////////////////
	//	splat
	///////////////////////////////////////////////////////////////////////////

	com->TeleFrag (self, 65535);

	warp_appear_think (self);
}
*/
///////////////////////////////////////////////////////////////////////////////
//	warp_next_dest
//
//	returns the next info_warp_destination from the passed target
//	Does not adjust the origin of a warping entity
///////////////////////////////////////////////////////////////////////////////
/*
userEntity_t	*warp_next_dest (CVector &origin, userEntity_t *target_dest)
{
	userEntity_t	*next_target;
	CVector			org;

	org = origin;

	///////////////////////////////////////////////////////////////////////////
	//	determine if the target path corner can be seen
	//	if it cannot, then jump to the next path corner
	///////////////////////////////////////////////////////////////////////////

	while (1)
	{
		tr = gstate->TraceLine_q2 (org, target_dest->s.origin, NULL, MASK_OPAQUE);
		if (tr.fraction != 1.0 || tr.startsolid || tr.allsolid)
		{
			if (!target_dest->target)
			{
				//	this must be the last info_teleport_destination in the chain
				break;
			}
			else
			{
				next_target = com->FindTarget (target_dest->target);
				if (next_target)
				{
					//	try again from next info_teleport_dest
					org = target_dest->s.origin;
					target_dest = next_target;
				}
				else
					gstate->Error ("warp_next_dest:  bad target.\n");
			}
		}
		else
			break;
	}

	///////////////////////////////////////////////////////////////////////////
	//	handle any SKIPTO flags
	///////////////////////////////////////////////////////////////////////////

	while (target_dest->spawnflags & INFOWARP_SKIPTO)
	{
		if (target_dest->target)
		{
			next_target = com->FindTarget (target_dest->target);

			if (next_target)
				target_dest = next_target;
			else
				gstate->Error ("warp_next_dest:  bad SKIPTO target.\n");
		}
		else
			break;
	}

	return	target_dest;
}
*/

///////////////////////////////////////////////////////////////////////////////
//	warp_path_length
//
//	pass a pointer to the trigger_warp and the length of the path
//	to the last info_warp_destination will be returned
///////////////////////////////////////////////////////////////////////////////
/*
float	warp_path_length( userEntity_t *warp)
{
	userEntity_t	*head, *target;
	CVector			temp;
	float			dist;

	head = warp;
	dist = 0.0;
	
	while (head)
	{
		if (!head->target) 
		{
			break;
		}
		target = com->FindTarget (head->target);
		if (!target) 
		{
			break;
		}

		temp = head->s.origin - target->s.origin;
		dist = dist + temp.Length();

		head = target;
	}

	return	dist;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	warp_final_dest
//
//	returns the final destination of the warp
///////////////////////////////////////////////////////////////////////////////
/*
userEntity_t *warp_final_dest( userEntity_t *warp)
{
	userEntity_t	*head;

	head = warp;
	
	while (head && head->target)
	{
		head = com->FindTarget (head->target);
	}

	if (!head)
		gstate->Error ("warp_final_dest:  no final destination!\n");

	return	head;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	warp_setvalues
//
///////////////////////////////////////////////////////////////////////////////
/*

userEntity_t	*warp_setvalues( userEntity_t *self, userEntity_t *target)
{
	float			dist;
	float			looking_yaw;
	float			looking_pitch;
	float			yaw_diff, pitch_diff;
	float			time_to_target;
	float			pitch_dir, yaw_dir;
	CVector			temp, ang;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	trigHook_t		*thook;
	trace_t			tr;
	userEntity_t	*next_target;

	///////////////////////////////////////////////////////////////////////////
	//	determine if the target path corner can be seen
	//	if it cannot, then jump to the next path corner
	///////////////////////////////////////////////////////////////////////////

	while (1)
	{
		tr = gstate->TraceLine_q2 (self->s.origin, target->s.origin, self, MASK_OPAQUE);
		if (tr.fraction != 1.0 || tr.startsolid || tr.allsolid)
		{
			if (!target->target)
			{
				//	this must be the last info_teleport_destination in the chain
				self->s.origin = target->s.origin;
				gstate->LinkEntity (self);
				break;
			}
			else
			{
				next_target = com->FindTarget (target->target);
				if (next_target)
				{
					self->s.origin = target->s.origin;
					gstate->LinkEntity (self);
					target = next_target;
				}
				else
				{
					gstate->Error ("warp_setvalues: could not find info_warp_destination target.\n");
				}
			}
		}
		else
			break;
	}

	///////////////////////////////////////////////////////////////////////////
	//	handle any SKIPTO flags
	///////////////////////////////////////////////////////////////////////////

	while (target->spawnflags & INFOWARP_SKIPTO)
	{
		if (target->target)
		{
			next_target = com->FindTarget (target->target);

			if (next_target)
			{
				self->s.origin = target->s.origin;
				gstate->LinkEntity (self);
				target = next_target;
			}
			else
				gstate->Error ("warp_setvalues: could not find info_warp_destination SKIPTO target.\n");
		}
		else
			break;
	}

//	target = warp_next_dest (self->s.origin, target);
	thook = (trigHook_t *) target->userHook;

	//	ensure we're facing the next target
	temp = target->s.origin - self->s.origin;
	temp.Length();
	VectorToAngles( temp, ang );

	gstate->SetClientAngles (self, ang);
	self->flags |= FL_FIXANGLES;

	///////////////////////////////////////////////////////////////////////////
	//	setup speed to next path
	///////////////////////////////////////////////////////////////////////////

	if (thook->speed)
		hook->run_speed = thook->speed;

//	hook->run_speed = 50.0;

	// get vector from client to target
	temp = target->s.origin - self->s.origin;
	dist = temp.Length();
	temp.Normalize();

	//	get the angles in direction of target
	VectorToAngles( temp, ang );

	//	set the client's velocity
	temp = temp * hook->run_speed;
	self->velocity = temp;

	looking_yaw = AngleMod (self->client->ps.viewangles [1]);
	hook->force_angles.y = AngleMod (ang.y);
	self->ideal_ang.y = hook->force_angles.y;

	looking_pitch = self->client->ps.viewangles.x;
	
	//	VecToAngles returns a pitch of 0 - 360 so we have to
	//	adjust ideal_pitch to -80 - 70, like v_angle [0] normally is

	hook->ideal_pitch = -ang.x;
	if (hook->ideal_pitch >= 180.0) 
		hook->ideal_pitch -= 360.0;
	else 
	if (hook->ideal_pitch <= -180)
		hook->ideal_pitch += 360.0;
	hook->force_angles.x = hook->ideal_pitch;

	BestDelta (looking_yaw, hook->force_angles.y, &yaw_dir, &yaw_diff);
	BestDelta (looking_pitch, hook->force_angles.x, &pitch_dir, &pitch_diff);

	//	divide dist by speed to get time to target
	time_to_target = dist / hook->run_speed;
	self->teleport_time = gstate->time + time_to_target;

	//	find rate of pitch change in degrees per second 
	hook->force_rate.x = (pitch_diff / time_to_target) * pitch_dir * 4.0;
	//	find rate of yaw change in degrees per second 
	hook->force_rate.y = (yaw_diff / time_to_target) * yaw_dir * 4.0;

	//	do one half revolution while traveling from one dest to the next
	hook->force_rate.z = 90.0 / time_to_target;

	return	target;
}
*/
//////////////////////////
//	stretchy_think
//
//////////////////////////
/*
void	stretchy_think( userEntity_t *self )
{
	////////////////////////////////////////////////
	//	side to side stretch
	////////////////////////////////////////////////
	////////////////////////////////////////////////
	//	upward stretch
	////////////////////////////////////////////////
	
	self->s.render_scale.x *= self->hacks;
	self->s.render_scale.y *= self->hacks;
	self->s.render_scale.z *= self->delay;

	if (self->s.render_scale.x < 0.1 && self->delay > 1.0)
	{
		self->delay = 0.5;
	}
	if (self->s.render_scale.z < 0.1)
	{
//		if (self->flags & FL_HCHILD)
//			gstate->hr_RemoveChild (self, self->owner, false);
		self->remove (self);
		return;
	}

	////////////////////////////////////////////////
	//	forward stretch -- bleh
	////////////////////////////////////////////////
	self->nextthink = gstate->time + THINK_TIME;
	self->think = stretchy_think;
}
*/
//////////////////////////
//	warp_spawn_stretchy
//
//////////////////////////
/*
void	warp_spawn_stretchy( userEntity_t *self )
{
	userEntity_t	*temp, *rubber;
	playerHook_t	*hook;
	int				i, imax;

// SCG[1/23/00]: moved here to make sure it's initialized properly
		hook = AI_GetPlayerHook( self );

	if (self->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
//		hook = AI_GetPlayerHook( self );

		imax = 1;
	}
	else
		imax = 1;
	
	//	if hierarchical model... spawn copies of head, torso, legs and weapon
	for (i = 0; i < imax; i++)
	{
		temp = self;

		rubber = gstate->SpawnEntity ();
		rubber->movetype = MOVETYPE_NONE;
		rubber->solid = SOLID_NOT;

		rubber->s.modelindex = temp->s.modelindex;

		rubber->s.mins.Zero();
		rubber->s.maxs.Zero();

		rubber->s.origin = temp->s.origin;
		gstate->LinkEntity (rubber);

		rubber->s.render_scale = temp->s.render_scale;
		rubber->s.angles = temp->s.angles;

		rubber->delay = 1.2;
		rubber->hacks = 0.8;

		rubber->think = stretchy_think;
		rubber->nextthink = gstate->time + THINK_TIME;
	}
}
*/
//////////////////////////
//	warp_touch
//
//////////////////////////
/*
void	warp_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userEntity_t	*target, *dest;
	CVector			temp, ang;
	playerHook_t	*hook;
	trigHook_t		*thook = (trigHook_t *) self->userHook;
	warpHook_t		*whook;
	float			dist, speed, time_to_target;
	teleportNode_t	*node_data;
	userEntity_t	*went;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);
	
	if (!com->ValidTouch (self, other))
		return;

	if (other->flags & FL_INWARP)
		return;

	if (other->deadflag != DEAD_NO)
		gstate->Error ("Dead thing tried to teleport!!\n");

	dest = warp_final_dest (self);

	if (other->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		//	force a teleport node to be placed at the location of this teleport
		hook = AI_GetPlayerHook( other );
		temp = (self->absmin + self->absmax) * 0.5;

		//	create the necessary node data for this teleport
		node_data = (teleportNode_t*)gstate->X_Malloc(sizeof(teleportNode_t), MEM_TAG_HOOK);
		node_data->start_origin = temp;
		node_data->dest_origin = dest->s.origin;

		//node_force_node (other, hook->pNodeList, node_data->start_origin, NODE_TELEPORTER, node_data);
	}

	if (other->flags & FL_CLIENT)
	{
		if (!self->target)
		{
			gstate->Con_Dprintf ("trigger_warp without target!");
			return;
		}

		target = com->FindTarget (self->target);
		if (!target) 
		{
			gstate->Con_Dprintf ("trigger_warp could not find target!");
			return;
		}

		//	make a duplicate of this model for warp effect
		warp_spawn_stretchy (other);

		other->solid = SOLID_NOT;
		other->movetype = MOVETYPE_NOCLIP;

		gstate->LinkEntity (other);

		// move client towards target ent
		hook = AI_GetPlayerHook( other );

//		Client_SetFOVRamp (other, 120, 170);

		//	don't draw any of the hierarchical models
		other->s.renderfx |= RF_NODRAW;
		other->viewheight = 0;

		if (!thook->speed)
		{
			// preferred 2 second travel time
			hook->run_speed = warp_path_length (self) / 2.0;
			if (hook->run_speed > 1500.0) hook->run_speed = 1500.0;
		}
		else
			hook->run_speed = thook->speed;

		// save the target for checking in the client's prethink
		other->enemy = warp_setvalues (other, target);

		//	make client glow if spawnflag is set on target
		if (target->spawnflags & INFOWARP_GLOW)
			other->s.effects |= EF_DIMLIGHT;

		//	if destination is not visible, then go straight to next path
		if (target->spawnflags & INFOWARP_SKIPTO)
		{
			//	put player right on path corner
			other->s.origin = target->s.origin;
			gstate->LinkEntity (other);

			//	make player face next target
			if (other->enemy->target)
			{
				target = com->FindTarget (other->enemy->target);
			
				if (target)
				{
					temp = target->s.origin - other->enemy->s.origin;
					temp.Length();
					VectorToAngles( temp, ang );

					gstate->SetClientAngles (other, ang);
					other->flags |= FL_FIXANGLES;
				}
			}

			self->teleport_time = gstate->time;
		}

		other->flags |= FL_INWARP;

		//	turn off chasecam if it is on
		camera_stop (other, false);

		//	turn on camera
		gstate->SetCameraState (other, true, CAMFL_LOCK_X + CAMFL_LOCK_Y + CAMFL_LOCK_Z);
		Client_SetForceAngles (other);

		//	turn off weapon model
		went =( userEntity_t *) other->client->ps.weapon;
		if (went)
			went->s.renderfx |= RF_NODRAW;

		// missing sound
		//gstate->sound (other, CHAN_BODY, "misc/warpstart.wav", 255, ATTN_NORM);
	}
	///////////////////////////////////////////////////////////////////
	//	other object warping
	//
	//	save some important data and then set objects think to
	//	warp_appear at time it takes to go through warp
	//
	//	objects with self->ptr1 != NULL won't teleport (floaters, etc.)
	///////////////////////////////////////////////////////////////////
	else if (!other->ptr1)
	{
		dist = warp_path_length (self);
		speed = dist / 2.0;
		if (speed > 1500.0) speed = 1500.0;

		time_to_target = dist / speed;

		//	make a duplicate of this model for warp effect
		warp_spawn_stretchy (other);
		
		whook = (warpHook_t*)gstate->X_Malloc(sizeof(warpHook_t), MEM_TAG_HOOK);
		other->ptr1 = whook;

		whook->think = other->think;
		whook->nextthink = other->nextthink - gstate->time;;
		whook->movetype = other->movetype;
		whook->solid = other->solid;
		whook->effects = other->s.effects;
		whook->velocity = other->velocity;
		whook->render_scale = other->s.render_scale;

		other->solid = SOLID_NOT;
		other->movetype = MOVETYPE_NONE;
		other->s.renderfx = RF_NODRAW;
		other->flags |= FL_INWARP;
		other->s.angles = dest->s.angles;
		other->velocity.Zero();

		gstate->SetOrigin2 (other, dest->s.origin);

		other->think = warp_appear;
		other->nextthink = gstate->time + time_to_target;
	}
}
*/
///////////////////////////////////////////////////////////////////////////////
//  trigger_warp
//
///////////////////////////////////////////////////////////////////////////////
/*
void	trigger_warp( userEntity_t *self )
{
	trigHook_t	*hook;
	int			i;
	char		*soundStr = NULL;

	trigger_init (self);

	// parse epairs
	hook = (trigHook_t *) self->userHook;
	hook->count = 0;
	hook->speed = 0.0;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "fog_value"))
			hook->model_1 = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;

		i++;
	}

	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

//	trigger_setup (self);

	self->touch = warp_touch;
	self->className = "trigger_warp";

	if (!self->target)
		com->Error ("warp has no target!\n");

	if (!hook->triggerSound)
	{
		hook->triggerSound = gstate->SoundIndex("misc/teleportstart.wav");
		//gstate->SoundIndex ("misc/teleportend.wav");
	}
}
*/
///////////////////////////////////////////////////////////////////////////////
//	info_warp_destination
///////////////////////////////////////////////////////////////////////////////
/*
void	info_warp_destination( userEntity_t *self )
{
	CVector		temp;
	int			i;
	trigHook_t	*hook;
	char		*soundStr = NULL;

	self->userHook = gstate->X_Malloc(sizeof(trigHook_t), MEM_TAG_HOOK);
	hook = (trigHook_t *) self->userHook;
	hook->speed = 0.0;

	// SCG[11/24/99]: Save game stuff
	self->save = trig_hook_save;
	self->load = trig_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sound") == 0)
			soundStr = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "target") == 0)
			self->target = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "speed") == 0)
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "mp3"))
			hook->mp3file = self->epair[i].value;
		i++;
	}
	
	if (soundStr)			
		hook->triggerSound = gstate->SoundIndex (soundStr);

	self->className = "info_warp_destination";

	hook->mangle = self->s.angles;
	hook->v_angle = CVector(0, 0, 0);
	self->modelName = "";
	
	temp = CVector(0.0, 0.0, 27.0) + self->s.origin;
	self->s.origin = temp;

	if (!self->targetname)
		com->Error ("No targetname for info_warp_dest\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//	trigger_monsterjump_touch
//
///////////////////////////////////////////////////////////////////////////////
/*

#define	MONSTJUMP_ANYDIR	0x01
#define	MONSTJUMP_ONCE		0x02

void	trigger_monsterjump_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userEntity_t	*target;
	CVector			angles, org1, org2;
	playerHook_t	*hook;
	CVector			vel;
	float			dp;

	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	hook = AI_GetPlayerHook( other );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack (hook);
	if (GOALSTACK_GetCurrentTaskType (pGoalStack) == TASKTYPE_JUMPFORWARD)
		//	monstser is already jumping forward
		return;

	target = com->FindTarget (self->target);
	if (!target)
	{
		//	no target, so just jump in direction of self->s.angles
		AI_AddNewTaskAtFront (other, TASKTYPE_JUMPFORWARD);

		//	face the direction we're jumping
		other->s.angles = self->s.angles;
		other->ideal_ang = self->s.angles;

		angles = self->s.angles;
		angles.AngleToVectors(forward, up, right);

		hook->forward_vel = hook->run_speed;
		hook->fnStartJumpForward (other);
		
		return;
	}

	//	make sure monster is travelling in the right direction
	if (!(self->spawnflags & MONSTJUMP_ANYDIR))
	{
		//	make sure that touch happened on the correct side of the brush
		vel = other->velocity;
		vel.z = 0;
		vel.Normalize();

		dp = DotProduct (vel, self->movedir);
		if (dp < 0.7071)
		{
			//	> 45 degrees from valid touch direction
			return;
		}
	}


	org1 = target->s.origin;
	org1.z = 0;
	org2 = other->s.origin;
	org2.z = 0;

	forward = org1 - org2;
	float jumpDist = forward.Length();
	forward.Normalize ();
	vel = forward;

	//	face the direction we're jumping
	VectorToAngles (vel, other->s.angles);
	other->ideal_ang = other->s.angles;

	//	re-adjust the velocities to hit the target
	if ( target->s.origin[2] > other->s.origin[2] )
	{
		//	target is above monster's current position
	}
	else
	{
		//	target is same height as or below monster's position
		float	monsterGravity = p_gravity->value * other->gravity;
		float	timeToMaxHeight = hook->upward_vel / monsterGravity;
		float	maxHeight = ((hook->upward_vel * hook->upward_vel) / (2.0f * monsterGravity)) + other->s.origin[2];

		float	fallDist = maxHeight - target->s.origin[2];
		float	timeToFall = sqrt (fallDist / (0.5 * monsterGravity));

		float	totalTime = timeToFall + timeToMaxHeight;
		float	forwardVel = jumpDist / totalTime;

		hook->forward_vel = forwardVel;
	}

	AI_AddNewTaskAtFront (other, TASKTYPE_JUMPFORWARD);

	//	let the AI routines know we're jumping
	hook->fnStartJumpForward (other);
}
*/
///////////////////////////////////////////////////////////////////////////////
//	trigger_monsterjump
//
//	When any monster going in the same direction as the trigger's angles
//	hits this trigger, it will jump towards the trigger's target
///////////////////////////////////////////////////////////////////////////////
/*
void	trigger_monsterjump( userEntity_t *self )
{
	CVector	angles;

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	//	setmodel is REQUIRED for BSP entities!  Just assigning the s.modelindex
	//	does not work
	gstate->SetModel (self, self->modelName);

	gstate->LinkEntity (self);

	//	set the angles for this trigger
	if (!(self->spawnflags & MONSTJUMP_ANYDIR))
	{
		//	save angles
		angles = self->s.angles;

		com->SetMovedir (self);

		//	restore angles
		self->s.angles = angles;
	}
	else
	{
		self->movedir = vec3_origin;
	}

	self->touch = trigger_monsterjump_touch;
}
*/

///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_use
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
  trigHook_t	*hook;

	gstate->Con_Dprintf("trigger_sidekick_use()\n");

	hook = (trigHook_t *) self->userHook;

	if (strcmpi(self->targetname,"mikiko"))
  {
		// inform server that Mikiko is now activated
		if (hook->bEnabled)
      com->Sidekick_Update(self,SIDEKICK_MIKIKO,SIDEKICK_STATUS_ACTIVATE,0);
		else
			com->Sidekick_Update(self,SIDEKICK_MIKIKO,SIDEKICK_STATUS_DEACTIVATE,0);
	}
	else
    if (strcmpi(self->targetname,"superfly"))
		{
			// inform server that Superfly is now activated
			if (hook->bEnabled)
			  com->Sidekick_Update(self,SIDEKICK_SUPERFLY,SIDEKICK_STATUS_ACTIVATE,0);
			else
				com->Sidekick_Update(self,SIDEKICK_SUPERFLY,SIDEKICK_STATUS_DEACTIVATE,0);
		}

	self->use   = NULL; // one-time deal
	self->touch = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_touch
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	gstate->Con_Dprintf("trigger_sidekick_touch()\n");

	trigger_sidekick_use(self, other, other); 
}


///////////////////////////////////////////////////////////////////////////////
//	trigger_sidekick
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick(userEntity_t *self )
// targetname holds 'mikiko' or 'superfly'
{
	trigHook_t	*hook;
	int i;

	trigger_init (self);
	hook = (trigHook_t *) self->userHook;

	// set defaults from keys
	trigger_setup (self);

	// override defaults
	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "sidekick") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "toggle") == 0)
			hook->bEnabled = atoi (self->epair [i].value);

		i++;
	}

	self->className = "trigger_sidekick";

	self->touch = trigger_sidekick_touch;
	self->use   = trigger_sidekick_use;

	if (!self->targetname)
	  gstate->Con_Dprintf("trigger_sidekick has no targetname ('mikiko' or 'superfly')\n");

	// never retriggers 
	hook->wait = -1.0;
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_stop_use
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_stop_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
    gstate->Con_Dprintf( "trigger_sidekick_stop_use()\n" );

    trigHook_t *hook = (trigHook_t *) self->userHook;

    CVector position = hook->mangle;		// SCG[1/25/00]: HACK - the position is stored in m_angle for this entity
    char *szSound = hook->mp3file;			// SCG[1/25/00]: HACK - sound string is stored in mp3 filename for this entity
    char *szAnimation = hook->console_msg;	// SCG[1/25/00]: HACK - the animation name is stored in the console_message for this entity
	char *pTarget = self->target;
/*
    CVector position;
    char *szSound = NULL;
    char *szAnimation = NULL;
	char *pTarget = NULL;

	int i = 0;
	while ( self->epair[i].key != NULL )
	{
		if ( _stricmp( self->epair[i].key, "sound" ) == 0 )
		{
        	szSound = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "animation" ) == 0 )
		{
        	szAnimation = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "x" ) == 0 )
		{
        	position.x = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "y" ) == 0 )
		{
        	position.y = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "z" ) == 0 )
		{
        	position.z = (float)atoi(self->epair[i].value);
		}
		else if( strcmp( self->epair[i].key, "target" ) == 0 )
		{
			pTarget = self->epair[i].value;
		}

		i++;
	}
*/

	// SCG[11/1/99]: position not specified, search targetname
	if( pTarget != NULL )
	{
		userEntity_t *pTargetEntity = com->FindTarget( pTarget );

		if( pTargetEntity != NULL )
		{
			position = pTargetEntity->s.origin;
		}
	}

    int nOrder = 1;
    userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
	if ( AI_IsAlive( pFirstSidekick ) )
	{
        if ( SIDEKICK_DoStopGoingFurther( pFirstSidekick, position, szSound, szAnimation, nOrder ) )
        {
            nOrder++;
        }
	}

    userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
	if ( AI_IsAlive( pSecondSidekick ) )
	{
        if ( SIDEKICK_DoStopGoingFurther( pSecondSidekick, position, szSound, szAnimation, nOrder ) )
        {
        }
    }
    

    self->use   = NULL; // one-time deal
    self->touch = NULL;

}


///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_stop_touch
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_stop_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	gstate->Con_Dprintf( "trigger_sidekick_stop_touch()\n" );

	trigger_sidekick_stop_use(self, other, other); 
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_sidekick_stop
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_stop( userEntity_t *self )
{
	trigger_init (self);
	trigHook_t *hook = (trigHook_t *) self->userHook;

	// set defaults from keys
	trigger_setup( self );

	self->className = "trigger_sidekick_stop";

	int i = 0;
	while ( self->epair[i].key != NULL )
	{
		if ( _stricmp( self->epair[i].key, "sound" ) == 0 )
		{
        	hook->mp3file = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "animation" ) == 0 )
		{
        	hook->console_msg = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "x" ) == 0 )
		{
        	hook->mangle.x = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "y" ) == 0 )
		{
        	hook->mangle.y = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "z" ) == 0 )
		{
        	hook->mangle.z = (float)atoi(self->epair[i].value);
		}
		else if( strcmp( self->epair[i].key, "target" ) == 0 )
		{
			self->target = self->epair[i].value;
		}
		i++;
	}

	self->touch = trigger_sidekick_stop_touch;
	self->use   = trigger_sidekick_stop_use;

	// never retriggers 
	hook->wait = -1.0;
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_teleport_use
///////////////////////////////////////////////////////////////////////////////
// SCG[2/19/00]: Thanks alot!
// SCG[2/19/00]: #define TRIGGER_SIDEKICK_TELEPORT_SUPERFLY	0x01
// SCG[2/19/00]: #define TRIGGER_SIDEKICK_TELEPORT_MIKIKO	0x02

#define TRIGGER_SIDEKICK_TELEPORT_STAY			0x01
#define TRIGGER_SIDEKICK_TELEPORT_COMEHERE		0x02
int SIDEKICK_FindClearSpotFrom( userEntity_t *self,CVector &fromPoint, CVector &clearPoint);
// NSS[2/23/00]:Oh boy... this should just about do the trick from all f*&king angles!
int Assign_Sidekick_Trigger_Teleport_Tasks(userEntity_t *self, userEntity_t *Sidekick,CVector &Position, char *szAnimation, char *szSound, int Animation_Completed)
{
	playerHook_t *phook;
	int bReturn = FALSE;
	if(Sidekick && AI_IsAlive( Sidekick ))
	{
		phook = AI_GetPlayerHook(Sidekick);
		if ( AI_IsSidekick(phook) )
		{
			
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(phook);
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			if(!pCurrentGoal)
			{
				pCurrentGoal = AI_AddNewGoal(Sidekick,GOALTYPE_SIDEKICK_FOLLOWOWNER);
			}
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			
			if(self->spawnflags & TRIGGER_SIDEKICK_TELEPORT_STAY)
			{

				phook->ai_flags &= ~AI_FOLLOW;
				phook->ai_flags |= AI_STAY;

				if ( nGoalType == GOALTYPE_STAY )
				{
				  AI_AddInitialTasksToGoal( Sidekick, pCurrentGoal, TRUE );
				}
				else
				{
				  //	get rid of all goals
				  GOALSTACK_ClearAllGoals( pGoalStack );
				  AI_AddNewGoal( Sidekick, GOALTYPE_STAY );
				}
		
			}		
			else if (self->spawnflags & TRIGGER_SIDEKICK_TELEPORT_COMEHERE)
			{
				phook->ai_flags &= ~AI_STAY;
				phook->ai_flags |= AI_FOLLOW;

				phook->ai_flags |= AI_COMEHERE;
        
				// find a point in directly front of the owner
				userEntity_t *pOwner = phook->owner;
				if(pOwner)
				{
					if(!AI_IsAlive(pOwner) || (pOwner->deadflag != DEAD_NO))
						return FALSE;
				  
				} 
				else// NSS[3/12/00]:No owner then get one or get out.
				{
					userEntity_t *head = alist_FirstEntity( client_list );
					// NSS[3/12/00]:Find a client to make our owner.
					while ( head != NULL )
					{
						if ( AI_IsAlive( head ) && (head->flags & FL_CLIENT) )
						{
							phook->owner = head;
							break;
						}	
						head = alist_NextEntity( client_list );
					}
					// NSS[3/12/00]:Try again... 
					pOwner = phook->owner;
					if(!pOwner)
					{
						// NSS[3/12/00]:At this point we have done everything in our God given power to find the client... if this happens then someone is being stupid
						// or lazy and has fucked something up big time.  This case should NEVER EVER EVER EVER happen.
						AI_Dprintf("Sidekick cannot Teleport and Come Here because Sidekick cannot find the owner!\n");
						return FALSE;
					}
				}
				CVector vector;
				pOwner->s.angles.AngleToForwardVector( vector );

				CVector position;
				VectorMA( pOwner->s.origin, vector, 64.0f, position );
        
				CVector groundPoint;
				// NSS[2/23/00]:Find a clear spot from the teleporter to the Owner
				if(SIDEKICK_FindClearSpotFrom( self, pOwner->s.origin , groundPoint ))
				{
					if(nGoalType == GOALTYPE_STAY)
					{
						if(phook->owner)
						{
							GOALSTACK_ClearAllGoals( pGoalStack );
							AI_AddNewGoal( Sidekick, GOALTYPE_FOLLOW, phook->owner );
							AI_AddNewTaskAtFront( Sidekick, TASKTYPE_SIDEKICK_COMEHERE, groundPoint );
						}
					}
					else
					{
						GOAL_ClearTasks( pCurrentGoal );
						AI_AddNewTaskAtFront( Sidekick, TASKTYPE_SIDEKICK_COMEHERE, groundPoint );
					}
				}			
			}
			AI_AddNewTaskAtFront(Sidekick,TASKTYPE_SIDEKICK_TELEPORT,Position);
			
			// NSS[2/23/00]:Sound and animation bullshit.
			// NSS[2/23/00]:This case deals with playing both sound and animation
			if ( szSound && strlen(szSound) > 0 && szAnimation && strlen(szAnimation) > 0 && !Animation_Completed)
			{
    			AIDATA aiData;
				frameData_t *pSequence = FRAMES_GetSequence( Sidekick, szAnimation );
				if ( pSequence )
				{
					aiData.pAnimSequence = pSequence;
				}
				aiData.pString = szSound;     
				AI_AddNewTaskAtFront( Sidekick, TASKTYPE_PLAYANIMATIONANDSOUND, &aiData );
				bReturn = TRUE;
			}
			else// NSS[2/23/00]:This case deals with playing either sound or animation  (hey I am just making it work... I didn't design the damned thing!)
			{
				if ( szAnimation && strlen(szAnimation) > 0 )
				{
    				frameData_t *pSequence = FRAMES_GetSequence( Sidekick, szAnimation );
					AI_AddNewTaskAtFront( Sidekick, TASKTYPE_PLAYANIMATION, pSequence );
				}
				if ( szSound && strlen(szSound) > 0 && !Animation_Completed)
				{
					AI_AddNewTaskAtFront( Sidekick, TASKTYPE_PLAYSOUND, szSound );
					bReturn = TRUE;
				}
			}
		}
	}
	return bReturn;
}


void trigger_sidekick_teleport_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
    gstate->Con_Dprintf( "trigger_sidekick_teleport_use()\n" );

    trigHook_t		*hook = (trigHook_t *) self->userHook;
	

    CVector position = hook->mangle;		// SCG[1/13/00]: HACK - the position is stored in m_angle for this entity
    char *szSound = hook->mp3file;			// SCG[1/13/00]: HACK - sound string is stored in mp3 filename for this entity
    char *szAnimation = hook->console_msg;	// SCG[1/13/00]: HACK - the animation name is stored in the console_message for this entity
	char *pTarget = self->target;
	
	// SCG[11/1/99]: position not specified, search targetname
	if( pTarget != NULL )
	{
		userEntity_t *pTargetEntity = com->FindTarget( pTarget );

		if( pTargetEntity != NULL )
		{
			position = pTargetEntity->s.origin;
		}
	}

	// NSS[2/23/00]:Condensed and modularized
	userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
	userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
	// NSS[2/23/00]:Ok...so sue me....this is hard to read... bleah.
	Assign_Sidekick_Trigger_Teleport_Tasks(self, pFirstSidekick,position,hook->console_msg,hook->mp3file,
		Assign_Sidekick_Trigger_Teleport_Tasks(self, pSecondSidekick,position,hook->console_msg,hook->mp3file,0));
}


///////////////////////////////////////////////////////////////////////////////
// trigger_sidekick_teleport_touch
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_teleport_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	//NSS[11/10/99]:
	Trigger_Reject( other, FL_MONSTER);

	if (!com->ValidTouch (self, other))
		return;

	gstate->Con_Dprintf( "trigger_sidekick_teleport_touch()\n" );

	self->touch = NULL;

	trigger_sidekick_teleport_use(self, other, other); 
}

///////////////////////////////////////////////////////////////////////////////
//	trigger_sidekick_teleport
///////////////////////////////////////////////////////////////////////////////
void trigger_sidekick_teleport( userEntity_t *self )
{

	trigger_init (self);
	trigger_setup( self );
	trigHook_t *hook = (trigHook_t *) self->userHook;
	int i = 0;
	while ( self->epair[i].key != NULL )
	{
		if ( _stricmp( self->epair[i].key, "sound" ) == 0 )
		{
        	hook->mp3file = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "animation" ) == 0 )
		{
        	hook->console_msg = self->epair[i].value;
		}
        else
		if ( _stricmp( self->epair[i].key, "x" ) == 0 )
		{
        	hook->mangle.x = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "y" ) == 0 )
		{
        	hook->mangle.y = (float)atoi(self->epair[i].value);
		}
        else
		if ( _stricmp( self->epair[i].key, "z" ) == 0 )
		{
        	hook->mangle.z = (float)atoi(self->epair[i].value);
		}
		else if( strcmp( self->epair[i].key, "target" ) == 0 )
		{
			self->target = self->epair[i].value;
		}
		i++;
	}

	self->className = "trigger_sidekick_teleport";

	self->touch = trigger_sidekick_teleport_touch;
	self->use   = trigger_sidekick_teleport_use;

	// never retriggers 
	hook->wait = -1.0;
}


//-----------------------------------------------------------------------------
typedef struct trigger_toggle_hook_s
{
	qboolean		bActivated;
	userEntity_t	*pActivator;
	float			fTriggerDist;
	CVector			vOrigin;
} trigger_toggle_hook_t;

#define	TRIGGER_TOGGLE_HOOK(x) (int)&(((trigger_toggle_hook_t *)0)->x)

field_t trigger_toggle_hook_fields[] = 
{
	{"bActivated",		TRIGGER_TOGGLE_HOOK(bActivated),	F_INT},
	{"pActivator",		TRIGGER_TOGGLE_HOOK(pActivator),	F_EDICT},
	{"fTriggerDist",	TRIGGER_TOGGLE_HOOK(fTriggerDist),	F_FLOAT},
	{"vOrigin",			TRIGGER_TOGGLE_HOOK(vOrigin),		F_VECTOR},
	{NULL, 0, F_INT}
};

void trigger_toggle_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, trigger_toggle_hook_fields, sizeof( trigger_toggle_hook_t ) );
}

void trigger_toggle_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, trigger_toggle_hook_fields, sizeof( trigger_toggle_hook_t ) );
}


void trigger_toggle_think( userEntity_t *self );
void trigger_toggle_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );

void trigger_toggle_think( userEntity_t *self )
{
	trigger_toggle_hook_t	*pHook = ( trigger_toggle_hook_t * ) self->userHook;

	// SCG[10/17/99]: make sure we've got valid data
	if( pHook->pActivator == NULL )
	{
		return;
	}

	// SCG[10/17/99]: we're thinking here :)
	// SCG[10/17/99]: if the distance of the player from the origin of the trigger
	// SCG[10/17/99]: is greater than the distance we set in the spawn,
	// SCG[10/17/99]: we need to reset everything...
	CVector vLength = pHook->vOrigin - pHook->pActivator->s.origin;
	float	fDist = fabs( vLength.Length() );

	// SCG[10/17/99]: if we are far enough away from the trigger, reset it.
	if( fDist > pHook->fTriggerDist )
	{
		if( self->spawnflags & TRIGGER_TOGGLE_USE_ON_EXIT )
		{
			com->UseTargets( self, pHook->pActivator, pHook->pActivator );
		}
		self->touch = trigger_toggle_touch;
		self->think = NULL;
		pHook->bActivated = 0;
		pHook->pActivator = NULL;
	}

	// SCG[10/17/99]: set the next think
	self->nextthink = gstate->time + 0.2;
}

void trigger_toggle_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	trigger_toggle_hook_t	*pHook = ( trigger_toggle_hook_t * ) self->userHook;

	// SCG[2/28/00]: Stevie wants sidekick only flag....
	if( self->spawnflags & TRIGGER_TOGGLE_SIDEKICK_ONLY )
	{
		if( !( other->flags & FL_BOT ) )
		{
			return;
		}
	}
	else
	{
		//NSS[11/10/99]:
		if( !( self->spawnflags & TRIGGER_INCLUDE_SIDEKICKS ) )
		{
			Trigger_Reject( other, ( FL_MONSTER|FL_BOT ) );
		}
		else
		{
			Trigger_Reject( other, FL_MONSTER);
		}

		if (!com->ValidTouch (self, other))
			return;
	}

	// SCG[10/17/99]: If the trigger has been touched and the current entity
	// SCG[10/17/99]: touching the trigger is not the same as the initial activator, we leave
	if( pHook->pActivator && pHook->pActivator != other )
	{
		return;
	}
	
	// SCG[10/17/99]: if we've already touch the trigger, we need to set a think and 
	// SCG[10/17/99]: set the touch to null
	if( pHook->bActivated )
	{
		self->touch = NULL;
		self->think = trigger_toggle_think;
		self->nextthink = gstate->time + 0.2;
	}
	else
	{
		// SCG[10/17/99]: set the trigger state
		pHook->bActivated = 1;
		// SCG[10/17/99]: set the triggers touch entity
		pHook->pActivator = other;

		// SCG[10/17/99]: use the target(s)
		com->UseTargets( self, other, other );
	}
}

void trigger_toggle( userEntity_t *self )
{
	trigger_toggle_hook_t	*pHook = ( trigger_toggle_hook_t * ) gstate->X_Malloc( sizeof( trigger_toggle_hook_t ), MEM_TAG_HOOK );

	trigger_init( self );

	// SCG[11/24/99]: Save game stuff
	self->save = trigger_toggle_hook_save;
	self->load = trigger_toggle_hook_load;

	for( int i = 0; self->epair[i].key != NULL; i++ )
	{
		if (!stricmp (self->epair[i].key, "health"))
			self->health = atof (self->epair[i].value);
		else if (!stricmp (self->epair[i].key, "delay"))
			self->delay = atof (self->epair[i].value);
		else if (!stricmp (self->epair[i].key, "targetname"))
			self->targetname = self->epair[i].value;
		else if (!stricmp (self->epair[i].key, "target"))
			self->target = self->epair[i].value;
		else if (!stricmp (self->epair[i].key, "killtarget"))
			self->killtarget = self->epair[i].value;
		else if (!stricmp (self->epair[i].key, "message"))
			self->message = self->epair[i].value;
	}

	self->userHook = pHook;

	pHook->bActivated = 0;
	pHook->pActivator = NULL;
	
	// SCG[10/17/99]: set the distance it will take to untoggle based on mins/maxs
	CVector vLength = self->s.maxs - self->s.mins;
	pHook->fTriggerDist = fabs( vLength.Length() / 2 );
	pHook->fTriggerDist *= 1.14;

	self->touch = trigger_toggle_touch;

	pHook->vOrigin.x = self->absmin.x;
	pHook->vOrigin.y = self->absmin.y;
	pHook->vOrigin.z = self->absmin.z;
	pHook->vOrigin.x += ( self->absmax.x - self->absmin.x ) / 2;
	pHook->vOrigin.y += ( self->absmax.y - self->absmin.y ) / 2;
	pHook->vOrigin.z += ( self->absmax.z - self->absmin.z ) / 2;

}

void trigger_changemusic_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);
void trigger_changemusic_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator);

#define MAX_MP3_PATH 256
#define TRIGGER_CHANGELEVEL_ALLOW_SIDEKICKS	0x01
typedef struct trigger_changemusic_hook_s
{
	char *name;
} trigger_changemusic_hook_t;

#define	TRIGGER_CHANGELEVEL_HOOK(x) (int)&(((trigger_changemusic_hook_t *)0)->x)
field_t trigger_changemusic_hook_fields[] = 
{
	{"name",		TRIGGER_CHANGELEVEL_HOOK(name),			F_LSTRING},
	{NULL, 0, F_INT}
};

void trigger_changemusic_pause_think( userEntity_t *self )
{
	self->use = trigger_changemusic_use;
	self->touch = trigger_changemusic_touch;
	self->think = NULL;
}

void trigger_changemusic_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	trigger_changemusic_hook_t *pHook = ( trigger_changemusic_hook_t * ) self->userHook;

	// SCG[3/17/00]: added so cineatic character can trigger this.
	if( !( other->flags & FL_CINEMATIC ) )
	{
		//NSS[11/10/99]:
		Trigger_Reject( other, ( FL_MONSTER | FL_BOT ));

		if( !com->ValidTouch( self, other) )
		{
			return;
		}
	}

	if( other->flags & FL_CINEMATIC )
	{
		gstate->StartMP3( pHook->name, CHAN_MUSIC_MAP, self->s.volume, 0, FALSE, NULL );
	}
	else
	{
		gstate->StartMP3( pHook->name, CHAN_MUSIC_MAP, self->s.volume, 0, FALSE, other );
	}

	self->think = trigger_changemusic_pause_think;
	self->use = NULL;
	self->touch = NULL;
	self->nextthink = gstate->time + 1.0;
}

void trigger_changemusic_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if( other->flags & ( FL_CLIENT | FL_BOT | FL_MONSTER ) )
	{
		trigger_changemusic_touch( self, other, NULL, NULL );
	}
	else
	{
		trigger_changemusic_touch( self, activator, NULL, NULL );
	}
}

void trigger_changemusic_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, trigger_changemusic_hook_fields, sizeof( trigger_changemusic_hook_t ) );
}

void trigger_changemusic_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, trigger_changemusic_hook_fields, sizeof( trigger_changemusic_hook_t ) );
}

void trigger_changemusic( userEntity_t *self )
{
	trigger_init( self );

	self->userHook = gstate->X_Malloc( sizeof( trigger_changemusic_hook_t ), MEM_TAG_HOOK );
	trigger_changemusic_hook_t	*pHook = ( trigger_changemusic_hook_t * ) self->userHook;

	for( int i = 0; self->epair[i].key != NULL; i++ )
	{
		if( !stricmp( self->epair[i].key, "path" ) )
		{
			pHook->name = self->epair[i].value;
		}
		else if( !stricmp( self->epair[i].key, "volume" ) )
		{
			self->s.volume = atoi( self->epair[i].value );
		}
	}

	if( self->s.volume == 0 )
	{
		self->s.volume = 1.0;
	}

	// SCG[11/24/99]: Save game stuff
	self->save = trigger_changemusic_save;
	self->load = trigger_changemusic_load;

	self->touch		= trigger_changemusic_touch;
	self->use		= trigger_changemusic_use;
}

void trigger_crosslevel_trigger_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	gstate->game->serverflags |= self->spawnflags;
	gstate->RemoveEntity( self );
}

void target_crosslevel_trigger( userEntity_t  *self )
{
	self->svflags = SVF_NOCLIENT;
	self->use = trigger_crosslevel_trigger_use;
}

void target_crosslevel_target_think( userEntity_t  *self )
{
	if( self->spawnflags == ( gstate->game->serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags ) )
	{
		com->UseTargets( self, self, self );
		gstate->RemoveEntity( self );
	}
}

void target_crosslevel_target( userEntity_t  *self )
{
	for( int i = 0; self->epair[i].key != NULL; i++ )
	{
		if( !stricmp( self->epair[i].key, "delay" ) )
		{
			self->delay = atoi( self->epair[i].value );
		}
	}

	if (!self->delay)
		self->delay = 1;
	self->svflags = SVF_NOCLIENT;

	self->think = target_crosslevel_target_think;
	self->nextthink = gstate->time + self->delay;
}

void trigger_remove_inventory_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	// SCG[12/7/99]: Make sure it's something that can touch it!
	Trigger_Reject( other, FL_MONSTER );

	if( !com->ValidTouch( self, other) )
	{
		return;
	}

	// SCG[12/7/99]: Check the users inventory for the item
	if( other->keyname )
	{
		if( other->inventory && ( other->flags & FL_CLIENT ) )
		{
			userInventory_t *pKeyItem = gstate->InventoryFindItem( other->inventory, self->keyname );
			if( pKeyItem )
			{
				gstate->InventoryDeleteItem( other, other->inventory, pKeyItem );
			}
		}
	}
}

void trigger_remove_inventory_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	trigger_remove_inventory_touch( self, other, NULL, NULL );
}

void trigger_remove_inventory_item( userEntity_t  *self )
{
	trigger_init( self );

	for( int i = 0; self->epair[i].key != NULL; i++ )
	{
		if( !stricmp( self->epair[i].key, "item" ) )
		{
			self->keyname = self->epair[i].value;
		}
	}

	self->touch		= trigger_remove_inventory_touch;
	self->use		= trigger_remove_inventory_use;

	if( self->keyname == NULL )
	{
		gstate->RemoveEntity( self );
	}
}

//-----------------------------------------------------------------------------

void trigger_register_func()
{
	gstate->RegisterFunc("trigger_console_touch",trigger_console_touch);
	gstate->RegisterFunc("trigger_skill_touch",trigger_skill_touch);
	gstate->RegisterFunc("multi_touch",multi_touch);
	gstate->RegisterFunc("tele_gib_touch",tele_gib_touch);
	gstate->RegisterFunc("teleport_touch",teleport_touch);
	gstate->RegisterFunc("onlyreg_touch",onlyreg_touch);
	gstate->RegisterFunc("trigger_hurt_touch",trigger_hurt_touch);
	gstate->RegisterFunc("trigger_push_touch",trigger_push_touch);
	gstate->RegisterFunc("trigger_changelevel_touch",trigger_changelevel_touch);
	gstate->RegisterFunc("trigger_change_sfx_touch",trigger_change_sfx_touch);
	gstate->RegisterFunc("trigger_script_touch",trigger_script_touch);
//	gstate->RegisterFunc("trigger_monsterjump_touch",trigger_monsterjump_touch);
//	gstate->RegisterFunc("trigger_fog_value_touch",trigger_fog_value_touch);
//	gstate->RegisterFunc("warp_touch",warp_touch);
	gstate->RegisterFunc("multi_use",multi_use);
	gstate->RegisterFunc("trigger_relay_use",trigger_relay_use);
	gstate->RegisterFunc("teleport_use",teleport_use);
	gstate->RegisterFunc("trigger_hurt_use",trigger_hurt_use);
	gstate->RegisterFunc("trigger_counter_use",trigger_counter_use);
	gstate->RegisterFunc("trigger_changelevel_use",trigger_changelevel_use);
	gstate->RegisterFunc("trigger_script_use",trigger_script_use);
	gstate->RegisterFunc("func_wall_explode_use",func_wall_explode_use);
//	gstate->RegisterFunc("trigger_fog_value_use",trigger_fog_value_use);
	gstate->RegisterFunc("trigger_sidekick_use",trigger_sidekick_use);

	gstate->RegisterFunc("trigger_sidekick_touch",trigger_sidekick_touch);
	gstate->RegisterFunc("trigger_sidekick_stop_touch",trigger_sidekick_stop_touch);
	gstate->RegisterFunc("trigger_sidekick_teleport_touch",trigger_sidekick_teleport_touch);
	gstate->RegisterFunc("multi_wait",multi_wait);
	gstate->RegisterFunc("trigger_remove",trigger_remove);
	gstate->RegisterFunc("spawn_tele_sound",spawn_tele_sound);
	gstate->RegisterFunc("tele_gib_remove",tele_gib_remove);
	gstate->RegisterFunc("trigger_hurt_reset",trigger_hurt_reset);
	gstate->RegisterFunc("changelevel_execute",changelevel_execute);
	gstate->RegisterFunc("trigger_changelevel_touch",trigger_changelevel_touch);
	gstate->RegisterFunc("trigger_changelevel_message_reset",trigger_changelevel_message_reset);
	gstate->RegisterFunc("trigger_changelevel_use",trigger_changelevel_use);
	gstate->RegisterFunc("trigger_changelevel_cinematic_think",trigger_changelevel_cinematic_think);
	gstate->RegisterFunc("rock_think",rock_think);
	gstate->RegisterFunc("blow_up",blow_up);
	gstate->RegisterFunc("func_wall_explode_remove2",func_wall_explode_remove2);
//	gstate->RegisterFunc("warp_appear_think",warp_appear_think);
//	gstate->RegisterFunc("stretchy_think",stretchy_think);
//	gstate->RegisterFunc("warp_appear",warp_appear);
	gstate->RegisterFunc("trigger_push_use",trigger_push_use);
	gstate->RegisterFunc("trigger_changetarget_use",trigger_changetarget_use);
	gstate->RegisterFunc("trigger_sidekick_stop_use",trigger_sidekick_stop_use);
	gstate->RegisterFunc("trigger_sidekick_teleport_use",trigger_sidekick_teleport_use);
	gstate->RegisterFunc("multi_pain",multi_pain);
	gstate->RegisterFunc("multi_killed",multi_killed);
	gstate->RegisterFunc("func_wall_explode_die",func_wall_explode_die);

	gstate->RegisterFunc( "trigger_toggle_touch", trigger_toggle_touch );
	gstate->RegisterFunc( "trigger_toggle_think", trigger_toggle_think );

	gstate->RegisterFunc( "trigger_changemusic_touch", trigger_changemusic_touch );
	gstate->RegisterFunc( "trigger_changemusic_use", trigger_changemusic_use );

	gstate->RegisterFunc( "warp_hook_save", warp_hook_save );
	gstate->RegisterFunc( "warp_hook_load", warp_hook_load );
	gstate->RegisterFunc( "sfx_trig_hook_save", sfx_trig_hook_save );
	gstate->RegisterFunc( "sfx_trig_hook_load", sfx_trig_hook_load );
	gstate->RegisterFunc( "script_hook_save", script_hook_save );
	gstate->RegisterFunc( "script_hook_load", script_hook_load );
	gstate->RegisterFunc( "trig_hook_save", trig_hook_save );
	gstate->RegisterFunc( "trig_hook_load", trig_hook_load );
	gstate->RegisterFunc( "rock_hook_save", rock_hook_save );
	gstate->RegisterFunc( "rock_hook_load", rock_hook_load );
	gstate->RegisterFunc( "trigger_changemusic_save", trigger_changemusic_save );
	gstate->RegisterFunc( "trigger_changemusic_load", trigger_changemusic_load );
	gstate->RegisterFunc( "trigger_toggle_hook_save", trigger_toggle_hook_save );
	gstate->RegisterFunc( "trigger_toggle_hook_load", trigger_toggle_hook_load );

	gstate->RegisterFunc( "trigger_crosslevel_trigger_use", trigger_crosslevel_trigger_use );
	gstate->RegisterFunc( "target_crosslevel_target_think", target_crosslevel_target_think );

	gstate->RegisterFunc( "trigger_remove_inventory_use", trigger_remove_inventory_use );
	gstate->RegisterFunc( "trigger_remove_inventory_touch", trigger_remove_inventory_touch );
}

