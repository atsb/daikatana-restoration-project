// r_misc.c

#include	"gl_local.h"
#include	"gl_beams.h"


// mdm99.03.01 - damn overwrites
//extern image_t	*r_inventoryTextures [5];
//extern image_t	*r_inventoryNumbers;
extern image_t	*charFlare;
extern void		*char_flare_image;
extern image_t	*r_shadowtexture;
extern image_t	*pEnvMapSkin;

void	R_InitMiscTextures (void)
{
	//	init beam textures
	r_beamTextures[BEAM_TEX_LIGHTNING]	= R_FindImage ("pics/misc/w_zap001.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_LASER]		= R_FindImage ("pics/misc/laser.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_LASERSPARK]		= R_FindImage ("pics/misc/beamspark.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_TRACER]		= R_FindImage ("pics/misc/tracer.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_NOVALASER]	= R_FindImage ("pics/misc/novalaser.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_NOVALASER]	= R_FindImage ("pics/misc/novalaser.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_MF_GENERIC]	= R_FindImage ("skins/we_mflash2.tga", it_sprite, RESOURCE_GLOBAL);
	r_beamTextures[BEAM_TEX_GRAPPLE_CHAIN] = R_FindImage("skins/we_sclawslash.bmp", it_sprite, RESOURCE_GLOBAL);
    r_beamTextures[BEAM_TEX_DISCUSTRAIL] = R_FindImage ("pics/misc/we_disctr.tga", it_sprite, RESOURCE_GLOBAL);

	char_flare_image = charFlare = R_FindImage ("pics/misc/charflare.tga", it_sprite, RESOURCE_GLOBAL);

	r_shadowtexture = R_FindImage( "skins/e_shado.tga", it_sprite, RESOURCE_GLOBAL );
	pEnvMapSkin = (image_t *)RegisterSkin("skins/player_chrome1.tga", RESOURCE_GLOBAL);

	if( r_shadowtexture == NULL )
	{
		r_shadowtexture = r_notexture;
	}

}

