///////////////////////////////////////////////////////////////////////////////
//	p_phys.c
//
//	general physics routines for physics.dll
///////////////////////////////////////////////////////////////////////////////

#include	"p_global.h"
#include	"p_user.h" 

/*
pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	PHYS_MAX_CLIP_PLANES	5
#define	PHYS_STOP_EPSILON		0.1
#define	PHYS_MAX_PUSHES			4

#define	PHYS_BLOCKED_FLOOR		0x01	//	1 = floor
#define	PHYS_BLOCKED_STEP		0x02	//	2 = wall / step
#define	PHYS_BLOCKED_STOPPED	0x04	//	4 = dead stop
#define	PHYS_BLOCKED_STUCK		0x08	//	8 = stuck inside something

//FIXME: P_AddRotationalFriction:  hacked in for E3 demo
#define	p_stopspeed		100
#define p_friction			6
#define p_waterfriction	1

#define	PHYS_STEPSIZE    18


///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct
{
	trace_t	trace;
} float_pushed_t;

typedef struct
{
	edict_t	*ent;
	CVector	origin;
	CVector	angles;
	float	deltayaw;
} pushed_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

pushed_t		pushed[MAX_EDICTS], *pushed_p;
edict_t			*obstacle;
static	int		p_run_think;


void P_GetCurrent( edict_t *ent )
{
	CVector	vOrigin, v;
	vOrigin = ent->s.origin;
	vOrigin.z += (ent->s.mins.z + 1);
	
	unsigned int nContents = gi.pointcontents( vOrigin );

	if( nContents & MASK_CURRENT )
	{
		v.Zero();

		if( nContents & CONTENTS_CURRENT_0 )	v.x += 1;
		if( nContents & CONTENTS_CURRENT_90 )	v.y += 1;
		if( nContents & CONTENTS_CURRENT_180 )	v.x -= 1;
		if( nContents & CONTENTS_CURRENT_270 )	v.y -= 1;
		if( nContents & CONTENTS_CURRENT_UP )	v.z += 1;
		if( nContents & CONTENTS_CURRENT_DOWN )	v.z -= 1;

		VectorMA( ent->velocity, v, 60, ent->velocity );
	}
}
///////////////////////////////////////////////////////////////////////////////
//	P_GetWaterLevel
//
//	gets the current liqud level of an entity
//	0 = not in a loquid
//	1 = feet in a liquid
//	2 = origin (-mins above feet) in a liquid
//	3 = entirely submerged in a liquid
//
//	also checks to see in the entity transitioned from a liquid to air or
//	vice versa, and plays a default sound if there was a transition and
//	SVF_NOSPLASH is not set
///////////////////////////////////////////////////////////////////////////////

void	P_GetWaterLevel (edict_t *ent)
{
	CVector	test_org;
	int		water_type = 0;
	int		last_water_type = ent->watertype;
	int		last_water_level = ent->waterlevel;

	test_org = ent->s.origin;
	ent->watertype = 0;
	ent->waterlevel = 0;
	
	///////////////////////////////////////////////////////////////////////////
	//	check feet first, if they aren't in water, don't bother check the
	//	other points
	///////////////////////////////////////////////////////////////////////////

	test_org.z += (ent->s.mins.z + 1);
	ent->watertype |= (gi.pointcontents (test_org) & MASK_WATER);
	if (ent->watertype != 0)
	{
		//	feet are in water
		ent->waterlevel = 1;

		///////////////////////////////////////////////////////////////////////////
		//	check origin
		///////////////////////////////////////////////////////////////////////////

		test_org.z -= (ent->s.mins.z + 1);
		water_type = (gi.pointcontents (test_org) & MASK_WATER);
		if (water_type != 0)
		{
			//	oriing is in water
			ent->watertype |= water_type;
			ent->waterlevel = 2;

			///////////////////////////////////////////////////////////////////////////
			//	check top
			///////////////////////////////////////////////////////////////////////////

			test_org.z += (ent->s.maxs.z - 1);
			water_type = (gi.pointcontents (test_org) & MASK_WATER);
			if (water_type != 0)
			{
				ent->watertype |= water_type;
				ent->waterlevel = 3;
			}
		}
	}

	//	don't splash if this is first frame for this entity
	//	otherwise, entity can spawn in water and make a splashing noise
	if (ent->svflags & SVF_NEWSPAWN)
	{
		return;
	}

	//	don't splash if SVF_NOSPLASH set or if this is the first physics frame
	if (ent->svflags & SVF_NOSPLASH || level.framenum < 2)
	{
		return;
	}

	if (ent->waterlevel == 0 && last_water_level > 0)
	{
		//	exited water
		// FIXME: missing sound
		//gi.positioned_sound (ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("engine/waterhit.wav"), 1, 1, 0);
	}
	else 
	if (ent->waterlevel > 0 && last_water_level == 0)
	{
		//	entered water
		// FIXME: missing sound
		//gi.positioned_sound (ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("engine/waterhit.wav"), 1, 1, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_CheckWaterTransition
//
//	determine if an object has just crossed a water boundary
///////////////////////////////////////////////////////////////////////////////

void	P_CheckWaterTransition (edict_t *ent, CVector &old_origin)
{
	qboolean	wasinwater, isinwater;

	wasinwater = (ent->watertype & MASK_WATER);
	ent->watertype = gi.pointcontents (ent->s.origin);
	isinwater = ent->watertype & MASK_WATER;

	if (isinwater)
		ent->waterlevel = 1;
	else
		ent->waterlevel = 0;

	//	don't splash if this is first frame for this entity
	//	otherwise, entity can spawn in water and make a splashing noise
	if (ent->svflags & SVF_NEWSPAWN)
		return;

	//	don't splash if SVF_NOSPLASH set or if this is the first physics frame
	if (ent->svflags & SVF_NOSPLASH || level.framenum < 2)
		return;

	if (ent->movetype == MOVETYPE_FLOAT)
	{
		//	MOVETYPE_FLOAT only spashes at high velocity
		if (ent->velocity.Length() > 180)//ent->velocity [2] > 240 || ent->velocity [2] < -240)
		{
			if (!wasinwater && isinwater)
				//	entering water sound
				gi.StartSound(old_origin, g_edicts, CHAN_AUTO, gi.soundindex("hiro/landinwater.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX, 0.0f);
			else if (wasinwater && !isinwater)
				//	exiting water sound
				gi.StartSound(ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("hiro/exitwater.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX, 0.0f);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_CheckGround
///////////////////////////////////////////////////////////////////////////////

void P_CheckGround (edict_t *ent)
{
	CVector		point;
	trace_t		trace;

//	if (ent->movetype == MOVETYPE_SWIM || ent->movetype == MOVETYPE_FLY || ent->movetype == MOVETYPE_HOVER)
//	{
//		ent->groundEntity = NULL;
//		ent->groundSurface = NULL;
//		return;
//	}
//NSS[10/24/99]:Quake2 Check Ground Code inserted here
/////////////////////////////////////////////////////////////////////////////////
	if (ent->velocity.z > 100)
	{
		ent->groundEntity = NULL;
		return;
	}

// if the hull point one-quarter unit down is solid the entity is on ground
	point.x = ent->s.origin.x;
	point.y = ent->s.origin.y;
//	point.z = ent->s.origin.z - 0.25;
	// SCG[2/15/00]: changed for bouncing stuff.
	point.z = ent->s.origin.z - 0.5f;

	trace = gi.TraceBox (ent->s.origin, ent->s.mins, ent->s.maxs, point, ent, MASK_MONSTERSOLID);

	// check steepness
	if ( trace.plane.normal.z < 0.7 && !trace.startsolid)// NSS[2/23/00]: && ent->movetype != MOVETYPE_SWIM)
	{
		ent->groundEntity = NULL;
		return;
	}

	if (!trace.startsolid && !trace.allsolid )
	{
		ent->s.origin = trace.endpos;
		ent->groundEntity = trace.ent;
		ent->groundSurface = trace.surface;
		ent->groundEntity_linkcount = trace.ent->linkcount;
		// NSS[2/16/00]:For Amphibious creatures.... if we are touching the ground but we still want to go up then
		// do not ZERO our Z axis velocity.		
		if(ent->movetype != MOVETYPE_SWIM && ent->movetype != MOVETYPE_FLY && ent->movetype != MOVETYPE_HOVER)
			ent->velocity.z = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_TestEntityPosition
//
///////////////////////////////////////////////////////////////////////////////

edict_t	*P_TestEntityPosition (edict_t *ent)
{
	trace_t	trace;

	if (ent->clipmask)
		trace = gi.TraceBox (ent->s.origin, ent->s.mins, ent->s.maxs, ent->s.origin, ent, ent->clipmask);
	else
		trace = gi.TraceBox (ent->s.origin, ent->s.mins, ent->s.maxs, ent->s.origin, ent, MASK_SOLID);
	
	if (trace.startsolid || trace.allsolid)
		return trace.ent;
		
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	P_CheckVelocity
//
///////////////////////////////////////////////////////////////////////////////

void P_CheckVelocity (edict_t *ent)
{
//
// bound velocity
//
	if ( ent->velocity.x > p_maxvelocity->value )
	{
		ent->velocity.x = p_maxvelocity->value;
	}
	else 
	if ( ent->velocity.x < -p_maxvelocity->value )
	{
		ent->velocity.x = -p_maxvelocity->value;
	}

	if ( ent->velocity.y > p_maxvelocity->value )
	{
		ent->velocity.y = p_maxvelocity->value;
	}
	else 
	if ( ent->velocity.y < -p_maxvelocity->value )
	{
		ent->velocity.y = -p_maxvelocity->value;
	}

	if ( ent->velocity.z > p_maxvelocity->value )
	{
		ent->velocity.z = p_maxvelocity->value;
	}
	else 
	if ( ent->velocity.z < -p_maxvelocity->value )
	{
		ent->velocity.z = -p_maxvelocity->value;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_RunThink
//	
//	Runs thinking code for this frame if necessary
///////////////////////////////////////////////////////////////////////////////

qboolean P_RunThink (edict_t *ent, int run_think)
{
	float	thinktime;

	//	physics called from p_client, so don't run a think
	if (!run_think)
		return	true;

	thinktime = ent->nextthink;
	
	if (thinktime <= 0 || thinktime > level.time + 0.001)
	{
//		if (ent->flags & (FL_MONSTER + FL_CLIENT + FL_BOT))
//			dll_think_stub (ent);
		return true;
	}
	
	if (thinktime < level.time)
		thinktime = level.time;

	ent->nextthink = 0;

//	if (!ent->think)
//		gi.error ("NULL ent->think");

	if (ent->think)
	{
		serverState.time = thinktime;
		ent->think( ent );
	}
#ifdef _DEBUG
    else
    {
        if ( _stricmp( ent->className, "superfly" ) == 0 ||
             _stricmp( ent->className, "mikiko" ) == 0 )
        {
            _ASSERTE( FALSE );
        }
    }
#endif _DEBUG
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//	P_StaticFriction
//
//	returns the force required to overcome a stopped entity's static friction
//
//	this isn't 100% correct, because true friction depends on both the 
//	ground surface and the object's surface, but this only considers the
//	ground surface.
///////////////////////////////////////////////////////////////////////////////

float	P_StaticFriction (edict_t *ent)
{
	float	force;
	float	friction_coefficient;

	if (!ent->groundEntity || !ent->groundSurface)
		return	0;

	//	get the friction coefficient according to the surface flags
	if (ent->groundSurface->flags & SURF_WOOD)
		friction_coefficient = 0.3;
	else 
	if (ent->groundSurface->flags & SURF_METAL)
		//	metal is really hard to start a push on
		//	but slides easily once it is moving
		friction_coefficient = 0.3;	
	else 
	if (ent->groundSurface->flags & SURF_STONE)
		friction_coefficient = 0.6;
	else 
	if (ent->groundSurface->flags & SURF_GLASS)
		friction_coefficient = 0.3;
	else 
	if (ent->groundSurface->flags & SURF_ICE)
		friction_coefficient = 0.025;
	else 
	if (ent->groundSurface->flags & SURF_SNOW)
		friction_coefficient = 0.4;
	else
		//	all other surface flags are standard friction
		friction_coefficient = 0.3;

	force = p_gravity->value * ent->mass * friction_coefficient;

	return	force;
}

///////////////////////////////////////////////////////////////////////////////
//	P_MomentumPush
//
//	totally inelastic collision
//
//	objects collide and both go off in the same direction,
//	with the same velocity
//
//	since object 1 has to be moving already, we check to
//	see if object 2 is stationary.  If so, we determine 
//	the force necessary to overcome its friction and subtract
//	that from the applied force of the pusher
//
//	pushes entities using velocity vectors
//	different from P_PushEntity which moves
//	the pushed entity instead of changing its
//	velocity
//
///////////////////////////////////////////////////////////////////////////////

void	P_MomentumPush (edict_t *e1, edict_t *e2, CVector &push_velocity)
{
	CVector	new_dir, impact_vel, end;
	float	momentum1, momentum2, vf, dp2;
	trace_t	trace;

	if (e1->movetype == MOVETYPE_PUSH || e1->solid == SOLID_BSP ||
		e2->movetype == MOVETYPE_PUSH || e2->solid == SOLID_BSP ||
		!(e2->flags & FL_PUSHABLE) || (e1->flags & FL_NOPUSH))
	{
		return;
	}

	if ( push_velocity.Length() == 0 )
	{
		impact_vel = e1->velocity;
	}
	else
	{
		impact_vel = push_velocity;
	}

	//	if standing on the pushed object, don't make it slide around in x/y
	if (e1->groundEntity == e2)
	{
		impact_vel.x = impact_vel.y = 0;
	}

	// find the momentum of each object
	momentum1 = impact_vel.Length() * e1->mass;
	momentum2 = e2->velocity.Length() * e2->mass;

	//	if the impacted object is not moving, find the force
	//	needed to overcome it's friction
	if ( e2->velocity.Length() == 0.0 || e2->groundEntity && !(e2->svflags & SVF_PUSHED))
	{
		// FIXME: once we have texture detection, base friction
		// coeffecient on the current surface
		// assume friction of approximately steel on steel
		dp2 = P_StaticFriction (e2);
//		dp2 = 800 * e2->mass * 0.3;
	// otherwise, give it no additional friction
	}
	else
	{
		dp2 = 800 * e2->mass * P_StaticFriction (e2) * 0.3;
//		dp2 = 0;
	}

	//	fudge momentum a bit...
//	dp2 *= 0.5;

	momentum1 = momentum1 - dp2;
	if (momentum1 < 0) 
	{
		momentum1 = 0;
	}

	//	add the two velocities to get the resulting direction
	new_dir = impact_vel + e2->velocity;
	new_dir.Normalize();

	//	calculate final velocities using Law of Conservation of Momentum
	vf = (momentum1 + momentum2) / (e1->mass + e2->mass);

	if (vf == 0)
	{
		return;
	}

	// scale the resultant vector by the final velocity
	e2->velocity = new_dir * vf;

	e2->groundEntity = NULL;	
	e2->groundSurface = NULL;
	e2->svflags |= SVF_PUSHED;

	//	move the pushed entity up one unit
	end = e2->s.origin;
	end.z += 1;
	if (e2->clipmask)
	{
		trace = gi.TraceBox (e2->s.origin, e2->s.mins, e2->s.maxs, end, e2, e2->clipmask);
	}
	else
	{
		trace = gi.TraceBox (e2->s.origin, e2->s.mins, e2->s.maxs, end, e2, MASK_SOLID);
	}

	e2->s.origin = trace.endpos;
}

///////////////////////////////////////////////////////////////////////////////
//	P_Impact
//	
//	Two entities have touched, so call P_MomentumPush to compute
//	velocities and then run their touch functions
///////////////////////////////////////////////////////////////////////////////

void P_Impact (edict_t *e1, trace_t *trace)
{
	edict_t *e2 = trace->ent;

	if (e1->touch && e1->solid != SOLID_NOT)
	{
		e1->touch (e1, e2, &trace->plane, trace->surface);
	}

	if (e2->touch && e2->solid != SOLID_NOT)
	{
		e2->touch (e2, e1, &trace->plane, trace->surface);    // mike: this should send trace info, right??
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_RunTouches
//	
//	Two entities have touched, so run their touch functions
///////////////////////////////////////////////////////////////////////////////

void P_RunTouches (edict_t *e1, trace_t *trace)
{
	edict_t *e2 = trace->ent;

	if (e1->touch && e1->solid != SOLID_NOT)
	{
		e1->touch (e1, e2, &trace->plane, trace->surface);
	}

	if (e2->touch && e2->solid != SOLID_NOT)
	{
		e2->touch (e2, e1, &trace->plane, trace->surface);    // mike: this should send trace info, right??
	}
}

///////////////////////////////////////////////////////////////////////////////
//	ClipVelocity
//
//	Slide off of the impacting object
//	returns the blocked flags (1 = floor, 2 = step / wall)
///////////////////////////////////////////////////////////////////////////////

int ClipVelocity (CVector &in_velocity, CVector &normal, CVector &out_velocity, float overbounce)
{
	float	backoff;
	float	change;
	int		blocked;
	
	blocked = 0;
	if ( normal.z > 0 )
	{
		blocked |= 1;		// floor
	}
	if ( !normal.z )
	{
		blocked |= 2;		// step
	}

	backoff = DotProduct (in_velocity, normal) * overbounce;

	change = normal.x * backoff;
	out_velocity.x = in_velocity.x - change;
	if (out_velocity.x > -PHYS_STOP_EPSILON && out_velocity.x < PHYS_STOP_EPSILON)
	{
		out_velocity.x = 0;
	}
	
	change = normal.y * backoff;
	out_velocity.y = in_velocity.y - change;
	if (out_velocity.y > -PHYS_STOP_EPSILON && out_velocity.y < PHYS_STOP_EPSILON)
	{
		out_velocity.y = 0;
	}

	change = normal.z * backoff;
	out_velocity.z = in_velocity.z - change;
	if (out_velocity.z > -PHYS_STOP_EPSILON && out_velocity.z < PHYS_STOP_EPSILON)
	{
		out_velocity.z = 0;
	}

	return blocked;
}

///////////////////////////////////////////////////////////////////////////////
//	ClipEntVelocity
//
//	reflects a velocity vector from a plane with normal normal.
//	scales speed by ent->elasticy, so based on elasticity, ents can either
//	lose speed, gain speed, or keep a constant speed (however, gravity is still
//	affecting the entity, so maintaining a constant speed is unlikely).
///////////////////////////////////////////////////////////////////////////////

int ClipEntVelocity (edict_t *ent, CVector &in_velocity, CVector &normal, CVector &out_velocity)
{
	float	dp;
	float	speed;
	int		blocked;

	blocked = 0;
	if ( normal.z > 0 )
	{
		blocked |= 1;		// floor
	}
	if ( !normal.z )
	{
		blocked |= 2;		// step
	}

	speed = in_velocity.Normalize();
	dp = DotProduct (in_velocity, normal) * 2.0;

	out_velocity.x = in_velocity.x - (normal.x * dp);
	if (out_velocity.x > -PHYS_STOP_EPSILON && out_velocity.x < PHYS_STOP_EPSILON)
	{
		out_velocity.x = 0;
	}

	out_velocity.y = in_velocity.y - (normal.y * dp);
	if (out_velocity.y > -PHYS_STOP_EPSILON && out_velocity.y < PHYS_STOP_EPSILON)
	{
		out_velocity.y = 0;
	}

	out_velocity.z = in_velocity.z - (normal.z * dp);
	if (out_velocity.z > -PHYS_STOP_EPSILON && out_velocity.z < PHYS_STOP_EPSILON)
	{
		out_velocity.z = 0;
	}

	out_velocity.Normalize();
	out_velocity = out_velocity * (speed * ent->elasticity);

	return blocked;
}


///////////////////////////////////////////////////////////////////////////////
//	P_FlyMove
//
//	The basic solid body movement clip that slides along multiple planes
//	Returns the clipflags if the velocity was modified (hit something solid)
//	1 = floor
//	2 = wall / step
//	4 = dead stop
//	8 = stuck inside something
///////////////////////////////////////////////////////////////////////////////

int P_FlyMove (edict_t *ent, float time, int mask)
{
	edict_t		*hit;
	int			bumpcount, numbumps;
	CVector		dir;
	float		d;
	int			numplanes;
	CVector		planes[PHYS_MAX_CLIP_PLANES];
	CVector		primal_velocity, original_velocity, new_velocity;
	int			i, j;
	trace_t		trace;
	CVector		end;
	float		time_left;
	int			blocked;
	
	numbumps	= 4;
	
	blocked		= 0;
	original_velocity = ent->velocity;
	primal_velocity = ent->velocity;
	numplanes	= 0;
	
	time_left = time;

	if( ent->movetype == MOVETYPE_STEP )
	{
		ent->groundEntity = NULL;
	}
	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		end.x = ent->s.origin.x + time_left * ent->velocity.x;
		end.y = ent->s.origin.y + time_left * ent->velocity.y;
		end.z = ent->s.origin.z + time_left * ent->velocity.z;

		trace = gi.TraceBox (ent->s.origin, ent->s.mins, ent->s.maxs, end, ent, mask);

//		if (trace.allsolid || trace.startsolid)
		if( trace.allsolid )
		{	// entity is trapped in another solid
			ent->velocity = vec3_origin;
			return PHYS_BLOCKED_STUCK;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			ent->s.origin = trace.endpos;
			original_velocity = ent->velocity;
			numplanes = 0;
		}

		if (trace.fraction == 1)
		{
			 break;		// moved the entire distance
		}
		hit = trace.ent;

		if (trace.plane.normal.z > 0.7)
		{
			blocked |= PHYS_BLOCKED_FLOOR;		// floor
			if ( hit != NULL && hit->solid == SOLID_BSP)// || hit == g_edicts)
			{
				ent->groundEntity = hit;
				ent->groundSurface = trace.surface;
				ent->groundEntity_linkcount = hit->linkcount;
			}
		}

		//	NelnoFIXME:	should we check for step sides that aren't straight up/down?
		if (!trace.plane.normal.z)
		{
			blocked |= PHYS_BLOCKED_STEP;		// step
		}

//
// run the impact function
//
		P_Impact (ent, &trace);
		if (!ent->inuse)
		{
			break;		// removed by the impact function
		}
		
		time_left -= time_left * trace.fraction;
		
	// cliped to another plane
		if (numplanes >= PHYS_MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			ent->velocity = vec3_origin;
			return PHYS_BLOCKED_STEP | PHYS_BLOCKED_FLOOR;
		}

		planes[numplanes] = trace.plane.normal;
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);
			for (j=0 ; j<numplanes ; j++)
			{
				if (j != i)
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			}
			
			if (j == numplanes)
			{
				break;
			}
		}
		
		if (i != numplanes)
		{	// go along this plane
			ent->velocity = new_velocity;
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				gi.dprintf ("clip velocity, numplanes == %i\n",numplanes);
//				gi.conprintf ("FlyMove: %s dead stop.\n", ent->className);
				ent->velocity = vec3_origin;
				return PHYS_BLOCKED_STEP | PHYS_BLOCKED_FLOOR | PHYS_BLOCKED_STOPPED;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->velocity);
			ent->velocity = dir * d;
		}

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
		if (DotProduct (ent->velocity, primal_velocity) <= 0)
		{
			ent->velocity = vec3_origin;
			return blocked;
		}
	}

	return blocked;
}


///////////////////////////////////////////////////////////////////////////////
//	P_AddGravity
//
///////////////////////////////////////////////////////////////////////////////

void P_AddGravity (edict_t *ent)
{
    // NSS[2/16/00]:In case I want to use Directional gravity.. 
	if(ent->gravity_dir.Length() > 0.0f)
	{
		CVector Dir = ent->gravity_dir;
		Dir.Normalize();
		
		if(ent->waterlevel > 2)
		{
			if(ent->velocity.z <= -80.0f)
				Dir = Dir * (ent->gravity*0.20) * p_gravity->value * p_frametime_scaled;
				ent->velocity += Dir;
		}
		else
		{
			Dir = Dir * ent->gravity * p_gravity->value * p_frametime_scaled;	
			ent->velocity += Dir;
		}
		
	}
	else
	{
		// NSS[2/16/00]:Reduce gravity in water
		if(ent->waterlevel > 2)
		{
			if(ent->velocity.z <= -80.0f)
				ent->velocity.z -= (ent->gravity*0.20) * p_gravity->value * p_frametime_scaled;
		}
		else
			ent->velocity.z -= ent->gravity * p_gravity->value * p_frametime_scaled;
	}
}

/* ------------------------------- PUSHMOVE -------------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	P_PushEntity
//
//	Does not change the entities velocity at all
///////////////////////////////////////////////////////////////////////////////

trace_t P_PushEntity (edict_t *ent, CVector &push)
{
	trace_t	trace;
	CVector	start;
	CVector	end;
	int		mask;

	start = ent->s.origin;
	end = start + push;

retry:
	if (ent->clipmask)
	{
		mask = ent->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}

	trace = gi.TraceBox (start, ent->s.mins, ent->s.maxs, end, ent, mask);

	// SCG[2/4/00]: this keeps decos and shit from falling inside the world.
	if( trace.allsolid || 
		trace.startsolid && 
		(trace.endpos.x != start.x && 
		trace.endpos.y != start.y && 
		trace.endpos.z != start.z))
	{
		//	entity is stuck inside another entity
		//	GET IT OUT!! IT HURTS!! IT HURTS!!
		//gi.dprintf ("%s is stuck in %s.\n", ent->className, trace.ent->className);
		trace.fraction = 0.0;
		trace.endpos = start;
	}

	ent->s.origin = trace.endpos;
	gi.linkentity (ent);

	if (trace.fraction != 1.0 && trace.ent)
	{
		P_Impact (ent, &trace);

		//	if the pushed entity went away and the pusher is still there
		if (!trace.ent->inuse && ent->inuse)
		{
			// move the pusher back and try again
			ent->s.origin = start;
			gi.linkentity (ent);
			goto retry;
		}
	}

	if (ent->inuse) 
		P_TouchTriggers (ent);

	return trace;
}					

///////////////////////////////////////////////////////////////////////////////
//	P_Float_PushEntity
//
//	tries to move any entity that is touched, except for worldspawn
///////////////////////////////////////////////////////////////////////////////

trace_t P_Float_PushEntity (edict_t *ent, CVector &push)
{
	trace_t			trace;
	CVector			start, end;
	int				mask, i, pushes;
	int				pushed_ents, breakout;
	float			push_len;
	float_pushed_t	float_pushed [PHYS_MAX_PUSHES];

	start = ent->s.origin;
	end = start + push;
	
	push_len = push.Length();

p_float_pushentity_retry:
	breakout = false;
	pushed_ents = 0;

	memset (float_pushed, 0x00, sizeof (float_pushed_t) * PHYS_MAX_PUSHES);

	if (ent->clipmask)
	{
		mask = ent->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}

	for (pushes = 0; pushes < PHYS_MAX_PUSHES; pushes++)
	{
		trace = gi.TraceBox (start, ent->s.mins, ent->s.maxs, end, ent, mask);

		if (trace.allsolid || trace.startsolid && (trace.endpos.x != start.x && trace.endpos.y != start.y && trace.endpos.y != start.y))
		{
			//	entity is stuck inside another entity
			//	GET IT OUT!! IT HURTS!! IT HURTS!!
			//gi.dprintf ("%s is stuck in %s.\n", ent->className, trace.ent->className);

			goto	p_float_pushentity_exit;
		}

		if (trace.fraction == 1.0 || trace.ent == NULL)
		{
			//	moved all the way
			break;
		}

		if (trace.ent->flags & FL_CLIENT && push.z < 0.125)
		{
			breakout = true;
		}
		else 
		if (trace.fraction != 1.0 && trace.ent != g_edicts &&
			trace.ent->flags & FL_PUSHABLE)
		{
			//	try to push the entity that is in the way
			P_MomentumPush (ent, trace.ent, ent->velocity);

			//	FIXME:	requires that all p_phys routines for FL_PUSHABLE objects
			//			are re-entrant!!

			if (!(ent->flags & FL_CLIENT))
			{
				P_RunEntity (trace.ent, p_run_think);
			}
			else
			{
				P_PushEntity (trace.ent, push);
			}

			//	if entity didn't move, we can't possible go any farther
			//	so break out of tracing and do the touch functions
			if (trace.ent->s.old_origin.x == trace.ent->s.origin.x &&
				trace.ent->s.old_origin.y == trace.ent->s.origin.y &&
				trace.ent->s.old_origin.z == trace.ent->s.origin.z)
			{
				breakout = true;
			}
		}

		//////////////////////////////////////////////////////////////////////
		//	add to list of pushed entities for touching at end of move
		///////////////////////////////////////////////////////////////////////
		if (trace.ent)// && trace.ent != g_edicts)
		{
			//	add the pushed entity to the float_pushed list if not already added
			if (pushed_ents == 0)
			{
				float_pushed[0].trace = trace;
				pushed_ents++;
			}
			else
			{
				// SCG[1/22/00]: haha. dumbfucker.... (n9otice the semicolon at the end...)
//				for (i = 0; i < pushed_ents && float_pushed [i].trace.ent != trace.ent; i++);
				for (i = 0; i < pushed_ents && float_pushed [i].trace.ent != trace.ent; i++)
				{
					//	wasn't in list, so add it
					if (i >= pushed_ents)
					{
						float_pushed [pushed_ents].trace = trace;
						pushed_ents++;
					}
				}
			}
		}

		if (breakout)
		{
			break;
		}
	}

	trace = gi.TraceBox (start, ent->s.mins, ent->s.maxs, end, ent, mask);
	if (trace.allsolid || trace.startsolid)// && (trace.endpos [0] != start [0] && trace.endpos [1] != start [1] && trace.endpos [2] != start [2]))
	{
		//	entity is stuck inside another entity
		//	GET IT OUT!! IT HURTS!! IT HURTS!!
		goto	p_float_pushentity_exit;
	}

	ent->s.origin = trace.endpos;
	gi.linkentity (ent);

	if (P_TestEntityPosition (ent))
	{
		//	this should never happen!!
		gi.Con_Printf("PusEntity: TestEntityPosition failed.\n");
	}

	for (i = 0; i < pushed_ents; i++)
	{
		P_RunTouches (ent, &float_pushed [i].trace);

		//	if the pushed entity went away and the pusher is still there
		if (!float_pushed [i].trace.ent->inuse && ent->inuse)
		{
			// move the pusher back and try again
			ent->s.origin = start;
			gi.linkentity (ent);
			goto p_float_pushentity_retry;
		}
	}

p_float_pushentity_exit:

	//	FIXME:	touch all pushed entities
	if (ent->inuse) 
	{
		P_TouchTriggers (ent);
	}

	return trace;
}					


void com_Damage (userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, CVector &point_of_impact, 
				 CVector &damage_vec, float damage, unsigned long damage_flags);
///////////////////////////////////////////////////////////////////////////////
//	P_Push
//
//	Objects need to be moved back on a failed push,
//	otherwise riders would continue to slide.
///////////////////////////////////////////////////////////////////////////////

qboolean P_PushForceMove( edict_t *pusher, CVector &move, CVector &amove )
{
	int			i, e;
	edict_t		*check, *block;
	CVector		mins, maxs;
	pushed_t	*p;
	CVector		org, org2, move2, forward, right, up;

	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	for (i=0 ; i<3 ; i++)
	{
		float	temp;
		temp = move[i]*8.0;
		if (temp > 0.0)
		{
			temp += 0.5;
		}
		else
		{
			temp -= 0.5;
		}

		move.SetValue( 0.125 * (int)temp, i );
	}

	// find the bounding box
	mins = pusher->absmin + move;
	maxs = pusher->absmax + move;

// we need this for pushing things later
	org = vec3_origin - amove;
	AngleToVectors (org, forward, right, up);

// save the pusher's original position
	pushed_p->ent = pusher;
	pushed_p->origin = pusher->s.origin;
	pushed_p->angles = pusher->s.angles;

#if 0
	// amw 3.20.99 - I can't figure out why we do this here.. deltayaw never really gets set for the
	// entity that might get pushed back below... and because of that.. the player ends up getting
	// a deltayaw from a door so something that then jacks the players angles
	if (pusher->client)
	{
		pushed_p->deltayaw = SHORT2ANGLE (pusher->client->ps.pmove.delta_angles[YAW]);
	}
#endif	
	
	pushed_p++;

// move the pusher to it's final position
	pusher->s.origin = pusher->s.origin + move;
	pusher->s.angles = pusher->s.angles + amove;
	gi.linkentity (pusher);

	// see if any solid entities are inside the final position
	check = g_edicts+1;
	for (e = 1; e < globals.num_edicts; e++, check++)
	{
		if (!check->inuse)
		{
			continue;
		}

		// don't collide with entities that don't care about collision
		if (check->movetype == MOVETYPE_PUSH || 
			check->movetype == MOVETYPE_STOP || 
			check->movetype == MOVETYPE_NONE || 
			check->movetype == MOVETYPE_NOCLIP)
		{
			continue;
		}

		// these entities (such as items in a cabinet) should not cause collisions
		// with bmodel doors
		if (check->svflags & SVF_NOPUSHCOLLIDE)
		{
			continue;
		}

		if (!check->area.prev)
			continue;

	// if the entity is standing on the pusher, it will definitely be moved
		if (check->groundEntity != pusher)
		{
			// see if the ent needs to be tested
			if (check->absmin.x >= maxs.x	|| 
				check->absmin.y >= maxs.y	|| 
				check->absmin.z >= maxs.z	|| 
				check->absmax.x <= mins.x	|| 
				check->absmax.y <= mins.y	|| 
				check->absmax.z <= mins.z )
			{
				continue;
			}

			// see if the ent's bbox is inside the pusher's final position
			if (!P_TestEntityPosition (check))
			{
				continue;
			}
		}

		if ((pusher->movetype == MOVETYPE_PUSH) || (check->groundEntity == pusher))
		{
			// move this entity
			pushed_p->ent = check;
			pushed_p->origin = check->s.origin;
			pushed_p->angles = check->s.angles;

			// amw 3.30.99 - save this here.. that way we can restore it properly
/*
			if (check->client && (check->flags & FL_CLIENT))
			{	
				pushed_p->deltayaw = SHORT2ANGLE (check->client->ps.pmove.delta_angles[YAW]);
			}
*/
			pushed_p++;

			// try moving the contacted entity 
			check->s.origin = check->s.origin + move;

