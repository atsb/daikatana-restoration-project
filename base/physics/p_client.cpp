///////////////////////////////////////////////////////////////////////////////
//	client.c	
//
//	physics routines for Clients
///////////////////////////////////////////////////////////////////////////////

#include	"p_global.h"
#include	"p_user.h"
#include  "hooks.h"
#include  "p_inventory.h"

///////////////////////////////////////////////////////////////////////////////
// view pitching times
///////////////////////////////////////////////////////////////////////////////


#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	push_s
{
	CVector	wish_vel;	//	desired velocity of player
	CVector	forward;	//	direction or travel 
	CVector	right;		//	right of direction of travel
	edict_t	*client;	//	client who is pushing
} push_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	push_t	push;

static	edict_t	*pm_passent;

static	edict_t		*current_player;
static	gclient_t	*current_client;

static	CVector	v_forward, v_right, v_up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

static	int	clsfx_inSlime = 0;
static	int	clsfx_inLava = 0;
static  int clsfx_startRun = 0;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void P_RunSyncFrame (edict_t *client);

/* ------------------------------ Client_Think ------------------------------- */

#define	CLIENT_THINK_IN_WORLD

///////////////////////////////////////////////////////////////////////////////
// pmove doesn't need to know about passent and contentmask
///////////////////////////////////////////////////////////////////////////////
trace_t	PM_TraceLine (CVector &start, CVector &end)
{
	trace_t		trace;
	CVector		dir, move_dir, move;
	edict_t		*groundEntity;

	//	called because player is jumping.  Try to give groundEnt some velocity
	memset (&trace, 0x00, sizeof (trace_t));

	groundEntity = (edict_t *) &end;

	if ((groundEntity == g_edicts) || 
		!groundEntity ||
		(groundEntity->flags & FL_CLIENT) ||
		(groundEntity->solid == SOLID_BSP)||	//&&& AMW 7.29.98 - added so it would not push off of trains etc..
		!(groundEntity->flags & FL_PUSHABLE))
	{
		return	trace;
	}

	start.Multiply( -1.0f );

	//	push off of our ground entity!
	P_MomentumPush (push.client, groundEntity, start);

	//	only needs to be moved, no thinking!
	P_RunEntity (groundEntity, false);

	return	trace;
}

