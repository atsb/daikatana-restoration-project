//Complex Particle by: N.S.S.

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
//#include "ai_func.h"// SCG[1/23/00]: not used
//#include "MonsterSound.h"// SCG[1/23/00]: not used
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "ai_common.h"

DllExport void sfx_complex_particle(userEntity_t *self);

//*****************************************  Particle types and related items
#define PAR_SIMPLE				0x00000001
#define PAR_CP3					0x00000002
#define PAR_RAIN				0x00000004
#define PAR_SMOKE				0x00000008
#define PAR_CP1					0x00000010
#define PAR_CP2					0x00000020
#define PAR_CP4					0x00000040
#define PAR_BUBBLE				0x00000080

//*****************************************  Other flag related items
#define EMIT_TOGGLE				0x00000100	
#define EMIT_RANDOM				0x00000200	//Turns random emission time on and off
#define TOGGLE_OFF				0x00000400	//Sets the particle in the off state.
#define TOGGLE_ONOFF			0x00000800	//Will toggle on and off depending on current state (used with Toggle_OFF).
#define NOTINPVS				0x00001000	//Flag for PVS stuff



/* ***************************** Local Structs *************************** */
typedef struct
{
    int onoff;									// Simple on/off
	int NumParticles;							// # Particles.
	int Velocity;								// Velocity for the particles
	int Spread;									// Spread for particles
	int Particle_Type;							// Type of particle translated from Epair flag.
	int Toggle_Emission;						// Toggle Emission deal
	int Random_Emit;							// Random Emit (I know waste of an INT space)
	int	InternalFlags;							// Flags that keep track of things
	int radius;									// radius for spread base
	float GType;								// Gravity amount applied
	float Scale;								// Scale for particles
	float Delta_Alpha;							// Frequency of Alpha change
	float Alpha_Level;							// Initial Alpha Level
	float Emit_Freq;							// Emission frequency
	float Emit_Time;							// Time for particle Emission(only if Toggle Emission flag set)
	float stoptime;								// Time for the entire SFX particle to generate what it is gonna do.. then quit until triggered again.
	float lifetime;								// time to live
	char message[32];							// Debug only(to tag specific particle types)
	CVector Color;								// Color of particle
	CVector Target;								// Target Direction
	CVector Gravity;							// Direction of Gravity
} sfx_particle_hook;

void SFX2_Think(userEntity_t *self);
void SFX_Spawn(userEntity_t *self);

void sfx_particle_hook_save( FILE *f, struct edict_s *ent )
{
	com->AI_SaveHook( f, ent->userHook, sizeof( sfx_particle_hook ) );
}

void sfx_particle_hook_load_think( userEntity_t *ent )
{
	sfx_particle_hook *hook =  ( sfx_particle_hook * )ent->userHook;

	if( hook->onoff )
	{
		hook->InternalFlags |= NOTINPVS;
//		hook->onoff = 0;
		if(!hook->stoptime)								//This case we are free running and only need call ourself once or until used again.(particles stay on)
		{
			ent->think = SFX_Spawn;
		}
		else											//This case we will check the stoptime in the think function to stop ourself.
		{
			SFX_Spawn( ent );
			hook->lifetime = hook->stoptime + gstate->time;
		}		
	}
	ent->nextthink = gstate->time + 2.5f;
}

void sfx_particle_hook_load( FILE *f, struct edict_s *ent )
{
	ent->userHook = gstate->X_Malloc( sizeof( sfx_particle_hook ),MEM_TAG_COM_MISC );
	com->AI_LoadHook( f, ent->userHook, sizeof( sfx_particle_hook ) );
	sfx_particle_hook *hook =  ( sfx_particle_hook * )ent->userHook;

	ent->think = sfx_particle_hook_load_think;
	ent->nextthink = gstate->time + 5.0f;
}

