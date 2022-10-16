#ifndef _NODELIST_H
#define _NODELIST_H

// IMPORTANT: these definitions are duplicated in nodes.h in IONRadiant project, 
// so any modification to these need to be reflected in nodes.h file

#define NODETYPE_GROUND			0x00000001	
#define NODETYPE_WATER			0x00000002
#define NODETYPE_AIR			0x00000004
#define NODETYPE_TRACK			0x00000008 
#define NODETYPE_HEALTH			0x00000010 
#define NODETYPE_WEAPON			0x00000020 
#define NODETYPE_DOOR			0x00000040 
#define NODETYPE_SWITCH			0x00000080 
#define NODETYPE_PLATFORM		0x00000100 
#define NODETYPE_TRAIN			0x00000200 
#define NODETYPE_ELEVATOR		0x00000400 
#define NODETYPE_SNIPE			0x00000800 
#define NODETYPE_HIDE			0x00001000 
#define NODETYPE_RETREAT		0x00002000 
#define NODETYPE_AMBUSH			0x00004000 
#define NODETYPE_JUMPUP			0x00008000 
#define NODETYPE_JUMPFORWARD	0x00010000 
#define NODETYPE_TELEPORTER		0x00020000 
#define NODETYPE_ROCKETJUMP		0x00040000
#define NODETYPE_LADDER			0x00080000
#define NODETYPE_DOORBUTTON		0x00100000
#define NODETYPE_SHOTCYCLERJUMP	0x00200000
#define NODETYPE_CROUCH         0x00400000
#define NODETYPE_STOPSIDEKICK   0x00800000
#define NODETYPE_TELEPORTSIDEKICK 0x01000000
#define NODETYPE_SUPERFLYTALK   0x02000000
#define NODETYPE_MIKIKOTALK     0x04000000


////////////////////////////////////////////////////////////////////////////
//	typedefs
////////////////////////////////////////////////////////////////////////////

typedef	struct	teleportNode_s
{
	CVector	start_origin;
	CVector	dest_origin;
} teleportNode_t;


////////////////////////////////////////////////////////////////////////////
//	globals and externs
////////////////////////////////////////////////////////////////////////////

extern	NODEHEADER_PTR pGroundNodes;
extern	NODEHEADER_PTR pAirNodes;
extern  NODEHEADER_PTR pTrackNodes;

////////////////////////////////////////////////////////////////////////////
//	debugging stuff
////////////////////////////////////////////////////////////////////////////

void node_marker_think( userEntity_t *marker );
void node_show_links( userEntity_t *self );
void node_toggle_nodes( userEntity_t *self );

////////////////////////////////////////////////////////////////////////////
//	node list management functions
////////////////////////////////////////////////////////////////////////////

NODELIST_PTR node_create_nodelist( NODEHEADER_PTR pNodeHeader );
MAPNODE_PTR node_best_time_for_ID( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode, int ID );
MAPNODE_PTR node_next_travel( userEntity_t *self, NODELIST_PTR pNodeList );
int	node_can_path( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode, int depth );
MAPNODE_PTR node_closest( NODELIST_PTR pNodeList, CVector &origin, float dist );
MAPNODE_PTR NODE_GetClosestNode( userEntity_t *self, CVector &destPoint );
MAPNODE_PTR NODE_GetClosestNode( userEntity_t *self );
NODEHEADER_PTR NODE_GetCurrentNodeHeader( userEntity_t *self );
MAPNODE_PTR NODE_GetClosestNodeToEntity( userEntity_t *self, userEntity_t *destEntity );
MAPNODE_PTR node_linked_with_target( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode, char *targetname );
int	node_find_cur_node( userEntity_t *self, NODELIST_PTR pNodeList );
void node_place_spawn_node( userEntity_t *self, NODELIST_PTR pNodeList );
void node_place_node( userEntity_t *self, NODELIST_PTR pNodeList, const CVector &last_loc );
void node_lay( userEntity_t *self );

//void node_force_node( userEntity_t *self, NODELIST_PTR pNodeList, const CVector &origin, int type, void *data );
MAPNODE_PTR NODE_FindClosestNode( NODEHEADER_PTR pNodeHeader, int nNodeType, CVector &position );
MAPNODE_PTR NODE_FindClosestGroundNode( userEntity_t *self );
MAPNODE_PTR NODE_FindClosestWaterNode( userEntity_t *self );
MAPNODE_PTR NODE_FindClosestAirNode( userEntity_t *self );
MAPNODE_PTR NODE_FindClosestTrackNode( userEntity_t *self );


