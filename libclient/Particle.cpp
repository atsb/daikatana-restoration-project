// Particle.cpp: implementation of the CParticle class.
//
//////////////////////////////////////////////////////////////////////
#include "client.h"
#include "ClientEntityManager.h"
#include "Particle.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParticle::CParticle()
{
    Reset();
}

CParticle::~CParticle()
{

}


void CParticle::Reset()
{
    acceleration.Zero();
    active = 0;
    alpha = 0;
    alphavel = 0;
    color = 0;
    color_rgb.Zero();
    die_time = 0;
    flags = 0;
    origin.Zero();
    rotation_vector = 0;
    scale = 0;
    time = 0;
    type = 0;
    velocity.Zero();
}
