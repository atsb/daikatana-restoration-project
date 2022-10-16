// r_alias.c: routines for setting up to draw alias models

/*
** use a real variable to control lerping
*/
#include "r_local.h"
#include "alias_quant.h"

#define LIGHT_MIN	5		// lowest light value we'll allow, to avoid the
							//  need for inner-loop light clamping

int				r_amodels_drawn;
int				model_version;

affinetridesc_t	r_affinetridesc;

CVector			r_plightvec;
CVector         r_lerped[1024];
CVector         r_lerp_frontv, r_lerp_backv, r_lerp_move;

int				r_ambientlight;
int				r_aliasblendcolor;
float			r_shadelight;


daliasframe_t	*r_thisframe, *r_lastframe;
daliasframe2_t	*r_thisframe2, *r_lastframe2;
dmdl_t			*s_pmdl;
dsurface_t		*s_psurface;

float	aliastransform[3][4];
float   aliasworldtransform[3][4];
float   aliasoldworldtransform[3][4];

static float	s_ziscale;
static CVector	s_alias_forward, s_alias_right, s_alias_up;

image_t			*pskindesc;

#if QCOMMON_ASH_NORMALS
    #include "vertnormals.h"
#else
    #define NUMVERTEXNORMALS 162
    #define PNT(x, y, z) {x, y, z}
    static float avertexnormals[NUMVERTEXNORMALS][3] = 
    {
        #include "anorms.h"
    };
#endif


void R_AliasSetUpLerpData( dmdl_t *pmdl, float backlerp );
void R_AliasSetUpTransform (void);
void R_AliasTransformVector (CVector &in, CVector &out, float m[3][4] );
void R_AliasProjectAndClipTestFinalVert (finalvert_t *fv);

void R_AliasTransformFinalVerts( int numpoints, finalvert_t *fv, dtrivertx_t *oldv, dtrivertx_t *newv );
void R_AliasTransformFinalVerts2( int numpoints, finalvert_t *fv, dtrivertx2_t *oldv, dtrivertx2_t *newv );

void R_AliasLerpFrames( dmdl_t *paliashdr, float backlerp );

void R_AliasSetupSurfaceSkin (int skinindex);
//static qboolean R_AliasSetupSurfaceSkin (int skinindex);

extern "C" void	(*d_pdrawspans)(void *);
extern "C" void R_PolysetDrawSpans8_Opaque( void * );
extern "C" void R_PolysetDrawSpans8_Opaque_f( void * );
extern "C" void R_PolysetDrawSpans8_33( void * );
extern "C" void R_PolysetDrawSpans8_66( void * );
extern "C" void R_PolysetDrawSpans8_33f( void * );
extern "C" void R_PolysetDrawSpans8_66f( void * );
extern "C" void R_PolysetDrawSpansConstant8_33( void * );
extern "C" void R_PolysetDrawSpansConstant8_66( void * );
void	(*d_pdrawspans)(void *);
void R_PolysetDrawSpans8_Opaque( void * );
void R_PolysetDrawSpans8_Opaque_f( void * );
void R_PolysetDrawSpans8_33( void * );
void R_PolysetDrawSpans8_66( void * );
void R_PolysetDrawSpans8_33f( void * );
void R_PolysetDrawSpans8_66f( void * );
void R_PolysetDrawSpansConstant8_33( void * );
void R_PolysetDrawSpansConstant8_66( void * );

/*
================
R_AliasCheckBBox
================
*/
typedef struct {
	int	index0;
	int	index1;
} aedge_t;

static aedge_t	aedges[12] = {
{0, 1}, {1, 2}, {2, 3}, {3, 0},
{4, 5}, {5, 6}, {6, 7}, {7, 4},
{0, 5}, {1, 4}, {2, 7}, {3, 6}
};

#define BBOX_TRIVIAL_ACCEPT 0
#define BBOX_MUST_CLIP_XY   1
#define BBOX_MUST_CLIP_Z    2
#define BBOX_TRIVIAL_REJECT 8

/*
** R_AliasCheckFrameBBox
**
** Checks a specific alias frame bounding box
*/
unsigned long R_AliasCheckFrameBBox( daliasframe_t *frame, float worldxf[3][4] )
{
	unsigned long	aggregate_and_clipcode = ~0U, 
					aggregate_or_clipcode = 0;
	int				i;
	CVector			mins, maxs;
	CVector			transformed_min, transformed_max;
	qboolean		zclipped = false, zfullyclipped = true;
	float			minz = 9999.0F;
	CVector			scale;

	if( model_version == ALIAS_VERSION2 ){
		scale.x = X_BIT_RES;
		scale.y = Y_BIT_RES;
		scale.z = Z_BIT_RES;
	}
	else{
		scale.x = scale.y = scale.z = 255;
	}

	/*
	** get the exact frame bounding box
	*/
	mins.x = frame->translate.x;
	maxs.x = mins.x + frame->scale.x * scale.x;

	mins.y = frame->translate.y;
	maxs.y = mins.y + frame->scale.y * scale.y;

	mins.z = frame->translate.z;
	maxs.z = mins.z + frame->scale.z * scale.z;

	/*
	** transform the min and max values into view space
	*/
	R_AliasTransformVector( mins, transformed_min, aliastransform );
	R_AliasTransformVector( maxs, transformed_max, aliastransform );

	if ( transformed_min.z >= ALIAS_Z_CLIP_PLANE )
		zfullyclipped = false;
	if ( transformed_max.z >= ALIAS_Z_CLIP_PLANE )
		zfullyclipped = false;

	if ( zfullyclipped )
	{
		return BBOX_TRIVIAL_REJECT;
	}
	if ( zclipped )
	{
		return ( BBOX_MUST_CLIP_XY | BBOX_MUST_CLIP_Z );
	}

	/*
	** build a transformed bounding box from the given min and max
	*/
	for ( i = 0; i < 8; i++ )
	{
		int      j;
		CVector   tmp, transformed;
		unsigned long clipcode = 0;

		if ( i & 1 )
			tmp.x = mins.x;
		else
			tmp.x = maxs.x;

		if ( i & 2 )
			tmp.y = mins.y;
		else
			tmp.y = maxs.y;

		if ( i & 4 )
			tmp.z = mins.z;
		else
			tmp.z = maxs.z;

		R_AliasTransformVector( tmp, transformed, worldxf );

		for ( j = 0; j < 4; j++ )
		{
			float dp = DotProduct( transformed, view_clipplanes[j].normal );

			if ( ( dp - view_clipplanes[j].dist ) < 0.0F )
				clipcode |= 1 << j;
		}

		aggregate_and_clipcode &= clipcode;
		aggregate_or_clipcode  |= clipcode;
	}

	if ( aggregate_and_clipcode )
	{
		return BBOX_TRIVIAL_REJECT;
	}
	if ( !aggregate_or_clipcode )
	{
		return BBOX_TRIVIAL_ACCEPT;
	}

	return BBOX_MUST_CLIP_XY;
}

qboolean R_AliasCheckBBox (void)
{
	unsigned long ccodes[2] = { 0, 0 };

	if( model_version == ALIAS_VERSION2 ){
		ccodes[0] = R_AliasCheckFrameBBox( (daliasframe_t*) r_thisframe2, aliasworldtransform );
	}
	else{
		ccodes[0] = R_AliasCheckFrameBBox( r_thisframe, aliasworldtransform );
	}

	/*
	** non-lerping model
	*/
	if ( currententity->backlerp == 0 )
	{
		if ( ccodes[0] == BBOX_TRIVIAL_ACCEPT )
			return BBOX_TRIVIAL_ACCEPT;
		else if ( ccodes[0] & BBOX_TRIVIAL_REJECT )
			return BBOX_TRIVIAL_REJECT;
		else
			return ( ccodes[0] & ~BBOX_TRIVIAL_REJECT );
	}

	if( model_version == ALIAS_VERSION2 ){
		ccodes[1] = R_AliasCheckFrameBBox( (daliasframe_t *) r_lastframe2, aliasoldworldtransform );
	}
	else{
		ccodes[1] = R_AliasCheckFrameBBox( r_lastframe, aliasoldworldtransform );
	}

	if ( ( ccodes[0] | ccodes[1] ) == BBOX_TRIVIAL_ACCEPT )
		return BBOX_TRIVIAL_ACCEPT;
	else if ( ( ccodes[0] & ccodes[1] ) & BBOX_TRIVIAL_REJECT )
		return BBOX_TRIVIAL_REJECT;
	else
		return ( ccodes[0] | ccodes[1] ) & ~BBOX_TRIVIAL_REJECT;
}


