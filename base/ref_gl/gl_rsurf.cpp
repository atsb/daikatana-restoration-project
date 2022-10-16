// GL_RSURF.C: surface-related refresh code
#include	<assert.h>

#include	"gl_local.h"

#include "dk_point.h"
#include "dk_array.h"

//#include	"gl_protex.h"

static CVector	modelorg;		// relative to viewpoint

msurface_t	*r_surfs_alpha;
msurface_t	*r_surfs_mid;
msurface_t	*r_surfs_fullbright;

#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	128

#define ONE_DIV_FOUR	0.25	// (1.0f/4.0f)

float	r_turbsin[] =
{
	#include "warpsin.h"
};
float	fWarpTime;

#define TURBSCALE (256.0 / (2 * M_PI))

#define ONE_DIV_SIXTYFOUR	 0.015625
//#define FLOWING_SPEED	( ( r_newrefdef.time * 0.5 ) - ( int )( r_newrefdef.time * 0.5) )
#define FLOWING_SPEED	( ( fWarpTime * 0.5 ) - ( int )( fWarpTime * 0.5) )

int		c_visible_lightmaps;
int		c_visible_lightmap_bytes;
byte	c_lightmap_list[MAX_LIGHTMAPS];	

int		c_visible_textures;
int		c_visible_texture_bytes;
byte	c_texture_list[MAX_GLTEXTURES];

#define GL_LIGHTMAP_FORMAT GL_RGBA

typedef struct
{
	int internal_format;
	int	current_lightmap_texture;
	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];

	int			allocated[BLOCK_WIDTH];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte		lightmap_buffer[4*BLOCK_WIDTH*BLOCK_HEIGHT];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;

static void		LM_InitBlock( void );
static void		LM_UploadBlock( qboolean dynamic );
static qboolean	LM_AllocBlock (int w, int h, int *x, int *y);

void R_SetCacheState( msurface_t *pSurf );
void R_BuildLightMap (msurface_t *pSurf, byte *dest, int stride);

static void GL_RenderLightmappedPoly( msurface_t *pSurf );

void DrawGLPolyChain( msurface_t *pSurf, float soffset, float toffset );
void DrawSubdividedLightmaps( msurface_t *pSurf, float soffset, float toffset );
int R_TranslateAlphaSurface (entity_t *pent);

entity_t r_global_ent;	// SCG[1/23/00]: Scratch entity for misc usage (see R_DrawFullbrightSurfaces, etc )
/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation (mtexinfo_t *tex)
{
	int		c;

	if( tex == NULL )
		return NULL;

	if (!tex->next)
		return tex->image;

	c = currententity->frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->image;
}

#if 0
/*
=================
WaterWarpPolyVerts

Mangles the x and y coordinates in a copy of the poly
so that any drawing routine can be water warped
=================
*/
glpoly_t *WaterWarpPolyVerts (glpoly_t *p)
{
	int		i;
	static byte	buffer[1024];
	glpoly_t *out;

	out = ri.X_Malloc(sizeof(glpoly_t),MEM_TAG_MODEL);

	out->numverts = p->numverts;

	for (i=0 ; i<p->numverts ; i++)
	{
		out->verts[i].x = p->verts[i].x + 4*sin(v[1]*0.05+r_newrefdef.time)*sin(v[2]*0.05+r_newrefdef.time)
		out->verts[i].y = p->verts[i].y + 4*sin(v[1]*0.05+r_newrefdef.time)*sin(v[2]*0.05+r_newrefdef.time)

		out->verts[i].z = p->verts[i].z;
		out->s1[i] = p->s1[i];
		out->t1[i] = p->t1[i];
		out->s2[i] = p->s2[i];
		out->t2[i] = p->t2[i];
	}

	return out;
}

/*
================
DrawGLWaterPoly

Warp the vertex coordinates
================
*/
void DrawGLWaterPoly (glpoly_t *p)
{
	int		i;

	p = WaterWarpPolyVerts (p);
	qglBegin (GL_TRIANGLE_FAN);
	for (i=0 ; i<p->numverts ; i++)
	{
		qglTexCoord2f (p->s1[i], p->t1[i]);
		qglVertex3fv (p->verts[i]);
	}
	qglEnd ();
}
void DrawGLWaterPolyLightmap (glpoly_t *p)
{
	int		i;

	p = WaterWarpPolyVerts (p);
	qglBegin (GL_TRIANGLE_FAN);
	for (i=0 ; i<p->numverts ; i++)
	{
		qglTexCoord2f (p->s1[i], p->t1[i]);
		qglVertex3fv (p->verts[i]);
	}
	qglEnd ();
}
#endif

