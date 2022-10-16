
#include <stdio.h>
#include "memmgrcommon.h"
#include "filebuffer.h"

CFileBuffer::CFileBuffer( int nNum )
{
	nPos = 0;
	nCounter = 0;
	nNumChars = nNum;
	pBuffer = (char*)memmgr.X_Malloc(sizeof(char)*nNum,MEM_TAG_FILE);
}

CFileBuffer::~CFileBuffer()
{
	memmgr.X_Free(pBuffer);
	pBuffer = NULL;
}

// get next line
char *CFileBuffer::FGetLine( char *string, int n )
{
	int nStringCounter = 0;
	int nCounter = nPos;
	while ( pBuffer[nCounter] != '\n' && pBuffer[nCounter] != '\r' && 
			pBuffer[nCounter] != EOF && nPos < nNumChars-1 )
	{
		string[nStringCounter++] = pBuffer[nCounter];
		nCounter++;
	}
	
	if ( nStringCounter == 0 )
	{
		return NULL;
	}

	// get rest of the line including line feed
	while ( pBuffer[nCounter] == ' ' || pBuffer[nCounter] == '\n' || pBuffer[nCounter] == '\r' )
	{
		string[nStringCounter++] = pBuffer[nCounter];
		nCounter++;
	}
	string[nStringCounter] = '\0';

	nPos = nCounter;

	return string;
}

CFileBuffer *FILEBUFFER_Delete( CFileBuffer *pFileBuffer )
{
	delete pFileBuffer;
	// SCG[2/16/00]: memmgr.X_Free(pFileBuffer);

	return NULL;
}
