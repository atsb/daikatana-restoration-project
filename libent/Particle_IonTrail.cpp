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
    manager = NULL;
    particle_list = NULL;
}

CParticleFX_IonTrail::~CParticleFX_IonTrail()
{

}


float le_costable[64]={
	1.000000, -1.000000, 1.000000, -1.000000, 1.000000, -1.000000, 1.000000, -1.000000, 1.000000, -1.000000, 1.000000, -0.999999, 0.999999, -0.999999, 0.999999, -0.999999, 0.999999, -0.999999, 0.999999, -0.999998, 0.999998, -0.999998, 0.999998, -0.999998, 0.999998, -0.999997, 0.999997, -0.999997, 0.999997, -0.999996, 0.999996, -0.999996, 0.999996, -0.999995, 0.999995, -0.999995, 0.999994, -0.999994, 0.999994, -0.999994, 0.999993, -0.999993, 0.999992, -0.999992, 0.999992, -0.999991, 0.999991, -0.999991, 0.999990, -0.999990, 0.999989, -0.999989, 0.999988, -0.999988, 0.999988, -0.999987, 0.999987, -0.999986, 0.999986, -0.999985, 0.999985, -0.999984, 0.999983, -0.999983
};
float le_sintable[64]={
	0.000000, 0.000093, -0.000185, 0.000277, -0.000371, 0.000463, -0.000555, 0.000649, -0.000741, 0.000833, -0.000926, 0.001020, -0.001110, 0.001204, -0.001298, 0.001388, -0.001483, 0.001577, -0.001667, 0.001761, -0.001851, 0.001949, -0.002039, 0.002130, -0.002220, 0.002318, -0.002408, 0.002498, -0.002596, 0.002687, -0.002777, 0.002875, -0.002965, 0.003055, -0.003153, 0.003243, -0.003334, 0.003432, -0.003522, 0.003612, -0.003702, 0.003800, -0.003898, 0.003981, -0.004079, 0.004177, -0.004259, 0.004357, -0.004440, 0.004538, -0.004636, 0.004718, -0.004816, 0.004914, -0.004997, 0.005095, -0.005193, 0.005275, -0.005373, 0.005471, -0.005554, 0.005652, -0.005749, 0.005832
};
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
	float		c, s;
    CVector     dir;
    CVector move;
    CVector vec;

    if(!manager || !particle_list)
        return;

    move = start;
    vec = end - start;
    float len = vec.Normalize();
    MakeNormalVecs (vec, right, up);
    
	if(len > 63)
		len = 63;

    for ( int i = 0; i < len; i++ )
    {
        if ((p=SpawnParticle(this)) == NULL)
        {
		    return;
		}
        p->type = PARTICLE_SIMPLE;
        p->time = manager->current_time;
        p->die_time = 0;
        p->acceleration.Zero();
		c = le_costable[i];
		s = le_sintable[i];
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

        p->alpha = 0.8;
        p->alphavel = -3.0;
        p->color_rgb.Set(0,1,0);
        p->scale = 0.5;

        move = move + vec;

  }
}


void CParticleFX_IonTrail::SpawnParticles()
{
    if(!ent_sibling)
        return;
    SpawnParticles(ent_sibling->origin, ent_sibling->old_origin);
}

CClientEntity * CParticleFX_IonTrail::Alloc()
{
    CClientEntity *newent = new CClientEntity;
    newent->manager = manager;
    return newent;
}
