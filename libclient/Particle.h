// Particle.h: interface for the CParticle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CParticle  
{
public:
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
	int color;
	CVector velocity;
	CVector origin;
	CParticle();
	virtual ~CParticle();
};

#endif // !defined(AFX_PARTICLE_H__CA3C14E3_AF08_11D2_A7E1_0000C087A6E9__INCLUDED_)
