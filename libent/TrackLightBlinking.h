// TrackLightBlinking.h: interface for the CTrackLightBlinking class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACKLIGHTBLINKING_H__5B096F01_DCB2_11D2_B085_00104B659729__INCLUDED_)
#define AFX_TRACKLIGHTBLINKING_H__5B096F01_DCB2_11D2_B085_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "TrackLight.h"

class CTrackLightBlinking : public CTrackLight  
{
public:
	class CTrackFlare * pFlare;
	void LightValues(float, float, float, float, float);
	CClientEntity * Alloc();
	void SendParametersToClient();
	void ReadParametersFromServer(sizebuf_s *msg);
	float fBaseIntensity;
	float fLastBlinkTime;
	float fBlinkRate;
	void CL_RunFrame();
	CTrackLightBlinking();
	virtual ~CTrackLightBlinking();

};

#endif // !defined(AFX_TRACKLIGHTBLINKING_H__5B096F01_DCB2_11D2_B085_00104B659729__INCLUDED_)
