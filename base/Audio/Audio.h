// ==========================================================================
//  File: IAudio.h
// 
//  Dependencies:
//      Assumes that a global object of type IFileSystem has been defined:
//          IFileSystem FileSystem ;
//      All file i/o uses this FileSystem object.
//
//  Contents : Audio class definitions for the following:
//                 IWaveFormat
//                 IAudioStream
//                 IAudioEmitter
//                 IAudioEffect
//                 IAudioMonitor
//                 IAudioEngine
//
//  Date Created: 7/17/98 jas
// ==========================================================================

#ifndef _IAUDIO_H
#define _IAUDIO_H

#include <windows.h>
#include <mmreg.h>
#include "AudioBase.h"
#include "IFileAccessorPriv.h"
#include "CollectionClasses.h"
#include "IReaders.h"

#define CLASS_DECLSPEC

// -- AAudioEffect ----------------------------------------------------------
//
// --------------------------------------------------------------------------


// ---------------------------- Constants -----------------------------------

// -- Can have up to this many active channels playing at once...
const int kMaxNumberOfChannels     = 32 ;

// -- event indices into AudioEngine::m_hEvents
const int kEngineEventStart = 0 ;
const int kEngineEventPause = 1 ;
const int kEngineEventStop  = 2 ;
const int kEngineEventStartOfChannels  = 4 ; // Start on even number

//// -- These "events" are Win32 Events...
//const int kMaxNumberEvents = kEngineEventStartOfChannels + kMaxNumberOfChannels * kMaxPacketsPerChannel ;

// -- These define the default ambient and attenuation regions for emitters...
const float kMinFrontRegion = 80.f,
            kMaxFronRegion  = 50000.f ;


// -- Reverb Settings...
//
//    Note: These reverb settings MUST correspond to the array of reverb settings
//          in AudioEngine.cpp.
//
enum
{
    kReverbPresetGeneric,       //    0
    kReverbPresetPaddedCell,    //    1
    kReverbPresetRoom,          //    2
    kReverbPresetBathroom,      //    3
    kReverbPresetLivingRoom,    //    4
    kReverbPresetStoneRoom,     //    5
    kReverbPresetAuditorium,    //    6
    kReverbPresetConcertHall,   //    7
    kReverbPresetCave,          //    8
    kReverbPresetArena,         //    9
    kReverbPresetHangar,         //  10
    kReverbPresetCarpetedHallway,//
    kReverbPresetHallway,        //
    kReverbPresetStoneCorridor,  //
    kReverbPresetAlley,          //
    kReverbPresetForest,         //  15
    kReverbPresetCity,           // 
    kReverbPresetMountains,      //
    kReverbPresetQuarry,         //
    kReverbPresetPlain,          //
    kReverbPresetParkinglot,     //  20
    kReverbPresetSewerpipe,      //  21
    kReverbPresetUnderwater,     //  22
    kReverbPresetDrugged,        //  23
    kReverbPresetDizzy,          //  24
    kReverbPresetPsychotic,      //  25

    kReverbPresetCount
};


// -- Use flags passed into IAudioImp_xxx::Initialize
enum
{
    kUseReverb=1,
    kUseEAX=2
} ;



// ------------------------------ Types -------------------------------------

typedef enum { 
    kBufferLoc_Default, 
    kBufferLoc_HW, 
    kBufferLoc_SystemMem 
} AudioBufferLocation_t ;

typedef enum { 
    kBufferType_Default, 
    kBufferType_Streamed,  
    kBufferType_InMemory 
} AudioBufferType_t ;

typedef enum { 
    kAFX_Default=-1, 
    kAFX_3D     =1,  	// Spatialize 
    kAFX_Doppler=2,     // ie. Doppler
    kAFX_Volume =4,  	// ie. Attenuate
    kAFX_Pan    =8,  	// ie. will use 2d panning instead of 3d
    kAFX_Reverb =16 
} AFXAttributes_t ;


typedef enum { 
    kAudioClassGeneral,
    kAudioClassWeapon,
    kAudioClassVoice,
    kAudioClassItem,
    kAudioClassBody,
    kAudioClassFeet,
    kAudioClassMusic, 
    kAudioClassAmbient, 
    kAudioClassStreaming
} AudioClass_t ;

typedef enum { 
    kPriorityImmediate, 
    kPriorityHigh, 
    kPriorityMed, 
    kPriorityLow
} AudioPriority_t ;

typedef enum { 
    kAudio8Bits, 
    kAudio16Bits, 
    kAudio24Bits, 
    kAudio32Bits 
} AudioBitsPerSample_t ;

typedef enum { 
    kAudioHeadphone,
    kAudioMono, 
    kAudioStereo, 
    kAudioQuad,
    kAudioSurround  // 5 channel
} SpeakerConfig_t ;

typedef enum {
    kLoadAllIntoMem,        // Load a wave entirely into mem
    kLoadForStreaming,      // Load a wave for streaming
    kLoadChooseBest         // Pick the best way for loading wave (either in mem or stream)
} LoadMethod_t ;


typedef struct
{
	// -- Can have up to this many active channels playing at once...
	int nMaxNumberOfChannels     ; //= 32 ;
	int nNumberOfChannels ; //= 16 ; // must be <= to MaxNumberOfChannels

	// No more than this number of packets will be used per channel. Note: that
	// this value along with the packet size directly affect latency...
	int nMaxPacketsPerChannel ; //= 2 ; 

	// This is the data block size that is buffered just ahead of the read ptr;
	// this is, in affect, the latency between the time a play sound request is
	// made and the time before it reaches the speakers.
	float fAudioLatency ; //= 0.020f ;    // 20 ms

	// This is the cache size used when streaming audio data...
	float fAudioStreamCacheSizeInSecs ; //= 2.0f ;

	// This is the size used to submit audio data to the mixer...
	float fAudioSubmissionSizeInSecs ; //= 1.0f ; //0.500f ; // 200 ms

	// This is the size used to send audio packets thru the special effects chain...
	float fAFXProcessingSizeInSecs ; //= 0.200f;

	// -- These "events" are Win32 Events...
	int nMaxNumberEvents ; //= kEngineEventStartOfChannels + kMaxNumberOfChannels * kMaxPacketsPerChannel ;

    // -- Stream any wave files > than this number (default 3.0 secs) 
    float fMinStreamSize;

} AudioDataConfigParams_t ;


// forward decls
class IAudioEngine ;    
class IAudioSpecificImplementation ;
class AAudioMixer ;
class ADataAccessor ;
class AFormatReader ;
class IAudioStream ;
class IAudioChannel ;
class IAudioPacket ;
class IAudioEmitter ;
class IAudioDeviceCaps ;
class ISoundDefs ;
class ISoundDef ;
class AudioPluginMgr ;
class AudioPlugin ;
class AAudioDecompressor ;

typedef list<IAudioEmitter*> EmitterList_t ;
typedef list<IAudioStream*>  StreamList_t ;


// -- 3d coord struct for audio positioning...
class CLASS_DECLSPEC AudioPosition_t
{
  public:
    AudioPosition_t ()                             { x = 0.f; y = 0.f ; z = 0.f; }
    AudioPosition_t (float fx, float fy, float fz) { x = fx; y = fy; z = fz; }
    float x, y, z ;
} ;
 
typedef AudioPosition_t AudioVector_t;



// -- IWaveFormat -----------------------------------------------
//
//  Purpose: 
//    Defines an audio format. The default format is for:
//          22kHz, 16-bit mono, PCM format
//
//  Notes:
//    We use MS Window's Wave format struct for defining wave data. This
//    class is just a simple wrapper around it that initializes it.
//    
//    IMPORTANT: Constructors DO NOT allocate any extra memory for
//               arCompressedData[]!  If compressed data is needed,
//               you need to alloc seperate memory and cast the memory
//               to a ptr to IWaveFormat. For example,
//                  byte* pBytes = new byte [sizeof (IWaveFormat) + extraBytes] ;
//                  IWaveFormat* pFormat = static_cast<IWaveFormat*>pBytes ;
//
//  For Reference:
//      typedef struct
//      {     
//          WORD  wFormatTag;     
//          WORD  nChannels; 
//          DWORD nSamplesPerSec;     
//          DWORD nAvgBytesPerSec;     
//          WORD  nBlockAlign; 
//          WORD  wBitsPerSample;     
//          WORD  cbSize;           // Size of "extra" bytes attached after
//                                  // this structure for holding compressor
//                                  // specific data.
//      } WAVEFORMATEX; 
// --------------------------------------------------------------------------

class CLASS_DECLSPEC IWaveFormat : public WAVEFORMATEX
{
  public:
    IWaveFormat () ;
    IWaveFormat (WAVEFORMATEX& wavFormat) ;
    IWaveFormat (WORD  wFormatTagIn, 
                 WORD  nChannelsIn,      
                 DWORD nSamplesPerSecIn,  
                 WORD  wBitsPerSampleIn) ;

    IWaveFormat (const IWaveFormat& waveToCopy) ;
    IWaveFormat& operator= (const IWaveFormat& waveToCopy) ;

   ~IWaveFormat () ;

    IWaveFormat* Clone () const ;

    bool IsCompressed ()  const      { return wFormatTag != 1 && cbSize != 0 ; }
    void Dump () const ;

	const byte* GetExtraData ()	const	 { return (reinterpret_cast<const byte*>(this) + sizeof (WAVEFORMATEX))  ; }
	
    // extra data to follow directly after WAVEFORMATEX struct...
    //
    //byte arCompressedData [0] ;  // This is actually cbSize in length
    short extraData ;
};

