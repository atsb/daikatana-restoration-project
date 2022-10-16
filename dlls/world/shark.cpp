// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"


/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void shark_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_shark( userEntity_t *self );


// ----------------------------------------------------------------------------
// NSS[2/24/00]:
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void shark_set_attack_seq( userEntity_t *self )
{

	frameData_t *pSequence;

	if( rand() % 3 )
		pSequence = FRAMES_GetSequence( self, "atakb" );
	else
		pSequence = FRAMES_GetSequence( self, "ataka" );

	float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
	AI_Dprintf( "%g\n", dist );

	if( dist < 100  )
		AI_ForceSequence( self, pSequence );
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
void shark_attack( userEntity_t *self )
{
	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			shark_set_attack_seq( self );
		}
	}
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
void shark_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->attack_finished = gstate->time + 1.0;

	shark_set_attack_seq( self );
}

// ----------------------------------------------------------------------------
//
// Name:		shark_start_pain
// Description: pain function for satyr that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void shark_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,25);
}



// ----------------------------------------------------------------------------
// NSS[2/10/00]:
// Name:SHARK_Think
// Description:The'pre' think for the Shark.  Will also keep him in the water
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SHARK_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook)
	{
		//Get the goal pointers and task pointers
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;

		if( pGoal == NULL )
		{
			AI_AddNewGoal(self,GOALTYPE_WANDER);
			self->nextthink = gstate->time + 0.1f;
			return;
		}
		if(pGoal->nGoalType == GOALTYPE_KILLENEMY && self->enemy && self->enemy->waterlevel < 1)
		{
				AI_AddNewGoal(self,GOALTYPE_WANDER);
				hook->ai_flags |= AI_IGNORE_PLAYER;
				self->nextthink = gstate->time + 0.1f;
				return;
		}
		else if(pGoal->nGoalType == GOALTYPE_WANDER && self->enemy && self->enemy->waterlevel >2)
		{
			AI_RemoveCurrentGoal(self);
			hook->ai_flags &= ~AI_IGNORE_PLAYER;
		}
	}
	AI_TaskThink(self);
}

// ----------------------------------------------------------------------------
// NSS[2/10/00]:
// Name:SHARK_ParseEpairs
// Description:Setup the thin function from here.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SHARK_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think		= SHARK_Think;
	self->nextthink = gstate->time + 0.2;
}



// ----------------------------------------------------------------------------
// NSS[2/24/00]:
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_shark( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_SHARK );

	self->className			= "monster_shark";
	self->netname			= tongue_monsters[T_MONSTER_SHARK];

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0 );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	////////////////	///////////////////////////////////////////

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 150;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->attack_dist		= 80;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 100;
	self->mass				= 2.0;

	//self->linkcount         = MOVETYPE_SWIM;
    self->movetype          = MOVETYPE_SWIM;

    hook->fnStartAttackFunc = shark_begin_attack;
	hook->fnAttackFunc		= shark_attack;

	self->pain				= shark_start_pain;

	self->think				= SHARK_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give shark a weapon
	///////////////////////////////////////////////////////////////////////////
	self->inventory = gstate->InventoryNew (MEM_MALLOC);
	
	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	if ( pWeaponAttributes )
	{
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[0].fBaseDamage, 
										  pWeaponAttributes[0].fRandomDamage,
										  pWeaponAttributes[0].fSpreadX,
										  pWeaponAttributes[0].fSpreadZ,
										  pWeaponAttributes[0].fSpeed,
										  pWeaponAttributes[0].fDistance,
										  pWeaponAttributes[0].offset, 
										  "punch", 
										  melee_punch, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_WATER_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;
	
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);
	hook->ai_flags |=	AI_CANSWIM;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_shark_register_func()
{
	// NSS[2/10/00]:Added these two functions
	gstate->RegisterFunc("SHARK_ParseEpairs",SHARK_ParseEpairs);
	gstate->RegisterFunc("SHARK_Think",SHARK_Think);
	gstate->RegisterFunc("shark_begin_attack",shark_begin_attack);
	gstate->RegisterFunc("shark_attack",shark_attack);
	gstate->RegisterFunc("shark_start_pain",shark_start_pain);
}

