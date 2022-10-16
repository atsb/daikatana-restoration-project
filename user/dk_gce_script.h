//#pragma once

#ifndef _DK_GCE_SCRIPT_H
#define _DK_GCE_SCRIPT_H

class CSplineSequence;
class CScriptSound;
class CScriptEntity;
class CVector;

#include "dk_array.h"
#include "dk_buffer.h"

typedef enum {
    CT_SEQUENCE,    //camera points in the direction stored in the sequence.
    CT_ENTITY,      //camera points at an entity.
} camera_target;

typedef enum {
    SE_SEQUENCE,    //the shot ends when the sequence is done playing.
    SE_ENTITY,      //we wait for a entity to reach it's destination before the shot ends.
} shot_end;

class CScriptSound {
  public:
    CScriptSound();
    ~CScriptSound();

//	void* CScriptSound::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CScriptSound::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CScriptSound::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CScriptSound::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
    void operator=(const CScriptSound &other);

    //this command can only be run in the dll
    void Save(FILE *file) const;
    
    //can be run from the exe only
    void Load(FILE *file, int version);

  protected:
    //the name of wave file we play
    char filename[64];

    //whether we loop or not
    bool loop;

    //the time we play this sound, as an offset from the start of the shot.
    float time;

	int channel;

  public:
    //
    //member access functions
    //
    const char *Name() const;
    bool Loop() const;
    float Time() const;
	int	Channel() const;

    //compare function used for sorting, etc.
    static int Compare(const CScriptSound **e1, const CScriptSound **e2);

    //
    //functions for modifying the sound
    //
    void Name(const char *name);
    void Loop(const bool val);
    void Time(float time);
	void Channel(int chan);
};

//data that is used to describe what happens during each shot of the script.
class CScriptShot {
  public:
    CScriptShot();
    ~CScriptShot();

//	void* CScriptShot::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CScriptShot::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CScriptShot::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CScriptShot::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const CScriptShot &other);

    //this command can only be run in the dll
    void Save(FILE *file) const;
    //can be run from the exe only
    void Load(FILE *file, int version);

  protected:
    //our spline sequence that the camera follows for this shot.
    CSplineSequence *sequence;

    //what the camera looks at during the shot.
    camera_target target_flag;

    //if the camera is supposed to look at an entity, we put the entity's name here.
    buffer16 entity_camera_target;

    //desciption for knowing when we are done with this shot
    shot_end end_flag;

	// here's the sky number to use for the shot
	int sky;

    //extra time we spend sitting at the beginning and end of the sequence.
    float extra_time_start;
    float extra_time_end;

    //if we end the shot when an enemy finishes it's tasks, we put the name 
    //of the entity here
    buffer16 entity_shot_end;

    //sounds that we play during this shot.
    array<CScriptSound> sounds;

    //things that entities do during this shot.
    array<CScriptEntity> entities;

    //true if we have a camera fov specified for the beginning of the shot.
    bool have_start_fov;
    //the shot's initial fov.
    float start_fov;
    
    //true if we have a camera timescale specified for the beginning of the shot.
    bool have_start_timescale;
    //the shot's initial timescale.
    float start_timescale;
    
    //true if we have a camera fade color specified for the beginning of the shot.
    bool have_start_color;
    //the shot's initial color.
    float start_r;
    float start_g;
    float start_b;
    float start_a;


    //events we perform during this shot.

    //
    //vars we use to keep track of our playback status.
    //
    int next_sound;
    //when we are waiting for an entity to complete it's tasks, then for the extra_time_end, we use these
    bool wait_entity_done;
    int wait_entity_done_time;

  public:
    //
    //member access functions.
    //
    const CSplineSequence *Sequence() const;
	int Sky() const;
    float ExtraTimeStart() const;
    float ExtraTimeEnd() const;
    const array<CScriptSound> &Sounds() const;
    float TotalTime() const;
    const array<CScriptEntity> &Entities() const;
    camera_target CameraTarget() const;
    shot_end EndFlag() const;
    const char *EntityCameraTarget() const;
    const char *EntityShotEnd() const;
    bool HaveStartFOV() const;
    float StartFOV() const;
    bool HaveStartTimescale() const;
    float StartTimescale() const;
    bool HaveStartColor() const;
    float StartR() const;
    float StartG() const;
    float StartB() const;
    float StartA() const;

    //
    //Playback
    //
    void PlaybackReset();
    //plays back all events starting before end_time.
    //if the shot is over, this returns the number of ms that were not used.
    //if ths shot is not over, it will return -1.
    int Playback(int end_time);
    void PlaybackCamera(CVector &pos, CVector &dir, float time);

    //
    //functions for modifying the shot.
    //
	void Sky(int sky);

    void ExtraTimeStart(float time);
    void ExtraTimeEnd(float time);

    void SetSequence(const CSplineSequence *sequence);
    void SetSounds(const array<CScriptSound> &sounds);
    void SetEntities(const array<CScriptEntity> &entities);

    void CameraTarget(camera_target target);
    void EntityCameraTarget(const char *entity_name);
    void EndFlag(shot_end end_flag);
    void EntityShotEnd(const char *entity_name);
    void HaveStartFOV(bool flag);
    void StartFOV(float fov);
    void HaveStartTimescale(bool flag);
    void StartTimescale(float timescale);
    void HaveStartColor(bool flag);
    void StartColor(float r, float g, float b, float a);
};

class CCinematicScript {
  public:
    CCinematicScript();
    ~CCinematicScript();

//	void* CCinematicScript::operator new (size_t size) { _ASSERTE(memmgr.X_Malloc); return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CCinematicScript::operator new[] (size_t size) { _ASSERTE(memmgr.X_Malloc); return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CCinematicScript::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CCinematicScript::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const CCinematicScript &other);

    //this command can only be run in the dll
    void Save(FILE *file) const;
    //can be run from the exe only
    void Load(FILE *file, int version, const char *name);

    //when the script loads or is modified, we can use this function to determine if it has valid camera information.
    void CheckCameraInfo();

  protected:
    //our array of shots.
    array<CScriptShot> shots;

    //the name of the script.
    buffer64 name;

    //the name of the script when we loaded off disk.
    buffer64 disk_name;

    //false if this script will not be controling the camera in any way.
    bool have_camera_info;

    //
    //playback vars.
    //
    int cur_shot;
    int end_shot;
    int shot_time;

  public:
    //
    //member access funcitons.
    //
    const char *Name() const;
    const char *DiskName() const;
    int NumShots() const;
    const CScriptShot *Shot(int num) const;
    bool HaveCameraInfo() const;

    //
    //playback functions
    //
    void PlaybackReset(int start_shot, int end_shot);
    void PlaybackTimeElapsed(int ms);
    bool PlaybackDone();
    void PlaybackCamera(CVector &pos, CVector &dir);
	void RegsterCinematicSounds();

    //returns true if the script moves the main character around.
    bool PlaybackMovesClient();
    

    //
    //functions for modifying the script.
    //
    void Name(const char *name);

    //adds a shot to the end of the script.
    void AddShot();

    //inserts a shot at the given index.
    void InsertShot(int position);

    //gets a pointer to the shot that we can modify.
    CScriptShot *GetShot(int num);

    //delete's the given shot from the script.
    void DeleteShot(CScriptShot *shot);
};

#endif // _DK_GCE_SCRIPT_H





