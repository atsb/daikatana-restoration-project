//////////////////////////////////////////////////////////////////////////////
// cl_pv.c
//
// - source file to handle particle volumes
//
//////////////////////////////////////////////////////////////////////////////

#include "client.h"

#define	PV_CLIP_DISTANCE		1024
#define	PV_HALF_CLIP_DISTANCE	512

// for effect_rain particle volumes
#define RAIN_NORTH	0x00000001	// it will rain to the north
#define RAIN_SOUTH	0x00000002	// it will rain to the south
#define RAIN_EAST	0x00000004	// it will rain to the east
#define RAIN_WEST	0x00000008	// it will rain to the west

#define SNOW_STRAIGHT 0x00000001

///////////////////////////////////
// extern definitions
///////////////////////////////////
extern		int					r_numparticleVolumes;						// number of particle volumes
extern		particleVolume_t	r_particleVolumes[MAX_PARTICLEVOLUMES];		// array of particle volume records
//extern		struct				model_s	*cl_mod_rainSplash;					// splash for rain drops

///////////////////////////////////
// global variable definitions
///////////////////////////////////

//static	int	snow_rotation = 0;
cpvolume_t	cl_pvolumes[MAX_PVOLUMES];
int			cl_numpvolumes;
//float CL_ParticleSplashTime(CVector vecOrg, float fZVelocity, float fAcceleration);

///////////////////////////////////
// prototypes
///////////////////////////////////

///////////////////////////////////
// function definitions
///////////////////////////////////

/*
===============
CL_SnowParticles
===============
*/
void CL_SnowParticles (int type, int flags, cpvolume_t *volume, CVector &p1, CVector &p2)
{
	int		i, count;
	int		width, depth, height;
	int		fall_height;
	cparticle_t	*p;

	width = (int) p2.x - p1.x;
	depth = (int) p2.y - p1.y;
	height = (int) p2.z - p1.z;
	
	if (!height)
		height = 10;

	//	a 256x256 square will have a density of 16 particles
	// volume->pv_max = (width * depth) / 4096;	
    volume->pv_max = (width * depth) >> 12;
	//	a 256x256x256 cube will have a density of 16 particles
	//	volume->pv_max = ((double)width * (double)depth * (double)height) / 2097152;

	if (volume->pv_active >= volume->pv_max)
		return;

	if (volume->pv_active < volume->pv_max >> 2)//volume->pv_min)
		//	Nelno:	fill the particle volume if it has few particles!
		count = volume->pv_max >> 1;
	else
		count = 1;

	// make sure we don't generate more particles then the volume maxes out at
	if ((count + volume->pv_active) > volume->pv_max)
		count = volume->pv_max - volume->pv_active;

	if( flags & SNOW_STRAIGHT )
	{
		for (i=0 ; i < count ; i++)
		{
			if (!free_comparticles)
				return;
			p = free_comparticles;
			free_comparticles = p->next;
			p->next = active_comparticles;
			active_comparticles = p;
			p->color_rgb.Set( 1, 1, 1 );
//			p->color = 13;

			p->org.x = p1.x + (rand() % width );
			p->org.y = p1.y + (rand() % depth );
			p->org.z = p1.z + (rand() % height );

			p->vel.x = 0;
			p->vel.y = 0;
			p->vel.z = -50 - (rand () & 10);
			
			//	no acceleration via gravity
			p->accel.Zero();

			//	time particle was spawned
			p->time = cl.time;

			//	actual distance particle will fall
			fall_height = height - (p2.z - p->org.z);
			//	time particle should die
			p->die_time = cl.time - ((fall_height / p->vel.z) * 1000);

			p->flags = 0;
			p->alpha = 1.0;
			p->alphavel = 1.0;

			p->volume = volume;
			p->volume->pv_active++;

			//	Nelno: added for testing complex particles
			p->type = PARTICLE_SNOW;
			
			p->pscale = (rand () / RAND_MAX) * 2.5 + 1.0;
		}
	}
	else
	{
		for (i=0 ; i < count ; i++)
		{
			if (!free_comparticles)
				return;
			p = free_comparticles;
			free_comparticles = p->next;
			p->next = active_comparticles;
			active_comparticles = p;
			p->color_rgb.Set( 1, 1, 1 );
//			p->color = 13;

			p->org.x = p1.x + (rand() % width );
			p->org.y = p1.y + (rand() % depth );
			p->org.z = p1.z + (rand() % height );

			p->vel.x = rand () % 40 - 20;
			p->vel.y = rand () % 40 - 20;
			p->vel.z = -50 - (rand () & 10);
			
			//	no acceleration via gravity
			p->accel.Zero();

			//	time particle was spawned
			p->time = cl.time;

			//	actual distance particle will fall
			fall_height = height - (p2.z - p->org.z);
			//	time particle should die
			p->die_time = cl.time - ((fall_height / p->vel.z) * 1000);

			p->flags = 0;
			p->alpha = 1.0;
			p->alphavel = 1.0;

			p->volume = volume;
			p->volume->pv_active++;

			//	Nelno: added for testing complex particles
			p->type = PARTICLE_SNOW;
			
			p->pscale = (rand () / RAND_MAX) * 2.5 + 1.0;
		}
	}
}

