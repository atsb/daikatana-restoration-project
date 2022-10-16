#include "p_user.h"
#include "ref.h"

#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "dk_gce_spline.h"

#include "gce_main.h"
#include "gce_hud.h"
#include "gce_fields.h"
#include "gce_sequence.h"
#include "gce_script.h"

typedef enum {
    FQ_SAVE,
    FQ_REENTER,
    FQ_EDIT_DIR,
    FQ_VEL_START,
    FQ_VEL_END,
    FQ_SEGMENT_TIME,

    FQ_START_FOV_RADIO,
    FQ_START_FOV,
    FQ_END_FOV_RADIO,
    FQ_END_FOV,

    FQ_START_TIMESCALE_RADIO,
    FQ_START_TIMESCALE,
    FQ_END_TIMESCALE_RADIO,
    FQ_END_TIMESCALE,

    FQ_START_COLOR_RADIO,
    FQ_START_RED,
    FQ_START_GREEN,
    FQ_START_BLUE,
    FQ_START_ALPHA,
    FQ_END_COLOR_RADIO,
    FQ_END_RED,
    FQ_END_GREEN,
    FQ_END_BLUE,
    FQ_END_ALPHA,

    FQ_PREV_POINT,
    FQ_NO_POINT,
    FQ_NEXT_POINT,
    FQ_CLOSEST_POINT,

    FQ_DELETE_POINT,

    FQ_PREV_SEGMENT,
    FQ_NO_SEGMENT,
    FQ_NEXT_SEGMENT,
    FQ_CLOSEST_SEGMENT,

    FQ_NUM_FIELDS,
} fields_sequence;

int num_fields_sequence = FQ_NUM_FIELDS;

//decriptions for the fields in each mode.
field_description field_desc_sequence[] = {
    {FT_BUTTON, 20, "Save Sequence"},
    {FT_BUTTON, 10, "Redo Point"},
    {FT_BUTTON, 12, "Change Angle"},
    {FT_LIST, 15, "Starting velocity: "},
    {FT_LIST, 15, "Ending velocity: "},
    {FT_FLOAT, 6, "Time of selected segment: "},

    {FT_RADIO, 0, "Have Start FOV"},
    {FT_FLOAT, 6, "Start FOV: "},
    {FT_RADIO, 0, "Have End FOV"},
    {FT_FLOAT, 6, "End FOV: "},

    {FT_RADIO, 0, "Have Start Timescale"},
    {FT_FLOAT, 6, "Start Timescale: "},
    {FT_RADIO, 0, "Have End Timescale"},
    {FT_FLOAT, 6, "End Timescale: "},

    {FT_RADIO, 0, "Have Start Color"},
    {FT_FLOAT, 6, "R: "},
    {FT_FLOAT, 6, "G: "},
    {FT_FLOAT, 6, "B: "},
    {FT_FLOAT, 6, "A: "},
    {FT_RADIO, 0, "Have End Color"},
    {FT_FLOAT, 6, "R: "},
    {FT_FLOAT, 6, "G: "},
    {FT_FLOAT, 6, "B: "},
    {FT_FLOAT, 6, "A: "},

    {FT_BUTTON, 3, "<"},
    {FT_BUTTON, 5, "None"},
    {FT_BUTTON, 3, ">"},
    {FT_BUTTON, 8, "Closest"},

    {FT_BUTTON, 5, "Del"},

    {FT_BUTTON, 3, "<"},
    {FT_BUTTON, 5, "None"},
    {FT_BUTTON, 3, ">"},
    {FT_BUTTON, 8, "Closest"},

};

//the choices in the velocity lists in the sequence mode..
char *vel_choices[] = {
    "Default",
    "Zero   ",
};

//list descriptors for the sequence velocity settings in the sequence mode.
list_field_description list_vel_start = {
    vel_choices, sizeof(vel_choices)/sizeof(char *)
};

list_field_description list_vel_end = {
    vel_choices, sizeof(vel_choices)/sizeof(char *)
};

