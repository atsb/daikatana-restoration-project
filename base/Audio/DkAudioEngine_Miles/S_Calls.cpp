#define AE_N0_DEFINE_EXTERNS

#include "MilesSound.h"
#include "Sample.h"

#define DLLEXPORT  __declspec( dllexport)

extern "C" DLLEXPORT void S_Play(void);
extern "C" DLLEXPORT void S_SoundList(void);
extern "C" DLLEXPORT void S_StopAllSounds(void);
extern "C" DLLEXPORT void S_Display3dProviders();
extern "C" DLLEXPORT void S_Test3dProvider();
extern "C" DLLEXPORT void S_Init (void* hWnd);
extern "C" DLLEXPORT void S_Shutdown(void);
extern "C" DLLEXPORT void S_BeginRegistration (void);
extern "C" DLLEXPORT void *S_RegisterSound (const char *name);
extern "C" DLLEXPORT void S_EndRegistration (int nFreeSequenceCode);
extern "C" DLLEXPORT void* S_StartSound(CVector& origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float dist_min, float dist_max);
extern "C" DLLEXPORT void* S_StartSoundQuake2(CVector& origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs);
extern "C" DLLEXPORT void S_StartStream(const char* file, float fvol);
DLLEXPORT void S_StopAllSounds(void);
extern "C" DLLEXPORT void S_PauseStreams( qboolean bOnOff );
extern "C" DLLEXPORT void S_Update(CVector &origin, CVector &forward, CVector &right, CVector &up);
extern "C" DLLEXPORT void S_StartLocalSound(const char *sound);
extern "C" DLLEXPORT void S_Display3dProviders();
extern "C" DLLEXPORT void S_Test3dProvider();
extern "C" DLLEXPORT void S_ReverbPreset (int nPresetID) ;
extern "C" DLLEXPORT void S_StopMP3(int channel);
extern "C" DLLEXPORT void S_StartMP3(char* name, int channel, float fvol, int play_count, qboolean bRestart = TRUE );
extern "C" DLLEXPORT void S_StartMusic(char* name, int channel);
extern "C" DLLEXPORT void S_StopMusic(int channel);

void S_ChangeMusicVolume(float vol);
void S_PlayMusic_f();
void S_Reload_f();
void S_CheckForWAVText(sfx_t *sfx);
void S_CheckForMP3Text(char *name, long datarate, long length);


void S_PlayMP3_f();
void S_Set3dProvider_f();
void S_Show3dProviders_f();

int			s_registration_sequence;
bool		snd_initialized = false;
int			sound_started = 0;
dma_t		dma;
CVector		listener_origin;
CVector		listener_forward;
CVector		listener_right;
CVector		listener_up;
qboolean	s_registering;
// during registration it is possible to have more sounds
// than could actually be referenced during gameplay,
// because we don't want to free anything until we are
// sure we won't need it.
#define		MAX_SFX		(MAX_SOUNDS*2)
sfx_t		known_sfx[MAX_SFX];
int			num_sfx;
cvar_t*		s_volume;
cvar_t*		s_khz;
cvar_t*		s_speeds;
cvar_t*		s_3d_provider;
cvar_t*		s_no_loop;
cvar_t*		s_music;
cvar_t*		s_cine;
cvar_t*		s_pause;
cvar_t*		s_debug;
cvar_t*		s_allow_uncached;
cvar_t*		s_dont_free_sounds;
cvar_t*		fs_cddir;

HDIGDRIVER	miles_driver;
int			s_rawend;
int			max_channels;
int			max_used;
int			max_cpu;
stream_t	streams[MAX_STREAMS];
mp3_t		mp3_channels[MAX_MP3_CHANNELS];

CVector vec3_zero(0,0,0);

// ====================================================================
// User-setable variables
// ====================================================================
void S_SoundInfo_f(void)
{
	long rate;
	long format;
	char desc[128];
	
	if (!sound_started)
	{
		ai.Com_Printf ("sound system not started\n");
		return;
	}
	
	AIL_digital_configuration(miles_driver,&rate,&format,desc);
	
	ai.Com_Printf("Device: %s\n",desc);
	ai.Com_Printf("Rate: %d\n",rate);
	ai.Com_Printf("%s %d bit\n",(format&DIG_F_STEREO_MASK) ? "Stereo" : "Mono", (format&DIG_F_16BITS_MASK) ? 16 : 8);
	ai.Com_Printf("Max Channels: %d\n\n",max_channels);
	ai.Com_Printf("Channels Used: %d\n",max_used);
	ai.Com_Printf("CPU Used: %d%%\n\n",max_cpu);
	
	if (using_3d)
	{
		ai.Com_Printf("3D Provider: %s\n\n",s_3d_provider->string);
	}
	
	// print validation info
	CSample::Validate(true);
}

void S_Status_f()
{
	CSample::Status();
}

