
#include "client.h"
#include "cmodel.h"

extern	bspModel_t	bspModel;

/*
===================
CL_CheckPredictionError
===================
*/
void CL_CheckPredictionError (void)
{
	int		frame;
	int		delta[3];
	int		len;

	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
		return;

	// calculate the last usercmd_t we sent that the server has processed
	frame = cls.netchan.incoming_acknowledged;
	frame &= (CMD_BACKUP-1);

	// compare what the server returned with what we had predicted it to be
	delta[0] = cl.frame.playerstate.pmove.origin[0] - cl.predicted_origins[frame][0];
	delta[1] = cl.frame.playerstate.pmove.origin[1] - cl.predicted_origins[frame][1];
	delta[2] = cl.frame.playerstate.pmove.origin[2] - cl.predicted_origins[frame][2];

	// save the prediction error for interpolation
	len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
	if (len > 640)	// 80 world units
	{	// a teleport or something
		cl.prediction_error.Zero();
	}
	else
	{
		if (cl_showmiss->value && (delta[0] || delta[1] || delta[2]) )
			Com_Printf ("prediction miss on %i: %i\n", cl.frame.serverframe, 
			delta[0] + delta[1] + delta[2]);

		cl.predicted_origins[frame][0] = cl.frame.playerstate.pmove.origin[0];
		cl.predicted_origins[frame][1] = cl.frame.playerstate.pmove.origin[1];
		cl.predicted_origins[frame][2] = cl.frame.playerstate.pmove.origin[2];

		// save for error itnerpolation
		cl.prediction_error.x = delta[0]*0.125;
		cl.prediction_error.y = delta[1]*0.125;
		cl.prediction_error.z = delta[2]*0.125;
	}
}


/*
====================
CL_ClipMoveToEntities

====================
*/
void CL_ClipMoveToEntities ( CVector &start, CVector &mins, CVector &maxs, CVector &end, int passent_num, trace_t *tr )
{
	int			i, x, zd, zu;
	trace_t		trace;
	int			headnode;
	CVector		angles;
	entity_state_t	*ent;
	int			num;
	cmodel_t	*cmodel;
	CVector		bmins, bmaxs;

	for ( i=0 ; i<cl.frame.num_entities ; i++ )
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;

		//	Nelno:	ignore passent instead of just ignoring client
		if (ent->number == passent_num)
			continue;
		if (ent->number == cl.playernum+1)
			continue;

//		if (ent->mins [0] != 0 || ent->mins [1] != 0 || ent->mins [2] != 0 ||
//			ent->maxs [0] != 0 || ent->maxs [1] != 0 || ent->maxs [2] != 0)
		if (ent->iflags & IF_CL_REALBBOX)
		{
			//	use real bounding box
			headnode = CM_HeadnodeForBox (ent->mins, ent->maxs);
			angles = vec3_origin;
		}
		else 
		if (ent->solid == 31)
		{	// special value for bmodel
			cmodel = cl.model_clip[ent->modelindex];
			if (!cmodel)
				continue;
			headnode = cmodel->headnode;
			angles = ent->angles;
		}
		else
		{	// encoded bbox
			x = 8*(ent->solid & 31);
			zd = 8*((ent->solid>>5) & 31);
			zu = 8*((ent->solid>>10) & 63) - 32;

			bmins.x = bmins.y = -x;
			bmaxs.x = bmaxs.y = x;
			bmins.z = -zd;
			bmaxs.z = zu;

			headnode = CM_HeadnodeForBox (bmins, bmaxs);

			angles = vec3_origin;	// boxes don't rotate
		}

		if (tr->allsolid)
			return;

		if( headnode > bspModel.numNodes )
		{
			Com_Printf( "ERROR: headnode > bspModel.numNodes\n" );
			return;
		}

		trace = CM_TransformedBoxTrace (start, end, mins, maxs, headnode,  MASK_PLAYERSOLID, ent->origin, angles);

		if (trace.allsolid || trace.startsolid || trace.fraction < tr->fraction)
		{
			trace.ent = (struct edict_s *)ent;
		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
		{
			tr->startsolid = true;
		}
	}
}


