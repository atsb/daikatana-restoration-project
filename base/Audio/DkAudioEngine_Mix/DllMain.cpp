// ==========================================================================
//  File: DkAudioEngine_Mix.c
// 
//  Contents    : Quake/Daikatana interface to Quake II audio engine that
//                does its own mixing into a DSound (2d) buffer.
//  Date Created: 10/13/98
// ==========================================================================

#include <math.h>

#define AE_COMPILING_DLL
#define NO_COMMAND_FNS 
#define NO_FILESYSTEM_FNS
#define NO_ZMALLOC
#include	"..\..\base\client\client.h"
#include	"..\..\base\win32\winquake.h"

// ---------------------------- Constants -----------------------------------

extern const int kAudioEngineVersionNum;

const int kAudioEngineVersionNum  = 1 ;
const char* szAudioEngineDesc = "ION Audio Engine Internal Mixer" ;

#ifndef	DLLEXPORT
#define DLLEXPORT	extern "C"	__declspec( dllexport )
#endif

// ------------------------------ Types -------------------------------------

// ----------------------------  Globals  -----------------------------------

HWND	         cl_hwnd = 0 ;       // Needed by DirectSound
client_state_t*  pcl = 0;
entity_state_t*	 pcl_parse_entities = 0 ;
client_static_t* pcls = 0 ;

void    (*Com_Printf)  (char *fmt, ...);
void    (*Com_sprintf) (char *dest, int size, char *fmt, ...);
void    (*CL_GetEntitySoundOrigin) (int ent, CVector &org, CVector &vecAngles);

void	(*Cmd_AddCommand) (char *name, void(*cmd)());
void	(*Cmd_RemoveCommand) (char *name);
void	(*Cmd_ExecuteText) (int exec_when, char *text);
int		(*GetArgc)();
char	*(*GetArgv) (int i);
cvar_t	*(*Cvar_Get) (const char *name, const char *value, int flags);
cvar_t	*(*Cvar_Set)(const char *name, const char *value );
void	 (*Cvar_SetValue)(const char *name, float value );
void	(*Con_Printf) (int print_level, char *str, ...);

int     (*FS_FOpenFile)  (const char *filename, FILE **file);
void    (*FS_FCloseFile) (FILE *f);
int		(*FS_LoadFile)   (const char *name, void **buf);
void	(*FS_FreeFile)   (void *buf);
int		(*FS_FileLength) (FILE *F);
void	(*FS_Read)		 (void *buffer, int len, FILE *f);
int		(*FS_Seek)		 (FILE *f, long offset, int origin);
long	(*FS_Tell)		 (FILE *f);
int		(*FS_Getc)		 (FILE *f);
char*	(*FS_Gets)		 (char *string, int n, FILE *f);
char	*(*FS_Gamedir)();

void    (*Com_DPrintf) (char *fmt, ...);
void    (*Com_Error) (int code, char *fmt, ...);
void    (*Com_PageInMemory) (byte *buffer, int size);
void    (*Z_Free)   (void *ptr);
void*   (*Z_Malloc) (int size); 
void	(*Sys_Warning) (char *msg, ...);



/*
vec_t VectorNormalize (CVector &v)
{
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);		// FIXME

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}
*/

DLLEXPORT int AEDLL_Init (DkAEInterfaceData_t* pData, AEInfo_t* pAEInfo)
{
    pAEInfo->uVersionNum = kAudioEngineVersionNum;
    pAEInfo->pszDesc     = szAudioEngineDesc ;

    cl_hwnd             = (HWND)pData->cl_hwnd          ;
    pcl                 = (client_state_t*)pData->pcl ;
    pcl_parse_entities  = (entity_state_t*)pData->cl_parse_entities ;
    pcls                = (client_static_t*)pData->cls ;

    Cmd_AddCommand      =  pData->Cmd_AddCommand   ;
    Cmd_RemoveCommand   =  pData->Cmd_RemoveCommand;
    GetArgc             =  pData->GetArgc          ;
    GetArgv             =  pData->GetArgv          ;
    FS_LoadFile         =  pData->FS_LoadFile      ;
    FS_FreeFile         =  pData->FS_FreeFile      ;
    FS_FileLength       =  pData->FS_FileLength    ;
    FS_FCloseFile       =  pData->FS_Close	       ;
    FS_FOpenFile        =  pData->FS_Open	       ;  
    FS_Read	            =  pData->FS_Read	       ;  
    FS_Seek	            =  pData->FS_Seek	       ;  
    FS_Tell	            =  pData->FS_Tell	       ;  
    FS_Getc	            =  pData->FS_Getc	       ;  
    FS_Gets	            =  pData->FS_Gets	       ;  

    Cvar_Get            =  pData->Cvar_Get         ;
    Cvar_Set            =  pData->Cvar_Set         ;
    Cvar_SetValue       =  pData->Cvar_SetValue    ;

    Com_Printf          = pData->Com_Printf ;
    Com_sprintf         = pData->Com_sprintf ;
    CL_GetEntitySoundOrigin = pData->CL_GetEntitySoundOrigin ;

    Com_DPrintf         = pData->Com_DPrintf ;
    Com_Error           = pData->Com_Error ;
    Com_PageInMemory    = pData->Com_PageInMemory ;
    Z_Malloc            = pData->Z_Malloc ;
    Z_Free              = pData->Z_Free ;

    return (1);
}    
