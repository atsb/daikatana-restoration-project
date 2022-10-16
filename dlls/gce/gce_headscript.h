#pragma once

class CEntityHeadScript;

//returns true if we are recording a headscript.
bool HeadscriptRecording();

//called once per frame when a script is recording.
void HeadscriptFrame();

//returns the headscript that was recorded.
const CEntityHeadScript &HeadscriptRecorded();

//
//command handlers.
//
void HeadscriptStart();
void HeadscriptSave();
void HeadscriptAbort();





