// ==========================================================================
//  File: AudioDSound.cpp
// 
//  Contents    :     
//  Date Created: 
// ==========================================================================

#include <windows.h>
#include "AudioImp_DSound3d.h"
#include "..\AudioDebug.h"
#include "..\AudioDSDebug.h"
#include "..\AudioDeviceEnum.h"
#include <math.h>
#include "..\eax.h"
#include "..\CReverb.h"

typedef HRESULT (WINAPI * PFN_DSCREATE)    (LPGUID lpguid, LPDIRECTSOUND * ppDS, IUnknown FAR * pUnkOuter);
typedef BOOL    (WINAPI * PFN_DSENUMERATE) (LPDSENUMCALLBACK lpDSEnumCallback, LPVOID lpContext);


// ---------------------------- Constants -----------------------------------

const float kMixerChannelBufferSizeInSecs = 1.f ;  // 1-sec buffer per mixer channel

// -- DSound uses values for Volume and Pan expressed in dbs scaled by 100.
//    Their min and max pan and vol settings are absurdly large -+100db, so
//    we only want to use a subset range of it.
const int   kPanMaxDb   = 18 ;   // Tweakable
const int   kPanDbScale = 100 ;  // scale db into DSound db
const int   kPanRange   = 2 * kPanMaxDb * kPanDbScale ; 
const int   kPanOffset  = kPanRange / 2 ;

const int   kVolMaxDb   = 25 ; //18 ;   // Tweakable to change range of min-max sound volume
const int   kVolDbScale = 100 ;  // scale db into DSound db
const int   kVolRange   = kVolMaxDb * kVolDbScale ; 
const int   kVolOffset  = kVolRange ;


// ------------------------------ Types -------------------------------------


// ----------------------------  Globals  -----------------------------------

IAudioEngine* g_pAudioEngine = 0 ;

// ---------------------------- Prototypes ----------------------------------

extern EAX_REVERBPROPERTIES garReverbProperties [] ;  // in AudioEngine.cpp

DWORD WINAPI AudioEngineThreadEntryPoint_DSound3d (LPVOID pWorkContext)  ;


#define SafeRelease(emitter)    \
    if (emitter) {           \
        emitter->Release ();     \
        emitter = NULL;      \
    }



inline int32 VolumeToDB (float fVolume)
{
    if (fVolume == 0.f)
        return (DSBVOLUME_MIN);
        
    return fVolume * kVolRange - kVolOffset ;
}    


inline void VecSubtract (AudioVector_t& vecResult, const AudioVector_t& vecA, const AudioVector_t& vecB)
{
    vecResult.x = vecA.x - vecB.x ;
    vecResult.y = vecA.y - vecB.y ;
    vecResult.z = vecA.z - vecB.z ;
}


inline void VecCross (AudioVector_t& vR, const AudioVector_t& vA, const AudioVector_t& vB)
{
	vR.x =  vA.y * vB.z - vA.z * vB.y;
	vR.y =  vA.z * vB.x - vA.x * vB.z ;
	vR.z =  vA.x * vB.y - vA.y * vB.x;
}

inline float VecDotProduct ( const AudioVector_t& vecA, const AudioVector_t& vecB)
{
    return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z) ;
}


inline float VecNormalize (AudioVector_t& vec)
{
    float dist ;

    dist = sqrt (VecDotProduct (vec, vec)) ;
    float recip = 1.f / dist;

    vec.x *= recip ;
    vec.y *= recip ;
    vec.z *= recip ;

    return (dist);
}



// =========================== iDSoundChannel ==============================
    
iDSoundChannel::iDSoundChannel ()
{
    m_nChannelNum      = 0 ;
    m_bPlaying         = false ;
    m_pDSB             = 0 ;
    m_pDSB3d           = 0 ;
    m_pDSNotify        = 0 ;
    m_uNextWriteOffset = 0 ;
    m_uBufferSize      = 0 ;
    m_uSilenceValue    = 0 ;
    m_pEmitter         = 0 ;
    m_uSubmissionSize  = 0 ;
    m_hEventFillReq    = 0 ;
    m_nCurrentFreq     = 0 ;

    for (int i=0; i < kDS3D_MaxPacketsPerChannel; i++)
	{
        m_arNotifyPos [i].dwOffset     = 0 ;
        m_arNotifyPos [i].hEventNotify = 0 ;
	}
}


iDSoundChannel::~iDSoundChannel ()
{
    SafeRelease (m_pDSNotify) ;
    SafeRelease (m_pDSB3d) ;    
    SafeRelease (m_pDSB) ;
}


inline bool iDSoundChannel::WriteSilence (uint32 uStart, uint32 uLength)
{                          
    PRECOND (m_pDSB) ;

    PBYTE   pb1, pb2;
    DWORD   cb1, cb2;

    if  (SUCCEEDED (m_pDSB->Lock (uStart, uLength,  
                                  (LPVOID *) &pb1, &cb1, 
                                  (LPVOID *) &pb2, &cb2, 
                                  0)) )  
    {
        FillMemory (pb1, cb1, m_uSilenceValue) ;

        if  (NULL != pb2 && cb2) 
            FillMemory (pb2, cb2, m_uSilenceValue) ;

        m_pDSB->Unlock (pb1, cb1, pb2, cb2) ;
        return true ;
    }

    return false ;
}



bool iDSoundChannel::Initialize (int nChannelIndex, uint32 uBuffSize, uint32 uSubmissionSizeInBytes,
                                 LPDIRECTSOUNDBUFFER pDSB, LPDIRECTSOUND3DBUFFER pDSB3d)
{
    PRECOND (pDSB && uBuffSize) ;
    HRESULT hr ;
    WAVEFORMATEX  wfx;
    DWORD         dwSizeWritten;

    hr = pDSB->GetFormat (&wfx, sizeof (WAVEFORMATEX), &dwSizeWritten) ;
    CHECK (SUCCEEDED(hr));

    m_nChannelNum      = nChannelIndex ;
    m_pDSB             = pDSB ;
    m_pDSB3d           = pDSB3d ;
    m_uBufferSize      = uBuffSize ;
    m_uNextWriteOffset = 0 ;
    m_uSilenceValue    = wfx.wBitsPerSample == 8  ? 128 : 0 ;
    m_uSubmissionSize  = uSubmissionSizeInBytes ;
    m_nCurrentFreq     = wfx.nSamplesPerSec ;
    m_hEventFillReq    = 0 ;

	return true;
}


bool iDSoundChannel::UseEvents (HANDLE* parEvents, uint32 uNumEventsPerChannel, uint32 uSubmissionSizeInBytes)
{
    CHECK (parEvents && *parEvents) ;
    CHECK (uNumEventsPerChannel <= kDS3D_MaxPacketsPerChannel) ;
    DbgMsg (3, "iDSoundChannel::UseEvents\n") ;

    //int sampleSz = wfx.nChannels * wfx.wBitsPerSample / 8; 
    //CHECK (sampleSz == wfx.nBlockAlign) ;

    HRESULT hr = m_pDSB->QueryInterface (IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify) ;
    if (FAILED (hr))
    {
        ConMsg ("ERROR: QueryInterface for DSNotify failed [%s]\n", TranslateDSError (hr));
        return (false);
    }
    CHECK (m_pDSNotify) ;

    m_hEventFillReq = *parEvents ;

    // -- Note: Event is triggered AFTER Notify Offset bytes.
    for (int i=0; i < uNumEventsPerChannel; i++)
    {
        m_arNotifyPos [i].dwOffset     = (i + 1) * uSubmissionSizeInBytes ;
        m_arNotifyPos [i].hEventNotify = m_hEventFillReq ;
    }

    m_arNotifyPos [i-1].dwOffset -= 1 ;

    hr = m_pDSNotify->SetNotificationPositions (uNumEventsPerChannel, &m_arNotifyPos[0]) ;
    if (FAILED (hr))
    {
        ConMsg ("ERROR: SetNotificationPositions failed [%s]\n", TranslateDSError (hr));
        m_hEventFillReq = 0 ;
        return (false);
    }
	return true;
}



// -- Fill to the next submission boundary with silence...
bool iDSoundChannel::SilentPacket ()
{
    //DbgMsg (5, "    iDSoundChannel writing a silent packet...\n") ;
    uint32 cnt = m_uNextWriteOffset % m_uSubmissionSize ;
    cnt = m_uSubmissionSize - cnt ;
    WriteSilence (m_uNextWriteOffset, cnt) ;

    // -- Advance our internal write cursor, compensating for wraparound.
    //
    m_uNextWriteOffset += cnt ; 
    if (m_uNextWriteOffset >= m_uBufferSize) 
        m_uNextWriteOffset -= m_uBufferSize;

    POSTCOND (0 == m_uNextWriteOffset % m_uSubmissionSize) ;
    return (true);
}    