void S_Init (void* hWnd)
{
	cvar_t	*cv;
	
	ai.Com_Printf("\n------- sound initialization -------\n");

	// cek[2-1-00]: removed per romero
//    ai.Com_Printf ("  Using AudioEngine:\n") ;
//    ai.Com_Printf ("    '%s'\n",         szAudioEngineDesc) ;
	
	cv = ai.Cvar_Get ("s_initsound", "1", CVAR_ARCHIVE);
	if (!cv->value)
		ai.Com_Printf ("not initializing.\n");
	else
	{
		s_volume			= ai.Cvar_Get("s_volume", "0.7", CVAR_ARCHIVE);
		s_khz				= ai.Cvar_Get("s_khz", "22", CVAR_ARCHIVE);
		s_speeds			= ai.Cvar_Get("s_speeds","0",0);
		s_debug				= ai.Cvar_Get("s_debug","0",0);
		s_3d_provider		= ai.Cvar_Get("s_3d_provider","Miles Fast 2D Positional Audio",CVAR_ARCHIVE);
		s_no_loop			= ai.Cvar_Get("s_no_loop","0",0);
		s_music				= ai.Cvar_Get("s_music","0.7",CVAR_ARCHIVE);
		s_cine				= ai.Cvar_Get("s_cine","0.7",CVAR_ARCHIVE);
		s_pause				= ai.Cvar_Get("paused","0",0);
		s_allow_uncached	= ai.Cvar_Get("s_allow_uncached","0",CVAR_ARCHIVE);
		s_dont_free_sounds	= ai.Cvar_Get( "s_dont_free_sounds","0" ,0 );
		fs_cddir			= ai.Cvar_Get( "cddir","0" ,0 );
		
		ai.Cmd_AddCommand("play", S_Play);
		ai.Cmd_AddCommand("stopsound", S_StopAllSounds);
		ai.Cmd_AddCommand("soundlist", S_SoundList);
		ai.Cmd_AddCommand("soundinfo", S_SoundInfo_f);
		ai.Cmd_AddCommand("s_stats",S_Status_f);
// SCG[6/28/00]: 		ai.Cmd_AddCommand("s_show_3d_providers",S_Show3dProviders_f);
// SCG[6/28/00]: 		ai.Cmd_AddCommand("show3d",S_Display3dProviders);
// SCG[6/28/00]: 		ai.Cmd_AddCommand("test3d",S_Test3dProvider);
		ai.Cmd_AddCommand("music",S_PlayMusic_f);
		ai.Cmd_AddCommand("soundreload",S_Reload_f);
		ai.Cmd_AddCommand("mp3",S_PlayMP3_f);
// SCG[6/28/00]: 		ai.Cmd_AddCommand("s_setprovider",S_Set3dProvider_f);
		
		if (Sound_Init())
		{
			sound_started = 1;
			num_sfx = 0;
			max_used = 0;
			max_cpu = 0;
			
			// init the samples
			CSample::InitSamples();
			
			memset(streams,0,sizeof(streams));
			
			S_StopAllSounds ();
		}
	}
	
	ai.Com_Printf("------------------------------------\n\n");
}

// =======================================================================
// Shutdown sound engine
// =======================================================================
void S_FreeSounds()
{
	int		i;
	sfx_t	*sfx;

	// free all sounds
	s_dont_free_sounds = ai.Cvar_Get( "s_dont_free_sounds","0" ,0 );

	if( s_dont_free_sounds->value == 1 )
	{
		for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
		{
			if (sfx->cache)
			{
				sfx->cache = NULL;
			}
			if (!sfx->name[0])
				continue;

			memset( sfx->name, 0, MAX_QPATH );
		}
	}
	else
	{
		for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
		{
			if( sfx->cache )
			{
				AIL_mem_free_lock( sfx->cache );
			}
			sfx->cache = NULL;

			if (!sfx->name[0])
				continue;

			memset( sfx->name, 0, MAX_QPATH );
		}
	}
	num_sfx = 0;
}
void S_Shutdown(void)
{
	int			i;
//	sfx_t*		sfx;
	mp3_t*		chan;

	// remove console commands
	ai.Cmd_RemoveCommand("play");
	ai.Cmd_RemoveCommand("stopsound");
	ai.Cmd_RemoveCommand("soundlist");
	ai.Cmd_RemoveCommand("soundinfo");
	ai.Cmd_RemoveCommand("s_stats");
	ai.Cmd_RemoveCommand("show3d");
	ai.Cmd_RemoveCommand("test3d");
	ai.Cmd_RemoveCommand("music");
	ai.Cmd_RemoveCommand("s_show_3d_providers");
	ai.Cmd_RemoveCommand("soundreload");
	ai.Cmd_RemoveCommand("mp3");
	ai.Cmd_RemoveCommand("s_setprovider");
	
	if (!sound_started)
		return;
	
	for (i=0, chan=mp3_channels;i<MAX_MP3_CHANNELS;i++, chan++)
	{
		if (chan->hMP3)
		{
			AIL_close_stream(chan->hMP3);
			chan->hMP3 = NULL;
			memset( chan->song, 0, MAX_QPATH );
		}
	}
	
	S_StopAllSounds();
	
	Sound_Shutdown();
	
	sound_started = 0;
	
	S_FreeSounds();
}

