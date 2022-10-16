#ifndef __DK_REF_COMMON_H__
#define __DK_REF_COMMON_H__

/*
**  This file contains functions used by mulitple ref_ dlls.  
**
*/

// background file names
/*
static char *bkg_rootNames[] = { "2dkcon.pcx", "2dkcon2.pcx",
                                 "5dkcon.pcx", "5dkcon2.pcx",
                                 "6dkcon.pcx", "6dkcon2.pcx",
                                 "7dkcon.pcx", "7dkcon2.pcx"
                               };
*/
//a global pointer to the refresh import function table.
extern refimport_t ri;

//the table that we exported to the exe.
extern refexport_t re;


const char * RandBkgFilename (int fname_idx);  // pluck out filename for random console screen

    
//loads a 8-bit pcx file.  pic is allocated and returned with the appropriate info, and
//the width and height parameters are filled in.  If palette is NULL, the palette will not be loaded,
//otherwise a 768 byte palette will be allocated and the data loaded from the pcx file
void LoadPCX (const char *filename, byte **pic, byte **palette, int *width, int *height);

//loads an 8-bit bmp file.  pic is allocated and returned with the appropriate info, and
//the width and height parameters are filled in.  If palette is NULL, the palette will not be loaded,
//otherwise a 768 byte palette will be allocated and the data loaded from the bmp file
void LoadBMP (const char *filename, byte **pic, byte **palette, int *width, int *height);


//an image we can pass to Draw_CharFlare
extern void *char_flare_image;

//the image that contains the default daikatana mono-spaced font.
extern void *default_chars_image;

//a float that stores the current angle of rotation for lasers.
extern float ref_laserRotation;

//
//functions implemented in dk_ref_font.cpp
//

void	*LoadFont(const char *name);
int		StringWidth(const char *string, void *font, int max_chars_counted);
int		FontHeight(void *font);
int		FontNumCharsInWidth(const char *string, void *font, int allowed_pixel_width, int start_pos);
void	FontGetFontData (void *vfont, int *height, byte **charWidth, byte **charX, byte **charY, void **image);
int     Draw_AlphaChar(int x, int y, int num, int startMsec, int endMsec, int curMsec, int timeOffset, CVector &rgbBlend, int flags, void *vfont);

void	Draw_CharFlare( int x, int y, int w, int h, void *image, CVector &rgbBlend, float alpha, float scale, float theta);
void	PurgeResources(resource_t resource);
void	BeginRegistration (const char *map);
void	SetSky (const char *name, const char *cloudname);
void	R_EndRegistration (void);
void	RenderFrame (refdef_t *fd);
//==================================
// consolidation change: SCG 3-11-99
int		R_DrawString( DRAWSTRUCT& drawStruct );
void	R_DrawPic( DRAWSTRUCT& drawStruct );
void	Draw_Fill (int x, int y, int w, int h, CVector rgbColor, float alpha );
// consolidation change: SCG 3-11-99
//==================================
void	Draw_ConsolePic (int x, int y, int w, int h);
void	Mod_Free (void *mod);
void	R_SetPaletteDir (const char *name);
void	Draw_GetPicSize (int *w, int *h, const char *name, resource_t resource);
void	DrawStretchPic( int destx, int desty, float scale_x, float scale_y, 
					int src_left, int src_top, int src_width, int src_height, void *data, CVector &rgbBlend, float alpha, int paletted );
void	R_SetPaletteDir (const char *name);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);
void	Draw_StringOriented (CVector &origin, CVector &normal, char *str, float scale, int flags);
int		R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );
void	R_SetPalette ( const unsigned char *palette);
void	BeginFrame( float camera_separation );
void	RImp_EndFrame( void );
void	RImp_AppActivate( qboolean active );
void	R_SetResourceDir (char *name);
void	DrawConsolePic (int x, int y, int w, int h, int episode_num, int map_num);
void	R_FrameNameForModel (char *modelname, int frameindex, char *framename);
int		R_SurfIndex (void *mod, char *name);
int		R_VertInfo (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &vert);
int		R_TriVerts (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3);
int		R_TriVerts_Lerp (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3, int oldFrame, float backlerp);
void	Mod_RegisterEpisodeSkins (void *mod, int episode, int flags);
void	R_GetModelHardpoint(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt);
void	R_SetInterfacePalette( const unsigned char *palette );
void	R_GetGlobalColors (int globalColor, float *r, float *g, float *b);
byte	R_BestColor (byte r, byte g, byte b, unsigned *palette);
int		R_GetModelSkinIndex (void *modelPtr);
void	surfSprite_AddSurfaceSprite (CVector &origin, CVector &entOrigin, 
				CVector &entAngles, void *hitModel, int planeIndex, void *model, 
				int frame, float roll, float scale, byte flags);

void GetSavegameScreenShot();
void ClearSavegameScreenShot();
void SaveSavegameScreenShot( char *path, char *savename );
#endif
