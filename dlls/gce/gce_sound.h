#pragma once

#include "dk_array.h"

class CScriptSound;

//returns the array of sounds we are editing.
const array<CScriptSound> &SoundCurrent();

//copies the array of sounds so they can be edited.
void SoundEdit(const array<CScriptSound> &sounds);


void SoundSave();
void SoundAdd();
void SoundDelete();
void SoundSetName();
void SoundSetLoop();
void SoundSetTime();
void SoundSetChannel();