///////////////////////////////////////////////////////////////////////////////
// pmove doesn't need to know about passent and contentmask
///////////////////////////////////////////////////////////////////////////////
trace_t	PM_TraceBox (CVector &start, CVector &mins, CVector &maxs, CVector &end)
{
//.	trace_t		trace;
//	int			pushes;
//	float		dp, move_dist, dist;
	CVector		dir, move_dir, move;

/*
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
*/
/*
	if ( pm_passent->health > 0 )
	{
		for ( pushes = 0; pushes < 4; pushes++ )
		{
			trace = gi.TraceBox (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
			if ((trace.ent && trace.ent->flags & FL_CLIENT) || (!trace.ent))
			{
				//	client cannot push each other by running
				return	trace;
			}

			if (trace.fraction != 1.0 && trace.ent != g_edicts && 
				trace.ent->flags & FL_PUSHABLE && 
				trace.ent != push.client->groundEntity)
			{
				//	find direction vector from pusher to pushed object
				//	if pushing direction is not toward object, then don't push it!
				move_dir = end - start;
				move_dist = move_dir.Normalize();
				dir = trace.ent->s.origin - push.client->s.origin;
				dir.Normalize();

				if ( dir.z < -0.95 || move_dir.z < -0.95 )
				{
					break;
				}
				dp = DotProduct (dir, move_dir);
				if (dp < 0.8660)
				{
					break;
				}

				P_MomentumPush (push.client, trace.ent, push.client->velocity);

				//	only needs to be moved, no thinking!
				P_RunEntity (trace.ent, false);

				//	if entity didn't move, then don't try to push it more
				if (trace.ent->s.old_origin.x == trace.ent->s.origin.x &&
					trace.ent->s.old_origin.y == trace.ent->s.origin.y &&
					trace.ent->s.old_origin.z == trace.ent->s.origin.z)
				{
					break;
				}
				else
				{
					//	recalculate trace end position based on how far ent moved
					move = trace.ent->s.origin - trace.ent->s.old_origin;
					dist = move.Normalize();
					trace.fraction = (dist / move_dist) - 0.0125;
					if (trace.fraction > 1.0)
						//	object was pushed at least as far as player wants
						//	to move
					{
						trace.fraction = 1.0;
						memset (&trace.plane, 0x00, sizeof (cplane_t));
						trace.surface = NULL;
						trace.endpos = trace.ent->s.old_origin + move;
						trace.ent = NULL;
					}
					else
					{
						VectorMA( trace.ent->s.old_origin, move, trace.fraction, trace.endpos );
					}
				}
			}
			else
			{
				break;
			}
		}

		return	trace;
	}
*/
	if (pm_passent->health > 0)
		return gi.TraceBox (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.TraceBox (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

///////////////////////////////////////////////////////////////////////////////
//	someone help me
///////////////////////////////////////////////////////////////////////////////

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}

///////////////////////////////////////////////////////////////////////////////
//	Nelno must kill now
///////////////////////////////////////////////////////////////////////////////

void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	gi.Con_Dprintf("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SetCameraState
//
//	turns camera mode on or off
//
//	TODO:	add flags for x, y, z control separation
///////////////////////////////////////////////////////////////////////////////

void	Client_SetCameraState (userEntity_t *ent, int state, int flags)
{
	if (state)
		ent->client->ps.pmove.pm_flags |= PMF_CAMERAMODE;
	else
		ent->client->ps.pmove.pm_flags &= ~PMF_CAMERAMODE;

	if (flags & CAMFL_LOCK_X)
		ent->client->ps.pmove.pm_flags |= PMF_CAMERA_LOCK_X;
	if (flags & CAMFL_LOCK_Y)
		ent->client->ps.pmove.pm_flags |= PMF_CAMERA_LOCK_Y;
	if (flags & CAMFL_LOCK_Z)
		ent->client->ps.pmove.pm_flags |= PMF_CAMERA_LOCK_Z;
	if (flags & CAMFL_NOLERP)
		ent->client->ps.pmove.pm_flags |= PMF_CAMERA_NOLERP;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SetInputEntity
//
///////////////////////////////////////////////////////////////////////////////

void	Client_SetInputEntity (userEntity_t *client, userEntity_t *inputEntity)
{
	client->input_entity = client;
//	gi.Con_Dprintf("Client_SetInputEntity: I am a stub, ass.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SetViewEntity
//
///////////////////////////////////////////////////////////////////////////////

void	Client_SetViewEntity (userEntity_t *clientEntity, userEntity_t *viewEntity)
{
	if (clientEntity == viewEntity)
		clientEntity->client->ps.view_entity = 0;
	else
		clientEntity->client->ps.view_entity = ((byte *)(viewEntity) - (byte *)(g_edicts)) / sizeof (userEntity_t);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SetClientAngles
//
//	turns camera mode on or off
///////////////////////////////////////////////////////////////////////////////

void	Client_SetClientAngles (userEntity_t *ent, CVector &angle)
{
	ent->client->fix_angles = angle;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_Think
//	
//	This will be called once for each client frame, which will
//	usually be a couple times for each server frame.
///////////////////////////////////////////////////////////////////////////////

void Client_Think (userEntity_t *ent, usercmd_t *ucmd)
{
	gclient_t		*client;
	userEntity_t	*other;
	int				i, j;
	pmove_t			pm;
	CVector			push_velocity;	//	velocity before impact clipping
	float			last_physics;
    CVector         vecPreVelocity, vecPostVelocity;    // Logic[4/28/99]: Check for move start
    playerHook_t    *hook;  // Logic[4/28/99]: play "whoosh" if speed boost is active and player starts moving
	level.current_entity = ent;
	client = ent->client;

	if( client->needsUpdate != 0 )
	{
		if (client->needsUpdate & HUD_UPDATE_INVEN)
		{
			// SCG[1/19/00]: check for savegems and inform client if necessary
			int nNumSavegems = serverState.InventoryItemCount( ent->inventory,"item_savegem" );
			{
				serverState.WriteByte( SVC_SAVEGEM );
				serverState.WriteByte( false );
				serverState.WriteByte( nNumSavegems );
				serverState.UniCast( ent, true );
			}

			P_InventoryUpdateClient( ent, false );
		}

		if (client->needsUpdate & HUD_UPDATE_WEAP)
			P_WeaponUpdateClient( ent, true );

		if (client->needsUpdate & HUD_UPDATE_BOOST)
		{
			playerHook_t *pHook = (playerHook_t *)ent->userHook;
			if (pHook)
			{
				if( pHook->power_boost_time )	com.Boost_Icons( ent, BOOSTICON_ADD, ICON_POWERBOOST, pHook->power_boost_time );
				if( pHook->attack_boost_time )	com.Boost_Icons( ent, BOOSTICON_ADD, ICON_ATTACKBOOST, pHook->attack_boost_time );
				if( pHook->speed_boost_time )	com.Boost_Icons( ent, BOOSTICON_ADD, ICON_SPEEDBOOST, pHook->speed_boost_time );
				if( pHook->acro_boost_time )	com.Boost_Icons( ent, BOOSTICON_ADD, ICON_ACROBOOST, pHook->acro_boost_time );
				if( pHook->vita_boost_time )	com.Boost_Icons( ent, BOOSTICON_ADD, ICON_VITABOOST, pHook->vita_boost_time );
			}
		}

		if (client->needsUpdate & HUD_UPDATE_XPLEVEL)
		{
			serverState.WriteByte(SVC_XPLEVEL);  //	send the 'level up' message to client
			serverState.UniCast(ent,true);      // broadcast
		}

		client->needsUpdate = 0;
	}

	//	handle intermissions
	if (level.intermissiontime)
	{
		//	force client to freeze
		client->ps.pmove.pm_type = PM_FREEZE;
		
		// can exit intermission after five seconds
		if ( (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) ) ||
			(deathmatch->value && (level.time > level.intermissiontime + 30)) )		// force continue in deathmatch
		{
/*
			client->endIntermission = TRUE;

			userEntity_t *self;
			qboolean fail = FALSE;
			for (i = 0; i < maxclients->value; i++)
			{
				self = g_edicts + 1 + i;

				if (!self->inuse)
					continue;

				if (!self->client->endIntermission)
				{
					fail = TRUE;
					break;
				}
			}

			if (!fail)*/
			{
//				ent->client->showflags &= ~(SHOW_SCORES_NOW|SHOW_SCORES);
				level.exitintermission = true;
			}
		}
		
		return;
	}


	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (client->ps.view_entity == 0)
		//	if view entity is player, use player's viewheight
		pm.viewheight = ent->viewheight;
	else
		//	use view entity's view height
		pm.viewheight = g_edicts [client->ps.view_entity].viewheight;

	if (ent->flags & FL_FORCEANGLES)
		client->ps.pmove.pm_type = PM_NOINPUT;
	else 
	if (ent->flags & FL_FREEZE)
		client->ps.pmove.pm_type = PM_FREEZEDEATH;
	else 
	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;

//	Nelno:	um, whatever...
//	else if (ent->s.modelindex != 255)
//		client->ps.pmove.pm_type = PM_GIB;

	else 
	if (ent->deadflag)// && !(client->ps.pmove.pm_flags & PMF_CAMERAMODE))
	{
		client->ps.pmove.pm_type = PM_DEAD;
	}
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = p_gravity->value;
	pm.s = client->ps.pmove;
	pm.max_speed = ent->max_speed;

	pm.s.origin[0] = ent->s.origin.x*8;
	pm.s.origin[1] = ent->s.origin.y*8;
	pm.s.origin[2] = ent->s.origin.z*8;
	pm.s.velocity[0] = ent->velocity.x*8;
	pm.s.velocity[1] = ent->velocity.y*8;
	pm.s.velocity[2] = ent->velocity.z*8;

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		pm.snapinitial = true;

	pm.cmd = *ucmd;

	//pm.trace = PM_trace;	// adds default parms
	pm.TraceLine		= PM_TraceLine;
	pm.TraceBox			= PM_TraceBox;
	pm.pointcontents	= gi.pointcontents;

	////////////////////////////////////////////////////////////////////////////
	//	set up push structure for pushing objects when player moves
	////////////////////////////////////////////////////////////////////////////
	CVector temp;
	AngleToVectors (client->ps.viewangles, push.forward, push.right, temp);

#ifdef CLIENT_ADDITIVE_VELOCITY

	push.wish_vel.x = push.forward.x * pm.cmd.forwardmove + push.right.x * pm.cmd.sidemove;
	push.wish_vel.y = push.forward.y * pm.cmd.forwardmove + push.right.y * pm.cmd.sidemove;

#else

	float fForward = pm.cmd.forwardmove;
	float fSide = pm.cmd.sidemove;
	if ( fabs(fForward) > 0.0f && fabs(fSide) > 0.0f )
	{
		fForward = fForward * 0.708f;
		fSide = fSide * 0.708f;
	}
	push.wish_vel.x = push.forward.x * fForward + push.right.x * fSide;
	push.wish_vel.y = push.forward.y * fForward + push.right.y * fSide;

#endif CLIENT_ADDITIVE_VELOCITY

	push.wish_vel.z = 0;
	push.client = ent;
	last_physics = (float)(gi.Sys_Milliseconds () - p_realtime) / 1000.0;
	p_frametime_scaled = last_physics * p_frametime->value;
	//	so that push physics don't screw up in debugger
	if (p_frametime_scaled > 0.2) 
	{
//		gi.conprintf ("p_client: clamped p_frametime_scaled!\n");
		p_frametime_scaled = 0.02;
	}

	//	save water level prior to move
	ent->client->old_waterlevel = ent->waterlevel;

	///////////////////////////////////////////////////////////////////////////
	//	move and clip the player
	///////////////////////////////////////////////////////////////////////////
    // Logic[4/28/99]: Check to see if player is beginning a move
    vecPreVelocity = ent->velocity;
	gi.Pmove (&pm);
	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;
	ent->groundSurface = pm.groundSurface;

	ent->s.origin.x = pm.s.origin[0] * 0.125f;
	ent->s.origin.y = pm.s.origin[1] * 0.125f;
	ent->s.origin.z = pm.s.origin[2] * 0.125f;
	
	ent->velocity.x = pm.s.velocity[0] * 0.125f;
	ent->velocity.y = pm.s.velocity[1] * 0.125f;
	ent->velocity.z = pm.s.velocity[2] * 0.125f;
    
    vecPostVelocity = ent->velocity;

	CVector vZero(0,0,0);
    if( (hook = (playerHook_t *)ent->userHook) && hook->speed_boost > 4)
    {
		if(!vecPreVelocity.Length() && (ucmd->forwardmove || ucmd->sidemove) && vecPostVelocity.Length())
		{
			gi.StartSound(vZero,ent, CHAN_BODY, clsfx_startRun, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX,0.0);
		}
	}
	
	push_velocity = ent->velocity * 4.0f;
	if (push_velocity.Length() > ent->max_speed)
	{
		push_velocity = push_velocity * (ent->max_speed / push_velocity.Length());
	}

	ent->s.mins = pm.mins;
	ent->s.maxs = pm.maxs;

	client->resp.cmd_angles.x = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles.y = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles.z = SHORT2ANGLE(ucmd->angles[2]);

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	if( ucmd->buttons & BUTTON_ATTACK )
	{
		client->latched_buttons |= client->buttons;			// mike: changed from above to this, so holding attack button
															//       constantly fires
	}
	else
	{
		client->latched_buttons |= client->buttons & ~client->oldbuttons;
	}

	//	set these now so that water damage can be done in ClientThink
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	//	check for splash
	if ((ent->client->old_waterlevel == 0 && ent->waterlevel) ||
		(ent->client->old_waterlevel < ent->waterlevel) && ent->velocity.z < -100)
	{
		int	speed = ent->velocity.Length();
		if (speed > 350)
			ent->client->clFlags |= CLF_BIGSPLASH;
		else 
		if (speed > 250)
			ent->client->clFlags |= CLF_MEDSPLASH;
		else 
		if (speed > 150)
			ent->client->clFlags |= CLF_SMALLSPLASH;
	}

	///////////////////////////////////////////////////////////////////////////////
	//	call dll_ClientThink
	///////////////////////////////////////////////////////////////////////////////

	dll_ClientThink (ent, ucmd, &pm);

	///////////////////////////////////////////////////////////////////////////////

	//	don't reset viewheight in camera mode
	if (!(client->ps.pmove.pm_flags & PMF_CAMERAMODE))
		ent->viewheight = pm.viewheight;

	ent->groundEntity = pm.groundEntity;
	
	if (pm.groundEntity)
		ent->groundEntity_linkcount = pm.groundEntity->linkcount;

	if (ent->flags & FL_FIXANGLES)
	{
		for (i = 0; i < 3; i++)
		{
			client->ps.pmove.delta_angles[i] = ANGLE2SHORT(client->fix_angles [i] - client->resp.cmd_angles [i]);
		}

		ent->s.angles.x = 0;
		ent->s.angles.y = client->fix_angles.y;
		ent->s.angles.z= 0;
	
		client->v_angle = client->fix_angles;
		client->ps.viewangles = client->fix_angles;

		//	always clear this flag each frame
		ent->flags -= FL_FIXANGLES;
	}
	else 
	if (client->ps.pmove.pm_flags & PMF_CAMERAMODE)
	{
		//	update view to fix_angles
		if (ent->flags & FL_FORCEANGLES)
		{
			client->v_angle = client->fix_angles;
		}
		else
		{
			client->v_angle = pm.viewangles;
		}

		if ( client->ps.pmove.pm_flags & PMF_CAMERA_LOCK_X )
		{
			client->ps.viewangles.x = client->fix_angles.x;
		}
		else
		{
			client->ps.viewangles.x = pm.viewangles.x;
		}

		if (client->ps.pmove.pm_flags & PMF_CAMERA_LOCK_Y)
		{
			client->ps.viewangles.y = client->fix_angles.y;
		}
		else
		{
			client->ps.viewangles.y = pm.viewangles.y;
		}

		if (client->ps.pmove.pm_flags & PMF_CAMERA_LOCK_Z)
		{
			client->ps.viewangles.z = client->fix_angles.z;
		}
		else
		{
			client->ps.viewangles.z = pm.viewangles.z;
		}
	}
	else 
	if (ent->deadflag)
	{
		client->ps.viewangles.z = 40;
		client->ps.viewangles.x = -15;
		client->ps.viewangles.y = client->ps.viewangles.y;
	}
	else 
	{
		client->v_angle = pm.viewangles;
		client->ps.viewangles = pm.viewangles;
	}

	gi.linkentity (ent);

	//	make client's body parts follow him around
//	hr_Move (ent);

	//	Q2FIXME:	touch triggers
	if (ent->movetype != MOVETYPE_NOCLIP)
		P_TouchTriggers (ent);

	//	touch other objects
	for (i = 0 ; i < pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j = 0 ; j < i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
/*
		if ((ent->solid != SOLID_NOT && other->solid != SOLID_NOT) &&
			(other != g_edicts))
		{
			if (other->flags & FL_PUSHABLE)
			{
				if (ent->movetype != MOVETYPE_NONE && other->movetype != MOVETYPE_NONE)
				{
					P_MomentumPush (ent, other, push_velocity);

					//	shouldn't run a think here
					P_RunEntity (other, false);
				}
			}
		}
*/
		if (!other->touch)
			continue;

		//	Q2FIXME:	touch functions now pass more parameters
		other->touch (other, ent, NULL, NULL);
	}

	// SCG[10/27/99]: For some rason, when use is latched two uses occur.
	// SCG[10/27/99]: This prevents that from happening...
	if( ucmd->buttons & BUTTON_USE )
	{
		client->latched_buttons &= ~BUTTON_USE;
	}

//	client->oldbuttons = client->buttons;
//	client->buttons = ucmd->buttons;
//	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_BeginServerFrame
//	
//	This will be called once for each server frame, before running
//	any other entities in the world.
///////////////////////////////////////////////////////////////////////////////

void Client_BeginServerFrame (edict_t *ent)
{
	//	clear event
	ent->s.event = 0;

	if (level.intermissiontime)
		//	in intermission, so exit
		return;

	dll_ClientBeginServerFrame (ent);

	if (ent->client)
		ent->client->latched_buttons = 0;
}



///////////////////////////////////////////////////////////////////////////////
//	P_CalcRoll
//
//	calculate client roll based on movement
///////////////////////////////////////////////////////////////////////////////

float P_CalcRoll (CVector &angles, CVector &velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct (velocity, v_right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = p_rollangle->value;

	if (side < p_rollspeed->value)
		side = side * value / p_rollspeed->value;
	else
		side = value;
	
	return side*sign;
	
}

///////////////////////////////////////////////////////////////////////////////
//	P_DamageFeedback
//
//	calculates view kickbacks, color blends for damage and plays pain anims
//
//	Q2FIXME:	needs serious work to work correctly with Daikatana DLLs
///////////////////////////////////////////////////////////////////////////////

void P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	side;
	float	realcount, count, kick;
	CVector	v;
//	int		r, l;
	static	CVector	power_color(0.0, 1.0, 0.0);
	static	CVector	acolor(1.0, 1.0, 1.0);
	static	CVector	bcolor(1.0, 0.0, 0.0);

	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor)
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	//	Q2KILL:	client animation crap
/*
	// start a pain animation
	if (client->anim_priority < ANIM_PAIN)
	{
		static int		i;

		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}
*/
	realcount = count;
	if (count < 10)
		count = 10;	// allways make a visible effect

	//	Q2KILL:	play pain sound
/*
	// play an apropriate pain sound
	if (level.time > player->pain_debounce_time)
	{
		r = 1 + (rand()&1);
		player->pain_debounce_time = level.time + 0.7;
		if (player->health < 25)
			l = 25;
		else if (player->health < 50)
			l = 50;
		else if (player->health < 75)
			l = 75;
		else
			l = 100;
		gi.sound (player, CHAN_VOICE, SexedSoundIndex(player, va("pain%i_%i", l, r)), 1, ATTN_NORM, 0);
	}
*/
	// the total alpha of the blend is allways proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	v.Zero();
	if (client->damage_parmor)
	{
		VectorMA( v, power_color, (float)client->damage_parmor/realcount, v );
	}
	if (client->damage_armor)
	{
		VectorMA( v, acolor, (float)client->damage_armor/realcount, v );
	}
	if (client->damage_blood)
	{
		VectorMA( v, bcolor, (float)client->damage_blood/realcount, v );
	}
	client->damage_blend = v;

	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count*0.5)
			kick = count*0.5;
		if (kick > 50)
			kick = 50;

		v = client->damage_from - player->s.origin;
		v.Normalize();
		
		side = DotProduct (v, v_right);
		client->v_dmg_roll = kick*side*0.3;
		
		side = -DotProduct (v, v_forward);
		client->v_dmg_pitch = kick*side*0.3;

//		client->v_dmg_time = level.time + DAMAGE_TIME;
		client->v_dmg_time = 0;
	}

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	P_CalcViewOffset
//
//	Auto pitching on slopes?
//
//  fall from 128: 400 = 160000
//  fall from 256: 580 = 336400
//  fall from 384: 720 = 518400
//  fall from 512: 800 = 640000
//  fall from 640: 960 = 
//
//  damage = deltavelocity*deltavelocity  * 0.0001
//
//	adjust view offset to viewheight + other factors such as bob, kickback, etc.
///////////////////////////////////////////////////////////////////////////////

void P_CalcViewOffset (edict_t *ent)
{
	CVector		angles;
	float		bob;
	float		ratio;
	float		delta;
	CVector		v;


//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	if( ent->client->ps.view_entity > game.maxentities )
	{
		ent->client->ps.view_entity = 0;
	}

	if (ent->client->ps.view_entity != 0)
	{
		//	no view movment in camera mode and not following client
		ent->client->ps.viewoffset.x = ent->client->ps.viewoffset.y = 0;
		ent->client->ps.viewoffset.z = g_edicts [ent->client->ps.view_entity].viewheight;
		
		return;
	}
/*
	else 
	if (ent->deadflag)
	{
		// if dead, fix the angle and don't add any kick
//		angles.Zero();

//		ent->client->ps.viewangles.z = 40;
//		ent->client->ps.viewangles.x = -15;
//		ent->client->ps.viewangles.y = ent->client->ps.viewangles.y;
	}
*/
	else
	{
		// add angles based on weapon kick

		angles = ent->client->kick_angles;

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles.x += ratio * ent->client->v_dmg_pitch;
		angles.z += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
		{
			ratio = 0;
		}
		angles.x += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, v_forward);
		angles.x += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, v_right);
		angles.z += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;// * 0.5;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			delta *= 6;		// crouching
		}
		angles.x += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;// * 0.5;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			delta *= 6;		// crouching
		}
		if (bobcycle & 1)
		{
			delta = -delta;
		}
		angles.z += delta;

		//&&& amw - add angles based on angle of surface player is standing on
	}

//===================================

	// base origin

	v.Zero();

	// add view height
	if (ent->client->ps.view_entity == 0)
	{
		//	from client
		v.z += ent->viewheight;
	}
	else
	{
		//	from view entity
		v.z += g_edicts[ent->client->ps.view_entity].viewheight;
	}

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
	{
		ratio = 0;
	}
	v.z -= ratio * ent->client->fall_value * 0.4;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
	{
		bob = 6;
	}
	//gi.DebugGraph (bob *2, 255);
	v.z += bob;

	// add kick offset
	v = v + ent->client->kick_origin;

	// absolutely bound offsets
	// so the view can never be outside the player box

	if (v.x < -14)
	{
		v.x = -14;
	}
	else 
	if (v.x > 14)
	{
		v.x = 14;
	}
	if (v.y < -14)
	{
		v.y = -14;
	}
	else 
	if (v.y > 14)
	{
		v.y = 14;
	}
	if (v.z < -22)
	{
		v.z = -22;
	}
	else 
	if (v.z > 30)
	{
		v.z = 30;
	}

	ent->client->ps.viewoffset = v;
}

///////////////////////////////////////////////////////////////////////////////
//	P_CalcGunOffset
//
//	calc gun placement based on bob
///////////////////////////////////////////////////////////////////////////////

void P_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// gun angles from bobbing
	ent->client->ps.gunangles.z = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles.y = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles.z = -ent->client->ps.gunangles.z;
		ent->client->ps.gunangles.y = -ent->client->ps.gunangles.y;
	}

	ent->client->ps.gunangles.x = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
		{
			ent->client->ps.gunangles.z += 0.1*delta;
		}
		ent->client->ps.gunangles.SetValue( 0.2 * delta, i );
	}

	// gun height
	ent->client->ps.gunoffset.Zero();
