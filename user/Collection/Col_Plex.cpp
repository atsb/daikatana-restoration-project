// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include <assert.h>
#include "Col_MFCDefs.h"

// Collection support
#ifdef AFX_COLL_SEG
#pragma code_seg(AFX_COLL_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// IPlex

IPlex* PASCAL IPlex::Create(IPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	IPlex* p = (IPlex*) new BYTE[sizeof(IPlex) + nMax * cbElement];
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void IPlex::FreeDataChain()     // free this one and links
{
	IPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		IPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}
