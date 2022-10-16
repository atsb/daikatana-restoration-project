//#pragma once

#ifndef _DK_GCE_ENTITIES_H
#define _DK_GCE_ENTITIES_H

class CEntityHeadScript;

//an enum to specify the different tasks.
typedef enum {
    TT_NONE,
    TT_MOVE_TO,
    TT_TURN,
    TT_MOVE_AND_TURN,

    TT_ATTRIBUTE_BACKUP,
    TT_ATTRIBUTE_RESTORE,
    TT_ATTRIBUTE_RUN_SPEED,
    TT_ATTRIBUTE_WALK_SPEED,
    TT_ATTRIBUTE_YAW_SPEED,

    TT_WAIT,
    TT_TELEPORT,

    TT_START_RUNNING,
    TT_START_WALKING,

    TT_TRIGGER_BRUSH_USE,

    TT_HEAD_SCRIPT,

    TT_PLAY_ANIMATION,
    TT_SET_IDLE_ANIMATION,
    TT_PLAY_SOUND,

	TT_SPAWN_ENTITY,
	TT_REMOVE_ENTITY,

	TT_CLEAR_GOALS,

} task_type;

class CScriptEntityTask {
public:
    CScriptEntityTask();
    ~CScriptEntityTask();
	
//	void* CScriptEntityTask::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CScriptEntityTask::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CScriptEntityTask::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CScriptEntityTask::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const CScriptEntityTask &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

protected:
    //the type of task 
    task_type type;

    //when the task is started.  a time of -1 means that the the task starts as soon as the previous task
    //is completed.
    float time;

    //a point that the entity will move to
    CVector destination;

    //a direction to face.
    CVector direction;

    //used to set the value of an attribute.
    float attribute_value;

    //for head script tasks, we keep the info here.
    CEntityHeadScript *headscript;

    //for animation names.
    buffer16 animation_name;

    //for using an entity
    buffer16 use_entity_name;

	buffer16 sound_name;

	buffer32 uniqueid;

	float sound_duration;


public:
    //
    //member access
    //
    task_type Type() const;
    float Time() const;
    const CVector &Destination() const;
    const CVector &Direction() const;
    const char *TypeString() const;
    float AttributeValue() const;
    const char *AnimationName() const;
	const char *SoundName() const;
	const float SoundDuration() const;
    const CEntityHeadScript *HeadScript() const;
    const char *UseEntityName() const;

    //
    // functions to modify the script.
    //
    //one function for each of the task types.
    //task types for entities with a task queue.
    void MoveTo(const CVector &dest);
    void Turn(const CVector &dir);
    void MoveAndTurn(const CVector &dest, const CVector &dir);
    void AttributeBackup();
    void AttributeRestore();
    void AttributeYawSpeed(float yaw_speed);
    void AttributeRunSpeed(float run_speed);
    void AttributeWalkSpeed(float walk_speed);
    void Wait(float time);
    void Teleport(const CVector &dest, const CVector &dir);
    void StartWalking();
    void StartRunning();
    void HeadScript(const CEntityHeadScript *headscript);
    void PlayAnimation(const char *name);
    void SetIdleAnimation(const char *name);
    void PlaySound(const char *name, const float duration);
	void SpawnEntity( const char *id, const CVector& dest, const CVector& dir );
	void RemoveEntity( const char *id );
	void ClearGoals( const char *id, float total_time );

    //task types for trigger brushes.
    void TriggerBrushUse();

    //for the attribute functions, we can use this to set the value of the attribute.
    void AttributeValue(float value);

    //for setting the animation name for the animation type tasks.
    void AnimationName(const char *name);

    //for setting the sound name
    void SoundName(const char *name);

    //for setting the sound duration
    void SoundDuration(const float duration);
    
    //for setting the entity name for the entity to use
    void UseEntityName(const char *name);
    
    //to set the time the task will execute.
    void Time(float time);

	// for getting the unique id
	const char *UniqueID() const;

	// for setting the unique id
	void UniqueID( const char *id );
};

typedef array<CScriptEntityTask> task_list;

class CScriptEntity {
public:
    CScriptEntity();
    ~CScriptEntity();

//	void* CScriptEntity::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CScriptEntity::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CScriptEntity::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CScriptEntity::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const CScriptEntity &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

protected:
    //the name of the entity.
    buffer32 name;

    //each entity has a list of tasks that it must perform
    task_list tasks;

	buffer32	uniqueid;

    //
    //playback vars
    //
    int next_task;
    //the time that the current task was started.
    int current_task_start_time;
    //the task number that went with the above time.
    int saved_task_num;

public:
    //
    //member access
    //
    const char *Name() const;
    const task_list &Tasks() const;

    //compare functions used for sorting, etc.
    static int Compare(const CScriptEntity **e1, const CScriptEntity **e2);
    static int Find(const CScriptEntity *entity, const void *identifier);

    //
    //for modifying the entity.
    //
    void Name(const char *name);
    void Add(CScriptEntityTask *task);
    task_list &GetTasks();

    //
    //playback functions
    //
    void PlaybackReset();
    void Playback(int end_time);
    bool Done() const;

	// for getting the unique id
	const char *UniqueID() const;

	// for setting the unique id
	void UniqueID( const char *id );
};

#endif // _DK_GCE_ENTITIES_H