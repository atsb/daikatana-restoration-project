///////////////////////////////////////////////////////////////////////////////
//	3/1/98 - changed to a single init function for each DLL which passes a
//			reason for call, along with a void pointer to data (usually serverState)
//
///////////////////////////////////////////////////////////////////////////////

// dll.c -- Handles dll interaction of code

#include	<stdlib.h>
#include	<dlfcn.h>
#include	<dirent.h>

#include	"p_global.h"

#include	"p_user.h"
#include	"dk_so.h"

//	for debugging release version
#include	"../dk_/dk_log.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define MAX_DLLS    50

#define DF_PERM_DLL		0x00000001
#define	DF_WORLD_DLL	0x00000002
#define	DF_WEAPONS_DLL	0x00000004
#define	DF_GCE_DLL		0x00000008

#define _MAX_FNAME		MAX_OSPATH
#define _MAX_EXT		MAX_OSPATH

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	unsigned int	flags;
	char			path [MAX_OSPATH];
	char			fname [_MAX_FNAME];
	char			ext [_MAX_EXT];
	HINSTANCE		dll;
	
	//	pointer to entry function for this dll
	dllEntry_t		 *dll_Entry;
} dll_t;

typedef	void	(*dll_ClientDisconnect_t)		(userEntity_t *self);
typedef	void	(*dll_ClientBegin_t)			(userEntity_t *self, int loadgame);
typedef	int		(*dll_ClientConnect_t)			(userEntity_t *self, char *userinfo, int loadgame);
typedef	void	(*dll_ClientUserinfoChanged_t)	(edict_t *ent, char *userinfo);
typedef	void	(*dll_ClientThink_t)			(userEntity_t *ent, usercmd_t *ucmd, pmove_t *pm);
typedef	void	(*dll_ClientBeginServerFrame_t)	(userEntity_t *ent);
typedef	void	(*dll_think_stub_t)				(userEntity_t *ent);
typedef	void	(*dll_SetStats_t)				(userEntity_t *ent);
typedef	void	(*dll_UpdateBoundingBoxes_t)	(userEntity_t *ent);
typedef	void	(*dll_NPCAlert_t)				(userEntity_t *owner, userEntity_t *target);
typedef void	(*dll_BeginIntermission_t)		(char *nextMap);


///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

void	(*dll_ClientDisconnect)		(userEntity_t *self);
void	(*dll_ClientBegin)			(userEntity_t *self, int loadgame);
int		(*dll_ClientConnect)		(userEntity_t *self, char *userinfo, int loadgame);
void	(*dll_ClientUserinfoChanged)(edict_t *ent, char *userinfo);
void	(*dll_ClientThink)			(userEntity_t *ent, usercmd_t *ucmd, pmove_t *pm);
void	(*dll_ClientBeginServerFrame)	(userEntity_t *ent);
void	(*dll_think_stub)			(userEntity_t *ent);
void	(*dll_SetStats)				(userEntity_t *ent);
void	(*dll_UpdateBoundingBoxes)	(userEntity_t *ent);
void	(*dll_NPCAlert)				(userEntity_t *owner, userEntity_t *attacker);
void	(*dll_BeginIntermission)	(char *nextMap);

static	dll_t dlls [MAX_DLLS];
int		dll_Loading = 0;

//	for reloading of DLLs (not used yet)
void	dll_Reload(void);

#ifdef REF
typedef struct _WIN32_FIND_DATA { // wfd  
    DWORD dwFileAttributes; 
    FILETIME ftCreationTime; 
    FILETIME ftLastAccessTime; 
    FILETIME ftLastWriteTime; 
    DWORD    nFileSizeHigh; 
    DWORD    nFileSizeLow; 
    DWORD    dwReserved0; 
    DWORD    dwReserved1; 
    TCHAR    cFileName[ MAX_PATH ]; 
    TCHAR    cAlternateFileName[ 14 ]; 
} WIN32_FIND_DATA; 
#endif

///////////////////////////////////////////////////////////////////////////////
//	DLL_IsName
//
//	returns true if the filename.ext from the path == name
///////////////////////////////////////////////////////////////////////////////

