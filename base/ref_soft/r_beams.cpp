///////////////////////////////////////////////////////////////////////////////
//	r_beam.c
//
//	Beam technology!!
//
//	Nelno, 9/23/98
///////////////////////////////////////////////////////////////////////////////

#include	"r_local.h"
#include	"r_beams.h"

image_t		*r_beamTextures [BEAM_TEX_NUMTEXTURES];

extern polydesc_t	r_polydesc;

#define FLAT_SHADED 1

///////////////////////////////////////////////////////////////////////////////
//	R_DrawSegmentedBeam_NoFade
//
//	draws a segmented beam that fades to 0 alpha as it goes towards endpoint
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSegmentedBeam_NoFade (beam_t *beam, float alpha, float radius)
{
	CVector		segToViewNormal, beamRight;
	CVector		beamPoint0, beamPoint1, beamPoint2, beamPoint3;
	float		dist;
	beamSeg_t	*beamSeg;

	beamSeg = beam->firstSeg;

	///////////////////////////////////////////////////////////////////////////
	//	draw starting segment
	///////////////////////////////////////////////////////////////////////////

	segToViewNormal = beamSeg->segStart - r_newrefdef.vieworg;
	//	get vector perpendicular to view and beam
	CrossProduct (segToViewNormal, beamSeg->segNormal, beamRight);

	//	make sure we didn't get an undefined cross product
	dist = beamRight.Length();
	if (dist < 0.5)
	{
		//	do nothing for now
		return;
	}

	//	normalize it and flip it
	//VectorScale (beamRight, -1.0 / dist, beamRight);
	beamRight = beamRight * (-1.0/dist);
	//VectorScale (beamRight, radius, beamRight);
	beamRight = beamRight * radius;

	//	starting points
	beamPoint3 = beamSeg->segEnd + beamRight;
	beamPoint2 = beamSeg->segEnd - beamRight;

	//	ending points are already in beamPoint0 and 1 for start of loop
	beamPoint0 = beamSeg->segStart + beamRight;
	beamPoint1 = beamSeg->segStart - beamRight;

	r_polydesc.stipple_parity = 1;

	R_TexturedQuad ( beamPoint0, beamPoint1, beamPoint2, beamPoint3, r_notexture_mip, 1.0);

	//	start looping from second segment
	beamSeg = beamSeg->nextSeg;

	while (beamSeg)
	{
		///////////////////////////////////////////////////////////////////////
		//	start each segment with end points of last segment
		///////////////////////////////////////////////////////////////////////
		beamPoint0 = beamPoint3;
		beamPoint1 = beamPoint2;

		//	get normal from viewer to beam end
		segToViewNormal = beamSeg->segStart - r_newrefdef.vieworg;
		//	get vector perpendicular to view and beam
		CrossProduct (segToViewNormal, beamSeg->segNormal, beamRight);

		dist = beamRight.Length();
		if (dist < 0.5)
		{
			//	do nothing for now
		}

		//	normalize it and flip it
		//VectorScale (beamRight, -1.0 / dist, beamRight);
		beamRight = beamRight * (-1.0/dist);
		//VectorScale (beamRight, radius, beamRight);
		beamRight = beamRight * radius;

		//	third and forth points are just the direction vectors plus the oldorigin
		beamPoint3 = beamSeg->segEnd + beamRight;
		beamPoint2 = beamSeg->segEnd - beamRight;

#ifndef FLAT_SHADED
		R_TexturedQuad (beamPoint0, beamPoint1, beamPoint2, beamPoint3, r_notexture_mip, 1.0);
#else
		R_IMFlatShadedQuad( beamPoint0, beamPoint1, beamPoint2, beamPoint3, 128, 1.0 );
#endif

		beamSeg = beamSeg->nextSeg;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	R_DrawSegmentedBeam
//
//	Two-tri version of the laser code.  Always orients to view and remains
//	virtually the same size at all angles.  
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSegmentedBeam (beam_t *beam)
{
	float		alpha = 1.0F, radius;
	
	alpha = beam->alpha;
	radius = beam->startRadius;

	beam_DrawSegmentedBeam_NoFade (beam, alpha, radius);

}

///////////////////////////////////////////////////////////////////////////////
//	beam_DrawSingleSegment
//
//	Two-tri version of the laser code.  Always orients to view and remains
//	virtually the same size at all angles.  
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSingleSegment (beam_t *beam)
{
//	int			index;
//	CVector		rgbColor;
	CVector		segToViewNormal, beamRight_startRadius, beamRight_endRadius;
	CVector		beamPoint0, beamPoint1, beamPoint2, beamPoint3;
	float		dist;
	beamSeg_t	*beamSeg;

	beamSeg = beam->firstSeg;

	///////////////////////////////////////////////////////////////////////////
	//	draw the segment
	///////////////////////////////////////////////////////////////////////////

	segToViewNormal = beamSeg->segStart - r_newrefdef.vieworg;
	//	get vector perpendicular to view and beam
	CrossProduct (segToViewNormal, beamSeg->segNormal, beamRight_startRadius);

	//	make sure we didn't get an undefined cross product
	dist = beamRight_startRadius.Length();
	if (dist < 0.5)
	{
		return;
	}

	//	normalize it and flip it
	beamRight_startRadius = beamRight_startRadius * (-1.0/dist);
	beamRight_endRadius = beamRight_startRadius;
	beamRight_startRadius = beamRight_startRadius * beam->startRadius;
	beamRight_endRadius = beamRight_endRadius * beam->endRadius;

	beamPoint3 = beamSeg->segEnd + beamRight_endRadius;
	beamPoint2 = beamSeg->segEnd - beamRight_endRadius;

	//	ending points are already in beamPoint0 and 1 for start of loop
	beamPoint0 = beamSeg->segStart + beamRight_startRadius;
	beamPoint1 = beamSeg->segStart - beamRight_startRadius;

	r_polydesc.viewer_position = beamSeg->segNormal;

	r_polydesc.stipple_parity = 1;

#ifndef FLAT_SHADED
	R_TexturedQuad (beamPoint0, beamPoint1, beamPoint2, beamPoint3, r_notexture_mip, 1.0);
#else
	R_IMFlatShadedQuad( beamPoint0, beamPoint1, beamPoint2, beamPoint3, 128, 1.0);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	beam_DrawBeamList
//
///////////////////////////////////////////////////////////////////////////////

void	R_DrawBeamList (void)
{
	int			i;
	beamList_t	*beamList;
	beam_t		*beam;

	beamList = r_newrefdef.beamList;

	for (i = 0; i < r_newrefdef.beamList->numBeams; i++)
	{
		beam = &beamList->beams [i];
		if (beam->numSegs > 1)
			beam_DrawSegmentedBeam (beam);
		else if (beam->numSegs == 1)
			beam_DrawSingleSegment (beam);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	beam_DrawBeam
//
//	draws a single, entity based beam using two tris.  Does not use the beam
//	list, but gets it's info from an entity
//
//	FIXME:	get rid of entity based beams altogether!
///////////////////////////////////////////////////////////////////////////////

void beam_DrawBeam (entity_t *e, image_t *lasertexture)
{
	float		alpha = 1.0F;
	float		frame;
	
	CVector		beamNormal, beamXYNormal, beamRight;
	CVector		beamPoint0, beamPoint1, beamPoint2, beamPoint3;
	float		dist;

	// mike: added this so ripgun laser length could be modified
	// render_scale[0] == length
	// mins[] is the forward vector
	if (e->flags & RF_BEAM_MOVING)
	{
	  e->oldorigin.x = e->origin.x - e->mins.x * e->render_scale.x;
	  e->oldorigin.y = e->origin.y - e->mins.y * e->render_scale.y;
	  e->oldorigin.z = e->origin.z - e->mins.z * e->render_scale.z;
	}

	if (e->flags & RF_TRANSLUCENT)
	{
		if( e->alpha >= 0.66 )
			alpha = 0.66;
		else
			alpha = 0.33;
	}
	else
		alpha = 1;

	if (e->frame < 0)             // -frame is fractional, 1000 = 1  THIS IS FUCKED, but, i can get fracs from an int
		frame = -(float)e->frame / 1000.0;
	else
		frame=e->frame;            // non-neg is normal little frame ...

	beamNormal = e->oldorigin - e->origin;
	beamXYNormal = e->origin - r_newrefdef.vieworg;

	//	get vector perpendicular to view and beam
	CrossProduct (beamXYNormal, beamNormal, beamRight);

	dist = beamRight.Length();
	if (dist < 1.001)
		return;

	//	normalize it and flip it
	//VectorScale (beamRight, -1.0 / dist, beamRight);
	beamRight = beamRight * (-1.0/dist);

	//VectorScale (beamRight, frame, beamRight);
	beamRight = beamRight * frame;

	//	third and forth points are just the direction vections plus the oldorigin
	beamPoint3 = beamRight + e->oldorigin;
	beamPoint2 = e->oldorigin - beamRight;

	beamPoint0 = beamRight + e->origin;
	beamPoint1 = e->origin - beamRight;

	r_polydesc.viewer_position = beamNormal;

	r_polydesc.stipple_parity = 1;

#ifndef FLAT_SHADED
//	R_TexturedQuad (beamPoint0, beamPoint1, beamPoint2, beamPoint3, lasertexture, 1.0 );
	R_TexturedQuad (beamPoint0, beamPoint1, beamPoint2, beamPoint3, r_notexture_mip, 1.0 );
#else
	R_IMFlatShadedQuad( beamPoint0, beamPoint1, beamPoint2, beamPoint3, 128, 1.0 );
#endif
}

