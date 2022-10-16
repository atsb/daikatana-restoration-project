// ==========================================================================
//
//  File:wisp.cpp
//  Contents: This is a modified version of the firefly.
//  Author:NOEL STEPHENS
//
// ==========================================================================
#include	"world.h"
#include	"ai.h"
#include	"ai_utils.h"
//#include	"ai_move.h"// SCG[1/23/00]: not used
#include	"ai_frames.h"
#include	"ai_func.h"
//#include	"MonsterSound.h"// SCG[1/23/00]: not used
#include	"WyndraxandWispShare.h"

/* ***************************** define types ****************************** */






/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */

#define	WISPHOOK(x) ((int)&(((wispHook_t *)0)->x))
field_t wisp_hook_fields[] = 
{
	{"sinofs",			WISPHOOK(sinofs),			F_INT},
	{"cosofs",			WISPHOOK(cosofs),			F_INT},
	{"ToORFrom",		WISPHOOK(ToORFrom),			F_INT},
	{"AlphaUpDown",		WISPHOOK(AlphaUpDown),		F_INT},
	{"Landing",			WISPHOOK(Landing),			F_INT},
	{"AlphaBlending",	WISPHOOK(AlphaBlending),	F_INT},
	{"AlphaCount",		WISPHOOK(AlphaCount),		F_INT},
	{"mode",			WISPHOOK(mode),				F_INT},
	{"Personality",		WISPHOOK(Personality),		F_FLOAT},
	{"SpawnTime",		WISPHOOK(SpawnTime),		F_FLOAT},
	{"Dir",				WISPHOOK(Dir),				F_VECTOR},
	{"Last_Origin",		WISPHOOK(Last_Origin),		F_VECTOR},
	{"Goal",			WISPHOOK(Goal),				F_VECTOR},
	{NULL, 0, F_INT}
};

#define	WISPMASTERHOOK(x) (int)&(((wispMasterHook_t *)0)->x)
field_t wisp_master_hook_fields[] = 
{
	{"NumFlies",	WISPMASTERHOOK(NumFlies),		F_INT},
	{"Max",			WISPMASTERHOOK(Max),			F_INT},
	{"Velocity",	WISPMASTERHOOK(Velocity),		F_INT},
	{"Mode",		WISPMASTERHOOK(Mode),			F_INT},
	{"CurrentWisp",	WISPMASTERHOOK(CurrentWisp),	F_INT},
	{"ActiveWisps",	WISPMASTERHOOK(ActiveWisps),	F_INT},
	{"Scale",		WISPMASTERHOOK(Scale),			F_FLOAT},
	{"Delta_Alpha",	WISPMASTERHOOK(Delta_Alpha),	F_FLOAT},
	{"Alpha_Level",	WISPMASTERHOOK(Alpha_Level),	F_FLOAT},
	{"Model",		WISPMASTERHOOK(Model),			F_IGNORE},
	{"NewGoal",		WISPMASTERHOOK(NewGoal),		F_VECTOR},
	{"FireFlies",	WISPMASTERHOOK(FireFlies),		F_EDICT},

	{NULL, 0, F_INT}
};


/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_wisp( userEntity_t *self );

void wisp_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, wisp_hook_fields, sizeof( wispHook_t ) );
}

void wisp_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, wisp_hook_fields, sizeof( wispHook_t ) );
}

void wisp_master_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, wisp_master_hook_fields, sizeof( wispMasterHook_t ) );
}

void wisp_master_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, wisp_master_hook_fields, sizeof( wispMasterHook_t ) );
}

