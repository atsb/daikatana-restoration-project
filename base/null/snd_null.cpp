// snd_null.c -- include this instead of all the other snd_* files to have
// no sound code whatsoever

#include "../client/client.h"

void S_Init (void)
{
}

void S_Shutdown (void)
{
}

void S_StartSound (vec3_t origin, int entnum, int entchannel, struct sfx_s *sfx, float fvol,  float attenuation, float timeofs)
{
}

void S_StartLocalSound (char *s)
{
}

void S_RawSamples (int samples, int rate, qboolean stereo, int bits, byte *data)
{
}

void S_StopAllSounds(void)
{
}

void S_Update (vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{
}

void S_Activate (qboolean active)
{
}

void S_BeginRegistration (void)
{
}

struct sfx_s *S_RegisterSound (char *sample)
{
	return NULL;
}

void S_EndRegistration (int nFreeSequenceCode)
{
}
