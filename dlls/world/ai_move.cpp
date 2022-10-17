
//	FIXME:	only here for DebugBreak!! Remove later!!
//#include <windows.h>

//////////////////////////////////////////////////////////////////////////////////
//
//									Bot AI 101
//
//////////////////////////////////////////////////////////////////////////////////
// self->goalentity:
//  this is an actual entity that the bot moves around in front of it to follow
//  when the bot goes to an item or node, it is actually going to its own goal
//	entity, which has been moved to the location of the item/node.
//  There is a good reason for making the goalentity its own separate entity.
//  In cases where temporary goals are being set (ie. to get around a wall while 
//  going to a node) there is no real entity which the bot can seek, so his 
//  goalentity is used to store the location he is moving to
//
// self->goalentity->owner:
//	this points to the actualy entity that the bot is seeking.  If the bot is 
//	seeking a node, it will point to that node.  If a bot is seeking an item
//	it will point to that item.  When the bot is seeking a temporary move goal,
//	it will be NULL
//
//////////////////////////////////////////////////////////////////////////////////

//	FIXME:	move some routines into an ai_utils file
//	FIXME:	make hook (self->userHook) a global

#if _MSC_VER
#include	<crtdbg.h>
#endif

#include	"world.h"

#include	"ai.h"			//	stuff global to all ai_ files
#include	"ai_utils.h"
#include	"ai_move.h"
//#include	"ai_frames.h"

#include	"nodelist.h"

//#include	"chasecam.h"
#include	"actorlist.h"
//#include	"GrphPath.h"
//#include	"ai_func.h"

///////////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////////

