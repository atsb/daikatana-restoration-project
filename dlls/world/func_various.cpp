#include "world.h"

/////////////////////////////////////////////////////////////////////
//	exports
/////////////////////////////////////////////////////////////////////

DllExport	void	func_water (userEntity_t *self);
DllExport	void	func_timer (userEntity_t *self);
DllExport	void	func_dynalight (userEntity_t *self);

/////////////////////////////////////////////////////////////////////
//	defines
/////////////////////////////////////////////////////////////////////

#define FUNC_DYNALIGHT_START_ON				0x0001
#define LIGHT_ON				0x0001

#define	USE_ONCE				0x0002
 
// for func_dynalight
#define SPOTLIGHT				0x0002
#define LIGHTFLARE				0x0004
#define FUNC_DYNALIGHT_X_AXIS			0x0008
#define FUNC_DYNALIGHT_Y_AXIS			0x0010
#define FUNC_DYNALIGHT_Z_AXIS			0x0020
#define REVERSE					0x0040

#define WATER_START_OPEN		0x01
#define WATER_TOGGLE			0x08

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

/////////////////////////////////////////////////////////////////////
//	typedefs
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
//	prototypes
/////////////////////////////////////////////////////////////////////

void	door_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void	door_parse_sounds (userEntity_t *self);
void	com_FindParent (userEntity_t *self);

// SCG[11/19/99]: Save game stuff
#define	TIMERHOOK(x) (int)&(((timerHook_t *)0)->x)
field_t timer_hook_fields[] = 
{
	{"wait",		TIMERHOOK(wait),		F_FLOAT},
	{"random",		TIMERHOOK(random),		F_FLOAT},
	{"delay",		TIMERHOOK(delay),		F_FLOAT},
	{"pausetime",	TIMERHOOK(pausetime),	F_FLOAT},
	{"other",		TIMERHOOK(other),		F_EDICT},
	{"activator",	TIMERHOOK(activator),	F_EDICT},
	{NULL, 0, F_INT}
};

///////////////////////////////////////////////////////////////////////////////
//	func_water
//	this is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.
//
//	flag		START_OPEN causes the water to move to its destination when spawned and operate in reverse.
//
//	angle		determines the opening direction (up or down only)
//	speed		movement speed (25 default)
//	wait		wait before returning (-1 default, -1 = TOGGLE)
//	lip			lip remaining at end of move (0 default)
//	sounds		0 = no sound  1 = water  2 = lava
//
///////////////////////////////////////////////////////////////////////////////

void door_hook_save( FILE *f, edict_t *ent );
void door_hook_load( FILE *f, edict_t *ent );

void	func_water (userEntity_t *self)
{
	doorHook_t	*hook;
	int			i;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(doorHook_t),MEM_TAG_HOOK);
	hook = (doorHook_t *) self->userHook;

	// this is really a hack (copied from func_door)  to make the door toggleable without
	// hook->no_more_move being set to true
	hook->wait = 32767;		

	self->targetname = self->target = self->killtarget = NULL;

	// SCG[2/2/00]: Save game stuff
	self->save = door_hook_save;
	self->load = door_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lip"))
			hook->lip = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "speed"))
			hook->speed = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;

		i++;
	}
	
	door_parse_sounds (self);

	com->SetMovedir (self);

	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	gstate->SetModel (self, self->modelName);

	//NSS[11/19/99]:Let's link this entity 
	gstate->LinkEntity(self);
	//gstate->SetOrigin (self, self->absmax.x - self->absmin.x, self->absmax.y - self->absmin.y, self->absmax.z - self->absmin.z);
	

	// precache the null sound so it can be used to cut off a sound playing when
	// a door opens but has no sound_top
	gstate->SoundIndex ("null.wav");

	self->use = door_use;

	if (!hook->speed)
		hook->speed = 25.0;

	// start position
	hook->pos1 = self->s.origin;

	// find ending position
	hook->pos2 = hook->pos1 + self->movedir * fabs( DotProduct(self->movedir, self->size) - hook->lip);

	if (self->spawnflags & WATER_START_OPEN)
	{
		gstate->SetOrigin (self, hook->pos2.x, hook->pos2.y, hook->pos2.z);
		hook->state = STATE_TOP;
	}
	else
		hook->state = STATE_BOTTOM;

	hook->move_finished = 0.0;
	hook->no_more_move = FALSE;		// we always want this to be false for func_water

	// this must be set up if LinkDoors is not working
	self->owner = self;
	hook->linked_door = NULL;

	// if there is no wait for return, make it toggleable 
	if (hook->wait == 32767)
		self->spawnflags |= WATER_TOGGLE;

	self->think = NULL;
	self->nextthink = -1;
	
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_timer_think
//
///////////////////////////////////////////////////////////////////////////////

