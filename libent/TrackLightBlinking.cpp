// TrackLightBlinking.cpp: implementation of the CTrackLightBlinking class.
//
//////////////////////////////////////////////////////////////////////

#include "ClientEntityManager.h"
#include "TrackLightBlinking.h"
#include "TrackFlare.h"
#include "p_user.h"
#include "client.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrackLightBlinking::CTrackLightBlinking()
{
    

    pFlare = NULL;    
    fBaseIntensity = 0;
    fBlinkRate = 0;
    fLastBlinkTime = 0;
    
}

CTrackLightBlinking::~CTrackLightBlinking(){}

void CTrackLightBlinking::CL_RunFrame()
{
    if(manager)
    {
        // Time to toggle the light?
        if(fLastBlinkTime + fBlinkRate < manager->current_time)
        {
            fLastBlinkTime = manager->current_time;
            if(intensity)
            {
                intensity = 0;
            } else {
                intensity = fBaseIntensity;
                pFlare->CL_RunFrame();
            }
        } else if(fLastBlinkTime + fBlinkRate + 10 > manager->current_time) {
            intensity = 0;
        }
    }
    
    CTrackLight::CL_RunFrame();
}

void CTrackLightBlinking::ReadParametersFromServer(sizebuf_s * msg)
{
    CVector flarescale;
    float   fScale;

    CTrackLight::ReadParametersFromServer(msg);
    if(manager)
    {
        fBaseIntensity = manager->NetClient->ReadFloat(msg);
        fBlinkRate = manager->NetClient->ReadFloat(msg);
        fScale = 1;
        flarescale.Set(fScale, fScale, fScale);
        if(!pFlare)
            pFlare = new CTrackFlare("models/e1/we_c4f.sp2", flarescale, this);
        pFlare->model->alpha = 1.0;
        
    }
}

void CTrackLightBlinking::SendParametersToClient()
{

    CTrackLight::SendParametersToClient();
    if(manager)
    {
        manager->NetServer->WriteFloat(fBaseIntensity);
        manager->NetServer->WriteFloat(fBlinkRate);
    }

}

CClientEntity * CTrackLightBlinking::Alloc()
{
    return new CTrackLightBlinking;

}

void CTrackLightBlinking::LightValues(float fRed, float fGreen, float fBlue, float fIntense, float fBlink)
{

    // if something has changed, send an update
    if( red != fRed || green != fGreen || fBlue != blue || intensity != fIntense || fBaseIntensity != fIntense || fBlinkRate != fBlink)
    {
        CTrackLight::LightValues(fRed, fGreen, fBlue, fIntense);
        fBaseIntensity = fIntense;
        fBlinkRate = fBlink;
        if( manager )
            Notify();
    }
}
