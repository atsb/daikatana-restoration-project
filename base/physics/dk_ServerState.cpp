
#include "p_global.h"
#include "p_user.h"
#include "p_client.h"
#include "p_inventory.h"
//#include "ClientEntityManager.h"

/////////////////////////////////////////////////////////////////////////
/// Init Server State
/////////////////////////////////////////////////////////////////////////

static void LightStyle (int style, char *val)
{
	//	gi.dprintf("Lightstyles were purged\n");
}

///////////////////////////////////////////////////////////////////////////////
//	SetOrigin
//
///////////////////////////////////////////////////////////////////////////////

static void SetOrigin (userEntity_t *ent, float x, float y, float z)
{
	ent->s.origin.Set( x, y, z );

	gi.linkentity (ent);
//	gi.dprintf("SetOrigin and call linkentity\n");
}

///////////////////////////////////////////////////////////////////////////////
//	SetOrigin2
//
///////////////////////////////////////////////////////////////////////////////

static void SetOrigin2 (userEntity_t *ent, CVector &pos)
{
	ent->s.origin = pos;

	gi.linkentity (ent);
//	gi.dprintf("SetOrigin and call linkentuty\n");
}

///////////////////////////////////////////////////////////////////////////////
//	TraceLine
//
///////////////////////////////////////////////////////////////////////////////

#define	MASK_NOMONSTERS (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_CLEAR|CONTENTS_NOSHOOT)
#define	MASK_MONSTERS (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_CLEAR|CONTENTS_NOSHOOT)
static void TraceLine ( CVector &start, CVector &end, int nomonsters, userEntity_t *passent, userTrace_t *trace)
{
	trace_t		t;

	if (nomonsters)
		t = gi.TraceLine (start, end, passent, MASK_NOMONSTERS);
	else
		t = gi.TraceLine (start, end, passent, MASK_MONSTERS);

	trace->allsolid = t.allsolid;
	trace->startsolid = t.startsolid;
	trace->fraction = t.fraction;

	trace->endpos = t.endpos;

	trace->ent = t.ent;

	trace->planeNormal = t.plane.normal;

	trace->inwater = 0;
	trace->inopen = 0;

	//gi.dprintf("TraceLine, use trace\n");
}

///////////////////////////////////////////////////////////////////////////////
//	SetSize
//
///////////////////////////////////////////////////////////////////////////////

static void SetSize(userEntity_t *self, float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
	self->s.mins.Set( minx, miny, minz );
	self->s.maxs.Set( maxx, maxy, maxz );

//	serverState.LinkEntity (self);
//	gi.dprintf("SetSize, user linkentuty\n");
}

///////////////////////////////////////////////////////////////////////////////
//	GetCvar
//	
///////////////////////////////////////////////////////////////////////////////

static	float	GetCvar (char *varName)
{
	cvar_t	*cv;

	cv = gi.cvar (varName, "", 0);
	return	atof (cv->string);
}

///////////////////////////////////////////////////////////////////////////////
//	SetCvar
//	
///////////////////////////////////////////////////////////////////////////////

static	void	SetCvar (char *varName, char *value)
{
	gi.cvar_forceset (varName, value);
}


///////////////////////////////////////////////////////////////////////////////
//	bprint
//
//	sends a text message to all clients
///////////////////////////////////////////////////////////////////////////////

static void bprint (char *fmt, ...)
{
	char	szBuf	[1024];
	va_list	ap;

	va_start (ap, fmt);
	vsprintf (szBuf, fmt, ap);
	va_end (ap);

	gi.bprintf (PRINT_HIGH, "%s", szBuf);
}

///////////////////////////////////////////////////////////////////////////////
//	sprint
//
//	sends a text message to a single client
///////////////////////////////////////////////////////////////////////////////

static void sprint (edict_t *ent, char *fmt, ...)
{
	char	szBuf	[1024];
	va_list	ap;

	va_start (ap, fmt);
	vsprintf (szBuf, fmt, ap);
	va_end (ap);

	gi.cprintf (ent, PRINT_HIGH, "%s", szBuf);
}

///////////////////////////////////////////////////////////////////////////////
//	particle
//
///////////////////////////////////////////////////////////////////////////////

