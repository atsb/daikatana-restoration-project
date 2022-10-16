// r_main.c
#include	"gl_local.h"
//#include	"gl_protex.h"
#include	"gl_beams.h"

#include "dk_ref_common.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define MAX_DEPTHHACK			20
#define	MAX_PARTICLE_ROTATIONS	16
//#define	ROTATING_SNOW	//	if you uncomment this, do it in cl_fx.c, too!

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void	R_Clear (void);
//void	R_DrawDepthHackEntitiesOnList (void);
void	GL_Strings_f( void );
//void	R_DrawBeamList (void);
qboolean R_CullParticleVolume (CVector &mins, CVector &maxs);
void	R_GetModelHardpoint(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt);


///////////////////////////////////////////////////////////////////////////////
//	globals out the ass...
///////////////////////////////////////////////////////////////////////////////

viddef_t	vid;
//refimport_t	ri;		// moved to dk_ref_init - SCG: 3-10-99

model_t		*r_worldmodel;

glconfig_t	gl_config;
glstate_t	gl_state;

entity_t	*currententity;
model_t		*currentmodel;

mplane_t	frustum[4];

// view origin
CVector	vup, vpn, vright;
CVector	r_origin;

float	r_proj_matrix [16];
int		r_viewport [4];
float	r_world_matrix[16];
float	r_base_world_matrix[16];

// screen size info
refdef_t	r_newrefdef;
int			r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

float		v_blend[4];			// final blending color

//	Nelno:	for doing time-based stuff in the render dll, exported in gl_local.h
int		ref_curTime;
int		ref_lastTime;
float	ref_frameTime;
float	ref_globalTime;	//	total time passed since R_Init, in seconds
float	ref_laserRotation = 0.0F;
int		ref_laserDir = 1;

// mike: depthhack
entity_t		depthHackList [MAX_DEPTHHACK];
sortentity_t	depthHackAlphaList[MAX_DEPTHHACK];
short			depthHackCount;
short			depthHackAlphaCount;

// underwater effect
bool            fInWater;
float           base_fov_y = -1.0;
float           base_roll = 0.0f;
float           base_water_time;

float		gldepthmin, gldepthmax;

int			gl_bit_depth;	// here's the bit depth for the current gl context

// Ash -- vert normals array, byte indexed
CVector r_vertnormals[256];

image_t		*r_notexture;		// use for bad textures
image_t		*r_particletexture;
image_t		*r_gunflaretexture;
image_t		*charFlare;
void *char_flare_image;

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

//	counters for debug/development
int			c_brush_polys, c_alias_polys, c_lightmap_polys, c_damage_polys;
int			c_wal_memory, c_skin_memory, c_pic_memory;
int			c_sky_memory, c_sprite_memory;
int			g_PVCullCount;
int			g_PVDrawCount;

cvar_t	*r_countpvolumes;
cvar_t	*r_countsnow;
cvar_t	*r_countents;
cvar_t	*r_drawbmodels;
cvar_t	*r_drawalias;
cvar_t	*r_drawsprites;

cvar_t  *weapon_visible;

cvar_t	*r_drawcomparticles;
int		ent_draw_count;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_lerpmodels;

cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

//cvar_t  *console_screen_idx; // 12.21  dsn

cvar_t	*gl_nosubimage;
cvar_t	*gl_allow_software;

cvar_t	*gl_vertex_arrays;

cvar_t	*gl_particle_min_size;
cvar_t	*gl_particle_max_size;
cvar_t	*gl_particle_size;
cvar_t	*gl_particle_att_a;
cvar_t	*gl_particle_att_b;
cvar_t	*gl_particle_att_c;

cvar_t	*gl_ext_swapinterval;
cvar_t	*gl_ext_gamma_control;
cvar_t	*gl_ext_multitexture;
cvar_t	*gl_ext_pointparameters;

cvar_t	*gl_log;
cvar_t	*gl_bitdepth;
cvar_t	*gl_drawbuffer;
cvar_t  *gl_driver;
cvar_t	*gl_lightmap;
cvar_t	*gl_shadows;
cvar_t	*gl_mode;
cvar_t	*gl_dynamic;
cvar_t  *gl_monolightmap;
cvar_t	*gl_modulate;
cvar_t	*gl_nobind;
cvar_t	*gl_round_down;
cvar_t	*gl_picmip;
cvar_t	*gl_skymip;
cvar_t	*gl_showtris;
cvar_t	*gl_ztrick;
cvar_t	*gl_finish;
cvar_t	*gl_clear;
cvar_t	*gl_cull;
cvar_t	*gl_polyblend;
cvar_t	*gl_flashblend;
cvar_t	*gl_playermip;
cvar_t  *gl_saturatelighting;
cvar_t	*gl_swapinterval;
cvar_t	*gl_texturemode;
cvar_t	*gl_lockpvs;
cvar_t	*gl_drawflat;
cvar_t  *gl_polylines;
cvar_t  *gl_vfog;
cvar_t  *gl_vfogents;
cvar_t  *gl_vertnormals;
cvar_t  *gl_envmap;
cvar_t  *gl_texenvmap;
cvar_t  *gl_texenvcolor;
cvar_t  *gl_predator;
cvar_t  *gl_midtextures;
cvar_t  *gl_surfacesprites;
cvar_t  *gl_alphasurfaces;
cvar_t  *gl_fogsurfaces;
cvar_t  *gl_procshell;
cvar_t  *gl_procp;
cvar_t  *gl_procm;
cvar_t  *gl_procd;
cvar_t  *gl_procn;
cvar_t  *gl_bbox;
cvar_t	*gl_texturealphamode;	// SCG[5/20/99]: Quake2 V3.19 addition
cvar_t	*gl_texturesolidmode;	// SCG[5/20/99]: Quake2 V3.19 addition
cvar_t	*gl_brute_force_state;
cvar_t  *gl_24bittextures;
cvar_t  *gl_32bitatextures;

cvar_t	*gl_3dlabs_broken;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_ref;

cvar_t	*r_palettedir;			//	Shawn:  added for palette per level
cvar_t	*r_drawparticleflag;	//	Shawn:  added for testing particle speed
cvar_t	*r_useimagelog;			//  Shawn:  added for image log 
cvar_t	*r_resourcedir;			//	Nelno:	added for per-level console backgrounds/characters
cvar_t	*gl_mem;				//  Shawn:  added for tracking memory usage

cvar_t	*gl_alphatest;

cvar_t	*r_fogcolor;
cvar_t	*r_testfog;
cvar_t	*r_fognear;
cvar_t	*r_fogfar;
cvar_t	*r_skyfogfar;

cvar_t	*gl_clip_surface_sprites;

cvar_t  *gl_cloudname;
cvar_t  *gl_skyname;
cvar_t  *gl_lightningfreq;
cvar_t  *gl_cloudxdir;
cvar_t  *gl_cloudydir;
cvar_t  *gl_cloud1tile;
cvar_t  *gl_cloud1speed;
cvar_t  *gl_cloud2tile;
cvar_t  *gl_cloud2speed;
cvar_t  *gl_cloud2alpha;

cvar_t  *gl_cloudname_2;
cvar_t  *gl_skyname_2;
cvar_t  *gl_lightningfreq_2;
cvar_t  *gl_cloudxdir_2;
cvar_t  *gl_cloudydir_2;
cvar_t  *gl_cloud1tile_2;
cvar_t  *gl_cloud1speed_2;
cvar_t  *gl_cloud2tile_2;
cvar_t  *gl_cloud2speed_2;
cvar_t  *gl_cloud2alpha_2;

cvar_t  *gl_cloudname_3;
cvar_t  *gl_skyname_3;
cvar_t  *gl_lightningfreq_3;
cvar_t  *gl_cloudxdir_3;
cvar_t  *gl_cloudydir_3;
cvar_t  *gl_cloud1tile_3;
cvar_t  *gl_cloud1speed_3;
cvar_t  *gl_cloud2tile_3;
cvar_t  *gl_cloud2speed_3;
cvar_t  *gl_cloud2alpha_3;

cvar_t  *gl_cloudname_4;
cvar_t  *gl_skyname_4;
cvar_t  *gl_lightningfreq_4;
cvar_t  *gl_cloudxdir_4;
cvar_t  *gl_cloudydir_4;
cvar_t  *gl_cloud1tile_4;
cvar_t  *gl_cloud1speed_4;
cvar_t  *gl_cloud2tile_4;
cvar_t  *gl_cloud2speed_4;
cvar_t  *gl_cloud2alpha_4;

cvar_t  *gl_cloudname_5;
cvar_t  *gl_skyname_5;
cvar_t  *gl_lightningfreq_5;
cvar_t  *gl_cloudxdir_5;
cvar_t  *gl_cloudydir_5;
cvar_t  *gl_cloud1tile_5;
cvar_t  *gl_cloud1speed_5;
cvar_t  *gl_cloud2tile_5;
cvar_t  *gl_cloud2speed_5;
cvar_t  *gl_cloud2alpha_5;

cvar_t	*gl_subdivide_size;
cvar_t	*gl_no_error_check;
cvar_t	*gl_no_alias_models;

cvar_t	*gl_drawfog;
/*
=================
R_GetModelSkinIndex

  - return the skin index for a particular alias model


FIXME_AMW: this doesn't account for custom skins....  
	
=================
*/
char *R_GetModelSkinName (void *modelPtr)
{
	image_t	*skin = NULL;
	
	if (!modelPtr)
		return (0);

	// cast the pointer over to a model structure
	model_t	*currentmodel = (model_t*) modelPtr;

	// get the first skin
	skin = currentmodel->skins[0];
	if (skin)
	{
		return (skin->name);
	}
	else
	{
		// no skins
		return NULL;
	}
}

int R_GetModelSkinIndex (void *modelPtr)
{
	image_t	*skin = NULL;
	
	if (!modelPtr)
		return (0);

	// cast the pointer over to a model structure
	model_t	*currentmodel = (model_t*) modelPtr;

	// get the first skin
	skin = currentmodel->skins[0];

	if (skin)
	{
		// pointer arithmetic gives us the index of the skin
		int skinIndex = skin - gltextures;
		return (skinIndex);
	}
	else
	{
		// no skins
		return (0);
	}
}