/*
================
DrawFlatPoly
// SCG[4/22/99]: Flat shaded polys
================
*/
void DrawFlatPoly( msurface_t *pSurf )
{
	glpoly_t	*pPoly;
	float		r, g, b;
	int			i;

	r = ( ( int ) pSurf & 0xff ) / 255.0;
	g = ( ( int ) pSurf->polys & 0xff ) / 255.0;
	b = ( ( int ) pSurf->plane & 0xff ) / 255.0;

	GL_SetState( GLSTATE_PRESET1 & ~GLSTATE_TEXTURE_2D );

	qglColor4f( r, g, b, 1.0 );

	for ( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
	{
		qglBegin (GL_POLYGON);
		for( i = 0 ; i < pPoly->numverts; i++ )
		{
			qglVertex3fv( pPoly->verts[i] );
		}
		qglEnd();
	}
}

/*
================
DrawSubdividedPolys
// SCG[4/22/99]: Subdivided polygons
================
*/
void DrawSubdividedPolys( msurface_t *pSurf )
{
	int			i;
	float		fScroll;
	glpoly_t	*pPoly;
	CVector		vVertex;
	int			nWidth = pSurf->texinfo->image->width;
	int			nHeight = pSurf->texinfo->image->height;
	float		fOrigS, fOrigT, fWarpS, fWarpT, fS, fT, zWave;
	float		fWaveModifier = 0;
	int			fWaveMultiplier = 0;
	qboolean	bSurge = FALSE;
	GLenum		nPrimType;

	if( pSurf->texinfo->flags & SURF_FOGPLANE )
	{
		nPrimType = GL_POLYGON;
	}
	else
	{
		nPrimType = GL_TRIANGLE_FAN;
	}

	if( pSurf->texinfo->flags & SURF_FLOWING )
		fScroll = 64 * FLOWING_SPEED;
	else
		fScroll = 0;

	if( pSurf->flags & SURF_DRAWTURB )
	{
		if( pSurf->texinfo->flags & ( SURF_SURGE | SURF_BIGSURGE ) )
		{
			bSurge = TRUE;

			if( pSurf->texinfo->flags & SURF_SURGE )
			{
				fWaveModifier = 0.250;		// small surge
				fWaveMultiplier = 1;
			}
			else
			{
				fWaveModifier = 0.500;		// big surge
				fWaveMultiplier = 4;
			}
		}

		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
		{
			c_brush_polys++;
			qglBegin( nPrimType );
			for( i = 0; i < pPoly->numverts ; i++ )
			{
				vVertex = pPoly->verts[i];

				fOrigS = pPoly->s1[i];
				fOrigT = pPoly->t1[i];
	#if !id386
				fWarpS = r_turbsin[( int ) ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
				fWarpT = r_turbsin[( int ) ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
	#else
				fWarpS = r_turbsin[Q_ftol( ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
				fWarpT = r_turbsin[Q_ftol( ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
	#endif
				fS = fOrigS + fWarpS;
				fS -= fScroll;
				fS /= nWidth;

				fT = fOrigT + fWarpT;
				fT /= nHeight;

				if( bSurge )
				{
					zWave = r_turbsin[Q_ftol( ( ( ( fS + fT ) * fWaveModifier + fWarpTime ) * TURBSCALE ) ) & 255];
					vVertex.z += zWave * fWaveMultiplier;
				}

				qglTexCoord2f( fS, fT );
				qglVertex3fv( vVertex );
			}
			qglEnd();
		}
	}
	else
	{
		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
		{
			c_brush_polys++;
			qglBegin( nPrimType );
			for( i = 0; i < pPoly->numverts ; i++ )
			{
				qglTexCoord2f( ( pPoly->s1[i] - fScroll ) / nWidth, pPoly->t1[i] / nHeight );
				qglVertex3fv( pPoly->verts[i] );
			}
			qglEnd();
		}
	}

}

/*
================
DrawSubdividedLightmaps
// SCG[4/22/99]: Lightmaps for subdivided polygons
================
*/
void DrawSubdividedLightmaps( msurface_t *pSurf, float soffset, float toffset )
{
	int			i;
	float		fScroll;
	glpoly_t	*pPoly;
	CVector		vVertex;
	int			nWidth = pSurf->texinfo->image->width;
	int			nHeight = pSurf->texinfo->image->height;
	float		fOrigS, fOrigT, fWarpS, fWarpT, fS, fT, zWave;
	float		fWaveModifier = 0;
	int			fWaveMultiplier = 0;
	qboolean	bSurge = FALSE;
	GLenum		nPrimType;

	if( pSurf->texinfo->flags & SURF_FOGPLANE )
	{
		nPrimType = GL_QUADS;
	}
	else
	{
		nPrimType = GL_TRIANGLE_FAN;
	}

	if( pSurf->texinfo->flags & SURF_FLOWING )
		fScroll = gl_subdivide_size->value * FLOWING_SPEED;
	else
		fScroll = 0;

	if( pSurf->texinfo->flags & ( SURF_SURGE | SURF_BIGSURGE ) )
	{
		bSurge = TRUE;

		if( pSurf->texinfo->flags & SURF_SURGE )
		{
			fWaveModifier = 0.250;		// small surge
			fWaveMultiplier = 1;
		}
		else
		{
			fWaveModifier = 0.500;		// big surge
			fWaveMultiplier = 4;
		}
	}

	if( soffset == 0 && toffset == 0 )
	{
		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
		{	
			c_lightmap_polys++;
			qglBegin( nPrimType );
			for( i = 0; i < pPoly->numverts ; i++ )
			{
				vVertex = pPoly->verts[i];
				if( bSurge )
				{
					fOrigS = pPoly->s1[i];
					fOrigT = pPoly->t1[i];
#if !id386
					fWarpS = r_turbsin[( int ) ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
					fWarpT = r_turbsin[( int ) ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
#else
					fWarpS = r_turbsin[Q_ftol( ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
					fWarpT = r_turbsin[Q_ftol( ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
#endif
					fS = fOrigS + fWarpS;
					fS -= fScroll;
					fS /= nWidth;

					fT = fOrigT + fWarpT;
					fT /= nHeight;

					zWave = r_turbsin[Q_ftol( ( ( ( fS + fT ) * fWaveModifier + fWarpTime ) * TURBSCALE ) ) & 255];
					vVertex.z += zWave * fWaveMultiplier;
				}

				qglTexCoord2f( pPoly->s2[i], pPoly->t2[i] );
				qglVertex3fv( vVertex );
			}
			qglEnd();
		}
	}
	else
	{
		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
		{
			c_lightmap_polys++;
			qglBegin( nPrimType );
			for( i = 0; i < pPoly->numverts ; i++ )
			{
				vVertex = pPoly->verts[i];
				if( bSurge )
				{
					fOrigS = pPoly->s1[i];
					fOrigT = pPoly->t1[i];
#if !id386
					fWarpS = r_turbsin[( int ) ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
					fWarpT = r_turbsin[( int ) ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) & 255];
#else
					fWarpS = r_turbsin[Q_ftol( ( ( fOrigT * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
					fWarpT = r_turbsin[Q_ftol( ( ( fOrigS * 0.125 + fWarpTime ) * TURBSCALE ) ) & 255];
#endif
					fS = fOrigS + fWarpS;
					fS -= fScroll;
					fS /= nWidth;

					fT = fOrigT + fWarpT;
					fT /= nHeight;

					zWave = r_turbsin[Q_ftol( ( ( ( fS + fT ) * fWaveModifier + fWarpTime ) * TURBSCALE ) ) & 255];
					vVertex.z += zWave * fWaveMultiplier;
				}

				qglTexCoord2f( pPoly->s2[i] - soffset, pPoly->t2[i]  - toffset );
				qglVertex3fv( vVertex );
			}
			qglEnd();
		}
	}
}

/*
================
DrawGLPoly
================
*/
void DrawGLPoly( msurface_t *pSurf )
{
	int			i;
	float		fScroll;
	glpoly_t	*pPoly;

	if( pSurf->texinfo->flags & SURF_FLOWING )
		fScroll = FLOWING_SPEED;
	else
		fScroll = 0;

	for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
	{
		c_brush_polys++;

		qglBegin (GL_POLYGON);
		for( i = 0; i < pPoly->numverts ; i++ )
		{
			qglTexCoord2f( ( pPoly->s1[i] - fScroll ), pPoly->t1[i] );
			qglVertex3fv( pPoly->verts[i] );
		}
		qglEnd();
	}
}

/*
================
DrawGLMPoly
================
*/

void DrawGLMPoly( msurface_t *pSurf )
{
	int		i;
	float		fScroll;
	glpoly_t *pPoly = pSurf->polys;

	if (pSurf->texinfo->flags & SURF_FLOWING)
		fScroll = FLOWING_SPEED;
	else
		fScroll = 0;

	c_brush_polys++;

	if( pSurf->texinfo->flags & SURF_FLOWING )
	{
		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
		{
			qglBegin( GL_POLYGON );
			for( i = 0; i < pPoly->numverts; i++ )
			{
				qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i] - fScroll, pPoly->t1[i]);
				qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
				qglVertex3fv( pPoly->verts[i] );
			}
			qglEnd();
		}
	}
	else
	{
		for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
		{
			qglBegin( GL_POLYGON );
			for( i = 0; i < pPoly->numverts; i++ )
			{
				qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i], pPoly->t1[i]);
				qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
				qglVertex3fv( pPoly->verts[i] );
			}
			qglEnd();
		}
	}
}

/*
** R_DrawTriangleOutlines
*/
void R_DrawTriangleOutlines (void)
{
	int			i, j;
	glpoly_t	*p;

	qglPointSize( 50.0 );   
	if( 1 == gl_showtris->value )  // 0 is off, 2 shows the fog volume convex hulls
    {
    	qglColor4f (1,1,1,1);
    
    	for( i = 0; i < MAX_LIGHTMAPS; i++ )
    	{
    		msurface_t *pSurf;
    
    		for( pSurf = gl_lms.lightmap_surfaces[i]; pSurf != 0; pSurf = pSurf->lightmapchain )
    		{
    			p = pSurf->polys;
    			for( ; p ; p = p->chain )
    			{
    				for( j = 2; j < p->numverts ; j++)
    				{
				    	qglColor4f( 1,1,1,1 );
    					qglBegin( GL_LINE_STRIP );
    					qglVertex3fv( p->verts[0] );
    					qglVertex3fv( p->verts[j-1] );
    					qglVertex3fv( p->verts[j] );
    					qglVertex3fv( p->verts[0] );
    					qglEnd();
    				}
    			}
    		}
    	}
    }
}

/*
** DrawGLPolyChain
*/
void DrawGLPolyChain( msurface_t *pSurf, float soffset, float toffset )
{
	glpoly_t	*pPoly, *pChainPoly;
	int			j;

	if( soffset == 0 && toffset == 0 )
	{
		for( pChainPoly = pSurf->polys; pChainPoly; pChainPoly = pChainPoly->chain )
		{
			for( pPoly = pChainPoly; pPoly; pPoly = pPoly->chain )
			{
				c_lightmap_polys++;
				qglBegin( GL_POLYGON );
				for( j = 0; j < pPoly->numverts; j++ )
				{
					qglTexCoord2f( pPoly->s2[j], pPoly->t2[j] );
					qglVertex3fv( pPoly->verts[j] );
				}
				qglEnd();
			}
		}
	}
	else
	{
		for( pChainPoly = pSurf->polys; pChainPoly; pChainPoly = pChainPoly->chain )
		{
			for( pPoly = pChainPoly; pPoly; pPoly = pPoly->chain )
			{
				c_lightmap_polys++;
				qglBegin( GL_POLYGON );
				for( j = 0; j < pPoly->numverts; j++ )
				{
					qglTexCoord2f( pPoly->s2[j] - soffset, pPoly->t2[j] - toffset );
					qglVertex3fv( pPoly->verts[j] );
				}
				qglEnd();
			}
		}
	}
}

/*
** R_BlendLightMaps
**
** This routine takes all the given light mapped surfaces in the world and
** blends them into the framebuffer.
*/
void R_BlendLightmaps( qboolean bReTransform )
{
	int				i;
	msurface_t		*pSurf, *newdrawsurf = 0;
	unsigned int	nStateFlags;
	int				matrix_modified = 0;

	// don't bother if we're set to fullbright
	if (r_fullbright->value)
		return;

	if (!r_worldmodel->lightdata)
		return;

	c_lightmap_polys++;

	nStateFlags = GLSTATE_PRESET1;
	nStateFlags &= ~GLSTATE_DEPTH_MASK;

	/*
	** set the appropriate blending mode unless we're only looking at the
	** lightmaps.
	*/
	if (!gl_lightmap->value)
	{
		nStateFlags |= GLSTATE_BLEND;

		if ( gl_saturatelighting->value )
		{
			qglBlendFunc( GL_ONE, GL_ONE );
		}
		else
		{
			if ( gl_monolightmap->string[0] != '0' )
			{
				switch ( toupper( gl_monolightmap->string[0] ) )
				{
				case 'I':
					qglBlendFunc (GL_ZERO, GL_SRC_COLOR );
					break;
				case 'L':
					qglBlendFunc (GL_ZERO, GL_SRC_COLOR );
					break;
				case 'A':
				default:
					qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
					break;
				}
			}
			else
			{
				qglBlendFunc (GL_ZERO, GL_SRC_COLOR );
			}
		}
	}

	// SCG[5/18/99]: Set the state
	GL_SetState( nStateFlags );

	/*
	** render static lightmaps first
	*/
	if( bReTransform == TRUE )
	{
		for( i = 1; i < MAX_LIGHTMAPS; i++ )
		{
			if( gl_lms.lightmap_surfaces[i] )
			{
				if (currentmodel == r_worldmodel)
				{
					c_visible_lightmaps++;
					c_visible_lightmap_bytes += BLOCK_WIDTH * BLOCK_HEIGHT * 4;
				}

				GL_Bind( gl_state.lightmap_textures + i);

				for( pSurf = gl_lms.lightmap_surfaces[i]; pSurf != 0; pSurf = pSurf->lightmapchain )
				{
					if ( pSurf->polys )
					{
						matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

						if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
						{
							DrawSubdividedLightmaps( pSurf, 0, 0 );
						}
						else
						{
							DrawGLPolyChain( pSurf, 0, 0 );
						}

						if ( matrix_modified )
						{
							qglPopMatrix();
						}
					}
				}
			}
		}
	}
	else
	{
		for( i = 1; i < MAX_LIGHTMAPS; i++ )
		{
			if( gl_lms.lightmap_surfaces[i] )
			{
				if (currentmodel == r_worldmodel)
				{
					c_visible_lightmaps++;
					c_visible_lightmap_bytes += BLOCK_WIDTH * BLOCK_HEIGHT * 4;
				}

				GL_Bind( gl_state.lightmap_textures + i);

				for( pSurf = gl_lms.lightmap_surfaces[i]; pSurf != 0; pSurf = pSurf->lightmapchain )
				{
					if ( pSurf->polys )
					{
						if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
						{
							DrawSubdividedLightmaps( pSurf, 0, 0 );
						}
						else
						{
							DrawGLPolyChain( pSurf, 0, 0 );
						}
					}
				}
			}
		}
	}

	/*
	** render dynamic lightmaps
	*/
	if( gl_dynamic->value )
	{
		LM_InitBlock();

		GL_Bind( gl_state.lightmap_textures+0 );

		if( currentmodel == r_worldmodel )
		{
			c_visible_lightmaps++;
			c_visible_lightmap_bytes += BLOCK_WIDTH * BLOCK_HEIGHT * 4;
		}

		newdrawsurf = gl_lms.lightmap_surfaces[0];

		for( pSurf = gl_lms.lightmap_surfaces[0]; pSurf != 0; pSurf = pSurf->lightmapchain )
		{
			int		smax, tmax;
			byte	*base;

			smax = ( pSurf->extents[0] >> 4 ) + 1;
			tmax = ( pSurf->extents[1] >> 4 ) + 1;

			if( LM_AllocBlock( smax, tmax, &pSurf->dlight_s, &pSurf->dlight_t ) )
			{
				base = gl_lms.lightmap_buffer;
				base += ( pSurf->dlight_t * BLOCK_WIDTH + pSurf->dlight_s ) * LIGHTMAP_BYTES;

				R_BuildLightMap( pSurf, base, BLOCK_WIDTH*LIGHTMAP_BYTES );
			}
			else
			{
				msurface_t *drawsurf;

				// upload what we have so far
				LM_UploadBlock( true );

				// draw all surfaces that use this lightmap
				for ( drawsurf = newdrawsurf; drawsurf != pSurf; drawsurf = drawsurf->lightmapchain )
				{
					if( drawsurf->polys )
					{
						matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

						if( drawsurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
						{
							DrawSubdividedLightmaps( drawsurf, 
											  ( drawsurf->light_s - drawsurf->dlight_s ) * ( 1.0 / 128.0 ), 
											( drawsurf->light_t - drawsurf->dlight_t ) * ( 1.0 / 128.0 ) );
						}
						else
						{
							DrawGLPolyChain( drawsurf, 
											  ( drawsurf->light_s - drawsurf->dlight_s ) * ( 1.0 / 128.0 ), 
											( drawsurf->light_t - drawsurf->dlight_t ) * ( 1.0 / 128.0 ) );
						}

						if ( matrix_modified )
						{
							qglPopMatrix();
						}
					}
				}

				newdrawsurf = drawsurf;

				// clear the block
				LM_InitBlock();

				// try uploading the block now
				if( !LM_AllocBlock( smax, tmax, &pSurf->dlight_s, &pSurf->dlight_t ) )
				{
					ri.Sys_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed (dynamic)\n", smax, tmax );
				}

				base = gl_lms.lightmap_buffer;
				base += ( pSurf->dlight_t * BLOCK_WIDTH + pSurf->dlight_s ) * LIGHTMAP_BYTES;

				R_BuildLightMap( pSurf, base, BLOCK_WIDTH*LIGHTMAP_BYTES );
			}
		}

		/*
		** draw remainder of dynamic lightmaps that haven't been uploaded yet
		*/
		if ( newdrawsurf )
			LM_UploadBlock( true );

		if( bReTransform == TRUE )
		{
			for ( pSurf = newdrawsurf; pSurf != 0; pSurf = pSurf->lightmapchain )
			{
				if ( pSurf->polys )
				{
					matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

					if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
					{
						DrawSubdividedLightmaps( pSurf, ( pSurf->light_s - pSurf->dlight_s ) * ( 1.0 / 128.0 ), ( pSurf->light_t - pSurf->dlight_t ) * ( 1.0 / 128.0 ) );
					}
					else
					{
						DrawGLPolyChain( pSurf, ( pSurf->light_s - pSurf->dlight_s ) * ( 1.0 / 128.0 ), ( pSurf->light_t - pSurf->dlight_t ) * ( 1.0 / 128.0 ) );
					}

					if ( matrix_modified )
					{
						qglPopMatrix();
					}
				}
			}
		}
		else
		{
			for ( pSurf = newdrawsurf; pSurf != 0; pSurf = pSurf->lightmapchain )
			{
				if ( pSurf->polys )
				{
					if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
					{
						DrawSubdividedLightmaps( pSurf, ( pSurf->light_s - pSurf->dlight_s ) * ( 1.0 / 128.0 ), ( pSurf->light_t - pSurf->dlight_t ) * ( 1.0 / 128.0 ) );
					}
					else
					{
						DrawGLPolyChain( pSurf, ( pSurf->light_s - pSurf->dlight_s ) * ( 1.0 / 128.0 ), ( pSurf->light_t - pSurf->dlight_t ) * ( 1.0 / 128.0 ) );
					}
				}
			}
		}
	}

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly( msurface_t *pSurf )
{
	int			maps;
	image_t		*image;
	qboolean is_dynamic = false;

	image = R_TextureAnimation( pSurf->texinfo );

	if( pSurf->texinfo->flags & SURF_TRANS33 )
	{
		qglColor4f( 1.0, 1.0, 1.0, 0.33 );
		GL_TexEnv( GL_MODULATE );
	}
	else if( pSurf->texinfo->flags & SURF_TRANS66 )
	{
		qglColor4f( 1.0, 1.0, 1.0, 0.66 );
		GL_TexEnv( GL_MODULATE );
	}
	else
	{
		qglColor4f( 1.0, 1.0, 1.0, 1.0 );
		GL_TexEnv( GL_REPLACE );
	}

	GL_Bind( image->texnum );

	if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) )
	{
		GL_TexEnv( GL_MODULATE );

		DrawSubdividedPolys( pSurf );
	}
	else
	{
		DrawGLPoly( pSurf );
	}

	if( r_fullbright->value != 0 )
	{
		return;
	}

	if( pSurf->texinfo->flags & ( SURF_SKY | SURF_FULLBRIGHT ) )
	{
		return;
	}

	if( ( pSurf->texinfo->flags & SURF_MIDTEXTURE ) && qglMTexCoord2fSGIS == NULL )
	{
		return;
	}

	/*
	** check for lightmap modification
	*/

	for ( maps = 0; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++ )
	{
		if ( r_newrefdef.lightstyles[pSurf->styles[maps]].white != pSurf->cached_light[maps] )
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
	if ( ( pSurf->dlightframe == r_framecount ) )
	{
dynamic:
		if ( gl_dynamic->value )
		{
			if( ( pSurf->texinfo->flags != SURF_FOGPLANE ) )
			{
				is_dynamic = true;
			}
		}
	}

	if ( is_dynamic )
	{
		if ( ( pSurf->styles[maps] >= 32 || pSurf->styles[maps] == 0 ) && ( pSurf->dlightframe != r_framecount ) )
		{
			unsigned	temp[34*34];
			int			smax, tmax;

			smax = (pSurf->extents[0]>>4)+1;
			tmax = (pSurf->extents[1]>>4)+1;

			R_BuildLightMap( pSurf, (byte *)temp, smax*4 );
			R_SetCacheState( pSurf );

			GL_Bind( gl_state.lightmap_textures + pSurf->lightmaptexturenum );

			qglTexSubImage2D( GL_TEXTURE_2D, 0,
							  pSurf->light_s, pSurf->light_t, 
							  smax, tmax, 
							  GL_LIGHTMAP_FORMAT, 
							  GL_UNSIGNED_BYTE, temp );

			pSurf->lightmapchain = gl_lms.lightmap_surfaces[pSurf->lightmaptexturenum];
			gl_lms.lightmap_surfaces[pSurf->lightmaptexturenum] = pSurf;
		}
		else
		{
			pSurf->lightmapchain = gl_lms.lightmap_surfaces[0];
			gl_lms.lightmap_surfaces[0] = pSurf;
		}
	}
	else
	{
		pSurf->lightmapchain = gl_lms.lightmap_surfaces[pSurf->lightmaptexturenum];
		gl_lms.lightmap_surfaces[pSurf->lightmaptexturenum] = pSurf;
	}
	GL_TexEnv (GL_REPLACE);
}

/*
================
R_TranslateAlphaSurface
================
*/
int R_TranslateAlphaSurface (entity_t *pent)
{
	CVector			mins, maxs;
	qboolean		rotated;
	model_t			*pmodel;
//	static entity_t	*last_entity = NULL;

	if ( pent == NULL || pent->model == r_worldmodel || pent->model == NULL )
		return 0;

//	if( pent == last_entity )
//		return 0 ;

//	last_entity = pent;

	pmodel = (model_t *)pent->model;

	if (pent->angles.x || pent->angles.y || pent->angles.z)
	{
		rotated = true;

		mins.x = pent->origin.x - pmodel->radius;
		mins.y = pent->origin.y - pmodel->radius;
		mins.z = pent->origin.z - pmodel->radius;

		maxs.x = pent->origin.x + pmodel->radius;
		maxs.y = pent->origin.y + pmodel->radius;
		maxs.z = pent->origin.z + pmodel->radius;
	}
	else
	{
		rotated = false;
		mins = pent->origin + pmodel->mins;
		maxs = pent->origin + pmodel->maxs;
	}

	modelorg = r_newrefdef.vieworg - pent->origin;

	if (rotated)
	{
		CVector	temp;
		CVector	forward, right, up;

		temp = modelorg;
		AngleToVectors (pent->angles, forward, right, up);
		modelorg.x = DotProduct (temp, forward);
		modelorg.y = DotProduct (temp, right);
		modelorg.z = DotProduct (temp, up);
	}

	qglPushMatrix ();

pent->angles.x = -pent->angles.x;
pent->angles.z = -pent->angles.z;

	qglTranslatef (pent->origin.x,  pent->origin.y,  pent->origin.z);

	qglRotatef (pent->angles.y,  0, 0, 1);
	qglRotatef (-pent->angles.x,  0, 1, 0);
	qglRotatef (-pent->angles.z,  1, 0, 0);

pent->angles.x = -pent->angles.x;
pent->angles.z = -pent->angles.z;
	
	return 1;
}

void R_DrawFogSurfaces()
{
	int			i;//, map;
	msurface_t*	pSurf;
//	image_t*	image;
//	unsigned	lmtex;
//	entity_t	ent;
//	qboolean	is_dynamic = false;
//	glpoly_t	*pPoly;
	msurface_t	*pTestSurface;
	qboolean	bGrabbedTestSurface;
//	float		fScroll;
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
			GL_SetTexturePalette( NULL, FALSE );
		}

		for( i = 0; i < r_fog_volume_count; i++ )
		{
			if( r_fogvolumes[i].surfaces == NULL )
				continue;

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

				GL_RenderLightmappedPoly( pSurf );
			}
		}
	}
	else
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags );

		if( qglColorTableEXT )
		{
			GL_SetTexturePalette( NULL, FALSE );
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

/*
================
R_DrawFullbrightSurfaces
================
*/
void R_DrawFullbrightSurfaces()
{
	msurface_t	*pSurf;
	int			matrix_modified = 0;

	if( r_surfs_fullbright == NULL  )
	{
		return;
	}

	// SCG[4/16/99]: For texture animation
	memset (&r_global_ent, 0, sizeof(r_global_ent));
	r_global_ent.frame = (int)(r_newrefdef.time*2);
	currententity = &r_global_ent;

	//
	// go back to the world matrix
	//
    qglLoadMatrixf (r_world_matrix);

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	unsigned int nStateFlags;

	nStateFlags = GLSTATE_PRESET1;

	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// SCG[5/18/99]: Set the state
	GL_SetState( nStateFlags | GLSTATE_BLEND );

	// SCG[5/18/99]: Set the world palette
	GL_SetTexturePalette( NULL, FALSE );

	GL_TexEnv( GL_MODULATE );

	for( pSurf = r_surfs_fullbright; pSurf; pSurf = pSurf->texturechain )
	{
		matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

		R_RenderBrushPoly( pSurf );

		if ( matrix_modified )
		{
			qglPopMatrix();
		}

	}

	r_surfs_fullbright = NULL;
}

/*
================
R_DrawAlphaSurfaces
================
*/
void R_DrawAlphaSurfaces (void)
{
	msurface_t	*pSurf;
	int			matrix_modified = 0;

	if( ( r_surfs_alpha == NULL ) || ( gl_alphasurfaces->value == 0 ) )
	{
		return;
	}

	// SCG[4/16/99]: For texture animation
	memset (&r_global_ent, 0, sizeof(r_global_ent));
	r_global_ent.frame = (int)(r_newrefdef.time*2);
	currententity = &r_global_ent;

	//
	// go back to the world matrix
	//
    qglLoadMatrixf (r_world_matrix);

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	unsigned int nStateFlags;

	nStateFlags = GLSTATE_PRESET1;

	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	nStateFlags &= ~GLSTATE_DEPTH_MASK;

	// SCG[5/18/99]: Set the state
	GL_SetState( nStateFlags | GLSTATE_BLEND );

	// SCG[5/18/99]: Set the world palette
	GL_SetTexturePalette( NULL, FALSE );

	GL_TexEnv( GL_MODULATE );

	memset( gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces ) );

	for( pSurf = r_surfs_alpha; pSurf; pSurf = pSurf->texturechain )
	{
		matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

		R_RenderBrushPoly( pSurf );

		if ( matrix_modified )
		{
			qglPopMatrix();
		}

	}

	R_BlendLightmaps( TRUE );

	r_surfs_alpha = NULL;
}
/*
================
R_DrawMidTextures
================
*/
// SCG[3/26/99]: MidTextures placed in their own draw list to fix draw problems...
// SCG[4/23/99]: Restructured this function to minimize state changes.
void R_DrawMidTextures (void)
{
	msurface_t	*pSurf;
	int			matrix_modified = 0;
//	qboolean	is_dynamic = false;
	
	if( ( r_surfs_mid == NULL ) || ( gl_midtextures->value == 0 ) )
	{
		return;
	}

	// SCG[4/16/99]: For texture animation
	memset (&r_global_ent, 0, sizeof(r_global_ent));
	r_global_ent.frame = (int)(r_newrefdef.time*2);
	currententity = &r_global_ent;

	//
	// go back to the world matrix
	//
    qglLoadMatrixf (r_world_matrix);

	GL_TexEnv( GL_REPLACE );

	// SCG[5/18/99]: Set the alpha func so transparent pixels
	// SCG[5/18/99]: do not get drawn.
	GL_SetFunc( GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0 );
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	qglShadeModel( GL_SMOOTH );

	unsigned int nStateFlags;

	nStateFlags = GLSTATE_PRESET1;

	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	memset( gl_lms.lightmap_surfaces, 0, sizeof( gl_lms.lightmap_surfaces ) );

	if( ( qglMTexCoord2fSGIS != NULL ) && ( r_fullbright->value == 0 ) )
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags | GLSTATE_MULTITEXTURE | GLSTATE_BLEND | GLSTATE_ALPHA_TEST );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE1_SGIS, GL_MODULATE );

		// SCG[5/18/99]: Set the world palette
		GL_SetTexturePalette( NULL, FALSE );

		for( pSurf = r_surfs_mid; pSurf; pSurf = pSurf->texturechain )
		{
			matrix_modified = R_TranslateAlphaSurface( pSurf->entity );

			GL_RenderLightmappedPoly( pSurf );

			if ( matrix_modified )
			{
				qglPopMatrix();
			}
		}
	}
	else
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags | GLSTATE_BLEND | GLSTATE_ALPHA_TEST );

		// SCG[5/18/99]: Set the world palette
		GL_SetTexturePalette( NULL, FALSE );

		for( pSurf = r_surfs_mid; pSurf; pSurf = pSurf->texturechain )
		{
			matrix_modified = R_TranslateAlphaSurface( pSurf->entity );
			
			R_RenderBrushPoly( pSurf );

			if ( matrix_modified )
			{
				qglPopMatrix();
			}
		}

		R_BlendLightmaps( TRUE );
	}

	GL_SetFunc( GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0 );

	r_surfs_mid = NULL;
}

/*
================
DrawTextureChains
================
*/
void DrawTextureChains (void)
{
	int		i;
	msurface_t	*s;
	image_t		*image;

	// SCG[5/18/99]: Set the state
	GL_SetState( GLSTATE_PRESET1 );

	if ( !qglSelectTextureSGIS )
	{
		for ( i = 0, image=gltextures ; i<numgltextures ; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			s = image->texturechain;
			if (!s)
				continue;
			c_visible_textures++;

			for ( ; s ; s=s->texturechain)
				R_RenderBrushPoly (s);

			image->texturechain = NULL;
		}
	}
	else
	{
		// draw non turbulent polys
		for ( i = 0, image=gltextures ; i<numgltextures ; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			if (!image->texturechain)
				continue;
			c_visible_textures++;

			for ( s = image->texturechain; s ; s=s->texturechain)
			{
				if ( !( s->flags & SURF_DRAWTURB ) )
					R_RenderBrushPoly (s);
			}
		}

		for ( i = 0, image=gltextures ; i<numgltextures ; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			s = image->texturechain;
			if (!s)
				continue;

			for ( ; s ; s=s->texturechain)
			{
				if ( s->flags & SURF_DRAWTURB )
					R_RenderBrushPoly (s);
			}

			image->texturechain = NULL;
		}
	}
}

//	nelno:	world poly rendering for multitexture systems

static void GL_RenderLightmappedPoly( msurface_t *pSurf )
{
	int			i, nv = pSurf->polys->numverts;
	int			map;
	image_t		*image = R_TextureAnimation( pSurf->texinfo );
	qboolean	is_dynamic = false;
	unsigned	lmtex = pSurf->lightmaptexturenum;
	glpoly_t	*pPoly;
	float		fScroll = FLOWING_SPEED;

	if( pSurf->texinfo->flags & ( SURF_SKY | SURF_FULLBRIGHT ) )
	{
		return;
	}

	for ( map = 0; map < MAXLIGHTMAPS && pSurf->styles[map] != 255; map++ )
	{
		if ( r_newrefdef.lightstyles[pSurf->styles[map]].white != pSurf->cached_light[map] )
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
	if ( ( pSurf->dlightframe == r_framecount ) )
	{
dynamic:
		if ( gl_dynamic->value )
		{
			if( ( pSurf->texinfo->flags != SURF_FOGPLANE ) )
			{
				is_dynamic = true;
			}
		}
	}

	c_brush_polys++;

	if ( is_dynamic )
	{
		unsigned	temp[128*128];
		int			smax, tmax;

		if ( ( pSurf->styles[map] >= 32 || pSurf->styles[map] == 0 ) && ( pSurf->dlightframe != r_framecount ) )
		{
			smax = ( pSurf->extents[0] >> 4 )+1;
			tmax = ( pSurf->extents[1] >> 4 )+1;

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
			smax = ( pSurf->extents[0] >> 4 ) + 1;
			tmax = ( pSurf->extents[1] >> 4 ) + 1;

			R_BuildLightMap( pSurf, (byte *)temp, smax*4 );

			GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + 0 );

			lmtex = 0;

			qglTexSubImage2D( GL_TEXTURE_2D, 0,
							  pSurf->light_s, pSurf->light_t, 
							  smax, tmax, 
							  GL_LIGHTMAP_FORMAT, 
							  GL_UNSIGNED_BYTE, temp );

		}

		GL_MBind( GL_TEXTURE0_SGIS, image->texnum );
		GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + lmtex );

		if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) && !( pSurf->texinfo->flags & SURF_FLOWING ) )
		{
			for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
			{
				qglBegin( GL_TRIANGLE_FAN );
				for (i=0 ; i< pPoly->numverts; i++)
				{

					qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i] / image->width, pPoly->t1[i]  / image->height );
					qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
					qglVertex3fv( pPoly->verts[i] );
				}
				qglEnd();
			}
		}
		else
		{
			if( pSurf->texinfo->flags & SURF_FLOWING )
			{
				for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
				{
					qglBegin( GL_POLYGON );
					for (i=0 ; i< nv; i++)
					{

						qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i] - fScroll, pPoly->t1[i]);
						qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
						qglVertex3fv( pPoly->verts[i] );
					}
					qglEnd();
				}
			}
			else
			{
				for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
				{
					qglBegin( GL_POLYGON );
					for (i=0 ; i< nv; i++)
					{

						qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i], pPoly->t1[i]);
						qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
						qglVertex3fv( pPoly->verts[i] );
					}
					qglEnd();
				}
			}
		}
	}
	else
	{
		GL_MBind( GL_TEXTURE0_SGIS, image->texnum );
		GL_MBind( GL_TEXTURE1_SGIS, gl_state.lightmap_textures + lmtex );

		// this block is debug code....
		if (r_speeds->value && image)
		{
			// see the below code.. this is how we calculate the correct index
			//	image->texnum = TEXNUM_IMAGES + (image - gltextures);
			int texnum = image - gltextures;
			
			// count how many textures and bytes we're using
			if (!c_texture_list[texnum])
			{
				c_texture_list[texnum] = 0x01;	 // mark this one as counted
				c_visible_textures++;
				c_visible_texture_bytes += image->upload_width * image->upload_height;
			}
			// count how many lightmaps and bytes we're using
			if (!c_lightmap_list[lmtex])
			{
				c_lightmap_list[lmtex] = 0x01;	 // mark this one as counted
				c_visible_lightmaps++;
				c_visible_lightmap_bytes += BLOCK_WIDTH * BLOCK_HEIGHT * 4;
			}
		}

		if( pSurf->flags & ( SURF_DRAWFOG | SURF_DRAWTURB ) && !( pSurf->texinfo->flags & SURF_FLOWING ) )
		{
			for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
			{
				qglBegin( GL_TRIANGLE_FAN );
				for( i = 0; i < pPoly->numverts; i++ )
				{
					qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i] / image->width, pPoly->t1[i]  / image->height );
					qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
					qglVertex3fv( pPoly->verts[i] );
				}
				qglEnd();
			}
		}
		else
		{
			if( pSurf->texinfo->flags & SURF_FLOWING )
			{
				for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
				{
					qglBegin( GL_POLYGON );
					for( i = 0; i < nv; i++ )
					{
						qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i] - fScroll, pPoly->t1[i]);
						qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
						qglVertex3fv( pPoly->verts[i] );
					}
					qglEnd();
				}
			}
			else
			{
				for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->chain )
				{
					qglBegin( GL_POLYGON );
					for( i = 0; i < nv; i++ )
					{
						qglMTexCoord2fSGIS( GL_TEXTURE0_SGIS, pPoly->s1[i], pPoly->t1[i]);
						qglMTexCoord2fSGIS( GL_TEXTURE1_SGIS, pPoly->s2[i], pPoly->t2[i]);
						qglVertex3fv( pPoly->verts[i] );
					}
					qglEnd();
				}
			}
		}
	}
}

