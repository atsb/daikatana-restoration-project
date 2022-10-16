// ==========================================================================
//  File:       GrphPath.cpp
//  Contents:
//              Implements an A* shortest path algorithm
//  Author:
//
// ==========================================================================


/*****************************************************************************/
/******************************* A* Algorithm ********************************/
/*****************************************************************************/
#include <stdio.h>
#if _MSC_VER
#include <crtdbg.h>
#endif

#include "world.h"
#include "nodelist.h"
#include "GrphPath.h"


/* ***************************** define types **************************** */

#define ADD_START_NODE	

#ifndef FALSE
#define FALSE			0
#endif

#ifndef TRUE
#define TRUE			1
#endif

#define NODE_IN_VOID	0
#define NODE_IN_OPEN	1
#define NODE_IN_CLOSED	2

#define MAX_NUM_NODES   400

//#define _DEBUG_CONSOLE_PATH  // uncomment this if you want to see path debug printouts on screen

// 40 bytes in following struct
typedef struct _node
{
   float f, g, h;
   float x, y, z;		// 3D
   int  nIndex;
   struct _node *pParent;
   struct _node *pPrevNode;
   struct _node *pNextNode;
   struct _node *pNextFreeNode;
} NODE, *NODEPTR;

/* ***************************** Local Variables **************************** */

static NODEHEADER_PTR pCurrentNodeHeader = NULL;// SCG[1/23/00]: not used
static NODEPTR pStartFreeNode = NULL;
static NODEPTR pCurrentFreeNode = NULL;
static int nNumFreeNodes = 0;// SCG[1/23/00]: not used
static NODEPTR OPEN   = NULL;
static NODEPTR CLOSED = NULL;

static int nNumInOPEN   = 0;// SCG[1/23/00]: not used
static int nNumInCLOSED = 0;// SCG[1/23/00]: not used

// save these info, so we don't have to pass them to functions
static NODEPTR pCurrentBestNode  = NULL;

static NODE startNode;
static NODE destNode;

/* ***************************** Local Functions **************************** */

// function prototypes
static float GoalDistanceEstimate( NODEPTR pNode );
static NODEPTR FindPath( NODEHEADER_PTR pNodeHeader );
static int GenerateSucc( MAPNODE_PTR pNode, short nDistance );

static NODEPTR ReturnBestNodeFromOpen();
static NODEPTR CheckOPEN( int nIndex );
static void InsertOPEN( NODEPTR Successor );
static void RemoveOPEN( NODEPTR pNode );

static NODEPTR CheckCLOSED( int nIndex );
static void InsertCLOSED( NODEPTR pNode );
static void RemoveCLOSED( NODEPTR pNode );


// ----------------------------------------------------------------------------
//                            LOCAL FUNCTIONS
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:          InitGraphPath
// Description:   Initialize the graph path variables
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void InitGraphPath()
{
   pCurrentFreeNode = pStartFreeNode;
   NODEPTR pPrevNode = pCurrentFreeNode;
   // link the free nodes together
   for ( int i = 1; i < MAX_NUM_NODES; i++ )
   {
      NODEPTR pTempNode = &(pCurrentFreeNode[i]);
	  pTempNode->pNextFreeNode = NULL;
      pPrevNode->pNextFreeNode = pTempNode;

      pPrevNode = pTempNode;
   }

   nNumFreeNodes = MAX_NUM_NODES;

   //////////////////////////////////////////////////////////////
   // initialize local variable to this module
   nNumInOPEN   = 0;
   nNumInCLOSED = 0;
}

