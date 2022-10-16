#include "ref.h"
#include "dk_ref_common.h"
#include "dk_ref_init.h"

///////////////////////////////////////////////////////////////////////////////
//
// dk_ref_init.cpp
//
// this file contains renderer-independant functionality that will be shared
// between the renderer dlls (currently ref_gl, ref_soft and ref_d3d)
///////////////////////////////////////////////////////////////////////////////

refexport_t	re;
refimport_t	ri;
memory_import_t memmgr;
char *R_GetModelSkinName (void *modelPtr);

refexport_t GetRefAPI (refimport_t rimp )
{
	ri = rimp;

	memmgr.X_Malloc = ri.X_Malloc;
	memmgr.X_Free = ri.X_Free;

	re.api_version = API_VERSION;

	re.PurgeResources				= PurgeResources;
	re.BeginRegistration			= BeginRegistration;
	re.Mod_Free						= Mod_Free;
	re.RegisterModel				= RegisterModel;
	re.RegisterSkin					= RegisterSkin;
	re.RegisterPic					= R_GetPicData;
	re.SetSky						= SetSky;
	re.SetPaletteDir				= R_SetPaletteDir;
	re.EndRegistration				= R_EndRegistration;
	re.RenderFrame					= RenderFrame;
	re.DrawPic						= R_DrawPic;
	re.DrawChar						= R_DrawChar;
	re.DrawTileClear				= DrawTileClear;
	re.DrawFill						= Draw_Fill;
//	re.DrawStretchRaw				= Draw_StretchRaw;
	re.Init							= R_Init;
	re.Shutdown						= R_Shutdown;
	re.CinematicSetPalette			= R_CinematicSetPalette;
	re.SetInterfacePalette			= R_SetInterfacePalette;
	re.BeginFrame					= BeginFrame;
	re.EndFrame						= RImp_EndFrame;
	re.AppActivate					= RImp_AppActivate;
	re.SetResourceDir				= R_SetResourceDir;
	re.GetFrameName					= R_FrameNameForModel;
	re.RegisterEpisodeSkins			= Mod_RegisterEpisodeSkins;
	re.GetModelHardpoint			= R_GetModelHardpoint;
	re.BestColor					= R_BestColor;
	re.GetModelSkinIndex			= R_GetModelSkinIndex;
	re.GetModelSkinName				= R_GetModelSkinName;
	re.AddSurfaceSprite				= R_SurfaceSpriteAdd;
    re.DrawString					= R_DrawString;
    re.DrawStringFlare				= R_DrawStringFlare;
	re.GetSavegameScreenShot		= GetSavegameScreenShot;
	re.ClearSavegameScreenShot		= ClearSavegameScreenShot;
	re.SaveSavegameScreenShot		= SaveSavegameScreenShot;
	return re;
}

#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ri.Sys_Error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	ri.Con_Printf (PRINT_ALL, "%s", text);
}
#endif
