// ==========================================================================
//
//  File:		NodeList.cpp
//  Contents:
//  Author:
//
// ==========================================================================
#include <stdio.h>
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "nodelist.h"
#include "GrphPath.h"
#include "ai_utils.h"
#include "ai_common.h"
#include "octree.h"
#include "PathTable.h"
#include "ai_func.h"

/* ************************** Signatures *********************************** */

#define	SIGNATURE_NONE				0
#define	SIGNATURE_GROUNDNODES		1
#define	SIGNATURE_GROUNDPATHTABLE	2
#define	SIGNATURE_AIRNODES			4
#define	SIGNATURE_AIRPATHTABLE		5
#define SIGNATURE_TRACKNODES		6
#define SIGNATURE_TRACKPATHTABLE	7
#define	SIGNATURE_OCTREE			8


const char	szNodeSignature[]			 = "NODES:";
const int	nNodeVersion				 = 0;

const char	szGroundNodeSignature[]		 = "GroundNodes:";
const int	nGroundNodeVersion			 = 0;

const char	szGroundPathTableSignature[] = "GroundPathTable:";
const int	nGroundPathTableVersion		 = 0;

const char	szAirNodeSignature[]		 = "AirNodes:";
const int	nAirNodeVersion = 0;

const char	szAirPathTableSignature[]	 = "AirPathTable:";
const int	nAirPathTableVersion		 = 0;

const char	szTrackNodeSignature[]		 = "TrackNodes:";
const int	nTrackNodeVersion = 0;

const char	szTrackPathTableSignature[]	 = "TrackPathTable:";
const int	nTrackPathTableVersion		 = 0;

const int nNodeDataVersion = 1;

/* ***************************** define types **************************** */

// maximum allowable distance between nodes
//#define		MAX_NODE_DIST			256.0f
#define		MAX_NODE_DIST			200.0f	// SCG[11/10/99]: 256 is the greatest distance nodes can be from each
											// SCG[11/10/99]: other and sometime causes an error when laying them in the game

#define		MAX_JUMP_HEIGHT			48.0f

#define		INITIAL_NUM_NODES		300
#define		INCREMENT_NUM_NODES		50
#define		NUM_VISIBLE_NODES		50
#define		MAX_VISIBLE_NODE_DIST	1024.0f

/* ***************************** Local Variables **************************** */

static	int	bShowNodes = FALSE;
static	int bShowGroundNodes = FALSE;
static	int bShowWaterNodes = FALSE;
static	int bShowAirNodes = FALSE;
static	int bShowTrackNodes = FALSE;
int bLayNodes = TRUE;

static userEntity_t	*pNodeEnts[NUM_VISIBLE_NODES];
static bool		bNodeEntsInitialized = FALSE;
static int		nNumVisibleNodes = 0;
static float	fLastNodeUpdate;


static char szNodeFileName[64] = { "\0" };

static int	g_nTotalBytes;
static int	g_nStartBytes;

/* ***************************** Local Functions **************************** */


/* ***************************** Global Variables *************************** */


// The following is used to keep track of two different kinds of nodes in Daikatana, ground 
// nodes for the ground based units and air nodes for those that can fly.
NODEHEADER_PTR pGroundNodes = NULL;	
NODEHEADER_PTR pAirNodes = NULL;
NODEHEADER_PTR pTrackNodes = NULL;

//*******************************************************************************
//	debugging functions 
//*******************************************************************************

#ifdef _DEBUG

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
MAPNODE_PTR NODE_GetNode( NODEHEADER_PTR pNodeHeader, int nIndex )
{
	if ( nIndex >= 0 && nIndex < pNodeHeader->nNumNodes )
	{
		return &(pNodeHeader->pNodes[nIndex]);
	}

	return NULL;
}

// ----------------------------------------------------------------------------

void NODE_EnableLayingNodes()
{
	bLayNodes = TRUE;
	
	AI_Dprintf( "Enabled Laying nodes.\n" );
}

void NODE_DisableLayingNodes()
{
	bLayNodes = FALSE;

	AI_Dprintf( "Disabled Laying nodes.\n" );
}

int NODE_IsOkToLayNodes()
{
	return bLayNodes;
}

// ----------------------------------------------------------------------------

char *NODE_GetTarget( MAPNODE_PTR pNode )
{
	return pNode->target;
}

// ----------------------------------------------------------------------------

char *NODE_GetTargetName( MAPNODE_PTR pNode )
{
	return pNode->targetname;
}

// ----------------------------------------------------------------------------

NODEDATA_PTR NODE_GetData( MAPNODE_PTR pNode )
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
NODEHEADER_PTR NODE_GetNodeHeader( userEntity_t *self )
{
	NODEHEADER_PTR pNodeHeader = NULL;
	int nNodeType = NODETYPE_GROUND;

	switch ( self->movetype  )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		case MOVETYPE_BOUNCE:
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
		case MOVETYPE_BOUNCEMISSILE:
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
			// NSS[12/13/99]:Fixed... this is an entity setting its movetype none
			// for a special case where they do not want to be moving(i.e. Rotworm hanging from the ceiling).
			pNodeHeader = pGroundNodes;
			if ( !self->groundEntity && self->waterlevel > 1 )
			{
				nNodeType = NODETYPE_WATER;
			}
			//_ASSERTE( FALSE );
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
NODEHEADER_PTR NODE_GetNodeHeaderAndType( userEntity_t *self, int &nNodeType )
{
	NODEHEADER_PTR pNodeHeader = NULL;
	nNodeType = NODETYPE_GROUND;

	switch ( self->movetype  )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		case MOVETYPE_BOUNCE:
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
			// NSS[12/13/99]:Fixed... this is an entity setting its movetype none
			// for a special case where they do not want to be moving(i.e. Rotworm hanging from the ceiling).
			// FIXME: need to find the real reason for this assert
			pNodeHeader = pGroundNodes;
			if ( !self->groundEntity && self->waterlevel > 1 )
			{
				nNodeType = NODETYPE_WATER;
			}
			//_ASSERTE( FALSE );
			break;
		}
	}

	return pNodeHeader;
}

#endif _DEBUG

