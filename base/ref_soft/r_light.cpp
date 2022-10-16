// r_light.c

#include "r_local.h"

int	r_dlightframecount;

#define	DLIGHT_CUTOFF	64

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
	msurface_t	*surf;
	int			i;
	
	if (node->contents != -1)
		return;

	splitplane = node->plane;
	dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;
	
	i=light->intensity;
	if(i<0)
		i=-i;

	if (dist > i)
	{
		R_MarkLights (light, bit, node->children[0]);
		return;
	}
	if (dist < -i)
	{
		R_MarkLights (light, bit, node->children[1]);
		return;
	}
		
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (model_t *model)
{
	int		i;
	dlight_t	*l;

	if( r_dynamiclights->value == 0 )
		return;

	r_dlightframecount = r_framecount;
	for (i=0, l = r_newrefdef.dlights ; i<r_newrefdef.num_dlights ; i++, l++)
	{
		R_MarkLights ( l, 1<<i, model->nodes + model->firstnode);
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

CVector	pointcolor;
mplane_t		*lightplane;		// used as shadow plane
CVector			lightspot;

int RecursiveLightPoint (mnode_t *node, CVector &start, CVector &end)
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	CVector		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	int			maps;
	int			r;

	if (node->contents != -1)
		return -1;		// didn't hit anything
	
// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end);
	
	frac = front / (front-back);
	mid = start + (end-start) * frac;

// go down front side	
	r = RecursiveLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something
		
	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing
		
// check for impact on this node
	lightspot = mid;
	lightplane = plane;

	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags&(SURF_DRAWTURB|SURF_DRAWSKY)) 
			continue;	// no lightmaps

		tex = surf->texinfo;
		
		s = DotProduct (mid, tex->s) + tex->s_offset;
		t = DotProduct (mid, tex->t) + tex->t_offset;

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		pointcolor = vec3_origin;
		if (lightmap)
		{
			CVector scale;

			lightmap += 3 * (dt * ((surf->extents[0]>>4)+1) + ds);

			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
					maps++)
			{
				scale.x = sw_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb.x;
				scale.y = sw_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb.y;
				scale.z = sw_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb.z;

				pointcolor.x += lightmap[0] * scale.x * ONEDIV255;
				pointcolor.y += lightmap[1] * scale.y * ONEDIV255;
				pointcolor.z += lightmap[2] * scale.z * ONEDIV255;

				lightmap += 3*((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);

			}
		}
		
		return 1;
	}

// go down back side
	return RecursiveLightPoint (node->children[!side], mid, end);
}

/*
===============
R_LightPoint
===============
*/
void R_LightPoint (CVector &p, CVector &color)
{
	CVector		end;
	float		r;
	int			lnum;
	dlight_t	*dl;
	float		light;
	CVector		dist;
	float		add;
	
	if (!r_worldmodel->lightdata)
	{
		color.x = color.y = color.z = 1.0;
		return;
	}
	
	end.x = p.x;
	end.y = p.y;
	end.z = p.z - 2048;
	
	r = RecursiveLightPoint (r_worldmodel->nodes, p, end);

	if (r == -1)
	{
		color = vec3_origin;
	}
	else
	{
		color = pointcolor;
	}

	//
	// add dynamic lights
	//
	light = 0;
	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		dl = &r_newrefdef.dlights[lnum];
		dist = currententity->origin - dl->origin;
		add = dl->intensity - dist.Length();
		add *= (1.0/256);
		if (add > 0)
		{
			VectorMA (color, dl->color, add, color);
		}
	}

	color = color * sw_modulate->value;
}

//===================================================================



//unsigned		blocklights[1024];	// allow some very large lightmaps
uint16		blocklights[4096];	// allow some very large lightmaps

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights (void)
{
	msurface_t *surf;
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	CVector		impact, local;
	int			s, t;
//	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	uint16		*pfBL;
	float		fsacc, ftacc;

	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[lnum];
		rad = dl->intensity;
		dist = DotProduct (dl->origin, surf->plane->normal) -
				surf->plane->dist;
		rad -= fabs(dist);

//		minlight = 32;		// dl->minlight;
		minlight = DLIGHT_CUTOFF;		// dl->minlight;

		if (rad < minlight)
			continue;

		minlight = rad - minlight;

		impact = dl->origin - surf->plane->normal * dist;

		local.x = DotProduct( impact, tex->s ) + tex->s_offset - surf->texturemins[0];
		local.y = DotProduct( impact, tex->t ) + tex->t_offset - surf->texturemins[1];

//		local[0] -= surf->texturemins[0];
//		local[1] -= surf->texturemins[1];
		
		pfBL = blocklights;
		for (t = 0, ftacc = 0 ; t<tmax ; t++, ftacc += 16)
		{
			td = local.y - ftacc;
			if ( td < 0 )
				td = -td;

			for ( s=0, fsacc = 0 ; s<smax ; s++, fsacc += 16, pfBL += 4)
			{
				sd = Q_ftol( local.x - fsacc );

				if ( sd < 0 )
					sd = -sd;

				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);

				if ( dist < minlight )
				{
					pfBL[0] += ( rad - dist ) * dl->color.x;
					pfBL[1] += ( rad - dist ) * dl->color.y;
					pfBL[2] += ( rad - dist ) * dl->color.z;
				}
			}
		}
	}
}

