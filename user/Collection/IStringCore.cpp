// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include <windows.h>
#include <mbstring.h>
#include <string.h>
#include "IString.h"
#include "limits.h"

//#define InterlockedIncrement(var) ++var
//#define InterlockedDecrement(var) --var

/////////////////////////////////////////////////////////////////////////////
// static class data, special inlines

// ionChNil is left for backward compatibility
AFX_DATADEF char ionChNil = '\0';

// For an empty string, m_pchData will point here
// (note: avoids special case of checking for NULL m_pchData)
// empty string data (and locked)
static int rgInitData[] = { -1, 0, 0, 0 };
static AFX_DATADEF IStringData* afxDataNil = (IStringData*)&rgInitData;
static LPCTSTR ionPchNil = (LPCTSTR)(((BYTE*)&rgInitData)+sizeof(IStringData));
// special function to make ionEmptyString work even during initialization
const IString& IONGetEmptyString()
	{ return *(IString*)&ionPchNil; }

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

IString::IString()
{
	Init();
}

IString::IString(const IString& stringSrc)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != afxDataNil);
		m_pchData = stringSrc.m_pchData;
		GetData()->nRefs++ ;
		//InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

void IString::AllocBuffer(int nLen)
// always allocate one extra character for '\0' termination
// assumes [optimistically] that data length will equal allocation length
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= INT_MAX-1);    // max size (enough room for 1 extra)

	if (nLen == 0)
		Init();
	else
	{
		IStringData* pData =
			(IStringData*)new BYTE[sizeof(IStringData) + (nLen+1)*sizeof(char)];
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		pData->nAllocLength = nLen;
		m_pchData = pData->data();
	}
}

void IString::Release()
{
	if (GetData() != afxDataNil)
	{
		ASSERT(GetData()->nRefs != 0);
		//if (InterlockedDecrement(&GetData()->nRefs) <= 0)
		if (--GetData()->nRefs <= 0)
			delete[] (BYTE*)GetData();
		Init();
	}
}

void PASCAL IString::Release(IStringData* pData)
{
	if (pData != afxDataNil)
	{
		ASSERT(pData->nRefs != 0);
		//if (InterlockedDecrement(&pData->nRefs) <= 0)
		if (--pData->nRefs <= 0)
			delete[] (BYTE*)pData;
	}
}

void IString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &ionChNil;
	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

void IString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		IStringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(char));
	}
	ASSERT(GetData()->nRefs <= 1);
}

void IString::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
}

IString::~IString()
//  free any attached data
{
	if (GetData() != afxDataNil)
	{
//		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
		if (--GetData()->nRefs <= 0)
			::delete[] (BYTE*)GetData();
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

void IString::AllocCopy(IString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

//*IString::IString(LPCTSTR lpsz)
//*{
//*	Init();
//*	if (lpsz != NULL && HIWORD(lpsz) == NULL)
//*	{
//*		UINT nID = LOWORD((DWORD)lpsz);
//*		if (!LoadString(nID))
//*			; //TRACE1("Warning: implicit LoadString(%u) failed\n", nID);
//*	}
//*	else
//*	{
//*		int nLen = SafeStrlen(lpsz);
//*		if (nLen != 0)
//*		{
//*			AllocBuffer(nLen);
//*			memcpy(m_pchData, lpsz, nLen*sizeof(char));
//*		}
//*	}
//*}

/////////////////////////////////////////////////////////////////////////////
// Special conversion constructors

//*#ifdef _UNICODE
//*IString::IString(LPCSTR lpsz)
//*{
//*	Init();
//*	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
//*	if (nSrcLen != 0)
//*	{
//*		AllocBuffer(nSrcLen);
//*		_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
//*		ReleaseBuffer();
//*	}
//*}
//*#else //_UNICODE
//*IString::IString(LPCWSTR lpsz)
//*{
//*	Init();
//*	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
//*	if (nSrcLen != 0)
//*	{
//*		AllocBuffer(nSrcLen*2);
//*		_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
//*		ReleaseBuffer();
//*	}
//*}
//*#endif //!_UNICODE

//////////////////////////////////////////////////////////////////////////////
// Diagnostic support

//*#ifdef _DEBUG
//*CDumpContext& AFXAPI operator<<(CDumpContext& dc, const IString& string)
//*{
//*	dc << string.m_pchData;
//*	return dc;
//*}
//*#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const IString&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void IString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(char));
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const IString& IString::operator=(const IString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != afxDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			// actual copy necessary since one of the strings is locked
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			// can just copy references around
			Release();
			ASSERT(stringSrc.GetData() != afxDataNil);
			m_pchData = stringSrc.m_pchData;
			//InterlockedIncrement(&GetData()->nRefs);
			GetData()->nRefs++ ;
		}
	}
	return *this;
}

const IString& IString::operator=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz, FALSE));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Special conversion assignment

