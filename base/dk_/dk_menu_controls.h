//#pragma once

#ifndef _DK_MENU_CONTROLS_H_
#define _DK_MENU_CONTROLS_H_

// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif

//classes for generic interface objects that we use
class CInterfaceControl {
public:
    CInterfaceControl();
    ~CInterfaceControl();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //resets the control's state
    virtual void Reset();

    //allows the control to process its animations.
    virtual void Animate(int32 elapsed_time);

    //lets the control draw any 2d artwork that will be behind the 3d models.
    virtual void DrawBackground();
    //places the 3d models.
    virtual void PlaceEntities();
    //lets the control draw any 2d artwork that will be in front of the 3d models.
    virtual void DrawForeground();
    //tells the control that the mouse is moving
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    //tells the control that the mouse is being pressed or released.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool Keydown(int32 key);

    //interface used by CInterfaceControlArray to change the position of the control
    virtual void SetPosition(int32 norm_x, int32 norm_y);
};

class CInterfaceButton : public CInterfaceControl {
public:
    CInterfaceButton();
    ~CInterfaceButton();

    virtual void Reset();

    //positions the button on the screen and resets it's state.
    void Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, void *font_buttons);
    void InitGraphics(const char *upchars, const char *downchars, const char *disablechars);
    void SetText(const char *text, bool center);

protected:
    //the position and size info for the button.
    int32 norm_x, norm_y;

    //the width and height of the button.
    int32 w;
    int32 h;

    //the font graphic that holds the graphics for this button.
    void *font_buttons;

    //the fonts that we use to draw the text.
    void *font_normal;
    void *font_bright;

    //the number of pieces that the graphic is split into.
    int32 num_pieces;

    //the characters that make up the up and down button graphics.
    buffer16 upchars;
    buffer16 downchars;
    buffer16 disablechars;

    //true if the button has mouse capture.
    bool have_capture;

    //true if the mouse cursor was over this button last frame.
    bool under_cursor;

    //the text string that is drawn on the button.
    buffer256 text;

    //the position that we draw the string.
    int32 text_x, text_y;

    //the state of the button.
    bool is_down;

    //false if the button is disabled.
    bool enabled;

    void Up();
    void Down();

    virtual bool HitTest(int32 norm_x, int32 norm_y);

public:
    //sets the button's enabled/disabled state.
    void Enable(bool enabled);

    bool IsDown();

    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    //a return value of true means this button was pressed by the user.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    virtual void DrawForeground();
    virtual void DrawBackground();
};


class CInterfaceButtonRadio : public CInterfaceButton {
public:
    CInterfaceButtonRadio();

protected:
    //true if we are checked.
    bool checked;

public:
    //sets the check on or off.
    void Check(bool on);

    //positions the button on the screen and resets it's state.
    void Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, const char *text);

    virtual void DrawForeground();
    virtual void DrawBackground();

    virtual void SetPosition(int32 norm_x, int32 norm_y);
};

class CInterfaceInputField : public CInterfaceControl {
public:
    CInterfaceInputField();
    ~CInterfaceInputField();

    //positions the button on the screen and resets it's state.
    void Init(int32 norm_x, int32 norm_y, void *font_normal, void *font_bright, void *font_buttons, char buttonchar);

protected:
    //the position and size info for the button.
    int32 norm_x, norm_y;

    //the width and height.  computed from the width and height of the backgroundchar in the font.
    int32 w, h;

    //the font graphic that holds the graphics for this button.
    void *font_buttons;

    //the fonts that we use to draw the text.
    void *font_normal;
    void *font_bright;

    //the character in the font that the background is stored at.
    char backgroundstring[2];

    //true if the field has mouse capture. (when the user clicked down in the field and hasn't released yet)
    bool have_capture;

    //true if we are editing the text in the field.
    bool editing;

    //when we are editing, we use these vars to keep track of what portion of the string is being displayed
    //and where the insertion bar is displayed.
    int32 insertion_bar_position;
    int32 num_chars_off_left;


    //true if the mouse cursor was over this field last frame.
    bool under_cursor;

    //the text string that is drawn in the field.
    buffer256 text;
#ifdef JPN
	buffer256 imetext;
#endif // JPN

