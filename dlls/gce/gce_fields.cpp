#include "p_user.h"
#include "keys.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "gce_main.h"
#include "gce_fields.h"
#include "gce_input.h"
#include "gce_hud.h"

#include "dk_buffer.h"

extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount(void);

//the field we are editing.
static field_description *edit_field = NULL;
//the text we have in the field we are editing.
static buffer128 edit_text;
//if we are editing a field with type FT_LIST, the description for the list.
static list_field_description *edit_list = NULL;
//the currently selected list choice 
static int list_choice_selected = 0;

int FieldListChoiceSelected() {
    return list_choice_selected;
}

const char *FieldEditText() {
    return edit_text;
}

//returns true if we are editing a field.
bool FieldEditing() {
    return edit_field != NULL;
}

int FieldKeyEvent(int key, hud_info &info) {
    if (edit_field == NULL) return 0;

    //if they hit tab, go to the next field.
    if (key == K_TAB) {
        server->CBuf_AddText("gce_hud_field");
        return 1;
    }

    if (key == K_ESCAPE) {
        //abort our editing.
        edit_field = NULL;
        return 1;
    }
    else if (key == K_ENTER || key == K_KP_ENTER) {
        //done editing.
        info.edit_end(info.cur_field);
        edit_field = NULL;
        return 1;
    }
    //check if we have one of the string-based types.
    else if (edit_field->type == FT_FLOAT || edit_field->type == FT_INT || edit_field->type == FT_STRING) {
        //get the length of the string we are editing so far.
        int len = strlen(edit_text);

        if (key == K_BACKSPACE) {
            //delete a character from the edit string.
            if (len > 0) {
                edit_text.NewEnding(len - 1, "");
            }
            return 1;
        }
        //check if we have any room for characters in our string.
        else if (len >= edit_field->width) {
            //no more room.
            return 1;
        }
        //check our field type.
        else if (edit_field->type == FT_FLOAT) {
            //decide what character we will add to our 
            char add_char = -1;
            //allow numbers.
            if (key >= '0' && key <= '9') {
                add_char = key;
            }
            //allow a - if it is the first character.
            else if (key == '-' && len == 0) {
                add_char = '-';
            }
            //allow a decimal if we dont have one already.
            else if (key == '.' && strchr(edit_text, '.') == NULL) {
                add_char = '.';
            }

            //check if we are going to add this character.
            if (add_char != -1) {
                //add the character.
                char new_ending[2] = " ";
                new_ending[0] = add_char;

                edit_text.NewEnding(len, new_ending);
            }
        }
        else if (edit_field->type == FT_INT) {
            //decide what character we will add to our 
            char add_char = -1;
            //allow numbers.
            if (key >= '0' && key <= '9') {
                add_char = key;
            }
            //allow a - if it is the first character.
            else if (key == '-' && len == 0) {
                add_char = '-';
            }

            //check if we are going to add this character.
            if (add_char != -1) {
                //add the character.
                char new_ending[2] = " ";
                new_ending[0] = add_char;

                edit_text.NewEnding(len, new_ending);
            }
        }
        else if (edit_field->type == FT_STRING) {
            char add_char = -1;

            //decide what characters we will add to the string.
            if (key >= '0' && key <= '9') {
                add_char = key;
            }
            else if (key >= 'a' && key <= 'z') {
                add_char = key;
            }
            else if (key >= 'A' && key <= 'Z') {
                add_char = key;
            }
            else if (key == '.' || key == '_' || key == '-') {
                add_char = key;
            }
            //allow spaces if it is not the first character.
            else if (key == ' ' && len != 0) {
                add_char = key;
            }
            else if (key == '/' || key == K_KP_SLASH) {
                add_char = '/';
            }

            //check if we will allow the character.
            if (add_char != -1) {
                //add the character.
                char new_ending[2] = " ";
                new_ending[0] = add_char;

                edit_text.NewEnding(len, new_ending);
            }
        }
    }
    else if (edit_field->type == FT_LIST) {
        //check what character was hit.
        if (key == K_LEFTARROW || key == K_UPARROW) {
            //go to the previous list entry.
            list_choice_selected--;
            if (list_choice_selected < 0) {
                list_choice_selected = edit_list->num_items - 1;
            }
        }
        else if (key == K_RIGHTARROW || key == K_DOWNARROW) {
            list_choice_selected++;
            if (list_choice_selected >= edit_list->num_items) {
                list_choice_selected = 0;
            }
        }
    }

    return 1;
}