void iDSoundChannel::Stop ()
{
    DbgMsg (4, "Channel %d: Stoping...\n", m_nChannelNum) ;

//*915    if (m_bPlaying)
    {
        HRESULT hr = m_pDSB->Stop () ;
        CHECK (SUCCEEDED (hr)) ;

        hr = m_pDSB->SetCurrentPosition (0);
        CHECK (SUCCEEDED (hr)) ;

        m_uNextWriteOffset = 0 ;
        m_pEmitter = 0 ;
        m_bPlaying = false ;
    }
}



void iDSoundChannel::AssignEmitter (IAudioEmitter_DSound3d* pEmitter)
{
    // -- stop if we're currently playing an emitter...
//*915    if (m_bPlaying)
        Stop () ;
    PRECOND (!pEmitter || !m_bPlaying) ; 

    if (pEmitter)
    {
        // -- change freq if pEmitter doesn't match the channel's
        if (m_nCurrentFreq != pEmitter->GetWaveFormat()->nSamplesPerSec)
        {
            m_nCurrentFreq = pEmitter->GetWaveFormat()->nSamplesPerSec ;
            DbgMsg (3, "  !!! Changing channel playback rate to: %d Hz\n",m_nCurrentFreq) ;
            m_pDSB->SetFrequency (m_nCurrentFreq) ;
        }

        if (m_pDSB3d)
        {
            HRESULT hr ;
            DWORD dwMode ;
            hr = m_pDSB3d->GetMode (&dwMode) ;
            CHECK (SUCCEEDED (hr)) ;

            if (dwMode == DS3DMODE_DISABLE && pEmitter->Is3D())
            {
                DbgMsg (4, "Enabling DSB3D on buffer\n") ;
                hr = m_pDSB3d->SetMode (DS3DMODE_NORMAL, DS3D_DEFERRED) ;
            }
            else if (dwMode == DS3DMODE_NORMAL && !pEmitter->Is3D())
            {
                DbgMsg (4, "Disabling 3D processing on DSB3D buffer\n") ;
                hr = m_pDSB3d->SetMode (DS3DMODE_DISABLE, DS3D_DEFERRED) ;
            }
            CHECK (SUCCEEDED (hr)) ;
        }
    }
    else
        m_bPlaying = false ;

    m_pEmitter = pEmitter ;
}


// =========================== IAudioMixer_DSound3d ==============================
    
IAudioMixer_DSound3d::IAudioMixer_DSound3d ()
{
    m_nNumChannels       = 16 ;
    m_nNumTrue3dChannels = 0 ;
}
    
IAudioMixer_DSound3d::~IAudioMixer_DSound3d ()
{
    // -- note: m_arChannels[] is an array of iSoundChannel's (ie. not ptrs) 
    //    whose destructors will get called automatically.

    for (int i=0; i < m_nNumChannels; i++)
    {
        if (m_arChannels [i].m_pEmitter)
        {
            m_arChannels [i].m_pEmitter->m_pChannel = 0 ;
        }
    }

}


bool IAudioMixer_DSound3d::create (IAudioImp_DSound3d& audioImp, 
                                   const IWaveFormat* pWaveFormat, 
                                   int nNumChannels, int nNumTrue3dChannels,
								   float fAudioSubmissionSizeInSecs,
                                   HANDLE parEvents[], uint32 uNumEventsPerChannel)
{
    m_nNumTrue3dChannels = nNumTrue3dChannels;
    m_nNumChannels       = nNumChannels ;

    // -- setup a direct sound buffer for each channel...
    //
    LPDIRECTSOUNDBUFFER   pDSB ;
    LPDIRECTSOUND3DBUFFER pDSB3d ;
    uint32                uAttrFlags ;

    // -- We need to make sure that the submission size is an integral number of 
    //    the sample size...
    uint32 uSubmissionSizeInBytes = fAudioSubmissionSizeInSecs * pWaveFormat->nAvgBytesPerSec ;
    uint32 uNumBytes              = kDS3D_MaxPacketsPerChannel * uSubmissionSizeInBytes ;


    DbgMsg (2, "AudioMixer using %d bytes per channel, submissionSz=%d bytes (%f secs)\n", 
    			uNumBytes, uSubmissionSizeInBytes, fAudioSubmissionSizeInSecs) ;
	DbgMsg (2, "   Total bytes: %d\n", uNumBytes * m_nNumChannels) ;

    for (int i=0; i < m_nNumChannels; i++)
    {
        // -- Set the buffer's attribute flags...
        //
        // TODO: Once we convert ALL waves to one sample rate (ie. 22k) then remove the
        //       DSBCAPS_CTRLFREQUENCY flag.
        uAttrFlags = DSBCAPS_CTRLVOLUME  | DSBCAPS_CTRLFREQUENCY | // DSBCAPS_LOCSOFTWARE |
                     DSBCAPS_GETCURRENTPOSITION2 ;
        //  DSBCAPS_LOCHARDWARE | DSBCAPS_GLOBALFOCUS | DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT

        if (parEvents)
            uAttrFlags |= DSBCAPS_CTRLPOSITIONNOTIFY ;

        if (i < nNumTrue3dChannels)
            uAttrFlags |= DSBCAPS_CTRL3D ;
            // -- NOTE: NT and some audio cards do NOT support this...
            //| DSBCAPS_MUTE3DATMAXDISTANCE ;
        else
            uAttrFlags |= DSBCAPS_CTRLPAN ;


        // -- create it...
        pDSB = audioImp.createSecondaryBuffer (pWaveFormat, uNumBytes, uAttrFlags) ;
        if (!pDSB)
        {
            ConMsg ("ERROR: Unable to create mixer!\n") ;
            return (false);
        }

        // -- Get 3d interface...
		pDSB3d = 0 ;
        if (i < nNumTrue3dChannels)
        {
            HRESULT hr = pDSB->QueryInterface (IID_IDirectSound3DBuffer, (LPVOID*)&pDSB3d) ;
            if (FAILED (hr))
            {
                ConMsg ("ERROR: Querying for DSound3d [%s]\n", TranslateDSError (hr));
                SafeRelease (pDSB) ;
                return (false);
            }

            CHECK (pDSB3d) ;
        }


        // -- attach the surface to a iDSoundChannel object...
        //
        if (!m_arChannels[i].Initialize (i, uNumBytes, uSubmissionSizeInBytes, pDSB, pDSB3d))
            return (false);

        if (parEvents)
        {
            int nEventIndex = kEngineEventStartOfChannels + i * uNumEventsPerChannel ;
            if (!m_arChannels[i].UseEvents (&parEvents [nEventIndex], uNumEventsPerChannel, uSubmissionSizeInBytes))
                return (false);
        }
    } // for

    return (true);
}



// -- AssignEmitterToChannel -----------------------------------------------
//
//  Purpose: 
//      Setup the channel for playing an emitter.
//
//  Params:
//
//  Notes:  pEmitter willbe 0 when a channel is being cleared.
// --------------------------------------------------------------------------

void IAudioMixer_DSound3d::AssignEmitterToChannel (int nChannelIndex, IAudioEmitter_DSound3d* pEmitter)
{
    PRECOND (nChannelIndex >= 0 && nChannelIndex < m_nNumChannels) ;
    DbgMsg (4, "=> AssignEmitterToChannel: [%d]  %s\n", nChannelIndex, pEmitter ? pEmitter->GetName () : "0");

    iDSoundChannel* pChannel = &m_arChannels [nChannelIndex] ;

    // -- Let the channel "adapt" itself to the emitter's properties...
    pChannel->AssignEmitter (pEmitter) ;

    // -- Init the IAudioEmitter_DSound vars...
    if (pEmitter)
    {
        pEmitter->m_pChannel       = pChannel ;
        pEmitter->m_nNumPacketsOut = 0 ;
        pEmitter->m_uChangedFlags  = 0xffffffff ; // force update of parms
        pEmitter->m_uLastPlayPos   = 0 ;
    }
}


// -- SubmitPacket ---------------------------------------------------------
//
//  Purpose: 
//    Send a packet of data to the mixer.
//
//  Params:
//    pEmitterIn  : Audio emitter providing the data
//    bInitialLoad: true if this is the first time the emitter is being played
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
//  IMPORTANT: Any calls to this routine MUST call pImp->CriticalSection() first!
// --------------------------------------------------------------------------

