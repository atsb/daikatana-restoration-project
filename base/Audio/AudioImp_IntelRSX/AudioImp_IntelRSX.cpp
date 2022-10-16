// ==========================================================================
//  File: IIntelRSXAudio.cpp
// 
//  Contents    : Interfaces Intel's RSX Audio
//  Date Created: 7/19/98 jas
// ==========================================================================

#include <windows.h>
#include "AudioImp_IntelRSX.h"
#include "..\AudioDebug.h"
#include "..\Audio.h"
#include <math.h>
#include "..\eax.h"

#pragma comment(lib,"uuid.lib")

// MOTHER FUCKERS! I spent a ridicules amount of time trying to fix a couple
// of bugs that turned out to be all related to using CoCreateInstance() which
// Intel RSX guys "highly recommend"; they must not spend much time testing
// there shit.
//#define USE_COCREATE

// ---------------------------- Constants -----------------------------------

// ------------------------------ Types -------------------------------------

// ----------------------------  Globals  -----------------------------------

IAudioEngine* g_pAudioEngine = 0 ;

// ---------------------------- Prototypes ----------------------------------

extern EAX_REVERBPROPERTIES garReverbProperties [] ;  // in AudioEngine.cpp
DWORD WINAPI AudioEngineThreadEntryPoint_IntelRSX (LPVOID pWorkContext)  ;



#define SafeRelease(emitter)    \
    if (emitter) {           \
        emitter->Release ();     \
        emitter = NULL;      \
    }

// ------------------------- IAudioMixer_IntelRSX --------------------------


IAudioMixer_IntelRSX::IAudioMixer_IntelRSX (IAudioEngine& rAudioEngine, int nNumChannels,
                                            const IWaveFormat& waveFormat,
											float fAudioSubmissionSizeInSecs,
                                            HANDLE* parEvents, uint32 uNumEventsPerChannel)
        : m_rAudioEngine (rAudioEngine),
          m_WaveFormat   (waveFormat),
          m_nNumChannels (nNumChannels) 
{
    m_arEvents             = parEvents ;
    m_uNumEventsPerChannel = uNumEventsPerChannel ;
    m_uSubmissionSize      = 0 ;
    m_pFreeBuff            = 0 ;

    for (int i=0; i < kRSX_MaxNumberEvents; i++)
    {
        ZeroMemory(&m_arBufferHdrs [i], sizeof(RSXBUFFERHDR));
        m_arBufferHdrs [i].cbSize = sizeof (RSXBUFFERHDR);
        m_arBufferHdrs [i].dwSize = -1 ; // mark as NOT USED
    }

    for (i = 0; i < kMaxNumberOfChannels; i++)
        m_arChannels [i] = 0 ;

    // -- Calc the submission size which needs to contain whole sample sizes...
    //
    m_uSubmissionSize = fAudioSubmissionSizeInSecs * m_WaveFormat.nAvgBytesPerSec ;
    uint32 uMod = m_uSubmissionSize % m_WaveFormat.nBlockAlign ;
    if (uMod)
        m_uSubmissionSize - uMod ;
    DbgMsg (3, "AE.Mixer submission size=%d\n", m_uSubmissionSize) ;

//11/04
// FIX number of allocations???
	// -- Allocate the mixer cache...
    for (i = 0; i < 4 * nNumChannels; i++)
    {
	    m_arChannelPackets[i].Alloc         (m_uSubmissionSize) ;
	    m_arChannelPackets[i].UseWaveFormat (&m_WaveFormat) ;  // note: pPacket will now delete this
    }
}


IAudioMixer_IntelRSX::~IAudioMixer_IntelRSX ()
{
}
                           

void IAudioMixer_IntelRSX::AssignEmitterToChannel (int nChannelIndex, IAudioEmitter_IntelRSX* pEmitter)
{
    PRECOND (nChannelIndex < kMaxNumberOfChannels) ;
    DbgMsg (3, "=> AssignEmitterToChannel: [%d]  %s\n", nChannelIndex, pEmitter ? pEmitter->GetName () : "0");

    m_arChannels [nChannelIndex] = pEmitter ;
    if (pEmitter)
        pEmitter->m_nNumPacketsOut = 0 ;
}


int IAudioMixer_IntelRSX::findFreeBufferHeader (int nFirstTry)
{
    PRECOND (nFirstTry < kNumberBufferHeaders) ;
    int cnt = kNumberBufferHeaders ;

    while (cnt--)
    {
        if (m_arBufferHdrs [nFirstTry].dwSize == -1)
            return (nFirstTry);
        if (--nFirstTry >= kNumberBufferHeaders)            
            nFirstTry = 0 ;
    }

    DbgMsg (3, "Mixer: FAILED to find a FREE BUFFER!\n") ;
    return (-1);
}



