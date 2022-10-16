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
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"


/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_sludgeminion"
#define SLUDGE_MODELNAME	"models/e1/e_sludge.dkm"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void sludge_start_idle( userEntity_t *self );
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_sludgeminion (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
// sludge specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void sludge_ranged_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		SLUDGEMINION_Scoop
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SLUDGEMINION_Scoop( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );


	if ( self->s.frameInfo.frameState & FRSTATE_STOPPED)
	{
		if(hook->acro_boost < 3+(5*rnd()))
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e1/m_sludgegetmud.wav"),0.75f, 500.0f, 700.0f);
			frameData_t *pSequence = FRAMES_GetSequence( self, "ambb" );
			AI_ForceSequence(self, pSequence);
			hook->acro_boost += 2+(5*rnd());
		}
		else
		{
			AI_RemoveCurrentTask( self );
			AI_StartSequence(self, "atakstart" );
		}
	}
	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SLUDGEMINION_StartScoop
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SLUDGEMINION_StartScoop( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_SLUDGEMINION_SCOOP )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_SLUDGEMINION_SCOOP );
		return;
	}
	
	if(hook->acro_boost < 0)
		hook->acro_boost = 2+(7*rnd());
	else
		hook->acro_boost += 3;
	AI_Dprintf( "Starting TASKTYPE_SLUDGEMINION_SCOOP.\n" );
	gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e1/m_sludgegetmud.wav"),0.65f, 500.0f, 700.0f);
	frameData_t *pSequence = FRAMES_GetSequence( self, "ambb" );
	AI_ForceSequence(self, pSequence);
	self->nextthink	= gstate->time + 0.1f;
	
	AI_Dprintf("Ammo:%f\n",hook->acro_boost);
}

// ----------------------------------------------------------------------------
//
// Name:		SLUDGEMINION_Dump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SLUDGEMINION_Dump( userEntity_t *self )
{
	if ( AI_IsEndAnimation( self ))
	{
		AI_RemoveCurrentTask( self );
	}

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SLUDGEMINION_StartDump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SLUDGEMINION_StartDump( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_SLUDGEMINION_DUMP )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_SLUDGEMINION_DUMP );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_SLUDGEMINION_DUMP.\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "speciala" );
	if ( AI_StartSequence(self, pSequence) == FALSE )
	{
		return;
	}

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		sludge_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if( hook->acro_boost <= 0  && self->waterlevel > 0 )
	{
			AI_ForceSequence(self, "atakstop" );
			AI_AddNewTaskAtFront( self, TASKTYPE_SLUDGEMINION_SCOOP );
	}
	else
	{
		self->nextthink	= gstate->time + 0.1f;
		if ( AI_IsFacingEnemy( self, self->enemy ) )
		{
			playerHook_t *hook = AI_GetPlayerHook( self );
			frameData_t *pSequence;

			if( rnd() < 0.2 )
				pSequence = FRAMES_GetSequence( self, "atakb" );
			else
				pSequence = FRAMES_GetSequence( self, "ataka" );

			if( hook && hook->cur_sequence )
				if( !strstr( hook->cur_sequence->animation_name, "atak" ) )
					AI_ForceSequence( self, "atakstart" );

			if ( AI_StartSequence(self, pSequence) == FALSE )
				return;
		}
		else
			AI_FaceTowardPoint( self, self->enemy->s.origin );
	}
}
void AI_OrientToFloor( userEntity_t *self );
// ----------------------------------------------------------------------------
//
// Name:		sludge_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_ranged_attack(userEntity_t *self)
{	
	
	//NSS[11/14/99]:How worthless is a programmer that sees a problem but instead of fixing it he leaves a little comment next to where he thinks the problem is? Wow... the level of dumb!
	_ASSERTE(self->enemy);	// amw: insoo.. I've seen it crash because s.origin is a bad vector (enemy was NULL)
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_OrientToFloor(self);
	if ( strstr( hook->cur_sequence->animation_name, "atak" ) )
	{
		AI_PlayAttackSounds( self );

		if ( AI_IsFacingEnemy( self, self->enemy ) )
        {
		    if ( AI_IsReadyToAttack1( self ) )
		    {
			    self->curWeapon = gstate->InventoryFindItem( self->inventory, "sludge left" );
			    hook->acro_boost --;
				ai_fire_curWeapon( self );
		    }
		    if ( AI_IsReadyToAttack2( self ) )
		    {
			    self->curWeapon = gstate->InventoryFindItem( self->inventory, "sludge right" );
			    ai_fire_curWeapon( self );
				hook->acro_boost --;
		    }
        }

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
			float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
			if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) || hook->acro_boost <= 0)
			{
				AI_RemoveCurrentTask( self );
				return;
			}
			else
			{
				sludge_set_attack_seq(self);
			}
		}
	}
	else
	{
		sludge_set_attack_seq(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sludge_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_begin_attack( userEntity_t *self )
{
	AI_ZeroVelocity( self );
	sludge_set_attack_seq (self);
}


// ----------------------------------------------------------------------------
//
// Name:        sludge_start_idle
// Description: Start idle function that selects between two seqs
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_start_idle( userEntity_t *self )
{
    frameData_t *pSequence;
	float sumpChance, chance = rnd();
	
	if( self->waterlevel > 0 )
		sumpChance = 0.4;
	else
		sumpChance = 1.0;

	if ( chance < sumpChance )
	{
		pSequence = FRAMES_GetSequence( self, "amba" );
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "ambb" );
	}

    if ( pSequence )
    {
        if ( AI_StartSequence(self, pSequence, pSequence->flags) == FALSE )
		{
			return;
		}
    }

	AI_StartIdleSettings( self );
}

// ----------------------------------------------------------------------------
//
// Name:		sludgeminion_start_pain
// Description: pain function for sludgeminion that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludgeminion_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,45);
}

