// ==========================================================================
//
//  File:
//	the Lab Monkey!!!
//  Contents:
//  Author:
//
//	TODO:	make ai switchable for monkey bonus!
//	TODO:	more ambient screaming, chest beating, etc.
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
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
DllExport void monster_labmonkey (userEntity_t *self);
//DllExport void monkey_message (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
// monkey specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void monkey_close_melee (userEntity_t *self);
void monkey_far_melee (userEntity_t *self);
void monkey_jump_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		monkey_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( dist > 108 && hook->dflags & DFL_JUMPATTACK )
	{
		if ( rnd() > 0.5 )
		{
			pSequence = FRAMES_GetSequence( self, "atakc" );
		}
		else
		{
			pSequence = FRAMES_GetSequence( self, "atakd" );
		}
		AI_ForceSequence( self, pSequence );

		//	give jumping velocity to monkey here
		CVector temp = self->s.angles;
		temp.AngleToVectors(forward, right, up);

		temp = forward * hook->run_speed * 1.5;
		temp.z = hook->upward_vel;
		self->velocity = temp;
		//NSS[11/14/99]: This is a No! No!
		//self->groundEntity = NULL;
	}
	else 
	if ( dist > 56 )
	{
		if (rnd () < 0.5)
		{
			pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence(self, pSequence);
		}
		else
		{
			pSequence = FRAMES_GetSequence( self, "atake" );
			AI_ForceSequence(self, pSequence);
		}
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "atakb" );
		AI_ForceSequence(self, pSequence);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_hop
