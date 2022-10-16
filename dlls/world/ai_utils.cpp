// ==========================================================================
//
//  File:		ai_utils.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
#include "actorlist.h"
#include "ai_move.h"
#include "ai_weapons.h"
#include "nodelist.h"
#include "ai_func.h"
#include "ai_frames.h"
#include "collect.h"
#include "ai_info.h"
#include "spawn.h"
#include "ai.h"
#include "action.h"
#include "items.h"
#include "MonsterSound.h"

#define ONE_OVER_360	        0.002777778f
#define ONE_OVER_PI		        0.318309886f

#define STRAIGHT_MOVE_DISTANCE  256.0f

/* ***************************** Local Variables **************************** */
static	int	bDisableAI = FALSE;

extern CPtrList *pWeaponList;
extern CPtrList *pAmmoList;
extern CPtrList *pArmorList;
extern CPtrList *pHealthList;
extern CPtrList *pStatBoostList;
extern CPtrList *pGoldenSoulList;


// ----------------------------------------------------------------------------
//
// Name:		AI_Dprintf
// Description:
//	only prints if ai_debug cvar is != 0 AND developer cvar is != 0
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Dprintf( char *fmt, ... )
{
	if (ai_debug->value == 0)
	{
		return;
	}

	char szBuf[1024];
	va_list	ap;

	va_start (ap, fmt);
	vsprintf (szBuf, fmt, ap);
	va_end (ap);

	gstate->Con_Dprintf (szBuf);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_debug_info
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_debug_info( userEntity_t *self, char *routine, char* extra )
{
    if ( gstate->GetCvar("ai_debug_info") != 0.0f )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	char msg[256];
	
	if (self->flags & FL_DEBUG)
	{
		gstate->printxy (0, 32, "%s", routine);
		gstate->Con_Dprintf ("--- %s ---\n", routine);

//		switch ( hook->goal_type )
//		{
//			case GOAL_TEMP:
//				gstate->printxy (0, 40, "GOAL_TEMP");
//				break;
//			case GOAL_NODE:
//				gstate->printxy (0, 40, "GOAL_NODE");
//				break;
//			case GOAL_PATH:
//				gstate->printxy (0, 40, "GOAL_PATH");
//				break;
//			case GOAL_ENEMY:
//				gstate->printxy (0, 40, "GOAL_ENEMY");
//				break;
//			default:
//				gstate->printxy (0, 40, "GOAL UNKNOWN");
//				break;
//		}

		gstate->printxy (0, 48, "%s", extra);

		msg [0] = 0x00;

		if (self->flags & FL_FORCEANGLES)
			strcat (msg, "FA ");
		if (self->flags & FL_FIXANGLES)
			strcat (msg, "FX ");
		if (self->flags & FL_FAKEENTITY)
			strcat (msg, "FE ");
		if (self->flags & FL_CLIENT)
			strcat (msg, "CL ");
		if (self->flags & FL_MONSTER)
			strcat (msg, "MN ");
		if (self->flags & FL_SWIM)
			strcat (msg, "SW ");
		if (self->flags & FL_NOTARGET)
			strcat (msg, "NT ");
		if (self->flags & FL_ITEM)
			strcat (msg, "IT ");
		if (self->flags & FL_WATERJUMP)
			strcat (msg, "WJ ");
		if (self->flags & FL_BOT)
			strcat (msg, "BT ");
		if (self->flags & FL_EXPLOSIVE)
			strcat (msg, "EX ");
		if (self->flags & FL_ATTACKHOLD)
			strcat (msg, "AH ");
		if (self->flags & FL_HCHILD)
			strcat (msg, "SC ");
		if (self->flags & FL_HPARENT)
			strcat (msg, "SP ");
		if (self->flags & FL_BODY)
			strcat (msg, "BD ");
		if (self->flags & FL_INWARP)
			strcat (msg, "IW ");
		if (self->flags & FL_CAMERAMODE)
			strcat (msg, "CM ");
		if (self->flags & FL_DEBUG)
			strcat (msg, "DB ");
		if (self->flags & FL_IGNOREUSETARGET)
			strcat (msg, "IU ");
		if (self->flags & FL_NORESPAWN)
			strcat (msg, "NR ");

		gstate->printxy (0, 56, "flags %s", msg);

		msg[0] = 0x00;

		gstate->printxy (0, 64, "flags %s", msg);

		msg[0] = 0x00;

		if (hook->ai_flags & AI_CONTINUE)
			strcat (msg, "CO ");
		else 
		if (hook->ai_flags & AI_SPECIAL)
			strcat (msg, "SP ");
		else 
		if (hook->ai_flags & AI_END_ATTACK)
			strcat (msg, "EA ");
		else 
		if (hook->ai_flags & AI_JUMP)
			strcat (msg, "JM ");
		else 
		if (hook->ai_flags & AI_TURN)
			strcat (msg, "TR ");
		else 
		if (hook->ai_flags & AI_ACTION)
			strcat (msg, "AC ");

		gstate->printxy (0, 72, "ai_fl %s", msg);
		
		gstate->printxy (0, 80, "water = %i", self->waterlevel);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_debug_print
// Description:
//	prints to the console if FL_DEBUG bit is set in self->flags
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_debug_print( userEntity_t *self, char *fmt, ... )
{
    if (gstate->GetCvar("ai_debug_info") != 0.0f) 
	{
		return;
	}

	char msg[255];
	va_list	argptr;
	
	if ( self->flags & FL_DEBUG )
	{
		va_start (argptr,fmt);
		vsprintf (msg,fmt,argptr);
		va_end (argptr);

		gstate->Con_Dprintf ("%s", msg);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_setfov
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_setfov( userEntity_t *self, float fov_degrees )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->fov = fov_degrees;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_client_near
// Description:
//	determines if a client or viewentity is within
//	hook->active_distance and returns TRUE if so
// Input:
// Output:
// Note: This is the main loop for monsters to actually spot the player //NSS[11/6/99]:
//
// ----------------------------------------------------------------------------
int	ai_client_near( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *head = alist_FirstEntity(client_list);
	while ( head )
	{
		if ( AI_IsAlive( head ))
        {
			if(head->flags & (FL_CLIENT | FL_BOT) && !(head->deadflag == DEAD_PUNK))
			{
				float fDistance = VectorDistance (self->s.origin, head->s.origin);

				//NSS[11/6/99]:Added the right hand portion to make monsters attack if attacked by
				//something outside of their attack distance but they can still see the attacker.
				if ( fDistance < hook->active_distance || (self->enemy && AI_IsVisible(self,self->enemy)) )
				{
					return TRUE;
				}
				else
				{
					float fCameraDistance = 0;
					playerHook_t *pPlayerHook = AI_GetPlayerHook( head );
					if(pPlayerHook)
					{
						userEntity_t *pCamera = pPlayerHook->camera;
						if ( pCamera )
						{
							fCameraDistance = VectorDistance(self->s.origin, pCamera->s.origin);
							if ( fCameraDistance < hook->active_distance)
							{
								return	TRUE;
							}
						}
					}
				}
			}
        }

		head = alist_NextEntity (client_list);
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_max_height
// Description:
//	returns the maximum height an entity can jump to starting with upward
//	velocity v0
//
//	accounts for self->gravity variations
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float ai_max_height( userEntity_t *self, float v0 )
{
	float g;

	if (self->gravity != 0.0)
	{
		g = p_gravity->value * self->gravity;
	}
	else
	{
		g = p_gravity->value;
	}

	//	scale by .75 to account for variance in frame rates
	return	((v0 * v0) / (2 * g)) * 0.75;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_jump_vel
// Description:
// Input:
// Output:
// Note:
//	sets the correct forward velocity need to jump distxy units forward and
//	land distz units below current position
//
// ----------------------------------------------------------------------------
float ai_jump_vel(userEntity_t *self, float distxy, float distz, float up_vel)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	//	FIXME:	use gstate->gravity instead of GetCVar!!  Slow!!!
	float g = p_gravity->value;
	if (self->gravity != 0.0)
	{
		g = p_gravity->value * self->gravity;
	}

	float y0 = self->s.origin.z;
	float y = self->s.origin.z + distz;

	float root = (up_vel * up_vel) - (3 * g * (y - y0));

	// negative value means we're trying to jump to a higher ledge!
	if (root < 0)
	{
		if(AI_IsSuperfly(hook) && gstate->episode == 4)
			return 400.0f;
		else
			return hook->run_speed;
	}

	float fJumpVelocity = (distxy * g) / (up_vel + sqrt(root));

	// cap the jump velocity to no more than twice the running speed
	if ( fJumpVelocity > hook->run_speed )
	{
		if(AI_IsSuperfly(hook)  && gstate->episode == 4)
			fJumpVelocity = 400.0f;
		else
			fJumpVelocity = hook->run_speed;
	}

	return fJumpVelocity;
};

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeNeededJumpVelocity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_ComputeNeededJumpVelocity( userEntity_t *self, float distxy, float up_vel )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	float g = p_gravity->value;
	if (self->gravity != 0.0)
	{
		g = p_gravity->value * self->gravity;
	}

	
	CVector velocity;
	velocity.x = hook->run_speed;
	velocity.z = up_vel;
	velocity.Normalize();

	CVector angles;
	VectorToAngles( velocity, angles );

	float fTwoTheta = 2 * angles.pitch;
	float fRad = DEG2RAD( fTwoTheta );

	float fNeededVelocity = sqrt( (distxy * g) / sin(fRad) );

	return fNeededVelocity;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_max_jump_dist
// Description:
//	returns the maximum distance an entity can jump with forward_vel
//	and upward_vel
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float ai_max_jump_dist( float forward_vel, float upward_vel )
{
	return forward_vel * (2 * (upward_vel / p_gravity->value));
}


// ----------------------------------------------------------------------------
//
// Name:		AI_CheckAirTerrain
// Description:
// Input:
// Output:
//	returns:
//	0 = not blocked (clear or a grade)
//	1 = detected a far obstacle
//	2 = impassable obstruction
//
// Note:
// MIN_WALL_DIST is how close a wall has to be before it considered
// an obstruction
// LOOKAHEAD_DIST is how far ahead the traces are made in front of
// the entity
// Obviously LOOKAHEAD_DIST needs to be greater than MIN_WALL_DIST or
// no walls will be considered blocking
//
// ----------------------------------------------------------------------------

#define	MIN_WALL_DIST	20.0		// walls must be this close to be blocking
#define	LOOKAHEAD_DIST	32.0
#define	WALL_Z			0.7
#define	Z_MINUS			24

#define	TOPLEFT			0
#define	MIDLEFT			1
#define	BOTTOMLEFT		2
#define	TOPRIGHT		3
#define	MIDRIGHT		4
#define	BOTTOMRIGHT		5 

#define	SHOWMARKERS		1
#define	SHOWOUTPUT 		1

int	AI_CheckAirTerrain( userEntity_t *self, const CVector &dir, float dist )
{
	_ASSERTE( self );

	CVector			org, start, end, dir_right;
	float			box_width;
	float			obs_height = 0, max_height = 0;
	float			tr_dist[6];
	CVector			tr_end[6];
	userEntity_t	*tr_ent[6];
	CVector			tr_normal[6];
	int				tr_fraction[6];
	int				i;
	
	if ( dist < LOOKAHEAD_DIST )
	{
		dist = LOOKAHEAD_DIST;
	}

	/////////////////////////////////////////////////////////////////////////
	//	determine if we are blocked at all
	/////////////////////////////////////////////////////////////////////////
	
	end = self->s.origin + dir * dist;

	tr = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);

	memset(&terrain, 0x00, sizeof(terrain));
	terrain.wall_normal = tr.plane.normal;

	if ( tr.fraction == 1.0 )
	{
		return FALSE;
	}

	CVector savedWallNormal = tr.plane.normal;

	//	get wall yaw and pitch
	VectorToAngles( terrain.wall_normal, org );
	terrain.wall_yaw = AngleMod(org.y - 90.0);
	terrain.wall_pitch = org.x;
	
	terrain.wall_dist = tr.fraction * dist;

	/////////////////////////////////////////////////////////////////////////
	//	check for passable terrain (grade or low obstruction)
	/////////////////////////////////////////////////////////////////////////

	//box_height = (self->s.maxs.z - self->s.mins.z) - (24 - Z_MINUS);
	box_width = (self->s.maxs.x - self->s.mins.x) * 0.5;

	//	add to dist because TraceBox actually traces 1/2 the bounding box 
	//	further than a TraceLine
	dist		= dist + box_width + box_width;
	dir_right.Set(dir.y, -dir.x, 0);
	org			= self->s.origin;


	tr_fraction[0] = 1.0;
	tr_fraction[1] = 1.0;
	tr_fraction[2] = 1.0;
	tr_fraction[3] = 1.0;
	tr_fraction[4] = 1.0;
	tr_fraction[5] = 1.0;

	/////////////////////////////////////////////////////////////////////////
	//	fill all tr_normals with a passable Z
	/////////////////////////////////////////////////////////////////////////
	tr_normal[0].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[1].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[2].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[3].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[4].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[5].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up

	/////////////////////////////////////////////////////////////////////////
	//	trace lines down left side of bounding box
	/////////////////////////////////////////////////////////////////////////

	//	lower left
	start	= org - dir_right * box_width;
	start.z -= (self->s.mins.z - 0.1f);
	end		= start + dir * dist;
	tr		= gstate->TraceLine_q2(start, end, self, MASK_SOLID);
	tr_dist[BOTTOMLEFT] = tr.fraction * dist;
	tr_end[BOTTOMLEFT]	= tr.endpos;
	tr_ent[BOTTOMLEFT]	= tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[BOTTOMLEFT] = tr.plane.normal;
		tr_fraction[BOTTOMLEFT] = tr.fraction;
	}

	//	middle left
	start.z += (self->s.mins.z - 0.1f);
	end.z	+= (self->s.mins.z - 0.1f);
	tr		= gstate->TraceLine_q2(start, end, self, MASK_SOLID);
	tr_dist[MIDLEFT] = tr.fraction * dist;
	tr_end[MIDLEFT]	 = tr.endpos;
	tr_ent[MIDLEFT]	 = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[MIDLEFT] = tr.plane.normal;
		tr_fraction[MIDLEFT] = tr.fraction;
	}

	//	top left
	start	= org - dir_right * box_width;
	start.z += self->s.maxs.z;
	end		= start + dir * dist;
	tr		= gstate->TraceLine_q2(start, end, self, MASK_SOLID);
	tr_dist[TOPLEFT] = tr.fraction * dist;
	tr_end[TOPLEFT]	 = tr.endpos;
	tr_ent[TOPLEFT]  = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[TOPLEFT] = tr.plane.normal;
		tr_fraction[TOPLEFT] = tr.fraction;
	}

	if ( tr_fraction[TOPLEFT] < 1.0f || tr_fraction[MIDLEFT] < 1.0f || tr_fraction[BOTTOMLEFT] < 1.0f )
	{
		terrain.left_blocked = TRUE;
	}

	//	find closest obstruction on left
	terrain.left_dist = tr_dist[BOTTOMLEFT];
	if ( terrain.left_dist > tr_dist[MIDLEFT] )
	{
		terrain.left_dist = tr_dist[MIDLEFT];
	}
	if ( terrain.left_dist > tr_dist[TOPLEFT] )
	{
		terrain.left_dist = tr_dist[TOPLEFT];
	}

	/////////////////////////////////////////////////////////////////////////
	//	trace lines down right side of bounding box
	/////////////////////////////////////////////////////////////////////////

	//	lower RIGHT
	start	= org + dir_right * box_width;
	start.z -= (self->s.mins.z - 0.1f);
	end		= start + dir * dist;
	tr		= gstate->TraceLine_q2 (start, end, self, MASK_SOLID);
	tr_dist[BOTTOMRIGHT] = tr.fraction * dist;
	tr_end[BOTTOMRIGHT]  = tr.endpos;
	tr_ent[BOTTOMRIGHT]  = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[BOTTOMRIGHT] = tr.plane.normal;
		tr_fraction[BOTTOMRIGHT] = tr.fraction;
	}

	//	middle RIGHT
	start.z += (self->s.mins.z - 0.1f);
	end.z	+= (self->s.mins.z - 0.1f);
	tr		= gstate->TraceLine_q2(start, end, self, MASK_SOLID);
	tr_dist[MIDRIGHT] = tr.fraction * dist;
	tr_end[MIDRIGHT]  = tr.endpos;
	tr_ent[MIDRIGHT]  = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[MIDRIGHT] = tr.plane.normal;
		tr_fraction[MIDRIGHT] = tr.fraction;
	}

	//	top RIGHT
	start	= org + dir_right * box_width;
	start.z += self->s.maxs.z;
	end		= start + dir * dist;
	tr		= gstate->TraceLine_q2(start, end, self, MASK_SOLID);
	tr_dist[TOPRIGHT] = tr.fraction * dist;
	tr_end[TOPRIGHT]  = tr.endpos;
	tr_ent[TOPRIGHT]  = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[TOPRIGHT] = tr.plane.normal;
		tr_fraction[TOPRIGHT] = tr.fraction;
	}

	if ( tr_fraction[TOPRIGHT] < 1.0f || tr_fraction[MIDRIGHT] < 1.0f || tr_fraction[BOTTOMRIGHT] < 1.0f )
	{
		terrain.right_blocked = TRUE;
	}

	//	find closest obstruction on right
	terrain.right_dist = tr_dist[BOTTOMRIGHT];
	if ( terrain.right_dist > tr_dist[MIDRIGHT] )
	{
		terrain.right_dist = tr_dist[MIDRIGHT];
	}
	if ( terrain.right_dist > tr_dist[TOPRIGHT] )
	{
		terrain.right_dist = tr_dist[TOPRIGHT];
	}

	if ( tr_fraction[TOPLEFT] < 1.0f && tr_fraction[MIDLEFT] < 1.0f && tr_fraction[BOTTOMLEFT] < 1.0f &&
		 tr_fraction[TOPRIGHT] < 1.0f && tr_fraction[MIDRIGHT] < 1.0f && tr_fraction[BOTTOMRIGHT] < 1.0f )
	{
		if ( tr_fraction[TOPLEFT] < tr_fraction[TOPRIGHT] || 
			 tr_fraction[MIDLEFT] < tr_fraction[MIDRIGHT] ||
			 tr_fraction[BOTTOMLEFT] < tr_fraction[BOTTOMRIGHT] )
		{
			terrain.wall_type = TER_WALL_LEFT;
			return TER_WALL_LEFT;
		}

		if ( tr_fraction[TOPLEFT] > tr_fraction[TOPRIGHT] || 
			 tr_fraction[MIDLEFT] > tr_fraction[MIDRIGHT] ||
			 tr_fraction[BOTTOMLEFT] > tr_fraction[BOTTOMRIGHT] )
		{
			terrain.wall_type = TER_WALL_RIGHT;
			return TER_WALL_RIGHT;
		}
		
		terrain.wall_type = TER_WALL;
		return TER_WALL;
	}

	if ( (tr_fraction[TOPLEFT] < 1.0f || tr_fraction[TOPRIGHT] < 1.0f) && 
		 (tr_fraction[BOTTOMLEFT] == 1.0f && tr_fraction[BOTTOMRIGHT] == 1.0f) )
	{
		terrain.wall_type = TER_HIGH_OBS;
		return TER_HIGH_OBS;	
	}

	if ( (tr_fraction[BOTTOMLEFT] < 1.0f || tr_fraction[BOTTOMRIGHT] < 1.0f) &&
		 (tr_fraction[TOPLEFT] == 1.0f && tr_fraction[TOPRIGHT] == 1.0f) )
	{
		terrain.wall_type = TER_LOW_OBS;
		return TER_LOW_OBS;	
	}

	if ( terrain.right_blocked && !terrain.left_blocked )
	{
		terrain.wall_type = TER_WALL_RIGHT;
		return TER_WALL_RIGHT;
	}

	if ( !terrain.right_blocked && terrain.left_blocked )
	{
		terrain.wall_type = TER_WALL_LEFT;
		return TER_WALL_LEFT;
	}
	
	if ( terrain.right_blocked && terrain.left_blocked )
	{
		// return general obstruction flag because somehow shooting the rays did
		// not catch the obstruction, but the box did
		terrain.wall_type = TER_OBSTRUCTED;
		terrain.wall_normal = savedWallNormal;

		return TER_OBSTRUCTED;
	}

	//	if not blocked or wall is far away, return clear
	if ( !terrain.right_blocked && !terrain.left_blocked )
	{
		// return general obstruction flag because somehow shooting the rays did
		// not catch the obstruction, but the box did
		terrain.wall_type = TER_OBSTRUCTED;
		terrain.wall_normal = savedWallNormal;

		return TER_OBSTRUCTED;
	}

	//////////////////////////////////////////////////////////////////////////
	//	show markers for debugging
	//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#ifdef	SHOWMARKERS
	if (self->flags & FL_DEBUG)
	{
		for (i = 0; i < 6; i++)
			com->SpawnMarker (self, tr_end[i], debug_models [DEBUG_MARKER], 0.2);
	}
#endif
#endif

#ifdef _DEBUG
#ifdef	SHOWOUTPUT
	switch	(terrain.wall_type)
	{
		case	TER_GRADEUP:
			ai_debug_print (self, "TER_GRADEUP\n");
			break;
		case	TER_STEPSUP:
			ai_debug_print (self, "TER_STEPSUP\n");
			break;
		case	TER_FAR_WALL:
			ai_debug_print (self, "TER_FAR_WALL\n");
			break;
		case	TER_WALL:
			ai_debug_print (self, "TER_WALL\n");
			break;
		case	TER_WALL_LEFT:
			ai_debug_print (self, "TER_WALL_LEFT\n");
			break;
		case	TER_WALL_RIGHT:
			ai_debug_print (self, "TER_WALL_RIGHT\n");
			break;
		case	TER_LOW_OBS:
			ai_debug_print (self, "TER_LOW_OBS\n");
			break;
		case	TER_CLEAR:
			ai_debug_print (self, "TER_CLEAR\n");
			break;
	}

	if (terrain.left_blocked)
	{
		ai_debug_print(self, "LB = TRUE, ");
	}
	else
	{
		ai_debug_print(self, "LB = FALSE, ");
	}

	if (terrain.right_blocked)
	{
		ai_debug_print(self, "RB = TRUE\n");
	}
	else
	{
		ai_debug_print(self, "RB = FALSE\n");
	}

	ai_debug_print(self, "LBN = %.1f %.1f %.1f, ", tr_normal[BOTTOMLEFT].x, tr_normal[BOTTOMLEFT].y, tr_normal[BOTTOMLEFT].z); 
	ai_debug_print(self, "RBN = %.1f %.1f %.1f\n", tr_normal[BOTTOMRIGHT].x, tr_normal[BOTTOMRIGHT].y, tr_normal[BOTTOMRIGHT].z);

	ai_debug_print(self, "LMN = %.1f %.1f %.1f, ", tr_normal[MIDLEFT].x, tr_normal[MIDLEFT].y, tr_normal[MIDLEFT].z); 
	ai_debug_print(self, "RMN = %.1f %.1f %.1f\n", tr_normal[MIDRIGHT].x, tr_normal[MIDRIGHT].y, tr_normal[MIDRIGHT].z);

	ai_debug_print(self, "LTN = %.1f %.1f %.1f, ", tr_normal[TOPLEFT].x, tr_normal[TOPLEFT].y, tr_normal[TOPLEFT].z); 
	ai_debug_print(self, "RTN = %.1f %.1f %.1f\n", tr_normal[TOPRIGHT].x, tr_normal[TOPRIGHT].y, tr_normal[TOPRIGHT].z);

	ai_debug_print(self, "obs_height = %.2f, max_height = %.2f\n", obs_height, max_height);
#endif
#endif
	return TER_CLEAR;
}

