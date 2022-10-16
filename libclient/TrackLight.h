// TrackLight.h: interface for the CTrackLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACKLIGHT_H__CA56A7C7_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_TRACKLIGHT_H__CA56A7C7_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "TrackingEntity.h"

class CTrackLight : public CTrackingEntity  
{
public:
    CTrackLight(float r, float g, float b, float i, CClientEntity *e);
	virtual void Modulate();
	void CL_RunFrame();
	float intensity;
	float blue;
	float green;
	float red;
	CTrackLight();
	virtual ~CTrackLight();

};

#endif // !defined(AFX_TRACKLIGHT_H__CA56A7C7_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
