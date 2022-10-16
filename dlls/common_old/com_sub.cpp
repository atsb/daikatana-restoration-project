//#define	HALT_ON_ERRORS
//#include <windows.h>
#include	<stdio.h>
#include	<math.h>
#include	<windows.h>

//	header files in katana\user
#include "p_user.h"
#include "hooks.h"

//	local headers files
#include	"common.h"
#include	"com_globals.h"


//////////////////////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////////////////////

//	globals for vectors that are used all over
Vector			forward, right, up;
vec3_t			zero_vector = {0.0, 0.0, 0.0};
userTrace_t		trace;
serverState_t	*gstate;
common_export_t	com;

static			ID_seed = 0;

////////////////////////////////////////
//	vtos
//
////////////////////////////////////////

static	int		index;
static	char	str [8][32];

char	*com_vtos (vec3_t v)
{
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str [index];
	index = (index + 1)&7;

	sprintf (s, "(%.2f %.2f %.2f)", v [0], v [1], v [2]);

	return s;
}

////////////////////////////////////////
//	vtos1
//
////////////////////////////////////////

char	*com_vtos1 (Vector v)
{
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str [index];
	index = (index + 1)&7;

	sprintf (s, "(%.2f %.2f %.2f)", v.x, v.y, v.z);

	return s;
}
////////////////////////////////////////
//	com_VecToYaw
//
//	returns the yaw equivalent of the passed vector
////////////////////////////////////////

