
#ifndef _OCTREE_H
#define _OCTREE_H

#define OCTREE_INTERNAL_NODE	0
#define OCTREE_LEAF_NODE		1

#define MAX_NUM_NEAR_PATHNODES	18

const char szOctreeSignature[] = "Octree: used for storing PathNodes";
const int nOctreeVersion = 0;

//-----------------------------------------------------------------------------

typedef struct _OctreeNode
{
	short nNodeType;
	short nIndex;

	CVector center;
	CVector mins;
	CVector maxs;

	short nParentNodeIndex;
	short aChildrenNodeIndices[8];

	short nNumNearGroundNodes;
	short *pNearGroundNodeIndices;
	
	short nNumNearWaterNodes;
	short *pNearWaterNodeIndices;

	short nNumNearAirNodes;
	short *pNearAirNodeIndices;

	short nNumNearTrackNodes;
	short *pNearTrackNodeIndices;

	short bRemove;				// flag used while building the tree

} OCTREENODE, *OCTREENODE_PTR;

typedef struct _Octree
{
	OCTREENODE_PTR pRoot;

	int nNumNodes;
	int nNumAllocatedNodes;
	OCTREENODE_PTR pNodes;
} OCTREE, *OCTREE_PTR;

//-----------------------------------------------------------------------------

OCTREENODE_PTR OCTREENODE_Construct();
OCTREENODE_PTR OCTREENODE_Destruct( OCTREENODE_PTR pNode );

void OCTREENODE_ReadData( OCTREENODE_PTR pNode, FILE *in );
void OCTREENODE_WriteData( OCTREENODE_PTR pNode, FILE *out );
void OCTREENODE_BuildOctree( OCTREE_PTR pTree, int nOctreeNodeIndex );

//-----------------------------------------------------------------------------

OCTREE_PTR OCTREE_Construct();
OCTREE_PTR OCTREE_Destruct( OCTREE_PTR pTree );

// ----------------------------------------------------------------------------
//
// Name:		OCTREE_GetNumNodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline int OCTREE_GetNumNodes( OCTREE_PTR pTree )
{
	return pTree->nNumNodes;
}

void OCTREE_ReadData( OCTREE_PTR pTree, FILE *in );
void OCTREE_WriteData( OCTREE_PTR pTree, FILE *out );

void OCTREE_BuildOctree( OCTREE_PTR pTree );

short OCTREE_GetNearGroundNodes( OCTREE_PTR pTree, const CVector &position, short *pNearGroundNodeIndices );
short OCTREE_GetNearWaterNodes( OCTREE_PTR pTree, const CVector &position, short *pNearWaterNodeIndices );
short OCTREE_GetNearAirNodes( OCTREE_PTR pTree, const CVector &position, short *pNearAirNodeIndices );
short OCTREE_GetNearTrackNodes( OCTREE_PTR pTree, const CVector &position, short *pNearTrackNodeIndices );

//-----------------------------------------------------------------------------

extern OCTREE_PTR pPathOctree;

#endif _OCTREE_H