/*
===============
CL_RainParticles
===============
*/
void CL_RainParticles (int type, cpvolume_t *volume, CVector &p1, CVector &p2, float dist)
{
	int			i, count, splashSprites = 0, particleFlags = 0;
	cparticle_t	*p;
	int			width, depth, height;
	CVector		velocity, pos;
	CVector		angles( -1,-1,-1 );	// angles is oriented towards player
	CVector		color( 0,0,0 );		// no color

	velocity.x = 0;
	count = 1;

	width = (int) p2.x - p1.x;
	depth = (int) p2.y - p1.y;
	height = (int) p2.z - p1.z;

	// let's just make sure we don't crash on map errors
	if (width <= 0 || depth <= 0 || height <= 0)
		return;

	// type stores what direction the rain particle should fall
	// the default is straight now.
	velocity.x = velocity.y = 0;
	velocity.z = -400;

	if (type & RAIN_NORTH)
	{
		velocity.y = 300;
		particleFlags = PF_RAIN_NORTH;
	}
	else 
	if (type & RAIN_SOUTH)
	{
		velocity.y = -300;
		particleFlags = PF_RAIN_SOUTH;
	}
	else 
	if (type & RAIN_EAST)
	{
		velocity.x = 300;
		particleFlags = PF_RAIN_EAST;
	}
	else 
	if (type & RAIN_WEST)
	{
		velocity.x = -300;
		particleFlags = PF_RAIN_WEST;
	}

	// volumes that are closer get full density rain... ones farther
	// than half the clip distance get half density rain
	if (dist < PV_HALF_CLIP_DISTANCE)
		// volume->pv_max = (width * depth) / 100;  //50
        volume->pv_max = (width * depth) * 0.01;
	else
		// volume->pv_max = (width * depth) / 200;	 //100
        volume->pv_max = (width * depth) * 0.005;
	if (volume->pv_active >= volume->pv_max)
		return;

	if (volume->pv_active < volume->pv_max >> 2)
		//	Nelno:	fill the particle volume if it has few particles!
		count = volume->pv_max >> 2;
	else
		count = 1;

	// make sure we don't generate more particles then the volume maxes out at
	if ((count + volume->pv_active) > volume->pv_max)
		count = volume->pv_max - volume->pv_active;

	// add the rain particles
	for (i=0 ; i < count ; i++)
	{
		if (!free_comparticles)
			return;

		p = free_comparticles;
		free_comparticles = p->next;
		p->next = active_comparticles;
		active_comparticles = p;

//		p->color = 7 + (rand()&7);

		p->org.x = p1.x + ( (rand() % width ) - 8); 
		p->org.y = p1.y + ( (rand() % depth ) - 8); 
		p->org.z = p1.z + ( (rand() % height ) - 8);

		p->vel = velocity;

		//	actual distance particle will fall
		float fall_height = height - (p2.z - p->org.z);
		p->time = cl.time;
		//	time particle should die
		p->die_time = cl.time - ((fall_height / p->vel.z) * 1000);

		//	no acceleration via gravity
		p->accel.Zero();

		p->alpha = .4;
		p->alphavel = -0.1;

		p->volume = volume;
		p->volume->pv_active++;

		// set the particle type and the directional flags
		p->type = PARTICLE_RAIN;
		p->flags |= particleFlags;
		p->color_rgb.Set( 1, 1, 1 );
//		p->pscale = 1;
	}

	// add the splash sprites (again, make closer particle volumes more dense)
	if (dist < PV_HALF_CLIP_DISTANCE)
		// splashSprites = (width * depth) / 8192; 
        splashSprites = (width * depth) >> 13; 
	else
		// splashSprites = (width * depth) / 16384; 
        splashSprites = (width * depth) >> 14; 

	// add the rain particle splashes
	for (i=0 ; i < splashSprites ; i++)
	{
		if (!free_comparticles)
			return;

		pos.x	= p1.x + (rand() % width ); 
		pos.y	= p1.y + (rand() % depth ); 
		pos.z	= p1.z;

		p = free_comparticles;
		free_comparticles = p->next;
		p->next = active_comparticles;
		active_comparticles = p;

		p->org = pos;

		// not associated with a  particle volume
		p->volume = NULL;
		//	no acceleration via gravity or velocity
		p->accel.Zero();
		p->vel.Zero();

		// should be transparent.. the particle is colored properly for water
//		p->color = 255;

		//	time particle should die
		p->die_time = cl.time + 101;

		p->flags = 0;
		p->alpha = .40;
		p->alphavel = -0.1;

		// p->type = PARTICLE_SPLASH1 + (rand()%3);
        p->type = PARTICLE_SPLASH1 + (rand()&2);
		p->color_rgb.Set( 1, 1, 1 );
		p->pscale = 1;
	}
}

