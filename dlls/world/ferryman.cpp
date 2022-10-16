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
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
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

DllExport void monster_ferryman(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		monster_ferryman
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_ferryman( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_FERRYMAN );

	self->className		= "monster_ferryman";
	self->netname		= tongue_monsters[T_MONSTER_FERRYMAN];

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

	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

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

	self->takedamage		= DAMAGE_NO;
	self->think				= AI_TaskThink;
	self->nextthink			= gstate->time + 0.2;


	//////////////////////////////////////////////
	//////////////////////////////////////////////

//	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	AIATTRIBUTE_SetInfo( self );// SCG[1/23/00]: pWeaponAttributes not used

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

