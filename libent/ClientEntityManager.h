// ClientEntityManager.h: interface for the CClientEntityManager class.
//
//  1999 Ion Storm LLP
//
//  Logic:
//      See ClientEntityManager.cpp for notes on the entity manager
//      implementation.
//
//  Notes:
//      The CClientEntity arrays are arrays of pointers. The 
//      application allocates and assigns this stuff up front, and the
//      real addresses for entityes never ever ever moves, so there
//      is little chance of memory corruption or slowdowns during
//      allcations. A common set of routines are used throughout the
//      manager code, and access to derivatives are simply "downcasts"
//      from the CClientEntity base class. To a C coder (like myself)
//      this looks like dangerous code. In C++ it works just fine. 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTENTITYMANAGER_H__17EE0AE9_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_CLIENTENTITYMANAGER_H__17EE0AE9_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "dk_shared.h"
// #include "Projectile.h"
// #include "IonBlast.h"
#include "ClientEntity.h"
#include "libclient.h"

#define CONTEXT_SERVER  1
#define CONTEXT_CLIENT  2
#define MAX_CLENTITIES  1024

///////////////////////////////////////////////////////////////////////
//  **** Entity Type Enumeration
//  Add your new entity class HERE! Also add a REGISTER_ENTITY_CLASS()
//  call to the constructor for this class!
//
typedef enum EntTypes_e
{
    ENT_CIonBlast,
    ENT_CRocket,
    ENT_CHUD,
    ENT_CHUDCSpeeds,
    ENT_CFadeEnt,
    ENT_CTrackLightBlinking,
    ENT_TotalTypes
} EntTypes;
///////////////////////////////////////////////////////////////////////

// quick defines to expedite/simplify new entity allocation
#define SpawnCE(x,y)    (x *)gstate->ClientEntityManager->EntityTypes[y]->Spawn()
#define SpawnLocal(x,y) (x *)manager->EntityTypes[y]->Spawn()

typedef struct net_client_s
{
	void	(*MultiCast)        ( CVector &origin, int to);
	// void	(*UniCast) (edict_t *ent, int reliable);
	// void	(*WriteChar) (int c);
	void	(*WriteByte)        (struct sizebuf_s *, int c);
	void	(*WriteShort)       (struct sizebuf_s *, int c);
	void	(*WriteLong)        (struct sizebuf_s *, int c);
	void	(*WriteFloat)       (struct sizebuf_s *, float f);
	void	(*WriteString)      (struct sizebuf_s *, const char *s);
	void	(*WritePosition)    (struct sizebuf_s *, CVector &pos);	
	void	(*WriteDir)         (struct sizebuf_s *, CVector &pos);
	void	(*WriteAngle)       (struct sizebuf_s *, float f);
    void    (*Com_Printf)       (char *fmt, ...);
    int     (*ReadChar)         (struct sizebuf_s *);
    int     (*ReadByte)         (struct sizebuf_s *);
    int     (*ReadShort)        (struct sizebuf_s *);
    int     (*ReadLong)     (struct sizebuf_s *);
    float   (*ReadFloat)        (struct sizebuf_s *);
    char    *(*ReadString)      (struct sizebuf_s *);
    char    *(*ReadStringLine)  (struct sizebuf_s *);
    float   (*ReadCoord)        (struct sizebuf_s *);
    void    (*ReadPos)          (struct sizebuf_s *, CVector &);
    float   (*ReadAngle)        (struct sizebuf_s *);
    float   (*ReadAngle16)      (struct sizebuf_s *);
    void    (*ReadDir)          (struct sizebuf_s *, CVector &);

    // rendering and physics support
    void    (*V_AddEntity)      (struct entity_s *ent);
    void    (*V_AddLight)       (CVector &org, float intensity, float r, float g, float b);
    void    (*V_AddParticle)    (CVector &org, int color, float alpha, particle_type_t type, float);
    void    (*V_AddParticleRGB) (CVector &org, int color, float alpha, particle_type_t type, CVector &color_rgb, float);
    void    (*V_AddComParticle) ( CVector &org, int color, float alpha, particle_type_t type, float pscale, short flags, CVector &color_rgb);
    struct beam_s	*(*beam_AllocBeam)  (void);
    struct beamSeg_s *(*beam_AllocSeg) (void);
    void    *(*S_RegisterSound) (const char *snd_name);
    struct particle_s *(*CL_SpawnParticle)(void);
    int     (*CL_ModelIndex)    (char *name);
    void*   (*S_StartSound) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float dist_min, float dist_max); 
    void*   (*S_StartSoundQuake2) (const CVector &origin, int entnum, int entchannel, void* pAudioStream, float fvol, float attenuation, float timeofs); 
} net_client_t;

