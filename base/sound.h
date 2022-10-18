#ifndef _SOUND_H_
#define _SOUND_H_

#include "dk_shared.h"

typedef struct tagAEInfo_t
{
    unsigned int uVersionNum ;
    const char*  pszDesc ;
} AEInfo_t ;

// -- Define the data struct used to pass data to the Audio Engine DLL...
typedef struct tagDkAEInterfaceData_t 
{
    unsigned int cl_hwnd ;
    void*        pcl ;  // actually a 'client_state_t*'
    void*         cl_parse_entities ; // actually entity_state_t*
    void*        cls ; // client_static_t

    // These fns are needed by the Audio Engine Dll...
    //
    void    (*Com_Printf)  (char *fmt, ...);
    void    (*Com_sprintf) (char *dest, int size, char *fmt, ...);
		void	  (*SCR_SubtitlePrint) (char *str, float msg_time);
    void    (*CL_GetEntitySoundOrigin) (int ent, CVector &org); //, CVector &vecAngles);

    void    (*Cmd_AddCommand) (char *name, void(*cmd)());
    void    (*Cmd_RemoveCommand) (char *name);
    int     (*GetArgc)();
    char*   (*GetArgv) (int i);
    cvar_t* (*Cvar_Get) (const char *name, const char *value, int flags);
    cvar_t* (*Cvar_Set)(const char *name, const char *value );
    void    (*Cvar_SetValue)(const char *name, float value );

    int     (*FS_LoadFile) (const char *name, void **buf);
    void    (*FS_FreeFile) (void *buf);
    int     (*FS_FileLength) (FILE *F);
    void    (*FS_Close)      (FILE *F);
    int     (*FS_Open)       (const char *filename, FILE **file);
		int     (*FS_OpenText)   (const char *filename, FILE **file);
    void    (*FS_Read)       (void *buffer, int len, FILE *f);
    int     (*FS_Seek)       (FILE *f, long offset, int origin);
    long    (*FS_Tell)       (FILE *f);
    int     (*FS_Getc)       (FILE *f);
    char*   (*FS_Gets)       (char *string, int n, FILE *f);
    long	(*FS_GetBytesRead)	();

    void    (*Com_DPrintf) (char *fmt, ...);
    void    (*Com_Error) (int code, char *fmt, ...);
    void    (*Com_PageInMemory) (byte *buffer, int size);

	int		(*Sys_Milliseconds) (void);

#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
    void    (*X_Free)   (void* mem);


		bool    (*DKM_InMenu)(void);

} DkAEInterfaceData_t ;


// -- DLL intitialation fn...
typedef int (*DLL_Init_t)(DkAEInterfaceData_t* dkData, AEInfo_t* pEntryPts) ;

#ifndef AE_N0_DEFINE_EXTERNS
    // -- If the audio engine is in a dll...
    //
    typedef void  (*S_Init_t) (void* hWnd);
    typedef void  (*S_Shutdown_t) (void);
    
	typedef void* (*S_StartSound_t) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float dist_min, float dist_max); 
	typedef void* (*S_StartSoundQuake2_t) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float attenuation, float timeofs); 

    typedef void  (*S_StartLocalSound_t) (const char *s);
    typedef void  (*S_StopAllSounds_t)(void);
    typedef void  (*S_Update_t) (CVector &origin, CVector &v_forward, CVector &v_right, CVector &v_up);
    typedef void  (*S_BeginRegistration_t) (void);
    typedef void* (*S_RegisterSound_t) (const char *pszName) ;
    typedef void  (*S_EndRegistration_t) (int nFreeSequenceCode);
    typedef void  (*S_ReverbPreset_t) (int nPresetID) ;
	typedef void  (*S_StartStream_t) (const char* name, float vol);

	typedef void  (*S_StartMusic_t) (const char* name, int chan);
	typedef void  (*S_StopMusic_t) (int chan);
	typedef void  (*S_StartMP3_t) (const char* name, int channel, float fvol, int play_count, qboolean bRestart);
	typedef void  (*S_StopMP3_t) (int channel);
	typedef void  (*S_PauseStreams_t) (qboolean bOnOff);

    // -- Define the global fn ptrs...
    extern void  (*S_Init) (void* hWnd);
    extern void  (*S_Shutdown) (void);
    
	extern void* (*S_StartSound) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float dist_min, float dist_max);
	extern void* (*S_StartSoundQuake2) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float attenuation, float timeofs);

    extern void  (*S_StartLocalSound) (const char *s);
    extern void  (*S_StopAllSounds)(void);
    extern void  (*S_Update) (CVector &origin, CVector &v_forward, CVector &v_right, CVector &v_up);
    extern void  (*S_BeginRegistration) (void);
    extern void* (*S_RegisterSound) (const char *pszName) ;
    extern void  (*S_EndRegistration) (int nFreeSequenceCode);
    extern void  (*S_ReverbPreset) (int nPresetID) ;
	extern void	 (*S_StartStream) (const char* name, float vol);
    
	extern void  (*S_StartMusic) (const char* name, int chan);
	extern void  (*S_StopMusic) (int chan);

	extern void  (*S_StartMP3) (const char* name, int chan, float fvol, int play_count, qboolean bRestart);
	extern void  (*S_StopMP3) (int chan);
	extern void  (*S_PauseStreams)( qboolean bOnOff );

    int  LoadAudioEngineDLL (const char* pszAudioEngineDLL) ;
    void FreeAudioEngineDLL () ;
#endif

#endif // _SOUND_H_
