// ==========================================================================
//  File: IIntelRSXAudio.h
// 
//  Contents    : Interfaces Intel's RSX Audio
//  Date Created: 7/19/98 jas
// ==========================================================================

#ifndef _IAUDIODEST_INTELRSX
#define _IAUDIODEST_INTELRSX

#include "..\Audio.h"
#include "..\AudioPrivate.h"
#include "..\rsx.h"

// ---------------------------- Constants -----------------------------------

const int kNumberBufferHeaders = 4 * kMaxNumberOfChannels; 

// No more than this number of packets will be used per channel. Note: that
// this value along with the packet size directly affect latency...
const int kRSX_MaxPacketsPerChannel = 2 ; 
const int kRSX_MaxNumberEvents = kEngineEventStartOfChannels + kMaxNumberOfChannels * kRSX_MaxPacketsPerChannel ;

// ------------------------------ Types -------------------------------------

class IAudioImp_IntelRSX ;
class IAudioMixer_IntelRSX ;

// ------------------------ IAudioEmitter_IntelRSX --------------------------

class CLASS_DECLSPEC IAudioEmitter_IntelRSX : public IAudioEmitter
{
  public:
    IAudioEmitter_IntelRSX (IAudioSpecificImplementation& audioReceiver, 
                            uint32 uAudioAttributes=kAFX_Default,
                            bool bStream=false) ;


    virtual ~IAudioEmitter_IntelRSX () ;

    virtual bool AttachStream (IAudioStream* pNewStream) ;

    // -- Actions...
    //
    virtual bool  Play           (uint32 uNumLoops=-1, float fInitialStartTimeInSecs=0) ;
    virtual bool  Stop           () ;
    virtual bool  Pause          () ;
    virtual bool  Resume         () ;
    virtual bool  Mute           () ;
    virtual bool  UnMute         () ;
    virtual bool  Rewind         () ;
//*    virtual bool  Flush          () ; 

    virtual void  SetPosition    (const AudioPosition_t& ptPosition) ;
    virtual void  SetOrientation (const AudioVector_t& vecDirection) ;
    virtual float SetPitch       (float fPitch) ;
    virtual float SetVolume      (float fVolume) ;

    virtual bool  SetAttenuationRange (float fMinFront, float fMaxFront) ;

    void ReceivedPacket ()            { /*PRECOND (m_nNumPacketsOut > 0) ; */m_nNumPacketsOut-- ; }
    int  GetNumOutstandingPackets ()  { return m_nNumPacketsOut ; }

  protected:
    // -- RSX specific data
    IAudioImp_IntelRSX* GetImp () { return ((IAudioImp_IntelRSX*)&m_AudioImp); }
    //IRSX* GetRSX ()         { return ((IAudioImp_IntelRSX*)&m_AudioImp)->m_pRSX; }

    IRSXStreamingEmitter* m_pSE ;
    int32                 m_nNumPacketsOut ;

    friend IAudioMixer_IntelRSX ;
};


// ------------------------ IAudioMixer_IntelRSX --------------------------

class CLASS_DECLSPEC IAudioMixer_IntelRSX
{
  public:
    IAudioMixer_IntelRSX (IAudioEngine& rAudioEngine, int nNumChannels,
                          const IWaveFormat& waveFormat,
						  float fAudioSubmissionSizeInSecs,
                          HANDLE* parEvents, uint32 uNumEventsPerChannel);
    virtual ~IAudioMixer_IntelRSX () ;

    void         SetFormat    (const IWaveFormat& waveFormat) ;
    virtual bool SubmitPacket (IAudioEmitter* pEmitter, bool bInitialLoad, int nEventIndex) ;

    uint32 GetSubmissionSize ()             { return m_uSubmissionSize ; }
    void   AssignEmitterToChannel (int nChannelInex, IAudioEmitter_IntelRSX* pEmitter) ;

    int  findFreeBufferHeader (int nFirstTry) ;

    int            m_nNumChannels ;
    IAudioEmitter_IntelRSX*  m_arChannels [kMaxNumberOfChannels] ;

    IAudioEngine&  m_rAudioEngine ;
	IAudioPacket   m_arChannelPackets [kNumberBufferHeaders] ;	// Uses same indexing as m_arBufferHdrs
    RSXBUFFERHDR   m_arBufferHdrs 	  [kNumberBufferHeaders] ;
    RSXBUFFERHDR*  m_pFreeBuff ;
    uint32         m_uSubmissionSize ;

    HANDLE*        m_arEvents ;
    uint32         m_uNumEventsPerChannel ;
    IWaveFormat    m_WaveFormat ;
}; // IAudioMixer_IntelRSX



// ------------------------ IAudioImp_IntelRSX --------------------------

