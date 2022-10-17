#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <mntent.h>

#include <dlfcn.h>

#include "../qcommon/qcommon.h"
#include "daikatana.h"

cvar_t *nostdout;

unsigned	sys_frame_time;

qboolean stdin_active = true;

// =======================================================================
// General routines
// =======================================================================

void Sys_ConsoleOutput (char *string)
{
	if (nostdout && nostdout->value)
		return;

	fputs(string, stdout);

	// mpath
	fflush (stdout);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];
	unsigned char		*p;

	va_start (argptr,fmt);
	vsprintf (text,fmt,argptr);
	va_end (argptr);

	if (strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

    if (nostdout && nostdout->value)
        return;

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

void Sys_Quit (void)
{
	CL_Shutdown ();
	Qcommon_Shutdown ();
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	_exit(0);
}

void Sys_Init(void)
{
#if id386
//	Sys_SetFPCW();
#endif
}

void Sys_Error (char *error, ...)
{ 
    va_list     argptr;
    char        string[1024];

// change stdin to non blocking
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    
    va_start (argptr,error);
    vsprintf (string,error,argptr);
    va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	CL_Shutdown ();
	Qcommon_Shutdown ();
	_exit (1);

} 

void Sys_Warning (char *warning, ...)
{ 
    va_list     argptr;
    char        string[1024];
    
    va_start (argptr,warning);
    vsprintf (string,warning,argptr);
    va_end (argptr);
	fprintf(stderr, "Warning: %s", string);
} 

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int	Sys_FileTime (char *path)
{
	struct	stat	buf;
	
	if (stat (path,&buf) == -1)
		return -1;
	
	return buf.st_mtime;
}

void floating_point_exception_handler(int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal(SIGFPE, floating_point_exception_handler);
}

char *Sys_ConsoleInput(void)
{
    static char text[256];
    int     len;
	fd_set	fdset;
    struct timeval timeout;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read (0, text, sizeof(text));
	if (len == 0) { // eof!
		stdin_active = false;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len-1] = 0;    // rip off the /n and terminate

	return text;
}

/*****************************************************************************/

#if 0
static void *game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame (void)
{
	if (game_library) 
		dlclose (game_library);
	game_library = NULL;
}

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);

	char	name[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*path;
	const char *gamename = "game.so";
#if defined __sun__
	const char *gamename = "gamesparc.so";
#endif

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	getcwd(curpath, sizeof(curpath));

	// mpath - added '\n'
	Com_Printf("------- Loading %s -------\n", gamename);

	// now run through the search paths
	path = NULL;
	while (1)
	{
		path = FS_NextPath (path);
		if (!path)
			return NULL;		// couldn't find one anywhere
		sprintf (name, "%s/%s/%s", curpath, path, gamename);
		game_library = dlopen (name, RTLD_NOW );
		if (game_library)
		{
			Com_DPrintf ("dlopen (%s)\n",name);
			break;
		} else
			Com_Printf("error: %s\n", dlerror());
	}

	GetGameAPI = (void *)dlsym (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();		
		return NULL;
	}

	return GetGameAPI (parms);
}
#endif

/*****************************************************************************/

void Sys_AppActivate (void)
{
}

void Sys_SendKeyEvents (void)
{
	// grab frame time 
	sys_frame_time = Sys_Milliseconds();
}

/*****************************************************************************/

char *Sys_GetClipboardData(void)
{
	return NULL;
}

/*
========================================================================

PHYSICS DLL

========================================================================
*/

static void* game_library;

///////////////////////////////////////////////////////////////////////////////
//	Sys_UnloadPhysicsDLL
//
//	unloads the physics DLL on "map" commands or at game exit
///////////////////////////////////////////////////////////////////////////////

void Sys_UnloadPhysicsDLL (void)
{
	if (dlclose (game_library))
		Com_Error (ERR_FATAL, "dlclose failed for game library");

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

	Com_sprintf (dllName, MAX_OSPATH, "./%s/%s", dk_dllDir, gamename);
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

	Sys_GetPhysicsDLLName (name);
	Com_DPrintf ("dlopen %s\n", name);
	game_library = dlopen ( name, RTLD_NOW );

	if (!game_library)
	{
		Com_DPrintf ("Could not find physics.so at %s.\n", name);
		Com_DPrintf ("dlerror: %s\n", dlerror());
		return	NULL;
	}

	GetGameAPI = (void *(*)(void *)) dlsym (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Com_DPrintf("dlsym failed for GetGameAPI\n");
		Com_DPrintf ("dlerror: %s\n", dlerror());

		Sys_UnloadPhysicsDLL ();
		return NULL;
	}

	//	call physics.dll init function, which will load up other DLLs
	return (GetGameAPI (parms));
}

#if 0

//=======================================================================

int main (int argc, char **argv)
{
	int 	time, oldtime, newtime;

#if 0
	int newargc;
	char **newargv;
	int i;

	// force dedicated
	newargc = argc;
	newargv = malloc((argc + 3) * sizeof(char *));
	newargv[0] = argv[0];
	newargv[1] = "+set";
	newargv[2] = "dedicated";
	newargv[3] = "1";
	for (i = 1; i < argc; i++)
		newargv[i + 3] = argv[i];
	newargc += 3;

	Qcommon_Init(newargc, newargv);
#else
	Qcommon_Init(argc, argv);
#endif

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0);

	if (!nostdout->value) {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	}

    oldtime = Sys_Milliseconds ();
    while (1)
    {
// find time spent rendering last frame
		do {
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
        Qcommon_Frame (time);
		oldtime = newtime;
    }

}

#endif

void Sys_CopyProtect(void)
{
	return;
}

#if 0
/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned int startaddr, unsigned int length)
{

	int r;
	unsigned int addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

//	fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//			addr, startaddr+length, length);

	r = mprotect((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");

}

#endif

//=======================================================================

void Sys_SleepMilliseconds (int ms)
{
    struct timeval tv;

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms - (tv.tv_sec * 1000)) * 1000;

    select (0, NULL, NULL, NULL, &tv);
}

//=======================================================================

int main (int argc, char **argv)
{
    int     time, oldtime, newtime, frametime;

    Qcommon_Init(argc, argv);

    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
    nostdout = Cvar_Get ("nostdout", "0", 0);
    if (!nostdout->value)
        fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

    oldtime = Sys_Milliseconds ();
    while (1)
    {
        newtime = Sys_Milliseconds ();
        time = newtime - oldtime;
        oldtime = newtime;

        Qcommon_Frame (time);

        frametime = Sys_Milliseconds () - newtime;
        if (frametime < SV_FRAME_MILLISECONDS)
            Sys_SleepMilliseconds (SV_FRAME_MILLISECONDS - frametime);
    }
}
