// ==========================================================================
//  File: DllMain.cpp  (for Miles Audio Engine)
// 
//  Contents    : Daikatana interface to Quake II audio engine
//				  that utilizes the Miles Audio Engine.
//  Date Created: 12/11/98
// ==========================================================================

#include <math.h>

#include	"..\..\..\base\client\client.h"
#include	"..\..\..\base\win32\winquake.h"
#include	"..\AudioEngineImports.h"

// ---------------------------- Constants -----------------------------------

extern "C" const int kAudioEngineVersionNum;

const int kAudioEngineVersionNum  = 2;
const char* szAudioEngineDesc = "ION Miles Audio v2.0" ;

#define DLLEXPORT  __declspec( dllexport)

// ------------------------------ Types -------------------------------------

// ----------------------------  Globals  -----------------------------------

HWND	         cl_hwnd = 0 ;       // Needed by DirectSound
client_state_t*  pcl = 0;
entity_state_t*	 pcl_parse_entities = 0 ;
client_static_t* pcls = 0 ;
audio_imports_t ai;

extern "C" DLLEXPORT int AEDLL_Init (DkAEInterfaceData_t* pData, AEInfo_t* pAEInfo)
{
    pAEInfo->uVersionNum		= kAudioEngineVersionNum;
    pAEInfo->pszDesc			= szAudioEngineDesc;
	
    cl_hwnd						= (HWND)pData->cl_hwnd;
    pcl							= (client_state_t*)pData->pcl;
    pcl_parse_entities			= (entity_state_t*)pData->cl_parse_entities;
    pcls						= (client_static_t*)pData->cls;
	
    ai.Cmd_AddCommand			=  pData->Cmd_AddCommand;
    ai.Cmd_RemoveCommand		=  pData->Cmd_RemoveCommand;
    ai.GetArgc					=  pData->GetArgc;
    ai.GetArgv					=  pData->GetArgv;
    ai.FS_LoadFile				=  pData->FS_LoadFile;
    ai.FS_FreeFile				=  pData->FS_FreeFile;
    ai.FS_FileLength			=  pData->FS_FileLength;
    ai.FS_FCloseFile			=  pData->FS_Close;
    ai.FS_FOpenFile				=  pData->FS_Open;  
	ai.FS_FOpenTextFile			=  pData->FS_OpenText;
    ai.FS_Read					=  pData->FS_Read;  
    ai.FS_Seek					=  pData->FS_Seek;  
    ai.FS_Tell					=  pData->FS_Tell;  
    ai.FS_Getc					=  pData->FS_Getc;  
    ai.FS_Gets					=  pData->FS_Gets;  
	
    ai.Cvar_Get					=  pData->Cvar_Get;
    ai.Cvar_Set					=  pData->Cvar_Set;
    ai.Cvar_SetValue			=  pData->Cvar_SetValue;
	
    ai.Com_Printf				= pData->Com_Printf;
    ai.Com_sprintf				= pData->Com_sprintf;
	ai.SCR_SubtitlePrint		= pData->SCR_SubtitlePrint;
	
    ai.CL_GetEntitySoundOrigin	= pData->CL_GetEntitySoundOrigin;
	
    ai.Com_DPrintf				= pData->Com_DPrintf;
    ai.Com_Error				= pData->Com_Error;
    ai.Com_PageInMemory			= pData->Com_PageInMemory;
	
	ai.X_Malloc					= pData->X_Malloc;
	ai.X_Free					= pData->X_Free;
	
	ai.DKM_InMenu				= pData->DKM_InMenu;
	
    return (1);
}    
