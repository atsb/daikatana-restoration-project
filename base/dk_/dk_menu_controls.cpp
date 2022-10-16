#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_model.h"
#include "dk_misc.h"
#include "client.h"

// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif

#include <ctype.h>

#include "l__language.h"


extern void *box_model;

CInterfaceControl::CInterfaceControl() {

}

CInterfaceControl::~CInterfaceControl() {

}

void CInterfaceControl::Reset() {
    
}

void CInterfaceControl::Animate(int32 elapsed_time) {

}

//lets the control draw any 2d artwork that will be behind the 3d models.
void CInterfaceControl::DrawBackground() {

}

//places the 3d models.
void CInterfaceControl::PlaceEntities() {

}

//lets the control draw any 2d artwork that will be in front of the 3d models.
void CInterfaceControl::DrawForeground() {

}

bool CInterfaceControl::MousePos(int32 norm_x, int32 norm_y) {
    return false;
}

//tells the control that the mouse is being pressed or released.
bool CInterfaceControl::MouseUp(int32 norm_x, int32 norm_y) {
    return false;
}

bool CInterfaceControl::MouseDown(int32 norm_x, int32 norm_y) {
    return false;
}

bool CInterfaceControl::Keydown(int32 key) {
    return false;
}

void CInterfaceControl::SetPosition(int32 norm_x, int32 norm_y) {

}


///////////////////////////////////////////////////////////////////////////
///  CInterfaceButton
///
///  Basic button that is pressed.
///////////////////////////////////////////////////////////////////////////

CInterfaceButton::CInterfaceButton() {
    font_buttons = NULL;
    font_normal = NULL;
    font_bright = NULL;

    num_pieces = 0;

    upchars = "";
    downchars = "";

    text_x = text_y = 0;
    norm_x = norm_y = 0;
    w = h = 1;

    have_capture = false;
    under_cursor = false;
    is_down = false;

    enabled = true;
}

CInterfaceButton::~CInterfaceButton() {

}

void CInterfaceButton::Reset() {
    have_capture = false;
    under_cursor = false;

    is_down = false;

    num_pieces = 0;

    enabled = true;
}

void CInterfaceButton::Enable(bool enabled) {
    this->enabled = enabled;
}

//positions the button on the screen and resets it's state.
void CInterfaceButton::Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, void *font_buttons) {
    Reset();

    this->norm_x = norm_x;
    this->norm_y = norm_y;

    this->font_buttons = font_buttons;
    this->font_normal = font_normal;
    this->font_bright = font_bright;
}

void CInterfaceButton::InitGraphics(const char *upchars, const char *downchars, const char *disablechars) {
    if (upchars == NULL) return;
    if (downchars == NULL) return;
    if (disablechars == NULL) return;

    //get the string length of the upchars string, which represents the number of 
    //pieces the button graphic is split into.
    int32 num_button_pieces = strlen(upchars);

    if (num_button_pieces != strlen(downchars) || num_button_pieces != strlen(disablechars)) return;

    this->upchars = upchars;
    this->downchars = downchars;
    this->disablechars = disablechars;

    this->num_pieces = num_button_pieces;

    //compute width and height of the button graphic.
    if (font_buttons != NULL) {
        h = num_pieces * re.FontHeight(font_buttons);
        w = re.StringWidth(upchars, font_buttons, 1);
    }
    else {
        w = 10;
        h = 10;
    }
}

void CInterfaceButton::SetText(const char *text, bool center) {
    this->text = text;

    if (font_normal == NULL) return;

    text_y = norm_y + (h - re.FontHeight(font_normal)) / 2;

    //compute the position to draw the text.
    if (center == TRUE) {
        text_x = norm_x + (w - re.StringWidth(text, font_normal, -1)) / 2;
    }
    else {
        text_x = norm_x + h / 2;
    }
}

bool CInterfaceButton::IsDown() {
    return is_down;
}

void CInterfaceButton::Up() {
    is_down = false;
}

void CInterfaceButton::Down() {
    is_down = true;
}

bool CInterfaceButton::HitTest(int32 norm_x, int32 norm_y) {
    return !(norm_x < this->norm_x || norm_x > this->norm_x + w || norm_y < this->norm_y || norm_y > this->norm_y + h);
}

bool CInterfaceButton::MousePos(int32 norm_x, int32 norm_y) {
    bool ret = false;

    if (HitTest(norm_x, norm_y) == false) {
        //we are off the button.
        under_cursor = false;
        ret = false;

        //if we have capture, we are down and should be going up now.
        if (have_capture == true) {
            Up();
        }
    }
    //we are on the button.
    else if (enabled == true) {
        //if the mouse cursor wasn't already on the button, make a sound.
        if (under_cursor == false) {
            S_StartLocalSound(DKM_sounds[DKMS_BUTTON3]);
        }

        //we are on the button.
        under_cursor = true;
        ret = true;

        //if we have capture, make sure the button is down.
        if (have_capture == true) {
            Down();
        }
    }

    //if we have the capture, we return true no matter what.
    if (have_capture == true) {
        ret = true;
    }

    return ret;
}

bool CInterfaceButton::MouseUp(int32 norm_x, int32 norm_y) {
    //if we dont have the capture, do nothing.
    if (have_capture == false) return false;

    //we go to our up state no matter if the cursor was on us or not.
    Up();

    //we lose capture.
    have_capture = false;

    //we have capture, check if the mouse is still over us.
    if (HitTest(norm_x, norm_y) == false) {
        //not over us.
        return false;
    }

    //the mouse was over us.  we were pressed.
    return true;
}

bool CInterfaceButton::MouseDown(int32 norm_x, int32 norm_y) {
    //we must be enabled to handle the mouse down event.
    if (enabled == false) return false;

    if (HitTest(norm_x, norm_y) == false) {
        //we are off the button.
        return false;
    }

    //we are on the button.
    S_StartLocalSound(DKM_sounds[DKMS_BUTTON6]);

    //capture the mouse.
    have_capture = true;
    
    //know this: that we are now in the down state.
    Down();

    return true;
}

void CInterfaceButton::DrawForeground() {
    //check if we should use the normal or bright font.
    if (enabled == true && (IsDown() == true || under_cursor == true)) {
        //use the bright font.
        if (font_bright == NULL) return;
        DKM_DrawString(text_x, text_y, text, font_bright, false, false);
    }
    else {
        //use the normal font.
        if (font_normal == NULL) return;
        DKM_DrawString(text_x, text_y, text, font_normal, false, false);
    }
}

void CInterfaceButton::DrawBackground() {
    if (font_buttons == NULL) return;

    //get the height of each piece of the button graphic.
    int32 piece_height = re.FontHeight(font_buttons);

    //draw the button by drawing our string.
    if (enabled == false) {
        //draw the button in it's diabled state.
        for (int32 i = 0; i < num_pieces; i++) {
            //pull the character out of the string.
            char string[2];
            string[0] = disablechars[i];
            string[1] = '\0';

            DKM_DrawString(norm_x, norm_y + i * piece_height, string, font_buttons, false, false);
        }
    }
    else if (IsDown() == true) {
        //draw the button pieces in the down state.
        for (int32 i = 0; i < num_pieces; i++) {
            //pull the character out of the string.
            char string[2];
            string[0] = downchars[i];
            string[1] = '\0';

            DKM_DrawString(norm_x, norm_y + i * piece_height, string, font_buttons, false, false);
        }
    }
    else {
        //draw the button pieces in the up state.
        for (int32 i = 0; i < num_pieces; i++) {
            //pull the character out of the string.
            char string[2];
            string[0] = upchars[i];
            string[1] = '\0';

            DKM_DrawString(norm_x, norm_y + i * piece_height, string, font_buttons, false, false);
        }
    }
}


///////////////////////////////////////////////////////////////////////////
///  CInterfaceButtonRadio
///
///  Used to let the user manipulate a boolean setting.
///////////////////////////////////////////////////////////////////////////

CInterfaceButtonRadio::CInterfaceButtonRadio() {
    checked = false;
}

#define RADIO_CIRCLE_WIDTH (20)

//positions the button on the screen and resets it's state.
void CInterfaceButtonRadio::Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, const char *text) {
    CInterfaceButton::Init(norm_x, norm_y, font_normal, font_bright, NULL);
    CInterfaceButton::InitGraphics("", "", "");
    CInterfaceButton::SetText(text, false);

    //compute the width of the button based on the width of the text.
    if (font_normal != NULL) {
        h = re.FontHeight(font_normal);
    
        //add to the width of the string the width of the radio circle itself and some space 
        //to the width of the button.
        w = re.StringWidth(text, font_normal, -1) + RADIO_CIRCLE_WIDTH;
    }
}

void CInterfaceButtonRadio::Check(bool on) {
    checked = on;
}



void CInterfaceButtonRadio::DrawForeground()
{
    //draw the circle.
    if (checked == true)
		  DKM_DrawString(norm_x, norm_y, SYMBOL_RADIOBUTTON, font_bright, false, false);
    else
		  DKM_DrawString(norm_x, norm_y, SYMBOL_RADIOBUTTON, font_normal, false, false);

    //draw the text.  check if the mouse is over the text.
    if (under_cursor == true)
      DKM_DrawString(norm_x + RADIO_CIRCLE_WIDTH, norm_y, text, font_bright, false, false);
    else
      DKM_DrawString(norm_x + RADIO_CIRCLE_WIDTH, norm_y, text, font_normal, false, false);
}



void CInterfaceButtonRadio::DrawBackground()
{
    //this function exists to override the functionality of the base class.
}

void CInterfaceButtonRadio::SetPosition(int32 norm_x, int32 norm_y) {
    //save the new norm_x and norm_y.
    this->norm_x = norm_x;
    this->norm_y = norm_y;
}


///////////////////////////////////////////////////////////////////////////
///  CInterfaceInputField
///
///  Used to let the user type a string.
///////////////////////////////////////////////////////////////////////////

#define INPUT_FIELD_SPACE_LEFT (6)

//this var helps keep track of the cursor flashing.
static int32 blink_start_time;

void ResetBlinkCounter() {
    blink_start_time = GetTickCount();
}

bool BlinkOn() {
    return ((GetTickCount() - blink_start_time) % 500) < 250;
}


//we keep track of a single field that is being edited.
static CInterfaceInputField *current_edit_field = NULL;

CInterfaceInputField *CurrentlyEditingField() {
    return current_edit_field;
}

CInterfaceInputField::CInterfaceInputField() {
    font_buttons = NULL;
    font_normal = NULL;
    font_bright = NULL;

    have_capture = false;
    under_cursor = false;
    editing = false;

    backgroundstring[0] = '\0';
}

CInterfaceInputField::~CInterfaceInputField() {

}

//positions the button on the screen and resets it's state.
void CInterfaceInputField::Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, void *font_buttons, char buttonchar) {
    this->norm_x = norm_x;
    this->norm_y = norm_y;

    this->font_normal = font_normal;
    this->font_bright = font_bright;
    this->font_buttons = font_buttons;

    //set the background string.
    backgroundstring[0] = buttonchar;
    backgroundstring[1] = '\0';

    //compute the width and height of the field.
    if (font_buttons == NULL) {
        w = 10;
        h = 10;
    }
    else
    {
        w = re.StringWidth(backgroundstring, font_buttons, -1);
        h = re.FontHeight(font_buttons);
    }

    //reset our state flags.
    have_capture = false;
    under_cursor = false;
    editing = false;
}

bool CInterfaceInputField::MousePos(int32 norm_x, int32 norm_y) {
    bool ret = false;

    //check if the mouse is over the field.
    if (norm_x < this->norm_x || norm_x > this->norm_x + w || norm_y < this->norm_y || norm_y > this->norm_y + h) {
        //we are off the field.
        under_cursor = false;
        ret = false;
    }
    else {
        //we are on the field.
        under_cursor = true;
        ret = true;
    }

    //if we have the mouse capture, return true.
    if (have_capture == true) {
        ret = true;
    }

    return ret;
}

