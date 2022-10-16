#include <windows.h>
#include "..\AudioDebug.h"
#include "AudioImp_Aureal3d.h"
#include "..\AudioPlugin.h"

#ifndef	DLLEXPORT
#define DLLEXPORT	extern "C"	__declspec( dllexport )
#endif

DLLEXPORT void  DestroyAurealImp (void* pImp) ;
DLLEXPORT void* CreateAurealImp  () ;


// ------------------------------ Globals ------------------------------------

extern IAudioEngine* g_pAudioEngine ;

HINSTANCE g_hinstThisDll;

static BOOL          g_bCrippledOS = FALSE;
static OSVERSIONINFO g_osviVerInfo;


const AudioPluginDesc_t descAurealImplementation = 
{
    0,
    "Aureal 3D Imp",
    "Daikatana Aureal Implementation",
    "ION Storm",
    kAudioImplementation,
    0,
    kAP_CPU_Medium,
    kAP_UseHWIfAvailable,
    0,
    0, // no properties
    CreateAurealImp,
    DestroyAurealImp
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
            return &descAurealImplementation ;
    }

    return (0);
}


// ------------------------------ Instance Management -----------------------------

DLLEXPORT void* CreateAurealImp ()
{
    IAudioSpecificImplementation* pImp ;

    pImp = new IAudioImp_Aureal3d ;

    return (pImp);
}    

DLLEXPORT void DestroyAurealImp (void* pVoidImp)
{
    IAudioSpecificImplementation* pImp = (IAudioSpecificImplementation*)pVoidImp ;
    delete pImp ;
}    





DLLEXPORT bool GetAudioImp (IAudioSpecificImplementation** ppImp)
{
    PRECOND (ppImp && *ppImp) ;

    *ppImp = new IAudioImp_Aureal3d ;

    return (true);
}    

DLLEXPORT bool ReleaseAudioImp (IAudioSpecificImplementation* pImp)
{
    delete pImp ;
    return (true);
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
