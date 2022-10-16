#include <assert.h>
#include "r_local.h"

#define AFFINE_SPANLET_SIZE      16
#define AFFINE_SPANLET_SIZE_BITS 4

typedef struct
{
	uint16     *pbase, *pdest;
	short	 *pz;
	fixed16_t s, t;
	fixed16_t sstep, tstep;
	int       izi, izistep, izistep_times_2;
	int       spancount;
	unsigned  u, v;
} spanletvars_t;

spanletvars_t s_spanletvars;

int r_polyblendcolor;

static espan_t	*s_polygon_spans;

polydesc_t	r_polydesc;

msurface_t *r_alpha_surfaces;

extern "C" int *r_turb_turb;

static int		clip_current;
CVector		r_clip_verts[2][MAXWORKINGVERTS+2];
float		r_clip_s[2][MAXWORKINGVERTS+2];
float		r_clip_t[2][MAXWORKINGVERTS+2];

static int		s_minindex, s_maxindex;

static void R_DrawPoly( qboolean iswater );

msurface_t		*currentsurface;
static CVector			transformed_modelorg;
static CVector			local_modelorg;
static CVector			world_transformed_modelorg;

qboolean	isinbmodel;

void R_DrawSpanletOpaque_f( void );
void R_DrawSpanletTurbulentStipple50_f( void );
void R_DrawSpanletTurbulentBlended50_f( void );
void R_DrawSpanlet50_f( void );
void R_DrawSpanletConstant50_f( void );
void R_DrawSpanlet50Stipple_f( void );

/*
================
R_RotateVector
================
*/
void R_RotateVector (CVector &vec)
{
	CVector	tvec;

	tvec = vec;

	vec[0] = DotProduct (entity_rotation[0], tvec);
	vec[1] = DotProduct (entity_rotation[1], tvec);
	vec[2] = DotProduct (entity_rotation[2], tvec);
}

/*
** R_DrawSpanletOpaque
*/
void R_DrawSpanletOpaque( void )
{
	unsigned btemp;

	do
	{
		unsigned ts, tt;

		ts = s_spanletvars.s >> 16;
		tt = s_spanletvars.t >> 16;

		btemp = *((uint16 *)s_spanletvars.pbase + (ts) + (tt) * cachewidth);
		if ( btemp != TRANSPARENT_COLOR )
		{
			if (*s_spanletvars.pz <= (s_spanletvars.izi >> 16))
			{
				*s_spanletvars.pz    = s_spanletvars.izi >> 16;
				*s_spanletvars.pdest = btemp;
			}
		}
		
		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
		s_spanletvars.s += s_spanletvars.sstep;
		s_spanletvars.t += s_spanletvars.tstep;
	} while (--s_spanletvars.spancount > 0);
}

/*
** R_DrawSpanletTurbulentStipple50
*/
void R_DrawSpanletTurbulentStipple50( void )
{
	unsigned btemp;
	int	     sturb, tturb;
	uint16  *pdest = (uint16 *)s_spanletvars.pdest;
	byte	*pbase;
	short   *pz    = s_spanletvars.pz;
	int      izi   = s_spanletvars.izi;
	
	pbase = (byte*) s_spanletvars.pbase;

	if ( s_spanletvars.v & 1 )
	{
		s_spanletvars.pdest += s_spanletvars.spancount;
		s_spanletvars.pz    += s_spanletvars.spancount;

		if ( s_spanletvars.spancount == AFFINE_SPANLET_SIZE )
			s_spanletvars.izi += s_spanletvars.izistep << AFFINE_SPANLET_SIZE_BITS;
		else
			s_spanletvars.izi += s_spanletvars.izistep * s_spanletvars.izistep;
		
		if ( s_spanletvars.u & 1 )
		{
			izi += s_spanletvars.izistep;
			s_spanletvars.s   += s_spanletvars.sstep;
			s_spanletvars.t   += s_spanletvars.tstep;

			pdest++;
			pz++;
			s_spanletvars.spancount--;
		}

		s_spanletvars.sstep   *= 2;
		s_spanletvars.tstep   *= 2;

		while ( s_spanletvars.spancount > 0 )
		{
			sturb = ((s_spanletvars.s + r_turb_turb[(s_spanletvars.t>>16)&(CYCLE-1)])>>16)&63;
			tturb = ((s_spanletvars.t + r_turb_turb[(s_spanletvars.s>>16)&(CYCLE-1)])>>16)&63;
			
			btemp = *( pbase + ( sturb ) + ( tturb << 6 ) );
			
			if ( *pz <= ( izi >> 16 ) )
				*pdest = ((r_polydesc.palette[btemp] & r_blendmask50) + (*pdest + r_blendmask50))>>1;
			
			izi               += s_spanletvars.izistep_times_2;
			s_spanletvars.s   += s_spanletvars.sstep;
			s_spanletvars.t   += s_spanletvars.tstep;
			
			pdest += 2;
			pz    += 2;
			
			s_spanletvars.spancount -= 2;
		}
	}
}



