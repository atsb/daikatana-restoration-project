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

DllExport void goldfish( userEntity_t *self );//the old entity
DllExport void fish_goldfish( userEntity_t *self );
DllExport void e_goldfish( userEntity_t *self );


///////////////////////////////////////////////////////////////////////////////
//	goldfish_turn
///////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:        fish_goldfish
// Description: Standard init code... most code above is not used, I am keeping it
// just for reference until the fish are 100% done.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fish_goldfish( userEntity_t *self )
{
	CVector	temp, ang;
	
	AI_InitMonster(self, TYPE_GOLDFISH);// SCG[1/23/00]: hook not used

	self->className     = "e_goldfish";// Must be the same name as in the AIDATA.CSV file <nss>
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
	self->prethink  = FISH_Update;
	self->nextthink = gstate->time + 0.1 + rnd () * 0.5;

	//Need this bad boy...
	//Not only does it set the moving environment, but it also sets up node paths.. 
	AI_DetermineMovingEnvironment( self );
	self->fragtype  = FRAGTYPE_ALWAYSGIB;
	self->ang_speed.Set( 2, 2, 2 );	
	gstate->LinkEntity( self );
}

// ----------------------------------------------------------------------------
//
// Name:        goldfish
// Description:Old function bc there is still an entity type called goldfish
// left here to just make sure... in case it doesn't get taken out of rad.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void goldfish( userEntity_t *self )
{
	fish_goldfish( self );
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

void e_goldfish( userEntity_t *self )
{
	fish_goldfish( self );
}
