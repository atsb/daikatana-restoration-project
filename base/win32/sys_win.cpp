// sys_win.h

#include "qcommon.h"
#include "winquake.h"
#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include "../win32/conproc.h"
#include "../dk_/daikatana.h"

// memory debug macros
#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

#define MINIMUM_WIN_MEMORY	0x0a00000
#define MAXIMUM_WIN_MEMORY	0x1000000

#define	DEMO

qboolean	s_win95;
int			starttime;
int			ActiveApp;
qboolean	Minimized;

static HANDLE		hinput, houtput;

unsigned	sys_msg_time;
unsigned	sys_frame_time;

static HANDLE		qwclsemaphore;

#define	MAX_NUM_ARGVS	128

int			argc;
char		*argv[MAX_NUM_ARGVS];

extern int bAppActive;

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void	Sys_Warning (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	MessageBox(NULL, text, "Warning", 0 /* MB_OK */);
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	CL_Shutdown ();
	Qcommon_Shutdown ();

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	MessageBox(NULL, text, "Error", 0 /* MB_OK */ );

	if (qwclsemaphore)
		CloseHandle (qwclsemaphore);

	// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (1);
}

extern cvar_t *no_shutdown;

void Sys_Quit (void)
{
	timeEndPeriod( 1 );

	if (qwclsemaphore)
		CL_Shutdown();

	CloseHandle (qwclsemaphore);

	if (dedicated && dedicated->value)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

    if (no_shutdown != NULL && no_shutdown->value != 0.0) {
        TerminateProcess(GetCurrentProcess(), 0);
    }

	Qcommon_Shutdown ();

	exit (1);
}


void WinError (void)
{
	LPVOID lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	// Display the string.
	MessageBox( NULL, (const char *)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}

//================================================================

/*
================
Sys_ScanForCD

================
*/
char *Sys_ScanForCD (void)
{
	static char	cddir[MAX_OSPATH];
	static qboolean	done;
#ifndef DEMO
	char		drive[4];
	FILE		*f;
	char		test[MAX_QPATH];

	if (done)		// don't re-check
		return cddir;

	// no abort/retry/fail errors
	SetErrorMode (SEM_FAILCRITICALERRORS);

	drive[0] = 'c';
	drive[1] = ':';
	drive[2] = '\\';
	drive[3] = 0;

	done = true;

	// scan the drives
	for (drive[0] = 'd' ; drive[0] <= 'z' ; drive[0]++)
	{
		// where activision put the stuff...

		// LAB - 19991203 - Stuff activision, this is where Eidos put stuff.

// SCG[1/16/00]: 		sprintf (cddir, "%sdata", drive);
		Com_sprintf (cddir,sizeof(cddir), "%sdata", drive);
// SCG[1/16/00]: 		sprintf (test, "%sdata\\pak1.pak", drive);
		Com_sprintf (test,sizeof(test), "%sdata\\pak1.pak", drive);
		f = fopen(test, "r");
		if (f)
		{
			fclose (f);
			if (GetDriveType (drive) == DRIVE_CDROM)
				return cddir;
		}
	}
#endif

	cddir[0] = 0;
	
	return NULL;
}

/*
================
Sys_CopyProtect

================
*/
void	Sys_CopyProtect (void)
{
#ifndef DEMO
	char	*cddir;

	cddir = Sys_ScanForCD();
	if (!cddir[0])
		Com_Error (ERR_FATAL, "You must have the Daikatana CD in the drive to play.");
#endif
}

//================================================================

/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	OSVERSIONINFO	vinfo;

#if 0
	// allocate a named semaphore on the client so the
	// front end can tell if it is alive

	// mutex will fail if semephore already exists
    qwclsemaphore = CreateMutex(
        NULL,         /* Security attributes */
        0,            /* owner       */
        "qwcl"); /* Semaphore name      */
	if (!qwclsemaphore)
		Sys_Error ("QWCL is already running on this system");
	CloseHandle (qwclsemaphore);

    qwclsemaphore = CreateSemaphore(
        NULL,         /* Security attributes */
        0,            /* Initial count       */
        1,            /* Maximum count       */
        "qwcl"); /* Semaphore name      */
#endif

	timeBeginPeriod( 1 );

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if (vinfo.dwMajorVersion < 4)
		Sys_Error ("Daikatana requires Windows version 4.0 or greater");

	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error ("Daikatana will not run on Win32s");
	else if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		s_win95 = true;

	if (dedicated->value)
	{
		if (!AllocConsole ())
			Sys_Error ("Couldn't create dedicated server console");
		hinput = GetStdHandle (STD_INPUT_HANDLE);
		houtput = GetStdHandle (STD_OUTPUT_HANDLE);

		// let QHOST hook in
		InitConProc (argc, argv);
	}
}

