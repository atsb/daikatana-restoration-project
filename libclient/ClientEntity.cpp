// ClientEntity.cpp: implementation of the CClientEntity class.
//
//  1999 Ion Storm LLP
//
//  The CClientEntity is the base class for client side entities.
//  Projectiles, flares and track entities are derived from the
//  CClientEntity class.
//
//  There are additional support classes which enhance the entities,
//  notably CParticleFX derivatives. The CParticleFX derived classes
//  render simple and complex particle effects on the client.
//
//  Physics are handled on a per-entity basis --that is, physics
//  routines can be overridden. They should be run in the CL_Frame
//  virtual member.
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <typeinfo.h>
#include "ClientEntity.h"
#include "ClientEntityManager.h"
#include "CEPhysics.h"
// #include "libclientlocals.h"
#include "p_user.h"
#include "client.h"
// #include "dk_point.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientEntity::CClientEntity()
{
    unsigned short found=0;
    int i = sizeof(CClientEntity);
    head = NULL;
    run_fx = 0;
    active = 0;
    server_ent = NULL;
    client_ent = NULL;
    // client_ent = NULL;
    // re = NULL;
//    client = 0;
//    server = 0;
    origin.Zero();
    old_origin.Zero();
    velocity.Zero();
    avelocity.Zero();
    angles.Zero();
    old_angles.Zero();
    manager = NULL;
    
}

CClientEntity::~CClientEntity()
{

}

////////////////////////////////////////////////////////////////////////
//  InitManager(man)
//
//  Description:
//      Initializes the elements in the EntityList[] with a pointer to
//      an instance of the manager. The manager class provides 
//      other application functionality (V_AddXXX routines, etc...) and
//      identifies the entities as client or server-side.
//
//  Parameters:
//      man         a pointer to a CClientEntityManager instance
//
//  Return:
//      void
//
//  BUGS:
//
void CClientEntity::InitManager(CClientEntityManager * man)
{
    CClientEntity *ptr;
    manager = man;

    int i, last_in_list;
    
    
    ptr = NULL;
    
    // organize the linked list, since entity allocation is static
    // this will expedite searches
    for(i=0;i<MAX_CLENTITIES;i++)
    {
        if(manager->EntityList[i]->class_index == class_index)
        {
            // found a sibling here!
            if(!ptr)
            {
                // first born in the list! yay!
                ptr = manager->EntityList[i];
                manager->EntityList[i]->head = manager->EntityList[i];
                head = manager->EntityList[i];                
            } else {
                // ptr->next = manager->EntityList[i];
                manager->EntityList[i]->head = head;
                ptr = manager->EntityList[i];
                last_in_list = i;
            }
        }
    }
}

void CClientEntity::ReInitialize()
{
    origin.Zero();
}

/*
void CClientEntity::SetNetState()
{
    if(manager)
    {
        server = manager->server;
        client = manager->client;
    }
}
*/
////////////////////////////////////////////////////////////////////////
//  InitBase()
//
//  Description:
//      Lives exclusively in the CClientEntity class to be inherited by 
//      its derivatives. These initializers are common to all sub-classes.
//
void CClientEntity::InitBase()
{
    origin.Zero();
//    re = manager->re;  
}

///////////////////////////////////////////////////////////////////////
//  Spawn()
//
//  Description:
//      Iterate through the EntityList, looking for a free CClientEntity
//      and return it's address. It is always called from the "head" for
//      this type.
//
//  Parameters:
//      none
//
//  Return:
//      a pointer to a free CClientEntity or NULL on failure.
//
//  BUGS:
//
CClientEntity *CClientEntity::Spawn()
{
    CClientEntity *retval;
    int i;

    retval = NULL;

    for(i=class_index;i<MAX_CLENTITIES;i+=ENT_TotalTypes)
    {
        if(manager->EntityList[i]->active || manager->EntityList[i]->run_fx)
            continue;
        retval = manager->EntityList[i];
        retval->active = 1;
        manager->active_count++;
        retval->InitBase();
        retval->ReInitialize();
        break;
    }

    return retval;
}
//
////////////////////////////////////////////////////////////////////////

void CClientEntity::Destroy()
{

    this->active = 0;
      
    if(manager->server)
    {
        // send a message to the client to destroy the server
        manager->NetServer->WriteByte(SVC_CLIENTENTITY);
        manager->NetServer->WriteByte(CE_DESTROY);
        manager->NetServer->WriteByte(class_index);
        manager->NetServer->WriteByte(array_index);
        manager->NetServer->MultiCast(origin, MULTICAST_ALL_R);
    } 

}


////////////////////////////////////////////////////////////////////////
//********************************************************************//
//  NETWORK FUNCTIONS
//
//  Standard:
//      SVC_CLIENTENTITY    <---- parsed in cl_parse.cpp
//          CE_SPAWN, CE_NOTIFY or CE_DESTROY   <-- parsed in manager
//          class_index     <---- parsed in manager
//          array_index/server_index <---parsed here
//          optional...
//
void CClientEntity::ReceiveServerMessage(int cmd, struct sizebuf_s *msg)
{
    switch(cmd)
    {
    case CE_SPAWN:
        Spawn(msg);
        break;
    case CE_NOTIFY:
        Notify(msg);
        break;
    case CE_DESTROY:
        Destroy(msg);
        break;
    }
}

