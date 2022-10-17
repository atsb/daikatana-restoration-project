
#ifndef COLLECTION_H
#define COLLECTION_H

//#include <windows.h>
#include "dk_system.h"
#if _MSC_VER
#include <windowsx.h>
#include <crtdbg.h>
#endif
//unix - added stdlib.h, dk_std.h, string.h
#include <stdlib.h>
#include "dk_std.h"
#include <string.h>

#include "memmgrcommon.h"

class CPtrArray;
class CPtrList;
struct CPlex;

// abstract iteration position
struct __POSITION { int unused; };
typedef __POSITION* POSITION;

#ifdef _DEBUG
#define ASSERT_VALID()  (AssertValid())

#else // _DEBUG

#define ASSERT_VALID()  ((void)0)
#endif // _DEBUG

#define BEFORE_START_POSITION ((POSITION)-1L)


BOOL IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE );

////////////////////////////////////////////////////////////////////////////

struct CPlex     // warning variable length structure
{
	CPlex* pNext;
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];    // align on 8 byte boundary
#endif
	// BYTE data[maxNum*elementSize];

	void* data() { return this+1; }

	static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};


////////////////////////////////////////////////////////////////////////////

class CPtrArray
{
public:

// Construction
	CPtrArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	void* GetAt(int nIndex) const;
	void SetAt(int nIndex, void* newElement);
	void*& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const void** GetData() const;
	void** GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, void* newElement);
	int Add(void* newElement);
	int Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

	// overloaded operator helpers
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArray* pNewArray);

// Implementation
protected:
	void** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CPtrArray();
#ifdef _DEBUG
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

inline int CPtrArray::GetSize() const
	{ return m_nSize; }
inline int CPtrArray::GetUpperBound() const
	{ return m_nSize-1; }
inline void CPtrArray::RemoveAll()
	{ SetSize(0); }
inline void* CPtrArray::GetAt(int nIndex) const
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline void CPtrArray::SetAt(int nIndex, void* newElement)
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
inline void*& CPtrArray::ElementAt(int nIndex)
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline const void** CPtrArray::GetData() const
	{ return (const void**)m_pData; }
inline void** CPtrArray::GetData()
	{ return (void**)m_pData; }
inline int CPtrArray::Add(void* newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
inline void* CPtrArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
inline void*& CPtrArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


/////////////////////////////////////////////////////////////////////////////

class CPtrList
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		void* data;
	};
public:

// Construction
	CPtrList(int nBlockSize = 10);