// ----------------------------------------------------------------------------
//
// Name:        WISP_Think
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_Monitor( userEntity_t *self)
{
	//Get the hook for the master
	wispMasterHook_t *mhook = (wispMasterHook_t *)self->userHook;
	userEntity_t *wisp;
	wispHook_t *whook;
	if(!mhook)
	{
		AI_Dprintf("Major problems!!! Wisp master has no HOOK!\n");
		self->nextthink = gstate->time + 0.1;
		return;
	}
	switch(mhook->Mode)
	{
		case WAITING:
			{
				//Wyndrax wants some wisps!
				if(self->enemy != NULL)
				{
					if(_stricmp(self->enemy->className,"monster_wyndrax")==0)
					{
						//Set the new Goal
						mhook->NewGoal = self->enemy->s.origin;
						
						//Look through wisps
						for(int i=0; i< mhook->NumFlies;i++)
						{
							//Get wisp 
							wisp  = (userEntity_t *)mhook->FireFlies[i];
							//Get wisp hook
							whook = (wispHook_t *)wisp->userHook;
							//Make sure this wisp is in wander mode
							if(whook)
							{
								if(whook->mode == WANDER)
								{
									whook->mode			= NEWGOAL|COLLECT;
									mhook->Mode			= SENDING;
									mhook->CurrentWisp	= i;
									break;
								}
							}
							else
							{
								mhook->NumFlies--;
							}
						}
					}
					
				}
				break;
			}
		case SENDING:
			{
				//Get wisp 
				wisp  = (userEntity_t *)mhook->FireFlies[mhook->CurrentWisp];
				//Get wisp hook
				whook = (wispHook_t *)wisp->userHook;

				if(whook->mode & DEAD)
				{
					AI_Dprintf("Wisp Consumed!\n");
					//Get Wyndrax's hook
					if(AI_IsAlive(self->enemy))
					{
						playerHook_t *hook = AI_GetPlayerHook( self->enemy );
						char Buffer[32];
//						sprintf(Buffer,"e3/m_wwispsuck%c.wav",'a'+(rnd()*3));
						Com_sprintf(Buffer,sizeof(Buffer),"e3/m_wwispsuck%c.wav",'a'+(rnd()*3));
						gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxsightb.wav"), 0.70f, 400.0f, 800.0f );
						hook->acro_boost ++;
					}
					else
					{
						AI_Dprintf("###Missed a wisp count due to timing issues!!###\n");
					}
					mhook->CurrentWisp = 0;
					mhook->Mode = WAITING;
				}
			}
	}
	
	if(mhook->ActiveWisps > 0)
	{
		if(self->delay < gstate->time)
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/e_wisploopa.wav"),0.08f, 100.0f, 900.0f );
			self->delay = gstate->time + (3.5f + rnd()*1.75f);
			if(rnd() > 0.65)
			{
				char Buffer[20];
				int Number = 1 + rnd()*5;
//				sprintf(Buffer,"e3/lostsole%d.wav",Number);
				Com_sprintf(Buffer,sizeof(Buffer),"e3/lostsole%d.wav",Number);
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(Buffer),0.30f, 300.0f, 600.0f );
			}
		}
	}
	//for now the main entity does nothing but hangs out
	self->nextthink = gstate->time + 0.1;
}


// ----------------------------------------------------------------------------
//
// Name:        WISP_Personality_Generator
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float WISP_Personality_Generator(float P)
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
// Name:        WISP_SineMove
// Description: This does the nifty movement for the wisp
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_SineMove(userEntity_t *self)
{
    wispHook_t *hook=(wispHook_t *)self->userHook;
	wispMasterHook_t  *mhook=(wispMasterHook_t *)self->owner->userHook;

	//Base everything off of the personality of the wisp
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

}

// ----------------------------------------------------------------------------
//
// Name:        WISP_AlphaBlend
// Description: Fluctuates the Wisp's Alpha blending
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_AlphaBlend(userEntity_t *self)
{
    wispHook_t *hook=(wispHook_t *)self->userHook;
	wispMasterHook_t  *mhook=(wispMasterHook_t *)self->owner->userHook;

	//Handle Alpha blending back and fourth...up and down... wow...neat
	
	hook->AlphaCount ++;
	
	if(mhook->Delta_Alpha != 0.0f)
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
}