// Description:
//	for simplicity's sake, monkey doesn't check where it's hopping
//	also saves us some trace lines
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_hop( userEntity_t *self, float yaw_add )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	float			hop_yaw;
	CVector			hop_angles;
	
	hop_yaw = self->s.angles [1];
	if ( rnd() < 0.5f )
	{
		hop_yaw += yaw_add;
	}
	else
	{
		hop_yaw -= yaw_add;
	}

	hop_angles = CVector(0, hop_yaw, 0);
	hop_angles.AngleToVectors(forward, right, up);

	forward = forward * 150.0;
	self->velocity = forward;
	//NSS[11/14/99]: This is a No! No!
	//self->groundEntity = NULL;
	self->velocity.z = hook->upward_vel;

	frameData_t *pSequence = FRAMES_GetSequence( self, "atakc" );
	AI_ForceSequence(self, pSequence);
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_roar
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_roar( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	//	face enemy if still alive
	if ( self->enemy->deadflag == DEAD_NO )
	{
		//ai_set_goal( self, self->enemy, self->enemy->s.origin, GOAL_ENEMY );
	}

//	int i = (int) floor(rnd () * 3) + 2;// SCG[1/23/00]: not used

	// killed the player, so roar
//	ai_set_sequence_sounds (self, hook->frames->stand [i]->soundframe1, hook->frames->stand [i]->sound1, 
//								  hook->frames->stand [i]->soundframe2, hook->frames->stand [i]->sound2);
	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
	AI_ForceSequence(self, pSequence);
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_jump_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
	{
		ai_fire_curWeapon( self );
	}

	// attack without any transitions
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			//AI_RemoveCurrentTask( self );
			AI_AddNewTaskAtFront( self, TASKTYPE_CHASE );
			AI_StartNextTask( self );

			//if ( rnd() < 0.3 )
			//{
			//	monkey_roar( self );
			//}
		}
		else
		{
			monkey_set_attack_seq( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_close_melee
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_close_melee( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			//	player ran away, so roar sometimes
			//	always transition back to stand first
			//if (rnd () < 0.3)
			//{
			//	monkey_roar (self);
			//}
		}
		else
		{
			monkey_set_attack_seq (self);

			//	random chance to hop
			if (rnd () < 0.5)
			{
				monkey_hop( self, 110 );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_far_melee
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_far_melee( userEntity_t *self )
{
//	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
	{
		ai_fire_curWeapon( self );
	}

	// attack without any transitions
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			//if ( rnd() < 0.3 )
			//{
			//	monkey_roar (self);
			//}

			AI_RemoveCurrentTask( self );
		}
		else
		{
			monkey_set_attack_seq (self);

			//	random chance to hop
			if ( rnd() < 0.75 )
			{
				monkey_hop (self, 70);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

//	float fDist = VectorDistance (self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( dist > 108 && hook->dflags & DFL_JUMPATTACK )
	{
		monkey_jump_attack( self );
	}
	else 
	if (dist > 56)
	{
		monkey_far_melee( self );
	}
	else
	{
		monkey_close_melee( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_begin_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	monkey_set_attack_seq (self);
}


// ----------------------------------------------------------------------------
//
// Name:	 monkey_start_pain
// Description: pain function for satyr that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		monkey_screw
// Description:
//	screw with monkey's head
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monkey_screw( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
//	playerHook_t	*hook = AI_GetPlayerHook( self );
//	playerHook_t	*mhook;
//	doorHook_t		*dhook;
//	userEntity_t	*head, *sw, *monkey;
//
//	if (self->deadflag != DEAD_NO)
//	{
//		return;
//	}
//
//	monkey = sw = NULL;
//	for (head = gstate->FirstEntity (); head; head = gstate->NextEntity (head))
//	{
//		if (!head->className)
//		{
//			continue;
//		}
//
//		//	find first monkey
//		if ( monkey == NULL && !stricmp (head->className, "monster_labmonkey") )
//		{
//			monkey = head;
//		}
//
//		if (!head->target)
//		{
//			continue;
//		}
//
//		if ( !_stricmp(head->target, self->targetname) )
//		{
//			sw = head;
//		}
//	}
//
//	if (!sw)
//	{
//		return;
//	}
//
//	dhook = (doorHook_t *) sw->userHook;
//
//	if (hook->dflags & DFL_BERSERK)
//	{
//		hook->dflags -= DFL_BERSERK;
//		hook->dflags |= DFL_EVIL;
//	}
//	else 
//	if (hook->dflags & DFL_EVIL)
//	{
//		hook->dflags -= DFL_EVIL;
//		
//		if (dhook->state == STATE_DOWN || dhook->state == STATE_BOTTOM)
//		{
//			hook->dflags |= DFL_BERSERK;
//		}
//		else
//		{
//			hook->dflags |= DFL_GOOD;
//		}
//	}
//	else 
//	if (hook->dflags & DFL_GOOD)
//	{
//		hook->dflags -= DFL_GOOD;
//		hook->dflags |= DFL_EVIL;
//	}
//
//	/////////////////////////////////////////////////////////////
//	//	if good or evil, reset monkey's enemy
//	/////////////////////////////////////////////////////////////
//
//	if (hook->dflags & (DFL_GOOD + DFL_EVIL))
//	{
//		task_clear (&hook->task_stack);
//		self->enemy = NULL;
//
//		task_stand (self);
//		task_wander (self);
//
//		hook->begin_wander (self);
//	}
//
//	/////////////////////////////////////////////////////////////
//	//	if good, target monsters
//	/////////////////////////////////////////////////////////////
//
//	if (hook->dflags & (DFL_GOOD))
//	{
//		hook->find_target = ai_find_target_monster_bot;
//	}
//
//	/////////////////////////////////////////////////////////////
//	//	if evil, target clients and bots
//	/////////////////////////////////////////////////////////////
//
//	else 
//	if (hook->dflags & (DFL_EVIL))
//	{
//		hook->find_target = ai_find_target_client_bot;
//	}
//
//	/////////////////////////////////////////////////////////////
//	//	if berserk, target everything
//	/////////////////////////////////////////////////////////////
//
//	else 
//	if (hook->dflags & (DFL_BERSERK))
//	{
//		hook->find_target = ai_find_target_berserk;
//	}
//
//	if (!monkey)
//	{
//		gstate->Con_Printf ("Monker-Meter registers no monkeys.\n");
//		return;
//	}
//
//	//	print message if this is the first monkey
//	if (monkey == self)
//	{
//		mhook = AI_GetPlayerHook( monkey );
//
//		if (mhook->dflags & DFL_GOOD)
//		{
//			gstate->bprint ("Monkey-Meter set to GOOD MONKEY.\n");
//		}
//		else 
//		if (mhook->dflags & DFL_EVIL)
//		{
//			gstate->bprint ("Monkey-Meter set to BAD MONKEY.\n");
//		}
//		else 
//		if (mhook->dflags & DFL_BERSERK)
//		{
//			gstate->bprint ("Monkey-Meter set to APE-SHIT.\n");
//		}
//	}
}


// ----------------------------------------------------------------------------
// NSS[3/9/00]:
// Name:		LABMONKEY_Activate
// Description: Will set the lab monkies into violent mode!
// Input:userEntity_t *self, userEntity_t *other, userEntity_t *activator
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void LABMONKEY_Activate(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		// remove the ignore player flag
		hook->ai_flags &= ~AI_IGNORE_PLAYER;

		//If we don't have a find enemy function then get outta here!
		if(!hook->fnFindTarget)
			return;
		
		//Find an enemy
		userEntity_t *pEnemy = hook->fnFindTarget( self );
		
		//If found then let's rock and roll!
		if(pEnemy)
		{
			AI_SetEnemy( self, pEnemy );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_labmonkey
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_labmonkey (userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_LABMONKEY );

	self->className			= "monster_labmonkey";
	self->netname			= tongue_monsters[T_MONSTER_LABMONKEY];

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

	hook->fnStartAttackFunc = monkey_begin_attack;
	hook->fnAttackFunc		= monkey_attack;
	self->pain				= monkey_start_pain;

	hook->dflags            |= DFL_JUMPATTACK;

	hook->run_speed			= 200;
	hook->walk_speed		= 70;
	hook->attack_dist		= 80;

	self->health			= 100;
	hook->base_health		= 100;
	hook->pain_chance		= 20;
	hook->jump_chance		= 50;
	self->mass				= 2.0;
	self->gravity			= 1.0;
	self->ang_speed.y		= 150;
	//self->use				= monkey_screw;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	///////////////////////////////////////////////////////////////////////////
	//	give monkey its melee weapon
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
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	self->use = LABMONKEY_Activate;
	
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


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_labmonkey_register_func()
{
	gstate->RegisterFunc("monkey_begin_attack",monkey_begin_attack);
	gstate->RegisterFunc("monkey_attack",monkey_attack);
	gstate->RegisterFunc("monkey_start_pain",monkey_start_pain);
	gstate->RegisterFunc("LABMONKEY_Activate",LABMONKEY_Activate);// NSS[3/9/00]:
}
