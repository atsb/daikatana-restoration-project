// ==========================================================================
//  File: AudioImp_DSound.h
// 
//  Contents    : Interfaces DSound3d implementation
//  Date Created: 7/19/98 jas
// ==========================================================================

#ifndef _IAUDIOIMP_DSOUND3D
#define _IAUDIOIMP_DSOUND3D

#include "..\Audio.h"
#include "..\AudioPrivate.h"
#include "dsound.h"

// ---------------------------- Constants -----------------------------------

// -- !NOTE: This data should be obtained from the AudioDataConfigParams_t
//           param on the Initialize() call.  Arrays will need to be dynamically
//           allocated.

// No more than this number of packets will be used per channel. Note: that
// this value along with the packet size directly affect latency...
const int kDS3D_MaxPacketsPerChannel = 2 ; 
const int kDS3D_MaxNumberEvents = kEngineEventStartOfChannels + kMaxNumberOfChannels * kDS3D_MaxPacketsPerChannel ;

// ------------------------------ Types -------------------------------------

class IAudioImp_DSound3d ;
class IAudioMixer_DSound3d ;
class iDSoundChannel ;
class CReverb ;

// ----------------------- IAudioEmitter_DSound3d ---------------------------

class CLASS_DECLSPEC IAudioEmitter_DSound3d : public IAudioEmitter
{
  public:
    IAudioEmitter_DSound3d (IAudioSpecificImplementation& audioReceiver, 
                            uint32 uAudioAttributes=kAFX_Default,
                            bool bStream=false) ;


    virtual ~IAudioEmitter_DSound3d () ;

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

    virtual void  SetPosition    (const AudioPosition_t& ptPosition) ;
    virtual void  SetOrientation (const AudioVector_t& vecDirection) ;
    virtual float SetPitch       (float fPitch) ;
    virtual float SetVolume      (float fVolume) ;

    virtual bool  SetAttenuationRange (float fMinFront, float fMaxFront) ;

    virtual float  GetSecondsPlayed ()  const ;
    virtual float  GetRemainingTime ()  const ;
    virtual uint32 GetRemainingBytes () const ;

    void ReceivedPacket           (uint32 uPacketSize) ;
    int  GetNumOutstandingPackets ()  { return m_nNumPacketsOut ; }

    IAudioImp_DSound3d* GetImp () { return ((IAudioImp_DSound3d*)&m_AudioImp); }

    uint32 GetLastPlayPos ()          { return m_uLastPlayPos; }

    void updateLastPlayPos (uint32 newPos)   { m_uLastPlayPos = newPos; }

  protected:
    // -- DSound3d specific data
    int32    m_nNumPacketsOut ;
    uint32   m_uLastPlayPos ;       // Used if we're not using events

    // Changes to the emitter get reflected to the channel
    iDSoundChannel* m_pChannel ;

    friend IAudioMixer_DSound3d ;
};


// ----------------------- iDSoundChannel ---------------------------

class iDSoundChannel
{
  public:
    iDSoundChannel () ;
   ~iDSoundChannel () ;

    bool Initialize    (int nChannelIndex, uint32 uBuffSize, uint32 uSubmissionSizeInBytes,
                        LPDIRECTSOUNDBUFFER pDSB, LPDIRECTSOUND3DBUFFER pDSB3d) ;
    bool UseEvents     (HANDLE* parEvents, uint32 uNumEventsPerChannel, uint32 uSubmissionSizeInBytes) ;

    bool WriteSilence  (uint32 uStart, uint32 uLength) ;
    bool SilentPacket  () ;
    bool WriteToBuffer (IAudioPacket& audioPacket) ;
    void Stop () ;
    void AssignEmitter (IAudioEmitter_DSound3d* pEmitter) ;
    IAudioEmitter_DSound3d* GetEmitter ()               { return m_pEmitter ; }

    LPDIRECTSOUNDBUFFER  GetDSB ()          { return m_pDSB ; }
    uint32               GetSubSize ()      { return m_uSubmissionSize ; }

    // data
  private:
    IAudioEmitter_DSound3d* m_pEmitter ;
    LPDIRECTSOUNDBUFFER   m_pDSB ;
    LPDIRECTSOUND3DBUFFER m_pDSB3d ;
    int                   m_nChannelNum ;
    LPDIRECTSOUNDNOTIFY   m_pDSNotify;
    DSBPOSITIONNOTIFY     m_arNotifyPos [kDS3D_MaxPacketsPerChannel] ;
    HANDLE                m_hEventFillReq ;

    uint32                m_uNextWriteOffset,
                          m_uBufferSize,
                          m_uSubmissionSize ;
    uint16                m_uSilenceValue ;
    bool                  m_bPlaying ;
    int32                 m_nCurrentFreq ;

    friend IAudioMixer_DSound3d ;
    friend IAudioImp_DSound3d ;
} ;


// ----------------------- IAudioMixer_DSound3d ---------------------------

class CLASS_DECLSPEC IAudioMixer_DSound3d
{
  public:
    IAudioMixer_DSound3d () ;
    virtual ~IAudioMixer_DSound3d () ;

    virtual bool SubmitPacket (IAudioEmitter* pEmitter, bool bInitialLoad, int nEventIndex) ;
//    virtual bool SubmitPacket (int nChannel, IAudioEmitter* pEmitterIn, 
//                               IAudioPacket& audioPacket, HANDLE hEventSignal, int nEventIndex) ;
    void AssignEmitterToChannel (int nChannelIndex, IAudioEmitter_DSound3d* pEmitter) ;

