///////////////////////////////////////////////////////////////////////////////
//	gl_beam.c
//
//	Beam technology!!
//
//	Nelno, 9/23/98
///////////////////////////////////////////////////////////////////////////////

#include	"gl_local.h"
#include	"gl_beams.h"

image_t		*r_beamTextures [BEAM_TEX_NUMTEXTURES];

///////////////////////////////////////////////////////////////////////////////
//	R_DrawSegmentedBeam_AlphaFade
//
//	draws a segmented beam that fades to 0 alpha as it goes towards endpoint
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSegmentedBeam_AlphaFade (beam_t *beam, float radius, const CVector &rgbColor)
{
	CVector		segToViewNormal, beamRight, segVec;
	CVector		beamPoint0, beamPoint1, beamPoint2, beamPoint3;
	float		dist, pointAlpha, segLength;
	beamSeg_t	*beamSeg;

	beamSeg = beam->firstSeg;
	pointAlpha = beam->alpha;

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
	}

	//	normalize it and flip it
	beamRight.x = beamRight.x / -dist;
	beamRight.y = beamRight.y / -dist;
	beamRight.z = beamRight.z / -dist;
	beamRight = beamRight * radius;

	//	starting points
	beamPoint3 = beamSeg->segEnd + beamRight;
	beamPoint2 = beamSeg->segEnd - beamRight;

	//	ending points are already in beamPoint0 and 1 for start of loop
	beamPoint0 = beamSeg->segStart + beamRight;
	beamPoint1 = beamSeg->segStart - beamRight;

	//	draw
	qglTexCoord2f (0, 0);
	qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, pointAlpha);
	qglVertex3fv (beamPoint0);
	qglTexCoord2f (0, 1.0);
	qglVertex3fv (beamPoint1);

	//	decrement alpha based on length of this segment
	segVec = beamSeg->segEnd - beamSeg->segStart;
	segLength = segVec.Length();
	pointAlpha -= (segLength / beam->beamLength) * beam->alpha;

	qglTexCoord2f (1.0, 1.0);
	qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, pointAlpha);		
	qglVertex3fv (beamPoint2);
	qglTexCoord2f (1.0, 0);
	qglVertex3fv (beamPoint3);

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
		beamRight.x = beamRight.x / -dist;
		beamRight.y = beamRight.y / -dist;
		beamRight.z = beamRight.z / -dist;
		beamRight = beamRight * radius;

		//	third and forth points are just the direction vectors plus the oldorigin
		beamPoint3 = beamSeg->segEnd + beamRight;
		beamPoint2 = beamSeg->segEnd - beamRight;

		//	draw it
		//	could speed this up with triangle strips
		qglTexCoord2f (0, 0);
		qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, pointAlpha);
		qglVertex3fv (beamPoint0);
		qglTexCoord2f (0, 1.0);
		qglVertex3fv (beamPoint1);
		
		//	decrement alpha based on length of this segment
		segVec = beamSeg->segEnd - beamSeg->segStart;
		segLength = segVec.Length();
		pointAlpha -= (segLength / beam->beamLength) * beam->alpha;

		qglTexCoord2f (1.0, 1.0);
		qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, pointAlpha);		
		qglVertex3fv (beamPoint2);
		qglTexCoord2f (1.0, 0);
		qglVertex3fv (beamPoint3);

		beamSeg = beamSeg->nextSeg;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	R_DrawSegmentedBeam_NoFade
//
//	draws a segmented beam that fades to 0 alpha as it goes towards endpoint
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSegmentedBeam_NoFade (beam_t *beam, float radius, const CVector &rgbColor)
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
	}

	//	normalize it and flip it
	beamRight.x = beamRight.x / -dist;
	beamRight.y = beamRight.y / -dist;
	beamRight.z = beamRight.z / -dist;
	beamRight = beamRight * radius;

	//	starting points
	beamPoint3 = beamSeg->segEnd + beamRight;
	beamPoint2 = beamSeg->segEnd - beamRight;

	//	ending points are already in beamPoint0 and 1 for start of loop
	beamPoint0 = beamSeg->segStart + beamRight;
	beamPoint1 = beamSeg->segStart - beamRight;

	//	draw
	qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, beamSeg->alpha);
	qglTexCoord2f (0, 0);
	qglVertex3fv (beamPoint0);
	qglTexCoord2f (0, 1.0);
	qglVertex3fv (beamPoint1);
	qglTexCoord2f (1.0, 1.0);
	qglVertex3fv (beamPoint2);
	qglTexCoord2f (1.0, 0);
	qglVertex3fv (beamPoint3);

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
		beamRight.x = beamRight.x / -dist;
		beamRight.y = beamRight.y / -dist;
		beamRight.z = beamRight.z / -dist;
		beamRight = beamRight * radius;

		//	third and forth points are just the direction vectors plus the oldorigin
		beamPoint3 = beamSeg->segEnd + beamRight;
		beamPoint2 = beamSeg->segEnd - beamRight;

		//	draw it
		//	could speed this up with triangle strips
	    qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, beamSeg->alpha);
		qglTexCoord2f (0, 0);
		qglVertex3fv (beamPoint0);
		qglTexCoord2f (0, 1.0);
		qglVertex3fv (beamPoint1);
		qglTexCoord2f (1.0, 1.0);
		qglVertex3fv (beamPoint2);
		qglTexCoord2f (1.0, 0);
		qglVertex3fv (beamPoint3);

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
	float		radius;
	CVector		rgbColor;
	
	radius = beam->startRadius;

	if (beam->flags & BEAMFLAG_RGBCOLOR || beam->texIndex <= 0)
	{
		//	no texture or flagged to use rgbcolor
		rgbColor = beam->rgbColor;
	}
	else
	{
		//	no color
		rgbColor.Set( 1.0, 1.0, 1.0 );
	}

	if (beam->texIndex >= 0)
	{
		GL_Bind (r_beamTextures [beam->texIndex]->texnum);
	}
	else
	{
		//	no texture, color will be beam->rgbColor unless 
		//	BEAMFLAG_GLOBALCOLOR is specified
//		qglDisable (GL_TEXTURE_2D);
	}

	qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, beam->alpha);
	qglBegin (GL_QUADS);

	if (beam->flags & BEAMFLAG_ALPHAFADE)
	{
		beam_DrawSegmentedBeam_AlphaFade (beam, radius, rgbColor);
	}
	else
		beam_DrawSegmentedBeam_NoFade (beam, radius, rgbColor);

	qglEnd ();

	qglColor4f (1.0, 1.0, 1.0, 1.0);

