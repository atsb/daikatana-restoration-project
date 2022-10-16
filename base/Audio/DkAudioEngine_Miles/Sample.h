#ifndef __CLASS_SAMPLE__
#define __CLASS_SAMPLE__

class CSample
{
public:
	CSample() 
	{ 
		Init(); 
		h = NULL;
		h3d = NULL;
//		pNext = NULL;
//		pPrev = NULL;
	}
	
	void Init()
	{
		is_3d = false;
		sfx = NULL;
		entnum = 0;
		entchannel = 0;
		fixed_origin = false;
		origin.Set(0,0,0);
		volume = 0.0f;
		dist_min = dist_max = 0.0f;
		autosound = false;
		refreshed = false;
	}

	bool ReleaseIfDone()
	{
		if (is_3d)
		{
			if (AIL_3D_sample_status(h3d) != SMP_PLAYING)
			{
				CSample::ReleaseSample(this);
				return true;
			}
		}
		else
		{
			if (AIL_sample_status(h) != SMP_PLAYING)
			{
				CSample::ReleaseSample(this);
				return true;
			}
		}
		return false;
	}

	void Stop()
	{
		if (is_3d)
			AIL_stop_3D_sample(h3d);
		else
			AIL_stop_sample(h);
	}

	S32 SetData(void* data)
	{
		if (is_3d)
			return AIL_set_3D_sample_file(h3d,data);
		else
			return AIL_set_sample_file(h,data,0);
	}

	void SetPosition(CVector& pos)
	{
		if (is_3d) 
			AIL_set_3D_position(h3d,pos.x,pos.z,pos.y);
		else
		{
			position = pos;
			Spatialize();
		}
	}

	void SetVolume(float vol)
	{
		if (is_3d)
			AIL_set_3D_sample_volume(h3d,(int)(127.0f*vol));	
		else
			volume = vol;
	}

	void SetLoopCount(int count)
	{
		if (is_3d)
			AIL_set_3D_sample_loop_count(h3d,count);	
		else
			AIL_set_sample_loop_count(h,count);	
	}

	void SetDistance(float min, float max)
	{
		if (is_3d)
//			AIL_set_3D_sample_distances(h3d,max,min,max,min);
			// SCG[11/21/99]: v6.0a
			AIL_set_3D_sample_distances(h3d,max,min);
		else
		{
			dist_min = min;
			dist_max = max;
		}
	}

	void Start()
	{
		if (is_3d)
			AIL_start_3D_sample(h3d);
		else
			if (dist_volume) AIL_start_sample(h);
	}

	void UpdatePosition()
	{
		// get origin
		if (fixed_origin)
			SetPosition(origin);
		else			
		{
			CVector org;
			ai.CL_GetEntitySoundOrigin(entnum,org);
			SetPosition(org);
		}
	}

	bool Spatialize();
	void SpatializeOrigin(CVector& origin);

	bool		is_3d;			// true if sample is 3d hardware accel
	HSAMPLE		h;				// handle of miles sample
	H3DSAMPLE	h3d;			// handle of miles sample if 3d

	sfx_t*		sfx;
	int			entnum;			// to allow overriding a specific sound
	int			entchannel;		//

	bool		fixed_origin;	// use origin instead of fetching entnum's origin
	CVector		origin;			// only use if fixed_origin is set

	float		volume;			// 0.0 to 1.0 max volume level
	float		dist_min, dist_max;		// distances
	CVector		position;		// position of sound
	int			pan;			// left/right panning
	float		dist_volume;	// distance volume

	bool		autosound;		// from an entity->sound, cleared each frame
	bool		refreshed;		// true if the sound was refreshed and is an autosound
	bool		non_directional;// true if sound should not pan

//	CSample*	pNext;			// next sample in linked lists
//	CSample*	pPrev;			// previous sample in used list

	// code to play with samples
	static void InitSamples();
	static CSample* GetSample(int entnum, int entchannel, sfx_t* sfx, bool bAlloc3D);
	static CSample* GetSample( sfx_t *sfx );
	static void ReleaseSample(CSample* pSamp);
	static void Validate(bool bOutput = false);
	static void ReleaseAllSamples();
	static void Status();
	
	// inlines
//	static	CSample* GetUsedHead() { return pSampUsed; }
	static	CSample* GetUsedHead();
	static	CSample* GetUsedNext();
	static	int		 index;
private:
	static CSample	samples[MAX_CHANNELS];
//	static CSample*	pSampFree;		// free list is SINGLE linked
//	static CSample*	pSampUsed;		// used list is double linked
};


#endif