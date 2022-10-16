#ifndef _CSV_H
#define _CSV_H

#include <stdio.h>
#include "filebuffer.h"


#define EOLN	(-2)

class CCSVFile
{
public:
	CCSVFile();
	~CCSVFile();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
	void Init();
	char *GetLine( char *line, int n );
	int OpenFile( const char *szFileName );
	void CloseFile();
	void SeekToBeginning();

	int GetNextLine( char *szLine );
	int GetFirstElement( const char *szLine, char *szElement );
	int GetNextElement( const char *szLine, char *szElement );

private:

	int bUsingFile;
	CFileBuffer *pFileBuffer;
	FILE *file;

	int nLinePosition;
	//char szLine[2048];
};

int CSV_OpenFile( const char *szFileName, CCSVFile **pCsvFile );
void CSV_CloseFile( CCSVFile *pCsvFile );
void CSV_SeekToBeginning( CCSVFile *pCsvFile );
int CSV_GetNextLine( CCSVFile *pCsvFile, char *szLine );
int CSV_GetFirstElement( CCSVFile *pCsvFile, const char *szLine, char *szElement );
int CSV_GetNextElement( CCSVFile *pCsvFile, const char *szLine, char *szElement );

#endif _CSV_H
