// ==========================================================================
//  File: AudioImp_Aureal3d.cpp
// 
//  Contents    :     
//  Date Created: 
// ==========================================================================

#include <windows.h>
#include "AudioImp_Aureal3d.h"
#include "..\AudioDebug.h"
#include "..\AudioDSDebug.h"
#include "..\AudioDeviceEnum.h"
#include <math.h>
#include "..\eax.h"
#include "..\CReverb.h"
#include "ia3d.h"

// ------------------------------ Types -------------------------------------

// ----------------------------  Globals  -----------------------------------

// ---------------------------- Prototypes ----------------------------------

#define SafeRelease(emitter)    \
    if (emitter) {           \
        emitter->Release ();     \
        emitter = NULL;      \
    }


// =========================== IAudioImp_Aureal3d ==============================
    
IAudioImp_Aureal3d::IAudioImp_Aureal3d ()
	: IAudioImp_DSound3d ()
{
}


IAudioImp_Aureal3d::~IAudioImp_Aureal3d ()
{
}


bool IAudioImp_Aureal3d::createDSoundObjects (LPGUID pGuid, 
                                              HWND hWnd, const IWaveFormat& WaveFormatToUse,
                                              int nNumChannels,     int nNumTrue3dChannels,
                                              bool bUseWin32Events, float fOneUnitInMeters)
{
    HRESULT hr ;

	if (FAILED (hr = A3dCreate (pGuid, (void**)&m_pDS, NULL)))
	{
		ConMsg ("ERROR: Failed to Create Aureal3d renderer (%s)\n", TranslateDSError (hr));
        Shutdown () ;
        return (false);
	}

	switch (hr)
	{
		case A3D_OK_OLD_DLL:
			ConMsg ("NOTE: Older A3d drivers detected.  You should consider downloading new drivers from www.aureal.com") ;
			break;
		case DS_OK:
			ConMsg ("NOTE: A3d Direct Sound failed to initialize--following back to running Microsoft Direct Sound.");
			break;
		default:
			break;
	}

    ConMsg ("Created DSound object...\n") ;
    CHECK (m_pDS) ;


    // -- Set the cooperative level such that we can change the output format...
    //
    if (FAILED (hr = m_pDS->SetCooperativeLevel (hWnd, DSSCL_PRIORITY))) 
	{
    	ConMsg ("\nERROR: Couldn't get PRIORITY cooperative level (%s)\n", TranslateDSError (hr));
        Shutdown () ;
        return (false);
	}
    ConMsg ("Set DSound priority to DSSCL_PRIORITY\n") ;


    // -- Create the primary mixing buffer...
    //
    if (!createPrimarySoundBuffer (nNumTrue3dChannels != 0, &WaveFormatToUse))
    {
        Shutdown () ;
        return (false);
    }
    CHECK (m_pPrimary) ;
    
    // -- Get listener interface.
    //
    if (nNumTrue3dChannels)
    {
        if (FAILED (hr = m_pPrimary->QueryInterface (IID_IDirectSound3DListener, (LPVOID*)&m_p3DListener)))
        {
        	ConMsg ("\nERROR: Failed to get the 3d Listener interface (%s)\n", TranslateDSError (hr));
            Shutdown () ;
            return (false);
        }
    }

    if (m_p3DListener)
    {
        m_p3DListener->SetDistanceFactor (fOneUnitInMeters, DS3D_DEFERRED) ;
        m_p3DListener->CommitDeferredSettings () ;
    }

    // -- Create the mixer object...
    //
	IAudioMixer_DSound3d* pMixer ;
    m_pMixer = pMixer = new IAudioMixer_DSound3d () ;

    // -- Each mixer channel matches the output format...
    const IWaveFormat* pWaveFormat = &WaveFormatToUse ;
    IWaveFormat waveFormat (pWaveFormat->wFormatTag, 
                            1, // nChannels
                            pWaveFormat->nSamplesPerSec,  
                            pWaveFormat->wBitsPerSample) ;

    if (m_bUseWin32Events)
    {
        if (!pMixer->create (*this, &waveFormat, nNumChannels, nNumTrue3dChannels, 
							 m_dataConifgParams.fAudioSubmissionSizeInSecs,
                             &m_hBufferEvents[0], kDS3D_MaxPacketsPerChannel))
        {
            Shutdown () ;
            return (false);
        }
    }
    else
    {
        if (!pMixer->create (*this, &waveFormat, nNumChannels, nNumTrue3dChannels, 
							 m_dataConifgParams.fAudioSubmissionSizeInSecs,
        					 0, 0))
        {
            Shutdown () ;
            return (false);
        }
    }


    // -- If EAX is available and we have True3d channels, setup for using it...
    //
    if (nNumTrue3dChannels)
		initEAX () ;

	return true;
} // createDSoundObjects



void IAudioImp_Aureal3d::destroyDSoundObjects ()
{
}


// ==========================================================================
//                              End of File
// ==========================================================================
