#include <windows.h>
#include "..\AudioDebug.h"
#include "AudioImp_DSound3d.h"
#include "..\AudioPlugin.h"

#ifndef	DLLEXPORT
#define DLLEXPORT	extern "C"	__declspec( dllexport )
#endif

DLLEXPORT void  DestroyDSoundImp (void* pImp) ;
DLLEXPORT void* CreateDSoundImp  () ;


// ------------------------------ Globals ------------------------------------

extern IAudioEngine* g_pAudioEngine ;

HINSTANCE g_hinstThisDll;

static BOOL          g_bCrippledOS = FALSE;
static OSVERSIONINFO g_osviVerInfo;


const AudioPluginDesc_t descDSoundImplementation = 
{
    0,
    "DSound3D Imp",
    "Daikatana DSound3D Implementation",
    "ION Storm",
    kAudioImplementation,
    0,
    kAP_CPU_Medium,
    kAP_UseHWIfAvailable,
    0,
    0, // no properties
    CreateDSoundImp,
    DestroyDSoundImp
} ;


const AudioPluginDesc_t descEcho = 
{
    1,
    "Echo Simple",
    "Echo AFX",
    "ION Storm",
    kAudioFX,
    0,  // subType
    kAP_CPU_Medium,
    kAP_SoftwareOnly,
    0,
    0, // no properties
    0,
    0
} ;


// ------------------------------ Plugin Interface -----------------------------

DLLEXPORT int GetNumPlugins ()
{
    return (2);
}


DLLEXPORT const AudioPluginDesc_t* GetPluginDesc (int nPluginIndex)
{
    switch (nPluginIndex)
    {
        case 0:
            return &descDSoundImplementation ;
        case 1:
            return &descEcho;
    }

    return (0);
}


// ------------------------------ Instance Management -----------------------------

DLLEXPORT void* CreateDSoundImp ()
{
    IAudioSpecificImplementation* pImp ;

    pImp = new IAudioImp_DSound3d ;

    return (pImp);
}    

DLLEXPORT void DestroyDSoundImp (void* pVoidImp)
{
    IAudioSpecificImplementation* pImp = (IAudioSpecificImplementation*)pVoidImp ;
    delete pImp ;
}    





DLLEXPORT bool GetAudioImp (IAudioSpecificImplementation** ppImp)
{
    PRECOND (ppImp && *ppImp) ;

    *ppImp = new IAudioImp_DSound3d ;

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