///////////////////////////////////////////////////////////////////////
//**** Network Messages
//
typedef enum
{
    CE_SPAWN,
    CE_DESTROY,
    CE_NOTIFY,
    CE_NOTIFY_PARAMETERS,       // defaults changed (e.g. another ReadOarameters takes care of it)
    CE_ACK,                     // acknowledging a packet
    CE_ACK_SPAWN,               // acknowledge spawn
    
    CE_TOTALMESSAGES
} CENetMessages;
//****
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// SOUNDS
//
typedef enum {
    CE_SND_AFTERBURNER,
    CE_SND_COUNT
} CE_SOUND_T;

class CClientEntityManager  
{
public:
	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
	void SoundRegistration(void);
	void WorldSpawn(void);
	float fps;
	class CParticle * FX_FreeParticle;
	int FX_ParticleBounds;
	class CParticle * FX_Particles;
	int entity_count;
	unsigned char show_client_frame_speeds;
	struct centity_s * client_entities;
	int active_count;
    unsigned long bytes_received;
    unsigned long bytes_second;
    unsigned long bytes_time;
	unsigned long received_server_frame;
	unsigned long elapsed_client_frame_time;
	unsigned long last_client_frame_time;
	unsigned long current_time;
	float crand();
	float frand();
	struct client_static_s * ClientStatic;
	class CPhysics                *Physics;
	struct refexport_s      *re;
	void                    RunFrame();
	void                    NetDestroy(sizebuf_s *msg);
	sizebuf_s               *network_message;
	void                    NetSpawn(struct sizebuf_s *msg);
	void                    ClientConnect(struct edict_s *);
	void                    SetClientState(struct client_state_s *cl);
	void                    SetServerState(struct serverState_s *);
	unsigned short          client;
	unsigned short          server;
	void                    ReceiveServerMessage(struct sizebuf_s *);
	CClientEntityManager();
	virtual                 ~CClientEntityManager();
    net_client_s            *NetClient;
    struct serverState_s    *NetServer;
    struct client_state_s   *ClientState;

    void                    *pvSounds[CE_SND_COUNT];

    ////////////////////////////////////////////////////////////////////
    //  To create new derivatives:
    //  
    //  1.  Insert a class index in the EntityTypes enumeration
    //  2.  Add construction code in 
    //      CClientEntityManager::CClientEntityManager() constructor.
    //
    //  Tips:
    //      -   ensure that CL_RunFrame() has an override.
    //      -   Any common physics routines are added to CPhysics. 
    //      -   Special physics can be run during CL_RunFrame, since
    //          times are updated just before and after CL_RunFrame
    //          is called for your class.
    //      -   Obviously, if you use a class that is allocated outside
    //          the manager, you will need to call CL_RunFrame and other
    //          common entry points from a class that was allocated from
    //          within the manager. The CClientEntityManager instance is
    //          responsible for executing most routines.
    //      -   any CClientEntity derivative that was allocated within
    //          the CClientEntityManager can access the manager instance
    //          via the manager member variable. This is usefull for 
    //          common game routines like Con_Printf/Com_Printf, and 
    //          determining whether the network context is server or client
    //
    // sorted by server index
    CClientEntity           *Entity_ServerIndex[MAX_CLENTITIES];  
    // puts the "head" of each type in this list, expedites searches by 
    // type (see ReceiveServerMessage)
    CClientEntity           *EntityTypes[ENT_TotalTypes];
    //
    void                    (*C_Printf) (char *str, ...);    
    class CHUDCSpeeds       *ClentStats;
    ////////////////////////////////////////////////////////////////////

};

#endif // !defined(AFX_CLIENTENTITYMANAGER_H__17EE0AE9_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