float com_VecToYaw (Vector &vec)
{
	float	yaw;

	if (vec.y == 0 && vec.x == 0)
		yaw = 0;
	else
	{
		yaw = (atan2 (vec.y, vec.x) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}

////////////////////////////////////////
//	com_VecToAngles
//	
//	returns the yaw and pitch equivalent of the passed vector
////////////////////////////////////////

void	com_VecToAngles (Vector &angles, Vector &vec1)
{
/*
	float	yaw, pitch, fw;

	if (vec1.y == 0.0F && vec1.x == 0.0F)
	{
		yaw = 0.0F;
		if (vec1.z > 0.0F)
			pitch = 90.0F;
		else
			pitch = 270.0F;
	}
	else
	{
		yaw = (atan2(vec1.y, vec1.x) * 180.0F / M_PI);
		if (yaw < 0.0F)
			yaw += 360.0F;

		// ask Pythagorus
		fw = sqrt (vec1.x*vec1.x + vec1.y*vec1.y);
		pitch = (atan2(vec1.z, fw) * 180.0F / M_PI);
		if (pitch < 0.0F)
			pitch += 360.0F;
	}

	vec2.x = pitch;
	vec2.y = yaw;
	vec2.z = 0.0F;
*/

	float	forward;
	float	yaw, pitch;
	
	if (vec1.y == 0 && vec1.x == 0)
	{
		yaw = 0;
		if (vec1.z > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec1.y, vec1.x) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec1.x * vec1.x + vec1.y * vec1.y);
		pitch = (int) (atan2 (vec1.z, forward) * 180 / M_PI);
		
		if (pitch < 0)
			pitch += 360;
	}

	angles.x = -pitch;
	angles.y = yaw;
	angles.z = 0;
}

////////////////////////////////////////
//	com_VecToYPR
//
//	returns the pitch, yaw and roll equivalents of the passed vector
//	for use with Bmodels
////////////////////////////////////////

void	com_VecToYPR (Vector &vec2, Vector &vec1)
{
	float	yaw, pitch, forward, roll;

	if (vec1.y == 0 && vec1.x == 0)
	{
		yaw = 0;
		if (vec1.z > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec1.y, vec1.x) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		// ask Pythagorus
		forward = sqrt (vec1.x*vec1.x + vec1.y*vec1.y);
		pitch = (int) (atan2(vec1.z, vec1.y) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;

		forward = sqrt (vec1.x*vec1.x + vec1.y*vec1.y);
		roll = (int) (atan2 (vec1.z, vec1.x) * 180 / M_PI);
		if (roll < 0)
			roll += 360;
	}

	vec2.x = pitch;
	vec2.y = yaw;
	vec2.z = roll;
}

////////////////////////////////////////
//	com_YawDiff
//
//	returns the size of the smallest arc between two yaw angles
////////////////////////////////////////

float	com_YawDiff (float facing_yaw, float to_targ_yaw)
{
	float	yaw_delta;

	// client v_angles [1] (yaw) is always between -180 and +180,
	// so adjust it by adding 360 if it is negative
	// this will give us a value from 0 to 360
	if (facing_yaw < 0) facing_yaw += 360;
	
	if (to_targ_yaw < 0) to_targ_yaw += 360;

	// now determine the shortest direction to turn, left or right, to
	// get to the to_targ_yaw from facing_yaw

	if (facing_yaw < to_targ_yaw)
	{
		yaw_delta = to_targ_yaw - facing_yaw;
		if (yaw_delta > 180)
			yaw_delta = fabs (yaw_delta - 360.0);
	}
	else
	{
		yaw_delta = facing_yaw - to_targ_yaw;
		if (yaw_delta > 180)
			yaw_delta = fabs (yaw_delta - 360.0);
	}

	return	yaw_delta;
}

////////////////////////////////////////
//	com_BestDelta
//
//	find the best delta (positive or negative) to add to 
//	get from start_angle to end_angle by adding the smallest
//	amount
//	the shortest distance between two angles is stored in angle_diff
//	the best direction is stored in best_delta
////////////////////////////////////////

void	com_BestDelta (float start_angle, float end_angle, float *best_delta, float *angle_diff)
{
//	start_angle = com_AngleMod (start_angle);
//	end_angle = com_AngleMod (end_angle);	

	if (start_angle < 0) start_angle += 360;
	if (end_angle < 0) end_angle += 360;

	// now determine the shortest direction to turn, left or right, to
	// get to the end_angle from start_angle

	if (start_angle < end_angle)
	{
		*angle_diff = end_angle - start_angle;
		if (*angle_diff < 180)
			*best_delta = 1.0;
		else
		{
			*best_delta = -1.0;
			*angle_diff = fabs (*angle_diff - 360.0);
		}

	}
	else
	{
		*angle_diff = start_angle - end_angle;
		if (*angle_diff < 180)
			*best_delta = -1.0;
		else
		{
			*best_delta = 1.0;
			*angle_diff = fabs (*angle_diff - 360.0);
		}
	}
}

////////////////////////////////////////
//	com_GetDir
//
//	finds the normalized direction to org1 from org2
//
//	result is stored in dir
//
//	Ummm... maybe this routine is a little redundant...
////////////////////////////////////////

////////////////////////////////////////
//	com_AngleMod
//
//	returns an equivalent angle 0-360
////////////////////////////////////////

float	com_AngleMod (float ang)
{
	return	(360.0 / 65536) * ((int)(ang * (65536 / 360.0)) & 65535);
}

////////////////////////////////////////
//	com_ZeroVector
////////////////////////////////////////

void	com_ZeroVector (vec3_t v)
{
	v [0] = 0.0;
	v [1] = 0.0;
	v [2] = 0.0;
}

////////////////////////////
//	com_MultVector
//
////////////////////////////

void	com_MultVector (vec3_t v, float f1, vec3_t v2)
{
	v2 [0] = v [0] * f1;
	v2 [1] = v [1] * f1;
	v2 [2] = v [2] * f1;
}

////////////////////////////////////////
//	com_SetVector
////////////////////////////////////////

void	com_SetVector (vec3_t v, float f1, float f2, float f3)
{
	v [0] = f1;
	v [1] = f2;
	v [2] = f3;
}

////////////////////////////////////////
//	com_CopyVector
////////////////////////////////////////

void	com_CopyVector (vec3_t dest, const vec3_t src)
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

////////////////////////////////////////
//	com_AngleVectors
////////////////////////////////////////

void com_AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * (M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (right)
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}
	if (up)
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// com_VectorMA
//
///////////////////////////////////////////////////////////////////////////////////////////

void com_VectorMA (vec3_t va, double scale, vec3_t vb, vec3_t vc)
{
	vc[0] = va[0] + scale*vb[0];
	vc[1] = va[1] + scale*vb[1];
	vc[2] = va[2] + scale*vb[2];
}

///////////////////////////////////////////////////////////////////////////////////////////
// com_VectorScale
//
///////////////////////////////////////////////////////////////////////////////////////////

void com_VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

///////////////////////////////////////////////////////////////////////////////////////////
// com_RotateBoundingBox
//
// rotate the bounding box of an entity... only boxes on 90, 180, 270 and 360 are generated
//
///////////////////////////////////////////////////////////////////////////////////////////
void com_RotateBoundingBox (userEntity_t *self)
{

	vec3_t	tmp, v;
	int		i, j;
	vec3_t	forward, right, up, mins, maxs, tmins, tmaxs;

	VectorCopy( self->s.mins, mins );
	VectorCopy( self->s.maxs, maxs );

	for (i=0 ; i<3 ; i++)
	{
		tmins[i] = 99999;
		tmaxs[i] = -99999;
	}

	com_AngleVectors (self->s.angles, forward, right, up);

	for ( i = 0; i < 8; i++ )
	{
		if ( i & 1 )
			tmp[0] = mins[0];
		else
			tmp[0] = maxs[0];

		if ( i & 2 )
			tmp[1] = mins[1];
		else
			tmp[1] = maxs[1];

		if ( i & 4 )
			tmp[2] = mins[2];
		else
			tmp[2] = maxs[2];

		com_VectorScale (forward, tmp[0], v);
		com_VectorMA (v, (double)-tmp[1], right, v);
		com_VectorMA (v, (double)tmp[2], up, v);

		for (j=0 ; j<3 ; j++)
		{
			if (v[j] < tmins[j])
				tmins[j] = v[j];
			if (v[j] > tmaxs[j])
				tmaxs[j] = v[j];
		}
	}

	// update the bounding box points
	VectorCopy( tmins, self->s.mins);
	VectorCopy( tmaxs, self->s.maxs);

}


////////////////////////////////////////
//	com_Visible
//
//	returns distance to ent if self can see ent, or FALSE if not
//
//	only checks from origin to origin, does not take into account the
//	size of an entity by checking its extents.
////////////////////////////////////////

int		com_Visible (userEntity_t *self, userEntity_t *ent)
{
	Vector			org, end;
//	userTrace_t		trace;

	// get center of bmodel as origin
	if (self->movetype == MOVETYPE_PUSH || self->solid == SOLID_BSP || self->solid == SOLID_TRIGGER)
		org = (Vector (self->absmin) + Vector (self->absmax)) * 0.5;
	else 
		org = Vector (self->s.origin);

	if (ent->movetype == MOVETYPE_PUSH || ent->solid == SOLID_BSP || ent->solid == SOLID_TRIGGER)
		end = (Vector (ent->absmin) + Vector (ent->absmax)) * 0.5;
	else 
		end = Vector (ent->s.origin);

	// make sure we're checking from eye to eye
	org = org + Vector (self->view_ofs);
	end = end + Vector (self->view_ofs);
	gstate->TraceLine (org.vec (), end.vec (), TRUE, self, &trace);

	if (trace.fraction == 1.0 || trace.ent == ent && !trace.startsolid && !trace.allsolid)
	{
		org = org - Vector (trace.endpos);
		return	(int) org.vlen ();
	}
	else
		return	FALSE;
}

//////////////////////
//	com_ChangeYaw
//
//////////////////////

void	com_ChangeYaw (userEntity_t *ent)
{
	float		ideal, current, move, speed;

	current = com_AngleMod (ent->s.angles [YAW]);
	ideal = com_AngleMod (ent->ideal_ang [YAW]);
	speed = ent->ang_speed [YAW];
	
	if (current == ideal)
		return;

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
			move = speed;
		else
		{
			ent->s.angles [YAW] = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
			move = -speed;
		else
		{
			ent->s.angles [YAW] = ideal;
			return;
		}
	}
	
	ent->s.angles [YAW] = com_AngleMod (current + move);
}

//////////////////////
//	com_ChangePitch
//
//////////////////////

void	com_ChangePitch (userEntity_t *ent)
{
	float		ideal, current, move, speed;
	Vector		temp;

	if (!ent->goalentity)
		return;

	current = com_AngleMod (ent->s.angles [PITCH]);
	ideal = com_AngleMod (ent->ideal_ang [PITCH]);
	speed = ent->ang_speed [PITCH];
	
	if (current == ideal)
		return;

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
			move = speed;
		else
		{
			ent->s.angles [PITCH] = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
			move = -speed;
		else
		{
			ent->s.angles [PITCH] = ideal;
			return;
		}
	}
	
	ent->s.angles [PITCH] = com_AngleMod (current + move);
}

//////////////////////
//	com_ChangeRoll
//
//////////////////////

void	com_ChangeRoll(userEntity_t *ent)
{
	float		ideal, current, move, speed;
	Vector		temp;

	if (!ent->goalentity)
		return;

	current = com_AngleMod (ent->s.angles [ROLL]);
	ideal = com_AngleMod (ent->ideal_ang [ROLL]);
	speed = ent->ang_speed [ROLL];
	
	if (current == ideal)
		return;

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
			move = speed;
		else
		{
			ent->s.angles [ROLL] = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
			move = -speed;
		else
		{
			ent->s.angles [ROLL] = ideal;
			return;
		}
	}
	
	ent->s.angles [ROLL] = com_AngleMod (current + move);
}

////////////////////////////////////////
//	com_SetMovedir
//
//	QEd only writes a single float for angles (bad idea), so up and down are
//	just constant angles.
////////////////////////////////////////

void	com_SetMovedir (userEntity_t *self)
{
	Vector	fw, up, right, temp;
	Vector	angles (self->s.angles), movedir (self->movedir);

	if (!self->className)
	{
		gstate->Con_Printf ("SetMovedir: no className\n");
		return;
	}
	if (!stricmp (self->className, "func_plat"))
	{
		if (angles.y == -1.0)
		{
			movedir = Vector (0.0, 0.0, 1.0);
		}
		else if (angles.y == -2.0)
		{
			movedir = Vector (0.0, 0.0, -1.0);
		}
		else
		{
			//	plats default to move up
			movedir = Vector (0.0, 0.0, -1.0);
		}
	}
	else
	{
		if (angles == Vector (0.0, -1.0, 0.0))
			movedir = Vector (0.0, 0.0, 1.0);
		else if (angles == Vector (0.0, -2.0, 0.0))
			movedir = Vector (0.0, 0.0, -1.0);
		else
		{
			movedir = Vector (0.0, 0.0, -1.0);
			angles.AngleVectors (movedir, up, right);
		}
	}

	angles = Vector (0.0, 0.0, 0.0);
	angles.set (self->s.angles);

	movedir.Normalize ();
	movedir.set (self->movedir);
};

////////////////////////////////////////
//	com_FindEntity
//
//	Returns the entity with matching classname 
////////////////////////////////////////

userEntity_t *com_FindEntity (const char *name)
{
	userEntity_t	*head;
	
	head = gstate->FirstEntity ();

	while (head)
	{
		if (head->className)
		{
			if (!stricmp(head->className, name))
			{
				return(head);
			}
		}

		head = gstate->NextEntity (head);
	}

	return NULL;
}

////////////////////////////////////////
//	com_FindClosestEntity
//
//	Returns the closest entity with matching classname 
////////////////////////////////////////

userEntity_t *com_FindClosestEntity (userEntity_t *self, const char *name)
{
	userEntity_t	*head, *closest;
	float			dist, last_dist;
	
	#define AWHOLEHELLOFALOT	1000000.0;
	
	head = gstate->FirstEntity ();
	last_dist = AWHOLEHELLOFALOT;
	closest = NULL;

	while (head)
	{
		if (head->className)
		{
			if (!stricmp(head->className, name))
			{
				dist = qv_dist (self->s.origin, head->s.origin);
				if (dist < last_dist && com_Visible (self, head))
				{
					closest = head;
					last_dist = dist;
				}
			}
		}

		head = gstate->NextEntity (head);
	}

	return closest;
}

////////////////////////////////////////
//	com_FindTarget
//
//	Returns the first entity with a matching targetname 
////////////////////////////////////////

userEntity_t *com_FindTarget (const char *name)
{
	userEntity_t	*head;
	
	head = gstate->FirstEntity ();

	while (head)
	{
		if (head->targetname)
		{
//			gstate->bprint ("targetname = %s\n", head->targetname);
			if (!stricmp(head->targetname, name))
				return(head);
		}

		head = gstate->NextEntity (head);
	}

	return NULL;
}

//////////////////////////////////////////////////////////
//	com_FindFirstGroupMember
//
//	Returns the first entity with a matching group name 
//////////////////////////////////////////////////////////

userEntity_t *com_FindFirstGroupMember (char *groupName, char *className = NULL)
{
	userEntity_t	*head;

	if (!groupName)
	{
		return (NULL);
	}
	
	head = gstate->FirstEntity ();

	while (head)
	{
		if (head->groupname)
		{
			// check the group name
			if (!stricmp(head->groupname, groupName))
			{
				// check the class name if defined
				if (className)
				{
					if (!stricmp(head->className, className))
						return(head);
				}
				else
				{
					return (head);
				}
			}
		}

		head = gstate->NextEntity (head);
	}

	return NULL;
}

//////////////////////////////////////////////////////////
//	com_FindNextGroupMember
//
//	Returns the next entity with a matching group name 
//////////////////////////////////////////////////////////

userEntity_t *com_FindNextGroupMember (char *groupName, userEntity_t *head, char *className = NULL)
{
	if (!groupName)
	{
		return (NULL);
	}

	// skip to the next one
	head = gstate->NextEntity (head);

	while (head)
	{
		if (head->groupname)
		{
			if (!stricmp(head->groupname, groupName))
			{
				// check the class name if defined
				if (className)
				{
					if (!stricmp(head->className, className))
						return(head);
				}
				else
				{
					return (head);
				}
			}	
		}

		head = gstate->NextEntity (head);
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
//	com_TransformChildOffset
//
//	Fills in 'out' with the transformed version of offs given 'origin' and 'angles' 
/////////////////////////////////////////////////////////////////////////////////////

void com_TransformChildOffset( vec3_t origin, vec3_t offs, vec3_t angles, bool bFlipYaw, vec3_t out )
{
	vec3_t forward, right, up, offset;

	// calculate the transformation matrix
	com_AngleVectors (angles, forward, right, up);

	// transform the offset from the parent
	VectorCopy (offs, offset);
	
	// negate Y
	if (bFlipYaw)
	{
		offset[1] = -offset[1];
	}
	// transform the coordinates
	out[0] = DotProduct (offset, forward) + origin[0];
	out[1] = DotProduct (offset, right) + origin[1];
	out[2] = DotProduct (offset, up) + origin[2];

	return;
}


////////////////////////////////////////
//	com_RemoveMarker
//	Spawns a temporary marker entity
////////////////////////////////////////

void	com_RemoveMarker (userEntity_t *self)
{
	gstate->RemoveEntity (self);
}

////////////////////////////////////////
//	com_SpawnMarker
//	Spawns a temporary marker entity
////////////////////////////////////////

void	com_SpawnMarker (userEntity_t *self, Vector &org, char *model_name, float time)
{
	userEntity_t	*marker;
/*
	if (self->className)
		if (stricmp (self->className, "player") != 0)
			return;
*/
	marker = gstate->SpawnEntity ();

	gstate->SetSize (marker, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin (marker, org.x, org.y, org.z);
	gstate->SetModel (marker, model_name);

	marker->solid = SOLID_NOT;
	marker->movetype = MOVETYPE_NONE;

	marker->nextthink = gstate->time + time;
	marker->think = com_RemoveMarker;

//	com_CopyVector (marker->velocity, forward);
//	com_MultVector (marker->velocity, 320);
}

///////////////////////////////////////////////////////////////////////////////
//	com_SpawnBlood
//
///////////////////////////////////////////////////////////////////////////////

void com_SpawnBlood (Vector origin, Vector normal, int damage, int type)
{
	if (damage > 255)
		damage = 255;

	gstate->WriteByte (SVC_TEMP_ENTITY);

	gstate->WriteByte (type);

	//	Fixme:	base blood on amount of damage
	//	gi.WriteByte (damage);

	gstate->WritePosition (origin.vec ());
	gstate->WriteDir (normal.vec ());
	gstate->MultiCast (origin.vec (), MULTICAST_PVS);
}

///////////////////////////////////////////////////////////////////////////////
//	com_ArmorDamage
//
//	computes the amount of damage done to armor during an attack
//	and adjusts armor value accordingly
///////////////////////////////////////////////////////////////////////////////

float	com_ArmorDamage (userEntity_t *self, float damage)
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			take, absorb, startval;

	startval = self->armor_val;

	// amount of damage armor takes
	absorb = damage * self->armor_abs;
	// amount of damage player takes
	take = damage - absorb;

	//gstate->Con_Printf ("absorb = %f, take = %f\n", absorb, take);
	// reduce armor value by amount of damage
	self->armor_val = self->armor_val - absorb;
	if (self->armor_val <= 0)
	{
		self->armor_val = 0;
		// get rid of our armor bit
      if (hook)
		   hook->items = hook->items - (hook->items & (IT_PLASTEEL | IT_CHROMATIC | IT_SILVER | 
					  IT_GOLD | IT_CHAINMAIL | IT_BLACK_ADAMANT | IT_KEVLAR | IT_EBONITE));
	}

	if (startval != self->armor_val)
	{
		// send new armor value down to client for display
		//	Q2FIXME:	Change to Q2 network functions
		/*
		gstate->WriteByte (self, MSG_ONE, SVC_ARMORVAL);
		gstate->WriteByte (self, MSG_ONE, (int)hook->armor_val);
		*/
	}

	return take;
}

///////////////////////////////////////////////////////////////////////////////
//	com_Damage
//
//	does damage to an entity
//
//	damage_type is the type of damage, used for creating damage effects
//	such as adding velocity to an entity from an explosion when an entity
//	is killed.
//
//	currently supported types:
//	explosion	- adds velocity to target along a vector from inflictor
//				to target.  Velocity magnitude is based on amount of 
//				damage 
//	inertial	- falling or smacking into something at hight speed
//				applies velocity along a vector from attack to target
//	squish		- getting squished by a door or rotating brush
//				applies a percentage of the squishers velocity to the target
//			
//	none or ""	- only reduces the health of the target
///////////////////////////////////////////////////////////////////////////////

void	com_Damage (userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, vec3_t point_of_impact, 
					vec3_t damage_vec, float damage, unsigned long damage_flags)
{
	Vector			temp, velocity, dir;
	float			mod_damage;
	playerHook_t	*hook, *ahook;
	int				pain_chance;

	gstate->attacker = attacker;
	gstate->inflictor = inflictor;
	gstate->damage_flags = damage_flags;

	////////////////////////////////////////////////////////////////////////////////
	//	clients, monsters and bots can have * 3 damage if rampage flag is set
	////////////////////////////////////////////////////////////////////////////////

	if (attacker->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		ahook = (playerHook_t *) attacker->userHook;

		if (ahook->dflags & DFL_RAMPAGE)
			damage = damage * 3;
	}

	////////////////////////////////////////////////////////////////////////////////
	//	if a client, bot or monster, compute damage done based on target's armor
	////////////////////////////////////////////////////////////////////////////////
	
	if (target->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		hook = (playerHook_t *) target->userHook;

		mod_damage = com_ArmorDamage (target, damage);
		gstate->damage_inflicted = mod_damage;

		if (hook->megashield_time < gstate->time)
		{
			if (!(hook->dflags & DFL_LLAMA))
				target->health = target->health - mod_damage;
		}
	}
	else 
	{
		gstate->damage_inflicted = damage;
		target->health = target->health - damage;	
	}

	////////////////////////////////////////////////////////////////////////////////
	//	if target is not pushable, then do DAMAGE_NONE so target can't possibly move
	////////////////////////////////////////////////////////////////////////////////
	
	if (!(target->flags & FL_PUSHABLE) || (damage_flags & DAMAGE_NO_VELOCITY))
	{
		damage_flags &= DAMAGE_ONLY_FLAGS;
		damage_flags |= DAMAGE_NONE;
	}

	if (damage_flags & DAMAGE_DIR_TO_ATTACKER)
		//	compute damage vector based on direction from attacker to target
		dir = Vector (target->s.origin) - (Vector (attacker->absmin) + Vector (attacker->absmax)) * 0.5;
	else if (damage_flags & DAMAGE_DIR_TO_INFLICTOR)
		//	compute damage vector based on direction from attacker to target
		dir = Vector (target->s.origin) - (Vector (inflictor->absmin) + Vector (inflictor->absmax)) * 0.5;
	else if (damage_flags & DAMAGE_INFLICTOR_VEL)
		dir = Vector (inflictor->velocity);
	else
		dir = Vector (damage_vec);

	//	normalize just in case...
	dir.Normalize ();
	//	set global damage vector
	dir.set (gstate->damage_vector);

	////////////////////////////////////////////////////////////////////////////////
	//	affect target according to type of damage
	////////////////////////////////////////////////////////////////////////////////
	if (damage_flags)
	{
		if (damage_flags & (DAMAGE_EXPLOSION | DAMAGE_SIDEWINDER))
		{
			if (target->health <= 0.0)
			{
				dir = dir *  10 * (rnd () * 80);
				target->velocity [0] = dir.x;
				target->velocity [1] = dir.y;
				target->velocity [2] = 100 + (rnd () * 500);
				target->groundEntity = NULL;
				gstate->SetOrigin (target, target->s.origin [0], target->s.origin [1], target->s.origin [2]);

				target->avelocity [1] = rnd () * 1200 - 600;
			}
			else
			{
				if (damage_flags & DAMAGE_SIDEWINDER && target == attacker && target->flags & (FL_CLIENT + FL_MONSTER + FL_BOT))
				{
					//	Nelno:	FIXME... not working yet
					gstate->Con_Dprintf ("adjusting for off-center launch.\n");
					//	adjust for off-center launching weapons
					temp = Vector (attacker->s.angles);
					temp.AngleVectors (forward, up, right);

					dir.set (gstate->damage_vector);
					dir = dir * damage * 8.0 + (right * damage);
				}
				else
				{
					dir.set (gstate->damage_vector);
					dir = dir * damage * 8.0;
				}
	

				velocity = Vector(target->velocity) + dir;
				velocity.set(target->velocity);

				//////////////////////////////////////////////////
				//	rocket jump flagging
				//////////////////////////////////////////////////
				if (attacker == target && target->flags & FL_CLIENT)
				{
					//	Nelno:	FIXME not working yet
					if (hook->jump_time > gstate->time)
						gstate->Con_Printf ("rocket jump flagged\n");
				}
			}

			target->groundEntity = NULL;
		}
		else if (damage_flags & DAMAGE_SQUISH)
		{
			//	special case... overrides DAMAGE_DIR_TO and DAMAGE_INFLICTOR_VEL flags...
			//	always gets dir from squisher's velocity vector
			temp = Vector (target->velocity) + Vector (inflictor->velocity);

			if (temp.z < 0)
			{
				temp.z = -temp.z;
				if (temp.z > 200)
					temp.z = 200;
			}
			temp.set (target->velocity);

			temp.Normalize ();
			temp.set (gstate->damage_vector);

			//	FIXME:	clear target->groundEntity?
			target->groundEntity = NULL;
			gstate->SetOrigin (target, target->s.origin [0], target->s.origin [1], target->s.origin [2]);

			target->groundEntity = NULL;
		}
		else if (damage_flags & DAMAGE_INERTIAL)
		{
			dir = dir * damage * 6;
			target->velocity [0] += dir.x;
			target->velocity [1] += dir.y;
			target->velocity [2] += dir.z + damage * 2;

			//	FIXME:	clear target->groundEntity?
		}
		else
		{
			//	Default damage type
			temp = Vector (rnd (), rnd (), rnd ());
			temp.Normalize ();
			temp.set (gstate->damage_vector);

			target->groundEntity = NULL;
		}
	}

	if (damage_flags & DAMAGE_ONLY_TYPES)
	{
		temp = Vector (target->avelocity) * (1.0 / target->mass);
		temp.set (target->avelocity);
		temp = Vector (target->velocity) * (1.0 / target->mass);
		temp.set (target->velocity);

		target->groundEntity = NULL;
	}

	//	FIXME:	project damage vector into model and find impact tri
	//	FIXME:	tie blood to actual damaged tri
	//	FIXME:	spawn correct blood type based on entity type
	if (!(damage_flags & DAMAGE_NO_BLOOD) && (target->flags & (FL_CLIENT | FL_BOT | FL_MONSTER)))
		com_SpawnBlood (point_of_impact, dir, damage, TE_BLOOD);
	else
		com_SpawnBlood (point_of_impact, dir, damage, TE_SPARKS);

	///////////////////////////////////////////////////////////////
	//	target killed
	///////////////////////////////////////////////////////////////

	if ((target->health <= 0.0) && (target->deadflag != DEAD_DEAD))  //&&& AMW 7.17.98 - added the dead flag
	{
		if (target->die)
		{
			if (target->flags & FL_CLIENT && target != attacker)
				//	make sure we'll follow killer
				target->enemy = attacker;
			else
				target->enemy = NULL;

			target->die (target, inflictor, attacker, damage, target->s.origin);
		}
	}

	///////////////////////////////////////////////////////////////
	//	target pain
	///////////////////////////////////////////////////////////////

	//	FIXME:	will < 2 cause some killtargets not to fire sometimes?  (secret doors, etc.)?  Does Nelno care?
	else if (target->pain)
	{
		if (damage > 2)
			target->pain (target, attacker, 0, damage);
		else if (target->flags & (FL_CLIENT | FL_BOT | FL_MONSTER) && hook->items & IT_POISON)
		{
			//	force pain to happen 25% of the time if poisoned
			pain_chance = hook->pain_chance;
			hook->pain_chance = 25;
			target->pain (target, attacker, 0, damage);
			hook->pain_chance = pain_chance;
		}
	}
}

////////////////////////////////////////
//	com_CanDamage
//
//	determines if the target can be damaged by tracing lines from
//	the inflictor to the target.  If any line reaches the target
//	then TRUE is returned
////////////////////////////////////////

int		com_CanDamage (userEntity_t *target, userEntity_t *inflictor)
{
	Vector			org, end, targ_org (target->s.origin);
	int				is_bmodel = FALSE;
	int				temp = FALSE;
	trace_t			tr;

	// bmodels suck because their origin == 0, 0, 0

	if (target->className)
	{
		if (strstr (target->className, "trigger_"))
			is_bmodel = TRUE;
	}
	if (target->movetype == MOVETYPE_PUSH)
		is_bmodel = TRUE;

	if (is_bmodel)
	{
		end = (Vector (target->absmin) + Vector (target->absmax)) * 0.5;
		
		tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, end.vec (), inflictor, MASK_SHOT);

		if (tr.fraction == 1 || tr.ent == target)
			temp = TRUE;
	}
	else
	{
		tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, target->s.origin, inflictor, MASK_SHOT);

		if (tr.fraction == 1.0 || tr.ent == target)
			temp = TRUE;

		//	Multiple traces... bad
/*
		end = Vector (15.0, 15.0, 0.0) + targ_org;
		tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, end.vec (), inflictor, MASK_SHOT);
		if (tr.fraction == 1.0)
			temp = TRUE;
		else
		{
			end = Vector (-15.0, -15.0, 0.0) + targ_org;
			tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, end.vec (), inflictor, MASK_SHOT);
			if (tr.fraction == 1.0)
				temp = TRUE;
			else
			{
				end = Vector (15.0, -15.0, 0.0) + targ_org;
				tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, end.vec (), inflictor, MASK_SHOT);
				if (tr.fraction == 1.0)
					temp = TRUE;
				else
				{
					end = Vector (-15.0, 15.0, 0.0) + targ_org;
					tr = gstate->trace_q2 (inflictor->s.origin, NULL, NULL, end.vec (), inflictor, MASK_SHOT);
					if (tr.fraction == 1.0)
						temp = TRUE;
				}
			}
		}
*/
	}

	return temp;
}

////////////////////////////////////////
//	com_RadiusDamage
////////////////////////////////////////

int	com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, unsigned long damage_flags)
{
	userEntity_t	*head;
	float			damage_points;
	Vector			org;
	int				hit_something;

	hit_something = FALSE;
	head = gstate->FirstEntity ();

	damage_flags &= ~(DAMAGE_DIR_TO_ATTACKER | DAMAGE_INFLICTOR_VEL);
	damage_flags |= DAMAGE_DIR_TO_INFLICTOR;

	while (head)
	{
		// find the distance to this entity
		if (head != ignore && head->takedamage)
		{
			if (head->solid == SOLID_BSP)
				org = Vector (head->s.origin) + (Vector (head->s.mins) + Vector (head->s.maxs)) * 0.5;
			else
				org = Vector (head->s.origin);

			damage_points = org.vdist (Vector (inflictor->s.origin)) * 0.5;
         if (damage_flags & DAMAGE_RADIUS2)
            damage_points /= 2;
         if (damage_flags & DAMAGE_RADIUS4)
            damage_points /= 4;
         if (damage_flags & DAMAGE_RADIUS8)
            damage_points /= 8;

			if (damage_points < 0)
				damage_points = 0;

			damage_points = damage - damage_points;

			// half damage from own weapon
			if (head == attacker)
				damage_points = damage_points / 2;

			if (damage_points > 0 && com_CanDamage (head, inflictor))
			{
				hit_something = TRUE;
				//	pass it zero_vector as dir because DAMAGE_DIR_TO_INFLICTOR flag will override dir anyway
				com_Damage (head, inflictor, attacker, org.vec (), zero_vector, damage_points, damage_flags);
			}
		}

		head = gstate->NextEntity (head);
	}

	return	hit_something;
}

////////////////////////////////////////
//	com_GetMinsMaxs
//
//	get the min size and max size of an entity
//	Ummm... it's difficult to explain this, but only alias
//	models have an origin that describes their actual location
//	on the map, so this procedure finds that origin by locating
//	their center based on self->absmin and self->absmax which
//	are basically the entities "corners".  Then the origin is
//	subtracted from absmax to get the max size and absmin is 
//	subtracted from the origin to get the min size.
////////////////////////////////////////

void com_GetMinsMaxs (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs)
{
	// get local origin
	org = (Vector (self->absmax) - Vector (self->absmin)) * 0.5 + Vector (self->absmin);

	// get maxs
	maxs = Vector (self->absmax) - org;
	mins = Vector (self->absmin) - org;
}


////////////////////////////////////////
//	com_ValidTouch
//
//	determines if the object was touched by an entity that can
//	activate it.
////////////////////////////////////////

int	com_ValidTouch (userEntity_t *self, userEntity_t *other) 
{
	if (other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_POSSESSED)
		return TRUE;
	else
		return FALSE;
}

//	prototype
void	com_UseTargets (userEntity_t *self, userEntity_t *other, userEntity_t *activator);

////////////////////////////////////////
//	DelayedUseTarget
////////////////////////////////////////

void	DelayedUseTarget (userEntity_t *self)
{
	float		temp;

	gstate->activator = self->owner;

	temp = self->delay;
	self->delay = 0;

	com_UseTargets (self, self->enemy, self->owner);

	self->delay = temp;

	self->think = NULL;
	self->nextthink = -1;
	
	gstate->RemoveEntity (self);
}

/////////////////////////////////////////////////////////////////////
//	DelayedUseTarget2
//
//	this version insures the target was triggered before deleting it
//
/////////////////////////////////////////////////////////////////////

void	DelayedUseTarget2 (userEntity_t *self)
{
	float		temp, nextthink;

	gstate->activator = self->owner;

	temp = self->delay;
	self->delay = 0;
	
	// save this entities nextthink time.. if it's incremented in the 'use' function
	// then that means it wasn't "used" yet and we shouldn't delete it
	nextthink = self->nextthink;

	com_UseTargets (self, self->enemy, self->owner);

	self->delay = temp;

	// it was not modified to delay additional time
	if (self->nextthink == nextthink)
	{
		self->think = NULL;
		self->nextthink = -1;
	
		gstate->RemoveEntity (self);
	}
}


////////////////////////////////////////
//	DelayedUse
////////////////////////////////////////


void	DelayedUse (userEntity_t *self)
{
	//	Q2FIXME: self->enemy always == self...
	//self->enemy->use (self->enemy, self->groundEntity, self->owner);
	self->enemy->use (self->enemy, self->groundEntity, self->owner);

	self->think = NULL;
	self->nextthink = -1;

	gstate->RemoveEntity (self);
}

////////////////////////////////////////
//	DelayedKill
////////////////////////////////////////

void	DelayedKill (userEntity_t *self)
{
	gstate->activator = self->owner;

	//	remove activated object
	gstate->RemoveEntity (self->enemy);
	//	remove delay object
	gstate->RemoveEntity (self);
}

////////////////////////////////////////
//	SpawnDelay
////////////////////////////////////////

void	SpawnDelay (userEntity_t *self, userEntity_t *other, userEntity_t *activator, think_t think)
{
	userEntity_t	*temp;

	temp = gstate->SpawnEntity ();
	temp->className = "DelayedUse";
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->owner = activator;
	temp->enemy = self;
	temp->groundEntity = other;

	temp->nextthink = gstate->time + self->delay;

	temp->think = think;
	temp->message = self->message;
	temp->target = self->target;
	temp->killtarget = self->killtarget;
}

////////////////////////////////////////
//	Com_CheckTargetForActiveChildren
////////////////////////////////////////

bool	com_CheckTargetForActiveChildren (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{

	//&&& AMW 6.15.98 - don't allow the use of a parent train if any of it's child entities are moving, etc
	if (self->teamchain && !(self->flags & FL_TEAMSLAVE))
	{
		userEntity_t *cur = self->teamchain;
		bool bChildIsActive = FALSE;

		while (cur)
		{
			// if this child has a thinktime, most likely it's still doing something.. right ???
			if ((cur->nextthink > 0) && cur->think)
			{
				bChildIsActive = TRUE;
				if (!stricmp(cur->className, "func_dynalight"))
					bChildIsActive = FALSE;
			}

			// go to the next child in the list
			cur = cur->teamchain;
		}
		
		// did we find a child that is doing something?
		if (bChildIsActive)
		{
			if (!stricmp (gstate->targeter->className, "DelayedUse"))
			{
				gstate->targeter->nextthink = gstate->time + 0.2;				
			}
			else
			{
				// save this entities original delay						 
				float tempDelay = self->delay;								 //
																			 //	
				// let's try again after a delay							 //	
				self->delay = 1.0;											 //	
				// spawn a DelayedUseTarget entity							 //	
//				SpawnDelay (self, other, activator, DelayedUseTarget);		 //	
				SpawnDelay (gstate->targeter, other, activator, DelayedUseTarget2);		 //	
				// reset the delay
				self->delay = tempDelay;
			}

			return (TRUE);
		}
	}
	// there are no active children
	return (FALSE);
}


////////////////////////////////////////
//	com_UseTargets
//
//	the global "activator" should be set to the entity that initiated the firing.
//
//	If self.delay is set, a DelayedUse entity will be created that will actually
//	do the com_UseTargets after that many seconds have passed.
//
//	Centerprints any self.message to the activator.
//
//	Removes all entities with a targetname that match self.killtarget,
//	and removes them, so some events can remove other triggers.
//
//	Search for (string)targetname in all entities that
//	match (string)self.target and call their .use function
////////////////////////////////////////

void	com_UseTargets (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	userEntity_t	*head, *temp;

	if (self->delay)
	{
		// if its a child entity, the delays are handled a little differently, use a different think function
		SpawnDelay (self, other, activator, (self->teamchain && self->teammaster) ? DelayedUseTarget2 : DelayedUseTarget);
		return;
	}

/*
	//FIXME: do we need a beep for message display?
	if (gstate->activator->className)
	{
		if (activator->className == "player" && self->message != "")
		{
			gstate->Con_Printf (self->message);
			// FIXME: play sound
		}
	}
*/
	// kill the killtargets
	if (self->killtarget)
	{
		head = gstate->FirstEntity ();

		do
		{
			temp = NULL;

			if (head != self && head->targetname)
			{
				if (!stricmp (head->targetname, self->killtarget))
				{
					//	func_wall_explode and func_wall "die" when killtargetted
					if (!stricmp (head->className, "func_wall_explode") || !stricmp (head->className, "func_wall"))
					{
						if (head->delay)
							SpawnDelay (head, other, activator, DelayedUse);
						else
							head->use (head, other, activator);
					}
					else
					{
						if (head->delay)
							SpawnDelay (head, other, activator, DelayedKill);
						else
						{
							temp = gstate->NextEntity (head);
							gstate->RemoveEntity (head);
							head = temp;
						}
					}
				}
			}
			
			
			if (!temp) head = gstate->NextEntity (head);
		} while (head);
	}

	// fire targets, as long as it's not a path_corner_train
	// path_corner_train's targets only point to the next path_corner, so don't use them

	if (self->target && !(self->flags & FL_IGNOREUSETARGET))
	{
		head = gstate->FirstEntity ();

		do
		{
			if (head != self && head->targetname)
			{
				//	uncomment for testing
//				if (strstr (head->targetname, self->target))
				if (!stricmp (head->targetname, self->target))
				{	
					if (head->use)
					{
						//	doors and platforms implement their own delays
						//	this is so linked doors can be fired by their master doors and still
						//	have a delay
						if (head->delay && !strstr (head->className, "door") && !strstr (head->className, "plat"))
							SpawnDelay (head, other, activator, DelayedUse);
						else
						{
							gstate->targeter = self;
							head->use (head, other, activator);
						}
					}
				}
			}

			head = gstate->NextEntity (head);
		} while (head);
	}
}

////////////////////////////////////////
//	com_Health
//
//	gives health to an entity, capping it at max_health
////////////////////////////////////////

void	com_Health (userEntity_t *self, float give, float max_health)
{
	self->health = self->health + give;
	
	gstate->Con_Printf ("health = %f max_health = %f\n", self->health, max_health);
	if (self->health > max_health)
		self->health = max_health;
}

////////////////////////////////////////
//	com_AmbientVolume
////////////////////////////////////////

void	com_AmbientVolume (userEntity_t *self, Vector &org, int volume)
{
	//	Q2FIXME:	change to Quake 2 network functions
	/*
	gstate->WriteByte (self, MSG_ALL, SVC_AMBIENTVOLUME);
	gstate->WriteCoord (self, MSG_ALL, org.x);
	gstate->WriteCoord (self, MSG_ALL, org.y);
	gstate->WriteCoord (self, MSG_ALL, org.z);
	gstate->WriteByte (self, MSG_ALL, volume);
	*/
}

////////////////////////////////////////
//	com_Respawn
////////////////////////////////////////

void	com_Respawn (userEntity_t *self)
{
	if (gstate->deathmatch || !gstate->deathmatch)//FIXME: no respawn unless deathmatch
	{
		gstate->SetModel (self, self->weaponmodel);
	
		self->solid = SOLID_TRIGGER;
		self->nextthink = -1;

		gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);

		// aiment is used to hold the ambient volume between respawns
		if (self->aiment)
			com_AmbientVolume (self, Vector (self->s.origin), self->aiment);
	}
}

////////////////////////////////////////
//	com_ClientPrint
//
//	sprints if self is a client
////////////////////////////////////////

void	com_ClientPrint (userEntity_t *self, char *msg)
{
	if (self->flags & FL_CLIENT)
		gstate->sprint (self, msg);
}

////////////////////////////////////////
//	com_SetRespawn
////////////////////////////////////////

void	com_SetRespawn (userEntity_t *self, float time, int reset_ambient, int ambient_vol)
{
	// save the model name for respawning
	self->weaponmodel = self->modelName;
	self->modelName = NULL;
	self->solid = SOLID_NOT;

	if (gstate->deathmatch)
	{
		if (reset_ambient)
			self->aiment = ambient_vol;
		else
			self->aiment = 0.0;
		
		self->nextthink = gstate->time + time;
		self->think = com_Respawn;
	}
	else
		gstate->RemoveEntity (self);
}

/////////////////////////////////////////////////////////////////////////////
//  spawn point selection code
//
//  find every spawn point, then find the closest player/bot entity to each
//  also find the number of players/bots visible from each.  Spawn player
//  at point with fewest and furthest entites.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//	com_SelectSpawnPoint
//
//	find the next player spawn spot
////////////////////////////////////////

typedef	struct spawn_s
{
	userEntity_t	*spot;
	int				ents_visible;
	float			closest;
} spawn_t;

void	spawn_findinfo (const char *className, spawn_t *spawn_point)
{
	userEntity_t	*ent;
	float			dist;

	spawn_point->ents_visible = 0;
	spawn_point->closest = 8192.0;

	ent = gstate->FirstEntity ();

	while (ent)
	{
		if (ent->flags & (FL_CLIENT | FL_BOT | FL_MONSTER) && ent->deadflag == DEAD_NO)
		{
			dist = qv_dist (spawn_point->spot->s.origin, ent->s.origin);

			// if this is the closest ent, save distance
			if (dist < spawn_point->closest)
				spawn_point->closest = dist;

			// if this ent is visible, inc counter
			if (com_Visible (spawn_point->spot, ent))
				spawn_point->ents_visible++;
		}

		ent = gstate->NextEntity (ent);
	}
}

/////////////////////////////////
//  com_DeathmatchSpawnPoint
//
//  returns the best spawn point
/////////////////////////////////

userEntity_t *com_DeathmatchSpawnPoint (const char *className)
{
	int				count, i, j;
	spawn_s			spawn_point [16];
	spawn_s			temp;
	userEntity_t	*ent;

	if (!className)
	{
		gstate->Con_Printf ("com_SelectSpawnPoint: NULL className.\n");
		return	NULL;
	}

	// go through entities and get all spawn points
	for (ent = gstate->FirstEntity (), count = 0; ent; ent = gstate->NextEntity (ent))
	{
		if (ent->className)
		{
			if (!stricmp (ent->className, className))
			{
				spawn_point [count].spot = ent;
				count++;
			}
		}
	}

	// exit if no spawn points found
	if (count == 0)
	{
		gstate->Con_Printf ("No %s found in entity list.\n", className);
		gstate->Error ("Check for deathmatch spawnpoints.");

		return	NULL;
	}

	// find number of visible ents and closest ent for each spawn point
	for (i = 0; i < count; i++)
		spawn_findinfo (className, &spawn_point [i]);

	// sort spawn points by closest ent
	for (i = 1; i < count; i++)
	{
		temp = spawn_point [i];
		j = i;

		while (j > 0 && spawn_point [j - 1].closest < temp.closest)
		{
			spawn_point [j] = spawn_point [j - 1];
			j--;
		}
		spawn_point [j] = temp;
	}

	// sort spawn points by number of entities visible
	for (i = 1; i < count; i++)
	{
		temp = spawn_point [i];
		j = i;

		while (j > 0 && spawn_point [j - 1].ents_visible > temp.ents_visible)
		{
			spawn_point [j] = spawn_point [j - 1];
			j--;
		}
		spawn_point [j] = temp;
	}

	// show the list of spawnpoints
//	for (i = 0; i <= count; i++)
//		gstate->Con_Printf ("closest = %f, visible = %i\n", spawn_point [i].closest, spawn_point [i].ents_visible);

	// make sure there is no entity within 32.0 units, or we might get stuck
	if (spawn_point [0].closest < 32.0)
		return	NULL;
	else	
		return	spawn_point [0].spot;
}

//////////////////////////////
//	com_SelectSpawnPoint
//////////////////////////////

userEntity_t *com_SelectSpawnPoint (const char *className, const char *target)
{
	userEntity_t	*head;

	if (target)
	{
		gstate->Con_Printf ("spawning at %s\n", target);
		head = gstate->FirstEntity ();

		while (head)
		{
			if (head->className)
			{
				if (!stricmp (head->className, className))
				{
					if (head->targetname)
					{
						if (!stricmp (head->targetname, target))
						{
							gstate->Con_Printf ("found %s\n", head->className);
							return	head;
						}
					}	
				}
			}

			head = gstate->NextEntity (head);
		}
	}

	return com_FindEntity (className);
}

////////////////////////////////////////
//	com_CalcBoosts
//
//	recalculates player attributes based on boost values
////////////////////////////////////////

void	com_CalcBoosts (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	hook->max_health = (float) ATTR_HEALTH + (ATTR_HEALTH / ATTR_MAX_LEVELS) * (float) hook->vita_boost;

//	self->max_speed = (float) 1.0 + (1.0 / 10) * (float) hook->speed_boost;//
	self->max_speed = ATTR_RUNSPEED + (ATTR_RUNSPEED / ATTR_MAX_LEVELS) * (float) hook->speed_boost;

//	gstate->Con_Dprintf ("pwr %i spd %i att %i acr %i vit %i\n", hook->power_boost, hook->speed_boost, 
//						hook->attack_boost, hook->acro_boost, hook->vita_boost);
}

////////////////////////////////////////
//	com_UpdateClientAmmo
//
//	sends current ammo count down to the client for display
////////////////////////////////////////

void	com_UpdateClientAmmo (userEntity_t *self)
{
	weapon_t	*weapon;
	ammo_t		*ammo;

	if (self->flags & FL_CLIENT)
	{
		weapon = (weapon_t *) self->curWeapon;
		if (weapon)
		{
			ammo = (ammo_t *) weapon->ammo;

			if (ammo)
			{
				//	Q2FIXME:	Change to Q2 network functions
				/*
				gstate->WriteByte (self, MSG_ONE, SVC_AMMOCOUNT);
				gstate->WriteByte (self, MSG_ONE, ammo->count);
				*/
			}
		}
	}
}

////////////////////////////////
// com_Showbbox
////////////////////////////////

void	com_Showbbox (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs)
{
	Vector	amin, amax;

	amin = org + mins;
	amax = org + maxs;

	com_SpawnMarker (self, org, "models\\grenade.mdl", 0.05);

	///////////////////////////////////////////////////////////////////////////////////
	// draw the four vertical lines of the bounding box
	///////////////////////////////////////////////////////////////////////////////////

	com_SpawnMarker (self, Vector (amax.x, amax.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amin.x, amax.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amax.x, amin.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amin.x, amin.y, amax.z), "models\\grenade.mdl", 0.05);

	com_SpawnMarker (self, Vector (amax.x, amax.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amin.x, amax.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amax.x, amin.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, Vector (amin.x, amin.y, amin.z), "models\\grenade.mdl", 0.05);
}

////////////////////////////////////////
//	com_GetFrames
//
//	queries the model for start and end frames
////////////////////////////////////////

int	com_GetFrames (userEntity_t *self, char *seq_name, int *startFrame, int *endFrame)
{
	// int ret;
	int				i, j, len, frameNum;
	int				_frameFirst, _frameLast, found_sequence;
	//userFrameInfo_t frameInfo;
	char			frameName [16];
	char			baseName[100];

	found_sequence = FALSE;
	_frameFirst = 10000; _frameLast = -1;

	// go through all frames and get start and stop info
	for (i = 0;; i++)
	{
//	    if (!gstate->FrameInfo(self->modelIndex, i, &frameInfo)) break;
		gstate->GetFrameName (self->s.modelindex, i, frameName);
		if (!frameName || frameName [0] == 0x00)
			break;

//		len = strlen(frameInfo.frameName);
		len = strlen (frameName);

		// separate base and frame number
		for (j = 0; j < len; j++)
		{
//			if ((frameInfo.frameName[j] >= '0') && (frameInfo.frameName[j] <= '9'))
			if ((frameName [j] >= '0') && (frameName [j] <= '9'))
			{
				break;
			}
		}

		// make sure frame has a name
		if (j >= len) continue;

		//strcpy (baseName, frameInfo.frameName);
		strcpy (baseName, frameName);

		baseName [j] = 0x00;
		//frameNum    = atoi (&frameInfo.frameName[j]);
		frameNum    = atoi (&frameName [j]);

		if (!stricmp (baseName, seq_name))
		{
			found_sequence = TRUE;

			if (frameNum < _frameFirst) 
	        {
				* startFrame = i;
				_frameFirst = frameNum;
			}
			if (frameNum > _frameLast)  
			{
				* endFrame = i;
				_frameLast = frameNum;
			}
		}
	}

	return	found_sequence;
}
/*
{
	// int ret;
	int				i, j, len, frameNum;
	int				_frameFirst, _frameLast;
	userFrameInfo_t frameInfo;
	char			baseName[100];

	_frameFirst = 10000; _frameLast = -1;

	// go through all frames and get start and stop info
	for (i = 0;; i++)
	{
	    if (!gstate->FrameInfo(self->modelIndex, i, &frameInfo)) break;

		len = strlen(frameInfo.frameName);

		// seperate base and frame number
		for (j = 0; j < len; j++)
		{
			if ((frameInfo.frameName[j] >= '0') && (frameInfo.frameName[j] <= '9'))
			{
				break;
			}
		}

		// make sure frame has a name
		if (j >= len) continue;

		strcpy (baseName, frameInfo.frameName);

		baseName[j] = 0x00;
		frameNum    = atoi (&frameInfo.frameName[j]);

		if (!stricmp (baseName, seq_name))
		{
			if (frameNum < _frameFirst) 
	        {
				* startFrame = i;
				_frameFirst = frameNum;
			}
			if (frameNum > _frameLast)  
			{
				* endFrame = i;
				_frameLast = frameNum;
			}
		}
	}
}
*/

///////////////////////////////////////////////////
// com_ClientVisible
//
// returns a pointer to the first visible client
// or NULL if no clients are visible
//
// check from all corners of the entity self rather
// than the center of the entity
///////////////////////////////////////////////////

userEntity_t	*com_ClientVisible (userEntity_t *self)
{
	userEntity_t	*head;
	Vector			org, start, end;
	Vector			max, min, temp;

	head = gstate->FirstEntity ();

	org = (Vector (self->s.mins) + Vector (self->s.maxs)) * 0.5;
	min = Vector (self->s.mins);
	max = Vector (self->s.maxs);

	while (head)
	{		
		if (head->flags & FL_CLIENT)
		{
			if (head->view_entity != head)
			{
				if (com_Visible (self, head->view_entity))
					return	head;
			}
			else
			{
				if (com_Visible (self, head))
					return	head;
			}
/*
			temp = Vector (head->s.origin) + Vector (head->view_ofs);

			gstate->TraceLine (min.vec (), temp.vec (), TRUE, self, &trace);
			if (trace.fraction == 1.0)
				return	head;

			gstate->TraceLine (max.vec (), temp.vec (), TRUE, self, &trace);
			if (trace.fraction == 1.0)
				return	head;
*/
		}

		head = gstate->NextEntity (head);
	}

	return	NULL;
}

//////////////////////////////
//	com_Warning
//
//	should be called when a non-critical error is
//	encountered.
//
//	define HALT_ON_WARNINGS to halt the game on any
//	error.  Do this to force attention to problems
//	with maps, AI, etc. before shipping or demos
//////////////////////////////

void	com_Warning (char *fmt, ...)
{
	char		msg	[255];
	va_list		argptr;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

#ifdef	HALT_ON_ERRORS
	gstate->Error (msg);
#else
	gstate->Con_Printf ("%s", msg);
#endif
}

//////////////////////////////
//	com_Error
//
//	should be called when a non-critical error is
//	encountered.
//
//	define HALT_ON_ERRORS to halt the game on any
//	error.  Do this to force attention to problems
//	with maps, AI, etc. before shipping or demos
//////////////////////////////

void	com_Error (char *fmt, ...)
{
	char		msg	[255];
	va_list		argptr;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	gstate->Error (msg);
}

//////////////////////////////
//	com_SetClientModels
//
//////////////////////////////

void	com_SetClientModels (userEntity_t *self, userEntity_t *head, userEntity_t *torso, userEntity_t *legs, userEntity_t *weapon)
{
	//	Q2FIXME:	change to Q2 network functions
/*
	gstate->WriteByte (self, MSG_ONE, SVC_SETCLIENTMODELS);
	gstate->WriteEntity (self, MSG_ONE, head);
	gstate->WriteEntity (self, MSG_ONE, torso);
	gstate->WriteEntity (self, MSG_ONE, legs);
	gstate->WriteEntity (self, MSG_ONE, weapon);
*/

	gstate->WriteByte (SVC_SETCLIENTMODELS);
	gstate->WriteShort (head->s.number);
	gstate->WriteShort (torso->s.number);
	gstate->WriteShort (legs->s.number);
	gstate->WriteShort (weapon->s.number);
	gstate->UniCast (self, true);
}

///////////////////////////////////////////////
//	com_AngleDiff
//
//	returns the difference between passed angles
///////////////////////////////////////////////

float	com_AngleDiff (float ang1, float ang2)
{
	float	diff;

	ang1 = com_AngleMod (ang1);
	ang2 = com_AngleMod (ang2);

	diff = ang1 - ang2;

	if (diff < -180)
	{
		diff += 360;
		diff *= -1;
	}
	else if (diff < 0)
		diff = fabs (diff);
	else if (diff > 180)
		diff = fabs (diff - 360);
	else 
		diff *= -1;

	return	diff;
}

///////////////////////////////////////////////
//	com_Normal
//
//	returns the normal of the plane described by
//	points v1, v2, and v3 in vector out
///////////////////////////////////////////////

void	com_Normal (Vector v1, Vector v2, Vector v3, Vector &out)
{
	Vector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	out = a % b;
}

///////////////////////////////////////////////
//	com_SpawnFakeEntity
//
///////////////////////////////////////////////

userEntity_t	*com_SpawnFakeEntity (void)
{
	userEntity_t	*temp;

	temp = (userEntity_t *) com.malloc (sizeof (userEntity_t), MEM_MALLOC);
	temp->flags = FL_FAKEENTITY;

	//	malloc up gclient_t struct so it's just like a client
	temp->client = (gclient_t *) com.malloc (sizeof (gclient_t), MEM_MALLOC);

	return	temp;
}

///////////////////////////////////////////////
//	com_RemoveFakeEntity
//
//	generates an error if a real entity is passed
///////////////////////////////////////////////

void	com_RemoveFakeEntity (userEntity_t *ent)
{
	if (!ent)
		return;

	if (ent->flags & FL_FAKEENTITY)
	{
		com.free (ent->client);
		com.free (ent);
	}
	else
		com.Error ("com_RemoveFakeEntity: tried to remove a real entity.\n");
}

////////////////////////////////////////////////
//	com_Poison
//
//	Poisons an entity
//
//	when an entity is poisoned, damage hits are removed
//	every interval seconds, for time seconds
////////////////////////////////////////////////

void	com_Poison (userEntity_t *target, float damage, float time, float interval)
{
	playerHook_t	*hook = (playerHook_t *) target->userHook;

	if (target->flags & (FL_MONSTER + FL_BOT + FL_CLIENT))
	{
		if (!(hook->items & IT_POISON))
		{
			hook->poison_time = gstate->time + time;
			hook->poison_next_damage_time = gstate->time + 3.0;
		}
		else
			hook->poison_time += time;

		hook->poison_damage = damage;
		hook->poison_interval = interval;

		//	set poison flags
		hook->items |= IT_POISON;	
		hook->exp_flags |= EXP_POISON;	
	}
}

/////////////////////////////////////////////////
//	com_TeleFrag
//
//	gib any entity within self's bounding box
/////////////////////////////////////////////////

void	com_TeleFrag (userEntity_t *self, float damage)
{
	userEntity_t	*ent;
	Vector			e_org, s_org, temp;

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (ent->takedamage && ent->solid == SOLID_BBOX)
		{
			if (ent != self)
			{
				//	FIXME: check for exact bounding box collision

				//	determine if bounding boxes touch in z plane
				if (fabs (ent->s.origin [2] - self->s.origin [2]) <= self->s.maxs [2] + ent->s.maxs [2])
				{
					//	determine if bounding boxes touch in xy plane

					e_org = Vector (ent->s.origin);
					e_org.z = 0;
					s_org = Vector (self->s.origin);
					s_org.z = 0;

					temp = e_org - s_org;

					if (temp.vlen () <= self->s.maxs [0] + ent->s.maxs [0])
						com_Damage (ent, self, self, ent->s.origin, zero_vector, damage, DAMAGE_TELEFRAG | DAMAGE_DIR_TO_INFLICTOR);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////
//	com_GenID
//
//	returns a unique ID number.
//
//	get ID number from global ID_seed, which is just
//	incremented in each call to com_GenID and which
//	will wrap around from 65535 to 0
//
//	will never return 0, since 0 is used to signify
//	and entity that hasn't been given an ID at all
/////////////////////////////////////////////////////

int	com_GenID (void)
{
	ID_seed++;

	if (ID_seed == 0)
		ID_seed++;

	return	ID_seed;
}

/*
///////////////////////////////////////////
//	rnd
//
//	returns a random float between 0 and 1
///////////////////////////////////////////

float	rnd (void)
{
	return (float)rand () / (float)RAND_MAX;
}
*/

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

typedef	struct	lineHook_s
{
	Vector	end;
} lineHook_t;

void	line_think (userEntity_t *self)
{
	userEntity_t	*ignore;
	Vector			start;
	Vector			end;
	Vector			point;
	Vector			last_movedir;
	Vector			temp;
	int				count;
	static			vec3_t	lmins = {-4, -4, -4};
	static			vec3_t	lmaxs = {4, 4, 4};
	lineHook_t		*hook = (lineHook_t *) self->userHook;

	// update the frame
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
	
	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		last_movedir = Vector (self->movedir);
		point = Vector (self->enemy->absmin) + 0.5 * Vector (self->enemy->size);
		temp = point - Vector (self->s.origin);
		temp.Normalize ();
		temp.set (self->movedir);

		if (temp != last_movedir)
			self->spawnflags |= 0x80000000;
	}
	else
		temp = Vector (self->movedir);

	ignore = self;

	start = Vector (self->s.origin);
//	end = start + temp * 64;
	hook->end.set (self->s.old_origin);

	if (gstate->time > self->delay)
		gstate->RemoveEntity (self);
	else
		self->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	com_DrawLine (userEntity_t *self, Vector org, Vector end, float lifetime)
{
	userEntity_t	*laser;
	Vector			temp;
	lineHook_t		*hook;

	laser = gstate->SpawnEntity ();

	laser->className = "LaserLine";
	laser->owner = self;
	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
	laser->s.alpha = 1.0;
	laser->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	laser->s.frame = 4;
	laser->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	// set the color
	laser->s.skinnum = 0xf2f2f0f0;

	VectorSet (laser->s.mins, -8, -8, -8);
	VectorSet (laser->s.maxs, 8, 8, 8);

	//	set the starting position
	org.set (laser->s.origin);
	gstate->LinkEntity (laser);

	//	find the direction to target
	temp = end - org;
	temp.Normalize ();
	temp.set (laser->movedir);

	laser->spawnflags |= 0x80000001;
	laser->svflags -= (laser->svflags & SVF_NOCLIENT);

	//	set the ending position
	end.set (laser->s.old_origin);

	laser->userHook = com.malloc_hook (sizeof (lineHook_t), MEM_MALLOC);
	hook = (lineHook_t *) laser->userHook;
	hook->end = end;

	laser->delay = lifetime;//gstate->time + 1.0;

	laser->think = line_think;
	laser->nextthink = gstate->time + 0.1;
}

/////////////////////////////////////////////////////////////////////
//	com_DrawBoundingBox
//	draw bounding box around an entity
/////////////////////////////////////////////////////////////////////

void	com_DrawBoundingBox (userEntity_t *self, float lifetime)
{
	Vector min = Vector (self->absmin), max = Vector (self->absmax);
	Vector org, end;

	// draw the top box (4 lines)

	com_DrawLine (self, Vector (min.x, min.y, min.z) ,  Vector (max.x, min.y, min.z), lifetime);
	com_DrawLine (self, Vector (max.x, min.y, min.z) ,  Vector (max.x, max.y, min.z), lifetime);
	com_DrawLine (self, Vector (max.x, max.y, min.z) ,  Vector (min.x, max.y, min.z), lifetime);
	com_DrawLine (self, Vector (min.x, max.y, min.z) ,  Vector (min.x, min.y, min.z), lifetime);

	// draw the bottom box (4 lines)
	com_DrawLine (self, Vector (min.x, min.y, max.z) ,  Vector (max.x, min.y, max.z), lifetime);
	com_DrawLine (self, Vector (max.x, min.y, max.z) ,  Vector (max.x, max.y, max.z), lifetime);
	com_DrawLine (self, Vector (max.x, max.y, max.z) ,  Vector (min.x, max.y, max.z), lifetime);
	com_DrawLine (self, Vector (min.x, max.y, max.z) ,  Vector (min.x, min.y, max.z), lifetime);

	// draw the Z connecting lines (4 lines)
	com_DrawLine (self, Vector (min.x, min.y, min.z) , Vector (min.x, min.y, max.z), lifetime);
	com_DrawLine (self, Vector (max.x, min.y, min.z) , Vector (max.x, min.y, max.z), lifetime); 
	com_DrawLine (self, Vector (max.x, max.y, min.z) , Vector (max.x, max.y, max.z), lifetime);
	com_DrawLine (self, Vector (min.x, max.y, min.z) , Vector (min.x, max.y, max.z), lifetime);

	return;
}

///////////////////////////////////////////////////////////////////////////////
//
// com_AnimateEntity()
//
///////////////////////////////////////////////////////////////////////////////
void com_AnimateEntity(userEntity_t *ent, short first, short last, unsigned short frameflags, float frametime)
{
   ent->s.frame=first;
   ent->s.frameInfo.startFrame=first;
   ent->s.frameInfo.endFrame=last;
   ent->s.frameInfo.frameFlags=frameflags | FRAME_FORCEINDEX;
   ent->s.frameInfo.frameInc=1;
   ent->s.frameInfo.frameTime=frametime;
   ent->s.frameInfo.frameState=FRSTATE_STARTANIM;

   return;
}

///////////////////////////////////////////////////////////////////////////////
//	com_FrameUpdate
//
//	handles frame updates on the server side
///////////////////////////////////////////////////////////////////////////////

void	com_FrameUpdate (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			temp;
	int				add, new_frame;
/*
	if (self->s.frameInfo.frameState & FRSTATE_STARTANIM)
	{
		//	so weapons, which get their frames set in-between server physics, 
		//	don't get a frame ahead
		self->s.frameInfo.frameState -= FRSTATE_STARTANIM;
//		return;
	}
*/
	// no frame changes if FRAME_STATIC is set
	if (!(self->s.frameInfo.frameFlags & FRAME_STATIC))	//damnthee
	{
		// increment frames according to length of last frame
		self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime + gstate->frametime;
		
		if (self->s.frameInfo.next_frameTime > self->s.frameInfo.frameTime)
		{
			temp = floor (self->s.frameInfo.next_frameTime / self->s.frameInfo.frameTime);
			add = (int)(self->s.frameInfo.frameInc * temp);
			new_frame = self->s.frame + add;

			///////////////////////////////////////////////////////////////////////
			//	set/clear FRSTATE_LAST flag if on last frame
			///////////////////////////////////////////////////////////////////////

			if (self->s.frameInfo.endFrame <= new_frame && self->s.frameInfo.endFrame > self->s.frame)
				self->s.frameInfo.frameState |= FRSTATE_LAST;

//			if (self->flags & FL_DEBUG)
//				gstate->printxy (0, 80, "sf=%i ef=%i fi=%i add=%i\n", self->s.frameInfo.startFrame, self->s.frameInfo.endFrame, self->s.frame, add);

			///////////////////////////////////////////////////////////////////////
			//	set FRSTATE_PLAYSOUND flags if correct frame
			///////////////////////////////////////////////////////////////////////

			if (self->s.frameInfo.sound1Frame >= self->s.frame && self->s.frameInfo.sound1Frame < new_frame)
				self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND1;
			if (self->s.frameInfo.sound2Frame >= self->s.frame &&  self->s.frameInfo.sound2Frame < new_frame)
				self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND2;

			self->s.frame += add;

			// are we currently looping within a larger sequence?
			if (self->s.frameInfo.frameFlags & FRAME_REPEAT)
			{
				// determine if sequence needs to loop back to loopStart
				if (self->s.frame > self->s.frameInfo.endLoop)
				{
					self->s.frame = self->s.frameInfo.startLoop;
					self->s.frameInfo.loopCount--;
					// if count is <= 0, stop the looping of this partial sequence
					if (self->s.frameInfo.loopCount <= 0)
					{
						self->s.frameInfo.frameFlags -= FRAME_REPEAT;
						self->s.frameInfo.startLoop = self->s.frameInfo.startFrame;
						self->s.frameInfo.endLoop = self->s.frameInfo.endFrame;
					}
				}
			}
			// going through an entire sequence
			else
			{
				// determine if sequence needs to loop back to start
				if (self->s.frame > self->s.frameInfo.endFrame)
				{
					if (self->s.frameInfo.frameFlags & FRAME_ONCE)
					{
						// we've gone through once, so set frames to static (no updates)
						self->s.frameInfo.frameState |= FRSTATE_STOPPED;
						self->s.frame = self->s.frameInfo.endFrame;

						//	no weapon switches in this update procedure
//						if (self->s.frameInfo.frameFlags & FRAME_WEAPONSWITCH)
//							hook->weapon_next (self);
					}
					else
						//	looping through entire sequence
						self->s.frame = self->s.frameInfo.startFrame;
				}
			}
			
			self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime - (temp * self->s.frameInfo.frameTime);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitSub
//
//	set up pointers to functions in com_sub.cpp
///////////////////////////////////////////////////////////////////////////////

void	com_InitSub (void)
{
	//	vec3_t functions
	com.AngleVectors = com_AngleVectors;
	com.AngleMod = com_AngleMod;
	com.VecToYaw = com_VecToYaw;
	com.VecToAngles = com_VecToAngles;
	com.YawDiff = com_YawDiff;
	com.BestDelta = com_BestDelta;
	com.AngleDiff = com_AngleDiff;
	com.Normal = com_Normal;
	com.VectorMA = com_VectorMA;
	com.VectorScale = com_VectorScale;


	com.ZeroVector = com_ZeroVector;
	com.SetVector = com_SetVector;
	com.MultVector = com_MultVector;
	com.CopyVector = com_CopyVector;
	com.vtos = com_vtos;
	com.vtos1 = com_vtos1;

	com.GetMinsMaxs = com_GetMinsMaxs;

	com.ChangeYaw = com_ChangeYaw;
	com.ChangePitch = com_ChangePitch;
	com.ChangeRoll = com_ChangeRoll;

	com.SetMovedir = com_SetMovedir;

	//	debugging functions
	com.SpawnMarker = com_SpawnMarker;
	com.DrawLine = com_DrawLine;
	com.Showbbox = com_Showbbox;
	com.DrawBoundingBox = com_DrawBoundingBox;

	//	miscellaneous functions
	com.Damage = com_Damage;
	com.CanDamage = com_CanDamage;
	com.RadiusDamage = com_RadiusDamage;
	com.ValidTouch = com_ValidTouch;
	com.UseTargets = com_UseTargets;
	com.Health = com_Health;
	com.Respawn = com_Respawn;
	com.CheckTargetForActiveChildren = com_CheckTargetForActiveChildren;

	com.Visible = com_Visible;
	com.FindEntity = com_FindEntity;
	com.FindClosestEntity = com_FindClosestEntity;
	com.FindTarget = com_FindTarget;
	com.DeathmatchSpawnPoint = com_DeathmatchSpawnPoint;
	com.SelectSpawnPoint = com_SelectSpawnPoint;

	//&&& AMW 6.12.98 - group searching functions
	com.FindFirstGroupMember = com_FindFirstGroupMember;
	com.FindNextGroupMember	= com_FindNextGroupMember;

	//&&& AMW 6.25.98 - function to calculate transformed offsets from an origin point
	com.TransformChildOffset = com_TransformChildOffset;
	
	com.SetRespawn = com_SetRespawn;
	com.CalcBoosts = com_CalcBoosts;

	com.GetFrames = com_GetFrames;

	com.ClientVisible = com_ClientVisible;
	com.Warning = com_Warning;
	com.Error = com_Error;

	com.SetClientModels = com_SetClientModels;

	com.Poison = com_Poison;
	com.TeleFrag = com_TeleFrag;
	com.GenID = com_GenID;

	//	Q2FIXME: thinkFuncs from world.dll, merge with FakeEntities
	com.SpawnFakeEntity = com_SpawnFakeEntity;
	com.RemoveFakeEntity = com_RemoveFakeEntity;

	//	Q2FIXME: still needed in common.dll?
	com.ClientPrint = com_ClientPrint;
	com.UpdateClientAmmo = com_UpdateClientAmmo;
	com.AmbientVolume = com_AmbientVolume;
	
	com.FrameUpdate = com_FrameUpdate;
	com.AnimateEntity = com_AnimateEntity;
	
	com.RotateBoundingBox = com_RotateBoundingBox;
}