void CClientEntity::Notify(struct sizebuf_s * msg)
{
    int cmd, sv_index;

    sv_index = manager->NetClient->ReadByte(msg);
    cmd = manager->NetClient->ReadByte(msg);
    switch(cmd)
    {
    case CE_NOTIFY_PARAMETERS:
        if(manager->Entity_ServerIndex[sv_index])
            manager->Entity_ServerIndex[sv_index]->ReadParametersFromServer(msg);
        break;
    }
}

void CClientEntity::Notify()
{
    manager->NetServer->WriteByte(SVC_CLIENTENTITY);
    manager->NetServer->WriteByte(CE_NOTIFY);
    manager->NetServer->WriteByte(class_index);
    manager->NetServer->WriteByte(array_index);
    manager->NetServer->WriteByte(CE_NOTIFY_PARAMETERS);
    SendParametersToClient();
    manager->NetServer->MultiCast(origin, MULTICAST_ALL_R);
}

////////////////////////////////////////////////////////////////////////
//  LinkToClient()
//
//  Description:
//      Sends relevant parameters to the remote to create a new
//      entity.
//
void CClientEntity::LinkToClient()
{
    if(!manager)
        return;
    manager->NetServer->WriteByte(SVC_CLIENTENTITY);
    manager->NetServer->WriteByte(CE_SPAWN);
    manager->NetServer->WriteByte(class_index);
    // this stuff is *ALWAYS* sent on a spawn
    manager->NetServer->WriteByte(array_index);
    SendParametersToClient();   // call virtual function, send the right 
                                // stuff for the right type of entity
    FinishLinkToClient();   // separate because some entities may
                            // have different multicast requirements, e.g. MULTICAST_ALL_R or
                            // UNICAST, etc...
}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  SendParametersToClient()
//
//  Description:
//      Sends parameters specific to this class to the client for its
//      initialization. Every ClientEntity receives the class_index 
//      (from the enumeration), and a server_index (array_index on the 
//      server)
//
//  Parameters:
//      none
//
//  Return:
//      void
//
//  BUGS:
//
void CClientEntity::SendParametersToClient()
{
    
}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  ReadParametersFromClient(sizebuf_s *msg)
//
//  Description:
//      Compliments SendParametersToClient(). The data read MUST MATCH
//      EXACTLY the data sent via SendParametersToClient(). These
//      are options specific to this type of class. The defaults, which
//      are sent for every CClientEntity and derivative, are read in
//      ReadDefaultsFromServer(), which matches the parameters in 
//      LinkToClient().
//
//  Parameters:
//      msg         a pointer to the sizebuf_s structure holding the
//                  net message.
//
//  Return:
//      void
//
//  BUGS:
//
void CClientEntity::ReadParametersFromServer(sizebuf_s * msg)
{

}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
void CClientEntity::ReadDefaultsFromServer(sizebuf_s *msg)
{
    server_index = manager->NetClient->ReadByte(msg);
    ReadParametersFromServer(msg);
}


void CClientEntity::FinishLinkToClient()
{
    // default multicast, override for other options
    manager->NetServer->MultiCast(origin, MULTICAST_ALL_R);
}

CClientEntity * CClientEntity::Spawn(struct sizebuf_s * msg)
{
    CClientEntity *newEnt;
    newEnt = NULL;
        
    if(manager->client)
    {
        if(newEnt = Spawn())
        {
            newEnt->ReadDefaultsFromServer(msg);
            if(newEnt->server_index > MAX_CLENTITIES)
                return NULL;
//            if(manager->Entity_ServerIndex[newEnt->server_index])
//                if(manager->Entity_ServerIndex[newEnt->server_index]->prev)
//                {
                    // kill what's just been created, this is a dupe 
//                    newEnt->Destroy();
//                    return NULL;    // DUPE! (can happen on slow net connections)
//                }
            manager->Entity_ServerIndex[newEnt->server_index] = newEnt;
        }
    }
    return newEnt;
}

void CClientEntity::Destroy(struct sizebuf_s *msg)
{
    int sv_index;
    
    sv_index = manager->NetClient->ReadByte(msg);
    if(manager->Entity_ServerIndex[sv_index])
    {
        manager->Entity_ServerIndex[sv_index]->Destroy();
        manager->Entity_ServerIndex[sv_index] = NULL;
    }
}


////////////////////////////////////////////////////////////////////////
//********************************************************************//
//  PHYSICS

void CClientEntity::SV_RunFrame()
{
}

void CClientEntity::CL_RunFrame()
{

}
/*
int CClientEntity::ModelIndex(char * name)
{
    if(client)
        if(manager->NetClient)
            return manager->NetClient->CL_ModelIndex(name);    
    return 0;
}
*/

/*
void * CClientEntity::RefModel(char * name, int resource)
{
    if(re)
        return re->RegisterModel(name, (resource_t)resource);
    else
        return NULL;
}
*/
////////////////////////////////////////////////////////////////////////
//********************************************************************//
//  




void CClientEntity::SendDefaultsToClient()
{

}

void CClientEntity::CL_StillFrame()
{
    if(client_ent)
    {
        client_ent->origin = origin;
        manager->NetClient->V_AddEntity(client_ent);
    }
}


void CClientEntity::RunFX()
{

}