/*
=================
R_BestColor
=================
*/
unsigned char R_BestColor (byte r, byte g, byte b, unsigned *palette)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	bestcolor;
	unsigned	*pal;

//
// let any color go to 0 as a last resort
//
	bestdistortion = 256*256*4;
	bestcolor = 0;

	if (palette == NULL)
		pal = d_8to24table;
	else
		pal = palette;

	for (i=0; i<= 255 ; i++)
	{
		dr = r - ((pal[i]>>0) & 0xff);
		dg = g - ((pal[i]>>8) & 0xff);
		db = b - ((pal[i]>>16) & 0xff);

		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			bestcolor = i;
		}
	}

	return bestcolor;
}

/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (CVector &mins, CVector &maxs)
{
	int		i;

	if (r_nocull->value)
		return false;

	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide (mins, maxs, (cplane_t *) &frustum[i]) == 2)
			return true;
	return false;
}

void R_RotateForEntity (entity_t *e)
{
    qglTranslatef (e->origin.x,  e->origin.y,  e->origin.z);

    qglRotatef (e->angles.y,  0, 0, 1);
    qglRotatef (-e->angles.x,  0, 1, 0);
    qglRotatef (-e->angles.z,  1, 0, 0);
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	float		alpha = 1.0F;
	CVector		point;
	CVector		vColor;
	dsprframe_t	*frame;
	CVector		up, right;
	dsprite_t	*psprite;
	int			origin_x, origin_y;
	int			height, width;
	image_t		*image;
	CVector		v_forward, v_right, v_up;
	unsigned int	nStateFlags;
	int framenum;
	
	currentmodel = (model_t*)e->model;

	psprite = (dsprite_t *)currentmodel->extradata;

	framenum = e->frame % psprite->numframes;

	if( framenum < 0 )
	{
		ri.Con_Printf( PRINT_ALL, "Invalid frame (%d) in model %s.\n", framenum, currentmodel->name );
		framenum = 0;
	}

	if( (image = currentmodel->skins[framenum]) == NULL)
		image = r_notexture;

	frame = &psprite->frames[framenum];

	if (e->flags & SPR_ORIENTED)
	{
		AngleToVectors (e->angles, v_forward, v_right, v_up);
		up	  = v_up;
		right = v_right;
	}
	else
	{
		up	  = vup;
		right = vright;
	}

	if (e->render_scale.x == 0 && e->render_scale.y == 0 && e->render_scale.z == 0)
		ri.Sys_Error (ERR_FATAL, "Entity with sprite %s has 0, 0, 0 render_scale.\n", currentmodel->name);

	if( ( e->color.x != 0 ) || ( e->color.y != 0 ) || ( e->color.z != 0 ) )
	{
		vColor = e->color;
	}
	else
	{
		vColor.Set( 1.0, 1.0, 1.0 );
	}

	if( e->flags & SPR_ALPHACHANNEL )
	{
		qglBlendFunc (GL_SRC_ALPHA, GL_ONE);
	}

	up.x *= e->render_scale.y;
	up.y *= e->render_scale.y;
	up.z *= e->render_scale.y;

	right.x *= e->render_scale.x;
	right.y *= e->render_scale.x;
	right.z *= e->render_scale.x;

	height = frame->height;
	width = frame->width;
	origin_y = frame->origin_y;
	origin_x = frame->origin_x;

 	nStateFlags = ( GLSTATE_PRESET1 | GLSTATE_BLEND | GLSTATE_ALPHA_TEST );

	if (e->flags & SPR_ALPHACHANNEL)
	{
		alpha = e->alpha;
	}

	if( image->has_alpha )
	{
		nStateFlags &= ~(GLSTATE_CULL_FACE | GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE_FRONT );
//		nStateFlags &= ~(GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT );
	}

	// double sided sprites

	if( e->flags & SPR_ORIENTED )
	{
		nStateFlags &= ~( GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT );
	}

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	// SCG[5/19/99]: Set the state
	GL_SetState( nStateFlags );

	GL_Bind(image->texnum);

	GL_TexEnv( GL_MODULATE );

	qglColor4f( vColor.x, vColor.y, vColor.z, alpha );

	qglBegin (GL_QUADS);

	qglTexCoord2f (0, 1);

	VectorMA (e->origin, up, -origin_y, point);
	VectorMA (point, right, -origin_x, point);
	qglVertex3fv (point);

	qglTexCoord2f (0, 0);
	VectorMA (e->origin, up, height - origin_y, point);
	VectorMA (point, right, -origin_x, point);
	qglVertex3fv (point);

	qglTexCoord2f (1, 0);
	VectorMA (e->origin, up, height - origin_y, point);
	VectorMA (point, right, width - origin_x, point);
	qglVertex3fv (point);

	qglTexCoord2f (1, 1);
	VectorMA (e->origin, up, -origin_y, point);
	VectorMA (point, right, width - origin_x, point);
	qglVertex3fv (point);

	qglEnd ();

	GL_TexEnv (GL_REPLACE);

	if( e->flags & SPR_ALPHACHANNEL )
	{
		qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

//==================================================================================

///////////////////////////////////////////////////////////////////////////////
//	R_GetProjectedVert
//
//	Nelno:	this isn't working yet
//	qglGetIntegerv seems to be returning monkey fuck as does qglGetDoublev
///////////////////////////////////////////////////////////////////////////////

#ifdef	gluProject_DOESNT_WORK_YET
int	R_GetProjectedVert (const CVector &org, int *x, int *y)
{
	GLdouble	mv_matrix [16];
	GLdouble	proj_matrix [16];
	GLdouble	winx, winy, winz;

	float		unfucked_mv_matrix [16];
	float		unfucked_proj_matrix [16];
	int			i, result;

	//	getting these with qglGetDoublev returns totally fucked values

//	qglGetDoublev (GL_MODELVIEW_MATRIX, mv_matrix);
	qglGetFloatv (GL_MODELVIEW_MATRIX, unfucked_mv_matrix);

//	qglGetDoublev (GL_PROJECTION_MATRIX, proj_matrix);
	qglGetFloatv (GL_PROJECTION_MATRIX, unfucked_proj_matrix);

	winx = winy = winz = 0;

	for (i = 0; i < 16; i++)
	{
		mv_matrix [i] = unfucked_mv_matrix [i];
		proj_matrix [i] = r_proj_matrix [i];
	}

	result = gluProject (org.x, org.y, org.z, mv_matrix, proj_matrix, r_viewport, &winx, &winy, &winz);

	if (result == GL_TRUE)
	{
		*x = (int)winx;
		*y = (int)winy;
		return	true;
	}
	else
	{
		*x = -1;
		*y = -1;
		return	false;
	}
}
#endif

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel (void)
{
	CVector	shadelight;
	int		i;

	if ( currententity->flags & RF_FULLBRIGHT )
		shadelight.x = shadelight.y = shadelight.z = 1.0F;
	else
		R_LightPoint (currententity->origin, shadelight );

    qglPushMatrix ();

	R_RotateForEntity (currententity);

	GL_SetState( GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST );

	qglColor3fv (shadelight);

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, -16);
	for (i=0 ; i<=4 ; i++)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, 16);
	for (i=4 ; i>=0 ; i--)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();

	qglPopMatrix ();
}



void R_SetupParticleVolumes( void )
{
	int	i = 0;
	particleVolume_t	*pVolume = NULL;

	// do we want to render particles?
	if (r_drawparticleflag->value)
	{
		// loop through the particle volumes and generate complex
		// particles for the ones that pass the culling tests
		for (i=0 ; i < r_newrefdef.num_particleVolumes ; i++)
		{
			// get a pointer to the particle volume record
			pVolume = &r_newrefdef.particleVolumes[i];

			// if this was marked as visible...
			if (pVolume->bVisible)
			{
				// cull it to the view frustrum
				if (!R_CullParticleVolume (pVolume->mins, pVolume->maxs))
				{
					// generate volume particles (pVolume acts as the unique 'key')
					ri.CL_GenerateVolumeParticles ((int) pVolume, pVolume->type, pVolume->mins, pVolume->maxs, pVolume->dist, pVolume->effects);
					g_PVDrawCount++;
				}
				else
				{
					//	remove any particle volumes from the active list that are not visible anymore
					//  NOTE: this should rarely get called since most instances are caught before
					//        the renderer is called in CL_AddParticleVolumes()
					ri.CL_RemoveParticleVolume ((int) pVolume);
					g_PVCullCount++;
				}
			}
		}

	}
}


/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawSpotlight( entity_t *e );
void R_DrawEntitiesOnList (void)
{
	int		i;
//	int		ent_count = 0;

	if (!r_drawentities->value)
		return;

	ent_draw_count = 0;
	depthHackCount=0;
	depthHackAlphaCount=0;

	// SCG[5/19/99]: Set the depth func
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	// draw non-transparent first
	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];

		// rf_depthhack entities are stored in a list and drawn later
		if (currententity->flags & RF_DEPTHHACK)
		{
			currententity->flags &= ~RF_DEPTHHACK;
			if (depthHackCount < MAX_DEPTHHACK)
			{
				memcpy(&depthHackList[depthHackCount],currententity,sizeof(entity_t));
				depthHackCount++;
			}
			continue;
		}
