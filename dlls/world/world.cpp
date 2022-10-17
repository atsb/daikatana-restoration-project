#include	<time.h>
#if _MSC_VER
#include	<crtdbg.h>
#endif

#include	"world.h"

#include	"dk_system.h"

#include	"ai.h"
#include	"ai_utils.h"
#include	"ai_move.h"
//#include	"ai_weapons.h"// SCG[1/23/00]: not used
#include	"ai_frames.h"

#include	"nodelist.h"
#include	"client.h"
#include	"bodylist.h"
#include	"actorlist.h"
#include	"light.h"
#include	"chasecam.h"
#include	"bot.h"
#include	"thinkFuncs.h"
#ifdef _DEBUG
#include	"viewthing.h"
#endif

#include	"epairs.h"

//	dll interface crap
//unix - dk_dll.h / dk_so.h
#if _MSC_VER
#include	"dk_dll.h"
#else
#include	"dk_so.h"
#endif

//	just for seeding random number generator on worldspawn
#include	"time.h"

#include	"GrphPath.h"
#include	"ai_func.h"
#include	"octree.h"
#include	"MonsterSound.h"
#include	"SequenceMap.h"
#include	"dk_io.h"
#include    "items.h"
#include	"NavTest.h"
#include	"action.h"
#include	"spawn.h"
#include	"Sidekick.h"
#include    "dumpstak.h"
#include    "PathTable.h"

//#include	"csv.h"// SCG[1/23/00]: not used

#include	"ctf.h"
#include	"coop.h"
#include	"flag.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport void worldspawn( userEntity_t *self );
DllExport void *dll_Query( void );
DllExport int dll_Entry( HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData );
DllExport void ShowBoundingBoxes ( userEntity_t *self );
DllExport void dll_LoadNodes( char *pMapName );
DllExport void dll_RegisterWorldFuncs();
DllExport void dll_EntityLoadCleanup( int nIndex );

void Inventory_Open_f( userEntity_t *self );
void Inventory_Next_f( userEntity_t *self );
void Inventory_Prev_f( userEntity_t *self );
void Inventory_Use_f( userEntity_t *self );
void Inventory_SetMode( userEntity_t *self, int nMode, qboolean bPerformAction );

///////////////////////////////////////////////////////////////////////////////
//	external globals
///////////////////////////////////////////////////////////////////////////////

CVector			forward, right, up;
CVector			zero_vector( 0, 0, 0 );
userTrace_t		trace;
trace_t			tr;
serverState_t	*gstate;
common_export_t	*com;
common_export_t *memory_com;
memory_import_t memmgr;
 
cvar_t	*maxclients;
cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*skill;
cvar_t	*p_gravity;
cvar_t	*ai_debug;
cvar_t	*sv_episode;
cvar_t	*sv_violence;
cvar_t  *sv_sidekickweapons;
cvar_t  *sv_aiwander;
cvar_t	*ai_scriptedit;
cvar_t	*ai_scriptrunning;
cvar_t	*sv_demomode;
cvar_t  *sv_frictionfactor;

cvar_t  *allow_friendly_fire;

// multiplayer options
//cvar_t	*dm_frag_limit;
//cvar_t	*dm_time_limit;
cvar_t	*dm_skill_level;
cvar_t	*dm_max_players;

// deathmatch flags
cvar_t	*dm_weapons_stay;
cvar_t	*dm_spawn_farthest;
cvar_t	*dm_allow_exiting;
cvar_t	*dm_same_map;
cvar_t	*dm_force_respawn;
cvar_t	*dm_falling_damage;
//cvar_t	*dm_instant_powerups;
cvar_t	*dm_allow_powerups;
cvar_t	*dm_allow_health;
cvar_t	*dm_allow_armor;
cvar_t	*dm_infinite_ammo;
cvar_t	*dm_fixed_fov;
cvar_t	*dm_teamplay;
cvar_t	*dm_friendly_fire;
cvar_t	*dm_footsteps;
cvar_t  *dm_allow_hook;
cvar_t  *dm_item_respawn;
cvar_t	*dm_instagib;
cvar_t	*unlimited_saves;

cvar_t  *p_cheats;
cvar_t  *gib_damage_point;
cvar_t	*gib_enable;
cvar_t	*kickview;

cvar_t  *modelname;
cvar_t  *skinname;
cvar_t  *skincolor;
cvar_t  *character;

cvar_t  *cvarBuildPathTable;
cvar_t	*dm_levellimit;

cvar_t	*sv_cinematics;


extern	int		func_explosive_count;
extern	int		func_wall_explode_count;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	unsigned long	world_thinks	= 0;	//	number of World_Prethinks called since level spawned
char	*dll_Description = "WORLD.DLL.  Client routines, entity routines, items and AI.\n";

// NSS[1/12/00]:
void SIDEKICK_RemoveItem(userEntity_t *Item);

#pragma message( "// SCG[1/11/00]: Remove for Release build" )
#ifdef _DEBUG
char	*debug_models [] = {"models/global/dv_curnode.dkm",
							"models/global/dv_lastnode.dkm",
							"models/global/dv_node.dkm",
							"models/global/dv_marker.dkm",
							"models/global/dv_jumptarg.dkm",
							"models/global/dv_goal.dkm",
							"models/global/dv_arrow.dkm",
							"models/global/dv_pathnode.dkm"};

#endif
char	*gib_models [] =   {//"models/global/e_gibtorso.dkm",
							//"models/global/e_gibshldr.dkm",
							//"models/global/e_gibleg.dkm",
							//"models/global/e_gibarm.dkm",
							//"models/global/e_gibchunk.dkm",

                            // new gibs, 2.24 dsn
                            "models/global/e_gibtorso.dkm",
							"models/global/e_gibleg.dkm",
							"models/global/e_gibfoot.dkm",
							"models/global/e_gibhand.dkm",
							"models/global/e_gibhead.dkm",
                            "models/global/e_gibchest.dkm",
                            "models/global/e_gibeye.dkm",
                            "models/global/e_gibarm.dkm",
                            "models/global/e_gibmisc.dkm",

							"models/global/e_wood1.dkm",
							"models/global/e_wood2.dkm",
							"models/global/e_glass1.dkm",
							"models/global/e_glass2.dkm",
							"models/global/e_metal1.dkm",
							"models/global/e_metal2.dkm",

							"models/global/e_gibrobot1.dkm",
							"models/global/e_gibrobot2.dkm",
							"models/global/e_gibrobot3.dkm",
							"models/global/e_gibrobot4.dkm",

							"models/global/e_rock1.dkm",
							"models/global/e_rock2.dkm",
							"models/global/e_rock3.dkm",
							"models/global/g_bone.dkm",};

///////////////////////////////////////////////////////////////////////////////
//	prototypes for hell
///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void	drop_monster (userEntity_t *self);
void	drop_frame (userEntity_t *self);
void	drop_rotate (userEntity_t *self);
void	drop_select (userEntity_t *self);
void	drop_move (userEntity_t *self);
void	drop_remove (userEntity_t *self);
void	drop_deco (userEntity_t *self);
void	drop_scale (userEntity_t *self);
void	drop_gravity (userEntity_t *self);
void	drop_set (userEntity_t *self);
void	drop_restore (userEntity_t *self);
void	drop_sprite (userEntity_t *self);
void	drop_movez (userEntity_t *self);
void	drop_help (userEntity_t *self);
void	drop_respawn (userEntity_t *self);
#endif

void	path_to_player (userEntity_t *self);

void	WorldCINInit();
void    WorldCINKill();

#ifndef _WIN32
#include <string.h>
#include<ctype.h>

char *_strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}
#endif

//#ifdef _DEBUG
#pragma message( "// SCG[1/11/00]: Remove for Release build" )
void	af_powerup (userEntity_t *self);
void	af_powerdown (userEntity_t *self);
//#endif

void	world_register_funcs();