/*
=================
R_DrawInlineBModel
=================
*/
void R_DrawInlineBModel (void)
{
	int			i, k;
	mplane_t	*pplane;
	float		dot;
	msurface_t	*pSurf;
	dlight_t	*lt;

	// calculate dynamic lighting for bmodel
	if ( !gl_flashblend->value )
	{
		lt = r_newrefdef.dlights;
		for( k = 0; k < r_newrefdef.num_dlights; k++, lt++ )
		{
			R_MarkLights( lt, 1 << k, currentmodel->nodes + currentmodel->firstnode );
		}
	}

	pSurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	//
	// draw texture
	//
	for( i = 0; i < currentmodel->nummodelsurfaces; i++, pSurf++)
	{
		if( pSurf->texinfo->flags & SURF_NODRAW )
			continue;

		// find which side of the node we are on
		pplane = pSurf->plane;

		dot = DotProduct( modelorg, pplane->normal ) - pplane->dist;

	// draw the polygon
		if ( ( ( pSurf->flags & SURF_PLANEBACK ) && ( dot < -BACKFACE_EPSILON ) ) ||
			( !( pSurf->flags & SURF_PLANEBACK ) && ( dot > BACKFACE_EPSILON ) ) )
		{
			if( gl_drawflat->value == 1 )
			{
				DrawFlatPoly( pSurf );
				continue;
			}
			
			// SCG[5/10/99]: Added for surface sprites.
			pSurf->visframe = r_framecount;

			// SCG[5/10/99]: Set the entity for this surface
			pSurf->entity = currententity;

			if( pSurf->texinfo->flags & ( SURF_TRANS33 | SURF_TRANS66 | SURF_ALPHACHAN /*| SURF_FULLBRIGHT | */ ) )
			{	// add to the translucent chain
				pSurf->texturechain = r_surfs_alpha;
				r_surfs_alpha = pSurf;
			}
			else if( pSurf->texinfo->flags & SURF_FULLBRIGHT )
			{	// add to the translucent chain
				pSurf->texturechain = r_surfs_fullbright;
				r_surfs_fullbright = pSurf;
			}
			else if( pSurf->texinfo->flags & SURF_MIDTEXTURE )
			{	// add to the translucent chain
				if( gl_midtextures->value == 0 )
				{
					continue;
				}
				qglShadeModel( GL_SMOOTH );
				GL_SetFunc( GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0 );
				GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
				qglEnable( GL_BLEND );
				qglEnable( GL_ALPHA_TEST );

				if( ( qglMTexCoord2fSGIS != NULL ) && ( r_fullbright->value == 0 ) )
				{
					GL_RenderLightmappedPoly( pSurf );	// SCG[1/7/00]: Stuck here for sorting reasons. Not as efficient due to 
														// SCG[1/7/00]: state changes and drawing setup. Bite me.
				}
				else
				{
					GL_EnableMultitexture( false );
					R_RenderBrushPoly( pSurf );
					GL_EnableMultitexture( true );
				}

				qglDisable( GL_ALPHA_TEST );
				qglDisable( GL_BLEND );

				GL_SetFunc( GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0 );
			}
			else if ( qglMTexCoord2fSGIS && !( pSurf->flags & SURF_DRAWTURB ) && ( r_fullbright->value != 1 ) )
			{
				GL_RenderLightmappedPoly( pSurf );
			}
			else
			{
				GL_EnableMultitexture( false );
				R_RenderBrushPoly( pSurf );
				GL_EnableMultitexture( true );
			}
		}
	}

//	if ( !( currententity->flags & RF_TRANSLUCENT ) )
	{
		if ( !qglMTexCoord2fSGIS )
		{
			R_BlendLightmaps( FALSE );
		}
	}

	GL_TexEnv( GL_REPLACE );
}

