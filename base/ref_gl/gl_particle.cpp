#include "gl_local.h"

#include <new>
#include <vector>

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

struct particle_worker_vertex_t
{
	float x, y, z;
	float s, t;
	float r, g, b, a;
};

struct particle_worker_point_t
{
	float x, y, z;
	float r, g, b, a;
};

struct particle_worker_job_t
{
	const particle_t *particles;
	int num_particles;
	const particle_t *comparticles;
	int num_comparticles;
	CVector r_origin;
	CVector vpn;
	CVector vup;
	CVector vright;
	CVector regular_rain_forward;
	CVector regular_rain_right;
	CVector complex_rain_forward;
	CVector complex_rain_right;
	qboolean regular_points;
};

static SDL_Thread *particle_worker_thread = NULL;
static SDL_Semaphore *particle_worker_start = NULL;
static SDL_Semaphore *particle_worker_done = NULL;
static qboolean particle_worker_exit = false;
static qboolean particle_worker_job_active = false;
static qboolean particle_worker_ready = false;
static qboolean particle_worker_failed = false;
static byte particle_worker_regular_rain_generated = FALSE;
static byte particle_worker_complex_rain_generated = FALSE;
static particle_worker_job_t particle_worker_job;
static std::vector<particle_worker_vertex_t> particle_worker_particles;
static std::vector<particle_worker_point_t> particle_worker_points;
static std::vector<particle_worker_vertex_t> particle_worker_comparticles;

static void R_ParticleWorkerPrepareRegularTriangles(void)
{
	int i;
	for (i = 0; i < particle_worker_job.num_particles; ++i)
	{
		const particle_t *p = &particle_worker_job.particles[i];
		const particle_def_t *def = &r_particledefs[p->type];
		CVector up, right, origin, org;
		float scale;
		float alpha;
		particle_worker_vertex_t *v = &particle_worker_particles[i * 3];

		scale = (p->origin.x - particle_worker_job.r_origin.x) * particle_worker_job.vpn.x +
			(p->origin.y - particle_worker_job.r_origin.y) * particle_worker_job.vpn.y +
			(p->origin.z - particle_worker_job.r_origin.z) * particle_worker_job.vpn.z;

		if (p->type == PARTICLE_RAIN)
		{
			up = particle_worker_job.regular_rain_forward;
			right = particle_worker_job.regular_rain_right;

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
			if (p->pscale)
				scale = scale * p->pscale;
			alpha = p->alpha * 1.0 / scale;
		}
		else
		{
			up = particle_worker_job.vup * def->fScale;
			right = particle_worker_job.vright * def->fScale;

			if (scale < 20)
				scale = 1;
			else
				scale = 1 + scale * 0.004;

			if (p->pscale)
				scale = scale * p->pscale;
			alpha = p->alpha;
		}

		if (p->pscale > 1.0)
			scale = p->pscale;
		else
		{
			if (scale < 20)
				scale = 1;
			else
				scale = 1 + scale * 0.004;
		}

		org = (up + right) * scale;
		org.Multiply(0.33);
		origin = p->origin - org;

		v[0].x = origin.x;
		v[0].y = origin.y;
		v[0].z = origin.z;
		v[0].s = def->s1;
		v[0].t = def->t1;
		v[0].r = p->color_rgb.x;
		v[0].g = p->color_rgb.y;
		v[0].b = p->color_rgb.z;
		v[0].a = alpha;

		v[1].x = origin.x + up.x * scale;
		v[1].y = origin.y + up.y * scale;
		v[1].z = origin.z + up.z * scale;
		v[1].s = def->s2;
		v[1].t = def->t1;
		v[1].r = p->color_rgb.x;
		v[1].g = p->color_rgb.y;
		v[1].b = p->color_rgb.z;
		v[1].a = alpha;

		v[2].x = origin.x + right.x * scale;
		v[2].y = origin.y + right.y * scale;
		v[2].z = origin.z + right.z * scale;
		v[2].s = def->s1;
		v[2].t = def->t2;
		v[2].r = p->color_rgb.x;
		v[2].g = p->color_rgb.y;
		v[2].b = p->color_rgb.z;
		v[2].a = alpha;
	}
}

static void R_ParticleWorkerPreparePoints(void)
{
	int i;
	for (i = 0; i < particle_worker_job.num_particles; ++i)
	{
		const particle_t *p = &particle_worker_job.particles[i];
		particle_worker_point_t *v = &particle_worker_points[i];
		v->x = p->origin.x;
		v->y = p->origin.y;
		v->z = p->origin.z;
		v->r = p->color_rgb.x;
		v->g = p->color_rgb.y;
		v->b = p->color_rgb.z;
		v->a = p->alpha;
	}
}

