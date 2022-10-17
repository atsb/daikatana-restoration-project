
#include "qcommon.h"
#include "client.h"

#define	STEPSIZE	18

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

#define	PMF_TIME_FLAGS	(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT)

typedef struct
{
	CVector		origin;			// full float precision
	CVector		velocity;		// full float precision

	CVector		forward, right, up;
	float		frametime;


	csurface_t	*groundsurface;
	cplane_t	groundplane;
	int			groundcontents;

	CVector		previous_origin;
	qboolean	ladder;
} pml_t;

pmove_t		*pm;
pml_t		pml;

static	int		pm_just_jumped;
//static	void	*pm_groundEntity;

// movement parameters
float	pm_stopspeed = 100;
float	pm_maxspeed = 320;
float	pm_duckspeed = 100;
float	pm_accelerate = 10;
float	pm_airaccelerate = 3;
float	pm_wateraccelerate = 10;
float	pm_friction = 6;
float	pm_waterfriction = 1;
float	pm_waterspeed = 400;

/*
  walking up a step should kill some velocity
*/

///////////////////////////////////////////////////////////////////////////////
//	PM_ClipVelocity
//
//	Slide off of the impacting object
//	returns the blocked flags (1 = floor, 2 = step / wall)
///////////////////////////////////////////////////////////////////////////////

#define	STOP_EPSILON	0.1

void PM_ClipVelocity (const CVector &in, const CVector &normal, CVector &out, float overbounce)
{
	float	backoff;
	float	change;
	
	backoff = DotProduct (in, normal) * overbounce;

	change = normal.x * backoff;
	out.x = in.x - change;
	if (out.x > -STOP_EPSILON && out.x < STOP_EPSILON)
	{
		out.x = 0;
	}
	change = normal.y * backoff;
	out.y = in.y - change;
	if (out.y > -STOP_EPSILON && out.y < STOP_EPSILON)
	{
		out.y = 0;
	}
	change = normal.z * backoff;
	out.z = in.z - change;
	if (out.z > -STOP_EPSILON && out.z < STOP_EPSILON)
	{
		out.z = 0;
	}
}




///////////////////////////////////////////////////////////////////////////////
//	PM_StepSlideMove
//
//	Each intersection will try to step over the obstruction instead of
//	sliding along it.
//
//	Returns a new origin, velocity, and contact entity
//	Does not modify any world state?
///////////////////////////////////////////////////////////////////////////////

#define	MIN_STEP_NORMAL	0.7		// can't step up onto very steep slopes
#define	MAX_CLIP_PLANES	5
void PM_StepSlideMove_ (void)
{
	int			bumpcount, numbumps;
	CVector		dir;
	float		d, rub;
	int			numplanes;
	CVector		planes[MAX_CLIP_PLANES];
	CVector		primal_velocity;
	trace_t	trace;
	CVector		end;
	float		time_left;
	int j, i;
	
	numbumps = 4;
	
	primal_velocity = pml.velocity;
	numplanes = 0;
	
	time_left = pml.frametime;

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		end = pml.origin + (pml.velocity);

		trace = pm->TraceBox(pml.origin, pm->mins, pm->maxs, end);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			pml.velocity.z = 0;	// don't build up falling damage
			return;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			pml.origin = trace.endpos;
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		// save entity for contact
		if (pm->numtouch < MAXTOUCH && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			pml.velocity = vec3_origin;
			break;
		}

		planes[numplanes] = trace.plane.normal;
		numplanes++;

// SCG[1/25/00]: quake 2 v1.19
#if 0
		//
		// modify velocity so it parallels all of the clip planes
		//
		if (numplanes == 1)
		{	// go along this plane
			dir = pml.velocity;
			dir.Normalize();
			rub = 1.0 + 0.5 * DotProduct (dir, planes[0]);

			// slide along the plane
			PM_ClipVelocity (pml.velocity, planes[0], pml.velocity, 1.01);
			// rub some extra speed off on xy axis
			// not on Z, or you can scrub down walls
			pml.velocity.x *= rub;
			pml.velocity.y *= rub;
			pml.velocity.z *= rub;
		}
		else 
		if (numplanes == 2)
		{	// go along the crease
			dir = pml.velocity;
			dir.Normalize();
			rub = 1.0 + 0.5 * DotProduct (dir, planes[0]);

			// slide along the plane
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, pml.velocity);
			pml.velocity = dir * d;

			// rub some extra speed off
			pml.velocity = pml.velocity * rub;
		}
		else
		{
//			Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
			pml.velocity = vec3_origin;
			break;
		}