bool field_mask_sequence(int field) {
    const CSplineSequence *sequence = SequenceCurrent();
    if (sequence == NULL) return false;

    if (field == FQ_SAVE) return true;
    if (field == FQ_VEL_START) return true;
    if (field == FQ_VEL_END) return true;

    //get the selected segment.
    int selected_seg = SequenceSelectedSegment();
    if (selected_seg != -1) {
        if (field == FQ_SEGMENT_TIME) return true;
        if (field == FQ_NO_SEGMENT) return true;

        if (field == FQ_START_FOV_RADIO) return true;
        if (field == FQ_END_FOV_RADIO) return true;
        
        if (field == FQ_START_TIMESCALE_RADIO) return true;
        if (field == FQ_END_TIMESCALE_RADIO) return true;
        
        if (field == FQ_START_COLOR_RADIO) return true;
        if (field == FQ_END_COLOR_RADIO) return true;

        //check if any of the fov parameters are specified.
        if (sequence->HaveStartFOV(selected_seg) == true) {
            if (field == FQ_START_FOV) return true;
        }

        if (sequence->HaveEndFOV(selected_seg) == true) {
            if (field == FQ_END_FOV) return true;
        }
        
        //check if any of the timescale parameters are specified.
        if (sequence->HaveStartTimescale(selected_seg) == true) {
            if (field == FQ_START_TIMESCALE) return true;
        }

        if (sequence->HaveEndTimescale(selected_seg) == true) {
            if (field == FQ_END_TIMESCALE) return true;
        }
        
        //check if any of the color parameters are specified.
        if (sequence->HaveStartColor(selected_seg) == true) {
            if (field == FQ_START_RED) return true;
            if (field == FQ_START_GREEN) return true;
            if (field == FQ_START_BLUE) return true;
            if (field == FQ_START_ALPHA) return true;
        }

        if (sequence->HaveEndColor(selected_seg) == true) {
            if (field == FQ_END_RED) return true;
            if (field == FQ_END_GREEN) return true;
            if (field == FQ_END_BLUE) return true;
            if (field == FQ_END_ALPHA) return true;
        }
    }
    //get the selected point.
    int selected_pnt = SequenceSelectedPoint();
    if (selected_pnt != -1) {
        if (field == FQ_REENTER) return true;
        if (field == FQ_EDIT_DIR) return true;
        if (field == FQ_NO_POINT) return true;

        if (field == FQ_DELETE_POINT) return true;
    }

    //check if the next/previous and closest buttons should be enabled for selecting points and segments.
    int num_points = sequence->NumPoints();
    if (num_points > 0) {
        if (field == FQ_NEXT_POINT && (selected_pnt == -1 || selected_pnt < num_points - 1)) return true;
        if (field == FQ_PREV_POINT && (selected_pnt == -1 || selected_pnt > 0)) return true;
        if (field == FQ_NO_POINT && selected_pnt != -1) return true;
        if (field == FQ_NEXT_SEGMENT && (selected_seg == -1 || selected_seg < num_points - 2)) return true;
        if (field == FQ_PREV_SEGMENT && (selected_seg == -1 || selected_seg > 0)) return true;
        if (field == FQ_NO_SEGMENT && selected_seg != -1) return true;
        if (field == FQ_CLOSEST_POINT) return true;
        if (field == FQ_CLOSEST_SEGMENT) return true;
        
    }

    return false;
}

