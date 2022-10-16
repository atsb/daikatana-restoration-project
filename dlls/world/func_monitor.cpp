// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
//#include "chasecam.h"// SCG[1/23/00]: not used
#include "ai_utils.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport	void	func_monitor (userEntity_t *self);
DllExport	void	info_camera (userEntity_t *self);

void com_FindParent (userEntity_t *self);

/////////////////////////////////////////////////////////////////////
//	defines
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
//	typedefs
/////////////////////////////////////////////////////////////////////

typedef	struct	monitorHook_s
{
	float			delay;
	float			fov;
	float			time_marker;
	userEntity_t	*lock_ent;
	userEntity_t	*Client;
	userEntity_t	*target;
	CVector			camera_origin;
	CVector			Old_Player_Angles;
} monitorHook_t;

#define	MONITORHOOK(x) (int)&(((monitorHook_t *)0)->x)
field_t monitor_hook_fields[] = 
{
	{"delay",				MONITORHOOK(delay),				F_FLOAT},
	{"fov",					MONITORHOOK(fov),				F_FLOAT},
	{"time_marker",			MONITORHOOK(time_marker),		F_FLOAT},
	{"lock_ent",			MONITORHOOK(lock_ent),			F_EDICT},
	{"Client",				MONITORHOOK(Client),			F_EDICT},
	{"target",				MONITORHOOK(target),			F_EDICT},
	{"camera_origin",		MONITORHOOK(camera_origin),		F_VECTOR},
	{"Old_Player_Angles",	MONITORHOOK(Old_Player_Angles),	F_VECTOR},
	{NULL, 0, F_INT}
};

void monitor_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, monitor_hook_fields, sizeof( monitorHook_t ) );
}

void monitor_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, monitor_hook_fields, sizeof( monitorHook_t ) );
}

/////////////////////////////////////////////////////////////////////
//	prototypes
/////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		info_camera
// Description:
// Input:
// Output:
// Note:
//	this is entity where the camera will look from
//	if info_camera has a target, then the view will
//	follow that target.
//
// ----------------------------------------------------------------------------
void info_camera (userEntity_t *self)
{
	self->className = "info_camera";

	//	default 0.1 second delay before using target
	self->delay = 0.1;

	for ( int i = 0; self->epair [i].key; i++ )
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
	}

	self->modelName = NULL;
	gstate->SetSize (self, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2 (self, self->s.origin);
}

// ----------------------------------------------------------------------------
//
// Name:		monitor_find_target
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monitor_find_target (userEntity_t *self)
{
	userEntity_t	*target;
	monitorHook_t	*hook = (monitorHook_t *) self->userHook;
	
	target = com->FindTarget (self->target);
	hook->target = target;
	if (!target)
	{
		gstate->Con_Printf ("WARNING:func_monitor at %s could not find target.\n",com->vtos(self->s.origin));
		return;
	}

	hook->camera_origin = target->s.origin;

	//	find the target of the camera
	if (target->target)
	{
		hook->lock_ent = com->FindTarget (target->target);
		//	don't need actual info_camera any longer
		//target->remove (target);
	}
	else
	{
		hook->lock_ent = NULL;
	}

	// is this camera attached to a moving entity?
	if (self->parentname)
	{
		// com_FindParent links the two entities together and when the parent moves, so will the child
		self->think = com_FindParent;
		self->nextthink = gstate->time + 0.10;
	}
	else
	{
		self->think = NULL;
	}
	return;
}

// ----------------------------------------------------------------------------
// NSS[11/2/99]:
// Name:		MONITOR_Stop
// Description:	Generic stop function to restore everything back to the client
// Input:		userEntity_t *self
// Output:
// Note:
//  This shouldn't crash the client anymore... when the client dies in camera mode.
// ----------------------------------------------------------------------------
void MONITOR_Stop(userEntity_t *self)
{
	playerHook_t	*phook	= AI_GetPlayerHook( self );
	userEntity_t	*camera = phook->camera;
	monitorHook_t	*mhook;
	
	if(camera)
	{
		mhook	= (monitorHook_t *) camera->userHook;
	}
	else
	{
		return;
	}
	
	
	//Turn off the letter box crap
	self->client->ps.rdflags &= ~RDF_LETTERBOX;

	//Reset the flags.
	self->flags &= ~(FL_CAMERAMODE|FL_FREEZE);

	//Clear the client's pmove flags
	self->client->ps.pmove.pm_flags = 0;
	
	//	turn off the camera
	gstate->SetCameraState (self, false, 0);

	// move view back to default entity
	gstate->SetClientViewEntity (self, self);
	
	//restore the player's original angles
	self->s.angles = mhook->Old_Player_Angles;

	//Restore the FOV to its original state
	phook->fov			= mhook->fov;
	
	//Reset the camera pointer
	phook->camera		= NULL;

	//Stop this camera from thinking
	camera->nextthink	= -1;
	camera->think		= NULL;

	//Now let's do the flash to the camera
	com->FlashClient(self, 1.0, -1.0, -1.0, -1.0, 0.75 );

	if(!deathmatch->value && !coop->value)
	{
		// NSS[3/12/00]:Go out of cinematic mode.(don't ask)
		gstate->bCinematicPlaying = 0;
	}
}


