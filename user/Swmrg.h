/*************************************************************
Module name: SWMRG.H
Notices: Copyright (c) 1995-1997 Jeffrey Richter
*************************************************************/


// The single-writer/multiple-reader guard 
// compound synchronization object
typedef struct SingleWriterMultiReaderGuard 
{
   // This mutex guards access to the other objects
   // managed by this data structure and also indicates 
   // whether any writer threads are writing.
   HANDLE hMutexNoWriter;

   HANDLE hEventCanRead;

   // This manual-reset event is signaled when
   // no reader threads are reading.
   HANDLE hEventNoReaders;

   // This semaphore is used simply as a counter that is
   // accessible between multiple processes. It is NOT
   // used for thread synchronization.
   // The count is the number of reader threads reading.
   HANDLE hSemNumReaders;

} SWMRG, *PSWMRG;


////////////////////////////////////////////////////////////


// Initializes a SWMRG structure. This structure must be 
// initialized before any writer or reader threads attempt
// to wait on it.
// The structure must be allocated by the application and 
// the structure's address is passed as the first parameter.
// The lpszName parameter is the name of the object. Pass
// NULL if you do not want to share the object.
BOOL  SWMRGInitialize (PSWMRG pSWMRG, LPCTSTR lpszName, int nFlag);


// Deletes the system resources associated with a SWMRG 
// structure. The structure must be deleted only when
// no writer or reader threads in the calling process
// will wait on it.
void  SWMRGDelete (PSWMRG pSWMRG);


// A writer thread calls this function to know when 
// it can successfully write to the shared data.
DWORD SWMRGWaitToWrite (PSWMRG pSWMRG, DWORD dwTimeout);


// A writer thread calls this function to let other threads
// know that it no longer needs to write to the shared data.
void  SWMRGDoneWriting (PSWMRG pSWMRG);


// A reader thread calls this function to know when 
// it can successfully read the shared data.
DWORD SWMRGWaitToRead  (PSWMRG pSWMRG, DWORD dwTimeout);


// A reader thread calls this function to let other threads
// know when it no longer needs to read the shared data.
void  SWMRGDoneReading (PSWMRG pSWMRG);


/////////////////////// End Of File ////////////////////////