// -- SubmitPacket ---------------------------------------------------------
//
//  Purpose: 
//    Send a packet of data to the mixer.
//
//  Params:
//    nChannel :
//    pEmitterIn  : Audio emitter providing the data
//    nEventIndex : index into IAudioEngine.m_hBufferEvents[] array that contains
//                  the 'hEventSignal'. Useful if the mixer needs to have
//                  a quik way to store unique info on a per event basis.
//  Returns:
//      true if sucessful
//
//  Notes:
//    Called by audio engine to play the emitter in a given channel
//    This is called in response to the Play() method or when a mixing buffer 
//    needs to be refilled (handleFillRequest()).
//
//  One little fucking thing they don't tell you is that you have to have
//  a SEPERATE buffer (ie. can't re-use one you just submitted) for each 
//  buffer submitted!
// --------------------------------------------------------------------------
#if 1 	// MODIFIED VERSION
bool IAudioMixer_IntelRSX::SubmitPacket (IAudioEmitter* pEmitterIn, bool bInitialLoad, int nEventIndex)
{
    PRECOND (pEmitterIn) ;
    DbgMsg (5, "v------------ SubmitPacket ------------\n") ;
    IAudioEmitter_IntelRSX* pEmitter      = (IAudioEmitter_IntelRSX*)pEmitterIn ;
    IAudioStream*           pAudioStream  = pEmitter->m_pAudioStream ;
    int                     nChannelIndex = pEmitter->m_nAssignedToChannel ;

    if (bInitialLoad)
    {
        AssignEmitterToChannel (pEmitter->m_nAssignedToChannel, pEmitter) ;
    }

    uint32 uBytesLeftToCopy = m_uSubmissionSize ;
	if (uBytesLeftToCopy > m_uSubmissionSize)
		uBytesLeftToCopy = m_uSubmissionSize ;

    HANDLE hEventSignal ;
    if (nEventIndex == -1)
        nEventIndex = kEngineEventStartOfChannels + nChannelIndex * kRSX_MaxPacketsPerChannel ;
    DbgMsg (4, "AudioEngine.binding: %d.%d -> '%s'\n", nChannelIndex, nEventIndex, pEmitter->GetName ()) ;

    // -- Copy the data...
    uint32       numBytesRecvd ;

    while (uBytesLeftToCopy)
    {
        // -- find an available buffer header...
        int nBuffIndex = nEventIndex ; //findFreeBufferHeader (2 * nEventIndex) ;

        // -- Since RSX doesn't make a copy of the buffer's header used to submit the
        //    audio data we cannot have more than two outstanding buffers.
        if (pEmitter->m_nNumPacketsOut >= 2)
        {
            DbgMsg (5, "SubmitPacket: having to break out early.\n") ;
            break;
        }

		// -- Wew have to supply RSX with a buffer filled with data.  It'd be MORE
		//	  efficient if THEY would give us a ptr into the sound buffer like we get
		//	  get from Locking a DSound buffer.
		IAudioPacket* pAudioPacket = &m_arChannelPackets [nEventIndex] ;
		if (pAudioPacket->GetNumBytesAllocated () != m_uSubmissionSize)
		{
			pAudioPacket->Alloc (m_uSubmissionSize) ;
			DbgMsg (1, "Mixer: Reallocating audio packet cache for channel=%d\n", nChannelIndex) ;	
		}
		pAudioPacket->UseWaveFormat (pEmitterIn->GetWaveFormat ()) ;

        // -- Get the audio packet from the stream...
        //
		if (!pAudioStream->GetPacketAt (*pAudioPacket, pEmitter, uBytesLeftToCopy))
			return false;

        numBytesRecvd = pAudioPacket->m_uNumBytes ;
        uBytesLeftToCopy -= numBytesRecvd ;

        // -- update emitter state...
        pEmitter->m_uNextStreamOffset += numBytesRecvd ;

        // -- Get an event to use...
        hEventSignal = m_arEvents [nEventIndex] ;

        // -- Get a free buffer to use, and send it to RSX...
        //
        m_pFreeBuff = &m_arBufferHdrs [nBuffIndex] ;
        m_pFreeBuff->dwSize       = pAudioPacket->GetNumBytes () ;
        m_pFreeBuff->lpData       = (PCHAR)pAudioPacket->GetData () ;
        m_pFreeBuff->hEventSignal = hEventSignal ;

        DbgMsg (1, "     AEMixer.Submiting: %d.%d  %dbytes -> '%s'\n",
                nChannelIndex,
                nEventIndex, 
                m_pFreeBuff->dwSize, 
                pEmitter->GetName ()) ;

        HRESULT hr = pEmitter->m_pSE->SubmitBuffer (m_pFreeBuff);
        //CHECK (hr == 0);

        pEmitter->m_nNumPacketsOut++ ;

        // -- Handle EOD...
        //
        if (pAudioStream->GetSizeInBytes () - pEmitter->m_uNextStreamOffset <= 0)
        {     
            CHECK (pEmitter->m_uNextStreamOffset == pAudioStream->GetSizeInBytes ()) ;

            // -- We've obtained all of the data, however, we may be looping.
            //    if we're looping, then reset things and grab the next block...
            //
            if (pEmitter->m_uNumLoops == 1)
            {   // we're done...
                pEmitter->m_bEOD = true ;
                break;
            }
            else
            {
                // -- if num loops is 0, loops infinitely...
                if (pEmitter->m_uNumLoops > 0)
                    pEmitter->m_uNumLoops-- ;

                DbgMsg (5, "  Sound looping %d more times: %s\n", pEmitter->m_uNumLoops, pEmitter->GetName ()) ;
                pEmitter->IAudioEmitter::Rewind () ;
            } 
        } // if 0
 
        // -- we need to load two sumbission size packets when wave is just starting...
        if (bInitialLoad && uBytesLeftToCopy == 0)
        {
            uBytesLeftToCopy = m_uSubmissionSize ;
            bInitialLoad = false;            
        }

        nEventIndex++ ;

    } // while

    DbgMsg (5, "^------------ SubmitPacket ------------\n") ;
    return true;
 }


#endif // MODFIEID

// ------------------------- IAudioEmitter_IntelRSX --------------------------

IAudioEmitter_IntelRSX::IAudioEmitter_IntelRSX (IAudioSpecificImplementation& audioReceiver,
                                                uint32 uAudioAttributes/*=kAFX_Default*/,
                                                bool bStream)
    : IAudioEmitter (audioReceiver, uAudioAttributes, bStream)
{
	m_pSE = 0 ;
    m_nNumPacketsOut = 0 ;
}    


IAudioEmitter_IntelRSX::~IAudioEmitter_IntelRSX ()
{
    GetImp ()->EnterCriticalSection () ;

    // -- make sure we've stopped...
    if (m_pSE)
        m_pSE->Flush () ;
    Stop () ;
    SafeRelease (m_pSE) ;

    GetImp ()->LeaveCriticalSection () ;
}



// -- AttachStream ----------------------------------------------------------
//
//  Purpose: 
//      AttachStream initializes the emitter and attach a stream object to 
//      itself. This should only be called once and is usually called from 
//      IAudioEngine::CreateEmtitter().
//
//  Params:
//      pNewStream : the data stream to get data from
//
//  Returns:
//      true if sucessful
// --------------------------------------------------------------------------

