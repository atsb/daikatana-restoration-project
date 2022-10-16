#ifndef _DK_THREAD_H
#define _DK_THREAD_H

#include "CmnHdr.H"
#include <windows.h>
#include "SWMRG.H"

class CThreadSync
{
public:
    CThreadSync();
    CThreadSync( LPCTSTR lpFileMappingName, LPCTSTR lpObjectName );
    ~CThreadSync();
    
    BOOL Init( LPCTSTR lpFileMappingName, LPCTSTR lpObjectName );
    BOOL CreateFileMap();
    BOOL OpenFileForRead();
    BOOL OpenFileForWrite();
    void CreateEvent();
    void OpenEvent();
    void CreateThread( PTHREAD_START lpStartAddr );
    void Destroy();
    int WaitToRead( DWORD dwTimeOut );
    void DoneReading();
    int WaitToWrite( DWORD dwTimeOut );
    void DoneWriting();
    void WriteToSharedData( const char *pData );
    void ReadSharedData( char *pData );
    int CreateFileMap_OpenWrite_CreateEvent();
    int CreateFileMap_OpenRead_CreateEvent();
    int OpenFileMapForRead_OpenEvent();
    int OpenFileMapForWrite_OpenEvent();

private:
    // single-writer/multiple-reader guard synchronization object
    SWMRG g_SWMRG;

    // Array of thread handles needed for process termination
    HANDLE g_hThread;

    // Flag indicating when the threads should terminate
    // The flag is volatile because it is changed asynchronously
    long volatile g_lTerminate;

    HANDLE SharedFile;
    LPVOID SharedMem;
    LPSTR  String;

    char szFileMappingName[64];
    char szObjectName[64];
};

#endif 
