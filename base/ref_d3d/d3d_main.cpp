#include <windows.h>
#define __MSC__

#include "d3d_local.h"
#include "dk_ref_common.h"
//#include "d3denum.h"
#include "d3dutil.h"




//the size of the window that is created.
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define CLASS_NAME "Daikatana"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

// global window procedure
WNDPROC		g_wndProc;

// application name
char		g_szAppName[] = "Daikatana";

d3d_state_t		d3d_state;		// renderer state information

int	old_win_xpos, old_win_ypos = 0;

float	ref_laserRotation = 0.0F;

//-----------------------------------------------------------------------------
// GLOBAL CONSOLE VARIABLES
//-----------------------------------------------------------------------------
cvar_t		*vid_xpos			= NULL;		// window x position
cvar_t		*vid_ypos			= NULL;		// window y position
cvar_t		*vid_fullscreen		= NULL;		// fullscreen flag

cvar_t		*r_resourcedir		= NULL;		// resource directory

cvar_t		*d3d_mode			= NULL;		// display mode
cvar_t		*d3d_backbuffer		= NULL;		// depth buffer creation flag
cvar_t		*d3d_zbuffer		= NULL;		// z buffer creation flag
cvar_t		*d3d_bpp			= NULL;		// number of bits per pixel

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------
void InitializeConsoleVariables();

//---------------------------------------------
//these are the functions we export to the exe.
//---------------------------------------------
qboolean Init(void *hinstance, void *wnd_proc);
void Shutdown();
void PurgeResources(resource_t resource);
void BeginRegistration(const char *model);
void *RegisterModel(const char *name, resource_t resource);
void *RegisterSkin(const char *name, resource_t resource);
void *RegisterPic(const char *name, resource_t resource);
void SetSky(const char *name, float rotate, CVector &axis);
void SetPaletteDir(const char *name);
void EndRegistration();
void RenderFrame(refdef_t *fd);
void DrawGetPicSize(int *w, int *h, const char *name, resource_t resource);
void DrawPic(int x, int y, const char *name, resource_t resource);
void DrawStretchPic(int x, int y, int w, int h, const char *name, resource_t resource);
void DrawChar(int x, int y, int c);
void DrawTileClear(int x, int y, int w, int h, const char *name);
void DrawFill(int x, int y, int w, int h, int c);
void DrawFadeScreen();
void DrawStretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data);
void CinematicSetPalette(const unsigned char *palette);
void SetInterfacePalette( const unsigned char *palette );
void BeginFrame(float camera_separation);
void EndFrame();
void AppActivate(qboolean activate);
void SetResourceDir(char *name);
void DrawConsolePic(int x, int y, int w, int h);
void GetFrameName(char *modelname, int frameindex, char *framename);
void RegisterEpisodeSkins(void *model, int episode, int flags);
int VertInfo(void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, CVector &oldorigin, CVector &angles, CVector &vert);
int SurfIndex(void *mod, char *name);
int TriVerts(void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
             CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3);



