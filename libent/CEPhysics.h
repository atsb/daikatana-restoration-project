// CEPhysics.h: interface for the CPhysics class.
//
//  1999 Ion Storm LLP
//
//  CPhysics is the base class for various physics routines to be
//  applied to CClientEntity derivatives. Many derivatives are 
//  shamelessly swiped from the main engine physics sources :)
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYSICS_H__488A2126_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_PHYSICS_H__488A2126_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CProjectile;
class CTrackingEntity;
class CClientEntity;
class CPhysics  
{
public:
	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	CVector Rotate(CVector angles, CVector avelocity);
	class CVector FlyMissile(class CVector origin, class CVector velocity);
	void Track(CTrackingEntity *tracker);
	float timescale;
	void Rotate(CClientEntity *clent);
	void PostFrame();
	void PreFrame();
	unsigned long elapsed_time;
	unsigned long last_frame_time;
	unsigned long current_time;
	unsigned long SysTime();
	struct client_state_s * ClientState;
	CClientEntityManager * manager;
	int frame;
	void FlyMissile(CClientEntity *clent);
	CClientEntity * m_clientEnt;
	struct edict_s * m_entity;
	CPhysics();
	virtual ~CPhysics();

};

#endif // !defined(AFX_PHYSICS_H__488A2126_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_)
