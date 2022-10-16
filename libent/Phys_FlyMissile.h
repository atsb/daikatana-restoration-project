// Phys_FlyMissile.h: interface for the CPhys_FlyMissile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYS_FLYMISSILE_H__488A2127_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_PHYS_FLYMISSILE_H__488A2127_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CEPhysics.h"

class CPhys_FlyMissile : public CPhysics  
{
public:
	void Physics();
	CPhys_FlyMissile();
	virtual ~CPhys_FlyMissile();

};

#endif // !defined(AFX_PHYS_FLYMISSILE_H__488A2127_ADBA_11D2_A7E1_0000C087A6E9__INCLUDED_)
