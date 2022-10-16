
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map from char* to value
//
/////////////////////////////////////////////////////////////////////////////

#include "collect.h"

//#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CMapIntegerToPtr::CMapIntegerToPtr(int nBlockSize)
{
	_ASSERTE(nBlockSize > 0);

	m_pHashTable		= NULL;
	m_nHashTableSize	= 17;  // default size
	m_nCount			= 0;
	m_pFreeList			= NULL;
	m_pBlocks			= NULL;
	m_nBlockSize		= nBlockSize;
}

void CMapIntegerToPtr::Init()
{
	m_pHashTable		= NULL;
	m_nHashTableSize	= 17;  // default size
	m_nCount			= 0;
	m_pFreeList			= NULL;
	m_pBlocks			= NULL;
	m_nBlockSize		= 10;
}

inline UINT CMapIntegerToPtr::HashKey(int key) const
{
	return ((UINT)(void*)(DWORD)key) >> 4;
}

void CMapIntegerToPtr::InitHashTable( UINT nHashSize, BOOL bAllocNow )
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
	ASSERT_VALID();
	_ASSERTE(m_nCount == 0);
	_ASSERTE(nHashSize > 0);

	if (m_pHashTable != NULL)
	{
		// free hash table
		free( m_pHashTable );
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = (CAssoc**)memmgr.X_Malloc( sizeof(CAssoc*) * nHashSize, MEM_TAG_MISC );
		//memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapIntegerToPtr::RemoveAll()
{
	ASSERT_VALID();

	if (m_pHashTable != NULL)
	{
		// free hash table
		memmgr.X_Free( m_pHashTable );
		m_pHashTable = NULL;
	}

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CMapIntegerToPtr::~CMapIntegerToPtr()
{
	RemoveAll();
	_ASSERTE(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapIntegerToPtr::CAssoc* CMapIntegerToPtr::NewAssoc()
{
	if (m_pFreeList == NULL)
	{
		// add another block
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapIntegerToPtr::CAssoc));
		// chain them into free list
		CMapIntegerToPtr::CAssoc* pAssoc = (CMapIntegerToPtr::CAssoc*) newBlock->data();
		// free in reverse order to make it easier to debug
		pAssoc += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	_ASSERTE(m_pFreeList != NULL);  // we must have something

	CMapIntegerToPtr::CAssoc* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	_ASSERTE(m_nCount > 0);  // make sure we don't overflow

	pAssoc->key = 0;
	pAssoc->value = 0;

	return pAssoc;
}

void CMapIntegerToPtr::FreeAssoc(CMapIntegerToPtr::CAssoc* pAssoc)
{
	//DestructElement(&pAssoc->key);  // free up string data

	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	_ASSERTE(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

CMapIntegerToPtr::CAssoc* CMapIntegerToPtr::GetAssocAt(int key, UINT& nHash) const
// find association (or return NULL)
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			return pAssoc;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMapIntegerToPtr::Lookup(int key, void*& rValue) const
{
	ASSERT_VALID();

	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rValue = pAssoc->value;
	return TRUE;
}

BOOL CMapIntegerToPtr::LookupKey(int key, int& rKey) const
{
	ASSERT_VALID();

	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rKey = pAssoc->key;
	return TRUE;
}

void*& CMapIntegerToPtr::operator[](int key)
{
	ASSERT_VALID();

	UINT nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = NewAssoc();
		pAssoc->nHashValue = nHash;
		pAssoc->key = key;
		// 'pAssoc->value' is a constructed object, nothing more

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;
	}
	return pAssoc->value;  // return new reference
}


BOOL CMapIntegerToPtr::RemoveKey(int key)
// remove key - return TRUE if removed
{
	ASSERT_VALID();

	if (m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapIntegerToPtr::GetNextAssoc(POSITION& rNextPosition, int &rKey, void*& rValue) const
{
	ASSERT_VALID();
	_ASSERTE(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)rNextPosition;
	_ASSERTE(pAssocRet != NULL);

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		{
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
		}
		_ASSERTE(pAssocRet != NULL);  // must find something
	}

	// find next association
	_ASSERTE(IsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (HashKey(pAssocRet->key) % m_nHashTableSize) + 1; nBucket < m_nHashTableSize; nBucket++)
		{
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
		}
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void CMapIntegerToPtr::AssertValid() const
{
	_ASSERTE(m_nHashTableSize > 0);
	_ASSERTE(m_nCount == 0 || m_pHashTable != NULL);
		// non-empty map should have hash table
}
#endif //_DEBUG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif


/////////////////////////////////////////////////////////////////////////////
