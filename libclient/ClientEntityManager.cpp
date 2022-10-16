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
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include "ClientEntityManager.h"
#include "libclientlocals.h"
#include "TrackLight.h"
#include "CEPhysics.h"
#include "IonBlast.h"
#include "Rocket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientEntityManager::CClientEntityManager()
{
    int i;

    NetClient = (struct net_client_s *)malloc(sizeof(struct net_client_s));
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

    Physics = new CPhysics;
    Physics->manager = this;

    // populate the entity list with ents
    for(i=0;i<MAX_CLENTITIES;i+=ENT_TotalTypes)
    {
        EntityList[i + ENT_CIonBlast]=new CIonBlast;
        EntityList[i + ENT_CRocket]=new CRocket;
    }   

    for(i=0;i<MAX_CLENTITIES;i++)
    {
        EntityList[i]->array_index = i;
        EntityList[i]->manager = this;
        Entity_ServerIndex[i] = NULL;
        
    }   
    
    // now initialize types and head info
    for(i=0;i<ENT_TotalTypes;i++)
    {
        EntityTypes[i]=EntityList[i];
        EntityTypes[i]->InitManager(this);
    }
}

CClientEntityManager::~CClientEntityManager()
{
    free(NetClient);
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
    int i; 
    if(!sstate)
        return;
    NetServer = (serverState_t *)sstate;

    // Set network context
    this->server = 1;
    this->client = 0;
    for(i=0;i<MAX_CLENTITIES;i++)
    {
//        EntityList[i]->SetNetState();
		EntityList[i]->ReInitialize();
    }

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
    int i;
    if(!cl) // this better NEVER happen!
        return;

    ClientState = (client_state_t *)cl;

    // Set network context
    this->client = 1;
    this->server = 0;
    Physics->ClientState = ClientState;
    for(i=0;i<MAX_CLENTITIES;i++)
    {
//        EntityList[i]->SetNetState();
		EntityList[i]->ReInitialize();	// set models and other functions (e.g. precache shit)
    }

    NetClient->Com_Printf("Client-Side ClientEntityManager Enabled\n");
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
//      ent         another (ech) void * for casting to an entity.
//  
//  Return:
//      void
//
//  BUGS:
//      yeah, another fucking void *
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

void CClientEntityManager::RunFrame()
{
    int i, fx_count=0;
    int elapsed_server_time, active_ents=0;
    
    if(!this || !active_count)
        return;

    if(!server && !client)
        return;

    // Initialize preframe physics for calculations (e.g. time)
    Physics->PreFrame();
    current_time = Physics->current_time+10; 
    if(client)
    {
        elapsed_client_frame_time = Physics->current_time - last_client_frame_time;
        elapsed_server_time = current_time - received_server_frame;
    }
    if(Physics->elapsed_time < 100)
    {
        active_count = 0;
        for(i=0;i<MAX_CLENTITIES;i++)   // replace with a linked-list!!
        {
            if(!EntityList[i]->active && !EntityList[i]->run_fx)
                continue;
            active_count++; // expedite when not running anything (including FX)
            if(server)
            {
                EntityList[i]->SV_RunFrame();
                
            } else if (client) {
                if(EntityList[i]->run_with_server_frames && elapsed_server_time < 1000)
                {
                    EntityList[i]->CL_RunFrame();
                    if(EntityList[i]->run_fx)
                    {
                        EntityList[i]->RunFX();  // run particle and other FX independantly (smoke rising after entity is removed)
                        fx_count++;
                    }
                } else {
                    EntityList[i]->CL_StillFrame();
                }
            }
            EntityList[i]->old_origin = EntityList[i]->origin;
        }
    }
    Physics->PostFrame();
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
