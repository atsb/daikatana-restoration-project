#include "dk_system.h"

#include "p_user.h"
#include "keys.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "gce_main.h"
#include "gce_sequence.h"
#include "gce_script.h"
#include "gce_hud.h"
#include "gce_position.h"

#include "dk_gce_spline.h"
#include "dk_misc.h"
#include "dk_model.h"
#include "dk_buffer.h"

#include "dk_beams.h"
#include "gce_input.h"

void SequenceBuildEntities();
void SequenceDestroyEntities();
static void sequenceUpdate();

#define seq_header() {if (cin == NULL) return; if (spline == NULL) spline = new CSplineSequence();}
#define MAX_SEQ_POINTS 1024
#define MAX_SEQ_MARKERS 6000

//our current spline sequence.
static CSplineSequence *spline = NULL;

//true if we should display the segment.
static bool seq_show = true;

//true if we are editing a point's location
static bool editing_point_loc = false;

//true if we are editing a point's yaw and pitch
static bool editing_point_angles = false;

//true if the camera was warped to the ed point
static bool sequence_first_person_edit = false;

// true if the camera is moving to the next selected point
static bool sequence_moving_camera = false;
static float sequence_moving_camera_t = 0.0f;
static float sequence_moving_camera_start_t = 0.0f;
static float sequence_moving_camera_end_t = 0.0f;
static float sequence_cur_time;

//the segment in the sequence that is selected.
static int selected_seg = -1;
static int selected_point = -1;
static int selected_point_old = -1;

// storage for the editing helper forward and up rays
static entity_t selected_point_forward_ent;
static entity_t selected_point_up_ent;

//a message that is displayed at the top of the screen.  usually information from the module that
//initiated sequence mode.
static buffer256 header;

void SequenceHeader(const char *text) {
    header = text;
}

const char *SequenceHeader() {
    return header;
}

//the function that we call when we done editing the script.
static void (*SequenceSaveEditing)(const CSplineSequence *spline) = NULL;

void SequenceDoneFunction(void (*done_func)(const CSplineSequence *spline)) {
    SequenceSaveEditing = done_func;
}

//call this when the sequence changes so any work that needs to be done 
//takes place.
void SequenceModified() {
    seq_header();

    //create the entities we use to draw the spline.
    SequenceBuildEntities();
}

void SequenceAddPoint() {
    seq_header();

    //get the last camera location and angle.
    CVector loc, angle;
    cin->LastCameraPos(loc, angle);

    //get the time we should spend to get to this point from the last.
    float time = 0.0f;
    if (server->GetArgc() > 1) {
        time = atof(server->GetArgv(1));
    }
    if (time <= 0.001f) {
        time = 1.0f;
    }

    // if no selection, add it to the end
    if ((-1==selected_point)||(selected_point==spline->NumPoints()-1))
    {
        //add to the end of our spline.
        spline->Add(loc, angle, time);
    }
    else
    {
        // insert after selected_point
        spline->Insert(selected_point+1, loc, angle, time);
    }

    SequenceModified();
}

