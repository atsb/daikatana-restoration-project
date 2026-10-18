#ifndef DK_SO_H
#define DK_SO_H

#include "dk_system.h"

//	Called by the server after DLL is loaded.
//	pvData contains a pointer to QDLLSERVERINFO unique to each DLL.
//	Check dllversion against IONSTORM_DLL_INTERFACE_VERSION and return
//	FALSE if it does not match.
#define QDLL_VERSION        1

//	Used for map editing.  Not implemented yet!  Ked and QRadient will
//	still look for the dll_Query function, so it must be exported from
//	any DLL that the editors need entity information from!
#define QDLL_QUERY          2

//	Return (char *) to a one-line (80 char) description of the DLL in 
//	pvData.  You can return NULL if you want.
#define QDLL_DESCRIPTION    3

//	Called by server after all DLL's have been loaded.  Internal DLL
//	setup and initialization should occur at this time. SERVERINITINFO
//	passed at this time.

//	Nelno says: called for each DLL after all DLLs have loaded and run their 
//	QDLL_LOAD functions.  At this point it should be safe for all DLLs to 
//	call routines from other DLLs
#define QDLL_SERVER_INIT    10
//	called when a DLL is exiting (being unloaded, ie. Daikatana is exiting).
#define QDLL_SERVER_KILL    11
//	called upon initial loading of DLL.  DLLs should only do internal setup
//	in this phase, since it is not guaranteed that other DLLs have been 
//	initialized.
#define	QDLL_SERVER_LOAD	12

//	Called when a new level has started.
#define QDLL_LEVEL_LOAD     20

//	Called when the current level is quit.  Level-based cleanup should 
//	occur at this time.
#define QDLL_LEVEL_EXIT     21

//	Called to retrieve the address of the common_export_t structure
//	DLLs other than COMMON.DLL should return false!!
#define	QDLL_QUERY_COMMON	30

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//typedef BOOL (WINAPI dllEntry_t) (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);
typedef int dllEntry_t (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

#define IONSTORM_DLL_INTERFACE_VERSION (sizeof(userEntity_t) + sizeof(serverState_t))

#endif
