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
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */
#define KNIGHT2_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f
#define KNIGHT2_CHASE_MIN_DISTANCE 100.0f
#define KNIGHT2_CHASE_MAX_DISTANCE 250.0f
#define KNIGHT2_CHASE_PROBABILITY 0.7f
#define KNIGHT2_AXE_THROW_PROBABILITY 0.6f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void knight2_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void knight2_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport	void	monster_knight2 (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// knight2 specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void	knight2_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		AI_Knight2TakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_Knight2TakeCoverAttack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
//		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 80) )
		{
			ai_fire_curWeapon( self );
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}
	
	if ( AI_IsEndAnimation( self ) )
	{
		AI_IsEnemyDead( self );

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Knight2TakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Knight2TakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

//	frameData_t *pSequence = NULL;// SCG[1/23/00]: not used
	if ( hook->cur_sequence &&
	     (strstr( hook->cur_sequence->animation_name, "atak" ) || strstr( hook->cur_sequence->animation_name, "stand" )) )
	{
//		pSequence = FRAMES_GetSequence( self, "ataka" );
		FRAMES_GetSequence( self, "ataka" );// SCG[1/23/00]: pSequence not used
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if ( AI_Knight2TakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		knight2_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = NULL;
	    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	    if ( dist > 80 )
	    {
			// ranged attack (lightning)
			pSequence = FRAMES_GetSequence( self, "atakc" );
			AI_ForceSequence(self, pSequence); 

			hook->nAttackMode = ATTACKMODE_RANGED;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning");
	    }
	    else
	    {
			// melee (lightning punch)
			switch( (int) rnd()*2 )
			{
				case 0:
					pSequence = FRAMES_GetSequence( self, "ataka" );
					break;
				case 1:
					pSequence = FRAMES_GetSequence( self, "atakb" );
					break;
			}
			AI_ForceSequence(self, pSequence);

			hook->nAttackMode = ATTACKMODE_MELEE;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning punch");
	    }
    }
}


// ----------------------------------------------------------------------------
//
// Name:		knight2_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void knight2_melee_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy( self, self->enemy ) && hook->nAttackMode != ATTACKMODE_NORMAL )
	{
		if( AI_IsReadyToAttack1( self ) )
			ai_fire_curWeapon( self );
	}
	
	if ( (hook->nAttackMode == ATTACKMODE_NORMAL) || AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning");
			AI_RemoveCurrentTask( self );
			return;
		}
		else
			knight2_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		knight2_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_ranged_attack( userEntity_t *self )
{
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning");
	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 0) )
		{
			ai_fire_curWeapon( self );
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			if ( AI_ShouldFollow( self ) == TRUE )
			{
				AI_RemoveCurrentTask( self );
			}
			else
			{
				// give up attacking the enemy
				AI_RemoveCurrentGoal( self );
			}
		}
		else
		{
			//NSS[11/11/99]:We want them to move around a tad bit more than this.
			//if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.2f && AI_CanMove( hook ) )
			if ( rnd() > 0.25f)
			{
				AI_DoEvasiveAction( self );
			}
			else
			{
				knight2_set_attack_seq (self);
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		knight2_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
		knight2_ranged_attack(self);
	else
		knight2_melee_attack(self);

	self->lastAIFrame = self->s.frame;
}

// ----------------------------------------------------------------------------
//
// Name:		knight2_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_begin_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	self->lastAIFrame = 0;

	AI_ZeroVelocity( self );

	knight2_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		knight2_start_pain
// Description: pain function for knight2 that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		knight2_start_die
// Description: die function for knight2
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight2_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	if( notdeadyet && self->deadflag == DEAD_DYING )
	{
		if( rand() % 2 )
			AI_ForceSequence( self, "diea" );
		else
			AI_ForceSequence( self, "dieb" );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		knight2_check_range
// Description: range check function for knight2
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int knight2_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning punch");

	if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		return TRUE;

	if ( rnd() < KNIGHT2_AXE_THROW_PROBABILITY )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "lightning");
		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
			return TRUE;
	}

	return ( dist > KNIGHT2_CHASE_MIN_DISTANCE && dist < KNIGHT2_CHASE_MAX_DISTANCE &&
	         rnd() < KNIGHT2_AXE_THROW_PROBABILITY );
}


