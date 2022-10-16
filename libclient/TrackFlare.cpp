// TrackFlare.cpp: implementation of the CTrackFlare class.
//
//////////////////////////////////////////////////////////////////////
#include "ref.h"
#include "ClientEntityManager.h"
#include "TrackFlare.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackFlare::CTrackFlare()
{

}

CTrackFlare::~CTrackFlare()
{
    free(model);
}

CTrackFlare::CTrackFlare(const char * model_name, CVector & modelscale, CClientEntity *owner)
{
    // defaults 
    clent_src = owner;
    model = (entity_s *)malloc(sizeof(entity_s));
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
    }
}

void CTrackFlare::ReInitialize()
{
	if(model)
		if(!model->model)
			model->model=(struct model_s *)clent_src->manager->re->RegisterModel((char *)modelname, RESOURCE_GLOBAL);
}