void FieldCycle(hud_info &info) {
    //get the field that was last active.
    int &cur_field = info.cur_field;

    //check if we are already editing a field.
    if (edit_field != NULL) {
        //we need to abort editing whatever field we are editing our last one.
        InputKeyEvent(K_ESCAPE, -1, false);

        //look for another field.
        cur_field = FieldFind(info, cur_field);
        if (cur_field != -1) {
            info.edit_start(cur_field);
        }
    }
    else {
        //nothing is selected right now.

        //check if we have ever had a field selected in this menu.
        if (cur_field == -1) {
            //no field selected yet.
            //check if we have any fields.
            cur_field = FieldFind(info, cur_field);
            if (cur_field != -1) {
                //start editing the field.
                info.edit_start(cur_field);
            }
        }
        else {
            //check if the last field we edited is still available.
            if (info.field_mask(cur_field) == true) {
                //edit this field
                info.edit_start(cur_field);
            }
            else {
                //look for another field.
                cur_field = FieldFind(info, cur_field);
                if (cur_field != -1) {
                    info.edit_start(cur_field);
                }
            }
        }
    }
}

int FieldFind(hud_info &info, int start_field) {
    //check if we have any fields available
    if (info.fields < 1) return -1;

    //go through all the fields.
    for (int i = start_field + 1; i != start_field; i++) {
        //check if we need to go back to the beginning field.
        if (i >= info.fields) {
            //if we started at -1 or 0, we're done now.
            if (start_field == -1 || start_field == 0) break;
            //start looking at the first field.
            i = 0;
        }

        //check if this field is available.
        if (info.field_mask(i) == true) {
            //this field is good.
            return i;
        }
    }

    //no fields available.
    return -1;
}

int TextWidth(char *pText)
{
    int width = 0;

    char *pString = pText;

    for( ;pString[ 0 ] != '\0'; pString++ )
    {
        unsigned char c = pString[ 0 ];

        if( c == ' ' )
            width += 4;
        else
            width += 9;
    }

    return width;
}

void FieldBox(field_description &desc, int &left, int &top, int &right, int &bottom) {
    //get the position for this field.
    int x = desc.x;
    int y = desc.y;

    if (desc.type == FT_FLOAT || desc.type == FT_INT || desc.type == FT_STRING) {
        //get the bounding rectanlge of the input area.
        left = x + TextWidth(desc.text) - 2;
        top = y - 2;
        right = left + desc.width * 8 + 4;
        bottom = top + 8 + 2;
    }
    else if (desc.type == FT_LIST) {
        //get the bounding rectangle of the list area.
        left = x + TextWidth(desc.text) - 2;
        top = y - 2;
        right = left + (desc.width + 2) * 8 + 4;
        bottom = y + 8;
    }
    else if (desc.type == FT_BUTTON) {
        left = x - 3;
        top = y - 4;
        right = x + desc.width * 8 + 3;
        bottom = y + 8 + 2;
    }
    else if (desc.type == FT_RADIO) {
        left = x - 2;
        top = y - 2;
        
        right = x + 10 + TextWidth(desc.text);
        bottom = y + 10;
    }
    else {
        left = right = top = bottom = -1;
    }
}

void FieldDrawBox(field_description &desc, CVector& rgbColor) {
    //get the bounding rectangle for the field.
    int left, top, right, bottom;
    FieldBox(desc, left, top, right, bottom);

    //draw the box.
    HUDDrawEmptyBox(left, top, right, bottom, rgbColor);
}

static char cursor_string[] = "|/-\\";
static int cursor_len = sizeof(cursor_string)/sizeof(char) - 1;
#define cursor_speed 75
static int cursor_cycle = cursor_len * cursor_speed;

void FieldDraw(field_description &desc, int x, int y, float float_val) {
    if (desc.type != FT_FLOAT) return;

    buffer128 buffer;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //check if we are editing this field.
    if (edit_field == &desc) {
        //draw the box around it.
        FieldDrawBox(desc, CVector( 0.6078, 0.4509, 0.3921 ));

        //the character we display as the cursor in the editing field.
        char cursor = cursor_string[(GetTickCount() % cursor_cycle) / cursor_speed];

        buffer.Set("%s%s%c", desc.text, (const char *)edit_text, cursor);
        cin->DrawString(x, y, buffer);
    }
    else {
        buffer.Set("%s%.2f", desc.text, float_val);
    
        //draw the text.
        cin->DrawString(x, y, buffer);
    }
}

void FieldDraw(field_description &desc, int x, int y, int integer_val) {
    if (desc.type != FT_INT) return;

    buffer128 buffer;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //check if we are editing this field.
    if (edit_field == &desc) {
        //draw the box around it.
        FieldDrawBox(desc, CVector( 0.6078, 0.4509, 0.3921 ));

        //the character we display as the cursor in the editing field.
        char cursor = cursor_string[(GetTickCount() % cursor_cycle) / cursor_speed];

        buffer.Set("%s%s%c", desc.text, (const char *)edit_text, cursor);
        cin->DrawString(x, y, buffer);
    }
    else {
        buffer.Set("%s%d", desc.text, integer_val);
    
        //draw the text.
        cin->DrawString(x, y, buffer);
    }
}

