// ==========================================================================
//
//  File:FIREFLY
//  Contents: The Amazing FireFly Code
//  Author:NOEL STEPHENS
//
// ==========================================================================
#include "world.h"
#include "ai.h"
//#include "ai_utils.h"		// SCG[1/23/00]: not used
//#include "ai_move.h"		// SCG[1/23/00]: not used
#include "ai_frames.h"
//#include "ai_func.h"		// SCG[1/23/00]: not used	
//#include "MonsterSound.h"	// SCG[1/23/00]: not used


/* ***************************** define types ****************************** */


#define MAX_FIREFLY				10


#define FLY_ROUND				0x00000000
#define FLY_4PRONG				0x00000001
#define FLY_4PRONGO				0x00000002
#define FLY_4PRONGX				0x00000004
#define FLY_4PRONGXO			0x00000008
#define FLY_8PRONGP				0x00000010
#define FLY_8PRONGPO			0x00000020
#define FLY_O					0x00000040

/* ***************************** Local Variables *************************** */
typedef struct
{
    int NumFlies;								// # of fireflies to spawn and fuck with
	int Max;									// Maximum distance for firefly to fly to
	int Velocity;								// Velocity for the FireFly
	float Scale;								// Scale for model
	float Delta_Alpha;							// Frequency of Alpha change
	float Alpha_Level;							// Initial Alpha Level
	char Model[100];							// Model/Sprite to use.
	CVector Color;								// Fly's Color
	CVector Color2;								// Color to interpolate towards
    userEntity_t *FireFlies[MAX_FIREFLY];		// list of firefly ents
} fireflyMasterHook_t;

typedef struct
{
	int sinofs;									// sine count offset
	int cosofs;									// cosine count offset
	int ToORFrom;								//Flag to tell me which way they should be going(to origin or away)
	int AlphaUpDown;							//Toggle Alpha blending up and down
	int Landing;								//Fly has decided to land on a FUCKING SOLID SURFACE!
	int AlphaBlending;							//Alpha Blending Personality number calculated from personality
	int AlphaCount;								//Alpha Blending Counter]
	int Color_Dir;								//Direction of Color lerping
	float Color_Lerp;							//Keeps track of color lerping
	float Personality;							//Personality factor
	float Scale_Dir;							//Scale Direction
	CVector Dir;								//Direction to be going in
	CVector Last_Origin;						//Our Last Origin

} fireflyHook_t;

// SCG[11/19/99]: Save Info
#define FIREFLYMASTERHOOK(x) (int)&(((fireflyMasterHook_t *)0)->x)
field_t firefly_master_hook_fields[] = 
{
	{"NumFlies",	FIREFLYMASTERHOOK(NumFlies),	F_INT},
	{"Max",			FIREFLYMASTERHOOK(Max),			F_INT},
	{"Velocity",	FIREFLYMASTERHOOK(Velocity),	F_INT},
	{"Scale",		FIREFLYMASTERHOOK(Scale),		F_FLOAT},
	{"Delta_Alpha",	FIREFLYMASTERHOOK(Delta_Alpha),	F_FLOAT},
	{"Alpha_Level",	FIREFLYMASTERHOOK(Alpha_Level),	F_FLOAT},
	{"Model",		FIREFLYMASTERHOOK(Model),		F_INT},
	{"Color",		FIREFLYMASTERHOOK(Color),		F_VECTOR},
	{"Color2",		FIREFLYMASTERHOOK(Color2),		F_VECTOR},
	{"FireFlies",	FIREFLYMASTERHOOK(FireFlies),	F_EDICT},
	{NULL, 0, F_INT}
};

