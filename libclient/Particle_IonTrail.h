// Particle_IonTrail.h: interface for the CParticle_IonTrail class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLE_IONTRAIL_H__17632501_AF00_11D2_9AF1_00104B659729__INCLUDED_)
#define AFX_PARTICLE_IONTRAIL_H__17632501_AF00_11D2_9AF1_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ParticleFX.h"

class CParticleFX_IonTrail : public CParticleFX 
{
public:
	void SpawnParticles();
	void SpawnParticles(CVector &start, CVector &end);
	CParticleFX_IonTrail();
	virtual ~CParticleFX_IonTrail();
};

#endif // !defined(AFX_PARTICLE_IONTRAIL_H__17632501_AF00_11D2_9AF1_00104B659729__INCLUDED_)
