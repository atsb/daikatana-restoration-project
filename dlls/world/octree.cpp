// ==========================================================================
//
//  File:		Octree
//  Contents:
//              The purpose of this module is to place path nodes in a table
//              so that finding a closest node from any location in a map can
//              be very fast.  The map is sub-divided into octree until a desired
//              number of nodes are in a sub-divided area or until the width of
//              the box is 256 units.  Once the world has been completely divided,
//              the path nodes that are visible from a box is found and stored to 
//              be quickly retrieved when needed.
//  Author:
//
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
//#include "ai_func.h"// SCG[1/23/00]: not used
#include "octree.h"
#include "nodelist.h"
#include "PathTable.h"

/* ***************************** define types ****************************** */
#define INITIAL_NUM_NODES	100
#define INCREMENT_NUM_NODES	50

/* ***************************** Local Variables *************************** */

static short nNumNearGroundNodes    = 0;
static short aNearGroundNodes[256];
static short nNumNearWaterNodes     = 0;
static short aNearWaterNodes[256];
static short nNumNearAirNodes       = 0;
static short aNearAirNodes[256];
static short nNumNearTrackNodes     = 0;
static short aNearTrackNodes[256];


/* **************************** Global Variables *************************** */

OCTREE_PTR pPathOctree = NULL;

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
int Node_IsWithinRange(CVector &position1, CVector &position2, int traceit, int NodeType);

//*****************************************************************************

// ----------------------------------------------------------------------------
//                          OCTREENODE functions
// ----------------------------------------------------------------------------

static void InitNearNodes()
{
	nNumNearGroundNodes = 0;
	nNumNearWaterNodes  = 0;
	nNumNearAirNodes    = 0;
}