// ----------------------------------------------------------------------------
//
// Name:		IsDead
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int IsDead( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->deadflag != DEAD_NO )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CanAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_CanAttack( userEntity_t *self, userEntity_t *enemy )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( hook->nAttackType & ATTACK_GROUND_MELEE )
	{
	}
	if ( hook->nAttackType & ATTACK_GROUND_MELEE )
	{
	}
	if ( hook->nAttackType & ATTACK_GROUND_RANGED )
	{
	}
	if ( hook->nAttackType & ATTACK_AIR_MELEE )
	{
	}
	if ( hook->nAttackType & ATTACK_AIR_SWOOP_MELEE )
	{
	}
	if ( hook->nAttackType & ATTACK_AIR_RANGED )
	{
	}
	if ( hook->nAttackType & ATTACK_AIR_SWOOP_RANGED )
	{
	}
	if ( hook->nAttackType & ATTACK_WATER_MELEE )
	{
	}
	if ( hook->nAttackType & ATTACK_WATER_RANGED )
	{
	}

	return FALSE;
}

int AI_CanAttack( userEntity_t *self )
{
    _ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

    if ( hook->ai_flags & AI_ATTACK )
    {
        return TRUE;
    }
    
    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeTurningDiameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_ComputeTurningDiameter( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fNumSecondsToFullCircle = ONE_OVER_360 * self->ang_speed[YAW];
	float fCircumference = fNumSecondsToFullCircle * hook->attack_speed;
	float fDiameter = fCircumference * ONE_OVER_PI;

	return fDiameter;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CanDodge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_CanDodge( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( (hook->ai_flags & AI_CANDODGE) && AI_CanMove( hook ) )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DoEvasiveAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoEvasiveAction( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_CanDodge( self ) == TRUE )
	{	
		if ( hook->dflags & DFL_RANGEDATTACK )
		{
			if ( (hook->ai_flags & AI_SNIPE) || rnd() < 0.5f )
			{
				if ( rnd() > 0.5f )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_STRAFE );
				}
				else
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
				}
			}
			else
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_DODGE );
			}
		}
		else
		{
			if ( rnd() < 0.5f )
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
				AI_SetTaskFinishTime( hook, 0.5f );
			}
			else
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_STRAFE );
				AI_SetTaskFinishTime( hook, 0.5f );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:          WillItBeCaughtInLoop
