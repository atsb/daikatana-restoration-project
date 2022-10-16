void R_FrameNameForModel( char *modelname, int frameindex, char *framename )
{
	Sys_Error( "R_FrameNameForModel no longer supported!\n" );
}



void Mod_Modellist_f( void )
{
	int		i;
	model_t	*mod;
	int		total;

	total = 0;
	ri.Con_Printf (PRINT_ALL,"Loaded models:\n");
	for( i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		if( mod->name[0] == NULL )
		{
			continue;
		}
		ri.Con_Printf( PRINT_ALL, "%8i : %s\n",mod->extradatasize, mod->name );
		total += mod->extradatasize;
	}
	ri.Con_Printf( PRINT_ALL, "Total resident: %i\n", total );
}



void Mod_Init( void )
{
	memset( mod_novis, 0xff, sizeof( mod_novis ) );
}



model_t *Mod_ForName( const char *name, qboolean crash, resource_t resource )
{
	model_t		*mod;
	unsigned	*buf;
	int			i, forced_index = -1;
	int			is_world_model = NOT_WORLD_MODEL;
	char		filename[256];
	
	if( name[0] == NULL)
	{
		ri.Sys_Error( ERR_DROP, "Mod_ForName: NULL name" );
	}

	if( strstr( name, ".bsp" ) )
	{
		is_world_model = IS_WORLD_MODEL;
	}

	//
	// inline models are grabbed only from worldmodel
	//
	if( name[0] == '*' )
	{
		i = atoi( name + 1 );
		if( i < 1 || ( r_worldmodel == NULL ) || ( i >= r_worldmodel->numsubmodels ) )
		{
			ri.Sys_Error( ERR_DROP, "bad inline model number" );
		}
		return &mod_inline[i];
	}

	//
	//	search the currently loaded models
	//

	//	Nelno:	world model can ONLY load at mod_known, nothing else can 
	//	load there or it causes major problems.  This was not a problem 
	//	with Quake 2 because the world model was always guaranteed to be
	//	the first model loaded, but with Daikatana that is not the case
	//	because R_FrameNameForModel can load a model before the .bsp is
	//	loaded.
	if( is_world_model )
	{
		mod = mod_known;

		//	if world model has not changed, just return the current one
		if( !strcmp( mod->name, name ) )
		{
			return	mod;
		}

		// SCG 3-17-99
		// this was not in GL but it was in software, 
		// find out what it's for
//		forced_index = 0;
	}
	else
	{
		for( i = 0, mod = mod_known + 1; i < mod_numknown; i++, mod++ )
		{
			if( mod->name[0] == NULL )
			{
				continue;
			}
			
			//	Nelno:	reload this model or return it as found
			if( !strcmp( mod->name, name ) )
			{
				return	mod;
			}
		}

		//	find a free model slot spot
		//
		//	Nelno:	start looking after mod_known, because that has to
		//	be the world model!!
		for( i = 0, mod = mod_known + 1; i < mod_numknown; i++, mod++ )
		{
			if( mod->name[0] == NULL )
			{
				break;	// free spot
			}
		}

		if( i >= mod_numknown )
		{
			if( mod_numknown == MAX_MOD_KNOWN )
			{
				ri.Sys_Error( ERR_DROP, "mod_numknown == MAX_MOD_KNOWN" );
			}
			mod_numknown++;
		}
	}

	strcpy( filename, name );
	for( i = 0; i < strlen( filename ); i++ )
	{
		if( filename[i] == '\\' )
			filename[i] = '/';
	}

	strcpy( mod->name, filename );
	mod->resource = resource;
	
	//
	// load the file
	//
	modfilelen = ri.FS_LoadFile( mod->name, ( void ** ) &buf );
	if( buf == NULL )
	{
		if( crash )
		{
			ri.Sys_Error( ERR_DROP, "Mod_NumForName: %s not found", mod->name );
		}
		memset( mod->name, 0, sizeof( mod->name ) );
		return NULL;
	}
	
	loadmodel = mod;

	//
	// fill it in
	//

	// call the apropriate loader
	switch (LittleLong( * ( unsigned * ) buf) )
	{
		case IDALIASHEADER:
			loadmodel->extradata = Hunk_Begin( 0x200000 );
			Mod_LoadAliasModel( mod, buf, resource );
			break;
			
		case IDSPRITEHEADER:
			loadmodel->extradata = Hunk_Begin( 0x10000 );
			Mod_LoadSpriteModel( mod, buf, resource );
			break;
		
		case IDBSPHEADER:
			loadmodel->extradata = Hunk_Begin( 0x1100000 );  //$$$ FIXMEAMW		s/b 0x1000000
			Mod_LoadBrushModel( mod, buf );
			break;

		default:
			ri.Con_Printf( PRINT_ALL, "Mod_NumForName: unknown field for %s\n", mod->name );
			ri.FS_FreeFile( buf );
			mod_numknown--;
			return NULL;
			break;
	}

	loadmodel->extradatasize = Hunk_End();

	if( mod == mod_known )
	{
		ri.Con_Printf( PRINT_ALL, "world size = %i\n", loadmodel->extradatasize );
	}

	ri.FS_FreeFile( buf );

	return mod;
}