void timer_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, timer_hook_fields, sizeof( timerHook_t ) );
}

void timer_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, timer_hook_fields, sizeof( timerHook_t ) );
}

void func_timer_think (userEntity_t *self)
{
	timerHook_t	*hook;

	hook = (timerHook_t *) self->userHook;

	com->UseTargets (self, hook->other, hook->activator);
	self->nextthink = gstate->time + hook->wait + crandom() * hook->random;

	// if it's only used once...
	if (self->spawnflags & USE_ONCE)
		self->remove(self);

	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_timer_use
//
///////////////////////////////////////////////////////////////////////////////

void func_timer_use	(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	timerHook_t	*hook;

	hook = (timerHook_t *) self->userHook;
	
	hook->other = other;
	hook->activator = activator;

	// if on, turn it off
	if (self->nextthink)
	{
		self->nextthink = 0;
		return;
	}

	// turn it on
	if (hook->delay)
		self->nextthink = gstate->time + hook->delay;
	else
		func_timer_think (self);
}

///////////////////////////////////////////////////////////////////////////////
//	func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) FUNC_DYNALIGHT_START_ON
//	wait		- base time between triggering all targets, default is 1
//	random		- wait variance, default is 0
//
//	so, the basic time between firing is a random time between
//	(wait - random) and (wait + random)
//
//	delay		- delay before first firing when turned on, default is 0
//
//	pausetime	- additional delay used only the very first time
//				  and only if spawned with FUNC_DYNALIGHT_START_ON
//
///////////////////////////////////////////////////////////////////////////////

void	func_timer (userEntity_t *self)
{
	timerHook_t	*hook;
	int			i;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(timerHook_t),MEM_TAG_HOOK);
	hook = (timerHook_t *) self->userHook;

	memset (hook, 0x00, sizeof (timerHook_t));
	
	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "wait"))
			hook->wait = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "random"))
			hook->random = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "delay"))
			hook->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "pausetime"))
			hook->pausetime = atof (self->epair [i].value);

		i++;
	}

	if (!hook->wait)
		hook->wait = 1.0;

	self->use = func_timer_use;
	self->think = func_timer_think;

	// SCG[11/24/99]: Save game stuff
	self->save = timer_hook_save;
	self->load = timer_hook_load;

	if (hook->random >= hook->wait)
	{
		hook->random = hook->wait - 0.1;
		gstate->Con_Printf("func_timer at (%f,%f,%f) has random >= wait\n", self->s.origin[0], self->s.origin[1], self->s.origin[2]);
	}

	if (self->spawnflags & 1)
	{
		self->nextthink = gstate->time + 1.0 + hook->pausetime + hook->delay + hook->wait + crandom() * hook->random;
		hook->other = hook->activator = self;
	}

	self->svflags |= SVF_NOCLIENT;
}


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&777



///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

typedef struct dynalightHook_s
{
	float	brightness;
	float	length;
	float	rotation_speed;
} dynalightHook_t;

#define	DYNALIGHTHOOK(x) (int)&(((dynalightHook_t *)0)->x)
field_t dynalight_hook_fields[] = 
{
	{"brightness",		DYNALIGHTHOOK(brightness),		F_FLOAT},
	{"length",			DYNALIGHTHOOK(length),			F_FLOAT},
	{"rotation_speed",	DYNALIGHTHOOK(rotation_speed),	F_FLOAT},
	{NULL, 0, F_INT}
};

