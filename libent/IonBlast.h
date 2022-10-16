// IonBlast.h: interface for the CIonBlast class.
//
//  1999 Ion Storm LLP
//
//  Logic:
//      This still leans pretty heavy on the server for parameters,
//      it gets its origin, avelocity, angles and velocity when it is
//      spawned, then again when it bounces. None of these are 
//      necessary, since we know where the weapon model is, how fast
//      the blast is supposed to go, what the rotation speed is, 
//      and even where, how and when it bounces. All we need from the
//      server (if things are done TOTALLY right) is a spawn message
//      and a destroy message. Nonetheless, this is very little code
//      for implementation, there are only 4 lines in the weapons
//      source for init, 1 line for the bounce. This could be knocked
//      down to 1 line there and just a few here with some good 
//      support routines in a parent class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IONBLAST_H__17EE0AE8_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_IONBLAST_H__17EE0AE8_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Projectile.h"
// #include "client.h"

/*
#ifndef dllExport
#define dllExport __declspec( dllexport )
#endif
*/
#ifndef LIMIT_IONBLASTS
#define LIMIT_IONBLASTS 128
#endif

class CIonBlast : public CProjectile  
{
public:
	CClientEntity * Alloc();
	class CSound * snd_shoot;
	class CSound * snd_flyby;
	class CSound * snd_hit;
	CSound * snd_explode;
	class CParticleFX_IonSpray * sprayFX;
	void Destroy();
	class CParticleFX_IonTrail * particleFX;
	class CTrackFlare * flare;
	class CTrackLight * light;
//	void Notify();
	void ReInitialize();
	void CL_RunFrame();
	void SV_RunFrame();
	// CIonBlast * Spawn();
	CIonBlast();
    virtual ~CIonBlast();

};

#endif // !defined(AFX_IONBLAST_H__17EE0AE8_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