/*
===============
CL_AddDripSplash
===============
*/

void CL_AddDripSplash (cparticle_t *p)
{
	int		oldType = p->type;
	float	time, time2;
	CVector	org, color_rgb;
	
	if (!free_comparticles)
		return;

	time = ((cl.time - p->time)*0.001);// - cls.frametime;
	time2 = time*time;
	org.x = p->org.x + p->vel.x*time + (p->accel.x*time2 * 0.5);
	org.y = p->org.y + p->vel.y*time + (p->accel.y*time2 * 0.5); 
	org.z = p->org.z + p->vel.z*time + (p->accel.z*time2 * 0.5);
    color_rgb = p->color_rgb;

	// make the splash where the drip is about to die
	p->org = org;
	// not associated with a  particle volume
	p->volume = NULL;
	//	no acceleration via gravity or velocity
	p->accel.Zero();
	p->vel.Zero();

	//	time particle should die


	// do this so we don't get the infinite splash :)
	p->flags = 0;
	// use a random splash particle
	p->type = PARTICLE_SPLASH1 + (rand()%3);

	switch (oldType)
	{
        case PARTICLE_BLOOD:
        case PARTICLE_BLOOD2:
        case PARTICLE_BLOOD3:
        case PARTICLE_BLOOD4:
        case PARTICLE_BLOOD5:
		case PARTICLE_DRIP_BLOOD:
        	p->die_time = cl.time + 101;
			p->color_rgb.Set( 1.0f, 0.0f, 0.0f );
          	p->alpha = 1.0 - (0.5 * frand());
            p->pscale = 3.0 * (frand() + 0.5);
            p->alphavel = -1.0;
			break;

		case PARTICLE_DRIP_SLUDGE:
			p->color_rgb.Set( 1, 1, 1 );
			break;
		case PARTICLE_DRIP_WATER:
		default:
            p->die_time = cl.time + 101;
			p->color_rgb = color_rgb;
        	p->alpha = 1.0 - (0.5 * frand());
        	//p->alphavel = -1.0;
            p->pscale = 2.0 * frand() + 1.0;
			break;

	}

	return;
}