/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;

float RadiusFromBounds( CVector &mins, CVector &maxs )
{
	CVector	corner;

	corner.x = fabs( mins.x ) > fabs( maxs.x ) ? fabs( mins.x ) : fabs( maxs.x );
	corner.y = fabs( mins.y ) > fabs( maxs.y ) ? fabs( mins.y ) : fabs( maxs.y );
	corner.z = fabs( mins.z ) > fabs( maxs.z ) ? fabs( mins.z ) : fabs( maxs.z );

	return corner.Length();
}



void Mod_LoadLighting( lump_t *l )
{
	if( l->filelen == NULL )
	{
		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata = ( byte * ) Hunk_Alloc( l->filelen );
	memcpy( loadmodel->lightdata, mod_base + l->fileofs, l->filelen );
}



void Mod_LoadVisibility( lump_t *l )
{
	int		i;

	if( l->filelen == NULL )
	{
		loadmodel->vis = NULL;
		return;
	}

#ifdef	CONSOLIDATE_BSP
	loadmodel->vis = ri.bspModel->dVis;
	return;
#endif

	loadmodel->vis = ( dvis_t * ) Hunk_Alloc( l->filelen );	
	memcpy( loadmodel->vis, mod_base + l->fileofs, l->filelen );

	loadmodel->vis->numclusters = LittleLong( loadmodel->vis->numclusters );
	for( i = 0; i < loadmodel->vis->numclusters; i++ )
	{
		loadmodel->vis->bitofs[i][0] = LittleLong( loadmodel->vis->bitofs[i][0] );
		loadmodel->vis->bitofs[i][1] = LittleLong( loadmodel->vis->bitofs[i][1] );
	}
}



void Mod_LoadVertexes( lump_t *l )
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = ( dvertex_t * ) ( mod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) )
	{
		ri.Sys_Error( ERR_DROP, "MOD_LoadVertexes: funny lump size in %s",loadmodel->name );
	}
	count = l->filelen / sizeof( *in );
	out = ( mvertex_t * ) Hunk_Alloc( count*sizeof( mvertex_t ) );	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for( i = 0; i < count; i++, in++, out++ )
	{
		out->position.x = LittleFloat( in->point[0] );
		out->position.y = LittleFloat( in->point[1] );
		out->position.z = LittleFloat( in->point[2] );
	}
}



void Mod_LoadExtendedSurfInfo( lump_t *l )
{
	float		*in;
	mtexinfo_t	*out;
	int			i, count;

	in = ( float * )( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		ri.Sys_Error( ERR_DROP, "MOD_LoadExtendedSurfInfo: funny lump size in %s",loadmodel->name );
	}

	count = loadmodel->numtexinfo;
	out = loadmodel->texinfo;

	for( i = 0; i < count; i++, out++ )
	{
		out->color.x = LittleFloat( *in );
		in++;
		out->color.y = LittleFloat( *in );
		in++;
		out->color.z = LittleFloat( *in );
		in++;
	}
}



void Mod_LoadSubmodels( lump_t *l )
{
	dmodel_t	*in;
	mmodel_t	*out;
	int			i, count;

	in = ( dmodel_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) )
	{
		ri.Sys_Error( ERR_DROP, "MOD_LoadSubmodels: funny lump size in %s",loadmodel->name );
	}
	count = l->filelen / sizeof( *in );
	out = ( mmodel_t * ) Hunk_Alloc( count * sizeof( *out ) );	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for( i = 0; i < count; i++, in++, out++ )
	{
		// spread the mins / maxs by a pixel
		out->mins.x = LittleFloat( in->mins.x ) - 1;
		out->mins.y = LittleFloat( in->mins.y ) - 1;
		out->mins.z = LittleFloat( in->mins.z ) - 1;

		out->maxs.x = LittleFloat( in->maxs.x ) + 1;
		out->maxs.y = LittleFloat( in->maxs.y ) + 1;
		out->maxs.z = LittleFloat( in->maxs.z ) + 1;

		out->origin.x = LittleFloat( in->origin.x );
		out->origin.y = LittleFloat( in->origin.y );
		out->origin.z = LittleFloat( in->origin.z );

		out->radius = RadiusFromBounds( out->mins, out->maxs );
		out->headnode = LittleLong( in->headnode );
		out->firstface = LittleLong( in->firstface );
		out->numfaces = LittleLong( in->numfaces );
	}
}