class CLASS_DECLSPEC AAudioEffect
{
};


class AAudioFilter
{
  public:
    virtual ~AAudioFilter ()  {} ;
	virtual bool   Init 		     (const IWaveFormat* pWaveFormat) { return true ; } ;
	virtual uint32 ProcessData 		 (IAudioPacket* pDataIn, IAudioPacket* pDataOut) = 0 ;
    virtual uint32 CalcProcessedSize (uint32 rawDataSizeInBytesv) = 0 ;
} ;


typedef byte AudioByte ;

class AAudioDecompressor : public AAudioFilter
{
  public:
	AAudioDecompressor ()
		: AAudioFilter()
	{};

	virtual ~AAudioDecompressor ()
	{};

	virtual bool   Init 		      (const IWaveFormat* pWaveFormat) = 0 ;
	virtual bool   Reset 		 	  (IAudioPacket* pCompressedDataPacket) = 0 ;
	virtual uint32 ProcessData 		  (uint32 numBytes, IAudioPacket* pDataOut) = 0 ;
	virtual uint32 ProcessData 		  (IAudioPacket* pDataIn, IAudioPacket* pDataOut) = 0 ;
    virtual uint32 CalcProcessedSize  (uint32 rawDataSizeInBytes) = 0 ;
	virtual uint32 GetSamplesPerBlock () = 0 ;
};



// -- IAudioStream ----------------------------------------------------------
//
//  The IAudioStream class represents a stream of audio data. It is responsible
//  for buffering the data intelligently and handling requests for data made
//  by the audio engine. The audio stream is reference counted so the same
//  audio data can be used simultaneously by multiple audio emitters.
//
//  IAudioStream uses a data accessor object to obtain the audio data which 
//  may come from a variety of data sources (ie. File, Network). Requests for
//  audio data are handled by GetPacket() and PutPacket() and is transfered
//  in audio "packets" represented by IAudioPacket. An IAudioStream object
//  consists of all of the channels (1-mono, 2-stereo) represented by a single
//  data accessor.
//
//  IAudioStream is also responsible for decompressing the audio data if it
//  has been compressed. To do so, it asks the AFormatReader object to create
//  a decompressor object that knows how to decode the specific audio data.
//
//  Note: This class should only be used by the Audio Engine directly; there
//  isn't a need for other objects to use it. IOTW, is should be treated as
//  private to the audio engine and not used by clients directly.
//
//  Note: 
//      There are 3 ways to initially setup and assign audio data to a stream:
//          1) call InitialLoadCache () to load a chuck of memory into the
//             stream's cache. To load another section into the stream's cache
//             call GetPacketAt().
//          2) call LoadAll() to load all of the data into memory.
//          3) if you already have audio data loaded in memory, than call
//             UseAudioData() to have the stream use that data.
//
//
//  Data Flow:
//
//	  ADataAccessor	<--> IAudioStream  <--> IAudioEmitter
//						   |   ^
//						   v   |
//						IAudioCodec 	
//
// --------------------------------------------------------------------------

class CLASS_DECLSPEC IAudioStream
{
  public:
    IAudioStream () ;
   ~IAudioStream () ;

    virtual FileErr_t Open   (const char* pszFilename) ;

	// -- main data request functions...
    virtual bool      GetPacketAt (IAudioPacket& audioPacket, IAudioEmitter* pEmitter, uint32 uRequestedSized) ;
// v--prev
virtual bool GetPacketAt (IAudioPacket& audioPacket, uint32 uOffset, uint32 uRequestedSize) ;
    virtual bool      PutPacket   (IAudioPacket& packetToPut) ;

	// -- Load the audio data into memory...
    virtual bool   	  LoadAll   () ;
    virtual bool   	  InitialLoadCache (float fCacheSizeInSecs, bool bDecompressAll=false) ;
    virtual void   	  UseAudioData (IAudioPacket* pData) ;
	virtual bool 	  DecompressAll () ;

	// -- Accessor methods...
    virtual float  	   GetCacheSizeInSecs () ;
    virtual uint32     GetCacheSizeAsBytes () ;
    const IWaveFormat* GetWaveFormat () const { return m_pWaveFormat ; }
    const char* GetName 	()      const    { return m_strName.c_str(); } 
    const char* GetFilename ()  	const    { return m_pDataAccessor ? m_pDataAccessor->GetFilename() : m_strFilename.c_str() ; }
    uint32 		GetSizeInBytes ()   const    { return m_uDataSizeInBytes ; }
    float  		GetSizeInSecs ()    const    { return float (m_uDataSizeInBytes) / float (m_pWaveFormat->nAvgBytesPerSec) ; }
    bool   		IsOpen		()      const    { return m_pDataAccessor != 0; }
    bool   		IsInMemory 	()      const    { return m_bIsAllInMemory ; }
    bool   		IsCompressed()      const    { return m_bIsCompressed; }
	bool   		IsStereo	()	    const 	 { return (m_pWaveFormat->nChannels > 1 ? true : false); }
    bool   		IsEOD 		()      const    { return m_bEOD ; }