// ----------------------------------------------------------------------------
//
// Name:		monster_sludgeminion
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_sludgeminion( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster (self, TYPE_SLUDGEMINION);

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_SLUDGEMINION];

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

	gstate->ModelIndex( SLUDGE_MODELNAME );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = sludge_begin_attack;
	hook->fnAttackFunc		= sludge_ranged_attack;
	hook->fnStartIdle       = sludge_start_idle;

	hook->dflags			|= DFL_MECHANICAL;
	hook->dflags			|= DFL_RANGEDATTACK;

	self->pain				= sludgeminion_start_pain;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	///////////////////////////////////////////////////////////////////////////
	//	give sludgeminion a weapon
	///////////////////////////////////////////////////////////////////////////

	self->inventory = gstate->InventoryNew (MEM_MALLOC);

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
										  "sludge left", 
										  sludge_attack, 
										  ITF_PROJECTILE | ITF_NOLEAD);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "sludge right", 
										  sludge_attack, 
										  ITF_PROJECTILE | ITF_NOLEAD);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	hook->nAttackType	= ATTACK_GROUND_RANGED;
	hook->nAttackMode	= ATTACKMODE_NORMAL;

	hook->pain_chance		= 5;

	//NSS[11/22/99]:DON'T F*&!ING 'AND' SH!T TO FLAGS!!! THIS WILL ERASE OTHER IMPORTANT FLAGS!!!
	//hook->ai_flags		&= AI_CANDODGE;
	hook->ai_flags		|=AI_CANDODGE;
	hook->acro_boost	= 5.0f;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

    gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_sludgeminion_register_func()
{
	gstate->RegisterFunc("sludgeminion_start_pain",sludgeminion_start_pain);
	gstate->RegisterFunc("sludge_begin_attack",sludge_begin_attack);
	gstate->RegisterFunc("sludge_ranged_attack",sludge_ranged_attack);
	gstate->RegisterFunc("sludge_start_idle",sludge_start_idle);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);
}