void FieldDraw(field_description &desc, int x, int y, const char *string) {
    if (desc.type != FT_STRING) return;
    if (string == NULL) string = "";

    buffer128 buffer;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //check if we are editing this field.
    if (edit_field == &desc) {
        //draw the box around it.
        FieldDrawBox(desc, CVector( 0.6078, 0.4509, 0.3921 ));

        //the character we display as the cursor in the editing field.
        char cursor = cursor_string[(GetTickCount() % cursor_cycle) / cursor_speed];

        buffer.Set("%s%s%c", desc.text, (const char *)edit_text, cursor);
        cin->DrawString(x, y, buffer);
    }
    else {
        buffer.Set("%s%s", desc.text, string);
    
        //draw the text.
        cin->DrawString(x, y, buffer);
    }
}

void FieldDraw(field_description &desc, int x, int y, list_field_description &list) {
    if (desc.type != FT_LIST) return;

    buffer128 buffer;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //check if we are editing this field.
    if (edit_field == &desc) {
        //draw the box around it.
        FieldDrawBox(desc, CVector( 0.6078, 0.4509, 0.3921 ));

        //get the string that is currently selected
        const char *selected_item = list.items[list_choice_selected];

        //our cursor character index.
        int index = (GetTickCount() % cursor_cycle) / cursor_speed;

        buffer.Set("%s%c%-*s%c", desc.text, cursor_string[index], edit_field->width, selected_item, cursor_string[cursor_len - 1 - index]);
        cin->DrawString(x, y, buffer);
    }
    else {
        //get the list field descriptor
        buffer.Set("%s %s", desc.text, list.items[list.selected]);
    
        //draw the text.
        cin->DrawString(x, y, buffer);
    }
}

void FieldDraw(field_description &desc, int x, int y, bool radio_state) {
    if (desc.type != FT_RADIO) return;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //draw the small box that shows the state of the radio button.
    HUDDrawEmptyBox(x, y, x + 8, y + 8, CVector( 1.0, 1.0, 1.0));

    //draw the description.
    cin->DrawString(x + 12, y, desc.text);

    //check if we are selected.
    if (edit_field == &desc) {

    }
    else {
        //fill in the box if the radio button is on.
        if (radio_state == true) {
            HUDDrawBox(x + 1, y + 1, x + 7, y + 7, CVector(1.0, 0.0, 0.0));
        }
    }
}

void FieldDraw(field_description &desc, int x, int y) {
    if (desc.type != FT_BUTTON) return;

    buffer128 buffer;

    //save the position we are drawing this field at
    desc.x = x;
    desc.y = y;

    //check if we are editing this field.
    if (edit_field == &desc) {
        //draw the box around it.
        FieldDrawBox(desc, CVector( 0.6078, 0.4509, 0.3921 ));

        //get the box for the button.
        int left, top, right, bottom;
        FieldBox(desc, left, top, right, bottom);

        //draw the box
        HUDDrawBox(left + 1, top + 1, right - 1, bottom - 1, CVector(1.0, 0.0, 0.0));

        //put the button text in buffer.
        buffer.Set("%s", desc.text);

        //move x over so that the text is centered.
        x += (desc.width - strlen(desc.text)) * 0.5f * 8.0f;

        //draw the text.
        cin->DrawString(x, y, buffer);
    }
    else {
        //get the box for the button.
        int left, top, right, bottom;
        FieldBox(desc, left, top, right, bottom);

        //draw the box
        HUDDrawBox(left, top, right, bottom, CVector(1.0, 0.0, 0.0));

        //put the button text in buffer.
        buffer.Set("%s", desc.text);

        //move x over so that the text is centered.
        x += (desc.width - strlen(desc.text)) * 0.5f * 8.0f;
    
        //draw the text.
        cin->DrawString(x, y, buffer);
    }
}

void FieldEdit(field_description &desc, const char *string_val) {
    if (desc.type != FT_STRING) return;

    edit_field = &desc;

    edit_text.Set("%s", string_val);
}

void FieldEdit(field_description &desc, float float_val) {
    if (desc.type != FT_FLOAT) return;

    edit_field = &desc;

    edit_text.Set("%.2f", float_val);
}

void FieldEdit(field_description &desc, int integer_val) {
    if (desc.type != FT_INT) return;

    edit_field = &desc;

    edit_text.Set("%d", integer_val);
}

void FieldEdit(field_description &desc, list_field_description &list) {
    if (desc.type != FT_LIST) return;

    edit_field = &desc;

    edit_list = &list;
    list_choice_selected = edit_list->selected;
}

void FieldEdit(field_description &desc) {
    if (desc.type != FT_BUTTON) return;

    edit_field = &desc;

    //we dont really "edit" this field.
    edit_text = "";
}

void FieldEdit(field_description &desc, bool checked) {
    if (desc.type != FT_RADIO) return;

    edit_field = &desc;

    //set the text to represent the state of the radio button.
    edit_text = (checked == true) ? "1" : "0";
}

