// HUDCSpeeds.h: interface for the CHUDCSpeeds class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUDCSPEEDS_H__E7E30CA1_BC2E_11D2_9AF1_00104B659729__INCLUDED_)
#define AFX_HUDCSPEEDS_H__E7E30CA1_BC2E_11D2_9AF1_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "HUD.h"

class CHUDCSpeeds : public CHUD  
{
public:
	CClientEntity * Alloc();
	void CL_RunFrame();
	int allocated_ents;
	int active_particles;
	int active_ents;
	unsigned int elapsed_time;
	unsigned long last_stat_time;
	CHUDCSpeeds();
	virtual ~CHUDCSpeeds();

};

#endif // !defined(AFX_HUDCSPEEDS_H__E7E30CA1_BC2E_11D2_9AF1_00104B659729__INCLUDED_)
