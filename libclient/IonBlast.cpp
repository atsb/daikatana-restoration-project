// IonBlast.cpp: implementation of the CIonBlast class.
//
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include "ref.h"
#include "IonBlast.h"
#include "ClientEntityManager.h"
#include "CEPhysics.h"
// #include "libclientlocals.h"
#include "TrackLight.h"
#include "TrackFlare.h"
#include "Particle_IonTrail.h"
#include "ParticleFX_IonSpray.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIonBlast::CIonBlast()
{
    CVector flare_scale(1,1,1);

    class_index = ENT_CIonBlast;
    modelindex=0;
    model = NULL;

    // permanently allocate a track light for each IonBlast
    light = new CTrackLight(0, 0.8, 0, 175, this);
    flare = new CTrackFlare("models/e1/we_ionbf.sp2", flare_scale, this);
    client_ent->render_scale.Set(3,3,3);
    client_ent->alpha = 0.3;
    client_ent->flags = RF_TRANSLUCENT;
    particleFX = new CParticleFX_IonTrail();
    sprayFX = new CParticleFX_IonSpray();
    run_with_server_frames = 1; // only run when the server sends data (don't get out of sync)
}

CIonBlast::~CIonBlast()
{

}

void CIonBlast::SV_RunFrame()
{

}

void CIonBlast::CL_RunFrame()
{
    int testme=0;
    float fr, fs;

    run_fx = 1;
    manager->Physics->FlyMissile(this);
    // run light
    fr = frand() * 250;
    light->intensity = fr; // flicker
    light->CL_RunFrame();
    // run flare
    flare->model->alpha = fr * 0.002;   // flicker alpha with light
    client_ent->alpha = fr * 0.002;
    fs = fr * 0.009;    // flicker scale with light
    if(fs)
        flare->model->render_scale.Set(fs, fs, fs);
    flare->CL_RunFrame();

    // now run the particle effects (maybe I should find a way to automagically do this?
    // RunFX();
}

void CIonBlast::ReInitialize()
{
    if(!client_ent)
        client_ent = (entity_s *)malloc(sizeof(entity_t));
    if(!manager)
        return;
  	if(!manager)
        return;
    if(manager->client)
    {
        if(!modelindex)
            modelindex = manager->NetClient->CL_ModelIndex("models/e1/we_ionbl.dkm");
        if(!model)
            model=(struct model_s *)manager->re->RegisterModel("models/e1/we_ionbl.dkm", RESOURCE_GLOBAL);
        client_ent->model = model;
    }
    if(manager->client)
    {
        flare->ReInitialize();
        light->ReInitialize();
        particleFX->ReInitialize(this);
        sprayFX->ReInitialize(this);
    }

}




void CIonBlast::Destroy()
{
    particleFX->ReInitialize(this);
    sprayFX->ReInitialize(this);
    CClientEntity::Destroy();
}

void CIonBlast::RunFX()
{
    if(active)
    {
        particleFX->SpawnParticles();
        sprayFX->SpawnParticles();
    }
    particleFX->Physics();  
    sprayFX->Physics();
}
