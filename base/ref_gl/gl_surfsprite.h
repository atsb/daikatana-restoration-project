#ifndef	GL_SURFSPRITE_H
#define	GL_SURFSPRITE_H

///////////////////////////////////////////////////////////////////////////////
//	gl_surfSprite.h
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	MAX_SURF_SPRITES		512
#define MAX_SPRITES_PER_SURFACE 32


///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//this struct holds a world and texture point that is used to build a surface sprite.
class sprite_point {
public:
    // the position
    CVector pos;

    //the texture coordinates.
    float s, t;
};

typedef	struct	surfSprite_s
{
	msurface_t		*pSurf;
	model_t			*pModel;

	image_t			*pImage;
    
    sprite_point    points[16];
    int             numpoints;
    
	CVector			vOrigin;		// SCG[5/3/99]: The origin of the surface sprite
	CVector			vLightValue;	// SCG[5/3/99]: LightValue
    CVector         vNormal;
	int				nFrame;

	float			fScale;
	float			fRoll;

	byte			nFlags;			//	flags

    DWORD           expire_time;
    float           alpha;
} surfSprite_t;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void R_InitSurfaceSprites();
void R_SurfaceSpriteAdd( CVector& vOrigin, CVector& vEntOrigin, CVector& vEntAngles, 
						 void *pHitModel, int nIndex, void *pSpriteModel, int nFrame, 
						 float fRoll, float fScale, byte nFlags );

#endif