/*
=================
R_DrawInlineBModelFogShell
=================
*/
void R_DrawInlineBModelFogShell (CVector& origin, bool fDrawMidtextures)
{
	int			i, j, nv;
	mplane_t	*pplane;
	float		dot;
	msurface_t	*psurf;
	glpoly_t    *p;
    float       alpha;

	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	//
	// draw shell
	//
    for (i=0 ; i<currentmodel->nummodelsurfaces ; i++, psurf++)
    {
        if( psurf->texinfo->flags & SURF_NODRAW )
            continue;

        // find which side of the node we are on
        pplane = psurf->plane;

        dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

        // draw the polygon
        if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
            (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
        {
            nv = psurf->polys->numverts;
            for ( p = psurf->polys; p; p = p->chain )
            {
                if (fDrawMidtextures == (0!=(psurf->texinfo->flags & SURF_MIDTEXTURE)))
                {
                    qglBegin (GL_POLYGON);
                    for (j=0 ; j< nv; j++)
                    {
                        alpha = GL_GetFogSampleAlphaAtZ(p->verts[j].z+origin.z, gFogSample);

#pragma message("TODO Ash: remove this 3dfx minidriver fix")
                        if (alpha<0.001f) alpha = 0.0f; // temp fix for 3dfx minidriver

                        qglColor4f(gFogSample.fog_r, gFogSample.fog_g, gFogSample.fog_b, alpha);
                        qglVertex3fv (p->verts[j]);
                    }
                    qglEnd ();
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//	R_CullParticleVolume
//
////////////////////////////////////////////////////////////////////////////////
qboolean R_CullParticleVolume (CVector &mins, CVector &maxs)
{
	// new, modified bounding box check.. we are testing to see if any of the 
	// points are in the view frustrum... so, if any are in front of one of the
	// frustrum planes, then we know it's good... return immediately
#if 1	
	int		i;

	if (r_nocull->value)
		return false;

	// check each of the view frustrum planes
	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide (mins, maxs, (cplane_t *) &frustum[i]) == 2)		// 1 == in front  -> DRAW
			return true;													// 2 == behind	  -> DON'T DRAW
																			// 3 ==	inbetween -> DRAW
	// if we're here, it's in the view frustrum
	return false;
#else
	
	int p, f, aggregatemask = ~0, i;
	CVector		bbox [8];

	//	copy mins/max into bounding box
	for (i = 0; i < 8; i++)
	{
		CVector   tmp;

		if (i & 1)
			tmp.x = mins.x;
		else
			tmp.x = maxs.x;

		if (i & 2)
			tmp.y = mins.y;
		else
			tmp.y = maxs.y;

		if (i & 4)
			tmp.z = mins.z;
		else
			tmp.z = maxs.z;

		bbox[i] = tmp;
	}

	for ( p = 0; p < 8; p++ )
	{
		int mask = 0;

		for (f = 0; f < 4; f++)
		{
			float dp = DotProduct (frustum[f].normal, bbox[p]);

			if ((dp - frustum[f].dist ) < 0)
			{
				mask |= (1 << f);
			}
		}

		aggregatemask &= mask;
	}

	if ( aggregatemask )
	{
		return true;
	}

	return false;
#endif
}


/*
=================
R_DrawBrushModel
=================
*/

void R_DrawBrushModel (entity_t *e)
{
	CVector     bbox[2];
	qboolean	rotated;
    bool        fFogged=false;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;
	gl_state.nTexnum[0] = gl_state.nTexnum[1] = -1;

	if (e->angles.x || e->angles.y || e->angles.z)
	{
		rotated = true;
		
		bbox[0].x = e->origin.x - currentmodel->radius;
		bbox[0].y = e->origin.y - currentmodel->radius;
		bbox[0].z = e->origin.z - currentmodel->radius;
		
		bbox[1].x = e->origin.x + currentmodel->radius;
		bbox[1].y = e->origin.y + currentmodel->radius;
		bbox[1].z = e->origin.z + currentmodel->radius;
	}
	else
	{
		rotated = false;
		bbox[0] = e->origin + currentmodel->mins;
		bbox[1] = e->origin + currentmodel->maxs;
	}

	if (R_CullBox (bbox[0], bbox[1]))
		return;

	qglColor3f (1,1,1);
	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));

	modelorg = r_newrefdef.vieworg - e->origin;
	if (rotated)
	{
		CVector	temp;
		CVector	forward, right, up;

		temp = modelorg;
		AngleToVectors (e->angles, forward, right, up);
		modelorg.x = DotProduct (temp, forward);
		modelorg.y = -DotProduct (temp, right);
		modelorg.z = DotProduct (temp, up);
	}

	//	set up rotation matrix for this model
    qglPushMatrix ();
	e->angles.x = -e->angles.x;		// stupid quake bug
	e->angles.z = -e->angles.z;		// stupid quake bug
	R_RotateForEntity (e);
	e->angles.x = -e->angles.x;		// stupid quake bug
	e->angles.z = -e->angles.z;		// stupid quake bug

	unsigned int nStateFlags;

	if( qglColorTableEXT )
	{
		nStateFlags = GLSTATE_PRESET1 | GLSTATE_SHARED_PALETTE;
	}
	else
	{
		nStateFlags = GLSTATE_PRESET1;
	}

	if ( currententity->flags & RF_TRANSLUCENT )
	{
		nStateFlags |= GLSTATE_BLEND;
		GL_TexEnv( GL_MODULATE );
	}

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	if ( qglMTexCoord2fSGIS )
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags | GLSTATE_MULTITEXTURE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE1_SGIS, GL_MODULATE );
	}
	else
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags );
	}

	if ( currententity->flags & RF_TRANSLUCENT )
	{
		qglColor4f (1,1,1,0.25);
	}

	if( qglColorTableEXT )
	{
		GL_SetTexturePalette( NULL, FALSE );
	}

	R_DrawInlineBModel ();

    if ((!gl_drawflat->value)&&(gl_vfogents->value))
    {
        int nFogSamples = R_MAX_FOG_SAMPLES;   // set this appropriately for the thing being fogged
        fFogged = GL_FogBBox(bbox, 2, gFogSample, nFogSamples);
        if (fFogged)
        {
			GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
			GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
            GL_TexEnv(GL_MODULATE);
				
            R_DrawInlineBModelFogShell(e->origin, false);  // don't draw the midtextures
        }
    }

	qglPopMatrix ();

    // draw the bmodel bbox
    if (gl_bbox->value)
    {
        GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
        GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
        GL_TexEnv(GL_MODULATE);

        qglColor4f (1.0f, 1.0f, 1.0f, 0.3f);
    	
        qglBegin( GL_QUADS );

        // min x face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        
        // max x face
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);

        // min y face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);
        
        // max y face
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);

        // min z face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);

        // max z face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);

        qglEnd();
    }

    // draw the fog volume bbox
    if (gl_bbox->value && fFogged)
    {
        CVector vMins = r_fogvolumes[r_current_fog_volume].vMins;
        CVector vMaxs = r_fogvolumes[r_current_fog_volume].vMaxs;

        GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
        GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST | GLSTATE_DEPTH_MASK);
        GL_TexEnv(GL_MODULATE);

        qglColor3fv (r_fogvolumes[r_current_fog_volume].vColor);

        qglBegin( GL_QUADS );

        // min x face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        
        // max x face
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);

        // min y face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);
        
        // max y face
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);

        // min z face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);

        // max z face
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);

        qglEnd();
    }

    // draw the bmodel bbox
    if (gl_bbox->value)
    {
        GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
        GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
        GL_TexEnv(GL_MODULATE);

        qglColor4f (1.0f, 1.0f, 1.0f, 0.3f);
    	
        qglBegin( GL_QUADS );

        // min x face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        
        // max x face
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);

        // min y face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);
        
        // max y face
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);

        // min z face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[0].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[0].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[0].z);

        // max z face
        qglVertex3f(bbox[0].x, bbox[0].y, bbox[1].z);
        qglVertex3f(bbox[0].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[1].y, bbox[1].z);
        qglVertex3f(bbox[1].x, bbox[0].y, bbox[1].z);

        qglEnd();
    }

    // draw the fog volume bbox
    if (gl_bbox->value && fFogged)
    {
        CVector vMins = r_fogvolumes[r_current_fog_volume].vMins;
        CVector vMaxs = r_fogvolumes[r_current_fog_volume].vMaxs;

        GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
        GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST | GLSTATE_DEPTH_MASK);
        GL_TexEnv(GL_MODULATE);

        qglColor3fv (r_fogvolumes[r_current_fog_volume].vColor);

        qglBegin( GL_QUADS );

        // min x face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        
        // max x face
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);

        // min y face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);
        
        // max y face
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);

        // min z face
        qglVertex3f(vMins.x, vMins.y, vMins.z);
        qglVertex3f(vMins.x, vMaxs.y, vMins.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMins.z);
        qglVertex3f(vMaxs.x, vMins.y, vMins.z);

        // max z face
        qglVertex3f(vMins.x, vMins.y, vMaxs.z);
        qglVertex3f(vMins.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMaxs.y, vMaxs.z);
        qglVertex3f(vMaxs.x, vMins.y, vMaxs.z);

        qglEnd();
    }

	GL_TexEnv (GL_REPLACE);
}