bool CInterfaceInputField::MouseDown(int32 norm_x, int32 norm_y) {
    //check if the mouse is over the field.
    if (norm_x < this->norm_x || norm_x > this->norm_x + w || norm_y < this->norm_y || norm_y > this->norm_y + h) {
        //we are off the field.
        return false;
    }

    //capture the mouse.
    have_capture = true;

    return true;
}
//a return value of true means the user clicked in the field and wants to edit the text.
bool CInterfaceInputField::MouseUp(int32 norm_x, int32 norm_y) {
    //check if we have the mouse capture.
    if (have_capture == false) return false;

    //release the mouse capture.
    have_capture = false;

    //check if the mouse is in the field.
    if (norm_x < this->norm_x || norm_x > this->norm_x + w || norm_y < this->norm_y || norm_y > this->norm_y + h) {
        //we are off the field.
        return false;
    }

    //we are in the field
    //check if we are already editing or not.
    if (editing == true) {
        //move the insertion bar.
        insertion_bar_position = num_chars_off_left + re.FontNumCharsInWidth(text.Right(num_chars_off_left), 
            font_normal, norm_x - this->norm_x - INPUT_FIELD_SPACE_LEFT, 0);
    }
    else {
        //start editing the text.
        StartEditing(norm_x - this->norm_x - INPUT_FIELD_SPACE_LEFT);
    }

    return true;
}

void CInterfaceInputField::DrawForeground() {
    if (font_normal == NULL || font_bright == NULL) return;

    //check if we are editing the text 
    if (editing == false) {
        //not editing, just display as much of the text as will fit.

        //compute the portion of the text that we will display.
        buffer256 print;  //we dont use operator= here because it will call the constructor and interpret text as the format string.
        print = text;

        //check if the text is too long for the input box.
        int32 text_width = re.StringWidth(print, font_normal, -1);

        if (text_width + 2 * INPUT_FIELD_SPACE_LEFT > w) {
            //get the width of the "..." we will put at the end.
            int32 dots_width = re.StringWidth("...", font_normal, -1);

            //the text is too wide, find out how many characters will fit in the field.
            int32 num_printable = re.FontNumCharsInWidth(print, font_normal, w - 2 * INPUT_FIELD_SPACE_LEFT - dots_width, 0);

            //put the "..." at that position in the string.
            print.NewEnding(num_printable, "...");
        }

        //draw the text.
        if (under_cursor == true) {
            DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT, norm_y + 2, print, font_bright, false, false);
        }
        else {
            DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT, norm_y + 2, print, font_normal, false, false);
        }
    }
    else {
        //we are editing the text.  Copy the text, removing the portion that is off the left of the field.
        buffer256 print; //we dont use operator= here because it will call the constructor and interpret text as the format string.
        print = text.Right(num_chars_off_left);

        //Determine how much of the text is visible.
        int32 num_visible_chars = re.FontNumCharsInWidth(print, font_normal, w - 2 * INPUT_FIELD_SPACE_LEFT, 0);

        //truncate the string we will print.
        print.NewEnding(num_visible_chars, "");

        //print the string.
        DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT, norm_y + 2, print, font_normal, false, false);

        //
        //draw the insertion bar.
        //
        
        //truncate the string at the insertion bar position.
        print.NewEnding(insertion_bar_position - num_chars_off_left, "");

        //get the width of that string.
        int32 bar_offset = re.StringWidth(print, font_normal, -1);

#ifdef JPN
		// Draw IME compositon string
		DisplayCompString();
		// get the width of the composition string
		int32 len_imetext = re.StringWidth(imetext,font_normal, -1);
		// show the caret after the composition
		bar_offset += len_imetext;
#endif // JPN

        //the string we use to draw the insertion bar.
        static char bar_string[2] = { INSERTION_BAR_CHAR, '\0' };
				//atic char bar_string[2] = { '\x05f', '\0' };

#ifdef JPN
        //make it flash on and off.
        if (BlinkOn() == true) {
            DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT + bar_offset - 3, norm_y + 5 - 2, bar_string, font_bright, false, false);
        }
#else // JPN
        //make it flash on and off. cursor blink  blinking cursor
        if (BlinkOn() == true)
          DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT + bar_offset - 2, norm_y + 2, bar_string, font_bright, false, false);
#endif // JPN
    }
}
void CInterfaceInputField::DrawBackground() {
    if (font_buttons == NULL) return;

    //draw the character that contains the graphics for the 
    DKM_DrawString(norm_x, norm_y, backgroundstring, font_buttons, false, false);
}

void CInterfaceInputField::StartEditing(int32 starting_pixel) {
    //determine which character to put the cursor after.
    insertion_bar_position = re.FontNumCharsInWidth(text, font_normal, starting_pixel, 0);

    //at first, there are no characters off of the field's left.
    num_chars_off_left = 0;

    //we are editing.
    editing = true;

    //set our global pointer the current edit field.
    current_edit_field = this;

    //reset the blink counter.
    ResetBlinkCounter();
}

bool CInterfaceInputField::IsEditing() {
    return editing;
}

void CInterfaceInputField::StopEditing() {
    editing = false;

    //reset our global pointer to the current edit field.
    current_edit_field = NULL;
#ifdef JPN
	//disable IME
	SteImeOpenClose(false);
#endif // JPN
}

const char *CInterfaceInputField::GetText() {
    return text;
}

void CInterfaceInputField::SetText(const char *text) {
    //set our string.
    this->text = text;

    //make sure we are not editing
    editing = false;
}

void CInterfaceInputField::CheckCursorPos() {
    if (editing == false) return;

    //check if the cursor is off the left of the field.
    if (insertion_bar_position < num_chars_off_left) {
        //adjust the number of characters we put off the left.
        num_chars_off_left = insertion_bar_position - 3;

// Encompass MarkMa 050399
#ifdef	JPN	// JPN
		if (IsDBCSLeadByte(text[num_chars_off_left-1]))
			num_chars_off_left--;
#endif	// JPN
// Encompass MarkMa 050399

        bound_min(num_chars_off_left, 0);
        return;
    }

    //find out how many pixels wide the string is up to the cursor position.
    int32 width_to_cursor = re.StringWidth(text, font_normal, insertion_bar_position);

    //check how wide the part of the string is that is off the left.
    int32 width_off_left = re.StringWidth(text, font_normal, num_chars_off_left);

    //check the offset of the cursor in the visible field.
    int32 cursor_offset = width_to_cursor - width_off_left;

    //check if the cursor will be in the field.
    if (cursor_offset > w - 2 * INPUT_FIELD_SPACE_LEFT) {
        //it is off.  determine how many characters we need to push off the left before
        //the cursor will be in the visible portion of the field.
        num_chars_off_left = re.FontNumCharsInWidth(text, font_normal, width_to_cursor - w * 0.75f, 0);
    }
}

#define valid_key(k) valid_keys[k] = true;

bool ValidStringChar(int32 key) {
    if (key < 0 || key > 255) return false;

    //a bool for each possible key.
    static bool key_valid[256];

    static bool inited = false;
    if (inited == false) {
        static char *valid_keys = "abcedfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`1234567890-=[];\',."
                                  "~!@#$^&*()_+{}|:\"<>? ";

        //setup the key_valid array.
        for (int32 i = 0; ; i++) {
            if (valid_keys[i] == '\0') break;

            key_valid[valid_keys[i]] = true;
        }

        inited = true;
    }

    return key_valid[key];
}

int CheckShiftKey(int32 key) {
    //check if this is a lowercase letter.
    if (isalpha(key) && islower(key)) {
        if (Key_IsDown(K_SHIFT) != 0) {
            //make it upper case.
            key = toupper(key);
        }
    }
    
    //return the key unchanged.
    return key;
}

#ifdef JPN
bool CInterfaceInputField::ImeKeydown(int32 leadbyte, int32 trailbyte) {
    if (editing == false) return false;
	
	if (leadbyte)
	{	
		//insert the DBCS lead byte into our string.
		text.InsertChar(leadbyte, insertion_bar_position);
		insertion_bar_position++;
	}
    //insert the DBCS trail byte into our string.
    text.InsertChar(trailbyte, insertion_bar_position);
    insertion_bar_position++;

    //make sure the cursor is still in the screen.
    CheckCursorPos();

    //reset the blink counter.
    ResetBlinkCounter();
    return true;
}
#endif // JPN

bool CInterfaceInputField::Keydown(int32 key) {
    if (editing == false) return false;

    //check for any character that can be a character in the string.
    if (ValidStringChar(key) == true) {
        //check if shift is down and we need to convert a letter to uppercase.
        key = CheckShiftKey(key);

        //insert the character into our string.
        text.InsertChar(key, insertion_bar_position);

        //move the insertion bar over.
        insertion_bar_position++;
        bound_max(insertion_bar_position, strlen(text));

        //make sure the cursor is still in the screen.
        CheckCursorPos();

        //reset the blink counter.
        ResetBlinkCounter();
        return true;
    }

    //check the key that was pressed.
    switch (key) {
        case K_LEFTARROW:
#ifdef JPN
			if 	( insertion_bar_position >0 )
			{
				int j,prv;
				prv=j=0;
				while ( j < insertion_bar_position)
				{
					prv = j;
					if (IsDBCSLeadByte(text[j]))
						j++;
					j++;
				}
				if (IsDBCSLeadByte(text[prv]))
				{
					insertion_bar_position--;
				}
			}
#endif // JPN
            //move the cursor position over.
            insertion_bar_position--;
            bound_min(insertion_bar_position, 0);

            //reset the blink counter.
            ResetBlinkCounter();

            //make sure the part of the string with the cursor is visible.
            CheckCursorPos();
            return true;
        case K_RIGHTARROW:
#ifdef JPN
			if ( IsDBCSLeadByte(text[insertion_bar_position]) )
			{
	            insertion_bar_position++;
			}
#endif // JPN
            //move the cursor over.
            insertion_bar_position++;
            bound_max(insertion_bar_position, strlen(text));

            //reset the blink counter.
            ResetBlinkCounter();

            //make sure the part of the string with the cursor is visible.
            CheckCursorPos();
            return true;
        case K_BACKSPACE:
#ifdef JPN
			if 	( insertion_bar_position >0 )
			{
				int j,prv;
				prv=j=0;
				while ( j < insertion_bar_position)
				{
					prv = j;
					if (IsDBCSLeadByte(text[j]))
						j++;
					j++;
				}
				if (IsDBCSLeadByte(text[prv]))
				{
					insertion_bar_position--;
					//delete the lead-byte
					text.RemoveChar(insertion_bar_position);
				}
				// delete the trail-byte
			}
#endif // JPN
            //delete the character behind the cursor.
            text.RemoveChar(insertion_bar_position - 1);

            //move the cursor back a space.
            insertion_bar_position--;
            bound_min(insertion_bar_position, 0);

            //check our cursor position.
            CheckCursorPos();

            ResetBlinkCounter();
            return true;
        case K_DEL:
#ifdef JPN
			if ( IsDBCSLeadByte(text[insertion_bar_position]) )
			{	
				// delete the lead-byte
				text.RemoveChar(insertion_bar_position);
			}
			// delete the trail-byte
#endif // JPN
            //delete the character in front of the cursor.
            text.RemoveChar(insertion_bar_position);

            CheckCursorPos();
            ResetBlinkCounter();
            return true;
        case K_HOME:
            //move the insertion bar to the beginning.
            insertion_bar_position = 0;
            CheckCursorPos();
            ResetBlinkCounter();
            return true;
        case K_END:
            //move the insertion bar to the end.
            insertion_bar_position = strlen(text);
            CheckCursorPos();
            ResetBlinkCounter();
            return true;
        case K_ENTER:
            S_StartLocalSound(DKM_sounds[DKMS_BUTTON7]);
            editing = false;
#ifdef JPN
			// disable IME
			SteImeOpenClose(false);
			current_edit_field = NULL;
#endif // JPN
            return true;
    }

    return false;
}

#ifdef JPN //4-6-99
void DisableIme(bool fFlag)
{
	HWND hWnd;
	HIMC            hIMC;

	hWnd = FindWindow("Daikatana",NULL);
	//
	// If fFlag is true then open IME; otherwise close it.
	//

	if ( !( hIMC = ImmGetContext( hWnd ) ) )
	return;

	ImmSetOpenStatus( hIMC, fFlag );

	ImmReleaseContext( hWnd, hIMC );
}