void Mod_LoadEdges( lump_t *l )
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = ( dedge_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) )
	{
		ri.Sys_Error( ERR_DROP, "MOD_LoadEdges: funny lump size in %s",loadmodel->name );
	}
	count = l->filelen / sizeof( *in );
	out = ( medge_t * ) Hunk_Alloc ( (count + 1) * sizeof( *out ) );	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for( i = 0; i < count; i++, in++, out++ )
	{
		out->v[0] = ( unsigned short ) LittleShort( in->v[0] );
		out->v[1] = ( unsigned short ) LittleShort( in->v[1] );
	}
}



void Mod_LoadTexinfo( lump_t *l )
{
	texinfo_t	*in;
	mtexinfo_t	*out, *step;
	int 		i, count;
	char		name[MAX_QPATH];
	int			next;

	in = ( texinfo_t* ) ( void * ) ( mod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) )
	{
		ri.Sys_Error( ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name );
	}
	count = l->filelen / sizeof( *in );
	out = ( mtexinfo_t * ) Hunk_Alloc ( ( count + 6 ) * sizeof( *out ) );	// extra for skybox

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for( i = 0; i < count; i++, in++, out++ )
	{
		out->s.x = LittleFloat( in->vecs[0][0] );
		out->s.y = LittleFloat( in->vecs[0][1] );
		out->s.z = LittleFloat( in->vecs[0][2] );
		out->s_offset = LittleFloat( in->vecs[0][3] );

		out->t.x = LittleFloat( in->vecs[1][0] );
		out->t.y = LittleFloat( in->vecs[1][1] );
		out->t.z = LittleFloat( in->vecs[1][2] );
		out->t_offset = LittleFloat( in->vecs[1][3] );

		len1 = out->s.Length();
		len2 = out->t.Length();
		len1 = ( len1 + len2 ) / 2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;

		out->flags = LittleLong( in->flags );

		next = LittleLong( in->nexttexinfo );
		if( next > 0 )
		{
			out->next = loadmodel->texinfo + next;
		}

		Com_sprintf( name, sizeof( name ), "textures/%s.wal", in->texture );
		out->image = ( image_t * ) R_FindImage( name, it_wall, RESOURCE_LEVEL );
		if ( out->image == NULL )
		{
			out->image = r_notexture; // texture not found
			out->flags = 0;
		}
	}

	// count animation frames
	for( i = 0 ; i < count ; i++ )
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;
		for( step = out->next ; step && step != out ; step=step->next )
		{
			out->numframes++;
		}
	}
}



void CalcSurfaceExtents( msurface_t *s )
{
	float	mins[2], maxs[2], val;
	int		i, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;
	
	for( i = 0; i < s->numedges; i++ )
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if( e >= 0 )
		{
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		}
		else
		{
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		}
		
		val = v->position.x * tex->s.x + 
			v->position.y * tex->s.y + 
			v->position.z * tex->s.z + 
			tex->s_offset;

		if(val < mins[0] )
		{
			mins[0] = val;
		}
		if( val > maxs[0] )
		{
			maxs[0] = val;
		}

		val = v->position.x * tex->t.x + 
			v->position.y * tex->t.y + 
			v->position.z * tex->t.z + 
			tex->t_offset;

		if(val < mins[1])
		{
			mins[1] = val;
		}
		if( val > maxs[1] )
		{
			maxs[1] = val;
		}
	}

	for( i = 0; i < 2; i++ )
	{	
		bmins[i] = floor( mins[i]/16 );
		bmaxs[i] = ceil( maxs[i]/16 );

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = ( bmaxs[i] - bmins[i] ) * 16;
		if( s->extents[i] < 16 )
		{
			s->extents[i] = 16;	// take at least one cache block
		}
		if ( !(tex->flags & (SURF_WARP|SURF_SKY)) && s->extents[i] > 256)
			ri.Sys_Error (ERR_DROP,"Bad surface extents");
	}
}