//		ent_count++;

		if ( currententity->flags & RF_SPOTLIGHT )
		{
			R_DrawSpotlight( currententity );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
		}
		else
		{
			currentmodel = (model_t *)currententity->model;
			if (!currentmodel)
			{
				R_DrawNullModel ();
	#ifdef	_DEBUG
		RImp_CheckError();
	#endif
				continue;
			}
			if( currentmodel->registration_sequence != registration_sequence )
			{
				continue;
			}

			switch (currentmodel->type)
			{
				case mod_alias:
					if (r_drawalias && r_drawalias->value)
					{
						R_DrawAliasModel (currententity);
	#ifdef	_DEBUG
		RImp_CheckError();
	#endif
					}
					break;
				case mod_brush:
					if (r_drawbmodels && r_drawbmodels->value)
					{
						R_DrawBrushModel (currententity);
	#ifdef	_DEBUG
		RImp_CheckError();
	#endif
					}
					break;
				case mod_sprite:
					if (r_drawsprites && r_drawsprites->value)
					{
						R_DrawSpriteModel (currententity);
	#ifdef	_DEBUG
		RImp_CheckError();
	#endif
					}
					break;
				default:
					ri.Sys_Error (ERR_DROP, "Bad modeltype");
					break;
			}
		}
	}
}
/*
=============
R_DrawAlphaEntitiesOnList
=============
*/
void R_DrawAlphaEntitiesOnList (void)
{
	int		i;
//	int		ent_count = 0;

	if (!r_drawentities->value)
		return;

	for (i=0 ; i<r_newrefdef.num_alphaentities ; i++)
	{
		currententity = &r_newrefdef.alphaentities[i].ent;

		// rf_depthhack entities are stored in a list and drawn later
		if (currententity->flags & RF_DEPTHHACK)
		{
			currententity->flags &= ~RF_DEPTHHACK;
			if (depthHackCount < MAX_DEPTHHACK)
			{
				memcpy(&depthHackAlphaList[depthHackCount],&r_newrefdef.alphaentities[i],sizeof(sortentity_t));
				depthHackAlphaCount++;
			}
			continue;
		}
		if ( currententity->flags & RF_BEAM_MOVING)
		{
			beam_DrawBeam ( currententity , r_beamTextures [BEAM_TEX_LASER]);
#ifdef	_DEBUG
	RImp_CheckError();
#endif
			continue;
		}

//		ent_count++;

		if ( currententity->flags & RF_SPOTLIGHT )
		{
			R_DrawSpotlight( currententity );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
		}
		else
		{
			currentmodel = (model_t*)currententity->model;

			if (!currentmodel)
			{
				R_DrawNullModel ();
#ifdef	_DEBUG
	RImp_CheckError();
#endif
				continue;
			}
			if( currentmodel->registration_sequence != registration_sequence )
			{
				continue;
			}

			switch (currentmodel->type)
			{
			case mod_alias:
				if (r_drawalias && r_drawalias->value)
				{
					R_DrawAliasModel (currententity);
#ifdef	_DEBUG
	RImp_CheckError();
#endif
				}
				break;
			case mod_brush:
				if (r_drawbmodels && r_drawbmodels->value)
				{
					R_DrawBrushModel (currententity);

#ifdef	_DEBUG
	RImp_CheckError();
#endif
				}
				break;
			case mod_sprite:
				if (r_drawsprites && r_drawsprites->value)
				{
					R_DrawSpriteModel (currententity);
#ifdef	_DEBUG
	RImp_CheckError();
#endif
				}
				break;
			default:
				ri.Sys_Error (ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}

	if (r_countents && r_countents->value)
		ri.Con_Printf (PRINT_ALL, "%i entities drawn.\n", ent_draw_count);
}

void R_DrawBModelMidTextureFogShells (void)
{
	int		i;
//	int		ent_count = 0;

	if ((!r_drawentities->value)||(gl_drawflat->value)||(!gl_vfogents->value))
		return;

	ent_draw_count = 0;

	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];

//		ent_count++;

		currentmodel = (model_t *)currententity->model;
		
        if (!currentmodel)
			continue;
        
		if( currentmodel->registration_sequence != registration_sequence )
		{
			continue;
		}

        switch (currentmodel->type)
		{
			case mod_alias:
				break;
			case mod_brush:
				if (r_drawbmodels && r_drawbmodels->value)
					R_DrawBrushModelMidTextureFogShell (currententity);
				break;
			case mod_sprite:
				break;
			default:
				ri.Sys_Error (ERR_DROP, "Bad modeltype");
				break;
		}
	}
}

/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (!gl_polyblend->value)
		return;
	if (!v_blend[3])
		return;

	GL_SetState( GLSTATE_BLEND );

	GL_TexEnv( GL_MODULATE );

    qglLoadIdentity ();

	// FIXME: get rid of these
    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up

	qglColor4fv (v_blend);

	qglBegin (GL_QUADS);

	qglVertex3f (10, 100, 100);
	qglVertex3f (10, -100, 100);
	qglVertex3f (10, -100, -100);
	qglVertex3f (10, 100, -100);
	qglEnd ();
}

//=======================================================================

int SignbitsForPlane (mplane_t *out)
{
	int	bits;

	// for fast box on planeside test

	bits = 0;
	if (out->normal.x < 0)
		bits |= 1<<0;
	if (out->normal.y < 0)
		bits |= 1<<1;
	if (out->normal.z < 0)
		bits |= 1<<2;

	return bits;
}


void R_SetFrustum (void)
{
	int		i;

#if 0
	/*
	** this code is wrong, since it presume a 90 degree FOV both in the
	** horizontal and vertical plane
	*/
	// front side is visible
	frustum[0].normal = vpn + vright;
	frustum[1].normal = vpn - vright;
	frustum[2].normal = vpn + vup;
	frustum[3].normal = vpn - vup;

	// we theoretically don't need to normalize these vectors, but I do it
	// anyway so that debugging is a little easier
	frustum[0].normal.Normalize();
	frustum[1].normal.Normalize();
	frustum[2].normal.Normalize();
	frustum[3].normal.Normalize();
#else
	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector( vup, vpn, -(90-r_newrefdef.fov_x / 2 ), frustum[0].normal );
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector( vup, vpn, 90-r_newrefdef.fov_x / 2, frustum[1].normal );
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector( vright, vpn, 90-r_newrefdef.fov_y / 2, frustum[2].normal );
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector( vright, vpn, -( 90 - r_newrefdef.fov_y / 2 ), frustum[3].normal );
#endif

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int i;
	mleaf_t	*leaf=NULL, 
            *upleaf=NULL,
            *downleaf=NULL;

	r_framecount++;

// build the transformation matrix for the given view angles
	r_origin = r_newrefdef.vieworg;

// current viewcluster
	if ( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf (r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if (!leaf->contents)
		{	// look down a bit
			CVector	temp;

			temp = r_origin;
			temp.z -= 16;
			downleaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(downleaf->contents & CONTENTS_SOLID) &&
				(downleaf->cluster != r_viewcluster2) )
				r_viewcluster2 = downleaf->cluster;
		}
		else
		{	// look up a bit
			CVector	temp;

			temp = r_origin;
			temp.z += 16;
			upleaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(upleaf->contents & CONTENTS_SOLID) &&
				(upleaf->cluster != r_viewcluster2) )
				r_viewcluster2 = upleaf->cluster;
		}
	}

    // do a water fov effect here when we're in the world
/*
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{

		if (leaf&&(leaf->contents&CONTENTS_WATER))
		{
			// if we weren't in water last time, init some stuff
			if (false==fInWater)
			{
				base_fov_y = r_newrefdef.fov_y;
				base_roll = r_newrefdef.viewangles.roll;
				base_water_time = r_newrefdef.time;
			}

			fInWater = true;

			// update the fov
			r_newrefdef.fov_y = base_fov_y + 0.75f*sin(r_newrefdef.time - base_water_time);

			// roll some huh?
			r_newrefdef.viewangles.roll = base_roll + 0.5f*sin(2.1f*(r_newrefdef.time - base_water_time));
		}
		else
		{
			fInWater = false;
			if (base_fov_y>0)
			{   
				r_newrefdef.fov_y = base_fov_y;
				r_newrefdef.viewangles.roll = base_roll;
			}
		}
	}
*/
	AngleToVectors (r_newrefdef.viewangles, vpn, vright, vup);

	for (i=0 ; i<4 ; i++)
		v_blend[i] = r_newrefdef.blend[i];

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		GL_SetState( GLSTATE_PRESET1 | GLSTATE_SCISSOR_TEST );
		qglClearColor( 0.3, 0.3, 0.3, 1 );
		qglScissor( r_newrefdef.x, vid.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );

		if (r_newrefdef.rdflags & RDF_CLEARCOLORBUFF)
			qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		else
			qglClear( GL_DEPTH_BUFFER_BIT );

		qglClearColor( 1, 0, 0.5, 0.5 );
	}
}


void MYgluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan( fovy * M_PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	xmin += -( 2 * gl_state.camera_separation ) / zNear;
	xmax += -( 2 * gl_state.camera_separation ) / zNear;

	qglFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}