void CInterfaceInputField::SteImeOpenClose(bool fFlag )
{
	DisableIme(fFlag);
	// Clear IME composition windows
	imetext = NULL;

}

void CInterfaceInputField::SetImeText(LPSTR lpStr)
{
	imetext = lpStr;
}

void CInterfaceInputField::DisplayCompString()
{
	if (imetext==NULL)
		return;
	//HideCaret( hwnd );

	//SetCaretPos( ( xPos + StrLen ) * cxMetrics, yPos * cyMetrics );
	
	// Get the Caret position
	//CompForm.ptCurrentPos.x = (int) norm_x + INPUT_FIELD_SPACE_LEFT + bar_offset - 2;
	//CompForm.ptCurrentPos.y = (int) norm_y + 5 - 2;
	
	// Show compostion
        //we are editing the text.  Copy the text, removing the portion that is off the left of the field.
        buffer256 print; //we dont use operator= here because it will call the constructor and interpret text as the format string.
        print = text.Right(num_chars_off_left);

    //Determine how much of the text is visible.
    int32 num_visible_chars = re.FontNumCharsInWidth(print, font_normal, w - 2 * INPUT_FIELD_SPACE_LEFT, 0);

    //truncate the string we will print.
    print.NewEnding(num_visible_chars, "");

    //print the string.
    //DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT, norm_y + 5, print, font_normal, false, false);

    //
    //draw the insertion bar.
    //
    
    //truncate the string at the insertion bar position.
    print.NewEnding(insertion_bar_position - num_chars_off_left, "");

    //get the width of that string.
    int32 bar_offset = re.StringWidth(print, font_normal, -1);

	DKM_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT + bar_offset , norm_y + 5 - 3 , imetext, font_bright, false, false);

	//ShowCaret( hwnd );
}
#endif // JPN

void CInterfaceInputField::SetPosition(int32 norm_x, int32 norm_y) {
    //save the new position.
    this->norm_x = norm_x;
    this->norm_y = norm_y;
}


///////////////////////////////////////////////////////////////////////////
///  CInterfaceBox
///
///  Used to draw a border around area of screen.
///////////////////////////////////////////////////////////////////////////

void CInterfaceBox::Init(int32 l, int32 t, int32 r, int32 b, int32 thick) {
    left = l;
    right = r;
    top = t;
    bottom = b;
    thickness = thick;
}