//-----------------------------------------------------------------------------
// Name: App_ConfirmDevice()
// Desc: Called during device intialization, this code checks to see if
//       the device can support alpha, and displays a warning if not.
//-----------------------------------------------------------------------------
HRESULT App_ConfirmDevice( DDCAPS* pddDriverCaps,
						   D3DDEVICEDESC* pd3dDeviceDesc )
{
    // Get triangle caps (Hardware or software) and check for alpha blending
    LPD3DPRIMCAPS pdpc = &pd3dDeviceDesc->dpcTriCaps;

	if( 0 == ( pdpc->dwSrcBlendCaps & pdpc->dwDestBlendCaps & D3DBLEND_ONE ) )
        return E_FAIL;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetRefAPI
// Desc: exports functions to the client
//-----------------------------------------------------------------------------
//the table of functions we get from the exe.
refimport_t ri;
refexport_t re;

refexport_t GetRefAPI( refimport_t rimp ){
	//save the import table passed in.
	ri = rimp;

	//set the version number
	re.api_version = API_VERSION;

	//get pointers to all the top-level functions.
	re.Init					= Init;
	re.Shutdown				= Shutdown;
	re.PurgeResources		= PurgeResources;
	re.BeginRegistration	= BeginRegistration;
	re.RegisterModel		= RegisterModel;
	re.RegisterSkin			= RegisterSkin;
	re.RegisterPic			= RegisterPic;
	re.SetSky				= SetSky;
	re.SetPaletteDir		= SetPaletteDir;
	re.EndRegistration		= EndRegistration;
	re.RenderFrame			= RenderFrame;
	re.DrawGetPicSize		= DrawGetPicSize;
	re.DrawPic				= DrawPic;
	re.DrawStretchPic		= DrawStretchPic;
	re.DrawChar				= DrawChar;
	re.DrawTileClear		= DrawTileClear;
	re.DrawFill				= DrawFill;
	re.DrawFadeScreen		= DrawFadeScreen;
	re.DrawStretchRaw		= DrawStretchRaw;
	re.CinematicSetPalette	= CinematicSetPalette;
	re.SetInterfacePalette	= SetInterfacePalette;
	re.BeginFrame			= BeginFrame;
	re.EndFrame				= EndFrame;
	re.AppActivate			= AppActivate;
	re.SetResourceDir		= SetResourceDir;
	re.DrawConsolePic		= DrawConsolePic;
	re.GetFrameName			= GetFrameName;
	re.VertInfo				= VertInfo;
	re.SurfIndex			= SurfIndex;
	re.TriVerts				= TriVerts;
	re.RegisterEpisodeSkins	= RegisterEpisodeSkins;

	//return our export table.
	return re;
}



//-----------------------------------------------------------------------------
// Name: ResizeWindow
// Desc: Resizes or moves the window
//-----------------------------------------------------------------------------
void ResizeWindow()
{
}



//-----------------------------------------------------------------------------
// Name: CreateMainWindow
// Desc: Creates the application window 
//-----------------------------------------------------------------------------
bool CreateMainWindow() 
{
	WNDCLASS cls;

	//Register a class for the main application window
	cls.hCursor        = LoadCursor(0,IDC_ARROW);
	cls.hIcon          = 0;
	cls.lpszMenuName   = NULL;
	cls.lpszClassName  = g_szAppName;
	cls.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	cls.hInstance      = d3d_state.hInst;
	cls.style          = NULL;
	cls.lpfnWndProc    = g_wndProc;
	cls.cbClsExtra     = 0;
	cls.cbWndExtra     = 0;

	vid_xpos = ri.Cvar_Get ("vid_xpos", "0", 0);
	vid_ypos = ri.Cvar_Get ("vid_ypos", "0", 0);

	old_win_xpos = (int) vid_xpos->value;
	old_win_ypos = (int) vid_ypos->value;

	if( RegisterClass(&cls) == FALSE ) 
		return false;

	int	winstyle = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE;

	// calc the window size
	RECT r;
	r.left		= 0;
	r.top		= 0;
	r.right		= WINDOW_WIDTH;
	r.bottom	= WINDOW_HEIGHT;

	// compute the size of the window required to get the correct size client area.
//	AdjustWindowRect(&r, 0, FALSE);

	// get the width and height of the window.
	int x = vid_xpos->value;
	int y = vid_ypos->value;
	int w = r.right - r.left;
	int h = r.bottom - r.top;

	// create the window.
	d3d_state.hWnd = CreateWindowEx( 0, g_szAppName, g_szAppName, winstyle, x, y, w, h, NULL, 0, d3d_state.hInst, 0 );

	// make sure the window is valid
	if( d3d_state.hWnd == NULL )  
		return false;

	//show the window, and make it paint.
	ShowWindow( d3d_state.hWnd, SW_SHOWNORMAL );
	UpdateWindow( d3d_state.hWnd );

	//get the input focus.
	SetFocus( d3d_state.hWnd );

	SetForegroundWindow( d3d_state.hWnd );
	SetFocus( d3d_state.hWnd );

	//tell the executable the size of our window.
	ri.Vid_NewWindow( WINDOW_WIDTH, WINDOW_HEIGHT );

	return true;
}



//-----------------------------------------------------------------------------
// Name: InitDirectX
// Desc: Initializes direct3d
//-----------------------------------------------------------------------------
BOOL D3D_InitializeAll();
HRESULT InitDirectX()
{
	HRESULT	hr		= 0;
	DWORD	dwFlags	= 0;;

	// enumerate the devices
	if( FAILED( hr = D3DEnum_EnumerateDevices( App_ConfirmDevice ) ) )
		return hr;

	d3d_state.bWindowed		= vid_fullscreen->value ? false : true;
	d3d_state.buffer_count	= Q_ftol( d3d_backbuffer->value );
	d3d_state.bZBuffered	= Q_ftol( d3d_zbuffer->value );
	d3d_state.bHardware		= true;
	d3d_state.bpp			= Q_ftol( d3d_bpp->value );

	d3d_state.width		= WINDOW_WIDTH;
	d3d_state.height	= WINDOW_HEIGHT;

	if( FAILED( hr = D3DEnum_GetSelectedDriver( &d3d_state.pDriverInfo, &d3d_state.pDeviceInfo ) ) )
		return hr;

	d3d_state.pDriverGUID = d3d_state.pDriverInfo->pGUID;
	d3d_state.pDeviceGUID = d3d_state.pDeviceInfo->pGUID;

	if( !D3D_InitializeAll() )
		return -1;

	return DD_OK;
}



//-----------------------------------------------------------------------------
// Name: SetMode
// Desc: 
//-----------------------------------------------------------------------------
qboolean SetMode()
{
	vid_fullscreen->modified	= false;
	d3d_mode->modified			= false;

	// destroy the window if it exists
    if( d3d_state.hWnd != NULL )
        DestroyWindow( d3d_state.hWnd );
	
	// create the widnow
	if( CreateMainWindow() == false )
		return false;

	// initialize DirectX
	if( FAILED( InitDirectX() ) )
		return false;

	return true;
}



//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initializes the refresh dll
//-----------------------------------------------------------------------------
qboolean Init(void *hinstance, void *wnd_proc) 
{
	d3d_state.hInst	= hinstance;
	g_wndProc		= (WNDPROC) wnd_proc;

	// initialize the console variables
	InitializeConsoleVariables();

	if( SetMode() == false )
		return -1;

	// local drawing variables
	Draw_InitLocal();

	return true;
}



//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Shuts down the renderer
//-----------------------------------------------------------------------------
void Shutdown() 
{
    if( d3d_state.hWnd != NULL ) 
        DestroyWindow( d3d_state.hWnd );

    //unregister our class.
    BOOL success = UnregisterClass( g_szAppName, d3d_state.hInst );
}



//-----------------------------------------------------------------------------
// Name: InitializeConsoleVariables
// Desc: initializes the console variables (a.k.a. r_register)
//-----------------------------------------------------------------------------
void InitializeConsoleVariables()
{
	// get the stored window x and y position
	vid_xpos			= ri.Cvar_Get ("vid_xpos", "0", 0);
	vid_ypos			= ri.Cvar_Get ("vid_ypos", "0", 0);
	vid_fullscreen		= ri.Cvar_Get ("vid_fullscreen", "0", 0);

	r_resourcedir		= ri.Cvar_Get( "r_resourcedir", "", 0 );

	d3d_mode			= ri.Cvar_Get( "d3d_mode", "3", CVAR_ARCHIVE );
	d3d_backbuffer		= ri.Cvar_Get( "d3d_backbuffer", "1", CVAR_ARCHIVE );
	d3d_zbuffer			= ri.Cvar_Get( "d3d_zbuffer", "1", CVAR_ARCHIVE );
	d3d_bpp				= ri.Cvar_Get( "d3d_bpp", "32", CVAR_ARCHIVE );
}



#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
//-----------------------------------------------------------------------------
// Name: Sys_Error
// Desc: 
//-----------------------------------------------------------------------------
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ri.Sys_Error (ERR_FATAL, "%s", text);
}

//-----------------------------------------------------------------------------
// Name: Com_Printf
// Desc: 
//-----------------------------------------------------------------------------
void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	ri.Con_Printf (PRINT_ALL, "%s", text);
}

#endif
