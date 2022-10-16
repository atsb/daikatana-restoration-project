// TrackingEntity.cpp: implementation of the CTrackingEntity class.
//
//////////////////////////////////////////////////////////////////////
#include "TrackingEntity.h"
#include "ClientEntityManager.h"
#include "dk_point.h"
#include "p_user.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackingEntity::CTrackingEntity()
{
    edict_index = 0;
    clent_src = (CClientEntity *)NULL;

}

CTrackingEntity::~CTrackingEntity()
{

}

void CTrackingEntity::ReadParametersFromServer(sizebuf_s * msg)
{
    edict_index = manager->NetClient->ReadLong(msg);
}

void CTrackingEntity::SendParametersToClient()
{
    if(server_ent)
        manager->NetServer->WriteLong(server_ent->s.number);
    else
        manager->NetServer->WriteLong(edict_index);

}
