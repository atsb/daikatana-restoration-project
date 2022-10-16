// HUDCSpeeds.cpp: implementation of the CHUDCSpeeds class.
//
//
//  This is an example of using a client-side HUD
//
//  Alloc is overridden, CClientEntityManager.h has an entry in it's
//  enumeration, and CClientEntityManager constructor adds this 
//  to it's type list.
//
//////////////////////////////////////////////////////////////////////
#include "ClientEntityManager.h"
#include "HUDCSpeeds.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHUDCSpeeds::CHUDCSpeeds()
{
    active_ents = 0;
    active_particles = 0;
    allocated_ents = 0;
    elapsed_time = 0;
    last_stat_time = 0;

}

CHUDCSpeeds::~CHUDCSpeeds()
{

}

void CHUDCSpeeds::CL_RunFrame()
{
    if(manager->show_client_frame_speeds)
    {
        PrintXY(100,100, NULL, 0, "C_SPEEDS: %ims, %i ents active/%i ents allocated", elapsed_time, active_ents, allocated_ents);
        PrintXY(100,110, NULL, 0, "C_SPEEDS: %i FX Active", active_particles);
        CHUD::CL_RunFrame();
    } else {
        PrintXY(100,100,NULL,0,NULL);
        PrintXY(100,110,NULL,0,NULL);
    }

}

CClientEntity * CHUDCSpeeds::Alloc()
{
    CHUDCSpeeds *newent;
    newent = new CHUDCSpeeds;
    newent->manager = manager;
    return newent;
}
