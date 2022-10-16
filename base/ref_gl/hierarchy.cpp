///////////////////////////////////////////////////////////////////////////////
//	hierarchy.c
//
//	hierachical model vertex transformation code
///////////////////////////////////////////////////////////////////////////////

#include "gl_local.h"
#include <alias_quant.h>
///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//CVector			hr_forward, hr_right, hr_up;
CVector			my_forward, my_right, my_up;

//float			hr_oldworldtransform[3][4];
//float			hr_worldtransform[3][4];
//float			hr_transform[3][4];
//static float	tmatrix [3][4], t2matrix [3][4];

daliasframe_t	*hr_thisframe;
daliasframe_t	*hr_lastframe;

//static	CVector         hr_lerp_frontv;
//static	CVector			hr_lerp_backv; 
//static	CVector			hr_lerp_move;
//static	float			hr_front_lerp;

//void VecToAngles (CVector &in, CVector &angles);

void R_GetModelHardpoint(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt)
{
	// WARNING: amw: this function won't work with the hi rez models yet.. need to change daliasframe_t
	//
	model_t			*pModelData = NULL;
	dmdl_t			*pModelHdr = NULL;
	daliasframe_t	*pCurFrame = NULL;
	daliasframe_t	*pLastFrame = NULL;
	daliasframe2_t	*pCurFrame2 = NULL;
	daliasframe2_t	*pLastFrame2 = NULL;
	dsurface_t		*pSurface = NULL;
	dtrivertx_t		*pCurVerts = NULL, *pLastVerts = NULL;
	dtrivertx2_t	*pCurVerts2 = NULL, *pLastVerts2 = NULL;
	int				triIndex = 0, surfIndex = 0;
	CVector			vCurVert, vLastVert;

	// is this a valid model pointer?
	if( ent.model == NULL )
	{
		return;
	}

	// the surface was not found
	if( pSurfName == NULL )
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
	if( pModelHdr->version == ALIAS_VERSION )
	{
		pCurFrame = (daliasframe_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (curFrame * pModelHdr->framesize));
		pLastFrame = (daliasframe_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (lastFrame * pModelHdr->framesize));
		if( pCurFrame == NULL || pLastFrame == NULL )
		{
			return;
		}
	}
	else if( pModelHdr->version == ALIAS_VERSION2 )
	{
		pCurFrame2 = (daliasframe2_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (curFrame * pModelHdr->framesize));
		pLastFrame2 = (daliasframe2_t *)((byte *)pModelHdr + pModelHdr->ofs_frames + (lastFrame * pModelHdr->framesize));
		if( pCurFrame2 == NULL || pLastFrame2 == NULL )
		{
			return;
		}
	}


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

	CVector move, frontv, backv;

	if( pModelHdr->version == ALIAS_VERSION )
	{
		move = ent.backlerp * pLastFrame->translate + frontlerp * pCurFrame->translate;
		frontv = frontlerp * pCurFrame->scale;
		backv = ent.backlerp * pLastFrame->scale;
	}
	else if( pModelHdr->version == ALIAS_VERSION2 )
	{
		move = ent.backlerp * pLastFrame2->translate + frontlerp * pCurFrame2->translate;
		frontv = frontlerp * pCurFrame2->scale;
		backv = ent.backlerp * pLastFrame2->scale;
	}

	// setup the transformation matrix
	CVector angs, forward, right, up;
	AngleToVectors( ent.angles, forward, right, up );
	right = -right;

	// change matrix orientation
	my_forward.x = forward.x; my_forward.y = right.x;  my_forward.z = up.x;
	my_right.x = forward.y; my_right.y = right.y; my_right.z = up.y;
	my_up.x = forward.z; my_up.y = right.z; my_up.z = up.z;

	CVector	temp;
	temp.Set( 0, 0, 0 );

	for( int index = 0; index < 3; index++ )
	{
		if( pModelHdr->version == ALIAS_VERSION )
		{
			pCurVerts = pCurFrame->verts + pTriangle->index_xyz[index];
			pLastVerts = pLastFrame->verts + pTriangle->index_xyz[index];
			vCurVert.x = pCurVerts->v[0];
			vCurVert.y = pCurVerts->v[1];
			vCurVert.z = pCurVerts->v[2];

			vLastVert.x = pLastVerts->v[0];
			vLastVert.y = pLastVerts->v[1];
			vLastVert.z = pLastVerts->v[2];
		}
		else if( pModelHdr->version == ALIAS_VERSION2 )
		{
			pCurVerts2 = pCurFrame2->verts + pTriangle->index_xyz[index];
			pLastVerts2 = pLastFrame2->verts + pTriangle->index_xyz[index];
			vCurVert.x = pCurVerts2->v >> X_BIT_SHIFT;
			vCurVert.y = ( pCurVerts2->v & Y_BIT_MASK ) >> Y_BIT_SHIFT;
			vCurVert.z = pCurVerts2->v & Z_BIT_MASK;

			vLastVert.x = pLastVerts2->v >> X_BIT_SHIFT;
			vLastVert.y = ( pLastVerts2->v & Y_BIT_MASK ) >> Y_BIT_SHIFT;
			vLastVert.z = pLastVerts2->v & Z_BIT_MASK;
		}

/*		if (ent.render_scale.Length() > 0.01)
		{
			vCurVert.x *= ent.render_scale.x;
			vCurVert.y *= ent.render_scale.y;
			vCurVert.z *= ent.render_scale.z;

			vLastVert.x *= ent.render_scale.x;
			vLastVert.y *= ent.render_scale.y;
			vLastVert.z *= ent.render_scale.z;
		}
*/
		temp.x += (move.x + ( vLastVert.x * backv.x ) + ( vCurVert.x * frontv.x ) );
		temp.y += (move.y + ( vLastVert.y * backv.y ) + ( vCurVert.y * frontv.y ) );
		temp.z += (move.z + ( vLastVert.z * backv.z ) + ( vCurVert.z * frontv.z ) );
	}

	temp.x = temp.x / 3;
	temp.y = temp.y / 3;
	temp.z = temp.z / 3;

	hardPt.x = DotProduct( temp, my_forward ) + ent.origin.x;
	hardPt.y = DotProduct( temp, my_right ) + ent.origin.y;
	hardPt.z = DotProduct( temp, my_up ) + ent.origin.z;
}

