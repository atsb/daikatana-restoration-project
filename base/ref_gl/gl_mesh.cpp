// gl_mesh.c: triangle model functions

#include "gl_local.h"
//#include "gl_protex.h"
#include "..\shared\alias_quant.h"
#include "gl_surfsprite.h"

//	prototype.  defined in gl_rmain.c
//int	R_GetProjectedVert (const CVector &org, int *x, int *y);
void GL_DrawAliasFrameLerpProcShell (entity_t *ent, dmdl_t *paliashdr);
void R_BuildHeadMatrix (entity_t *e, float *pMatrix);

/*
=============================================================

  ALIAS MODELS


=============================================================
*/

typedef float vec4_t[4];

static vec4_t	s_lerped[MAX_VERTS];

//static CVector	lerped[MAX_VERTS];

CVector	shadevector;
CVector	shadelight;
CVector	oldshadelight;
qboolean	shademodified = false;
image_t *pEnvMapSkin;
image_t	*r_shadowtexture;
msurface_t pointSurface;
CVector model_cam_pos;

dmdl_t			*s_pmdl;
dsurface_t		*s_psurface;

#include "vertnormals.h"
static CVertexNormals avertexnormals;

// precalculated dot products
float	avertexnormaldots[NUMVERTEXNORMALS];

CVector avertexcolors[NUMVERTEXNORMALS];

float   gHeadMatrix[16];
bool    gfHaveHeadMatrix;

qboolean IsPlayerModel()
{
	char *ptr = &currentmodel->name[14];

	// SCG[7/14/00]: This sucks.  Basically, make sure that player model
	// SCG[7/14/00]: skins do not use alpha.
	if( currentmodel->name[7] == 'g' )	// SCG[7/14/00]: check for 'g' ( global )
	{
		if( currentmodel->name[14] == 'm' )	// SCG[7/14/00]: check for 'm' ( m_?????.dkm )
		{
			// SCG[7/14/00]: do a strcmp to make sure it's a player model we're drawing
			if(	( strcmp( ptr, "m_hiro.dkm" ) == 0 ) ||
				( strcmp( ptr, "m_mikiko.dkm" ) == 0 ) ||
				( strcmp( ptr, "m_mikikofly.dkm" ) == 0 ) ||
				( strcmp( ptr, "m_superfly.dkm" ) == 0 ) )
			{
				return TRUE;
			}
		}
			
	}

	return FALSE;
}

//
// Ash -- precalc all the vertnormal dots
// 
void GL_PrecalcShadevectorDots()
{
    int i;

    for (i=0;i<NUMVERTEXNORMALS;i++)
        avertexnormaldots[i] = max(shadevector.DotProduct(avertexnormals.GetNormal(i)), 0.0f);
}


// 
// Ash -- do all the lighting at once
//
void GL_LightVerts()
{
    int i;

	if( r_fullbright->value )
	{
		for (i=0;i<NUMVERTEXNORMALS;i++)
		{
			// total light is ambient (= shadelight) + directional
			avertexcolors[i].x = 1.0;
			avertexcolors[i].y = 1.0;
			avertexcolors[i].z = 1.0;
		}
		return;
	}

	if( currententity->color.x || currententity->color.y || currententity->color.z )
	{
		for (i=0;i<NUMVERTEXNORMALS;i++)
		{
			// total light is ambient (= shadelight) + directional
			avertexcolors[i].x = currententity->color.x;
			avertexcolors[i].y = currententity->color.y;
			avertexcolors[i].z = currententity->color.z;
		}
	}
	else
	{
		for (i=0;i<NUMVERTEXNORMALS;i++)
		{
			// total light is ambient (= shadelight) + directional
			avertexcolors[i].x = min(shadelight.x + avertexnormaldots[i], 1.0f);
			avertexcolors[i].y = min(shadelight.y + avertexnormaldots[i], 1.0f);
			avertexcolors[i].z = min(shadelight.z + avertexnormaldots[i], 1.0f);
		}
	}
}                                        


inline void GL_EnvMapTexCoords(int vert_index, int normal_index, float* uv, CVector& entangles)
{
	CVector	v( s_lerped[vert_index][0], s_lerped[vert_index][1], s_lerped[vert_index][2] );
//    CVector& vert_pos = lerped[vert_index];
    CVector& vert_pos = v;
    CVector normal = avertexnormals.GetNormal(normal_index);
    CVector incident = model_cam_pos - vert_pos;

    incident.Normalize();

    CVector bounce = incident - (2.0f*incident.DotProduct(normal))*normal;
    CVector angles;

    bounce.VectorToAngles(angles);
    angles += entangles;
    angles.AngleToForwardVector(bounce);

    // do what gl does for envmapping:
    bounce.z += 1.0f;

    float invK = 0.5f/bounce.Length();

    uv[0] = 0.5f + bounce.x*invK;
    uv[1] = 0.5f + bounce.y*invK;
}

void GL_LerpVerts(int nverts, dtrivertx_t *v, dtrivertx_t *ov, const CVector &move, const CVector &frontv, const CVector &backv, float backlerp)
{
	int i;

/*
    if (0.0f == backlerp)
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		lerped[i].x = move.x + v->v[0] * frontv.x;
    		lerped[i].y = move.y + v->v[1] * frontv.y;
    		lerped[i].z = move.z + v->v[2] * frontv.z;
        }
    }
    else if (1.0f == backlerp)
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		lerped[i].x = move.x + ov->v[0] * backv.x;
    		lerped[i].y = move.y + ov->v[1] * backv.y;
    		lerped[i].z = move.z + ov->v[2] * backv.z;
        }
    }
    else
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		lerped[i].x = move.x + ov->v[0] * backv.x + v->v[0] * frontv.x;
    		lerped[i].y = move.y + ov->v[1] * backv.y + v->v[1] * frontv.y;
    		lerped[i].z = move.z + ov->v[2] * backv.z + v->v[2] * frontv.z;
        }
    }
*/
    if (0.0f == backlerp)
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		s_lerped[i][0] = move.x + v->v[0] * frontv.x;
    		s_lerped[i][1] = move.y + v->v[1] * frontv.y;
    		s_lerped[i][2] = move.z + v->v[2] * frontv.z;
        }
    }
    else if (1.0f == backlerp)
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		s_lerped[i][0] = move.x + ov->v[0] * backv.x;
    		s_lerped[i][1] = move.y + ov->v[1] * backv.y;
    		s_lerped[i][2] = move.z + ov->v[2] * backv.z;
        }
    }
    else
    {
        for ( i = 0 ; i < nverts; i++, v++, ov++ )
        {
    		s_lerped[i][0] = move.x + ov->v[0] * backv.x + v->v[0] * frontv.x;
    		s_lerped[i][1] = move.y + ov->v[1] * backv.y + v->v[1] * frontv.y;
    		s_lerped[i][2] = move.z + ov->v[2] * backv.z + v->v[2] * frontv.z;
        }
    }
}

