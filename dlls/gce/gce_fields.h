#pragma once


//the different types of fields.
typedef enum {
    FT_FLOAT,
    FT_INT,
    FT_STRING,
    FT_LIST,
    FT_BUTTON,
    FT_RADIO,
} field_type;


//information needed to draw and edit a field.
typedef struct {
    //the type of field.
    field_type type;

    //the maximum number of characters available.
    int width;

    //text that is displayed to the left of the field.
    char *text;

    //coordinate of the field.
    int x, y;
} field_description;

//structure used to draw a list field.
typedef struct {
    //the array of strings we display
    char **items;
    //the number of selections.
    int num_items;
    //the item that is selected.
    int selected;
} list_field_description;

//info we need for each hud mode.
typedef struct {
    //number of fields that we can edit in this mode.
    int fields;

    //the current or last field that was active.
    int cur_field;

    //function that returns true or false depending if the field can be edited.
    bool (*field_mask)(int field);

    //function that begins the edit process on the given field.
    void (*edit_start)(int field);

    //function that ends the edit process, the value is saved.
    //for buttons, this function is called when the button is clicked with the mouse.
    void (*edit_end)(int field);
} hud_info;


bool FieldEditing();

void FieldEdit(field_description &desc, const char *string_val);
void FieldEdit(field_description &desc, int integer_val);
void FieldEdit(field_description &desc, float float_val);
void FieldEdit(field_description &desc, list_field_description &list_val);
void FieldEdit(field_description &desc);    //for button types.
void FieldEdit(field_description &desc, bool radio_state);

void FieldDraw(field_description &desc, int x, int y, const char *string_val);
void FieldDraw(field_description &desc, int x, int y, int integer_val);
void FieldDraw(field_description &desc, int x, int y, float float_val);
void FieldDraw(field_description &desc, int x, int y, list_field_description &list_val);
void FieldDraw(field_description &desc, int x, int y);  //for button types.
void FieldDraw(field_description &desc, int x, int y, bool radio_state);

//void FieldDrawBox(field_description &desc, draw_color color);
void FieldDrawBox(field_description &desc, CVector& rgbColor);

//computes the hilight box for this field.
void FieldBox(field_description &desc, int &left, int &top, int &right, int &bottom);

int FieldFind(hud_info &info, int start_field);
void FieldCycle(hud_info &info);
int FieldKeyEvent(int key, hud_info &info);

//when we are done editing, these functions give us info about what the user has input.
int FieldListChoiceSelected();
const char *FieldEditText();
