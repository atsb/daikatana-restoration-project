//unix - avoid conflict with standard library symbol "index" - part 1
#if !_MSC_VER
#define index _index
#endif

//#define	HALT_ON_ERRORS
//#include <windows.h>
#include	<stdio.h>
#include	<math.h>
//#include	<windows.h>
#include	"dk_system.h"

//	header files in daikatana\user
#include	"p_user.h"
#include	"hooks.h"

#include	"p_global.h"
#include "l__language.h"

#include "com_teamplay.h"

//unix - avoid conflict with standard library symbol "index" - part 2
#if !_MSC_VER
#undef index
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Prototypes
//////////////////////////////////////////////////////////////////////////////////////////

userEntity_t *FindRadius(userEntity_t *ent_from, CVector &origin, float radius);
void *DLL_FindFunction(char *funcName);
void com_ClientPrint (userEntity_t *self, char *msg);
void com_Damage (userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, CVector &point_of_impact, 
				 CVector &damage_vec, float damage, unsigned long damage_flags);


//////////////////////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////////////////////

//	globals for vectors that are used all over
CVector			forward, right, up;
CVector			zero_vector(0.0, 0.0, 0.0);
userTrace_t		trace;
common_export_t	com;
common_export_t *memory_com = &com;

static int			ID_seed = 0;

////////////////////////////////////////
//	vtos
//
////////////////////////////////////////

static	int		index;
static	char	str [8][32];

char	*com_vtos (CVector &v)
{
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str [index];
	index = (index + 1)&7;

// SCG[1/16/00]: 	sprintf (s, "(%.2f %.2f %.2f)", v.x, v.y, v.z);
	Com_sprintf (s, 32, "(%.2f %.2f %.2f)", v.x, v.y, v.z);

	return s;
}

////////////////////////////////////////
//	com_VecToYPR
//
//	returns the pitch, yaw and roll equivalents of the passed vector
//	for use with Bmodels
////////////////////////////////////////