void GL_LerpVerts(int nverts, dtrivertx2_t *v, dtrivertx2_t *ov, const CVector &move, const CVector &frontv, const CVector &backv, float backlerp)
{
	int i;

/*
    if (0.0f == backlerp)
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
            lerped[i].x = move.x + (v->v >> X_BIT_SHIFT) * frontv.x;
            lerped[i].y = move.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
            lerped[i].z = move.z + (v->v & Z_BIT_MASK) * frontv.z;
        }
    }
    else if (1.0f == backlerp)
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
    		lerped[i].x = move.x + (ov->v >> X_BIT_SHIFT) * backv.x;
    		lerped[i].y = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y;
    		lerped[i].z = move.z + (ov->v & Z_BIT_MASK) * backv.z;
    	}
    }
    else
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
    		lerped[i].x = move.x + (ov->v >> X_BIT_SHIFT) * backv.x + (v->v >> X_BIT_SHIFT) * frontv.x;
    		lerped[i].y = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
    		lerped[i].z = move.z + (ov->v & Z_BIT_MASK) * backv.z + (v->v & Z_BIT_MASK) * frontv.z;
    	}
    }
*/
    if (0.0f == backlerp)
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
            s_lerped[i][0] = move.x + (v->v >> X_BIT_SHIFT) * frontv.x;
            s_lerped[i][1] = move.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
            s_lerped[i][2] = move.z + (v->v & Z_BIT_MASK) * frontv.z;
        }
    }
    else if (1.0f == backlerp)
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
    		s_lerped[i][0] = move.x + (ov->v >> X_BIT_SHIFT) * backv.x;
    		s_lerped[i][1] = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y;
    		s_lerped[i][2] = move.z + (ov->v & Z_BIT_MASK) * backv.z;
    	}
    }
    else
    {
        for (i=0 ; i < nverts; i++, v++, ov++)
        {
    		s_lerped[i][0] = move.x + (ov->v >> X_BIT_SHIFT) * backv.x + (v->v >> X_BIT_SHIFT) * frontv.x;
    		s_lerped[i][1] = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
    		s_lerped[i][2] = move.z + (ov->v & Z_BIT_MASK) * backv.z + (v->v & Z_BIT_MASK) * frontv.z;
    	}
    }
}

/*
=============
GL_AddAliasGlow
=============
*/
void GL_AddAliasGlow (void)
{
	float	scale;
	float	min;

	oldshadelight = shadelight;

	scale = 0.05 * sin(r_newrefdef.time*7);

	min = shadelight.x * 0.8;
	shadelight.x += scale;
	if (shadelight.x < min)
		shadelight.x = min;
	min = shadelight.y * 0.8;
	shadelight.y += scale;
	if (shadelight.y < min)
		shadelight.y = min;
	min = shadelight.z * 0.8;
	shadelight.z += scale;
	if (shadelight.z < min)
		shadelight.z = min;

	shademodified = true;
}

#define SURFANIM_LOOP	0x01

/*
=============
GL_DrawAliasFrameLerp

interpolates between two frames and origins

=============
*/
void GL_DrawAliasFrameLerp( entity_t *ent, dmdl_t *paliashdr, float backlerp )
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t			*skin;
	GLenum			primtype;
	int				surface;				
//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
//	int				sCount[4] = {0,0,0,0};
//	int				lastSurf = -1;
    CVector         vColor;

	frame = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

    //adam: this is the replacement for the old block....
    move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;
    
    GL_LerpVerts( s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);

    // Ash -- now that we're lerped, build the head matrix
    R_BuildHeadMatrix(ent, gHeadMatrix);

	if( ent->flags & RF_TRANSLUCENT )
	{
		alpha = ent->alpha;
		qglEnable (GL_BLEND);
	}
	else
	{
		if( IsPlayerModel() == TRUE )
		{
			qglDisable (GL_BLEND);
			qglDisable (GL_ALPHA_TEST);
		}
		alpha = 1.0;
	}


	GL_LightVerts();

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t *)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures [ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += 3 * count;
			continue;
		}

		if( !skin )
		{
			skin = r_notexture;
		}

		if( skin->has_alpha )
		{
			qglDisable( GL_CULL_FACE );
		}
		else
		{
			qglEnable( GL_CULL_FACE );
		}

        GL_Bind( skin->texnum );
        
        qglBegin(primtype);

        do
        {
            index_xyz = *order++;
            
            // texture coordinates come from the draw list
            qglTexCoord2f (((float *)order)[0], ((float *)order)[1]);
            
            qglColor4f(avertexcolors[ov[index_xyz].lightnormalindex].x, 
                       avertexcolors[ov[index_xyz].lightnormalindex].y, 
                       avertexcolors[ov[index_xyz].lightnormalindex].z, alpha);
            
//            qglVertex3fv (lerped[index_xyz]);
            qglVertex3fv (s_lerped[index_xyz]);

            order += 2;
        
        } while (--count);

        qglEnd();
            
	}

	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);

	qglEnable (GL_TEXTURE_2D);
	qglEnable( GL_CULL_FACE );

}

void GL_DrawAliasFrameLerpEnv( entity_t *ent, dmdl_t *paliashdr, float backlerp )
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t			*skin;
	GLenum			primtype;
	int				surface;				
//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
//	int				sCount[4] = {0,0,0,0};
//	int				lastSurf = -1;
    CVector         vColor;

	frame = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

    //adam: this is the replacement for the old block....
    move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;
    
    GL_LerpVerts( s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);

    // Ash -- now that we're lerped, build the head matrix
    R_BuildHeadMatrix(ent, gHeadMatrix);

	if( ent->flags & RF_TRANSLUCENT )
	{
		alpha = ent->alpha;
		qglEnable (GL_BLEND);
	}
	else
	{
		if( IsPlayerModel() == TRUE )
		{
			qglDisable (GL_BLEND);
			qglDisable (GL_ALPHA_TEST);
		}
		alpha = 1.0;
	}

	GL_LightVerts();

    float uv[2];
    float texEnvColor[4];

	GL_EnableMultitexture(true);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t *)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures [ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += 3 * count;
			continue;
		}

		if( !skin )
		{
			skin = r_notexture;
		}

		if( skin->has_alpha )
		{
			qglDisable( GL_CULL_FACE );
		}
		else
		{
			qglEnable( GL_CULL_FACE );
		}

		if( qglMTexCoord2fSGIS )
		{
    		GL_MBind(GL_TEXTURE0_SGIS, skin->texnum);
    		GL_MBind(GL_TEXTURE1_SGIS, pEnvMapSkin->texnum);

			GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );
			GL_MTexEnv( GL_TEXTURE1_SGIS, GL_BLEND );

			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = gl_texenvcolor->value;
			GL_SelectTexture( GL_TEXTURE1_SGIS );
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;
            
				// texture coordinates come from the draw list
    			qglMTexCoord2fSGIS(GL_TEXTURE0_SGIS, ((float *)order)[0], ((float *)order)[1]);
    			
				GL_EnvMapTexCoords(index_xyz, ov[index_xyz].lightnormalindex, uv, ent->angles);
            
				qglMTexCoord2fSGIS(GL_TEXTURE1_SGIS, uv[0], uv[1]);
            
				qglColor4f(avertexcolors[ov[index_xyz].lightnormalindex].x, 
						   avertexcolors[ov[index_xyz].lightnormalindex].y, 
						   avertexcolors[ov[index_xyz].lightnormalindex].z, alpha);

//				qglVertex3fv (lerped[index_xyz]);
				qglVertex3fv (s_lerped[index_xyz]);

				order += 2;
        
			} while (--count);

			qglEnd();
        
			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = 0;
			GL_SelectTexture( GL_TEXTURE1_SGIS );
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);
        
		}
	}

	GL_EnableMultitexture(false);

	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);

	qglEnable (GL_TEXTURE_2D);
	qglEnable( GL_CULL_FACE );
}

/*
=============
GL_DrawAliasFrameLerpFogShell

Ash:  Does what FrameLerp does, 
except draws a fog shell instead
of the skin.


=============
*/