// ----------------------------------------------------------------------------
//
// Name:		node_marker
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#ifdef _DEBUG
static void node_marker( MAPNODE_PTR pNode )
{
	_ASSERTE( pNode );

	userEntity_t	*temp;

	if ( !bShowNodes || pNode->marker )
	{
		return;
	}

	pNode->marker = temp = gstate->SpawnEntity();

	temp->s.modelindex = gstate->ModelIndex( debug_models [DEBUG_NODE] );
	gstate->SetSize(temp, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2(temp, pNode->position);

	temp->solid = SOLID_NOT;
	temp->clipmask = 0;
	temp->movetype = MOVETYPE_FLY;
	temp->className = "node_marker";
	
	temp->s.render_scale.Set(1.0, 1.0, 1.0);
//	com->SetVector (temp->avelocity, 90, 90, 90);

	temp->delay = 1.05;

//	temp->think = node_marker_think;
//	temp->nextthink = gstate->time + rnd () * 0.25;
}
#endif

// ----------------------------------------------------------------------------
//
// Name:		node_show_links
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void node_show_links( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = NULL;

	if ( !bShowNodes )
	{
		return;
	}

	if ( self->input_entity )
	{
		hook = AI_GetPlayerHook( self->input_entity );
	}
	else
	{
		hook = AI_GetPlayerHook( self );
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );
	MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );

	if ( self->movetype == MOVETYPE_NOCLIP && pAirNodes )
	{
        // find air nodes when no clipping
        pNodeHeader = pAirNodes;
        pCurrentNode = NODE_FindClosestNode( pAirNodes, NODETYPE_AIR, self->s.origin );
        if ( !pCurrentNode )
        {
            pNodeHeader = pNodeList->pNodeHeader;
            pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
        }
    }

    if ( pCurrentNode )
    {
	    for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
	    {
		    MAPNODE_PTR pLinkNode = &(pNodeHeader->pNodes[pCurrentNode->aLinks[i].nIndex]);
		    _ASSERTE( pLinkNode );

		    com->DrawLine( self, pCurrentNode->position, pLinkNode->position, 10 );
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		node_toggle_nodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#ifdef _DEBUG
void node_toggle_nodes( userEntity_t *self )
{
	_ASSERTE( self );

	int i = 0;

	if ( bShowNodes )
	{
		bShowNodes = FALSE;
		fLastNodeUpdate = 0.0f;
		nNumVisibleNodes = 0;
		self->flags &= ~FL_DEBUG;	// disallows showing node links
		self->flags |= FL_NOSAVE;	// SCG[11/26/99]: Let's not save these

		// hide all of the visible nodes
		if ( bNodeEntsInitialized )
		{
			for ( i = 0; i < NUM_VISIBLE_NODES; i++ )
			{
				if ( pNodeEnts[i] )
				{
					pNodeEnts[i]->svflags |= SVF_NOCLIENT;
					pNodeEnts[i]->flags |= FL_NOSAVE;
					pNodeEnts[i]->owner = NULL;
				}
			}
		}
	}
	else
	{
		bShowNodes = TRUE;
		self->flags |= FL_DEBUG;	// allows showing node links
		self->flags |= FL_NOSAVE;	// SCG[11/26/99]: Let's not save these

		if ( !bNodeEntsInitialized )
		{
			fLastNodeUpdate = 0.0f;
			nNumVisibleNodes = 0;
			bNodeEntsInitialized = TRUE;

			for ( i = 0; i < NUM_VISIBLE_NODES; i++ )
			{
				pNodeEnts[i] = gstate->SpawnEntity();

				_ASSERTE ( pNodeEnts[i] );

				pNodeEnts[i]->s.modelindex  = gstate->ModelIndex( debug_models [DEBUG_NODE] );
				pNodeEnts[i]->solid			= SOLID_NOT;
				pNodeEnts[i]->clipmask		= 0;
				pNodeEnts[i]->movetype		= MOVETYPE_NONE;
				pNodeEnts[i]->className		= "node_marker";
				pNodeEnts[i]->s.render_scale.Set(1.0, 1.0, 1.0);
				pNodeEnts[i]->delay			= 1.05;							// FIXME: is this needed?
				pNodeEnts[i]->s.renderfx	= 0; //RF_TRANSLUCENT;
				pNodeEnts[i]->s.effects		= 0; //EF_ROTATE;
				pNodeEnts[i]->s.alpha		= 0.0f;
				pNodeEnts[i]->svflags		|= SVF_NOCLIENT;
				pNodeEnts[i]->owner			= NULL;
				pNodeEnts[i]->flags |= FL_NOSAVE;
				gstate->SetSize( pNodeEnts[i], 0, 0, 0, 0, 0, 0 );
			}
		}
	}
}
#endif

// ----------------------------------------------------------------------------
//
// Name:		node_add_visible_node
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void node_add_visible_node( userEntity_t *self, MAPNODE_PTR pNode )
{
//	int i = 0;// SCG[1/23/00]: not used

	float fDistance = VectorDistance( pNode->position, self->s.origin );
	if ( fDistance <= MAX_VISIBLE_NODE_DIST )
	{
		if ( nNumVisibleNodes < NUM_VISIBLE_NODES )
		{
			int idx = nNumVisibleNodes;

			// not visible/used ?
			pNodeEnts[idx]->svflags		&= ~SVF_NOCLIENT;
			pNodeEnts[idx]->s.origin	= pNode->position;
			pNodeEnts[idx]->s.old_origin = pNode->position;
			pNodeEnts[idx]->s.event		= EV_ENTITY_FORCEMOVE;		// make sure we don't lerp to our new pos
			pNodeEnts[idx]->owner		= (userEntity_t*) pNode;		// hack

			gstate->LinkEntity( pNodeEnts[idx] );

			pNode->marker = pNodeEnts[idx];

			nNumVisibleNodes++;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		node_update_visible_nodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void node_update_visible_nodes( userEntity_t *self )
{
	int i = 0;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	NODELIST_PTR	pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR	pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	if ( bShowNodes && ((gstate->time - fLastNodeUpdate) > 0.5f))
	{
		// remove all the current ones
		for ( i = 0; i < NUM_VISIBLE_NODES; i++ )
		{
			if ( pNodeEnts[i] )
			{
				pNodeEnts[i]->svflags |= SVF_NOCLIENT;
				MAPNODE_PTR pNode = (MAPNODE_PTR) pNodeEnts[i]->owner;
				if (pNode)
				{
					pNode->marker = NULL;
				}
				pNodeEnts[i]->owner = NULL;
			}
		}
		nNumVisibleNodes = 0;
		
		// start with the air nodes since there are a lot less of 
        // these than the ground nodes, usually
        if ( pAirNodes )
		{
			int nMaxAllowedVisibleAirNodes = (int)(NUM_VISIBLE_NODES * 0.4f);

            for ( int i = 0; i < pAirNodes->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pAirNodes->pNodes[i]);
				_ASSERTE( pNode );
				node_add_visible_node( self, pNode );

                if ( nNumVisibleNodes > nMaxAllowedVisibleAirNodes )
                {
                    break;
                }
			}
		}

		if ( pTrackNodes )
		{
			for ( int i = 0; i < pTrackNodes->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pTrackNodes->pNodes[i]);
				_ASSERTE( pNode );
				node_add_visible_node( self, pNode );
			}
		}

		// add the new ones
		if ( pGroundNodes )
		{
			for ( int i = 0; i < pGroundNodes->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pGroundNodes->pNodes[i]);
				_ASSERTE( pNode );
				node_add_visible_node( self, pNode );
			}
		}

	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ToggleGround
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_ToggleGround()
{
}

//*******************************************************************************
//	node list management functions 
//*******************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		node_create_nodelist
// Description:
//				creates a new list pointing to header
// Input:
// Output:
// Note:
//	one of these is created for each entity
//	who needs to follow the nodes in the list
//	pointed to by a header
//
// ----------------------------------------------------------------------------
NODELIST_PTR node_create_nodelist( NODEHEADER_PTR pNodeHeader )
{
	NODELIST_PTR pNewList;

//	pNewList = new NODELIST;

	pNewList = (NODELIST_PTR)gstate->X_Malloc(sizeof(NODELIST),MEM_TAG_NODE);
	pNewList->pNodeHeader  = pNodeHeader;
	pNewList->nCurrentNodeIndex = -1;
	pNewList->nLastNodeIndex = -1;

	return pNewList;
}

//*******************************************************************************
//	node list building functions 
//*******************************************************************************

//*******************************************************************************
//	node navigation
//*******************************************************************************


// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestNode
// Description:
//				returns the node that is closest
//				to and within dist units of origin
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestNode( userEntity_t *self, CVector &destPoint )
{
	NODEHEADER_PTR pNodeHeader = NULL;
	int nNodeType = NODETYPE_GROUND;

	switch ( self->movetype  )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_NONE:
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
		case MOVETYPE_BOUNCEMISSILE:
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
	
	if ( pNodeHeader )
	{
		return NODE_FindClosestNode( pNodeHeader, nNodeType, destPoint );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestNode
// Description:
//				returns the node that is closest
//				to and within dist units of origin
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestNode( userEntity_t *self )
{
	return NODE_GetClosestNode( self, self->s.origin );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetCurrentNodeHeader
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_GetCurrentNodeHeader( userEntity_t *self )
{
	NODEHEADER_PTR pNodeHeader = NULL;
	int nNodeType = NODETYPE_GROUND;

	switch ( self->movetype )
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

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestNodeToEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestNodeToEntity( userEntity_t *self, userEntity_t *destEntity )
{
	int nMoveType = self->movetype;

	MAPNODE_PTR pDestNode = NULL;
	switch ( nMoveType )
	{
		case MOVETYPE_WALK:
		case MOVETYPE_WHEEL:
		case MOVETYPE_HOP:
		case MOVETYPE_NONE:	// SCG[11/5/99]: um. will this work?
		{
			if ( !destEntity->groundEntity && destEntity->waterlevel > 1 )
			{
				pDestNode = NODE_FindClosestWaterNode( destEntity );
			}
			else
			{
				pDestNode = NODE_FindClosestGroundNode( destEntity );
			}
			break;
		}
		case MOVETYPE_SWIM:
		{
			pDestNode = NODE_FindClosestWaterNode( destEntity );
			break;
		}
		case MOVETYPE_FLY:
		case MOVETYPE_TOSS:
		case MOVETYPE_HOVER:
		{
			pDestNode = NODE_FindClosestAirNode( destEntity );
			break;
		}
		case MOVETYPE_TRACK:
		{
			pDestNode = NODE_FindClosestTrackNode( destEntity );
			break;
		}
		default:
		{
			_ASSERTE( FALSE );
			break;
		}
	}

	return pDestNode;
}

//*****************************************************************************
//*****************************************************************************
//
//*****************************************************************************
//*****************************************************************************



// ----------------------------------------------------------------------------
//
// Name:		NODE_DestroyPathTable
// Description:
//				free the memory used to store the path table
// Input:
//				pNodeHeader	=> pointer to the node header
// Output:
// Note:
//				this function should be called only once per level at exit
//
// ----------------------------------------------------------------------------
static
void NODE_DestroyPathTable( NODEHEADER_PTR pNodeHeader )
{
	if ( pNodeHeader && pNodeHeader->pPathTable )
	{
		for ( int i = 0; i < pNodeHeader->nNumPathTableNodes; i++ )
		{
			//delete pNodeHeader->pPathTable[i];
			gstate->X_Free(pNodeHeader->pPathTable[i]);
		}

		//delete [] pNodeHeader->pPathTable;
		gstate->X_Free(pNodeHeader->pPathTable);
		pNodeHeader->pPathTable = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_AllocatePathTable
// Description:
//				allocate memory for the path table
// Input:
//				pNodeHeader	=> pointer to the node header
// Output:
//				TRUE	=> if mmory is allocated successfully
//				FALSE	=> allocation failed
// Note:
//				this function should only be called once per level at start up
//
// ----------------------------------------------------------------------------
int NODE_AllocatePathTable( NODEHEADER_PTR pNodeHeader )
{
	NODE_DestroyPathTable( pNodeHeader );

	// create a two dimensional array to hold path table info
	pNodeHeader->pPathTable = (short**)gstate->X_Malloc(sizeof(short*)*pNodeHeader->nNumNodes,MEM_TAG_NODE);
//	pNodeHeader->pPathTable = new short*[pNodeHeader->nNumNodes];
//	if ( !pNodeHeader->pPathTable )
//	{
//		com->Error( "Could not allocate memory for path table.\n" );
//		return FALSE;
//	}

	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		pNodeHeader->pPathTable[i] = (short*)gstate->X_Malloc(sizeof(short)*pNodeHeader->nNumNodes,MEM_TAG_NODE);
//		pNodeHeader->pPathTable[i] = new short[pNodeHeader->nNumNodes];
//		if ( !pNodeHeader->pPathTable[i] )
//		{
//			com->Error( "Could not allocate memory for path table.\n" );
//			return FALSE;
//		}
	}
	pNodeHeader->nNumPathTableNodes = pNodeHeader->nNumNodes;
	
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_AllocateMapNodes
// Description:
//				allocate memory for map nodes in a level
// Input:
//				nNumNodes		=> number of nodes to allocate
// Output:
//				MAPNODE_PTR		=> pointer to the allocated map nodes
//				NULL			=> if failed to allcate
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_AllocateMapNodes( int nNumNodes )
{
	if ( nNumNodes == 0 )
	{
		return NULL;
	}

	MAPNODE_PTR pNodes = NULL;
//	int nSize = nNumNodes * sizeof(MAPNODE);// SCG[1/23/00]: not used
	pNodes = (MAPNODE_PTR)gstate->X_Malloc(sizeof(MAPNODE)*nNumNodes,MEM_TAG_NODE);

//	pNodes = new MAPNODE[nNumNodes];
//	if ( !pNodes )
//	{
//		com->Error( "Error Allocating path nodes." );
//		return NULL;
//	}
//	memset( pNodes, 0x00, nSize );

/*
	for ( int i = 0; i < nNumNodes; i++ )
	{
		//	malloc memory for time stamps
//		pNodes[i].node_time = new(MEM_MALLOC) nodeTime_t;
		if ( !pNodes[i].node_time )
		{
			com->Error( "Error Allocating path node's node time." );
			return NULL;
		}	
		memset(pNodes[i].node_time, 0x00, sizeof(nodeTime_t));
	}
*/
	return pNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_AllocateMapHeader
// Description:
//				allcate memory for a map header
// Input:
// Output:
//				NODEHEADER_PTR	=> pointer node header 
//				NULL			=> if failed to allocate memory
// Note:
//				this function should be called to initialize for each different 
//				types of node header in a level
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_AllocateMapHeader()
{
	NODEHEADER_PTR pNodeHeader = NULL;

	pNodeHeader = (NODEHEADER_PTR)gstate->X_Malloc(sizeof(NODEHEADER),MEM_TAG_NODE);
//	pNodeHeader = new NODEHEADER;
//	if ( !pNodeHeader )
//	{
//		com->Error( "Error initializing NODE." );
//		return NULL;
//	}
		
	pNodeHeader->bUsePathTable	= FALSE;
	pNodeHeader->pPathTable		= NULL;
	pNodeHeader->nNumPathTableNodes = 0;
	pNodeHeader->nNumNodes		= 0;
	pNodeHeader->nTotalNumNodes = INITIAL_NUM_NODES;
	pNodeHeader->pNodes			= NULL;

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Init
// Description:
//				initializes the node information, allocating memory as necessary
// Input:
//				pNodeHeader		=> pointer to the node header
// Output:
//				NODEHEADER_PTR	=> successfully allocated and initialized node
//									header
//				NULL			=> if initialization failed
// Note:
//				this function should be called once per level
//				NODE_Destroy() should be called to deallocate what this function
//				allocated
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_Init( NODEHEADER_PTR pNodeHeader )
{
	// just in case
	pNodeHeader = NODE_Destroy( pNodeHeader );
	
	pNodeHeader = NODE_AllocateMapHeader();
	if ( !pNodeHeader )
	{
		return NULL;
	}

	if ( pNodeHeader->nTotalNumNodes > 0 )
	{
		pNodeHeader->pNodes = NODE_AllocateMapNodes( pNodeHeader->nTotalNumNodes );
		if ( !pNodeHeader->pNodes )
		{
			return NULL;
		}
	}
	else
	{
		pNodeHeader->pNodes = NULL;
	}

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Init
// Description:
//				initializes the node information, allocating memory as necessary
// Input:
//				pNodeHeader		=> pointer to the node header
// Output:
//				NODEHEADER_PTR	=> successfully allocated and initialized node
//									header
//				NULL			=> if initialization failed
// Note:
//				this function should be called once per level
//				NODE_Destroy() should be called to deallocate what this function
//				allocated
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_Init( NODEHEADER_PTR pNodeHeader, int nNumNodes )
{
	// just in case
	pNodeHeader = NODE_Destroy( pNodeHeader );
	
	pNodeHeader = NODE_AllocateMapHeader();
	if ( !pNodeHeader )
	{
		return NULL;
	}

	if ( nNumNodes > 0 )
	{
		pNodeHeader->pNodes = NODE_AllocateMapNodes( nNumNodes );
		if ( !pNodeHeader->pNodes )
		{
			return NULL;
		}
	}
	else
	{
		pNodeHeader->pNodes = NULL;
	}

	pNodeHeader->nTotalNumNodes = nNumNodes;

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Destroy
// Description:
//				destroy a node header
// Input:
//				pNodeHeader		=> pointer to a node header to be destroyed
// Output:
//				NULL
// Note:
//				call this function to detroy a node header once per level
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_Destroy( NODEHEADER_PTR pNodeHeader )
{
	if ( pNodeHeader )
	{
		if ( pNodeHeader->pNodes )
		{
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				if ( pNodeHeader->pNodes[i].target )
				{
					//delete pNodeHeader->pNodes[i].target;
					gstate->X_Free(pNodeHeader->pNodes[i].target);
				}

				if ( pNodeHeader->pNodes[i].targetname )
				{
					//delete pNodeHeader->pNodes[i].targetname;
					gstate->X_Free(pNodeHeader->pNodes[i].targetname);
				}
			}
			
			//delete [] pNodeHeader->pNodes;
			gstate->X_Free(pNodeHeader->pNodes);
			pNodeHeader->pNodes = NULL;
		}

		NODE_DestroyPathTable( pNodeHeader );

		//delete pNodeHeader;
		gstate->X_Free(pNodeHeader);
		pNodeHeader = NULL;
	}

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_LinkToAllPossibleNodes
// Description:
//				Link a node to all visible nodes within a certain distance
// Input:
//				pNodeHeader		=> pointer to node header
//				nIndex			=> index into the node list
// Output:
// Note:
//				not yet finished
//
// ----------------------------------------------------------------------------
void NODE_LinkToAllPossibleNodes( NODEHEADER_PTR pNodeHeader, int nIndex )
{
	int nTotalNumNodes = pNodeHeader->nNumNodes;
	for ( int i = 0; i < nTotalNumNodes; i++ )
	{
		if ( i == nIndex )
		{
			continue;
		}

		MAPNODE_PTR pPathNode = &(pNodeHeader->pNodes[i]);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_IsLink
// Description:
//				Determine if there is a link between node1 to node2
// Input:
//				pNodeHeader	=> pointer to a node header
//				nNodeIndex1 => first node's index 
//				nNodeIndex2	=> second node's index
// Output:
//				TRUE	=> if there is a link
//				FALSE	=> otherwise	
// Note:
//
// ----------------------------------------------------------------------------
int NODE_IsLink( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 )
{
	_ASSERTE( nNodeIndex1 >= 0 && nNodeIndex1 < pNodeHeader->nNumNodes );
	_ASSERTE( nNodeIndex2 >= 0 && nNodeIndex2 < pNodeHeader->nNumNodes );

	MAPNODE_PTR pNode1 = &(pNodeHeader->pNodes[nNodeIndex1]);
	_ASSERTE( pNode1 );

	for ( int i = 0; i < pNode1->nNumLinks; i++ )
	{
		if ( pNode1->aLinks[i].nIndex == nNodeIndex2 )
		{
			return 1;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_LinkInOneDirection
// Description:
//				Link two nodes in one direction from the first node to the second
// Input:
//				pNodeHeader		=> pointer to a node header
//				nNodeIndex1		=> first node's index
//				nNodeIndex2		=> second node's index
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_LinkInOneDirection( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 )
{
	if ( NODE_IsOkToLayNodes() == FALSE )
	{
		return;
	}

	_ASSERTE( nNodeIndex1 >= 0 && nNodeIndex1 < pNodeHeader->nNumNodes );
	_ASSERTE( nNodeIndex2 >= 0 && nNodeIndex2 < pNodeHeader->nNumNodes );
	MAPNODE_PTR pNode1 = &(pNodeHeader->pNodes[nNodeIndex1]);
	_ASSERTE( pNode1 );

	if ( pNode1->nNumLinks < MAX_NODE_LINKS && !NODE_IsLink( pNodeHeader, nNodeIndex1, nNodeIndex2) )
	{
		MAPNODE_PTR pNode2 = &(pNodeHeader->pNodes[nNodeIndex2]);
		_ASSERTE( pNode2 );
		
		tr = gstate->TraceLine_q2( pNode1->position, pNode2->position, NULL, MASK_SOLID );
		if ( tr.fraction >= 0.9f )
		{
			pNode1->aLinks[pNode1->nNumLinks].nDistance = (short)VectorDistance( pNode1->position, pNode2->position );
			pNode1->aLinks[pNode1->nNumLinks].nIndex = nNodeIndex2;
			pNode1->nNumLinks++;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_LinkInBothDirection
// Description:
//				Link two nodes in both directions
// Input:
//				pNodeHeader		=> pointer to a node header
//				nNodeIndex1		=> first node's index
//				nNodeIndex2		=> second node's index
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_LinkInBothDirection( NODEHEADER_PTR pNodeHeader, short nNodeIndex1, short nNodeIndex2 )
{
	NODE_LinkInOneDirection( pNodeHeader, nNodeIndex1, nNodeIndex2 );
	NODE_LinkInOneDirection( pNodeHeader, nNodeIndex2, nNodeIndex1 );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Find
// Description:
//				Find a matching map node in a node header
// Input:
//				pNodeHeader	=> pointer to a node header
//				pNode		=> pointer to a map node that need to be searched in
//							   the node header
// Output:
//				>= 0	=> if found
//				-1		=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int NODE_Find( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode )
{
	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	_ASSERTE( pNodes );

	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		if ( &(pNodes[i]) == pNode )
		{
			return i;
		}
	}
	
	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Add
// Description:
//				Add a node to a node header
// Input:
//				pNodeHeader		=> pointer to a node header
//				CVector			=> vector array (x,y,z)
// Output:
//				int				=> index of the added node
// Note:
//
// ----------------------------------------------------------------------------
int NODE_Add( NODEHEADER_PTR pNodeHeader, const CVector &point )
{
	_ASSERTE( pNodeHeader );
	if ( pNodeHeader->nNumNodes >= pNodeHeader->nTotalNumNodes )
	{
		int nPrevNumNodes = pNodeHeader->nTotalNumNodes;

		pNodeHeader->nTotalNumNodes += INCREMENT_NUM_NODES;
		MAPNODE_PTR pNewNodes = NODE_AllocateMapNodes( pNodeHeader->nTotalNumNodes );

		int nSize = pNodeHeader->nTotalNumNodes * sizeof(MAPNODE);
		for ( int i = 0; i < pNodeHeader->nTotalNumNodes; i++ )
		{
			pNewNodes[i].position.Zero();

			pNewNodes[i].nIndex		= -1;
			pNewNodes[i].nNumLinks	= 0;
			memset( &(pNewNodes[i].aLinks), 0x00, sizeof(LINK) * MAX_NODE_LINKS );

			pNewNodes[i].node_type	= NODETYPE_GROUND;
			memset( &(pNewNodes[i].node_data), 0x00, sizeof(NODEDATA) );

			pNewNodes[i].target		= NULL;
			pNewNodes[i].targetname	= NULL;
			pNewNodes[i].pUserHook	= NULL;
		}

		// copy the data to the new allocated nodes
		for ( i = 0; i < nPrevNumNodes; i++ )
		{
			memcpy( &(pNewNodes[i]), &(pNodeHeader->pNodes[i]), sizeof(MAPNODE) );
			//memcpy( pNewNodes[i].node_time, pNodeHeader->pNodes[i].node_time, sizeof(nodeTime_t));
		}

		// time to delete the old nodes
		//delete [] pNodeHeader->pNodes;
		gstate->X_Free(pNodeHeader->pNodes);

		// set the newly allocated data to the old one
		pNodeHeader->pNodes = pNewNodes;
	}

	MAPNODE_PTR pNode = &(pNodeHeader->pNodes[pNodeHeader->nNumNodes]);
	_ASSERTE( pNode );
	pNode->position = point;

	pNode->nIndex = pNodeHeader->nNumNodes;
	pNode->nNumLinks = 0;
	pNode->node_type = NODETYPE_GROUND;		// all nodes added this way are ground nodes 

	// increment the node counter
	pNodeHeader->nNumNodes++;

	// show node if need to
#ifdef _DEBUG
	node_marker( pNode );
#endif

	AI_Dprintf( "Adding %dth Node.\n", pNode->nIndex );

	// just added a path node, go back to using the dynamic path finding code 
	// instead of the table
	if ( pNodeHeader->bUsePathTable )
	{
		pNodeHeader->bUsePathTable = FALSE;
		AI_Dprintf( "No longer using Path Table.\n" );
	}

	return pNode->nIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Place
// Description:
//				Place a node in a level, linking with the last node as needed
// Input:
//				pNodeHeader	=> pointer to a node header
//				point		=> x,y,z coordinate for a node
//				pLastNode	=> pointer to a MAPNODE that the current node should
//							   be linked
// Output:
//				int			=> index of the placed node
// Note:
//
// ----------------------------------------------------------------------------
int NODE_Place( NODEHEADER_PTR pNodeHeader, const CVector &point, int nLastNodeIndex )
{
	_ASSERTE( pNodeHeader );
	int nNewNodeIndex = NODE_Add( pNodeHeader, point );

	MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, nLastNodeIndex );
	if ( pLastNode )
	{
		//if ( VectorDistance(pNodeHeader->pNodes[nNewNodeIndex].position, pLastNode->position) > MAX_NODE_DIST )
		if ( fabs(pLastNode->position.z - pNodeHeader->pNodes[nNewNodeIndex].position.z) > MAX_JUMP_HEIGHT ) 
		{
			NODE_LinkInOneDirection( pNodeHeader, pLastNode->nIndex, nNewNodeIndex );
		}
		else 
		{
			NODE_LinkInBothDirection( pNodeHeader, nNewNodeIndex, pLastNode->nIndex );
		}
	}

	return nNewNodeIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Delete
// Description:
//				delete a node
// Input:
//				pNodeHeader	=> pointer the a node header
//				nNodeIndex	=> the node's index that needs to be deleted
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_Delete( NODEHEADER_PTR pNodeHeader, short nNodeIndex )
{
	_ASSERTE( pNodeHeader );
	_ASSERTE( nNodeIndex >= 0 && nNodeIndex < pNodeHeader->nNumNodes );

	if ( pNodeHeader->nNumNodes > 0 )
	{
		for ( int i = nNodeIndex; i < pNodeHeader->nNumNodes-1; i++ )
		{
			memcpy( &(pNodeHeader->pNodes[i]), &(pNodeHeader->pNodes[i+1]), sizeof(MAPNODE) );
			//memcpy( pNodeHeader->pNodes[i].node_time, pNodeHeader->pNodes[i+1].node_time, sizeof(nodeTime_t));
		}

		pNodeHeader->nNumNodes--;

		memset( &(pNodeHeader->pNodes[pNodeHeader->nNumNodes]), 0, sizeof(MAPNODE) );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Delete
// Description:
//				delete a node
// Input:
//				pNodeHeader	=> pointer to a node header
//				pNode		=> pointer to a map node that is to be deleted	
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_Delete( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode )
{
	_ASSERTE( pNodeHeader );
	int nNodeIndex = NODE_Find( pNodeHeader, pNode );
	if ( nNodeIndex >= 0 && nNodeIndex < pNodeHeader->nNumNodes )
	{
		NODE_Delete( pNodeHeader, nNodeIndex );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Move
// Description:
//				move a node to a new location
// Input:
//				pNode		=> pointer to a map node that is to be moved
//				newPoint	=> new location
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_Move( MAPNODE_PTR pNode, const CVector &newPoint )
{
	_ASSERTE( pNode );
	pNode->position = newPoint;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_FixNodeLinks
// Description:
//				Attempt to fix links between nodes
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_FixNodeLinks( NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	AI_Dprintf( "Attempting to fix node links......\n" );
	
	MAPNODE_PTR pCurrentNode = NULL;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode1 = NODE_GetNode( pNodeHeader, i );
		_ASSERTE( pNode1 );
	
		for ( int j = 0; j < pNodeHeader->nNumNodes; j++ )
		{
			if ( i == j )
			{
				continue;
			}

			MAPNODE_PTR pNode2 = NODE_GetNode( pNodeHeader, j );
			_ASSERTE( pNode2 );

			float fDistance = VectorDistance( pNode1->position, pNode2->position );
			if ( fDistance <= MAX_NODE_DIST && !NODE_IsLink( pNodeHeader, i, j ) && 
				 fabs(pNode1->position.z - pNode2->position.z) < MAX_JUMP_HEIGHT )
			{
				gstate->TraceLine( pNode1->position, pNode2->position, TRUE, NULL, &trace );
				if ( trace.fraction >= 0.9 )
				{
					NODE_LinkInOneDirection( pNodeHeader, i, j );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ComputePathTable
// Description:
//				compute a new path table for a node header
// Input:
//				pNodeHeader	=> pointer to a node header
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_ComputePathTable( NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	if ( NODE_AllocatePathTable( pNodeHeader ) )
	{
		AI_Dprintf( "Computing Path Table.............\n" );

#ifdef _DEBUG
		int time1 = gstate->Sys_Milliseconds();
#endif _DEBUG

		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode1 = NODE_GetNode( pNodeHeader, i );
			_ASSERTE( pNode1 );

			for ( int j = 0; j < pNodeHeader->nNumNodes; j++ )
			{
				if ( i == j )
				{
					// set it to itself
					pNodeHeader->pPathTable[i][i] = i;
					continue;
				}
			
				MAPNODE_PTR pNode2 = NODE_GetNode( pNodeHeader, j );
				_ASSERTE( pNode2 );

				short nFirstIndex = PATH_ComputePath( pNodeHeader, pNode1, pNode2 );

				// store the index
				_ASSERTE( nFirstIndex != i );
				pNodeHeader->pPathTable[i][j] = nFirstIndex;
			}

            PATHTABLE_ContinueThread();
		}
		
		pNodeHeader->nNumPathTableNodes = pNodeHeader->nNumNodes;
		pNodeHeader->bUsePathTable = TRUE;

#ifdef _DEBUG
		int timeDiff = gstate->Sys_Milliseconds() - time1;
		AI_Dprintf( "Computing Path Table took = %d miliseconds.\n", timeDiff );
#endif _DEBUG

		AI_Dprintf( "Using Path Table.\n" );
	}
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

void NODE_InitNodeFileName()
{
    szNodeFileName[0] = '\0';
}

// ----------------------------------------------------------------------------

void NODE_SetNodeFileName( char *szNewFileName )
{
    szNodeFileName[0] = '\0';

    if ( strlen( szNewFileName ) > 0 )
    {
        strcpy( szNodeFileName, szNewFileName );
    }
}

// ----------------------------------------------------------------------------

char *NODE_GetNodeFileName()
{
    return szNodeFileName;
}


// ----------------------------------------------------------------------------
//
// Name:		NODE_WritePathTable
// Description:
//				write out the path table to disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WritePathTable( FILE *f, NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	if ( pNodeHeader->bUsePathTable && pNodeHeader->pPathTable )
	{
		fwrite( &pNodeHeader->nNumPathTableNodes, sizeof(pNodeHeader->nNumPathTableNodes), 1, f );

		short **pPathTable = pNodeHeader->pPathTable;

		for ( int i = 0; i < pNodeHeader->nNumPathTableNodes; i++ )
		{
			fwrite( pPathTable[i], pNodeHeader->nNumPathTableNodes * sizeof(pPathTable[i][0]), 1, f );
		}
		
		AI_Dprintf( "Wrote path table.\n" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WritePathTable
// Description:
//				write out the path table to disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WritePathTable( NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	FILE	*f;
	char	base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, gstate->mapName );
	strcat( base_name, ".ptb" );

	if ( !(f = fopen (base_name, "wb")) )
	{
		AI_Dprintf( "Unable to create path table file %s.\n", base_name );
		return;
	}

	AI_Dprintf ("mapname = %s\n", gstate->mapName);

	NODE_WritePathTable( f, pNodeHeader );

	fclose (f);
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ReadPathTable
// Description:
//				read a path table for a node header
// Input:
//				pNodeHeader	=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to a node header with path table read in
//				NULL			=> if error occurred
// Note:
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_ReadPathTable( FILE *f, NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );
		
	int nVersion;
//	fread( &nVersion, sizeof(nVersion), 1, f );
	gstate->FS_Read( &nVersion, sizeof(nVersion), f );
	switch ( nVersion )
	{
		case 0:
		{
//			fread( &pNodeHeader->nNumPathTableNodes, sizeof(pNodeHeader->nNumPathTableNodes), 1, f );
			gstate->FS_Read( &pNodeHeader->nNumPathTableNodes, sizeof(pNodeHeader->nNumPathTableNodes), f );
			pNodeHeader->nNumNodes = pNodeHeader->nNumPathTableNodes;

			NODE_AllocatePathTable( pNodeHeader );

			short **pPathTable = pNodeHeader->pPathTable;
			for ( int i = 0; i < pNodeHeader->nNumPathTableNodes; i++ )
			{
//				fread( pPathTable[i], pNodeHeader->nNumPathTableNodes * sizeof(pPathTable[i][0]), 1, f );
				gstate->FS_Read( pPathTable[i], pNodeHeader->nNumPathTableNodes * sizeof(pPathTable[i][0]), f );
			}
			break;
		}

		default:
			break;
	}

	pNodeHeader->bUsePathTable = TRUE;
	AI_Dprintf( "Using Path Table.\n" );

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ReadPathTable
// Description:
//				read a path table for a node header
// Input:
//				pNodeHeader	=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to a node header with path table read in
//				NULL			=> if error occurred
// Note:
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_ReadPathTable( NODEHEADER_PTR pNodeHeader )
{
	FILE	*f;
	char	base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, gstate->mapName );
	strcat( base_name, ".ptb" );

	if ( !(f = fopen (base_name, "rb")) )
	{
		AI_Dprintf( "Could not find %s.\n", base_name );
		return pNodeHeader;
	}

	pNodeHeader = NODE_ReadPathTable( f, pNodeHeader );
	if ( !pNodeHeader )
	{
		AI_Dprintf( "Wrong file %s.\n", base_name );
	}
	else
	{
		AI_Dprintf( "Read path table from %s.\n", base_name );
	}

	fclose(f);

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteNodeData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static
void NODE_WriteNodeData( FILE *f, MAPNODE_PTR pNode )
{
	fwrite( &nNodeDataVersion, sizeof(nNodeDataVersion), 1, f );
	fwrite( &(pNode->node_data), sizeof(pNode->node_data), 1, f );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Write
// Description:
//				write out the node header information to disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static
void NODE_Write( FILE *f, NODEHEADER_PTR pNodeHeader )
{
	fwrite( &pNodeHeader->nNumNodes, sizeof(pNodeHeader->nNumNodes), 1, f );
	fwrite( &pNodeHeader->nTotalNumNodes, sizeof(pNodeHeader->nTotalNumNodes), 1, f );

	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		//	save this node's number
		fwrite( &pNodes[i].nIndex, sizeof(pNodes[i].nIndex), 1, f );

		//	save node's origin
		//fwrite( pNodes[i].position, sizeof(vec3_t), 1, f );
		float x, y, z;
		x = pNodes[i].position.X();
		y = pNodes[i].position.Y();
		z = pNodes[i].position.Z();
		fwrite( &x, sizeof(x), 1, f );
		fwrite( &y, sizeof(y), 1, f );
		fwrite( &z, sizeof(z), 1, f );
		
		//	save node type
		fwrite( &(pNodes[i].node_type), sizeof(pNodes[i].node_type), 1, f );

		NODE_WriteNodeData( f, &(pNodes[i]) );

		int nTargetLen = 0;
		if ( pNodes[i].target )
		{
			nTargetLen = strlen(pNodes[i].target) + 1;
			fwrite( &nTargetLen, sizeof(nTargetLen), 1, f );
			if ( nTargetLen > 0 )
			{
				fwrite( pNodes[i].target, sizeof(char), nTargetLen, f );
			}
		}
		else
		{
			fwrite( &nTargetLen, sizeof(nTargetLen), 1, f );
		}

		int nTargetNameLen = 0;
		if ( pNodes[i].targetname )
		{
			nTargetNameLen = strlen(pNodes[i].targetname) + 1;
			fwrite( &nTargetNameLen, sizeof(nTargetNameLen), 1, f );
			if ( nTargetNameLen > 0 )
			{
				fwrite( pNodes[i].targetname, sizeof(char), nTargetNameLen, f );
			}
		}
		else
		{
			fwrite( &nTargetNameLen, sizeof(nTargetNameLen), 1, f );
		}

		fwrite( &(pNodes[i].nNumLinks), sizeof(pNodes[i].nNumLinks), 1, f );
		for ( int j = 0; j < pNodes[i].nNumLinks; j++ )
		{
			fwrite( &(pNodes[i].aLinks[j]), sizeof(LINK), 1, f );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteGroundNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteGroundNodes( FILE *f )
{
	if ( pGroundNodes->nNumNodes > 0 )
	{
		fwrite( szGroundNodeSignature, sizeof(szGroundNodeSignature), 1, f );
		fwrite( &nGroundNodeVersion, sizeof(nGroundNodeVersion), 1, f );

		NODE_Write( f, pGroundNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteGroundPathTable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteGroundPathTable( FILE *f )
{
	if ( pGroundNodes->nNumPathTableNodes > 0 )
	{
		fwrite( szGroundPathTableSignature, sizeof(szGroundPathTableSignature), 1, f );
		fwrite( &nGroundPathTableVersion, sizeof(nGroundPathTableVersion), 1, f );

		NODE_WritePathTable( f, pGroundNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteAirNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteAirNodes( FILE *f )
{
	if ( pAirNodes->nNumNodes > 0 )
	{
		fwrite( szAirNodeSignature, sizeof(szAirNodeSignature), 1, f );
		fwrite( &nAirNodeVersion, sizeof(nAirNodeVersion), 1, f );

		NODE_Write( f, pAirNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteAirPathTable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteAirPathTable( FILE *f )
{
	if ( pAirNodes->nNumPathTableNodes > 0  )
	{
		fwrite( szAirPathTableSignature, sizeof(szAirPathTableSignature), 1, f );
		fwrite( &nAirPathTableVersion, sizeof(nAirPathTableVersion), 1, f );

		NODE_WritePathTable( f, pAirNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteTrackNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteTrackNodes( FILE *f )
{
	if ( pTrackNodes->nNumNodes > 0 )
	{
		fwrite( szTrackNodeSignature, sizeof(szTrackNodeSignature), 1, f );
		fwrite( &nTrackNodeVersion, sizeof(nTrackNodeVersion), 1, f );

		NODE_Write( f, pTrackNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WriteTrackPathTable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WriteTrackPathTable( FILE *f )
{
	if ( pTrackNodes->nNumPathTableNodes > 0 )
	{
		fwrite( szTrackPathTableSignature, sizeof(szTrackPathTableSignature), 1, f );
		fwrite( &nTrackPathTableVersion, sizeof(nTrackPathTableVersion), 1, f );

		NODE_WritePathTable( f, pTrackNodes );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Write
// Description:
//				write out the node header information to disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_Write()
{
	FILE	*f;
	char	base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, gstate->mapName );
	strcat( base_name, ".nod" );

	AI_Dprintf ("mapname = %s\n", gstate->mapName);

	if ( !(f = fopen (base_name, "wb")) )
	{
		AI_Dprintf( "Unable to create node file %s.\n", base_name );
		return;
	}

	//NODE_FixNodeLinks( pNodeHeader );

	fwrite( szNodeSignature, sizeof(szNodeSignature), 1, f );
	fwrite( &nNodeVersion, sizeof(nNodeVersion), 1, f );

	// ground nodes 
	if ( pGroundNodes )
	{
		NODE_WriteGroundNodes( f );
		NODE_WriteGroundPathTable( f );
		AI_Dprintf( "Wrote %i ground nodes to %s.\n", pGroundNodes->nNumNodes, base_name );
	}
			
	// air nodes 
	if ( pAirNodes )
	{
		NODE_WriteAirNodes( f );
		NODE_WriteAirPathTable( f );
		AI_Dprintf( "Wrote %i air nodes to %s.\n", pAirNodes->nNumNodes, base_name );
	}

	// track nodes 
	if ( pTrackNodes )
	{
		NODE_WriteTrackNodes( f );
		NODE_WriteTrackPathTable( f );
		AI_Dprintf( "Wrote %i track nodes to %s.\n", pTrackNodes->nNumNodes, base_name );
	}

	if ( pPathOctree )
	{
		OCTREE_WriteData( pPathOctree, f );
	}

	fclose (f);
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_DeleteDuplicateNodeLinks
// Description:
//				Get rid of duplicate links in a node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NODE_DeleteDuplicateNodeLinks( NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	AI_Dprintf( "Deleting duplicate node links......\n" );

	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		_ASSERTE( pNode );

		for ( int j = 0; j < pNode->nNumLinks; j++ )
		{
			int nCurrentIndex = pNode->aLinks[j].nIndex;
			for ( int k = j+1; k < pNode->nNumLinks; k++ )
			{
				if ( nCurrentIndex == pNode->aLinks[k].nIndex )
				{
					// remove this link
					for ( int l = k; l < pNode->nNumLinks-1; l++ )
					{
						pNode->aLinks[l] = pNode->aLinks[l+1];
					}
					pNode->nNumLinks--;

					// redo the k index
					k--;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_CheckSignature
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int NODE_CheckSignature( FILE *f )
{
	// save the current file position
/*
	fpos_t pos;				
	if ( fgetpos( f, &pos ) != 0 )         
	{	
		perror( "fgetpos error" );
	}
*/		
	if( gstate->FS_GetBytesRead() >= g_nTotalBytes )
	{
		return SIGNATURE_NONE;
	}

	char buff[256];
//	int nReadCount = fread( buff, sizeof(char), 256, f );
	gstate->FS_Read( buff, 256, f );

	// SCG[12/9/99]: move back 11 bytes.  hardcoded because quake_fs doesn't support
	// SCG[12/9/99]: getpos...
	gstate->FS_Seek( f, -256, SEEK_CUR );
//	if ( nReadCount != 0 )
	{
/*
		// reset to previous position in file
		if ( fsetpos( f, &pos ) != 0 )
		{
			perror( "fsetpos error" );
		}
*/
		if ( _stricmp( buff, szGroundNodeSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szGroundNodeSignature), f );
//			fread( buff, sizeof(szGroundNodeSignature), 1, f );
			return SIGNATURE_GROUNDNODES;
		}
		else
		if ( _stricmp( buff, szGroundPathTableSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szGroundPathTableSignature), f );
//			fread( buff, sizeof(szGroundPathTableSignature), 1, f );
			return SIGNATURE_GROUNDPATHTABLE;
		}
		else
		if ( _stricmp( buff, szAirNodeSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szAirNodeSignature), f );
//			fread( buff, sizeof(szAirNodeSignature), 1, f );
			return SIGNATURE_AIRNODES;
		}
		else
		if ( _stricmp( buff, szAirPathTableSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szAirPathTableSignature), f );
//			fread( buff, sizeof(szAirPathTableSignature), 1, f );
			return SIGNATURE_AIRPATHTABLE;
		}
		else
		if ( _stricmp( buff, szTrackNodeSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szTrackNodeSignature), f );
//			fread( buff, sizeof(szTrackNodeSignature), 1, f );
			return SIGNATURE_TRACKNODES;
		}
		else
		if ( _stricmp( buff, szTrackPathTableSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szTrackPathTableSignature), f );
//			fread( buff, sizeof(szTrackPathTableSignature), 1, f );
			return SIGNATURE_TRACKPATHTABLE;
		}
		else
		if ( _stricmp( buff, szOctreeSignature ) == 0 )
		{
			gstate->FS_Read( buff, sizeof(szOctreeSignature), f );
//			fread( buff, sizeof(szOctreeSignature), 1, f );
			return SIGNATURE_OCTREE;
		}
	}

	return SIGNATURE_NONE;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ReadNodeData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static
void NODE_ReadNodeData( FILE *f, MAPNODE_PTR pNode )
{
	int nVersion;
//	fread( &nVersion, sizeof(nVersion), 1, f );
	gstate->FS_Read( &nVersion, sizeof(nVersion), f );
	switch ( nVersion )
	{
		case 0:
		{
			typedef union nodeData
			{
				float vector[3];			// position or angles
				char szTarget[32];		// name of the target
				
			} PREV_NODEDATA, *PREV_NODEDATA_PTR;

			PREV_NODEDATA prevNodeData;
//			fread( &(prevNodeData), sizeof(prevNodeData), 1, f );
			gstate->FS_Read( &(prevNodeData), sizeof(prevNodeData), f );
			break;
		}

		case 1:
		{
//			fread( &(pNode->node_data), sizeof(pNode->node_data), 1, f );
			gstate->FS_Read( &(pNode->node_data), sizeof(pNode->node_data), f );
			break;
		}

		default:
			break;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Read
// Description:
//				read the node header information from the disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to a newly read in node header
// Note:
//
// ----------------------------------------------------------------------------
static NODEHEADER_PTR NODE_Read( FILE *f )
{
	NODEHEADER_PTR pNodeHeader = NULL;

	int nVersion;
//	fread( &nVersion, sizeof(nVersion), 1, f );
	gstate->FS_Read( &nVersion, sizeof(nVersion), f );
	switch ( nVersion )
	{
		case 0:
		{
			int nNumNodes, nTotalNumNodes;

			// ground nodes
//			fread( &nNumNodes, sizeof(nNumNodes), 1, f );
//			fread( &nTotalNumNodes, sizeof(nTotalNumNodes), 1, f );
			gstate->FS_Read( &nNumNodes, sizeof(nNumNodes), f );
			gstate->FS_Read( &nTotalNumNodes, sizeof(nTotalNumNodes), f );

			pNodeHeader = NODE_Init( pNodeHeader, nTotalNumNodes );
			_ASSERTE( pNodeHeader  );
			pNodeHeader->nTotalNumNodes = nTotalNumNodes;
			pNodeHeader->nNumNodes		= nNumNodes;

			_ASSERTE( pNodeHeader );

			MAPNODE_PTR pNodes = pNodeHeader->pNodes;
			//_ASSERTE( pNodes );
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				//	read this node's number
//				fread( &pNodes[i].nIndex, sizeof(pNodes[i].nIndex), 1, f );
				gstate->FS_Read( &pNodes[i].nIndex, sizeof(pNodes[i].nIndex), f );

				//	read node's origin
				float x, y, z;
//				fread( &x, sizeof(x), 1, f );
//				fread( &y, sizeof(y), 1, f );
//				fread( &z, sizeof(z), 1, f );
				gstate->FS_Read( &x, sizeof(x), f );
				gstate->FS_Read( &y, sizeof(y), f );
				gstate->FS_Read( &z, sizeof(z), f );
				pNodes[i].position.Set( x, y, z );
				
				//	read node type
//				fread( &(pNodes[i].node_type), sizeof(pNodes[i].node_type), 1, f );
				gstate->FS_Read( &(pNodes[i].node_type), sizeof(pNodes[i].node_type), f );
				NODE_ReadNodeData( f, &(pNodes[i]) );

				int nTargetLen = 0;
//				fread( &nTargetLen, sizeof(nTargetLen), 1, f );
				gstate->FS_Read( &nTargetLen, sizeof(nTargetLen), f );
				if ( nTargetLen > 0 )
				{
//					pNodes[i].target = new char[nTargetLen];
					pNodes[i].target = (char*)gstate->X_Malloc(sizeof(char)*nTargetLen,MEM_TAG_NODE);
//					fread( pNodes[i].target, sizeof(char), nTargetLen, f );
					gstate->FS_Read( pNodes[i].target, nTargetLen, f );
				}

				int nTargetNameLen = 0;
//				fread( &nTargetNameLen, sizeof(nTargetNameLen), 1, f );
				gstate->FS_Read( &nTargetNameLen, sizeof(nTargetNameLen), f );
				if ( nTargetNameLen > 0 )
				{
//					pNodes[i].targetname = new char[nTargetNameLen];
					pNodes[i].targetname = (char*)gstate->X_Malloc(sizeof(char)*nTargetNameLen,MEM_TAG_NODE);
//					fread( pNodes[i].targetname, sizeof(char), nTargetNameLen, f );
					gstate->FS_Read( pNodes[i].targetname, nTargetNameLen, f );
				}

//				fread( &(pNodes[i].nNumLinks), sizeof(pNodes[i].nNumLinks), 1, f );
				gstate->FS_Read( &(pNodes[i].nNumLinks), sizeof(pNodes[i].nNumLinks), f );
				for ( int j = 0; j < pNodes[i].nNumLinks; j++ )
				{
//					fread( &(pNodes[i].aLinks[j]), sizeof(LINK), 1, f );
					gstate->FS_Read( &(pNodes[i].aLinks[j]), sizeof(LINK), f );
				}

                pNodes[i].bTriggered = FALSE;
			}

			// now compute the distance between link nodes
			for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				for ( int j = 0; j < pNodes[i].nNumLinks; j++ )
				{
					MAPNODE_PTR pLinkedNode = NODE_GetNode( pNodeHeader, pNodes[i].aLinks[j].nIndex );
					if(!pLinkedNode)
					{
						char Buffer[200];
//						sprintf(Buffer,"Node has Ghost Index Link:\n--------------------------\nNodeID:%d\nLink#:%d\n",i,j);
						Com_sprintf(Buffer,sizeof(Buffer),"Node has Ghost Index Link:\n--------------------------\nNodeID:%d\nLink#:%d\n",i,j);
						gstate->Error(Buffer);
					}
					_ASSERTE( pLinkedNode );
					short nDistance = (short)VectorDistance( pNodes[i].position, pLinkedNode->position );
					pNodes[i].aLinks[j].nDistance = nDistance;
				}
			}

			break;
		}
		
		default:
			break;
	}		

	NODE_DeleteDuplicateNodeLinks( pNodeHeader );

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_Read
// Description:
//				read the node header information from the disk
// Input:
//				pNodeHeader		=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to a newly read in node header
// Note:
//
// ----------------------------------------------------------------------------
void NODE_Read()
{
	FILE *f = NULL;
	char base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

//	strcpy( base_name, gstate->basedir );
//	strcat( base_name, "/maps/nodes/" );
	strcpy( base_name, "maps/nodes/" );

    char *pszNodeFileName = NODE_GetNodeFileName();
    if ( strlen( pszNodeFileName ) > 0 )
    {
        strcat( base_name, pszNodeFileName );    
    }
    else
    {
	    strcat( base_name, gstate->mapName );
    }
    if ( !strstr( base_name, ".nod" ) )
    {
	    strcat( base_name, ".nod" );
    }

	
//	if ( !(f = fopen (base_name, "rb")) )
	if ( !( gstate->FS_Open( base_name, &f ) ) )
	{
		AI_Dprintf( "Could not find %s.\n", base_name );
		return;
	}

	if( f == NULL )
	{
		return;
	}

	g_nTotalBytes = gstate->FS_FileLength( f ) + gstate->FS_GetBytesRead();

	char buff[64];
//	int nReadCount = fread( buff, sizeof(szNodeSignature), 1, f );
	gstate->FS_Read( buff, sizeof(szNodeSignature), f );
//	if ( nReadCount == 0 || _stricmp( buff, szNodeSignature ) != 0 )
	if ( _stricmp( buff, szNodeSignature ) != 0 )
	{
//		fclose( f );
		gstate->FS_Close( f );
		AI_Dprintf( "Bad node file format: %s.\n", base_name );
		return;
	}

	int nVersion;
//	fread( &nVersion, sizeof(nVersion), 1, f );
	gstate->FS_Read( &nVersion, sizeof(nVersion), f );
	switch ( nVersion )
	{
		case 0:
		{
			while ( 1 )
			{
				int bBreak = FALSE;
				int nSignature = NODE_CheckSignature( f );
				switch ( nSignature )
				{
					case SIGNATURE_NONE:
					{
						bBreak = TRUE;
						break;
					}
					case SIGNATURE_GROUNDNODES:
					{
						pGroundNodes = NODE_Read( f );
						break;
					}
					case SIGNATURE_GROUNDPATHTABLE:
					{
   						NODE_ReadPathTable( f, pGroundNodes );
						break;
					}
					case SIGNATURE_AIRNODES:
					{
						pAirNodes = NODE_Read( f );
						break;
					}
					case SIGNATURE_AIRPATHTABLE:
					{
   						NODE_ReadPathTable( f, pAirNodes );
						break;
					}
					case SIGNATURE_TRACKNODES:
					{
						pTrackNodes = NODE_Read( f );
						break;
					}
					case SIGNATURE_TRACKPATHTABLE:
					{
   						NODE_ReadPathTable( f, pTrackNodes );
						break;
					}
					case SIGNATURE_OCTREE:
					{
						if ( pPathOctree )
						{
							OCTREE_ReadData( pPathOctree, f );
						}

						break;
					}
					default:
					{
						bBreak = TRUE;
						break;
					}
				}

				if ( bBreak )
				{
					break;
				}
			}

			break;
		}

		default:
			break;
	}

//	fclose(f);
	gstate->FS_Close( f );

	if ( pGroundNodes )
	{
		if ( pGroundNodes->nNumNodes > 0 )
		{
			NODE_DisableLayingNodes();
		}

		AI_Dprintf( "Read %i Ground nodes from %s.\n", pGroundNodes->nNumNodes, base_name );
	}
	if ( pAirNodes )
	{
		AI_Dprintf( "Read %i Air nodes from %s.\n", pAirNodes->nNumNodes, base_name );
	}
	if ( pTrackNodes )
	{
		AI_Dprintf( "Read %i Track nodes from %s.\n", pTrackNodes->nNumNodes, base_name );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_WritePrevious
// Description:
//				write the previous version of the node header
// Input:
//				pNodeHeader	=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to newly allocated node header
// Note:
//
// ----------------------------------------------------------------------------
void NODE_WritePrevious( NODEHEADER_PTR pNodeHeader )
{
	_ASSERTE( pNodeHeader );

	FILE	*f;
	char	base_name[128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, "Prev-" );
	strcat( base_name, gstate->mapName );
	strcat( base_name, ".nod" );

	AI_Dprintf ("mapname = %s\n", gstate->mapName);

	if ( !(f = fopen (base_name, "wb")) )
	{
		AI_Dprintf( "Unable to create node file %s.\n", base_name );
		return;
	}

	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		fwrite( &(pNodes[i].nIndex), sizeof(int), 1, f);

		//fwrite( pNodes[i].position, sizeof(vec3_t), 1, f );
		float x, y, z;
		x = pNodes[i].position.X();
		y = pNodes[i].position.Y();
		z = pNodes[i].position.Z();
		fwrite( &x, sizeof(x), 1, f );
		fwrite( &y, sizeof(y), 1, f );
		fwrite( &z, sizeof(z), 1, f );

		//	read node type
		fwrite( &(pNodes[i].node_type), sizeof(int), 1, f);

		//	write node type
		int aTempLinks[MAX_NODE_LINKS];
		for ( int j = 0; j < MAX_NODE_LINKS; j++ )
		{
			aTempLinks[j] = pNodes[i].aLinks[j].nIndex;
		}
		fwrite( aTempLinks, sizeof(int) * MAX_NODE_LINKS, 1, f );
	}
		
	fclose (f);

	AI_Dprintf( "Wrote %i nodes to %s.\n", pNodeHeader->nNumNodes, base_name );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_ReadPrevious
// Description:
//				read in the previous version of the node header
// Input:
//				pNodeHeader	=> pointer to a node header
// Output:
//				NODEHEADER_PTR	=> pointer to newly allocated node header
// Note:
//
// ----------------------------------------------------------------------------
NODEHEADER_PTR NODE_ReadPrevious( NODEHEADER_PTR pNodeHeader )
{
	FILE	*f;
	char	base_name [128];

	///////////////////////////////////////////
	//	get the name of the current map's wad
	///////////////////////////////////////////

	strcpy( base_name, gstate->basedir );
	strcat( base_name, "/maps/nodes/" );
	strcat( base_name, gstate->mapName );
	strcat( base_name, ".nod");

	if ( !(f = fopen (base_name, "rb")) )
	{
		AI_Dprintf( "Could not find %s.\n", base_name );
		return pNodeHeader;
	}

	pNodeHeader = NODE_Init( pNodeHeader );
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	_ASSERTE( pNodes );

	int count = 0;

	// read in the entire list of nodes
	int aTempLinks[MAX_NODE_LINKS];
	while ( !feof(f) )
	{
//		if ( !fread( &(pNodes[count].nIndex), sizeof(int), 1, f) )
//		{
//			break;
//		}
		gstate->FS_Read( &(pNodes[count].nIndex), sizeof(int), f);

		// it should be zero based, previous version started at 1
		pNodes[count].nIndex--;

		//fread( pNodes[count].position, sizeof(vec3_t), 1, f );
		float x, y, z;
//		fread( &x, sizeof(x), 1, f );
//		fread( &y, sizeof(y), 1, f );
//		fread( &z, sizeof(z), 1, f );
		gstate->FS_Read( &x, sizeof(x), f );
		gstate->FS_Read( &y, sizeof(y), f );
		gstate->FS_Read( &z, sizeof(z), f );
		pNodes[count].position.Set( x, y, z );

		//	read node type
//		fread( &(pNodes[count].node_type), sizeof(int), 1, f);
		gstate->FS_Read( &(pNodes[count].node_type), sizeof(int), f);

		//	read links
//		fread( aTempLinks, sizeof(int), MAX_NODE_LINKS, f );
		gstate->FS_Read( aTempLinks, MAX_NODE_LINKS * sizeof(int), f );

		pNodes[count].nNumLinks = 0;
		for ( int i = 0; i < MAX_NODE_LINKS; i++ )
		{
			if ( aTempLinks[i] )
			{
				pNodes[count].aLinks[i].nIndex = aTempLinks[i] - 1;
				pNodes[count].nNumLinks++;
			}
		}

		count++;
	}

	fclose(f);

	pNodeHeader->nNumNodes = count;

	// now find the distance between all links
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		_ASSERTE( pNode );

		for ( int j = 0; j < pNode->nNumLinks; j++ )
		{
			short nLinkIndex = pNode->aLinks[j].nIndex;
			MAPNODE_PTR pLinkNode = NODE_GetNode( pNodeHeader, nLinkIndex );
			_ASSERTE( pLinkNode );

			pNode->aLinks[j].nDistance = (short)VectorDistance( pNode->position, pLinkNode->position );
		}
	}

	AI_Dprintf( "Read %i nodes from %s.\n", pNodeHeader->nNumNodes, base_name );

	return pNodeHeader;
}

// ----------------------------------------------------------------------------
//
// Name:		node_find_cur_node
// Description:
//              find nCurrentNodeIndex
// Input:
// Output:
// Note:
//              air units and units in water do not need to call this function
//              these units need to call FindClosestNode() whenever they need
//              to find a path
//
// ----------------------------------------------------------------------------
int	node_find_cur_node( userEntity_t *self, NODELIST_PTR pNodeList )
{
	_ASSERTE( self );
	_ASSERTE( pNodeList );

	playerHook_t	*hook = AI_GetPlayerHook( self );
	float			dist, min_dist, max_dist;
	CVector			temp;
	CVector			origin;

	if ( AI_IsInAir( self ) || AI_IsInWater( self ) )
	{
		//	flying things don't leave their own nodes, so check for closest
		//	node from a point 24 units above the ground

		//_ASSERTE( FALSE );
		origin = hook->ground_org;
	}
	else
	{
		origin = self->s.origin;
	}

	// SCG[10/30/99]: Make sure the node list is valid!
	if( pNodeList == NULL )
	{
		return FALSE;
	}

	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	//First make sure we have a header
	if ( !pNodeHeader || pNodeHeader->nNumNodes == 0 )
	{
		return FALSE;
	}
	_ASSERTE( pNodeHeader );
	//Next make sure there are actual nodes to search through
	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	if ( !pNodes )
	{
		return FALSE;
	}

	int nCurrentNodeIndex = -1;
	//Do we use the octree method?
	if ( pPathOctree && OCTREE_GetNumNodes( pPathOctree ) > 0 )
	{
		max_dist = min_dist = MAX_NODE_DIST * 5.0f;

		short aNearGroundNodeIndices[MAX_NUM_NEAR_PATHNODES];
		int nNumNearGroundNodes = OCTREE_GetNearGroundNodes( pPathOctree, self->s.origin, aNearGroundNodeIndices );
		for ( int i = 0; i < nNumNearGroundNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodes[aNearGroundNodeIndices[i]]);
			if(!aNearGroundNodeIndices[i] == pNode->nIndex)
			{
				return FALSE;
			}
			//_ASSERTE( aNearGroundNodeIndices[i] == pNode->nIndex );

			//	never choose a NODE_TELEPORTER as the current node!
			//if ( !(pNode->node_type & NODE_TELEPORTER) )
			{
				dist = VectorDistance(origin, pNode->position);
				
				if ( dist <= 16.0f )
				{
					nCurrentNodeIndex = i;
					break;
				}

				if (dist < min_dist)
				{
					// is this node nearly the same height as our current node?
					if ( fabs(origin.z - pNode->position.z) < MAX_JUMP_HEIGHT )
					{
						nCurrentNodeIndex = aNearGroundNodeIndices[i];
						min_dist = dist;
					}
				}
			}
		}

		min_dist = MAX_NODE_DIST;
		int nTestNodeIndex = -1;
		for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodes[i]);
		
			//	never choose a NODE_TELEPORTER as the current node!
			//if ( !(pNode->node_type & NODE_TELEPORTER) )
			{
				dist = VectorDistance(origin, pNode->position);
				
				if ( dist <= 16.0f )
				{
					nCurrentNodeIndex = i;
					break;
				}

				if (dist < min_dist)
				{
					// is this node is nearly the same height as our current node?
					if ( fabs(origin.z - pNode->position.z) < MAX_JUMP_HEIGHT )
					{
						// can this node see the entity who is checking it?
						tr = gstate->TraceLine_q2( origin, pNode->position, self, MASK_SOLID );
						if ( tr.fraction == 1.0)
						{
							nTestNodeIndex = i;
							min_dist = dist;
						}
					}
				}
			}
		}

		//_ASSERTE( nCurrentNodeIndex != -1 );
	}
	else  //Non-Octree method
	{
		max_dist = min_dist = MAX_NODE_DIST;

		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodes[i]);
		
			dist = VectorDistance(origin, pNode->position);
			
			if ( dist <= 16.0f )
			{
				nCurrentNodeIndex = i;
				break;
			}

			if (dist < min_dist)
			{
				// is this node is nearly the same height as our current node?
				if ( fabs(origin.z - pNode->position.z) < MAX_JUMP_HEIGHT )
				{
					// can this node see the entity who is checking it?
					tr = gstate->TraceLine_q2( origin, pNode->position, self, MASK_SOLID );
					if ( tr.fraction == 1.0 )
					{
						nCurrentNodeIndex = i;
						min_dist = dist;
					}
				}
			}
		}
	}

	// was a visible node found that is < MAX_NODE_DIST away or closer than our current node?  
	// if so, make it the cur_node
	if ( nCurrentNodeIndex != -1 )
	{
		if ( nCurrentNodeIndex != pNodeList->nCurrentNodeIndex )
		{
			//MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
			MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, nCurrentNodeIndex );
			if ( pCurrentNode )
			{
				pNodeList->nLastNodeIndex = pNodeList->nCurrentNodeIndex;
				pNodeList->nCurrentNodeIndex = nCurrentNodeIndex;

				MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, pNodeList->nLastNodeIndex );
				if ( pLastNode && pNodeList->nLastNodeIndex != pNodeList->nCurrentNodeIndex )
				{
					gstate->TraceLine( pLastNode->position, pCurrentNode->position, TRUE, NULL, &trace );
					if ( trace.fraction >= 0.9 )
					{
						NODE_LinkInOneDirection( pNodeHeader, pNodeList->nLastNodeIndex, pNodeList->nCurrentNodeIndex );
					}
				}
			}
		}

		return	TRUE;
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestNode( NODEHEADER_PTR pNodeHeader, int nNodeType, CVector &position )
{
	_ASSERTE( pNodeHeader );

	
	float fMinDistance = MAX_NODE_DIST * 5.0f;

	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	if ( !pNodes )
	{
		return NULL;
	}

	int nCurrentNodeIndex = -1;
	if ( pPathOctree && OCTREE_GetNumNodes( pPathOctree ) > 0 )
	{
		fMinDistance = 1e20;

		short aNearNodeIndices[MAX_NUM_NEAR_PATHNODES];
		int nNumNearNodes = 0;
		if ( nNodeType & NODETYPE_GROUND )
		{
			nNumNearNodes = OCTREE_GetNearGroundNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_WATER )
		{
			nNumNearNodes = OCTREE_GetNearWaterNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_AIR )
		{
			nNumNearNodes = OCTREE_GetNearAirNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_TRACK )
		{
			nNumNearNodes = OCTREE_GetNearTrackNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		{
			_ASSERTE( FALSE );
		}				

		if ( nNodeType & NODETYPE_GROUND )
		{
			for ( int i = 0; i < nNumNearNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
				_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorXYDistance( pNode->position, position );
					float fZDistance = VectorZDistance( pNode->position, position );

					if ( fZDistance < MAX_JUMP_HEIGHT )
					{
						if ( fDistance <= 32.0f )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							break;
						}

						if ( fDistance < fMinDistance )
						{
							tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
							if ( tr.fraction >= 1.0 )
							{
								nCurrentNodeIndex = aNearNodeIndices[i];
								fMinDistance = fDistance;
							}
						}
					}
				}
			}
			
			if ( nCurrentNodeIndex == -1 )
			{
				for ( i = 0; i < nNumNearNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
					_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorXYDistance( pNode->position, position );
						float fZDistance = VectorZDistance( pNode->position, position );

						if ( fDistance <= 32.0f )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							break;
						}

						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							fMinDistance = fDistance;
						}
					}
				}
			
			}

//			_ASSERTE( nCurrentNodeIndex != -1 );
		}
		else  //Air / Water units
		{
			for ( int i = 0; i < nNumNearNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
				_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorDistance( pNode->position, position );
					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = aNearNodeIndices[i];
						break;
					}

					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							fMinDistance = fDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		fMinDistance = 1e20;

		if ( nNodeType & NODETYPE_GROUND )
		{
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[i]);
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorXYDistance(position, pNode->position);
					float fZDistance = VectorZDistance( position, pNode->position );

					if ( fZDistance < MAX_JUMP_HEIGHT )
					{
						if ( fDistance <= 32.0f )
						{
							nCurrentNodeIndex = i;
							break;
						}

						if ( fDistance < fMinDistance )
						{
							tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
							if ( tr.fraction >= 1.0 )
							{
								nCurrentNodeIndex = i;
								fMinDistance = fDistance;
							}
						}
					}
				}
			}

			if ( nCurrentNodeIndex == -1 )
			{
				fMinDistance = 1e20;
				for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[i]);
					_ASSERTE( pNode );
					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorDistance( pNode->position, position );
						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}
			//_ASSERTE( nCurrentNodeIndex != -1 );
		}
		else  //Air / Water units
		{
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[i]);
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorDistance(position, pNode->position);
					
					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = i;
						break;
					}

					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}

			if ( nCurrentNodeIndex == -1 )
			{
				fMinDistance = 1e20;
				for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[i]);
					_ASSERTE( pNode );
					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorDistance( pNode->position, position );
						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}
			//_ASSERTE( nCurrentNodeIndex != -1 );
		}
	}

	// was a visible node found that is < MAX_NODE_DIST away or closer than our current node?  
	// if so, make it the cur_node
	if ( nCurrentNodeIndex != -1 )
	{
		MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, nCurrentNodeIndex );

		return pCurrentNode;
	}

	return NULL;
}





// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestNodeForLanding
// Description:This function will find the closest node to land on.
// Input:
// Output:
// Note:This is a copy from the generic FindClosestNode and is modified to not
// discriminate against hieght and other things of that nature.
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestNodeForLanding( NODEHEADER_PTR pNodeHeader, int nNodeType, CVector &position )
{
	_ASSERTE( pNodeHeader );

	float fMinDistance = MAX_NODE_DIST * 5.0f;

	MAPNODE_PTR pNodes = pNodeHeader->pNodes;
	if ( !pNodes )
	{
		return NULL;
	}

	int nCurrentNodeIndex = -1;
	if ( pPathOctree && OCTREE_GetNumNodes( pPathOctree ) > 0 )
	{
		fMinDistance = 1e20;

		short aNearNodeIndices[MAX_NUM_NEAR_PATHNODES];
		int nNumNearNodes = 0;
		if ( nNodeType & NODETYPE_GROUND )
		{
			nNumNearNodes = OCTREE_GetNearGroundNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_WATER )
		{
			nNumNearNodes = OCTREE_GetNearWaterNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_AIR )
		{
			nNumNearNodes = OCTREE_GetNearAirNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		if ( nNodeType & NODETYPE_TRACK )
		{
			nNumNearNodes = OCTREE_GetNearTrackNodes( pPathOctree, position, aNearNodeIndices );
		}
		else
		{
			_ASSERTE( FALSE );
		}				

		if ( nNodeType & NODETYPE_GROUND )
		{
			for ( int i = 0; i < nNumNearNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
				_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
				// NSS[2/7/00]:
//				if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorXYDistance( pNode->position, position );
//					float fZDistance = VectorZDistance( pNode->position, position );// SCG[1/23/00]: not used

					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = aNearNodeIndices[i];
						break;
					}

					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							fMinDistance = fDistance;
						}
					}
				}
			}
			
			if ( nCurrentNodeIndex == -1 )
			{
				for ( i = 0; i < nNumNearNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
					_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorXYDistance( pNode->position, position );
//						float fZDistance = VectorZDistance( pNode->position, position );// SCG[1/23/00]: not used

						if ( fDistance <= 32.0f )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							break;
						}

						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							fMinDistance = fDistance;
						}
					}
				}
			
			}

//			_ASSERTE( nCurrentNodeIndex != -1 );
		}
		else
		{
			for ( int i = 0; i < nNumNearNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[aNearNodeIndices[i]]);
				_ASSERTE( aNearNodeIndices[i] == pNode->nIndex );
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorDistance( pNode->position, position );
					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = aNearNodeIndices[i];
						break;
					}

					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = aNearNodeIndices[i];
							fMinDistance = fDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		fMinDistance = 1e20;

		if ( nNodeType & NODETYPE_GROUND )
		{
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[i]);
				// NSS[2/7/00]:
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorXYDistance(position, pNode->position);
//					float fZDistance = VectorZDistance( position, pNode->position );// SCG[1/23/00]: not used

					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = i;
						break;
					}
					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}

			if ( nCurrentNodeIndex == -1 )
			{
				fMinDistance = 1e20;
				for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[i]);
					_ASSERTE( pNode );
					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorDistance( pNode->position, position );
						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}
			//_ASSERTE( nCurrentNodeIndex != -1 );
		}
		else
		{
			for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
			{
				MAPNODE_PTR pNode = &(pNodes[i]);
			
				
				//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
				if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
				{
					float fDistance = VectorDistance(position, pNode->position);
					
					if ( fDistance <= 32.0f )
					{
						nCurrentNodeIndex = i;
						break;
					}

					if ( fDistance < fMinDistance )
					{
						tr = gstate->TraceLine_q2( position, pNode->position, NULL, MASK_SOLID );
						if ( tr.fraction >= 1.0 )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}

			if ( nCurrentNodeIndex == -1 )
			{
				fMinDistance = 1e20;
				for ( i = 0; i < pNodeHeader->nNumNodes; i++ )
				{
					MAPNODE_PTR pNode = &(pNodes[i]);
					_ASSERTE( pNode );

					// NSS[2/7/00]:
					//if ( (pNode->node_type & nNodeType) && gstate->inPVS( position, pNode->position ) )
					if ( (pNode->node_type & nNodeType) && Node_IsWithinRange(position,  pNode->position, 0, nNodeType) )
					{
						float fDistance = VectorDistance( pNode->position, position );
						if ( fDistance < fMinDistance )
						{
							nCurrentNodeIndex = i;
							fMinDistance = fDistance;
						}
					}
				}
			}
			//_ASSERTE( nCurrentNodeIndex != -1 );
		}
	}

	// was a visible node found that is < MAX_NODE_DIST away or closer than our current node?  
	// if so, make it the cur_node
	if ( nCurrentNodeIndex != -1 )
	{
		MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, nCurrentNodeIndex );

		return pCurrentNode;
	}

	return NULL;
}





// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestGroundNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestGroundNode( userEntity_t *self )
{
	_ASSERTE( self );

	return NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, self->s.origin );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestWaterNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestWaterNode( userEntity_t *self )
{
	_ASSERTE( self );

	return NODE_FindClosestNode( pGroundNodes, NODETYPE_WATER, self->s.origin );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestAirNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestAirNode( userEntity_t *self )
{
	_ASSERTE( self );

	return NODE_FindClosestNode( pAirNodes, NODETYPE_AIR, self->s.origin );
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_FindClosestTrackNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_FindClosestTrackNode( userEntity_t *self )
{
	_ASSERTE( self );

	CVector origin;

	// this is a hack for bmodel origins (which are 0,0,0)
	if (self->solid == SOLID_BSP)
	{
		origin.x = (self->absmin.x + self->absmax.x) * 0.5;
		origin.y = (self->absmin.y + self->absmax.y) * 0.5;
		origin.z = (self->absmin.z + self->absmax.z) * 0.5;
	}							 
	else
	{
		origin = self->s.origin;
	}

	return NODE_FindClosestNode( pTrackNodes, NODETYPE_TRACK, origin );
}


// ----------------------------------------------------------------------------
//
// Name:		node_place_node
// Description:
//	This is the major function called from player/bot routines.
//	It places a node at self's last location if:
//	
//	1) the last node is not visible from self's origin 
//	2) there is no other node which is visible from this spot
//	which also can see the last node
//
//	if a node is placed or linked, then that node is returned.
//	otherwise NULL is returned
//
//	the code that calls node_place_node must update the cur_node and
//	last_node variable accordingly.  This is because bots and players
//	will be using different hook types, so we won't necessarily know
//	which type of hook self is using here, so we don't want to update
//	it.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void node_place_node( userEntity_t *self, NODELIST_PTR pNodeList, const CVector &last_loc )
{
	_ASSERTE( self );
	_ASSERTE( pNodeList );

	playerHook_t *hook = AI_GetPlayerHook( self );

#ifdef _DEBUG
    pathcorner_Draw( self );
#endif

	if ( self->flags & FL_DEBUG )
	{
		if ( bShowNodes )
		{
			node_show_links( self );
			node_update_visible_nodes( self );
		}
	}

	// typically, clients are set to MOVETYPE_BOUNCE when they are dead
	if ( (self->movetype == MOVETYPE_NOCLIP) || (self->movetype == MOVETYPE_BOUNCE) )
	{
		return;
	}

	// did bot move a significant distance?
	CVector temp = hook->last_origin - self->s.origin;
	if ( temp.Length() < 4.0 )
	{
		return;
	}

	if ( self->movetype == MOVETYPE_FLY )
	{
		node_find_cur_node( self, pNodeList );
		return;
	}

	//FIXME : do we already have the point contents somewhere?
	int pc = gstate->PointContents (self->s.origin);
	// if entity is falling do not place a node
	if ( !(self->groundEntity) && (self->velocity .z != 0.0) && (pc != CONTENTS_WATER) )
	{
		return;
	}

	if ( self->groundEntity )
	{
		if ( self->groundEntity->velocity.Length() != 0.0 )
		{
			return;
		}
	}

	//	FIXME: adjust for teleporting
	//	we need a new node!!  Make cur_node the closest visible map node
	if ( node_find_cur_node( self, pNodeList ) )
	{
		// an existing, visible node was found that is < MAX_NODE_DIST away, so return
		return;
	}

	if ( NODE_IsOkToLayNodes() == FALSE )
	{
		return;
	}

	// no existing node was found that fits the criteria, so make one
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	int nNewNodeIndex = NODE_Place( pNodeHeader, last_loc, pNodeList->nCurrentNodeIndex );

	MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, pNodeList->nLastNodeIndex );

	pNodeList->nLastNodeIndex = pNodeList->nCurrentNodeIndex;
	pNodeList->nCurrentNodeIndex = nNewNodeIndex;

	MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	pLastNode = NODE_GetNode( pNodeHeader, pNodeList->nLastNodeIndex );
}

// ----------------------------------------------------------------------------
//
// Name:		node_lay
// Description:
//				forces a node to be placed at self->s.origin
//
//	data must be a MEM_MALLOC block
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void node_lay( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( NODE_IsOkToLayNodes() == FALSE )
	{
		return;
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	// no existing node was found that fits the criteria, so make one
	int nNewNodeIndex = NODE_Place( pNodeHeader, self->s.origin, pNodeList->nCurrentNodeIndex );

	pNodeList->nLastNodeIndex = pNodeList->nCurrentNodeIndex;
	pNodeList->nCurrentNodeIndex = pNodeList->nCurrentNodeIndex;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int NODE_GetTotalNumNodesInSpace( const CVector &mins, const CVector &maxs )
{
	int nCounter = 0;

	NODEHEADER_PTR pNodeHeader = pGroundNodes;
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				nCounter++;
			}
		}
	}

	pNodeHeader = pAirNodes;
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				nCounter++;
			}
		}
	}

	pNodeHeader = pTrackNodes;
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				nCounter++;
			}
		}
	}

	return nCounter;
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
int NODE_GetTotalNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	int nCounter = 0;

	NODEHEADER_PTR pNodeHeader = pGroundNodes;

	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				if ( nCounter < MAX_NUM_NEAR_PATHNODES )
				{
					pNodeIndices[nCounter] = i;
					nCounter++;
				}
			}
		}
	}

	pNodeHeader = pAirNodes;
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				if ( nCounter < MAX_NUM_NEAR_PATHNODES )
				{
					pNodeIndices[nCounter] = i;
					nCounter++;
				}
			}
		}
	}

	pNodeHeader = pTrackNodes;
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			CVector position = pNode->position;
			if ( position.x >= mins.x && position.x <= maxs.x &&
				 position.y >= mins.y && position.y <= maxs.y &&
				 position.z >= mins.z && position.z <= maxs.z )
			{
				if ( nCounter < MAX_NUM_NEAR_PATHNODES )
				{
					pNodeIndices[nCounter] = i;
					nCounter++;
				}
			}
		}
	}

	return nCounter;
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
__inline static
int NODE_GetNodesInSpace( NODEHEADER_PTR pNodeHeader, int nNodeType, const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	int nCounter = 0;

	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			if ( pNode->node_type & nNodeType )
			{
				CVector position = pNode->position;
				if ( position.x >= mins.x && position.x <= maxs.x &&
					 position.y >= mins.y && position.y <= maxs.y &&
					 position.z >= mins.z && position.z <= maxs.z )
				{
					if ( nCounter < MAX_NUM_NEAR_PATHNODES )
					{
						pNodeIndices[nCounter] = i;
						nCounter++;
					}
				}
			}
		}
	}

	return nCounter;
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
int NODE_GetGroundNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	return NODE_GetNodesInSpace( pGroundNodes, NODETYPE_GROUND, mins, maxs, pNodeIndices );
}

int NODE_GetWaterNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	return NODE_GetNodesInSpace( pGroundNodes, NODETYPE_WATER, mins, maxs, pNodeIndices );
}

int NODE_GetAirNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	return NODE_GetNodesInSpace( pAirNodes, NODETYPE_AIR, mins, maxs, pNodeIndices );
}

int NODE_GetTrackNodesInSpace( const CVector &mins, const CVector &maxs, short *pNodeIndices )
{
	return NODE_GetNodesInSpace( pTrackNodes, NODETYPE_TRACK, mins, maxs, pNodeIndices );
}

// ----------------------------------------------------------------------------
//
// Name:	NODE_FindFurthestFrom
// Description:	returns the node that is furthest from origin, that is linked to 
//				pCurNode.  Will not return a node that only has one link (ie.,
//				a node that is only linked back to the current node).
// Input:
//			pNodeList	= a pointer to a list of nodes
//			origin		= a vector represent a point in 3D space
// Output:
//			returns the index of the node furthest from origin, or -1 if there 
//			are no linked nodes
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR	NODE_FurthestFrom (NODELIST_PTR pNodeList, const CVector &origin)
{
	_ASSERTE( pNodeList );
	if( pNodeList == NULL )
	{
		return NULL;
	}

	if ( pNodeList->nCurrentNodeIndex == -1 )
	{
		return NULL;
	}
	
	MAPNODE_PTR	pBestLinkNode = NULL;
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	MAPNODE_PTR pCurrentNode = &(pNodeHeader->pNodes[pNodeList->nCurrentNodeIndex]);
	_ASSERTE( pCurrentNode );

	float fMinDistance = VectorXYDistance( pCurrentNode->position, origin );
	for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pLinkNode = &(pNodeHeader->pNodes[pCurrentNode->aLinks[i].nIndex]);
		if ( pLinkNode->nNumLinks > 1 )
		{
			float fDistance = VectorXYDistance(pLinkNode->position, origin);
			if ( fDistance > fMinDistance )
			{
				pBestLinkNode = pLinkNode;
				fMinDistance = fDistance;
			}
		}
	}

	return pBestLinkNode;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetNodeFromTargetName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetNodeFromTargetName( NODEHEADER_PTR pNodeHeader, char *targetName )
{
	if ( pNodeHeader )
	{
		for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
			if ( _stricmp( pNode->targetname, targetName ) == 0 )
			{
				return pNode;
			}
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestCompleteHideNode
// Description:
//				Find a closest hide node that is not visible from the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define MIN_HIDE_NODE_DISTANCE	768.0f

MAPNODE_PTR NODE_GetClosestCompleteHideNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy )
{
	if ( !pNodeHeader )
	{
		return NULL;
	}

	float fHalfWidth = (self->s.maxs.x - self->s.mins.x) * 0.6;

	int nHideNodeIndex = -1;
	float fMinDist = MIN_HIDE_NODE_DISTANCE;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_HIDE) /*&& !gstate->inPVS(pEnemy->s.origin, pNode->position)*/ )
		{
			float fDistance = VectorDistance(pNode->position, self->s.origin);
			
			if ( fDistance < fMinDist)
			{
				tr = gstate->TraceLine_q2( pEnemy->s.origin, pNode->position, pEnemy, MASK_MONSTERSOLID);//MASK_SOLID );
				if ( tr.fraction < 1.0f &&  AI_FindPathToNode( self, pNode ) != 0 )
				{
					// do two traces from both sides to make sure that the hide node is a good hiding place
					CVector directionVector;
					directionVector.SetX( pEnemy->s.origin.x - pNode->position.x );
					directionVector.SetY( pEnemy->s.origin.y - pNode->position.y );
					directionVector.SetZ( 0.0f );
					directionVector.Normalize();
					
					// left side
					CVector leftVector( directionVector.y, -directionVector.x, 0.0f );

					// right side
					CVector rightVector( -directionVector.y, directionVector.x, 0.0f );

					CVector leftPos, rightPos;
					VectorMA( pNode->position, leftVector, fHalfWidth, leftPos );
					VectorMA( pNode->position, rightVector, fHalfWidth, rightPos );
					
					trace_t leftTrace = gstate->TraceLine_q2( pEnemy->s.origin, leftPos, pEnemy, MASK_SOLID );
					if ( leftTrace.fraction < 1.0f )
					{
						trace_t rightTrace = gstate->TraceLine_q2( pEnemy->s.origin, rightPos, pEnemy, MASK_SOLID );
						if ( rightTrace.fraction < 1.0f )
						{
							fMinDist = fDistance;
							nHideNodeIndex = i;
						}
					}
				}
			}
		}
	}

	if ( nHideNodeIndex != -1 )
	{
		return NODE_GetNode( pNodeHeader, nHideNodeIndex );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestPartialHideNode
// Description:
//				Find a closest hide node that is partially not visible from the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestPartialHideNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy )
{
	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nHideNodeIndex = -1;
	float fMinDist = MIN_HIDE_NODE_DISTANCE;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_HIDE) /*&& !gstate->inPVS(pEnemy->s.origin, pNode->position)*/ )
		{
			float fDistance = VectorDistance(pNode->position, self->s.origin);
			if ( fDistance < fMinDist )
			{
				tr = gstate->TraceLine_q2( pEnemy->s.origin, pNode->position, pEnemy, MASK_SOLID );
				if ( tr.fraction < 1.0f )
				{
					fMinDist = fDistance;
					nHideNodeIndex = i;
				}
			}
		}
	}

	if ( nHideNodeIndex != -1 )
	{
		return NODE_GetNode( pNodeHeader, nHideNodeIndex );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestHideNodes
// Description:
//				Find a max of 4 closest hide node within a certain distance 
//				that is partially not visible from the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define NODE_CLOSEST_HIDE_DISTANCE	768.0f

int NODE_GetClosestHideNodes( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy, MAPNODE_PTR *pHideNodes )
{
	_ASSERTE( pHideNodes );
    if (pEnemy == NULL || self == NULL || pHideNodes == NULL) 
	{
		return NULL;
	}

	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nNumHideNodes = 0;

	float fMinDist = NODE_CLOSEST_HIDE_DISTANCE;
	for ( int i = 0; i < pNodeHeader->nNumNodes && nNumHideNodes < MAX_CLOSEST_HIDENODES; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_HIDE) )
		{
			float fDistance = VectorDistance(pNode->position, self->s.origin);
			if ( fDistance < fMinDist )
			{
				tr = gstate->TraceLine_q2( pEnemy->s.origin, pNode->position, pEnemy, MASK_SOLID );
				if ( tr.fraction < 1.0f )
				{
					pHideNodes[nNumHideNodes++] = pNode;	
				}
			}
		}
	}

	return nNumHideNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestVisibleNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestVisibleNode( NODEHEADER_PTR pNodeHeader, userEntity_t *self, userEntity_t *pEnemy )
{
	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nVisibleNodeIndex = -1;
	float fMinDistance = MIN_HIDE_NODE_DISTANCE;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		float fDistance = VectorDistance(self->s.origin, pNode->position);
		if ( fDistance < fMinDistance )
		{
			tr = gstate->TraceLine_q2( pEnemy->s.origin, pNode->position, pEnemy, MASK_SOLID );
			if ( tr.fraction >= 1.0f )
			{
				fMinDistance = fDistance;
				nVisibleNodeIndex = i;
			}
		}
	}

	if ( nVisibleNodeIndex != -1 )
	{
		return NODE_GetNode( pNodeHeader, nVisibleNodeIndex );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestSnipeNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestSnipeNode( userEntity_t *self )
{
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );

	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nSnipeNodeIndex = -1;
	float fMinDistance = 1e20;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_SNIPE) )
		{
			float fDistance = VectorDistance(self->s.origin, pNode->position);
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;
				nSnipeNodeIndex = i;
			}
		}
	}

	if ( nSnipeNodeIndex != -1 )
	{
		return NODE_GetNode( pNodeHeader, nSnipeNodeIndex );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestSnipeNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define NODE_CLOSEST_SNIPE_DISTANCE	512.0f

int NODE_GetClosestSnipeNodes( userEntity_t *self, MAPNODE_PTR *pSnipeNodes )
{
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );

	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nNumSnipeNodes = 0;
	for ( int i = 0; i < pNodeHeader->nNumNodes && nNumSnipeNodes < MAX_CLOSEST_SNIPENODES; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_SNIPE) )
		{
			float fDistance = VectorDistance(self->s.origin, pNode->position);
			if ( fDistance < NODE_CLOSEST_SNIPE_DISTANCE )
			{
				pSnipeNodes[nNumSnipeNodes++] = pNode;
			}
		}
	}

	return nNumSnipeNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetMoveAwayNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetMoveAwayNode( userEntity_t *anchorEntity, userEntity_t *self, float fMinDistance )
{
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );

	if ( !pNodeHeader )
	{
		return NULL;
	}

	MAPNODE_PTR pClosestNode = NODE_GetClosestNode( self );
	for ( int i = 0; i < pClosestNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pClosestNode->aLinks[i].nIndex );
		float fDistanceToAnchor = VectorDistance( pNode->position, anchorEntity->s.origin );
		float fDistanceToSelf = VectorDistance( pNode->position, self->s.origin );

		if ( fDistanceToAnchor > fDistanceToSelf && fDistanceToSelf >= fMinDistance )
		{
			return pNode;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		NODE_GetClosestRetreatNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR NODE_GetClosestRetreatNode( userEntity_t *self )
{
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );

	if ( !pNodeHeader )
	{
		return NULL;
	}

	int nRetreatNodeIndex = -1;
	float fMinDistance = 1e20;
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, i );
		if ( (pNode->node_type & NODETYPE_RETREAT) )
		{
			float fDistance = VectorDistance(self->s.origin, pNode->position);
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;
				nRetreatNodeIndex = i;
			}
		}
	}

	if ( nRetreatNodeIndex != -1 )
	{
		return NODE_GetNode( pNodeHeader, nRetreatNodeIndex );
	}

	return NULL;
}

int NODE_IsTriggered( MAPNODE_PTR pNode )
{
    return pNode->bTriggered;
}

void NODE_SetTriggered( MAPNODE_PTR pNode )
{
    _ASSERTE( pNode );
    pNode->bTriggered = TRUE;
}

// ----------------------------------------------------------------------------

//*******************************************************************************
//	path finding functions
//*******************************************************************************



//*****************************************************************************
//								PATH node stuff
//*****************************************************************************

#define MAX_NUM_PATHNODES	500

static PATHNODE_PTR pStartFreePathNode = NULL;
static PATHNODE_PTR pCurrentFreePathNode = NULL;
static int nNumFreePathNodes = 0;


// ----------------------------------------------------------------------------
//
// Name:			PATH_PreAllocatePathNodes
// Description:
//					allcate memory for the path nodes
// Input:
// Output:
//					TRUE	=> if successful
//					FALSE	=> otherwise
// Note:
//					this chunk of memory is pre-allocated so that we don't have to
//					dynamically allocate every time a path node is needed
//
// ----------------------------------------------------------------------------
int PATH_PreAllocatePathNodes()
{
	pStartFreePathNode = (PATHNODE_PTR)gstate->X_Malloc(sizeof(PATHNODE)*MAX_NUM_PATHNODES,MEM_TAG_NODE);
//   pStartFreePathNode = new PATHNODE[MAX_NUM_PATHNODES];
//   if ( pStartFreePathNode == NULL )
//   {
//      return FALSE;
//   }
//   memset( pStartFreePathNode, 0, MAX_NUM_PATHNODES * sizeof(PATHNODE) );

   pCurrentFreePathNode = pStartFreePathNode;
   PATHNODE_PTR pPrevNode = pCurrentFreePathNode;

   // link the free nodes together
   for ( int i = 1; i < MAX_NUM_PATHNODES; i++ )
   {
      PATHNODE_PTR pTempNode = &(pCurrentFreePathNode[i]);
      pTempNode->pNextFreeNode = NULL;
      pPrevNode->pNextFreeNode = pTempNode;

      pPrevNode = pTempNode;
   }

   nNumFreePathNodes = MAX_NUM_PATHNODES;

   return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		PATH_DestroyPreAllocatedPathNodes
// Description:
//				destroy pre-allocated path nodes
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATH_DestroyPreAllocatedPathNodes()
{
   //delete pStartFreePathNode;
	gstate->X_Free(pStartFreePathNode);
   pStartFreePathNode = NULL;

   nNumFreePathNodes = 0;
}

// ----------------------------------------------------------------------------
//
// Name:          GetFreePathNode
// Description:   Get next free node
// Input:
// Output:		  NULL => if no more free nodes
// Note:
//                get a free node from a pre-allocated node table
//
// ----------------------------------------------------------------------------
PATHNODE_PTR PATH_GetFreePathNode()
{
	PATHNODE_PTR pNode = pCurrentFreePathNode;

	if(pCurrentFreePathNode->pNextFreeNode == NULL)
	{
		return NULL;
	}
	//_ASSERTE(pCurrentFreePathNode->pNextFreeNode != NULL);

	pCurrentFreePathNode = pCurrentFreePathNode->pNextFreeNode;

	
	nNumFreePathNodes--;

	if ( pNode == NULL )
	{
		AI_Dprintf( "No more free path nodes.\n" );
	}

	return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:          DeleteFreePathNode
// Description:   free a node
// Input:
// Output:
// Note:
//                return a node to a pool of nodes
//
// ----------------------------------------------------------------------------
void PATH_DeletePathNode( PATHNODE_PTR pNode )
{
	
	_ASSERTE(pCurrentFreePathNode!=NULL);
	memset( pNode, 0, sizeof(PATHNODE) );

	PATHNODE_PTR pTempNode = pCurrentFreePathNode;
	_ASSERT(pCurrentFreePathNode!=NULL);
	pCurrentFreePathNode = pNode;
	pCurrentFreePathNode->pNextFreeNode = pTempNode;

	nNumFreePathNodes++;
}

// ----------------------------------------------------------------------------
//
// Name:		path_add_node
// Description:
//	adds a node to the beginning of a path
//	a path is a list of nodes that each point to a map_node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHNODE_PTR path_add_node( PATHNODE_PTR path, MAPNODE_PTR map_node )
{
	_ASSERTE( pStartFreePathNode != NULL );

	PATHNODE_PTR last_node = NULL;
	PATHNODE_PTR new_node = NULL;

	if (path == NULL)
	{
		// set up first path node
		// we are using malloc because these nodes have to
		// be deallocated each time a new path is formed
//		AI_Dprintf ("creating path: malloced %i bytes\n", sizeof (PATHNODE));
		path = PATH_GetFreePathNode();
		if ( path )
		{
			path->nNodeIndex = map_node->nIndex;
			path->next_node = NULL;
		}

		return	path;
	}

	// append a path node
//	AI_Dprintf ("appending path: malloced %i bytes\n", sizeof (PATHNODE));
	new_node = PATH_GetFreePathNode();
	if ( new_node )
	{
		new_node->nNodeIndex = map_node->nIndex;
		new_node->next_node = path;
	}

	path = new_node;

	return	path;
}

// ----------------------------------------------------------------------------
//
// Name:		path_delete_first
// Description:
//	deletes and deallocates the first node in a path list
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHNODE_PTR path_delete_first( PATHNODE_PTR path )
{
	PATHNODE_PTR temp;

	if (path)
	{
		temp = path->next_node;
		PATH_DeletePathNode( path );
		return	temp;
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		path_kill
// Description:
//	deletes and deallocates an entire path
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHNODE_PTR path_kill( PATHNODE_PTR path )
{
	PATHNODE_PTR temp;

	while (path)
	{
		temp = path->next_node;
		PATH_DeletePathNode( path );
		path = temp;
	}

	return	path;
}


// ----------------------------------------------------------------------------
//
// Name:		PATH_Length
// Description:
//	returns the number of nodes in a path
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	PATH_Length( PATHNODE_PTR path )
{
	int				i = 0;
	PATHNODE_PTR	path_node;

	for (path_node = path; path; path = path->next_node)
	{
		i++;
	}

	return	i;
}

// ----------------------------------------------------------------------------
//
// Name:		PATHLIST_KillPath
// Description:	removes all of the nodes in a path list's path
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATHLIST_KillPath(PATHLIST_PTR pPathList)
{
	PATHNODE_PTR next = NULL;

	if(pPathList)
	{
		while (pPathList->pPath)
		{
			next = pPathList->pPath->next_node;

			PATH_DeletePathNode (pPathList->pPath);
			pPathList->nPathLength--;

			pPathList->pPath = next;
		}

		//	make sure length == 0
		_ASSERTE (pPathList->nPathLength == 0);
		_ASSERTE (pPathList->nPathLength == NULL);
		
		pPathList->nMaxPathLength = 0;
		pPathList->nPathLength = 0;
		pPathList->pPathEnd = NULL;
		pPathList->pPath = NULL;
	
	}

}

// ----------------------------------------------------------------------------
//
// Name:        PATHLIST_Create
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHLIST_PTR PATHLIST_Create()
{
//	PATHLIST_PTR pPathList = new PATHLIST;
	PATHLIST_PTR pPathList = (PATHLIST_PTR)gstate->X_Malloc(sizeof(PATHLIST),MEM_TAG_NODE);

	pPathList->pPath = NULL;
	pPathList->pPathEnd = NULL;
	pPathList->nPathLength = 0;
	pPathList->nMaxPathLength = 50;

	return	pPathList;
}

// ----------------------------------------------------------------------------
//
// Name:        PATHLIST_Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHLIST_PTR PATHLIST_Destroy( PATHLIST_PTR pPathList )
{
	PATHLIST_KillPath(pPathList);

	//delete (pPathList);
	gstate->X_Free(pPathList);

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        PATHLIST_DeleteFirstInPath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATHLIST_DeleteFirstInPath( PATHLIST_PTR pPathList )
{
	if ( pPathList && pPathList->pPath )
	{
		if ( pPathList->pPath == pPathList->pPathEnd )
		{
			pPathList->pPathEnd = NULL;
		}

		pPathList->pPath = path_delete_first (pPathList->pPath);
		pPathList->nPathLength--;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        PATHLIST_AddNodeToPath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	PATHLIST_AddNodeToPath (PATHLIST_PTR pPathList, MAPNODE_PTR pNode)
{
	PATHNODE_PTR pNewListHead = path_add_node (pPathList->pPath, pNode);

	//_ASSERTE (pNewListHead != NULL);
	if (pNewListHead == NULL)
	{
		return false;
	}

	if (pPathList->nPathLength == 0)
	{
		//	there is only one node in the list, so set the end node to it
		pPathList->pPathEnd = pNewListHead;
	}

	pPathList->pPath = pNewListHead;
	pPathList->nPathLength++;

	return true;
}

//-----------------------------------------------------------------------------

#if 0
// ----------------------------------------------------------------------------
//
// Name:		node_marker_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void node_marker_think( userEntity_t *marker )
{
	_ASSERTE( marker );

	if ( marker->s.render_scale.x > 2.0 )
	{
		marker->delay = 0.95;
	}
	else 
	if ( marker->s.render_scale.x < 0.5 )
	{
		marker->delay = 1.05;
	}

	marker->s.render_scale = marker->s.render_scale * marker->delay;

	marker->think = node_marker_think;
	marker->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:		node_set_marker
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void node_set_marker( userEntity_t *viewer, MAPNODE_PTR pNode, char *model )
{
	_ASSERTE( viewer );

	if ( !bShowNodes )
	{
		return;
	}

	if ( pNode )
	{
		if ( pNode->marker )
		{
			pNode->marker->s.modelindex = gstate->ModelIndex(model);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		node_visible
// Description:
//	trace a line from self->s.origin to node->origin to determine
//	if there is anything blocking the view of this node  
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int node_visible( userEntity_t *ignore, CVector start, CVector end )
{
	gstate->TraceLine( start, end, TRUE, ignore, &trace );
	
	if ( trace.fraction == 1.0f )
	{
		return TRUE;
	}
	
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		node_place_spawn_node
// Description:
//	places a node at a spawn position if there is no other node
//	visible from self's current location.  Otherwise, the visible
//	node is considered the current node.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void node_place_spawn_node( userEntity_t *self, NODELIST_PTR pNodeList )
{
	_ASSERTE( self );
	_ASSERTE( pNodeList );
	_ASSERTE( pNodeList->pNodeHeader );
	MAPNODE_PTR pNodes = pNodeList->pNodeHeader->pNodes;	

	pNodeList->nCurrentNodeIndex = -1;
	pNodeList->nLastNodeIndex = -1;

	MAPNODE_PTR pFoundNode = NULL;

	float fMinDistance = 1e20;
	for ( int i = 0; i < pNodeList->pNodeHeader->nNumNodes; i++ )
	{
		float fDistance = VectorDistance( self->s.origin, pNodes[i].position );
		if ( fDistance < fMinDistance && 
			 node_visible(self, pNodes[i].position, self->s.origin) )
		{
			pFoundNode = &(pNodes[i]);
			fMinDistance = fDistance;
		}
	}

	if ( pFoundNode )
	{
		pNodeList->nLastNodeIndex = -1;
		pNodeList->nCurrentNodeIndex = pFoundNode->nIndex;
	}
	else
	{
		if ( NODE_IsOkToLayNodes() == TRUE )
		{
			int nNodeIndex = NODE_Add( pNodeList->pNodeHeader, self->s.origin );
			pNodeList->nLastNodeIndex = -1;
			pNodeList->nCurrentNodeIndex = nNodeIndex;
		}
	}

	//node_log_items( self, pNodeList );

	if ( pNodeList->nCurrentNodeIndex != -1 )
	{
		MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeList->pNodeHeader, pNodeList->nCurrentNodeIndex );
		//	put this entities time_stamp here
		//node_stamp_time( self, pCurrentNode );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		node_next_travel
// Description:
//	chooses a random node linked to current node that
//	is not the last node visited
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR node_next_travel( userEntity_t *self, NODELIST_PTR pNodeList )
{
	int			last, num_links;

	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, pNodeList->nLastNodeIndex );

	// find the number of beast... I mean link that links to last_node
	// and the number of nodes that this node is linked to
	for ( int i = 0; i < pCurrentNode->nNumLinks; i++ );
	{
		MAPNODE_PTR pLinkNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex ); //&(pNodeHeader->pNodes[pCurrentNode->aLinks[i].nIndex]);
		if ( pLinkNode == pLastNode )
		{
			last = i;
		}
	}

	num_links = i;

	if ( pCurrentNode->nNumLinks > 1 )
	{
		// choose a random node that isn't last node
		int j;
		do
		{
			j = (int) (rnd () * (float)pCurrentNode->nNumLinks);
		} while (j == last);

		MAPNODE_PTR pLinkNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[j].nIndex ); //&(pNodeHeader->pNodes[pCurrentNode->aLinks[j].nIndex]);
		if ( pLinkNode == pLastNode )
		{
			AI_Dprintf( "screwed! going to last node!!\n" );
		}
		else 
		if ( pLinkNode == pCurrentNode )
		{
			AI_Dprintf( "double screwed! node linked to itself!!\n" );
		}

		if ( pLinkNode == NULL )
		{
			AI_Dprintf( "num links = %i\n", num_links );
			AI_Dprintf( "chose link #%i\n", j );
		}

		return	pLinkNode;
	}
	else 
	if ( pCurrentNode->nNumLinks == 1 )
	{
		// only node is the node we came from
		return	&(pNodeHeader->pNodes[pCurrentNode->aLinks[0].nIndex]);
	}
	else
	{
		// at a node with no links to anywhere.
		// most likely a spawn node that hasn't been linked yet, 
		// but could be a node that a fall is required to reach
		// and which was never linked to
		// FIXME: should we flag and release such nodes if they 
		// don't get new links?

		return	NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		node_can_path
// Description:
//	recursive search to determine if one node
//	is reachable from another node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	node_can_path( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode, int depth )
{
	if ( pStartNode == pEndNode )
	{
		return	TRUE;
	}
	else 
	if ( depth == 0 )
	{
		return FALSE;
	}

	for ( int i = 0; i < pStartNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pLinkNode = NODE_GetNode( pNodeHeader, pStartNode->aLinks[i].nIndex );//&(pNodeHeader->pNodes[pStartNode->aLinks[i].nIndex]);

		if ( pLinkNode )
		{
			if ( node_can_path( pNodeHeader, pLinkNode, pEndNode, depth - 1 ) )
			{
				return TRUE;
			}
		}
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		node_closest
// Description:
//				returns the node that is closest
//				to and within dist units of origin
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR node_closest( NODELIST_PTR pNodeList, CVector &origin, float dist )
{
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pClosestNode = NULL;
	
	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
		_ASSERTE( pNode );

		float fDistance = VectorDistance( pNode->position, origin );
		if ( fDistance < dist )
		{
			dist = fDistance;
			pClosestNode = pNode;
		}
	}

	return	pClosestNode;
}

// ----------------------------------------------------------------------------
//
// Name:		node_linked_with_target
// Description:
//	returns the node that is closest
//	to and within dist units of origin
// Input:
//Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR node_linked_with_target( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pNode, char *targetname)
{
	for ( int i = 0; i < pNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pLinkNode = NODE_GetNode( pNodeHeader, pNode->aLinks[i].nIndex );//&(pNodeHeader->pNodes[pNode->aLinks[i].nIndex]);
		if ( pLinkNode && pLinkNode->target )
		{
			if ( !stricmp( pLinkNode->target, targetname ) )
			{
				return pLinkNode;
			}
		}
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		node_find_node
// Description:
//	returns the closest node within dist units of origin
//	that is of the same node_type as type
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static MAPNODE_PTR node_find_node( NODELIST_PTR pNodeList, const CVector &org, float dist, int type )
{
	_ASSERTE( pNodeList );

	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	for ( int i = 0; i < pNodeHeader->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = &(pNodeHeader->pNodes[i]);
		if ( pNode->node_type & type )
		{
			float fDistance = VectorDistance( pNode->position, org );
			if ( fDistance <= dist )
			{
				return pNode;
			}
		}
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		path_last_node
// Description:
//
//	returns the last node in a path
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
PATHNODE_PTR path_last_node(PATHNODE_PTR path)
{
	PATHNODE_PTR head;

	head = path;
	while (head && head->next_node)
	{
		head = head->next_node;
	}

	return head;
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
void world_nodelist_register_func()
{
//	gstate->RegisterFunc("node_marker_think",node_marker_think);
}
 