static void R_ParticleWorkerPrepareComplex(const particle_t *particles, int count,
	std::vector<particle_worker_vertex_t> &vertices,
	const CVector &rain_forward_local, const CVector &rain_right_local)
{
	int i;
	for (i = 0; i < count; ++i)
	{
		const particle_t *p = &particles[i];
		const particle_def_t *def = &r_particledefs[p->type];
		CVector up, right, origin, org;
		float scale;
		float alpha;
		particle_worker_vertex_t *v = &vertices[i * 3];

		scale = (p->origin.x - particle_worker_job.r_origin.x) * particle_worker_job.vpn.x +
			(p->origin.y - particle_worker_job.r_origin.y) * particle_worker_job.vpn.y +
			(p->origin.z - particle_worker_job.r_origin.z) * particle_worker_job.vpn.z;

		if (p->type == PARTICLE_RAIN)
		{
			up = rain_forward_local * (def->fScale * 0.8);
			right = rain_right_local * (def->fScale * 0.8);

			if (scale < 10)
				scale = 2.0;
			else
				scale = def->fScale;
			alpha = p->alpha * 1.0 / scale;
		}
		else
		{
			up = particle_worker_job.vup * def->fScale;
			right = particle_worker_job.vright * def->fScale;

			if (p->pscale > 1.0)
				scale = p->pscale;
			else
			{
				if (scale < 20)
					scale = 1;
				else
					scale = 1 + scale * 0.004;
			}
			alpha = p->alpha;
		}

		org = (up + right) * scale;
		org.Multiply(0.33);
		origin = p->origin - org;

		v[0].x = origin.x;
		v[0].y = origin.y;
		v[0].z = origin.z;
		v[0].s = def->s1;
		v[0].t = def->t1;
		v[0].r = p->color_rgb.x;
		v[0].g = p->color_rgb.y;
		v[0].b = p->color_rgb.z;
		v[0].a = alpha;

		v[1].x = origin.x + up.x * scale;
		v[1].y = origin.y + up.y * scale;
		v[1].z = origin.z + up.z * scale;
		v[1].s = def->s2;
		v[1].t = def->t1;
		v[1].r = p->color_rgb.x;
		v[1].g = p->color_rgb.y;
		v[1].b = p->color_rgb.z;
		v[1].a = alpha;

		v[2].x = origin.x + right.x * scale;
		v[2].y = origin.y + right.y * scale;
		v[2].z = origin.z + right.z * scale;
		v[2].s = def->s1;
		v[2].t = def->t2;
		v[2].r = p->color_rgb.x;
		v[2].g = p->color_rgb.y;
		v[2].b = p->color_rgb.z;
		v[2].a = alpha;
	}
}

static int SDLCALL R_ParticleWorkerMain(void *unused)
{
	(void)unused;

	for (;;)
	{
		SDL_WaitSemaphore(particle_worker_start);

		if (particle_worker_exit)
			break;

		particle_worker_failed = false;
		try
		{
			if (particle_worker_job.regular_points)
				R_ParticleWorkerPreparePoints();
			else if (particle_worker_job.num_particles > 0)
				R_ParticleWorkerPrepareRegularTriangles();

			if (particle_worker_job.num_comparticles > 0)
				R_ParticleWorkerPrepareComplex(particle_worker_job.comparticles,
					particle_worker_job.num_comparticles,
					particle_worker_comparticles,
					particle_worker_job.complex_rain_forward,
					particle_worker_job.complex_rain_right);

		}
		catch (...)
		{
			particle_worker_failed = true;
		}

		SDL_SignalSemaphore(particle_worker_done);
	}

	return 0;
}

void R_ParticleWorkerInit(void)
{
	if (particle_worker_thread != NULL)
		return;

	particle_worker_start = SDL_CreateSemaphore(0);
	particle_worker_done = SDL_CreateSemaphore(0);
	if (particle_worker_start == NULL || particle_worker_done == NULL)
	{
		if (particle_worker_start != NULL)
			SDL_DestroySemaphore(particle_worker_start);
		if (particle_worker_done != NULL)
			SDL_DestroySemaphore(particle_worker_done);
		particle_worker_start = NULL;
		particle_worker_done = NULL;
		return;
	}

	particle_worker_exit = false;
	particle_worker_ready = false;
	particle_worker_failed = false;
	particle_worker_job_active = false;
	particle_worker_regular_rain_generated = FALSE;
	particle_worker_complex_rain_generated = FALSE;

	particle_worker_thread = SDL_CreateThread(R_ParticleWorkerMain,
		"DaikatanaParticleWorker", NULL);
	if (particle_worker_thread == NULL)
	{
		SDL_DestroySemaphore(particle_worker_start);
		SDL_DestroySemaphore(particle_worker_done);
		particle_worker_start = NULL;
		particle_worker_done = NULL;
		return;
	}
}

