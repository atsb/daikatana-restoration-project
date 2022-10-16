#pragma once

int InputActive();
int InputKeyEvent(int keydown, int keyup, bool shift_down);

int InputMouseMove(int dx, int dy);

void InputMouseHide();
void InputMouseHideNoMLook();
void InputMouseShow();

bool MouseVisible();

//returns the current coordinates of the mouse cursor
void InputMousePos(int &x, int &y);

//returns the most recent mouse deltas
void InputMouseDeltaPos(int &dx, int &dy);

//draws the mouse cursor at its current position
void InputMouseDraw();

// these functions describe command input (not key input) we're interested in
bool InputForward();
bool InputBack();
bool InputRight();
bool InputLeft();
bool InputUp();
bool InputDown();




