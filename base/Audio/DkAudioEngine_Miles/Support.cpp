#include	"MilesSound.h"
#include	"Sample.h"

///////////////////////////////////////////////////////////////////////////////
//
//  variables for support.cpp
//
///////////////////////////////////////////////////////////////////////////////

static qboolean		snd_firsttime = true; 
WAVEFORMATEX		wf;
HPROVIDER			provider_3d;
qboolean			using_3d = false;
H3DPOBJECT			listener;
bool				using_eax;

typedef struct provider_stuff
{
  char		*name;
  HPROVIDER	id;
} provider_stuff;

#define MAXPROVIDERS	64
HPROVIDER		providers[MAXPROVIDERS];
unsigned int	nProviderIndex, nProviderCount;
provider_stuff	pi[MAXPROVIDERS];

static int __cdecl comp(const void *p1,const void *p2)
{
	provider_stuff	*s1, *s2;
	s1 = ( provider_stuff * )p1;
	s2 = ( provider_stuff * )p2;

	return( _stricmp( s1->name, s2->name ) );
}

void S_Enumerate3dProviders()
{
	unsigned int	nCount = 0;
	HINTENUM		next = HINTENUM_FIRST;

	while( AIL_enumerate_3D_providers(&next, &pi[nCount].id, &pi[nCount].name) && ( nCount < MAXPROVIDERS ) )
	{
		++nCount;
	}

	qsort( pi, nCount, sizeof( pi[0] ), comp );

	for( int i = 0; i < nCount; i++ )
	{
		providers[i] = pi[i].id;
	}

	nProviderCount = nCount;
}

void S_Show3dProviders_f()
{
	for( int i = 0; i < nProviderCount; i++ )
	{
		if( i == nProviderIndex )
		{
			ai.Com_Printf("Current: -- " );
		}
		ai.Com_Printf("%d.  %s\n", i, pi[i].name );
	}

}