/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
//	float	yfov;
	int		x, x2, y2, y, w, h;
	GLfloat fogcolor[4];
	float	r, g, b;

	//
	// set up viewport
	//
	x = floor(r_newrefdef.x * vid.width / vid.width);
	x2 = ceil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = floor(vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = ceil(vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;
	h = y - y2;

	//	Nelno:	save for getting projected vertices
	r_viewport [0] = x;
	r_viewport [1] = y2;
	r_viewport [2] = w;
	r_viewport [3] = h;

	qglViewport (x, y2, w, h);

	// setup fog

	if (r_newrefdef.foginfo.active)
	{
		fogcolor[0] = r_newrefdef.foginfo.r;
		fogcolor[1] = r_newrefdef.foginfo.g;
		fogcolor[2] = r_newrefdef.foginfo.b;
		fogcolor[3] = 1.0;

		qglFogi (GL_FOG_MODE, GL_LINEAR);
		qglFogfv (GL_FOG_COLOR, fogcolor);
		qglHint (GL_FOG_HINT, GL_NICEST);
		qglFogf (GL_FOG_START, r_newrefdef.foginfo.start);
		qglFogf (GL_FOG_END, r_newrefdef.foginfo.end);
	}

	if( r_testfog->value ){
		sscanf( r_fogcolor->string, "%f,%f,%f", &r, &g, &b );
		fogcolor[0] = r * ONEDIV255;
		fogcolor[1] = g * ONEDIV255;
		fogcolor[2] = b * ONEDIV255;
		fogcolor[3] = 1.0;

		qglFogi( GL_FOG_MODE, GL_LINEAR );
		qglFogfv( GL_FOG_COLOR, fogcolor);
		qglHint( GL_FOG_HINT, GL_NICEST);
		qglFogf( GL_FOG_START, r_fognear->value );
		qglFogf( GL_FOG_END, r_fogfar->value );
	}


	//
	// set up projection matrix
	//
    screenaspect = (float)r_newrefdef.width/r_newrefdef.height;
//	yfov = 2*atan((float)r_newrefdef.height/r_newrefdef.width)*180/M_PI;
	qglMatrixMode(GL_PROJECTION);
    qglLoadIdentity ();
    MYgluPerspective (r_newrefdef.fov_y,  screenaspect,  4, 8192);

	qglGetFloatv (GL_PROJECTION_MATRIX, r_proj_matrix);

	qglMatrixMode(GL_MODELVIEW);
    qglLoadIdentity ();

    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up
    qglRotatef (-r_newrefdef.viewangles[2],  1, 0, 0);
    qglRotatef (-r_newrefdef.viewangles[0],  0, 1, 0);
    qglRotatef (-r_newrefdef.viewangles[1],  0, 0, 1);
    qglTranslatef (-r_newrefdef.vieworg[0],  -r_newrefdef.vieworg[1],  -r_newrefdef.vieworg[2]);

//	if ( gl_state.camera_separation != 0 && gl_state.stereo_enabled )
//		qglTranslatef ( gl_state.camera_separation, 0, 0 );

	qglGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

	GL_SetState( GLSTATE_PRESET1 );
    // 
    // set polygon mode
    //
    if (1==gl_polylines->value)
    {
        qglClearColor( 0, 0, 0, 1 );
        qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        qglPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    }
    else
        qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	GL_SetState( GLSTATE_PRESET1 );

	if (gl_ztrick->value)
	{
		static int trickframe;

		if (gl_clear->value)
			qglClear (GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
//			qglDepthFunc (GL_LEQUAL);
			GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
//			qglDepthFunc (GL_GEQUAL);
			GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_GEQUAL, -1 );
		}
	}
	else
	{
		if (gl_clear->value)
			qglClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			qglClear (GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 1;
//		qglDepthFunc( GL_LEQUAL );
		GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
	}

	qglDepthRange (gldepthmin, gldepthmax);

}

//---------------------------------------------------------------------------
// R_DrawComplexDepthHackParticles()
//---------------------------------------------------------------------------
void GL_DrawComplexParticles( int num_particles, const particle_t particles [] );

void R_DrawComplexDepthHackParticles(void)
{
	if (!r_drawparticleflag->value || !r_drawcomparticles->value)
		return; 

	// if d_8to24table is global.. there's no need to pass a huge array on the stack
	GL_DrawComplexParticles( r_newrefdef.num_dhcomparticles, r_newrefdef.dhcomparticles );
}

//---------------------------------------------------------------------------
// R_DrawDepthHack()
//---------------------------------------------------------------------------
void R_DrawDepthHack(void)
{
	int i;
	entity_t *oldlist=r_newrefdef.entities;
	int oldcount=r_newrefdef.num_entities;

	for (i=0; i<depthHackCount; i++)
		depthHackList[i].flags &= ~RF_DEPTHHACK;

	// clear z buffer
	GL_SetState( GLSTATE_PRESET2 & ~GLSTATE_DEPTH_TEST );
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );
//	qglClear( GL_DEPTH_BUFFER_BIT );

	qglDepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
	// entities

	// change newrefdef entity list to my list of depthhack ents
	r_newrefdef.entities=depthHackList;
	r_newrefdef.num_entities=depthHackCount;
	r_newrefdef.alphaentities=depthHackAlphaList;
	r_newrefdef.num_alphaentities=depthHackAlphaCount;

	// draw depthhack entities
	R_DrawEntitiesOnList();
	R_DrawAlphaEntitiesOnList();

	// restore newrefdef entity list
	r_newrefdef.entities=oldlist;
	r_newrefdef.num_entities=oldcount;

	// complex particles

	// draw comparticles
//	R_DrawComplexDepthHackParticles();

	// it was so much fun the first time, let's clear the Z buffer again!!!
//	qglClear( GL_DEPTH_BUFFER_BIT );
	qglDepthRange (gldepthmin, gldepthmax);

}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/

extern int  scrap_uploads;
extern void inventory_DrawIcons (void);
extern void GL_Boost_Icons_Display(void);
//void R_ProceduralFrame();

void R_RenderView (refdef_t *fd)
{
	if (r_norefresh->value)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		ri.Sys_Error (ERR_DROP, "R_RenderView: NULL worldmodel");

	if (r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
		c_lightmap_polys = 0;
		c_damage_polys = 0;
		scrap_uploads = 0;

		c_visible_textures = 0;
		c_visible_texture_bytes = 0;
		memset(c_texture_list, 0, sizeof(c_texture_list));

		c_visible_lightmaps = 0;
		c_visible_lightmap_bytes = 0;
		memset(c_lightmap_list, 0, sizeof(c_lightmap_list));

	}

	g_PVCullCount = 0;
	g_PVDrawCount = 0;

	text_numEntities = 0;

	currententity = NULL;
	currentmodel = NULL;

	R_PushDlights ();

	if (gl_finish->value)
		qglFinish ();

	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_SetupParticleVolumes();

	R_DrawWorld ();

	R_DrawFullbrightSurfaces();

	R_DrawMidTextures();

	R_DrawEntitiesOnList ();

	R_DrawFullbrightSurfaces();

	R_DrawSurfaceSprites();

	R_DrawAlphaSurfaces();

	R_DrawParticles ();

	R_DrawComplexParticles ();

	R_DrawDepthHack();				// mike: weapon hack

	R_DrawFogVolumes(); // Ash -- draw after alpha surfaces to get the fog bottom surface on top of water.

	R_DrawBModelMidTextureFogShells();  // draw after midtextures, so bmodel midtextures get fogged too
                                        // also draw after fogvolumes, so midtextures don't block fog

	beam_DrawBeamList ();

	Text_DrawEntities ();

	R_RenderDlights ();

	R_PolyBlend();

//	R_DrawDepthHack();				// mike: weapon hack

	if( ! (r_newrefdef.rdflags & ( RDF_NOWORLDMODEL | RDF_LETTERBOX ) ) )  // don't draw in cinematic mode
	{
		inventory_DrawIcons ();		 // draw inventory models
		GL_Boost_Icons_Display();  // draw skill boost models
	}

	if (r_speeds->value)
	{
		float texMB = c_visible_texture_bytes / 1024000.0;
		float lmapMB = c_visible_lightmap_bytes / 1024000.0;

		ri.Con_Printf (PRINT_ALL, "WP:%04i  LP:%04i  SS: %4i  EP:%04i  TX:%i - %.2fMB  LM:%0i - %0.2fMB  T:%0.2fMB\n",
			c_brush_polys, 
			c_lightmap_polys,
			c_damage_polys,
			c_alias_polys, 
			c_visible_textures, 
			texMB,
			c_visible_lightmaps, 
			lmapMB, 
			texMB + lmapMB); 
	}

	if (gl_mem->value)
	{
		ri.Con_Printf (PRINT_ALL, "%9i bytes skin memory\n", c_skin_memory);
		ri.Con_Printf (PRINT_ALL, "%9i bytes sprite memory\n", c_sprite_memory);
		ri.Con_Printf (PRINT_ALL, "%9i bytes wal memory\n", c_wal_memory);
		ri.Con_Printf (PRINT_ALL, "%9i bytes pic memory\n", c_pic_memory);
		ri.Con_Printf (PRINT_ALL, "%9i bytes sky memory\n", c_sky_memory);
	}
	
	if (r_countpvolumes->value)
		ri.dk_printxy (0, 64, "Drew %i, culled %i\n", g_PVDrawCount, g_PVCullCount);
}

void	R_SetGL2D (void)
{
	// set 2D virtual screen size
	qglViewport (0,0, vid.width, vid.height);
	qglMatrixMode(GL_PROJECTION);
    qglLoadIdentity ();
	qglOrtho  (0, vid.width, vid.height, 0, -99999, 99999);
	qglMatrixMode(GL_MODELVIEW);
    qglLoadIdentity ();
	qglColor4f (1,1,1,1);
    qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	GL_SetState( GLSTATE_ALPHA_TEST | GLSTATE_TEXTURE_2D );
}

static void GL_DrawColoredStereoLinePair( float r, float g, float b, float y )
{
	qglColor3f( r, g, b );
	qglVertex2f( 0, y );
	qglVertex2f( vid.width, y );
	qglColor3f( 0, 0, 0 );
	qglVertex2f( 0, y + 1 );
	qglVertex2f( vid.width, y + 1 );
}

static void GL_DrawStereoPattern( void )
{
	int i;

	if ( !( gl_config.renderer & GL_RENDERER_INTERGRAPH ) )
		return;

	if ( !gl_state.stereo_enabled )
		return;

	R_SetGL2D();

	qglDrawBuffer( GL_BACK_LEFT );

	for ( i = 0; i < 20; i++ )
	{
		qglBegin( GL_LINES );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 0 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 2 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 4 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 6 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 8 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 10);
			GL_DrawColoredStereoLinePair( 1, 1, 0, 12);
			GL_DrawColoredStereoLinePair( 0, 1, 0, 14);
		qglEnd();
		
		RImp_EndFrame();
	}
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void)
{
	CVector		shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)

	R_LightPoint (r_newrefdef.vieworg, shadelight );

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight.x > shadelight.y)
	{
		if (shadelight.x > shadelight.z)
			r_lightlevel->value = 150*shadelight.x;
		else
			r_lightlevel->value = 150*shadelight.z;
	}
	else
	{
		if (shadelight.y > shadelight.z)
			r_lightlevel->value = 150*shadelight.y;
		else
			r_lightlevel->value = 150*shadelight.z;
	}

}