    void   		SetLoadGroupID (uint32 uLoadGroupID)  { m_uLoadGroupID = uLoadGroupID ; }

    virtual void AddRef () ;
    virtual void DecRef () ;

    virtual bool createReaderDecompressorWaveFormat () ;
    IAudioPacket* getRawDataPacket ()		 { return m_pPData ;	}

  protected:
    string               m_strName,
                         m_strFilename ;
    int32                m_nRefCnt ;
    bool                 m_bIsCompressed,
                         m_bIsAllInMemory,
                         m_bEOD ;
    int32                m_uLoadGroupID ;    
    uint32               m_nCacheStartOffset,
                         m_nCacheEndOffset ;  // one PAST last valid byte
    uint32               m_uDataSizeInBytes,
    					 m_uRawDataSizeInBytes ;

    IAudioPacket*        m_pPData ;
    ADataAccessor*       m_pDataAccessor ;
    AFormatReader*       m_pAudioReader ;
    const IWaveFormat*   m_pWaveFormat ;
    AAudioDecompressor*  m_pDecompressor ;
    float                m_fCacheSizeInSecs ;

    // -- info data...
    uint32               m_nNumTimesUsed ;

    friend IAudioEngine ; 
}; // IAudioStream



// -- IAudioEmitter ---------------------------------------------------------
//
//  
//  Notes:
//    Each emitter will be automatically deleted when it finishes playing unless
//    you call DisableAutoDelete ();
//
//    If auto delete is enabled (ie. DisableAutoDelete (false); it's ON by
//    default) the emitter will be deleted whenever the emitter stops playing;
//    either by an explicite call to Stop() or when the emitter has finished
//    playing.
// --------------------------------------------------------------------------

// -- bits for IAudioEmitter::m_uChangedFlags...
const uint32 kPositionChanged         =  1 ;
const uint32 kVelocityChanged         =  2 ;
const uint32 kOrientationChanged      =  4 ;
const uint32 kAttenuationRangeChanged =  8 ;
const uint32 kConeParametersChanged   = 16 ;
const uint32 kPitchChanged            = 32 ;
const uint32 kVolumeChanged           = 64 ;


class CLASS_DECLSPEC IAudioEmitter
{
  protected:
    // Note: MUST create emitters by calling IAudioEngine::CreateEmitter()
    IAudioEmitter (IAudioSpecificImplementation& audioReceiver, 
                   uint32 uAudioAttributes=kAFX_Default,
                   bool bStream=false) ;
	// Note: call DeleteMe () instead of using the delete operator...

  public:
	virtual void DeleteMe () ;
    virtual bool AttachStream (IAudioStream* pNewStream) ;

    typedef enum { kPlaying, kMuted, kPaused, kStopped } EmitterState_t ;

    // -- Actions...
    //
    virtual bool  Play           (uint32 uNumLoops=-1, float fInitialStartTimeInSecs=0) ;
    virtual bool  Stop           () ;
    virtual bool  Pause          () ;
    virtual bool  Resume         () ;
    virtual bool  Mute           () ;
    virtual bool  UnMute         () ;
    virtual bool  Rewind         () ;

    // -- Get accessors
    //
    void           GetPosition    (AudioPosition_t& ptPosition) const ;
    void           GetOrientation (AudioVector_t& vecDirection) const ;
    void           GetVelocity    (AudioVector_t& vecVelocity)  const  { vecVelocity = m_vecVelocity; }
    float          GetPitch       () const ;    
    float          GetVolume      () const ;
    void           GetAttenuationRange (float& fMin, float& fMax) const { fMin = m_fInnerRange ; fMax = m_fOuterRange ; } ;
    bool           GetAttenuationState () const  { return m_bUseAttenuation; }
    void           GetConeParameters   (uint32& uInsideAngleInDegs, uint32& uOutsideAngleInDegs, float& fConeOutsideVolumeInDB) ;

    // -- NOTE: Thses do NOT take into account the number of loops an emitter may have to play...
    virtual float  GetSecondsPlayed  ()  const ;
    virtual float  GetRemainingTime  ()  const ;
    virtual uint32 GetRemainingBytes ()  const ;

    float          GetBreakoutTime ()     const  { return m_fBreakoutDelta ; }
    EmitterState_t GetState () const ;

    bool           Is3D         ()      const   { return m_bitsDesiredAttributes & kAFX_3D; }
	
