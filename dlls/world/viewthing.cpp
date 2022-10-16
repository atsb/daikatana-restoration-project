///////////////////////////////////////////////////////////////////////////////
//	viewthing.cpp
//
//	for viewing models and their animations
//
//	FIXME:	remove or disable for final release
//	FIXME:	fix viewframe
///////////////////////////////////////////////////////////////////////////////
//#ifdef _DEBUG
#include	"world.h"
#include	"ai_frames.h"
#include "MonsterSound.h"
#include "ai_utils.h"
#include "ai_func.h"


static char VM_ClassName[255];

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	viewthing (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

//#define	VT_MAX_SEQUENCES	96
#define	VT_MAX_SEQUENCES	150

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//typedef	struct
//{
//	char	seqName [16];		//	name of sequence
//	
//	int		seqStart;			//	starting frame of sequence
//	int		seqLength;			//	# of frames in sequence
//} seqList_t;

typedef	struct
{
	//seqList_t	*list;
	frameData_t *list;

	//	list of sequences to play... will loop through them in order
	int			PlayList [VT_MAX_SEQUENCES];

	int			PlayListLength;
	int			CurrentSequence;

	int			startFrame;
	int			endFrame;
} seqHeader_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//static	int				vt_startFrame = 0;
//static	int				vt_endFrame = 0;

static	userEntity_t	*vt;		
static	float			vt_thinkTime = THINK_TIME;
static	frameData_t		vt_List [VT_MAX_SEQUENCES];
static	seqHeader_t		vt_Header;


///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	vt_InitList
//
///////////////////////////////////////////////////////////////////////////////

void	vt_InitList (seqHeader_t *header, frameData_t *list)
{
	memset (header->PlayList, 0x00, sizeof (header->PlayList));
	memset (list, 0x00, sizeof (frameData_t) * VT_MAX_SEQUENCES);
	header->list = list;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_InList
//
//	returns FALSE if sequence name is not already in list
///////////////////////////////////////////////////////////////////////////////

int	vt_InList (frameData_t *list, char *name)
{
	int		i;

	for (i = 0; list [i].animation_name [0] != 0x00; i++)
	{
		if (!stricmp (name, list [i].animation_name))
			return	TRUE;
	}

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_ListIndex
//
//	returns the index in vt_List of the specified seqeunce name
//	or -1 if name was not in vt_List
///////////////////////////////////////////////////////////////////////////////

int	vt_ListIndex (frameData_t *list, char *name)
{
	int		i;

	if(!(list))
		return -1;
	for (i = 0; list [i].animation_name [0] != 0x00; i++)
	{
		if (!stricmp (name, list [i].animation_name))
			return	i;
	}

	return	-1;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_AddToList
//
//	adds a sequence at the end of vt_List
///////////////////////////////////////////////////////////////////////////////

void	vt_AddToList (frameData_t *list, char *name, int start, int len)
{
	int		i;
	if(!(list))
		return;
	for (i = 0; list [i].animation_name [0] != 0x00; i++);

	if (i < VT_MAX_SEQUENCES)
	{
		strcpy (list [i].animation_name, name);
		list [i].first = start;
		list [i].last = len;
	}
	else
	{
		gstate->Con_Printf ("Too many unique sequences in model!\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_isAlpha
//
//	returns true if character is an upper or lowercase letter
///////////////////////////////////////////////////////////////////////////////

int	vt_isAlpha (char c)
{
	if (c >= 0x41 && c <= 0x5A)
		return	true;
	if (c >= 0x61 && c <= 0x7A)
		return  true;

	return	false;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_GetSequences
//
//	shows all sequences and the number of frames in each
//
//	goes through all frames and saves each unique name
///////////////////////////////////////////////////////////////////////////////

static
void	vt_GetSequences (userEntity_t *self, seqHeader_t *header)
{
	char	frameName [16];
	int		frame = 0;
	int		startFrame, endFrame, i;

	//	init list
	memset (header->list, 0x00, sizeof (frameData_t) * VT_MAX_SEQUENCES);

	while (1)
	{
		gstate->GetFrameName (self->s.modelindex, frame, frameName);

		if ( frameName[0] == 0x00 || strlen(frameName) == 0 || !vt_isAlpha(frameName[0]) )
        {
			break;
        }

		//	strip frame number from end of name
		for (i = 0; frameName [i] < 0x30 || frameName [i] > 0x39; i++);

		_ASSERTE( i < 16 );
        frameName [i] = 0x00;

        if ( vt_ListIndex (header->list, frameName) < 0 && strlen(frameName) > 0 )
		{
			com->GetFrames (self, frameName, &startFrame, &endFrame);
			vt_AddToList (header->list, frameName, startFrame, endFrame - startFrame);
		}

		frame++;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_ListSequences
//
//	shows all sequences and the number of frames in each
//
//	goes through all frames and saves each unique name
//	only lists the sequences available to the legs for hierarhical models
///////////////////////////////////////////////////////////////////////////////

void	vt_ListSequences (userEntity_t *self)
{
	int				i = 0, frameTotal = 0;
	char			name [32], start [16], len [16];
	seqHeader_t		*header;

	// SCG[12/4/99]: Duh.
	if( vt == NULL )
	{
		return;
	}

	playerHook_t	*hook = AI_GetPlayerHook( vt );

	header = &vt_Header;
	vt_GetSequences (vt, header);

	gstate->Con_Printf ("\nname                             start  length\n");
	gstate->Con_Printf ("----                             -----  ------\n");

	while (1)
	{
		if (header->list [i].animation_name [0] == 0x00)
			break;

		//	format output
		memset (name, 0x20, sizeof (name));
		strcpy (name, header->list [i].animation_name);
		name [strlen (header->list [i].animation_name)] = 0x20;
		name [31] = 0x00;

//		sprintf (start, "%5i", header->list [i].first);
//		sprintf (len, "%6i", header->list [i].last + 1);
		Com_sprintf (start, sizeof(start),"%5i", header->list [i].first);
		Com_sprintf (len, sizeof(len),"%6i", header->list [i].last + 1);

		gstate->Con_Printf ("%s %s %s\n", name, start, len);

		frameTotal += header->list [i].last;

		i++;
	}

	if (!frameTotal)
		gstate->Con_Printf ("No frames in model.\n");
	else
	{
		gstate->Con_Printf ("----------------------------\n");
		gstate->Con_Printf ("%i total frames\n\n", frameTotal);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_ShowInfo
//
//	Will show generic debug information
//	<nss>
///////////////////////////////////////////////////////////////////////////////
void vt_ShowInfo(userEntity_t *self)
{
	char Buffer[256];
	
//	sprintf(Buffer,"Frame:%d ",self->s.frame);
	Com_sprintf(Buffer,sizeof(Buffer),"Frame:%d ",self->s.frame);

	if(self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1)
//		sprintf(Buffer,"%s FrameState:PS1",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s FrameState:PS1",Buffer);
	else if(self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
//		sprintf(Buffer,"%s FrameState:PS2",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s FrameState:PS2",Buffer);
	else
//		sprintf(Buffer,"%s FrameState:NA ",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s FrameState:NA ",Buffer);

	if(self->s.frameInfo.sound1Frame)
//		sprintf(Buffer,"%s SFX1:Yes",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s SFX1:Yes",Buffer);
	else
//		sprintf(Buffer,"%s SFX1:No",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s SFX1:No",Buffer);
	
	if(self->s.frameInfo.sound2Frame)
//		sprintf(Buffer,"%s SFX2:Yes",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s SFX2:Yes",Buffer);
	else
//		sprintf(Buffer,"%s SFX2:No",Buffer);
		Com_sprintf(Buffer,sizeof(Buffer),"%s SFX2:No",Buffer);

	AI_Dprintf("%s\n",Buffer);
	AI_Dprintf("\n");
	AI_Dprintf("\n");
	AI_Dprintf("\n");
	AI_Dprintf("\n");
	AI_Dprintf("\n");
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Stop
//
//	stops animation of all viewthings
//	stops animation of all hierarchical bits
///////////////////////////////////////////////////////////////////////////////

void	vt_Stop (userEntity_t *self)
{
	userEntity_t	*ent;
	int				count = 0;

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->nextthink = -1;
			ent->s.frameInfo.frameFlags = FRAME_STATIC;

			count++;
		}
	}

	if (!count)
		gstate->Con_Printf ("No viewthing on map.\n");
}


///////////////////////////////////////////////////////////////////////////////
//	vt_Start <nss>
//
//	Starts animation of all viewthings
//	Starts animation of all hierarchical bits
///////////////////////////////////////////////////////////////////////////////
void	vt_Start (userEntity_t *self)
{
	userEntity_t	*ent;
	int				count = 0;

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->nextthink = gstate->time + 0.01f;
			ent->s.frameInfo.frameFlags = FRAME_LOOP;
			count++;
		}
	}
	if (!count)
		gstate->Con_Printf ("No viewthing on map.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Delay
//
//	Sets the Delay for inbetween sequences
//	<nss>
///////////////////////////////////////////////////////////////////////////////
void vt_Delay(userEntity_t *self)
{
	userEntity_t	*ent;
	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: vdelay [delay(float)]\n");
		return;
	}
	float Delay = atof (gstate->GetArgv (1));
	//	do for all viewthings
	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className || !ent->inuse)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->volume = Delay;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Forward
//
//	Plays the animation one frame forward
//	<nss>
///////////////////////////////////////////////////////////////////////////////
void vt_StepFrame(userEntity_t *self)
{
	//self->s.frameInfo.frameFlags &= ~FRAME_STATIC;
	self->nextthink = gstate->time + vt_thinkTime;
	self->s.frame = self->lastAIFrame;
	vt_ShowInfo(self);
}

///////////////////////////////////////////////////////////////////////////////
//	vt_PlaySoundFrame
//
//	Plays the animation sound if applicable if not clears the sound flag
//	<nss>
///////////////////////////////////////////////////////////////////////////////
void vt_PlaySoundFrame(userEntity_t *self, int currentframe, int newframe)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	if ( self->s.frameInfo.sound1Frame >= currentframe && self->s.frameInfo.sound1Frame < newframe )
	{
		if ( self->s.frameInfo.sound1Index )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, self->s.frameInfo.sound1Index, 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND1;
		}
	}
	else
	{
		self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;
	}

	if ( self->s.frameInfo.sound2Frame >= currentframe &&  self->s.frameInfo.sound2Frame < newframe)
	{
		if ( self->s.frameInfo.sound2Index )
		{
			gstate->StartEntitySound(self, CHAN_AUTO,self->s.frameInfo.sound2Index, 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			self->s.frameInfo.frameState |= FRSTATE_PLAYSOUND2;
		}
	}
	else
	{
		self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Forward
//
//	Plays the animation one frame forward
//	<nss>
///////////////////////////////////////////////////////////////////////////////
void vt_Forward(userEntity_t *self)
{
	userEntity_t	*ent;
	int				count = 0;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->nextthink = gstate->time + vt_thinkTime;
			ent->think = vt_StepFrame;
			ent->s.frameInfo.frameFlags = FRAME_FORCEINDEX|FRAME_STATIC;
			if(ent->s.frame <= (ent->s.frameInfo.endFrame - 1))
			{
				ent->s.frame++;
				vt_PlaySoundFrame(ent,ent->lastAIFrame,ent->s.frame);//Important to do this BEFORE we set lastAIframe
				ent->lastAIFrame = ent->s.frame;
				if(hook->fxFrameFunc)
					hook->fxFrameFunc(self);
			}
			else
			{
				AI_Dprintf("End of Sequence Reached.\n");
			}
			count++;
		}
	}
	if (!count)
		gstate->Con_Printf ("No viewthing on map.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Backward
//
//	Plays the animation one frame Backward
//  <nss>	
///////////////////////////////////////////////////////////////////////////////
void vt_Backward(userEntity_t *self)
{
	userEntity_t	*ent;
	int				count = 0;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->nextthink = gstate->time + vt_thinkTime;
			ent->think = vt_StepFrame;
			ent->s.frameInfo.frameFlags = FRAME_FORCEINDEX|FRAME_STATIC;
			if(ent->s.frame >= (ent->s.frameInfo.startFrame + 1))
			{				
				
				ent->s.frame--;
				ent->lastAIFrame-=2;
				vt_PlaySoundFrame(ent,ent->lastAIFrame,ent->s.frame);
				ent->lastAIFrame = ent->s.frame;
				if(hook->fxFrameFunc)
					hook->fxFrameFunc(self);
			}
			else
			{
				AI_Dprintf("Start of Sequence Reached.\n");
			}
			count++;
		}
	}

	if (!count)
		gstate->Con_Printf ("No viewthing on map.\n");
}


///////////////////////////////////////////////////////////////////////////////
//<nss>
///////////////////////////////////////////////////////////////////////////////

void	vt_ViewFrame (userEntity_t *self)
{
	userEntity_t	*ent;
	int				count = 0;
	int				frame;
	playerHook_t	*hook;	

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: vf [frame]\n");
		return;
	}
	
	vt_Stop (self);

	frame = atoi (gstate->GetArgv (1));

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			hook = AI_GetPlayerHook( ent );

			ent->s.frame = frame;
			ent->s.frameInfo.frameFlags = FRAME_FORCEINDEX | FRAME_STATIC;
			count++;
		}
	}

	if (!count)
		gstate->Con_Printf ("No viewthing on map.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	vt_IncFrames
//
//	changes self->s.frame
//	doesn't frame skip, so percieved playback can drop below desired rate
///////////////////////////////////////////////////////////////////////////////

void	vt_IncFrames (userEntity_t *self, seqHeader_t *header)
{
	self->s.frame++;
	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	if (self->s.frame > header->endFrame)
	{
		if (header->PlayListLength > 1)
		{
			header->CurrentSequence++;
			if (header->CurrentSequence >= header->PlayListLength)
				header->CurrentSequence = 0;

			header->startFrame = header->list [header->PlayList [header->CurrentSequence]].first;
			header->endFrame = header->startFrame + header->list [header->PlayList [header->CurrentSequence]].last;
		}

		self->s.frame = header->startFrame;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_CheckFrames
//
//	checks frameState for FRAME_STOPPED and restarts appropriate sequence 
///////////////////////////////////////////////////////////////////////////////

void	vt_CheckFrames (userEntity_t *self, seqHeader_t *header)
{
	CVector		hp;

	if (header->PlayListLength == 1)
		//	client will auto-loop frame
		return;

	if (self->s.frameInfo.frameState & FRSTATE_STOPPED)
	{
		self->s.frameInfo.frameState -= FRSTATE_STOPPED;

		header->CurrentSequence++;
		if (header->CurrentSequence >= header->PlayListLength)
			header->CurrentSequence = 0;

		self->s.frameInfo.startFrame = header->startFrame = header->list [header->PlayList [header->CurrentSequence]].first;
		self->s.frameInfo.endFrame = header->endFrame = header->startFrame + header->list [header->PlayList [header->CurrentSequence]].last;

		self->s.frame = header->startFrame;
	}
}
void	vt_Animate (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	vt_delayThink
//  <nss>
///////////////////////////////////////////////////////////////////////////////
void vt_DelayThink(userEntity_t *self)
{
	self->think = vt_Animate;
	self->nextthink = gstate->time + vt_thinkTime;
	self->s.frameInfo.frameFlags = FRAME_LOOP;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Animate
//  <nss>
///////////////////////////////////////////////////////////////////////////////

void	vt_Animate (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	vt_CheckFrames (self, &vt_Header);

	// play sound 1
	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		//Show debug information
		vt_ShowInfo(self);

		playerHook_t *hook = AI_GetPlayerHook( self );
		_ASSERTE( hook );
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;
			if ( self->s.frameInfo.sound1Index )
			{
				gstate->StartEntitySound(self, CHAN_AUTO, self->s.frameInfo.sound1Index, 
                                         1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			}
		}
		else 
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;
			if ( self->s.frameInfo.sound2Index )
			{
				gstate->StartEntitySound(self, CHAN_AUTO,self->s.frameInfo.sound2Index, 
                                         1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			}
		}
		if((self->s.frameInfo.endFrame -1) <= self->s.frame)
		{
			if(self->volume > 0.0f)
			{
				self->nextthink = gstate->time + self->volume;
				self->s.frameInfo.frameFlags += FRAME_STATIC; 
				self->think     = vt_DelayThink;
				return;
			}
		}
	}
	self->nextthink = gstate->time + vt_thinkTime;
	
}

///////////////////////////////////////////////////////////////////////////////
//	vt_GetPlayList
//
//	gets play list a model from the current console line
///////////////////////////////////////////////////////////////////////////////

void	vt_GetPlayList (seqHeader_t *header)
{
	int		i, index;

	//	get number of each sequence specified
	memset (header->PlayList, 0x00, sizeof (header->PlayList));

	for (i = 1; i < gstate->GetArgc (); i++)
	{
		index = vt_ListIndex (header->list, gstate->GetArgv (i));
		if (index < 0)
		{
			gstate->Con_Printf ("Sequence %s is not in model.\n", gstate->GetArgv (i));
			return;
		}

		header->PlayList [i - 1] = index;
	}

	header->PlayListLength = i - 1;
	header->CurrentSequence = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	vt_SetFrames
//
///////////////////////////////////////////////////////////////////////////////

void	vt_SetFrames (userEntity_t *ent, seqHeader_t *header)
{
	if (header->PlayListLength > 1)
		ent->s.frameInfo.frameFlags = FRAME_ONCE;
	else
		ent->s.frameInfo.frameFlags = FRAME_LOOP;

	ent->s.frameInfo.startFrame = header->startFrame = header->list [header->PlayList [0]].first;
	ent->s.frameInfo.endFrame = header->endFrame = header->startFrame + header->list [header->PlayList [0]].last;
	ent->s.frame = header->startFrame;
	ent->s.frameInfo.frameInc = 1;
	ent->s.frameInfo.frameState = 1;
	ent->s.frameInfo.frameTime = FRAMETIME_FPS20;
	ent->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;
	ent->s.frameInfo.sound1Frame = header->list[header->PlayList[0]].soundframe1;
	ent->s.frameInfo.sound2Frame = header->list[header->PlayList[0]].soundframe2;
	ent->s.frameInfo.sound1Index = gstate->SoundIndex(header->list[header->PlayList[0]].sound1);
	ent->s.frameInfo.sound2Index = gstate->SoundIndex(header->list[header->PlayList[0]].sound2);
}

///////////////////////////////////////////////////////////////////////////////
//	vt_ViewSequence
//
//	starts a sequence animating
//	handles multiple viewthings -- who knows why Nelno bothered...
//
//	also handles looping of multiple sequences like:
//		tranza, ataka, tranzb, tranza, ataka, tranzb, ...
///////////////////////////////////////////////////////////////////////////////

void	vt_ViewSequence (userEntity_t *self)
{
	userEntity_t	*ent;
	seqHeader_t		*header;

	if (gstate->GetArgc () < 2)
	{
		gstate->Con_Printf ("  USAGE: vs [sequence name]\n");
		return;
	}

	header = &vt_Header;

	vt_GetPlayList (&vt_Header);

	//	do for all viewthings
	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className || !ent->inuse)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			//	start think for playing frames
			ent->prethink = com->FrameUpdate;
			ent->nextthink = gstate->time + vt_thinkTime;
			ent->think = vt_Animate;
			ent->className = VM_ClassName;
			ent->className = "viewthing";
			vt_SetFrames (ent, &vt_Header);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_Rotate
//
//	rotate the viewmodel x degrees
///////////////////////////////////////////////////////////////////////////////

void	vt_Rotate (userEntity_t *self)
{
	userEntity_t	*ent;
	int				angle_add;

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: vr [degrees]\n");
		return;
	}

	angle_add = atof (gstate->GetArgv (1));

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->s.angles.y = AngleMod(ent->s.angles.y + angle_add);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_SetTranslucent
//
//	set the models translucency flags
///////////////////////////////////////////////////////////////////////////////

void	vt_SetTranslucent (userEntity_t *self)
{
	userEntity_t	*ent = NULL;
	float			alpha = 1.0f;
	bool			bReset = FALSE;

	if (gstate->GetArgc () != 2)
	{
		gstate->Con_Printf ("  USAGE: vt [alpha 0.0 - 1.0] or vt [nothing] to reset\n");
		gstate->Con_Printf ("  RESETTING view model translucency\n");
		bReset = TRUE;
	}
	else
	{
		alpha = atof (gstate->GetArgv (1));
		if (alpha < 0.0)
			alpha = 0.0f;
		else if (alpha > 1.0)
			alpha = 1.0f;
	}

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			if (bReset)
			{
				ent->s.alpha = 1.0;
				ent->s.renderfx &= ~RF_TRANSLUCENT;
			}
			else
			{

				ent->s.alpha = -alpha;
				ent->s.renderfx |= RF_TRANSLUCENT;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//	vt_ScaleModel
///////////////////////////////////////////////////////////////////////////////

void	vt_ScaleModel (userEntity_t *ent)
{
	int			count = gstate->GetArgc ();
	float		xscale, yscale, zscale;

	if (count != 2 && count != 4)
	{
		gstate->Con_Printf ("  USAGE: vscale [scale factor] or vscale [scale x] [scale y] [scale z]\n");
		return;
	}

	if (count == 2)
		xscale = yscale = zscale = atof (gstate->GetArgv (1));
	else
	{
		xscale = atof (gstate->GetArgv (1));
		yscale = atof (gstate->GetArgv (2));
		zscale = atof (gstate->GetArgv (3));
	}

	for (ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (!ent->className)
			continue;

		if (!stricmp (ent->className, "viewthing"))
		{
			ent->s.render_scale.Set( xscale, yscale, zscale );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	vt_ViewModelOnly
///////////////////////////////////////////////////////////////////////////////
void vt_ViewModelOnly(userEntity_t *ent)
{

	playerHook_t *hook = NULL;
	userEntity_t	*vm;
	int				modelIndex;
	char name[256];
    char sound[256];

	if (gstate->GetArgc () < 2)
	{
		gstate->Con_Printf ("  USAGE: vmm [modelpath and name]\nFor example: vmm e1/a_tazer.dkm\n");
		return;
	}
	
	vt_Stop (ent);
	//	find viewthing entity
	for (vm = gstate->FirstEntity (); vm && stricmp (vm->className, "viewthing"); vm = gstate->NextEntity (vm));

	if (!vm)
	{
		gstate->Con_Printf ("No viewthing on map.\n");
		return;
	}

//    sprintf( name, "models/%s", gstate->GetArgv(1) );
    Com_sprintf( name, sizeof(name),"models/%s", gstate->GetArgv(1) );
	if (gstate->GetArgc() > 2)
	{
//        sprintf( sound,"sounds/%s", gstate->GetArgv(2) );
        Com_sprintf( sound,sizeof(sound),"sounds/%s", gstate->GetArgv(2) );
	}
    else
	{
//        sprintf( sound,"sounds/%s", gstate->GetArgv(1) );
        Com_sprintf( sound,sizeof(sound),"sounds/%s", gstate->GetArgv(1) );
    }

	modelIndex = gstate->ModelIndex(name);

	//	set all viewthing frames to 0
	for ( vm = gstate->FirstEntity(); vm; vm = gstate->NextEntity(vm) )
	{
		if ( stricmp( vm->className, "viewthing") == 0 )
		{
			//	turn off all hierarchical models
			vm->s.frame = 0;
			vm->s.modelindex = modelIndex;
            break;
		}
	}

	seqHeader_t *header = &vt_Header;
	vt_GetSequences( vt, header );
	FRAMEDATA_ViewthingReadFile( ent, sound, header->list );

}

///////////////////////////////////////////////////////////////////////////////
//	vt_ViewModel
///////////////////////////////////////////////////////////////////////////////

void	vt_ViewModel (userEntity_t *ent)
{
	playerHook_t *hook = NULL;
	userEntity_t	*vm;
	char			Monster[256];
	int				modelIndex;

	if (gstate->GetArgc () < 2)
	{
		gstate->Con_Printf ("  USAGE: vm monster_[monster name]\nFor example: vm monster_froginator\n");
		return;
	}

	vt_Stop (ent);

	//	find viewthing entity
	for (vm = gstate->FirstEntity (); vm && stricmp (vm->className, "viewthing"); vm = gstate->NextEntity (vm));

	if (!vm)
	{
		gstate->Con_Printf ("No viewthing on map.\n");
		return;
	}
//	sprintf(Monster,"%s",gstate->GetArgv (1));
	Com_sprintf(Monster,sizeof(Monster),"%s",gstate->GetArgv (1));

	char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( Monster );
    if ( szCSVFileName )
    {
	    //Clear out MapAnimationToSequence
	    if ( vm->pMapAnimationToSequence )
	    {
		    delete vm->pMapAnimationToSequence;
		    vm->pMapAnimationToSequence = NULL;
	    }
	    //Setup a new one
	    if ( !vm->pMapAnimationToSequence )
        {
	        vm->pMapAnimationToSequence = new CMapStringToPtr;
            if ( !vm->pMapAnimationToSequence )
            {
                com->Error( "MEM allocation failed." );
            }
        }

	    ////////////////////////////////
	    // initialize user hook
	    ////////////////////////////////
	    hook = (playerHook_t *) vm->userHook;
	    if ( hook )
	    {
		    memset(hook,0,sizeof( playerHook_t));
	    }
//	    vm->userHook = new playerHook_t;
	    vm->userHook = gstate->X_Malloc( sizeof( playerHook_t ), MEM_TAG_HOOK );
	    if ( !vm->userHook )
	    {
		    com->Error( "MEM Allocation failed" );	
	    }	 
	    hook = AI_GetPlayerHook( vm );	
	    

	    FRAMEDATA_ReadFile( szCSVFileName, vm );

	    ai_register_sounds(vm);

	    
	    char *szModelName = AIATTRIBUTE_GetModelName( Monster );
        _ASSERTE( szModelName );
        vm->s.modelindex = gstate->ModelIndex( szModelName );
	    modelIndex = gstate->ModelIndex ( szModelName );
	    vm->s.frame = 0;
	    vm->s.modelindex = modelIndex;


	    sprintf(VM_ClassName,"%s",Monster);
	    vm->className = Monster;
	    vm->inventory = gstate->InventoryNew (MEM_MALLOC);
	    WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( vm );	
	    vm->className = "viewthing";
	    seqHeader_t		*header;

	    header = &vt_Header;

	    vt_GetSequences (vt, header);
	    
	    if(!FRAMEDATA_ViewthingReadFile( ent, szCSVFileName, header->list ))
	    {
		    AI_Dprintf("Failed to load Sounds!\n");
	    }
    }
}


///////////////////////////////////////////////////////////////////////////////
//	viewthing_init
//
//	checks to see if a client has connected and if so, it sets all the model
//	indexes for the viewthing's hierarchical parts to those of the client
//	
//	if no client has connected, the viewthing's think is set up to try again
//	on the next frame
///////////////////////////////////////////////////////////////////////////////

void	viewthing_init (userEntity_t *self)
{
	userEntity_t	*first_client;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	playerHook_t	*phook;

	//	check the first client to see if he is here...
	first_client = gstate->g_edicts;
	first_client++;

	if (first_client->inuse)
	{
		phook = AI_GetPlayerHook( first_client );

		if (phook)
		{
			self->nextthink = -1;

			return;
		}
	}

	self->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//	viewthing
//
//	spawn function
///////////////////////////////////////////////////////////////////////////////

void	viewthing (userEntity_t *self)
{
	playerHook_t	*hook;
	int				episode;
	char			sound[256];

//	self->userHook = new playerHook_t;
	self->userHook = gstate->X_Malloc( sizeof( playerHook_t ), MEM_TAG_HOOK );
	hook = AI_GetPlayerHook( self );

	//	set up all structures
	vt_InitList (&vt_Header, vt_List);
	episode = gstate->GetCvar ("sv_episode");

	//	load viewmodel based on episode
	if (episode == 1)
	{
		self->s.modelindex = gstate->ModelIndex ("models/e1/m_mwskny.dkm");
//		sprintf (sound,"sounds/e1/m_skinnyworker.csv");
		Com_sprintf (sound,sizeof(sound),"sounds/e1/m_skinnyworker.csv");
	}
	else if (episode == 2)
	{
		self->s.modelindex = gstate->ModelIndex ("models/e2/m_satyr.dkm");
//		sprintf (sound,"sounds/e2/m_satyr.csv");
		Com_sprintf (sound,sizeof(sound),"sounds/e2/m_satyr.csv");
	}
	else if (episode == 3)
	{
		self->s.modelindex = gstate->ModelIndex ("models/e3/m_lycanthir.dkm");
//		sprintf (sound,"sounds/e3/m_lycanthir.csv");
		Com_sprintf (sound,sizeof(sound),"sounds/e3/m_lycanthir.csv");
	}
	else if (episode == 4)
	{
		self->s.modelindex = gstate->ModelIndex ("models/e4/m_rockgang.dkm");
//		sprintf (sound,"sounds/e4/m_rockgang.csv");
		Com_sprintf (sound,sizeof(sound),"sounds/e4/m_rockgang.csv");
	}
/*
	else 
		self->s.modelindex = gstate->ModelIndex (debug_models [DEBUG_GOAL]);
*/	
	gstate->LinkEntity (self);

	self->think = viewthing_init;
	self->nextthink = gstate->time + 1.0;

	vt = self;

	//	get sequences here so that a vt_ViewSequences will work right off
	seqHeader_t		*header;

	header = &vt_Header;
	vt_GetSequences (vt, header);

	FRAMEDATA_ViewthingReadFile( self, sound, header->list );

}
//#endif
///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_viewthing_register_func()
{
	gstate->RegisterFunc("vt_StepFrame",vt_StepFrame);
	gstate->RegisterFunc("vt_Animate",vt_Animate);
	gstate->RegisterFunc("vt_DelayThink",vt_DelayThink);
	gstate->RegisterFunc("viewthing_init",viewthing_init);
}
