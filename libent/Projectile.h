// Projectile.h: interface for the CProjectile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECTILE_H__17EE0AE7_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_PROJECTILE_H__17EE0AE7_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ClientEntity.h"
#include "CEPhysics.h"
#ifndef dllExport
#define dllExport __declspec( dllexport )
#endif

#ifndef LIMIT_PROJECTILES
#define LIMIT_PROJECTILES   128
#endif

class CProjectile : public CClientEntity  
{
public:
	void ReInitialize();
	virtual void CL_RunFrame();
	virtual void SendParametersToClient();
	virtual void ReadParametersFromServer(struct sizebuf_s *msg);
	float speed;
	CPhysics * RunPhysics;
	struct model_s * model;
//	struct edict_s * server_ent;
//	struct entity_s * client_ent;
	void PathFunction();
	int modelindex;
    CProjectile();
    virtual ~CProjectile();

};

#endif // !defined(AFX_PROJECTILE_H__17EE0AE7_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
