// ParticleFX_Smoke.h: interface for the CParticleFX_Smoke class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLEFX_SMOKE_H__2762AA23_B2E8_11D2_A7E2_0000C087A6E9__INCLUDED_)
#define AFX_PARTICLEFX_SMOKE_H__2762AA23_B2E8_11D2_A7E2_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ParticleFX.h"

class CParticleFX_Smoke : public CParticleFX  
{
public:
	void Physics();
	void SpawnParticles(CVector & start, CVector & end, float density);
	CParticleFX_Smoke();
	virtual ~CParticleFX_Smoke();

};

#endif // !defined(AFX_PARTICLEFX_SMOKE_H__2762AA23_B2E8_11D2_A7E2_0000C087A6E9__INCLUDED_)
