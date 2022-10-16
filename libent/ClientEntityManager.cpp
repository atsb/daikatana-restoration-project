// ClientEntityManager.cpp: implementation of the CClientEntityManager class.
//
//  1999 Ion Storm, LLP
//
//  Logic:
//      The ClientEntityManager is *THE* interface and entity 
//      management routine for ClientEntities in the game. There are
//      at MOST two instances of CClientEntityManager during run time.
//      One for the server and for the client. If the client is 
//      in a deathmatch game, then only the client is instantiated.
//      Likewise, a dedicated server does not spawn a client instance
//      of CClientEntityManager.
//
//      The class has the smarts to know what network context it is
//      executing in.
//
//////////////////////////////////////////////////////////////////////
#include "dk_system.h"
#if _MSC_VER
#include <mmsystem.h>
#endif
#include <stdlib.h>
#include "p_user.h"
#include "client.h"
#include "ClientEntityManager.h"
#include "TrackLight.h"
#include "TrackLightBlinking.h"
#include "CEPhysics.h"
#include "IonBlast.h"
#include "Rocket.h"
#include "TrackFlare.h"
#include "Particle.h"
#include "ParticleFX.h"
#include "HUDCSpeeds.h"
#include "FadeEnt.h"
#include "memmgrcommon.h"


///////////////////////////////////////////////////////////////////////
// REGISTER_ENTITY_CLASS(x,y) where x is the enumerator
// as defined in ClientEntityManager.h and y is the class name.
//
//  Keeping registration tasks to a minimum
#define REGISTER_ENTITY_CLASS(x,y) EntityTypes[x] = new y(); EntityTypes[x]->class_size = sizeof(class y); EntityTypes[x]->manager = this; EntityTypes[x]->class_index = x;
///////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientEntityManager::CClientEntityManager()
{
    int i, csize=0;
    CClientEntityManager *man=this;
    // Initialize type list
    for(i=0;i<ENT_TotalTypes;i++)
    {
        EntityTypes[i] = NULL;
    }
    ///////////////////////////////////////////////////////////////////
    // Build a list of entity types, find the largest class and set
    // our allocation block size to it. 
    //
    // REGISTERENTITY_CLASS(ENUMERATOR);
    // This macro is defined above, and the enumaration is defined
    // in ClientEntityManager.h
    //
    // **** THIS IS WHERE NEW ENTITY CLASSES ARE ADDED ****
    REGISTER_ENTITY_CLASS(ENT_CIonBlast,CIonBlast);
    REGISTER_ENTITY_CLASS(ENT_CRocket,CRocket);
    REGISTER_ENTITY_CLASS(ENT_CHUD,CHUD);
    REGISTER_ENTITY_CLASS(ENT_CHUDCSpeeds,CHUDCSpeeds);
    REGISTER_ENTITY_CLASS(ENT_CFadeEnt, CFadeEnt);
    REGISTER_ENTITY_CLASS(ENT_CTrackLightBlinking, CTrackLightBlinking);
    ///////////////////////////////////////////////////////////////////

    // Initialize members
    ClentStats = NULL;
    show_client_frame_speeds = 0;
    NetClient = (struct net_client_s *)memmgr.X_Malloc(sizeof(struct net_client_s), MEM_TAG_ENTITY);
    server = 0;
    client = 0;
    ClientState = NULL;
    re = NULL;
    bytes_received = 0;
    bytes_second = 0;
    bytes_time = 0;
    ClientState = NULL;
    ClientStatic = NULL;
    current_time = 0;
    elapsed_client_frame_time = 0;
    last_client_frame_time = 0;
    network_message = NULL;
    received_server_frame = 0;
    entity_count = 0;    
    Physics = new CPhysics;
    Physics->manager = this;

    // particle system
    FX_Particles = new CParticle[MAX_PARTICLES];
    FX_FreeParticle = NULL;
    FX_ParticleBounds = 0;  // track the array upper bounds, it gets 
                            // knocked back once the last particle is 
                            // freed, not the theoretical best-case, 
                            // but plenty fast in normal use.


    for(i=0;i<MAX_CLENTITIES;i++)
    {        
        Entity_ServerIndex[i] = NULL;
    }

}

CClientEntityManager::~CClientEntityManager()
{
    int i;
    CClientEntity *ent, *next_ent;
    for(i=0;i<ENT_TotalTypes;i++)
    {
        for(ent=EntityTypes[i]->Next;ent;ent=next_ent)
        {
            next_ent = ent->Next;
            ent->Destroy();
            delete ent;
        }
    }
    delete[] FX_Particles;

    memmgr.X_Free(NetClient);
}