    bool           IsEOD        ()      const   { return m_bEOD ; }
    bool   		   IsCompressed ()      const   { return m_pAudioStream->IsCompressed (); }
	bool   		   IsStereo ()			const   { return (m_pAudioStream->IsStereo ()) ; }
    bool           IsAllInMemory()      const   { return m_pAudioStream->IsInMemory() ; }
    bool           IsLooping    ()      const   { return (m_uNumLoops != 1); }
    bool           IsDopplerShifted ()  const   { return m_bitsDesiredAttributes & kAFX_Doppler ? 1 : 0; }
    //bool           IsForcePanning3d  () const   { return m_bForcePanning3d ; }
    bool           IsDeleteWhenOutOfRange () const   { return m_bDeleteWhenOutOfRange ; }
    virtual bool   IsOutOfRange (AudioPosition_t& ptListener) ;
    bool           HasCone ()             const  { return m_uConeInsideAngleInDegs != 360; }

    int32          GetSoundEntityID ()   const  { return m_nEntityID ; }
    int32          GetUserData ()        const  { return m_nUserData2 ; }
    int32          GetSoundChannel ()    const  { return m_nSoundChannel ;}    
    const char*    GetName ()            const  { return m_pAudioStream->GetName (); }

    AudioPriority_t GetPriority ()         { return m_ePriority; }
    const IWaveFormat* GetWaveFormat ()   const  { return m_pAudioStream->GetWaveFormat() ; }
    uint32         GetSizeInBytes ()      const  { return m_pAudioStream->GetSizeInBytes() ; }
    float          GetSizeInSecs ()       const  { return m_pAudioStream->GetSizeInSecs () ; }
    int32          GetAssignedChannel ()  const  { return m_nAssignedToChannel ; }

    // -- Set methods...
    //
    virtual void   SetPosition         (const AudioPosition_t& ptPosition) ;
    virtual void   SetOrientation      (const AudioVector_t& vecDirection) ;
    virtual float  SetPitch            (float fPitch) ;
    virtual float  SetVolume           (float fVolume) ;
    virtual bool   SetAttenuationRange (float fMinFront, float fMaxFront) ;
    void           SetAttenuationState (bool bEnabled)  { m_bUseAttenuation = bEnabled; }
    virtual void   SetRandomAmbient    (float fMinDelay, float fMaxDelay) ;
    virtual void   SetConeParameters   (uint32 uInsideAngleInDegs, uint32 uOutsideAngleInDegs, float fConeOutsideVolumeInDB) ;
    virtual void   SetVelocity         (AudioVector_t& vecVelocity)  ;

    void           SetBreakoutTime (float fSecs) ;   // play for an additional 'fSecs' 
    void           SetDeleteWhenOutOfRange (bool bDelete) { m_bDeleteWhenOutOfRange = bDelete ; }
    void           SetEntityID (int32 n)              { m_nEntityID = n; }    
    void           SetSoundTypeID (int32 n)           { m_nSoundChannel = n; }    

    void           SetUserData (int32 n)              { m_nUserData2 = n; }    
    void           DisableAutoDelete (bool bTrueToDisable=true)    { m_bAutoDelete = !bTrueToDisable; }
    bool           IsAutoDelete () const              { return m_bAutoDelete; }

    uint32         GetChangedFlags () const           { return m_uChangedFlags; }
    void           SetChangedFlags (uint32 flags)     { m_uChangedFlags = flags; }

    IAudioStream*  GetStream () const       	      { return m_pAudioStream; }

    int  m_nLoopCnt ;         // Used to tell us when a looping sound has gone out of the PHS


	// -- AGH this SHOULD BE protected but if it is, then any classes derived from IAudioEmitter()
	//	  will NOT be able to cal delete pEmitter.
    virtual ~IAudioEmitter () ;

  protected:
    void                setState (EmitterState_t state) ;

    // -- data
    //
    uint32                m_uChangedFlags ;

    IAudioStream*         m_pAudioStream ;
    AudioPosition_t       m_ptPosition ;
    AudioVector_t         m_vecDirection,
                          m_vecVelocity ;

    uint32                m_bitsDesiredAttributes ;
    AudioBufferType_t     m_eBufferType ;
    AudioBufferLocation_t m_eBufferLoc ;
    AudioClass_t          m_eClass ;
    AudioPriority_t       m_ePriority ;

    uint32                m_uNextStreamOffset ;   // offset into stream of the next byte to be read
    float                 m_fRemainingTimeInSecs ; // number of secs remaining for the emitter
    uint32                m_uStartTimeInMSecs ;    // time emitter started playing

    IAudioSpecificImplementation& m_AudioImp ;

    bool                  m_bEOD ;
    bool                  m_bTryToStream ;
    bool                  m_bDeleteWhenOutOfRange ;

    EmitterState_t        m_eState ;
    float                 m_fVolume ;
    float                 m_fPitch ;
    uint32                m_uNumLoops ;     // Number of loops remaining to play emitter

