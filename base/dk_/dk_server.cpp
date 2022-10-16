#include "..\server\server.h"

// mdm99.03.15 -- wow, if this entire file is #def'd out, why do we still have it?

#if 0
#include "dk_model.h"

model_t	servermodels[MAX_MODELS];

model_t	*loadmodel;
int		modfilelen;

void dk_LoadAliasModel (model_t *mod, void *buffer);
void dk_LoadAliasExtra (model_t *mod);
int dk_FindExtraData (model_t *mod);

/*
=================
dk_LoadAliasModel
=================
*/
void dk_LoadAliasModel (model_t *mod, void *buffer)
{
	int					i, j;
	dmdl_t				*pinmodel, *pheader;
	dstvert_t			*pinst, *poutst;
	dtriangle_t			*pintri, *pouttri;
	daliasframe_t		*pinframe, *poutframe;
	int					*pincmd, *poutcmd;
	int					version;
	int					extdatasize;

	pinmodel = (dmdl_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

	extdatasize = dk_FindExtraData (mod);

	pheader = X_Malloc(LittleLong(pinmodel->ofs_end + extdatasize), MEM_TAG_MODEL);
	
	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dmdl_t)/4 ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	if (pheader->skinheight > MAX_LBM_HEIGHT)
		Sys_Error ("model %s has a skin taller than %d", mod->name,
				   MAX_LBM_HEIGHT);

	if (pheader->num_xyz <= 0)
		Sys_Error ("model %s has no vertices", mod->name);

	if (pheader->num_xyz > MAX_VERTS)
		Sys_Error ("model %s has too many vertices", mod->name);

	if (pheader->num_st <= 0)
		Sys_Error ("model %s has no st vertices", mod->name);

	if (pheader->num_tris <= 0)
		Sys_Error ("model %s has no triangles", mod->name);

	if (pheader->num_frames <= 0)
		Sys_Error ("model %s has no frames", mod->name);

//
// load base s and t vertices (not used in gl version)
//
	pinst = (dstvert_t *) ((byte *)pinmodel + pheader->ofs_st);
	poutst = (dstvert_t *) ((byte *)pheader + pheader->ofs_st);

	for (i=0 ; i<pheader->num_st ; i++)
	{
		poutst[i].s = LittleShort (pinst[i].s);
		poutst[i].t = LittleShort (pinst[i].t);
	}

//
// load triangle lists
//
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_st[j] = LittleShort (pintri[i].index_st[j]);
		}
	}

//
// load the frames
//
	for (i=0 ; i<pheader->num_frames ; i++)
	{
		pinframe = (daliasframe_t *) ((byte *)pinmodel 
			+ pheader->ofs_frames + i * pheader->framesize);
		poutframe = (daliasframe_t *) ((byte *)pheader 
			+ pheader->ofs_frames + i * pheader->framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
		}
		// verts are all 8 bit, so no swapping needed
		memcpy (poutframe->verts, pinframe->verts, 
			pheader->num_xyz*sizeof(dtrivertx_t));

	}

	mod->type = mod_alias;

	//
	// load the glcmds
	//
	pincmd = (int *) ((byte *)pinmodel + pheader->ofs_glcmds);
	poutcmd = (int *) ((byte *)pheader + pheader->ofs_glcmds);
	for (i=0 ; i<pheader->num_glcmds ; i++)
		poutcmd[i] = LittleLong (pincmd[i]);


/*
	// register all skins
	memcpy ((char *)pheader + pheader->ofs_skins, (char *)pinmodel + pheader->ofs_skins,
		pheader->num_skins*MAX_SKINNAME);
	for (i=0 ; i<pheader->num_skins ; i++)
	{
		sprintf (name, "%s", (char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME);
		mod->skins[i] = GL_FindImage (name, it_skin);
	}
*/
}

/*
=================
dk_FindExtraData
=================
*/
int dk_FindExtraData (model_t *mod)
{
	char		filename[1024];
	unsigned	*buffer;
	int			size;

	strcpy (filename, mod->name);
	filename [strlen (filename) - 4] = 0x00;
	strcat (filename, ".ext");

	size = FS_LoadFile (filename, &buffer);

	if (size == -1)
		return 0;

	FS_FreeFile (buffer);

	return size;
}