MAPNODE_PTR NODE_AllocateMapNodes( int nNumNodes );
NODEHEADER_PTR NODE_AllocateMapHeader();
NODEHEADER_PTR NODE_Init( NODEHEADER_PTR pNodeHeader );
NODEHEADER_PTR NODE_Init( NODEHEADER_PTR pNodeHeader, int nNumNodes );
NODEHEADER_PTR NODE_Destroy( NODEHEADER_PTR pNodeHeader );
void NODE_LinkToAllPossibleNodes( NODEHEADER_PTR pNodeHeader, int nIndex );
int NODE_IsLink( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 );
void NODE_LinkInOneDirection( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 );
void NODE_LinkInBothDirection( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 );
int NODE_Find( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode );
MAPNODE_PTR NODE_Get( NODEHEADER_PTR pNodeHeader, short nNodeIndex );
int NODE_Add( NODEHEADER_PTR pNodeHeader, const CVector &point );
int NODE_Place( NODEHEADER_PTR pNodeHeader, const CVector &point, int nLastNodeIndex );
void NODE_Delete( NODEHEADER_PTR pNodeHeader, short nNodeIndex );
void NODE_Delete( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode );
void NODE_Move( MAPNODE_PTR pNode, const CVector &newPoint );

////////////////////////////////////////////////////////////////////////////
//	saving and restoring nodes
////////////////////////////////////////////////////////////////////////////

void NODE_InitNodeFileName();
void NODE_SetNodeFileName( char *szNewFileName );
char *NODE_GetNodeFileName();

void NODE_Write();
void NODE_WritePrevious( NODEHEADER_PTR pNodeHeader );
void NODE_Read();
NODEHEADER_PTR NODE_ReadPrevious( NODEHEADER_PTR pNodeHeader );

////////////////////////////////////////////////////////////////////////////
//	path table computation and io routines
////////////////////////////////////////////////////////////////////////////

void NODE_ComputePathTable( NODEHEADER_PTR pNodeHeader );
void NODE_WritePathTable( NODEHEADER_PTR pNodeHeader );
NODEHEADER_PTR NODE_ReadPathTable( NODEHEADER_PTR pNodeHeader );


int NODE_GetTotalNumNodesInSpace( const CVector &mins, const CVector &maxs );
int NODE_GetTotalNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices );
int NODE_GetGroundNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices );
int NODE_GetWaterNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices );
int NODE_GetAirNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices );
int NODE_GetTrackNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices );
MAPNODE_PTR	NODE_FurthestFrom (NODELIST_PTR pNodeList, const CVector &origin);

MAPNODE_PTR NODE_GetNodeFromTargetName( NODEHEADER_PTR pNodeHeader, char *targetName );
MAPNODE_PTR NODE_GetClosestCompleteHideNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy );
MAPNODE_PTR NODE_GetClosestPartialHideNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy );
int NODE_GetClosestHideNodes( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy, MAPNODE_PTR *pHideNodes );
MAPNODE_PTR NODE_GetClosestVisibleNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy );
MAPNODE_PTR NODE_GetClosestSnipeNode( userEntity_t *self );
int NODE_GetClosestSnipeNodes( userEntity_t *self, MAPNODE_PTR *pSnipeNodes );
MAPNODE_PTR NODE_GetClosestRetreatNode( userEntity_t *self );
int NODE_IsTriggered( MAPNODE_PTR pNode );
void NODE_SetTriggered( MAPNODE_PTR pNode );




//Nss Modified
MAPNODE_PTR NODE_FindClosestNodeForLanding( NODEHEADER_PTR pNodeHeader, int nNodeType, CVector &position );

////////////////////////////////////////////////////////////////////////////
//	routines for finding paths
////////////////////////////////////////////////////////////////////////////

// preallocation of the path nodes used in storing the path nodes
int PATH_PreAllocatePathNodes();
void PATH_DestroyPreAllocatedPathNodes();
PATHNODE_PTR PATH_GetFreePathNode();
void PATH_DeletePathNode( PATHNODE_PTR pNode );

PATHNODE_PTR path_last_node(PATHNODE_PTR path);
PATHNODE_PTR path_add_node( PATHNODE_PTR path, MAPNODE_PTR map_node );
PATHNODE_PTR path_delete_first( PATHNODE_PTR path );
PATHNODE_PTR path_kill( PATHNODE_PTR path );
int			 PATH_Length ( PATHNODE_PTR path );

void			PATHLIST_KillPath (PATHLIST_PTR pPathList);
PATHLIST_PTR	PATHLIST_Create (void);
PATHLIST_PTR	PATHLIST_Destroy (PATHLIST_PTR pPathList);
void			PATHLIST_DeleteFirstInPath (PATHLIST_PTR pPathList);
int				PATHLIST_AddNodeToPath (PATHLIST_PTR pPathList, MAPNODE_PTR pNode);

// path corner stuff
void pathcorner_Toggle( userEntity_t *self );
void pathcorner_Draw( userEntity_t *self );


//unix - DllExport
#if _MSC_VER
#define		DllExport	extern "C"	__declspec( dllexport )
#else
#define DllExport extern "C"
#endif

DllExport	void	node_node (userEntity_t *self);


#ifdef _DEBUG

