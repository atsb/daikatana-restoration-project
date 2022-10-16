
#ifndef _ICOLLECTION_CLASSES_H
#define _ICOLLECTION_CLASSES_H

#include "IString.h"

#define IBEFORE_START_POSITION ((IPOSITION)-1L)

// abstract iteration position
struct __IPOSITION { };
typedef __IPOSITION* IPOSITION;


class IPtrArray
{
public:
// Construction
	IPtrArray();

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
	int Append(const IPtrArray& src);
	void Copy(const IPtrArray& src);

	// overloaded operator helpers
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, IPtrArray* pNewArray);

// Implementation
protected:
	void** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~IPtrArray();
#ifdef _DEBUG
//	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};




class IPtrList
{
protected:
	struct INode
	{
		INode* pNext;
		INode* pPrev;
		void* data;
	};
public:
// Construction
	IPtrList(int nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	bool IsEmpty() const;

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
	IPOSITION AddHead(void* newElement);
	IPOSITION AddTail(void* newElement);

	// add another list of elements before head or after tail
	void AddHead(IPtrList* pNewList);
	void AddTail(IPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	IPOSITION GetHeadPosition() const;
	IPOSITION GetTailPosition() const;
	void*& GetNext(IPOSITION& rPosition); // return *Position++
	void* GetNext(IPOSITION& rPosition) const; // return *Position++
	void*& GetPrev(IPOSITION& rPosition); // return *Position--
	void* GetPrev(IPOSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(IPOSITION position);
	void* GetAt(IPOSITION position) const;
	void SetAt(IPOSITION pos, void* newElement);
	void RemoveAt(IPOSITION position);

	// inserting before or after a given position
	IPOSITION InsertBefore(IPOSITION position, void* newElement);
	IPOSITION InsertAfter(IPOSITION position, void* newElement);

	// helper functions (note: O(n) speed)
	IPOSITION Find(void* searchValue, IPOSITION startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	IPOSITION FindIndex(int nIndex) const;
						// get the 'nIndex'th element (may return NULL)

    void Remove (void* pRemoveMe)      
    {
        IPOSITION pos = Find (pRemoveMe) ;
        if (pos) RemoveAt (pos) ;
    }

// Implementation
protected:
	INode* m_pNodeHead;
	INode* m_pNodeTail;
	int m_nCount;
	INode* m_pNodeFree;
	struct IPlex* m_pBlocks;
	int m_nBlockSize;

	INode* NewNode(INode*, INode*);
	void FreeNode(INode*);

public:
	~IPtrList();
#ifdef _DEBUG
//	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};



class IMapStringToPtr
{
protected:
	// Association
	struct IAssoc
	{
		IAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		IString key;
		void* value;
	};

public:

// Construction
	IMapStringToPtr(int nBlockSize = 10);

// Attributes
	// number of elements
	int GetCount() const;
	bool IsEmpty() const;

	// Lookup
	bool Lookup(LPCTSTR key, void*& rValue) const;
	bool LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

// Operations
	// Lookup and add if not there
	void*& operator[](LPCTSTR key);

	// add a new (key, value) pair
	void SetAt(LPCTSTR key, void* newValue);

	// removing existing (key, ?) pair
	bool RemoveKey(LPCTSTR key);
	void RemoveAll();

	// iterating all (key, value) pairs
	IPOSITION GetStartPosition() const;
	void GetNextAssoc(IPOSITION& rNextPosition, IString& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, bool bAllocNow = true);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT HashKey(LPCTSTR key) const;

// Implementation
protected:
	IAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	IAssoc* m_pFreeList;
	struct IPlex* m_pBlocks;
	int m_nBlockSize;

	IAssoc* NewAssoc();
	void FreeAssoc(IAssoc*);
	IAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~IMapStringToPtr();
#ifdef _DEBUG
//	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	// local typedefs for CTypedPtrMap class template
//	typedef IString BASE_KEY;
//	typedef LPCTSTR BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#include <crtdbg.h>
#include "CollectionClasses.inl"

#endif // _ICOLLECTION_CLASSES_H