void S_Set3dProvider_f()
{
	nProviderIndex = Find3dProvider( "Miles Fast 2D Positional Audio" );
	if( ai.GetArgc() != 0 )
	{
		nProviderIndex = atoi( ai.GetArgv(1) );
		if( nProviderIndex < 0 )
		{
			nProviderIndex = 0;
		}
	}

	ai.Cvar_SetValue( "s_3d_provider", nProviderIndex );
}
// Init the audio system
HPROVIDER Find3dProviderFromIndex( unsigned int nProvider );
int Sound_Init(void)
{
	int err;
	char desc[128];
	char version[128];
	int result;
	bool failed_3d;
	char provider[256];
	
	// get special miles cvars
	cvar_t* wave_only   = ai.Cvar_Get("s_wavonly","0",CVAR_ARCHIVE);
	cvar_t* use_wave	= ai.Cvar_Get("s_use_waveout","1",CVAR_ARCHIVE);
	cvar_t* use_mss_mix = ai.Cvar_Get("s_use_mss_mixer","1",CVAR_ARCHIVE);
	cvar_t* max_chan	= ai.Cvar_Get("s_max_channels","32",CVAR_ARCHIVE);
	cvar_t* use3d		= ai.Cvar_Get("s_use3d","1",CVAR_ARCHIVE);

	// init miles audio engine
	AIL_startup();
	
	if( use3d->value )
	{
		S_Enumerate3dProviders();
	}

	// select format
	memset (&dma,0,sizeof(dma));
	dma.channels = 2;
	dma.samplebits = 16;
	if (s_khz->value == 44)
		dma.speed = 44100;
	else
	{
		if (s_khz->value == 22)
			dma.speed = 22050;
		else
			dma.speed = 11025;
	}
	
	// setup waveformat struct
	memset (&wf,0,sizeof(wf));
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = dma.channels;
	wf.nSamplesPerSec = dma.speed;
	wf.nAvgBytesPerSec = dma.speed * (dma.samplebits/8) * dma.channels;
	wf.nBlockAlign = (dma.samplebits/8) * dma.channels;
	wf.wBitsPerSample = dma.samplebits;
	wf.cbSize = 0;
	
	ai.Com_Printf("Channels: %d, Bits: %d, Rate: %d\n",dma.channels,dma.samplebits,dma.speed);
	
	// get DLL version
	AIL_DLL_version(version,sizeof(version));
	ai.Com_Printf("Miles Audio v%s\n",version);
	
	// set maximum channels
	max_channels = max_chan->value;
	ai.Com_Printf("Maximum Samples: %d\n",max_channels);
	
	// no wave out yet
	AIL_set_preference(DIG_USE_WAVEOUT,wave_only->value ? YES : NO);
	// use the miles mixer (faster than DX mixer)
	// SCG[11/21/99]: does not exist in 6.0a
	//	AIL_set_preference(DIG_USE_MSS_MIXER,use_mss_mix->value ? YES : NO);
	// set mixer channels
	AIL_set_preference(DIG_MIXER_CHANNELS,(max_channels*2));
	
	// opens audio system 	
	err = AIL_waveOutOpen(&miles_driver,0,WAVE_MAPPER,(LPWAVEFORMAT)&wf);
	// try again using wave out if we haven't already			
	if (use_wave->value && !wave_only->value) 
	{
		if (!err)
		{
			AIL_digital_configuration(miles_driver,0,0,desc);
			if (strstr(desc,"Emulated") == NULL) goto audiook;
			// emulated DS, switch to waveout
			AIL_waveOutClose(miles_driver);
		}
		
		// allow waveout
		AIL_set_preference(DIG_USE_WAVEOUT,YES);
		
		// try to open audio system once again
		err = AIL_waveOutOpen(&miles_driver,0,WAVE_MAPPER,(LPWAVEFORMAT)&wf);
	}
	
audiook:			
	// error in init, report it
	if (err)
	{
		// some sort of error
		ai.Com_DPrintf("Miles Init Error: %s\n",AIL_last_error());
		return 0;
	}
	
	// setup volume
	//	AIL_set_digital_master_volume(miles_driver,s_volume->value*127);
	AIL_set_digital_master_volume(miles_driver,127);
	
	snd_firsttime = false;
	provider_3d = NULL;
	using_3d = false;
	using_eax = false;
	failed_3d = false;
	
	// setup 3d sound if we want to use it
//	if (use3d->value)
	if (0)
	{
		ai.Com_Printf("\nInitializing 3D Sound Engine ...\n");
		
		strcpy(provider,s_3d_provider->string);
		
retry3d:
		ai.Com_Printf("%s: ",provider);
		// get the provider
		provider_3d = Find3dProvider(provider);
		if( provider_3d == NULL )
		{
			provider_3d = Find3dProviderFromIndex( atoi( provider ) );
		}

		if (provider_3d)
		{
			AIL_set_3D_provider_preference(provider_3d,"Maximum supported samples",&max_channels);			
			if (AIL_open_3D_provider(provider_3d) == M3D_NOERR)
			{	
				using_3d = true;			
				listener = AIL_3D_open_listener(provider_3d);
				
				ai.Com_Printf("Success\n");
				
				AIL_3D_provider_attribute(provider_3d,"EAX environment selection",&result);
				if (result != -1)
				{
					using_eax = true;
					ai.Com_Printf("EAX Extensions Enabled\n");
				}
				
				AIL_3D_provider_attribute(provider_3d,"Maximum supported samples",&result);
				ai.Com_Printf("\nMaximum 3D Samples: %d\n",result);
			}
			else
			{
				if (failed_3d)
				{
					provider_3d = NULL;
					ai.Com_Printf("3D Sound Engine Failed to Initialize!\n");
				}
				else
				{
					failed_3d = true;
					provider_3d = NULL;
					ai.Com_Printf("Failed\n");
					strcpy(provider,"Miles Fast 2D Positional Audio");
					goto retry3d;
				}
			}
		}
	}
	else
	{
		ai.Com_Printf("Not Initializing 3D Sound Engine.\n");
	}

	return 1;
}

