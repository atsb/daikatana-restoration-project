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
#define MAX_CLENTITIES  64

#ifndef dllExport
#define dllExport __declspec( dllexport )
#endif

typedef enum EntTypes_e
{
    // ENT_CClientEntity,   // never really used, was just for test
    // ENT_CProjectile,
    ENT_CIonBlast,
    ENT_CRocket,
//    ENT_CTrackLight,      // tied to other entities, allocate with relevant ents
    ENT_TotalTypes
} EntTypes;

#define SpawnCE(x,y)   (x *)gstate->ClientEntityManager->EntityList[y]->Spawn()


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
    void    (*V_AddParticle)    (CVector &org, int color, float alpha, particle_type_t type);
    void    (*V_AddComParticle) ( CVector &org, int color, float alpha, particle_type_t type, int rotation_vec, float pscale, short flags, CVector &color_rgb);
    void    *(*S_RegisterSound) (const char *snd_name);
    struct particle_s *(*CL_SpawnParticle)(void);
    int     (*CL_ModelIndex)    (char *name);
} net_client_t;

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

class dllExport CClientEntityManager  
{
public:
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
    CClientEntity           *EntityList[MAX_CLENTITIES];  // main client-side entity list
    // sorted by server index
    CClientEntity           *Entity_ServerIndex[MAX_CLENTITIES];  
    // puts the "head" of each type in this list, expedites searches by 
    // type (see ReceiveServerMessage)
    CClientEntity           *EntityTypes[ENT_TotalTypes];
    //
    ////////////////////////////////////////////////////////////////////

};

#endif // !defined(AFX_CLIENTENTITYMANAGER_H__17EE0AE9_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
