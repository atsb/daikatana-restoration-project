// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
#include	<assert.h>
#include	<float.h>
#include <stdio.h>
#include	"client.h"
#include	"winquake.h"
#include	"daikatana.h"
#include	"dk_beams.h"
#include	"l__language.h"
#include	"dk_menu.h"
#include <SDL3/SDL.h>

//#include "zmouse.h"
// Structure containing functions exported from refresh DLL
refexport_t re;
cvar_t *win_noalttab;

void VID_ModifySequenceStart();
bool VID_ModifySequenceFailure();
void VID_ModifySequenceSuccess();

// Console variables that we need to access from this module
cvar_t      *vid_gamma;
cvar_t      *vid_ref;           // Name of Refresh DLL loaded
cvar_t      *vid_xpos;          // X coordinate of window position
cvar_t      *vid_ypos;          // Y coordinate of window position
cvar_t      *vid_fullscreen;

//some people dont want the video to shutdown, namely those with
//a second monitor plugged into their 3dfx card.
cvar_t *no_shutdown;

// Global variables used internally by this module

viddef_t    viddef;             // global video state; used by other modules
HINSTANCE   reflib_library;     // Handle to refresh DLL 
qboolean    reflib_active = 0;
extern HWND        cl_hwnd;            // Native handle retained only for legacy subsystems
static SDL_Window *vid_sdl_window = NULL;

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )
static qboolean s_alttab_disabled;


//	font stuff -- must get reset with vid_restarts
extern	void	*scr_centerFont;
extern	void	*con_font;
extern	void	*notify_font;
extern	void	*sbar_font_g;
extern	void	*sbar_font_r;
extern  void	*scoreboard_font;
extern  void	*statbar_font;
extern  void	*aim_font;

int bAppActive = TRUE;


/*
** WIN32 helper functions
*/
/* SDL3 owns the application window, so the old Win32 Alt-Tab suppression
 * hooks are intentionally no-ops.  The cvar remains for config compatibility. */
static void WIN_DisableAltTab(void)
{
    s_alttab_disabled = true;
}

static void WIN_EnableAltTab(void)
{
    s_alttab_disabled = false;
}
/*
==========================================================================
DLL GLUE
==========================================================================
*/
#define MAXPRINTMSG 4096
void VID_Printf (int print_level, char *fmt, ...)
{
    va_list     argptr;
    char        msg[MAXPRINTMSG];
    static qboolean inupdate;
    
    va_start (argptr,fmt);
    vsprintf (msg,fmt,argptr);
    va_end (argptr);
    if (print_level == PRINT_ALL)
    {
        Com_Printf ("%s", msg);
    }
    else if ( print_level == PRINT_DEVELOPER )
    {
        Com_DPrintf ("%s", msg);
    }
    else if ( print_level == PRINT_ALERT )
    {
        MessageBox( 0, msg, "PRINT_ALERT", MB_ICONWARNING );
        OutputDebugString( msg );
    }
}
void VID_Error (int err_level, char *fmt, ...)
{
    va_list     argptr;
    char        msg[MAXPRINTMSG];
    static qboolean inupdate;
    
    va_start (argptr,fmt);
    vsprintf (msg,fmt,argptr);
    va_end (argptr);
    Com_Error (err_level,"%s", msg);
}

