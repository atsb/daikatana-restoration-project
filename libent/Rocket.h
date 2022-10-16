// Rocket.h: interface for the CRocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROCKET_H__E6181603_B00C_11D2_A7E2_0000C087A6E9__INCLUDED_)
#define AFX_ROCKET_H__E6181603_B00C_11D2_A7E2_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Projectile.h"

class CRocket : public CProjectile  
{
public:
	void * m_pvAfterBurnerSound;
	unsigned short m_usAfterBurner;
	unsigned long start_time;
	CClientEntity * Alloc();
	CVector smoke_end;
	CVector smoke_start;
	class CParticleFX_Smoke *smoke;
	float rotation;   // 1=clockwise, -1=counterclockwise
	CVector start;
	void MakeNormalVecs(CVector & forward, CVector & right, CVector & up);
	int last_spin_time;
	void ReadParametersFromServer(struct sizebuf_s * msg);
	void SendParametersToClient();
//	CVector up;
//	CVector right;
//	CVector forward;
	int delta_vel_y;
	int delta_vel_x;
	int delta_phiy;
	int delta_phix;
	void CL_RunFrame();
	class CTrackFlare * flare;
	class CTrackLight * light;
	void ReInitialize();
	CRocket();
//    CRocket(){};
	virtual ~CRocket();

};

#endif // !defined(AFX_ROCKET_H__E6181603_B00C_11D2_A7E2_0000C087A6E9__INCLUDED_)