    //called when we want to start editing the text.  the passed number
    //is the number of pixels into the text we want to put the insertion bar.
    void StartEditing(int32 starting_pixel);

    //called whenever the insertion bar moves, allowing the editing vars to be modified so the 
    //point where the bar is located will be within the drawn portion of the text.
    void CheckCursorPos();

public:
    //functions for controling the edit field.
    bool IsEditing();
    void StopEditing();
    const char *GetText();
    void SetText(const char *text);

    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    //a return value of true means the user clicked in the field and wants to edit the text.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual bool Keydown(int32 key);

    virtual void SetPosition(int32 norm_x, int32 norm_y);

#ifdef JPN
	// function for IME control
	void SteImeOpenClose(bool onoff);
	void SetIMECompFormPos();
	void DisplayCompString();
	void SetImeText(LPSTR lpStr);
	bool ImeKeydown(int32 leadbyte, int32 trailbyte);
#endif // JPN
};

class CInterfaceBox : public CInterfaceControl {
public:
    void Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness);

    virtual void PlaceEntities();

protected:
    int32 left, top, right, bottom;
    int32 thickness;
};

class CInterfaceLine : public CInterfaceControl {
public:
    // horizontal
    void Init(int32 left, int32 top, int32 right, int32 thickness, const char *skin_name = "skins/iu_buttonmini.wal");

    // vertical
    void InitV(int32 x, int32 y, int32 y2, int32 thickness, const char *skin_name = "skins/iu_buttonmini.wal");

    virtual void PlaceEntities();
    virtual void PlaceEntitiesV();  // vertical

protected:
    int32 left, top, right, bottom;
    int32 thickness;

    const char *skin_name;
};


class CInterfaceSlider : public CInterfaceControl {
public:
    CInterfaceSlider();

    //sets the position of the slider on the screen.
    void Init(int32 norm_x, int32 norm_y, void *button_font, char backgroundchar, char tabchar);
    //sets the linear scale.
    void InitLinear(int32 steps, float low, float high);
    //sets the exponential scale.
    void InitExp(int32 steps, float log_low, float log_high);

protected:

    //our geometry.
    int32 norm_x, norm_y, w, h;
    //the width of the tab that moves back and forth.
    int32 tabwidth;
    //the left and right extents of the tab's movement.
    int32 left, right;

    //the number of steps allowed in the slider.
    int32 steps;

    //true if we are log.
    bool log_scale;

    //the ranges of the values represented by the slider.
    float low;
    float high;

    //the font that contains the graphics.
    void *button_font;
    //the character positions in the font for the background and tab graphics.
    char backgroundstring[2], tabstring[2];

    //the current position, how many ticks are below and left of the current step.
    int32 position;

    //true if we have the mouse captured.
    bool have_capture;

public:
    virtual void DrawForeground();
    virtual void DrawBackground();

    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    //a return value of true means this button was pressed by the user.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    //returns the current value of the slider.
    float Position();
    //sets the position of the slider.
    void Position(float pos);
};

class CInterfaceScrollbar : public CInterfaceControl {
public:
    CInterfaceScrollbar();

    //sets the position of the slider on the screen.
    void Init(int32 norm_x, int32 norm_y, void *button_font, char backgroundchar, char tabchar);
    //sets the linear scale.
    void InitLinear(int32 steps, float low, float high);
    //sets the exponential scale.
    void InitExp(int32 steps, float log_low, float log_high);

protected:

    //our geometry.
    int32 norm_x, norm_y, w, h;
    //the width of the tab that moves back and forth.
    int32 tabwidth;
    //the top and bottom extents of the tab's movement.
    int32 top, bottom;

    //the number of steps allowed in the slider.
    int32 steps;

    //true if we are log.
    bool log_scale;

    //the ranges of the values represented by the slider.
    float low;
    float high;

    //the font that contains the graphics.
    void *button_font;
    //the character positions in the font for the background and tab graphics.
    char backgroundstring[2], tabstring[2];

    //the current position, how many ticks are below and left of the current step.
    int32 position;

