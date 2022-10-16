// r_light.c

#include "gl_local.h"

int	r_dlightframecount;

#define	DLIGHT_CUTOFF	64

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

void R_RenderDlight (dlight_t *light)
{
	int		i;
	float	a;
	CVector	v;
	float	rad;

	rad = light->intensity * 0.35;

	v = light->origin - r_origin;

	qglBegin (GL_TRIANGLE_FAN);
	qglColor3f (light->color.x*0.2, light->color.y*0.2, light->color.z*0.2);

	v.x = light->origin.x - vpn.x*rad;
	v.y = light->origin.y - vpn.y*rad;
	v.z = light->origin.z - vpn.z*rad;

	qglVertex3fv (v);
	qglColor3f (0,0,0);

	for (i=16 ; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;

		v.x = light->origin.x + vright.x*cos(a)*rad + vup.x * sin(a) * rad;
		v.y = light->origin.y + vright.y*cos(a)*rad + vup.y * sin(a) * rad;
		v.z = light->origin.z + vright.z*cos(a)*rad + vup.z * sin(a) * rad;

		qglVertex3fv (v);
	}

	qglEnd ();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!gl_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	qglShadeModel (GL_SMOOTH);

	qglBlendFunc (GL_ONE, GL_ONE);
	GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );

	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
		R_RenderDlight (l);

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*pSurf;
	int			i;
	
	if (node->contents != -1)
		return;

	splitplane = node->plane;
	dist = DotProduct( light->origin, splitplane->normal ) - splitplane->dist;
	
	if( dist > light->intensity - DLIGHT_CUTOFF )
	{
		R_MarkLights( light, bit, node->children[0] );
		return;
	}
	if( dist < -light->intensity + DLIGHT_CUTOFF )
	{
		R_MarkLights( light, bit, node->children[1] );
		return;
	}
		
// mark the polygons
	pSurf = r_worldmodel->surfaces + node->firstsurface;
	for( i = 0; i < node->numsurfaces ; i++, pSurf++ )
	{
		if( pSurf->dlightframe != r_dlightframecount )
		{
			pSurf->dlightbits = 0;
			pSurf->dlightframe = r_dlightframecount;
		}
		pSurf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);

	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (gl_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
		R_MarkLights ( l, 1<<i, r_worldmodel->nodes );
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

CVector			pointcolor;
mplane_t		*lightplane;		// used as shadow plane
CVector			lightspot;

int RecursiveLightPoint (mnode_t *node, CVector start, CVector end, msurface_t *pPointSurf )
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	CVector		mid;
	msurface_t	*pSurf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	int			maps;
	int			r;

	if( node->contents != -1 )
	{
		return -1;		// didn't hit anything
	}
	
// calculate mid point
	plane = node->plane;

	switch( plane->type )
	{
	case PLANE_X:
		front = start.x - plane->dist;
		back = end.x - plane->dist;
		break;
	case PLANE_Y:
		front = start.y - plane->dist;
		back = end.y - plane->dist;
		break;
	case PLANE_Z:
		front = start.z - plane->dist;
		back = end.z - plane->dist;
		break;
	default:
		front = DotProduct( start, plane->normal ) - plane->dist;
		back = DotProduct( end, plane->normal ) - plane->dist;
		break;
	}

//	front = DotProduct( start, plane->normal ) - plane->dist;
//	back = DotProduct( end, plane->normal ) - plane->dist;
	side = front < 0;
	
	if( (back < 0 ) == side )
	{
		return RecursiveLightPoint( node->children[side], start, end, pPointSurf );
	}
	
	frac = front / ( front - back );
	mid = start + ( end - start ) * frac;
	
// go down front side	
	r = RecursiveLightPoint( node->children[side], start, mid, pPointSurf );
	if( r >= 0 )
	{
		return r;		// hit something
	}
		
	if ( ( back < 0 ) == side )
	{
		return -1;		// didn't hit anuthing
	}
		
// check for impact on this node
	lightspot = mid;

	lightplane = plane;

	pSurf = r_worldmodel->surfaces + node->firstsurface;

	for( i = 0; i < node->numsurfaces ; i++, pSurf++ )
	{
		if( pSurf->flags & ( SURF_DRAWSKY | SURF_NODRAW ) )
		{
			continue;	// no lightmaps
		}
		if( pSurf->texinfo->flags == ( SURF_FOGPLANE | SURF_FULLBRIGHT ) )
		{
			continue;	// no lightmaps
		}
		
		tex = pSurf->texinfo;
		
		s = DotProduct( mid, tex->s ) + tex->s_offset;
		t = DotProduct( mid, tex->t ) + tex->t_offset;

		if (s < pSurf->texturemins[0] || t < pSurf->texturemins[1])
			continue;
		
		ds = s - pSurf->texturemins[0];
		dt = t - pSurf->texturemins[1];
		
		if( ds > pSurf->extents[0] || dt > pSurf->extents[1] )
		{
			continue;
		}

		if( !pSurf->samples )
		{
			return 0;
		}

		ds >>= 4;
		dt >>= 4;

		lightmap = pSurf->samples;
		pointcolor = vec3_origin;

		if( lightmap )
		{
			CVector scale;

			lightmap += 3 * ( dt * ( ( pSurf->extents[0] >> 4 ) + 1 ) + ds );

			for( maps = 0; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++ )
			{
				scale.x = gl_modulate->value * r_newrefdef.lightstyles[pSurf->styles[maps]].rgb.x;
				scale.y = gl_modulate->value * r_newrefdef.lightstyles[pSurf->styles[maps]].rgb.y;
				scale.z = gl_modulate->value * r_newrefdef.lightstyles[pSurf->styles[maps]].rgb.z;

				pointcolor.x += lightmap[0] * scale.x * ( 1.0 / 255 );
				pointcolor.y += lightmap[1] * scale.y * ( 1.0 / 255 );
				pointcolor.z += lightmap[2] * scale.z * ( 1.0 / 255 );

				lightmap += 3 * ( ( pSurf->extents[0] >> 4 ) + 1) * ( ( pSurf->extents[1] >> 4 ) + 1 );
			}
		}

		if( pPointSurf )
		{
			*pPointSurf = *pSurf;
		}

		return 1;
	}

// go down back side
	return RecursiveLightPoint( node->children[!side], mid, end, pPointSurf );
}