/*
=================
R_DrawBrushModelMidTextureFogShell
=================
*/

void R_DrawBrushModelMidTextureFogShell (entity_t *e)
{
	CVector     bbox[2];
	qboolean	rotated;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;

	if (e->angles.x || e->angles.y || e->angles.z)
	{
		rotated = true;
		
		bbox[0].x = e->origin.x - currentmodel->radius;
		bbox[0].y = e->origin.y - currentmodel->radius;
		bbox[0].z = e->origin.z - currentmodel->radius;
		
		bbox[1].x = e->origin.x + currentmodel->radius;
		bbox[1].y = e->origin.y + currentmodel->radius;
		bbox[1].z = e->origin.z + currentmodel->radius;
	}
	else
	{
		rotated = false;
		bbox[0] = e->origin + currentmodel->mins;
		bbox[1] = e->origin + currentmodel->maxs;
	}

	if (R_CullBox (bbox[0], bbox[1]))
		return;

	modelorg = r_newrefdef.vieworg - e->origin;
	if (rotated)
	{
		CVector	temp;
		CVector	forward, right, up;

		temp = modelorg;
		AngleToVectors (e->angles, forward, right, up);
		modelorg.x = DotProduct (temp, forward);
		modelorg.y = -DotProduct (temp, right);
		modelorg.z = DotProduct (temp, up);
	}

	//	set up rotation matrix for this model
    qglPushMatrix ();
	e->angles.x = -e->angles.x;		// stupid quake bug
	e->angles.z = -e->angles.z;		// stupid quake bug
	R_RotateForEntity (e);
	e->angles.x = -e->angles.x;		// stupid quake bug
	e->angles.z = -e->angles.z;		// stupid quake bug

    int nFogSamples = R_MAX_FOG_SAMPLES;   // set this appropriately for the thing being fogged
    if (true==GL_FogBBox(bbox, 2, gFogSample, nFogSamples))
    {
        GL_TexEnv(GL_MODULATE);

		GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
		GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST );
		GL_TexEnv(GL_MODULATE);
        R_DrawInlineBModelFogShell(e->origin, true);   // only draw the midtextures
    }

	qglPopMatrix ();

	GL_TexEnv (GL_REPLACE);
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node)
{
	int			c, side, sidebit;
	mplane_t	*plane;
	msurface_t	*pSurf, **mark;
	mleaf_t		*pleaf;
	float		dot;
	image_t		*image;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != r_visframecount)
		return;

	if( ( node->mins.y > node->maxs.y ) || 
		( node->mins.x > node->maxs.x ) || 
		( node->mins.z > node->maxs.z ) ){
		ri.Sys_Error (ERR_FATAL, "Flipped mins/maxs on node or leaf!!\n");
	}