void Sound_Shutdown()
{
	// close out 3d
	if (using_3d && provider_3d)
	{
		AIL_3D_close_listener(listener);
		AIL_close_3D_provider(provider_3d);
	}
	
	// close wave out
	AIL_waveOutClose(miles_driver);
	
	// kill miles audio
	AIL_shutdown();
}

/*
==================
S_AddLoopSounds
Entities with a ->sound field will generated looped sounds
that are automatically started, stopped, and merged together
as the entities are sent to the client
==================
*/
void S_AddLoopSounds (void)
{
	int			i;
	int			sounds[MAX_EDICTS];
	sfx_t		*sfx;
	sfxcache_t	*sc;
	int			num;
	entity_state_t	*ent;
	CVector		origin;
	float		min, max;
	CSample*	samp;
	
	if (pcls->state != ca_active)
		return;
	
	if (!pcl->sound_prepped)
		return;
	
	for (i=0 ; i<pcl->frame.num_entities ; i++)
	{
		num = (pcl->frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &pcl_parse_entities[num];
		sounds[i] = ent->sound;
	}
	for (i=0 ; i<pcl->frame.num_entities ; i++)
	{
		if (!sounds[i])
			continue;
		sfx = (sfx_t *)pcl->sound_precache[sounds[i]];
		if (!sfx)
			continue;		// bad sound effect
		sc = sfx->cache;
		if (!sc)
			continue;
		
		num = (pcl->frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &pcl_parse_entities[num];
		
		// allocate a sample
		if (ent->snd_flags & SND_NONDIRECTIONAL)
			samp = CSample::GetSample(ent->number,0,sfx,false);
		else
			samp = CSample::GetSample(ent->number,0,sfx,true);
		
		if (!samp) continue;
		
		samp->refreshed  = true;	// don't remove this frame
		// get origin
		ai.CL_GetEntitySoundOrigin(ent->number,origin);
		
		// new sample?
		if (samp->sfx != sfx)
		{
			samp->sfx = sfx;
			samp->autosound		= true;		// remove next frame
			
			if (samp->SetData(sfx->cache->data))
			{
				samp->entnum = ent->number;
				// set non directional flag
				if (ent->snd_flags & SND_NONDIRECTIONAL)
					samp->non_directional = true;
				// infinite looping
				samp->SetLoopCount(0);
				// set volume level
				if (ent->volume)
					samp->SetVolume(ent->volume);
				else
					samp->SetVolume(1.0f);
				// set distance values
				if (ent->dist_min) min = ent->dist_min; else min = ATTN_NORM_MIN;
				if (ent->dist_max) max = ent->dist_max; else max = ATTN_NORM_MAX;
				// set attenuation
				samp->SetDistance(min,max);
				// set position
				samp->SetPosition(origin);
				// start sample
				samp->Start();
			}
			else
			{
				ai.Com_DPrintf("S_AddLoopSounds: Error: %s [%s]\n",AIL_last_error(),sfx->name);
			}
		}
		else
		{
			// set position
			samp->SetPosition(origin);
		}
	}
}

// grab a provider handle by name
HPROVIDER Find3dProviderFromIndex( unsigned int nProvider )
{
	if( ( nProviderCount != 0 ) && ( nProvider < nProviderCount ) )
	{
		return providers[nProvider];
	}

	return NULL;
}

HPROVIDER Find3dProvider(char* provider)
{
	HPROENUM	next;
	HPROVIDER	prov;
	char*		name;
	
	next = HPROENUM_FIRST;
	// enum 3d providers
	while (AIL_enumerate_3D_providers(&next,&prov,&name))
	{
		if (!strcmp(name,provider))
		{
			return prov;
		}
	}
	
	return NULL;
}