    uint32                m_uConeInsideAngleInDegs,
                          m_uConeOutsideAngleInDegs ;
    int32                 m_fConeOutsideVolumeInDB ;

    bool                  m_bUseAttenuation ;
    float                 m_fInnerRange, 
                          m_fOuterRange ;
    float                 m_fMinDelay, 
                          m_fMaxDelay ;

    bool                  m_bAutoDelete ;
    int32                 m_nEntityID, m_nUserData2 ;
    int32                 m_nSoundChannel ;

    int                   m_nAssignedToChannel ;
    float                 m_fBreakoutDelta ;

    AAudioDecompressor*  m_pDecompressor ;

friend IAudioStream ; // to get/set m_pDecompressor
    friend IAudioEngine ;
}; // IAudioEmitter



// -- IAudioMonitor ----------------------------------------------------------
//
//  Serves to collect various audio engine statistics like the amount of
//  memory currently allocated and avg processing time.
// --------------------------------------------------------------------------

class CLASS_DECLSPEC IAudioMonitor
{
  public:
    IAudioMonitor (const IAudioEngine& ae) ;
   ~IAudioMonitor () ;

    uint32  m_uNumEmitters,        // Number of IAudioEmitters created
            m_uMaxNumEmitters,
            m_uNumCurrStreams,
            m_uMaxNumStreams,
            m_uTotalMemoryAllocated,   // Current amount of mem allocated
            m_uMaxMemoryAllocated,     // Hi water mark for memory
            m_fAvgProcessingTime ;
}; 


// ----------------------------  IAudioChannel  -----------------------------------

class CLASS_DECLSPEC IAudioChannel
{
  public:
    IAudioChannel () ;    
   ~IAudioChannel () ;    

    IAudioEmitter* m_pEmitter ;
    int32          m_nIndex ;   // channel index [0..m_nActiveChannels)
} ; // IAudioChannel



// -- IAudioEngine ----------------------------------------------------------
//
//  IMPORTANT: If any methods need to be called by an audio implementation
//  from a dll, then that method MUST be declared as virtual so a ptr to the
//  method and NOT the actual definition can be used.
// --------------------------------------------------------------------------

class CLASS_DECLSPEC IAudioEngine
{
  public:
    IAudioEngine () ;
   ~IAudioEngine () ;

    bool Initialize (IAudioDeviceCaps* pAudioDev, 
                     int 	hWnd, 
                     const  IWaveFormat& waveFormatForOutput, 
                     bool 	bAllowSoundsOnLossOfFocus,
                     int 	nNumChannels = 16,
                     int 	nNumTrue3dChannels=8,
                     float	fOneUnitInMeters=1.0f,
                     uint32 uUseFlags=0,
                     IAudioSpecificImplementation* pAudioDevice=0,
                     float  fSubmissionSizeInSecs=0.f);

    IAudioSpecificImplementation* CreateImplementation (const char* pszPluginName) ;

    bool Shutdown () ;
    bool StartPlaying () ;
    void BeginFrame () ;       // call at the start of a new frame; will update audio emitter values
    bool Connect    () ;       // grabs the audio device (ie. call upon WM_ACTIVATE TRUE)
    bool DisConnect () ;       // releases the audio device (ie. call upon WM_ACTIVATE FALSE
    bool Pause () ;
    bool Resume () ;

    IAudioStream* Load (const char* pszFilename, LoadMethod_t eLoadAs=kLoadChooseBest) ;
	void Unload (const char* pszFilename) ;

	// -- Create emitter from a filename and passed in parameters...
    bool CreateEmitter  (IAudioEmitter** ppAudioEmitterOut, 
                         const char*     pszFilename,
                         uint32          uAudioAttributes,
                         AudioClass_t    eAudioClass,
                         LoadMethod_t 	 eLoadAs=kLoadAllIntoMem) ;

	// -- Create emitter from a stream and passed in parameters...
    bool CreateEmitter  (IAudioEmitter** ppAudioEmitterOut, 
                         IAudioStream*   pAudioStream,
                         uint32          nAudioAttributes,
                         AudioClass_t    eAudioClass,
                         LoadMethod_t eLoadAs=kLoadAllIntoMem) ;

	// -- Create emitter from a file and a sounddef...
	bool CreateEmitter  (IAudioEmitter** ppAudioEmitterOut, 
	                     const char*	 pszSoundFilename,
	                     const char*     pszSoundDefName) ;
                         
	// -- Create emmitter from a stream and a sound def...                         
    bool CreateEmitter  (IAudioEmitter** ppAudioEmitterOut, 
                         IAudioStream*   pAudioStream,
                         const char*     pszSoundDefName) ;


    bool ChangeOutputFormat (const IWaveFormat& newFormat) ;

