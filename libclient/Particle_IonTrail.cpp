// Particle_IonTrail.cpp: implementation of the CParticle_IonTrail class.
//
//////////////////////////////////////////////////////////////////////

#include "client.h"
#include "ref.h"
#include "dk_point.h"
#include "ClientEntityManager.h"
#include "Particle_IonTrail.h"
#include "CEPhysics.h"
#include "Particle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParticleFX_IonTrail::CParticleFX_IonTrail()
{
    particle_list_size = 256; 
    particle_list = new CParticle[particle_list_size];
    ResetParticles();
}

CParticleFX_IonTrail::~CParticleFX_IonTrail()
{

}



///////////////////////////////////////////////////////////////////////
//  SpawnParticles(start, end)
//
//  Description:
//      Spawns the particles for this game frame
//
//  Parameters:
//      start       new ionblast origin
//      end         old ionblast origin
//
//  Return:
//      void
//
//  BUGS:
//
//
// NOTES:
//      Using start,end for now so this effect may be used elsewhere.
//      I will probably add a quick override for SpawnParticles and
//      use the CClientEntity * for the IonBlast just to simplify
//
void CParticleFX_IonTrail::SpawnParticles(CVector & start, CVector & end)
{
    CParticle   *p;
    CVector     right, up;
    float       d, c, s;
    CVector     dir;
    int         particle_count=0;
    CVector move;
    CVector vec;

    move = start;
    vec = end - start;
    float len = vec.Normalize();
    if(!manager)
        return;
    MakeNormalVecs (vec, right, up);

    for ( int i = 0; i < len; i++ )
    {
        particle_count++;
        if ((p=SpawnParticle()) == NULL)
        {
		    return;
		}

        p->time = manager->current_time;
        p->die_time = 0;
        p->acceleration.Zero();

        d = i * 0.31415;
        c = cos(d);
        s = sin(d);

        dir = right * c;
        VectorMA (dir, up, s, dir);
        
        p->origin.x = move.x + dir.x*5;
        p->origin.y = move.y + dir.y*5;
        p->origin.z = move.z + dir.z*5;

        p->velocity.x = dir.x*5;
        p->velocity.y = dir.y*5;
        p->velocity.z = dir.z*5;
		
        p->acceleration.x = p->velocity.x * -62;
		p->acceleration.y = p->velocity.y * -62;
		p->acceleration.z = p->velocity.z * -62;

        p->alpha = 0.2;
        p->alphavel = -0.8;// / (1+frand()*0.5);
		p->color = 0x84;// + (rand()&7);// + (rand()&3); 

        move = move + vec;

  }
    ent_sibling->run_fx = particle_count;
}


void CParticleFX_IonTrail::SpawnParticles()
{
    if(!ent_sibling)
        return;
    SpawnParticles(ent_sibling->origin, ent_sibling->old_origin);
}
