// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"
//#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_priest(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		monster_priest
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_priest( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_PRIEST );

	self->className		= "monster_priest";
	self->netname		= tongue_monsters[T_MONSTER_PRIEST];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
	ai_register_sounds(self);

	gstate->SetSize (self, -15.0, -15.0, 0.0, 15.0, 15.0, 57.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 180, 90 );

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_speed		= 160;
	hook->pain_chance		= 20;

	self->think				= AI_TaskThink;
	self->nextthink			= gstate->time + 0.2;

	//////////////////////////////////////////////
	//////////////////////////////////////////////

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

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