void GL_DrawAliasFrameLerpFogShell (entity_t *ent, dmdl_t *paliashdr)
{
	daliasframe_t	*frame;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			alpha;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface;				
    float           zOffset;

	frame = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
    
    order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);
    
    while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += ( ( 2 ) + 1 ) * count;
			continue;
		}
    	
        if ((!stricmp(s_psurface[surface].name, "s_head"))&&gfHaveHeadMatrix)
        {
            qglPushMatrix();
            qglLoadMatrixf(gHeadMatrix);
        }

	    zOffset = ent->origin.z - ((ent->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

		qglBegin (primtype);

        do
        {
            index_xyz = *order++;
            order += 2;

//            alpha = GL_GetFogSampleAlphaAtZ(lerped[index_xyz].z+zOffset, gFogSample);
            alpha = GL_GetFogSampleAlphaAtZ(s_lerped[index_xyz][2]+zOffset, gFogSample);

#pragma message("TODO Ash: remove this 3dfx minidriver fix")
            if (alpha<0.001f) alpha = 0.0f; // temp fix for 3dfx minidriver
            
            // normals and vertexes come from the frame list
            qglColor4f (gFogSample.fog_r, gFogSample.fog_g, gFogSample.fog_b, alpha);
//            qglVertex3fv (lerped[index_xyz]);
            qglVertex3fv (s_lerped[index_xyz]);
        } while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd ();

        if ((!stricmp(s_psurface[surface].name, "s_head"))&&gfHaveHeadMatrix)
        {
            qglPopMatrix();
        }
	}
}

/*
=============
GL_DrawAliasFrameLerp2
=============
*/
void GL_DrawAliasFrameLerp2 (entity_t *ent, dmdl_t *paliashdr, float backlerp)
{
	daliasframe2_t	*frame, *oldframe;
	dtrivertx2_t		*v, *ov;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t			*skin;
	GLenum			primtype;
	int				surface;				
//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
//	int				sCount[4] = {0,0,0,0};
//	int				lastSurf = -1;
    CVector         vColor;
    dstvert_t		*stvert;

	frame = (daliasframe2_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe2_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	stvert = ( dstvert_t * ) ( byte * ) s_pmdl + s_pmdl->ofs_st;

	order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

    //adam: this is the replacement for the old block....
    move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts( s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);

    if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable (GL_BLEND);
	}
	else
	{
		if( IsPlayerModel() == TRUE )
		{
			qglDisable (GL_BLEND);
			qglDisable (GL_ALPHA_TEST);
		}
		alpha = 1.0;
	}

	GL_LightVerts();

	if ( gl_vertex_arrays->value )
	{
		float colorArray[MAX_VERTS*3];
		float normalArray[MAX_VERTS*3];
//		float texcoordArray[MAX_VERTS*2];
/*
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		qglTexCoordPointer( 2, GL_FLOAT, 8, texcoordArray );

		// get the texture coords
		index_xyz = 0;
		while (1)
		{
			count = *order++;
			if( !count )
				break;

			if( count < 0 )
				count = -count;

			order++;
			surface = *order++;

			do
			{
				order++;
				texcoordArray[index_xyz * 2 + 0] = ( ( float * )order )[0];
				texcoordArray[index_xyz * 2 + 1] = ( ( float * )order )[1];
				index_xyz++;
				order += 2;
			} while( --count );
		}
*/
		//
		// pre light everything
		//
		for ( index_xyz = 0; index_xyz < s_pmdl->num_xyz; index_xyz++ )
		{
			colorArray[index_xyz * 3 + 0] = avertexcolors[ov[index_xyz].lightnormalindex].x;
			colorArray[index_xyz * 3 + 1] = avertexcolors[ov[index_xyz].lightnormalindex].y;
			colorArray[index_xyz * 3 + 2] = avertexcolors[ov[index_xyz].lightnormalindex].z;

			normalArray[index_xyz * 3 + 0] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[0];
			normalArray[index_xyz * 3 + 1] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[1];
			normalArray[index_xyz * 3 + 2] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[2];
		}

		qglEnableClientState( GL_VERTEX_ARRAY );
		qglVertexPointer( 3, GL_FLOAT, 16, s_lerped );

		qglEnableClientState( GL_COLOR_ARRAY );
		qglColorPointer( 3, GL_FLOAT, 0, colorArray );

		qglEnableClientState( GL_NORMAL_ARRAY );
		qglNormalPointer( GL_FLOAT, 12, normalArray );

		order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

		if ( qglLockArraysEXT != 0 )
		{
			qglLockArraysEXT( 0, s_pmdl->num_xyz );
		}


		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done
			if (count < 0)
			{
				count = -count;
				primtype = GL_TRIANGLE_FAN;
			}
			else
				primtype = GL_TRIANGLE_STRIP;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t *)ent->skin;	// assign skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures [ent->skinnum];
				}
				else
				{
					skin = currentmodel->skins[*order];
					if (!skin)
						skin = currentmodel->skins[0];
				}
			}

			order++;
			surface = *order;
			order++;

			// dont draw this surface
			if (s_psurface[surface].flags & SRF_NODRAW)
			{	
				order += 3 * count;
				continue;
			}

			if( !skin )
			{
				skin = r_notexture;
			}

			if( skin->has_alpha )
			{
				qglDisable( GL_CULL_FACE );
			}
			else
			{
				qglEnable( GL_CULL_FACE );
			}

			GL_Bind( skin->texnum );
        
			qglBegin(primtype);

			do
			{
				index_xyz = *order++;
            
				// texture coordinates come from the draw list
				qglTexCoord2f (((float *)order)[0], ((float *)order)[1]);
            
//				qglColor4f(avertexcolors[ov[index_xyz].lightnormalindex].x, 
//						   avertexcolors[ov[index_xyz].lightnormalindex].y, 
//						   avertexcolors[ov[index_xyz].lightnormalindex].z, alpha);
            
//				qglVertex3fv( lerped[index_xyz] );
				qglArrayElement( index_xyz );

				order += 2;
        
			} while (--count);

			qglEnd();
            
		}

		if ( qglUnlockArraysEXT != 0 )
			qglUnlockArraysEXT();
	}
	else
	{
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done
			if (count < 0)
			{
				count = -count;
				primtype = GL_TRIANGLE_FAN;
			}
			else
				primtype = GL_TRIANGLE_STRIP;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t *)ent->skin;	// assign skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures [ent->skinnum];
				}
				else
				{
					skin = currentmodel->skins[*order];
					if (!skin)
						skin = currentmodel->skins[0];
				}
			}

			order++;
			surface = *order;
			order++;

			// dont draw this surface
			if (s_psurface[surface].flags & SRF_NODRAW)
			{	
				order += 3 * count;
				continue;
			}

			if( !skin )
			{
				skin = r_notexture;
			}

			if( skin->has_alpha )
			{
				qglDisable( GL_CULL_FACE );
			}
			else
			{
				qglEnable( GL_CULL_FACE );
			}

			GL_Bind( skin->texnum );
        
			qglBegin(primtype);

			do
			{
				index_xyz = *order++;
            
				// texture coordinates come from the draw list
				qglTexCoord2f (((float *)order)[0], ((float *)order)[1]);
            
				qglColor4f(avertexcolors[ov[index_xyz].lightnormalindex].x, 
						   avertexcolors[ov[index_xyz].lightnormalindex].y, 
						   avertexcolors[ov[index_xyz].lightnormalindex].z, alpha);
            
//				qglVertex3fv( lerped[index_xyz] );
				qglVertex3fv( s_lerped[index_xyz] );

				order += 2;
        
			} while (--count);

			qglEnd();
            
		}
	}

	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);

	qglEnable (GL_TEXTURE_2D);
	qglEnable( GL_CULL_FACE );
}

