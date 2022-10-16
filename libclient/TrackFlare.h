// TrackFlare.h: interface for the CTrackFlare class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACKFLARE_H__CA56A7C8_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_TRACKFLARE_H__CA56A7C8_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "TrackingEntity.h"

class CTrackFlare : public CTrackingEntity  
{
public:
	void ReInitialize();
	const char * modelname;
	void CL_RunFrame();
    CTrackFlare(const char * modelname, CVector & modelscale, CClientEntity *owner);
	struct entity_s * model;
	CTrackFlare();
	virtual ~CTrackFlare();

};

#endif // !defined(AFX_TRACKFLARE_H__CA56A7C8_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
