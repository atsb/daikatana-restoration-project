//////////////////////////////////////////////////////////////////////////////
// dk_system.h
//////////////////////////////////////////////////////////////////////////////

#ifndef _DK_SYSTEM_H
#define _DK_SYSTEM_H

//////////////////////////////////////////////////////////////////////////////
// windows
//////////////////////////////////////////////////////////////////////////////

#if _MSC_VER

#include <windows.h>
#include <crtdbg.h>

#endif // _MSC_VER

//////////////////////////////////////////////////////////////////////////////
// unix
//////////////////////////////////////////////////////////////////////////////

#if __GNUG__

#include <unistd.h>

typedef unsigned char		BYTE;
typedef int			BOOL;
typedef unsigned int		UINT;
typedef unsigned long long	ULONG;
typedef unsigned int		DWORD;
typedef char*			LPSTR;
typedef char*			LPCTSTR;
typedef void*			PVOID;
typedef void*			LPVOID;
typedef void*			HANDLE;
typedef void*			HINSTANCE;
typedef void*           	HMODULE;

#ifndef strcmpi
    #define strcmpi strcasecmp
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define PASCAL
#define WINAPI
#define _ASSERTE(exp)

extern "C" DWORD timeGetTime (void);

#endif // __GNUG__

#endif // _DK_SYSTEM_H
