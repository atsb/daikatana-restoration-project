// ParticleFX_IonSpray.h: interface for the CParticleFX_IonSpray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLEFX_IONSPRAY_H__D7AC68E2_AFA9_11D2_9AF1_00104B659729__INCLUDED_)
#define AFX_PARTICLEFX_IONSPRAY_H__D7AC68E2_AFA9_11D2_9AF1_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ParticleFX.h"

class CParticleFX_IonSpray : public CParticleFX  
{
public:
    void SpawnParticles();
    void SpawnParticles(CVector &start, CVector &end);
    CParticleFX_IonSpray();
	virtual ~CParticleFX_IonSpray();

};

#endif // !defined(AFX_PARTICLEFX_IONSPRAY_H__D7AC68E2_AFA9_11D2_9AF1_00104B659729__INCLUDED_)
