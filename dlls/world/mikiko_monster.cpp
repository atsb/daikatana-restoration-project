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
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_mikiko (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		mikiko_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mikiko_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence;
		switch( rand() % 3 )
		{
			case 0:
				{
					pSequence = FRAMES_GetSequence( self, "ataka" );
					hook->acro_boost = 0;
					break;
				}
			case 1:
				{
					pSequence = FRAMES_GetSequence( self, "atakb" );
					hook->acro_boost = 1;
					break;
				}
			case 2:
				{
					pSequence = FRAMES_GetSequence( self, "atakc" );
					hook->acro_boost = 2;
					break;
				}
			// SCG[1/23/00]: make sure pSequence gets set!
			default:
				{
					pSequence = FRAMES_GetSequence( self, "ataka" );
					hook->acro_boost = 0;
					break;
				}
		}
		AI_ForceSequence(self, pSequence);
    }
}

// ----------------------------------------------------------------------------
//
// Name:		mikiko_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mikiko_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_Dprintf("Frame:%d\n",self->s.frame);
	switch((int)hook->acro_boost)
	{
		case 0:
			{
				if(self->s.frame >= 40 && self->s.frame <=41)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_swordwhoosha.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				break;
			}
		case 1:
			{
				
				if(self->s.frame >= 52 && self->s.frame <=53)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("mikiko/jump5.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				if(self->s.frame >= 58 && self->s.frame <=59)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_swordwhooshf.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				break;
			}
		case 2:
			{
				
				if(self->s.frame >= 70 && self->s.frame <=71)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("mikiko/jump8.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				if(self->s.frame >= 73 && self->s.frame <=74)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_swordwhooshc.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				if(self->s.frame >= 83 && self->s.frame <=84)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_swordwhooshd.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				break;
			}

	}
	
	if(AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() >= 0.50f)
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_DODGE);
	}
	if(AI_IsFacingEnemy(self,self->enemy))
	{
		if ( AI_IsReadyToAttack1( self ) )
		{
			ai_fire_curWeapon( self );
		}
		else if (AI_IsReadyToAttack2( self ) )
		{
			ai_fire_curWeapon( self );
		}
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_SetOkToAttackFlag( hook, TRUE );
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			mikiko_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		mikiko_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mikiko_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	mikiko_set_attack_seq( self );
	AI_SetOkToAttackFlag( hook, FALSE );
}


// ----------------------------------------------------------------------------
//
// Name:		mikiko_start_pain
// Description: pain function for sdiver that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mikiko_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		mikiko_start_die
// Description: die function for mikiko
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mikiko_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	// stop the ambient			
	self->s.sound = 0;

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
		self->think			= NULL;
		self->s.renderfx	|= RF_NODRAW|SPR_ALPHACHANNEL;
		self->s.alpha		= 0.0f;
	}

}

void MM_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(self->enemy)
	{
		if(AI_IsVisible(self,self->enemy) && hook->fatigue==0)
		{
			//Attach the Sprite Light&Glow and fire stuff 
			trackInfo_t tinfo;

			// clear this variable
			memset(&tinfo, 0, sizeof(tinfo));

			tinfo.ent			= self;
			tinfo.srcent		= self;
			tinfo.modelindex = gstate->ModelIndex("models/global/e_flare4+.sp2");		// sword flare
			tinfo.modelindex2	= gstate->ModelIndex("models/global/e_flare4xo.sp2");		// eye flare
			tinfo.lightColor.Set(1,0.1,0.1);											// fx color!
			tinfo.altpos2.Set(1+frand(),-1,-1);		// neg. light color
			tinfo.Long1 = ART_DAIKATANA;
			tinfo.scale = 0.2;
			tinfo.scale2 = 0.3;

			tinfo.flags |= TEF_SCALE|TEF_SCALE2|TEF_MODELINDEX|TEF_MODELINDEX2|TEF_FXFLAGS|TEF_LONG1|TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_ALTPOS2;
			tinfo.fxflags |= TEF_ARTIFACT_FX|TEF_FX_ONLY;
						
			com->trackEntity(&tinfo,MULTICAST_PVS);
			hook->fatigue = 1;

			// set up the ambient			
			self->s.sound = gstate->SoundIndex("global/we_dk_03a.wav"); 
			self->s.volume = 0.7;
		}
		else if (!(AI_IsVisible(self,self->enemy)) && hook->acro_boost == 1)
		{
			RELIABLE_UNTRACK(self);
			hook->acro_boost = 0;
		}
	}

	AI_TaskThink( self);}

void MM_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = MM_Think;
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		monster_mikiko
// Description: The end monster... Mikiko... ? Woah... 
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_mikiko( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_MIKIKOMON );

	self->className			= "monster_mikiko";
	self->netname			= tongue_monsters[T_MONSTER_MIKIKOMON];

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

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	////////////////	///////////////////////////////////////////

	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );


	hook->fnStartAttackFunc = mikiko_begin_attack;
	hook->fnAttackFunc		= mikiko_attack;

	self->die				= mikiko_start_die;
	self->pain				= mikiko_start_pain;
	hook->pain_chance		= 10;

//	self->think			= AI_ParseEpairs;
	self->think			= MM_ParseEpairs;
	self->nextthink		= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give sdiver a weapon
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
										  "swing", 
										  melee_swing, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType	= ATTACK_GROUND_MELEE;
	hook->nAttackMode	= ATTACKMODE_NORMAL;

	hook->ai_flags		|= AI_CANSWIM;

    AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	gstate->LinkEntity(self);

	hook->acro_boost = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_mikikomon_register_func()
{
	gstate->RegisterFunc("mikiko_start_pain",mikiko_start_pain);
	gstate->RegisterFunc("mikiko_start_die",mikiko_start_die);
	gstate->RegisterFunc("mikiko_begin_attack",mikiko_begin_attack);
	gstate->RegisterFunc("mikiko_attack",mikiko_attack);
	gstate->RegisterFunc("mikiko_MM_ParseEpairs",MM_ParseEpairs);
	gstate->RegisterFunc("mikiko_MM_Think",MM_Think);
}
