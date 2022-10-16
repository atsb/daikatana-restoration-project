///////////////////////////////////////////////////////////////////////////////
//	Surface Sprites
//
//	Routines for managing surface sprites
//
//	Animation:	frames of a surface sprite are stored in sprite->skins
//
//	Nelno, 9/14/98
///////////////////////////////////////////////////////////////////////////////
#include	"gl_local.h"

#include "dk_point.h"
#include "dk_array.h"

#include	"gl_surfSprite.h"
#include	"gl_protex.h"

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

int RecursiveLightPoint (mnode_t *node, CVector start, CVector end, msurface_t *pPointSurf );
void BoundPoly (int numverts, CVector *verts, CVector &mins, CVector &maxs);
void BoundSubdividedPoly ( msurface_t *pSurf, CVector &mins, CVector &maxs);
inline void ClipSurfaceSpriteToFaceEdge(surfSprite_t& surfSprite, CVector& v0Pos, CVector& v1Pos);
void UpdateSurfaceSprites();

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

array_obj<surfSprite_t>	g_surfSpriteList;
extern			CVector	pointcolor;
CVector			vSurfaceMins, vSurfaceMaxs;
model_t			*pCurrentModel;

#define SURF_SPRITE_DURATION		(60.0f)
#define SURF_SPRITE_QUICK_DURATION	(2.0f)
#define SURF_SPRITE_FADE_TIME		(5.0f)
#define SURF_SPRITE_INV_FADE_TIME	(1.0f/SURF_SPRITE_FADE_TIME)

/*
  p1        p2
   \       /
	o-----o
	|     |
	|     |
	|     |
	o-----o
   /       \
  p0        p3
*/

glpoly_t *CreateClipPolyFromEdges( msurface_t *pSurf )
{
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
//	int			vertpage;
	CVector		vec;
	glpoly_t	*pPoly;

	pCurrentModel = r_worldmodel;

	pedges = pCurrentModel->edges;
	lnumverts = pSurf->numedges;
//	vertpage = 0;

	pPoly = AllocGLPoly( lnumverts );

	pPoly->next = NULL;
	pPoly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = pCurrentModel->surfedges[pSurf->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = pCurrentModel->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = pCurrentModel->vertexes[r_pedge->v[1]].position;
		}

		pPoly->verts[i] = vec;
	}

	return pPoly;
}

void CreateSurfaceSpritePoly( surfSprite_t& surfSprite, int nWidth, int nHeight )
{
	CVector		vForward, vRight, vUp;
	int			nOriginX, nOriginY;
	CVector		vAngles;
    int i, numverts = surfSprite.pSurf->polys->numverts;
//    float invfScale = 1.0f/surfSprite.fScale;
    sprite_point point;
    int v0, v1; // verts describing an edge of the face
    CVector v0Pos, v1Pos;
	glpoly_t	*pClipPoly;

	//	get normal of this surface
	surfSprite.vNormal = surfSprite.pSurf->plane->normal;


	if( !( surfSprite.pSurf->flags & SURF_PLANEBACK ) )
	{
		surfSprite.vNormal.Multiply( -1.0 );
	}

	VectorMA( surfSprite.vOrigin, surfSprite.vNormal, -0.125, surfSprite.vOrigin );

	surfSprite.vNormal.Normalize();

	surfSprite.vNormal.VectorToAngles( vAngles );
	vAngles.z = surfSprite.fRoll;
//    surfSprite.fScale = 1.0f;

	AngleToVectors( vAngles, vForward, vRight, vUp );
	vUp.Multiply( surfSprite.fScale );
	vRight.Multiply( surfSprite.fScale );

	nOriginX = nWidth / 2;
	nOriginY = nHeight / 2;
    
    // start off with the unclipped surface sprite
	// SCG[2/8/00]: The un-clipped surface sprite will have 4 verts
    surfSprite.numpoints = 4;

	VectorMA (surfSprite.vOrigin, vUp, -nOriginY, point.pos);
	VectorMA (point.pos, vRight, -nOriginX, point.pos);
	point.s = 0;
	point.t = 1;
    surfSprite.points[0] = point;

	VectorMA (surfSprite.vOrigin, vUp, nHeight - nOriginY, point.pos);
	VectorMA (point.pos, vRight, -nOriginX, point.pos);
	point.s = 0;
	point.t = 0;
    surfSprite.points[1] = point;

	VectorMA (surfSprite.vOrigin, vUp, nHeight - nOriginY, point.pos);
	VectorMA (point.pos, vRight, nWidth - nOriginX, point.pos);
	point.s = 1;
	point.t = 0;
    surfSprite.points[2] = point;

	VectorMA (surfSprite.vOrigin, vUp, -nOriginY, point.pos);
	VectorMA (point.pos, vRight, nWidth - nOriginX, point.pos);
	point.s = 1;
	point.t = 1;
    surfSprite.points[3] = point;

	if( surfSprite.pSurf->flags & SURF_DRAWFOG )
	{
		pClipPoly = CreateClipPolyFromEdges( surfSprite.pSurf );
	}
	else
	{
		pClipPoly = surfSprite.pSurf->polys;
	}

	numverts = pClipPoly->numverts;

	if( numverts > 10 )
	{
		if( surfSprite.pSurf->flags & SURF_DRAWFOG )
		{
			ri.X_Free( pClipPoly );
		}
		return;
	}

	for (i=0;i<numverts;i++)
	{
		v0 = i;
		v1 = (i<numverts-1)?(i+1):0;

		v0Pos = pClipPoly->verts[v0];
		v1Pos = pClipPoly->verts[v1];

		ClipSurfaceSpriteToFaceEdge(surfSprite, v0Pos, v1Pos);
	}

	if( surfSprite.pSurf->flags & SURF_DRAWFOG )
	{
		ri.X_Free( pClipPoly );
	}
}

