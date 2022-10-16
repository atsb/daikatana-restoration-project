#include "world.h"
#include "nodelist.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai_func.h"
#include "ai_utils.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

// distance at which Garroth tries a melee attack
#define GARROTH_TRY_MELEE_DIST 200.0f
// distance at which Garroth attacks at all
#define GARROTH_ATTACK_DIST 600.0f
// frequency of Garroth's attack
#define GARROTH_ATTACK_CHANCE 0.80
// chance of Garroth's stave attack
#define GARROTH_STAVE_CHANCE 1
// chance of Garroth's wisp attack
#define GARROTH_WISP_CHANCE 1


/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

int garroth_check_range(userEntity_t *self, float dist, userEntity_t *enemy);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* **************************** Extern Functions *************************** */

////////////////////////////////////////////////////////////////////////////////
// exports
////////////////////////////////////////////////////////////////////////////////

DllExport	void	monster_garroth (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
// local functions
///////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		garroth_check_range
// Description: Checks given distance to determine if it's in range.
// Input:
// Output:
// Note:        The enemy pointer will usually be NULL.
//
// ----------------------------------------------------------------------------
int garroth_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// Looking to try a melee attack?
	if ( dist < GARROTH_TRY_MELEE_DIST )
	{
		self->curWeapon = gstate->InventoryFindItem( self->inventory, "punch" );

		return ( dist < ((ai_weapon_t *)self->curWeapon)->distance );
	}

	return ( dist <= GARROTH_ATTACK_DIST );
}


// ----------------------------------------------------------------------------
//
// Name:		garroth_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void garroth_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;
	CVector Skill_Values;
	Skill_Values.Set(0.30f,0.70f,0.85f);
	float Attack_Chance = AI_GetSkill(Skill_Values ); 
	if ( AI_IsFacingEnemy( self, self->enemy ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		self->curWeapon = gstate->InventoryFindItem( self->inventory, "punch" );

		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		{
			hook->nAttackMode = ATTACKMODE_MELEE;

			// go for a melee attack
			AI_ForceSequence( self, FRAMES_GetSequence( self, "atakc" ) );
		}
		else
		{
			if ( rnd() <  Attack_Chance)
			{
				int chance = (int)(rnd()*3.0f);

				hook->nAttackMode = ATTACKMODE_RANGED;

				// run magic attack sequence
				AI_ForceSequence( self, FRAMES_GetSequence( self, "ataka" ) );

				switch(chance)
				{
					case 1:
					{
						self->curWeapon = gstate->InventoryFindItem( self->inventory, "stave" );

						// check distance and switch if outside range
						if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
						{
							self->curWeapon = gstate->InventoryFindItem( self->inventory, "WyndraxWisp" );
							if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
							{
								self->curWeapon = gstate->InventoryFindItem( self->inventory, "garroth summon" );
								// run the summon anim
								AI_ForceSequence( self, FRAMES_GetSequence( self, "ataka" ) );
							}
						}
						break;
					}
					case 2:
					{
						self->curWeapon = gstate->InventoryFindItem( self->inventory, "WyndraxWisp" );

						// check distance and switch if outside range
						if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
						{
							self->curWeapon = gstate->InventoryFindItem( self->inventory, "stave" );
							if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
							{
								self->curWeapon = gstate->InventoryFindItem( self->inventory, "garroth summon" );
								// run the summon anim
								AI_ForceSequence( self, FRAMES_GetSequence( self, "ataka" ) );
							}
						}
						break;
					}
					default:
					{
						self->curWeapon = gstate->InventoryFindItem( self->inventory, "garroth summon" );

						// check distance and switch if outside range
						if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
						{
							self->curWeapon = gstate->InventoryFindItem( self->inventory, "stave" );
							if ( dist > ((ai_weapon_t *)self->curWeapon)->distance )
							{
								self->curWeapon = gstate->InventoryFindItem( self->inventory, "WyndraxWisp" );
							}
						}
						else
						{
							// run the summon anim
							AI_ForceSequence( self, FRAMES_GetSequence( self, "ataka" ) );
						}						
					}
				}
				// choose a weapon
			/*	if(chance < GARROTH_STAVE_CHANCE)
				{

				}
				else if(chance < GARROTH_STAVE_CHANCE + GARROTH_WISP_CHANCE)
				{

				}
				else
				{

				}*/
			}
			else
			{
				AI_ForceSequence( self, FRAMES_GetSequence( self, "amba" ) );
			}
		}
	}
	else
	{
		AI_ForceSequence( self, FRAMES_GetSequence( self, "amba" ) );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		garroth_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void garroth_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( hook->nAttackMode != ATTACKMODE_NORMAL &&
	     (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) )
	{
		ai_fire_curWeapon( self );
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		// Can we melee attack?
		if ( dist > hook->attack_dist || !AI_IsVisible(self, self->enemy) )
		{
			// stop attacking for now
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
			AI_StartNextTask( self );
		}
		else
		{
			garroth_set_attack_seq( self );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		garroth_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void garroth_begin_attack( userEntity_t *self )
{
	garroth_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		garroth_start_die
// Description: die function for garroth
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void garroth_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( rnd() < 0.5f )
		{
			AI_ForceSequence( self, "diea" ); // keel over
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // collapse bacwards
		}
	}
}




///////////////////////////////////////////////////////////////////////////////
//	monster_garroth
//
//
///////////////////////////////////////////////////////////////////////////////

void	monster_garroth (userEntity_t *self)
{
	playerHook_t *hook = AI_InitMonster(self, TYPE_GARROTH);

	self->className = "monster_garroth";
	self->netname = tongue_monsters[T_MONSTER_GARROTH];

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

	// Garroth can't gib, he's got to appear in cinematics
	self->fragtype         |= FRAGTYPE_NEVERGIB;
	hook->run_speed         = 250;
	hook->walk_speed        = 50;
	hook->attack_speed      = 250;

	hook->attack_dist       = 1000;

	hook->pain_chance       = 5;

	hook->fnStartAttackFunc = garroth_begin_attack;
	hook->fnAttackFunc		= garroth_attack;

	hook->fnInAttackRange   = garroth_check_range;

	self->die				= garroth_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink         = gstate->time + 0.2; // node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give garroth a weapon
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
		self->curWeapon = ai_init_weapon( self,
										  pWeaponAttributes[1].fBaseDamage,
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset,
										  "stave",
										  stave_attack,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
		self->curWeapon = ai_init_weapon( self,
										  pWeaponAttributes[2].fBaseDamage,
										  pWeaponAttributes[2].fRandomDamage,
										  pWeaponAttributes[2].fSpreadX,
										  pWeaponAttributes[2].fSpreadZ,
										  pWeaponAttributes[2].fSpeed,
										  pWeaponAttributes[2].fDistance,
										  pWeaponAttributes[2].offset,
										  "WyndraxWisp",
										  WyndraxWisp_Attack,
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		// give the summon weapon the same attributes as the stave
		self->curWeapon = ai_init_weapon( self,
										  pWeaponAttributes[1].fBaseDamage,
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset,
										  "garroth summon",
										  garroth_summon,
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_garroth_register_func()
{
	gstate->RegisterFunc("garroth_start_die",garroth_start_die);
	gstate->RegisterFunc("garroth_begin_attack",garroth_begin_attack);
	gstate->RegisterFunc("garroth_attack",garroth_attack);
	gstate->RegisterFunc("garroth_check_range",garroth_check_range);
}
