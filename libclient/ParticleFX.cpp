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
    particle_list_size = 128;
    particle_list = new CParticle[particle_list_size];
    ent_sibling = NULL;
    // manager = NULL;
//    active_particle = NULL;
//    free_particle = particle_list;
    ResetParticles();
    particle_template = new CParticle;
}

CParticleFX::~CParticleFX()
{
    delete[] particle_list;
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
void CParticleFX::Physics()
{
    // rip this stuff from the client
    CParticle *p;
    float alpha;
    float time, time2;
    CVector origin;
    int color,i,particle_count=0;

    for(i=0;i<particle_list_size;i++)
    {
        p=&particle_list[i];
        if(! p->active)
            continue;

        time = (manager->current_time - p->time)*0.001;
        alpha = p->alpha + time*p->alphavel;
        if ( alpha <= 0 && p->die_time == 0 )
        {
            free_particle = p;
            p->active = 0;
            continue;
        }

        if ( p->die_time > 0 && p->die_time <= time )
        {
            p->die_time = 0;
            free_particle = p;
            p->active = 0;
            continue;
        }

        if ( alpha > 1.0 )
        {
		    alpha = 1;
        }
		color = p->color;
        time2 = time*time;
        origin.x = p->origin.x + p->velocity.x*time + p->acceleration.x*time2;
        origin.y = p->origin.y + p->velocity.y*time + p->acceleration.y*time2;
        origin.z = p->origin.z + p->velocity.z*time + p->acceleration.z*time2;
        if(p->type)
            manager->NetClient->V_AddComParticle (origin, color, alpha, (particle_type_t)p->type, p->rotation_vector, p->scale, p->flags, p->color_rgb);
            
        else
            manager->NetClient->V_AddParticle (origin, color, alpha, (particle_type_t)p->type);
        particle_count++;
    }
    if(particle_count)
        ent_sibling->run_fx = 1;
    else
        ent_sibling->run_fx = 0;
  
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
class CParticle *CParticleFX::SpawnParticle()
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
    if(free_particle)
    {
        if(!free_particle->active)
        {
            new_particle = free_particle;
            new_particle->active = 1;
            free_particle = NULL;
            return new_particle;
        }
    }

    for(i=0;i<particle_list_size;i++)
    {
        if(!particle_list[i].active)
        {
            new_particle = &particle_list[i];
            new_particle->active = 1;
            free_particle = NULL;
            break;
        }
    }
    return new_particle;
}

void CParticleFX::CL_RunFrame()
{
    SpawnParticles();
    Physics();          // default physics are fine

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
//    ResetParticles();
}

void CParticleFX::ResetParticles()
{
    int i;
    for(i=0;i<particle_list_size;i++)
    {
        particle_list[i].Reset();
    }
    free_particle = &particle_list[0];
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

    for(i=0;i<particle_list_size;i++)
    {
        
        if (manager->frand() > 0.9 )
        {
            if(! (p=SpawnParticle()))
                return;
            
            p->acceleration = particle_template->acceleration;
            p->time = manager->current_time;
            p->die_time = 0;
            // p->type = type;
            p->alpha = particle_template->alpha;
            // p->alphavel = -1 / (1+manager->frand()*0.2);
            p->alphavel = particle_template->alphavel;
		    // p->color = 0x84; + (rand()&7);// + (rand()&3); 
            p->color = particle_template->color;
            // p->pscale = crand () * 2.0;
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