/*
===============
R_BuildColorLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
void R_BuildColorLightMap (void)
{
	int			smax, tmax;
	int			i, size;
	byte		*lightmap;
	float		scale[4];
	int			maps, nummaps;
	msurface_t	*surf;
	int			r, g, b, max;

	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;

// clear to no light
	for( i = 0; i < size; i++ ){
		blocklights[i] = 0L;
	}

	if ( r_fullbright->value || !r_worldmodel->lightdata ){
		for( i = 0; i < size; i++ )
		{
			blocklights[i*4+0] = 128;
			blocklights[i*4+1] = 128;
			blocklights[i*4+2] = 128;
			blocklights[i*4+3] = 0;
		}
		return;
	}

//	if ( surf->texinfo->flags & (SURF_MIDTEXTURE|SURF_FULLBRIGHT|SURF_TRANS33|SURF_TRANS66|SURF_WARP) ){
	if ( surf->texinfo->flags & (SURF_FULLBRIGHT|SURF_WARP) ){
		for( i = 0; i < size; i++ ){
			blocklights[i*4+0] = 64;
			blocklights[i*4+1] = 64;
			blocklights[i*4+2] = 64;
			blocklights[i*4+3] = 0;
		}
		return;
	}

	// count the # of maps
	for ( nummaps = 0 ; nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255 ; nummaps++)
		;

// add all the lightmaps
	lightmap = surf->samples;

	if (lightmap){
		if( nummaps == 1){
			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++){
				for (i=0 ; i<3 ; i++){
					scale[i] = sw_modulate->value * r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];
				}

				if( scale[0] == 1.0f && scale[1] == 1.0f && scale[2] == 1.0f){
					for( i = 0; i < size; i++ ){
						blocklights[i*4+0] = lightmap[i*3+0];
						blocklights[i*4+1] = lightmap[i*3+1];
						blocklights[i*4+2] = lightmap[i*3+2];
						blocklights[i*4+3] = 0;
					}
				}
				else{
					for( i = 0; i < size; i++ ){
						blocklights[i*4+0] = lightmap[i*3+0] * scale[0];
						blocklights[i*4+1] = lightmap[i*3+1] * scale[1];
						blocklights[i*4+2] = lightmap[i*3+2] * scale[2];
						blocklights[i*4+3] = 0;
					}
				}
				lightmap += size * 3;	// skip to next lightmap
			}
		}
		else{
			memset( blocklights, 0, sizeof( blocklights[0] ) * 1024 );

			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++){
				for (i=0 ; i<3 ; i++){
					scale[i] = sw_modulate->value * r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];
				}

				if( scale[0] == 1.0f && scale[1] == 1.0f && scale[2] == 1.0f){
					for( i = 0; i < size; i++ ){
						blocklights[i*4+0] += lightmap[i*3+0];
						blocklights[i*4+1] += lightmap[i*3+1];
						blocklights[i*4+2] += lightmap[i*3+2];
						blocklights[i*4+3] = 0;
					}
				}
				else{
					for( i = 0; i < size; i++ ){
						blocklights[i*4+0] += lightmap[i*3+0] * scale[0];
						blocklights[i*4+1] += lightmap[i*3+1] * scale[1];
						blocklights[i*4+2] += lightmap[i*3+2] * scale[2];
						blocklights[i*4+3] = 0;
					}
				}
				lightmap += size * 3;	// skip to next lightmap
			}
		}
	}

	for( i = 0; i < size; i++ ){
		r = blocklights[i*4+0];
		g = blocklights[i*4+1];
		b = blocklights[i*4+2];

		if( r < 0 )
			r = 0;
		if( g < 0 )
			g = 0;
		if( b < 0 )
			b = 0;

		/*
		** determine the brightest of the three color components
		*/
		if (r > g)
			max = r;
		else
			max = g;
		if (b > max)
			max = b;

		if( max > 255 ){
			float t = 255.0F / max;

			r = r*t;
			g = g*t;
			b = b*t;
		}

		blocklights[i*4+0] = r;
		blocklights[i*4+1] = g;
		blocklights[i*4+2] = b;
	}
	
// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights ();
}

void R_BuildLightMap (void)
{
	int			smax, tmax;
	int			t;
	int			i, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	msurface_t	*surf;

	R_BuildColorLightMap();
	return;

	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;

	if (r_fullbright->value || !r_worldmodel->lightdata || surf->texinfo->flags & SURF_FULLBRIGHT)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to no light
	for (i=0 ; i<size ; i++)
		blocklights[i] = 0;


// add all the lightmaps
	lightmap = surf->samples;
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights ();

// bound, invert, and shift
	for (i=0 ; i<size ; i++)
	{
		t = (int)blocklights[i];
		if (t < 0)
			t = 0;
		t = (255*256 - t) >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
}

