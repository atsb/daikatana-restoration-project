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
#if _MSC_VER
#include <typeinfo.h>
#endif
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
    head = NULL;
    Next = NULL;
    run_fx = 0;
    active = 0;
    server_ent = NULL;
    client_ent = NULL;
    client_ent = NULL;
    origin.Zero();
    old_origin.Zero();
    velocity.Zero();
    avelocity.Zero();
    angles.Zero();
    old_angles.Zero();
    manager = NULL;
    class_size = 0;
    max_entities = 128;     // default max entities    
}
CClientEntity::~CClientEntity()
{

}

void CClientEntity::ReInitialize()
{
    origin.Zero();
}

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
    CClientEntity *retval=NULL;
    int i;

    for(retval = this,i=0;retval->Next;retval = retval->Next,i++)
    {
        //if((!retval->active) && (!retval->run_fx) && (retval != this))
        if((!retval->active) && (retval != this))
            break;
    }

    if(i >= MAX_CLENTITIES-1)   
    {
        // no free entities! Attempt to render, destroy and use another entity
        if(this->Next)
        {
            retval = this->Next;
            if(retval->server_index < MAX_CLENTITIES)
                manager->Entity_ServerIndex[retval->server_index]=NULL;
            retval->Destroy();
        } else {
            return NULL;
        }
    }
        
    if( (retval->active || retval == this) )
    {
        // need to allocate a new entity
        retval->Next = Alloc(); // call the appropriate allocation (each class needs override!)
        retval->Next->head = this;
        retval = retval->Next;

        retval->index = manager->entity_count;
        retval->class_index = class_index;
        retval->manager = manager;
        manager->entity_count++;
    }
    if(retval && i < max_entities && manager->entity_count < MAX_CLENTITIES)
    {
        retval->active = 1;
        manager->active_count++;
        retval->ReInitialize();
    } else if (i >= max_entities || manager->entity_count >= MAX_CLENTITIES) {
        delete retval;
        retval = NULL;
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
        manager->NetServer->WriteByte(index);
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
//          index/server_index <---parsed here
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
        else
            ReadParametersFromServer(msg);  // read them anyway, for unused Type index entity
        break;
    }
}

void CClientEntity::Notify()
{
    manager->NetServer->WriteByte(SVC_CLIENTENTITY);
    manager->NetServer->WriteByte(CE_NOTIFY);
    manager->NetServer->WriteByte(class_index);
    manager->NetServer->WriteByte(index);
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
    manager->NetServer->WriteByte(index);
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
//      (from the enumeration), and a server_index (index on the 
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
            {
                delete newEnt;
                return NULL;
            }
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


CClientEntity* CClientEntity::Alloc()
{
    CClientEntity *newent;
    newent = new CClientEntity();
    newent->manager = manager;
    return newent;
}
