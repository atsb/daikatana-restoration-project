#include "gl_local.h"

particle_def_t	r_particledefs[NUM_PARTICLETYPES];
int	r_num_particle_defs;

static		CVector	rain_forward;
static		CVector	rain_right;
static		CVector	rain_up;

/*
==================
R_InitParticleTexture
==================
*/
byte	dottexture[8][8] =
{
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,0,0,0,0},
	{0,1,1,1,1,0,0,0},
	{0,1,1,1,1,0,0,0},
	{0,0,1,1,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

void R_InitNoTexture()
{
	int		x,y;
	byte	data[8][8][4];

	for (x=0 ; x<8 ; x++)
	{
		for (y=0 ; y<8 ; y++)
		{
			data[y][x][0] = dottexture[x&3][y&3]*255;
			data[y][x][1] = 0;
			data[y][x][2] = 0;
			data[y][x][3] = 255;
		}
	}

	r_notexture = GL_LoadPic ("***r_notexture***", (byte *)data, 8, 8, it_wall, 32, NULL);

	r_notexture->resource = RESOURCE_GLOBAL;
}

void R_InitParticles()
{
	particle_def_t	*p;
	float			w, h;
	int				i;

	r_particletexture = R_FindImage( "pics/particles/particles.tga", it_sprite, RESOURCE_GLOBAL );

	if( r_particletexture == NULL )
	{
		for( i = 0; i < NUM_PARTICLETYPES; i++ )
		{
			r_particledefs[i].s1 = 1.0 / ( r_notexture->width * 2 );
			r_particledefs[i].s2 = 1 + r_particledefs[i].s1;
			r_particledefs[i].t1 = 1.0 / ( r_notexture->height * 2 );
			r_particledefs[i].t2 = 1 + r_particledefs[i].t1;
		}

		r_particletexture = r_notexture;

		return;
	}

	w = r_particletexture->width;
	h = r_particletexture->height;

	p = &r_particledefs[PARTICLE_SNOW];
	p->s1 = 0.0;	p->t1 = 0.0;	p->s2 = 63.0;	p->t2 = 63.0;	p->fScale = 2.3;
	p = &r_particledefs[PARTICLE_RAIN];
//	p->s1 = 0.0;	p->t1 = 96.0;	p->s2 = 31.0;	p->t2 = 127.0;	p->fScale = 2.0;
	p->s1 = 225.0;	p->t1 = 32.0;	p->s2 = 255.0;	p->t2 = 63.0;	p->fScale = 2.0;
	p = &r_particledefs[PARTICLE_BLOOD];
	p->s1 = 112.0;	p->t1 = 96.0;	p->s2 = 127.0;	p->t2 = 111.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BLOOD2];
	p->s1 = 128.0;	p->t1 = 96.0;	p->s2 = 143.0;	p->t2 = 111.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BLOOD3];
	p->s1 = 144.0;	p->t1 = 96.0;	p->s2 = 159.0;	p->t2 = 111.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BLOOD4];
	p->s1 = 112.0;	p->t1 = 112.0;	p->s2 = 127.0;	p->t2 = 127.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BLOOD5];
	p->s1 = 128.0;	p->t1 = 112.0;	p->s2 = 143.0;	p->t2 = 127.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BUBBLE];
	p->s1 = 0.0;	p->t1 = 64.0;	p->s2 = 31.0;	p->t2 = 95.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_SMOKE];
	p->s1 = 64.0;	p->t1 = 0.0;	p->s2 = 127.0;	p->t2 = 64.0;	p->fScale = 7.0;
	p = &r_particledefs[PARTICLE_SPARKS];
	p->s1 = 144.0;	p->t1 = 112.0;	p->s2 = 159.0;	p->t2 = 127.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_POISON];
	p->s1 = 128.0;	p->t1 = 0.0;	p->s2 = 127.0;	p->t2 = 63.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_BLUE_SPARKS];
	p->s1 = 96.0;	p->t1 = 112.0;	p->s2 = 111.0;	p->t2 = 127.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_ICE];
	p->s1 = 192.0;	p->t1 = 64.0;	p->s2 = 255.0;	p->t2 = 127.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_SPARKLE1];
	p->s1 = 32.0;	p->t1 = 96.0;	p->s2 = 63.0;	p->t2 = 127.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_SPARKLE2];
	p->s1 = 64.0;	p->t1 = 96.0;	p->s2 = 95.0;	p->t2 = 127.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_DRIP_BLOOD];
	p->s1 = 96.0;	p->t1 = 96.0;	p->s2 = 111.0;	p->t2 = 111.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_SPLASH1];
	p->s1 = 96.0;	p->t1 = 64.0;	p->s2 = 127.0;	p->t2 = 91.0;	p->fScale = 4.0;
	p = &r_particledefs[PARTICLE_SPLASH2];
	p->s1 = 128.0;	p->t1 = 64.0;	p->s2 = 159.0;	p->t2 = 91.0;	p->fScale = 4.0;
	p = &r_particledefs[PARTICLE_SPLASH3];
	p->s1 = 160.0;	p->t1 = 64.0;	p->s2 = 191.0;	p->t2 = 91.0;	p->fScale = 4.0;
	p = &r_particledefs[PARTICLE_FIRE];
	p->s1 = 192.0;	 p->t1 = 0.0;	p->s2 = 255.0;	p->t2 = 63.0;	p->fScale = 3.0;
	p = &r_particledefs[PARTICLE_CRYOSPRAY];
	p->s1 = 160.0;	p->t1 = 96.0;	p->s2 = 191.0;	p->t2 = 127.0;	p->fScale = 1.0;
	p = &r_particledefs[PARTICLE_SPARK_1];
	p->s1 = 32.0;	p->t1 = 64.0;	p->s2 = 63.0;	p->t2 = 95.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_SPARK_2];
	p->s1 = 64.0;	p->t1 = 64.0;	p->s2 = 95.0;	p->t2 = 95.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_SIMPLE];
	p->s1 = 48.0;	p->t1 = 48.0;	p->s2 = 63.0;	p->t2 = 63.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_CP1];
	p->s1 = 96.0;	p->t1 = 32.0;	p->s2 = 127.0;	p->t2 = 63.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_CP2];
	p->s1 = 160.0;	p->t1 = 32.0;	p->s2 = 191.0;	p->t2 = 63.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_CP3];
	p->s1 = 224.0;	p->t1 = 32.0;	p->s2 = 255.0;	p->t2 = 63.0;	p->fScale = 1.5;
	p = &r_particledefs[PARTICLE_CP4];
	p->s1 = 224.0;	p->t1 = 96.0;	p->s2 = 255.0;	p->t2 = 127.0;	p->fScale = 1.5;

	r_particledefs[PARTICLE_BIG_SPARKS]			= r_particledefs[PARTICLE_SPARKS];
	r_particledefs[PARTICLE_DRIP_SLUDGE]		= r_particledefs[PARTICLE_DRIP_BLOOD];
	r_particledefs[PARTICLE_DRIP_WATER]			= r_particledefs[PARTICLE_DRIP_BLOOD];
	r_particledefs[PARTICLE_BEAM_SPARKS]		= r_particledefs[PARTICLE_SPARKS];
	r_particledefs[PARTICLE_FIREFLY]			= r_particledefs[PARTICLE_FIRE];

	r_particledefs[PARTICLE_BIG_SPARKS].fScale	= 3.0;
	r_particledefs[PARTICLE_FIREFLY].fScale		= 3.0;
	r_particledefs[PARTICLE_SIMPLE].fScale		= 1.0;
	
	for( i = 0; i < NUM_PARTICLETYPES; i++ )
	{
		r_particledefs[i].s1 /= w;
	    r_particledefs[i].s2 /= w;
	    r_particledefs[i].t1 /= h;
	    r_particledefs[i].t2 /= h;

		// SCG[6/22/99]: This indexes halfway into a pixel
		// SCG[6/22/99]: Carmack always added 0.0625 because his images were 8 pixels wide
		// SCG[6/22/99]: 8 pixels is 16 half-pixels and 1/16 is 0.0625
		// SCG[6/22/99]: so basicaly we end up with the following
	    r_particledefs[i].s1 += 1.0 / ( w * 2.0 );
	    r_particledefs[i].s2 += 1.0 / ( w * 2.0 );
	    r_particledefs[i].t1 += 1.0 / ( h * 2.0 );
	    r_particledefs[i].t2 += 1.0 / ( h * 2.0 );
	}
}

