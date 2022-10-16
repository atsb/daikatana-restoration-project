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
   
    if(!manager || !particle_list)
        return;

    len = path.Normalize();
    offset = start;
    for ( i = 0; i < len; i+=density )
    {
        if ((p=SpawnParticle(this)) == NULL)
		    return;
        offset = offset+(path*density);
        p->time = manager->current_time;
        p->die_time = 0;
        p->acceleration.Zero();
        p->type = PARTICLE_SMOKE;
        p->origin = offset;
        p->alpha = 0.5;
        p->alphavel = -0.3;// / (1+frand()*0.5);
        p->velocity.x = manager->crand() * 20;
        p->velocity.y = manager->crand() * 20;
        p->velocity.z = manager->crand() * 20;
        p->color_rgb.Set(255,255,255);
        p->scale = 1.0;
    }
}

void CParticleFX_Smoke::Physics(CParticle *p)
{
    float c;
    CVector vecDir;
    if(! p->active)
        return;
    if(p->scale>0.1)
        p->scale+=0.01;
    c=p->color_rgb.z;
    if(c>0)
        c=c*0.95;

    if(p->velocity.z < 5)
        p->velocity.z++;
    p->color_rgb.Set(c,c,c);
    CParticleFX::Physics(p);
}