    //true if we have the mouse captured.
    bool have_capture;

public:
    virtual void DrawForeground();
    virtual void DrawBackground();

    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    //a return value of true means this button was pressed by the user.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    //returns the current value of the slider.
    float Position();
    //sets the position of the slider.
    void Position(float pos);
};

//a control with a centered title, and a second line with a left and right
//toenail button to select a string from a list.
class CInterfacePicker : public CInterfaceControl {
public:
    CInterfacePicker();

    //sets the position of the picker.
    void Init(int32 norm_x, int32 norm_y, int32 w, void *menu_font_normal, void *menu_font_bright, void *button_font, const char *title);
    //adds a string into the picker.
    void AddString(const char *string);

    //makes sure the current_string setting is valid for the number of strings we have in our array.
    void CheckCurrentString();

    //resets the string list.
    void ResetStrings();

	// returns the number of strings in the picker
	int StringCount()	{return strings.Num();}

protected:
    //the geometry of the control.
    int32 norm_x, norm_y, w, h;
    int32 toenail_width;

    //we use this object to draw some stuff behind the text.
    CInterfaceLine background;

    //the left and right scroll buttons.
    CInterfaceButton left_button;
    CInterfaceButton right_button;

    //the title.
    buffer256 title;

    //the array of strings that we display.
    array<buffer256> strings;

    //the current string that is shown.
    int32 current_string;

    //true if the mouse is on the picker at all.  The text is drawn bright if it is.
    bool mouse_on_picker;

    //the fonts that we use to draw the text.
    void *font_normal;
    void *font_bright;

    void EnableScrollButtons();

public:
    virtual void DrawForeground();

    virtual void PlaceEntities();

    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    //a return value of true means this button was pressed by the user.
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    //
    //  functions to retrieve the status of the control.
    //
    const char *CurrentString();
    int CurrentIndex();

    void SetCurrentString(int32 num);

};

//implements a scrollable list of same-type controls.

class CInterfaceControlArray : public CInterfaceControl {
public:
    CInterfaceControlArray();
    ~CInterfaceControlArray();

    //specifys the dimensions of the control
    void Init(int32 norm_x, int32 norm_y, int32 width, int32 num_rows, int32 row_height);
    //specifys data needed to draw the scroll buttons.
    void InitScrollButtons(void *menu_font_normal, void *menu_font_bright, void *button_font);

    //adds a control to the array.
    void AddControl(CInterfaceControl *control);

protected:
    //the initialization info for the control.
    int32 norm_x, norm_y, width;
    int32 num_rows;
    int32 row_height;

    //the index of the control that is top in the list.
    int32 top_index;

    //the array of interface control pointers.
    array_obj<CInterfaceControl *> controls;

    //a border.
    CInterfaceBox border;

    //the scroll buttons.
    CInterfaceButton scroll_up;
    CInterfaceButton scroll_down;

	CInterfaceScrollbar scroll_bar;

    //fonts used to draw the scroll buttons.
    void *menu_font_normal;
    void *menu_font_bright;
    void *button_font;

    //moves the visible controls to the correct positions.
    void PositionVisibleControls();

    //sets the up_visible and down_visible vars.
    void ShowScrollButtons();

public:
    //returns true if the given control is currently visible.
    bool ControlVisible(const CInterfaceControl *control);

    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual void DrawForeground();
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
};

//returns the field we are editing, if there is a field that is being edited.
CInterfaceInputField *CurrentlyEditingField();


class CInterfaceScrollThumb : public CInterfaceBox
{
public:
							CInterfaceScrollThumb();
	void					SetPosition(int32 left, int32 top, int32 right, int32 bottom, bool bVertical);
	void					Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, bool bVertical);
    virtual void			PlaceEntities();
	void					Enable(bool bEnable) {enabled = bEnable;}

    virtual bool			MousePos(int32 norm_x, int32 norm_y);
    virtual bool			MouseUp(int32 norm_x, int32 norm_y);
    virtual bool			MouseDown(int32 norm_x, int32 norm_y);

	bool					have_capture;
	bool					under_cursor;
	int32					Top()			{return top;}
	int32					Left()			{return left;}