void func_dynalight_think (userEntity_t *self)
{

	userEntity_t	*ignore;
	trace_t			tr;
	CVector			start, end, point, dir;
	dynalightHook_t	*dynalightHook;
	
	dynalightHook = ( dynalightHook_t * ) self->userHook;

	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	if( dynalightHook == NULL )
	{
		self->think = NULL;
		return;
	}

	if (self->enemy)
	{
		point = self->enemy->absmin + self->enemy->size * 0.5;
		dir = point - self->s.origin;
		dir.Normalize();
		self->movedir = dir;
	}
	else
	{
		CVector	ang = self->s.angles;
		AngleToVectors( ang, dir );
	}
												// scoot the start of the trace forward..
	start = self->s.origin + (dir * 16);		// the dynalight might be embedded in a bmodel or something
	end = start + (dir * dynalightHook->length);

	ignore = self->enemy;

	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}

	self->s.render_scale = tr.endpos;

	if (!self->enemy && !self->parentname && !((self->s.renderfx & RF_SPOTLIGHT) && (self->spawnflags & (FUNC_DYNALIGHT_X_AXIS|FUNC_DYNALIGHT_Y_AXIS|FUNC_DYNALIGHT_Z_AXIS))))
	{
		// don't waste processor cycles for dynalights just sitting there
		self->think = NULL;
	}
	else
	{
		self->think = func_dynalight_think;
		self->nextthink = gstate->time + 0.10;
	}

	// SCG[10/28/99]: Entities need to have an alpha less than 1.0 in order to be added to the alpha entities list.
	self->s.alpha = 0.99;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void func_dynalight_on (userEntity_t *self)
{
	CVector moveDir, temp;
	dynalightHook_t	*dynalightHook;
	
	dynalightHook = ( dynalightHook_t * ) self->userHook;

	if( dynalightHook == NULL )
	{
		return;
	}

	self->spawnflags |= LIGHT_ON;
	self->svflags &= ~SVF_NOCLIENT;	

	
	// start rotating
	if (self->spawnflags & (FUNC_DYNALIGHT_X_AXIS|FUNC_DYNALIGHT_Y_AXIS|FUNC_DYNALIGHT_Z_AXIS))
	{
		if (self->spawnflags & FUNC_DYNALIGHT_X_AXIS)
			moveDir.x = 1.0;
		if (self->spawnflags & FUNC_DYNALIGHT_Y_AXIS)
			moveDir.z = 1.0;
		if (self->spawnflags & FUNC_DYNALIGHT_Z_AXIS)
			moveDir.y = 1.0;

		CVector temp = moveDir * dynalightHook->rotation_speed;

		if (self->spawnflags & REVERSE)
			temp = temp * -1.0;

		self->avelocity = temp;
	}

	// we only need to call the think function if this is a spotlight
	if (self->s.renderfx & RF_SPOTLIGHT)
		func_dynalight_think (self);
	else
		self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void func_dynalight_off (userEntity_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
	self->nextthink = 0;

	// stop rotating
	if (self->spawnflags & (FUNC_DYNALIGHT_X_AXIS|FUNC_DYNALIGHT_Y_AXIS|FUNC_DYNALIGHT_Z_AXIS))
	{
		self->avelocity.Zero();
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void func_dynalight_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (self->spawnflags & LIGHT_ON)
		func_dynalight_off (self);
	else
		func_dynalight_on (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void func_dynalight_start (userEntity_t *self)
{

	if (self->spawnflags & FUNC_DYNALIGHT_START_ON)
		func_dynalight_on (self);
	else
		func_dynalight_off (self);

	// if this has a parent, initialize the parent/child link
	if (self->parentname)
	{
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.3;
	}
//	else
//	{
//		self->think = NULL;
//		self->nextthink = -1;
//	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	func_dynalight
//
///////////////////////////////////////////////////////////////////////////////

void func_dynalight (userEntity_t *self)
{
	int				i, radius = 0, length = 2048;
	userEntity_t	*ent;
	CVector			ang, temp;
	CVector			color;
	qboolean		bFoundColor = FALSE;
	float			brightness = 200.0, speed = 100.0;
	char *model		= "models/global/e_flare2.sp2";	// default value here.. taken from light.cpp
	dynalightHook_t	*dynalightHook;
	
	self->userHook = gstate->X_Malloc( sizeof( dynalightHook_t ), MEM_TAG_HOOK );
	dynalightHook = ( dynalightHook_t * ) self->userHook;

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_NOT;
	self->s.renderfx = RF_PARTICLEVOLUME;	// KICK ASS HACK SO THE SERVER GETS THE NEW ENTITY VALUES
	self->s.modelindex = 1;		// must be non-zero

	self->s.color.Set( 1, 1, 1 );

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair[i].key, "light_lev"))	// note "light" is changed to "light_lev" in dksent
			 brightness = atof (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "radius"))
			 radius = atoi (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "length"))
			 length = atoi (self->epair[i].value);

		else if (!stricmp (self->epair[i].key, "model"))
			 model = self->epair[i].value;

		else if (!stricmp (self->epair[i].key, "_color"))
		{
			bFoundColor = TRUE;
			sscanf(self->epair [i].value, "%f%f%f", 
			    &self->s.color.x,
			    &self->s.color.y,
			    &self->s.color.z);
		}
		else if (!stricmp (self->epair[i].key, "speed"))
			 speed = atof (self->epair[i].value);
	}

	// add a spotlight to this thing
	if (self->spawnflags & SPOTLIGHT)
	{
		self->s.renderfx |= (RF_SPOTLIGHT|RF_TRANSLUCENT);
		if (self->spawnflags & LIGHTFLARE)		// you can also specify a lightflare
		{
			self->s.renderfx |= RF_LIGHTFLARE;
		}
		self->s.renderfx &= ~RF_PARTICLEVOLUME;
		self->s.modelindex = gstate->ModelIndex (model);
	}
	else if (self->spawnflags & LIGHTFLARE)
	{
		self->s.renderfx = SPR_ALPHACHANNEL | RF_TRANSLUCENT | RF_LIGHTFLARE;
		self->s.modelindex = gstate->ModelIndex (model);
	}
	else 
		self->s.effects |= EF_BRIGHTLIGHT;
	

	// set the spotlight diameter
	if (radius)
		self->s.frame = radius;
	else
		self->s.frame = 4;
	
	dynalightHook->brightness = brightness;
	dynalightHook->length = length;
	dynalightHook->rotation_speed = speed;

	// set the normal light color
	if (!self->owner)
		self->owner = self;

	// if the dynalight is to target something, get a pointer to that entity
	if (self->target)
	{
		ent = com->FindTarget (self->target);
		self->enemy = ent;
	}
	else
	{		
		// this sets up the angles right for dynalights that have no targets
		if ((self->s.angles.x == 0.0) && (self->s.angles.y == -1.0) && (self->s.angles.z == 0.0))
		{
			// up
			self->s.angles.Set(270.0, 0.0, 0.0);
		}
		else 
		if ((self->s.angles.x == 0.0) && (self->s.angles.y == -2.0) && (self->s.angles.z == 0.0))
		{
			// down
			self->s.angles.Set(90.0, 0.0, 0.0);
		}
	}



	self->use = func_dynalight_use;
	self->think = func_dynalight_think;

	// let everything else get spawned before we start setting up
	self->think = func_dynalight_start;
	self->nextthink = gstate->time + 0.1;

	// put this entity in the world
	gstate->LinkEntity (self);
}





