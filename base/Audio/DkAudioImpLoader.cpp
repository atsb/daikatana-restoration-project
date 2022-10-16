// ==========================================================================
//  File: DkAudioImpLoader.cpp
// 
//  Contents    : Handles loading in the Audio Engine for Daikatana
//  Date Created: 10-12-98
// ==========================================================================

//#ifdef AE_USE_ENGINE_DLL

#include <math.h>
#include "client.h"
//#include "AudioDebug.h"
//#include "Audio.h"
#include <windows.h>
#include "sound.h"

// ---------------------------- Constants -----------------------------------

// ------------------------------ Types -------------------------------------

// ----------------------------  Globals  -----------------------------------

HWND cl_hwnd ;
HANDLE g_hAEDLL = 0 ;

// -- Define the global fn ptrs that will point to the fns in the dll...
int LoadAudioEngineDLL (const char* pszAudioEngineDLL);
void FreeAudioEngineDLL ();

void  (*S_Init) (void* hWnd);
void  (*S_Shutdown) (void);

void* (*S_StartSound) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float dist_min, float dist_max);
void* (*S_StartSoundQuake2) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float attenuation, float timeofs);

void  (*S_StartLocalSound) (const char *s);
void  (*S_StopAllSounds)(void);
void  (*S_Update) (const CVector &origin, const CVector &v_forward, const CVector &v_right, const CVector &v_up);
void  (*S_BeginRegistration) (void);
void* (*S_RegisterSound) (const char *pszName) ;
void  (*S_EndRegistration) (int nFreeSequenceCode);
void  (*S_ReverbPreset) (int nPresetID) ;
void  (*S_StartStream) (const char* name, float vol);
void  (*S_StartMusic) (const char* name, int chan);
void  (*S_StopMusic) (int chan);
void  (*S_StartMP3)(const char* name, int channel, float fvol, int play_count, qboolean bRestart );
void  (*S_StopMP3)(int channel);
void  (*S_PauseStreams)(qboolean bOnOff);

void CL_GetEntitySoundOrigin(int ent, CVector &org); //, CVector &vecAngles);


static const char* szAudioEngineDLL    = "audio.dll" ;


// ---------------------------- Functions ----------------------------------

