#pragma once

#include "ref.h"

#include "d3d_model.h"
#include "d3d_error.h"
#include "d3denum.h"


//-----------------------------------------------------------------------------
// TYPEDEFS
//-----------------------------------------------------------------------------
typedef struct vertex_s
{
    D3DVALUE x, y, z;
    D3DCOLOR color;
    D3DVALUE tu, tv, tu2, tv2;
} vertex_t;

typedef struct d3d_state_s
{
	HWND					hWnd;				// window handle
	HINSTANCE				hInst;				// instance handle
	
	DWORD					width;				// render width
	DWORD					height;				// render height 
	DWORD					bpp;				// bits per pixel
	DWORD					buffer_count;		// number of secondary buffers (for double/triple buffering)

	RECT					view_rect;
	RECT					window_rect;

	BOOL					bHardware;			// hardware or software 
	BOOL					bWindowed;			// windowed or fullscreen
	BOOL					bZBuffered;			// z-buffered or not

	GUID					*pDriverGUID;		// pointer to the driver GUID
	GUID					*pDeviceGUID;		// pointer to the device GUID
	D3DEnum_DriverInfo		*pDriverInfo;
	D3DEnum_DeviceInfo		*pDeviceInfo;

	// direct3d objects, etc
	LPDIRECTDRAW4			lpdd;				// pointer to direct draw 4 object
	LPDIRECT3D3				lpd3d;				// pointer to direct3d 3 object
	LPDIRECTDRAWSURFACE4	lpddsFront;			// pointer to primary surface
	LPDIRECTDRAWSURFACE4	lpddsBack;			// pointer to secondary surface (double buffering)
	LPDIRECTDRAWSURFACE4	lpddsRender;		// pointer to the render target
    LPDIRECTDRAWSURFACE4	lpddsZBuffer;		// pointer to the z buffer
	LPDIRECT3DDEVICE3		lpd3dDevice;		// pointer to the direct3d 3 device
	LPDIRECT3DVIEWPORT3		lpd3dViewport;		// pointer to the direct3d viewport
    DDPIXELFORMAT			ddpsSurface;		// the pixel format of the render surfaces
    DDPIXELFORMAT			ddpfZBuffer;		// the pixel format of the z buffer
	D3DDEVICEDESC			ddDeviceDesc;		// the direct3d 3 device description

	// vertex buffer objects, etc
	vertex_t				*pVerts;
	DWORD					dwSizeVerts;
	short					indices[65535];
	LPDIRECT3DVERTEXBUFFER	lpd3dvbVerts;
	LPDIRECT3DVERTEXBUFFER	lpd3dvbFinalVerts;
} d3d_state_t;

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;



typedef struct
{
	int		width, height;					// coordinates from main game
} viddef_t;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

extern refdef_t	r_newrefdef;
extern model_t	*r_worldmodel;
extern entity_t	*currententity;
extern model_t	*currentmodel;

extern int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern d3d_state_t		d3d_state;		// renderer state information

extern array<CTexture>	loaded_textures;
extern array<CWal>		loaded_wals;
extern array<CPic>		loaded_pics;

extern refimport_t		ri;

//-----------------------------------------------------------------------------
// CONSOLE VARIABLES
//-----------------------------------------------------------------------------
extern cvar_t		*vid_xpos;				// window x position
extern cvar_t		*vid_ypos;				// window y position
extern cvar_t		*vid_fullscreen;		// fullscreen flag

extern cvar_t		*r_resourcedir;			// resource directory

extern cvar_t		*d3d_mode;				// display mode
extern cvar_t		*d3d_backbuffer;		// depth buffer creation flag
extern cvar_t		*d3d_zbuffer;			// z buffer creation flag
extern cvar_t		*d3d_bpp;				// number of bits per pixel

extern CVector		vup, base_vup;
extern CVector		vpn, base_vpn;
extern CVector		vright, base_vright;
extern CVector		r_origin;

extern CVector		r_entorigin;
extern CVector		modelorg;


//-----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//
// d3d_draw.cpp
//
void Draw_InitLocal();
void DrawStretchPic( int x, int y, int w, int h, const char *name, resource_t resource );
void DrawPic( int x, int y, const char *name, resource_t resource );
void DrawPicPortion(int destx, int desty, int src_left, int src_top, int src_width, int src_height, void *font);

//
// d3d_ref_main.cpp
//
void *RegisterSkin( const char *name, resource_t resource );
void *RegisterPic( const char *name, resource_t resource );

//
// d3d_sprite.cpp
//
void R_DrawSprite();

//
// d3d_mesh.cpp
//
void R_DrawAliasModel();

//
// d3d_model.h
//
void Mod_Free( model_t *mod );
void Mod_FreeAll();

//
// d3d_math.h
//
void SetIdentityMatrix( D3DMATRIX matrix );
void SetTranslateMatrix( D3DMATRIX &matrix, D3DVECTOR &v );
void SetTranslateMatrix( D3DMATRIX &matrix, float x, float y, float z );
void SetRotateXMatrix( D3DMATRIX &matrix, float angle );
void SetRotateYMatrix( D3DMATRIX &matrix, float angle );
void SetRotateZMatrix( D3DMATRIX &matrix, float angle );
void MatrixMultiply( D3DMATRIX c, D3DMATRIX a, D3DMATRIX b );
D3DMATRIX CreateWorldMatrix( entity_t *entity );
D3DMATRIX CreateViewMatrix();
D3DMATRIX CreateProjectionMatrix( float nplane, float fplane, float hfov, float vfov );
