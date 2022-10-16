// FadeEnt.h: interface for the CFadeEnt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FADEENT_H__3CEAA641_BE14_11D2_9AF1_00104B659729__INCLUDED_)
#define AFX_FADEENT_H__3CEAA641_BE14_11D2_9AF1_00104B659729__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ClientEntity.h"

class CFadeEnt : public CClientEntity  
{
public:
	void Message(float a, float av, int i);
	float alpha_vel;
	float alpha;
	void CL_RunFrame();
	unsigned long start_time;
	int entity_index;
	void SendParametersToClient();
	void ReadParametersFromServer(struct sizebuf_s *msg);
	CClientEntity * Alloc();
	CFadeEnt();
	virtual ~CFadeEnt();

};

#endif // !defined(AFX_FADEENT_H__3CEAA641_BE14_11D2_9AF1_00104B659729__INCLUDED_)