#if 0	//&&& AMW - not finished merging this in yet...

#define	CLOCK_MESSAGE_SIZE	16


///////////////////////////////////////////////////////////////////////////////
//
//	func_clock_reset
//
//  don't let field width of any clock messages change, or it
//  could cause an overwrite after a game load
//
///////////////////////////////////////////////////////////////////////////////

static void func_clock_reset (userEntity_t *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_clock_format_countdown
//
///////////////////////////////////////////////////////////////////////////////

static void func_clock_format_countdown (userEntity_t *self)
{
	if (self->style == 0)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_clock_think
//
///////////////////////////////////////////////////////////////////////////////

void func_clock_think (userEntity_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets (self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_clock_use
//
///////////////////////////////////////////////////////////////////////////////

void func_clock_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think (self);
}

///////////////////////////////////////////////////////////////////////////////
//
//	func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
//
//  target a target_string with this
//
//The default is to be a time of day clock
//
//TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
//If START_OFF, this entity must be used before it starts
//
//"style"		0 "xx"
//			1 "xx:xx"
//			2 "xx:xx:xx"
//
//
///////////////////////////////////////////////////////////////////////////////

void func_clock (userEntity_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60*60;;

	func_clock_reset (self);

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, MEM_TAG_LEVEL);

	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_func_various_register_func()
{
	gstate->RegisterFunc("func_timer_think",func_timer_think);
	gstate->RegisterFunc("func_dynalight_think",func_dynalight_think);
	gstate->RegisterFunc("func_dynalight_start",func_dynalight_start);
//	gstate->RegisterFunc("func_clock_think",func_clock_think);
//	gstate->RegisterFunc("func_clock_use",func_clock_use);
	gstate->RegisterFunc("timer_hook_save",timer_hook_save);
	gstate->RegisterFunc("timer_hook_load",timer_hook_load);
	gstate->RegisterFunc("func_timer_use",func_timer_use);
}