void R_ParticleWorkerShutdown(void)
{
	if (particle_worker_thread == NULL)
		return;

	if (particle_worker_job_active)
		R_ParticleWorkerWait();

	particle_worker_exit = true;
	SDL_SignalSemaphore(particle_worker_start);
	SDL_WaitThread(particle_worker_thread, NULL);
	particle_worker_thread = NULL;

	SDL_DestroySemaphore(particle_worker_start);
	SDL_DestroySemaphore(particle_worker_done);
	particle_worker_start = NULL;
	particle_worker_done = NULL;
	particle_worker_job_active = false;
	particle_worker_ready = false;
}

void R_ParticleWorkerBegin(void)
{
	int total_tri_vertices;

	if (particle_worker_thread == NULL)
		return;
	if (particle_worker_job_active)
		R_ParticleWorkerWait();
	if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) != 0 || !r_drawparticleflag->value)
		return;

	if (r_newrefdef.num_particles == 0 &&
		r_newrefdef.num_comparticles == 0)
		return;

	particle_worker_job.particles = r_newrefdef.particles;
	particle_worker_job.num_particles = (gl_ext_pointparameters->value && qglPointParameterfEXT) ?
		r_newrefdef.num_particles : r_newrefdef.num_particles;
	particle_worker_job.comparticles = r_newrefdef.comparticles;
	particle_worker_job.num_comparticles = r_drawcomparticles->value ? r_newrefdef.num_comparticles : 0;
	particle_worker_job.r_origin = r_origin;
	particle_worker_job.vpn = vpn;
	particle_worker_job.vup = vup;
	particle_worker_job.vright = vright;
	particle_worker_job.regular_points = (gl_ext_pointparameters->value && qglPointParameterfEXT) ? true : false;

	if (!particle_worker_job.regular_points && particle_worker_job.num_particles > 0 &&
		!particle_worker_regular_rain_generated)
	{
		GL_GenerateRainVectors(r_newrefdef.vieworg, 0);
		particle_worker_regular_rain_generated = TRUE;
	}
	particle_worker_job.regular_rain_forward = rain_forward;
	particle_worker_job.regular_rain_right = rain_right;

	if (particle_worker_job.num_comparticles > 0 &&
		!particle_worker_complex_rain_generated)
	{
		GL_GenerateRainVectors(r_newrefdef.vieworg, 0);
		particle_worker_complex_rain_generated = TRUE;
	}
	particle_worker_job.complex_rain_forward = rain_forward;
	particle_worker_job.complex_rain_right = rain_right;

	try
	{
		if (particle_worker_job.regular_points)
			particle_worker_points.resize(particle_worker_job.num_particles);
		else
		{
			total_tri_vertices = particle_worker_job.num_particles * 3;
			particle_worker_particles.resize(total_tri_vertices);
		}

		particle_worker_comparticles.resize(particle_worker_job.num_comparticles * 3);
	}
	catch (const std::bad_alloc &)
	{
		particle_worker_ready = false;
		particle_worker_failed = true;
		particle_worker_job_active = false;
		return;
	}

	particle_worker_ready = true;
	particle_worker_job_active = true;
	SDL_SignalSemaphore(particle_worker_start);
}

void R_ParticleWorkerWait(void)
{
	if (!particle_worker_job_active)
		return;

	SDL_WaitSemaphore(particle_worker_done);
	particle_worker_job_active = false;
	if (particle_worker_failed)
		particle_worker_ready = false;
}

