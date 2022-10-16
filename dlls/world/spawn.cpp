// ==========================================================================
//
//  File:		NavTest
//  Contents:
//				Test a map's navigation by spawning a monster and have it 
//				roam around
//  Author:
//
// ==========================================================================
// Logic[5/25/99]: #include <windows.h>
#include "dk_system.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
//#include "MonsterSound.h"// SCG[1/23/00]: not used
#include "ai.h"
#include "collect.h"
#include "action.h"
#include "Sidekick.h"
#include "ai_think.h"
#include "actorlist.h"

/* ***************************** define types ****************************** */
typedef void (*ai_func) ( userEntity_t *self );


/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

// ----------------------------------------------------------------------------

CRespawnData::CRespawnData()
{
	szScriptAction = NULL;
	bRespawnAfterDeath = FALSE;
}

CRespawnData::CRespawnData( const CVector &newSpawnPoint, char *szNewScriptAction, int bRespawnFlag )
{
	spawnPoint = newSpawnPoint;
	szScriptAction = szNewScriptAction;
	bRespawnAfterDeath = bRespawnFlag;
}

// ----------------------------------------------------------------------------
//								SPAWN functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// NSS[1/25/00]:Made changes... like reduced the code down to 1/3rd its size.
// Name:		SPAWN_FindNearSpawnPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define SPAWN_DISTANCE  96.0f