/* moved to localization
//==========================================================================
byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
    'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
    '\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
    'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
    K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
    K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0  , K_HOME, 
    K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4 
    K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
    K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 
*/
void AppActivate(BOOL fActive, BOOL minimize)
{
    Minimized = minimize;
    Key_ClearStates();
    // we don't want to act like we're active if we're minimized
    if (fActive && !Minimized)
        ActiveApp = true;
    else
        ActiveApp = false;
    // minimize/restore mouse-capture on demand
    if (!ActiveApp)
    {
        IN_Activate (false);
        CDAudio_Activate (false);
        if ( win_noalttab->value )
        {
            WIN_EnableAltTab();
        }
    }
    else
    {
        IN_Activate (true);
        CDAudio_Activate (true);
        if ( win_noalttab->value )
        {
            WIN_DisableAltTab();
        }
    }
}
/*
============
VID_InitFonts
This is called any time the video mode changes to make sure that 
the fonts are valid...
============
*/
void VID_InitFonts( void )
{
	// SCG[5/28/99]: Init misc fonts
	scr_centerFont	= NULL;// re.LoadFont( "scr_cent" );
	con_font		= NULL;// re.LoadFont( "con_font" );
	notify_font		= NULL;// re.LoadFont( "scr_cent" );
	sbar_font_g		= NULL;// re.LoadFont( "mainnums" );
	sbar_font_r		= NULL;// re.LoadFont( "mainred" );
	scoreboard_font	= NULL;// re.LoadFont( "scorefont" );
	statbar_font	= NULL;
	aim_font		= NULL;
	
	// SCG[5/28/99]: Init the menu fonts
	if( reflib_active )
	{
		if( DKM_InMenu() )
		{
			DKM_Leave();
			DKM_Enter();
		}
	}
}

/*
============
VID_Restart_f
Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void VID_Restart_f (void)
{
    vid_ref->modified = true;
}
void VID_Front_f( void )
{
    if (vid_sdl_window != NULL)
        SDL_RaiseWindow(vid_sdl_window);
}
/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
    int         mode;
} vidmode_t;
static const int VID_LEGACY_MODE_COUNT = 10;
static const int VID_MAX_MODES = 64;
static vidmode_t vid_modes[VID_MAX_MODES] =
{
    { "Mode 0: 320x240",   320, 240,   0 },
    { "Mode 1: 400x300",   400, 300,   1 },
    { "Mode 2: 512x384",   512, 384,   2 },
    { "Mode 3: 640x480",   640, 480,   3 },
    { "Mode 4: 800x600",   800, 600,   4 },
    { "Mode 5: 960x720",   960, 720,   5 },
    { "Mode 6: 1024x768",  1024, 768,  6 },
    { "Mode 7: 1152x864",  1152, 864,  7 },
    { "Mode 8: 1280x960",  1280, 960, 8 },
    { "Mode 9: 1600x1200", 1600, 1200, 9 }
};
static int vid_num_modes = VID_LEGACY_MODE_COUNT;
static char vid_mode_names[VID_MAX_MODES][64];

static bool VID_ModeAlreadyPresent(int width, int height)
{
    int i;
    for (i = 0; i < vid_num_modes; ++i)
    {
        if (vid_modes[i].width == width && vid_modes[i].height == height)
            return true;
    }
    return false;
}

void VID_RefreshModeList(void)
{
    SDL_DisplayID display;
    SDL_DisplayMode **modes;
    int mode_count;
    int i;

    /* Preserve the original mode indices so existing configs remain valid.
       Append only real fullscreen modes exposed by the current display. */
    vid_num_modes = VID_LEGACY_MODE_COUNT;

    display = vid_sdl_window ? SDL_GetDisplayForWindow(vid_sdl_window) : SDL_GetPrimaryDisplay();
    if (display == 0)
        return;

    modes = SDL_GetFullscreenDisplayModes(display, &mode_count);
    if (modes == NULL)
        return;

    for (i = 0; i < mode_count && vid_num_modes < VID_MAX_MODES; ++i)
    {
        const SDL_DisplayMode *mode = modes[i];
        if (mode == NULL || mode->w < 640 || mode->h < 480)
            continue;
        if (VID_ModeAlreadyPresent(mode->w, mode->h))
            continue;

        vid_modes[vid_num_modes].width = mode->w;
        vid_modes[vid_num_modes].height = mode->h;
        vid_modes[vid_num_modes].mode = vid_num_modes;
        snprintf(vid_mode_names[vid_num_modes], sizeof(vid_mode_names[vid_num_modes]),
                 "Mode %d: %dx%d", vid_num_modes, mode->w, mode->h);
        vid_modes[vid_num_modes].description = vid_mode_names[vid_num_modes];
        ++vid_num_modes;
    }

    SDL_free(modes);
}