static short GetNumNearGroundNodes()
{
	return nNumNearGroundNodes;
}
static void AddNearGroundNode( short nIndex )
{
	int bFound = FALSE;
	for ( int i = 0; i < nNumNearGroundNodes; i++ )
	{
		if ( nIndex == aNearGroundNodes[i] )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( bFound == FALSE )
	{
		aNearGroundNodes[nNumNearGroundNodes] = nIndex;
		nNumNearGroundNodes++;
	}
}

static short GetNumNearWaterNodes()
{
	return nNumNearWaterNodes;
}
static void AddNearWaterNode( short nIndex )
{
	int bFound = FALSE;
	for ( int i = 0; i < nNumNearWaterNodes; i++ )
	{
		if ( nIndex == aNearWaterNodes[i] )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( bFound == FALSE )
	{
		aNearWaterNodes[nNumNearWaterNodes] = nIndex;
		nNumNearWaterNodes++;
	}
}

static short GetNumNearAirNodes()
{
	return nNumNearAirNodes;
}
static void AddNearAirNode( short nIndex )
{
	int bFound = FALSE;
	for ( int i = 0; i < nNumNearAirNodes; i++ )
	{
		if ( nIndex == aNearAirNodes[i] )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( bFound == FALSE )
	{
		aNearAirNodes[nNumNearAirNodes] = nIndex;
		nNumNearAirNodes++;
	}
}

static short GetNumNearTrackNodes()
{
	return nNumNearTrackNodes;
}
static void AddNearTrackNode( short nIndex )
{
	int bFound = FALSE;
	for ( int i = 0; i < nNumNearTrackNodes; i++ )
	{
		if ( nIndex == aNearTrackNodes[i] )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( bFound == FALSE )
	{
		aNearTrackNodes[nNumNearTrackNodes] = nIndex;
		nNumNearTrackNodes++;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_Initialize
// Description:
//              Initialize a single octree node 
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_Initialize( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	pNode->nNodeType = OCTREE_INTERNAL_NODE;
	pNode->nIndex	 = -1;

	pNode->center.x = 0.0f;
	pNode->center.y = 0.0f;
	pNode->center.z = 0.0f;
	pNode->mins.x = 0.0f;
	pNode->mins.y = 0.0f;
	pNode->mins.z = 0.0f;
	pNode->maxs.x = 0.0f;
	pNode->maxs.y = 0.0f;
	pNode->maxs.z = 0.0f;

	pNode->nParentNodeIndex = -1;
	pNode->aChildrenNodeIndices[0] = -1;
	pNode->aChildrenNodeIndices[1] = -1;
	pNode->aChildrenNodeIndices[2] = -1;
	pNode->aChildrenNodeIndices[3] = -1;
	pNode->aChildrenNodeIndices[4] = -1;
	pNode->aChildrenNodeIndices[5] = -1;
	pNode->aChildrenNodeIndices[6] = -1;
	pNode->aChildrenNodeIndices[7] = -1;

	pNode->nNumNearGroundNodes	  = 0;
	pNode->pNearGroundNodeIndices = NULL;
	pNode->nNumNearWaterNodes	  = 0;
	pNode->pNearWaterNodeIndices  = NULL;
	pNode->nNumNearAirNodes		  = 0;
	pNode->pNearAirNodeIndices	  = NULL;
	pNode->nNumNearTrackNodes     = 0;
	pNode->pNearTrackNodeIndices  = NULL;

	pNode->bRemove = FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_Construct
// Description:
//              constructs an octree node, allocate memory and initializing it
// Input:
// Output:
//              OCTREENODE_PTR      => just constructed octree node
// Note:
//
// ----------------------------------------------------------------------------
OCTREENODE_PTR OCTREENODE_Construct()
{
	OCTREENODE_PTR pNode = (OCTREENODE_PTR)memmgr.X_Malloc(sizeof(OCTREENODE),MEM_TAG_MISC);

	OCTREENODE_Initialize( pNode );

	return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_Destruct
// Description:
//              Destroy an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node to be deleted
// Output:
// Note:
//
// ----------------------------------------------------------------------------
OCTREENODE_PTR OCTREENODE_Destruct( OCTREENODE_PTR pNode )
{
	if ( pNode )
	{
		memmgr.X_Free(pNode->pNearGroundNodeIndices);
		memmgr.X_Free(pNode->pNearWaterNodeIndices);
		memmgr.X_Free(pNode->pNearAirNodeIndices);
		memmgr.X_Free(pNode->pNearTrackNodeIndices);

		memmgr.X_Free(pNode);
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNode
// Description:
//              Return an octree node
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              int nIndex          => index to an octree nodes 
// Output:
//              OCTREENODE_PTR      => pointer to an octree node
// Note:
//
// ----------------------------------------------------------------------------
__inline static OCTREENODE_PTR OCTREE_GetNode( OCTREE_PTR pTree, unsigned int nIndex )
{
    _ASSERTE( pTree );

	if ( nIndex >= pTree->nNumNodes )
	{
		return NULL;
	}

	OCTREENODE_PTR pNode = &(pTree->pNodes[nIndex]);
    _ASSERTE( pNode );

    return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNodeType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetNodeType( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nNodeType;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNodeType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNodeType( OCTREENODE_PTR pNode, short nNewType )
{
	_ASSERTE( pNode );

    pNode->nNodeType = nNewType;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNodeType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetIndex( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetIndex
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetIndex( OCTREENODE_PTR pNode, short nNewIndex )
{
    _ASSERTE( pNode );

	pNode->nIndex = nNewIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetCenter
// Description:
//              Return the center position of an octree node
// Input:
//              OCTREENODE_PTR pNode    => a pointer to an octree node
// Output:
//              CVector &               => center position of an octree node
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetCenter( OCTREENODE_PTR pNode, CVector &center )
{
    _ASSERTE( pNode );

	center = pNode->center;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetCenter
// Description:
//              Set the center position of an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              CVector&                => new center position of the octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetCenter( OCTREENODE_PTR pNode, const CVector &newCenter )
{
    _ASSERTE( pNode );

	pNode->center = newCenter;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetMins
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetMins( OCTREENODE_PTR pNode, CVector &mins )
{
	_ASSERTE( pNode );

    mins = pNode->mins;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetMins
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetMins( OCTREENODE_PTR pNode, const CVector &newMins )
{
    _ASSERTE( pNode );

	pNode->mins = newMins;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetMaxs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetMaxs( OCTREENODE_PTR pNode, CVector &maxs )
{
    _ASSERTE( pNode );

	maxs = pNode->maxs;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetMaxs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetMaxs( OCTREENODE_PTR pNode, const CVector &newMaxs )
{
    _ASSERTE( pNode );

	pNode->maxs = newMaxs;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetParentIndex
// Description:
//              Get an index of its parent node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
__inline static short OCTREENODE_GetParentIndex( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nParentNodeIndex;
}
*/

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetParentIndex
// Description:
//              Assigne a parent node index to an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              shot    nNewIndex       => new parent node index 
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetParentIndex( OCTREENODE_PTR pNode, short nNewIndex )
{
    _ASSERTE( pNode );

	pNode->nParentNodeIndex = nNewIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetChildrenIndices
// Description:
//              return all eight indices of an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
// Output:
//              short *pIndices     => passed in array of 8 shorts
//                                     expects the array to be already allocated 
//                                     before passed in to this function
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
__inline static void OCTREENODE_GetChildrenIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	pIndices[0] = pNode->aChildrenNodeIndices[0];
	pIndices[1] = pNode->aChildrenNodeIndices[1];
	pIndices[2] = pNode->aChildrenNodeIndices[2];
	pIndices[3] = pNode->aChildrenNodeIndices[3];
	pIndices[4] = pNode->aChildrenNodeIndices[4];
	pIndices[5] = pNode->aChildrenNodeIndices[5];
	pIndices[6] = pNode->aChildrenNodeIndices[6];
	pIndices[7] = pNode->aChildrenNodeIndices[7];
}
*/
// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetChildrenIndices
// Description:
//              Set children indices for an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              short *pNewIndices      => array of 8 shorts 
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetChildrenIndices( OCTREENODE_PTR pNode, short *pNewIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pNewIndices );

	pNode->aChildrenNodeIndices[0] = pNewIndices[0];
	pNode->aChildrenNodeIndices[1] = pNewIndices[1];
	pNode->aChildrenNodeIndices[2] = pNewIndices[2];
	pNode->aChildrenNodeIndices[3] = pNewIndices[3];
	pNode->aChildrenNodeIndices[4] = pNewIndices[4];
	pNode->aChildrenNodeIndices[5] = pNewIndices[5];
	pNode->aChildrenNodeIndices[6] = pNewIndices[6];
	pNode->aChildrenNodeIndices[7] = pNewIndices[7];
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetChildIndex
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetChildIndex( OCTREENODE_PTR pNode, short nIndex, short nChildIndex )
{
    _ASSERTE( pNode );

	pNode->aChildrenNodeIndices[nIndex] = nChildIndex;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetChildIndex
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetChildIndex( OCTREENODE_PTR pNode, short nThChild )
{
    _ASSERTE( pNode );

	return pNode->aChildrenNodeIndices[nThChild];
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNumNearGroundNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNumNearGroundNodes( OCTREENODE_PTR pNode, short nNewNum )
{
    _ASSERTE( pNode );

	if ( nNewNum > 0 )
	{
		pNode->nNumNearGroundNodes = nNewNum;
		pNode->pNearGroundNodeIndices = (short*)memmgr.X_Malloc(sizeof(short)*pNode->nNumNearGroundNodes,MEM_TAG_MISC);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNumNearGroundNodes
// Description:
//              Return the number of near ground nodes
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetNumNearGroundNodes( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nNumNearGroundNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNearGroundNodeIndices
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNearGroundNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	if ( pNode->pNearGroundNodeIndices )
	{
		memcpy( pNode->pNearGroundNodeIndices, pIndices, pNode->nNumNearGroundNodes * sizeof(pNode->pNearGroundNodeIndices[0]) );

#ifdef _DEBUG
        for ( int i = 0; i < pNode->nNumNearGroundNodes; i++ )
		{
			_ASSERTE( pNode->pNearGroundNodeIndices[i] == pIndices[i] );
		}
#endif _DEBUG
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNearGroundNodeIndices
// Description:
//              Return the indices of near ground nodes
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetNearGroundNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	memcpy( pIndices, pNode->pNearGroundNodeIndices, pNode->nNumNearGroundNodes * sizeof(pNode->pNearGroundNodeIndices[0]) );

#ifdef _DEBUG
    for ( int i = 0; i < pNode->nNumNearGroundNodes; i++ )
	{
		_ASSERTE( pIndices[i] == pNode->pNearGroundNodeIndices[i] );
	}
#endif _DEBUG
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNumNearWaterNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNumNearWaterNodes( OCTREENODE_PTR pNode, short nNewNum )
{
	_ASSERTE( pNode );

    if ( nNewNum > 0 )
	{
		pNode->nNumNearWaterNodes = nNewNum;
		pNode->pNearWaterNodeIndices = (short*)memmgr.X_Malloc(sizeof(short)*pNode->nNumNearWaterNodes,MEM_TAG_MISC);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNumNearWaterNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetNumNearWaterNodes( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nNumNearWaterNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNearWaterNodeIndices
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNearWaterNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	if ( pNode->pNearWaterNodeIndices )
	{
		memcpy( pNode->pNearWaterNodeIndices, pIndices, pNode->nNumNearWaterNodes * sizeof(pNode->pNearWaterNodeIndices[0]) );

#ifdef _DEBUG
        for ( int i = 0; i < pNode->nNumNearWaterNodes; i++ )
		{
		    _ASSERTE( pNode->pNearWaterNodeIndices[i] == pIndices[i] );
		}
#endif _DEBUG
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNearWaterNodeIndices
// Description:
//              return the indices of near water nodes in an octree node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetNearWaterNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	memcpy( pIndices, pNode->pNearWaterNodeIndices, pNode->nNumNearWaterNodes * sizeof(pNode->pNearWaterNodeIndices[0]) );
//    for ( int i = 0; i < pNode->nNumNearWaterNodes; i++ )
//	{
//		pIndices[i] = pNode->pNearWaterNodeIndices[i];
//	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNumNearAirNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNumNearAirNodes( OCTREENODE_PTR pNode, short nNewNum )
{
    _ASSERTE( pNode );

	if ( nNewNum > 0 )
	{
		pNode->nNumNearAirNodes = nNewNum;
		pNode->pNearAirNodeIndices = (short*)memmgr.X_Malloc(sizeof(short)*pNode->nNumNearAirNodes,MEM_TAG_MISC);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNumNearAirNodes
// Description:
//              Return the number of near air nodes in an octree node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_GetNumNearAirNodes( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->nNumNearAirNodes;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNearAirNodeIndices
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNearAirNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	if ( pNode->pNearAirNodeIndices )
	{
		memcpy( pNode->pNearAirNodeIndices, pIndices, pNode->nNumNearAirNodes * sizeof(pNode->pNearAirNodeIndices[0]) );
//        for ( int i = 0; i < pNode->nNumNearAirNodes; i++ )
//		{
//			pNode->pNearAirNodeIndices[i] = pIndices[i];
//		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNearAirNodeIndices
// Description:
//              Return near air node indices of an octree node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetNearAirNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	memcpy( pIndices, pNode->pNearAirNodeIndices, pNode->nNumNearAirNodes * sizeof(pNode->pNearAirNodeIndices[0]) );
//    for ( int i = 0; i < pNode->nNumNearAirNodes; i++ )
//	{
//		pIndices[i] = pNode->pNearAirNodeIndices[i];
//	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNumNearTrackNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNumNearTrackNodes( OCTREENODE_PTR pNode, short nNewNum )
{
    _ASSERTE( pNode );

	if ( nNewNum > 0 )
	{
		pNode->nNumNearTrackNodes = nNewNum;
		pNode->pNearTrackNodeIndices = (short*)memmgr.X_Malloc(sizeof(short)*pNode->nNumNearTrackNodes,MEM_TAG_MISC);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNumNearTrackNodes
// Description:
//              Return the number of near track nodes
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
__inline static short OCTREENODE_GetNumNearTrackNodes( OCTREENODE_PTR pNode )
{
	_ASSERTE( pNode );

    return pNode->nNumNearTrackNodes;
}
*/

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetNearTrackNodeIndices
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetNearTrackNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	if ( pNode->pNearTrackNodeIndices )
	{
		memcpy( pNode->pNearTrackNodeIndices, pIndices, pNode->nNumNearTrackNodes * sizeof(pNode->pNearTrackNodeIndices[0]) );
//        for ( int i = 0; i < pNode->nNumNearTrackNodes; i++ )
//		{
//			pNode->pNearTrackNodeIndices[i] = pIndices[i];
//		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNearTrackNodeIndices
// Description:
//              Return the near track node indices of an octree
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_GetNearTrackNodeIndices( OCTREENODE_PTR pNode, short *pIndices )
{
    _ASSERTE( pNode );
    _ASSERTE( pIndices );

	memcpy( pIndices, pNode->pNearTrackNodeIndices, pNode->nNumNearTrackNodes * sizeof(pNode->pNearTrackNodeIndices[0]) );
//    for ( int i = 0; i < pNode->nNumNearTrackNodes; i++ )
//	{
//		pIndices[i] = pNode->pNearTrackNodeIndices[i];
//	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_RemoveChildNode
// Description:
//              Remove a child node from an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              short nChildNodeIndex   => index of a child node to be removed
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_RemoveChildNode( OCTREENODE_PTR pNode, short nChildNodeIndex )
{
    _ASSERTE( pNode );

	for ( int i = 0; i < 8; i++ )
	{
		if ( pNode->aChildrenNodeIndices[i] == nChildNodeIndex )
		{
			pNode->aChildrenNodeIndices[i] = -1;
			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_SetRemoveFlag
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_SetRemoveFlag( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	pNode->bRemove = TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_ClearRemoveFlag
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREENODE_ClearRemoveFlag( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	pNode->bRemove = FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_CanRemove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static short OCTREENODE_CanRemove( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	return pNode->bRemove;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_ReadData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void OCTREENODE_ReadData( OCTREENODE_PTR pNode, FILE *in )
{
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_AddNode
// Description:
//              Add a new octree node to another octree node
// Input:
//              OCTREE_PTR pTree        => pointer to an octree
//              OCTREENODE_PTR pNode    => pointer to an octree node to be added
//                                         to an octree
// Output:
//              OCTREENODE_PTR          => pointer to just added octree node
// Note:
//
// ----------------------------------------------------------------------------
__inline static OCTREENODE_PTR OCTREE_AddNode( OCTREE_PTR pTree, OCTREENODE_PTR pNewNode )
{
	_ASSERTE( pTree );
    _ASSERTE( pNewNode );

    if ( pTree->nNumNodes >= pTree->nNumAllocatedNodes )
	{
		int nPrevNumNodes = pTree->nNumNodes;

		pTree->nNumAllocatedNodes += INCREMENT_NUM_NODES;
		OCTREENODE_PTR pNewNodes = (OCTREENODE_PTR)memmgr.X_Malloc(sizeof(OCTREENODE)*pTree->nNumAllocatedNodes,MEM_TAG_MISC);
		
	    for ( int i = 0; i < pTree->nNumAllocatedNodes; i++ )
	    {
		    OCTREENODE_Initialize( &(pNewNodes[i]) );
	    }

		for ( i = 0; i < nPrevNumNodes; i++ )
		{
			memcpy( &(pNewNodes[i]), &(pTree->pNodes[i]), sizeof(OCTREENODE) );
		}

		memmgr.X_Free(pTree->pNodes);
		pTree->pNodes = pNewNodes;
		pTree->pRoot = pTree->pNodes;
	}
	
	OCTREENODE_PTR pNode = &(pTree->pNodes[pTree->nNumNodes]);
	memcpy( pNode, pNewNode, sizeof(OCTREENODE) );
	OCTREENODE_SetIndex( pNode, pTree->nNumNodes );

	memmgr.X_Free(pNewNode);
	pNewNode = NULL;

	pTree->nNumNodes++;

	return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_AddNode
// Description:
//              Add an octree node to another octree node
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              CVector &newCenter  => center point of a new octree node
//              CVector &newMins    => mins of a new octree node
//              CVector &newMaxs    => maxs of a new octree node
//              int nParentNodeIndex=> index of the parent octree node
// Output:
//              OCTREENODE_PTR      => pointer to the newly created octree node
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
__inline static OCTREENODE_PTR OCTREE_AddNode( OCTREE_PTR pTree, const CVector &newCenter, 
			            const CVector &newMins, const CVector &newMaxs, int nParentNodeIndex )
{
	_ASSERTE( pTree );

    if ( pTree->nNumNodes >= pTree->nNumAllocatedNodes )
	{
		int nPrevNumNodes = pTree->nNumNodes;

		pTree->nNumAllocatedNodes += INCREMENT_NUM_NODES;
		OCTREENODE_PTR pNewNodes = (OCTREENODE_PTR)memmgr.X_Malloc(sizeof(OCTREENODE)*pTree->nNumAllocatedNodes,MEM_TAG_MISC);
		
	    for ( int i = 0; i < pTree->nNumAllocatedNodes; i++ )
	    {
		    OCTREENODE_Initialize( &(pNewNodes[i]) );
	    }

		for ( i = 0; i < nPrevNumNodes; i++ )
		{
			memcpy( &(pNewNodes[i]), &(pTree->pNodes[i]), sizeof(OCTREENODE) );
		}

		memmgr.X_Free(pTree->pNodes);
		pTree->pNodes = pNewNodes;
	}
	
	OCTREENODE_PTR pNode = &(pTree->pNodes[pTree->nNumNodes]);
	OCTREENODE_SetCenter( pNode, newCenter );
	OCTREENODE_SetMins( pNode, newMins );
	OCTREENODE_SetMaxs( pNode, newMaxs );
	OCTREENODE_SetParentIndex( pNode, nParentNodeIndex );
	OCTREENODE_SetIndex( pNode, pTree->nNumNodes );

	pTree->nNumNodes++;

	return pNode;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_WriteData
// Description:
//              Write an octree node data to a file
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              FILE *out               => pointer to an opened file
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void OCTREENODE_WriteData( OCTREENODE_PTR pNode, FILE *out )
{
    _ASSERTE( pNode );
    _ASSERTE( out );

	fwrite( &pNode->nNodeType, sizeof(pNode->nNodeType), 1, out );
	fwrite( &pNode->nIndex, sizeof(pNode->nIndex), 1, out );
	fwrite( &pNode->center.x, sizeof(pNode->center.x), 1, out );
	fwrite( &pNode->center.y, sizeof(pNode->center.y), 1, out );
	fwrite( &pNode->center.z, sizeof(pNode->center.z), 1, out );
	fwrite( &pNode->mins.x, sizeof(pNode->mins.x), 1, out );
	fwrite( &pNode->mins.y, sizeof(pNode->mins.y), 1, out );
	fwrite( &pNode->mins.z, sizeof(pNode->mins.z), 1, out );
	fwrite( &pNode->maxs.x, sizeof(pNode->maxs.x), 1, out );
	fwrite( &pNode->maxs.y, sizeof(pNode->maxs.y), 1, out );
	fwrite( &pNode->maxs.z, sizeof(pNode->maxs.z), 1, out );
	fwrite( &pNode->nParentNodeIndex, sizeof(pNode->nParentNodeIndex), 1, out );
	fwrite( pNode->aChildrenNodeIndices, sizeof(pNode->aChildrenNodeIndices), 1, out );

	fwrite( &pNode->nNumNearGroundNodes, sizeof(pNode->nNumNearGroundNodes), 1, out );
	for ( int i = 0; i < pNode->nNumNearGroundNodes; i++ )
	{
		fwrite( &pNode->pNearGroundNodeIndices[i], sizeof(pNode->pNearGroundNodeIndices[i]), 1, out );
	}
	fwrite( &pNode->nNumNearWaterNodes, sizeof(pNode->nNumNearWaterNodes), 1, out );
	for ( i = 0; i < pNode->nNumNearWaterNodes; i++ )
	{
		fwrite( &pNode->pNearWaterNodeIndices[i], sizeof(pNode->pNearWaterNodeIndices[i]), 1, out );
	}
	fwrite( &pNode->nNumNearAirNodes, sizeof(pNode->nNumNearAirNodes), 1, out );
	for ( i = 0; i < pNode->nNumNearAirNodes; i++ )
	{
		fwrite( &pNode->pNearAirNodeIndices[i], sizeof(pNode->pNearAirNodeIndices[i]), 1, out );
	}
	fwrite( &pNode->nNumNearTrackNodes, sizeof(pNode->nNumNearTrackNodes), 1, out );
	for ( i = 0; i < pNode->nNumNearTrackNodes; i++ )
	{
		fwrite( &pNode->pNearTrackNodeIndices[i], sizeof(pNode->pNearTrackNodeIndices[i]), 1, out );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_GetNearVisiblePathNodes
// Description:
//              Find all visible path nodes from a given position
// Input:
//              CVector &startPosition  => point from where near path nodes
//                                         are searched
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void OCTREENODE_GetNearVisiblePathNodes( CVector &startPosition )
{
	for ( int i = 0; i < pGroundNodes->nNumNodes; i++ )
	{
		MAPNODE_PTR pNode = &(pGroundNodes->pNodes[i]);

		CVector position = pNode->position;

		// NSS[2/7/00]:
		//if ( gstate->inPVS( startPosition, position ) )
		if(Node_IsWithinRange(startPosition, position, TRUE, pNode->node_type))
		{
			//tr = gstate->TraceLine_q2( startPosition, position, NULL, MASK_SOLID );
			//if ( tr.fraction >= 1.0 )
			//{
				if ( pNode->node_type & NODETYPE_GROUND )
				{
					AddNearGroundNode( i );
				}
				else
				{
					AddNearWaterNode( i );
				}
			//}
		}
	}

	if ( pAirNodes )
	{
		for ( i = 0; i < pAirNodes->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pAirNodes->pNodes[i]);

			CVector position = pNode->position;

			// NSS[2/7/00]:
			//if ( gstate->inPVS( startPosition, position ) )
			if(Node_IsWithinRange(startPosition, position, TRUE, pNode->node_type))
			{
				//tr = gstate->TraceLine_q2( startPosition, position, NULL, MASK_SOLID );
				//if ( tr.fraction >= 1.0 )
				//{
					AddNearAirNode( i );
				//}
			}
		}
	}

	if ( pTrackNodes )
	{
		for ( i = 0; i < pTrackNodes->nNumNodes; i++ )
		{
			MAPNODE_PTR pNode = &(pTrackNodes->pNodes[i]);

			CVector position = pNode->position;

			// NSS[2/7/00]:
			//if ( gstate->inPVS( startPosition, position ) )
			if(Node_IsWithinRange(startPosition, position, 0, pNode->node_type))
			{
				tr = gstate->TraceLine_q2( startPosition, position, NULL, MASK_SOLID );
				if ( tr.fraction >= 1.0 )
				{
					AddNearTrackNode( i );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_ComputeNearNodes
// Description:
//              Find all near path nodes from an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void OCTREENODE_ComputeNearNodes( OCTREENODE_PTR pNode )
{
    _ASSERTE( pNode );

	InitNearNodes();

	CVector mins, maxs, center;
	
	OCTREENODE_GetMins( pNode, mins );
	OCTREENODE_GetMaxs( pNode, maxs );
	OCTREENODE_GetCenter( pNode, center );

	short aNodeIndices[MAX_NUM_NEAR_PATHNODES];

	// add those nodes in octree subspace in the leaf node
	// ground nodes
	int nNumGroundNodesInSpace = NODE_GetGroundNodesInSpace( mins, maxs, aNodeIndices );
	for ( int i = 0; i < nNumGroundNodesInSpace; i++ )
	{
		AddNearGroundNode( aNodeIndices[i] );		
	}

	// water nodes
	int nNumWaterNodesInSpace = NODE_GetWaterNodesInSpace( mins, maxs, aNodeIndices );
	for ( i = 0; i < nNumWaterNodesInSpace; i++ )
	{
		AddNearWaterNode( aNodeIndices[i] );		
	}

	// air nodes
	int nNumAirNodesInSpace = NODE_GetAirNodesInSpace( mins, maxs, aNodeIndices );
	for ( i = 0; i < nNumAirNodesInSpace; i++ )
	{
		AddNearAirNode( aNodeIndices[i] );		
	}

	// track nodes
	int nNumTrackNodesInSpace = NODE_GetTrackNodesInSpace( mins, maxs, aNodeIndices );
	for ( i = 0; i < nNumTrackNodesInSpace; i++ )
	{
		AddNearTrackNode( aNodeIndices[i] );		
	}

	float fStartX = mins.x + 1.0f;
	float fEndX	  = maxs.x - 1.0f;
	float fStartY = mins.y + 1.0f;
	float fEndY   = maxs.y - 1.0f;
	float fStartZ = mins.z + 1.0f;
	float fEndZ   = maxs.z - 1.0f;

	float fWidth = fEndX - fStartX;

	float fIncrement = (fWidth * 0.125f) - 0.1f;
	for ( float x = fStartX; x <= fEndX; x += fIncrement )
	{
		for ( float y = fStartY; y <= fEndY; y += fIncrement )
		{
			for ( float z = fStartZ; z <= fEndZ; z += fIncrement )
			{
				CVector currentPoint( x, y, z );
				if ( !(gstate->PointContents(currentPoint) & MASK_SOLID) )
				{
					OCTREENODE_GetNearVisiblePathNodes( currentPoint );
				}	
			}
		}
	}

	if ( nNumNearGroundNodes > MAX_NUM_NEAR_PATHNODES )
	{
		// find MAX_NUM_NEAR_PATHNODES closest nodes
		// use bubble sort to sort nodes, leave the path nodes that are in 
		// octree node's subspace
		for ( int i = nNumGroundNodesInSpace; i < nNumNearGroundNodes; i++ )
		{
			float fMinDist = 1e20;
			int nMinDistIndex = -1;

			for ( int j = i; j < nNumNearGroundNodes; j ++ )
			{
				MAPNODE_PTR pNode = &(pGroundNodes->pNodes[aNearGroundNodes[j]]);	
				float fDistance = VectorDistance( center, pNode->position );

				if ( fDistance < fMinDist )
				{
					fMinDist = fDistance;
					nMinDistIndex = j;
				}
			}

			// swap current index with min index found
			int nTemp;
			nTemp = aNearGroundNodes[i];
			aNearGroundNodes[i] = aNearGroundNodes[j];
			aNearGroundNodes[j] = nTemp;
		}

		nNumNearGroundNodes = MAX_NUM_NEAR_PATHNODES;
	}

	OCTREENODE_SetNumNearGroundNodes( pNode, nNumNearGroundNodes );
	OCTREENODE_SetNearGroundNodeIndices( pNode, aNearGroundNodes );

	// now for water nodes
	if ( nNumNearWaterNodes > MAX_NUM_NEAR_PATHNODES )
	{
		// find MAX_NUM_NEAR_PATHNODES closest nodes
		// use bubble sort to sort nodes, leave the path nodes that are in 
		// octree node's subspace
		for ( int i = nNumWaterNodesInSpace; i < nNumNearWaterNodes; i++ )
		{
			float fMinDist = 1e20;
			int nMinDistIndex = -1;

			for ( int j = i; j < nNumNearWaterNodes; j ++ )
			{
				MAPNODE_PTR pNode = &(pGroundNodes->pNodes[aNearWaterNodes[j]]);	
				float fDistance = VectorDistance( center, pNode->position );

				if ( fDistance < fMinDist )
				{
					fMinDist = fDistance;
					nMinDistIndex = j;
				}
			}

			// swap current index with min index found
			int nTemp;
			nTemp = aNearWaterNodes[i];
			aNearWaterNodes[i] = aNearWaterNodes[j];
			aNearWaterNodes[j] = nTemp;
		}

		nNumNearWaterNodes = MAX_NUM_NEAR_PATHNODES;
	}

	OCTREENODE_SetNumNearWaterNodes( pNode, nNumNearWaterNodes );
	OCTREENODE_SetNearWaterNodeIndices( pNode, aNearWaterNodes );

	// air nodes
	if ( nNumNearAirNodes > MAX_NUM_NEAR_PATHNODES )
	{
		// find MAX_NUM_NEAR_PATHNODES closest nodes
		// use bubble sort to sort nodes, leave the path nodes that are in 
		// octree node's subspace
		for ( int i = nNumAirNodesInSpace; i < nNumNearAirNodes; i++ )
		{
			float fMinDist = 1e20;
			int nMinDistIndex = -1;

			for ( int j = i; j < nNumNearAirNodes; j ++ )
			{
				MAPNODE_PTR pNode = &(pAirNodes->pNodes[aNearAirNodes[j]]);	
				float fDistance = VectorDistance( center, pNode->position );

				if ( fDistance < fMinDist )
				{
					fMinDist = fDistance;
					nMinDistIndex = j;
				}
			}

			// swap current index with min index found
			int nTemp;
			nTemp = aNearAirNodes[i];
			aNearAirNodes[i] = aNearAirNodes[j];
			aNearAirNodes[j] = nTemp;
		}

		nNumNearAirNodes = MAX_NUM_NEAR_PATHNODES;
	}

	OCTREENODE_SetNumNearAirNodes( pNode, nNumNearAirNodes );
	OCTREENODE_SetNearAirNodeIndices( pNode, aNearAirNodes );

	// track nodes
	if ( nNumNearTrackNodes > MAX_NUM_NEAR_PATHNODES )
	{
		// find MAX_NUM_NEAR_PATHNODES closest nodes
		// use bubble sort to sort nodes, leave the path nodes that are in 
		// octree node's subspace
		for ( int i = nNumTrackNodesInSpace; i < nNumNearTrackNodes; i++ )
		{
			float fMinDist = 1e20;
			int nMinDistIndex = -1;

			for ( int j = i; j < nNumNearTrackNodes; j ++ )
			{
				MAPNODE_PTR pNode = &(pTrackNodes->pNodes[aNearTrackNodes[j]]);	
				float fDistance = VectorDistance( center, pNode->position );

				if ( fDistance < fMinDist )
				{
					fMinDist = fDistance;
					nMinDistIndex = j;
				}
			}

			// swap current index with min index found
			int nTemp;
			nTemp = aNearTrackNodes[i];
			aNearTrackNodes[i] = aNearTrackNodes[j];
			aNearTrackNodes[j] = nTemp;
		}

		nNumNearTrackNodes = MAX_NUM_NEAR_PATHNODES;
	}

	OCTREENODE_SetNumNearTrackNodes( pNode, nNumNearTrackNodes );
	OCTREENODE_SetNearTrackNodeIndices( pNode, aNearTrackNodes );
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_BuildOctree
// Description:
//				Split the current node into 8 children
// Input:
//              OCTREE_PTR  pTree   => pointer to an octree
//              int nOctreeNodeIndex=> parent node index
// Output:
// Note:
//              this is a recursive function that calls itself to subdivide further
//              until a desired size has been reached
//
// ----------------------------------------------------------------------------
void OCTREENODE_BuildOctree( OCTREE_PTR pTree, int nOctreeNodeIndex )
{
    _ASSERTE( pTree );

	OCTREENODE_PTR pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
	_ASSERTE( pNode );

	CVector mins, maxs, center;
	
	OCTREENODE_GetMins( pNode, mins );
	OCTREENODE_GetMaxs( pNode, maxs );
	OCTREENODE_GetCenter( pNode, center );

	float fWidth = maxs.x - mins.x;
	// if there is one of less path nodes in this subspace, return
	if ( fWidth <= 256.0f || NODE_GetTotalNumNodesInSpace( mins, maxs ) <= 1 )
	{
		PATHTABLE_ContinueThread();

        OCTREENODE_SetNodeType( pNode, OCTREE_LEAF_NODE );
		OCTREENODE_ComputeNearNodes( pNode );
		return;
	}
	
	OCTREENODE_PTR pChildNode = NULL;
	CVector childMins, childMaxs, childCenter;

	// index 0
	{
		pChildNode = OCTREENODE_Construct();
		childMins.x = mins.x;
		childMins.y = center.y;
		childMins.z = mins.z;
		childMaxs.x = center.x;
		childMaxs.y = maxs.y;
		childMaxs.z = center.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );
		
		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 0, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 1
	{
		pChildNode = OCTREENODE_Construct();
		childMins.x = center.x;
		childMins.y = center.y;
		childMins.z = mins.z;
		childMaxs.x = maxs.x;
		childMaxs.y = maxs.y;
		childMaxs.z = center.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );
		
		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 1, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 2
	{
		pChildNode = OCTREENODE_Construct();
		childMins.x = mins.x;
		childMins.y = center.y;
		childMins.z = center.z;
		childMaxs.x = center.x;
		childMaxs.y = maxs.y;
		childMaxs.z = maxs.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );
		
		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 2, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 3
	{		 
		pChildNode = OCTREENODE_Construct();
		childMins.x = center.x;
		childMins.y = center.y;
		childMins.z = center.z;
		childMaxs.x = maxs.x;
		childMaxs.y = maxs.y;
		childMaxs.z = maxs.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );
		
		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 3, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 4
	{		 
		pChildNode = OCTREENODE_Construct();
		childMins.x = mins.x;
		childMins.y = mins.y;
		childMins.z = mins.z;
		childMaxs.x = center.x;
		childMaxs.y = center.y;
		childMaxs.z = center.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );
		
		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 4, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 5
	{		 
		pChildNode = OCTREENODE_Construct();
		childMins.x = center.x;
		childMins.y = mins.y;
		childMins.z = mins.z;
		childMaxs.x = maxs.x;
		childMaxs.y = center.y;
		childMaxs.z = center.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );

		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 5, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 6
	{		 
		pChildNode = OCTREENODE_Construct();
		childMins.x = mins.x;
		childMins.y = mins.y;
		childMins.z = center.z;
		childMaxs.x = center.x;
		childMaxs.y = center.y;
		childMaxs.z = maxs.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );

		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 6, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}

	// index 7
	{		 
		pChildNode = OCTREENODE_Construct();
		childMins.x = center.x;
		childMins.y = mins.y;
		childMins.z = center.z;
		childMaxs.x = maxs.x;
		childMaxs.y = center.y;
		childMaxs.z = maxs.z;
		childCenter.x = (childMaxs.x - childMins.x) * 0.5f;
		childCenter.y = (childMaxs.y - childMins.y) * 0.5f;
		childCenter.z = (childMaxs.z - childMins.z) * 0.5f;
		childCenter.x += childMins.x;
		childCenter.y += childMins.y;
		childCenter.z += childMins.z;

		OCTREENODE_SetMins( pChildNode, childMins );
		OCTREENODE_SetMaxs( pChildNode, childMaxs );
		OCTREENODE_SetCenter( pChildNode, childCenter );
		OCTREENODE_SetParentIndex( pChildNode, nOctreeNodeIndex );
		pChildNode = OCTREE_AddNode( pTree, pChildNode );

		pNode = OCTREE_GetNode( pTree, nOctreeNodeIndex );
		int nChildNodeIndex = OCTREENODE_GetIndex( pChildNode );
		OCTREENODE_SetChildIndex( pNode, 7, nChildNodeIndex );

		OCTREENODE_BuildOctree( pTree, nChildNodeIndex );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREENODE_IsInside
// Description:
//              Is a point inside an octree node
// Input:
//              OCTREENODE_PTR pNode    => pointer to an octree node
//              CVector &position       => position to be tested 
// Output:
//              TRUE    => if position is inside the octree node
//              FALSE   => otherwise
// Note:
//
// ----------------------------------------------------------------------------
__inline static int OCTREENODE_IsInside( OCTREENODE_PTR pNode, const CVector &position )
{
    _ASSERTE( pNode );

	CVector mins, maxs;
	OCTREENODE_GetMins( pNode, mins );
	OCTREENODE_GetMaxs( pNode, maxs );

	if ( position.x >= mins.x && position.x <= maxs.x &&
		 position.y >= mins.y && position.y <= maxs.y &&
		 position.z >= mins.z && position.z <= maxs.z )
	{
		return TRUE;
	}	

	return FALSE;
}

//*****************************************************************************

// ----------------------------------------------------------------------------
//                          OCTREE functions
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
//
// Name:		OCTREE_Construct
// Description:
//              Construct an octree, initializing data as necessary
// Input:
// Output:
//              OCTREE_PTR  => pointer to a newly created octree
// Note:
//
// ----------------------------------------------------------------------------
OCTREE_PTR OCTREE_Construct()
{
	OCTREE_PTR pTree          = (OCTREE_PTR)memmgr.X_Malloc(sizeof(OCTREE),MEM_TAG_MISC);
	
	pTree->pRoot			  = NULL;
	pTree->nNumNodes		  = 0;
	pTree->nNumAllocatedNodes = INITIAL_NUM_NODES;
	pTree->pNodes			  = (OCTREENODE*)memmgr.X_Malloc(sizeof(OCTREENODE)*INITIAL_NUM_NODES,MEM_TAG_MISC);
	
	for ( int i = 0; i < pTree->nNumAllocatedNodes; i++ )
	{
		OCTREENODE_Initialize( &(pTree->pNodes[i]) );
	}

	return pTree;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_Destruct
// Description:
//              Destroy an octree
// Input:
//              OCTREE_PTR pTree    => pointer to an octree to be deleted
// Output:
//              NULL
// Note:
//
// ----------------------------------------------------------------------------
OCTREE_PTR OCTREE_Destruct( OCTREE_PTR pTree )
{
	if ( pTree )
	{
		for ( int i = 0; i < pTree->nNumAllocatedNodes; i++ )
		{
			OCTREENODE_PTR pNode = &(pTree->pNodes[i]);
			if ( pNode )
			{				
				if( pNode->pNearGroundNodeIndices && ( *pNode->pNearGroundNodeIndices != 0 ) )
				{
					memmgr.X_Free(pNode->pNearGroundNodeIndices);
					pNode->pNearGroundNodeIndices = NULL;
				}
				if( pNode->pNearWaterNodeIndices && ( *pNode->pNearWaterNodeIndices != 0 ) )
				{
					memmgr.X_Free(pNode->pNearWaterNodeIndices);
					pNode->pNearWaterNodeIndices = NULL;
				}
				if( pNode->pNearAirNodeIndices && ( *pNode->pNearAirNodeIndices != 0 ) )
				{
					memmgr.X_Free(pNode->pNearAirNodeIndices);
					pNode->pNearAirNodeIndices = NULL;
				}
				if( pNode->pNearTrackNodeIndices && ( *pNode->pNearTrackNodeIndices != 0 ) )
				{
					memmgr.X_Free(pNode->pNearTrackNodeIndices);
					pNode->pNearTrackNodeIndices = NULL;
				}
			}
		}
		memmgr.X_Free(pTree->pNodes);
		pTree->pNodes = NULL;

		memmgr.X_Free(pTree);
	}
			
	
	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetRoot
// Description:
//              Return the root of an octree
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
// Output:
//              OCTREENODE_PTR  => pointer to a root octree node
// Note:
//
// ----------------------------------------------------------------------------
__inline static OCTREENODE_PTR OCTREE_GetRoot( OCTREE_PTR pTree )
{
    _ASSERTE( pTree );

	return pTree->pRoot;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_SetRoot
// Description:
//              Set the root of an octree
// Input:
//              OCTREE_PTR pTree        => pointer to an octree
//              OCTREENODE_PTR pNewRoot => pointer to an octree node
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREE_SetRoot( OCTREE_PTR pTree, OCTREENODE_PTR pNewRoot )
{
	_ASSERTE( pTree );
    _ASSERTE( pNewRoot );

    pTree->pRoot = pNewRoot;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNodes
// Description:
//              Return all octree nodes of an octree
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
__inline static OCTREENODE_PTR OCTREE_GetNodes( OCTREE_PTR pTree )
{
	_ASSERTE( pTree );

    return pTree->pNodes;
}
*/
// ----------------------------------------------------------------------------
//
// Name:		OCTREE_RemoveNodeAndChildren
// Description:
//				Delete this and all its children
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static	void OCTREE_RemoveNodeAndChildren( OCTREE_PTR pTree, OCTREENODE_PTR pNode )
{
	_ASSERTE( pTree );
    _ASSERTE( pNode );

    for ( int i = 0; i < 8; i++ )
	{
		if ( pNode->aChildrenNodeIndices[i] >= 0 )
		{
			OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, pNode->aChildrenNodeIndices[i] );

			OCTREE_RemoveNodeAndChildren( pTree, pChildNode );
		}
	}

	OCTREENODE_PTR pParentNode = OCTREE_GetNode( pTree, pNode->nParentNodeIndex );
	OCTREENODE_RemoveChildNode( pParentNode, pNode->nIndex );

	// flag it to be removed later
	OCTREENODE_SetRemoveFlag( pNode );
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_RemoveNode
// Description:
//              Remove an octree node from an octree
// Input:
//              OCTREE_PTR pTree     => pointer to an octree
//              OCTREENODE_PTR pNode => pointer to an octree node to be removed
//                                      from an octree
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREE_RemoveNode( OCTREE_PTR pTree, OCTREENODE_PTR pNode )
{
    _ASSERTE( pTree );
    _ASSERTE( pNode );
    
	for ( int i = OCTREENODE_GetIndex(pNode); i < OCTREE_GetNumNodes(pTree)-1; i++ )
	{
		memcpy( &(pTree->pNodes[i]), &(pTree->pNodes[i+1]), sizeof(OCTREENODE) );
	}

	pTree->nNumNodes--;
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_DeleteNode
// Description:
//              Delete an octree node from an octree
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              OCTREENODE_PTR pNode => pointer to an octree node to be deleted
//                                      from an octree
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREE_DeleteNode( OCTREE_PTR pTree, OCTREENODE_PTR pNode )
{
    _ASSERTE( pTree );
    _ASSERTE( pNode );

	OCTREE_RemoveNode( pTree, pNode );

	for ( int i = 0; i < OCTREE_GetNumNodes(pTree); i++ )
	{
		OCTREENODE_PTR pCurrentNode = OCTREE_GetNode( pTree, i );
		if ( pCurrentNode && OCTREENODE_CanRemove(pCurrentNode) == TRUE )
		{
			OCTREE_RemoveNode( pTree, pCurrentNode );

			i--;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_DeleteNode
// Description:
//              Delete an octree node from an octree
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              int nIndex          => index of an octree node to be removed from
//                                     an octree
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void OCTREE_DeleteNode( OCTREE_PTR pTree, int nIndex )
{
	_ASSERTE( pTree );

    OCTREENODE_PTR pNode = OCTREE_GetNode( pTree, nIndex );
	if ( pNode )
	{
		OCTREE_DeleteNode( pTree, pNode );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_ReadData
// Description:
//              Read in the octree info from file
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              FILE *in            => pointer to an opened file stream
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void OCTREE_ReadData( OCTREE_PTR pTree, FILE *in )
{
    _ASSERTE( pTree );
    _ASSERTE( in );

//
//	char szSignature[64];
//	int nReadCount = fread( szSignature, sizeof(szOctreeSignature), 1, in );
//	if ( nReadCount == 0 || _stricmp( szSignature, szOctreeSignature ) != 0 )
//	{
//		return;
//	}
//

	int nVersion;
	fread( &nVersion, sizeof(nOctreeVersion), 1, in );

	switch( nVersion )
	{
		case 0:
		{
			int nNumNodes;
			fread( &nNumNodes, sizeof(nNumNodes), 1, in );

			// allocate enough to store all nodes
			memmgr.X_Free(pTree->pNodes);
			
			pTree->nNumNodes		  = nNumNodes;
			pTree->nNumAllocatedNodes = nNumNodes;
			pTree->pNodes			  = (OCTREENODE*)memmgr.X_Malloc(sizeof(OCTREENODE)*pTree->nNumNodes,MEM_TAG_MISC);
	        for ( int i = 0; i < pTree->nNumNodes; i++ )
	        {
		        OCTREENODE_Initialize( &(pTree->pNodes[i]) );
	        }

			for ( i = 0; i < nNumNodes; i++ )
			{
				short nNodeType, nIndex;
				CVector center, mins, maxs;
				
				short nParentNodeIndex;
				short aChildrenNodeIndices[8];
				fread( &nNodeType, sizeof(nNodeType), 1, in );
				fread( &nIndex, sizeof(nIndex), 1, in );
				fread( &center.x, sizeof(center.x), 1, in );
				fread( &center.y, sizeof(center.y), 1, in );
				fread( &center.z, sizeof(center.z), 1, in );
				fread( &mins.x, sizeof(mins.x), 1, in );
				fread( &mins.y, sizeof(mins.y), 1, in );
				fread( &mins.z, sizeof(mins.z), 1, in );
				fread( &maxs.x, sizeof(maxs.x), 1, in );
				fread( &maxs.y, sizeof(maxs.y), 1, in );
				fread( &maxs.z, sizeof(maxs.z), 1, in );
				fread( &nParentNodeIndex, sizeof(nParentNodeIndex), 1, in );
				fread( aChildrenNodeIndices, sizeof(aChildrenNodeIndices), 1, in );

				short nNumNearGroundNodes, nNumNearWaterNodes, nNumNearAirNodes, nNumNearTrackNodes;
				short aNearGroundNodeIndices[MAX_NUM_NEAR_PATHNODES];
				short aNearWaterNodeIndices[MAX_NUM_NEAR_PATHNODES];
				short aNearAirNodeIndices[MAX_NUM_NEAR_PATHNODES];
				short aNearTrackNodeIndices[MAX_NUM_NEAR_PATHNODES];

				fread( &nNumNearGroundNodes, sizeof(nNumNearGroundNodes), 1, in );
				for ( int j = 0; j < nNumNearGroundNodes; j++ )
				{
					fread( &aNearGroundNodeIndices[j], sizeof(aNearGroundNodeIndices[j]), 1, in );
				}
				fread( &nNumNearWaterNodes, sizeof(nNumNearWaterNodes), 1, in );
				for ( j = 0; j < nNumNearWaterNodes; j++ )
				{
					fread( &aNearWaterNodeIndices[j], sizeof(aNearWaterNodeIndices[j]), 1, in );
				}
				fread( &nNumNearAirNodes, sizeof(nNumNearAirNodes), 1, in );
				for ( j = 0; j < nNumNearAirNodes; j++ )
				{
					fread( &aNearAirNodeIndices[j], sizeof(aNearAirNodeIndices[j]), 1, in );
				}
				fread( &nNumNearTrackNodes, sizeof(nNumNearTrackNodes), 1, in );
				for ( j = 0; j < nNumNearTrackNodes; j++ )
				{
					fread( &aNearTrackNodeIndices[j], sizeof(aNearTrackNodeIndices[j]), 1, in );
				}

				OCTREENODE_PTR pNode = &(pTree->pNodes[i]);
				OCTREENODE_SetNodeType( pNode, nNodeType );
				_ASSERTE( i == nIndex );
				OCTREENODE_SetIndex( pNode, i );
				OCTREENODE_SetCenter( pNode, center );
				OCTREENODE_SetMins( pNode, mins );
				OCTREENODE_SetMaxs( pNode, maxs );
				OCTREENODE_SetParentIndex( pNode, nParentNodeIndex );
				OCTREENODE_SetChildrenIndices( pNode, aChildrenNodeIndices );
				
				OCTREENODE_SetNumNearGroundNodes( pNode, nNumNearGroundNodes );
				OCTREENODE_SetNearGroundNodeIndices( pNode, aNearGroundNodeIndices );
				OCTREENODE_SetNumNearWaterNodes( pNode, nNumNearWaterNodes );
				OCTREENODE_SetNearWaterNodeIndices( pNode, aNearWaterNodeIndices );
				OCTREENODE_SetNumNearAirNodes( pNode, nNumNearAirNodes );
				OCTREENODE_SetNearAirNodeIndices( pNode, aNearAirNodeIndices );
				OCTREENODE_SetNumNearTrackNodes( pNode, nNumNearTrackNodes );
				OCTREENODE_SetNearTrackNodeIndices( pNode, aNearTrackNodeIndices );
				
				OCTREENODE_ClearRemoveFlag( pNode );
			}
			// first node in the list should be the roor
			OCTREE_SetRoot( pTree, &(pTree->pNodes[0]) );

			break;
		}
	}

	gstate->Con_Dprintf( "Read Octree: %d subspaces.\n", pTree->nNumNodes );
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_WriteData
// Description:
//              Write out the octree to file
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              FILE *out           => pointer to an opened file stream
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void OCTREE_WriteData( OCTREE_PTR pTree, FILE *out )
{
    _ASSERTE( pTree );
    _ASSERTE( out );

	if ( pTree->nNumNodes > 0 )
	{
		//fwrite( szOctreeSignature, sizeof(szOctreeSignature), 1, out );
		fwrite( &nOctreeVersion, sizeof(nOctreeVersion), 1, out );

		fwrite( &pTree->nNumNodes, sizeof(pTree->nNumNodes), 1, out );
		for ( int i = 0; i < pTree->nNumNodes; i++ )
		{
			OCTREENODE_PTR pNode = &(pTree->pNodes[i]);
			OCTREENODE_WriteData( pNode, out );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_BuildOctree
// Description:
//              start building an octree of the map
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void OCTREE_BuildOctree( OCTREE_PTR pTree )
{
    _ASSERTE( pTree );

#ifdef _DEBUG
	int time1 = gstate->Sys_Milliseconds();
#endif _DEBUG

	pTree->pRoot = OCTREENODE_Construct();

	CVector mins, maxs, center;
	mins.x = -4096;
	mins.y = -4096;
	mins.z = -4096;
	maxs.x = 4096;
	maxs.y = 4096;
	maxs.z = 4096;
	center.x = (maxs.x - mins.x) * 0.5f;
	center.y = (maxs.y - mins.y) * 0.5f;
	center.z = (maxs.z - mins.z) * 0.5f;
	center.x += mins.x;
	center.y += mins.y;
	center.z += mins.z;

	OCTREENODE_SetMins( pTree->pRoot, mins );
	OCTREENODE_SetMaxs( pTree->pRoot, maxs );
	OCTREENODE_SetCenter( pTree->pRoot, center );
	pTree->pRoot = OCTREE_AddNode( pTree, pTree->pRoot );

	OCTREENODE_BuildOctree( pTree, OCTREENODE_GetIndex( pTree->pRoot ) );

#ifdef _DEBUG
	int timeDiff = gstate->Sys_Milliseconds() - time1;
	gstate->Con_Dprintf( "Building Octree: %d subspaces, took %d miliseconds.\n", pTree->nNumNodes, timeDiff );
#endif _DEBUG
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNearGroundNodes
// Description:
//              Return near ground nodes from a given point
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              CVector &position   => point from which near path nodes are computed
// Output:
//              short *             => array of shorts that stores indices of
//                                     near path nodes
// Note:
//              assumes that pNearGroundNodeIndices array is not empty
//
// ----------------------------------------------------------------------------

#define CHECK_POINT_INSIDE_CHILDNODE( nThChildIndex ) { int nIndex = OCTREENODE_GetChildIndex( pNode, nThChildIndex ); if ( nIndex >= 0 ) { OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, nIndex ); if ( OCTREENODE_IsInside( pChildNode, position ) ) { pNode = pChildNode; continue; } } }


short OCTREE_GetNearGroundNodes( OCTREE_PTR pTree, const CVector &position, short *pNearGroundNodeIndices )
{
	_ASSERTE( pTree );

    OCTREENODE_PTR pNode,pCurrentNode;
		
	pNode = pCurrentNode = OCTREE_GetRoot( pTree );

    _ASSERTE( pNode );
	
	while( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		for(int i = 0; i <= 7; i++)
		{
			int nIndex = OCTREENODE_GetChildIndex( pNode, i );
			if ( nIndex >= 0 )
			{ 
				OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, nIndex );
				if ( OCTREENODE_IsInside( pChildNode, position ) )
				{
					pNode = pChildNode; 
					break;
				}
			}
		}
		
		if(pNode == pCurrentNode)
		{
			return NULL;
		}
		else
		{
			pCurrentNode = pNode;
		}
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearGroundNodeIndices( pNode, pNearGroundNodeIndices );

	return OCTREENODE_GetNumNearGroundNodes( pNode );
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNearWaterNodes
// Description:
//              Return near water nodes from a given point
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              CVector &position   => point from which near path nodes are computed
// Output:
//              short *             => array of shorts that stores indices of
//                                     near path nodes
// Note:
//              assumes that pNearWaterNodeIndices array is not empty
//
// ----------------------------------------------------------------------------
/*short OCTREE_GetNearWaterNodes( OCTREE_PTR pTree, const CVector &position, short *pNearWaterNodeIndices )
{
    _ASSERTE( pTree );

	OCTREENODE_PTR pNode = OCTREE_GetRoot( pTree );
    _ASSERTE( pNode );
	while ( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		CHECK_POINT_INSIDE_CHILDNODE( 0 );
		CHECK_POINT_INSIDE_CHILDNODE( 1 );
		CHECK_POINT_INSIDE_CHILDNODE( 2 );
		CHECK_POINT_INSIDE_CHILDNODE( 3 );
		CHECK_POINT_INSIDE_CHILDNODE( 4 );
		CHECK_POINT_INSIDE_CHILDNODE( 5 );
		CHECK_POINT_INSIDE_CHILDNODE( 6 );
		CHECK_POINT_INSIDE_CHILDNODE( 7 );
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearWaterNodeIndices( pNode, pNearWaterNodeIndices );

	return OCTREENODE_GetNumNearWaterNodes( pNode );
}*/
short OCTREE_GetNearWaterNodes( OCTREE_PTR pTree, const CVector &position, short *pNearWaterNodeIndices )
{
	_ASSERTE( pTree );

    OCTREENODE_PTR pNode,pCurrentNode;
		
	pNode = pCurrentNode = OCTREE_GetRoot( pTree );

    _ASSERTE( pNode );
	
	while( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		for(int i = 0; i <= 7; i++)
		{
			int nIndex = OCTREENODE_GetChildIndex( pNode, i );
			if ( nIndex >= 0 )
			{ 
				OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, nIndex );
				if ( OCTREENODE_IsInside( pChildNode, position ) )
				{
					pNode = pChildNode; 
					break;
				}
			}
		}
		
		if(pNode == pCurrentNode)
		{
			return NULL;
		}
		else
		{
			pCurrentNode = pNode;
		}
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearWaterNodeIndices( pNode, pNearWaterNodeIndices );

	return OCTREENODE_GetNumNearWaterNodes( pNode );
}

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNearAirNodes
// Description:
//              Return air nodes that are near and visible
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              CVector &position   => point from which near path nodes are computed
// Output:
//              short *             => array of shorts that stores indices of
//                                     near path nodes
// Note:
//              assumes that pNearAirNodeIndices array is not empty
//
// ----------------------------------------------------------------------------
/*short OCTREE_GetNearAirNodes( OCTREE_PTR pTree, const CVector &position, short *pNearAirNodeIndices )
{
    _ASSERTE( pTree );

	OCTREENODE_PTR pNode = OCTREE_GetRoot( pTree );
    _ASSERTE( pNode );
	while ( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		CHECK_POINT_INSIDE_CHILDNODE( 0 );
		CHECK_POINT_INSIDE_CHILDNODE( 1 );
		CHECK_POINT_INSIDE_CHILDNODE( 2 );
		CHECK_POINT_INSIDE_CHILDNODE( 3 );
		CHECK_POINT_INSIDE_CHILDNODE( 4 );
		CHECK_POINT_INSIDE_CHILDNODE( 5 );
		CHECK_POINT_INSIDE_CHILDNODE( 6 );
		CHECK_POINT_INSIDE_CHILDNODE( 7 );
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearAirNodeIndices( pNode, pNearAirNodeIndices );

	return OCTREENODE_GetNumNearAirNodes( pNode );
}*/

short OCTREE_GetNearAirNodes( OCTREE_PTR pTree, const CVector &position, short *pNearAirNodeIndices )
{
	_ASSERTE( pTree );

    OCTREENODE_PTR pNode,pCurrentNode;
		
	pNode = pCurrentNode = OCTREE_GetRoot( pTree );

    _ASSERTE( pNode );
	
	while( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		for(int i = 0; i <= 7; i++)
		{
			int nIndex = OCTREENODE_GetChildIndex( pNode, i );
			if ( nIndex >= 0 )
			{ 
				OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, nIndex );
				if ( OCTREENODE_IsInside( pChildNode, position ) )
				{
					pNode = pChildNode; 
					break;
				}
			}
		}
		
		if(pNode == pCurrentNode)
		{
			return NULL;
		}
		else
		{
			pCurrentNode = pNode;
		}
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearAirNodeIndices( pNode, pNearAirNodeIndices );

	return OCTREENODE_GetNumNearAirNodes( pNode );
}


// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNearTrackNodes
// Description:
//              Return track nodes that are near and visible
// Input:
//              OCTREE_PTR pTree    => pointer to an octree
//              CVector &position   => point from which near path nodes are computed
// Output:
//              short *             => array of shorts that stores indices of
//                                     near path nodes
// Note:
//              assumes that pNearTrackNodeIndices array is not empty
//
// ----------------------------------------------------------------------------
/*short OCTREE_GetNearTrackNodes( OCTREE_PTR pTree, const CVector &position, short *pNearTrackNodeIndices )
{
    _ASSERTE( pTree );

	OCTREENODE_PTR pNode = OCTREE_GetRoot( pTree );
    _ASSERTE( pNode );
	while ( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		CHECK_POINT_INSIDE_CHILDNODE( 0 );
		CHECK_POINT_INSIDE_CHILDNODE( 1 );
		CHECK_POINT_INSIDE_CHILDNODE( 2 );
		CHECK_POINT_INSIDE_CHILDNODE( 3 );
		CHECK_POINT_INSIDE_CHILDNODE( 4 );
		CHECK_POINT_INSIDE_CHILDNODE( 5 );
		CHECK_POINT_INSIDE_CHILDNODE( 6 );
		CHECK_POINT_INSIDE_CHILDNODE( 7 );
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearTrackNodeIndices( pNode, pNearTrackNodeIndices );

	return OCTREENODE_GetNumNearAirNodes( pNode );
}
*/
short OCTREE_GetNearTrackNodes( OCTREE_PTR pTree, const CVector &position, short *pNearTrackNodeIndices )
{
	_ASSERTE( pTree );

    OCTREENODE_PTR pNode,pCurrentNode;
		
	pNode = pCurrentNode = OCTREE_GetRoot( pTree );

    _ASSERTE( pNode );
	
	while( OCTREENODE_GetNodeType( pNode ) == OCTREE_INTERNAL_NODE )
	{
		for(int i = 0; i <= 7; i++)
		{
			int nIndex = OCTREENODE_GetChildIndex( pNode, i );
			if ( nIndex >= 0 )
			{ 
				OCTREENODE_PTR pChildNode = OCTREE_GetNode( pTree, nIndex );
				if ( OCTREENODE_IsInside( pChildNode, position ) )
				{
					pNode = pChildNode; 
					break;
				}
			}
		}
		
		if(pNode == pCurrentNode)
		{
			return NULL;
		}
		else
		{
			pCurrentNode = pNode;
		}
	}

	_ASSERTE( OCTREENODE_GetNodeType( pNode ) == OCTREE_LEAF_NODE );

	OCTREENODE_GetNearTrackNodeIndices( pNode, pNearTrackNodeIndices );

	return OCTREENODE_GetNumNearAirNodes( pNode );
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
