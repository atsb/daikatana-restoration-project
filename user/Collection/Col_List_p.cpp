
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
// Implementation of parameterized List
//
/////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <string.h>
#include <memory.h>
#include "IString.h"
#include "CollectionClasses.h"

/////////////////////////////////////////////////////////////////////////////

IPtrList::IPtrList(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void IPtrList::RemoveAll()
{
	ASSERT_VALID(this);

	// destroy elements


	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

IPtrList::~IPtrList()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
/*
 * Implementation note: INode's are stored in IPlex blocks and
 *  chained together. Free blocks are maintained in a singly linked list
 *  using the 'pNext' member of INode with 'm_pNodeFree' as the head.
 *  Used blocks are maintained in a doubly linked list using both 'pNext'
 *  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
 *   as the head/tail.
 *
 * We never free a IPlex block unless the List is destroyed or RemoveAll()
 *  is used - so the total number of IPlex blocks may grow large depending
 *  on the maximum past size of the list.
 */

IPtrList::INode*
IPtrList::NewNode(IPtrList::INode* pPrev, IPtrList::INode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		IPlex* pNewBlock = IPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(INode));

		// chain them into free list
		INode* pNode = (INode*) pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  // we must have something

	IPtrList::INode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow


	pNode->data = 0; // start with zero

	return pNode;
}

void IPtrList::FreeNode(IPtrList::INode* pNode)
{

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

IPOSITION IPtrList::AddHead(void* newElement)
{
	ASSERT_VALID(this);

	INode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (IPOSITION) pNewNode;
}

IPOSITION IPtrList::AddTail(void* newElement)
{
	ASSERT_VALID(this);

	INode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (IPOSITION) pNewNode;
}

void IPtrList::AddHead(IPtrList* pNewList)
{
	ASSERT_VALID(this);

	ASSERT(pNewList != NULL);
	ASSERT_KINDOF(IPtrList, pNewList);
	ASSERT_VALID(pNewList);

	// add a list of same elements to head (maintain order)
	IPOSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void IPtrList::AddTail(IPtrList* pNewList)
{
	ASSERT_VALID(this);
	ASSERT(pNewList != NULL);
	ASSERT_KINDOF(IPtrList, pNewList);
	ASSERT_VALID(pNewList);

	// add a list of same elements
	IPOSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

void* IPtrList::RemoveHead()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!
	ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(INode)));

	INode* pOldNode = m_pNodeHead;
	void* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

void* IPtrList::RemoveTail()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeTail != NULL);  // don't call on empty list !!!
	ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(INode)));

	INode* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

IPOSITION IPtrList::InsertBefore(IPOSITION position, void* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	// Insert it before position
	INode* pOldNode = (INode*) position;
	INode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(INode)));
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (IPOSITION) pNewNode;
}

IPOSITION IPtrList::InsertAfter(IPOSITION position, void* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	INode* pOldNode = (INode*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(INode)));
	INode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(INode)));
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (IPOSITION) pNewNode;
}

void IPtrList::RemoveAt(IPOSITION position)
{
	ASSERT_VALID(this);

	INode* pOldNode = (INode*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(INode)));

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(INode)));
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(INode)));
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}


/////////////////////////////////////////////////////////////////////////////
// slow operations

IPOSITION IPtrList::FindIndex(int nIndex) const
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nCount)
		return NULL;  // went too far

	INode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(INode)));
		pNode = pNode->pNext;
	}
	return (IPOSITION) pNode;
}

IPOSITION IPtrList::Find(void* searchValue, IPOSITION startAfter) const
{
	ASSERT_VALID(this);

	INode* pNode = (INode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(INode)));
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (IPOSITION) pNode;
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
//*void IPtrList::Dump(CDumpContext& dc) const
//*{
//*	CObject::Dump(dc);
//*
//*	dc << "with " << m_nCount << " elements";
//*	if (dc.GetDepth() > 0)
//*	{
//*		IPOSITION pos = GetHeadPosition();
//*		while (pos != NULL)
//*			dc << "\n\t" << GetNext(pos);
//*	}
//*
//*	dc << "\n";
//*}

void IPtrList::AssertValid() const
{
	if (m_nCount == 0)
	{
		// empty list
		ASSERT(m_pNodeHead == NULL);
		ASSERT(m_pNodeTail == NULL);
	}
	else
	{
		// non-empty list
		ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(INode)));
		ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(INode)));
	}
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
