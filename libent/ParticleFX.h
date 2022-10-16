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
	void FreeParticle(class CParticle *p);
	class CParticle * particle_template;
    virtual void SpawnParticles();
    void SpawnParticles(CVector &start, CVector &end);
	class CClientEntity * ent_sibling;
	void ReInitialize(CClientEntity *clen);
	void MakeNormalVecs (CVector &forward, CVector &right, CVector &up);
	virtual void CL_RunFrame();
	class CClientEntityManager * manager;
	virtual class CParticle* SpawnParticle(class CParticleFX *owner);
	virtual void Physics(class CParticle *p);
	class CParticle * particle_list;
    CParticleFX();
	virtual ~CParticleFX();

};

#endif // !defined(AFX_PARTICLEFX_H__D7AC68E1_AFA9_11D2_9AF1_00104B659729__INCLUDED_)
