// ParticleFX.h: interface for the CParticleFX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLEFX_H__D7AC68E1_AFA9_11D2_9AF1_00104B659729__INCLUDED_)
#define AFX_PARTICLEFX_H__D7AC68E1_AFA9_11D2_9AF1_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ClientEntity.h"

class CParticleFX : public CClientEntity  
{
public:
	class CParticle * particle_template;
	virtual void ResetParticles();
    virtual void SpawnParticles();
    void SpawnParticles(CVector &start, CVector &end);
	class CClientEntity * ent_sibling;
	void ReInitialize(CClientEntity *clen);
	void MakeNormalVecs (CVector &forward, CVector &right, CVector &up);
	virtual void CL_RunFrame();
	class CClientEntityManager * manager;
//	class CParticle * active_particle;
	class CParticle * free_particle;
	int particle_list_size;
	virtual class CParticle* SpawnParticle();
	virtual void Physics();
	class CParticle * particle_list;
	CParticleFX();
	virtual ~CParticleFX();

};

#endif // !defined(AFX_PARTICLEFX_H__D7AC68E1_AFA9_11D2_9AF1_00104B659729__INCLUDED_)
