// TrackingEntity.h: interface for the CTrackingEntity class.
//
//  1999 Ion Storm LLP
//
//  Logic:
//      Base class for tracking entities (lights and other entities 
//      that will coordinate their movements with other entities)
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACKINGENTITY_H__CA56A7C6_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_TRACKINGENTITY_H__CA56A7C6_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ClientEntity.h"

class CTrackingEntity : public CClientEntity  
{
public:
	CClientEntity * clent_src;
	CTrackingEntity();
	virtual ~CTrackingEntity();

};

#endif // !defined(AFX_TRACKINGENTITY_H__CA56A7C6_AEC6_11D2_A7E1_0000C087A6E9__INCLUDED_)