void GL_GenerateRainVectors (const CVector &p_origin, int type)
{
	float dist;
	CVector viewNormal, endPt, pNormal, rainVel, newPt;
	int i = 0;

	rainVel.x = rainVel.y = 0;
	rainVel.z = -400;

	if (type & PF_RAIN_NORTH)
		rainVel.y = 300;
	else 
	if (type & PF_RAIN_SOUTH)
		rainVel.y = -300;
	else 
	if (type & PF_RAIN_EAST)
		rainVel.x = 300;
	else 
	if (type & PF_RAIN_WEST)
		rainVel.x = -300;

	newPt = vpn * 5.0;
	newPt = newPt + r_origin;

	// calculate where the particle is going to end up in one second's time
	endPt = newPt + rainVel;
	// get a vector of the rain drop's direction
	pNormal = endPt - newPt;
	// get a vector from the view position to the particle
	viewNormal = newPt - r_origin;

	//	get vector perpendicular to view and rain direction
	CrossProduct (viewNormal, pNormal, rain_right);

	dist = rain_right.Length();
	if (dist < 1.001)
		return;

	//	normalize it and flip it
	rain_right = rain_right * (-1.0 / dist);
	rain_right = rain_right * .8;
	pNormal.Normalize();
	rain_forward = pNormal * 20;
}

