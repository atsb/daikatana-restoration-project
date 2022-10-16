
#include "gl_local.h"

// SCG[3/30/99]: Fog volume support

//void DrawGLPoly( msurface_t *pSurf );
//void DrawGLMPoly( msurface_t *pSurf );
//void DrawGLPolyLightmap( msurface_t *pSurf );

void R_DrawFogSurfaces();
void R_FogPrecalc();
void R_DrawFog();
inline float R_CalcDensityAdjustment(float, float);
float R_FogTraceLine(CVector&, bool);
void R_DrawOutlines();

#define MAX_FOGVOLUMES	64
fogvolume_t	r_fogvolumes[MAX_FOGVOLUMES];

//image_t *R_TextureAnimation( mtexinfo_t *tex );

CVector	g_vColor;

static float gScale;
static float gfMinFogAlt, gfMaxFogAlt, gfInvFogDepth;
static bool gInsideFog;
static CVector gvCamPos;
int		r_fog_volume_count;
int		r_current_fog_volume;

//static bool gSurfaceCount;
void R_BuildConvexHull();

fogsample_t gFogSample;

inline bool R_SurfIsBackfaced(msurface_t *pSurf, CVector &pos)
{
    float dot;

    dot = pos.DotProduct(pSurf->plane->normal) - pSurf->plane->dist;

    if (pSurf->flags & SURF_PLANEBACK)
        dot = -dot;
    
    if (dot < -BACKFACE_EPSILON)
        return true;
    else
        return false;
}

int R_FogAddVolume( int brushnum )
{
	r_fogvolumes[r_fog_volume_count].brushnum = brushnum;
	r_fogvolumes[r_fog_volume_count].surface_count = 0;
	r_fogvolumes[r_fog_volume_count].hull.RemoveAll();

	r_fog_volume_count++;

	return r_fog_volume_count - 1;
}

int R_FogVolumeForBrush( int brushnum )
{
	int found_parent;

	found_parent = 0;

	if( r_fog_volume_count == 0 )
	{
		return R_FogAddVolume( brushnum );
	}

	for( int i = 0; i < r_fog_volume_count; i++ )
	{
		if( brushnum == r_fogvolumes[i].brushnum )
		{
			found_parent = 1;
			break;
		}
	}

	if( found_parent )
	{
		return i;
	}

	return R_FogAddVolume( brushnum );
}

void R_DrawFogVolumes()
{
	int i;
	unsigned int	nStateFlags;

	if( ( r_fog_volume_count > MAX_FOGVOLUMES ) || ( r_fog_volume_count == 0 ) || ( gl_vfog->value == 0 ) )
	{
		for( i = 0; i < r_fog_volume_count; i++ )
		{
			r_fogvolumes[i].surfaces = NULL;
		}

		return;
	}

	//
	// go back to the world matrix
	//
    qglLoadMatrixf (r_world_matrix);

#pragma message( "// SCG[6/4/99]: StateHack" )

	nStateFlags = GLSTATE_PRESET2 | GLSTATE_FOG | GLSTATE_BLEND;
	nStateFlags &= ~GLSTATE_DEPTH_MASK;

	GL_SetState( nStateFlags );
	GL_TexEnv( GL_MODULATE );

	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	qglShadeModel (GL_SMOOTH);

    for( i = 0; i < r_fog_volume_count; i++ )
    {
        r_current_fog_volume = i;

        if( r_fogvolumes[i].surfaces == NULL )
        {
            continue;
        }
        
        if (gl_showtris->value!=2)
        {
            R_FogPrecalc();
            R_DrawFog();
        }
   
        R_DrawOutlines();

        r_fogvolumes[i].surfaces = NULL;
        r_fogvolumes[r_current_fog_volume].surface_count = 0;
    }
}

