// screen.h

#define STATUS_INFO_END    ((char *)-1)
#define STATUS_LAYOUT_END  NULL

typedef struct statusInfo_s {
   char *name;                      // name of image to load
   short x,y;                       // image is centered on bottom of screen -- x,y is added to that position
   short level;                     // level of this stat (0 - 5)  (-1 = no levels...just an image)
   short stat;                      // stat value to print            cl.frame.playerstate.stat[]
} statusInfo_t;

typedef struct statusLayout_s {
   statusInfo_t *info;
   short active;
} statusLayout_t;



void	SCR_Init (void);
void  SCR_InitVars(void);

void	SCR_UpdateScreen (void);

void	SCR_SizeUp (void);
void	SCR_SizeDown (void);
void	SCR_CenterPrint (char *str, float msg_time, bool bmotd = false);
void  SCR_SubtitlePrint(char *str, float msg_time);

void	SCR_BeginLoadingPlaque (void);
void	SCR_EndLoadingPlaque (void);

void	SCR_DebugGraph (float value, int color);

void	SCR_RegisterCrosshair (void);
void	SCR_RegisterStatus (void);

void	SCR_RunConsole (void);

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	int			sb_lines;

extern	cvar_t		*scr_viewsize;
extern	cvar_t		*cv_crosshair;
extern	cvar_t		*cv_autoaim;

extern	vrect_t		scr_vrect;		// position of render window

extern qboolean  override_showhuds;


// crosshair types
#define MAX_CROSSHAIRS           4

typedef enum crosshair_types_t{
   // standard types
   CROSSHAIR_NOTARGET,
   CROSSHAIR_TARGET,
   CROSSHAIR_ITEM,

   // special crosshair pics
   CROSSHAIR_CENTER,

   MAX_CROSSHAIR_TYPES
} crosshair_types_t;

typedef struct crosshair_s {
   char pic[MAX_QPATH];
   int width,height;
} crosshair_t;

extern crosshair_t crosshairList[MAX_CROSSHAIRS][MAX_CROSSHAIR_TYPES];
extern crosshair_t *crosshair;

void SCR_AddDirtyPoint (int x, int y);
void SCR_DirtyScreen (void);

qboolean check_weapon_hud(void);
void invmode_hud_setfade(int dir);

//
// scr_cin.c
//
void SCR_PlayCinematic (char *name);
qboolean SCR_DrawCinematic (void);
void SCR_RunCinematic (void);
void SCR_StopCinematic (void);
void SCR_FinishCinematic (void);

