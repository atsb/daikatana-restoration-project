#pragma once

//draws the hud.
void GCE_HUD();

//switches the hud mode.
void HUDModeSequence();
void HUDModeSounds();
void HUDModeScript();
void HUDModeEntities();
void HUDModePosition();
void HUDModeHeadScript();

//shows/hides the cursor based on the current state of the dll.
void HUDShowCursor();

//turns the hud on/off
void GCE_HUD_On();
void GCE_HUD_Off();
void GCE_HUD_Toggle();

void HUDFieldCycle();
void HUDToggleCursor();
void HUDToggleNodes();

//adds the entities appropriate for our current state.
void AddEntities();

// the currently selected entity
int HUD_GetSelectedEntity();

//returns true if we are editing a field in the hud.
bool HUDEditing();
//returns 1 if the key was handled.
int HUDKeyEvent(int key);

void HUD_MouseDown();

void SetCamera(CVector &viewpos, CVector &viewangles);

//
//drawing primitives
//

//void HUDDrawBox(int left, int top, int right, int bottom, draw_color color);
//void HUDDrawEmptyBox(int left, int top, int right, int bottom, draw_color color);
void HUDDrawBox(int left, int top, int right, int bottom, CVector& rgbColor);
void HUDDrawEmptyBox(int left, int top, int right, int bottom, CVector& rgbColor);