/*
===============
CL_DripParticles
===============
*/
void CL_DripParticles (int type, cpvolume_t *volume, CVector &p1, CVector &p2)
{
	int			i, count;
	int			width, depth, height;
	qboolean	bSingleDrip = FALSE;	
	cparticle_t	*p;
    float       fDieTime;

	width = (int) p2.x - p1.x;
	depth = (int) p2.y - p1.y;
	height = (int) p2.z - p1.z;
	
	if (!height)
		height = 10;
	
	//	a 16x16 (or less) square will have a density of 1 particles i.e. just a drip
	//  a larger drip brush has the volume of a snow brush / 2
	if (width <= 16 && depth <= 16)
	{
		bSingleDrip = TRUE;
		volume->pv_max = 1;
	}
	else
	{
		// volume->pv_max = (width * depth) / 4096;
        volume->pv_max = (width * depth) >> 12;
	}

	if (volume->pv_active >= volume->pv_max)
		return;

	if ((volume->pv_max > 1) && (volume->pv_active < (volume->pv_max >> 2)))
		//	Nelno:	fill the particle volume if it has few particles!
		count = volume->pv_max >> 1;
	else
		count = 1;

	// make sure we don't generate more particles then the volume maxes out at
	if ((count + volume->pv_active) > volume->pv_max)
		count = volume->pv_max - volume->pv_active;
    
    fDieTime = (80 - sqrt((float)(6400 - 2 * PARTICLE_GRAVITY * height))) / PARTICLE_GRAVITY;
    fDieTime = cl.time + (fDieTime * 1000);

	for (i=0 ; i < count ; i++)
	{
		if (!free_comparticles)
			return;
		p = free_comparticles;
		free_comparticles = p->next;
		p->next = active_comparticles;
		active_comparticles = p;

//		p->color = 13;

		// no x and y velocities
		p->vel.x = 0;
		p->vel.y = 0;
		// if it's a single drip, have it come out of the center of the drip volume
		if (bSingleDrip)
		{
			p->org.x = p1.x + width/2;
			p->org.y = p1.y + depth/2;
			p->org.z = p2.z;
	
			p->vel.z = -80;// - (rand () & 10);
		}
		else
		{
			// otherwise, randomize the spread
			p->org.x = p1.x + (rand() % width );
			p->org.y = p1.y + (rand() % depth );
			p->org.z = p2.z;

			p->vel.z = -80;// - (frand() * 40);
		}
		
		// no lateral acceleration 
		p->accel.x = p->accel.y = 0;
//		p->accel.z = -100;
        p->accel.z = PARTICLE_GRAVITY;
		
		//	time particle was spawned
		p->time = cl.time;

		//	actual distance particle will fall
//		fall_height = height - (p2.z - p->org.z);
		//	time particle should die
		//p->die_time = cl.time - ((fall_height / p->vel.z) * 1000);
        p->die_time = fDieTime;

		p->alpha = .7;
		p->alphavel = 0.0;

		p->pscale = 0.5;

		p->volume = volume;
		p->volume->pv_active++;

		p->type = type;	//PARTICLE_DRIP_WATER;

        p->flags = PF_ADDSPLASH;		// add a splash when this particle dies
		p->color_rgb.Set( 1, 1, 1 );
	}
}

//////////////////////////////////////////////////////////
//
//	PARTICLE VOLUME MANAGEMENT
//
//////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////
//  Function: CL_InitParticleVolume
//////////////////////////////////////////////////////////