void SequenceSelectSegment() {
    seq_header();

    //check the arguments.
    if (server->GetArgc() == 2) {
        const char *arg = server->GetArgv(1);

        //check the argument.
        if (stricmp(arg, "clear") == 0) {
            //clear the selection.
            selected_seg = -1;
            return;
        }

        //check if we have any segments to select
        if (spline->NumPoints() < 2) return;

        if (selected_seg == -1 || stricmp(arg, "closest") == 0)
        {
            // if no segment is selected, choose the closest
            int i;
            CVector campos, camangle;
            cin->LastCameraPos(campos, camangle);

            // first get the closest point
            float testdist2;
            float bestdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(0)));
            int bestpoint = 0;

            for (i=1;i<spline->NumPoints();i++)
            {  
                testdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(i)));

                if (testdist2<bestdist2)
                {
                    bestdist2 = testdist2;
                    bestpoint = i;
                }
            }

            // now choose which seg with this endpoint to take after making sure we're not an endseg
            if (0 == bestpoint)
                selected_seg = 0;
            else if (spline->NumPoints()-1 == bestpoint)
                selected_seg = spline->NumPoints()-2;
            else
            {

                int lowerdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(bestpoint-1)));
                int upperdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(bestpoint+1)));

                if (lowerdist2<upperdist2)
                    selected_seg = bestpoint-1;
                else
                    selected_seg = bestpoint;
            }
        }
        else if (stricmp(arg, "next") == 0) {
            //advance the selection
            selected_seg++;
            //make sure it doesnt go too high.
            bound_max(selected_seg, spline->NumPoints() - 2);
        }
        else if (stricmp(arg, "prev") == 0) {
            //decrement the selection
            selected_seg--;
            //make sure it doesnt go too low.
            bound_min(selected_seg, 0);
        }
        else {
            //see if the argument is an integer.
            int num = atoi(server->GetArgv(1));
            if (num == 0) {
                //make sure there is a real zero there, and not just some junk string.
                if (server->GetArgv(1)[0] != '0') {
                    return;
                }
            }

            selected_seg = num;
            bound_min(selected_seg, 0);
            bound_max(selected_seg, spline->NumPoints() - 2);
        }
    }
}

void SequenceSelectPoint() {
    seq_header();

    if (true==sequence_moving_camera)
        return;
    
    //check the arguments.
    if (server->GetArgc() > 1) {
        const char *arg = server->GetArgv(1);

        if (stricmp(arg, "clear") == 0) {
            //make no point selected.
            selected_point = -1;
            return;
        }
        
        //make sure we have points to select.
        if (spline->NumPoints() < 1) return;
        
        if ((-1 == selected_point)||(stricmp(arg, "closest") == 0))
        {
            selected_point_old = -1;

            // if no point is selected, choose the closest
            
            int i;
            CVector campos, camangle;
            cin->LastCameraPos(campos, camangle);

            float testdist2;
            float bestdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(0)));
            selected_point = 0;

            for (i=1;i<spline->NumPoints();i++)
            {
                testdist2 = campos.Distance2(const_cast<CVector&>(spline->Point(i)));

                if (testdist2<bestdist2)
                {
                    bestdist2 = testdist2;
                    selected_point = i;
                }
            }
        }
        else if (stricmp(arg, "next") == 0) 
        {
            selected_point_old = selected_point;
            
            //make the next point selected.
            selected_point++;
            //validate the point.
            bound_max(selected_point, spline->NumPoints() - 1);

        }
        else if (stricmp(arg, "prev") == 0) 
        {
            selected_point_old = selected_point;
            
            //make the previous point selected.
            selected_point--;
            //validate the point.
            bound_min(selected_point, 0);
        }
        else 
		{
            //see if the argument is an integer.
            int num = atoi(server->GetArgv(1));
            if (num == 0) 
			{
                //make sure there is a real zero there, and not just some junk string.
                if (server->GetArgv(1)[0] != '0') 
				{
                    return;
                }
            }
			selected_point_old = selected_point;

			selected_point = num;
		}

        if ((-1!=selected_point_old)&&(selected_point_old!=selected_point)&&(true==sequence_first_person_edit))
        {
            int seg;

            sequence_moving_camera_start_t = 0.0f;
            for (seg=0;seg<selected_point_old;seg++)
                sequence_moving_camera_start_t += spline->Time(seg);
            
            sequence_moving_camera_end_t = 0.0f;
            for (seg=0;seg<selected_point;seg++)
                sequence_moving_camera_end_t += spline->Time(seg);
            
            sequence_moving_camera_t = sequence_moving_camera_start_t;
            sequence_moving_camera = true;
            sequence_cur_time = timeGetTime()/1000.0f;
        }

        // setup the direction entity for this point
        CVector angle = spline->Angle(selected_point);
        CVector forward, right, up;

        angle.AngleToVectors(forward, right, up);
        
        // set up lineseg that shows where the angle is aiming
        selected_point_forward_ent.origin = spline->Point(selected_point);
        selected_point_forward_ent.oldorigin = spline->Point(selected_point) + 4000.0f*forward;
        selected_point_forward_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_forward_ent.frame = 2;
        selected_point_forward_ent.alpha = 1.0f;
        
        // set up the lineseg that shows which way is up
        selected_point_up_ent.origin = spline->Point(selected_point);
        selected_point_up_ent.oldorigin = spline->Point(selected_point) + 8.0f*up;
        selected_point_up_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_up_ent.frame = 2;
        selected_point_up_ent.alpha = 1.0f;
    }
}