///////////////////////////////////////////////////////////////////////////////
//	R_InitTiming
//
//	initialize timing vars when render dll starts up
/////////////////////////////////////////////////////////////////////////////////

void	R_InitTiming (void)
{
	ref_curTime = 0;
	ref_frameTime = 0;
	ref_lastTime = timeGetTime ();
	ref_globalTime = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	R_DoRotations
//
///////////////////////////////////////////////////////////////////////////////

void	R_DoRotations (void)
{
	ref_laserRotation = (ref_laserRotation + (60 * ref_frameTime * (float) ref_laserDir));
	if (ref_laserDir == 1 && ref_laserRotation > 360)
	{
		ref_laserRotation -= 360.0;
	}
}

/*
@@@@@@@@@@@@@@@@@@@@@
RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void RenderFrame (refdef_t *fd)
{
	//	set palette to default
	GL_SetTexturePalette( NULL, FALSE );

	///////////////////////////////////////////////////////////////////////////
	//	calc frame time for time-based animations, rotations and procedural stuff
	///////////////////////////////////////////////////////////////////////////
	ref_curTime = timeGetTime ();
	ref_frameTime = ((float) (ref_curTime - ref_lastTime)) / 1000.0;
	ref_lastTime = ref_curTime;
	ref_globalTime += ref_frameTime;

	R_DoRotations ();

	R_RenderView (fd);

	R_SetLightLevel ();
	R_SetGL2D ();
}

void R_InitSurfaceSprites();

void R_Register( void )
{
	r_countpvolumes = ri.Cvar_Get ("r_pvolumes", "0", 0);
	r_countsnow = ri.Cvar_Get ("r_countsnow", "0", 0);
	r_countents = ri.Cvar_Get ("r_countents", "0", 0);
	r_drawbmodels = ri.Cvar_Get ("r_drawbmodels", "1", 0);
	r_drawalias = ri.Cvar_Get ("r_drawalias", "1", 0);
	r_drawsprites = ri.Cvar_Get ("r_drawsprites", "1", 0);	
	//r_drawweapon = ri.Cvar_Get ("r_drawweapon", "1", 0);
	r_drawcomparticles = ri.Cvar_Get ("r_drawcomparticles", "1", 0);

    weapon_visible = ri.Cvar_Get("weapon_visible", "1", CVAR_ARCHIVE);

	//r_lefthand = ri.Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	r_norefresh = ri.Cvar_Get ("r_norefresh", "0", 0);
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get ("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get ("r_nocull", "0", 0);
	r_lerpmodels = ri.Cvar_Get ("r_lerpmodels", "1", 0);
	r_speeds = ri.Cvar_Get ("r_speeds", "0", 0);

	r_lightlevel = ri.Cvar_Get ("r_lightlevel", "0", 0);

//    console_screen_idx = ri.Cvar_Get ("console_screen_idx", "0", CVAR_ARCHIVE); // 12.21  dsn
//    if (console_screen_idx->value < 0 || console_screen_idx->value > ((NUM_RANDOM_BKG_SCREENS-1) * 2)) 
//      ri.Cvar_SetValue("console_screen_idx", 0);

	gl_nosubimage = ri.Cvar_Get( "gl_nosubimage", "0", 0 );
	gl_allow_software = ri.Cvar_Get( "gl_allow_software", "1", CVAR_ARCHIVE );

	gl_particle_min_size = ri.Cvar_Get( "gl_particle_min_size", "2", CVAR_ARCHIVE );
	gl_particle_max_size = ri.Cvar_Get( "gl_particle_max_size", "40", CVAR_ARCHIVE );
	gl_particle_size = ri.Cvar_Get( "gl_particle_size", "40", CVAR_ARCHIVE );
	gl_particle_att_a = ri.Cvar_Get( "gl_particle_att_a", "0.01", CVAR_ARCHIVE );
	gl_particle_att_b = ri.Cvar_Get( "gl_particle_att_b", "0.0", CVAR_ARCHIVE );
	gl_particle_att_c = ri.Cvar_Get( "gl_particle_att_c", "0.01", CVAR_ARCHIVE );

	gl_modulate = ri.Cvar_Get ("gl_modulate", "2", CVAR_ARCHIVE );
	gl_log = ri.Cvar_Get( "gl_log", "0", 0 );
	gl_bitdepth = ri.Cvar_Get( "gl_bitdepth", "0", 0 );
	gl_mode = ri.Cvar_Get( "gl_mode", "3", CVAR_ARCHIVE);
	gl_lightmap = ri.Cvar_Get ("gl_lightmap", "0", 0);
	gl_shadows = ri.Cvar_Get ("gl_shadows", "0", CVAR_ARCHIVE);
	gl_dynamic = ri.Cvar_Get ("gl_dynamic", "1", 0);
	gl_nobind = ri.Cvar_Get ("gl_nobind", "0", 0);
	gl_round_down = ri.Cvar_Get ("gl_round_down", "1", 0);
	gl_picmip = ri.Cvar_Get ("gl_picmip", "0", CVAR_ARCHIVE);
	gl_skymip = ri.Cvar_Get ("gl_skymip", "0", 0);
	gl_showtris = ri.Cvar_Get ("gl_showtris", "0", 0);
	gl_ztrick = ri.Cvar_Get ("gl_ztrick", "0", 0);
	gl_finish = ri.Cvar_Get ("gl_finish", "0", 0);
	gl_clear = ri.Cvar_Get ("gl_clear", "0", 0);
	gl_cull = ri.Cvar_Get ("gl_cull", "1", 0);
	gl_polyblend = ri.Cvar_Get ("gl_polyblend", "1", 0);
	gl_flashblend = ri.Cvar_Get ("gl_flashblend", "0", 0);
	gl_playermip = ri.Cvar_Get ("gl_playermip", "0", 0);
	gl_monolightmap = ri.Cvar_Get( "gl_monolightmap", "0", 0 );
	gl_driver = ri.Cvar_Get( "gl_driver", "opengl32", CVAR_ARCHIVE );
	gl_texturemode = ri.Cvar_Get( "gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	gl_lockpvs = ri.Cvar_Get ("gl_lockpvs", "0", 0);
	gl_drawflat = ri.Cvar_Get ("gl_drawflat", "0", 0);
	gl_polylines = ri.Cvar_Get ("gl_polylines", "0", 0);
	gl_vfog = ri.Cvar_Get ("gl_vfog", "1", CVAR_ARCHIVE);
	gl_vfogents = ri.Cvar_Get ("gl_vfogents", "1", CVAR_ARCHIVE);
	gl_vertnormals = ri.Cvar_Get ("gl_vertnormals", "0", 0);
	gl_envmap = ri.Cvar_Get ("gl_envmap", "0", CVAR_ARCHIVE);
	gl_texenvmap = ri.Cvar_Get ("gl_texenvmap", "0", 0);
	gl_texenvcolor = ri.Cvar_Get ("gl_texenvcolor", "0.5", 0);
	gl_predator = ri.Cvar_Get ("gl_predator", "0", 0);
	gl_midtextures = ri.Cvar_Get ("gl_midtextures", "1", 0);
	gl_surfacesprites = ri.Cvar_Get ("gl_surfacesprites", "1", CVAR_ARCHIVE);
	gl_alphasurfaces = ri.Cvar_Get ("gl_alphasurfaces", "1", 0);
	gl_fogsurfaces = ri.Cvar_Get ("gl_fogsurfaces", "1", 0);
	gl_procshell = ri.Cvar_Get ("gl_procshell", "0", 0);
	gl_procp = ri.Cvar_Get ("gl_procp", "3", 0);
	gl_procm = ri.Cvar_Get ("gl_procm", "8", 0);
	gl_procd = ri.Cvar_Get ("gl_procd", "10", 0);
	gl_procn = ri.Cvar_Get ("gl_procn", "3", 0);
	gl_bbox = ri.Cvar_Get ("gl_bbox", "0", 0);
	gl_texturealphamode = ri.Cvar_Get( "gl_texturealphamode", "default", CVAR_ARCHIVE );	// SCG[5/20/99]: Quake2 V3.19 addition
	gl_texturesolidmode = ri.Cvar_Get( "gl_texturesolidmode", "default", CVAR_ARCHIVE );	// SCG[5/20/99]: Quake2 V3.19 addition
	gl_brute_force_state = ri.Cvar_Get( "gl_brute_force_state", "0", CVAR_ARCHIVE );
	gl_24bittextures = ri.Cvar_Get( "gl_24bittextures", "0", CVAR_ARCHIVE );
	gl_32bitatextures = ri.Cvar_Get( "gl_32bitatextures", "1", CVAR_ARCHIVE );
	

    gl_skyname = ri.Cvar_Get("gl_skyname", "", 0);
    gl_cloudname = ri.Cvar_Get("gl_cloudname", "", 0);
    gl_lightningfreq = ri.Cvar_Get("gl_lightningfreq", "0.25", 0);
    gl_cloudxdir = ri.Cvar_Get("gl_cloudxdir", "1", 0);
    gl_cloudydir = ri.Cvar_Get("gl_cloudydir", ".8", 0);
    gl_cloud1tile = ri.Cvar_Get("gl_cloud1tile", "8", 0);
    gl_cloud1speed = ri.Cvar_Get("gl_cloud1speed", "1", 0);
    gl_cloud2tile = ri.Cvar_Get("gl_cloud2tile", "2", 0);
    gl_cloud2speed = ri.Cvar_Get("gl_cloud2speed", "4", 0);
    gl_cloud2alpha = ri.Cvar_Get("gl_cloud2alpha", "0.7", 0);
    
    gl_skyname_2 = ri.Cvar_Get("gl_skyname_2", "", 0);
    gl_cloudname_2 = ri.Cvar_Get("gl_cloudname_2", "", 0);
    gl_lightningfreq_2 = ri.Cvar_Get("gl_lightningfreq_2", "0.25", 0);
    gl_cloudxdir_2 = ri.Cvar_Get("gl_cloudxdir_2", "1", 0);
    gl_cloudydir_2 = ri.Cvar_Get("gl_cloudydir_2", ".8", 0);
    gl_cloud1tile_2 = ri.Cvar_Get("gl_cloud1tile_2", "8", 0);
    gl_cloud1speed_2 = ri.Cvar_Get("gl_cloud1speed_2", "1", 0);
    gl_cloud2tile_2 = ri.Cvar_Get("gl_cloud2tile_2", "2", 0);
    gl_cloud2speed_2 = ri.Cvar_Get("gl_cloud2speed_2", "4", 0);
    gl_cloud2alpha_2 = ri.Cvar_Get("gl_cloud2alpha_2", "0.7", 0);
    
    gl_skyname_3 = ri.Cvar_Get("gl_skyname_3", "", 0);
    gl_cloudname_3 = ri.Cvar_Get("gl_cloudname_3", "", 0);
    gl_lightningfreq_3 = ri.Cvar_Get("gl_lightningfreq_3", "0.25", 0);
    gl_cloudxdir_3 = ri.Cvar_Get("gl_cloudxdir_3", "1", 0);
    gl_cloudydir_3 = ri.Cvar_Get("gl_cloudydir_3", ".8", 0);
    gl_cloud1tile_3 = ri.Cvar_Get("gl_cloud1tile_3", "8", 0);
    gl_cloud1speed_3 = ri.Cvar_Get("gl_cloud1speed_3", "1", 0);
    gl_cloud2tile_3 = ri.Cvar_Get("gl_cloud2tile_3", "2", 0);
    gl_cloud2speed_3 = ri.Cvar_Get("gl_cloud2speed_3", "4", 0);
    gl_cloud2alpha_3 = ri.Cvar_Get("gl_cloud2alpha_3", "0.7", 0);
    
    gl_skyname_4 = ri.Cvar_Get("gl_skyname_4", "", 0);
    gl_cloudname_4 = ri.Cvar_Get("gl_cloudname_4", "", 0);
    gl_lightningfreq_4 = ri.Cvar_Get("gl_lightningfreq_4", "0.25", 0);
    gl_cloudxdir_4 = ri.Cvar_Get("gl_cloudxdir_4", "1", 0);
    gl_cloudydir_4 = ri.Cvar_Get("gl_cloudydir_4", ".8", 0);
    gl_cloud1tile_4 = ri.Cvar_Get("gl_cloud1tile_4", "8", 0);
    gl_cloud1speed_4 = ri.Cvar_Get("gl_cloud1speed_4", "1", 0);
    gl_cloud2tile_4 = ri.Cvar_Get("gl_cloud2tile_4", "2", 0);
    gl_cloud2speed_4 = ri.Cvar_Get("gl_cloud2speed_4", "4", 0);
    gl_cloud2alpha_4 = ri.Cvar_Get("gl_cloud2alpha_4", "0.7", 0);
    
    gl_skyname_5 = ri.Cvar_Get("gl_skyname_5", "", 0);
    gl_cloudname_5 = ri.Cvar_Get("gl_cloudname_5", "", 0);
    gl_lightningfreq_5 = ri.Cvar_Get("gl_lightningfreq_5", "0.25", 0);
    gl_cloudxdir_5 = ri.Cvar_Get("gl_cloudxdir_5", "1", 0);
    gl_cloudydir_5 = ri.Cvar_Get("gl_cloudydir_5", ".8", 0);
    gl_cloud1tile_5 = ri.Cvar_Get("gl_cloud1tile_5", "8", 0);
    gl_cloud1speed_5 = ri.Cvar_Get("gl_cloud1speed_5", "1", 0);
    gl_cloud2tile_5 = ri.Cvar_Get("gl_cloud2tile_5", "2", 0);
    gl_cloud2speed_5 = ri.Cvar_Get("gl_cloud2speed_5", "4", 0);
    gl_cloud2alpha_5 = ri.Cvar_Get("gl_cloud2alpha_5", "0.7", 0);
    
	gl_subdivide_size = ri.Cvar_Get("gl_subdivide_size", "256", CVAR_ARCHIVE);

	gl_no_error_check = ri.Cvar_Get("gl_no_error_check", "1", CVAR_ARCHIVE);

	gl_no_alias_models = ri.Cvar_Get("gl_no_alias_models", "0", 0);

	gl_drawfog = ri.Cvar_Get("gl_drawfog", "1", 0);

	gl_vertex_arrays = ri.Cvar_Get( "gl_vertex_arrays", "0", CVAR_ARCHIVE );

	gl_ext_swapinterval = ri.Cvar_Get( "gl_ext_swapinterval", "1", CVAR_ARCHIVE );
	gl_ext_gamma_control = ri.Cvar_Get( "gl_ext_gamma", "1", CVAR_ARCHIVE );
	gl_ext_multitexture = ri.Cvar_Get( "gl_ext_multitexture", "1", CVAR_ARCHIVE );
	gl_ext_pointparameters = ri.Cvar_Get( "gl_ext_pointparameters", "1", CVAR_ARCHIVE );

	gl_drawbuffer = ri.Cvar_Get( "gl_drawbuffer", "GL_BACK", 0 );
	gl_swapinterval = ri.Cvar_Get( "gl_swapinterval", "1", CVAR_ARCHIVE );

	gl_saturatelighting = ri.Cvar_Get( "gl_saturatelighting", "0", 0 );

	gl_3dlabs_broken = ri.Cvar_Get( "gl_3dlabs_broken", "1", CVAR_ARCHIVE );

	vid_fullscreen = ri.Cvar_Get( "vid_fullscreen", "0", CVAR_ARCHIVE );
	vid_gamma = ri.Cvar_Get( "vid_gamma", "1.0", CVAR_ARCHIVE );
	vid_ref = ri.Cvar_Get( "vid_ref", "soft", CVAR_ARCHIVE );

	r_palettedir = ri.Cvar_Get( "r_palettedir", "", 0 );

	r_drawparticleflag = ri.Cvar_Get( "r_drawparticleflag", "1", 0 );
	r_useimagelog = ri.Cvar_Get( "r_useimagelog", "0", CVAR_ARCHIVE );
	gl_mem = ri.Cvar_Get ("gl_mem", "0", 0);

	gl_alphatest = ri.Cvar_Get ("gl_alphatest", "0.5", 0);

	r_fogcolor = ri.Cvar_Get ("r_fogcolor", "128,128,128", 0);
	r_testfog = ri.Cvar_Get ("r_testfog", "0", 0);
	r_fognear = ri.Cvar_Get ("r_fognear", "4", 0);
	r_fogfar = ri.Cvar_Get ("r_fogfar", "2048", 0);
	r_skyfogfar = ri.Cvar_Get ("r_skyfogfar", "2048", 0);

	gl_clip_surface_sprites = ri.Cvar_Get ("gl_clip_surface_sprites", "1", 0);

//	pt_type = ri.Cvar_Get( "pt_type", "0", 0);

	ri.Cmd_AddCommand( "imagelist", GL_ImageList_f );
	ri.Cmd_AddCommand( "screenshot", GL_ScreenShot_f );
	ri.Cmd_AddCommand( "modellist", Mod_Modellist_f );
	ri.Cmd_AddCommand( "gl_strings", GL_Strings_f );
	ri.Cmd_AddCommand( "gl_clear_surface_sprites", R_InitSurfaceSprites );
}

/*
==================
R_SetMode
==================
*/
qboolean R_SetMode (void)
{
	rserr_t err;
	qboolean fullscreen;

	if ( vid_fullscreen->modified && !gl_config.allow_cds )
	{
		ri.Con_Printf( PRINT_ALL, "R_SetMode() - CDS not allowed with this driver\n" );
		ri.Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->value );
		vid_fullscreen->modified = false;
	}

	fullscreen = vid_fullscreen->value;

	vid_fullscreen->modified = false;
	gl_mode->modified = false;

	if ( ( err = (rserr_t)RImp_SetMode( (int*)&vid.width, (int*)&vid.height, gl_mode->value, fullscreen ) ) == rserr_ok )
	{
		gl_state.prev_mode = gl_mode->value;
	}
	else
	{
		if ( err == rserr_invalid_fullscreen )
		{
			ri.Cvar_SetValue( "vid_fullscreen", 0);
			vid_fullscreen->modified = false;
			ri.Con_Printf( PRINT_ALL, "ref_gl::R_SetMode() - fullscreen unavailable in this mode\n" );
			if ( ( err = (rserr_t)RImp_SetMode( (int*)&vid.width, (int*)&vid.height, gl_mode->value, false ) ) == rserr_ok )
				return true;
		}
		else if ( err == rserr_invalid_mode )
		{
			ri.Cvar_SetValue( "gl_mode", gl_state.prev_mode );
			gl_mode->modified = false;
			ri.Con_Printf( PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n" );
		}

		// try setting it back to something safe
		if ( ( err = (rserr_t)RImp_SetMode( (int*)&vid.width, (int*)&vid.height, gl_state.prev_mode, false ) ) != rserr_ok )
		{
			ri.Con_Printf( PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n" );
			return false;
		}
	}


	gl_state.nColorBits = RImp_GetBitDepth();
	gl_state.nDepthBits = RImp_GetZDepth();

	return true;
}

/*
===============
R_Init
===============
*/
int R_Init( void *hinstance, void *hWnd )
{	
	char renderer_buffer[1000];
	int		err;
	int		j;
	extern float r_turbsin[256];

	g_currentTexPalette = -3;

	GL_InitState();

	R_InitTiming ();		//	Nelno:	set up timing vars

	for ( j = 0; j < 256; j++ )
	{
		r_turbsin[j] *= 0.5;
	}

	c_skin_memory = 0;
	c_sprite_memory = 0;
	c_wal_memory = 0;
	c_pic_memory = 0;
	c_sky_memory = 0;

	ri.Con_Printf (PRINT_ALL, "ref_gl version: "REF_VERSION"\n");

	R_Register();

	Draw_GetPalette ();

	// initialize our QGL dynamic bindings
	if ( !QGL_Init( gl_driver->string ) )
	{
		QGL_Shutdown();
        ri.Con_Printf (PRINT_ALL, "ref_gl::R_Init() - could not load \"%s\"\n", gl_driver->string );
		return -1;
	}

	// initialize OS-specific parts of OpenGL
	if ( !RImp_Init( hinstance, hWnd ) )
	{
		QGL_Shutdown();
		return -1;
	}

	// set our "safe" modes
	gl_state.prev_mode = 3;

	// create the window and set up the context
	if ( !R_SetMode () )
	{
		QGL_Shutdown();
        ri.Con_Printf (PRINT_ALL, "ref_gl::R_Init() - could not R_SetMode()\n" );
		return -1;
	}

	ri.Vid_MenuInit();

	/*
	** get the bit depth (and special case the crappy 3dfx minidriver that doesn't do these glGet's)
	*/
	if (strstr(strlwr(gl_driver->string), "3dfxgl"))
	{
		gl_bit_depth = 16;
	}
	else
	{
		int bits;
		gl_bit_depth = 0;
		qglGetIntegerv(GL_RED_BITS, &bits);
		gl_bit_depth+=bits;
		qglGetIntegerv(GL_GREEN_BITS, &bits);
		gl_bit_depth+=bits;
		qglGetIntegerv(GL_BLUE_BITS, &bits);
		gl_bit_depth+=bits;
		qglGetIntegerv(GL_ALPHA_BITS, &bits);
		gl_bit_depth+=bits;

		// sanity check here, since we know some gl drivers don't report this info
		if(!((gl_bit_depth == 16)||(gl_bit_depth == 32)))
			gl_bit_depth = 16;
	}

	/*
	** get our various GL strings
	*/
	gl_config.vendor_string = (const char*)qglGetString (GL_VENDOR);
	ri.Con_Printf (PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string );
	gl_config.renderer_string = (const char*)qglGetString (GL_RENDERER);
	ri.Con_Printf (PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string );
	gl_config.version_string = (const char*)qglGetString (GL_VERSION);
	ri.Con_Printf (PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string );
	gl_config.extensions_string = (const char*)qglGetString (GL_EXTENSIONS);
    ri.Con_Printf (PRINT_ALL, "GL_EXTENSIONS:\n");
    
    // copy, tokenize and list the extensions
    char *ext_string = _strdup(gl_config.extensions_string);
    char *ext = strtok((char *)ext_string, " ");
    while (ext)
    {
        ri.Con_Printf (PRINT_ALL,"%s\n", ext);
        ext = strtok(NULL, " ");
    }
    // free it
    free(ext_string);

	strcpy( renderer_buffer, gl_config.renderer_string );
	strlwr( renderer_buffer );

	if ( strstr( renderer_buffer, "voodoo" ) )
	{
		if ( !strstr( renderer_buffer, "rush" ) )
			gl_config.renderer = GL_RENDERER_VOODOO;
		else
			gl_config.renderer = GL_RENDERER_VOODOO_RUSH;
	}

	else if ( strstr( renderer_buffer, "permedia" ) )
		gl_config.renderer = GL_RENDERER_PERMEDIA2;
	else if ( strstr( renderer_buffer, "glint" ) )
		gl_config.renderer = GL_RENDERER_GLINT_MX;
	else if ( strstr( renderer_buffer, "glzicd" ) )
		gl_config.renderer = GL_RENDERER_REALIZM;
	else if ( strstr( renderer_buffer, "gdi" ) )
		gl_config.renderer = GL_RENDERER_MCD;
	else if ( strstr( renderer_buffer, "pcx2" ) )
		gl_config.renderer = GL_RENDERER_PCX2;
	else if ( strstr( renderer_buffer, "verite" ) )
		gl_config.renderer = GL_RENDERER_RENDITION;
	else
		gl_config.renderer = GL_RENDERER_OTHER;

	if ( toupper( gl_monolightmap->string[1] ) != 'F' )
	{
		if ( gl_config.renderer == GL_RENDERER_PERMEDIA2 )
		{
			ri.Cvar_Set( "gl_monolightmap", "A" );
			ri.Con_Printf( PRINT_ALL, "...using gl_monolightmap 'a'\n" );
		}
		else if ( gl_config.renderer & GL_RENDERER_POWERVR ) 
		{
			ri.Cvar_Set( "gl_monolightmap", "0" );
		}
		else
		{
			ri.Cvar_Set( "gl_monolightmap", "0" );
		}
	}

	// power vr can't have anything stay in the framebuffer, so
	// the screen needs to redraw the tiled background every frame
	/*
	if ( gl_config.renderer & GL_RENDERER_POWERVR ) 
	{
		ri.Cvar_Set( "scr_drawall", "1" );
	}
	else
	{
		ri.Cvar_Set( "scr_drawall", "0" );
	}
	*/

	// MCD has buffering issues
	if ( gl_config.renderer == GL_RENDERER_MCD )
	{
		ri.Cvar_SetValue( "gl_finish", 1 );
	}

	if ( gl_config.renderer & GL_RENDERER_3DLABS )
	{
		if ( gl_3dlabs_broken->value )
			gl_config.allow_cds = false;
		else
			gl_config.allow_cds = true;
	}
	else
	{
		gl_config.allow_cds = true;
	}

	if ( gl_config.allow_cds )
		ri.Con_Printf( PRINT_ALL, "...allowing CDS\n" );
	else
		ri.Con_Printf( PRINT_ALL, "...disabling CDS\n" );

	/*
	** grab extensions
	*/
#ifdef WIN32
	// SCG[5/20/99]: Quake2 V3.19 addition
	if ( strstr( gl_config.extensions_string, "GL_EXT_compiled_vertex_array" ) || 
		 strstr( gl_config.extensions_string, "GL_SGI_compiled_vertex_array" ) )
	{
		ri.Con_Printf( PRINT_ALL, "...enabling GL_EXT_compiled_vertex_array\n" );
		qglLockArraysEXT = ( void (APIENTRY *) ( int, int ) ) qwglGetProcAddress( "glLockArraysEXT" );
		qglUnlockArraysEXT = ( void (APIENTRY *) ( void ) ) qwglGetProcAddress( "glUnlockArraysEXT" );
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	if ( strstr( gl_config.extensions_string, "WGL_EXT_swap_control" ) )
	{
		qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
		ri.Con_Printf( PRINT_ALL, "...enabling WGL_EXT_swap_control\n" );
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
	}

	if ( strstr( gl_config.extensions_string, "WGL_NVIDIA_gamma_control" ) ||
		 strstr( gl_config.extensions_string, "WGL_EXT_gamma_control" ) )
	{
		if ( gl_ext_gamma_control->value )
		{
			qwglGetDeviceGammaRampEXT = ( BOOL (WINAPI *)(unsigned char*, unsigned char*, unsigned char *)) qwglGetProcAddress( "wglGetDeviceGammaRampNVIDIA" );
			qwglSetDeviceGammaRampEXT = ( BOOL (WINAPI *)(const unsigned char *, const unsigned char *, const unsigned char *)) qwglGetProcAddress( "wglSetDeviceGammaRampNVIDIA" );

			ri.Con_Printf( PRINT_ALL, "...enabling WGL_EXT_gamma_control\n" );

			qwglGetDeviceGammaRampEXT(	gl_state.originalRedGammaTable,
										gl_state.originalGreenGammaTable,
										gl_state.originalBlueGammaTable );
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, "...ignoring WGL_EXT_gamma_control\n" );
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...WGL_EXT_gamma_control not found\n" );
	}

	if ( strstr( gl_config.extensions_string, "GL_EXT_point_parameters" ) )
	{
		if ( gl_ext_pointparameters->value )
		{
			qglPointParameterfEXT = ( void (APIENTRY *)( GLenum, GLfloat ) ) qwglGetProcAddress( "glPointParameterfEXT" );
			qglPointParameterfvEXT = ( void (APIENTRY *)( GLenum, const GLfloat * ) ) qwglGetProcAddress( "glPointParameterfvEXT" );
			ri.Con_Printf( PRINT_ALL, "...using GL_EXT_point_parameters\n" );
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, "...ignoring GL_EXT_point_parameters\n" );
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...GL_EXT_point_parameters not found\n" );
	}

	if ( strstr( gl_config.extensions_string, "GL_EXT_paletted_texture" ) && 
		 strstr( gl_config.extensions_string, "GL_EXT_shared_texture_palette" ) )
	{
		ri.Con_Printf( PRINT_ALL, "...using GL_EXT_shared_texture_palette\n" );
		qglColorTableEXT = ( void ( APIENTRY * ) ( int, int, int, int, int, const void * ) ) qwglGetProcAddress( "glColorTableEXT" );
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...GL_EXT_shared_texture_palette not found\n" );
	}

	if ( strstr( gl_config.extensions_string, "GL_SGIS_multitexture" ) )
	{
		if ( gl_ext_multitexture->value )
		{
			ri.Con_Printf( PRINT_ALL, "...using GL_SGIS_multitexture\n" );
			qglMTexCoord2fSGIS = (void (APIENTRY *)(unsigned int,float,float)) qwglGetProcAddress( "glMTexCoord2fSGIS" );
			qglSelectTextureSGIS = (void (APIENTRY *)(unsigned int)) qwglGetProcAddress( "glSelectTextureSGIS" );
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, "...ignoring GL_SGIS_multitexture\n" );
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...GL_SGIS_multitexture not found\n" );
	}
#endif

	GL_SetDefaultState();

	/*
	** draw our stereo patterns
	*/
	GL_DrawStereoPattern();

	R_OpenImageLog ();				//  Shawn:  added for image logging

	GL_InitImages ();
	Mod_Init ();
	R_InitNoTexture();
	R_InitParticles();
	R_InitMiscTextures ();

	Draw_InitLocal ();


	err = qglGetError();
	if ( err != GL_NO_ERROR )
		ri.Con_Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	return 1;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown (void)
{	
	ri.Cmd_RemoveCommand ("modellist");
	ri.Cmd_RemoveCommand ("screenshot");
	ri.Cmd_RemoveCommand ("imagelist");
	ri.Cmd_RemoveCommand ("gl_strings");
	ri.Cmd_RemoveCommand ("gl_clear_surface_sprites");

	Mod_FreeAll ();

	GL_ShutdownImages ();

	/*
	** shut down OS specific OpenGL stuff like contexts, etc.
	*/
	RImp_Shutdown();

	/*
	** shutdown our QGL subsystem
	*/
	QGL_Shutdown();

	R_CloseImageLog ();				//  Shawn:  added for image logging
}



/*
@@@@@@@@@@@@@@@@@@@@@
BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void BeginFrame( float camera_separation )
{

	gl_state.camera_separation = camera_separation;

	/*
	** change modes if necessary
	*/
	if ( gl_mode->modified || vid_fullscreen->modified )
	{	// FIXME: only restart if CDS is required
		cvar_t	*ref;

		ref = ri.Cvar_Get ("vid_ref", "gl", 0);
		ref->modified = true;
	}

	if ( gl_log->modified )
	{
		GLimp_EnableLogging( gl_log->value );
		gl_log->modified = false;
	}

	if ( gl_log->value )
	{
		GLimp_LogNewFrame();
	}

	/*
	** update 3Dfx gamma -- it is expected that a user will do a vid_restart
	** after tweaking this value
	*/
	if ( vid_gamma->modified )
	{
		vid_gamma->modified = false;

		if ( qwglSetDeviceGammaRampEXT != 0 )
		{
		}
		else if ( gl_config.renderer & ( GL_RENDERER_VOODOO ) )
		{
			char envbuffer[1024];
			float g;

			g = 2.00 * ( 0.8 - ( vid_gamma->value - 0.5 ) ) + 1.0F;
			Com_sprintf( envbuffer, sizeof(envbuffer), "SSTV2_GAMMA=%f", g );
			putenv( envbuffer );
			Com_sprintf( envbuffer, sizeof(envbuffer), "SST_GAMMA=%f", g );
			putenv( envbuffer );
		}
	}

	RImp_BeginFrame( camera_separation );

	/*
	** go into 2D mode
	*/
	R_SetGL2D();

	/*
	** draw buffer stuff
	*/
	if ( gl_drawbuffer->modified )
	{
		gl_drawbuffer->modified = false;

		if ( gl_state.camera_separation == 0 || !gl_state.stereo_enabled )
		{
			if ( stricmp( gl_drawbuffer->string, "GL_FRONT" ) == 0 )
				qglDrawBuffer( GL_FRONT );
			else
				qglDrawBuffer( GL_BACK );
		}
	}

	/*
	** texturemode stuff
	*/
	if ( gl_texturemode->modified )
	{
		GL_TextureMode( gl_texturemode->string );
		gl_texturemode->modified = false;
	}

	// SCG[5/20/99]: Quake2 V3.19 addition
	if ( gl_texturealphamode->modified )
	{
		GL_TextureAlphaMode( gl_texturealphamode->string );
		gl_texturealphamode->modified = false;
	}

	// SCG[5/20/99]: Quake2 V3.19 addition
	if ( gl_texturesolidmode->modified )
	{
		GL_TextureSolidMode( gl_texturesolidmode->string );
		gl_texturesolidmode->modified = false;
	}

	/*
	** swapinterval stuff
	*/
	GL_UpdateSwapInterval();

	//
	// clear screen if desired
	//
	R_Clear ();
}

///////////////////////////////////////////////////////////////////////////////
//	R_SetPalette
//
///////////////////////////////////////////////////////////////////////////////

unsigned r_rawpalette[256];

void R_CinematicSetPalette ( const unsigned char *palette)
{
	int		i;
	byte *rp = ( byte * ) r_rawpalette;

	if ( palette )
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = palette[i*3+0];
			rp[i*4+1] = palette[i*3+1];
			rp[i*4+2] = palette[i*3+2];
			rp[i*4+3] = 0xff;
		}
	}
	else
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = d_8to24table[i] & 0xff;
			rp[i*4+1] = ( d_8to24table[i] >> 8 ) & 0xff;
			rp[i*4+2] = ( d_8to24table[i] >> 16 ) & 0xff;
			rp[i*4+3] = 0xff;
		}

		GL_UploadPalette ( r_rawpalette, -1 );	
		return;
	}

	qglClearColor (0,0,0,0);
	qglClear (GL_COLOR_BUFFER_BIT);
	qglClearColor (1,0, 0.5 , 0.5);

	GL_UploadPalette ( r_rawpalette, -2 );
}