inline void ClipSurfaceSpriteToFaceEdge(surfSprite_t& surfSprite, CVector& v0Pos, CVector& v1Pos)
{
    int i, numverts;
    int u0, u1; // verts describing an edge of the surface sprite
    CVector u0Pos, u1Pos, n;
    float t;
    sprite_point newpoint;
    bool u0Inside, u1Inside;

    // calc the inward normal
    n.CrossProduct(surfSprite.vNormal, v1Pos-v0Pos);

    // copy off old verts
    sprite_point oldpoints[16];
    numverts = surfSprite.numpoints;
    memcpy(oldpoints, surfSprite.points, numverts * sizeof(sprite_point));

    // clear points array
    surfSprite.numpoints=0;

    // clip the surface sprite edges
    for (i=0;i<numverts;i++)
    {
        u0 = i;
        u1 = (i<numverts-1)?(i+1):0;
        
        u0Pos = oldpoints[u0].pos;
        u1Pos = oldpoints[u1].pos;

        if ((u0Pos-v0Pos).DotProduct(n)>0)
            u0Inside = true;
        else
            u0Inside = false;
    
        if ((u1Pos-v0Pos).DotProduct(n)>0)
            u1Inside = true;
        else
            u1Inside = false;

        // if u0 is inside, add it
        if (u0Inside)
            surfSprite.points[surfSprite.numpoints++] = oldpoints[u0];
        
        // if they're on opposite sides, interpolate
        if (u0Inside!=u1Inside)
        {   
            t = (v0Pos-u0Pos).DotProduct(n)/(u1Pos-u0Pos).DotProduct(n);
            
            newpoint.pos = (1-t)*u0Pos + t*u1Pos;
            newpoint.s   = (1-t)*oldpoints[u0].s + t*oldpoints[u1].s;
            newpoint.t   = (1-t)*oldpoints[u0].t + t*oldpoints[u1].t;
            
            surfSprite.points[surfSprite.numpoints++] = newpoint;
        }
    }
}

void GetLightPoint( CVector& vOrg, CVector& vColor)
{
	CVector		vEnd;
	float		fRadius;
	
	if( r_worldmodel->lightdata == NULL )
	{
		vColor.x = vColor.y = vColor.z = 1.0;
		return;
	}
	
	vEnd.x = vOrg.x;
	vEnd.y = vOrg.y;
	vEnd.z = vOrg.z - 2048;
	
	fRadius = RecursiveLightPoint( r_worldmodel->nodes, vOrg, vEnd, NULL );
	
	if( fRadius == -1 )
	{
		vColor = vec3_origin;
	}
	else
	{
		vColor = pointcolor;
	}
}

void GetPolyCenter( glpoly_t *pPoly, CVector& vCenter )
{
	vCenter.Zero();

	for( int i = 0; i < pPoly->numverts; i++ )
	{
		vCenter += pPoly->verts[i];
	}

	vCenter.x /= ( float )i;
	vCenter.y /= ( float )i;
	vCenter.z /= ( float )i;
}

///////////////////////////////////////////////////////////////////////////////
//	AddSurfaceSpriteBeam
//
//	adds beams of light to a surface sprite
///////////////////////////////////////////////////////////////////////////////