    bool create (IAudioImp_DSound3d& audioImp, const IWaveFormat* pWaveFormat, 
                 int nNumChannels, int nNumTrue3dChannels,
				 float fAudioSubmissionSizeInSecs,
                 HANDLE arEvents[], uint32 uNumEventsPerChannel);

  protected:
    int            m_nNumChannels ;
    int            m_nNumTrue3dChannels ;
    iDSoundChannel m_arChannels [kMaxNumberOfChannels] ;

    friend IAudioImp_DSound3d ;
}; // IAudioMixer_DSound3d



// ----------------------- IAudioImp_DSound3d ---------------------------

class CLASS_DECLSPEC IAudioImp_DSound3d : public IAudioSpecificImplementation
{
  public:
    IAudioImp_DSound3d () ;
    virtual ~IAudioImp_DSound3d () ;

    virtual bool Initialize (IAudioDeviceCaps* pAudioDev, 
                             IAudioEngine& audioEngine, 
                             HWND hWnd, 
                             const IWaveFormat& WaveFormatToUse, 
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

    virtual const char* GetName () ;

    // -- Environment settings...
    //
    virtual bool IsEAXSupported () ;
    virtual void SetMasterVolume (float fVol) ;
    virtual void SetReverb (bool bEnable, float fVolume, float fDecayTime, float fReverbDamping) ;
    virtual void SetReverbPreset (int nPresetID) ;

    virtual void SetSpeakerConfiguration (SpeakerConfig_t speakerConfig, float fSpeakerSeperationInDegrees) ;

    virtual void SetPrimaryCacheSizeMS (uint32 uSizeInMS) ;

    virtual int PlayControl (PlayCode_t eCode) ;

    // -- non-IAudioSpecificImplementation methods...

    bool CalcSpacialization (IAudioEmitter_DSound3d* pEmitter,
                             AudioPosition_t& posListener, AudioVector_t& vecListenerForward, 
                             AudioVector_t& vecListenerUp, AudioVector_t& vecListenerRight,
                             int32& nVolDBOut, int32& nPanDBOut) ;

    LPDIRECTSOUNDBUFFER createSecondaryBuffer (const IWaveFormat* pWaveFormat, uint32 uNumBytes, uint32 uAttrFlags) ;

    void  EnterCriticalSection () ;
    void  LeaveCriticalSection () ;

    //void  EnterLock ()                  {}; // EnterCriticalSection () ; }
    //void  ReleaseLock ()                {}; //{ LeaveCriticalSection () ; }
    void  EnterLock ()                  { EnterCriticalSection () ; }
    void  ReleaseLock ()                { LeaveCriticalSection () ; }

  //protected:
    bool createAudioThread  () ;
    void handleFillRequestThread  (int nEventNum) ;
    void handleFillRequest (IAudioEmitter_DSound3d* pEmitter, iDSoundChannel* pChannel, int nEventNum) ;
    virtual void DumpChannels () ;

  protected:
    virtual bool  createDSoundObjects (LPGUID pGuid, HWND hWnd, const IWaveFormat& WaveFormatToUse,
		                               int nNumChannels,     int nNumTrue3dChannels,
		                               bool bUseWin32Events, float fOneUnitInMeters) ;
    virtual void  destroyDSoundObjects () ;
	bool  initEAX () ;
    void  refillBuffers () ;
    void  onEmitterStop (IAudioEmitter_DSound3d* pEmitter) ;
    bool  createPrimarySoundBuffer (bool bUse3d, const IWaveFormat* pWaveFormat)  ;
    bool  restoreAnyLostBuffers () ;

    // -- DirectSound specific data...
    HANDLE                  m_hDSoundLib ;
    LPDIRECTSOUND	        m_pDS;
    LPDIRECTSOUNDBUFFER     m_pPrimary;
    //LPWAVEFORMATEX	        m_pWaveFormat;
    LPDIRECTSOUND3DLISTENER m_p3DListener;
    IAudioMixer_DSound3d*   m_pMixer ;
    CReverb*                m_pEAXReverb ;

    float                   m_fMasterVolume ; // Vol expressed as 0. to 1.

    // -- Thread and Event data...
    bool                m_bUseWin32Events ;
    HANDLE              m_hThread ;       // Handle to thread that services audio buffers with data
    CRITICAL_SECTION    m_CriticalSection;
    bool                m_bCriticalSectionInited ;
    int32               m_nNumberEvents ;
    HANDLE              m_hBufferEvents [kDS3D_MaxNumberEvents] ;

    friend DWORD WINAPI AudioEngineThreadEntryPoint_DSound3d (LPVOID pWorkContext)  ;
    friend bool  IAudioEmitter_DSound3d::Stop () ;

}; // IAudioImp_DSound3d


// ----------------------------  Globals  -----------------------------------

// ---------------------------- Prototypes ----------------------------------


inline bool IAudioImp_DSound3d::SubmitPacket (IAudioEmitter* pEmitterIn, bool bInitialLoad, int nEventIndex)
{
    // we need to make sure that from the time we get the audio stream data and submit it
    // that the audio stream's data doesn't change.
    EnterCriticalSection () ;

    bool bOk = m_pMixer->SubmitPacket (pEmitterIn, bInitialLoad, nEventIndex) ;

    LeaveCriticalSection () ;
    return bOk;
}


inline void IAudioImp_DSound3d::EnterCriticalSection ()
{
    if (m_bCriticalSectionInited)
        ::EnterCriticalSection (&m_CriticalSection);
}

inline void IAudioImp_DSound3d::LeaveCriticalSection ()
{
    if (m_bCriticalSectionInited)
        ::LeaveCriticalSection (&m_CriticalSection);
}


#endif // _IAUDIOIMP_DSOUND3D
// ==========================================================================
//                              End of File
// ==========================================================================