// ----------------------------------------------------------------------------
// NSS[11/2/99]:
// Name:		MONITOR_Think
// Description:	Think function to assure that the monitor exits after 'x' time
// Input:		userEntity_t *self
// Output: NA
// Note:
//
// ----------------------------------------------------------------------------
void MONITOR_Think(userEntity_t *self)
{
	monitorHook_t	*mhook	= (monitorHook_t *) self->userHook;
	
	if(gstate->time > mhook->time_marker)
	{
		MONITOR_Stop(mhook->Client);
	}
	self->nextthink = gstate->time + 0.5;
}

// ----------------------------------------------------------------------------
// NSS[11/2/99]:
// Name:		monitor_use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monitor_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	monitorHook_t	*mhook	= (monitorHook_t *) self->userHook;
	playerHook_t	*phook	= AI_GetPlayerHook( activator );

	if( phook == NULL )
	{
		gstate->Con_Printf ("WARNING: monitor_use: phook was NULL\n");
		return;
	}

	if( mhook == NULL )
	{
		gstate->Con_Printf ("WARNING: monitor_use: mhook was NULL\n");
		return;
	}

	//Timer trap for func_monitor.
	if( mhook->time_marker < gstate->time && activator->delay < gstate->time)
	{
		//NSS[11/23/99]:If we have no real player activator find the first client and use that
		if (!(activator->flags & FL_CLIENT))
		{
			userEntity_t *ent = gstate->FirstEntity();
			//Process through all the entities on the map
			while(ent)
			{
				//Make sure the entity is the right classname
				if(ent->flags & FL_CLIENT)
				{
					activator = ent;
					phook	= AI_GetPlayerHook( activator );
					break;
				}
				ent = gstate->NextEntity(ent);
			}
		}

		if(activator->flags & FL_CAMERAMODE)
		{
			if(mhook->Client)
			{
				MONITOR_Stop (mhook->Client);
			}
		}
		
		//Set the thinks to NULLS
		self->prethink = NULL;
		self->postthink = NULL;
		
		//Store the player's fov
		mhook->fov = phook->fov;
		//Store the player's current angles
		mhook->Old_Player_Angles = activator->s.angles;
		
		if (!mhook->lock_ent)
		{
			gstate->Con_Printf ("WARNING: monitor_use has no target\n");
			return;
		}
		
		
		//Store who is viewing this monitor
		mhook->Client = activator;
		//Store the monitor on the viewer's end too
		phook->camera = self;
		
		
		//Get the facing angles for the monitor
		CVector ang,Dir;
		Dir = mhook->lock_ent->s.origin - mhook->target->s.origin;
		Dir.Normalize();
		VectorToAngles(Dir,ang);
	
		//Set the angles of the target
		mhook->target->s.angles = ang;
		//Set the view entity
		gstate->SetClientViewEntity (activator, mhook->target);

		//Set the view to letterbox
//		gstate->SetCvar("scr_letterbox","1");	
		activator->client->ps.rdflags |= RDF_LETTERBOX;

		//	simply turns a camera to track its target
		//	lock all angles to camera
		gstate->SetCameraState (activator, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z | CAMFL_NOLERP);
		
		//Set the view angles for the client
		gstate->SetClientAngles( activator, ang );

		//Put the client into cameramode and freeze it.
		activator->flags |= (FL_CAMERAMODE | FL_FREEZE);


		//Timer for monitor to display;
		mhook->time_marker = gstate->time + mhook->delay;
		self->think = MONITOR_Think;
		self->nextthink = gstate->time + 0.5;
		activator->delay = mhook->time_marker;
		
		//gstate->LinkEntity(activator);

		//Now let's do the flash to the camera
		com->FlashClient(activator, 1.0, -1.0, -1.0, -1.0, 0.75 );

		if(!deathmatch->value && !coop->value)
		{
			// NSS[3/12/00]:Go into cinematic mode.(don't ask)
			gstate->bCinematicPlaying = 1;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		func_monitor
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void func_monitor (userEntity_t *self)
{
	int					i;
	monitorHook_t	*hook;


	userEntity_t *ent = gstate->FirstEntity();
	
	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(monitorHook_t),MEM_TAG_HOOK);
	hook = (monitorHook_t *) self->userHook;

	self->className = "func_monitor";

	//FIXME: find actual player's default FOV and keep it instead
	//hook->fov = 90.0; 
	
	//default delay is 10 seconds
	hook->delay = 3.0f;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "fov"))
			hook->fov = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "wait"))
		{
			hook->delay = atof (self->epair [i].value);
			if(hook->delay < 0.75f)
				hook->delay = 0.75f;
		}
	}

	hook->time_marker = gstate->time;

	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;

	gstate->SetOrigin2 (self, self->s.origin);
	gstate->SetModel (self, self->modelName);

	self->use = monitor_use;

	self->save = monitor_hook_save;
	self->load = monitor_hook_load;

	self->nextthink = gstate->time + 0.2;
	self->think = monitor_find_target;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void monitor_register_func()
{
	gstate->RegisterFunc("monitor_use",monitor_use);
	gstate->RegisterFunc("monitor_find_target",monitor_find_target);
	gstate->RegisterFunc("monitor_hook_save",monitor_hook_save);
	gstate->RegisterFunc("monitor_hook_load",monitor_hook_load);
}




