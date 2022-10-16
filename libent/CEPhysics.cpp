// Physics.cpp: implementation of the CPhysics class.
//
//////////////////////////////////////////////////////////////////////
//#include <windows.h>
#include "dk_system.h"
#if _MSC_VER
#include <mmsystem.h>
#endif
#include "client.h"
#include "ClientEntityManager.h"
#include "ClientEntity.h"
#include "CEPhysics.h"
#include "TrackingEntity.h"
#include "dk_point.h"
#include "p_user.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhysics::CPhysics()
{
    // initialize time variables
    current_time = last_frame_time = SysTime();
    elapsed_time = 0;
}

CPhysics::~CPhysics()
{

}

void CPhysics::FlyMissile(CClientEntity * clent)
{
    // save physics data
    clent->old_origin = clent->origin;
    clent->old_angles = clent->angles;
    clent->origin  = FlyMissile(clent->origin, clent->velocity);
    Rotate(clent);
    
    // leverage existing render code, use this shit for now
    if(clent->client_ent)
    {
        clent->client_ent->origin = clent->origin;
        manager->NetClient->V_AddEntity(clent->client_ent);
    }

}

////////////////////////////////////////////////////////////////////////
//  PreFrame()
//
//  Description:
//      Executed before physics are applied during a game frame. Things 
//      like current time and other pre-frame physics related variables
//      are set here.
//  
//  Parameters:
//      none
//
//  Return:
//      void
//
//  BUGS:
// 
void CPhysics::PreFrame()
{
    current_time = SysTime();
    if(manager)
        manager->current_time = current_time;
    elapsed_time = current_time - last_frame_time;
    timescale = elapsed_time * 0.001;
}

void CPhysics::PostFrame()
{
    last_frame_time = SysTime();
}

void CPhysics::Rotate(CClientEntity * clent)
{
	VectorMA(clent->angles, clent->avelocity, timescale, clent->angles);
    if(clent->client_ent)
    {
        clent->client_ent->angles = clent->angles;
    }
}

void CPhysics::Track(CTrackingEntity * tracker)
{
    tracker->origin = tracker->clent_src->origin;
}

CVector CPhysics::FlyMissile(CVector origin, CVector velocity)
{
    CVector retval;
    VectorMA(origin, velocity, timescale, retval);	
    return retval;
}

CVector CPhysics::Rotate(CVector angles, CVector avelocity)
{
	CVector retval;
	VectorMA(angles, avelocity, timescale, retval);
	return retval;
}

unsigned long CPhysics::SysTime()
{
    unsigned long ulReturn;

    ulReturn = 0;

    if(manager)
    {
        if(manager->client && manager->ClientState)
            ulReturn = manager->ClientState->time;
        else if (manager->server && manager->NetServer)
            ulReturn = (unsigned long)(manager->NetServer->time * 1000);
    }

	// return timeGetTime(); 
    return ulReturn;
}