int LoadAudioEngineDLL (const char* pszAudioEngineDLL)
{
    Com_Printf ("-> Loading Audio Engine DLL '%s'\n", pszAudioEngineDLL) ;
    if (g_hAEDLL)
        FreeAudioEngineDLL () ;

    // -- load the dll...
    g_hAEDLL = ::LoadLibrary (pszAudioEngineDLL) ;
    if (!g_hAEDLL)
    {
        Com_Printf ("ERROR Loading Audio Engine DLL '%s'\n", pszAudioEngineDLL) ;

		char szBuff [256] ;
		Com_sprintf(szBuff, sizeof( szBuff ), "ERROR Loading Audio Engine DLL '%s'\n", pszAudioEngineDLL) ;
	    MessageBeep (MB_ICONEXCLAMATION);
		MessageBox  (NULL, szBuff, "Error", MB_OK | MB_ICONEXCLAMATION);
		
        return (0);
    }

    // -- setup the fn pts that the dll needs...
    //
    DkAEInterfaceData_t dkData ;
    memset (&dkData, 0, sizeof (dkData)) ;

    DKM_InMenu();

    dkData.cl_hwnd  = (unsigned int)cl_hwnd ;
    dkData.pcl      = &cl ;
    dkData.cl_parse_entities = &cl_parse_entities ;
    dkData.cls       = &cls ;
    
    dkData.Cmd_AddCommand = Cmd_AddCommand;
    dkData.Cmd_RemoveCommand = Cmd_RemoveCommand;
    dkData.GetArgc = GetArgc;
    dkData.GetArgv = GetArgv;
    dkData.Com_Printf = Com_Printf;
    dkData.Com_sprintf = Com_sprintf;
    dkData.SCR_SubtitlePrint = SCR_SubtitlePrint;
    dkData.CL_GetEntitySoundOrigin = CL_GetEntitySoundOrigin ;

	dkData.FS_FileLength = FS_filelength;
	dkData.FS_Close = FS_FCloseFile;
	dkData.FS_Open = FS_FOpenFile;
	dkData.FS_OpenText = FS_FOpenTextFile;
	dkData.FS_Read = FS_Read;
	dkData.FS_LoadFile = FS_LoadFile;
	dkData.FS_FreeFile = FS_FreeFile;
	dkData.FS_Seek = FS_Seek;
	dkData.FS_Tell = FS_Tell;
	dkData.FS_Getc = FS_Getc;
	dkData.FS_Gets = FS_Gets;

    dkData.Cvar_Get = Cvar_Get;
    dkData.Cvar_Set = Cvar_Set;
    dkData.Cvar_SetValue = Cvar_SetValue;

    dkData.Com_DPrintf = Com_DPrintf;
    dkData.Com_Error = Com_Error;
    dkData.Com_PageInMemory = Com_PageInMemory;

    dkData.X_Malloc = X_Malloc;
	dkData.X_Free = X_Free;

	dkData.DKM_InMenu = DKM_InMenu;

    // -- Initialize the DLL with the daikatana import data...
    //
    // -- get the dll init fn...
    DLL_Init_t fnDLL_Init = (DLL_Init_t) GetProcAddress ((HINSTANCE)g_hAEDLL, "AEDLL_Init");
    if (!fnDLL_Init)
    {
        Com_Printf ("Invalid Audio Engine DLL '%s'\n", pszAudioEngineDLL) ;
        return (0);
    }

    // -- call the init fn...
    AEInfo_t AEInfo ;
	if (!fnDLL_Init (&dkData, &AEInfo))
	{
        Com_Printf ("Invalid Audio Engine DLL '%s'\n", pszAudioEngineDLL) ;
        return (0);
	}

    Com_Printf ("AudioEngine: %s - version %u\n", AEInfo.pszDesc, AEInfo.uVersionNum) ;

	unsigned int nInitFlag = 0;
    // -- Get the Audio Engine API Entry points...
    S_Init					= (S_Init_t)				GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_Init");
    S_Shutdown				= (S_Shutdown_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_Shutdown");
    S_StartSound			= (S_StartSound_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartSound");
	S_StartSoundQuake2		= (S_StartSoundQuake2_t)	GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartSoundQuake2");
    S_StartLocalSound		= (S_StartLocalSound_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartLocalSound");
    S_StopAllSounds			= (S_StopAllSounds_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StopAllSounds");
    S_Update				= (S_Update_t)				GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_Update");
    S_BeginRegistration		= (S_BeginRegistration_t)	GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_BeginRegistration");
    S_RegisterSound			= (S_RegisterSound_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_RegisterSound");
    S_EndRegistration		= (S_EndRegistration_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_EndRegistration");
    S_ReverbPreset			= (S_ReverbPreset_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_ReverbPreset");
    S_ReverbPreset			= (S_ReverbPreset_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_ReverbPreset");
	S_StartStream			= (S_StartStream_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartStream");
	S_StartMusic			= (S_StartMusic_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartMusic");
	S_StopMusic				= (S_StopMusic_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StopMusic");
	S_StartMP3				= (S_StartMP3_t)			GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StartMP3");
	S_StopMP3				= (S_StopMP3_t)				GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_StopMP3");
	S_PauseStreams			= (S_PauseStreams_t)		GetProcAddress  ((HINSTANCE)g_hAEDLL, "S_PauseStreams");

    // -- need to have valid values for all ptrs...
    if (!S_Init || !S_Shutdown || !S_StartSound || !S_StartSoundQuake2 || !S_StartLocalSound || !S_StopAllSounds   
       || !S_Update || !S_BeginRegistration || !S_RegisterSound || !S_EndRegistration || !S_ReverbPreset 
	   || !S_ReverbPreset || !S_StartMusic || !S_StopMusic || !S_StartMP3 || !S_StopMP3 || !S_PauseStreams)
	{
        FreeAudioEngineDLL () ;
        return (0);
    }
	
	return 1;
}    


void FreeAudioEngineDLL ()
{
    Com_Printf ("Freeing Audio Engine DLL...\n") ;
    if (g_hAEDLL)
    {
        S_Init              = 0;
        S_Shutdown          = 0;
        S_StartSound        = 0;
		S_StartSoundQuake2  = NULL;
        S_StartLocalSound   = 0;
        S_StopAllSounds     = 0;
        S_Update            = 0;
        S_BeginRegistration = 0;
        S_RegisterSound     = 0;
        S_EndRegistration   = 0;
        S_ReverbPreset      = 0;
        S_ReverbPreset      = 0;
		S_StartMP3			= 0;
		S_StopMP3			= 0;
		S_PauseStreams		= 0;
		FreeLibrary( ( HMODULE ) g_hAEDLL );	// SCG[1/9/00]: Let's not create any memory leaks.
		g_hAEDLL = 0 ;
    }
}


//#endif // ifdef AE_USE_ENGINE_DLL
// ==========================================================================
//                              End of File
// ==========================================================================