void R_FogPrecalc()
{
	msurface_t*	pSurf;
	CVector		vCamDist;
    int         surfNum;

    // init our camera position
    gvCamPos = r_newrefdef.vieworg;

	gfMaxFogAlt = r_fogvolumes[r_current_fog_volume].vMaxs.z;
    gfMinFogAlt = r_fogvolumes[r_current_fog_volume].vMins.z;
    gfInvFogDepth = 1.0f/(gfMaxFogAlt-gfMinFogAlt);

	gScale = r_fogvolumes[r_current_fog_volume].distance;
	g_vColor = r_fogvolumes[r_current_fog_volume].vColor;

	// are we inside or outside the convex hull?
    gInsideFog = true;
    surfNum = 0;

	for( surfNum = 0; surfNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfNum++ )
	{
		pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfNum );

		if( gInsideFog == false )
			break;

        if (R_SurfIsBackfaced(pSurf, gvCamPos))
			gInsideFog = false;
    }

    // precalc the cam distance to each hull plane
    for( surfNum = 0; surfNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfNum++ )
    {
		pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfNum );

        // camera distance to test plane is stored in texturemins since it's not used for fogsurfaces
        *(float*)pSurf->texturemins = (gvCamPos.DotProduct(pSurf->plane->normal)-pSurf->plane->dist);
    }
}

void DrawGLMPoly( msurface_t *pSurf );
//void protex_GL_Bind( image_t *image );
void DrawGLPoly( msurface_t *pSurf );
//void surfSprite_DrawSpritesForSurface( msurface_t *pSurf, int isAlphaSurface);

//extern msurface_t *r_surfs_damage;

//void R_SetCacheState( msurface_t *pSurf );
//void R_BuildLightMap (msurface_t *pSurf, byte *dest, int stride);
void DrawGLPolyLightmap( msurface_t *pSurf );
void R_RenderBrushPoly( msurface_t *pSurf );

//#define GL_LIGHTMAP_FORMAT GL_RGBA
//#define FLOWING_SPEED	( ( r_newrefdef.time * 0.5 ) - ( int )( r_newrefdef.time * 0.5) )