void CL_InitClientSideParticleVolume (entity_state_t *ent)
{
	particleVolume_t *pVolume = NULL;

	if (!(ent->effects2 & (EF2_SNOW|EF2_RAIN|EF2_FIRE|EF2_FOG|EF2_DRIP)))
	{	
		Com_Printf ("\nWARNING: Invalid particle volume defined!!!\n\n");
		return;
	}

	// if we have too many particle volumes, ignore this one
	if (r_numparticleVolumes >= MAX_PARTICLEVOLUMES)
	{
		Com_Printf ("\nWARNING: Too many particle volumes!!!\n\n");
		return;
	}

	// FIXME:ANDREW: check to make sure this entity number isn't already added...

	// get a pointer to the new particle volume record
	pVolume = &r_particleVolumes[r_numparticleVolumes++];

	pVolume->bVisible = FALSE;

	// store the mins and maxs (inherently stores the height)
	pVolume->mins = ent->mins;
	pVolume->maxs = ent->maxs;

	// set the height, etc...
	pVolume->height	= pVolume->maxs[2] - pVolume->mins[2];	

	// what type of particle volume is this?
	if (ent->effects2 & EF2_SNOW)
		pVolume->type = EF2_SNOW;
	else if (ent->effects2 & EF2_RAIN)
		pVolume->type = EF2_RAIN;
	else if (ent->effects2 & EF2_FOG)
		pVolume->type = EF2_FOG;
	else if (ent->effects2 & EF2_DRIP)
		pVolume->type = EF2_DRIP;
	else
		Com_Printf ("\nWARNING: Invalid particle volume!!!\n\n");


	// effects stores the particle type passed over from the server
	pVolume->effects = ent->skinnum;
	pVolume->effects2 = ent->effects2;

	// generate 4 corners with z==0 for quick culling and clipping
	pVolume->corners[0].Set(ent->maxs.x, ent->maxs.y, 0);
	pVolume->corners[1].Set(ent->maxs.x, ent->mins.y, 0);
	pVolume->corners[2].Set(ent->mins.x, ent->mins.y, 0);
	pVolume->corners[3].Set(ent->mins.x, ent->maxs.y, 0);

	return;
}

//////////////////////////////////////////////////////////
//
//  Function: CL_ClipAndCullParticleVolume
//
//	Purpose : test the bounds of the volume to see if they
//            are all behind the player and within range
//
//////////////////////////////////////////////////////////
qboolean CM_inPVS( CVector &p1, CVector &p2 );
int CL_ClipAndCullParticleVolume (particleVolume_t *pVolume)
{
	int			i;
	float		dotProduct = 0.0, distance, clipDistance, minDistance;
	CVector		forward, origin, vecToOrigin;
	qboolean	bRainVolume = FALSE;

	CVector vParticleVolumeOrigin, vPlayerOrigin;

	vPlayerOrigin = cl.refdef.vieworg;
	vParticleVolumeOrigin = pVolume->maxs - pVolume->mins;
	vParticleVolumeOrigin.Multiply( 0.5 );
	vParticleVolumeOrigin.Add( pVolume->mins );

	// get the forward facing vector
	forward = cl.v_forward;

	// copy the view origin 
	origin = cl.refdef.vieworg;
	origin.z = 0;	// clear z

	// rain clips closer than snow and drips do...  (because it's more dense up close)
	if (pVolume->type & EF2_RAIN)
	{
		bRainVolume = TRUE;
		minDistance = clipDistance = PV_HALF_CLIP_DISTANCE;
	}
	else
	{
		minDistance = clipDistance = PV_CLIP_DISTANCE;
	}

	//	clip against all 4 corners of the bounding box
	//	it's 4 corners because we're zeroing the z value, so
	//	it is as if the box is totally flat in the z dimension
	//	this handles larger particle volumes a bit better

	//  loop through the volume's precalculated corners
	for (i = 0; i < 4; i++)
	{
		// get a vector from the corners to the view origin
		vecToOrigin = pVolume->corners[i] - origin;
		
		// first, see if this point is in front of us
		dotProduct = DotProduct( forward, vecToOrigin );

		// this point is in front of the vieworg, don't cull, check the clip
		if (dotProduct >= 0)  
		{
			// if at any time the distance is less then the clip distance
			// then it is close enough so exit quickly
			distance = vecToOrigin.Length();

			if (distance < clipDistance)
			{
				// so... this point is in front of us and within the 
				// clip distance so return false so that it is sent
				// to the renderer
				
				// if it's a rain volume, let's check the four corners and
				// return the smallest distance
				if (!bRainVolume)
				{
					return	false;
				}
				else
				{
					// for rain, record the distance and go on to the next corner
					if (distance < minDistance)
						minDistance = distance;
				}
			}
		}
	}

	// if this is a rain volume and the minDistance is less than our clipDistance,
	// then it means we found the closest corner to us and it was in view
	if (bRainVolume && (minDistance < clipDistance))
	{
		pVolume->dist = minDistance;
		return false;
	}

	// at this point, this particle volumn is about to be clipped/culled out because it's 4 corners are
	// behind the player.. we don't want to add/draw it at all.. it would be culled out in the renderer anyway
	
	// clip/cull it out
	return true;
}

