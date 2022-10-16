//////////////////////////////////////////////////////////////////////
//	Chase Cam Routines
//
//	mainly for debugging creature AI
//
//
//	Camera Modes:
//
//  mode 0
//		camera always faces player's back
//		distance from player and angle are affected by pitch
//
//		client still controls facing yaw and pitch of player entity
//		client still controls movement of player entity
//
//	mode 1
//		camera chooses a best angle to view the entity from
//		camera moves from old best angle to a new best angle
//		while locked on camera->goalentity
//
//		client still controls movement of player entity
//		client still controls facing yaw and pitch of player entity
//			even though the client view is facing a different direction
//
//	mode 2
//		camera views from view entity and turns to track target
//
//	mode 3
//		camera always faces target's back
//		distance from target is affected by obstructions
//
//////////////////////////////////////////////////////////////////////
#include	<math.h>

#include	"world.h"

#include	"ai_common.h"
#include	"ai_move.h"

#include	"chasecam.h"
#include	"nodelist.h"
#include	"ai_utils.h"
#include	"ai_func.h"
#include	"ai.h"
#include    "ai_frames.h"
// exported chasecam functions
//DllExport   void	camera_start(userEntity_t *target);
//DllExport   void	camera_stop(userEntity_t *target);

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define		NUMDIR				16
#define		NUMPITCH			2
#define		CAMERA_PITCH		15
#define		CAMERA_MAXSPEED		320.0
#define		CAMERA_CATCHMULT	4.0
#define		CAMERA_ACCELERATE	1.25 
#define		CAMERA_DECELERATE	0.75

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//userEntity_t	*camera;

//userEntity_t	*cam_debug_target;		// for toggling back and forth between player and an entity

static	CVector cam_mins(-8, -8, -8);
static	CVector cam_maxs(8, 8, 8);

// SCG[11/20/99]: Save game stuff
#define CAMERAHOOK(x) (int)&(((cameraHook_t *)0)->x)
field_t camera_hook_fields[] = 
{
	{"pre_think",			CAMERAHOOK(pre_think),				F_FUNC},
	{"post_think",			CAMERAHOOK(post_think),				F_FUNC},
	{"mode",				CAMERAHOOK(mode),					F_INT},
	{"demonic",				CAMERAHOOK(demonic),				F_INT},
	{"follow_dist",			CAMERAHOOK(follow_dist),			F_FLOAT},
	{"target_dist",			CAMERAHOOK(target_dist),			F_FLOAT},
	{"next_change",			CAMERAHOOK(next_change),			F_FLOAT},
	{"min_change_time",		CAMERAHOOK(min_change_time),		F_FLOAT},
	{"speed",				CAMERAHOOK(speed),					F_FLOAT},
	{"max_speed",			CAMERAHOOK(max_speed),				F_FLOAT},
	{"view_angle",			CAMERAHOOK(view_angle),				F_VECTOR},
	{"ideal_angle",			CAMERAHOOK(ideal_angle),			F_VECTOR},
	{"target_angles",		CAMERAHOOK(target_angles),			F_VECTOR},
	{"best_org",			CAMERAHOOK(best_org),				F_VECTOR},
	{"best_dir",			CAMERAHOOK(best_dir),				F_VECTOR},
	{"target_think",		CAMERAHOOK(target_think),			F_FUNC},
	{"target_prethink",		CAMERAHOOK(target_prethink),		F_FUNC},
	{"target_postthink",	CAMERAHOOK(target_postthink),		F_FUNC},
	{"target_ai_flags",		CAMERAHOOK(target_ai_flags),		F_INT},
	{"target_th_state",		CAMERAHOOK(target_th_state),		F_BYTE},
	{"target_movetype",		CAMERAHOOK(target_movetype),		F_INT},
	{"last_target_pos",		CAMERAHOOK(last_target_pos),		F_VECTOR},
	{"last_target_angles",	CAMERAHOOK(last_target_angles),		F_VECTOR},
	{"last_target_v_angle",	CAMERAHOOK(last_target_v_angle),	F_VECTOR},
	{"debug_target",		CAMERAHOOK(debug_target),			F_EDICT},
	{NULL, 0, F_INT}
};

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void camera_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, camera_hook_fields, sizeof( cameraHook_t ) );
}

void camera_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, camera_hook_fields, sizeof( cameraHook_t ) );
}
///////////////////////////////////////////////////////////////////////////////
//	camera_check_target
//
//	checks the camera's target to determine if it was removed
//	
//	if the camera's target was removed, a new FL_FAKEENTITY target is spawned 
//	for it at the old target's last known position
//
//	if the camera is turned off while it has an FL_FAKEENTITY target, the
//	target entity is freed
//
//	this should be done first thing in EVERY camera prethink!!
///////////////////////////////////////////////////////////////////////////////

