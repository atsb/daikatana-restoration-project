#include "MilesSound.h"
#include "Sample.h"

//// samples
CSample		CSample::samples[MAX_CHANNELS];
//CSample*	CSample::pSampFree;
//CSample*	CSample::pSampUsed;
int	CSample::index = 0;

#define SAMPLE_INUSE(pSamp)			(pSamp && ((pSamp)->h || (pSamp)->h3d))
///////////////////////////////////////////////////////////////////////////////
//
//  static function to initialize the samples
//
///////////////////////////////////////////////////////////////////////////////
void CSample::InitSamples()
{
	// setup free list	
/*
	pSampFree = &samples[0];
	for (int i=0;i<max_channels-1;i++)
	{		
		samples[i].pNext = NULL;//&samples[i+1];
	}
	// last link
	samples[max_channels].pNext = NULL;

	// setup used list
	pSampUsed = NULL;
*/
	CSample::Validate(false);
}

///////////////////////////////////////////////////////////////////////////////
//
//  static function to get a sample
//
///////////////////////////////////////////////////////////////////////////////
CSample* CSample::GetSample( sfx_t *sfx )
{
	CSample* pSamp = NULL;

	if (sfx)
	{
		for( int i = 0; i < max_channels; i++ )
		{
			if( samples[i].sfx == sfx )  
				return &samples[i];
		}
	}

	return NULL;
}