    bool 		   UseSoundDefFile (const char* pszFilename) ;
    ISoundDef*     GetSoundDef (const char* pszDefName) const ;
    ISoundDef*     GetSoundDef (int nIndex)             const ;
	bool		   AddSoundDef (ISoundDef* pDef) ;

    int32          GetNumActiveEmitters () ;
    IAudioEmitter* GetActiveEmitter     (int32 index) ;

    // -- Audio Destination...
    //
    IAudioSpecificImplementation* GetImplementation () ;
    IAudioSpecificImplementation* ReplaceImplementation (IAudioSpecificImplementation* pNewImp,
                                                         int nActiveChannels, int nNumTrue3dChannel) ;

    // -- Listener interface...
    //
    virtual void GetListenerOrientation (AudioVector_t& vecDirection, AudioVector_t& vecUp, AudioVector_t& vecRight) ;
    virtual void GetListenerPosition    (AudioPosition_t& ptPosition) ;

    void SetListenerOrientation (const AudioVector_t& vecDirection, const AudioVector_t& vecUp, const AudioVector_t& vecRight) ;
    void SetListenerPosition    (const AudioPosition_t& ptPosition) ;
    void SetListenerPosition    (float x, float y, float z) ;

    float SetMinStreamSizeInSecs   (float fMinSize) ;
	float SetStreamCacheSizeInSecs (float fStreamCacheSize) ;

    // -- Stream management...
    //
    void FreeLoadGroupID    (uint32 nGroupID) ;
    void FreelAllButGroupID (uint32 nGroupID) ; 
    void DeleteAllEmitters  () ;

    // -- Emmitter management...
    void EmitterFinished (IAudioEmitter* pEmitter) ;

    // -- Method MUST be called whenever an app is managing an emitter itself (ie. AutoDelete 
    //    is disabled) is done using the emitter. This effectively tells the audio engine
    //    to remove any reference to the emitter it may have.
    void RemoveEmitter (IAudioEmitter* pEmitter) ;

    // -- Environment settings...
    //
    void  SetMasterVolume (float fMasterVol) ;
    float GetMasterVolume () const                  { return m_fMasterVolume; }

    bool IsEAXSupported () ;
    void SetReverbPreset (int nPresetID) ;
    void SetReverbState  (bool bEnable) ;
    void SetReverbParms  (float fVolume, float fDecayTime, float fDamping) ;
    virtual void GetReverbParms  (bool& bEnabled, float& fVolume, float& fDecayTime, float& fDamping) ;

    void SetSpeakerConfiguration (SpeakerConfig_t speakerConfig) ;
    void SetStereoSpeakerAngle   (float fDegrees) ;

    void SetPrimaryCacheSizeMS (uint32 uSizeInMS) ;

    bool CalcSpacialization (IAudioEmitter* pEmitter, 
                             AudioPosition_t& posListener, AudioVector_t& vecListenerForward, 
                             AudioVector_t& vecListenerUp, AudioVector_t& vecListenerRight,
                             int& nLeftOut, int& nRightOut) ;

    typedef enum { kStopped, kPlaying, kPaused } AudioEngineState_t ;

    bool IsPaused ()            { return m_eState == kPaused; }

    const IWaveFormat& GetWaveFormat ()     { return m_WaveFormat ; }
    const char* GetImplementationName() ;

    int  SetDebugLevel (int nLevel) ;
    void DumpStats () ;
    void DumpState () ;


    int   GetPlugins          (const char* pszPluginFileNames) ;
    void* CreatePluginObject  (const char* pszPluginName) ;
    void  DestroyPluginObject (void* pObject) ;

    // -- Iterate the list of plugins...
    int   GetNumPlugins      () ;
    AudioPlugin* GetPlugin   (int indx) ;

    // -- These are all of the methods that need to be accessed from an Audio Impelentation DLL...
    //
    virtual void AddToPlayQ (IAudioEmitter& rEmitter) ; // Called by IAudioEmitter when it 
                                                        // needs to play with itself.
    virtual void AddToFinishedList (IAudioEmitter* pEmitter) ;

    virtual AAudioDecompressor* CreateDecompressor (const IAudioStream* pStream) ;

	
  protected:

    // -- private methods  ----------------------------------------------------

    void           deleteAudioImp () ;
    bool           loadStream (IAudioStream* pAudioStream, LoadMethod_t eLoadAs) ;
    void           startAnyEmitters () ;
    IAudioChannel* assignChannel (IAudioEmitter& rEmitter);
    void           abortEmitter (IAudioEmitter* pEmitter) ;
    void           purgeFinishedEmitters () ;
    void           stopOutOfRangeEmitters () ;
    IAudioStream*  findAudioStreamFor (const char* pszFilename) ;