/*
			if (check->client && (check->flags & FL_CLIENT))
			{	// FIXME: doesn't rotate monsters?
				check->client->ps.pmove.delta_angles[YAW] += ANGLE2SHORT (amove[YAW]);
			}
*/
			// figure movement due to the pusher's amove
			org = check->s.origin - pusher->s.origin;
			org2.x = DotProduct (org, forward);
			org2.y = -DotProduct (org, right);
			org2.z = DotProduct (org, up);
			move2 = org2 - org;
			check->s.origin = check->s.origin + move2;

			// may have pushed them off an edge
			if (check->groundEntity != pusher)
			{
				check->groundEntity = NULL;
				check->groundSurface = NULL;
			}

			block = P_TestEntityPosition (check);
			if (!block)
			{	// pushed ok
				gi.linkentity (check);
				// impact?
				continue;
			}

			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			check->s.origin = check->s.origin - move - move2;
			if( check->client )
				check->client->ps.pmove.delta_angles[YAW] -= ANGLE2SHORT (amove[YAW]);
			block = P_TestEntityPosition (check);
			if (!block)
			{
				pushed_p--;
				continue;
			}

			// is the pusher an entity that should force other entities out of 
			// the way instead of not moving ?  (i.e. an escalator)
			if (pusher->flags & FL_FORCEMOVE)
			{
				// if the entity we're colliding with is really SOLID_NOT, 
				// (like a glock shell) we don't care so blow off this collision
				if (check->solid == SOLID_NOT)
				{
					pushed_p--;
					continue;
				}

				// last check... if this is the client or projectile, it could be on
				// an escalator.. if so, let's try bumping it up instead
				// of into the escalator itself..

				if (move.z < 0)	// train is going down
				{
					// push it forward and up
					check->s.origin.x += move.x;
					check->s.origin.y += move.y;
					check->s.origin.z += 8;
				}
				else				// train is going up
				{
					// push it backwards and up
					check->s.origin.x -= move.x;
					check->s.origin.y -= move.y;
					check->s.origin.z += 8;
				}

				block = P_TestEntityPosition (check);
				if (!block)
				{
					gi.linkentity (check);
					// impact?
					continue;
				}

				// if we're here... we're kinda fucked.. the FL_FORCEMOVE entity is
				// going to collide with 'check'  ... we need to add a 'remove_t' 
				// function pointer to projectiles etc so that we can remove them
				// from the world
				if (check->remove)
				{
					gi.Con_Printf("P_PushForceMove: Removing entity colliding with FL_FORCEMOVE entity.\n");
					check->remove(check);
					pushed_p--;
					continue;
				}
			}
		}
		
		// save off the obstacle so we can call the block function
		obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for (p=pushed_p-1 ; p>=pushed ; p--)
		{
			p->ent->s.origin = p->origin;
			p->ent->s.angles = p->angles;

			//	Nelno:	causes client to slide slightly
/*
			if (p->ent->client && p->ent->flags & FL_CLIENT)
			{
				p->ent->client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT (p->deltayaw);
			}
*/
			gi.linkentity (p->ent);
		}
		return false;
	}