/*
================
R_AliasTransformVector
================
*/
void R_AliasTransformVector(CVector &in, CVector &out, float xf[3][4] )
{
	out.x = DotProduct(in, xf[0]) + xf[0][3];
	out.y = DotProduct(in, xf[1]) + xf[1][3];
	out.z = DotProduct(in, xf[2]) + xf[2][3];
}

#define SURFANIM_LOOP	0x01

/*
===============
R_AliasAnimateSurface
===============
*/
int R_AliasAnimateSurface (int surface)
{
	int			i;
	uvaentity_t	*entity;

	return 0;
//	return (int)(r_newrefdef.time) % s_psurface[surface].num_uvframes;

	// find the entity
	for (i = 0; i < r_newrefdef.num_uvaentities; i++, r_newrefdef.uvaentities++)
	{
		if (r_newrefdef.uvaentities->key == (int)(currententity))
			entity = r_newrefdef.uvaentities;
	}
		
	// return frame 0 if the entity could not be found
	if (!entity)
		return 0;

	// return 0 if the entity is not animating
	if (entity->current_frame[surface] == -1)
		return 0;

	// increment the frame
	entity->current_frame[surface]++;

	// bounds checking
	if (entity->current_frame[surface] >= s_psurface[surface].num_uvframes)
	{
		if (entity->flags[surface] & SURFANIM_LOOP)
			entity->current_frame[surface] = 0;
		else
			entity->current_frame[surface] = -1;
	}

	// return the frame
	return entity->current_frame[surface];
}

/*
===============
R_AliasSetupSurfaceSkin
===============
*/
void D_Patch (void);
extern "C" uint16 *skin_colormap;
void R_AliasSetupSurfaceSkin (int skinindex)
{
	pskindesc = (image_t*)currententity->skin;

	if ( !pskindesc )
		pskindesc = currentmodel->skins[skinindex];

	if ( !pskindesc )
		pskindesc = currentmodel->skins[0];

	if ( !pskindesc )
		pskindesc = r_notexture_mip;

	r_affinetridesc.pskin = pskindesc->pixels[0];
	r_affinetridesc.skinwidth = pskindesc->width;
	r_affinetridesc.skinheight = pskindesc->height;
	skin_colormap = &pskindesc->palette[0];

	D_Patch();
	
//	R_PolysetUpdateTables ();		// FIXME: precalc edge lookups

}


/*
================
R_AliasSetupSurfaceLighting

  FIXME: put lighting into tables
================
*/
void R_AliasSetupSurfaceLighting (int surface)
{
	alight_t		lighting;
//	float			lightvec[3] = {-1, 0, 0};
	CVector			light;
	int				j;
	float			an;

	// all components of light should be identical in software
	if ( currententity->flags & RF_FULLBRIGHT )
	{
		light.x = 1.0;
		light.y = 1.0;
		light.z = 1.0;
	}
	else
	{
		R_LightPoint (currententity->origin, light);
	}

	// save off light value for server to look at (BIG HACK!)
	if ( currententity->flags & RF_WEAPONMODEL )
		r_lightlevel->value = 150.0 * light.x;

	if ( currententity->flags & RF_MINLIGHT )
	{
		if (light.x < 0.2)
			light.x = 0.2;
		if (light.y < 0.2)
			light.y = 0.2;
		if (light.z < 0.2)
			light.z = 0.2;
	}

//	if ( (s_psurface[surface].flags & SRF_GLOW) || (currententity->flags & RF_GLOW))
	if( currententity->flags & RF_GLOW )
	{	// bonus items will pulse with time
		float	scale;
		float	min;

		scale = 0.1 * sin(r_newrefdef.time*7);
		min = light.x * 0.8;
		light.x += scale;
		if (light.x < min)
			light.x = min;

		min = light.y * 0.8;
		light.y += scale;
		if (light.y < min)
			light.y = min;

		min = light.z * 0.8;
		light.z += scale;
		if (light.z < min)
			light.z = min;
	}

	j = (light.x + light.y + light.z)*0.3333*255;

	lighting.ambientlight = j;
	lighting.shadelight = j;

//	lighting.plightvec = lightvec;

	an = currententity->angles[1]/180*M_PI;
	lighting.plightvec.x = cos(-an);
	lighting.plightvec.y = sin(-an);
	lighting.plightvec.z = 1;
	lighting.plightvec.Normalize();

	// clamp lighting so it doesn't overbright as much
	if (lighting.ambientlight > 128)
		lighting.ambientlight = 128;
	if (lighting.ambientlight + lighting.shadelight > 192)
		lighting.shadelight = 192 - lighting.ambientlight;

// guarantee that no vertex will ever be lit below LIGHT_MIN, so we don't have
// to clamp off the bottom
	r_ambientlight = lighting.ambientlight;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;

	r_ambientlight = (255 - r_ambientlight) << VID_CBITS;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;

	r_shadelight = lighting.shadelight;

	if (r_shadelight < 0)
		r_shadelight = 0;

	r_shadelight *= VID_GRADES;

// rotate the lighting vector into the model's frame of reference
	r_plightvec.x =  DotProduct( lighting.plightvec, s_alias_forward );
	r_plightvec.y = -DotProduct( lighting.plightvec, s_alias_right );
	r_plightvec.z =  DotProduct( lighting.plightvec, s_alias_up );
}

/*
================
R_AliasSetSpanRoutine
================
*/
void R_AliasSetSpanRoutine (int surface)
{

	/*
	** select the proper span routine based on translucency
	*/

	if (r_newrefdef.foginfo.active)
	{
		if( (s_psurface[surface].flags & SRF_TRANS66) )
		{
			d_pdrawspans = R_PolysetDrawSpans8_66f;
		}
		else if( (s_psurface[surface].flags & SRF_TRANS33) )
		{
			d_pdrawspans = R_PolysetDrawSpans8_33f;
		}
		else if( currententity->flags & RF_TRANSLUCENT )
		{
			if( currententity->alpha > 0.66 )
				d_pdrawspans = R_PolysetDrawSpans8_Opaque_f;
			else if( currententity->alpha > 0.33 )
				d_pdrawspans = R_PolysetDrawSpans8_66f;
			else
				d_pdrawspans = R_PolysetDrawSpans8_33f;
		}
		else
			d_pdrawspans = R_PolysetDrawSpans8_Opaque_f;
	}
	else
	{
		if( (s_psurface[surface].flags & SRF_TRANS66) )
		{
			d_pdrawspans = R_PolysetDrawSpans8_66;
		}
		else if( (s_psurface[surface].flags & SRF_TRANS33) )
		{
			d_pdrawspans = R_PolysetDrawSpans8_33;
		}
		else if( currententity->flags & RF_TRANSLUCENT )
		{
			if( currententity->alpha > 0.66 )
				d_pdrawspans = R_PolysetDrawSpans8_Opaque;
			else if( currententity->alpha > 0.33 )
				d_pdrawspans = R_PolysetDrawSpans8_66;
			else
				d_pdrawspans = R_PolysetDrawSpans8_33;
		}
		else
			d_pdrawspans = R_PolysetDrawSpans8_Opaque;
	}
}

/*
================
R_AliasPreparePoints

General clipped case
================
*/
typedef struct
{
	int          num_points;
	dtrivertx_t *last_verts;   // verts from the last frame
	dtrivertx_t *this_verts;   // verts from this frame
	finalvert_t *dest_verts;   // destination for transformed verts
} aliasbatchedtransformdata_t;

typedef struct
{
	int				num_points;
	dtrivertx2_t	*last_verts;   // verts from the last frame
	dtrivertx2_t	*this_verts;   // verts from this frame
	finalvert_t		*dest_verts;   // destination for transformed verts
} aliasbatchedtransformdata2_t;

aliasbatchedtransformdata_t aliasbatchedtransformdata;
aliasbatchedtransformdata2_t aliasbatchedtransformdata2;

