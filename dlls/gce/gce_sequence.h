#pragma once

class CSplineSequence;


//header file for current sequence module

//sets the string that will be displayed at the top of the sequence display screen.
void SequenceHeader(const char *text);
//retrieves the text displayed at the top of the sequence display screen.
const char *SequenceHeader();

//registers a function that is called to restore the mode that initiated the sequence module.
void SequenceDoneFunction(void (*done_func)(const CSplineSequence *spline));

//command handlers.
void SequenceAddPoint();
void SequenceSave();
void SequenceSelectSegment();
void SequenceSelectPoint();
void SequenceTime();
void SequenceHaveStartFOVToggle();
void SequenceHaveEndFOVToggle();
void SequenceStartFOV();
void SequenceEndFOV();
void SequenceHaveStartTimescaleToggle();
void SequenceHaveEndTimescaleToggle();
void SequenceStartTimescale();
void SequenceEndTimescale();
void SequenceHaveStartColorToggle();
void SequenceHaveEndColorToggle();
void SequenceStartColor();
void SequenceEndColor();
void SequenceVelStart();
void SequenceVelEnd();
void SequenceShow();
void SequenceHide();
void SequenceAddEntities();
void SequenceSave();
void SequenceEditPointSave();
void SequenceEditPointAbort();
void SequenceDeletePoint();

void SequenceEditPointLoc();
void SequenceEditPointAngles();

//returns true if we are currently editing a sequence point's location or angles.
bool EditingSequencePoint();

// returns true if we are currently editing a sequence point's location
bool EditingSequencePointLoc();

//returns a pointer to the current sequence.
const CSplineSequence *SequenceCurrent();
//returns the index of the segment that is selected in the current sequence.
int SequenceSelectedSegment();
//returns the index of the point that is selected in the current sequence.
int SequenceSelectedPoint();
//copies the given sequence as the current sequence.
void SequenceSet(const CSplineSequence *sequence);

void SequenceSetCamera(CVector &pos, CVector &angles);

bool SequenceGetFirstPersonEdit();
void SequenceSetFirstPersonEdit(bool);

