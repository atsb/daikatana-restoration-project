// Launches mplayer.com into a specific URL with LaunchMplayer()
// By Rich Rice --rich@mplayer.com

// updated for use with Daikatana - 1-25-00

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////////
// Instructions:
//
// Modify the defines, MPLAYER_URL and MPI_FILE to whatever Mplayer says it 
// should be and then call LaunchMplayer() (found at the end of this file) and 
// have your application exit.
#ifdef JPN
#define MPLAYER_URL "http://www.37play.com/retailgames/daikatana.phtml"
#else
#define MPLAYER_URL "http://www.mplayer.com/gamers/action/daikatana/"
#endif

#define MPI_FILE    "mpkatana.mpi"

#ifdef _MSC_VER
   #define stat _stat
#endif

// Execute executes whatever the text in cmdline is.
// If an executable is successfully launched as a 
// result, TRUE is returned. False is returned if 
// nothing happens.
static int
Execute(char *cmdline)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;

	memset (&StartupInfo, 0, sizeof StartupInfo);
	StartupInfo.cb          = sizeof (StartupInfo);
	StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOWNORMAL;

	if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE,
			CREATE_DEFAULT_ERROR_MODE, NULL, NULL,
			&StartupInfo, &ProcessInfo) == 0)
		return FALSE;

	return TRUE;
}

// Mplayer stores it installed location in the Windows Registry
// under HKEY_LOCAL_MACHINE\Software\Mplayer\Main\Root Directory
// This function returns the location stored in that string value.
// Upon failure, FALSE is returned and, upon success, TRUE is
// returned.
// The location of the Mplayer's install path is returned in
// the parameter mplayer_directory.

static int 
GetMplayerDirectory(char *mplayer_directory)
{
	HKEY hkey;
	HKEY key = HKEY_LOCAL_MACHINE;
	char subkey[]="software\\mpath\\mplayer\\main";
	char valuename[]="root directory";
	char buffer[MAX_PATH];
	DWORD dwType, dwSize;

	
	if (RegOpenKeyEx(key, subkey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		dwSize = MAX_PATH;
		if (RegQueryValueEx(hkey, valuename, 0, &dwType, (LPBYTE) buffer,
			&dwSize) == ERROR_SUCCESS)
		{
			sprintf(mplayer_directory, "%s", buffer);
			return TRUE;
		}
		RegCloseKey(hkey);
	}

	return FALSE;
}

// If the named file exists, this function
// returns TRUE, otherwise FALSE is returned.
static int 
FileExists(char *file)
{
	struct stat buf;
	int result;

	// Get data associated with 'file'
	result = stat( file, &buf );

	// Check if we have a file
	if( result == 0 )
	{
	   return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// This is a public function, it launches Mplayer's user interface.
// This function returns FALSE upon failure and TRUE on success.
int
LaunchMplayer()
{
	char cmdline[MAX_PATH], mplaunch_exe[MAX_PATH], mplayer_directory[MAX_PATH],
			mpinstaller[MAX_PATH];

	if (GetMplayerDirectory(mplayer_directory))
	{
		sprintf(mplaunch_exe, "%s\\programs\\mplaunch.exe", mplayer_directory);
		if (FileExists(mplaunch_exe))
		{
			sprintf(cmdline, "%s %s", mplaunch_exe, MPI_FILE);
			return Execute(cmdline);
		}
	}

	// let's run the mplayer installer since we know it is installed locally
	// in 'install\mplayer\mplayerinstaller.exe'
#ifdef JPN
	sprintf(mpinstaller, "install\\37player\\37playerinstaller.exe");
#else
	sprintf(mpinstaller, "install\\mplayer\\mplayerinstaller.exe");
#endif

	if(FileExists(mpinstaller))
		Execute(mpinstaller);

	sprintf(cmdline, "rundll32.exe url.dll,FileProtocolHandler %s", MPLAYER_URL);
	return Execute(cmdline);
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// This is a public function, it launches the default browser to the
// specified url.
// This function returns FALSE upon failure and TRUE on success.
int
LaunchBrowser(char *url)
{
	char cmdline[MAX_PATH];
	if(url)
		sprintf(cmdline, "rundll32.exe url.dll,FileProtocolHandler %s", url);
	else
		sprintf(cmdline, "rundll32.exe url.dll,FileProtocolHandler %s", MPLAYER_URL);

	return Execute(cmdline);
}