bool IAudioEmitter_IntelRSX::AttachStream (IAudioStream* pNewStream)
{
    PRECOND (!m_pSE) ;      // Can only attach one stream during lifetime of emitter
    if (!IAudioEmitter::AttachStream (pNewStream))
        return (false);

#if 1
    // -- Initialize the emitter...
    //
	RSXSTREAMINGEMITTERDESC seDesc;
	memset(&seDesc, 0, sizeof(RSXSTREAMINGEMITTERDESC));
	seDesc.cbSize  = sizeof(RSXSTREAMINGEMITTERDESC);
    seDesc.dwUser  = (DWORD)this ;

    // -- Use the same wave format that the stream uses...
    //
    const IWaveFormat* pFormat = pNewStream->GetWaveFormat () ;
    seDesc.lpwf = const_cast<IWaveFormat*>(pFormat) ;

    // -- Convert attribute flags to RSX values...
    //
    CHECK (m_bitsDesiredAttributes != kAFX_Default) ; // Default should already have been converted to bit flags

    if (!(m_bitsDesiredAttributes & kAFX_3D))
        seDesc.dwFlags |= RSXEMITTERDESC_NOSPATIALIZE ;

    if (!(m_bitsDesiredAttributes & kAFX_Doppler))
        seDesc.dwFlags |= RSXEMITTERDESC_NODOPPLER ;

    // can't specify RSXEMITTERDESC_NOATTENUATE without also specifiying RSXEMITTERDESC_NOSPATIALIZE
    if (!(m_bitsDesiredAttributes & kAFX_Volume) && !(m_bitsDesiredAttributes & kAFX_3D))
        seDesc.dwFlags |= RSXEMITTERDESC_NOATTENUATE ;

    if (!(m_bitsDesiredAttributes & kAFX_Reverb))
        seDesc.dwFlags |= RSXEMITTERDESC_NOREVERB ;
    // ??? not using kAFX_Pan


//*  // This flag indicates that the emitter will always use the high-quality
//*  // but HRTF-base true 3D algorithm regardless of the global settings
//*  #define RSXEMITTERDESC_USETRUE3D        0x00000010
//*  
//*  // This flag indicates that the emitter will always use the lower-quality
//*  // but higher performance Panning 3D algorithm regardless of the global settings
//*  #define RSXEMITTERDESC_USEPANNING3D     0x00000020

    HRESULT hr ;
#ifdef USE_COCREATE
    // -- Create the RSX streaming emitter...        
    //
	hr = CoCreateInstance (CLSID_RSXSTREAMINGEMITTER,  // GUID for streaming object
                 		   NULL,
                    	   CLSCTX_INPROC_SERVER,
                    	   IID_IRSXStreamingEmitter,
                   		   (void **)&m_pSE);
    if(FAILED(hr)) 
    {
        Audio_ErrorMessage ("ERROR: Failed to CreateStreamingEmitter()\n") ;
        return (false);
    }

    hr = m_pSE->Initialize (&seDesc, GetImp ()->m_pRSXUnknown) ;
#else
    hr = GetImp()->m_pRSX->CreateStreamingEmitter(&seDesc, &m_pSE, NULL);
#endif
    if(FAILED(hr)) 
    {
        Audio_ErrorMessage ("ERROR: Failed to Initialize StreamingEmitter()\n") ;
        return (false);
    }

      SetAttenuationRange (1.f, 2000.f) ;

#endif
    return (true);
}


bool  IAudioEmitter_IntelRSX::Play (uint32 uNumLoops/*=-1*/, float fInitialStartTimeInSecs/*=0*/) 
{
    if (m_pSE) m_pSE->Flush () ;
    if (m_eState == IAudioEmitter::kPlaying)
    {
//          if (m_pSE) m_pSE->Flush () ;
//        GetImp()->onEmitterStop (this) ;
//        // NOTE: If we crash after onEmitterStop(), then the AutoDelete flag for this
//        //       emitter was probably set causing the emitter to get deleted.
//       CHECK (m_pSE) ;
    }

    IAudioEmitter::Play (uNumLoops, fInitialStartTimeInSecs) ;
    PRECOND (m_pSE) ;

    //AudioEngine.play (*this) ;

    return true;
}


bool  IAudioEmitter_IntelRSX::Stop () 
{
    // -- this will inform  AudioEngine that we've finished...
    GetImp()->onEmitterStop (this) ;

    return (true);
}


bool  IAudioEmitter_IntelRSX::Pause () 
{
    IAudioEmitter::Pause () ;
    PRECOND (m_pSE) ;
// TODO:
    return (true);
}

bool  IAudioEmitter_IntelRSX::Resume () 
{
    IAudioEmitter::Resume () ;
    PRECOND (m_pSE) ;
// TODO:
    return (true);
}


bool  IAudioEmitter_IntelRSX::Rewind  () 
{
    IAudioEmitter::Rewind () ;
    //? m_nNumPacketsOut = 0 ;
// TODO:
    return (true);
}


bool  IAudioEmitter_IntelRSX::Mute () 
{
    IAudioEmitter::Mute () ;
    PRECOND (m_pSE) ;

    m_pSE->SetMuteState (TRUE) ;
    return (true);
}

bool  IAudioEmitter_IntelRSX::UnMute () 
{
    IAudioEmitter::UnMute () ;
    PRECOND (m_pSE) ;

    m_pSE->SetMuteState (FALSE) ;
    return (true);
}


//*bool  IAudioEmitter_IntelRSX::Flush () 
//*{
//*    IAudioEmitter::Flush () ;
//*    PRECOND (m_pSE) ;
//*
//*    m_pSE->Flush ();
//*    return (true);
//*}



void  IAudioEmitter_IntelRSX::SetPosition (const AudioPosition_t& ptPosition) 
{
    PRECOND (m_pSE) ;
    IAudioEmitter::SetPosition (ptPosition) ;

	RSXVECTOR3D v3d;
    v3d.x = ptPosition.x ;
    v3d.y = ptPosition.y ;
    v3d.z = ptPosition.z ;

    HRESULT hr = m_pSE->SetPosition (&v3d) ;
    PRECOND (hr == S_OK) ;
}


void  IAudioEmitter_IntelRSX::SetOrientation (const AudioVector_t& vecDirection) 
{
	RSXVECTOR3D v3d;
    v3d.x = vecDirection.x ;
    v3d.y = vecDirection.y ;
    v3d.z = vecDirection.z ;

    IAudioEmitter::SetOrientation (vecDirection) ;
    HRESULT hr = m_pSE->SetOrientation (&v3d) ; //(RSXVECTOR3D*)&vecDirection) ;
    PRECOND (hr == S_OK) ;
}


float IAudioEmitter_IntelRSX::SetPitch (float fPitch) 
{
    float prev = IAudioEmitter::SetPitch (fPitch) ;

    if (fPitch < 0.25f) 
        fPitch = 0.25f ;
    else if (fPitch > 4.f) 
        fPitch = 4.f ;

    HRESULT hr = m_pSE->SetPitch (fPitch) ;
    PRECOND (hr == S_OK) ;
    return prev ;
}