void R_AliasPreparePoints (void)
{
	int			s, uvframe;
	int			i;
	dstvert_t	*pstverts;
	dtriangle_t	*ptri;
	finalvert_t	*pfv[3];
	finalvert_t	finalverts[MAXALIASVERTS +
						((CACHE_SIZE - 1) / sizeof(finalvert_t)) + 3];
	finalvert_t	*pfinalverts;

	// put work vertexes on stack, cache aligned
	pfinalverts = (finalvert_t *)
			(((long)&finalverts[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if( model_version == ALIAS_VERSION2 ){
		aliasbatchedtransformdata2.num_points = s_pmdl->num_xyz;
		aliasbatchedtransformdata2.last_verts = r_lastframe2->verts;
		aliasbatchedtransformdata2.this_verts = r_thisframe2->verts;
		aliasbatchedtransformdata2.dest_verts = pfinalverts;

		R_AliasTransformFinalVerts2( aliasbatchedtransformdata2.num_points,
									aliasbatchedtransformdata2.dest_verts,
									aliasbatchedtransformdata2.last_verts,
									aliasbatchedtransformdata2.this_verts );
	}
	else{
		aliasbatchedtransformdata.num_points = s_pmdl->num_xyz;
		aliasbatchedtransformdata.last_verts = r_lastframe->verts;
		aliasbatchedtransformdata.this_verts = r_thisframe->verts;
		aliasbatchedtransformdata.dest_verts = pfinalverts;

		R_AliasTransformFinalVerts( aliasbatchedtransformdata.num_points,
									aliasbatchedtransformdata.dest_verts,
									aliasbatchedtransformdata.last_verts,
									aliasbatchedtransformdata.this_verts );
	}

// clip and draw all triangles
	for (s = 0; s < s_pmdl->num_surfaces; s++)
	{

		R_AliasSetupSurfaceSkin (s_psurface[s].skinindex);

		if (s_psurface[s].flags == SRF_NODRAW)
			continue;

//		uvframe = R_AliasAnimateSurface (s);
		uvframe = 0;

		R_AliasSetSpanRoutine (s);

		pstverts = (dstvert_t *)((byte *)s_pmdl + s_pmdl->ofs_st);
		ptri = (dtriangle_t *)((byte *)s_pmdl + s_pmdl->ofs_tris);

		/*
        if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
		{
			for (i=0 ; i<s_pmdl->num_tris ; i++)//, ptri++)
			{
				if (ptri->index_surface == s)
				{
					pfv[0] = &pfinalverts[ptri->index_xyz[0]];
					pfv[1] = &pfinalverts[ptri->index_xyz[1]];
					pfv[2] = &pfinalverts[ptri->index_xyz[2]];

					if ( pfv[0]->flags & pfv[1]->flags & pfv[2]->flags )
					{
						ptri = (dtriangle_t *) ((byte *) ptri + sizeof (dtriangle_t) +
							((ptri->num_uvframes - 1) * sizeof (dstframe_t)));
						continue;		// completely clipped
					}

					// insert s/t coordinates
					pfv[0]->s = pstverts[ptri->stframes[uvframe].index_st[0]].s << 16;
					pfv[0]->t = pstverts[ptri->stframes[uvframe].index_st[0]].t << 16;

					pfv[1]->s = pstverts[ptri->stframes[uvframe].index_st[1]].s << 16;
					pfv[1]->t = pstverts[ptri->stframes[uvframe].index_st[1]].t << 16;

					pfv[2]->s = pstverts[ptri->stframes[uvframe].index_st[2]].s << 16;
					pfv[2]->t = pstverts[ptri->stframes[uvframe].index_st[2]].t << 16;

					if ( ! (pfv[0]->flags | pfv[1]->flags | pfv[2]->flags) )
					{	// totally unclipped
						aliastriangleparms.a = pfv[2];
						aliastriangleparms.b = pfv[1];
						aliastriangleparms.c = pfv[0];

						R_DrawTriangle();
					}
					else
					{
						R_AliasClipTriangle (pfv[2], pfv[1], pfv[0]);
					}
				}

				ptri = (dtriangle_t *) ((byte *) ptri + sizeof (dtriangle_t) +
					((ptri->num_uvframes - 1) * sizeof (dstframe_t)));
			}
		}
		else
        */
		{
			for (i=0 ; i<s_pmdl->num_tris ; i++)//, ptri++)
			{
				if (ptri->index_surface == s)
				{
					pfv[0] = &pfinalverts[ptri->index_xyz[0]];
					pfv[1] = &pfinalverts[ptri->index_xyz[1]];
					pfv[2] = &pfinalverts[ptri->index_xyz[2]];

					if ( pfv[0]->flags & pfv[1]->flags & pfv[2]->flags )
					{
						ptri = (dtriangle_t *) ((byte *) ptri + sizeof (dtriangle_t) +
							((ptri->num_uvframes - 1) * sizeof (dstframe_t)));
						continue;		// completely clipped
					}

					// insert s/t coordinates
					pfv[0]->s = pstverts[ptri->stframes[uvframe].index_st[0]].s << 16;
					pfv[0]->t = pstverts[ptri->stframes[uvframe].index_st[0]].t << 16;

					pfv[1]->s = pstverts[ptri->stframes[uvframe].index_st[1]].s << 16;
					pfv[1]->t = pstverts[ptri->stframes[uvframe].index_st[1]].t << 16;

					pfv[2]->s = pstverts[ptri->stframes[uvframe].index_st[2]].s << 16;
					pfv[2]->t = pstverts[ptri->stframes[uvframe].index_st[2]].t << 16;

					if ( ! (pfv[0]->flags | pfv[1]->flags | pfv[2]->flags) )
					{	// totally unclipped
						aliastriangleparms.a = pfv[0];
						aliastriangleparms.b = pfv[1];
						aliastriangleparms.c = pfv[2];

						R_DrawTriangle();
					}
					else		
					{	// partially clipped
						R_AliasClipTriangle (pfv[0], pfv[1], pfv[2]);
					}
				}

				ptri = (dtriangle_t *) ((byte *) ptri + sizeof (dtriangle_t) +
					((ptri->num_uvframes - 1) * sizeof (dstframe_t)));
			}
		}
	}
}


/*
================
R_AliasSetUpTransform
================
*/
void R_AliasSetUpTransform (void)
{
	int				i;
	static float	viewmatrix[3][4];
	CVector			angles, scale_ofs;

// TODO: should really be stored with the entity instead of being reconstructed
// TODO: should use a look-up table
// TODO: could cache lazily, stored in the entity
// 
	angles.z = currententity->angles.z;
	angles.x = currententity->angles.x;
	angles.y = currententity->angles.y;
	AngleToVectors( angles, s_alias_forward, s_alias_right, s_alias_up );

// TODO: can do this with simple matrix rearrangement

	memset( aliasworldtransform, 0, sizeof( aliasworldtransform ) );
	memset( aliasoldworldtransform, 0, sizeof( aliasworldtransform ) );

	aliasworldtransform[0][0]	= currententity->render_scale.x;	// shawn
	aliasworldtransform[1][1]	= currententity->render_scale.y;	// shawn
	aliasworldtransform[2][2]	= currententity->render_scale.z;	// shawn

	s_alias_forward[0]			*= currententity->render_scale.x;	// shawn
	s_alias_forward[1]			*= currententity->render_scale.x;	// shawn
	s_alias_forward[2]			*= currententity->render_scale.x;	// shawn

	s_alias_right[0]			*= currententity->render_scale.y;	// shawn
	s_alias_right[1]			*= currententity->render_scale.y;	// shawn
	s_alias_right[2]			*= currententity->render_scale.y;	// shawn

	s_alias_up[0]				*= currententity->render_scale.z;	// shawn
	s_alias_up[1]				*= currententity->render_scale.z;	// shawn
	s_alias_up[2]				*= currententity->render_scale.z;	// shawn

	for (i = 0; i < 3; i++)
	{
//		aliasworldtransform[i][i]	= currententity->render_scale[i];	// shawn
//		s_alias_forward[i]			*= currententity->render_scale[0];	// shawn
//		s_alias_right[i]			*= currententity->render_scale[1];	// shawn
//		s_alias_up[i]				*= currententity->render_scale[2];	// shawn

 		aliasoldworldtransform[i][0] = aliasworldtransform[i][0] =  s_alias_forward[i];
		aliasoldworldtransform[i][0] = aliasworldtransform[i][1] = -s_alias_right[i];
		aliasoldworldtransform[i][0] = aliasworldtransform[i][2] =  s_alias_up[i];
	}

	scale_ofs.x = (currententity->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x;
	scale_ofs.y = (currententity->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y;
	scale_ofs.z = (currententity->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z;

	aliasworldtransform[0][3] = (currententity->origin.x - scale_ofs.x)-r_origin.x;
	aliasworldtransform[1][3] = (currententity->origin.y - scale_ofs.y)-r_origin.y;
	aliasworldtransform[2][3] = (currententity->origin.z - scale_ofs.z)-r_origin.z;

	aliasoldworldtransform[0][3] = (currententity->oldorigin.x - scale_ofs.x)-r_origin.x;
	aliasoldworldtransform[1][3] = (currententity->oldorigin.y - scale_ofs.y)-r_origin.y;
	aliasoldworldtransform[2][3] = (currententity->oldorigin.z - scale_ofs.z)-r_origin.z;

// FIXME: can do more efficiently than full concatenation
//	memcpy( rotationmatrix, t2matrix, sizeof( rotationmatrix ) );

//	R_ConcatTransforms (t2matrix, tmatrix, rotationmatrix);

// TODO: should be global, set when vright, etc., set
/*
	for(i = 0; i < 3; i++ ){
		viewmatrix[0][i] = vright[i];
		viewmatrix[1][i] = -vup[i];
//		VectorInverse (viewmatrix[1]);
		viewmatrix[2][i] = vpn[i];
	}
*/
	viewmatrix[0][0] = vright.x;
	viewmatrix[1][0] = -vup.x;
	viewmatrix[2][0] = vpn.x;

	viewmatrix[0][1] = vright.y;
	viewmatrix[1][1] = -vup.y;
	viewmatrix[2][1] = vpn.y;

	viewmatrix[0][2] = vright.z;
	viewmatrix[1][2] = -vup.z;
	viewmatrix[2][2] = vpn.z;

	viewmatrix[0][3] = 0;
	viewmatrix[1][3] = 0;
	viewmatrix[2][3] = 0;

	R_ConcatTransforms (viewmatrix, aliasworldtransform, aliastransform);

	aliasworldtransform[0][3] = currententity->origin.x;
	aliasworldtransform[1][3] = currententity->origin.y;
	aliasworldtransform[2][3] = currententity->origin.z;

	aliasoldworldtransform[0][3] = currententity->oldorigin.x;
	aliasoldworldtransform[1][3] = currententity->oldorigin.y;
	aliasoldworldtransform[2][3] = currententity->oldorigin.z;
}


/*
================
R_AliasTransformFinalVerts

//  Nelno:	transform the lerped verts into their true worldspace coords

//	Nelno:	we could make a pulsating shell around a model
//	by changing values for POWERSUIT_SCALE to a global scaling
//	factor that is computed once per frame (sine wave).  Might 
//	be a useful effect
================
*/
#if id386
void R_AliasTransformFinalVerts( int numpoints, finalvert_t *fv, dtrivertx_t *oldv, dtrivertx_t *newv )
{
	float  lightcos;
	float	lerped_vert[3];
	int    byte_to_dword_ptr_var;
	int    tmpint;
	float  one = 1.0F;
	float  zi;

	static float  FALIAS_Z_CLIP_PLANE = ALIAS_Z_CLIP_PLANE;
	static float  PS_SCALE = POWERSUIT_SCALE;

#if QCOMMON_ASH_NORMALS
    initVertNormalsArray();
#endif

	__asm mov ecx, numpoints

	/*
	lerped_vert[0] = r_lerp_move[0] + oldv->v[0]*r_lerp_backv[0] + newv->v[0]*r_lerp_frontv[0];
	lerped_vert[1] = r_lerp_move[1] + oldv->v[1]*r_lerp_backv[1] + newv->v[1]*r_lerp_frontv[1];
	lerped_vert[2] = r_lerp_move[2] + oldv->v[2]*r_lerp_backv[2] + newv->v[2]*r_lerp_frontv[2];
	*/
top_of_loop:

	__asm mov esi, oldv
	__asm mov edi, newv

	__asm xor ebx, ebx

	__asm mov bl, byte ptr [esi+DTRIVERTX_V0]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_backv+0]                  ; oldv[0]*rlb[0]

	__asm mov bl, byte ptr [esi+DTRIVERTX_V1]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_backv+4]                  ; oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov bl, byte ptr [esi+DTRIVERTX_V2]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_backv+8]                  ; oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov bl, byte ptr [edi+DTRIVERTX_V0]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_frontv+0]                 ; newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov bl, byte ptr [edi+DTRIVERTX_V1]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_frontv+4]                 ; newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov bl, byte ptr [edi+DTRIVERTX_V2]
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_frontv+8]                 ; newv[2]*rlf[2] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm fxch st(5)                     ; oldv[0]*rlb[0] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	__asm faddp st(2), st                ; newv[1]*rlf[1] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	__asm faddp st(3), st                ; oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	__asm fxch st(1)                     ; oldv[2]*rlb[2] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	__asm faddp st(3), st                ; oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fadd dword ptr [r_lerp_move+0] ; lv0 | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fxch st(1)                     ; oldv[1]*rlb[1] + newv[1]*rlf[1] | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fadd dword ptr [r_lerp_move+4] ; lv1 | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fxch st(2)                     ; oldv[2]*rlb[2] + newv[2]*rlf[2] | lv0 | lv1
	__asm fadd dword ptr [r_lerp_move+8] ; lv2 | lv0 | lv1
	__asm fxch st(1)                     ; lv0 | lv2 | lv1
	__asm fstp dword ptr [lerped_vert+0] ; lv2 | lv1
	__asm fstp dword ptr [lerped_vert+8] ; lv2
	__asm fstp dword ptr [lerped_vert+4] ; (empty)

/*
	__asm mov  eax, currententity
	__asm mov  eax, dword ptr [eax+ENTITY_FLAGS]
	__asm mov  ebx, RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE
	__asm and  eax, ebx
	__asm jz   not_powersuit
*/
	/*
	**    lerped_vert[0] += lightnormal[0] * POWERSUIT_SCALE
	**    lerped_vert[1] += lightnormal[1] * POWERSUIT_SCALE
	**    lerped_vert[2] += lightnormal[2] * POWERSUIT_SCALE
	*/

/*
	__asm xor ebx, ebx
	__asm mov bl,  byte ptr [edi+DTRIVERTX_LNI]
	__asm mov eax, 12
	__asm mul ebx
	__asm lea eax, [avertexnormals+eax]

	__asm fld  dword ptr [eax+0]				; n[0]
	__asm fmul PS_SCALE							; n[0] * PS
	__asm fld  dword ptr [eax+4]				; n[1] | n[0] * PS
	__asm fmul PS_SCALE							; n[1] * PS | n[0] * PS
	__asm fld  dword ptr [eax+8]				; n[2] | n[1] * PS | n[0] * PS
	__asm fmul PS_SCALE							; n[2] * PS | n[1] * PS | n[0] * PS
	__asm fld  dword ptr [lerped_vert+0]		; lv0 | n[2] * PS | n[1] * PS | n[0] * PS
	__asm faddp st(3), st						; n[2] * PS | n[1] * PS | n[0] * PS + lv0
	__asm fld  dword ptr [lerped_vert+4]		; lv1 | n[2] * PS | n[1] * PS | n[0] * PS + lv0
	__asm faddp st(2), st						; n[2] * PS | n[1] * PS + lv1 | n[0] * PS + lv0
	__asm fadd dword ptr [lerped_vert+8]		; n[2] * PS + lv2 | n[1] * PS + lv1 | n[0] * PS + lv0
	__asm fxch st(2)							; LV0 | LV1 | LV2
	__asm fstp dword ptr [lerped_vert+0]		; LV1 | LV2
	__asm fstp dword ptr [lerped_vert+4]		; LV2
	__asm fstp dword ptr [lerped_vert+8]		; (empty)
*/

//not_powersuit:

	/*
	fv->flags = 0;

	fv->xyz[0] = DotProduct(lerped_vert, aliastransform[0]) + aliastransform[0][3];
	fv->xyz[1] = DotProduct(lerped_vert, aliastransform[1]) + aliastransform[1][3];
	fv->xyz[2] = DotProduct(lerped_vert, aliastransform[2]) + aliastransform[2][3];
	*/
	__asm mov  eax, fv
	__asm mov  dword ptr [eax+FINALVERT_FLAGS], 0

	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+0]        ; lv0*at[0][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[0][0]
	__asm fmul dword ptr [aliastransform+4]        ; lv1*at[0][1] | lv0*at[0][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[0][1] | lv0*at[0][0]
	__asm fmul dword ptr [aliastransform+8]        ; lv2*at[0][2] | lv1*at[0][1] | lv0*at[0][0]
	__asm fxch st(2)                               ; lv0*at[0][0] | lv1*at[0][1] | lv2*at[0][2]
	__asm faddp st(1), st                          ; lv0*at[0][0] + lv1*at[0][1] | lv2*at[0][2]
	__asm faddp st(1), st                          ; lv0*at[0][0] + lv1*at[0][1] + lv2*at[0][2]
	__asm fadd  dword ptr [aliastransform+12]      ; FV.X

	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+16]       ; lv0*at[1][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[1][0]
	__asm fmul dword ptr [aliastransform+20]       ; lv1*at[1][1] | lv0*at[1][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[1][1] | lv0*at[1][0]
	__asm fmul dword ptr [aliastransform+24]       ; lv2*at[1][2] | lv1*at[1][1] | lv0*at[1][0]
	__asm fxch st(2)                               ; lv0*at[1][0] | lv1*at[1][1] | lv2*at[1][2]
	__asm faddp st(1), st                          ; lv0*at[1][0] + lv1*at[1][1] | lv2*at[1][2]
	__asm faddp st(1), st                          ; lv0*at[1][0] + lv1*at[1][1] + lv2*at[1][2]
	__asm fadd dword ptr [aliastransform+28]       ; FV.Y | FV.X
	__asm fxch st(1)                               ; FV.X | FV.Y
	__asm fstp  dword ptr [eax+FINALVERT_X]        ; FV.Y
	
	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+32]       ; lv0*at[2][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[2][0]
	__asm fmul dword ptr [aliastransform+36]       ; lv1*at[2][1] | lv0*at[2][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[2][1] | lv0*at[2][0]
	__asm fmul dword ptr [aliastransform+40]       ; lv2*at[2][2] | lv1*at[2][1] | lv0*at[2][0]
	__asm fxch st(2)                               ; lv0*at[2][0] | lv1*at[2][1] | lv2*at[2][2]
	__asm faddp st(1), st                          ; lv0*at[2][0] + lv1*at[2][1] | lv2*at[2][2]
	__asm faddp st(1), st                          ; lv0*at[2][0] + lv1*at[2][1] + lv2*at[2][2]
	__asm fadd dword ptr [aliastransform+44]       ; FV.Z | FV.Y
	__asm fxch st(1)                               ; FV.Y | FV.Z
	__asm fstp dword ptr [eax+FINALVERT_Y]         ; FV.Z
	__asm fstp dword ptr [eax+FINALVERT_Z]         ; (empty)

	/*
	**  lighting
	**
	**  plightnormal = avertexnormals[newv->lightnormalindex];
	**	lightcos = DotProduct (plightnormal, r_plightvec);
	**	temp = r_ambientlight;
	*/
	__asm xor ebx, ebx
	__asm mov bl,  byte ptr [edi+DTRIVERTX_LNI]
	__asm mov eax, 12
	__asm mul ebx
	__asm lea eax, [avertexnormals+eax]
	__asm lea ebx, r_plightvec

	__asm fld  dword ptr [eax+0]
	__asm fmul dword ptr [ebx+0]
	__asm fld  dword ptr [eax+4]
	__asm fmul dword ptr [ebx+4]
	__asm fld  dword ptr [eax+8]
	__asm fmul dword ptr [ebx+8]
	__asm fxch st(2)
	__asm faddp st(1), st
	__asm faddp st(1), st
	__asm fstp dword ptr lightcos
	__asm mov eax, lightcos
	__asm mov ebx, r_ambientlight

	/*
	if (lightcos < 0)
	{
		temp += (int)(r_shadelight * lightcos);

		// clamp; because we limited the minimum ambient and shading light, we
		// don't have to clamp low light, just bright
		if (temp < 0)
			temp = 0;
	}

	fv->v[4] = temp;
	*/
	__asm or  eax, eax
	__asm jns store_fv4

	__asm fld   dword ptr r_shadelight
	__asm fmul  dword ptr lightcos
	__asm fistp dword ptr tmpint
	__asm add   ebx, tmpint

	__asm or    ebx, ebx
	__asm jns   store_fv4
	__asm mov   ebx, 0

store_fv4:
	__asm mov edi, fv
	__asm mov dword ptr [edi+FINALVERT_V4], ebx

	__asm mov edx, dword ptr [edi+FINALVERT_FLAGS]

	/*
	** do clip testing and projection here
	*/
	/*
	if ( dest_vert->xyz[2] < ALIAS_Z_CLIP_PLANE )
	{
		dest_vert->flags |= ALIAS_Z_CLIP;
	}
	else
	{
		R_AliasProjectAndClipTestFinalVert( dest_vert );
	}
	*/
	__asm mov eax, dword ptr [edi+FINALVERT_Z]
	__asm and eax, eax
	__asm js  alias_z_clip
	__asm cmp eax, FALIAS_Z_CLIP_PLANE
	__asm jl  alias_z_clip

	/*
	This is the code to R_AliasProjectAndClipTestFinalVert

	float	zi;
	float	x, y, z;

	x = fv->xyz[0];
	y = fv->xyz[1];
	z = fv->xyz[2];
	zi = 1.0 / z;

	fv->v[5] = zi * s_ziscale;

	fv->v[0] = (x * aliasxscale * zi) + aliasxcenter;
	fv->v[1] = (y * aliasyscale * zi) + aliasycenter;
	*/
	__asm fld   one                             ; 1
	__asm fdiv  dword ptr [edi+FINALVERT_Z]     ; zi

	__asm mov   eax, dword ptr [edi+32]
	__asm mov   eax, dword ptr [edi+64]

	__asm fst   zi                              ; zi
	__asm fmul  s_ziscale                       ; fv5
	__asm fld   dword ptr [edi+FINALVERT_X]     ; x | fv5
	__asm fmul  aliasxscale                     ; x * aliasxscale | fv5
	__asm fld   dword ptr [edi+FINALVERT_Y]     ; y | x * aliasxscale | fv5
	__asm fmul  aliasyscale                     ; y * aliasyscale | x * aliasxscale | fv5
	__asm fxch  st(1)                           ; x * aliasxscale | y * aliasyscale | fv5
	__asm fmul  zi                              ; x * asx * zi | y * asy | fv5
	__asm fadd  aliasxcenter                    ; fv0 | y * asy | fv5
	__asm fxch  st(1)                           ; y * asy | fv0 | fv5
	__asm fmul  zi                              ; y * asy * zi | fv0 | fv5
	__asm fadd  aliasycenter                    ; fv1 | fv0 | fv5
	__asm fxch  st(2)                           ; fv5 | fv0 | fv1
	__asm fistp dword ptr [edi+FINALVERT_V5]    ; fv0 | fv1
	__asm fistp dword ptr [edi+FINALVERT_V0]    ; fv1
	__asm fistp dword ptr [edi+FINALVERT_V1]    ; (empty)

	/*
	if (fv->v[0] < r_refdef.aliasvrect.x)
		fv->flags |= ALIAS_LEFT_CLIP;
	if (fv->v[1] < r_refdef.aliasvrect.y)
		fv->flags |= ALIAS_TOP_CLIP;
	if (fv->v[0] > r_refdef.aliasvrectright)
		fv->flags |= ALIAS_RIGHT_CLIP;
	if (fv->v[1] > r_refdef.aliasvrectbottom)
		fv->flags |= ALIAS_BOTTOM_CLIP;
	*/
	__asm mov eax, dword ptr [edi+FINALVERT_V0]
	__asm mov ebx, dword ptr [edi+FINALVERT_V1]

	__asm cmp eax, r_refdef.aliasvrect.x
	__asm jge ct_alias_top
	__asm or  edx, ALIAS_LEFT_CLIP
ct_alias_top:
	__asm cmp ebx, r_refdef.aliasvrect.y
	__asm jge ct_alias_right
	__asm or edx, ALIAS_TOP_CLIP
ct_alias_right:
	__asm cmp eax, r_refdef.aliasvrectright
	__asm jle ct_alias_bottom
	__asm or edx, ALIAS_RIGHT_CLIP
ct_alias_bottom:
	__asm cmp ebx, r_refdef.aliasvrectbottom
	__asm jle end_of_loop
	__asm or  edx, ALIAS_BOTTOM_CLIP

	__asm jmp end_of_loop

alias_z_clip:
	__asm or  edx, ALIAS_Z_CLIP

end_of_loop:

	__asm mov dword ptr [edi+FINALVERT_FLAGS], edx
	__asm add oldv, DTRIVERTX_SIZE
	__asm add newv, DTRIVERTX_SIZE
	__asm add fv, FINALVERT_SIZE

	__asm dec ecx
	__asm jnz top_of_loop
}
#endif

void R_AliasTransformFinalVerts2( int numpoints, finalvert_t *fv, dtrivertx2_t *oldv, dtrivertx2_t *newv )
{
	float	lightcos;
	float	lerped_vert[3];
	int		byte_to_dword_ptr_var;
	int		tmpint;
	float	one = 1.0F;
	float	zi;

	static float  FALIAS_Z_CLIP_PLANE = ALIAS_Z_CLIP_PLANE;
	static float  PS_SCALE = POWERSUIT_SCALE;

#if QCOMMON_ASH_NORMALS
    initVertNormalsArray();
#endif

	__asm mov ecx, numpoints

	/*
	lerped_vert[0] = r_lerp_move[0] + oldv->v[0]*r_lerp_backv[0] + newv->v[0]*r_lerp_frontv[0];
	lerped_vert[1] = r_lerp_move[1] + oldv->v[1]*r_lerp_backv[1] + newv->v[1]*r_lerp_frontv[1];
	lerped_vert[2] = r_lerp_move[2] + oldv->v[2]*r_lerp_backv[2] + newv->v[2]*r_lerp_frontv[2];
	*/
top_of_loop:

	__asm mov esi, oldv
	__asm mov edi, newv

	__asm xor ebx, ebx

	__asm mov ebx, dword ptr [esi+DTRIVERTX2_V0]
	__asm shr ebx, X_BIT_SHIFT
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_backv+0]                  ; oldv[0]*rlb[0]

	__asm mov ebx, dword ptr [esi+DTRIVERTX2_V1]
	__asm and ebx, Y_BIT_MASK
	__asm shr ebx, Y_BIT_SHIFT
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_backv+4]                  ; oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov ebx, dword ptr [esi+DTRIVERTX2_V2]
	__asm and ebx, Z_BIT_MASK
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_backv+8]                  ; oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov ebx, dword ptr [edi+DTRIVERTX2_V0]
	__asm shr ebx, X_BIT_SHIFT
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_frontv+0]                 ; newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov ebx, dword ptr [edi+DTRIVERTX2_V1]
	__asm and ebx, Y_BIT_MASK
	__asm shr ebx, Y_BIT_SHIFT
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var      
	__asm fmul dword ptr [r_lerp_frontv+4]                 ; newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm mov ebx, dword ptr [edi+DTRIVERTX2_V2]
	__asm and ebx, Z_BIT_MASK
	__asm mov byte_to_dword_ptr_var, ebx
	__asm fild dword ptr byte_to_dword_ptr_var
	__asm fmul dword ptr [r_lerp_frontv+8]                 ; newv[2]*rlf[2] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	__asm fxch st(5)                     ; oldv[0]*rlb[0] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	__asm faddp st(2), st                ; newv[1]*rlf[1] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	__asm faddp st(3), st                ; oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	__asm fxch st(1)                     ; oldv[2]*rlb[2] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	__asm faddp st(3), st                ; oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fadd dword ptr [r_lerp_move+0] ; lv0 | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fxch st(1)                     ; oldv[1]*rlb[1] + newv[1]*rlf[1] | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fadd dword ptr [r_lerp_move+4] ; lv1 | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	__asm fxch st(2)                     ; oldv[2]*rlb[2] + newv[2]*rlf[2] | lv0 | lv1
	__asm fadd dword ptr [r_lerp_move+8] ; lv2 | lv0 | lv1
	__asm fxch st(1)                     ; lv0 | lv2 | lv1
	__asm fstp dword ptr [lerped_vert+0] ; lv2 | lv1
	__asm fstp dword ptr [lerped_vert+8] ; lv2
	__asm fstp dword ptr [lerped_vert+4] ; (empty)

