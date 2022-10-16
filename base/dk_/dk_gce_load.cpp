#include "qcommon.h"

#include "dk_array.h"
#include "dk_buffer.h"
#include "dk_point.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_gce_spline.h"
#include "dk_gce_script.h"
#include "dk_gce_entities.h"
#include "dk_gce_headbob.h"

void CCubicSpline::Load(FILE *file) {
    fread(&a, sizeof(float), 1, file);
    fread(&b, sizeof(float), 1, file);
    fread(&c, sizeof(float), 1, file);
    fread(&d, sizeof(float), 1, file);
}

void CPointSpline::Load(FILE *file) {
    x.Load(file);
    y.Load(file);
    z.Load(file);
}

void CSplineSequenceComponent::Load(FILE *file, int version) {
    Reset();

    //load the number of points.
    fread(&num_points, sizeof(num_points), 1, file);

    //allocate our arrays.
    if (num_points > 0) {
        points = new CVector[num_points];
        velocity_norm = new CVector[num_points];
        velocity = (float*)memmgr.X_Malloc(sizeof(float)*num_points,MEM_TAG_MISC);
    }

    //read the points.
    for (int i = 0; i < num_points; i++) {
        fread(&points[i], sizeof(CVector), 1, file);
        fread(&velocity_norm[i], sizeof(CVector), 1, file);
        fread(&velocity[i], sizeof(float), 1, file);
    }

    //allocate the spline array.
    if (num_points > 1) {
        splines = new CPointSpline *[num_points - 1];
    }

    for (i = 0; i < num_points - 1; i++) {
        //allocate a new spline.
        splines[i] = new CPointSpline();

        splines[i]->Load(file);
    }
}

void CSplineSequenceComponent::Init(int num_points, CVector *points, CVector *velocity_norm, float *velocity, CPointSpline **splines) {
    Reset();

    //store the given data.
    this->num_points = num_points;
    this->points = points;
    this->velocity_norm = velocity_norm;
    this->velocity = velocity;
    this->splines = splines;
}

void CSplineSequenceSegmentData::Load(FILE *file, int version) {
    //load all of our data.
    fread(&time, sizeof(time), 1, file);

    fread(&have_start_fov, sizeof(have_start_fov), 1, file);
    fread(&have_end_fov, sizeof(have_end_fov), 1, file);

    fread(&start_fov, sizeof(start_fov), 1, file);
    fread(&end_fov, sizeof(end_fov), 1, file);
    
    if (version>=11)
    {
        fread(&have_start_timescale, sizeof(have_start_timescale), 1, file);
        fread(&have_end_timescale, sizeof(have_end_timescale), 1, file);
    
        fread(&start_timescale, sizeof(start_timescale), 1, file);
        fread(&end_timescale, sizeof(end_timescale), 1, file);
    }

    fread(&have_start_color, sizeof(have_start_color), 1, file);
    fread(&have_end_color, sizeof(have_end_color), 1, file);

    fread(&start_r, sizeof(start_r), 1, file);
    fread(&start_g, sizeof(start_g), 1, file);
    fread(&start_b, sizeof(start_b), 1, file);
    fread(&start_a, sizeof(start_a), 1, file);
    
    fread(&end_r, sizeof(end_r), 1, file);
    fread(&end_g, sizeof(end_g), 1, file);
    fread(&end_b, sizeof(end_b), 1, file);
    fread(&end_a, sizeof(end_a), 1, file);
}



void CSplineSequence::Load(FILE *file, int version) {
    //reset our shit.
    Reset();

    //load the number of points we have.
    fread(&num_points, sizeof(int), 1, file);

    if (version < 6) {
        //version 6 is when we switched everything over to the CSplineSequenceComponent system.
        //load up all the data from the files into temporary arrays, then pass them on to the components.
        CPointSpline **pos_splines = NULL;
        CPointSpline **dir_splines = NULL;

        CVector *points = NULL;
        CVector *velocity_norm = NULL;
        float *velocity = NULL;

        CVector *directions = NULL;
        CVector *dir_vel_norm = NULL;
        float *dir_vel = NULL;

        //allocate the arrays.
        if (num_points > 0) {
            points = new CVector[num_points];
            velocity_norm = new CVector[num_points];
            velocity = (float*)memmgr.X_Malloc(sizeof(float)*num_points,MEM_TAG_MISC);

            directions = new CVector[num_points];
            dir_vel_norm = new CVector[num_points];
            dir_vel = (float*)memmgr.X_Malloc(sizeof(float)*num_points,MEM_TAG_MISC);
        }

        if (num_points > 1) {
            segment_data.Length(num_points - 1);

            pos_splines = new CPointSpline *[num_points - 1];
            dir_splines = new CPointSpline *[num_points - 1];
        }

        //load the "stuff"
        for (int i = 0; i < num_points; i++) {
            fread(&points[i], sizeof(CVector), 1, file);
            fread(&velocity_norm[i], sizeof(CVector), 1, file);
            fread(&velocity[i], sizeof(float), 1, file);

            fread(&directions[i], sizeof(CVector), 1, file);
            fread(&dir_vel_norm[i], sizeof(CVector), 1, file);
            fread(&dir_vel[i], sizeof(float), 1, file);
        }

        for (i = 0; i < num_points - 1; i++) {
            float time;
            fread(&time, sizeof(float), 1, file);
            segment_data.Item(i).time = time;

            pos_splines[i] = new CPointSpline();
            pos_splines[i]->Load(file);

            dir_splines[i] = new CPointSpline();
            dir_splines[i]->Load(file);
        }

        //give the loaded arrays to our components.
        position.Init(num_points, points, velocity_norm, velocity, pos_splines);
        angle.Init(num_points, directions, dir_vel_norm, dir_vel, dir_splines);
    }
    else {
        //load the components.
        position.Load(file, version);
        angle.Load(file, version);

        //allocate the segent data array.
        segment_data.Length(num_points - 1);

        if (version < 9) {
            //only times are saved.
            for (int i = 0; i < num_points - 1; i++) {
                float time;
                fread(&time, sizeof(time), 1, file);

                segment_data.Item(i).time = time;
            }
        }
        else {
            //load the segment data.
            segment_data.Load(file, version);
        }
    }

    //load our velocities.
    fread(&velocity_start, sizeof(end_vel), 1, file);
    fread(&velocity_end, sizeof(end_vel), 1, file);

    //load the sequence total time.
    fread(&total_time, sizeof(float), 1, file);
}

