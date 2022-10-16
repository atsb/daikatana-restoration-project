#ifndef	DK_BEAMS_H
#define	DK_BEAMS_H

///////////////////////////////////////////////////////////////////////////////
//	externs
///////////////////////////////////////////////////////////////////////////////

extern	beamSeg_t	segList [BEAM_MAX_SEGMENTS];
extern	beamList_t	beamList;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////


int		beam_AddLightning (CVector &start, CVector &end, CVector &rgbColor, float alpha, float radius, int texIndex, unsigned short flags, float Mod);
int		beam_AddLaser (CVector &start, CVector &end, CVector &rgbColor, float alpha, float radius, float endRadius, 
					   int texIndex, unsigned short flags);
void	beam_InitFrame (void);
beam_t	*beam_AllocBeam (void);
beamSeg_t	*beam_AllocSeg (void);
int		beam_AddGCESplines (entity_t *entList, int numEnts, int selectedSeg, float r=1.0f, float g=1.0f, float b=1.0f);

#endif 