// Description:   Test if the current destination is too close and too sharp
//                a turn in order for the hover to make it
// Input:
// Output:
//                TRUE  => if hover can not make it using current turn rate
//                FALSE => otherwise
//                *pfDiameter => the diameter of the hovercraft's turn rate
// Note:
//
// ----------------------------------------------------------------------------
int AI_WillItBeCaughtInLoop( userEntity_t *self, const CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector directionVector;
	directionVector = destPoint - self->s.origin;
	directionVector.z = 0;
	directionVector.Normalize();

	CVector angleTowardPoint;
	directionVector.VectorToAngles( angleTowardPoint );

	CVector facingAngle = self->s.angles;
	if ( AI_IsSameAngle2D( self, facingAngle, angleTowardPoint ) == TRUE )
	{
		return FALSE;
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );

	// compute the turning circle's diameter
	float fNumUpdates = 360.0f / (float)self->ang_speed.yaw;
	float fCircumference = fNumUpdates * fSpeed;
	float fDiameter = (fCircumference / M_PI) * 1.2f;

	float fDistance = VectorXYDistance( self->s.origin, destPoint );
	if ( fDistance < fDiameter )
	{
		CVector facingDir;
		YawToVector( self->s.angles.yaw, facingDir );

		float fRadius = (fDiameter * 0.5f);

		// try the left side
		CVector leftVector;
        AI_LeftVector( facingDir, leftVector );

		CVector centerPoint;
		VectorMA( self->s.origin, leftVector, fRadius, centerPoint );

		fDistance = VectorXYDistance( centerPoint, destPoint );
		if ( fDistance < (fRadius + 16.0f) )
		{
			return TRUE;
		}

		// now, try the other side
		// right side
        CVector rightVector;
        AI_RightVector( facingDir, rightVector );
		VectorMA( self->s.origin, rightVector, fRadius, rightVector );

		fDistance = VectorXYDistance( centerPoint, destPoint );
		if ( fDistance < (fRadius + 16.0f) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:          FindTurnRateAdjuster
// Description:
// Input:
// Output:
// Note:
//                The purpose of this function is to find a turn rate modifier, so
//                that it can make the turn without having to deal with getting
//                caught in a loop
//
// ----------------------------------------------------------------------------
float AI_FindTurnRateAdjuster( userEntity_t *self, const CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fDistance = VectorDistance( self->s.origin, destPoint );
	
	float fTurnRateAdjuster = 5.0;
	while ( fTurnRateAdjuster < self->ang_speed.yaw )
	{
		// compute the turning circle's diameter
		float fNumUpdates = 360.0f / (float)(self->ang_speed.yaw + fTurnRateAdjuster);
		float fCircumference = fNumUpdates * fSpeed;
		float fDiameter = fCircumference / M_PI;

		if ( fDiameter < fDistance )
		{
			break;
		}

		fTurnRateAdjuster += 5.0f;
	}

	return fTurnRateAdjuster;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsCollisionWithOtherMonsters
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_IsCollisionWithOtherMonsters( userEntity_t *self )
{
	_ASSERTE( self );

	float fSelfRadius = (self->s.maxs.y - self->s.mins.y) * 1.5f;
	float fSelfHeight = self->s.maxs.z - self->s.mins.z;
	
	userEntity_t *pMonster = alist_FirstEntity( monster_list );
	while ( pMonster )
	{
		if ( self != pMonster )
		{
			float fMonsterRadius = (pMonster->s.maxs.y - pMonster->s.mins.y) * 1.5f;
			float fMonsterHeight = pMonster->s.maxs.z - pMonster->s.mins.z;

			float fMaxHeight = __max( fSelfHeight, fMonsterHeight );

			float fYDistance = VectorYDistance(self->s.origin, pMonster->s.origin);
			float fZDistance = VectorZDistance(self->s.origin, pMonster->s.origin);
			if ( fYDistance < (fSelfRadius + fMonsterRadius) && fZDistance < fMaxHeight )
			{
				return pMonster;
			}
		}

		pMonster = alist_NextEntity( monster_list );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_register_sounds
// Description:
//  registers the sounds for this ai with the sound engine
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_register_sounds(userEntity_t* self)
{
	_ASSERTE( self );
    _ASSERTE( self->pMapAnimationToSequence );

    char* key = NULL;
	frameData_t* pFrameData = NULL;

	POSITION pos = self->pMapAnimationToSequence->GetStartPosition();
	while ( pos )
	{
		self->pMapAnimationToSequence->GetNextAssoc(pos,key,(void*&)pFrameData);
		if ( pFrameData )
		{
			if ( pFrameData->sound1 && strlen( pFrameData->sound1 ) > 0 )
			{
				gstate->SoundIndex(pFrameData->sound1);
			}
			if ( pFrameData->sound2 && strlen( pFrameData->sound2 ) > 0 )
			{
				gstate->SoundIndex(pFrameData->sound2);
			}
		}
	}

    // now precache sight sounds
    playerHook_t *hook = AI_GetPlayerHook( self );
    CPtrArray *pSoundArray = SIGHTSOUND_GetArray( hook->type );
    int nNumSightSounds = pSoundArray->GetSize();
	for ( int i = 0; i < nNumSightSounds; i++ )
	{
        char *pSound = (char *)(*pSoundArray)[i];
        if ( strlen( pSound ) > 0 )
        {
            gstate->SoundIndex( pSound );
        }
    }

}


int AI_4CastTrace(CVector &Origin, float x, float z, CVector &Destination,userEntity_t *self, unsigned long Mask);
#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif
// ----------------------------------------------------------------------------
// NSS[2/14/00]:
// Name:		AI_IsGroundBelowBetween
// Description:
// Input:
// Output:
// Note:Tweaked this a bit
//
// ----------------------------------------------------------------------------
int AI_IsGroundBelowBetween( userEntity_t *self, CVector &point1, CVector &point2, 
                             float fCheckGroundDistance /* = CHECK_GROUND_DISTANCE */)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.1f;

	int bGround = TRUE;

	CVector bottomPoint = point2;
	bottomPoint.z -= fCheckGroundDistance;
	tr = gstate->TraceLine_q2( point1, bottomPoint, NULL, MASK_SOLID );
	bottomPoint.z *= tr.fraction;
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		float fDistance = VectorDistance( point1, point2 );

		float fFractionDistance = fOneFrameSpeed;

		CVector vector = point2 - point1;
		vector.Normalize();
		
		float fCurrentDistance = fFractionDistance;
		CVector nextPoint;
		while ( fCurrentDistance < fDistance )
		{
			VectorMA( point1, vector, fCurrentDistance, nextPoint );
			CVector bottomPoint = nextPoint;
			bottomPoint.z -= fCheckGroundDistance;
			
			tr = gstate->TraceLine_q2( nextPoint, bottomPoint, NULL, MASK_SOLID);
			if ( tr.fraction >= 1.0f && !tr.startsolid )
			{
				bGround = FALSE;
				break;
			}

			fCurrentDistance += fFractionDistance;
		}
	}
	else
	{
		bGround = FALSE;
	}

	return bGround;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetPlayerHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
playerHook_t *AI_GetPlayerHook( userEntity_t *self )
{
	if( self == NULL )
	{
		return NULL;
	}

	playerHook_t *hook = (playerHook_t *)self->userHook;
//	_ASSERTE( hook );

	return hook;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsAlive
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsAlive( userEntity_t *self )
{
	
	if( self == NULL )
	{
		return FALSE;
	}

	if(self->health < 0 && self->flags & FL_MONSTER)
	{
		return FALSE;
	}
	if( self->className == NULL )
	{
		return FALSE;
	}

	if ( self->deadflag == DEAD_NO && self->health > 0 && self->inuse &&
		 _stricmp( self->className, "freed" ) != 0 &&
		 _stricmp( self->className, "noclass" ) != 0 )
	{
		return TRUE;
	}

	// SCG[10/6/99]: If this is a bsp entity, return true.
	// SCG[11/2/99]: Sigh, hack.
	/*if( (( self->solid == SOLID_BSP || self->solid == SOLID_NOT || self->solid == SOLID_TRIGGER )) &&
			_stricmp( self->className, "freed" ) != 0 &&
			 _stricmp( self->className, "noclass" ) != 0 ) 
	{

		playerHook_t *hook = AI_GetPlayerHook( self );
		//Sidekicks getting punked by monsters goes here
		if(hook)
		{
			if(hook->type == TYPE_MIKIKO || hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
			{
				if((self->deadflag != DEAD_PUNK))
					return TRUE;
				else
					return FALSE;
			}
		}


	}*/
	
	//NSS[11/23/99]:Hack ontop of a hack.. we DO NOT want to return a true if we are the player and we are PUNK DEAD!
	if((self->flags & FL_CLIENT|FL_BOT) && (self->deadflag == DEAD_PUNK || self->deadflag == DEAD_FROZEN) && (sv_violence->value == 0))
		return FALSE;
	else
		return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetMovingCounter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetMovingCounter( playerHook_t *hook, int nValue )
{
	_ASSERTE( hook );

	hook->nMovingCounter = nValue;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsDucking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsDucking( playerHook_t *hook )
{
	_ASSERTE( hook );
	return hook->bDucking;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartDucking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartDucking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( hook->bDucking == FALSE );

	self->s.maxs.z -= 32.0f;

	hook->bDucking = TRUE;


	char szAnimation[16];
	//AI_SelectDuckingAnimation( self, szAnimation );
    AI_ForceSequence(self, szAnimation, FRAME_LOOP);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_EndDucking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_EndDucking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( hook->bDucking == TRUE );

	self->s.maxs.z += 32.0f;

	hook->bDucking = FALSE;

	if ( AI_IsStateIdle( hook ) )
	{
	}
	else
	if ( AI_IsStateWalking( hook ) )
	{
	}
	else
	if ( AI_IsStateRunning( hook ) )
	{

	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetRunFollowDistance
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_GetRunFollowDistance( playerHook_t *hook )
{
	_ASSERTE( hook );

	float fFollowingDistance = hook->follow_dist;
	if ( hook->bDucking )
	{
		fFollowingDistance = 64.0f;
	}

	return fFollowingDistance;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetWalkFollowDistance
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_GetWalkFollowDistance( playerHook_t *hook )
{
	_ASSERTE( hook );

	float fWalkFollowingDistance = hook->walk_follow_dist;
	if ( hook->bDucking )
	{
		fWalkFollowingDistance = 64.0f;
	}

	return fWalkFollowingDistance;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UpdateCurrentNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_UpdateCurrentNode( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	node_find_cur_node( self, pNodeList );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ZeroVelocity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ZeroVelocity( userEntity_t *self )
{
	_ASSERTE( self );

	self->velocity.Zero();
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetVelocity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetVelocity( userEntity_t *self, CVector &vector, float fSpeed )
{
	_ASSERTE( self );

	self->movedir = vector;
    self->velocity = vector * fSpeed;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		AI_GetStraightMoveDistance
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_GetStraightMoveDistance( playerHook_t *hook )
{
	float fDistance = STRAIGHT_MOVE_DISTANCE;
	if ( AI_IsSidekick( hook ) )
	{
		fDistance = hook->walk_follow_dist;
	}
	
	return fDistance;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsOkToMoveStraight
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsOkToMoveStraight( userEntity_t *self, CVector &destPoint, float fXYDistance, float fZDistance )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) || hook->ai_flags & AI_DROWNING || self->flags & FL_CINEMATIC)
    {
        if ( AI_IsLineOfSight( self, destPoint ) )
        {
            return TRUE;
        }
    }
    else
    {
		CVector Mins = self->s.mins;
		CVector QuarterDest,Dir;
		float fSpeed = self->velocity.Length() * 0.125;
		Dir = destPoint - self->s.origin;
		Dir.Normalize();
		QuarterDest = self->s.origin + (Dir * fSpeed);
		Mins.z += 4.0f;
		tr = gstate->TraceBox_q2( self->s.origin,Mins,self->s.maxs,QuarterDest, self,MASK_SOLID);
	    if ( hook->ai_flags & AI_ALWAYS_USENODES )
	    {
		    if ( (tr.fraction >= 1.0f && fXYDistance < AI_GetStraightMoveDistance( hook ) && fZDistance < MAX_JUMP_HEIGHT) ||
			     (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fXYDistance+fZDistance)) < 96.0f) )
		    {
				if ( AI_IsGroundBelowBetween( self, self->s.origin, destPoint,256.0f ) )
			    {
				    return TRUE;
			    }
		    }
	    }
	    else
	    {
		    if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
			     (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fXYDistance+fZDistance)) < 32.0f) )
		    {
			    if ( AI_IsGroundBelowBetween( self, self->s.origin, destPoint ) )
			    {
				    return TRUE;
			    }
		    }
	    }
    }

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ClearTeam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ClearTeam( userEntity_t *self )
{
    self->team = 0;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetToSameTeam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetToSameTeam( userEntity_t *pOwner, userEntity_t *pEntity )
{
    pEntity->team = pOwner->team;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetOwner( userEntity_t *self, userEntity_t *pOwner )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->owner = pOwner;
	if ( pOwner )
	{
		playerHook_t *pOwnerHook = AI_GetPlayerHook( pOwner );
		if ( AI_IsAlive( pOwner ) && pOwnerHook->type == TYPE_CLIENT )
		{
			AI_SetToSameTeam( pOwner, self );

            AIINFO_AddSidekick( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_InitNodeList
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_InitNodeList( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !hook->pNodeList )
	{
		int nNodeType;
		NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
		if ( pNodeHeader )
		{
			hook->pNodeList = node_create_nodelist(pNodeHeader);
			hook->pNodeList->pNodeHeader = pNodeHeader;
			if ( self->movetype != MOVETYPE_WALK && self->movetype != MOVETYPE_WHEEL &&
				 self->movetype != MOVETYPE_HOP )
			{
				// VERY IMPORTANT: if not on ground then the current index should be set to -1
				hook->pNodeList->nCurrentNodeIndex = -1;
			}
			else
			{
				node_find_cur_node( self, hook->pNodeList );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DeleteNodeList
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
NODELIST_PTR AI_DeleteNodeList( NODELIST_PTR pNodeList )
{
//	delete pNodeList;
    gstate->X_Free(pNodeList);

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_WhichSide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

__inline static int Area( CVector &a, CVector &b, CVector &c )
{
    // this is basically a cross product of three vectors
    return  a.x * b.y - a.y * b.x +
            a.y * c.x - a.x * c.y +
            b.x * c.y - c.x * b.y;
}

int AI_WhichSide( CVector &startPoint, CVector &endPoint, CVector &testPoint )
{
    int nArea = Area( startPoint, endPoint, testPoint );
    if ( nArea > 0 )
    {
        return ON_LEFT;
    }
    else
    if ( nArea < 0 )
    {
        return ON_RIGHT;
    }

    return COLLINEAR;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ComputeRealWorldWeaponOffset
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ComputeAbsoluteWeaponOffset( userEntity_t *self, const CVector &weaponOffset, CVector &absoluteOffset )
{
	self->s.angles.AngleToVectors(forward, right, up);

	absoluteOffset = self->s.origin + (weaponOffset.x * right) + 
				    (weaponOffset.y * forward) + (weaponOffset.z * up);
}

// ----------------------------------------------------------------------------
//
// Name:        AI_IsClearShot
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsClearShot( userEntity_t *self, userEntity_t *pEntity )
{
	ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
    if ( !pWeapon )
    {
        return FALSE;
    }
    
	self->s.angles.AngleToVectors(forward, right, up);

	CVector weaponOrigin = self->s.origin + (pWeapon->ofs.x * right) + 
				           (pWeapon->ofs.y * forward) + (pWeapon->ofs.z * up);

	tr = gstate->TraceLine_q2( weaponOrigin, pEntity->s.origin, self, MASK_SHOT );
    if ( tr.ent && AI_IsAlive( tr.ent ) && tr.ent != pEntity )
    {
        playerHook_t *hook = AI_GetPlayerHook( self );

        userEntity_t *pHitEntity = tr.ent;
        playerHook_t *pHitEntityHook = (playerHook_t *)pHitEntity->userHook;
        if ( pHitEntityHook )
        {
            if ( ((hook->dflags & DFL_EVIL) && (pHitEntityHook->dflags & DFL_EVIL)) ||
                 ((hook->dflags & DFL_GOOD) && (pHitEntityHook->dflags & DFL_GOOD)) )
            {
                return FALSE;
            }
        }
    }

//    if ( tr.ent && (tr.ent == pEntity || !AI_IsAlive(tr.ent)) )
//    {
//        playerHook_t *hook = AI_GetPlayerHook( self );
//
//        userEntity_t *pHitEntity = tr.ent;
//        playerHook_t *pHitEntityHook = (playerHook_t *)pHitEntity->userHook;
//        if ( pHitEntityHook )
//        {
//            if ( ((hook->dflags & DFL_EVIL) && (pHitEntityHook->dflags & DFL_GOOD)) ||
//                 ((hook->dflags & DFL_GOOD) && (pHitEntityHook->dflags & DFL_EVIL)) )
//            {
//                return TRUE;
//            }
//        }
//
//    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_GetEntityOrigin
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_GetEntityOrigin( userEntity_t *self, CVector &origin )
{
	if ( self->s.origin.Length() == 0.0f )
	{
        origin.x = (self->absmax.x + self->absmin.x) * 0.5f;
		origin.y = (self->absmax.y + self->absmin.y) * 0.5f;
		origin.z = (self->absmax.z + self->absmin.z) * 0.5f;
	}
    else
    {
        origin = self->s.origin;
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DoDeathTarget
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoDeathTarget( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// do we need to be firing off some triggers or something?
//	if ( hook->nSpawnValue & SPAWN_DEATHTARGET )
	// SCG[11/8/99]: Deathtarget flag removed.
	if ( self->deathtarget != NULL )
	{
		userEntity_t	*pHead = gstate->FirstEntity();

		// NSS[3/1/00]:Hack hack... hack. 
		// NSS[3/1/00]:This was done because of poop.
		char *Buffer = strdup(self->deathtarget);
		//NSS[11/11/99]:Added loop... never was looping through all entities. !ack!
		while(pHead)
		{
			//NSS[11/11/99]:Check everything make sure NOTHING is invalid data!!!
			if(pHead)
			{
				if(pHead->targetname)
				{
					if( strcmp( pHead->targetname, Buffer ) == 0 )
					{
						if( pHead->use )
						{
							pHead->use( pHead, self, self );
						}	
					}
				}
			}
			pHead = gstate->NextEntity( pHead );
		}
		// NSS[3/1/00]:End of hack
		free(Buffer);
	}

	self->deathtarget = NULL;
}

bool ITEM_CanPath( CVector &point1, CVector &point2 );
// ----------------------------------------------------------------------------
//
// Name:		AI_DoSpawnName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoSpawnName( userEntity_t *self )
{
	
	// if this entity has 'spawnname' defined, try to spawn an entity using 'spawnname' 
	// as the classname
	if (self->spawnname)
	{
		// spawn a monster of class self->spawnname
		bool bIsMonster = bool(strstr(self->spawnname, "monster") != NULL);
		userEntity_t *pNewEntity = com->SpawnDynamicEntity (self, self->spawnname, bIsMonster);
		
		//NSS[11/16/99]:Added this so monsters can pass along spawnflags.
		pNewEntity->spawnflags = self->nSidekickFlag;
		// SCG[1/23/00]: pNewEntity needs to be checked for both cases
		//        if ( pNewEntity && bIsMonster )
        if ( pNewEntity )
		{
			if( bIsMonster )
			{
				char szModelName[64];
				int nType = GetMonsterInfo( self->spawnname, szModelName );
				
				int i = 0;
				while(pNewEntity->epair[i].key != NULL)
				{
					if(!stricmp(pNewEntity->epair[i].key, "spawnname"))
					{
						pNewEntity->epair[i].value = NULL;
					}
					i++;
				}
				
				if ( nType < 0 )
				{
					com->Warning( "Monster %s was not found", self->spawnname );
					return;
				}
				
				SPAWN_CallInitFunction( pNewEntity, self->spawnname );

				IncrementMonsterCount();
				pNewEntity->spawnname = NULL;
			}
			else
			{
				
				// SCG[2/24/00]: sad hack to make sure the boost pieces stay together
				if ( _stricmp( pNewEntity->className, "item_power_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_acro_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_attack_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_speed_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_vita_boost" ) == 0 )
				{
					itemHook_t *pItemHook = ( itemHook_t * )pNewEntity->userHook;
					if( pItemHook->link != NULL )
					{
						pNewEntity->s.origin = self->s.origin;
						pItemHook->link->s.origin = self->s.origin;
						gstate->LinkEntity( pItemHook->link );
					}
				}

				userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
				
				// NSS[1/11/00]:If there are no sidekicks do not update the lists
				if(!pFirstSidekick)
					return;
				
				CVector point1 = pFirstSidekick->s.origin;  
				
				if ( strstr( pNewEntity->className, "weapon" ) )
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pWeaponList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
				else if ( strstr( pNewEntity->className, "ammo" ) )
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pAmmoList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
				else if (strstr( pNewEntity->className, "armor")) // NSS[12/16/99]:This should register all armor types...wasn't doing this before.
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pArmorList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
				else if ( _stricmp( pNewEntity->className, "item_power_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_acro_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_attack_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_speed_boost" ) == 0 ||
					_stricmp( pNewEntity->className, "item_vita_boost" ) == 0 )
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pStatBoostList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
				else if ( strstr( pNewEntity->className, "item_health" ) )
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pHealthList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
				else if ( _stricmp( pNewEntity->className, "item_goldensoul" ) == 0 )
				{
					if ( !deathmatch->value || ITEM_CanPath( point1, pNewEntity->s.origin ) )
					{
						pGoldenSoulList->AddTail( pNewEntity );
					}
					else
					{
						AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							pNewEntity->className, 
							pNewEntity->s.origin.x,
							pNewEntity->s.origin.y,
							pNewEntity->s.origin.z );
					}
				}
			}
		}
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Suicide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Suicide( userEntity_t *self, int damage )
{
	com->Damage( self, self, self, CVector(0, 0, 0), CVector(0, 0, 0), damage, DAMAGE_NONE );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_IsSidekick
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsSidekick( playerHook_t *hook )
{
    if(hook)
	{
		if ( hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKO || hook->type == TYPE_MIKIKOFLY)
		{
			return TRUE;
		}
	}
    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_IsSuperfly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsSuperfly( playerHook_t *hook )
{
    if ( hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_IsMikiko
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsMikiko( playerHook_t *hook )
{
    if ( hook->type == TYPE_MIKIKO )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_IsPointObstructedByEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsPointObstructedByEntity( userEntity_t *self, const CVector &point )
{
	userEntity_t *head = gstate->FirstEntity();

	while ( head )
	{
		if ( self != head )
        {
            if ( head->flags & FL_MONSTER || head->flags & FL_BOT || head->flags & FL_CLIENT )
		    {
		        float fDistance = VectorDistance( head->s.origin, point );
                if ( fDistance < 32.0f )
                {
                    return TRUE;
                }
            }
        }

		head = gstate->NextEntity(head);
	}

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_FindGroundPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindGroundPoint( userEntity_t *self, CVector &testPoint, CVector &groundPoint )
{
    // try tracing 64 units up and down

	groundPoint = testPoint;

    CVector bottomPoint, upPoint;

	bottomPoint = testPoint;
	bottomPoint.z -= 64.0f;
	tr = gstate->TraceLine_q2( testPoint, bottomPoint, self, MASK_SOLID );
	if ( tr.fraction >= 1.0f )
	{
		// try up
        upPoint = testPoint;
        upPoint.z += 64.0f;
        tr = gstate->TraceLine_q2( testPoint, upPoint, self, MASK_SOLID );
        if ( tr.fraction >= 1.0f )
        {
            // no ground fround
            return FALSE;
        }
        else
        {
            groundPoint.z = testPoint.z + (tr.fraction * 64.0f) + 24.0f;
        }
	}
    else
    {
        groundPoint.z = testPoint.z - (tr.fraction * 64.0f) + 24.0f;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsNeedHealth
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsNeedHealth( userEntity_t *self )
{
	if ( self->health < 50 )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsNeedWeapon
// Description:
// Input:
//				userEntity_t *self
//				
// Output:
//				TRUE	=> if need weapon
//				FALSE	=> otherwise
//				char *szWeaponName	=> name of the weapon needed
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsNeedWeapon( userEntity_t *self, char *szWeaponName )
{
	_ASSERTE( self->inventory );

	int bNeedWeapon = FALSE;

	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_shotcycler" );
			if ( !pWeapon )
			{
				strcpy( szWeaponName, "weapon_shotcycler" );

				pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_sidewinder" );
				if ( !pWeapon )
				{
					strcpy( szWeaponName, "weapon_sidewinder" );

					bNeedWeapon = TRUE;
				}
			}

			break;
		}	
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}

		default:
			break;
	}

	return bNeedWeapon;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsNeedAmmo
// Description:
// Input:
// Output:
//				TRUE	=> if need ammo
//				FALSE	=> otherwise
//				char *szAmmoName	=> name of the ammo needed
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsNeedAmmo( userEntity_t *self, char *szAmmoName )
{
	_ASSERTE( self->inventory );

	int bNeedAmmo = FALSE;

	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			weapon_t *pWeapon = (weapon_t *) self->curWeapon;
			if ( pWeapon )
			{
				if ( stricmp( pWeapon->name, "weapon_shotcycler" ) == 0 )
				{
					ammo_t *pAmmo = (ammo_t*)gstate->InventoryFindItem( self->inventory, "ammo_shells" );
					if ( pAmmo && pAmmo->count < 8 )
					{
						strcpy( szAmmoName, "ammo_shells" );

						bNeedAmmo = TRUE;
					}
				}
				else
				if ( stricmp( pWeapon->name, "weapon_sidewinder" ) == 0 )
				{
					ammo_t *pAmmo = (ammo_t*)gstate->InventoryFindItem( self->inventory, "ammo_rocket" );
					if ( pAmmo && pAmmo->count < 4 )
					{
						strcpy( szAmmoName, "ammo_rocket" );

						bNeedAmmo = TRUE;
					}
				}
				else
				if ( stricmp( pWeapon->name, "weapon_ionblaster" ) == 0 )
				{
					ammo_t *pAmmo = (ammo_t*)gstate->InventoryFindItem( self->inventory, "ammo_ionpack" );
					if ( pAmmo && pAmmo->count < 20 )
					{
						strcpy( szAmmoName, "ammo_ionpack" );

						bNeedAmmo = TRUE;
					}
				}
			}

			break;
		}	
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}

		default:
			break;
	}

	return bNeedAmmo;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DetermineNeededItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_DetermineNeededItem( userEntity_t *self )
{
	_ASSERTE( self->inventory );

	userEntity_t *pNeedItem = NULL;
	
	char szItemName[64];
	if (  AI_IsNeedHealth( self ) == TRUE )
	{
		pNeedItem = ITEM_GetClosestHealth( self );
	}
	else
	if ( AI_IsNeedWeapon( self, szItemName ) == TRUE )
	{
		pNeedItem = ITEM_GetClosestWeapon( self, szItemName );
	}
	else
	if ( AI_IsNeedAmmo( self, szItemName ) == TRUE )
	{
		pNeedItem = ITEM_GetClosestAmmo( self, szItemName );
	}

	return pNeedItem;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DetermineWantItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_DetermineWantItem( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pWantItem = NULL;
	if ( self->health >= 90 && self->health < ((hook->base_health*1.5f) - 25) )
	{
		// find a goldensoul
		pWantItem = ITEM_GetClosestGoldenSoul( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	else
	if ( self->health < 90 )
	{
		pWantItem = ITEM_GetClosestHealth( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	if ( self->armor_val <= 75 )
	{
		// find a nearest armor	
		pWantItem = ITEM_GetClosestArmor( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	else
	if ( self->armor_val <= 150 )
	{
		// find a chromatic armor
		pWantItem = ITEM_GetClosestArmor( self, "item_plasteel_armor" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	// look for stat boost
	if ( !(hook->items & IT_POWERBOOST) )
	{
		pWantItem = ITEM_GetClosestStatBoost( self, "item_power_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_ATTACKBOOST) )
	{
		pWantItem = ITEM_GetClosestStatBoost( self, "item_attack_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_SPEEDBOOST) )
	{
		pWantItem = ITEM_GetClosestStatBoost( self, "item_speed_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_ACROBOOST) )
	{
		pWantItem = ITEM_GetClosestStatBoost( self, "item_acro_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_VITABOOST) )
	{
		pWantItem = ITEM_GetClosestStatBoost( self, "item_vita_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	// now look for weapons
	_ASSERTE( self->inventory );

	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_shotcycler" );
			if ( !pWeapon )
			{
				// find a closest shotcyler
				pWantItem = ITEM_GetClosestWeapon( self, "weapon_shotcycler" );
				if ( pWantItem )
				{
					return pWantItem;
				}
			}

			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_sidewinder" );
			if ( !pWeapon )
			{
				// find a closest sidewinder
				pWantItem = ITEM_GetClosestWeapon( self, "weapon_sidewinder" );
				if ( pWantItem )
				{
					return pWantItem;
				}
			}

			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_ionblaster" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem = ITEM_GetClosestWeapon( self, "weapon_ionblaster" );
				if ( pWantItem )
				{
					return pWantItem;
				}
			}

			break;
		}	
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}

		default:
			break;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsWeaponAvailable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsWeaponAvailable( userEntity_t *self, char *szWeaponName )
{
	_ASSERTE( self );
	
	if ( !self->inventory )
	{
		return FALSE;
	}

	weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, szWeaponName );
	if ( !pWeapon )
	{
		return FALSE;
	}

	// see if we have ammo for the weapon
	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			ammo_t *pAmmo = NULL;
			if ( _stricmp(szWeaponName, "weapon_c4") == 0 )
			{
				pAmmo = (ammo_t *)gstate->InventoryFindItem( self->inventory, "ammo_c4" );
			}
			else 
			if ( _stricmp(szWeaponName, "weapon_ionblaster") == 0 )
			{
				pAmmo = (ammo_t *)gstate->InventoryFindItem( self->inventory, "ammo_ionpack" );
			}
			else 
			if ( _stricmp(szWeaponName, "weapon_shotcycler") == 0 )
			{
				pAmmo = (ammo_t *)gstate->InventoryFindItem( self->inventory, "ammo_shells" );
			}
			else 
			if ( _stricmp(szWeaponName, "weapon_sidewinder") == 0 )
			{
				pAmmo = (ammo_t *)gstate->InventoryFindItem( self->inventory, "ammo_rockets" );
			}

			if ( !pAmmo || (pAmmo && pAmmo->count == 0) )
			{
				return FALSE;
			}

			break;
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetTargetedEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_GetTargetedEntity( userEntity_t *self )
{
	CVector facingVector;
    self->s.angles.AngleToForwardVector( facingVector );

	CVector startPoint = self->s.origin;
    if ( self->flags & FL_CLIENT )
    {
	    startPoint.z += self->view_ofs.z;
    }

    CVector endPoint;
    VectorMA( startPoint, facingVector, 1000.0f, endPoint );
    tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0 && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
    {
    	return tr.ent;
    }

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:	AI_GetDirDot
// Description: Gets the dot product between two entities
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_GetDirDot( userEntity_t *self, userEntity_t *attacker )
{
	float deathdirdot = 0.0f; // default to death from the side
	if( self && attacker )
	{
		// if we have an inflictor, find the direction of death
		CVector inflictordir = attacker->s.origin - self->s.origin;
		CVector dir;

		// get direction, not distance
		inflictordir.Normalize();

		// find this entity's direction
		self->s.angles.AngleToForwardVector(dir);

		// find the dot product of the two directions
		deathdirdot = dir.DotProduct(inflictordir);
	}
	return deathdirdot;
}

// ----------------------------------------------------------------------------
//
// Name:	AI_GetDirDot
// Description: Decides if we are allowed to attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool AI_IsFrameInRange( userEntity_t *self, int attackFrame )
{
	if( self->lastAIFrame < attackFrame && self->s.frame >= attackFrame )
		return true;

	return false;
}

// ----------------------------------------------------------------------------
//
// Name:	    AI_ComputeDistanceToPoint
// Description: 
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_ComputeDistanceToPoint( userEntity_t *self, CVector &point )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// compute the time that may take to get there
	float fXYDistance = VectorXYDistance( self->s.origin, point );
	float fZDistance = VectorZDistance( self->s.origin, point );

	float fDistance = 0.0f;
	if ( AI_IsOkToMoveStraight( self, point, fXYDistance, fZDistance ) )
	{
		fDistance = VectorDistance( self->s.origin, point );
	}
	else
	{
		if ( AI_FindPathToPoint( self, point ) )
		{
			CVector currentPos = self->s.origin;

			NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
			PATHNODE_PTR pPath = hook->pPathList->pPath;
			while ( pPath )
			{
				MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
				fDistance += VectorDistance( currentPos, pNode->position );
				
				currentPos = pNode->position;

				pPath = pPath->next_node;
			}
		}
        else
        {
            fDistance = VectorDistance( self->s.origin, point );
        }
	}

	return fDistance;
}

// ----------------------------------------------------------------------------
//
// Name:	    AI_DisableAI
// Description: 
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DisableAI()
{
    bDisableAI = TRUE;
}
void AI_EnableAI()
{
    bDisableAI = FALSE;
}
int AI_IsAIDisabled()
{
    return bDisableAI;
}


// ----------------------------------------------------------------------------
//
// Name:		IsInsideBox
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool AI_IsInsideBox( userEntity_t *self, CVector point, trace_t& trace )
{
	trace = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, point, self, MASK_SOLID );
	if( trace.fraction >= 1.0f )
		return false;
	else
		return true;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CheckVectorCollision	
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool AI_CheckLineCollision( userEntity_t *self, CVector endPt )
{
//	trace_t		(*TraceLine_q2) ( CVector &start, CVector &end, edict_t *passent, int contentmask);
	trace_t	trace = gstate->TraceLine_q2( self->s.origin, endPt, self, MASK_SOLID );
	if( trace.fraction >= 1.0f )
		return false;
	else
		return true;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CanAttackEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_CanAttackEnemy( userEntity_t *self )
{
	if ( !AI_IsAlive( self->enemy ) || 
	     !AI_IsWithinAttackDistance( self, VectorDistance(self->s.origin, self->enemy->s.origin) ) || 
         !AI_IsVisible( self, self->enemy ) )
	{
		return FALSE;
	}

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetThinkFunc
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetThinkFunc( userEntity_t *self, think_t fnThinkFunc )
{
    self->think	= fnThinkFunc;
}

// ----------------------------------------------------------------------------
// NSS[3/10/00]:
// Name:		AI_SetNextThinkTime
// Description:
// Input:
// Output:
// ----------------------------------------------------------------------------
void AI_SetNextThinkTime( userEntity_t *self, const float fTime )
{
    self->nextthink	= gstate->time + fTime;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsJustFired
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsJustFired( playerHook_t *hook )
{
    return hook->bJustFired;
}

void AI_SetJustFired( playerHook_t *hook )
{
    hook->bJustFired = 1;
}

void AI_DecreaseJustFired( playerHook_t *hook )
{
    if ( hook->bJustFired > 0 )
    {
        hook->bJustFired--;
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StopEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StopEntity( userEntity_t *self )
{
	_ASSERTE( self );
    AI_AddNewTaskAtFront( self, TASKTYPE_STOPENTITY );
    
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsRangeAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsRangeAttack( playerHook_t *hook )
{
    if ( hook->dflags & DFL_RANGEDATTACK )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEntityBelow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsEntityBelow( userEntity_t *self, userEntity_t *pEntity )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

    if ( !AI_IsAlive( pEntity ) )
    {
        return FALSE;
    }

	CVector checkPoint = self->s.origin;
    checkPoint.z -= 24.0f;

    tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, checkPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
	{
		if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
		{
			userEntity_t *pCollidedEnt = tr.ent;

            if ( pCollidedEnt == pEntity )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEntityAbove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsEntityAbove( userEntity_t *self, userEntity_t *pEntity )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

    if ( !AI_IsAlive( pEntity ) )
    {
        return FALSE;
    }

	CVector checkPoint = self->s.origin;
    checkPoint.z += 24.0f;

    tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, checkPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
	{
		if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
		{
			userEntity_t *pCollidedEnt = tr.ent;

            if ( pCollidedEnt == pEntity )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEntityBelow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_IsEntityBelow( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	CVector checkPoint = self->s.origin;
    checkPoint.z -= 4.0f;

    tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, checkPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
	{
		if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
		{
            return tr.ent;
        }
    }

    return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEntityAbove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_IsEntityAbove( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	CVector checkPoint = self->s.origin;
    checkPoint.z += 4.0f;

    tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, checkPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
	{
		if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
		{
            return tr.ent;
        }
    }

    return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindCollisionFreePoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindCollisionFreePoint( userEntity_t *self, CVector &freePoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// search the eight directions around the self to find a spawn point
    float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.15f;

	CVector angle = self->s.angles;
	angle.pitch = -5.0f;

	CVector vector;
	angle.AngleToForwardVector( vector );

	CVector endPoint;
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
        tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
    VectorMA( self->s.origin, vector, fOneFrameSpeed, endPoint );
    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
    {
	    CVector bottomPoint = endPoint;
	    bottomPoint.z -= 1.0f;
	    tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	    {
		    freePoint = endPoint;
		    return TRUE;
	    }
    }

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SubLevelTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SubLevelTransition()
{
	userEntity_t *pEntity = alist_FirstEntity( client_list );
	while ( pEntity != NULL )
	{
        playerHook_t *hook = AI_GetPlayerHook( pEntity );
		if ( AI_IsSidekick( hook ) )
        {
		    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		    _ASSERTE( pGoalStack );
            GOALSTACK_ClearAllGoals( pGoalStack );
        }
        
		pEntity = alist_NextEntity( client_list );
	}


	pEntity = alist_FirstEntity( monster_list );
	while ( pEntity != NULL )
	{
        playerHook_t *hook = AI_GetPlayerHook( pEntity );
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
        GOALSTACK_ClearAllGoals( pGoalStack );

		pEntity = alist_NextEntity( monster_list );
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