#define	FIREFLYHOOK(x) (int)&(((fireflyHook_t *)0)->x)
field_t firefly_hook_fields[] = 
{
	{"sinofs",			FIREFLYHOOK(sinofs),			F_INT},
	{"cosofs",			FIREFLYHOOK(cosofs),			F_INT},
	{"ToORFrom",		FIREFLYHOOK(ToORFrom),			F_INT},
	{"AlphaUpDown",		FIREFLYHOOK(AlphaUpDown),		F_INT},
	{"Landing",			FIREFLYHOOK(Landing),			F_INT},
	{"AlphaBlending",	FIREFLYHOOK(AlphaBlending),		F_INT},
	{"AlphaCount",		FIREFLYHOOK(AlphaCount),		F_INT},
	{"Color_Dir",		FIREFLYHOOK(Color_Dir),			F_INT},
	{"Color_Lerp",		FIREFLYHOOK(Color_Lerp),		F_FLOAT},
	{"Personality",		FIREFLYHOOK(Personality),		F_FLOAT},
	{"Scale_Dir",		FIREFLYHOOK(Scale_Dir),			F_FLOAT},
	{"Dir",				FIREFLYHOOK(Dir),				F_VECTOR},
	{"Last_Origin",		FIREFLYHOOK(Last_Origin),		F_VECTOR},
	{NULL, 0, F_INT}
};
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */

extern float sin_tbl[];
extern float cos_tbl[];
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_firefly( userEntity_t *self );

void firefly_master_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, firefly_master_hook_fields, sizeof( fireflyMasterHook_t ) );
}

void firefly_master_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, firefly_master_hook_fields, sizeof( fireflyMasterHook_t ) );
}

void firefly_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, firefly_hook_fields, sizeof( fireflyHook_t ) );
}

void firefly_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, firefly_hook_fields, sizeof( fireflyHook_t ) );
}

// ----------------------------------------------------------------------------
//
// Name:        FIREFLY_Think
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FIREFLY_Monitor( userEntity_t *self)
{
		//Get the hook for the master
//	fireflyMasterHook_t *mhook = (fireflyMasterHook_t *)self->userHook;// SCG[1/23/00]: not referenced
	
	//for now the main entity does nothing but hangs out
	self->nextthink = gstate->time + 0.8;
}


// ----------------------------------------------------------------------------
//
// Name:        FIREFLY_Personality_Generator
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float FIREFLY_Personality_Generator(float P)
{
	//Half of the time these buggers don't want to change
	if(rnd() > 0.50f)
	{
		//But when they do they get a new personality...
		P = rnd();
		if(P < 0.25f)
		{
			P = 0.25f;
		}
	}
	return P;
}