    // -- Channel management...
    IAudioChannel* findFreeChannel   (IAudioEmitter& rEmitter) ;
    IAudioChannel* findChannelToBump (IAudioEmitter& rEmitter, uint32 uDoNotInterruptUnlessAtLeastHasPlayed) ;
    bool           bindTogether      (IAudioChannel* pChannel, IAudioEmitter& rEmitter) ;
    void           handleFillRequest (int nEventNum) ;
    void           freeUpChannel (int i) ;

    bool removeStreamFromList  (IAudioStream* pStream) ;

    // -- Data ----------------------------------------------------------------

    IPtrList       m_lstStreams, 
                   m_lstFinishedEmitters,
                   m_lstActiveEmitters, 
                   m_lstPriorityQ ;         // Emitters waiting to be played

    bool               m_bConnected ; 
    IAudioSpecificImplementation* m_pAudioImp ; // can only have ONE Destination
    IWaveFormat        m_WaveFormat ;           // Output wave format

    AudioEngineState_t m_eState ;
    bool            m_bMasterReverb ;           // true if reverb is enabled
    float           m_fReverbIntensity,
                    m_fReverbDecayTime,
                    m_fReverbDamping ;
    uint32          m_uUseFlags ;
    uint32          m_uBufferSizeInMS ;

    AudioPosition_t m_ptPosition ;              // The Destination's location in the World.
    AudioVector_t   m_vecDirection,             // Heading of the Destination
                    m_vecUp,                    // Up vector of the Destination
                    m_vecRight ;
    bool            m_bUseLeftHandedCoords ;    // false if we are using a right handed system
    float           m_fSpeedOfSound ;           // the speed that sound travels thru the "medium";
                                                // used for calculating Doppler effects.
    HWND            m_hWnd ;
    int32           m_nActiveChannels ;         // Number of active channels
    int32           m_nNumTrue3dChannel ;       // All channels < than this will play in true 3d, others simulated 3d
    IAudioChannel   m_arChannels [kMaxNumberOfChannels] ;

    float           m_fPrimaryCacheSizeInMS ;   // Amt to cache before sending to card.
    bool            m_bAllowSoundsOnLossOfFocus ;
    float           m_fOneUnitInMeters ;

    float           m_fMasterVolume ;
    IAudioDeviceCaps*   m_pAudioDevCaps ;

    SpeakerConfig_t m_eSpeakerConfig ;
    float           m_fSpeakerSeperationInDegrees ;

    ISoundDefs*     m_pSoundDefs ;
    uint32          m_uDisconnectTimeInMS ; 

	AudioDataConfigParams_t m_dataConifgParams ;
	int				m_nDebugLevel ;

    // -- Monitor data...
    uint32 m_uMaxEmitters,
           m_uMaxStreams,
           m_uTotalMemoryAllocated,
           m_uMaxMemoryAllocated ;
    float  m_fAvgProcessingTime ;

    AudioPluginMgr*  m_pAPMgr ;

    // -- Audio Implementation data -----------------------------------------
    //
    HANDLE   m_hAudioImpDll ;

    IPtrList m_lstAudioPlugins ;

    // -- Friends of AudioEngine --------------------------------------------
    friend IAudioMonitor ;
    friend DWORD WINAPI AudioEngineThreadEntryPoint (LPVOID pWorkContext)  ;
    friend void IAudioStream::DecRef () ; // to call removeStreamFromList()
    friend bool IAudioEmitter::Stop () ;
}; // IAudioEngine


// ----------------------------  Inlines  -----------------------------------

inline int32 IAudioEngine::GetNumActiveEmitters ()
{
    return m_lstActiveEmitters.GetCount () ;
}


inline IAudioEmitter* IAudioEngine::GetActiveEmitter (int32 index)
{
    //PRECOND_RANGE (index, 0, GetNumActiveEmitters() - 1) ;
    IAudioEmitter* pEmitter ;
    IPOSITION pos = m_lstActiveEmitters.GetHeadPosition () ;
    int i=0;

    while (pos)
    {
        pEmitter = (IAudioEmitter*)m_lstActiveEmitters.GetNext (pos) ;
        if (i++ == index)
            return pEmitter ;
    }

    return 0;
}


// ----------------------------  Globals  -----------------------------------

// -- Define common wave formats globally here...
extern const IWaveFormat kWaveFormat_11kHz_Mono ; 
extern const IWaveFormat kWaveFormat_22kHz_Mono ; 
extern const IWaveFormat kWaveFormat_44kHz_Mono ; 

extern const IWaveFormat kWaveFormat_11kHz_Stereo ;
extern const IWaveFormat kWaveFormat_22kHz_Stereo ;
extern const IWaveFormat kWaveFormat_44kHz_Stereo ;


extern IAudioEngine AudioEngine ;   // One and only audio engine


// ---------------------------- Prototypes ----------------------------------


#endif // _IAUDIO_H
// ==========================================================================
//                              End of File
// ==========================================================================