/*
===============
R_LightPoint
===============
*/
// Ash --   returns true if there's directional light info returned
//          color = ambient light value
//          pDColor = total dynamic light value
//          pDLoc = location of strongest dynamic light 
bool R_LightPoint (CVector &p, CVector &color, CVector *pDLoc, msurface_t *pPointSurf )
{
	CVector		end;
	float		r;
	int			lnum;
	dlight_t	*dl;
	CVector		dist;
	float		add, bestAdd;
    int         bestDLight = 0;
    bool        fDLight=false;
	
	if( ( r_worldmodel == NULL ) || 
		( r_worldmodel->lightdata == NULL ) || 
		( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		color.x = color.y = color.z = 1.0;
		return fDLight;
	}
	
	end.x = p.x;
	end.y = p.y;
	end.z = p.z - 2048;
	
	r = RecursiveLightPoint (r_worldmodel->nodes, p, end, pPointSurf );
	
	if (r == -1)
	{
		color = vec3_origin;
	}
	else
	{
		color = pointcolor;
	}

	//
	// handle dynamic lights
	//

    if (pDLoc)
    {
        // determine location of strongest dynamic light
        fDLight = false;
        bestAdd = 0.0f;
        dl = r_newrefdef.dlights;
    	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++, dl++)
    	{
    		dist = currententity->origin - dl->origin;
    		add = dl->intensity - dist.Length();
    		add *= (1.0/256);
    		if (add > 0)
    		{
    			VectorMA (color, dl->color, add, color);
                
                fDLight = true;
                
                if (add>bestAdd)
                {
                    bestAdd = add;
                    bestDLight = lnum;
                }
    		}
    	}
    
    	color = color * gl_modulate->value;
        
        if (fDLight)
            *pDLoc = r_newrefdef.dlights[bestDLight].origin;
    }
    else
    {
        dl = r_newrefdef.dlights;
    	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++, dl++)
    	{
    		dist = currententity->origin - dl->origin;
    		add = dl->intensity - dist.Length();
    		add *= (1.0/256);
    		if (add > 0)
    		{
    			VectorMA (color, dl->color, add, color);
    		}
    	}
    
    	color = color * gl_modulate->value;
    }

    return fDLight;

}


//===================================================================