//////////////////////////////////////////////////////////
//
//	Function: CL_AddParticleVolumes
//	
//	Purpose : called in CL_AddEntities to add the 
//            client side particle volumes to the 
//            rendering list
//
//////////////////////////////////////////////////////////


void CL_AddParticleVolumes( void )
{
	int i = 0;
	particleVolume_t *pVolume = NULL;
	
	// loop through all the particle volumes and do clipping and prelim culling
	// on them.. flag them visible if they pass
	for (i=0; i < r_numparticleVolumes; i++)
	{
		// get a pointer to the particle volume record
		pVolume = &r_particleVolumes[i];
	
		// check and see if the four corners at z==0 are behind the player
		// and within a visible distance
		if (CL_ClipAndCullParticleVolume (pVolume))
			pVolume->bVisible = FALSE;
		// we've passed all the culling and clipping tests, pass it on
		// to the renderer and let it cull it against the view frustrum
		else
			pVolume->bVisible = TRUE;
	}
}


//////////////////////////////////////////////////////////
//
//  Function: CL_AllocParticleVolume
//
//////////////////////////////////////////////////////////

cpvolume_t *CL_AllocParticleVolume (int key)
{
	int			i;
	cpvolume_t	*pv = NULL;

	// if the entity has already been alloc'd, send it
	for (i = 0; i < MAX_PVOLUMES; i++)
	{
		// NOTE: key is actually the entity pointer
		if (cl_pvolumes[i].key == key)
			return &cl_pvolumes[i];
		if (!pv && cl_pvolumes [i].key == 0x00)
		{
			//	key == 0 means this is a free volume, so allocate it
			pv = &cl_pvolumes [i];
		}
	}

	// this is a new particle volume
	if (pv)
	{
		cl_numpvolumes++;
		pv->key = key;
	}
	//	find a free volume
	return pv;
}

//////////////////////////////////////////////////////////
//  Function: CL_RemoveParticleVolume
//////////////////////////////////////////////////////////

void CL_RemoveParticleVolume (int key)
{
	cpvolume_t	*pv = NULL;
	int			i;

	//	search through particle volumes to find the one with a matching key
	for (i = 0; i < MAX_PVOLUMES; i++)
	{
		if (cl_pvolumes [i].key == key)
		{
			pv = &cl_pvolumes [i];
			break;
		}
	}

	if (!pv)
		return;

	//	remove all complex particles that are associated with this volume
	memset (pv, 0, sizeof (cpvolume_t));
	cl_numpvolumes--;
}


//////////////////////////////////////////////////////////
//
//  Function: CL_GenerateVolumeParticles
//
//	Purpose : this routine calls the necessary functions to 
//            generate the particles for the given volume
//
//////////////////////////////////////////////////////////

void CL_GenerateVolumeParticles (int key, int type, CVector &mins, CVector &maxs, float dist, int flags)
{
	cpvolume_t	*pv;
	
	if (!cl_particlevol || !cl_particlevol->value)
		return;

	pv = CL_AllocParticleVolume (key);

	if (pv == NULL)
		return;

	if (type & EF2_SNOW)
		CL_SnowParticles (type, flags, pv, mins, maxs);
	else if (type & EF2_RAIN)
		CL_RainParticles (flags, pv, mins, maxs, dist);
	else if (type & EF2_DRIP)
		CL_DripParticles (flags, pv, mins, maxs);
}


//////////////////////////////////////////////////////////
//  Function: CL_ClearPVolumes
//////////////////////////////////////////////////////////

void CL_ClearPVolumes (void)
{
	memset (cl_pvolumes, 0, sizeof(cl_pvolumes));
	cl_numpvolumes = 0;

	// reset the client-side particle volume list
	r_numparticleVolumes = 0;
}
