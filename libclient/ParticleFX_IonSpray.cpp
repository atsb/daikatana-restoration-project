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
    particle_list_size = 32;
    particle_list = new CParticle[particle_list_size];
    ResetParticles();
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
    CParticle *p;
    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();
    int i;
    float dec = 0.5;
    vec = vec * dec;

    for(i=0;i<particle_list_size;i++)
    {
        
        if (manager->frand() > 0.9 )
        {
            if(! (p=SpawnParticle()))
                return;
            
            p->acceleration.Zero();
            p->time = manager->current_time;
            p->die_time = 0;
            // p->type = type;
            p->alpha = 0.3;
            p->alphavel = -1.0 / (1+manager->frand()*0.2);
            
		    p->color = 0x84; + (rand()&7);// + (rand()&3); 
            
            // p->pscale = crand () * 2.0;
            
            p->origin.x = move.x + manager->crand()*5;
            p->origin.y = move.y + manager->crand()*5;
            p->origin.z = move.z + manager->crand()*5;

            p->velocity.x = manager->crand()*250;
            p->velocity.y = manager->crand()*250;
            p->velocity.z = manager->crand()*250;

 //           p->acceleration.z = 0;//-PARTICLE_GRAVITY;
        }
        move = move + vec;
    }
}