//	if (R_CullBox (node->minmaxs, node->minmaxs+3))
	if (R_CullBox (node->mins, node->maxs))
		return;
	
// if a leaf node, draw stuff
	if (node->contents != -1)
	{
		pleaf = (mleaf_t *)node;

		// check for door connected areas
		if (r_newrefdef.areabits)
		{
			if (! (r_newrefdef.areabits[pleaf->area>>3] & (1<<(pleaf->area&7)) ) )
				return;		// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
#ifdef	_DEBUG
				if (!(*mark))
					Sys_Error ("Error parsing BSP.  This map has likely exceeded BSP limitations.\n");
#endif
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}

		return;
	}

// node is just a decision point, so go down the apropriate sides

// find which side of the node we are on
	plane = node->plane;

	switch (plane->type)
	{
	case PLANE_X:
		dot = modelorg.x - plane->dist;
		break;
	case PLANE_Y:
		dot = modelorg.y - plane->dist;
		break;
	case PLANE_Z:
		dot = modelorg.z - plane->dist;
		break;
	default:
		dot = DotProduct (modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0)
	{
		side = 0;
		sidebit = 0;
	}
	else
	{
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

// recurse down the children, front side first
	R_RecursiveWorldNode( node->children[side] );

	// draw stuff
	for( c = node->numsurfaces, pSurf = r_worldmodel->surfaces + node->firstsurface; c ; c--, pSurf++ )
	{
		if( pSurf->texinfo->flags & SURF_NODRAW )
			continue;

		if( pSurf->visframe != r_framecount )
			continue;

		if ( ( pSurf->flags & SURF_PLANEBACK ) != sidebit )
			continue;		// wrong side

		if( gl_drawflat->value == 1 )
		{
			DrawFlatPoly( pSurf );
			continue;
		}

		// SCG[5/10/99]: Set the entity for this surface
		pSurf->entity = NULL;

		if( pSurf->texinfo->flags & SURF_SKY )
		{	// just adds to visible sky bounds
			R_AddSkySurface( pSurf );
		}
		else if( pSurf->flags & SURF_DRAWFOG && !( pSurf->texinfo->flags & ( SURF_DRAWTURB | SURF_FLOWING) ) )
		{
			r_fogvolumes[pSurf->brushnum].surface_count++;
			pSurf->texturechain = r_fogvolumes[pSurf->brushnum].surfaces;
			r_fogvolumes[pSurf->brushnum].surfaces = pSurf;
		}
		else if( pSurf->texinfo->flags & ( SURF_TRANS33 | SURF_TRANS66 | SURF_ALPHACHAN ) ) 
		{	// add to the translucent chain
			pSurf->texturechain = r_surfs_alpha;
			r_surfs_alpha = pSurf;
		}
		else if( pSurf->texinfo->flags & SURF_MIDTEXTURE )
		{	// add to the translucent chain
			pSurf->texturechain = r_surfs_mid;
			r_surfs_mid = pSurf;
		}
		else if( pSurf->texinfo->flags & SURF_FULLBRIGHT )
		{
			pSurf->texturechain = r_surfs_fullbright;
			r_surfs_fullbright = pSurf;
		}
		else
		{
			if( pSurf->texinfo->flags == SURF_FOGPLANE )
			{
				continue;
			}

			if( qglMTexCoord2fSGIS && !( pSurf->flags & SURF_DRAWTURB ) && ( r_fullbright->value != 1 ) )
			{
				GL_RenderLightmappedPoly( pSurf );
			}
			else
			{
				// the polygon is visible, so add it to the texture
				// sorted chain
				// FIXME: this is a hack for animation
				image = R_TextureAnimation( pSurf->texinfo );
				pSurf->texturechain = image->texturechain;
				image->texturechain = pSurf;
			}
		}
	}

	// recurse down the back side
	R_RecursiveWorldNode( node->children[!side] );
}


/*
=============
R_DrawWorld
=============
*/

void R_DrawWorld (void)
{
	if (!r_drawworld->value)
		return;

	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
		return;

	fWarpTime = r_newrefdef.time;

	// SCG[4/22/99]: Initialize surface chains
	r_surfs_alpha = NULL;
	r_surfs_mid = NULL;
	r_surfs_fullbright = NULL;

	// SCG[4/23/99]: Initialize fog volume surfaces
	for( int i = 0; i < r_fog_volume_count; i++ )
	{
		r_fogvolumes[i].surfaces = NULL;
	}

	currentmodel = r_worldmodel;

	modelorg = r_newrefdef.vieworg;

	// auto cycle the world frame for texture animation
	memset (&r_global_ent, 0, sizeof(r_global_ent));
	r_global_ent.frame = (int)(r_newrefdef.time*2);
	currententity = &r_global_ent;

	gl_state.nTexnum[0] = gl_state.nTexnum[1] = -1;

	qglColor3f (1,1,1);
	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));

	R_ClearSkyBox ();
	
	unsigned int nStateFlags;

	nStateFlags = GLSTATE_PRESET1;

	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	if ( qglMTexCoord2fSGIS )
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags | GLSTATE_MULTITEXTURE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );

		// SCG[5/18/99]: Set tmu 0 texture environment
		if ( gl_lightmap->value )
			GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );
		else 
			GL_MTexEnv( GL_TEXTURE1_SGIS, GL_MODULATE );
	}
	else
	{
		// SCG[5/18/99]: Set the state
		GL_SetState( nStateFlags );
	}

	// SCG[5/18/99]: Set the world palette
	if( qglColorTableEXT )
	{
		GL_SetTexturePalette( NULL, FALSE );
	}

	R_RecursiveWorldNode (r_worldmodel->nodes);

	R_DrawFogSurfaces();
	/*
	** theoretically nothing should happen in the next two functions
	** if multitexture is enabled
	*/
	DrawTextureChains();
	R_BlendLightmaps( FALSE );

	R_DrawSkyBox();

	R_DrawTriangleOutlines();
}


