#define	AREAPORTAL_CLOSED	FALSE
#define	AREAPORTAL_OPEN		TRUE

// func_door et al..

#define DOOR_START_OPEN		1
#define	DOOR_REVERSE		2
#define DOOR_DONT_LINK		4
#define DOOR_TOGGLE			8
#define	DOOR_AUTO_OPEN		16
#define	DOOR_USE_TO_CLOSE	32
#define	DOOR_CONTINUOUS		64
#define DOOR_LOOP_SOUNDS	128
//// SCG[2/4/00]: door struggle is 256...
#define DOOR_FORCEMOVE		512

//NSS[11/28/99]:Strictly for reference...
//	{"flag",         "START_OPEN"},			1
//	{"flag",         "REVERSE"},			2
//	{"flag",         "DOOR_DONT_LINK"},		4
//	{"flag",         "TOGGLE"},				8
//	{"flag",		 "AUTO_OPEN"},			16
//	{"flag",		 "USE_TO_CLOSE"},		32		
//	{"flag",		 "CONTINUOUS"},			64
//	{"flag",		 "LOOP_DOOR_SOUNDS"},	256
//	{"flag",		 "STRUGGLE"},			1024





// func_door_rotate

#define ROTATE_START_OPEN	1
#define ROTATE_REVERSE		2
#define ROTATE_DONT_LINK	4
#define ROTATE_TOGGLE		8
#define ROTATE_AUTO_OPEN	16
#define ROTATE_USE_TO_CLOSE	32
#define ROTATE_CONTINUOUS	64
#define ROTATE_X_AXIS		128
#define ROTATE_Y_AXIS		256
#define ROTATE_SWING		512
#define ROTATE_LOOP_SOUNDS  2048
#define ROTATE_FORCEMOVE	4096

//NSS[11/28/99]: Reference for func_door_rotate
//	{"flag",         "START_OPEN"},			1
//	{"flag",         "REVERSE"},			2
//	{"flag",         "DOOR_DONT_LINK"},		4
//	{"flag",         "TOGGLE"},				8
//	{"flag",		 "AUTO_OPEN"},			16
//	{"flag",		 "USE_TO_CLOSE"},		32
//	{"flag",		 "CONTINUOUS"},			64
//	{"flag",         "X_AXIS"},				128
//	{"flag",         "Y_AXIS"},				256
//	{"flag",		 "SWING"},				512
//	{"flag",		 "STRUGGLE"},			1024 -->defined in Door.cpp
//	{"flag",		 "ROTATE_LOOP_SOUNDS"}, 2048

// func_platform

#define	PLAT_START_UP		1
#define	PLAT_TOGGLE			8

//func_walls

#define	FUNCWALL_TRIGGER_SPAWN	0x01
#define	FUNCWALL_TOGGLE			0x02
#define	FUNCWALL_START_ON		0x04
#define	FUNCWALL_ANIM_ALL		0x08
#define	FUNCWALL_ANIM_FAST		0x10
#define	FUNCWALL_NOT_SOLID		0x20
#define FUNCWALL_CTF			0x40

// func_button

#define	PUSH_TOUCH			1

// func_train

#define	X_AXIS					0x01
#define	Y_AXIS					0x02
#define	Z_AXIS					0x04
#define	WAITFORTRIGGER			0x08
#define MULTI_PATH				0x10
#define TELEPORT				0x20
#define FORCEMOVE				0x40
#define FUNC_TRAIN_START_ON		0x80

#define	STATE_DELAYED		255

#define	CONTINUOUS_OFF			0
#define	CONTINUOUS_ON			1
#define	CONTINUOUS_TRIGGERED	2
#define	CONTINUOUS_STOP_NEXT	3

#define	OFFSET_NONE				0
#define	OFFSET_RESET_TO_ORIGIN	1
#define	OFFSET_SPAWN_ORIGIN		2

///////////////////////////////////////////////////////////////////////////////
// enums
///////////////////////////////////////////////////////////////////////////////

// NOTE: these functions will eventually be moved to common

void func_dynalight_off (userEntity_t *self);
void func_dynalight_on (userEntity_t *self);

void com_FindParent (userEntity_t *self);
void com_SpawnEffect(CVector &pos,float scale,float amount,float spread,float maxvel);
void com_GenerateDustCloud( userEntity_t *self, int count );
void com_PlayEntityScript(userEntity_t *pActivator, char *pCineScript, char *pAiScript);
void com_SpawnEarthQuake( userEntity_t *self, float fRadius, float fScale, float fDamage );
void com_CalcChildPositions (userEntity_t *self);
void com_CalcMoveFinished (userEntity_t *self);
void com_CalcMoveDone (userEntity_t *self);
void com_CalcAbsolutePosition (userEntity_t *self);
void com_CalcMove (userEntity_t *self, const CVector &tdest, float tspeed, think_t func);
void com_CalcAngleMoveDone (userEntity_t *self);
void com_CalcAcceleratedAngleMove (userEntity_t *self);
void com_CalcAngleMove (userEntity_t *self, const CVector &destangle, float tspeed, think_t func);
int com_DoKeyFunctions( userEntity_t *self, userEntity_t *other, char *message );