//*#ifdef _UNICODE
//*const IString& IString::operator=(LPCSTR lpsz)
//*{
//*	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
//*	AllocBeforeWrite(nSrcLen);
//*	_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
//*	ReleaseBuffer();
//*	return *this;
//*}
//*#else //!_UNICODE
//*const IString& IString::operator=(LPCWSTR lpsz)
//*{
//*	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
//*	AllocBeforeWrite(nSrcLen*2);
//*	_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
//*	ReleaseBuffer();
//*	return *this;
//*}
//*#endif  //!_UNICODE

//////////////////////////////////////////////////////////////////////////////
// concatenation

// NOTE: "operator+" is done as friend functions for simplicity
//      There are three variants:
//          IString + IString
// and for ? = char, LPCTSTR
//          IString + ?
//          ? + IString

void IString::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
	int nSrc2Len, LPCTSTR lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new IString object

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(char));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(char));
	}
}

IString AFXAPI operator+(const IString& string1, const IString& string2)
{
	IString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

IString AFXAPI operator+(const IString& string, LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz, FALSE));
	IString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		IString::SafeStrlen(lpsz), lpsz);
	return s;
}

IString AFXAPI operator+(LPCTSTR lpsz, const IString& string)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz, FALSE));
	IString s;
	s.ConcatCopy(IString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// concatenate in place

void IString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	//  -- the main routine for += operators

	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)
		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		// we have to grow the buffer, use the ConcatCopy routine
		IStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		ASSERT(pOldData != NULL);
		IString::Release(pOldData);
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(char));
		GetData()->nDataLength += nSrcLen;
		ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

const IString& IString::operator+=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz, FALSE));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const IString& IString::operator+=(char ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const IString& IString::operator+=(const IString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

LPTSTR IString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		// we have to grow the buffer
		IStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(char));
		GetData()->nDataLength = nOldLen;
		IString::Release(pOldData);
	}
	ASSERT(GetData()->nRefs <= 1);

	// return a pointer to the character storage for this string
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

void IString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();  // just in case GetBuffer was not called

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData); // zero terminated

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR IString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void IString::FreeExtra()
{
	ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		IStringData* pOldData = GetData();
		AllocBuffer(GetData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(char));
		ASSERT(m_pchData[GetData()->nDataLength] == '\0');
		IString::Release(pOldData);
	}
	ASSERT(GetData() != NULL);
}

LPTSTR IString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

void IString::UnlockBuffer()
{
	ASSERT(GetData()->nRefs == -1);
	if (GetData() != afxDataNil)
		GetData()->nRefs = 1;
}

///////////////////////////////////////////////////////////////////////////////
// Commonly used routines (rarely used routines in STREX.CPP)

int IString::Find(char ch) const
{
	// find first single character
	LPTSTR lpsz = strchr (m_pchData, (char)ch);

	// return -1 if not found and index otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int IString::FindOneOf(LPCTSTR lpszCharSet) const
{
	ASSERT(AfxIsValidString(lpszCharSet, FALSE));
	LPTSTR lpsz = strpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void IString::MakeUpper()
{
	CopyBeforeWrite();
	strupr (m_pchData);
}

void IString::MakeLower()
{
	CopyBeforeWrite();
	strlwr (m_pchData);
}

void IString::MakeReverse()
{
	CopyBeforeWrite();
	strrev (m_pchData);
}

void IString::SetAt(int nIndex, char ch)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

#ifndef _UNICODE
void IString::AnsiToOem()
{
	CopyBeforeWrite();
	::AnsiToOem(m_pchData, m_pchData);
}
void IString::OemToAnsi()
{
	CopyBeforeWrite();
	::OemToAnsi(m_pchData, m_pchData);
}
#endif

#if 0 // __AFX_H__
///////////////////////////////////////////////////////////////////////////////
// IString conversion helpers (these use the current system locale)

int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= (int)count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, count);
	ASSERT(wcstr == NULL || result <= (int)count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}

LPWSTR AFXAPI AfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	if (lpa == NULL)
		return NULL;
	ASSERT(lpw != NULL);
	// verify that no illegal character present
	// since lpw was allocated based on the size of lpa
	// don't worry about the number of chars
	lpw[0] = '\0';
	VERIFY(MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars));
	return lpw;
}

LPSTR AFXAPI AfxW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	if (lpw == NULL)
		return NULL;
	ASSERT(lpa != NULL);
	// verify that no illegal character present
	// since lpa was allocated based on the size of lpw
	// don't worry about the number of chars
	lpa[0] = '\0';
	VERIFY(WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL));
	return lpa;
}

#endif // __AFX_H__

///////////////////////////////////////////////////////////////////////////////