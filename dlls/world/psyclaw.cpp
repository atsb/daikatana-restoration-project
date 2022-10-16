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
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_common.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_psyclaw"

#define MELEE_ATTACK_DISTANCE	135.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_psyclaw( userEntity_t *self );
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
void AI_OrientToFloor( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		psyclaw_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> melee attack
//				atakb	=> melee swinging attack
//				atakc	=> range attack
//
// ----------------------------------------------------------------------------
void psyclaw_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = NULL;
	    float fDist = VectorDistance (self->s.origin, self->enemy->s.origin);
	    if ( fDist > MELEE_ATTACK_DISTANCE && !(self->enemy->flags & FL_WARPEDVIEW) )
	    {
		    pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence( self, pSequence );
		    self->curWeapon = gstate->InventoryFindItem(self->inventory, "psyclaw blast");
			AI_ZeroVelocity(self);
		    hook->nAttackMode = ATTACKMODE_RANGED;
	    }
	    else
	    {
			pSequence = FRAMES_GetSequence( self, "atakb" );
		    AI_ForceSequence( self, pSequence );
		    self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
		    hook->nAttackMode = ATTACKMODE_MELEE;
	    }
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
    }
}

// ----------------------------------------------------------------------------
// NSS[2/25/00]:
// Name:		PSYCLAW_StartJumpUp
// Description:Begin the jump up process
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void PSYCLAW_StartJumpUp( userEntity_t *self )
{
	_ASSERTE( self );
 	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector Dir;

	char szAnimation[16];
	AI_SelectRunningAnimation( self, szAnimation );
	AI_ForceSequence( self, szAnimation, FRAME_LOOP );
	AIDATA *pAIData = AI_GetAIDATA(self);

	if(self->enemy)
	{
		CVector Angles;
		Dir = self->enemy->s.origin - self->s.origin;
		Dir.Normalize();
		VectorToAngles(Dir,Angles);
		self->s.angles = Angles;
	}
	else
	{
		self->s.angles.AngleToForwardVector(Dir);	
	}
	
	CVector vector( Dir.x, Dir.y, 1.0f );
	
	
	AI_SetVelocity( self, vector, hook->upward_vel*0.90);

	
	pAIData->destPoint.x	= self->velocity.x * 0.25;
	pAIData->destPoint.y	= self->velocity.y * 0.25;
	pAIData->fValue			= gstate->time + 0.45f;
	pAIData->nValue			= 0;
	AI_SetNextThinkTime( self, 0.1f );
	
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );
}


// ----------------------------------------------------------------------------
// NSS[2/25/00]:
// Name:		PSYCLAW_JumpUp
// Description:The jump up function... you know.. jump up... like when they say jump...
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void PSYCLAW_JumpUp( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	AIDATA *pAIData = AI_GetAIDATA(self);
	if(pAIData->fValue < gstate->time)
	{
		if(!pAIData->nValue)
		{
			self->velocity.x = pAIData->destPoint.x;
			self->velocity.y = pAIData->destPoint.y;
			pAIData->nValue  = 1;
		}
		
		if ( self->groundEntity)
		{
			AI_UpdateCurrentNode( self );
			AI_RemoveCurrentTask( self );
			return;
		}

		ai_frame_sounds( self );
	}
}

// ----------------------------------------------------------------------------
// NSS[2/25/00]:
// Name:		psyclaw_doattack
// Description:Hack hack hack hack hack... we here at ION believe in HACKING
// because we have NO CLUE as to what we want things to do... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
int psyclaw_chasethink(userEntity_t *self)
{
	if(self->enemy)
	{
		float fXYDistance	= VectorXYDistance(self->s.origin,self->enemy->s.origin);
		float fZDistance	= VectorZDistance(self->s.origin,self->enemy->s.origin);
		if(fXYDistance < 200.0f && (fZDistance > 32.0f && fZDistance < 136.0f)&& !AI_HasTaskInQue(self,TASKTYPE_PSYCLAW_JUMPUP) && self->s.origin.z < self->enemy->s.origin.z)
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_PSYCLAW_JUMPUP);
			return 1;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
//
// Name:		psyclaw_do_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_do_attack( userEntity_t *self )
{

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.5f && AI_CanMove( hook ) && hook->nAttackMode != ATTACKMODE_MELEE )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
			AI_SetTaskFinishTime( hook, 2.5f );
		}
		else
		{
			AI_SetOkToAttackFlag( hook, TRUE);
			AI_RemoveCurrentTask( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		psyclaw_doattack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_doattack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDist = VectorDistance(self->s.origin, self->enemy->s.origin);
	if ( fDist > MELEE_ATTACK_DISTANCE )
	{
		
		if(self->enemy->flags & FL_WARPEDVIEW)
		{
			AI_RemoveCurrentTask(self,FALSE);
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
		    hook->nAttackMode = ATTACKMODE_MELEE;
			return;
		}
		if ( hook->nAttackMode != ATTACKMODE_RANGED )
		{
			psyclaw_set_attack_seq( self );
		}

	}
	else
	{
		if ( hook->nAttackMode != ATTACKMODE_MELEE )
		{
			psyclaw_set_attack_seq( self );
		}		
	}
	
	psyclaw_do_attack(self);
}

// ----------------------------------------------------------------------------
//
// Name:		psyclaw_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_begin_attack( userEntity_t *self )
{
	_ASSERTE( self->enemy );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	
	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.5f )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
		AI_SetTaskFinishTime( hook, 2.5f );
		//AI_DoEvasiveAction( self );
	}
	else
	{
		psyclaw_set_attack_seq (self);
	}
	AI_SetOkToAttackFlag( hook, FALSE);
	AI_OrientToFloor(self);
}


