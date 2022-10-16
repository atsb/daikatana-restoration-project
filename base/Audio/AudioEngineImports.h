#ifndef _AUDIOENGINEIMPORTS_H
#define _AUDIOENGINEIMPORTS_H

typedef struct audio_imports_s
{
	void    (*Com_Printf)  (char *fmt, ...);
	void    (*Com_sprintf) (char *dest, int size, char *fmt, ...);
	void    (*CL_GetEntitySoundOrigin) (int ent, CVector& org); //, CVector& vecAngles);

	void	(*Cmd_AddCommand) (char *name, void(*cmd)());
	void	(*Cmd_RemoveCommand) (char *name);
	void	(*Cmd_ExecuteText) (int exec_when, char *text);
	int		(*GetArgc)();
	char	*(*GetArgv) (int i);
	cvar_t	*(*Cvar_Get) (const char *name, const char *value, int flags);
	cvar_t	*(*Cvar_Set)(const char *name, const char *value );
	void	 (*Cvar_SetValue)(const char *name, float value );
	void	(*Con_Printf) (int print_level, char *str, ...);
	void  (*SCR_SubtitlePrint) (char *str, float msg_time);

	int     (*FS_FOpenFile)  (const char *filename, FILE **file);
	int     (*FS_FOpenTextFile)  (const char *filename, FILE **file);
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
	void	(*Sys_Warning) (char *msg, ...);

#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free)	(void* mem);

  bool  (*DKM_InMenu) (void);

} audio_imports_t;

extern audio_imports_t ai;

#endif // _AUDIOENGINEIMPORTS_H