//	if (beam->texIndex < 0)
//		qglEnable (GL_TEXTURE_2D);
}

///////////////////////////////////////////////////////////////////////////////
//	beam_DrawSingleSegment
//
//	Two-tri version of the laser code.  Always orients to view and remains
//	virtually the same size at all angles.  
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawSingleSegment (beam_t *beam)
{
	CVector		rgbColor;
	CVector		segToViewNormal, beamRight_startRadius, beamRight_endRadius;
	CVector		beamPoint0, beamPoint1, beamPoint2, beamPoint3;
	float		dist;
	beamSeg_t	*beamSeg;

	if (beam->flags & BEAMFLAG_RGBCOLOR || beam->texIndex <= 0)
	{
		//	no texture or flagged to use rgbcolor
		rgbColor = beam->rgbColor;
	}
	else
	{
		//	no color
		rgbColor.Set( 1.0, 1.0, 1.0 );
	}

	if (beam->texIndex >= 0)
	{
		GL_Bind (r_beamTextures [beam->texIndex]->texnum);
	}
	else
	{
		//	no texture, color will be beam->rgbColor unless 
		//	BEAMFLAG_GLOBALCOLOR is specified
//		qglDisable (GL_TEXTURE_2D);
	}

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
//		return;
	}

	//	normalize it and flip it
	beamRight_startRadius.x = beamRight_startRadius.x / -dist;
	beamRight_startRadius.y = beamRight_startRadius.y / -dist;
	beamRight_startRadius.z = beamRight_startRadius.z / -dist;
	beamRight_endRadius =  beamRight_startRadius * beam->endRadius;
	beamRight_startRadius = beamRight_startRadius * beam->startRadius;

	beamPoint3 = beamSeg->segEnd + beamRight_endRadius;
	beamPoint2 = beamSeg->segEnd - beamRight_endRadius;

	//	ending points are already in beamPoint0 and 1 for start of loop
	beamPoint0 = beamSeg->segStart + beamRight_startRadius;
	beamPoint1 = beamSeg->segStart - beamRight_startRadius;

	//	vertices
	qglBegin (GL_TRIANGLE_STRIP);

	qglTexCoord2f (0, 1.0);
	qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, beam->alpha);
	qglVertex3fv (beamPoint1);
	qglTexCoord2f (0, 0);
	qglVertex3fv (beamPoint0);

	qglTexCoord2f (1.0, 1.0);
	if (beam->flags & BEAMFLAG_ALPHAFADE)
		qglColor4f (rgbColor.x, rgbColor.y, rgbColor.z, 0.0);
	qglVertex3fv (beamPoint2);
	qglTexCoord2f (1.0, 0);
	qglVertex3fv (beamPoint3);

	qglEnd ();

//	if (beam->texIndex < 0)
//		qglEnable (GL_TEXTURE_2D);
}

///////////////////////////////////////////////////////////////////////////////
//	beam_DrawBeamList
//
///////////////////////////////////////////////////////////////////////////////

void	beam_DrawBeamList (void)
{
	int			i;
	beamList_t	*beamList;
	beam_t		*beam;

	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		return;
	}

	if (0==r_newrefdef.beamList->numBeams)
		return;

	//	Setup GL state to draw the beam
	qglShadeModel (GL_SMOOTH);

	GL_SetState( GLSTATE_CULL_FACE_FRONT | GLSTATE_TEXTURE_2D | GLSTATE_DEPTH_TEST | GLSTATE_BLEND );

	GL_TexEnv (GL_MODULATE);
	
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
		alpha = e->alpha;
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
	beamRight.x = beamRight.x / -dist;
	beamRight.y = beamRight.y / -dist;
	beamRight.z = beamRight.z / -dist;
	beamRight = beamRight * frame;

	//	third and forth points are just the direction vections plus the oldorigin
	beamPoint3 = beamRight + e->oldorigin;
	beamPoint2 = e->oldorigin - beamRight;

	beamPoint0 = beamRight + e->origin;
	beamPoint1 = e->origin - beamRight;

	GL_Bind (lasertexture->texnum);

//	this REALLY should just be done once for alpha entities, not for each beam!
	GL_TexEnv (GL_MODULATE);

	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_DEPTH_TEST | GLSTATE_BLEND );

	qglColor4f (1.0, 1.0, 1.0, alpha);
	
	qglBegin (GL_QUADS);

	qglTexCoord2f (0, 0);
	qglVertex3fv (beamPoint0);
	qglTexCoord2f (0, 1.0);
	qglVertex3fv (beamPoint1);
	qglTexCoord2f (1.0, 1.0);
	qglVertex3fv (beamPoint2);
	qglTexCoord2f (1.0, 0);
	qglVertex3fv (beamPoint3);

	qglEnd ();
	GL_TexEnv (GL_REPLACE);
}

