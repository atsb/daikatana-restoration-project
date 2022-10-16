#ifndef _LOGFILE_H

#define LOG_TO_NONE		0x00000000
#define LOG_TO_FILE		0x00000001
#define LOG_TO_DBWIN	0x00000002
#define LOG_TO_NETWORK	0x00000004


#define	LOGTYPE_GRAPHICS	0
#define	LOGTYPE_AI			1
#define	LOGTYPE_WEAPONS		2
#define	LOGTYPE_AUDIO		3
#define	LOGTYPE_PHYSICS		4
#define	LOGTYPE_SCRIPT		5
#define	LOGTYPE_NETWORK		6
#define	LOGTYPE_USERIO		7
#define LOGTYPE_MEMORY		8
#define LOGTYPE_MISC  		9
#define LOGTYPE_MAX		   10


#ifdef _DEBUG

int DKLOG_Initialize( char *logFilePath );
void DKLOG_Close();
void DKLOG_Write( int nLogType, float fCurrentTime, char *fmt, ... );

#else

__inline int DKLOG_Initialize( char *logFilePath )	{ return 0; };
__inline void DKLOG_Close()							{};
__inline void DKLOG_Write( int nLogType, float fCurrentTime, char *fmt, ... )	{};

#endif _DEBUG

#endif _LOGFILE_H

