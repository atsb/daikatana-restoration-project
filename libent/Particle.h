// Particle.h: interface for the CParticle class.
//
//	LibEnt particle system
//
//	Particles are allocated in one contiguous array, and are managed
//	through the particle's FX manager CParticleFX_ classes, and the
//	ClientEntityManager itself.
//
//	When a particle is allocated, the FXManager is set, so that
//	the particle's "Physics" can run each frame. Whenever a frame
//	is run, the particle's effects manager is passed a pointer to
//	this particle and physics are applied.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CParticle  
{
public:
	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	class CParticleFX * FXManager;
	int speed;
	void Reset();
	CVector color_rgb;      // only used by complex particles
	short flags;            // only used by complex particles
	int rotation_vector;    // only used by complex particles
	float scale;            // only used by complex particles
	char active;
	int type;
	CVector acceleration;
	float die_time;
	float alphavel;
	float alpha;
	float time;
	CVector velocity;
	CVector origin;
	CParticle();
	virtual ~CParticle();
};

#endif // !defined(AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_)
