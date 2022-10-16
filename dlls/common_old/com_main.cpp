//#include	<windows.h>
#include	<stdio.h>

#include	"p_user.h"
#include	"hooks.h"

#include	"common.h"
#include	"com_globals.h"

//	DLL interface defines
#include	"dk_dll.h"	

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

char			*dll_Description = "COMMON.DLL.  Standard functions for Daikatana DLLs.\n";

///////////////////////////////////////////////////////////////////////////////
//	com_InitMain
///////////////////////////////////////////////////////////////////////////////

void	com_InitMain (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitExports
///////////////////////////////////////////////////////////////////////////////

void	com_InitExports (void)
{
	com_InitFriction ();
	com_InitHeap ();
	com_InitList ();
	com_InitMain ();
	com_InitSub ();

	gstate->Con_Printf ("com_InitExports: initialized export struct.\n");
}

///////////////////////////////////////////////////////////////////////////////
// dll_ServerLoad
// 
// initialization functions called when the DLL is loaded
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerLoad (serverState_t *state)
{
	gstate = state;
	gstate->Con_Printf ("* Loaded COMMON.DLL, %i\n", sizeof (playerHook_t));

	com_InitExports ();

	//	initialize memory list
	malloc_list = com.list_init (malloc_list);
}

///////////////////////////////////////////////////////////////////////////////
// dll_LevelLoad
// 
///////////////////////////////////////////////////////////////////////////////

void	dll_LevelLoad (void)
{
	gstate->Con_Dprintf ("COMMON.DLL, dll_LoadLevel: malloc_count = %i\n", malloc_count);
	gstate->Con_Dprintf ("  %i MEM_HUNK\n  %i MEM_MALLOC\n  %i MEM_PERMANENT\n", 
		com.count_mem_type (MEM_HUNK), com.count_mem_type (MEM_MALLOC), com.count_mem_type (MEM_PERMANENT)); 
}

///////////////////////////////////////////////////////////////////////////////
// dll_Version
//
// returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version	(int size)
{
	if (size == IONSTORM_DLL_INTERFACE_VERSION)
		return	TRUE;
	else
		return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// dll_LevelExit
//
///////////////////////////////////////////////////////////////////////////////

void 	dll_LevelExit (void)
{
	gstate->Con_Dprintf ("COMMON.DLL, dll_ExitLevel: malloc_count = %i\n", malloc_count);
	gstate->Con_Dprintf ("  %i MEM_HUNK\n  %i MEM_MALLOC\n  %i MEM_PERMANENT\n", 
		com.count_mem_type (MEM_HUNK), com.count_mem_type (MEM_MALLOC), com.count_mem_type (MEM_PERMANENT)); 

	// free any non-permanent memory in the malloc_list
	com.free_most ();
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerInit
//
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerInit (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerKill
//
//	Quake is about to unload this DLL so do cleanup tasks
///////////////////////////////////////////////////////////////////////////////

void 	dll_ServerKill (void)
{
	gstate->Con_Dprintf ("common, dll_Exit: malloc_count = %i\n", malloc_count);
	gstate->Con_Dprintf ("  %i MEM_HUNK\n  %i MEM_MALLOC\n  %i MEM_PERMANENT\n", 
		com.count_mem_type (MEM_HUNK), com.count_mem_type (MEM_MALLOC), com.count_mem_type (MEM_PERMANENT)); 

	// free all malloc'd memory
	com.free_all ();

	// Daikatana is exiting, so kill the malloc_list completely
	com.list_kill (malloc_list);
}

///////////////////////////////////////////////////////////////////////////////
//	Allows us to command/query this DLL from other Daikatana DLLs that are
//	linked to it
//
//	FIXME: remove hParent parameter
///////////////////////////////////////////////////////////////////////////////

int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
	int			size;

	switch (dwReasonForCall)
	{
		case	QDLL_VERSION:
			size = *(int *) pvData;
			return	dll_Version (size);
			break;
		
		case	QDLL_QUERY:
			* (userEpair_t **) pvData = NULL;

			return	TRUE;
			break;

		case	QDLL_DESCRIPTION:
			* (char **) pvData = dll_Description;

			return	TRUE;
			break;

		case	QDLL_SERVER_INIT:
			dll_ServerInit ();

			return	TRUE;
			break;

		case	QDLL_SERVER_KILL:
			dll_ServerKill ();

			return	TRUE;
			break;

		case	QDLL_SERVER_LOAD:
			dll_ServerLoad ((serverState_t *) pvData);

			return	TRUE;
			break;

		case	QDLL_LEVEL_LOAD:
			dll_LevelLoad ();

			return	TRUE;
			break;

		case	QDLL_LEVEL_EXIT:
			dll_LevelExit ();
			
			return	TRUE;
			break;

		case	QDLL_QUERY_COMMON:	//	query for common structure
			* (common_export_t **) pvData = &com;

			return	TRUE;
			break;
			
	}

	return	FALSE;	//	DLL doesn't know this command
}

///////////////////////////////////////////////////////////////////////////////
//	DllMain
//
//	standard DLL startup/exit routine.  
///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	switch	(ul_reason_for_call)
	{
		case	DLL_PROCESS_ATTACH:
			break;

	}
	return TRUE;
}