bool IAudioMixer_DSound3d::SubmitPacket (IAudioEmitter* pEmitterIn, bool bInitialLoad, int nEventIndex)
{
    PRECOND (pEmitterIn) ;
    //DbgMsg (5, "v------------ SubmitPacket ------------\n") ;
    IAudioEmitter_DSound3d* pEmitter      = (IAudioEmitter_DSound3d*)pEmitterIn ;
    IAudioStream*           pAudioStream  = pEmitter->m_pAudioStream ;
    int                     nChannelIndex = pEmitter->m_nAssignedToChannel ;
    CHECK (nChannelIndex >= 0 && nChannelIndex < m_nNumChannels) ;
    iDSoundChannel*         pChannel      = &m_arChannels [nChannelIndex] ;

	// -- we need to copy one submission size block of data...
    uint32 uBytesLeftToCopy = pChannel->m_uSubmissionSize ;

    // -- first packet submitted for emitter...
    if (bInitialLoad)
    {
        AssignEmitterToChannel (pEmitter->m_nAssignedToChannel, pEmitter) ;
    }

    // -- Lock the buffer section we need to write to...
    //
    HRESULT hr;
    LPBYTE  pbOutput1;
    DWORD   cbOutput1;
    LPBYTE  pbOutput2;
    DWORD   cbOutput2;
    LPDIRECTSOUNDBUFFER pDSB = pChannel->m_pDSB ;
    CHECK (pDSB) ;
    CHECK (pChannel->m_uNextWriteOffset + uBytesLeftToCopy <= pChannel->m_uBufferSize) ;
    
    hr = pDSB->Lock (pChannel->m_uNextWriteOffset,
                     bInitialLoad ? 2 * uBytesLeftToCopy : uBytesLeftToCopy,
                     (LPVOID *) &pbOutput1, &cbOutput1, 
                     (LPVOID *) &pbOutput2, &cbOutput2, 
                     0) ;
    if (FAILED (hr))
    {
        ConMsg ("ERROR: Audio Lock failed [%s]\n", TranslateDSError (hr));
        return (false);
    }

    // -- since we're writing in submission size packets, we should never 
    //    wrap around the DSound buffer.
    CHECK (0 == pChannel->m_uNextWriteOffset % pChannel->m_uSubmissionSize) ;


    // -- Copy the data to the mixing buffer...
    //    note: while loop will copy ONE complete submission sized packet.
    //
    byte*        pBuff = pbOutput1 ;
    uint32       numBytesRecvd,
                 numTotalBytesCopied = 0 ;  // total copied in one submission sized chunk.
    IAudioPacket audioPacket ;

    while (uBytesLeftToCopy)
    {
		CHECK (uBytesLeftToCopy <= pChannel->m_uSubmissionSize) ;

        // -- Get the audio packet from the stream...
        //    Note: streams are not req'd to cache more than kAudioSubmissionSizeInSecs
        //
		audioPacket.UseDataBlock (pBuff, 0, uBytesLeftToCopy, pEmitter->GetWaveFormat()) ;

		if (!pAudioStream->GetPacketAt (audioPacket, pEmitter, uBytesLeftToCopy))
			return false;

        numBytesRecvd        = audioPacket.m_uNumBytes ;
        uBytesLeftToCopy    -= numBytesRecvd ;
        numTotalBytesCopied += numBytesRecvd ;

        // -- update emitter state...
        pEmitter->m_uNextStreamOffset += numBytesRecvd ;

        // -- Copy the data to the channel buffer...
        //
#ifdef AUDIO_DEBUG
        DbgMsg (5, "     AEMixer.Submiting: Chan=%d sz=%d bytes emitterNextPos=%d (%f) -> '%s'\n",
                nChannelIndex,
                numBytesRecvd,
                pEmitter->m_uNextStreamOffset,
                pEmitter->m_fRemainingTimeInSecs,
                pEmitter->GetName ()) ;
#endif
        // -- Write the data to the DSound buffer...
        //
		// -- if we've filled in pbOutput, point to pbOutput2...
        pBuff += numBytesRecvd ;
        if (pBuff > pbOutput1 + cbOutput1)
            pBuff = pbOutput2 ;

        // -- Handle EOD...
        //
        if (pAudioStream->GetSizeInBytes () - pEmitter->m_uNextStreamOffset <= 0)
        {     
            CHECK (pEmitter->m_uNextStreamOffset == pAudioStream->GetSizeInBytes ()) ;
			DbgMsg (5, "\n===>Reached end of wave '%s' ===\n\n", pAudioStream->GetName ()) ;
            
            // -- We've obtained all of the data, however, we may be looping.
            //    if we're looping, then reset things and grab the next block...
            //
            if (pEmitter->m_uNumLoops == 1)
            {   // we're done...
                pEmitter->m_bEOD = true ;

                // -- fill the rest of the submission packet with silence...
                uint32 cnt = pChannel->m_uSubmissionSize - numTotalBytesCopied ;
                DbgMsg (5, "    Writing silence for %d bytes...\n", cnt) ;
                FillMemory (pBuff, cnt, pChannel->m_uSilenceValue) ;

                uBytesLeftToCopy = 0 ;
            }
            else
            {
                // -- if num loops is 0, loops infinitely...
                if (pEmitter->m_uNumLoops > 0)
                    pEmitter->m_uNumLoops-- ;

                DbgMsg (5, "  Sound looping %d more times: %s\n", pEmitter->m_uNumLoops, pEmitter->GetName ()) ;
                pEmitter->IAudioEmitter::Rewind () ;
            } 
        } // if EOD

        if (uBytesLeftToCopy == 0)
        {
            // -- Advance the channel's write cursor, compensating for wraparound.
            pEmitter->m_nNumPacketsOut++ ;
            pChannel->m_uNextWriteOffset += pChannel->m_uSubmissionSize ; 
            if (pChannel->m_uNextWriteOffset >= pChannel->m_uBufferSize) 
                pChannel->m_uNextWriteOffset = 0 ;
            DbgMsg (5, "      Channel: Next buffer offset=%d\n", pChannel->m_uNextWriteOffset) ;

           // -- we need to load two sumbission size packets when wave is just starting...
            if (bInitialLoad)
            {
                if (pEmitter->m_bEOD)
                {   // In this case, all of the wave fit ONE submission sized chuck.
                    // We need to submit a second chunk that is filled with silence.
                    uint32 cnt = pChannel->m_uSubmissionSize ;
                    DbgMsg (5, "    Writing silence for %d bytes...\n", cnt) ;
                    FillMemory (pBuff, cnt, pChannel->m_uSilenceValue) ;
                }
                else
                {
                    uBytesLeftToCopy = pChannel->m_uSubmissionSize ;
                    numTotalBytesCopied = 0 ;
                }
                bInitialLoad = false;            
            }
        }

    } // while

    // -- Unlock the buffer...
    //
    pDSB->Unlock (pbOutput1, cbOutput1, pbOutput2, cbOutput2) ;

    //DbgMsg (5, "^------------ SubmitPacket ------------\n") ;
    return true;
 }



// =========================== IAudioImp_DSound3d ==============================
    
IAudioImp_DSound3d::IAudioImp_DSound3d ()
{
    m_hDSoundLib  = 0 ;
    m_hWnd        = 0 ;
    m_pDS         = 0 ;
    m_pPrimary    = 0 ;
    m_p3DListener = 0 ;
    m_pMixer      = 0 ;
    m_hThread       = 0 ;
    m_nNumberEvents = 0 ;
    m_pEAXReverb  = 0 ;
    m_bUseWin32Events = false ;  // set to true to use thread and Win32 events
    m_bCriticalSectionInited = false ;
    m_fMasterVolume = 1.0f ;

    for (int i=0; i < kDS3D_MaxNumberEvents; i++)
        m_hBufferEvents [i] = 0 ;
}


IAudioImp_DSound3d::~IAudioImp_DSound3d ()
{
}