void SequenceTime() {
    seq_header();

    if (server->GetArgc() != 2) return;

    //get the time that was specified 
    float time = atof(server->GetArgv(1));

    if (time < 0.05f) return;

    //change the time in the spline.
    spline->Time(selected_seg, time);

    SequenceModified();
}

void SequenceHaveStartFOVToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveStartFOV(selected_seg, !spline->HaveStartFOV(selected_seg));
}

void SequenceHaveEndFOVToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveEndFOV(selected_seg, !spline->HaveEndFOV(selected_seg));
}

void SequenceStartFOV() {
    seq_header();

    if (server->GetArgc() != 2) return;

    //get the fov that was specified 
    float fov = atof(server->GetArgv(1));

    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    //change the time in the spline.
    spline->StartFOV(selected_seg, fov);

    SequenceModified();
}

void SequenceEndFOV() {
    seq_header();

    if (server->GetArgc() != 2) return;

    //get the fov that was specified 
    float fov = atof(server->GetArgv(1));

    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    //change the time in the spline.
    spline->EndFOV(selected_seg, fov);

    SequenceModified();
}

void SequenceHaveStartTimescaleToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveStartTimescale(selected_seg, !spline->HaveStartTimescale(selected_seg));
}

void SequenceHaveEndTimescaleToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveEndTimescale(selected_seg, !spline->HaveEndTimescale(selected_seg));
}

void SequenceStartTimescale() {
    seq_header();

    if (server->GetArgc() != 2) return;

    //get the timescale that was specified 
    float timescale = atof(server->GetArgv(1));

    bound_min(timescale, SCRIPT_TIMESCALE_MIN);
    bound_max(timescale, SCRIPT_TIMESCALE_MAX);

    //change the time in the spline.
    spline->StartTimescale(selected_seg, timescale);

    SequenceModified();
}

void SequenceEndTimescale() {
    seq_header();

    if (server->GetArgc() != 2) return;

    //get the timescale that was specified 
    float timescale = atof(server->GetArgv(1));

    bound_min(timescale, SCRIPT_TIMESCALE_MIN);
    bound_max(timescale, SCRIPT_TIMESCALE_MAX);

    //change the time in the spline.
    spline->EndTimescale(selected_seg, timescale);

    SequenceModified();
}

void SequenceHaveStartColorToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveStartColor(selected_seg, !spline->HaveStartColor(selected_seg));
}

void SequenceHaveEndColorToggle() {
    seq_header();

    if (server->GetArgc() != 1) return;

    spline->HaveEndColor(selected_seg, !spline->HaveEndColor(selected_seg));
}

void SequenceStartColor() {
    seq_header();

    if (server->GetArgc() != 5) return;

    //get the color that was specified 
    float r, g, b, a;
    
    r = spline->StartR(selected_seg);
    g = spline->StartG(selected_seg);
    b = spline->StartB(selected_seg);
    a = spline->StartA(selected_seg);
    
    float val;

    val = atof(server->GetArgv(1));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        r = val;
    }

    val = atof(server->GetArgv(2));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        g = val;
    }

    val = atof(server->GetArgv(3));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        b = val;
    }   
    
    val = atof(server->GetArgv(4));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        a = val;
    }   
    
    spline->StartColor(selected_seg, r, g, b, a);

    SequenceModified();
}

