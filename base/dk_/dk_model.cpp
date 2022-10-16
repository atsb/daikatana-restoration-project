#include "qcommon.h"
#include "client.h"
#include "server.h"
#include "daikatana.h"

void dk_FrameNameFromServer (int modelindex, int frameindex, char *name);
void dk_FrameNameFromClient (int modelindex, int frameindex, char *name);

// Shawn:
///////////////////////////////////////////////////////////////////////////////
//
// ADDED FOR SERVER SIDE MODEL INFORMATION
//
///////////////////////////////////////////////////////////////////////////////

typedef	struct	modelFrames_s
{
	char	Name [16];
} modelFrames_t;

typedef	struct	serverModel_s
{
	char			*Name;
	int				ModelIndex;
	int				NumFrames;
	modelFrames_t	*Frames;
} serverModel_t;

static	serverModel_t	serverModel;


///////////////////////////////////////////////////////////////////////////////
//	dk_GetModelSkinIndex
//
///////////////////////////////////////////////////////////////////////////////
char*	dk_GetModelSkinName(int modelIndex)
{
#ifdef DEDICATED_ONLY
	return 0;
#else
	void *model = NULL;
	
	if (modelIndex==0)
		return (0);

	// get a pointer to the model
	model = cl.model_draw [modelIndex];
	
	if (model)
	{
		// call the function in the renderer to get the skin index
		return re.GetModelSkinName (model);
	}
	else
	{
		return NULL;
	}
#endif
}