void R_SetInterfacePalette( const unsigned char *palette)
{
	R_CinematicSetPalette( palette );
}

#define START_ALPHA		0.25
#define END_ALPHA		0.05

// 1/16 * PI * 2
#define ONE_16TH_PI_SQR	0.3926990816986
#define TWO_THIRDS		0.6666666666666

///////////////////////////////////////////////////////////////////////////////
//
//  R_DrawSpotlight
//
///////////////////////////////////////////////////////////////////////////////
void R_DrawSpotlight (entity_t *e)
{
	int		i;
	float	a, sina, cosa;
	CVector	length;
	float	radius1, radius2;
	CVector	start_points[17], end_points[17];
	CVector	forward, right, up;

	AngleToVectors (e->angles, forward, right, up);

	qglShadeModel (GL_SMOOTH);

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE);
	// spotlights now store the destination point in mins
	length = e->render_scale - e->origin;

	radius1 = e->frame;

	GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );

	for (int light=0;light<2;light++)
	{

		radius2 = radius1 + (length.Length() * 0.2);

	#if 1  // amw - additional optimization

		// create start and end points
		for (i = 16; i>=0 ; i--)
		{
			// mdm 98.01.14 - hmm can't we rewrite this as:
			//a = i / 16.0   * M_PI*2;
			//a = i * 0.0625 * M_PI*2;
			a = i * ONE_16TH_PI_SQR; 
			
			// calculate these once for speed
			sina = sin(a);
			cosa = cos(a);
			
			start_points[i].x = e->origin.x + forward.x*radius1 + right.x * cosa * radius1 + up.x * sina * radius1;
			start_points[i].y = e->origin.y + forward.y*radius1 + right.y * cosa * radius1 + up.y * sina * radius1;
			start_points[i].z = e->origin.z + forward.z*radius1 + right.z * cosa * radius1 + up.z * sina * radius1;

			end_points[i].x = e->render_scale.x + forward.x*radius2 + right.x * cosa * radius2 + up.x * sina * radius2;
			end_points[i].y = e->render_scale.y + forward.y*radius2 + right.y * cosa * radius2 + up.y * sina * radius2;
			end_points[i].z = e->render_scale.z + forward.z*radius2 + right.z * cosa * radius2 + up.z * sina * radius2;

		}

	#else

	// create start points
		for (i = 16; i>=0 ; i--)
		{
			a = i/16.0 * M_PI*2;
			
			// calculate these once for speed
			sina = sin(a);
			cosa = cos(a);
			
			start_points[i].x = e->origin.x + forward.x*radius1 + right.x * cosa * radius1 + up.x * sina * radius1;
			start_points[i].y = e->origin.y + forward.y*radius1 + right.y * cosa * radius1 + up.y * sina * radius1;
			start_points[i].z = e->origin.z + forward.z*radius1 + right.z * cosa * radius1 + up.z * sina * radius1;
		}

	// create end points
		for (i = 16; i>=0 ; i--)
		{
			a = i/16.0 * M_PI*2;

			// calculate these once for speed
			sina = sin(a);
			cosa = cos(a);
			
			end_points[i].x = e->render_scale.x + forward.x*radius2 + right.x * cosa * radius2 + up.x * sina * radius2;
			end_points[i].y = e->render_scale.y + forward.y*radius2 + right.y * cosa * radius2 + up.y * sina * radius2;
			end_points[i].z = e->render_scale.z + forward.z*radius2 + right.z * cosa * radius2 + up.z * sina * radius2;
		}
	#endif

	// draw the tube (uh, tube?)
		qglBegin (GL_TRIANGLE_STRIP);

		for (i=0 ; i<16 ; i++)
		{			
			qglColor4f (e->color.x, e->color.y, e->color.z, START_ALPHA);
			qglVertex3fv (start_points[i]);
			qglVertex3fv (start_points[(i+1)%16]);
			qglColor4f (0, 0, 0, END_ALPHA);
			qglVertex3fv (end_points[i]);
			qglVertex3fv (end_points[(i+1)%16]);
		}
		// finish up last triangle of strip		
		qglColor4f (e->color.x, e->color.y, e->color.z, START_ALPHA);
		qglVertex3fv (start_points[0]);		
		qglColor4f (0, 0, 0, END_ALPHA);
		qglVertex3fv (end_points[0]);
		qglColor4f (e->color.x, e->color.y, e->color.z, START_ALPHA);
		qglVertex3fv (start_points[1]);

		qglEnd ();
		
		length = length * TWO_THIRDS;
		radius1	 *= TWO_THIRDS;			// ditto above
	}

	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//===================================================================

///////////////////////////////////////////////////////////////////////////////
//	PointOnLineSide
//
//	determines which side of a line a point lies on
//
//	return	
//	< 0 = left side
//	> 0 = rigt side
//	0 = point is on line
///////////////////////////////////////////////////////////////////////////////

#define PointOnLineSideXY(start, end, point, length) (((start [1] - point [1]) * (end [0] - start [0]) - (start [0] - point [0]) * (end [1] - start [1])) / (length * 2.0f))
#define PointOnLineSideXZ(start, end, point, length) (((start [2] - point [2]) * (end [0] - start [0]) - (start [0] - point [0]) * (end [2] - start [2])) / (length * 2.0f))

