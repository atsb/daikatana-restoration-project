// vid_null.c -- null video driver to aid porting efforts
// this assumes that one of the refs is statically linked to the executable

#include "../client/client.h"

viddef_t	viddef;				// global video state

refexport_t	re;

refexport_t GetRefAPI (refimport_t rimp);

/*
==========================================================================

DIRECT LINK GLUE

==========================================================================
*/

#define	MAXPRINTMSG	4096
void VID_Printf (int print_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

        if (print_level == PRINT_ALL)
                Com_Printf ("%s", msg);
        else
                Com_DPrintf ("%s", msg);
}

void VID_Error (int err_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

		Com_Error (err_level, "%s", msg);
}

void VID_NewWindow (int width, int height)
{
        viddef.width = width;
        viddef.height = height;
}


void	VID_Init (void)
{
    refimport_t	ri;

    viddef.width = 320;
    viddef.height = 240;

    ri.Cmd_AddCommand = Cmd_AddCommand;
    ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
    ri.Cmd_Argc = Cmd_Argc;
    ri.Cmd_Argv = Cmd_Argv;
    ri.Cmd_ExecuteText = Cbuf_ExecuteText;
    ri.Con_Printf = VID_Printf;
    ri.Sys_Error = VID_Error;
    ri.FS_LoadFile = FS_LoadFile;
    ri.FS_FreeFile = FS_FreeFile;
    ri.FS_Gamedir = FS_Gamedir;
	ri.Vid_NewWindow = VID_NewWindow;
    ri.Cvar_Get = Cvar_Get;

    re = GetRefAPI(ri);

    if (re.api_version != API_VERSION)
        Com_Error (ERR_FATAL, "Re has incompatible api_version");
    
        // call the init function
    if (re.Init (NULL, NULL) == -1)
		Com_Error (ERR_FATAL, "Couldn't start refresh");
}

void	VID_Shutdown (void)
{
    if (re.Shutdown)
	    re.Shutdown ();
}

void	VID_CheckChanges (void)
{
}

void	VID_MenuInit (void)
{
}

void	VID_MenuDraw (void)
{
}

const char *VID_MenuKey( int k)
{
	return NULL;
}