void GL_DrawAliasFrameLerp2Env( entity_t *ent, dmdl_t *paliashdr, float backlerp )
{
	daliasframe2_t	*frame, *oldframe;
	dtrivertx2_t		*v, *ov;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t			*skin;
	GLenum			primtype;
	int				surface;				
//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
//	int				sCount[4] = {0,0,0,0};
//	int				lastSurf = -1;
    CVector         vColor;
    dstvert_t		*stvert;

	frame = (daliasframe2_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe2_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	stvert = ( dstvert_t * ) ( byte * ) s_pmdl + s_pmdl->ofs_st;

	order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

    //adam: this is the replacement for the old block....
    move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts( s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);

    if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable (GL_BLEND);
	}
	else
	{
		if( IsPlayerModel() == TRUE )
		{
			qglDisable (GL_BLEND);
			qglDisable (GL_ALPHA_TEST);
		}
		alpha = 1.0;
	}

	GL_LightVerts();

    float uv[2];
    float texEnvColor[4];

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t *)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures [ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += 3 * count;
			continue;
		}

		if( !skin )
		{
			skin = r_notexture;
		}

		if( skin->has_alpha )
		{
			qglDisable( GL_CULL_FACE );
		}
		else
		{
			qglEnable( GL_CULL_FACE );
		}

		if( qglMTexCoord2fSGIS )
		{
			GL_EnableMultitexture(true);

    		GL_MBind(GL_TEXTURE0_SGIS, skin->texnum);
    		GL_MBind(GL_TEXTURE1_SGIS, pEnvMapSkin->texnum);

			GL_MTexEnv( GL_TEXTURE0_SGIS, GL_REPLACE );
			GL_MTexEnv( GL_TEXTURE1_SGIS, GL_BLEND );

			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = gl_texenvcolor->value;
			GL_SelectTexture( GL_TEXTURE1_SGIS );
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;
            
				// texture coordinates come from the draw list
    			qglMTexCoord2fSGIS(GL_TEXTURE0_SGIS, ((float *)order)[0], ((float *)order)[1]);
    			
				GL_EnvMapTexCoords(index_xyz, ov[index_xyz].lightnormalindex, uv, ent->angles);
            
				qglMTexCoord2fSGIS(GL_TEXTURE1_SGIS, uv[0], uv[1]);
            
				qglColor4f(avertexcolors[ov[index_xyz].lightnormalindex].x, 
						   avertexcolors[ov[index_xyz].lightnormalindex].y, 
						   avertexcolors[ov[index_xyz].lightnormalindex].z, alpha);

				qglVertex3fv (s_lerped[index_xyz]);

				order += 2;
        
			} while (--count);

			qglEnd();
        
			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = 0;
			GL_SelectTexture( GL_TEXTURE1_SGIS );
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);
        
			GL_EnableMultitexture(false);
		}
		else
		{
			order += 3;
		}
	}

	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);

	qglEnable (GL_TEXTURE_2D);
	qglEnable( GL_CULL_FACE );
}

/*
=============
GL_DrawAliasFrameLerp2FogShell

Ash:  Does what FrameLerp does, 
except draws a fog shell instead
of the skin.


=============
*/

void GL_DrawAliasFrameLerp2FogShell (entity_t *ent, dmdl_t *paliashdr)
{
	daliasframe2_t	*frame;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			alpha;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface;				
	float			r, g, b;
    float           zOffset;

	frame = (daliasframe2_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
    
    order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);
	
    while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += ( ( 2 ) + 1 ) * count;
			continue;
		}
    	
        if ((!stricmp(s_psurface[surface].name, "s_head"))&&gfHaveHeadMatrix)
        {
            qglPushMatrix();
            qglLoadMatrixf(gHeadMatrix);
        }

	    zOffset = ent->origin.z - ((ent->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

		qglBegin (primtype);

        r = gFogSample.fog_r;
        g = gFogSample.fog_g;
        b = gFogSample.fog_b;
        do
        {
            index_xyz = *order++;
            order += 2;

//            alpha = GL_GetFogSampleAlphaAtZ(lerped[index_xyz].z+zOffset, gFogSample);
            alpha = GL_GetFogSampleAlphaAtZ(s_lerped[index_xyz][2]+zOffset, gFogSample);

#pragma message("TODO Ash: remove this 3dfx minidriver fix")
            if (alpha<0.001f) alpha = 0.0f; // temp fix for 3dfx minidriver
            
            // normals and vertexes come from the frame list
            qglColor4f (r, g, b, alpha);
//            qglVertex3fv (lerped[index_xyz]);
            qglVertex3fv (s_lerped[index_xyz]);
        } while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd ();

        if ((!stricmp(s_psurface[surface].name, "s_head"))&&gfHaveHeadMatrix)
        {
            qglPopMatrix();
        }
	}
}

/*
=============
GL_DrawClusterFrameLerp

interpolates between two frames and origins

=============
*/




/*
=============
GL_DrawSpotShadow
=============
*/
extern	CVector			lightspot;
void CreateSurfaceSpritePoly( surfSprite_t& surfSprite, int nWidth, int nHeight );
void BoundPoly (int numverts, CVector *verts, CVector &mins, CVector &maxs);
qboolean PointInSurface( msurface_t *pSurface, CVector vPoint, float radius, CVector& vMins, CVector& vMaxs );

#define MAX_HIT_SURFACES	6

void GL_DrawSpotShadow( entity_t *ent, CVector& lightdir )
{
	surfSprite_t	surfSprite;
	msurface_t		*pCurrentSurface;
	faceList_t		*pFaceList;
	int				i, numHitSurfaces, nIndex;
	CVector			vMins, vMaxs;
    msurface_t*     hitSurfaces[MAX_HIT_SURFACES];
	int				nWidth, nHeight;//, nLHeight;
	model_t			*pCurrentModel;
	float			fHeight;

	fHeight = lightspot.z;

	if( r_worldmodel == NULL )
	{
		return;
	}

	if( ent->flags & RF_WEAPONMODEL )
	{
		return;
	}

	if( pointSurface.polys == NULL )
	{
		return;
	}

	if( pointSurface.polys->verts == NULL )
	{
		return;
	}

	if( pointSurface.texinfo->flags & SURF_MIDTEXTURE )
	{
		return;
	}

	for( i = 0; i < MAX_HIT_SURFACES; i++ )
	{
		hitSurfaces[i] = NULL;
	}

	qglDepthMask( 0 );

	surfSprite.vOrigin		= ent->origin;
//	surfSprite.vOrigin.z	= pointSurface.polys->verts[0].z;
//	surfSprite.vOrigin.z	= ent->origin.z + s_pmdl->org.z;
	surfSprite.vOrigin.z	= fHeight;

	surfSprite.nFrame		= 0;
	surfSprite.fScale		= 1.0;
	surfSprite.nFlags		= 0;
	surfSprite.fRoll		= -ent->angles.y + 90;
	surfSprite.pImage		= r_shadowtexture;
	surfSprite.pModel		= NULL;
	surfSprite.vLightValue	= shadelight;
	
	nWidth	= ( ent->maxs.x - ent->mins.x ) * 0.5;
	nHeight	= ( ent->maxs.y - ent->mins.y ) * 0.5;

	nIndex	= pointSurface.plane->planeIndex;

	pFaceList = r_worldmodel->planePolys->planes[nIndex];

	if( pFaceList == NULL )
	{
		return;
	}

    numHitSurfaces = 0;

	pCurrentModel = r_worldmodel;

	for( i = 0; i < pFaceList->numFaces; i++ )
	{
		pCurrentSurface = pCurrentModel->surfaces + pFaceList->faceNum[i];

		// SCG[5/10/99]: Get the mins maxs for this surface
		BoundPoly( pCurrentSurface->polys->numverts, pCurrentSurface->polys->verts, vMins, vMaxs );

		if( PointInSurface( pCurrentSurface, surfSprite.vOrigin, nWidth / 2, vMins, vMaxs ) && numHitSurfaces<MAX_HIT_SURFACES)
            hitSurfaces[numHitSurfaces++] = pCurrentSurface;
	}

	GL_Bind( surfSprite.pImage->texnum );

	if( ent->flags & RF_TRANSLUCENT )
	{
		qglColor4f( surfSprite.vLightValue.x, surfSprite.vLightValue.y, surfSprite.vLightValue.z, ent->alpha );
	}
	else
	{
		qglColor4f( surfSprite.vLightValue.x, surfSprite.vLightValue.y, surfSprite.vLightValue.z, 1.0 );
	}

	for( i = 0; i < numHitSurfaces; i++ )
	{
		surfSprite.pSurf		= hitSurfaces[i];
		surfSprite.vNormal		= hitSurfaces[i]->plane->normal; 
		surfSprite.numpoints	= 0;

		R_LightPoint( surfSprite.vOrigin, surfSprite.vLightValue );

		CreateSurfaceSpritePoly( surfSprite, nWidth, nHeight );

		if( surfSprite.numpoints > 8 )
		{
			return;
		}

		qglBegin( GL_POLYGON );
		for( int j = 0; j < surfSprite.numpoints; j++ )
		{
			qglTexCoord2f( surfSprite.points[j].s, surfSprite.points[j].t );
			qglVertex3fv( surfSprite.points[j].pos );
		}
		qglEnd ();
	}

	qglDepthMask( 1 );
}