CSample* CSample::GetSample(int entnum, int entchannel, sfx_t* sfx, bool bAlloc3D)
{
	int i;

	// we want to see if this sfx already exists
	if (sfx)
	{
		for( i = 0; i < max_channels; i++ )
		{
			if( (samples[i].sfx == sfx) && (samples[i].entnum == entnum) && (samples[i].entchannel == entchannel) )  
				return &samples[i];
		}
	}

	// get next free sample
	CSample* pSamp = NULL;
	for( i = 0; i < max_channels; i++ )
	{
		if (!SAMPLE_INUSE(&samples[i]))
		{
			pSamp = &samples[i];
			pSamp->Init();
			break;
		}
	}

	// no sample!  Try to find one on the same entnum
	if ( (pSamp == NULL) && (entchannel != 0) )
	{
		for( i = 0; i < max_channels; i++ )
		{
			if ( (samples[i].entnum == entnum) && (samples[i].entchannel == entchannel) )
			{	
				// always override sound from same entity
				pSamp = &samples[i];
				break;
			}
		}

		if (pSamp) pSamp->Init();
	}

	if (pSamp)
	{
		// already have a handle?  return it!
		if (pSamp->h) 
		{
			if (using_3d && bAlloc3D)
			{
				AIL_release_sample_handle(pSamp->h);
				pSamp->h = NULL;
			}
			else
			{
				AIL_init_sample(pSamp->h);
				return pSamp;  
			}
		}

		// 3d handle
		if (pSamp->h3d) 
		{
			if (using_3d && bAlloc3D)
			{
				pSamp->is_3d = true;
				return pSamp;
			}
			else
			{
				AIL_release_3D_sample_handle(pSamp->h3d);
				pSamp->h3d = NULL;
			}			
		}

		// should we alloc a 3d sample?
		if (using_3d && bAlloc3D)
		{
			// allocate new one
			pSamp->h3d = AIL_allocate_3D_sample_handle(provider_3d);
			if (pSamp->h3d)
			{
				// return sample
				pSamp->is_3d = true;
				return pSamp;
			}
		}

		// allocate sample 
		pSamp->h = AIL_allocate_sample_handle(miles_driver);
		if (pSamp->h)
		{
			// init the sample
			AIL_init_sample(pSamp->h);
			// return sample
			return pSamp;
		}
	}
	
	return NULL;
}
/*
CSample* CSample::GetSample(int entnum, int entchannel, sfx_t* sfx, bool bAlloc3D)
{
	CSample* pSamp = NULL;
	
	// we want to see if this sfx already exists
	if (sfx)
	{
		for (pSamp = pSampUsed;pSamp;pSamp = pSamp->pNext)
		{
			// found it so send it back to them
			if (pSamp->sfx == sfx && pSamp->entnum == entnum && pSamp->entchannel == entchannel)  
				return pSamp;
		}
	}

	// get next free sample
	if (pSampFree)
	{
		pSamp = pSampFree;
		
		if (pSamp)
		{
			// relink chain
			pSampFree = pSamp->pNext;

			// initialize it
			pSamp->Init();

			// add to in use chain
			pSamp->pNext = pSampUsed;
			pSamp->pPrev = NULL;
			if (pSampUsed) pSampUsed->pPrev = pSamp;
			pSampUsed = pSamp;
		}
	}
	else
	{
		// no sample!
		for (pSamp = pSampUsed;pSamp;pSamp = pSamp->pNext)
		{
			if (entchannel != 0		// channel 0 never overrides
			&& pSamp->entnum == entnum
			&& pSamp->entchannel == entchannel)
			{	// always override sound from same entity
				break;
			}
		}
	
		// already in chain, so init it
		if (pSamp) pSamp->Init();
	}

	if (pSamp)
	{
		// already have a handle?  return it!
		if (pSamp->h) 
		{
			if (using_3d && bAlloc3D)
			{
				AIL_release_sample_handle(pSamp->h);
				pSamp->h = NULL;
			}
			else
			{
				AIL_init_sample(pSamp->h);
				return pSamp;  
			}
		}

		// 3d handle
		if (pSamp->h3d) 
		{
			if (using_3d && bAlloc3D)
			{
				pSamp->is_3d = true;
				return pSamp;
			}
			else
			{
				AIL_release_3D_sample_handle(pSamp->h3d);
				pSamp->h3d = NULL;
			}			
		}

		// should we alloc a 3d sample?
		if (using_3d && bAlloc3D)
		{
			// allocate new one
			pSamp->h3d = AIL_allocate_3D_sample_handle(provider_3d);
			if (pSamp->h3d)
			{
				// return sample
				pSamp->is_3d = true;
				return pSamp;
			}
		}

		// allocate sample 
		pSamp->h = AIL_allocate_sample_handle(miles_driver);
		if (pSamp->h)
		{
			// init the sample
			AIL_init_sample(pSamp->h);
			// return sample
			return pSamp;
		}
	}
	
	return NULL;
}
*/
///////////////////////////////////////////////////////////////////////////////
//
//  static function to release sample
//
///////////////////////////////////////////////////////////////////////////////
void CSample::ReleaseSample(CSample* pSamp)
{
	pSamp->Stop();

	// SCG[2/18/00]: Make sure we stop the sound from playing before we throw the sample into la la land
	if( pSamp->h != NULL )
	{
		if( AIL_sample_status( pSamp->h ) != SMP_DONE )
		{
			AIL_end_sample( pSamp->h );
		}
		AIL_release_sample_handle( pSamp->h );
		pSamp->h = NULL;
	}

	if( pSamp->h3d != NULL )
	{
		if( AIL_3D_sample_status( pSamp->h3d ) != SMP_DONE )
		{
			AIL_end_3D_sample( pSamp->h3d );
		}
		AIL_release_3D_sample_handle( pSamp->h3d );
		pSamp->h3d = NULL;
	}

	pSamp->Init();
/*
	// unlink from chain
	if (pSamp == pSampUsed) 
	{		
		pSampUsed = pSamp->pNext;
		if (pSampUsed) pSampUsed->pPrev = NULL;
	}
	else
	{
		if (pSamp->pPrev) pSamp->pPrev->pNext = pSamp->pNext;
		if (pSamp->pNext) pSamp->pNext->pPrev = pSamp->pPrev;
	}


	// add to free list
	pSamp->pNext = pSampFree;
	pSampFree = pSamp;
*/
}

///////////////////////////////////////////////////////////////////////////////
//
//  used for debugging
//
///////////////////////////////////////////////////////////////////////////////
void CSample::Validate(bool bOutput /* = false */)
{
	int free_count = 0, used_count = 0;
	for( int i = 0; i < max_channels; i++ )
	{
		if (SAMPLE_INUSE(&samples[i]))
			used_count++;
		else
			free_count++;
	}

/*
	int free_count = 0;
	if (pSampFree)
	{
		for (CSample* pSamp = pSampFree;pSamp;pSamp = pSamp->pNext)
			free_count++;
	}

	if (bOutput) ai.Com_Printf("There are %d free samples\n",free_count);
	
	int used_count = 0;
	if (pSampUsed)
	{
		for (CSample* pSamp = pSampUsed;pSamp;pSamp = pSamp->pNext)
			used_count++;
	}
*/
	if (bOutput) ai.Com_Printf("There are %d used samples\n",used_count);

	_ASSERTE((free_count+used_count) == max_channels);
}

void CSample::ReleaseAllSamples()
{
	for( int i = 0; i < max_channels; i++ )
	{
		if (SAMPLE_INUSE(&samples[i]))
		{
			CSample::ReleaseSample(&samples[i]);
		}
	}
}

