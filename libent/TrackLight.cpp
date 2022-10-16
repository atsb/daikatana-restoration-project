// TrackLight.cpp: implementation of the CTrackLight class.
//
//////////////////////////////////////////////////////////////////////
#include "ClientEntityManager.h"
#include "TrackLight.h"
#include "p_user.h"
#include "client.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackLight::CTrackLight()
{
    intensity = 0;
    red = 0;
    green = 0;
    blue = 0;
    
    manager = NULL;
}

CTrackLight::~CTrackLight()
{

}

void CTrackLight::CL_RunFrame()
{
    if(manager)
    {
        if(clent_src)
        {
            origin = clent_src->origin;
        } else if(edict_index) {
            origin = manager->client_entities[edict_index].lerp_origin;
        }
        if(intensity)
            manager->NetClient->V_AddLight(origin, intensity, red, green, blue);
    } else if(clent_src) {
        origin = clent_src->origin;
        if(intensity)
			clent_src->manager->NetClient->V_AddLight(clent_src->origin, intensity, red, green, blue);
    }

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

CClientEntity * CTrackLight::Alloc()
{
    return new CTrackLight;
}

void CTrackLight::ReadParametersFromServer(sizebuf_s * msg)
{
    CTrackingEntity::ReadParametersFromServer(msg);
    if(manager)
    {
        red = manager->NetClient->ReadFloat(msg);
        green = manager->NetClient->ReadFloat(msg);
        blue = manager->NetClient->ReadFloat(msg);
        intensity = manager->NetClient->ReadFloat(msg);
    }
}

void CTrackLight::SendParametersToClient()
{
    CTrackingEntity::SendParametersToClient();
    if(manager)
    {
        manager->NetServer->WriteFloat(red);
        manager->NetServer->WriteFloat(green);
        manager->NetServer->WriteFloat(blue);
        manager->NetServer->WriteFloat(intensity);
    }
}


void CTrackLight::LightValues(float fRed, float fGreen, float fBlue, float fIntensity)
{
    if(red != fRed || green != fGreen || blue != fBlue || intensity != fIntensity)
    {
        red = fRed;
        green = fGreen;
        blue = fBlue;
        intensity = fIntensity;
        if(manager && manager->server)
            Notify();
    }
}
