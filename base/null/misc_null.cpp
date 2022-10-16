//////////////////////////////////////////////////////////////////////////////
// misc_null.cpp
//////////////////////////////////////////////////////////////////////////////

#include "p_user.h"

void CIN_CamLocation (CVector &pos, CVector &angles) {};
void* CIN_GCE_Interface (void) {};
void CIN_WorldPlayback (void*) {};

void dk_InitResourcePath (void) {};
void dk_printxy (int x, int y, char *s, ...) {};
void dk_print_clear (void) {};

void _S_ReverbPreset (int nPresetID) {};
void (*S_ReverbPreset) (int nPresetID) = _S_ReverbPreset;
void _S_StartStream (const char *name, float vol) {};
void (*S_StartStream) (const char *name, float vol) = _S_StartStream;

void SCR_BeginLoadingPlaque (void) {};
void SCR_EndLoadingPlaque (void) {};
void SCR_DebugGraph (float f, int i) {};