/*
void R_DrawFogSurfaces()
{
	int			i, map;
	msurface_t*	pSurf;
	image_t*	image;
	unsigned	lmtex;
	entity_t	ent;
	qboolean	is_dynamic = false;
	glpoly_t	*pPoly;
	msurface_t	*pTestSurface;
	qboolean	bGrabbedTestSurface;
	float		fScroll;
	unsigned int nStateFlags;

	if( gl_fogsurfaces->value == 0 )
	{
		return;
	}

#pragma message ( "Shawn:Re-entrant surface hack" )
	pTestSurface = NULL;
	bGrabbedTestSurface = FALSE;

	nStateFlags = GLSTATE_PRESET1;

	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// SCG[5/18/99]: Set the world palette
	if ( qglMTexCoord2fSGIS && ( r_fullbright->value != 1 ) )
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags | GLSTATE_MULTITEXTURE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );

		// SCG[5/18/99]: Set tmu 1 texture environment
		GL_MTexEnv( GL_TEXTURE1_SGIS, GL_MODULATE );

		if( qglColorTableEXT )
		{
			GL_SetTexturePalette( NULL );
		}

		for( i = 0; i < r_fog_volume_count; i++ )
		{
			if( r_fogvolumes[i].surfaces == NULL )
				continue;

			for( pSurf = r_fogvolumes[i].surfaces; pSurf; pSurf = pSurf->texturechain )
			{
				if( pSurf->texinfo->flags & SURF_FLOWING )
					fScroll = gl_subdivide_size->value * FLOWING_SPEED;
				else
					fScroll = 0;

#pragma message ( "Shawn:Re-entrant surface hack" )
				if( pTestSurface == pSurf )
				{
					return;
				}

				if( bGrabbedTestSurface == FALSE )
				{
					pTestSurface = pSurf;
					bGrabbedTestSurface = TRUE;
				}

				if( pTestSurface == pSurf )
				{
					return;
				}

				if( bGrabbedTestSurface == FALSE )
				{
					pTestSurface = pSurf;
					bGrabbedTestSurface = TRUE;
				}

				if( pSurf->texinfo->flags == SURF_FOGPLANE )
				{
					continue;
				}

				image = R_TextureAnimation( pSurf->texinfo );

				GL_MBind( GL_TEXTURE0_SGIS, image->texnum );

				for ( map = 0; map < MAXLIGHTMAPS && pSurf->styles[map] != 255; map++ )
				{
					if ( r_newrefdef.lightstyles[pSurf->styles[map]].white != pSurf->cached_light[map] )
						goto dynamic;
				}

				if ( ( pSurf->dlightframe == r_framecount ) )
				{
			dynamic:
					if ( gl_dynamic->value )
					{
						if( !( pSurf->texinfo->flags & ( SURF_SKY | SURF_FULLBRIGHT ) )	)
						{
							is_dynamic = true;
						}
					}
				}

				if ( is_dynamic )
				{
					unsigned	temp[128*128];
					int			smax, tmax;

					if ( ( pSurf->styles[map] >= 32 || pSurf->styles[map] == 0 ) && ( pSurf->dlightframe != r_framecount ) )
					{
						smax = (pSurf->extents[0]>>4)+1;
						tmax = (pSurf->extents[1]>>4)+1;

						R_BuildLightMap( pSurf, (byte *)temp, smax*4 );
						R_SetCacheState( pSurf );

						GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + pSurf->lightmaptexturenum );

						lmtex = pSurf->lightmaptexturenum;

						qglTexSubImage2D( GL_TEXTURE_2D, 0,
										  pSurf->light_s, pSurf->light_t, 
										  smax, tmax, 
										  GL_LIGHTMAP_FORMAT, 
										  GL_UNSIGNED_BYTE, temp );

					}
					else
					{
						smax = (pSurf->extents[0]>>4)+1;
						tmax = (pSurf->extents[1]>>4)+1;

						R_BuildLightMap( pSurf, (byte *)temp, smax*4 );

						GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + 0 );

						lmtex = 0;

						qglTexSubImage2D( GL_TEXTURE_2D, 0,
										  pSurf->light_s, pSurf->light_t, 
										  smax, tmax, 
										  GL_LIGHTMAP_FORMAT, 
										  GL_UNSIGNED_BYTE, temp );

					}
				}
				else
				{
					lmtex = pSurf->lightmaptexturenum;
					GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + lmtex );
				}
				
				for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
				{
					qglBegin (GL_TRIANGLE_FAN);
					for( i = 0; i < pPoly->numverts; i++ )
					{
						qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, ( pPoly->s1[i] - fScroll ) / image->width, pPoly->t1[i] / image->height );
						qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i] );
						qglVertex3fv( pPoly->verts[i] );
					}
					qglEnd ();
				}
			}
		}
	}
	else
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags );

		if( qglColorTableEXT )
		{
			GL_SetTexturePalette( NULL );
		}

		for( i = 0; i < r_fog_volume_count; i++ )
		{
			if( r_fogvolumes[i].surfaces == NULL )
			{
				continue;
			}

			for( pSurf = r_fogvolumes[i].surfaces; pSurf; pSurf = pSurf->texturechain )
			{
#pragma message ( "Shawn:Re-entrant surface hack" )
				if( pTestSurface == pSurf )
				{
					return;
				}

				if( bGrabbedTestSurface == FALSE )
				{
					pTestSurface = pSurf;
					bGrabbedTestSurface = TRUE;
				}

				if( pSurf->texinfo->flags == SURF_FOGPLANE )
				{
					continue;
				}

				R_RenderBrushPoly( pSurf );
			}
		}
	}
	GL_TexEnv (GL_REPLACE);
}
*/
void R_DrawFog()
{
	msurface_t*	pSurf;
	glpoly_t*	pPoly;
	float		fAlpha;
    float       fParametricDist, fFogDist;
    float       fAlt1, fAlt2, fDensityAdjust;
    CVector		vT;
	int			i;
    CVector     offsetvert;
//    int         numpolysdrawn;

    //numpolysdrawn = 0;

    if (true==gInsideFog)
    {
        for( pSurf = r_fogvolumes[r_current_fog_volume].surfaces; pSurf; pSurf = pSurf->texturechain )
        {
            for( pPoly = pSurf->polys ; pPoly ; pPoly = pPoly->next )
            {
				qglBegin (GL_TRIANGLE_FAN);
                for( i = 0; i < pPoly->numverts; i++ )
                {
                    fFogDist = VectorDistance(gvCamPos, pPoly->verts[i]);
                    
                    fAlt1 = pPoly->verts[i].z;
                    fAlt2 = gvCamPos.z;
                    fDensityAdjust = R_CalcDensityAdjustment(fAlt1, fAlt2);
                    
                    fAlpha = fFogDist*fDensityAdjust / gScale;
    
					if( fAlpha > 1 )
						fAlpha = 1;

                    qglColor4f( g_vColor.x, g_vColor.y, g_vColor.z, fAlpha );
                    
                    // move vert slightly toward eye to fix z fighting
                    offsetvert = pPoly->verts[i];
                    offsetvert += 0.001f * (gvCamPos-pPoly->verts[i]);
                    
                    qglVertex3fv( offsetvert);
                }
                qglEnd ();
//                numpolysdrawn++;
            }
    	}

    }
    else // outside fog
    {
        for( pSurf = r_fogvolumes[r_current_fog_volume].surfaces; pSurf; pSurf = pSurf->texturechain )
        {
            if (R_SurfIsBackfaced(pSurf, gvCamPos))
                continue;
    
            for( pPoly = pSurf->polys ; pPoly ; pPoly = pPoly->next )
            {
				qglBegin (GL_TRIANGLE_FAN);
                for( i = 0; i < pPoly->numverts; i++ )
                {
                    fParametricDist = R_FogTraceLine(pPoly->verts[i], 0!=(pSurf->texinfo->flags&SURF_FOGPLANE) );
                    fFogDist = fParametricDist * VectorDistance(gvCamPos, pPoly->verts[i]);
                    
                    fAlt1 = pPoly->verts[i].z;
                    fAlt2 = fAlt1 + fParametricDist*(gvCamPos.z-fAlt1);
                    fDensityAdjust = R_CalcDensityAdjustment(fAlt1, fAlt2);
                    
                    fAlpha = fFogDist*fDensityAdjust / gScale;
    
					if( fAlpha > 1 )
						fAlpha = 1;

                    qglColor4f( g_vColor.x, g_vColor.y, g_vColor.z, fAlpha );
                    
                    // move vert slightly toward eye to fix z fighting
                    offsetvert = pPoly->verts[i];
                    offsetvert += 0.001f * (gvCamPos-pPoly->verts[i]);
                    
                    qglVertex3fv( offsetvert);
                }
                qglEnd ();
//                numpolysdrawn++;
            }
    	}
    }
}

