#ifndef _BUFFER_H
#define _BUFFER_H

#ifndef NULL
#define NULL (0L)
#endif

class CFileBuffer
{
public:
	CFileBuffer()
	{
		nPos = 0;
		nCounter = 0;
		nNumChars = 0;
		pBuffer = NULL;
	};
	CFileBuffer( int nNum );
	~CFileBuffer();

	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void Init()
	{
		nPos = 0;
		nCounter = 0;
	}
	int GetNumChars()	{ return nNumChars; }
	char *GetBuffer()	{ return pBuffer; }

	void SetChar( char c, int nIndex )
	{
		pBuffer[nIndex] = c;
	}
	void PutC( char c )
	{
		pBuffer[nCounter++] = c;
	}
	char GetC( int nIndex )		{ return pBuffer[nIndex]; }

	char *FGetLine( char *string, int n );

private:
	int nPos;
	int nCounter;
	int	nNumChars;
	char *pBuffer;
};

CFileBuffer *FILEBUFFER_Delete( CFileBuffer *pFileBuffer );

#endif _BUFFER_H
