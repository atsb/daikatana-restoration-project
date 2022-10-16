// ==========================================================================
//
//  File:       mishimaguard.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
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

#define MONSTERNAME			     "monster_mishimaguard"
#define MISHIMAGUARD_ATTACKSTR_1 "pistol"
#define MISHIMAGUARD_AMMOCOUNT   8


/* ******************************* exports ********************************* */
DllExport void monster_mishimaguard( userEntity_t *self );

void MISHIMAGUARD_StartReload( userEntity_t *self );
void MISHIMAGUARD_Reload( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		AI_MishimaguardTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_MishimaguardTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10 ) )
		{
			ai_fire_curWeapon( self );
			hook->attack_finished = gstate->time + 2.0f;	//	won't attack for another 2 seconds
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
// NSS[1/8/00]:
// Name:		mishimaguard_start_pain
// Description: pain function for mishimaguard
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MishimaguardTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MishimaguardTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_MishimaguardTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}
// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_add_muzzle_light
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_add_muzzle_light( userEntity_t * self )
{
    // create track light
    ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
    CVector light_org, ang;
    trackInfo_t tinfo;

    // determine forward, right, up
	// SCG[10/26/99]: monsters do not get a client struct
//    ang = self->client->v_angle;
    ang = self->s.angles;
    ang.AngleToVectors(forward, right, up);

    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent			= self;
    tinfo.srcent		= self;
    //tinfo.dstent		= self;
    //tinfo.modelindex	= 0;
    //tinfo.numframes	= 10;
    //tinfo.frametime	= 1;
    //tinfo.scale		= 1;

    //tinfo.lightColor.Set(1.0,1.0,0.5);
    tinfo.lightColor.Set(1.0,0.0,0.0);       // temp RED light

    tinfo.lightSize	= 175;

    // set light at weapon origin
    tinfo.fru = self->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;
    //tinfo.fru = self->s.origin + (weapon->ofs.x) + (weapon->ofs.y) + (weapon->ofs.z);

    tinfo.flags		= TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_FRU;
    tinfo.fxflags		= TEF_LIGHT;
    com->trackEntity(&tinfo,MULTICAST_ALL);

}

// ----------------------------------------------------------------------------
//
// Name:        MISHIMAGUARD_StartReload
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MISHIMAGUARD_StartReload( userEntity_t *self )
{
    AI_Dprintf( "Starting TASKTYPE_MISHIMAGUARD_RELOADING\n" );

    AI_ForceSequence(self, "reload");
	
}

// ----------------------------------------------------------------------------
//
// Name:        MISHIMAGUARD_Reload
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MISHIMAGUARD_Reload( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    //NSS[11/22/99]:Added for reload sound.
	if(self->s.frame >= 256 && self->s.frame < 258)
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/i_scammo.wav"),0.75f, 256.0f, 512.0f );
	// done reloading animation seq?
    if (AI_IsEndAnimation( self ))
    {
       hook->fired = MISHIMAGUARD_AMMOCOUNT; // restock
    
	   AI_RemoveCurrentTask( self );

       // delay time on next attack after reload
       hook->fAttackTime = gstate->time + 0.5f + rnd();
	}
}

// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_outofammo
// Description:
// Input:
// Output:
// Note:
//              reload function
//
// ----------------------------------------------------------------------------
void mishimaguard_outofammo( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

    //gstate->Con_Dprintf( "mishimaguard out of ammo\n" );

    // add a reloading task
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_MISHIMAGUARD_RELOAD )
    {
        AI_AddNewTaskAtFront( self, TASKTYPE_MISHIMAGUARD_RELOAD );
        return;
    }
}

// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_set_attack_seq( userEntity_t *self )
{
	_ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

    _ASSERTE( AI_IsAlive( self->enemy ) );

    if ( hook->fired <= 0 )            // has ammo to fire?
    {
        mishimaguard_outofammo(self);  // ohshit... reload!
    }
    else
    {
        if ( gstate->time > hook->fAttackTime &&
             AI_IsFacingEnemy( self, self->enemy ) && 
             AI_IsClearShot( self, self->enemy ) 
           )
        {
            float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

            // enemy within attack distance?
            if ( AI_IsWithinAttackDistance( self, dist ) )
            {
                frameData_t *pSequence = NULL;

                int nFlag = 0;
                if ( (hook->fired % 4) == 0 )  // time for new position? (every 4 shots)
                {
                    nFlag = rand() % 3;
                }

                switch ( nFlag )
                { 
                    case 0 : pSequence = FRAMES_GetSequence( self, "ataka" ); break;
                    case 1 : pSequence = FRAMES_GetSequence( self, "atakb" ); break;
                    case 2 : pSequence = FRAMES_GetSequence( self, "atakc" ); break;
                    default: break;
                        
                }

//                if ( AI_StartSequence(self, pSequence) == FALSE )
//                {
//                    return;
//                }

                AI_ForceSequence( self, pSequence );

                // set current weapon
                self->curWeapon = gstate->InventoryFindItem(self->inventory, MISHIMAGUARD_ATTACKSTR_1);

                // TTD: need to add an attack_rate to the .csv!
                hook->fAttackTime = gstate->time + 0.2f + rnd();
                hook->nAttackMode = ATTACKMODE_RANGED;
            }
        }
        else
        {
            AI_FaceTowardPoint( self, self->enemy->s.origin );
        }  
    }
}

// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_do_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_do_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( !AI_CanAttackEnemy( self ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

    // sequence was wiped out?  restart attack sequence
 	if ( !hook->cur_sequence || !strstr( hook->cur_sequence->animation_name, "atak" ) )
	{
        mishimaguard_set_attack_seq( self );
        return;
	}
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );


	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) && hook->fired > 0) // has ammo to fire?
    {
	    AI_PlayAttackSounds( self );
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10 ) )
		{
//            self->client->v_angle = self->s.angles;
            ai_fire_curWeapon( self );
            //mishimaguard_add_muzzle_light(self);

            hook->fired--; // reduce fake ammo count
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
    }

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		//	finished one attack, decide what to do next
		AI_RemoveCurrentTask(self,FALSE);
	}

}

// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_attack( userEntity_t *self )
{
    //gstate->Con_Dprintf( "mishimaguard_attack()\n" );
	playerHook_t *hook  = AI_GetPlayerHook(self); 

    if( hook->pain_finished < gstate->time)
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

	    _ASSERTE( AI_IsAlive( self->enemy ) );

	    if ( hook->nAttackMode != ATTACKMODE_RANGED )
		{
		    mishimaguard_set_attack_seq( self );
		}

		_ASSERTE( AI_IsAlive( self->enemy ) );

		mishimaguard_do_attack(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        mishimaguard_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mishimaguard_begin_attack( userEntity_t *self )
{
    
	playerHook_t *hook  = AI_GetPlayerHook(self); 
	if( hook->pain_finished < gstate->time)
		AI_StopCurrentSequence( self );
	self->velocity.Zero();

	mishimaguard_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:        monster_mishimaguard
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_mishimaguard( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_MISHIMAGUARD);

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_MISHIMAGUARD];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
	ai_register_sounds(self);  // register all the animation sounds

	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	hook->fnStartAttackFunc = mishimaguard_begin_attack;
	hook->fnAttackFunc		= mishimaguard_attack;
	hook->fnTakeCoverFunc	= AI_MishimaguardTakeCover;

	hook->nAttackType       = ATTACK_GROUND_RANGED;
	hook->nAttackMode       = ATTACKMODE_NORMAL;
    hook->dflags			|= DFL_RANGEDATTACK;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;
	self->pain				= mishimaguard_start_pain;

//	// amw: right now zero means don't look at anyone
//	self->s.numClusters = 0 ;
//	self->s.renderfx |= RF_CLUSTERBASED;

    // read in attributes from .csv file
    WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	give cryotech its melee weapon
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew( MEM_MALLOC );

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
										  MISHIMAGUARD_ATTACKSTR_1,
										  pistol_fire,
										  ITF_BULLET | ITF_NOLEAD); // no lead in this bullet

		gstate->InventoryAddItem(self,self->inventory,self->curWeapon);

        // use this variable to track fake ammo for fake weapon
        hook->fired = MISHIMAGUARD_AMMOCOUNT;
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

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_mishimaguard_register_func()
{
	gstate->RegisterFunc( "mishimaguard_begin_attack", mishimaguard_begin_attack );
	gstate->RegisterFunc( "mishimaguard_attack", mishimaguard_attack );
	gstate->RegisterFunc( "AI_MishimaguardTakeCover", AI_MishimaguardTakeCover );
	gstate->RegisterFunc( "AI_ParseEpairs", AI_ParseEpairs );
	gstate->RegisterFunc( "mishimaguard_start_pain", mishimaguard_start_pain );
}