// ----------------------------------------------------------------------------
//
// Name:        FIREFLY_Think
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FIREFLY_Think( userEntity_t *self)
{
    fireflyHook_t *hook=(fireflyHook_t *)self->userHook;
	fireflyMasterHook_t  *mhook=(fireflyMasterHook_t *)self->owner->userHook;
    
	
	CVector vec,Dir,Destination;
//	int GoBack = 0;// SCG[1/23/00]: not referenced
	
	self->velocity = hook->Dir;
	//Handle Alpha blending... wow...neat
	hook->AlphaCount ++;
	if(mhook)
	{
		if(hook->AlphaCount >= hook->AlphaBlending)
		{
			if(hook->AlphaUpDown == 0)
			{
				self->s.alpha -= mhook->Delta_Alpha;
				if(self->s.alpha < 0.01)
				{
					hook->AlphaUpDown	= 1;
					hook->AlphaBlending = 5*hook->Personality;	//Number of frames to wait for change in Alpha
				}
			}
			else
			{
				self->s.alpha += mhook->Delta_Alpha;
				if(self->s.alpha > mhook->Alpha_Level)
				{
					hook->AlphaUpDown	= 0;
					hook->AlphaBlending = 5*hook->Personality;	//Number of frames to wait for change in Alpha
				}
			}
			hook->AlphaCount	= 0;					//Counter
		}
	}
	else
	{
		//our master is dead... let's die too.
		self->remove(self);
		return;
	}

	//Check to see if we have landed
	if(hook->Last_Origin == self->s.origin)
	{
		hook->Landing++;
	}
	//Keep track of where we were
	hook->Last_Origin = self->s.origin;
	
	//Color interpolation stuff starts here
	if(!(mhook->Color2.x == 0.0f && mhook->Color2.y == 0.0f && mhook->Color2.z == 0.0f))
	{
		if(hook->Color_Dir)
		{
			self->s.color = self->s.color.Interpolate( mhook->Color,mhook->Color2,hook->Color_Lerp);
		}	
		else
		{
			self->s.color = self->s.color.Interpolate( mhook->Color2,mhook->Color,hook->Color_Lerp);
		}
		
		//Lerp and directional stuff is done here for color lerping 
		if(hook->Color_Lerp >= 1.0f)
		{
			if(hook->Color_Dir)
				hook->Color_Dir = 0;
			else
				hook->Color_Dir = 1;
			hook->Color_Lerp = (0.25f * hook->Personality);
		}
		else
		{
			hook->Color_Lerp += (0.25f * hook->Personality);
		}
	}


	
	//Simple check to make sure we get back
	if( VectorDistance( self->s.origin, self->owner->s.origin ) >= mhook->Max || hook->Landing >=1 )
	{
		hook->Dir = self->owner->s.origin - self->s.origin;
		hook->Dir.Normalize();
		hook->Dir = hook->Dir * mhook->Velocity;
		hook->ToORFrom = 0;
		hook->Landing = 0;
		//Let's make them real random and purtttty!
		//Change their personality every once in awhile
		hook->Personality   = FIREFLY_Personality_Generator(hook->Personality);
	}	
	if(hook->ToORFrom == 0)
	{
		if(VectorDistance( self->s.origin, self->owner->s.origin ) <= 10)
		{
			hook->Dir.Set(crand(),crand(),crand());
			hook->Dir = hook->Dir * mhook->Velocity;
			hook->ToORFrom = 1;
			//Let's make them real random and purtttty!
			//Change their personality every once in awhile
			hook->Personality   = FIREFLY_Personality_Generator(hook->Personality);
		}
	}
	else
	{
		if( VectorDistance( self->s.origin, self->owner->s.origin ) >= mhook->Max)
		{
			hook->Dir = self->owner->s.origin - self->s.origin;
			hook->Dir.Normalize();
			hook->Dir = hook->Dir * mhook->Velocity;
			hook->ToORFrom = 0;
			//Let's make them real random and purtttty!
			//Change their personality every once in awhile
			hook->Personality   = FIREFLY_Personality_Generator(hook->Personality);
		}
	}

	if(rnd() > hook->Personality)
	{
		self->velocity.y += cos_tbl[hook->sinofs] * (mhook->Velocity/(2*hook->Personality));
	}
	else
	{
		self->velocity.x += sin_tbl[hook->sinofs] * (mhook->Velocity/(2*hook->Personality));
	}
	
	self->velocity.z += sin_tbl [hook->sinofs] * (mhook->Velocity/(2*hook->Personality));
   
    hook->sinofs++;
	hook->cosofs++;
    if ( hook->sinofs > 11 )
        hook->sinofs=0;	

	if ( hook->cosofs > 11 )
        hook->cosofs=0;	

	self->nextthink = gstate->time + 0.1;
}


void FIREFLY_InitThink( userEntity_t *self)
{

    fireflyHook_t *hook=(fireflyHook_t *)self->userHook;
	hook->sinofs		= 0;		
	hook->cosofs		= 0;
	hook->ToORFrom		= 0;		//0 is away 1 is back
	hook->AlphaUpDown	= 0;		//0 is down 1 is up(decrement/increment)
	
	hook->Last_Origin	= self->s.origin;
	hook->Landing		= 0;
	hook->Personality   = rnd();

	hook->AlphaBlending = 5*hook->Personality;	//Number of frames to wait for change in Alpha
	hook->AlphaCount	= 0;					//Counter of frames

	hook->Color_Lerp	= 0.10;					//Start Lerp Fraction
	hook->Color_Dir		= 0;					//From Color to Color2
	
	hook->Scale_Dir		= 0.25f;				//Scale factor
	self->think = FIREFLY_Think;
	self->nextthink = gstate->time + 0.1;
}