#else
		for (int i=0 ; i<numplanes ; i++)
		{
			PM_ClipVelocity (pml.velocity, planes[i], pml.velocity, 1.01);
			for (int j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct (pml.velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	// go along this plane
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
				pml.velocity = vec3_origin;
				break;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, pml.velocity);
			pml.velocity = dir * d;
		}
#endif
		//
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (DotProduct (pml.velocity, primal_velocity) <= 0)
		{
			pml.velocity = vec3_origin;
			break;
		}
	}

	if (pm->s.pm_time)
	{
		pml.velocity = primal_velocity;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	PM_StepSlideMove
//
///////////////////////////////////////////////////////////////////////////////

void PM_StepSlideMove (void)
{
	CVector		start_o, start_v;
	CVector		down_o, down_v;
	trace_t		trace;
	float		down_dist, up_dist;
	CVector		delta;
	CVector		up, down;

	start_o = pml.origin;
	start_v = pml.velocity;

	PM_StepSlideMove_ ();

	down_o = pml.origin;
	down_v = pml.velocity;

	up = start_o;
	up.z += STEPSIZE;

	trace = pm->TraceBox (up, pm->mins, pm->maxs, up);
	if (trace.allsolid)
		return;		// can't step up

	// try sliding above
	pml.origin = up;
	pml.velocity = start_v;

	PM_StepSlideMove_ ();

	// push down the final amount
	down = pml.origin;
	down.z -= STEPSIZE;
	trace = pm->TraceBox (pml.origin, pm->mins, pm->maxs, down);
	if (!trace.allsolid)
	{
		pml.origin = trace.endpos;
	}

	delta = pml.origin - up;
	up_dist = DotProduct (delta, start_v);

	delta = down_o - start_o;
	down_dist = DotProduct (delta, start_v);

	if (down_dist > up_dist || trace.plane.normal[2] < MIN_STEP_NORMAL)
	{
		pml.origin = down_o;
		pml.velocity = down_v;
		return;
	}

}

///////////////////////////////////////////////////////////////////////////////
//	PM_FrictionCoefficient
//
//	returns the friction coefficient of the passed surface
///////////////////////////////////////////////////////////////////////////////

float	PM_FrictionCoefficient (csurface_t *surf)
{
	float	friction_coefficient;
	float	fLength = pml.velocity.Length();

	if (!surf)
		return	0;

	//	get the friction coefficient according to the surface flags
	if (surf->flags & SURF_WOOD)
		friction_coefficient = 0.97;
	else 
	if (surf->flags & SURF_METAL)
		friction_coefficient = 0.9;
	else 
	if (surf->flags & SURF_STONE)
		friction_coefficient = 1.1;
	else 
	if (surf->flags & SURF_GLASS)
		friction_coefficient = 0.8;
	else 
	if (surf->flags & SURF_ICE && fLength > 170 )
		friction_coefficient = 0.025;
	else 
	if (surf->flags & SURF_SNOW)
		friction_coefficient = 1.3;
	else
		friction_coefficient = 1.0;

/*
	if (surf->flags & SURF_WOOD)
		Com_Printf ("SURF_WOOD\n");
	else if (surf->flags & SURF_METAL)
		Com_Printf ("SURF_METAL\n");
	else if (surf->flags & SURF_STONE)
		Com_Printf ("SURF_STONE\n");
	else if (surf->flags & SURF_GLASS)
		Com_Printf ("SURF_GLASS\n");
	else if (surf->flags & SURF_ICE)
		Com_Printf ("SURF_ICE\n");
	else if (surf->flags & SURF_CARPET)	//	FIXME:	change to SURF_SNOW!
		Com_Printf ("SURF_SNOW\n");
	else
		Com_Printf ("standard friction\n");
*/
	return	friction_coefficient;
}

///////////////////////////////////////////////////////////////////////////////
//	PM_CheckForLedge
//
//	determines if there is a whole big enough to fall through in the 
//	direction of travel
///////////////////////////////////////////////////////////////////////////////

int	PM_CheckForLedge (void)
{
	CVector		dir, org, end;
	trace_t		trace;

	dir = pml.velocity;
	dir.Normalize();
	VectorMA (pml.origin, dir, pm->maxs.x * 2, org);
	end.x = org.x;
	end.y = org.y;
	end.z = org.z - STEPSIZE * 2;

	trace = pm->TraceBox (org, pm->mins, pm->maxs, end);
	if (trace.fraction == 1.0 && !trace.startsolid && !trace.allsolid || 
		(trace.contents & MASK_WATER))
		return	true;
	else
		return	false;
}

///////////////////////////////////////////////////////////////////////////////
//	PM_Friction
//
//	Handles both ground friction and water friction
///////////////////////////////////////////////////////////////////////////////

void PM_Friction (void)
{
	CVector	vel;
	float	speed, newspeed, control;
	float	friction;
	float	drop;
	
	vel = pml.velocity;
	
	speed = sqrt(vel.x*vel.x + vel.y*vel.y + vel.z*vel.z);
	if (speed < 1)
	{
		vel.x = 0;
		vel.y = 0;
		pml.velocity = vel;
		return;
	}

	drop = 0;

	// apply ground friction
/*
	if ((pm->groundEntity && pml.groundsurface && !(pml.groundsurface->flags & SURF_ICE) ) || (pml.ladder) )
	{
		friction = pm_friction;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop = control*friction*pml.frametime;

		Com_Printf ("drop = %.3f\n", drop);
	}
*/
	//	apply ladder friction
	if (pml.ladder)
	{
		friction = pm_friction;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;
	}
	//	apply ground friction based on surface flags
	else 
	if (pm->groundEntity)
	{
		if (!pml.groundsurface)
		{
			friction = 1.0;
		}
		else
		{
			friction = PM_FrictionCoefficient(pml.groundsurface);
		}

		if ( !pm->cmd.forwardmove && !pm->cmd.sidemove )
		{
			if ( PM_CheckForLedge() )
			{
				friction *= 4.0;
			}
		}

		friction = friction * pm_friction;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop = control * pml.frametime * (friction * sv_frictionfactor->value);
	}

// apply water friction
	if (pm->waterlevel && !pml.ladder)
	{
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
	{
		newspeed = 0;
	}
	newspeed /= speed;

	vel = vel * newspeed;

	pml.velocity = vel;

	// limit lateral movement while on ladder
	if (pml.ladder)
	{
		pml.velocity.x *= 0.5f;
		pml.velocity.y *= 0.5f;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	PM_Accelerate
//
//	Handles user intended acceleration
///////////////////////////////////////////////////////////////////////////////

void PM_Accelerate (const CVector &wishdir, float wishspeed, float accel)
{
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
	{
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	if (pml.groundsurface && pml.groundsurface->flags & SURF_ICE)
	{
		//	some surfaces are slippery
		accelspeed *= PM_FrictionCoefficient (pml.groundsurface);
	}

	pml.velocity.x += accelspeed*wishdir.x;
	pml.velocity.y += accelspeed*wishdir.y;
	pml.velocity.z += accelspeed*wishdir.z;
}


///////////////////////////////////////////////////////////////////////////////
//	PM_AddCurrents
//
///////////////////////////////////////////////////////////////////////////////

#define	LADDER_VERTICAL_VELOCITY	200
#define	LADDER_HORIZONTAL_VELOCITY	75

void PM_AddCurrents (CVector &wishvel)
{
	CVector	v;
	float	s;

	//
	// account for ladders
	//

	if (pml.ladder && fabs(pml.velocity.z) <= LADDER_VERTICAL_VELOCITY)
	{
		if ((pm->viewangles.pitch <= -15) && (pm->cmd.forwardmove > 0))
			wishvel.z = LADDER_VERTICAL_VELOCITY;
		else 
		if ((pm->viewangles.pitch >= 15) && (pm->cmd.forwardmove > 0))
			wishvel.z = -LADDER_VERTICAL_VELOCITY;
		else 
		if (pm->cmd.upmove > 0)
			wishvel.z = LADDER_VERTICAL_VELOCITY;
		else 
		if (pm->cmd.upmove < 0)
			wishvel.z = -LADDER_VERTICAL_VELOCITY;
		else
			wishvel.z = 0;

		// limit horizontal speed when on a ladder
		if (wishvel.x < -LADDER_HORIZONTAL_VELOCITY)
			wishvel.x = -LADDER_HORIZONTAL_VELOCITY;
		else 
		if (wishvel.x > LADDER_HORIZONTAL_VELOCITY)
			wishvel.x = LADDER_HORIZONTAL_VELOCITY;

		if (wishvel.y < -LADDER_HORIZONTAL_VELOCITY)
			wishvel.y = -LADDER_HORIZONTAL_VELOCITY;
		else 
		if (wishvel.y > LADDER_HORIZONTAL_VELOCITY)
			wishvel.y = LADDER_HORIZONTAL_VELOCITY;
	}


	//
	// add water currents
	//

	if (pm->watertype & MASK_CURRENT)
	{
		v.Zero();

		if (pm->watertype & CONTENTS_CURRENT_0)
			v.x += 1;
		if (pm->watertype & CONTENTS_CURRENT_90)
			v.y += 1;
		if (pm->watertype & CONTENTS_CURRENT_180)
			v.x -= 1;
		if (pm->watertype & CONTENTS_CURRENT_270)
			v.y -= 1;
		if (pm->watertype & CONTENTS_CURRENT_UP)
			v.z += 1;
		if (pm->watertype & CONTENTS_CURRENT_DOWN)
			v.z -= 1;

		s = pm_waterspeed;
		if ((pm->waterlevel == 1) && (pm->groundEntity))
			s /= 2;

		VectorMA (wishvel, v, s, wishvel);
	}

	//
	// add conveyor belt velocities
	//

	if (pm->groundEntity)
	{
		v.Zero();

		if (pml.groundcontents & CONTENTS_CURRENT_0)
			v.x += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_90)
			v.y += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_180)
			v.x -= 1;
		if (pml.groundcontents & CONTENTS_CURRENT_270)
			v.y -= 1;
		if (pml.groundcontents & CONTENTS_CURRENT_UP)
			v.z += 1;
		if (pml.groundcontents & CONTENTS_CURRENT_DOWN)
			v.z -= 1;

		VectorMA (wishvel, v, 100 /* pm->groundEntity->speed */, wishvel);
	}
}


///////////////////////////////////////////////////////////////////////////////
//	PM_WaterMove
//
///////////////////////////////////////////////////////////////////////////////

void PM_WaterMove (void)
{
	CVector	wishvel;
	float	wishspeed;
	CVector	wishdir;

//
// user intentions
//
	wishvel.x = pml.forward.x*pm->cmd.forwardmove + pml.right.x*pm->cmd.sidemove;
	wishvel.y = pml.forward.y*pm->cmd.forwardmove + pml.right.y*pm->cmd.sidemove;
	wishvel.z = pml.forward.z*pm->cmd.forwardmove + pml.right.z*pm->cmd.sidemove;

	if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
		wishvel.z -= 60;		// drift towards bottom
	else
		wishvel.z += pm->cmd.upmove;

	PM_AddCurrents (wishvel);

	wishdir = wishvel;
	wishspeed = wishdir.Normalize();

	if (wishspeed > pm_maxspeed)
	{
		wishvel = wishvel * (pm_maxspeed/wishspeed);
		wishspeed = pm_maxspeed;
	}
	wishspeed *= 0.5;

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	PM_StepSlideMove ();
}

///////////////////////////////////////////////////////////////////////////////
//	PM_SurfaceAcceleration
//
///////////////////////////////////////////////////////////////////////////////

float	PM_SurfaceAcceleration (csurface_t *surf)
{
	float	scale = 1.0;

	if (!surf)
		return	0;

	//	get the friction coefficient according to the surface flags
	if (surf->flags == 0)
		scale = 1.0;
	else if (surf->flags & SURF_WOOD)
		scale = 1.0;
	else if (surf->flags & SURF_METAL)
		scale = 1.0;
	else if (surf->flags & SURF_STONE)
		scale = 1.0;
	else if (surf->flags & SURF_GLASS)
		scale = 1.1;
	else if (surf->flags & SURF_ICE)
		scale = 1.5;
	else if (surf->flags & SURF_SNOW)
		scale = 1.0;

	return	scale;
}

///////////////////////////////////////////////////////////////////////////////
//	PM_AirMove
//
///////////////////////////////////////////////////////////////////////////////

void PM_AirMove (void)
{
	CVector		wishvel;
	float		fmove, smove;
	CVector		wishdir;
	float		wishspeed;
	float		maxspeed;
	
	trace_t		trace;
	CVector		saved_vel;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
	pml.forward.z = 0;
	pml.right.z = 0;
	pml.forward.Normalize();
	pml.right.Normalize();

	wishvel.x = pml.forward.x*fmove + pml.right.x*smove;
	wishvel.y = pml.forward.y*fmove + pml.right.y*smove;
	wishvel.z = 0;

	PM_AddCurrents (wishvel);

	wishdir = wishvel;
	wishspeed = wishdir.Normalize();

//
// clamp to server defined max speed
//
	maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if (wishspeed > maxspeed)
	{
		wishvel = wishvel * (maxspeed/wishspeed);
		wishspeed = maxspeed;
	}
	
	if ( pml.ladder )
	{
		PM_Accelerate (wishdir, wishspeed, pm_accelerate);
		if (!wishvel.z)
		{
			if (pml.velocity.z > 0)
			{
				pml.velocity.z -= pm->s.gravity * pml.frametime;
				if (pml.velocity.z < 0)
					pml.velocity.z  = 0;
			}
			else
			{
				pml.velocity.z += pm->s.gravity * pml.frametime;
				if (pml.velocity.z > 0)
					pml.velocity.z  = 0;
			}
		}
		PM_StepSlideMove ();
	}
	else 
	if ( pm->groundEntity )
	{	// walking on ground
		PM_Accelerate (wishdir, wishspeed, pm_accelerate);

		pml.velocity.z = 0;
		if (!pml.velocity.x && !pml.velocity.y)
			return;
		PM_StepSlideMove ();
	}
	else
	{	
		// not on ground, so little effect on velocity
		PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

		saved_vel = pml.velocity;

		// add gravity
		pml.velocity.z -= pm->s.gravity * pml.frametime;
		PM_StepSlideMove ();

		if (pm_just_jumped)
		{
			//	push groundEntity away
			// CHECKME: ISP
			//trace = pm->TraceLine(saved_vel, vec3_origin );//(float *)pm_groundEntity);
		}
	}
}

// amw 4.27.99 - quake2 version 1.19

/*
=============
PM_CatagorizePosition
=============
*/
void PM_CatagorizePosition (void)
{
	CVector		point;
	int			cont;
	trace_t		trace;
	int			sample1;
	int			sample2;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid	
	point.x = pml.origin.x;
	point.y = pml.origin.y;
	point.z = pml.origin.z - 0.25;
	if (pml.velocity.z > 180) //!!ZOID changed from 100 to 180 (ramp accel)
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundEntity = NULL;
	}
	else
	{
		trace = pm->TraceBox (pml.origin, pm->mins, pm->maxs, point);
		pml.groundplane = trace.plane;
		pml.groundsurface = trace.surface;
		pml.groundcontents = trace.contents;

		if (!trace.ent || (trace.plane.normal.z < 0.7 && !trace.startsolid) )
		{
			pm->groundEntity = NULL;
			pm->s.pm_flags &= ~PMF_ON_GROUND;
		}
		else
		{
			pm->groundEntity = trace.ent;

			// hitting solid ground will end a waterjump
			if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
			{
				pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
				pm->s.pm_time = 0;
			}

			if (! (pm->s.pm_flags & PMF_ON_GROUND) )
			{	// just hit the ground
				pm->s.pm_flags |= PMF_ON_GROUND;
				// don't do landing time if we were just going down a slope
				if (pml.velocity.z < -200)
				{
					pm->s.pm_flags |= PMF_TIME_LAND;
					// don't allow another jump for a little while
					if (pml.velocity.z < -400)
						pm->s.pm_time = 25;	
					else
						pm->s.pm_time = 18;
				}
			}
		}

// amw 4.27.98 - obviously the id guys were playing with this.. this eliminates the
// last of the gib on angled surface bugs...
#if 1
		if (trace.fraction < 1.0 && trace.ent && pml.velocity.z < 0)
			pml.velocity.z = 0;
#endif

		if (pm->numtouch < MAXTOUCH && trace.ent )
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
	}

//
// get waterlevel, accounting for ducking
//
	pm->waterlevel = 0;
	pm->watertype = 0;

	sample2 = pm->viewheight - pm->mins.z;
	sample1 = sample2 / 2;

	point.z = pml.origin.z + pm->mins.z + 1;	
	cont = pm->pointcontents (point);

	if (cont & MASK_WATER)
	{
		pm->watertype = cont;
		pm->waterlevel = 1;
		point.z = pml.origin.z + pm->mins.z + sample1;
		cont = pm->pointcontents (point);
		if (cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			point.z = pml.origin.z + pm->mins.z + sample2;
			cont = pm->pointcontents (point);
			if (cont & MASK_WATER)
				pm->waterlevel = 3;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
//	PM_CheckJump
//
//	checks for pressing of jump button.  This needs to be done in world.dll,
//	client.cpp.  Unfortunately, this code is also used by the client to predict
//	player movement, which means the client code would have to call it from
//	physics.dll.  Right now, client does not have access to the game_export_t
//	global ge.  Is this because it doesn't load the physics.dll???
//
//	Q2FIXME:	try to fix this after E3...
///////////////////////////////////////////////////////////////////////////////

void PM_CheckJump (void)
{
	if (pm->s.pm_flags & PMF_TIME_LAND)
	{	// hasn't been long enough since landing to jump again
		return;
	}

	if (pm->cmd.upmove < 10)
	{	// not holding jump
		pm->s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}

	// must wait for jump to be released
	if (pm->s.pm_flags & PMF_JUMP_HELD)
     		return;

	// if dead just set the flag
	if (pm->s.pm_type == PM_DEAD)
	{
//		pm->s.pm_flags |= PMF_JUMP_HELD;
		return;
	}

	if (pm->waterlevel >= 2)
	{	// swimming, not jumping
		pm->groundEntity = NULL;

		if (pml.velocity.z <= -300)
			return;

		if (pm->watertype == CONTENTS_WATER)
			pml.velocity.z = 100;
		else 
		if (pm->watertype == CONTENTS_SLIME)
			pml.velocity.z = 80;
		else
			pml.velocity.z = 50;
		return;
	}

	if (pm->groundEntity == NULL)
		return;		// in air, so no effect

	pm->s.pm_flags |= PMF_JUMP_HELD;

	pm_just_jumped = true;

	pm->groundEntity = NULL;
	pml.velocity.z += 270;
	if (pml.velocity.z < 270)
		pml.velocity.z = 270;

}

///////////////////////////////////////////////////////////////////////////////
//	PM_CheckSpecialMovement
//
///////////////////////////////////////////////////////////////////////////////

void PM_CheckSpecialMovement (void)
{
	CVector	spot;
	int		cont;
	CVector	flatforward;
	trace_t	trace;

	if (pm->s.pm_time)
		return;

	pml.ladder = false;

	// check for ladder
	flatforward.x = pml.forward.x;
	flatforward.y = pml.forward.y;
	flatforward.z = 0;
	flatforward.Normalize();

	VectorMA (pml.origin, flatforward, 1, spot);
	trace = pm->TraceBox (pml.origin, pm->mins, pm->maxs, spot);
	if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
		pml.ladder = true;
	else
	{
		VectorMA (pml.origin, flatforward, -1, spot);
		trace = pm->TraceBox (pml.origin, pm->mins, pm->maxs, spot);
		if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
			pml.ladder = true;
	}


	// check for water jump
	if (pm->waterlevel != 2)
		return;

    // only check if you are moving in the direction you are facing
	CVector tempVel(pm->s.velocity[0], pm->s.velocity[1], pm->s.velocity[2]);

    if (flatforward.DotProduct(tempVel)<0)
        return;

	VectorMA (pml.origin, flatforward, 30, spot);
	spot.z += 4;
	cont = pm->pointcontents (spot);
	if (!(cont & CONTENTS_SOLID))
		return;

	spot.z += 16;
	cont = pm->pointcontents (spot);
	if (cont & (MASK_SOLID | MASK_WATER | CONTENTS_PLAYERCLIP))
	{
		return;
	}
	// jump out of water
	//pml.velocity = pml.forward * 25;
    pml.velocity.x = 0;
    pml.velocity.y = 0;
	pml.velocity.z = 300;

	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}


///////////////////////////////////////////////////////////////////////////////
//	PM_NoclipMove
//
///////////////////////////////////////////////////////////////////////////////

void PM_NoclipMove (void)
{
	float	speed, drop, friction, control, newspeed;
	float	currentspeed, addspeed, accelspeed;
	CVector		wishvel;
	float		fmove, smove;
	CVector		wishdir;
	float		wishspeed;

	pm->viewheight = 22;

	// friction

	speed = pml.velocity.Length();
	if (speed < 1)
	{
		pml.velocity = vec3_origin;
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		pml.velocity = pml.velocity * newspeed;
	}

	// accelerate
	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
	pml.forward.Normalize();
	pml.right.Normalize();

	wishvel.x = pml.forward.x*fmove + pml.right.x*smove;
	wishvel.y = pml.forward.y*fmove + pml.right.y*smove;
	wishvel.z = pml.forward.z*fmove + pml.right.z*smove;
	wishvel.z += pm->cmd.upmove;

	wishdir = wishvel;
	wishspeed = wishdir.Normalize();

	//
	// clamp to server defined max speed
	//
	//	Nelno:	no clipping is twice as fast as normal speed
	if (wishspeed > pm_maxspeed * 2.0)
	{
		wishvel = wishvel * (pm_maxspeed/(wishspeed * 2.0));
		wishspeed = pm_maxspeed * 2.0;
	}

	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = pm_accelerate*pml.frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity.x += accelspeed*wishdir.x;
	pml.velocity.y += accelspeed*wishdir.y;
	pml.velocity.z += accelspeed*wishdir.z;

	// move
	VectorMA (pml.origin, pml.velocity, pml.frametime, pml.origin);
}

///////////////////////////////////////////////////////////////////////////////
//	PM_CheckDuck
//
//	Sets mins, maxs, and pm->viewheight
///////////////////////////////////////////////////////////////////////////////

void PM_CheckDuck (void)
{
	trace_t	trace;

	pm->mins.x = -16;
	pm->mins.y = -16;

	pm->maxs.x = 16;
	pm->maxs.y = 16;

	if (pm->s.pm_type == PM_GIB)
	{
		pm->mins.z = 0;	  //CHECKME - is this the reason player gibs still have bounding boxes?
		pm->maxs.z = 16;
		pm->viewheight = 8;
		return;
	}

	pm->mins.z = -24;

	if (pm->s.pm_type == PM_DEAD)
	{
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else 
	if (pm->cmd.upmove < 0)// /* 3.14 stuff */ && (pm->s.pm_flags & PMF_ON_GROUND))
	{	// duck
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else
	{	// stand up if possible
		if (pm->s.pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->maxs.z = 32;
			trace = pm->TraceBox (pml.origin, pm->mins, pm->maxs, pml.origin);
			if (!trace.allsolid)
				pm->s.pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->s.pm_flags & PMF_DUCKED)
	{
		pm->maxs.z = 4;
		pm->viewheight = -2;
	}
	else// if (!(pm->s.pm_flags & PMF_CAMERAMODE))
	{
		pm->maxs.z = 32;
		pm->viewheight = 22;
	}
}


///////////////////////////////////////////////////////////////////////////////
//	PM_DeadMove
//
///////////////////////////////////////////////////////////////////////////////

void PM_DeadMove (void)
{
	float	forward;

	if (!pm->groundEntity)
		return;

	// extra friction

	forward = pml.velocity.Length();
	forward -= 20;
	if (forward <= 0)
	{
		pml.velocity.Zero();
	}
	else
	{
		pml.velocity.Normalize();
		pml.velocity = pml.velocity * forward;
	}
}


qboolean	PM_GoodPosition (void)
{
	trace_t	trace;
	CVector	origin, end;

	if (pm->s.pm_type == PM_SPECTATOR)
		return true;

	origin.x = end.x = pm->s.origin[0]*0.125;
	origin.y = end.y = pm->s.origin[1]*0.125;
	origin.z = end.z = pm->s.origin[2]*0.125;

	trace = pm->TraceBox(origin, pm->mins, pm->maxs, end);

	return !trace.allsolid;
}

///////////////////////////////////////////////////////////////////////////////
//	PM_SnapPosition
//
//	On exit, the origin will have a value that is pre-quantized to the 0.125
//	precision of the network channel and in a valid position.
///////////////////////////////////////////////////////////////////////////////

void PM_SnapPosition (void)
{
	CVector	sign;
	int		j, bits;
	short	base[3];
	// try all single bits first
	static int jitterbits[8] = {0,4,1,2,3,5,6,7};

	// snap velocity to eigths
	pm->s.velocity[0] = (int)(pml.velocity.x*8);
	pm->s.velocity[1] = (int)(pml.velocity.y*8);
	pm->s.velocity[2] = (int)(pml.velocity.z*8);

	if (pml.origin.x >= 0)
		sign.x = 1;
	else 
		sign.x = -1;
	pm->s.origin[0] = (int)(pml.origin.x*8);
	if (pm->s.origin[0]*0.125 == pml.origin.x)
		sign.x = 0;

	if (pml.origin.y >= 0)
		sign.y = 1;
	else 
		sign.y = -1;
	pm->s.origin[1] = (int)(pml.origin.y*8);
	if (pm->s.origin[1]*0.125 == pml.origin.y)
		sign.y = 0;

	if (pml.origin.z >= 0)
		sign.z = 1;
	else 
		sign.z = -1;
	pm->s.origin[2] = (int)(pml.origin.z*8);
	if (pm->s.origin[2]*0.125 == pml.origin.z)
		sign.z = 0;

	base[0] = pm->s.origin[0];
	base[1] = pm->s.origin[1];
	base[2] = pm->s.origin[2];

	// try all combinations
	for (j=0 ; j<8 ; j++)
	{
		bits = jitterbits[j];
		pm->s.origin[0] = base[0];
		pm->s.origin[1] = base[1];
		pm->s.origin[2] = base[2];

		if (bits & 1 )
			pm->s.origin[0] += sign.x;
		if (bits & 2 )
			pm->s.origin[1] += sign.y;
		if (bits & 4 )
			pm->s.origin[2] += sign.z;

		if (PM_GoodPosition ())
		{
			return;
		}
	}

	// go back to the last position
	pm->s.origin[0] = pml.previous_origin[0];
	pm->s.origin[1] = pml.previous_origin[1];
	pm->s.origin[2] = pml.previous_origin[2];
//	Com_DPrintf ("using previous_origin\n");
}

///////////////////////////////////////////////////////////////////////////////
//	PM_InitialSnapPosition
//
///////////////////////////////////////////////////////////////////////////////
#if 0
void PM_InitialSnapPosition (void)
{
	int		x, y, z;
	short	base[3];

	base = pm->s.origin;

	for (z=1 ; z>=-1 ; z--)
	{
		pm->s.origin.z = base[2] + z;
		for (y=1 ; y>=-1 ; y--)
		{
			pm->s.origin.y = base[1] + y;
			for (x=1 ; x>=-1 ; x--)
			{
				pm->s.origin.x = base[0] + x;
				if (PM_GoodPosition ())
				{
					pml.origin.x = pm->s.origin[0]*0.125;
					pml.origin.y = pm->s.origin[1]*0.125;
					pml.origin.z = pm->s.origin[2]*0.125;
					pml.previous_origin = pm->s.origin;
					return;
				}
			}
		}
	}

	Com_DPrintf ("Bad InitialSnapPosition\n");
}
#else	  //amw : from Q2_v317   .. this fixes the bug where you slide southwest on a train
/*
================
PM_InitialSnapPosition

================
*/
void PM_InitialSnapPosition(void)
{
	int        x, y, z;
	short      base[3];
	static int offset[3] = { 0, -1, 1 };

	base[0] = pm->s.origin[0];
	base[1] = pm->s.origin[1];
	base[2] = pm->s.origin[2];

	for ( z = 0; z < 3; z++ ) 
	{
		pm->s.origin[2] = base[2] + offset[ z ];
		for ( y = 0; y < 3; y++ ) 
		{
			pm->s.origin[1] = base[1] + offset[ y ];
			for ( x = 0; x < 3; x++ ) 
			{
				pm->s.origin[0] = base[0] + offset[ x ];
				if (PM_GoodPosition ()) 
				{
					pml.origin.x = pm->s.origin[0]*0.125;
					pml.origin.y = pm->s.origin[1]*0.125;
					pml.origin.z = pm->s.origin[2]*0.125;
					pml.previous_origin.x = pm->s.origin[0];
					pml.previous_origin.y = pm->s.origin[1];
					pml.previous_origin.z = pm->s.origin[2];
					return;
				}
			}
		}
	}

	Com_DPrintf ("Bad InitialSnapPosition\n");
}

#endif


///////////////////////////////////////////////////////////////////////////////
//	PM_ClampAngles
//
///////////////////////////////////////////////////////////////////////////////

void PM_ClampAngles (void)
{
    /*
	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles.y = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
		pm->viewangles.x = 0;
		pm->viewangles.z = 0;
	}
	else
	{
        frame_t *oldframe;
        player_state_t *ops;
        int i;
        static CVector old_delta_angles, new_delta_angles;
        CVector cur_delta_angles;

        i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	    oldframe = &cl.frames[i];
    	
        if (oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid)
	    	oldframe = &cl.frame;		// previous frame was dropped or invalid

        //get the old player state.
	    ops = &oldframe->playerstate;

        
        // interpolate the delta_angles, which come from rotating bmodels, etc.
        // the delta_angles come in a frame out of sync, so deal with it hackily...

//        if(cl.ClientEntityManager->NetClient)
        {
            for (i=0;i<3;i++)
                cur_delta_angles[i] = LerpAngle(old_delta_angles[i], new_delta_angles[i], cl.lerpfrac);
        }

        // circularly clamp the angles with deltas
		pm->viewangles.x = SHORT2ANGLE(pm->cmd.angles[0]) + cur_delta_angles[0];
		pm->viewangles.y = SHORT2ANGLE(pm->cmd.angles[1]) + cur_delta_angles[1];
		pm->viewangles.z = SHORT2ANGLE(pm->cmd.angles[2]) + cur_delta_angles[2];

        // update the delta angles here, since they're 1 frame old
        for (i=0;i<3;i++)
        {
            new_delta_angles[i] = SHORT2ANGLE(pm->s.delta_angles[i]);
            old_delta_angles[i] = SHORT2ANGLE(ops->pmove.delta_angles[i]);
        }
        
		// don't let the player look up or down more than 90 degrees
		if (pm->viewangles.x > 89 && pm->viewangles.x < 180)
			pm->viewangles.x = 89;
		else 
		if (pm->viewangles.x < 271 && pm->viewangles.x >= 180)
			pm->viewangles.x = 271;
	}
	AngleToVectors (pm->viewangles, pml.forward, pml.right, pml.up);
*/

	short	temp;
	int		i;

	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
		pm->viewangles[PITCH] = 0;
		pm->viewangles[ROLL] = 0;
	}
	else
	{
		// circularly clamp the angles with deltas
		for (i=0 ; i<3 ; i++)
		{
			temp = pm->cmd.angles[i] + pm->s.delta_angles[i];
			pm->viewangles[i] = SHORT2ANGLE(temp);
		}

		// don't let the player look up or down more than 90 degrees
		if (pm->viewangles[PITCH] > 89 && pm->viewangles[PITCH] < 180)
			pm->viewangles[PITCH] = 89;
		else if (pm->viewangles[PITCH] < 271 && pm->viewangles[PITCH] >= 180)
			pm->viewangles[PITCH] = 271;
	}
	AngleToVectors (pm->viewangles, pml.forward, pml.right, pml.up);


}

///////////////////////////////////////////////////////////////////////////////
//	PM_RunCounter
///////////////////////////////////////////////////////////////////////////////

void	PM_RunCounter (void)
{
	int		msec;

/*
	if (pm->s.pm_flags & PMF_TIME_OFFGROUND)
	{
		Com_Printf ("pm_time = %i\n", pm->s.pm_time);
		//	count up, milliseconds since last jump
		pm->s.pm_time += msec;
		if (pm->s.pm_time > pm_moveinairtime)
		{
			pm->s.pm_flags &= ~PMF_TIME_FLAGS;
			pm->s.pm_time = 0;
		}
	}
	else 
*/	
	if (pm->s.pm_time)
	{
		msec = pm->cmd.msec >> 3;
		if (!msec)
			msec = 1;
		//	count down
		if ( msec >= pm->s.pm_time) 
		{
			pm->s.pm_flags &= ~PMF_TIME_FLAGS;
			pm->s.pm_time = 0;
		}
		else
			pm->s.pm_time -= msec;
	}
	
}

///////////////////////////////////////////////////////////////////////////////
//	Pmove
//
//	Can be called by either the server or the client
///////////////////////////////////////////////////////////////////////////////

void Pmove (pmove_t *pmove)
{
	pm = pmove;

	pm_just_jumped = 0;
	//pm_groundEntity = NULL;
	pm_maxspeed = pmove->max_speed;

	// clear results
	pm->numtouch = 0;
	pm->viewangles.Zero();
	//	Nelno:	don't zero this, idiot
	pm->viewheight = 0;
	pm->groundEntity = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// convert origin and velocity to float values
	pml.origin.x = pm->s.origin[0]*0.125;
	pml.origin.y = pm->s.origin[1]*0.125;
	pml.origin.z = pm->s.origin[2]*0.125;

	pml.velocity.x = pm->s.velocity[0]*0.125;
	pml.velocity.y = pm->s.velocity[1]*0.125;
	pml.velocity.z = pm->s.velocity[2]*0.125;

	// save old org in case we get stuck
	pml.previous_origin.x = pm->s.origin[0];
	pml.previous_origin.y = pm->s.origin[1];
	pml.previous_origin.z = pm->s.origin[2];

	pml.frametime = pm->cmd.msec * 0.001;

	pm->groundSurface = pml.groundsurface;

	if( pm->s.pm_type != PM_NOINPUT && 
		pm->s.pm_type != PM_DEAD &&
		pm->s.pm_type != PM_FREEZEDEATH)
		//	don't allow client to change viewing angle
		PM_ClampAngles ();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		pm->cmd.upmove *= 2.0;
		pm->cmd.forwardmove *= 2.0;

		PM_NoclipMove ();
		PM_SnapPosition ();

		pm->groundSurface = NULL;
		return;
	}

	if (pm->s.pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}

	if ( pm->s.pm_type == PM_FREEZE || pm->s.pm_type == PM_FREEZEDEATH )
		return;		// no movement at all

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	if (pm->snapinitial)
		PM_InitialSnapPosition ();

	// set groundEntity, watertype, and waterlevel
	PM_CatagorizePosition ();

	if (pm->s.pm_type == PM_DEAD)
		PM_DeadMove ();

	PM_CheckSpecialMovement ();

	PM_RunCounter ();

	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{	// teleport pause stays exactly in place
	}
	//	Nelno:	commented out to give player control in a water jump
// SCG[1/25/00]: back to Quake code
	else if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{	// waterjump has no control, but falls
		pml.velocity[2] -= pm->s.gravity * pml.frametime;
		if (pml.velocity[2] < 0)
		{	// cancel as soon as we are falling down again
			pm->s.pm_flags &= ~PMF_TIME_FLAGS;
			pm->s.pm_time = 0;
		}

		PM_StepSlideMove ();
	}
	else
	{
		PM_CheckJump ();

		PM_Friction ();

		if (pm->waterlevel >= 2)
			PM_WaterMove ();
		else {
			CVector angles;

			angles = pm->viewangles;
			if (angles[PITCH] > 180)
				angles[PITCH] = angles[PITCH] - 360;
			angles[PITCH] /= 3;

			AngleToVectors (angles, pml.forward, pml.right, pml.up);

			PM_AirMove ();
		}
	}
// SCG[1/25/00]: non quake code
/*
	else if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{
		if (pml.velocity[2] < 0)
		{	// cancel as soon as we are falling down again
			pm->s.pm_flags &= ~PMF_TIME_FLAGS;
			pm->s.pm_time = 0;
		}
	}

		if (!(pm->s.pm_flags & PMF_TIME_WATERJUMP))
		{
			PM_CheckJump ();

			PM_Friction ();
		}

		if ((pm->waterlevel >= 2) && !(pm->s.pm_flags & PMF_TIME_WATERJUMP))
		{
			PM_WaterMove ();
		}
		else 
		{
			CVector	angles = pm->viewangles;
			if (angles.x > 180)
				angles.x = angles.x - 360;
			angles.x /= 3;

			AngleToVectors (angles, pml.forward, pml.right, pml.up);

			PM_AirMove ();
		}
	}
#endif
*/

	// set groundEntity, watertype, and waterlevel for final spot
	PM_CatagorizePosition ();

	PM_SnapPosition ();

	pm->groundSurface = pml.groundsurface;
}

