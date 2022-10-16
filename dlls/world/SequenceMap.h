
#ifndef _SEQUENCEMAP_H
#define _SEQUENCEMAP_H

class CSequenceList
{
private:
	int nNumSequences;
	frameData_t *pSequences;

public:
	CSequenceList();
	CSequenceList( int nNumNum );
	~CSequenceList();

//	void* CSequenceList::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CSequenceList::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CSequenceList::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CSequenceList::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	frameData_t *AllocateSequences( int nNewNum );
	void DeleteSequences();

	int GetNumSequences()					{ return nNumSequences; }
	frameData_t *GetSequences()				{ return pSequences; }
	frameData_t *GetSequence( int nIndex )	{ return &(pSequences[nIndex]); }
	frameData_t *GetSequence( const char *animName );

	void MapAnimationNameToSequences( userEntity_t *self );

	void Verify();
};

CSequenceList *SEQUENCEMAP_Lookup( int nModelIndex );
void SEQUENCEMAP_Add( int nModelIndex, CSequenceList *pSequenceList );
void SEQUENCEMAP_RemoveAll();
void SEQUENCEMAP_AllocSequenceMap( userEntity_t *self );

#endif _SEQUENCEMAP_H