inline float R_CalcDensityAdjustment(float fZInFog, float fZMaybeInFog)
{
    if (fZMaybeInFog>gfMaxFogAlt)
        fZMaybeInFog=gfMaxFogAlt;
    
    return (gfMaxFogAlt-0.75f*min(fZInFog, fZMaybeInFog)-0.25f*max(fZInFog, fZMaybeInFog))*gfInvFogDepth;
}

// Ash 
float R_FogTraceLine(CVector& vTargetP, bool fFogPlane)
{
    float fMinDist, fCurDist, fNum, fDenom;
    msurface_t* pTestSurf;
    float epsilon = 0.01f;
    bool fIntersected;
    int surfNum;
    float dot;

    if (0)//fFogPlane)
    {
        // This code is actually correct, but low poly tessellation results in really ugly
        // gouraud artifacts, so we only use it for translucent fogsurfaces, where it's always correct
        
        // if camera and target are outside or on any fog plane, then the ray is outside, 
        // so early out with 0
    	for( surfNum = 0; surfNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfNum++ )
        {
    		pTestSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfNum );
            
            // is camera outside or on test surface?
            // camera distance to test plane is stored in texturemins since it's not used for fogsurfaces
            dot = *(float *)pTestSurf->texturemins;
    
            if (pTestSurf->flags & SURF_PLANEBACK)
                dot = -dot;
            
            if (dot < BACKFACE_EPSILON)
            {
                // is target outside or on test surface?
                dot = vTargetP.DotProduct(pTestSurf->plane->normal) - pTestSurf->plane->dist;
        
                if (pTestSurf->flags & SURF_PLANEBACK)
                    dot = -dot;
                
                if (dot < BACKFACE_EPSILON)
                {
                    return 0;
                }
            }
        }
    }
    else
    {
        // if point is at top of fog and we're above target, early out with 0
        if ((vTargetP.z==gfMaxFogAlt)&&(gvCamPos.z>=vTargetP.z))
            return 0.0f;
        // if point is at bottom of fog and we're below target, early out in some cases
        else if ((vTargetP.z==gfMinFogAlt)&&(gvCamPos.z<=vTargetP.z))
            return 0.0f;
    }
    
    fIntersected = false;
    fMinDist = 1.0f;
    surfNum = 0;
	for( surfNum = 0; surfNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfNum++ )
    {
		pTestSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfNum );

        CVector& vTestN = pTestSurf->plane->normal;     // test surface normal
        float    vTestDist = pTestSurf->plane->dist;
        
        // if camera is on test plane register an intersection
        // camera distance to test plane is stored in texturemins since it's not used for fogsurfaces
        if (fabs(*(float *)pTestSurf->texturemins)<epsilon)
        {
            fIntersected = true;
            continue;
        }

        fDenom = (gvCamPos-vTargetP).DotProduct(vTestN);

        // no intersection if camera and target vert are in plane parallel to test plane
        if (fabs(fDenom)<epsilon)
            continue;
        
        fNum = vTestDist-vTargetP.DotProduct(vTestN);

        // no intersection if target vert is on test plane
        if (fabs(fNum)<epsilon)
            continue;

        // no meaningful intersection unless camera and target are on opposite sides of test plane
        // this happens when fNum and fDenom have the same sign and abs(fDenom)>=abs(fNum)
        if (fNum>0&&fDenom<fNum)
            continue;
        else if (fNum<0&&fDenom>fNum)
            continue;
        
        // calc dist of seg vTargetP to testface along vTargetP-gvCamPos
        fCurDist = fNum/fDenom;

        if (fCurDist<fMinDist&&fCurDist>epsilon)
        {
            fMinDist = fCurDist;
            fIntersected = true;
        }
	}

    if (fIntersected)
		return fMinDist;
    else
        return 0;
}                                 



