//////////////////////////////////////////////////////////////////////////
//	Physics routines for MOVETYPE_FLOAT
//////////////////////////////////////////////////////////////////////////

#include	"p_global.h"
#include	"p_user.h" 

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	P_PercentSubmerged
//
//	Finds percent of an object that is submerged using a single trace
//
//	sets contents and percent_submerged
//	1.0 == completely submerged
//	0.5 == 50% submerged
//	0.0 == not in water
///////////////////////////////////////////////////////////////////////////////

void	P_PercentSubmerged (userEntity_t *self, int *contents, float *percent_submerged)
{
	CVector	top, bottom;
	float	bbox_height, water_height;
//	float	add_top, add_bottom;
	trace_t	tr;

//	add_top = self->s.maxs.z + (self->s.maxs.z / 2);
//	add_bottom = self->s.mins.z / 4;

	//	compute top of object bounding box
	// SCG[1/22/00]: huh?
//	self->s.origin, top = self->s.origin;
	top = self->s.origin;
	top.z += self->s.maxs.z;

	//	compute bottom of object bounding box
	bottom = self->s.origin;
	bottom.z += (self->s.mins.z / 4);

	//	total height of bounding box
	bbox_height = (self->s.maxs.z - (self->s.mins.z / 4));

	//	trace from top of bounding box to bottom of bounding box, stop at water
	tr = gi.TraceLine (top, bottom, NULL, MASK_WATER);
	if (tr.allsolid && tr.startsolid)
	{
		water_height = bbox_height;
	}
	else
	{
		water_height = bbox_height - (bbox_height * tr.fraction);
	}

	//	determine the percentage submerged
	if (water_height <= 0)
	{
		*(float *) percent_submerged = 0.0;
		*(int *) contents = CONTENTS_EMPTY;
	}
	else 
	if (water_height >= bbox_height)
	{
		*(float *) percent_submerged = 1.0;
		*(int *) contents = tr.contents;
	}
	else
	{
		*(float *) percent_submerged = 1 / (bbox_height / water_height);
		*(int *) contents = tr.contents;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_PhysicsFloat
//
//	returns:
//		percent submerged
///////////////////////////////////////////////////////////////////////////////

float	P_PhysicsFloat (userEntity_t *self)
{
	float		percent_submerged, g, ratio;
	float		density, fluid_density = 1.0;
	float		velocity_scale;
	int			contents;

	//	find percent submerged
	self->submerged_func (self, &contents, &percent_submerged);

	//	compute object properties
	density = self->mass / self->volume;

	//	recalc ratio if contents changed
	if (contents != self->watertype || self->svflags & SVF_NEWSPAWN)
	{
		if (contents & CONTENTS_LAVA)
			fluid_density = 2.0;
		else if (contents & CONTENTS_SLIME)
			fluid_density = 1.2;
	}
	
	ratio = fluid_density / density;
	self->watertype = contents;
	
	g = p_gravity->value;
	if (self->gravity != 0.0)
		g *= self->gravity;

	if (percent_submerged > 0.0)
	{
		self->velocity.z += (g / fluid_density) * (p_frametime_scaled * (ratio * percent_submerged - 1.0));

//		if (fabs (self->velocity [2]) > self->velocity_cap)
//			self->velocity [2] *= self->dissipate;

		//	slow velocity according to fluid_density
		velocity_scale = (0.9875 + ((fluid_density - 1.0) * 0.7125)) / fluid_density;
		self->velocity.x *= velocity_scale * self->dissipate;
		self->velocity.y *= velocity_scale * self->dissipate;
		if (fabs (self->velocity.z) > self->velocity_cap)
		{
			self->velocity.z *= velocity_scale * self->dissipate;
		}

		// scale angular movement dependent on fluid's density 
		VectorMA( self->s.angles, self->avelocity, p_frametime_scaled * (0.5 / fluid_density), self->s.angles );
		self->avelocity.Multiply( velocity_scale );
	}
	else
	{
		//	normal angular movement
		VectorMA (self->s.angles, self->avelocity, p_frametime_scaled, self->s.angles);
	}

	return	percent_submerged;
}