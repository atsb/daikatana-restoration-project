/************************************************************
Module name: SWMRG.C
Notices: Copyright (c) 1995-1997 Jeffrey Richter
************************************************************/

//#include "stdafx.h"
#include "CmnHdr.H"                  /* See Appendix C. */
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "SWMRG.H"		      // The header file


/////////////////////////////////////////////////////////////


static LPCTSTR ConstructObjName( LPCTSTR lpszPrefix, LPCTSTR lpszSuffix, LPTSTR lpszFullName, size_t cbFullName, PBOOL fOk) 
{
	*fOk = TRUE;	// Assume success.

	if (lpszSuffix == NULL)
		return(NULL);

	if ((_tcslen(lpszPrefix) + _tcslen(lpszSuffix)) >= cbFullName) 
	{
		// If the strings will overflow the buffer,
		// indicate an error.
		*fOk = FALSE;
		return(NULL);
	}

	_tcscpy(lpszFullName, lpszPrefix);
	_tcscat(lpszFullName, lpszSuffix);
	return(lpszFullName);
}


/////////////////////////////////////////////////////////////

// nFlag == 0 => CreateEvent
// nFlag == 1 => OpenEvent
BOOL SWMRGInitialize(PSWMRG pSWMRG, LPCTSTR lpszName, int nFlag ) 
{
	TCHAR szFullObjName[100];
	LPCTSTR lpszObjName;
	BOOL fOk;

	// Initialize all data members to NULL so that we can
	// accurately check whether an error has occurred.
	pSWMRG->hMutexNoWriter = NULL;
	pSWMRG->hEventNoReaders = NULL;
	pSWMRG->hSemNumReaders = NULL;

	// This mutex guards access to the other objects
	// managed by this data structure and also indicates 
	// whether there are any writer threads writing.
	// Initially no thread owns the mutex.
	lpszObjName = ConstructObjName(	__TEXT("SWMRGMutexNoWriter"), lpszName,
				szFullObjName, chDIMOF(szFullObjName), &fOk);
	if (fOk)
	{
		if ( nFlag == 0 )
		{
			pSWMRG->hMutexNoWriter = CreateMutex(NULL, FALSE, lpszObjName);
		}
		else
		{
			pSWMRG->hMutexNoWriter = OpenMutex(MUTEX_ALL_ACCESS | SYNCHRONIZE, FALSE, lpszObjName);
		}
	}

	// Create the manual-reset event that is signalled when 
	// no reader threads are reading.
	// Initially no reader threads are reading.
	lpszObjName = ConstructObjName( __TEXT("SWMRGEventNoWriter"), lpszName,
		szFullObjName, chDIMOF(szFullObjName), &fOk);
	if (fOk)
	{
		if ( nFlag == 0 )
		{
			pSWMRG->hEventCanRead = CreateEvent(NULL, TRUE, FALSE, lpszObjName);
		}
		else
		{
			pSWMRG->hEventCanRead = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, lpszObjName);
		}
	}

	// Create the manual-reset event that is signalled when 
	// no reader threads are reading.
	// Initially no reader threads are reading.
	lpszObjName = ConstructObjName( __TEXT("SWMRGEventNoReaders"), lpszName,
		szFullObjName, chDIMOF(szFullObjName), &fOk);
	if (fOk)
	{
		if ( nFlag == 0 )
		{
			pSWMRG->hEventNoReaders = CreateEvent(NULL, TRUE, TRUE, lpszObjName);
		}
		else
		{
			pSWMRG->hEventNoReaders = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, lpszObjName);
		}
	}

	// Initialize the variable that indicates the number of 
	// reader threads that are reading.
	// Initially no reader threads are reading.
	lpszObjName = ConstructObjName( __TEXT("SWMRGSemNumReaders"), lpszName,
		szFullObjName, chDIMOF(szFullObjName), &fOk);
	if (fOk)
	{
		if ( nFlag == 0 )
		{
			pSWMRG->hSemNumReaders = CreateSemaphore(NULL, 0, 0x7FFFFFFF, lpszObjName);
		}
		else
		{
			pSWMRG->hSemNumReaders = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, lpszObjName);
		}
	}

	if ((NULL == pSWMRG->hMutexNoWriter)   || 
		 (NULL == pSWMRG->hEventCanRead)   ||	
		 (NULL == pSWMRG->hEventNoReaders)	||
		 (NULL == pSWMRG->hSemNumReaders)) 
	 {
		// If a synchronization object could not be created,
		// destroy any created objects and return failure.
		SWMRGDelete(pSWMRG);
		fOk = FALSE;
	} 
	else 
	{
		fOk = TRUE;
	}

	// Return TRUE upon success, FALSE upon failure.
	return(fOk);
}