int SPAWN_FindNearSpawnPoint( userEntity_t *self, CVector &spawnPoint )
{
	// search the eight directions around the self to find a spawn point
	float delta_angle = 0.0f;
	CVector angle = self->s.angles;
	angle.pitch = -5.0f;

	CVector vector;
	angle.AngleToForwardVector( vector );
	CVector endPoint;

	while(delta_angle < 360)
	{
		VectorMA( self->s.origin, vector, SPAWN_DISTANCE, endPoint );
		endPoint.z += 32.0f;
		if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
		{
			CVector bottomPoint = endPoint;
			bottomPoint.z -= 1.0f;
			tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
			if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)// && !tr.ent )
			{
				spawnPoint = endPoint;
				return TRUE;
			}
		}
		delta_angle += 45.0f;
		angle.yaw = AngleMod( angle.yaw + delta_angle );
		angle.AngleToForwardVector( vector );
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SPAWN_CallInitFunction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_CallInitFunction( userEntity_t *self, const char *szAIClassName )
{
	HMODULE hModule;
#ifdef WIN32
    hModule = GetModuleHandle( "world.dll" );
	if ( hModule )
	{
		ai_func pAIFunc = (ai_func)GetProcAddress( hModule, szAIClassName );
#else
    hModule = dlopen("world.so", RTLD_NOW);
    if(hModule)
    {
        ai_func pAIFunc = (ai_func)dlsym(hModule, szAIClassName);
#endif

		if ( pAIFunc )
		{
			pAIFunc( self );
		}
	}
#ifndef WIN32
    dlclose("word.so");
#endif
//NSS[11/3/99]: This should be done INSIDE of the initialization function for the monster being spawned.
//otherwise you will get an error going into this function without having something intialized... 
//primarily the userhook.
//    AI_InitNodeList( self );       
}

// ----------------------------------------------------------------------------
//
// Name:		SPAWN_AI
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_AI( userEntity_t *self, const char *szAIClassName )
{
	char szClassName[32];
	if ( szAIClassName == NULL )
	{
		strcpy( szClassName, "monster_bot" );
	}
	else
	{
		strcpy( szClassName, szAIClassName );
	}

	if ( _stricmp( szClassName, "monster_bot" ) == 0 )
	{
		bot_spawn( self );
		return;
	}
	else
	if ( _stricmp( szClassName, "Hiro" ) == 0 )
	{
		SIDEKICK_SpawnHiro( self );
		return;
	}
	else
	if ( _stricmp( szClassName, "SuperFly" ) == 0 )
	{
		SIDEKICK_SpawnSuperfly( self );
		return;
	}
	else
	if ( _stricmp( szClassName, "Mikiko" ) == 0 )
	{
		SIDEKICK_SpawnMikiko( self );
		return;
	}


	userEntity_t *pMonster = gstate->SpawnEntity();

	// look for place to put player

	CVector spawnPoint;
	int bFoundSpot = SPAWN_FindNearSpawnPoint( self, spawnPoint );
	if ( !bFoundSpot )
	{
		com->Warning( "No info_player_start spawn points." );
		return;
	}
	pMonster->s.origin = spawnPoint;
	pMonster->s.angles = self->s.angles;

	char szModelName[64];
	int nType = GetMonsterInfo( szClassName, szModelName );
	if ( nType < 0 )
	{
		com->Warning( "Monster %s was not found", szClassName );
		return;
	}
	
	SPAWN_CallInitFunction( pMonster, szClassName );

}

// ----------------------------------------------------------------------------
//
// Name:		SPAWN_AI
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_AI( userEntity_t *self, const char *szClassName, const char *szUniqueId, const CVector &spawnPoint, 
			   const CVector &facingAngle, int bRespawnAfterDeath, char *szScriptAction, char *szSkin )
{
	_ASSERTE( szClassName != NULL );

	userEntity_t *pMonster = NULL;
	if ( _stricmp( szClassName, "Hiro" ) == 0 )
	{
		pMonster = SIDEKICK_SpawnHiro( spawnPoint, facingAngle );
	}
	else
	if ( _stricmp( szClassName, "SuperFly" ) == 0 )
	{
		pMonster = SIDEKICK_SpawnSuperfly( spawnPoint, facingAngle );
	}
	else
	if ( _stricmp( szClassName, "Mikiko" ) == 0 )
	{
		pMonster = SIDEKICK_SpawnMikiko( spawnPoint, facingAngle );
	}
	else
	{
		pMonster = gstate->SpawnEntity();
		_ASSERTE( pMonster );

		pMonster->s.origin = spawnPoint;
		pMonster->s.angles = facingAngle;

		char szModelName[64];
		int nType = GetMonsterInfo( szClassName, szModelName );
		if( strstr( szClassName, "cine_" ) == 0 )
		{
			if ( nType < 0 )
			{
				com->Warning( "Monster %s was not found", szClassName );
				return;
			}
		}
		
		SPAWN_CallInitFunction( pMonster, szClassName );
	}

	UNIQUEID_Add( szUniqueId, pMonster );

	if ( bRespawnAfterDeath )
	{
		playerHook_t *hook = AI_GetPlayerHook( pMonster );
		_ASSERTE( hook );
		
		hook->pRespawnData = new CRespawnData( spawnPoint, szScriptAction, bRespawnAfterDeath );
		if ( !hook->pRespawnData )
		{
			com->Error( "MEM Allocation failed" );
		}
	}

	if ( szSkin )
    {
		pMonster->s.skinnum = gstate->ImageIndex( szSkin );
		pMonster->s.effects2 |= EF2_IMAGESKIN;
    }

    if ( szScriptAction )
	{
		AI_AddScriptActionGoal( pMonster, szScriptAction );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SPAWN_Respawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_Respawn( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !hook->pRespawnData )
	{
		return;
	}

	CRespawnData *pTempRespawnData = new CRespawnData( hook->pRespawnData->GetSpawnPoint(),
				 									   hook->pRespawnData->GetScriptAction(),
													   hook->pRespawnData->GetRespawnFlag() );
	if ( !pTempRespawnData )
	{
		com->Error( "MEM Allocation failed" );	
	}

	self->s.origin = hook->pRespawnData->GetSpawnPoint();

	SPAWN_CallInitFunction( self, self->className );

	hook = AI_GetPlayerHook( self );
	hook->pRespawnData = pTempRespawnData;
	_ASSERTE( hook->pRespawnData );

	char *szScriptAction = hook->pRespawnData->GetScriptAction();
	if ( szScriptAction )
	{
		AI_AddScriptActionGoal( self, szScriptAction );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SPAWN_RespawnThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_RespawnThink( userEntity_t *self )
{
	SPAWN_Respawn( self );
}

// ----------------------------------------------------------------------------
//
// Name:        SPAWN_Remove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_Remove( const char *szUniqueId )
{
	userEntity_t *pEntity = UNIQUEID_Lookup( szUniqueId );
	if ( AI_IsAlive( pEntity ) )
    {
    	playerHook_t *hook = AI_GetPlayerHook( pEntity );
		
        pEntity->pain		= NULL;
		pEntity->use		= NULL;
		pEntity->die		= NULL;
		pEntity->think		= NULL;
		pEntity->prethink	= NULL;
		pEntity->postthink  = NULL;

		if ( hook->pGoals )
		{
			GOALSTACK_Delete( hook->pGoals );
			hook->pGoals = NULL;
		}
		if ( hook->pScriptGoals )
		{
			GOALSTACK_Delete( hook->pScriptGoals );
			hook->pScriptGoals = NULL;
		}
		
		UNIQUEID_Remove( hook->szScriptName );
		free( hook->szScriptName );
		hook->szScriptName = NULL;

	    alist_remove( pEntity );
	    pEntity->remove( pEntity );
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

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_spawn_register_func()
{
	gstate->RegisterFunc("SPAWN_RespawnThink",SPAWN_RespawnThink);
}
