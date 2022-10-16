//////////////////////////////////////////////////////////////////////////
//	floating, bobbing crap
//
//////////////////////////////////////////////////////////////////////////

#include	"world.h"
//#include	"floater.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	func_floater (userEntity_t *self);

//////////////////////////////////////////////////////////////////
//	float_pain
//
//	called for pain function if floater has FLOATER_TAKEDAMAGE spawnflag set
//
//	allows floater to be moved by explosion damage without
//	ever being destroyed
//////////////////////////////////////////////////////////////////

void	float_pain (userEntity_t *self, userEntity_t *other, float kick, int damage)
{
	self->health = 65535;
}

//////////////////////////////////////////////////////////////////
//	float_die
//
//	called for die function if floater has FLOATER_TAKEDAMAGE spawnflag set
//
//	allows floater to be moved by explosion damage without
//	ever being destroyed
//////////////////////////////////////////////////////////////////

void	float_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	self->health = 65535;
}

/////////////////////////////////////////////////
//	func_floater
/////////////////////////////////////////////////

#define	MIN_VELOCITY_CAP	8.0

void	func_floater (userEntity_t *self)
{
	int			i;
	float		dissipate = 0.90, velocity_cap = 32.0;
	float		fluid_density = 1.0, fluid_viscosity = 1.0;
	float		object_mass = 1.0, object_volume = 4.0;
	float		max_x = 0.0, max_y = 60.0, max_z = 5.0;

	self->modelName = "models/global/dv_floater.dkm";

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "model"))
			self->modelName = self->epair [i].value;
		else 
		if (!stricmp (self->epair [i].key, "mass"))
			object_mass = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "volume"))
			object_volume = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "fluid_density"))
			fluid_density = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "dissipate"))
			dissipate = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "velocity_cap"))
			velocity_cap = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "max_x"))
			max_x = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "max_y"))
			max_y = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "max_z"))
			max_z = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "pitch"))
			self->s.angles.x= atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "yaw"))
			self->s.angles.y = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "roll"))
			self->s.angles.z = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "mins_x"))
			self->s.mins.x = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "mins_y"))
			self->s.mins.y = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "mins_z"))
			self->s.mins.z = atof (self->epair [i].value);
		else 
		if (!stricmp (self->epair [i].key, "maxs_x"))
			self->s.maxs.x = atof (self->epair [i].value);	
		else 
		if (!stricmp (self->epair [i].key, "maxs_y"))
			self->s.maxs.y = atof (self->epair [i].value);	
		else 
		if (!stricmp (self->epair [i].key, "maxs_z"))
			self->s.maxs.z = atof (self->epair [i].value);	
	}
	
	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_FLOAT;
	self->className = "func_floater";
	self->clipmask = MASK_MONSTERSOLID;	//	just MASK_SOLID won't clip against player!
	self->svflags |= SVF_MONSTER;

	self->flags |= FL_PUSHABLE;
	self->takedamage = DAMAGE_YES;
	self->pain = float_pain;
	self->die = float_die;

	self->s.modelindex = gstate->ModelIndex (self->modelName);
	self->s.iflags |= IF_SV_SENDBBOX;		//	send the real BBOX size down to client for prediction

	if (!stricmp  (self->modelName, "models/global/dv_floater.dkm"))
		gstate->SetSize (self, -32.0, -32.0, -32.0, 32.0, 32.0, 16.0);
	else 
		gstate->SetSize (self, -16.0, -16.0, -18.0, 16.0, 16.0, 18.0);
//		gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);

	//com->SetVector (self->avelocity, rnd () * 60 - 30, rnd () * 120 - 60, 0);
	self->avelocity.Set( rnd () * max_x - (max_x / 2), rnd () * max_y - (max_y / 2), rnd () * max_z - (max_z / 2));

	self->mass = object_mass;
	self->volume = object_volume;
	self->dissipate = dissipate;
	self->velocity_cap = (object_volume / object_mass) * MIN_VELOCITY_CAP;	//	velocity_cap;
//	float_init (self, object_mass, object_volume, fluid_density, dissipate, velocity_cap, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_float_register_func()
{
	gstate->RegisterFunc("float_pain",float_pain);
	gstate->RegisterFunc("float_die",float_die);
}