DllExport	void	ai_think_stub (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////////

ledgeList_t	ledge_list;  // stores status of ledge searches up to 1000 units wide
ledgeData_t	ledge_data;

terInfo_t	terrain;

static	int	total_monsters;
static	int	ai_disable = 0;

///////////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////////

void			ai_attack_stub		(userEntity_t *self);
void			ai_seek_stub		(userEntity_t *self);
int				ai_action_stub		(userEntity_t *self);
void			ai_special_stub		(userEntity_t *self);
void			ai_think_stub		(userEntity_t *self);
userEntity_t	*ai_find_target_stub (userEntity_t *self);


//-----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		ai_spawn_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_spawn_goal( userEntity_t *self )
{
	userEntity_t *temp;
	CVector		 org, end, ang;

	temp = gstate->SpawnFakeEntity ();
	
	//	put the goal a little bit out in front of the monster so that
	//	it won't turn towards 0,0,0 when it does an ambient

	ang = self->s.angles;
	ang.AngleToVectors(forward, right, up);

	org = self->s.origin;
	end = org + forward * 64.0;
	gstate->TraceLine( org, end, TRUE, self, &trace );

	temp->s.origin = trace.endpos;	

	return temp;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_check_gap
// Description:
//	determines if there is an unwalkable gap directly in front of the bot
//	checking begins at dist units in front of the bot
//
//	Pre:	 assumes that forward, up and right are set
//
//	Input:   float dist - distance from self to begin checking for a gap
//	Output:  float - TRUE if a gap was found, FALSE if not
//	Changes: none
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_check_gap( userEntity_t *self, terInfo_t ter, float dist )
{
	CVector			org, end;
	CVector			mins, maxs;
	userEntity_t	*client;
	int				clip;

	mins.Set( -8, -8, -8 );
	maxs.Set( 8, 8, 8 );

	// if we made if this far then there is no wall in front
	// so we can check for a gap now

	// start at feet -- doesn't handle gravity other than down!
	org = self->s.origin + forward * dist;
	if (gstate->gravity_dir.z == 1.0)
	{
		org.z += self->s.maxs.z;
	}
	else
	{
		org.z += self->s.mins.z;
	}

	end = org;
	if (gstate->gravity_dir.z == 1.0)
	{
		end.z += 34.0;
	}
	else
	{
		end.z += -34.0;
	}

/*
	//	Q2KILL: Quake1 tracing
//	gstate->TraceBox (org.vec (), end.vec (), self->mins, self->maxs, TRUE, self, &trace);


	if (self->flags & FL_DEBUG)
	{
		client = gstate->FirstEntity ();
		client = gstate->NextEntity (client);
	}

	if (trace.fraction != 1.0 || trace.startsolid || trace.allsolid)
		return	FALSE;
	else
		return	TRUE;
*/

// SCG[12/10/99]: 	clip = CHOOSE_CLIP(self);
	clip = self->clipmask;
	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );

#ifdef _DEBUG
#ifdef	DEBUGINFO
	if (self->flags & FL_DEBUG)
		com->SpawnMarker (self, trace.endpos, debug_models[DEBUG_MARKER], 0.1);
#endif
#endif

	if ( self->flags & FL_DEBUG )
	{
		client = gstate->FirstEntity();
		client = gstate->NextEntity(client);
	}

	if (tr.fraction != 1.0 || tr.startsolid || tr.allsolid)
	{
		return	FALSE;
	}

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_check_water
// Description:
//	determines if there is an unwalkable water body directly in front
//	checking begins at dist units in front of the bot
//
//	Pre:	assumes that forward, up and right are valid
//
//	Input:   float dist - distance from self to begin checking for water
//	Output:  float - TRUE if water was found, FALSE if not
//	Changes: none
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_check_water( userEntity_t *self, terInfo_t ter, float dist )
{
	float	trace_dist, fwd_inc;
	CVector	org, mod_org, mod_end, temp;
	int		pc, i, water_count;

	// trace lines horizontally in front to make sure there is
	// no obstructing wall

	org = self->s.origin;

	mod_org = org + forward * dist - CVector(0.0, 0.0, self->s.mins[2] - 1);

	gstate->TraceLine( org, mod_org, TRUE, self, &trace );
	if (trace.fraction != 1.0) 
	{
		return FALSE;
	}

	mod_org = org + forward * dist;

	gstate->TraceLine( org, mod_org, TRUE, self, &trace );
	if (trace.fraction != 1.0) 
	{
		return FALSE;
	}

	mod_org = org + forward * dist - CVector(0.0, 0.0, self->s.maxs[2] - 16);

	gstate->TraceLine( org, mod_org, TRUE, self, &trace );
	if (trace.fraction != 1.0) 
	{
		return FALSE;
	}

	// if we made if this far then there is now wall in front
	// so we can check for a gap now
	trace_dist = 1000.0;
	fwd_inc = 8.0;

	// begin dist units in front of bot at wait level
	mod_org = org + forward * dist;

	// end is trace_dist units below org
	mod_end = org;
	mod_end.z = mod_end.z - trace_dist;
	water_count = 0;
	ter.water_type = CONTENTS_EMPTY;

	i = 0;

	while (i < 4)
	{
		// trace line down, add height to down dist, increment out_dist

		// hmmm... this next line is questionable...
		mod_org = org;
		temp = forward * fwd_inc;
		mod_org = mod_org + temp;
		mod_end = mod_end + temp;

		gstate->TraceLine( mod_org, mod_end, TRUE, self, &trace );
		if (trace.inwater)
		{
			water_count = water_count + 1;
			pc = gstate->PointContents (trace.endpos);
			if (pc == CONTENTS_WATER || pc == CONTENTS_LAVA || pc == CONTENTS_SLIME)
			{
				ter.water_type = pc;
			}
		}

		i++;
	}

	if (water_count >=3)
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_terrain_type
// Description:
// Input:
// Output:
//	returns:
//	0 = not blocked (clear or a grade)
//	1 = detected a far obstacle
//	2 = impassable obstruction
//
// Note:
// MIN_WALL_DIST is how close a wall has to be before it considered
// an obstruction
// LOOKAHEAD_DIST is how far ahead the traces are made in front of
// the entity
// Obviously LOOKAHEAD_DIST needs to be greater than MIN_WALL_DIST or
// no walls will be considered blocking
//
// ----------------------------------------------------------------------------

#define	MIN_WALL_DIST	20.0		// walls must be this close to be blocking
#define	LOOKAHEAD_DIST	32.0
#define	TOPLEFT			0
#define	MIDLEFT			1
#define	BOTTOMLEFT		2
#define	TOPRIGHT		3
#define	MIDRIGHT		4
#define	BOTTOMRIGHT		5 
#define	WALL_Z			0.8
//#define	Z_MINUS			24

//#define	SHOWMARKERS		1
//#define	SHOWOUTPUT 		1
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const float fValue );
int	ai_terrain_type( userEntity_t *self, CVector &dir, float dist )
{
	playerHook_t	*hook = AI_GetPlayerHook(self);
	CVector			org, start, end, dir_right;
	float			box_height, box_width;
	float			obs_height = 0, max_height = 0;
	float			tr_dist[6];
	CVector			tr_end[6];
	userEntity_t	*tr_ent[6];
	CVector			tr_normal[6];
	int				i;

	if ( dist < LOOKAHEAD_DIST )
	{
		dist = LOOKAHEAD_DIST;
	}

	
	/////////////////////////////////////////////////////////////////////////
	//	determine if we are blocked at all
	/////////////////////////////////////////////////////////////////////////
	
	end = self->s.origin + dir * dist;

//	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins*1.10, self->s.maxs * 1.10, end, self, MASK_SOLID);//self->clipmask );
	CVector Mins;
	Mins = self->s.mins;
	Mins.z += 4.0f;
	tr = gstate->TraceBox_q2( self->s.origin, Mins, self->s.maxs, end, self, MASK_SOLID);//self->clipmask );
	memset( &terrain, 0x00, sizeof(terrain) );
	terrain.wall_normal = tr.plane.normal;
	if ( tr.fraction == 1.0 && (tr.startsolid != 1 ))
	{
		return FALSE;
	}
   /* else
    {
		// test collision only against the environment
        if ( tr.ent && (_stricmp( tr.ent->className, "worldspawn" ) != 0 || !strstr(tr.ent->className,"door")))
		{
            return FALSE;    
        }
    }*/

	// NSS[2/12/00]:Added in handling of the good old doors that get in our fucking way!
	
	if(tr.ent)
	{
		if(strstr(tr.ent->className,"door"))
		{
			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			if(hook)
			{
				if((hook->dflags & DFL_CANUSEDOOR) && !(tr.ent->targetname) && !(tr.ent->keyname))
				{
					if(tr.ent->use)
					{
						tr.ent->use(tr.ent, self, self);
						AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,0.5f);
					}
				}
				else if(!AI_IsSidekick(hook))
				{
					AI_FindNewCourse(self, &dir);
				}
			}
			return FALSE;
		}
	}
	//	get wall yaw and pitch
	VectorToAngles( terrain.wall_normal, org );
	terrain.wall_yaw = AngleMod(org.y - 90.0);
	terrain.wall_pitch = org.x;
	
	terrain.wall_dist = tr.fraction * dist;

	/////////////////////////////////////////////////////////////////////////
	//	check for passable terrain (grade or low obstruction)
	/////////////////////////////////////////////////////////////////////////

	box_height = (self->s.maxs.z - self->s.mins.z) * 0.5f; //- (24 - Z_MINUS);
	box_width = (self->s.maxs.x - self->s.mins.x) * 0.5f;
	dir.z = 0;
	dir.Normalize();

	//	add to dist because TraceBox actually traces 1/2 the bounding box 
	//	further than a TraceLine
	dist = dist + box_width;

	dir_right.Set(dir.y, -dir.x, 0);

	org = self->s.origin;

	/////////////////////////////////////////////////////////////////////////
	//	fill all tr_normals with a passable Z
	/////////////////////////////////////////////////////////////////////////
	tr_normal[0].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[1].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[2].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[3].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[4].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up
	tr_normal[5].Set(0, 0, 1.0);	// 1.0 == flat, normal straight up

	/////////////////////////////////////////////////////////////////////////
	//	trace lines down left side of bounding box
	/////////////////////////////////////////////////////////////////////////

	//	lower left
	start	= org - dir_right * box_width;
	start.z += self->s.mins.z;			// mins.z is already negative
	end		= start + dir * dist;

	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr_dist[BOTTOMLEFT] = tr.fraction * dist;
	tr_end[BOTTOMLEFT] = tr.endpos;
	tr_ent[BOTTOMLEFT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[BOTTOMLEFT] = tr.plane.normal;
	}

	//	middle left
	start.z	= org.z;
	end.z	= org.z;
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	tr_dist[MIDLEFT] = tr.fraction * dist;
	tr_end[MIDLEFT] = tr.endpos;
	tr_ent[MIDLEFT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[MIDLEFT] = tr.plane.normal;
	}

	//	top left
	start	= org - dir_right * box_width;
	start.z += self->s.maxs.z;
	end		= start + dir * dist;
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	tr_dist[TOPLEFT] = tr.fraction * dist;
	tr_end[TOPLEFT] = tr.endpos;
	tr_ent[TOPLEFT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[TOPLEFT] = tr.plane.normal;
	}

	if ( tr_normal[TOPLEFT].z < WALL_Z || tr_normal[MIDLEFT].z < WALL_Z || tr_normal[BOTTOMLEFT].z < WALL_Z )
//		|| tr_dist [TOPLEFT] < dist || tr_dist [MIDLEFT] < dist || tr_dist [BOTTOMLEFT] < dist)
	{
		terrain.left_blocked = TRUE;
	}

	//	find closest obstruction on left
	terrain.left_dist = tr_dist[BOTTOMLEFT];
	if ( terrain.left_dist > tr_dist[MIDLEFT] )
	{
		terrain.left_dist = tr_dist[MIDLEFT];
	}
	if ( terrain.left_dist > tr_dist[TOPLEFT] )
	{
		terrain.left_dist = tr_dist[TOPLEFT];
	}

	/////////////////////////////////////////////////////////////////////////
	//	trace lines down right side of bounding box
	/////////////////////////////////////////////////////////////////////////

	//	lower RIGHT
	start	= org + dir_right * box_width;
	start.z -= self->s.mins.z;
	end		= start + dir * dist;
	//	Q2KILL
	//	gstate->TraceLine (start.vec (), end.vec (), FALSE, self, &trace);
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	tr_dist[BOTTOMRIGHT] = tr.fraction * dist;
	tr_end[BOTTOMRIGHT] = tr.endpos;
	tr_ent[BOTTOMRIGHT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[BOTTOMRIGHT] = tr.plane.normal;
	}

	//	middle RIGHT
	start.z = org.z;
	end.z	= org.z;
	//	Q2KILL
	//	gstate->TraceLine (start.vec (), end.vec (), FALSE, self, &trace);
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	tr_dist[MIDRIGHT] = tr.fraction * dist;
	tr_end[MIDRIGHT] = tr.endpos;
	tr_ent[MIDRIGHT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[MIDRIGHT] = tr.plane.normal;
	}

	//	top RIGHT
	start	= org + dir_right * box_width;
	start.z += self->s.maxs.z;
	end		= start + dir * dist;
	//	Q2KILL
	//	gstate->TraceLine (start.vec (), end.vec (), FALSE, self, &trace);
	//tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
	tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID);
	tr_dist[TOPRIGHT] = tr.fraction * dist;
	tr_end[TOPRIGHT] = tr.endpos;
	tr_ent[TOPRIGHT] = tr.ent;
	if ( tr.fraction < 1.0 )
	{
		tr_normal[TOPRIGHT] = tr.plane.normal;
	}

	if ( tr_normal [TOPRIGHT].z < WALL_Z || tr_normal [MIDRIGHT].z < WALL_Z || tr_normal [BOTTOMRIGHT].z < WALL_Z )
//		|| tr_dist [TOPRIGHT] < dist || tr_dist [MIDRIGHT] < dist || tr_dist [BOTTOMRIGHT] < dist)
	{
		terrain.right_blocked = TRUE;
	}

	//	find closest obstruction on right
	terrain.right_dist = tr_dist[BOTTOMRIGHT];
	if ( terrain.right_dist > tr_dist[MIDRIGHT] )
	{
		terrain.right_dist = tr_dist[MIDRIGHT];
	}
	if ( terrain.right_dist > tr_dist[TOPRIGHT] )
	{
		terrain.right_dist = tr_dist[TOPRIGHT];
	}

	////////////////////////////////////////////////////////////////////
	//	determine if it's a door that is blocking
	////////////////////////////////////////////////////////////////////
/*
	for (i = 0; i < 6; i++)
	{
		if (tr_ent [i]->movetype == MOVETYPE_PUSH && tr_ent [i]->use)
		{
			tr_ent [i]->use (tr_ent [i]);
			return	TER_CLEAR;
		}
	}
*/
	//	if not blocked or wall is far away, return clear
	if ( !terrain.right_blocked && !terrain.left_blocked && self->movetype != MOVETYPE_FLY )
	{
		terrain.wall_type = TER_CLEAR;
		return TER_CLEAR;
	}
	else 
	if ( terrain.wall_dist > MIN_WALL_DIST )
	{
		terrain.wall_type = TER_FAR_WALL;
		return TER_FAR_WALL;
	}

	//////////////////////////////////////////////////////////////////////////
	//	show markers for debugging
	//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#ifdef	SHOWMARKERS
	if (self->flags & FL_DEBUG)
	{
		for (i = 0; i < 6; i++)
			com->SpawnMarker (self, tr_end[i], debug_models[DEBUG_MARKER], 0.2);
	}
#endif
#endif

	//////////////////////////////////////////////////////////////////////////
	//	check for a low obstruction where one or both of bottom traces are blocked
	//	but no mid or top traces are blocked
	//////////////////////////////////////////////////////////////////////////
	if ((tr_normal[BOTTOMLEFT].z < WALL_Z || tr_normal[BOTTOMRIGHT].z < WALL_Z) &&
		(tr_normal[MIDLEFT].z >= WALL_Z && tr_normal[MIDRIGHT].z >= WALL_Z &&
		 tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z))
	{
		//	since any obstruction < 24 units high is considered a stair, engine
		//	code will automatically step over this, so just condider it TER_CLEAR
		terrain.wall_type = TER_CLEAR;
	}
	//////////////////////////////////////////////////////////////////////////
	//	check for no obstructions
	//////////////////////////////////////////////////////////////////////////
	else 
	if (tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[BOTTOMRIGHT].z >= WALL_Z &&
		tr_normal[MIDLEFT].z >= WALL_Z && tr_normal[MIDRIGHT].z >= WALL_Z &&
		tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z &&
		terrain.right_dist == dist && terrain.left_dist == dist)
	{
		terrain.wall_type = TER_CLEAR;
	}
	//////////////////////////////////////////////////////////////////////////
	//	check for a low obstruction where bottom and mid traces are blocked
	//	and mid trace distance is equal to bottom trace distace (if not, then
	//	it is likely a stair) but no top traces are blocked.  If this sitation 
	//	exists, determine height of obstruction by tracing down from above it 
	//	and then determine if the monster tracing can jump high enough to get 
	//	over it
	//////////////////////////////////////////////////////////////////////////
//	else if ((tr_normal [BOTTOMLEFT].z < WALL_Z || tr_normal [BOTTOMRIGHT].z < WALL_Z) &&
//		(tr_normal [MIDLEFT].z <= WALL_Z || tr_normal [MIDRIGHT].z <= WALL_Z) &&
//		 tr_normal [TOPLEFT].z >= WALL_Z && tr_normal [TOPRIGHT].z >= WALL_Z)
	else 
	if ((((tr_normal[BOTTOMLEFT].z < WALL_Z || tr_normal[MIDLEFT].z < WALL_Z) && tr_dist[MIDLEFT] - tr_dist[BOTTOMLEFT] <= 8.0) ||
		((tr_normal[BOTTOMRIGHT].z < WALL_Z || tr_normal[MIDRIGHT].z < WALL_Z) && tr_dist[MIDRIGHT] - tr_dist[BOTTOMRIGHT] <= 8.0)) &&
		  tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z)
	{
		terrain.wall_type = TER_LOW_OBS;

		if (terrain.left_dist < terrain.right_dist)
		{
			end = (self->s.origin) - (dir_right * box_width) + dir * (terrain.left_dist + 1.0);
		}
		else
		{
			end = (self->s.origin) + (dir_right * box_width) + dir * (terrain.right_dist + 1.0);
		}

		org = end;
		org.z += self->s.maxs.z;

		//	Q2KILL
		//	gstate->TraceLine (org.vec (), end.vec (), FALSE, self, &trace);
		tr = gstate->TraceLine_q2( org, end, self, self->clipmask );
		if (!tr.startsolid && !tr.allsolid)
		{
			obs_height = box_height - tr.fraction * self->s.maxs.z;

			if (self->movetype != MOVETYPE_FLY)
			{
				hook = AI_GetPlayerHook( self );

				max_height = ai_max_height (self, hook->upward_vel);
				// NSS[2/28/00]:
				if(AI_IsSidekick(hook))
				{
					max_height *= 0.50f;					
				}

				if (obs_height < max_height)
				{
					terrain.wall_type = TER_LOW_OBS;
				}
				else
				{
					terrain.wall_type = TER_WALL;
				}
				if(AI_IsSidekick(hook) && obs_height <= 8.0f)
				{
					terrain.wall_type = 0;
				}
			}
			else
			{
				terrain.wall_type = TER_LOW_OBS;
			}
		}
		else
		{
			terrain.wall_type = TER_WALL;
		}
	}
	else 
	if ((((tr_normal[TOPLEFT].z < WALL_Z || tr_normal[MIDLEFT].z < WALL_Z) && tr_dist[TOPLEFT] - tr_dist[MIDLEFT] <= 8.0) ||
		((tr_normal[TOPRIGHT].z < WALL_Z || tr_normal[MIDRIGHT].z < WALL_Z) && tr_dist[TOPRIGHT] - tr_dist[MIDRIGHT] <= 8.0)) &&
		 tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[BOTTOMRIGHT].z >= WALL_Z)
	{
		terrain.wall_type = TER_HIGH_OBS;
	}
	//////////////////////////////////////////////////////////////////////////
	//	check for a grade up, where all normals are >= WALL_Z
	//////////////////////////////////////////////////////////////////////////
	else 
	if (tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[BOTTOMRIGHT].z >= WALL_Z &&
		tr_normal[MIDLEFT].z >= WALL_Z && tr_normal[MIDRIGHT].z >= WALL_Z &&
		tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z &&
		(terrain.right_dist < dist || terrain.left_dist < dist))
	{
		terrain.wall_type = TER_GRADEUP;

		if (terrain.wall_normal.z == 0.0)
		{
			//	find a non-zero z normal
			for (i = 0; i < 6; i++)
			{
				if (tr_normal[i].z != 0)
				{
					terrain.wall_normal = tr_normal[i];
					break;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//	check for steps where some or all normals are 0.0, but where there is
	//	a distance > 1.0 between obstructions
	//////////////////////////////////////////////////////////////////////////
	else 
	if (((((tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[MIDLEFT].z == 0.0) ||
			 (tr_normal[BOTTOMLEFT].z == 0.0 && tr_normal[MIDLEFT].z == 0.0) ||
			 (tr_normal[BOTTOMLEFT].z == 0.0 && tr_normal[MIDLEFT].z >= WALL_Z) ||
			 (tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[MIDLEFT].z >= WALL_Z)) &&
			 tr_dist[MIDLEFT] - tr_dist[BOTTOMLEFT] >= 1.0)  &&
			(((tr_normal[BOTTOMRIGHT].z >= WALL_Z && tr_normal[MIDRIGHT].z == 0.0) ||
			 (tr_normal[BOTTOMRIGHT].z == 0.0 && tr_normal[MIDRIGHT].z == 0.0) ||
			 (tr_normal[BOTTOMRIGHT].z == 0.0 && tr_normal[MIDRIGHT].z >= WALL_Z) ||
			 (tr_normal[BOTTOMRIGHT].z >= WALL_Z && tr_normal[MIDRIGHT].z >= WALL_Z)) &&
			 tr_dist[MIDRIGHT] - tr_dist[BOTTOMRIGHT] >= 1.0))  &&
			 tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z)
	{
		terrain.wall_type = TER_STEPSUP;
	}
	//////////////////////////////////////////////////////////////////////////
	//	check for steps on left
	//////////////////////////////////////////////////////////////////////////
	else 
	if (((((tr_normal[BOTTOMLEFT].z >= WALL_Z && tr_normal[MIDLEFT].z == 0.0) ||
		   (tr_normal[BOTTOMLEFT].z == 0.0 && tr_normal[MIDLEFT].z == 0.0) ||
		   (tr_normal[BOTTOMLEFT].z == 0.0 && tr_normal[MIDLEFT].z >= WALL_Z)) &&
			tr_dist[MIDLEFT] - tr_dist[BOTTOMLEFT] >= 1.0)  &&
			(((tr_normal[BOTTOMRIGHT].z >= WALL_Z && tr_normal[MIDRIGHT].z == 0.0) ||
			 (tr_normal[BOTTOMRIGHT].z == 0.0 && tr_normal[MIDRIGHT].z == 0.0) ||
			 (tr_normal[BOTTOMRIGHT].z == 0.0 && tr_normal[MIDRIGHT].z >= WALL_Z)) &&
			 tr_dist[MIDRIGHT] - tr_dist[BOTTOMRIGHT] >= 1.0))  &&
			 tr_normal[TOPLEFT].z >= WALL_Z && tr_normal[TOPRIGHT].z >= WALL_Z)
	{
		terrain.wall_type = TER_STEPSUP;
	}

	//////////////////////////////////////////////////////////////////////////
	//	check for block on left side only
	//////////////////////////////////////////////////////////////////////////
	else 
	if (terrain.left_blocked && !terrain.right_blocked)
	{
		terrain.wall_type = TER_WALL_LEFT;
	}
	else 
	if (terrain.right_blocked && !terrain.left_blocked)
	{
		terrain.wall_type = TER_WALL_RIGHT;
	}
	else
	{
		terrain.wall_type = TER_WALL;
	}

#ifdef	SHOWOUTPUT
	switch	(terrain.wall_type)
	{
		case	TER_GRADEUP:
			ai_debug_print (self, "TER_GRADEUP\n");
			break;
		case	TER_STEPSUP:
			ai_debug_print (self, "TER_STEPSUP\n");
			break;
		case	TER_FAR_WALL:
			ai_debug_print (self, "TER_FAR_WALL\n");
			break;
		case	TER_WALL:
			ai_debug_print (self, "TER_WALL\n");
			break;
		case	TER_WALL_LEFT:
			ai_debug_print (self, "TER_WALL_LEFT\n");
			break;
		case	TER_WALL_RIGHT:
			ai_debug_print (self, "TER_WALL_RIGHT\n");
			break;
		case	TER_LOW_OBS:
			ai_debug_print (self, "TER_LOW_OBS\n");
			break;
		case	TER_CLEAR:
			ai_debug_print (self, "TER_CLEAR\n");
			break;
	}

	if (terrain.left_blocked)
		ai_debug_print (self, "LB = TRUE, ");
	else
		ai_debug_print (self, "LB = FALSE, ");

	if (terrain.right_blocked)
		ai_debug_print (self, "RB = TRUE\n");
	else
		ai_debug_print (self, "RB = FALSE\n");

	ai_debug_print (self, "LBN = %.1f %.1f %.1f, ", tr_normal [BOTTOMLEFT].x, tr_normal [BOTTOMLEFT].y, tr_normal [BOTTOMLEFT].z); 
	ai_debug_print (self, "RBN = %.1f %.1f %.1f\n", tr_normal [BOTTOMRIGHT].x, tr_normal [BOTTOMRIGHT].y, tr_normal [BOTTOMRIGHT].z);

	ai_debug_print (self, "LMN = %.1f %.1f %.1f, ", tr_normal [MIDLEFT].x, tr_normal [MIDLEFT].y, tr_normal [MIDLEFT].z); 
	ai_debug_print (self, "RMN = %.1f %.1f %.1f\n", tr_normal [MIDRIGHT].x, tr_normal [MIDRIGHT].y, tr_normal [MIDRIGHT].z);

	ai_debug_print (self, "LTN = %.1f %.1f %.1f, ", tr_normal [TOPLEFT].x, tr_normal [TOPLEFT].y, tr_normal [TOPLEFT].z); 
	ai_debug_print (self, "RTN = %.1f %.1f %.1f\n", tr_normal [TOPRIGHT].x, tr_normal [TOPRIGHT].y, tr_normal [TOPRIGHT].z);

	ai_debug_print (self, "obs_height = %.2f, max_height = %.2f\n", obs_height, max_height);
#endif

	return	terrain.wall_type;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_check_water_jump
// Description:
//		called when self->waterlevel == 2 (head out of water) to check
//		if there is a ledge in front of the player.  If so, upward and
//		forward velocity given to self
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_check_water_jump( userEntity_t *self )
{
	CVector	mod_org, mod_end, temp;
	CVector	max_size, min_size;

	temp = self->s.angles;
	temp.AngleToVectors( forward, right, up );

	// mod_org = self->s.origin + '0 0 8';
	mod_org = self->s.origin;

	// add 8 to height for trace start
	mod_org.z += 8.0;

	// set forward z to 0
	forward.SetZ( 0.0 );
	
	mod_end = forward * 24 + mod_org;

	gstate->TraceLine( mod_org, mod_end, TRUE, self, &trace );

	// did we find an obstruction?
	if ( trace.fraction < 1.0 )
	{
		mod_org.z += self->s.maxs.z - 8.0;

		mod_end = mod_org + forward * 24.0;

		// self.movedir = traceplanenormal * -50;
		temp = (trace.planeNormal) * -50;
		temp.Set(self->movedir);

		gstate->TraceLine( mod_org, mod_end, TRUE, self, &trace );
		if (trace.fraction == 1.0)
		{
			self->flags = self->flags | FL_WATERJUMP;
			self->velocity.SetZ( 225.0 );
			return;
		}
	}
};

// ----------------------------------------------------------------------------
//
// Name:		ai_jump_obstructed
// Description:
//	finds the maximum height of a jump and determines
//	if there are any obstructions from current position
//	to jumping position
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_jump_obstructed( userEntity_t *self, float dist, float vz )
{
	CVector	org, end, temp;
	float	dz, g;
	int		clip;

	if (self->gravity != 0.0)
	{
		g = p_gravity->value * self->gravity;
	}
	else
	{
		g = p_gravity->value;
	}

	// find maximum height of jump
	dz = vz * vz / (2 * g);

	org = self->s.origin;
	end = org + forward * dist;

// SCG[12/10/99]: 	clip = CHOOSE_CLIP(self);
	clip = self->clipmask;
	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
	if ( tr.fraction == 1.0 )
	{
		org.z += dz;
		end.z += dz;

		tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
		if ( tr.fraction == 1.0 )
		{
			return	FALSE;
		}
	}
	
	return TRUE;
}



// ----------------------------------------------------------------------------
//
// Name:		ai_find_ledge_list
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

#define	LEDGE_ABOVE	32
#define	LEDGE_BELOW	256
#define	LEDGE_INC	32

int	ai_find_ledge_list( userEntity_t *self, float start_dist )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	CVector			start, end, forward_add;
	CVector			mins, maxs;
	int				i, found_ledge, pc;

	//	debugging vars
	int	ledges = 0;

	mins.Set( -8, -8, -8 );
	maxs.Set( 8, 8, 8 );

	//////////////////////////////////////////////////
	// globals forward, right and up should be set!!!
	//////////////////////////////////////////////////
	
	// start start_dist in front and 256 units above bot's FEET
	start = self->s.origin + forward * start_dist;
	start.z += LEDGE_ABOVE - 24;  // subtract 24 to account for distance from bot's origin to floor
	ledge_list.num_ledges = found_ledge = 0;

	forward_add = forward * LEDGE_INC;
	for ( i = 0; i <= hook->max_jump_dist; i += LEDGE_INC, ledge_list.num_ledges++ )
	{
		// trace a box down to 768 units below bot's feet to find a ledge
		start = start + forward_add;
		end = start;
		end.z += -(LEDGE_BELOW + LEDGE_ABOVE);

// SCG[12/10/99]: 		tr = gstate->TraceLine_q2( start, end, self, CHOOSE_CLIP(self) );
		tr = gstate->TraceLine_q2( start, end, self, self->clipmask );
		
		if ( tr.fraction < 1.0 && !tr.startsolid )
		{
			pc = gstate->PointContents (trace.endpos);
			if ( pc == CONTENTS_EMPTY )
			{
				ledges++;
				ledge_list.ledge_height[ledge_list.num_ledges] = (int) floor(tr.fraction * (LEDGE_ABOVE + LEDGE_BELOW));
				found_ledge = TRUE;
			}
			else 
			{
				ledge_list.ledge_height[ledge_list.num_ledges] = 0;
			}
		}
		else
		{
			ledge_list.ledge_height[ledge_list.num_ledges] = 0;
		}
	}

	ai_debug_print (self, "found %i ledges, i=%i\n", ledges, i);

	return found_ledge;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_choose_ledge
// Description:
//			chooses the best ledge to jump to based on where goal is
//			
//			searches the ledge list set up by ai_find_ledge_list
//			if a ledge's z is within +/- 24 units of the goal's z
//			then that ledge is chosen, otherwise, a ledge with z within +/- 24
//			units of self's z is chosen
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_choose_ledge( userEntity_t *self, userEntity_t *goal, float start_dist )
{
	int		i, chose_second_ledge, found_ledge;
	float	goal_z, self_z, start_z, ledge_z;
	float	goal_dist, ledge_dist;
	float	VectorXYDist, check_dist;
	CVector	org;

	start_z = self->s.origin[2] + LEDGE_ABOVE - 24;  // subtract 24 to account for distance from bot's origin to floor
	goal_z = goal->s.origin[2] - 24;
	self_z = self->s.origin[2] - 24;
	
	// find dist in xy plane
	goal_dist = VectorDistance( self->s.origin, goal->s.origin );
	
	chose_second_ledge = FALSE;
	ledge_dist = 8192.0;
	found_ledge = ledge_list.num_ledges + 1;
	for ( i = 0; i <= ledge_list.num_ledges; i++ )
	{
		ledge_z = start_z - (float)ledge_list.ledge_height[i];

		// is this ledge near to the goal?
		if ( fabs(ledge_z - goal_z) <= 24.0f )
		{
			// record this ledge's xy and z distance
			VectorXYDist = start_dist + i * LEDGE_INC;
			check_dist = fabs (VectorXYDist - goal_dist);

			if ( check_dist < ledge_dist && check_dist > 16 )
			{
				//	this is the best ledge so far based on xy 
				//	and z distances, so record it
				ledge_data.ledge_z_dist = ledge_z - self_z;	
				ledge_data.ledge_xy_dist = VectorXYDist;
				ledge_dist = VectorXYDist;
				
				found_ledge = i;
			}
		}

		if ( fabs(ledge_z - self_z) <= 24.0 && !chose_second_ledge )
		{
			ledge_data.ledge_z_dist = ledge_z - self_z;
			ledge_data.ledge_xy_dist = start_dist + i * LEDGE_INC;
			chose_second_ledge = TRUE;
		}
	}

	if ( self->flags & FL_DEBUG )
	{
		if ( found_ledge <= ledge_list.num_ledges || chose_second_ledge )
		{
			org = (self->s.origin) + forward * ledge_data.ledge_xy_dist;
			org.z += ledge_data.ledge_z_dist;
		}
	}

	if (found_ledge <= ledge_list.num_ledges)
	{
		return	TRUE;
	}
	else 
	if (chose_second_ledge)
	{
		return	TRUE;
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_water_level
// Description:
//		set's self's water level to 0 - 3
//		0 = no water
//		1 = feet in water
//		2 = waist in water
//		3 = under water
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_water_level( userEntity_t *self )
{
	int				pc;
	CVector			temp;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	temp = self->s.origin + self->s.maxs;
	pc = gstate->PointContents( temp );

	//NSS[11/12/99]:Maychapes we start off in WALK mode?  
	//Swimming units CANNOT MAKE IT OUT OF WATER their z velocity is zero'd out.
	//Thus we need to give 'Amphibious' units the ability to walk in shallow water.
	if( hook->dflags & DFL_AMPHIBIOUS )
			self->movetype = MOVETYPE_WALK;
	//DON'T CHANGE THIS UNLESS TALKING WITH NOEL FIRST!
	if (pc & (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME))
	{
		self->waterlevel = 3;
		self->watertype = pc;
		
		if( hook->dflags & DFL_AMPHIBIOUS )
			self->movetype = MOVETYPE_SWIM;

		return;
	}
	else
	{
		temp = self->s.origin;
		pc = gstate->PointContents (temp);

		if ( pc & (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME) )
		{
			self->waterlevel = 2;
			self->watertype = pc;
			if( hook->dflags & DFL_AMPHIBIOUS )
				self->movetype = MOVETYPE_SWIM;
		}
		else
		{
			temp = self->s.origin + self->s.mins;
			pc = gstate->PointContents( temp );

			if ( pc & (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME) )
			{
				self->waterlevel = 1;
				self->watertype = pc;
			}
			else
			{
				self->waterlevel = 0;
				self->watertype = CONTENTS_EMPTY;


			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_water_damage
// Description:
//		damage entity according to type and level of liquid  
// 
//		just a stub for now  
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_water_damage( userEntity_t *self )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	int pc = gstate->PointContents(self->s.origin);
	if (pc == CONTENTS_LAVA || pc == CONTENTS_SLIME)
	{
		com->Damage( self, self, self, zero_vector, zero_vector, 1000.0, DAMAGE_NONE );
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//	wander, follow, path_follow and attack routines
//
///////////////////////////////////////////////////////////////////////////////












////////////////////////////////////////////////////////////////
// monster initialization functions
////////////////////////////////////////////////////////////////



// ----------------------------------------------------------------------------
//
// Name:		ai_teleported
// Description:
//				resets goals/paths upon teleportation
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_teleported( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->last_origin = self->s.origin;
	
}

// ----------------------------------------------------------------------------
//			No longer used code
// ----------------------------------------------------------------------------

#if 0
// ----------------------------------------------------------------------------
//
// Name:		ai_init_monster
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
playerHook_t *ai_init_monster( userEntity_t *self, int type )
{
	playerHook_t		*hook;
	int					i;

	total_monsters++;
	gstate->Con_Dprintf ("total_monsters = %i\n", total_monsters);

	////////////////////////////////
	// set up standard entity values
	////////////////////////////////
	if ( self->epair )
	{
		for ( i = 0; self->epair [i].key != NULL; i++ )
		{
			if ( !stricmp(self->epair [i].key, "target") )
			{
				self->target = self->epair [i].value;
			}
		}
	}

	self->solid		= SOLID_SLIDEBOX;
	self->movetype	= MOVETYPE_WALK;
	self->clipmask  = MASK_MONSTERSOLID;
	
	if ( type == TYPE_BOT )
	{
		self->flags = FL_BOT + FL_PUSHABLE;	
	}
	else
	{
		self->flags = FL_MONSTER + FL_PUSHABLE;
	}

	//	MUST DO |= HERE OR SVF_NEWSPAWN FLAG WILL BE CLEARED!!!
	self->svflags		|= SVF_NOSPLASH + SVF_NOTHUD + SVF_MONSTER;
	self->takedamage	= DAMAGE_YES;
	self->health		= 100;
	self->inventory		= NULL;

	//	turn rates for each axis in degrees per second
	self->ang_speed.SetPitch( 90 );
	self->ang_speed.SetYaw( 360 );
	self->ang_speed.SetRoll( 90 );

//	self->max_speed = 1.0;
	self->input_entity		= self;
	
	self->elasticity		= 0.5;	// half the bounce of a normal MOVETYPE_BOUNCE
	self->mass				= 300.0;	// same mass as a player

	if ( !self->className )
	{
		self->className = "monster_default";
	}
	if ( !self->netname )
	{
		self->netname = "default monster";
	}

	self->view_ofs.Set( 0.0, 0.0, 22.0 );

	self->s.modelindex	= 0;
	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);
	gstate->SetOrigin2 (self, self->s.origin);

	////////////////////////////////
	// initialize user hook
	////////////////////////////////
	
//	self->userHook = new playerHook_t; 
	self->userHook = gstate->X_Malloc( sizeof( playerHook_t ), MEM_TAG_HOOK ); 
	hook = (playerHook_t *) self->userHook;

	hook->owner		= self;

	self->s.frameInfo.frameTime			= FRAMETIME_FPS20;
	self->s.frameInfo.frameInc			= 1;
	self->s.frameInfo.next_frameTime	= gstate->time + self->s.frameInfo.frameTime;	

	hook->last_origin = self->s.origin + CVector(128, 128, 128);

	//////////////////////////////////
	// parse epair values
	//////////////////////////////////
	
	ai_parse_epairs (self);

	////////////////////////////////////////////////////////////////
	// map node stuffffff
	////////////////////////////////////////////////////////////////

	// create the "treasure" list for the player/bot that records
	// locations containing valuable items
	hook->skat_list			= NULL;
//!	hook->path				= NULL;
//!	hook->max_path_length	= 50;

	hook->attack_dist		= 250.0;
	hook->attack_finished	= 0;

	hook->upward_vel		= 270.0;
	hook->run_speed			= 125;
	hook->walk_speed		= 25;
	hook->attack_speed		= 125;
	hook->max_jump_dist		= 240;
	hook->jump_chance		= 0;

	// create a list that points to the master nodelist header
	hook->pNodeList = node_create_nodelist(pGroundNodes);
	hook->pPathList = PATHLIST_Create ();

	//	don't place a spawn node or check for new spawn nodes, though
	//	because we must wait until any nodes included in the map are
	//	spawned.  ai_get_spawn_node will do that as the first think
	//	of every bot/monster

	////////////////////////////////////////////////////////////////
	// init goal
	////////////////////////////////////////////////////////////////
/*
	// set up the bot's roam_goal which is his carrot-on-a-string
	self->goalentity			= gstate->SpawnEntity ();
	self->goalentity->solid		= SOLID_NOT;
	self->goalentity->movetype	= MOVETYPE_NONE;
	self->goalentity->flags		= FL_DEBUG;
	gstate->SetModel (self->goalentity, debug_models [DEBUG_GOAL]);
	gstate->SetOrigin2 (self->goalentity, self->s.origin);
*/
	self->goalentity = ai_spawn_goal (self);

	///////////////////////////////////////////////////////////////
	// set up thinks
	///////////////////////////////////////////////////////////////

	hook->begin_jump				= ai_begin_jump;
	hook->begin_stand				= ai_begin_stand;
	hook->begin_follow				= ai_begin_follow;
	hook->begin_path_follow			= ai_begin_path_follow;
	//hook->begin_time_stamp_follow	= ai_begin_time_stamp_follow;
	hook->begin_node_wander			= ai_begin_node_wander;
	hook->begin_wander				= ai_begin_wander;
	hook->begin_attack				= ai_attack_stub;
	hook->begin_seek				= ai_seek_stub;
	hook->wander_sound				= ai_wander_sound;
	hook->sight_sound				= ai_sight_sound;
	hook->begin_turn				= ai_begin_turn;
	hook->begin_path_wander			= ai_begin_path_wander;
	hook->begin_action				= (think_t) ai_action_stub;
	hook->begin_transition			= NULL;//ai_begin_transition;
	hook->begin_node_retreat		= ai_begin_node_retreat;
	hook->find_target				= ai_find_target_client_bot;
	hook->sight						= NULL;
	
	hook->special_think				= ai_special_stub;
	hook->continue_think			= hook->begin_stand;

	// SCG[12/15/99]: Ok, for testing, but not for release
#ifdef _DEBUG
	if (strstr (gstate->mapName, "zoo"))
	{
		ai_disable = true;
		hook->begin_follow				= ai_think_stub;
		hook->begin_path_follow			= ai_think_stub;
		hook->begin_time_stamp_follow	= ai_think_stub;
		hook->begin_node_wander			= ai_think_stub;
		hook->begin_wander				= ai_think_stub;
		hook->begin_time_stamp_follow	= ai_think_stub;
		hook->find_target				= ai_find_target_stub;
	}
#endif _DEBUG

	hook->think_time		= THINK_TIME;

	hook->back_away_dist	= 128;
	hook->active_distance	= 1500;

	self->die				= ai_begin_die;
	self->pain				= ai_begin_pain;
	self->prethink			= ai_prethink;

	///////////////////////////////////////////////////////////////
	//	set dflags
	///////////////////////////////////////////////////////////////

	if ( !ai_disable )
	{
		hook->dflags |= (DFL_CANSTRAFE + DFL_EVIL + DFL_TIMEFOLLOW);
	}
	else
	{
		hook->dflags |= (DFL_CANSTRAFE + DFL_EVIL + DFL_TIMEFOLLOW);
	}

	hook->sound_time	= gstate->time + rnd () * 10.0;

	// save the monster's type
	hook->type			= type;
	hook->pain_chance	= 75;

	// malloc up the space for our frame data
	//hook->frames = new (HOOK_MALLOC) sequenceTable_t;

	//	get unique ID for this entity
	hook->ID = com->GenID ();

	// FIXME: implement to spread out think times
//	hook->nextthink = gstate->time + global_nextthink;

	////////////////////////////////////////////////////////////////
	//	task stack setup
	////////////////////////////////////////////////////////////////

	if ( self->spawnflags & SPAWN_WANDER )
	{
		task_wander (self);
	}
	else
	{
		task_stand (self);
	}

	return	hook;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_get_spawn_node
// Description:
//				inits a monster's node data after all nodes have been spawned
//				
//				then resest a monster's think according to spawn flags
//				
//				also randomly spreads out monster think times
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_get_spawn_node( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	node_place_spawn_node(self, hook->pNodeList);

	if (self->spawnflags & SPAWN_WANDER)
	{
		task_wander (self);
		self->think = hook->begin_wander;
	}
	else if (self->spawnflags & SPAWN_WANDER)
	{
		task_node_wander (self);
		self->think = hook->begin_node_wander;
	}
	else if (self->spawnflags & SPAWN_PATHFOLLOW || self->target)
	{
		task_path_wander (self);
		self->think = hook->begin_path_wander;
	}
	else
		//	don't push a TASK_STAND, ai_init_monster should have done this
		self->think = hook->begin_stand;

	self->nextthink = gstate->time + rnd () * 2.0 + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_next_node
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR	ai_next_node( userEntity_t *self )
{
	MAPNODE_PTR		travel_node;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	travel_node = node_next_travel (self, hook->pNodeList);		
			
	if (!travel_node)
	{
		//	we're at a node with no links.  Ideally this shouldn't happen
		//	since we were following nodes to get here, but the bot could
		//	have fallen off course
		ai_set_think (self, hook->begin_wander, "ai_next_node");

		ai_debug_print (self, "node_next_travel: 0 link node\n");
		return	NULL;
	}
	else
	{
		ai_set_goal (self, travel_node, travel_node->position, GOAL_NODE);
	}

	return	travel_node;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_node_wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_node_wander( userEntity_t *self )
{
	float			dist;
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	MAPNODE_PTR		next_node;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_node_wander", NULL);
#endif

	//	check for enemies
	if (hook->find_target (self))
		return;

	if (hook->wander_sound) 
		hook->wander_sound (self);

	ai_frame_sounds (self);

	dist = ai_goal_dist (self, self->goalentity);
//	dist = VectorDistance (self->s.origin, self->goalentity->s.origin);

	/////////////////////////////////////////////////////////////////////////////
	//	make sure that our goal is not unreachable 
	//	this can happen if the bot falls or gets blown above it
	/////////////////////////////////////////////////////////////////////////////

	if (fabs (self->s.origin [2] - self->goalentity->s.origin [2]) > 96)
	{
		if (!ai_next_node (self))
			return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	temporary goal seeking (to get around obstructions
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_type == GOAL_TEMP)
	{
		ai_debug_print (self, "ai_node_wander: seeking temp goal", NULL);

		ai_seek_temporary_goal (self, dist);

		// only need to check this after movetogoal since that is when ai_jump is called
		if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
			ai_set_think (self, ai_node_wander, "ai_node_wander");

		return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	determine if we've reached our goal node
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_search_time <= gstate->time)
	{
		ai_debug_print (self, "ai_node_wander: node search time expired.\n");
		//	find the next node from this one
		if (!ai_next_node (self))
			return;
	}

	if (hook->goal_type == GOAL_NODE)
	{
		if (dist < REACHED_GOAL_DIST)
		{
			ai_debug_print (self, "ai_node_wander: reached node\n");	
/*			
			//	determine if this node is targetting a switch or other object
			//	that must be used in order to proceed
			if (!ai_check_node_target (self, hook->node_list->cur_node))
			{
				//	find the next node from this one
				if (!ai_next_node (self))
					return;
			}
			else
			{
				gstate->Con_Printf ("node has a target\n");
				return;
			}
*/
			next_node = ai_next_node (self);
			if (!next_node)
				return;

			MAPNODE_PTR pCurrentNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
			if (ai_check_node_target (self, pCurrentNode, next_node))
				return;
		}
	}

	// go to goal entity
	ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);

	// only need to check this after movetogoal since that is when ai_jump is called
	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_node_wander, "ai_node_wander");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_node_wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_node_wander( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	frameData_t *pSequence = NULL;
	if (self->movetype == MOVETYPE_FLY)
	{
		pSequence = FRAMES_GetSequence( self, "flya" );	
		AI_StartSequence(self, pSequence);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "runa" );
		AI_StartSequence(self, pSequence);
	}

	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	ai_node_wander (self);
}


// ----------------------------------------------------------------------------
//
// Name:		ai_node_retreat
// Description:
//	monster runs away by choosing a node who distance in links
//	to his enemy is greater than the current node
//
//	FIXME: not coded yet
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_node_retreat( userEntity_t *self )
{
	float			dist;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	MAPNODE_PTR		next_node;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_node_wander", NULL);
#endif

	//	check for enemies
	if (hook->find_target (self))
		return;

	if (hook->wander_sound) 
		hook->wander_sound (self);

	ai_frame_sounds (self);

	dist = ai_goal_dist (self, self->goalentity);
	//dist = VectorDistance (self->s.origin, self->goalentity->s.origin);

	/////////////////////////////////////////////////////////////////////////////
	//	make sure that our goal is not unreachable 
	//	this can happen if the bot falls or gets blown above it
	/////////////////////////////////////////////////////////////////////////////

	if (fabs (self->s.origin [2] - self->goalentity->s.origin [2]) > 96)
	{
		if (!ai_next_node (self))
			return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	temporary goal seeking (to get around obstructions
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_type == GOAL_TEMP)
	{
		ai_debug_print (self, "ai_node_wander: seeking temp goal", NULL);

		ai_seek_temporary_goal (self, dist);

		// only need to check this after movetogoal since that is when ai_jump is called
		if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
			ai_set_think (self, ai_node_wander, "ai_node_retreat");

		return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	determine if we've reached our goal node
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_search_time <= gstate->time)
	{
		ai_debug_print (self, "ai_node_wander: node search time expired.\n");
		//	find the next node from this one
		if (!ai_next_node (self))
			return;
	}

	if (hook->goal_type == GOAL_NODE)
	{
		if (dist < REACHED_GOAL_DIST)
		{
			ai_debug_print (self, "ai_node_wander: reached node\n");	
/*			
			//	determine if this node is targetting a switch or other object
			//	that must be used in order to proceed
			if (!ai_check_node_target (self, hook->node_list->cur_node))
			{
				//	find the next node from this one
				if (!ai_next_node (self))
					return;
			}
			else
			{
				gstate->Con_Printf ("node has a target\n");
				return;
			}
*/
			next_node = ai_next_node (self);
			if (!next_node)
				return;

			MAPNODE_PTR pCurrentNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
			if (ai_check_node_target (self, pCurrentNode, next_node))
				return;
		}
	}

	// go to goal entity
	ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);

	// only need to check this after movetogoal since that is when ai_jump is called
	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_node_retreat, "ai_node_retreat");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_node_retreat
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_node_retreat( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	frameData_t *pSequence = NULL;
	if (self->movetype == MOVETYPE_FLY)
	{
		pSequence = FRAMES_GetSequence( self, "flya" );
		AI_StartSequence(self, pSequence);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "runa" );
		AI_StartSequence(self, pSequence);
	}
	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	ai_node_retreat (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_should_follow
// Description:
//	determines if a bot should attack, path to, or continue following
//	an enemy based on how far away the enemy is, and whether or not
//	the bot has a ranged weapon and is close enough to attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_should_follow( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ehook = (playerHook_t *) self->enemy->userHook;
	int				should_follow = TRUE;
	int				depth;

	if (hook->dflags & DFL_RANGEDATTACK)
	{
		if (self->enemy->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
		{
			MAPNODE_PTR pNode1 = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
			MAPNODE_PTR pNode2 = NODE_GetNode( hook->pNodeList->pNodeHeader, ehook->pNodeList->nCurrentNodeIndex );

			depth = node_can_path( hook->pNodeList->pNodeHeader, pNode1, pNode2, 10);
		}
/*
		else
		{
			task_data = task_current_data (hook->task_stack);
			depth = node_can_path( hook->pNodeList->pNodeHeader, hook->pNodeList->pCurrentNode, task_data->goal_node, 10);
		}
*/
		if (fabs (self->enemy->s.origin [2] - self->s.origin [2]) > 128 && !depth)
			should_follow = FALSE;
	}

	PATHLIST_KillPath (hook->pPathList);

	if (should_follow)
	{
		ai_debug_print (self, "ai_should_follow: following\n");
		ai_set_think (self, hook->begin_follow, "ai_should_follow 3");
	}
	else 
	if (self->spawnflags & SPAWN_WANDER)
	{
		ai_debug_print (self, "ai_should_follow: wandering\n");
		ai_set_think (self, hook->begin_wander, "ai_should_follow 4");
	}
	else
	{
		ai_debug_print (self, "ai_should_follow: standing\n");
		ai_set_think (self, hook->begin_stand, "ai_should_follow 5");
	}

	return	should_follow;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_path_follow_do_task
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_path_follow_do_task( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	int				task_type;

	task_current (hook->task_stack, &task_data, &task_type);

	switch	(task_type)
	{
		case	TASK_FOLLOW:
			//	at node where owner was, so try to go straight to him
//			ai_set_goal (self, hook->owner, hook->owner->s.origin, GOAL_OWNER);
			ai_set_task_goal (self);
			ai_set_think (self, hook->begin_follow, "ai_path_follow_do_task 1");
			break;
		case	TASK_CHASE:
			//	at node where enemy was, so try to go straight to enemy
//			ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);
			ai_set_task_goal (self);
			ai_set_think (self, hook->begin_follow, "ai_path_follow_do_task 2");
			break;
		case	TASK_RIDE_PLAT:
		case	TASK_SWITCH:
			//	at node closest to object, so go straight to it
//			ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OBJECT);
			ai_set_task_goal (self);
			ai_set_think (self, hook->begin_follow, "ai_path_follow_do_task 3");
			break;
		case	TASK_NODE_WANDER:
			ai_set_think (self, hook->begin_node_wander, "ai_path_follow_do_task 4");
//			hook->begin_node_wander (self);
			break;
		case	TASK_WANDER:
			ai_set_think (self, hook->begin_wander, "ai_path_follow_do_task 5");
//			hook->begin_wander (self);
			break;
		case	TASK_PLAT:
		case	TASK_DOOR:
			//	FIXME: may be able to get here without actually
			//	reaching the door/plat
			gstate->Con_Printf ("waiting for obj_desired_state\n");
//			ai_begin_wait_for_state (self);
			ai_set_think (self, ai_begin_wait_for_state, "ai_path_follow_do_task 6");
			break;
		case	TASK_ATTACK:
			ai_set_think (self, hook->begin_attack, "ai_path_follow_do_task 7");
			break;
		case	TASK_STAND:
//			hook->begin_stand (self);
			ai_set_think (self, hook->begin_stand, "ai_path_follow_do_task 8");
			break;
		default:
			com->Error ("Unimplimented TASK type in ai_path_follow_do_task.");
			break;
	}

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_path_follow_check_goal
// Description:
// Input:
// Output:
//	only returns TRUE if end of path was reached
// Note:
//
// ----------------------------------------------------------------------------
int	ai_path_follow_check_goal( userEntity_t *self, float dist, int task_type, int visible )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	///////////////////////////////////////////////////////////////////////////////
	//	determine if monster has spent too long trying to reach the next node.
	//	if so, add the current node to the monsters path, forcing it to go back
	//	to that node and try again for the next node.
	///////////////////////////////////////////////////////////////////////////////
	if (gstate->time > hook->goal_search_time)
	{
//		gstate->Con_Printf ("Could not reach goal node.  Restarting path_follow from cur_node.\n");
		//	always clear a ATTACKFOLLOW flag when we reach a goal or restart path
		hook->dflags -= (hook->dflags & DFL_FOLLOWATTACK);

		//	add the current node back to the beginning of the path so that 
		//	monster will go back to current node and try again.
		MAPNODE_PTR pCurrentNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
		_ASSERTE( pCurrentNode );

//		hook->path = path_add_node (hook->path, pCurrentNode);
		PATHLIST_AddNodeToPath (hook->pPathList, pCurrentNode);

		ai_set_goal (self, pCurrentNode, pCurrentNode->position, GOAL_NODE);
	}
	else if (dist < REACHED_GOAL_DIST)
	{
		ai_debug_print (self, "ai_path_follow_check_goal: reached node\n");

		//	always clear a ATTACKFOLLOW flag when we reach a goal
		hook->dflags -= (hook->dflags & DFL_FOLLOWATTACK);

//!		hook->path = path_delete_first (hook->path);
		PATHLIST_DeleteFirstInPath (hook->pPathList);
	
		//	is this the last goal in the path?  Then base bot's next
		//	action on the task it is performing
//!		if (!hook->path)
		if (!hook->pPathList->pPath)
		{
			//	always returns true right now
			if (ai_path_follow_do_task (self))
				return	TRUE;
		}
		else
		{
			// set the goalentity's location to the first path node's origin
//!			MAPNODE_PTR pMapNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->path->nNodeIndex );
			MAPNODE_PTR pMapNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pPathList->pPath->nNodeIndex );
			ai_set_goal (self, pMapNode, pMapNode->position, GOAL_NODE);

			if (task_type == TASK_CHASE)
			{
				/////////////////////////////////////////////////////////////
				//	determine if we should fire while coming around a corner
				/////////////////////////////////////////////////////////////
				if (hook->dflags & DFL_RANGEDATTACK && hook->follow_attack && task_current_type (hook->task_stack) == TASK_CHASE)
				{
					gstate->Con_Dprintf ("checking for corner\n");
					//	if we can't currently see our enemy, determine if it's around the corner
					if (!visible)
					{
						gstate->Con_Dprintf ("enemy not visible\n");
						//	we have a next travel node, determine if the enemy can be seen from it
						gstate->TraceLine (self->goalentity->s.origin, self->enemy->s.origin, TRUE, self, &trace);

						if (trace.fraction == 1.0)
						{
							gstate->Con_Dprintf ("enemy visible from next node\n");
							//	enemy can be seen from next node so face enemy and begin firing
							//	NOTE: enemies with explosive weapons shouldn't actually fire until
							//	they have a clear shot

							hook->dflags |= DFL_FOLLOWATTACK;
						}
					}
				}
			}
		}
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_path_follow
// Description:
//	bot is going to a distant goal
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_path_follow( userEntity_t *self )
{
	int				visible, task_type;
	float			dist;
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ehook, *ohook, to_hook;	
	int				nGoalNodeIndex = -1;
	taskData_t		*task_data;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_path_follow", NULL);
#endif

	if (self->enemy)
	{
		ehook = (playerHook_t *) self->enemy->userHook;
	}
	if (hook->owner)
	{
		ohook = (playerHook_t *) hook->owner->userHook;
	}

	if (hook->wander_sound) 
	{
		hook->wander_sound (self);
	}

	ai_frame_sounds (self);

	dist = ai_goal_dist (self, self->goalentity);

	task_current (hook->task_stack, &task_data, &task_type);

	/////////////////////////////////////////////////////////////////////////////
	//	make sure that our goal is not unreachable 
	//	this can happen if the bot falls or gets blown above it
	/////////////////////////////////////////////////////////////////////////////

	if ( fabs(self->s.origin[2] - self->goalentity->s.origin[2]) > 96 )
	{
		//	go to follow, which should repath on next frame
		ai_path_follow_do_task (self);
	}

	/////////////////////////////////////////////////////////////////////////////
	//	temporary goal seeking (to get around obstructions)
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_type == GOAL_TEMP)
	{
		ai_debug_print (self, "ai_path_follow: seeking temp goal", NULL);

		ai_seek_temporary_goal (self, dist);

		// only need to check this after movetogoal since that is when ai_jump is called
		if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		{
			ai_set_think (self, ai_path_follow, "ai_path_follow");
		}

		return;
	}
	//	pick up items
	else 
	if (hook->goal_type == GOAL_ITEM)
	{
		ai_debug_print (self, "ai_path_follow: seeking goalitem", NULL);

		ai_seek_item (self, dist);

		// only need to check this after movetogoal since that is when ai_jump is called
		if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
			ai_set_think (self, ai_path_follow, "ai_path_follow");

		return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	determine if attacking, and if we should attack our enemy
	/////////////////////////////////////////////////////////////////////////////

	if (task_type == TASK_CHASE)
	{
		task_data = task_current_data (hook->task_stack);

		if (task_data->object->flags & (FL_BOT + FL_CLIENT + FL_MONSTER))
		{
			nGoalNodeIndex = ehook->pNodeList->nCurrentNodeIndex;
		}
		else
		{
			nGoalNodeIndex = task_data->nGoalNodeIndex;
		}

		visible = AI_IsVisible (self, self->enemy);

		if (hook->dflags & DFL_RANGEDATTACK)
		{
			ai_debug_print (self, "ai_path_follow: ranged attack\n");
			
			// monster can attack at range, so visiblity and range are the only requirements
			// to break off path following and attack enemy.

			if (visible && VectorDistance (self->s.origin, self->enemy->s.origin) < hook->attack_dist)
			{
				if (hook->attack_finished <= gstate->time && hook->begin_attack)
				{
					ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);

					task_attack (self, self->enemy);

					ai_set_think (self, hook->begin_attack, "ai_path_follow");
				
					return;
				}
			}
		}
		// FIXME: check number of nodes away, and if only a few (2?) go straight for enemy
		// this will hopefully avoid wierdo running around right in front of the enemy...
		// also, maybe check distance to enemy and if it is really short, go straight for it
		else 
		if (hook->pNodeList->nCurrentNodeIndex == nGoalNodeIndex && visible)
		{
			ai_debug_print (self, "ai_path_follow: melee attack\n");

			ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);

			// in the same node as our enemy, so break off into visual following routine
			ai_debug_print (self, "begin_follow 2\n");
			ai_set_think (self, hook->begin_follow, "ai_path_follow");

			return;
		}
	}
	else if (task_type == TASK_FOLLOW)
	{
		task_data = task_current_data (hook->task_stack);

		if (task_data->object->flags & (FL_BOT + FL_CLIENT + FL_MONSTER))
		{
			nGoalNodeIndex = ohook->pNodeList->nCurrentNodeIndex;
		}
		else
		{
			nGoalNodeIndex = task_data->nGoalNodeIndex;
		}

		visible = AI_IsVisible (self, hook->owner);

		if (hook->pNodeList->nCurrentNodeIndex == nGoalNodeIndex && visible)
		{
			ai_debug_print (self, "ai_path_follow: TASK_FOLLOW\n");

			ai_set_goal (self, hook->owner, hook->owner->s.origin, GOAL_OWNER);

			// in the same node as our owner, so break off into visual following routine
			ai_set_think (self, hook->begin_follow, "ai_path_follow");

			return;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	//	determine if we've reached the next node in the path
	/////////////////////////////////////////////////////////////////////////////

	if (ai_path_follow_check_goal (self, dist, task_type, visible))
		//	reached end of path
		return;
/*
	if (hook->dflags & DFL_FOLLOWATTACK && task_type == TASK_CHASE)
		hook->follow_attack (self);
	else
*/
		ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);

	// only need to check this after movetogoal since that is when ai_jump is called
	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_path_follow, "ai_path_follow");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_path_follow
// Description:
//	monster must already have a path and a GOAL_NODE 
//	before using this routine
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_path_follow( userEntity_t *self )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;

	frameData_t *pSequence = NULL;
	if (self->movetype == MOVETYPE_FLY)
	{
		pSequence = FRAMES_GetSequence( self, "flya" );
		AI_StartSequence(self, pSequence);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "runa" );
		AI_StartSequence(self, pSequence);
	}

	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	ai_path_follow(self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_remove_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_remove_goal( userEntity_t *self )
{
	//	now handles removing of fake entities, too.
	self->remove (self);
/*
	if (!(self->flags & FL_FAKEENTITY))
	{
		// not a fake entity
		self->remove (self);
	}
	else
	{
		com->free (self);
	}
*/
}

// ----------------------------------------------------------------------------
//
// Name:		ai_set_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_set_goal( userEntity_t *self, void *ptr, const CVector &new_origin, int goal_type )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

//	if (hook->goal_type != GOAL_ENEMY && task_current_type (hook->task_stack) == TASK_CHASE)
//		DebugBreak ();

	switch ( goal_type )
	{
		case GOAL_TEMP:

			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = NULL;

			hook->tempgoal_search_time = gstate->time + VectorDistance(new_origin, self->s.origin) / hook->walk_speed + 0.5;
			if (hook->goal_type != GOAL_TEMP)
			{
				hook->last_goal_type = hook->goal_type;
			}

			ai_debug_print(self, "set_goal: GOAL_TEMP\n");
			break;

		case GOAL_NONE:
			//	This may or may not be a bad thing...  I haven't decided
			//	if this should ever get here or not
//			DebugBreak ();
			break;

		case GOAL_NODE:
			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = NULL;
			hook->goalnode = (MAPNODE_PTR) ptr;
			
			hook->goal_search_time = gstate->time + VectorDistance(new_origin, self->s.origin) / hook->walk_speed;
			
			ai_debug_print (self, "set_goal: GOAL_NODE\n");
			break;

		case GOAL_ENEMY:
			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = (userEntity_t *) ptr;
			ai_debug_print (self, "set_goal: GOAL_ENEMY\n");
			break;

		case GOAL_OWNER:
			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = (userEntity_t *) ptr;
			ai_debug_print (self, "set_goal: GOAL_OWNER\n");
			break;
			
		case GOAL_PATH:
			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = (userEntity_t *) ptr;
			hook->goalpath = (userEntity_t *) ptr;
			ai_debug_print (self, "set_goal: GOAL_PATH\n");
			break;

		case GOAL_ITEM:
			self->goalentity->s.origin = new_origin;
			self->goalentity->owner = (userEntity_t *) ptr;
			ai_debug_print (self, "set_goal: GOAL_ITEM\n");
			break;

		case GOAL_OBJECT:
			self->goalentity->s.origin = new_origin;
			hook->goalorigin = new_origin;
			self->goalentity->owner = (userEntity_t *) ptr;
			hook->goalobject = (userEntity_t *) ptr;
			ai_debug_print (self, "set_goal: GOAL_OBJECT\n");
			break;

		default:
			com->Error ("ai_set_goal:  %s has an invalid AI goal type of type %i!", self->className, goal_type);
	}

	if (self->flags & FL_DEBUG)
		gstate->SetOrigin2 (self->goalentity, self->goalentity->s.origin);

	hook->goal_type = goal_type;

	ai_face_goal (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_set_task_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_set_task_goal( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	int				task_type;

	task_current (hook->task_stack, &task_data, &task_type);

	switch	(task_type)
	{
		case	TASK_STAND:
		case	TASK_SWITCH:
		case	TASK_WANDER:
		case	TASK_NODE_WANDER:
		case	TASK_PATH_WANDER:
			ai_set_goal (self, task_data->object, task_data->obj_origin, task_data->goal_type);
			break;
		case	TASK_RIDE_PLAT:
		case	TASK_PLAT:
		case	TASK_DOOR:
			//	update origins of these entities?
			ai_set_goal (self, task_data->object, task_data->obj_origin, task_data->goal_type);
			break;
		case	TASK_FOLLOW:
			task_data->obj_origin = hook->owner->s.origin;
			ai_set_goal (self, task_data->object, task_data->obj_origin, task_data->goal_type);
			break;
		case	TASK_ATTACK:
			//	shouldn't ever get here...
			gstate->Con_Printf ("ai_set_goal: TASK_ATTACK!\n");
			task_data->obj_origin = self->enemy->s.origin;
			ai_set_goal (self, task_data->object, task_data->obj_origin, task_data->goal_type);
			break;
		case	TASK_CHASE:
			task_data->obj_origin = self->enemy->s.origin;
			ai_set_goal (self, task_data->object, task_data->obj_origin, task_data->goal_type);
			break;
		default:
			com->Error ("ai_set_task_goal without TASK_FOLLOW or TASK_CHASE.");
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_resore_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_restore_goal( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			save_time;

	if (hook->goal_type == GOAL_TEMP)
	{
		//	restoring from a temporary goal shouldn't reset goal_search_time!!
		save_time = hook->goal_search_time;

		switch	(hook->last_goal_type)
		{
			case GOAL_NODE:
				ai_set_goal (self, hook->goalnode, hook->goalnode->position, GOAL_NODE);
				break;
			case GOAL_ENEMY:
				ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);
				break;
			case GOAL_OWNER:
				ai_set_goal (self, hook->owner, self->enemy->s.origin, GOAL_OWNER);
				break;
			case GOAL_PATH:
				ai_set_goal (self, hook->goalpath, hook->goalpath->s.origin, GOAL_PATH);
				break;
			case GOAL_ITEM:
				ai_set_goal (self, hook->goalitem, hook->goalitem->s.origin, GOAL_ITEM);
				break;
			case GOAL_OBJECT:
				ai_set_goal (self, hook->goalobject, hook->goalorigin, GOAL_OBJECT);
				break;

			default:
				ai_set_goal (self, self, self->s.origin, GOAL_TEMP);
				break;
				//com->Error ("Invalid last_goal_type!");
		}

		hook->goal_search_time = save_time;
	}
	else
		gstate->Con_Dprintf ("Can't restore from a non GOAL_TEMP\n");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_water_best_direction
// Description:
//	tries to determine the best direction to a goal when an
//	obstruction is encountered while swimming
//
//	traces from corners of bounding box in up, down, forward, back, right and left
//	directions to find a clear route
//
//	expects forward to be the direction self is traveling and right to be perpendicular
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_water_best_direction( userEntity_t *self )
{
	CVector			org, end, temp;
	float			max_x, max_y, min_x, min_y, max_z, min_z;
	int				go_left, go_right, go_down, go_up;
	int				up_clear, down_clear, left_clear, right_clear;
	int				up_left, up_right, down_left, down_right;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	int				clip;

	#define BLOCKED		TRUE
	#define NOT_BLOCKED	FALSE
	#define CHECK_DIST	8.0

	max_x = self->s.maxs[0];
	min_x = self->s.mins[0];
	max_y = self->s.maxs[1];
	min_y = self->s.mins[1];
	max_z = self->s.maxs[2];
	min_z = self->s.mins[2];

// SCG[12/10/99]: 	clip = CHOOSE_CLIP(self);
	clip = self->clipmask;

	////////////////////////////////////////////////////////////
	/// trace forward and find where we are being blocked
	////////////////////////////////////////////////////////////

	up_left = down_left = up_right = down_right = NOT_BLOCKED;

	org = self->s.origin + forward * max_y + right * max_x + up * max_z;
	end = org + forward * CHECK_DIST;

	tr = gstate->TraceLine_q2( org, end, self, clip );
	if (tr.fraction != 1.0)
	{
		up_right = BLOCKED;
	}

	org = (self->s.origin) + forward * max_y + right * min_x + up * max_z;
	end = org + forward * CHECK_DIST;

	tr = gstate->TraceLine_q2( org, end, self, clip );
	if (tr.fraction != 1.0)
	{
		up_left = BLOCKED;
	}

	org = (self->s.origin) + forward * max_y + right * max_x + up * min_z;
	end = org + forward * CHECK_DIST;
	
	tr = gstate->TraceLine_q2( org, end, self, clip );
	if (tr.fraction != 1.0)
	{
		down_right = BLOCKED;
	}

	org = self->s.origin + forward * max_y + right * min_x + up * min_z;
	end = org + forward * CHECK_DIST;
	
	tr = gstate->TraceLine_q2( org, end, self, clip );
	if (tr.fraction != 1.0)
	{
		down_left = BLOCKED;
	}

	if ( up_right == NOT_BLOCKED && up_left == NOT_BLOCKED && down_right == NOT_BLOCKED && down_left == NOT_BLOCKED )
	{
		hook->stuck = FALSE;
		return;
	}

	////////////////////////////////////////////////////////////
	/// trace up from 4 corners of bounding box
	////////////////////////////////////////////////////////////

	up_clear = TRUE;
	
	org = self->s.origin + right * max_x + forward * max_y + up * max_z;
	end = org + up * CHECK_DIST;
	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
	if (tr.fraction != 1.0)
	{
		up_clear = FALSE;
	}

	////////////////////////////////////////////////////////////
	/// trace up from 4 corners of bounding box
	////////////////////////////////////////////////////////////

	down_clear = TRUE;
	
	org = self->s.origin + right * max_x + forward * max_y + up * min_z;
	end = org + up * -CHECK_DIST;

	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
	if (tr.fraction != 1.0)
	{
		down_clear = FALSE;
	}

	////////////////////////////////////////////////////////////
	/// trace right from 4 corners of bounding box
	////////////////////////////////////////////////////////////

	right_clear = TRUE;
	
	org = self->s.origin + right * max_x + forward * max_y + up * max_z;
	end = org + right * CHECK_DIST;

	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
	if (tr.fraction != 1.0)
	{
		right_clear = FALSE;
	}

	////////////////////////////////////////////////////////////
	/// trace left from 4 corners of bounding box
	////////////////////////////////////////////////////////////

	left_clear = TRUE;
	
	org = self->s.origin + right * min_x + forward * max_y + up * max_z;
	end = org + right * -CHECK_DIST;

	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, clip );
	if (tr.fraction != 1.0)
	{
		left_clear = FALSE;
	}

	///////////////////////////////////////////////////////////////
	/// choose a direction to go based on which traces were blocked
	/// and which were open
	///////////////////////////////////////////////////////////////

	go_left = go_right = go_up = go_down = FALSE;

	// if blocked only on bottom, then try to move up, then right, then left, then down
	if ((down_right == BLOCKED && up_right == NOT_BLOCKED) || (down_left == BLOCKED && up_left == NOT_BLOCKED))
	{
		if (up_clear)
			go_up = TRUE;
		else 
		if (right_clear)
			go_right = TRUE;
		else 
		if (left_clear)
			go_left = TRUE;
		else 
		if (down_clear)
			go_down = TRUE;
	}
	// if blocked only on top, then try to move down, then right, then left, then up
	else 
	if ((up_right == BLOCKED && down_right == NOT_BLOCKED) || (up_left == BLOCKED && down_left == NOT_BLOCKED))
	{
		if (down_clear)
			go_down = TRUE;
		else 
		if (right_clear)
			go_right = TRUE;
		else 
		if (left_clear)
			go_left = TRUE;
		else 
		if (up_clear)
			go_up = TRUE;
	}
	// if only right is blocked, then try to move left, then up, then down, then left
	else if ((up_right == BLOCKED && up_left == NOT_BLOCKED) || (down_right == BLOCKED && down_left == NOT_BLOCKED))
	{
		if (left_clear)
			go_left = TRUE;
		else if (up_clear)
			go_up = TRUE;
		else if (down_clear)
			go_down = TRUE;
		else if (right_clear)
			go_right = TRUE;
	}
	// if only right is blocked, then try to move left, then up, then down, then left
	else 
	if ((up_right == BLOCKED && up_left == NOT_BLOCKED) || (down_right == BLOCKED && down_left == NOT_BLOCKED))
	{
		if (left_clear)
			go_left = TRUE;
		else 
		if (up_clear)
			go_up = TRUE;
		else 
		if (down_clear)
			go_down = TRUE;
		else 
		if (right_clear)
			go_right = TRUE;
	}
	// multiple blockages, just try right, up, left, down
	else 	
	{
		if (right_clear)
			go_right = TRUE;
		else 
		if (down_clear)
			go_down = TRUE;
		else 
		if (left_clear)
			go_left = TRUE;
		else
			go_up = TRUE;
	}

	////////////////////////////////////////////////
	// set movement goal based on chosen direction
	////////////////////////////////////////////////
	if (go_right)
	{
		ai_debug_print (self, "going right\n");
		temp = self->s.origin + right * CHECK_DIST * 2.0;

		tr = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs, temp, self, clip);	
		
		ai_set_goal(self, NULL, tr.endpos, GOAL_TEMP);
	}
	else 
	if (go_down)
	{
		ai_debug_print (self, "going down\n");
		temp = self->s.origin + up * -CHECK_DIST * 2.0;
		
		tr = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs, temp, self, clip);	

		ai_set_goal(self, NULL, tr.endpos, GOAL_TEMP);
	}
	else 
	if (go_left)
	{
		ai_debug_print (self, "going left\n");
		temp = self->s.origin + right * -CHECK_DIST * 2.0;

		tr = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs, temp, self, clip);	
		ai_set_goal(self, NULL, tr.endpos, GOAL_TEMP);
	}
	else
	{
		ai_debug_print (self, "going up\n");
		temp = self->s.origin + up * CHECK_DIST * 2.0;

		tr = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs, temp, self, clip);
		ai_set_goal(self, NULL, tr.endpos, GOAL_TEMP);
	}

	hook->stuck = TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_trace_up
// Description:
// Input:
// Output:
//		returns FALSE if movement up blocked
// Note:
//
// ----------------------------------------------------------------------------
int	ai_trace_up( userEntity_t *self )
{
	CVector	base, org, end;
	float z_add = 16.0f;

	org = self->s.origin + CVector(self->s.maxs[1], self->s.maxs[0], 0.0);
	end = org;
	end.z += z_add;


// SCG[12/10/99]: 	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, CHOOSE_CLIP(self) );
	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, self->clipmask );
	if (tr.fraction == 1.0)
	{
		return	TRUE;
	}
	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_trace_down
// Description:
// Input:
// Output:
//			returns FALSE if movement down blocked
// Note:
//
// ----------------------------------------------------------------------------
int	ai_trace_down( userEntity_t *self )
{
	CVector	base, org, end;
	float	z_add = -16.0f;

	org = self->s.origin + CVector(self->s.maxs[1], self->s.maxs[0], 0.0);
	end = org;
	end.z += z_add;

// SCG[12/10/99]: 	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, CHOOSE_CLIP(self) );
	tr = gstate->TraceBox_q2( org, self->s.mins, self->s.maxs, end, self, self->clipmask );
	if (tr.fraction == 1.0)
	{
		return	TRUE;
	}
	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_swimtogoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_swimtogoal( userEntity_t * self, float speed, int move_dir )
{
	CVector			dir, temp;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			dist;

	//ai_debug_print (self, "--- ai_swimtogoal ---\n");

	// get forward, up and right vectors
	if ( move_dir == MOVE_BACKWARD )
	{
		forward = -1.0 * forward;
		right = -1.0 * right;
	}

	// try to stay at same height as goal
	dist = self->goalentity->s.origin[2] - self->s.origin[2];
	if ( dist < -24.0f )
	{
		// swim down
		if ( ai_trace_down(self) )
		{
			dir		= forward;
			forward = up * -1.0;
			up		= dir;
			dir		= forward;
		}
		else
		{
			dir = forward;
		}
	}
	else 
	if ( dist > 24.0f )
	{
		// swim up
		if ( ai_trace_up(self) )
		{
			dir		= forward;
			forward = up;
			up		= dir * -1.0;
			dir		= forward;
		}
		else
		{ 
			dir = forward;
		}
	}
	else 
	{
		dir = forward;
	}

	// choose best direction based on obstructions
	ai_water_best_direction(self);

	// clear groundEntity off so we can always move
	self->groundEntity = NULL;

	// face the goal, unless stuck
	ai_face_goal(self);

	dir = dir * (speed / 2);
	dir.Set(self->velocity);

	ai_check_water_jump (self);

	hook->last_origin = self->s.origin;
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
void arrow_remove( userEntity_t *self )
{
	self->remove (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_follow_wall
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static float ai_follow_wall( userEntity_t *self, const CVector &dest, 
							 const CVector &normal, CVector &dir )
{
	CVector	newdir;
	float	dot, yaw_dir;

	dir = self->s.origin - dest;
	dir.Normalize();

	//	rotate right of wall normal
	newdir.Set( normal.y, - normal.x, 0.0 );
	
	// get cos of angle between goal direction and vector along wall
	dot = dir.DotProduct( newdir );

	if ( self->ang_speed[YAW] > 15 )
	{
		yaw_dir = 15;
	}
	else 
	{
		yaw_dir = self->ang_speed[YAW];
	}

	if ( dot > 0 )
	{
		dir.x = -normal.y;
		dir.y = normal.x;

		yaw_dir = -yaw_dir;
	}
	else
	{
		dir.x = normal.y;
		dir.y = -normal.x;
	}

	dir.z = 0;
	dir.Normalize();

	return	yaw_dir;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_choose_turn
// Description:
//	returns the best yaw based on the normal
//	of the wall passed to it
//
//	currently performs a max of 18 tracelines
//	which may be too slow
//
//	dest is the location monster is trying to reach
//	normal is the normal of the blocking plane
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float ai_choose_turn( userEntity_t *self, const CVector &dest, const CVector &normal )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	CVector			dir, newdir, end;
	float			dot, yaw_dir;

	dir = self->s.origin - dest;
	dir.SetZ( 0 );
	dir.Normalize();

	//	right of wall normal
	newdir.Set( normal.y, -normal.x, 0.0 );
	
	// get cos of angle between goal direction and vector along wall
	dot = dir.DotProduct( newdir );

	yaw_dir = self->ang_speed[YAW];

	if ( dot > 0 )
	{
		dir.SetX( -normal.y );
		dir.SetY( normal.x );

		yaw_dir = -yaw_dir;
		hook->turn_dir = TURN_RIGHT;
	}
	else
	{
		dir.SetX( normal.y );
		dir.SetY( -normal.x );
		
		hook->turn_dir = TURN_LEFT;
	}

	end = self->s.origin + dir * (REACHED_GOAL_DIST + 16);
	ai_set_goal(self, NULL, end, GOAL_TEMP);

	return	yaw_dir;
};

// ----------------------------------------------------------------------------
//
// Name:		ai_movetogoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_movetogoal( userEntity_t *self, float speed, int move_dir )
{
	CVector			org, end, temp, dir;
	float			dist, move_dist, yaw, scale, temp_speed;
	int				grade, gap, i;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	// get forward, up and right vectors
	if ( move_dir == MOVE_STRAFE )
	{
		// forward is direction to goal, but entity should not face it
		forward = self->goalentity->s.origin - self->s.origin;
		forward.Normalize ();
		// get right vector
		right.Set( forward.y, - forward.x, 0 );

		// get up vector
		up.CrossProduct( forward, right );
		up.Multiply( -1 );
		up.Normalize();

		//	make entity face self->enemy
		hook->dflags |= DFL_FACEENEMY;
	}
	else
	{
		// forward is true forward
		temp = self->s.angles;
		temp.SetX( 0 );
		temp.SetZ( 0 );
		AngleToVectors( temp, forward, right, up );

		//	make entity face self->goalentity
		hook->dflags -= (hook->dflags & DFL_FACEENEMY);
	}

	org = self->s.origin;
	move_dist = hook->last_origin.Distance(org);

	// if under water, then swim
	if ( self->waterlevel > 1 )
	{
		ai_swimtogoal(self, speed, move_dir);
		return;
	}
	else 
	if (self->flags & FL_WATERJUMP)
	{
		gstate->Con_Dprintf ("water jumping\n");
		org = self->s.origin;
		org.z = org.z + (self->s.mins[2] - 4);
		end = org + forward * 24;
		gstate->TraceLine( org, end, FALSE, self, &trace );
		if ( trace.fraction == 1.0 )
		{
			self->flags = self->flags - FL_WATERJUMP;
			self->movetype = MOVETYPE_WALK;
			end = forward * speed;
			self->velocity = end;
		}
		else
		{
			end = forward * speed;
			self->velocity = end;
			self->velocity.z = 225;
		}

		return;
	}	
	else 
	if (!self->groundEntity)
	{
		if ( self->waterlevel == 0 || (self->waterlevel > 0 && self->velocity[2] <= 0.0) )
		{
			//	just find out current node, so we don't loose track when "falling"
			//	down steps
			//	node_find_cur_node (self, hook->node_list);
			return;
		}
	}
	else	
	{
		self->movetype = MOVETYPE_WALK;
	}

	if ( move_dir == MOVE_BACKWARD )
	{
		forward = -1.0 * forward;
		right = -1.0 * right;
	}

	if ( self->velocity.Length() > 0)
	{
		dist = speed / 5.0;
	}
	else
	{
		dist = speed / 10.0;
	}

	if (dist < 32.0) 
	{
		dist = 32.0;
	}

	grade = ai_terrain_type (self, forward, dist);
	if ( grade < TER_OBSTRUCTED && terrain.wall_type == TER_FAR_WALL && terrain.wall_dist < 20 )
	{
		ai_follow_wall( self, self->goalentity->s.origin, terrain.wall_normal, dir );

		dir = self->s.origin + dir * 32.0;
		ai_set_goal( self, NULL, dir, GOAL_TEMP );

		hook->begin_turn(self);

		hook->last_origin = self->s.origin;

		return;
	}

	gap = ai_check_gap(self, terrain, 64.0);

	if ( (move_dist <= 0.1) && !(hook->dflags & DFL_TURNED) )
	{
		ai_debug_print (self, "Help me, Nelnobi Wan Kenobi!!  You're my only hope!!\n");
	}

	if ( gap || grade >= TER_OBSTRUCTED )
	{
		// stop all motion
		self->velocity.Zero();
		hook->last_origin = self->s.origin;

		if ( gap && hook->begin_jump )
		{
			if ( ai_find_ledge_list(self, dist) )
			{
				// choose ledge
				if (self->goalentity->owner)
				{
					i = ai_choose_ledge(self, self->goalentity->owner, 64);
				}
				else
				{
					i = ai_choose_ledge(self, self->goalentity, 64);
				}

				if (i)
				{
					if ( !ai_jump_obstructed(self, (float) i, hook->upward_vel) )
					{
						hook->forward_vel = ai_jump_vel(self, ledge_data.ledge_xy_dist, ledge_data.ledge_z_dist, hook->upward_vel);
						hook->begin_jump (self);

						return;
					}
				}
			}
		}

		if ( gap && !hook->turn_dir )
		{
			// couldn't find a ledge to jump to, so turn to the normal of the ledge we're on
			org = self->s.origin + forward * 64.0;
			org.z += (self->s.mins.z - 7.0);

			end = self->s.origin + forward * 32.0;
			end.z += (self->s.mins.z - 7.0);

			gstate->TraceLine( org, end, TRUE, self, &trace );
			if ( trace.fraction != 1.0 )
			{
				// trace hit the ledge
				ai_debug_print (self, "gap: turning to ledge normal %.0f, md = %.2f\n", hook->turn_yaw, move_dist);

				hook->turn_yaw = ai_choose_turn( self, self->goalentity->s.origin, trace.planeNormal );
			}
			else
			{
				// no ledge, found so turn 135 degrees
				if ( rnd() < 0.5 )
				{
					if ( self->ang_speed[YAW] > 15 )
					{
						hook->turn_yaw = -15;
					}
					else
					{
						hook->turn_yaw = - self->ang_speed[YAW];
					}
					self->ideal_ang.SetY( AngleMod(self->s.angles[YAW] - 135) );
				}
				else
				{
					if ( self->ang_speed[YAW] > 15 )
					{
						hook->turn_yaw = 15;
					}
					else
					{
						hook->turn_yaw = self->ang_speed[YAW];
					}

					self->ideal_ang.SetY( AngleMod(self->s.angles[YAW] + 135) );
				}

				ai_debug_print( self, "gap: turning 135 degrees\n", hook->turn_yaw, move_dist );
			}
		}
		else 
		if ( grade && terrain.wall_type == TER_LOW_OBS )
		{
			ai_begin_jump_up( self );

			return;
		}
		else 
		if ( move_dir == MOVE_FORWARD && terrain.wall_type == TER_WALL_RIGHT )
		{
			hook->strafe_dir = STRAFE_LEFT;
			hook->strafe_time = gstate->time + hook->think_time;

			yaw = self->s.angles[YAW];
			temp_speed = hook->attack_speed;
			hook->attack_speed = speed;
			ai_strafetogoal(self, MOVE_STRAFE);
			self->s.angles.SetY( yaw );
			hook->attack_speed = temp_speed;
			
			return;
		}
		else 
		if ( move_dir == MOVE_FORWARD && terrain.wall_type == TER_WALL_LEFT )
		{
			hook->strafe_dir = STRAFE_RIGHT;
			hook->strafe_time = gstate->time + hook->think_time;

			yaw = self->s.angles[YAW];
			temp_speed = hook->attack_speed;
			hook->attack_speed = speed;
			ai_strafetogoal(self, MOVE_STRAFE);
			self->s.angles.SetY( yaw );
			hook->attack_speed = temp_speed;

			return;
		}
		else 
		if ( !hook->turn_dir )
		{
			hook->turn_yaw = ai_choose_turn( self, self->goalentity->s.origin, terrain.wall_normal );
			ai_debug_print( self, "wall: turning %.0f, md = %.2f\n", hook->turn_yaw, move_dist );
		}
		else
		{
			////////////////////////////////////////////////////////////
			//	last turn didn't get us unstuck, so turn again in the
			//	same direction
			////////////////////////////////////////////////////////////

			if ( hook->turn_yaw > 0 )
			{
				self->ideal_ang.SetY( self->s.angles[YAW] + 90 );
				ai_debug_print( self, "wall: stuck, turning 90, md = %.2f\n", move_dist );
			}
			else
			{
				self->ideal_ang.SetY( self->s.angles[YAW] - 90 );
				ai_debug_print( self, "wall: stuck, turning -90, md = %.2f\n", move_dist );
			}
		}
	
		hook->begin_turn (self);

		return;
	}	
	else
	{
		hook->dflags -= (hook->dflags & DFL_TURNED);
		hook->turn_dir = 0;

		//	support old type of turning for now
		if ( hook->dflags & DFL_FACENONE )
		{
			// face the goal entity
			if ( move_dir == MOVE_STRAFE && self->enemy )
			{
				ai_face_enemy(self);
			}
			else
			{
				ai_face_goal(self);
			}
		}
	}

	// if there is no gap and bot is close to facing its goal, then move
	if ( !gap )
	{
		dist = self->velocity[2];
		
		//	scale speed based on current frame's move_scale
		scale = FRAMES_ComputeFrameScale( hook );
		temp = forward * (speed * scale);
		self->velocity = temp;

		self->velocity.SetZ( 0 );
	}
	else
	{
		self->velocity.Zero();
	}

	hook->last_origin = self->s.origin;
}


// ----------------------------------------------------------------------------
//
// Name:		ai_ground_ent
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_ground_ent( userEntity_t *self )
{
	CVector	end = self->s.origin;
	end.z += (self->s.mins.z - 2);

// SCG[12/10/99]: 	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, end, self, CHOOSE_CLIP(self) );
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask );

	return tr.ent;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_strafetogoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_strafetogoal( userEntity_t *self, int move_dir )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			yaw_right, yaw_left, wall_yaw;
	CVector			end, normal, dir;
	int				restart_count = 0;

	ai_debug_print (self, "ai_strafetogoal\n");

	// choose a random direction if we aren't already strafing
	if ( hook->strafe_dir < 1 || hook->strafe_dir > 2 || hook->strafe_time <= gstate->time )
	{
		hook->strafe_time = gstate->time + rnd () * (640 / hook->attack_speed) + 0.5;
		hook->strafe_dir = rand () % 2 + 1;
	}

	yaw_right = yaw_left = self->ideal_ang [YAW];

	if (move_dir == MOVE_STRAFE)
	{
		yaw_right -= 90.0;
		yaw_left += 90.0;
	}
	else 
	if (move_dir == MOVE_STRAFEFORWARD)
	{
		yaw_right -= 45;
		yaw_left += 45;
	}
	else 
	if (move_dir == MOVE_STRAFEBACKWARD)
	{
		yaw_right -= 135;
		yaw_left += 135;
	}

strafe_restart:
	yaw_right = AngleMod(yaw_right);
	yaw_left = AngleMod(yaw_left);

	if (hook->strafe_dir == STRAFE_LEFT)
	{
		dir.Set(0, yaw_left, 0);
	}
	else
	{
		dir.Set(0, yaw_right, 0);
	}

	dir.AngleToVectors(forward, right, up);
		
	end = self->s.origin + forward * 32.0;//hook->run_speed * hook->think_time;

// SCG[12/10/99]: 	tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, CHOOSE_CLIP(self));
	tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
	if ( tr.fraction < 1.0 )
	{
		if ( restart_count < 7 )
		{
			//	get yaw of obstruction normal, disregarding z
			normal = tr.plane.normal;
			normal.SetZ( 0 );
			normal.Normalize ();
		
			wall_yaw = VectorToYaw(normal);
			yaw_right = wall_yaw - 90;
			yaw_left = wall_yaw + 90;

			restart_count++;

			goto strafe_restart;
		}
		else
		{
			self->velocity.Zero();
			return;
		}
	}
	else
	{
		//	set a temp goal where trace ended
		ai_set_goal(self, NULL, tr.endpos, GOAL_TEMP);
	}

	ai_movetogoal(self, hook->attack_speed, MOVE_STRAFE);
/*
	arrow = gstate->SpawnEntity ();
	arrow->movetype = MOVETYPE_NONE;
	arrow->solid = SOLID_NOT;
	gstate->SetModel (arrow, debug_models [DEBUG_ARROW]);
	gstate->SetOrigin (arrow, self->s.origin [0], self->s.origin [1], self->s.origin [2] + 16.0);
	arrow->s.angles = dir;
	arrow->nextthink = gstate->time + 0.25;
	arrow->think = arrow_remove;
*/
}

// ----------------------------------------------------------------------------
//
// Name:		ai_attack_move
// Description:
//			try to stay within hook->attack_dist - 64 and 
//			hook->attack_dist / 2 units of enemy
//			circle strafe if bot is allowed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_attack_move( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			dist;

	dist = ai_goal_dist(self, self->enemy);

	if (dist > hook->attack_dist - 64.0)
	{
		if (hook->dflags & DFL_CANSTRAFE)
		{
			ai_strafetogoal (self, MOVE_STRAFEFORWARD);
		}
		else
		{
			ai_movetogoal (self, hook->attack_speed, MOVE_FORWARD);
		}
	}
	else 
	if (dist < hook->attack_dist / 2)
	{
		if (hook->dflags & DFL_CANSTRAFE)
		{
			ai_strafetogoal (self, MOVE_STRAFEBACKWARD);
		}
		else
		{
			ai_movetogoal (self, hook->attack_speed, MOVE_FORWARD);
		}
	}
	else
	{
		ai_strafetogoal (self, MOVE_STRAFEFORWARD);
	}
/*
	if (hook->dflags & DFL_CANSTRAFE)
	{
		if (dist > hook->attack_dist - 64.0)
			ai_strafetogoal (self, MOVE_STRAFEFORWARD);
		else if (dist < hook->attack_dist / 2)
			ai_strafetogoal (self, MOVE_STRAFEBACKWARD);
		else
			ai_strafetogoal (self, MOVE_STRAFEFORWARD);
	}
	else
	{
		if (dist < hook->attack_dist / 2)
			ai_movetogoal (self, hook->attack_speed, MOVE_BACKWARD);
		else
			ai_movetogoal (self, hook->attack_speed, MOVE_FORWARD);
	}
*/
}

// ----------------------------------------------------------------------------
//
// Name:		ai_need_item
// Description:
//	searches the entities inventory
//	and determines if item is needed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_need_item (userEntity_t *self, userEntity_t *item)
{
	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_orient_to_normal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_orient_to_normal( userEntity_t *self, CVector &f_norm )
{
	CVector	f_ang;
	float	theta, pitch;

	f_norm.Normalize();

	//	get floor normal's angles
	f_norm.VectorToAngles( f_ang );
	
	//	get angle between floor normal's yaw and model's yaw
	theta = AngleDiff( f_ang.y, self->s.angles[1] );
	if (theta > 90)
	{
		pitch = 90 - f_ang.Pitch();

		self->ideal_ang.SetPitch( ((theta - 90) / 90) * pitch );
		self->ideal_ang.SetRoll( pitch - self->ideal_ang[PITCH] );
	}
	else 
	if (theta < -90)
	{
		pitch = 90 - f_ang.Pitch();

		self->ideal_ang.SetPitch( ((theta + 90) / 90) * (-pitch) );
		self->ideal_ang.SetRoll( -(pitch - self->ideal_ang[PITCH]) );
	}
	else 
	if (theta > 0)  // && theta <= 90
	{
		pitch = f_ang.Pitch() - 90;

		self->ideal_ang.SetRoll( (theta / 90) * (-pitch) );
		self->ideal_ang.SetPitch( pitch + self->ideal_ang[ROLL] );
	}
	else // theta < 0 && theta >= -90
	{
		pitch = f_ang.Pitch() - 90;
		
		self->ideal_ang.SetRoll( (theta / 90) * (-pitch) );
		self->ideal_ang.SetPitch( pitch - self->ideal_ang[ROLL] );
	}

	//	gl hack
	self->ideal_ang.SetRoll( -self->ideal_ang[ROLL] );

//	gstate->Con_Printf ("theta = %.2f, pitch = %.2f ideal_pitch = %.2f roll = %.2f\n", theta, pitch, self->ideal_ang [0], self->s.angles [2]);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_orient_to_floor
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_orient_to_floor( userEntity_t *self )
{
	CVector m_ang, end;

	m_ang = self->s.angles;
	m_ang.AngleToVectors(forward, right, up);

	end = self->s.origin;
	end = end - up * 32.0;

	tr = gstate->TraceLine_q2( self->s.origin, end, self, MASK_MONSTERSOLID );
	if ( tr.fraction < 1.0 )
	{
		ai_orient_to_normal(self, tr.plane.normal);
	}
	else
	{
		self->ideal_ang.SetPitch( 0 );
		self->ideal_ang.SetRoll( 0 );
	}

	//	adjust instantly
	com->ChangePitch(self);
	com->ChangeRoll(self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_orient_to_gravity
// Description:
//	fun for a laugh, but there are too many things that need fixing in
//	physics (actually I fixed most of that), environment sampling and client
//	side control for this to pay off
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_orient_to_gravity( userEntity_t *self )
{
	CVector	f_norm, f_ang, m_ang, ang_speed_save;
	float	theta, pitch;

	//	FIXME: orient to self->gravity_dir when implemented in engine physics

	f_norm = gstate->gravity_dir * -1;
	f_norm.Normalize ();

	//	get floor normal's angles
	f_norm.VectorToAngles( f_ang );
	
	//	get angle between floor normal's yaw and model's yaw
	theta = AngleDiff( f_ang.Yaw(), m_ang.Yaw() );

	if ( theta > 90 )
	{
		pitch = 90 - f_ang.Pitch();

		self->ideal_ang.SetPitch( ((theta - 90) / 90) * pitch );
		self->ideal_ang.SetRoll( pitch - self->ideal_ang[PITCH] );
	}
	else 
	if (theta < -90)
	{
		pitch = 90 - f_ang.Pitch();

		self->ideal_ang.SetPitch( ((theta + 90) / 90) * (-pitch) );
		self->ideal_ang.SetRoll( -(pitch - self->ideal_ang[PITCH]) );
	}
	else 
	if (theta > 0)  // && theta <= 90
	{
		pitch = f_ang.Pitch() - 90;

		self->ideal_ang.SetRoll( (theta / 90) * (-pitch) );
		self->ideal_ang.SetPitch( pitch + self->ideal_ang[ROLL] );
	}
	else // theta < 0 && theta >= -90
	{
		pitch = f_ang.Pitch() - 90;
		
		self->ideal_ang.SetRoll( (theta / 90) * (-pitch) );
		self->ideal_ang.SetPitch( pitch - self->ideal_ang[ROLL] );
	}

	//	gl hack
	self->ideal_ang.SetRoll( -self->ideal_ang[ROLL] );

	ang_speed_save = self->ang_speed;
	self->ang_speed.Multiply( 0.25f );

	com->ChangePitch(self);
	com->ChangeRoll(self);

	if ( self->flags & FL_CLIENT && (self->ideal_ang[PITCH] != self->s.angles[PITCH]) )
	{
		self->fixangle = TRUE;
	}

	self->ang_speed = ang_speed_save;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_adjust_angles
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_adjust_angles( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	CVector			ang_speed_save;

	//	get ideal_ang for this entity
	if ( !ai_set_ideal(self) )
	{
		return;
	}

	ang_speed_save = self->ang_speed;

	self->ang_speed.Multiply( gstate->frametime );

	com->ChangeYaw(self);

	if (hook->dflags & DFL_FACEPITCH )
	{
		com->ChangePitch(self);
	}

	com->ChangeRoll(self);

	self->ang_speed = ang_speed_save;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_prethink
// Description:
//			handles some things that must be done every frame
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_prethink( userEntity_t *self )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;

	if (self->flags & FL_INWARP)
	{
		return;
	}

	// determine if the player/bot has left the ground between nodes
	// used for determining when to link nodes back

	if ( !self->groundEntity )
	{
		hook->bOffGround = TRUE;
	}

	// get the bot's water level for this frame
	ai_water_level (self);

	// do damage to bot if in water/lava/slime
	ai_water_damage (self);

	com->FrameUpdate (self);

	///////////////////////////////////////////////////////////////////////////
	//	turning done here for much smoother turning rates
	//	ang_speed is in degree's per second, so scale ang_speed for each frame
	//	based on gstate->frametime
	///////////////////////////////////////////////////////////////////////////
	
	//	don't turn if DFL_FACENONE or jumping
	if (!(hook->dflags & DFL_FACENONE) && !(hook->ai_flags & AI_JUMP))
	{
		ai_adjust_angles (self);
	}

//	ai_orient_to_gravity (self);

	//if (hook->dflags & DFL_ORIENTTOFLOOR)
	//	ai_orient_to_floor (self);

	///////////////////////////////////////////////////////////////////////////
	//	poisoning
	///////////////////////////////////////////////////////////////////////////

	if ( (hook->items & IT_POISON) && hook->type != TYPE_MEDUSA )
	{
		//	do damage
		if ( hook->poison_next_damage_time <= 0 )
		{
			gstate->Con_Printf ("poisoning\n");
			com->Damage (self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time <= 0)
		{
			hook->items &= ~IT_POISON;
		}
		else
		{
			hook->poison_time -= 0.1f;
//			hook->poison_interval -= 0.1;
			hook->poison_next_damage_time -= 0.1f;
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		ai_seek_temporary_goal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_seek_temporary_goal( userEntity_t *self, float dist )
{
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_seek_temporary_goal", NULL);
#endif

	up = self->goalentity->s.origin - self->s.origin;
	up.VectorToAngles( temp );
	AngleToVectors( temp, forward, right, up );

	// if we are close enough to our goalentity, go for owner again
	if ( dist < REACHED_GOAL_DIST )
	{
		ai_restore_goal(self);
	}
	else 
	if ( hook->tempgoal_search_time <= gstate->time )
	{
		ai_debug_print(self, "tempgoal_search_time expired.\n");
		ai_restore_goal(self);
	}
	else
	{
		ai_movetogoal(self, hook->run_speed, MOVE_FORWARD);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_seek_item
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_seek_item( userEntity_t *self, float dist )
{
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_seek_item", NULL);
#endif

	up = self->goalentity->s.origin - self->s.origin;
	up.VectorToAngles( temp );
	AngleToVectors( temp, forward, right, up );

	// if we are close enough to our goalentity, go for owner again
	if (dist <= self->s.maxs [1] + 8.0)
	{
		ai_restore_goal (self);
		ai_debug_print (self, "reached goal item\n");
	}
	else 
	if (hook->tempgoal_search_time <= gstate->time)
	{
		ai_restore_goal (self);
	}
	else
	{
		ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_path_for_task
// Description:
// Input:
// Output:
//	returns:
//		FALSE if no valid path
//		TRUE if a path was found
// Note:
//
// ----------------------------------------------------------------------------
int	ai_path_for_task( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	int				task_type;
	taskData_t		*task_data;

	// SGC:2-26-99
	// variable is used before it was initialized
	// statement moved below task_current
//	if (task_data->nGoalNodeIndex < 0)
//		return	FALSE;

	task_current (hook->task_stack, &task_data, &task_type);

	if (task_data->nGoalNodeIndex < 0)
		return	FALSE;

	//////////////////////////////////////////////////////////////////
	//	path to goal node
	//////////////////////////////////////////////////////////////////

	// get rid of old path
	PATHLIST_KillPath (hook->pPathList);

	// find shortest path from current node to goal node
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );

	MAPNODE_PTR pGoalNode = NODE_GetNode( pNodeHeader, task_data->nGoalNodeIndex );
	PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);


	// store last node in the path for comparison later

	if (hook->pPathList->pPath)
	{
		if (hook->pPathList->pPath->nNodeIndex >= 0)
		{
			if (hook->pPathList->pPath->nNodeIndex == hook->pNodeList->nCurrentNodeIndex)
			{
				// we're already at this node, so delete the first one
				PATHLIST_DeleteFirstInPath (hook->pPathList);
				if (hook->pPathList->pPath)
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
					_ASSERTE( pNode );

					ai_set_goal (self, pNode, pNode->position, GOAL_NODE);
				}
			}
			else
			{
				MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
				_ASSERTE( pNode );
				
				ai_set_goal (self, pNode, pNode->position, GOAL_NODE);
			}
		}
		return	TRUE;

	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_go_to_owner
// Description:
//	set up a new TASK_FOLLOW to owner
//
//	if a path cannot be found, then pop off the TASK_CHASE
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_go_to_owner( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;

	//	enemy is no longer around
	if (!hook->owner)
		return	FALSE;

	//	enemy is dead already
	if (hook->owner->deadflag != DEAD_NO)
		return	FALSE;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_FOLLOW
	//////////////////////////////////////////////////////////////////

	task_chase (self, hook->owner);
	task_data = task_current_data (hook->task_stack);
	ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OWNER);

	if (task_data->nGoalNodeIndex != task_data->nBaseNodeIndex)
	{
		if (ai_path_for_task (self))
		{
			hook->begin_path_follow (self);
			return	TRUE;
		}
	}

	hook->begin_follow (self);
	
	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_go_to_enemy
// Description:
//		set up a new TASK_CHASE and find a path to the the enemy
//
//		if a path cannot be found, then pop off the TASK_CHASE
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_go_to_enemy( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;

	//	enemy is no longer around
	if (!self->enemy)
		return	FALSE;

	//	enemy is dead already
	if (self->enemy->deadflag != DEAD_NO)
		return	FALSE;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_CHASE
	//////////////////////////////////////////////////////////////////
	task_chase (self, self->enemy);
	task_data = task_current_data (hook->task_stack);
	ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_ENEMY);

	if (task_data->nGoalNodeIndex != task_data->nBaseNodeIndex)
	{
		if (ai_path_for_task (self))
		{
			hook->begin_path_follow (self);
			return	TRUE;
		}
	}

	hook->begin_follow (self);
	
	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_go_to_switch
// Description:
//	set up a new task switch and find a path
//	to the switch
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_go_to_switch( userEntity_t *self, userEntity_t *sw )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_SWITCH
	//////////////////////////////////////////////////////////////////
	task_switch (self, sw);
	task_data = task_current_data (hook->task_stack);
	ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OBJECT);

	if (task_data->nGoalNodeIndex == task_data->nBaseNodeIndex)
	{
		hook->begin_follow (self);
		return	TRUE;
	}
	else if (ai_path_for_task (self))
	{
		hook->begin_path_follow (self);
		return	TRUE;
	}

	//	can't get to switch, so pop TASK_SWITCH
	task_pop (&hook->task_stack);
	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_ride_plat
// Description:
//			set up a TASK_RIDE_PLAT
//
//			make bot go to location of plat (final_node of TASK_PLAT/TASK_DOOR)
//			then make bot wait until correct state is achieved
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_ride_plat( userEntity_t *self, userEntity_t *plat, MAPNODE_PTR final_node )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	//CVector			org;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_RIDE_PLAT
	//////////////////////////////////////////////////////////////////

	task_ride_plat (self, plat, final_node);
	task_data = task_current_data (hook->task_stack);
	
	//	make sure bot stops on center of plat to avoid
	//	getting hung up on overhangs
	ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OBJECT);

	if (task_data->nGoalNodeIndex == task_data->nBaseNodeIndex)
	{
		hook->begin_follow (self);
		return	TRUE;
	}

	else if (ai_path_for_task (self))
	{
		hook->begin_path_follow (self);
		return	TRUE;
	}

	//	can't get to plat so pop TASK_RIDE_PLAT
	task_pop (&hook->task_stack);

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_go_to_plat
// Description:
//	set up a door task:
//	path to a point in front of the plat
//	and wait for correct state
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_go_to_plat( userEntity_t *self, userEntity_t *plat, MAPNODE_PTR base_node )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	MAPNODE_PTR		goal_node;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_PLAT
	//////////////////////////////////////////////////////////////////

	goal_node = base_node;

	if (ai_ground_ent (self) == plat)
	{
		goal_node = node_linked_with_target( hook->pNodeList->pNodeHeader, base_node, plat->targetname);

		ai_ride_plat (self, plat, goal_node);
		return	TRUE;
	}

	if (goal_node)
	{
		task_plat (self, plat, base_node, goal_node);

		task_data = task_current_data (hook->task_stack);
		MAPNODE_PTR pFinalNode = node_linked_with_target( hook->pNodeList->pNodeHeader, base_node, plat->targetname);
		task_data->nFinalNodeIndex = pFinalNode->nIndex;
	
		MAPNODE_PTR pGoalNode = NODE_GetNode( hook->pNodeList->pNodeHeader, task_data->nGoalNodeIndex );
		ai_set_goal(self, pGoalNode, pGoalNode->position, GOAL_NODE);

		if (task_data->nGoalNodeIndex == task_data->nBaseNodeIndex)
		{
			hook->begin_follow (self);
			return	TRUE;
		}
		else if (ai_path_for_task (self))
		{
			hook->begin_path_follow (self);
			return	TRUE;
		}

		//	can't get to plat so pop TASK_PLAT
		task_pop (&hook->task_stack);
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_go_to_door
// Description:
//	set up a door task:
//	path to a point in front of the door
//	and wait for it to open
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_go_to_door( userEntity_t *self, userEntity_t *door, MAPNODE_PTR base_node )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	MAPNODE_PTR		goal_node;

	//////////////////////////////////////////////////////////////////
	//	push/update a TASK_DOOR
	//////////////////////////////////////////////////////////////////

	goal_node = base_node;

	if (ai_ground_ent (self) == door)
	{
//		gstate->Con_Printf ("standing on door\n");

		goal_node = node_linked_with_target( hook->pNodeList->pNodeHeader, base_node, door->targetname);

		ai_ride_plat (self, door, goal_node);
		return	TRUE;
	}

	if (goal_node)
	{
		task_door (self, door, base_node, goal_node);

		task_data = task_current_data (hook->task_stack);
		MAPNODE_PTR pFinalNode = node_linked_with_target( hook->pNodeList->pNodeHeader, base_node, door->targetname );
		task_data->nFinalNodeIndex = pFinalNode->nIndex;

		MAPNODE_PTR pGoalNode = NODE_GetNode( hook->pNodeList->pNodeHeader, task_data->nGoalNodeIndex );
		ai_set_goal(self, pGoalNode, pGoalNode->position, GOAL_NODE);		

		if (task_data->nGoalNodeIndex == task_data->nBaseNodeIndex)
		{
			hook->begin_follow (self);
			return	TRUE;
		}

		else if (ai_path_for_task (self))
		{
			hook->begin_path_follow (self);
			return	TRUE;
		}

		//	can't get to door so pop TASK_PLAT
		task_pop (&hook->task_stack);
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_do_task
// Description:
//	called when a series of tasks is completed
//	and a bot needs to go back to doing what it
//	was before those tasks were pushed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_do_task( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ehook;
	taskData_t		*task_data;
	int				task_type;

	task_current (hook->task_stack, &task_data, &task_type);

	switch (task_type)
	{
		case	TASK_NODE_WANDER:
		{
			MAPNODE_PTR pCurrentNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
			ai_set_goal (self, pCurrentNode, pCurrentNode->position, GOAL_NODE);

			hook->begin_node_wander (self);
			break;
		}
		case	TASK_CHASE:
		{
			//	make sure goalentity is enemy!!
			ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_ENEMY);
			self->enemy = task_data->object;
			
			//	update TASK_CHASE with new cur_node of enemy
			ehook = (playerHook_t *) self->enemy->userHook;

			if (self->enemy->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))				
				task_data->nGoalNodeIndex = ehook->pNodeList->nCurrentNodeIndex;
			else
			{
				MAPNODE_PTR pClosestNode = node_closest (hook->pNodeList, task_data->object->s.origin, 32767.0);
				task_data->nGoalNodeIndex = pClosestNode->nIndex;
			}

			if (!ai_path_for_task (self))
				//	can't path to enemy, so go back to ai_follow
				hook->begin_follow (self);
			break;
		}
		case	TASK_STAND:
		{
			hook->begin_stand (self);
			break;
		}
		default:
			com->Error ("ai_do_task: unimplemented task type");
			break;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wait_for_state_do_task
// Description:
//	obj_desired_state is achieved
//	so go to final_node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_wait_for_state_do_task( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	int				task_type;
	MAPNODE_PTR		final_node;
	userEntity_t	*plat;

	task_current (hook->task_stack, &task_data, &task_type);
	plat = task_data->object;
	final_node = NODE_GetNode( hook->pNodeList->pNodeHeader, task_data->nFinalNodeIndex );

	switch (task_type)
	{
		case	TASK_DOOR:
			//	TASK_DOOR completed, so pop it
			task_pop (&hook->task_stack);

			//	go to the door and ride it until next static state
			if (!ai_ride_plat (self, plat, final_node))
				ai_do_task (self);
			break;
		case	TASK_PLAT:
			//	TASK_PLAT completed, so pop it
			task_pop (&hook->task_stack);

			//	go to the plat and ride it until next static state
			if (!ai_ride_plat (self, plat, final_node))
				ai_do_task (self);
			break;
		case	TASK_RIDE_PLAT:
			//	TASK_RIDE_PLAT completed, so pop it
			task_pop (&hook->task_stack);

			//	act on previous task
			ai_do_task (self);
			break;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wait_for_state
// Description:
//	bot is waiting for a door or plat
//	to reach obj_desired_state before
//	going through/getting on it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_wait_for_state( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	doorHook_t		*dhook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_stand", NULL);
#endif

	if (hook->find_target (self))
	{
		//	FIXME: we've now got a TASK_DOOR or TASK_PLAT
		//	pushed on before a TASK_CHASE... probably won't
		//	be handled correctly once TASK_CHASE is complete.
		//	perhaps it should just be removed
		return;
	}

	if (hook->wander_sound) 
		hook->wander_sound (self);

	ai_frame_sounds (self);	

	/////////////////////////////////////////////////////////////////
	//	check for correct state of goal object
	/////////////////////////////////////////////////////////////////

	task_data = task_current_data (hook->task_stack);
	dhook = (doorHook_t *) task_data->object->userHook;

	if (dhook->state == task_data->obj_desired_state)
	{
		gstate->Con_Printf ("obj_desired_state achieved.\n");

		ai_wait_for_state_do_task (self);
		return;
	}

	//	face plat/door
	ai_face_goal (self);

	// do an ambient standing sequence
	// FIXME: won't search for targets while doing this...
	// may be a problem for long ambients, may need a special
	// transition think for this
	if (self->s.frameInfo.frameState & FRSTATE_LAST)
	{
		self->s.frameInfo.frameState -= FRSTATE_LAST;

		if (rnd () < 0.2)
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
			AI_StartSequence(self, pSequence);
			return;
		}
	}

	ai_set_think (self, ai_wait_for_state, "ai_wait_for_state");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_wait_for_state
// Description:
//	bot is waiting for a door or plat
//	to reach obj_desired_state before
//	going through/getting on it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_wait_for_state( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	CVector			vel;

	self->velocity.Zero();
	
	/////////////////////////////////////////////////////////////////
	//	face the final destination
	/////////////////////////////////////////////////////////////////

	task_data = task_current_data (hook->task_stack);
	ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OBJECT);
	ai_face_goal (self);

	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
	AI_StartSequence(self, pSequence);

	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	ai_wait_for_state (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_find_target_client_bot
// Description:
//	returns true if a targetable entity was found
//
//	DOES NOT search for items!
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_find_target_client_bot( userEntity_t *self )
{
	userEntity_t	*head;
	userEntity_t	*item, *enemy;
	int				dist, item_d, enemy_d, alignment;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*head_hook;

	if (!hook->begin_follow)
		return	NULL;

	if (!hook->begin_follow && !hook->sight_sound)
		return	NULL;

	//////////////////////////////////////////////////////////////////
	//	find alignment of searching creature
	//////////////////////////////////////////////////////////////////

	alignment = hook->dflags & (DFL_BERSERK + DFL_EVIL + DFL_NEUTRAL + DFL_GOOD);
	
	//	neutral entities never attack unless provoked
	if (alignment == DFL_NEUTRAL)
		return NULL;

	item = enemy = NULL;
	item_d = enemy_d = 32767;
	
	//	FIXME: searches throught entire list...
	//	perhaps stupider monsters could stop searching at first visible
	//	monster or item
	for (head = alist_FirstEntity (client_bot_list); head; head = alist_NextEntity (client_bot_list))
	{
		if (head == self)
			continue;

		// is this a client, bot or monster?
		if (head->health > 0 && !(head->flags & FL_NOTARGET) && (head->deadflag == DEAD_NO))
		{
			head_hook = (playerHook_t *) head->userHook;

			if ((alignment == DFL_EVIL && head_hook->dflags & DFL_GOOD) ||
				(alignment == DFL_GOOD && head_hook->dflags & DFL_EVIL))
			{
				dist = VectorDistance (self->s.origin, head->s.origin);

				if (dist < enemy_d)
				{
					if (ai_visible (self, head))
					{
						enemy = head;
						enemy_d = dist;
					}
				}
			}
		}
/*
		///////////////////////////////////////////////////////////////
		//	item search, bots only
		///////////////////////////////////////////////////////////////
		else if (self->flags & FL_BOT)
		{
			if (head->flags & FL_ITEM)
			{
				dist = VectorDistance (self->s.origin, head->s.origin);

				if (dist < item_d)
				{
					if (ai_need_item (self, head))
					{
						if (AI_IsVisible(self, head))
						{
							// FIXME: make sure item is in our current node
							item = head;
							item_d = dist;
						}
					}
				}
			}
		}
*/
	}

	if (enemy)
	{
		if (hook->sight_sound && enemy != self->enemy) 
			hook->sight_sound (self);		

		self->enemy = enemy;
		ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);

#ifdef	DEBUGINFO
		ai_debug_print (self, "sighted enemy %s\n", enemy->className);
#endif

		if (!ai_go_to_enemy (self))
			return	NULL;
		else if (hook->sight)
			hook->sight (self);

		return	enemy;
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_find_target_monster_bot
// Description:
//	searches monster_list for targets
//	returns true if a targetable entity was found
//
//	DOES NOT search for items!
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_find_target_monster_bot( userEntity_t *self )
{
	userEntity_t	*head;
	userEntity_t	*item, *enemy;
	int				dist, item_d, enemy_d, alignment;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*head_hook;

	if (!hook->begin_follow)
		return	NULL;

	if (!hook->begin_follow && !hook->sight_sound)
		return	NULL;

	//////////////////////////////////////////////////////////////////
	//	find alignment of searching creature
	//////////////////////////////////////////////////////////////////

	alignment = hook->dflags & (DFL_BERSERK + DFL_EVIL + DFL_NEUTRAL + DFL_GOOD);
	
	//	neutral entities never attack unless provoked
	if (alignment == DFL_NEUTRAL)
		return NULL;

	item = enemy = NULL;
	item_d = enemy_d = 32767;
	
	//	FIXME: searches throught entire list...
	//	perhaps stupider monsters could stop searching at first visible
	//	monster or item
	for (head = alist_FirstEntity (monster_bot_list); head; head = alist_NextEntity (monster_bot_list))
	{
		if (head == self)
			continue;

		// is this a client, bot or monster?
		if (head->health > 0 && !(head->flags & FL_NOTARGET) && (head->deadflag == DEAD_NO))
		{
			head_hook = (playerHook_t *) head->userHook;

			if ((alignment == DFL_EVIL && head_hook->dflags & DFL_GOOD) ||
				(alignment == DFL_GOOD && head_hook->dflags & DFL_EVIL))
			{
				dist = VectorDistance (self->s.origin, head->s.origin);

				if (dist < enemy_d)
				{
					if (ai_visible (self, head))
					{
						enemy = head;
						enemy_d = dist;
					}
				}
			}
		}
/*
		///////////////////////////////////////////////////////////////
		//	item search, bots only
		///////////////////////////////////////////////////////////////
		else if (self->flags & FL_BOT)
		{
			if (head->flags & FL_ITEM)
			{
				dist = VectorDistance (self->s.origin, head->s.origin);

				if (dist < item_d)
				{
					if (ai_need_item (self, head))
					{
						if (AI_IsVisible (self, head))
						{
							// FIXME: make sure item is in our current node
							item = head;
							item_d = dist;
						}
					}
				}
			}
		}
*/
	}

	if (enemy)
	{
		if (hook->sight_sound && enemy != self->enemy) 
			hook->sight_sound (self);		

		self->enemy = enemy;
		ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);

#ifdef	DEBUGINFO
		ai_debug_print (self, "sighted enemy %s\n", enemy->className);
#endif

		if (!ai_go_to_enemy (self))
			return	NULL;
		else if (hook->sight)
			hook->sight (self);

		return	enemy;
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_find_target_berserk
// Description:
//	searches entire entity list for targets
//	returns true if a targetable entity was found
//
//	DOES NOT search for items!
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_find_target_berserk( userEntity_t *self )
{
	userEntity_t	*head;
	userEntity_t	*item, *enemy;
	int				dist, item_d, enemy_d, alignment;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	if (!hook->begin_follow)
		return	NULL;

	if (!hook->begin_follow && !hook->sight_sound)
		return	NULL;

	//////////////////////////////////////////////////////////////////
	//	find alignment of searching creature
	//////////////////////////////////////////////////////////////////

	alignment = hook->dflags & (DFL_BERSERK + DFL_EVIL + DFL_NEUTRAL + DFL_GOOD);
	
	//	neutral entities never attack unless provoked
	if (alignment == DFL_NEUTRAL)
		return NULL;

	item = enemy = NULL;
	item_d = enemy_d = 32767;
	
	//	FIXME: searches throught entire list...
	//	perhaps stupider monsters could stop searching at first visible
	//	monster or item
	for (head = gstate->FirstEntity (); head; head = gstate->NextEntity (head))
	{
		if (head == self)
			continue;

		//////////////////////////////////////////////////////////////
		//	berserk entity, attacks anything that can be damaged
		//////////////////////////////////////////////////////////////
		if (head->flags & (FL_CLIENT + FL_BOT + FL_MONSTER + FL_ITEM) &&
			head->takedamage == DAMAGE_YES)
		{
			dist = VectorDistance (self->s.origin, head->s.origin);

			if (dist < enemy_d)
			{
				if (ai_visible (self, head))
				{
					enemy = head;
					enemy_d = dist;
				}
			}
		}
	}

	if (enemy)
	{
		if (hook->sight_sound && enemy != self->enemy) 
			hook->sight_sound (self);		

		self->enemy = enemy;
		ai_set_goal (self, self->enemy, self->enemy->s.origin, GOAL_ENEMY);

#ifdef	DEBUGINFO
		ai_debug_print (self, "sighted enemy %s\n", enemy->className);
#endif

		if (!ai_go_to_enemy (self))
			return	NULL;
		else if (hook->sight)
			hook->sight (self);

		return	enemy;
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_next_monster_path_corner
// Description:
//			finds the closest monster_path_corner
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_next_monster_path_corner( userEntity_t *self )
{
	userEntity_t	*corner = self->goalentity->owner;
	mpathHook_t		*hook = (mpathHook_t *) corner->userHook;
	userEntity_t	*next;
	int				num_targets, target_num;

	if (corner)
		ai_debug_print (self, "corner->className = %s\n", corner->className);
	if (!corner)
	{
		ai_debug_print (self, "no corner\n");
		return	FALSE;
	}
	if (!hook)
	{
		ai_debug_print (self, "no hook\n");
		return	FALSE;
	}

	// find the number of targets on the path corner that we have reached
	for (num_targets = 0; num_targets < 4 && hook->target [num_targets] != NULL; num_targets++);

	ai_debug_print (self, "num_targets = %i\n", num_targets);

	target_num = 0;
	if (num_targets > 1)
		target_num = (int) (rnd () * (float) num_targets);

	ai_debug_print (self, "looking for target #%i, %s\n", target_num, hook->target [target_num]);
	// find the entity with the matching target name
	next = com->FindTarget (hook->target [target_num]);

	if (!next)
		return	FALSE;

	ai_set_goal (self, next, next->s.origin, GOAL_PATH);
	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_path_wander
// Description:
//				monster is wandering from one monster_path_corner
//				to the next
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_path_wander( userEntity_t *self )
{
	float			dist;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_path_wander", NULL);
#endif

	if (!ai_client_near (self) && self->groundEntity)
	{
		self->velocity.Zero();
		self->nextthink = gstate->time + hook->think_time;
		self->think = ai_path_wander;
		return;
	}

	if (hook->find_target (self))
		return;

	if (hook->wander_sound) hook->wander_sound (self);

	dist = ai_goal_dist (self, self->goalentity);
//	dist = VectorDistance (self->s.origin, self->goalentity->s.origin);

	ai_frame_sounds (self);

	// go to goal entity
	ai_face_goal (self);

	if (hook->goal_type != GOAL_TEMP)
	{
		// we've reached the path_corner but haven't started the action yet
		if (dist < REACHED_GOAL_DIST)
		{
			if (!(hook->ai_flags & AI_ACTION))
				hook->begin_action (self);
			else
				hook->ai_flags -= AI_ACTION;
							
			if (!(hook->ai_flags & AI_ACTION))
			{
				if (!ai_next_monster_path_corner (self))
				{
					ai_debug_print (self, "ai_path_wander: couldn't find monster_path_corner\n");
		
					self->think = hook->begin_stand;
					self->nextthink = gstate->time + hook->think_time;
					return;
				}
			}
			else 
				return;
		}

		ai_movetogoal (self, hook->walk_speed, MOVE_FORWARD);
	}
	// we must be seeking to a temporary goal to get around an obstruction
	else
		ai_seek_temporary_goal (self, dist);

	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_path_wander, "ai_path_wander");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_path_wander
// Description:
//	begin wandering from monster_path_corner to 
//	monster_path_corner
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_path_wander( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	userEntity_t	*target;

	ai_debug_print (self, "ai_begin_path_wander\n");

	if (!self->target)
	{
		// no target, so find the nearest monster_path_corner
		target = com->FindClosestEntity (self, "monster_path_corner");

		if (!target)
		{
			ai_debug_print (self, "ai_begin_path_wander: no visible target!\n");

			ai_set_think (self, hook->begin_stand, "ai_begin_path_wander");
	
			return;
		}
	}
	else
	{
		ai_debug_print (self, "finding target %s\n", self->target);
		// find the entity with the matching targetname
		target = com->FindTarget (self->target);
	}
	
	if (!target)
		com->Error ("ai_begin_path_wander: invalid target!\n");

	ai_set_goal (self, target, target->s.origin, GOAL_PATH);
	self->target = target->targetname;
	
	frameData_t *pSequence = FRAMES_GetSequence( self, "walka" );
	AI_StartSequence(self, pSequence);

	self->s.frameInfo.loopCount = 0;

	ai_path_wander (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wander_turn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_wander_turn( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	CVector			ang, end;
	
	ang = self->s.angles;
	
	if (rnd () < 0.5)
	{
		ang.y += (rnd() * 180.0);
	}
	else
	{
		ang.y += (-(rnd() * 180.0));
	}

	ang.AngleToVectors( forward, right, up );
	end = self->s.origin + forward * (REACHED_GOAL_DIST + 16.0);
	
	ai_set_goal( self, self->goalentity, end, GOAL_OBJECT );

	hook->goal_search_time = gstate->time + (REACHED_GOAL_DIST + 16.0) / hook->walk_speed;

	hook->begin_turn (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_choose_wander_goal
// Description:
//	puts self's goalentity slightly in front of it
//	lets ai_movetogoal handle turning along walls
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_choose_wander_goal( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	CVector			end, ang;

	ang = self->s.angles;
	ang.SetPitch( 0 );
	ang.SetRoll( 0 );
	ang.AngleToVectors(forward, right, up);
	end = self->s.origin + forward * (REACHED_GOAL_DIST + 16.0);
	
	ai_set_goal( self, self->goalentity, end, GOAL_OBJECT );

	hook->goal_search_time = gstate->time + (REACHED_GOAL_DIST + 16.0) / hook->walk_speed;

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_wander( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	float			dist;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_wander", NULL);
#endif

	//////////////////////////////////////////////////////////
	//	if no client is near, then stop and do nothing
	//	this saves framerate when lots of monsters are wandering
	//////////////////////////////////////////////////////////

	if (!ai_client_near (self) && self->groundEntity)
	{
		self->velocity.Zero();

		ai_set_think (self, ai_wander, "ai_wander");
		return;
	}

	if (hook->find_target (self))
		return;

	if (hook->wander_sound) hook->wander_sound (self);

	dist = ai_goal_dist (self, self->goalentity);

	ai_frame_sounds (self);

	/////////////////////////////////////////////////////////////////////////////
	//	temporary goal seeking (to get around obstructions)
	/////////////////////////////////////////////////////////////////////////////

	if (hook->goal_type == GOAL_TEMP)
	{
		ai_debug_print (self, "ai_wander: seeking temp goal", NULL);

		ai_seek_temporary_goal (self, dist);

		// only need to check this after movetogoal since that is when ai_jump is called
		if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
			//	FIXME: HUH?!?!?  Why ai_path_follow -- copy/paste bug?
			//	ai_set_think (self, ai_path_follow, "ai_wander");
			ai_set_think (self, ai_wander, "ai_wander");

		return;
	}

	/////////////////////////////////////////////////////////////////////////////
	//	going towards real goal
	/////////////////////////////////////////////////////////////////////////////

	if (dist < REACHED_GOAL_DIST || hook->goal_search_time <= gstate->time)
	{
		if (!ai_choose_wander_goal (self))
		{
			ai_set_think (self, hook->begin_stand, "ai_wander");
			return;
		}
		if ( rnd() < 0.075 )
		{
			ai_wander_turn (self);

			return;
		}
		else 
		if (self->s.frameInfo.frameState & FRSTATE_LAST)
		{
			if ( rnd() < 0.1 )
			{
				self->s.frameInfo.frameState -= FRSTATE_LAST;

				frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
				AI_StartSequence(self, pSequence);

				return;
			}
		}
	}

	// go to goal entity
	ai_movetogoal (self, hook->walk_speed, MOVE_FORWARD);

	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_wander, "ai_wander");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_wander( userEntity_t *self )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;

	//	set up a new goal
	if (!ai_choose_wander_goal (self))
	{
		ai_set_think (self, hook->begin_stand, "ai_begin_wander");

		return;
	}

	frameData_t *pSequence = NULL;
	if (self->movetype == MOVETYPE_FLY)
	{
		pSequence = FRAMES_GetSequence( self, "flya" );
		AI_StartSequence(self, pSequence);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "walka" );
		AI_StartSequence(self, pSequence);
	}

	ai_wander (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_check_node_target
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_check_node_target( userEntity_t *self, MAPNODE_PTR cur_node, MAPNODE_PTR next_node )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	userEntity_t	*node_target, *switch_target, *next_target;
	//CVector			origin;

	if (!cur_node->target)
		return	FALSE;

	node_target = com->FindTarget (cur_node->target);
	if (!node_target)
		return	FALSE;

	//	set up the correct task to deal with node's target
	if (!stricmp (node_target->className, "func_button") && node_target->use)
	{
		//	node we are going to doesn't target anything, so ignore cur_node target
		if (!next_node->target)
			return	FALSE;

		switch_target = com->FindTarget (node_target->target);
		next_target = com->FindTarget (next_node->target);

		//	switch or next_node have no target, so ignore cur_node target
		if (!switch_target || !next_target)
			return	FALSE;

		if (ai_go_to_switch (self, node_target))
		{
			gstate->Con_Printf ("going to switch\n");
			return	TRUE;
		}
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_furtherst_from
// Description:
//	finds a node linked to cur_node that is farther from
//	enemy node than is cur_node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_furthest_from( userEntity_t *self, MAPNODE_PTR cur_node, MAPNODE_PTR enemy_node )
{

}

// ----------------------------------------------------------------------------
//
// Name:		ai_use_switch
// Description:
//	use a switch that is the object of 
//	current task
//
//	start going to whatever object the switch
//	activated
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	ai_use_switch( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	taskData_t		*task_data;
	userEntity_t	*sw, *target;
	MAPNODE_PTR		base_node; 

	task_data = task_current_data (hook->task_stack);

	task_data->object->use (task_data->object, self, self);

	///////////////////////////////////////////////////////////
	//	now go to the correct node near the activated object
	///////////////////////////////////////////////////////////

	//	find the target of the switch bot pressed
	sw = task_data->object;
	base_node = NODE_GetNode( hook->pNodeList->pNodeHeader, task_data->nBaseNodeIndex );

	///////////////////////////////////////////////////////////
	//	pop off the TASK_SWITCH
	///////////////////////////////////////////////////////////
	
	task_pop (&hook->task_stack);

	target = com->FindTarget (sw->target);

	if (!target)
		//	something screwy is going on -- switch has no target
		return	FALSE;

	if (!stricmp (target->className, "func_plat"))
	{
		//	try to do a TASK_PLAT
		if (!ai_go_to_plat (self, target, base_node))
			return	FALSE;
	}
	else if (!stricmp (target->className, "func_door"))
	{
		//	try to do a TASK_DOOR
		if (!ai_go_to_door (self, target, base_node))
			return	FALSE;
	}
	else
		com->Error ("Used switch for unimplemented target");

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_follow_do_task
// Description:
//			reached target, decide what to do based
//			on current task type
//			
//			this is the only routine in which a TASK_ATTACK
//			should be encountered, because it was pushed on
//			immediately before in ai_follow and should be
//			removed before ever leaving monsters attack function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int	ai_follow_do_task( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ehook, *ohook;
	taskData_t		*task_data;
	int				task_type;

	task_current (hook->task_stack, &task_data, &task_type);

	switch	(task_type)
	{
		case	TASK_FOLLOW:
			//	restore owner from task_data->object... could this screw
			//	up if owner changes?  Will owners ever even change?
			hook->owner = task_data->object;

			//	make sure goalentity is our owner
			task_data->obj_origin = hook->owner->s.origin;
			ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_OWNER);
			
			//	update TASK_FOLLOW with new cur_node of owner and origin
			ohook = (playerHook_t *) hook->owner->userHook;
			if (hook->owner->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
			{
				task_data->nGoalNodeIndex = ohook->pNodeList->nCurrentNodeIndex;
			}
			else
			{
				// FIXME: shouldn't TASK_FOLLOW anything other than clients, bots and monsters
				
				MAPNODE_PTR pClosestNode = node_closest (hook->pNodeList, task_data->object->s.origin, 32767.0);
				task_data->nGoalNodeIndex = pClosestNode->nIndex;
			}

			if (ai_path_for_task (self))
				ai_set_think (self, hook->begin_path_follow, "ai_follow_do_task 2");
			else
			{
				//	can't path to owner, so go back to ai_follow
				ai_debug_print (self, "ai_follow_do_task: Can't path, returning to ai_follow\n");
				return	FALSE;
			}

			break;
			
		case	TASK_CHASE:
			//	make sure enemy is task_data->object, because it can change
			//	when an old task is restored in ai_killed_enemy
			self->enemy = task_data->object;

			//	make sure goalentity is enemy!!
			task_data->obj_origin = self->enemy->s.origin;
			ai_set_task_goal (self);
//			ai_set_goal (self, task_data->object, task_data->obj_origin, GOAL_ENEMY);
			
			//	update TASK_CHASE with new cur_node of enemy and origin
			ehook = (playerHook_t *) self->enemy->userHook;
			if (self->enemy->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
			{
				task_data->nGoalNodeIndex = ehook->pNodeList->nCurrentNodeIndex;
			}
			else
			{
				// FIXME: shouldn't TASK_CHASE anything other than clients, bots and monsters
				MAPNODE_PTR pClosestNode = node_closest (hook->pNodeList, task_data->object->s.origin, 32767.0);
				task_data->nGoalNodeIndex = pClosestNode->nIndex;
			}

			if (ai_path_for_task (self))
				ai_set_think (self, hook->begin_path_follow, "ai_follow_do_task 4");
			else
			{
				//	can't path to enemy, so go back to ai_follow
				ai_debug_print (self, "ai_follow_do_task: Can't path, returning to ai_follow\n");
				return	FALSE;
			}

			break;
		case	TASK_ATTACK:
//			hook->begin_attack (self);
			ai_set_think (self, hook->begin_attack, "ai_follow_do_task 5");
			break;
		case	TASK_NODE_WANDER:
//			hook->begin_node_wander (self);
			ai_set_think (self, hook->begin_node_wander, "ai_follow_do_task 6");
			break;
		case	TASK_WANDER:
			ai_set_think (self, hook->begin_wander, "ai_follow_do_task 7");
//			hook->begin_wander (self);
			break;
		case	TASK_PATH_WANDER:
			ai_set_think (self, hook->begin_path_wander, "ai_follow_do_task 8");
//			hook->begin_path_wander (self);
			break;
		case	TASK_SWITCH:
			if (!ai_use_switch (self))
			{
				//	switch had no target, so go back to ai_follow
				gstate->Con_Printf ("couldn't use switch\n");
				self->think = NULL;
				return	FALSE;
			}
			break;
		case	TASK_STAND:
			ai_set_think (self, hook->begin_stand, "ai_follow_do_task 9");
//			hook->begin_stand (self);
			break;
		case	TASK_PLAT:
		case	TASK_DOOR:
		case	TASK_RIDE_PLAT:
			gstate->Con_Printf ("ai_follow_do_task: waiting for obj_desired_state\n");
			ai_set_think (self, ai_begin_wait_for_state, "ai_follow_do_task 10");
			break;
		default:
			com->Error ("Unimplimented TASK type");
	}

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_killed_enenmy
// Description:
//			monster's current enemy was killed
//
//			pop off everything having to do with it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_killed_enemy( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	task_pop_thru (&hook->task_stack, self->enemy, TASK_CHASE);

	if (self->enemy->flags & FL_CLIENT)
	{
		//	make killer wander around if killed a player
		task_wander (self);

		//	player's view will follow enemy
		self->enemy->enemy = self;
	}

	//	monster goes back to doing whatever it was before 
	//	chasing last enemy
	ai_follow_do_task (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_follow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_follow( userEntity_t *self )
{
	int				task_type, depth;
	float			dist;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ehook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_follow", NULL);
#endif

	if (hook->wander_sound) hook->wander_sound (self);

	ai_frame_sounds (self);	

	// are we going for our enemy?
	if (hook->goal_type != GOAL_TEMP)
	{
		task_type = task_current_type (hook->task_stack);

		switch (task_type)
		{
			case	TASK_FOLLOW:
				if (!self->goalentity->owner || hook->goal_type != GOAL_OWNER)
				{
					ai_debug_print (self, "TASK_FOLLOW in ai_follow without GOAL_OWNER\n");
					com->Error ("%s has TASK_FOLLOW in ai_follow without GOAL_OWNER\n", self->className);
					ai_set_think (self, NULL, "ai_follow");

					return;
				}

				dist = VectorDistance (self->s.origin, hook->owner->s.origin);

				if (AI_IsVisible (self, self->goalentity->owner))
				{
					///////////////////////////////////////////////////
					//	owner is visible, are we close enough to stand?
					///////////////////////////////////////////////////

					self->goalentity->s.origin = self->goalentity->owner->s.origin;
					ai_debug_print (self, "ai_follow: owner visible\n");

					if (dist < hook->follow_dist)
					{
						hook->begin_stand (self);
						return;
					}
				}
				else
				{
					//////////////////////////////////////////////////////
					//	lost sight of owner
					//////////////////////////////////////////////////////

					ai_debug_print (self, "ai_follow: owner NOT visible\n");

					//	FIXME: always follow?
					if (ai_follow_do_task (self))
						return;
				}

				if (dist < 32.0)
					ai_movetogoal (self, hook->run_speed, MOVE_BACKWARD);
				else
					ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);
				break;
			case	TASK_CHASE:
				if (!self->goalentity->owner || hook->goal_type != GOAL_ENEMY)
				{
					ai_debug_print (self, "TASK_CHASE in ai_follow without GOAL_ENEMY\n");
					com->Error ("%s has TASK_CHASE in ai_follow without GOAL_ENEMY\n", self->className);
					ai_set_think (self, NULL, "ai_follow");

					return;
				}

				self->goalentity->s.origin = self->enemy->s.origin;
				dist = ai_goal_dist (self, self->goalentity);				

				if (self->goalentity->owner->deadflag != DEAD_NO)
				{
					ai_debug_print (self, "ai_follow: enemy DEAD\n");

					//	pop off everything thru TASK_CHASE for this enemy
					ai_killed_enemy (self);

					return;
				}
				else if (AI_IsVisible (self, self->goalentity->owner))
				{
					///////////////////////////////////////////////////
					//	enemy is visible, should we follow or attack?
					///////////////////////////////////////////////////

					self->goalentity->s.origin = self->goalentity->owner->s.origin;
					ai_debug_print (self, "ai_follow: enemy visible\n");

					// if we are close enough to the enemy we want to go into
					// attack mode
					if ((dist < hook->attack_dist || (hook->dflags & DFL_JUMPATTACK && dist < hook->max_jump_dist))
						&& hook->attack_finished <= gstate->time && hook->begin_attack)
					{
						task_attack (self, self->enemy);
						ai_follow_do_task (self);
						return;
					}
					else if (!(hook->dflags & DFL_RANGEDATTACK))
					{
						//	check to see how many nodes away enemy is
						//	if more than two nodes, go to path following
						ehook = (playerHook_t *) self->enemy->userHook;
						
						MAPNODE_PTR pNode1 = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
						MAPNODE_PTR pNode2 = NODE_GetNode( hook->pNodeList->pNodeHeader, ehook->pNodeList->nCurrentNodeIndex );

						depth = node_can_path( hook->pNodeList->pNodeHeader, pNode1, pNode2, 2);
						if (!depth)
						{
							if (ai_follow_do_task (self))
								return;

						}
					}
				}
				else
				{
					//////////////////////////////////////////////////////
					//	lost sight of the enemy
					//////////////////////////////////////////////////////

					ai_debug_print (self, "ai_follow: enemy NOT visible\n");

					if (ai_should_follow (self))
					{
						if (ai_follow_do_task (self))
							return;
					}
				}

				if (dist < 32.0)
					ai_movetogoal (self, hook->run_speed, MOVE_BACKWARD);
				else
					ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);
				break;
			case	TASK_ATTACK:
				ai_set_think (self, hook->begin_attack, "ai_follow");
				return;
				break;
			case	TASK_SWITCH:
				dist = ai_goal_dist (self, self->goalentity);

				//	following an entity which does not move
				if (dist < 64.0)
				{
					ai_follow_do_task (self);

					//	FIXME: can fail on a TASK_SWITCH.  Then what?
					return;
				}
				else
					ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);
				break;
			case	TASK_PLAT:
			case	TASK_DOOR:
				//	going to node in front of func_plat or func_door
				//	to wait for obj_desired_state
				dist = ai_goal_dist (self, self->goalentity);

				if (dist < REACHED_GOAL_DIST)
				{
					ai_follow_do_task (self);

					return;
				}
				else
					ai_movetogoal (self, hook->run_speed, MOVE_FORWARD);
				break;
			case	TASK_RIDE_PLAT:
				//	going to center of func_plat so that we don't
				//	clip on overhangs while riding it
				
				//	update goalentity z to self z to compensate for plats
				//	that are already moving

				self->goalentity->s.origin.z = self->s.origin.z;
				dist = ai_goal_dist (self, self->goalentity);				
				//dist = VectorDistance (self->s.origin, self->goalentity->s.origin);				

				if (dist < 16.0)
				{
					ai_follow_do_task (self);

					return;
				}
				else
					ai_movetogoal (self, hook->run_speed / 2, MOVE_FORWARD);

				break;
		}
	}
	// we must be seeking to a temporary goal to get around an obstruction
	else
	{
		ai_debug_print (self, "ai_follow: seeking temporary goal\n");
		dist = ai_goal_dist (self, self->goalentity);
		ai_seek_temporary_goal (self, dist);
	}

	if (!(hook->ai_flags & AI_JUMP) && !(hook->ai_flags & AI_TURN))
		ai_set_think (self, ai_follow, "ai_follow");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_follow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_follow( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

//	if (task_current_type (hook->task_stack) == TASK_CHASE && hook->goal_type != GOAL_ENEMY)
//		DebugBreak ();

	//	restore the correct goal for our current task
	ai_set_task_goal (self);

	frameData_t *pSequence = NULL;
	if (self->movetype == MOVETYPE_FLY)
	{
		pSequence = FRAMES_GetSequence( self, "flya" );
		AI_StartSequence(self, pSequence);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "runa" );
		AI_StartSequence(self, pSequence);
	}
	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	////////////////////////////////////////////////////////////////
	//	determine here if monster will jump at enemy when in range
	////////////////////////////////////////////////////////////////

	if ((int) floor (rnd () * 100) + 1 <= hook->jump_chance)
		hook->dflags |= DFL_JUMPATTACK;
	else
		hook->dflags -= (hook->dflags & DFL_JUMPATTACK);

	ai_follow (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_stand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_stand( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	int				i, task_type;
	float			dist;
	taskData_t		*task_data;

	int				seq;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_stand", NULL);
#endif

	if (!ai_client_near (self))
	{
		ai_set_think (self, ai_stand, "ai_stand 1");
		return;
	}

	if (!ai_disable && hook->find_target (self))
		return;

	if (hook->wander_sound) 
		hook->wander_sound (self);

	ai_frame_sounds (self);	

	task_current (hook->task_stack, &task_data, &task_type);

	switch	(task_type)
	{
		case	TASK_FOLLOW:
			//	turn head to follow owner
//			ai_face_owner (self);

			dist = VectorDistance (hook->owner->s.origin, self->s.origin);
			if (dist > hook->follow_dist + 64.0)
			{
				ai_set_task_goal (self);
				ai_set_think (self, hook->begin_follow, "ai_stand 2");
				return;
			}
			break;
	}

	// do an ambient standing sequence
	// FIXME: won't search for targets while doing this...
	// may be a problem for long ambients, may need a special
	// transition think for this
	if (self->s.frameInfo.frameState & FRSTATE_LAST)
	{
		self->s.frameInfo.frameState -= FRSTATE_LAST;

		if (rnd () < 0.2)
		{
			if (ai_disable)
			{
				i = (int) (rnd () * 2);

				frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
				if (i == 0)
				{
					seq = SEQUENCE_STAND;
				}
				else
				{
					seq = SEQUENCE_ATAK;
					pSequence = FRAMES_GetSequence( self, "ataka" );
				}

				if (seq == SEQUENCE_STAND)
				{
					AI_StartSequence(self, pSequence);
				}
				else 
				if (seq == SEQUENCE_ATAK)
				{
					AI_StartSequence(self, pSequence);
				}
				
				return;
			}
			else
			{
				frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
				AI_StartSequence(self, pSequence);

				return;
			}
		}
	}

	ai_set_think (self, ai_stand, "ai_stand 3");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_stand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_stand( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	CVector			goal_org, ang;

	if ( self->groundEntity && self->velocity.Length() > 0 )
	{
		self->velocity.Zero();
	}

	///////////////////////////////////////////////////////////////////////////////
	//	put goal out in front so that we don't turn away from initial facing
	//	directions
	///////////////////////////////////////////////////////////////////////////////
	self->enemy = self;

	ang = self->s.angles;
	ang.SetPitch( 0.0f );
	ang.SetRoll( 0.0 );
	ang.AngleToVectors( forward, right, up );
	goal_org = self->s.origin + forward * 32.0;

	ai_set_goal (self, self, goal_org, GOAL_TEMP);

	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
	AI_StartSequence(self, pSequence);	

	ai_stand (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_resume_think
// Description:
//				goes back to the correct think routine based on th_state
//				restores correct goal according to current task
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_resume_think( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_print (self, "ai_resume_think\n");
#endif

//	ai_set_task_goal (self);

	if (hook->ai_flags & AI_CONTINUE)
		hook->continue_think (self);
	else if (hook->ai_flags & AI_SPECIAL)
		hook->special_think (self);

	else if (hook->begin_stand) 
		hook->begin_stand (self);
	else
	{
		ai_debug_print (self, "ai_resume_think: I'm screwed!\n");
		self->think = ai_think_stub;
		self->nextthink = 0;
	}

	return;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_jump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_jump( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_jump", NULL);
#endif

	ai_frame_sounds (self);	

	if (self->groundEntity || self->velocity[2] == 0)
	{
		hook->ai_flags = hook->ai_flags - AI_JUMP;

		ai_resume_think (self);
		
		return;
	}

	self->nextthink = gstate->time + hook->think_time;
	self->think = ai_jump;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_jump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_jump( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	CVector temp = forward * hook->forward_vel;
	self->velocity = temp;
	self->velocity.SetZ( hook->upward_vel );

	self->groundEntity = NULL;

	frameData_t *pSequence = FRAMES_GetSequence( self, "jumpa" );
	AI_StartSequence(self, pSequence);

	self->s.frameInfo.frameFlags = FRAME_LOOP;
	self->s.frameInfo.frameInc = 1;

	hook->ai_flags |= AI_JUMP;

	ai_jump (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_pain( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_pain", NULL);
#endif

	ai_frame_sounds (self);	

	if (self->s.frame == self->s.frameInfo.endFrame)
	{	
		if (self->flags & FL_POSSESSED)
		{
			self->think = NULL;
			self->nextthink = -1;
		}
		else
			ai_resume_think (self);

		return;
	}

	ai_set_think (self, ai_pain, "ai_pain");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_pain( userEntity_t *self, userEntity_t *other, float kick, int damage )
{
	int				new_enemy = FALSE;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	playerHook_t	*ahook;
	userEntity_t	*attacker;

	// still in a pain function from last hit?
	if (hook->pain_finished >= gstate->time || self->deadflag != DEAD_NO)
		return;

	////////////////////////////////////////////////////////////////
	//	make monsters fight one another
	////////////////////////////////////////////////////////////////

	attacker = gstate->attacker;
	if (attacker->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) && attacker != self->enemy)
	{
		ahook = (playerHook_t *) attacker->userHook;	 

		if (self->flags & FL_MONSTER)
		{
			//	attacked by a player or bot so attack back
			if (attacker->flags & (FL_CLIENT + FL_BOT))
				new_enemy = TRUE;
			//	berserk, so attack anything 
			else if (hook->dflags & DFL_BERSERK)
				new_enemy = TRUE;
			//  attacked by another monster, attack back if
			//	not of the same type
			else if (attacker->flags & FL_MONSTER && hook->type != ahook->type)
				new_enemy = TRUE;
		}
		else if (self->flags & FL_BOT)
		{
			//	bots only attack entities with opposite alignment
			if (hook->dflags & DFL_EVIL && 
				(ahook->dflags & DFL_GOOD || ahook->dflags & DFL_NEUTRAL))
				new_enemy = TRUE;
			else if (hook->dflags & DFL_GOOD &&
				(ahook->dflags & DFL_EVIL || ahook->dflags & DFL_NEUTRAL))
				new_enemy = TRUE;
			//	or anything if they are berserk
			else if (hook->dflags & DFL_BERSERK)
				new_enemy = TRUE;
		}

		if (!ahook->pNodeList)
			new_enemy = FALSE;
		else 
		if (ahook->pNodeList->nCurrentNodeIndex < 0)
			new_enemy = FALSE;

		if (new_enemy)
		{
			//	if monster currently has a TASK_ATTACK, pop it before
			//	pushing on a new TASK_CHASE
			if (task_current_type (hook->task_stack) == TASK_ATTACK)
				task_pop (&hook->task_stack);

			self->enemy = attacker;
			ai_go_to_enemy (self);

			//	make ai_resume_think go to following
//			ai_set_think (self, hook->begin_follow, "ai_begin_pain");
		}
	}

	if ((int)(rnd () * 100) > hook->pain_chance)
		return;

	ai_pain_sound (self);
   
	// huh? :)
	//- Unreferenced after this, potential to get a divide by zero error
	// i = (int)(rand() % hook->sequence_max[SEQUENCE_PAIN]);

	frameData_t *pSequence = FRAMES_GetSequence( self, "paina" );
	AI_StartSequence(self, pSequence);
	self->s.frameInfo.frameFlags = FRAME_ONCE;
	self->s.frameInfo.frameInc = 1;

	ai_set_think (self, ai_pain, "ai_begin_pain");

	hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_body_pain
// Description:
//				called when a dead body is shot
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_GibLimit(userEntity_t *self, float damage);

void ai_begin_body_pain( userEntity_t *self, userEntity_t *other, float kick, int damage )
{
	int				i;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	//	only gib a body if massive damage is done
	if (AI_GibLimit(self,damage) || (self->fragtype & FRAGTYPE_ALWAYSGIB))
	{
		//////////////////////////////////////////////////////////////
		//	major damage, so gib
		//////////////////////////////////////////////////////////////

		self->deadflag = DEAD_DEAD;
		self->s.renderfx = RF_NODRAW;
		self->pain = NULL;
		AI_StartGibFest(self, other);
	
	}
	else
	{
		self->health = 32767;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_body_die
// Description:
//	called when a dead body is killed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_body_die( userEntity_t *self, userEntity_t *other, float kick, int damage )
{
}

// ----------------------------------------------------------------------------
//
// Name:		ai_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_die( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_die", NULL);
#endif

	ai_frame_sounds (self);	

	if (self->s.frameInfo.frameState & FRSTATE_STOPPED)
	{
		if (self->flags & FL_POSSESSED)
		{
			// FIXME: put the possesser back in their own body
			// this does it right now, but it's just a hack
			camera_stop (hook->demon, false);
		}

		self->deadflag	= DEAD_DEAD;
		ai_remove_goal (self->goalentity);

		self->s.mins.Set(-16, -16, -24);
		self->s.maxs.Set(32, 32, 0);
		gstate->LinkEntity (self);

		self->pain		= ai_begin_body_pain;
		self->die		= NULL;

		self->prethink	= NULL;
		self->postthink = NULL;
		self->think		= NULL;

		return;
	}

	ai_set_think (self, ai_die, "ai_die");
}
int AI_GibLimit(userEntity_t *self, float damage);
// ----------------------------------------------------------------------------
//
// Name:		ai_begin_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point )
{
	int				i;
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	self->svflags -= (self->svflags & SVF_MONSTER);
	self->svflags |= SVF_DEADMONSTER;

	if ( 
         (AI_GibLimit(self, damage) && !(self->fragtype & FRAGTYPE_ALWAYSGIB)) || 
         //(hook->dflags & DFL_MECHANICAL) ||         // 5.9 mechanical/robot monsters can gib
         (sv_violence->value == 0)
       )
	{
		//////////////////////////////////////////////////////////////
		//	not too much damage, so just die
		//////////////////////////////////////////////////////////////
		if (self->movetype == MOVETYPE_FLY)
		{
			self->avelocity.Zero();
			hook->dflags &= ~(DFL_FACEENEMY | DFL_FACEPITCH | DFL_FACEOWNER | DFL_FACEENEMY);
			hook->dflags |= DFL_FACENONE;
		}

		self->view_ofs.Set(0, 0, -8.0);
		self->deadflag = DEAD_DYING;
		self->takedamage = DAMAGE_YES;
		self->health = 32767;
//		self->flags &= ~(FL_PUSHABLE | FL_NOPUSH);

		self->movetype = MOVETYPE_TOSS;
		self->nextthink = 0;
		gstate->LinkEntity (self);

		frameData_t *pSequence = FRAMES_GetSequence( self, "diea" );
		AI_StartSequence(self, pSequence);
		self->s.frameInfo.frameFlags = FRAME_ONCE;
		self->s.frameInfo.frameInc = 1;

		ai_die_sound (self);

		ai_die (self);
	}
	else
	{
		//////////////////////////////////////////////////////////////
		//	major damage, so gib
		//////////////////////////////////////////////////////////////

		self->deadflag = DEAD_DEAD;
		self->s.renderfx = RF_NODRAW;
		AI_StartGibFest(self, inflictor);
	}

	if ( hook->type == TYPE_BOT )
	{
		alist_remove (bot_list, self);
		alist_remove (client_bot_list, self);
	}
	else
		alist_remove (monster_list, self);

//	FIXME: need to remove entity, but what if another monster is
//	tracking it as an enemy?
	if (AI_GibLimit(self, damage) || //!(hook->dflags & DFL_MECHANICAL))
        (self->fragtype & FRAGTYPE_ALWAYSGIB))
	{
		self->pain = NULL;
		self->use = NULL;
		self->die = NULL;
		self->think = NULL;
		self->prethink = NULL;
		self->postthink = NULL;
	}
//		self->remove (self);

	if ( hook->pGoals )
	{
		GOALSTACK_Delete( hook->pGoals );
		hook->pGoals = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_turn
// Description:
//	think function called when a bot is turning to ideal_ang [YAW]
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_turn( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

#ifdef DEBUGINFO
	ai_debug_info (self, "ai_turn", NULL);
#endif

	//	support for old, think-based turning
	if (hook->dflags & DFL_FACENONE)
		com->ChangeYaw (self);

	ai_frame_sounds (self);

	ai_debug_print (self, "ideal = %.2f yaw = %.2f\n", AngleMod (self->ideal_ang [YAW]), AngleMod (self->s.angles [1]));

	if (AngleMod (self->ideal_ang [YAW]) == AngleMod (self->s.angles [YAW]))
	{
		ai_debug_print (self, "turning complete!\n");

		hook->ai_flags -= AI_TURN;
		hook->dflags = hook->dflags | DFL_TURNED;	// reset in ai_movetogoal on each successful move

		ai_resume_think (self);

		return;
	}

	ai_set_think (self, ai_turn, "ai_turn");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_turn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_turn( userEntity_t *self )
{
	playerHook_t	*hook = (playerHook_t *) self->userHook;

	hook->ai_flags |= AI_TURN;

	frameData_t *pSequence = FRAMES_GetSequence( self, "runa" );
	// if monster is not running, and turn frames exist, then play them
	if (hook->cur_seq_type != SEQUENCE_RUN && hook->cur_seq_type != SEQUENCE_FLY && 
		(self->s.frame < pSequence->first) && (self->s.frame > pSequence->last))
	{
		pSequence = FRAMES_GetSequence( self, "turna" );
		if ( pSequence )
		{
			AI_StartSequence(self, pSequence);
			self->s.frameInfo.frameFlags = FRAME_ONCE;
			self->s.frameInfo.frameInc = 1;
		}
	}

	//	slow down for turn
	self->velocity.Multiply( 0.5f );

	ai_set_think (self, ai_turn, "ai_begin_turn");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_jump_up
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_jump_up( userEntity_t *self )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;

	if (self->groundEntity || self->velocity [2] < 0 || self->s.frameInfo.frameState & FRSTATE_STOPPED)
	{
		hook->ai_flags -= AI_JUMP;

		ai_resume_think (self);
		return;
	}

	CVector ang = self->s.angles;
	ang.SetPitch( 0.0f );
	ang.SetRoll( 0.0f );
	ang.AngleToVectors(forward, right, up);

	CVector vel = forward * hook->run_speed;
	vel.SetZ( self->velocity[2] );
	self->velocity = vel;

	ai_set_think (self, ai_jump_up, "ai_jump_up");
}

// ----------------------------------------------------------------------------
//
// Name:		ai_begin_jump_up
// Description:
//	bot is trying to jump up onto a low obstruction
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_begin_jump_up( userEntity_t *self )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;

	self->velocity.Set(0, 0, hook->upward_vel);

	self->groundEntity = NULL;

	frameData_t *pSequence = FRAMES_GetSequence( self, "jumpa" );
	AI_StartSequence(self, pSequence);
	self->s.frameInfo.frameFlags = FRAME_ONCE;
	self->s.frameInfo.frameInc = 1;

	hook->ai_flags |= AI_JUMP;

	ai_jump_up (self);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_parse_epairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_parse_epairs( userEntity_t *self )
{
	if (self->className)
	{
		ai_debug_print (self, "%s\n", self->className);
	}

	if (!self->epair)
	{
		return;
	}

	for ( int i = 0; self->epair [i].key != NULL; i++ )
	{
		ai_debug_print (self, "%s %s\n", self->epair [i].key, self->epair [i].value);
		if (!stricmp (self->epair [i].key, "target"))
		{
			self->target = self->epair [i].value;
		}
		else 
		if (!stricmp (self->epair [i].key, "targetname"))
		{
			self->targetname = self->epair [i].value;
		}
	}
}

#endif 0

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_move_register_func()
{
//	gstate->RegisterFunc("ai_path_wander",ai_path_wander);
//	gstate->RegisterFunc("ai_think_stub",ai_think_stub);
//	gstate->RegisterFunc("ai_jump",ai_jump);
//	gstate->RegisterFunc("ai_begin_pain",ai_begin_pain);
//	gstate->RegisterFunc("ai_begin_body_pain",ai_begin_body_pain);
//	gstate->RegisterFunc("ai_begin_die",ai_begin_die);
//	gstate->RegisterFunc("ai_prethink",ai_prethink);
}