void CInterfaceBox::PlaceEntities() {
    //compute half our thickness.
    float half = thickness * 0.5f;

    entity_t *corner;
    entity_t *line;

    //make the corner entities.
    //top-left.
    corner = NewEntity();
    corner->flags = RF_FULLBRIGHT;
    corner->model = box_model;
    corner->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    corner->origin.x = 879.0f;
    corner->origin.y = 320 - left - half;
    corner->origin.z = 240 - top - half;
    corner->angles.Zero();
    corner->render_scale.x = 1.0f;
    corner->render_scale.y = thickness;
    corner->render_scale.z = thickness;
    corner->frame = 0;
  
    //top-right
    corner = NewEntity();
    corner->flags = RF_FULLBRIGHT;
    corner->model = box_model;
    corner->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    corner->origin.x = 879.0f;
    corner->origin.y = 320 - right + half;
    corner->origin.z = 240 - top - half;
    corner->angles.Set( 0, 0, 270 );
    corner->render_scale.x = 1.0f;
    corner->render_scale.y = thickness;
    corner->render_scale.z = thickness;
    corner->frame = 0;

    //bottom-right
    corner = NewEntity();
    corner->flags = RF_FULLBRIGHT;
    corner->model = box_model;
    corner->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    corner->origin.x = 879.0f;
    corner->origin.y = 320 - right + half;
    corner->origin.z = 240 - bottom + half;
    corner->angles.Set( 0, 0, 180 );
    corner->render_scale.x = 1.0f;
    corner->render_scale.y = thickness;
    corner->render_scale.z = thickness;
    corner->frame = 0;
  
    //bottom-left
    corner = NewEntity();
    corner->flags = RF_FULLBRIGHT;
    corner->model = box_model;
    corner->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    corner->origin.x = 879.0f;
    corner->origin.y = 320 - left - half;
    corner->origin.z = 240 - bottom + half;
    corner->angles.Set( 0, 0, 90 );
    corner->render_scale.x = 1.0f;
    corner->render_scale.y = thickness;
    corner->render_scale.z = thickness;
    corner->frame = 0;

    //draw the horizontal and vertical lines.
    //left
    
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - left - half;
    line->origin.z = 240 - top - (bottom - top) * 0.5f;
    line->angles.Set( 0, 0, 90 );
    line->render_scale.x = 1.0f;
    line->render_scale.y = (bottom - top) - 2.0f * thickness;
    line->render_scale.z = thickness;
    

    //top
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - left - (right - left) * 0.5f;
    line->origin.z = 240 - top - half;
    line->angles.Zero();
    line->render_scale.x = 1.0f;
    line->render_scale.y = (right - left) - 2.0f * thickness;
    line->render_scale.z = thickness;

    //right
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - right + half;
    line->origin.z = 240 - top - (bottom - top) * 0.5f;
    line->angles.Set( 0, 0, 270 );
    line->render_scale.x = 1.0f;
    line->render_scale.y = (bottom - top) - 2.0f * thickness;
    line->render_scale.z = thickness;

    //bottom
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin("skins/iu_buttonmini.wal", RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - left - (right - left) * 0.5f;
    line->origin.z = 240 - bottom + half;
    line->angles.Set( 0, 0, 180 );
    line->render_scale.x = 1.0f;
    line->render_scale.y = (right - left) - 2.0f * thickness;
    line->render_scale.z = thickness;
}


///////////////////////////////////////////////////////////////////////////
///  CInterfaceLine
///
///  Used to draw a horizontal line
///////////////////////////////////////////////////////////////////////////

void CInterfaceLine::Init(int32 l, int32 t, int32 r, int32 thick, const char *skin_name)
{
    left = l;
    right = r;
    top = t;
    bottom = 0;
    thickness = thick;

    this->skin_name = skin_name;
}


void CInterfaceLine::InitV(int32 x, int32 y, int32 y2, int32 thick, const char *skin_name)
{
    left = x;
    right = x + thick;
    top = y;
    bottom = y2;
    thickness = thick;

    this->skin_name = skin_name;
}

void CInterfaceLine::PlaceEntities()    // horizontal line
{
    //compute half our thickness.
    float half = thickness * 0.5f;

    entity_t *line;

    //top
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin(skin_name, RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - left - (right - left) * 0.5f;
    line->origin.z = 240 - top - half;
    line->angles.Zero();
    line->render_scale.x = 1.0f;
    line->render_scale.y = right - left;
    line->render_scale.z = thickness;
}

void CInterfaceLine::PlaceEntitiesV()   // vertical line
{
    //compute half our thickness.
    float half = thickness * 0.5f;

    entity_t *line;

    //top
    line = NewEntity();
    line->flags = RF_FULLBRIGHT;
    line->model = box_model;
    line->skin = re.RegisterSkin(skin_name, RESOURCE_INTERFACE);
    line->origin.x = 879.0f;
    line->origin.y = 320 - left - half;
    line->origin.z = 240 - top - (bottom - top) * 0.5f;
    line->angles.Set( 0, 0, 90 );
    line->render_scale.x = 1.0f;
    line->render_scale.y = (bottom - top) - 2.0f * thickness;
    line->render_scale.z = thickness;
}



///////////////////////////////////////////////////////////////////////////
///
///  CInterfaceSlider
///
///////////////////////////////////////////////////////////////////////////

CInterfaceSlider::CInterfaceSlider() {
    position = 0;
    log_scale = false;
    low = 0;
    high = 1;
    steps = 10;
    have_capture = false;

    button_font = NULL;
    backgroundstring[0] = tabstring[0] = '\0';

    norm_x = 0;
    norm_y = 0;
    w = 10;
    h = 10;
}

void CInterfaceSlider::Init(int32 norm_x, int32 norm_y, void *button_font, char backgroundchar, char tabchar) {
    this->button_font = button_font;

    //put the characters into strings.
    backgroundstring[0] = backgroundchar;
    backgroundstring[1] = '\0';

    tabstring[0] = tabchar;
    tabstring[1] = '\0';

    //set our position info.
    this->norm_x = norm_x;
    this->norm_y = norm_y;

    //get the width and height from the font.
    if (button_font != NULL) {
        w = re.StringWidth(backgroundstring, button_font, -1);
        h = re.FontHeight(button_font);

        tabwidth = re.StringWidth(tabstring, button_font, -1);

        left = norm_x + tabwidth / 2;
        right = norm_x + w - tabwidth / 2;
    }

    have_capture = false;
}

void CInterfaceSlider::InitLinear(int32 steps, float low, float high) {
    //reset our state.
    position = 0;

    //we are not log scale.
    log_scale = false;

    //set our function vars.
    this->steps = steps;
    this->low = low;
    this->high = high;
}

void CInterfaceSlider::InitExp(int32 steps, float low, float high) {
    //reset our state.
    position = 0;

    //we are log scale.
    log_scale = true;

    //set our function vars.
    this->steps = steps;
    this->low = low;
    this->high = high;
}


bool CInterfaceSlider::MousePos(int32 norm_x, int32 norm_y) {
    //check if we have the mouse capture.
    if (have_capture == true) {
        //get the tick that corresponds to this norm_x value.
        int32 x_diff = norm_x - this->norm_x;
        float percent = x_diff / float(w);
        int32 tick = percent * steps;

        //clamp it.
        bound_min(tick, 0);
        bound_max(tick, steps - 1);

        position = tick;

        return true;
    }

    return false;
}

bool CInterfaceSlider::MouseDown(int32 norm_x, int32 norm_y) {
    //check if the position is on us.
    if (norm_x < this->norm_x || norm_y < this->norm_y || norm_x > this->norm_x + w || norm_y > this->norm_y + h) {
        //the mouse is not on us.
        return false;
    }

    //the user clicked the mouse down on us.
    have_capture = true;

    //move the slider position to where they clicked.
    MousePos(norm_x, norm_y);

    return true;
}

//a return value of true means this button was pressed by the user.
bool CInterfaceSlider::MouseUp(int32 norm_x, int32 norm_y) {
    //check if we have the capture.
    if (have_capture == true) {
        //release capture.
        have_capture = false;
        return true;
    }

    return false;
}

float CInterfaceSlider::Position() {
    //interpolate between low and high.
    float pos = 0.001f + low + (high - low) * (position / float(steps - 1));

    if (log_scale == true) {
        return pow(10, pos);
    }
    else {
        return pos;
    }
}

void CInterfaceSlider::Position(float pos) {
    //check if we are log or not.
    if (log_scale == true) {
        //check the value.
        if (pos < 0.00001f) {
            pos = 0.00001f;
        }

        //take the log of the value.
        float mag = log10(pos);

        //get the slider position.
        position = int32((mag - low)/(high - low) * (steps - 1));
    }
    else {
        //get the slider position.
        position = int32((pos - low)/(high - low) * (steps - 1) + 0.001f);
    }

    //error check the new position.
    bound_min(position, 0);
    bound_max(position, steps - 1);
}

void CInterfaceSlider::DrawForeground() {
    if (button_font == NULL) return;

    //compute the position of the tab.
    int32 pos = norm_x + position * float(w - tabwidth) / (steps - 1);

    //draw the tab.
    DKM_DrawString(pos, norm_y, tabstring, button_font, false, false);
}

void CInterfaceSlider::DrawBackground() {
    if (button_font == NULL) return;

    //draw our background graphic.
    DKM_DrawString(norm_x, norm_y, backgroundstring, button_font, false, false);
}


///////////////////////////////////////////////////////////////////////////
///
///  CInterfaceSlider
///
///////////////////////////////////////////////////////////////////////////

CInterfaceScrollbar::CInterfaceScrollbar() {
    position = 0;
    log_scale = false;
    low = 0;
    high = 1;
    steps = 10;
    have_capture = false;

    button_font = NULL;
    backgroundstring[0] = tabstring[0] = '\0';

    norm_x = 0;
    norm_y = 0;
    w = 10;
    h = 10;
}

void CInterfaceScrollbar::Init(int32 norm_x, int32 norm_y, void *button_font, char backgroundchar, char tabchar) {
    this->button_font = button_font;

    //put the characters into strings.
    backgroundstring[0] = backgroundchar;
    backgroundstring[1] = '\0';

    tabstring[0] = tabchar;
    tabstring[1] = '\0';

    //set our position info.
    this->norm_x = norm_x;
    this->norm_y = norm_y;

    //get the width and height from the font.
    if (button_font != NULL) {
        w = re.StringWidth(backgroundstring, button_font, -1);
        h = re.FontHeight(button_font);

        tabwidth = re.StringWidth(tabstring, button_font, -1);

        top = norm_x + tabwidth / 2;
        bottom = norm_x + w - tabwidth / 2;
    }

    have_capture = false;
}

void CInterfaceScrollbar::InitLinear(int32 steps, float low, float high) {
    //reset our state.
    position = 0;

    //we are not log scale.
    log_scale = false;

    //set our function vars.
    this->steps = steps;
    this->low = low;
    this->high = high;
}

void CInterfaceScrollbar::InitExp(int32 steps, float low, float high) {
    //reset our state.
    position = 0;

    //we are log scale.
    log_scale = true;

    //set our function vars.
    this->steps = steps;
    this->low = low;
    this->high = high;
}


bool CInterfaceScrollbar::MousePos(int32 norm_x, int32 norm_y) {
    //check if we have the mouse capture.
    if (have_capture == true) {
        //get the tick that corresponds to this norm_x value.
        int32 x_diff = norm_x - this->norm_x;
        float percent = x_diff / float(w);
        int32 tick = percent * steps;

        //clamp it.
        bound_min(tick, 0);
        bound_max(tick, steps - 1);

        position = tick;

        return true;
    }

    return false;
}

bool CInterfaceScrollbar::MouseDown(int32 norm_x, int32 norm_y) {
    //check if the position is on us.
    if (norm_x < this->norm_x || norm_y < this->norm_y || norm_x > this->norm_x + w || norm_y > this->norm_y + h) {
        //the mouse is not on us.
        return false;
    }

    //the user clicked the mouse down on us.
    have_capture = true;

    //move the slider position to where they clicked.
    MousePos(norm_x, norm_y);

    return true;
}

//a return value of true means this button was pressed by the user.
bool CInterfaceScrollbar::MouseUp(int32 norm_x, int32 norm_y) {
    //check if we have the capture.
    if (have_capture == true) {
        //release capture.
        have_capture = false;
        return true;
    }

    return false;
}

float CInterfaceScrollbar::Position() {
    //interpolate between low and high.
    float pos = 0.001f + low + (high - low) * (position / float(steps - 1));

    if (log_scale == true) {
        return pow(10, pos);
    }
    else {
        return pos;
    }
}

void CInterfaceScrollbar::Position(float pos) {
    //check if we are log or not.
    if (log_scale == true) {
        //check the value.
        if (pos < 0.00001f) {
            pos = 0.00001f;
        }

        //take the log of the value.
        float mag = log10(pos);

        //get the slider position.
        position = int32((mag - low)/(high - low) * (steps - 1));
    }
    else {
        //get the slider position.
        position = int32((pos - low)/(high - low) * (steps - 1) + 0.001f);
    }

    //error check the new position.
    bound_min(position, 0);
    bound_max(position, steps - 1);
}

void CInterfaceScrollbar::DrawForeground() {
    if (button_font == NULL) return;

    //compute the position of the tab.
    int32 pos = norm_x + position * float(w - tabwidth) / (steps - 1);

    //draw the tab.
    DKM_DrawString(pos, norm_y, tabstring, button_font, false, false);
}

void CInterfaceScrollbar::DrawBackground() {
    if (button_font == NULL) return;

    //draw our background graphic.
    DKM_DrawString(norm_x, norm_y, backgroundstring, button_font, false, false);
}


///////////////////////////////////////////////////////////////////////////
///
///  CInterfacePicker
///    a control with a centered title, and a second line with a left and right
///    toenail button to select a string from a list.
///
///////////////////////////////////////////////////////////////////////////

#define PICKER_TITLE_HEIGHT (12)

CInterfacePicker::CInterfacePicker() {
    font_normal = NULL;
    font_bright = NULL;

    mouse_on_picker = false;

    current_string = 0;
}

//sets the position of the picker.
void CInterfacePicker::Init(int32 norm_x, int32 norm_y, int32 w, void *font_normal, void *font_bright, void *font_buttons, const char *title) {
    this->font_normal = font_normal;
    this->font_bright = font_bright;

    this->title = title;

    this->norm_x = norm_x;
    this->norm_y = norm_y;
    this->w = w;

    mouse_on_picker = false;

    if (font_buttons != NULL) {
        h = re.FontHeight(font_buttons);
        toenail_width = re.StringWidth(TOENAIL_LEFT_ON_STRING, font_buttons, -1) - 1;
    }
    else {
        h = 10;
        toenail_width = 10;
    }

    //compute the top of the toenails.
    int32 toenail_top = norm_y;
    if (this->title == true) {
        toenail_top += PICKER_TITLE_HEIGHT;
    }

    //initialize the background.
    background.Init(norm_x + toenail_width + 1, toenail_top, norm_x + w - toenail_width - 1, h, "skins/iu_plaque.bmp");

    //initialize the scroll buttons.
    left_button.Init(norm_x, toenail_top, font_normal, font_bright, font_buttons);
    left_button.InitGraphics(TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING);
    //left_button.SetText(TRIARROW_LEFT_STRING, true);
		left_button.SetText(ARROW_LEFT_STRING, true);

    right_button.Init(norm_x + w - toenail_width, toenail_top, font_normal, font_bright, font_buttons);
    right_button.InitGraphics(TOENAIL_RIGHT_OFF_STRING, TOENAIL_RIGHT_ON_STRING, TOENAIL_RIGHT_DISABLE_STRING);

    //right_button.SetText(TRIARROW_RIGHT_STRING, true);
		right_button.SetText(ARROW_RIGHT_STRING, true);

    EnableScrollButtons();
}

//adds a string into the picker.
void CInterfacePicker::AddString(const char *string) {
    if (string == NULL || string[0] == '\0') return;

    //make a new string object.
    buffer256 *new_string = new buffer256;
    *new_string = string;

    //add the string to the array.
    strings.Add(new_string);

    //enable/disable the scroll toenails as necessary.
    EnableScrollButtons();
}

void CInterfacePicker::CheckCurrentString() {
    bound_min(current_string, 0);
    bound_max(current_string, strings.Num() - 1);
}

void CInterfacePicker::ResetStrings() {
    strings.Reset();
}

void CInterfacePicker::DrawForeground() {
    //the top pixel of the toenail buttons.
    int32 button_top = norm_y;

    //check if we have a title
    if (title == true) {
        button_top += PICKER_TITLE_HEIGHT;
    }

    //draw the left button background.
    left_button.DrawBackground();

    //draw the right bottom background.
    right_button.DrawBackground();

    if (font_normal == NULL) return;
    if (font_bright == NULL) return;

    //check if we have a title
    if (title == true) {
        //get the width of the title string.
        int32 title_width = re.StringWidth(title, font_normal, -1);

        //draw the title string.
        DKM_DrawString(norm_x + (w - title_width) / 2, norm_y - 4, title, font_normal, false, false);
    }

    //draw the arrow foregrounds
    left_button.DrawForeground();
    right_button.DrawForeground();

    //get the currently selected string.
    const buffer256 *string = strings.Item(current_string);

    if (string == NULL) return;

    //get the width of the string.
    int32 string_width = re.StringWidth(*string, font_normal, -1);

    //draw the string. (+2 = adjust for font centering)
    if (mouse_on_picker == true) {
        DKM_DrawString(norm_x + (w - string_width) / 2, button_top + 2, *string, font_bright, false, false);
    }																															 
    else {
        DKM_DrawString(norm_x + (w - string_width) / 2, button_top + 2, *string, font_normal, false, false);
    }
}

void CInterfacePicker::PlaceEntities() {
    background.PlaceEntities();
}

bool CInterfacePicker::MousePos(int32 norm_x, int32 norm_y) {
    bool ret = false;

    //tell the scroll buttons where the mouse is.
    if (left_button.MousePos(norm_x, norm_y) == true) ret = true;
    if (right_button.MousePos(norm_x, norm_y) == true) ret = true;
    
    //compute the top and bottom of the highlightable area.
    int32 top = this->norm_y;
    if (title == true) {
        top += PICKER_TITLE_HEIGHT;
    }

    int32 bottom = top + h;

    //check if the mouse is in the picker at all.
    if (norm_x < this->norm_x || norm_y < top || norm_x > this->norm_x + w || norm_y > bottom) {
        //the mouse is off everything.
        mouse_on_picker = false;
        return ret;
    }

    //the mouse is on the picker
    mouse_on_picker = true;

    return true;
}

bool CInterfacePicker::MouseDown(int32 norm_x, int32 norm_y) {
    if (left_button.MouseDown(norm_x, norm_y) == true) return true;
    if (right_button.MouseDown(norm_x, norm_y) == true) return true;

    return false;
}

//a return value of true means this button was pressed by the user.
bool CInterfacePicker::MouseUp(int32 norm_x, int32 norm_y) {
    if (left_button.MouseUp(norm_x, norm_y) == true) {
        //they clicked the left toenail.
        current_string--;
        CheckCurrentString();

        //enable/disable the scroll toenails as necessary.
        EnableScrollButtons();

        return true;
    }
    if (right_button.MouseUp(norm_x, norm_y) == true) {
        //they clicked on the right toenail.
        current_string++;
        CheckCurrentString();

        //enable/disable the scroll toenails as necessary.
        EnableScrollButtons();

        return true;
    }

    return false;
}

const char *CInterfacePicker::CurrentString() {
    return *strings.Item(current_string);
}

int CInterfacePicker::CurrentIndex() {
    return current_string;
}

void CInterfacePicker::SetCurrentString(int32 num) {
    current_string = num;

    CheckCurrentString();

    //enable/disable the scroll toenails as necessary.
    EnableScrollButtons();
}

void CInterfacePicker::EnableScrollButtons() {
    //check if we should disable the left button
    if (current_string <= 0) {
        left_button.Enable(false);
    }
    else {
        left_button.Enable(true);
    }

    //check if we should disable the right button.
    if (current_string >= strings.Num() - 1) {
        right_button.Enable(false);
    }
    else {
        right_button.Enable(true);
    }
}


///////////////////////////////////////////////////////////////////////////
///
///  CInterfaceControlArray
///     implements a scrollable list of same-type controls.
///
///////////////////////////////////////////////////////////////////////////

CInterfaceControlArray::CInterfaceControlArray() {
    menu_font_normal = NULL;
    menu_font_bright = NULL;
    button_font = NULL;
    norm_x = 0;
    norm_y = 0;
    num_rows = 0;
    row_height = 0;
    top_index = 0;
    width = 0;
}

CInterfaceControlArray::~CInterfaceControlArray() {

}

//specifys the dimensions of the control
void CInterfaceControlArray::Init(int32 norm_x, int32 norm_y, int32 width, int32 num_rows, int32 row_height) {
    this->norm_x = norm_x;
    this->norm_y = norm_y;
    this->width = width;
    this->num_rows = num_rows;
    this->row_height = row_height;

    //compute the bottom of the border.
    int32 border_bottom = norm_y + num_rows * row_height + 10;

    //initilize the border.
    border.Init(norm_x, norm_y, norm_x + width, border_bottom, 2);

    //move the visible controls.
    PositionVisibleControls();
}

void CInterfaceControlArray::InitScrollButtons(void *menu_font_normal, void *menu_font_bright, void *button_font) {
    //save the pointers to the fonts.
    this->menu_font_normal = menu_font_normal;
    this->menu_font_bright = menu_font_bright;
    this->button_font = button_font;
    
    //compute the top of the scroll buttons
    int32 button_top = (norm_y + num_rows * row_height + 10) + 4;  //the border_bottom + 4

    //get the width of the scroll toenail.
    int32 toenail_width = re.StringWidth(TOENAIL_LEFT_OFF_STRING, button_font, -1);

    //compute the left of the up button.
    int32 up_left = norm_x + width / 2 - 10 - toenail_width;
    //compute the left of the down button.
    int32 down_left = norm_x + width / 2 + 10;

    //initialize the buttons.TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING
    scroll_up.Init(up_left, button_top, menu_font_normal, menu_font_bright, button_font);
    scroll_up.InitGraphics(TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING);
    //scroll_up.SetText(TRIARROW_UP_STRING, true);
		scroll_up.SetText(ARROW_UP_STRING, true);

    scroll_down.Init(down_left, button_top, menu_font_normal, menu_font_bright, button_font);
    scroll_down.InitGraphics(TOENAIL_DOWN_OFF_STRING, TOENAIL_DOWN_ON_STRING, TOENAIL_DOWN_DISABLE_STRING);
    //scroll_down.SetText(TRIARROW_DOWN_STRING, true);
		scroll_down.SetText(ARROW_DOWN_STRING, true);

    //decide if the up and down scroll buttons are visible.
    ShowScrollButtons();
}

//adds a control to the array.
void CInterfaceControlArray::AddControl(CInterfaceControl *control) {
    //add the control to our array.
    controls.Add(control);

    //move the visible controls.
    PositionVisibleControls();

    //decide if the up and down scroll buttons are visible.
    ShowScrollButtons();
}

//returns true if the given control is currently visible.
bool CInterfaceControlArray::ControlVisible(const CInterfaceControl *desired_control) {
    //look through our visible controls.
    for (int32 i = 0; i < num_rows; i++) {
        //get the control.
        const CInterfaceControl *control = controls.Item(i + top_index);

        if (control == NULL) continue;

        //check if this is the control we are looking for.
        if (desired_control == control) return true;
    }

    //the desired control is not visible.
    return false;
}

void CInterfaceControlArray::DrawBackground() {
    //draw our sub-controls.
    border.DrawBackground();

    if( top_index > 0 )
	{
	    scroll_up.DrawBackground();
	}

    if( top_index + num_rows < controls.Num() ) 
	{
	    scroll_down.DrawBackground();
	}
}

void CInterfaceControlArray::PlaceEntities() {
    //draw our sub-controls.
    border.PlaceEntities();

    scroll_up.PlaceEntities();
    scroll_down.PlaceEntities();
}

void CInterfaceControlArray::DrawForeground() {
    //draw our sub-controls.
    border.DrawForeground();

    if( top_index > 0 )
	{
	    scroll_up.DrawForeground();
	}

    if( top_index + num_rows < controls.Num() ) 
	{
	    scroll_down.DrawForeground();
	}
}

bool CInterfaceControlArray::MousePos(int32 norm_x, int32 norm_y) {
    bool ret = false;

    if( top_index > 0 )
	{
	    if (scroll_up.MousePos(norm_x, norm_y) == true) ret = true;
	}

    if( top_index + num_rows < controls.Num() ) 
	{
	    if (scroll_down.MousePos(norm_x, norm_y) == true) ret = true;
	}

    return ret;
}

bool CInterfaceControlArray::MouseUp(int32 norm_x, int32 norm_y) {
    if( top_index > 0 )
	{
		if (scroll_up.MouseUp(norm_x, norm_y) == true) {
			//they want to scroll up.
			top_index--;
			bound_min(top_index, 0);

			//move the visible controls.
			PositionVisibleControls();

			//decide if the up and down scroll buttons are visible.
			ShowScrollButtons();
			return true;
		}
	}

    if( top_index + num_rows < controls.Num() ) 
	{
		if (scroll_down.MouseUp(norm_x, norm_y) == true) {
			//scroll down.
			top_index++;
			bound_max(top_index, controls.Num() - num_rows);

			//move the visible controls.
			PositionVisibleControls();

			//decide if the up and down scroll buttons are visible.
			ShowScrollButtons();
			return true;
		}
	}

    return false;
}

bool CInterfaceControlArray::MouseDown(int32 norm_x, int32 norm_y) {

    if( top_index > 0 )
	{
		if (scroll_up.MouseDown(norm_x, norm_y) == true) return true;
	}

    if( top_index + num_rows < controls.Num() ) 
	{
		if (scroll_down.MouseDown(norm_x, norm_y) == true) return true;
	}

    return false;
}

void CInterfaceControlArray::PositionVisibleControls() {
    //compute the left of the controls.
    int32 left = norm_x + 5;

    //go through all the visible controls.
    for (int32 i = 0; i < num_rows; i++) {
        //get the control.
        CInterfaceControl *control = controls.Item(i + top_index);

        if (control == NULL) continue;

        //compute the top of this control.
        int32 top = norm_y + 5 + i * row_height;

        //position the control.
        control->SetPosition(left, top);
    }
}

void CInterfaceControlArray::ShowScrollButtons() {
    //check if we need to show the up scroll button.
    if (top_index > 0) {
        scroll_up.Enable(true);
    }
    else {
        scroll_up.Enable(false);
    }

    //check if we need to show the down scroll button.
    if (top_index + num_rows < controls.Num()) {
        scroll_down.Enable(true);
    }
    else {
        scroll_down.Enable(false);
    }
}



hilight_delay_control::hilight_delay_control() {
    timer_on = false;

    row = -1;
    col = -1;
}

void hilight_delay_control::Start(int row, int column, int delay) {
    //check if we have the timer already started for the given row and column
    if (timer_on == true && row == this->row && column == col) {
        //the timer is already going.
        return;
    }

    //remember what row and column to hilight when the time expires.
    this->row = row;
    this->col = column;

    //get the current time.
    timer_start = GetTickCount();

    //turn the timer on
    timer_on = true;

    //save the time.
    time = delay;
}

void hilight_delay_control::Abort() {
    timer_on = false;
}

int hilight_delay_control::Row() {
    return row;
}   

int hilight_delay_control::Col() {
    return col;
}

bool hilight_delay_control::TimeUp(int cur_time) {
    if (timer_on == false) return false;

    //get the difference between the start and current time.
    int elapsed_time = cur_time - timer_start;

    if (elapsed_time >= time) {
        //time is up.
        return true;
    }

    //must wait longer.
    return false;
}

bool hilight_delay_control::On() {
    return timer_on;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
// CInterfaceHighlight - Just a box that can have a solid/translucent background
//----------------------------------------------------------------------------------------------------------------------------------------------------
void CInterfaceHighlight::PlaceEntities()
{
	if (thickness > 0)
		CInterfaceBox::PlaceEntities();

	if (alpha > 0.01)
	{
		//get the current x and y stretch factors.
		float norm_to_screen_x = viddef.width / 640.0f;
		float norm_to_screen_y = viddef.height / 480.0f;

		//get screen coordinates for the text.
		int32 screen_l = norm_to_screen_x * (left + 2);
		int32 screen_t = norm_to_screen_y * (top + 2);
		int32 screen_r = norm_to_screen_x * (right - 2);
		int32 screen_b = norm_to_screen_y * (bottom - 1);

		DRAWSTRUCT drawStruct;
		memset(&drawStruct,0,sizeof(drawStruct));

		drawStruct.pImage = NULL;
		drawStruct.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
		
		drawStruct.rgbColor = color;						// init rgb
		
		drawStruct.nLeft   = screen_l;							// x position
		drawStruct.nTop    = screen_t;							// y position
		drawStruct.nRight  = screen_r-screen_l;					// set width of nodule
		drawStruct.nBottom = screen_b-screen_t;					// height of nodule
		drawStruct.fAlpha  = alpha;
		
		re.DrawPic( drawStruct ); // do it
	}
}

//---------------------------------------------------------------------------------------------------------------------
// Init - Initializes the hilight bar
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceHighlight::Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, float _alpha)
{
	CInterfaceBox::Init(left,top,right,bottom,thickness);
	alpha = _alpha;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
// CInterfaceScroller
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define SCROLL_THUMB_HEIGHT									(16)
#define SCROLL_THUMB_WIDTH									(16)

#define SCROLL_HEIGHT										(21)
#define SCROLL_WIDTH										(20)

//---------------------------------------------------------------------------------------------------------------------
// Init - Initializes the scroller
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, bool bVertical)
{
	CInterfaceBox::Init(left,top,right,bottom,thickness);
	vertical = bVertical;
	
	if (vertical)
	{
		// up/left button
		scrollMin.Init(left, top, menu_font, menu_font_bright, button_font);
		scrollMin.InitGraphics(TOENAIL_UP_OFF_STRING, TOENAIL_UP_ON_STRING, TOENAIL_UP_DISABLE_STRING);
		scrollMin.SetText(ARROW_UP_STRING, true);
		// down button
		scrollMax.Init(left, bottom - SCROLL_HEIGHT, menu_font, menu_font_bright, button_font);
		scrollMax.InitGraphics(TOENAIL_DOWN_OFF_STRING, TOENAIL_DOWN_ON_STRING, TOENAIL_DOWN_DISABLE_STRING);
		scrollMax.SetText(ARROW_DOWN_STRING, true);

		_left = left;_right = right;
		_top = top + SCROLL_HEIGHT;
		_bottom = bottom -SCROLL_HEIGHT;

		scrollThumb.Init(_left,_top,_right,_top + SCROLL_HEIGHT, 2, true);
	}
	else
	{
		// up/left button
		scrollMin.Init(left, top, menu_font, menu_font_bright, button_font);
		scrollMin.InitGraphics(TOENAIL_LEFT_OFF_STRING, TOENAIL_LEFT_ON_STRING, TOENAIL_LEFT_DISABLE_STRING);
		scrollMin.SetText(ARROW_LEFT_STRING, true);
		// down button
		scrollMax.Init(right - SCROLL_WIDTH, top, menu_font, menu_font_bright, button_font);
		scrollMax.InitGraphics(TOENAIL_RIGHT_OFF_STRING, TOENAIL_RIGHT_ON_STRING, TOENAIL_RIGHT_DISABLE_STRING);
		scrollMax.SetText(ARROW_RIGHT_STRING, true);

		_top = top; _bottom = bottom;
		_left = left + SCROLL_WIDTH;
		_right = right - SCROLL_WIDTH;

		scrollThumb.Init(_left,_top,_left + SCROLL_WIDTH, _bottom, 2, false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// SetRange - Set the operating range of the scroller.  This is different from top-bottom, etc.
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::SetRange(int32 _min, int32 _max, int32 _ticks, int32 _pageSize)
{
	min = _min;
	max = _max;
	ticks = _ticks;
	tickSize = ((float)(max - min)) / ((float)ticks);
	pageSize = _pageSize;
	showThumb = ((max - min) > pageSize);
	scrollThumb.Enable(showThumb);
	SetScrollPos(0);
}

//---------------------------------------------------------------------------------------------------------------------
// Tick - Moves the selection by num spaces.  goes as far as it can.
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::Tick(int32 num)
{
	SetScrollPos(current_position + num * tickSize);
}

//---------------------------------------------------------------------------------------------------------------------
// SetScrollPos - Moves the scroll bar to the desired position
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::SetScrollPos(int32 pos)
{
	// validate the new position
	current_position = pos;
	bound_min(current_position,min);
	bound_max(current_position,max);

	// enable the scroll buttons
	scrollMin.Enable(current_position > min);
	scrollMax.Enable(current_position < max);

	// set up the thumb position
	int32 _min,_max,dist;
	if (vertical)
	{
		_min = _top;//top + SCROLL_HEIGHT;
		_max = _bottom - SCROLL_THUMB_HEIGHT;//bottom - SCROLL_THUMB_HEIGHT - SCROLL_HEIGHT;
	}
	else
	{
		_min = _left;//left + SCROLL_WIDTH;
		_max = _right - SCROLL_THUMB_WIDTH;//right - SCROLL_THUMB_WIDTH - SCROLL_WIDTH;
	}

	dist = _max - _min;

	if (!showThumb)
	{
		thumb_frac = 0;
	}
	else
		thumb_frac = ((float)(current_position - min)) / ((float)(max - min));

	scrollThumb.Enable(showThumb);
	int32 thumb_pos = _min + (int32)(dist * thumb_frac);

	if (vertical)
		scrollThumb.SetPosition(left, thumb_pos, right, thumb_pos + SCROLL_HEIGHT, true);
	else
		scrollThumb.SetPosition(thumb_pos, top, thumb_pos + SCROLL_WIDTH, bottom, false);

}

//---------------------------------------------------------------------------------------------------------------------
// Animate
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::Animate(int32 elapsed_time)
{
	if (!enabled)
		return;

	CInterfaceBox::Animate(elapsed_time);
	scrollMin.Animate(elapsed_time);
	scrollMax.Animate(elapsed_time);

	if (showThumb)
		scrollThumb.Animate(elapsed_time);
}

//---------------------------------------------------------------------------------------------------------------------
// PlaceEntities
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::PlaceEntities()
{
	if (!enabled)
		return;

	CInterfaceBox::PlaceEntities();
	scrollMax.PlaceEntities();
	scrollMin.PlaceEntities();

	if (showThumb)
		scrollThumb.PlaceEntities();
}

//---------------------------------------------------------------------------------------------------------------------
// DrawForeground
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::DrawForeground()
{
	if (!enabled)
		return;

	CInterfaceBox::DrawForeground();
	scrollMin.DrawForeground();
	scrollMax.DrawForeground();

	if (showThumb)
		scrollThumb.DrawForeground();
}

//---------------------------------------------------------------------------------------------------------------------
// DrawBackGround
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScroller::DrawBackground()
{
	if (!enabled)
		return;

	CInterfaceBox::DrawBackground();
	scrollMin.DrawBackground();
	scrollMax.DrawBackground();

	if (showThumb)
		scrollThumb.DrawBackground();
}

//---------------------------------------------------------------------------------------------------------------------
// MousePos
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceScroller::MousePos(int32 norm_x, int32 norm_y)
{
	if (!enabled)
		return false;

	scrollThumb.MousePos(norm_x,norm_y);
	if (scrollThumb.have_capture)
	{
		return CheckMoveThumb(norm_x,norm_y);
	}

	if (scrollMin.MousePos(norm_x,norm_y)) return true;
	if (scrollMax.MousePos(norm_x,norm_y)) return true;
	if (CInterfaceBox::MousePos(norm_x,norm_y)) return true;

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// MouseDown
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceScroller::MouseDown(int32 norm_x, int32 norm_y)
{
	if (!enabled)
		return false;

	if (scrollThumb.MouseDown(norm_x,norm_y)) return true;
	if (scrollMin.MouseDown(norm_x,norm_y)) return true;
	if (scrollMax.MouseDown(norm_x,norm_y)) return true;
	if (CInterfaceBox::MouseDown(norm_x,norm_y)) return true;

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// MouseUp
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceScroller::MouseUp(int32 norm_x, int32 norm_y)
{
	if (!enabled)
		return false;

	if (scrollThumb.MouseUp(norm_x,norm_y)) return true;
	
	// check the up/down/left/right buttons
	if (scrollMin.MouseUp(norm_x,norm_y))
	{
		SetScrollPos(current_position - tickSize);
		return true;
	}

	if (scrollMax.MouseUp(norm_x,norm_y))
	{
		SetScrollPos(current_position + tickSize);
		return true;
	}

	// check to see if we should move the thumb.
	if (CheckMoveThumb(norm_x,norm_y)) return true;

	return false;
}

bool CInterfaceScroller::CheckMoveThumb(int32 norm_x, int32 norm_y)
{
	if (showThumb && (scrollThumb.have_capture || ( !scrollThumb.have_capture && (norm_x >= _left) && (norm_x <= _right) && (norm_y >= _top) && (norm_y <= _bottom)) ))
	{
		float frac;
		float dist;
		if (vertical)
		{
			dist = _bottom - _top;
			frac = ((float)(norm_y - _top)) / ((float)dist);
		}
		else
		{
			dist = _right - _left;
			frac = ((float)(norm_x - _left)) / ((float)dist);
		}

		int32 pos = min + frac * ticks;

		if (scrollThumb.have_capture)
			SetScrollPos(pos);
		else
		{
			if (pos >= current_position)
				SetScrollPos(current_position + pageSize*tickSize);
			else if (pos < current_position)
				SetScrollPos(current_position - pageSize*tickSize);
		}

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// Keydown
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceScroller::Keydown(int32 key)
{
	if (!enabled)
		return false;

	return false;
}

CInterfaceScrollThumb::CInterfaceScrollThumb()
{
	have_capture = under_cursor = false;
}
//---------------------------------------------------------------------------------------------------------------------
// Init - Thumb control for the scroll bar control
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScrollThumb::Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, bool bVertical)
{
	have_capture = under_cursor = false;
	SetPosition(left,top,right,bottom,bVertical);
}

void CInterfaceScrollThumb::SetPosition(int32 left, int32 top, int32 right, int32 bottom, bool bVertical)
{
	CInterfaceBox::Init(left,top,right,bottom,thickness);
	vertical = bVertical;
}

bool CInterfaceScrollThumb::HitTest(int32 norm_x, int32 norm_y) 
{
    return !(norm_x < left || norm_x > right || norm_y < top || norm_y > bottom);
}

bool CInterfaceScrollThumb::MousePos(int32 norm_x, int32 norm_y)
{
	bool ret = false;
    if (!HitTest(norm_x, norm_y)) 
	{
        //we are off the button.
        under_cursor = false;
        ret = false;
    }
    //we are on the button.
    else if (enabled) 
	{
        //if the mouse cursor wasn't already on the button, make a sound.
        if (!under_cursor && !have_capture) 
		{
            S_StartLocalSound(DKM_sounds[DKMS_BUTTON3]);
        }

        //we are on the button.
        under_cursor = true;
        ret = true;
    }

    //if we have the capture, we return true no matter what.
    if (have_capture) 
	{
        ret = true;
    }

    return ret;
}

bool CInterfaceScrollThumb::MouseUp(int32 norm_x, int32 norm_y)
{
    //if we dont have the capture, do nothing.
    if (!have_capture) return false;

    //we lose capture.
    have_capture = false;

    //we have capture, check if the mouse is still over us.
    if (!HitTest(norm_x, norm_y)) return false;

    //the mouse was over us.  we were pressed.
    return true;
}

bool CInterfaceScrollThumb::MouseDown(int32 norm_x, int32 norm_y)
{
    //we must be enabled to handle the mouse down event.
    if (!enabled) return false;

    if (!HitTest(norm_x, norm_y)) return false;

    //capture the mouse.
    have_capture = true;
    
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
// PlaceEntities
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceScrollThumb::PlaceEntities()
{
	if (!enabled)
		return;

	DRAWSTRUCT drawStruct;
	memset(&drawStruct,0,sizeof(drawStruct));

	drawStruct.pImage = NULL;
	drawStruct.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
	
	drawStruct.rgbColor.Set(1,0,0);						// init rgb
	
	//get the current x and y stretch factors.
	float norm_to_screen_x = viddef.width / 640.0f;
	float norm_to_screen_y = viddef.height / 480.0f;

	//get screen coordinates for the text.
	int32 screen_l = norm_to_screen_x * left;
	int32 screen_t = norm_to_screen_y * top;
	int32 screen_r = norm_to_screen_x * right;
	int32 screen_b = norm_to_screen_y * bottom;

	drawStruct.nLeft   = screen_l;							// x position
	drawStruct.nTop    = screen_t;							// y position
	drawStruct.fAlpha  = 0.1;

	if (under_cursor || have_capture)
		drawStruct.fAlpha += 0.1;

	if (vertical)
	{
		drawStruct.nRight  = screen_r-screen_l;					// set width of nodule
		drawStruct.nBottom = SCROLL_THUMB_HEIGHT * norm_to_screen_y;			// height of nodule
	}
	else
	{
		drawStruct.nRight  = SCROLL_THUMB_WIDTH * norm_to_screen_x;			// set width of nodule
		drawStruct.nBottom = screen_b-screen_t;					// height of nodule
	}

	while (1)
	{
		re.DrawPic( drawStruct ); // do it
		if (vertical)
		{
			drawStruct.nRight  = screen_r-screen_l;					// set width of nodule
			drawStruct.nBottom -= 4 * norm_to_screen_y;							// height of nodule
			drawStruct.nTop += 2 * norm_to_screen_y;
			if (drawStruct.nBottom <= 1)
				break;
		}
		else
		{
			drawStruct.nBottom = screen_b-screen_t;					// height of nodule
			drawStruct.nLeft += 2 * norm_to_screen_x;
			drawStruct.nRight  -= 4 * norm_to_screen_x;							// set width of nodule
			if (drawStruct.nRight <= 1)
				break;
		}
		
		drawStruct.fAlpha += 0.02;
		if (drawStruct.fAlpha >= 0.8)
			break;
	}
}


//----------------------------------------------------------------------------------------------------------------------------------------------------
// CInterfaceFileBox
//----------------------------------------------------------------------------------------------------------------------------------------------------
CInterfaceFileBox::CInterfaceFileBox():
nameLeft(0),dateLeft(0),timeLeft(0),
nameWidth(0),dateWidth(0),timeWidth(0),
lineHeight(0),lines(0),
top_file_index(0),selected_file_index(-1),always_highlight_index(-1),
scroller(),
file_list_box(),
isDateTime(true)
{
	CInterfaceBox::CInterfaceBox();
	memset(filter,0,sizeof(filter));
	refreshFunc = DefRefresh;
	verifyFunc = NULL;
}

//---------------------------------------------------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, char *_path, char *_filter, char *_ignore, int32 _lineHeight, int32 _lines)
{
	_ASSERTE(_lines);

	lineHeight = _lineHeight;
	lines = _lines;

	int32 bTest = top + lines*lineHeight + 5;
	if (bottom > bTest)
	{
		bottom = bTest;
	}

	CInterfaceBox::Init(left,top,right,bottom,thickness);

	strcpy(filter,_filter);
	if (_ignore)
		strcpy(ignore,_ignore);
	strcpy(path,_path);

	// scroll bar
	scroller.Init(right - SCROLL_WIDTH,top,right,bottom,2,true);
	static_highlight.SetBKColor(CVector(1,1,0));
}

//---------------------------------------------------------------------------------------------------------------------
// sets up the displayable fields.  Left is the starting position of the text.  Width is the width.  0 will omit this field
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::SetFields(int32 fnLeft, int32 fnWidth, int32 fdLeft, int32 fdWidth, int32 ftLeft, int32 ftWidth)
{
	nameLeft = fnLeft;				// character offsets of the fields
	dateLeft = fdLeft;
	timeLeft = ftLeft;

	nameWidth = fnWidth;				// character widths of the fields (0 to not draw)
	dateWidth = fdWidth;
	timeWidth = ftWidth;
}

#define SELECTED_VALID(sel_index)			((sel_index != -1) && (sel_index >= top_file_index) && (sel_index < (top_file_index + lines)))

//---------------------------------------------------------------------------------------------------------------------
// Animate
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::Animate(int32 elapsed_time)
{
	CInterfaceBox::Animate(elapsed_time);
	scroller.Animate(elapsed_time);

	if (SELECTED_VALID(selected_file_index) && (selected_file_index != always_highlight_index))
		selection_box.Animate(elapsed_time);

	if (SELECTED_VALID(always_highlight_index))
		static_highlight.Animate(elapsed_time);
}

//---------------------------------------------------------------------------------------------------------------------
// PlaceEntities
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::PlaceEntities()
{
	CInterfaceBox::PlaceEntities();
	scroller.PlaceEntities();

	if (SELECTED_VALID(selected_file_index) && (selected_file_index != always_highlight_index))
		selection_box.PlaceEntities();

	if (SELECTED_VALID(always_highlight_index))
		static_highlight.PlaceEntities();
}

//---------------------------------------------------------------------------------------------------------------------
// DrawForeGround
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::DrawForeground()
{
	scroller.DrawForeground();

	if (SELECTED_VALID(selected_file_index) && (selected_file_index != always_highlight_index))
		selection_box.DrawForeground();

	if (SELECTED_VALID(always_highlight_index))
		static_highlight.DrawForeground();

	DrawFileLines();
}

//---------------------------------------------------------------------------------------------------------------------
// DrawBackGround
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::DrawBackground()
{
	scroller.DrawBackground();

	if (SELECTED_VALID(selected_file_index) && (selected_file_index != always_highlight_index))
		selection_box.DrawBackground();

	if (SELECTED_VALID(always_highlight_index))
		static_highlight.DrawBackground();
}

//---------------------------------------------------------------------------------------------------------------------
// FileLine - Draws a line in the box, using the passed strings.  prints in position i in the list box
//---------------------------------------------------------------------------------------------------------------------
#define FILE_WIDTH(c)										(8*c)
#define FILE_PLACEMENT(c)									(left + 5 + FILE_WIDTH(c))

void CInterfaceFileBox::FileLine(int i, bool hilighted, const char *file, const char *date, const char *time )
{
	int yOff = top + lineHeight * i;
	// file
	if (nameWidth)
		DKM_DrawString(FILE_PLACEMENT(nameLeft), yOff, file,FILE_WIDTH(nameWidth), hilighted);

	// date
	if (dateWidth)
		DKM_DrawString(FILE_PLACEMENT(dateLeft), yOff, date,FILE_WIDTH(dateWidth), hilighted);

	// time
	if (timeWidth)
		DKM_DrawString(FILE_PLACEMENT(timeLeft), yOff, time,FILE_WIDTH(timeWidth), hilighted);
}

//---------------------------------------------------------------------------------------------------------------------
// DrawFileLines - Draws lines of text until the list box has been filled
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::DrawFileLines()
{
	int index;
	bool hilighted = false;
	//draw as many local games as we can fit in the list.
	for (int i = 0; i < lines; i++)
	{
		index = i + top_file_index;
		if (index < 0)
		{
			i--;
			continue;
		}

		bool hilighted = ( (index == selected_file_index) || (i == highlighted_file_index) );
		
		if (strlen(file_list[index].name) == 0) 
		{
			FileLine(i,false,"--","--","--");
		}
		else
		{
			FileLine(i,hilighted,file_list[index].comment,file_list[index].field1,file_list[index].field2);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
// MousePos
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::MousePos(int32 norm_x, int32 norm_y)
{
	if (scroller.MousePos(norm_x,norm_y))
	{
		TopIndex(scroller.CurrentPos());
		return true;
	}

	highlighted_file_index = HitTest(norm_x,norm_y);//list_box_hit_test(left, top, right, bottom, norm_x, norm_y, lines);

	// do a clicky thing if this item is different than the last one
	if (highlighted_file_index != o_h_file_index)
	{
		if (highlighted_file_index != -1)
			S_StartLocalSound(DKM_sounds[DKMS_BUTTON3]);
		o_h_file_index = highlighted_file_index;
	}

	if (CInterfaceBox::MousePos(norm_x,norm_y)) return true;
	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// MouseUp
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::MouseUp(int32 norm_x, int32 norm_y)
{
	if (scroller.MouseUp(norm_x,norm_y))
	{
		TopIndex(scroller.CurrentPos());
		return true;
	}

	if (CInterfaceBox::MouseUp(norm_x,norm_y)) return true;

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// MouseDown
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::MouseDown(int32 norm_x, int32 norm_y)
{
	if (scroller.MouseDown(norm_x,norm_y)) return true;

	// no deselecting just cuz we click on something empty
	int32 si = HitTest(norm_x,norm_y);//list_box_hit_test(left, top, right, bottom, norm_x, norm_y, lines);
	if (si != -1)
	{
		Select(si + top_file_index);
		return true;
	}

	if (CInterfaceBox::MouseDown(norm_x,norm_y)) return true;
	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// Keydown
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::Keydown(int32 key)
{
//	        if (!(key == K_MWHEELUP || key == K_MWHEELDOWN  || (key >= K_MOUSE1 && key <= K_MOUSE3)))
    switch (key)
    {
    case K_UPARROW:
		Select(selected_file_index - 1);
        return true;

    case K_DOWNARROW:
		Select(selected_file_index + 1);
        return true;

    case K_PGUP:
		scroller.Tick(-4);
		TopIndex(scroller.CurrentPos());
        return true;

    case K_PGDN:
		scroller.Tick(4);
		TopIndex(scroller.CurrentPos());
        return true;
	case K_MWHEELUP:
		scroller.Tick(-1);
		TopIndex(scroller.CurrentPos());
        return true;

	case K_MWHEELDOWN:
		scroller.Tick(1);
		TopIndex(scroller.CurrentPos());
        return true;
    }

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// Select - Places a selection box around the item.  also sets the selected item index.  always sets the static selection box
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::Select(int32 index, bool bAlways)
{
	// clear selection
	bool result = true;
	selected_file_index = index;
	if (selected_file_index < 0)
	{
		selected_file_index = 0;
		result = false;
	}
	else if (selected_file_index >= total_files)
	{
		selected_file_index = total_files - 1;
		result = false;
	}

	if (selected_file_index < top_file_index)
	{
		scroller.SetScrollPos(selected_file_index);
		TopIndex(scroller.CurrentPos());
	}
	else if (selected_file_index >= (top_file_index + lines))
	{
		int32 newIndex = selected_file_index - lines + 1;
		scroller.SetScrollPos(newIndex);
		TopIndex(newIndex);
	}
	else
	{
		SelectBoxUpdate(selection_box,selected_file_index,true);
	}

	if (result && bAlways)
	{
		always_highlight_index = selected_file_index;
		SelectBoxUpdate(static_highlight,always_highlight_index,false);
	}

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
// Select - Finds the item with name and selects it.  always sets the static selection box
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::Select(char *name, bool bAlways)
{
	char *findName = StripStr(name);
	for (int i = 0; i < total_files; i++)
	{
		if (!strnicmp(file_list[i].name,findName,strlen(name)))
		{
			Select(i,bAlways);
			return true;
		}
	}
	// select first item by default....
	Select(0,false);
	return false;
}

//---------------------------------------------------------------------------------------------------------------------
// Selection - Retrieve the selected item's data into the given character arrays.  arrays should be sized as in the header
//---------------------------------------------------------------------------------------------------------------------
bool CInterfaceFileBox::Selection(char *name, char *field1, char *field2, char *comment)
{
	if ((selected_file_index < 0) || (selected_file_index > total_files - 1) || (!name && !field1 && !field2))
		return false;

	config_file_t selConfig = file_list[selected_file_index];

	if (name)
	{
		strcpy(name,selConfig.name);
	}

	if (field1)
	{
		strcpy(field1,selConfig.field1);
	}

	if (field2)
	{
		strcpy(field2,selConfig.field2);
	}

	if (comment)
	{
		strcpy(comment, file_list[total_files].comment);
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// HitTest - determines which item is closest to the mouse cursor
//---------------------------------------------------------------------------------------------------------------------
int32 CInterfaceFileBox::HitTest(int32 x, int32 y)
{
	int32 result = ((x > left) && (x < (right - SCROLL_WIDTH)) && (y > top) && (y < bottom)) ? (((y-top + (lineHeight * 0.5)) - 0.5 * lineHeight) / lineHeight) : (-1);
	if ((result >= 0) && (result < lines)) 
	{
		int32 index = top_file_index + result;
		if (index > total_files - 1)
			return -1;
	}
	else 
		return -1;

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
// TopIndex - The given item is placed at the first position in the list box
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::TopIndex(int32 index)
{
	if ((index < 0) || (lines > total_files))
		index = 0;
	else if (index + lines > total_files)
		index = total_files - lines;

    top_file_index = index;
	// move the selection box, if necessary.
	SelectBoxUpdate(selection_box,selected_file_index,true);
	SelectBoxUpdate(static_highlight,always_highlight_index,false);
}

//---------------------------------------------------------------------------------------------------------------------
// Clear - Empties the file list
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::Clear()
{
	memset(&file_list,0,MAX_FILES * sizeof(config_file_t));
}

//---------------------------------------------------------------------------------------------------------------------
// StripStr - will strip off the path and/or the extension
//---------------------------------------------------------------------------------------------------------------------
char *CInterfaceFileBox::StripStr(char *str)
{
	char *fn = str;
	if (stripPath)
	{
		fn = strrchr(str,'/');
		if (fn && fn++)
			str = fn;
	}
	if (stripExt)
	{
		fn = strchr(str,'.');
		if (fn)
			*fn = NULL;
	}
	return str;
}
void Sys_DateTime(char *path, char *date, int dateSize, char *time, int timeSize);

//---------------------------------------------------------------------------------------------------------------------
// DefRefresh - default refresh func
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::DefRefresh(CInterfaceFileBox *self)
{
	char *_ignore = self->ignore;
	char *found;
	char name[MAX_OSPATH];
	char* ext;

	char date[MAX_FIELD];
	char time[MAX_FIELD];

	strcpy(name,self->path);
	strcat(name,self->filter);

	ext = strstr(self->filter,".");

	// read in the list here
	found = Sys_FindFirst(name, 0, 0 );
	while (found && self->Count() < MAX_FILES)
	{
		found = self->StripStr(found);
		if (found && (strlen(found) > 0))
		{
			strcpy(name,self->path);
			strcat(name,found);
			strcat(name,ext);

			Sys_DateTime(name,date,MAX_FIELD,time,MAX_FIELD);
			if (!(_ignore && (strlen(_ignore) > 0) && !strnicmp(found,_ignore,strlen(_ignore))))
			{
				self->AddItem(found,date,time,found);
			}
		}
		found = Sys_FindNext(0,0);
	}
	Sys_FindClose ();
}

//---------------------------------------------------------------------------------------------------------------------
// Refresh - clear and reload the file list
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::Refresh()
{
	total_files = 0;

	Clear();
	
	// call the refresh function
	refreshFunc(this);

	// verify what the refresher put there
	if (verifyFunc)
	{
		int cur = -1;
		int stop = total_files;
		for (int i = 0; i < stop; i++)
		{
			if (!(verifyFunc(file_list[i].name,file_list[i].field1,file_list[i].field2,file_list[i].comment)))
			{
				// bad...clear it out and set up the pointer.
				total_files--;
				memset(&file_list[i],0,sizeof(config_file_t));
				if (cur == -1) cur = i;		// don't disturb this offset if it's non-zero!
			}
			else
			{
				// if cur is non-zero, it means we cleaned out a spot and need to fill it in.
				if (cur != -1)
				{
					file_list[cur] = file_list[i];
					memset(&file_list[i],0,sizeof(config_file_t));
					while ((strlen(file_list[cur].name)) && (cur < i)) cur++;
					if (cur > i)
						cur = i;
				}
			}

		}
	}

	TopIndex(0);
	scroller.SetRange(0,total_files - lines, total_files - lines, 4);
	scroller.Enable(lines < total_files);

	QSort(file_list,0,total_files-1, sortField1, sortField2, sortField3);
}

bool CInterfaceFileBox::AddItem(char *name, char *field1, char *field2, char *comment)
{
	if ( !name || (total_files >= MAX_FILES))
		return false;

	if (name)
	{
		Com_sprintf(file_list[total_files].name,MAX_OSPATH,"%s",name);
	}

	if (field1)
	{
		Com_sprintf(file_list[total_files].field1,MAX_FIELD-1,"%s",field1);
	}

	if (field2)
	{
		Com_sprintf(file_list[total_files].field2,MAX_FIELD-1,"%s",field2);
	}

	if (comment)
	{
		Com_sprintf(file_list[total_files].comment,MAX_FIELD-1,"%s",comment);
	}

	total_files++;
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
// SelectBoxUpdate - Moves the selection box as the selected item is scrolled around.
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::SelectBoxUpdate(CInterfaceHighlight	&box, int32 selIndex, bool sound)
{
	int32 index = selIndex - top_file_index;
//	if ((index < 0) || (index >= lines))
//		return;

	int yOff = top + lineHeight * index;

	int32 _right = right;
	if (scroller.IsEnabled())
		_right -= SCROLL_WIDTH;

	if (sound && (old_s_file_index != selected_file_index))
	{
		S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);  // play sound
		old_s_file_index = selIndex;
	}

	box.Init(left, yOff, _right, yOff + lineHeight + 2, 2,0.3);
}

//---------------------------------------------------------------------------------------------------------------------
// CompareField compares the given field of the two files
// if equal, returns 0
// if first < second, returns -1 if _UP and returns 1 if _DN
// if first > second, returns 1 if _UP and returns -1 if _DN
//---------------------------------------------------------------------------------------------------------------------
#define IS_DASH_DASH(str)	(( strncmp(str,"--",2) == 0) ? true : false)
int Sys_CompareFileDateTime(char *path1, char *path2);
int CInterfaceFileBox::CompareField(config_file_t &first, config_file_t &second, short field)
{
	if (!isDateTime)
	{
		switch(field)
		{
		case SORT_NAME_UP:
			return stricmp(first.name,second.name);
			break;
		case SORT_NAME_DN:
			return -stricmp(first.name,second.name);
			break;

		case SORT_FIELD1_UP:
			return stricmp(first.field1,second.field1);
			break;
		case SORT_FIELD1_DN:
			return -stricmp(first.field1,second.field1);
			break;

		case SORT_FIELD2_UP:
			return stricmp(first.field2,second.field2);
			break;
		case SORT_FIELD2_DN:
			return -stricmp(first.field2,second.field2);
			break;
		}
	}
	else
	{
		switch(field)
		{
		case SORT_NAME_UP:
			return stricmp(first.name,second.name);
			break;
		case SORT_NAME_DN:
			return -stricmp(first.name,second.name);
			break;

		case SORT_FIELD1_UP:
		case SORT_FIELD2_UP:
			{
				// check for fields with --
				bool isFirstDash = IS_DASH_DASH(first.field1) || IS_DASH_DASH(first.field2);
				bool isSecondDash = IS_DASH_DASH(second.field1) || IS_DASH_DASH(second.field2);
				if (isFirstDash && isSecondDash)
				{
					return 0;
				}
				else if (isFirstDash)
				{
					return -1;
				}
				else if (isSecondDash)
				{
					return 1;
				}
				else
				{
					char path1[MAX_OSPATH];
					char path2[MAX_OSPATH];
					Com_sprintf(path1,sizeof(path1),"%s/save/%s/game.ssv",FS_SaveGameDir(),first.name);
					Com_sprintf(path2,sizeof(path2),"%s/save/%s/game.ssv",FS_SaveGameDir(),second.name);
					return Sys_CompareFileDateTime(path1,path2);
				}
				break;
			}
		case SORT_FIELD1_DN:
		case SORT_FIELD2_DN:
			{
				// check for fields with --
				bool isFirstDash = IS_DASH_DASH(first.field1) || IS_DASH_DASH(first.field2);
				bool isSecondDash = IS_DASH_DASH(second.field1) || IS_DASH_DASH(second.field2);
				if (isFirstDash && isSecondDash)
				{
					return 0;
				}
				else if (isFirstDash)
				{
					return 1;
				}
				else if (isSecondDash)
				{
					return -1;
				}
				else
				{
					char path1[MAX_OSPATH];
					char path2[MAX_OSPATH];
					Com_sprintf(path1,sizeof(path2),"%s/save/%s/game.ssv",FS_SaveGameDir(),first.name);
					Com_sprintf(path2,sizeof(path2),"%s/save/%s/game.ssv",FS_SaveGameDir(),second.name);
					return -Sys_CompareFileDateTime(path1,path2);
				}
				break;
			}
		}
	}
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// Compare - compares the two files by field1 then field2 then field3
//---------------------------------------------------------------------------------------------------------------------
int CInterfaceFileBox::Compare(config_file_t &first, config_file_t &second, short field1, short field2, short field3)
{
	int result = CompareField(first,second,field1);
	if (!result && (field2 != SORT_NONE))						// they're equal, try field 2
	{
		result = CompareField(first,second,field2);
		if (!result && (field3 != SORT_NONE))					// they're equal too, try field 3
		{
			result = CompareField(first,second,field3);
		}
	}
	
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
// QSPartition - Quicksort partitioning function.  goes up or down.
//---------------------------------------------------------------------------------------------------------------------
int CInterfaceFileBox::QSPartition(config_file_t ary[], int l, int r, short field1, short field2, short field3)
{
	int i = l-1;
	int j = r;
	config_file_t v = ary[r];

	config_file_t temp;
	while (1)
	{
		while (Compare(ary[++i],v,field1,field2,field3) < 0);
		while (Compare(v,ary[--j],field1,field2,field3) < 0) if (j == i) break;
		if (i >= j) break;
		
		temp = ary[i];
		ary[i] = ary[j];
		ary[j] = temp;
	}

	temp = ary[i];
	ary[i] = ary[r];
	ary[r] = temp;
	return i;
}

//---------------------------------------------------------------------------------------------------------------------
// QSort - um, Quicksort
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::QSort(config_file_t ary[], int l, int r, short field1, short field2, short field3)
{
	int i;
	if (r <= l) return;

	i = QSPartition(ary,l,r,field1,field2,field3);
	QSort(ary,l,i-1,field1,field2,field3);
	QSort(ary,i+1,r,field1,field2,field3);
}

//---------------------------------------------------------------------------------------------------------------------
// SortBy - sorts the file list by field1 then field2 then field 3.
//---------------------------------------------------------------------------------------------------------------------
void CInterfaceFileBox::SortBy(short field1, short field2, short field3)
{
	sortField1 = field1;
	sortField2 = field2;
	sortField3 = field3;
}

CInterfaceStatusText::CInterfaceStatusText()
{
	memset(&text,0,sizeof(text));
	int32 drawMS = 0;
}

void CInterfaceStatusText::Animate(int32 elapsed_time)
{
	drawMS -= elapsed_time;
	bound_min(drawMS,0);
}

void CInterfaceStatusText::PlaceEntities()
{
	if (drawMS)
	{
		if (thickness)
			CInterfaceBox::PlaceEntities();
	}
}

void CInterfaceStatusText::DrawForeground()
{
	if (drawMS)
	{
		if (thickness)
			CInterfaceBox::DrawForeground();
		DKM_DrawString(left + 4,top + 1,text,NULL,false,true);
	}
}

void CInterfaceStatusText::DrawBackground()
{
	if (drawMS)
	{
		if (thickness)
			CInterfaceBox::DrawBackground();
	}
}

void CInterfaceStatusText::SetText(char *_text, int32 timeS)
{
	strcpy(text,_text);
	drawMS = timeS * 1000;
}

CInterfaceConfirmDlg::CInterfaceConfirmDlg()
{
	message1=message2=prompt=NULL;
	memset(lines,0,sizeof(lines));
	lineCount = 0;
	showing = false;
	bnClicked = false;
	yesClicked = false;
}

void CInterfaceConfirmDlg::Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, int32 ynTop, int32 yLeft, int32 nLeft)
{
	CInterfaceBox::Init(left,top,right,bottom,thickness);

	// init the positions of the yes/no buttons
	bnTop = ynTop;
    yes.Init(yLeft, ynTop, menu_font, menu_font_bright, button_font);
    yes.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
    yes.SetText(tongue_menu[T_MENU_YES], true);

    no.Init(nLeft, ynTop, menu_font, menu_font_bright, button_font);
    no.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
    no.SetText(tongue_menu[T_MENU_NO], true);
}

void CInterfaceConfirmDlg::PlaceEntities()
{
	if (showing)
	{
		CInterfaceBox::PlaceEntities();
		yes.PlaceEntities();
		no.PlaceEntities();
	}
}

#define MSG_SPACING 14
void CInterfaceConfirmDlg::DrawForeground()
{
	if (showing)
	{
		int32 center = (left + right) / 2;
		CInterfaceBox::DrawForeground();
		int32 msgTop = top + MSG_SPACING;
/*
		if (message1 && strlen(message1))
		{
			DKM_DrawString(center, msgTop,message1, NULL, true, true);  // highlight title
			msgTop += MSG_SPACING;
		}
		if (message2 && strlen(message2))
		{
			DKM_DrawString(center, msgTop,message2, NULL, true, true);  // highlight title
			msgTop += MSG_SPACING;
		}
*/
		for (int i = 0; i < lineCount; i++)
		{
			if (lines[i] && strlen(lines[i]))
			{
				DKM_DrawString(center, msgTop,lines[i], NULL, true, true);  // highlight title
				msgTop += MSG_SPACING;
			}
		}
		if (prompt && strlen(prompt))
			DKM_DrawString(center, msgTop,prompt, NULL, true, true);  // highlight title

		yes.DrawForeground();
		no.DrawForeground();
	}
}

void CInterfaceConfirmDlg::DrawBackground()
{
	if (showing)
	{
		CInterfaceBox::DrawBackground();
		yes.DrawBackground();
		no.DrawBackground();
	}
}

bool CInterfaceConfirmDlg::MousePos(int32 norm_x, int32 norm_y)
{
	if (showing)
	{
		bool ans = false;
		if (yes.MousePos(norm_x,norm_y)) return true;
		if (no.MousePos(norm_x,norm_y)) return true;
		return ans;
	}

	return false;
}

bool CInterfaceConfirmDlg::MouseUp(int32 norm_x, int32 norm_y)
{
	if (showing)
	{
		if (yes.MouseUp(norm_x,norm_y))
		{
			bnClicked = true;
			yesClicked = true;
			return true;
		}
		if (no.MouseUp(norm_x,norm_y))
		{
			bnClicked = true;
			yesClicked = false;
			return true;
		}
	}

	return false;
}

bool CInterfaceConfirmDlg::MouseDown(int32 norm_x, int32 norm_y)
{
	if (showing)
	{
		if (yes.MouseDown(norm_x,norm_y)) return true;
		if (no.MouseDown(norm_x,norm_y)) return true;
	}

	return false;
}

bool CInterfaceConfirmDlg::Keydown(int32 key)
{
	if (showing)
	{
		switch(key)
		{
		case K_ENTER:
#ifdef TONGUE_ENGLISH
		case 'y':
#endif

			bnClicked = true;
			yesClicked = true;
			break;

		case K_ESCAPE:
#ifdef TONGUE_ENGLISH
		case 'n':
#endif

			bnClicked = true;
			yesClicked = false;
		};
		return true;
	}

	return false;
}

void CInterfaceConfirmDlg::Show(char *_message, char *_prompt)
{
	FreeStrings();
	char message[512];
	Com_sprintf(message,sizeof(message),_message);

	// split up the string
	char *chr = NULL;
	char *msg = message;
	while (lineCount < MAX_LINES)
	{
		chr = strchr(msg,'\n');
		if (chr)
		{
			*chr = NULL;
			lines[lineCount] = strdup(msg);
			msg = chr + 1;
		}
		else
		{
			lines[lineCount] = strdup(msg);
			break;
		}
		lineCount++;
	}
	lineCount++;
/*
	char *chr = strrchr(message,'\n');
	if (chr)
	{
		*chr = NULL;
		message1 = strdup(message);
		message2 = strdup(chr+1);
	}
	else
	{
		message2 = strdup(message);
		message1 = NULL;
	}
*/
	prompt = strdup(_prompt);
	showing = true;
	bnClicked = false;
	yesClicked = false;
	ResetPos();
}

void CInterfaceConfirmDlg::Hide()
{
	FreeStrings();
	showing = false;
	bnClicked = false;
	yesClicked = false;
}

bool CInterfaceConfirmDlg::Result(bool &yes)
{
	yes = false;

	if (showing && bnClicked)
	{
		yes = yesClicked;
		Hide();
		return true;
	}
	else
		return false;
}

void CInterfaceConfirmDlg::FreeStrings()
{
	lineCount = 0;
	for (int i = 0; i < MAX_LINES; i++)
	{
		if (lines[i])
			free(lines[i]);
		lines[i] = NULL;
	}

	if (message1) free(message1);
	if (message2) free(message2);
	if (prompt) free(prompt);

	message1=message2=prompt=NULL;
}

void CInterfaceConfirmDlg::ResetPos()
{
	int32 _top = bnTop - MSG_SPACING;
	if (prompt)
		_top -= MSG_SPACING;

	_top -= MSG_SPACING * (lineCount + 1);
	CInterfaceBox::Init(left,_top,right,bottom,thickness);
}