float IAudioEmitter_IntelRSX::SetVolume (float fVolume) 
{
    PRECOND (m_pSE) ;
    if (fVolume == m_fVolume) return (fVolume);

    float prev = IAudioEmitter::SetVolume (fVolume) ;

    RSXEMITTERMODEL rsxModel;
    rsxModel.cbSize     = sizeof (RSXEMITTERMODEL);
    rsxModel.fIntensity = m_fVolume ;
    rsxModel.fMinBack   = m_fInnerRange ;
    rsxModel.fMinFront  = m_fInnerRange ;
    rsxModel.fMaxBack   = m_fOuterRange ;
    rsxModel.fMaxFront  = m_fOuterRange ;

    HRESULT hr = m_pSE->SetModel (&rsxModel);

    PRECOND (hr == S_OK) ;

    return prev ;
}


bool  IAudioEmitter_IntelRSX::SetAttenuationRange (float fMinFront, float fMaxFront)
{
    PRECOND (m_pSE) ;
    IAudioEmitter::SetAttenuationRange (fMinFront, fMaxFront) ;

    RSXEMITTERMODEL rsxModel;
    rsxModel.cbSize     = sizeof (RSXEMITTERMODEL);

	// The intensity (volume) of the emitter, generally between 0.00f and 1.00f
    rsxModel.fIntensity = m_fVolume ;

	// This defines the inner ellipsoid of the emitter. Sound is ambient in this region
    rsxModel.fMinFront  = fMinFront ;
    rsxModel.fMinBack   = fMinFront ;

	// This defines the outer ellipsoid of the emitter Sound is localized 
    // between the edge of the inner ellipsoid and the outer ellipse. If the 
    // listener position is outside of this ellipsoid the emitter can not be heard.
    //
    rsxModel.fMaxFront  = fMaxFront ;
    rsxModel.fMaxBack   = fMaxFront ;

    HRESULT hr = m_pSE->SetModel (&rsxModel);

    PRECOND (hr == S_OK) ;
    return (true);
}



// ----------------------------  IAudioImp_IntelRSX  -----------------------------------

IAudioImp_IntelRSX::IAudioImp_IntelRSX ()
{
    m_bShutdown = true ;
    m_pRSX    = 0 ;
    m_pDL     = 0 ;
    m_pRSXUnknown = 0 ;
    m_nActiveChannels = 0 ;
    m_hThread           = 0 ;

    m_nNumberEvents     = 0 ;
    for (int i=0; i < kRSX_MaxNumberEvents; i++)
        m_hBufferEvents [i] = 0 ;
}


IAudioImp_IntelRSX::~IAudioImp_IntelRSX ()
{
    Shutdown () ;
}


bool IAudioImp_IntelRSX::Initialize (IAudioDeviceCaps* pAudioDev, 
                                     IAudioEngine& audioEngine, 
                                     HWND hWnd, 
                                     const IWaveFormat& WaveFormatToUse, 
                                     int nNumChannels, int nNumTrue3dChannels,
                                     float fOneUnitInMeters,
		                             uint32 uUseFlags,
		                             AudioDataConfigParams_t* pDataConifgParams)
{
    PRECOND (m_pRSX == 0) ;

    IAudioSpecificImplementation::Initialize (pAudioDev, audioEngine, hWnd, WaveFormatToUse, 
							 				  nNumChannels, nNumTrue3dChannels, fOneUnitInMeters, 
											  uUseFlags, pDataConifgParams) ;

    m_nActiveChannels = nNumChannels ;

    ::InitializeCriticalSection (&m_CriticalSection);
    m_bCriticalSectionInited = TRUE;

g_pAudioEngine = &audioEngine ;

    // -- Create Win32 Events...
    //
    if (m_nNumberEvents == 0)
    {
        DbgMsg (5, "Creating Audio events\n") ;

        // -- Create Win32 events for start, pause, stop, and buffer fill 
        //    notifications. Each event is autoreset and not intially signalled.
        //
        for (int i=0; i < kEngineEventStartOfChannels + kRSX_MaxPacketsPerChannel * m_nActiveChannels; i++)
            m_hBufferEvents [i] = ::CreateEvent (NULL, FALSE, FALSE, NULL) ;
        m_nNumberEvents = i ;
    }

    m_pMixer = new IAudioMixer_IntelRSX (audioEngine, nNumChannels, 
                                         WaveFormatToUse,
										 pDataConifgParams->fAudioSubmissionSizeInSecs,
                                         &m_hBufferEvents[0], kRSX_MaxPacketsPerChannel);

    updateRegistry () ;

    HRESULT hr ;
    hr = CoInitialize (NULL) ; // !AGH!!!!! Got to call this
    if (FAILED(hr))
    {
        Audio_ErrorMessage ("Audio: CoInitialize() failed!") ;        
        return (false);
    }
        
	hr = CoCreateInstance (CLSID_RSX20,
                   		 NULL,
                  		 CLSCTX_INPROC_SERVER,
                   		 IID_IUnknown,
                   		 (void ** ) &m_pRSXUnknown);
    if (FAILED (hr) || m_pRSXUnknown == 0)  
    {
        Audio_ErrorMessage ("Intel's RSX sound implementation is either not installed "
                      "or needs to be re-installed. You need to exit the game, "
                      "install RSX and restart.") ;
        return (false);
    }

	hr = CoCreateInstance (CLSID_RSX20,
                    		 NULL,
                    		 CLSCTX_INPROC_SERVER,
                     		 IID_IRSX20,
                     		 (void ** ) &m_pRSX);
    if (FAILED (hr) || m_pRSX == 0)  
    {
        SafeRelease (m_pRSXUnknown) ;
        Audio_ErrorMessage ("Fatal Sound Error: Failed to create RSX Object!\n") ;
        return (false);
    }

	// -- Setup the Default Audio Environment...
    //
	RSXENVIRONMENT rsxEnv;

	rsxEnv.cbSize  = sizeof(RSXENVIRONMENT);
	rsxEnv.dwFlags = RSXENVIRONMENT_COORDINATESYSTEM ;
	rsxEnv.bUseRightHand = FALSE ;
	m_pRSX->SetEnvironment(&rsxEnv);
#if 0
	rsxEnv.dwFlags = RSXENVIRONMENT_COORDINATESYSTEM |
                     RSXENVIRONMENT_SPEEDOFSOUND     |
                     RSXENVIRONMENT_CPUBUDGET ;

	rsxEnv.bUseRightHand =          /* coordinate system */
	rsxEnv.fSpeedOfSound =           /* Speed of Sound for Doppler */
	rsxEnv.fSpeedOfSound = 400.0f;

	m_pRSX->SetEnvironment(&rsxEnv);
#endif

	m_pRSX->GetEnvironment(&rsxEnv);
    DbgMsg (4, "RSX Environ: RH=%d  SpdOfSnd=%f CPUBudget=%d\n", 
            rsxEnv.bUseRightHand, rsxEnv.fSpeedOfSound, rsxEnv.fSpeedOfSound) ;


    // -- Initialize the listener...
    //
    RSXDIRECTLISTENERDESC rsxDL;            // listener description

    rsxDL.cbSize   = sizeof(RSXDIRECTLISTENERDESC);
    rsxDL.hMainWnd = hWnd ;
    rsxDL.dwUser   = 0;
    rsxDL.lpwf     = const_cast<IWaveFormat*>(&WaveFormatToUse) ;

#ifdef USE_COCREATE
    // -- Create a listener and save the IRSXDirectListener interface
    //
    hr = CoCreateInstance (CLSID_RSXDIRECTLISTENER,		// GUID for streaming object
         				   NULL,
            			   CLSCTX_INPROC_SERVER,
            			   IID_IRSXDirectListener,
            			   (void ** )&m_pDL);
    if (FAILED (hr))
    {
        Audio_ErrorMessage ("Fatal Sound Error: Unable to get access to the audio device!\n") ;
        SafeRelease (m_pRSXUnknown) ;
        SafeRelease (m_pRSX) ;
        return (false);
    }

    hr = m_pDL->Initialize (&rsxDL, m_pRSXUnknown) ;
#else
	hr = m_pRSX->CreateDirectListener (&rsxDL, &m_pDL, NULL);
#endif
    if (FAILED (hr))
    {
        Audio_ErrorMessage ("Fatal Sound Error: Unable to initialize the Sound Implementation System!\n") ;
        SafeRelease (m_pRSX) ;
        SafeRelease (m_pDL) ;
        return (false);
    }


	AudioPosition_t v3d        ( 0.f, 0.f, 0.f ) ;
    AudioVector_t   v3dForward ( 0.f, 0.f, 1.f ),
                    v3dUp      ( 0.f, 1.f, 0.f ),
                    v3dRight   ( 1.f, 0.f, 0.f ) ;

    UpdateListenerPosition    (v3d) ;
    UpdateListenerOrientation (v3dForward, v3dUp, v3dRight);

    m_bShutdown = false ;

    return (true);
}    