// ----------------------------------------------------------------------------
//
// Name:          GetFreeNode
// Description:   Get next free node
// Input:
// Output:		  NULL => if no more free nodes
// Note:
//                get a free node from a pre-allocated node table
//
// ----------------------------------------------------------------------------
__inline static NODEPTR GetFreeNode()
{
   //_ASSERTE( pCurrentFreeNode != NULL );

   NODEPTR pNode	 = pCurrentFreeNode;

   pCurrentFreeNode = pCurrentFreeNode->pNextFreeNode;

   nNumFreeNodes--;

   //_ASSERTE( nNumFreeNodes >= 0 );

   return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:          DeleteFreeNode
// Description:   free a node
// Input:
// Output:
// Note:
//                return a node to a pool of nodes
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
__inline static void DeleteFreeNode( NODEPTR pNode )
{
   _ASSERTE( pCurrentFreeNode != NULL );

   memset( pNode, 0, sizeof(NODE) );

   NODEPTR pTempNode = pCurrentFreeNode;

   pCurrentFreeNode = pNode;
   pCurrentFreeNode->pNextFreeNode = pTempNode;

   nNumFreeNodes++;
}
*/
// ----------------------------------------------------------------------------
//
// Name:          GoalDistanceEstimate
// Description:   estimate a distance to the goal
// Input:
// Output:
// Note:
//                uses a manhattan distance formula for estimation
//
// ----------------------------------------------------------------------------
__inline static float GoalDistanceEstimate( NODEPTR pNode )
{
   int dx = (pNode->x - destNode.x);
   int dy = (pNode->y - destNode.y);
   int dz = (pNode->z - destNode.z);

   // Manhattan (dx + dy)
   return (float)( abs(dx) + abs(dy) + abs(dz) );
}

///////////////////////////////////////////////////////////////////////////////
// operations on OPEN list

// ----------------------------------------------------------------------------
//
// Name:          ReturnBestNodeFromOpen
// Description:   get the best node from the OPEN list
// Input:
// Output:
//                NODEPTR  => pointer to best current node
// Note:
//
// ----------------------------------------------------------------------------
__inline static NODEPTR ReturnBestNodeFromOpen(void)
{
   NODEPTR pTempNode;

   // Pick Node with lowest f, in this case it's the first node in list
   // because we sort the OPEN list with lowest f. Call it BESTNODE.

   pTempNode = OPEN->pNextNode;              // point to first node on OPEN

   NODEPTR pNextNode = pTempNode->pNextNode;
   OPEN->pNextNode = pNextNode;              // Make OPEN point to nextnode or NULL.
   if ( pNextNode != NULL )
   {
      pNextNode->pPrevNode = OPEN;
   }

   nNumInOPEN--;

   return(pTempNode);
}

// ----------------------------------------------------------------------------
//
// Name:          CheckOPEN
// Description:   check the OPEN list for the same node
// Input:
//                INT_POINT2DPTR => position of the node to check for
// Output:
//                NODEPTR  => found NODE
//                NULL     => no matching NODE found
// Note:
//
// ----------------------------------------------------------------------------
__inline static NODEPTR CheckOPEN( int nIndex )
{
   NODEPTR pTempNode;

   pTempNode = OPEN->pNextNode;
   while ( pTempNode != NULL )
   {
      if ( pTempNode->nIndex == nIndex )
      {
         return pTempNode;
      }

      pTempNode = pTempNode->pNextNode;
   }
   return (NULL);
}

// ----------------------------------------------------------------------------
//
// Name:          InsertOPEN
// Description:
//                Insert a NODE in the OPEN list
// Input:
//                NODEPTR  => node to be inserted
// Output:
// Note:
//                keeps a priority list
//
// ----------------------------------------------------------------------------
__inline static void InsertOPEN( NODEPTR pNode )
{
   _ASSERTE( pNode != NULL );
   _ASSERTE( pNode != pNode->pPrevNode && pNode != pNode->pNextNode );

   NODEPTR pTemp1;
   NODEPTR pTemp2;
   float f;

   f = pNode->f;
   pTemp1 = OPEN;
   pTemp2 = OPEN->pNextNode;

   // keep the lowest f at the front of the list
   while ( (pTemp2 != NULL) && (pTemp2->f < f) )
   {
      pTemp1 = pTemp2;
      pTemp2 = pTemp2->pNextNode;
   }
   _ASSERTE( pTemp1 != NULL );
   _ASSERTE( pTemp1 != pNode && pTemp2 != pNode );

   pTemp1->pNextNode = pNode;
   pNode->pPrevNode = pTemp1;

   pNode->pNextNode = pTemp2;
   if ( pTemp2 != NULL )
   {
      pTemp2->pPrevNode = pNode;
   }

   nNumInOPEN++;

// check to make sure we have a priority list
#ifdef _DEBUG_PATH
   NODEPTR pTempNode = OPEN->pNextNode;
   float fPreviousF = pTempNode->f;
   
   pTempNode = pTempNode->pNextNode;
   while ( pTempNode != NULL )
   {
      _ASSERTE( fPreviousF <= pTempNode->f );
      fPreviousF = pTempNode->f;

      pTempNode = pTempNode->pNextNode;
   }
#endif _DEBUG_PATH
}

// ----------------------------------------------------------------------------
//
// Name:          RemoveOPEN
// Description:
//                Remove a node from the OPEN list
// Input:
//                NODEPTR  => node to be removed
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void RemoveOPEN( NODEPTR pNode )
{
   _ASSERTE( pNode != NULL );
   _ASSERTE( pNode != pNode->pPrevNode && pNode != pNode->pNextNode );

   NODEPTR pPrevNode = pNode->pPrevNode;
   NODEPTR pNextNode = pNode->pNextNode;

   pPrevNode->pNextNode = pNextNode;
   if ( pNextNode != NULL )
   {
      pNextNode->pPrevNode = pPrevNode;
   }

   pNode->pParent = NULL;
   pNode->pPrevNode = NULL;
   pNode->pNextNode = NULL;

   nNumInOPEN--;

#ifdef _DEBUG_PATH
   NODEPTR pTempNode = OPEN;
   while ( pTempNode != NULL )
   {
      _ASSERTE( pTempNode != pNode );
      _ASSERTE( pTempNode != pTempNode->pNextNode );

      pTempNode = pTempNode->pNextNode;
   }
#endif _DEBUG_PATH
}

////////////////////////////////////////////////////////////////////////////////////
// operations on CLOSED list

// ----------------------------------------------------------------------------
//
// Name:          ClosedCLOSED
// Description:   
//                check CLOSED list for a matching NODE
// Input:
//                INT_POINT2DPTR => position of the node to check for
// Output:
//                NODEPTR  => a node is found
//                NULL     => no matching node is found
// Note:
//
// ----------------------------------------------------------------------------
__inline static NODEPTR CheckCLOSED( int nIndex )
{
   NODEPTR pTempNode;

   pTempNode = CLOSED->pNextNode;
   while ( pTempNode != NULL )
   {
      if ( pTempNode->nIndex == nIndex )
      {
         return pTempNode;
      }
      pTempNode = pTempNode->pNextNode;
   }
   return (NULL);
}

// ----------------------------------------------------------------------------
//
// Name:          InsertCLOSED
// Description:
//                Insert a node to CLOSED list
// Input:
//                NODEPTR  => pointer to a node
// Output:
// Note:
//                appends the node to the end
//
// ----------------------------------------------------------------------------
__inline static void InsertCLOSED( NODEPTR pNode )
{
   // insert the node at front of the list
   NODEPTR pTemp1;
   NODEPTR pTemp2;
   pTemp1 = CLOSED;
   pTemp2 = CLOSED->pNextNode;

   pTemp1->pNextNode = pNode;
   pNode->pPrevNode = pTemp1;

   pNode->pNextNode = pTemp2;
   if ( pTemp2 != NULL )
   {
      pTemp2->pPrevNode = pNode;
   }

   nNumInCLOSED++;
}

// ----------------------------------------------------------------------------
//
// Name:          RemoveCLOSED
// Description:
//                Remove a node from the CLOSED list
// Input:
//                NODEPTR  => pointer to a node to be removed
// Output:
// Note:
//                a node is removed from a preallocated pool of nodes
//
// ----------------------------------------------------------------------------
__inline static void RemoveCLOSED( NODEPTR pNode )
{
   _ASSERTE( pNode != NULL );
   _ASSERTE( pNode != pNode->pPrevNode && pNode != pNode->pNextNode );

   NODEPTR pPrevNode = pNode->pPrevNode;
   NODEPTR pNextNode = pNode->pNextNode;

   pPrevNode->pNextNode = pNextNode;
   if ( pNextNode != NULL )
   {
      pNextNode->pPrevNode = pPrevNode;
   }

   pNode->pParent = NULL;
   pNode->pPrevNode = NULL;
   pNode->pNextNode = NULL;

#ifdef _DEBUG_PATH
   NODEPTR pTempNode = CLOSED;
   while ( pTempNode != NULL )
   {
      _ASSERTE( pTempNode != pNode );
      _ASSERTE( pTempNode != pTempNode->pNextNode );

      pTempNode = pTempNode->pNextNode;
   }
#endif _DEBUG_PATH

   nNumInCLOSED--;
}

///////////////////////////////////////////////////////////////////////////////
//    The following A* algorithm is from Game Developer's Magazine 
//    October/November 1996 by Bryan Stout at page 31
//
// s.g = 0                          =>       s is the start node
// s.h = GoalDistanceEstimate( s ) 
// s.f = s.g + s.h
// s.parent = NULL
// push s on OPEN
// while OPEN is not empty
// {
//    pop best node n from OPEN     =>    n has the lowest f
//    if n is the goal node
//    {
//       construct path
//       return success
//    }
//    for each successor n' of n
//    {
//       newG = n.g + Cost( n, n' )
//       if n' is in OPEN or CLOSED, and n'.g <= newG
//       {
//          skip
//       }
//       n'.parent = n
//       n'.g = newG
//       n'.h = GoalDistanceExtimate( n' )
//       n'.f = n'.g + n'.h
//       if n' is in CLOSED
//       {
//          remove it from CLOSED
//       }
//       if n' is not yet in OPEN
//       {
//          push n' on OPEN
//       }
//    }
//    push n onto CLOSED
// }
// return failure                   => no path found
//
// ----------------------------------------------------------------------------
//
// Name:          GenerateSucc
// Description:   
//                Generate all possible nodes from the current node
// Input:
//                POINT_DISTANCE_PTR   => pointer to the current position
// Output:
//                TRUE  => if a successor nodes are generated or no error occurred
//                FALSE => error has occured while generating successors
// Note:
//
// ----------------------------------------------------------------------------
static int GenerateSucc( MAPNODE_PTR pNode, short nDistance )
{
   float newG, newH, newF;
   NODEPTR pSuccessor;
   int bInOPEN = FALSE;
   int bInCLOSED = FALSE;
   float x, y, z;
   
   x = pNode->position[0];
   y = pNode->position[1];
   z = pNode->position[2];

   // add the value of the tile, this way we can have different cost associated with
   // each tile and still find the cheapest route
   newG = pCurrentBestNode->g + nDistance;

   int dx = (x - destNode.x);
   int dy = (y - destNode.y);
   int dz = (z - destNode.z);

   // Manhattan (dx + dy)
   newH = (float)( abs(dx) + abs(dy) + abs(dz) );
   newF = newG + newH;

   pSuccessor = CheckOPEN( pNode->nIndex );
   if ( pSuccessor != NULL )
   {
      if ( pSuccessor->g <= newG )
      {
         // skip
         return TRUE;
      }
      else
      {
         RemoveOPEN( pSuccessor );
         
         pSuccessor->g = newG;
         pSuccessor->h = newH;
         pSuccessor->f = newF;
         pSuccessor->pParent = pCurrentBestNode;

         InsertOPEN( pSuccessor );
         return TRUE;
      }
      //bInOPEN = TRUE;
   }

   pSuccessor = CheckCLOSED( pNode->nIndex );
   if ( pSuccessor != NULL )
   {
      if ( pSuccessor->g <= newG )
      {
         // skip
         return TRUE;
      }
      bInCLOSED = TRUE;
   }
   // a node can be in one or the other, but not in both
   _ASSERTE( !(bInOPEN == TRUE && bInCLOSED == TRUE) );
      
   if ( bInCLOSED == TRUE )
   {
      _ASSERTE( bInOPEN == FALSE );

      _ASSERTE( pSuccessor->x == x && pSuccessor->y == y && pSuccessor->z == z );

      // remove from CLOSED
      RemoveCLOSED( pSuccessor );

      _ASSERTE( pSuccessor != NULL );
      // push Successor on OPEN
      pSuccessor->pParent = pCurrentBestNode;
      pSuccessor->x       = x;
      pSuccessor->y       = y;
	  pSuccessor->z		  = z;
      pSuccessor->nIndex  = pNode->nIndex;
      pSuccessor->g       = newG;
      pSuccessor->h       = GoalDistanceEstimate( pSuccessor );
      pSuccessor->f       = pSuccessor->g + pSuccessor->h;
   
      InsertOPEN( pSuccessor );

      return TRUE;
   }

   if ( bInOPEN == FALSE )
   {
      _ASSERTE( bInCLOSED == FALSE );

      // push Successor on OPEN
      pSuccessor          = GetFreeNode();
	  if ( !pSuccessor )
	  {
		return FALSE;
	  }
      pSuccessor->pParent = pCurrentBestNode;
      pSuccessor->x       = x;
      pSuccessor->y       = y;
	  pSuccessor->z		  = z;
      pSuccessor->nIndex  = pNode->nIndex;
      pSuccessor->g       = newG;
      pSuccessor->h       = GoalDistanceEstimate( pSuccessor );
      pSuccessor->f       = pSuccessor->g + pSuccessor->h;
      pSuccessor->pPrevNode = NULL;
      pSuccessor->pNextNode = NULL;
   
      InsertOPEN( pSuccessor );
   }

   return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:          IsAdjacentToDestNode
// Description:
//                check if passed in node is adjacent to the destination node
// Input:
//                NODEPTR  => pointer to the NODE to be tested
// Output:
//                TRUE  => if node is adjacent
//                FALSE => otherwise
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsAdjacentToDestNode( NODEHEADER_PTR pNodeHeader, NODEPTR pNode )
{
	MAPNODE_PTR pMapNode = &(pNodeHeader->pNodes[pNode->nIndex]);
	for ( int i = 0; i < pMapNode->nNumLinks; i++ )
	{
		if ( pMapNode->aLinks[i].nIndex == destNode.nIndex )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:          FindPath
// Description:
//                find a path to the destination
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static NODEPTR FindPath( NODEHEADER_PTR pNodeHeader )
{
	InitGraphPath();

	NODEPTR pNode = NULL;

	// initialize OPEN and CLOSED
	OPEN              = GetFreeNode();
	if ( !OPEN )
	{
		return NULL;
	}
	OPEN->pParent     = NULL;
	OPEN->pPrevNode   = NULL;
	OPEN->pNextNode   = NULL;
	CLOSED            = GetFreeNode();
	if ( !CLOSED )
	{
		return NULL;
	}
	CLOSED->pParent   = NULL;
	CLOSED->pPrevNode = NULL;
	CLOSED->pNextNode = NULL;

	// create a new start node
	pNode             = GetFreeNode();
	if ( !pNode )
	{
		return NULL;
	}
	pNode->x          = startNode.x;
	pNode->y          = startNode.y;
	pNode->z		  = startNode.z;
	pNode->nIndex     = startNode.nIndex;
	pNode->g          = 0.0f;
	pNode->h          = GoalDistanceEstimate( pNode );
	pNode->f          = pNode->g + pNode->h;
	pNode->pParent    = NULL;
	pNode->pPrevNode  = NULL;
	pNode->pNextNode  = NULL;

	// make Open List point to first node
	InsertOPEN( pNode );
	while ( OPEN->pNextNode != NULL )
	{
		// get the best node with lowest f
		pCurrentBestNode = (NODEPTR)ReturnBestNodeFromOpen();

		// check if the current best node is one adjacent node of the destination
		if ( IsAdjacentToDestNode( pNodeHeader, pCurrentBestNode ) == TRUE )
		{
			// if we've found the end, finish

			// push pCurrentBestNode onto CLOSED
			InsertCLOSED( pCurrentBestNode );
			return pCurrentBestNode;
		}

		MAPNODE_PTR pMapNode = &(pNodeHeader->pNodes[pCurrentBestNode->nIndex]);
		for ( int i = 0; i < pMapNode->nNumLinks; i++ )
		{
			MAPNODE_PTR pLinkNode = &(pNodeHeader->pNodes[pMapNode->aLinks[i].nIndex]);

			if ( GenerateSucc( pLinkNode, pMapNode->aLinks[i].nDistance ) == FALSE )
			{
				return NULL;
			}
		}

		// push pCurrentBestNode onto CLOSED
		InsertCLOSED( pCurrentBestNode );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//                            GLOBAL FUNCTIONS
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:          AllocateGraphPath
// Description:
//                allocate everything about graph path
// Input:
// Output:
//                
// Note:
//
// ----------------------------------------------------------------------------
int PATH_AllocateGraphPath()
{
   if ( pStartFreeNode != NULL )
   {
      // already allocated
      return FALSE;
   }

   pStartFreeNode = (NODE*)gstate->X_Malloc(sizeof(NODE)*MAX_NUM_NODES,MEM_TAG_NODE);
//   pStartFreeNode = new NODE[MAX_NUM_NODES];
   if ( pStartFreeNode == NULL )
   {
      com->Error( "Mem allocation failed." );
      return FALSE;
   }

   memset( pStartFreeNode, 0, MAX_NUM_NODES * sizeof(NODE) );

   return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:          DestroyGraphPath
// Description:   destroy all graph path
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATH_DestroyGraphPath()
{
//    delete [] pStartFreeNode;
	gstate->X_Free(pStartFreeNode);
   pStartFreeNode = NULL;

   nNumFreeNodes = 0;

   nNumInOPEN = 0;
   nNumInCLOSED = 0;
}

// ----------------------------------------------------------------------------
//
// Name:          ComputePath
// Description:   compute a path
// Input:
//                x1, y1      start point
//                x2, y2      dest point
// Output:
//                RESERVED_WAYPOINTSPTR   => waypoints to be returned 
// Note:
//
// ----------------------------------------------------------------------------
int	PATH_ComputePath( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, 
					  MAPNODE_PTR pEndNode, PATHLIST_PTR pPathList )
{
	int			nPathLength = 0;
	MAPNODE_PTR	pTempNode = NULL;

	_ASSERTE( pNodeHeader );

	pCurrentNodeHeader = pNodeHeader;

	if (pPathList->pPath != NULL)
	{
		//	kill path if it already exists
		PATHLIST_KillPath (pPathList);
	}
	if ( pStartNode == pEndNode )
	{
		PATHLIST_AddNodeToPath(pPathList, pEndNode);
		return 1;
	}

	PATHNODE_PTR pPathNode = NULL;

	if ( pNodeHeader->bUsePathTable )
	{
		//	using path table

		static short aPathLinks[MAX_NUM_NODES];
		int nNumLinks = 0;

		short **pPathTable = pNodeHeader->pPathTable;

		int nCurrentNodeIndex = pStartNode->nIndex;
		int nEndNodeIndex = pEndNode->nIndex;

		while ( nCurrentNodeIndex != nEndNodeIndex )
		{
			short nFirstPathIndex = pPathTable[nCurrentNodeIndex][nEndNodeIndex];
			if ( nFirstPathIndex < 0 )
			{
				return 0;
			}
			_ASSERTE( nCurrentNodeIndex != nFirstPathIndex );
			
			aPathLinks[nNumLinks++] = nFirstPathIndex;

			nCurrentNodeIndex = nFirstPathIndex;
		}

		// now connect them in reverse order
		nPathLength = 0;

		PATHLIST_AddNodeToPath(pPathList, pEndNode);
		pTempNode = &(pNodeHeader->pNodes[pPathList->pPath->nNodeIndex]);
		pPathList->pPathEnd = pPathList->pPath;

		if ( pTempNode )
		{
			nPathLength++;

			for ( int i = nNumLinks-1; i >= 0; i-- )
			{
				MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, aPathLinks[i] );
				_ASSERTE( pNode );

				if ( !PATHLIST_AddNodeToPath( pPathList, pNode ) )
				{
					PATHLIST_KillPath ( pPathList );
					return 0;
				}
				nPathLength++;

				_ASSERTE( nPathLength <= pNodeHeader->nNumNodes );
			}

#ifdef ADD_START_NODE
			// add the start node
			PATHLIST_AddNodeToPath( pPathList, pStartNode );
			nPathLength++;
#endif ADD_START_NODE
		}
	}
	else
	{
		//	not using path table

		//////////////////////////////////////////////////
		// save to local variables
		// reverse the start and dest so that we will get a link to the destination
		startNode.x = pStartNode->position[0];
		startNode.y = pStartNode->position[1];
		startNode.z = pStartNode->position[2];
		startNode.nIndex = pStartNode->nIndex;

		destNode.x = pEndNode->position[0];
		destNode.y = pEndNode->position[1];
		destNode.z = pEndNode->position[2];
		destNode.nIndex = pEndNode->nIndex;

		NODEPTR pPath = (NODEPTR)FindPath( pNodeHeader );
		if ( pPath == NULL )
		{
			return NULL;
		}

		// add the last node first since the link list expects in reverse order
		PATHLIST_AddNodeToPath(pPathList, pEndNode);
		pPathList->pPathEnd = pPathList->pPath;
		nPathLength = 1;

		while ( pPath != NULL && pPath->nIndex != pStartNode->nIndex )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[pPath->nIndex]);

			PATHLIST_AddNodeToPath( pPathList, pNode );
			nPathLength++;

			pPath = pPath->pParent;
		}
		
#ifdef ADD_START_NODE
		// add the start node
		PATHLIST_AddNodeToPath( pPathList, pStartNode );
		nPathLength++;
#endif ADD_START_NODE

		_ASSERTE( nPathLength <= pNodeHeader->nNumNodes );
	}

#ifdef _DEBUG
	{

		//_ASSERTE( pPathList->nPathLength == nPathLength );
	}
#endif _DEBUG
	if(pPathList->nPathLength != nPathLength)
	{
		pPathList	= NULL;
		nPathLength = 0;
	}
	return nPathLength;
}

// ----------------------------------------------------------------------------
//
// Name:          ComputePath
// Description:   compute a path
// Input:
//                x1, y1      start point
//                x2, y2      dest point
// Output:
//                RESERVED_WAYPOINTSPTR   => waypoints to be returned 
// Note:
//
// ----------------------------------------------------------------------------
PATHNODE_PTR PATH_ComputePath( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode, int &nPathLength )
{
	_ASSERTE( pNodeHeader );

	pCurrentNodeHeader = pNodeHeader;

	PATHNODE_PTR pPathNode = NULL;
	if ( pStartNode == pEndNode )
	{
		pPathNode = path_add_node( pPathNode, pEndNode );
		return pPathNode;
	}

	nPathLength = 0;

	if ( pNodeHeader->bUsePathTable )
	{
		static short aPathLinks[MAX_NUM_NODES];
		int nNumLinks = 0;

		short **pPathTable = pNodeHeader->pPathTable;

		int nCurrentNodeIndex = pStartNode->nIndex;
		int nEndNodeIndex = pEndNode->nIndex;

		while ( nCurrentNodeIndex != nEndNodeIndex )
		{
			short nFirstPathIndex = pPathTable[nCurrentNodeIndex][nEndNodeIndex];
			if ( nFirstPathIndex < 0 )
			{
				return NULL;
			}
			_ASSERTE( nCurrentNodeIndex != nFirstPathIndex );
			
			aPathLinks[nNumLinks++] = nFirstPathIndex;

			nCurrentNodeIndex = nFirstPathIndex;
		}

		// now connect them in reverse order
		PATHNODE_PTR pTempNode = path_add_node( pPathNode, pEndNode );
		if ( pTempNode )
		{
			pPathNode = pTempNode;
			nPathLength++;
			for ( int i = nNumLinks-1; i >= 0; i-- )
			{
				MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, aPathLinks[i] );
				_ASSERTE( pNode );

				pTempNode = path_add_node( pPathNode, pNode );
				if ( !pTempNode )
				{
					path_kill( pPathNode );
					return NULL;
				}
				pPathNode = pTempNode;
				nPathLength++;

				_ASSERTE( nPathLength <= pNodeHeader->nNumNodes );
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////
		// save to local variables
		// reverse the start and dest so that we will get a link to the destination
		startNode.x = pStartNode->position[0];
		startNode.y = pStartNode->position[1];
		startNode.z = pStartNode->position[2];
		startNode.nIndex = pStartNode->nIndex;

		destNode.x = pEndNode->position[0];
		destNode.y = pEndNode->position[1];
		destNode.z = pEndNode->position[2];
		destNode.nIndex = pEndNode->nIndex;

		NODEPTR pPath = (NODEPTR)FindPath( pNodeHeader );
		if ( pPath == NULL )
		{
			return NULL;
		}

		// add the last node first since the link list expects in reverse order
		pPathNode = path_add_node( pPathNode, pEndNode );	
		nPathLength++;
		while ( pPath != NULL && pPath->nIndex != pStartNode->nIndex )
		{
			MAPNODE_PTR pNode = &(pNodeHeader->pNodes[pPath->nIndex]);

			pPathNode = path_add_node( pPathNode, pNode );
			nPathLength++;

			pPath = pPath->pParent;
		}

		_ASSERTE( nPathLength <= pNodeHeader->nNumNodes );

	}

	return pPathNode;
}


// ----------------------------------------------------------------------------
//
// Name:          ComputePath
// Description:   This function is used in building the path table
// Input:
//                x1, y1      start point
//                x2, y2      dest point
// Output:
//                short		=> the first node index in the path list
// Note:
//
// ----------------------------------------------------------------------------
int PATH_ComputePath( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode )
{
	_ASSERTE( pNodeHeader );

	if ( pStartNode == pEndNode )
	{
		return 0;
	}

	pCurrentNodeHeader = pNodeHeader;

	PATHNODE_PTR pPathNode = NULL;

	//////////////////////////////////////////////////
	// save to local variables
	// reverse the start and dest so that we will get a link to the destination
	startNode.x = pStartNode->position[0];
	startNode.y = pStartNode->position[1];
	startNode.z = pStartNode->position[2];
	startNode.nIndex = pStartNode->nIndex;

	destNode.x = pEndNode->position[0];
	destNode.y = pEndNode->position[1];
	destNode.z = pEndNode->position[2];
	destNode.nIndex = pEndNode->nIndex;

	NODEPTR pPath = (NODEPTR)FindPath( pNodeHeader );
	if ( pPath == NULL )
	{
		return -1;
	}

	short nIndex = pEndNode->nIndex;

	while ( pPath != NULL && pPath->nIndex != pStartNode->nIndex )
	{
		nIndex = pPath->nIndex;

		pPath = pPath->pParent;
	}
	return nIndex;
}

// ----------------------------------------------------------------------------
//
// Name:          TestComputePath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATH_TestComputePath()
{
}
















#if 0


/*****************************************************************************/
/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////


/* take binary sign of a, either -1, or 1 if >= 0 */
//#define SGN(a)		(((a)<0) ? -1 : 1)

/*
 * digline: draw digital line from (x1,y1) to (x2,y2),
 * Does no clipping.  Uses Bresenham's algorithm.
 *
 * Paul Heckbert	3 Sep 85
 */

/*
//static int LineOfSight( int x1, int y1, int x2, int y2 )
BOOL LineOfSight( INT_POINT2DPTR pPoint1, INT_POINT2DPTR pPoint2 )
{
   int d, x, y, ax, ay, sx, sy, dx, dy;

   dx = pPoint2->x - pPoint1->x;  ax = abs(dx)<<1;  sx = SGN(dx);
   dy = pPoint2->y - pPoint1->y;  ay = abs(dy)<<1;  sy = SGN(dy);

   x = pPoint1->x;
   y = pPoint1->y;
   if (ax>ay) 
   {		
      // x dominant
      d = ay - ( ax >> 1 );
      for (;;) 
      {
         if ( pCurrentCostMap->pCosts[y][x] != 0 )
         {
            return FALSE;
         }
         if ( x == pPoint2->x ) 
         {
            return TRUE;
         }
         if ( d >= 0 ) 
         {
            y += sy;
            d -= ax;
         }
         x += sx;
         d += ay;
      }
   }
   else 
   {			
      // y dominant
      d = ax - ( ay >> 1 );
      for (;;) 
      {
         if ( pCurrentCostMap->pCosts[y][x] != 0 )
         {
            return FALSE;
         }
         if ( y== pPoint2->y ) 
         {
            return TRUE;
         }
         if ( d >= 0 ) 
         {
            x += sx;
            d -= ay;
         }
         y += sy;
         d += ax;
      }
   }

   return TRUE;
}
*/

/*
///////////////////////////////////////////////////////////////////////////////
//    The following A* algorithm is from a book called Search in Artificial 
//    Intelligence by L. Kanal and V. Kumar
//    ISBN 0-387-96750-8
//    ISBN 3-540-96750-8
//
// 1) Put the start node s, on a list called OPEN of unexpanded nodes.
// 2) If Open is empty, exit with failure; no soluntion exits
// 3) Remove from OPEN a node, an, at thwich f = g + h is minimum
//    (break ties arbitrarily, but in favor of a goal node) and place
//    it on a list called CLOSED to be used for expanded nodes.
// 4) If n is a goal node, exit successfully with the solution obtained
//    by tracing back the path along the pointers from n to s, (pointers
//    are assigned in Steps 5 and 6).
// 5) Expand node n, generating all its successors with pointers back to n.
// 6) For every successor n' of n:
//    a. Calculate f(n')
//    b. If n' was neither in OPEN nor in CLOSED, then add it to OPEN. Assign
//       the newly computed f(n') to node n'.
//    c. If n' already resided in OPEN or CLOSED, compare the newly computed 
//       f(n') with that previously assigned to n'. If the new value is lower, 
//       substitute it for the old (n' now points back to n instead of to its 
//       predecessor). If the matching node n' reside in CLOSED, move it back 
//       to OPEN.
// 7) Go to (2).
//
//
static BOOL 
GenerateSucc( INT_POINT2DPTR pPoint )
{
   float newG, newH, newF;
   NODEPTR pSuccessor;
   BOOL bInOPEN = FALSE;
   BOOL bInCLOSED = FALSE;

   // add the value of the tile, this way we can have different cost associated with
   // each tile and still find the cheapest route
   newG = pCurrentBestNode->g + Cost( pCurrentBestNode, pPoint );

   int dx = (pPoint->x - destPoint.x);
   int dy = (pPoint->y - destPoint.y);
   // Manhattan (dx + dy)
   newH = (float)( abs(dx) + abs(dy) );
   newF = newG + newH;

   // check if n' is already in OPEN or CLOSED
   pSuccessor = CheckOPEN( pPoint );
   if ( pSuccessor != NULL )
   {
      if ( newF < pSuccessor->f )
      {
         RemoveOPEN( pSuccessor );
         
         pSuccessor->g = newG;
         pSuccessor->h = newH;
         pSuccessor->f = newF;
         // now  this points back to current best node
         pSuccessor->pParent = pCurrentBestNode;

         InsertOPEN( pSuccessor );

         // skip
         return FALSE;
      }
      bInOPEN = TRUE;
   }

   
   pSuccessor = CheckCLOSED( pPoint );
   if ( pSuccessor != NULL )
   {
      if ( newF < pSuccessor->f )
      {
         //
         // if the mathching node n' is in CLOSED, move it back to OPEN
         //
         _ASSERTE( pSuccessor->x == pPoint->x && pSuccessor->y == pPoint->y );

         // remove from CLOSED
         RemoveCLOSED( pSuccessor, FALSE );

         // push Successor on OPEN
         pSuccessor->pParent = pCurrentBestNode;
         pSuccessor->x       = pPoint->x;
         pSuccessor->y       = pPoint->y;
         pSuccessor->g       = newG;
         pSuccessor->h       = newH;
         pSuccessor->f       = newF;

         InsertOPEN( pSuccessor );     // Insert Successor on OPEN list wrt f

         // skip
         return FALSE;
      }
      bInCLOSED = TRUE;
   }
      
   if ( bInCLOSED == FALSE && bInOPEN == FALSE )
   {
      // push Successor on OPEN
//      pSuccessor = (NODEPTR)malloc(sizeof(NODE ));
      pSuccessor->pParent  = pCurrentBestNode;
      pSuccessor->x        = pPoint->x;
      pSuccessor->y        = pPoint->y;
      pSuccessor->g        = newG;
      pSuccessor->h        = newH;
      pSuccessor->f        = newF;
      pSuccessor->pNextNode = NULL;
   
      InsertOPEN( pSuccessor );     // Insert Successor on OPEN list wrt f
   }

   return TRUE;
}
*/

#endif 0