void CCinematicScript::Load(FILE *file, int version, const char *name) {
    //clear out all of our data.
    Reset();

    shots.Load(file, version);

    //set our names.
    this->name = name;
    disk_name = name;

    //check if we have camera info.
    CheckCameraInfo();
}

void CScriptShot::Load(FILE *file, int version) {
    Reset();

    //load the spline sequence.
    sequence->Load(file, version);

    //load camera target info.
    fread(&target_flag, sizeof(target_flag), 1, file);

    //load shot end info.
    fread(&end_flag, sizeof(end_flag), 1, file);
    fread(&extra_time_start, sizeof(extra_time_start), 1, file);
    fread(&extra_time_end, sizeof(extra_time_end), 1, file);

    if (version >= 1) {
        sounds.Load(file, version);
    }

    if (version >= 2) {
        entities.Load(file, version);
    }

    if (version >= 3) {
        entity_camera_target.Load(file, version);
        entity_shot_end.Load(file, version);
    }

    if (version >= 10) {
        fread(&have_start_fov, sizeof(bool), 1, file);
        fread(&start_fov, sizeof(float), 1, file);
    }

	if (version >= 14)
		fread(&sky, sizeof(int), 1, file);

}

const CCinematicScript *LoadScript(const char *name) {
    if (name == NULL || name[0] == '\0') return NULL;

    //make the file name.
    buffer128 filename("cin/scripts/%s.script", name);

    //open the file.
    FILE *file = NULL;
    FS_FOpenFile(filename, &file);
    if (file == NULL) return NULL;

    //read the version number out of the file.
    int version;
    fread(&version, sizeof(int), 1, file);

    //check the version number
    if (version > SCRIPT_FILE_VERSION) {
        //the version on disk is greater than we know how to read.
        FS_FCloseFile(file);
        return NULL;
    }

    //create a new script.
    CCinematicScript *script = new CCinematicScript();

    //load the script.
    script->Load(file, version, name);

    //close the file
    FS_FCloseFile(file);

    //return the script.
    return script;
}

void CScriptSound::Load(FILE *file, int version) {
    fread(&filename[0], sizeof(filename), 1, file);
    
    if (version>=11)
        fread(&loop, sizeof(loop), 1, file);

    if( version >= 15 )
        fread(&channel, sizeof(channel), 1, file);

    fread(&time, sizeof(time), 1, file);
}

void DeleteScript(const CCinematicScript *script) {
    if (script == NULL) return;

    delete (CCinematicScript *)script;
}

void CScriptEntity::Load(FILE *file, int version) {
    //load our name.
    if (version >= 3) {
        name.Load(file, version);
    }
    else {
        char name_str[16];
        fread(&name_str[0], sizeof(name_str), 1, file);

        name = name_str;
    }

	if( version >= 15 )
	{
		uniqueid.Load( file, version );
	}

    //load the tasks.
    tasks.Load(file, version);
}

void CScriptEntityTask::Load(FILE *file, int version) {
    //load the type.
    fread(&type, sizeof(type), 1, file);

    //load the time;
    fread(&time, sizeof(time), 1, file);

    //load the position and direction.
    fread(&destination, sizeof(destination), 1, file);
    fread(&direction, sizeof(direction), 1, file);

    if (version < 4) {
        //subtract off the camera offset from the destination point
        destination.z -= 22;
    }

    //load the attribute value.
    if (version >= 5) {
        fread(&attribute_value, sizeof(attribute_value), 1, file);
    }

    if (version >= 7) {
        if (type == TT_HEAD_SCRIPT) {
            //make a head script object.
            headscript = new CEntityHeadScript();

            //load the head script.
            headscript->Load(file, version);
        }
    }

    //load the animation name.
    if (version >= 8) {
        animation_name.Load(file, version);

        use_entity_name.Load(file, version);
    }

	if (version >= 12)
		sound_name.Load(file, version);
	
	if (version >= 13)
	    fread(&sound_duration, sizeof(sound_duration), 1, file);

	if( version >= 15 )
	{
		uniqueid.Load( file, version );
	}
}

void CEntityHeadScript::Load(FILE *file, int version) {
    //load the sequence data.
    angle.Load(file, version);
}



