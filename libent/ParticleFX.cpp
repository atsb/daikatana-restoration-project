// ParticleFX.cpp: implementation of the CParticleFX class.
//
//
//  1999 Ion Storm LLP
//
//  ParticleFX is the base class for new particle effects in the
//  Daikatana engine. Each ParticleFX instance keeps its own list
//  of particles for deformation/physics. 
//////////////////////////////////////////////////////////////////////
#include "client.h"
#include "ParticleFX.h"
#include "ClientEntityManager.h"
#include "Particle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CParticleFX::CParticleFX()
{
    manager = NULL;
    particle_list = NULL;
    ent_sibling = NULL;
    particle_template = new CParticle;
}

CParticleFX::~CParticleFX()
{
    delete particle_template;
//    delete[] particle_list;
}

////////////////////////////////////////////////////////////////////////
//  physics
//
//  Description:
//      Default particle physics, override in new FX derivatives for 
//      different particle effects.
//
//  Parameters:
//      none
//
//  Return:
//      void
//  
//  BUGS:
//
void CParticleFX::Physics(CParticle *p)
{
    // rip this stuff from the client
    float alpha;
    float time, time2;
    CVector origin;
    int color, particle_count=0;

    if(! p->active)
        return;

    time = (manager->current_time - p->time)*0.001;
    alpha = p->alpha + time*p->alphavel;
    if ( alpha <= 0 && p->die_time == 0 )
    {
        FreeParticle(p);
        return;
    }

    if ( p->die_time > 0 && p->die_time <= time )
    {
        p->die_time = 0;
        FreeParticle(p);
        return;
    }

    if ( alpha > 1.0 )
    {
	    alpha = 1;
    }
    color = 0;
    time2 = time*time;
    origin.x = p->origin.x + p->velocity.x*time + p->acceleration.x*time2;
    origin.y = p->origin.y + p->velocity.y*time + p->acceleration.y*time2;
    origin.z = p->origin.z + p->velocity.z*time + p->acceleration.z*time2;
    if(p->type)
        manager->NetClient->V_AddComParticle (origin, color, alpha, (particle_type_t)p->type, p->scale, p->flags, p->color_rgb);
    else
        manager->NetClient->V_AddParticleRGB (origin, color, alpha, (particle_type_t)p->type, p->color_rgb, p->scale);
    particle_count++;
    
}



////////////////////////////////////////////////////////////////////////
//  SpawnParticle()
//
//  Description:
//      Find the next free particle in the particle list, return it.
//      keep a reverse-order linked list of active particles
//
//  Parameters:
//      none
//
//  Return:
//      A pointer to a free particle from the particle_list member 
//      array of CParticles
//
//  BUGS:
//
class CParticle *CParticleFX::SpawnParticle(CParticleFX *owner)
{
    CParticle *new_particle;
    int i;
    new_particle = NULL;

    if(!particle_list)
        return NULL;
    if(!manager)
        return NULL;
    if(!manager->ClientState)
        return NULL;
    if(manager->FX_FreeParticle)
    {
        if(!manager->FX_FreeParticle->active)
        {
            new_particle = manager->FX_FreeParticle;
            new_particle->active = 1;
            manager->FX_FreeParticle = NULL;
            manager->FX_ParticleBounds++;
            new_particle->FXManager = owner;
            return new_particle;
        }
    }

    for(i=0;i<MAX_PARTICLES;i++)
    {
        if(!particle_list[i].active)
        {
            new_particle = &particle_list[i];
            new_particle->active = 1;
            manager->FX_FreeParticle = NULL;
            new_particle->FXManager = owner;
            if(manager->FX_ParticleBounds < i + 1 && i < MAX_PARTICLES)
                manager->FX_ParticleBounds = i+1;
            break;
        }
    }
    return new_particle;
}

void CParticleFX::CL_RunFrame()
{
//    SpawnParticles();
//    Physics();          // default physics are fine

}

void CParticleFX::MakeNormalVecs(CVector & forward, CVector & right, CVector & up)
{
    // this rotate and negat guarantees a vector
    // not colinear with the original
    right.y = -forward[0];
    right.z = forward[1];
    right.x = forward[2];

    float d = DotProduct (right, forward);
    VectorMA (right, forward, -d, right);
    right.Normalize();
    CrossProduct (right, forward, up);

}

void CParticleFX::ReInitialize(CClientEntity * clen)
{
    ent_sibling = clen;
    manager = clen->manager;
    particle_list = manager->FX_Particles;
}

void CParticleFX::SpawnParticles()
{
    if(!ent_sibling)
        return;
    SpawnParticles(ent_sibling->old_origin, ent_sibling->origin);
}

void CParticleFX::SpawnParticles(CVector &start, CVector &end)
{
    CParticle *p;
    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();
    int i;
    float dec = 0.5;
    vec = vec * dec;

    for(i=0;i<16;i+=len)
    {
        
        if (manager->frand() > 0.5 )
        {
            if(! (p=SpawnParticle(this)))
                return;
            
            p->acceleration = particle_template->acceleration;
            p->time = manager->current_time;
            p->die_time = 0;
            p->alpha = particle_template->alpha;
            p->alphavel = particle_template->alphavel;
            p->color_rgb = particle_template->color_rgb;
            p->scale = particle_template->scale;
    
            p->origin.x = move.x + manager->crand()*5;
            p->origin.y = move.y + manager->crand()*5;
            p->origin.z = move.z + manager->crand()*5;

            p->velocity.x = manager->crand()*particle_template->speed;
            p->velocity.y = manager->crand()*particle_template->speed;
            p->velocity.z = manager->crand()*particle_template->speed;

            p->acceleration.z = 0;//-PARTICLE_GRAVITY;
        }
        move = move + vec;
    }
}


void CParticleFX::FreeParticle(class CParticle * p)
{
    manager->FX_FreeParticle = p;
    if(&particle_list[manager->FX_ParticleBounds-1] == p)
        manager->FX_ParticleBounds--;
    p->active = 0;
}