// ----------------------------------------------------------------------------
// <NSS>
// Name:        sfx2_ParseEpairs
// Description:  Get information to figure out what we need to do.
// Input:userEntity_t *self
// Output:NA
// Note:
// Definitions for assignment of values
/*
   float length;	----> Alpha_Level
   CVector lightColor;--> Color
   float lightSize;	----> Delta_Alpha
   int modelindex;  ----> Particle_Type
   short numframes; ----> Spread
   float scale;		----> Scale	
   float frametime; ----> Emit_Freq
   altpos.x   ----> NumParticles
   altpos.y   ----> Velocity
   altpos.z   ----> GType
*/
// ----------------------------------------------------------------------------
void SFX_Spawn(userEntity_t *self)
{
	sfx_particle_hook *hook =  (sfx_particle_hook *)self->userHook;
	
	trackInfo_t tinfo;

    // clear this variable
	AI_Dprintf("Sending a packet!\n");
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=self;
    tinfo.srcent=self;
	if(hook->message)
		AI_Dprintf("SFXCP:%s created track entity!\n",hook->message);
	//Configure the track entity
	tinfo.length		= hook->Alpha_Level;
	tinfo.lightColor	= hook->Color;
	tinfo.lightSize		= hook->Delta_Alpha;
	tinfo.modelindex	= hook->Particle_Type;
	tinfo.scale			= hook->Scale;
	tinfo.frametime		= hook->Emit_Freq;
	tinfo.altpos.x		= hook->NumParticles;
	tinfo.altpos.y		= hook->Velocity;
	tinfo.scale2		= hook->Toggle_Emission;//total fucked up hack  (ahhh... perhaps add more var's to track entity)
	tinfo.altangle.y    = hook->Emit_Time;
	tinfo.altangle.x	= (float) hook->radius;
	tinfo.numframes     = (short) hook->Spread;
	tinfo.altpos2		= hook->Target;
	tinfo.modelindex2	= hook->Random_Emit;
	tinfo.dstpos		= (hook->Gravity * hook->GType);
	tinfo.fxflags		= TEF_COMPLEXPARTICLE;
//	sprintf(tinfo.HardPoint_Name,"%s",hook->message);
	Com_sprintf(tinfo.HardPoint_Name,sizeof(tinfo.HardPoint_Name),"%s",hook->message);
	
	//Flags to tell what settings we have changed
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_SCALE|TEF_ALTPOS|TEF_MODELINDEX|TEF_NUMFRAMES|TEF_LENGTH|TEF_FRAMETIME|TEF_DSTPOS|TEF_ALTPOS2|TEF_ALTANGLE|TEF_MODELINDEX2|TEF_HARDPOINT|TEF_DSTPOS|TEF_SCALE2;
	//Flags to tell what FX to add to the track ent
	
    
	com->trackEntity(&tinfo,MULTICAST_ALL);

	self->think = SFX2_Think;
	self->nextthink = gstate->time + 0.5f;

	hook->onoff = 1;
}

//NSS[11/28/99]:Added specificly for this scenario.
int FindClientInPVS(userEntity_t *self);


// ----------------------------------------------------------------------------
// N.S.S.
// Name:        SFX2_Think
// Description:  Think function for complex particle
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SFX2_Think(userEntity_t *self)
{
	sfx_particle_hook *hook =  (sfx_particle_hook *)self->userHook;

	//Check to see if we have a client near.
	int InPVS = FindClientInPVS(self);

	//If not and we are running, let's turn ourself off.//NSS[11/30/99]:Will helped me find out my stupidity... added NOTINPVS CHECK FLAG!!!
	if( (!InPVS) && (hook->onoff == 1) && !(hook->InternalFlags & NOTINPVS))
	{
		AI_Dprintf("Sending a packet!\n");
		RELIABLE_UNTRACK(self);
		self->nextthink = gstate->time + 0.5f;
		hook->InternalFlags |= NOTINPVS;
		return;
	}
	else if( InPVS && (hook->InternalFlags & NOTINPVS) ) 
	{//If a client enters into our PVS and we were turned off bc of PVS b4 then turn on again.
		//If we were on before
		if(hook->onoff)
		{
			if(!hook->stoptime)								//This case we are free running and only need call ourself once or until used again.(particles stay on)
			{
				self->think		= SFX_Spawn;
				self->nextthink = gstate->time + 0.2;
			}
			else											//This case we will check the stoptime in the think function to stop ourself.
			{
				SFX_Spawn(self);
				hook->lifetime	= hook->stoptime + gstate->time;
				self->nextthink = gstate->time + 0.3;
			}		
		}
		//Reset the internal flag thingy...
		hook->InternalFlags &= ~NOTINPVS;
	}
	
	if(hook->stoptime && hook->onoff)
	{
		if(hook->lifetime < gstate->time)
		{
			AI_Dprintf("Sending a packet!\n");
			RELIABLE_UNTRACK(self);
			//Reset our thinking now
			//self->think		= NULL;
			//self->nextthink = -1;
			//Set the on off hook to off
			hook->onoff		= 0;
			//return;
		}
	}
	
	self->nextthink = gstate->time + 0.3;
}


