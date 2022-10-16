#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"
#include "dk_array.h"

#include "dk_gce_spline.h"
#include "dk_gce_script.h"
#include "dk_gce_entities.h"
#include "dk_gce_headbob.h"
#include "dk_buffer.h"

#include "gce_script.h"

extern "C" __declspec(dllimport) unsigned long __stdcall GetFileAttributesA(const char *lpFileName);
extern "C" __declspec(dllimport) int __stdcall CopyFileA(const char *lpExistingFileName, const char *lpNewFileName, int bFailIfExists);

void CCubicSpline::Save(FILE *file) {
    fwrite(&a, sizeof(float), 1, file);
    fwrite(&b, sizeof(float), 1, file);
    fwrite(&c, sizeof(float), 1, file);
    fwrite(&d, sizeof(float), 1, file);
}

void CPointSpline::Save(FILE *file) {
    x.Save(file);
    y.Save(file);
    z.Save(file);
}

void CSplineSequenceComponent::Save(FILE *file) const {
    //save the number of points.
    fwrite(&num_points, sizeof(num_points), 1, file);

    //write points.
    for (int i = 0; i < num_points; i++) {
        fwrite(&points[i], sizeof(CVector), 1, file);
        fwrite(&velocity_norm[i], sizeof(CVector), 1, file);
        fwrite(&velocity[i], sizeof(float), 1, file);
    }

    for (i = 0; i < num_points - 1; i++) {
        splines[i]->Save(file);
    }
}

void CSplineSequenceSegmentData::Save(FILE *file) const {
    //save all our data.
    fwrite(&time, sizeof(time), 1, file);

    fwrite(&have_start_fov, sizeof(have_start_fov), 1, file);
    fwrite(&have_end_fov, sizeof(have_end_fov), 1, file);

    fwrite(&start_fov, sizeof(start_fov), 1, file);
    fwrite(&end_fov, sizeof(end_fov), 1, file);
    
    fwrite(&have_start_timescale, sizeof(have_start_timescale), 1, file);
    fwrite(&have_end_timescale, sizeof(have_end_timescale), 1, file);

    fwrite(&start_timescale, sizeof(start_timescale), 1, file);
    fwrite(&end_timescale, sizeof(end_timescale), 1, file);
    
    fwrite(&have_start_color, sizeof(have_start_color), 1, file);
    fwrite(&have_end_color, sizeof(have_end_color), 1, file);

    fwrite(&start_r, sizeof(start_r), 1, file);
    fwrite(&start_g, sizeof(start_g), 1, file);
    fwrite(&start_b, sizeof(start_b), 1, file);
    fwrite(&start_a, sizeof(start_a), 1, file);
    
    fwrite(&end_r, sizeof(end_r), 1, file);
    fwrite(&end_g, sizeof(end_g), 1, file);
    fwrite(&end_b, sizeof(end_b), 1, file);
    fwrite(&end_a, sizeof(end_a), 1, file);
}

void CSplineSequence::Save(FILE *file) {
    //save the number of points we have.
    fwrite(&num_points, sizeof(int), 1, file);

    //save the position and angle sequence components.
    position.Save(file);
    angle.Save(file);

    //save the segment data.
    segment_data.Save(file);

    fwrite(&velocity_start, sizeof(end_vel), 1, file);
    fwrite(&velocity_end, sizeof(end_vel), 1, file);
    fwrite(&total_time, sizeof(float), 1, file);
}


void CCinematicScript::Save(FILE *file) const {
    shots.Save(file);
}
                                         
void CScriptShot::Save(FILE *file) const {
    //save the spline sequence.
    sequence->Save(file);

    //save camera target info.
    fwrite(&target_flag, sizeof(target_flag), 1, file);

    //save shot end info.
    fwrite(&end_flag, sizeof(end_flag), 1, file);
    fwrite(&extra_time_start, sizeof(extra_time_start), 1, file);
    fwrite(&extra_time_end, sizeof(extra_time_end), 1, file);

    //save the sounds.
    sounds.Save(file);

    //save the entities.
    entities.Save(file);

    //save the entity names we use to keep the camera target and shot completion data.
    entity_camera_target.Save(file);
    entity_shot_end.Save(file);

    //save FOV data.
    fwrite(&have_start_fov, sizeof(bool), 1, file);
    fwrite(&start_fov, sizeof(float), 1, file);

	// save sky index
	fwrite(&sky, sizeof(int), 1, file);
}

void CScriptSound::Save(FILE *file) const {
    fwrite(&filename[0], sizeof(filename), 1, file);
    fwrite(&loop, sizeof(loop), 1, file);
    fwrite(&channel, sizeof(channel), 1, file);
    fwrite(&time, sizeof(time), 1, file);
}

void CScriptEntity::Save(FILE *file) const {
    //save our name.
    name.Save(file);

	// save the uniqueid
	uniqueid.Save(file);

    //save the tasks.
    tasks.Save(file);
}

void CScriptEntityTask::Save(FILE *file) const {
    //save the type.
    fwrite(&type, sizeof(type), 1, file);

    //save the time;
    fwrite(&time, sizeof(time), 1, file);

    //save the position and direction.
    fwrite(&destination, sizeof(destination), 1, file);
    fwrite(&direction, sizeof(direction), 1, file);

    fwrite(&attribute_value, sizeof(attribute_value), 1, file);

    if (type == TT_HEAD_SCRIPT) {
        //save the head script data.
        if (headscript == NULL) {
            //there is no head script, make a temporary one to save.
            CEntityHeadScript *tempscript = new CEntityHeadScript();
            tempscript->Save(file);

            //delete our temporary script.
            delete tempscript;
        }
        else {
            headscript->Save(file);
        }
    }

    //save the animation name.
    animation_name.Save(file);

    //save the use entity name
    use_entity_name.Save(file);

	//save the sound name
	sound_name.Save(file);

	// save the sound duration
    fwrite(&sound_duration, sizeof(sound_duration), 1, file);

	// SCG[10/1/99]: save uniqueid
	uniqueid.Save(file);
}

void CEntityHeadScript::Save(FILE *file) const {
    //save the sequence component data.
    angle.Save(file);
}

void BackupOldScript(const char *filename) {
    buffer128 backupname;

    //go through a 1000 backup names.
    for (int i = 0; i < 1000; i++) {
        //make the name.
        backupname.Set("%s.%03d.bak", filename, i);

        //check if the file exists.
        if (GetFileAttributesA(backupname) == 0xffffffff) {
            //use this file name as backup.
            //copy the file to the backup name.
            CopyFileA(filename, backupname, 0);
            return;
        }
    }
}

void ScriptSave() {
    //get the current script.
    const CCinematicScript *script = ScriptCurrent();
    if (script == NULL) return;

    //get the name of the script.
    const char *name = script->Name();

    //check if the name is valid.
    if (name == NULL || name[0] == '\0' || name[0] == ' ') {
        return;
    }

    //make the file name.
    buffer128 filename("data\\cin\\scripts\\%s.script", name);

    //backup our old script.
    BackupOldScript(filename);

    //open the file.
    FILE *file = fopen(filename, "wb");
    if (file == NULL) return;

    //write the version number.
    int version = SCRIPT_FILE_VERSION;
    fwrite(&version, sizeof(version), 1, file);

    //save the script.
    script->Save(file);

    fclose(file);
}




