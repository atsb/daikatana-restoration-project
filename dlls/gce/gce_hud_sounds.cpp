#include "p_user.h"

#include "dk_array.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"

#include "dk_gce_script.h"

#include "gce_main.h"
#include "gce_fields.h"
#include "gce_hud.h"
#include "gce_sound.h"
#include "gce_script.h"


typedef enum {
    FN_SAVE,
    FN_ADD,
    FN_DELETE,

    FN_SCROLL_UP,
    FN_SCROLL_DOWN,

    //we have a fixed maximum number of sounds that we can edit the name/time for
    FN_TIME0,
    FN_TIME1,
    FN_TIME2,
    FN_TIME3,
    FN_TIME4,
    FN_TIME5,
    FN_TIME6,
    FN_TIME7,
    FN_TIME8,
    FN_TIME9,

    FN_CHAN0,
    FN_CHAN1,
    FN_CHAN2,
    FN_CHAN3,
    FN_CHAN4,
    FN_CHAN5,
    FN_CHAN6,
    FN_CHAN7,
    FN_CHAN8,
    FN_CHAN9,

    FN_LOOP0,
    FN_LOOP1,
    FN_LOOP2,
    FN_LOOP3,
    FN_LOOP4,
    FN_LOOP5,
    FN_LOOP6,
    FN_LOOP7,
    FN_LOOP8,
    FN_LOOP9,

    FN_NAME0,
    FN_NAME1,
    FN_NAME2,
    FN_NAME3,
    FN_NAME4,
    FN_NAME5,
    FN_NAME6,
    FN_NAME7,
    FN_NAME8,
    FN_NAME9,

    FN_NUM_FIELDS,
} fields_sounds;

int num_fields_sounds = FN_NUM_FIELDS;

field_description field_desc_sounds[] = {
    {FT_BUTTON, 12, "Save Sounds"},
    {FT_BUTTON, 12, "Add Sound"},
    {FT_BUTTON, 14, "Delete Sound"},

    {FT_BUTTON, 2, "^"},
    {FT_BUTTON, 2, "V"},

    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},
    {FT_FLOAT, 6, ""},

    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},
    {FT_INT, 2, ""},

    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},
    {FT_RADIO, 5, ""},

    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
    {FT_STRING, 36, ""}, 
};

fields_sounds fields_sounds_names[] = {FN_NAME0, FN_NAME1, FN_NAME2, FN_NAME3, FN_NAME4, FN_NAME5, FN_NAME6, FN_NAME7, FN_NAME8, FN_NAME9};
fields_sounds fields_sounds_chans[] = {FN_CHAN0, FN_CHAN1, FN_CHAN2, FN_CHAN3, FN_CHAN4, FN_CHAN5, FN_CHAN6, FN_CHAN7, FN_CHAN8, FN_CHAN9};
fields_sounds fields_sounds_loops[] = {FN_LOOP0, FN_LOOP1, FN_LOOP2, FN_LOOP3, FN_LOOP4, FN_LOOP5, FN_LOOP6, FN_LOOP7, FN_LOOP8, FN_LOOP9};
fields_sounds fields_sounds_times[] = {FN_TIME0, FN_TIME1, FN_TIME2, FN_TIME3, FN_TIME4, FN_TIME5, FN_TIME6, FN_TIME7, FN_TIME8, FN_TIME9};

//when we are in sound mode, we use this info to keep track of what fields are visible.
static int sounds_num_visible_lines = 0;
static int sounds_top_index = 0;
static int sounds_selected_row = -1;

