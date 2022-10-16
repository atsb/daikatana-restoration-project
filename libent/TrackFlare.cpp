// TrackFlare.cpp: implementation of the CTrackFlare class.
//
//////////////////////////////////////////////////////////////////////
#include "ref.h"
#include "p_user.h"
#include "client.h"
#include "ClientEntityManager.h"
#include "TrackFlare.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackFlare::CTrackFlare()
{
    active = 0;
    run_fx = 0;
//    class_index = ENT_CTrackFlare;
    model = NULL;
    modelname = NULL;
}

CTrackFlare::~CTrackFlare()
{
    memmgr.X_Free(model);
}

CTrackFlare::CTrackFlare(char * model_name, CVector & modelscale, CClientEntity *owner)
{
    // defaults 
    active = 0;
    run_fx = 0;
//    class_index = ENT_CTrackFlare;
    clent_src = owner;
    model = (entity_s *)memmgr.X_Malloc(sizeof(entity_s), MEM_TAG_ENTITY);
    memset(model, 0, sizeof(entity_s));
    modelname = model_name;
    model->frame = 0;
    model->alpha = 0.3;
    model->flags = RF_TRANSLUCENT;
    if(owner)
    {
        if(owner->manager)
        {
            model->model=(struct model_s *)owner->manager->re->RegisterModel((char *)model_name, RESOURCE_GLOBAL);
            model->render_scale = modelscale;
        }
    }
}

void CTrackFlare::CL_RunFrame()
{
    if(clent_src && model)
    {
        model->origin = clent_src->origin;
        clent_src->manager->NetClient->V_AddEntity(model);
    } else if (edict_index && model && manager) {
        // this track is attached to an edict/old entity
        if(manager->client_entities[edict_index].render_ent)
        {
            model->origin = manager->client_entities[edict_index].render_ent->origin;
            origin = model->origin;
        }
    }        
}

void CTrackFlare::ReInitialize()
{
	if(model)
		model->model=(struct model_s *)clent_src->manager->re->RegisterModel((char *)modelname, RESOURCE_GLOBAL);
}

CTrackFlare *CTrackFlare::Spawn(char *mdlname, CVector & modelscale, int e_index)
{
    CTrackFlare *newflare;

    if(!manager)
        return NULL;

    if( (newflare = (CTrackFlare *)manager->EntityTypes[class_index]->Spawn()) != NULL)
    {
        if(manager->client)
        {
            if(!modelname)
                modelname = (char *)memmgr.X_Malloc(64, MEM_TAG_MISC);
            strcpy(mdlname, modelname);
            if(!model)
                model = (entity_s *)memmgr.X_Malloc(sizeof(entity_s), MEM_TAG_ENTITY);
            model->render_scale = modelscale;
            edict_index = e_index;
        } 
    }
    return newflare;
}

void CTrackFlare::SendParametersToClient()
{
    manager->NetServer->WriteString(modelname);
}

void CTrackFlare::ReadParametersFromServer(sizebuf_s *msg)
{
    strcpy(modelname, manager->NetClient->ReadString(msg));
}
