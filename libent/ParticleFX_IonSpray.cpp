// ParticleFX_IonSpray.cpp: implementation of the CParticleFX_IonSpray class.
//
//////////////////////////////////////////////////////////////////////
#include "client.h"
#include "ClientEntityManager.h"
#include "ParticleFX_IonSpray.h"
#include "Particle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParticleFX_IonSpray::CParticleFX_IonSpray()
{
    manager = NULL;
    particle_list = NULL;
}

CParticleFX_IonSpray::~CParticleFX_IonSpray()
{

}

void CParticleFX_IonSpray::SpawnParticles()
{
    if(!ent_sibling)
        return;
    SpawnParticles(ent_sibling->old_origin, ent_sibling->origin);
}

void CParticleFX_IonSpray::SpawnParticles(CVector &start, CVector &end)
{
    CParticle   *p;
    float       r;
    int         iLoop;
    if(!manager || !particle_list)
        return;


    for(iLoop = 0; iLoop < 10; iLoop++)
    {
        if(rand()&1)
            continue;
        if(! (p=SpawnParticle(this)))
            return;
    
        r = manager->frand();
        p->type = PARTICLE_SIMPLE;
        p->acceleration.Zero();
        p->time = manager->current_time;
        p->die_time = 0;
        p->alpha = 1.0;
        p->alphavel = -1.0 * (r + 0.1);
        p->scale = 2.0 * r;
        p->color_rgb.Set(0,1,0);
            
        p->origin.x = end.x + manager->crand()*5;
        p->origin.y = end.y + manager->crand()*5;
        p->origin.z = end.z + manager->crand()*5;

        p->velocity.x = manager->crand()*100;
        p->velocity.y = manager->crand()*100;
        p->velocity.z = manager->crand()*100;
    }
}