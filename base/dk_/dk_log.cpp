//#include <windows.h>
#include "dk_system.h"
#if _MSC_VER
#include <direct.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
//	quick and dirty log file routines so Nelno can see what the hell is going
//	on in 16 bpp modes that aren't working
//
//	opens a log file named [username].log
///////////////////////////////////////////////////////////////////////////////

static	FILE	*logfile;

///////////////////////////////////////////////////////////////////////////////
//	opens the damn log file
///////////////////////////////////////////////////////////////////////////////

void	log_open (void)
{
	char			userName [256];
	unsigned long	size = 256;

// mdm 98.02.08 - gets the username - this should probably be one function call
//	defined differently depending on the OS
#if _MSC_VER
	GetUserName (userName, &size);
#else
	strcpy (userName, getlogin());
#endif
	strcat (userName, ".log");

	if (!(logfile = fopen (userName, "a+")))
		return;
}

///////////////////////////////////////////////////////////////////////////////
//	log_close
///////////////////////////////////////////////////////////////////////////////

void	log_close (void)
{
	fclose (logfile);
}

///////////////////////////////////////////////////////////////////////////////
//	log_printf
///////////////////////////////////////////////////////////////////////////////

void	log_printf (char *fmt, ...)
{
	char		line [256];
	va_list		argptr;

	va_start (argptr, fmt);
	vsprintf (line, fmt, argptr);
	va_end (argptr);

//	strcat (line, "\n");

	log_open ();
	fwrite (line, strlen (line), 1, logfile);
	log_close ();
}