////////////////////////////////////////////////////////////////////////
//  SetServerState(*sstate)
//
//  Description:
//      Initializes the dll local NetServer variable. Routines in the 
//      lib can call serverState functions and refer to data (equiv to
//      "gstate" elsewhere in the code. e.g. NetServer->Com_Printf() 
//      works
//
//      The CClientEntityManager class uses SetServerState to determine
//      it's network context (e.g. client or server). 
//
//      This function is called from dk_ServerState.cpp InitServerStat(). 
//      General extention and use of the new entity system requires zero
//      interaction with this routine.
//
//  Parameters:
//      sstate      a pointer to a serverState structure. 
//
//  Return:
//      void
//
//  BUGS:
//      the quake2 engine just isn't written to accomodate this stuff
//      cleanly. rewrite it.
//
void CClientEntityManager::SetServerState(struct serverState_s *sstate)
{
    if(!sstate)
        return;
    NetServer = (serverState_t *)sstate;

    // Set network context
    this->server = 1;
    this->client = 0;
    this->C_Printf = NetServer->Con_Printf;
    C_Printf("Server side Client Entity Manager enabled\n");
}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  SetCientState(cl)
//
//  Description:
//      Client-side state structure. Sets ClientState to the address of
//      cl for referencing important variables in the game client. 
//      ClientState scope is limited to the library (including any class
//      which includes libclientlocals.h).
//      
//      SetClientState(), like SetServerState(), is called once during
//      execution. Entity implementors should not have to mess with this
//      routine.
//
//  Parameters:
//      cl          the address of the game's client_state_t instance.
//
//  Return:
//      void
//
//  BUGS:
//
void CClientEntityManager::SetClientState(struct client_state_s * cl)
{
    if(!cl) // this better NEVER happen!
        return;

    ClientState = (client_state_t *)cl;

    // Set network context
    this->client = 1;
    this->server = 0;
    this->C_Printf = NetClient->Com_Printf;
    Physics->ClientState = ClientState;
    ClentStats = (CHUDCSpeeds *)EntityTypes[ENT_CHUDCSpeeds]->Spawn();
    // Sound registration

    C_Printf("Client side Client Entity Manager enabled\n");

}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  ClientConnect(ent)
//
//  Description:
//      Called when a client connects to the server. 
//
//  Parameters:
//      ent         client's edict struct .
//  
//  Return:
//      void
//
//  BUGS:
//
void CClientEntityManager::ClientConnect(struct edict_s *ent)
{

}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  ReceiveServerMessage()
//
//  Description:
//      Called by CL_ParseServerMessage when an svc_cliententity message
//      arrives from the server.
//
//      Dispatches server messages intended for CClientEntities 
//      approrpriately.
//
//      See the CENetMessages enumeration in ClientEntityManager.h for
//      details on what messages are recognized.
//
//  Parameters:
//      msg         a pointer to a sizebuf_t containing the packet
//
//  Return:
//      void
//
//  BUGS:
//
void CClientEntityManager::ReceiveServerMessage(struct sizebuf_s *msg)
{
    int cmd, entity_type;
    
    // determine the message type and intended class recipient
    cmd = NetClient->ReadByte(msg);
    entity_type = NetClient->ReadByte(msg);

    // Ensure the net message is reasonably correct
    // ReceiveServerMessage is called from the appropriate entity class,
    // so allocation, handlers and such run in the context of the 
    // derived class, though code sits in the CClientEntity class.
    // The prototype class is guaranteed to exist. It is allocated
    // when the entity manager initializes, and handles spawning classes
    // of the same type. (e.g. there is an IonBlast ent that is created
    // when the manager inits, and if this message is for an IonBlast, it
    // calls IonBlast::ReceiveServerMessage()
    if(entity_type < ENT_TotalTypes)
        EntityTypes[entity_type]->ReceiveServerMessage(cmd, msg);
}
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//  RunFrame()
//  
//  Description:
//      Runs every frame. If this is a client context, executes during
//      client frame. If it is a server context, it runs every server
//      frame. Calls 
//
//  Parameters:
//      none
//
//  Return:
//      void
//
//  BUGS:
//
//  Notes:
//      This is where the frame-by-frame action really happens. 
//
void CClientEntityManager::RunFrame()
{
    int i, fx_count=0;
    int elapsed_server_time, active_ents=0;
    CClientEntity *ent;
    CVector rgb(255,255,255);
    
    if(!this)
        return;

    if(!server && !client)
        return;

    // Initialize preframe physics for calculations (e.g. time)
    Physics->PreFrame();

    current_time = Physics->current_time+10; 

	// jar 990304 - particles are extremely hungry for CPU, if the client is crunching, don't run 'em or the other effects.
//	if(Physics->elapsed_time < 100) // getting better than 10 FPS? run it
//	{
		if(client)
		{
			elapsed_client_frame_time = Physics->current_time - last_client_frame_time;
			elapsed_server_time = current_time - received_server_frame;
			for(i=0;i<FX_ParticleBounds && i < MAX_PARTICLES;i++)
			{
				if(FX_Particles[i].active)
				{
					FX_Particles[i].FXManager->Physics(&FX_Particles[i]);
					fx_count++;
				}
			}
		}
		if(active_count)
		{
			active_count = 0;
			// Entities
			for(i=0;i<ENT_TotalTypes;i++)
			{
				for(ent=EntityTypes[i]->Next;ent;ent=ent->Next)
				{
					if(!ent || !ent->active)
						continue;
					active_count++;
					if(server)
					{
						ent->SV_RunFrame();
					} else if (client) {
						if(ent->run_with_server_frames && elapsed_server_time < 1000)
						{
							ent->CL_RunFrame();
						} else {
							ent->CL_StillFrame();
						}
					}
					ent->old_origin = ent->origin;
				}
			}
		}
	// }

    Physics->PostFrame();
    if(client && show_client_frame_speeds)
    {
        if(ClentStats->last_stat_time < current_time)
        {
            ClentStats->elapsed_time = Physics->last_frame_time - Physics->current_time;
            ClentStats->active_ents = active_count;
            ClentStats->allocated_ents = entity_count;
            ClentStats->active_particles = fx_count;
            ClentStats->last_stat_time = current_time + 500;
        }
    }
    active_count+=fx_count;
}
//
////////////////////////////////////////////////////////////////////////

float CClientEntityManager::frand()
{
	return (rand()&32767)* (1.0/32767);
}

float CClientEntityManager::crand()
{
    return (rand()&32767)* (2.0/32767) - 1;
}


void CClientEntityManager::WorldSpawn()
{

}

void CClientEntityManager::SoundRegistration()
{
    pvSounds[CE_SND_AFTERBURNER] = NetClient->S_RegisterSound("e1/we_sidewinderaft.wav");

}