/*
================
CL_PMTraceLine
================
*/
// SCG[1/25/00]: Look comments!!!!
// SCG[1/25/00]: This function is replaced by cl_pmtracebox which does the same thing.
// SCG[1/25/00]: It's just named differently.
// SCG[1/25/00]: Don't ask me why, ask Nelno he did it.
trace_t	CL_PMTraceLine(CVector &start, CVector &end)
{
	trace_t	t;

	//	HACK FOR PUSHING OFF OF FLOATING OBJECTS
	memset (&t, 0x00, sizeof (trace_t));
	return	t;

/*
	// check against world
	t = CM_BoxTrace (start, end, vec3_origin, vec3_origin, 0, MASK_PLAYERSOLID);

	//	HACK:	if trace hit something just set ent pointer to 1
	//			so that pmove code doesn't think we're in the air
	//			Did I mention this is dumb?
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *)1;

	// check all other solid models
	//	Nelno:	send player number for entity to ignore
	CL_ClipMoveToEntities (start, vec3_origin, vec3_origin, end, cl.playernum + 1, &t);
	//	CL_ClipMoveToEntities (start, mins, maxs, end, &t);

	return t;
*/
}

/*
================
CL_PMTraceBox
================
*/
trace_t CL_PMTraceBox(CVector &start, CVector &mins, CVector &maxs, CVector &end)
{
	trace_t	t;

	// check against world
	t = CM_BoxTrace (start, end, mins, maxs, 0, MASK_PLAYERSOLID);

	//	HACK:	if trace hit something just set ent pointer to 1
	//			so that pmove code doesn't think we're in the air
	//			Did I mention this is dumb?
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *)1;

	// check all other solid models
	CL_ClipMoveToEntities (start, mins, maxs, end, cl.playernum + 1, &t);

	return t;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_TraceLine
//
//	same as PMTrace, but uses the passed content mask instead of MASK_PLAYERSOLID
///////////////////////////////////////////////////////////////////////////////
trace_t CL_TraceLine(CVector &start, CVector &end, int passent_num, int clip_mask)
{
	trace_t	trace;

	//	make sure we're connected to the server (ie. the server is set up)
/*
	if (cls.state != ca_active)
	{
		memset (&trace, 0x00, sizeof (trace_t));
		return	trace;
	}
*/
	// check against world
	CVector vZero(0, 0, 0);
	trace = CM_BoxTrace (start, end, vZero, vZero, 0, clip_mask);

	//	HACK:	if trace hit something just set ent pointer to 1
	//			so that pmove code doesn't think we're in the air
	//			Did I mention this is dumb?
	if (trace.fraction < 1.0)
		trace.ent = (struct edict_s *)1;

	// check all other solid models
	//	Nelno:	send number of entity to ignore
	CL_ClipMoveToEntities (start, vZero, vZero, end, passent_num, &trace);
	//	CL_ClipMoveToEntities (start, mins, maxs, end, &t);

	return trace;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_TraceBox
//
//	same as PMTrace, but uses the passed content mask instead of MASK_PLAYERSOLID
///////////////////////////////////////////////////////////////////////////////
trace_t CL_TraceBox(CVector &start, CVector &mins, CVector &maxs, CVector &end, int passent_num, int clip_mask)
{
	trace_t	trace;

	// check against world
	trace = CM_BoxTrace (start, end, mins, maxs, 0, clip_mask);
	if (trace.fraction < 1.0)
		trace.ent = (struct edict_s *)1;

	// check all other solid models
	CL_ClipMoveToEntities (start, mins, maxs, end, passent_num, &trace);

	return trace;
}

int		CL_PMpointcontents (CVector &point)
{
	int			i;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	int			contents;

	contents = CM_PointContents (point, 0);

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (ent->solid != 31) // special value for bmodel
			continue;

		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;

		contents |= CM_TransformedPointContents (point, cmodel->headnode, ent->origin, ent->angles);
	}

	return contents;
}


