#ifndef _MFCDEFS_H
#define _MFCDEFS_H

#ifndef ASSERT_VALID
#  define ASSERT_VALID assert
#  define ASSERT       assert
#  define _AFXCOLL_INLINE inline
#endif

#ifndef TCHAR
#  define TCHAR char
#endif

// AFXAPI is used on global public functions
#ifndef AFXAPI
	#define AFXAPI __stdcall
#endif

// AFX_CDECL is used for rare functions taking variable arguments
#ifndef AFX_CDECL
	#define AFX_CDECL __cdecl
#endif

// AFX_EXPORT is used for functions which need to be exported
#ifndef AFX_EXPORT
	#define AFX_EXPORT EXPORT
#endif

// PASCAL is used for static member functions
#ifndef PASCAL
	#define PASCAL
#endif

#ifndef NULL
#  define NULL 0
#endif

#ifndef VERIFY
#  define inline  inline
#  define VERIFY(expr)    expr
#  define ASSERT_KINDOF(type, ptr)  (1)
#  define AfxIsValidAddress(ptr, sz)  (1)
#  define AfxIsValidString(psz, b) (1)
#endif


// The following macros are used on data declarations/definitions
//  (they are redefined for extension DLLs and the shared MFC DLL)
#define AFX_DATA
#define AFX_DATADEF

#ifndef min
#  define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

#ifndef max
#  define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif

#ifndef HIWORD
#  define HIWORD(l)   ((WORD) (((DWORD) (l) >> 16) & 0xFFFF)) 
#  define LOWORD(l)   ((WORD) (l)) 
#endif

#ifndef TRUE
#  define FALSE 0
#  define TRUE  !FALSE
#endif

typedef unsigned short WORD ;
typedef unsigned long  DWORD ;
typedef unsigned int UINT ;
typedef unsigned char BYTE ;
typedef int BOOL ;
typedef const char* LPCSTR ;
typedef const char* LPCTSTR ;
typedef char* LPTSTR ;
// -----------
typedef unsigned char byte ;

struct IPlex     // warning variable length structure
{
	IPlex* pNext;
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];    // align on 8 byte boundary
#endif
	// BYTE data[maxNum*elementSize];

	void* data() { return this+1; }

	static IPlex* PASCAL Create(IPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};

#endif // _MFCDEFS_H
