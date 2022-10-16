// ==========================================================================
//
//  File:seagull.cpp
//  Contents:
//  Author:N.S.S.
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
#include "ai_move.h"
#include "ai_frames.h"
#include "ai_func.h"
#include "MonsterSound.h"


/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
extern float sin_tbl[];
/* **************************** Global Functions *************************** */
void AI_ParseEpairs2( userEntity_t *self );
/* ******************************* exports ********************************* */

DllExport void e_seagull( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////
//	Seagull
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		SEAGULL_Think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SEAGULL_Think(userEntity_t *self)
{
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_TaskThink( self);
 	
	//Add a sin wave to its flight	
	if(hook->acro_boost_time > 12.0f)
	{	
		hook->acro_boost_time = 0.0f;
	}
	self->velocity.z += 12.5*(sin_tbl[(int)hook->acro_boost_time]);
	hook->acro_boost_time +=1.25;
}



// ----------------------------------------------------------------------------
//
// Name:		SEAGULL_FlyTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SEAGULL_FlyTowardPoint( userEntity_t *self, CVector &Destination)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	self->think = SEAGULL_Think;
	//Setup turn rate
	pAIData->fValue = 0.01f;

	//Setup destination
	pAIData->destPoint = Destination;

	//Fly there
	AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue); 

	//AI_Dprintf("sY:%f  sR:%f\n",self->s.angles.yaw,self->s.angles.roll);
	
	return (1);
}


// ----------------------------------------------------------------------------
//
// Name:        seagull
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void e_seagull( userEntity_t *self )
{
	CVector	temp, ang;
	
//	playerHook_t *hook  = AI_InitMonster(self, TYPE_SEAGULL);
	AI_InitMonster(self, TYPE_SEAGULL);// SCG[1/23/00]: hook not used

	self->className     = "e_seagull";// Must be the same name as in the AIDATA.CSV file <nss>
	self->netname       = tongue_monsters[T_MONSTER_SEAGULL];

	char *szModelName   = AIATTRIBUTE_GetModelName( self->className );
	_ASSERTE( szModelName );
	self->s.modelindex  = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        if ( szCSVFileName )
        {
            FRAMEDATA_ReadFile( szCSVFileName, self );
        }
	}

	ai_register_sounds(self);

    //Get attributes from the CSV file
	AIATTRIBUTE_SetInfo( self );



	//Need this bad boy...
	AI_DetermineMovingEnvironment( self );

	self->movetype  = MOVETYPE_FLY;
	
	self->solid     = SOLID_BBOX;
	self->flags     &= ~FL_MONSTER;
	self->flags     |= FL_NOSAVE;

	self->fragtype  = FRAGTYPE_ALWAYSGIB;

    self->think     = AI_ParseEpairs2;
	self->nextthink = gstate->time + 0.1f;

	//hook->run_speed = 200.0f;

	gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_seagull_register_func()
{
	gstate->RegisterFunc("SEAGULL_Think",SEAGULL_Think);
	// SCG[2/2/00]: Redundant.  This function is registered in func2.cpp
//	gstate->RegisterFunc("AI_ParseEpairs2",AI_ParseEpairs2);
}