void	com_VecToYPR (CVector &vec2, CVector &vec1)
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
		{
			yaw += 360;
		}

		// ask Pythagorus
		forward = sqrt (vec1.x*vec1.x + vec1.y*vec1.y);
		pitch = (int) (atan2(vec1.z, vec1.y) * 180 / M_PI);
		if (pitch < 0)
		{
			pitch += 360;
		}

		forward = sqrt (vec1.x*vec1.x + vec1.y*vec1.y);
		roll = (int) (atan2 (vec1.z, vec1.x) * 180 / M_PI);
		if (roll < 0)
		{
			roll += 360;
		}
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
	if (facing_yaw < 0) 
	{
		facing_yaw += 360;
	}
	if (to_targ_yaw < 0) 
	{
		to_targ_yaw += 360;
	}

	// now determine the shortest direction to turn, left or right, to
	// get to the to_targ_yaw from facing_yaw

	if (facing_yaw < to_targ_yaw)
	{
		yaw_delta = to_targ_yaw - facing_yaw;
		if (yaw_delta > 180)
		{
			yaw_delta = fabs (yaw_delta - 360.0);
		}
	}
	else
	{
		yaw_delta = facing_yaw - to_targ_yaw;
		if (yaw_delta > 180)
		{
			yaw_delta = fabs (yaw_delta - 360.0);
		}
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
//	start_angle = AngleMod (start_angle);
//	end_angle = AngleMod (end_angle);	

	if (start_angle < 0) 
	{
		start_angle += 360;
	}
	if (end_angle < 0) 
	{
		end_angle += 360;
	}

	// now determine the shortest direction to turn, left or right, to
	// get to the end_angle from start_angle

	if (start_angle < end_angle)
	{
		*angle_diff = end_angle - start_angle;
		if (*angle_diff < 180)
		{
			*best_delta = 1.0;
		}
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
		{
			*best_delta = -1.0;
		}
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


///////////////////////////////////////////////////////////////////////////////////////////
// com_RotateBoundingBox
//
// rotate the bounding box of an entity... only boxes on 90, 180, 270 and 360 are generated
//
///////////////////////////////////////////////////////////////////////////////////////////
void com_RotateBoundingBox (userEntity_t *self)
{
	CVector	tmp, v;
	CVector	forward, right, up, mins, maxs, tmins, tmaxs;

	mins = self->s.mins;
	maxs = self->s.maxs;

	tmins.Set( 99999, 99999, 99999 );
	tmaxs.Set( -99999, -99999, -99999 );

	AngleToVectors(self->s.angles, forward, right, up);

	for ( int i = 0; i < 8; i++ )
	{
		if ( i & 1 )
		{
			tmp.x = mins.x;
		}
		else
		{
			tmp.x = maxs.x;
		}

		if ( i & 2 )
		{
			tmp.y = mins.y;
		}
		else
		{
			tmp.y = maxs.y;
		}

		if ( i & 4 )
		{
			tmp.z = mins.z;
		}
		else
		{
			tmp.z = maxs.z;
		}

		v = forward * tmp.x;
		VectorMA(v, right, -tmp.y, v);
		VectorMA(v, up, tmp.z, v);

		if (v.x < tmins.x)
		{
			tmins.x = v.x;
		}
		if (v.x > tmaxs.x)
		{
			tmaxs.x = v.x;
		}
		if (v.y < tmins.y)
		{
			tmins.y = v.y;
		}
		if (v.y > tmaxs.y)
		{
			tmaxs.y = v.y;
		}
		if (v.z < tmins.z)
		{
			tmins.z = v.z;
		}
		if (v.z > tmaxs.z)
		{
			tmaxs.z = v.z;
		}
	}

	// update the bounding box points
	self->s.mins = tmins;
	self->s.maxs = tmaxs;

	// SCG[7/23/99]: Since we've rotated and everything, 
	// SCG[7/23/99]: it would be a good idea to calculate this too...
	self->absmin = self->s.origin + self->s.mins;	
	self->absmax = self->s.origin + self->s.maxs;

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
	CVector			org, end;
	trace_t			tr;

	// get center of bmodel as origin
	if (self->movetype == MOVETYPE_PUSH || self->solid == SOLID_BSP || self->solid == SOLID_TRIGGER)
	{
		org = (self->absmin + self->absmax) * 0.5;
	}
	else 
	{
		org = self->s.origin;
	}

	if (ent->movetype == MOVETYPE_PUSH || ent->solid == SOLID_BSP || ent->solid == SOLID_TRIGGER)
	{
		end = (ent->absmin + ent->absmax) * 0.5;
	}
	else 
	{
		end = ent->s.origin;
	}

	// make sure we're checking from eye to eye
	org = org + self->view_ofs;
	end = end + self->view_ofs;
	tr = gi.TraceLine(org, end, self, MASK_OPAQUE);

	if (tr.fraction == 1.0 || tr.ent == ent && !tr.startsolid && !tr.allsolid)
	{
		org = org - tr.endpos;
		return	(int) org.Length();
	}

	return	FALSE;
}

//////////////////////
//	com_ChangeYaw
//
//////////////////////

void	com_ChangeYaw (userEntity_t *ent)
{
	float		ideal, current, move, speed;

	current = AngleMod (ent->s.angles.yaw);
	ideal = AngleMod (ent->ideal_ang.yaw);
	speed = ent->ang_speed.yaw;
	
	if (current == ideal)
		return;

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
		{
			move = move - 360;
		}
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
		{
			move = move + 360;
		}
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
		{
			move = speed;
		}
		else
		{
			ent->s.angles.y = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
		{
			move = -speed;
		}
		else
		{
			ent->s.angles.y = ideal;
			return;
		}
	}
	
	ent->s.angles.y = AngleMod (current + move);
}

//////////////////////
//	com_ChangePitch
//
//////////////////////

void	com_ChangePitch (userEntity_t *ent)
{
	float		ideal, current, move, speed;
	CVector		temp;

//	if (!ent->goalentity)
//		return;

	current = AngleMod (ent->s.angles.pitch);
	ideal = AngleMod (ent->ideal_ang.pitch);
	speed = ent->ang_speed.pitch;
	
	if (current == ideal)
	{
		return;
	}

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
		{
			move = move - 360;
		}
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
		{
			move = move + 360;
		}
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
		{
			move = speed;
		}
		else
		{
			ent->s.angles.x = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
		{
			move = -speed;
		}
		else
		{
			ent->s.angles.x = ideal;
			return;
		}
	}
	
	ent->s.angles.x = AngleMod(current + move);
}

//////////////////////
//	com_ChangeRoll
//
//////////////////////

void	com_ChangeRoll(userEntity_t *ent)
{
	float		ideal, current, move, speed;
	CVector		temp;

	if (!ent->goalentity)
	{
		return;
	}

	current = AngleMod (ent->s.angles.roll);
	ideal = AngleMod (ent->ideal_ang.roll);
	speed = ent->ang_speed.roll;
	
	if (current == ideal)
	{
		return;
	}

	// get the difference between the ideal and current yaws
	move = ideal - current;

	// if ideal yaw is greater than current and difference is >= 180
	// subtract 360
	if (ideal > current)
	{
		if (move >= 180)
		{
			move = move - 360;
		}
	}
	// if ideal is < current and difference is <= -180 then add 360
	else
	{
		if (move <= -180)
		{
			move = move + 360;
		}
	}

	// if move direction is positive, speed
	if (move > 0)
	{
		if (move > speed)
		{
			move = speed;
		}
		else
		{
			ent->s.angles.z = ideal;
			return;
		}
	}
	else
	{
		if (move < -speed)
		{
			move = -speed;
		}
		else
		{
			ent->s.angles.z = ideal;
			return;
		}
	}
	
	ent->s.angles.z = AngleMod (current + move);
}

////////////////////////////////////////
//	com_SetMovedir
//
//	QEd only writes a single float for angles (bad idea), so up and down are
//	just constant angles.
////////////////////////////////////////

void	com_SetMovedir (userEntity_t *self)
{
	CVector	fw, up, right, temp;
	CVector	angles = self->s.angles;
	CVector movedir = self->movedir;

	if (!self->className)
	{
		gi.Con_Printf("SetMovedir: no className\n");
		return;
	}
	if (!stricmp (self->className, "func_plat"))
	{
		if (angles.y == -1.0)
		{
			movedir.Set(0.0, 0.0, 1.0);
		}
		else 
		if (angles.y == -2.0)
		{
			movedir.Set(0.0, 0.0, -1.0);
		}
		else
		{
			//	plats default to move up
			movedir.Set(0.0, 0.0, -1.0);
		}
	}
	else
	{
		if (angles.x == 0.0 && angles.y == -1.0 && angles.z == 0.0)
		{
			movedir.Set(0.0, 0.0, 1.0);
		}
		else 
		if (angles.x == 0.0 && angles.y == -2.0 && angles.z == 0.0)
		{
			movedir.Set(0.0, 0.0, -1.0);
		}
		else
		{
			movedir.Set(0.0, 0.0, -1.0);
			angles.AngleToVectors(movedir, up, right);
		}
	}

	angles.Zero();
	self->s.angles = angles;

	movedir.Normalize();
	self->movedir = movedir;
};

////////////////////////////////////////
//	com_FindEntity
//
//	Returns the entity with matching classname 
////////////////////////////////////////

userEntity_t *com_FindEntity (const char *name)
{
	userEntity_t	*head;
	
	head = P_FirstEntity ();

	while (head)
	{
		if (head->className)
		{
			if (!stricmp(head->className, name))
			{
				return(head);
			}
		}

		head = P_NextEntity (head);
	}

	return NULL;
}

////////////////////////////////////////
//	com_FindClosestEntity
//
//	Returns the closest entity with matching classname 
////////////////////////////////////////

userEntity_t *com_FindClosestEntity (userEntity_t *self, const char *classname)
{
	userEntity_t	*head, *closest;
	float			dist, last_dist;
	
	#define AWHOLEHELLOFALOT	1000000.0;
	
	head = P_FirstEntity ();
	last_dist = AWHOLEHELLOFALOT;
	closest = NULL;

	while (head)
	{
		if (head->className)
		{
			if (!stricmp(head->className, classname))
			{
				dist = VectorDistance (self->s.origin, head->s.origin);
				if (dist < last_dist && com_Visible (self, head))
				{
					closest = head;
					last_dist = dist;
				}
			}
		}

		head = P_NextEntity (head);
	}

	return closest;
}

////////////////////////////////////////
//	com_FindClosestEntityPt
//
//	Returns the closest entity with matching classname to pos with optional targetname
////////////////////////////////////////

userEntity_t *com_FindClosestEntityPt ( CVector &pos, const char *classname, const char *targetname)
{
	userEntity_t	*head, *closest;
	float			dist, last_dist;
	
	#define AWHOLEHELLOFALOT	1000000.0;
	
	head = P_FirstEntity ();
	last_dist = AWHOLEHELLOFALOT;
	closest = NULL;

	while (head)
	{
		if ( head->className && !stricmp(head->className, classname) )
		{
			// find the entity whose target == targetname
			// NSS[12/7/99]:If the entity even has a target check for it.
			if(head->target)
			{
				if ( !targetname || !stricmp(head->target, targetname) )
				{
					if ( head->s.origin.Length() > 0 )
					{
						dist = VectorDistance (pos, head->s.origin);
						if (dist < last_dist && gi.inPVS(pos, head->s.origin))
						{
							closest = head;
							last_dist = dist;
						}
					}
					else
					{
						CVector newOrigin;
						newOrigin = (head->absmax + head->absmin) * 0.5f;

						dist = VectorDistance(pos, newOrigin);
						if (dist < last_dist && gi.inPVS(pos, newOrigin))
						{
							closest = head;
							last_dist = dist;
						}
					}
				}
			}
		}

		head = P_NextEntity (head);
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
	
    // handle bad parameter
    if(!name)
    {
        gi.Con_Dprintf("com_FindTarget(name=NULL)\n");
        return NULL;
    }

	head = P_FirstEntity ();

	while (head)
	{
		if (head->targetname)
		{
			if (!stricmp(head->targetname, name))
				return(head);
		}

		head = P_NextEntity (head);
	}

	return NULL;
}

//////////////////////////////////////////////////////////////
//	com_FindNodeTarget
//
//	Returns the first entity with a matching nodeTargetName 
//////////////////////////////////////////////////////////////

userEntity_t *com_FindNodeTarget (const char *name)
{
	if ( name )
	{
		userEntity_t	*head;
		
		head = P_FirstEntity ();

		while (head)
		{
			if (head->nodeTargetName)
			{
				if (!stricmp(head->nodeTargetName, name))
					return(head);
			}

			head = P_NextEntity (head);
		}
	}

	return NULL;
}


/*
////////////////////////////////////////
//	FindScriptEntity
//
//	Returns the first entity with a matching scriptname
////////////////////////////////////////
//
// 8.12 dsn: this function is REALLY misleading, because the self->scriptname variable
// is used for AI.. *not* cinescripting
//
userEntity_t *FindScriptEntity(const char *name)
{
    if (name == NULL) return NULL;

    userEntity_t *head;
    
    head = P_FirstEntity();

    while (head != NULL)
    {
        //see if this entity has the given scriptname.
        if (head->scriptname != NULL) {
            if (stricmp(head->scriptname, name) == 0) {
                //found it.
                return head;
            }
        }

        //look at the next entity.
        head = P_NextEntity(head);
    }

    //couldn't find an entity with the given scriptname.
    return NULL;
}
*/


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
	
	head = P_FirstEntity ();

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

		head = P_NextEntity (head);
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
	head = P_NextEntity (head);

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

		head = P_NextEntity (head);
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
//	com_TransformChildOffset
//
//	Fills in 'out' with the transformed version of offs given 'origin' and 'angles' 
/////////////////////////////////////////////////////////////////////////////////////

void com_TransformChildOffset( CVector &origin, CVector &offs, CVector &angles, bool bFlipYaw, CVector &out )
{
	CVector forward, right, up, offset;

	// calculate the transformation matrix
	AngleToVectors(angles, forward, right, up);

	// transform the offset from the parent
	offset = offs;
	
	// negate Y
	if (bFlipYaw)
	{
		offset.y = -offset.y;
	}
	// transform the coordinates
	out.x = DotProduct(offset, forward) + origin.x;
	out.y = DotProduct(offset, right) + origin.y;
	out.z = DotProduct(offset, up) + origin.z;

	return;
}


////////////////////////////////////////
//	com_RemoveMarker
//	Spawns a temporary marker entity
////////////////////////////////////////

void	com_RemoveMarker (userEntity_t *self)
{
	self->remove (self);
}

////////////////////////////////////////
//	com_SpawnMarker
//	Spawns a temporary marker entity
////////////////////////////////////////

void	com_SpawnMarker (userEntity_t *self, CVector &org, char *model_name, float time)
{
	userEntity_t	*marker;
/*
	if (self->className)
		if (stricmp (self->className, "player") != 0)
			return;
*/
	// don't ever spawn these in deathmatch games
	if (deathmatch->value)
		return;

	marker = P_SpawnEdict ();

	marker->s.mins.Zero();
	marker->s.maxs.Zero();

	marker->s.origin = org;
	gi.linkentity (marker);

	marker->s.modelindex = gi.modelindex (model_name);

	marker->solid = SOLID_NOT;
	marker->movetype = MOVETYPE_NONE;

	marker->nextthink = level.time + time;
	marker->think = com_RemoveMarker;

//	com_CopyVector (marker->velocity, forward);
//	com_MultVector (marker->velocity, 320);
}

///////////////////////////////////////////////////////////////////////////////
//	com_FlashClient
//
//	NOTE: this is only for client entities that you know should be visible
//	to the effect.  i.e. it doesn't check visibility etc
//
///////////////////////////////////////////////////////////////////////////////

void com_FlashClient (userEntity_t *self, float amt, float r, float g, float b, float vel = 0)
{
    if (!self)
      return;

	// don't flash clients that are no clipping..
	if (self->movetype == MOVETYPE_NOCLIP)
		return;

	if (self && self->client && (self->flags & FL_CLIENT))
	{
		self->client->flash_alpha	 = amt;
		self->client->flash_blend.x = r;
		self->client->flash_blend.y = g;
		self->client->flash_blend.z = b;
        self->client->flash_velocity = vel;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	com_SpawnBlood
//
///////////////////////////////////////////////////////////////////////////////

void com_SpawnBlood ( CVector &origin, CVector &normal, int damage, float health, int type)
{

//	if (type == TE_SPARKS)
//		return;

    CVector vecColor(0.8, 0.8, 0.4);

	// SCG[9/8/99]: The client doesn't respond to anything more than 64
	// SCG[9/8/99]: so why do it here?
//	if (damage > 255)
//		damage = 255;
	if (damage > 64)
		damage = 64;

// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
//Actung!  No Blood!
	if(sv_violence->value > 0)
		return;
	type = TE_SPARKS;
#else
	if (type == TE_BLOOD)
	{
		//NSS[11/17/99]:I think these were backwards... but why exit when not and re-assign the type to sparks if so?
		//if (sv_violence->value == 1)
		//	type = TE_SPARKS;
		//else if (sv_violence->value == 0)
		//	return;
		if(sv_violence->value > 0)
			return;
	}
#endif
	gi.WriteByte (SVC_TEMP_ENTITY);

	gi.WriteByte (type);

    
	if(health > 0.0)
	{
		// SCG[9/8/99]: more blood for less health
		float invHealth = 100.0 / health;
		damage *= invHealth;
	}

	//	Fixme:	base blood on amount of damage
	gi.WriteByte (damage);

	gi.WritePosition (origin);
	
    if(type == TE_SPARKS)
    {
        gi.WritePosition (normal);			
        gi.WritePosition(vecColor);
        gi.WriteByte(1);
    } else 
	{
        gi.WriteDir (normal);			
    }
	gi.multicast (origin, MULTICAST_PVS);
}

///////////////////////////////////////////////////////////////////////////////
//	com_ArmorDamage
//
//	computes the amount of damage done to armor during an attack
//	and adjusts armor value accordingly
///////////////////////////////////////////////////////////////////////////////
#define ALL_ARMOR_MASK ( IT_PLASTEEL | IT_CHROMATIC | IT_SILVER | IT_GOLD | IT_CHAINMAIL | IT_BLACK_ADAMANT | IT_KEVLAR | IT_EBONITE )
float com_ArmorDamage( userEntity_t *pTarget, userEntity_t *pInflictor, float fDamage )
{
	// SCG[11/12/99]: We dont need to check to see of hook is valid
	// SCG[11/12/99]: because this routine does not get called if it is not.
	playerHook_t	*pHook = ( playerHook_t * ) pTarget->userHook;
	float			fModifiedDamage, fOriginalArmorValue, fAbsorbAmount;

	// SCG[11/12/99]: Check for megashield damage
	if( pHook->invulnerability_time >= level.time )
	{
		return 0;
	}

	// SCG[11/12/99]: Calculate the damage
	fOriginalArmorValue = pTarget->armor_val;

	// SCG[11/12/99]: Check for special case damage modification
	if( ( pHook->items & IT_CHROMATIC ) && ( pInflictor->EntityType == PROJECTILE_ION ) )
	{
		fModifiedDamage = fDamage * 0.1;
		fAbsorbAmount	= fDamage * 0.9;
	}
	else
	{
		//fAbsorbAmount	= fDamage * (0.01 * pTarget->armor_abs); // cek[11-27-99] this is a percent..
		fAbsorbAmount	= fDamage * pTarget->armor_abs; //NSS[11/29/99]:abs value is already a percent value.
		fModifiedDamage	= fDamage - fAbsorbAmount;
	}

	// SCG[11/12/99]: Reduce the armor value by the absorb amount
	pTarget->armor_val -= fAbsorbAmount;

	if( pTarget->armor_val <= 0 )
	{
		pTarget->armor_val = 0;
        pTarget->armor_abs = 0;

		if( pHook )
		{
			pHook->items &= ~ALL_ARMOR_MASK;
		}
	}

	// SCG[11/12/99]: Notify client of armor change
	// SCG[11/12/99]: Do we need to do this? This was left 
	// SCG[11/12/99]: because original com_ArmorDamage had it.
	if( fOriginalArmorValue != pTarget->armor_val )
	{

	}

	return fModifiedDamage;
}


#define ANNOYING_DMG_MSGS		0

///////////////////////////////////////////////////////////////////////
//  com_KickView
//
//  Description:
//      Adjusts the client view angles and FOV, including the velocity
//      to and from the effect extents.
//
//  Parameters:
//      userEntity_t *entPlayer     Affected client
//      CVector vecAngles           Extent of view angles
//      float fFOV                  Extent of Field Of View change
//      short sToVelocity           How long to take to get TO the FX extents
//      short sReturnVelocity       How long to take to return to a "normal" view
//
//  Return:
//      void
//
//  Bugs:
//      Overrides currently executing extent effects, that is, effects are 
//      not additive.
void com_KickView(userEntity_t *entPlayer, CVector vecAngles, float fFOV, short sToVelocity, short sReturnVelocity)
{
    if(!(entPlayer->flags & FL_CLIENT) || !serverState.gi->TestUnicast(entPlayer, true))
        return;

    gi.WriteByte(SVC_TEMP_ENTITY);
    gi.WriteByte(TE_KICKVIEW);
    gi.WritePosition(vecAngles);
    gi.WriteFloat(fFOV);
    gi.WriteShort(sToVelocity);
    gi.WriteShort(sReturnVelocity);
    gi.unicast(entPlayer, true);
}
///////////////////////////////////////////////////////////////////////

qboolean com_CheckFriends( userEntity_t *attacker, userEntity_t *target )
{
	// on the same team
	if( attacker->team == target->team )
	{
		return true;
	}

	return false;
}

CVector com_DamageVelocity( userEntity_t *target, 
				userEntity_t *inflictor, 
				userEntity_t *attacker, 
				CVector &damage_vec, 
				float damage, 
				unsigned long damage_flags )
{
	CVector vDamageDir, temp, velocity;

	if (!(target->flags & FL_PUSHABLE) || (damage_flags & DAMAGE_NO_VELOCITY))
	{
		damage_flags &= DAMAGE_ONLY_FLAGS;
		damage_flags |= DAMAGE_NONE;
	}

	if (damage_flags & DAMAGE_DIR_TO_ATTACKER)
		//	compute damage vector based on direction from attacker to target
		vDamageDir = target->s.origin - ((attacker->absmin + attacker->absmax) * 0.5);
	else if (damage_flags & DAMAGE_DIR_TO_INFLICTOR)
		//	compute damage vector based on direction from attacker to target
		vDamageDir = target->s.origin - ((inflictor->absmin + inflictor->absmax) * 0.5);
	else if (damage_flags & DAMAGE_INFLICTOR_VEL)
		// take on velocity of inflictor
		vDamageDir = inflictor->velocity;
	else if (damage_flags & DAMAGE_TOWARDS_ATTACKER)
		//	compute damage vector based on direction from target to attacker
		vDamageDir = ((attacker->absmin + attacker->absmax) * 0.5) - target->s.origin;
	else
		vDamageDir = damage_vec;

	//	normalize just in case...
	vDamageDir.Normalize ();
	
	//	set global damage vector
	serverState.damage_vector = vDamageDir;

	////////////////////////////////////////////////////////////////////////////////
	//	affect target according to type of damage
	////////////////////////////////////////////////////////////////////////////////
	if( damage_flags )
	{
		if( damage_flags & ( DAMAGE_EXPLOSION | DAMAGE_SIDEWINDER ) )
		{
			if( target->health < 1.0 )
			{
				vDamageDir = vDamageDir *  10 * (rnd() * 80);
				target->velocity.x = vDamageDir.x;
				target->velocity.y = vDamageDir.y;
				target->velocity.z = 100 + (rnd() * 500);
				target->groundEntity = NULL;
				gi.linkentity (target);
				target->avelocity.y = rnd() * 1200 - 600;
			}
			else
			{
				if (damage_flags & DAMAGE_SIDEWINDER && target == attacker && target->flags & (FL_CLIENT + FL_MONSTER + FL_BOT))
				{
					//	adjust for off-center launching weapons
					temp = attacker->s.angles;
					temp.AngleToVectors (forward, up, right);

					serverState.damage_vector = vDamageDir;
					vDamageDir = vDamageDir * damage * 8.0 + (-forward/*right*/ * damage);
				}
				else
				{
					serverState.damage_vector = vDamageDir;
					vDamageDir = vDamageDir * damage * 8.0;
				}

				velocity = target->velocity + vDamageDir;
				target->velocity = velocity;
			}

			target->groundEntity = NULL;
		}
		else if( damage_flags & DAMAGE_SQUISH )
		{
			//	special case... overrides DAMAGE_DIR_TO and DAMAGE_INFLICTOR_VEL flags...
			//	always gets dir from squisher's velocity vector
			temp = target->velocity + inflictor->velocity;
 
			if( temp.z < 0 )
			{
				temp.z = -temp.z;
				if (temp.z > 200)
					temp.z = 200;
			}
			target->velocity = temp;

			temp.Normalize();
			serverState.damage_vector = temp;

			target->groundEntity = NULL;
			gi.linkentity (target);
		}
		else if( damage_flags & DAMAGE_INERTIAL )
		{
			vDamageDir = vDamageDir * damage * 6;
			target->velocity.x += vDamageDir.x;
			target->velocity.y += vDamageDir.y;
			target->velocity.z += vDamageDir.z + damage * 2;

			target->groundEntity = NULL;
			gi.linkentity (target);
		}
		else
		{
			//	Default damage type
			temp = CVector (rnd (), rnd (), rnd ());
			temp.Normalize ();
			serverState.damage_vector = temp;

			target->groundEntity = NULL;
			gi.linkentity (target);
		}
	}
    
	if (damage_flags & DAMAGE_ONLY_TYPES)
	{
		target->groundEntity = NULL;
	}

	return vDamageDir;
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
/*
void com_Damage2( userEntity_t *target, 
				userEntity_t *inflictor, 
				userEntity_t *attacker, 
				CVector &point_of_impact, 
				CVector &damage_vec, 
				float damage, 
				unsigned long damage_flags )
{
	int				calculated_damage, random_damage;
	qboolean		bAttackerIsClient, bInflictorIsClient, bTargetIsClient;
	qboolean		bAttackerHasPlayerHook, bInflictorHasPlayerHook, bTargetHasPlayerHook;
	unsigned int	nAttackerDflags, nInflictorDflags, nTargetDflags;	
	playerHook_t	*pAttackerHook, *pInflictorHook, *pTargetHook;
	qboolean		bIgnoreDamage = false;
	qboolean		bTargetIsNotAttacker;

	// SCG[11/12/99]: No valid info, so early out
    if( ( target == NULL ) || ( inflictor == NULL ) || ( attacker == NULL ) )
	{
		gi.Con_Dprintf("DEBUG: com_Damage, bad parameter\n");
		return;
	}

	// SCG[11/12/99]: The target cannot be damaged, so early out
    if( target->takedamage == 0.0 )
	{
        return;
	}

	// SCG[11/12/99]: Set the damage ( I know we could just use damage, but this is for clarity )
	calculated_damage = damage;

	// SCG[11/12/99]: calculate random damage (originally added by NSS)
	random_damage = damage * ( crand() * 0.1 );

	// SCG[11/12/99]: Set up some necessary flags
	bAttackerIsClient			= attacker->flags & FL_CLIENT;
	bInflictorIsClient			= inflictor->flags & FL_CLIENT;
	bTargetIsClient				= target->flags & FL_CLIENT;

	bAttackerHasPlayerHook		= ( ( attacker->userHook != NULL ) && ( attacker->flags & ( FL_CLIENT | FL_MONSTER | FL_BOT ) ) );
	bInflictorHasPlayerHook		= ( ( inflictor->userHook != NULL ) && ( inflictor->flags & ( FL_CLIENT | FL_MONSTER | FL_BOT ) ) );
	bTargetHasPlayerHook		= ( ( target->userHook != NULL ) && ( target->flags & ( FL_CLIENT | FL_MONSTER | FL_BOT ) ) );

	bTargetIsNotAttacker		= target != attacker;

	serverState.attacker		= attacker;
	serverState.inflictor		= inflictor;
	serverState.damage_flags	= damage_flags;

	// SCG[11/12/99]: Get the hook and deathflags
	if( bAttackerHasPlayerHook )
	{
		pAttackerHook		= ( playerHook_t * ) attacker->userHook;
		nAttackerDflags		= pAttackerHook->dflags;
	}
	if( bInflictorHasPlayerHook )
	{
		pInflictorHook		= ( playerHook_t * ) inflictor->userHook;
		nInflictorDflags	= pInflictorHook->dflags;
	}
	if( bTargetHasPlayerHook )
	{
		pTargetHook			= ( playerHook_t * ) target->userHook;
		nTargetDflags		= pTargetHook->dflags;
	}

	if( com_TeamCheck( attacker, target ) && dm_friendly_fire->value && attacker != target )
	{
		bIgnoreDamage = true;
	}

	// SCG[11/12/99]: Add power boost damage
	if( bAttackerHasPlayerHook && pAttackerHook->power_boost && bTargetIsNotAttacker )
	{
		calculated_damage = ( calculated_damage * ( pAttackerHook->power_boost * 0.3 ) );
		gi.Con_Dprintf( "powerboost! new damage=%f\n", calculated_damage);
	}

	// SCG[11/12/99]: Alert the sidekicks that the player is attacking a monster
	if( bAttackerIsClient && bTargetHasPlayerHook )
	{
		dll_NPCAlert( attacker, target );
	}

	// SCG[11/12/99]: Check for rampage mode
	if( bAttackerHasPlayerHook )
	{
		if( pAttackerHook->dflags & DFL_RAMPAGE )
		{
			calculated_damage *= 3;
		}
	}

	// SCG[11/12/99]: add this to one area that handles no damage
	if ( !bIgnoreDamage && (damage_flags & DAMAGE_FREEZE) &&
	     attacker->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		target->flags |= FL_FREEZE;
	}

	if( bAttackerHasPlayerHook && !( inflictor->flags & FL_IGNORE_ARMOR ) )
	{
        calculated_damage = com_ArmorDamage( target, inflictor, calculated_damage );
    } 

	serverState.damage_inflicted = calculated_damage;

	target->health -= calculated_damage;

	CVector vDamageDir = com_DamageVelocity( target, inflictor, attacker, damage_vec, calculated_damage, damage_flags );

	if( !( damage_flags & DAMAGE_NO_BLOOD ) )
	{
		unsigned int nBloodType;

        if( damage_flags & DAMAGE_SPARKLES )
		{
	        nBloodType = TE_SPARKLES;
		}
		// Logic[5/7/99]: ensure this is not the client
		else if( bAttackerHasPlayerHook && !( target->fragtype & FRAGTYPE_ROBOTIC ) ) 
		{
	        nBloodType = TE_BLOOD;
		}
		else
		{
	        nBloodType = TE_SPARKS;
		}

		com_SpawnBlood( point_of_impact, vDamageDir, calculated_damage, target->health, nBloodType );
	}

	// SCG[11/12/99]: Pain stuff
	if( bTargetIsClient && !bIgnoreDamage )
	{
		target->client->damage_alpha += calculated_damage / 40;
		if (target->client->damage_alpha > 2.0)
			target->client->damage_alpha = 2.0;
		//NSS[10/26/99]:Kick view for pain is now being done in client.cpp in client_pain.
	}
}
*/

// ----------------------------------------------------------------------------
//NSS[11/21/99]:
// Name:		com_check_inventoty
// Description:Check to see if we don't need to apply full damage
// Input:	userEntity_t *target ,  userEntity_t *attacker
// Output:either 1.0 or % of damage to apply because of item in inventory.
// Note:
// ----------------------------------------------------------------------------
// some magic numbers taken from the AIFiles

// lightning
#define	TYPE_WYNDRAX			60
#define	TYPE_KNIGHT2			65
// fire
#define	TYPE_STAVROS			68
#define	TYPE_KNIGHT1			64
#define	TYPE_DRAGON				67
// undead
#define	TYPE_NHARRE				62
#define	TYPE_BUBOID				54
#define	TYPE_LYCANTHIR			58

#define DAMAGE_FRAC				0.25

float com_check_inventory(userEntity_t *target, userEntity_t *attacker)
{
	if (!target || !attacker || (target == attacker))
		return 1.0;

	playerHook_t *phook = (playerHook_t *)target->userHook;
	if (!phook || !(target->flags & (FL_CLIENT|FL_BOT)))
		return 1.0;

	float result = 1.0;

	if (attacker->flags & FL_MONSTER)		// stavros, wyndrax, nharre
	{
		playerHook_t *ahook = (playerHook_t *)attacker->userHook;
		if (!ahook)
			return 1.0;

		switch(ahook->type)
		{
		case TYPE_KNIGHT2:
		case TYPE_WYNDRAX:
			if (phook->items & IT_RINGOFLIGHTNING) result = DAMAGE_FRAC;
			break;
		case TYPE_STAVROS:
		case TYPE_KNIGHT1:
		case TYPE_DRAGON:
			if (phook->items & IT_RINGOFIRE) result = DAMAGE_FRAC;
			break;
		case TYPE_NHARRE:
		case TYPE_BUBOID:
		case TYPE_LYCANTHIR:
			if (phook->items & IT_RINGOFUNDEAD) result = DAMAGE_FRAC;
			break;
		default:
			result = 1.0;
		};
	}
	else if (attacker->flags & (FL_CLIENT|FL_BOT))			// player using zeus, wyndrax, stavros or nightmare
	{
		weapon_t *weapon=(weapon_t *)attacker->curWeapon;
		if (!weapon || (weapon && !weapon->winfo))
			return 1.0;
		
		weaponInfo_t *winfo = weapon->winfo;
		if ((phook->items & IT_RINGOFLIGHTNING) && (!stricmp(winfo->weaponName,"weapon_zeus") || !stricmp(winfo->weaponName,"weapon_wyndrax")))
			result =  DAMAGE_FRAC;
		else if ((phook->items & IT_RINGOFIRE) && !stricmp(winfo->weaponName,"weapon_stavros"))
			result = DAMAGE_FRAC;
		else if ((phook->items & IT_RINGOFUNDEAD) && !stricmp(winfo->weaponName,"weapon_nightmare"))
			result = DAMAGE_FRAC;
		else result = 1.0;
	}

	return result;
}

qboolean com_CanHurt( userEntity_t *attacker, userEntity_t *target );
void com_Damage ( userEntity_t *target, 
				  userEntity_t *inflictor, 
				  userEntity_t *attacker, 
				  CVector &point_of_impact, 
				  CVector &damage_vec, 
				  float damage, 
				  unsigned long damage_flags )
{
	CVector			temp, velocity, dir, vecKickView;
	float			mod_damage;
	playerHook_t	*hook = NULL, *ahook = NULL;
	int				pain_chance;
    qboolean        ignore_damage = false;
	unsigned int	hook_dflags = 0;	// SCG[8/25/99]: 

	if ((target && (target->flags & FL_CINEMATIC)) || (serverState.bCinematicPlaying))
	{
		serverState.attacker			= NULL;
		serverState.inflictor			= NULL;
		serverState.damage_inflicted	= 0;
		return;
	}
    //NSS[10/25/99]:Random damage Addition.
	damage = damage + damage*(crand()*0.10);


	// NSS[2/11/00]:Added a 50% discount for client/sidekick  or sidekick/sidekick damage
	if( ((target->flags & FL_CLIENT) && (attacker->flags & FL_BOT)) || ((attacker->flags & FL_CLIENT) && (target->flags & FL_BOT)) || ((attacker->flags & FL_BOT) && (target->flags & FL_BOT)))
	{
		playerHook_t *hook2 = (playerHook_t *)attacker->userHook;
		playerHook_t *hook1 = (playerHook_t *)target->userHook;
		
		// NSS[2/11/00]:Check for mikiko or superfly.
		if( (hook1->type >= 2 && hook1->type <=3) || (hook2->type >=2 && hook2->type <=3) )
		{
			damage *= 0.50f;
		}		
	}
	
	//NSS[11/21/99]:Check for special items(i.e. ring of fire, ring of undead, etc..)
	if(damage_flags & DAMAGE_CHECK_INVENTORY)
	{
		float ratio = com_check_inventory(target, attacker);
//		if (ratio < 1.0)
//			gi.Con_Dprintf("%f Damage reduced to %f!\n",damage,damage*ratio);
		damage *= ratio;
	}

	// Fix Me: Need to check the parameters, handle as gracefully as possible
    if(!target || !inflictor || ! attacker)
    {
		gi.Con_Dprintf("DEBUG: com_Damage, bad parameter\n");
        return;
    }

    //gi.Con_Dprintf("%s %f base pts from %s's %s\n", target->netname, damage, attacker->netname, inflictor->className);

	if (!(attacker->flags & FL_CLIENT))
	{
		serverState.attackWeap = NULL;
	}

    if(!target->takedamage)
        return;

	// see if this target is oblivious to the world
	if ((serverState.GetCvar("deathtag") > 0.0f) && (deathmatch->value > 0.0f))
	{
		if (!dll_DT_CanDamage(target,attacker,damage_flags))
			return;
	}
	// SCG[8/25/99]: This is kinda fucking dumb.  Not all entities use the player hook
	// SCG[8/25/99]: so any global test condition ( like the one done below ) is FUCKING STUPID!
	// SCG[8/25/99]: DO PEOPLE NOT FUCKING THINK BEFORE THEY TOUCH THE FUCKING KEYBOARD?
	// SCG[8/25/99]: I'll move it to inside the check for FL_CLIENT
	// hook = (playerHook_t *) target->userHook;

    // if team play and friendly fire is not allowed, ignore damage
	// SCG[8/25/99]: We don't want to do this check unless the target is a client (since hook will contain bogus data 
	// SCG[8/25/99]: for anything else )
	// NSS[3/2/00]: Need to put the sidekicks in this same boat.
	if( target->flags & (FL_CLIENT|FL_BOT) )
	{
		hook = (playerHook_t *) target->userHook;
		if( hook )
		{
			hook_dflags = hook->dflags;
		}

		// cek[1-8-00] ignore damage if on same team and friendly fire (hurt teammates) is not on
//		if ( ( com_TeamCheck( attacker, target ) && !dm_friendly_fire->value && attacker != target ) || 
//             ( hook_dflags & DFL_LLAMA ) )
		if ( !com_CanHurt(attacker,target) || 
             ( hook_dflags & DFL_LLAMA ) )
		{
			ignore_damage = true;
			// SCG[10/1/99]: If were not doing dmage, how about we just get the fuck outta here?
//			return;
		}

		if ( !ignore_damage && (damage_flags & (DAMAGE_SLIME|DAMAGE_POISON)) && (serverState.episode == 2) && 
			 (target->client->pers.fAntidotePoints > 0.0f) && (hook->items & IT_ANTIDOTE) )
		{
			userInventory_t *antidote = serverState.InventoryFindItem(target->inventory,"item_antidote");
			if (!antidote)
			{
				hook->items &= ~IT_ANTIDOTE;
				target->client->pers.fAntidotePoints = 0.0f;
			}
			else
			{
				serverState.StartEntitySound(target,CHAN_AUTO,serverState.SoundIndex("artifacts/antidoteuse.wav"),1.0,ATTN_NORM_MIN,ATTN_NORM_MAX);
				ignore_damage = true;
				target->client->pers.fAntidotePoints -= damage;
				if (target->client->pers.fAntidotePoints <= 0.0f)
				{
					//serverState.centerprint(target,2.0,tongue[T_ANTIDOTE_EMPTY]);
					serverState.cs.BeginSendString();
					serverState.cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue, T_ANTIDOTE_EMPTY),0);
					serverState.cs.Unicast(target,CS_print_center,2.0);
					serverState.cs.EndSendString();
					hook->items &= ~IT_ANTIDOTE;
					target->client->pers.fAntidotePoints = 0.0f;
					serverState.InventoryDeleteItem(target,target->inventory,antidote);
				}
			}
		}
	}

    // if (!stricmp(attacker->className, "player"))
    if(attacker->flags & FL_CLIENT)
	{
		ahook = (playerHook_t *) attacker->userHook;
	}
	else
	{
		// do we have an owner?
		if (attacker->owner) 
			ahook = (playerHook_t *) attacker->owner->userHook;
	}
	
	// amw:check to make sure ahook is valid before using 2.22.99
	if(ahook && ahook->power_boost && target != attacker)
	{
		damage += damage * (ahook->power_boost * 0.3);
//		gi.Con_Dprintf("powerboost! new damage=%f\n",damage);
	}

	serverState.attacker = attacker;
	serverState.inflictor = inflictor;
	serverState.damage_flags = damage_flags;

	////////////////////////////////////////////////////////////////////////////////
	//	if this is a client, then alert any following NPCs that he has attacked
	//	something
	////////////////////////////////////////////////////////////////////////////////

	if (attacker->flags & FL_CLIENT && target->flags & (FL_CLIENT | FL_BOT | FL_MONSTER))
	{
		dll_NPCAlert (attacker, target);
	}

	////////////////////////////////////////////////////////////////////////////////
	//	clients, monsters and bots can have * 3 damage if rampage flag is set
	////////////////////////////////////////////////////////////////////////////////

	if (attacker->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
	{
		if (ahook && ahook->dflags & DFL_RAMPAGE)
			damage = damage * 3;
	}

	// check for instagib stuff here
	if (deathmatch->value && dm_instagib->value)
	{
		// doesn't apply to shooting yourself and only applies to clients
		if ( (attacker != target) && (attacker->flags & FL_CLIENT) && (target->flags & FL_CLIENT) )
		{
			damage = 2000;	// this otta be plenty...no armor...
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//	if a client, bot or monster, compute damage done based on target's armor
	////////////////////////////////////////////////////////////////////////////////
	// cek[11-21-99] Armor doesn't help much againat poisoning
	if (target->flags & ( FL_CLIENT + FL_BOT + FL_MONSTER ) && 
		!( ( inflictor->flags & FL_IGNORE_ARMOR ) || ( damage_flags & DAMAGE_IGNORE_ARMOR ) ) && 
		!( damage_flags & DAMAGE_POISON ) )
	{
        mod_damage = com_ArmorDamage(target, inflictor, damage);
    } 
    else if (hook && target->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) && hook->items & IT_CHROMATIC) 
    {
        if(inflictor->EntityType == PROJECTILE_ION)
        {
            mod_damage = damage * 0.1;
            target->armor_val = target->armor_val - (damage * 0.9);
	        if (target->armor_val <= 0)
	        {
		        target->armor_val = 0;
                target->armor_abs = 0;
		        // get rid of our armor bit
                if( hook )
		            hook->items &= ~ALL_ARMOR_MASK;
	        }
        }    
    }
    else 
	{
		mod_damage = damage;
	}

	// check for invulnerability
	if (!ignore_damage && hook && (hook->items & IT_MANASKULL) && (hook->invulnerability_time > 0))
		ignore_damage = TRUE;

	// SCG[8/25/99]:  The following code replaces a bunch of duplicate shit up top.
	serverState.damage_inflicted = !ignore_damage ? mod_damage : 0;

	if( target->flags & FL_BOT )
	{
		if ( allow_friendly_fire->value == 0.0f && ( attacker != target ) && com_CheckFriends( attacker, target ) )
		{
			ignore_damage = TRUE;

			// SCG[10/1/99]: If were not doing dmage, how about we just get the fuck outta here?
			return;
		}
	}

    if( !ignore_damage )
	{
/*		if( hook )
		{
			if ( hook->invulnerability_time < 0 )
			{
				target->health = target->health - mod_damage;
			}
		}
		else*/
		{
			target->health = target->health - mod_damage;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//	if target is not pushable, then do DAMAGE_NONE so target can't possibly move
	////////////////////////////////////////////////////////////////////////////////
	
	if (!(target->flags & FL_PUSHABLE) || (damage_flags & DAMAGE_NO_VELOCITY))
	{
		damage_flags &= DAMAGE_ONLY_FLAGS;
		damage_flags |= DAMAGE_NONE;
	}

	////////////////////////////////////////////////////////////////////////////////
	//	if damage_freeze is applied and the thing dies, put FL_FREEZ on it.
	////////////////////////////////////////////////////////////////////////////////

	if ( !ignore_damage && (target->health <= 0) && (damage_flags & DAMAGE_FREEZE) &&
	     attacker->flags & (FL_CLIENT | FL_BOT | FL_MONSTER))
	{
		target->flags |= FL_FREEZE;
	}

	if (damage_flags & DAMAGE_DIR_TO_ATTACKER)
		//	compute damage vector based on direction from attacker to target
		dir = target->s.origin - ((attacker->absmin + attacker->absmax) * 0.5);
	else if (damage_flags & DAMAGE_DIR_TO_INFLICTOR)
		//	compute damage vector based on direction from attacker to target
		dir = target->s.origin - ((inflictor->absmin + inflictor->absmax) * 0.5);
	else if (damage_flags & DAMAGE_INFLICTOR_VEL)
		// take on velocity of inflictor
		dir = inflictor->velocity;
	else if (damage_flags & DAMAGE_TOWARDS_ATTACKER)
		//	compute damage vector based on direction from target to attacker
		dir = ((attacker->absmin + attacker->absmax) * 0.5) - target->s.origin;
	else
		dir = damage_vec;

	//	normalize just in case...
	dir.Normalize ();
	//	set global damage vector
	serverState.damage_vector = dir;

	////////////////////////////////////////////////////////////////////////////////
	//	affect target according to type of damage
	////////////////////////////////////////////////////////////////////////////////
	if (damage_flags)
	{
		if (damage_flags & (DAMAGE_EXPLOSION | DAMAGE_SIDEWINDER))
		{
			if (target->health < 1.0)
			{
				dir = dir *  10 * (rnd() * 80);
				target->velocity.x = dir.x;
				target->velocity.y = dir.y;
				target->velocity.z = 100 + (rnd() * 500);
				target->groundEntity = NULL;
				gi.linkentity (target);
				target->avelocity.y = rnd() * 1200 - 600;
			}
			else
			{
				if (damage_flags & DAMAGE_SIDEWINDER && target == attacker && target->flags & (FL_CLIENT + FL_MONSTER + FL_BOT))
				{
					//	adjust for off-center launching weapons
					temp = attacker->s.angles;
					temp.AngleToVectors (forward, up, right);

					serverState.damage_vector = dir;
					dir = dir * damage * 13.0 + (-forward/*right*/ * damage);
				}
				else
				{
					serverState.damage_vector = dir;
					dir = dir * damage * 8.0;
				}

				if (target->mass > 100)
					dir.Multiply(200.0 / target->mass);		// 300 / mass

				velocity = target->velocity + dir;
				target->velocity = velocity;
			}

			target->groundEntity = NULL;
		}
		else if (damage_flags & DAMAGE_SQUISH)
		{
			//	special case... overrides DAMAGE_DIR_TO and DAMAGE_INFLICTOR_VEL flags...
			//	always gets dir from squisher's velocity vector
			temp = target->velocity + inflictor->velocity;

			if (temp.z < 0)
			{
				temp.z = -temp.z;
				if (temp.z > 200)
					temp.z = 200;
			}
			target->velocity = temp;

			temp.Normalize();
			serverState.damage_vector = temp;

			target->groundEntity = NULL;
			gi.linkentity (target);
		}
		else if (damage_flags & DAMAGE_INERTIAL)
		{
			dir = dir * damage * 1.75;
			dir.z += damage * 2;
			if (target->mass > 100)
				dir.Multiply(100.0 / target->mass);		// 300 / mass

			target->velocity.x += dir.x;
			target->velocity.y += dir.y;
			target->velocity.z += dir.z;

			target->groundEntity = NULL;
			gi.linkentity (target);
		}
		else
		{
			//	Default damage type
			temp = CVector (rnd (), rnd (), rnd ());
			temp.Normalize ();
			serverState.damage_vector = temp;

			//target->groundEntity = NULL;
			gi.linkentity (target);
		}
	}
    
	if (damage_flags & DAMAGE_ONLY_TYPES)
	{
		target->groundEntity = NULL;
	}

	//	FIXME:	project damage vector into model and find impact tri
	//	FIXME:	tie blood to actual damaged tri
	//	FIXME:	spawn correct blood type based on entity type
	if (!(damage_flags & DAMAGE_NO_BLOOD))
	{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
		com_SpawnBlood (point_of_impact, dir, damage, target->health, TE_SPARKLES);
#else
        if (damage_flags & DAMAGE_SPARKLES)
	    	//	make sparkles instead of blood or sparks
	        com_SpawnBlood (point_of_impact, dir, damage, target->health, TE_SPARKLES);
		else if (target->flags & (FL_BOT | FL_MONSTER | FL_CLIENT) && !(target->fragtype & (FRAGTYPE_ROBOTIC|FRAGTYPE_BONE)) ) // Logic[5/7/99]: ensure this is not the client
			com_SpawnBlood (point_of_impact, dir, damage, target->health, TE_BLOOD);
		else
			com_SpawnBlood (point_of_impact, dir, damage, target->health, TE_SPARKS);
#endif
	}

	///////////////////////////////////////////////////////////////
	//	target killed
	///////////////////////////////////////////////////////////////

	if ((target->health < 1.0) && (target->deadflag != DEAD_DEAD))  //&&& AMW 7.17.98 - added the dead flag
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
    //else if (target->deadflag != DEAD_DEAD)
    {
		//	always do damage flash for clients
//		if (target->flags & FL_CLIENT && !(hook_dflags & DFL_LLAMA))
		if (target->flags & FL_CLIENT && !ignore_damage)
		{
			if (damage_flags & (DAMAGE_COLD|DAMAGE_POISON))
			{
				target->client->damage_alpha = 0;
			}
			else
			{
				target->client->damage_alpha += mod_damage / 40;
				if (target->client->damage_alpha > 2.0)
					target->client->damage_alpha = 2.0;
			}
			//NSS[10/26/99]:Kick view for pain is now being done in client.cpp in client_pain.
		}
		// NSS[3/2/00]:Had to add this last part for manaskull and sidekicks
		if (target->pain && !ignore_damage)
		{
            serverState.damage_vector = dir;
			if (damage > 2)
            {
                if (!(damage_flags & DAMAGE_PAINOVERRIDE))                       // 3.8 dsn
				  target->pain (target, attacker, 0, damage);
            }
			else if( hook )
			{
				if (target->flags & (FL_CLIENT | FL_BOT | FL_MONSTER) && hook->items & IT_POISON)
				{
					//	force pain to happen 25% of the time if poisoned
					pain_chance = hook->pain_chance;
					hook->pain_chance = 25;

					if (!(damage_flags & DAMAGE_PAINOVERRIDE))                     // 3.8 dsn
						target->pain (target, attacker, 0, damage);

					hook->pain_chance = pain_chance;
				}
				else
				{
					if (!(damage_flags & DAMAGE_PAINOVERRIDE))                       // 3.8 dsn
					  target->pain (target, attacker, 0, damage);
				}
			}
			else
			{
                if (!(damage_flags & DAMAGE_PAINOVERRIDE))                       // 3.8 dsn
				  target->pain (target, attacker, 0, damage);
			}
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
	CVector			org, end, targ_org (target->s.origin);
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
		end = (target->absmin + target->absmax) * 0.5;
		
		tr = gi.TraceLine(inflictor->s.origin, end, inflictor, MASK_SHOT);

		if (tr.fraction == 1 || tr.ent == target)
			temp = TRUE;
	}
	else
	{
		tr = gi.TraceLine (inflictor->s.origin, target->s.origin, inflictor, MASK_SHOT);

		if (tr.fraction == 1.0 || tr.ent == target)
			temp = TRUE;
	}

	return temp;
}

////////////////////////////////////////
//	com_RadiusDamage
////////////////////////////////////////

/*
int	com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, unsigned long damage_flags, float fRadiusMultiplier)
{
	userEntity_t	*head;
	float			damage_points;
	CVector			org, distance;
	int				hit_something;
	float			dist;

	hit_something = FALSE;
	head = P_FirstEntity ();

	damage_flags &= ~(DAMAGE_DIR_TO_ATTACKER | DAMAGE_INFLICTOR_VEL);
	damage_flags |= DAMAGE_DIR_TO_INFLICTOR;

	for (head = P_FirstEntity (); head != NULL; head = P_NextEntity (head))
	{
		// find the distance to this entity
		if (head != ignore && head->takedamage)
		{
			if (head->solid == SOLID_BSP)
			{
				org = head->s.maxs + head->s.mins;
				VectorMA( head->s.origin, org, 0.5, org);
			}
			else
			{
				org = head->s.origin;
			}
            distance = org-inflictor->s.origin;
			dist = distance.Length();
			if (dist > damage * fRadiusMultiplier)
			{
				//	cannot hit if more than damage units away
				continue;
			}


            damage_points = damage - (dist * 0.25);
            
			if (damage_flags & DAMAGE_RADIUS2)
				damage_points /= 2;
			if (damage_flags & DAMAGE_RADIUS4)
				damage_points /= 4;
			if (damage_flags & DAMAGE_RADIUS8)
				damage_points /= 8;

			// do half damage on the owner (for rocket jumping)
			if (head == inflictor->owner)
				damage_points = damage_points * 0.5;;

            // Logic [8/5/99]: um, we've established above that the entity will take damage,
            // this is silly (CanDamage makes string compares and excludes some entities, why?
			if (damage_points > 0) // && com_CanDamage (head, inflictor))
			{
				hit_something = TRUE;
				//	pass it zero_vector as dir because DAMAGE_DIR_TO_INFLICTOR flag will override dir anyway
				com_Damage (head, inflictor, attacker, org, zero_vector, damage_points, damage_flags);
			}
		}
	}

	return	hit_something;
}
*/
int	com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, unsigned long damage_flags, float fRadius)
{
	userEntity_t	*head;
	float			damage_points;
	CVector			org, distance, v;
	int				hit_something;
	float			dist;
	trace_t			tr;
	CVector			vMins, vMaxs;

	// cek[2-3-00]: modified to return the number of things hit.
	hit_something = 0;
	head = P_FirstEntity ();

	damage_flags &= ~(DAMAGE_DIR_TO_ATTACKER | DAMAGE_INFLICTOR_VEL);
	damage_flags |= DAMAGE_DIR_TO_INFLICTOR;

	for (head = P_FirstEntity (); head != NULL; head = P_NextEntity (head))
	{
		// find the distance to this entity
		if (head != ignore && head->takedamage)
		{
			if (head->solid == SOLID_BSP)
			{
				org = head->s.mins + head->s.maxs;
				VectorMA( head->s.origin, org, 0.5, org);
			}
			else
			{
				org = head->s.origin;
			}
            distance = org-inflictor->s.origin;
			dist = distance.Length();

			if (dist > fRadius)
			{
				//	cannot hit if more than damage units away
				continue;
			}

			tr = gi.TraceLine( inflictor->s.origin, org, NULL, MASK_SOLID );
			if( tr.fraction != 1.0 )
			{
				if( tr.ent != NULL )
				{
					if( strcmp( tr.ent->className, "worldspawn" ) == 0 )
					{
						continue;
					}
					if( strcmp( head->className , tr.ent->className ) )
					{
						continue;
					}

				}
				else
				{
					continue;
				}
			}

			damage_points = damage * (1.0 - ((dist*dist)/(fRadius*fRadius)));
			// What the hell is this code suposed to do?

			// SCG[11/29/99]: In simple terms, this gets the length of the vector between head and inflictor and stores it in v
			// SCG[11/29/99]: It gets the length of v, multiplies it by 0.5 ( divide by 2 ) and subtracts that value from the original
			// SCG[11/29/99]: damage.  The end result it the farther away you are, the less it hurts, thus radius damage.
			// SCG[11/29/99]: In reality, this function did not need to be changed.

			// cek[11-26-99] um, it WORKS
//			v = head->s.mins + head->s.maxs;
//			VectorMA( head->s.origin, v, 0.5, v);
//			v.Subtract( inflictor->s.origin, v );
//			damage_points = damage - 0.5 * v.Length();

// cek[11-26-99] good crunch, bad aim...need to make sure dist is actually correct.
//			float factor = 1.0f - (dist / fRadius);
//			damage_points = damage * factor;

			// do half damage on the owner (for rocket jumping)
			if (head == inflictor->owner)
			{
				damage_points *= 0.5;

				// cek[11-29-99] do even less if it's a sidewinder
				if (damage_flags & DAMAGE_SIDEWINDER)
				{
					float frac = 2.0 * (1.0 - ((2.0*damage_points) / damage));
					CVector dir = head->s.origin - ((inflictor->absmin + inflictor->absmax) * 0.5);
					dir.Normalize();
					head->velocity += dir * (frac * 150);
					
					damage_points *= 0.8;
				}
			}

			if (damage_points > 0)
			{
				hit_something++;
				//	pass it zero_vector as dir because DAMAGE_DIR_TO_INFLICTOR flag will override dir anyway
				com_Damage (head, inflictor, attacker, org, zero_vector, damage_points, damage_flags);
			}
		}
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

void com_GetMinsMaxs (userEntity_t *self, CVector &org, CVector &mins, CVector &maxs)
{
	// get local origin
	org = (self->absmax - self->absmin) * 0.5 + self->absmin;

	// get maxs
	maxs = self->absmax - org;
	mins = self->absmin - org;
}


////////////////////////////////////////
//	com_ValidTouch
//
//	determines if the object was touched by an entity that can
//	activate it.
////////////////////////////////////////

int	com_ValidTouch (userEntity_t *self, userEntity_t *other) 
{
    if (other == NULL) return TRUE;

	if (other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_POSSESSED)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////
//	com_ValidTouch2
//
//	determines if the object was touched by an entity that can
//	activate it - including monsters
////////////////////////////////////////

int	com_ValidTouch2 (userEntity_t *self, userEntity_t *other) 
{
	if (other->flags & (FL_CLIENT | FL_BOT | FL_POSSESSED | FL_MONSTER))
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

	serverState.activator = self->owner;

	temp = self->delay;
	self->delay = 0;

	com_UseTargets (self, self->enemy, self->owner);

	self->delay = temp;

	self->think = NULL;
	self->nextthink = -1;
	
	serverState.RemoveEntity (self);
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

	serverState.activator = self->owner;

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
	
		serverState.RemoveEntity (self);
	}
}


////////////////////////////////////////
//	DelayedUse
////////////////////////////////////////


void	DelayedUse (userEntity_t *self)
{
	//	Q2FIXME: self->enemy always == self...
	//self->enemy->use (self->enemy, self->groundEntity, self->owner);
	//<nss> Always check to see if the thing you are using has a use function!
	// KAHDUH!

	if(*self->enemy->use != 0x00000000)
	{
		self->enemy->use (self->enemy, self->groundEntity, self->owner);
	}

	self->think = NULL;
	self->nextthink = -1;

	serverState.RemoveEntity (self);
}

////////////////////////////////////////
//	DelayedKill
////////////////////////////////////////

void	DelayedKill (userEntity_t *self)
{
	serverState.activator = self->owner;

	//	remove activated object
	self->enemy->remove (self->enemy);

	//	remove delay object
	self->remove (self);
}

////////////////////////////////////////
//	SpawnDelay
////////////////////////////////////////

void	SpawnDelay (userEntity_t *self, userEntity_t *other, userEntity_t *activator, think_t think)
{
	userEntity_t	*temp;

	temp = P_SpawnEdict ();
	temp->className = "DelayedUse";
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->owner = activator;
	temp->enemy = self;
	temp->groundEntity = other;

	temp->nextthink = level.time + self->delay;

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
				
				// things that don't move on their own shouldn't impede a parent entity
				if ((cur->movetype == MOVETYPE_NONE) ||	!stricmp(cur->className, "func_dynalight") || !stricmp(cur->className,"projectile_c4"))
					bChildIsActive = FALSE;
			}

			// go to the next child in the list
			cur = cur->teamchain;
		}
		
		// did we find a child that is doing something?
		if (bChildIsActive)
		{
			if (!stricmp (serverState.targeter->className, "DelayedUse"))
			{
				serverState.targeter->nextthink = level.time + 0.2;				
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
				SpawnDelay (serverState.targeter, other, activator, DelayedUseTarget2);		 //	
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
	if (serverState.activator->className)
	{
		if (activator->className == "player" && self->message != "")
		{
			gi.conprintf (self->message);
			// FIXME: play sound
		}
	}
*/
	// kill the killtargets
	if (self->killtarget)
	{
		head = P_FirstEntity ();

		do
		{
			temp = NULL;

			if (head != self && head->targetname)
			{
				if (!stricmp (head->targetname, self->killtarget))
				{
					//	func_wall_explode and func_wall "die" when killtargetted
//					if (!stricmp (head->className, "func_wall_explode") || !stricmp (head->className, "func_wall"))
					if (!stricmp (head->className, "func_wall_explode") )
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
							temp = P_NextEntity (head);
							head->remove (head);
							head = temp;
						}
					}
				}
			}
			
			
			if (!temp) head = P_NextEntity (head);
		} while (head);
	}

	// fire targets, as long as it's not a path_corner_train
	// path_corner_train's targets only point to the next path_corner, so don't use them

	int count = 0;

	if (self->target && !(self->flags & FL_IGNOREUSETARGET))
	{
		head = P_FirstEntity ();

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
							serverState.targeter = self;
							head->use (head, other, activator);
						}
						// increment the use counter
						count++;
						if (count > 8)
						{
							gi.Con_Dprintf("WARNING: %s with %d USE targets at %s\n", head->className, count, com_vtos (head->s.origin));
							gi.Con_Dprintf("Limit is 8\n");
						}
					}
				}
			}

			head = P_NextEntity (head);
		} while (head);
	}
}

////////////////////////////////////////
//	com_Health
//
//	gives health to an entity, capping it at base_health
////////////////////////////////////////

void	com_Health (userEntity_t *self, float give, float max_health)
{
	self->health = self->health + give;
	
	if (self->health > max_health)
		self->health = max_health;
}

////////////////////////////////////////
//	com_AmbientVolume
////////////////////////////////////////

void	com_AmbientVolume (userEntity_t *self, CVector &org, int volume)
{
	//	Q2FIXME:	change to Quake 2 network functions
	/*
	gi.WriteByte (self, MSG_ALL, SVC_AMBIENTVOLUME);
	gi.WriteCoord (self, MSG_ALL, org.x);
	gi.WriteCoord (self, MSG_ALL, org.y);
	gi.WriteCoord (self, MSG_ALL, org.z);
	gi.WriteByte (self, MSG_ALL, volume);
	*/
}

////////////////////////////////////////
//	com_Respawn
////////////////////////////////////////

void	com_Respawn (userEntity_t *self)
{
	if (deathmatch->value || !(deathmatch->value))//FIXME: no respawn unless deathmatch
	{
		self->s.modelindex = gi.modelindex (self->modelName);
	
		self->solid = SOLID_TRIGGER;
		self->nextthink = -1;

		self->s.effects |= EF_RESPAWN;	//	tell client to start respawn effect
		self->s.renderfx |= RF_GLOW;

		gi.linkentity (self);

		//	Q2FIXME:	re-implement ambient volume changes
		// aiment is used to hold the ambient volume between respawns
		if (self->soundAmbientIndex)
			self->s.sound = self->soundAmbientIndex;
	}
}

////////////////////////////////////////
//	com_SpawnDynamicEntity
//NSS[12/2/99]:*****NOTE TO ANYONE CHANGING THIS!!!*****
//When strcpy'ing anything into the key value you MUST make it the size of the originator's key value!
////////////////////////////////////////
userEntity_t	*com_SpawnDynamicEntity (userEntity_t *self, char *szClassName, bool bCopyEpairs)
{

	int indexkey = 0;
	// spawn a monster of class hook->monsterclass
	userEntity_t *ent = NULL;

	ent = serverState.SpawnEntity();									 
	
	// set classname
	ent->className = szClassName;

	// find spawn function for this class
	think_t	func = (think_t)DLL_FindFunction(ent->className);

	if (func == NULL)
	{
		gi.Con_Dprintf ("SpawnDynamicEntity has an undefined className specified at %s\n", com_vtos (self->s.origin));
		self->remove(self);
		return NULL;
	}

	// do we want to copy epairs (typically done for monsters)
	if ( bCopyEpairs && self->epair )
	{
		int nEpairsCount = 0;
		while ( self->epair[nEpairsCount].key )
		{
			nEpairsCount++;
		}

		userEpair_t *epair = (userEpair_t*)gi.X_Malloc(sizeof(userEpair_t)*(nEpairsCount+1), MEM_TAG_COM_EPAIR);	
//		userEpair_t *epair = (userEpair_t *) com_malloc (sizeof (userEpair_t) * (nEpairsCount + 1), MEM_MALLOC);	//	new(MEM_MALLOC) userEpair_t[nEpairsCount+1];
//		memset( epair, 0, (nEpairsCount+1) * sizeof(userEpair_t) );
		
		for(int i = 0; i < nEpairsCount; i++)
		{
			// don't add the 'spawnname' key or create another entity of 'targetname'
			if (stricmp(self->epair[i].key, "spawnflags") != 0 || _stricmp(self->className,"target_monster_spawn")==0)
			{
				int nKeyLength = strlen( self->epair[i].key );
				int nValueLength = strlen( self->epair[i].value );

				epair[indexkey].key = (char*)gi.X_Malloc(sizeof(char)*(nKeyLength+1), MEM_TAG_COM_EPAIR);
				epair[indexkey].value = (char*)gi.X_Malloc(sizeof(char)*(nValueLength+1), MEM_TAG_COM_EPAIR);
//				epair[i].key = (char *) com_malloc (sizeof (char) * (nKeyLength + 1), MEM_MALLOC);	//	new(MEM_MALLOC) char[nKeyLength+1];
//				epair[i].value = (char *) com_malloc (sizeof (char) * (nValueLength + 1), MEM_MALLOC);	//	new(MEM_MALLOC) char[nValueLength+1];

				//NSS[11/16/99]:Hack for monster_spawn bullshit
				if(_stricmp(self->className,"target_monster_spawn")==0)
				{
					if (!stricmp(self->epair[i].key, "monsterclass")) 
					{
						//	everything gets at least a classname epair
						strcpy( epair[indexkey].key, "classname" );
					}		
					else if (!stricmp(self->epair[i].key, "spawnname")) 
					{
						//	everything gets at least a classname epair
						strcpy( epair[indexkey].key, "spawnname" );
					}
					else if (!stricmp(self->epair[i].key, "targetname"))
					{
						// this prevents a NULL from showing up in the epair key list
						strcpy( epair[indexkey].key, "targetname" );
					}
					else if (!stricmp(self->epair[i].key, "muniqueid"))
					{
						// this prevents the unique id loaf bullshit from happening.
						// we need to create a new unique id for this bad boy.
						strcpy( epair[indexkey].key, "uniqueid" );
					}
					else if(!_stricmp("classname",self->epair[i].key))
					{
						continue;
					}
					else
					{
						strcpy( epair[indexkey].key, self->epair[i].key );
					}
				}
				else
				{
					if (!stricmp(self->epair[i].key, "spawnname") || !stricmp(self->epair[i].key, "monsterclass")) 
					{
						//	everything gets at least a classname epair
						strcpy( epair[indexkey].key, "classname" );
					}
					if (!stricmp(self->epair[indexkey].key, "targetname"))
					{
						// this prevents a NULL from showing up in the epair key list
						strcpy( epair[indexkey].key, "unused" );
					}
					else 
					{
						strcpy( epair[indexkey].key, self->epair[i].key );
					}
				}
				// copy the value over
				strcpy( epair[indexkey].value, self->epair[i].value );
				indexkey++;
			}
		}

		ent->epair = epair;
	}
	else
	{
		userEpair_t *epair = (userEpair_t*)gi.X_Malloc(sizeof(userEpair_t)*2, MEM_TAG_COM_EPAIR);
//		userEpair_t *epair = (userEpair_t *) com_malloc (sizeof (userEpair_t) * 2, MEM_MALLOC);	//	new(MEM_MALLOC) userEpair_t[nEpairsCount+1];
//		memset( epair, 0, 2 * sizeof(userEpair_t) );

		int nKeyLength = strlen( "classname" );
		int nValueLength = strlen( szClassName );

		epair[0].key = (char*)gi.X_Malloc(sizeof(char)*(nKeyLength+1), MEM_TAG_COM_EPAIR);
		epair[0].value = (char*)gi.X_Malloc(sizeof(char)*(nValueLength+1), MEM_TAG_COM_EPAIR);
//		epair[0].key = (char *) com_malloc (sizeof (char) * (nKeyLength + 1), MEM_MALLOC);	//	new(MEM_MALLOC) char[nKeyLength+1];
//		epair[0].value = (char *) com_malloc (sizeof (char) * (nValueLength + 1), MEM_MALLOC);	//	new(MEM_MALLOC) char[nValueLength+1];

		//	everything gets at least a classname epair
		strcpy( epair[0].key, "classname" );
		strcpy( epair[0].value, szClassName );

		ent->epair = epair;
	}

	// call spawn function
	func (ent);

	// set the entity's origin to the spawn point
	ent->s.origin = self->s.origin;
	ent->s.old_origin = self->s.old_origin;

	//	if this is a MOVETYPE_TOSS, MOVETYPE_BOUNCE, MOVETYPE_BOUNCEMISSILE or MOVETYPE_FLY
	//	add an epsilon to it so that it won't clip into the world if it was placed directly
	//	on the floor

	if (ent->movetype == MOVETYPE_TOSS || 
		ent->movetype == MOVETYPE_BOUNCE || 
		ent->movetype == MOVETYPE_BOUNCEMISSILE || 
		ent->movetype == MOVETYPE_FLY ||
		ent->movetype == MOVETYPE_FLOAT || 
		ent->movetype == MOVETYPE_WALK ||
		ent->movetype == MOVETYPE_HOP ||
        ent->movetype == MOVETYPE_SWIM )
	{
		ent->s.origin.z += DIST_EPSILON;
	}

	// put this entity into the world
	serverState.LinkEntity(ent);

	return ent;
}

////////////////////////////////////////
//	com_ClientPrint
//
//	sprints if self is a client
////////////////////////////////////////

void	com_ClientPrint (userEntity_t *self, char *msg)
{
	if (self->flags & FL_CLIENT)
		gi.cprintf (self, PRINT_HIGH, msg);
}

////////////////////////////////////////
//	com_SetRespawn
////////////////////////////////////////

void	com_SetRespawn (userEntity_t *self, float time)
{
	self->s.modelindex = 0;	//	set model to 0, modelName should still be valid, though
	self->solid = SOLID_NOT;

	//	shut off looping ambient sounds
	if (self->s.sound > 0)
	{
		self->soundAmbientIndex = self->s.sound;
		self->s.sound = 0;
	}
	else
		self->soundAmbientIndex = 0;

	if (deathmatch->value && time > 0.0f)
	{
		self->nextthink = level.time + time;
		self->think = com_Respawn;
	}
	else
		self->remove (self);
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

	ent = P_FirstEntity ();

	while (ent)
	{
		if (ent->flags & (FL_CLIENT | FL_BOT | FL_MONSTER) && ent->deadflag == DEAD_NO)
		{
			dist = VectorDistance (spawn_point->spot->s.origin, ent->s.origin);

			// if this is the closest ent, save distance
			if (dist < spawn_point->closest)
				spawn_point->closest = dist;

			// if this ent is visible, inc counter
			if (com_Visible (spawn_point->spot, ent))
				spawn_point->ents_visible++;
		}

		ent = P_NextEntity (ent);
	}
}

/////////////////////////////////
//  com_DeathmatchSpawnPoint
//
//  returns the best spawn point
/////////////////////////////////
extern cvar_t *dm_spawn_farthest;
extern cvar_t  *cvarBuildPathTable;

userEntity_t *com_DeathmatchSpawnPoint (const char *className)
{
    // ISP:09-17-99
    if ( cvarBuildPathTable->value > 0.0f )
    {
        return NULL;
    }

	if (!className)
	{
		gi.Con_Printf("com_SelectSpawnPoint: NULL className.\n");
		return	NULL;
	}

	int				count, i, j, r;
	spawn_s			spawn_point [MAX_CLIENTS];  // Logic[4/14/99]: was set for 16, when more than 16 spawn points exists, game bombs
	spawn_s			temp;
	userEntity_t	*ent;
    userEntity_t    *entSpot = NULL;

	// go through entities and get all spawn points
	for (ent = P_FirstEntity (), count = 0; ent; ent = P_NextEntity (ent))
	{
		if (ent->className)
		{
			if (!stricmp (ent->className, className))
			{
				spawn_point [count].spot = ent;
		        spawn_findinfo (className, &spawn_point [count]);
				count++;
                // Logic[4/14/99]: Don't let maps with too many spawns bomb the game, 
                // notify the server that the map won't work with too many spawns.
                if(count > MAX_CLIENTS)
                {
                    gi.Con_Printf("Map Spawnpoints exceed limit of %i\n", MAX_CLIENTS);
                    break;
                }
			}
		}
	}

	// exit if no spawn points found
	if (count == 0)
	{
		return	NULL;
	}

    if(dm_spawn_farthest->value)
    {
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

		// this is the best it's going to get...just return this one.
		entSpot = spawn_point [0].spot;
	}
	else
	{
        j = 0;
        while(j < 2 * count)	// only let it spin for a while
        {
            r = (int)(rnd() * count);
            if(spawn_point[r].closest < 32)
            {
				spawn_point[r].closest += 8;		// nudge it out... we'll just telefrag
            }
            else
            {
                entSpot = spawn_point[r].spot;
                break;
            }

			j++;
        }
	}

	// just pick this one if everything else fails!
	if (!entSpot)
		entSpot = spawn_point [0].spot;

	return entSpot;
}

// this whole thing is stupid.  If there IS a spot, this routine MUST return one.
#if 0
userEntity_t *com_DeathmatchSpawnPoint (const char *className)
{
    // ISP:09-17-99
    if ( cvarBuildPathTable->value > 0.0f )
    {
        return NULL;
    }

	int				count, i, j, r;
	spawn_s			spawn_point [MAX_CLIENTS];  // Logic[4/14/99]: was set for 16, when more than 16 spawn points exists, game bombs
	spawn_s			temp;
	userEntity_t	*ent;
    userEntity_t    *entSpot = NULL;

	if (!className)
	{
		gi.Con_Printf("com_SelectSpawnPoint: NULL className.\n");
		return	NULL;
	}

	// go through entities and get all spawn points
	for (ent = P_FirstEntity (), count = 0; ent; ent = P_NextEntity (ent))
	{
		if (ent->className)
		{
			if (!stricmp (ent->className, className))
			{
				spawn_point [count].spot = ent;
				count++;
                // Logic[4/14/99]: Don't let maps with too many spawns bomb the game, 
                // notify the server that the map won't work with too many spawns.
                if(count > MAX_CLIENTS)
                {
                    gi.Con_Printf("Map Spawnpoints exceed limit of %i\n", MAX_CLIENTS);
                    break;
                }
			}
		}
	}

	// exit if no spawn points found
	if (count == 0)
	{
		//gi.Con_Printf("No %s found in entity list.\n", className);
		//gi.Error ("Check for deathmatch spawnpoints.");

		return	NULL;
	}

	    // find number of visible ents and closest ent for each spawn point
    for (i = 0; i < count; i++)
        spawn_findinfo (className, &spawn_point [i]);

    if(dm_spawn_farthest->value)
    {
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

        // 19990312 JAR - Maybe I am just dumb, but isn't this a bit redundant?
        /*
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
        */
	    // show the list of spawnpoints
    //	for (i = 0; i <= count; i++)
    //		gi.conprintf ("closest = %f, visible = %i\n", spawn_point [i].closest, spawn_point [i].ents_visible);

	    // make sure there is no entity within 32.0 units, or we might get stuck
		// um, let the error handling go on where this is used, not here.  don't want to crash a server just because
		// somebody is too close...that's what telefragging is for
//	    if (spawn_point [0].closest < 32.0)
//		    entSpot = NULL;
//	    else	
	    entSpot = spawn_point [0].spot;
    } 
	else 
	{
		r = (int)(rnd() * count);
		entSpot = spawn_point[r].spot;
/*	this is plain dumb.  same problem as above.  what if r is the same every time?  hmmm?  happens alot when there are only two spots...
        j = 0;
        while(j < count)
        {
            r = (int)(rnd() * count);
            if(spawn_point[r].closest < 32)
            {
                j++;    // retry
            }
            else
            {
                entSpot = spawn_point[r].spot;
                break;
            }
        }
*/
    }
    return entSpot;
}
#endif
//////////////////////////////
//	com_SelectSpawnPoint
//////////////////////////////

userEntity_t *com_SelectSpawnPoint (const char *className, const char *target)
{
	userEntity_t	*head;

	if (target)
	{
// SCG[3/16/00]: 		gi.Con_Printf("spawning at %s\n", target);
		head = P_FirstEntity ();

		while (head)
		{
			if (head->className)
			{
				if (!stricmp (head->className, className))
				{
					if( ( head->targetname == NULL ) && ( target[0] == NULL ) )
						return head;

					if (head->targetname)
					{
						if (!stricmp (head->targetname, target))
						{
//							gi.Con_Printf("found %s\n", head->className);
							return	head;
						}
					}	
				}
			}

			head = P_NextEntity (head);
		}
	}

	// SCG[11/29/99]: Try to find the first instance of "className" with no target
	head = P_FirstEntity ();

	while (head)
	{
		if (head->className)
		{
			if (!stricmp (head->className, className))
			{
				if( head->targetname == NULL )
				{
					return	head;
				}	
			}
		}

		head = P_NextEntity (head);
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
//    float fScale;
	// WAW: The base things need to be taken into account.
	// ALWAYS ADJUST TO THE BASE VALUE.
	if (hook->vita_boost < hook->base_vita)
		hook->vita_boost = hook->base_vita;
	if (hook->speed_boost < hook->base_speed)
		hook->speed_boost = hook->base_speed;
	if (hook->power_boost < hook->base_power)
		hook->power_boost = hook->base_power;
	if (hook->attack_boost < hook->base_attack)
		hook->attack_boost = hook->base_attack;
	if (hook->acro_boost < hook->base_acro)
		hook->acro_boost = hook->base_acro;
    // 19990308 JAR - this shouldn't run... ever.    
/*
	// add in base attributes

	hook->vita_boost += hook->base_vita;
	hook->speed_boost += hook->base_speed;
	hook->power_boost += hook->base_power;
	hook->attack_boost += hook->base_attack;
	hook->acro_boost += hook->base_acro;
*/
    // cap attributes HERE (NO CHEATERS)
    if(hook->vita_boost > ATTR_MAX_LEVELS)
        hook->vita_boost = ATTR_MAX_LEVELS;
    if(hook->speed_boost > ATTR_MAX_LEVELS)
        hook->speed_boost = ATTR_MAX_LEVELS;
    if(hook->power_boost > ATTR_MAX_LEVELS)
        hook->power_boost = ATTR_MAX_LEVELS;
    if(hook->attack_boost > ATTR_MAX_LEVELS)
        hook->attack_boost = ATTR_MAX_LEVELS;
    if(hook->acro_boost > ATTR_MAX_LEVELS)
        hook->acro_boost = ATTR_MAX_LEVELS;

	hook->base_health	= (float) ATTR_HEALTH + (50 * (float) hook->vita_boost);
	self->max_speed		= ATTR_RUNSPEED + (ATTR_RUNSPEED / ATTR_MAX_LEVELS) * (float) hook->speed_boost;

    // Logic[5/18/99]: resize player model based on vitality level
//    fScale = 1 + (hook->vita_boost * 0.10);
    // FIX ME: Somewhere in the code, the bounding box for the player
    // entity is reset to defaults within this frame
    // self->s.mins.Set(-24 * fScale, -24 * fScale, -48 * fScale);    
    // self->s.maxs.Set(24 * fScale, 24 * fScale, 48 * fScale);
//    self->s.alpha = fScale;
    if(hook->vita_boost == ATTR_MAX_LEVELS)
        self->s.effects2 = EF2_RENDER_SCALE;
    gi.linkentity(self);

}




void com_Boost_Icons(userEntity_t *self, byte icon_action, byte icon_type, int delay_time)
// add an onscreen icon (power boosts, )
{
	// send message down to client
	gi.WriteByte(SVC_ONSCREEN_ICON);
	gi.WriteByte(icon_action);
	gi.WriteByte(icon_type);
	gi.WriteShort(delay_time); 
	gi.unicast(self, true); 
}


void com_InformClientDeath(userEntity_t *self, byte gibbed)
// update client of death
{
	// cek[2-5-00] so we can prevent icon/hud drawing
	if (self->flags & FL_CLIENT)
		self->client->ps.rdflags |= RDF_LETTERBOX;
	// send message down to client
	gi.WriteByte(SVC_CLIENTDEATH);
	gi.WriteByte(gibbed);
	gi.unicast(self, true); 
}



void com_Sidekick_Update(userEntity_t *self, byte whom, int status, int val)
// update client of sidekick stats
{
	// send to current user  (may need to send to multiple for co-op)
	//userEntity_t *ent = &g_edicts[0];
  //ent++;  
	if (!gi.TestUnicast(self,true))
		return;

  gi.WriteByte(SVC_SIDEKICK_UPDATE);
  gi.WriteByte(whom);                    // sidekick_superfly, sidekick_mikiko
  gi.WriteShort(status);                 // type of status
  gi.WriteShort(val);                    // value

	gi.unicast(self, true);                 // update 
  //gi.unicast(ent, true);                 // update 

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
				gi.WriteByte (self, MSG_ONE, SVC_AMMOCOUNT);
				gi.WriteByte (self, MSG_ONE, ammo->count);
				*/
			}
		}
	}
}

////////////////////////////////
// com_Showbbox
////////////////////////////////

void	com_Showbbox (userEntity_t *self, CVector &org, CVector &mins, CVector &maxs)
{
	CVector	amin, amax;

	amin = org + mins;
	amax = org + maxs;

	com_SpawnMarker (self, org, "models\\grenade.mdl", 0.05);

	///////////////////////////////////////////////////////////////////////////////////
	// draw the four vertical lines of the bounding box
	///////////////////////////////////////////////////////////////////////////////////
// SCG[1/22/00]: This is silly!
/*							
	com_SpawnMarker (self, CVector (amax.x, amax.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amin.x, amax.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amax.x, amin.y, amax.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amin.x, amin.y, amax.z), "models\\grenade.mdl", 0.05);

	com_SpawnMarker (self, CVector (amax.x, amax.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amin.x, amax.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amax.x, amin.y, amin.z), "models\\grenade.mdl", 0.05);
	com_SpawnMarker (self, CVector (amin.x, amin.y, amin.z), "models\\grenade.mdl", 0.05);
*/
	char szModel[] = "models\\grenade.mdl";
	CVector vTemp;
	vTemp.Set(amax.x, amax.y, amax.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amin.x, amax.y, amax.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amax.x, amin.y, amax.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amin.x, amin.y, amax.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amax.x, amax.y, amin.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amin.x, amax.y, amin.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amax.x, amin.y, amin.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
	vTemp.Set(amin.x, amin.y, amin.z);	com_SpawnMarker (self, vTemp, szModel, 0.05);
}

////////////////////////////////////////
//	com_GetFrames
//
//	queries the model for start and end frames
////////////////////////////////////////

int	com_GetFrames (userEntity_t *self, char *seq_name, int *startFrame, int *endFrame)
{
    _ASSERTE( seq_name );
    if ( strlen( seq_name ) == 0 )
    {
        return FALSE;
    }

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
		gi.dk_GetFrameName (self->s.modelindex, i, frameName);
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

		_ASSERTE( seq_name );
        _ASSERTE( strlen( seq_name ) > 0 );

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


//////////////////////////////////////////////////
//	com_GetFrameData
//
//	queries the model for all animation sequence data
//
//	when called with pFrameData == NULL, the routine 
//	returns the number of sequences found
//
//	when called again with pFrameData pointing to 
//  user allocated memory, this routine will fill
//  in the sequence name and first and last frame
//  values of the frameData_t record
//
//////////////////////////////////////////////////

int com_GetFrameData (userEntity_t *self, frameData_t *pFrameData)
{
	int modelIndex = self->s.modelindex;
	int nNumSequences = 0;
	animSeq_t *pSeq = NULL;

	// if NULL, we are only requesting how many animation
	// sequences this model contains
	if (!pFrameData)
	{
		nNumSequences = gi.dk_GetAnimSequences(modelIndex, NULL);
	}
	else
	{
		nNumSequences = gi.dk_GetAnimSequences(modelIndex, pFrameData);
	}

#ifdef _DEBUG
	bool bOld_Format_Model = FALSE;

	// check for old model format
	// most likely nNumSequences will be whacked out at this point if this is an old model
	if (nNumSequences < 0 || nNumSequences > 256)
	{
		bOld_Format_Model = TRUE;
		_ASSERTE(!bOld_Format_Model);
		nNumSequences = 0;
	}
#endif

	return (nNumSequences);
}

char *com_ProperNameOf(userEntity_t *self)
{
	if (self->flags & FL_CLIENT)
	{
		char *result = self->netname;
		if (!deathmatch->value)
		{
			playerHook_t *hook = (playerHook_t *)self->userHook;
			if (!hook)
				return tongue_world[T_HIRO_MIYAMOTO];

			switch(hook->iPlayerClass)
			{
			default:
			case PLAYERCLASS_HIRO:
				result = tongue_world[T_HIRO_MIYAMOTO];
				break;
			case PLAYERCLASS_MIKIKO:
				result = tongue_world[T_MIKIKO_IBIHARA];
				break;
			case PLAYERCLASS_SUPERFLY:
				result = tongue_world[T_SUPERFLY_JOHNSON];
				break;
			}
		}

		return result;
	}
	else
		return self->netname;
}
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
	CVector			org, start, end;
	CVector			max, min, temp;

	head = P_FirstEntity ();

	org = (self->s.mins + self->s.maxs) * 0.5;
	min = self->s.mins;
	max = self->s.maxs;

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
			temp = head->s.origin + head->view_ofs;

			tr = gi.trace (min, NULL, NULL, temp, self, MASK_OPAQUE);
			if (trace.fraction == 1.0)
				return	head;

			tr = gi.trace (min, NULL, NULL, temp, self, MASK_OPAQUE);
			if (trace.fraction == 1.0)
				return	head;
*/
		}

		head = P_NextEntity (head);
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
	gi.Error (msg);
#else
	gi.Con_Printf("%s", msg);
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

	gi.Error (msg);
}

//////////////////////////////
//	com_SetClientModels
//
//	amw 4.29.99 - not used anymore...
//
//////////////////////////////

/*
void	com_SetClientModels (userEntity_t *self, userEntity_t *head, userEntity_t *torso, userEntity_t *legs, userEntity_t *weapon)
{
	gi.WriteByte (SVC_SETCLIENTMODELS);
	gi.WriteShort (head->s.number);
	gi.WriteShort (torso->s.number);
	gi.WriteShort (legs->s.number);
	gi.WriteShort (weapon->s.number);
	gi.unicast (self, true);
}
*/


/*
///////////////////////////////////////////////
//	com_AngleDiff
//
//	returns the difference between passed angles
///////////////////////////////////////////////

float	com_AngleDiff (float ang1, float ang2)
{
	float	diff;

	ang1 = AngleMod (ang1);
	ang2 = AngleMod (ang2);

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

void	com_Normal (CVector &v1, CVector &v2, CVector &v3, CVector &out)
{
	CVector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	CrossProduct( a, b, out );
}
*/

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
			hook->poison_time = time;
			hook->poison_next_damage_time = interval;
			hook->poison_interval = interval;
		}
		if(hook->poison_damage <= 0.0f)
		{			
			hook->poison_damage = damage;
		}
		else
			
		//NSS[10/24/99]:commented this shit out.
        //if(hook->poison_interval && interval < hook->poison_interval)
		
		//NSS[10/24/99]:If the new poison has a smaller frequency then it will apply it
		//only if the new damage is greater than the old damage
		//This is NOT realistic, however it will prevent poison from being stupidly high
		//when getting bitten by the same creature or creature with like poison repetitively
		if(hook->poison_damage && hook->poison_interval)
		{
			if(hook->poison_damage < damage && interval < hook->poison_interval)
			{
				hook->poison_damage += damage ; // additive poison damage since this poison type is a greater poison
				hook->poison_interval = interval;
				if(hook->items & IT_POISON) // add more time to the poison if we made it this far
					hook->poison_time += time;
			}
		}
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
	CVector			e_org, s_org, temp;

	for (ent = P_FirstEntity (); ent; ent = P_NextEntity (ent))
	{
		if (ent->takedamage && ent->solid == SOLID_BBOX)
		{
			if (ent != self)
			{
				//	FIXME: check for exact bounding box collision

				//	determine if bounding boxes touch in z plane
				if (fabs (ent->s.origin.z - self->s.origin.z) <= self->s.maxs.z + ent->s.maxs.z)
				{
					//	determine if bounding boxes touch in xy plane

					e_org = ent->s.origin;
					e_org.z = 0;
					s_org = self->s.origin;
					s_org.z = 0;

					temp = e_org - s_org;

					if (temp.Length() <= self->s.maxs.x + ent->s.maxs.x)
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
	CVector	end;
} lineHook_t;

void	line_think (userEntity_t *self)
{
	userEntity_t	*ignore;
	CVector			start;
	CVector			end;
	CVector			point;
	CVector			last_movedir;
	CVector			temp;
	int				count;
	static			CVector	lmins(-4, -4, -4);
	static			CVector	lmaxs(4, 4, 4);
	lineHook_t		*hook = (lineHook_t *) self->userHook;

	// update the frame
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
	
	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		last_movedir = self->movedir;
		point = self->enemy->absmin + 0.5 * self->enemy->size;
		temp = point - self->s.origin;
		temp.Normalize ();
		self->movedir = temp;

		if (temp != last_movedir)
			self->spawnflags |= 0x80000000;
	}
	else
	{
		temp = self->movedir;
	}

	ignore = self;

	start = self->s.origin;
//	end = start + temp * 64;
	self->s.old_origin = hook->end;

	if (level.time > self->delay)
		self->remove (self);
	else
		self->nextthink = level.time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	com_DrawLine (userEntity_t *self, CVector &org, CVector &end, float lifetime)
{
	userEntity_t	*laser;
	CVector			temp;
	lineHook_t		*hook;

	laser = P_SpawnEdict ();

	laser->className = "LaserLine";
	laser->owner = self;
	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM | RF_TRANSLUCENT ;
	laser->s.alpha = 0.5;
	laser->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	laser->s.frame = 3;
	laser->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	// set the color

	laser->s.mins.Set(-8, -8, -8);
	laser->s.maxs.Set(8, 8, 8);

	//	set the starting position
	laser->s.origin = org;
	gi.linkentity (laser);

	//	find the direction to target
	temp = end - org;
	temp.Normalize ();
	laser->movedir = temp;

	laser->spawnflags |= 0x80000001;
	laser->svflags -= (laser->svflags & SVF_NOCLIENT);

	//	set the ending position
	laser->s.old_origin = end;

	laser->userHook = gi.X_Malloc(sizeof (lineHook_t), MEM_TAG_HOOK);
	hook = (lineHook_t *) laser->userHook;
	hook->end = end;

	laser->delay = lifetime;

	laser->think = line_think;
	laser->nextthink = level.time + 0.1;
}


// ----------------------------------------------------------------------------
//
// Name:		com_MarkerThink
// Description:Think function for Markers
// Input:userEntity_t *self
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void com_MarkerThink (userEntity_t *self)
{
	if(self->delay < level.time)
		self->remove(self);
	self->nextthink = level.time + 0.01f;
}

// ----------------------------------------------------------------------------
//
// Name:		com_DrawMarker
// Description:Drops a Marker at the CVector Target origin and leaves it for
// 20 seconds
// Input:userEntity_t *self & float delay(lifetime)
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void com_DrawMarker(userEntity_t *self, float delay)
{
	userEntity_t *Flare;
	CVector Offset_Origin,Angles,Forward;

	Flare = P_SpawnEdict ();

	Flare->owner = Flare;
	Flare->solid = SOLID_NOT;
	Flare->movetype = MOVETYPE_NONE;
	Flare->className = "drop_sprite";

	Flare->s.renderfx |= (SPR_ALPHACHANNEL);

	Flare->s.modelindex = serverState.ModelIndex("models/global/e_flred.sp2");
	
	Flare->s.render_scale.Set( .15, .15, .15);
	Flare->delay = level.time + delay;
	Flare->think = com_MarkerThink;

	serverState.SetSize (Flare, -2, -2, -3, 2, 2, 3);
	
	
	//Figure out offset origin stuff	
	Angles = self->s.angles;
	Angles.AngleToForwardVector(Forward);
	Forward.Normalize();
	
	
	Forward = Forward * (((self->s.maxs.x - self->s.mins.x)*.25f)+((self->s.maxs.y - self->s.mins.y)*.25f));
	Forward.z = 0.0f;
	Offset_Origin = Forward + self->s.origin;
	serverState.SetOrigin2 (Flare, Offset_Origin);
	serverState.LinkEntity (Flare);

	Flare->s.frame = 0;
	Flare->nextthink = level.time + 0.01f;

}


/////////////////////////////////////////////////////////////////////
//	com_DrawBoundingBox
//	draw bounding box around an entity
/////////////////////////////////////////////////////////////////////

void	com_DrawBoundingBox (userEntity_t *self, float lifetime)
{
	CVector min = self->absmin, max = self->absmax;
	CVector org, end;

	
	//Draw the Origin of the Entity
	com_DrawMarker(self, 0.02f);

	CVector vOrg, vEnd;
	// draw the top box (4 lines)
	vOrg.Set(min.x, min.y, min.z); vEnd.Set(max.x, min.y, min.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, min.y, min.z); vEnd.Set(max.x, max.y, min.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, max.y, min.z); vEnd.Set(min.x, max.y, min.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(min.x, max.y, min.z); vEnd.Set(min.x, min.y, min.z); com_DrawLine (self, vOrg,  vEnd, lifetime);

	// draw the bottom box (4 lines)
	vOrg.Set(min.x, min.y, max.z); vEnd.Set(max.x, min.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, min.y, max.z); vEnd.Set(max.x, max.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, max.y, max.z); vEnd.Set(min.x, max.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(min.x, max.y, max.z); vEnd.Set(min.x, min.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);

	// draw the Z connecting lines (4 lines)
	vOrg.Set(min.x, min.y, min.z); vEnd.Set(min.x, min.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, min.y, min.z); vEnd.Set(max.x, min.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(max.x, max.y, min.z); vEnd.Set(max.x, max.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);
	vOrg.Set(min.x, max.y, min.z); vEnd.Set(min.x, max.y, max.z); com_DrawLine (self, vOrg,  vEnd, lifetime);

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
   
	// if the last frame is less then the first, then we must want to play in reverse
	if (first > last)
		ent->s.frameInfo.frameInc = -1;
	else
		ent->s.frameInfo.frameInc = 1;

	ent->s.frameInfo.frameTime=frametime;
	ent->s.frameInfo.frameState=FRSTATE_STARTANIM;

	return;
}

///////////////////////////////////////////////////////////////////////////////
//	com_FrameUpdate
//
//	handles frame updates on the server side
//	NOTE:
//			if self->s.frameInfo.frameInc < 0 denotes that the sequence is playing
//				backwards
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
		self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime + serverState.frametime;
		
		if (self->s.frameInfo.next_frameTime > self->s.frameInfo.frameTime)
		{
			temp = floor (self->s.frameInfo.next_frameTime / self->s.frameInfo.frameTime);
			add = (int)(self->s.frameInfo.frameInc * temp);
			new_frame = self->s.frame + add;

//			if (self->s.renderfx & RF_WEAPONMODEL)
//				serverState.Con_Dprintf("FrameUpdate:  next_ft: %f, ft: %f, add: %d, old: %d, new: %d\n",self->s.frameInfo.next_frameTime,self->s.frameInfo.frameTime,add,self->s.frame,new_frame);
			if ( self->s.frameInfo.frameInc > 0 )
			{
				///////////////////////////////////////////////////////////////////////
				//	set/clear FRSTATE_LAST flag if on last frame
				///////////////////////////////////////////////////////////////////////
				if ( new_frame >= self->s.frameInfo.endFrame && self->s.frame < self->s.frameInfo.endFrame )
				{
					self->s.frameInfo.frameState |= FRSTATE_LAST;
				}

				///////////////////////////////////////////////////////////////////////
				//	set FRSTATE_PLAYSOUND flags if correct frame
				///////////////////////////////////////////////////////////////////////

				if ( self->s.frameInfo.sound1Frame >= self->s.frame && self->s.frameInfo.sound1Frame < new_frame )
				{
   					self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND1;
				}
				if ( self->s.frameInfo.sound2Frame >= self->s.frame &&  self->s.frameInfo.sound2Frame < new_frame )
				{
					self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND2;
				}

				// ISP: 11/02/98
				if ( self->s.frameInfo.nAttackFrame1 >= self->s.frame && self->s.frameInfo.nAttackFrame1 < new_frame )
				{
					self->s.frameInfo.frameState |= FRSTATE_PLAYATTACK1;
				}
				if ( self->s.frameInfo.nAttackFrame2 >= self->s.frame && self->s.frameInfo.nAttackFrame2 < new_frame )
				{
					self->s.frameInfo.frameState |= FRSTATE_PLAYATTACK2;
				}

				//	ECTS hack for monsters to fire player weapons
				if( ( self->flags & ( FL_MONSTER | FL_BOT ) ) && ( hook != NULL ) )
				{
					if ((hook->fxFrameFunc) && (self->s.frame <= hook->fxFrameNum) && (new_frame >= hook->fxFrameNum))
					{
						hook->fxFrameFunc(self);
					}
				}

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
						}
						else
						{
							//	looping through entire sequence
							self->s.frame = self->s.frameInfo.startFrame;
						}
					}
				}
			}
			else
			{
				// reverse animation
				if ( new_frame <= self->s.frameInfo.endFrame && self->s.frame > self->s.frameInfo.endFrame )
				{
					self->s.frameInfo.frameState |= FRSTATE_LAST;
				}

				self->s.frame += add;

				// determine if sequence needs to loop back to start
				if (self->s.frame < self->s.frameInfo.endFrame)
				{
					if (self->s.frameInfo.frameFlags & FRAME_ONCE)
					{
						// we've gone through once, so set frames to static (no updates)
						self->s.frameInfo.frameState |= FRSTATE_STOPPED;
						self->s.frame = self->s.frameInfo.endFrame;
					}
					else
					{
						//	looping through entire sequence
						self->s.frame = self->s.frameInfo.startFrame;
					}
				}
			}
						
			self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime - (temp * self->s.frameInfo.frameTime);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	com_BloodSplat
//
//	Places a random blood splat on the specified surface.  Used when monsters
//	die, gibs impact, etc.  
///////////////////////////////////////////////////////////////////////////////

void	com_BloodSplat (userEntity_t *impactedEnt, CVector &impactPos, cplane_t *plane)
{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	return;
#endif
	CVector vZero(0,0,0);
	serverState.WriteByte (SVC_TEMP_ENTITY);
	serverState.WriteByte (TE_BLOODSPLAT);
	serverState.WritePosition (impactPos);
	serverState.WriteShort (plane->planeIndex);

	serverState.WriteShort (impactedEnt->s.modelindex);	//	write model index so that we can stick to
													//	bmodels correctly
	serverState.WriteShort (impactedEnt->s.number);		//	entity number for bmodels

	serverState.MultiCast(vZero,MULTICAST_ALL);
}

#define UNICAST_ENT        127

///////////////////////////////////////////////////////////////////////////////
//
//	com_trackEntity
//
///////////////////////////////////////////////////////////////////////////////
void com_trackEntity(trackInfo_t *tinfo,short broadcasttype)
{
	// SCG[11/11/99]: Asserte for debugging
	_ASSERTE( tinfo->ent );
	// SCG[11/11/99]: handle the error
	if( tinfo->ent == NULL )
	{
		return;
	}

   serverState.WriteByte(SVC_TEMP_ENTITY);                            // evaluated with temp entities
   serverState.WriteByte(TE_TRACK_ENTITY);                            // tracking msg
   // FIXME: why send a long when the max is 4096?
   serverState.WriteLong(tinfo->ent->s.number);                       // track index / id
   serverState.WriteByte(1);                                          // track mode   (1=new/update   2=kill)
   serverState.WriteLong(tinfo->flags);                               // flags
   serverState.WriteLong(tinfo->renderfx);                            // flags

   if (tinfo->flags & TEF_FXFLAGS)
      serverState.WriteLong(tinfo->fxflags);                             // flags

   // FIXME: why send a long when the max is 4096?
   if (tinfo->flags & TEF_SRCINDEX)
      serverState.WriteLong(tinfo->srcent->s.number);                    // src index

   if (tinfo->flags & TEF_FRU)
      serverState.WritePosition(tinfo->fru);                             // fwd, rt, up offsets

   if (tinfo->flags & TEF_DSTINDEX)
      serverState.WriteLong((int)(tinfo->dstent-serverState.g_edicts));   // dst index

   if (tinfo->flags & TEF_DSTPOS)
      serverState.WritePosition(tinfo->dstpos);                       // dst position

   if (tinfo->flags & TEF_LENGTH)
      serverState.WriteFloat(tinfo->length);                          // length

   if (tinfo->flags & TEF_LIGHTCOLOR)
      serverState.WritePosition(tinfo->lightColor);                   // light color

   if (tinfo->flags & TEF_LIGHTSIZE)
      serverState.WriteFloat(tinfo->lightSize);                       // light size

   if (tinfo->flags & TEF_MODELINDEX)
      serverState.WriteLong(tinfo->modelindex);                       // model index

   if (tinfo->flags & TEF_NUMFRAMES)
      serverState.WriteShort(tinfo->numframes);                       // num frames for TEF_ANIMATION

   if (tinfo->flags & TEF_SCALE)
      serverState.WriteFloat(tinfo->scale);                           // scale for TEF_ANIMATION

   if (tinfo->flags & TEF_FRAMETIME)
      serverState.WriteFloat(tinfo->frametime);                       // frametime for TEF_ANIMATION

   if (tinfo->flags & TEF_EXTRAINDEX)
      serverState.WriteLong(tinfo->extra->s.number);                  // extra entity

   if (tinfo->flags & TEF_ALTPOS)                                 // extra position
      serverState.WritePosition(tinfo->altpos);

   if (tinfo->flags & TEF_ALTPOS2)
      serverState.WritePosition(tinfo->altpos2);

   if (tinfo->flags & TEF_ALTANGLE)
   {
   	// SCG[11/1/99]: ReadPos is a short and truncates values
	// SCG[11/1/99]: on the server side.  In order to retain some
	// SCG[11/1/99]: of the precision, we multiply the value by 100 
	// SCG[11/1/99]: on the server side.
	tinfo->altangle.Multiply( 100 );
      serverState.WritePosition(tinfo->altangle);
   }

   if (tinfo->flags & TEF_MODELINDEX2)
		serverState.WriteLong(tinfo->modelindex2);                      // Second model index
   if (tinfo->flags & TEF_SCALE2)
		serverState.WriteFloat(tinfo->scale2);                          // Second scale for Above Model 
   if (tinfo->flags & TEF_HARDPOINT)
		serverState.WriteString(tinfo->HardPoint_Name);					// Specific name for the hardpoint
   if (tinfo->flags & TEF_HARDPOINT2)
		serverState.WriteString(tinfo->HardPoint_Name2);				// Specific name for the hardpoint2 //NSS[11/7/99]:
   if (tinfo->flags & TEF_LONG1)
		serverState.WriteLong(tinfo->Long1);							// Specific name for the hardpoint2 //NSS[11/7/99]:
   if (tinfo->flags & TEF_LONG2)
		serverState.WriteLong(tinfo->Long2);							// Specific name for the hardpoint2 //NSS[11/7/99]:
   
   switch (broadcasttype)
   {
      case UNICAST_ENT:       serverState.UniCast(tinfo->srcent,false);                                      break;
      default:                serverState.MultiCast(tinfo->srcent->s.origin, (multicast_t)broadcasttype);    break;
   }
}

///////////////////////////////////////////////////////////////////////////////
//
//	com_untrackEntity()
//
///////////////////////////////////////////////////////////////////////////////

void com_untrackEntity(userEntity_t *ent, userEntity_t *to, short broadcasttype)
{
   serverState.WriteByte(SVC_TEMP_ENTITY);                         // evaluated with temp entities
   serverState.WriteByte(TE_TRACK_ENTITY);                         // tracking msg
   serverState.WriteLong(ent->s.number);                           // track index / id
   serverState.WriteByte(2);                                       // track mode   (1=new/update   2=kill)

   switch (broadcasttype)
   {
      case UNICAST_ENT:       serverState.UniCast(to,true);                                         break;
      default:                serverState.MultiCast(ent->s.origin, (multicast_t)broadcasttype);     break;
   }
}

///////////////////////////////////////////////////////////////////////////////
// FindRadius(from_entity, origin, radius)
//
// Quake/Quake2 findradius routine
//
// if ent_from is NULL, it will search through the entire entity list
///////////////////////////////////////////////////////////////////////////////
userEntity_t *FindRadius(userEntity_t *ent_from, CVector &origin, float radius)
{
	CVector	eorg;

	if (!ent_from)
		ent_from = g_edicts;
	else
		ent_from++;
	for ( ; ent_from < &g_edicts[globals.num_edicts]; ent_from++)
	{
		if (!ent_from->inuse)
			continue;
		if (ent_from->solid == SOLID_NOT)
			continue;

		eorg = ent_from->absmin + ent_from->absmax;
		eorg.Multiply( 0.5 );
//		eorg += ent_from->s.origin;	[cek 11-6-99] This was giving wacky results
		eorg -= origin;

		if ( eorg.Length() > radius)
			continue;
		return ent_from;
	}

	return NULL;
}

void com_EarthCrack(CVector vecPos, short sPlaneIndex, short sHitModelIndex, int iEntityNumber, float fScale, float fRoll)
{
    gi.WriteByte(SVC_TEMP_ENTITY);
    gi.WriteByte(TE_EARTHCRACK);
    gi.WritePosition(vecPos);
    gi.WriteShort(sPlaneIndex);
    gi.WriteShort(sHitModelIndex);
    gi.WriteShort(iEntityNumber);
    gi.WriteFloat(fScale);
    gi.WriteFloat(fRoll);
    gi.multicast(vecPos, MULTICAST_ALL);
}

int com_transitionType(char *mapFrom, char *mapTo, qboolean loadgame)
{
	int result;
	int		length = strlen(mapTo);
	char	test = mapTo[length - 1];
	if( loadgame )
	{
		result = 3;// loadgame
	}
	else if( ( ( mapTo[1] != mapFrom[1] ) || ( mapFrom[0] == NULL ) ) && ( loadgame == false ) )
	{
		result = 2;// episode transition
	}
	else if( ( test != 'a' ) ||
		( ( test == 'a' ) && ( strncmp( mapTo, mapFrom, length - 1 ) == 0 ) ) )
	{
		result = 1;// level transition
	}
	else
	{
		result = 0;// map transition
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitSub
//
//	set up pointers to functions in com_sub.cpp
///////////////////////////////////////////////////////////////////////////////

void WriteField1 (FILE *f, field_t *field, byte *base);
void WriteField2( FILE *f, field_t *field, byte *base );
void ReadField( FILE *f, field_t *field, byte *base );
void SaveHook( FILE *f, void *pHook, int nSize );
void LoadHook( FILE *f, void *pHook, int nSize );
void P_InventorySave( void **buf, userEntity_t *self, int aryEntries );
void P_InventoryLoad( void **buf, userEntity_t *self, int aryEntries );
void SavePersistantInventory( userEntity_t *pEnt );

void	com_InitSub (void)
{
	//	CVector functions
	com.vtos					= com_vtos;
								
	com.GetMinsMaxs				= com_GetMinsMaxs;
    com.TeamCheck				= com_TeamCheck;
	com.ChangeYaw				= com_ChangeYaw;
	com.ChangePitch				= com_ChangePitch;
	com.ChangeRoll				= com_ChangeRoll;

	com.SetMovedir				= com_SetMovedir;

	//	debugging functions
	com.SpawnMarker				= com_SpawnMarker;
	com.DrawLine				= com_DrawLine;
	com.Showbbox				= com_Showbbox;
	com.DrawBoundingBox			= com_DrawBoundingBox;

	//	miscellaneous functions
	com.FlashClient				= com_FlashClient;
    com.KickView				= com_KickView;
	com.Damage					= com_Damage;
	com.CanDamage				= com_CanDamage;
	com.RadiusDamage			= com_RadiusDamage;
	com.ValidTouch				= com_ValidTouch;
	com.ValidTouch2				= com_ValidTouch2;
	com.UseTargets				= com_UseTargets;
	com.Health					= com_Health;
	com.Respawn					= com_Respawn;
	com.CheckTargetForActiveChildren = com_CheckTargetForActiveChildren;

	com.Visible					= com_Visible;
	com.FindEntity				= com_FindEntity;
	com.FindClosestEntity		= com_FindClosestEntity;
	com.FindClosestEntityPt		= com_FindClosestEntityPt;
	com.FindTarget				= com_FindTarget;
	com.FindNodeTarget			= com_FindNodeTarget;
	com.DeathmatchSpawnPoint	= com_DeathmatchSpawnPoint;
	com.SelectSpawnPoint		= com_SelectSpawnPoint;

	//&&& AMW 6.12.98 - group searching functions
	com.FindFirstGroupMember	= com_FindFirstGroupMember;
	com.FindNextGroupMember		= com_FindNextGroupMember;

	//&&& AMW 6.25.98 - function to calculate transformed offsets from an origin point
	com.TransformChildOffset	= com_TransformChildOffset;
	
	com.SetRespawn				= com_SetRespawn;
	com.CalcBoosts				= com_CalcBoosts;

	com.Boost_Icons				= com_Boost_Icons;
	com.InformClientDeath		= com_InformClientDeath;   // 3.4

	com.Sidekick_Update			= com_Sidekick_Update;     // 5.25

	com.GetFrames				= com_GetFrames;
	com.GetFrameData			= com_GetFrameData;

	com.ClientVisible			= com_ClientVisible;
	com.Warning					= com_Warning;
	com.Error					= com_Error;

	com.Poison					= com_Poison;
	com.TeleFrag				= com_TeleFrag;
	com.GenID					= com_GenID;

	//	Q2FIXME: still needed in common.dll?
	com.ClientPrint				= com_ClientPrint;
	com.UpdateClientAmmo		= com_UpdateClientAmmo;
	com.AmbientVolume			= com_AmbientVolume;
	
	com.FrameUpdate				= com_FrameUpdate;
	com.AnimateEntity			= com_AnimateEntity;
	
	com.RotateBoundingBox		= com_RotateBoundingBox;

	com.FindSpawnFunction		= DLL_FindFunction;
	com.SpawnDynamicEntity		= com_SpawnDynamicEntity;

	com.BloodSplat				= com_BloodSplat;

	com.trackEntity				= com_trackEntity;
	com.untrackEntity			= com_untrackEntity;
	com.FindRadius				= FindRadius;
	com.EarthCrack				= com_EarthCrack;

	com.AI_WriteSaveField1		= WriteField1;
	com.AI_WriteSaveField2		= WriteField2;
	com.AI_ReadField			= ReadField;
	com.AI_SaveHook				= SaveHook;
	com.AI_LoadHook				= LoadHook;
	com.SavePersInventory		= SavePersistantInventory;

	com.ProperNameOf			= com_ProperNameOf;
	com.TransitionType			= com_transitionType;

	com.InventorySave			= P_InventorySave;
	com.InventoryLoad			= P_InventoryLoad;
}

#if 0
/*
////////////////////////////////////////
//	vtos1
//
////////////////////////////////////////

char	*com_vtos1 (CVector &v)
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

float com_VecToYaw (CVector &vec)
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

void	com_VecToAngles (CVector &angles, CVector &vec1)
{
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

*/
#endif 0