/*
** R_CullAliasModel
*/
static qboolean R_CullAliasModel( CVector bbox[8], entity_t *e)
{
    CVector         mins, maxs;
	int i;
	CVector			vectors[3];
	CVector			thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t	*pframe, *poldframe;
	daliasframe2_t	*pframe2, *poldframe2;
	CVector			angles;
	CVector			scaled_org;
	CVector			scale;
	CVector			frame_scale, frame_translate;
	CVector			oldframe_scale, oldframe_translate;
	int				version, same_frame = false;

	if ( ( e->frame >= s_pmdl->num_frames ) || ( e->frame < 0 ) )
	{
//		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel: no such frame %d\n", e->frame);
		e->frame = 0;
	}
	if ( ( e->oldframe >= s_pmdl->num_frames ) || ( e->oldframe < 0 ) )
	{
//		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel: no such oldframe %d\n", e->oldframe);
		e->oldframe = 0;
	}

	version = LittleLong (s_pmdl->version);

	if( version == ALIAS_VERSION2 ){
		pframe2 = ( daliasframe2_t * ) ( ( byte * ) s_pmdl + 
										  s_pmdl->ofs_frames +
										  e->frame * s_pmdl->framesize);

		poldframe2 = ( daliasframe2_t * ) ( ( byte * ) s_pmdl + 
										  s_pmdl->ofs_frames +
										  e->oldframe * s_pmdl->framesize);
	}
	else{
		pframe = ( daliasframe_t * ) ( ( byte * ) s_pmdl + 
										  s_pmdl->ofs_frames +
										  e->frame * s_pmdl->framesize);

		poldframe = ( daliasframe_t * ) ( ( byte * ) s_pmdl + 
										  s_pmdl->ofs_frames +
										  e->oldframe * s_pmdl->framesize);
	}

	/*
	** compute axially aligned mins and maxs
	*/
	if( version == ALIAS_VERSION2 ){
		if ( pframe2 == poldframe2 )
			same_frame = true;
		scale.x = X_BIT_RES;
		scale.y = Y_BIT_RES;
		scale.z = Z_BIT_RES;
		frame_scale			= pframe2->scale;
		frame_translate		= pframe2->translate;
		oldframe_scale		= poldframe2->scale;
		oldframe_translate	= poldframe2->translate;
	}
	else{
		if ( pframe == poldframe )
			same_frame = true;
		scale.x = scale.y = scale.z = 255;
		frame_scale			= pframe->scale;
		frame_translate		= pframe->translate;
		oldframe_scale		= poldframe->scale;
		oldframe_translate	= poldframe->translate;
	}
	if ( same_frame )
	{
		mins = frame_translate;
		maxs.x = mins.x + frame_scale.x * scale.x;
		maxs.y = mins.y + frame_scale.y * scale.y;
		maxs.z = mins.z + frame_scale.z * scale.z;
	}
	else
	{
		thismins = frame_translate;
		thismaxs.x = thismins.x + frame_scale.x * scale.x;
		thismaxs.y = thismins.y + frame_scale.y * scale.y;
		thismaxs.z = thismins.z + frame_scale.z * scale.z;

		oldmins  = oldframe_translate;
		oldmaxs.x  = oldmins.x + oldframe_scale.x * scale.x;
		oldmaxs.y  = oldmins.y + oldframe_scale.y * scale.y;
		oldmaxs.z  = oldmins.z + oldframe_scale.z * scale.z;

		if ( thismins.x < oldmins.x )
			mins.x = thismins.x;
		else
			mins.x = oldmins.x;
		if ( thismaxs.x > oldmaxs.x )
			maxs.x = thismaxs.x;
		else
			maxs.x = oldmaxs.x;

		if ( thismins.y < oldmins.y )
			mins.y = thismins.y;
		else
			mins.y = oldmins.y;
		if ( thismaxs.y > oldmaxs.y )
			maxs.y = thismaxs.y;
		else
			maxs.y = oldmaxs.y;

		if ( thismins.z < oldmins.z )
			mins.z = thismins.z;
		else
			mins.z = oldmins.z;
		if ( thismaxs.z > oldmaxs.z )
			maxs.z = thismaxs.z;
		else
			maxs.z = oldmaxs.z;
	}

	//	Nelno:	scale bounding box
	mins.x = mins.x * e->render_scale.x;
	mins.y = mins.y * e->render_scale.y;
	mins.z = mins.z * e->render_scale.z;
	maxs.x = maxs.x * e->render_scale.x;
	maxs.y = maxs.y * e->render_scale.y;
	maxs.z = maxs.z * e->render_scale.z;
	
	//	Nelno:	compute scaled center of model
	scaled_org.x = e->origin.x - ((e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x);
	scaled_org.y = e->origin.y - ((e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y);
	scaled_org.z = e->origin.z - ((e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

	/*
	** compute a full bounding box
	*/
	for ( i = 0; i < 8; i++ )
	{
		CVector   tmp;

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

		bbox[i] = tmp;
	}

	/*
	** rotate the bounding box
	*/
	angles = e->angles;
	angles.y = -angles.y;
	AngleToVectors( angles, vectors[0], vectors[1], vectors[2] );

	e->mins = mins;
	e->maxs = maxs;

	for ( i = 0; i < 8; i++ )
	{
		CVector tmp;

		tmp = bbox[i];

		bbox[i].x = DotProduct( vectors[0], tmp );
		bbox[i].y = -DotProduct( vectors[1], tmp );
		bbox[i].z = DotProduct( vectors[2], tmp );

		bbox[i] = bbox[i] + scaled_org;
	}

	{
		int p, f, aggregatemask = ~0;

		for ( p = 0; p < 8; p++ )
		{
			int mask = 0;

			for ( f = 0; f < 4; f++ )
			{
				float dp = DotProduct( frustum[f].normal, bbox[p] );

				if ( ( dp - frustum[f].dist ) < 0 )
				{
					mask |= ( 1 << f );
				}
			}

			aggregatemask &= mask;
		}

		if ( aggregatemask )
		{
			return true;
		}

		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void R_RotateAndScaleEntity (entity_t *e)
{
	CVector		scale_ofs;

	if (e->render_scale.x == 0.0 && e->render_scale.y == 0.0 && e->render_scale.z == 0.0)
	{
//		ri.Sys_Error (ERR_FATAL, "Entity with model %s has 0, 0, 0 render_scale.\n", currentmodel->name);
		ri.Con_Printf( PRINT_DEVELOPER, "R_DrawAliasModel: %s has 0 scale: %f %f %f\n", currentmodel->name, e->render_scale.x, e->render_scale.y, e->render_scale.z );
		e->render_scale.x = e->render_scale.y = e->render_scale.z = 1.0;

	}

	//	Nelno:	add x, y and z origin offsets here.
	scale_ofs.x = (e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x;
	scale_ofs.y = (e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y;
	scale_ofs.z = (e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z;

	qglTranslatef (e->origin.x - scale_ofs.x,  e->origin.y - scale_ofs.y,  e->origin.z - scale_ofs.z);
	
	qglRotatef (e->angles.y,  0, 0, 1);
	qglRotatef (-e->angles.x,  0, 1, 0);
	qglRotatef (-e->angles.z,  1, 0, 0);
 
	qglScalef (e->render_scale.x, e->render_scale.y, e->render_scale.z);
}


void R_BuildHeadMatrix (entity_t *e, float *pMatrix)
{
	CVector		scale_ofs;
    CVector     hardPt, hardPt1, hardPt2;
    dsurface_t *pSurfaceList = (dsurface_t *) ((byte *) s_pmdl + s_pmdl->ofs_surfaces);
    dsurface_t *pSurface = NULL;
    CVector     deltaAngles;

    gfHaveHeadMatrix = false;
	
	for (int i = 0; i < s_pmdl->num_surfaces; i++)
	{
		if (!stricmp (pSurfaceList[i].name, "s_head"))
		{
			pSurface = &pSurfaceList[i];
			break;
		}
	}

    if ((!pSurface)||(-1==pSurface->num_uvframes))
        return;

    // check that we have cluster info (size in e->count)
    if (0 == e->count)
        return;
    
    // look angles stored in entry 0 in cluster array e->pDef
    deltaAngles = e->pDef[0].angles;

    dtriangle_t *pHPTri = (dtriangle_t *)((byte *)s_pmdl + s_pmdl->ofs_tris) + pSurface->num_uvframes;

//    hardPt += lerped[pHPTri->index_xyz[0]];
//    hardPt += lerped[pHPTri->index_xyz[1]];
//    hardPt += lerped[pHPTri->index_xyz[2]];
    hardPt += s_lerped[pHPTri->index_xyz[0]];
    hardPt += s_lerped[pHPTri->index_xyz[1]];
    hardPt += s_lerped[pHPTri->index_xyz[2]];

    hardPt = (1.0f/3.0f)*hardPt;

    qglPushMatrix();
    qglLoadMatrixf(gHeadMatrix);    // this was saved off before RotateAndScaleEntity

    if (e->render_scale.x == 0.0 && e->render_scale.y == 0.0 && e->render_scale.z == 0.0)
	{
//		ri.Sys_Error (ERR_FATAL, "Entity with model %s has 0, 0, 0 render_scale.\n", currentmodel->name);
		ri.Con_Printf( PRINT_DEVELOPER, "R_DrawAliasModel: %s has 0 scale: %f %f %f\n", currentmodel->name, e->render_scale.x, e->render_scale.y, e->render_scale.z );
		e->render_scale.x = e->render_scale.y = e->render_scale.z = 1.0;
	}

    //	Nelno:	add x, y and z origin offsets here.
    scale_ofs.x = (e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x;
    scale_ofs.y = (e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y;
    scale_ofs.z = (e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z;

    qglTranslatef (e->origin.x - scale_ofs.x,  e->origin.y - scale_ofs.y,  e->origin.z - scale_ofs.z);
    
    qglRotatef (e->angles.y,  0, 0, 1);
    qglRotatef (-e->angles.x,  0, 1, 0);
    qglRotatef (-e->angles.z,  1, 0, 0);

    // rotate the head
    qglTranslatef(hardPt.x, hardPt.y, hardPt.z);
    qglRotatef(deltaAngles.yaw, 0, 0, 1);
    qglRotatef(deltaAngles.pitch, 0, 1, 0);
    qglTranslatef(-hardPt.x, -hardPt.y, -hardPt.z);

    qglScalef (e->render_scale.x, e->render_scale.y, e->render_scale.z);

    qglGetFloatv(GL_MODELVIEW_MATRIX, pMatrix);
    gfHaveHeadMatrix = true;

    qglPopMatrix();
}

/*
=================
R_DrawAliasModel

=================
*/

void R_DrawAliasModel (entity_t *ent)
{
	int			i;
	CVector		bbox[8];
    int			version;
	float		blend_colors [4] = {0.5, 0.5, 0.5, 0.5};
    bool        fDLight;
    CVector     dloc;
    bool        fDrawShadow;

	memset( &pointSurface, 0, sizeof( msurface_t ) );

	GL_SetState( GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST );
	GL_SetFunc( GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0 );

	currentmodel = (model_t*)ent->model;
		
	s_pmdl = (dmdl_t *)currentmodel->extradata;
	s_psurface = (dsurface_t *) ((byte *) s_pmdl + s_pmdl->ofs_surfaces);

	if ( !( ent->flags & RF_WEAPONMODEL ) )
	{
		if ( R_CullAliasModel( bbox, ent) )
			return;
	}

	ent_draw_count++;

	version = LittleLong (s_pmdl->version);
	
    if ( ent->flags & RF_WEAPONMODEL )
	{
		//if ( r_lefthand->value == 2 || r_drawweapon->value == 0)
        if (weapon_visible->value == 0)
			return;

		qglDisable( GL_FOG );
	}

    // resolve shadowing
    if(gl_shadows->value && 
		( r_worldmodel != NULL ) &&
       r_worldmodel->lightdata && 
       !(ent->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)) && 
       version != ALIAS_VERSION2 )
    {
        fDrawShadow = true;
    }
    else
        fDrawShadow = false;

	//
	// get lighting information
	//
    if ( ent->flags & RF_FULLBRIGHT || r_fullbright->value)
	{
        // if shadowed, we need to compute the shadevector and lightspot
        if (fDrawShadow)
            R_LightPoint (ent->origin, shadelight, &dloc, &pointSurface );

        fDLight = false;

		shadelight.Set( 1.0, 1.0, 1.0 );
	}
	else
	{
		fDLight = R_LightPoint (ent->origin, shadelight, &dloc, &pointSurface );

		// player lighting hack for communication back to server
		// big hack!

		if ( ent->flags & RF_WEAPONMODEL )
		{
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
		
		if ( gl_monolightmap->string[0] != '0' )
		{
			float s = shadelight.x;

			if ( s < shadelight.y )
				s = shadelight.y;
			if ( s < shadelight.z )
				s = shadelight.z;

			shadelight.x = s;
			shadelight.y = s;
			shadelight.z = s;
		}
	}

	if ( ent->flags & RF_MINLIGHT )
	{
		//	Nelno:	changed from 0.1 for testing
		for (i=0 ; i<3 ; i++)
		{
			if (shadelight[i] > 0.3)
				break;
		}

		if (i == 3)
		{
			shadelight.x = 0.3;
			shadelight.y = 0.3;
			shadelight.z = 0.3;
		}
	}

	if ( ent->flags & RF_GLOW )
	{	// bonus items will pulse with time
		float	scale;
		float	min;

		scale = sin (r_newrefdef.time*2);

		min = shadelight.x;
		shadelight.x += scale;
		if (shadelight.x < min)
			shadelight.x = min;
		min = shadelight.y;
		shadelight.y += scale;
		if (shadelight.y < min)
			shadelight.y = min;
		min = shadelight.z;
		shadelight.z += scale;
		if (shadelight.z < min)
			shadelight.z = min;
	}

    CVector fixedshadevector, fixedshadeangles, shadeangles;

    // Ash -- here's the fixed light direction
    fixedshadevector.Set(-1, -1, -1);                           // world space
    fixedshadevector.VectorToAngles(fixedshadeangles);
    fixedshadeangles.yaw -= ent->angles.yaw;
    fixedshadeangles.AngleToForwardVector(fixedshadevector);    // entity space

    // Ash -- here's the dynamic light direction
    if (fDLight)
    {
        // Ash -- use direction to the strongest dynamic light
        shadevector = dloc - ent->origin;                       // world space
        shadevector.Normalize();

        shadevector.VectorToAngles(shadeangles);
        shadeangles.yaw -= ent->angles.yaw;
        shadeangles.AngleToForwardVector(shadevector);          // entity space

    }
    else
    {
        // Ash -- no dynamic lights -- use a fixed direction
        shadevector = fixedshadevector;
    }

    // Ash -- for envmapping, get the camera's position in the model's coord system
    {
        model_cam_pos = r_newrefdef.vieworg - ent->origin;
        float mag = model_cam_pos.Normalize();
        CVector angles;
        model_cam_pos.VectorToAngles(angles);
        angles.yaw -= ent->angles.yaw;
        angles.AngleToForwardVector(model_cam_pos);
        model_cam_pos = mag*model_cam_pos;
    }

    
    // Now precalc the vert normals with the shadevector before rendering the model
    GL_PrecalcShadevectorDots();
    

	//
	// locate the proper data
	//

	c_alias_polys += s_pmdl->num_tris;

	//
	// draw all the triangles
	//

    qglPushMatrix ();
	ent->angles.x = -ent->angles.pitch;	// sigh.
    
    qglGetFloatv(GL_MODELVIEW_MATRIX, gHeadMatrix);  // save this off to use inside DrawAliasFrameLerp*
    R_RotateAndScaleEntity (ent);

	ent->angles.x = -ent->angles.pitch;	// sigh.

	//	show name of model for testing
//	R_GetProjectedVert (ent->origin, &x, &y);
//	ri.dk_printxy (x, y, ent->currentmodel->name);

	// draw it

	qglShadeModel (GL_SMOOTH);

	GL_TexEnv( GL_MODULATE );

	if ( (ent->frame >= s_pmdl->num_frames) 
		|| (ent->frame < 0) )
	{
//		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel: no such frame %d\n",
//			ent->frame);
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ( (ent->oldframe >= s_pmdl->num_frames)
		|| (ent->oldframe < 0))
	{
//		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel: no such oldframe %d\n",
//			ent->oldframe);
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ( !r_lerpmodels->value )
		ent->backlerp = 0;

	if (qglColorTableEXT)
	{
		qglEnable( GL_SHARED_TEXTURE_PALETTE_EXT );

		image_t* skin = NULL;

/*
		if( gl_envmap->value )
		{
//			skin = (image_t *)RegisterSkin("skins/player_chrome1.wal", RESOURCE_GLOBAL);
			skin = pEnvMapSkin;
		}
		else
*/		{
			// support custom and reassigned skins
			if (ent->skin)
			{
				skin = (image_t *)ent->skin;	// custom player skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures [ent->skinnum];
				}
				else
				{
					// default
					skin = currentmodel->skins[0];
				}
			}
		}

		// SCG[10/30/99]: This fuckin sucks!
		// SCG[10/30/99]: someone decided to make this a surface flags instead of an entity flag...
		int count, surface, *order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);

		while( 1 )
		{
			// get the vertex count and primitive type
			count = *order++;
			if( !count)	break;
			if( count < 0 ) count = -count;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t *)ent->skin;	// assign skin
				break;
			}

			order++;
			surface = *order;
			order++;

			order += 3 * count;
		}

		GL_SetTexturePalette( skin, TRUE );
    }

	version = LittleLong (s_pmdl->version);

    if( version == ALIAS_VERSION2 ){
		if( qglMTexCoord2fSGIS && ent->flags & RF_ENVMAP && gl_envmap->value ) GL_DrawAliasFrameLerp2Env( ent, s_pmdl, ent->backlerp );
		else GL_DrawAliasFrameLerp2( ent, s_pmdl, ent->backlerp );
	}
	else{
		if( qglMTexCoord2fSGIS && ent->flags & RF_ENVMAP && gl_envmap->value ) GL_DrawAliasFrameLerpEnv( ent, s_pmdl, ent->backlerp );
		else GL_DrawAliasFrameLerp( ent, s_pmdl, ent->backlerp );
	}

//
//  Ash -- no comment
//

    if (gl_procshell->value)
    {   
        qglDisable (GL_TEXTURE_2D);
        qglEnable (GL_BLEND);
        GL_TexEnv(GL_MODULATE);
    
        qglDepthFunc(GL_LEQUAL);
        qglDepthMask(0);
    
        if( version != ALIAS_VERSION2 )
            GL_DrawAliasFrameLerpProcShell (ent, s_pmdl);

        qglDepthMask(1);
        qglDisable(GL_BLEND);
        qglEnable(GL_TEXTURE_2D);
    }


//
//  Ash - Draw the fog shell
//
    if (!gl_drawflat->value)
    {
        int nFogSamples = R_MAX_FOG_SAMPLES;   // set this appropriately for the thing being fogged
        if ((gl_vfogents->value==1)&&(true==GL_FogBBox(bbox, 8, gFogSample, nFogSamples)))
        {
    		qglDisable (GL_TEXTURE_2D);
    		qglEnable (GL_BLEND);
            GL_TexEnv(GL_MODULATE);
            
            qglDepthFunc(GL_LEQUAL);
            qglDepthMask(0);
        	
            if( version == ALIAS_VERSION2 )
                GL_DrawAliasFrameLerp2FogShell (ent, s_pmdl);
            else
                GL_DrawAliasFrameLerpFogShell (ent, s_pmdl);
            
            qglDepthMask(1);
            qglDisable(GL_BLEND);
            qglEnable(GL_TEXTURE_2D);
        }
    }

    qglPopMatrix ();

//
// Draw the shadow
//
	if( ( gl_shadows->value ) && 
		!( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) && 
		( r_worldmodel != NULL ) && 
		( r_worldmodel->lightdata != NULL ) )
	{
		qglPushMatrix ();
		qglEnable (GL_BLEND);
		GL_TexEnv( GL_MODULATE );
		qglShadeModel( GL_SMOOTH );
		qglDepthFunc( GL_LESS );
        
		GL_DrawSpotShadow( ent, fixedshadevector );

		qglDepthFunc( GL_LEQUAL );
		qglDisable( GL_BLEND );
		qglPopMatrix();
	}

	GL_TexEnv( GL_REPLACE );
    qglShadeModel (GL_FLAT);
//
// Draw the bounding box
//
    if (gl_bbox->value)
    {
        qglColor4f (1.0f, 1.0f, 1.0f, 0.3f);
    	qglDisable( GL_CULL_FACE );
    	qglDisable( GL_TEXTURE_2D );
    	qglEnable( GL_BLEND );
    	
        qglBegin( GL_QUADS );
  		
        qglVertex3fv( bbox[0] );
        qglVertex3fv( bbox[1] );
        qglVertex3fv( bbox[3] );
        qglVertex3fv( bbox[2] );

        qglVertex3fv( bbox[0] );
        qglVertex3fv( bbox[1] );
        qglVertex3fv( bbox[5] );
        qglVertex3fv( bbox[4] );
        
        qglVertex3fv( bbox[1] );
        qglVertex3fv( bbox[3] );
        qglVertex3fv( bbox[7] );
        qglVertex3fv( bbox[5] );
        
        qglVertex3fv( bbox[2] );
        qglVertex3fv( bbox[3] );
        qglVertex3fv( bbox[7] );
        qglVertex3fv( bbox[6] );
        
        qglVertex3fv( bbox[0] );
        qglVertex3fv( bbox[2] );
        qglVertex3fv( bbox[6] );
        qglVertex3fv( bbox[4] );
        
        qglVertex3fv( bbox[4] );
        qglVertex3fv( bbox[5] );
        qglVertex3fv( bbox[7] );
        qglVertex3fv( bbox[6] );
        
        qglEnd();

    	qglDisable( GL_BLEND);
    	qglEnable( GL_TEXTURE_2D );
        qglEnable( GL_CULL_FACE );
    }

	if ( ent->flags & RF_TRANSLUCENT )
	{
		qglDisable (GL_BLEND);
	}

	qglColor4f (1,1,1,1);

	if ( ent->flags & RF_WEAPONMODEL && (r_newrefdef.foginfo.active || r_testfog->value) ){
		qglEnable( GL_FOG );
	}

	GL_TexEnv (GL_REPLACE);

	GL_SetState( 0 );
}

///////////////////////////////////////////////////////////////////////////////
//	R_DrawAliasIcon
//
///////////////////////////////////////////////////////////////////////////////

void R_DrawAliasIcon (entity_t *ent)
{
	float		an;
	unsigned int	nStateFlags;
    int			version;

	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		return;
	}

//	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LESS, -1 );

//	nStateFlags = GLSTATE_PRESET1;

//	nStateFlags &= ~( GLSTATE_CULL_FACE_FRONT | GLSTATE_BLEND );

	currentmodel = (model_t*)ent->model;

	s_pmdl = (dmdl_t *)currentmodel->extradata;
	if( s_pmdl == NULL )
	{
		return;
	}

	s_psurface = (dsurface_t *) ((byte *) s_pmdl + s_pmdl->ofs_surfaces);
	if( s_psurface == NULL )
	{
		return;
	}

	ent_draw_count++;

    if ( ent->flags & RF_FULLBRIGHT )
	{
		shadelight.x = 1.0;
		shadelight.y = 1.0;
		shadelight.z = 1.0;
	}
	else
	{
		shadelight.x = 0.5;
		shadelight.y = 0.5;
		shadelight.z = 0.5;
	}

	an = ent->angles.y/180*M_PI;
	shadevector.x = cos(-an);
	shadevector.y = sin(-an);
	shadevector.z = 1;
	shadevector.Normalize();

	//
	// locate the proper data
	//

	c_alias_polys += s_pmdl->num_tris;

	//
	// draw all the triangles
	//

    qglPushMatrix ();

	ent->angles.x = -ent->angles.x;	// sigh.
	R_RotateAndScaleEntity (ent);
	ent->angles.x = -ent->angles.x;	// sigh.

	if (ent->flags & RF_TRANSLUCENT)
	{
		nStateFlags |= GLSTATE_BLEND;
		GL_TexEnv( GL_MODULATE );
	}

	if ( (ent->frame >= s_pmdl->num_frames) 
		|| (ent->frame < 0) )
	{
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ( (ent->oldframe >= s_pmdl->num_frames)
		|| (ent->oldframe < 0))
	{
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ( !r_lerpmodels->value )
		ent->backlerp = 0;


	if (qglColorTableEXT)
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;

		GL_SetTexturePalette( currentmodel->skins[0], FALSE );
 	}

	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE );
//	GL_SetState( nStateFlags );

//	GL_DrawAliasFrameLerp (ent, s_pmdl, ent->backlerp);
	version = LittleLong (s_pmdl->version);
	
    if( version == ALIAS_VERSION2 ){
		GL_DrawAliasFrameLerp2( ent, s_pmdl, ent->backlerp );
	}
	else{
		GL_DrawAliasFrameLerp( ent, s_pmdl, ent->backlerp );
	}

	GL_TexEnv( GL_REPLACE );

	qglPopMatrix ();

	qglColor4f (1,1,1,1);
	
	GL_SetState( 0 );
}

inline void R_ShellAgePatch(shell_t *pShell, int index)
{
    float val = 0.2f/gl_procd->value;
    pShell->pPatches[index].data++;
    pShell->pVData[pShell->pPatches[index].vindex[0]]+=val;
    pShell->pVData[pShell->pPatches[index].vindex[1]]+=val;
    pShell->pVData[pShell->pPatches[index].vindex[2]]+=val;
}

inline void R_ShellKillPatch(shell_t *pShell, int index)
{
    float val = 0.2f*pShell->pPatches[index].data/gl_procd->value;
    pShell->pVData[pShell->pPatches[index].vindex[0]]-=val;
    pShell->pVData[pShell->pPatches[index].vindex[1]]-=val;
    pShell->pVData[pShell->pPatches[index].vindex[2]]-=val;
    pShell->pPatches[index].data=0;
}

void R_ShellUpdate(shell_t *pShell)
{
    int index, nnum, nindex;
    shell_patch_t *pPatch;

    if (gl_procshell->value==1)
    {
        if (!pShell->numActive)
        {
            // if none are active, turn one on at random
            index = rand() % pShell->numPatches;
            R_ShellAgePatch(pShell, index);
            pShell->numActive = 1;
        }
        else
        {
            // advance the ages of the active
            for (index=0;index<pShell->numPatches;index++)
            {
                pPatch = pShell->pPatches+index;

                // if it's alive, make it older
                if (0<pPatch->data)
                {
                    R_ShellAgePatch(pShell, index);

                    // if it's too old, kill it
                    if (pPatch->data>gl_procd->value)
                    {
                        R_ShellKillPatch(pShell, index);
                        pShell->numActive--;

                        // if it's the last one, maybe start over here
                        if ((1==pShell->numActive) && (rand()&1))
                        {
                            R_ShellAgePatch(pShell, index);
                            pShell->numActive++;
                        }
                    }
                    // if it's fertile, have some babies
                    else if (pPatch->data>=gl_procp->value&&pPatch->data<=gl_procm->value)
                    {
                        for (nnum=0;nnum<3;nnum++)
                        {
                            nindex = pPatch->nindex[nnum];
                            
                            if (0==pShell->pPatches[nindex].data)
                            {
                                R_ShellAgePatch(pShell, nindex);
                                pShell->numActive++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void GL_DrawAliasFrameLerpProcShell (entity_t *ent, dmdl_t *paliashdr)
{
	daliasframe_t	*frame;
	int				*order;
	int				count/*, stripFanCount = 0*/;
	float			color;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface, i;
    shell_t         *pShell; 

	frame = (daliasframe_t *)((byte *)s_pmdl + s_pmdl->ofs_frames 
		+ ent->frame * s_pmdl->framesize);
    
    order = (int *)((byte *)s_pmdl + s_pmdl->ofs_glcmds);
	
    // find the shell
    i=0;
    while (i<gNumAliasShells&&gAliasShells[i].pModel != currentmodel)
        i++;

    if (i>=gNumAliasShells)
        return;
    else
        pShell = gAliasShells+i;

    R_ShellUpdate(pShell);

    while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{	
			order += ( ( 2 ) + 1 ) * count;
			continue;
		}
    	
		qglBegin (primtype);

        do
        {
            index_xyz = *order++;
            order += 2;

            color = min(pShell->pVData[index_xyz], 1.0f);

            qglColor4f(color, color, color, color);
//            qglVertex3fv (lerped[index_xyz]);
            qglVertex3fv (s_lerped[index_xyz]);
        } while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd ();

	}

	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);
	qglEnable (GL_TEXTURE_2D);
}