void	camera_check_target (userEntity_t *camera)
{
	if (camera->goalentity == NULL || camera->goalentity->inuse != TRUE)
	{
		cameraHook_t	*chook = (cameraHook_t *) camera->userHook;
		userEntity_t	*new_target;

		new_target = gstate->SpawnFakeEntity ();

		
		new_target->s.origin = chook->last_target_pos;
		new_target->s.angles = chook->last_target_angles;
//		new_target->client->v_angle = chook->last_target_v_angle;
		new_target->velocity.Zero();

		camera->goalentity = new_target;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_save_target_info
//
//	saves some info about the current target for use by camera_check_target
//	if the target is removed
//
//	this should be done first thing in EVERY camera postthink!!
///////////////////////////////////////////////////////////////////////////////

void	camera_save_target_info (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;

	camera_check_target (camera);

	chook->last_target_pos = camera->goalentity->s.origin;
//	chook->last_target_angles = camera->goalentity->s.angles;
//	chook->last_target_v_angle = camera->goalentity->client->v_angle;
}

///////////////////////////////////////////////////////////////////////////////
//	camera_set_yaw
//
//	sets client's view angle to ang
///////////////////////////////////////////////////////////////////////////////

void	camera_set_yaw (userEntity_t *client, CVector &ang)
{
	CVector	angles;

	angles = ang;
//	angles.x = client->client->v_angle.x;
	angles.x = client->s.angles.x;
	gstate->SetClientAngles (client, angles);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_set_viewangle
//
//	sets client's view angle to ang
///////////////////////////////////////////////////////////////////////////////

void	camera_set_viewangle (userEntity_t *client, CVector &ang)
{
	gstate->SetClientAngles( client, ang );
}

///////////////////////////////////////////////////////////////////////////////
//	camera_track_client
//
///////////////////////////////////////////////////////////////////////////////

void	camera_track_client (userEntity_t *camera)
{
	CVector		ang, dir;

	dir = (camera->goalentity->s.origin + CVector(0, 0, camera->goalentity->viewheight)) - camera->s.origin;
	dir.Normalize();

	// get the angle from camera to self, forward is already the vector
	VectorToAngles(dir, ang);
	camera_set_viewangle(camera->owner, ang);

	camera_save_target_info(camera);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_track
//
//	turns the camera to face target
///////////////////////////////////////////////////////////////////////////////

void	camera_track (userEntity_t *camera)
{
	CVector ang, dir;

	dir = camera->goalentity->s.origin - camera->s.origin;
	dir.Normalize();

	// get the angle from camera to self, forward is already the vector
	VectorToAngles(dir, ang);
	camera_set_viewangle(camera->owner, ang);

	camera_save_target_info (camera);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_track_yaw
//
///////////////////////////////////////////////////////////////////////////////

void	camera_track_yaw (userEntity_t *camera)
{
	CVector	ang, dir;

	dir = camera->goalentity->s.origin - camera->s.origin;
	dir.Normalize();

	// get the angle from camera to self, forward is already the vector
	VectorToAngles(dir, ang);
	camera_set_yaw(camera->owner, ang);

	camera_save_target_info(camera);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_ignore_input
//
//	sets client's view angle to ang
///////////////////////////////////////////////////////////////////////////////

void	camera_ignore_input (userEntity_t *client)
{
	//	Q2FIXME:	change to Q2 network functions
	//	gstate->WriteByte (client, MSG_ONE, SVC_IGNOREINPUT);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_restore_input
//
//	sets client's view angle to ang
///////////////////////////////////////////////////////////////////////////////

void	camera_restore_input (userEntity_t *client)
{
	//	Q2FIXME:	change to Q2 network functions
	//	gstate->WriteByte (client, MSG_ONE, SVC_RESTOREINPUT);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_best_angle
//
//	finds the best (least obstructed) direction
//	to view an entity from
//
//	traces lines out CAMERA_PITCH degrees above horizontal
//
///////////////////////////////////////////////////////////////////////////////

void	camera_best_angle (userEntity_t *camera, CVector &best_org)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	float			dist	[NUMDIR][NUMPITCH];
	float			best	[NUMDIR][NUMPITCH];
	CVector			end		[NUMDIR][NUMPITCH];
	float			yaw_add, yaw, yaw_start;
	float			best_yaw, best_pitch;
	int				y, i, lo, hi, best_num, best_y;
	CVector			org, temp, goal_org;
	CVector			mins, maxs;

	mins.Set( -8, -8, -8 );
	maxs.Set( 8, 8, 8 );

	// find the angle opposite of the angle the target is facing and start there
	yaw = yaw_start = fmod (camera->goalentity->s.angles.y + 180.0, 360);

	yaw_add = 360.0 / NUMDIR;
	org = camera->goalentity->s.origin;
	org.z += camera->goalentity->viewheight;

	// trace lines out in NUMDIR directions, looking for most open view
	for (y = 0; y < NUMPITCH; y++)
	{
		for (i = 0; i < NUMDIR; i++)
		{
			temp.Set(-(CAMERA_PITCH * y), yaw, 0.0);
			temp.AngleToVectors (forward, right, up);
	
			end [i][y] = org + forward * chook->follow_dist;

			//	Q2KILL
			//	gstate->TraceBox (org, end[i][y], mins, maxs, FALSE, camera->goalentity, &trace);
			tr = gstate->TraceBox_q2(org, mins, maxs, end[i][y], camera->goalentity, MASK_OPAQUE);
			
			dist [i][y] = tr.fraction * chook->follow_dist;
			if (y == 1) 
			{
				dist [i][y] *= 1.25;	// weight overhead view higher than others
			}
//			gstate->particle (tr.endpos, mins, gstate->global_white, 1);
//			com->SpawnMarker (gcamera, tr.endpos, debug_models[DEBUG_MARKER], 0.1);

			yaw += fmod (yaw_add, 360);
		}
	}

	// get the "best" total for each direction by adding the distance
	// each trace went to the distance its two immediate neighbors went

	// doesn't consider how open the traces above/below were, but it probably should
	for (y = 0; y < NUMPITCH; y++)
	{
		for (i = 0; i < NUMDIR; i++)
		{
			lo = i - 1;
			if (lo < 0) lo = NUMDIR - 1;

			hi = i + 1;
			if (hi > NUMDIR - 1) hi = 0;

			best [i][y] = dist [lo][y] + dist [i][y] + dist [hi][y];
	
			// weight the total if it is visible from current location
			// this will cut down on quick camera switches
			
			//	Q2KILL
			//	gstate->TraceBox (camera->s.origin, end[i][y], mins, maxs, FALSE, camera, &trace);
			tr = gstate->TraceBox_q2 (camera->s.origin, mins, maxs, end[i][y], camera, MASK_OPAQUE);

			if (tr.fraction == 1.0)
				best [i][y] *= 2.0;
		}
	}

	hi = 0;
	// now find the best with the highest value
	for (y = 0; y < NUMPITCH; y++)
	{
		for (i = 0; i < NUMDIR; i++)
		{
			if (best [i][y] > hi)
			{
				best_num = i;
				best_y = y;
				hi = best [i][y];
			}
		}
	}

	best_yaw = fmod ((float) best_num * yaw_add + yaw_start, 360);
	best_pitch = -(CAMERA_PITCH * best_y);
	
	temp = CVector(best_pitch, best_yaw, 0.0);
	temp.AngleToVectors(forward, right, up);

	best_org = org + forward * (dist [best_num][best_y] - 32.0);
}

//************************* Camera Mode 0 *******************************//
//  mode 0
//		camera always faces player's back
//		distance from player and angle are affected by pitch
//
//		client still controls facing yaw and pitch of player entity
//		client still controls movement of player entity

///////////////////////////////////////////////////////////////////////////////
//	camera_point_behind
//
//	mode 0 placement
//
//	finds optimal point behind camera target based on target's view angles
//	takes obstructions into account
///////////////////////////////////////////////////////////////////////////////

void	camera_point_behind (userEntity_t *camera, CVector &point)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	float			dist_mult, pitch_scale, back_off, dist;
	CVector			org, end;
	userEntity_t	*target;

	target = camera->goalentity;
	dist = chook->follow_dist;

	if (chook->mode < 128)
	{
		// get multipliers for forward and up
		dist_mult = dist * 0.7;

		// find forward, right and up vectors
		if(target->client)
		{
			end = target->client->v_angle;
			end.x *= 0.33;
			end = end + chook->view_angle;
			end.AngleToVectors(forward, right, up);
			pitch_scale = target->client->v_angle [0] / 240.0; // (80 * 3) or v_angle / 3.0

		}
		else
		{
			end = target->s.angles;
			end = end + chook->view_angle;
			end.AngleToVectors(forward, right, up);		
			pitch_scale = target->s.angles.pitch / 240.0; // (80 * 3) or v_angle / 3.0
		}

		org = target->s.origin;
		org.z += target->viewheight;
		end = org + forward * -(dist_mult * (1.0 - fabs (pitch_scale))) + up * (dist_mult * pitch_scale);

		tr = gstate->TraceBox_q2( org, cam_mins, cam_maxs, end, camera->goalentity, MASK_OPAQUE );
		if (tr.fraction < 1.0)
		{
			end = tr.endpos;

			org = org - end;
			back_off = org.Length();

			if (back_off < 8)
			{
				back_off = back_off * 0.5;
			}
			else
			{
				back_off = 8;
			}

			// back off a bit from the endpoint, toward camera
			//end = end + forward * (back_off * (1.0 - pitch_scale)) + up * (back_off * pitch_scale);
			end = end + forward * back_off;
		}

		point = end;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_place_behind
//
//	Original mode 0 placement with Quake 1 engine.  Doesn't look very good in 
//	Q2 because camera origin is set directly, and that screws with interpolation 
//	a bit
///////////////////////////////////////////////////////////////////////////////

void	camera_place_behind (userEntity_t *camera)
{
	CVector	point;

	camera_point_behind (camera, point);
	camera->s.origin = point;
	gstate->LinkEntity (camera);

	camera_track_yaw (camera);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_chase_behind
//
//	rotates the camera around the target
//	while turning to face it
///////////////////////////////////////////////////////////////////////////////

void	camera_chase_behind (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	float			dist, wish_speed;
	float			pred_dist, time_to_target;
	CVector			vel, pred_org, temp;

	if (chook->mode < 128)
	{
		chook->target_dist = VectorDistance (camera->s.origin, camera->goalentity->s.origin);

		camera_point_behind (camera, chook->best_org);
		chook->next_change = gstate->time + chook->min_change_time;

		chook->best_dir = chook->best_org - camera->s.origin;
		dist = chook->best_dir.Length();
		chook->best_dir.Normalize();

		// can we reach the new best_org?
		tr = gstate->TraceBox_q2( chook->best_org, cam_mins, cam_maxs, camera->s.origin, camera->goalentity, MASK_OPAQUE );
		if (tr.fraction != 1.0)
		{
			camera->s.origin = chook->best_org;
			gstate->LinkEntity (camera);
			chook->speed = 0;
		}
		else 
		if ( camera->s.origin != chook->best_org )
		{
/*
			vel = camera->velocity;
			time_to_target = dist / vel.Length();
			dist_per_frame = vel.Length() * gstate->frametime;
			pred_org = camera->s.origin + vel * gstate->frametime;

			temp = chook->best_org - pred_org;
			pred_dist = temp.Length();

			if (chook->speed == 0.0)
				chook->speed = 32.0;

			if (dist <= 8.0 || pred_dist < 8.0)
			{
				if (dist < dist_per_frame || pred_dist < dist_per_frame || time_to_target <= gstate->frametime)
				{
					ZeroVector (camera->velocity);
					camera->s.origin = chook->best_org;
					chook->speed = 0;
					gstate->LinkEntity (camera);
				}
				else
				{
					chook->speed *= CAMERA_DECELERATE;
					if (chook->speed < 32)  chook->speed = 0;

					temp = chook->best_dir * chook->speed;
					camera->velocity = temp;
				}
			}
			else if (pred_dist > 8.0)
			{
				chook->speed *= CAMERA_ACCELERATE;
				
				if (chook->speed > chook->max_speed)
					chook->speed = chook->max_speed;

				temp = chook->best_dir * chook->speed;
				camera->velocity = temp;
			}
*/
			vel = camera->velocity;
			time_to_target = dist / vel.Length();
			pred_org = camera->s.origin + vel * gstate->frametime;

			temp = chook->best_org - pred_org;
			pred_dist = temp.Length();

			if (dist < 8 || pred_dist < 8 || time_to_target <= gstate->frametime)
			{
				camera->velocity.Zero();
				camera->s.origin = chook->best_org;
				gstate->LinkEntity (camera);
			}
			else
			{
				wish_speed = 640 * ((pred_dist + 40) / 160);
				if (wish_speed > 640)
					wish_speed = 640;
				vel = chook->best_dir * wish_speed;
				camera->velocity = vel;
			} 
		}
	}

	chook->target_angles = camera->goalentity->s.angles;

//	camera_track_client (camera);
	camera_track_yaw (camera);
}

//************************* Camera Mode 1 *******************************//
//
//	mode 1
//		camera chooses a best angle to view the entity from
//		camera moves from old best angle to a new best angle
//		while locked on camera->goalentity
//
//		client still controls movement of player entity
//		client still controls facing yaw and pitch of player entity
//			even though the client view is facing a different direction

///////////////////////////////////////////////////////////////////////////////
//	camera_auto_chase
//
//	rotates the camera around the target
//	while turning to face it
///////////////////////////////////////////////////////////////////////////////

void	camera_auto_chase (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	CVector			ang, dir, next_cam, next_targ;
	float			dist;
	float			pred_dist, time_to_target, dist_per_frame;
	CVector			vel, pred_org, temp;

	if (chook->mode < 128)
	{
		chook->best_dir = chook->best_org - camera->s.origin;
		dist = chook->best_dir.Length();
		chook->best_dir.Normalize();

		chook->target_dist = VectorDistance(camera->s.origin, camera->goalentity->s.origin);

		// determine if we can reach the best_org by moving, otherwise
		// place us there...
		gstate->TraceLine(chook->best_org, camera->s.origin, TRUE, camera, &trace);
		if (trace.fraction != 1.0)
		{
			camera->s.origin = chook->best_org;
			gstate->SetOrigin2 (camera, camera->s.origin);
		}
		else 
		if (trace.fraction == 1.0)
		{
			gstate->TraceLine (camera->goalentity->s.origin, camera->s.origin, TRUE, camera->goalentity, &trace);	
		}

		// is it time to look for a better view?
		if (gstate->time >= chook->next_change || trace.fraction != 1.0 ||
			chook->target_dist > chook->follow_dist)
		{
			temp = chook->best_org;

			camera_best_angle (camera, chook->best_org);
			chook->next_change = gstate->time + chook->min_change_time;

			chook->best_dir = chook->best_org - camera->s.origin;
			dist = chook->best_dir.Length();
			chook->best_dir.Normalize ();

			// can we reach the new best_org?
			gstate->TraceLine (chook->best_org, camera->s.origin, TRUE, camera->goalentity, &trace);
			if (trace.fraction != 1.0)
				gstate->SetOrigin2 (camera, chook->best_org);

			if (chook->speed < 32.0 && chook->best_org != temp)
				chook->speed = 32.0;
			
			if (chook->best_org != temp)
			{
				chook->max_speed = CAMERA_MAXSPEED;
				// get distance to target
				if (chook->target_dist > chook->follow_dist * 2.0)
					chook->max_speed *= CAMERA_CATCHMULT;
			}
		}

/*		
		if (dist < 64.0)
		{
			chook->speed *= CAMERA_DECELERATE;
			if (chook->speed < 32)  chook->speed = 0;
		}
		else
			chook->speed *= CAMERA_ACCELERATE;

		if (chook->speed > chook->max_speed)
			chook->speed = chook->max_speed;

		temp = chook->best_dir * chook->speed;

		//	camera_accelerate (camera, temp, wish_speed);
		camera->velocity = temp;
*/
		vel = camera->velocity;
		time_to_target = dist / vel.Length();
		dist_per_frame = vel.Length() * gstate->frametime;
		pred_org = camera->s.origin + vel * gstate->frametime;

		temp = chook->best_org - pred_org;
		pred_dist = temp.Length();

		if (dist <= 64.0 || pred_dist < 64.0)
		{
			if (dist < dist_per_frame || pred_dist < dist_per_frame || time_to_target <= gstate->frametime)
			{
				camera->velocity.Zero();
				camera->s.origin = chook->best_org;
				gstate->LinkEntity (camera);
			}
			else
			{
				chook->speed *= CAMERA_DECELERATE;
				if (chook->speed < 32)  chook->speed = 0;

				temp = chook->best_dir * chook->speed;
				camera->velocity = temp;
			}
		}
		else if (pred_dist > 64.0)
		{
			chook->speed *= CAMERA_ACCELERATE;
			
			if (chook->speed > chook->max_speed)
				chook->speed = chook->max_speed;

			temp = chook->best_dir * chook->speed;
			camera->velocity = temp;
		}
	}

	camera_track_client (camera);
}

//************************* Camera Mode 2 *******************************//

//************************* Camera Mode 3 *******************************//

///////////////////////////////////////////////////////////////////////////////
//	camera_follow
//
//	mode 3 placement
//
//  places the camera dist units from self->s.origin
//	at a 45 degree angle up and -forward
//  or at the furthest unobstructed distance
//	from self->s.origin at a 45 degree angle
///////////////////////////////////////////////////////////////////////////////

void	camera_follow (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	CVector			end, ang;

	camera_check_target (camera);

	if (chook->mode < 128)
	{
		ang = camera->goalentity->s.angles;
		ang.AngleToVectors (forward, right, up);
		forward = forward * -1.0;

		end = camera->goalentity->s.origin + forward * chook->follow_dist;

		gstate->TraceLine (camera->goalentity->s.origin, end, TRUE, camera->goalentity, &trace);
		gstate->SetOrigin2 (camera, trace.endpos);
	}
}

//************************* Camera Mode 4 *******************************//

///////////////////////////////////////////////////////////////////////////////
//	camera_rotate
//
//	rotates the camera around the target
//	while turning to face it
///////////////////////////////////////////////////////////////////////////////

void	camera_rotate (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	CVector			ang, temp, org, dir;
	CVector			mins, maxs;
	float			dist;

	if (chook->mode < 128)
	{
		mins.Set( -8, -8, -8 );
		maxs.Set( 8, 8, 8 );

		if (gstate->time >= chook->next_change)
		{
			camera_best_angle (camera, chook->best_org);
			chook->next_change = gstate->time + chook->min_change_time;
		}
		
		temp = camera->goalentity->s.origin - camera->s.origin;
		VectorToAngles( ang, temp );
		ang.AngleToVectors (forward, right, up);
		dist = temp.Length();

		org = camera->s.origin + right * (CAMERA_MAXSPEED * gstate->frametime + 8.0);
		
		//	Q2KILL
		//	gstate->TraceBox (camera->s.origin, org.vec (), mins, maxs, TRUE, camera, &trace);
		tr = gstate->TraceBox_q2 (camera->s.origin, mins, maxs, org, camera, MASK_OPAQUE);

		if (tr.fraction < 1.0)
		{
			gstate->bprint ("going forward\n");
			// go forward to avoid a collision with world
			temp = forward * CAMERA_MAXSPEED;
		}
		else if (chook->follow_dist - dist > 16.0)
		{
			gstate->bprint ("going diagonally backwards\n");

			// try going back to reach max distance
			dist = (CAMERA_MAXSPEED * gstate->frametime + 8.0);
			org = camera->s.origin + right * dist - forward * dist;
			
			//	Q2KILL
			//	gstate->TraceBox (camera->s.origin, org.vec (), mins, maxs, TRUE, camera, &trace);
			tr = gstate->TraceBox_q2 (camera->s.origin, mins, maxs, org, camera, MASK_OPAQUE);

			if (tr.fraction == 1.0)
				temp = right * CAMERA_MAXSPEED - forward * CAMERA_MAXSPEED;
			else
				temp = -forward * CAMERA_MAXSPEED;
		}
		else if (dist - chook->follow_dist > 16.0)
		{
			gstate->bprint ("going diagonally forward\n");

			// try going back to reach max distance
			dist = (CAMERA_MAXSPEED * gstate->frametime + 8.0);
			org = camera->s.origin + right * dist + forward * dist;
			
			//	Q2KILL
			//	gstate->TraceBox (camera->s.origin, org.vec (), mins, maxs, TRUE, camera, &trace);
			tr = gstate->TraceBox_q2(camera->s.origin, mins, maxs, org, camera, MASK_OPAQUE);

			if (tr.fraction == 1.0)
				temp = right * CAMERA_MAXSPEED + forward * CAMERA_MAXSPEED;
			else
				temp = forward * CAMERA_MAXSPEED;
		}
		else
		{
			gstate->bprint ("going right\n");
			temp = right * CAMERA_MAXSPEED;
		}

		camera->velocity = temp;

		camera_set_viewangle (camera->owner, ang);
	}

	camera_save_target_info (camera);
}

//************************* Camera Mode 5 *******************************//

///////////////////////////////////////////////////////////////////////////////
//	camera_place_dist
//
//	mode 4 placement
//
//	places the camera dist units in yaw direction
//	from self->s.origin in direction yaw
///////////////////////////////////////////////////////////////////////////////

void	camera_place_dist (userEntity_t *camera, userEntity_t *target, float dist)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	CVector			temp, org, end;
	float			back_off, dist_mult;

	// get multipliers for forward and up
	dist_mult = sqrt (dist * dist / 2.0);

	temp = chook->view_angle;
	temp.AngleToVectors (forward, right, up);

	org = target->s.origin;
	end = org + forward * dist + up * dist;

	gstate->TraceLine(org, end, TRUE, camera->goalentity, &trace);
	end = trace.endpos;

	temp = org - end;
	back_off = temp.Length();

	if (back_off > 16.0)
		back_off = 16.0;
	// back off a bit from the endpoint, toward camera
	end = end - forward * back_off - up * back_off;

	gstate->SetOrigin2 (camera, end);
}

//************************* Camera Mode 6 *******************************//

///////////////////////////////////////////////////////////////////////////////
//	camera_best_view
//
//	rotates the camera around the target
//	while turning to face it
///////////////////////////////////////////////////////////////////////////////

void	camera_best_view (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	
	CVector			ang, dir, temp, next_cam, next_targ;
	float			dist;

	// get the speed before the origin is set
	temp = camera->velocity;
	chook->speed = temp.Length();

	// determine if we can reach the best_org by moving, otherwise
	// place us there...
	gstate->TraceLine (chook->best_org, camera->s.origin, TRUE, camera, &trace);
	if (trace.fraction == 1.0)
	{
		gstate->TraceLine (camera->goalentity->s.origin, camera->s.origin, TRUE, camera->goalentity, &trace);	
	}
	// is it time to look for a better view?
	if (gstate->time >= chook->next_change || trace.fraction != 1.0)
	{
		camera_best_angle(camera, chook->best_org);
		chook->next_change = gstate->time + chook->min_change_time;

		chook->best_dir = chook->best_org - camera->s.origin;
		chook->best_dir.Normalize();

		// can we reach the new best_org?
		gstate->TraceLine(chook->best_org, camera->s.origin, TRUE, camera->goalentity, &trace);
		if (trace.fraction != 1.0)
			gstate->SetOrigin2(camera, chook->best_org);

		if (chook->speed < 32.0)
			chook->speed = 32.0;
	}

	temp = camera->s.origin - chook->best_org;
	dist = temp.Length();

	if (dist < 16.0)
	// if we're close to the best position, slow down
		temp.Zero();
	else 
	if (dist < 64.0)
	// if we're close to the best position, slow down
		temp = chook->best_dir * chook->speed * CAMERA_DECELERATE;
	else
	{
		// otherwise, go right for the best viewpoint
		temp = chook->best_dir * chook->speed * CAMERA_ACCELERATE;
		if ( temp.Length() > CAMERA_MAXSPEED )
		{
			temp = chook->best_dir * 160.0;
		}
	}

	// add the view object's velocity to ours
	temp = temp + camera->goalentity->velocity;
	camera->velocity = temp;

	camera_track_client (camera);
}

//************************* Camera Mode 100 *******************************//

///////////////////////////////////////////////////////////////////////////////
//	camera_cinematic
//
///////////////////////////////////////////////////////////////////////////////

void camera_cinematic(userEntity_t *camera) 
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;	

    //check if the cinematic is still running.

    //get the current position and direction of the camera in the cinematic.
    gstate->CIN_CamLocation(camera->s.origin, camera->s.angles);

	gstate->SetOrigin2(camera, camera->s.origin);

    gstate->SetClientAngles(camera->owner, camera->s.angles);

	camera_save_target_info (camera);
}


////////////////////////////////////////////////////////////////////////////
//	end of camera think functions
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	camera_cast_out
//
//	returns control to camera->goalentity from camera->owner
///////////////////////////////////////////////////////////////////////////////

void	camera_cast_out (userEntity_t *camera)
{
	playerHook_t	*mhook = AI_GetPlayerHook( camera->goalentity );
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;

	// you can't cast yourself out, imbecile!!
	if(!mhook || !chook)
		return;
	if (camera->goalentity == camera->owner || !chook->demonic)
		return;

	// clear possession flag and pointer
	camera->goalentity->flags -= (camera->goalentity->flags & FL_POSSESSED);
	mhook->demon = NULL;

	// give them back their brains
	camera->goalentity->think = chook->target_think;
	camera->goalentity->nextthink = gstate->time + 0.1;

	mhook->ai_flags = chook->target_ai_flags;
	mhook->th_state = chook->target_th_state;

	// restore the old movetype
	camera->goalentity->movetype = chook->target_movetype;

	// if another client, then restore their pre and post thinks
	if (camera->goalentity->flags & FL_CLIENT)
	{
		camera->goalentity->prethink = chook->target_prethink;
		camera->goalentity->prethink = chook->target_postthink;
		
		chook->target_prethink = NULL;
		chook->target_postthink = NULL;

		gstate->SetClientInputEntity (camera->owner, camera->owner);
	}

	// now give them back their body!
	gstate->SetClientInputEntity (camera->owner, camera->owner);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_possess
//
//	gives control of the camera->goalentity to camera->owner
///////////////////////////////////////////////////////////////////////////////

void	camera_possess (userEntity_t *camera)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;

	// you can't possess yourself, moron!!
	if (camera->goalentity == camera->owner || !chook->demonic)
		return;

	playerHook_t *mhook = AI_GetPlayerHook( camera->goalentity );

	if(!mhook)
		return;
	// set the new entity's possesser 
	mhook->demon = camera->owner;
	camera->goalentity->flags |= FL_POSSESSED;

	// make them stupid
	chook->target_think = camera->goalentity->think;
	camera->goalentity->think = NULL;

	chook->target_ai_flags = mhook->ai_flags;
	chook->target_th_state = mhook->th_state;
	mhook->ai_flags = 4;
	camera->goalentity->max_speed = mhook->run_speed / 320.0;

	// make sure we have a valid movetype
	chook->target_movetype = camera->goalentity->movetype;
	camera->goalentity->movetype = MOVETYPE_WALK;

	//	FIXME:	doesn't work
	// if another client, then take away their pre and post thinks
	if (camera->goalentity->flags & FL_CLIENT)
	{
		chook->target_prethink = camera->goalentity->prethink;
		chook->target_postthink = camera->goalentity->postthink;
		
		camera->goalentity->prethink = NULL;
		camera->goalentity->postthink = NULL;
	}

	//	set the input entity back to the camera's target, monsters only?
	gstate->SetClientInputEntity (camera->owner, camera->goalentity);
}

///////////////////////////////////////////////////////////////////////////////
//	camera_set_target
//
//	sets the camera's target to a specific entity
///////////////////////////////////////////////////////////////////////////////

void	camera_set_target (userEntity_t *camera, userEntity_t *target)
{
	cameraHook_t	*chook = (cameraHook_t *) camera->userHook;
	playerHook_t	*phook;

	if (camera->goalentity != target)
	{
		camera_cast_out (camera);

		if (camera->owner->flags & (FL_CLIENT + FL_MONSTER + FL_BOT))
		{
			phook = AI_GetPlayerHook( camera->owner );
			phook->camera = camera;
		}
		else
		{
			gstate->bprint ("camera target has no playerHook\n");
			return;
		}

		camera->goalentity = target;
		chook->next_change = -1;

		camera_possess (camera);

		camera_track (camera);
	}
}
int AI_IsInSmallSpace( userEntity_t *self);
///////////////////////////////////////////////////////////////////////////////
//	camera_next_monster
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////
void	camera_next_close_monster (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	cameraHook_t	*chook;	
	userEntity_t	*ent, *start_ent, *camera, *reserve, *sidekick1,*sidekick2, *client;
	float			entDistance = 9999;
	//NSS[11/30/99]:Noel(er..NULL) out everything.
	ent = start_ent = camera = reserve = sidekick1 = sidekick2 = client = NULL;
	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		camera_cast_out (camera);

		if (phook->camera->owner != camera->goalentity)
		{
			mhook = AI_GetPlayerHook( camera->goalentity );
			if(mhook)
			{
				mhook->camera = NULL;
			}
		}

		ent = gstate->NextEntity (phook->camera->goalentity);
		if (!ent) ent = gstate->FirstEntity ();
	
		start_ent = ent;
	
		while (1)
		{
			
			float distance = VectorDistance(self->s.origin,ent->s.origin);
			if ((ent->flags & FL_MONSTER || ent->flags & FL_BOT) && ent != camera)
			{
				playerHook_t	*hook = AI_GetPlayerHook( ent );
				if(!AI_IsInSmallSpace( ent ))
				{
					if ( (ent->flags & FL_CLIENT) && !(ent->deadflag & DEAD_PUNK) )
					{
						client = ent;
					}
					else
					if(hook)
					{
						GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
						if(pGoalStack)
						{
							TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
							if(pCurrentTask)
							{
								if(distance < 1500.0f && self->enemy != ent && AI_IsAlive(ent) && pCurrentTask->nTaskType != TASKTYPE_IDLE)
								{
									if(entDistance > distance)
									{
										if(camera->goalentity != ent)
										{
											entDistance = distance;
											self->enemy = ent;
										}
										else
										{
											reserve		= ent;
										}
									}
									
								}
								else if (AI_IsAlive(ent))
								{
									reserve = ent;
								}

							}
							//NSS[11/23/99]:IF we find a sidekick then this is the first thing we will want to watch when we are dead.
							if(hook)
							{
								if(hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKO || hook->type == TYPE_MIKIKOFLY)
								{
									if(AI_IsAlive(ent))
									{
										if(!sidekick1)
										{
											sidekick1 = ent;
										}
										else
										{
											sidekick2 = ent;
										}
									}
								}
							}
						}
					}
				}
			}

		
			ent = gstate->NextEntity (ent);
			if (!ent) ent = gstate->FirstEntity ();
	
			if (start_ent == ent)//If we have parsed through the list and we have found no monsters but we have found sidekicks go to them
			{
				if(reserve)
				{
					//We have looped through and found all entities but none really met our criteria
					//let's get the last best choice.
					ent = reserve;
					break;
				}
				else if (sidekick1 == NULL && sidekick2 == NULL)//If we have gone through the list and found nothing including sidekicks return out.
				{
					// we didn't find anything new
					return;
				}
				else //otheriwse we found stuff so keep on going.
				{
					break;
				}
			}
			
		}
		
		if(client && AI_IsAlive(client))//NSS[11/30/99]:That special moment when one of our pals eats the big turd stick and we get to watch ourself die.
		{
			ent = client;
		}
		else//NSS[12/1/99]:Setup to toggle between two sidekicks if we have them
		if(sidekick1 || sidekick2)
		{
			if(sidekick1 && sidekick2)//NSS[12/1/99]:If we do indeed have two let's figure out who to choose
			{
				 if(camera->goalentity == sidekick1)
				 {
					ent = sidekick2;
				 }
				 else if(camera->goalentity == sidekick2)
				 {
					ent = sidekick1;
				 }
				 else if(sidekick1)
				 {
					ent = sidekick1;
				 }
				 else if(sidekick2)
				 {
					ent = sidekick2;
				 }
				//Store the sidekick's enemy in our enemy pointer for later in case they get punked down
				self->enemy = ent->enemy;
			}
			else
			{
				if(sidekick1)
				{
					ent = sidekick1;
				}
				else if(sidekick2)
				{
					ent= sidekick2;
				}
			}
		}
		else if(self->enemy)
		{
			if(AI_IsAlive(self->enemy))
			{
				if(self->enemy->flags & (FL_BOT|FL_MONSTER|FL_CLIENT))
					ent = self->enemy;
			}
		
		}
		
		if(!ent) // if we found nothing do nothing.
			return;
		//if (ent->netname)
		//	gstate->sprint (self, "ChaseCam: following %s\n", ent->netname);
		//else
		//	gstate->sprint (self, "ChaseCam: following %s\n", ent->className);

		mhook = AI_GetPlayerHook( ent );
		if(mhook)
		{
			mhook->camera = camera;
		}
		camera->goalentity = ent;
		chook->next_change = -1;
		
		chook->follow_dist = 75.0f + (100*rnd());
		chook->max_speed   = 250.0f;

		camera_possess (camera);
	}

}


// ----------------------------------------------------------------------------
// NSS[2/11/00]:This is a quick hack to get this thing done!
// Name:        camera_first_dead_sidekick
// Description:Will find the first dead sidekick
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void camera_first_dead_sidekick(userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	cameraHook_t	*chook;	
	userEntity_t	*ent, *start_ent, *camera;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		camera_cast_out (camera);

		if (phook->camera->owner != camera->goalentity)
		{
			mhook = AI_GetPlayerHook( camera->goalentity );
			// NSS[1/26/00]:check the pointers!
			if(mhook)
			{
				mhook->camera = NULL;
			}
			else
			{
				return;
			}
		}

		ent = gstate->NextEntity (phook->camera->goalentity);
		if (!ent) ent = gstate->FirstEntity ();
	
		start_ent = ent;

		while (1)
		{
			if(deathmatch->value)
			{
				if (ent->flags & FL_CLIENT && AI_IsAlive(ent))
					break;
			}
			else
			{
				if (ent->flags & FL_BOT && !AI_IsAlive(ent))
					break;
			}
			
			ent = gstate->NextEntity (ent);
			if (!ent) ent = gstate->FirstEntity ();
	
			if (start_ent == ent)
				// looped through once and didn't find a monster
				return;
		}
	
		if (ent->netname)
			gstate->sprint (self, "ChaseCam: following %s\n", ent->netname);
		else
			gstate->sprint (self, "ChaseCam: following %s\n", ent->className);

		mhook = AI_GetPlayerHook( ent );
		mhook->camera = camera;

		camera->goalentity = ent;
		chook->next_change = -1;

		camera_possess (camera);
	}
}



// ----------------------------------------------------------------------------
// NSS[2/11/00]:
// Name:        camera_next_sidekick
// Description:Will cycle through the sidekicks and the player
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void camera_next_sidekick(userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	cameraHook_t	*chook;	
	userEntity_t	*ent, *start_ent, *camera;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		camera_cast_out (camera);

		if (phook->camera->owner != camera->goalentity)
		{
			mhook = AI_GetPlayerHook( camera->goalentity );
			// NSS[1/26/00]:check the pointers!
			if(mhook)
			{
				mhook->camera = NULL;
			}
			else
			{
				return;
			}
		}

		ent = gstate->NextEntity (phook->camera->goalentity);
		if (!ent) ent = gstate->FirstEntity ();
	
		start_ent = ent;

		while (1)
		{
			if (ent->flags & FL_BOT || ent == camera->owner || (ent->flags & FL_CLIENT && ent != camera))
				break;
			
			ent = gstate->NextEntity (ent);
			if (!ent) ent = gstate->FirstEntity ();
	
			if (start_ent == ent)
				// looped through once and didn't find a monster
				return;
		}
	
		if (ent->netname)
			gstate->sprint (self, "ChaseCam: following %s\n", ent->netname);
		else
			gstate->sprint (self, "ChaseCam: following %s\n", ent->className);

		mhook = AI_GetPlayerHook( ent );
		mhook->camera = camera;

		camera->goalentity = ent;
		chook->next_change = -1;

		camera_possess (camera);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_next_monster
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_next_monster (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	cameraHook_t	*chook;	
	userEntity_t	*ent, *start_ent, *camera;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		camera_cast_out (camera);

		if (phook->camera->owner != camera->goalentity)
		{
			mhook = AI_GetPlayerHook( camera->goalentity );
			// NSS[1/26/00]:check the pointers!
			if(mhook)
			{
				mhook->camera = NULL;
			}
			else
			{
				return;
			}
		}

		ent = gstate->NextEntity (phook->camera->goalentity);
		if (!ent) ent = gstate->FirstEntity ();
	
		start_ent = ent;

		while (1)
		{
			if (ent->flags & FL_MONSTER || ent->flags & FL_BOT || ent == camera->owner || (ent->flags & FL_CLIENT && ent != camera))
				break;
			
			ent = gstate->NextEntity (ent);
			if (!ent) ent = gstate->FirstEntity ();
	
			if (start_ent == ent)
				// looped through once and didn't find a monster
				return;
		}
	
		if (ent->netname)
			gstate->sprint (self, "ChaseCam: following %s\n", ent->netname);
		else
			gstate->sprint (self, "ChaseCam: following %s\n", ent->className);

		mhook = AI_GetPlayerHook( ent );
		mhook->camera = camera;

		camera->goalentity = ent;
		chook->next_change = -1;

		camera_possess (camera);
	}

}

///////////////////////////////////////////////////////////////////////////////
//	camera_prev_monster
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_prev_monster (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	cameraHook_t	*chook;	
	userEntity_t	*last_ent, *ent, *camera;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		camera_cast_out (camera);		

		if (phook->camera->owner != camera->goalentity)
		{
			mhook = AI_GetPlayerHook( camera->goalentity );
			mhook->camera = NULL;
		}

		last_ent = camera->goalentity;
		ent = gstate->NextEntity (camera->goalentity);
	
		if (!ent) ent = gstate->FirstEntity ();

		while (1)
		{
			if (ent->flags & FL_MONSTER || ent->flags & FL_BOT || ent == camera->owner || (ent->flags & FL_CLIENT && ent != camera))
				last_ent = ent;

			ent = gstate->NextEntity (ent);
			if (!ent) ent = gstate->FirstEntity ();
		
			if (ent == camera->goalentity)
				// back to where we started
				break;
		}
	
		if (last_ent == camera->goalentity)
			return;

		ent = last_ent;
		
		if (ent->netname)
			gstate->sprint (self, "ChaseCam: following %s\n", ent->netname);
		else
			gstate->sprint (self, "ChaseCam: following %s\n", ent->className);

		mhook = AI_GetPlayerHook( ent );
		mhook->camera = camera;

		camera->goalentity = ent;
		chook->next_change = -1;

		camera_possess (camera);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_debug
//
//	toggles FL_DEBUG on the camera's target
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_debug (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	CVector			org;
	userEntity_t	*goal, *target;
	userEntity_t	*owner, *camera;

	// make sure client is actually connected
	if (!phook)
		return;
	
	if (phook->camera)
	{
		camera = phook->camera;
		target = camera->goalentity;
		goal = target->goalentity;

		if (!(target->flags & FL_CLIENT))
		{
			org = goal->s.origin;
			owner = goal->owner;
		}
		
		if (target->flags & FL_DEBUG)
		{
			target->flags -= FL_DEBUG;
			gstate->Con_Printf ("FL_DEBUG info OFF\n");

			if (!(target->flags & FL_CLIENT))
			{
				//	if they don't have a fake goal, remove the real entity and
				//	replace it with a fake one
				if (!(goal->flags & FL_FAKEENTITY))
				{
					goal->remove (goal);

					goal = ai_spawn_goal (target);
					goal->s.origin = org;
				}
			}
		}
		else
		{
			target->flags |= FL_DEBUG;
			gstate->Con_Printf ("FL_DEBUG info ON\n");

			if (!(target->flags & FL_CLIENT))
			{
				//	if they have a fake goal, remove it and replace it with
				//	a goalentity that we can see
#ifdef _DEBUG
				if (goal->flags & FL_FAKEENTITY)
				{
					gstate->bprint ("setting up visible goalentity.\n");
//					delete goal;
					gstate->RemoveEntity( goal );
					goal = NULL;

					// set up the bot's roam_goal which is his carrot-on-a-string
					goal = gstate->SpawnEntity ();
					goal->solid = SOLID_NOT;
					goal->movetype = MOVETYPE_NONE;
					goal->flags = 0;
					goal->s.modelindex = gstate->ModelIndex (debug_models [DEBUG_GOAL]);
					goal->s.mins.Zero();
					goal->s.maxs.Zero();
					goal->s.origin = org;
					gstate->LinkEntity (goal);
				}
#endif
			}
		}

		//	clients don't have a goal!
		if (!(target->flags & FL_CLIENT))
		{
			goal->owner = owner;
			target->goalentity = goal;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_toggle_possession
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_toggle_possession (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	userEntity_t	*camera;
	cameraHook_t	*chook;

	// make sure client is actually connected
	if (!phook)
		return;
	
	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) phook->camera->userHook;

		if (chook->demonic)
		{
			camera_cast_out (camera);
			chook->demonic = FALSE;

			gstate->sprint (self, "ChaseCam: possession disabled.\n");
		}
		else
		{
			chook->demonic = TRUE;
			camera_possess (camera);

			gstate->sprint (self, "ChaseCam: possession enabled.\n");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_mode
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_mode (userEntity_t *self, int cmode, int showMsg)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	
	if (phook->camera)
	{
		userEntity_t	*camera = phook->camera;
		cameraHook_t	*chook = (cameraHook_t *) phook->camera->userHook;

		//	can't mode switch when locked
		if (cmode >= 128)
			return;

		//NSS[10/26/99]:Trap for special case monitor.
		if(cmode != CAMERAMODE_MONITOR)
			if (cmode == chook->mode && chook->mode != CAMERAMODE_NEXT)
				return;

		////////////////////////////////////////////////////////////////////
		//	if CAMERAMODE_NEXT is passed, then the mode is just incremented 
		////////////////////////////////////////////////////////////////////

		if (cmode == CAMERAMODE_NEXT)
		{
			chook->mode++;

			if (chook->mode == CAMERAMODE_MONITOR || chook->mode == CAMERAMODE_NEXT)
				chook->mode = 0;
		}
		else
			chook->mode = cmode;

		switch (chook->mode)
		{
			case	CAMERAMODE_BESTCHASE:
				//	chooses the best point to view the target from, with a
				//	slight bias to the back of the client
				//	lock all angles to camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z);

				chook->ideal_angle = CVector(0, 90, 0);
				chook->view_angle = CVector(0, 90, 0);
				chook->next_change = -1;
				chook->min_change_time = 1.0;
				chook->follow_dist = 80.0;

				chook->pre_think = camera_check_target;
				chook->post_think = camera_auto_chase;
				chook->best_org = camera->s.origin;

				// reset angles so target doesn't get stuck if possessed
				camera->goalentity->s.angles.x = 0.0;
				camera->goalentity->s.angles.z = 0.0;

				camera->s.event = EV_ENTITY_FORCEMOVE;

				break;
			case	CAMERAMODE_MONITOR:
				// monitor mode goes into letterbox.. let's turn it on now
#pragma message( "// SCG[10/27/99]: This wont work in multiplayer!" )
				if (chook->mode == CAMERAMODE_MONITOR)
				{
//					gstate->SetCvar("scr_letterbox","1");	
					self->client->ps.rdflags |= RDF_LETTERBOX;
				}
				//	simply turns a camera to track its target
				//	lock all angles to camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z | CAMFL_NOLERP);
				gstate->LinkEntity(self);
				chook->next_change = -1;
				chook->min_change_time = 1.0;
				chook->follow_dist = 80.0;

				chook->best_org = camera->s.origin;

				camera->s.event = EV_ENTITY_FORCEMOVE;
				chook->pre_think = camera_check_target;
				chook->post_think = camera_track_client;
				break;

			case	CAMERAMODE_TEST1:
				//	lock all angles to camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z);

				chook->view_angle = CVector(0, 0, 0);				
				chook->follow_dist = 128.0;

				chook->pre_think = camera_follow;
				chook->post_think = camera_track_client;
				break;

			case	CAMERAMODE_TEST2:
				//	lock all angles to camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z);

				chook->follow_dist = 128.0;

				chook->pre_think = camera_check_target;
				chook->post_think = camera_rotate;
				break;

			case	CAMERAMODE_GIBCAM:
				//	gib cam
				//	lock all angles to camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z);

				chook->ideal_angle = CVector(15, 90, 0);
				chook->view_angle = CVector(15, 90, 0);
				chook->next_change = -1;
				chook->min_change_time = 1.0;
				chook->follow_dist = 156.0;

				chook->pre_think = camera_check_target;
				chook->post_think = camera_auto_chase;

				// reset angles so target doesn't get stuck if possessed
				camera->goalentity->s.angles.x = 0.0;
				camera->goalentity->s.angles.z = 0.0;
				break;

            case CAMERAMODE_CINEMATIC:
                //cinematic camera
				gstate->SetCameraState (self, true, CAMFL_LOCK_X | CAMFL_LOCK_Y | CAMFL_LOCK_Z);

                chook->pre_think = camera_check_target;
                chook->post_think = camera_cinematic;
                break;

			default:
				//	MODE 0

				//	only let pitch movement translate
				gstate->SetCameraState (self, true, CAMFL_LOCK_Y | CAMFL_LOCK_Z);

				chook->ideal_angle = CVector(0, 0, 0);
				chook->view_angle = CVector(15, 0, 0);
				chook->next_change = -1;
				chook->min_change_time = 0.5;
				chook->follow_dist = 80.0;
				chook->max_speed = CAMERA_MAXSPEED * 2.0;

				chook->pre_think = camera_check_target;
				chook->post_think = camera_place_behind;

				/*
				chook->view_angle = CVector (15, 0, 0);				
				chook->follow_dist = 128.0;					

				chook->pre_think = camera_place_behind;
				chook->post_think = camera_track_yaw;
				*/
				break;
		}

		if (showMsg)
			gstate->Con_Printf ("camera mode %i\n", chook->mode);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_start
//
//  creates a view entity
//	behind self
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_start (userEntity_t *target, int showMsg)
{
	CVector			org, temp;
	cameraHook_t	*chook;
	playerHook_t	*phook = AI_GetPlayerHook( target );
	userEntity_t	*camera;

	camera = gstate->SpawnEntity ();

	camera->solid = SOLID_NOT;
	camera->movetype = MOVETYPE_FLY;
	camera->className = "camera";
	camera->flags = FL_CLIENT;
	camera->svflags |= SVF_DEADMONSTER;
	camera->owner = target;	// who started the camera

	camera->goalentity = target;	// entity the camera is following
	camera->s.effects = 0;
	camera->viewheight = 0;
	camera->s.event = EV_ENTITY_FORCEMOVE;

	camera->view_ofs.Zero();

	// SCG[11/24/99]: Save game stuff
	camera->save = camera_hook_save;
	camera->load = camera_hook_load;

#ifdef _DEBUG
	camera->s.modelindex = gstate->ModelIndex (debug_models [DEBUG_LASTNODE]);
#endif
	camera->s.renderfx |= RF_NODRAW;
//	camera->modelName = NULL;
	gstate->SetSize (camera, 0, 0, 0, 0, 0, 0);
	camera->s.origin = target->s.origin;
	gstate->LinkEntity (camera);

	camera->userHook = gstate->X_Malloc(sizeof(cameraHook_t),MEM_TAG_HOOK);
	camera->client = ( gclient_t * ) gstate->X_Malloc( sizeof( gclient_t ), MEM_TAG_MISC );
	chook = (cameraHook_t *) camera->userHook;

	chook->mode = CAMERAMODE_DEFAULT;
	chook->demonic = FALSE;

	// give the client a point to the camera
	phook->camera = camera;

	gstate->SetClientViewEntity (target, camera);

	if (chook->demonic)
		gstate->SetClientInputEntity (target, camera->goalentity);
	else
		gstate->SetClientInputEntity (target, camera->owner);

	camera_mode (target, CAMERAMODE_NEXT, showMsg);

	camera_place_behind (camera);

	if (showMsg)
		gstate->sprint (target, "ChaseCam activated\n");
}

///////////////////////////////////////////////////////////////////////////////
//	camera_stop
//
//	sets view ent back to camera->userHook->master
//	and removes camera
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_stop (userEntity_t *self, int showMsg)
{
	playerHook_t	*phook;

    // Check parms, recover if something sends this dll unexpected info,
    // if DEVELOPER is set, notify
	if(! self)
	{
		gstate->Con_Dprintf("camera_stop():%s:%i:bad param 1 == NULL\n", __FILE__, __LINE__);
		return;
	}

    if(! (phook = AI_GetPlayerHook( self )))
    {
        gstate->Con_Dprintf("camera_stop():%s:%i:self->userHook == NULL\n", __FILE__, __LINE__);
        return;
    }

    if (phook->camera)
	{
		userEntity_t	*camera = phook->camera;
		cameraHook_t	*chook = (cameraHook_t *) camera->userHook;
		playerHook_t	*ghook = NULL;
		CVector			temp;

		if (! camera->goalentity)
		{
			// if developer is set, notify the coder of a bad goalentity and dodge the affected execution path
			gstate->Con_Dprintf("camera_stop():%s:%i:camera->goalentity == NULL\n", __FILE__, __LINE__);
		} 
		else	
		{
			if (camera->goalentity->flags & FL_FAKEENTITY)
				//	remove target if it is a fake entity
				gstate->RemoveFakeEntity (camera->goalentity);
			else
			{
	            ghook = (playerHook_t *)phook->camera->goalentity->userHook;

				//	remove the target's camera
				if (ghook && camera->goalentity->flags & (FL_CLIENT | FL_MONSTER | FL_BOT))
					ghook->camera = NULL;

				if (camera->goalentity->flags & FL_POSSESSED)
					camera_cast_out (camera);
			}
		}

		// monitor mode goes into letterbox.. let's turn it off now
		if(chook)	// this can be NULL in some situations
		{
			if (chook->mode == CAMERAMODE_MONITOR)
			{
//				gstate->SetCvar("scr_letterbox","0");	
				self->client->ps.rdflags &= ~RDF_LETTERBOX;
			}
		}

		//	turn off the camera
		gstate->SetCameraState (self, false, 0);

		// move view back to default entity
		gstate->SetClientViewEntity (self, camera->owner);
		
		// set the input entity back to the client
		gstate->SetClientInputEntity (self, camera->owner);

		temp = self->s.angles;
		temp.z = 0;
		camera_set_viewangle(self, temp);

		if( camera->remove )
		{
			// free the allocated client struct
			if (camera->client)
			{
				gstate->X_Free(camera->client);
				camera->client = NULL;
			}

			camera->remove (camera);
		}

		//	remove the client's camera
		phook->camera = NULL;

		if (showMsg)
		gstate->sprint (self, "ChaseCam deactivated\n");

//		self->s.renderfx = RF_NODRAW;

		self->client->damage_alpha = 0;
		self->client->flash_alpha = 0;
		self->client->bonus_alpha = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_toggle_gravity
//
//	ONLY CALLED WITH SELF == CLIENT
///////////////////////////////////////////////////////////////////////////////

void	camera_toggle_gravity (userEntity_t *self)
{
	gstate->gravity_dir.z *= -1;
}

///////////////////////////////////////////////////////////////////////////////
//	cinematic camera stuff
///////////////////////////////////////////////////////////////////////////////

//the camera mode we were in before we went into cinematic camera mode.
static int cin_camera_mode_before = 0;
//1 if we were in any camera mode before we started cinematics.
static int cin_camera_mode_started = 0;

///////////////////////////////////////////////////////////////////////////////
//	camera_cinematic_on
//
///////////////////////////////////////////////////////////////////////////////

void camera_cinematic_on(userEntity_t *self, cameraHook_t *chook, userEntity_t *camera) 
{
    if (self == NULL || chook == NULL) return;

    //save the current mode.
    cin_camera_mode_before = chook->mode;

    //check if we are in a camera mode.
    if (camera == NULL) 
	{
        cin_camera_mode_started = 0;
    }
    else 
	{
        cin_camera_mode_started = 1;
    }

    camera_mode(self, CAMERAMODE_CINEMATIC, gstate->GetCvar ("developer") );
}

///////////////////////////////////////////////////////////////////////////////
//	camera_cinematic_off
//
///////////////////////////////////////////////////////////////////////////////

void camera_cinematic_off(userEntity_t *self, cameraHook_t *chook) 
{
    if (self == NULL) return;

    //check if we were in a camera mode.
    if (cin_camera_mode_started == 0) 
	{
        //just stop the camera mode.
        camera_stop(self, gstate->GetCvar ("developer") );
    }
    else 
	{
        //restore the previous mode.
        camera_mode(self, cin_camera_mode_before, gstate->GetCvar ("developer") );
    }
}

///////////////////////////////////////////////////////////////////////////////
//	check_cinematic_camera
//
///////////////////////////////////////////////////////////////////////////////

bool check_cinematic_camera(userEntity_t *self, cameraHook_t *chook, userEntity_t *camera) 
{
    if (self == NULL || chook == NULL) return false;

    //check if we are enabling/disabling the cinematic camera
    if (gstate->GetArgc() == 2 && stricmp(gstate->GetArgv(1), "cin_on") == 0) 
	{
        //turn on the cinematic camera
        camera_cinematic_on(self, chook, camera);
        return true;
    }
    if (gstate->GetArgc() == 2 && stricmp(gstate->GetArgv(1), "cin_off") == 0) 
	{
        //turn off the cinematic camera.
        camera_cinematic_off(self, chook);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//	camera_toggle
//
//	ONLY CALLED WITH SELF == CLIENT
//
//	toggles camera on, between modes, and off again
///////////////////////////////////////////////////////////////////////////////

void	camera_toggle (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	cameraHook_t	*chook;

	// make sure client is actually connected
	if (!phook || self->deadflag != DEAD_NO)
		return;

    //check if we are already in a camera mode.
	if (phook->camera) 
	{
        //camera mode already started.
		chook = (cameraHook_t *) phook->camera->userHook;

        if(!chook)
			return;
		if (check_cinematic_camera(self, chook, phook->camera) == true) return;

		if (chook->mode < CAMERAMODE_BESTCHASE)
			camera_mode (self, CAMERAMODE_NEXT, gstate->GetCvar ("developer") );
		else
			camera_stop (self, gstate->GetCvar ("developer") );
	}
    else 
	{
        //start the camera mode.
		camera_start (self, gstate->GetCvar ("developer") );
		camera_mode (self, CAMERAMODE_DEFAULT, gstate->GetCvar ("developer") );

        chook = (cameraHook_t *) phook->camera->userHook;
        if (check_cinematic_camera(self, chook, NULL) == true) return;
    }
}

///////////////////////////////////////////////////////////////////////////////
//	camera_set_debug_target
//
//	ONLY CALLED WITH SELF == CLIENT
//	sets the current goalentity to the debug target
///////////////////////////////////////////////////////////////////////////////

void	camera_set_debug_target (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	userEntity_t	*camera;
	cameraHook_t	*chook;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		chook->debug_target = camera->goalentity;
		gstate->sprint (self, "Debug target set to %s\n", chook->debug_target->className);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_toggle_debug_target
//
//	ONLY CALLED WITH SELF == CLIENT
//	toggles between the debug target and the camera owner
///////////////////////////////////////////////////////////////////////////////

void	camera_toggle_debug_target (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	cameraHook_t	*chook;
	userEntity_t	*camera;
	playerHook_t	*ghook;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		if (chook->debug_target)
		{
			if (camera->goalentity == camera->owner)
				camera->goalentity = chook->debug_target;
			else
			{
				ghook = AI_GetPlayerHook( phook->camera->goalentity );
				//	remove the target's camera
				ghook->camera = NULL;
		
				camera->goalentity = camera->owner;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_kill_target
//
//	ONLY CALLED WITH SELF == CLIENT
//	breaks the camera's target in half
///////////////////////////////////////////////////////////////////////////////

void	camera_kill_target (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	userEntity_t	*camera;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;

		if (camera->goalentity != camera->owner)
		{
			if (camera->goalentity->flags &  (FL_MONSTER + FL_BOT))
			{
				camera->goalentity->die (camera->goalentity, self, self, 32768, camera->goalentity->s.origin);
			}
			else
			{
				camera->goalentity->remove (camera->goalentity);
				camera->goalentity = camera->owner;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	camera_lock
//
//	ONLY CALLED WITH SELF == CLIENT
//	locks the camera at it's current position
//	the camera can still track its target's movement by turning, but cannot move
///////////////////////////////////////////////////////////////////////////////

void	camera_lock (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	cameraHook_t	*chook;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		chook = (cameraHook_t *) phook->camera->userHook;
		
		if (chook->mode >= 128)
			chook->mode -= 128;
		else
		{
			chook->mode += 128;
			phook->camera->velocity.Zero();
		}
	}
}

void camera_register_func(void)
{
	gstate->RegisterFunc("camera_hook_save",camera_hook_save);
	gstate->RegisterFunc("camera_hook_load",camera_hook_load);
}

#if 0
///////////////////////////////////////////////////////////////////////////////
//	camera_force_enemy
//
//	ONLY CALLED WITH SELF == CLIENT
//
//	forces the camera's target (if a monster or bot) to consider
//	the camera's owner its enemy., even if the target can't see
//	the camera's owner
///////////////////////////////////////////////////////////////////////////////

void	camera_force_enemy (userEntity_t *self)
{
	playerHook_t	*phook = AI_GetPlayerHook( self );
	playerHook_t	*mhook;
	userEntity_t	*camera;
	cameraHook_t	*chook;

	// make sure client is actually connected
	if (!phook)
		return;

	if (phook->camera)
	{
		camera = phook->camera;
		chook = (cameraHook_t *) camera->userHook;

		if (camera->goalentity != camera->owner)
		{
			if (camera->goalentity->flags &  (FL_MONSTER + FL_BOT))
			{
				mhook = AI_GetPlayerHook( camera->goalentity );
				if (mhook->sight_sound) 
					mhook->sight_sound (camera->goalentity);

				camera->goalentity->enemy = camera->owner;
				ai_set_goal (camera->goalentity, camera->goalentity->enemy, camera->goalentity->enemy->s.origin, GOAL_ENEMY);

				gstate->Con_Printf ("Forcing %s's enemy to %s.\n", camera->goalentity->className, camera->owner->className);

				if (!ai_go_to_enemy (camera->goalentity))
					//	can't go for some reason
					return;
				else if (mhook->sight)
					mhook->sight (camera->goalentity);
			}
		}
	}
}

#endif 0