void IAudioImp_IntelRSX::Shutdown ()
{
    EnterCriticalSection ();

    m_bShutdown = true ;

    // -- Shutdown the thread...
    //
    if (m_hThread)
    {
         DbgMsg (2, "\nTerminating Audio Thread...\n") ;

        // Signal the thread to commit suicide...
        if (m_hBufferEvents [kEngineEventStop])
            SetEvent (m_hBufferEvents [kEngineEventStop]) ;

        // Wait for thread to terminate...
        WaitForSingleObject (m_hThread, INFINITE) ;
        m_hThread = 0 ;
    }

    // -- cleanup the Win32 events...
    //
    if (m_nNumberEvents)
    {
        for (int i=0; i < kEngineEventStartOfChannels + kRSX_MaxPacketsPerChannel * m_nActiveChannels; i++)
        {
            if (m_hBufferEvents [i])
            {
                ::CloseHandle (m_hBufferEvents [i]) ;
                m_hBufferEvents [i] = 0 ;
                m_nNumberEvents-- ;
            }
        }
        CHECK (m_nNumberEvents == 0) ;
    }

    SafeDelete (m_pMixer) ;
    if (m_pRSX)
    {
        SafeRelease (m_pDL) ;
        SafeRelease (m_pRSXUnknown) ;
        SafeRelease (m_pRSX) ;
        CoUninitialize () ;
    }

    LeaveCriticalSection ();

    if (m_bCriticalSectionInited)
    {
        m_bCriticalSectionInited = false ;
        DeleteCriticalSection (&m_CriticalSection);
    }
}


void IAudioImp_IntelRSX::BeginFrame (AudioPosition_t& posListener, AudioVector_t& vecListenerDirection, 
                             AudioVector_t& vecListenerUp, AudioVector_t& vecListenerRight)
{
}


bool IAudioImp_IntelRSX::ChangeOutputFormat (const IWaveFormat& newFormat)
{
    IAudioSpecificImplementation::ChangeOutputFormat (newFormat) ;

    AudioPosition_t ptPosition ;
    AudioVector_t vecForward, vecUp, vecRight ;
// TODO: Remove ref to AudioEngine
g_pAudioEngine->GetListenerOrientation (vecForward, vecUp, vecRight) ;
g_pAudioEngine->GetListenerPosition    (ptPosition) ;

    HRESULT hr ;
    bool  bEnabled;
    float fVolume, fDecayTime, fDamping ;
    g_pAudioEngine->GetReverbParms (bEnabled, fVolume, fDecayTime, fDamping) ;


   SafeRelease (m_pDL) ;

    // -- Create a listener and save the IRSXDirectListener interface
    //
    // -- Initialize the listener...
    //
    RSXDIRECTLISTENERDESC rsxDL;            // listener description

    rsxDL.cbSize   = sizeof(RSXDIRECTLISTENERDESC);
    rsxDL.hMainWnd = m_hWnd ;
    rsxDL.dwUser   = 0;
    rsxDL.lpwf     = const_cast<IWaveFormat*>(&newFormat) ;

#ifdef USE_COCREATE
    hr = CoCreateInstance (CLSID_RSXDIRECTLISTENER,		// GUID for streaming object
         				   NULL,
            			   CLSCTX_INPROC_SERVER,
            			   IID_IRSXDirectListener,
            			   (void ** )&m_pDL);
    if (FAILED (hr))
    {
        Audio_ErrorMessage ("Fatal Sound Error: Unable to get access to the audio device!\n") ;
        return (false);
    }

    hr = m_pDL->Initialize (&rsxDL, m_pRSXUnknown) ;
#else
	hr = m_pRSX->CreateDirectListener (&rsxDL, &m_pDL, NULL);
#endif
    if (FAILED (hr))
    {
        Audio_ErrorMessage ("Fatal Sound Error: Failed to initialize DirectListener Object!\n") ;
        SafeRelease (m_pDL) ;
        return (false);
    }

    UpdateListenerPosition    (ptPosition) ;
    UpdateListenerOrientation (vecForward, vecUp, vecRight);
    SetReverb (bEnabled, fVolume, fDecayTime, fDamping) ;

// TODO:
    // UseHeadphones ();
    return true;
}   


