// ==========================================================================
//
//  File:
//  Contents:
//	the current start and end path_corner_track's for the gun are stored in the
//	tgunHook_s that is pointed to by hook->trackHook
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "chasecam.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "GrphPath.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

DllExport	void	monster_trackattack (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
// define
///////////////////////////////////////////////////////////////////////////////////

#define MONSTERNAME			"monster_trackattack"
#define MODELNAME_MID		"models/e1/m_tatakmid.dkm"
#define MODELNAME_TOP		"models/e1/m_tataktop.dkm"

//static MAPNODE_PTR pLastNode = NULL;

// ----------------------------------------------------------------------------
//
// Name:		DistanceFromPointToLineSegment
// Description:
// Input:
//				c		=> point
//				a, b	=> two end points that make a line segment
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float DistanceFromPointToLineSegment( CVector c, CVector a, CVector b )
{
	float L = VectorXYDistance( a, b );
	float LSquare = L * L;

	//float r = (((a.y-c.y) * (a.y-b.y)) - ((a.x-c.x) * (b.x-a.x))) / LSquare;

	float s = (((a.y-c.y) * (b.x-a.x)) - ((a.x-c.x) * (b.y-a.y))) / LSquare;

	float fDistance = fabs(s * L);

	return fDistance;
}

// ----------------------------------------------------------------------------

static int nNumTrackNodes = 0;
static short aTrackNodeIndices[100];

int AddTrackNode( MAPNODE_PTR pNode )
{
	int bFound = FALSE;
	for ( int i = 0; i < nNumTrackNodes; i++ )
	{
		if ( pNode->nIndex == aTrackNodeIndices[i] )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( !bFound )
	{
		aTrackNodeIndices[nNumTrackNodes++] = pNode->nIndex;
	}

	return bFound;
}

// ----------------------------------------------------------------------------

void _FindAllTrackNodes( MAPNODE_PTR pNode )
{
	for ( int i = 0; i < pNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pNewNode = NODE_GetNode( pTrackNodes, pNode->aLinks[i].nIndex );

		if ( !AddTrackNode( pNewNode ) )
		{
			_FindAllTrackNodes( pNewNode );
		}
	}
}

// ----------------------------------------------------------------------------

void FindAllTrackNodes( MAPNODE_PTR pNode )
{
	nNumTrackNodes = 0;

	_FindAllTrackNodes( pNode );
}

// ----------------------------------------------------------------------------
//
// Name:		trackattack_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> ranged attack
//				atakb	=> melee attack
//
// ----------------------------------------------------------------------------
static void trackattack_set_attack_seq(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	if ( AI_StartSequence( self, pSequence ) == FALSE )
	{
		return;
	}
}

// ----------------------------------------------------------------------------

static void trackattack_FindClosestTwoNodes( userEntity_t *self, MAPNODE_PTR &pNode1, MAPNODE_PTR &pNode2 )
{
    float fCombinedMinDistance = 1e20;
	pNode1 = NULL;
	pNode2 = NULL;
	for ( int i = 0; i < nNumTrackNodes; i++ )
	{
		int nNodeIndex = aTrackNodeIndices[i];
		MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, nNodeIndex );

		for ( int j = 0; j < pNode->nNumLinks; j++ )
		{
			if ( pNode->aLinks[j].nIndex != -1 )
            {
                MAPNODE_PTR pLinkNode = NODE_GetNode( pTrackNodes, pNode->aLinks[j].nIndex );
                _ASSERTE( pLinkNode );
				
				float fDistanceToLine = DistanceFromPointToLineSegment( self->s.origin, pNode->position, pLinkNode->position );
				if ( fDistanceToLine < fCombinedMinDistance )
				{
					pNode1 = pNode;
					pNode2 = pLinkNode;
					fCombinedMinDistance = fDistanceToLine;
				}
			}
        }
	}
}

// ----------------------------------------------------------------------------

static MAPNODE_PTR trackattack_FindClosestTwoNodes( userEntity_t *self )
{
    float fMinDistance = 1e20;
	MAPNODE_PTR pClosestNode = NULL;
	for ( int i = 0; i < nNumTrackNodes; i++ )
	{
		int nNodeIndex = aTrackNodeIndices[i];
		MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, nNodeIndex );
		float fDistance = VectorDistance( self->s.origin, pNode->position );
		if ( fDistance < fMinDistance )
		{
			pClosestNode = pNode;
			fMinDistance = fDistance;
		}
	}

    return pClosestNode;
}

// ----------------------------------------------------------------------------

