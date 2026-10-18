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

#define	PROTEX_FLAG_FIRE		0x00000001
#define	PROTEX_FLAG_SWAMPGAS	0x00000002
#define	PROTEX_FLAG_STEAM		0x00000004
#define	PROTEX_FLAG_STATIC		0x00000008
#define	PROTEX_FLAG_FIRESKIN	0x00000010
#define	PROTEX_FLAG_LAVA		0x00000020
#define	PROTEX_FLAG_WATER		0x00000040
#define	PROTEX_FLAG_MIST		0x00000080

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	proTex_s
{
	byte		*data;
	int			width;
	int			height;
	int			bytes_per_pixel;
	
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

//#ifdef __cplusplus
//extern "C" {
//#endif

extern	proTex_t	protex_textures [PROTEX_NUM_TYPES];

extern	cvar_t		*pt_type;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////
void R_ProcAlloc( proTex_t *texture, int w, int h, int bytes_per_pixel );
void R_ProcFree( proTex_t *texture );
void Proc_InitSinTable ();
void R_ProcInit();
void R_ProcFreeTextures ();
image_t	*R_ProcTextureInit( const char *name, int width, int height );
void R_ProceduralFrame();

//#ifdef __cplusplus
//}
//#endif

#endif