void edit_start_sequence(int field) {
    //get the current sequence.
    const CSplineSequence *sequence = SequenceCurrent();
    if (sequence == NULL) return;

    if (field == FQ_SAVE) {
        FieldEdit(field_desc_sequence[field]);
    }
    else if (field == FQ_VEL_START) {
        list_vel_start.selected = sequence->VelStart();
        FieldEdit(field_desc_sequence[field], list_vel_start);
    }
    else if (field == FQ_VEL_END) {
        list_vel_end.selected = sequence->VelEnd();
        FieldEdit(field_desc_sequence[field], list_vel_end);
    }

    if (field == FQ_NEXT_POINT || field == FQ_PREV_POINT || field == FQ_NO_POINT || field == FQ_CLOSEST_POINT) {
        FieldEdit(field_desc_sequence[field]);
    }
    else if (field == FQ_NEXT_SEGMENT || field == FQ_PREV_SEGMENT || field == FQ_NO_SEGMENT || field == FQ_CLOSEST_SEGMENT) {
        FieldEdit(field_desc_sequence[field]);
    }

    //get the selected segment.
    int selected_seg = SequenceSelectedSegment();
    if (selected_seg != -1) {
        if (field == FQ_SEGMENT_TIME) {
            FieldEdit(field_desc_sequence[field], sequence->Time(selected_seg));
        }
        else if (field == FQ_START_FOV_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveStartFOV(selected_seg));
        }
        else if (field == FQ_END_FOV_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveEndFOV(selected_seg));
        }
        else if (field == FQ_START_FOV) {
            FieldEdit(field_desc_sequence[field], sequence->StartFOV(selected_seg));
        }
        else if (field == FQ_END_FOV) {
            FieldEdit(field_desc_sequence[field], sequence->EndFOV(selected_seg));
        }
        else if (field == FQ_START_TIMESCALE_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveStartTimescale(selected_seg));
        }
        else if (field == FQ_END_TIMESCALE_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveEndTimescale(selected_seg));
        }
        else if (field == FQ_START_TIMESCALE) {
            FieldEdit(field_desc_sequence[field], sequence->StartTimescale(selected_seg));
        }
        else if (field == FQ_END_TIMESCALE) {
            FieldEdit(field_desc_sequence[field], sequence->EndTimescale(selected_seg));
        }
        else if (field == FQ_START_COLOR_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveStartColor(selected_seg));
        }
        else if (field == FQ_END_COLOR_RADIO) {
            FieldEdit(field_desc_sequence[field], sequence->HaveEndColor(selected_seg));
        }
        else if (field == FQ_START_RED) {
            FieldEdit(field_desc_sequence[field], sequence->StartR(selected_seg));
        }
        else if (field == FQ_START_GREEN) {
            FieldEdit(field_desc_sequence[field], sequence->StartG(selected_seg));
        }
        else if (field == FQ_START_BLUE) {
            FieldEdit(field_desc_sequence[field], sequence->StartB(selected_seg));
        }
        else if (field == FQ_START_ALPHA) {
            FieldEdit(field_desc_sequence[field], sequence->StartA(selected_seg));
        }
        else if (field == FQ_END_RED) {
            FieldEdit(field_desc_sequence[field], sequence->EndR(selected_seg));
        }
        else if (field == FQ_END_GREEN) {
            FieldEdit(field_desc_sequence[field], sequence->EndG(selected_seg));
        }
        else if (field == FQ_END_BLUE) {
            FieldEdit(field_desc_sequence[field], sequence->EndB(selected_seg));
        }
        else if (field == FQ_END_ALPHA) {
            FieldEdit(field_desc_sequence[field], sequence->EndA(selected_seg));
        }
    }

    //get the selected point.
    int selected_pnt = SequenceSelectedPoint();
    if (selected_pnt != -1) {
        if (field == FQ_REENTER) {
            FieldEdit(field_desc_sequence[field]);
        }
        else if (field == FQ_EDIT_DIR) {
            FieldEdit(field_desc_sequence[field]);
        }
        else if (field == FQ_DELETE_POINT) {
            FieldEdit(field_desc_sequence[field]);
        }
    }
}