//=============================================================================
/*
==============
S_LoadSound
==============
*/
sfxcache_t* S_LoadSound (sfx_t* s)
{
    char			namebuffer[MAX_QPATH];
	byte*			data;
	AILSOUNDINFO	info;
	float			stepscale;
	sfxcache_t*		sc;
	int				size;
	
	if (s->name[0] == '*')
		return NULL;
	
	// see if still in memory
	sc = s->cache;
	if (sc) return sc;
	
	// load it in
	if (s->name[0] == '#')
		strcpy(namebuffer, &s->name[1]);
	else
		ai.Com_sprintf(namebuffer,sizeof(namebuffer),"sounds/%s",s->name);
	
	// load wave file
	size = ai.FS_LoadFile(namebuffer,(void**)&data);
	if (!data)
	{
		ai.Com_DPrintf ("S_LoadSound: Couldn't load %s\n", namebuffer);
		return NULL;
	}
	
	// get wave info from miles
	AIL_WAV_info(data,&info);
	
	if (info.channels != 1)
	{
		ai.Com_DPrintf ("%s is a stereo sample\n",s->name);
		ai.FS_FreeFile (data);
		return NULL;
	}
	
	stepscale = (float)info.rate / dma.speed;	
//	sc = s->cache = (sfxcache_t*)malloc(size+sizeof(sfxcache_t));
	sc = s->cache = (sfxcache_t*)AIL_mem_alloc_lock(size+sizeof(sfxcache_t));
	if (!sc)
	{
		ai.FS_FreeFile (data);
		return NULL;
	}
	
	sc->length = info.samples;
	sc->speed = info.rate;
	sc->width = info.bits;
	sc->stereo = info.channels;
	// copy over the data
	memcpy(sc->data,data,size);
	ai.FS_FreeFile(data);
	
	return sc;
}

// =======================================================================
// Load a sound
// =======================================================================
/*
==================
S_FindName
==================
*/
sfx_t* S_FindName(const char *name, qboolean create)
{
	int		i;
	sfx_t*	sfx;
	if (!name)
		ai.Com_Error (ERR_FATAL, "S_FindName: NULL\n");
	if (!name[0])
		ai.Com_Error (ERR_FATAL, "S_FindName: empty name\n");
	if (strlen(name) >= MAX_QPATH)
		ai.Com_Error (ERR_FATAL, "Sound name too long: %s", name);
	// see if already loaded
	for (i=0 ; i < num_sfx ; i++)
	{
		if (!strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}
	}
	
	if (!create)
		return NULL;
	// find a free sfx
	for (i=0 ; i < num_sfx ; i++)
	{
		if (!known_sfx[i].name[0])
			//			registration_sequence < s_registration_sequence)
			break;
	}
	if (i == num_sfx)
	{
		if (num_sfx == MAX_SFX)
			ai.Com_Error (ERR_FATAL, "S_FindName: out of sfx_t");
		num_sfx++;
	}
	
	sfx = &known_sfx[i];
	memset (sfx, 0, sizeof(sfx_t));
	strcpy (sfx->name, name);
	sfx->registration_sequence = s_registration_sequence;
	
	return sfx;
}


//=====================
//S_BeginRegistration
//=====================
void S_BeginRegistration (void)
{
	s_registration_sequence++;
	s_registering = true;
}

//==================
//S_RegisterSound
//==================
void *S_RegisterSound (const char *name)
{
	sfx_t	*sfx;
	if (!sound_started)
		return NULL;

	char filename[MAX_QPATH];
	strcpy( filename, name );

	sfx = S_FindName (strlwr( filename ), true);
//	sfx = S_FindName (strlwr( name ), true);
	sfx->registration_sequence = s_registration_sequence;
	if (s_debug->value && !s_registering && !sfx->cache)
	{
		ai.Com_Printf("Uncached Sound: %s\n",sfx->name);
//		if (s_allow_uncached->value) S_LoadSound(sfx);
	}
	return sfx;
}

//=====================
//S_EndRegistration
//=====================
void S_EndRegistration (int nFreeSequenceCode)
{
	int		i;
	sfx_t	*sfx;
	
	// free any sounds not from this registration sequence
	s_dont_free_sounds = ai.Cvar_Get( "s_dont_free_sounds","0" ,0 );

	if( s_dont_free_sounds->value == 1 )
	{
		for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
		{
			if (!sfx->name[0])
				continue;
			if (sfx->registration_sequence != s_registration_sequence)
			{	// don't need this sound
				memset (sfx, 0, sizeof(*sfx));
			}
		}
	}
	else
	{
		for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
		{
			if (sfx->registration_sequence != s_registration_sequence)
			{	// don't need this sound
				if (sfx->cache)				// it is possible to have a leftover
				{
					AIL_mem_free_lock( sfx->cache );	// from a server that didn't finish loading
					sfx->cache = NULL;
				}
				memset (sfx, 0, sizeof(*sfx));
			}
			if (!sfx->name[0])
				continue;
		}
	}

	
	// load everything in
	for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
	{
		if (!sfx->name[0])
			continue;
		
		// load the sound
		S_LoadSound (sfx);
	}
	s_registering = false;
}