int VID_GetModeCount(void)
{
    return vid_num_modes;
}

const char *VID_GetModeDescription(int mode)
{
    if (mode < 0 || mode >= vid_num_modes)
        return "Unknown";
    return vid_modes[mode].description;
}

qboolean VID_GetModeInfo( int *width, int *height, int mode )
{
    if ( mode < 0 || mode >= vid_num_modes )
        return false;
    *width  = vid_modes[mode].width;
    *height = vid_modes[mode].height;
    return true;
}
/*
** VID_UpdateWindowPosAndSize
*/
void VID_UpdateWindowPosAndSize( int x, int y )
{
    if (vid_sdl_window == NULL || vid_fullscreen->value)
        return;

    SDL_SetWindowPosition(vid_sdl_window, x, y);
    SDL_SetWindowSize(vid_sdl_window, viddef.width, viddef.height);
    SDL_SyncWindow(vid_sdl_window);
}
void VID_SetSDLWindow(void *window)
{
    SDL_PropertiesID properties;

    vid_sdl_window = (SDL_Window *)window;
    IN_SetSDLWindow(vid_sdl_window);

    cl_hwnd = NULL;
    if (vid_sdl_window == NULL)
        return;

    /* Keep the native handle available only for legacy code such as the
     * optional Bink video path.  Window creation and management remain SDL3. */
    properties = SDL_GetWindowProperties(vid_sdl_window);
    if (properties != 0)
    {
        cl_hwnd = (HWND)SDL_GetPointerProperty(
            properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    }
}

/*
** VID_NewWindow
*/
void VID_NewWindow ( int width, int height)
{
    viddef.width  = width;
    viddef.height = height;
    cl.force_refdef = true;     // can't use a paused refdef
}
void VID_FreeReflib (void)
{
    if ( !FreeLibrary( reflib_library ) )
        Com_Error( ERR_FATAL, "Reflib FreeLibrary failed" );
    memset (&re, 0, sizeof(re));
    reflib_library = NULL;
    reflib_active  = false;
}
/*
==============
VID_LoadRefresh
==============
*/

extern	void	Sys_Warning (char *msg, ...);
extern	bspModel_t	bspModel;
extern	char	*clhr_SurfNames;
extern	char	*clhr_SurfFlags;

typedef struct refexport_s (*game_api_func)(struct refimport_s);

qboolean VID_LoadRefresh( char *name )
{
    refimport_t ri;
    GetRefAPI_t GetRefAPI;
    
    if ( reflib_active )
    {
        re.Shutdown();
        VID_FreeReflib ();
    }
    Com_Printf( "------- Loading %s -------\n", name );
    if ( ( reflib_library = LoadLibrary( name ) ) == 0 )
    {
        Com_Printf( "LoadLibrary(\"%s\") failed\n", name );
        return false;
    }

    ri.Cmd_AddCommand = Cmd_AddCommand;
    ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
    ri.GetArgc = GetArgc;
    ri.GetArgv = GetArgv;
    ri.Cmd_ExecuteText = Cbuf_ExecuteText;
    ri.Con_Printf = VID_Printf;
    ri.Sys_Error = VID_Error;
	ri.Sys_Warning = Sys_Warning;

	ri.FS_FileLength = FS_filelength;
	ri.FS_Close = FS_FCloseFile;
	ri.FS_Open = FS_FOpenFile;
	ri.FS_Read = FS_Read;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Seek = FS_Seek;
	ri.FS_Tell = FS_Tell;
	ri.FS_Getc = FS_Getc;
	ri.FS_Gets = FS_Gets;

	ri.X_Malloc = X_Malloc;
	ri.X_Free = X_Free;
	ri.Mem_Heap_Walk = Mem_Heap_Walk;

    ri.FS_Gamedir = FS_Gamedir;
    ri.Cvar_Get = Cvar_Get;
    ri.Cvar_Set = Cvar_Set;
    ri.Cvar_SetValue = Cvar_SetValue;
    ri.Vid_GetModeInfo = VID_GetModeInfo;
    ri.Vid_MenuInit = VID_MenuInit;
    ri.Vid_NewWindow = VID_NewWindow;
    ri.Vid_SetSDLWindow = VID_SetSDLWindow;
    ri.Vid_RefreshModeList = VID_RefreshModeList;
    
    ri.CL_GenerateVolumeParticles = CL_GenerateVolumeParticles;
    ri.CL_RemoveParticleVolume = CL_RemoveParticleVolume;   
    ri.dk_printxy = dk_printframe_xy;

	ri.beam_AllocBeam = beam_AllocBeam;
	ri.beam_AllocSeg = beam_AllocSeg;
	
	ri.bspModel = &bspModel;

    if ( ( GetRefAPI = (game_api_func) GetProcAddress( reflib_library, "GetRefAPI" ) ) == 0 ) {
        return false;
    }
    re = GetRefAPI( ri );
    if (re.api_version != API_VERSION)
    {
        VID_FreeReflib ();
        Com_Error (ERR_FATAL, "%s has incompatible api_version", name);
    }
    if ( re.Init( NULL, NULL ) == -1 )
    {
        re.Shutdown();
        VID_FreeReflib ();
        return false;
    }
    Com_Printf( "------------------------------------\n");
    reflib_active = true;
    return true;
}
/*
============
VID_CheckChanges
This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to 
update the rendering DLL and/or video mode to match.
============
*/
void VID_CheckChanges (void)
{
    char name[100];
    if ( win_noalttab->modified )
    {
        if ( win_noalttab->value )
        {
            WIN_DisableAltTab();
        }
        else
        {
            WIN_EnableAltTab();
        }
        win_noalttab->modified = false;
    }
    if ( vid_ref->modified )
    {
        cl.force_refdef = true;     // can't use a paused refdef
        if (S_StopAllSounds)
	        S_StopAllSounds();

        //we are about to attempt to change refresh settings.
        VID_ModifySequenceStart();
    }

    while (vid_ref->modified)
    {
        /*
        ** refresh has changed
        */
        vid_ref->modified = false;
        vid_fullscreen->modified = true;
        cl.refresh_prepped = false;
        cls.disable_screen = true;
        Com_sprintf( name, sizeof(name), "ref_%s.dll", vid_ref->string );
	
        //try to load the new refresh.
        if (VID_LoadRefresh(name) == false) 
		{
            //there was an error trying to load the new refresh dll.  load a new set of parameters that
            //we hope will work, either the last successful ones, or a default.
			
            if (VID_ModifySequenceFailure() == true) 
			{
                //we dont have any more modes to try.
//                Com_Error(ERR_FATAL, "Couldn't fall back to software refresh!");
				Com_Error(ERR_FATAL, "An error occured while initializing the opengl driver.");
                return;
            }
			else
			{
		        vid_ref->modified = true;
			}

            //
            // drop the console if we fail to load a refresh
            //
            if ( cls.key_dest != key_console && cls.key_dest != key_menu)
            {
                Con_ToggleConsole_f();
            }
        }
        else {
            //we successfully set the refresh mode, save the settings so we can go back to them
            //if there is an error setting the mode later.
            VID_ModifySequenceSuccess();
        }

        cls.disable_screen = false;

		// SCG[5/28/99]: Initialize the fonts
		VID_InitFonts();

    }
    /*
    ** update our window position
    */
    if ( vid_xpos->modified || vid_ypos->modified )
    {
        if (!vid_fullscreen->value)
            VID_UpdateWindowPosAndSize( vid_xpos->value, vid_ypos->value );
        vid_xpos->modified = false;
        vid_ypos->modified = false;
    }
}
/*
============
VID_Init
============
*/
void VID_Init (void)
{
    /* Create the video variables so we know how to start the graphics drivers */
//    vid_ref = Cvar_Get ("vid_ref", "soft", CVAR_ARCHIVE);
    vid_ref = Cvar_Get ("vid_ref", "gl", CVAR_ARCHIVE);
    vid_xpos = Cvar_Get ("vid_xpos", "3", CVAR_ARCHIVE);
    vid_ypos = Cvar_Get ("vid_ypos", "22", CVAR_ARCHIVE);
    vid_fullscreen = Cvar_Get ("vid_fullscreen", "0", CVAR_ARCHIVE);
    vid_gamma = Cvar_Get( "vid_gamma", "1", CVAR_ARCHIVE );
    win_noalttab = Cvar_Get( "win_noalttab", "0", CVAR_ARCHIVE );

    no_shutdown = Cvar_Get("vid_noshutdown", "0", CVAR_ARCHIVE);

    /* Add some console commands that we want to handle */
    Cmd_AddCommand ("vid_restart", VID_Restart_f);
    Cmd_AddCommand ("vid_front", VID_Front_f);
    /*
    ** this is a gross hack but necessary to clamp the mode for 3Dfx
    */
#if 0
    {
        cvar_t *gl_driver = Cvar_Get( "gl_driver", "opengl32", 0 );
        cvar_t *gl_mode = Cvar_Get( "gl_mode", "3", 0 );
        if ( stricmp( gl_driver->string, "3dfxgl" ) == 0 )
        {
            Cvar_SetValue( "gl_mode", 3 );
            viddef.width  = 640;
            viddef.height = 480;
        }
    }
#endif

    /* Start the graphics mode and load the OpenGL refresh DLL. */
    VID_CheckChanges();
}
/*
============
VID_Shutdown
============
*/
void VID_Shutdown (void)
{
    if (no_shutdown != NULL && no_shutdown->value != 0.0f) return;

    if ( reflib_active )
    {
        re.Shutdown ();
        VID_FreeReflib ();
    }
}


//takes a paramter from 0.5f to 1.3f, with 1.3f being brightest.
void VID_SetGamma(float gamma) {
    //invert the range that was given.
    gamma = 0.8f - (gamma - 0.5f) + 0.5f;
    Cvar_SetValue("vid_gamma", gamma);

    if (stricmp(vid_ref->string, "gl") == 0) {
        /*
        ** Gamma is handled by the OpenGL renderer on the normal Windows
        ** framebuffer path.  The old SSTV2_GAMMA/SST_GAMMA environment
        ** variables were only for 3dfx hardware and are intentionally gone.
        */
        static cvar_t *gl_driver = NULL;
        if (gl_driver == NULL) {
            gl_driver = Cvar_Get("gl_driver", "opengl32", 0);
        }

        if (vid_gamma->modified || gl_driver->modified) {
            vid_ref->modified = true;
        }
    }
}

//
///////////////////////////////
//
//  VID_ModifySequence*  functions manage the changing of refresh setttings. 
//
///////////////////////////////
//

static bool vid_modify_backup_available = false;
static bool vid_modify_backup_tried = false;

void VID_ModifySequenceStart() {
    //reset our backup tried flag.
    vid_modify_backup_tried = false;
}

bool VID_ModifySequenceFailure() {
    //check if we have a backup that we can try that we havent tried already.
    if (vid_modify_backup_available == true && vid_modify_backup_tried == false) {
        //use our backup settings.
        vid_modify_backup_tried = true;
        return false;
    }

/*
    //check if we are already using our default settings.
    if (strcmp(vid_ref->string, "soft") == 0) {
        //we already tried the software refresh, and that failed.
        return true;
    }
        
    //try the default setting of software refresh.
    Cvar_Set("vid_ref", "soft");
*/
	cvar_t	*gl_driver = Cvar_Get( "gl_driver", "opengl32", 0 );

    if (strcmp(gl_driver->string, "opengl32") == 0) 
	{
        //we already tried the software refresh, and that failed.
        return true;
    }

    Cvar_Set("gl_driver", "opengl32");

    return false;
}

void VID_ModifySequenceSuccess() {
    //we successfully changed refresh settings.  save the current settings for later use, if needed.
//    vid_modify_backup_available = true;
}