int SinglePlayerCheat()
{
	int				nNumClients;
	userEntity_t	*ent;
	playerHook_t	*hook;
	int				nMultiplayerGame;

	if( deathmatch->value || coop->value || deathtag->value || ctf->value )
	{
		nMultiplayerGame = TRUE;
	}
	else
	{
		nMultiplayerGame = FALSE;
	}

	nNumClients = 0;
	for( int i = 1; i <= gstate->game->maxclients; i++ )
	{
		ent = &gstate->g_edicts[i];

		if( ent->inuse == 0 )
		{
			continue;
		}
		if( !( ent->flags & FL_CLIENT ) )
		{
			break;
		}
		nNumClients++;
	}

	if( gstate->GetCvar( "cheats" ) == 0 )
	{
		if( ( nNumClients > 1 ) || ( TRUE == nMultiplayerGame ) ) 
		{
			for( int i = 1; i <= gstate->game->maxclients; i++ )
			{
				ent = &gstate->g_edicts[i];

				if( ent->inuse == 0 )
				{
					continue;
				}
				if( !( ent->flags & FL_CLIENT ) )
				{
					break;
				}

				ent->movetype = MOVETYPE_WALK;
				ent->flags &= ~FL_NOTARGET;

				hook = ( playerHook_t * ) ent->userHook;
				if( hook != NULL )
				{
					hook->dflags &= ~DFL_RAMPAGE;
					hook->dflags &= ~DFL_LLAMA;
				}
			}
			return FALSE;
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	DllMain
//
//	standard DLL startup/exit routine.  We don't have to do anything in here
//	because Daikatana calls dll_Entry at appropriate times.
///////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	next_skin
//
//	player skin testing
///////////////////////////////////////////////////////////////////////////////
// SCG[7/11/00]: 
/*
void	next_skin (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	
	self->s.skinnum++;
	if (self->s.skinnum > 255)
		self->s.skinnum = 0;

	gstate->Con_Printf ("skin = %i\n", self->s.skinnum);
}
*/
///////////////////////////////////////////////////////////////////////////////
//	show_contents
///////////////////////////////////////////////////////////////////////////////

void	print_contents (char *str1, int pc)
{
	char	str [1024];

	str [0] = 0x00;

	if (pc & CONTENTS_SOLID)
		strcat (str, "SOLID ");
	else if (pc & CONTENTS_WINDOW)
		strcat (str, "WINDOW ");
	else if (pc & CONTENTS_AUX)
		strcat (str, "AUX ");
	else if (pc & CONTENTS_LAVA)
		strcat (str, "LAVA ");
	else if (pc & CONTENTS_SLIME)
		strcat (str, "SLIME ");
	else if (pc & CONTENTS_WATER)
		strcat (str, "WATER ");
	else if (pc & CONTENTS_MIST)
		strcat (str, "MIST ");
	else if (pc & CONTENTS_CLEAR)
		strcat (str, "CLEAR ");
	else if (pc & CONTENTS_NOTSOLID)
		strcat (str, "NOTSOLID ");
	else if (pc & CONTENTS_NOSHOOT)
		strcat (str, "NOSHOOT ");
	else if (pc & LAST_VISIBLE_CONTENTS)
		strcat (str, "LAST_VISIBLE ");
	else if (pc & CONTENTS_AREAPORTAL)
		strcat (str, "AREAPORTAL ");
	else if (pc & CONTENTS_PLAYERCLIP)
		strcat (str, "PLAYERCLIP ");
	else if (pc & CONTENTS_MONSTERCLIP)
		strcat (str, "MONSTERCLIP ");
	else if (pc & (CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 |
		CONTENTS_CURRENT_270 |CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN ))
		strcat (str, "CURRENT ");
	else if (pc & CONTENTS_ORIGIN)
		strcat (str, "ORIGIN ");
	else if (pc & CONTENTS_MONSTER)
		strcat (str, "MONSTER ");
	else if (pc & CONTENTS_DEADMONSTER)
		strcat (str, "DEADMONSTER ");
	else if (pc & CONTENTS_DETAIL)
		strcat (str, "DETAIL ");
	else if (pc & CONTENTS_TRANSLUCENT)
		strcat (str, "TRANSLUCENT ");
	else if (pc & CONTENTS_LADDER)
		strcat (str, "LADDER ");
	else if (pc & CONTENTS_NPCCLIP)
		strcat (str, "NPCCLIP ");

	gstate->Con_Printf ("%s%s\n", str1, str);
}

///////////////////////////////////////////////////////////////////////////////
//	show_contents
///////////////////////////////////////////////////////////////////////////////

void	show_contents (userEntity_t *self)
{
	CVector	end, ang;
	int		pc1, pc2;

	pc1 = gstate->PointContents (self->s.origin);

	end = self->s.origin;
	ang = self->s.angles;
	ang.AngleToVectors(forward, right, up);
	end = end + forward * 32.0;

	pc2 = gstate->PointContents( end );

	print_contents ("pc1 contents = ", pc1);
	print_contents ("pc2 contents = ", pc2);
}

///////////////////////////////////////////////////////////////////////////////
//	stop_entity
//
//	freezes an entity
///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void	stop_entity (userEntity_t *self)
{
	int					argc, count = 0;
	char				*name;
	char				other_name [256];
	userEntity_t		*head;
	int					freeze_all = false;

	argc = gstate->GetArgc();

	if ((argc <= 1) || (argc > 2))
	{
		gstate->bprint ("usage: freeze [classname]\n");
		return;
	}

	name = gstate->GetArgv (1);

	if (!stricmp (gstate->GetArgv (1), "all"))
	{
		freeze_all = true;
		strcpy (other_name, "ass");
	}
	else
	{
		strcpy (other_name, "monster_");
		strcat (other_name, name);
	}

	head = gstate->FirstEntity ();

	while (head)
	{
		if (head->className)
		{
			if ( (freeze_all && head->flags & (FL_MONSTER | FL_BOT)) || 
				(!stricmp (head->className, name) || !stricmp (head->className, other_name)) )
			{
				head->prethink = NULL;
				head->think = NULL;
				head->postthink = NULL;
				head->pain = NULL;
				head->die = NULL;
				head->use = NULL;

				head->movetype = MOVETYPE_NONE;
				head->solid = SOLID_NOT;

				head->s.frameInfo.frameFlags = FRAME_STATIC;

				head->velocity.Zero();

				count++;
			}
		}

		head = gstate->NextEntity (head);
	}

	if (freeze_all)
		gstate->Con_Printf ("Froze %i entities.\n", count);
	else if (count == 1)
		gstate->Con_Printf ("Froze %i %s.\n", count, name);
	else if (count > 1)
		gstate->Con_Printf ("Froze %i %ss.\n", count, name);
	else
		gstate->Con_Printf ("No %s or %s entities to freeze.\n", name, other_name);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//  path_to_player
//
//	makes camera goalentity path to player
///////////////////////////////////////////////////////////////////////////////

void	path_to_player (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	userEntity_t	*monster;

	if (!hook->camera || self->deadflag != DEAD_NO)
		return;

	monster = hook->camera->goalentity;
	if (!(monster->flags & (FL_MONSTER | FL_BOT)))
		return;

	//monster->enemy = self;

	monster->goalentity = self;
	AI_AddNewGoal( monster, GOALTYPE_MOVETOLOCATION, self->s.origin );

	//ai_set_goal (monster, monster->enemy, monster->enemy->s.origin, GOAL_ENEMY);

	//ai_go_to_enemy (monster);
}

///////////////////////////////////////////////////////////////////////////////
// massacre
///////////////////////////////////////////////////////////////////////////////

void	massacre (userEntity_t *self)
{
	userEntity_t *head;

	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			return;
		}
	}

	gstate->Con_Printf ("MASSACRE!\n");

	head = gstate->FirstEntity ();

	while (head)
	{
		if( head->flags & FL_CINEMATIC )
		{
			playerHook_t *hook = AI_GetPlayerHook( head );

			if( hook )
			{
				if( hook->szScriptName )
				{
					SPAWN_Remove( hook->szScriptName );
				}
				else
				{
					alist_remove( head );
					head->remove( head );
				}
			}
		}
//		else if ( head->flags & FL_MONSTER || head->flags & FL_BOT )
		else if ( head->flags & FL_MONSTER )
		{
			alist_remove( head );
			head->remove( head );
		}
/*
		else if( head->movetype == MOVETYPE_FLY || head->movetype == MOVETYPE_SWIM )
		{
			alist_remove( head );
			head->remove( head );
		}
*/
		head = gstate->NextEntity (head);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ShowBoundingBoxes
//
// shows the bounding box of whatever entity is direction in front of the player
///////////////////////////////////////////////////////////////////////////////

void ShowBoundingBoxes( userEntity_t *self )
{
	if (!self || !self->client)
		return;
	
	CVector	mins(-8, -8, -8);
	CVector	maxs(8, 8, 8);
	CVector	ang, end, start;

	if( self->client )
	{
		ang = self->client->v_angle;
		AngleToVectors(ang, forward, right, up);
	}

	start = self->s.origin;
	start.z += self->viewheight;
	end = start + forward * 1000.0;

	tr = gstate->TraceLine_q2( start, end, self, MASK_PLAYERSOLID );
	if (tr.ent)
	{
		if (!strcmp(tr.ent->className,"player") || !strcmp(tr.ent->className,"worldspawn"))
		{
			return;
		}
		else
		{
			if (gstate->GetCvar ("p_showboxes") == 2)
				gstate->Con_Printf ("Entity: %s at %s\n", tr.ent->className, com->vtos(tr.ent->s.origin));
			// show the box for 5 seconds
			com->DrawBoundingBox( tr.ent, 8 );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// node_move
///////////////////////////////////////////////////////////////////////////////

void	node_move (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	MAPNODE_PTR pCurrentNode = NODE_GetNode( hook->pNodeList->pNodeHeader, hook->pNodeList->nCurrentNodeIndex );
	pCurrentNode->position = self->s.origin;
	if (pCurrentNode->marker)
		pCurrentNode->marker->s.origin = self->s.origin;
}

///////////////////////////////////////////////////////////////////////////////
// node_save
///////////////////////////////////////////////////////////////////////////////

void	node_save (userEntity_t *self)
{
	if (pGroundNodes)
	{
		NODE_Write();
		//node_writelist (pGroundNodes);
	}
	else
	{
		gstate->bprint ("No nodes to write.\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
// node_save_prev
///////////////////////////////////////////////////////////////////////////////

void	node_save_prev (userEntity_t *self)
{
	if (pGroundNodes)
	{
		NODE_WritePrevious( pGroundNodes );
	}
	else
	{
		gstate->bprint ("No nodes to write.\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
// node_compute_table
///////////////////////////////////////////////////////////////////////////////

void node_compute_table( userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	NODELIST_PTR	pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR	pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	NODE_ComputePathTable( pNodeHeader );
}

///////////////////////////////////////////////////////////////////////////////
// node_write_table
///////////////////////////////////////////////////////////////////////////////

void node_write_table( userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	NODELIST_PTR	pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR	pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	NODE_WritePathTable( pNodeHeader );
}

///////////////////////////////////////////////////////////////////////////////
// node_read_table
///////////////////////////////////////////////////////////////////////////////

void node_read_table( userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	NODELIST_PTR	pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR	pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	NODE_ReadPathTable( pNodeHeader );
}

///////////////////////////////////////////////////////////////////////////////
// node_load
///////////////////////////////////////////////////////////////////////////////

/*
void	node_load (userEntity_t *self)
{
	// create and load in the new list
	NODE_Read();

	if ( !pGroundNodes )
	{
		pGroundNodes = NODE_Init( pGroundNodes );
	}
	if ( !pAirNodes )
	{
		pAirNodes = NODE_Init( pAirNodes );
	}
	if ( !pTrackNodes )
	{
		pTrackNodes = NODE_Init( pTrackNodes );
	}

	// reassign each entity's node_list to the newly created list
	userEntity_t *head = gstate->FirstEntity();
	while (head)
	{
		if (head->flags & (FL_CLIENT + FL_BOT + FL_MONSTER))
		{
//			gstate->Con_Printf ("------------------------------------\nresetting nodes for %s\n", head->className);

			AI_InitNodeList( head );
		}

		head = gstate->NextEntity(head);
	}

//	hook = AI_GetPlayerHook( self );
//	gstate->Con_Printf ("self at node #%i\n", hook->node_list->cur_node->number);
}
*/
//NSS[11/10/99]:Testing
bool node_read_octree2( char *FName );
OCTREE_PTR OCTREE_Construct();
OCTREE_PTR OCTREE_Destruct( OCTREE_PTR pTree );
void dll_LoadNodes( char *pMapName )
{
	// SCG[11/3/99]: Set the node filename
	NODE_SetNodeFileName( pMapName );

	// SCG[11/3/99]: Load the nodes
	NODE_Read();
	if ( !pGroundNodes )
	{
		pGroundNodes = NODE_Init( pGroundNodes );
	}
	if ( !pAirNodes )
	{
		pAirNodes = NODE_Init( pAirNodes );
	}
	if ( !pTrackNodes )
	{
		pTrackNodes = NODE_Init( pTrackNodes );
	}

	//NSS[11/10/99]:Check for exisitng octree if so kill it.
	if ( pPathOctree )
	{
		pPathOctree = OCTREE_Destruct( pPathOctree );
        pPathOctree = NULL;
    }
	//NSS[11/10/99]:Construct a new octree
	pPathOctree = OCTREE_Construct();

	//NSS[11/12/99]:Load the octree if it exists
	node_read_octree2(pMapName);
}

void dll_RegisterWorldFuncs()
{
	world_register_funcs();
}

void restore_timeout ( userEntity_t *self )
{
	char buffer[64];

	//unix - use ansi call instead
	//itoa((int)self->delay, buffer, 10 );
	sprintf (buffer, "%d", (int)self->delay);
	gstate->SetCvar( "timeout", buffer );	

	self->remove (self);
}

void node_build_octree( userEntity_t *self )
{
	userEntity_t *temp;

	temp = gstate->SpawnEntity ();
	temp->nextthink = gstate->time + 0.2;
	temp->think = restore_timeout;

	temp->delay  = gstate->GetCvar( "timeout" );
	gstate->SetCvar( "timeout", "1000" );

	if ( pPathOctree )
	{
		OCTREE_BuildOctree( pPathOctree );
	}
}

void node_save_octree( userEntity_t *self )
{
	if ( pPathOctree )
	{
		FILE	*f;
		char	base_name[128];

		///////////////////////////////////////////
		//	get the name of the current map's wad
		///////////////////////////////////////////

		strcpy( base_name, gstate->basedir );
		strcat( base_name, "/maps/nodes/" );
		strcat( base_name, gstate->mapName );
		strcat( base_name, ".oct" );

		gstate->Con_Dprintf ("mapname = %s\n", gstate->mapName);

		if ( !(f = fopen (base_name, "wb")) )
		{
			gstate->Con_Dprintf( "Unable to create octree file %s.\n", base_name );
			return;
		}

		OCTREE_WriteData( pPathOctree, f );

		fclose (f);

		gstate->Con_Dprintf( "Wrote octree to %s.\n", base_name );
	}
}

void node_read_octree( userEntity_t *self )
{
	if ( pPathOctree )
	{
		FILE	*f;
		char	base_name[128];

		///////////////////////////////////////////
		//	get the name of the current map's wad
		///////////////////////////////////////////

		strcpy( base_name, gstate->basedir );
		strcat( base_name, "/maps/nodes/" );
		strcat( base_name, gstate->mapName );
		strcat( base_name, ".oct" );

		gstate->Con_Dprintf ("mapname = %s\n", gstate->mapName);

		if ( !(f = fopen (base_name, "rb")) )
		{
			gstate->Con_Dprintf( "Unable to open octree file %s.\n", base_name );
			return;
		}

		OCTREE_ReadData( pPathOctree, f );

		fclose (f);
	}
}


bool node_read_octree2( char *FName )
{
	FILE	*f;
	char	base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, FName );
	strcat( base_name, ".oct" );

	gstate->Con_Dprintf ("mapname = %s\n", FName);

	if ( !(f = fopen (base_name, "rb")) )
	{
		gstate->Con_Dprintf( "Unable to open octree file %s.\n", base_name );
		return FALSE;
	}

	OCTREE_ReadData( pPathOctree, f );

	fclose (f);
	return TRUE;
}

#ifdef _DEBUG
void train_test( userEntity_t *self )
{
	AI_TestTrain();
}

void takecover_test( userEntity_t *self )
{
	AI_TestTakeCover();
}

void door_test( userEntity_t *self )
{
	AI_TestDoor();
}

void ladder_test( userEntity_t *self )
{
	AI_TestLadder();
}

void boar_test( userEntity_t *self )
{
	AI_TestBoar();
}
#endif //_DEBUG

void node_start( userEntity_t *self )
{
	NODE_EnableLayingNodes();
}

void node_end( userEntity_t *self )
{
	NODE_DisableLayingNodes();
}

void node_place( userEntity_t *self )
{
	node_lay( self );
}

///////////////////////////////////////////////////////////////////////////////
// pathto
///////////////////////////////////////////////////////////////////////////////

void	pathto (userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int			 argc;

	argc = gstate->GetArgc();

	if ((argc <= 1) || (argc > 2))
	{
		gstate->bprint ("usage: pathto [classname]\n");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	spawn
///////////////////////////////////////////////////////////////////////////////
void spawn( userEntity_t *self )
{
	int nNumArgs = gstate->GetArgc();

	// NSS[2/2/00]:No spawny spawn in any deathmatch mode.  No matter what.
	if ( !(coop->value  || deathmatch->value) )
	{
		if ( nNumArgs == 1 )
		{
			SPAWN_AI( self, NULL );
		}
		else
		if ( nNumArgs == 2 )
		{
			SPAWN_AI( self, gstate->GetArgv(1) );
		}
		else
		{
			com->Warning( "Usage: spawn [arg1]." );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	bot_spawn_bot
///////////////////////////////////////////////////////////////////////////////
void bot_spawn_bot (userEntity_t *self)
{
	bot_spawn (self);
}

///////////////////////////////////////////////////////////////////////////////
// navigation of map test
///////////////////////////////////////////////////////////////////////////////
/*
void nav_test( userEntity_t *self )
{
	int nNumArgs = gstate->GetArgc();
	if ( nNumArgs == 1 )
	{
		NAVTEST_Start( self, NULL );
	}
	else
	if ( nNumArgs == 2 )
	{
		NAVTEST_Start( self, gstate->GetArgv(1) );
	}
	else
	{
		com->Warning( "Usage: nav_test [arg1]." );
	}
}
*/

/*
void dump_stack( userEntity_t *self )
{
    DumpStack( AFX_STACK_DUMP_TARGET_CLIPBOARD );
}
*/

void script( userEntity_t *self )
{
	int nNumArgs = gstate->GetArgc();

	if ( nNumArgs < 2 )
	{
		gstate->bprint( "Usage: script [scriptname]\n");
		return;
	}

    AI_AddScriptActionGoal( self, gstate->GetArgv(1) );
}

/*
void sidekick_attack( userEntity_t *self )
{
    SIDEKICK_TestAttack( self );
}

void sidekick_attack_player( userEntity_t *self )
{
    SIDEKICK_TestAttackPlayer( self );
}

void superfly_start_carry_mikiko( userEntity_t *self )
{
    SIDEKICK_TestStartCarryMikiko( self );
}

void superfly_stop_carry_mikiko( userEntity_t *self )
{
    SIDEKICK_TestStopCarryMikiko( self );
}
*/

#ifdef _DEBUG
void pathcorner_toggle( userEntity_t *self )
{
    pathcorner_Toggle( self );
}
#endif

void sidekick_god( userEntity_t *self )
{
	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			return;
		}
	}

    SIDEKICK_ToggleGodMode();
}

///////////////////////////////////////////////////////////////////////////////
//	next_frame
///////////////////////////////////////////////////////////////////////////////

/*
void	next_frame (userEntity_t *self)
{
	userEntity_t *torso;

	torso = self->hchild->hchild;
	torso->s.frame++;
	if (torso->s.frame > 19)
		torso->s.frame = 0;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	next_frame_legs
///////////////////////////////////////////////////////////////////////////////
/*
void	next_frame_legs (userEntity_t *self)
{
	userEntity_t *legs;

	legs = self->hchild;
	legs->s.frame++;
	if (legs->s.frame > 2)
		legs->s.frame = 0;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	next_weapon
///////////////////////////////////////////////////////////////////////////////
/*
void	next_weapon (userEntity_t *self)
{
	userEntity_t *weapon;

	weapon = self->hchild->hchild->hchild->hchild;
	if (!stricmp (weapon->modelName, "models/global/hw_katana.mdl"))
		gstate->SetModel (weapon, "models/global/hw_pistol.dkm");
	else
		gstate->SetModel (weapon, "models/global/hw_katana.dkm");
}
*/
///////////////////////////////////////////////////////////////////////////////
//	ledge
///////////////////////////////////////////////////////////////////////////////

void	ledge (userEntity_t *self)
{
	CVector			temp;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	temp = self->s.angles;
	temp.x = 0;
	AngleToVectors( temp, forward, right, up );
	hook->max_jump_dist = 256;

//	ai_find_ledge_list	(self, 32.0);

	ai_jump_obstructed (self, 240, 270.0);
}

///////////////////////////////////////////////////////////////////////////////
//	facing_test - tests calculations for determining position of entities
//                in relation to the player
///////////////////////////////////////////////////////////////////////////////
/*
void facing_test( userEntity_t *self )
{
	userEntity_t	*ent = NULL;
	CVector			angles, entVector, forward;
	float			dotProduct = 0.0;

	for ( ent = gstate->FirstEntity(); ent; ent = gstate->NextEntity(ent) )
	{
		if (!ent->inuse || !ent->className || ent == self)
		{
			continue;
		}

		if ( ent->targetname && !stricmp(ent->targetname, "facing_target") )
		{
			entVector = ent->s.origin - self->s.origin;
			
			angles = self->s.angles;
			AngleToVectors(angles, forward);

			dotProduct = DotProduct( forward, entVector );
			if (dotProduct < 0)
			{
				gstate->Con_Dprintf( "facing_target is BEHIND you\n" );
			}
			else
			{
				gstate->Con_Dprintf( "facing_target is IN FRONT of you\n" );
			}
			return;	
		}
	}
}
*/
///////////////////////////////////////////////////////////////////////////////
// player_get_pos
///////////////////////////////////////////////////////////////////////////////

void player_get_pos( userEntity_t *self )
{
	if (self)
	{
		gstate->Con_Printf( "### Player Position: %s  Angles: %s ###\n", com->vtos(self->s.origin), com->vtos(self->s.angles) );
	}
}

///////////////////////////////////////////////////////////////////////////////
// player_set_pos
///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void player_set_pos( userEntity_t *self )
{
	if (gstate->GetArgc () != 4)
	{
		gstate->Con_Printf ("  USAGE: set_pos [x y z]\n");
		return;
	}
	
	CVector pos;

	pos.x = atof (gstate->GetArgv (1));
	pos.y = atof (gstate->GetArgv (2));
	pos.z = atof (gstate->GetArgv (3));

	if (self)
	{
		self->s.origin = pos;
		self->s.old_origin = pos;
	}
}

///////////////////////////////////////////////////////////////////////////////
// player_set_angles
///////////////////////////////////////////////////////////////////////////////

void player_set_angles( userEntity_t *self )
{
	if (gstate->GetArgc () != 4)
	{
		gstate->Con_Printf ("  USAGE: set_angles [pitch yaw roll]\n");
		return;
	}
	
	CVector ang;

	ang.x = atof (gstate->GetArgv (1));
	ang.y = atof (gstate->GetArgv (2));
	ang.z = atof (gstate->GetArgv (3));

	if (self)
	{
		gstate->SetClientAngles (self, ang);
		self->flags |= FL_FIXANGLES;
	}
}
#endif


///////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void toggle_sound(struct edict_s *)
{
	AI_ToggleSound();
}
#endif
///////////////////////////////////////////////////////////////////////////////
//	show_terrain
///////////////////////////////////////////////////////////////////////////////

void	show_terrain (userEntity_t *self)
{
	CVector			temp;
//	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	self->flags |= FL_DEBUG;
	if( self->client )
	{
		temp = self->client->v_angle;
	}
	temp.x = temp.z = 0;

	temp.AngleToVectors(forward, right, up);

	ai_terrain_type (self, forward, 32.0);
	self->flags &= ~FL_DEBUG;
}


/*
void crash_test(userEntity_t *self)
// crash current user, testing purposes only
{
   int i, x = 0;

   if (gstate->GetCvar ("developer"))
   {
     i = 1 / x; // div by 0 crash   
   }
}
*/

/*
void assert_test(userEntity_t *self)
// assert current user, testing purposes only
{
   if (gstate->GetCvar ("developer"))
   {
     _ASSERTE(0); // force assert
   }
}
*/


/*
void force_hps (userEntity_t *self)
// force hit points to any level between -10 and 999
{
	int				hps;

  	
    if (deathmatch->value)
      return;

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: force_hps [# of hit points]\n");
		return;
	}
	
	hps = atoi (gstate->GetArgv (1));

    // boundary checking
    if (hps < -10) 
      hps = -10;
    else
      if (hps > 999)
        hps = 999;

    self->health = hps; // set hit points
}

void force_xps (userEntity_t *self)
// force experience points to any value
{
	int xps;

  	
  if (deathmatch->value)
    return;

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: force_xps [# of experience points]\n");
		return;
	}
	
	xps = atoi (gstate->GetArgv (1));

	gstate->Con_Printf("current xps = %d,  new xps = %d\n",self->record.exp, xps);

  // boundary checking
  if (xps < 0) 
    xps = 0;

  self->record.exp = xps; // set experience points

	recalc_level(self); // recalculate experience level
}
*/



///////////////////////////////////////////////////////////////////////////////
//	cam_force_attack
///////////////////////////////////////////////////////////////////////////////

void	cam_force_attack (userEntity_t *self)
{
//	playerHook_t	*hook, *phook = AI_GetPlayerHook( self );
	playerHook_t	*phook = AI_GetPlayerHook( self );// SCG[1/23/00]: hook not used
	userEntity_t	*camera;
	cameraHook_t	*camHook;
	
	if (phook->camera)
	{
		camera = phook->camera;
		camHook = (cameraHook_t *) camera->userHook;

		if (!camHook->debug_target)
			return;

//		hook = AI_GetPlayerHook( camHook->debug_target );// SCG[1/23/00]: not used

		//ai_set_think (camHook->debug_target, hook->begin_attack, "cam_force_attack");
	}
}

///////////////////////////////////////////////////////////////////////////////
//	cam_force_ambient
///////////////////////////////////////////////////////////////////////////////

void	cam_force_ambient (userEntity_t *self)
{

//	playerHook_t	*hook, *phook = AI_GetPlayerHook( self );// SCG[1/23/00]: hook not used
	playerHook_t	*phook = AI_GetPlayerHook( self );
	userEntity_t	*camera;
	cameraHook_t	*camHook;

	if (phook->camera)
	{
		camera = phook->camera;
		camHook = (cameraHook_t *) camera->userHook;
	
		if (!camHook->debug_target)
			return;

//		hook = AI_GetPlayerHook( camHook->debug_target );// SCG[1/23/00]: not used

		gstate->Con_Printf ("forcing ambient\n");
		frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
		AI_StartSequence( camHook->debug_target, pSequence );
	//	ai_init_sequence (cam_debug_target, hook->frames->stand [1], SEQUENCE_STAND, 1);
	//	self->s.frameInfo.frameFlags = FRAME_ONCE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	cam_superfly (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	userEntity_t	*sfly;

	if (hook->camera)
	{
		//	turn off camera
		camera_stop (self, false);
	}
	else
	{
		sfly = com->FindEntity ("Superfly");

		if (!sfly)
			return;

		camera_start (self, false);
		camera_set_target (hook->camera, sfly);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
void	hiro_spawn (userEntity_t *self)
{
	hiro_SpawnScriptActor (self, self->s.origin, self->s.angles);
}
*/

///////////////////////////////////////////////////////////////////////////////
//	Kill
//
//	kills the user who issued the command
///////////////////////////////////////////////////////////////////////////////
void	Kill (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	CVector		ang;

	ang = self->s.angles;
	AngleToVectors(ang, forward, right, up);

	// take off llama mode so we don't get stuck in a loop
	hook->dflags &= ~DFL_LLAMA;

	//	give a nice big gib for mommie!!
	com->Damage (self, self, self, self->s.origin, up, 10000, DAMAGE_NONE);
}


/*-----------------------------------------------------------------------------
	DLL initialization/deinitialization functions
-------------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
//	world_InitCVars
///////////////////////////////////////////////////////////////////////////////

void	world_InitCVars (void)
{
	deathmatch			= gstate->cvar ("deathmatch", "0", CVAR_SERVERINFO|CVAR_LATCH);
	//NSS[11/7/99]: Added kickview flag to turn on or off
	kickview			= gstate->cvar ("kickview", "1", CVAR_SERVERINFO|CVAR_LATCH);
	coop				= gstate->cvar ("coop", "0", CVAR_SERVERINFO|CVAR_LATCH);
	skill				= gstate->cvar ("skill", "100", CVAR_SERVERINFO|CVAR_LATCH);
	maxclients			= gstate->cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	p_gravity			= gstate->cvar ("p_gravity", "800", CVAR_SERVERINFO | CVAR_LATCH);
	ai_debug			= gstate->cvar ("ai_debug", "0", CVAR_ARCHIVE);
	sv_episode			= gstate->cvar ("sv_episode", "0", CVAR_SERVERINFO | CVAR_LATCH);
	sv_violence			= gstate->cvar ("sv_violence", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE);
	
	sv_sidekickweapons  = gstate->cvar ("sv_sidekickweapons", "0", CVAR_SERVERINFO | CVAR_LATCH);
	sv_aiwander			= gstate->cvar ("sv_aiwander", "0", CVAR_SERVERINFO);


	sv_demomode			= gstate->cvar ("sv_demomode", "1", CVAR_SERVERINFO | CVAR_LATCH);

	ai_scriptedit		= gstate->cvar ("ai_scriptedit", "0", 0);
	ai_scriptrunning	= gstate->cvar ("ai_scriptrunning", "0", 0);

//	dm_frag_limit		= gstate->cvar ("dm_frag_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
//	dm_time_limit		= gstate->cvar ("dm_time_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_skill_level		= gstate->cvar ("dm_skill_level", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_max_players		= gstate->cvar ("dm_max_players", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_weapons_stay		= gstate->cvar ("dm_weapons_stay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_spawn_farthest	= gstate->cvar ("dm_spawn_farthest", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_exiting	= gstate->cvar ("dm_allow_exiting", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_same_map			= gstate->cvar ("dm_same_map", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_force_respawn	= gstate->cvar ("dm_force_respawn", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_falling_damage	= gstate->cvar ("dm_falling_damage", "1", CVAR_SERVERINFO|CVAR_LATCH);
//	dm_instant_powerups	= gstate->cvar ("dm_instant_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_powerups	= gstate->cvar ("dm_allow_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_health		= gstate->cvar ("dm_allow_health", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_armor		= gstate->cvar ("dm_allow_armor", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_infinite_ammo	= gstate->cvar ("dm_infinite_ammo", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_fixed_fov		= gstate->cvar ("dm_fixed_fov", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_teamplay			= gstate->cvar ("dm_teamplay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_friendly_fire	= gstate->cvar ("dm_friendly_fire", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_footsteps        = gstate->cvar ("dm_footsteps", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_hook       = gstate->cvar ("dm_allow_hook", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_item_respawn     = gstate->cvar ("dm_item_respawn", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_levellimit		= gstate->cvar ("dm_levellimit","0",CVAR_SERVERINFO|CVAR_LATCH);
	dm_instagib			= gstate->cvar ("dm_instagib", "0", CVAR_SERVERINFO|CVAR_LATCH);
	unlimited_saves		= gstate->cvar ("unlimited_saves", "0", CVAR_ARCHIVE);
    
    gib_damage_point    = gstate->cvar ("gib_damage", "0.30", CVAR_ARCHIVE );
    gib_enable			= gstate->cvar ("gib_enable", "1", CVAR_ARCHIVE );
    p_cheats            = gstate->cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);

    sv_cinematics		= gstate->cvar ("sv_cinematics", "1", CVAR_SERVERINFO|CVAR_LATCH);
    
	allow_friendly_fire = gstate->cvar( "allow_friendly_fire", "1", CVAR_ARCHIVE );

    cvarBuildPathTable = gstate->cvar( "BuildPathTable", "1", 0 );    
}

///////////////////////////////////////////////////////////////////////////////
//	client_model
//
//	changes client model 
///////////////////////////////////////////////////////////////////////////////

void	client_model (userEntity_t *self)
{
	playerHook_t	*hook;
	int				argc;
	char			*name;

	argc = gstate->GetArgc();

	if ((argc <= 1) || (argc > 2))
	{
		gstate->bprint ("usage: client_model [model prefix]\nValid prefixes are hiro, sfly, and miko");
		return;
	}

	name = gstate->GetArgv (1);

	hook = AI_GetPlayerHook( self );

	if (!stricmp (name, "hiro"))
		hook->type = TYPE_CLIENT;
	else if (!stricmp (name, "sfly"))
		hook->type = TYPE_SUPERFLY;
	else if (!stricmp (name, "miko"))
		hook->type = TYPE_MIKIKO;

#if 0
	//	destroy all our hierarchical parts
	hr_remove_parts(self, true);

	hr_init_models (self);
	//	tell the client what models not to draw for this player
//	com->SetClientModels (self, hook->head, hook->torso, hook->legs, hook->weapon);
#else
	Client_InitClientModel ( self );
	//&&&FIXME - fix this hack... it's sending down 4 model index values...
//	com->SetClientModels (self, self, self, self, self);
#endif


}

void DecrementMonsterCount()
{
	gstate->numMonsters--;
}

void IncrementMonsterCount()
{
	gstate->numMonsters++;
}

#ifdef _DEBUG
void UseTarget_f( userEntity_t *self )
{
	userEntity_t	*pEntity = NULL;
	char			*pTarget = NULL;

	if( gstate->GetArgc() != 2 )
	{
		gstate->bprint ("usage: use_target [targetname]\n");
		return;
	}

	pTarget = strdup( gstate->GetArgv( 1 ) );

	for( pEntity = gstate->FirstEntity (); pEntity; pEntity = gstate->NextEntity( pEntity ) )
	{
		if( pEntity->targetname != NULL )
		{
			if( strcmp( pTarget, pEntity->targetname ) == 0 )
			{
				if( pEntity->use != NULL )
				{
					pEntity->use( pEntity, self, self );
				}
			}
		}
	}

	if( pTarget != NULL )
	{
		free( pTarget );
	}
}

void UseUniqueID_f( userEntity_t *self )
{
	userEntity_t	*pEntity = NULL;
	char			*pTarget = NULL;

	if( gstate->GetArgc() != 2 )
	{
		gstate->bprint ("usage: use_uniqueid [uniqueid]\n");
		return;
	}

	pTarget = strdup( gstate->GetArgv( 1 ) );
	if( pTarget != NULL )
	{
		pEntity = UNIQUEID_Lookup( pTarget );
		if( pEntity != NULL )
		{
			if( pEntity->use != NULL )
			{
				pEntity->use( pEntity, self, self );
			}
		}
		else
		{
			gstate->bprint ("entity not found\n");
		}
	}

	if( pTarget != NULL )
	{
		free( pTarget );
	}
}

typedef	(*func_t)(edict_t *);
void SpawnItem_f( userEntity_t *self )
{
	char *pItemName = NULL;

	if( gstate->GetArgc() != 2 )
	{
		gstate->bprint ("usage: spawn_item [classname]\n");
		return;
	}

	pItemName = strdup( gstate->GetArgv( 1 ) );

	if( strncmp( pItemName, "item_", 5 ) == 0 )
	{
		func_t func = ( func_t ) com->FindSpawnFunction( pItemName );
		if( func != NULL )
		{
			userEntity_t	*pItem = gstate->SpawnEntity ();
			pItem->s.origin = self->s.origin;
			pItem->className = pItemName;
			if( pItem != NULL )
			{
				func( pItem );
			}
			pItem->touch( pItem, self, NULL, NULL );
		}
	}

	if( pItemName != NULL )
	{
		free( pItemName );
	}
}
#endif _DEBUG

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerLoad
//
//	called when Daikatana first loads this DLL
//
//	NO ROUTINES FROM COMMON ARE AVAILABLE HERE!!
///////////////////////////////////////////////////////////////////////////////
//void	Client_FreeWeapon (userEntity_t *self);// SCG[1/23/00]: not used
void	dll_ServerLoad (serverState_t *state)
{
	gstate = state;
	memory_com = com = (common_export_t *) gstate->common_exports;	
	memmgr.X_Malloc = gstate->X_Malloc;
	memmgr.X_Free = gstate->X_Free;

	gstate->Con_Dprintf ("* Loaded WORLD.DLL, %i.\n", sizeof (playerHook_t));

	// cheat/debugging commands
	gstate->AddCommand ("god", Client_LlamaCheat);
	gstate->AddCommand ("rampage", Client_RampageCheat);
	gstate->AddCommand ("health", Client_HealthCheat);
	gstate->AddCommand ("showstats", Client_ShowStats);
	gstate->AddCommand ("notarget", Client_NoTarget);
	gstate->AddCommand( "noclip", Client_NoClip);
//	gstate->AddCommand ("crash",  crash_test); 
//	gstate->AddCommand ("assert", assert_test);

    gstate->AddCommand ("use", Client_Use);  // this ONLY covers the single-shot use. "+use" works for repeating (ie: hosportals)

	gstate->AddCommand ("kill", Kill);
#ifdef _DEBUG
	gstate->AddCommand ("freeze", stop_entity);
#endif
	gstate->AddCommand ("massacre", massacre);
	gstate->AddCommand ("boost", af_powerup);
	gstate->AddCommand ("breakme", af_powerdown);
	
	//	camera commands
	gstate->AddCommand ("cam_prevmon", camera_prev_monster);
	gstate->AddCommand ("cam_nextmon", camera_next_monster);
	gstate->AddCommand ("cam_nextsidekick", camera_next_sidekick);
	gstate->AddCommand ("cam_toggle", camera_toggle);
	gstate->AddCommand ("cam_debug", camera_debug);
	gstate->AddCommand ("cam_toggle_possession", camera_toggle_possession);
	gstate->AddCommand ("cam_toggle_gravity", camera_toggle_gravity);
	gstate->AddCommand ("cam_toggle_debug", camera_toggle_debug_target);
	gstate->AddCommand ("cam_set_debug", camera_set_debug_target);
	gstate->AddCommand ("cam_kill", camera_kill_target);
	gstate->AddCommand ("cam_lock", camera_lock);
	//gstate->AddCommand ("cam_force_enemy", camera_force_enemy);
	gstate->AddCommand ("cam_force_attack", cam_force_attack);
	gstate->AddCommand ("cam_force_ambient", cam_force_ambient);
	gstate->AddCommand ("cam_superfly", cam_superfly);

	//gstate->AddCommand ("skat_show", skat_show_list);

	//	node system commands
	gstate->AddCommand ("node_start", node_start );
	gstate->AddCommand ("node_end", node_end );
	gstate->AddCommand ("node_place", node_place );
	gstate->AddCommand ("node_links", node_show_links);
#ifdef _DEBUG
	gstate->AddCommand ("node_toggle", node_toggle_nodes);
#endif
	gstate->AddCommand ("node_save", node_save);
	gstate->AddCommand ("node_save_prev", node_save_prev);
//	gstate->AddCommand ("node_load", node_load);
	gstate->AddCommand ("node_move", node_move);
	gstate->AddCommand ("node_compute_table", node_compute_table );
	gstate->AddCommand ("node_write_table", node_write_table );
	gstate->AddCommand ("node_read_table", node_read_table );
	gstate->AddCommand ("node_build_octree", node_build_octree );
	gstate->AddCommand ("node_save_octree", node_save_octree );
	gstate->AddCommand ("node_read_octree", node_read_octree );

	//	hierarchical testing commands
//	gstate->AddCommand ("next_frame", next_frame);
//	gstate->AddCommand ("next_frame_legs", next_frame_legs);
//	gstate->AddCommand ("next_weapon", next_weapon);

	gstate->AddCommand ("ai_ledge", ledge);
	gstate->AddCommand ("ai_terrain_type", show_terrain);

	//	Viewthing commands, some duplicated to remain compatible with Q1 viewthing commands
#ifdef _DEBUG
	gstate->AddCommand ("vs", vt_ViewSequence);
	gstate->AddCommand ("viewseq", vt_ViewSequence);
	gstate->AddCommand ("vstop", vt_Stop);
	gstate->AddCommand ("vstart",vt_Start);
	gstate->AddCommand ("vdelay",vt_Delay);
	gstate->AddCommand ("vforward",vt_Forward);
	gstate->AddCommand ("vbackward",vt_Backward);
	gstate->AddCommand ("vf", vt_ViewFrame);
	gstate->AddCommand ("viewframe", vt_ViewFrame);
	gstate->AddCommand ("vl", vt_ListSequences);
	gstate->AddCommand ("listseq", vt_ListSequences);
	gstate->AddCommand ("vm", vt_ViewModel);
	gstate->AddCommand ("vmm", vt_ViewModelOnly);
	gstate->AddCommand ("viewmodel", vt_ViewModel);
	gstate->AddCommand ("vr", vt_Rotate);
	gstate->AddCommand ("vscale", vt_ScaleModel);
	gstate->AddCommand ("vt", vt_SetTranslucent);
#endif
	//	drop_ commands
#ifdef _DEBUG
	gstate->AddCommand ("drop_monster", drop_monster);
	gstate->AddCommand ("drop_rotate", drop_rotate);
	gstate->AddCommand ("drop_select", drop_select);
	gstate->AddCommand ("drop_move", drop_move);
	gstate->AddCommand ("drop_frame", drop_frame);
	gstate->AddCommand ("drop_remove", drop_remove);
	gstate->AddCommand ("drop_deco", drop_deco);
	gstate->AddCommand ("drop_scale", drop_scale);
	gstate->AddCommand ("drop_gravity", drop_gravity);
	gstate->AddCommand ("drop_set", drop_set);
	gstate->AddCommand ("drop_restore", drop_restore);
	gstate->AddCommand ("drop_sprite", drop_sprite);
	gstate->AddCommand ("drop_movez", drop_movez);
	gstate->AddCommand ("drop_help", drop_help);
	gstate->AddCommand ("drop_respawn", drop_respawn);
#endif

	//	debugging crap
	gstate->AddCommand ("path_to_player", path_to_player);
#ifdef _DEBUG
	gstate->AddCommand ("show_contents", show_contents);
	gstate->AddCommand ("spawn_sf", SIDEKICK_SpawnSuperfly2);
	gstate->AddCommand ("spawn_mk", SIDEKICK_SpawnMikiko2);
	gstate->AddCommand ("spawn_mkfly", SIDEKICK_SpawnMikikoFly2);

	gstate->AddCommand ("teleport_sf",		SIDEKICK_TeleportSuperfly);			// NSS[2/15/00]:
	gstate->AddCommand ("teleport_mk",		SIDEKICK_TeleportMikiko);			// NSS[2/15/00]:
	gstate->AddCommand ("teleport_mkfly",	SIDEKICK_TeleportMikikoFly);		// NSS[2/15/00]:
#endif
	gstate->AddCommand ("stats_sf",			SIDEKICK_Stats_SF);					// NSS[3/6/00]:
	gstate->AddCommand ("stats_mk",			SIDEKICK_Stats_MK);					// NSS[3/6/00]:


	//	client commands
	gstate->AddCommand ("client_model", client_model);
// SCG[7/11/00]: 	gstate->AddCommand ("next_skin", next_skin);
	
	gstate->AddCommand( "Superfly", CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "Mikiko",   CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Yes",      CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "No",       CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Stay",     CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Follow",   CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Attack",   CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "NoAttack", CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "ComeHere", CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Pickup",   CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "sidekick_stay",     CLIENT_IssueCommandToSidekicks );
//	gstate->AddCommand( "Follow",   CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "sidekick_attack",   CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "sidekick_backoff", CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "sidekick_come", CLIENT_IssueCommandToSidekicks );
	gstate->AddCommand( "sidekick_get",   CLIENT_IssueCommandToSidekicks );

#ifdef _DEBUG
	gstate->AddCommand( "use_target",   UseTarget_f );
	gstate->AddCommand( "use_uniqueid",   UseUniqueID_f );
	gstate->AddCommand( "spawn_item",   SpawnItem_f );
#endif _DEBUG

	gstate->AddCommand( "sidekick_swap", SIDEKICK_SwapCommanding );
	gstate->AddCommand( "item_savegem_use", item_savegem_use );

	// SCG[12/13/99]: New inventory commands
	gstate->AddCommand( "hud_cycle", Inventory_Open_f );
	gstate->AddCommand( "hud_next", Inventory_Next_f );
	gstate->AddCommand( "hud_prev", Inventory_Prev_f );
	gstate->AddCommand( "hud_use", Inventory_Use_f );

	// test train commands
#ifdef _DEBUG
	gstate->AddCommand( "TestTrain", train_test );
	gstate->AddCommand( "TakeCover", takecover_test );
	gstate->AddCommand( "TestDoor", door_test );
	gstate->AddCommand( "TestLadder", ladder_test );
	gstate->AddCommand( "TestBoar", boar_test );
	gstate->AddCommand( "toggle_sound", toggle_sound );
	gstate->AddCommand( "setpos", player_set_pos );
	gstate->AddCommand( "setangles", player_set_angles );
#endif _DEBUG

//	gstate->AddCommand( "facing_test", facing_test );
//	gstate->AddCommand( "force_hps", force_hps );        // set hitpoints manually   1.26 dsn
//	gstate->AddCommand( "force_xps", force_xps );        // set experience points manually 

	gstate->AddCommand( "getpos", player_get_pos );

	// bot related commands
	gstate->AddCommand( "spawn" , spawn );
	gstate->AddCommand ("ai_bot", bot_spawn_bot);

	// navigation test
//	gstate->AddCommand( "nav_test", nav_test );

//    gstate->AddCommand( "dumpstack", dump_stack );

    gstate->AddCommand( "script", script );

/*
    gstate->AddCommand( "sidekick_attack", sidekick_attack );
    gstate->AddCommand( "sidekick_attack_player", sidekick_attack_player );
    gstate->AddCommand( "superfly_start_carry_mikiko", superfly_start_carry_mikiko );
    gstate->AddCommand( "superfly_stop_carry_mikiko", superfly_stop_carry_mikiko );
*/

#ifdef _DEBUG
    gstate->AddCommand( "pathcorner_toggle", pathcorner_toggle );
#endif
	gstate->AddCommand("xplevelup",xplevelup);

//    gstate->AddCommand( "sidekick_god", sidekick_god );
    	
    //register our cinematic playback functions with the exe.
    WorldCINInit();

    // Logic[5/24/99]: _CRT_ is microsoft specific....
#ifdef WIN32
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_WNDW );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );
#endif
	// NSS[1/12/00]:
	gstate->SIDEKICK_RemoveItem = SIDEKICK_RemoveItem ;
	gstate->InventorySetMode = Inventory_SetMode;
	gstate->UNIQUEID_Remove = UNIQUEID_RemoveEnt;
}

///////////////////////////////////////////////////////////////////////////////
// dll_Version
//
// returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version	(int size)
{
	if (size == IONSTORM_DLL_INTERFACE_VERSION)
		return	TRUE;
	else
		return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// dll_LevelLoad
//
// Initialize any variables here before the actual level/map loads
///////////////////////////////////////////////////////////////////////////////

void AIINFO_Init();

void dll_EntityLoadCleanup( int nIndex )
{
	userEntity_t *pEntity = &gstate->g_edicts[nIndex];

	// SCG[3/21/00]: Now, now.  Lets not waste memory!
	if( pEntity->pMapAnimationToSequence != NULL )
	{
		delete pEntity->pMapAnimationToSequence;
		pEntity->pMapAnimationToSequence = NULL;
	}
}

void dll_LevelLoad (void)
{
	gstate->Con_Dprintf ("WORLD.DLL: dll_LevelLoad\n");

	AIINFO_Init();

	SEQUENCEMAP_RemoveAll();
	world_InitCVars ();
	FLAG_Init();

	// init server global variables
//	gstate->numSecrets			= 0;			//  number of secrets in this map
//	gstate->numSecretsFound		= 0;			//  number of secrets found in this map

	client_list     = alist_init();
	monster_list    = alist_init();

	PATH_AllocateGraphPath();
	PATH_PreAllocatePathNodes();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerInit (void)
{
	//	get console variables
	world_InitCVars ();
//	CTFInitCVars();
	FLAG_Init();
	NAVTEST_Init();

//	g_cvarLogOutput = gstate->cvar( "LogOutput", "1", CVAR_ARCHIVE );

//	cvar_t* cvarLogFilePath;
//	cvarLogFilePath = gstate->cvar( "LogFilePath", "", CVAR_ARCHIVE );
//	DKLOG_Initialize( cvarLogFilePath->string );

	DKIO_Init( gstate->FS_Open,
			   gstate->FS_Close,
			   gstate->FS_FileLength,
			   gstate->FS_Read,
			   gstate->FS_Seek,
			   gstate->FS_Tell,
			   gstate->FS_Getc,
			   gstate->FS_Gets );

	// read in the AI attributes for all monsters at load time
	if ( AIATTRIBUTE_ReadFile( "AIData.cs2" ) == FALSE )
    {
		if ( AIATTRIBUTE_ReadFile( "AIData.csv" ) == FALSE )
		{
			gstate->Con_Printf( "AI file (AIDATA.CSV) not found!\n");
		}
	}

	if ( AISOUNDS_ReadFile( "MonsterSounds.cs2" ) == FALSE )
    {
		if ( AISOUNDS_ReadFile( "MonsterSounds.csv" ) == FALSE )
		{
			gstate->Con_Printf( "AI file (MonsterSounds.CSV) not found!\n");
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerKill (void)
{
    WorldCINKill();

	//DKLOG_Close();

    AIATTRIBUTE_Destroy();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void AIINFO_Init();
void	dll_LevelExit (void)
{
	if (coop->value)
		COOP_LevelExit();

	thinkFunc_remove_thinks ();

	if (client_list)
	{
		alist_destroy( client_list );
		client_list = NULL;
	}

	if (monster_list)
	{
		alist_destroy( monster_list );
		monster_list = NULL;
	}

	PATH_DestroyGraphPath();
	PATH_DestroyPreAllocatedPathNodes();
	
	if ( pGroundNodes )
	{
		NODE_Destroy( pGroundNodes );
		pGroundNodes = NULL;
	}
	if ( pAirNodes )
	{
		NODE_Destroy( pAirNodes );
		pAirNodes = NULL;
	}
	if ( pTrackNodes )
	{
		NODE_Destroy( pTrackNodes );
		pTrackNodes = NULL;
	}

	//	this was crashing on demo playback after a demo ended
	OCTREE_Destruct( pPathOctree );
	pPathOctree = NULL;

	STRINGLIST_Destruct( pSoundList );
	pSoundList = NULL;

	ITEM_Destroy();

	NAVTEST_End();

	ACTION_End();

	UNIQUEID_Destroy();

	SIGHTSOUND_FreeSounds();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void *dll_Query(void)
{
	return(eclassStr);
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Entry
//
//
///////////////////////////////////////////////////////////////////////////////

int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
	int			size;

	switch (dwReasonForCall)
	{
		case	QDLL_VERSION:
			size = *(int *) pvData;

			return	dll_Version (size);
			break;
		
		case 	QDLL_QUERY:
			* (userEpair_t **) pvData = eclassStr;

			return	TRUE;
			break;

		case	QDLL_DESCRIPTION:
			* (char **) pvData = dll_Description;

			return	TRUE;
			break;

		case	QDLL_SERVER_INIT:
			dll_ServerInit ();

			return	TRUE;
			break;

		case	QDLL_SERVER_KILL:
			dll_ServerKill ();

			return	TRUE;
			break;

		case	QDLL_SERVER_LOAD:
			dll_ServerLoad ((serverState_t *) pvData);

			return	TRUE;
			break;

		case	QDLL_LEVEL_LOAD:
			dll_LevelLoad ();

			return	TRUE;
			break;

		case	QDLL_LEVEL_EXIT:
			dll_LevelExit ();

			return	TRUE;
			break;
	}

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

//static	unsigned long	basetime = 0;
//static	unsigned long	asstime = 0;
//static	unsigned long	newtime = 0;
/*
int Sys_Milliseconds (void)
{
	if (!basetime)
		basetime = timeGetTime ();
	
	newtime = timeGetTime ();

	asstime = newtime - basetime;
	basetime = newtime;

	return asstime;
}
*/
userEntity_t *AIINFO_GetPlayer();
void InitMonsterCount()
{
	gstate->Con_Dprintf("InitMonsterCount()\n");
	userEntity_t	*pEntity;

	if( gstate->numMonsters == 0 )
	{
		for( pEntity = gstate->FirstEntity (); pEntity; pEntity = gstate->NextEntity( pEntity ) )
		{
			if( pEntity->flags & FL_MONSTER )
			{
				IncrementMonsterCount();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	worldspawn_prethink
//
//	this is the first think function called after everything has been spawned
//	it is used to build the actorlist for clients, bots and monsters and
//	to run thinkFuncs, which are basically fake entities whose sole purpose
//	is to call a function at specific intervals
///////////////////////////////////////////////////////////////////////////////
void AIINFO_AddAllSidekicks();
void worldspawn_prethink (userEntity_t *self)
{

	if (world_thinks == 0)
	{
		AIINFO_Init();

		AIINFO_AddAllSidekicks();

		// WAW[12/7/99]: Resolve conflicting CTF/COOP/DEATHMATCH
		if (coop->value)
		{
			gstate->SetCvar("deathmatch","0");
			gstate->SetCvar("ctf","0");
		}
		if ((gstate->GetCvar("ctf") || gstate->GetCvar("deathtag")) && !deathmatch->value)
			gstate->SetCvar( "deathmatch", "1" );
		
		if (deathmatch->value)
		{
			if (ctf->value)
				gstate->SetCvar("deathtag","0");
		}
		// WAW[12/7/99]: Set the game based on CTF/COOP/DEATHMATCH
		if (coop->value)
		{
			gstate->SetCvar( "gamemode", "COOP" );
			CoopLevelStart();	// Let the coop game know that we are starting a new level.
		}
		else if (gstate->GetCvar("deathtag"))
		{
			gstate->SetCvar( "gamemode", "deathtag" );
		}
		else if (gstate->GetCvar("ctf"))
		{
			gstate->SetCvar( "gamemode", "CTF" );
			CTFLevelStart();	// Let the ctf game know that we are starting a new level.
		}
		else if (deathmatch->value)
		{
			if (dm_teamplay->value)
				gstate->SetCvar( "gamemode", "TDM" );
			else
				gstate->SetCvar( "gamemode", "DM" );
		}
		else 
			gstate->SetCvar( "gamemode", "Single" );
			


		//	do startup stuff for a level
//		gstate->Con_Printf ("-----------------------\nworldspawn_prethink called!\n-----------------------\n");

		//	build actor lists now, except for client_list, because clients
		//	haven't been connected yet
		alist_build( monster_list, FL_MONSTER );

		//	load up node files if they exist
		//	FIXME:	only for demo, 5/21/98
// SCG[11/3/99]: node_load is obsolete
/*
		if ( !pGroundNodes || pGroundNodes->nNumNodes == 0 )
		{
			node_load (self);
		}
*/
		UNIQUEID_Init();

        if (!ITEM_Initialize())
		{
			com->Error("Item initialization error.\n");
		}

//		node_restore_links (self);

		thinkFunc_init ();

		AI_DoLevelStartScript();

		//	seed random number generator
		srand ((unsigned)time (NULL));

//		gstate->Con_Printf ("%i func_explosives spawned.\n", func_explosive_count);
//		gstate->Con_Printf ("%i func_wall_explodes spawned.\n", func_wall_explode_count);

		// WAW[11/13/99]: Added for capture the flag... I don't know of a better place.
		if (ctf->value)
			CTFInitialize();
    }
    else
    if ( world_thinks == 2 )
    {
		
		InitMonsterCount();

        if ( !cvarBuildPathTable )
        {
            cvarBuildPathTable = gstate->cvar( "BuildPathTable", "", 0 );
        }

        if ( cvarBuildPathTable->value > 0.0f )
        {
            massacre( NULL );
            ITEM_RemoveAll();
            PATHTABLE_BuildPathTable();
	    }
    }

	/////////////////////////////////////////////////////////
	//	handle think_funcs which is a list of pointers
	//	to functions that need to be called each frame
	//
	//	this allows us to have thinks without actually
	//	spawning a new entity
	/////////////////////////////////////////////////////////

	thinkFunc_run_thinks ();

	world_thinks++;
}


char *clientsounds[] =
{
//#if 0	// FIXME: remove after mplay demo    
		"exitwater.wav",
	    "landinwater.wav",
	    "swim1.wav",
	    "breathe.wav",
	    "breathe2.wav",
	    "choke1.wav",
	    "choke3.wav",
	    "waterdeath.wav",
	    "waterchoke1.wav",
//#endif
	    "jump1.wav",
	    "jump2.wav",
	    "jump3.wav",
	    "jump4.wav",
	    "jump5.wav",
	    "land1.wav",
	    "land2.wav",
	    "land3.wav",
	    "land4.wav",
	    "land5.wav",
//	    "landhurt.wav",
		"landhurt1.wav",
	    "landhurt2.wav",
	    "death1.wav",
	    "death2.wav",
	    "death3.wav",
	    "death4.wav",
	    "death5.wav",
	    "death6.wav",
	    "death7.wav",
		"pain1.wav",
	    "pain2.wav",
	    "pain3.wav",
	    "pain4.wav",
	    "pain5.wav",
	    "pain6.wav",
	    "pain7.wav",
		"udeath.wav",
        NULL
};

void PrecacheClientSounds(char **clienttypes)
{
    int i,j;
    char path[256];

    i=0;
    while(clienttypes[i])
    {
        j=0;
        while(clientsounds[j])
        {
//            sprintf(path,"%s/%s", clienttypes[i], clientsounds[j]);
            Com_sprintf(path,sizeof(path),"%s/%s", clienttypes[i], clientsounds[j]);
        	gstate->SoundIndex (path);
            j++;
        }
        i++;
    }
}

///////////////////////////////////////////////////////////////////////////////
//	WorldSpam
//
//	Spams necessary world objects and caches models, sounds, etc.
///////////////////////////////////////////////////////////////////////////////
void artifact_precache_sounds(void);

void worldspawn (userEntity_t *self)
{
	int	i;
    char *clienttypes[]={"mikiko","hiro","superfly", NULL};

	//	this should be done in physics
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	self->inuse = true;			// since the world doesn't use G_Spawn()
	self->s.modelindex = 1;		// world model is always index 1

	//	seed random number generator with time
	srand	((unsigned)time (NULL));

	gstate->fogactive = 0;

	i = 0;

//	NODE_InitNodeFileName();

    while ( self->epair[i].key != NULL )
	{
		if (!stricmp(self->epair[i].key, "mapname"))
		{
			Com_sprintf(gstate->mapTitle,sizeof(gstate->mapTitle),"%s",self->epair[i].value);
		}
		if (!stricmp(self->epair[i].key, "episode"))
		{
			gstate->episode = atoi (self->epair [i].value);
		}
		else if (!stricmp(self->epair[i].key, "fog_value"))
		{
			gstate->fogactive = atoi (self->epair [i].value);
		}
		else if (!stricmp (self->epair[i].key, "fog_start"))
		{
			gstate->fogstart = atof (self->epair [i].value);
		}
		else if (!stricmp (self->epair[i].key, "fog_end"))
		{
			gstate->fogend = atof (self->epair [i].value);
		}
		else if (!stricmp (self->epair[i].key, "fog_skyend"))
		{
			gstate->fogskyend = atof (self->epair [i].value);
		}
		else if (!stricmp (self->epair[i].key, "fog_color"))
		{
			sscanf (self->epair[i].value, "%f %f %f", &gstate->fogcolor[0], &gstate->fogcolor[1], &gstate->fogcolor[2]);
			gstate->fogcolor[0] /= 255;
			gstate->fogcolor[1] /= 255;
			gstate->fogcolor[2] /= 255;
		}
		else if (!stricmp (self->epair[i].key, "_color"))
		{
			sscanf (self->epair[i].value, "%f %f %f", &gstate->fogcolor[0], &gstate->fogcolor[1], &gstate->fogcolor[2]);
		}
		else if (!stricmp (self->epair [i].key, "cdtrack"))
		{
			if (!gstate->GetCvar ("developer"))
			{
				gstate->cdtrack = atof (self->epair [i].value);

				gstate->ConfigString (CS_CDTRACK, self->epair [i].value);
			}
		}
		else if (!stricmp (self->epair [i].key, "sky"))
        {
            gstate->ConfigString (CS_SKY, self->epair [i].value);
            gstate->SetCvar("gl_skyname", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "palette"))
			gstate->ConfigString (CS_PALETTE, self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lightningfreq"))
			gstate->SetCvar("gl_lightningfreq", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudname"))
        {
            gstate->ConfigString (CS_CLOUDNAME, self->epair [i].value);
            gstate->SetCvar("gl_cloudname", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "cloudxdir"))
			gstate->SetCvar("gl_cloudxdir", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudydir"))
			gstate->SetCvar("gl_cloudydir", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1tile"))
			gstate->SetCvar("gl_cloud1tile", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1speed"))
			gstate->SetCvar("gl_cloud1speed", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2tile"))
			gstate->SetCvar("gl_cloud2tile", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2speed"))
			gstate->SetCvar("gl_cloud2speed", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2alpha"))
			gstate->SetCvar("gl_cloud2alpha", self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "sky_2"))
        {
            gstate->ConfigString (CS_SKY_2, self->epair [i].value);
            gstate->SetCvar("gl_skyname_2", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "lightningfreq_2"))
			gstate->SetCvar("gl_lightningfreq_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudname_2"))
        {
            gstate->ConfigString (CS_CLOUDNAME_2, self->epair [i].value);
            gstate->SetCvar("gl_cloudname_2", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "cloudxdir_2"))
			gstate->SetCvar("gl_cloudxdir_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudydir_2"))
			gstate->SetCvar("gl_cloudydir_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1tile_2"))
			gstate->SetCvar("gl_cloud1tile_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1speed_2"))
			gstate->SetCvar("gl_cloud1speed_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2tile_2"))
			gstate->SetCvar("gl_cloud2tile_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2speed_2"))
			gstate->SetCvar("gl_cloud2speed_2", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2alpha_2"))
			gstate->SetCvar("gl_cloud2alpha_2", self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "sky_3"))
        {
            gstate->ConfigString (CS_SKY_3, self->epair [i].value);
            gstate->SetCvar("gl_skyname_3", self->epair [i].value);
        }
        else if (!stricmp (self->epair [i].key, "lightningfreq_3"))
			gstate->SetCvar("gl_lightningfreq_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudname_3"))
        {
            gstate->ConfigString (CS_CLOUDNAME_3, self->epair [i].value);
            gstate->SetCvar("gl_cloudname_3", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "cloudxdir_3"))
			gstate->SetCvar("gl_cloudxdir_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudydir_3"))
			gstate->SetCvar("gl_cloudydir_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1tile_3"))
			gstate->SetCvar("gl_cloud1tile_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1speed_3"))
			gstate->SetCvar("gl_cloud1speed_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2tile_3"))
			gstate->SetCvar("gl_cloud2tile_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2speed_3"))
			gstate->SetCvar("gl_cloud2speed_3", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2alpha_3"))
			gstate->SetCvar("gl_cloud2alpha_3", self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "sky_4"))
        {
            gstate->ConfigString (CS_SKY_4, self->epair [i].value);
            gstate->SetCvar("gl_skyname_4", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "lightningfreq_4"))
			gstate->SetCvar("gl_lightningfreq_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudname_4"))
        {
            gstate->ConfigString (CS_CLOUDNAME_4, self->epair [i].value);
            gstate->SetCvar("gl_cloudname_4", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "cloudxdir_4"))
			gstate->SetCvar("gl_cloudxdir_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudydir_4"))
			gstate->SetCvar("gl_cloudydir_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1tile_4"))
			gstate->SetCvar("gl_cloud1tile_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1speed_4"))
			gstate->SetCvar("gl_cloud1speed_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2tile_4"))
			gstate->SetCvar("gl_cloud2tile_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2speed_4"))
			gstate->SetCvar("gl_cloud2speed_4", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2alpha_4"))
			gstate->SetCvar("gl_cloud2alpha_4", self->epair [i].value);

		else if (!stricmp (self->epair [i].key, "sky_5"))
        {
            gstate->ConfigString (CS_SKY_5, self->epair [i].value);
            gstate->SetCvar("gl_skyname_5", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "lightningfreq_5"))
			gstate->SetCvar("gl_lightningfreq_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudname_5"))
        {
            gstate->ConfigString (CS_CLOUDNAME_5, self->epair [i].value);
            gstate->SetCvar("gl_cloudname_5", self->epair [i].value);
        }
		else if (!stricmp (self->epair [i].key, "cloudxdir_5"))
			gstate->SetCvar("gl_cloudxdir_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloudydir_5"))
			gstate->SetCvar("gl_cloudydir_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1tile_5"))
			gstate->SetCvar("gl_cloud1tile_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud1speed_5"))
			gstate->SetCvar("gl_cloud1speed_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2tile_5"))
			gstate->SetCvar("gl_cloud2tile_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2speed_5"))
			gstate->SetCvar("gl_cloud2speed_5", self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "cloud2alpha_5"))
			gstate->SetCvar("gl_cloud2alpha_5", self->epair [i].value);
        else if ( _stricmp( self->epair[i].key, "NodeFile" ) == 0 )
        {
            NODE_SetNodeFileName( self->epair[i].value );
        }
		i++;
	}

	self->className = "worldspawn";

	//	don't let mappers accidentally put an angle in the world spawn...
	//	causes world clipping to be totally screwed (in Q1, at least).
	self->s.angles.Zero();

	///////////////////////////////////////////////////
	// monster weapons
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// gibs / debug 
	///////////////////////////////////////////////////

	ai_precache_global_models ();

	////////////////////////////////////////////////////////
	// sound caches
	////////////////////////////////////////////////////////

	// Daikatana client sounds
    PrecacheClientSounds(clienttypes);
	artifact_precache_sounds();

    // Boost effect sounds
    gstate->SoundIndex("global/a_powerquad.wav");
	gstate->SoundIndex("global/Acroboost.wav");

	// item pickup sounds
	gstate->SoundIndex("global/i_pickup6.wav");
    gstate->SoundIndex("global/e_teleportstart.wav"); // 3.5 dsn  boost blink sounds
	gstate->SoundIndex("global/i_c4ammo.wav");

/*
	//cek[12-7-99] added per Romero
	gstate->SoundIndex("global/i_ionammo.wav");
	gstate->SoundIndex("global/i_scammo.wav");
	gstate->SoundIndex("global/i_swaveammo.wav");
	gstate->SoundIndex("global/i_swinderammo.wav");
	gstate->SoundIndex("global/i_scyclerammo.wav");
*/	
	// talk sound
	gstate->SoundIndex("global/e_talk.wav");
	gstate->SoundIndex("global/a_ames.wav");

	// menu sounds
/*
    gstate->SoundIndex("menus/Enter Menu_001.wav");
    gstate->SoundIndex("menus/600ms rotate descend_001.wav");
    gstate->SoundIndex("menus/600ms rotate ascend_001.wav");
    gstate->SoundIndex("menus/no_001.wav");
    gstate->SoundIndex("menus/button_003.wav");
    gstate->SoundIndex("menus/button_004.wav");
    gstate->SoundIndex("menus/button_005.wav");
*/
	// lazer zap sounds
	gstate->SoundIndex("global/we_zapa.wav");
	gstate->SoundIndex("global/we_zapb.wav");

    // misc sounds
	gstate->SoundIndex("global/e_explodeh.wav")	;
	//NSS[11/1/99]: Wisps Electrical sound
	gstate->SoundIndex("e3/we_wwispcorditea.wav");
	gstate->SoundIndex("e3/we_wwispcorditeb.wav");
	gstate->SoundIndex("e3/we_wwispcorditec.wav");
	
	//NSS[11/13/99]:Add swim noises from hiro's dir
	gstate->SoundIndex("hiro/swim1.wav");
	gstate->SoundIndex("hiro/swim2.wav");
	gstate->SoundIndex("hiro/swim3.wav");
	gstate->SoundIndex("hiro/swim4.wav");
	
	//NSS[10/25/99]:Bullets hitting flesh pre-cache
    gstate->SoundIndex("global/e_bulflesha.wav");
    gstate->SoundIndex("global/e_bulfleshb.wav");
    gstate->SoundIndex("global/e_bulfleshc.wav");
    gstate->SoundIndex("global/e_bulfleshd.wav");
    gstate->SoundIndex("global/e_bulfleshe.wav");

	//NSS[10/29/99]:generic firetraveling noise
	gstate->SoundIndex("global/e_firetravela.wav");
	gstate->SoundIndex("global/e_firetravelb.wav");
	gstate->SoundIndex("global/e_firetravelc.wav");
	gstate->SoundIndex("global/e_firetraveld.wav");
	gstate->SoundIndex("global/e_firetravele.wav");

	//NSS[10/25/99]:Earthquake sounds pre-cache
    gstate->SoundIndex("global/earthquake_a.wav");
    gstate->SoundIndex("global/earthquake_b.wav");
    gstate->SoundIndex("global/earthquake_c.wav");
    gstate->SoundIndex("global/earthquake_d.wav");
    gstate->SoundIndex("global/earthquake_e.wav");

	//NSS[10/25/99]:Added 3 more pain and death sounds for Hiro
    gstate->SoundIndex("hiro/death8.wav");
    gstate->SoundIndex("hiro/death9.wav");
    gstate->SoundIndex("hiro/death10.wav");
    gstate->SoundIndex("hiro/pain8.wav");
    gstate->SoundIndex("hiro/pain9.wav");
    gstate->SoundIndex("hiro/pain10.wav");

	// NSS[2/26/00]:Pre-Cache our bone crunching noise.
	gstate->SoundIndex("global/m_gibbonecrka.wav");
	gstate->SoundIndex("global/m_gibbonecrkb.wav");
	gstate->SoundIndex("global/m_gibbonecrkc.wav");
	gstate->SoundIndex("global/m_gibbonecrkd.wav");
	gstate->SoundIndex("global/m_gibbonecrke.wav");


    gstate->SoundIndex("global/e_ricocheta.wav");
    gstate->SoundIndex("global/e_ricochetb.wav");
    gstate->SoundIndex("global/e_ricochetc.wav");
    gstate->SoundIndex("global/e_ricochetd.wav");
    gstate->SoundIndex("global/e_ricochete.wav");
    gstate->SoundIndex("global/e_ricochetf.wav");
    gstate->SoundIndex("global/e_ricochetg.wav");
    gstate->SoundIndex("global/e_ricocheth.wav");

	gstate->SoundIndex("global/we_noammo.wav");       // global no-ammo sound

	gstate->SoundIndex("global/we_noammo.wav");       // global no-ammo sound

	// explosions
    gstate->SoundIndex("global/e_explodeb.wav");
    gstate->SoundIndex("global/e_explodec.wav");

	// NSS[2/26/00]:
	// gib sounds
	gstate->SoundIndex("global/m_gibslopa.wav");
	gstate->SoundIndex("global/m_gibslopb.wav");
	gstate->SoundIndex("global/m_gibslopc.wav");
	gstate->SoundIndex("global/m_gibslopd.wav");
	gstate->SoundIndex("global/m_gibslope.wav");
    gstate->SoundIndex("global/m_gibsurfa.wav");
    gstate->SoundIndex("global/m_gibsurfb.wav");
    gstate->SoundIndex("global/m_gibsurfc.wav");
	gstate->SoundIndex("global/m_gibmeata.wav");
	gstate->SoundIndex("global/m_gibmeatb.wav");
	gstate->SoundIndex("global/m_gibmeatc.wav");
	gstate->SoundIndex("global/m_gibmeatd.wav");
	gstate->SoundIndex("global/m_gibmeate.wav");


	gstate->SoundIndex("global/skill_increase.wav");
	gstate->SoundIndex("global/skill_selector.wav");
	gstate->SoundIndex("global/skill_start.wav");

	gstate->SoundIndex("global/inventory_open.wav");
	gstate->SoundIndex("global/inventory_update.wav");

	// AI Weapon Sounds
    gstate->SoundIndex("global/e_arrowimp.wav");
	if( gstate->episode == 3 )
		gstate->SoundIndex("e3/m_dwaraxfly.wav");

	// SCG[10/29/99]: Added grunting sounds!!!
	gstate->SoundIndex( "hiro/grunt11.wav" );
	gstate->SoundIndex( "hiro/grunt12.wav" );
	gstate->SoundIndex( "hiro/grunt13.wav" );
	gstate->SoundIndex( "hiro/grunt1.wav" );

	// NSS[2/26/00]:Sounds for debris
	gstate->SoundIndex( "global/e_Glassbreaksa.wav");
	gstate->SoundIndex( "global/e_Glassbreaksb.wav");
	gstate->SoundIndex( "global/e_Glassbreaksc.wav");
	gstate->SoundIndex( "global/e_Glassbreaksd.wav");
	gstate->SoundIndex( "global/e_Glassbreakse.wav");
	gstate->SoundIndex( "global/e_Glassbreaksf.wav");

	gstate->SoundIndex( "global/e_metalbreaksa.wav");
	gstate->SoundIndex( "global/e_metalbreaksb.wav");
	gstate->SoundIndex( "global/e_metalbreaksc.wav");
	gstate->SoundIndex( "global/e_metalbreaksd.wav");
	gstate->SoundIndex( "global/e_metalbreakse.wav");
	gstate->SoundIndex( "global/e_metalbreaksf.wav");

	gstate->SoundIndex( "global/e_woodbreaksa.wav");
	gstate->SoundIndex( "global/e_woodbreaksb.wav");
	gstate->SoundIndex( "global/e_woodbreaksc.wav");
	gstate->SoundIndex( "global/e_woodbreaksd.wav");
	gstate->SoundIndex( "global/e_woodbreakse.wav");
	gstate->SoundIndex( "global/e_woodbreaksf.wav");

	// NSS[2/29/00]:Register sounds needed for psyclaw
	gstate->SoundIndex("global/e_atmospheref.wav");

	// SCG[2/22/00]: only needed for multiplayer
	if( ( coop->value != 0 ) || ( deathmatch->value != 0 ) )
	{
		// for a door with struggle on it.
		gstate->SoundIndex( "superfly/s_struggle1.wav" );
		gstate->SoundIndex( "superfly/s_struggle2.wav" );
		gstate->SoundIndex( "superfly/s_struggle3.wav" );
		gstate->SoundIndex( "superfly/s_struggle4.wav" );
		gstate->SoundIndex( "superfly/s_struggle5.wav" );
		gstate->SoundIndex( "mikiko/m_struggle1.wav" );
		gstate->SoundIndex( "mikiko/m_struggle2.wav" );
		gstate->SoundIndex( "mikiko/m_struggle3.wav" );
		gstate->SoundIndex( "mikiko/m_struggle4.wav" );
		gstate->SoundIndex( "mikiko/m_struggle5.wav" );

		// using something not usable
		gstate->SoundIndex( "superfly/s_grunt1.wav" );
		gstate->SoundIndex( "mikiko/m_grunt1.wav" );
	}

	// NSS[3/2/00]:More sounds... ack!
	gstate->SoundIndex("global/a_poisonfade.wav");
#ifdef TONGUE_ENGLISH
	// SCG[11/10/99]: These are the deathmatch taunts, BTW
	if (deathmatch->value)
	{
		if( gstate->episode == 1 )
		{
			gstate->SoundIndex( "e1/dmsg_1_1.wav" );
			gstate->SoundIndex( "e1/dmsg_1_7.wav" );
			gstate->SoundIndex( "e1/dmsg_1_2.wav" );
			gstate->SoundIndex( "e1/dmsg_1_3.wav" );
			gstate->SoundIndex( "e1/dmsg_1_4.wav" );
			gstate->SoundIndex( "e1/dmsg_1_5.wav" );
			gstate->SoundIndex( "e1/dmsg_1_6.wav" );
		}
		else if( gstate->episode == 2 )
		{
			gstate->SoundIndex( "e2/dmsg_2_1.wav" );
			gstate->SoundIndex( "e2/dmsg_2_2.wav" );
			gstate->SoundIndex( "e2/dmsg_2_3.wav" );
			gstate->SoundIndex( "e2/dmsg_2_4.wav" );
			gstate->SoundIndex( "e2/dmsg_2_5.wav" );
			gstate->SoundIndex( "e2/dmsg_2_6.wav" );
		}
		else if( gstate->episode == 3 )
		{
			gstate->SoundIndex( "e3/dmsg_3_1.wav" );
			gstate->SoundIndex( "e3/dmsg_3_2.wav" );
			gstate->SoundIndex( "e3/dmsg_3_3.wav" );
			gstate->SoundIndex( "e3/dmsg_3_4.wav" );
			gstate->SoundIndex( "e3/dmsg_3_5.wav" );
			gstate->SoundIndex( "e3/dmsg_3_6.wav" );
		}
		else if( gstate->episode == 4 )
		{
			gstate->SoundIndex( "e4/dmsg_4_1.wav" );
			gstate->SoundIndex( "e4/dmsg_4_2.wav" );
			gstate->SoundIndex( "e4/dmsg_4_3.wav" );
			gstate->SoundIndex( "e4/dmsg_4_4.wav" );
			gstate->SoundIndex( "e4/dmsg_4_5.wav" );
			gstate->SoundIndex( "e4/dmsg_4_6.wav" );
		}

		gstate->SoundIndex( "global/matchcomplete.wav" );
		gstate->SoundIndex( "global/dmsg_daikatana.wav" );
		gstate->SoundIndex( "global/dmsg_telefragged.wav" );
	}
#endif

	// SCG[11/10/99]: Drowning/drowning death sounds
	gstate->SoundIndex( "superfly/waterchoke2.wav" );
	gstate->SoundIndex( "superfly/waterchoke3.wav" );
	gstate->SoundIndex( "superfly/waterchoke4.wav" );
	gstate->SoundIndex( "mikiko/waterchoke1.wav" );
	gstate->SoundIndex( "mikiko/waterchoke2.wav" );
	gstate->SoundIndex( "mikiko/waterchoke3.wav" );
	gstate->SoundIndex( "hiro/waterdeath1.wav" );
	gstate->SoundIndex( "hiro/waterdeath2.wav" );
	gstate->SoundIndex( "hiro/waterdeath3.wav" );
	gstate->SoundIndex( "hiro/waterland4.wav" );
	gstate->SoundIndex( "hiro/waterland6.wav" );
	gstate->SoundIndex( "superfly/icehurt2.wav" );
	gstate->SoundIndex( "superfly/icehurt1.wav" );
	gstate->SoundIndex( "mikiko/icehurt2.wav" );
	gstate->SoundIndex( "mikiko/icehurt1.wav" );
	gstate->SoundIndex( "hiro/icehurt2.wav" );
	gstate->SoundIndex( "hiro/icehurt1.wav" );

	// client ice death sounds
	if (deathmatch->value || coop->value)
	{
		gstate->SoundIndex( "superfly/ice1.wav" );
		gstate->SoundIndex( "superfly/ice2.wav" );
		gstate->SoundIndex( "superfly/ice3.wav" );
		gstate->SoundIndex( "superfly/ice4.wav" );
		gstate->SoundIndex( "mikiko/ice2.wav" );
		gstate->SoundIndex( "mikiko/ice3.wav" );
		gstate->SoundIndex( "mikiko/ice4.wav" );
		gstate->SoundIndex( "mikiko/ice5.wav" );
		gstate->SoundIndex( "mikiko/ice6.wav" );
		gstate->SoundIndex( "mikiko/ice7.wav" );
	}
	gstate->SoundIndex( "hiro/icedeath1.wav" );
	gstate->SoundIndex( "hiro/icedeath2.wav" );
	gstate->SoundIndex( "hiro/icedeath3.wav" );
	gstate->SoundIndex( "hiro/icedeath4.wav" );
	gstate->SoundIndex( "hiro/icedeath5.wav" );

	// cek[11/17/99]: Some cool sounds for the daikatana
	if (deathmatch->value || (gstate->episode > 1))
	{
		gstate->SoundIndex( "global/we_swordwhoosha.wav");
		gstate->SoundIndex( "global/we_swordwhooshb.wav");
		gstate->SoundIndex( "global/we_swordwhooshc.wav");
		gstate->SoundIndex( "global/we_swordwhooshd.wav");
		gstate->SoundIndex( "global/we_swordwhooshe.wav");
		gstate->SoundIndex( "global/we_swordwhooshf.wav");
		gstate->SoundIndex( "global/m_swordhita.wav");
		gstate->SoundIndex( "global/m_swordhitb.wav");
		gstate->SoundIndex( "global/m_swordhitc.wav");
		gstate->SoundIndex( "global/m_swordhitd.wav");
		gstate->SoundIndex( "global/m_swordhite.wav");
		gstate->SoundIndex( "global/we_swordwclanka.wav");
		gstate->SoundIndex( "global/we_swordwclankb.wav");
		gstate->SoundIndex( "global/we_swordwclankc.wav");
		gstate->SoundIndex( "global/we_swordwclankd.wav");
		gstate->SoundIndex( "global/we_swordwclanke.wav");
		gstate->SoundIndex( "global/m_armorhita.wav");
		gstate->SoundIndex( "global/m_armorhitb.wav");
		gstate->SoundIndex( "global/m_armorhitc.wav");
		gstate->SoundIndex( "global/m_armorhitd.wav");
		gstate->SoundIndex( "global/m_armorhite.wav");
		gstate->SoundIndex( "global/we_swordstaba.wav");
		gstate->SoundIndex( "global/we_swordstabb.wav");
		gstate->SoundIndex( "global/we_swordstabc.wav");
		gstate->SoundIndex( "global/we_swordstabd.wav");
	}

	// register models for the boost icons.
	gstate->ModelIndex("models/global/a_pwrb.dkm");
	gstate->ModelIndex("models/global/a_pwrc.dkm");
	gstate->ModelIndex("models/global/a_atkb.dkm");
	gstate->ModelIndex("models/global/a_atkc.dkm");
	gstate->ModelIndex("models/global/a_spdb.dkm");
	gstate->ModelIndex("models/global/a_spdc.dkm");
	gstate->ModelIndex("models/global/a_acrb.dkm");
	gstate->ModelIndex("models/global/a_acrc.dkm");
	gstate->ModelIndex("models/global/a_vtlb.dkm");
	gstate->ModelIndex("models/global/a_vtlc.dkm");

// SCG[3/11/00]: debug model registration
#pragma message( "// SCG[1/11/00]: Remove for Release build" )
#ifdef _DEUBG
	gstate->ModelIndex( "models/global/dv_curnode.dkm" );
	gstate->ModelIndex( "models/global/dv_lastnode.dkm" );
	gstate->ModelIndex( "models/global/dv_node.dkm" );
	gstate->ModelIndex( "models/global/dv_marker.dkm" );
	gstate->ModelIndex( "models/global/dv_jumptarg.dkm" );
	gstate->ModelIndex( "models/global/dv_goal.dkm" );
	gstate->ModelIndex( "models/global/dv_arrow.dkm" );
	gstate->ModelIndex( "models/global/dv_pathnode.dkm" );
#endif

	///////////////////////////////////////////////////////
	//	set up light styles
	///////////////////////////////////////////////////////

	light_setstyles ();
	// set up node mapping for clients/bots/monsters
	
	//NSS[11/12/99]:We handle this at dll_levelload now
	//	pPathOctree = OCTREE_Construct();
	pSoundList = STRINGLIST_Construct();
	
//	DKLOG_Write( LOGTYPE_GRAPHICS, 2.0f, "worldspawn()" );

	// set up circular body list
	bodylist_init (self);

	// init client count
	cur_clients = 0;
	world_thinks = 0;

	gstate->gravity_dir.Set( 0, 0, -1.0f );

	///////////////////////////////////////////////////////////////////////////
	//	reset some debug variables
	///////////////////////////////////////////////////////////////////////////

	func_explosive_count = 0;
	func_wall_explode_count = 0;

	///////////////////////////////////////////////////////////////////////////
	//	setup prethink
	///////////////////////////////////////////////////////////////////////////

	self->prethink = worldspawn_prethink;

	// add prethink to function list
	gstate->RegisterFunc("worldspawn_prethink",worldspawn_prethink);

	ACTION_Start();
}
void world_ai_dopefish_register_func();
void target_register_func();
void misc_register_func();
void door_register_func();
void monitor_register_func();
void trigger_register_func();
void artifact_register_func();

void world_ai_move_register_func();
void world_bodylist_register_func();
void world_bot_register_func();
void world_effects_register_func();

void world_light_register_func();
void world_nodelist_register_func();

void world_SFX_ENTITIES_register_func();
void world_ai_skeleton_register_func();
void world_sidekick_register_func();
void world_ai_func_register_func();
#ifdef _DEBUG// SCG[12/9/99]: 
void world_ai_test_register_func();
#endif //_DEBUG// SCG[12/9/99]: 
void world_ai_think_register_func();
void world_ai_weapons_register_func();

// E1 Monsters
void world_ai_battleboar_register_func();
void world_ai_cambot_register_func();
void world_ai_crox_register_func();
void world_ai_cryotech_register_func();
void world_ai_deathsphere_register_func();
void world_ai_fatworker_register_func();
void world_ai_firefly_register_func();
void world_ai_froginator_register_func();
void world_ai_inmater_register_func();
void world_ai_lasergat_register_func();
void world_ai_mishimaguard_register_func();
void world_ai_prisoner_register_func();
void world_ai_protopod_register_func();
void world_ai_psyclaw_register_func();
void world_ai_ragemaster_register_func();
void world_ai_rockgat_register_func();
void world_ai_skeeter_register_func();
void world_ai_skinnyworker_register_func();
void world_ai_sludgeminion_register_func();
void world_ai_surgeon_register_func();
void world_ai_thunderskeet_register_func();
void world_ai_venomvermin_register_func();

// E2 Monsters
void world_ai_centurion_register_func();
void world_ai_cerberus_register_func();
void world_ai_griffon_register_func();
void world_ai_harpy_register_func();
void world_ai_medusa_register_func();
void world_ai_satyr_register_func();
void world_ai_skeleton_register_func();
void world_ai_smallspider_register_func();
void world_ai_spider_register_func();
void world_ai_thief_register_func();

// E3 Monsters
void world_ai_buboid_register_func();
void world_ai_doombat_register_func();
void world_ai_dragon_register_func();
void world_ai_dwarf_register_func();
void world_ai_fletcher_register_func();
void world_ai_garroth_register_func();
void world_ai_paladin_register_func();
void world_ai_valkyrie_register_func();
void world_ai_lycanthir_register_func();
void world_ai_nharre_register_func();
void world_ai_prat_register_func();
void world_ai_rotworm_register_func();
void world_ai_stavros_register_func();
void world_ai_wisp_register_func();
void world_ai_wyndrax_register_func();

// E4 Monsters
void world_ai_blackpris_register_func();
void world_ai_chaingang_register_func();
void world_ai_femgang_register_func();
void world_ai_kage_register_func();
void world_ai_labmonkey_register_func();
void world_ai_piperat_register_func();
void world_ai_rocketgang_register_func();
void world_ai_scomndo_register_func();
//void world_ai_sdiver_register_func();// SCG[1/23/00]: not used
void world_ai_mikikomon_register_func();
void world_ai_seagull_register_func();
void world_ai_sealcap_register_func();
void world_ai_sgirl_register_func();
void world_ai_shark_register_func();
void world_ai_uzigang_register_func();
void world_ai_whitepris_register_func();

void world_client_register_func();
void world_common_register_func();
// SCG[12/5/99]: void world_float_register_func();
void world_func2_register_func();
// SCG[12/5/99]: void world_func_anim_register_func();
void world_func_event_generator_register_func();
void world_func_various_register_func();
void world_gib_register_func();
void world_healthtree_register_func();
void world_hosportal_register_func();

void world_spawn_register_func();
#ifdef _DEBUG
void world_viewthing_register_func();
#endif

void flag_register_func();
void coop_register_func();

// cek[6-16-00]: register your functions!!!
void camera_register_func(void);

void	world_register_funcs()
{
	// register all of the callback functions for world.dll
	gstate->RegisterFunc("Client_PreThink",Client_PreThink);
	gstate->RegisterFunc("Client_PostThink",Client_PostThink);
	gstate->RegisterFunc("Client_Pain",Client_Pain);
	gstate->RegisterFunc("Client_Die",Client_Die);
	gstate->RegisterFunc("restore_timeout",restore_timeout);

	target_register_func();		// target.cpp
	misc_register_func();		// misc.cpp
	door_register_func();		// door.cpp
	monitor_register_func();	// func_monitor.cpp
	trigger_register_func();	// triggers.cpp
	artifact_register_func();	// artifacts.cpp
	world_ai_move_register_func();
	world_bodylist_register_func();
	world_bot_register_func();

	// E1 Monsters
	world_ai_battleboar_register_func();
	world_ai_cambot_register_func();
	world_ai_crox_register_func();
	world_ai_cryotech_register_func();
	world_ai_deathsphere_register_func();
	world_ai_fatworker_register_func();
	world_ai_firefly_register_func();
	world_ai_froginator_register_func();
	world_ai_inmater_register_func();
	world_ai_lasergat_register_func();
	world_ai_mishimaguard_register_func();
	world_ai_prisoner_register_func();
	world_ai_protopod_register_func();
	world_ai_psyclaw_register_func();
	world_ai_ragemaster_register_func();
	world_ai_rockgat_register_func();
	world_ai_skeeter_register_func();
	world_ai_skinnyworker_register_func();
	world_ai_sludgeminion_register_func();
	world_ai_surgeon_register_func();
	world_ai_thunderskeet_register_func();
	world_ai_venomvermin_register_func();
	// E2 Monsters
	world_ai_centurion_register_func();
	world_ai_cerberus_register_func();
	world_ai_griffon_register_func();
	world_ai_harpy_register_func();
	world_ai_medusa_register_func();
	world_ai_satyr_register_func();
	world_ai_skeleton_register_func();
	world_ai_smallspider_register_func();
	world_ai_spider_register_func();
	world_ai_thief_register_func();
	world_ai_buboid_register_func();
	world_ai_doombat_register_func();
	world_ai_dragon_register_func();
	world_ai_dwarf_register_func();
	world_ai_fletcher_register_func();
	world_ai_garroth_register_func();
	world_ai_paladin_register_func();
	world_ai_valkyrie_register_func();
	world_ai_lycanthir_register_func();
	world_ai_nharre_register_func();
	world_ai_prat_register_func();
	world_ai_rotworm_register_func();
	world_ai_stavros_register_func();
	world_ai_wisp_register_func();
	world_ai_wyndrax_register_func();
	world_ai_blackpris_register_func();
	world_ai_chaingang_register_func();
	world_ai_femgang_register_func();
	world_ai_kage_register_func();
	world_ai_labmonkey_register_func();
	world_ai_piperat_register_func();
	world_ai_rocketgang_register_func();
	world_ai_scomndo_register_func();
// SCG[12/5/99]: 	world_ai_sdiver_register_func();
	world_ai_seagull_register_func();
	world_ai_sealcap_register_func();
	world_ai_sgirl_register_func();
	world_ai_shark_register_func();
	world_ai_uzigang_register_func();
	world_ai_whitepris_register_func();
	world_ai_mikikomon_register_func();

	world_effects_register_func();
	world_light_register_func();
	world_nodelist_register_func();
	world_SFX_ENTITIES_register_func();
	world_sidekick_register_func();
	world_ai_func_register_func();
#ifdef _DEBUG
	world_ai_test_register_func();
#endif //_DEBUG
	world_ai_think_register_func();
	world_ai_weapons_register_func();
	world_client_register_func();
	world_common_register_func();
// SCG[12/5/99]: 	world_float_register_func();
	world_func2_register_func();
// SCG[12/5/99]: 	world_func_anim_register_func();
	world_func_event_generator_register_func();
	world_func_various_register_func();
	world_gib_register_func();
	world_healthtree_register_func();
	world_hosportal_register_func();
	world_spawn_register_func();
#ifdef _DEBUG
	world_viewthing_register_func();
#endif

	flag_register_func();
	coop_register_func();
	world_ai_dopefish_register_func();

// cek[6-16-00]: register your functions!!!
	camera_register_func();
}
