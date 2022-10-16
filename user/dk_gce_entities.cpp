#include "dk_misc.h"
#include "ref.h"

#include "dk_shared.h"
#include "dk_gce_script.h"
#include "dk_point.h"
#include "dk_gce_entities.h"
#include "dk_gce_spline.h"
#include "dk_gce_headbob.h"


CScriptEntity::CScriptEntity() {
    name = "";
	uniqueid = "";
    next_task = 0;
    current_task_start_time = 0;
    saved_task_num = 0;

}

CScriptEntity::~CScriptEntity() {

}

void CScriptEntity::Reset() {
    name = "";

    tasks.Reset();
}

void CScriptEntity::operator=(const CScriptEntity &other) {
    name = other.name;
    tasks = other.tasks;
	uniqueid = other.uniqueid;
}

const char *CScriptEntity::Name() const {
    return name;
}

const task_list &CScriptEntity::Tasks() const {
    return tasks;
}

void CScriptEntity::Name(const char *name) {
    this->name = name;
}

void CScriptEntity::Add(CScriptEntityTask *task) {
    if (task == NULL) return;

    tasks.Add(task);
}

task_list &CScriptEntity::GetTasks() {
    return tasks;
}

int CScriptEntity::Compare(const CScriptEntity **e1, const CScriptEntity **e2) {
    if (e1 == NULL || e2 == NULL) return 0;
    if (*e1 == NULL || *e2 == NULL) return 0;

    return stricmp((*e1)->Name(), (*e2)->Name());
}

int CScriptEntity::Find(const CScriptEntity *entity, const void *identifier) {
    if (entity == NULL || identifier == NULL) return 0;

    //the identifier is a string.
    const char *name = (const char *)identifier;

    //compare the name with the entity's name.
    int compare = stricmp(name, entity->Name());

    if (compare == 0) return 0;
    if (compare < 0) return -1;
    else return 1;
}

const char *CScriptEntity::UniqueID() const {
	return uniqueid;
}

void CScriptEntity::UniqueID( const char *id ) {
	uniqueid = id;
}

CScriptEntityTask::CScriptEntityTask() {
    time = -1.0f;
    type = TT_NONE;

    headscript = NULL;
}

CScriptEntityTask::~CScriptEntityTask() {
    delc(headscript);
}

void CScriptEntityTask::Reset() {
    time = -1.0f;
    type = TT_NONE;
	sound_duration = 0.0f;
}

void CScriptEntityTask::operator=(const CScriptEntityTask &other) {
    time = other.time;
    type = other.type;

    destination = other.destination;

    direction = other.direction;

    attribute_value = other.attribute_value;

    //delete our old headscript.
    delc(headscript);

    //check if the other task has a headscript.
    if (other.headscript != NULL) {
        //allocate a new one.
        headscript = new CEntityHeadScript();

        //copy the other head script.
        *headscript = *other.headscript;
    }

    animation_name = other.animation_name;
    
    use_entity_name = other.use_entity_name;

	sound_name = other.sound_name;
	
	sound_duration = other.sound_duration;

	uniqueid = other.uniqueid;
}

task_type CScriptEntityTask::Type() const {
    return type;
}

float CScriptEntityTask::AttributeValue() const {
    return attribute_value;
}

const char *CScriptEntityTask::AnimationName() const {
    return animation_name;
}

const char *CScriptEntityTask::SoundName() const {
    return sound_name;
}

const float CScriptEntityTask::SoundDuration() const {
    return sound_duration;
}

const char *CScriptEntityTask::UseEntityName() const {
    return use_entity_name;
}

const CEntityHeadScript *CScriptEntityTask::HeadScript() const {
    return headscript;
}

const char *CScriptEntityTask::TypeString() const {
    switch (type) {
        case TT_NONE:
            return "None";
        case TT_MOVE_TO:
            return "Move To";
        case TT_TELEPORT:
            return "Teleport";
        case TT_TURN:
            return "Turn";
        case TT_MOVE_AND_TURN:
            return "Move and Turn";
        case TT_ATTRIBUTE_BACKUP:
            return "Backup Attributes";
        case TT_ATTRIBUTE_RESTORE:
            return "Restore Attributes";
        case TT_ATTRIBUTE_RUN_SPEED:
            return "Set Run Speed";
        case TT_ATTRIBUTE_WALK_SPEED:
            return "Set Walk Speed";
        case TT_ATTRIBUTE_YAW_SPEED:
            return "Set Yaw Speed";
        case TT_WAIT:
            return "Wait";
        case TT_START_RUNNING:
            return "Start Running";
        case TT_START_WALKING:
            return "Start Walking";
        case TT_TRIGGER_BRUSH_USE:
            return "Trigger Brush Use";
        case TT_HEAD_SCRIPT:
            return "Head Script";
        case TT_PLAY_ANIMATION:
            return "Play Animation";
        case TT_SET_IDLE_ANIMATION:
            return "Set Idle Animation";
        case TT_PLAY_SOUND:
            return "Play Sound";
		case TT_SPAWN_ENTITY:
			return "Spawn Entity";
		case TT_REMOVE_ENTITY:
			return "Remove Entity";
		case TT_CLEAR_GOALS:
			return "Clear Goals";
        default:
            return "Invalid Type";
    }
}