void R_DrawOutlines()
{
	msurface_t*	pSurf;
	glpoly_t*	pPoly;
	int			i;
    int         surfaceNum;

	if(0==gl_showtris->value)
		return;

	g_vColor = r_fogvolumes[r_current_fog_volume].vColor;

	qglColor4f( g_vColor.x, g_vColor.y, g_vColor.z, 1.0f );

	if (2==gl_showtris->value)
	{
		for( surfaceNum = 0; surfaceNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfaceNum++ )
		{
			pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfaceNum );

			// draw just the convex hull?
            if (R_SurfIsBackfaced(pSurf, gvCamPos))
				continue;

			for( pPoly = pSurf->polys ; pPoly ; pPoly = pPoly->next )
			{
				for( i = 2; i < pPoly->numverts; i++ )
				{
					qglBegin (GL_LINE_STRIP);
					qglVertex3fv( pPoly->verts[0] );
					qglVertex3fv( pPoly->verts[i-1] );
					qglVertex3fv( pPoly->verts[i] );
					qglVertex3fv( pPoly->verts[0] );
					qglEnd ();
				}
			}
		}
	}
	else
	{
		for( pSurf = r_fogvolumes[r_current_fog_volume].surfaces; pSurf; pSurf = pSurf->texturechain)
		{
            if (R_SurfIsBackfaced(pSurf, gvCamPos))
				continue;

			for( pPoly = pSurf->polys ; pPoly ; pPoly = pPoly->next )
			{
				for( i = 2; i < pPoly->numverts; i++ )
				{
					qglBegin (GL_LINE_STRIP);
					qglVertex3fv( pPoly->verts[0] );
					qglVertex3fv( pPoly->verts[i-1] );
					qglVertex3fv( pPoly->verts[i] );
					qglVertex3fv( pPoly->verts[0] );
					qglEnd ();
				}
			}
            
		}
	}

	qglLineWidth( 1 );
}

