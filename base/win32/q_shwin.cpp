
#include "qcommon.h"
#include "winquake.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

//===============================================================================
int		hunkcount;

byte	*membase;
int		hunkmaxsize;
int		cursize;

#define	VIRTUAL_ALLOC
void *Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;
#ifdef VIRTUAL_ALLOC
	membase = ( unsigned char * ) VirtualAlloc (NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS);
#else
	membase = malloc (maxsize);
	memset (membase, 0, maxsize);
#endif
	if (!membase)
		Sys_Error ("VirtualAlloc reserve failed");
	return (void *)membase;
}
void *Hunk_Alloc (int size)
{
	void	*buf;
	// round to cacheline
	size = (size+31)&~31;

#ifdef VIRTUAL_ALLOC
	// commit pages as needed
//	buf = VirtualAlloc (membase+cursize, size, MEM_COMMIT, PAGE_READWRITE);
	buf = VirtualAlloc (membase, cursize+size, MEM_COMMIT, PAGE_READWRITE);
	if (!buf)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buf, 0, NULL);
		Sys_Error ("VirtualAlloc commit failed.\n%s", buf);
	}
#endif
	cursize += size;
	if (cursize > hunkmaxsize)
		Sys_Error ("Hunk_Alloc overflow");
	return (void *)(membase+cursize-size);
}
int Hunk_End (void)
{
	// free the remaining unused virtual memory
#if 0
	void	*buf;
	// write protect it
	buf = VirtualAlloc (membase, cursize, MEM_COMMIT, PAGE_READONLY);
	if (!buf)
		Sys_Error ("VirtualAlloc commit failed");
#endif

	hunkcount++;
//Com_Printf ("hunkcount: %i\n", hunkcount);
	return cursize;
}
void Hunk_Free (void *base)
{
	if ( base )
#ifdef VIRTUAL_ALLOC
		VirtualFree (base, 0, MEM_RELEASE);
#else
		free (base);
#endif

	hunkcount--;
}
//===============================================================================
/*
================
Sys_Milliseconds
================
*/
int	curtime;
int Sys_Milliseconds (void)
{
	static int		base;
	static qboolean	initialized = false;

	if (!initialized)
	{	// let base retain 16 bits of effectively random data
		base = timeGetTime() & 0xffff0000;
		initialized = true;
	}
	curtime = timeGetTime() - base;

	return curtime;
}
void Sys_Mkdir (char *path)
{
	_mkdir (path);
}
//============================================
char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
int		findhandle;

static qboolean CompareAttributes( unsigned found, unsigned musthave, unsigned canthave )
{
	if ( ( found & _A_RDONLY ) && ( canthave & SFF_RDONLY ) )
		return false;
	if ( ( found & _A_HIDDEN ) && ( canthave & SFF_HIDDEN ) )
		return false;
	if ( ( found & _A_SYSTEM ) && ( canthave & SFF_SYSTEM ) )
		return false;
	if ( ( found & _A_SUBDIR ) && ( canthave & SFF_SUBDIR ) )
		return false;
	if ( ( found & _A_ARCH ) && ( canthave & SFF_ARCH ) )
		return false;

	if ( ( musthave & SFF_RDONLY ) && !( found & _A_RDONLY ) )
		return false;
	if ( ( musthave & SFF_HIDDEN ) && !( found & _A_HIDDEN ) )
		return false;
	if ( ( musthave & SFF_SYSTEM ) && !( found & _A_SYSTEM ) )
		return false;
	if ( ( musthave & SFF_SUBDIR ) && !( found & _A_SUBDIR ) )
		return false;
	if ( ( musthave & SFF_ARCH ) && !( found & _A_ARCH ) )
		return false;

	return true;
}
char *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave )
{
	struct _finddata_t findinfo;
	if (findhandle)
		Sys_Error ("Sys_BeginFind without close");
	findhandle = 0;
	COM_FilePath (path, findbase);
	findhandle = _findfirst (path, &findinfo);
	if (findhandle == -1)
		return NULL;
	if ( !CompareAttributes( findinfo.attrib, musthave, canthave ) )
		return NULL;
	Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}
char *Sys_FindNext ( unsigned musthave, unsigned canthave )
{
	struct _finddata_t findinfo;
	if (findhandle == -1)
		return NULL;
	if (_findnext (findhandle, &findinfo) == -1)
		return NULL;
	if ( !CompareAttributes( findinfo.attrib, musthave, canthave ) )
		return NULL;
	Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}
void Sys_FindClose (void)
{
	if (findhandle != -1)
		_findclose (findhandle);
	findhandle = 0;
}
void Sys_Delete(char *path)
{
	DeleteFile(path);
}
void Sys_DateTime(char *path, char *date, int dateSize, char *time, int timeSize)
{
	WIN32_FIND_DATA fd;
	HANDLE FindHandle = 0;

	FindHandle = FindFirstFile(path,&fd);
 	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		if (date) *date = NULL;
		if (time) *time = NULL;
		return;
	}

	if (date && dateSize)
	{
		FILETIME ft;
		SYSTEMTIME st;
		FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft);
		FileTimeToSystemTime(&ft,&st);
		GetDateFormat(LOCALE_SYSTEM_DEFAULT,DATE_SHORTDATE,&st,NULL,date,dateSize);
	}

	if (time && timeSize)
	{
		FILETIME ft;
		SYSTEMTIME st;
		FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft);
		FileTimeToSystemTime(&ft,&st);
		GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_NOSECONDS,&st,NULL,time,timeSize);
	}

	if (FindHandle != INVALID_HANDLE_VALUE)
		FindClose(FindHandle);
}

int Sys_CompareFileDateTime(char *path1, char *path2)
{
	if (!path1 || !path2)
		return 0;

	FILETIME ft1,ft2;
	WIN32_FIND_DATA fd;
	HANDLE FindHandle = 0;

	// get the info for the first file
	FindHandle = FindFirstFile(path1,&fd);
	if (FindHandle == INVALID_HANDLE_VALUE)
		return 0;

	FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft1);
	if (FindHandle != INVALID_HANDLE_VALUE)
		FindClose(FindHandle);

	// get the info for the second file
	FindHandle = FindFirstFile(path2,&fd);
	if (FindHandle == INVALID_HANDLE_VALUE)
		return 0;

	FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft2);
	if (FindHandle != INVALID_HANDLE_VALUE)
		FindClose(FindHandle);

	return CompareFileTime(&ft1,&ft2);
}
//============================================
