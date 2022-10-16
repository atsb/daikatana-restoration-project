#if _MSC_VER
#include <crtdbg.h>
#endif
#include <stdlib.h>
//#include "dk_std.h"// SCG[1/23/00]: not used
#include "world.h"
#include "ai_utils.h"
#include "ai_move.h"
#include "ai_frames.h"
#include "ai.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "nodelist.h"
//#include "GrphPath.h"// SCG[1/23/00]: not used
#include "ai_func.h"
//#include "ai_weapons.h"// SCG[1/23/00]: not used
//#include "collect.h"// SCG[1/23/00]: not used
#include "action.h"
//#include "spawn.h"// SCG[1/23/00]: not used
//#include "Sidekick.h"// SCG[1/23/00]: not used
//#include "ai_info.h"// SCG[1/23/00]: not used
#include "SequenceMap.h"
//#include "ai_think.h"// SCG[1/23/00]: not used


extern int AI_FindNextPathCorner( userEntity_t *self );
extern int AI_HandleAirCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed );
extern int AI_HandleAirObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode );
extern int AI_HandleAirUnitGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed );
int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution);
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// New Functions for AI to use.  This is mainly for Flying Units, but there are some other odd ball
// functions in here for other AI types to use.  <nss>
/////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif

// ----------------------------------------------------------------------------
// NSS[2/26/00]:
// Name:		AI_GetAIDATA
// Description:Just trying to use the same code over.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
AIDATA *AI_GetAIDATA(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	GOALSTACK_PTR pGoalStack;
	TASK_PTR pCurrentTask;
	AIDATA_PTR pAIData;
	
	if(hook)
	{
		pGoalStack = AI_GetCurrentGoalStack( hook );
		if(pGoalStack)
		{
			pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
			if(pCurrentTask)
			{
				pAIData = TASK_GetData( pCurrentTask );
				return pAIData;
			}
		}
	}
	return NULL;
}




// ----------------------------------------------------------------------------
// NSS[1/19/00]:
// Name:		AI_4CastTrace
// Description: Will trace four point and if any collid with the mask type it wiil
// return TRUE... 
// Input:CVector &Origin, float x, float z, CVector &Destination,userEntity_t *self, unsigned long Mask
// Output: TRUE --> Hit something, FALSE---> Clear path
// Note:
// ----------------------------------------------------------------------------
int AI_4CastTrace(CVector &Origin, float x, float z, CVector &Destination,userEntity_t *self, unsigned long Mask)
{
	CVector Delta_Origin = Origin;
	int Hit = 0;

	Delta_Origin.x += x;
	Delta_Origin.z += z;
	tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, Mask );
	if(tr.fraction < 1.0f)
		Hit++;
	Delta_Origin.x -= x;
	Delta_Origin.z += z;
	tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, Mask );
	if(tr.fraction < 1.0f)
		Hit++;
	Delta_Origin.x += x;
	Delta_Origin.z -= z;
	tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, Mask );
	if(tr.fraction < 1.0f)
		Hit++;
	Delta_Origin.x -= x;
	Delta_Origin.z -= z;
	tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, Mask );
	if(tr.fraction < 1.0f)
		Hit++;

	if(Hit)
	{
		return TRUE;
	}
	return FALSE;
}