//static float s_blocklights[34*34*3];
static float s_blocklights[130*130*3];
/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights( msurface_t *pSurf )
{
	int			lnum;
	int			sd, td;
	float		fdist, frad, fminlight;
	CVector		impact, local;
	int			s, t;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	float		*pfBL;
	float		fsacc, ftacc;

	smax = ( pSurf->extents[0] >> 4 ) + 1;
	tmax = ( pSurf->extents[1] >> 4 ) + 1;
	tex = pSurf->texinfo;

	for( lnum = 0; lnum < r_newrefdef.num_dlights; lnum++ )
	{
		if ( !( pSurf->dlightbits & ( 1 << lnum ) ) )
		{
			continue;		// not lit by this light
		}

		dl = &r_newrefdef.dlights[lnum];
		frad = dl->intensity;
		fdist = DotProduct( dl->origin, pSurf->plane->normal ) - pSurf->plane->dist;
		frad -= fabs(fdist);
		// rad is now the highest intensity on the plane

		fminlight = DLIGHT_CUTOFF;	// FIXME: make configurable?
		if( frad < fminlight )
		{
			continue;
		}
		fminlight = frad - fminlight;

		impact = dl->origin - pSurf->plane->normal * fdist;

		local.x = DotProduct( impact, tex->s ) + tex->s_offset - pSurf->texturemins[0];
		local.y = DotProduct( impact, tex->t ) + tex->t_offset - pSurf->texturemins[1];

		pfBL = s_blocklights;
		for( t = 0, ftacc = 0; t < tmax; t++, ftacc += 16 )
		{
			td = local.y - ftacc;
			if ( td < 0 )
			{
				td = -td;
			}

			for( s = 0, fsacc = 0; s < smax; s++, fsacc += 16, pfBL += 3 )
			{
				sd = Q_ftol( local.x - fsacc );

				if( sd < 0 )
				{
					sd = -sd;
				}

				if( sd > td )
				{
					fdist = sd + ( td >> 1 );
				}
				else
				{
					fdist = td + ( sd >> 1 );
				}

				if( fdist < fminlight )
				{
					pfBL[0] += ( frad - fdist ) * dl->color[0];
					pfBL[1] += ( frad - fdist ) * dl->color[1];
					pfBL[2] += ( frad - fdist ) * dl->color[2];
				}
			}
		}
	}
}