// ----------------------------------------------------------------------------
//
// Name:        FIREFLY_Spawn
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FIREFLY_Spawn( userEntity_t *self)
{
	userEntity_t *temp;
	//Get the hook for the master
	fireflyMasterHook_t *mhook = (fireflyMasterHook_t *)self->userHook;
	int i;
	
	for(i=0;i < mhook->NumFlies; i++)
	{
		temp = gstate->SpawnEntity();
		
		temp->className		= "Fly";
		temp->s.modelindex  = gstate->ModelIndex( mhook->Model );
		temp->movetype		= MOVETYPE_FLY;
		temp->s.renderfx	= SPR_ALPHACHANNEL;
		temp->s.frame		= 0;
		temp->clipmask		= MASK_WATER;
		//Set them up at different Random alpha values
		temp->s.alpha		= 0.75; //Alway have a little bit of translucent value
		temp->solid			= SOLID_NOT;
		temp->flags			&= ~FL_MONSTER;
		//Make sure we set the owner so we have an origin to start from
		temp->owner			= self;

		temp->s.origin.x	= self->s.origin.x +(rnd()*5.0);
		temp->s.origin.y	= self->s.origin.y +(rnd()*5.0);
		temp->s.origin.z	= self->s.origin.z +(rnd()*5.0);

		temp->nextthink		= gstate->time + 0.1;

		temp->userHook		= gstate->X_Malloc(sizeof(fireflyHook_t),MEM_TAG_HOOK);

		temp->save = firefly_hook_save;
		temp->load = firefly_hook_load;

		temp->s.render_scale.Set(mhook->Scale, mhook->Scale, mhook->Scale);
		
//		fireflyHook_t *hook = (fireflyHook_t *)self->userHook;	// SCG[1/23/00]: not used
		temp->think			= FIREFLY_InitThink;
		temp->s.color		= mhook->Color;
		temp->nextthink		= gstate->time + 0.1 + rnd () * 0.5;
		
		gstate->SetSize(temp, -1, -1, -1, 1, 1, 1);
		gstate->SetOrigin2( temp, self->s.origin );
		gstate->LinkEntity( temp );
		mhook->FireFlies[i] = temp;

	}
	self->think = FIREFLY_Monitor;
	self->nextthink = gstate->time + 0.8;

}

// ----------------------------------------------------------------------------
//
// Name:		FIREFLY_ParseEpairs
// Description:Get all initialization stuff here
// Input:
// Output:Nothing
// Note:General setting up for the fire flies
//
// ----------------------------------------------------------------------------
void FIREFLY_ParseEpairs( userEntity_t *self )
{
	float dummy_float;
	
	//Get the hook
	fireflyMasterHook_t *hook = (fireflyMasterHook_t *)self->userHook;
	
	
    //Setup default values
	hook->NumFlies		= 4;
	hook->Max			= 75;
	hook->Velocity		= 55;
	hook->Scale			= 0.50f;
	hook->Alpha_Level	= 0.75;
	hook->Delta_Alpha	= 0.0f;
	
	hook->Color.Set(1.0,1.0,1.0);
	hook->Color2.Set(0.0f,0.0f,0.0f);

//	sprintf(hook->Model,"models/global/e_flare.sp2");
	Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare.sp2");
	//Parse through the key values to get the number of fireflies
	//*sigh*... I made this a little more modular than intended... :)
	if (self->epair)
	{
	    int i = 0;
		while ( self->epair[i].key )
	    {
		    //Get the values
			if ( _stricmp(self->epair[i].key, "count") == 0 )
		    {
			    hook->NumFlies = atoi(self->epair[i].value);
		    }
		    else if ( _stricmp(self->epair[i].key, "distance") == 0 )
			{
				hook->Max = atoi(self->epair[i].value);
			}
		    else if ( _stricmp(self->epair[i].key, "velocity") == 0 )
			{
				hook->Velocity = atoi(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "scale") == 0)
			{
				dummy_float   = atof(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_float == 0)
				{
					dummy_float = 1;
				}
				hook->Scale =  dummy_float;
			}
			else if ( _stricmp(self->epair[i].key, "_color") == 0 )
			{
				sscanf(self->epair[i].value,"%f%f%f",&hook->Color.x,&hook->Color.y,&hook->Color.z);
			}
			else if ( _stricmp(self->epair[i].key, "color2") == 0 )
			{
				sscanf(self->epair[i].value,"%f%f%f",&hook->Color2.x,&hook->Color2.y,&hook->Color2.z);
			}
			else if( _stricmp(self->epair[i].key, "delta_alpha") == 0)
			{
				hook->Delta_Alpha = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "alpha_level") == 0)
			{
				hook->Alpha_Level = atof(self->epair[i].value);
			}
			//Set the model for the fly
			else if ( _stricmp( self->epair[i].key, "spawnflags" ) == 0 )
		    {
				int nSpawnValue = atoi( self->epair[i].value );
				
				if(nSpawnValue & FLY_ROUND)
				{
//					sprintf(hook->Model,"models/global/e_flare.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare.sp2");
				}
				else if(nSpawnValue & FLY_4PRONG)
				{
//					sprintf(hook->Model,"models/global/e_flare4+.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare4+.sp2");
				}
				else if(nSpawnValue & FLY_4PRONGO)
				{
//					sprintf(hook->Model,"models/global/e_flare4+o.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare4+o.sp2");
				}
				else if(nSpawnValue & FLY_4PRONGX)
				{
//					sprintf(hook->Model,"models/global/e_flare4x.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare4x.sp2");
				}
				else if(nSpawnValue & FLY_4PRONGXO)
				{
//					sprintf(hook->Model,"models/global/e_flare4xo.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare4x.sp2");
				}
				else if(nSpawnValue & FLY_8PRONGP)
				{
//					sprintf(hook->Model,"models/global/e_flare8+.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare8+.sp2");
				}
				else if(nSpawnValue & FLY_8PRONGPO)
				{
//					sprintf(hook->Model,"models/global/e_flare8+o.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flare8+o.sp2");
				}
				else if(nSpawnValue & FLY_O)
				{
//					sprintf(hook->Model,"models/global/e_flareo.sp2");
					Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_flareo.sp2");
				}
			}
			//Double check to make sure they are valid
			//Make sure we don't put in a crazy Maximum distance
			if(hook->Max > 200)
			{
				hook->Max = 200;
			}
			//Make sure we don't put in a dumb Maximum distance
			if(hook->Max < 20)
			{
				hook->Max = 20;
			}

			//Same goes for Number of Flies
			if(hook->NumFlies > 10)
			{
				hook->NumFlies = 10;
			}
			if(hook->NumFlies < 1)
			{
				hook->NumFlies = 1;
			}
			
			//And yes, again, same goes for velocity
			if(hook->Velocity > 500)
			{
				hook->Velocity = 500;
			}
			else if(hook->Velocity < 1)
			{
				hook->Velocity = 1;
			}
			
			
			i++;
	    }		
	}
	self->think = FIREFLY_Spawn;
	self->nextthink = gstate->time + 0.1;
}



