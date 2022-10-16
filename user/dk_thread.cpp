// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

#include <process.h>            // For _beginthreadex
#include <stdio.h>
#include "dk_thread.h"


static char buff[128];



CThreadSync::CThreadSync()
{
    Init( "CThreadSyncFileMap", "CThreadSyncObject" );
}

CThreadSync::CThreadSync( LPCTSTR lpFileMappingName, LPCTSTR lpObjectName )
{
    Init( lpFileMappingName, lpObjectName );
}

CThreadSync::~CThreadSync()
{
    Destroy();
}

BOOL CThreadSync::Init( LPCTSTR lpFileMappingName, LPCTSTR lpObjectName )
{
    g_SWMRG.hMutexNoWriter  = NULL;
    g_SWMRG.hEventCanRead   = NULL;
    g_SWMRG.hEventNoReaders = NULL;
    g_SWMRG.hSemNumReaders  = NULL;

    g_hThread = NULL;

    g_lTerminate = 0;

    SharedFile	= NULL;
    SharedMem	= NULL;
    String		= NULL;

    strcpy( szFileMappingName, lpFileMappingName );
    strcpy( szObjectName, lpObjectName );

    return TRUE;
}

BOOL CThreadSync::CreateFileMap()
{
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    if ( !InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ) 
	{
		sprintf( buff, "unable to InitializeSecurityDescriptor, err == %d\n", GetLastError() );
		MessageBox( NULL, buff, "ERROR", MB_OK );
		return FALSE;
    }

    if ( !SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE) )
	{
        sprintf(buff,"unable to SetSecurityDescriptorDacl, err == %d\n", GetLastError() );
		MessageBox( NULL, buff, "ERROR", MB_OK );
		return FALSE;
    }

    SharedFile = CreateFileMapping( (HANDLE)-1, &sa, PAGE_READWRITE, 0, 256, szFileMappingName );
    if ( !SharedFile )
	{
        sprintf( buff, "Unable to create file mapping object, err == %d\n", GetLastError() );
		MessageBox( NULL, buff, "ERROR", MB_OK );
		return FALSE;
    }

    return TRUE;
}

BOOL CThreadSync::OpenFileForRead()
{
	if ( !SharedFile )
    {
	    SharedFile = OpenFileMapping( FILE_MAP_READ, TRUE, szFileMappingName );
        if (!SharedFile) 
	    {
		    return FALSE;
        }
    }

    SharedMem = MapViewOfFile( SharedFile, FILE_MAP_READ, 0, 0, 256 );
    if ( !SharedMem )
	{
        sprintf( buff, "Unable to map shared memory, err == %d\n", GetLastError() );
		MessageBox( NULL, buff, "ERROR", MB_OK );
		return FALSE;
    }

	String = (LPSTR)SharedMem;

    return TRUE;
}

BOOL CThreadSync::OpenFileForWrite()
{
    if ( !SharedFile )
    {	
	    SharedFile = OpenFileMapping( FILE_MAP_WRITE, TRUE, szFileMappingName );
        if ( !SharedFile )
	    {
		    return FALSE;
        }
    }

    SharedMem = MapViewOfFile( SharedFile, FILE_MAP_WRITE, 0, 0, 256 );
    if ( !SharedMem ) 
	{
        sprintf( buff, "Unable to map shared memory, err == %d\n", GetLastError() );
		MessageBox( NULL, buff, "ERROR", MB_OK );
		return FALSE;
    }

	String = (LPSTR)SharedMem;

    return TRUE;
}

void CThreadSync::CreateEvent()
{
	// Initialize the single-writer/multiple-reader 
	// guard synchronization object. This must be done 
	// before any thread attempts to use it.
	SWMRGInitialize( &g_SWMRG, szObjectName, 0 );
}

void CThreadSync::OpenEvent()
{
    SWMRGInitialize( &g_SWMRG, szObjectName, 1 );
}

void CThreadSync::CreateThread( PTHREAD_START lpStartAddr )
{
	DWORD dwThreadID;
	g_hThread = chBEGINTHREADEX( NULL, 0, lpStartAddr, NULL, 0, &dwThreadID );
}

void CThreadSync::Destroy()
{
	// When the user shuts down the process, clean up.
	// 1. Inform the threads that the process is terminating.
	InterlockedIncrement((PLONG) &g_lTerminate);

	if ( g_hThread )
    {
        // 2. Wait for all of the threads to terminate. This 
	    // might take a long time because some threads might be
	    // sleeping and therefore not checking the g_lTerminate
	    // variable.
	    WaitForMultipleObjects(1, &g_hThread, TRUE, INFINITE);

	    // 3. Close all of our handles to the threads.
	    CloseHandle(g_hThread);
    }

	// 4. Delete the Single Writer/Multiple Reader Guard
	// synchronization object. This must be done when it
	// is known that no threads will attempt to use it.
	SWMRGDelete(&g_SWMRG);
}

int CThreadSync::WaitToRead( DWORD dwTimeOut )
{
    return SWMRGWaitToRead(&g_SWMRG, dwTimeOut);
}
void CThreadSync::DoneReading()
{
    SWMRGDoneReading(&g_SWMRG);
}

int CThreadSync::WaitToWrite( DWORD dwTimeOut )
{
    return SWMRGWaitToWrite(&g_SWMRG, dwTimeOut);
}
void CThreadSync::DoneWriting()
{
    SWMRGDoneWriting(&g_SWMRG);
}

void CThreadSync::WriteToSharedData( const char *pData )
{
	strcpy( String, pData );
}

void CThreadSync::ReadSharedData( char *pData )
{
	strcpy( pData, String );
}

int CThreadSync::CreateFileMap_OpenWrite_CreateEvent()
{
    if ( !CreateFileMap() )
    {
        return FALSE;
    }
    if ( !OpenFileForWrite() )
    {
        return FALSE;
    }
    CreateEvent();

    return TRUE;
}

int CThreadSync::CreateFileMap_OpenRead_CreateEvent()
{
    if ( !CreateFileMap() )
    {
        return FALSE;
    }
    if ( !OpenFileForRead() )
    {
        return FALSE;
    }
    CreateEvent();

    return TRUE;
}

int CThreadSync::OpenFileMapForRead_OpenEvent()
{
    if ( !OpenFileForRead() )
    {
        return FALSE;
    }
    OpenEvent();

    return TRUE;
}

int CThreadSync::OpenFileMapForWrite_OpenEvent()
{
    if ( !OpenFileForWrite() )
    {
        return FALSE;
    }
    OpenEvent();

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