/*
	__asm mov  eax, currententity
	__asm mov  eax, dword ptr [eax+ENTITY_FLAGS]
	__asm mov  ebx, RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE
	__asm and  eax, ebx
	__asm jz   not_powersuit
*/
	/*
	**    lerped_vert[0] += lightnormal[0] * POWERSUIT_SCALE
	**    lerped_vert[1] += lightnormal[1] * POWERSUIT_SCALE
	**    lerped_vert[2] += lightnormal[2] * POWERSUIT_SCALE
	*/

/*
	__asm xor ebx, ebx
	__asm mov bl,  byte ptr [edi+DTRIVERTX2_LNI]
	__asm mov eax, 12
	__asm mul ebx
	__asm lea eax, [avertexnormals+eax]

	__asm fld  dword ptr [eax+0]				; n[0]
	__asm fmul PS_SCALE							; n[0] * PS
	__asm fld  dword ptr [eax+4]				; n[1] | n[0] * PS
	__asm fmul PS_SCALE							; n[1] * PS | n[0] * PS
	__asm fld  dword ptr [eax+8]				; n[2] | n[1] * PS | n[0] * PS
	__asm fmul PS_SCALE							; n[2] * PS | n[1] * PS | n[0] * PS
	__asm fld  dword ptr [lerped_vert+0]		; lv0 | n[2] * PS | n[1] * PS | n[0] * PS
	__asm faddp st(3), st						; n[2] * PS | n[1] * PS | n[0] * PS + lv0
	__asm fld  dword ptr [lerped_vert+4]		; lv1 | n[2] * PS | n[1] * PS | n[0] * PS + lv0
	__asm faddp st(2), st						; n[2] * PS | n[1] * PS + lv1 | n[0] * PS + lv0
	__asm fadd dword ptr [lerped_vert+8]		; n[2] * PS + lv2 | n[1] * PS + lv1 | n[0] * PS + lv0
	__asm fxch st(2)							; LV0 | LV1 | LV2
	__asm fstp dword ptr [lerped_vert+0]		; LV1 | LV2
	__asm fstp dword ptr [lerped_vert+4]		; LV2
	__asm fstp dword ptr [lerped_vert+8]		; (empty)
*/