// gets the hardpoint in modelspace coords
bool R_GetModelHardpointRel(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt)
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
    CVector         hardPt0, hardPt1, hardPt2;

	// is this a valid model pointer?
	if( ent.model == NULL )
	{
		return false;
	}

	// the surface was not found
	if( pSurfName == NULL )
		return false;

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
		return false;

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

	// lerp point1
	pCurVerts = pCurFrame->verts + pTriangle->index_xyz[0];
	pLastVerts = pLastFrame->verts + pTriangle->index_xyz[0];
    hardPt0.x = (move.x + pLastVerts->v[0] * backv.x + pCurVerts->v[0] * frontv.x);
	hardPt0.y = (move.y + pLastVerts->v[1] * backv.y + pCurVerts->v[1] * frontv.y);
	hardPt0.z = (move.z + pLastVerts->v[2] * backv.z + pCurVerts->v[2] * frontv.z);
	
    pCurVerts = pCurFrame->verts + pTriangle->index_xyz[1];
	pLastVerts = pLastFrame->verts + pTriangle->index_xyz[1];
    hardPt1.x = (move.x + pLastVerts->v[0] * backv.x + pCurVerts->v[0] * frontv.x);
	hardPt1.y = (move.y + pLastVerts->v[1] * backv.y + pCurVerts->v[1] * frontv.y);
	hardPt1.z = (move.z + pLastVerts->v[2] * backv.z + pCurVerts->v[2] * frontv.z);
	
    pCurVerts = pCurFrame->verts + pTriangle->index_xyz[2];
	pLastVerts = pLastFrame->verts + pTriangle->index_xyz[2];
    hardPt2.x = (move.x + pLastVerts->v[0] * backv.x + pCurVerts->v[0] * frontv.x);
	hardPt2.y = (move.y + pLastVerts->v[1] * backv.y + pCurVerts->v[1] * frontv.y);
	hardPt2.z = (move.z + pLastVerts->v[2] * backv.z + pCurVerts->v[2] * frontv.z);

    hardPt = (1.0f/3.0f)*(hardPt0+hardPt1+hardPt2);

//	hardPt.x *= ent.render_scale.x;
//	hardPt.y *= ent.render_scale.y;
//	hardPt.z *= ent.render_scale.z;

	return true;
}