bool IAudioImp_DSound3d::Initialize (IAudioDeviceCaps* pAudioDevCaps,
                                     IAudioEngine& audioEngine, 
                                     HWND hWnd, 
                                     const IWaveFormat& WaveFormatToUse, 
                                     int nNumChannels, 
                                     int nNumTrue3dChannels,
                                     float fOneUnitInMeters,
		                             uint32 uUseFlags,
		                             AudioDataConfigParams_t* pDataConifgParams)
{
    PRECOND (hWnd) ;
    IAudioSpecificImplementation::Initialize (pAudioDevCaps, audioEngine, hWnd, WaveFormatToUse, 
                                              nNumChannels, nNumTrue3dChannels, fOneUnitInMeters,
											   uUseFlags, pDataConifgParams) ;

    ::InitializeCriticalSection (&m_CriticalSection);
    m_bCriticalSectionInited = true ;

	g_pAudioEngine = &audioEngine ;

    if (m_bUseWin32Events)
    {

        DbgMsg (1, "Using Events and audio thread.\n") ;

        // -- Create Win32 Events...
        //
        if (m_nNumberEvents == 0)
        {
            DbgMsg (2, "Creating Audio events\n") ;

            // -- Create Win32 events for start, pause, stop, and buffer fill 
            //    notifications. Each event is autoreset and not intially signalled.
            //
            for (int i=0; i < kEngineEventStartOfChannels + kDS3D_MaxPacketsPerChannel * nNumChannels; i++)
                m_hBufferEvents [i] = ::CreateEvent (NULL, FALSE, FALSE, NULL) ;
            m_nNumberEvents = i ;
        }
    }
    else
    {
        DbgMsg (1, "Using buffer position as refill method.\n") ;
    }
    

    HRESULT hr ;
    hr = CoInitialize (NULL) ; 
    if (FAILED(hr))
    {
        Audio_ErrorMessage ("Audio: CoInitialize() failed!") ;        
        return (false);
    }
        

    if (!createDSoundObjects (&pAudioDevCaps->m_Guid, hWnd, WaveFormatToUse,
							  nNumChannels, nNumTrue3dChannels, 
							  m_bUseWin32Events, fOneUnitInMeters))
    {
        return (false);
    }

	return true ;
} // Initialize




