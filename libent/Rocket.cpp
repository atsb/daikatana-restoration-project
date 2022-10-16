// Rocket.cpp: implementation of the CRocket class.
//
//////////////////////////////////////////////////////////////////////
#include "client.h"
#include "ClientEntityManager.h"
#include "Rocket.h"
#include "TrackLight.h"
#include "TrackFlare.h"
#include "p_user.h"
#include "ParticleFX_Smoke.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRocket::CRocket()
{
    CVector flarescale;
    class_index = ENT_CRocket;
    modelindex  = 0;
    model = NULL;
    flarescale.Set(1,1,1);
    light = new CTrackLight(1,1,1,175,this);
    flare = new CTrackFlare("models/global/we_flareamber.sp2", flarescale, this);
    client_ent->render_scale.Set(2,2,2);
    run_with_server_frames = 1;
    delta_phix=0;
    delta_phiy=0;
    delta_vel_x=40;
    delta_vel_y=40;
    rotation=0;
    light->red = 0.7;
    light->green = 0.5;
    light->blue = 0.3;
    smoke = new CParticleFX_Smoke();
    m_usAfterBurner = 0;
    
    // class_size = sizeof(CRocket);
}

CRocket::~CRocket()
{
    delete light;
    delete flare;
    delete smoke;
}

void CRocket::ReInitialize()
{
    if(!client_ent)
        client_ent = (entity_s *)memmgr.X_Malloc(sizeof(entity_t), MEM_TAG_ENTITY);
    if(!manager)
        return;
    if(manager->client)
    {
        modelindex = manager->NetClient->CL_ModelIndex("models/e1/we_swrocket.dkm");
        model=(struct model_s *)manager->re->RegisterModel("models/e1/we_swrocket.dkm", RESOURCE_GLOBAL);
        client_ent->model = model;
    }
    if(manager->client)
    {
        flare->ReInitialize();
        light->ReInitialize();
    }
    last_spin_time = 0;
    delta_phix = delta_phiy = 0;
//    forward.Zero();
//    up.Zero();
//    right.Zero();
    delta_vel_x = delta_vel_y = 0;
    speed = 0;
    m_usAfterBurner = 0;
    smoke->ReInitialize(this);
}

////////////////////////////////////////////////////////////////////////
//  CL_RunFrame()
//
//  Description:
//      Run the missile physics. This is a sidewinder rocket, so it 
//      needs to oscillate (sine/cosine wave). In 3 dimensions, it's
//      not a wave, but a corkscrew/spiral. Apply special effects
//      during this run as well, placing smoke trails and adjusting
//      the rocket flare with velocity
//
void CRocket::CL_RunFrame()
{
    // sidewinder is an unusual weapon, takes a spiraling path
    float scale, period=75, s, c, r, fscale, radius=3, fTime;
    CVector neworg, distance, dir, forward, right, up;
    CVector vecAngles;
    unsigned long t;

    // ensure the Entity manager runs the particle FX 
    run_fx = 1;     

    if(!active)
        return;

    // get the new forward position in space
    VectorMA(origin, velocity, manager->Physics->timescale, neworg);

    // how far has the rocket travelled since it started?
    distance = neworg-start;
    smoke_end = neworg;    // store this for RunFX() calls
    smoke_start = origin;
    
    // Flare and light effects
    r = manager->frand();    // coordinated effects
    if(distance.Length() > 400)
    {
        // this rocket has the afterburners fired up!
        fscale = (2.0 + (r * 2));
        flare->model->render_scale.Set(fscale, fscale, fscale);
        flare->model->alpha = 0.7 + (r * 0.3);
        light->intensity = 150 * (1+r);
        if(!m_usAfterBurner)
        {
            // play sound
            m_usAfterBurner = 1;
            if(manager->pvSounds[CE_SND_AFTERBURNER])
                manager->NetClient->S_StartSoundQuake2(origin, 0, 0, manager->pvSounds[CE_SND_AFTERBURNER], 1, ATTN_NORM, 0);
            // increase velocity
        }
    } else {
        fscale = (0.2 + r);
        flare->model->render_scale.Set(fscale, fscale, fscale);
        flare->model->alpha = 0.3 + (r * 0.1);
        light->intensity = 75 * (1+r);
    }
    flare->CL_RunFrame();
    light->CL_RunFrame();

    // compute the location in the oscillation
    t = manager->Physics->current_time;
    scale = velocity.Length() ;
    // Offset in the oscillation period, if necessary
    // scale = scale * rotation;
    fTime = (t - start_time) * 0.01;
    s = radius * sin(fTime+rotation);
    c = radius * cos(fTime+rotation);
    distance.Normalize();
    MakeNormalVecs (distance, right, up);
    dir = right * c;
    VectorMA(dir, up, s, dir);
    smoke->SpawnParticles(neworg,origin,10);
    // place the entity where it belongs!
    
    
    origin = neworg + dir;
    vecAngles = neworg - client_ent->origin;
    vecAngles.VectorToAngles(vecAngles);
    
    if(client_ent)
    {
        client_ent->origin = origin;
        client_ent->angles = vecAngles; // face "velocity"
        manager->NetClient->V_AddEntity(client_ent);
    }
}

void CRocket::SendParametersToClient()
{
    CProjectile::SendParametersToClient();
//    manager->NetServer->WritePosition(forward);
//    manager->NetServer->WritePosition(right);
//    manager->NetServer->WritePosition(up);
    manager->NetServer->WriteLong(speed);
    manager->NetServer->WriteFloat(rotation);
    manager->NetServer->WriteByte(delta_phix);
    manager->NetServer->WriteByte(delta_phiy);
    manager->NetServer->WriteByte(delta_vel_x);
    manager->NetServer->WriteByte(delta_vel_y);
}

void CRocket::ReadParametersFromServer(struct sizebuf_s * msg)
{
    CProjectile::ReadParametersFromServer(msg);
//    manager->NetClient->ReadPos(msg, forward);
//    manager->NetClient->ReadPos(msg, right);
//    manager->NetClient->ReadPos(msg, up);
    speed = manager->NetClient->ReadLong(msg);
    rotation = manager->NetClient->ReadFloat(msg);
    delta_phix = manager->NetClient->ReadByte(msg);
    delta_phiy = manager->NetClient->ReadByte(msg);
    delta_vel_x = manager->NetClient->ReadByte(msg);
    delta_vel_y = manager->NetClient->ReadByte(msg);
    start = origin;
    start_time = manager->Physics->current_time;
}

void CRocket::MakeNormalVecs(CVector & forward, CVector & right, CVector & up)
{
    right.y = -forward[0];
    right.z = forward[1];
    right.x = forward[2];

    float d = DotProduct (right, forward);
    VectorMA (right, forward, -d, right);
    right.Normalize();
    CrossProduct (right, forward, up);

}

CClientEntity * CRocket::Alloc()
{
    return new CRocket;

}
