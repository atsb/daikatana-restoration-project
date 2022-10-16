// Sound.cpp: implementation of the CSound class.
//
//
//////////////////////////////////////////////////////////////////////
#include "sound.h"
#include "snd_loc.h"
#include "SoundFX.h"
#include "ClientEntityManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSound::CSound()
{

}

CSound::~CSound()
{

}

CSound::CSound(class CClientEntity * clent, char * name)
{
    ent_sibling = clent;
    sound_name = name;
    sfx = RegisterSound(name);  
}

struct sfx_s * CSound::RegisterSound(const char * name)
{
    if(!ent_sibling)
        return NULL;
    if(!ent_sibling->manager)
        return NULL;
    if(!ent_sibling->manager->NetClient)
        return NULL;
    return (sfx_t *)ent_sibling->manager->NetClient->S_RegisterSound(name);
}

void CSound::Play()
{

}

void CSound::Play(CVector & origin)
{

}
