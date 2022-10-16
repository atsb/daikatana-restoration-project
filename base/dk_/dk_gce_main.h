//#pragma once

#ifndef _DK_GCE_MAIN_H_
#define _DK_GCE_MAIN_H_

void GCE_Init();

void GCE_ClientFrame();

void GCE_SetCamera();

//returns 1 if a cinematic is running.  The view is not neccessisarily being changed
//by the cinematic though.
int CIN_Running();

//returns 1 if a cinematic is running that sets the view.  In this mode, we dont show status
//bars, and the viewport is letterboxed.
int CIN_RunningWithView();

//starts a cinematic.
void CIN_Start(const char *name, int start_shot, int end_shot);

//ends an active cinematic
void CIN_Stop(const char *name);

// SCG[10/2/99]: removes all cinematic entities...
void CIN_RemoveCinematicEntities();

//advances the positions of all active cinematic seqences.
void CIN_Play();

// SCG[8/26/99]: Advances script for playing multiple shots
void CIN_PlayNext();

//gets the location of the camera, 
void CIN_CamLocation(CVector &pos, CVector &angles);
void *CIN_GCE_Interface();
void CIN_WorldPlayback(void *funcs);

//client side drawing.  called at the end of the drawing sequence.
void CIN_GCE_HUD();
//adds entities needed to display the objects we are editing in the script editor.
void CIN_GCE_AddEntities();

//
//functions needed to get input to the dll.
//

//returns 1 if we are editing a field in the dll.
int CIN_GCE_CurrentlyEditing();
//returns 1 if the key was used by the editor.
int CIN_GCE_KeyEvent(int keydown, int keyup, int shift_down);

//returns 1 if the dll used mouse movement information.
int CIN_GCE_MouseMove(int dx, int dy);


#endif // _DK_GCE_MAIN_H_