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

extern "C" int				*r_turb_turb;
extern int				r_polyblendcolor;
extern espan_t			*s_polygon_spans;
extern polydesc_t		r_polydesc;
extern spanletvars_t	s_spanletvars;

unsigned int		surface_fogz;

#define FOG_NEAR	35.0
#define FOG_FAR		1024.0

/*
** R_DrawSpanletOpaque
*/
void R_DrawSpanletOpaque_f( void )
{
	unsigned	btemp;

	fogz = s_spanletvars.izi >> 16;				
	surface_fogz = fogz;		
	if (surface_fogz > 255)
		surface_fogz = 255;

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
				*s_spanletvars.pz    = s_spanletvars.izi >> 16;
//				*s_spanletvars.pdest = btemp;
				*s_spanletvars.pdest = ((d_fogtable[surface_fogz] & r_blendmask50) + (btemp & r_blendmask50)) >> 1;
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
** R_DrawSpanletTurbulentStipple50_f
*/
void R_DrawSpanletTurbulentStipple50_f( void )
{
	unsigned btemp;
	int	     sturb, tturb;
	uint16  *pdest = (uint16 *)s_spanletvars.pdest;
	byte	*pbase;
	short   *pz    = s_spanletvars.pz;
	int      izi   = s_spanletvars.izi;
	
	fogz = s_spanletvars.izi >> 16;		// fogz / 65535
	surface_fogz = fogz * 0x8000 * 0x10000;
	surface_fogz >>= (int) r_alias_fogz->value;
	if (surface_fogz > 31)
		surface_fogz = 31;

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
			
			btemp = *( s_spanletvars.pbase + ( sturb ) + ( tturb << 6 ) );
			
			if ( *pz <= ( izi >> 16 ) )
				*pdest = ((r_polydesc.palette[btemp] & r_blendmask50) + (*pdest & r_blendmask50)) >> 1;
			
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
** R_DrawSpanletTurbulentBlended50_f
*/
void R_DrawSpanletTurbulentBlended50_f( void )
{
	unsigned	btemp;
	byte		*pbase;
	int			sturb, tturb;

	fogz = 0x10000 / ((float)s_spanletvars.izi);
	surface_fogz = (fogz * 0x8000 * 0x10000);
	surface_fogz >>= (int) r_alias_fogz->value;
	if (surface_fogz > 31)
		surface_fogz = 31;

	pbase = (byte*) s_spanletvars.pbase;

	do
	{
		sturb = ((s_spanletvars.s + r_turb_turb[(s_spanletvars.t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((s_spanletvars.t + r_turb_turb[(s_spanletvars.s>>16)&(CYCLE-1)])>>16)&63;

		btemp = *( pbase + ( sturb ) + ( tturb << 6 ) );

		if ( *s_spanletvars.pz <= ( s_spanletvars.izi >> 16 ) )
		{
			*s_spanletvars.pdest = ((r_polydesc.palette[btemp] & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50)) >> 1;;
			*s_spanletvars.pdest = ((d_fogtable[31] & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50)) >> 1;
		}

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
		s_spanletvars.s += s_spanletvars.sstep;
		s_spanletvars.t += s_spanletvars.tstep;

	} while ( --s_spanletvars.spancount > 0 );
}



/*
** R_DrawSpanlet50_f
*/
void R_DrawSpanlet50_f( void )
{
	unsigned btemp;

	fogz = 0x10000 / ((float)s_spanletvars.izi);
	surface_fogz = (fogz * 0x8000 * 0x10000);
	surface_fogz >>= (int) r_alias_fogz->value;
	if (surface_fogz > 31)
		surface_fogz = 31;

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
				*s_spanletvars.pdest = ((btemp & d_fogtable[surface_fogz]) + (*s_spanletvars.pdest & r_blendmask50))>>1;
			}
		}

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
		s_spanletvars.s += s_spanletvars.sstep;
		s_spanletvars.t += s_spanletvars.tstep;
	} while (--s_spanletvars.spancount > 0);
}

void R_DrawSpanletConstant50_f( void )
{
	fogz = 0x10000 / ((float)s_spanletvars.izi);
	surface_fogz = (fogz * 0x8000 * 0x10000);
	surface_fogz >>= (int) r_alias_fogz->value;
	if (surface_fogz > 31)
		surface_fogz = 31;

	do
	{
		if (*s_spanletvars.pz <= (s_spanletvars.izi >> 16))
		{
			*s_spanletvars.pdest = ((r_polyblendcolor & r_blendmask50) + (*s_spanletvars.pdest & r_blendmask50)) >> 1;
		}

		s_spanletvars.izi += s_spanletvars.izistep;
		s_spanletvars.pdest++;
		s_spanletvars.pz++;
	} while (--s_spanletvars.spancount > 0);
}



/*
** R_DrawSpanlet50Stipple_f
*/
void R_DrawSpanlet50Stipple_f( void )
{
	unsigned btemp;
	uint16  *pdest = (uint16 *)s_spanletvars.pdest;
	short   *pz    = s_spanletvars.pz;
	int      izi   = s_spanletvars.izi;
	
	fogz = 0x10000 / ((float)s_spanletvars.izi);
	surface_fogz = (fogz * 0x8000 * 0x10000);
	surface_fogz >>= (int) r_alias_fogz->value;
	if (surface_fogz > 31)
		surface_fogz = 31;

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
				if ( *pz <= ( izi >> 16 ) ){
					*pdest = ((btemp & r_blendmask50) + (*pdest & r_blendmask50)) >> 1;
				}
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