int	DLL_IsName (char *path, char *name)
{
	char	fname [_MAX_FNAME];
	char	ext [_MAX_EXT];
	char	filename [_MAX_FNAME + _MAX_EXT + 1];

	if (!path || !name)
	{
		if (path == name)
			return	true;
		else
			return	false;
	}

	_splitpath (path, NULL, NULL, fname, ext);

	strcpy (filename, fname);
	strcat (filename, ext);

	if (!stricmp (filename, name))
		return	true;
	else
		return	false;
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_Loaded
//
//	returns true if an existing DLL has the same name.ext as the one passed in
//	path
///////////////////////////////////////////////////////////////////////////////

static int DLL_Loaded (char *filename, char *extension)
{
	int		i;

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (!dlls [i].dll) continue;

		if (!stricmp (filename, dlls [i].fname) && !stricmp (extension, dlls [i].ext)) 
			return	1;
	}

	return(0);
}

///////////////////////////////////////////////////////////////////////////////
//	
///////////////////////////////////////////////////////////////////////////////

static void DLL_LoadPath (char *path, unsigned int flags)
{
	int		i = 0, size;
	char	*desc;
	char	fname [_MAX_FNAME];
	char	ext [_MAX_EXT];

	//	don't load physics or physicsa.dll, would just get unloaded anyway but 
	//	may as well not bother
	if (!stricmp (path + strlen (path) - strlen (gi.dk_gamename), gi.dk_gamename))
		return;

	_splitpath (path, NULL, NULL, fname, ext);
	
	// make sure we haven't already loaded this dll
	if (DLL_Loaded (fname, ext))
	{
		gi.Con_Dprintf("Skipping dll %s\n", path);
		return;
	}

	// ok we found a dll, find a place to load it
	for (i = 0; i < MAX_DLLS; i++)
	{
		if (!dlls [i].dll) break;
	}

	// see if we overflowed
	if (i >= MAX_DLLS)
	{
		Sys_Error ("MAX_DLLS = %i.  No more DLLs can load.\n", MAX_DLLS);
		return;
	}

    dlls [i].dll = dlopen (path, RTLD_NOW); 

    gi.Con_Dprintf("Loading %s\n", path);
	if (dlls [i].dll == NULL)
	{
		gi.Con_Dprintf ("dlerror: %s\n", dlerror());
		Sys_Error ("Unable to load dll: %s", path);
		return;
	}

	memcpy (dlls [i].path, path, MAX_OSPATH);
	dlls[i].flags = flags;
	
	//	loaded this DLL, so copy name and fname over for later compares
	strcpy (dlls [i].fname, fname);
	strcpy (dlls [i].ext, ext);

	//	find entry function and store a pointer to it
	dlls [i].dll_Entry = (dllEntry_t *) dlsym (dlls [i].dll, "dll_Entry");
	if (dlls [i].dll_Entry == NULL)
		gi.Con_Dprintf ("dlerror: %s\n", dlerror());

	if (dlls [i].dll_Entry)
	{
		//	pass the size of serverState_t and userEntity_t
		size = IONSTORM_DLL_INTERFACE_VERSION;

		//	returns 0 if size does not match
		if (!dlls [i].dll_Entry (dlls [i].dll, QDLL_VERSION, &size))
		{
			gi.Con_Dprintf("Skipping dll %s, size mismatch\n", path);
			//Sys_Log("Skipping dll %s, size mismatch\n", path);

			dlclose (dlls[i].dll);
			
			dlls [i].dll = NULL;
			dlls [i].dll_Entry = NULL;

			return;
		}
	}
	else
	{
		gi.Con_Dprintf("Skipping dll %s, no dll_Entry function.\n", path);

		dlclose(dlls [i].dll);
		
		dlls [i].dll = NULL;
		dlls [i].dll_Entry = NULL;

		return;
	}

	///////////////////////////////////////////////////////////////////////////
	//	run one-time initialization routines for this DLL
	///////////////////////////////////////////////////////////////////////////

	dlls [i].dll_Entry (dlls [i].dll, QDLL_SERVER_LOAD, &serverState);
	gi.Con_Dprintf("Physics: loaded %s.\n", path);

	///////////////////////////////////////////////////////////////////////////
	//	get description for debugging...
	///////////////////////////////////////////////////////////////////////////

	desc = NULL;
	dlls [i].dll_Entry (dlls [i].dll, QDLL_DESCRIPTION, &desc);
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_CommandLine
//
//	Load any DLLs that are specified on the command line
///////////////////////////////////////////////////////////////////////////////

static void DLL_CommandLine (void)
{
    int    i;
	bool	has_overrides = false, overrode_world = false;

	//	make sure world.dll gets loaded first if it is overridden
    for (i = 1; i < gi.EXEGetArgc() - 1; i++)
    {
		if (!gi.EXEGetArgv(i) || !gi.EXEGetArgv(i + 1))
            continue;      // NEXTSTEP sometimes clears appkit vars.

        if (!stricmp ("-dll", gi.EXEGetArgv(i)))
		{
			if (DLL_IsName (gi.EXEGetArgv(i + 1), "world.so"))
			{
				DLL_LoadPath (gi.EXEGetArgv(i + 1), DF_PERM_DLL);
				overrode_world = true;
			}
		}
    }

	//	load the rest of the dlls
    for (i = 1; i < gi.EXEGetArgc() - 1; i++)
    {
		if (!gi.EXEGetArgv(i) || !gi.EXEGetArgv(i + 1))
            continue;      // NEXTSTEP sometimes clears appkit vars.

        if (!stricmp ("-dll", gi.EXEGetArgv(i)))
		{
			if (gi.EXEGetArgv(i + 1))
			{
				DLL_LoadPath (gi.EXEGetArgv(i + 1), DF_PERM_DLL);
				has_overrides = true;
			}
		}
    }

	if (overrode_world != true && has_overrides == true)
	{
		Sys_Error ("World.dll MUST have a -dll override if any -dll overrides are used for other dlls.\n");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_LoadDir
//
//	Load all the DLLs in netpath, as long as another DLL with the same
//	name was not loaded from the command line
///////////////////////////////////////////////////////////////////////////////

static void DLL_LoadDir (char *netpath, unsigned int flags)
{
	DIR *dirPtr;
	struct dirent *direntPtr;
	char path[PATH_MAX];

	dirPtr = opendir (netpath);
	while ((direntPtr = readdir (dirPtr)) != NULL)
	{
		if (strstr (direntPtr->d_name, ".so") != NULL)
		{
			sprintf (path, "%s/%s", netpath, direntPtr->d_name);
			DLL_LoadPath (path, flags);
		}
	}
	closedir (dirPtr);
}

#if 0
static void DLL_LoadDir (char *netpath, unsigned int flags)
{
	HANDLE           dirHandle;
	WIN32_FIND_DATA  findData;
	int              found = 1;
	char             fullpath [MAX_OSPATH];
	char             path [MAX_OSPATH];

	sprintf(fullpath, "%s/*.so", netpath);

	// get first dll file
	dirHandle = FindFirstFile(fullpath, &findData);

	if (dirHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (found)
	{
		//	don't load physics.dll or physicsa.dll
		if (stricmp (findData.cFileName, "physics.so") && stricmp (findData.cFileName, "physicsa.so"))
		{
			// load dll
			sprintf(path, "%s\\%s", netpath, findData.cFileName);
			// DLL_LoadPath(netpath, findData.cFileName, flags);
			DLL_LoadPath(path, flags);
		}

		found = FindNextFile(dirHandle, &findData);
	}

	FindClose(dirHandle);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//	DLL_SearchDirs
//
//	FIXME: doesn't work for PAK files, but DLLs don't really need to be in the
//	pak file, do they???
///////////////////////////////////////////////////////////////////////////////

static void DLL_SearchDirs(char *dirName, unsigned int flags)
{
//	searchpath_t    *search;
//	char             netpath [MAX_OSPATH];
/*
    for (search = com_searchpaths ; search ; search = search->next)
	{
	    // if a pak file process later
		if (search->pack)
		{
			continue;
		}
		
		else
		{               
			sprintf (netpath, "%s\\%s", search->filename, dirName);

			// look for dlls in this directory
			DLL_LoadDir(netpath, flags);
		}
	}
*/
	DLL_LoadDir(dirName, flags);
/*
	strcpy(netpath, "\\katana\\q2_kee\\data\\ndlls");
	DLL_LoadDir(netpath, flags);
*/
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_LoadWorld
//
//	loads world.dll.  This should be done first, before weapons.dll or any
//	dll that uses world.lib.  If world wishes to use any functions from the
//	other DLLs it must use LoadLibrary and GetProcAddress to do so.
///////////////////////////////////////////////////////////////////////////////

void	DLL_LoadWorld (char *dirName, unsigned int flags)
{
	char	path [1024];

	strcpy (path, dirName);
	strcat (path, "/world.so");
	DLL_LoadPath (path, flags | DF_WORLD_DLL);
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_FindFunction
//
//	Search through all loaded DLLs for function funcName
///////////////////////////////////////////////////////////////////////////////

void *DLL_FindFunction (char *funcName)
{
	int   i;
	void *func;

	// use time sorting in the future
	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls[i].dll)
		{
			func = dlsym(dlls[i].dll, funcName);

			if (func != NULL) return(func);
		}
	}

	return(NULL);
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_SetReqFunctions
//
//	find and set the required WORLD.DLL functions
///////////////////////////////////////////////////////////////////////////////

void DLL_SetReqFunctions (void)
{
	// find pointer to dll_ClientDisconnect in all dlls loaded
	dll_ClientDisconnect = (dll_ClientDisconnect_t) DLL_FindFunction("dll_ClientDisconnect");
	if (dll_ClientDisconnect == NULL)
		Sys_Error("Unable to find dll_ClientDisconnect in dlls.");

	dll_ClientConnect = (dll_ClientConnect_t) DLL_FindFunction("dll_ClientConnect");
	if (dll_ClientConnect == NULL)
		Sys_Error("Unable to find dll_ClientConnect in dlls.");

	dll_ClientBegin = (dll_ClientBegin_t) DLL_FindFunction("dll_ClientBegin");
	if (dll_ClientBegin == NULL)
		Sys_Error("Unable to find dll_ClientBegin in dlls.");

	dll_ClientUserinfoChanged = (dll_ClientUserinfoChanged_t) DLL_FindFunction("dll_ClientUserinfoChanged");
	if (dll_ClientUserinfoChanged == NULL)
		Sys_Error("Unable to find dll_ClientUserinfoChanged in dlls.");

	dll_ClientThink = (dll_ClientThink_t) DLL_FindFunction("dll_ClientThink");
	if (dll_ClientThink == NULL)
		Sys_Error("Unable to find dll_ClientThink in dlls.");

	dll_ClientBeginServerFrame = (dll_ClientBeginServerFrame_t) DLL_FindFunction("dll_ClientBeginServerFrame");
	if (dll_ClientBeginServerFrame == NULL)
		Sys_Error("Unable to find dll_ClientBeginServerFrame in dlls.");

	dll_think_stub = (dll_think_stub_t) DLL_FindFunction("ai_think_stub");

	dll_SetStats = (dll_SetStats_t) DLL_FindFunction("dll_SetStats");
	if (dll_SetStats == NULL)
		Sys_Error("Unable to find dll_SetStats in dlls.");

	dll_NPCAlert = (dll_NPCAlert_t) DLL_FindFunction("NPC_Alert");
	if (dll_NPCAlert == NULL)
		Sys_Error("Unable to find NPC_Alert in dlls.");

	dll_UpdateBoundingBoxes = (dll_UpdateBoundingBoxes_t) DLL_FindFunction("ShowBoundingBoxes");

	dll_BeginIntermission = (dll_BeginIntermission_t) DLL_FindFunction("dll_BeginIntermission");
	if (dll_BeginIntermission == NULL)
		Sys_Error("Unable to find dll_BeginIntermission in dlls.");

}


///////////////////////////////////////////////////////////////////////////////
//	
///////////////////////////////////////////////////////////////////////////////

void DLL_InitFunctions(void)
{
	int i;
//	int (*dll_Init)(serverState_t *);

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls [i].dll)
		{
		
			// if the return from this dll is 0 then unload dll
		    if (!dlls [i].dll_Entry (dlls [i].dll, QDLL_SERVER_INIT, &serverState))
			{
				gi.Con_Dprintf("Unloaded %s\n", dlls[i].path);

				dlclose(dlls[i].dll);
			    dlls[i].dll = NULL;
			}
	    }
	}
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_LoadLevelFunctions
//
//	call dll_Entry with QDLL_LEVEL_LOAD for each DLL
///////////////////////////////////////////////////////////////////////////////

void DLL_LoadLevelFunctions(void)
{
	int i;
//	void (*dll_LoadLevel)(serverState_t *);

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls[i].dll)
		{
			dlls [i].dll_Entry (dlls [i].dll, QDLL_LEVEL_LOAD, &serverState);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_ExitLevelFunctions
//
//	call dll_Entry with QDLL_LEVEL_EXIT for each DLL
///////////////////////////////////////////////////////////////////////////////

void DLL_ExitLevelFunctions(void)
{
	int i;

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls [i].dll)
		{
			dlls [i].dll_Entry (dlls [i].dll, QDLL_LEVEL_EXIT, &serverState);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_ExitFunctions
//
//	call dll_Entry with QDLL_SERVER_KILL for each DLL
///////////////////////////////////////////////////////////////////////////////

void DLL_ExitFunctions(void)
{
	int				i;

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls [i].dll)
		{
			dlls [i].dll_Entry (dlls [i].dll, QDLL_SERVER_KILL, &serverState);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_LoadDLLs
//
//	Load all dlls, first from command line, then from directory dirName
//	Set the required functions (usually found in WORLD.DLL)
///////////////////////////////////////////////////////////////////////////////

void DLL_LoadDLLs (char *dirName)
{
	dll_Loading = 1;

	//	clear the dll array
	memset (dlls, 0x00, sizeof (dlls));

	// search commmand line and load any dlls from there first
	DLL_CommandLine();

	//	world.dll must be loaded first or windows will try to auto-load it when 
	//	weapons or anything linked with world.lib is loaded.  This will not work
	//	because world.dll is not in the exe path or the path.
	DLL_LoadWorld (dirName, DF_PERM_DLL);

	// load all dlls from level and mark them as permenant dlls
	DLL_SearchDirs(dirName, DF_PERM_DLL);

	// there are some dll functions that we have to have to run
	// we might have defaults for these

	DLL_SetReqFunctions();

	dll_Loading = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	DLL_UnloadDLLs
//
//	Run QDLL_SERVER_KILL functions for each DLL
//	unload each DLL
///////////////////////////////////////////////////////////////////////////////

void DLL_UnloadDLLs (void)
{
	int i;

	//	allow dlls chance to clean up
	DLL_ExitFunctions();

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls[i].dll)
		{
			dlclose(dlls[i].dll);
			dlls [i].dll = NULL;
			dlls [i].dll_Entry = NULL;
		}
	}

	//	get rid of any commands associated with DLLs
	gi.RemoveDLLCommands ();
}

////////////////////////////////////////////////////////////////////////////////////
/// dll reloading
////////////////////////////////////////////////////////////////////////////////////


#ifdef NEW

void dll_Reload(void)
{
	char *target = "steed";

	for (i = 0; i < MAX_DLLS; i++)
	{
		if (dlls[i].dll)
		{
			if (strstr(dlls[i].path, target))
			{
				// see if dll has a reload function
				dll_Reload = (void *) dlsym(dlls[i].dll, "dll_Reload");

				if (dll_Reload)
				{
					dll_Reload();

			dlclose (dlls[i].dll);

					// ok reload dll
					dlls[i].dll = dlopen (path, RTLD_NOW); 

	if (dlls[i].dll == NULL)
	{
		Sys_Error("Unable to load dll: %s.", path);
	}
				}
			}
		}
	}
}

#endif