// this returns true if pTestVert is in front of or on pSurf
inline bool R_TestSurfaceToVert(msurface_t *pSurf, CVector *pTestVert)
{
    bool  fInFrontOf;

    if (R_SurfIsBackfaced(pSurf, *pTestVert))
        fInFrontOf = false;
    else
        fInFrontOf = true;
    
    return fInFrontOf;
}

// this returns true if all the verts of pTestPoly are in front of or on pSurf
inline bool R_TestSurfaceToPoly(msurface_t *pSurf, glpoly_t *pTestPoly)
{
    int     i;
    bool    fInFrontOf;

    i=0;
    fInFrontOf = true;

    while ((true==fInFrontOf)&&(i<pTestPoly->numverts))
    {
        fInFrontOf = R_TestSurfaceToVert(pSurf, pTestPoly->verts+i);

        i++;
    }

    return fInFrontOf;
}

// this returns true if all the polys of pTestSurf are in front of or on pSurf
inline bool R_TestSurfaceToSurface(msurface_t *pSurf, msurface_t *pTestSurf)
{
    glpoly_t    *pTestPoly;
    bool        fInFrontOf;

    pTestPoly = pTestSurf->polys;
    fInFrontOf = true;

    while ((true==fInFrontOf)&&pTestPoly)
    {
        fInFrontOf = R_TestSurfaceToPoly(pSurf, pTestPoly);

        pTestPoly = pTestPoly->next;
    }

    return fInFrontOf;
}


void R_BuildConvexHull()
{
    msurface_t  *pSurf, *pTestSurf;
    bool    fInConvexHull;
    int     surfaceNum, testSurfaceNum;
    int     numHullSurfaces;

	numHullSurfaces = 0;
    // Ash - remove nonconvex faces leaving convex hull (unoptimized)
    surfaceNum = 0;
    while (surfaceNum<r_fogvolumes[r_current_fog_volume].hull.GetSize())
    {
        pSurf = (msurface_t *)r_fogvolumes[r_current_fog_volume].hull.GetAt(surfaceNum);

        testSurfaceNum = 0;
        fInConvexHull = true;
        while ((true==fInConvexHull)&&(testSurfaceNum<r_fogvolumes[r_current_fog_volume].hull.GetSize()))
        {
            pTestSurf = (msurface_t *)r_fogvolumes[r_current_fog_volume].hull.GetAt(testSurfaceNum);

            if (pTestSurf!=pSurf)
                fInConvexHull = R_TestSurfaceToSurface(pSurf, pTestSurf);
            
            testSurfaceNum++;
        }

        if (fInConvexHull)
        {
            // keep it
            numHullSurfaces++;
            surfaceNum++;
        }
        else
        {
            // remove it
			r_fogvolumes[r_current_fog_volume].hull.RemoveAt(surfaceNum);
        }
    }

    // Ash - optimize by removing coplanar faces
    surfaceNum = 0;
	while(surfaceNum<r_fogvolumes[r_current_fog_volume].hull.GetSize())
	{
		pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfaceNum );

        testSurfaceNum=surfaceNum+1;
        while(testSurfaceNum<r_fogvolumes[r_current_fog_volume].hull.GetSize())
        {
            pTestSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( testSurfaceNum );
            CVector *pTestPoint = pTestSurf->polys->verts;

            if(( pTestSurf->plane==pSurf->plane)&&((pTestSurf->flags&SURF_PLANEBACK)==(pSurf->flags&SURF_PLANEBACK)))
            {
                // remove it
                r_fogvolumes[r_current_fog_volume].hull.RemoveAt(testSurfaceNum);
                numHullSurfaces--;
            }
            else
            {
                // keep it
                testSurfaceNum++;
            }
        }
        surfaceNum++;
    }
}

