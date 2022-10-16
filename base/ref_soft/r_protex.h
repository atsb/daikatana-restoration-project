#ifndef	PROTEX_H_
#define	PROTEX_H_

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

//	set if this texture was already updated this frame, cleared at the
//	start of each frame
#define	PTF_RAN_THIS_FRAME	0x00000001
#define	PTF_RUN_ONCE		0x00000002	//	only runs once on initialization

///////////////////////////////////////////////////////////////////////////////
//	enumi-wummi-nummi-dumms
///////////////////////////////////////////////////////////////////////////////

typedef	enum
{
	PROTEX_FIRE,
	PROTEX_SWAMPGAS,
	PROTEX_STEAM,
	PROTEX_STATIC,
	PROTEX_FIRESKIN,
	PROTEX_LAVA,
	PROTEX_WATER,
	PROTEX_MIST,
	PROTEX_NUM_TYPES
} protex_type;

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	proTex_s
{
	byte		*data;
	int			width;
	int			height;
	int			bit_depth;
	
	//	type of procedural texture
	protex_type	type;
	//	for easy management of slight procedural variations
	int			sub_type;

	unsigned long	flags;

	//	pointer to procedural function for this texture
	void		(*func)(struct proTex_s *texture);	

	float		frame_time;	//	time between frames for this texture
	float		next_time;	//	next ref_globalTime to run this texture

	image_t		*image;
} proTex_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern	proTex_t	protex_textures [PROTEX_NUM_TYPES];

extern	cvar_t		*pt_type;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

//	utility functions
void	protex_Alloc (proTex_t *texture, int w, int h, int bit_depth, float frame_time, void (*func)(proTex_t *texture));
void	protex_Free (proTex_t *texture);
void	protex_InitTextures (void);
void	protex_FreeTextures (void);
void	protex_InitForFrame (void);
image_t	*protex_InitProceduralFromWAL (const char *name, byte *pic, int width, int height);
void	protex_RunProceduralTexture (proTex_t *texture);

#endif