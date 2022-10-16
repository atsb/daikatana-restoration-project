///////////////////////////////////////////////////////////////////////////////
// Content Friction Code
//
///////////////////////////////////////////////////////////////////////////////
//#include <windows.h>
#include <stdio.h>
#include <math.h>

#include	"p_user.h"
#include	"hooks.h"

#include	"p_global.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	friction_apply (userEntity_t *self, frictionHook_t *fhook)
{
	CVector			dir, ang;
	float			pc, wobble_speed, wander_speed;
	float			invs_mass;

	pc = gi.pointcontents (self->s.origin);

	////////////////////////////////////////////////////////////////////
	// object is not in a liquid
	////////////////////////////////////////////////////////////////////

	if (pc != CONTENTS_LAVA && pc != CONTENTS_SLIME && pc != CONTENTS_WATER)
	{
		// not currently in a liquid
		if (fhook->last_content == CONTENTS_LAVA || fhook->last_content == CONTENTS_SLIME ||
			fhook->last_content == CONTENTS_WATER || self->velocity.Length() < fhook->max_vel)
		{
			if (fhook->last_content == CONTENTS_LAVA || fhook->last_content == CONTENTS_SLIME ||
				fhook->last_content == CONTENTS_WATER)
			{
				// just left a liquid, so play exit sound
				serverState.StartEntitySound (self, CHAN_AUTO, serverState.SoundIndex(fhook->exit_sound), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			}

			// just left a liquid, or velocity < max_vel
			// if this is a self-propelled MOVETYPE_FLYMISSILE 
			// then increase its velocity until it is at max_vel
			if (self->movetype == MOVETYPE_FLYMISSILE)
			{
				dir = self->velocity * 1.25 * fhook->mass_scale;

				// cap velocity if necessary
				if (dir.Length() > fhook->max_vel)
				{
					dir = dir * (fhook->max_vel / dir.Length());
				}

				self->velocity = dir;
			}
			
			// make sure gravity goes back to normal
			self->gravity = 1.0;
		}

		fhook->last_content = pc;

		return;
	}

	////////////////////////////////////////////////////////////////////
	// object is in a liquid
	////////////////////////////////////////////////////////////////////
	
	if (fhook->last_content != pc && fhook->last_content == CONTENTS_EMPTY)
	{
		// just hit liquid, so slow down
		
		// save position where water was first encountered
		// we will use this as a point to drift around
		fhook->liquid_pos = self->s.origin;
		
		// scale velocity down by mass_scale
		dir = self->velocity * 0.5 * fhook->mass_scale;
		self->velocity = dir;
		
		// set change_time to current time
		fhook->change_time = level.time;
		
		if (fhook->flags & FR_SPIRAL)
		{
			if (rnd () < 0.5)
			{
				fhook->spiral_dir = -1.0;
			}
			else
			{
				fhook->spiral_dir = 1.0;
			}
		}

		// decrease effect of gravity
		self->gravity = 0.5 * fhook->mass_scale;		
		
		// play appropriate content hit sound
		serverState.StartEntitySound (self, CHAN_AUTO, serverState.SoundIndex(fhook->enter_sound), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
	else 
	if ((self->movetype == MOVETYPE_BOUNCE) || (self->movetype == MOVETYPE_TOSS))
	{
		// this is an object affected by gravity which has already entered
		// a liquid on a previous frame
		if (fhook->change_time <= level.time)
		{
			invs_mass = 2.0 - fhook->mass_scale;

			// change the z of liquid_pos to 
			// approximately the objects current z
			if (fhook->flags & FR_SEMIFLOAT)
			{
				fhook->liquid_pos.z = self->s.origin.z + (rnd() - 0.2) * (rnd() * 224.0 + 32.0);
			}
			else
			{
				fhook->liquid_pos.z = self->s.origin.z;// + (rnd () - 0.2) * (rnd () * 256.0 + 32.0);
			}

			// find the vector from current position to liquid_pos
			dir = fhook->liquid_pos - self->s.origin;
			dir.Normalize ();

			VectorToAngles(dir, ang);
			ang.AngleToVectors (forward, right, up);

			if (fhook->flags & FR_SPIRAL)
			{
				right = right * fhook->spiral_dir;
				
				// less lateral movement for greater masses
				wobble_speed = 200 * invs_mass;

				dir = right * wobble_speed;
				if (fhook->flags & FR_SEMIFLOAT)
				{
					dir = dir + forward * 200 * invs_mass;
				}

				fhook->change_time = level.time + 0.05 * fhook->mass_scale * 2.0;
			}
			else
			{
				// randomly choose left or right
				if (rnd() < 0.5)
				{
					right = right * -1.0;
				}

				// randomly choose towards or away
				if (rnd() < 0.5)
				{
					forward = forward * -1.0;
				}

				wobble_speed = (60 + rnd() * 100) * fhook->mass_scale;
				wander_speed = (60 + rnd() * 100) * fhook->mass_scale;

				dir = right * wobble_speed + forward * wander_speed;

				// choose a random time to effect the next change
				fhook->change_time = level.time + rnd() * invs_mass + 0.1;
			}

			self->velocity = dir;

			// change angular velocity to make drift look more realistic
			//self->avelocity.Set(fhook->size_scale * (rnd() - 0.5) * 700.0, fhook->size_scale * (rnd() - 0.5) * 700.0, fhook->size_scale * (rnd() - 0.5) * 700.0);
		}
	}

	fhook->last_content = pc;
}

//////////////////////////////
// friction_init
//////////////////////////////

frictionHook_t	*friction_init (userEntity_t *self, frictionHook_t *fhook)
{
	fhook = (frictionHook_t *)gi.X_Malloc(sizeof (frictionHook_t), MEM_TAG_HOOK);

	// get current content
	fhook->last_content = gi.pointcontents (self->s.origin);

	fhook->change_time = level.time;

	fhook->mass_scale = 1.0;
	fhook->size_scale = 1.0;

	// current position
//	fhook->last_pos = self->s.origin;
	
	if (fhook->last_content != CONTENTS_EMPTY)
	{
		fhook->liquid_pos = self->s.origin;
	}
	else
	{
		fhook->liquid_pos.Zero();
	}

	fhook->max_vel = 1000.0;
	fhook->flags = 0;

	fhook->enter_sound = "shared/bloop4.wav";
	fhook->exit_sound = "shared/bloop4.wav";

	return	fhook;
}

//////////////////////////////
// friction_set_sounds
//////////////////////////////

void	friction_set_sounds (userEntity_t *self, frictionHook_t *fhook, char *enter, char *exit)
{
	fhook->enter_sound = enter;
	fhook->exit_sound = exit;
}

//////////////////////////////
// friction_set_physics
//////////////////////////////

void	friction_set_physics (userEntity_t *self, frictionHook_t *fhook, float size, float mass, float max_vel, int flags)
{
	if (mass > 2.0)
	{
		mass = 2.0;
	}
	if (size > 2.0)
	{
		size = 2.0;
	}

	fhook->mass_scale = mass;
	fhook->size_scale = size;
	fhook->max_vel = max_vel;
	fhook->flags = flags;
}

//////////////////////////////
//	friction_remove
//////////////////////////////

void	friction_remove (frictionHook_t *fhook)
{
	if (fhook)
	{
		gi.X_Free(fhook);
//		com_free (fhook);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitFriction
//
//	set up pointers to com_friction.cpp functions
///////////////////////////////////////////////////////////////////////////////

void	com_InitFriction (void)
{
	com.friction_apply = friction_apply;
	com.friction_init = friction_init;
	com.friction_set_sounds = friction_set_sounds;
	com.friction_set_physics = friction_set_physics;
	com.friction_remove = friction_remove;
}
