#ifndef _SED_H
#define _SED_H

#include "katanox.h"
#include "anoxcam.h"
#include "gleditor.h"

#include "p_user.h"
#include "hooks.h"
#include "common.h"

typedef serverState_t STATE;
typedef userEntity_t  USERENT;

#define EDIT_NOTHING  0
#define EDIT_POS      1
#define EDIT_VEL      2
#define EDIT_TIME     3
#define EDIT_PLAY     10
#define EDIT_MARIOVW  12

#define CAMENT_NORMAL       0
#define CAMENT_EDITPOS      1
#define CAMENT_EDITPOS2     2
#define CAMENT_EDITVEL      3
#define CAMENT_EDITVEL2     4
#define CAMENT_EDITTIME     6
#define CAMENT_PLAYBACK     100
#define CAMENT_MARIOTIME    101


extern GLCAMEDIT gl_Editor;
extern JSCRIPT gl_Script;
extern STATE *gl_pState;
extern SCRIPTINFO gl_Info;
extern int gl_iEdit;
extern float gl_fTimeEditScale;
extern int gl_bSedOn, gl_bRestoreCamera;
extern char gl_szLoginName[];
extern char gl_szScriptDir[];

void CameraSendViewAngle( USERENT *Client, const VEC3 &vAng );
void CameraDestroy( USERENT *Self );
void CameraSpawn( USERENT *Target, int iCamType );
void CameraSwitch();


#endif