// ----------------------------------------------------------------------------
//
// Name:        WISP_BackToGoalSet
// Description: This will determine how the wisp gets back to the Goal
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_Away(userEntity_t *self)
{

    wispHook_t *hook=(wispHook_t *)self->userHook;
	wispMasterHook_t  *mhook=(wispMasterHook_t *)self->owner->userHook;

	
	int Mode = hook->mode & STRIP_MASK;
	
	switch (Mode)
	{
		case WANDER:
			{
				
				//Do blending stuff
				WISP_AlphaBlend(self);

				//Add nifty Sine/Cos Movement
				WISP_SineMove(self);

				if( VectorDistance( self->s.origin, hook->Goal ) >= mhook->Max)
				{

					//Keep our Course towards the Goal
					hook->Dir = hook->Goal - self->s.origin;
					hook->Dir.Normalize();
					hook->Dir = hook->Dir * mhook->Velocity;
					hook->ToORFrom = 0;
					hook->Landing = 0;
					//Let's make them real random and purtttty!
					//Change their personality every once in awhile
					hook->Personality   = WISP_Personality_Generator(hook->Personality);
				}

				
				break;
			}
		case COLLECT:
			{
				//Go collect on Wyndrax's staff
				hook->Dir = mhook->NewGoal - self->s.origin;
				hook->Dir.Normalize();
				hook->Dir = hook->Dir * mhook->Velocity;
				hook->ToORFrom = 0;
				hook->Landing = 0;
				hook->Goal = mhook->NewGoal;
				hook->Personality   = WISP_Personality_Generator(hook->Personality);
				break;

			}
		case DEAD:
			{
				//Only thing we do here is wait for time to expire
				self->velocity.Zero();
				self->s.alpha = 0.00f;
				self->s.origin = self->owner->s.origin;
				if(hook->SpawnTime < gstate->time)
				{
					self->s.alpha = mhook->Alpha_Level;
					hook->mode = WANDER;
					hook->Goal = self->owner->s.origin;
					hook->ToORFrom = 0;
					mhook->ActiveWisps ++;					//Add ourselves back into the active wisp count
				}
				break;
				
			}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        WISP_Think
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_BackToGoal( userEntity_t *self)
{
    wispHook_t *hook=(wispHook_t *)self->userHook;
	wispMasterHook_t  *mhook=(wispMasterHook_t *)self->owner->userHook;



	int MAX_DIS = 20;
	int Mode = hook->mode & STRIP_MASK;
	float Distance;

	if(Mode == COLLECT)
	{
		MAX_DIS = 50;
	}

	if(Mode != DEAD)
	{
		//Do blending stuff
		WISP_AlphaBlend(self);

		//Add nifty Sine/Cos Movement
		WISP_SineMove(self);	
	}
	Distance = VectorDistance( self->s.origin, hook->Goal );
	if(Distance <= MAX_DIS)
	{
		hook->ToORFrom = 1;
		switch(Mode)
		{
			case WANDER:
			{
				hook->Dir.Set(crand(),crand(),crand());
				hook->Dir = hook->Dir * mhook->Velocity;
				//Let's make them real random and purtttty!
				//Change their personality every once in awhile
				hook->Personality   = WISP_Personality_Generator(hook->Personality);
				break;
			
			}
			case COLLECT:
			{
				//We have reached the Staff
				//Let's go away  (better cool FX code goes here too...after I get the whole thing working)
				CVector SmokeandShit = self->s.origin;
				// Smoke FX
				CVector color;
				color.Set( 0.35f, 0.35f, 0.85f );
				SmokeandShit.x +=4;
				gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 6, color, 7, PARTICLE_BLUE_SPARKS|PARTICLE_RANDOM_SPREAD);
				SmokeandShit = self->s.origin;
				SmokeandShit.y +=4;
				gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 4, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
				SmokeandShit = self->s.origin;
				SmokeandShit.x -=4;
				gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 8, color, 7, PARTICLE_BLUE_SPARKS|PARTICLE_RANDOM_SPREAD);
				SmokeandShit = self->s.origin;
				SmokeandShit.y -=4;
				gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 5, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
				self->s.alpha = 0.00f;
				self->s.origin = self->owner->s.origin;
				self->velocity.Zero();
				hook->mode = DEAD;
				hook->SpawnTime = gstate->time + 100.0f; //let's stay away for at least 300 seconds(needs to be a value that mappers can set for gameplay issues)
				hook->ToORFrom = 1;						 //Set it to the other function to check the time it needs to wait.
				mhook->ActiveWisps --;					 //remove ourselves from the active wisp count
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        WISP_Think
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_Think( userEntity_t *self)
{
    wispHook_t *hook=(wispHook_t *)self->userHook;
	wispMasterHook_t  *mhook=(wispMasterHook_t *)self->owner->userHook;
    
	//Make sure we have a master!  If not this means a massacre has occured dont do a damned thing
	if(mhook)
	{
		CVector vec,Dir,Destination;
//		int GoBack = 0;// SCG[1/23/00]: not used
	
		self->velocity = hook->Dir;

		//Check to see if we have landed
		if(hook->Last_Origin == self->s.origin)
		{
			hook->Landing++;
		}
		//Keep track of where we were
		hook->Last_Origin = self->s.origin;


		//Simple check to make sure we get back to the goal
		float Dis = VectorDistance( self->s.origin, hook->Goal );
		if( Dis >= (mhook->Max+100) || hook->Landing >= (10*hook->Personality) || (hook->mode & NEWGOAL))
		{
			hook->mode &= STRIP_MASK;		//Reset the command... we gotit;
			WISP_Away( self );				//Go away from the curent Goal or Set a new one
			
		}	
		if(hook->ToORFrom == 0)//Go to the Current Goal(within 10 units)
		{
			WISP_BackToGoal( self );
		}
		else
		{
			WISP_Away( self );//Go away from the Goal
		}
		
	}
	else
	{
		self->remove(self);
	}

	self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:        WISP_InitThink
// Description: This will setup the wisp for the first time
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_InitThink( userEntity_t *self)
{

    wispHook_t *hook=(wispHook_t *)self->userHook;
	hook->sinofs		= 0;		
	hook->cosofs		= 0;
	hook->ToORFrom		= 0;		//0 is away 1 is back
	hook->AlphaUpDown	= 0;		//0 is down 1 is up(decrement/increment)
	
	hook->Last_Origin	= self->s.origin;		//set our origin
	hook->Landing		= 0;					//reset landing counter
	hook->Personality   = rnd();				//Generate a random personality

	hook->AlphaBlending = 5*hook->Personality;	//Number of frames to wait for change in Alpha
	hook->AlphaCount	= 0;					//Counter of frames

	hook->mode			= WANDER;				//set intial state to wander

	hook->Goal			= self->owner->s.origin; //set the master as the goal focus point
	
	self->think = WISP_Think;					//We are ready to start thinking
	self->nextthink = gstate->time + 0.1;		//Next server frame let's think!
}


// ----------------------------------------------------------------------------
//
// Name:        WISP_Spawn
// Description: The master wisp god spawns his little wisps into thy world!
// Input:		userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WISP_Spawn( userEntity_t *self)
{
	userEntity_t *temp;
	//Get the hook for the master
	wispMasterHook_t *mhook = (wispMasterHook_t *)self->userHook;
	int i;
	
	for(i=0;i < mhook->NumFlies; i++)
	{
		temp = gstate->SpawnEntity();
		
		temp->s.modelindex  = gstate->ModelIndex( mhook->Model );
		
		temp->movetype  = MOVETYPE_FLY;

		temp->s.frame	= 0;
		
		temp->s.renderfx = SPR_ALPHACHANNEL;

		temp->clipmask	= MASK_DEADSOLID;
		
		temp->s.render_scale.Set(mhook->Scale, mhook->Scale, mhook->Scale);	
		
		temp->s.alpha = mhook->Alpha_Level; //Alway have a little bit of translucent value
		
		temp->solid     = SOLID_NOT;
		temp->flags     &= ~FL_MONSTER;
		temp->flags		&= ~FL_BOT;

		//Make sure we set the owner so we have an origin to start from
		temp->owner = self;

		temp->s.origin.x = self->s.origin.x +(rnd()*5.0);
		temp->s.origin.y = self->s.origin.y +(rnd()*5.0);
		temp->s.origin.z = self->s.origin.z +(rnd()*5.0);

		temp->nextthink = gstate->time + 0.1;

		temp->userHook = gstate->X_Malloc(sizeof(wispHook_t),MEM_TAG_HOOK);
		temp->save = wisp_hook_save;
		temp->load = wisp_hook_load;

//		wispHook_t *hook = (wispHook_t *)self->userHook;// SCG[1/23/00]: not used

		
		temp->think     = WISP_InitThink;
		
		temp->nextthink = gstate->time + 0.1 + rnd () * 0.5;
		
		gstate->SetSize(temp, -1, -1, -1, 1, 1, 1);

		gstate->SetOrigin2( temp, self->s.origin );
		
		gstate->LinkEntity( temp );

		mhook->FireFlies[i] = temp;
		//Attach the Sprite Light&Glow stuff (add track entity)
		trackInfo_t tinfo;

		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent=temp;
		tinfo.srcent=temp;
		tinfo.fru.Zero();
		tinfo.lightColor.x = 0.35;//R
		tinfo.lightColor.y = 0.35;//G
		tinfo.lightColor.z = 0.75;//B
		tinfo.lightSize= 135;
		
		tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX;
		tinfo.fxflags = TEF_LIGHT;

		com->trackEntity(&tinfo,MULTICAST_PVS);

		mhook->ActiveWisps++; //Add ourselves to the active list

	}
	//Now we are done spawning, let's monitor the activity
	self->think = WISP_Monitor;
	self->nextthink = gstate->time + 0.8;

}

// ----------------------------------------------------------------------------
//
// Name:		WISP_ParseEpairs
// Description:Get all initialization stuff here
// Input:
// Output:Nothing
// Note:General setting up for the fire flies
//
// ----------------------------------------------------------------------------
void WISP_ParseEpairs( userEntity_t *self )
{
	int dummy_int;
	float dummy_float;
	//Get the hook
	wispMasterHook_t *hook = (wispMasterHook_t *)self->userHook;
	
	
    //Setup default values
	hook->NumFlies		= 3;
	hook->Max			= 75;
	hook->Velocity		= 35;
	hook->Scale			= 1.00f;
	hook->Alpha_Level	= 0.35;
	hook->Delta_Alpha	= 0.0f;
	hook->ActiveWisps	= 0;
	//Wisp set the wisp model
//	sprintf(hook->Model,"models/global/e_sflblue.sp2");
	Com_sprintf(hook->Model,sizeof(hook->Model),"models/global/e_sflblue.sp2");
	
	
	
	//Parse through the key values to get the number of wisps and other things
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
			else if( _stricmp(self->epair[i].key, "delta_alpha") == 0)
			{
				dummy_int   = atoi(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_int <=0)
				{
					hook->Delta_Alpha = 0.0f;
				}
				else
				{
					hook->Delta_Alpha = (float) dummy_int/100.0f;
				}
			}
			else if( _stricmp(self->epair[i].key, "alpha_level") == 0)
			{
				dummy_int   = atoi(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_int <=0)
				{
					hook->Alpha_Level = 0.01f;
				}
				else
				{
					hook->Alpha_Level = (float) dummy_int/100.0f;
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


	
	self->think = WISP_Spawn;
	self->nextthink = gstate->time + 0.1;
}



// ----------------------------------------------------------------------------
//
// Name:        monster_wisp
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_wisp( userEntity_t *self )
{
	CVector	temp, ang;
	
	self->userHook = gstate->X_Malloc(sizeof(wispMasterHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	self->save = wisp_master_hook_save;
	self->load = wisp_master_hook_load;

	self->className     = "monster_wisp";// Must be the same name as in the AIDATA.CSV file <nss>
	self->netname       = tongue_monsters[T_MONSTER_WISP];

	AIATTRIBUTE_GetCSVFileName( self->className );
	//self->s.modelindex  = gstate->ModelIndex( "models/global/e_flred.sp2" );
	//self->s.render_scale.Set(.40f, .40f, .40f);

	self->movetype  = MOVETYPE_HOVER;

	self->s.renderfx = SPR_ALPHACHANNEL;
	
	self->solid     = SOLID_NOT;
	self->flags     &= ~FL_MONSTER;
	self->flags		&= ~FL_BOT;

    self->think     = WISP_ParseEpairs;
	self->nextthink = gstate->time + 0.1;

	self->velocity.Zero();

	self->enemy = NULL;
	
	self->delay = gstate->time + 2.5f + rnd()*1.75f;

	gstate->LinkEntity( self );

}



///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_wisp_register_func()
{
	gstate->RegisterFunc("WISP_Think",WISP_Think);
	gstate->RegisterFunc("WISP_InitThink",WISP_InitThink);
	gstate->RegisterFunc("WISP_Monitor",WISP_Monitor);
	gstate->RegisterFunc("WISP_Spawn",WISP_Spawn);
	gstate->RegisterFunc("WISP_ParseEpairs",WISP_ParseEpairs);

	gstate->RegisterFunc( "wisp_hook_save", wisp_hook_save );
	gstate->RegisterFunc( "wisp_hook_load", wisp_hook_load );
	gstate->RegisterFunc( "wisp_master_hook_save", wisp_master_hook_save );
	gstate->RegisterFunc( "wisp_master_hook_load", wisp_master_hook_load );
}