//	ent->ps->gunorigin[2] += bob;

	if (gun_x && gun_y && gun_z)
	{
		// gun_x / gun_y / gun_z are development tools
		ent->client->ps.gunoffset.x += v_forward.x * (gun_y->value);
		ent->client->ps.gunoffset.x += v_right.x * gun_x->value;
		ent->client->ps.gunoffset.x += v_up.x * (-gun_z->value);

		ent->client->ps.gunoffset.y += v_forward.y * (gun_y->value);
		ent->client->ps.gunoffset.y += v_right.y * gun_x->value;
		ent->client->ps.gunoffset.y += v_up.y * (-gun_z->value);

		ent->client->ps.gunoffset.z += v_forward.z * (gun_y->value);
		ent->client->ps.gunoffset.z += v_right.z * gun_x->value;
		ent->client->ps.gunoffset.z += v_up.z * (-gun_z->value);
	}
}


///////////////////////////////////////////////////////////////////////////////
//	P_AddBlend
///////////////////////////////////////////////////////////////////////////////

void P_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


///////////////////////////////////////////////////////////////////////////////
//	P_CalcBlend
//
///////////////////////////////////////////////////////////////////////////////

void P_CalcBlend (edict_t *ent)
{
	edict_t	*view_ent;
	int		contents, iEntContents;
	CVector	vieworg;
    
	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 0;
	ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	if (ent->client->ps.view_entity > 0)
		view_ent = &g_edicts [ent->client->ps.view_entity];
	else
		view_ent = ent;

	// add for contents
	vieworg = view_ent->s.origin + ent->client->ps.viewoffset;
	contents = gi.pointcontents (vieworg);

    // Logic[5/10/99]: E3 Nitro Hack
    if(vieworg != ent->s.origin)
        iEntContents = gi.pointcontents(ent->s.origin);
    else
        iEntContents = contents;
    if(iEntContents & CONTENTS_NITRO)
    {
        ent->s.renderfx = RF_IN_NITRO;
        ent->s.alpha = 0.1111;
        if(!ent->nextthink)
            ent->nextthink = level.time + 3.0f;
        
        if(contents & CONTENTS_NITRO)
            P_AddBlend (0.3, 0.3, 1.0, 0.6, ent->client->ps.blend);
    } else if(ent->s.renderfx & RF_IN_NITRO) {
        ent->s.renderfx &= ~RF_IN_NITRO;

        if(ent->s.alpha == 0.1111)
            ent->s.alpha = 0.0f;
    }
    //----------------------------------


	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if( contents & CONTENTS_FOG )
	{
		ent->client->ps.rdflags |= RDF_INFOG;
	}
	else
	{
		ent->client->ps.rdflags &= ~RDF_INFOG;
	}

	//	Nelno:	separate blends for SOLID and LAVA
/*
	if (contents & CONTENTS_LAVA)
		P_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		P_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		P_AddBlend (0.0, 0.1, 0.1, 0.3, ent->client->ps.blend);
*/
	//	don't do any of the other blends in camera mode
	if (view_ent == ent || _stricmp("info_camera",view_ent->className)==0)
	{
		// flash alpha supercedes everything else
		if (ent->client->flash_alpha)
		{
			float r,g,b;
			r = ent->client->flash_blend[0];
			g = ent->client->flash_blend[1];
			b = ent->client->flash_blend[2];

			//	variably colored flash
			P_AddBlend (r, g, b, ent->client->flash_alpha, ent->client->ps.blend);

			// drop the bonus value
            if(ent->client->flash_velocity)
            {
                ent->client->flash_alpha -= ent->client->flash_velocity;
            } else {
			    ent->client->flash_alpha -= 0.01;
            }
			if (ent->client->flash_alpha < 0)
				ent->client->flash_alpha = 0;
		}
		else if (ent->client->damage_alpha > 0)
		{
			//	damage flash
			//	cap max damage alpha to 0.75
			if (ent->client->damage_alpha > 0.75)
				ent->client->damage_alpha = 0.75;
//				P_AddBlend (1.0, 0.0, 0.0, 0.75, ent->client->ps.blend);
//			else
				P_AddBlend (1.0, 0.0, 0.0, ent->client->damage_alpha, ent->client->ps.blend);

			// drop the damage value
			//nt->client->damage_alpha -= 0.045;
			ent->client->damage_alpha -= 0.045;
			if (ent->client->damage_alpha < 0)
				ent->client->damage_alpha = 0;
		}
		else if (ent->client->bonus_alpha > 0)
		{
			//	flash on item  pickups
			P_AddBlend (0.4, 0.4, 1.0, ent->client->bonus_alpha, ent->client->ps.blend);

			// drop the bonus value
			ent->client->bonus_alpha -= 0.1;
			if (ent->client->bonus_alpha < 0)
				ent->client->bonus_alpha = 0;
		}

	}
}


