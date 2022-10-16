#include "dk_misc.h"

#include "ref.h"
#include "dk_array.h"
#include "dk_point.h"

#include "dk_cin_types.h"
#include "dk_gce_spline.h"
#include "dk_gce_script.h"
#include "dk_gce_entities.h"

CScriptSound::CScriptSound() {
    time = 0.0f;
    loop = false;
	channel = 2;
    filename[0] = '\0';
}

CScriptSound::~CScriptSound() {

}

void CScriptSound::operator=(const CScriptSound &other) {
    time = other.time;
	loop = other.loop;		
	channel = other.channel;
    strncpy(filename, other.filename, 64);
    filename[63] = '\0';
}

const char *CScriptSound::Name() const {
    return filename;
}

bool CScriptSound::Loop() const {
    return loop;
}

float CScriptSound::Time() const {
    return time;
}

int CScriptSound::Channel() const {
    return channel;
}


CScriptShot::CScriptShot() {
    sequence = new CSplineSequence();

    target_flag = CT_SEQUENCE;
    end_flag = SE_SEQUENCE;

	sky = 1;

    extra_time_start = 0.0f;
    extra_time_end = 0.0f;

    have_start_fov = false;
    start_fov = SCRIPT_FOV_DEFAULT;

    have_start_timescale = false;
    start_timescale = SCRIPT_TIMESCALE_DEFAULT;

    have_start_color = false;
    start_r = SCRIPT_R_DEFAULT;
    start_g = SCRIPT_G_DEFAULT;
    start_b = SCRIPT_B_DEFAULT;
    start_a = SCRIPT_A_DEFAULT;

    //playback vars.
    next_sound = 0;
}

CScriptShot::~CScriptShot() {
    delete sequence;
}

void CScriptShot::Reset() {
    delc(sequence);

    sequence = new CSplineSequence();

    sounds.Reset();
    entities.Reset();

    entity_camera_target = "";
    entity_shot_end = "";
}

void CScriptShot::operator=(const CScriptShot &other) {
    //copy our sequence.
    *sequence = *other.sequence;

    //copy the sounds.
    sounds = other.sounds;

    //copy the entities.
    entities = other.entities;

    //copy camera target info.
    target_flag = other.target_flag;

	// copy sky info
	sky = other.sky;

    //copy shot end info.
    end_flag = other.end_flag;
    extra_time_start = other.extra_time_start;
    extra_time_end = other.extra_time_end;

    entity_camera_target = other.entity_camera_target;
    entity_shot_end = other.entity_shot_end;

    //copy fov variables.
    have_start_fov = other.have_start_fov;
    start_fov = other.start_fov;
    
    //copy timescale variables.
    have_start_timescale = other.have_start_timescale;
    start_timescale = other.start_timescale;
    
    //copy color variables.
    have_start_color = other.have_start_color;
    start_r = other.start_r;
    start_g = other.start_g;
    start_b = other.start_b;
    start_a = other.start_a;
}

const CSplineSequence *CScriptShot::Sequence() const {
    return sequence;
}

int CScriptShot::Sky() const {
    return sky;
}

float CScriptShot::ExtraTimeEnd() const {
    return extra_time_end;
}

float CScriptShot::ExtraTimeStart() const {
    return extra_time_start;
}

float CScriptShot::TotalTime() const {
    return extra_time_start + extra_time_end + sequence->TotalTime();
}

const array<CScriptSound> &CScriptShot::Sounds() const {
    return sounds;
}

const array<CScriptEntity> &CScriptShot::Entities() const {
    return entities;
}

camera_target CScriptShot::CameraTarget() const {
    return target_flag;
}

shot_end CScriptShot::EndFlag() const {
    return end_flag;
}

const char *CScriptShot::EntityCameraTarget() const {
    return entity_camera_target;
}

const char *CScriptShot::EntityShotEnd() const {
    return entity_shot_end;
}

bool CScriptShot::HaveStartFOV() const {
    return have_start_fov;
}

float CScriptShot::StartFOV() const {
    return start_fov;
}

bool CScriptShot::HaveStartTimescale() const {
    return have_start_timescale;
}

float CScriptShot::StartTimescale() const {
    return start_timescale;
}

bool CScriptShot::HaveStartColor() const {
    return have_start_color;
}

float CScriptShot::StartR() const {
    return start_r;
}

float CScriptShot::StartG() const {
    return start_g;
}

float CScriptShot::StartB() const {
    return start_b;
}

float CScriptShot::StartA() const {
    return start_a;
}






CCinematicScript::CCinematicScript() {
    have_camera_info = false;
}

CCinematicScript::~CCinematicScript() {

}

void CCinematicScript::Reset() {
    shots.Reset();

    name = "";
    disk_name = "";
}

