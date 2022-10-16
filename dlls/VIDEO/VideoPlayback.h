// The interface for connecting to the Video Playback Dll.
//
// NOTE: One file MUST define "#define DEFINE_VIDEO_GLOBALS" before
// 		 including this file to initialize the globals.
//
#ifndef _ION_VIDEO_PLAYBACK
#define _ION_VIDEO_PLAYBACK

#ifdef DEFINE_VIDEO_GLOBALS
#  define IVIDEO_EXTERN
#  define INIT_VIDEO_VAR(a) =a
#else
#  define INIT_VIDEO_VAR(a)
#  define IVIDEO_EXTERN extern 
#endif

typedef  bool (*Video_Init_t)		  (int nWidth, int nHeight, int nBitsPerPixel, LPDIRECTDRAW pDDraw) ;
typedef  bool (*Video_Cleanup_t) 	  () ;
typedef  bool (*Video_Play_t) 		  (const char* pszFilename, VIDEO_TYPE eType) ;

//typedef  bool (*Video_SetVideoMode_t) (int nWidth, int nHeight, int nBitsPerPixel) ;
//typedef  void (*Video_Pause_t) 		  (bool bPause) ;
//typedef  void (*Video_OnActivate_t)   (bool bActivating) ;

IVIDEO_EXTERN Video_Init_t		   Video_Init		  INIT_VIDEO_VAR (0) ;
IVIDEO_EXTERN Video_Cleanup_t 	   Video_Cleanup 	  INIT_VIDEO_VAR (0) ;
IVIDEO_EXTERN Video_Play_t 		   Video_Play 		  INIT_VIDEO_VAR (0) ;

//IVIDEO_EXTERN Video_SetVideoMode_t Video_SetVideoMode INIT_VIDEO_VAR (0) ;
//IVIDEO_EXTERN Video_Pause_t 	   Video_Pause 		  INIT_VIDEO_VAR (0) ;
//IVIDEO_EXTERN Video_OnActivate_t   Video_OnActivate   INIT_VIDEO_VAR (0) ;


//  bool Video_Init () ;
//  bool Video_Cleanup () ;
//  bool Video_SetVideoMode (int nWidth, int nHeight, int nBitsPerPixel) ;
//  void Video_Pause (bool bPause) ;
//  void Video_OnActivate (bool bActivating) ;
//  bool Video_Play (const char* pszFilename, VIDEO_TYPE eType) ;

#endif // _ION_VIDEO_PLAYBACK