/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	byte	fatvis[MAX_MAP_LEAFS/8];
	mnode_t	*node;
	int		i, c;
	mleaf_t	*leaf;
	int		cluster;

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !r_novis->value && r_viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the pvs ends
	if (gl_lockpvs->value)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (r_novis->value || r_viewcluster == -1 || !r_worldmodel->vis)
	{
		// mark everything
		for (i=0 ; i<r_worldmodel->numleafs ; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i=0 ; i<r_worldmodel->numnodes ; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		return;
	}

	vis = Mod_ClusterPVS (r_viewcluster, r_worldmodel);
	// may have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy (fatvis, vis, (r_worldmodel->numleafs+7)/8);
		vis = Mod_ClusterPVS (r_viewcluster2, r_worldmodel);
		c = (r_worldmodel->numleafs+31)/32;
		for (i=0 ; i<c ; i++)
			((int *)fatvis)[i] |= ((int *)vis)[i];
		vis = fatvis;
	}
	
	for (i=0,leaf=r_worldmodel->leafs ; i<r_worldmodel->numleafs ; i++, leaf++)
	{
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster>>3] & (1<<(cluster&7)))
		{
			node = (mnode_t *)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}

#if 0
	for (i=0 ; i<r_worldmodel->vis->numclusters ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&r_worldmodel->leafs[i];	// FIXME: cluster
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
#endif
}