// ----------------------------------------------------------------------------
//NSS[11/3/99]:
// Name:Knight2_Think
// Description:Think trap for knight before AI_Think
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void KNIGHT2_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(self->enemy)
	{
		if(AI_IsVisible(self,self->enemy) && hook->acro_boost==0)
		{
			//Attach the Sprite Light&Glow and fire stuff 
			trackInfo_t tinfo;

			// clear this variable
			memset(&tinfo, 0, sizeof(tinfo));
			tinfo.fru.Zero();
			tinfo.ent			= self;
			tinfo.srcent		= self;
			tinfo.lightColor.x	= 0.15;//R
			tinfo.lightColor.y	= 0.25;//G
			tinfo.lightColor.z	= 0.95;//B
			tinfo.lightSize		= 125.00;	
			tinfo.scale			= 1.25;
			tinfo.modelindex	= gstate->ModelIndex("models/global/e_flblue.sp2");
			tinfo.scale2		= 0.85;
			tinfo.modelindex2	= gstate->ModelIndex("models/global/e_flblue.sp2");
			

//			sprintf(tinfo.HardPoint_Name,"hr_muzzle1");
//			sprintf(tinfo.HardPoint_Name2,"hr_muzzle2");
			Com_sprintf(tinfo.HardPoint_Name,sizeof(tinfo.HardPoint_Name),"hr_muzzle1");
			Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle2");
			
			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_HARDPOINT|TEF_MODELINDEX|TEF_SCALE|TEF_HARDPOINT2|TEF_MODELINDEX2|TEF_SCALE2;
			tinfo.fxflags = TEF_LIGHT|TEF_ELECTRICSWORD|TEF_FX_ONLY|TEF_SPRITE|TEF_MODEL|TEF_ATTACHMODEL;
			tinfo.renderfx = SPR_ALPHACHANNEL;
			
			com->trackEntity(&tinfo,MULTICAST_PVS);
			hook->acro_boost = 1;
		}
		else if ((!(AI_IsVisible(self,self->enemy)) && hook->acro_boost == 1)|| !AI_IsAlive(self))
		{
			RELIABLE_UNTRACK(self);
			hook->acro_boost = 0;
		}
	}

	AI_TaskThink( self);
}


// ----------------------------------------------------------------------------
//NSS[11/3/99]:
// Name:Knight2_ParseEpairs
// Description:Trap to set the think sequence
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void Knight2_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = KNIGHT2_Think;
	self->nextthink = gstate->time + 0.1f;
}



// ----------------------------------------------------------------------------
//
// Name:		monster_knight2
// Description:
//				spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_knight2(userEntity_t *self)
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_KNIGHT2);

	self->className		= "monster_knight2";
	self->netname		= tongue_monsters[T_MONSTER_KNIGHT2];

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

	///////////////////////////////////////////////////////////////////////////
	//	give knight2 a weapon
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
										  "lightning", 
										  lightning_attack, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "lightning punch", 
										  lightning_punch, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;  //ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->fnStartAttackFunc = knight2_begin_attack;
	hook->fnAttackFunc		= knight2_attack;
	hook->fnTakeCoverFunc	= AI_Knight2TakeCover;

	hook->fnInAttackRange   = knight2_check_range;

	hook->pain_chance		= 5;

	self->pain				= knight2_start_pain;
	self->die				= knight2_start_die;

	self->think				= Knight2_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	hook->pain_chance		= 5;

	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_valkyrie_register_func()
{
	gstate->RegisterFunc("knight2_begin_attack",knight2_begin_attack);
	gstate->RegisterFunc("knight2_attack",knight2_attack);
	gstate->RegisterFunc("AI_Knight2TakeCover",AI_Knight2TakeCover);
	gstate->RegisterFunc("knight2_check_range",knight2_check_range);
	gstate->RegisterFunc("knight2_start_pain",knight2_start_pain);
	gstate->RegisterFunc("knight2_start_die",knight2_start_die);
	gstate->RegisterFunc("Knight2_ParseEpairs",Knight2_ParseEpairs);
	gstate->RegisterFunc("KNIGHT2_Think",KNIGHT2_Think);	
}