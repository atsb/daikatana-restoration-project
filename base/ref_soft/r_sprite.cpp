// r_sprite.c
#include "r_local.h"
#include "r_protex.h"

extern polydesc_t r_polydesc;

void R_BuildPolygonFromSurface(msurface_t *fa);
void R_PolygonCalculateGradients (void);

extern void R_PolyChooseSpanletRoutine( float alpha, qboolean isturbulent );

//extern vec5_t r_clip_verts[2][MAXWORKINGVERTS+2];

extern void	R_ClipAndDrawPoly( float alpha, qboolean isturbulent, qboolean textured );

/*
** R_DrawSprite
**
** Draw currententity / currentmodel as a single texture
** mapped polygon
*/
void R_DrawSprite (void)
{
	int			i;
	CVector		left, up, right, down;
	dsprite_t	*s_psprite;
	dsprframe_t	*s_psprframe;
	CVector		v_pn, v_right, v_up;
	CVector		p0, p1, p2, p3; 
	image_t		*image;
//	byte		*pixels;
	uint16		*pixels;
	float		alpha, scale_x, scale_y;

	image = currentmodel->skins[currententity->frame];
	alpha = currententity->alpha;
	alpha = 1;

	s_psprite = (dsprite_t *)currentmodel->extradata;
	currententity->frame %= s_psprite->numframes;

	s_psprframe = &s_psprite->frames[currententity->frame];

	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawSprite: NULL skin\n");
		ri.Con_Printf (PRINT_ALL, "model: %s\n", currentmodel->name);
		ri.Con_Printf (PRINT_ALL, "frame: %d\n", currententity->frame);
		return;
	}

	// generate the sprite's axes, completely parallel to the viewplane.
	if (currententity->flags & SPR_ORIENTED)
	{
		AngleToVectors (currententity->angles, v_pn, v_right, v_up);
		r_polydesc.vup = v_up;
		r_polydesc.vright = v_right;
	}
	else
	{
		r_polydesc.vup = vup;
		r_polydesc.vright = vright;
	}

	scale_x = currententity->render_scale[0];
	scale_y = currententity->render_scale[1];

/*
	if (image->type == it_procedural)
	{
		protex_RunProceduralTexture (&protex_textures[image->pt_type]);
//		pixels = (unsigned short*)protex_textures[image->pt_type].data;
		pixels = (byte*)protex_textures[image->pt_type].data;
		scale_x /= 4;
		scale_y /= -4;
	}
	else
*/		pixels = (uint16*) image->pixels[0];

	// build local sprite points
	//VectorScale (r_polydesc.vright, s_psprframe->width - s_psprframe->origin_x, right);
	right = r_polydesc.vright * (s_psprframe->width - s_psprframe->origin_x);
	//VectorScale (r_polydesc.vup, s_psprframe->height - s_psprframe->origin_y, up);
	up = r_polydesc.vup * (s_psprframe->height - s_psprframe->origin_y);
	//VectorScale (r_polydesc.vright, -s_psprframe->origin_x, left);
	left = r_polydesc.vright * -s_psprframe->origin_x;
	//VectorScale (r_polydesc.vup, -s_psprframe->origin_y, down);
	down = r_polydesc.vup * -s_psprframe->origin_y;

	//VectorScale (r_polydesc.vup, 1/scale_y, r_polydesc.vup);
	r_polydesc.vup = r_polydesc.vup * (1/scale_y);
	//VectorScale (r_polydesc.vright, 1/scale_x, r_polydesc.vright);
	r_polydesc.vright = r_polydesc.vright * (1/scale_x);

 	// transform points into world space
	for (i = 0; i < 3; i++)
	{
		p0[i] = r_entorigin[i] + up[i] * scale_y + left[i] * scale_x;
		p1[i] = r_entorigin[i] + up[i] * scale_y + right[i] * scale_x;
		p2[i] = r_entorigin[i] + down[i] * scale_y + right[i] * scale_x;
		p3[i] = r_entorigin[i] + down[i] * scale_y + left[i] * scale_x;
	}

	if (currententity->flags & SPR_ALPHACHANNEL)
	{
		currententity->alpha = 0.66;
		currententity->flags |= RF_TRANSLUCENT;
	}

	R_TexturedQuad (p0, p1, p2, p3, image, alpha);

	r_polydesc.stipple_parity = 0;
}