// ----------------------------------------------------------------------------
// <nss>
// Name:AI_SetAttackFinished
// Description:Sets an over-all attackfinish time 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_SetAttackFinished(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );

	switch(pCurrentTask->nTaskType)
	{
		case TASKTYPE_TAKECOVER_ATTACK:
		{
			hook->attack_finished = gstate->time + 2.0f + (rnd()*6.0f);	
			break;
		}
		default:
		{
			hook->attack_finished = gstate->time + 2.0f;	//won't attack for another 2 seconds
		}
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:AI_SetDeathBoundingBox
// Description:Reset the bounding box so that players can hit all monsters on the ground
// Input:CVector Skill_Values
// Output:float Skill_Value
// Note:
// ----------------------------------------------------------------------------
void AI_SetDeathBoundingBox(userEntity_t *self)
{
	float MinDistance=0.0f;
	float MaxDistance=0.0f;
	
	//Find the largest side.
	if(abs(self->s.mins.x) > abs(MaxDistance))
		MaxDistance = abs(self->s.mins.x);
	if(abs(self->s.mins.y) > abs(MaxDistance))
		MaxDistance = abs(self->s.mins.y);
	if(abs(self->s.mins.z) > abs(MaxDistance))
		MaxDistance = abs(self->s.mins.z);
	if(abs(self->s.maxs.x) > abs(MaxDistance))
		MaxDistance = abs(self->s.maxs.x);
	if(abs(self->s.maxs.y) > abs(MaxDistance))
		MaxDistance = abs(self->s.maxs.y);
	if(abs(self->s.maxs.z) > abs(MaxDistance))
		MaxDistance = abs(self->s.maxs.z);

	MinDistance = MaxDistance*-1;

	//Set our new bounding box
	gstate->SetSize(self,MinDistance,MinDistance,self->s.mins.z,MaxDistance,MaxDistance,5.0f);


	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:AI_GetSkill
// Description:Get's the skill level and returns it
// Input:CVector Skill_Values
// Output:float Skill_Value
// Note:Pass this function a CVector which holds 3 values related to skill.
// This function will just return the value which corisponds to the current
// select skill level.
// 0=X, 1=Y, 2=Z
// ----------------------------------------------------------------------------
float AI_GetSkill(CVector Skill_Values )
{
	switch((int)gstate->GetCvar("skill"))
	{
		case 0:
		{
			return Skill_Values.x;
		}
		case 1:
		{
			return Skill_Values.y;
		}
		case 2:
		default:
		{
			return Skill_Values.z;

		}
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:FindEntityClient
// Description:Search's through the entities to find a client and test to is if 
// it is visible.
// Input:userEntity_t *self 
// Output:userEntity_t *
// Note:NA
// ----------------------------------------------------------------------------
int FindClient(userEntity_t *self, int Distance)
{
	float Check_Dis;
	userEntity_t *ent = gstate->FirstEntity();
	
	//Distance factor
	if(!Distance)
		Distance = 2000;

	//Process through all the entities on the map
	while(ent)
	{
		//Make sure the entity is the right classname
		if(ent->flags & FL_CLIENT)
		{
			//Check its distance 
			Check_Dis = VectorDistance( self->s.origin, ent->s.origin);	
			if( Check_Dis < Distance)
			{
				return TRUE;
			}
		}
		ent = gstate->NextEntity(ent);
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:AdjustHealth_BasedOn_NOC
// Description:Will adjust the health of the entity passed to it based on the number
// of clients connected to the server at the time this function is called.
// Input:userEntity_t *self 
// Output:NA
// Note:NA
// ----------------------------------------------------------------------------
void AdjustHealth_BasedOn_NOC(userEntity_t *self)
{
	playerHook_t *hook = (playerHook_t *)self->userHook;
	if(hook && (hook->dflags & DFL_ISBOSS))
	{
		int Number_Of_Clients = 0;
		userEntity_t *ent = gstate->FirstEntity();
		//Process through all the entities on the map
		while(ent)
		{
			//Make sure the entity is the right classname
			if(ent->flags & FL_CLIENT)
			{
				Number_Of_Clients++;
			}
			ent = gstate->NextEntity(ent);
		}

		if(	Number_Of_Clients==0)
			Number_Of_Clients = 1;

		self->health = self->health * Number_Of_Clients;
		hook->base_health = hook->base_health * Number_Of_Clients;
		
		// NSS[6/7/00]:One time deal... 
		hook->dflags &= ~DFL_ISBOSS;
	}
}


// ----------------------------------------------------------------------------
///NSS[11/28/99]:
// Name:FindClientInPVS
// Description:Search's through the entities to find a client and test to is if 
// it is within the PVS.
// Input:userEntity_t *self 
// Output:userEntity_t *
// Note:NA
// ----------------------------------------------------------------------------
int FindClientInPVS(userEntity_t *self)
{
//	userEntity_t *Ret = NULL;// SCG[1/23/00]: not used
	userEntity_t *ent = gstate->g_edicts + 1;
	if(gstate->bCinematicPlaying)
		return TRUE;
	for( int i = 1; i <= gstate->game->maxclients; i++ )
	{
		ent = &gstate->g_edicts[i];

		if( ent->inuse == 0 )
		{
			continue;
		}

		if( ent->flags & FL_CLIENT )
		{
			userEntity_t *real_ent;
			// NSS[2/25/00]:Added this so in camera mode SFX complex particles will turn on when in camera mode.
			if(ent->view_entity)
			{
				real_ent = ent->view_entity;
			}
			else
			{
				real_ent = ent;
			}

			float fDistance = VectorDistance(self->s.origin,real_ent->s.origin);
			if(fDistance < 1000.0f)
			{
				//Check to see if this client is within the PVS
				if( gstate->inPVS( self->s.origin, real_ent->s.origin ) )
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:FindEntityByClassName
// Description:Search's through the entities to find a specific thing by classname
// you pass it a MOVE_TYPE to expediate searching.  (int cmp < time than str cmp)
// Input:userEntity_t *self, char *classname, float Distance
// Output:userEntity_t *
// Note:NA
// ----------------------------------------------------------------------------
userEntity_t * FindEntityByClassName(userEntity_t *self, char *classname, float Distance)
{
	float Check_Dis;
	userEntity_t *Ret = NULL;
	userEntity_t *ent = gstate->FirstEntity();
	
	//Distance factor
	if(!Distance)
		Distance = 10000;

	//Process through all the entities on the map
	while(ent)
	{
		//Make sure the entity is the right classname
		if(_stricmp(ent->className, classname)==0)
		{
			//Check its distance and make sure it has some active wisps
			Check_Dis = VectorDistance( self->s.origin, ent->s.origin);	
			if( Check_Dis < Distance)
			{
				//setup our new goal entity
				Ret = ent;
				Distance = Check_Dis;
			}
		}
		ent = gstate->NextEntity(ent);
	}
	return Ret;
}




// ----------------------------------------------------------------------------
//NSS[12/01/99]:
// Name:AI_HandleWallCollision
// Description:Handler to deal with walls and such
// Input: userEntity_t *self, CVector &forward, CVector &normal, CVector &dir
// Output:Nothing
// *** This is for GROUND UNITS only!!!
// ----------------------------------------------------------------------------
void AI_HandleWallCollision(  userEntity_t *self, CVector &forward, CVector &normal, CVector &dir )
{
	
	CVector AwayPoint;
	if(AI_ComputeBestAwayYawPoint(self, AwayPoint, 300.0f, 0.15f))
	{
		dir = AwayPoint - self->s.origin;
	}
	else //Do generic directional finding.
	{
		forward.Normalize();

		//	rotate right of wall normal
		CVector newdir;
		if ( normal.x == 0.0f && normal.y == 0.0f )
		{
			newdir.Set( -forward.y, forward.x, 0.0f );
		}
		else
		{
			newdir.Set( normal.y, -normal.x, 0.0f );
		}
		newdir.Normalize();

		// get cos of angle between goal direction and vector along wall
		float dot = DotProduct( forward, newdir );
		if ( dot > 0.0f )
		{
			dir.x = normal.y;
			dir.y = -normal.x;
		}
		else
		{
			dir.x = -normal.y;
			dir.y = normal.x;
		}

		dir.z = 0;
		dir.Normalize ();	
	}
}


// ----------------------------------------------------------------------------
//NSS[11/30/99]:
// Name:Node_IsWithinRange
// Description:A Replacement for the gstate->inPVS segments used in the node system.
// Input:CVector position1, CVector position2, int traceit
// Output:TRUE/FALSE
//
// ----------------------------------------------------------------------------
int Node_IsWithinRange(CVector &position1, CVector &position2, int traceit, int NodeType)
{
	
	if(NodeType &  (NODETYPE_WATER|NODETYPE_AIR))
	{
		float fDistance = VectorDistance(position1,position2);
		if(fDistance > 1024.0f)
		{
			return FALSE;
		}
	}
	else
	{
		float zDistance		= VectorZDistance(position1,position2);
		float xyDistance	= VectorXYDistance(position1,position2);
		if(zDistance > 64.0f || xyDistance > 1024.0f)
		{
			return FALSE;
		}
	}
	if(traceit)
	{
		 tr = gstate->TraceLine_q2( position1, position2, NULL, MASK_SOLID);
		if(tr.fraction < 1.0f)
		{
			return FALSE;
		}
	}
	return TRUE;
}


// ----------------------------------------------------------------------------
// NSS[2/11/00]:
// Name:        AI_HasGoalInQue
// Description: Determine if the Sidekick has 'Goal' in its Que
// Input:	userEntity_t *self
// Output:  Number of times we encountered this Goal
// Note:
// ----------------------------------------------------------------------------
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal )
{
	playerHook_t *hook			= AI_GetPlayerHook( self );
	userEntity_t *owner			= hook->owner;

	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack(hook);
	GOAL_PTR	Goal;
	if(!pGoalStack)
		return FALSE;

	int hit = 0;
	// NSS[1/16/00]:Get the next task to be executed
	if(pGoalStack->pTopOfStack)
	{
		Goal = pGoalStack->pTopOfStack;
		while(Goal)
		{
			if(Goal->nGoalType == SearchGoal)
			{
				hit ++;
				if(Goal->pNext)
				{
					Goal = Goal->pNext;
				}
				else
				{
					Goal = NULL;
				}
			}
			else
			{
				if(Goal->pNext)
				{
					Goal = Goal->pNext;
				}
				else
				{
					Goal = NULL;
				}
			}
		}
	}

	return hit;
}

// ----------------------------------------------------------------------------
// NSS[2/11/00]:
// Name:        AI_HandleWallMoveAwayPoints
// Description: This will find a node closest to this point within the AI's bounding
//region and will set its next task to move to this location.
// Input:	userEntity_t *self
// Output:  We can find a valid node to move to.
// Note:
// ----------------------------------------------------------------------------
int AI_HandleWallMoveAwayPoints(userEntity_t *self, CVector Dir)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	CVector destination;
	float Distance;
	if(!hook)
		return FALSE ;
	
	Distance    = VectorDistance(self->s.mins,self->s.maxs);
	if(self->movetype != MOVETYPE_SWIM && self->movetype != MOVETYPE_FLY && self->movetype != MOVETYPE_HOVER)
	{
		Dir.z = 0.0f;
	}
	destination = self->s.origin + (Dir * Distance);
	
	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, destination);
	
	if(Node)
	{
		tr = gstate->TraceLine_q2( self->s.origin, destination, self, MASK_MONSTERSOLID );
		
		if(!AI_HasGoalInQueue( self,GOALTYPE_MOVETOLOCATION) && (tr.fraction >= 1.0 && !tr.startsolid) )
		{
			AI_AddNewGoal(self,GOALTYPE_MOVETOLOCATION,Node->position);
			return TRUE;
		}
	}
	return FALSE;
}
// ----------------------------------------------------------------------------
// NSS[2/11/00]:
// Name:        AI_HasTaskInQue
// Description: Determine if the Sidekick has 'Task' in its Que
// Input:	userEntity_t *self
// Output:  Number of times we encountered this task
// Note:Yeah yeah yeah... queue.. it is mis-spelled... so sueue me... :P
// ----------------------------------------------------------------------------
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask )
{
	playerHook_t *hook			= AI_GetPlayerHook( self );
	userEntity_t *owner			= hook->owner;

	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack(hook);
	if(!pGoalStack)
		return FALSE;
	GOAL_PTR pCurrentGoal		= GOALSTACK_GetCurrentGoal( pGoalStack );
	if(!pCurrentGoal || !pCurrentGoal->pTasks)
		return FALSE;
	

	TASK_PTR Task;
	int hit = 0;
	// NSS[1/16/00]:Get the next task to be executed
	if(pCurrentGoal->pTasks->pStartList)
	{
		Task = pCurrentGoal->pTasks->pStartList;
		while(Task)
		{
			if(Task->nTaskType == SearchTask)
			{
				hit ++;
				if(Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
				}
			}
			else
			{
				if(Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
				}
			}
		}
	}

	return hit;
}


// ----------------------------------------------------------------------------
// NSS[11/30/99]:
// Name:AI_SetInitialThinkTime
// Description:Sets the initial think time to assure that all monsters don't think
// on the same frame.  Stager their original time offset... basically trying to get
// half the entities on one frame and the other half on the next one.
// and get 'stuck' in a loop.
// Input:userEntity_t *self
// Output:Nothing
// ----------------------------------------------------------------------------
void AI_SetInitialThinkTime(userEntity_t *self)
{
	if(self->s.number & 1)
	{
		self->nextthink = gstate->time + 0.2f;
	}
	else
	{
		self->nextthink = gstate->time + 0.3f;
	}
}


// ----------------------------------------------------------------------------
//NSS[11/30/99]:
// Name:AI_FindNewCourse
// Description:Handler to make the AI find a new course when they start colliding
// and get 'stuck' in a loop.
// Input:userEntity_t *self
// Output:Nothing
//
// ----------------------------------------------------------------------------
void AI_FindNewCourse( userEntity_t *self, CVector *Dir)
{
	
	//NSS[12/1/99]:Turn Around relative from the colliding entity (no room to find path around
	CVector Next_Dir,Destination;
	Next_Dir.Set(Dir->x,Dir->y,Dir->z);
	Next_Dir.Normalize();
	Next_Dir.Negate();
	float distance = 64.0;
	
	
	while(distance < 512)
	{
		
		Destination = (Next_Dir * distance) + self->s.origin;

		MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Destination);
		if(Node)
		{
			AI_Dprintf("Moving To Reverse Node.\n");
			tr = gstate->TraceLine_q2( self->s.origin, Node->position, self, MASK_SOLID);
			if(!AI_HasGoalInQueue( self,GOALTYPE_MOVETOLOCATION) && (tr.fraction >= 1.0 && !tr.startsolid) )
			{
				AI_AddNewGoal(self,GOALTYPE_MOVETOLOCATION,Node->position);
				return;
			}
			//we are probably stuck, let's start over... 
			/*if(AI_HasTaskInQue(self,TASKTYPE_MOVETOLOCATION) > 5)
			{
				AI_RestartCurrentGoal( self );
			}
			else
			{
				AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION, Node->position);
			}
			return;*/
		}
		distance += 64.0f;
	}
	
	// NSS[2/12/00]:Just a little something that might turn them out of this situation...otherwise they are stuck in some weird geometry situation
	if(rnd() > 0.50f)
	{
		if(AI_HasGoalInQueue( self,GOALTYPE_MOVETOLOCATION))
		{
			AI_RemoveCurrentGoal(self);
			return;
		}
		AI_RestartCurrentGoal( self );
	}

	AI_Dprintf("Cannot find a valid course!\n");
}



// ----------------------------------------------------------------------------
//NSS[11/30/99]:
// Name:AI_IsInSmallSpace
// Description:Determines if we are in a small space.
// Input:userEntity_t *self
// Output:Nothing
//
// ----------------------------------------------------------------------------
int AI_IsInSmallSpace( userEntity_t *self)
{
	int count = 0;
	CVector Dir,mins,maxs;
	CVector BoundIt;

	//Use only a little bit of our bounding region for this one.
	mins = self->s.mins * 0.35;
	mins.z += 4.0f;			//Lift it above the ground a tad
	maxs = self->s.maxs * 0.35;

	//Create a vector representation of our bounding box.
	BoundIt.Set(self->s.maxs.x - self->s.mins.x, self->s.maxs.y - self->s.mins.y, 0.0); //We don't check Z axis for this particular case
	//Increase it by say 50%
	BoundIt = BoundIt * 1.5;
		
	//Check to the left,right,forward, and behind
	Dir.Set(1.0f*BoundIt.x,0.0f,0.0f);
	Dir = self->s.origin + Dir;

	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, Dir, self, MASK_SOLID);
	if(tr.fraction < 1.0f)
		count++;
	Dir.Set(-1.0f*BoundIt.x,0.0f,0.0f);
	Dir = self->s.origin + Dir;
	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, Dir, self, MASK_SOLID);
	if(tr.fraction < 1.0f)
		count++;
	Dir.Set(0.0f,1.0f*BoundIt.y,0.0f);
	Dir = self->s.origin + Dir;
	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, Dir, self, MASK_SOLID);
	if(tr.fraction < 1.0f)
		count++;
	Dir.Set(0.0f,-1.0f*BoundIt.y,0.0f);
	Dir = self->s.origin + Dir;
	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, Dir, self, MASK_SOLID);
	if(tr.fraction < 1.0f)
		count++;
	
	if(count >= 2)
		return count;
	else
		return 0;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Checkbelow_For_Liquid
// Description: Checks directly below the monster for liquid.(landing creatures)
// Input:userEntity_t *self, int Resolution
// Output:1 = Found Water; 0 = You are clear below with ground!
// Notes: 
// Created:<nss>
// ----------------------------------------------------------------------------
int AI_Checkbelow_For_Liquid(userEntity_t *self,int Resolution)
{
	CVector Destination,mins,maxs;
	
	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	
	//Increase the bounding region a bit just to be safe
	mins.x *= 1.25;
	mins.y *= 1.25;
	mins.z *= 1.25;
	maxs.x *= 1.25;
	maxs.y *= 1.25;
	maxs.z *= 1.25;
	
	for(int i = 1; i < Resolution; i++)//check depth for Resolution*100 units down in iterations of 100 at a time
	{
		Destination = self->s.origin;
		Destination.z -= 100*i;	
		//Check for water
		tr = gstate->TraceLine_q2( self->s.origin, Destination, self, CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME);			
		if( tr.fraction < 1.0f )//we have liquid!
		{
			return 1;
		}
		tr = gstate->TraceLine_q2( self->s.origin, Destination, self, MASK_MONSTERSOLID );
		if( tr.fraction < 1.0f )//we have a solid surface!
		{
			return 0;
		}
				
	}
	return 1; // We are above max resolution and shouldn't be dropping anyway!

}

// ----------------------------------------------------------------------------
// <nss>
// Name:		Markers_Think
// Description:Think function for Markers
// Input:userEntity_t *self
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void Markers_Think (userEntity_t *self)
{
	if(self->delay < gstate->time)
		self->remove(self);
	self->nextthink = gstate->time + 0.01f;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Drop_Markers
// Description:Drops a Marker at the CVector Target origin and leaves it for
// 20 seconds
// Input:CVector Target
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void AI_Drop_Markers (CVector &Target, float delay)
{
	if(gstate->GetCvar("ai_debug_info") != 0.0f)
	{
		userEntity_t *Flare;
		
		Flare = gstate->SpawnEntity ();

		Flare->owner = Flare;
		Flare->solid = SOLID_NOT;
		Flare->movetype = MOVETYPE_NONE;
		Flare->className = "drop_sprite";

		Flare->s.renderfx |= (SPR_ALPHACHANNEL);

		Flare->s.modelindex = gstate->ModelIndex ("models/global/e_flare1.sp2");
		
		Flare->s.render_scale.Set( .25, .25, .25);
		Flare->delay = gstate->time + delay;
		Flare->think = Markers_Think;

		gstate->SetSize (Flare, -2, -2, -3, 2, 2, 3);
		gstate->SetOrigin2 (Flare, Target);
		gstate->LinkEntity (Flare);

		Flare->s.frame = 0;
		Flare->nextthink = gstate->time + 0.01f;
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Drop_Marker
// Description:Drops a Marker at the CVector Target origin
// Input:userEntity_t *self, CVector Target
// Output:<nothing>
// Note:  This function will destroy the marker once a new one is created
// the pointer to the marker is kept within self->teamchain....if you use this
// teamchain pointer somewhere else then you don't want to use this function!
// Created:<nss>
// ----------------------------------------------------------------------------
void AI_Drop_Marker (userEntity_t *self, CVector &Target)
{
	userEntity_t *Flare;
	
	if(self->teamchain)
	{
		gstate->RemoveEntity(self->teamchain);
	}
	
	Flare = gstate->SpawnEntity ();

	Flare->owner = self;
	Flare->solid = SOLID_NOT;
	Flare->movetype = MOVETYPE_NONE;
	Flare->className = "drop_sprite";

	Flare->s.renderfx |= (SPR_ALPHACHANNEL);

	Flare->s.modelindex = gstate->ModelIndex ("models/global/e_flare1.sp2");
	
	Flare->s.render_scale.Set( .25, .25, .25);

	gstate->SetSize (Flare, -2, -2, -3, 2, 2, 3);
	gstate->SetOrigin2 (Flare, Target);
	gstate->LinkEntity (Flare);

	Flare->s.frame = 0;

	self->teamchain = Flare;

}
#endif



// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_VectorMA
// Description: Will set a new destination Vector
// Input:useEntity_t *self(the monster),CVector scale(original scale),
// Input:CVector &out(final destination)
// Output:Affects the CVector out values.
// Notes:
// Created:<nss>
// ----------------------------------------------------------------------------

void AI_FlyVectorMA(userEntity_t *self, CVector *scale, CVector &out,int Degrees_Change)
{

	float fX,fY,fZ;
	float fSavedYaw = self->s.angles.yaw;
	
	CVector Vscale,facingDir;
	CVector angleVector( -20, self->s.angles.yaw, self->s.angles.roll );

	angleVector.y = AngleMod( fSavedYaw + Degrees_Change);
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();


	
	fX = self->s.origin.x + (facingDir.x*scale->x);//left right
	fY = self->s.origin.y + (facingDir.y*scale->y);//forward

	//Randomly change the altitude
	//This needs to be fixed due to the alternating altitude... 
	//The enemy's head is NOT a good point of reference.
	if( ( rnd() > 0.50f ) && ( self->s.origin.z > self->enemy->s.origin.z + (scale->z/2)) )
	{
		fZ = self->s.origin.z - (facingDir.z*scale->z);//Up< this might not work right >
	}
	else
	{
		fZ = self->s.origin.z + (facingDir.z*scale->z);//Down< this might not work right >
	}

	out.Set( fX, fY, fZ );
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_VectorMA
// Description: Will set a new destination Vector
// Input:useEntity_t *self(the monster),CVector scale(original scale),
// Input:CVector &out(final destination)
// Output:Affects the CVector out values.
// Notes:
// Created:<nss>
// ----------------------------------------------------------------------------

void AI_NewVectorMA(userEntity_t *self, CVector *scale, CVector &out,int Degrees_Change)
{

	float fX,fY,fZ;
	float fSavedYaw = self->s.angles.yaw;
	
	CVector Vscale,facingDir;
	CVector angleVector( self->s.angles.pitch, self->s.angles.yaw, self->s.angles.roll );

	angleVector.y = AngleMod( fSavedYaw + Degrees_Change);
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();


	
	fX = self->s.origin.x + (facingDir.x*scale->x);//left right
	fY = self->s.origin.y + (facingDir.y*scale->y);//forward

	//Randomly change the altitude
	//This needs to be fixed due to the alternating altitude... 
	//The enemy's head is NOT a good point of reference.
	if( ( rnd() > 0.50f ) && ( self->s.origin.z > self->enemy->s.origin.z + (scale->z/2)) )
	{
		fZ = self->s.origin.z - (facingDir.z*scale->z);//Up< this might not work right >
	}
	else
	{
		fZ = self->s.origin.z + (facingDir.z*scale->z);//Down< this might not work right >
	}

	out.Set( fX, fY, fZ );
}
// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_ComputeFlyAwayPoint
// Description: Modified version of FlyAwayPoint will find a point towards Axis
// Input:userEntity_t *self, CVector &flyAwayPoint, long Distance
// Output:1 = Found one; 0 = Could not find a path
// Notes: 
// Created:<nss>
// ----------------------------------------------------------------------------
int AI_ComputeFlyAwayPoint2( userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution, CVector &Axis)
{
	
	int i=0;
	float Degrees;
	float RL = rnd();
	float Cur_Dist = Distance;
	CVector flyAwayScale,mins,maxs,Dir,Yaw,target_angles;

	playerHook_t *hook = AI_GetPlayerHook( self );

	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	
	//Increase the bounding region
	mins.x *= 1.25;
	mins.y *= 1.25;
	mins.z *= 1.25;
	maxs.x *= 1.25;
	maxs.y *= 1.25;
	maxs.z *= 1.25;
	
	//Setup the range for the new Destination
	Dir = Axis - self->s.origin;
	Dir.Normalize();
	
	//Need to get our Yaw relative to the destination
	
	VectorToAngles(Dir,target_angles);

	Degrees = target_angles.yaw - 45; //Get the start point in the yaw spread to cover and find a point

	while(Cur_Dist > 50)
	{
		
		
		flyAwayScale.x = Cur_Dist * Dir.x;
		flyAwayScale.y = Cur_Dist * Dir.y;
		flyAwayScale.z = Cur_Dist * Dir.z;

		for(i=0;i<(360/Resolution);i++)
		{
			AI_FlyVectorMA(self, &flyAwayScale, flyAwayPoint, Degrees);

			tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, flyAwayPoint, self, MASK_SOLID|CONTENTS_MONSTER );	
			//tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, flyAwayPoint, self, self->clipmask );
			float fSpeed = AI_ComputeMovingSpeed( hook );
			CVector facingDir = self->velocity;
			facingDir.Normalize();
			
			
			if( tr.fraction >= 1.0f)
			{
				//Debug only
#ifdef _DEBUG
				AI_Drop_Marker (self, flyAwayPoint);
#endif
				return 1;
			}
			if(RL > 0.50f)
			{
				Degrees += Resolution;
			}
			else
			{
				Degrees -= Resolution;
			}
			if(Degrees > target_angles.yaw+45)
			{
				Degrees -= (target_angles.yaw +45);
			}
			if(Degrees < target_angles.yaw-45)
			{
				Degrees += (target_angles.yaw -45);
			}
		}
		//Reduce the distance by 85%
		Cur_Dist *= .85;	
	}
	AI_Dprintf( "FAILED TO FIND FLYAWAY POINT.\n" );
	return 0;	
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_ComputeBestAwayYawPoint
// Description: A much improved version of computing the flyaway points
// Input:userEntity_t *self, CVector &flyAwayPoint, long Distance, Resolution
// Output:1 = Found one; 0 = Could not find a path
// Notes: 
// Created:<nss>
// ----------------------------------------------------------------------------
int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	CVector Dir,target_angles,delta_angles,delta,endPoint,Target_Angles,Axis;
	int Choice,Trap;
	float Fraction,fDistance;
	Fraction = 0.0f;
	Trap =0;
	Choice = 0;
	//Let's create a distance for both our width and hieght
	//Also let's add a little bit of "extra" width in there to make sure we don't fucking hit something
	CVector nOrigin = self->s.origin;

	if(Distance < 1000.0f)
		fDistance= Distance;
	else
		fDistance = 1000.0f;
	while(fDistance > 50.0f)
	{
		delta_angles = self->s.angles;
		for(int i=-1;i <= 2; i++)
		{
			delta_angles.yaw += 90 * i;
			delta_angles.AngleToForwardVector(delta);
			//get new displacement
			delta = delta * fDistance;
			//set new endpoint
			endPoint = nOrigin + delta;

			//trace the endpoint
			tr = gstate->TraceLine_q2( nOrigin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
			if ( tr.fraction >= 1.0f)
			{
				Choice		= i*90;
				Fraction	= tr.fraction;
				Trap		= 1;
				break;
			}
		}
		if(Trap)
			break;
		else
			fDistance -= 50.0f;
	}
	
	Target_Angles.x += Choice + (crand() * Resolution);
	Target_Angles.AngleToForwardVector(Axis);
	Axis.z = 0.0f;
	Axis = self->s.origin + (Axis * (fDistance + 64.0f*rnd()));
	
	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);
	//NSS[12/3/99]:Now run an extra double check to make sure this is a valid node.
	if(Node)
	{
		fDistance = VectorDistance(self->s.origin, Node->position);
		if(Node->nNumLinks && fDistance < 64.0f)
		{
			for(int i=0; i < Node->nNumLinks ; i++)
			{
				// update the current node for this entity
				NODELIST_PTR pNodeList = hook->pNodeList;
				_ASSERTE( pNodeList );
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				if(Node->aLinks[i].nDistance > 64.0f)
				{
					MAPNODE_PTR tNode =  NODE_GetNode( pNodeHeader, Node->aLinks[i].nIndex );
					if(tNode != NULL)
					{
						Node = tNode;
					}
				}
			}
		}
		//AI_Drop_Markers(Node->position,1.0f);
		flyAwayPoint = Node->position;
		return 1;
	}
	flyAwayPoint = Axis;
	return 0;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_ComputeFlyAwayPoint
// Description: This will find and set a new "waypoint" for the flying entity
// Input:userEntity_t *self, CVector &flyAwayPoint, long Distance
// Output:1 = Found one; 0 = Could not find a path
// Notes: 
// Created:<nss>
// ----------------------------------------------------------------------------
int AI_ComputeFlyAwayPoint( userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution, CVector *Axis)
{
	
	int i=0;
	float Degrees=rnd()*360;
	float RL = rnd();
	float Cur_Dist = Distance;
	CVector flyAwayScale,mins,maxs;

	playerHook_t *hook = AI_GetPlayerHook( self );

	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	
	//Increase the bounding region
	mins.x *= 1.25;
	mins.y *= 1.25;
	mins.z *= 1.25;
	maxs.x *= 1.25;
	maxs.y *= 1.25;
	maxs.z *= 1.25;
	
	//Setup the range for the new Destination
	
	while(Cur_Dist > 100)
	{
		
		
		flyAwayScale.x = Cur_Dist * Axis->x;
		flyAwayScale.y = Cur_Dist * Axis->y;
		flyAwayScale.z = Cur_Dist * Axis->z;

		for(i=0;i<(360/Resolution);i++)
		{
			AI_FlyVectorMA(self, &flyAwayScale, flyAwayPoint, Degrees);

			tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, flyAwayPoint, self, MASK_SOLID|CONTENTS_MONSTER );	
			//tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, flyAwayPoint, self, self->clipmask );
			float fSpeed = AI_ComputeMovingSpeed( hook );
			CVector facingDir = self->velocity;
			facingDir.Normalize();
			
			
			if( tr.fraction >= 1.0f)
			{
				//Debug only
				//AI_Drop_Marker (self, flyAwayPoint);
				return 1;
			}
			if(RL > 0.50f)
			{
				Degrees += Resolution;
			}
			else
			{
				Degrees -= Resolution;
			}
			if(Degrees > 360.0f)
			{
				Degrees -= 360.0f;
			}
			if(Degrees < 0.0f)
			{
				Degrees += 360.0f;
			}
		}
		//Reduce the distance by 65%
		Cur_Dist *= .65;	
	}
	AI_Dprintf( "FAILED TO FIND FLYAWAY POINT.\n" );
	return 0;	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_ComputeAwayPoint
// Description: This will find and set a new "waypoint" for the flying entity
// Input:userEntity_t *self, CVector &flyAwayPoint, long Distance
// Output:1 = Found one; 0 = Could not find a path
// Notes: 
// Created:<nss>
// ----------------------------------------------------------------------------
int AI_ComputeAwayPoint( userEntity_t *self, CVector &AwayPoint, int Distance, int Resolution, CVector *Axis)
{
	
	int i=0;
	float Degrees=rnd()*360;
	float RL = rnd();
	float Cur_Dist = Distance;
	CVector AwayScale,mins,maxs;

	playerHook_t *hook = AI_GetPlayerHook( self );

	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	
	//Setup the range for the new Destination
	
	while(Cur_Dist > 50)
	{
		
		
		AwayScale.x = Cur_Dist * Axis->x;
		AwayScale.y = Cur_Dist * Axis->y;
		AwayScale.z = Cur_Dist * Axis->z;

		for(i=0;i<(360/Resolution);i++)
		{
			AI_NewVectorMA(self, &AwayScale, AwayPoint, Degrees);

			tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, AwayPoint, self, MASK_SOLID|CONTENTS_MONSTER );	
			//tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, flyAwayPoint, self, self->clipmask );
			float fSpeed = AI_ComputeMovingSpeed( hook );
			CVector facingDir = self->velocity;
			facingDir.Normalize();
			
			
			if( tr.fraction >= 1.0f )
			{
				return 1;
			}
			if(RL > 0.50f)
			{
				Degrees += Resolution;
			}
			else
			{
				Degrees -= Resolution;
			}
			if(Degrees > 360.0f)
			{
				Degrees -= 360.0f;
			}
			if(Degrees < 0.0f)
			{
				Degrees += 360.0f;
			}
		}
		//Reduce the distance by 65%
		Cur_Dist *= .50;	
	}
	AI_Dprintf( "FAILED TO FIND AWAY POINT.\n" );
	return 0;	
}



// ----------------------------------------------------------------------------
//<nss>
// Name:		AI_SmoothTurnToTarget
// Description: This will give the flying unit some curve to his turning
// Input: entSelf, Target, fTurnRate, fVelocity
// Output:<nothing>
// Notes:  
// 
// 
// 
// ----------------------------------------------------------------------------
CVector AI_SmoothTurnToTarget(userEntity_t *entSelf, CVector &Target, float fTurnRate, float fVelocity)
{
    CVector     vecGoalDir, vecChangeDir, vecReturn, vecStart;
    
	float fDistance = VectorDistance( entSelf->s.origin, Target );
	float fSpeedPerFrame = fVelocity * 0.1f;
	
	float Ratio = (M_PI*fSpeedPerFrame)/fDistance;
	
	float Mod	= ( 1.05 - fTurnRate) * Ratio;
	float TurnRate = fTurnRate + Mod;
	
	if(entSelf && Target)
    {
        vecStart = entSelf->s.origin;

		vecGoalDir = Target - vecStart;
        
		vecChangeDir = entSelf->velocity - vecGoalDir;
		
		if(TurnRate < 1.0f)
        {
            // triangulation
            vecChangeDir = vecChangeDir * TurnRate;
            vecGoalDir = entSelf->velocity - vecChangeDir;
            
			vecGoalDir.Normalize();
			vecReturn = vecGoalDir * fVelocity;
        }
		else
		{
            // home in directly
            vecGoalDir.Normalize();
            vecReturn = vecGoalDir * fVelocity;
        }
    }

    return vecReturn;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		HandleAirObstructions2
// Description:  New Air Handling routine..
// Input:userEntity_t *self, CVector *Dest
// Output:
// Note:Still under Testing
//
// ----------------------------------------------------------------------------
void HandleAirObstructions2(userEntity_t *self, CVector &Dest)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	CVector Dir,target_angles,delta_angles,delta,endPoint;
	//Let's create a distance for both our width and hieght
	//Also let's add a little bit of "extra" width in there to make sure we don't fucking hit something
	
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fThreeFrameSpeed = fSpeed * 0.50f;
	float fWidth = (((self->absmax.x - self->absmin.x)+(self->absmax.y - self->absmin.y)+(self->absmax.z - self->absmin.z))/3.25f);
	
	CVector nOrigin = self->s.origin;

	//Check origin
	//AI_Drop_Markers(nOrigin,0.18f);

	//Get the initial direction of travel
	Dir = Dest - nOrigin;
	
	Dir.Normalize();
	
	//Get the angles from the direction of travel
	VectorToAngles(Dir,target_angles);

	delta_angles = target_angles;
	
	delta_angles.yaw += 45;
	delta_angles.pitch += 45;

	delta_angles.AngleToForwardVector(delta);
	
	//get new displacement
	delta = delta * fWidth;
	//set new endpoint
	endPoint = nOrigin + delta;

	//Strictly Debug
	//AI_Drop_Markers(endPoint,0.18f);

	//trace the endpoint
    //  tr = gstate->TraceLine_q2( nOrigin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	
	if ( tr.fraction < 1.0f && tr.ent && !strstr(tr.ent->className,"door") && !strstr(tr.ent->className,"train"))
	{
		
		//Handle stuff from the Left
		delta.Negate();
		
		delta.Normalize();
		delta = delta * fThreeFrameSpeed;
		self->velocity = delta;
		//AI_Dprintf("Clip Left!\n");
		return;
	}

	delta_angles = target_angles;
	//Right forward
	delta_angles.yaw -= 45;
	delta_angles.pitch += 45;

	delta_angles.AngleToForwardVector(delta);

	//get new displacement
	delta = delta * fWidth;

	//set new endpoint
	endPoint = nOrigin + delta;

	//Strictly Debug
	//AI_Drop_Markers(endPoint,0.18f);
	
	//trace the endpoint
	//	tr = gstate->TraceLine_q2( nOrigin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && !strstr(tr.ent->className,"door") && !strstr(tr.ent->className,"train"))
	{
		//Handle stuff from the Right
		delta.Negate();
		delta.Normalize();
		delta = delta * fThreeFrameSpeed;
		self->velocity = delta;
		//AI_Dprintf("Clip Right!\n");
		return;
	}

	delta_angles = target_angles;

	//bottom left
	delta_angles.pitch -= 45;
	delta_angles.yaw   -= 45;

	delta_angles.AngleToForwardVector(delta);
	
	//get new displacement
	delta = delta * fWidth;
	//set new endpoint
	endPoint = nOrigin + delta;

	//Strictly Debug
	//AI_Drop_Markers(endPoint,0.18f);

	//trace the endpoint
	//	tr = gstate->TraceLine_q2( nOrigin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && !strstr(tr.ent->className,"door") && !strstr(tr.ent->className,"train"))
	{
		//Handle stuff from below
		delta.Negate();
		delta.Normalize();
		delta = delta * fThreeFrameSpeed;
		//self->velocity += delta;
		self->velocity = delta;
		//AI_Dprintf("Clip Below!\n");
		return;
	}

	delta_angles = target_angles;
	
	//bottom right
	delta_angles.pitch -= 45;
	delta_angles.yaw   += 45;
	
	delta_angles.AngleToForwardVector(delta);
	delta = delta * fWidth;
	endPoint = nOrigin + delta;

	//Strictly Debug
	//AI_Drop_Markers(endPoint,0.18f);
	//tr = gstate->TraceLine_q2( nOrigin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && !strstr(tr.ent->className,"door") && !strstr(tr.ent->className,"train"))
	{
		//Handle stuff from Above
		delta.Negate();
		delta.Normalize();
		delta = delta * fThreeFrameSpeed;
		self->velocity = delta;
		//AI_Dprintf("Clip Above!\n");
		return;
	}
}


// ----------------------------------------------------------------------------
// <nss>
// Name:	 AI_StartFlyTowardPoint2
// Description:
// Input:
// Output:
// Notes:  
// The wave on the movement is not working the way I would like it to work.  It
// seems to get to the base of the wave and then stops fluctuating the amplitude
// of the Z-Axis.  
// 
// 
// 
// ----------------------------------------------------------------------------
int AI_FlyTowardPoint2( userEntity_t *self, CVector &destPoint, float TurnRate )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	
	CVector vecGoalDir,vecDelta,vecStart, vecAnother;

	
	float fSpeed;

	if(AI_IsStateRunning(hook))
	{
		fSpeed = hook->run_speed;
	}
	else
	{
		fSpeed = (hook->walk_speed*2);
	}
	//Get our current velocity
	vecStart = self->velocity;
	
	//Turn to the Target
	if(TurnRate == 0.0)
		TurnRate = 0.01;
	vecGoalDir = AI_SmoothTurnToTarget( self, destPoint , TurnRate , fSpeed);

	vecAnother = vecGoalDir;
	//Get the change in velocity
	vecDelta  = vecGoalDir - self->velocity;

	//Get the new Raw roll value
	//float Roll = abs(vecDelta.Length());	
	CVector Temp1 = self->velocity;
	CVector Temp2 = vecGoalDir;
	float Roll = VectorDistance( Temp1, Temp2 );
	
	//Get our Current velocity
	vecDelta.CrossProduct(vecStart,vecGoalDir);
	vecDelta.Normalize();
	
	//Set our new velocity vector that we got from turnToTarget
	self->velocity = vecGoalDir;

	//nermalize the vectors!
	vecGoalDir.Normalize();
    vecStart.Normalize();

	//Get the angles to each vector
	vecGoalDir.VectorToAngles(vecGoalDir);
	vecStart.VectorToAngles(vecStart);
	

	//If we are a flying creature
	//let's have roll and Z axis movement
	if(self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_SWIM)
	{
		//Set our Roll... 
		//Another hack....
		//self->s.angles.roll = (Roll*vecDelta.z);
		//Set my angles to the new goal(face the new goal)
		self->s.angles.yaw	 = vecGoalDir.yaw;
		self->s.angles.pitch = vecGoalDir.pitch;

		float Roll;
		float yaw1,yaw2;
		
		//Shift Degrees to make sure we do not fucking ROLL OVER 
		if((vecStart.yaw > 345.0f)&&(vecGoalDir.yaw < 45.0f))
		{
			yaw1 = vecStart.yaw - 345.0f;
			yaw2 = vecGoalDir.yaw + 14.0f;
		}
		else if((vecStart.yaw < 45.0f) && (vecGoalDir.yaw > 345.0f))
		{
			yaw1 = vecStart.yaw + 14.0f;
			yaw2 = vecGoalDir.yaw - 345.0f;
		}
		else
		{
			yaw1 = vecStart.yaw;
			yaw2 = vecGoalDir.yaw;
		}
		//Are we turning left or right?
		if(vecDelta.z < 0)
		{
			Roll = -2.75  * abs(yaw1 - yaw2);
		}
		else
		{
			Roll = 2.75 * abs(yaw1 - yaw2);
		}
		//Don't Roll too much
		if(abs(Roll) > 360)
		{
			Roll = self->s.angles.roll/130.9;//(360/2.75)
		}
		if(abs(Roll) > 45)
		{
			//Hack to keep the sign direction of the roll
			Roll = (Roll/abs(Roll))*45;
		}

		//Make sure we don't roll too much too soon
		if((abs(Roll-self->s.angles.roll))>5)
		{
			if(Roll > self->s.angles.roll)
				self->s.angles.roll += 5.0f;
			else
				self->s.angles.roll -= 5.0f;
		}
		if(hook->type == TYPE_CHAINGANG || hook->type == TYPE_HARPY )
		{
			self->s.angles.roll = 0;
			self->s.angles.pitch = 0;
		}
		if(self->movetype == MOVETYPE_SWIM)
		{
			self->s.angles.roll = 0;
		}
	}
	else
	{
		//Set my angles to the new goal(face the new goal)
		self->s.angles = vecGoalDir;
	}
	
	//If we are a walking creature
	//let's orient ourselves to the floor.
	if ( AI_IsGroundUnit(self) )
	{
		if ( !AI_IsGroundBelowBetween( self, self->s.origin, destPoint ) )//make sure we got our feet on the ground
		{
			self->velocity.z -= 80.0f;
		}
	}
	else
	{
		//HardCore Collision detection... Should not be used for monsters that are air/ground units
		if(hook->type == TYPE_DOOMBAT || hook->type == TYPE_SEAGULL || hook->type == TYPE_SL_SKEET || hook->type == TYPE_CAMBOT)
		{
			HandleAirObstructions2(self, destPoint);
		}
		else
		{
				
			if ( AI_IsInsideBoundingBox( self ) == FALSE )
			{
				AI_RemoveCurrentGoal( self );
				return FALSE;
			}

			if ( !AI_CanMove( hook ) )
			{
				AI_RemoveCurrentTask( self );
				return FALSE;
			}

			float fDistance = VectorDistance( self->s.origin, destPoint );
			if ( AI_IsCloseDistance2( self, fDistance ) )
			{
				return TRUE;
			}
			
			float fSpeed = AI_ComputeMovingSpeed( hook );

			CVector vectorTowardPoint;
			CVector angleTowardPoint;
			vectorTowardPoint = destPoint - self->s.origin;
			vectorTowardPoint.Normalize();

			if ( self->movetype == MOVETYPE_HOVER )
			{
				forward = vectorTowardPoint;
			}
			else
			{
				VectorToAngles( vectorTowardPoint, angleTowardPoint );
				CVector angleVector = self->s.angles;
				CVector right, up;
				if ( AI_IsSameAngle3D( self, angleVector, angleTowardPoint ) == TRUE )
				{
					forward = vectorTowardPoint;
				}
				else
				{
					angleVector.AngleToForwardVector( forward );
					forward.Normalize();
				}
			}
				
			if ( AI_HandleAirUnitGettingStuck( self, destPoint, fSpeed ) )
			{
				return FALSE;
			}		

			if ( AI_HandleAirCollisionWithEntities( self, destPoint, fSpeed ) )
			{
				return FALSE;
			}

			if ( AI_HandleAirObstacle( self, fSpeed, FALSE ) )
			{
				forward.Normalize();

				//scale speed based on current frame's move_scale
				float fScale = FRAMES_ComputeFrameScale( hook );
				AI_SetVelocity( self, forward, (fSpeed * fScale) );
				return FALSE;
			}


	
		}
	}
	// NSS[2/10/00]:Trap for swim types trying to go down on ridges in the water.
	if(self->groundEntity && self->movetype == MOVETYPE_SWIM && self->velocity.z < 0.0f)
	{
		self->velocity.z = 0.0f;
	}
	if(hook->type == TYPE_TH_SKEET)
	{
		self->s.angles.pitch = 0.0f;
		self->s.angles.roll  = 0.0f;
	}

	// NSS[2/24/00]:Limit the shark's pitch.
	if(hook->type == TYPE_SHARK)
	{
		if(self->s.angles.pitch > 45.0f && self->s.angles.pitch < 180.0f)
		{
			self->s.angles.pitch = 45.0f;
		}
		else if(self->s.angles.pitch < 315.0f && self->s.angles.pitch > 180.0f)
		{
			self->s.angles.pitch = 315.0f;
		}
	}
	//Make the standard flying sounds... 
	ai_frame_sounds( self );

	float fDistance = VectorDistance( self->s.origin, destPoint );
	if ( fDistance < 16.0f + ((self->s.maxs.x - self->s.mins.x)*0.5f))
	{
		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Fly_Patrol
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Fly_Patrol( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	ai_wander_sound( self );

	userEntity_t *target = com->FindTarget( self->target );
	_ASSERTE( target );

	ai_frame_sounds( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );

	float fXYDistance = VectorXYDistance( self->s.origin, target->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, target->s.origin );
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
		if ( !AI_FindNextPathCorner( self ) )
		{
			ai_debug_print( self, "ai_path_wander: couldn't find monster_path_corner\n" );


			if ( nGoalType == GOALTYPE_PATROL )
			{
				GOAL_Satisfied( pCurrentGoal );
			}

			AI_RemoveCurrentTask( self );
			return;
		}
	}

	target = com->FindTarget( self->target );
	
	//New Fly Type... 
	AI_FlyTowardPoint2( self, target->s.origin, 0.10f);
	//AI_MoveTowardPoint( self, target->s.origin, TRUE );
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_FLY_Debug_Origin
// Description: Will check to see if we have moved or not.  If we haven't it is bad.
// Input:self
// Output:T/F
// Note:
//
// ----------------------------------------------------------------------------
int AI_FLY_Debug_Origin(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
//	AI_Dprintf( "poX:%f  poY:%f poZ:%f\n",hook->last_origin.x , hook->last_origin.y,hook->last_origin.z);
//	AI_Dprintf( "pnX:%f  pnY:%f pnZ:%f\n",self->s.origin.x,self->s.origin.y,self->s.origin.z);
	if(hook->last_origin == self->s.origin)
	{
		return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------------
// Creator:<nss>
// Name:		Terrain_Node_Change
// Description:When going from air to ground or vice-versa we need to reset node stuff
// Input:userEntity_t *self, int MOVETYPE
// Output:NA
// Note: VERY IMPORTANT WHEN CHAING MOVETYPE FROM AIR MOVEMENT TO GROUND MOVEMENT
// ----------------------------------------------------------------------------
void Terrain_Node_Change(userEntity_t *self, movetype_t MoveType)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	int nNodeType;
	
	self->movetype = MoveType;
	
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
	if ( pNodeHeader )
	{
		hook->pNodeList = node_create_nodelist(pNodeHeader);
		hook->pNodeList->pNodeHeader = pNodeHeader;
		if ( !AI_IsGroundUnit(self) )
		{
			// VERY IMPORTANT: if not on ground then the current index should be set to -1
			hook->pNodeList->nCurrentNodeIndex = -1;
		}
		else
		{
			node_find_cur_node( self, hook->pNodeList );
		}
	}
	PATHLIST_KillPath( hook->pPathList );
	if (AI_IsGroundUnit(self))
	{
		AI_UpdateCurrentNode( self );
	}
	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Determine_Room_Height
// Description:This function will first determine the height of the room the
// (userEntity_t *self) is in.
// Input:userEntity_t *self  --->Monster
//		Max_Mid = the Maximum Mid-Height value for above and then below the origin
// Output:<hieght>(float)
// Note:
//
// ----------------------------------------------------------------------------
float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid)
{
	float Height;
	CVector Destination,mins,maxs;
	
	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	//Set the Destination to be above the monster
	Destination.x = self->s.origin.x;
	Destination.y = self->s.origin.y;
	Destination.z = self->s.origin.z + Max_Mid;

	tr = gstate->TraceLine_q2( self->s.origin, Destination, self, MASK_MONSTERSOLID);
	//Set the upper distance
	Height = tr.fraction * Max_Mid;

	//Set the Destination to be below us
	Destination.z = self->s.origin.z - Max_Mid;
	tr = gstate->TraceLine_q2( self->s.origin, Destination, self, MASK_MONSTERSOLID);
	//calculate the lower Distance and add it to the upper Distance
	Height += tr.fraction * Max_Mid;//We now have the Height of the room from the perspective of the monster
	
	return Height;
}



// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Determine_Room_Height
// Description:This function will first determine the height of the room the
// (userEntity_t *self) is in.
// Input:userEntity_t *self  --->Monster
//		Max_Mid = the Maximum Mid-Height value for above and then below the origin
//		type:
//      0---> return the height from the floor to the ceiling in distance and not coordinates
//      1---> return the height from the floor to the entity
//      2---> return the height from the entity to the ceiling.
// Output:<hieght>(float)
// Note:
//
// ----------------------------------------------------------------------------
float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type)
{
	float Height,Floor,Ceiling;
	CVector Destination,mins,maxs;
	
	//get our bounding region
	mins = self->s.mins;
	maxs = self->s.maxs;

	//Set the Destination to be above the monster
	Destination.x = self->s.origin.x;
	Destination.y = self->s.origin.y;
	Destination.z = self->s.origin.z + Max_Mid;

	tr = gstate->TraceLine_q2( self->s.origin, Destination, self, MASK_SOLID);
	//Set the upper distance
	Ceiling = tr.fraction * Max_Mid;

	//Set the Destination to be below us
	Destination.z = self->s.origin.z - Max_Mid;
	tr = gstate->TraceLine_q2( self->s.origin, Destination, self, MASK_MONSTERSOLID);
	//calculate the lower Distance and add it to the upper Distance
	Floor = tr.fraction * Max_Mid;//We now have the distance from the monster to the floor.
	
	
	Height = Ceiling + Floor;//We now have the Height of the room from the perspective of the monster
	
	if(type == 0)
		return Height;
	else if(type==1)
		return Floor;
	else
		return Ceiling;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_TraceZLimit
// Description: 
// Input:       userEntity_t *self => entity to check
//              float maxdist => the maximum distance up to check, if positive
//                               the maximum distance down to check, if negative
// Output:      The distance (less than or equal to maxdist) traced in the Z
//              axis from the entity to the ground or to the ceiling.
// Note:
//
// ----------------------------------------------------------------------------
float AI_TraceZLimit( userEntity_t *self, float maxdist )
{
	CVector dest = self->s.origin;
	
	// set dest to be above or below entity
	dest.z = self->s.origin.z + maxdist;

	tr = gstate->TraceLine_q2( self->s.origin, dest, self, MASK_MONSTERSOLID);

	return tr.fraction * maxdist;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_ParseEpairs2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ParseEpairs2( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	AI_InitNodeList( self );
	Terrain_Node_Change(self, self->movetype );
	AI_ParseEpairs(self);
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_DetectForLikeMonsters
// Description: This function will check to make sure we are not going to bust 
// open our monster buddies. It will return a 1 if we are and a 0 if all is clear.
// Input: userEntity_t *self
// Output:1.0 to 0.0f   0.50 is a monster of its same type and anything below but
// above 0.0 is a monster 'lower' on the food chain.
// Note:
//
// ----------------------------------------------------------------------------
float AI_DetectForLikeMonsters( userEntity_t *self)
{
	float Weight = 0.0f;
//	playerHook_t *hook		= AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	CVector Origin = self->enemy->s.origin;
	Origin.z -=24.0f;
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, Origin, self, MASK_SOLID|CONTENTS_MONSTER );	
	if ( (tr.fraction < 1.0f) && (tr.contents & CONTENTS_MONSTER) )
	{
		
		if(AI_IsAlive(tr.ent))
		{
//			playerHook_t *mhook		= AI_GetPlayerHook( tr.ent );// SCG[1/23/00]: not used
			if(tr.ent->flags & FL_MONSTER)
			{
				Weight += 1.0f;
			}
		}
	}
	return Weight;
}


// ----------------------------------------------------------------------------
// NSS[1/7/00]:
// Name:		Fish_Think
// Description:Loop to snag fish thought so we can test various things.
// Input: userEntity_t *self
// Output:NA
// Note:NA
//
// ----------------------------------------------------------------------------
void FISH_Think(userEntity_t *self)
{
	//AI_Dprintf("vL:%f\n", self->velocity.Length() );
	AI_TaskThink(self);
	if(self->waterlevel < 3 && self->velocity.z > 0.0f)
	{
		self->velocity.z = 0.0f;
	}
	//AI_Dprintf("vL:%f\n", self->velocity.Length() );
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		Fish_ParseEpairs
// Description:The fish need to have their node list updated
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FISH_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	AI_InitNodeList( self );
	Terrain_Node_Change(self, MOVETYPE_SWIM);
	AI_ParseEpairs(self);
	self->think		=	FISH_Think;
	self->nextthink =	gstate->time + 0.10;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		FISH_Update
// Description: Simple trap before each think which checks to make sure the fish
// are either pathfollowing or wandering.  Here is where the master/slave code
// will eventually go.
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FISH_Update( userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	AI_Update(self);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_func2_register_func()
{
	gstate->RegisterFunc("Markers_Think",Markers_Think);
	gstate->RegisterFunc("AI_ParseEpairs2",AI_ParseEpairs2);
	gstate->RegisterFunc("FISH_ParseEpairs",FISH_ParseEpairs);
	gstate->RegisterFunc("FISH_Update",FISH_Update);
	gstate->RegisterFunc("FISH_Think",FISH_Think);
}
