///////////////////////////////////////////////////////////////////////////////
//
//  Cryotech (formerly the lab worker)
//
//  He ignores the player until fired upon or in proximity of firing activity.
//  His primary task as existing in the world is to spray the walls and keep 
//  the computers cool.  Mishima Workers with clipboards direct them to the 
//  areas that need maintenance.  Once fired upon the CRYOTECH boys will fire 
//  their cryo spray at the player and go into attack AI.  The Mishima Workers 
//  will run at the first sign of gunfire.
//
///////////////////////////////////////////////////////////////////////////////

#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_cryotech"

#define MELEE_ATTACK_DISTANCE	80.0f

DllExport void monster_cryotech( userEntity_t *self );

class attack_info
{
public:
	attack_info(int frame, CVector& offset)
	{
		this->frame = frame;
		this->offset = offset;
	}

	int		frame;
	CVector	offset;
};

attack_info spray_attack[] =
{
	attack_info( 8,CVector( 17.30,27.28,11.96)),
	attack_info(10,CVector( 12.44,29.27,14.44)),
	attack_info(12,CVector(  6.15,30.87,16.48)),
	attack_info(14,CVector(  0.15,31.36,18.29)),
	attack_info(16,CVector( -5.34,30.82,19.59)),
	attack_info(18,CVector( -8.84,29.94,19.62)),
	attack_info(20,CVector(-11.23,29.48,17.42)),
	attack_info(22,CVector(-11.39,29.53,13.50)),
};

int num_attacks = sizeof(spray_attack) / sizeof(attack_info);

