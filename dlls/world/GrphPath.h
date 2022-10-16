#ifndef BESTPATH_H
#define BESTPATH_H


int PATH_AllocateGraphPath();
void PATH_DestroyGraphPath();
PATHNODE_PTR PATH_ComputePath( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode, int &nPathLength );
int PATH_ComputePath( NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode );
int PATH_ComputePath( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pStartNode, MAPNODE_PTR pEndNode, PATHLIST_PTR pPathList);


#endif BESTPATH_H