// ----------------------------------------------------------------------------
// N.S.S.
// Name:        SFX2_Use
// Description:  The use function for Complex Particles
// Input:userEntity_t *self, userEntity_t *other, userEntity_t *activator
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SFX2_Use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	sfx_particle_hook *hook =  (sfx_particle_hook *)self->userHook;
	if(!hook->onoff)									//Turn On
	{
		hook->onoff = 1;
		if(!hook->stoptime)								//This case we are free running and only need call ourself once or until used again.(particles stay on)
		{
			self->think		= SFX_Spawn;
			self->nextthink = gstate->time + 0.2;
		}
		else											//This case we will check the stoptime in the think function to stop ourself.
		{
			SFX_Spawn(self);
			self->think		= SFX2_Think;
			hook->lifetime	= hook->stoptime + gstate->time;
			self->nextthink = gstate->time + 0.3;
		}
	}
	else if(hook->InternalFlags & TOGGLE_ONOFF)			//Turn off (Completely turn off don't worry about thinking...wait until toggled on again).
	{
		RELIABLE_UNTRACK(self);
		hook->onoff		= 0;
		self->think		= NULL;
		self->nextthink = -1;
	}
}

// ----------------------------------------------------------------------------
// N.S.S.
// Name:        sfx2_ParseEpairs
// Description:  Get information to figure out what we need to do.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SFX2_ParseEpairs(userEntity_t *self)
{
	int nSpawnValue;
	float dummy_float;

	self->flags     &= ~FL_MONSTER;
	self->flags     &= ~FL_BOT;

	sfx_particle_hook *hook =  (sfx_particle_hook *)self->userHook;
	//Setup default values
	hook->onoff				= 0;				//Start off
	hook->NumParticles		= 1;				//Number of particles to start with
	hook->Toggle_Emission	= 0;				//Don't toggle between emissions
	hook->Spread			= 2.0f;				//Spread for particles.
	hook->GType				= 0.0f;				//Amount of Gravity to apply
	hook->Velocity			= 35;				//Velocity of particles
	hook->Scale				= 1.00f;			//Scale of particles
	hook->Alpha_Level		= 0.75;				//Starting Alpha level
	hook->Delta_Alpha		= 0.75f;			//Change in Alpha level
	hook->Emit_Time			= 12.0f	;			//Set a default Emit Time(even though most times the toggle emission won't be set)
	hook->Random_Emit		= 0;				//Random Emission
	hook->Particle_Type		= PARTICLE_SIMPLE;	//Set the particle type
	hook->stoptime			= 0.0f;				//Time for particles to emit.
	hook->radius			= 0;				//default radius spread is 0
	hook->lifetime			= 0;				//Zero out the lifetime variable.
	hook->Color.Set(1.0f,1.0f,1.0f);			//Set default color to white.
	hook->Gravity.Set(0.0f,.0.0f,-1.0f);		//Automatically set the gravity direction to go down.
	
	
	
	
	//Parse through the key values to get the number of particles and other things
	//I am guessing that this will be a commonly used entity for SFX.  Trying to add
	//as much as possible...but I could miss something.
	if (self->epair)
	{
	    int i = 0;
		while ( self->epair[i].key )
	    {
			//Get the values
			if( !stricmp(self->epair [i].key, "target"))//find the target and get the vector to the target
			{
				userEntity_t *ent = findEntityFromName(self->epair[i].value);
				if(ent != NULL)
				{
					hook->Target = ent->s.origin - self->s.origin;
					hook->Target.Normalize();
				}
				else
				{
					hook->Target = self->s.origin;
				}
			}
			else if( !stricmp(self->epair [i].key, "gravitydir"))//find the gravity target and calculate the normal to it
			{
				userEntity_t *ent = findEntityFromName(self->epair[i].value);
				if(ent != NULL)
				{
					hook->Gravity = ent->s.origin - self->s.origin;
					hook->Gravity.Normalize();
					gstate->RemoveEntity(ent);
				}
				else
				{
					//Set the direction to down.  Default direction is down.
					hook->Gravity.Set(0.0f,.0.0f,-1.0f);
				}
			}
			else if ( _stricmp(self->epair[i].key, "spread") == 0 )
		    {
			    hook->Spread = atoi(self->epair[i].value);
		    }
			else if ( _stricmp(self->epair[i].key, "emissiontime") == 0 )
		    {
			    hook->Emit_Time = atof(self->epair[i].value);
				if(hook->Emit_Time <= 0.01)
				{
					hook->Emit_Time = 0.01f;
				}

		    }
			else if ( _stricmp(self->epair[i].key, "radius") == 0 )
		    {
			    hook->radius = atoi(self->epair[i].value);
		    }
			else if ( _stricmp(self->epair[i].key, "count") == 0 )
		    {
			    hook->NumParticles = atoi(self->epair[i].value);
		    }

			else if ( _stricmp(self->epair[i].key, "velocity") == 0 )
			{
				hook->Velocity = atoi(self->epair[i].value);
			}
			else if ( _stricmp(self->epair[i].key, "gravity") == 0 )
			{
				hook->GType = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "stoptime") == 0 )
			{
				hook->stoptime = atof(self->epair[i].value);
			}
			else if ( _stricmp(self->epair[i].key, "emission") == 0 )
			{
				hook->Emit_Freq = atof(self->epair[i].value);
				if(hook->Emit_Freq <= 0.01)
				{
					hook->Emit_Freq = 0.01f;
				}
			}
			else if ( _stricmp(self->epair[i].key, "_color") == 0 )
			{
				sscanf(self->epair[i].value,"%f%f%f",&hook->Color.x,&hook->Color.y,&hook->Color.z);
			}
			else if( _stricmp(self->epair[i].key, "message") == 0)
			{
				//Debug point
				AI_Dprintf("Message SFX found. Debug here.\n");
//				sprintf(hook->message,"%s",self->epair[i].value);
				Com_sprintf(hook->message,sizeof(hook->message),"%s",self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "scale") == 0)
			{
				dummy_float   = atof(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_float < 0.01f)
				{
					dummy_float = 0.01f;
				}
				else if(dummy_float > 200.f)
				{
					dummy_float = 200.0f;
				}
				hook->Scale =  dummy_float;
			}
			else if( _stricmp(self->epair[i].key, "delta_alpha") == 0)
			{
				dummy_float   = atof(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_float <=0.01f)
				{
					hook->Delta_Alpha = 0.75f;
				}
				else
				{
					hook->Delta_Alpha = dummy_float;
				}
			}
			else if( _stricmp(self->epair[i].key, "alpha_level") == 0)
			{
				dummy_float   = atof(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_float <= 0.01)
				{
					hook->Alpha_Level = 0.01f;
				}
				else
				{
					hook->Alpha_Level = dummy_float;
				}
			}
			else if( _stricmp(self->epair[i].key,"violence") == 0)
			{
				// NSS[3/28/00]:Make sure that we shouldn't be going off for those non-violent people.
				int Set_Violence = atoi(self->epair[i].value);
				// NSS[3/28/00]:It always removes anything marked as a 'violent' thing for German version.
// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
				if(gstate->GetCvar("sv_violence") && Set_Violence)
#endif
				{
					self->remove( self );
					return;
				}

			}
			else if ( _stricmp( self->epair[i].key, "spawnflags" ) == 0 )
		    {
					hook->InternalFlags = nSpawnValue = atoi( self->epair[i].value );
				
					//Get the particle Type
					if( PAR_SIMPLE & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_SIMPLE;
					}
					else if( PAR_CP3 & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_CP3;
					}
					else if( PAR_RAIN & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_RAIN;
					}
					else if( PAR_SMOKE & nSpawnValue)					
					{
						hook->Particle_Type = PARTICLE_SMOKE;
					}
					else if( PAR_CP1 & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_CP1;
					}
					else if( PAR_CP2 & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_CP2;
					}
					else if( PAR_CP4 & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_CP4;
					}
					else if( PAR_BUBBLE & nSpawnValue)
					{
						hook->Particle_Type = PARTICLE_BUBBLE;
					}
					///////////////////////////
					// Get all the other flags
					if( EMIT_TOGGLE & nSpawnValue)
					{
						hook->Toggle_Emission = 1;
					}
					if( EMIT_RANDOM & nSpawnValue)
					{
						hook->Random_Emit = 1;
					}
			}
			i++;
	    }		

		if(hook->Particle_Type == 0)
		{
			hook->Particle_Type = PARTICLE_SIMPLE;
		}
		//Same goes for Number of Particles
		if(hook->NumParticles > 10)
		{
			hook->NumParticles = 10;
		}
		else if(hook->NumParticles < 1)
		{
			hook->NumParticles = 1;
		}
		
		//And yes, again, same goes for velocity
		if(hook->Velocity > 1000)
		{
			hook->Velocity = 1000;
		}
		else if(hook->Velocity < 1)
		{
			hook->Velocity = 1;
		}
		//Last thing we do!
		if( TOGGLE_OFF & nSpawnValue)
		{
			self->use = SFX2_Use;
			self->think = NULL;
			self->nextthink = -1;
		}
		else
		{
			self->think = SFX_Spawn;
			//If we are a toggle type and we are  supposed to be on
			//then set our use function and also set the on-off setting to 1;
			if(TOGGLE_ONOFF & nSpawnValue)
			{
				self->use = SFX2_Use;
				hook->onoff = 1;
			}
			self->nextthink = gstate->time + 0.5 + rnd();
		}
	}
	else  // No epairs... something is fucked... let's get rid of ourselves.
	{
		self->remove(self);
		AI_Dprintf("Removed SFX2 Particle due to lack of any Epair information!\n");
	}

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	if( sv_violence->value != 0)
#endif
	{
		if( ( hook->Particle_Type >= PARTICLE_BLOOD ) && ( hook->Particle_Type <= PARTICLE_BLOOD5 ) )
		{
			self->remove( self );
		}
	}
}