static char	console_text[256];
static int	console_textlen;

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	int		dummy;
	int		ch, numread, numevents;
	if (!dedicated || !dedicated->value)
		return NULL;
	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, (unsigned long *)&numevents))
			Sys_Error ("Error getting # of console events");
		if (numevents <= 0)
			break;
		if (!ReadConsoleInput(hinput, recs, 1, (unsigned long *)&numread))
			Sys_Error ("Error reading console input");
		if (numread != 1)
			Sys_Error ("Couldn't read console input");
		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;
				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, (unsigned long *)&dummy, NULL);	
						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;
					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, (unsigned long *)&dummy, NULL);	
						}
						break;
					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, (unsigned long *)&dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}
						break;
				}
			}
		}
	}
	return NULL;
}
/*
================
Sys_ConsoleOutput
Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
	int		dummy;
	char	text[256];
	if (!dedicated || !dedicated->value)
		return;

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, (unsigned long *)&dummy, NULL);
	}
	WriteFile(houtput, string, strlen(string), (unsigned long *)&dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, (unsigned long *)&dummy, NULL);
}
/*
================
Sys_SendKeyEvents
Send Key_Event calls
================
*/
void Sys_SendKeyEvents (void)
{
    MSG        msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
		sys_msg_time = msg.time;
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	// grab frame time 
	sys_frame_time = timeGetTime();	// FIXME: should this be at start?
}
/*
================
Sys_GetClipboardData

================
*/
char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) != 0 )
	{
		HANDLE hClipboardData;

		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 )
		{
			if ( ( cliptext = (char *)GlobalLock( hClipboardData ) ) != 0 ) 
			{
				data = (char *)X_Malloc( GlobalSize( hClipboardData ) + 1, MEM_TAG_MISC );
				strcpy( data, cliptext );
				GlobalUnlock( hClipboardData );
			}
		}
		CloseClipboard();
	}
	return data;
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
=================
Sys_AppActivate
=================
*/
void Sys_AppActivate (void)
{
	ShowWindow ( cl_hwnd, SW_RESTORE);
	SetForegroundWindow ( cl_hwnd );
}

void Sys_Minimize(void)
{
	ShowWindow ( cl_hwnd, SW_MINIMIZE);
}
/*
========================================================================

PHYSICS DLL

========================================================================
*/

static HINSTANCE	game_library;

///////////////////////////////////////////////////////////////////////////////
//	Sys_UnloadPhysicsDLL
//
//	unloads the physics DLL on "map" commands or at game exit
///////////////////////////////////////////////////////////////////////////////