void SequenceEndColor() {
    seq_header();

    if (server->GetArgc() != 5) return;

    //get the color that was specified 
    float r, g, b, a;
    
    r = spline->EndR(selected_seg);
    g = spline->EndG(selected_seg);
    b = spline->EndB(selected_seg);
    a = spline->EndA(selected_seg);
    
    float val;

    val = atof(server->GetArgv(1));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        r = val;
    }

    val = atof(server->GetArgv(2));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        g = val;
    }

    val = atof(server->GetArgv(3));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        b = val;
    }

    val = atof(server->GetArgv(4));
    if (val!=-1)
    {
        bound_min(val, SCRIPT_COLOR_MIN);
        bound_max(val, SCRIPT_COLOR_MAX);
        a = val;
    }

    spline->EndColor(selected_seg, r, g, b, a);

    SequenceModified();
}

//entities that we are using to display the current sequence.
static int num_seq_points = 0;
static entity_t seq_point[MAX_SEQ_POINTS];
static int num_seq_markers = 0;
static entity_t seq_marker[MAX_SEQ_MARKERS];

static CVector unity(1.0f, 1.0f, 1.0f);

void SequenceShow() {
    seq_header();

    seq_show = true;

    SequenceModified();
}

//our sequence marker allocater
entity_t *GetMarkerEntity() {
    //check if we have all our entities allocated already.
    if (num_seq_markers >= MAX_SEQ_MARKERS) return NULL;

    //increment the count.
    num_seq_markers++;

    //return the next entity.
    return &seq_marker[num_seq_markers - 1];
}

void SequenceBuildEntities() {
    //get the ref export structure.
    refexport_t *re = cin->GetRefExport();
    if (re == NULL) return;

    //get the models we are going to use.
    void *point_model = re->RegisterModel("models/global/dv_arrow.dkm", RESOURCE_GLOBAL);

    //destroy all the old entities if we have any.
    SequenceDestroyEntities();

    //determine how many points we are going to have.
    num_seq_points = spline->NumPoints();
    bound_max(num_seq_points, MAX_SEQ_POINTS);

    if (num_seq_points < 1) return;

    //make our entities for the points.
    for (int i = 0; i < num_seq_points; i++) {
        //set the model pointer.
        seq_point[i].model = point_model;

        //position the entity.
        seq_point[i].origin = spline->Point(i);

        //get the direction we should face the arrow.
        seq_point[i].angles = spline->Angle(i);

        seq_point[i].render_scale = unity;
        
        seq_point[i].frame = seq_point[i].oldframe = 0;
    }

    //if we dont have any splines yet, stop now.
    if (num_seq_points < 2) return;

    //make a reference entity to use while creating the curve entities.
    entity_t reference;

    //set the width of the laser.
    reference.frame = 3;
    //flags.
    reference.flags = RF_TRANSLUCENT | RF_BEAM;
    //alpha
    reference.alpha = 0.1f;

    //make the entities.
    spline->MakeCurveEntities(&reference, GetMarkerEntity, 5.0f, 30.0f);
}

void SequenceAddEntities() {
    if (spline == NULL) return;
    if (cin == NULL) return;
    if (seq_show == false) return;

    //get the ref export structure.
    refexport_t *re = cin->GetRefExport();
    if (re == NULL) return;

    //add the entities used for the points.
    for (int i = 0; i < num_seq_points; i++) {

        float scale = 1.0f;
        double dummy;

		// see if the point is supposed to be selected
        if (i == selected_point) {

            // don't draw if we're at the point in first-person mode
            if (true==sequence_first_person_edit&&false==sequence_moving_camera)
                continue;

            // pulse twice per second
            scale += modf((float)timeGetTime()/500.0f, &dummy);
        }
        
        seq_point[i].render_scale.x = scale;

        seq_point[i].flags |= RF_FULLBRIGHT;
        seq_point[i].alpha = 0.5;
        cin->V_AddEntity(&seq_point[i]);
    }

    //add the entities used for the curve.
    if (num_seq_markers >= 2) {
        
        for (i = 0; i < num_seq_markers; i++) {
            //see if the marker is supposed to be selected.
            if (seq_marker[i].oldframe == selected_seg) {
                //it is part of the selected sequence, make it bright.
                seq_marker[i].alpha = 1.0f;
            }
            else {
                //it is part of the non-selected sequence, make it dim.
                seq_marker[i].alpha = 0.2f;
            }
        }
        
        cin->beam_AddGCESplines (seq_marker, num_seq_markers, selected_seg, 1.0f, 1.0f, 1.0f);
    }
    
    // add the point direction entity
    if (true == EditingSequencePoint())
    {
        cin->beam_AddGCESplines (&selected_point_forward_ent, 1, -1, 1.0f, 0.0f, 0.0f);
        cin->beam_AddGCESplines (&selected_point_up_ent, 1, -1, 1.0f, 0.0f, 0.0f);
    }   
}

