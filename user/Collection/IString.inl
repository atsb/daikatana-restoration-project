
// IString
inline IStringData* IString::GetData() const
	{ _ASSERTE(m_pchData != NULL); return ((IStringData*)m_pchData)-1; }
inline void IString::Init()
	{ m_pchData = ionEmptyString.m_pchData; }
inline IString::IString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const IString& IString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
#ifdef _UNICODE
inline const IString& IString::operator+=(char ch)
	{ *this += (char)ch; return *this; }
inline const IString& IString::operator=(char ch)
	{ *this = (char)ch; return *this; }
inline IString AFXAPI operator+(const IString& string, char ch)
	{ return string + (char)ch; }
inline IString AFXAPI operator+(char ch, const IString& string)
	{ return (char)ch + string; }
#endif

inline int IString::GetLength() const
	{ return GetData()->nDataLength; }
inline int IString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline bool IString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline IString::operator LPCTSTR() const
	{ return m_pchData; }
inline int PASCAL IString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : strlen(lpsz); }

// IString support (windows specific)
inline int IString::Compare(LPCTSTR lpsz) const
	{ return strcmp(m_pchData, lpsz); }    // MBCS/Unicode aware
inline int IString::CompareNoCase(LPCTSTR lpsz) const
	{ return stricmp (m_pchData, lpsz); }   // MBCS/Unicode aware
// IString::Collate is often slower than Compare but is MBSC/Unicode
//  aware as well as locale-sensitive with respect to sort order.
//inline int IString::Collate(LPCTSTR lpsz) const
//	{ return _tcscoll(m_pchData, lpsz); }   // locale sensitive

inline char IString::GetAt(int nIndex) const
{
	_ASSERTE(nIndex >= 0);
	_ASSERTE(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline char IString::operator[](int nIndex) const
{
	// same as GetAt
	_ASSERTE(nIndex >= 0);
	_ASSERTE(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline bool AFXAPI operator==(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool AFXAPI operator==(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool AFXAPI operator==(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool AFXAPI operator!=(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool AFXAPI operator!=(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool AFXAPI operator!=(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool AFXAPI operator<(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool AFXAPI operator<(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
inline bool AFXAPI operator<(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool AFXAPI operator>(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool AFXAPI operator>(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
inline bool AFXAPI operator>(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool AFXAPI operator<=(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool AFXAPI operator<=(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline bool AFXAPI operator<=(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool AFXAPI operator>=(const IString& s1, const IString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool AFXAPI operator>=(const IString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline bool AFXAPI operator>=(LPCTSTR s1, const IString& s2)
	{ return s2.Compare(s1) <= 0; }