/*
=================
dk_LoadAliasExtra
=================
*/
void dk_LoadAliasExtra (model_t *mod)
{
	int			i;
	dmdl_t		*paliashdr;
	extinfo_t	*pin, *pheader;
	extsurf_t	*pinsurf, *poutsurf;
	extpoly_t	*pinpoly, *poutpoly;
	int			*pincmd, *poutcmd;
	char		filename[1024];
	unsigned	*buffer;
	int			id, version;
		
	strcpy (filename, mod->name);
	filename [strlen (filename) - 4] = 0x00;
	strcat (filename, ".ext");

	if ((FS_LoadFile (filename, &buffer)) == -1)
		return;

	id = LittleLong(*(unsigned *)buffer);

	if (id != IDEXTRAHEADER)
	{
		Com_Printf ("Could not find 'XTRA' id in: %s\n", filename);
		return;
	}

	Com_Printf ("Found extra data for: %s\n", mod->name);

	pin = (extinfo_t *)buffer;

	version = LittleLong (pin->version);
	if (version != EXTRA_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

	paliashdr = (dmdl_t *) ((byte *)mod->extradata);
	pheader = (extinfo_t *) ((byte *) paliashdr + paliashdr->ofs_end);

	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(extinfo_t)/4 ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	pinsurf = (extsurf_t *) ((byte *)pin + pheader->ofs_surf);
	poutsurf = (extsurf_t *) ((byte *)pheader + pheader->ofs_surf);
	for (i = 0; i < pheader->num_surfs; i++)
	{
		memcpy (poutsurf[i].name, pinsurf[i].name, 32);
		poutsurf[i].skinindex	= pinsurf[i].skinindex;
		poutsurf[i].skinwidth	= pinsurf[i].skinwidth;
		poutsurf[i].skinheight	= pinsurf[i].skinheight;
		poutsurf[i].num_frames	= pinsurf[i].num_frames;
		poutsurf[i].num_polys	= pinsurf[i].num_polys;
		poutsurf[i].flags		= pinsurf[i].flags;
		poutsurf[i].normal[0]	= pinsurf[i].normal[0];
		poutsurf[i].normal[1]	= pinsurf[i].normal[1];
		poutsurf[i].normal[2]	= pinsurf[i].normal[2];
	}

	pinpoly = (extpoly_t*) ((byte *)pin + pheader->ofs_poly);
	poutpoly = (extpoly_t *) ((byte *)pheader + pheader->ofs_poly);
	for (i = 0; i < pheader->num_polys; i++)
	{
		poutpoly[i].surfid		= pinpoly[i].surfid;
		poutpoly[i].index_xyz	= pinpoly[i].index_xyz;
	}

	pincmd = (int *) ((byte *) pin + pheader->ofs_glcmds);
	poutcmd = (int *) ((byte *) pheader + pheader->ofs_glcmds);
	for (i = 0; i < pheader->num_glcmds; i++)
		poutcmd[i] = pincmd[i];

	FS_FreeFile (buffer);
}

/*
================
dk_LoadServerModel
================
*/
void dk_LoadServerModel (int modelnum, char *name)
{
	model_t		*mod;
	unsigned	*buf;

	if ((!name[0]) || (name[0] == '*'))
		return;

	mod = &servermodels[modelnum];

	strcpy (mod->name, name);

	modfilelen = FS_LoadFile (mod->name, &buf);
	if (!buf)
	{
		memset (mod->name, 0, sizeof(mod->name));
		return;
	}

	loadmodel = mod;

	switch (LittleLong(*(unsigned *)buf))
	{
	case IDALIASHEADER:
//		loadmodel->extradata = Hunk_Begin (0x200000);
		dk_LoadAliasModel (mod, buf);
		dk_LoadAliasExtra (mod);
		break;
		
	case IDSPRITEHEADER:
		return;
	
	case IDBSPHEADER:
		return;

	default:
		return;
	}

//	loadmodel->extradatasize = Hunk_End ();

	FS_FreeFile (buf);
}
#endif
/*
================
dk_FrameNameFromServer
================
*/
void dk_FrameNameFromServer (int modelindex, int frameindex, char *name)
{
/*
	struct model_s	*mod;
	dmdl_t			*paliashdr;
	daliasframe_t	*paliasframe;
	
	mod = &servermodels[modelindex];

	paliashdr = (dmdl_t *) mod->extradata;

	paliasframe = (daliasframe_t *) ((byte *) paliashdr + paliashdr->ofs_frames);

	strcpy (name, paliasframe[frameindex].name);
*/
}