bool IAudioImp_DSound3d::createDSoundObjects (LPGUID pGuid, 
                                              HWND hWnd, const IWaveFormat& WaveFormatToUse,
                                              int nNumChannels,     int nNumTrue3dChannels,
                                              bool bUseWin32Events, float fOneUnitInMeters)
{
    HRESULT hr ;

    // -- attempt to load the DSound dll...
    //
	m_hDSoundLib = LoadLibrary ("dsound.dll");
	if (m_hDSoundLib == NULL)
	{
		ConMsg ("Could not load DSOUND.DLL");
		return false;
	}

	// Create a directsound object.
    //
	PFN_DSCREATE    pfn_DSCreate;
	pfn_DSCreate = (PFN_DSCREATE)GetProcAddress (m_hDSoundLib, "DirectSoundCreate");
	if (pfn_DSCreate == NULL)
	{
		ConMsg ("\nERROR: DSOUND.DLL does not implement DirectSoundCreate ()\n");
        Shutdown () ;
        return (false);
	}

	if (FAILED (hr = (*pfn_DSCreate) (pGuid, &m_pDS, NULL)))
	{
		ConMsg ("ERROR: Couldn't open DirectSound device (%s)\n", TranslateDSError (hr));
        Shutdown () ;
        return (false);
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
        //m_p3DListener->SetRolloffFactor  (0.01*fOneUnitInMeters, DS3D_DEFERRED) ;
        //m_p3DListener->SetOrientation (, DS3D_DEFERRED) ;
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
    {
		initEAX () ;
    }

	return true;
} // createDSoundObjects



bool IAudioImp_DSound3d::createPrimarySoundBuffer (bool bUse3d, const IWaveFormat* pWaveFormat) 
{
    DSBUFFERDESC    dsbd;
    DbgMsg (2, "Creating Primary Sound buffer: ") ;
    pWaveFormat->Dump () ;

    ZeroMemory (&dsbd, sizeof (DSBUFFERDESC));
    dsbd.dwSize  = sizeof (DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME ;
    if (bUse3d) dsbd.dwFlags |= DSBCAPS_CTRL3D ;

    HRESULT hr  ;
    if (FAILED (hr = m_pDS->CreateSoundBuffer (&dsbd, &m_pPrimary, NULL))) 
	{
    	ConMsg ("Couldn't create primary buffer (%s)", TranslateDSError (hr));
    	return false ;
	}

    CHECK (m_pPrimary) ;

    // Set primary buffer format.
    WAVEFORMATEX* pWvFmt = const_cast<IWaveFormat*>(pWaveFormat) ;

    hr = m_pPrimary->SetFormat (pWvFmt); 
    if (FAILED (hr))
	{
    	ConMsg ("Couldn't set format: %d\n", TranslateDSError (hr));
    	return false ;
	}

    return true ;
}


LPDIRECTSOUNDBUFFER IAudioImp_DSound3d::createSecondaryBuffer (const IWaveFormat* pWaveFormat, uint32 uNumBytes, uint32 uAttrFlags)
{
    HRESULT dsrval;
    LPDIRECTSOUNDBUFFER pDSB ;

    DSBUFFERDESC bufferDesc;
	ZeroMemory (&bufferDesc, sizeof (bufferDesc));
    bufferDesc.dwSize        = sizeof (bufferDesc);
    bufferDesc.dwFlags       = uAttrFlags ;
    bufferDesc.dwBufferBytes = uNumBytes ;
    bufferDesc.lpwfxFormat   = const_cast<IWaveFormat*>(pWaveFormat) ;

    if (FAILED (dsrval = m_pDS->CreateSoundBuffer (&bufferDesc, &pDSB, NULL))) 
	{
    	ConMsg ("Unable to create secondary sound buffer (%s)", TranslateDSError (dsrval));
        return (0);
	}

    return pDSB ;
}



void IAudioImp_DSound3d::destroyDSoundObjects ()
{
    SafeDelete (m_pEAXReverb) ; // this needs to be deleted BEFORE m_pMixer
    SafeDelete (m_pMixer) ;

    // note: releasing the DS will release the other buffers.
    SafeRelease (m_p3DListener) ;
	SafeRelease (m_pPrimary) ;
	if (m_pDS)
	{
	    m_pDS->Release () ;
	    m_pDS = 0 ;
	}

	if (m_hDSoundLib)
	{
		::FreeLibrary (m_hDSoundLib) ;
		m_hDSoundLib = 0 ;
	}
}


void IAudioImp_DSound3d::Shutdown ()
{
    EnterCriticalSection ();

    if (m_bUseWin32Events)
    {
        // -- Shutdown the thread...
        //
        if (m_hThread)
        {
            ConMsg ("\nTerminating Audio Thread...\n") ;

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
            for (int i=0; i < kDS3D_MaxNumberEvents; i++)
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
    }


    destroyDSoundObjects () ;

    LeaveCriticalSection ();

    if (m_bCriticalSectionInited)
    {
        m_bCriticalSectionInited = false ;
        DeleteCriticalSection (&m_CriticalSection);
    }

}


bool IAudioImp_DSound3d::Connect ()
{
    ConMsg ("Connecting to audio device...\n") ;

    if (m_pDS)
        return true ;

    if (!createDSoundObjects (&m_pAudioDev->m_Guid, m_hWnd, m_WaveFormat,
    						  m_nNumChannels, m_nNumTrue3dChannels, 
    						  m_bUseWin32Events, m_fOneUnitInMeters))
        return (false);

    return (true);
}


bool IAudioImp_DSound3d::DisConnect ()
{
    ConMsg ("DISConnecting from audio device...\n") ;

    if (m_pDS)
        destroyDSoundObjects () ;
    return (true);
}


bool IAudioImp_DSound3d::ChangeOutputFormat (const IWaveFormat& newFormat)
{
// TODO:
    // -- delete primary surface and create a new one...

    return (true);
}



bool IAudioImp_DSound3d::CreateEmitter (IAudioEmitter** pAudioEmitter, uint32 nAudioAttributes, 
                                        LoadMethod_t eLoadAs /*=kLoadAllIntoMem*/)
{
    IAudioEmitter_DSound3d*	pNewEmitter = new IAudioEmitter_DSound3d (*this, nAudioAttributes, eLoadAs == kLoadForStreaming) ;
    *pAudioEmitter = pNewEmitter ;

    return (true);
}


void IAudioImp_DSound3d::DestroyEmitter (IAudioEmitter* pEmitter)
{
    delete pEmitter ;
}



bool IAudioImp_DSound3d::restoreAnyLostBuffers ()
{
    DWORD dwStatus ;
    int cnt;

	cnt = 0;

    if (FAILED (m_pPrimary->GetStatus (&dwStatus)))
    {
        ConMsg ("Failed to get status of primary buffer!") ;
        return false ;
    }
    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {
         ConMsg ("=> AudioBuffers lost. Restoring...\n") ;

         if (FAILED (m_pPrimary->Restore ()))
         {
            Audio_ErrorMessage ("Failed to aquire the audio device. Another application "
                                "has probably grabbed. Try exiting all other applictaions "
                                "and try again.") ;
         }
         
         cnt++ ;

        // -- if we lost the primary buffer, then we've probably lost the secondary
        //    ones also...
        for (int i=0; i < m_pMixer->m_nNumChannels; i++)
        {
            iDSoundChannel* pChannel = &m_pMixer->m_arChannels [i] ;
            if (FAILED (pChannel->m_pDSB->GetStatus (&dwStatus)))
            {
                ConMsg ("Failed to get status of secondary buffer!") ;
            }
            else if (dwStatus & DSBSTATUS_BUFFERLOST)
            {
                pChannel->m_pDSB->Restore () ;

                // -- TODO: SubmitBuffer ()
                cnt++ ;
            }
        }

        DbgMsg (3, "   Restored %d buffers.\n", cnt) ;
    }

    return true ;
}


float gain (float thetaLis, float thetaSrc, float fSpeakerTheta, float fSrcLisLen, float fSpeakerDist)
{
    static float fRightAngle ;
    float fDiff ;
    static bool bFirst = true ;

    if (bFirst)
    {
        bFirst = false ;
        fRightAngle = 2.f * atan (1.f) ;
    }

    float fThetaDiff = thetaSrc - thetaLis ;
    DbgMsg (5, "thetaSrc=%f   thetaLis=%f   diff=%f\n", RadToDeg (thetaSrc), RadToDeg (thetaLis), RadToDeg(fSpeakerTheta - fThetaDiff)) ;
    fDiff = fabs (fSpeakerTheta - fThetaDiff) ;
    if (fDiff > fRightAngle)
        return (0.f);
    return cos (fDiff) ; //* fSpeakerDist / fSrcLisLen ;
}



inline bool IsPtInCone (float fAngleDegs, float fS, float fE, float& fDiff)
{
    // -- Case when not straddling 0 degrees...
    if (fS < fE)
    {
        if (fS >= fAngleDegs && fE <= fAngleDegs)
            return (true);
        else
            return (false);
    }
    else // We're straddling 0 degs...
    {
        if (fAngleDegs >= 0.f)
            return (fAngleDegs < fE);
        else
            return (fAngleDegs > fS);
    }
}

// -- CalcSpacialization ----------------------------------------------------
//
//  Purpose: 
//     Calculate the values for Volume and Panning for a software emitter.
//
//  Params:
//
//
//  Returns:
//      true if sucessful
//
//  Notes: 
//      Doesn't take into account the master volume since it is usually
//      controlled by the primary buffer.
// --------------------------------------------------------------------------

inline bool IAudioImp_DSound3d::CalcSpacialization (IAudioEmitter_DSound3d* pEmitter, 
                                                    AudioPosition_t&  posListener, 
                                                    AudioVector_t&    vecListenerForward, 
                                                    AudioVector_t&    vecListenerUp, 
                                                    AudioVector_t&    vecListenerRight,
                                                    int32& nVolDBOut, int32& nPanDBOut)
{
    AudioVector_t   vecSrc ;
    AudioPosition_t posSrc ;
    float           fLeftVol, fRightVol ;
    float           fMinRange, fMaxRange ;

    // -- early out if a 2d sound or we're mono out...
    if (!pEmitter->Is3D() || m_WaveFormat.nChannels == 1)
    {
        DbgMsg (4, "  Emitter is 2d and not attenuated -> full volume, no pan\n") ;

        nVolDBOut  = DSBVOLUME_MAX ;
        fLeftVol   = 0.5f ;
        fRightVol  = 0.5f ;
    }
    else
    {
        // -- compute the distance attenuation...
        //
        pEmitter->GetPosition (posSrc) ;
        pEmitter->GetAttenuationRange (fMinRange, fMaxRange) ;

        VecSubtract (vecSrc, posSrc, posListener) ;
		AudioVector_t vecSrcPlayer (vecSrc);
        float fDistAtten = VecNormalize (vecSrc) - fMinRange ;

        if (fDistAtten < 0.f)
        {
            DbgMsg (4, "      Emitter inside of min attenuation range -> full volume\n") ;
			nVolDBOut = VolumeToDB (pEmitter->GetVolume()) ;
        }
        // -- if outside max threshold, then it is not audible...
        else if (fDistAtten >= fMaxRange - fMinRange)
        {
            DbgMsg (4, "      Emitter is outside of max attenuation range: fDistAtten=%f.  SrcPos=%f, %f, %f\n", 
                    fDistAtten + fMinRange, posSrc.x, posSrc.y, posSrc.z) ;
            nVolDBOut = DSBVOLUME_MIN ;
        }
        else
        {   // we're somewhere within the attenuation region.

            // -- Calc output attenuated intensity...
			int32 nDeltaDB = VolumeToDB (pEmitter->GetVolume()) ;   // inner region is 0 db attenuated
            float fBandDist = fMinRange ; // starting band
            fDistAtten     += fMinRange ; // add back in

            while (1)
            {
                fBandDist *= 2 ;

                // -- are we in the band that the emitter is in...
                if (fBandDist > fDistAtten)
                {
                    // -- linearly interpolate the db in this last band...
                    nDeltaDB -= (fDistAtten - fBandDist / 2.f) / (fBandDist - fBandDist / 2.f) ;
                    break;
                }

                nDeltaDB  -= 600 ; // 6 dB of attenuation
            } // while


            nVolDBOut = nDeltaDB ;
		} // else

        // -- Take into account if the sound emitter is directional...
        //
        if (nVolDBOut != DSBVOLUME_MIN)
        {
            if (pEmitter->HasCone ())
            {
                AudioVector_t vecOrientation ;
                pEmitter->GetOrientation (vecOrientation) ;
VecNormalize (vecOrientation) ;

				VecNormalize (vecSrcPlayer) ;
                float fCosAngle  = VecDotProduct (vecSrcPlayer, vecOrientation) ;
                float fAngleDegs = 180.f * acos (fCosAngle) / 3.1415f ;

                uint32 uInsideAngleInDegs, uOutsideAngleInDegs ;
                float  fConeOutsideVolumeInDB ;
                pEmitter->GetConeParameters (uInsideAngleInDegs, uOutsideAngleInDegs, fConeOutsideVolumeInDB) ;

                DbgMsg (1, "angle between cone orientation and player=%f (%f degs)  %f  %f\n", 
                        fCosAngle, fCosAngle * 180.f/3.1415f, cos (uInsideAngleInDegs), cos (uOutsideAngleInDegs)) ;

                if (fCosAngle >= cos (uOutsideAngleInDegs))
                {
                    // -- Player is outside of the cone; play at attenuation of outside cone....
                    nVolDBOut = DSBVOLUME_MIN ;
                }
                else if (fCosAngle <= cos (uInsideAngleInDegs))
                {
                    // -- When inside the inner cone, we don't attenuate anymore
                    DbgMsg (1, "Inside of CONE!\n") ;
                }
                else
                {   // Player is BETWEEN the inside and outside cones so we need
                    // to linear interpolate to get the amount of attenuation...
                    DbgMsg (1, "Inside of OUTSIDE CONE!\n") ;
					// TODO:
                }
            } // HasCone
        }

        // -- Calc the intensity panning for right and left channels...
        float fCosTheta = VecDotProduct (vecSrc, vecListenerRight) ;
        fLeftVol  = 0.5f * (1.f - fCosTheta) ;
        fRightVol = 0.5f * (1.f + fCosTheta) ;
     } // else a 3d sound

    // -- convert to pan to db for DSound...
    nPanDBOut = (fRightVol * kPanRange) - kPanOffset ;

    DbgMsg (4, "    VolDb=%d  PanDb=%d\n", nVolDBOut, nPanDBOut) ;

    return true;
} // CalcSpacialization



// -- BeginFrame ------------------------------------------------------------
//
//  Purpose: 
//    The AudioEngine calls this routine once per frame to process audio tasks.
//
//  Params:
//    The listener's position and orientation.
// --------------------------------------------------------------------------

void IAudioImp_DSound3d::BeginFrame (AudioPosition_t& posListener, AudioVector_t& vecListenerForward, 
                                     AudioVector_t& vecListenerUp, AudioVector_t& vecListenerRight)
{
    if (!m_pDS) return;

    iDSoundChannel* pChannel ;
    IAudioEmitter_DSound3d* pChEmitter ;
    LPDIRECTSOUND3DBUFFER pDSB3d ;
    LPDIRECTSOUNDBUFFER   pDSB ;
    AudioPosition_t pt ;
    uint32 uFlags ;
    int i ;
    HRESULT hr ;

    // -- If not using thread, then we need to check and fill any buffers...
    //
    if (!m_bUseWin32Events)
        refillBuffers () ;


    // -- TODO: Throttle number of calls to, say, 20 Hz...

#ifdef AUDIO_DEBUG
    // temp! to not show debug strings unless there's something to see
    int nChannelsPlaying=0;
    for (int q=0; q < m_pMixer->m_nNumChannels; q++)
    {
        if (m_pMixer->m_arChannels [q].m_pEmitter)
            nChannelsPlaying++ ;
    }
    if (!nChannelsPlaying) return;
#endif

    DbgMsg (5, "==>BeginFrame...\n") ;

    // -- Restore the primary buffer it if was lost...
    if (!restoreAnyLostBuffers ())
        return;

    EnterCriticalSection () ;

    DbgMsg (5, "  Listener Pos: %f %f %f\n", posListener.x, posListener.y, posListener.z) ;

    for (i=0; i < m_pMixer->m_nNumChannels; i++)
    {
        pChannel   = &m_pMixer->m_arChannels [i] ;
        pChEmitter = pChannel->m_pEmitter ;

        // -- if an emitter is playing on the channel, then update the channel's
        //    attribute values to reflect the emitters....
        //
        if (pChEmitter) // && pChEmitter->GetChangedFlags ())
        {
            pDSB   = pChannel->m_pDSB ;
            pDSB3d = pChannel->m_pDSB3d ;
            uFlags = pChEmitter->GetChangedFlags () ;

            // -- If channel is a TRUE3D channel ---------------------------------------------
            //
            if (pDSB3d)
            {
                DbgMsg (5, "  [%d] Updating True3d: %s\n", i, pChEmitter->GetName ()) ;
                if (uFlags & kPositionChanged)
                {
                    pChEmitter->GetPosition (pt) ;
                    DbgMsg (5, "  Updating Position: %f %f %f\n", pt.x, pt.y, pt.z) ;
                    pDSB3d->SetPosition (pt.x, pt.y, pt.z, DS3D_DEFERRED) ;
                }

                if (uFlags & kOrientationChanged)
                {
                    // -- Orientation is only applicable if the emitter has a cone...
                    if (pChEmitter->HasCone ())
                    {
                        pChEmitter->GetOrientation (pt) ;
                        //DbgMsg (5, "  Updating Orientation: %f %f %f\n", pt.x, pt.y, pt.z) ;

                        pDSB3d->SetConeOrientation (pt.x, pt.y, pt.z, DS3D_DEFERRED) ;
                    }
                }

                if (pChEmitter->IsDopplerShifted())
                {
                    if (uFlags & kVelocityChanged)
                    {
                        pChEmitter->GetVelocity (pt) ;
                        DbgMsg (5, "  Updating Velocity: %f %f %f\n", pt.x, pt.y, pt.z) ;
                        pDSB3d->SetVelocity (pt.x, pt.y, pt.z, DS3D_DEFERRED) ;
                    }
                }

                if (uFlags & kAttenuationRangeChanged)
                {
                    float fMin, fMax ;
                    pChEmitter->GetAttenuationRange (fMin, fMax) ;
                    DbgMsg (5, "  Updating Attenuation range: min=%f  max=%f\n", fMin, fMax) ;
                    pDSB3d->SetMinDistance (fMin, DS3D_DEFERRED) ;
                    pDSB3d->SetMaxDistance (fMax, DS3D_DEFERRED) ;
                }

                if (uFlags & kConeParametersChanged)
                {
                    uint32 uInside, uOutside ;
                    float  fOutsideVol ;
                    pChEmitter->GetConeParameters (uInside, uOutside, fOutsideVol) ;

                    pDSB3d->SetConeAngles        (uInside, uOutside, DS3D_DEFERRED) ;

                    int32 nOutsideVol = VolumeToDB (fOutsideVol) ;
                    pDSB3d->SetConeOutsideVolume (nOutsideVol, DS3D_DEFERRED) ;
					DbgMsg (3, "Set Cone for %s to %u %u %d\n", pChEmitter->GetName(), uInside, uOutside, nOutsideVol) ;
                }

                if (uFlags & kVolumeChanged)
                {
                    // -- DSound uses a volume expressed in db, ranging from 
                    //    DSBVOLUME_MAX (0) to DSBVOLUME_MIN(-10000) ;
                    //uint32 uDb = pChEmitter->GetVolume() * (DSBVOLUME_MIN - DSBVOLUME_MAX) / (1.0 - 0.0) ;
                    //uDb = DSBVOLUME_MAX ;
				    uint32 uDb = VolumeToDB (pChEmitter->GetVolume()) ;
                    DbgMsg (3, "  Updating Volume: %f (%ddb)\n", pChEmitter->GetVolume(), uDb) ;
                    hr = pDSB->SetVolume (uDb) ;
                    CHECK (SUCCEEDED (hr)) ;
                }
#if 0
                if (uFlags & kPitchChanged)
                {
                    DbgMsg (5, "  Updating Pitch: %f\n", pChEmitter->GetPitch ()) ;
//                    pDSB->SetFrequency (pChEmitter->GetPitch ()) ;
                }
#endif
            }

            else // -- Pseuodo 3d channel ----------------------------------------------------------
            {   
                //if (uFlags & (kPositionChanged | kOrientationChanged | kAttenuationRangeChanged |
                //              kPitchChanged | kVolumeChanged | kVelocityChanged))
                {
                    DbgMsg (5, "  [%d] Updating Pseudo-3d: %s\n", i, pChEmitter->GetName ()) ;

                    int32 nVolDB, nPanDB ;
                    CalcSpacialization (pChEmitter,
                                        posListener, vecListenerForward, vecListenerUp, vecListenerRight,
                                        nVolDB, nPanDB) ;
                    DbgMsg (5, "      volDB=%d  panDB=%d\n", nVolDB, nPanDB) ;

                    hr = pDSB->SetVolume (nVolDB) ;
                    CHECK (SUCCEEDED (hr)) ;

                    hr = pDSB->SetPan (nPanDB) ;
                    CHECK (SUCCEEDED (hr)) ;
                } // if something changed

            } // else pseudo-3d

            // -- clear the changed flags...
            pChEmitter->SetChangedFlags (0) ;
        }  // if change flags

    } // for


    // -- Commit and calc the changes we made that were defered...
    if (m_p3DListener)
        m_p3DListener->CommitDeferredSettings () ;    


    // -- Now that any changes have been commited, we can start any new emitters
    //    playing...
    for (i=0; i < m_pMixer->m_nNumChannels; i++)
    {
        pChannel = &m_pMixer->m_arChannels [i] ;

        // -- If this channel hasn't been started, do so...
        if (pChannel->m_pEmitter && !pChannel->m_bPlaying)
        {
            //DbgMsg (5, "      * Channel %d: Starting up...\n", pChannel->m_nChannelNum) ;
            HRESULT hr = pChannel->m_pDSB->Play (0, 0, DSBPLAY_LOOPING );
            if (FAILED (hr))
        		ConMsg ("ERROR: Couldn't PLAY (%s)\n", TranslateDSError (hr));
            else
                pChannel->m_bPlaying = true ;
        }
    }

    LeaveCriticalSection () ;

    DbgMsg (5, "==>End Frame\n") ;
} // BeginFrame



void IAudioImp_DSound3d::UpdateListenerPosition (const AudioPosition_t& ptPos)
{
    if (m_p3DListener)
    {
        HRESULT hr = m_p3DListener->SetPosition (ptPos.x, ptPos.y, ptPos.z, DS3D_DEFERRED) ;
        CHECK (SUCCEEDED(hr)) ;
    }
}


void IAudioImp_DSound3d::UpdateListenerOrientation (const AudioVector_t& vecDirection, const AudioVector_t& vecUp, const AudioVector_t& vecRight)
{
    if (m_p3DListener)
    {
        HRESULT hr = m_p3DListener->SetOrientation (vecDirection.x, vecDirection.y, vecDirection.z, 
                                                    vecUp.x, vecUp.y, vecUp.z, DS3D_DEFERRED) ;
        CHECK (SUCCEEDED(hr)) ;
    }
}


bool IAudioImp_DSound3d::IsEAXSupported ()
{
    return (m_pEAXReverb != 0);
}



void IAudioImp_DSound3d::SetReverbPreset (int nPresetID)
{
    if (m_pEAXReverb)
    {
        EAX_REVERBPROPERTIES* p = &garReverbProperties [nPresetID] ;
        m_pEAXReverb->SetPreset (p->environment, p->fVolume, p->fDecayTime_sec, p->fDamping) ;
    }
}


void IAudioImp_DSound3d::SetReverb (bool bEnable, float fVolume, float fDecayTime, float fDamping)
{
    if (m_pEAXReverb)
    {
        EAX_REVERBPROPERTIES* p = &garReverbProperties [0] ;
        m_pEAXReverb->SetPreset (p->environment, p->fVolume, p->fDecayTime_sec, p->fDamping) ;
    }
}


void IAudioImp_DSound3d::SetSpeakerConfiguration (SpeakerConfig_t speakerConfig, float fSpeakerSeperationInDegrees) 
{
    PRECOND (m_pDS) ;
    IAudioSpecificImplementation::SetSpeakerConfiguration (speakerConfig, fSpeakerSeperationInDegrees);
    int nDSId, nDSAngle ;

    switch (speakerConfig)
    {
        case kAudioHeadphone:
            nDSId = DSSPEAKER_HEADPHONE ;
            fSpeakerSeperationInDegrees = 180.f ;
            break;
        case kAudioMono:
            nDSId = DSSPEAKER_MONO ;
            break;
        case kAudioStereo:
            nDSId = DSSPEAKER_STEREO ;
            break;
        case kAudioQuad:
            nDSId = DSSPEAKER_QUAD ;
            break;
        case kAudioSurround:
            nDSId = DSSPEAKER_SURROUND ;
            break;
        default:
            CHECK (0);
    }


    if (fSpeakerSeperationInDegrees < 7.f)
        nDSAngle = DSSPEAKER_GEOMETRY_MIN ; // 5 degs
    else if (fSpeakerSeperationInDegrees < 15.f)
        nDSAngle = DSSPEAKER_GEOMETRY_NARROW ; // 10 degs
    else if (fSpeakerSeperationInDegrees < 120.f)
        nDSAngle = DSSPEAKER_GEOMETRY_WIDE ; // 20 degs
    else
        nDSAngle = DSSPEAKER_GEOMETRY_MAX ; // 180 degs

    m_pDS->SetSpeakerConfig (DSSPEAKER_COMBINED (nDSId, nDSAngle)) ;
}



void IAudioImp_DSound3d::SetPrimaryCacheSizeMS (uint32 uSizeInMS)
{
// TODO:    
}



void IAudioImp_DSound3d::SetMasterVolume (float fVol)
{
    m_fMasterVolume = fVol ;
    if (m_pPrimary)
    {
        m_pPrimary->SetVolume (fVol * kVolRange - kVolOffset) ;
        DbgMsg (2, "SetMasterVolume  db=%d\n", int (fVol * kVolRange - kVolOffset)) ;
    }
}



// ----------------------- IAudioEmitter_DSound3d ---------------------------


IAudioEmitter_DSound3d::IAudioEmitter_DSound3d (IAudioSpecificImplementation& audioReceiver,
                                                uint32 uAudioAttributes/*=kAFX_Default*/,
                                                bool bStream)
    : IAudioEmitter (audioReceiver, uAudioAttributes, bStream)
{
    m_pChannel       = 0 ;
    m_nNumPacketsOut = 0 ;
    m_uLastPlayPos   = 0 ;
}    



IAudioEmitter_DSound3d::~IAudioEmitter_DSound3d ()
{
}


void IAudioEmitter_DSound3d::ReceivedPacket (uint32 uPacketSize) 
{
    m_nNumPacketsOut-- ; 
    m_fRemainingTimeInSecs -= float(uPacketSize) / float(GetWaveFormat()->nAvgBytesPerSec) ;
    if (m_fRemainingTimeInSecs <0.f)
        m_fRemainingTimeInSecs = 0.f ;
    
	DbgMsg (3, "=> ReceivedPacket: numOut=%d  remainingSecs=%f  '%s'\n", m_nNumPacketsOut, m_fRemainingTimeInSecs, GetName ()) ;
}



float IAudioEmitter_DSound3d::GetSecondsPlayed () const
{
    // NOTE: This hasn't really been tested!
    if (!m_pChannel) 
		return (0);
		
    DWORD uPlayPos, uWritePos ;
    LPDIRECTSOUNDBUFFER  pDSB = m_pChannel->GetDSB () ;

    // -- Determine how many bytes have played of the current
    //    submission block...
    HRESULT hr = pDSB->GetCurrentPosition (&uPlayPos, &uWritePos);
    if (FAILED(hr))
    {
        uPlayPos = uWritePos = 0 ;
    }

    // -- find the start of the submission sized block currently
    //    being played....
    uint32 subSize     = m_pChannel->GetSubSize () ;
    uint32 segmentStart = 0 ;
    while (uPlayPos > segmentStart + subSize) 
    {
        segmentStart += subSize ;
    }    

    uint32 deltaBytesPlayed = uPlayPos - segmentStart ;

    return float (m_uNextStreamOffset - (subSize - deltaBytesPlayed)) / float (GetWaveFormat()->nAvgBytesPerSec) ;
}


float IAudioEmitter_DSound3d::GetRemainingTime () const 
{
    //return m_fRemainingTimeInSecs ; 
    return m_pAudioStream->GetSizeInSecs () -  GetSecondsPlayed () ;
}


uint32 IAudioEmitter_DSound3d::GetRemainingBytes () const
{
    return (float (GetWaveFormat()->nAvgBytesPerSec) / GetRemainingTime ()) ;
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

bool IAudioEmitter_DSound3d::AttachStream (IAudioStream* pNewStream)
{
    if (!IAudioEmitter::AttachStream (pNewStream))
        return (false);

    CHECK (m_bitsDesiredAttributes != kAFX_Default) ; // Default should already have been converted to bit flags

    return (true);
}



bool  IAudioEmitter_DSound3d::Play (uint32 uNumLoops/*=-1*/, float fInitialStartTimeInSecs/*=0*/) 
{
    IAudioEmitter::Play (uNumLoops, fInitialStartTimeInSecs) ;

    return true;
}

bool  IAudioEmitter_DSound3d::Stop () 
{
    GetImp()->onEmitterStop (this) ;
    return (true);
}


bool  IAudioEmitter_DSound3d::Pause () 
{
    IAudioEmitter::Pause () ;
    return (true);
}

bool  IAudioEmitter_DSound3d::Resume () 
{
    IAudioEmitter::Resume () ;
    return (true);
}


bool  IAudioEmitter_DSound3d::Rewind  () 
{
    IAudioEmitter::Rewind () ;
//???    
m_nNumPacketsOut = 0 ;
    return (true);
}


bool  IAudioEmitter_DSound3d::Mute () 
{
    IAudioEmitter::Mute () ;
    return (true);
}

bool  IAudioEmitter_DSound3d::UnMute () 
{
    IAudioEmitter::UnMute () ;
    return (true);
}


void  IAudioEmitter_DSound3d::SetPosition (const AudioPosition_t& ptPosition) 
{
    IAudioEmitter::SetPosition (ptPosition) ;
}


void  IAudioEmitter_DSound3d::SetOrientation (const AudioVector_t& vecDirection) 
{
    IAudioEmitter::SetOrientation (vecDirection) ;
}


float IAudioEmitter_DSound3d::SetPitch (float fPitch) 
{
    return IAudioEmitter::SetPitch (fPitch) ;
}


float IAudioEmitter_DSound3d::SetVolume (float fVolume) 
{
    if (fVolume == m_fVolume) return (fVolume);
    float prev = IAudioEmitter::SetVolume (fVolume) ;
    return prev ;
}


bool  IAudioEmitter_DSound3d::SetAttenuationRange (float fMinFront, float fMaxFront)
{
    IAudioEmitter::SetAttenuationRange (fMinFront, fMaxFront) ;
    return (true);
}


int IAudioImp_DSound3d::PlayControl (PlayCode_t eCode)
{
    if (!m_pPrimary) return (0);
    IAudioSpecificImplementation::PlayControl (eCode);

    if (eCode == kPlay)
    {
        if (m_bUseWin32Events)
        {
            // -- Create the thread...
            if (!m_hThread)
            {
                if (!createAudioThread ())
                    return (false);
            }
        }

        // -- Start things rolling...
        m_pPrimary->Play (0, 0, DSBPLAY_LOOPING) ;
    }
    else if (eCode == kStop)
    {
        m_pPrimary->Stop ();
    }
 
    return true;
}


const char* IAudioImp_DSound3d::GetName ()   
{ 
    return m_nNumTrue3dChannels == 0 ? "Pseudo-3D" : "DirectSound3D" ; 
}



// -- only called from IAudioEmitter::Stop()
//
void IAudioImp_DSound3d::onEmitterStop (IAudioEmitter_DSound3d* pEmitter)
{
    if (pEmitter->GetState() == IAudioEmitter::kStopped) 
        return ;
    pEmitter->IAudioEmitter::Stop () ;

    // -- if emitter was assigned a mixing channel, clear it from the channel...
    if (pEmitter->GetAssignedChannel() != -1)
        m_pMixer->AssignEmitterToChannel (pEmitter->GetAssignedChannel(), 0) ;

    DbgMsg (3, "onEmitterStop: %s\n", pEmitter->GetName()) ;

    m_lstFinishedEmitters.AddHead (pEmitter) ;
}


void IAudioImp_DSound3d::DumpChannels ()
{
    // -- Debug dump...
    static DWORD dwDumpTime = 0 ;
//**    if (dwDumpTime + 1000 < timeGetTime())
    {
        dwDumpTime = timeGetTime() ;
        DbgMsg (2, "\n------ Channel State ------\n") ;
        for (int i=0; i < m_pMixer->m_nNumChannels; i++)
        {
            DbgMsg (2, "  [%d] %s\n", 
                     i, 
                    (m_pMixer->m_arChannels [i].m_pEmitter ? m_pMixer->m_arChannels [i].m_pEmitter->GetName () : "")) ;
        }
        DbgMsg (2, "\n") ;
    }
}



bool IAudioImp_DSound3d::createAudioThread ()
{
    PRECOND (!m_hThread) ;
    DWORD   dwThreadID;

    ConMsg ("\nStarting Audio Thread...\n") ;

    m_hThread = ::CreateThread (NULL, 
                                0, 
                                AudioEngineThreadEntryPoint_DSound3d,
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


void IAudioImp_DSound3d::handleFillRequest (IAudioEmitter_DSound3d* pEmitter, iDSoundChannel* pChannel, int nEventNum)
{
    if (pEmitter == 0) return;
    PRECOND (pChannel) ;

    //DbgMsg (5, "FillReQ: channel %d.%d - %s\n", nChannelIndex, nEventNum, pEmitter->GetName ()) ;

    // -- May have aborted playback, if so, then we don't want to sumbit any more buffers...    
    if (pEmitter->GetState () != IAudioEmitter::kPlaying)
        return ;
        
    // -- Is the audio done playing...
    //
    if (0 == pEmitter->GetNumOutstandingPackets ()) 
    {
        // finished playing wave; mark channel as vacant...
        DbgMsg (4, "  Sound finished: [evnt=%d] %s  %s\n", nEventNum, pEmitter->GetName (), pEmitter->IsAutoDelete() ? "(auto deleting)" : "") ;
        pEmitter->Stop () ;
    }
    else
    {   // Send next packet of data...

        // -- if there is more audio data to send, then send it...
        if (!pEmitter->IsEOD())
        {
            bool bOk = m_pMixer->SubmitPacket (pEmitter, false, nEventNum) ;
        }
        else
        {
            pChannel->SilentPacket () ;
        }
    }

#if 0
    // -- Debug dump...
    static DWORD dwDumpTime = 0 ;
    if (dwDumpTime + 1000 < timeGetTime())
    {
        dwDumpTime = timeGetTime() ;
        DbgMsg (1, "\n------ Channel State ------\n") ;
        for (int i=0; i < m_pMixer->m_nNumChannels; i++)
        {
            DbgMsg (1, "  [%d] %s\n", 
                     i, 
                    (m_pMixer->m_arChannels [i].m_pEmitter ? m_pMixer->m_arChannels [i].m_pEmitter->GetName () : "")) ;
        }
        DbgMsg (1, "\n") ;
    }
#endif
}


// -- refillBuffers ---------------------------------------------------------
//
//  Purpose: 
//     Check and fill any buffers that are running low of audio data.
//
//  Note:
//     This routine needs to be called enough such that none of the audio 
//     buffers run out of valid audio data.
//
//     The submission packet size directly affects the maximum delay
//     between successive calls to refillBuffers, If this size is to small,
//     then the audio will break up.
// --------------------------------------------------------------------------

void IAudioImp_DSound3d::refillBuffers ()
{
    iDSoundChannel*         pChannel ;
    IAudioEmitter_DSound3d* pEmitter ;
    LPDIRECTSOUND3DBUFFER   pDSB3d ;
    LPDIRECTSOUNDBUFFER     pDSB ;
    int i ;
    HRESULT hr ;
    DWORD uPlayPos, uWritePos ;

    for (i=0; i < m_pMixer->m_nNumChannels; i++)
    {
        pChannel   = &m_pMixer->m_arChannels [i] ;
        pEmitter = pChannel->m_pEmitter ;

        if (pEmitter == 0)
            continue;
// SHAMEFUL HACK FOR NOW
if (!pEmitter->GetStream () || pEmitter->GetStream () == (IAudioStream*)0xdddddddd)
	continue;

        pDSB   = pChannel->m_pDSB ;
        pDSB3d = pChannel->m_pDSB3d ;

        hr = pDSB->GetCurrentPosition (&uPlayPos, &uWritePos);
        if (FAILED(hr))
            continue;

        // We need to fill the buffer whenever we've played half way
        // thru the buffer and when it wraps back to the top of the
        // buffer.
        //
        uint32 subSize = pChannel->m_uSubmissionSize ;

        if (uPlayPos < pEmitter->GetLastPlayPos ()  // we wrapped
            || (pEmitter->GetLastPlayPos() < subSize && uPlayPos >= subSize))
        {
            // -- increment the packets recv'd counter for each submission
            //    size block we refill...
            pEmitter->ReceivedPacket (pChannel->m_uSubmissionSize) ;
            handleFillRequest (pEmitter, pChannel, -1) ;

        } // if
                
        pEmitter->updateLastPlayPos (uPlayPos) ;
    } // for i    

} // refillBuffers



bool IAudioImp_DSound3d::initEAX ()
{
	CHECK (m_pMixer->m_arChannels[0].m_pDSB3d) ;
	m_pEAXReverb = new CReverb (m_pMixer->m_arChannels[0].m_pDSB3d) ;
	if (!m_pEAXReverb->PropertySetOk ())
	{
		DbgMsg (2, "EAX is NOT available.\n") ;
		SafeDelete (m_pEAXReverb) ;
		return false ;
	}
	else
		DbgMsg (2, "System has EAX\n") ;
	return true;
}


// ===========================================================================
//                          Thread Handling
// ===========================================================================

// -- handleFillRequestThread -----------------------------------------------
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

void IAudioImp_DSound3d::handleFillRequestThread (int nEventNum)
{
    CHECK (nEventNum >= kEngineEventStartOfChannels && nEventNum < kDS3D_MaxNumberEvents) ;

    // -- get the audio channel and emitter that is using 'nEventNum'...
    int nChannelIndex = (nEventNum - kEngineEventStartOfChannels) / kDS3D_MaxPacketsPerChannel ;
    iDSoundChannel* pChannel = &m_pMixer->m_arChannels [nChannelIndex] ;
    IAudioEmitter_DSound3d* pEmitter = pChannel->GetEmitter () ;

    pEmitter->ReceivedPacket (pChannel->m_uSubmissionSize) ;

    handleFillRequest (pEmitter, pChannel, nEventNum) ;

} // handleFillRequest



static DWORD WINAPI AudioEngineThreadEntryPoint_DSound3d (LPVOID pWorkContext) 
{
    DWORD dwReturnedEvent = 0;
    DWORD dwState = 0;
    int i = 0;
    IAudioImp_DSound3d* pThis = (IAudioImp_DSound3d*)pWorkContext;
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
                DbgMsg (3, "Thread: Recvd STOP event\n") ;
                return 0;

            case kEngineEventStart:
            {
                DbgMsg (3, "Thread: Recvd START event\n") ;
                break;
            }

            case kEngineEventPause:
                DbgMsg (3, "Thread: Recvd PAUSE event\n") ;
                break;

            default:
                // This must be a buffer fill request...
                CHECK (dwReturnedEvent >= kEngineEventStartOfChannels && dwReturnedEvent < pThis->m_nNumberEvents) ;

                pThis->EnterCriticalSection ();
                    pThis->handleFillRequestThread (dwReturnedEvent) ;
                pThis->LeaveCriticalSection () ;

                break;
        } // endswitch
    } // endwhile

    return 0;
}



// ==========================================================================
//                              End of File
// ==========================================================================