void CCinematicScript::operator=(const CCinematicScript &other) {
    //clear out all our data first.
    Reset();

    //copy the shots.
    shots = other.shots;

    //copy the names.
    name = other.name;
    disk_name = other.disk_name;

    //check if we have camera info.
    CheckCameraInfo();
}

const char *CCinematicScript::Name() const {
    return name;
}

const char *CCinematicScript::DiskName() const {
    return disk_name;
}

int CCinematicScript::NumShots() const {
    return shots.Num();
}

const CScriptShot *CCinematicScript::Shot(int num) const {
    return shots.Item(num);
}

void CCinematicScript::CheckCameraInfo() {
    //a script with no shots does not have valid camera info, until we find shots, assume we have no camera info.
    have_camera_info = false;

    //check each shot and check if it's spline has points.
    for (int i = 0; i < shots.Num(); i++) {
        //get the shot.
        const CScriptShot *shot = shots.Item(i);
        if (shot == NULL) continue;

        //get the sequence.
        const CSplineSequence *sequence = shot->Sequence();
        if (sequence == NULL) {
            //no sequence, the no camera info.
            continue;
        }

        //check how many points there are in sequence.
        if (sequence->NumPoints() > 0) {
            //we have at least one shot with camera info.
            have_camera_info = true;
            return;
        }

        //this shot doesn't have camera info, check the next.
    }

    //no shots have camera info.
}

//
//Editing Functions
//

void CScriptSound::Name(const char *name) {
    strncpy(filename, name, 64);
    filename[63] = '\0';
}

void CScriptSound::Loop(const bool val) {
    loop = val;
}

void CScriptSound::Time(float t) {
    time = t;
}

void CScriptSound::Channel(int c) {
    channel = c;
}

void CScriptShot::Sky(int num) {
    sky = num;
}

void CScriptShot::ExtraTimeStart(float time) {
    extra_time_start = time;
}

void CScriptShot::ExtraTimeEnd(float time) {
    extra_time_end = time;
}

void CScriptShot::SetSequence(const CSplineSequence *sequence) {
    if (sequence == NULL) return;

    *this->sequence = *sequence;
}

void CScriptShot::SetSounds(const array<CScriptSound> &sounds) {
    //copy the given sound array to our own.
    this->sounds = sounds;
}

void CScriptShot::SetEntities(const array<CScriptEntity> &entities) {
    //copy the given earray.
    this->entities = entities;
}

void CScriptShot::CameraTarget(camera_target target) {
    target_flag = target;
}

void CScriptShot::EntityCameraTarget(const char *entity_name) {
    entity_camera_target = entity_name;
}

void CScriptShot::EndFlag(shot_end end_flag) {
    this->end_flag = end_flag;
}

void CScriptShot::EntityShotEnd(const char *entity_name) {
    entity_shot_end = entity_name;
}

void CScriptShot::HaveStartFOV(bool flag) {
    have_start_fov = flag;
}

void CScriptShot::StartFOV(float fov) {
    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    start_fov = fov;
}

void CScriptShot::HaveStartTimescale(bool flag) {
    have_start_timescale = flag;
}

void CScriptShot::StartTimescale(float timescale) {
    bound_min(timescale, SCRIPT_TIMESCALE_MIN);
    bound_max(timescale, SCRIPT_TIMESCALE_MAX);

    start_timescale = timescale;
}

void CScriptShot::HaveStartColor(bool flag) {
    have_start_color = flag;
}

void CScriptShot::StartColor(float r, float g, float b, float a) {
    bound_min(r, SCRIPT_COLOR_MIN);
    bound_max(r, SCRIPT_COLOR_MAX);

    bound_min(g, SCRIPT_COLOR_MIN);
    bound_max(g, SCRIPT_COLOR_MAX);

    bound_min(b, SCRIPT_COLOR_MIN);
    bound_max(b, SCRIPT_COLOR_MAX);

    bound_min(a, SCRIPT_COLOR_MIN);
    bound_max(a, SCRIPT_COLOR_MAX);

    start_r = r;
    start_g = g;
    start_b = b;
    start_a = a;
}





void CCinematicScript::Name(const char *name) {
    if (name == NULL || name[0] == '\0') return;

    this->name = name;
}

void CCinematicScript::AddShot() {
    //make a new shot.
    CScriptShot *shot = new CScriptShot();

    //add the shot to our array.
    shots.Add(shot);
}

void CCinematicScript::InsertShot(int position) {
    if (position < 0 || position > shots.Num()) return;

    //make a new shot.
    CScriptShot *shot = new CScriptShot();

    //insert the shot.
    shots.Add(shot, position);
}

CScriptShot *CCinematicScript::GetShot(int num) {
    return shots.Item(num);
}

void CCinematicScript::DeleteShot(CScriptShot *shot) {
    if (shot == NULL) return;

    shots.Remove(shot);
}