//	void* CPtrList::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CPtrList::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CPtrList::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CPtrList::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	void* RemoveHead();
	void* RemoveTail();

	// add before head or after tail
	POSITION AddHead(void* newElement);
	POSITION AddTail(void* newElement);

	// add another list of elements before head or after tail
	void AddHead(CPtrList* pNewList);
	void AddTail(CPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	void*& GetNext(POSITION& rPosition); // return *Position++
	void* GetNext(POSITION& rPosition) const; // return *Position++
	void*& GetPrev(POSITION& rPosition); // return *Position--
	void* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(POSITION position);
	void* GetAt(POSITION position) const;
	void SetAt(POSITION pos, void* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, void* newElement);
	POSITION InsertAfter(POSITION position, void* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(void* searchValue, POSITION startAfter = NULL);
						// defaults to starting at the HEAD
						// return NULL if not found
	POSITION FindIndex(int nIndex);
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CPtrList();
#ifdef _DEBUG
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

////////////////////////////////////////////////////////////////////////////

inline int CPtrList::GetCount() const
	{ return m_nCount; }
inline BOOL CPtrList::IsEmpty() const
	{ return m_nCount == 0; }
inline void*& CPtrList::GetHead()
	{ _ASSERTE(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void* CPtrList::GetHead() const
	{ _ASSERTE(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void*& CPtrList::GetTail()
	{ _ASSERTE(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline void* CPtrList::GetTail() const
	{ _ASSERTE(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline POSITION CPtrList::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
inline POSITION CPtrList::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
inline void*& CPtrList::GetNext(POSITION& rPosition) // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void* CPtrList::GetNext(POSITION& rPosition) const // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void*& CPtrList::GetPrev(POSITION& rPosition) // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void* CPtrList::GetPrev(POSITION& rPosition) const // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void*& CPtrList::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		return pNode->data; }
inline void* CPtrList::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		return pNode->data; }
inline void CPtrList::SetAt(POSITION pos, void* newElement)
	{ CNode* pNode = (CNode*) pos;
		_ASSERTE(IsValidAddress(pNode, sizeof(CNode)));
		pNode->data = newElement; }

/////////////////////////////////////////////////////////////////////////////

class CMapStringToPtr
{

protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT	nHashValue;  // needed for efficient iteration
		char	key[32];
		void*	value;
	};

public:

// Construction
	CMapStringToPtr(int nBlockSize = 10);

//	void* CMapStringToPtr::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CMapStringToPtr::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CMapStringToPtr::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CMapStringToPtr::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void Init();

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(LPCTSTR key, void*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

// Operations
	// Lookup and add if not there
	void*& operator[](LPCTSTR key);

	// add a new (key, value) pair
	void SetAt(LPCTSTR key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, char*& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT HashKey(LPCTSTR key) const;

// Implementation
protected:
	CAssoc**	m_pHashTable;
	UINT		m_nHashTableSize;
	int			m_nCount;
	CAssoc*		m_pFreeList;
	struct CPlex* m_pBlocks;
	int			m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToPtr();
#ifdef _DEBUG
	//void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
protected:
	// local typedefs for CTypedPtrMap class template
	typedef char *BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};

////////////////////////////////////////////////////////////////////////////
__inline int CMapStringToPtr::GetCount() const
	{ return m_nCount; }
__inline BOOL CMapStringToPtr::IsEmpty() const
	{ return m_nCount == 0; }
__inline void CMapStringToPtr::SetAt(LPCTSTR key, void* newValue)
	{ (*this)[key] = newValue; }
__inline POSITION CMapStringToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
__inline UINT CMapStringToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }

/////////////////////////////////////////////////////////////////////////////

class CMapIntegerToPtr
{

protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT	nHashValue;  // needed for efficient iteration
		int		key;
		void*	value;
	};

public:

// Construction
	CMapIntegerToPtr(int nBlockSize = 10);

	void Init();

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(int key, void*& rValue) const;
	BOOL LookupKey(int key, int& rKey) const;

// Operations
	// Lookup and add if not there
	void*& operator[](int key);

	// add a new (key, value) pair
	void SetAt(int key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(int key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, int& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT HashKey(int key) const;

// Implementation
protected:
	CAssoc**	m_pHashTable;
	UINT		m_nHashTableSize;
	int			m_nCount;
	CAssoc*		m_pFreeList;
	struct CPlex* m_pBlocks;
	int			m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(int, UINT&) const;

public:
	~CMapIntegerToPtr();
#ifdef _DEBUG
	//void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
protected:
	// local typedefs for CTypedPtrMap class template
	typedef int BASE_KEY;
	typedef int BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};

////////////////////////////////////////////////////////////////////////////
__inline int CMapIntegerToPtr::GetCount() const
	{ return m_nCount; }
__inline BOOL CMapIntegerToPtr::IsEmpty() const
	{ return m_nCount == 0; }
__inline void CMapIntegerToPtr::SetAt(int key, void* newValue)
	{ (*this)[key] = newValue; }
__inline POSITION CMapIntegerToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
__inline UINT CMapIntegerToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }


/////////////////////////////////////////////////////////////////////////////
// CTypedPtrArray<BASE_CLASS, TYPE>

template<class BASE_CLASS, class TYPE>
class CTypedPtrArray : public BASE_CLASS
{
public:
	// Accessing elements
	TYPE GetAt(int nIndex) const
		{ return (TYPE)BASE_CLASS::GetAt(nIndex); }
	TYPE& ElementAt(int nIndex)
		{ return (TYPE&)BASE_CLASS::ElementAt(nIndex); }
	void SetAt(int nIndex, TYPE ptr)
		{ BASE_CLASS::SetAt(nIndex, ptr); }

	// Potentially growing the array
	void SetAtGrow(int nIndex, TYPE newElement)
	   { BASE_CLASS::SetAtGrow(nIndex, newElement); }
	int Add(TYPE newElement)
	   { return BASE_CLASS::Add(newElement); }
	int Append(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
	   { return BASE_CLASS::Append(src); }
	void Copy(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
		{ BASE_CLASS::Copy(src); }

	// Operations that move elements around
	void InsertAt(int nIndex, TYPE newElement, int nCount = 1)
		{ BASE_CLASS::InsertAt(nIndex, newElement, nCount); }
	void InsertAt(int nStartIndex, CTypedPtrArray<BASE_CLASS, TYPE>* pNewArray)
	   { BASE_CLASS::InsertAt(nStartIndex, pNewArray); }

	// overloaded operator helpers
	TYPE operator[](int nIndex) const
		{ return (TYPE)BASE_CLASS::operator[](nIndex); }
	TYPE& operator[](int nIndex)
		{ return (TYPE&)BASE_CLASS::operator[](nIndex); }
};

/////////////////////////////////////////////////////////////////////////////
// CTypedPtrList<BASE_CLASS, TYPE>

template<class BASE_CLASS, class TYPE>
class _CTypedPtrList : public BASE_CLASS
{
public:
// Construction
	_CTypedPtrList(int nBlockSize = 10)
		: BASE_CLASS(nBlockSize) { }

	// peek at head or tail
	TYPE& GetHead()
		{ return (TYPE&)BASE_CLASS::GetHead(); }
	TYPE GetHead() const
		{ return (TYPE)BASE_CLASS::GetHead(); }
	TYPE& GetTail()
		{ return (TYPE&)BASE_CLASS::GetTail(); }
	TYPE GetTail() const
		{ return (TYPE)BASE_CLASS::GetTail(); }

	// get head or tail (and remove it) - don't call on empty list!
	TYPE RemoveHead()
		{ return (TYPE)BASE_CLASS::RemoveHead(); }
	TYPE RemoveTail()
		{ return (TYPE)BASE_CLASS::RemoveTail(); }

	// iteration
	TYPE& GetNext(POSITION& rPosition)
		{ return (TYPE&)BASE_CLASS::GetNext(rPosition); }
	TYPE GetNext(POSITION& rPosition) const
		{ return (TYPE)BASE_CLASS::GetNext(rPosition); }
	TYPE& GetPrev(POSITION& rPosition)
		{ return (TYPE&)BASE_CLASS::GetPrev(rPosition); }
	TYPE GetPrev(POSITION& rPosition) const
		{ return (TYPE)BASE_CLASS::GetPrev(rPosition); }

	// getting/modifying an element at a given position
	TYPE& GetAt(POSITION position)
		{ return (TYPE&)BASE_CLASS::GetAt(position); }
	TYPE GetAt(POSITION position) const
		{ return (TYPE)BASE_CLASS::GetAt(position); }
	void SetAt(POSITION pos, TYPE newElement)
		{ BASE_CLASS::SetAt(pos, newElement); }
};

template<class BASE_CLASS, class TYPE>
class CTypedPtrList : public _CTypedPtrList<BASE_CLASS, TYPE>
{
public:
// Construction
	CTypedPtrList(int nBlockSize = 10)
		: _CTypedPtrList<BASE_CLASS, TYPE>(nBlockSize) { }

	// add before head or after tail
	POSITION AddHead(TYPE newElement)
		{ return BASE_CLASS::AddHead(newElement); }
	POSITION AddTail(TYPE newElement)
		{ return BASE_CLASS::AddTail(newElement); }

	// add another list of elements before head or after tail
	void AddHead(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
		{ BASE_CLASS::AddHead(pNewList); }
	void AddTail(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
		{ BASE_CLASS::AddTail(pNewList); }
};

#endif COLLECTION_H