/*
** R_SetCacheState
*/
void R_SetCacheState( msurface_t *pSurf )
{
	int maps;

	for( maps = 0; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++)
	{
		pSurf->cached_light[maps] = r_newrefdef.lightstyles[pSurf->styles[maps]].white;
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
void R_BuildLightMap( msurface_t *pSurf, byte *dest, int stride )
{
	int				smax, tmax;
	int				r, g, b, a, max;
	int				i, j, size;
	byte			*lightmap;
	float			scale[4];
	int				nummaps;
	float			*bl;
	lightstyle_t	*style;
	int				monolightmap;

	if ( pSurf->texinfo->flags & ( SURF_SKY | SURF_FULLBRIGHT ) || ( pSurf->texinfo->flags == SURF_FOGPLANE ) )
		ri.Sys_Error (ERR_DROP, "R_BuildLightMap called for non-lit surface");

	smax = ( pSurf->extents[0] >> 4 ) + 1;
	tmax = ( pSurf->extents[1] >> 4 ) + 1;
	size = smax*tmax;

	if( size > ( sizeof( s_blocklights ) >> 4) )
		ri.Sys_Error( ERR_DROP, "Bad s_blocklights size" );

// set to full bright if no light data
	if( !pSurf->samples )
	{
		int maps;

		for( i = 0; i < size * 3; i++ )
		{
			s_blocklights[i] = 255;
		}

		for( maps = 0; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++ )
		{
			style = &r_newrefdef.lightstyles[pSurf->styles[maps]];
		}
		goto store;
	}

	// count the # of maps
	for( nummaps = 0; nummaps < MAXLIGHTMAPS && pSurf->styles[nummaps] != 255; nummaps++)
		;

	lightmap = pSurf->samples;

	// add all the lightmaps
	if( nummaps == 1 )
	{
		int maps;

		for( maps = 0; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++ )
		{
			bl = s_blocklights;

			for(i=0; i < 3; i++ )
			{
				scale[i] = gl_modulate->value*r_newrefdef.lightstyles[pSurf->styles[maps]].rgb[i];
			}

			if ( scale[0] == 1.0F && scale[1] == 1.0F && scale[2] == 1.0F )
			{
				for( i = 0; i < size; i++, bl += 3 )
				{
					bl[0] = lightmap[i * 3 + 0];
					bl[1] = lightmap[i * 3 + 1];
					bl[2] = lightmap[i * 3 + 2];
				}
			}
			else
			{
				for( i = 0; i < size ; i++, bl += 3 )
				{
					bl[0] = lightmap[i * 3 + 0] * scale[0];
					bl[1] = lightmap[i * 3 + 1] * scale[1];
					bl[2] = lightmap[i * 3 + 2] * scale[2];
				}
			}
			lightmap += size * 3;		// skip to next lightmap
		}
	}
	else
	{
		int maps;

		memset( s_blocklights, 0, sizeof( s_blocklights[0] ) * size * 3 );

		for( maps = 0 ; maps < MAXLIGHTMAPS && pSurf->styles[maps] != 255; maps++)
		{
			bl = s_blocklights;

			for(i=0; i < 3; i++ )
			{
				scale[i] = gl_modulate->value*r_newrefdef.lightstyles[pSurf->styles[maps]].rgb[i];
			}

			if ( scale[0] == 1.0F && scale[1] == 1.0F && scale[2] == 1.0F )
			{
				for( i = 0; i < size; i++, bl += 3 )
				{
					bl[0] += lightmap[i * 3 + 0];
					bl[1] += lightmap[i * 3 + 1];
					bl[2] += lightmap[i * 3 + 2];
				}
			}
			else
			{
				for( i = 0; i < size; i++, bl += 3 )
				{
					bl[0] += lightmap[i * 3 + 0] * scale[0];
					bl[1] += lightmap[i * 3 + 1] * scale[1];
					bl[2] += lightmap[i * 3 + 2] * scale[2];
				}
			}
			lightmap += size * 3;		// skip to next lightmap
		}
	}

// add all the dynamic lights
	if( pSurf->dlightframe == r_framecount )
	{
		R_AddDynamicLights( pSurf );
	}

// put into texture format
store:
	stride -= ( smax << 2 );
	bl = s_blocklights;

	monolightmap = gl_monolightmap->string[0];

	if ( monolightmap == '0' )
	{
		for( i = 0; i < tmax; i++, dest += stride )
		{
			for( j = 0; j < smax; j++ )
			{
				r = Q_ftol( bl[0] );
				g = Q_ftol( bl[1] );
				b = Q_ftol( bl[2] );

				// catch negative lights
				if( r < 0 )
				{
					r = 0;
				}
				if( g < 0 )
				{
				 	g = 0;
				}
				if( b < 0 )
				{
					b = 0;
				}

				/*
				** determine the brightest of the three color components
				*/
				if( r > g )
				{
					max = r;
				}
				else
				{
					max = g;
				}

				if( b > max )
				{
					max = b;
				}

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
				if( max > 255 )
				{
					float t = 255.0F / max;

					r = r * t;
					g = g * t;
					b = b * t;
					a = a * t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
			}
		}
	}
	else
	{
		for( i = 0; i < tmax; i++, dest += stride )
		{
			for( j = 0; j < smax; j++ )
			{
				r = Q_ftol( bl[0] );
				g = Q_ftol( bl[1] );
				b = Q_ftol( bl[2] );

				// catch negative lights
				if( r < 0 )
				{
					r = 0;
				}
				if( g < 0 )
				{
					g = 0;
				}
				if( b < 0 )
				{
					b = 0;
				}

				/*
				** determine the brightest of the three color components
				*/
				if( r > g )
				{
					max = r;
				}
				else
				{
					max = g;
				}

				if( b > max )
				{
					max = b;
				}

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
				if( max > 255 )
				{
					float t = 255.0F / max;

					r = r * t;
					g = g * t;
					b = b * t;
					a = a * t;
				}

				/*
				** So if we are doing alpha lightmaps we need to set the R, G, and B
				** components to 0 and we need to set alpha to 1-alpha.
				*/
				switch ( monolightmap )
				{
				case 'L':
				case 'I':
					r = a;
					g = b = 0;
					break;
				case 'C':
					// try faking colored lighting
					a = 255 - ( ( r + g + b ) / 3 );
					r *= a/255.0;
					g *= a/255.0;
					b *= a/255.0;
					break;
				case 'A':
				default:
					r = g = b = 0;
					a = 255 - a;
					break;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
			}
		}
	}
}

