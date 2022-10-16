// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
//#include "ai_common.h"// SCG[1/23/00]: not used
#include "ai_utils.h"
#include "ai_weapons.h"
//#include "thinkFuncs.h"// SCG[1/23/00]: not used
//#include "ai_frames.h"// SCG[1/23/00]: not used
//#include "misc.h"// SCG[1/23/00]: not used
//#include "weapondefs.h"// SCG[1/23/00]: not used
#include "ai_aim.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
static AIMDATA aim_data;
    
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

//*****************************************************************************
//							LOCAL functions
//*****************************************************************************


// ----------------------------------------------------------------------------
// <nss>
// Name:		W_IsTargetCrouching
// Description:Will determine if your target is crouching or not.
// Input:userEntity_t *target
// Output:1=yes 0 =no
// Note:
//
// ----------------------------------------------------------------------------
int W_IsTargetCrouching(userEntity_t *target)
{
	gclient_t *client = target->client;
	
	if( !client )
	{
		return 0;
	}

	//AI_Dprintf("client->ps.pmove.pm_flags:%d\n",client->ps.pmove.pm_flags&PMF_DUCKED);
	if ( client->ps.pmove.pm_flags & PMF_DUCKED )
		return 1;
	return 0;

}

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeSpread
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_ComputeSpread( ai_weapon_t *weapon, const CVector &endPoint, 
                const CVector &right, const CVector &up, CVector &spreadPoint )
{
	float sx = (weapon->spread_x * rnd());
    if ( rnd() < 0.5f )
    {
        sx = -sx;
    }
	float sz = (weapon->spread_z * rnd());
    if ( rnd() < 0.5f )
    {
        sz = -sz;
    }

	spreadPoint = endPoint + (right * sx) + (up * sz);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Aim
// Description:
//				make sure the weapon is pointed in the direction the AI is 
//				facing
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void AI_Aim( userEntity_t *self, ai_weapon_t *weapon )
{
	aim_data.angles = self->s.angles;
	aim_data.angles.AngleToVectors(forward, right, up);

	aim_data.org = self->s.origin + (weapon->ofs.x * right) + 
				   (weapon->ofs.y * forward) + (weapon->ofs.z * up);

	userEntity_t *pEnemy = self->enemy;
    int bEnemyAlive = AI_IsAlive( pEnemy );
    if ( bEnemyAlive )
    {
        AI_ComputeSpread( weapon, pEnemy->s.origin, right, up, aim_data.spread_end );

	    if ( pEnemy )
	    {
		    //aim_data.dir = self->enemy->s.origin - aim_data.org;
            aim_data.dir = aim_data.spread_end - aim_data.org;
	    }
	    else
	    {
		    aim_data.angles.AngleToForwardVector( aim_data.dir );
	    }
    }
    else
    {
        aim_data.dir = forward;
    }

	//<nss>
	//Determine if the target is crouching... if so adjust for it.
	if ( bEnemyAlive && W_IsTargetCrouching(pEnemy) )
    {
		aim_data.dir.z -= (pEnemy->absmax.z - pEnemy->absmin.z)*0.65f;
	}
    aim_data.dir.Normalize();

	aim_data.end = aim_data.org + (aim_data.dir * weapon->distance);

}

// ----------------------------------------------------------------------------
//	<nss>
// Name:		ai_lead_target
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_lead_target( userEntity_t *targ, const CVector &offset, const CVector &org, 
  					 float speed, CVector &predicted_org )
{
	//	FIXME: base accuracy on skill  (working on it.. these changes should help..) <nss>

	/////////////////////////////////////////////////////////
	//	find time from self to target at speed
	/////////////////////////////////////////////////////////

	float	dist = VectorDistance( org, targ->s.origin );
	float	time_to_target = (dist / speed);
	float	Delta_FuckUp = 0;  //Our change in how much the monster is gonna fuck up his aim
	
	//Obtain the direction our target is traveling in
	CVector tForward;
	targ->s.angles.AngleToForwardVector(tForward);
	tForward.Normalize();
	
	float Fspeed = targ->velocity.Length() * 0.1f;
	if(Fspeed == 0.0)
		Fspeed = 1.0f;

	//Get the new 'projected' coordinates for the target;
	tForward = (tForward * Fspeed ) + targ->s.origin;
	float Ftime = VectorDistance( org, tForward )/(speed) ? speed : 1.0;


	

    int skill = gstate->GetCvar("skill");

	switch(skill)
	{
		case 0:
		{
			if(rnd() > 0.25)		//Just a 75% chance
			{
				Delta_FuckUp = 0.5;
			}
			break;
		}
		case 1:
		{
			if(rnd() > 0.25 && Fspeed > 80 )  //75% chance of missing and player has to be running.
			{
				Delta_FuckUp =3;
			}
			break;
		}
		case 2:
		default:
		{
			if(rnd() > 0.85 && Fspeed > 100 )  //15% chance of missing and player really has to be running.
			{
				Delta_FuckUp =6;
			}
			break;
		}
	}
	
	if(Delta_FuckUp)
	{
		CVector Target_Angles;
		Fspeed = targ->velocity.Length() * 0.1f;
		if(rnd() > 0.50f)
			Fspeed *= -1;
		//Get the new 'projected' coordinates for the target;
		Target_Angles = targ->s.angles;
		Target_Angles.yaw += 30/Delta_FuckUp * + (crand() * (90/Delta_FuckUp));
		Target_Angles.pitch += 5/Delta_FuckUp * + (crand() * (10/Delta_FuckUp));
		Target_Angles.AngleToForwardVector(tForward);

		tForward = (tForward * Fspeed) + targ->s.origin;
		predicted_org = tForward;
		//AI_Dprintf("Fuck Up----------------------------\n");
		//AI_Dprintf("Tox:%f Toy:%f Toz:%f\n",targ->s.origin.x,targ->s.origin.y,targ->s.origin.z);
		//AI_Dprintf("Pox:%f Poy:%f Poz:%f\n",predicted_org.x,predicted_org.y,predicted_org.z);
		//AI_Dprintf("End--------------------------------\n");
	}
	else
	{
		predicted_org = tForward;	
		//AI_Dprintf("Dead On*****************************\n");
		//AI_Dprintf("Tox:%f Toy:%f Toz:%f\n",targ->s.origin.x,targ->s.origin.y,targ->s.origin.z);
		//AI_Dprintf("Pox:%f Poy:%f Poz:%f\n",predicted_org.x,predicted_org.y,predicted_org.z);
		//AI_Dprintf("End Dead on*************************\n");
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AimLeading
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void AI_AimLeading( userEntity_t *self, ai_weapon_t *weapon )
{
	aim_data.angles = self->s.angles;
	aim_data.angles.AngleToVectors(forward, right, up);

	if(self->owner)
	{
		aim_data.org  =	self->owner->s.origin + (weapon->ofs.x * right) + 
		                (weapon->ofs.y * forward) + (weapon->ofs.z * up);
	}
	else
	{
		aim_data.org = self->s.origin + (weapon->ofs.x * right) + 
		                (weapon->ofs.y * forward) + (weapon->ofs.z * up);
	}
	
	userEntity_t *pEnemy = self->enemy;
    CVector endPoint;
	if ( pEnemy )
	{
        //AI_ComputeSpread( weapon, pEnemy->s.origin, right, up, endPoint );
		
		if ( weapon->weapon.flags & ITF_SPLASH )
		{
			//	fires at ground for splash damage
			ai_lead_target( pEnemy, CVector(0, 0, 0), aim_data.org, weapon->speed, endPoint );
		}
		else
		{
			//	fires at view origin to look pretty
			ai_lead_target( pEnemy, CVector(0, 0, 22.0), aim_data.org, weapon->speed, endPoint );
		}
	}
	else
	{
		endPoint = aim_data.org + (forward * MAX_WEAPON_TRACE);
	}

	aim_data.dir = endPoint - aim_data.org;
	
	if ( pEnemy && W_IsTargetCrouching(pEnemy)||(self->enemy->deadflag != DEAD_PUNK && self->enemy->deadflag != DEAD_NO))
    {
		aim_data.dir.z -= (pEnemy->absmax.z - pEnemy->absmin.z);//*0.50f;
	}
	aim_data.dir.Normalize();

	aim_data.end = aim_data.org + (aim_data.dir * weapon->distance);

	if(self->enemy->deadflag != DEAD_PUNK && self->enemy->deadflag != DEAD_NO && (self->enemy->deadflag == DEAD_DEAD || self->enemy->deadflag == DEAD_DYING))
	{
		if(self->owner)
			com->Damage( pEnemy, self->owner, self->owner, aim_data.end , aim_data.dir, 15.0f, DAMAGE_INERTIAL );
		else
			com->Damage( pEnemy, self, self, aim_data.end , aim_data.dir, 15.00f, DAMAGE_INERTIAL );
	}
	
	//aim_data.end = endPoint;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_aim_trace_leading
// Description:
// Input:
// Output:
// Note:
//				sets up v_angle to aim a bullet, leading target time seconds into the future
//
// ----------------------------------------------------------------------------
static void ai_aim_trace_leading( userEntity_t *self, float time )
{
	CVector	ang, dir, predicted_org;

	//	HACK:	scale accuracy based on distance, because the .1 second resolution is not
	//			enough for accuracy

	float lead_scale = (rnd() - 0.5) * 0.25 * 0.6;

	predicted_org = self->enemy->s.origin + CVector(self->enemy->velocity.x, self->enemy->velocity.y, 0.0) * time * lead_scale;

	dir = predicted_org - self->s.origin;
	dir.Normalize ();

	VectorToAngles( dir, ang );

	//	try to change yaw to match direction to lead_org.
	float save_yaw = self->s.angles.yaw;
	self->ideal_ang.yaw = ang.yaw;

	// ISP: do not need to do this 10 times, ang_speed should be set to once per frame
	//for (i = 0; i < (int )(time * 10); i++)
	{
		com->ChangeYaw (self);
	}

	ang.y = self->s.angles.yaw;		//	always fire the direction monster is facing, because movetogoal can turn us around
	ang.z = 0;					

	self->s.angles.yaw = save_yaw;
	if( self->client )
	{
		self->client->v_angle = ang;
	}
}

//*****************************************************************************
//						GLOBAL functions
//*****************************************************************************


// ----------------------------------------------------------------------------
//
// Name:		ai_aim_curWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
AIMDATA_PTR ai_aim_curWeapon( userEntity_t *self, ai_weapon_t *weapon )
{
	unsigned long nWeaponFlags = weapon->weapon.flags;

	if ( (nWeaponFlags & ITF_NOLEAD) )
	{
		AI_Aim( self, weapon );
	}
	else
	{
		AI_AimLeading( self, weapon );
	}

	return &aim_data;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_aim_playerWeapon
// Description:
// Input:
// Output:
// Note:
//	HACK FOR ECTS:	consolidate monster weapons so they have the same flags as 
//					player weapons
//
// ----------------------------------------------------------------------------
AIMDATA_PTR ai_aim_playerWeapon( userEntity_t *self, ai_weapon_t *weapon, float time )
{
/*
	if (weapon->flags & WEAPON_PROJECTILE)
		ai_aim_projectile (self, weapon);
	else if (weapon->flags & WEAPON_BULLET)
		ai_aim_bullet (self);
	else if (weapon->flags & WEAPON_TRACE)
*/
    userEntity_t *pEnemy = self->enemy;
    if ( AI_IsAlive( pEnemy ) )
    {
    	ai_aim_trace_leading( self, time );
    }
    else
    {
        AI_Aim( self, weapon );
    }

	return &aim_data;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wack_aim
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
AIMDATA_PTR ai_wack_aim( userEntity_t *self, ai_weapon_t *weapon )
{
	if ( !self->enemy )
	{
		return ai_aim_straight( self, weapon );
	}

	CVector destPoint = self->enemy->s.origin;

	// set aim angles to be the same as the orientation
	aim_data.angles = self->s.angles;
	// get forward, right, and up vectors from angles
	aim_data.angles.AngleToVectors(forward, right, up);

	// find aim origin by adding weapon's offset to self origin
	aim_data.org = self->s.origin + weapon->ofs.x * right + 
					weapon->ofs.y * forward + weapon->ofs.z * up;

	// find direction to aim in
	aim_data.dir = destPoint - aim_data.org;
	aim_data.dir.Normalize ();

	// calculate aiming endpoint from weapon distance
	aim_data.end = aim_data.org + aim_data.dir * weapon->distance;

	return &aim_data;
}


// ----------------------------------------------------------------------------
//
// Name:        ai_aim_straight
// Description: Aim function that doesn't, it just aims straight ahead.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

AIMDATA_PTR ai_aim_straight( userEntity_t *self, ai_weapon_t *weapon )
{
	// set aim angles to be the same as the orientation
	aim_data.angles = self->s.angles;
	// get forward, right, and up vectors from angles
	aim_data.angles.AngleToVectors(forward, right, up);

	// find aim origin by adding weapon's offset to origin
	aim_data.org = self->s.origin + weapon->ofs.x * right + 
					weapon->ofs.y * forward + weapon->ofs.z * up;

	// set direction to aim in, forward
	aim_data.dir = forward;
	// set the aim endpoint
	aim_data.end = aim_data.org + forward * weapon->distance;

	return &aim_data;
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


#if 0

// ----------------------------------------------------------------------------
//
// Name:		ai_aim
// Description:
//				using self->client->v_angle, find the origin and ending
//				position of an attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_aim( userEntity_t *self, ai_weapon_t *weapon )
{
	if( self->client )
	{
		aim_data.angles = self->client->v_angle;
	}
//	aim_data.angles.x = -aim_data.angles.x;
	aim_data.angles.AngleToVectors(forward, right, up);

	aim_data.org = self->s.origin + weapon->ofs.x * right + 
					weapon->ofs.y * forward + weapon->ofs.z * up;

	float sx = weapon->spread_x * rnd() * 2.0 - weapon->spread_x;
	float sz = weapon->spread_z * rnd() * 2.0 - weapon->spread_z;

	aim_data.spread_end = aim_data.org + forward * MAX_WEAPON_TRACE + 
						(right * sx * MAX_WEAPON_TRACE) + (up * sz * MAX_WEAPON_TRACE);

	aim_data.dir = aim_data.spread_end - aim_data.org;
	aim_data.dir.Normalize ();

	aim_data.end = aim_data.org + aim_data.dir * weapon->distance;

	//com->DrawLine (self, aim_data.org, aim_data.end, 10.0);
}


// ----------------------------------------------------------------------------
//
// Name:		ai_aim_projectile
// Description:
// Input:
// Output:
// Note:
//			sets up v_angle to aim a bullet
//
// ----------------------------------------------------------------------------
static void ai_aim_projectile( userEntity_t *self, ai_weapon_t *weapon )
{
	CVector			ang, org, dir, lead_org;
	playerHook_t	*hook = AI_GetPlayerHook( self );


	//	won't fire up in the air if entity is not on the ground
	if ( weapon->weapon.flags & ITF_NOLEAD )
	{
		//	calculate angle to target from actual firing position of projectile
		ang = self->s.angles;
		ang.AngleToVectors(forward, right, up);
		org = self->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;

		dir = self->enemy->s.origin - org;
		dir.Normalize ();

		VectorToAngles( dir, ang );

		if( self->client )
		{
			self->client->v_angle = ang;
		}
	}
	else
	{
		if (weapon->weapon.flags & ITF_SPLASH)
		{
			//	fires at ground for splash damage
			ai_lead_target (self->enemy, CVector(0, 0, 0), self->s.origin, weapon->speed, lead_org);
		}
		else
		{
			//	fires at view origin to look pretty
			ai_lead_target (self->enemy, CVector(0, 0, 22.0), self->s.origin, weapon->speed, lead_org);
		}

		if (weapon->weapon.flags & ITF_EXPLOSIVE  && weapon->weapon.flags & ITF_SPLASH)
		{
			//	if difficulty level is is high, or monster intelligence is high
			//	then monster will aim for splash damage
			if (ai_skill_rand () || hook->intQuota > INT_HIGH)
			{
				//	aim at ground in front of target
				lead_org.z -= 32.0;
			}
		}

#ifdef	MARKERS
		com->SpawnMarker (self, lead_org, debug_models [DEBUG_MARKER], 2.0);
#endif

		//	calculate angle to target from actual firing position of projectile
		ang = self->s.angles;
		ang.AngleToVectors(forward, right, up);

		org = self->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;

		dir = lead_org - org;
		dir.Normalize();

		VectorToAngles( dir, ang );

		//	try to change yaw to match direction to lead_org.  
		self->ideal_ang.yaw = ang.yaw;
		com->ChangeYaw(self);

		//ang.x = ang.x;
		ang.y = self->s.angles.yaw;		//	always fire the direction monster is facing, because movetogoal can turn us around
		ang.z = 0;					

		if( self->client )
		{
			self->client->v_angle = ang;
		}
	}
	
	//	debugging
//	ang.Normalize ();
//	com->DrawLine (self, org, lead_org, 3.0);

	// ISP: shitty way of doing things
	// NEED to fix after E3
	if (self->client )
	{
		aim_data.angles = self->client->v_angle;
	}
	aim_data.angles.AngleToVectors(forward, right, up);

	aim_data.org = self->s.origin + weapon->ofs.x * right + 
				   weapon->ofs.y * forward + weapon->ofs.z * up;

	float sx = weapon->spread_x * rnd() * 2.0 - weapon->spread_x;
	float sz = weapon->spread_z * rnd() * 2.0 - weapon->spread_z;

	aim_data.spread_end = aim_data.org + forward * MAX_WEAPON_TRACE + 
						(right * sx * MAX_WEAPON_TRACE) + (up * sz * MAX_WEAPON_TRACE);

	if ( self->enemy )
	{
		aim_data.dir = self->enemy->s.origin - aim_data.org;
		aim_data.dir.Normalize ();
	}
	else
	{
		if( self->client )
		{
			self->client->v_angle.AngleToForwardVector( aim_data.dir );
		}
	}

	aim_data.end = aim_data.org + aim_data.dir * weapon->distance;
}

#endif 0
