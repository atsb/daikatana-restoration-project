// ==========================================================================
//  File: AudioImp_Aureal.h
// 
//  Contents    : Interfaces DSound3d implementation
//  Date Created: 7/19/98 jas
// ==========================================================================

#ifndef _IAUDIOIMP_AUREAL3D
#define _IAUDIOIMP_AUREAL3D

#include "..\Audio.h"
#include "..\AudioPrivate.h"
#include "..\AudioImp_DSound3d\AudioImp_DSound3d.h"
#include "dsound.h"

// ---------------------------- Constants -----------------------------------

// ------------------------------ Types -------------------------------------
//

// ----------------------- IAudioImp_Aureal3d ---------------------------

class CLASS_DECLSPEC IAudioImp_Aureal3d : public IAudioImp_DSound3d
{
  public:
    IAudioImp_Aureal3d () ;
    virtual ~IAudioImp_Aureal3d () ;

  protected:
    virtual bool createDSoundObjects (LPGUID pGuid, HWND hWnd, const IWaveFormat& WaveFormatToUse,
		                              int nNumChannels,     int nNumTrue3dChannels,
		                              bool bUseWin32Events, float fOneUnitInMeters) ;
    virtual void destroyDSoundObjects () ;
}; // IAudioImp_Aureal3d


// ----------------------------  Globals  -----------------------------------

// ---------------------------- Prototypes ----------------------------------


#endif // _IAUDIOIMP_AUREAL3D
// ==========================================================================
//                              End of File
// ==========================================================================