attack_info bambc[] =
{
	attack_info( 0,CVector(  9.16,32.68, 9.67)),
	attack_info( 2,CVector( 10.59,34.30,10.29)),
	attack_info( 4,CVector(  9.38,33.97,10.75)),
	attack_info( 6,CVector(  9.94,34.60,10.54)),
	attack_info( 8,CVector( 10.34,34.20,10.59)),
};
static int nNumAttacks_bambc = sizeof(bambc) / sizeof(attack_info);

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_spray
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_spray(userEntity_t *self, attack_info *pAttackInfo, int nAttackIndex)
{
//	playerHook_t*	hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used
	ai_weapon_t*	weapon = (ai_weapon_t *) self->curWeapon;
	CVector			ang, dir, old_angles;

	if (!self->curWeapon)
	{
		return;
	}

	weapon->ofs	= pAttackInfo[nAttackIndex].offset;

	// SCG[10/26/99]: monsters do not get a client struct
//	self->client->v_angle = self->s.angles;

	//	fire the weapon
	self->curWeapon->use(self->curWeapon, self);
}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_bambb
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_bambb( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// spray the cryo juice
	for ( int i = 0; i < num_attacks; i++ )
	{
		if (spray_attack[i].frame == (self->s.frame - hook->cur_sequence->first))
		{
			cryotech_spray(self, spray_attack, i);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_bambc
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_bambc( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// spray the cryo juice
	for ( int i = 0; i < nNumAttacks_bambc; i++ )
	{
		if (bambc[i].frame == (self->s.frame - hook->cur_sequence->first))
		{
			cryotech_spray(self, bambc, i);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_fire_spray
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_fire_spray(userEntity_t *self, int nAttackIndex)
{
//	playerHook_t*	hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used
	ai_weapon_t*	weapon = (ai_weapon_t *) self->curWeapon;
	CVector			ang, dir, old_angles;

	if (!self->curWeapon)
	{
		return;
	}

	weapon->ofs	= spray_attack[nAttackIndex].offset;

// SCG[10/26/99]: monsters do not get a client struct
//	self->client->v_angle = self->s.angles;

	//	fire the weapon
	self->curWeapon->use(self->curWeapon, self);
}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_set_attack_seq
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_set_attack_seq (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if (AI_IsFacingEnemy(self, self->enemy, 5.0f, 25.0f))
	{
		if ( gstate->time > hook->fAttackTime )
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "bambb" );
			if ( AI_StartSequence(self, pSequence) == FALSE )
			{
				return;
			}

			// grab the cryo spray
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "cryo spray");
			// wait a while after we do the cryo
			hook->fAttackTime = gstate->time + 3.0f;
		}
	}
	else
	{
    	AI_FaceTowardPoint( self, self->enemy->s.origin );
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_attack
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_attack( userEntity_t *self )
{
	playerHook_t*	hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used
	bool fired = false;

	_ASSERTE( self );
	_ASSERTE( hook );
	_ASSERTE( self->enemy );

	if( hook->pain_finished < gstate->time)
	{
		if ( !hook->cur_sequence )
		{
			cryotech_set_attack_seq( self );
			return;
		}

		AI_PlayAttackSounds( self );

		// spray the cryo juice
		for (int i = 0; i < num_attacks; i++)
		{
			if (spray_attack[i].frame == (self->s.frame - hook->cur_sequence->first) )
			{
				fired = true;
				cryotech_fire_spray(self,i);
				break;
			}
		}
		
		if (fired)
		{
			AI_FaceTowardPoint( self, self->enemy->s.origin );
		}

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
			playerHook_t *hook = AI_GetPlayerHook( self );

			float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
			if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy))
			{
				AI_RemoveCurrentTask( self );
				return;
			}
			else
			{
				//	finished one attack, decide what to do next
				cryotech_set_attack_seq( self );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  cryotech_begin_attack
//
///////////////////////////////////////////////////////////////////////////////
void cryotech_begin_attack(userEntity_t *self)
{
	playerHook_t*	hook = AI_GetPlayerHook( self );
	if( hook->pain_finished < gstate->time)
	{
		AI_StopCurrentSequence( self );
		cryotech_set_attack_seq( self );
	}

	AI_ZeroVelocity( self );
}

// ----------------------------------------------------------------------------
//
// Name:		cryotech_start_pain
// Description: pain function for cryotech
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cryotech_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_Start_Die
// Description: die handler for cryotech
// Input:
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void cryotech_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);
	playerHook_t *hook = AI_GetPlayerHook( self );

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ 
		if( hook->cur_sequence )
		{
			if( strstr( hook->cur_sequence->animation_name, "caa" ) ||
				strstr( hook->cur_sequence->animation_name, "camb" ) ||
				!strcmp( hook->cur_sequence->animation_name, "aabaa" ) )
			{
				AI_ForceSequence( self, "diec" );
			}
			else
				AI_ForceSequence( self, "diea" );
		}
		else
			AI_ForceSequence( self, "diea" );
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  monster_cryotech
//
///////////////////////////////////////////////////////////////////////////////
void monster_cryotech (userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_CRYOTECH);

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_CRYOTECH];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
	// register all the animation sounds
	ai_register_sounds(self);

	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);


	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = cryotech_begin_attack;
	hook->fnAttackFunc		= cryotech_attack;
	self->pain				= cryotech_start_pain;
	self->die				= cryotech_start_die;

	hook->run_speed			= 200;
	hook->walk_speed		= 55;
	hook->attack_dist		= 256;

	self->health			= 75;
	hook->base_health		= 100;
	hook->pain_chance		= 25;
	
	self->gravity			= 1.0;
	self->ang_speed.y		= 150;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	give cryotech its melee weapon
	//////////////////////////////////////////////

	self->inventory = gstate->InventoryNew (MEM_MALLOC);

	if ( pWeaponAttributes )
	{
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "cryo spray",
										  cryo_spray,
										  ITF_PROJECTILE|ITF_NOLEAD);
		gstate->InventoryAddItem(self,self->inventory,self->curWeapon);
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

void world_ai_cryotech_register_func()
{
	gstate->RegisterFunc( "cryotech_begin_attack", cryotech_begin_attack );
	gstate->RegisterFunc( "cryotech_attack", cryotech_attack );
	gstate->RegisterFunc( "cryotech_start_pain", cryotech_start_pain );
	gstate->RegisterFunc( "cryotech_start_die", cryotech_start_die );
	gstate->RegisterFunc( "AI_ParseEpairs", AI_ParseEpairs );
}