int		dk_GetModelSkinIndex(int modelIndex)
{
#ifdef DEDICATED_ONLY
	return 0;
#else
	void *model = NULL;
	
	if (modelIndex==0)
		return (0);

	// get a pointer to the model
	model = cl.model_draw [modelIndex];
	
	if (model)
	{
		// call the function in the renderer to get the skin index
		int skinIndex = re.GetModelSkinIndex (model);
		return (skinIndex);
	}
	else
	{
		return (0);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	dk_FreeServerModel
//
///////////////////////////////////////////////////////////////////////////////

void	dk_FreeServerModel (serverModel_t *sModel)
{
	if (sModel->Name != NULL)
		// mdm99.03.10 - memmgr change
		X_Free (sModel->Name);
	sModel->ModelIndex = -1;
	
	if (sModel->Frames)
		// mdm99.03.11 - memmgr change
		X_Free (sModel->Frames);
}

//unix - byte-order fix - from mike at mplayer
///////////////////////////////////////////////////////////////////////////////
//	dk_LittleDmdl
//
///////////////////////////////////////////////////////////////////////////////
void	dk_LittleDmdl (dmdl_t *dmdl)
{
	dmdl->ident 		= LittleLong (dmdl->ident);
	dmdl->version 		= LittleLong (dmdl->version);

	dmdl->org[0] 		= LittleLong (dmdl->org[0]);
	dmdl->org[1] 		= LittleLong (dmdl->org[1]);
	dmdl->org[2] 		= LittleLong (dmdl->org[2]);

	dmdl->framesize 	= LittleLong (dmdl->framesize);

	dmdl->num_skins 	= LittleLong (dmdl->num_skins);
	dmdl->num_xyz 		= LittleLong (dmdl->num_xyz);
	dmdl->num_st 		= LittleLong (dmdl->num_st);
	dmdl->num_tris 		= LittleLong (dmdl->num_tris);
	dmdl->num_glcmds 	= LittleLong (dmdl->num_glcmds);
	dmdl->num_frames 	= LittleLong (dmdl->num_frames);
	dmdl->num_surfaces 	= LittleLong (dmdl->num_surfaces);

	dmdl->ofs_skins 	= LittleLong (dmdl->ofs_skins);
	dmdl->ofs_st 		= LittleLong (dmdl->ofs_st);
	dmdl->ofs_tris 		= LittleLong (dmdl->ofs_tris);
	dmdl->ofs_frames 	= LittleLong (dmdl->ofs_frames);
	dmdl->ofs_glcmds 	= LittleLong (dmdl->ofs_glcmds);
	dmdl->ofs_surfaces 	= LittleLong (dmdl->ofs_surfaces);
	dmdl->ofs_end 		= LittleLong (dmdl->ofs_end);

	dmdl->num_sequences	= LittleLong (dmdl->num_sequences);
	dmdl->ofs_sequences = LittleLong (dmdl->ofs_sequences);
}

///////////////////////////////////////////////////////////////////////////////
//	dk_LoadServerModel
//
///////////////////////////////////////////////////////////////////////////////

qboolean	dk_LoadServerModel (serverModel_t *sModel, int ModelIndex)
{
	FILE			*f;
	byte			*buffer;
	char			fName [MAX_OSPATH];
	int				fLen, fStartPos, fFrameStartPos;
	dmdl_t			modelHeader;
	daliasframe_t	*FramePtr;
	daliasframe2_t	*Frame2Ptr;
	int				i, frameDataSize;

	if (ModelIndex == sModel->ModelIndex)
		return true;

	if (sModel->ModelIndex >= 0)
		//	there is already memory allocated for the last model, so free it
		dk_FreeServerModel (sModel);

// SCG[1/16/00]: 	sprintf (fName, "%s", sv.configstrings [CS_MODELS + ModelIndex]);
	Com_sprintf (fName,sizeof(fName), "%s", sv.configstrings [CS_MODELS + ModelIndex]);
	fLen = FS_FOpenFile (fName, &f);

	if (fLen < 0)
		return	false;

	sModel->ModelIndex = ModelIndex;
	// mdm99.03.10 - memmgr
	sModel->Name = (char *) X_Malloc (strlen (fName) + 1, MEM_TAG_MODEL);
	strcpy (sModel->Name, fName);

	fStartPos = FS_Tell (f);

	FS_Read (&modelHeader, sizeof (dmdl_t), f);
	//unix - byte-order fix
	dk_LittleDmdl (&modelHeader);
	fFrameStartPos = fStartPos + modelHeader.ofs_frames;

	//	malloc up the memory for all the modelFrames
	sModel->NumFrames = modelHeader.num_frames;
	// mdm99.03.10 - memmgr
	sModel->Frames = (modelFrames_t *) X_Malloc (sizeof (modelFrames_t) * (sModel->NumFrames + 1), MEM_TAG_MODEL);

#ifdef	READ_ONE_FRAME_AT_A_TIME
	//	read one frame at a time...
	for (i = 0; i < modelHeader.num_frames; i++)
	{
		FS_Seek (f, fFrameStartPos + modelHeader.framesize * i, SEEK_SET);
		FS_Read (&modelFrame, sizeof (daliasframe_t), f);

		strcpy (sModel->Frames [i].Name, modelFrame.name);
	}
#else
	//	read all the frame data in one big lump for speed... take a little more
	//	memory, but it gets freed immediately after
	frameDataSize = modelHeader.framesize * modelHeader.num_frames;
	// mdm99.03.10 - memmgr
	buffer = (byte *)X_Malloc (frameDataSize, MEM_TAG_MODEL);
	FS_Seek (f, fFrameStartPos, SEEK_SET);
	FS_Read (buffer, frameDataSize, f);

	if( modelHeader.version == 2 )
	{
		for (i = 0; i < modelHeader.num_frames; i++)
		{
			Frame2Ptr = (daliasframe2_t *) ((byte *) buffer + modelHeader.framesize * i);
			strcpy (sModel->Frames [i].Name, Frame2Ptr->name);
		}
	}
	else
	{
		for (i = 0; i < modelHeader.num_frames; i++)
		{
			FramePtr = (daliasframe_t *) ((byte *) buffer + modelHeader.framesize * i);
			strcpy (sModel->Frames [i].Name, FramePtr->name);
		}
	}

	X_Free (buffer);
#endif

	FS_FCloseFile (f);

	return	true;
}

/*
================
dk_GetFrameName

//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
================
*/
void dk_GetFrameName (int modelindex, int frameindex, char *framename)
{
	if (!dk_LoadServerModel (&serverModel, modelindex) || frameindex > serverModel.NumFrames)
	{
		framename [0] = 0x00;
		return;
	}

	strcpy (framename, serverModel.Frames [frameindex].Name);

/*
	//	this is the old, bad way, that uses client-side data
	char	modelname[128];

	strcpy (modelname, sv.configstrings[CS_MODELS+modelindex]);

	if (modelname [0] == 0x00)
	{
		framename[0] = 0x00;
		return;
	}
	
	re.GetFrameName (modelname, frameindex, framename);
*/
}

///////////////////////////////////////////////////////////////////////////////
//	dk_GetAnimSequences
//
//	- gets the animation sequence data out of a model
//	I DON'T THINK THIS WILL NOT WORK IN DEDICATED MODE!!!
///////////////////////////////////////////////////////////////////////////////

int	dk_GetAnimSequences	(int modelindex, frameData_t *pFrameData)
{
	FILE		*f = NULL;
	dmdl_t		modelHeader;
	animSeq_t	*pSeqData = NULL;
	char		fName [MAX_OSPATH];
	int			fLen, fStartPos, fSequenceStartPos, nNumSequences = 0, nSeqDataSize = 0;

// SCG[1/16/00]: 	sprintf (fName, "%s", sv.configstrings [CS_MODELS + modelindex]);
	Com_sprintf (fName,sizeof(fName), "%s", sv.configstrings [CS_MODELS + modelindex]);
	fLen = FS_FOpenFile (fName, &f);

	// error opening file?
	if (fLen < 0)
	{
		return	(0);
	}

	// read the header (remember, FS_Tell here b/c the file could be in a pack file)
	fStartPos = FS_Tell (f);
	// fill in the header record
	FS_Read (&modelHeader, sizeof (dmdl_t), f);
	//unix - byte-order fix
	dk_LittleDmdl (&modelHeader);

	// fill in the number of animation sequences contained in this model
	nNumSequences = modelHeader.num_sequences;		

	// if pFrameData is NULL, then just return the number of anim sequences
	if (pFrameData)
	{
		// find out where to read from next...
		fSequenceStartPos = fStartPos + modelHeader.ofs_sequences;
		nSeqDataSize = nNumSequences * sizeof(animSeq_t);

		if (nSeqDataSize > 0)
		{
			// allocate the memory for the temporary anim sequence data
			pSeqData = (animSeq_t*) X_Malloc (nSeqDataSize,MEM_TAG_MODEL);		// mdm 99.03.11 - memmgr change

			// check the pointer!
			if (pSeqData)
			{
				FS_Seek (f, fSequenceStartPos, SEEK_SET);
				FS_Read ((byte*)pSeqData, nSeqDataSize, f);

				// data is now read in.. fill in the frameData_t records
				for (int seqIdx = 0; seqIdx < nNumSequences; seqIdx++)
				{
					strcpy( pFrameData[seqIdx].animation_name, pSeqData[seqIdx].seqName );
					//unix - byte-order fix
					pFrameData[seqIdx].first = LittleLong (pSeqData[seqIdx].startFrame);
					pFrameData[seqIdx].last = LittleLong (pSeqData[seqIdx].endFrame);

					// fill in defaults for the other fields
					pFrameData[seqIdx].flags = 0;
					
					pFrameData[seqIdx].soundframe1 = -1;
					pFrameData[seqIdx].soundframe2 = -1;
				}
				// all data filled in... free the temporary sequence data
				X_Free (pSeqData);		// mdm 99.03.11 - memmgr change
			}
		}
	}

	FS_FCloseFile (f);

	return (nNumSequences);
}
 
///////////////////////////////////////////////////////////////////////////////
//	dk_ForceModelLoad
//
//	forces a model at a specific index to be reloaded at any time
//	THIS WILL NOT WORK IN DEDICATED MODE!!!
///////////////////////////////////////////////////////////////////////////////

void	dk_ForceModelLoad (int modelindex, char *name)
{
	void	*model;

	//	FIXME:	free model before reloading
	if (dedicated->value)
		return;

#ifndef DEDICATED_ONLY
	//	make sure we free the frames so that viewthings work correctly!
	if (serverModel.ModelIndex == modelindex)
		dk_FreeServerModel (&serverModel);

	strcpy (cl.configstrings [CS_MODELS + modelindex], name);
	strcpy (sv.configstrings [CS_MODELS + modelindex], name);

	//	free old model
	model = cl.model_draw [modelindex];
	if (model)
		re.Mod_Free (model);
	//	load model
	cl.model_draw [modelindex] = re.RegisterModel (cl.configstrings [CS_MODELS + modelindex], RESOURCE_LEVEL);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	dk_PreCacheModel
//
//	forces a model at a specific index to be pre-loaded
///////////////////////////////////////////////////////////////////////////////

void	dk_PreCacheModel (int modelindex, char *name)
{
	// FIXME:  oh well.. 
	if (dedicated->value)
		return;
	
#ifndef DEDICATED_ONLY
	// if it already has a value, assume it's been loaded
	if (cl.model_draw [modelindex])
		return;

	// just to be sure
	_ASSERTE( cl.configstrings[CS_MODELS + modelindex][0] == NULL );
	_ASSERTE( sv.configstrings[CS_MODELS + modelindex][0] == NULL );
	strcpy (cl.configstrings [CS_MODELS + modelindex], name);
	strcpy (sv.configstrings [CS_MODELS + modelindex], name);

	//	load model
	cl.model_draw [modelindex] = re.RegisterModel (cl.configstrings [CS_MODELS + modelindex], RESOURCE_LEVEL);
#endif
}


///////////////////////////////////////////////////////////////////////////////
//	dk_VertInfo
//
//	returns the transformed location of a specified vertex in a specified
//	surface for the specified frame of the specified entity, got that?
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
///////////////////////////////////////////////////////////////////////////////

int	dk_VertInfo (edict_t *ent, int surfIndex, int vertIndex, CVector &vert)
{
#ifdef DEDICATED_ONLY
	return 0;
#else
	void	*mod;

	Sys_Error ("dk_VertInfo called!  This will crash dedicated servers!\n");

	mod = cl.model_draw [ent->s.modelindex];

	if(!mod)
		return 0;

//	return re.VertInfo (mod, surfIndex, ent->s.frame, vertIndex, ent->s.origin, ent->s.old_origin, ent->s.angles, vert);
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	dk_SurfIndex
//
//	returns an index for the passed surface name
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
///////////////////////////////////////////////////////////////////////////////

int	dk_SurfIndex (int modelIndex, char *surfName)
{
#ifdef DEDICATED_ONLY
	return 0;
#else
	void	*mod;

	Sys_Error ("dk_SurfIndex called!  This will crash dedicated servers!\n");

	mod = cl.model_draw [modelIndex];

	if(!mod)
		//	so hierarchical code knows that model's haven't been loaded
		return 0;

//	return	re.SurfIndex (mod, surfName);
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	dk_TriVerts
//
//	returns three transformed vertices
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
//////////////////////////////////////////////////////////////////////

int	dk_TriVerts (edict_t *ent, int surfIndex, int vertIndex, CVector &v1, CVector &v2, CVector &v3)
{
#ifdef DEDICATED_ONLY
	return 0;
#else
	void	*mod;

	Sys_Error ("dk_TriVerts called!  This will crash dedicated servers!\n");

	mod = cl.model_draw [ent->s.modelindex];

	if(!mod)
		return 0;

//	return	re.TriVerts (mod, surfIndex, ent->s.frame, vertIndex, ent->s.origin, ent->s.old_origin, ent->s.angles, v1, v2, v3);
	return 0;
#endif
}

#if 0
/*
///////////////////////////////////////////////////////////////////////////////
//	dk_Trace
//
///////////////////////////////////////////////////////////////////////////////

client_trace_t	dk_Trace (const CVector &origin, const CVector &mins, const CVector &maxs, const CVector &end, int passent_num, int clipmask)
{
	edict_t			*passedict;
	trace_t			trace;
	client_trace_t	ctrace;

	//	make sure we're connected to the server (ie. the server is set up)
	if (cls.state != ca_active || sv.state != ss_game)
	{
		memset (&ctrace, 0x00, sizeof (client_trace_t));
		return	ctrace;
	}

	passedict = EDICT_NUM (passent_num);

	//	Nelno:	pass a real clip mask from client
	clipmask = MASK_SOLID;

	trace = SV_Trace(origin, mins, maxs, end, passedict, clipmask);

	ctrace.allsolid = trace.allsolid;
	ctrace.startsolid = trace.startsolid;
	ctrace.fraction = trace.fraction;
	ctrace.endpos = trace.endpos;
	ctrace.plane = trace.plane;
	ctrace.surface = trace.surface;
	ctrace.contents = trace.contents;
	ctrace.ent = NUM_FOR_EDICT (trace.ent);

	return	ctrace;
}
*/

#endif 0



///////////////////////////////////////////////////////////////////////////////
//	dk_GetSpecficAnimSequences
//
//	- gets the animation sequence data out of a model
//	I DON'T THINK THIS WILL NOT WORK IN DEDICATED MODE!!!
///////////////////////////////////////////////////////////////////////////////

int	dk_GetSpecificAnimSequences(int modelindex, char *fName, frameData_t *pFrameData, char *seqName)
{
	FILE		  *f = NULL;
	dmdl_t		modelHeader;
	animSeq_t	*pSeqData = NULL;
	int			  fLen, fStartPos, fSequenceStartPos, nNumSequences = 0, nSeqDataSize = 0;
	

  if (modelindex > -1) // if modelindex specified, find filename in config strings
// SCG[1/16/00]:     sprintf (fName, "%s", sv.configstrings [CS_MODELS + modelindex]);
    Com_sprintf (fName,sizeof(fName), "%s", sv.configstrings [CS_MODELS + modelindex]);

  fLen = FS_FOpenFile(fName, &f);

	if (fLen < 0)  	// error opening file?
		return(0);

	fStartPos = FS_Tell (f);   	// read the header (remember, FS_Tell here b/c the file could be in a pack file)
	FS_Read (&modelHeader, sizeof (dmdl_t), f);  // fill in the header record
	
	dk_LittleDmdl (&modelHeader);                //unix - byte-order fix

	// fill in the number of animation sequences contained in this model
	nNumSequences = modelHeader.num_sequences;		

  // find out where to read from next
  fSequenceStartPos = fStartPos + modelHeader.ofs_sequences;
	nSeqDataSize = nNumSequences * sizeof(animSeq_t);

	if (nSeqDataSize > 0)
	{
	  // allocate the memory for the temporary anim sequence data
		pSeqData = (animSeq_t*) X_Malloc (nSeqDataSize,MEM_TAG_MODEL);

		// check the pointer!
		if (pSeqData)
		{
		  FS_Seek (f, fSequenceStartPos, SEEK_SET);
			FS_Read ((byte*)pSeqData, nSeqDataSize, f);

			// data is now read in.. fill in the frameData_t records
			for (int seqIdx = 0; seqIdx < nNumSequences; seqIdx++)
			{
			  if (!stricmp(pSeqData[seqIdx].seqName,seqName)) // found animation sequence name match?
				{
				  strcpy( pFrameData->animation_name, pSeqData[seqIdx].seqName );

				  pFrameData->first = LittleLong (pSeqData[seqIdx].startFrame);
				  pFrameData->last = LittleLong (pSeqData[seqIdx].endFrame);

				  // fill in defaults for the other fields
				  pFrameData->flags = 0;
					
				  pFrameData->soundframe1 = -1;
				  pFrameData->soundframe2 = -1;

          break; // break out of loop since we've found the goods
				}
			}
			
      // all data filled in... free the temporary sequence data
			X_Free (pSeqData);
		}
	}

	FS_FCloseFile (f);

	return (nNumSequences);
}

