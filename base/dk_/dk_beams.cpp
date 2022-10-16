///////////////////////////////////////////////////////////////////////////////
//	dk_beams
//
//	Functions for managing the beam list on the client
///////////////////////////////////////////////////////////////////////////////

#include	"client.h"

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

beamSeg_t	segList [BEAM_MAX_SEGMENTS];
beamList_t	beamList;

///////////////////////////////////////////////////////////////////////////////
//	beam_InitFrame
//
//	initializes all the necessary beamList and seqList stuff for a frame
///////////////////////////////////////////////////////////////////////////////

void	beam_InitFrame (void)
{
	beamList.freeBeam = 0;
	beamList.freeSeg = 0;

	beamList.numBeams = 0;
	beamList.numSegs = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AllocBeam
//
//	returns a pointer to a free member in beamList
///////////////////////////////////////////////////////////////////////////////

beam_t	*beam_AllocBeam (void)
{
	beam_t	*beam;

	if (beamList.freeBeam >= BEAM_MAX_BEAMS)
		//	no more free beams
		return	NULL;

	beam = &beamList.beams [beamList.freeBeam];

	beamList.freeBeam++;
	beamList.numBeams++;

	return	beam;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AllocSeg
//
//	returns a pointer to a free seqment in segList
///////////////////////////////////////////////////////////////////////////////

beamSeg_t	*beam_AllocSeg (void)
{
	beamSeg_t	*seg;

	if (beamList.freeSeg >= BEAM_MAX_SEGMENTS)
		//	no more free segments
		return	NULL;

	seg = &segList [beamList.freeSeg];
	seg->nextSeg = NULL;

	beamList.freeSeg++;
	beamList.numSegs++;

	return	seg;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_SetSeg
//
///////////////////////////////////////////////////////////////////////////////

void	beam_SetSeg (beamSeg_t *beamSeg, CVector &segStart, CVector &segNormal, CVector &segEnd)
{
	beamSeg->segStart = segStart;
	beamSeg->segNormal = segNormal;
	beamSeg->segEnd = segEnd;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AddLightning
//
///////////////////////////////////////////////////////////////////////////////

//	random displacement, beamList version of lightning
//#define	MIN_SEG_LENGTH	25	//	minimum distance between two points
//#define	MAX_SEG_LENGTH	100	//	maximum distance between two points

int	beam_AddLightning ( CVector &start, CVector &end, CVector &rgbColor, float alpha, float radius, int texIndex, unsigned short flags, float Mod)
{
	int				i;
	float			curLength, beamLength, segLength;
	float			maxSegLength, minSegLength, segLengthDiff;
	float			beamDisplacement, halfBeamDisplacement;
	CVector			beamDir;
	beamSeg_t		*beamSeg, *lastSeg;
	beam_t			*beam;

	beam = beam_AllocBeam ();
	if (!beam)
	{
		//	no free beams
		return	false;
	}

	beam->alpha = alpha;

	beam->startRadius = radius;
	beam->endRadius = radius;
	beam->texIndex = texIndex;
	beam->flags = flags;
	beam->numSegs = 0;

	beam->rgbColor = rgbColor;
	beam->beamStart = start;
	beam->beamEnd = end;

	beamDir = end - start;
	beam->beamLength = beamLength = beamDir.Normalize();

	//	longer beams can have greater displacement
	//beamDisplacement = 20.0 * (beamLength / 256.0);
	float RN = frand();
	beamDisplacement = ((20.0+(40.0*RN)) * (beamLength / ((128.0)+(256.0*RN))))*Mod;
	halfBeamDisplacement = (beamDisplacement / 3.5 + (3.5*RN))*Mod;
	//maxSegLength = beamLength / 5.0;
	maxSegLength = (beamLength / 5.0+(15*RN));//*Mod;
	//minSegLength = beamLength / 20.0;
	minSegLength = (beamLength / 20.0+(10*RN));//*Mod;
	if (minSegLength < beamDisplacement)
	{
		minSegLength += beamDisplacement;
	}
	segLengthDiff = maxSegLength - minSegLength;

	beamLength -= maxSegLength;

	if (beamLength < 0)
	{
		beamList.freeBeam--;
		beamList.numBeams--;
		return	false;
	}

	curLength = i = 0;
	lastSeg = NULL;

	for (curLength = i = 0; curLength < beamLength; curLength += segLength, i++)
	{
		beamSeg = beam_AllocSeg ();
		if (beamSeg == NULL)
		{
			//	no more free seqments, so stop adding!
			return	false;
		}

		beam->numSegs++;

        //adam: copy the alpha value to the segment.
        beamSeg->alpha = beam->alpha;

		segLength = frand () * segLengthDiff + minSegLength;

		if (i == 0)
		{
			//	starting point of beam
			beamSeg->segStart = start;
			beam->firstSeg = beamSeg;
		}
		else
		{
			//	start of this segment is the end of the last
			if( lastSeg )
			{
				beamSeg->segStart = lastSeg->segEnd;
			}
		}

		//	always point back to final ending point, then deviate from that
		beamDir = end - beamSeg->segStart;
		beamDir.Normalize();

		VectorMA (beamSeg->segStart, beamDir, segLength, beamSeg->segEnd);

		beamSeg->segEnd.x += frand() * beamDisplacement - halfBeamDisplacement;
		beamSeg->segEnd.y += frand() * beamDisplacement - halfBeamDisplacement;
		beamSeg->segEnd.z += frand() * beamDisplacement - halfBeamDisplacement;

		beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
		//	recalc segLength for a bit more accuracy
		segLength = beamSeg->segNormal.Normalize();

		if (lastSeg)
		{
			//	make sure previous segment points to this one
			lastSeg->nextSeg = beamSeg;
		}

		//	store for next iteration
		lastSeg = beamSeg;
	}

	//	add on the last beam that goes from the previous segment to 
	//	the desired endpoint
	beamSeg = beam_AllocSeg ();
	if (!beamSeg)
	{
		return	false;
	}

    //adam: copy the alpha value to the segment.
    beamSeg->alpha = beam->alpha;

	beam->numSegs++;
	beamSeg->segEnd = end;
	beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
	beamSeg->nextSeg = NULL;
	if( lastSeg != NULL )
	{
		beamSeg->segStart = lastSeg->segEnd;
		lastSeg->nextSeg = beamSeg;
	}

	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AddLaser
//
///////////////////////////////////////////////////////////////////////////////

int	beam_AddLaser ( CVector &start, CVector &end, CVector &rgbColor, float alpha, float radius, float endRadius, int texIndex, unsigned short flags)
{
	beam_t		*beam;
	beamSeg_t	*beamSeg;

	beam = beam_AllocBeam ();
	if (!beam)
		//	no free beams
		return	false;

	beam->alpha = alpha;

	beam->startRadius = radius;
	beam->endRadius = endRadius;
	beam->texIndex = texIndex;
	beam->flags = flags;

	beam->rgbColor = rgbColor;
	beam->beamStart = start;
	beam->beamEnd = end;

	beamSeg = beam_AllocSeg ();
	if (!beamSeg)
		return	false;

    //adam: copy the alpha value to the segment.
    beamSeg->alpha = beam->alpha;

	beam->firstSeg = beamSeg;
	beamSeg->segStart = start;
	beamSeg->segEnd = end;
	beamSeg->segNormal = end - start;
	beamSeg->segNormal.Normalize();
	beamSeg->nextSeg = NULL;

	beam->numSegs = 1;

	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AddNovaLaser
//
///////////////////////////////////////////////////////////////////////////////

int	beam_AddNovaLaser( CVector &start, CVector &end, CVector &rgbColor, float alpha, 
					  float radius, int texIndex, unsigned short flags)
{
	int				i;
	float			curLength, beamLength, segLength;
	float			maxSegLength, minSegLength, segLengthDiff;
	float			beamDisplacement, halfBeamDisplacement;
	CVector			beamDir;
	beamSeg_t		*beamSeg, *lastSeg;
	beam_t			*beam;

	if (alpha / 2.0 < 0.4)
	{
		//	beam goes straight
		return	beam_AddLaser (start, end, rgbColor, alpha, radius * 0.7, radius * 0.7, texIndex, flags);
	}

	beam = beam_AllocBeam ();
	if (!beam)
	{
		//	no free beams
		return	false;
	}

	beam->alpha = alpha;

	//	fudge radius to fit with gun because Nelno took out fractional
	//	beam widths so he could pass a beginning and ending radius
	beam->startRadius = radius * 0.7;
	beam->endRadius = radius * 0.7;
	beam->texIndex = texIndex;
	beam->flags = flags;
	beam->numSegs = 0;

	beam->rgbColor = rgbColor;
	beam->beamStart = start;
	beam->beamEnd = end;

	beamDir = end - start;
	beam->beamLength = beamLength = beamDir.Normalize();

	//	longer beams can have greater displacement
	beamDisplacement = 20.0;// * (beamLength / 256.0);
	halfBeamDisplacement = beamDisplacement / 2.0;

	//	for novabeam, scale displacement by alpha / 2
	beamDisplacement *= (alpha / 10.0);
	halfBeamDisplacement *= (alpha / 10.0);

	maxSegLength = beamLength / 18.0;
	minSegLength = beamLength / 20.0;
	if (minSegLength < beamDisplacement)
		minSegLength += beamDisplacement;
	segLengthDiff = maxSegLength - minSegLength;

	beamLength -= maxSegLength;

	if (beamLength < 0)
	{
		beamList.freeBeam--;
		beamList.numBeams--;
		return	false;
	}

	curLength = i = 0;
	lastSeg = NULL;

	for (curLength = i = 0; curLength < beamLength; curLength += segLength, i++)
	{
		beamSeg = beam_AllocSeg ();
		if (beamSeg == NULL)
		{
			//	no more free seqments, so stop adding!
			return	false;
		}

		beam->numSegs++;

        //adam: copy the alpha value to the segment.
        beamSeg->alpha = beam->alpha;

		segLength = frand () * segLengthDiff + minSegLength;

		if (i == 0)
		{
			//	starting point of beam
			beamSeg->segStart = start;
			beam->firstSeg = beamSeg;
		}
		else
		{
			//	start of this segment is the end of the last
			if( lastSeg != NULL )
			{
				beamSeg->segStart = lastSeg->segEnd;
			}
		}

		//	always point back to final ending point, then deviate from that
		beamDir = end - beamSeg->segStart;
		beamDir.Normalize();

		VectorMA (beamSeg->segStart, beamDir, segLength, beamSeg->segEnd);

		beamSeg->segEnd.x += frand () * beamDisplacement - halfBeamDisplacement;
		beamSeg->segEnd.y += frand () * beamDisplacement - halfBeamDisplacement;
		beamSeg->segEnd.z += frand () * beamDisplacement - halfBeamDisplacement;

		beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
		//	recalc segLength for a bit more accuracy
		segLength = beamSeg->segNormal.Normalize();

		if (lastSeg)
		{
			//	make sure previous segment points to this one
			lastSeg->nextSeg = beamSeg;
		}

		//	store for next iteration
		lastSeg = beamSeg;
	}

	//	add on the last beam that goes from the previous segment to 
	//	the desired endpoint
	beamSeg = beam_AllocSeg ();
	if (!beamSeg)
	{
		return	false;
	}

    //adam: copy the alpha value to the segment.
    beamSeg->alpha = beam->alpha;

	beam->numSegs++;
	beamSeg->segEnd = end;
	beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
	beamSeg->nextSeg = NULL;
	if (lastSeg != NULL )
	{
		beamSeg->segStart = lastSeg->segEnd;
		lastSeg->nextSeg = beamSeg;
	}

	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	beam_AddNovaLaser
//
///////////////////////////////////////////////////////////////////////////////

int	beam_AddGCESplines (entity_t *entList, int numEnts, int selectedSeg, float r=1.0f, float g=1.0f, float b=1.0f)
{
	entity_t	*ent;
	beam_t		*beam;
	beamSeg_t	*beamSeg, *lastSeg = NULL;
	int			i;

	beam = beam_AllocBeam ();
	if (beam == NULL)
		return	0;

	beam->alpha = 1.0;
	beam->startRadius = 2.0;
	beam->endRadius = 2.0;
	beam->texIndex = BEAM_TEX_LASER;
    beam->rgbColor.Set( r, g, b );
	
    if (r==1.0f&&g==1.0f&&b==1.0f)
        beam->flags = 0;
    else
        beam->flags = BEAMFLAG_RGBCOLOR;
    
	//	add the entities used for the curve.
    for (i = 0; i < numEnts; i++) 
	{
		ent = entList + i;

		beamSeg = beam_AllocSeg ();
		if (beamSeg == NULL)
			return	beam->numSegs;

		beam->numSegs++;

		if (i == 0)
		{
			beam->firstSeg = beamSeg;
			beam->beamStart = ent->origin;
		}
		else
		{
			if( lastSeg != NULL )
			{
				lastSeg->nextSeg = beamSeg;
			}
		}

		lastSeg = beamSeg;


        //set the alpha value of this segment.
        beamSeg->alpha = ent->alpha;

		beamSeg->segStart = ent->origin;
		beamSeg->segEnd = ent->oldorigin;
		beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
		beamSeg->segNormal.Normalize();
    }

	return	beam->numSegs;
}