class CLASS_DECLSPEC IAudioImp_IntelRSX : public IAudioSpecificImplementation
{
  public:
    IAudioImp_IntelRSX () ;
    virtual ~IAudioImp_IntelRSX () ;

    virtual bool Initialize (IAudioDeviceCaps* pAudioDev, 
                             IAudioEngine& audioEngine, 
                             HWND hWnd, const IWaveFormat& WaveFormatToUse, 
                             int nNumChannels, int nNumTrue3dChannels,
                             float fOneUnitInMeters,
                             uint32 uUseFlags,
                             AudioDataConfigParams_t* pDataConifgParams) ;

    virtual void Shutdown () ;
    virtual bool Connect () ;       // grabs the audio device
    virtual bool DisConnect () ;    // releases the audio device
    virtual bool ChangeOutputFormat (const IWaveFormat& newFormat) ;

    virtual bool CreateEmitter  (IAudioEmitter** pAudioEmitter, uint32 nAudioAttributes, 
                                 LoadMethod_t eLoadAs=kLoadAllIntoMem) ;
    virtual void DestroyEmitter (IAudioEmitter* pAudioEmitter) ;
    virtual bool SubmitPacket (IAudioEmitter* pEmitter, bool bInitialLoad, int nEventIndex) ;

    virtual void BeginFrame (AudioPosition_t& posListener, AudioVector_t& vecListenerDirection, 
                             AudioVector_t& vecListenerUp, AudioVector_t& vecListenerRight) ;
    virtual void UpdateListenerPosition    (const AudioPosition_t& ptPosition) ;
    virtual void UpdateListenerOrientation (const AudioVector_t& vecDirection, const AudioVector_t& vecUp, const AudioVector_t& vecRight) ;

    // -- Environment settings...
    //
    virtual void SetMasterVolume (float fVol) ;
    virtual void SetReverb (bool bEnable, float fVolume, float fDecayTime, float fReverbDamping) ;
    virtual void SetReverbPreset (int nPresetID) ;

    virtual void SetSpeakerConfiguration (SpeakerConfig_t speakerConfig, float fSpeakerSeperationInDegrees)  ;

    virtual void SetPrimaryCacheSizeMS (uint32 uSizeInMS) ;
    
    virtual int PlayControl (PlayCode_t eCode) ;

    virtual const char* GetName ()   { return "Intel's RSX" ; }

    // -- non-IAudioSpecificImplementation methods...
    void  onEmitterStop (IAudioEmitter_IntelRSX* pEmitter) ;

    IRSX* GetRSX ()         { return m_pRSX; }
    //virtual void SilentPacket (int nChannel)        {} ;

    virtual bool UseHWBuffers ()        { return false ; }

    void  EnterCriticalSection () ;
    void  LeaveCriticalSection () ;

    void  EnterLock ()                  { EnterCriticalSection () ; }
    void  ReleaseLock ()                { LeaveCriticalSection () ; }

//  protected:
    bool createAudioThread  () ;
    void handleFillRequest (int nEventNum) ;
    bool createMixer    (int nNumChannels=16) ;
    bool updateRegistry () ;

    int                   m_nActiveChannels ;

    // -- RSX specific data...
    IRSX*                 m_pRSX;
    IRSXDirectListener*   m_pDL;
    IUnknown*             m_pRSXUnknown ;
    IAudioMixer_IntelRSX* m_pMixer ;

    // -- Thread and Event data...
    HANDLE              m_hThread ;       // Handle to thread that services audio buffers with data
    CRITICAL_SECTION    m_CriticalSection;
    bool                m_bCriticalSectionInited ;
    int32               m_nNumberEvents ;
    HANDLE              m_hBufferEvents [kRSX_MaxNumberEvents] ;
    bool                m_bShutdown ;

}; // IAudioImp_IntelRSX


inline void IAudioImp_IntelRSX::EnterCriticalSection ()
{
    if (m_bCriticalSectionInited)
        ::EnterCriticalSection (&m_CriticalSection);
}

inline void IAudioImp_IntelRSX::LeaveCriticalSection ()
{
    if (m_bCriticalSectionInited)
        ::LeaveCriticalSection (&m_CriticalSection);
}

// ----------------------------  Globals  -----------------------------------

// ---------------------------- Prototypes ----------------------------------

inline bool IAudioImp_IntelRSX::SubmitPacket (IAudioEmitter* pEmitterIn, bool bInitialLoad, int nEventIndex)
{
    EnterCriticalSection () ;

    //return m_pMixer->SubmitPacket (nChannel, pEmitterIn, audioPacket, hEventSignal, nEventIndex) ;
    bool bOk = m_pMixer->SubmitPacket (pEmitterIn, bInitialLoad, nEventIndex) ;

    LeaveCriticalSection ();

    return bOk;
}


#endif // _IAUDIODEST_INTELRSX
// ==========================================================================
//                              End of File
// ==========================================================================