/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictMovement (void)
{
	int			ack, current;
	int			frame;
	int			oldframe;
	usercmd_t	*cmd;
	pmove_t		pm;
	int			step;
	int			oldz;
    player_state_t  *psCurrent;
    float       fLerp;
    
	if (cls.state != ca_active)
		return;

	if (cl_paused->value)
		return;

	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION) || bspModel.fixModelClip)
	{	// just set angles

		cl.predicted_angles.x = cl.viewangles.x + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[0]);
		cl.predicted_angles.y = cl.viewangles.y + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[1]);
		cl.predicted_angles.z = cl.viewangles.z + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[2]);

		return;
	}

	ack = cls.netchan.incoming_acknowledged;
	current = cls.netchan.outgoing_sequence;

	// if we are too far out of date, just freeze
	if (current - ack >= CMD_BACKUP)
	{
		if (cl_showmiss->value)
			Com_Printf ("exceeded CMD_BACKUP\n");
		return;	
	}

	// copy current state to pmove
	memset (&pm, 0, sizeof(pm));
	pm.TraceLine = CL_PMTraceLine;
	pm.TraceBox = CL_PMTraceBox;
	pm.pointcontents = CL_PMpointcontents;

	pm.s = cl.frame.playerstate.pmove;

//	SCR_DebugGraph (current - ack - 1, 0);

	frame = 0;

	//	calc max speed for this player, MAJOR HACK
	//	FIXME:	move ATTR_ from p_user to dk_shared
#define	ATTR_RUNSPEED	320.0
#define	ATTR_MAX_LEVELS	5
	pm.max_speed = ATTR_RUNSPEED + (ATTR_RUNSPEED / ATTR_MAX_LEVELS) * (float) cl.frame.playerstate.stats[STAT_SPEED];

	// run frames
	while (++ack < current)
	{
		frame = ack & (CMD_BACKUP-1);
		cmd = &cl.cmds[frame];

		pm.cmd = *cmd;

		Pmove (&pm);

		// save for debug checking
		cl.predicted_origins[frame][0] = pm.s.origin[0];
		cl.predicted_origins[frame][1] = pm.s.origin[1];
		cl.predicted_origins[frame][2] = pm.s.origin[2];
	}

	oldframe = (ack-2) & (CMD_BACKUP-1);
	oldz = cl.predicted_origins[oldframe][2];
	step = pm.s.origin[2] - oldz;
	if (step > 63 && step < 160 && (pm.s.pm_flags & PMF_ON_GROUND) )
	{
		cl.predicted_step = step * 0.125;
		cl.predicted_step_time = cls.realtime - cls.frametime * 500;
	}


	// copy results out for rendering
	cl.predicted_origin.x = pm.s.origin[0]*0.125;
	cl.predicted_origin.y = pm.s.origin[1]*0.125;
	cl.predicted_origin.z = pm.s.origin[2]*0.125;

    if ( 
		( cl.frame.playerstate.pmove.pm_type < PM_DEAD ) && 
		( !( cl.frame.playerstate.pmove.pm_flags & ( PMF_CAMERAMODE | PMF_CAMERA_NOLERP ) ) ) &&
		( !( cl.frame.playerstate.rdflags & RDF_NOLERP ) )
		)
    {	// use predicted values
        fLerp = 1.0 - (cl.frame.servertime - cl.time) * 0.01;
        psCurrent = &cl.frame.playerstate;
		cl.predicted_angles.x = LerpAngle(psCurrent->viewangles.x, pm.viewangles.x, fLerp);
		cl.predicted_angles.y = LerpAngle(psCurrent->viewangles.y, pm.viewangles.y, fLerp);
		cl.predicted_angles.z = LerpAngle(psCurrent->viewangles.z, pm.viewangles.z, fLerp);
    } else {
        cl.predicted_angles = pm.viewangles;
    }
}