///////////////////////////////////////////////////////////////////////////////
//	P_FallingDamage
//
//	calc damage for falls
//	Q2KILL
///////////////////////////////////////////////////////////////////////////////

/*
void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	CVector	dir;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	if (!ent->groundEntity)
		return;

	delta = ent->velocity[2] - ent->client->oldvelocity[2];
	delta = delta*delta * 0.0001;

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		ent->s.event = EV_FOOTSTEP;
		return;
	}

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (ent->health > 0)
		{
			if (!strcmp (ent->client->pers.sounddir, "player/female"))
			{
				if (delta >= 55)
					ent->s.event = EV_FEMALE_FALLFAR;
				else
					ent->s.event = EV_FEMALE_FALL;
			}
			else
			{
				if (delta >= 55)
					ent->s.event = EV_MALE_FALLFAR;
				else
					ent->s.event = EV_MALE_FALL;
			}
		}

		//	Q2FIXME:	how important s this field?
		ent->pain_debounce_time = level.time;	// no normal pain sound

		damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		dir.Set(0, 0, 1);

		if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING) )
			T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0);
	}
	else
	{
		ent->s.event = EV_FALLSHORT;
		return;
	}
}
*/


///////////////////////////////////////////////////////////////////////////////
//	G_SetClientEffects
//
//	set various effects such as god mode, quad, pent, etc.
///////////////////////////////////////////////////////////////////////////////
/*
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (ent->health <= 0 || level.intermissiontime)
		return;

	// show cheaters!!!
	if (ent->flags & FL_GODMODE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
}
*/

