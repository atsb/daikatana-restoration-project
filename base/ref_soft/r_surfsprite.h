#ifndef	R_SURFSPRITE_H
#define	R_SURFSPRITE_H

///////////////////////////////////////////////////////////////////////////////
//	gl_surfSprite.h
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	MAX_SURF_SPRITES	1024


///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//this struct holds a world and texture point that is used to build a surface sprite.
class sprite_point : public CVector {
public:
    //the texture coordinates.
    float u, v;
};

typedef	struct	surfSprite_s
{
	msurface_t		*surf;
	struct	model_s	*model;		//	pointer to a sprite model
									//	this probably won't be sufficient once
									//	we start clipping to surface polys
									//	NULL is surfSprite is free

    //the number of points that make the polygon.
    array_obj<sprite_point> points;

	vec3_t			origin;			//	either in world coords or local model coords
									//	if SSF_LOCALORIGIN flag is set
	vec3_t			lightValue;		//	intensity/color of light at world origin
	short			Index;			//	-1 if surfSprite is free
	short			nextIndex;		//	Index to next sprite on surfacenext in list

//	int				allocTime;		//	when this sprite was allocated
	int				frame;

	float			roll;
	float			scale;
	byte			flags;			//	flags
} surfSprite_t;

///////////////////////////////////////////////////////////////////////////////
//	globals... these really should be invisible to other c files... but, hey
//	I'm up for a hack every now and then!
///////////////////////////////////////////////////////////////////////////////

//#ifdef __cplusplus
//extern "C" {
//#endif

extern	short			g_numSurfSprites;
extern	surfSprite_t	g_surfSpriteList [MAX_SURF_SPRITES];

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

surfSprite_t	*surfSprite_Allocate (void);
void			surfSprite_Free (surfSprite_t *sprite);
void			surfSprite_Init (void);
void			surfSprite_PolyCenter (glpoly_t *poly, vec3_t center);
void			surfSprite_Add (msurface_t *surf, surfSprite_t *sprite);
void			surfSprite_Remove (msurface_t *surf, surfSprite_t *sprite);
void			surfSprite_DrawSprite (msurface_t *surf, surfSprite_t *sprite);
void			surfSprite_DrawSpritesForSurface (msurface_t *surf, int isAlphaSurface);

void			surfSprite_AddSurfaceSprite (vec3_t origin, vec3_t entOrigin, 
				vec3_t entAngles, void *hitModel, int planeIndex, void *model, 
				int frame, float roll, float scale, byte flags);

//void			surfSprite_AddSurfaceSprite (vec3_t origin, void *hitModel, int planeIndex, void *model, int frame, float roll, float scale, byte flags);

//#ifdef __cplusplus
//}
//#endif


#endif