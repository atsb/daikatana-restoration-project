#pragma once

typedef void (*PositionUpdateCall)(void);

//sets the strings that will be used as prompts in the various position modes
void PositionPrompt(const char *string);
void PositionPromptAnglesOff(const char *string);
void PositionPromptAnglesYawPitch(const char *string);
void PositionPromptAnglesRoll(const char *string);

//sets the command that is executed when the user wants to save/abort their position
void PositionSaveCommand(const char *command);
void PositionAbortCommand(const char *command);

//gets the prompt...
const char *PositionPrompt();

//gets the location/angles that were saved
const CVector &PositionLoc();
const CVector &PositionAngles();

//gets the start location/angles
const CVector PositionLocStart();
const CVector PositionAnglesStart();

//gets the current location/angles
const CVector PositionLocCurrent();
const CVector PositionAnglesCurrent();

void PositionSetCamera(CVector &viewpos, CVector &viewangles);

void PositionSetFirstPersonEdit(bool);
bool PositionGetFirstPersonEdit();

void PositionCycleMouseAngleControl();
bool PositionGetMouseEnabled();

//
//command handlers.
//
void PositionSave();
void PositionAbort();


// give the position module starting positions and angles, and whether we want this module to update that data
void PositionUpdateObjects(CVector, bool , CVector, bool);

// callback func so the calling module can do whatever updates it needs to do
void PositionUpdateFunc(PositionUpdateCall);

// called once per frame from GCE_HUD()
void PositionUpdate(void);


