// Sound.h: interface for the CSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUND_H__22710603_B0B4_11D2_A7E2_0000C087A6E9__INCLUDED_)
#define AFX_SOUND_H__22710603_B0B4_11D2_A7E2_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSound  
{
public:
	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void Play(CVector & origin);
	void Play();
	struct sfx_s * RegisterSound(const char *name);
	struct sfx_s * sfx;
	 CSound(class CClientEntity *clent, char *name);
	char * sound_name;
	class CClientEntity * ent_sibling;
	CSound();
	virtual ~CSound();

};

#endif // !defined(AFX_SOUND_H__22710603_B0B4_11D2_A7E2_0000C087A6E9__INCLUDED_)