/*
///////////////////////////////////////////////////////////////////////////////
//
//  static function to release all of the samples
//
///////////////////////////////////////////////////////////////////////////////
void CSample::ReleaseAllSamples()
{
	if (!pSampUsed) return;
	
	CSample* pNext = NULL;
	for (CSample* pSamp = pSampUsed;pSamp;pSamp = pNext)
	{
		// get next sample
		pNext = pSamp->pNext;
		// release miles handle
		// release current sample
		CSample::ReleaseSample(pSamp);
	}
}
*/

///////////////////////////////////////////////////////////////////////////////
//
//  static function to show what samples are in use
//
///////////////////////////////////////////////////////////////////////////////
void CSample::Status()
{
	ai.Com_Printf("Current sample status:\n");
	CSample *pSamp;
	for( int i = 0; i < max_channels; i++ )
	{
		pSamp = &samples[i];
		if (!SAMPLE_INUSE(pSamp))
		{
			ai.Com_Printf("%20s v: %0.2f s: %d\n",pSamp->sfx->name,pSamp->dist_volume,AIL_sample_status(pSamp->h));
		}
	}
/*
	for (CSample* pSamp = pSampUsed;pSamp;pSamp = pSamp->pNext)
	{
		ai.Com_Printf("%20s v: %0.2f s: %d\n",pSamp->sfx->name,pSamp->dist_volume,AIL_sample_status(pSamp->h));
	}
*/
}


///////////////////////////////////////////////////////////////////////////////
//
//  member function - spatializes leftvol and rightvol based on direction
//  and attenuation
//
///////////////////////////////////////////////////////////////////////////////
void CSample::SpatializeOrigin(CVector& origin)
{
    float		dot;
    float		dist;
	float		scale;
    CVector		source_vec;
	float		temp_dist;

	// full volume, dead center
	dist_volume = 1.0f;
	pan = 64;

	if (pcls->state != ca_active)
	{
		return;
	}

	// calculate stereo seperation and distance attenuation
	source_vec = origin - listener_origin;
	dist = temp_dist = source_vec.Normalize();

	if (dist > dist_max)
	{
		// too far
		dist_volume = 0.0f;
		return;
	}

	if (dist > dist_min)
	{
		// scale dist to 0.0 to 1.0	
		dist -= dist_min;
		dist = dist / (dist_max-dist_min);
	}
	else
	{
		dist = 0.0f;
	}

	// set volume
	dist_volume = (1.0 - dist);

	// no panning
	if (non_directional) return;

	// set panning
	dot = DotProduct(listener_right, source_vec);
	scale = 0.5 * (1.0 + dot);
	pan = scale * 127.0f;

	//ai.Com_Printf("[%d-%s] d: %0.2f  v: %0.2f  p: %0.2f\n",entnum,sfx->name,temp_dist,dist_volume,scale);
}

///////////////////////////////////////////////////////////////////////////////
//
//  member function - spatializes a sound, returns true if sound is
//	  still playing
//
////////////////////////////////////////////////////////////////////////////////
bool CSample::Spatialize()
{
	float	mastervol;

	// not for 3d samples
	if (is_3d) return true;

	// anything coming from the view entity will always be full volume
	if (entnum == pcl->playernum+1)
	{		
		// set max volume
		dist_volume = 1.0f;
		pan = 64;
	}
	else
	{
		SpatializeOrigin(position);	
	}

	if (!dist_volume)
	{
		// release sample
		CSample::ReleaseSample(this);
		return false;
	}

	// average out left/right then get a number between 1-127
	// SCG[8/5/99]: Volume for sounds are set here
//	mastervol = volume * dist_volume;
	mastervol = ( volume * dist_volume ) * s_volume->value;
	mastervol *= 127.0f;

	AIL_set_sample_volume(h,(int)mastervol);
	AIL_set_sample_pan(h,pan);

	return true;
}

CSample* CSample::GetUsedHead()
{
	index = -1;
	for( int i = 0; i < max_channels; i++ )
	{
		if (SAMPLE_INUSE(&samples[i]))
		{
			index = i;
			return &samples[i];
		}
	}

	return NULL;
}

CSample* CSample::GetUsedNext()
{
	if (index == -1)
		return NULL;

	for( int i = index + 1; i < max_channels; i++ )
	{
		if (SAMPLE_INUSE(&samples[i]))
		{
			index = i;
			return &samples[i];
		}
	}
	
	index = -1;
	return NULL;
}
