#ifdef _DEBUG
#include	"world.h"
#include	"client.h"

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static	userEntity_t	*last_ass = NULL;

static	CVector			set_origin;
static	CVector			set_angles;
static	CVector			set_viewangles;

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	drop_change_owner (userEntity_t *self)
{
	self->owner = self;
	self->nextthink = -1;
}

///////////////////////////////////////////////////////////////////////////////
//	drop_
//
//	for taking screenshots with content that hasn't been completely integrated
///////////////////////////////////////////////////////////////////////////////

void	drop_monster (userEntity_t *self)
{
	int				argc;
	char			name [128];
	userEntity_t	*ass;

	argc = gstate->GetArgc();

	//	MODELFIX:	parse models correctly for new dir structure

	if (argc < 2)
		strcpy (name, "models/e1/m_MWSkinny.dkm");
	else if (argc == 2)
//		sprintf (name, "models/%s.dkm", gstate->GetArgv (1), gstate->GetArgv (1));
		Com_sprintf (name, sizeof(name),"models/%s.dkm", gstate->GetArgv (1), gstate->GetArgv (1));
	else
	{
		gstate->Con_Printf ("Please enter a model name  i.e. e1/m_MWSkinny\n");
		return;
	}

	ass = gstate->SpawnEntity ();

	ass->owner = self;
	ass->solid = SOLID_BBOX;
	ass->movetype = MOVETYPE_TOSS;
	ass->className = "drop_entity";
	
	ass->s.modelindex = gstate->ModelIndex (name);
	gstate->SetSize (ass, -16, -16, -24, 16, 16, 32);
	gstate->SetOrigin2 (ass, self->s.origin);
	gstate->LinkEntity (ass);

//	ass->nextthink = gstate->time + 2.0;
//	ass->think = drop_change_owner;

	ass->s.frame = 0;

	last_ass = ass;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_frame (userEntity_t *self)
{
	int		frame;
	char	arg_str [128];

	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  usage: drop_frame [frame number]\n");
		return;
	}

//	sprintf (arg_str, "%s", gstate->GetArgv (1));
	Com_sprintf (arg_str, sizeof(arg_str),"%s", gstate->GetArgv (1));

	if (arg_str [0] == '+' || arg_str [0] == '-')
	{
		frame = atoi (arg_str);

		if ((int)last_ass->s.frame + frame < 0)
		{
			gstate->Con_Printf ("Negative frames not allowed\n");
			return;
		}
		last_ass->s.frame += frame;
	}
	else
		last_ass->s.frame = atoi (arg_str);

	last_ass->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_rotate (userEntity_t *self)
{
	int		argc;
	CVector	ang;
	char	arg_str [3][128];

	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	argc = gstate->GetArgc ();

	if (argc != 2 && argc != 4)
	{
		gstate->Con_Printf ("  usage: drop_rotate [yaw angle]\n  Angle should be 0 - 360.");
		gstate->Con_Printf ("               OR\n");
		gstate->Con_Printf ("  usage: drop_rotate [pitch] [yaw] [roll]\n");
		return;
	}

//	sprintf (arg_str [0], "%s", gstate->GetArgv (1));
//	sprintf (arg_str [1], "%s", gstate->GetArgv (2));
//	sprintf (arg_str [2], "%s", gstate->GetArgv (3));
	Com_sprintf (arg_str [0], sizeof(arg_str [0]),"%s", gstate->GetArgv (1));
	Com_sprintf (arg_str [1], sizeof(arg_str [1]),"%s", gstate->GetArgv (2));
	Com_sprintf (arg_str [2], sizeof(arg_str [2]),"%s", gstate->GetArgv (3));

	if (argc == 2)
	{
		ang = last_ass->s.angles;

		if (arg_str [0][0] == '+' || arg_str [0][0] == '-')
			ang.y += atof (arg_str [0]);
		else
			ang.y = atof (gstate->GetArgv (1));
	}
	else
	{
		if (arg_str [0][0] == '+')
			ang.x += atof (arg_str [0]);
		else
			ang.x = atof (arg_str [0]);

		if (arg_str [1][0] == '+')
			ang.y += atof (arg_str [1]);
		else
			ang.y = atof (arg_str [1]);

		if (arg_str [2][0] == '+')
			ang.z += atof (arg_str [2]);
		else
			ang.z = atof (arg_str [2]);
	}

	last_ass->s.angles = ang;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_select (userEntity_t *self)
{
	CVector mins(-8, -8, -8);
	CVector maxs(8, 8, 8);
	CVector	ang, end, start;

	if( self->client )
	{
		ang = self->client->v_angle;
	}
	ang.AngleToVectors(forward, right, up);

	start = self->s.origin + forward * 32.0;
	start.z += 32;
	end = start + forward * 1000.0;

	tr = gstate->TraceBox_q2( start, mins, maxs, end, NULL, MASK_PLAYERSOLID );
	if (!tr.ent)
	{
		gstate->Con_Printf ("Nothing selected\n");
		return;
	}

	if (stricmp (tr.ent->className, "drop_entity"))
	{
		gstate->Con_Printf ("%s is not a drop_entity.\n", tr.ent->className);
		return;
	}

	last_ass = tr.ent;
	last_ass->velocity.z = 270;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_move (userEntity_t *self)
{
	char	arg_str [128];
	CVector	dir, start, end;
	float	dist;
	int		zmove = false;

	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	if (gstate->GetArgc () > 1)
	{
		//	move along a vector towards user
//		sprintf (arg_str, "%s", gstate->GetArgv (1));
		Com_sprintf (arg_str, sizeof(arg_str),"%s", gstate->GetArgv (1));
		if ((arg_str [strlen (arg_str) - 1] == 'z') || (arg_str [strlen (arg_str) - 1] == 'Z'))
		{
			zmove = true;		
			arg_str [strlen (arg_str) - 1] = 0x00;
		}

		dist = atof (arg_str);

		start = last_ass->s.origin;
		if (!zmove)
			start.z = 0;
		end = self->s.origin;
		if (!zmove)
			end.z = 0;

		dir = start - end;
		dir.Normalize();

		if (dist > 0)
			gstate->Con_Printf ("Moving drop_entity away.\n");
		else 
			gstate->Con_Printf ("Moving drop_entity closer.\n");

		start = last_ass->s.origin;
		end = last_ass->s.origin + dir * dist;

		//	look for a step
		tr = gstate->TraceBox_q2 (start, last_ass->s.mins, last_ass->s.maxs,
			end, last_ass, MASK_MONSTERSOLID);

		if (tr.fraction != 1.0)
		{
			start.z += 18;
			end.z += 18;

			tr = gstate->TraceBox_q2 (start, last_ass->s.mins, last_ass->s.maxs,
				end, last_ass, MASK_MONSTERSOLID);
			
			if (tr.fraction != 1.0)
			{
				dist = dist * tr.fraction;
				end = last_ass->s.origin + dir * dist;
			}
		}

		last_ass->groundEntity = NULL;
		last_ass->s.origin = end;
		gstate->LinkEntity (last_ass);
	}
	else
		gstate->SetOrigin2 (last_ass, self->s.origin);
}

///////////////////////////////////////////////////////////////////////////////
//	drop_gravity
//
///////////////////////////////////////////////////////////////////////////////

void	drop_gravity (userEntity_t *self)
{
	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}
	
	if (last_ass->movetype == MOVETYPE_TOSS)
	{
		gstate->Con_Printf ("Gravity OFF.\n");
		last_ass->movetype = MOVETYPE_NONE;
	}
	else
	{
		gstate->Con_Printf ("Gravity ON.\n");
		last_ass->movetype = MOVETYPE_TOSS;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_remove (userEntity_t *self)
{
	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	last_ass->remove (last_ass);
	gstate->Con_Printf ("drop_entity removed.  Select another drop entity.\n");

	last_ass = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	drop_deco
//
//	for taking screenshots with content that hasn't been completely integrated
///////////////////////////////////////////////////////////////////////////////

void	drop_deco (userEntity_t *self)
{
	int				argc;
	char			name [128];
	userEntity_t	*ass;

	//	MODELFIX:	parse correctly for new dir structure

	argc = gstate->GetArgc();

	if (argc < 3)
	{
		strcpy (name, "models/global/cup.dkm");
	}
	else if (argc == 3)
	{
//		sprintf (name, "models/%s.dkm", gstate->GetArgv (1), gstate->GetArgv (2), gstate->GetArgv (2));
		Com_sprintf (name, sizeof(name),"models/%s.dkm", gstate->GetArgv (1), gstate->GetArgv (2), gstate->GetArgv (2));
	}
	else
	{
		gstate->Con_Printf ("Please enter a model name  i.e. e1/d1_dressr2.dkm\n");
		gstate->Con_Printf ("USAGE: drop_deco [episode] [modelname]\n");
		return;
	}

	ass = gstate->SpawnEntity ();

	ass->owner = self;
	ass->solid = SOLID_BBOX;
	ass->movetype = MOVETYPE_TOSS;
	ass->className = "drop_deco";
	
	ass->s.modelindex = gstate->ModelIndex (name);
	gstate->SetSize (ass, -16, -16, -24, 16, 16, 32);
	gstate->SetOrigin2 (ass, self->s.origin);
	gstate->LinkEntity (ass);

	ass->s.frame = 0;

	last_ass = ass;
}

///////////////////////////////////////////////////////////////////////////////
//	drop_scale
//
///////////////////////////////////////////////////////////////////////////////

void	drop_scale (userEntity_t *self)
{
	int		argc;
	float	scale;

	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	argc = gstate->GetArgc ();

	if (argc != 2 && argc != 4)
	{
		gstate->Con_Printf ("  USAGE: drop_scale [scale factor]\n");
		return;
	}

	scale = atof (gstate->GetArgv (1));

	last_ass->s.render_scale.Set( scale, scale, scale );
}

///////////////////////////////////////////////////////////////////////////////
//	drop_set
//
///////////////////////////////////////////////////////////////////////////////

void	drop_set (userEntity_t *self)
{
	set_origin = self->s.origin;
	set_angles = self->s.angles;
	set_viewangles = self->client->ps.viewangles;

	gstate->Con_Printf ("View locked.  Use drop_restore to come return here.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	drop_restore
//
///////////////////////////////////////////////////////////////////////////////

void	drop_restore (userEntity_t *self)
{
	self->s.origin = set_origin;
	gstate->LinkEntity (self);

	self->s.angles = set_angles;

	self->flags |= FL_FIXANGLES;
	gstate->SetClientAngles( self, set_viewangles );

	gstate->Con_Printf ("View restored.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	drop_sprite
//
///////////////////////////////////////////////////////////////////////////////

void	drop_sprite (userEntity_t *self)
{
	int				argc;
	char			name [128];
	userEntity_t	*ass;

	argc = gstate->GetArgc();

	//	MODELFIX:	parse correctly

	if (argc < 2)
		strcpy (name, "models/global/e_flare1.sp2");
	else if (argc == 2)
//		sprintf (name, "models/%s.sp2", gstate->GetArgv (1), gstate->GetArgv (1));
		Com_sprintf (name, sizeof(name),"models/%s.sp2", gstate->GetArgv (1), gstate->GetArgv (1));
	else
	{
		gstate->Con_Printf ("Please enter a sprite name  i.e. global/e_flare1.sp2\n");
		return;
	}

	ass = gstate->SpawnEntity ();

	ass->owner = self;
	ass->solid = SOLID_BBOX;
	ass->movetype = MOVETYPE_NONE;
	ass->className = "drop_sprite";

	ass->s.renderfx |= SPR_ALPHACHANNEL;

	ass->s.modelindex = gstate->ModelIndex (name);
	gstate->SetSize (ass, -16, -16, -24, 16, 16, 32);
	gstate->SetOrigin2 (ass, self->s.origin);
	gstate->LinkEntity (ass);

//	ass->nextthink = gstate->time + 2.0;
//	ass->think = drop_change_owner;

	ass->s.frame = 0;

	last_ass = ass;
}

///////////////////////////////////////////////////////////////////////////////
//	drop_help
//
///////////////////////////////////////////////////////////////////////////////

void	drop_help (userEntity_t *self)
{
	gstate->Con_Printf ("Drop commands are: ");
	gstate->Con_Printf ("  drop_monster [model name]\n");
	gstate->Con_Printf ("  drop_rotate [degrees] or +[degrees]\n");
	gstate->Con_Printf ("  drop_select -- point at entity to select\n");
	gstate->Con_Printf ("  drop_move +/-[distance][z]\n");
	gstate->Con_Printf ("  drop_deco [episode] [model name]\n");
	gstate->Con_Printf ("  drop_scale [scale factor]\n");
	gstate->Con_Printf ("  drop_gravity -- toggles on and off\n");
	gstate->Con_Printf ("  drop_set -- save current view\n");
	gstate->Con_Printf ("  drop_restore - restore view\n");
	gstate->Con_Printf ("  drop_sprite [sprite name]\n");
	gstate->Con_Printf ("  drop_movez +/-[distance]\n");
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_movez (userEntity_t *self)
{
	float	dist;

	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("USAGE: drop_movez [+/-][distance].\n");
		return;
	}

	dist = atof (gstate->GetArgv (1));

	last_ass->s.origin.z += dist;
	last_ass->groundEntity = NULL;
	last_ass->gravity = MOVETYPE_NONE;

	gstate->LinkEntity (last_ass);
}

///////////////////////////////////////////////////////////////////////////////
//	drop_respawn
///////////////////////////////////////////////////////////////////////////////

//#define	EF_RESPAWN	0x01000000

void	drop_clear_respawn (userEntity_t *self)
{
	self->s.effects &= ~EF_RESPAWN;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	drop_respawn (userEntity_t *self)
{
	if (!last_ass)
	{
		gstate->Con_Printf ("No drop entity selected.\n");
		return;
	}

	last_ass->s.effects |= EF_RESPAWN;
//	last_ass->s.renderfx |= RF_TRANSLUCENT;
}
#endif