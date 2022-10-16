// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "ai_func.h"
#include "MonsterSound.h"


/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void fish_guppy1( userEntity_t *self );
DllExport void e_guppy( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////
//	goldfish_turn
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:        guppyone_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void guppyone_pain( userEntity_t *self, userEntity_t *other, float kick, int damage)
{

}

// ----------------------------------------------------------------------------
//
// Name:        guppyone_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void guppyone_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point )
{
	self->deadflag = DEAD_DEAD;
	self->nextthink = -1;
	self->prethink = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        fish_guppy1
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fish_guppy1( userEntity_t *self )
{
	CVector	temp, ang;
	
//	playerHook_t *hook  = AI_InitMonster(self, TYPE_GOLDFISH);
	AI_InitMonster(self, TYPE_GOLDFISH);// SCG[1/23/00]: hook not used

	self->className     = "e_guppy";// Must be the same name as in the AIDATA.CSV file <nss>
	self->netname       = tongue_monsters[T_MONSTER_GOLDFISH];

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

	self->movetype  = MOVETYPE_SWIM;
	
	self->solid     = SOLID_BBOX;
	self->flags     &= ~FL_MONSTER;
	self->flags     |= FL_NOSAVE;
	self->svflags |= SVF_DEADMONSTER;

    self->think     = FISH_ParseEpairs;
	self->nextthink = gstate->time + 0.1 + rnd () * 0.5;

	//Need this bad boy...
	AI_DetermineMovingEnvironment( self );

	self->fragtype  = FRAGTYPE_ALWAYSGIB;
	self->ang_speed.Set( 2, 2, 2 );	
	gstate->LinkEntity( self );
}


void e_guppy( userEntity_t *self )
{
	fish_guppy1( self );
}