void R_FogCalcMinsMaxs( CVector& vMins, CVector& vMaxs)
{
	msurface_t	*pSurf;
	glpoly_t	*pPoly;
	int			i, surfaceNum;

	vMins.x = vMins.y = vMins.z = 9999;
	vMaxs.x = vMaxs.y = vMaxs.z = -9999;

	for( surfaceNum = 0; surfaceNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfaceNum++ )
	{
		pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfaceNum );

		for( pPoly = pSurf->polys ; pPoly ; pPoly = pPoly->next )
		{
			for ( i = 0 ; i < pPoly->numverts ; i++ )
			{
				if( pPoly->verts[i].x < vMins.x )
					vMins.x = pPoly->verts[i].x;
				if( pPoly->verts[i].x > vMaxs.x )
					vMaxs.x = pPoly->verts[i].x;
				
   				if( pPoly->verts[i].y < vMins.y )
					vMins.y = pPoly->verts[i].y;
				if( pPoly->verts[i].y > vMaxs.y )
					vMaxs.y = pPoly->verts[i].y;
                
                if( pPoly->verts[i].z < vMins.z )
					vMins.z = pPoly->verts[i].z;
				if( pPoly->verts[i].z > vMaxs.z )
					vMaxs.z = pPoly->verts[i].z;
			}
		}
	}
}

void R_FogSetColor()
{
	msurface_t	*pSurf;
	int			surfaceNum;

	for( surfaceNum = 0; surfaceNum < r_fogvolumes[r_current_fog_volume].hull.GetSize(); surfaceNum++ )
	{
		pSurf = ( msurface_t * ) r_fogvolumes[r_current_fog_volume].hull.GetAt( surfaceNum );

		if( pSurf->texinfo->flags & SURF_FOGPLANE )
		{
			r_fogvolumes[r_current_fog_volume].vColor.x = ( float )( ( pSurf->texinfo->color >> 8 ) & 0xf8 ) / 255.0;
			r_fogvolumes[r_current_fog_volume].vColor.y = ( float )( ( pSurf->texinfo->color >> 3 ) & 0xfc ) / 255.0;
			r_fogvolumes[r_current_fog_volume].vColor.z = ( float )( ( pSurf->texinfo->color << 3 ) & 0xf8 ) / 255.0;

			r_fogvolumes[r_current_fog_volume].distance = pSurf->texinfo->value;
			if( r_fogvolumes[r_current_fog_volume].distance == 0 )
			{
				r_fogvolumes[r_current_fog_volume].distance = 128;
			}
		}
	}
}

void R_FogInit()
{
	for( int i = 0; i < r_fog_volume_count; i++ )
	{
		r_current_fog_volume = i;

		R_FogCalcMinsMaxs( r_fogvolumes[i].vMins, r_fogvolumes[i].vMaxs );
		R_BuildConvexHull();
		R_FogSetColor();

		r_fogvolumes[i].surfaces = NULL;
	}
}

// Ash -- this returns the first visible fog volume to intersect the given axially 
// aligned bounding box
inline int R_GetFogVolumeForBBox(CVector& mins, CVector& maxs)
{
    int i, found_volume = -1;
    
    i=0;
    while (i<r_fog_volume_count&&-1==found_volume)
    {
        CVector& vMins = r_fogvolumes[i].vMins;
        CVector& vMaxs = r_fogvolumes[i].vMaxs;
        
        if ((mins.x<vMaxs.x)&&(maxs.x>vMins.x))
        {
            if ((mins.y<vMaxs.y)&&(maxs.y>vMins.y))
            {
                if ((mins.z<vMaxs.z)&&(maxs.z>vMins.z))
                {
                   found_volume = i;
                }
            }
        }

        i++;
    }

    return found_volume;
}

