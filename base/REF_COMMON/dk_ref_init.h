#ifndef __DK_REF_INIT_H__
#define __DK_REF_INIT_H__

void PurgeResources( resource_t type );
void BeginRegistration( const char *model );
void Mod_Free( void *mod );
void *RegisterModel( const char *name, resource_t resource );
void *RegisterSkin(	const char *name, resource_t flags );
void *R_GetPicData(	const char *name, int *pWidth, int *pHeight, resource_t resource );
void SetSky ( const char *name, const char *cloudname, int skynum=0);
void R_SetPaletteDir( const char *name );
void R_EndRegistration( void );
void RenderFrame( refdef_t *fd );
void R_DrawPic( DRAWSTRUCT& drawStruct );
int R_DrawString( DRAWSTRUCT& drawStruct );
int R_DrawStringFlare( DRAWSTRUCT& drawStruct );
void R_DrawChar( int x, int y, int num );
void DrawTileClear(	int x, int y, int w, int h, const char *pic );
void Draw_Fill(	int x, int y, int w, int h, CVector rgbColor, float alpha );
void Draw_StretchRaw( int x, int y, int w, int h, int cols, int rows, byte *data );
int R_Init(	void *hinstance, void *hWnd );
void R_Shutdown( void );
void R_CinematicSetPalette ( const unsigned char *palette);
void R_SetInterfacePalette ( const unsigned char *palette);
void BeginFrame( float camera_separation );
void RImp_EndFrame( void );
void RImp_AppActivate( qboolean active );
void R_SetResourceDir( char *name );
void DrawConsolePic( int x, int y, int w, int h, int episode_num, int map_num);
void R_FrameNameForModel( char *modelname, int frameindex, char *framename );
void Mod_RegisterEpisodeSkins( void *mod, int episode, int flags );
void R_GetModelHardpoint( char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt );
void R_GetGlobalColors(	int globalColor, float *r, float *g, float *b );
byte R_BestColor( byte r, byte g, byte b, unsigned *palette );
int R_GetModelSkinIndex( void *modelPtr );
void R_SurfaceSpriteAdd( CVector& vOrigin, CVector& vEntOrigin, CVector& vEntAngles, 
						 void *pHitModel, int nIndex, void *pSpriteModel, int nFrame, 
						 float fRoll, float fScale, byte nFlags );
#endif