bool field_mask_sounds(int field) {
    //get the array of sounds we are editing.
    const array<CScriptSound> &sounds = SoundCurrent();

    if (field == FN_SAVE) return true;
    else if (field == FN_ADD) return true;
    else if (field == FN_DELETE) {
        if (sounds_selected_row != -1) return true;
    }

    if (field == FN_SCROLL_DOWN) {
        if (sounds.Num() - sounds_top_index > 10) return true;
    }

    if (field == FN_SCROLL_UP) {
        if (sounds_top_index > 0) return true;
    }

    //check if its a name field.
    else if (field >= FN_NAME0 && field <= FN_NAME9) {
        //get the index 
        int index = field - FN_NAME0;
        if (index < sounds_num_visible_lines) return true;
    }
    //check if its a loop field.
    else if (field >= FN_LOOP0 && field <= FN_LOOP9) {
        //get the index 
        int index = field - FN_LOOP0;
        if (index < sounds_num_visible_lines) return true;
    }
    //check if its a channel field
    else if (field >= FN_CHAN0 && field <= FN_CHAN9) {
        //get the index.
        int index = field - FN_CHAN0;
        if (index < sounds_num_visible_lines) return true;
    }
    //check if its a time field
    else if (field >= FN_TIME0 && field <= FN_TIME9) {
        //get the index.
        int index = field - FN_TIME0;
        if (index < sounds_num_visible_lines) return true;
    }

    return false;
}

void edit_start_sounds(int field) {
    //get the array of sounds we are editing.
    const array<CScriptSound> &sounds = SoundCurrent();

    if (field == FN_SAVE) {
        FieldEdit(field_desc_sounds[field]);
    }
    else if (field == FN_ADD) {
        FieldEdit(field_desc_sounds[field]);
    }
    else if (field == FN_DELETE) {
        FieldEdit(field_desc_sounds[field]);
    }
    else if (field >= FN_NAME0 && field <= FN_NAME9) {
        //get the index.
        int index = field - FN_NAME0;
        //get the sound
        const CScriptSound *sound = sounds.Item(index + sounds_top_index);
        //get the initial name.
        const char *name = "";
        if (sound != NULL) {
            name = sound->Name();
        }
        FieldEdit(field_desc_sounds[field], name);
    }
    else if (field >= FN_LOOP0 && field <= FN_LOOP9) {
        //get the index.
        int index = field - FN_LOOP0;
        //get the sound
        const CScriptSound *sound = sounds.Item(index + sounds_top_index);
        //get the initial value.
        bool loop=false;
        if (sound != NULL) {
            loop = sound->Loop();
        }
        FieldEdit(field_desc_sounds[field], loop);
    }
    else if (field >= FN_CHAN0 && field <= FN_CHAN9) {
        //get the index
        int index = field - FN_CHAN0;
        //get the sound.
        const CScriptSound *sound = sounds.Item(index + sounds_top_index);
        //get the initial value of the field.
        int channel = 0;
        if (sound != NULL) {
            channel = sound->Channel();
        }
        FieldEdit(field_desc_sounds[field], channel);
    }
    else if (field >= FN_TIME0 && field <= FN_TIME9) {
        //get the index
        int index = field - FN_TIME0;
        //get the sound.
        const CScriptSound *sound = sounds.Item(index + sounds_top_index);
        //get the initial value of the field.
        float time = 0.0f;
        if (sound != NULL) {
            time = sound->Time();
        }
        FieldEdit(field_desc_sounds[field], time);
    }
    else if (field == FN_SCROLL_DOWN) {
        FieldEdit(field_desc_sounds[field]);
    }
    else if (field == FN_SCROLL_UP) {
        FieldEdit(field_desc_sounds[field]);
    }
}