///////////////////////////////////////////////////////////////////////////////
//	GL_DrawComplexParticles
//
///////////////////////////////////////////////////////////////////////////////
void GL_DrawComplexParticles( int num_particles, const particle_t particles[] )
{
	const			particle_t *p;
	int				i;
	CVector			up, right, origin, org;
	float			scale, alpha;
	static byte		bRainVectorsGenerated = FALSE;
    unsigned int    nStateFlags;

	// easy out...
	if (num_particles == 0)
		return;

	if (!bRainVectorsGenerated)
	{
		GL_GenerateRainVectors (r_newrefdef.vieworg, 0);
		bRainVectorsGenerated = TRUE;
	}

//	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
    nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
    nStateFlags &= ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT );
	GL_SetState( nStateFlags );

	// set up GL

	GL_TexEnv( GL_MODULATE );
	GL_Bind( r_particletexture->texnum );	

	for ( p = particles, i=0 ; i < num_particles ; i++,p++)
	{
		scale = ( p->origin.x - r_origin.x ) * vpn.x + 
				( p->origin.y - r_origin.y ) * vpn.y +
				( p->origin.z - r_origin.z ) * vpn.z;

		if (p->type == PARTICLE_RAIN)
		{
			// we only want this run on the first rain particle 
			// PHM
//			up = rain_forward * r_particledefs[p->type].fScale * 0.8;
//			right = rain_right * r_particledefs[p->type].fScale * 0.8;
			up = rain_forward * (r_particledefs[p->type].fScale * 0.8);
			right = rain_right * (r_particledefs[p->type].fScale * 0.8);

			if( scale < 10 )
			{
				scale = 2.0;
			}
			else
			{
				scale = r_particledefs[p->type].fScale;
			}
			alpha = p->alpha * 1.0/scale;
		}
		else
		{
			up = vup * r_particledefs[p->type].fScale;
			right = vright * r_particledefs[p->type].fScale;

			if( p->pscale > 1.0 )
			{
//				scale = scale * p->pscale;	// SCG[11/15/99]: remove per John's request
				scale = p->pscale;
			}
			else
			{
				if (scale < 20)
					scale = 1;
				else
					scale = 1 + scale * 0.004;
			}
			alpha = p->alpha;
		}

//scale *= 1.5;

		qglBegin( GL_TRIANGLES );
		qglColor4f( p->color_rgb.x, p->color_rgb.y, p->color_rgb.z, alpha );

		// SCG[11/15/99]: Begin addidtion for centering particle
		org = ( up + right ) * scale;
		org.Multiply( 0.33 );
		origin = p->origin - org;
		// SCG[11/15/99]: End addidtion for centering particle

		qglTexCoord2f( r_particledefs[p->type].s1, r_particledefs[p->type].t1 );
		qglVertex3fv( origin );

		qglTexCoord2f( r_particledefs[p->type].s2, r_particledefs[p->type].t1 );
		qglVertex3f( origin.x + up.x * scale, 
					 origin.y + up.y * scale, 
					 origin.z + up.z * scale);

		qglTexCoord2f( r_particledefs[p->type].s1, r_particledefs[p->type].t2 );
		qglVertex3f( origin.x + right.x * scale, 
					 origin.y + right.y * scale, 
					 origin.z + right.z * scale);
		qglEnd ();
	}

	qglColor4f( 1, 1, 1, 1 );

	GL_TexEnv (GL_REPLACE);
}

