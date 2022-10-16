#ifndef DEBUG_H
#define DEBUG_H
/*
**-----------------------------------------------------------------------------
**  File:       Debug.h
**  Purpose:    Sample Debug code
**  Notes:
**
**  Copyright (c) 1995 - 1997 by Microsoft, all rights reserved
**-----------------------------------------------------------------------------
*/

/*
**-----------------------------------------------------------------------------
**  Include files
**-----------------------------------------------------------------------------
*/

#include <assert.h>
//#include "Common.h"

/*
**-----------------------------------------------------------------------------
**  Defines
**-----------------------------------------------------------------------------
*/

#ifdef _DEBUG
// Note:  Define DEBUG_PROMPTME if you want MessageBox Error prompting
//          This can get annoying quickly...
// #define DEBUG_PROMPTME

    // Pre and Post debug string info
    #define START_STR    TEXT ("PROF: ")
    #define END_STR        TEXT ("\r\n")
#endif // _DEBUG

// Debug Levels
#define DEBUG_ALWAYS        0L
#define DEBUG_CRITICAL    1L
#define DEBUG_ERROR        2L
#define DEBUG_MINOR        3L
#define DEBUG_WARN        4L
#define DEBUG_DETAILS        5L


// Sample Errors
#define APPERR_GENERIC          MAKE_DDHRESULT (10001)
#define APPERR_INVALIDPARAMS    MAKE_DDHRESULT (10002)
#define APPERR_NOTINITIALIZED    MAKE_DDHRESULT (10003)
#define APPERR_OUTOFMEMORY        MAKE_DDHRESULT (10004)
#define APPERR_NOTFOUND            MAKE_DDHRESULT (10005)



/*
**-----------------------------------------------------------------------------
**  Macros
**-----------------------------------------------------------------------------
*/

#ifdef _DEBUG
    #define DPF dprintf
#ifndef ASSERT
    #define ASSERT(x) \
        if (! (x)) \
        { \
            DPF (DEBUG_ALWAYS, TEXT("Assertion violated: %s, File = %s, Line = #%ld\n"), \
                 TEXT(#x), TEXT(__FILE__), (DWORD)__LINE__ ); \
            abort (); \
        }        
#endif

   #define REPORTERR(x) \
       ReportDDError ((x), TEXT("File = %s, Line = #%ld\n"), \
                      TEXT(__FILE__), (DWORD)__LINE__ );

   #define FATALERR(x) \
       ReportDDError ((x), TEXT("File = %s, Line = #%ld\n"), \
                      TEXT(__FILE__), (DWORD)__LINE__ ); \
       OnPause (TRUE); \
       DestroyWindow (g_hMainWindow);
#else
   #define REPORTERR(x)
   #define DPF 1 ? (void)0 : (void)

#ifndef ASSERT
   #define ASSERT(x)
#endif
   #define FATALERR(x) \
       OnPause (TRUE); \
       DestroyWindow (g_hMainWindow);
#endif // _DEBUG


/*
**-----------------------------------------------------------------------------
**  Global Variables
**-----------------------------------------------------------------------------
*/

// Debug Variables
#ifdef _DEBUG
    extern DWORD g_dwDebugLevel;
//  extern HWND  g_hMainWindow;
#endif

extern BOOL  g_fDebug;



/*
**-----------------------------------------------------------------------------
**  Function Prototypes
**-----------------------------------------------------------------------------
*/

// Debug Routines
#ifdef _DEBUG
    void __cdecl dprintf (DWORD dwDebugLevel, LPCTSTR szFormat, ...);
#endif //_DEBUG

void _cdecl ReportDDError (HRESULT hResult, LPCTSTR szFormat, ...);

BOOL SetDDErrorReporting (BOOL bUseMsgBox) ;
/*
**-----------------------------------------------------------------------------
**  End of File
**-----------------------------------------------------------------------------
*/
#endif // End DEBUG_H