/*
** R_DrawSpanletTurbulentBlended50
*/
void R_DrawSpanletTurbulentBlended50( void )
{
	unsigned	btemp;
	byte		*pbase;
	int			sturb, tturb;

	pbase = (byte*) s_spanletvars.pbase;

	do
	{
		sturb = ((s_spanletvars.s + r_turb_turb[(s_spanletvars.t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((s_spanletvars.t + r_turb_turb[(s_spanletvars.s>>16)&(CYCLE-1)])>>16)&63;

		btemp = *( pbase + ( sturb ) + ( tturb << 6 ) );

		if ( *s_spanletvars.pz <= ( s_spanletvars.izi >> 16 ) )
				*s_spanletvars.pdest = ((r_polydesc.palette[btemp] & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50))>>1;

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
		s_spanletvars.s += s_spanletvars.sstep;
		s_spanletvars.t += s_spanletvars.tstep;

	} while ( --s_spanletvars.spancount > 0 );
}



/*
** R_DrawSpanlet50
*/
void R_DrawSpanlet50( void )
{
	unsigned btemp;

	do
	{
		unsigned ts, tt;

		ts = s_spanletvars.s >> 16;
		tt = s_spanletvars.t >> 16;

		btemp = *(s_spanletvars.pbase + (ts) + (tt) * cachewidth);

		if ( btemp != TRANSPARENT_COLOR )
		{
			if (*s_spanletvars.pz <= (s_spanletvars.izi >> 16))
			{
				*s_spanletvars.pdest = ((btemp & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50))>>1;
			}
		}

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
		s_spanletvars.s += s_spanletvars.sstep;
		s_spanletvars.t += s_spanletvars.tstep;
	} while (--s_spanletvars.spancount > 0);
}

void R_DrawSpanletConstant50( void )
{
	do
	{
		if (*s_spanletvars.pz <= (s_spanletvars.izi >> 16))
		{
				*s_spanletvars.pdest = ((r_polyblendcolor & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50))>>1;
		}

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
	} while (--s_spanletvars.spancount > 0);
}



/*
** R_DrawSpanlet50Stipple
*/
void R_DrawSpanlet50Stipple( void )
{
	unsigned btemp;
	uint16  *pdest = (uint16 *)s_spanletvars.pdest;
	short   *pz    = s_spanletvars.pz;
	int      izi   = s_spanletvars.izi;
	
	if ( r_polydesc.stipple_parity ^ ( s_spanletvars.v & 1 ) )
	{
		s_spanletvars.pdest += s_spanletvars.spancount;
		s_spanletvars.pz    += s_spanletvars.spancount;

		if ( s_spanletvars.spancount == AFFINE_SPANLET_SIZE )
			s_spanletvars.izi += s_spanletvars.izistep << AFFINE_SPANLET_SIZE_BITS;
		else
			s_spanletvars.izi += s_spanletvars.izistep * s_spanletvars.izistep;
		
		if ( r_polydesc.stipple_parity ^ ( s_spanletvars.u & 1 ) )
		{
			izi += s_spanletvars.izistep;
			s_spanletvars.s   += s_spanletvars.sstep;
			s_spanletvars.t   += s_spanletvars.tstep;

			pdest++;
			pz++;
			s_spanletvars.spancount--;
		}

		s_spanletvars.sstep *= 2;
		s_spanletvars.tstep *= 2;

		while ( s_spanletvars.spancount > 0 )
		{
			unsigned s = s_spanletvars.s >> 16;
			unsigned t = s_spanletvars.t >> 16;

			btemp = *( s_spanletvars.pbase + ( s ) + ( t * cachewidth ) );
			
			if ( btemp != TRANSPARENT_COLOR )
			{
				if ( *pz <= ( izi >> 16 ) )
					*pdest = ((btemp * r_blendmask50) + (*pdest & r_blendmask50))>>1;
			}
			
			izi               += s_spanletvars.izistep_times_2;
			s_spanletvars.s   += s_spanletvars.sstep;
			s_spanletvars.t   += s_spanletvars.tstep;
			
			pdest += 2;
			pz    += 2;
			
			s_spanletvars.spancount -= 2;
		}
	}
}


/*
** R_ClipPolyFace
**
** Clips the winding at clip_verts[clip_current] and changes clip_current
** Throws out the back side
*/
int R_ClipPolyFace (int nump, clipplane_t *pclipplane)
{
	int		i, outcount;
	float	dists[MAXWORKINGVERTS+3];
	float	frac, clipdist;
	CVector pclipnormal;
	CVector	*in, *instep, *outstep, *vert2;

	clipdist = pclipplane->dist;
	pclipnormal = pclipplane->normal;
	
// calc dists
	if (clip_current)
	{
		// clip vertex array 1 vertex 0
		in = &r_clip_verts[1][0];

		// clip vertex array 0 vertex 0
		outstep = &r_clip_verts[0][0];
		clip_current = 0;
	}
	else
	{
		// clip vertex array 0 vertex 0
		in = &r_clip_verts[0][0];

		// clip vertex array 1 vertex 0
		outstep = &r_clip_verts[1][0];

		clip_current = 1;
	}
	
	instep = in;
	for ( i = 0; i < nump; i++ ){
		dists[i] = DotProduct( instep[i], pclipnormal ) - clipdist;
	}
	
// handle wraparound case
	dists[nump] = dists[0];
	instep[nump] = *in;

// clip the winding
	instep = in;
	outcount = 0;

	for ( i = 0; i < nump; i++, instep++){
		if (dists[i] >= 0)
		{
			*outstep = *instep;
			outstep++;
			outcount++;
		}

		if (dists[i] == 0 || dists[i+1] == 0)
			continue;

		if ( (dists[i] > 0) == (dists[i+1] > 0) )
			continue;
			
	// split it into a new vertex
		frac = dists[i] / (dists[i] - dists[i+1]);
			
		vert2 = &instep[1];
		
		outstep[0] = instep[0] + frac*(vert2[0] - instep[0]);
		outstep[1] = instep[1] + frac*(vert2[1] - instep[1]);
		outstep[2] = instep[2] + frac*(vert2[2] - instep[2]);

		outstep++;
		outcount++;
	}	
	
	return outcount;
}

/*
** R_PolygonDrawSpans
*/
void R_PolygonDrawSpans(espan_t *pspan, qboolean iswater )
{
	int			count;
	fixed16_t	snext, tnext;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivzspanletstepu, tdivzspanletstepu, zispanletstepu;

	s_spanletvars.pbase = (uint16*)cacheblock;

	if ( iswater )
		r_turb_turb = sintable + ((int)(r_newrefdef.time*SPEED)&(CYCLE-1));

	sdivzspanletstepu = d_sdivzstepu * AFFINE_SPANLET_SIZE;
	tdivzspanletstepu = d_tdivzstepu * AFFINE_SPANLET_SIZE;
	zispanletstepu = d_zistepu * AFFINE_SPANLET_SIZE;

// we count on FP exceptions being turned off to avoid range problems
	s_spanletvars.izistep = (int)(d_zistepu * 0x8000 * 0x10000);
	s_spanletvars.izistep_times_2 = s_spanletvars.izistep * 2;

	s_spanletvars.pz = 0;

	do
	{
		s_spanletvars.pdest   = (uint16*)((byte *)d_viewbuffer + d_scantable[pspan->v] + (pspan->u * BYTES_PER_PIXEL));
		s_spanletvars.pz      = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
		s_spanletvars.u       = pspan->u;
		s_spanletvars.v       = pspan->v;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;

		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		s_spanletvars.izi = (int)(zi * 0x8000 * 0x10000);

		s_spanletvars.s = (int)(sdivz * z) + sadjust;
		s_spanletvars.t = (int)(tdivz * z) + tadjust;

		if ( !iswater )
		{
			if (s_spanletvars.s > bbextents)
				s_spanletvars.s = bbextents;
			else if (s_spanletvars.s < 0)
				s_spanletvars.s = 0;

			if (s_spanletvars.t > bbextentt)
				s_spanletvars.t = bbextentt;
			else if (s_spanletvars.t < 0)
				s_spanletvars.t = 0;
		}

		do
		{
		// calculate s and t at the far end of the span
			if (count >= AFFINE_SPANLET_SIZE )
				s_spanletvars.spancount = AFFINE_SPANLET_SIZE;
			else
				s_spanletvars.spancount = count;

			count -= s_spanletvars.spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivzspanletstepu;
				tdivz += tdivzspanletstepu;
				zi += zispanletstepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				tnext = (int)(tdivz * z) + tadjust;

				if ( !iswater )
				{
					if (snext > bbextents)
						snext = bbextents;
					else if (snext < AFFINE_SPANLET_SIZE)
						snext = AFFINE_SPANLET_SIZE;	// prevent round-off error on <0 steps from
									//  from causing overstepping & running off the
									//  edge of the texture

					if (tnext > bbextentt)
						tnext = bbextentt;
					else if (tnext < AFFINE_SPANLET_SIZE)
						tnext = AFFINE_SPANLET_SIZE;	// guard against round-off error on <0 steps
				}

				s_spanletvars.sstep = (snext - s_spanletvars.s) >> AFFINE_SPANLET_SIZE_BITS;
				s_spanletvars.tstep = (tnext - s_spanletvars.t) >> AFFINE_SPANLET_SIZE_BITS;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(s_spanletvars.spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				tnext = (int)(tdivz * z) + tadjust;

				if ( !iswater )
				{
					if (snext > bbextents)
						snext = bbextents;
					else if (snext < AFFINE_SPANLET_SIZE)
						snext = AFFINE_SPANLET_SIZE;	// prevent round-off error on <0 steps from
									//  from causing overstepping & running off the
									//  edge of the texture

					if (tnext > bbextentt)
						tnext = bbextentt;
					else if (tnext < AFFINE_SPANLET_SIZE)
						tnext = AFFINE_SPANLET_SIZE;	// guard against round-off error on <0 steps
				}

				if (s_spanletvars.spancount > 1)
				{
					s_spanletvars.sstep = (snext - s_spanletvars.s) / (s_spanletvars.spancount - 1);
					s_spanletvars.tstep = (tnext - s_spanletvars.t) / (s_spanletvars.spancount - 1);
				}
			}

			if ( iswater )
			{
				s_spanletvars.s = s_spanletvars.s & ((CYCLE<<16)-1);
				s_spanletvars.t = s_spanletvars.t & ((CYCLE<<16)-1);
			}

			r_polydesc.drawspanlet();

			s_spanletvars.s = snext;
			s_spanletvars.t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

/*
**
** R_PolygonScanLeftEdge
**
** Goes through the polygon and scans the left edge, filling in 
** screen coordinate data for the spans
*/
void R_PolygonScanLeftEdge (void)
{
	int			i, v, itop, ibottom, lmaxindex;
	emitpoint_t	*pvert, *pnext;
	espan_t		*pspan;
	float		du, dv, vtop, vbottom, slope;
	fixed16_t	u, u_step;

	pspan = s_polygon_spans;
	i = s_minindex;
	if (i == 0)
		i = r_polydesc.nump;

	lmaxindex = s_maxindex;
	if (lmaxindex == 0)
		lmaxindex = r_polydesc.nump;

	vtop = ceil (r_polydesc.pverts[i].v);

	do
	{
		pvert = &r_polydesc.pverts[i];
		pnext = pvert - 1;

		vbottom = ceil (pnext->v);

		if (vtop < vbottom)
		{
			du = pnext->u - pvert->u;
			dv = pnext->v - pvert->v;

			slope = du / dv;
			u_step = (int)(slope * 0x10000);
		// adjust u to ceil the integer portion
			u = (int)((pvert->u + (slope * (vtop - pvert->v))) * 0x10000) +
					(0x10000 - 1);
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v=itop ; v<ibottom ; v++)
			{
				pspan->u = u >> 16;
				pspan->v = v;
				u += u_step;
				pspan++;
			}
		}

		vtop = vbottom;

		i--;
		if (i == 0)
			i = r_polydesc.nump;

	} while (i != lmaxindex);
}

/*
** R_PolygonScanRightEdge
**
** Goes through the polygon and scans the right edge, filling in
** count values.
*/
void R_PolygonScanRightEdge (void)
{
	int			i, v, itop, ibottom;
	emitpoint_t	*pvert, *pnext;
	espan_t		*pspan;
	float		du, dv, vtop, vbottom, slope, uvert, unext, vvert, vnext;
	fixed16_t	u, u_step;

	pspan = s_polygon_spans;
	i = s_minindex;

	vvert = r_polydesc.pverts[i].v;
	if (vvert < r_refdef.fvrecty_adj)
		vvert = r_refdef.fvrecty_adj;
	if (vvert > r_refdef.fvrectbottom_adj)
		vvert = r_refdef.fvrectbottom_adj;

	vtop = ceil (vvert);

	do
	{
		pvert = &r_polydesc.pverts[i];
		pnext = pvert + 1;

		vnext = pnext->v;
		if (vnext < r_refdef.fvrecty_adj)
			vnext = r_refdef.fvrecty_adj;
		if (vnext > r_refdef.fvrectbottom_adj)
			vnext = r_refdef.fvrectbottom_adj;

		vbottom = ceil (vnext);

		if (vtop < vbottom)
		{
			uvert = pvert->u;
			if (uvert < r_refdef.fvrectx_adj)
				uvert = r_refdef.fvrectx_adj;
			if (uvert > r_refdef.fvrectright_adj)
				uvert = r_refdef.fvrectright_adj;

			unext = pnext->u;
			if (unext < r_refdef.fvrectx_adj)
				unext = r_refdef.fvrectx_adj;
			if (unext > r_refdef.fvrectright_adj)
				unext = r_refdef.fvrectright_adj;

			du = unext - uvert;
			dv = vnext - vvert;
			slope = du / dv;
			u_step = (int)(slope * 0x10000);
		// adjust u to ceil the integer portion
			u = (int)((uvert + (slope * (vtop - vvert))) * 0x10000) +
					(0x10000 - 1);
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v=itop ; v<ibottom ; v++)
			{
				pspan->count = (u >> 16) - pspan->u;
				u += u_step;
				pspan++;
			}
		}

		vtop = vbottom;
		vvert = vnext;

		i++;
		if (i == r_polydesc.nump)
			i = 0;

	} while (i != s_maxindex);

	pspan->count = DS_SPAN_LIST_END;	// mark the end of the span list 
}

void R_PolySetSpanRoutine (float alpha, qboolean isturbulent, qboolean textured)
{
	if( r_newrefdef.foginfo.active ){
		if( !textured ){
			r_polydesc.drawspanlet = R_DrawSpanletConstant50_f;
		}
		else{
			if( alpha == 1 ){
				r_polydesc.drawspanlet = R_DrawSpanletOpaque_f;
			}
			else{
				if ( sw_stipplealpha->value ){
					if( isturbulent ){
						r_polydesc.drawspanlet = R_DrawSpanletTurbulentStipple50_f;
					}
					else{
						r_polydesc.drawspanlet = R_DrawSpanlet50Stipple_f;
					}
				}
				else{
					if( isturbulent ){
						r_polydesc.drawspanlet = R_DrawSpanletTurbulentBlended50_f;
					}
					else{
						r_polydesc.drawspanlet = R_DrawSpanlet50_f;
					}
				}
			}
		}
	}
	else
	{
		if( !textured ){
			r_polydesc.drawspanlet = R_DrawSpanletConstant50;
		}
		else{
			if( alpha == 1 ){
				r_polydesc.drawspanlet = R_DrawSpanletOpaque;
			}
			else{
				if( sw_stipplealpha->value ){
					if( isturbulent ){
						r_polydesc.drawspanlet = R_DrawSpanletTurbulentStipple50;
					}
					else{
						r_polydesc.drawspanlet = R_DrawSpanlet50Stipple;
					}
				}
				else{
					if( isturbulent ){
							r_polydesc.drawspanlet = R_DrawSpanletTurbulentBlended50;
					}
					else{
						r_polydesc.drawspanlet = R_DrawSpanlet50;
					}
				}
			}
		}
	}
}

/*
** R_ClipAndDrawPoly
*/
void R_ClipAndDrawPoly(float alpha, qboolean isturbulent, qboolean textured)
{
	emitpoint_t	outverts[MAXWORKINGVERTS+3], *pout;
	float		*ps, *pt;
	int			i, nump;
	float		scale;
	CVector		transformed, local, origin;

	R_PolySetSpanRoutine (alpha, isturbulent, textured);

	// clip to the frustum in worldspace
	nump = r_polydesc.nump;
	clip_current = 0;

	for (i=0 ; i<4 ; i++)
	{
		nump = R_ClipPolyFace (nump, &view_clipplanes[i]);
		if (nump < 3)
			return;
		if (nump > MAXWORKINGVERTS)
			ri.Sys_Error(ERR_DROP, "R_ClipAndDrawPoly: too many points: %d", nump );
	}

// transform vertices into viewspace and project
	ps = &r_clip_s[clip_current][0];
	pt = &r_clip_t[clip_current][0];

	if (currentsurface)
	{
		if (currentsurface->entity)
			origin = modelorg;
		else
			origin = r_origin;
	}
	else
		origin = r_origin;

	for (i=0 ; i<nump ; i++)
	{
		local = r_clip_verts[clip_current][i] - origin;
		TransformVector (local, transformed);

		if (transformed[2] < NEAR_CLIP)
			transformed[2] = NEAR_CLIP;

		pout = &outverts[i];
		pout->zi = 1.0 / transformed[2];

		pout->s = *ps++;
		pout->t = *pt++;
				
		scale = xscale * pout->zi;
		pout->u = (xcenter + scale * transformed[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale * transformed[1]);
	}

// draw it
	r_polydesc.nump = nump;
	r_polydesc.pverts = outverts;

	R_DrawPoly( isturbulent );
}

/*
** R_BuildPolygonFromSurface
*/
void R_BuildPolygonFromSurface(void)
{
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
	int			vertpage;
	CVector		vec;
	CVector		*pverts;

	float       tmins[2] = { 0, 0 };

	r_polydesc.nump = 0;

	// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = currentsurface->numedges;
	vertpage = 0;

	pverts = r_clip_verts[0];

	for (i = 0; i < lnumverts; i++ ){
		lindex = currentmodel->surfedges[currentsurface->firstedge + i];

		if (lindex > 0){
			r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		}
		else{
			r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}

		r_clip_verts[0][i] = vec;

	}

	r_polydesc.vright = currentsurface->texinfo->s;		// right
	r_polydesc.vup = currentsurface->texinfo->t;			// up
	r_polydesc.vpn = currentsurface->plane->normal;			// forward
	r_polydesc.viewer_position = r_origin;

	if ( currentsurface->flags & SURF_PLANEBACK ){
		r_polydesc.vpn = vec3_origin - r_polydesc.vpn; // invert forward
	}

	if ( currentsurface->texinfo->flags & SURF_WARP ){
		r_polydesc.pixels       = currentsurface->texinfo->image->pixels[0];
		r_polydesc.pixel_width  = currentsurface->texinfo->image->width;
		r_polydesc.pixel_height = currentsurface->texinfo->image->height;
		r_polydesc.palette		= currentsurface->texinfo->image->palette;
	}
	else{
		surfcache_t *scache;

		scache = D_CacheSurface( currentsurface, 0 );

		r_polydesc.pixels       = (byte*)scache->data;
		r_polydesc.pixel_width  = scache->width;
		r_polydesc.pixel_height = scache->height;

		tmins[0] = currentsurface->texturemins[0];
		tmins[1] = currentsurface->texturemins[1];
	}

	r_polydesc.dist = DotProduct( r_polydesc.vpn, pverts[0] );
	r_polydesc.s_offset = currentsurface->texinfo->s_offset - tmins[0];
	r_polydesc.t_offset = currentsurface->texinfo->t_offset - tmins[1];

	// scrolling texture addition
	if (currentsurface->texinfo->flags & SURF_FLOWING){
		r_polydesc.s_offset += -128 * ( (r_newrefdef.time*0.25) - (int)(r_newrefdef.time*0.25) );
	}
	r_polydesc.nump = lnumverts;
}

/*
** R_PolygonCalculateGradients
*/
void R_PolygonCalculateGradients (void)
{
	CVector		p_normal, p_saxis, p_taxis;
	float		distinv;
	CVector		p_temp1;
	float		t;
	float		mipscale;
	int			miplevel;
	
	TransformVector (r_polydesc.vpn, p_normal);	
	TransformVector (r_polydesc.vright, p_saxis);
	TransformVector (r_polydesc.vup, p_taxis);

// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = (r_polydesc.pixel_width << 16) - 1;
	bbextentt = (r_polydesc.pixel_height << 16) - 1;

	if (currentsurface){
		if (currentsurface->entity){
			miplevel = 0;
			mipscale = 1.0 / (float)(1 << miplevel);

			distinv = 1.0 / (r_polydesc.dist - DotProduct (modelorg, r_polydesc.vpn));

			t = xscaleinv * mipscale;
			d_sdivzstepu = p_saxis[0] * t;
			d_tdivzstepu = p_taxis[0] * t;

			t = yscaleinv * mipscale;
			d_sdivzstepv = -p_saxis[1] * t;
			d_tdivzstepv = -p_taxis[1] * t;

			d_sdivzorigin = p_saxis[2] * mipscale - xcenter * d_sdivzstepu -
					ycenter * d_sdivzstepv;
			d_tdivzorigin = p_taxis[2] * mipscale - xcenter * d_tdivzstepu -
					ycenter * d_tdivzstepv;

			//VectorScale (transformed_modelorg, mipscale, p_temp1);
			p_temp1 = transformed_modelorg * mipscale;

			d_zistepu	=	p_normal[0] * xscaleinv * distinv;
			d_zistepv	=	-p_normal[1] * yscaleinv * distinv;
			d_ziorigin	=	p_normal[2] * distinv - xcenter * d_zistepu - ycenter * d_zistepv;

			t = 0x10000 * mipscale;
			sadjust = ((fixed16_t)(DotProduct (p_temp1, p_saxis) * 0x10000 + 0.5)) - ((currentsurface->texturemins[0] << 16) >> miplevel)
					+ currentsurface->texinfo->s_offset*t;
			tadjust = ((fixed16_t)(DotProduct (p_temp1, p_taxis) * 0x10000 + 0.5)) - ((currentsurface->texturemins[1] << 16) >> miplevel)
					+ currentsurface->texinfo->t_offset*t;

			return;
		}
	}

	distinv = 1.0 / (-(DotProduct (r_polydesc.viewer_position, r_polydesc.vpn)) + r_polydesc.dist );

	d_sdivzstepu  =  p_saxis[0] * xscaleinv;
	d_tdivzstepu  =  p_taxis[0] * xscaleinv;

	d_tdivzstepv  = -p_taxis[1] * yscaleinv;
	d_sdivzstepv  = -p_saxis[1] * yscaleinv;

	d_sdivzorigin =  p_saxis[2] - xcenter * d_sdivzstepu - ycenter * d_sdivzstepv;
	d_tdivzorigin =  p_taxis[2] - xcenter * d_tdivzstepu - ycenter * d_tdivzstepv;

	d_zistepu =   p_normal[0] * xscaleinv * distinv;
	d_zistepv =  -p_normal[1] * yscaleinv * distinv;
	d_ziorigin =  p_normal[2] * distinv - xcenter * d_zistepu - ycenter * d_zistepv;

	sadjust = (fixed16_t) ( ( DotProduct( r_polydesc.viewer_position, r_polydesc.vright) + r_polydesc.s_offset ) * 0x10000 );
	tadjust = (fixed16_t) ( ( DotProduct( r_polydesc.viewer_position, r_polydesc.vup) + r_polydesc.t_offset ) * 0x10000 );
}

/*
** R_DrawPoly
**
** Polygon drawing function.  Uses the polygon described in r_polydesc
** to calculate edges and gradients, then renders the resultant spans.
**
** This should NOT be called externally since it doesn't do clipping!
*/
static void R_DrawPoly( qboolean iswater ){
	int			i, nump;
	float		ymin, ymax;
	emitpoint_t	*pverts;
	espan_t	spans[MAXHEIGHT+1];

	s_polygon_spans = spans;

// find the top and bottom vertices, and make sure there's at least one scan to
// draw
	ymin = 999999.9;
	ymax = -999999.9;
	pverts = r_polydesc.pverts;

	for ( i = 0; i < r_polydesc.nump; i++){
		if (pverts->v < ymin){
			ymin = pverts->v;
			s_minindex = i;
		}

		if (pverts->v > ymax){
			ymax = pverts->v;
			s_maxindex = i;
		}

		pverts++;
	}

	ymin = ceil (ymin);
	ymax = ceil (ymax);

	if (ymin >= ymax)
		return;		// doesn't cross any scans at all

	cachewidth = r_polydesc.pixel_width;
	cacheblock = (uint16*)r_polydesc.pixels;

// copy the first vertex to the last vertex, so we don't have to deal with
// wrapping
	nump = r_polydesc.nump;
	pverts = r_polydesc.pverts;
	pverts[nump] = pverts[0];

	R_PolygonCalculateGradients ();

	R_PolygonScanLeftEdge ();
	R_PolygonScanRightEdge ();

	R_PolygonDrawSpans( s_polygon_spans, iswater );

}

/*
** R_DrawAlphaSurfaces
*/
void R_DrawAlphaSurfaces( void )
{
	msurface_t	*s = r_alpha_surfaces, *test;
	entity_t	*oldentity;
	int			firsttime;

//	modelorg[0] = -r_origin[0];
//	modelorg[1] = -r_origin[1];
//	modelorg[2] = -r_origin[2];

	currentmodel = r_worldmodel;

	oldentity = currententity;

	currentsurface = NULL;

	test = s;
	firsttime = 1;

	while (s)
	{
		if (!firsttime)
		{
			if (s == test)
				return;
		}

		currentsurface = s;

		if (currentsurface->entity)
		{
			currententity = currentsurface->entity;

			modelorg = r_origin - currententity->origin;
		}
		else
			currententity = NULL;

		R_BuildPolygonFromSurface();

		if (currententity)
		{
			TransformVector (modelorg, transformed_modelorg);

			R_RotateBmodel ();
		}
		r_polyblendcolor = 100;

		if (s->texinfo->flags & SURF_TRANS66)
			R_ClipAndDrawPoly (0.60f, (s->texinfo->flags & SURF_WARP) != 0, true);
		else if (s->texinfo->flags & SURF_MIDTEXTURE)
			R_ClipAndDrawPoly (1.0f, (s->texinfo->flags & SURF_WARP) != 0, true);
		else
			R_ClipAndDrawPoly (0.30f, (s->texinfo->flags & SURF_WARP) != 0, true);

		if (currententity)
		{
			vpn = base_vpn;
			vup = base_vup;
			vright = base_vright;
			modelorg = r_origin;
			R_TransformFrustum ();
		}

		s = s->nextalphasurface;
		firsttime = 0;
	}

	r_alpha_surfaces = currentsurface = NULL;

	currententity = oldentity;
}

/*
** R_IMFlatShadedQuad
*/
void R_IMFlatShadedQuad( CVector &a, CVector &b, CVector &c, CVector &d, int color, float alpha )
{
	CVector	s0, s1, point;
	float	dot;

	r_polydesc.nump = 4;
	r_polydesc.viewer_position = r_origin;

	// compute normal for poly
	s0 = d - c;
	s1 = c - b;
	CrossProduct( s0, s1, r_polydesc.vpn );
	r_polydesc.vpn.Normalize();
	
	// get the distance from viewpoint to poly
	point = r_origin - c;

	// get the dot product 
	dot = DotProduct (r_polydesc.vpn, point);

	if (dot < 0)
	{
		// swap points		  
		r_clip_verts[0][0] = d;
		r_clip_verts[0][1] = c;
		r_clip_verts[0][2] = b;
		r_clip_verts[0][3] = a;

		// invert the normal
		r_polydesc.vpn = vec3_origin - r_polydesc.vpn;
	}
	else
	{
		r_clip_verts[0][0] = a;
		r_clip_verts[0][1] = b;
		r_clip_verts[0][2] = c;
		r_clip_verts[0][3] = d;
	}

	r_clip_s[0][0] = 0;
	r_clip_s[0][1] = 0;
	r_clip_s[0][2] = 0;
	r_clip_s[0][3] = 0;

	r_clip_t[0][0] = 0;
	r_clip_t[0][1] = 0;
	r_clip_t[0][2] = 0;
	r_clip_t[0][3] = 0;

	r_polydesc.dist = DotProduct( r_polydesc.vpn, r_clip_verts[0][0] );

	r_polyblendcolor = color;

	R_ClipAndDrawPoly( alpha, false, false );
}

void R_TexturedQuad( CVector &a, CVector &b, CVector &c, CVector &d, image_t *image, float alpha)
{
	CVector		s0, s1, point;
	float		dot;

	// compute normal for poly
	s0 = d - c;
	s1 = c - b;
	CrossProduct( s0, s1, r_polydesc.vpn );
	r_polydesc.vpn.Normalize();

	// get the distance from viewpoint to poly
	point = r_origin - c;

	// get the dot product 
	dot = DotProduct (r_polydesc.vpn, point);

	// if looking at the back of the poly
	if (dot < 0)
	{
		// swap points		  
		r_clip_verts[0][0] = d;
		r_clip_verts[0][1] = c;
		r_clip_verts[0][2] = b;
		r_clip_verts[0][3] = a;

		// invert the normal
		r_polydesc.vpn = vec3_origin - r_polydesc.vpn;
	}
	else
	{
		r_clip_verts[0][0] = a;
		r_clip_verts[0][1] = b;
		r_clip_verts[0][2] = c;
		r_clip_verts[0][3] = d;
	}

	r_polydesc.viewer_position = modelorg;

	r_clip_s[0][0] = 0;
	r_clip_s[0][1] = 0;
	r_clip_s[0][2] = 0;
	r_clip_s[0][3] = 0;

	r_clip_t[0][0] = 0;
	r_clip_t[0][1] = 0;
	r_clip_t[0][2] = 0;
	r_clip_t[0][3] = 0;

	r_polydesc.nump			= 4;
	r_polydesc.pixels       = image->pixels[0];
	r_polydesc.pixel_width	= image->width;
	r_polydesc.pixel_height	= image->height;
	r_polydesc.s_offset		= ( r_polydesc.pixel_width  >> 1);
	r_polydesc.t_offset		= ( r_polydesc.pixel_height >> 1);
	r_polydesc.dist			= 0;

	R_ClipAndDrawPoly( alpha, false, true );
}
