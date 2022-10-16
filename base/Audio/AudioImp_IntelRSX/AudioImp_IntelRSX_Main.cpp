 #include <windows.h>
#include "..\AudioDebug.h"
#include "AudioImp_IntelRSX.h"
#include "..\AudioPlugin.h"

#ifndef	DLLEXPORT
#define DLLEXPORT	extern "C"	__declspec( dllexport )
#endif

DLLEXPORT void  DestroyRSXImp (void* pImp) ;
DLLEXPORT void* CreateRSXImp  () ;


// ------------------------------ Globals ------------------------------------


HINSTANCE g_hinstThisDll;

static BOOL          g_bCrippledOS = FALSE;
static OSVERSIONINFO g_osviVerInfo;


const AudioPluginDesc_t descRSXImplementation = 
{
    0,
    "Intel RSX Imp",
    "Daikatana RSX Implementation",
    "ION Storm",
    kAudioImplementation,
    0,
    kAP_CPU_Medium,
    kAP_UseHWIfAvailable,
    0,
    0, // no properties
    CreateRSXImp,
    DestroyRSXImp
} ;



// ------------------------------ Plugin Interface -----------------------------

DLLEXPORT int GetNumPlugins ()
{
    return (1);
}


DLLEXPORT const AudioPluginDesc_t* GetPluginDesc (int nPluginIndex)
{
    switch (nPluginIndex)
    {
        case 0:
            return &descRSXImplementation ;
    }

    return (0);
}


// ------------------------------ Instance Management -----------------------------

DLLEXPORT void* CreateRSXImp ()
{
    IAudioSpecificImplementation* pImp ;

    pImp = new IAudioImp_IntelRSX ;

    return (pImp);
}    

DLLEXPORT void DestroyRSXImp (void* pVoidImp)
{
    IAudioSpecificImplementation* pImp = (IAudioSpecificImplementation*)pVoidImp ;
    delete pImp ;
}    



int APIENTRY DllMain (HINSTANCE hInstance, DWORD  dwReason, LPVOID  lpReserved)
{
    BOOL bRC = TRUE;

    if (DLL_PROCESS_ATTACH == dwReason)
    {
        g_hinstThisDll = hInstance;

        g_osviVerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx(&g_osviVerInfo);

        if (VER_PLATFORM_WIN32_WINDOWS == g_osviVerInfo.dwPlatformId)
            g_bCrippledOS = TRUE;

    }

    return (TRUE);
}





// ==========================================================================
//                              End of File
// ==========================================================================