// ----------------------------------------------------------------------------
// N.S.S.
// Name:        sfx_particles
// Description:  Particle Generator
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sfx_complex_particle(userEntity_t *self)
{
	CVector	temp, ang;
	
	self->userHook = gstate->X_Malloc(sizeof(sfx_particle_hook),MEM_TAG_COM_MISC);

	self->className     = "sfx_complex_particle";

	self->s.modelindex  = gstate->ModelIndex( "models/global/e_flred.sp2" );
	self->s.render_scale.Set(.01f, .01f, .01f);
	
	self->s.alpha = 0.01f;

//	self->flags |= FL_NOSAVE;

	self->save = sfx_particle_hook_save;
	self->load = sfx_particle_hook_load;

	self->movetype  = MOVETYPE_NONE;

	self->s.renderfx = RF_TRACKENT|SPR_ALPHACHANNEL;
	
	self->solid     = SOLID_NOT;

    self->think     = SFX2_ParseEpairs;
	self->nextthink = gstate->time + 1.75 + (rnd()*1.5);

	self->velocity.Zero();

	gstate->LinkEntity( self );

	self->flags     &= ~FL_MONSTER;
	self->flags     &= ~FL_BOT;

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_SFX_ENTITIES_register_func()
{
	gstate->RegisterFunc("SFX_Spawn",SFX_Spawn);
	gstate->RegisterFunc("SFX2_ParseEpairs",SFX2_ParseEpairs);
	gstate->RegisterFunc("SFX2_Use",SFX2_Use);
	gstate->RegisterFunc("SFX2_Think",SFX2_Think);
	gstate->RegisterFunc("sfx_particle_hook_save",sfx_particle_hook_save);
	gstate->RegisterFunc("sfx_particle_hook_load",sfx_particle_hook_load);
}