static void R_DrawPreparedTriangles(const std::vector<particle_worker_vertex_t> &vertices,
	int count, qboolean reset_color)
{
	if (count <= 0)
		return;

	qglEnableClientState(GL_VERTEX_ARRAY);
	qglVertexPointer(3, GL_FLOAT, sizeof(particle_worker_vertex_t), &vertices[0].x);

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, sizeof(particle_worker_vertex_t), &vertices[0].s);

	qglEnableClientState(GL_COLOR_ARRAY);
	qglColorPointer(4, GL_FLOAT, sizeof(particle_worker_vertex_t), &vertices[0].r);

	qglDrawArrays(GL_TRIANGLES, 0, count * 3);

	qglDisableClientState(GL_COLOR_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState(GL_VERTEX_ARRAY);

	if (reset_color)
		qglColor4f(1, 1, 1, 1);
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

	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

//	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
    nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
    nStateFlags &= ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT );
	GL_SetState( nStateFlags );

	// set up GL

	GL_TexEnv( GL_MODULATE );
	GL_Bind( r_particletexture->texnum );	
	R_TextureFilterBeginBase(r_particletexture);

	qglBegin( GL_TRIANGLES );

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
	}

	qglEnd ();

	qglColor4f( 1, 1, 1, 1 );
	R_TextureFilterEnd();

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

	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	if (gl_ext_pointparameters->value && qglPointParameterfEXT &&
		(gl_texture_filter == NULL || gl_texture_filter->value != 2.0f))
	{
		int i;

		R_ParticleWorkerWait();
		if (!particle_worker_ready || particle_worker_job.num_particles != r_newrefdef.num_particles)
		{
			GL_SetState(GLSTATE_BLEND | GLSTATE_DEPTH_TEST);
			qglPointSize(gl_particle_size->value);
			qglBegin(GL_POINTS);
			for (i = 0; i < r_newrefdef.num_particles; ++i)
			{
				const particle_t *p = &r_newrefdef.particles[i];
				qglColor4f(p->color_rgb.x, p->color_rgb.y, p->color_rgb.z, p->alpha);
				qglVertex3fv(p->origin);
			}
			qglEnd();
			return;
		}

		GL_SetState(GLSTATE_BLEND | GLSTATE_DEPTH_TEST);
		qglPointSize(gl_particle_size->value);
		qglEnableClientState(GL_VERTEX_ARRAY);
		qglVertexPointer(3, GL_FLOAT, sizeof(particle_worker_point_t), &particle_worker_points[0].x);
		qglEnableClientState(GL_COLOR_ARRAY);
		qglColorPointer(4, GL_FLOAT, sizeof(particle_worker_point_t), &particle_worker_points[0].r);
		qglDrawArrays(GL_POINTS, 0, particle_worker_job.num_particles);
		qglDisableClientState(GL_COLOR_ARRAY);
		qglDisableClientState(GL_VERTEX_ARRAY);
	}
	else
	{
		R_ParticleWorkerWait();
		if (!particle_worker_ready || particle_worker_job.num_particles != r_newrefdef.num_particles)
		{
			GL_DrawParticles(r_newrefdef.num_particles, r_newrefdef.particles);
			return;
		}

		{
			unsigned int nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
			nStateFlags &= ~GLSTATE_DEPTH_MASK;
			GL_SetState(nStateFlags);
		}
		GL_TexEnv(GL_MODULATE);
		GL_Bind(r_particletexture->texnum);
		R_TextureFilterBeginBase(r_particletexture);
		R_DrawPreparedTriangles(particle_worker_particles, particle_worker_job.num_particles, false);
		R_TextureFilterEnd();
		GL_TexEnv(GL_REPLACE);
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

	if (r_newrefdef.num_comparticles == 0)
		return;

	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	R_ParticleWorkerWait();
	if (!particle_worker_ready || particle_worker_job.num_comparticles != r_newrefdef.num_comparticles)
	{
		GL_DrawComplexParticles(r_newrefdef.num_comparticles, r_newrefdef.comparticles);
		return;
	}

	{
		unsigned int nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
		nStateFlags &= ~(GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT);
		GL_SetState(nStateFlags);
	}
	GL_TexEnv(GL_MODULATE);
	GL_Bind(r_particletexture->texnum);
	R_TextureFilterBeginBase(r_particletexture);
	R_DrawPreparedTriangles(particle_worker_comparticles, particle_worker_job.num_comparticles, true);
	R_TextureFilterEnd();
	GL_TexEnv(GL_REPLACE);
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

	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

//	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_TEST );
    nStateFlags = GLSTATE_PRESET1 | GLSTATE_BLEND;
    nStateFlags &= ~GLSTATE_DEPTH_MASK;
	GL_SetState( nStateFlags );

	GL_TexEnv( GL_MODULATE );

	GL_Bind(r_particletexture->texnum);
	R_TextureFilterBeginBase(r_particletexture);

	qglBegin( GL_TRIANGLES );

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
	}

	qglEnd ();
	R_TextureFilterEnd();
	GL_TexEnv( GL_REPLACE );
}