void SequenceHide() {
    seq_show = false;

    SequenceDestroyEntities();
}

void SequenceDestroyEntities() {
    //reset our counts.
    num_seq_points = 0;
    num_seq_markers = 0;
}

const CSplineSequence *SequenceCurrent() {
    if (cin == NULL) return NULL;
    if (spline == NULL) spline = new CSplineSequence();

    return spline;
}

int SequenceSelectedSegment() {
    return selected_seg;
}

int SequenceSelectedPoint() {
    return selected_point;
}

void SequenceSet(const CSplineSequence *sequence) {
    seq_header();
    if (sequence == NULL) return;

    //copy the given spline.
    *spline = *sequence;

    //reset our selections.
    selected_seg = -1;
    selected_point = -1;

    SequenceModified();
}

void SequenceVelStart() {
    seq_header();

    //check for our parameter.
    if (server->GetArgc() != 2) return;

    if (stricmp(server->GetArgv(1), "0") == 0) {
        spline->VelStart(EV_DEFAULT);
    }
    else if (stricmp(server->GetArgv(1), "1") == 0) {
        spline->VelStart(EV_ZERO);
    }

    SequenceModified();
}

void SequenceVelEnd() {
    seq_header();

    //check for our parameter.
    if (server->GetArgc() != 2) return;

    if (stricmp(server->GetArgv(1), "0") == 0) {
        spline->VelEnd(EV_DEFAULT);
    }
    else if (stricmp(server->GetArgv(1), "1") == 0) {
        spline->VelEnd(EV_ZERO);
    }

    SequenceModified();
}

bool EditingSequencePoint() {
    return editing_point_loc || editing_point_angles;
}

bool EditingSequencePointLoc() {
    return editing_point_loc;
}

void SequenceEditPointLoc() {
    seq_header();

    //make sure we are not aready editing a point
    if (EditingSequencePoint()) return;

    //make sure we have a point selected.
    if (selected_point < 0 || selected_point >= spline->NumPoints()) return;

    //set our flag.
    editing_point_loc = true;

    //set up the point entry module.
    PositionPromptAnglesOff("Move to new position.  Angles locked.  Right click to edit yaw/pitch.");
    PositionPromptAnglesYawPitch("Move to new position.  Editing yaw/pitch.  Right click to edit roll.");
    PositionPromptAnglesRoll("Move to new position.  Editing roll.  Right click to lock angles.");
    PositionSaveCommand("gce_seq_edit_point_save\n");
    PositionAbortCommand("gce_seq_edit_point_abort\n");

    // give the position module the update data
    PositionUpdateObjects(spline->Point(selected_point), true, spline->Angle(selected_point), true);  // only update angles
    PositionUpdateFunc(sequenceUpdate);

    //change hud modes.
    HUDModePosition();
}

