// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

#include <stdio.h>
#include "dk_system.h"
#include "dk_shared.h"

/* ***************************** define types ****************************** */

typedef struct _logFile
{
	int nLogType;
	char *szFileName;
} LOGFILE;

/* ***************************** Local Variables *************************** */

static int	bFileLogsReady		= FALSE;

static HANDLE	SharedFile	= NULL;
static LPSTR	String		= NULL;

static char	logPath[256];
static FILE	*aLogFiles[LOGTYPE_MAX];

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */

cvar_t *g_cvarLogOutput = NULL;

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */


LOGFILE logFileDefines[] = 
{
	{ LOGTYPE_GRAPHICS, "log_graphics.log" },
	{ LOGTYPE_AI,		"log_ai.log" },
	{ LOGTYPE_WEAPONS,	"log_weapons.log" },
	{ LOGTYPE_AUDIO,	"log_audio.log" },
	{ LOGTYPE_PHYSICS,	"log_physics.log" },
	{ LOGTYPE_SCRIPT,	"log_script.log" },
	{ LOGTYPE_NETWORK,	"log_network.log" },
	{ LOGTYPE_USERIO,	"log_userio.log" },
	{ LOGTYPE_MEMORY,	"log_memory.log" },
    { LOGTYPE_MISC,     "log_misc.log" }
};


//*****************************************************************************
//							LOCAL FUNCTIONS
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_FileInitialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_FileInitialize( char *logFilePath )
{
	if ( bFileLogsReady == TRUE )
	{
		return;
	}

	int bError = FALSE;
	// open files for logging to files
	char logFileName[256];
	for ( int i = 0; i < LOGTYPE_MAX; i++ )
	{
		strcpy( logFileName, logFilePath );
		strcat( logFileName, "/" );
		strcat( logFileName, logFileDefines[i].szFileName );
		aLogFiles[i] = fopen( logFileName, "a" );
		if ( aLogFiles[i] == NULL )
		{
			bError = TRUE;
			break;
		}
	}

	if ( bError == TRUE )
	{
		for ( int j = 0; j < i; j++ )
		{
			if ( aLogFiles[j] )
			{
				fclose( aLogFiles[j] );
				aLogFiles[j] = NULL;
			}
		}
		return;
	}

	bFileLogsReady = TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_FileClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_FileClose()
{
	for ( int i = 0; i < LOGTYPE_MAX; i++ )
	{
		if ( aLogFiles[i] )
		{
			fclose( aLogFiles[i] );
			aLogFiles[i] = NULL;
		}
	}
	bFileLogsReady = FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_DebugWindowInitialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int DKLOG_DebugWindowInitialize()
{
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_DebugWindowClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_DebugWindowClose()
{
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_WriteToDebugWindow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_WriteToDebugWindow( char *newString )
{
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_WriteToFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_WriteToFile( int nLogType, char *msg )
{
	_ASSERTE( nLogType >= LOGTYPE_GRAPHICS && nLogType < LOGTYPE_MAX );

	int nStrLen = strlen(msg);
	fwrite( msg, nStrLen, 1, aLogFiles[nLogType] );

	if ( !(msg[nStrLen - 1] == '\n') )
	{
		fprintf( aLogFiles[nLogType], "\n" );
	}
}

//*****************************************************************************
//							GLOBAL FUNCTIONS
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Initialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int DKLOG_Initialize( char *logFilePath )
{
	DKLOG_FileInitialize( logFilePath );
	return DKLOG_DebugWindowInitialize();
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Close
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DKLOG_Close()
{
	DKLOG_FileClose();
	DKLOG_DebugWindowClose();
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Write
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DKLOG_Write( int nLogType, float fCurrentTime, char *fmt, ... )
{
	if ( !g_cvarLogOutput )
	{
		return;
	}

	char line[256] = { "\0" };
	va_list	argptr;

	int nLogOutput = (int)g_cvarLogOutput->value;

	if ( bFileLogsReady && (nLogOutput & LOG_TO_FILE) )
	{
		// pack log type and time at the beginning of data
		sprintf( line, "%4.1f\t", fCurrentTime );

		va_start(argptr, fmt);
		vsprintf(line + strlen(line), fmt, argptr);
		va_end(argptr);

		DKLOG_WriteToFile( nLogType, line );
	}
}
