#include "collect.h"

BOOL IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite )
{
//unix - Win32 pointer validation
#if _MSC_VER
	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
#else
	return (lp != NULL);
#endif
}

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	_ASSERTE(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*)memmgr.X_Malloc( sizeof(CPlex) + nMax * cbElement, MEM_TAG_MISC );
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
};

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		memmgr.X_Free( bytes );
		p = pNext;
	}
};

