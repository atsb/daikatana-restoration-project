// FadeEnt.cpp: implementation of the CFadeEnt class.
//
//////////////////////////////////////////////////////////////////////

#include "ClientEntityManager.h"
#include "FadeEnt.h"
#include "p_user.h"
#include "client.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFadeEnt::CFadeEnt()
{

}

CFadeEnt::~CFadeEnt()
{

}

CClientEntity * CFadeEnt::Alloc()
{
    CFadeEnt *newent = new CFadeEnt;
    if(newent)
        newent->manager = manager;
    return newent;
}

void CFadeEnt::ReadParametersFromServer(struct sizebuf_s * msg)
{
    entity_index = manager->NetClient->ReadLong(msg);
    alpha = manager->NetClient->ReadFloat(msg);
    alpha_vel = manager->NetClient->ReadFloat(msg);
    start_time = manager->current_time;


}

void CFadeEnt::SendParametersToClient()
{
    manager->NetServer->WriteLong(entity_index);
    manager->NetServer->WriteFloat(alpha);
    manager->NetServer->WriteFloat(alpha_vel);
}

void CFadeEnt::CL_RunFrame()
{
    float newalpha;

//    cent = &manager->client_entities[entity_index];
    newalpha = alpha + (alpha_vel * ((manager->current_time - start_time) * 0.01));
    // fade local copy
    if(newalpha <= 0.0)
    {
        newalpha = 0;
        Destroy();
    }
    
}

void CFadeEnt::Message(float a, float av, int i)
{
    alpha = a;
    alpha_vel = av;
    entity_index = i;
    LinkToClient();
    Destroy();
}