void SequenceEditPointAngles() {
    seq_header();

    //make sure we are not aready editing a point
    if (EditingSequencePoint()) return;

    //make sure we have a point selected.
    if (selected_point < 0 || selected_point >= spline->NumPoints()) return;

    //set our flag.
    editing_point_angles = true;
    
    //set up the point entry module.
    PositionPromptAnglesOff("Angles locked.  Right click to edit yaw/pitch");
    PositionPromptAnglesYawPitch("Editing yaw/pitch.  Right click to edit roll");
    PositionPromptAnglesRoll("Editing roll.  Right click to lock angles");
    PositionSaveCommand("gce_seq_edit_point_save\n");
    PositionAbortCommand("gce_seq_edit_point_abort\n");
    
    // give Position module the update data
    PositionUpdateObjects(spline->Point(selected_point), false, spline->Angle(selected_point), true);  // only update angles
    PositionUpdateFunc(sequenceUpdate);

    //change hud modes.
    HUDModePosition();
}

static void sequenceUpdate() {
    CVector pos, angles, forward, right, up;
    
    if (true == EditingSequencePoint()) {
        
        // allow angular updates in both modes
        spline->Angle(selected_point, PositionAnglesCurrent());

        // allow positional updates only in editing_point_loc mode
        if (true == editing_point_loc) 
            spline->Pos(selected_point, PositionLocCurrent());
            
        SequenceModified();

        pos = spline->Point(selected_point);
        angles = spline->Angle(selected_point);
        angles.AngleToVectors(forward, right, up);

        // set up lineseg that shows where the angle is aiming
        selected_point_forward_ent.origin = pos;
        selected_point_forward_ent.oldorigin = pos + 4000.0f*forward;
        selected_point_forward_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_forward_ent.frame = 2;
        selected_point_forward_ent.alpha = 1.0f;

        // set up the lineseg that shows which way is up
        selected_point_up_ent.origin = pos;
        selected_point_up_ent.oldorigin = pos + 8.0f*up;
        selected_point_up_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_up_ent.frame = 2;
        selected_point_up_ent.alpha = 1.0f;
    }
}

void SequenceEditPointSave() {
    if (editing_point_angles == true) {
        editing_point_angles = false;

        //change the hud mode back.
        HUDModeSequence();
    }
    else if (editing_point_loc == true) {
        editing_point_loc = false;

        //change the hud mode back.
        HUDModeSequence();
    }
}

void SequenceEditPointAbort() {
    if (true == editing_point_angles) {
        // restore old info
        spline->Angle(selected_point, PositionAnglesStart());
        
        // restore the old direction
        CVector angle = spline->Angle(selected_point);
        CVector forward, right, up;

        angle.AngleToVectors(forward, right, up);

        // set up lineseg that shows where the angle is aiming
        selected_point_forward_ent.origin = spline->Point(selected_point);
        selected_point_forward_ent.oldorigin = spline->Point(selected_point) + 4000.0f*forward;
        selected_point_forward_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_forward_ent.frame = 2;
        selected_point_forward_ent.alpha = 1.0f;

        // set up the lineseg that shows which way is up
        selected_point_up_ent.origin = spline->Point(selected_point);
        selected_point_up_ent.oldorigin = spline->Point(selected_point) + 8.0f*up;
        selected_point_up_ent.flags = RF_TRANSLUCENT | RF_BEAM;
        selected_point_up_ent.frame = 2;
        selected_point_up_ent.alpha = 1.0f;

        editing_point_angles = false;
        SequenceModified();
    }
    else if (true == editing_point_loc) {
        // restore old info
        spline->Pos(selected_point, PositionLocStart());
        spline->Angle(selected_point, PositionAnglesStart());
        editing_point_loc = false;
        SequenceModified();
    }

    //change the hud mode back.
    HUDModeSequence();
}

void SequenceDeletePoint() {
    seq_header();
    if (selected_point == -1) return;

    //tell the sequence to delete the current point.
    spline->DeletePoint(selected_point);

    //deselect our point.
    selected_point = -1;

    //make the modified sequence draw.
    SequenceModified();
}