void particle (int te_type, CVector &org, CVector &dir, float speed, CVector& color, int count, int type)
{
	if (p_sendparticles->value == 0 && (deathmatch->value != 0 || coop->value != 0))
		//	don't send particles in multiplayer to reduce network traffic
		return;

	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (te_type);

	gi.WritePosition (org);
	gi.WriteDir (dir);
	gi.WriteByte (speed);
	gi.WriteByte (count);
	gi.WriteFloat( color.x );
	gi.WriteFloat( color.y );
	gi.WriteFloat( color.z );
	gi.WriteByte (type);

	//	send to everyone in the same PVS
	gi.multicast (org, MULTICAST_PVS);
}

/////////////////////////////////////////////////////////////////////////////////
////	
////
/////////////////////////////////////////////////////////////////////////////////
//
//void sound (userEntity_t *ent, int channel, char *sample, int volume, float attenuation)
//{
//	int		index;
//
////	gi.Con_Dprintf ("winfoSound is broken!\n");
////	return;
//
//	index = gi.soundindex (sample);
//	
//	gi.sound (ent, channel, index, volume, attenuation, 0);
//}

void StartSound(userEntity_t* ent, int channel, int sound_index, float volume, float dist_min, float dist_max)
{
	CVector vZero(0,0,0);
	gi.StartSound(vZero,ent,channel,sound_index,volume,dist_min,dist_max,0.0f);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
static void ClientCmd(userEntity_t *ent, char *cmd)
{
	gi.Con_Dprintf("ClientCmd : Not implemented\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
int ModelInfo(int modelIndex, userModelInfo_t *modelInfo) 
{ 
	gi.Con_Dprintf("ModelInfo: not implemented.\n");

	return(0); 
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
int FrameInfo(int modelIndex, int frameIndex, userFrameInfo_t *frameInfo) 
{ 
	gi.Con_Dprintf("FrameInfo function no longer supported.\nUse dk_GetFrameName.\n");
	return(0); 
}
*/
///////////////////////////////////////////////////////////////////////////////
//	AmbientSound
//
//	starts a sound at a specified location
//	Channel is auto-selected
//
//	Q2FIXME:	rework this function for ambient sounds that can be toggled
//	Q2FIXME:	implement fade rate for ambient sounds (attenuation modifier)
///////////////////////////////////////////////////////////////////////////////

void AmbientSound( CVector &pos, char *samp, int vol, int attenuation, float fade) 
{
//	int		index;

//	index = gi.soundindex (samp);
	gi.soundindex (samp);

//	gi.positioned_sound (pos, NULL, 0, index, vol, attenuation, 0);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
void MakeStatic(userEntity_t *ent) 
{
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
char *GetArgv(int arg) 
{ 
	return(NULL); 
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
int   GetArgc(void) 
{ 
	return(0); 
}
*/

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
void  SetFunction(char *funcName, void *func, char *funcDesc) 
{
	gi.Error ("SetFunction not supported in Q2 base, ass!\n");
}
*/

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void *GetFunction(char *funcName) 
{ 
	gi.Error ("GetFunction not supported in Q2 base, ass!\n");
	return(NULL); 
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void  ListFunction(char *funcName) 
{
	gi.Error ("ListFunction not supported in Q2 base, ass!\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void  TraceBox (CVector &start, CVector &end, CVector &mins, CVector &maxs,
		int nomonsters, userEntity_t *passent, userTrace_t *trace)
{
	gi.Error ("TraceBox: use trace_q2, ass.\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
int VertInfo (userEntity_t *self, int surfaceIndex, int frameIndex, int vertIndex, CVector &vert)
{
	gi.Con_Dprintf("VertInfo: not implemented\n");
	return(0);
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
int	SurfInfo (int modelIndex, char *surfName)
{
	gi.Con_Dprintf("SurfInfo: not implemented\n");
	return(0);
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void SetClientViewEntity (userEntity_t *client, userEntity_t *viewEntity)
{
//	gi.Con_Dprintf("SetClientViewEntity not implemented.\n");
	client->view_entity = viewEntity;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void SetClientInputEntity (userEntity_t *client, userEntity_t *inputEntity)
{
//	gi.Con_Dprintf("SetClientInputEntity not implemented.\n");
	client->input_entity = inputEntity;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
void StickyInit (userEntity_t *parent, userEntity_t *child, unsigned long int flags)
{
	gi.Con_Dprintf("StickyInit: not implemented\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void StickyRemove (userEntity_t *parent, userEntity_t *child)
{
	gi.Con_Dprintf("SkickyRemove: not implemented.\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void StickySetFlags (userEntity_t *parent, userEntity_t *child, unsigned long int flags)
{
	gi.Con_Dprintf("SkickySetFlags: not implemented.\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void StickyRemoveChildren (userEntity_t *parent)
{
	gi.Con_Dprintf("StickyRemoveChildren: not implemented.\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/*
void StickySurfaceInit (userEntity_t *parent, userEntity_t *child, unsigned long int flags, char *f_surf, int f_vIndex, char *r_surf, int r_vIndex, CVector &offset)
{
	gi.Con_Dprintf("StickySurfaceInit: not implemented.\n");
}
*/
///////////////////////////////////////////////////////////////////////////////
//	SetModel
//
///////////////////////////////////////////////////////////////////////////////

void	SetModel (userEntity_t *self, char *modelName)
{
	//	Q2FIXME:	for testing until LWO code is in

	if (modelName && modelName [0] != '*')
	{
		if (self->movetype != MOVETYPE_PUSH && self->solid != SOLID_NOT && self->solid != SOLID_TRIGGER)
		{
			gi.setmodel (self, "models/global/dv_node.dkm");
			self->modelName = "models/global/dv_node.dkm";

			gi.linkentity (self);

			return;
		}
	}

	gi.setmodel (self, modelName);

	gi.linkentity (self);

	self->modelName = modelName;

	//	calculate self->size for door lips to work properly
	self->size = self->s.maxs - self->s.mins;
	//	account for mins maxs extending +1 on all sides
	self->size.x -= 2;
	self->size.y -= 2;
	self->size.z -= 2;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*
void	dk_ClientCmd (userEntity_t *ent, char *cmd)
{
	gi.Error ("ClientCmd no longer supported.  See Nelson.\n");
}
*/

void StartEntitySound(userEntity_t* ent, int channel, int sound_index, float volume, float dist_min, float dist_max)
{
	CVector vZero(0,0,0);
	gi.StartSound(vZero,ent,channel,sound_index,volume,dist_min,dist_max,0.0);
}

///////////////////////////////////////////////////////////////////////////////
//	InitiServerState
//
///////////////////////////////////////////////////////////////////////////////

void P_InventoryNext( userEntity_t *self );
void P_InventoryPrev( userEntity_t *self );
void P_InventoryUse( userEntity_t *self );
void P_InventoryOperate( userEntity_t *self, int bHideDisplay );
void P_CommandSelectorPrev(userEntity_t *self);
void P_CommandSelectorNext(userEntity_t *self);
void P_CommandSelectorApply(userEntity_t *self);

void	InitServerState (game_import_t *import)
{
	memset (&serverState, 0x00, sizeof (serverState_t));

	serverState.basedir						= import->dk_gamedir;

	serverState.Con_Printf					= import->Con_Printf;
	serverState.Con_Dprintf					= import->Con_Dprintf;
											
	serverState.SetOrigin					= SetOrigin;
	serverState.SetOrigin2					= SetOrigin2;
	serverState.SetModel					= SetModel;
	serverState.TraceLine					= TraceLine;
	serverState.PointContents				= import->pointcontents;
	serverState.RemoveEntity				= P_FreeEdict;
	serverState.SetSize						= SetSize;

	serverState.SpawnEntity					= P_SpawnEdict;
	serverState.FirstEntity					= P_FirstEntity;
	serverState.NextEntity					= P_NextEntity;	//	Nelno: implemented this

	serverState.bprint						= bprint;
	serverState.sprint						= sprint;
	serverState.particle					= particle;
	serverState.centerprint					= import->centerprint;
	
#ifdef	QUAKE1_NETWORK_CODE
	serverState.WriteFloat  = WriteFloat;
	serverState.WriteByte   = WriteByte;
	serverState.WriteChar   = WriteByte;
	serverState.WriteShort  = WriteShort;
	serverState.WriteLong   = WriteLong;
	serverState.WriteAngle  = WriteAngle;
	serverState.WriteCoord  = WriteCoord;
	serverState.WriteString = WriteString;
	serverState.WriteEntity = WriteEntity;
#endif

	serverState.CBuf_AddText				= import->CBuf_AddText;
// SCG[1/22/00]: 	serverState.ClientCmd					= ClientCmd;

	serverState.GetCvar						= GetCvar;
	serverState.SetCvar						= SetCvar;

	serverState.printxy						= import->printxy;
	serverState.clearxy						= import->clearxy;

	serverState.InventoryNew				= P_InventoryNew;
	serverState.InventoryFree				= P_InventoryFree;
	serverState.InventoryFindItem			= P_InventoryFindItem;
	serverState.InventoryFirstItem			= P_InventoryFirstItem;
	serverState.InventoryNextItem			= P_InventoryNextItem;
	serverState.InventoryAddItem			= P_InventoryAddItem;
	serverState.InventoryDeleteItem			= P_InventoryDeleteItem;
	serverState.InventoryCreateItem			= P_InventoryCreateItem;
	serverState.InventoryItemCount			= P_InventoryItemCount;
	serverState.InventoryUpdateClient		= P_InventoryUpdateClient;
	serverState.InventoryNext				= P_InventoryNext;
	serverState.InventoryPrev				= P_InventoryPrev;
	serverState.InventoryUse				= P_InventoryUse;
	serverState.InventoryOperate			= P_InventoryOperate;

	serverState.CommandSelectorPrev			= P_CommandSelectorPrev;
	serverState.CommandSelectorNext			= P_CommandSelectorNext;
	serverState.CommandSelectorApply		= P_CommandSelectorApply;

	serverState.WeaponDisplayUpdateClient	= P_WeaponUpdateClient;

// SCG[1/22/00]: 	serverState.ModelInfo					= ModelInfo;
// SCG[1/22/00]: 	serverState.FrameInfo					= FrameInfo;
	serverState.GetFrameName				= import->dk_GetFrameName;
	serverState.GetModelSkinIndex			= import->dk_GetModelSkinIndex;
	serverState.GetModelSkinName			= import->dk_GetModelSkinName;
	serverState.ForceModelLoad				= import->dk_ForceModelLoad;
	serverState.PreCacheModel				=	import->dk_PreCacheModel;

	serverState.AmbientSound				= AmbientSound;
// SCG[1/22/00]: 	serverState.MakeStatic					= MakeStatic;

	serverState.GetArgv						= import->GetArgv;
	serverState.GetArgc						= import->GetArgc;

	serverState.Error						= import->Error;

// SCG[1/22/00]: 	serverState.SetFunction					= SetFunction;
// SCG[1/22/00]: 	serverState.GetFunction					= GetFunction;
// SCG[1/22/00]: 	serverState.ListFunction				= ListFunction;

//	serverState.VertInfo = import->VertInfo;
//	serverState.SurfIndex = import->SurfIndex;
//	serverState.TriVerts = import->TriVerts;

	serverState.SetClientViewEntity  = Client_SetViewEntity;
	serverState.SetClientInputEntity = Client_SetInputEntity;

	//	Nelno's additions
	serverState.AddCommand	= import->AddCommand;
	serverState.LightStyle	= LightStyle;

	//	New for Q2
	serverState.TraceLine_q2		= import->TraceLine;
	serverState.TraceLineTrigger	= import->TraceLineTrigger;

	serverState.TraceBox_q2			= import->TraceBox;
	serverState.inPVS				= import->inPVS;
	serverState.inPHS				= import->inPHS;
	serverState.SetAreaPortalState	= import->SetAreaPortalState;
	serverState.AreasConnected		= import->AreasConnected;

	serverState.LinkEntity		= import->linkentity;
	serverState.UnlinkEntity	= import->unlinkentity;
	serverState.BoxEdicts		= import->BoxEdicts;
	serverState.Pmove			= import->Pmove;

	serverState.MultiCast		= import->multicast;
	serverState.UniCast			= import->unicast;
	serverState.WriteChar		= import->WriteChar;
	serverState.WriteByte		= import->WriteByte;
	serverState.WriteShort		= import->WriteShort;
	serverState.WriteLong		= import->WriteLong;
	serverState.WriteFloat		= import->WriteFloat;
	serverState.WriteString		= import->WriteString;
	serverState.WritePosition	= import->WritePosition;
	serverState.WriteDir		= import->WriteDir;
	serverState.WriteAngle		= import->WriteAngle;
	
	serverState.DebugGraph		= import->DebugGraph;
	
//	serverState.PositionedSound	= import->positioned_sound;
//	serverState.Sound			= import->sound;
//	serverState.StartSound		= import->StartSound;
//	serverState.StartSound  = StartSound;

	serverState.StartEntitySound = StartEntitySound;
	serverState.SetReverbPreset = import->set_reverb_preset ; // jas
	serverState.StreamedSound	= import->streamed_sound;
	serverState.StartMusic		= import->StartMusic;
	serverState.StartMP3		= import->StartMP3;
	serverState.StopMP3			= import->StopMP3;

	serverState.ConfigString	= import->configstring;

	serverState.ModelIndex	= import->modelindex;
	serverState.SoundIndex	= import->soundindex;
	serverState.ImageIndex	= import->imageindex;

	serverState.bprintf		= import->bprintf;
	serverState.Con_Dprintf = import->Con_Dprintf;
	serverState.cprintf		= import->cprintf;
	serverState.centerprintf= import->centerprintf;

//	serverState.TagMalloc	= import->TagMalloc;
//	serverState.TagFree		= import->TagFree;
//	serverState.FreeTags	= import->FreeTags;

	serverState.X_Malloc	= import->X_Malloc;
	serverState.X_Free		= import->X_Free;
	serverState.Mem_Free_Tag= import->Mem_Free_Tag;
	serverState.Mem_Heap_Walk=import->Mem_Heap_Walk;

	serverState.Sys_Milliseconds = import->Sys_Milliseconds;
	serverState.SetCameraState = Client_SetCameraState;
	serverState.SetClientAngles = Client_SetClientAngles;

	serverState.PushMove = P_Push;

	serverState.FS_FileLength = import->FS_FileLength;
	serverState.FS_Close = import->FS_Close;
	serverState.FS_Open = import->FS_Open;
	serverState.FS_Read = import->FS_Read;
	serverState.FS_LoadFile = import->FS_LoadFile;
	serverState.FS_FreeFile = import->FS_FreeFile;
	serverState.FS_Seek = import->FS_Seek;
	serverState.FS_Tell = import->FS_Tell;
	serverState.FS_Getc = import->FS_Getc;
	serverState.FS_Ungetc = import->FS_Ungetc;
	serverState.FS_Gets = import->FS_Gets;
	serverState.FS_GetBytesRead = import->FS_GetBytesRead;

	//	set up pointers to global structures contained in physics.dll
	serverState.game	= &game;
	serverState.level	= &level;
	serverState.gi		= import;
	serverState.globals	= &globals;
	
	serverState.g_edicts= g_edicts;

	serverState.mapName = level.mapname;

    //functions exported from the exe for in game cinematic playback.
    serverState.CIN_CamLocation = import->CIN_CamLocation;
    serverState.CIN_GCE_Interface = import->CIN_GCE_Interface;
    serverState.CIN_WorldPlayback = import->CIN_WorldPlayback;

	serverState.SpawnFakeEntity = P_SpawnFakeEntity;
	serverState.RemoveFakeEntity = P_RemoveFakeEntity;

	serverState.PercentSubmerged = P_PercentSubmerged;
	serverState.RegisterFunc = P_RegisterFunc;
	
	serverState.cvar_forceset = import->cvar_forceset;
	serverState.cvar = import->cvar;
	serverState.cvar_set = import->cvar_set;

	serverState.common_exports = &com;

	serverState.cs = import->cs;
    // set up the entity manager
//    serverState.ClientEntityManager = new CClientEntityManager; // instantiate
//    serverState.ClientEntityManager->SetServerState(&serverState);    // give the server functions an interface to net routines
}