/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/* 
================== 
GL_ScreenShot_f
================== 
*/  
void GL_ScreenShot_f (void) 
{
	byte		*buffer;
	char		picname[80]; 
	char		checkname[MAX_OSPATH];
	int			i, c, temp;
	FILE		*f;

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", ri.FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 
	strcpy(picname,"daikatana00.tga");

	for (i=0 ; i<=99 ; i++) 
	{ 
		picname[9] = i/10 + '0'; 
		picname[10] = i%10 + '0'; 
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", ri.FS_Gamedir(), picname);
		f = fopen (checkname, "rb");
		if (!f)
			break;	// file doesn't exist
		fclose (f);
	} 
	if (i==100) 
	{
		ri.Con_Printf (PRINT_ALL, "SCR_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}


	buffer = ( byte * ) ri.X_Malloc( vid.width * vid.height * 3 + 18, MEM_TAG_MISC );
	if( buffer == NULL )
	{
		return;
	}
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = vid.width&255;
	buffer[13] = vid.width>>8;
	buffer[14] = vid.height&255;
	buffer[15] = vid.height>>8;
	buffer[16] = 24;	// pixel size

	qglReadPixels (0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 

	// swap rgb to bgr
	c = 18+vid.width*vid.height*3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	f = fopen (checkname, "wb");
	fwrite (buffer, 1, c, f);
	fclose (f);

	ri.X_Free( buffer );
	ri.Con_Printf (PRINT_ALL, "Wrote %s\n", picname);
} 

/*
** GL_Strings_f
*/
void GL_Strings_f( void )
{
	ri.Con_Printf (PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string );
	ri.Con_Printf (PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string );
	ri.Con_Printf (PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string );
	ri.Con_Printf (PRINT_ALL, "GL_EXTENSIONS: %s\n", gl_config.extensions_string );
}

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void )
{
	qglClearColor (1,0, 0.5 , 0.5);
	qglCullFace(GL_FRONT);

//	unsigned int nStateFlags = ( GLSTATE_TEXTURE_2D | GLSTATE_ALPHA_TEST | GLSTATE_CULL_FACE_FRONT | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST );
	unsigned int nStateFlags = GLSTATE_PRESET1;

	if ( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	GL_SetState( nStateFlags );

	GL_SetFunc( GLSTATE_ALPHA_TEST, GL_GREATER, 0.666);

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	qglShadeModel (GL_FLAT);
	GL_TextureMode( gl_texturemode->string );
	GL_TextureAlphaMode( gl_texturealphamode->string );	// SCG[5/20/99]: Quake2 V3.19 addition
	GL_TextureSolidMode( gl_texturesolidmode->string );	// SCG[5/20/99]: Quake2 V3.19 addition

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_TexEnv( GL_REPLACE );

	if ( qglPointParameterfEXT )
	{
		float attenuations[3] =
		{
			gl_particle_att_a->value,
			gl_particle_att_b->value,
			gl_particle_att_c->value
		};

		qglEnable( GL_POINT_SMOOTH );
		qglPointParameterfEXT( GL_POINT_SIZE_MIN_EXT, gl_particle_min_size->value );
		qglPointParameterfEXT( GL_POINT_SIZE_MAX_EXT, gl_particle_max_size->value );
		qglPointParameterfvEXT( GL_DISTANCE_ATTENUATION_EXT, attenuations );
	}
	
	GL_SetTexturePalette( NULL, FALSE );	//	set to 8to24table

	GL_UpdateSwapInterval();
}

void GL_UpdateSwapInterval( void )
{
	if ( gl_swapinterval->modified )
	{
		gl_swapinterval->modified = false;

		if ( !gl_state.stereo_enabled ) 
		{
#ifdef _WIN32
			if ( qwglSwapIntervalEXT )
				qwglSwapIntervalEXT( gl_swapinterval->value );
#endif
		}
	}
}

static byte *pSavegamePic = NULL;

#define SCREENSHOT_WIDTH	( 256 )
#define SCREENSHOT_HEIGHT	( 256 )
#define HEADER_SIZE			( 18 )
#define PIXELWIDTH			( 3 )
#define IMAGE_START			( HEADER_SIZE )
#define IMAGE_SIZE			( ( SCREENSHOT_WIDTH * SCREENSHOT_HEIGHT ) * PIXELWIDTH + HEADER_SIZE )

void ScaleDownScreenShot(byte *dst, byte *src, int dstWidth, int dstHeight, int srcWidth, int srcHeight)
{
	if (!dst || !src)
		return;

	if ((dstWidth > srcWidth) || (dstHeight > srcHeight))
		return;

	if ((dstWidth == srcWidth) && (dstHeight == srcHeight))
	{
		memcpy(dst,src,(dstWidth * dstHeight * PIXELWIDTH));
		return;
	}

	float srcpixeli,srcpixelj;
	srcpixeli = srcpixelj = 0;

	float stepj, stepi;
	stepi = ((float)srcHeight / (float)dstHeight);
	stepj = ((float)srcWidth / (float)dstWidth);

	int i,j;						// destination

	for (i = 0; i < dstHeight; i++)
	{
		srcpixelj = 0;
		for (j = 0; j < dstWidth; j++)
		{
			memcpy(dst + i * dstWidth * 3 + j * 3,src + (int)srcpixeli * srcWidth * 3 + ((int)srcpixelj) * 3,3);
			srcpixelj += stepj;
		}
		srcpixeli += stepi;
	}
}

void GetSavegameScreenShot()
{
//	int		i, j, temp;
	int		i, temp;

	// SCG[12/8/99]: Allocate memory for the shot
	pSavegamePic = ( byte * ) ri.X_Malloc( IMAGE_SIZE, MEM_TAG_MISC );

	// SCG[12/8/99]: Clean the buffer
	memset( pSavegamePic, 0, 18 );

	// SCG[12/8/99]: write the .tga headerinfo
	pSavegamePic[2]		= 2;		// uncompressed type
	pSavegamePic[12]	= SCREENSHOT_WIDTH & 255;
	pSavegamePic[13]	= SCREENSHOT_WIDTH >> 8;
	pSavegamePic[14]	= SCREENSHOT_HEIGHT & 255;
	pSavegamePic[15]	= SCREENSHOT_HEIGHT >> 8;
	pSavegamePic[16]	= 24;	// pixel size

	byte *buffer = ( byte * ) ri.X_Malloc( vid.width * vid.height * 3, MEM_TAG_MISC );
	memset (buffer, 0, 18);

	qglReadPixels( 0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
	
	ScaleDownScreenShot(pSavegamePic + IMAGE_START,buffer,SCREENSHOT_WIDTH,SCREENSHOT_HEIGHT,vid.width,vid.height);

	// swap rgb to bgr
	for( i = IMAGE_START; i < IMAGE_SIZE ;i += PIXELWIDTH )
	{
		temp = pSavegamePic[i];
		pSavegamePic[i] = pSavegamePic[i + 2];
		pSavegamePic[i + 2] = temp;
	}

	ri.X_Free( buffer );
}

void ClearSavegameScreenShot()
{
	// SCG[12/8/99]: Make sure the pointer is valid
	if( pSavegamePic == NULL )
	{
		return;
	}

	// SCG[12/8/99]: Free the memory and set the pointer to NULL
	ri.X_Free( pSavegamePic );
	pSavegamePic = NULL;
}

void SaveSavegameScreenShot( char *path, char *savename )
{
	FILE		*f;
	char		name[32];
	char		filename[128];

	_splitpath( savename, NULL, NULL, name, NULL );

	// SCG[12/8/99]: Make sure the pointer is valid
	if( pSavegamePic == NULL )
	{
		return;
	}

	// cek[12-10-99] make sure path ends in a /
	char c = *(path + strlen(path) - 1);
	if (c != '/')
// SCG[1/16/00]: 		sprintf( filename, "%s/%s.tga", path, name );
		Com_sprintf( filename,sizeof(filename), "%s/%s.tga", path, name );
	else
		Com_sprintf( filename,sizeof(filename), "%s%s.tga", path, name );

	f = fopen( filename, "wb" );

	if( f == NULL )
	{
		ri.Con_Printf( PRINT_ALL, "Failed to open %s\n", filename );
	}
	fwrite( pSavegamePic, 1, IMAGE_SIZE, f );
	fclose( f );
}
