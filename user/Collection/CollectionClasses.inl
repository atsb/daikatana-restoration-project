////////////////////////////////////////////////////////////////////////////

inline int IPtrArray::GetSize() const
	{ return m_nSize; }
inline int IPtrArray::GetUpperBound() const
	{ return m_nSize-1; }
inline void IPtrArray::RemoveAll()
	{ SetSize(0); }
inline void* IPtrArray::GetAt(int nIndex) const
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline void IPtrArray::SetAt(int nIndex, void* newElement)
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
inline void*& IPtrArray::ElementAt(int nIndex)
	{ _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
inline const void** IPtrArray::GetData() const
	{ return (const void**)m_pData; }
inline void** IPtrArray::GetData()
	{ return (void**)m_pData; }
inline int IPtrArray::Add(void* newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
inline void* IPtrArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
inline void*& IPtrArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


////////////////////////////////////////////////////////////////////////////

inline int IPtrList::GetCount() const
	{ return m_nCount; }
inline bool IPtrList::IsEmpty() const
	{ return m_nCount == 0; }
inline void*& IPtrList::GetHead()
	{ _ASSERTE(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void* IPtrList::GetHead() const
	{ _ASSERTE(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void*& IPtrList::GetTail()
	{ _ASSERTE(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline void* IPtrList::GetTail() const
	{ _ASSERTE(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline IPOSITION IPtrList::GetHeadPosition() const
	{ return (IPOSITION) m_pNodeHead; }
inline IPOSITION IPtrList::GetTailPosition() const
	{ return (IPOSITION) m_pNodeTail; }
inline void*& IPtrList::GetNext(IPOSITION& rPosition) // return *Position++
	{ INode* pNode = (INode*) rPosition;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		rPosition = (IPOSITION) pNode->pNext;
		return pNode->data; }
inline void* IPtrList::GetNext(IPOSITION& rPosition) const // return *Position++
	{ INode* pNode = (INode*) rPosition;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		rPosition = (IPOSITION) pNode->pNext;
		return pNode->data; }
inline void*& IPtrList::GetPrev(IPOSITION& rPosition) // return *Position--
	{ INode* pNode = (INode*) rPosition;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		rPosition = (IPOSITION) pNode->pPrev;
		return pNode->data; }
inline void* IPtrList::GetPrev(IPOSITION& rPosition) const // return *Position--
	{ INode* pNode = (INode*) rPosition;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		rPosition = (IPOSITION) pNode->pPrev;
		return pNode->data; }
inline void*& IPtrList::GetAt(IPOSITION position)
	{ INode* pNode = (INode*) position;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		return pNode->data; }
inline void* IPtrList::GetAt(IPOSITION position) const
	{ INode* pNode = (INode*) position;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		return pNode->data; }
inline void IPtrList::SetAt(IPOSITION pos, void* newElement)
	{ INode* pNode = (INode*) pos;
		_ASSERTE(AfxIsValidAddress(pNode, sizeof(INode)));
		pNode->data = newElement; }

////////////////////////////////////////////////////////////////////////////
inline int IMapStringToPtr::GetCount() const
	{ return m_nCount; }
inline bool IMapStringToPtr::IsEmpty() const
	{ return m_nCount == 0; }
inline void IMapStringToPtr::SetAt(LPCTSTR key, void* newValue)
	{ (*this)[key] = newValue; }
inline IPOSITION IMapStringToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : IBEFORE_START_POSITION; }
inline UINT IMapStringToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }


