// Projectile.cpp: implementation of the CProjectile class.
//
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include "ClientEntityManager.h"
#include "Projectile.h"
#include "p_user.h"
#include "client.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectile::CProjectile()
{
    
//    class_index = ENT_CProjectile;
    client_ent = (entity_s *)memmgr.X_Malloc(sizeof(entity_t), MEM_TAG_ENTITY);
    // server_ent = NULL;
    //memset(client_ent, 0, sizeof(entity_t));
    client_ent->render_scale.Set(1,1,1);
    client_ent->alpha = 1.0;
    // class_size = sizeof(CProjectile);
}

CProjectile::~CProjectile()
{
    memmgr.X_Free(client_ent);
}

void CProjectile::PathFunction()
{
    
}



void CProjectile::ReadParametersFromServer(struct sizebuf_s * msg)
{
    manager->NetClient->ReadPos(msg, origin);
    manager->NetClient->ReadPos(msg, velocity);
    manager->NetClient->ReadPos(msg, avelocity);
    manager->NetClient->ReadPos(msg, angles);
    if(client_ent)
    {
        client_ent->origin = origin;
        client_ent->angles = angles;
    }
}

void CProjectile::SendParametersToClient()
{
    if(server_ent)
    {
        // leverage existing entity system, get parms from server side entity
        manager->NetServer->WritePosition(server_ent->s.origin);
        manager->NetServer->WritePosition(server_ent->velocity);
        manager->NetServer->WritePosition(server_ent->avelocity);
        manager->NetServer->WritePosition(server_ent->s.angles);
    } else {
        manager->NetServer->WritePosition(origin);
        manager->NetServer->WritePosition(velocity);
        manager->NetServer->WritePosition(avelocity);
        manager->NetServer->WritePosition(angles);
    }
    

}


void CProjectile::CL_RunFrame()
{
    manager->Physics->FlyMissile(this);
}

void CProjectile::ReInitialize()
{
}