static MAPNODE_PTR trackattack_FindClosestNode( userEntity_t *self, MAPNODE_PTR pNode1, MAPNODE_PTR pNode2 )
{
    float fDistance1 = VectorDistance( self->s.origin, pNode1->position );
    float fDistance2 = VectorDistance( self->s.origin, pNode2->position );
    if ( fDistance1 < fDistance2 )
    {
        return pNode1;
    }

    return pNode2;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TrackUpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void AI_TrackUpdateAttackMovement( userEntity_t *self )
{
	AI_ZeroVelocity( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
    {
        return;
    }

    ai_frame_sounds( self );

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	float fSpeed = hook->attack_speed;
	float fMovementLengthInOneUpdate = fSpeed * fScale * 0.1f;

	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pTrackNodes );
	
		MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, hook->pPathList->pPath->nNodeIndex );
		
		CVector destPoint = pNode->position;
        float fDistance = VectorDistance( self->s.origin, pNode->position );
	    if ( AI_IsCloseDistance( hook, fDistance ) )
	    {
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath( hook->pPathList );

            if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
            {
                pNode = NODE_GetNode( pTrackNodes, hook->pPathList->pPath->nNodeIndex );
                destPoint = pNode->position;
            }
            else
            {
                destPoint.Zero();
            }
        }

		if ( destPoint.Length() > 0.0f )
        {
            CVector vector = destPoint - self->s.origin;
            vector.Normalize();
            AI_SetVelocity( self, vector, (fSpeed * fScale) );

			hook->last_origin = self->s.origin;
        }

        return;
    }

	MAPNODE_PTR pClosestTrackNode = NODE_FindClosestTrackNode( self );
	if ( !pClosestTrackNode )
	{
		return;
	}

	float fDistanceToEnemy = VectorDistance( self->s.origin, pEnemy->s.origin );
	
	float fDistance = VectorDistance( self->s.origin, pClosestTrackNode->position );
	if ( AI_IsCloseDistance( hook, fDistance ) )
	{
		for ( int i = 0; i < pClosestTrackNode->nNumLinks; i++ )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, pClosestTrackNode->aLinks[i].nIndex );

			CVector vector = pNode->position - self->s.origin;
			vector.Normalize();

			CVector tempPos;
            VectorMA( self->s.origin, vector, fMovementLengthInOneUpdate, tempPos );
			fDistance = VectorDistance( tempPos, pEnemy->s.origin );
			if ( fDistance < fDistanceToEnemy )
			{
				AI_SetVelocity( self, vector, (fSpeed * fScale) );
				hook->last_origin = self->s.origin;
			
				break;
			}
		}
	}
	else
	{
		FindAllTrackNodes( pClosestTrackNode );
		
		MAPNODE_PTR pNode1 = NULL;
		MAPNODE_PTR pNode2 = NULL;
        trackattack_FindClosestTwoNodes( self, pNode1, pNode2 );
        _ASSERTE( pNode1 );

		// find two track nodes that the enemy is closest to
		MAPNODE_PTR pNode3 = trackattack_FindClosestTwoNodes( pEnemy );

        if ( pNode1 == pNode3 || pNode2 == pNode3 )
        {
            // both self and enemy are close to the same two track nodes
            CVector vector = pNode1->position - pNode2->position;
			vector.Normalize();

			CVector tempPos;
            VectorMA( self->s.origin, vector, fMovementLengthInOneUpdate, tempPos );
			fDistance = VectorDistance( tempPos, pEnemy->s.origin );
			
			int bFound = FALSE;
			if ( fDistance < fDistanceToEnemy )
			{
				bFound = TRUE;
			}
			else
			{
				vector.Negate();

				CVector tempPos;
                VectorMA( self->s.origin, vector, fMovementLengthInOneUpdate, tempPos );
				fDistance = VectorDistance( tempPos, pEnemy->s.origin );
				if ( fDistance < fDistanceToEnemy )
				{
					bFound = TRUE;
				}
			}

			if ( bFound )
			{
				AI_SetVelocity( self, vector, (fSpeed * fScale) );
				hook->last_origin = self->s.origin;
			}
        }
        else
        {
	        MAPNODE_PTR pCurrentNode = trackattack_FindClosestNode( self, pNode1, pNode2 );
	        MAPNODE_PTR pGoalNode = pNode3;

	        int nPathLength = PATH_ComputePath(self, pTrackNodes, pCurrentNode, pGoalNode, hook->pPathList);
	        if ( !hook->pPathList->pPath )
	        {
		        return;
	        }

		    MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, hook->pPathList->pPath->nNodeIndex );
		    
		    CVector destPoint = pNode->position;
            CVector vector = destPoint - self->s.origin;
            vector.Normalize();
            AI_SetVelocity( self, vector, (fSpeed * fScale) );

			hook->last_origin = self->s.origin;

        }
	}

}

// ----------------------------------------------------------------------------
//
// Name:		trackattack_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trackattack_attack( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
    {
        return;
    }

    AI_FaceTowardPoint( self, pEnemy->s.origin );
	AI_UpdatePitchTowardEnemy( self );
	AI_TrackUpdateAttackMovement( self );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		float dist = VectorDistance(self->s.origin, pEnemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, pEnemy) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			trackattack_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		trackattack_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trackattack_begin_attack (userEntity_t *self)
{
	AI_ZeroVelocity( self );

	trackattack_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		monster_trackattack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_trackattack( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_TRACKATTACK );

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_TRACKATTACK];

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

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 90, 90 );

	self->movetype			= MOVETYPE_TRACK;
	hook->dflags			|= DFL_MECHANICAL;
	hook->dflags			|= DFL_RANGEDATTACK;
	hook->dflags			|= DFL_FACEPITCH;
    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts

	hook->fnStartAttackFunc = trackattack_begin_attack;
	hook->fnAttackFunc		= trackattack_attack;

	hook->run_speed			= 160;
	hook->walk_speed		= 80;
	hook->attack_speed		= 250;

	hook->active_distance	= 3000;
	hook->attack_dist		= 768;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	self->mass				= 2.0;
	hook->bOkToAttack		= FALSE;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

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
										  "laser", 
										  laser_fire, 
										  ITF_PROJECTILE | ITF_NOLEAD );
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );
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

