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

DllExport void fish_grayfish( userEntity_t *self );
DllExport void e_grayfish( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////
//	goldfish_turn
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:        goldfish_init_frametable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void greyfish_init_frametable( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

//	frameData_t *pSequence = FRAMES_GetSequence( self, "swima" );
	FRAMES_GetSequence( self, "swima" );// SCG[1/23/00]: pSequence not used
	//ai_set_sequence_data (self, pSequence, 1, NULL, 6, NULL, 0, -1, 0, -1, FRAME_LOOP);
}



// ----------------------------------------------------------------------------
//
// Name:        goldfish_I_am_your_master_now
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void greyfish_I_am_your_master_now( userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	userEntity_t	*head;

	self->targetname = self->target;
	self->target = NULL;
//	hook->head = NULL;

	//	make all the other fishes who were following the master
	//	follow this one
	for (head = gstate->FirstEntity (); head; head = gstate->NextEntity (head))
	{
		if (head->className && !stricmp (head->className, "goldfish"))
		{
			if (head->target && !stricmp (head->target, self->targetname))
			{
				hook = AI_GetPlayerHook( head );
//				hook->head = self;
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        greyfish_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void greyfish_pain( userEntity_t *self, userEntity_t *other, float kick, int damage)
{

}

// ----------------------------------------------------------------------------
//
// Name:        goldfish_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void greyfish_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point )
{
	self->deadflag = DEAD_DEAD;
	self->nextthink = -1;
	self->prethink = NULL;
}


// ----------------------------------------------------------------------------
//
// Name:        fish_grayfish
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fish_grayfish( userEntity_t *self )
{
	CVector	temp, ang;
	
//	playerHook_t *hook  = AI_InitMonster(self, TYPE_GOLDFISH);
	AI_InitMonster(self, TYPE_GOLDFISH);// SCG[1/23/00]: hook not used

	self->className     = "e_greyfish";// Must be the same name as in the AIDATA.CSV file <nss>
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

void e_grayfish( userEntity_t *self )
{
	fish_grayfish( self );
}
