// ParticleFX_Smoke.cpp: implementation of the CParticleFX_Smoke class.
//
//////////////////////////////////////////////////////////////////////
#include "dk_shared.h"
#include "ClientEntityManager.h"
#include "ParticleFX_Smoke.h"
#include "Particle.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParticleFX_Smoke::CParticleFX_Smoke()
{
    particle_list_size = 256;   // lots of particles!
    particle_list = new CParticle[particle_list_size];
    ResetParticles();
}

CParticleFX_Smoke::~CParticleFX_Smoke()
{

}

void CParticleFX_Smoke::SpawnParticles(CVector & start, CVector & end, float density)
{
    CParticle   *p;
    CVector path, offset;
    path = end - start;
    float len, i;
   
    if(!manager)
        return;

    len = path.Normalize();
    offset = start;
    for ( i = 0; i < len; i+=density )
    {
        if ((p=SpawnParticle()) == NULL)
        {
            manager->NetClient->Com_Printf("smoke, no particles!\n");
		    return;
		}
        offset = offset+(path*density);
        p->time = manager->current_time;
        p->die_time = 0;
        p->acceleration.Zero();
        p->type = PARTICLE_SMOKE;
        p->origin = offset;
        p->alpha = 0.5;
        p->alphavel = -0.3;// / (1+frand()*0.5);
		// p->color = 0x84;// + (rand()&7);// + (rand()&3); 
        p->velocity.x = manager->crand()*2;
        p->velocity.y = manager->crand()*2;
        p->velocity.z = manager->crand()*2;
        p->color = 0xD7 + (rand()&7);
        p->scale = 0.1;
  }
}

void CParticleFX_Smoke::Physics()
{
    int i;
    CParticle *p;
    for(i=0;i<particle_list_size;i++)
    {
        p=&particle_list[i];
        if(! p->active)
            continue;
        p->scale+=0.1;
    }
    CParticleFX::Physics();

}
