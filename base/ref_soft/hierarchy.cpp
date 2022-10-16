///////////////////////////////////////////////////////////////////////////////
//	hierarchy.c
//
//	hierachical model vertex transformation code
///////////////////////////////////////////////////////////////////////////////

#include	"r_local.h"

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

CVector			hr_forward, hr_right, hr_up;
CVector			my_forward, my_right, my_up;

float			hr_oldworldtransform[3][4];
float			hr_worldtransform[3][4];
float			hr_transform[3][4];

daliasframe_t	*hr_thisframe;
daliasframe_t	*hr_lastframe;
CVector         hr_lerp_frontv;
CVector			hr_lerp_backv; 
CVector			hr_lerp_move;

void R_GetModelHardpoint(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt)
{
	// WARNING: amw: this function won't work with the hi rez models yet.. need to change daliasframe_t
	//
	model_t			*pModelData = NULL;
	dmdl_t			*pModelHdr = NULL;
	daliasframe_t	*pCurFrame = NULL;
	daliasframe_t	*pLastFrame = NULL;
	dsurface_t		*pSurface = NULL;
	dtrivertx_t		*pCurVerts = NULL, *pLastVerts = NULL;
	int				triIndex = 0, surfIndex = 0;

	// is this a valid model pointer?
	if (!ent.model)
		return;
	
	// the surface was not found
	if (!pSurfName)
		return;

	// get a pointer to the model data
    pModelData = (model_t *)ent.model;
	// get a pointer to the model header
	pModelHdr = (dmdl_t *)pModelData->extradata;

	// find the surface
	dsurface_t *pSurfaceList = (dsurface_t *) ((byte *) pModelHdr + pModelHdr->ofs_surfaces);
	for (int i = 0; i < pModelHdr->num_surfaces; i++)
	{
		if (!stricmp (pSurfaceList[i].name, pSurfName))
		{
			surfIndex = i;	//AMW: not needed once new model format is used
			pSurface = &pSurfaceList[i];
			break;
		}
	}

	// did we find this hardpoint surface?
	if (!pSurface)
		return;

	// validate the frame numbers
	if ((curFrame >= pModelHdr->num_frames) || (curFrame < 0))
	{
		curFrame = 0;
	}
	if ((lastFrame >= pModelHdr->num_frames) || (lastFrame < 0))
	{
		lastFrame = 0;
	}

	// get pointers to the frame data
	pCurFrame = (daliasframe_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (curFrame * pModelHdr->framesize));
	pLastFrame = (daliasframe_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (lastFrame * pModelHdr->framesize));

#if 0
	// get the tri-index of the hardpoint poly
	triIndex = pSurface->hpTriIndex;
#else
	dtriangle_t *pTriangleList = (dtriangle_t *)((byte *)pModelHdr + pModelHdr->ofs_tris);

	for (i=0;i<pModelHdr->num_tris;i++)
	{
		if (pTriangleList[i].index_surface == surfIndex)
		{
			triIndex = i;
			break;
		}
	}
#endif

	// validate the index
	if (triIndex < 0 || triIndex > pModelHdr->num_tris)
		triIndex = 0;

	// get a pointer to the triangle data
	dtriangle_t	*pTriangle = &pTriangleList[triIndex];

	// decode the vertex info (copied from GL_LerpVerts() in gl_mesh.cpp)
	float frontlerp = 1.0 - ent.backlerp;

	CVector move = ent.backlerp * pLastFrame->translate + frontlerp * pCurFrame->translate;
	CVector frontv = frontlerp * pCurFrame->scale;
	CVector backv = ent.backlerp * pLastFrame->scale;

	// setup the transformation matrix
	CVector angs, forward, right, up;
	AngleToVectors( ent.angles, forward, right, up );
	right = -right;

	// change matrix orientation
	my_forward.x = forward.x; my_forward.y = right.x;  my_forward.z = up.x;
	my_right.x = forward.y; my_right.y = right.y; my_right.z = up.y;
	my_up.x = forward.z; my_up.y = right.z; my_up.z = up.z;

	CVector	p1, p2, p3, temp;

	// lerp point1
	pCurVerts = pCurFrame->verts + pTriangle->index_xyz[0];
	pLastVerts = pLastFrame->verts + pTriangle->index_xyz[0];
	temp.x = (move.x + pLastVerts->v[0] * backv.x + pCurVerts->v[0] * frontv.x);
	temp.y = (move.y + pLastVerts->v[1] * backv.y + pCurVerts->v[1] * frontv.y);
	temp.z = (move.z + pLastVerts->v[2] * backv.z + pCurVerts->v[2] * frontv.z);

	hardPt.x = DotProduct( temp, my_forward ) + ent.origin.x;
	hardPt.y = DotProduct( temp, my_right ) + ent.origin.y;
	hardPt.z = DotProduct( temp, my_up ) + ent.origin.z;

	temp = p1 + p2 + p3;

	return;
}