void edit_end_sounds(int field) {
    buffer128 buffer;
    //get the array of sounds we are editing.
    const array<CScriptSound> &sounds = SoundCurrent();

    if (field == FN_SAVE) {
        buffer.Set("gce_sound_save\n");
    }
    else if (field == FN_ADD) {
        buffer.Set("gce_sound_add\n");
    }
    else if (field == FN_DELETE) {
        buffer.Set("gce_sound_delete %d\n", sounds_selected_row + sounds_top_index);
        //unselect whatever row we have selected.
        sounds_selected_row = -1;
    }
    //check if its a name field.
    else if (field >= FN_NAME0 && field <= FN_NAME9) {
        //get the index 
        int index = field - FN_NAME0;
        buffer.Set("gce_sound_name %d \"%s\"\n", index + sounds_top_index, FieldEditText());
    }
    //check if its a loop field.
    else if (field >= FN_LOOP0 && field <= FN_LOOP9) {
        //get the index 
        int index = field - FN_LOOP0;
        //get the sound.
        const CScriptSound *sound = sounds.Item(index + sounds_top_index);
        //flip the loop state
        bool fLoop = !sound->Loop();
        int loop = (1?(true==fLoop):0);
        buffer.Set("gce_sound_loop %d %d\n", index + sounds_top_index, loop);
    }
    //check if its a channel field.
    else if (field >= FN_CHAN0 && field <= FN_CHAN9) {
        //get the index.
        int index = field - FN_CHAN0;
        //get the value.
        int channel = atof(FieldEditText());
        buffer.Set("gce_sound_channel %d %d\n", index + sounds_top_index, channel);
    }
    //check if its a time field
    else if (field >= FN_TIME0 && field <= FN_TIME9) {
        //get the index.
        int index = field - FN_TIME0;
        //get the value.
        float time = atof(FieldEditText());
        buffer.Set("gce_sound_time %d %.2f\n", index + sounds_top_index, time);
    }
    else if (field == FN_SCROLL_DOWN) {
        sounds_top_index++;
    }
    else if (field == FN_SCROLL_UP) {
        sounds_top_index--;
    }

    if (buffer) server->CBuf_AddText(buffer);
}

#define ROW_HEIGHT 15

#define SOUND_RIGHT 620
#define SOUND_LEFT 10
#define SOUND_COL0 120 - 40
#define SOUND_COL1 200 - 40
#define SOUND_COL2 250 - 40
#define SOUND_COL3 300 - 40

static int sounds_rows_top;
static int sounds_rows_bottom;


