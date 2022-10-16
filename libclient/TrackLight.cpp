// TrackLight.cpp: implementation of the CTrackLight class.
//
//////////////////////////////////////////////////////////////////////
#include "ClientEntityManager.h"
#include "TrackLight.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackLight::CTrackLight()
{
    manager = NULL;
}

CTrackLight::~CTrackLight()
{

}

void CTrackLight::CL_RunFrame()
{
    if(manager)
		manager->NetClient->V_AddLight(clent_src->origin, intensity, red, green, blue);
    else
		if(clent_src)
			clent_src->manager->NetClient->V_AddLight(clent_src->origin, intensity, red, green, blue);
}

void CTrackLight::Modulate()
{

}

CTrackLight::CTrackLight(float r, float g, float b, float i, CClientEntity * e)
{
    manager = NULL;
    red=r;
    green=g;
    blue=b;
    intensity=i;
    clent_src = e;
    
}