bool IAudioImp_IntelRSX::Connect ()
{
    DbgMsg (3, "Connecting to audio device...\n") ;
    m_pDL->Connect () ;
    return (true);
}    

bool IAudioImp_IntelRSX::DisConnect ()
{
    DbgMsg (3, "DISConnecting from audio device...\n") ;
    m_pDL->Disconnect () ;
    return (true);
}    


void IAudioImp_IntelRSX::SetSpeakerConfiguration (SpeakerConfig_t speakerConfig, float fDegrees) 
{
    IAudioSpecificImplementation::SetSpeakerConfiguration (speakerConfig, fDegrees);
}




void IAudioImp_IntelRSX::SetPrimaryCacheSizeMS (uint32 uSizeInMS)
{
    m_uPrimaryCacheSizeInMS = uSizeInMS ;
    // TODO:
}



void IAudioImp_IntelRSX::SetMasterVolume (float fVol)
{
// TODO:
    // IS THERE NOT A MASTER VOLUME MECH?!
//    if (m_pRSX)
//        m_pRSX->SetVolume (fVol) ;
}


void IAudioImp_IntelRSX::SetReverbPreset (int nPresetID)
{
    EAX_REVERBPROPERTIES* p = &garReverbProperties [nPresetID] ;
    SetReverb (true, p->fVolume, p->fDecayTime_sec, p->fDamping) ;
}


void IAudioImp_IntelRSX::SetReverb (bool bEnable, float fVolume, float fDecayTime, float fReverbDamping)
{
    PRECOND (m_pRSX) ;
    PRECOND (fVolume >= 0.f) ;    
    PRECOND (fDecayTime >= 0.f);
// TEMP!!!    
return;

    // -- RSX's reverb clips badly if fvol gets above 0.3...
    if (fVolume < 0.3f)
        fVolume = 0.3f ;

    //fDecayTime /= 2.f ;
    if (fDecayTime > 3.f)
        fDecayTime = 3.f ;

    DbgMsg (2, "  Using reverb settings for RSX of vol=%f  decayTime=%f\n", fVolume, fDecayTime) ;

	RSXREVERBMODEL rsxRvb;
	rsxRvb.cbSize     = sizeof (RSXREVERBMODEL);
	rsxRvb.bUseReverb = bEnable ;
	rsxRvb.fIntensity = fVolume ;
	rsxRvb.fDecayTime = fDecayTime ;

	HRESULT hr = m_pRSX->SetReverb (&rsxRvb);
    CHECK (SUCCEEDED (hr)) ;
}



bool IAudioImp_IntelRSX::CreateEmitter  (IAudioEmitter** pAudioEmitter,
                                         uint32          nAudioAttributes,
                                         LoadMethod_t    eLoadAs/*=kLoadAllIntoMem*/)
{
    IAudioEmitter_IntelRSX*	pNewEmitter = new IAudioEmitter_IntelRSX (*this, nAudioAttributes, eLoadAs==kLoadForStreaming) ;
    *pAudioEmitter = pNewEmitter ;

    return (true);
}    


void IAudioImp_IntelRSX::DestroyEmitter (IAudioEmitter* pEmitter)
{
    delete pEmitter ;
}


void IAudioImp_IntelRSX::UpdateListenerPosition (const AudioPosition_t& ptPosition)
{
    PRECOND (m_pDL) ;

//DbgMsg (5, "  UpdateListenerPos: %f, %f, %f\n", ptPosition.x, ptPosition.y, ptPosition.z) ;
RSXVECTOR3D pos ;
pos.x = ptPosition.x ;
pos.y = ptPosition.y ;
pos.z = ptPosition.z ;

    // -- Note: RSXVECTOR3d MUST be the same as AudioPosition_t.
	m_pDL->SetPosition (&pos) ; //(RSXVECTOR3D*)&ptPosition);
}


void IAudioImp_IntelRSX::UpdateListenerOrientation (const AudioVector_t& vecDirection, 
                                                            const AudioVector_t& vecUp,
                                                            const AudioVector_t& vecRight)
{
    PRECOND (m_pDL) ;

    // Note: vecRight is not used
RSXVECTOR3D vD, vU;
vD.x = vecDirection.x ;
vD.y = vecDirection.y ;
vD.z = vecDirection.z ;

vU.x = vecUp.x ;
vU.y = vecUp.y ;
vU.z = vecUp.z ;

//vR.x = vecRight.x ;
//vR.y = vecRight.y ;
//vR.z = vecRight.z ;

//vU.x = 0 ;
//vU.y = 1 ;
//vU.z = 0.f;

float angle = acos (vD.x) * (360.f / (2. * 3.1415 )) ;
//*DbgMsg (5, " UpdateOrientation: angle=%f\n", angle) ;

    // -- Note: RSXVECTOR3d MUST be the same as AudioPosition_t.
    //m_pDL->SetOrientation ((RSXVECTOR3D*)&vecDirection, (RSXVECTOR3D*)&vecUp);
    m_pDL->SetOrientation (&vD, &vU) ;
}


void IAudioImp_IntelRSX::onEmitterStop (IAudioEmitter_IntelRSX* pEmitter)
{
    if (pEmitter->GetState() != IAudioEmitter::kPlaying) 
        return ;
    pEmitter->IAudioEmitter::Stop () ;

    // -- if emitter was assigned a mixing channel, clear it from the channel...
    if (pEmitter->GetAssignedChannel() != -1)
        m_pMixer->AssignEmitterToChannel (pEmitter->GetAssignedChannel(), 0) ;

    DbgMsg (3, "onEmitterStop: %s\n", pEmitter->GetName()) ;

    m_lstFinishedEmitters.AddHead (pEmitter) ;
}


static const char szRSXRegKey [] = "SOFTWARE\\Intel\\Realistic Sound Experience\\2.0" ;

