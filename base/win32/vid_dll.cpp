// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
#include	<assert.h>
#include	<float.h>
#include	"client.h"
#include	"winquake.h"
#include	"daikatana.h"
#include	"dk_beams.h"
#include	"l__language.h"

#ifdef JPN
#include	<windows.h>
#include	<imm.h>
#include	"dk_menu.h"
#include	"imefull.h" // 4-6-98

#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_model.h"
#include "dk_misc.h"
#endif

//#include "zmouse.h"
// Structure containing functions exported from refresh DLL
refexport_t re;
cvar_t *win_noalttab;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS 
#endif

void VID_ModifySequenceStart();
bool VID_ModifySequenceFailure();
void VID_ModifySequenceSuccess();

static UINT MSH_MOUSEWHEEL;
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
extern HWND        cl_hwnd;            // Main window handle for life of program

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )
LONG WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static qboolean s_alttab_disabled;

extern  unsigned    sys_msg_time;

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


#ifdef JPN

UINT GetKeyBuffer(UINT key);
void SetKeyBuffer(UINT key, UINT chr);

bool Skip_KeyPad = false;
int key_down = 0;
int lastKey = 0; // for IME / added by yokoyama

#define KEY_BUFFER_LEN 20

// added for Japanese version for KEY_BUG fix
struct KEYBUFFERST{
	UINT key;
	UINT chr;
}keyBuffer[KEY_BUFFER_LEN];

UINT keyBufferBak;

extern CInterfaceInputField *CurrentlyEditingField();
extern void ImeKey_Console (int leadbyte, int trailbyte);
extern void DisableIme(bool fFlag);
#endif // JPN