//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for (p=pushed_p-1 ; p>=pushed ; p--)
		P_TouchTriggers (p->ent);

	return true;
}

qboolean P_Push( edict_t *pusher, CVector &move, CVector &amove )
{
//	int			i, e;
	int			e;
	edict_t		*check, *block;
	CVector		mins, maxs;
	pushed_t	*p;
	CVector		org, org2, move2, forward, right, up,dir;

	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	float temp;

	temp = move.x * 8.0;
	if (temp > 0.0) temp += 0.5;
	else temp -= 0.5;
	move.SetValue( 0.125 * ( int )temp, 0 );

	temp = move.y * 8.0;
	if (temp > 0.0) temp += 0.5;
	else temp -= 0.5;
	move.SetValue( 0.125 * ( int )temp, 1 );

	temp = move.z * 8.0;
	if (temp > 0.0) temp += 0.5;
	else temp -= 0.5;
	move.SetValue( 0.125 * ( int )temp, 2 );

	// find the bounding box
	mins = pusher->absmin + move;
	maxs = pusher->absmax + move;

// we need this for pushing things later
	org = vec3_origin - amove;
	AngleToVectors (org, forward, right, up);

// save the pusher's original position
	pushed_p->ent = pusher;
	pushed_p->origin = pusher->s.origin;
	pushed_p->angles = pusher->s.angles;

	if( pusher->client != NULL )
	{
		pushed_p->deltayaw = SHORT2ANGLE (pusher->client->ps.pmove.delta_angles[YAW]);
	}
	
	pushed_p++;

// move the pusher to it's final position
	pusher->s.origin = pusher->s.origin + move;
	pusher->s.angles = pusher->s.angles + amove;
	gi.linkentity (pusher);

	// see if any solid entities are inside the final position
	check = g_edicts+1;
	for (e = 1; e < globals.num_edicts; e++, check++)
	{
		if( !check->inuse )
		{
			continue;
		}

		// don't collide with entities that don't care about collision
		if( check->movetype == MOVETYPE_PUSH || 
			check->movetype == MOVETYPE_STOP || 
			check->movetype == MOVETYPE_NONE || 
			check->movetype == MOVETYPE_NOCLIP )
		{
			continue;
		}

		// these entities (such as items in a cabinet) should not cause collisions
		// with bmodel doors
		if( check->svflags & SVF_NOPUSHCOLLIDE )
		{
			continue;
		}

		if( !check->area.prev )
		{
			continue;
		}

	// if the entity is standing on the pusher, it will definitely be moved
		if (check->groundEntity != pusher)
		{
			// see if the ent needs to be tested
			if( check->absmin.x >= maxs.x	|| 
				check->absmin.y >= maxs.y	|| 
				check->absmin.z >= maxs.z	|| 
				check->absmax.x <= mins.x	|| 
				check->absmax.y <= mins.y	|| 
				check->absmax.z <= mins.z )
			{
				continue;
			}

			// see if the ent's bbox is inside the pusher's final position
			if (!P_TestEntityPosition (check))
			{
				continue;
			}
		}

		if( ( pusher->movetype == MOVETYPE_PUSH ) || ( check->groundEntity == pusher ) )
		{
			// move this entity
			pushed_p->ent = check;
			pushed_p->origin = check->s.origin;
			pushed_p->angles = check->s.angles;
			pushed_p++;

			// try moving the contacted entity 
			check->s.origin = check->s.origin + move;

/*
			if( ( check->client != NULL ) && ( check->flags & FL_CLIENT ) )
			{	// FIXME: doesn't rotate monsters?
				check->client->ps.pmove.delta_angles[YAW] += ANGLE2SHORT (amove[YAW]);
			}
*/
			// figure movement due to the pusher's amove
			org = check->s.origin - pusher->s.origin;
			org2.x = DotProduct (org, forward);
			org2.y = -DotProduct (org, right);
			org2.z = DotProduct (org, up);
			move2 = org2 - org;
			check->s.origin = check->s.origin + move2;

			// may have pushed them off an edge
			if( check->groundEntity != pusher )
			{
				check->groundEntity = NULL;
				check->groundSurface = NULL;
			}

			block = P_TestEntityPosition (check);
			if (!block)
			{	// pushed ok
				gi.linkentity (check);
				// impact?
				continue;
			}

			check->s.origin = check->s.origin - move;
/*
			if( ( check->client != NULL ) && ( check->flags & FL_CLIENT ) )
			{
				check->client->ps.pmove.delta_angles[YAW] -= ANGLE2SHORT (amove[YAW]);
			}
*/
			block = P_TestEntityPosition (check);
			if (!block)
			{
				pushed_p--;
				continue;
			}
		}

		// save off the obstacle so we can call the block function
		obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for( p = pushed_p - 1; p >= pushed; p-- )
		{
			p->ent->s.origin = p->origin;
			p->ent->s.angles = p->angles;

			//	Nelno:	causes client to slide slightly
/*
			if( ( p->ent->client != NULL ) && ( check->flags & FL_CLIENT ) )
			{
				p->ent->client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT( p->deltayaw );
			}
*/
			gi.linkentity (p->ent);
		}
		return false;
	}

//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for (p=pushed_p-1 ; p>=pushed ; p--)
		P_TouchTriggers (p->ent);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Pusher
//
//	Bmodel objects don't interact with each other, but
//	push all box objects
///////////////////////////////////////////////////////////////////////////////

void P_Physics_Pusher (edict_t *ent)
{
	CVector		move, amove, new_ang, new_org, offset;
	CVector		forward, right, up;

	edict_t		*part, *mv, *parent = NULL;
	qboolean	bIsParent = FALSE, bParentMoved = FALSE;

	int			i = 0;

	// if not a team captain, so movement will be handled elsewhere
	if ( ent->flags & FL_TEAMSLAVE)
	{
		if( strstr( ent->className, "projectile_c4" ) )
		{
			if( ent->think ) 
			{	
				P_RunThink( ent, p_run_think );
			}
		}
		return;
	}

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out

//retry:
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain)
	{
		// is this the parent entity?
		if (part->teamchain && !(part->flags & FL_TEAMSLAVE))
		{
			bIsParent = TRUE;
		}
		else
		{
			bIsParent = FALSE;
		}

		// is this the parent ? if so.. save his original angles
		if (bIsParent)
		{
			parent = part;
		
			part->s.angles.x = anglemod ( part->s.angles.x );
			part->s.angles.y = anglemod ( part->s.angles.y );
			part->s.angles.z = anglemod ( part->s.angles.z );
		}
		else if( parent != NULL )
		{
			// we need to modify the parent for the movement and rotation of the parent
			// transform according to parent->s.angles... if bParentMoved == TRUE then we'll 
			// recalculate the child's position in relation to the parent
			if (bParentMoved)
			{
				////////////////////////////////////
				// update the position of the child
				////////////////////////////////////

				// transform the offset from the parent
				offset = part->childOffset;
				// negate Y
				offset.y = -offset.y;

				new_org.x = DotProduct(offset, forward) + parent->s.origin.x;
				new_org.y = DotProduct(offset, right) + parent->s.origin.y;
				new_org.z = DotProduct(offset, up) + parent->s.origin.z;

				// update the child origin
				part->s.origin = new_org;
				part->s.old_origin = part->s.origin;

				// update the transformed origin difference
				part->transformedOffset = part->s.origin - parent->s.origin;
				
				////////////////////////////////////
				// update the angles of the child
				////////////////////////////////////
				part->s.angles = parent->s.angles + part->s.angle_delta;  // 2.17.99

				// make sure they are between 0-359
				part->s.angles.x = anglemod ( part->s.angles.x );
				part->s.angles.y = anglemod ( part->s.angles.y );
				part->s.angles.z = anglemod ( part->s.angles.z );
			}
		}

		// if this part has any velocities, it means it's going to move so we need to check physics
		if (bParentMoved ||
			part->velocity.x  || part->velocity.y  || part->velocity.z ||
			part->avelocity.x || part->avelocity.y || part->avelocity.z)
		{	// object is moving
			move = part->velocity * p_frametime_scaled;
			amove = part->avelocity * p_frametime_scaled;

/*
			if( part->flags & FL_FORCEMOVE )
			{
				if( P_PushForceMove( part, move, amove ) == false ) 
				{
					break;	// move was blocked
				}
			}
			else
*/			{
				if( P_Push( part, move, amove ) == false )
				{
					break;	// move was blocked
				}
			}
		

			if (bIsParent)
			{
				bParentMoved = TRUE;
			}
		}
		
		// save the angular difference of the parent
		if (bIsParent && bParentMoved && parent != NULL)
		{
			// calculate the transformation matrix
			new_ang = parent->s.angles;
			AngleToVectors (new_ang, forward, right, up);
		}

	}

	if (pushed_p > &pushed[MAX_EDICTS])
	{
		gi.Error ("pushed_p > &pushed[MAX_EDICTS], memory corrupted");
	}

	if (part)
	{
		// the move failed, bump all nextthink times and back out moves
		for (mv = ent ; mv ; mv=mv->teamchain)
		{
			if (mv->nextthink > 0)
			{
				mv->nextthink += p_frametime_scaled;
			}
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		if (part->blocked)
		{
			part->blocked (part, obstacle);
		}
#if 0
		// if the pushed entity went away and the pusher is still there
		if (!obstacle->inuse && part->inuse)
			goto retry;
#endif
	}
	else
	{
		// the move succeeded, so call all think functions
		for (part = ent ; part ; part=part->teamchain)
		{
			if (part->think) 
			{	
				P_RunThink (part, p_run_think);
			}
		}
	}
}

/* -------------------------- MOVETYPE_NONE -------------------------------- */


///////////////////////////////////////////////////////////////////////////////
//  P_Physics_None
//
//	Non moving objects can only think
///////////////////////////////////////////////////////////////////////////////

void P_Physics_None (edict_t *ent)
{
	if (ent->think)
	{
		P_RunThink (ent, p_run_think);
	}
}

/* -------------------------- MOVETYPE_NOCLIP ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Noclip
//
//	A moving object that doesn't obey physics
///////////////////////////////////////////////////////////////////////////////

void P_Physics_Noclip (edict_t *ent)
{
	if (!P_RunThink (ent, p_run_think))
	{
		return;
	}

	VectorMA(ent->s.angles, ent->avelocity, p_frametime_scaled, ent->s.angles);
	VectorMA(ent->s.origin, ent->velocity, p_frametime_scaled, ent->s.origin);

	gi.linkentity (ent);
}


/* ------------------ MOVETYPE_TOSS, MOVETYPE_BOUNCE ---------------------- */


///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Toss
//
//	Toss, bounce, and fly movement.  When onground, do nothing.
///////////////////////////////////////////////////////////////////////////////

void P_Physics_Toss (edict_t *ent)
{
	trace_t		trace;
	CVector		move;
	CVector		old_origin;
	int			mask, blocked;
	float		backoff;

	// regular thinking
	if (ent->think) 
	{
		P_RunThink (ent, p_run_think);
		if (!ent->inuse)
		{
			//	entity was removed during its think
			return;
		}
	}

	// SCG[2/11/00]: we need to do this so items don't float in the air.
	// SCG[2/14/00]: oh god. I feel another hack coming on.
	// SCG[2/14/00]: only certain type need to check for ground entity ( because it zeroes our upard velocity
	// SCG[2/14/00]: thus preventing anything from bouncing.
	switch( ent->movetype )
	{
	case MOVETYPE_BOUNCEMISSILE:
	case MOVETYPE_FLYMISSILE:
		break;

	case MOVETYPE_SWIM:
    case MOVETYPE_TOSS:
	case MOVETYPE_FLY:
	case MOVETYPE_HOVER:

	case MOVETYPE_BOUNCE:
	default:
		P_CheckGround( ent );
		break;
	}

	// SCG[12/7/99]: Quake2 addition
	if (ent->velocity.z > 0 && ent->movetype != MOVETYPE_SWIM)
	{
		ent->groundEntity = NULL;
	}

	//	already come to a rest, so exit
	//	FIXME:	this won't register touches on toss's that have stopped
/*
	if( ent->groundEntity && !( ent->movetype == MOVETYPE_SWIM ) )
	{
		return;
	}
*/
	//	slave, so move when the master moves
	if ( ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	old_origin = ent->s.origin;

	//	clamp velocity to p_maxvelocity
	P_CheckVelocity (ent);

    int nMoveType = ent->movetype;

	if ( ( nMoveType == MOVETYPE_TOSS ) ||
		( nMoveType == MOVETYPE_BOUNCE ) ||
//		( nMoveType == MOVETYPE_BOUNCEMISSILE ) ||
		( nMoveType == MOVETYPE_SWIM ) && 
		( ent->waterlevel < 3 ) )
	{
		//	add gravity to swimming things if they are not completely submerged
		//	add gravity to movetypes that have it
		// NSS[2/7/00]:No gravity if we are swiming and touching the ground.
		if(!ent->groundEntity && nMoveType == MOVETYPE_SWIM)
		{
			P_AddGravity( ent );
		}
		else if( nMoveType != MOVETYPE_SWIM )
		{
			P_AddGravity( ent );		
		}
	}

	// move angles
	VectorMA(ent->s.angles, ent->avelocity, p_frametime_scaled, ent->s.angles);
	
	if (ent->svflags & SVF_PUSHED)
	{
		//	entity was pushed by something on last frame, so make it slide
		//	along planes rather than bounce off of them
		if (p_run_think)
		{
			ent->svflags -= SVF_PUSHED;
		}

		if (ent->clipmask)
		{
			mask = ent->clipmask;
		}
		else
		{
			mask = MASK_SOLID;
		}

		blocked = P_FlyMove (ent, p_frametime_scaled, mask);
		if (blocked)
		{
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}
		else 
		if (P_TestEntityPosition (ent))
		{
//			gi.conprintf ("SVF_PUSHED:  TestentityPosition failed!\n");
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}

		if (ent->percent_submerged == 0.0)
		{
			P_CheckGround (ent);
		}

		gi.linkentity (ent);
	}
	else
	{
		// move origin
		move = ent->velocity * p_frametime_scaled;
		trace = P_PushEntity (ent, move);

		if (!ent->inuse)
		{
			//	touch function may have removed entity
			return;
		}

		//	movement vector is down and entity has not moved
		if ((nMoveType != MOVETYPE_SWIM &&  ent->velocity.z < 0 && (ent->s.old_origin.x == ent->s.origin.x && ent->s.old_origin.y == ent->s.origin.y && ent->s.old_origin.z == ent->s.origin.z)))
		{
			ent->velocity = vec3_origin;
			ent->avelocity = vec3_origin;
			ent->groundEntity = g_edicts;
			ent->groundSurface = trace.surface;
			ent->groundEntity_linkcount = g_edicts[0].linkcount;
			return;
		}
		if (trace.fraction < 1)
		{
            if(ent->movetype == MOVETYPE_BOUNCEMISSILE)
                ClipEntVelocity (ent, ent->velocity, trace.plane.normal, ent->velocity);
            else 
            {
                if ((ent->movetype == MOVETYPE_BOUNCE) || (ent->movetype == MOVETYPE_FLOAT))
				    backoff = 1.5;
			    else
				    backoff = 1;

			    //ClipEntVelocity (ent, ent->velocity, trace.plane.normal, ent->velocity);
			    ClipVelocity (ent->velocity, trace.plane.normal, ent->velocity, backoff);

			    // stop if on ground
			    if (trace.plane.normal.z > 0.7)
			    {		
				    //if (ent->velocity.z < 60 || (ent->movetype != MOVETYPE_BOUNCE) )
					if (ent->velocity.z < 60 && (ent->movetype == MOVETYPE_TOSS) )
				    {
					    //ent->groundentity = trace.ent;
					    //ent->groundentity_linkcount = trace.ent->linkcount;
					    ent->velocity.Set(0,0,0);
					    ent->avelocity.Set(0,0,0);
				    }
			    }
            }

		}
	}

	//	do MOVETYPE_TOSS's need to touch triggers???
	if (ent->inuse) 
	{
		P_TouchTriggers (ent);
	}

	P_CheckWaterTransition (ent, old_origin);
}

///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Float
//
///////////////////////////////////////////////////////////////////////////////

void P_Physics_Float (edict_t *ent)
{
	trace_t		trace;
	CVector		move;
	CVector		old_origin;
	int			mask, blocked;

	// regular thinking
	if (ent->think) 
	{
		P_RunThink (ent, p_run_think);
		if (!ent->inuse)
		{
			//	entity was removed during its think
			return;
		}
	}

	if (ent->groundEntity && ent->percent_submerged == 0.0)
	{
		P_CheckGround (ent);
		if (ent->groundEntity)
		{
			return;
		}
	}

	//	slave, so move when the master moves
	if ( ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	old_origin = ent->s.origin;

	//	clamp velocity to p_maxvelocity
	P_CheckVelocity (ent);

	ent->percent_submerged = P_PhysicsFloat (ent);
	if (ent->percent_submerged == 0.0)
	{
		//	not floating, so add gravity
		P_AddGravity (ent);
	}

	if (ent->svflags & SVF_PUSHED)
	{
		//	entity was pushed by something on last frame, so make it slide
		//	along planes rather than bounce off of them
		if (p_run_think)
		{
			ent->svflags -= SVF_PUSHED;
		}

		if (ent->clipmask)
		{
			mask = ent->clipmask;
		}
		else
		{
			mask = MASK_SOLID;
		}

		blocked = P_FlyMove (ent, p_frametime_scaled, mask);
		if (blocked)
		{
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}
		else 
		if (P_TestEntityPosition (ent))
		{
//			gi.conprintf ("SVF_PUSHED:  TestentityPosition failed!\n");
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}

		if (ent->percent_submerged == 0.0)
		{
			P_CheckGround (ent);
		}

		gi.linkentity (ent);
	}
	else
	{
		// move origin
		move = ent->velocity * p_frametime_scaled;
//		trace = P_PushEntity (ent, move);
		trace = P_Float_PushEntity (ent, move);

		if (!ent->inuse)
		{
			//	touch function may have removed entity
			return;
		}

		//	movement vector is down and entity has not moved
		if (ent->velocity.z < 0 && ent->percent_submerged == 0.0 && 
			ent->s.old_origin.x == ent->s.origin.x && 
			ent->s.old_origin.y == ent->s.origin.y && 
			ent->s.old_origin.z == ent->s.origin.z)
		{
			ent->velocity = vec3_origin;
			ent->avelocity = vec3_origin;
			ent->groundEntity = g_edicts;
			ent->groundSurface = trace.surface;
			ent->groundEntity_linkcount = g_edicts[0].linkcount;
			return;
		}

		if (trace.fraction < 1)
		{
			ClipEntVelocity (ent, ent->velocity, trace.plane.normal, ent->velocity);
		}
	}

	//	do MOVETYPE_FLOAT's need to touch triggers???
	if (ent->inuse) 
	{
		P_TouchTriggers (ent);
	}

	P_CheckWaterTransition (ent, old_origin);
}

/* ---------------------------- MOVETYPE_STEP ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//	P_CheckBottom
//
//	Returns false if any part of the bottom of the entity is off an edge that
//	is not a staircase.
///////////////////////////////////////////////////////////////////////////////
#define	STEPSIZE	18

int c_yes, c_no;

qboolean P_CheckBottom (edict_t *ent)
{
/*
	CVector	start, stop;
	trace_t	trace;

	start = ent->s.origin;
	stop = ent->s.origin;
	stop.z += 1;

	trace = gi.TraceBox (start, ent->s.mins, ent->s.maxs, stop, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1.0)
	{
		//	not on ground
		return	false;
	}
	
	return	true;
*/
	CVector mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float	mid, bottom;
	
	mins = ent->s.origin + ent->s.mins;
	maxs = ent->s.origin + ent->s.maxs;

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
	start.z = mins.z - 1;
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start.x = x ? maxs.x : mins.x;
			start.y = y ? maxs.y : mins.y;
			if (gi.pointcontents (start) != CONTENTS_SOLID)
				goto realcheck;
		}

	c_yes++;
	return true;		// we got out easy

realcheck:
	c_no++;
//
// check it for real...
//
	start.z = mins.z;
	
// the midpoint must be within 16 of the bottom
	start.x = stop.x = (mins.x + maxs.x)*0.5;
	start.y = stop.y = (mins.y + maxs.y)*0.5;
	stop.z = start.z - 2*STEPSIZE;
	trace = gi.TraceBox(start, CVector( 0, 0, 0 ), CVector( 0, 0, 0 ), stop, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1.0)
		return false;
	mid = bottom = trace.endpos[2];
	
// the corners must be within 16 of the midpoint	
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start.x = stop.x = x ? maxs.x : mins.x;
			start.y = stop.y = y ? maxs.y : mins.y;
			
			trace = gi.TraceBox(start, CVector( 0, 0, 0 ), CVector( 0, 0, 0 ), stop, ent, MASK_MONSTERSOLID);
			
			if (trace.fraction != 1.0 && trace.endpos.z > bottom)
				bottom = trace.endpos.z;
			if (trace.fraction == 1.0 || mid - trace.endpos.z > STEPSIZE)
				return false;
		}

	c_yes++;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	P_AddRotationalFricton
//
///////////////////////////////////////////////////////////////////////////////

void P_AddRotationalFriction (edict_t *ent)
{
	float	adjustment;

	VectorMA(ent->s.angles, ent->avelocity, p_frametime_scaled, ent->s.angles);
	adjustment = p_frametime_scaled * p_stopspeed * p_friction;

	if (ent->avelocity.x > 0)
	{
		ent->avelocity.x -= adjustment;
		if (ent->avelocity.x < 0)
		{
			ent->avelocity.x = 0;
		}
	}
	else
	{
		ent->avelocity.x += adjustment;
		if (ent->avelocity.x > 0)
		{
			ent->avelocity.x = 0;
		}
	}

	if (ent->avelocity.y > 0)
	{
		ent->avelocity.y -= adjustment;
		if (ent->avelocity.y < 0)
		{
			ent->avelocity.y = 0;
		}
	}
	else
	{
		ent->avelocity.y += adjustment;
		if (ent->avelocity.y > 0)
		{
			ent->avelocity.y = 0;
		}
	}

	if (ent->avelocity.z > 0)
	{
		ent->avelocity.z -= adjustment;
		if (ent->avelocity.z < 0)
		{
			ent->avelocity.z = 0;
		}
	}
	else
	{
		ent->avelocity.z += adjustment;
		if (ent->avelocity.z > 0)
		{
			ent->avelocity.z = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Step
//
//	Monsters freefall when they don't have a ground entity, otherwise
//	all movement is done with discrete steps.
//
//	This is also used for objects that have become still on the ground, but
//	will fall if the floor is pulled out from under them.
//	FIXME: is this true?
///////////////////////////////////////////////////////////////////////////////

void P_Physics_Step (edict_t *ent)
{
	qboolean	wasonground;
	qboolean	hitsound = false;
	CVector		vel;
	float		speed, newspeed, control;
	float		friction;
	edict_t		*groundEntity;
	int			mask;

	// airborn monsters should always check for ground
	//	Nelno: removed temporarily
	if (!ent->groundEntity)
	{
		P_CheckGround (ent);
	}

	groundEntity = ent->groundEntity;

	P_CheckVelocity (ent);

	if (groundEntity)
	{
		wasonground = true;
	}
	else
	{
		wasonground = false;
	}
		
	if (ent->avelocity.x || ent->avelocity.y || ent->avelocity.z)
	{
		P_AddRotationalFriction (ent);
	}

	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if ( !wasonground )
	{
//		if ( (ent->movetype == MOVETYPE_FLY || ent->movetype == MOVETYPE_HOVER) && 
//             (ent->movetype == MOVETYPE_SWIM) )
		if( ( ent->movetype != MOVETYPE_FLY ) && ( ent->movetype != MOVETYPE_HOVER ) )
		{
			if( !( ( ent->movetype == MOVETYPE_SWIM ) && ( ent->waterlevel > 2 ) ) )
			{
				if (ent->velocity.z < p_gravity->value*-0.1)
				{
					hitsound = true;
				}
				if (ent->waterlevel == 0)
				{
					P_AddGravity (ent);
				}
			}
		}
	}

	//	Nelno:	why is this in here twice?
	//	friction for flying monsters that have been given vertical velocity
//	if ( (ent->movetype == MOVETYPE_FLY || ent->movetype == MOVETYPE_HOVER) && 
//         !(ent->movetype == MOVETYPE_SWIM) && (ent->velocity.z != 0) )
	if ( ( ( ent->movetype == MOVETYPE_FLY ) || ( ent->movetype == MOVETYPE_HOVER ) ) && 
         ( ent->velocity.z != 0) )
	{
		speed = fabs(ent->velocity.z);
		control = speed < p_stopspeed ? p_stopspeed : speed;
		friction = p_friction/3;
		newspeed = speed - (p_frametime_scaled * control * friction);
		if (newspeed < 0)
		{
			newspeed = 0;
		}
		newspeed /= speed;
		ent->velocity.z *= newspeed;
	}

	//	friction for swimming monsters that have been given vertical velocity
	if ( (ent->movetype == MOVETYPE_SWIM) && (ent->velocity.z != 0) )
	{
		speed = fabs(ent->velocity.z);
		control = speed < p_stopspeed ? p_stopspeed : speed;
		newspeed = speed - (p_frametime_scaled * control * p_waterfriction * ent->waterlevel);
		if (newspeed < 0)
		{
			newspeed = 0;
		}
		newspeed /= speed;
		ent->velocity.z *= newspeed;
	}

	if (ent->velocity.z || ent->velocity.y || ent->velocity.x)
	{
		// apply friction
		// let dead monsters who aren't completely onground slide
		if ((wasonground) || (ent->movetype == MOVETYPE_FLY || ent->movetype == MOVETYPE_HOVER))
		{
			if (!(ent->health <= 0.0 && !P_CheckBottom (ent)))
			{
				vel = ent->velocity;
				speed = sqrt(vel.x*vel.x +vel.y*vel.y);
				if (speed)
				{
					friction = p_friction;

					control = speed < p_stopspeed ? p_stopspeed : speed;
					newspeed = speed - p_frametime_scaled * control * friction;

					if (newspeed < 0)
					{
						newspeed = 0;
					}
					newspeed /= speed;

					vel.x *= newspeed;
					vel.y *= newspeed;

					ent->velocity.x = vel.x;
					ent->velocity.y = vel.y;
				}
			}
		}

		if (ent->svflags & SVF_MONSTER)
		{
			mask = MASK_MONSTERSOLID;
		}
		else
		{
			mask = MASK_SOLID;
		}
		P_FlyMove (ent, p_frametime_scaled, mask);

		gi.linkentity (ent);
		P_TouchTriggers (ent);

		if (ent->groundEntity)
		{
			if (!wasonground)
			{
				if (hitsound)
				{
					CVector vZero( 0,0,0 );
					gi.StartSound(vZero, ent, CHAN_AUTO, gi.soundindex("world/land.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX, 0.0f);
				}
			}
		}
	}

// regular thinking
	if (ent->think) 
	{
		P_RunThink (ent, p_run_think);
	}
}

/* --------------------------- MOVETYPE_WALK ------------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	P_BotFriction
//
// NSS[2/13/00]:Noel's cheesy friction
//	
//	FIXME:	make so only bots with a flag set can walk off of ledges
//	Q2FIXME:make p_friction, p_stopspeed and cvars!
///////////////////////////////////////////////////////////////////////////////

void P_BotFriction (edict_t *ent)
{
	CVector	vel, origin;
	float	speed, newspeed;
	CVector	start, stop;
	float	friction;
	trace_t	trace;

	// if bot is in the air, don't apply friction!!!
	if (!ent->groundEntity)
	{
		return;
	}

	vel = ent->velocity;
	origin = ent->s.origin;
	
	//	get the speed in the xy plane
	speed = sqrt (vel.x * vel.x + vel.y * vel.y);
	
	if (!speed)
	{
		return;
	}

	//	if the leading edge is over a dropoff, increase friction
	//	start exactly at the bottom of ent's bounding box
	start.x = stop.x = origin.x;
	start.y = stop.y = origin.y;
	start.z = origin.z;
	
	//	end 34 units below (stair height + 10, or jump height + 2)
	stop.z = start.z + ent->s.mins.z;

	trace = gi.TraceBox (start, ent->s.mins, ent->s.maxs, stop, ent, ent->clipmask);

	// NSS[2/13/00]:Cheesy Hack to get surface friction working with monsters.
	if(trace.surface && trace.surface->flags & (SURF_ICE|SURF_SNOW))
	{
		friction = 0.80;
	}
	else
	{
		friction = 0.20;
	}
	// NSS[2/13/00]:Decrease the friction if we are going down a slope.
	if ( trace.fraction < 1.0 && trace.plane.normal.z < 1.0 && speed != 0 )
	{
		friction = 0.80;
		if(trace.plane.normal.z != 0.0)
		{
			friction *=  1/trace.plane.normal.z;
		}
		else
		{
			friction = 0.80;
		}
	}

	// NSS[2/13/00]:Cheese hack to make sure that our fraction is always less than or = 1
	if(friction > 1.0)
	{
		friction = 0.80;
	}

	newspeed = speed * friction;
	vel.Normalize();
	ent->velocity = vel * newspeed;
}

///////////////////////////////////////////////////////////////////////////////
//	P_BotWalkMove
//
//	monster/bot movement code
//	FIXME:	optimize this, only call this when monster is moving
///////////////////////////////////////////////////////////////////////////////

void P_BotWalkMove (edict_t *ent)
{
    CVector			upmove, downmove;
    CVector			oldorg, oldvel;
    CVector			nosteporg, nostepvel;
    int				clip, mask;
    trace_t			downtrace;

	///////////////////////////////////////////////////////////////////////////
	//	determine if there is ground right under our feet
	///////////////////////////////////////////////////////////////////////////

    oldorg = ent->s.origin;
	oldvel = ent->velocity;
    
	if (!ent->clipmask)
	{
		mask = MASK_MONSTERSOLID;
	}
	else
	{
		mask = ent->clipmask;
	}


    clip = P_FlyMove (ent, p_frametime_scaled, mask);
	
	nosteporg = ent->s.origin;
    nostepvel = ent->velocity;

    if (!(clip & PHYS_BLOCKED_STEP))
	{
		P_TouchTriggers (ent);
		
		if ( ent->s.origin.x != oldorg.x || ent->s.origin.y != oldorg.y || ent->s.origin.z != oldorg.z )
		{
			return;        // move didn't block on a step
		}
	}

	//	Q2FIXME:	this should work...
//    if (!oldonground && ent->waterlevel == 0)
//        return;        // don't stair up while jumping

    if (ent->flags & FL_WATERJUMP)
	{
		P_TouchTriggers (ent);
		//	jumping out of water
        return;
	}


//
// try moving up and forward to go up a step
//
	ent->s.origin = oldorg;    // back to start pos

    upmove = vec3_origin;
    downmove = vec3_origin;
    
	upmove.z = PHYS_STEPSIZE;
    downmove.z = - PHYS_STEPSIZE + oldvel.z * p_frametime_scaled;

	// move up
    P_PushEntity (ent, upmove);

	// move forward
    ent->velocity.x = oldvel.x;
    ent->velocity.y = oldvel.y;
    ent->velocity.z = 0;
    
	clip = P_FlyMove (ent, p_frametime_scaled, mask);

	// move down
    downtrace = P_PushEntity (ent, downmove);

    if (downtrace.plane.normal.z > 0.7)
    {
        if (ent->solid == SOLID_BSP || ent->solid == SOLID_BBOX)
		{
            ent->groundEntity = downtrace.ent;
			ent->groundSurface = downtrace.surface;
		}
    }
    else
    {
		//	if the push down didn't end up on good ground, use the move without
		//	the step up.  This happens near wall / slope combinations, and can
		//	cause the player to hop up higher on a slope too steep to climb    
        ent->s.origin = nosteporg;
        ent->velocity = nostepvel;
    }
}


///////////////////////////////////////////////////////////////////////////////
//	Physics_Walk
//
//	physics for MOVETYPE_WALK
//	FIXME:	add water detection here!!
///////////////////////////////////////////////////////////////////////////////

void	Physics_Walk (edict_t *ent)
{
	CVector	old_origin;

	old_origin = ent->s.origin;

	P_CheckGround (ent);

	if (ent->velocity.x == 0 && ent->velocity.y == 0 && ent->groundEntity)
	{
		P_TouchTriggers( ent );

		if (ent->think) 
		{
			P_RunThink (ent, p_run_think);
		}
		return;
	}

	P_GetWaterLevel (ent);

	if (!ent->groundEntity)
		P_AddGravity (ent);

	//	if ent is not moving in x/y plane, just account for up/down movement
/*
	if (ent->velocity [0] == 0 && ent->velocity [1] == 0 && !ent->groundEntity)
	{
		if (ent->velocity [2] != 0)
		{
			//	entity is falling, so determine if it will hit the floor in
			//	this frame
        	CVector	move;
			move = ent->velocity * p_frametime_scaled;
        	trace_t	trace;
			trace = P_PushEntity (ent, move);

			if (trace.allsolid || trace.startsolid)
			{
				if (trace.ent && trace.ent->className && ent->className)
				{
					//gi.conprintf ("Physics_Walk: %s is stuck inside of %s!\n", ent->className, trace.ent->className);
				}
			}

			if (trace.ent && trace.fraction < 1.0)
			{
				if (ent->velocity [2] < 0)
				{
					//	if falling, entity must have just hit the ground
					ent->groundEntity = trace.ent;
					ent->groundSurface = trace.surface;
					ent->groundEntity_linkcount = trace.ent->linkcount;
				}

				ent->velocity[2] = 0;
			}
		}
	}
	else
*/
	{
		P_BotWalkMove (ent);
		P_BotFriction (ent);

		gi.linkentity (ent);
	}

	if (ent->velocity.z <= 0)
	{
		//	P_TouchTriggers may have given entity upward velocity
		//	but they haven't had a chance to move up yet.  Checking
		//	for ground now will end up clearing that velocity
		P_CheckGround (ent);
	}

	//	run think function
	if (ent->think) 
	{
		P_RunThink (ent, p_run_think);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_Physics_Track
//
///////////////////////////////////////////////////////////////////////////////
void P_Physics_Track(edict_t *ent)
{
	trace_t		trace;
	CVector		move;
	CVector		old_origin;
	int			mask, blocked;

	// regular thinking
	if (ent->think) 
	{
		P_RunThink (ent, p_run_think);
		if (!ent->inuse)
		{
			//	entity was removed during its think
			return;
		}
	}

	//	slave, so move when the master moves
	if ( ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	old_origin = ent->s.origin;

	//	clamp velocity to p_maxvelocity
	P_CheckVelocity (ent);

	// move angles
	VectorMA(ent->s.angles, ent->avelocity, p_frametime_scaled, ent->s.angles);
	
	if (ent->svflags & SVF_PUSHED)
	{
		//	entity was pushed by something on last frame, so make it slide
		//	along planes rather than bounce off of them
		if (p_run_think)
		{
			ent->svflags -= SVF_PUSHED;
		}

		if (ent->clipmask)
		{
			mask = ent->clipmask;
		}
		else
		{
			mask = MASK_SOLID;
		}

		blocked = P_FlyMove (ent, p_frametime_scaled, mask);
		if (blocked)
		{
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}
		else 
		if (P_TestEntityPosition (ent))
		{
			//	can't go anywhere
			ent->s.origin = old_origin;
			ent->velocity = vec3_origin;
		}

		gi.linkentity (ent);
	}
	else
	{
		// move origin
		move = ent->velocity * p_frametime_scaled;
		trace = P_PushEntity (ent, move);

		if (!ent->inuse)
		{
			//	touch function may have removed entity
			return;
		}

		//	movement vector is down and entity has not moved
		if ((ent->velocity.z < 0 && 
			(ent->s.old_origin.x == ent->s.origin.x && ent->s.old_origin.y == ent->s.origin.y && 
			 ent->s.old_origin.z == ent->s.origin.z)))
		{
			ent->velocity = vec3_origin;
			ent->avelocity = vec3_origin;
			ent->groundEntity = g_edicts;
			ent->groundSurface = trace.surface;
			ent->groundEntity_linkcount = g_edicts[0].linkcount;
			return;
		}

		if (trace.fraction < 1)
		{
			ClipEntVelocity (ent, ent->velocity, trace.plane.normal, ent->velocity);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_RunEntity
//
//	Nelno:	similar to P_Physics in Quake 1 source
///////////////////////////////////////////////////////////////////////////////

void P_RunEntity (edict_t *ent, int run_think)
{
/*
	if( ent->flags & FL_MONSTER )
	{
		gi.Con_Printf("serverState time %f\n", serverState.time );
		gi.Con_Printf("level time %f\n", level.time );
		gi.Con_Printf("ent %s think %f\n", ent->className, ent->nextthink );
	}
*/
	p_run_think = run_think;

	switch (ent->movetype)
	{
		case	MOVETYPE_PUSH:
		case	MOVETYPE_STOP:
				P_Physics_Pusher (ent);
				break;

		case	MOVETYPE_NONE:
				P_Physics_None (ent);
				break;

		case	MOVETYPE_NOCLIP:
				P_Physics_Noclip (ent);
				break;

		case	MOVETYPE_STEP:
				P_Physics_Step (ent);
				break;

		case    MOVETYPE_SWIM:
        case	MOVETYPE_TOSS:
		case	MOVETYPE_BOUNCE:
		case	MOVETYPE_FLY:
		case	MOVETYPE_FLYMISSILE:
		case	MOVETYPE_BOUNCEMISSILE:
		case	MOVETYPE_HOVER:
				P_Physics_Toss (ent);
				break;

		case	MOVETYPE_FLOAT:
				P_Physics_Float (ent);
				break;

		case	MOVETYPE_WALK:
		case	MOVETYPE_WHEEL:
		case	MOVETYPE_HOP:
				P_GetCurrent( ent );
				Physics_Walk (ent);
//				P_Physics_Step (ent);
				break;
		
		case	MOVETYPE_TRACK:
				P_Physics_Track( ent );
				break;

		default:
				gi.Error ("P_RunEntity: bad movetype %i", (int)ent->movetype);			
	}
}