void SequenceSetCamera(CVector &viewpos, CVector &viewangles) {
    seq_header();

    if (false == sequence_first_person_edit)
        return;
    else if (true == sequence_moving_camera)
    {
        // get the pos and angles at this time
        spline->Pos(sequence_moving_camera_t, viewpos);
        spline->Dir(sequence_moving_camera_t, viewangles);

        float lastTime = sequence_cur_time;
        sequence_cur_time = timeGetTime()/1000.0f;
        float deltaTime = sequence_cur_time-lastTime;

        if (sequence_moving_camera_t == sequence_moving_camera_end_t)
            sequence_moving_camera = false;
        else if (sequence_moving_camera_t<sequence_moving_camera_end_t)
        {
            sequence_moving_camera_t+=deltaTime;

            if (sequence_moving_camera_t>=sequence_moving_camera_end_t)
                sequence_moving_camera_t = sequence_moving_camera_end_t;
        }
        else if (sequence_moving_camera_t>sequence_moving_camera_end_t)
        {
            sequence_moving_camera_t-=deltaTime;

            if (sequence_moving_camera_t<=sequence_moving_camera_end_t)
                sequence_moving_camera_t = sequence_moving_camera_end_t;
        }
    }
    //check if we have a point selected.
    else if (selected_point != -1 && selected_point < spline->NumPoints()) {
        //we have a point selected.
        //get the location and the angles.
        CVector loc, angles;

        loc = spline->Point(selected_point);
        angles = spline->Angle(selected_point);

        //copy the points to the view pos and angles.
        viewpos = loc;
        viewangles = angles;
    }
}

void SequenceSave() {
    seq_header();

    if (SequenceSaveEditing == NULL) return;

    // kill the selections
    selected_seg = -1;
    selected_point = -1;

    // kill the first person edit
    sequence_first_person_edit = false;

    //call the function to go back to the old mode.
    SequenceSaveEditing(spline);
}

void SequenceSetFirstPersonEdit(bool fVal)
{
    sequence_first_person_edit = fVal;

    if (false==fVal)
        sequence_moving_camera = false;
}

bool SequenceGetFirstPersonEdit()
{
    return sequence_first_person_edit;
}


extern refdef_t	*rd;
void SequenceSelectPointFromTargetBox()
{
	int		sx, sy;
	int		left, top, right, bottom;
	int		left1, top1, right1, bottom1;
	int		h_clip, v_clip;

	h_clip = v_clip = true;

	CVector	position;
	
	rd = cin->GetRefDef();

	left = ( rd->width / 2 )  - 32;
	right = ( rd->width / 2 )  + 32;
	top = ( ( rd->height / 2 )  - 8 ) + 60;		// SCG[8/20/99]: + 60 for the letter box
	bottom = ( ( rd->height / 2 )  + 8 ) + 60;	// SCG[8/20/99]: + 60 for the letter box

    for( int i = 0; i < num_seq_points; i++ ) 
	{

        float scale = 1.0f;
        double dummy;

		// see if the point is supposed to be selected
        if (i == selected_point) {

            // don't draw if we're at the point in first-person mode
            if (true==sequence_first_person_edit&&false==sequence_moving_camera)
                continue;

            // pulse twice per second
            scale += modf((float)timeGetTime()/500.0f, &dummy);
        }
        
        seq_point[i].render_scale.x = scale;
        
        cin->V_AddEntity(&seq_point[i]);

		position = seq_point[i].origin;

		cin->WorldToScreen( &position.x, &sx, &sy );

		if( sx > left && sx < right )
		{
			h_clip = false;
		}
		if( sy > top && sy < bottom )
		{
			v_clip = false;
		}

		left1 = sx - 32;
		right1 = sx + 32;
		top1 = ( sy  - 8 );		// SCG[8/20/99]: + 60 for the letter box
		bottom1 = ( sy  + 8 );	// SCG[8/20/99]: + 60 for the letter box

		HUDDrawEmptyBox(left1, top1, right1, bottom1, CVector(1.0, 1.0, 1.0));
		if( h_clip == false && v_clip == false )
		{
 			server->CBuf_AddText(va("gce_seq_select_point %d\n", i ) );
			break;
		}
    }
}