void HUD_Sounds() {
    buffer128 buffer;
    int y = 100;

    //print help string.
    cin->DrawString(50, 450, "Filenames have extensions and are relative to data/sounds directory.");

    //print the shot number that is being edited.
    if (ScriptCurrentShot() != -1) {
        buffer.Set("Current Shot: %d", ScriptCurrentShot());
        cin->DrawString(10, 10, buffer);
    }

    //print the total time of the shot that is being edited.
    if (ScriptCurrent() != NULL && ScriptCurrent()->Shot(ScriptCurrentShot()) != NULL) {
        buffer.Set("Shot total time: %.2f", ScriptCurrent()->Shot(ScriptCurrentShot())->TotalTime());
        cin->DrawString(250, 10, buffer);
    }

    //get the sounds array we are editing.
    const array<CScriptSound> &sounds = SoundCurrent();

    //draw the save button.
    FieldDraw(field_desc_sounds[FN_SAVE], 100, 30);

    //draw the add button
    FieldDraw(field_desc_sounds[FN_ADD], 230, 30);

    //draw the delete button
    FieldDraw(field_desc_sounds[FN_DELETE], 360, 30);

    //get the number of sounds.
    int num_sounds = sounds.Num();

    buffer.Set("Num Sounds: %d", num_sounds);
    cin->DrawString(100, y, buffer); y += ROW_HEIGHT;

    if (num_sounds < 1) {
        //make sure our vars are consistent with having no sounds in our array.
        sounds_num_visible_lines = 0;
        sounds_selected_row = -1;
        sounds_rows_top = -1;
        sounds_rows_bottom = -1;
        return;
    }

    //check the validity of the current value of sounds_top_index.
    bound_max(sounds_top_index, sounds.Num() - 10);
    bound_min(sounds_top_index, 0);

    //draw the column heading for the table.
    HUDDrawBox(SOUND_COL0, y, SOUND_RIGHT, y + ROW_HEIGHT, CVector(0.0, 0.0, 0.0));
    cin->DrawString(SOUND_COL0 + 10, y + 4, "Time");
    cin->DrawString(SOUND_COL1 + 10, y + 4, "Chan");
    cin->DrawString(SOUND_COL2 + 10, y + 4, "Loop");
    cin->DrawString(SOUND_COL3 + 10, y + 4, "File");

    //draw a horizontal line above the column headings.
    HUDDrawBox(SOUND_COL0, y, SOUND_RIGHT, y, CVector(0.5, 0.5, 0.5));

    y += ROW_HEIGHT;

    //draw the scroll up button.
    if (sounds_top_index > 0) {
        FieldDraw(field_desc_sounds[FN_SCROLL_UP], 15, y);
    }

    //decide how many of our fields we should draw.
    sounds_num_visible_lines = sounds.Num() - sounds_top_index;
    bound_max(sounds_num_visible_lines, 10);

    //draw the scroll down button.
    if (sounds.Num() - sounds_top_index > 10) {
        FieldDraw(field_desc_sounds[FN_SCROLL_DOWN], 15, y + 20);
    }

    //draw the background behind the row headings.
    HUDDrawBox(SOUND_LEFT, y, SOUND_COL0, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.0, 0.0, 0.0));

    //draw a different color background behind the selected row heading.
    if (sounds_selected_row >= 0) {
        HUDDrawBox(SOUND_LEFT, y + sounds_selected_row * ROW_HEIGHT, 
                   SOUND_COL0, y + (1 + sounds_selected_row) * ROW_HEIGHT, CVector(1.0, 1.0, 1.0));
    }

    //draw vertical lines separating the columns.
    HUDDrawBox(SOUND_COL0, y - ROW_HEIGHT, SOUND_COL0, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SOUND_COL1, y - ROW_HEIGHT, SOUND_COL1, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SOUND_COL2, y - ROW_HEIGHT, SOUND_COL2, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SOUND_COL3, y - ROW_HEIGHT, SOUND_COL3, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SOUND_RIGHT, y - ROW_HEIGHT, SOUND_RIGHT, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));

    //draw vertical lines on left of row labels.
    HUDDrawBox(SOUND_LEFT, y, SOUND_LEFT, y + sounds_num_visible_lines * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));

    sounds_rows_top = y;

    //draw the rows.
    for (int i = 0; i < sounds_num_visible_lines; i++) {
        //draw the row number.
        buffer.Set("%d", i + sounds_top_index);
        cin->DrawString(SOUND_LEFT + 10, 4 + y + i * ROW_HEIGHT, buffer);
        
        //draw the horizontal line above this column.
        HUDDrawBox(SOUND_LEFT, y + i * ROW_HEIGHT, SOUND_RIGHT, y + i * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));

        //get the name and time for the sound.
        const CScriptSound *sound = sounds.Item(i + sounds_top_index);
        if (sound == NULL) break;

        //draw the time field.
        FieldDraw(field_desc_sounds[fields_sounds_times[i]], SOUND_COL0 + 10, 4 + y + i * ROW_HEIGHT, sound->Time());

        //draw the channel field.
        FieldDraw(field_desc_sounds[fields_sounds_chans[i]], SOUND_COL1 + 20, 4 + y + i * ROW_HEIGHT, sound->Channel());

        //draw the loop field.
        FieldDraw(field_desc_sounds[fields_sounds_loops[i]], SOUND_COL2 + 20, 4 + y + i * ROW_HEIGHT, sound->Loop());
        
        //draw the name field.
        FieldDraw(field_desc_sounds[fields_sounds_names[i]], SOUND_COL3 + 10, 4 + y + i * ROW_HEIGHT, sound->Name());
    }

    sounds_rows_bottom = y + i * ROW_HEIGHT;

    //draw horizontal line at bottom of table.
    HUDDrawBox(SOUND_LEFT, sounds_rows_bottom, SOUND_RIGHT, sounds_rows_bottom, CVector(0.5, 0.5, 0.5));
}

void MouseDownSounds(int x, int y) {
    //check if we are clicking on a row label.
    if (x >= SOUND_LEFT && x <= SOUND_COL0 && y >= sounds_rows_top && y <= sounds_rows_bottom) {
        //get the row that was clicked.
        int row = (y - sounds_rows_top) / ROW_HEIGHT;
        bound_min(row, 0);
        bound_max(row, sounds_num_visible_lines - 1);

        sounds_selected_row = row;
    }
}