//not_powersuit:

	/*
	fv->flags = 0;

	fv->xyz[0] = DotProduct(lerped_vert, aliastransform[0]) + aliastransform[0][3];
	fv->xyz[1] = DotProduct(lerped_vert, aliastransform[1]) + aliastransform[1][3];
	fv->xyz[2] = DotProduct(lerped_vert, aliastransform[2]) + aliastransform[2][3];
	*/
	__asm mov  eax, fv
	__asm mov  dword ptr [eax+FINALVERT_FLAGS], 0

	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+0]        ; lv0*at[0][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[0][0]
	__asm fmul dword ptr [aliastransform+4]        ; lv1*at[0][1] | lv0*at[0][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[0][1] | lv0*at[0][0]
	__asm fmul dword ptr [aliastransform+8]        ; lv2*at[0][2] | lv1*at[0][1] | lv0*at[0][0]
	__asm fxch st(2)                               ; lv0*at[0][0] | lv1*at[0][1] | lv2*at[0][2]
	__asm faddp st(1), st                          ; lv0*at[0][0] + lv1*at[0][1] | lv2*at[0][2]
	__asm faddp st(1), st                          ; lv0*at[0][0] + lv1*at[0][1] + lv2*at[0][2]
	__asm fadd  dword ptr [aliastransform+12]      ; FV.X

	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+16]       ; lv0*at[1][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[1][0]
	__asm fmul dword ptr [aliastransform+20]       ; lv1*at[1][1] | lv0*at[1][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[1][1] | lv0*at[1][0]
	__asm fmul dword ptr [aliastransform+24]       ; lv2*at[1][2] | lv1*at[1][1] | lv0*at[1][0]
	__asm fxch st(2)                               ; lv0*at[1][0] | lv1*at[1][1] | lv2*at[1][2]
	__asm faddp st(1), st                          ; lv0*at[1][0] + lv1*at[1][1] | lv2*at[1][2]
	__asm faddp st(1), st                          ; lv0*at[1][0] + lv1*at[1][1] + lv2*at[1][2]
	__asm fadd dword ptr [aliastransform+28]       ; FV.Y | FV.X
	__asm fxch st(1)                               ; FV.X | FV.Y
	__asm fstp  dword ptr [eax+FINALVERT_X]        ; FV.Y
	
	__asm fld  dword ptr [lerped_vert+0]           ; lv0
	__asm fmul dword ptr [aliastransform+32]       ; lv0*at[2][0]
	__asm fld  dword ptr [lerped_vert+4]           ; lv1 | lv0*at[2][0]
	__asm fmul dword ptr [aliastransform+36]       ; lv1*at[2][1] | lv0*at[2][0]
	__asm fld  dword ptr [lerped_vert+8]           ; lv2 | lv1*at[2][1] | lv0*at[2][0]
	__asm fmul dword ptr [aliastransform+40]       ; lv2*at[2][2] | lv1*at[2][1] | lv0*at[2][0]
	__asm fxch st(2)                               ; lv0*at[2][0] | lv1*at[2][1] | lv2*at[2][2]
	__asm faddp st(1), st                          ; lv0*at[2][0] + lv1*at[2][1] | lv2*at[2][2]
	__asm faddp st(1), st                          ; lv0*at[2][0] + lv1*at[2][1] + lv2*at[2][2]
	__asm fadd dword ptr [aliastransform+44]       ; FV.Z | FV.Y
	__asm fxch st(1)                               ; FV.Y | FV.Z
	__asm fstp dword ptr [eax+FINALVERT_Y]         ; FV.Z
	__asm fstp dword ptr [eax+FINALVERT_Z]         ; (empty)

	/*
	**  lighting
	**
	**  plightnormal = avertexnormals[newv->lightnormalindex];
	**	lightcos = DotProduct (plightnormal, r_plightvec);
	**	temp = r_ambientlight;
	*/
	__asm xor ebx, ebx
	__asm mov bl,  byte ptr [edi+DTRIVERTX2_LNI]
	__asm mov eax, 12
	__asm mul ebx
	__asm lea eax, [avertexnormals+eax]
	__asm lea ebx, r_plightvec

	__asm fld  dword ptr [eax+0]
	__asm fmul dword ptr [ebx+0]
	__asm fld  dword ptr [eax+4]
	__asm fmul dword ptr [ebx+4]
	__asm fld  dword ptr [eax+8]
	__asm fmul dword ptr [ebx+8]
	__asm fxch st(2)
	__asm faddp st(1), st
	__asm faddp st(1), st
	__asm fstp dword ptr lightcos
	__asm mov eax, lightcos
	__asm mov ebx, r_ambientlight

	/*
	if (lightcos < 0)
	{
		temp += (int)(r_shadelight * lightcos);

		// clamp; because we limited the minimum ambient and shading light, we
		// don't have to clamp low light, just bright
		if (temp < 0)
			temp = 0;
	}

	fv->v[4] = temp;
	*/
	__asm or  eax, eax
	__asm jns store_fv4

	__asm fld   dword ptr r_shadelight
	__asm fmul  dword ptr lightcos
	__asm fistp dword ptr tmpint
	__asm add   ebx, tmpint

	__asm or    ebx, ebx
	__asm jns   store_fv4
	__asm mov   ebx, 0

store_fv4:
	__asm mov edi, fv
	__asm mov dword ptr [edi+FINALVERT_V4], ebx

	__asm mov edx, dword ptr [edi+FINALVERT_FLAGS]

	/*
	** do clip testing and projection here
	*/
	/*
	if ( dest_vert->xyz[2] < ALIAS_Z_CLIP_PLANE )
	{
		dest_vert->flags |= ALIAS_Z_CLIP;
	}
	else
	{
		R_AliasProjectAndClipTestFinalVert( dest_vert );
	}
	*/
	__asm mov eax, dword ptr [edi+FINALVERT_Z]
	__asm and eax, eax
	__asm js  alias_z_clip
	__asm cmp eax, FALIAS_Z_CLIP_PLANE
	__asm jl  alias_z_clip

	/*
	This is the code to R_AliasProjectAndClipTestFinalVert

	float	zi;
	float	x, y, z;

	x = fv->xyz[0];
	y = fv->xyz[1];
	z = fv->xyz[2];
	zi = 1.0 / z;

	fv->v[5] = zi * s_ziscale;

	fv->v[0] = (x * aliasxscale * zi) + aliasxcenter;
	fv->v[1] = (y * aliasyscale * zi) + aliasycenter;
	*/
	__asm fld   one                             ; 1
	__asm fdiv  dword ptr [edi+FINALVERT_Z]     ; zi

	__asm mov   eax, dword ptr [edi+32]
	__asm mov   eax, dword ptr [edi+64]

	__asm fst   zi                              ; zi
	__asm fmul  s_ziscale                       ; fv5
	__asm fld   dword ptr [edi+FINALVERT_X]     ; x | fv5
	__asm fmul  aliasxscale                     ; x * aliasxscale | fv5
	__asm fld   dword ptr [edi+FINALVERT_Y]     ; y | x * aliasxscale | fv5
	__asm fmul  aliasyscale                     ; y * aliasyscale | x * aliasxscale | fv5
	__asm fxch  st(1)                           ; x * aliasxscale | y * aliasyscale | fv5
	__asm fmul  zi                              ; x * asx * zi | y * asy | fv5
	__asm fadd  aliasxcenter                    ; fv0 | y * asy | fv5
	__asm fxch  st(1)                           ; y * asy | fv0 | fv5
	__asm fmul  zi                              ; y * asy * zi | fv0 | fv5
	__asm fadd  aliasycenter                    ; fv1 | fv0 | fv5
	__asm fxch  st(2)                           ; fv5 | fv0 | fv1
	__asm fistp dword ptr [edi+FINALVERT_V5]    ; fv0 | fv1
	__asm fistp dword ptr [edi+FINALVERT_V0]    ; fv1
	__asm fistp dword ptr [edi+FINALVERT_V1]    ; (empty)

	/*
	if (fv->v[0] < r_refdef.aliasvrect.x)
		fv->flags |= ALIAS_LEFT_CLIP;
	if (fv->v[1] < r_refdef.aliasvrect.y)
		fv->flags |= ALIAS_TOP_CLIP;
	if (fv->v[0] > r_refdef.aliasvrectright)
		fv->flags |= ALIAS_RIGHT_CLIP;
	if (fv->v[1] > r_refdef.aliasvrectbottom)
		fv->flags |= ALIAS_BOTTOM_CLIP;
	*/
	__asm mov eax, dword ptr [edi+FINALVERT_V0]
	__asm mov ebx, dword ptr [edi+FINALVERT_V1]

	__asm cmp eax, r_refdef.aliasvrect.x
	__asm jge ct_alias_top
	__asm or  edx, ALIAS_LEFT_CLIP
ct_alias_top:
	__asm cmp ebx, r_refdef.aliasvrect.y
	__asm jge ct_alias_right
	__asm or edx, ALIAS_TOP_CLIP
ct_alias_right:
	__asm cmp eax, r_refdef.aliasvrectright
	__asm jle ct_alias_bottom
	__asm or edx, ALIAS_RIGHT_CLIP
ct_alias_bottom:
	__asm cmp ebx, r_refdef.aliasvrectbottom
	__asm jle end_of_loop
	__asm or  edx, ALIAS_BOTTOM_CLIP

	__asm jmp end_of_loop

alias_z_clip:
	__asm or  edx, ALIAS_Z_CLIP

end_of_loop:

	__asm mov dword ptr [edi+FINALVERT_FLAGS], edx
	__asm add oldv, DTRIVERTX2_SIZE
	__asm add newv, DTRIVERTX2_SIZE
	__asm add fv, FINALVERT_SIZE

	__asm dec ecx
	__asm jnz top_of_loop
}

/*
================
R_AliasProjectAndClipTestFinalVert
================
*/
void R_AliasProjectAndClipTestFinalVert( finalvert_t *fv )
{
	float	zi;
	float	x, y, z;

	// project points
	x = fv->xyz[0];
	y = fv->xyz[1];
	z = fv->xyz[2];
	zi = 1.0 / z;

	fv->zi = zi * s_ziscale;

	fv->u = (x * aliasxscale * zi) + aliasxcenter;
	fv->v = (y * aliasyscale * zi) + aliasycenter;

	if (fv->u < r_refdef.aliasvrect.x)
		fv->flags |= ALIAS_LEFT_CLIP;
	if (fv->v < r_refdef.aliasvrect.y)
		fv->flags |= ALIAS_TOP_CLIP;
	if (fv->u > r_refdef.aliasvrectright)
		fv->flags |= ALIAS_RIGHT_CLIP;
	if (fv->v > r_refdef.aliasvrectbottom)
		fv->flags |= ALIAS_BOTTOM_CLIP;	
}

/*
=================
R_AliasSetupFrames

=================
*/
void R_AliasSetupFrames( dmdl_t *pmdl )
{
	int thisframe = currententity->frame;
	int lastframe = currententity->oldframe;

	if ( ( thisframe >= pmdl->num_frames ) || ( thisframe < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_AliasSetupFrames: no such thisframe %d\n", thisframe);
		thisframe = 0;
	}
	if ( ( lastframe >= pmdl->num_frames ) || ( lastframe < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_AliasSetupFrames: no such lastframe %d\n", lastframe);
		lastframe = 0;
	}

	if( model_version == ALIAS_VERSION2 ){
		r_thisframe2 = (daliasframe2_t *)((byte *)pmdl + pmdl->ofs_frames 
			+ thisframe * pmdl->framesize);

		r_lastframe2 = (daliasframe2_t *)((byte *)pmdl + pmdl->ofs_frames 
			+ lastframe * pmdl->framesize);
	}
	else{
		r_thisframe = (daliasframe_t *)((byte *)pmdl + pmdl->ofs_frames 
			+ thisframe * pmdl->framesize);

		r_lastframe = (daliasframe_t *)((byte *)pmdl + pmdl->ofs_frames 
			+ lastframe * pmdl->framesize);
	}
}

/*
** R_AliasSetUpLerpData
**
** Precomputes lerp coefficients used for the whole frame.
*/
void R_AliasSetUpLerpData( dmdl_t *pmdl, float backlerp )
{
	float	frontlerp;
	CVector	translation, vectors[3];
//	int		i;

	frontlerp = 1.0F - backlerp;

	/*
	** convert entity's angles into discrete vectors for R, U, and F
	*/
	AngleToVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);

	/*
	** translation is the vector from last position to this position
	*/
	translation = currententity->oldorigin - currententity->origin;

	/*
	** move should be the delta back to the previous frame * backlerp
	*/

	r_lerp_move.x =  DotProduct(translation, vectors[0]);	// forward
	r_lerp_move.y = -DotProduct(translation, vectors[1]);	// left
	r_lerp_move.z =  DotProduct(translation, vectors[2]);	// up

	if( model_version == ALIAS_VERSION2 ){
		r_lerp_move = r_lerp_move + r_lastframe2->translate;

/*
		for (i=0 ; i<3 ; i++){
			r_lerp_move[i] = backlerp * r_lerp_move[i] + frontlerp * r_thisframe2->translate[i];
		}

		for (i=0 ; i<3 ; i++){
			r_lerp_frontv[i] = frontlerp * r_thisframe2->scale[i];
			r_lerp_backv[i]  = backlerp  * r_lastframe2->scale[i];
		}
*/
		r_lerp_move.x = backlerp * r_lerp_move.x + frontlerp * r_thisframe2->translate.x;
		r_lerp_move.y = backlerp * r_lerp_move.y + frontlerp * r_thisframe2->translate.y;
		r_lerp_move.z = backlerp * r_lerp_move.z + frontlerp * r_thisframe2->translate.z;

		r_lerp_frontv.x = frontlerp * r_thisframe2->scale.x;
		r_lerp_backv.x  = backlerp  * r_lastframe2->scale.x;
		r_lerp_frontv.y = frontlerp * r_thisframe2->scale.y;
		r_lerp_backv.y  = backlerp  * r_lastframe2->scale.y;
		r_lerp_frontv.z = frontlerp * r_thisframe2->scale.z;
		r_lerp_backv.z  = backlerp  * r_lastframe2->scale.z;
	}
	else{
		r_lerp_move = r_lerp_move + r_lastframe->translate;

/*
		for (i=0 ; i<3 ; i++){
			r_lerp_move[i] = backlerp*r_lerp_move[i] + frontlerp * r_thisframe->translate[i];
		}

		for (i=0 ; i<3 ; i++){
			r_lerp_frontv[i] = frontlerp * r_thisframe->scale[i];
			r_lerp_backv[i]  = backlerp  * r_lastframe->scale[i];
		}
*/
		r_lerp_move.x = backlerp * r_lerp_move.x + frontlerp * r_thisframe->translate.x;
		r_lerp_move.y = backlerp * r_lerp_move.y + frontlerp * r_thisframe->translate.y;
		r_lerp_move.z = backlerp * r_lerp_move.z + frontlerp * r_thisframe->translate.z;

		r_lerp_frontv.x = frontlerp * r_thisframe->scale.x;
		r_lerp_backv.x  = backlerp  * r_lastframe->scale.x;
		r_lerp_frontv.y = frontlerp * r_thisframe->scale.y;
		r_lerp_backv.y  = backlerp  * r_lastframe->scale.y;
		r_lerp_frontv.z = frontlerp * r_thisframe->scale.z;
		r_lerp_backv.z  = backlerp  * r_lastframe->scale.z;
	}


/*
	for (i=0 ; i<3 ; i++)
	{
		r_lerp_move[i] = backlerp*r_lerp_move[i] + frontlerp * r_thisframe->translate[i];
	}

	for (i=0 ; i<3 ; i++)
	{
		r_lerp_frontv[i] = frontlerp * r_thisframe->scale[i];
		r_lerp_backv[i]  = backlerp  * r_lastframe->scale[i];
	}
*/
}

/*
================
R_AliasDrawModel
================
*/
void R_AliasDrawModel (void)
{
	s_pmdl = (dmdl_t *)currentmodel->extradata;
	s_psurface = (dsurface_t *) ((byte *) s_pmdl + s_pmdl->ofs_surfaces);
	
	if (!s_psurface)
		return;

	if ( r_lerpmodels->value == 0 )
		currententity->backlerp = 0;

	if ( currententity->flags & RF_WEAPONMODEL )
	{
        currententity->render_scale[0]=currententity->render_scale[1]=currententity->render_scale[2]=1;
		
        //if ( r_lefthand->value == 1.0F )
		//	aliasxscale = -aliasxscale;
		//else if ( r_lefthand->value == 2.0F )
			return;
	}

	/*
	** we have to set our frame pointers and transformations before
	** doing any real work
	*/
	model_version = LittleLong (s_pmdl->version);

	R_AliasSetupSurfaceLighting (0);

	R_AliasSetupFrames( s_pmdl );

	R_AliasSetUpTransform();

	// see if the bounding box lets us trivially reject, also sets
	// trivial accept status

	if ( R_AliasCheckBBox() == BBOX_TRIVIAL_REJECT )
	{
		/*
        if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
		{
			aliasxscale = -aliasxscale;
		}
        */
		return;
	}

	r_amodels_drawn++;

	/*
	** compute this_frame and old_frame addresses
	*/
	R_AliasSetUpLerpData( s_pmdl, currententity->backlerp );

	//	Nelno:	keep weapon models from clipping into walls...
	if (currententity->flags & RF_DEPTHHACK)
		s_ziscale = (float)0x8000 * (float)0x10000 * 3.0;
	else
		s_ziscale = (float)0x8000 * (float)0x10000;

	R_AliasPreparePoints ();

	
    /*
    if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
	{
		aliasxscale = -aliasxscale;
	}
    */
}

void R_DrawAliasIcon (entity_t *ent)
{
	currentmodel = (model_t*)ent->model;
	currententity = ent;
	s_pmdl = (dmdl_t *)currentmodel->extradata;
	s_psurface = (dsurface_t *) ((byte *) s_pmdl + s_pmdl->ofs_surfaces);

	if (!s_psurface)
		return;

	if ( r_lerpmodels->value == 0 )
		currententity->backlerp = 0;

	/*
	** we have to set our frame pointers and transformations before
	** doing any real work
	*/
	model_version = LittleLong (s_pmdl->version);

	R_AliasSetupSurfaceLighting (0);

	R_AliasSetupFrames( s_pmdl );

	R_AliasSetUpTransform();

	r_amodels_drawn++;

	/*
	** compute this_frame and old_frame addresses
	*/
	R_AliasSetUpLerpData( s_pmdl, currententity->backlerp );

	R_AliasPreparePoints ();
}