/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		return;
	}

	if (!r_drawparticleflag->value)
	{
		return; 
	}

	if( r_newrefdef.num_particles == 0 )
	{
		return;
	}

	if ( gl_ext_pointparameters->value && qglPointParameterfEXT )
	{
		int i;
		const particle_t *p;

		GL_SetState( GLSTATE_BLEND | GLSTATE_DEPTH_TEST );

		qglPointSize( gl_particle_size->value );

		qglBegin( GL_POINTS );
		for ( i = 0, p = r_newrefdef.particles; i < r_newrefdef.num_particles; i++, p++ )
		{
			qglColor4f( p->color_rgb.x, p->color_rgb.y, p->color_rgb.z, p->alpha );

			qglVertex3fv( p->origin );
		}
		qglEnd();

	}
	else
	{
		GL_DrawParticles( r_newrefdef.num_particles, r_newrefdef.particles );
	}
}


/*
============
R_DrawComplexParticles
============
*/
void R_DrawComplexParticles (void)
{
	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		return;
	}

	if (!r_drawparticleflag->value || !r_drawcomparticles->value)
		return; 

	GL_DrawComplexParticles( r_newrefdef.num_comparticles, r_newrefdef.comparticles );
}


/*
** GL_DrawParticles
**
*/
void GL_DrawParticles( int num_particles, const particle_t particles[] )
{
	const particle_t *p;
	int				i;
	CVector			up, right, origin, org;
	float			scale, alpha;
    unsigned int    nStateFlags;
	static byte		bRainVectorsGenerated = FALSE;

	if (num_particles == 0)
		return;

	if (!bRainVectorsGenerated)
	{
		GL_GenerateRainVectors (r_newrefdef.vieworg, 0);
		bRainVectorsGenerated = TRUE;
	}

//	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
    nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
    nStateFlags &= ~GLSTATE_DEPTH_MASK;
	GL_SetState( nStateFlags );

	GL_TexEnv( GL_MODULATE );

	GL_Bind(r_particletexture->texnum);

	for ( p = particles, i=0 ; i < num_particles ; i++,p++)
	{
		// hack a scale up to keep particles from disapearing
		scale = ( p->origin.x - r_origin.x ) * vpn.x + 
			    ( p->origin.y - r_origin.y ) * vpn.y +
			    ( p->origin.z - r_origin.z ) * vpn.z;

		if (p->type == PARTICLE_RAIN)
		{
			// we only want this run on the first rain particle 
			up = rain_forward;
			right = rain_right;

			if (scale < 0)
			{
				if (scale > -20)
					scale = -1;
				else
					scale = -1 + scale * 0.004;
			}
			else
			{
				if (scale < 20)
					scale = 1;
				else
					scale = 1 + scale * 0.004;
			}
			if( p->pscale )
			{
				scale = scale * p->pscale;
			}
			alpha = p->alpha * 1.0/scale;
		}
		else
		{
			up = vup * r_particledefs[p->type].fScale;
			right = vright * r_particledefs[p->type].fScale;

			if (scale < 20)
				scale = 1;
			else
				scale = 1 + scale * 0.004;

			if( p->pscale )
			{
				scale = scale * p->pscale;
			}
			alpha = p->alpha;
		}

		if( p->pscale > 1.0 )
		{
//			scale = scale * p->pscale;	// SCG[11/15/99]: remove per John's request
			scale = p->pscale;
		}
		else
		{
			if (scale < 20)
				scale = 1;
			else
				scale = 1 + scale * 0.004;
		}

//		scale *= 1.5;

		qglBegin( GL_TRIANGLES );
		qglColor4f( p->color_rgb.x, p->color_rgb.y, p->color_rgb.z, alpha );

		// SCG[11/15/99]: Begin addidtion for centering particle
		org = ( up + right ) * scale;
		org.Multiply( 0.33 );
		origin = p->origin - org;
		// SCG[11/15/99]: End addidtion for centering particle

		qglTexCoord2f( r_particledefs[p->type].s1, r_particledefs[p->type].t1 );
		qglVertex3fv( origin );

		qglTexCoord2f( r_particledefs[p->type].s2, r_particledefs[p->type].t1 );
		qglVertex3f( origin.x + up.x * scale, 
					 origin.y + up.y * scale, 
					 origin.z + up.z * scale);

		qglTexCoord2f( r_particledefs[p->type].s1, r_particledefs[p->type].t2 );
		qglVertex3f( origin.x + right.x * scale, 
					 origin.y + right.y * scale, 
					 origin.z + right.z * scale);
		qglEnd ();
	}

	GL_TexEnv( GL_REPLACE );
}