/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

static void LM_InitBlock( void )
{
	memset( gl_lms.allocated, 0, sizeof( gl_lms.allocated ) );
}

static void LM_UploadBlock( qboolean dynamic )
{
	int texture;
	int height = 0;

	if ( dynamic )
	{
		texture = 0;
	}
	else
	{
		texture = gl_lms.current_lightmap_texture;
	}

	GL_Bind( gl_state.lightmap_textures + texture );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ( dynamic )
	{
		int i;

		for ( i = 0; i < BLOCK_WIDTH; i++ )
		{
			if ( gl_lms.allocated[i] > height )
				height = gl_lms.allocated[i];
		}

		qglTexSubImage2D( GL_TEXTURE_2D, 
						  0,
						  0, 0,
						  BLOCK_WIDTH, height,
						  GL_LIGHTMAP_FORMAT,
						  GL_UNSIGNED_BYTE,
						  gl_lms.lightmap_buffer );
	}
	else
	{
		qglTexImage2D( GL_TEXTURE_2D, 
					   0, 
					   gl_lms.internal_format,
					   BLOCK_WIDTH, BLOCK_HEIGHT, 
					   0, 
					   GL_LIGHTMAP_FORMAT, 
					   GL_UNSIGNED_BYTE, 
					   gl_lms.lightmap_buffer );
		if ( ++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS )
			ri.Sys_Error( ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n" );
	}
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;

	best = BLOCK_HEIGHT;

	for (i=0 ; i<BLOCK_WIDTH-w ; i++)
	{
		best2 = 0;

		for (j=0 ; j<w ; j++)
		{
			if (gl_lms.allocated[i+j] >= best)
				break;
			if (gl_lms.allocated[i+j] > best2)
				best2 = gl_lms.allocated[i+j];
		}
		if (j == w)
		{	// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > BLOCK_HEIGHT)
		return false;

	for (i=0 ; i<w ; i++)
		gl_lms.allocated[*x + i] = best + h;

	return true;
}

/*
================
GL_BuildPolygonFromSurface
================
*/
void GL_BuildPolygonFromSurface( msurface_t *pSurf )
{
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
//	int			vertpage;
	CVector		vec;
	float		s, t;
	glpoly_t	*poly;
	CVector		total;

// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = pSurf->numedges;
//	vertpage = 0;

	total.Zero();
	//
	// draw texture
	//

	poly = AllocGLPoly( lnumverts );

	poly->next = pSurf->polys;
//	poly->flags = pSurf->flags;
	pSurf->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[pSurf->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}

		// SCG[6/3/99]: Find the s coord for this vert
		s = DotProduct( vec, pSurf->texinfo->s ) + pSurf->texinfo->s_offset;

		// SCG[6/3/99]: normalize it
		s /= pSurf->texinfo->image->width;

		// SCG[6/3/99]: Find the t coord for this vert
		t = DotProduct( vec, pSurf->texinfo->t ) + pSurf->texinfo->t_offset;

		// SCG[6/3/99]: normalize it
		t /= pSurf->texinfo->image->height;

		total = total + vec;
		poly->verts[i] = vec;
		poly->s1[i] = s;
		poly->t1[i] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct( vec, pSurf->texinfo->s ) + pSurf->texinfo->s_offset;
		s -= pSurf->texturemins[0];
		s += pSurf->light_s * 16;
		s += 8;
		s /= 2048;	// SCG[6/9/99]: 2048 = BLOCK_WIDTH * 16
	
		t = DotProduct( vec, pSurf->texinfo->t ) + pSurf->texinfo->t_offset;
		t -= pSurf->texturemins[1];
		t += pSurf->light_t * 16;
		t += 8;
		t /= 2048;	// SCG[6/9/99]: 2048 = BLOCK_WIDTH * 16

		poly->s2[i] = s;		
		poly->t2[i] = t;
	}
}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
void GL_CreateSurfaceLightmap (msurface_t *surf)
{
	int		smax, tmax;
	byte	*base;

	if (surf->flags & (SURF_DRAWSKY))
		return;

	// SCG[6/3/99]: lightmaps are 1/16th of the surface size.
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
	{
		LM_UploadBlock( false );
		LM_InitBlock();
		if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
		{
			ri.Sys_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax );
		}
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	base = gl_lms.lightmap_buffer;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState( surf );
	R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
}


/*
==================
GL_BeginBuildingLightmaps

==================
*/
void GL_BeginBuildingLightmaps (model_t *m)
{
	static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
	int				i;
	unsigned		dummy[128*128];

	memset( gl_lms.allocated, 0, sizeof(gl_lms.allocated) );

	r_framecount = 1;		// no dlightcache

	GL_EnableMultitexture( true );
	GL_SelectTexture( GL_TEXTURE1_SGIS );

	/*
	** setup the base lightstyles so the lightmaps won't have to be regenerated
	** the first time they're seen
	*/
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		lightstyles[i].rgb.x = 1;
		lightstyles[i].rgb.y = 1;
		lightstyles[i].rgb.z = 1;
		lightstyles[i].white = 3;
	}
	r_newrefdef.lightstyles = lightstyles;

	if (!gl_state.lightmap_textures)
	{
		gl_state.lightmap_textures	= TEXNUM_LIGHTMAPS;
//		gl_state.lightmap_textures	= gl_state.texture_extension_number;
//		gl_state.texture_extension_number = gl_state.lightmap_textures + MAX_LIGHTMAPS;
	}

	gl_lms.current_lightmap_texture = 1;

	/*
	** if mono lightmaps are enabled and we want to use alpha
	** blending (a,1-a) then we're likely running on a 3DLabs
	** Permedia2.  In a perfect world we'd use a GL_ALPHA lightmap
	** in order to conserve space and maximize bandwidth, however 
	** this isn't a perfect world.
	**
	** So we have to use alpha lightmaps, but stored in GL_RGBA format,
	** which means we only get 1/16th the color resolution we should when
	** using alpha lightmaps.  If we find another board that supports
	** only alpha lightmaps but that can at least support the GL_ALPHA
	** format then we should change this code to use real alpha maps.
	*/
	if ( toupper( gl_monolightmap->string[0] ) == 'A' )
	{
		gl_lms.internal_format = gl_tex_alpha_format;// SCG[5/20/99]: Quake2 V3.19 addition
	}
	/*
	** try to do hacked colored lighting with a blended texture
	*/
	else if ( toupper( gl_monolightmap->string[0] ) == 'C' )
	{
		gl_lms.internal_format = gl_tex_alpha_format;// SCG[5/20/99]: Quake2 V3.19 addition
	}
	else if ( toupper( gl_monolightmap->string[0] ) == 'I' )
	{
		gl_lms.internal_format = GL_INTENSITY8;
	}
	else if ( toupper( gl_monolightmap->string[0] ) == 'L' ) 
	{
		gl_lms.internal_format = GL_LUMINANCE8;
	}
	else
	{
		gl_lms.internal_format = gl_tex_solid_format;// SCG[5/20/99]: Quake2 V3.19 addition
	}

	/*
	** initialize the dynamic lightmap texture
	*/
	GL_Bind( gl_state.lightmap_textures + 0 );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexImage2D( GL_TEXTURE_2D, 
				   0, 
				   gl_lms.internal_format,
				   BLOCK_WIDTH, BLOCK_HEIGHT, 
				   0, 
				   GL_LIGHTMAP_FORMAT, 
				   GL_UNSIGNED_BYTE, 
				   dummy );

}

/*
=======================
GL_EndBuildingLightmaps
=======================
*/
void GL_EndBuildingLightmaps (void)
{
	LM_UploadBlock( false );
	GL_EnableMultitexture( false );
}