// ----------------------------------------------------------------------------
//
// Name:		psyclaw_start_pain
// Description: pain function for psyclaw that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		psyclaw_start_die
// Description: die function for psyclaw
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( rnd() < 0.5f )
		{
			AI_ForceSequence( self, "diea" ); // fall over slowly
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[11/9/99]:
// Name:		Psyclaw_Think
// Description:Basic Pre-Think Think function... not the actual pre-think
// but something that should be done before the generic AI_TaskThink
// Input:useEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Psyclaw_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(self->enemy)
	{
		ai_weapon_t	*weapon = (ai_weapon_t *) self->curWeapon;
		//This is a check for after the 'psycho' attack if the player is finished triping
		//and we are trying to chase them down, switch back to psycho mode and try to make the
		//player trip more... 
		if(!(self->enemy->flags & FL_WARPEDVIEW) && (_stricmp(weapon->weapon.name,"punch")==0))
		{
			float fDist = VectorDistance(self->s.origin, self->enemy->s.origin);
			if(fDist > MELEE_ATTACK_DISTANCE)
			{
				playerHook_t *hook = AI_GetPlayerHook( self );
				AI_RemoveCurrentTask(self,FALSE);
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "psyclaw blast");
				hook->nAttackMode = ATTACKMODE_MELEE;
			}
		}
	}
	// NSS[3/23/00]:Hack to prevent psyclaws from being killed from the start.
	if(hook && self->takedamage == DAMAGE_NO && hook->envirosuit_time < gstate->time)
	{
		self->takedamage = DAMAGE_YES;
	}
	
	AI_TaskThink(self);
}


// ----------------------------------------------------------------------------
// NSS[11/9/99]:
// Name:		Psyclaw_ParseEpairs
// Description:This is a hook between the think function and the psyclaw
// Input:useEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Psyclaw_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = Psyclaw_Think;
	self->nextthink = gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
//NSS[11/9/99]:
// Name:		monster_psyclaw
// Description: The wonderful world of weird things!
// Input:userEntity_t *self 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void monster_psyclaw( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PSYCLAW );

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_PSYCLAW];

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

	hook->fnStartAttackFunc = psyclaw_begin_attack;
	hook->fnAttackFunc		= psyclaw_doattack;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->fnStartJumpUp		= PSYCLAW_StartJumpUp;
	hook->fnJumpUp			= PSYCLAW_JumpUp;
	
	self->pain				= psyclaw_start_pain;
	hook->pain_chance		= 5;
	
	self->die				= psyclaw_start_die;

	self->think				= Psyclaw_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	give monster its weapon
	//////////////////////////////////////////////
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
										  "punch", 
										  melee_punch, 
										  ITF_TRACE );
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "psyclaw blast", 
										  psyclaw_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
	}

	hook->pain_chance		= 15;

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

	self->takedamage		= DAMAGE_NO;

	hook->envirosuit_time	= gstate->time + 0.75f;

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;

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

#if 0
#endif 0


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//NSS[11/9/99]:
///////////////////////////////////////////////////////////////////////////////
void world_ai_psyclaw_register_func()
{
	gstate->RegisterFunc("psyclaw_start_pain",psyclaw_start_pain);
	gstate->RegisterFunc("psyclaw_start_die",psyclaw_start_die);
	gstate->RegisterFunc("Psyclaw_ParseEpairs",Psyclaw_ParseEpairs);
	gstate->RegisterFunc("Psyclaw_Think",Psyclaw_Think);
	gstate->RegisterFunc("psyclaw_doattack",psyclaw_doattack);
	gstate->RegisterFunc("psyclaw_begin_attack",psyclaw_begin_attack);
}