// ----------------------------------------------------------------------------
//<nss>
// Name:        monster_firefly
// Description: Main firefly function when spawning this entity type.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_firefly( userEntity_t *self )
{
	CVector	temp, ang;
	
	self->userHook = gstate->X_Malloc(sizeof(fireflyMasterHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	self->save = firefly_master_hook_save;
	self->load = firefly_master_hook_load;

	self->className     = "monster_firefly";// Must be the same name as in the AIDATA.CSV file <nss>
	self->netname       = tongue_monsters[T_MONSTER_FIREFLY];

	//self->s.modelindex  = gstate->ModelIndex( "models/global/e_flred.sp2" );
	self->s.render_scale.Set(.40f, .40f, .40f);

	self->movetype  = MOVETYPE_FLY;

	self->s.renderfx = SPR_ALPHACHANNEL;
	
	self->solid     = SOLID_NOT;
	self->flags     &= ~FL_MONSTER;

    self->think     = FIREFLY_ParseEpairs;
	self->nextthink = gstate->time + 0.1;

	self->velocity.Zero();

	gstate->LinkEntity( self );

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_firefly_register_func()
{
	gstate->RegisterFunc("FIREFLY_Think",FIREFLY_Think);
	gstate->RegisterFunc("FIREFLY_InitThink",FIREFLY_InitThink);
	gstate->RegisterFunc("FIREFLY_Monitor",FIREFLY_Monitor);
	gstate->RegisterFunc("FIREFLY_Spawn",FIREFLY_Spawn);
	gstate->RegisterFunc("FIREFLY_ParseEpairs",FIREFLY_ParseEpairs);
	gstate->RegisterFunc( "firefly_master_hook_save", firefly_master_hook_save );
	gstate->RegisterFunc( "firefly_master_hook_load", firefly_master_hook_load );
	gstate->RegisterFunc( "firefly_hook_save", firefly_hook_save );
	gstate->RegisterFunc( "firefly_hook_load", firefly_hook_load );
}
