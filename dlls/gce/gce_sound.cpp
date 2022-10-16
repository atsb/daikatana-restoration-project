#include "p_user.h"
#include "keys.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "gce_main.h"
#include "gce_script.h"
#include "gce_hud.h"
#include "gce_sound.h"

#include "dk_gce_spline.h"
#include "dk_gce_script.h"

#include <ctype.h>

void SoundSort();

#define sound_header() if (cin == NULL || server == NULL) {return;}

//the shot we are currently editing.
static array<CScriptSound> sounds;

//returns an array of sounds.
const array<CScriptSound> &SoundCurrent() {
    return sounds;
}

//copies the array of sounds from the shot to allow them to be edited.
void SoundEdit(const array<CScriptSound> &sounds) {
    //copy the given array.
    ::sounds = sounds;

    SoundSort();
}

int CScriptSound::Compare(const CScriptSound **s1, const CScriptSound **s2) {
    //get the times.
    float t1 = (*s1)->Time();
    float t2 = (*s2)->Time();

    if (t1 < t2) 
        return -1;
    else
        return 1;
}

//sorts the sounds in our array in order of increasing time.
void SoundSort() {
    sounds.Sort(CScriptSound::Compare);
}

void SoundSave() {
    sound_header();

    //send the sounds back to the script module.
    ScriptSoundModified(sounds);
}

void SoundAdd() {
    sound_header();

    CScriptSound *sound = new CScriptSound();

    sounds.Add(sound);

    SoundSort();
}

void SoundDelete() {
    sound_header();

    //check the arguments.
    if (server->GetArgc() != 2) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    int index = atoi(server->GetArgv(1));

    sounds.Remove(index);
}

void SoundSetName() {
    sound_header();

    //check the arguments.
    if (server->GetArgc() != 3) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    //get the index that was given.
    int index = atoi(server->GetArgv(1));

    //get the sound.
    CScriptSound *sound = sounds.Item(index);
    if (sound == NULL) return;

    //set the name.
    sound->Name(server->GetArgv(2));
}

void SoundSetLoop() {
    sound_header();

    //check the arguments.
    if (server->GetArgc() != 3) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    //get the index that was given.
    int index = atoi(server->GetArgv(1));

    //get the sound.
    CScriptSound *sound = sounds.Item(index);
    if (sound == NULL) return;

    //set the loop status.
    bool fLoop = (true?(1==atoi(server->GetArgv(2))):false);
    sound->Loop(fLoop);
}

void SoundSetTime() {
    sound_header();

    //check the arguments.
    if (server->GetArgc() != 3) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    //get the index that was given.
    int index = atoi(server->GetArgv(1));

    //get the sound.
    CScriptSound *sound = sounds.Item(index);
    if (sound == NULL) return;

    //get the time that was given.
    float time = atof(server->GetArgv(2));

    //set the time.
    sound->Time(time);

    SoundSort();
}

void SoundSetChannel() {
    sound_header();

    //check the arguments.
    if (server->GetArgc() != 3) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    //get the index that was given.
    int index = atoi(server->GetArgv(1));

    //get the sound.
    CScriptSound *sound = sounds.Item(index);
    if (sound == NULL) return;

    //get the channel that was given.
    int channel = atoi(server->GetArgv(2));

    //set the time.
    sound->Channel(channel);

    SoundSort();
}