/////////////////////////////////////////////////////////////


void SWMRGDelete(PSWMRG pSWMRG) 
{

	// Destroy any synchronization objects that were 
	// successfully created.
	if (NULL != pSWMRG->hMutexNoWriter)
		CloseHandle(pSWMRG->hMutexNoWriter);

	if (NULL != pSWMRG->hEventCanRead)
		CloseHandle(pSWMRG->hEventCanRead);

	if (NULL != pSWMRG->hEventNoReaders)
		CloseHandle(pSWMRG->hEventNoReaders);

	if (NULL != pSWMRG->hSemNumReaders)
		CloseHandle(pSWMRG->hSemNumReaders);
}



















/////////////////////////////////////////////////////////////


DWORD SWMRGWaitToWrite(PSWMRG pSWMRG, DWORD dwTimeout) 
{
	DWORD dw; 
	HANDLE aHandles[2];

	// We can write if the following are true:
	// 1. The mutex guard is available and
	//    no other threads are writing.
	// 2. No threads are reading.
	aHandles[0] = pSWMRG->hMutexNoWriter;
	aHandles[1] = pSWMRG->hEventNoReaders;
	dw = WaitForMultipleObjects(2, aHandles, TRUE, dwTimeout);
	if (dw == WAIT_OBJECT_0 )//!= WAIT_TIMEOUT) 
	{
		// This thread can write to the shared data.

		// Because a writer thread is writing, the mutex 
		// should not be released. This stops other 
		// writers and readers.

		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////


void SWMRGDoneWriting(PSWMRG pSWMRG) 
{
	// Presumably, a writer thread calling this function has
	// successfully called WaitToWrite. This means that we
	// do not have to wait on any synchronization objects 
	// here because the writer already owns the mutex.

	SetEvent(pSWMRG->hEventCanRead);

	// Allow other writer/reader threads to use
	// the SWMRG synchronization object.
	ReleaseMutex(pSWMRG->hMutexNoWriter);
}


/////////////////////////////////////////////////////////////


DWORD SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout) 
{
	DWORD dw; 
	LONG lPreviousCount;
	HANDLE aHandles[2];

	// We can write if the following are true:
	// 1. The mutex guard is available and
	//    no other threads are writing.
	// 2. No threads are reading.
	aHandles[0] = pSWMRG->hMutexNoWriter;
	aHandles[1] = pSWMRG->hEventCanRead;
	dw = WaitForMultipleObjects(2, aHandles, TRUE, dwTimeout);
	if (dw == WAIT_OBJECT_0 )//!= WAIT_TIMEOUT) 
	{
		// This thread can read from the shared data.

		// Increment the number of reader threads.
		ReleaseSemaphore(pSWMRG->hSemNumReaders, 1, &lPreviousCount);
		if (lPreviousCount == 0) 
		{
			// If this is the first reader thread, 
			// set our event to reflect this.
			ResetEvent(pSWMRG->hEventNoReaders);
		}

		// Allow other writer/reader threads to use
		// the SWMRG synchronization object.
		ReleaseMutex(pSWMRG->hMutexNoWriter);

		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////


void SWMRGDoneReading (PSWMRG pSWMRG) 
{
	HANDLE aHandles[2];
	LONG lNumReaders;

	// We can stop reading if the mutex guard is available,
	// but when we stop reading we must also decrement the
	// number of reader threads.
	aHandles[0] = pSWMRG->hMutexNoWriter;
	aHandles[1] = pSWMRG->hSemNumReaders;
	DWORD dw = WaitForMultipleObjects(2, aHandles, TRUE, INFINITE);

	// Get the remaining number of readers by releasing the
	// semaphore and then restoring the count by immediately
	// performing a wait.
	ReleaseSemaphore(pSWMRG->hSemNumReaders, 1,	&lNumReaders);
	WaitForSingleObject(pSWMRG->hSemNumReaders, INFINITE);

	// If there are no remaining readers, 
	// set the event to relect this.
	if (lNumReaders == 0) 
	{
		// If there are no reader threads, 
		// set our event to reflect this.
		SetEvent(pSWMRG->hEventNoReaders);
		ResetEvent( pSWMRG->hEventCanRead );
	}

	// Allow other writer/reader threads to use
	// the SWMRG synchronization object.
	ReleaseMutex(pSWMRG->hMutexNoWriter);
}


/////////////////////////////////////////////////////////////



//////////////////////// End Of File ////////////////////////