float CScriptEntityTask::Time() const {
    return time;
}

const CVector &CScriptEntityTask::Destination() const {
    return destination;
}

const CVector &CScriptEntityTask::Direction() const {
    return direction;
}

void CScriptEntityTask::MoveTo(const CVector &dest) {
    type = TT_MOVE_TO;

    destination = dest;
}

void CScriptEntityTask::Turn(const CVector &dir) {
    type = TT_TURN;

    direction = dir;
}

void CScriptEntityTask::MoveAndTurn(const CVector &dest, const CVector &dir) {
    type = TT_MOVE_AND_TURN;

    destination = dest;
    direction = dir;
}

void CScriptEntityTask::Teleport(const CVector &dest, const CVector &dir) {
    type = TT_TELEPORT;

    destination = dest;
    direction = dir;
}

void CScriptEntityTask::AttributeBackup() {
    type = TT_ATTRIBUTE_BACKUP;
}

void CScriptEntityTask::AttributeRestore() {
    type = TT_ATTRIBUTE_RESTORE;
}

void CScriptEntityTask::StartWalking() {
    type = TT_START_WALKING;
}

void CScriptEntityTask::StartRunning() {
    type = TT_START_RUNNING;
}

void CScriptEntityTask::HeadScript(const CEntityHeadScript *headscript) {
    if (headscript == NULL) return;

    //set the type.
    type = TT_HEAD_SCRIPT;

    //delete our old headscript, if we had one.
    delc(this->headscript);

    //make a copy of the given script.
    this->headscript = new CEntityHeadScript();
    *this->headscript = *headscript;
}

void CScriptEntityTask::PlayAnimation(const char *name) {
    type = TT_PLAY_ANIMATION;

    //save the given animation name
    animation_name = name;
}

void CScriptEntityTask::SetIdleAnimation(const char *name) {
    type = TT_SET_IDLE_ANIMATION;

    //save the given animation name
    animation_name = name;
}

void CScriptEntityTask::PlaySound(const char *name, const float duration) {
    type = TT_PLAY_SOUND;

    //save the given sound name
    sound_name = name;
	sound_duration = duration;
}

void CScriptEntityTask::SpawnEntity( const char *id, const CVector& dest, const CVector& dir )
{
	type = TT_SPAWN_ENTITY;
	uniqueid = id;
	destination = dest;
	direction = dir;
}

void CScriptEntityTask::ClearGoals( const char *id, float total_time )
{
	time = total_time;
	type = TT_CLEAR_GOALS;
	uniqueid = id;
}

void CScriptEntityTask::RemoveEntity( const char *id )
{
	type = TT_REMOVE_ENTITY;
	uniqueid = id;
}

void CScriptEntityTask::TriggerBrushUse() {
    type = TT_TRIGGER_BRUSH_USE;
}

void CScriptEntityTask::AttributeYawSpeed(float yaw_speed) {
    type = TT_ATTRIBUTE_YAW_SPEED;

    //do some error checking.
    bound_min(yaw_speed, 1.0f);
    bound_max(yaw_speed, 1000.0f);

    //save the value.
    attribute_value = yaw_speed;
}

void CScriptEntityTask::AttributeRunSpeed(float run_speed) {
    type = TT_ATTRIBUTE_RUN_SPEED;

    AttributeValue(run_speed);
}

void CScriptEntityTask::AttributeWalkSpeed(float walk_speed) {
    type = TT_ATTRIBUTE_WALK_SPEED;

    AttributeValue(walk_speed);
}

void CScriptEntityTask::Wait(float time) {
    type = TT_WAIT;

    AttributeValue(time);
}

void CScriptEntityTask::AttributeValue(float value) {
    //do error checking first.
    if (type == TT_ATTRIBUTE_RUN_SPEED) {
        bound_min(value, 1.0f);
        bound_max(value, 1000.0f);
    }
    else if (type == TT_ATTRIBUTE_WALK_SPEED) {
        bound_min(value, 1.0f);
        bound_max(value, 1000.0f);
    }
    else if (type == TT_ATTRIBUTE_YAW_SPEED) {
        bound_min(value, 1.0f);
        bound_max(value, 1000.0f);
    }
    else if (type == TT_WAIT) {
        bound_min(value, 0.0f);
        bound_max(value, 60.0f);
    }
    else {
        //invalid type.
        return;
    }

    attribute_value = value;
}

void CScriptEntityTask::AnimationName(const char *name) {
    animation_name = name;
}

void CScriptEntityTask::SoundName(const char *name) {
    sound_name = name;
}

void CScriptEntityTask::SoundDuration(const float duration) {
    sound_duration = duration;
}

void CScriptEntityTask::UseEntityName(const char *name) {
    use_entity_name = name;
}

void CScriptEntityTask::Time(float time) {
    this->time = time;
}

const char *CScriptEntityTask::UniqueID() const {
	return uniqueid;
}

void CScriptEntityTask::UniqueID( const char *id ) {
	uniqueid = id;
}
