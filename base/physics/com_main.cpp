#include	<stdio.h>

#include	"p_user.h"
#include	"hooks.h"
#include	"common.h"
#include	"p_global.h"

//	DLL interface defines
//unix - dk_dll.h / dk_so.h
#if _MSC_VER
#include	"dk_dll.h"	
#else
#include	"dk_so.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

//	Nelno:	old stuff from when common was its own DLL
//	DllExport	int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//	Nelno:	old stuff from when common was its own DLL
//	char			*dll_Description = "COMMON.DLL.  Standard functions for Daikatana DLLs.\n";

///////////////////////////////////////////////////////////////////////////////
//	com_InitMemMgr
///////////////////////////////////////////////////////////////////////////////

void com_InitMemMgr(void)
{
	// setup memory manager functions
	com.X_Malloc = gi.X_Malloc;
	com.X_Free = gi.X_Free;
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitExports
///////////////////////////////////////////////////////////////////////////////

void	com_InitExports (void)
{
	com_InitFriction ();
	com_InitMemMgr();
	com_InitList ();
	com_InitSub ();
	com_InitWeapons ();

	gi.Con_Dprintf("com_InitExports: initialized export struct.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	com_ServerLoad
// 
//	initialization functions called when physics.dll is loaded
///////////////////////////////////////////////////////////////////////////////

void	com_ServerLoad (void)
{
	gi.Con_Dprintf("* Initializing common routines:\n");

	com_InitExports ();
	gi.Con_Dprintf("... initialized common exports.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	com_LevelLoad
// 
//	initialization called when a new level is loaded
///////////////////////////////////////////////////////////////////////////////

void	com_LevelLoad (void)
{
	gi.Con_Dprintf("com_LoadLevel:  \n");
//	gi.Mem_Heap_Walk(1);
}

///////////////////////////////////////////////////////////////////////////////
//	com_LevelExit
//
//	shutdown functions when a level is exited
///////////////////////////////////////////////////////////////////////////////

void 	com_LevelExit (void)
{
	gi.Con_Dprintf("com_LevelExit:  \n");
//	gi.Mem_Heap_Walk(1);

	// free any non-permanent memory in the malloc_list
//	gi.Mem_Free_Tag(MEM_TAG_HOOK);
//	gi.Mem_Free_Tag(MEM_TAG_COM_EPAIR);
//	gi.Mem_Free_Tag(MEM_TAG_COM_ENTITY);
//	gi.Mem_Free_Tag(MEM_TAG_COM_INV);
//	gi.Mem_Free_Tag(MEM_TAG_COM_MISC);
}

///////////////////////////////////////////////////////////////////////////////
//	com_ServerKill
//
//	Engine is about to unload this DLL so do cleanup tasks
///////////////////////////////////////////////////////////////////////////////

void 	com_ServerKill (void)
{
	gi.Con_Dprintf("com_Exit:  \n");
//	gi.Mem_Heap_Walk(1);

	// free all malloc'd memory
	gi.Mem_Free_Tag(MEM_TAG_HOOK);
	gi.Mem_Free_Tag(MEM_TAG_COM_EPAIR);
	gi.Mem_Free_Tag(MEM_TAG_COM_ENTITY);
	gi.Mem_Free_Tag(MEM_TAG_COM_INV);
	gi.Mem_Free_Tag(MEM_TAG_COM_MISC);
}