/*
====================
S_StartSound

  Validates the parms and ques the sound up
  if pos is NULL, the sound will be dynamically sourced from the entity
  Entchannel 0 will never override a playing sound
  ====================
*/
void* S_StartSound(CVector& origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float dist_min, float dist_max)
{
	sfxcache_t	*sc;
	CSample*	samp;
	bool		use_3d;
	
	if (!sound_started)
		return 0;
	if (!sfx)
		return 0;
	
	// see if sound is loaded
	sc = sfx->cache;
	if (!sc)
	{
		return 0;
/*
		if (s_debug->value) ai.Com_DPrintf("S_StartSound: Not Cached: %s\n",sfx->name);
		if (s_allow_uncached->value)
		{
		sc = S_LoadSound(sfx);
		if (!sc) return 0;
		}
		else
		return 0;
		sc = S_LoadSound(sfx);
		if (!sc) return 0;
*/
	}
	
	// no attenuation or its a player sound with no position
	if ((!dist_min && !dist_max) || ((entnum == 1) && (origin.Length() == 0)))
		use_3d = false;
	else
		use_3d = true;
	
	// see if we wanted a looping sound
	if (entchannel == CHAN_LOOP)
	{
		samp = CSample::GetSample(entnum,entchannel,sfx,use_3d);	// looping sound, send sfx
	}
	else
	{
		samp = CSample::GetSample(entnum,entchannel,NULL,use_3d);	// don't send sfx here cause we want multiples to play
	}
	if (samp)
	{
		if (samp->SetData(sfx->cache->data))
		{			
			// set entnum and channel
			samp->entnum = entnum;
			samp->entchannel = entchannel;
			// set sfx
			samp->sfx = sfx;
			
			// get origin
			if (origin.Length() > 0 )
			{
				samp->origin = origin;
				samp->fixed_origin = true;
			}
			else
				samp->fixed_origin = false;
			
			// loop if necessary
			if (entchannel == CHAN_LOOP) 
				samp->SetLoopCount(0);	// infinite
			
			// set volume
			samp->SetVolume(fvol);
			// set distances		
			samp->SetDistance(dist_min,dist_max);
			
			// get origin
			if (!samp->fixed_origin)
			{
				ai.CL_GetEntitySoundOrigin(samp->entnum,origin);
			}	
			
			//S_CheckForWAVText(sfx);  // display WAV's text file
			
			// set sound position
			samp->SetPosition(origin);
			
			// start playing sample
			samp->Start();
		}
		else
		{
			ai.Com_DPrintf("S_StartSound: Error: %s [%s]\n",AIL_last_error(),sfx->name);
		}
	}
	else
	{
		ai.Com_DPrintf("S_StartSound: No free samples\n");
	}
	
    return sfx;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Old Start Sound for sound calls.  So we don't have to convert the
//  whole engine for this thing to work.
//
///////////////////////////////////////////////////////////////////////////////
void* S_StartSoundQuake2(CVector& origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs)
{
	float min, max;
	
	// set attenuation 
	if (attenuation == ATTN_IDLE)
	{
		min = 80.0f;
		max = 1000.0f;
	}
	else if (attenuation == ATTN_STATIC)
	{
		min = 64.0f;
		max = 224.0f;
	}
	else
	{
		min = ATTN_NORM_MIN;
		max = ATTN_NORM_MAX;
	}
	
	return S_StartSound(origin,entnum,entchannel,sfx,fvol,min,max);
}

void S_StartStream(const char* file, float fvol)
{
	char    buff[MAX_QPATH];
	
	stream_t* stream = NULL;
	
	// find an new stream and clear out old ones
	for (int z=0;z<MAX_STREAMS;z++)
	{
		// if we got one open see if its done
		if (streams[z].handle)
		{
			if (AIL_stream_status(streams[z].handle) == SMP_DONE)
			{
				AIL_close_stream(streams[z].handle);
				memset(&streams[z],0,sizeof(stream_t));
			}
		}
		else if (!stream)
		{
			stream = &streams[z];
		}
	}
	
	if (!stream) return;
	
	// open file
//	sprintf(buff,"data/sounds/%s",file);
	Com_sprintf(buff,sizeof( buff ), "data/sounds/%s",file);
	
	// open new stream
	stream->handle = AIL_open_stream(miles_driver,buff,0);
	
	if( stream->handle == NULL )
	{
		if( ( fs_cddir != NULL ) && ( fs_cddir->string[0] != NULL ) )
		{
			Com_sprintf( buff,sizeof( buff ), "%s/data/%s", fs_cddir->string, file);
			stream->handle = AIL_open_stream(miles_driver,buff,0);
		}
	}

	if (stream->handle)
	{
		// set the volume
		AIL_set_stream_volume(stream->handle,(int)(fvol*127.0f));
		// start the stream a playing
		AIL_start_stream(stream->handle);
	}
	else
	{
		ai.Com_DPrintf("Stream Error: %s\n",AIL_last_error());
	}
}


/*
===============================================================================
console functions
===============================================================================
*/
void S_Play(void)
{
	int 	i;
	char name[256];
	sfx_t	*sfx;
	
	i = 1;
	CVector vZero(0,0,0);
	while (i<ai.GetArgc())
	{
		if (!strrchr(ai.GetArgv(i), '.'))
		{
			strcpy(name, ai.GetArgv(i));
			strcat(name, ".wav");
		}
		else
			strcpy(name, ai.GetArgv(i));
		sfx = (sfx_t *)S_RegisterSound(name);
		
		S_StartSound(vZero, pcl->playernum+1, 0, sfx, 1.0f, 0.0f, 0.0f);
		
		i++;
	}
}

void S_SoundList(void)
{
	int		i;
	sfx_t	*sfx;
	sfxcache_t	*sc;
	int		size, count;
	long	total;
	
	total = count = 0;
	for (sfx=known_sfx, i=0 ; i<num_sfx ; i++, sfx++)
	{
		if (!sfx->registration_sequence)
			continue;
		sc = sfx->cache;
		count++;
		if (sc)
		{
			size = sc->length*sc->width*(sc->stereo+1);
			total += size;
			if (sc->loopstart > 0)
				ai.Com_Printf ("L");
			else
				ai.Com_Printf (" ");
			ai.Com_Printf("[%2dkhz] %6i : %s\n",(sc->speed/1000),  size, sfx->name);
		}
		else
		{
			if (sfx->name[0] == '*')
				ai.Com_Printf("  placeholder : %s\n", sfx->name);
			else
				ai.Com_Printf("  not loaded  : %s\n", sfx->name);
		}
	}
	ai.Com_Printf ("Total resident: %d (%lu bytes)\n", count, total);
}

/*
==================
S_StopAllSounds
==================
*/
DLLEXPORT void S_StopAllSounds(void)
{
	int			idx;
	stream_t*	stream;
	
	if (!sound_started)
		return;
	
	// release all samples (thus turning them off)
	CSample::ReleaseAllSamples();
	
	for (idx=0, stream = streams; idx < MAX_STREAMS; idx++, stream++)
	{
		if (stream->hfile)
		{
			CloseHandle(stream->hfile);
		}
		if (stream->handle)
		{
			AIL_close_stream(stream->handle);
		}
	}
	
	// clear all the streams
	memset(streams,0,sizeof(streams));
}

//=============================================================================
/*
============
S_Update
Called once each time through the main loop
============
*/

void S_PauseStreams( qboolean bOnOff )
{
	for (int i = CHAN_MUSIC_MAP; i < CHAN_CINE_6; i++ )
	{
		AIL_pause_stream(mp3_channels[i].hMP3, bOnOff );
	}
}

void S_UpdateSpatialization()
{
	// update spatialization for sounds
	CSample* pSamp = CSample::GetUsedHead();
	for (;pSamp;pSamp = CSample::GetUsedNext())
	{
		// get next sample
		// see if we can release these
		if (pSamp->ReleaseIfDone())
			continue;
		// dont process autosounds
		if (pSamp->autosound)
			continue;
		// get new position sound
		pSamp->UpdatePosition();
	}
}

void S_UpdateLoops(int &looping, int &sounds_2d, int &sounds_3d, int &sounds_active)
{
	// kill loops that we lost
	CSample *pSamp = CSample::GetUsedHead();
	for (;pSamp;pSamp = CSample::GetUsedNext())
	{
		// get next sample
		
		// see if we lost our loop
		if (pSamp->autosound)
		{
			if (pSamp->refreshed && !s_no_loop->value)
				pSamp->refreshed = false;
			else
			{
				// its done, kill it
				pSamp->Stop();
				CSample::ReleaseSample(pSamp);
				continue;
			}
			looping++;
		}
		if (pSamp->is_3d) 
			sounds_3d++;
		else
			sounds_2d++;
		sounds_active++;
	}
}

void S_Update(CVector &origin, CVector &forward, CVector &right, CVector &up)
{
	long		tick = GetTickCount();
	int			sounds_active, looping, sounds_2d, sounds_3d;
	int			cpu;
	
	if (!sound_started)
		return;
	
	sounds_active = looping = sounds_2d = sounds_3d = 0;
	
	// if the loading plaque is up, clear everything
	// out to make sure we aren't looping a dirty
	// dma buffer while loading
	//if (pcls->disable_screen || s_pause->value)
	
	// 9.1  checking the s_pause->value after a map is loaded is TRUE,
	//      so sounds in the interface would not update
	s_pause	= ai.Cvar_Get("paused","0",0);
	if (pcls->disable_screen || (s_pause->value && !ai.DKM_InMenu()) )
	{
		S_StopAllSounds();

		return;
	}

	// set the volume if modified
	// SCG[8/5/99]: This should be for a master volume control 
	// SCG[8/5/99]: as it affects al volumes...
	// SCG[8/5/99]: Volume is now set in the sample class in the Spatialize()
	// SCG[8/5/99]: member function
	/*	if (s_volume->modified)
	{
	s_volume->modified = false;
	AIL_set_digital_master_volume(miles_driver,s_volume->value*127);
	}
	*/
	// music volume modified?
	if (s_music->modified)
	{
		s_music->modified = false;
		//		S_ChangeMusicVolume();
		if( mp3_channels[CHAN_MUSIC_MENU].hMP3 )
			AIL_set_stream_volume(mp3_channels[CHAN_MUSIC_MENU].hMP3, ( int )( s_music->value * 127.0f ) );
		if( mp3_channels[CHAN_MUSIC_MAP].hMP3 )
			AIL_set_stream_volume(mp3_channels[CHAN_MUSIC_MAP].hMP3, ( int )( s_music->value * 127.0f ) );
	}
	
	if (s_cine->modified)
	{
		s_cine->modified = false;
		if( mp3_channels[CHAN_CINE_1].hMP3 )
			AIL_set_stream_volume( mp3_channels[CHAN_CINE_1].hMP3, ( int )( s_cine->value * 127.0f ) );
		if( mp3_channels[CHAN_CINE_2].hMP3 )
			AIL_set_stream_volume( mp3_channels[CHAN_CINE_2].hMP3, ( int )( s_cine->value * 127.0f ) );
		if( mp3_channels[CHAN_CINE_3].hMP3 )
			AIL_set_stream_volume( mp3_channels[CHAN_CINE_3].hMP3, ( int )( s_cine->value * 127.0f ) );
		if( mp3_channels[CHAN_CINE_4].hMP3 )
			AIL_set_stream_volume( mp3_channels[CHAN_CINE_4].hMP3, ( int )( s_cine->value * 127.0f ) );
		if( mp3_channels[CHAN_CINE_5].hMP3 )
			AIL_set_stream_volume( mp3_channels[CHAN_CINE_5].hMP3, ( int )( s_cine->value * 127.0f ) );
	}
	
	// -- origin may be null if we're in the interface code...
//	if (origin.Length() != 0) uhh, or if we happen to be...at 0,0,0...
// not to mention that this stuff still has to happen in the interface...
	{
		listener_origin = origin;
		listener_forward = forward;
		listener_right = right;
		listener_up = up;
		
		if (using_3d)
		{
			AIL_set_3D_position(listener,origin.x,origin.z,origin.y);
			AIL_set_3D_orientation(listener,listener_forward.x,listener_forward.z,listener_forward.y,listener_up.x,listener_up.z,listener_up.y);
		}
/*		
		// update spatialization for sounds
		CSample* pSamp = CSample::GetUsedHead();
		CSample* pNext = NULL;
		for (;pSamp;pSamp = CSample::GetUsedNext())
		{
			// get next sample
//			pNext = pSamp->pNext;
			// see if we can release these
			if (pSamp->ReleaseIfDone())
				continue;
			// dont process autosounds
			if (pSamp->autosound)
				continue;
			// get new position sound
			pSamp->UpdatePosition();
		}
*/		
		S_UpdateSpatialization();
		// add loopsounds
		if (!s_no_loop->value) S_AddLoopSounds();

		S_UpdateLoops(looping,sounds_2d,sounds_3d,sounds_active);
/*		
		// kill loops that we lost
		pSamp = CSample::GetUsedHead();
		pNext = NULL;		
		for (;pSamp;pSamp = CSample::GetUsedNext())
		{
			// get next sample
//			pNext = pSamp->pNext;
			
			// see if we lost our loop
			if (pSamp->autosound)
			{
				if (pSamp->refreshed && !s_no_loop->value)
					pSamp->refreshed = false;
				else
				{
					// its done, kill it
					pSamp->Stop();
					CSample::ReleaseSample(pSamp);
					continue;
				}
				looping++;
			}
			if (pSamp->is_3d) 
				sounds_3d++;
			else
				sounds_2d++;
			sounds_active++;
		}
*/
		
	} // if origin
	
	// profiling info - really need to remove this eventually
	if (s_speeds->value > 0)
	{
		cpu = AIL_digital_CPU_percent(miles_driver);
		if (sounds_active > max_used) max_used = sounds_active;
		if (cpu > max_cpu) max_cpu = cpu;
		
		ai.Com_Printf("SP: %d  2D: %d  3D: %d  LP: %d  CPU: %d%%  %dms\n", sounds_active, sounds_2d, sounds_3d, looping, cpu, GetTickCount()-tick);
	}
}

/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound(const char *sound) {
    static int channel = 0;
	sfx_t	*sfx;
	if (!sound_started)
		return;
	
	sfx = (sfx_t *)S_RegisterSound (sound);
	if (!sfx)
	{
		ai.Com_Printf ("S_StartLocalSound: can't cache %s\n", sound);
		return;
	}
	
    channel++;
    if (channel >= 5) channel = 0;
	
	S_StartSound(CVector(0,0,0), pcl->playernum+1, channel, sfx, 1.0f, 0.0f, 0.0f);
}

void S_Display3dProviders()
{
	HPROENUM	next;
	HPROVIDER	prov;
	char*		name;
	
	ai.Com_Printf("3D Providers:\n");
	
	next = HPROENUM_FIRST;
	// enum 3d providers
	while (AIL_enumerate_3D_providers(&next,&prov,&name))
	{
		ai.Com_Printf("  %s\n",name);
	}
}

void S_Test3dProvider()
{
	HPROVIDER	prov;
	char		provider[256];
	
	if (!ai.GetArgc()) return;
	
	strcpy(provider, ai.GetArgv(1));
	
	prov = Find3dProvider(provider);
	if (prov)
	{
		if (AIL_open_3D_provider(prov) == M3D_NOERR)
		{
			ai.Com_Printf("Opened %s\n",provider);
			AIL_close_3D_provider(prov);
			return;
		}
		else
		{
			ai.Com_DPrintf("Error: %s\n",AIL_last_error());
			return;
		}
	}
	
	ai.Com_Printf("Provider %s not found\n",provider);
	
	return;
}

void S_ReverbPreset (int nPresetID) 
{
	// these map directly to the EAX enum
	AIL_set_3D_provider_preference(provider_3d,"EAX environment selection",&nPresetID);
}

void S_ChangeMusicVolume(float vol)
{
	int i;
	mp3_t* chan; 
	
	for (i=0, chan=mp3_channels;i<MAX_MP3_CHANNELS;i++, chan++)
	{
		if (chan->hMP3)	AIL_set_stream_volume(chan->hMP3,(int)(vol*127.0f));
	}
}

void S_StopMP3(int channel)
{
	mp3_t*	chan;
	
	chan = &mp3_channels[channel];
	
	if (chan->hMP3) 
	{
		AIL_close_stream(chan->hMP3);
		chan->hMP3 = NULL;
		memset( chan->song, 0, MAX_QPATH );
	}
}

void S_StartMP3(char* name, int channel, float fvol, int play_count, qboolean bRestart )
{
	mp3_t*	chan;
	char	file[_MAX_PATH];
	long	datarate, length, sndtype, memptr;
	float	fVolume;
	
	datarate = 1;													 // assume 5 seconds default time for subtitle
	length   = 5;													 // (length / datarate)
	sndtype = memptr = 0;
	
	s_music = ai.Cvar_Get( "s_music", "", 0 );
	s_cine = ai.Cvar_Get( "s_cine", "", 0 );
	if( channel == CHAN_MUSIC_MAP )
	{
		fVolume = fvol * s_music->value;
	}
	else if( ( channel >= CHAN_CINE_1 ) && ( channel <= CHAN_CINE_6 ) )
	{
		fVolume = fvol * s_cine->value;
	}
	else
	{
		fVolume = fvol;
	}

	if (!sound_started)
	{
		S_CheckForMP3Text(name,datarate,length); // check for subtitles, regardless if sound is active
		
		return;
	}
	
	// check the channel
	if (channel > (MAX_MP3_CHANNELS-1))
	{
		ai.Com_DPrintf("ERROR: S_StartMP3: Bad channel %d",channel);
		return;
	}
	
	// get music channel
	chan = &mp3_channels[channel];

	if( bRestart == TRUE )
	{
		// close old stream is necessary
		if (chan->hMP3) 
		{
			AIL_close_stream(chan->hMP3);
			chan->hMP3 = NULL;
			memset( chan->song, 0, MAX_QPATH );
		}
	}
	else
	{
		if( stricmp( chan->song, name ) == 0 )
		{
			return;
		}
		else
		{
			AIL_close_stream(chan->hMP3);
			chan->hMP3 = NULL;
			memset( chan->song, 0, MAX_QPATH );
		}
	}
	
	// copy in name
	strncpy(chan->song,name,63);
	
	// get filename
// SCG[1/16/00]: 	sprintf(file,"data/%s",name);
	Com_sprintf(file,sizeof( file), "data/%s",name);
	
	// open new stream
	chan->hMP3 = AIL_open_stream(miles_driver,file,0);

	if( chan->hMP3 == NULL )
	{
		if( ( fs_cddir != NULL ) && ( fs_cddir->string[0] != NULL ) )
		{
			Com_sprintf( file,sizeof( file ), "%s/data/%s", fs_cddir->string, name );
			chan->hMP3 = AIL_open_stream(miles_driver,file,0);
		}
	}

	if (chan->hMP3)
	{
		// get mp3 information
		length   = chan->hMP3->totallen;
		datarate = chan->hMP3->datarate;
		
		// infinite looping
		AIL_set_stream_loop_count(chan->hMP3,play_count);			
		
		// set the volume
		AIL_set_stream_volume(chan->hMP3,(int)(fVolume*127.0f));
		
		// start the stream a playing
		AIL_start_stream(chan->hMP3);
	}
	else
	{
		ai.Com_DPrintf("ERROR: S_StartMP3: %s (%s)\n",AIL_last_error(),file);
	}
	
	S_CheckForMP3Text(file,datarate,length); // check for subtitles, regardless if sound is active
}


void S_StartMusic(char* name, int channel)
{
	if (!sound_started)
		return;
	
	mp3_t* chan = &mp3_channels[channel];
	
	char musicname[MAX_QPATH];
// SCG[1/16/00]: 	sprintf(musicname, "music/%s.mp3", name);
	Com_sprintf(musicname, sizeof( musicname ), "music/%s.mp3", name);
	
	if (!name || !_stricmp(musicname,chan->song))
	{
		// same song, lets resume
		if (chan->hMP3) AIL_pause_stream(chan->hMP3,0);
	}
	else
	{
		S_StartMP3( musicname, channel, s_music->value, 0 );
	}
}

void S_StopMusic(int channel)
{
	mp3_t* chan;
	
	if (channel > (MAX_MP3_CHANNELS-1))
	{
		ai.Com_DPrintf("ERROR: S_StopMusic: Bad music channel %d\n",channel);
		return;
	}
	
	// get channel
	chan = &mp3_channels[channel];
	// pause music
	if (chan->hMP3) AIL_pause_stream(chan->hMP3,-1);
}

void S_PlayMP3_f()
{
	int channel;
	char *name;
	
	if (ai.GetArgc() > 2)
	{
		name = ai.GetArgv(1);
		channel = atoi( ai.GetArgv(2) );
		S_StartMP3( name, channel,1.0, 1 );
	}
}

void S_PlayMusic_f()
{
	if (ai.GetArgc() > 1)
	{
		S_StartMusic(ai.GetArgv(1),CHAN_MUSIC_MAP);
	}
	else
	{
		S_StopMusic(CHAN_MUSIC_MAP);
	}
}

void S_Reload_f()
{
	int i;
	sfx_t* sfx;
	
	ai.Com_Printf("Reloading Sounds ...\n");
	
	S_FreeSounds();

	// load everything in
	for (i=0, sfx=known_sfx ; i < num_sfx ; i++,sfx++)
	{
		if (!sfx->name[0])
			continue;
		
		// load the sound
		S_LoadSound (sfx);
	}
	
	ai.Com_Printf("Sounds Reloaded.\n");
}



void S_CheckForWAVText(sfx_t *sfx)
// display sound's text file
{
	char buf[MAX_QPATH];
	char txt[1024];
	int file_len;
	FILE *fhandle;
	float display_seconds;
	
// SCG[1/16/00]: 	sprintf(buf,"subtitles/%s.txt",sfx->name);
	Com_sprintf(buf,sizeof( buf ), "subtitles/%s.txt",sfx->name);
	
	file_len = ai.FS_FOpenFile(buf,&fhandle);
	
	if (file_len && fhandle)  // valid file and open?
	{
		// determine time length of sound
		display_seconds = (float)sfx->cache->length / (sfx->cache->speed * sfx->cache->stereo);
		
		memset(&txt[0],0x00,sizeof(txt));  // reset text buffer
		
		if (file_len > sizeof(txt))		             // boundary checking
			file_len = sizeof(txt) - 1;
		
		ai.FS_Read(&txt[0],file_len,fhandle);
		
#ifndef JPN // changed by yokoyama : BUGFIX:NO.9
		ai.SCR_SubtitlePrint(txt,display_seconds);  // signal subtitle print / seconds to display
#else
		if(display_seconds > 5.0)
			ai.SCR_SubtitlePrint(txt,display_seconds);
		else
			ai.SCR_SubtitlePrint(txt,5.0);  // signal subtitle print / seconds to display
#endif
	}
	
}

#define TEXT_BUFFER_SIZE	( 1024 )
void S_CheckForMP3Text(char *name, long datarate, long length)
// display .mp3's text file
{
	char	buf[MAX_QPATH];
	char	txt[TEXT_BUFFER_SIZE];
	char	*token;
	int		txtfile_len;
	FILE	*fhandle;
	float	display_seconds;

	token = &name[strlen(name) - 1]; // start at back

	while (token && *token != '/')  // look for first slash separator
		token--;
	
	token++; // advance passed '/' separator
	
// SCG[1/16/00]: 	sprintf(buf,"subtitles/%s.txt",token);           // pre-fix subtitles directory
	Com_sprintf(buf,sizeof( buf ), "subtitles/%s.txt",token);           // pre-fix subtitles directory
	
	txtfile_len = ai.FS_FOpenFile(buf,&fhandle);     // open the file
	
	if (txtfile_len && fhandle && datarate)          // valid file and open?
	{
		// determine time length of sound
		display_seconds = (float)length / (float)datarate;
		
		memset(&txt[0],0x00,sizeof(txt));              // reset text buffer
		
		if (txtfile_len > sizeof(txt))		             // boundary checking
			txtfile_len = sizeof(txt) - 1;
		
		ai.FS_Read(&txt[0],txtfile_len,fhandle);

#ifndef JPN // changed by yokoyama : BUGFIX:NO.9
		ai.SCR_SubtitlePrint(txt,display_seconds);  // signal subtitle print / seconds to display
#else
		if(display_seconds > 5.0)
			ai.SCR_SubtitlePrint(txt,display_seconds);
		else
			ai.SCR_SubtitlePrint(txt,5.0);  // signal subtitle print / seconds to display
#endif
	}
}