// ----------------------------------------------------------------------------
//
// Name:		BeginRegistration 
// Description:	Specifies the model that will be used as the world
// Input:		a pointer to the world model
// Output:
// Note:
//
// ---------------------------------------------------------------------------
void BeginRegistration( const char *model )
{
	char	fullname[MAX_QPATH];
	cvar_t	*flushmap;

	g_startRegTime = timeGetTime ();

	registration_sequence++;
	r_oldviewcluster = -1;		//	force markleafs
	g_currentTexPalette = -3;	//	no palette set

	Com_sprintf( fullname, sizeof( fullname ), "maps/%s.bsp", model );

	// explicitly free the old map if different
	// this guarantees that mod_known[0] is the world map
	flushmap = ri.Cvar_Get( "flushmap", "0", CVAR_ARCHIVE );

	//	force all models to reload
	if( flushmap->value )
	{
		Mod_FreeAll ();
	}

	if ( strcmp( mod_known[0].name, fullname ) || flushmap->value )
	{
		Mod_Free( &mod_known[0] );
	}

	r_worldmodel = Mod_ForName( fullname, true, RESOURCE_LEVEL );

	r_viewcluster = -1;			// NOTE: why was this only in gl...  SCG 3-16-99

	surfSprite_Init ();
}



// ----------------------------------------------------------------------------
//
// Name:		R_RegisterModel
// Description:	Generic model registration
// Input:		model name and resource type
// Output:		
// Note:		
//
// ---------------------------------------------------------------------------
void *R_RegisterModel( const char *name, resource_t resource )
{
	int			i;
	model_t		*mod;
	dsprite_t	*sprout;
	dmdl_t		*pheader;

	mod = Mod_ForName( name, false, resource );

	if( mod != NULL )
	{
		mod->registration_sequence = registration_sequence;

		// register any images used by the models
		if (mod->type == mod_sprite)
		{
			sprout = (dsprite_t *)mod->extradata;
			for (i=0 ; i<sprout->numframes ; i++)
				mod->skins[i] = (image_t*)R_FindImage (sprout->frames[i].name, it_sprite, resource);
		}
		else if( mod->type == mod_alias )
		{
			pheader = (dmdl_t *)mod->extradata;
			for( i = 0; i < pheader->num_skins; i++ )
			{
				mod->skins[i] = ( image_t * ) RegisterSkin( ( char * ) pheader + pheader->ofs_skins + i * MAX_SKINNAME, resource );
			}
		}
		else if( mod->type == mod_brush )
		{
			for( i = 0; i < mod->numtexinfo; i++ )
			{
				mod->texinfo[i].image->registration_sequence = registration_sequence;
			}
		}
	}

	return ( void * ) mod;
}



// ----------------------------------------------------------------------------
//
// Name:		R_EndRegistration
// Description:	Checks for unused models and frees them if necessary.  In ref_soft
//				it also checks to make sure the model data is loaded if needed.
// Input:		
// Output:		
// Note:		
//
// ---------------------------------------------------------------------------
void R_EndRegistration( void )
{
	int		i;
	model_t	*mod;
	float	time;

	g_endRegTime = timeGetTime();

	time = (float)(g_endRegTime - g_startRegTime) / 1000.0;
	ri.Con_Printf( PRINT_ALL, "----\nRegistration time: %f seconds.----\n", time );

	for( i = 0, mod = mod_known; i < mod_numknown; i++, mod++ )
	{
		if( mod->name[0] == NULL )
		{
			continue;
		}

		if( mod->registration_sequence != registration_sequence )
		{
			Mod_Free (mod);
		}
		else if( strcmp( vid_ref->string, "ref_soft" == 0 ) )
		{
			Com_PageInMemory( ( unsigned char * ) mod->extradata, mod->extradatasize );
		}
	}

	R_FreeUnusedImages();
}



// ----------------------------------------------------------------------------
//
// Name:		Mod_Free
// Description:	
// Input:		
// Output:		
// Note:		
//
// ---------------------------------------------------------------------------
void Mod_Free( void *mod )
{
	model_t *model = ( model_t * ) mod;

	if( model == NULL )
	{
		return;
	}

	Hunk_Free( model->extradata );

	memset( model, 0, sizeof( model_t ) );
}



// ----------------------------------------------------------------------------
//
// Name:		Mod_FreeAll
// Description:	
// Input:		
// Output:		
// Note:		
//
// ---------------------------------------------------------------------------
void Mod_FreeAll (void)
{
	int		i;

	for( i = 0; i < mod_numknown; i++ )
	{
		if (mod_known[i].extradatasize)
		{
			Mod_Free( &mod_known[i] );
		}
	}
}