void Sys_UnloadPhysicsDLL (void)
{
	if (!FreeLibrary (game_library))
		Com_Error (ERR_FATAL, "FreeLibrary failed for game library");

	game_library = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	Sys_GetPhysicsDllName
//
//	looks at command line parameters to determine if the user wants to override
//	the location of the physics DLL (normally data\dlls)
//
//	should pass the default physics
///////////////////////////////////////////////////////////////////////////////

void	Sys_GetPhysicsDLLName (char *dllName)
{
	int		i;
	char	temp [MAX_OSPATH];

	for (i = 0; i < EXEGetArgc(); i++)
	{
		strcpy (temp, EXEGetArgv(i));
		temp [4] = 0x00;

		if (!stricmp (temp, "-dll"))
		{
			strcpy (temp, EXEGetArgv(i + 1));

			//	this command is a -dll override
			if (!stricmp (temp + strlen (temp) - strlen (gamename), gamename))
			{
				//	overriding physics.dll, so return this arg as the name
				strcpy (dllName, temp);
				return;
			}
		}
	}

	Com_sprintf (dllName, MAX_OSPATH, "%s\\%s", dk_dllDir, gamename);
}

///////////////////////////////////////////////////////////////////////////////
//	Sys_GetGameAPI
//	
//	Loads the game dll
///////////////////////////////////////////////////////////////////////////////

void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];

	if (game_library)
	{
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");
	}

	Com_DPrintf ("LoadLibrary()\n");

	Sys_GetPhysicsDLLName (name);

	game_library = LoadLibrary ( name );

	if (!game_library)
	{
		Com_DPrintf ("Could not find PHYSICS.DLL at %s.\n", name);
		return	NULL;
	}

	GetGameAPI = (void *(*)(void *))GetProcAddress (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Com_DPrintf("GetProcAddress failed for GetGameAPI\n");

		Sys_UnloadPhysicsDLL ();
		return NULL;
	}

	//	call physics.dll init function, which will load up other DLLs
	return (GetGameAPI (parms));
}

//=======================================================================


/*
==================
ParseCommandLine

==================
*/
void ParseCommandLine (LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
			
		}
	}

}

/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG				msg;
	int				time, oldtime, newtime;
	char			*cddir;

	long			lcid, langid;

	// check for leaks 
	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    /* previous instances do not exist in Win32 */
    if (hPrevInstance)
        return 0;

	if (_access ("./kick.dat", 0) != -1)
	{
		WinExec( "./PleaseWait.exe", 0 ); 
		Com_Quit ();
		return 0;
	}

#ifdef DAIKATANA_DEMO

	//AJR 04/18/2000: Check for a German machine -- if so, exit with a dialogue
 	lcid = GetUserDefaultLCID();
 	langid = GetSystemDefaultLangID();
 
	if ((lcid == 1031) || (lcid == 5127)) 
	{
		MessageBox(0, 
			// This program is incompatible with German computers and so will not run
			"Dieses Programm ist mit deutschen Computern nicht kompatibel und wird nicht laufen.",
			// warning
			"Warnung",
			MB_OK | MB_ICONEXCLAMATION);

		exit(1);
 	}

#endif

// added for Japanese version
#ifdef JPN
#ifdef DAIKATANA_DEMO
	if(!strstr( lpCmdLine, "JPDEMO"))
		exit(0);
#else
	if(!strstr( lpCmdLine, "JPPRODUCT"))
		exit(0);
#endif // DAIKATANA_DEMO
#endif // JPN

	global_hInstance = hInstance;
	ParseCommandLine (lpCmdLine);
	// if we find the CD, add a +set cddir xxx command line
	cddir = Sys_ScanForCD ();
	if (cddir && argc < MAX_NUM_ARGVS - 3)
	{
		int		i;

		// don't override a cddir on the command line
		for (i=0 ; i<argc ; i++)
			if (!strcmp(argv[i], "cddir"))
				break;
		if (i == argc)
		{
			argv[argc++] = "+set";
			argv[argc++] = "cddir";
			argv[argc++] = cddir;
		}
	}
	Qcommon_Init (argc, argv);
	oldtime = Sys_Milliseconds ();
    /* main window message loop */
	while (1)
	{
#ifdef _DEBUG
		if ( !bAppActive )
		{
			Sleep( 1 );
		}
#endif _DEBUG

		// if at a full screen console, don't update unless needed
		if (Minimized || (dedicated && dedicated->value) )
		{
			Sleep (1);
		}

		while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage (&msg, NULL, 0, 0))
				Com_Quit ();
			sys_msg_time = msg.time;
			TranslateMessage (&msg);
   			DispatchMessage (&msg);
		}
		do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
//			Con_Printf ("time:%5.2f - %5.2f = %5.2f\n", newtime, oldtime, time);

		//	_controlfp( ~( _EM_ZERODIVIDE /*| _EM_INVALID*/ ), _MCW_EM );
		_controlfp( _PC_24, _MCW_PC );
		Qcommon_Frame (time);
		oldtime = newtime;
	}
	// never gets here
    return TRUE;
}