bool IAudioImp_IntelRSX::updateRegistry ()
{
    HKEY hAppKey;

    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                       szRSXRegKey,
                                       0,
                                       KEY_READ,
                                       &hAppKey))
    {
        ULONG cbSize = 4 ;
    	DWORD dwType = NULL, dwCount=4;
        DWORD dwError, uDbgLevel, uBuffersize, uTrue3d, uSoftware3d, uUseHeadphones ;
 
        dwError = RegQueryValueEx (hAppKey, "Debug Level", NULL, &dwType, (LPBYTE)&uDbgLevel,      &dwCount);
        // -- get the number of ms of buffer used by the direct listener
        dwError = RegQueryValueEx (hAppKey, "Buffersize",  NULL, &dwType, (LPBYTE)&uBuffersize,    &dwCount);
        dwError = RegQueryValueEx (hAppKey, "Peripheral",  NULL, &dwType, (LPBYTE)&uUseHeadphones, &dwCount);
        dwError = RegQueryValueEx (hAppKey, "True 3D",     NULL, &dwType, (LPBYTE)&uTrue3d,        &dwCount);
        dwError = RegQueryValueEx (hAppKey, "ForceSoftware3D", NULL, &dwType, (LPBYTE)&uSoftware3d,        &dwCount);

        DbgMsg (1, "RSX: DbgLevel=%d  True3d=%d Software3D=%d BufferSz=%d HeadPhones=%d\n",
                uDbgLevel, uTrue3d, uSoftware3d, uBuffersize, uUseHeadphones) ;

//   "Cache Time"
//   "Clip"
//   "Data"
//   "Device Format"
//   "Device Type"
//   "NOMMX"
//   "NumWaveBuffers"
//   "RSXToolPath"
//   "Speaker Help"
//   "Tray Enabled"
//   

        RegCloseKey (hAppKey);

        // If we failed to get the data from the registry we can't return
        // yet, we will proceed to initialize the options with default
        // values
        if (ERROR_SUCCESS == dwError)
        {
            return false;
        }
    }
    return true;
}


int IAudioImp_IntelRSX::PlayControl (PlayCode_t eCode)
{
    IAudioSpecificImplementation::PlayControl (eCode);

    if (eCode == kPlay)
    {
        // -- Create the thread...
        if (!m_hThread)
        {
            if (!createAudioThread ())
                return (false);
        }
    }
    return true;
}


bool IAudioImp_IntelRSX::createAudioThread ()
{
    DWORD   dwThreadID;
    DbgMsg (3, "\nStarting Audio Thread...\n") ;

    m_hThread = ::CreateThread (NULL, 
                                0, 
                                AudioEngineThreadEntryPoint_IntelRSX,
                                this, 
                                0, 
                                &dwThreadID) ;
    if (NULL == m_hThread) 
    {
        ConMsg ("ERROR: Unable to start the Audio Thread!\n") ;        
        return false;
    }

    ::SetThreadPriority (m_hThread, THREAD_PRIORITY_NORMAL) ;
    //THREAD_PRIORITY_ABOVE_NORMAL
    //THREAD_PRIORITY_HIGHEST
    return (true);
}


// -- handleFillRequest -----------------------------------------------------
//
//  Purpose: 
//
//
//  Params:
//
//
//  Returns:
//      true if sucessful
//
// The event array is layed out as:
//
//      0 :  Start                    // kEngineEventStart
//      1 :  Pause                    // kEngineEventPause
//      2 :  Stop                     // kEngineEventStop
//      3 :    not used               
//      4 :  Event 1 for channel 1    // kEngineEventStartOfChannels
//      5 :  Event 2 for channel 1
//      6 :  Event 3 for channel 1
//      7 :  Event 1 for channel 2
//      8 :  Event 2 for channel 2
//      9 :  Event 3 for channel 2
//     ...      ...
//        :  Event 1 for channel n
//        :  Event 2 for channel n
//        :  Event 3 for channel n
//      
// --------------------------------------------------------------------------

void IAudioImp_IntelRSX::handleFillRequest (int nEventNum)
{
    CHECK (nEventNum >= kEngineEventStartOfChannels && nEventNum < m_dataConifgParams.nMaxNumberEvents) ;
    if (m_bShutdown) return;

    // -- get the audio channel and emitter that is using 'nEventNum'...
    int nChannelIndex = (nEventNum - kEngineEventStartOfChannels) / kRSX_MaxPacketsPerChannel ;
    IAudioEmitter_IntelRSX* pEmitter = m_pMixer->m_arChannels [nChannelIndex] ;

    if (pEmitter == 0 || DWORD(pEmitter) == 0xdddddddd) return;  // emitter may have been deleted before last event
    //if (pEmitter->m_pSE == 0) return ;

    DbgMsg (5, "FillReQ: channel %d.%d - %s\n", nChannelIndex, nEventNum, pEmitter->GetName ()) ;

    pEmitter->ReceivedPacket () ;

    // -- May have aborted playback, if so, then we don't want to sumbit any more buffers...    
    if (pEmitter->GetState () != IAudioEmitter::kPlaying)
        return ;
        
    // -- Is the audio done playing...
    //
    if (0 == pEmitter->GetNumOutstandingPackets ()) 
    {
        // finished playing wave; mark channel as vacant and assign a sound 
        // from the priority queue...
        DbgMsg (4, "  Sound finished: [evnt=%d] %s  %s\n", nEventNum, pEmitter->GetName (), pEmitter->IsAutoDelete() ? "(auto deleting)" : "") ;
        m_pMixer->m_arChannels [nChannelIndex] = 0 ;

//9/1        AudioEngine.EmitterFinished (pEmitter) ;
        pEmitter->Stop () ;
    }
    else
    {   // Send next packet of data...

        // -- if there is more audio data to send, then send it...
        if (!pEmitter->IsEOD())
        {
            bool bOk = SubmitPacket (pEmitter, false, nEventNum) ;
        }

    }

#if 0 //def AUDIO_DEBUG
    // -- Debug dump...
    static DWORD dwDumpTime = 0 ;
    if (dwDumpTime + 1000 < timeGetTime())
    {
        dwDumpTime = timeGetTime() ;
        DbgMsg (4, "\n------ Channel State ------\n") ;
        for (int i=0; i < m_nActiveChannels; i++)
        {
            DbgMsg (4, "  [%d] %s\n", 
                     i, 
                    (m_arChannels [i].m_pEmitter ? m_arChannels [i].m_pEmitter->GetName () : "")) ;
        }
        DbgMsg (4, "\n") ;
    }
#endif

} // handleFillRequest



