// vid.h -- video driver defs

typedef struct vrect_s
{
	int				x,y,width,height;
} vrect_t;

typedef struct
{
	int		width;		
	int		height;
} viddef_t;

extern	viddef_t	viddef;				// global video state

// Video module initialisation etc
void	VID_Init (void);
void	VID_Shutdown (void);
void	VID_CheckChanges (void);

//for setting some of the more complicated refresh settings.
//takes a paramter from 0.5f to 1.3f, with 1.3f being brightest.
void    VID_SetGamma(float gamma);

void	VID_MenuInit( void );
void	VID_MenuDraw( void );
const char *VID_MenuKey( int );