void edit_end_sequence(int field) {
    buffer64 buffer;

    if (field == FQ_SAVE) {
        buffer.Set("gce_seq_save\n");
    }
    else if (field == FQ_VEL_START) {
        buffer.Set("gce_seq_vel_start %d\n", FieldListChoiceSelected());
    }
    else if (field == FQ_VEL_END) {
        buffer.Set("gce_seq_vel_end %d\n", FieldListChoiceSelected());
    }
    else if (field == FQ_SEGMENT_TIME) {
        //get the value that was entered.
        float time = 0.0f;
        sscanf(FieldEditText(), "%f", &time);

        //check the value.
        if (time < 0.005f) return;

        //change the value.
        buffer.Set("gce_seq_time %.2f\n", time);
    }
    else if (field == FQ_START_FOV_RADIO) {
        buffer.Set("gce_seq_have_start_fov_toggle\n");
    }
    else if (field == FQ_END_FOV_RADIO) {
        buffer.Set("gce_seq_have_end_fov_toggle\n");
    }
    else if (field == FQ_START_FOV) {
        //get the value that was entered.
        float fov = SCRIPT_FOV_DEFAULT;
        sscanf(FieldEditText(), "%f", &fov);

        //change the value.
        buffer.Set("gce_seq_start_fov %.2f\n", fov);
    }
    else if (field == FQ_END_FOV) {
        //get the value that was entered.
        float fov = SCRIPT_FOV_DEFAULT;
        sscanf(FieldEditText(), "%f", &fov);

        //change the value.
        buffer.Set("gce_seq_end_fov %.2f\n", fov);
    }
    else if (field == FQ_START_TIMESCALE_RADIO) {
        buffer.Set("gce_seq_have_start_timescale_toggle\n");
    }
    else if (field == FQ_END_TIMESCALE_RADIO) {
        buffer.Set("gce_seq_have_end_timescale_toggle\n");
    }
    else if (field == FQ_START_TIMESCALE) {
        //get the value that was entered.
        float timescale = SCRIPT_TIMESCALE_DEFAULT;
        sscanf(FieldEditText(), "%f", &timescale);

        //change the value.
        buffer.Set("gce_seq_start_timescale %.2f\n", timescale);
    }
    else if (field == FQ_END_TIMESCALE) {
        //get the value that was entered.
        float timescale = SCRIPT_TIMESCALE_DEFAULT;
        sscanf(FieldEditText(), "%f", &timescale);

        //change the value.
        buffer.Set("gce_seq_end_timescale %.2f\n", timescale);
    }
    else if (field == FQ_START_COLOR_RADIO) {
        buffer.Set("gce_seq_have_start_color_toggle\n");
    }
    else if (field == FQ_END_COLOR_RADIO) {
        buffer.Set("gce_seq_have_end_color_toggle\n");
    }
    else if (field == FQ_START_RED) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_start_color %.2f %.2f %.2f %.2f\n", color, -1.0f, -1.0f, -1.0f);
    }
    else if (field == FQ_START_GREEN) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_start_color %.2f %.2f %.2f %.2f\n", -1.0f, color, -1.0f, -1.0f);
    }
    else if (field == FQ_START_BLUE) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_start_color %.2f %.2f %.2f %.2f\n", -1.0f, -1.0f, color, -1.0f);
    }
    else if (field == FQ_START_ALPHA) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_start_color %.2f %.2f %.2f %.2f\n", -1.0f, -1.0f, -1.0f, color);
    }
    else if (field == FQ_END_RED) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_end_color %.2f %.2f %.2f %.2f\n", color, -1.0f, -1.0f, -1.0f);
    }
    else if (field == FQ_END_GREEN) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_end_color %.2f %.2f %.2f %.2f\n", -1.0f, color, -1.0f, -1.0f);
    }
    else if (field == FQ_END_BLUE) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_end_color %.2f %.2f %.2f %2.f\n", -1.0f, -1.0f, color, -1.0f);
    }
    else if (field == FQ_END_ALPHA) {
        //get the value that was entered.
        float color = 0.0f;
        sscanf(FieldEditText(), "%f", &color);
        
        //change the value.
        buffer.Set("gce_seq_end_color %.2f %.2f %.2f %2.f\n", -1.0f, -1.0f, -1.0f, color);
    }
    else if (field == FQ_CLOSEST_SEGMENT) {
        buffer.Set("gce_seq_select_segment closest\n");
    }
    else if (field == FQ_NEXT_SEGMENT) {
        buffer.Set("gce_seq_select_segment next\n");
    }
    else if (field == FQ_PREV_SEGMENT) {
        buffer.Set("gce_seq_select_segment prev\n");
    }
    else if (field == FQ_NO_SEGMENT) {
        buffer.Set("gce_seq_select_segment clear\n");
    }
    else if (field == FQ_CLOSEST_POINT) {
        buffer.Set("gce_seq_select_point closest\n");
    }
    else if (field == FQ_NEXT_POINT) {
        buffer.Set("gce_seq_select_point next\n");
    }
    else if (field == FQ_PREV_POINT) {
        buffer.Set("gce_seq_select_point prev\n");
    }
    else if (field == FQ_NO_POINT) {
        buffer.Set("gce_seq_select_point clear\n");
    }
    else if (field == FQ_DELETE_POINT) {
        buffer.Set("gce_seq_delete_point\n");
    }

    //get the selected point.
    int selected_pnt = SequenceSelectedPoint();
    if (selected_pnt != -1) {
        if (field == FQ_REENTER) {
            buffer.Set("gce_seq_edit_point_loc\n");
        }
        else if (field == FQ_EDIT_DIR) {
            buffer.Set("gce_seq_edit_point_angles\n");
        }
    }

    if (buffer) server->CBuf_AddText(buffer);
}