/*
** WIN32 helper functions
*/
extern qboolean s_win95;
static void WIN_DisableAltTab( void )
{
    if ( s_alttab_disabled )
        return;

    if ( s_win95 )
    {
        BOOL old;

        SystemParametersInfo( SPI_SCREENSAVERRUNNING, 1, &old, 0 );
    }
    else
    {
        RegisterHotKey( 0, 0, MOD_ALT, VK_TAB );
        RegisterHotKey( 0, 1, MOD_ALT, VK_RETURN );
    }
    s_alttab_disabled = true;
}
static void WIN_EnableAltTab( void )
{
    if ( s_alttab_disabled )
    {
        if ( s_win95 )
        {
            BOOL old;

            SystemParametersInfo( SPI_SCREENSAVERRUNNING, 0, &old, 0 );
        }
        else
        {
            UnregisterHotKey( 0, 0 );
            UnregisterHotKey( 0, 1 );
        }
        s_alttab_disabled = false;
    }
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

#ifdef JPN
typedef struct
{
	int		vk;
	int		dk;
} VKmap_t;

int DisableKPChar[]=
{
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_DECIMAL,
	VK_SEPARATOR,
	VK_SUBTRACT,
	VK_ADD,
	VK_ESCAPE,
	VK_TAB,
	VK_BACK,
	VK_SPACE,
	NULL
};

VKmap_t VKkey[] =
{

	{VK_F1, K_F1},
	{VK_F2, K_F2},
	{VK_F3, K_F3},
	{VK_F4, K_F4},
	{VK_F5, K_F5},
	{VK_F6, K_F6},
	{VK_F7, K_F7},
	{VK_F8, K_F8},
	{VK_F9, K_F9},
	{VK_F10, K_F10},
	{VK_F11, K_F11},
	{VK_F12, K_F12},

	{VK_TAB, K_TAB},
	{VK_RETURN, K_ENTER},
	{VK_ESCAPE, K_ESCAPE},
	{VK_SPACE, K_SPACE},
	{VK_UP, K_UPARROW},
	{VK_DOWN, K_DOWNARROW},
	{VK_LEFT, K_LEFTARROW},
	{VK_RIGHT, K_RIGHTARROW},
	{VK_BACK, K_BACKSPACE},

	{VK_MENU, K_ALT},
	{VK_CONTROL, K_CTRL},
	{VK_SHIFT, K_SHIFT},

	{VK_INSERT, K_INS},
	{VK_DELETE, K_DEL},
	{VK_NEXT, K_PGDN},
	{VK_PRIOR, K_PGUP},
	{VK_HOME, K_HOME},
	{VK_END, K_END},

	{VK_NUMPAD7,			K_KP_HOME },
	{VK_NUMPAD8,		K_KP_UPARROW },
	{VK_NUMPAD9,			K_KP_PGUP },
	{VK_NUMPAD6,	K_KP_LEFTARROW },
	{VK_NUMPAD5,			K_KP_5 },
	{VK_NUMPAD4,	K_KP_RIGHTARROW },
	{VK_NUMPAD1,			K_KP_END },
	{VK_NUMPAD2,	K_KP_DOWNARROW },
	{VK_NUMPAD3,			K_KP_PGDN },
	{VK_RETURN,		K_KP_ENTER },
	{VK_NUMPAD0,			K_KP_INS },
	{VK_DECIMAL,			K_KP_DEL },
	{VK_SEPARATOR,		K_KP_SLASH },
	{VK_SUBTRACT,		K_KP_MINUS },
	{VK_ADD,			K_KP_PLUS },

	{VK_PAUSE, K_PAUSE},

	{NULL,0}
};

int MapKey(int key,int lParam)
{
	VKmap_t *kn;
	int result;

	// skip the char when user type in keypad
	int *iChar;
	Skip_KeyPad = false;
	for (iChar = DisableKPChar; *iChar; iChar++)
		if (*iChar==key)
			Skip_KeyPad = true;

	result = 0;
	for (kn = VKkey; kn->vk ; kn++)
	{
		if (kn->vk == key)
		{
			result = kn->dk;
			break;
		}
	}

    qboolean is_extended = false;

    if ( lParam & ( 1 << 24 ) )
        is_extended = true;
    if ( !is_extended )
    {
        switch ( result )
        {
        case K_HOME:
            return K_KP_HOME;
        case K_UPARROW:
            return K_KP_UPARROW;
        case K_PGUP:
            return K_KP_PGUP;
        case K_LEFTARROW:
            return K_KP_LEFTARROW;
        case K_RIGHTARROW:
            return K_KP_RIGHTARROW;
        case K_END:
            return K_KP_END;
        case K_DOWNARROW:
            return K_KP_DOWNARROW;
        case K_PGDN:
            return K_KP_PGDN;
        case K_INS:
            return K_KP_INS;
        case K_DEL:
            return K_KP_DEL;
        default:
            return result;
        }
    }
		return result;
}


void SetKeyBuffer(UINT key, UINT chr)
{
	UINT count = 0;

	if(key == 0x20 || key == 0x09 || key == 0x0d || key == 0x1b)
		return;

	while(keyBuffer[count].key != 0 && count < KEY_BUFFER_LEN)
		count++;

	keyBuffer[count].key = key;
	keyBuffer[count].chr = chr;

}

UINT GetKeyBuffer(UINT key)
{
	UINT count, result;

	count = 0;
	while(keyBuffer[count].key != key && count < KEY_BUFFER_LEN)
		count++;

	result = keyBuffer[count].chr;

	if(count > KEY_BUFFER_LEN)
		return 0;

	keyBuffer[count].key = 0;
	keyBuffer[count].chr = 0;

	return result;
}


#else // JPN

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
/*
=======
MapKey
Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
    int result;
    int modified = ( key >> 16 ) & 255;
    qboolean is_extended = false;
    if ( modified > 127)
        return 0;

    if ( key & ( 1 << 24 ) )
        is_extended = true;
    result = scantokey[modified];

    if ( !is_extended )
    {
        switch ( result )
        {
        case K_HOME:
            return K_KP_HOME;
        case K_UPARROW:
            return K_KP_UPARROW;
        case K_PGUP:
            return K_KP_PGUP;
        case K_LEFTARROW:
            return K_KP_LEFTARROW;
        case K_RIGHTARROW:
            return K_KP_RIGHTARROW;
        case K_END:
            return K_KP_END;
        case K_DOWNARROW:
            return K_KP_DOWNARROW;
        case K_PGDN:
            return K_KP_PGDN;
        case K_INS:
            return K_KP_INS;
        case K_DEL:
            return K_KP_DEL;
        default:
            return result;
        }
    }
    else
    {
        switch ( result )
        {
        case 0x0D:
            return K_KP_ENTER;
        case 0x2F:
            return K_KP_SLASH;
        case 0xAF:
            return K_KP_PLUS;
        }
        return result;
    }
}

#endif // JPN

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
====================
MainWndProc
main window procedure
====================
*/
void DKM_SetMenuPalette();
bool DKM_InMenu();

#ifndef JPN
void DKM_Enter(dk_main_button force_menu = DKMB_FIRST_MENU_IN); // thanks for the hack prototype
#endif
void DKM_Leave();

LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG            lRet = 0;

#ifdef JPN
	static	chCharCode1 = NULL;
	bool is_extended;
	int mykey;
#endif // JPN

	// SCG[11/17/99]: Don't accept mousewheel messages while paused per John's request
//	if( cl_paused != NULL && !cl_paused->value )
	{
		if ( uMsg == MSH_MOUSEWHEEL )
		{
			if ( ( ( int ) wParam ) > 0 )
			{
				Key_Event( K_MWHEELUP, true, sys_msg_time );
				Key_Event( K_MWHEELUP, false, sys_msg_time );
			}
			else
			{
				Key_Event( K_MWHEELDOWN, true, sys_msg_time );
				Key_Event( K_MWHEELDOWN, false, sys_msg_time );
			}
			return DefWindowProc (hWnd, uMsg, wParam, lParam);
		}
	}
    switch (uMsg)
    {
    case WM_MOUSEWHEEL:
        /*
        ** this chunk of code theoretically only works under NT4 and Win98
        ** since this message doesn't exist under Win95
        */
		// SCG[11/17/99]: Don't accept mousewheel messages while paused per John's request
//		if( cl_paused != NULL && !cl_paused->value )
		{
			if ( ( short ) HIWORD( wParam ) > 0 )
			{
				Key_Event( K_MWHEELUP, true, sys_msg_time );
				Key_Event( K_MWHEELUP, false, sys_msg_time );
			}
			else
			{
				Key_Event( K_MWHEELDOWN, true, sys_msg_time );
				Key_Event( K_MWHEELDOWN, false, sys_msg_time );
			}
		}
        break;
    case WM_HOTKEY:
        return 0;

    case WM_CREATE:
#ifdef JPN //4-6-99 FrankZ
	ImeInit(hWnd);
#endif
        cl_hwnd = hWnd;

        MSH_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG"); 
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    case WM_PAINT:
        SCR_DirtyScreen (); // force entire screen to update next frame
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    case WM_DESTROY:
        // let sound and input know about this?
		ShowWindow( cl_hwnd, SW_HIDE );
        cl_hwnd = NULL;
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    case WM_ACTIVATE:
        {
            int fActive, fMinimized;
            // KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
            fActive = LOWORD(wParam);
            fMinimized = (BOOL) HIWORD(wParam);
            AppActivate( fActive != WA_INACTIVE, fMinimized);
            if ( reflib_active )
            {
			    re.AppActivate( !( fActive == WA_INACTIVE ) );
				if( !( fActive == WA_INACTIVE ) )
				{
					if( DKM_InMenu() )
					{
						DKM_SetMenuPalette();
					}
				}
#ifdef _DEBUG
				// ISP - while debugging in single player mode, make this app sleep,
				// a hack
//				int bSinglePlayerMode = !Cvar_VariableValue("deathmatch");
//				if ( bSinglePlayerMode )
				{
					bAppActive = fActive;
				}
#endif _DEBUG

			}

        }
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    case WM_MOVE:
        {
            int     xPos, yPos;
            RECT r;
            int     style;
            if (!vid_fullscreen->value)
            {
                xPos = (short) LOWORD(lParam);    // horizontal position 
                yPos = (short) HIWORD(lParam);    // vertical position 
                r.left   = 0;
                r.top    = 0;
                r.right  = 1;
                r.bottom = 1;
                style = GetWindowLong( hWnd, GWL_STYLE );
                AdjustWindowRect( &r, style, FALSE );
                Cvar_SetValue( "vid_xpos", xPos + r.left);
                Cvar_SetValue( "vid_ypos", yPos + r.top);
                vid_xpos->modified = false;
                vid_ypos->modified = false;
                if (ActiveApp)
                    IN_Activate (true);
            }
        }
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        {
            int temp;
            temp = 0;
            if (wParam & MK_LBUTTON)
                temp |= 1;
            if (wParam & MK_RBUTTON)
                temp |= 2;
            if (wParam & MK_MBUTTON)
                temp |= 4;
            IN_MouseEvent (temp);
        }
        break;

    case WM_SYSCOMMAND:
        if ( wParam == SC_SCREENSAVE )
            return 0;
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    case WM_SYSKEYDOWN:
        if ( wParam == 13 )
        {
            if ( vid_fullscreen )
            {
                Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->value );
            }
            return 0;
        }
        // fall through
    case WM_KEYDOWN:
#ifdef JPN
		lastKey = lParam; // for IME / added by yokoyama
		mykey =  MapKey(wParam, lParam);
        if ( (gImeUIData.ImeState & IME_ON) && 
			(DKM_InMenu() == true || 
            cls.key_dest == key_console ||
            (cls.key_dest == key_game && cls.state != ca_active)) )
		{
			if ( (gImeUIData.ImeState & IME_IN_COMPOSITION) || (mykey != K_ENTER && mykey != K_ESCAPE && mykey<127))
				break;
			if ( (gImeUIData.ImeState & IME_ON) && mykey == K_ESCAPE )
				// disable IME
				DisableIme(false);

		}
		key_down = wParam; // for key backup / added by yokoyama
        Key_Event( mykey, true, sys_msg_time);
#else // JPN
        Key_Event( MapKey( lParam ), true, sys_msg_time);
#endif
        break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
#ifdef JPN
        if ( gImeUIData.ImeState)
			break;
		mykey =  MapKey(wParam, lParam);
		if (!mykey) // get original char
		{
			mykey = GetKeyBuffer(wParam);
			//mykey = key_down; // got from WM_CHAR
		}
		keyBufferBak = 0; // added by yokoyama
		Key_Event( mykey, false, sys_msg_time);
#else // JPN
        Key_Event( MapKey( lParam ), false, sys_msg_time);
#endif // JPN
        break;


// Double-byte
#ifdef JPN
    case WM_CHAR:
		if (!gImeUIData.ImeState) 
		{
			// IME isn't be actived, save the key and then pass to WM_KEYUP
			//key_down = wParam; // fixed WM_KEYUP

			// add keyBuffer / added by yokoyama
			if(keyBufferBak != wParam)
			{
				SetKeyBuffer(key_down, wParam); 
				keyBufferBak = wParam;
			}
		}
		if (chCharCode1) {
			CInterfaceInputField *edit;
			edit = CurrentlyEditingField();
//#ifdef CONSOLE
			if (edit)
				edit->ImeKeydown(chCharCode1, wParam); // DK_MENU
			else if (cls.key_dest == key_console)
			{
				ImeKey_Console (chCharCode1, wParam); // DK_CONSOLE
			}
/*#else
			if (edit)
				edit->ImeKeydown(chCharCode1, wParam); // DK_MENU
#endif*/ // CONSOLE
			// clear the lead byte
			chCharCode1 = NULL;
			break;
		}

		if (IsDBCSLeadByte(wParam)) {
			chCharCode1 = wParam;
			break; // wait for trailbyte for DBCS
		}

		if (wParam >= 0xa1 && wParam <= 0xfe) {
			CInterfaceInputField *edit;
			edit = CurrentlyEditingField();
//#ifdef CONSOLE
			if (edit)
				edit->ImeKeydown(0, wParam); //Hankaku
			else if (cls.key_dest == key_console)
			{
				ImeKey_Console (0, wParam); // DK_CONSOLE
			}
/*#else
			if (edit)
				edit->ImeKeydown(0, wParam); //Hankaku
#endif*/
			break;
		}

		is_extended = false;

		if ( Skip_KeyPad ) // ignore the keypad input
			break;

        Key_Event(wParam, true, sys_msg_time);
		break;

    case WM_IME_SETCONTEXT:
		lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
		return DefWindowProc( hWnd, uMsg, wParam, lParam );

	case WM_IME_STARTCOMPOSITION:
		// Reset global variables.
		gImeUIData.uCompLen = 0;             // length of composition string.
		gImeUIData.ImeState |= IME_IN_COMPOSITION;
		break;
		//return 0;

    case WM_IME_ENDCOMPOSITION:
		ImeUIEndComposition();
		break;

    case WM_IME_COMPOSITION:
        ImeUIComposition( hWnd, wParam, lParam );
	    if ( lParam & GCS_COMPSTR )
			return 0;
		break;
	case WM_IME_NOTIFY:
		if (ImeUINotify( hWnd, wParam, lParam ))
			return 0;
		break;
#endif // JPN

    case MM_MCINOTIFY:
        {
            LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
            lRet = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
        }
        break;
    default:    // pass all unhandled messages to DefWindowProc
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    /* return 0 if handled message, 1 if not */
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
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
    SetWindowLong( cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST );
    SetForegroundWindow( cl_hwnd );
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
vidmode_t vid_modes[] =
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
qboolean VID_GetModeInfo( int *width, int *height, int mode )
{
    if ( mode < 0 || mode >= VID_NUM_MODES )
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
    RECT r;
    int     style;
    int     w, h;

    r.left   = 0;
    r.top    = 0;
    r.right  = viddef.width;
    r.bottom = viddef.height;
    style = GetWindowLong( cl_hwnd, GWL_STYLE );
    AdjustWindowRect( &r, style, FALSE );
    w = r.right - r.left;
    h = r.bottom - r.top;
    MoveWindow( cl_hwnd, vid_xpos->value, vid_ypos->value, w, h, TRUE );
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
    if ( re.Init( global_hInstance, MainWndProc ) == -1 )
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

    /* Disable the 3Dfx splash screen */
    _putenv("FX_GLIDE_NO_SPLASH=0");
        
    /* Start the graphics mode and load refresh DLL */
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
        //get the gl_driver cvar.
        static cvar_t *gl_driver = NULL;
        if (gl_driver == NULL) {
            gl_driver = Cvar_Get("gl_driver", "opengl32", 0);
        }

        if (vid_gamma->modified) {
            vid_ref->modified = true;
            if (stricmp(gl_driver->string, "3dfxgl") == 0) {
                char envbuffer[1024];
                float g;
                vid_ref->modified = true;
                g = 2.00 * ( 0.8 - ( vid_gamma->value - 0.5 ) ) + 1.0F;
                Com_sprintf( envbuffer, sizeof(envbuffer), "SSTV2_GAMMA=%f", g );
                putenv( envbuffer );
                Com_sprintf( envbuffer, sizeof(envbuffer), "SST_GAMMA=%f", g );
                putenv( envbuffer );
                vid_gamma->modified = false;
            }
        }

        if (gl_driver->modified) {
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

