#include "world.h"
#include "ai_utils.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"cine_toshiro"

DllExport void cine_toshiro( userEntity_t *self );

void cine_toshiro( userEntity_t *self )
{

	playerHook_t *hook	= AI_InitMonster(self, TYPE_CINE_ONLY);

	self->className		= MONSTERNAME;

//	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
	char szModelName[64] = "models/cinematic/c_tosh_";

	strncat( szModelName, gstate->mapName, 4 );
	strncat( szModelName, ".dkm", 4 );

    self->s.modelindex = gstate->ModelIndex( szModelName );

	ai_get_sequences(self);

	self->s.mins.Set( -12, -12, -24 );
	self->s.maxs.Set( 12, 12, 30 );

/*
	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
*/
	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////
	
	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->think_time		= THINK_TIME;
	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;
	self->flags				|= FL_BOT;

//	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	gstate->LinkEntity(self);
}