// Ash
// Looks for a visible fog region that intersects the arbitrarily aligned
// bbox.  If it finds one, it provides nFogSamples of the fog.  The samples 
// are taken over the z range of the bbox that is in the fog -- no sense
// in wasting samples on points outside the fog.
bool GL_FogBBox(CVector* bbox, int bboxSize, fogsample_t& fogSample, int nFogSamples)
{
    CVector mins, maxs;
	int i, found_volume;
    CVector samplePoint;
    float fAlt1, fAlt2;
    float fParametricDist, fFogDist, fDensityAdjust;
    float minSampleZ, maxSampleZ;

    // get an axially aligned bbox
    mins = bbox[0];
	maxs = bbox[0];
    for (i=1;i<bboxSize;i++)
    {
        if (bbox[i].x<mins.x)
            mins.x=bbox[i].x;
        else if (bbox[i].x>maxs.x)
            maxs.x=bbox[i].x;
        
        if (bbox[i].y<mins.y)
            mins.y=bbox[i].y;
        else if (bbox[i].y>maxs.y)
            maxs.y=bbox[i].y;
        
        if (bbox[i].z<mins.z)
            mins.z=bbox[i].z;
        else if (bbox[i].z>maxs.z)
            maxs.z=bbox[i].z;
    }

    found_volume = R_GetFogVolumeForBBox(mins, maxs);
    fogSample.fogVolume = found_volume;

    if (found_volume>=0)
    {
        // set this as the current fog volume
        r_current_fog_volume = found_volume;
        fogSample.fogVolume = found_volume;

        R_FogPrecalc();
        
        // set the fog color
        fogSample.fog_r = g_vColor.x;
        fogSample.fog_g = g_vColor.y;
        fogSample.fog_b = g_vColor.z;
        

        // set the number of samples we're providing
        // only give as many samples as there is space for
        nFogSamples = min(nFogSamples, R_MAX_FOG_SAMPLES);
        fogSample.nFogSamples = nFogSamples;

        if (nFogSamples)
        {
            // detect a partial fog intersection
            if (mins.z<gfMaxFogAlt && maxs.z>gfMaxFogAlt)
                fogSample.fPartial = true;
            else
                fogSample.fPartial = false;

            // get the z range of the samples
            minSampleZ = max(mins.z, gfMinFogAlt);
            maxSampleZ = min(maxs.z, gfMaxFogAlt);
    
            fogSample.deltaZ = (maxSampleZ - minSampleZ)/(nFogSamples-1);
            fogSample.invDeltaZ = (nFogSamples-1)/(maxSampleZ - minSampleZ);
            
            // init for the samples
            samplePoint.x = 0.5f*(mins.x+maxs.x);
            samplePoint.y = 0.5f*(mins.y+maxs.y);
            samplePoint.z = minSampleZ;
    
            if (true==gInsideFog)
            {
                fogSample.fViewerInFog = true;

                // take fog samples over the z range of the model
                for (i=0;i<nFogSamples;i++)
                {
                    fFogDist = VectorDistance(gvCamPos, samplePoint);
                    
                    fAlt1 = samplePoint.z;
                    fAlt2 = gvCamPos.z;
                    fDensityAdjust = R_CalcDensityAdjustment(fAlt1, fAlt2);
                    
                    fogSample.fogSamples[i].z = samplePoint.z;
                    fogSample.fogSamples[i].alpha = fFogDist*fDensityAdjust / gScale;
        
    				if( fogSample.fogSamples[i].alpha > 1 )
    					fogSample.fogSamples[i].alpha = 1;
    
                    samplePoint.z += fogSample.deltaZ;
                }
            }
            else
            {
                fogSample.fViewerInFog = false;

                // take fog samples over the z range of the model
                for (i=0;i<nFogSamples;i++)
                {
                    fParametricDist = R_FogTraceLine(samplePoint, false);
                    fFogDist = fParametricDist * VectorDistance(gvCamPos, samplePoint);
                    
                    fAlt1 = samplePoint.z;
                    fAlt2 = gvCamPos.z;
                    fDensityAdjust = R_CalcDensityAdjustment(fAlt1, fAlt2);
                    
                    fogSample.fogSamples[i].z = samplePoint.z;
                    fogSample.fogSamples[i].alpha = fFogDist*fDensityAdjust / gScale;
        
    				if( fogSample.fogSamples[i].alpha > 1 )
    					fogSample.fogSamples[i].alpha = 1;
    
                    samplePoint.z += fogSample.deltaZ;
                }
            }
        }
        else
        {
            fogSample.deltaZ = 0.0f;
            fogSample.invDeltaZ = 0.0f;
        }
        
        return true;
    }
    else
        return false;
}