///////////////////////////////////////////////////////////////////////////////
//	G_SetClientEvent
//
//	set footstep event based on bobcycle
///////////////////////////////////////////////////////////////////////////////

#define	EV_SPLASH	( EV_FOOTSTEP_PUDDLE + 1 )

void G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

	CVector vZero(0,0,0);
	if (ent->client->clFlags & (CLF_BIGSPLASH | CLF_MEDSPLASH | CLF_SMALLSPLASH))
	{
		if ( ent->client->clFlags & CLF_SMALLSPLASH)
		{
			ent->s.event = EV_SPLASH_SMALL;
			ent->client->clFlags &= ~CLF_SMALLSPLASH;
		}
		if ( ent->client->clFlags & CLF_MEDSPLASH)
		{
			ent->s.event = EV_SPLASH_MED;
			ent->client->clFlags &= ~CLF_MEDSPLASH;
		}
		if ( ent->client->clFlags & CLF_BIGSPLASH)
		{
			ent->s.event = EV_SPLASH_BIG;
			ent->client->clFlags &= ~CLF_BIGSPLASH;
		}

		if (ent->watertype & CONTENTS_LAVA)
			gi.StartSound(vZero, ent, CHAN_BODY, clsfx_inLava, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX, 0.0f);
		else if (ent->watertype & CONTENTS_SLIME)
			gi.StartSound(vZero, ent, CHAN_BODY, clsfx_inSlime, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX, 0.0f);
		

		
	}
	//cek[1-10-00]: added check for footsteps here.  this is a server var that has wrong meaning when checked on the client
	else if (ent->watertype & CONTENTS_WATER  &&  ent->velocity.Length() > 125.0f && ent->waterlevel == 3)
	{
		ent->s.event = EV_SWIM;	
	}
	else if ( ent->groundEntity && xyspeed > 190 && ent->groundSurface && ( !deathmatch->value || ( deathmatch->value && dm_footsteps->value ) ) )
	{
		if ( (int)(ent->client->bobtime + bobmove) != bobcycle )
		{
			if (ent->groundSurface->flags & SURF_WOOD)
				ent->s.event = EV_FOOTSTEP_WOOD;
			else if (ent->groundSurface->flags & SURF_METAL)
				ent->s.event = EV_FOOTSTEP_METAL;
			else if (ent->groundSurface->flags & SURF_STONE)
				ent->s.event = EV_FOOTSTEP_STONE;
			else if (ent->groundSurface->flags & SURF_GLASS)
				ent->s.event = EV_FOOTSTEP_GLASS;
			else if (ent->groundSurface->flags & SURF_ICE)
				ent->s.event = EV_FOOTSTEP_ICE;
			else if (ent->groundSurface->flags & SURF_SNOW)
				ent->s.event = EV_FOOTSTEP_SNOW;
			else if (ent->groundSurface->flags & SURF_PUDDLE)
				ent->s.event = EV_FOOTSTEP_PUDDLE;
			else if (ent->groundSurface->flags & SURF_SAND)
				ent->s.event = EV_FOOTSTEP_SAND;
			else
				ent->s.event = EV_FOOTSTEP;
		}
	}
	else if( ( ent->velocity.z ) )
	{
		int time = serverState.time * 1000;
		if( time % 12 == 0 )
		{
			CVector flatforward, spot;
			int		nOnLadder = 0;
			trace_t	trace;
			ent->s.angles.AngleToForwardVector( flatforward );

			VectorMA( ent->s.origin, flatforward, 1, spot );
			trace = gi.TraceBox( ent->s.origin, ent->s.mins, ent->s.maxs, spot, ent, MASK_PLAYERSOLID );
			if( ( trace.fraction < 1 ) && ( trace.contents & CONTENTS_LADDER ) )
			{
				if( trace.surface->flags & SURF_WOOD )
				{
					ent->s.event = EV_FOOTSTEP_LADDER_WOOD;
				}
				else if( trace.surface->flags & SURF_METAL )
				{
					ent->s.event = EV_FOOTSTEP_LADDER_METAL;
				}
			}
			else
			{
				VectorMA( ent->s.origin, flatforward, -1, spot );
				trace = gi.TraceBox( ent->s.origin, ent->s.mins, ent->s.maxs, spot, ent, MASK_PLAYERSOLID );
				if( ( trace.fraction < 1 ) && ( trace.contents & CONTENTS_LADDER ) )
				{
					if( trace.surface->flags & SURF_WOOD )
					{
						ent->s.event = EV_FOOTSTEP_LADDER_WOOD;
					}
					else if( trace.surface->flags & SURF_METAL )
					{
						ent->s.event = EV_FOOTSTEP_LADDER_METAL;
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	G_SetClientSound
//
//	set sounds based on various crap
///////////////////////////////////////////////////////////////////////////////
/*
void G_SetClientSound (edict_t *ent)
{
	char	*weap;

	// help beep
	if (game.helpchanged && !(level.framenum&63) )
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.sound = snd_fry;
	else if (strcmp(weap, "weapon_railgun") == 0)
		ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
	else if (strcmp(weap, "weapon_bfg") == 0)
		ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	G_SetClientFrame
//
//	play correct frame sequence based on action
///////////////////////////////////////////////////////////////////////////////
/*
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	// check for stand/duck and stop/go transitions
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;

	if (ent->s.frame < client->anim_end)
	{	// continue an animation
		ent->s.frame++;
		return;
	}

	if (client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (client->anim_priority == ANIM_JUMP)
	{
		if (!ent->groundentity)
			return;		// stay there
		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->client->anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity)
	{
		client->anim_priority = ANIM_JUMP;
		if (ent->s.frame != FRAME_jump2)
			ent->s.frame = FRAME_jump1;
		client->anim_end = FRAME_jump2;
	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else
		{
			ent->s.frame = FRAME_run1;
			client->anim_end = FRAME_run6;
		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}
*/

///////////////////////////////////////////////////////////////////////////////
//	P_ScoreboardMessage
//
///////////////////////////////////////////////////////////////////////////////
int P_SortScores(long* pSorted_scores, int *pSorted)
{
	int				i, j, k;
	long			score;
    userEntity_t    *ent;

	int total = 0;
	//	sort clients by score
	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts [i + 1];

		if (!ent->inuse)
			continue;

		score = ent->record.frags;		// sort score by frags
		//score = ent->record.exp;
		for (j = 0; j < total; j++)
		{
			if (score > *(pSorted_scores + j))
				break;
		}
		for (k = total; k > j; k--)
		{
			*(pSorted + k) = *(pSorted + (k - 1));
			*(pSorted_scores + k) = *(pSorted_scores + (k - 1));
		}
		*(pSorted + j) = i;
		*(pSorted_scores + j) = score;
		total++;
	}

	if (total > 16)
		total = 16;
	
	return total;
}

void P_ScoreboardSingle( userEntity_t *dest_ent )
{
	gi.WriteByte( SCOREBOARD_SINGLE );		// SCG[12/20/99]: Tells client this is a single player update
	gi.WriteByte( serverState.episode );

	gi.unicast( dest_ent, false );
}

void P_ScoreboardCoop( userEntity_t *dest_ent )
{
	int				i;
    userEntity_t    *ent;
	gclient_t		*client;
	long			sorted_scores [MAX_CLIENTS];
	int				sorted [MAX_CLIENTS];
	int				total = 0;
    playerHook_t    *userHook;

	gi.WriteByte( SCOREBOARD_COOP );		// SCG[1/5/00]: Tells client this is a coop update

	total = P_SortScores(&sorted_scores[0],&sorted[0]);
	gi.WriteByte( total );

	gi.WriteByte( serverState.level->intermissiontime ? 1:0);
	gi.WriteByte( serverState.episode );

	for (i = 0; i < total; i++)
	{
		ent = &g_edicts [sorted[i]+1];
        client = ent->client;

		gi.WriteShort (sorted [i]);
		gi.WriteShort (ent->record.frags);
		//gi.WriteLong (sorted_scores [i]);
		gi.WriteShort (ent->record.level);
        
		gi.WriteShort (client->ping < 9999 ? client->ping : 9999);
		gi.WriteShort ((level.framenum - client->resp.enterframe)/600);
        userHook = (playerHook_t *) ent->userHook;
        if(userHook)
        {
		    gi.WriteShort (userHook->fired);
		    gi.WriteShort (userHook->hit);
		    gi.WriteShort (userHook->died);
        } else {
		    gi.WriteShort (0);
            gi.WriteShort (0);
		    gi.WriteShort (0);
        }
	}

	gi.unicast( dest_ent, false );
}

typedef struct teamScore_s
{
	short team;
	short score;
} teamScore_t;

void P_ScoreboardMulti( userEntity_t *dest_ent )
{
	int				i;
    userEntity_t    *ent;
	gclient_t		*client;
	long			sorted_scores [MAX_CLIENTS];
	int				sorted [MAX_CLIENTS];
	short			teamscores[8];
	int				total = 0;
    playerHook_t    *userHook;

	memset(&teamscores,0,sizeof(teamscores));
	gi.WriteByte( SCOREBOARD_MULTI );		// SCG[1/5/00]: Tells client this is a multi player update

	total = P_SortScores(&sorted_scores[0],&sorted[0]);
	gi.WriteByte( total );

	gi.WriteByte( serverState.level->intermissiontime ? 1:0);
	gi.WriteByte( serverState.episode );

	byte teamPlayFlags = (dm_teamplay->value > 0) ? 1 : 0;
	int time = serverState.time;
	if (teamPlayFlags && ( ((time % 10) <= 4) || (serverState.level->intermissiontime) ) )
	{
		teamPlayFlags |= 2;
	}
	gi.WriteByte( teamPlayFlags );

	if ((teamPlayFlags & 3) == 3)
	{
		// set up the team score thingy
		teamScore_t _teamscores[8];
		for (i = 0; i < 8; i++)
		{
			_teamscores[i].score = 0;
			_teamscores[i].team = i+1;
		}

		for (i = 0; i < total; i++)
		{
			ent = &g_edicts [sorted[i]+1];
			int team = ent->team - 1;
			if (teamPlayFlags && (team >= 0) && (team < 8))
			{
				teamscores[team] += ent->record.frags;
				_teamscores[team].score += ent->record.frags;
			}
		}

		short max,k,maxTeam;
		for (int j = 1; j < 9; j++)
		{
			// find the highest team score
			max = -9000;
			for (k = 1; k < 9; k++)
			{
				if (_teamscores[k-1].team == -1)
					continue;

				if (_teamscores[k-1].score > max)
				{
					max = _teamscores[k-1].score;
					maxTeam = k;
				}
			}

			// make sure we can't get that one again...
			_teamscores[maxTeam - 1].team = -1;

			for (i = 0; i < total; i++)
			{
				ent = &g_edicts [sorted[i]+1];
				if (ent->team != maxTeam)
					continue;

				client = ent->client;

				gi.WriteShort (sorted [i]);
				gi.WriteShort (ent->record.frags);
				//gi.WriteLong (sorted_scores [i]);
				gi.WriteShort (ent->record.level);
				gi.WriteShort (ent->team);
        
				gi.WriteShort (client->ping < 9999 ? client->ping : 9999);
				gi.WriteShort ((level.framenum - client->resp.enterframe)/600);
				userHook = (playerHook_t *) ent->userHook;
				if(userHook)
				{
					gi.WriteShort (userHook->fired);
					gi.WriteShort (userHook->hit);
					gi.WriteShort (userHook->died);
				} else {
					gi.WriteShort (0);
					gi.WriteShort (0);
					gi.WriteShort (0);
				}
			}
		}
	}
	else
	{
		for (i = 0; i < total; i++)
		{
			ent = &g_edicts [sorted[i]+1];
			client = ent->client;

			gi.WriteShort (sorted [i]);
			gi.WriteShort (ent->record.frags);
			//gi.WriteLong (sorted_scores [i]);
			gi.WriteShort (ent->record.level);
			gi.WriteShort (ent->team);
        
			gi.WriteShort (client->ping < 9999 ? client->ping : 9999);
			gi.WriteShort ((level.framenum - client->resp.enterframe)/600);
			userHook = (playerHook_t *) ent->userHook;
			if(userHook)
			{
				gi.WriteShort (userHook->fired);
				gi.WriteShort (userHook->hit);
				gi.WriteShort (userHook->died);
			} else {
				gi.WriteShort (0);
				gi.WriteShort (0);
				gi.WriteShort (0);
			}

			int team = ent->team - 1;
			if (teamPlayFlags && (team >= 0) && (team < 8))
			{
				teamscores[team] += ent->record.frags;
			}
		}
	}

	// send over the team scores!
	if (teamPlayFlags)
	{
		for (i = 0; i < 8; i++)
		{
			gi.WriteShort (teamscores[i]);
		}
	}

	gi.unicast (dest_ent, false);
}

void P_ScoreboardCTF(userEntity_t *dest_ent)
{
	int				i;
    userEntity_t    *ent;
	gclient_t		*client;
	long			sorted_scores [MAX_CLIENTS];
	int				sorted [MAX_CLIENTS];
	int				total = 0;
    playerHook_t    *userHook;

	gi.WriteByte( SCOREBOARD_CTF );		// SCG[1/5/00]: Tells client this is a multi player update
	
	// get and sort the scores
	total = P_SortScores(&sorted_scores[0],&sorted[0]);
	gi.WriteByte( total );

	gi.WriteByte( serverState.level->intermissiontime ? 1:0);

	for (i = 0; i < total; i++)
	{
		ent = &g_edicts [sorted[i]+1];
        client = ent->client;

		gi.WriteShort (sorted [i]);
		gi.WriteShort (ent->team);
		gi.WriteShort (ent->record.frags);
		gi.WriteShort (ent->record.level);
		gi.WriteShort (client->ping < 9999 ? client->ping : 9999);
		gi.WriteShort ((level.framenum - client->resp.enterframe)/600);
        userHook = (playerHook_t *) ent->userHook;
        if(userHook)
        {
		    gi.WriteShort (userHook->fired);
		    gi.WriteShort (userHook->hit);
		    gi.WriteShort (userHook->died);
        } 
		else 
		{
		    gi.WriteShort (0);
            gi.WriteShort (0);
		    gi.WriteShort (0);
        }
	}

	teamInfo_t scorearr[ 2 ];
	int scores = dll_FLAG_GetScores( scorearr, 2 );
	gi.WriteShort (scores);
	for(i=0;i<scores;i++)
	{
		gi.WriteShort (scorearr[i].teamNum );
		gi.WriteShort (scorearr[i].captures);
		gi.WriteShort (scorearr[i].score);
	}

	gi.unicast (dest_ent, false);
}

void	P_ScoreboardMessage (userEntity_t *dest_ent)
{
	teamInfo_t scorearr[ 2 ];

	gi.WriteByte (SVC_SCOREBOARD);

	// SCG[12/20/99]: Multiplayer or Singleplayer 
	if( dll_FLAG_GetScores( scorearr, 2 ) )
	{
		P_ScoreboardCTF( dest_ent );
	}
	else if( deathmatch->value == 1 )
	{
		P_ScoreboardMulti( dest_ent );
	}
	else if( coop->value == 1 )
	{
		P_ScoreboardSingle( dest_ent );
	}
	else
	{
		P_ScoreboardSingle( dest_ent );
	}
}
/*
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/

///////////////////////////////////////////////////////////////////////////////
//	Client_EndServerFrame
//
//	Called for each player at the end of the server frame
//	and right after spawning
///////////////////////////////////////////////////////////////////////////////

void Client_EndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
	}

	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;

		dll_SetStats (ent);
		if (!( level.framenum & 31 ) )// && deathmatch->value != 0)
		{
			P_ScoreboardMessage (ent);
		}		
		return;
	}

	AngleToVectors (ent->client->v_angle, v_forward, v_right, v_up);

	// burn from lava, etc
//	P_WorldEffects ();

	//	Q2FIXME:	Change for Hierarchical model stuff
	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->client->v_angle.x > 180)
	{
		ent->s.angles.x = (-360 + ent->client->v_angle.x)/3;
	}
	else
	{
		ent->s.angles.x = ent->client->v_angle.x/3;
	}
	ent->s.angles.y = ent->client->v_angle.y;
	ent->s.angles.z = 0;
	ent->s.angles.z = P_CalcRoll (ent->s.angles, ent->velocity)*4;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	xyspeed = sqrt(ent->velocity.x*ent->velocity.x + ent->velocity.y*ent->velocity.y);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else 
	if (ent->groundEntity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			// Logic[4/30/99]: bobmove = 0.25;
            bobmove = 0.35;
		else if (xyspeed > 190)
			// Logic[4/30/99]: bobmove = 0.25;
            bobmove = 0.225;
		else 
		if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);

	//	scale down because framerate increased
	//bobtime *= 0.5;
	//bobmove *= 0.5;

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime * M_PI));

	// detect hitting the floor
	//	P_FallingDamage (ent);

	// apply all the damage taken this frame, do color blends
	//	P_DamageFeedback (ent);

	// determine the view offsets
	P_CalcViewOffset (ent);

	// determine the gun offsets
	P_CalcGunOffset (ent);

	//	determine the full screen color blend must be after 
	//	viewoffset, so eye contents can be accurately determined
	//	FIXME: with client prediction, the contents should be 
	//	determined by the client
	P_CalcBlend (ent);

	//	P_SetStats (ent);
	dll_SetStats (ent);

	//	Q2FIXME:	footsteps based on bobcycle
	G_SetClientEvent (ent);

	//	set up quad/pent/god effects
	//	G_SetClientEffects (ent);

	//	set up sounds for weapons, lava frying, etc.
	//	G_SetClientSound (ent);

	//	set animation frame based on movement
	//	G_SetClientFrame (ent);

	ent->client->oldvelocity = ent->velocity;
	ent->client->oldviewangles = ent->client->ps.viewangles;

	// clear weapon kicks
	ent->client->kick_origin.Zero();
	ent->client->kick_angles.Zero();

	if (ent->client->showflags & SHOW_SCORES_NOW) // && deathmatch->value != 0)
	{
		//	user has just turned on the scoreboard, so update immediately
		P_ScoreboardMessage (ent);
		ent->client->showflags &= ~SHOW_SCORES_NOW;
	}
	else if( ( ent->client->showflags & SHOW_SCORES ) && !( level.framenum & 31 ) )// && deathmatch->value != 0)
	{
		P_ScoreboardMessage (ent);
	}

  // if inventoryTime is set, and the real time has passed the inventoryTime, switch to items mode
/*
	if( ( ent->client->inventoryTime > 0 ) && ( p_realtime > ent->client->inventoryTime ) )
	{
		P_InventoryUpdateClient (ent, true);	// this will disable ALL inventory/weapons mode
	}
*/
	//	FIXME:	should not run for multiple clients in the same frame...
	//	P_RunSyncFrame (ent);
}

//////////////////////////////////////////////////////////////////////////////////
//	ClientEndServerFrames
//
//	not used, moved to P_RunFrame in p_main.c
//////////////////////////////////////////////////////////////////////////////////

void Client_EndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		Client_EndServerFrame (ent);
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	P_RunSyncFrame
//
//	Runs physics for all entities with the SVF_SYNCTOCLIENT flag
//////////////////////////////////////////////////////////////////////////////////

void P_RunSyncFrame (edict_t *client)
{
	int		i;
	int		curtime, elapsed;
	edict_t	*ent;

	if (!p_sync->value)
		//	for debugging -- don't sync
		return;

	//	calculate the actual time since physics was run on ANY entity
	//	this includes SVF_SYNCTOCLIENT entities
	curtime = gi.Sys_Milliseconds ();
	elapsed = curtime - p_realtime;

	//	calculate p_frametime based on p_frametime multiplier
	p_frametime_scaled = ((float)elapsed / 1000.0) * p_frametime->value;

	//	don't mess with this stuff in here... only in server physics loop
/*
	level.framenum++;
	//	Nelno:	FIXME:	is it right not to scale frametime???
	level.time = level.framenum * FRAMETIME;
	serverState.frametime = level.time - serverState.time; 
	serverState.time = level.time;
*/	

	ent = &g_edicts[0];

	if (!p_runphysics->value)
	{
		//	NO SYNCING FOR CLIENTS!!
/*
		//	special loop for testing with no world physics
		for (i=0 ; i <= maxclients->value; i++, ent++)
		{
			if (!ent->inuse)
				continue;

			level.current_entity = ent;

			///////////////////////////////////////////////////////////////////////////
			//	Run client-only stuff, skip over clients
			///////////////////////////////////////////////////////////////////////////

			//	make sure we're connected!!  inuse only gets set right before ClientBegin is called
			if (!ent->inuse || !ent->client)
				continue;

			Client_BeginServerFrame (ent);

			///////////////////////////////////////////////////////////////////////////
			//	PreThinks!!
			///////////////////////////////////////////////////////////////////////////
			if (ent->prethink)
				ent->prethink (ent);

			P_RunThink (ent);

			///////////////////////////////////////////////////////////////////////////
			//	move children
			///////////////////////////////////////////////////////////////////////////

			if (!(ent->flags & FL_HCHILD))
				//	child hr_Moves get called when parent runs
				hr_Move (ent);

			///////////////////////////////////////////////////////////////////////////
			//	PostThinks!!
			///////////////////////////////////////////////////////////////////////////
			if (ent->postthink)
				ent->postthink (ent);
		}
*/
	}
	else
	{
		for (i=0 ; i<globals.num_edicts ; i++, ent++)
		{
			//	NO SYNCING FOR CLIENTS!!
			if (!ent->inuse || !(ent->svflags & SVF_SYNCTOCLIENT) || i <= maxclients->value)
				continue;

			level.current_entity = ent;

			//	clear SVF_SYNCTOCLIENT flag
			ent->svflags &= ~SVF_SYNCTOCLIENT;

			if ( !(ent->s.renderfx & (RF_BEAM | RF_SPOTLIGHT)) )
			{
				ent->s.old_origin = ent->s.origin;
			}

			// if the ground entity moved, make sure we are still on it
			if ((ent->groundEntity) && (ent->groundEntity->linkcount != ent->groundEntity_linkcount))
			{
				ent->groundEntity = NULL;
				if ((ent->movetype != MOVETYPE_FLY || ent->movetype != MOVETYPE_HOVER) && (ent->svflags & SVF_MONSTER) )
				{
					P_CheckGround (ent);
				}
			}

			///////////////////////////////////////////////////////////////////////////
			//	PreThinks!!
			///////////////////////////////////////////////////////////////////////////
			if (ent->prethink)
				ent->prethink (ent);

			//	handle physics for this entity
			//	hierarchical children shouldn't have physics done on them, they just follow the parent
			if (ent->movetype == MOVETYPE_NONE)
			{
				//	Nelno:  entity should be able to think here since this is
				//			in-between physics frames
/*
				if (ent->think && i > maxclients->value && !(ent->flags & FL_HCHILD))
					P_RunThink (ent);
*/
			}
			else
			{
				if (i > maxclients->value && !(ent->flags & FL_HCHILD))
					P_RunEntity (ent, false);
				else if (ent->think)
					P_RunThink (ent, true);
			}

			///////////////////////////////////////////////////////////////////////////
			//	PostThinks!!
			///////////////////////////////////////////////////////////////////////////
			if (ent->postthink)
				ent->postthink (ent);

			gi.SV_EmitSyncEntity (client, ent);
		}
	}
	
	//	NO SYNCING FOR CLIENTS!!
	//	Client_EndServerFrames ();
}

///////////////////////////////////////////////////////////////////////////////
//	P_GetSoundIndices
//
///////////////////////////////////////////////////////////////////////////////

void	P_GetSoundIndices ()
{
	// missing sounds
//	clsfx_inLava = gi.soundindex ("player/inlava.wav");
//	clsfx_inSlime = gi.soundindex ("player/slimbrn2.wav");
    clsfx_startRun = gi.soundindex("global/a_speedwhoosh.wav");
}