MAPNODE_PTR NODE_GetNode( NODEHEADER_PTR pNodeHeader, int nIndex );
void NODE_EnableLayingNodes();
void NODE_DisableLayingNodes();
int NODE_IsOkToLayNodes();
char *NODE_GetTarget( MAPNODE_PTR pNode );
char *NODE_GetTargetName( MAPNODE_PTR pNode );
NODEDATA_PTR NODE_GetData( MAPNODE_PTR pNode );
NODEHEADER_PTR NODE_GetNodeHeader( userEntity_t *self );
NODEHEADER_PTR NODE_GetNodeHeaderAndType( userEntity_t *self, int &nNodeType );

#else _DEBUG

//-----------------------------------------------------------------------------
// inline functions defined below
//-----------------------------------------------------------------------------
#include "ai_utils.h"

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetNode
// Description:
//				get a node from a node header
// Input:
//				pNodeHeader => pointer to a node header
//				nIndex		=> index for the nodes
// Output:
//				MAPNODE_PTR		=> pointer to the node in a level
// Note:
//				this function gets used alot, should be inline
//
// ----------------------------------------------------------------------------
__inline MAPNODE_PTR NODE_GetNode( NODEHEADER_PTR pNodeHeader, int nIndex )
{
	if ( nIndex >= 0 && nIndex < pNodeHeader->nNumNodes )
	{
		return &(pNodeHeader->pNodes[nIndex]);
	}

	return NULL;
}

// ----------------------------------------------------------------------------

extern int bLayNodes;
__inline void NODE_EnableLayingNodes()
{
	bLayNodes = TRUE;
	
	AI_Dprintf( "Enabled Laying nodes.\n" );
}

__inline void NODE_DisableLayingNodes()
{
	bLayNodes = FALSE;

	AI_Dprintf( "Disabled Laying nodes.\n" );
}

__inline int NODE_IsOkToLayNodes()
{
	return bLayNodes;
}

// ----------------------------------------------------------------------------

__inline char *NODE_GetTarget( MAPNODE_PTR pNode )
{
	return pNode->target;
}

// ----------------------------------------------------------------------------

__inline char *NODE_GetTargetName( MAPNODE_PTR pNode )
{
	return pNode->targetname;
}

// ----------------------------------------------------------------------------

__inline NODEDATA_PTR NODE_GetData( MAPNODE_PTR pNode )
{
	return &(pNode->node_data);
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetNodeHeader
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline NODEHEADER_PTR NODE_GetNodeHeader( userEntity_t *self )
{
	NODEHEADER_PTR pNodeHeader = NULL;
//	int nNodeType = NODETYPE_GROUND;	// SCG[1/23/00]: not used

	switch ( self->movetype  )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		{
			pNodeHeader = pGroundNodes;
// SCG[1/23/00]: not used
/*
			if ( !self->groundEntity && self->waterlevel > 1 )
			{
				nNodeType = NODETYPE_WATER;
			}
*/
			break;
		}
		case MOVETYPE_SWIM:
		{
			pNodeHeader = pGroundNodes;
//			nNodeType = NODETYPE_WATER;	// SCG[1/23/00]: not used
			break;
		}
		case MOVETYPE_FLY:
		case MOVETYPE_TOSS:
		case MOVETYPE_HOVER:
		{
			pNodeHeader = pAirNodes;
//			nNodeType = NODETYPE_AIR;	// SCG[1/23/00]: not used
			break;
		}
		case MOVETYPE_TRACK:
		{
			pNodeHeader = pTrackNodes;
//			nNodeType = NODETYPE_TRACK;	// SCG[1/23/00]: not used
			break;
		}
		default:
		{
			_ASSERTE( FALSE );
			break;
		}
	}

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetNodeHeaderAndType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline NODEHEADER_PTR NODE_GetNodeHeaderAndType( userEntity_t *self, int &nNodeType )
{
	NODEHEADER_PTR pNodeHeader = NULL;
	nNodeType = NODETYPE_GROUND;

	switch ( self->movetype  )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		{
			pNodeHeader = pGroundNodes;
			if ( !self->groundEntity && self->waterlevel > 1 )
			{
				nNodeType = NODETYPE_WATER;
			}
			break;
		}
		case MOVETYPE_SWIM:
		{
			pNodeHeader = pGroundNodes;
			nNodeType = NODETYPE_WATER;
			break;
		}
		case MOVETYPE_FLY:
		case MOVETYPE_TOSS:
		case MOVETYPE_HOVER:
		{
			pNodeHeader = pAirNodes;
			nNodeType = NODETYPE_AIR;
			break;
		}
		case MOVETYPE_TRACK:
		{
			pNodeHeader = pTrackNodes;
			nNodeType = NODETYPE_TRACK;
			break;
		}
		default:
		{
			_ASSERTE( FALSE );
			break;
		}
	}

	return pNodeHeader;
}

#endif _DEBUG

#endif