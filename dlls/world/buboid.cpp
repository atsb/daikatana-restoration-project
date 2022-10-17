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
#include "ai_info.h"
#include "MonsterSound.h"
#include "actorlist.h"

/* ***************************** define types ****************************** */

#define BUBOID_HEAVY_HIT_DAMAGE			40
#define BUBOID_BLOWN_AWAY_MIN_DAMAGE	20
#define BUBOID_BLOWN_AWAY_ANGLECOS		0.707f
#define BUBOID_UNMELT_DELAY				3.0f
#define BUBOID_MELT_CHANCE				0.75f
#define BUBOID_RESURRECTION_DELAY		10.0f

#define BUBOID_MELT_DISTANCE_NEAR		250
#define BUBOID_MELT_DISTANCE_FAR		300

#define HOLY_GROUND						0x20000
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void buboid_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void buboid_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                       int damage, CVector &destPoint );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

void BUBOID_StartGetOutOfCoffin( userEntity_t *self );
void BUBOID_GetOutOfCoffin( userEntity_t *self );
void BUBOID_StartMelt( userEntity_t *self );
void BUBOID_Melt( userEntity_t *self );
void BUBOID_StartMelted( userEntity_t *self );
void BUBOID_Melted( userEntity_t *self );
void BUBOID_StartUnmelt( userEntity_t *self );
void BUBOID_Unmelt( userEntity_t *self );

bool BuboidEnemyClear( userEntity_t *enemy );

//void AI_Drop_Markers (CVector &Target, float delay);// SCG[1/23/00]: not used
/* ******************************* exports ********************************* */

DllExport void monster_buboid (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
// prototypes
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		BUBOID_GetOutOfCoffinThink
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BUBOID_GetOutOfCoffinThink( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_BUBOID_GETOUTOFCOFFIN )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_BUBOID_GETOUTOFCOFFIN );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_BUBOID_GETOUTOFCOFFIN.\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "speciala" );
	AI_ForceSequence(self, pSequence);

	self->think		= AI_TaskThink;
	self->nextthink	= gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:		BUBOID_StartGetOutOfCoffin
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BUBOID_StartGetOutOfCoffin( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	hook->ai_flags |= AI_IGNORE_PLAYER;

	// make sure that the buboid is loaded before starting the animation
	self->think		= BUBOID_GetOutOfCoffinThink;
	self->nextthink	= gstate->time + 0.0f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		BUBOID_GetOutOfCoffin
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BUBOID_GetOutOfCoffin( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->cur_sequence && strstr( hook->cur_sequence->animation_name, "special" ) && 
		 self->s.frameInfo.frameState & FRSTATE_LAST )
	{
		self->s.frameInfo.frameState -= FRSTATE_LAST;

		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_BUBOID_GETOUTOFCOFFIN )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );

		hook->ai_flags &= ~AI_IGNORE_PLAYER;
	}
}


// ----------------------------------------------------------------------------
//
// Name:        BUBOID_StartMelt
// Description: start melt task function for buboid
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_StartMelt( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_BUBOID_MELT.\n" );

	// play death sequence
	AI_StartSequence( self, "atakc" );

	AI_ZeroVelocity( self );

	self->takedamage = DAMAGE_NO;
	self->solid			= SOLID_NOT;
	self->s.renderfx	|= SPR_ALPHACHANNEL;
	self->s.alpha		= 0.80;
	
	hook->ai_flags |= AI_IGNORE_PLAYER;

	//Create the Special FX for the spawning
	trackInfo_t tinfo;
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent			=self;
	tinfo.srcent		=self;
	tinfo.fru.Zero();
	tinfo.Long1			= ART_BUBOID_MELT;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1;
	tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
	
	com->trackEntity(&tinfo,MULTICAST_ALL);

	gstate->LinkEntity(self);

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
}


// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_Melt
// Description: melt task function for buboid
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_Melt( userEntity_t *self )
{
	AI_ZeroVelocity( self );
	ai_frame_sounds( self );
	if(self->s.alpha > 0.05)
		self->s.alpha -= 0.25;
	if ( AI_IsEndAnimation( self ) )
	{
		// NSS[1/31/00]:Untrack the FX.
		RELIABLE_UNTRACK(self);

		AI_RemoveCurrentTask( self );
		self->s.renderfx |= RF_NODRAW;

		userEntity_t *pSuperfly = AIINFO_GetSuperfly();
		userEntity_t *pMikiko = AIINFO_GetMikiko();
	}
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_FindUnmeltPoint
// Description: start unmelt task function for buboid
// Input:       self
// Output:
// Note:Re-wrote this pile
//
// ----------------------------------------------------------------------------
int BUBOID_FindUnmeltPoint( userEntity_t *self )
{
	CVector Angles,Dir,Destination,Start,vMins,vMaxs;
	userEntity_t	*enemy;
	csurface_t		*groundSurface;
	trace_t			trace;
	
	float Delta_Angle = 0.0f;

	// SCG[1/24/00]: Check your muddafuggin pointers!!!!!
	if( self->enemy == NULL )
	{
		self->nextthink	= gstate->time + 1.0;
		return FALSE;
	}
	else
	{
		enemy = self->enemy;
	}
	// NSS[1/31/00]:Make sure they are on the ground
	if(enemy->groundSurface)
	{
		groundSurface = enemy->groundSurface;
		// NSS[1/31/00]:Make sure they are not on holy ground
		if(groundSurface->flags & HOLY_GROUND)
		{
			self->nextthink	= gstate->time + 3.0;
			return FALSE;
		}
	}
	else
	{
		self->nextthink	= gstate->time + 1.0;
		return FALSE;
	}

	//If we made it this far then all is clear lets look for a spot to come out of the ground.
	// Setup the mins maxs for the trace.
	vMins	= self->s.mins * 1.35;
	vMaxs	= self->s.maxs * 1.35;

	while(Delta_Angle < 360)
	{
		Angles.yaw		= Delta_Angle;
		Angles.pitch	= 0.0f;
		Angles.roll		= 0.0f;

		Angles.AngleToForwardVector(Dir);
		Start		= enemy->s.origin + (Dir * 32.0f);
		Destination = enemy->s.origin + (Dir * 64.0f);

		
		trace = gstate->TraceBox_q2( Start, vMins, vMaxs, Destination, self, MASK_MONSTERSOLID );
		
		if(trace.fraction >= 1.0f)
		{
			// Good spot, nowlet's get the closest node to be absolutely sure

			MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self, Destination);
			if(pCurrentNode)
			{
				Destination		= pCurrentNode->position;
				Destination.z	+= 32.0f;
				trace = gstate->TraceBox_q2( Destination, vMins, vMaxs, Destination, self, MASK_MONSTERSOLID );
				if(trace.fraction >=1.0f)
				{
					// NSS[1/31/00]:Let's unmelt!
					Destination.z	-= 16.0f;
					self->s.origin = Destination;
					return TRUE;
				}
			}

		}
		Delta_Angle += 45.0f;
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_StartMelted
// Description: start "melted" task function for buboid
// Input:       self
// ----------------------------------------------------------------------------
void BUBOID_StartMelted( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[1/31/00]:Turn off drawing, monster flags, and no solid please.
	self->s.renderfx	|= RF_NODRAW;
	self->flags			&= ~FL_MONSTER;
	self->solid			= SOLID_NOT;

	// NSS[1/31/00]:Link tha' bitch
	gstate->LinkEntity(self);

	AI_SetTaskFinishTime( hook, -1);
	AI_SetOkToAttackFlag( hook, FALSE );

	// NSS[1/31/00]:setup delay till un-melt 
	self->delay = gstate->time + (BUBOID_UNMELT_DELAY + (rand() % 4));
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_Melted
// Description: "melted" task function for buboid
// Input:       self
// ----------------------------------------------------------------------------
void BUBOID_Melted( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(self->delay < gstate->time)
	{
		if(BUBOID_FindUnmeltPoint(self))
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
		else
		{
			// NSS[1/31/00]:Every second or so check again.
			self->delay = gstate->time + 1.0f;
			ai_frame_sounds( self );
			hook->pMovingAnimation = NULL;
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_StartUnmelt
// Description: start unmelt task function for buboid
// Input:       self
// Note:
// ----------------------------------------------------------------------------

void BUBOID_StartUnmelt( userEntity_t *self )
{
	AI_Dprintf( "Starting TASKTYPE_BUBOID_UNMELT.\n" );

	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_ForceSequence( self, "atakd", FRAME_ONCE );
	
	AI_SetTaskFinishTime( hook, -1 );
	AI_SetOkToAttackFlag( hook, FALSE );
	
	self->s.renderfx &= ~RF_NODRAW;
	
	// NSS[1/31/00]:Set us back to being able to be shot and seen as a valid enemy
	self->s.renderfx	&= ~RF_NODRAW;
	self->solid			= SOLID_BBOX;
	//Create the Special FX for the spawning
	trackInfo_t tinfo;
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent			=self;
	tinfo.srcent		=self;
	tinfo.fru.Zero();
	tinfo.Long1			= ART_BUBOID_MELT;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1;
	tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
	
	com->trackEntity(&tinfo,MULTICAST_ALL);
	gstate->LinkEntity(self);
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        BUBOID_Unmelt
// Description: unmelt task function for buboid
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BUBOID_Unmelt( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );
	
	ai_frame_sounds( self );

	if(self->s.alpha < 1.0f)
		self->s.alpha += 0.20f;
	
	if(AI_IsEndAnimation( self ) )
	{
		// NSS[1/31/00]:Untrack the FX.
		RELIABLE_UNTRACK(self);
		self->s.alpha	= 1.0f;
		hook->ai_flags	&= ~AI_IGNORE_PLAYER;
		self->takedamage = DAMAGE_YES;
		self->flags		|= FL_MONSTER;
		AI_SetOkToAttackFlag( hook, TRUE );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );		
		// Goal complete, now go right back to killing enemy!
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );		
	}
	self->nextthink	= gstate->time + 0.2;
}


// ----------------------------------------------------------------------------
//
// Name:		buboid_set_attack_seq
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void buboid_set_attack_seq(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if( rand() % 3 <= 1 )
		AI_StartSequence( self, "atakb" );
	else
		AI_StartSequence( self, "ataka" );
}

// ----------------------------------------------------------------------------
//
// Name:		buboid_attack
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void buboid_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_PlayAttackSounds( self );

	if(self->solid == SOLID_NOT)
	{
		AI_Dprintf("This is bad...this is very bad.!\n");
		self->solid = SOLID_BBOX;
		gstate->LinkEntity(self);
	}
	
	self->takedamage = DAMAGE_YES;

	
	// NSS[2/21/00]:
	if(self->enemy)
	{
		float fDistance = VectorDistance(self->enemy->s.origin,self->s.origin);
		if(!AI_IsWithinAttackDistance(self,fDistance,self->enemy))
		{
			AI_RemoveCurrentTask(self,FALSE);
			return;
		}
	}
	
	if( AI_IsFacingEnemy( self, self->enemy ) )
	{
		if( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
			ai_fire_curWeapon( self );
	}
	
	if( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !AI_IsVisible(self, self->enemy))
        	AI_RemoveCurrentTask( self );
        else
			buboid_set_attack_seq (self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		buboid_begin_attack
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void buboid_begin_attack( userEntity_t *self )
{
	AI_StopCurrentSequence( self );
	AI_ZeroVelocity( self );

	buboid_set_attack_seq (self);
}

////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:        buboid_start_pain
// Description: "Pain of the Lycanthir"
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_GibLimit(userEntity_t *self, float damage);
void buboid_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALTYPE goaltype = GOALSTACK_GetCurrentGoalType( AI_GetCurrentGoalStack( hook ) );

	// NSS[2/6/00]:Make sure we get rid of the cloud... this means we were hit while coming up from the ground.
	if(hook->type == TYPE_BUBOID  && SIDEKICK_HasTaskInQue(self,TASKTYPE_BUBOID_UNMELT) || SIDEKICK_HasTaskInQue(self,TASKTYPE_BUBOID_MELT))
	{
		RELIABLE_UNTRACK(self);
		return;
	}

	if(hook)
	{
		// Is it the (gasp) silverclaw?
		if(AI_GibLimit(self, damage) )
		{
			// if self is playing dead, make self really dead
			if( goaltype == GOALTYPE_BUBOID_RESURRECT )
			{
				if( self != attacker )
					com->Damage( self, self, self, CVector(0, 0, 0), CVector(0, 0, 0), 32047, DAMAGE_NONE );
			}
			else
				AI_StartPain( self, attacker, kick, damage );
		}
		else
		{
			// check whether seeking resurrect goal
			if ( goaltype == GOALTYPE_BUBOID_RESURRECT )
			{
				// Is this an external attack?
				if ( attacker != self || damage < 32000 )
					self->health = 1.0;	// give ourselves a damage rebate
				else
					AI_StartPain( self, attacker, kick, damage );
			}
			else
			{

				ai_generic_pain_handler( self, attacker, kick,damage,BUBOID_HEAVY_HIT_DAMAGE);
				
				if( AI_IsAlive( self ) && self->enemy )
				{
					float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
					if( damage && rnd() < BUBOID_MELT_CHANCE && dist > BUBOID_MELT_DISTANCE_NEAR )
						AI_AddNewGoal( self, GOALTYPE_BUBOID_MELT );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        buboid_die_maybe
// Description: die function for buboid... maybe it'll die, maybe it won't
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void buboid_die_maybe( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
	// Is it the silverclaw or a self-gibbing [see buboid_start_pain()]?
	if( inflictor == self && damage >= 32000 )
	{
		int notdeadyet = (self->deadflag == DEAD_NO);

		// call AI_StartDie, which does generic dying stuff
		AI_StartDie( self, inflictor, attacker, damage, destPoint );

		if ( notdeadyet && self->deadflag == DEAD_DYING )
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
	else
	{ // fake own death
		playerHook_t *hook = AI_GetPlayerHook( self );
		GOALTYPE goaltype = GOALSTACK_GetCurrentGoalType( AI_GetCurrentGoalStack( hook ) );

		// check our goal and add resurrect goal if necessary
		if ( goaltype != GOALTYPE_BUBOID_RESURRECT )
		{
			// start resurrection goal
			AI_AddNewGoal( self, GOALTYPE_BUBOID_RESURRECT );

			// notify other enemies
			AI_EnemyAlert( self, attacker );
		}
		else
		{
			AI_StartDie( self, inflictor, attacker, 32047, destPoint );
			self->s.frame += 10;
		}

		// set health to its minimum
		self->health = 1.0;
	}
}


// ----------------------------------------------------------------------------
//
// Name:		buboid_start_die
// Description: die handler for buboid, selects falling or collapsing death
// Input:       self, inflictor, attacker, damage, destPoint
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void buboid_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                       int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	// pick a different sequence for different deaths
    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{
		if ( damage >= BUBOID_BLOWN_AWAY_MIN_DAMAGE &&
			 AI_GetDirDot( self, inflictor ) > BUBOID_BLOWN_AWAY_ANGLECOS )
			AI_ForceSequence( self, "dieb" ); // collapse
		else
			AI_ForceSequence( self, "diea" ); // fall over slowly
	}
}

// ----------------------------------------------------------------------------
//
// Name:        BUBOID_StartResurrectWait
// Description: function called when resurrect wait task starts
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_StartResurrectWait(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_BUBOID_RESURRECT_WAIT.\n" );

	// play death sequence
	AI_ForceSequence( self, "diea" );

//	self->s.maxs.z -= BUBOID_RESURRECT_BOX_HEIGHT_SHRINK;

	// not OK to attack while resurrecting
	AI_SetOkToAttackFlag( hook, FALSE );
	hook->fnStartAttackFunc = NULL;

	self->movetype = MOVETYPE_TOSS;

	AI_SetTaskFinishTime( hook, BUBOID_RESURRECTION_DELAY );
}


// ----------------------------------------------------------------------------
//
// Name:        BUBOID_StartResurrectWait
// Description: function called for resurrect wait task
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_ResurrectWait(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	ai_frame_sounds( self );
}


// ----------------------------------------------------------------------------
//
// Name:        BUBOID_StartResurrectWait
// Description: function called when resurrect task starts
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_StartResurrect(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ForceSequence( self, "speciala", FRAME_ONCE );

	AI_SetTaskFinishTime( hook, 10.0f );

	// set health to a healthy number
	self->health = hook->base_health;

	AI_Dprintf( "Starting TASKTYPE_BUBOID_RESURRECT.\n" );
	AI_SetOkToAttackFlag( hook, FALSE );
}


// ----------------------------------------------------------------------------
//
// Name:        BUBOID_Resurrect
// Description: function called for resurrect task
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void BUBOID_Resurrect(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	// Are we back on our feet?
	if ( AI_IsEndAnimation( self ) )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );

		// OK to attack again
		AI_SetOkToAttackFlag( hook, TRUE );
		hook->fnStartAttackFunc = buboid_begin_attack;

		// set health to a healthy number
		self->health = hook->base_health;

		// move as normal
	    self->movetype = hook->nOriginalMoveType;

		// Goal complete, now go right back to killing enemy!
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		BUBOID_ChaseThink
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool BUBOID_ChaseThink( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	self->nextthink	= gstate->time + 0.2;
	
	if(!(self->flags & FL_MONSTER))
	{
		// NSS[3/3/00]:Some weird reason we have reached this point... HACK.
		RELIABLE_UNTRACK(self);
		self->s.alpha	= 1.0f;
		hook->ai_flags	&= ~AI_IGNORE_PLAYER;
		self->takedamage = DAMAGE_YES;
		self->flags		|= FL_MONSTER;
		AI_SetOkToAttackFlag( hook, TRUE );
	}
	if( self->enemy )
	{
		
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );

		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if( dist > BUBOID_MELT_DISTANCE_FAR && !AI_IsVisible( self, self->enemy ))
		{
			GOAL_Satisfied( pCurrentGoal );
			AI_AddNewGoal( self, GOALTYPE_BUBOID_MELT );
			return true;
		}

		if( GOAL_GetType( pCurrentGoal ) != GOALTYPE_BUBOID_MELT )
		{
			CVector forward, point, endPt;
			self->s.angles.AngleToForwardVector( forward );
			point = self->s.origin + ( forward * 36.0f ); 
			endPt = point;
			endPt.z -= 200;
			//AI_Drop_Marker( self, point );
			trace_t trace = gstate->TraceLine_q2( point, endPt, self, MASK_SOLID );
			if( trace.surface && trace.surface->flags & HOLY_GROUND )
			{
				//AI_FaceTowardPoint( self, self->enemy->s.origin );
				AI_AddNewGoal( self, GOALTYPE_BUBOID_MELT );
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------------
//
// Name:		BuboidEnemyClear
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool BuboidEnemyClear( userEntity_t *enemy )
{
	if( !enemy )
		return false;

	float dist;
	int count = 0;

	userEntity_t *pMonster = alist_FirstEntity( monster_list );
	while( pMonster )
	{
		dist = VectorDistance( pMonster->s.origin, enemy->s.origin );
		if( dist < 120 )
			count++;
		pMonster = alist_NextEntity( monster_list );
	}

	if( count < 4 )
	{
		AI_Dprintf( "Good %d\n", count );
		return true;
	}
	else
	{
		AI_Dprintf( "Bad %d\n", count );
		return false;
	}
}

int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
// ----------------------------------------------------------------------------
// NSS[2/8/00]:
// Name:		Buboid_Think
// Description:Setup a trap to be able to monitor Buboid thinking
// Input:       userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Buboid_Think(userEntity_t *self)
{
	if(!AI_HasGoalInQueue(self,GOALTYPE_BUBOID_MELT) && !AI_HasGoalInQueue(self,GOALTYPE_BUBOID_RESURRECT) && self->s.alpha < 1.0f)
	{
		AI_AddNewGoal(self,GOALTYPE_BUBOID_MELT);
	}
	AI_TaskThink(self);
}

// ----------------------------------------------------------------------------
// NSS[2/8/00]:
// Name:		Buboid_ParseEpairs
// Description:Setup a trap to be able to monitor Buboid thinking
// Input:       userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Buboid_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	
	self->think		= Buboid_Think;
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		monster_buboid
// Description:
// Input:       self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_buboid( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_BUBOID);

	self->className		= "monster_buboid";
	self->netname		= tongue_monsters[T_MONSTER_BUBOID];

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0 );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = buboid_begin_attack;
	hook->fnAttackFunc		= buboid_attack;

	self->pain              = buboid_start_pain;
	self->die               = buboid_die_maybe;

	hook->pain_chance		= 20;
	hook->run_speed			= 80;
	hook->walk_speed		= 30.0;
	hook->attack_dist		= 60.0;

	self->ang_speed.y		= 100.0;

	self->think				= Buboid_ParseEpairs;

	self->nextthink			= gstate->time + 0.2;

	//self->fragtype          |= FRAGTYPE_ALWAYSGIB; 

	//////////////////////////////////////////////
	//	give the buboid his fists
	//////////////////////////////////////////////
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
	hook->nOriginalMoveType = self->movetype;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

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
void world_ai_buboid_register_func()
{
	gstate->RegisterFunc("buboid_begin_attack",buboid_begin_attack);
	gstate->RegisterFunc("buboid_attack",buboid_attack);

	gstate->RegisterFunc("BUBOID_GetOutOfCoffinThink",BUBOID_GetOutOfCoffinThink);
	gstate->RegisterFunc("buboid_start_pain",buboid_start_pain);
	gstate->RegisterFunc("buboid_die_maybe",buboid_die_maybe);
	gstate->RegisterFunc("Buboid_ParseEpairs",Buboid_ParseEpairs);
	gstate->RegisterFunc("Buboid_Think",Buboid_Think);
}