protected:
	inline bool				HitTest(int32 norm_x, int32 norm_y); 
	bool					vertical;
	bool					enabled;
};

class CInterfaceHighlight : public CInterfaceBox
{
public:
							CInterfaceHighlight()			{color.Set(1,0,0);}
    virtual void			PlaceEntities();
	void					Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, float _alpha = 0.0f);
	void					SetBKColor(CVector &_color)		{color = _color;}

protected:
	float					alpha;
	CVector					color;
};

#define MAX_LINES			5
class CInterfaceConfirmDlg : public CInterfaceBox
{
public:
							CInterfaceConfirmDlg();
	void					Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, int32 ynTop, int32 yLeft, int32 nLeft);
    virtual void			PlaceEntities();
    virtual void			DrawForeground();
	virtual void			DrawBackground();

    virtual bool			MousePos(int32 norm_x, int32 norm_y);
    virtual bool			MouseUp(int32 norm_x, int32 norm_y);
    virtual bool			MouseDown(int32 norm_x, int32 norm_y);
    virtual bool			Keydown(int32 key);

	virtual void			Show(char *_message, char *_prompt);
	virtual void			Hide();
	virtual bool			Result(bool &yes);
	virtual bool			Showing()							{return showing;}
protected:
	int32					bnTop;
	void					ResetPos();
	void					FreeStrings();
	char					*lines[MAX_LINES];
	int						lineCount;
	char					*message1;
	char					*message2;
	char					*prompt;
	bool					bnClicked;
	bool					yesClicked;
	bool					showing;
	CInterfaceButton		yes;
	CInterfaceButton		no;
};


class CInterfaceScroller : public CInterfaceBox
{
public:
	virtual void			Animate(int32 elapsed_time);
    virtual void			PlaceEntities();
    virtual void			DrawForeground();
	virtual void			DrawBackground();

    virtual bool			MousePos(int32 norm_x, int32 norm_y);
    virtual bool			MouseUp(int32 norm_x, int32 norm_y);
    virtual bool			MouseDown(int32 norm_x, int32 norm_y);
    virtual bool			Keydown(int32 key);

	void					Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, bool bVertical);
	void					SetRange(int32 _min, int32 _max, int32 _ticks, int32 _pageSize);
	void					SetScrollPos(int32 pos);
	void					Tick(int32 num);
	int32					CurrentPos()			{return current_position;}
	void					Enable(bool bEnable)	{enabled = bEnable;}
	bool					IsEnabled()				{return enabled;}

protected:
	int32					_left;				// range of the slider box between the buttons
	int32					_top;
	int32					_right;
	int32					_bottom;

	int32					min;				// slider range
	int32					max;
	int32					ticks;
	float					tickSize;
	int32					pageSize;
	int32					current_position;

	float					thumb_frac;
	bool					showThumb;
	bool					vertical;
	bool					enabled;

	CInterfaceButton		scrollMin;
	CInterfaceButton		scrollMax;
	CInterfaceScrollThumb	scrollThumb;

	bool					CheckMoveThumb(int32 norm_x, int32 norm_y);
};


// some storage for our filenames and such.
#define MAX_FILES	100
#define MAX_FIELD	50

typedef struct
{
	char name[MAX_OSPATH];
	char field1[MAX_FIELD];
	char field2[MAX_FIELD];
	char comment[MAX_FIELD];
} config_file_t;

class CInterfaceFileBox;

typedef int	(*verify_t)	(char *name, char *field1, char *field2, char *comment);
typedef void (*refresh_t) (CInterfaceFileBox *self);

class CInterfaceFileBox : public CInterfaceBox
{
public:
							CInterfaceFileBox();
	virtual void			Animate(int32 elapsed_time);
    void					PlaceEntities();
    virtual void			DrawForeground();
	virtual void			DrawBackground();

    virtual bool			MousePos(int32 norm_x, int32 norm_y);
    virtual bool			MouseUp(int32 norm_x, int32 norm_y);
    virtual bool			MouseDown(int32 norm_x, int32 norm_y);
    virtual bool			Keydown(int32 key);