static DWORD WINAPI AudioEngineThreadEntryPoint_IntelRSX (LPVOID pWorkContext) 
{
    DWORD dwReturnedEvent = 0;
    DWORD dwState = 0;
    int i = 0;
    IAudioImp_IntelRSX* pThis = (IAudioImp_IntelRSX*)pWorkContext;
    CHECK (pThis) ;

    while (1) 
    {
        dwReturnedEvent = WaitForMultipleObjects (pThis->m_nNumberEvents,
                                                  pThis->m_hBufferEvents,
                                                  FALSE,
                                                  INFINITE);
        switch (dwReturnedEvent) 
        {

            case kEngineEventStop:
                DbgMsg (4, "Thread: Recvd STOP event\n") ;
                return 0;

            case kEngineEventStart:
            {
                DbgMsg (4, "Thread: Recvd START event\n") ;
                break;
            }

            case kEngineEventPause:
                DbgMsg (4, "Thread: Recvd PAUSE event\n") ;
                break;


            default:
                pThis->EnterCriticalSection () ;

                // This must be a buffer fill request...
                CHECK (dwReturnedEvent >= kEngineEventStartOfChannels && dwReturnedEvent < pThis->m_nNumberEvents) ;

                pThis->handleFillRequest (dwReturnedEvent) ;

                pThis->LeaveCriticalSection () ;
                break;
        } // endswitch
    } // endwhile

    return 0;
}


///////////////////////// PREVIOUS VERSION
#if 0
bool IAudioMixer_IntelRSX::SubmitPacket (IAudioEmitter* pEmitterIn, bool bInitialLoad, int nEventIndex)
{
    PRECOND (pEmitterIn) ;
    DbgMsg (5, "v------------ SubmitPacket ------------\n") ;
    IAudioEmitter_IntelRSX* pEmitter      = (IAudioEmitter_IntelRSX*)pEmitterIn ;
    IAudioStream*           pAudioStream  = pEmitter->m_pAudioStream ;
    int                     nChannelIndex = pEmitter->m_nAssignedToChannel ;

    if (bInitialLoad)
    {
        AssignEmitterToChannel (pEmitter->m_nAssignedToChannel, pEmitter) ;
    }

    uint32 uBytesLeftToCopy = m_uSubmissionSize ;

    HANDLE hEventSignal ;
    if (nEventIndex == -1)
        nEventIndex = kEngineEventStartOfChannels + nChannelIndex * kRSX_MaxPacketsPerChannel ;
    DbgMsg (4, "AudioEngine.binding: %d.%d -> '%s'\n", nChannelIndex, nEventIndex, pEmitter->GetName ()) ;

    // -- Copy the data...
    uint32       numBytesRecvd ;
    IAudioPacket audioPacket ;
    byte*        pOrigData ;

    while (uBytesLeftToCopy)
    {
        // -- find an available buffer header...
        int nBuffIndex = nEventIndex ; //findFreeBufferHeader (2 * nEventIndex) ;
        //if (nBuffIndex == -1)
        //    break;

        // -- Since RSX doesn't make a copy of the buffer's header used to submit the
        //    audio data we cannot have more than two outstanding buffers.
        if (pEmitter->m_nNumPacketsOut >= 2)
        {
            DbgMsg (5, "SubmitPacket: having to break out early.\n") ;
            break;
        }


        // -- Get the audio packet from the stream...
        //
        if (!pAudioStream->GetPacketAt (audioPacket, pEmitter->m_uNextStreamOffset, uBytesLeftToCopy))
            return false;

        pOrigData     = audioPacket.m_pData ;
        numBytesRecvd = audioPacket.m_uNumBytes ;
        uBytesLeftToCopy -= numBytesRecvd ;

        // -- update emitter state...
        pEmitter->m_uNextStreamOffset    += numBytesRecvd ;

        // -- Get an event to use...
        hEventSignal = m_arEvents [nEventIndex] ;

        // -- Get a free buffer to use, and send it to RSX...
        //
        m_pFreeBuff = &m_arBufferHdrs [nBuffIndex] ;

        m_pFreeBuff->dwSize       = audioPacket.GetNumBytes () ;
        m_pFreeBuff->lpData       = (PCHAR)audioPacket.GetData () ;
        m_pFreeBuff->hEventSignal = hEventSignal ;

        DbgMsg (5, "     AEMixer.Submiting: %d.%d  %dbytes -> '%s'\n",
                nChannelIndex,
                nEventIndex, 
                m_pFreeBuff->dwSize, 
                pEmitter->GetName ()) ;

        HRESULT hr = pEmitter->m_pSE->SubmitBuffer (m_pFreeBuff);
        //CHECK (hr == 0);

        pEmitter->m_nNumPacketsOut++ ;

        // -- Handle EOD...
        //
        if (0 == pAudioStream->GetSizeInBytes () - pEmitter->m_uNextStreamOffset)
        {     
            CHECK (pEmitter->m_uNextStreamOffset == pAudioStream->GetSizeInBytes ()) ;

            // -- We've obtained all of the data, however, we may be looping.
            //    if we're looping, then reset things and grab the next block...
            //
            if (pEmitter->m_uNumLoops == 1)
            {   // we're done...
                pEmitter->m_bEOD = true ;
                break;
            }
            else
            {
                // -- if num loops is 0, loops infinitely...
                if (pEmitter->m_uNumLoops > 0)
                    pEmitter->m_uNumLoops-- ;

                DbgMsg (5, "  Sound looping %d more times: %s\n", pEmitter->m_uNumLoops, pEmitter->GetName ()) ;
                pEmitter->IAudioEmitter::Rewind () ;
            } 
        } // if 0
 
        // -- we need to load two sumbission size packets when wave is just starting...
        if (bInitialLoad && uBytesLeftToCopy == 0)
        {
            uBytesLeftToCopy = m_uSubmissionSize ;
            bInitialLoad = false;            
        }

        nEventIndex++ ;

    } // while

    DbgMsg (5, "^------------ SubmitPacket ------------\n") ;
    return true;
 }

#endif
// ==========================================================================
//                              End of File
// ==========================================================================

