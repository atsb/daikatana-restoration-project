#ifndef _ISTRING_H
#define _ISTRING_H

//#include <tchar.h>
#include <assert.h>
#include <stdarg.h>
#include "Col_MFCDefs.h"

// NOTE: Bringing in new collections:
//
//  replace #include <stdafx.h> with
//     #include <assert.h>
//     #include <string.h>
//     #include <memory.h>
//     #include "IString.h"
//     #include "ICollectionClasses.h"
//     #include "MFCDefs.h"
//     
//     Replace BOOL with bool
//     Replace TRUE with true, FALSE with false
//     Replace CString with IString
//     Remove ::Dump()
//     Remove IMPLEMENT_DYNAMIC(xxx
//     Remove any CObject

/////////////////////////////////////////////////////////////////////////////
// Strings

struct IStringData
{
	long nRefs;     // reference count
	int nDataLength;
	int nAllocLength;
	// char data[nAllocLength]

	char* data()
		{ return (char*)(this+1); }
};


class IString
{
public:
// Constructors
	IString();
	IString(const IString& stringSrc);
	IString(char ch, int nRepeat = 1);
	IString(LPCSTR lpsz);
	IString(LPCTSTR lpch, int nLength);
	IString(const unsigned char* psz);
//	IString(LPCWSTR lpsz);

// Attributes & Operations
	// as an array of characters
	int GetLength() const;
	bool IsEmpty() const;
	void Empty();                       // free up the data

	char GetAt(int nIndex) const;      // 0 based
	char operator[](int nIndex) const; // same as GetAt
	void SetAt(int nIndex, char ch);
	operator LPCTSTR() const;           // as a C string

	// overloaded assignment
	const IString& operator=(const IString& stringSrc);
	const IString& operator=(char ch);
#ifdef _UNICODE
	const IString& operator=(char ch);
#endif
	const IString& operator=(LPCSTR lpsz);
//*	const IString& operator=(LPCWSTR lpsz);
	const IString& operator=(const unsigned char* psz);

	// string concatenation
	const IString& operator+=(const IString& string);
	const IString& operator+=(char ch);
#ifdef _UNICODE
	const IString& operator+=(char ch);
#endif
	const IString& operator+=(LPCTSTR lpsz);

	friend IString AFXAPI operator+(const IString& string1,
			const IString& string2);
	friend IString AFXAPI operator+(const IString& string, char ch);
	friend IString AFXAPI operator+(char ch, const IString& string);
#ifdef _UNICODE
	friend IString AFXAPI operator+(const IString& string, char ch);
	friend IString AFXAPI operator+(char ch, const IString& string);
#endif
	friend IString AFXAPI operator+(const IString& string, LPCTSTR lpsz);
	friend IString AFXAPI operator+(LPCTSTR lpsz, const IString& string);

	// string comparison
	int Compare(LPCTSTR lpsz) const;         // straight character
	int CompareNoCase(LPCTSTR lpsz) const;   // ignore case
	int Collate(LPCTSTR lpsz) const;         // NLS aware

	// simple sub-string extraction
	IString Mid(int nFirst, int nCount) const;
	IString Mid(int nFirst) const;
	IString Left(int nCount) const;
	IString Right(int nCount) const;

	IString SpanIncluding(LPCTSTR lpszCharSet) const;
	IString SpanExcluding(LPCTSTR lpszCharSet) const;

	// upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	// trimming whitespace (either side)
	void TrimRight();
	void TrimLeft();

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(char ch) const;               // like "C" strchr
	int ReverseFind(char ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	// look for a specific sub-string
	int Find(LPCTSTR lpszSub) const;        // like "C" strstr

	// simple formatting
	void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
	void AFX_CDECL Format(UINT nFormatID, ...);

//*#ifndef _MAC
//*	// formatting for localization (uses FormatMessage API)
//*	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
//*	void AFX_CDECL FormatMessage(UINT nFormatID, ...);
//*#endif
//*
//*	// input and output
//*#ifdef _DEBUG
//*	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
//*				const IString& string);
//*#endif
//*	friend CArchive& AFXAPI operator<<(CArchive& ar, const IString& string);
//*	friend CArchive& AFXAPI operator>>(CArchive& ar, IString& string);

	// Windows support
	bool LoadString(UINT nID);          // load from string resource
										// 255 chars max
#ifndef _UNICODE
	// ANSI <-> OEM support (convert string in place)
	void AnsiToOem();
	void OemToAnsi();
#endif

//*#ifndef _AFX_NO_BSTR_SUPPORT
//*	// OLE BSTR support (use for OLE automation)
//*	BSTR AllocSysString() const;
//*	BSTR SetSysString(BSTR* pbstr) const;
//*#endif

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	// Use LockBuffer/UnlockBuffer to turn refcounting off
	LPTSTR LockBuffer();
	void UnlockBuffer();

// Implementation
public:
	~IString();
	int GetAllocLength() const;

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data

	// implementation helpers
	IStringData* GetData() const;
	void Init();
	void AllocCopy(IString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(IStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
};

// Compare helpers
bool AFXAPI operator==(const IString& s1, const IString& s2);
bool AFXAPI operator==(const IString& s1, LPCTSTR s2);
bool AFXAPI operator==(LPCTSTR s1, const IString& s2);
bool AFXAPI operator!=(const IString& s1, const IString& s2);
bool AFXAPI operator!=(const IString& s1, LPCTSTR s2);
bool AFXAPI operator!=(LPCTSTR s1, const IString& s2);
bool AFXAPI operator<(const IString& s1, const IString& s2);
bool AFXAPI operator<(const IString& s1, LPCTSTR s2);
bool AFXAPI operator<(LPCTSTR s1, const IString& s2);
bool AFXAPI operator>(const IString& s1, const IString& s2);
bool AFXAPI operator>(const IString& s1, LPCTSTR s2);
bool AFXAPI operator>(LPCTSTR s1, const IString& s2);
bool AFXAPI operator<=(const IString& s1, const IString& s2);
bool AFXAPI operator<=(const IString& s1, LPCTSTR s2);
bool AFXAPI operator<=(LPCTSTR s1, const IString& s2);
bool AFXAPI operator>=(const IString& s1, const IString& s2);
bool AFXAPI operator>=(const IString& s1, LPCTSTR s2);
bool AFXAPI operator>=(LPCTSTR s1, const IString& s2);

//*// conversion helpers
//*int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
//*int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

// Globals
extern AFX_DATA char ionChNil;
const IString&  IONGetEmptyString();
#define ionEmptyString IONGetEmptyString()

#include <crtdbg.h>
#include "IString.inl"

#endif // _IString_H