	void					Init(int32 left, int32 top, int32 right, int32 bottom, int32 thickness, char *_path, char *_filter, char *_ignore, int32 _lineHeight, int32 _lines);
	void					SetFields(int32 fnLeft, int32 fnWidth, int32 fdLeft, int32 fdWidth, int32 ftLeft, int32 ftWidth);
	void					Strip(bool bStripPath, bool bStripExt)		{stripPath = bStripPath;stripExt = bStripExt;}

	void					Clear();
	void					Verify(verify_t ver)				{verifyFunc = ver;}
	void					RefreshFunc(refresh_t ref)			{refreshFunc = ref;}
	void					Refresh();																// find the file list
	void					SortBy(short field1, short field2, short field3);						// sort by field1 then field2 then field3
	bool					AddItem(char *name, char *field1, char *field2, char *comment);			// add something to the list
	int32					Selection()	{return selected_file_index;}								// returns index of selected item
	bool					Selection(char *name, char *field1, char *field2, char *comment);		// returns info of selection (true if returned data is valid)
	bool					Select(int32 index, bool bAlways = false);								// selects item # index
	bool					Select(char *name, bool bAlways = false);								// selects item with name
	void					TopIndex(int32 index);													// places item at the top
	int32					Count()								{return total_files;}

	enum					{	SORT_NONE, 
								SORT_NAME_UP,
								SORT_NAME_DN,
								SORT_FIELD1_UP,
								SORT_FIELD1_DN,
								SORT_FIELD2_UP,
								SORT_FIELD2_DN};													// for SortBy!
protected:
	static void				DefRefresh(CInterfaceFileBox *self);									// default refresh func
	refresh_t				refreshFunc;
	verify_t				verifyFunc;																// called to verify a file
	int32					nameLeft;																// character offsets of the fields
	int32					dateLeft;																
	int32					timeLeft;																
																									
	int32					nameWidth;																// character widths of the fields (0 to not draw)
	int32					dateWidth;																
	int32					timeWidth;																
																									
	char					path[MAX_OSPATH];														// the search path
	char					filter[MAX_OSPATH];														// file filter
	char					ignore[MAX_OSPATH];														// ignore some files.
																									
	int32					lineHeight;																// height of a line
	int32					lines;																	// nubmer of displayed lines
																									
	int32					top_file_index;															// first displayed item
	int32					always_highlight_index;													// an item that always has a highlight
	int32					selected_file_index;													// selected item
	int32					old_s_file_index;														// for selection click
	int32					highlighted_file_index;													// for mouseover
	int32					o_h_file_index;															// for mouseover click.
	int32					total_files;															// count of non-null files
	int32					thumb_pos;																// the scroller thumb
																									
	bool					stripPath;																// for cleaning up the displayed info
	bool					stripExt;
	bool					isDateTime;			


	CInterfaceHighlight		selection_box;
	CInterfaceHighlight		static_highlight;
	CInterfaceScroller		scroller;

    CInterfaceBox			file_list_box;

	void					DrawFileLines();
	void					FileLine(int i, bool hilighted, const char *file, const char *date, const char *time );
	int32					HitTest(int32 x, int32 y);
	void					SelectBoxUpdate(CInterfaceHighlight	&box, int32 selIndex, bool sound);
	char					*StripStr(char *str);

	config_file_t			file_list[MAX_FILES];

	// sorting funcs
	int						CompareField(config_file_t &first, config_file_t &second, short field);
	int						Compare(config_file_t &first, config_file_t &second, short field1, short field2, short field3);
	void					QSort(config_file_t ary[], int l, int r, short field1, short field2, short field3);
	int						QSPartition(config_file_t ary[], int l, int r, short field1, short field2, short field3);

	long					sortField1;
	long					sortField2;
	long					sortField3;
};

class CInterfaceStatusText : public CInterfaceBox
{
public:
							CInterfaceStatusText();
	virtual void			Animate(int32 elapsed_time);
    void					PlaceEntities();
    virtual void			DrawForeground();
	virtual void			DrawBackground();

	void					SetText(char *_text, int32 timeS = 999);
protected:
	char					text[255];
	int32					drawMS;
};

#endif // _DK_MENU_CONTROLS_H_