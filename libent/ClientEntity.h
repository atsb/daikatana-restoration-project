// ClientEntity.h: interface for the CClientEntity class.
//
//  1999 Ion Storm, LLP
//
//  Client Entities:
//  To reduce net traffic, improve the maintainability and 
//  extensibility of the code, and otherwise make Daikatana kick
//  extreme ass, the client has been granted some brains!
//
//  First and foremost, the original quake2 design uses the client
//  like a dumb terminal. Client Entities are to the quake2 engine
//  what X-Windows is to terminals --let the client to as much work
//  as it can, use the network as little as possible!
//
//  How It Works:
//  The server creates it's own notion of what the entity should
//  be and do --it will track physics and otherwise do what it
//  always has in the past, EXCEPT spam the network with extraneous
//  messages. The server creates it's entity, tags it as a CLIENT_ENTITY
//  and proceeds to send an svc_cliententity message specifying the 
//  type of entity that needs to be created on the other side of the
//  wire.
//
//  The client receives this message, then tells the ClientEntityManager
//  that a message has been received. The entity manager is responsible
//  for forwarding the right messages to the right ClientEntity 
//  derivitives, or creating new instances of client ent classes.
//  
//  Each entity then further processes the messages as appropriate
//  for it's type. E.g. a generic projectile will receive an origin,
//  velocity and model index so the client can render it. Rather
//  than send a message every time the goddam origin changes, the
//  server only has to send it's usual frame update messages and
//  a notification message that the projectile should be removed or
//  otherwise altered.
//
//  The client, meanwhile, only does its rendering for cleint entities
//  when it receives frame updates from the server --voila! the entities
//  *behave* as if they were getting their physics from the server, 
//  without the overhead of 30-80 byte messages every freaking frame!
//
//  There are also special effects, and entities which can exist 
//  for an arbitrary period and run their physics WITHOUT frame 
//  updates (ambient entities, certain special FX like skyboxes, etc.)
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTENTITY_H__17EE0AE5_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
#define AFX_CLIENTENTITY_H__17EE0AE5_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// #include "ClientEntityManager.h"
#ifndef dllExport
#define dllExport __declspec( dllexport )
#endif

#include "dk_point.h"

// limits for this entity class
#ifndef LIMIT_CLIENTENTITY
#define LIMIT_CLIENTENTITY    256
#endif

struct sizebuf_s;
class CClientEntityManager;
// class CVector;
////////////////////////////////////////////////////////////////////////
//  CClientEntity
//  
//  A lot of this can probably be relocated to derived classes to 
//  decrease the overall memory load. There are way too many functions
//  here.
//
class CClientEntity  
{
public:
	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_ENTITY); }
	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
	int max_entities;
	virtual CClientEntity * Alloc();
	int class_size;
	virtual void RunFX();
	short run_fx;
    ////////////////////////////////////////////////////////////////////
    //**** NETWORK
	virtual void            ReadDefaultsFromServer(struct sizebuf_s *msg);
    virtual void            Destroy(struct sizebuf_s *msg); // tag this sucker as free
	unsigned int            server_index;       // server's id for this instance
	virtual void            FinishLinkToClient();   // for multicast/unicast variations
	virtual void            SendParametersToClient();   // class specific parms
	virtual void            LinkToClient(); // server execs this to send data
	virtual void            ReadParametersFromServer(struct sizebuf_s *msg);
	virtual void            ReceiveServerMessage(int cmd, struct sizebuf_s *msg);
	short                   run_with_server_frames; // exec physics only when server sends data?
	virtual void            SendDefaultsToClient();
	virtual void            Notify();
	virtual void            Notify(struct sizebuf_s *msg);
	virtual CClientEntity   *Spawn(struct sizebuf_s *msg);

    ////////////////////////////////////////////////////////////////////
    //**** Hooks for engine entities
    struct edict_s          *server_ent;    // server side
    struct entity_s         *client_ent;   // client side

    ////////////////////////////////////////////////////////////////////
    //**** Initialization and management
	virtual CClientEntity   *Spawn();
	void                    InitBase();
	virtual void            Destroy();
	unsigned int            index;
	virtual void            ReInitialize();
    CClientEntityManager    *manager;
	unsigned short          class_index;

    ////////////////////////////////////////////////////////////////////
    //**** Physics
	class CVector                 origin;
    class CVector                 old_origin;
	class CVector                 velocity;
    class CVector                 angles;
    class CVector                 old_angles;
	class CVector                 avelocity;
	void CL_StillFrame();
	virtual void            CL_RunFrame();
	virtual void            SV_RunFrame();

    ////////////////////////////////////////////////////////////////////
    //**** List Management --sacrafice some memory for speed
    //
    CClientEntity *head;
    CClientEntity *Next;
    short   active;
    ////////////////////////////////////////////////////////////////////
    //**** Constructor/Destructor
    CClientEntity();
    virtual ~CClientEntity();
};

#endif // !defined(AFX_CLIENTENTITY_H__17EE0AE5_AC16_11D2_A7E1_0000C087A6E9__INCLUDED_)