void AddSurfaceSpriteBeam( surfSprite_t &surfSprite )
{
	beam_t		*beam;
	beamSeg_t	*beamSeg;

	beam = ri.beam_AllocBeam ();

	if (!beam)
		return;

	beamSeg = ri.beam_AllocSeg ();
	if (!beamSeg)
		return;

	beam->firstSeg = beamSeg;
	beam->alpha = 0.1;
	beam->startRadius = 2.0 * surfSprite.fScale;
	beam->endRadius = 4.0 * surfSprite.fScale;
	beam->beamStart = surfSprite.vOrigin;
	beam->beamLength = 560.0;
	VectorMA (surfSprite.vOrigin, surfSprite.pSurf->plane->normal, beam->beamLength, beam->beamEnd);
	beam->numSegs = 1;
	beam->flags = BEAMFLAG_ALPHAFADE;
	beam->texIndex = BEAM_TEX_NONE;

	beam->rgbColor = surfSprite.vLightValue;

	beamSeg->segStart = beam->beamStart;
	beamSeg->segEnd = beam->beamEnd;
	beamSeg->segNormal = surfSprite.pSurf->plane->normal;
	beamSeg->nextSeg = NULL;
	beamSeg->alpha = 1.0;
}

qboolean PointInSurface( msurface_t *pSurface, CVector vPoint, float radius, CVector& vMins, CVector& vMaxs )
{
	qboolean bTest1, bTest2, bTest3;

	bTest1 = bTest2 = false;

	bTest3 = true;

	switch( pSurface->plane->type )
	{
	case PLANE_X:
		if( ( vPoint.y + radius >= vMins.y ) && ( vPoint.y - radius <= vMaxs.y ) )
		{
			bTest1 = true;
		}
		if( ( vPoint.z + radius >= vMins.z ) && ( vPoint.z - radius <= vMaxs.z ) )
		{
			bTest2 = true;
		}
		break;
	case PLANE_Y:
		if( ( vPoint.x + radius >= vMins.x ) && ( vPoint.x - radius <= vMaxs.x ) )
		{
			bTest1 = true;
		}
		if( ( vPoint.z + radius >= vMins.z ) && ( vPoint.z - radius <= vMaxs.z ) )
		{
			bTest2 = true;
		}
		break;
	case PLANE_Z:
		if( ( vPoint.x + radius >= vMins.x ) && ( vPoint.x - radius <= vMaxs.x ) )
		{
			bTest1 = true;
		}
		if( ( vPoint.y + radius >= vMins.y ) && ( vPoint.y - radius <= vMaxs.y ) )
		{
			bTest2 = true;
		}
		break;
	default:
		bTest3 = false;
		if( ( vPoint.x + radius >= vMins.x ) && ( vPoint.x - radius <= vMaxs.x ) )
		{
			bTest1 = true;
		}
		if( ( vPoint.y + radius >= vMins.y ) && ( vPoint.y - radius <= vMaxs.y ) )
		{
			bTest2 = true;
		}
		if( ( vPoint.z + radius >= vMins.z ) && ( vPoint.z - radius <= vMaxs.z ) )
		{
			bTest3 = true;
		}

		break;
	}

	if( bTest1 && bTest2 && bTest3 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

#define MAX_HIT_SURFACES 4

void R_SurfaceSpriteAdd( CVector& vOrigin, CVector& vEntOrigin, CVector& vEntAngles, 
						 void *pHitModel, int nIndex, void *pSpriteModel, int nFrame, 
						 float fRoll, float fScale, byte nFlags )
{
	faceList_t		*pFaceList;
	msurface_t		*pCurrentSurface, *pHitSurface;
    msurface_t*     hitSurfaces[MAX_HIT_SURFACES];
    int             numHitSurfaces;
	surfSprite_t	surfSprite;
	dsprite_t		*pSprite;
//	float			fClosestDist;
	CVector			vCenter, vTempAngles, vTempOrigin;
	CVector			vPolyVerts[4];
	int				nWidth, nHeight;
    
    if (0==gl_surfacesprites->value)
        return;
     
	pCurrentModel = r_worldmodel;

	if( pCurrentModel == NULL || ( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		return;
	}

	if( pCurrentModel->planePolys == NULL )
	{
		return;
	}

	for( int i = 0; i < MAX_HIT_SURFACES; i++ )
	{
		hitSurfaces[i] = NULL;
	}
	surfSprite.vOrigin = vOrigin;
	surfSprite.nFrame = nFrame;
	surfSprite.fScale = fabs( fScale );
	surfSprite.nFlags = nFlags;
	surfSprite.fRoll = fRoll;

	// SCG[5/3/99]: Get a pointer to the sprite model
	if( (pCurrentModel = ( model_t * ) pSpriteModel) == NULL)   // Logic[8/4/99]: Bail out!
    {
#ifdef _DEBUG
        ri.Con_Printf(PRINT_DEVELOPER, "%s:%i - NULL SPRITE MODEL, does resource exist?\n", __FILE__, __LINE__);
#endif _DEBUG
        return;
    }
	// SCG[5/4/99]: Get a pointer to the sprite data
	pSprite = ( dsprite_t * ) pCurrentModel->extradata;

	surfSprite.pImage = pCurrentModel->skins[surfSprite.nFrame];
	nWidth = pSprite->frames[surfSprite.nFrame].width;
	nHeight = pSprite->frames[surfSprite.nFrame].height;

	surfSprite.pModel = ( model_t * ) pSpriteModel;

	pCurrentModel = ( model_t * ) pHitModel;

	if( !pCurrentModel )
		return;

	if( pCurrentModel->type != mod_brush )
		return;

	pFaceList = pCurrentModel->planePolys->planes[nIndex];

	if( pFaceList == NULL )
	{
//		ri.Con_Printf( PRINT_ALL, "GL_SurfaceSpriteAdd: pFaceList == NULL\n" );
		return;
	}

//	bool bOnBModel = ( pCurrentModel->firstmodelsurface > 0 );

//	fClosestDist = 9999.0;

	if(surfSprite.nFrame < 0 || surfSprite.nFrame >= pSprite->numframes )
		surfSprite.nFrame = 0;
    
    numHitSurfaces=0;
	for( i = 0; i < pFaceList->numFaces; i++ )
	{
		pCurrentSurface = pCurrentModel->surfaces + pFaceList->faceNum[i];

		if( pCurrentSurface->polys->next )
		{
			BoundSubdividedPoly( pCurrentSurface, vSurfaceMins, vSurfaceMaxs );
		}
		else
		{
			BoundPoly( pCurrentSurface->polys->numverts, pCurrentSurface->polys->verts, vSurfaceMins, vSurfaceMaxs );
		}

		if( PointInSurface( pCurrentSurface, vOrigin, nWidth*fScale, vSurfaceMins, vSurfaceMaxs ) && numHitSurfaces<MAX_HIT_SURFACES)
            hitSurfaces[numHitSurfaces++] = pCurrentSurface;
	}

	if( surfSprite.nFlags & SSF_FULLBRIGHT )
	{
		surfSprite.vLightValue.Set( 1.0, 1.0, 1.0 );
	}

    for (i=0;i<numHitSurfaces;i++)
    {
        pHitSurface = hitSurfaces[i];

    	if( pHitSurface == NULL || ( pHitSurface->texinfo->flags & ( SURF_MIDTEXTURE | SURF_SKY ) ) )
    	{
    		return;
    	}
    
    	// SCG[5/4/99]: Set the surface pointer
    	surfSprite.pSurf = pHitSurface;
    
    	if( surfSprite.nFlags & SSF_LIGHTBEAM )
    	{
    		//	bullet holes emitting light
    		if( !( pHitSurface->texinfo->flags & SURF_BULLETLIGHT ) )
    		{ 
    			surfSprite.nFlags &= ~SSF_LIGHTBEAM;
    			GetLightPoint( surfSprite.vOrigin, surfSprite.vLightValue );
    		}
    		else if( !( surfSprite.nFlags & SSF_FULLBRIGHT ) )
    		{
    			if( pHitSurface->texinfo->color == 0 )
    			{
    				surfSprite.vLightValue.x = 0.85;
    				surfSprite.vLightValue.y = 1.0;
    				surfSprite.vLightValue.z = 1.0; 
    			}
    			else
    			{
					surfSprite.vLightValue.x = ( float )( ( pHitSurface->texinfo->color >> 8 ) & 0xf8 ) / 255.0;
					surfSprite.vLightValue.y = ( float )( ( pHitSurface->texinfo->color >> 3 ) & 0xfc ) / 255.0;
					surfSprite.vLightValue.z = ( float )( ( pHitSurface->texinfo->color << 3 ) & 0xf8 ) / 255.0;
    			}
    		}

			AddSurfaceSpriteBeam( surfSprite );

    	}
    	else
    	{
    		if( !( surfSprite.nFlags & SSF_FULLBRIGHT ) )
			{
	    		GetLightPoint( surfSprite.vOrigin, surfSprite.vLightValue );
			}
    	}
    
    	CreateSurfaceSpritePoly( surfSprite, nWidth, nHeight );
   
        // if we're out of space, expire the oldest
        if (MAX_SURF_SPRITES == g_surfSpriteList.Num())
            const_cast<surfSprite_t *>(&g_surfSpriteList[0])->expire_time = r_newrefdef.time;

        // if there are too many on this surface, expire the oldest on this surface
        if (MAX_SPRITES_PER_SURFACE <= surfSprite.pSurf->nNumSurfSprites)
        {
            int iTest;

            // find the oldest unexpired on this surface
            for (iTest=0;iTest<g_surfSpriteList.Num();iTest++)
            {
                surfSprite_t *pTestSprite;

                pTestSprite = const_cast<surfSprite_t *>(&g_surfSpriteList[iTest]);

                if (pTestSprite->pSurf == surfSprite.pSurf && pTestSprite->expire_time > r_newrefdef.time)
                {
                    pTestSprite->expire_time = r_newrefdef.time;
                    break;
                }
            }
        }

        // add it to the end
		if( surfSprite.nFlags & SSF_QUICKFADE )
		{
	        surfSprite.expire_time = r_newrefdef.time + SURF_SPRITE_QUICK_DURATION;
		}
		else
		{
	        surfSprite.expire_time = r_newrefdef.time + SURF_SPRITE_DURATION;
		}

        surfSprite.pSurf->nNumSurfSprites++;
        surfSprite.alpha = 1.0f;
        g_surfSpriteList.Add(surfSprite);
    }
}

int R_TranslateAlphaSurface (entity_t *pent);

void R_DrawSurfaceSprites()
{
	int	matrix_modified, i;
	surfSprite_t *pSurfSprite;

    if (0==gl_surfacesprites->value) 
        return;

	if( g_surfSpriteList.Num() == 0 )
		return;

    UpdateSurfaceSprites();

    qglLoadMatrixf (r_world_matrix);

	GL_SetState( ( GLSTATE_PRESET1 | GLSTATE_BLEND ) & ~GLSTATE_DEPTH_MASK );

	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	GL_TexEnv( GL_MODULATE );

	// SCG[5/18/99]: Draw surface sprites for opaque surfaces
	for( i = 0; i < g_surfSpriteList.Num(); i++ )
	{
		pSurfSprite = const_cast<surfSprite_t *>(&g_surfSpriteList[i]);

		if( ( pSurfSprite->pSurf == NULL ) || 
			( pSurfSprite->pImage == NULL ) )
		{
			continue;
		}

		if( pSurfSprite->pSurf->visframe != r_framecount )
			continue;

		matrix_modified = R_TranslateAlphaSurface( pSurfSprite->pSurf->entity );

		qglColor4f( pSurfSprite->vLightValue.x, pSurfSprite->vLightValue.y, pSurfSprite->vLightValue.z, pSurfSprite->alpha );

		GL_Bind( pSurfSprite->pImage->texnum );
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		qglBegin( GL_POLYGON );
		for( int j = 0; j < pSurfSprite->numpoints; j++ )
		{
			qglTexCoord2f (pSurfSprite->points[j].s, pSurfSprite->points[j].t);
			qglVertex3fv (pSurfSprite->points[j].pos);
		}
		qglEnd ();

        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if ( matrix_modified )
			qglPopMatrix();
	}

	GL_TexEnv (GL_REPLACE);
}

void UpdateSurfaceSprites()
{
    int i;
    surfSprite_t *pSprite;

    i = 0;
    while (i<g_surfSpriteList.Num())
    {
        if (g_surfSpriteList[i].expire_time < r_newrefdef.time)
        {
            pSprite = const_cast<surfSprite_t *>(&g_surfSpriteList[i]);

            pSprite->alpha = (pSprite->expire_time+SURF_SPRITE_FADE_TIME-r_newrefdef.time)*SURF_SPRITE_INV_FADE_TIME;

            if (pSprite->alpha<0)
            {
                pSprite->pSurf->nNumSurfSprites--;
                g_surfSpriteList.Remove(i);
            }
            else
                i++;
        }
        else
        {
            i++;
        }
    }
}


void R_InitSurfaceSprites (void)
{
    g_surfSpriteList.Length(0);
}