//#define SEQUENCE_COL0	100
#define SEQUENCE_COL0	10

void HUD_Sequence() {
    buffer128 buffer;

    //draw the sequence display screen header text.
    cin->DrawString(SEQUENCE_COL0 + 36,30, SequenceHeader());

    //draw the save field.
    FieldDraw(field_desc_sequence[FQ_SAVE], SEQUENCE_COL0, 10);

    //draw the modify point buttons.
    FieldDraw(field_desc_sequence[FQ_REENTER], 300, 10);
    FieldDraw(field_desc_sequence[FQ_EDIT_DIR], 450, 10);

    //get the current sequence.
    const CSplineSequence *sequence = SequenceCurrent();
    if (sequence == NULL) return;

    cin->DrawString(SEQUENCE_COL0, 40, "Current Sequence:");

    int y = 30;

    //get the starting and end velocity.
    list_vel_start.selected = sequence->VelStart();
    list_vel_end.selected = sequence->VelEnd();

    //draw the fields.
    FieldDraw(field_desc_sequence[FQ_VEL_START], 300, y, list_vel_start); y += 10;
    FieldDraw(field_desc_sequence[FQ_VEL_END], 300, y, list_vel_end); y += 10;

    //get the number of points in the sequence.
    int num_points = sequence->NumPoints();

    buffer.Set("Num points: %d", num_points);
    cin->DrawString(SEQUENCE_COL0 + 54, y, buffer); y += 10;

    //get the total time for the sequence.
    float total_time = sequence->TotalTime();
    buffer.Set("Time of sequence: %.2f", total_time);
    cin->DrawString(SEQUENCE_COL0 + 5, y, buffer); y += 10;

    //draw the segment selection buttons.
    y += 10;
    cin->DrawString(SEQUENCE_COL0, y, "Select a sequence segment:"); y += 15;
    FieldDraw(field_desc_sequence[FQ_PREV_SEGMENT], SEQUENCE_COL0, y);
    FieldDraw(field_desc_sequence[FQ_NO_SEGMENT], SEQUENCE_COL0 + 40, y);
    FieldDraw(field_desc_sequence[FQ_NEXT_SEGMENT], SEQUENCE_COL0 + 97, y);
    FieldDraw(field_desc_sequence[FQ_CLOSEST_SEGMENT], SEQUENCE_COL0 + 139, y); y += 15;

    //get the selected segment.
    int selected = SequenceSelectedSegment();
    if (selected != -1) {
        buffer.Set("Selected segment: %d", selected);
        cin->DrawString(SEQUENCE_COL0 + 62, y, buffer); y += 10;

        //get the time of the selected segment.
        FieldDraw(field_desc_sequence[FQ_SEGMENT_TIME], SEQUENCE_COL0, y, sequence->Time(selected)); 
        y += 13;

        //draw the radio buttons for the FOV settings.
        FieldDraw(field_desc_sequence[FQ_START_FOV_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveStartFOV(selected));
        if (sequence->HaveStartFOV(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_START_FOV], SEQUENCE_COL0 + 220, y, sequence->StartFOV(selected));
        }
        y += 12;

        FieldDraw(field_desc_sequence[FQ_END_FOV_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveEndFOV(selected));
        if (sequence->HaveEndFOV(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_END_FOV], SEQUENCE_COL0 + 220, y, sequence->EndFOV(selected));
        }
        y += 12;
        
        //draw the radio buttons for the timescale settings.
        FieldDraw(field_desc_sequence[FQ_START_TIMESCALE_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveStartTimescale(selected));
        if (sequence->HaveStartTimescale(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_START_TIMESCALE], SEQUENCE_COL0 + 220, y, sequence->StartTimescale(selected));
        }
        y += 12;

        FieldDraw(field_desc_sequence[FQ_END_TIMESCALE_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveEndTimescale(selected));
        if (sequence->HaveEndTimescale(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_END_TIMESCALE], SEQUENCE_COL0 + 220, y, sequence->EndTimescale(selected));
        }
        y += 12;
        
        //draw the radio buttons for the color settings.
        FieldDraw(field_desc_sequence[FQ_START_COLOR_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveStartColor(selected));
        if (sequence->HaveStartColor(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_START_RED], SEQUENCE_COL0 + 220, y, sequence->StartR(selected));
            FieldDraw(field_desc_sequence[FQ_START_GREEN], SEQUENCE_COL0 + 320, y, sequence->StartG(selected));
            FieldDraw(field_desc_sequence[FQ_START_BLUE], SEQUENCE_COL0 + 420, y, sequence->StartB(selected));
            FieldDraw(field_desc_sequence[FQ_START_ALPHA], SEQUENCE_COL0 + 520, y, sequence->StartA(selected));
        }
        y += 12;

        FieldDraw(field_desc_sequence[FQ_END_COLOR_RADIO], SEQUENCE_COL0 + 20, y, sequence->HaveEndColor(selected));
        if (sequence->HaveEndColor(selected) == true) {
            FieldDraw(field_desc_sequence[FQ_END_RED], SEQUENCE_COL0 + 220, y, sequence->EndR(selected));
            FieldDraw(field_desc_sequence[FQ_END_GREEN], SEQUENCE_COL0 + 320, y, sequence->EndG(selected));
            FieldDraw(field_desc_sequence[FQ_END_BLUE], SEQUENCE_COL0 + 420, y, sequence->EndB(selected));
            FieldDraw(field_desc_sequence[FQ_END_ALPHA], SEQUENCE_COL0 + 520, y, sequence->EndA(selected));
        }
        y += 12;
    }

    //draw the point selection buttons.
    y += 10;
    cin->DrawString(SEQUENCE_COL0, y, "Select a sequence point:"); y += 15;
    FieldDraw(field_desc_sequence[FQ_PREV_POINT], SEQUENCE_COL0, y);
    FieldDraw(field_desc_sequence[FQ_NO_POINT], SEQUENCE_COL0 + 40, y);
    FieldDraw(field_desc_sequence[FQ_NEXT_POINT], SEQUENCE_COL0 + 97, y);
    FieldDraw(field_desc_sequence[FQ_CLOSEST_POINT], SEQUENCE_COL0 + 139, y); y += 15;

    //get the selected point.
    selected = SequenceSelectedPoint();
    if (selected != -1) {
        buffer.Set("Selected point: %d", selected);
        cin->DrawString(SEQUENCE_COL0, y, buffer); y += 15;

        //draw the delete point button.
        FieldDraw(field_desc_sequence[FQ_DELETE_POINT], SEQUENCE_COL0, y); y += 15;
    }
}

