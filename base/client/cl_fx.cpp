// cl_fx.c -- entity effects parsing and management

#include "client.h"
#include "dk_beams.h"

void    CL_LogoutEffect ( CVector &org, int type);
void    CL_ItemRespawnParticles ( CVector &org);
void    CL_BubbleParticles ( CVector &org, CVector &mins, CVector &maxs, int numBubbles);
void    CL_AddDripSplash (cparticle_t *p);
void	CL_GenerateComplexParticle2( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity);//NSS[10/28/99]:

#include "vertnormals.h"
static CVertexNormals avertexnormals;

static CVector avelocities [NUMVERTEXNORMALS];
extern cvar_t *sv_violence;

//extern  struct model_s  *cl_mod_smoke;
//extern  struct model_s  *cl_mod_flash;

/*
==============================================================

LIGHT STYLE MANAGEMENT

==============================================================
*/

typedef struct
{
    int     length;
    float   value[3];
    float   map[MAX_QPATH];
} clightstyle_t;

clightstyle_t   cl_lightstyle[MAX_LIGHTSTYLES];
int         lastofs;

/*
================
CL_ClearLightStyles
================
*/
void CL_ClearLightStyles (void)
{
    memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
    lastofs = -1;
}

/*
================
CL_RunLightStyles
================
*/
void CL_RunLightStyles (void)
{
    int     ofs;
    int     i;
    clightstyle_t   *ls;

    ofs = cl.time / CL_FRAME_MILLISECONDS;
    if ( ofs == lastofs )
        return;
    lastofs = ofs;

    for ( i=0,ls=cl_lightstyle ; i<MAX_LIGHTSTYLES ; i++, ls++ )
    {
        if ( !ls->length )
        {
            ls->value[0] = ls->value[1] = ls->value[2] = 1.0;
            continue;
        }
        if ( ls->length == 1 )
            ls->value[0] = ls->value[1] = ls->value[2] = ls->map[0];
        else
            ls->value[0] = ls->value[1] = ls->value[2] = ls->map[ofs%ls->length];
    }
}


void CL_SetLightstyle (int i)
{
    char    *s;
    int     j, k;

    s = cl.configstrings[i+CS_LIGHTS];

    j = strlen (s);
    if ( j >= MAX_QPATH )
        Com_Error (ERR_DROP, "svc_lightstyle length=%i", j);

    cl_lightstyle[i].length = j;

    for ( k=0 ; k<j ; k++ )
        cl_lightstyle[i].map[k] = (float)(s[k]-'a')/(float)('m'-'a');
}

/*
================
CL_AddLightStyles
================
*/
void CL_AddLightStyles (void)
{
    int     i;
    clightstyle_t   *ls;
    for ( i=0,ls=cl_lightstyle ; i<MAX_LIGHTSTYLES ; i++, ls++ )
        V_AddLightStyle (i, ls->value[0], ls->value[1], ls->value[2]);
}

/*
==============================================================

DLIGHT MANAGEMENT

==============================================================
*/

cdlight_t       cl_dlights[MAX_DLIGHTS];

/*
================
CL_ClearDlights
================
*/
void CL_ClearDlights (void)
{
    memset (cl_dlights, 0, sizeof(cl_dlights));
}

/*
===============
CL_AllocDlight

===============
*/
cdlight_t *CL_AllocDlight (int key)
{
    int     i;
    cdlight_t   *dl;

// first look for an exact key match
    if ( key )
    {
        dl = cl_dlights;
        for ( i=0 ; i<MAX_DLIGHTS ; i++, dl++ )
        {
            if ( dl->key == key )
            {
                memset (dl, 0, sizeof(*dl));
                dl->key = key;
                return dl;
            }
        }
    }

// then look for anything else
    dl = cl_dlights;
    for ( i=0 ; i<MAX_DLIGHTS ; i++, dl++ )
    {
        if ( dl->die < cl.time )
        {
            memset (dl, 0, sizeof(*dl));
            dl->key = key;
            return dl;
        }
    }

    dl = &cl_dlights[0];
    memset (dl, 0, sizeof(*dl));
    dl->key = key;
    return dl;
}

/*
===============
CL_NewDlight
===============
*/
void CL_NewDlight (int key, float x, float y, float z, float radius, float time)
{
    cdlight_t   *dl;

    dl = CL_AllocDlight (key);
    dl->origin.Set( x, y, z );
    dl->radius = radius;
    dl->die = cl.time + time;
}


/*
===============
CL_RunDLights

===============
*/
void CL_RunDLights (void)
{
    int         i;
    cdlight_t   *dl;

    dl = cl_dlights;
    for ( i=0 ; i<MAX_DLIGHTS ; i++, dl++ )
    {
        if ( !dl->radius )
            continue;

        if ( dl->die < cl.time )
        {
            dl->radius = 0;
            return;
        }
        dl->radius -= cls.frametime*dl->decay;
        if ( dl->radius < 0 )
            dl->radius = 0;
    }
}


/*
===============
CL_AddDLights

===============
*/
void CL_AddDLights (void)
{
    int         i;
    cdlight_t   *dl;

    dl = cl_dlights;
    for ( i=0 ; i<MAX_DLIGHTS ; i++, dl++ )
    {
        if ( !dl->radius )
            continue;
        V_AddLight (dl->origin, dl->radius, dl->color[0], dl->color[1], dl->color[2]);
    }
}



/*
==============================================================

EFFECTS MANAGEMENT

Flares are just like dynamic lights (and should probably replace them)
with new features:

Flares are blended polys (like dynamic lights in glquake)
Flares have a controllable radius
Flares have a controllable falloff (i.e. the rate at which the light fades)

==============================================================
*/

// SCG[8/10/99]: Not used
//cflare_t        cl_flares[MAX_FLARES];

/*
================
CL_ClearFlares
================
*/
// SCG[8/10/99]: Not used
/*
void CL_ClearFlares (void)
{
    memset (cl_flares, 0, sizeof(cl_flares));
}
*/

/*
===============
CL_AllocFlare
===============
*/
// SCG[8/10/99]: Not used
/*
cflare_t *CL_AllocFlare (int key)
{
    int         i;
    cflare_t    *f;

// first look for an exact key match
    if ( key )
    {
        f = cl_flares;
        for ( i=0 ; i<MAX_FLARES ; i++, f++ )
        {
            if ( f->key == key )
            {
                memset (f, 0, sizeof(*f));
                f->key = key;
                return f;
            }
        }
    }

// then look for anything else
    f = cl_flares;
    for ( i=0 ; i<MAX_FLARES ; i++, f++ )
    {
        if ( f->die < cl.time )
        {
            memset (f, 0, sizeof(*f));
            f->key = key;
            return f;
        }
    }

    f = &cl_flares[0];
    memset (f, 0, sizeof(*f));
    f->key = key;
    return f;
}
*/

/*
===============
CL_AddFlares
===============
*/
// SCG[8/10/99]: Not used
/*
void CL_AddFlares (void)
{
    int         i;
    cflare_t    *f;

    f = cl_flares;
    for ( i=0 ; i<MAX_FLARES ; i++, f++ )
    {
        if ( !f->radius )
            continue;
        V_AddFlare (f->origin, f->angles, f->radius, f->falloff,
                    f->color[0], f->color[1], f->color[2]);
    }
}
*/

/*
===============
CL_RunFlares
===============
*/
// SCG[8/10/99]: Not used
/*
void CL_RunFlares (void)
{
    int         i;
    cflare_t    *f;
    centity_t   *ent;

    //	Nelno:	I changed this because it is not initialized yet.  Move it into loop.
    //	Maybe I didn't merge the right stuff or you were in the middle of doing this???
    //	ent = &cl_entities[i];

    f = cl_flares;

    for ( i=0 ; i<MAX_DLIGHTS ; i++, f++ )
    {
        if ( (!f->radius) || (!f->key) )
            continue;

        ent = &cl_entities [f->key];

        if ( f->die < cl.time )
        {
            f->radius = 0;
            return;
        }
        f->radius -= cls.frametime*f->decay;
        if ( f->radius < 0 )
            f->radius = 0;
        
        f->origin = ent->current.origin;
    }
}
*/

/*
===============
CL_ParseFlare
===============
*/
// SCG[8/10/99]: Not used
/*
void CL_ParseFlare (void)
{
    int         i;
    centity_t   *ent;
    cflare_t    *flare;

    i = MSG_ReadShort (&net_message);               // get entity number

    if ( i < 1 || i >= MAX_EDICTS )
        Com_Error (ERR_DROP, "CL_ParseFlare: bad entity");

    flare = CL_AllocFlare (i);                      // reference flare by entity number

    ent = &cl_entities[i];

    // get origin
    flare->origin = ent->current.origin;

    // parse flare information
    flare->color.x  = MSG_ReadByte (&net_message);
    flare->color.y  = MSG_ReadByte (&net_message);
    flare->color.z  = MSG_ReadByte (&net_message);
    flare->radius   = MSG_ReadFloat (&net_message);
    flare->die      = cl.time + MSG_ReadFloat (&net_message);
    flare->decay    = MSG_ReadFloat (&net_message);
    flare->falloff  = MSG_ReadFloat(&net_message);
}
*/

/*
===============
CL_ClearFogInfo
===============
*/
extern foginfo_t	global_fog_state;
extern int			last_player_contents;

void CL_ClearFogInfo( void )
{
    memset( &r_foginfo, 0, sizeof(foginfo_t) );
	memset( &global_fog_state, 0, sizeof(foginfo_t) );
	last_player_contents = CONTENTS_EMPTY;
}

/*
===============
CL_SetFogInfo
===============
*/
void CL_SetFogInfo (qboolean on, int inc, float r, float g, float b, float start, float end, float skyend)
{
    r_foginfo.active = on;

    if ( !r_foginfo.active )
        return;

	r_foginfo.r = r;
	r_foginfo.g = g;
	r_foginfo.b = b;

    r_foginfo.start = start;
    r_foginfo.end = end;
    r_foginfo.inc = inc;
    r_foginfo.skyend = skyend;

    if ( r_foginfo.r > 1 )
        r_foginfo.r = 1;
    if ( r_foginfo.g > 1 )
        r_foginfo.g = 1;
    if ( r_foginfo.b > 1 )
        r_foginfo.b = 1;
}

/*
===============
CL_RunFog
===============
*/
void CL_RunFog (void)
{
    if ( !r_foginfo.active || r_foginfo.inc == 0 )
        return;

    if ( r_foginfo.inc < 0 )
    {// negative increment - fade in (specified as dec in map entity)
        if ( r_foginfo.start > 4 )
            r_foginfo.start += r_foginfo.inc;
        if ( r_foginfo.start <= 4 )
            r_foginfo.start = 4;
    }
    else
    {// positive increment - fade out (specified as inc in map entity)
        if ( r_foginfo.start < r_foginfo.end )
            r_foginfo.start += r_foginfo.inc;
        if ( r_foginfo.start >= r_foginfo.end )
        {
            r_foginfo.start = r_foginfo.end;
            r_foginfo.active = 0;
        }
    }
}

/*
===============
CL_ParseFog
===============
*/
void CL_ParseFog (void)
{
    qboolean on;
    float	r, g, b;
    float	start, end, skyend;
	int		inc;

    on = 0;
	r = 0;
	g = 0;
	b = 0;
	start = 0;
	end = 0;
	inc = 0;

    on      = MSG_ReadByte (&net_message);          // fogging active?
    r       = MSG_ReadFloat (&net_message);          // red componant
    g       = MSG_ReadFloat (&net_message);          // green componant
    b       = MSG_ReadFloat (&net_message);          // blue componant
    start   = MSG_ReadFloat (&net_message);          // distance from the player the fog starts
    end     = MSG_ReadFloat (&net_message);          // where the fog ends
    skyend  = MSG_ReadFloat (&net_message);          // where the fog for the sky ends
    inc     = MSG_ReadByte (&net_message);          // amount to increase the distance from the player each frame

    // 'MSG_Write' functions only send positive values :P

    if ( inc > 127 )
        inc -= 256;

    CL_SetFogInfo (on, inc, r, g, b, start, end, skyend);

	global_fog_state = r_foginfo;
}

/*
===============
CL_ParseEffect
===============
*/
void CL_ParseEffect (void)
{
    byte type;

    type = MSG_ReadLong (&net_message);

    switch ( type )
    {
// SCG[8/10/99]: Not used
/*
        case ET_FLARE:
            CL_ParseFlare ();
            break;
*/
        case ET_FOG:
            CL_ParseFog ();
            break;
    }
}
/*
==============================================================

PARTICLE MANAGEMENT

==============================================================
*/

cparticle_t *active_particles, *free_particles;

cparticle_t particles[MAX_PARTICLES];
int         cl_numparticles = MAX_PARTICLES;

cparticle_t *active_comparticles, *free_comparticles;

cparticle_t comparticles[MAX_PARTICLES]; 
int         cl_numcomparticles = MAX_PARTICLES;

//---------------------------------------------------------------------------
// CL_SpawnParticle()
//---------------------------------------------------------------------------
cparticle_t *CL_SpawnParticle()
{
    cparticle_t *p;

    if ( !free_particles )
        return(NULL);

    p = free_particles;
    free_particles = p->next;
    p->next = active_particles;
    active_particles = p;

    // initialize stuff
    p->pscale=0;

    return(p);
}

//---------------------------------------------------------------------------
// CL_SpawnComParticle()
//---------------------------------------------------------------------------
cparticle_t *CL_SpawnComParticle(void)
{
    cparticle_t *p;

    if ( !free_comparticles )
        return(NULL);

    p = free_comparticles;
    free_comparticles = p->next;

    memset(p,0,sizeof(*p));             // memset after p->next is read, and before it's written

    p->next = active_comparticles;
    active_comparticles = p;

    return(p);
}

/*
===============
CL_ClearParticles
===============
*/
void CL_ClearParticles (void)       //CHECKME : is this completely working right?
{
	int i;
    //////////////////////////////////////
    // particles

    free_particles = &particles[0];
    active_particles = NULL;

    for ( i = 0; i < cl_numparticles; i++ )
	{
        particles[i].next = &particles[i+1];
        particles[i].pFunc = NULL;
	}
    particles[cl_numparticles-1].next = NULL;

    //////////////////////////////////////
    // complex particles

    free_comparticles = comparticles;
    active_comparticles = NULL;

    for ( i = 0; i < cl_numcomparticles; i++ )
    {
	    comparticles[i].next = &comparticles[i+1];
        comparticles [i].pFunc = NULL;
    }
	comparticles[cl_numcomparticles-1].next = NULL;
}

float CL_ParticleSplashTime(CVector vecOrg, float fZVelocity, float fAcceleration)
{
    trace_t tr;
    float fHeight, fDieTime;
    CVector vecTraceEnd;

    vecTraceEnd = vecOrg;
    vecTraceEnd.z =- 4096;
	
	fDieTime = cl.time;

    tr = CL_TraceLine(vecOrg, vecTraceEnd, 1, MASK_ALL);
    if(tr.fraction < 1.0)
    {
        fHeight = vecOrg.z - tr.endpos.z;
        fDieTime = (-fZVelocity - sqrt(fZVelocity * fZVelocity - 2 * fAcceleration * fHeight)) / fAcceleration;
        fDieTime = cl.time + (fDieTime * 1000);
    }
    return fDieTime;
}
///////////////////////////////////////////////////////////////////////////////
//	CL_BloodThink
//
///////////////////////////////////////////////////////////////////////////////
void CL_BloodThink(cparticle_t *p)
{
	if(p->type == PARTICLE_CP4)
	{
		if(p->pscale < 35.0f)
		{
			p->pscale	*= 1.10;
			p->alpha	*= 0.95;
		}
		else
		{
			p->alpha	*= 0.85;
		}
	}
	else
	{
		if(p->pscale < 15.0f)
		{
			p->alpha	*= 0.95;
			p->pscale	*= 1.10;
		}
		else
		{
			p->alpha	*= 0.85;
			p->pscale	*= 1.05;
		}
	
	}
}

///////////////////////////////////////////////////////////////////////////////
//	CL_BloodSplash
//
///////////////////////////////////////////////////////////////////////////////
// void CL_BloodSplash (CVector &org, CVector &dir, int color, int count)
//  
//<nss> Older code = dumb
//<nss> Newer code = better
void CL_BloodSplash (void)
{
    cparticle_t *p;
    int         count, iDamage, type;
	float		size;
    CVector     org,dir,target_angles,forward,vel;

    iDamage = MSG_ReadByte(&net_message);
    MSG_ReadPos (&net_message, org);
    MSG_ReadDir (&net_message, dir);

// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	return;
#endif

	if (sv_violence->value > 0.0)
		return;

	count	= iDamage & 63;
	type	= 0;
	size	= (float)count/76; 
	if(size < 0.05)
	{
		size = 0.05;
	}
	else if(size > 1.0)
	{
		size = 1.0;
	}

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}
        p->time		= cl.time;
 
        p->org = org;
		
		dir.Negate();

		VectorToAngles(dir,target_angles);

		//Let's spread in a 30 degree cone either way
		target_angles.yaw += crand()*20;
		target_angles.pitch += crand()*20;

		target_angles.AngleToForwardVector(forward);
        p->accel.x	= p->accel.y = 0;
		if(type >3)
		{
			type = 0;
		}
		
		if(frand()>0.35)
		{
			p->color_rgb.Set(0.8,0,0);
			vel			= forward * ( 45 + ( frand() * 65 ) );
			vel.z		= vel.z * 2.75;
			p->accel.z	= PARTICLE_GRAVITY;
			p->type		= PARTICLE_BLOOD2 + type;
			p->pscale	= ((1.0 + (frand() * 8.0))*size)+3.0f;
	        p->alpha	= 0.35 + crand()*0.15;
			p->alphavel = (0.65 + frand()*0.3);
			p->die_time = cl.time + 300 + (800*frand());
			type++;
		}
		else
		{
			vel			= forward * ( 25 + ( crand() * 10 ) );
			vel.z		= vel.z * 2.75;
			p->pFunc	= CL_BloodThink;
			if(frand()>0.50)
			{
				p->color_rgb.Set(0.40f,-1,-1);
				p->pscale	= (5.0*size) + ((3.5*size) * crand());
				p->type		= PARTICLE_CP4;
				p->die_time = cl.time + 200 + (400*frand());
		        p->alpha	= 0.80 + crand()*0.05;
			}
			else
			{
				p->die_time = cl.time + 200 + (300*frand());
				p->color_rgb.Set(1.0,-1,-1);
				p->pscale	= (10.0*size);
				p->type		= PARTICLE_BLOOD2 + type;
		        p->alpha	= 0.90 + crand()*0.05;
				type++;
			}
			p->accel.z	= PARTICLE_GRAVITY;
		}
		p->vel = vel;

		//Set a random die time
		
    }
}

//---------------------------------------------------------------------------
// CL_SprayBlood()
//---------------------------------------------------------------------------
void CL_SprayBlood( CVector &org, CVector &dir, int color, int count)
{
    cparticle_t *p;
    float       d;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
//        p->color = 69;
        p->color_rgb.Set(0.8,0,0);
        p->type = PARTICLE_BLOOD + (rand() & 3); // 4 random blood particles;

        d = rand()&31;
        p->org.x = org.x + ((rand()&3)-1) + d*dir.x;
        p->org.y = org.y + ((rand()&3)-1) + d*dir.y;
        p->org.z = org.z + ((rand()&3)-1) + d*dir.z;
        p->vel.Zero();

        p->accel.Zero();
        p->alpha = 1.0;

        p->alphavel = -(.6+crand()*.2);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SparkleSplash
//
///////////////////////////////////////////////////////////////////////////////
// Logic[3/26/99]: void CL_SparkleSplash ( CVector &org, CVector &dir, int color, int count)
void CL_SparkleSplash ( void )
{

   	//<nss> Redo of the SparkleSplash using new ComplexParticle Function. 
	CVector Alpha,Color,Dir,org;
	
    MSG_ReadByte(&net_message);
    MSG_ReadPos (&net_message, org);
    MSG_ReadDir (&net_message, Dir);	
	
	
	Dir.Set(crand(),crand(),crand());//Make it go in random directions
	Color.Set(0.45,0.45,0.65);//Set the particle color
	Alpha.Set(0.65,0.55,0.25);//Set the initial alpha, the depletion rate, and the random addition to depletion
	//The sparkle function was randomly assigning one or the other sparkle, this is how it should be done.
	if(frand() > 0.50f)
	{
		CL_GenerateComplexParticle(org , Dir, Color, Alpha, 0.75f, 2, 7, 500,PARTICLE_SPARKLE1,2);//Make Da' Sparks
	}
	else
	{
		CL_GenerateComplexParticle(org , Dir, Color, Alpha, 0.75f, 2, 7, 500,PARTICLE_SPARKLE2,2);//Make Da' Sparks
	}
	//Dunno about the below function may look into this one later. 
	
	
//	cparticle_t *p;
//  float       d;
//  CVector org;
//  CVector dir;
//   int color;
//   int count;
    
//    count = MSG_ReadByte(&net_message);
//    MSG_ReadPos (&net_message, org);
//    MSG_ReadDir (&net_message, dir);
//    color = 0xe8;
//    count = 3;  //Uhhhh used to be 60 particles per hit... WAY FUCKING TOO MANY.<nss>

//    for ( int i = 0; i < count; i++ )
//   {
//        if ( !(p=CL_SpawnComParticle()) )
//            return;

//        p->time = cl.time;
//        p->color = 0x10;
//		p->color_rgb.Set(0.65,0.35,0.15);
//        if ( rand () & 1 )
//           p->type = PARTICLE_SPARKLE1;
//        else
//            p->type = PARTICLE_SPARKLE2;
//
//      d = rand()&31;
//        p->org = org;
//        p->vel.x = crand()*20;
//        p->vel.y = crand()*20;
//        p->vel.z = crand()*20;

//        p->accel.x = p->accel.y = 0;
//        p->accel.z = PARTICLE_GRAVITY;
//        p->alpha = 0.40;

//        p->alphavel = -1.0;// / (0.5 + frand()*0.3);
		//p->color_rgb.Set( 1, 1, 1 );
//    }
}

//---------------------------------------------------------------------------
// CL_BloodFade()
//---------------------------------------------------------------------------
// Logic[3/26/99]: void CL_BloodFade( CVector &org, CVector &dir, int color, int count)
void CL_BloodFade(void)
{
    cparticle_t *p;
    CVector org;
    CVector dir;
    int count;

    MSG_ReadPos(&net_message, org);
    MSG_ReadDir(&net_message, dir);

    count = 10 + rand() % 30;
    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}
        p->time = cl.time;
        p->color_rgb.Set(0.8,0,0);
        p->type = PARTICLE_BLOOD + (rand() & 3); // 4 random blood particles;

        p->org.x = org.x + ((rand()&7)-4);
        p->org.y = org.y + ((rand()&7)-4);
        p->org.z = org.z + ((rand()&7)-4);
        p->vel.Zero();

        p->accel.Zero();
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1;// / (0.5 + frand()*0.3);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_BloodSpurt
//
///////////////////////////////////////////////////////////////////////////////

void CL_BloodSpurt ( CVector &org, CVector &dir, int color, int count)
{
    cparticle_t *p;
    float       d;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
//        p->color = 69;
        p->color_rgb.Set(0.8,0,0);
        p->type = PARTICLE_BLOOD + (rand() & 3); // 4 random blood particles;

        d = crand () * 64.0;
        p->org = org;
        p->vel.x = d * dir.x;
        p->vel.y = d * dir.y;
        p->vel.z = d * dir.z;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0;// / (0.5 + frand()*0.3);
    }
}

/*
===============
CL_ParticleEffect

Wall impact puffs
===============
*/
void CL_ParticleEffect ( CVector &org, CVector &dir, int color, int count)
{
    cparticle_t *p;
    float       d;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        d = rand()&31;
        p->org.x = org.x + ((rand()&7)-4) + d*dir.x;
        p->org.y = org.y + ((rand()&7)-4) + d*dir.y;
        p->org.z = org.z + ((rand()&7)-4) + d*dir.z;
        p->vel.x = crand()*20;
        p->vel.y = crand()*20;
        p->vel.z = crand()*20;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;
        // Logic[6/24/99]: looking for a random value between -2.0 and -1.25?
        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
    }
}

/*
===============
CL_ParticleEffect2
===============
*/
/*
void CL_ParticleEffect2 ( CVector &org, CVector &dir, int color, int count)
{
    cparticle_t *p;
    float       d;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->color = color;

        d = rand()&7;
        p->org.x = org.x + ((rand()&7)-4) + d*dir.x;
        p->org.y = org.y + ((rand()&7)-4) + d*dir.y;
        p->org.z = org.z + ((rand()&7)-4) + d*dir.z;

        p->vel.x = crand()*20;
        p->vel.y = crand()*20;
        p->vel.z = crand()*20;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
    }
}
*/

//---------------------------------------------------------------------------
// CL_IceCrystals()
//---------------------------------------------------------------------------
// Logic[3/26/99]: void CL_IceCrystals( CVector &org, CVector &dir, int count)
// Logic[3/26/99]: only called from CL_ParseTEnts()
void CL_IceCrystals(void)
{
    CVector org;
    CVector dir;
    cparticle_t *p;
    float       d;
    int count;

    MSG_ReadPos(&net_message, org);
    MSG_ReadDir(&net_message, dir);
    count=MSG_ReadByte(&net_message);

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->type=PARTICLE_ICE;
        p->pscale=.4;
        p->time = cl.time;
//		p->color = 110+crand()*2.9;
		p->color_rgb.Set( 1, 1, 1 );

        d = rand()&7;
        p->org.x = org.x + ((rand()&7)-4) + d*dir.x;
        p->org.y = org.y + ((rand()&7)-4) + d*dir.y;
        p->org.z = org.z + ((rand()&7)-4) + d*dir.z;

        p->vel.x = crand()*20;
        p->vel.y = crand()*20;
        p->vel.z = crand()*20;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -(.4+frand()*.1);         //-1.0 / (0.2 + frand()*0.3);
    }
}

/*
===============
CL_TeleporterParticles
===============
*/
void CL_TeleporterParticles (entity_state_t *ent)
{
    cparticle_t *p;

    for ( int i = 0; i < 8; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
            return;

        p->time = cl.time;

		p->color_rgb.Set( 1.0, 1.0, 1.0 );
        p->org.x = ent->origin.x - 16 + (rand()&31);
        p->org.y = ent->origin.y - 16 + (rand()&31);
        p->org.z = ent->origin.z - 16 + (rand()&31);
        p->vel.x = crand()*14;
        p->vel.y = crand()*14;
        p->vel.z = crand()*14;

        p->org.z = ent->origin.z - 8 + (rand()&7);
        p->vel.z = 80 + (rand()&7);

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -0.5;
    }
}

/*
===============
CL_LogoutEffect

===============
*/
void CL_LogoutEffect ( CVector &org, int type)
{
    cparticle_t *p;

    for ( int i = 0; i < 500; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
            return;

        p->time = cl.time;

		p->color_rgb.Set( 1.0, 1.0, 0.25 );

        p->org.x = org.x - 16 + frand()*32;
        p->org.y = org.y - 16 + frand()*32;
        p->org.z = org.z - 24 + frand()*56;

        p->vel.x = crand()*20;
        p->vel.y = crand()*20;
        p->vel.z = crand()*20;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        // Logic[6/24/99]: value between -1 and -0.769230769230769230769230769230769
        // p->alphavel = -1.0 / (1.0 + frand()*0.3);
        p->alphavel = -1.0 + (frand()*0.3);
    }
}


/*
===============
CL_ItemRespawnParticles

===============
*/
void CL_ItemRespawnParticles ( CVector &org)
{
    cparticle_t *p;

    for ( int i = 0; i < 64; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;

		p->color_rgb.Set( 0.0, 0.0, 1.0 );
        p->org.x = org.x + crand()*8;
        p->org.y = org.y + crand()*8;
        p->org.z = org.z + crand()*8;

        p->vel.x = crand()*8;
        p->vel.y = crand()*8;
        p->vel.z = crand()*8;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY*0.02;
        p->alpha = 1.0;

        // p->alphavel = -1.0 / (1.0 + frand()*0.3);
        p->alphavel = -1.0 + (frand()*0.3);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_NovaEffect
///////////////////////////////////////////////////////////////////////////////
void CL_NovaEffect( CVector &org, CVector &fwd, CVector &normal)
{
    cparticle_t *p;
    float       d;
    short count=5;

    CVector reflect;

    // reflection vector: fwd - 2*((fwd DOT normal) * normal))
    reflect = normal * DotProduct(fwd,normal);
    reflect.Multiply( 2 );
    reflect = normal;

    reflect = fwd - reflect;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
//        p->color = color;
		if (rand() & 0x01)
			p->color_rgb.Set( 0.3,0.3,0.3 );
		else
			p->color_rgb.Set( 0.8,0.4,0.2 );

        d = rand()&7;
        p->org.x = org.x + ((rand()&7)-4) + d*reflect.x;
        p->org.y = org.y + ((rand()&7)-4) + d*reflect.y;
        p->org.z = org.z + ((rand()&7)-4) + d*reflect.z;

        p->vel.x = crand()*20 + reflect.x*40;
        p->vel.y = crand()*20 + reflect.y*40;
        p->vel.z = crand()*20 + reflect.z*40;

        p->type = PARTICLE_SPARKS;
        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;

        p->alpha = 1.0;
        p->alphavel = -1.0 + (frand()*0.9);
        p->pscale = 1.0 + 0.5*crand();
    }
}


///////////////////////////////////////////////////////////////////////////////
//	CL_ParticleEffect_Sparkles
//
//	creates "count" particles of type "type"
///////////////////////////////////////////////////////////////////////////////

/*
void CL_ParticleEffect_Sparkles ( CVector &org, CVector &dir, int color, int count)
{
    cparticle_t *p;
    float       d;
	
	//Put a limit on the number of particles that we can spawn... 10 is WAY plenty.
	if(count > 10)
	{
		count = 10;
	}

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
//        p->color = color;
		p->color_rgb.Set(1.0, 1.0, 1.0);

        d = rand()&7;
        p->org.x = org.x + ((rand()&7)-4) + d*dir.x;
        p->org.y = org.y + ((rand()&7)-4) + d*dir.y;
        p->org.z = org.z + ((rand()&7)-4) + d*dir.z;
        p->vel.x = crand()*250;
        p->vel.y = crand()*250;
        p->vel.z = crand()*250;

        if ( rand () & 1 )
           p->type = PARTICLE_SPARKLE1;
        else
            p->type = PARTICLE_SPARKLE2;

        p->pscale = crand () * 2.0;
        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;

        p->alpha = 1.0;

        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
		p->color_rgb.Set( 1, 1, 1 );
    }
}
*/

void CL_ParticleEffectSparks(CVector vecOrigin, CVector vecDir, CVector vecColor, int iDamage)
{
    cparticle_t *p;
    float       d;
    int         i, iCount;
    int         iSparkIndex[2] = {PARTICLE_SPARK_1, PARTICLE_SPARK_2};

    iDamage = iDamage & 31; // cap damage, don't launch sparks so hard!
    iCount = (int)rand() & 31;
    vecDir.Normalize();
    vecDir = vecDir * iDamage;
    iCount = (iCount * (1+iDamage) & 63);   // vecDir can be non-normalized velocity as well, jack up count based on impact rate
    for( i=0; i<iCount; i++)
    {
        if(! (p = CL_SpawnComParticle()) )
            return;

        d = rand() & 7;
        p->time = cl.time;
        p->color_rgb = vecColor;
        /*
        p->org.x = vecOrigin.x + ((rand() & 7) -4) + d;// * vecDir.x;
        p->org.y = vecOrigin.y + ((rand() & 7) -4) + d;// * vecDir.y;
        p->org.z = vecOrigin.z + ((rand() & 7) -4) + d;// * vecDir.z;
        */
        p->org = vecOrigin;
        p->lastorg = p->org;
                    // deviation                  
        p->vel.x = (vecDir.x * iDamage) + (crand() * 100);
        p->vel.y = (vecDir.y * iDamage) + (crand() * 100);
        p->vel.z = (vecDir.z * iDamage) + (crand() * 100);
        
//        p->type = iSparkIndex[iPIndex];
        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.Set(p->vel.x * -1, p->vel.y * -1, PARTICLE_GRAVITY);
        //p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;
        //p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->scalevel = p->alphavel = -0.5 - (0.10 * d);
        p->pscale = d * iDamage * 0.005;

    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_ParticleEffect_Complex
//
//	creates "count" particles of type "type"
///////////////////////////////////////////////////////////////////////////////
void CL_ParticleEffect_Complex ( CVector &org, CVector &dir, CVector color, int count, int type)
{
    cparticle_t *p;
    float       d;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->color_rgb = color;

        
        d = rand()&7;

        p->org.x = org.x + ((rand()&3)-2) + d*dir.x;
        p->org.y = org.y + ((rand()&3)-2) + d*dir.y;
        p->org.z = org.z + ((rand()&3)-2) + d*dir.z;

        p->lastorg = p->org;

        p->vel.x = crand()*20;
        p->vel.y = crand()*20;
        // p->vel.z = crand()*20;
        //p->vel = dir * -300;
        //p->vel.x = p->vel.y = 0;
        //p->vel.z = PARTICLE_GRAVITY * 0.2;

        p->type = type;
        // p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY * 0.2;
//        p->accel = dir * 400;

        p->alpha = 1.0;

        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SuperTrident
///////////////////////////////////////////////////////////////////////////////
// Logic[3/26/99]: void CL_SuperTrident( CVector &org, CVector &dir, int color, int count, int type)
// Logic[3/26/99]: This code is only called from CL_ParseTEnts()
void CL_SuperTrident(void)
{
    CVector org;
    CVector dir;
    float       fAcceleration = PARTICLE_GRAVITY;
    float       d;

    org.Zero();
    MSG_ReadPos (&net_message, org);
    cparticle_t *p;

	// [cek 11-18-99]: 250?! WTF?!
    // Logic[3/26/99]: 500?! WTF?!
    for ( int i = 0; i < 50; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
//        p->color = 0x53;

        d = crand()*7;
        p->org.x = org.x + (crand()*20-10) + d*dir.x;
        p->org.y = org.y + (crand()*20-10) + d*dir.y;
        p->org.z = org.z + (crand()*20-10) + d*dir.z;

        p->vel.x = crand()*200;
        p->vel.y = crand()*200;
        p->vel.z = crand()*200;

        p->type = PARTICLE_BLUE_SPARKS;
        p->accel.x = p->accel.y = 0;
        p->accel.z = fAcceleration;

        p->alpha = 0.3;

        p->alphavel = -(.5 + crand()*.2);
        p->pscale = 10.0;
		p->color_rgb.Set( 1, 1, 1 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//	CL_ParticleEffect_Complex2
//
//	creates "count" particles of type "type"
///////////////////////////////////////////////////////////////////////////////
void CL_ParticleEffect_Complex2( CVector &org, CVector &dir, int speed, CVector& color, int count, int type, float gravity)
{
    cparticle_t *p;
    float       d;

    int spread = (type & PARTICLE_RANDOM_SPREAD);
    type &= ~PARTICLE_RANDOM_SPREAD;

    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
		p->color_rgb = color;

        d = rand()&7;

        if ( spread )
        {
            p->org.x = org.x + ((rand() & 7) - 4) + d * dir.x;
            p->org.y = org.y + ((rand() & 7) - 4) + d * dir.y;
            p->org.z = org.z + ((rand() & 7) - 4) + d * dir.z;
        }
        else
        {
            p->org = org;
        }

        p->vel = dir * speed;

        p->type = type;
        p->accel.x = p->accel.y = 0;
        p->accel.z = gravity;   //-PARTICLE_GRAVITY

        p->alpha = 1.0;

        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
    }
}

/*
===============
CL_ExplosionParticles
===============
*/
void CL_ExplosionParticles ( CVector &org)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
	N_Origin = org;

	//See notes on Complex particle creation to understand this section 
	//Create the sparky fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color.Set(0.65f,0.635f,0.15f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.75f,0.10f);

	//Gravity settings
	Gravity.Set(crand()*20.0+10.0f,crand()*20.0+10.0f,crand()*20.0+10.0f);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.0f+frand()*0.75,5.0f, 35,150.0f,PARTICLE_CP4,Gravity);


	//Create the smokey looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color to a nice black
	Color.Set(0.01f,0.01f,0.01f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.65f,0.10f);

	//Gravity settings
	Gravity.Set(0.0f,0.0,frand()*20.0+50.0f);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,5.0f,10.0f, 65,150.0f,PARTICLE_SMOKE,Gravity);
    
	//	Nelno:	use complex particles, you ass!!
/*    for ( int i = 0; i < 64; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->org.x = org.x + ((rand()%64)-32);
        p->org.y = org.y + ((rand()%64)-32);
        p->org.z = org.z + ((rand()%64)-32);
        p->vel.x = (rand()%150)-75;
        p->vel.y = (rand()%150)-75;
        p->vel.z = (rand()%150)-75;

        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;

        p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 + (frand()*0.83);
        p->color = 0x48 + (rand()&3);
    }

    for ( i = 0 ; i < 64; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;

        p->org.x = org.x + ((rand()%12)-6);
        p->org.y = org.y + ((rand()%12)-6);
        p->org.z = org.z + ((rand()%12)-6);
        p->vel.x = (rand()%12)-6;
        p->vel.y = (rand()%12)-6;
        p->vel.z = (rand()%12)-6;

        p->type = PARTICLE_SMOKE;
        p->accel.x = p->accel.y = 0;
        p->accel.z = 20;

        p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.8);
        p->alphavel = -1.0 + (frand()*0.555);
        p->color = 4 + (rand()&7);
    }
	*/
}

void CL_BigExplosionParticles( CVector &org,float scale)
{
//    int         ofs,ofsd2,vofs,vofsd2;
    int         ofs,vofs,vofsd2;
    cparticle_t *p;
    int         count;

    // ofs from center
    ofs = 64.0*scale;
//    ofsd2 = ofs * 0.5;

    // num particles
    count = (int)(64.0f*(float)scale) & 255;

    // velocity ofs
    vofs = 64.0*scale;
    vofsd2 = vofs * 0.5;

    for ( int i = 0 ; i < count; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->time = cl.time;

        p->org = org;
		//NSS[11/10/99]:
		p->lastorg = p->org;
        p->vel.x = (frand() * vofs - vofsd2) * 5;
        p->vel.y = (frand() * vofs - vofsd2) * 5;
        p->vel.z = (frand() * vofs - vofsd2) * 5;// + PARTICLE_GRAVITY;

        // p->type = PARTICLE_BIG_SPARKS;
        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;

        p->alpha = 0.8;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 + (frand()*0.833);
        p->color_rgb.Set(1.0, 0.6, 0.3);
        p->pscale=(frand() * 0.3);
    }

}

///////////////////////////////////////////////////////////////////////////////
//	CL_SmallExplosionParticles
//
///////////////////////////////////////////////////////////////////////////////

void CL_SmallExplosionParticles ( CVector &org)
{
    cparticle_t *p;

    //	Nelno:	use complex particles, you ass!!
    for ( int i = 0 ; i < 16 ; i++ )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

        p->time = cl.time;

        p->org.x = org.x + ((rand()%64)-32);
        p->org.y = org.y + ((rand()%64)-32);
        p->org.z = org.z + ((rand()%64)-32);

        p->vel.x = (rand()%150)-75;
        p->vel.y = (rand()%150)-75;
        p->vel.z = (rand()%150)-75;

        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
		//NSS[11/10/99]:
		p->lastorg = p->org;
        
		p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 + (frand()*0.833);
//        p->color = 0x48 + (rand()&3);
		p->color_rgb.Set( 1.0, 0.25, 0.25 );
    }
}

/*
===============
CL_BigTeleportParticles
===============
*/
/*
void CL_BigTeleportParticles ( CVector &org)
{
    cparticle_t *p;
    float       angle, dist;
    static int colortable[4] = {2*8,13*8,21*8,18*8};

    for ( int i = 0; i < 4096; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;

        p->color = colortable[rand()&3];

        angle = M_PI*2*(rand()&1023)/1023.0;
        dist = rand()&31;
        p->org.x = org.x + cos(angle)*dist;
        p->vel.x = cos(angle)*(70+(rand()&63));
        p->accel.x = -cos(angle)*100;

        p->org.y = org.y + sin(angle)*dist;
        p->vel.y = sin(angle)*(70+(rand()&63));
        p->accel.y = -sin(angle)*100;

        p->org.z = org.z + 8 + (rand()%90);
        p->vel.z = -100 + (rand()&31);
        p->accel.z = -PARTICLE_GRAVITY*0.5;
        p->alpha = 1.0;
    
        // p->alphavel = -0.3 / (0.5 + frand()*0.3);
        p->alphavel = -0.6 + (frand()*0.375);
    }
}
*/

/*
===============
CL_BlasterParticles

Wall impact puffs
===============
*/
/*
void CL_BlasterParticles ( CVector &org,  CVector &dir)
{
    cparticle_t *p;
    float       d;
    int         count;
    count = 40;
    for ( int i = 0; i < count; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->color = 0xe0 + (rand()&7);

        d = rand()&15;
        p->org.x = org.x + ((rand()&7)-4) + d*dir.x;
        p->org.y = org.y + ((rand()&7)-4) + d*dir.y;
        p->org.z = org.z + ((rand()&7)-4) + d*dir.z;

        p->vel.x = dir.x * 30 + crand()*40;
        p->vel.y = dir.y * 30 + crand()*40;
        p->vel.z = dir.z * 30 + crand()*40;

        p->accel.x = p->accel.y = 0;
        p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;

        // p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->alphavel = -2.0 + (frand()*0.75);
    }
}
*/

// Logic [6/22/99]: Holy shit... this ... isn't ... *right*
/*
===============
CL_DiminishingTrail

===============
*/
void CL_DiminishingTrail (CVector &start, CVector &end, centity_t *old, int flags)
{
    cparticle_t *p;
    float       orgscale;
    float       velscale, fZSpeed, fDieTime = 0;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    float dec = 2.0;
    vec = vec * dec;

    if ( old->trailcount > 700 )
    {
        orgscale = 4;
        velscale = 15;
    }
    else 
	if ( old->trailcount > 400 )
    {
        orgscale = 2;
        velscale = 10;
    }
    else
    {
        orgscale = 1;
        velscale = 5;
    }

    //	Nelno:	separate loop for complex particles
    if ( flags & (EF_GIB | EF_SLUDGE | EF_SMOKETRAIL) )
    {
        while ( len > 0 )
        {
            len -= dec;

            if ( !free_comparticles )
            {
			    return;
			}

			// SCG[1/21/00]: moved here in case "if ( (rand()&128) < old->trailcount ):" fails.
			// SCG[1/21/00]: resultant else needs a valid particle too!
            if ( !(p=CL_SpawnComParticle()) )
            {
				return;
			}
            // drop less particles as it flies
            if ( (rand()&128) < old->trailcount )
            {
//                if ( !(p=CL_SpawnComParticle()) )
//                {
//				    return;
//				}

                p->time = cl.time;

                if ( flags & EF_GIB )
                {
                    if(frand() > 0.7)
                    {
                        fZSpeed = crand()*velscale;
                        fDieTime = CL_ParticleSplashTime(move, fZSpeed, PARTICLE_GRAVITY);
                    }
					else
					{
						fZSpeed = 0;
					}
                    //for(i=0;i<1;i++)
                    //{
                        // just for grins, crank it up, haven't seen a slowdown. hmmm..
                        p->type = PARTICLE_BLOOD + (rand() & 3); // 4 random blood particles;
                        p->alpha = 1.0;
                        p->alphavel = -1.0;//-1.0 / (1+frand()*0.4);
//                        p->color = 0x30 + (rand()&7);
				        p->color_rgb.Set(0.8,0,0);
                        p->pscale = frand()*5.0;
                        p->org.x = move.x + crand()*orgscale;
                        p->org.y = move.y + crand()*orgscale;
                        p->org.z = move.z + crand()*orgscale;

                        p->vel.x = crand()*velscale;
                        p->vel.y = crand()*velscale;
                        p->vel.z = fZSpeed;

                        p->accel.Zero();
                        p->accel.z = PARTICLE_GRAVITY;
                        if(fDieTime)
                        {
//                            p->flags = PF_ADDSPLASH;
                            p->die_time = fDieTime;
                        }
                        //if(i<2)
                          //  if ( !(p=CL_SpawnComParticle()) )
                          //      return;
	    			        
                    //}
                }
                else
                if ( flags & EF_SLUDGE )
                {
		            float spread=12;
	                float r;

                    p->type = PARTICLE_SMOKE;
                    //p->type = PARTICLE_POISON;
					p->alpha = 0.85;
                    p->pscale=0.95;
					// p->alphavel = -1.0 / (1+frand()*0.2);
                    p->alphavel = -1.0 + (frand()*0.50);
					
					//p->color_rgb.Set(0.13,0.25,0.05);
					p->color_rgb.Set(1.0,1.0,1.0);

					r = frand()*spread - ((float)spread/2);
					p->org.x = move.x + r ;
					p->org.y = move.y + r ;
					p->org.z = move.z + (r*crand());

                    p->vel.x = crand()*velscale;
                    p->vel.y = crand()*velscale;
                    p->vel.z = crand()*velscale;

                    p->accel.z = -20;
                }
            }
            else
            if ( flags & EF_SMOKETRAIL )
            {
                float spread=2,maxvel=2.2;
                float r;

                p->time = cl.time;
                p->pscale=.4;

                p->type = PARTICLE_SMOKE;
                p->alpha = .5;
                p->alphavel = -(.5 + frand()*.09);      
                // r = -1.0 / (1+frand()*0.2);
                p->alphavel = -1.0 - (frand()*0.8333);
//                p->color = 4 + (rand()&7);
				p->color_rgb.Set( 1.0, 1.0, 1.0 );

                r = frand()*spread - ((float)spread/2);
                p->org.x = move.x + r;
                p->org.y = move.y + r;
                r = 0;
                p->org.z = move.z + r;

                p->vel.x = 0;
                p->vel.y = frand()*maxvel - maxvel/2;
                p->vel.z = frand()*maxvel - maxvel/2;

                p->accel.z = maxvel;
            }

            old->trailcount -= 5;
            if ( old->trailcount < 100 )
            {
			    old->trailcount = 100;
            }
			move = move + vec;
        }
    }
    else
    {
        while ( len > 0 )
        {
            len -= dec;

            if ( !free_particles )
            {
			    return;
			}

            // drop less particles as it flies
            if ( (rand()&1023) < old->trailcount )
            {
                if ( !(p=CL_SpawnParticle()) )
                {
				    return;
				}

                p->time = cl.time;

                if ( flags & EF_GREENGIB )
                {
                    p->alpha = 1.0;
                    p->alphavel = -1.0 / (1+frand()*0.4);
//                    p->color = 0xd0 + (rand()&7);
					p->color_rgb.Set( 1.0, 1.0, 1.0 );

                    p->org.x = move.x + crand()*orgscale;
                    p->org.y = move.y + crand()*orgscale;
                    p->org.z = move.z + crand()*orgscale;

                    p->vel.x = crand()*velscale;
                    p->vel.y = crand()*velscale;
                    p->vel.z = crand()*velscale;
                    p->vel.z = PARTICLE_GRAVITY;

                    p->accel.Zero();
                }
                else
                {
                    p->alpha = 1.0;
                    // p->alphavel = -1.0 / (1+frand()*0.2);
                    p->alphavel = -1.0 - (frand()*0.8333);
//                    p->color = 4 + (rand()&7);
					p->color_rgb.Set( 1.0, 1.0, 1.0 );

                    p->org.x = move.x + crand()*orgscale;
                    p->org.y = move.y + crand()*orgscale;
                    p->org.z = move.z + crand()*orgscale;

                    p->vel.x = crand()*velscale;
                    p->vel.y = crand()*velscale;
                    p->vel.z = crand()*velscale;
                    p->accel.z = 20;
                }
            }

            old->trailcount -= 5;
            if ( old->trailcount < 100 )
            {
			    old->trailcount = 100;
            }
			move = move + vec;
        }
    }
}

void MakeNormalVectors (CVector &forward, CVector &right, CVector &up)
{
    // this rotate and negat guarantees a vector
    // not colinear with the original
    right.y = -forward[0];
    right.z = forward[1];
    right.x = forward[2];

    float d = DotProduct (right, forward);
    VectorMA (right, forward, -d, right);
    right.Normalize();
    CrossProduct (right, forward, up);
}

/*
===============
CL_RocketTrail

===============
*/
/*
void CL_RocketTrail (CVector &start, CVector &end, centity_t *old)
{
    CVector     move;
    CVector     vec;
    float       len;
    cparticle_t *p;
    float       dec;

    // smoke
    CL_DiminishingTrail (start, end, old, EF_ROCKET);

    // fire
    move = start;
    vec = end - start;
    len = vec.Normalize();

    dec = 1;
    vec = vec * dec;

    //	Nelno:	complex particle sparks
    while ( len > 0 )
    {
        len -= dec;

        if ( !free_comparticles )
        {
		    return;
		}

        if ( (rand()&7) == 0 )
        {
            if ( !(p=CL_SpawnComParticle()) )
            {
			    return;
			}

            p->accel.Zero();
            p->time = cl.time;

            p->type = PARTICLE_SPARKS;
            p->alpha = 1.0;
            // p->alphavel = -1.0 / (1+frand()*0.2);
            p->alphavel = -1.0 - (frand()*0.8333);
            p->color = 0x48 + (rand()&3);

            p->org.x = move.x + crand()*5;
            p->org.y = move.y + crand()*5;
            p->org.z = move.z + crand()*5;

            p->vel.x = crand()*20;
            p->vel.y = crand()*20;
            p->vel.z = crand()*20;
            p->accel.z = -PARTICLE_GRAVITY;
        }
        move = move + vec;
    }
}
*/
//---------------------------------------------------------------------------
// CL_MeteorTrail()
//---------------------------------------------------------------------------
void CL_MeteorTrail(CVector &start, CVector &end, centity_t *old, int flags)
{
    CVector     move;
    CVector     vec;
    float       len;
    int         count,r;
    cparticle_t *p;
    float       dec;
    float       orgscale;
    float       velscale;

    // SMOKE
    if ( flags & EF_METEOR )
    {
        move = start;
        vec = end - start;
        len = vec.Normalize();

        dec = 0.5;
	    vec = vec * dec;

        if ( old->trailcount > 900 )
        {
            orgscale = 4;
            velscale = 15;
        }
        else 
		if ( old->trailcount > 800 )
        {
            orgscale = 2;
            velscale = 10;
        }
        else
        {
            orgscale = 1;
            velscale = 5;
        }

        while ( len > 0 )
        {
            len -= dec;

            if ( !free_comparticles )
            {
			    return;
			}

            // drop less particles as it flies
            if ( (rand()&1023) < old->trailcount )
            {
                if ( !(p=CL_SpawnComParticle()) )
                {
				    return;
				}

                p->time = cl.time;

                p->pscale=4.5;
                p->type = PARTICLE_SMOKE;
                p->alpha = 1.0;
                // p->alphavel = -1.0 / (1+frand()*0.2);
                p->alphavel = -1.0 - (frand()*0.8333);
//                p->color = 4 + (rand()&7);
				p->color_rgb.Set( 0.1, 0.1, 0.1);

                r = crand() > .5 ? crand()*orgscale*20 : -crand()*orgscale*20;
                p->org.x = move.x + r;
                r = crand() > .5 ? crand()*orgscale*20 : -crand()*orgscale*20;
                p->org.y = move.y + r;
                r = crand() > .5 ? crand()*orgscale*20 : -crand()*orgscale*20;
                p->org.z = move.z + r;

                p->vel.y = crand()*velscale;
                p->vel.z = crand()*velscale;

                p->accel.z = 20;
            }

            old->trailcount -= 5;
            if ( old->trailcount < 100 )
            {
			    old->trailcount = 100;
            }
			move = move + vec;
        }
    }

    // FIRE
    if ( flags & EF_FIRE )
    {
        count=2;
        while ( count-- )
        {
            move = start;
            vec = end - start;
            len = vec.Normalize();

            dec = 1;
            //vec = vec * dec;// SCG[1/21/00]: WTF?  vec = vec * 1?  Can you say REDUNDANT?

            while ( len > 0 )
            {
                len -= dec;

                if ( !free_comparticles )
                {
				    return;
				}

                if ( (rand()&7) == 0 )
                {
                    if ( !(p=CL_SpawnComParticle()) )
                    {
					    return;
					}

                    p->accel.Zero();
                    p->time = cl.time;

                    p->type = PARTICLE_SPARKS;
                    p->alpha = 1.0;
                    // p->alphavel = -1.0 / (1+frand()*0.2);
                    p->alphavel = -1.0 - (frand()*0.8333);
//                    p->color = 0x48 + (rand()&3);
					p->color_rgb.Set( 1.0, 1.0, 1.0 );

                    r = crand() > .5 ? crand()*20 : -crand()*20;
                    p->org.x = move.x + r;
                    r = crand() > .5 ? crand()*20 : -crand()*20;
                    p->org.y = move.y + r;
                    r = crand() > .5 ? crand()*20 : -crand()*20;
                    p->org.z = move.z + r;

                    p->vel.x = crand()*20;
                    p->vel.y = crand()*20;
                    p->vel.z = crand()*20;

                    p->accel.z = PARTICLE_GRAVITY;
                }
                move = move + vec;
            }

            start.x += crand() > .5 ? crand()*20 : -crand()*20;
            start.y += crand() > .5 ? crand()*20 : -crand()*20;
            start.z += crand() > .5 ? crand()*20 : -crand()*20;
        }
    }
}

void CL_BlueSparkTrail ( CVector &start, CVector &end )
{
    cparticle_t *p;

    // blue sparks, man
    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    float dec = 0.5;
    vec.Multiply( dec );

    //	Nelno:	complex particle sparks
    while ( len > 0 )
    {
        len -= dec;

        if ( !free_comparticles )
        {
		    return;
		}

        if ( (rand()&7) == 0 )
        {
            if ( !(p=CL_SpawnComParticle()) )
            {
			    return;
			}

            p->accel.Zero();
            p->time = cl.time;

            p->type = PARTICLE_BLUE_SPARKS;
            p->alpha =  1;
            // p->alphavel = -1.0 / (1+frand()*0.2);
            p->alphavel = -0.1;// - (frand()*0.8333);
//            p->color = 0x48 + (rand()&3);

            p->org.x = move.x + crand()*5;
            p->org.y = move.y + crand()*5;
            p->org.z = move.z + crand()*5;

            p->vel.x = crand()*20;
            p->vel.y = crand()*20;
            p->vel.z = crand()*20;

            p->accel.z = PARTICLE_GRAVITY;
			p->color_rgb.Set( 1, 1, 1 );
        }
        move = move + vec;
    }
}

void CL_TridentTrail ( CVector &start, CVector &end )
{
    cparticle_t *p;
    trace_t     tr;
    CVector     diff = end - start;
    CVector     vecTraceStart, vecTraceEnd, vecVel;
    float       len = diff.Normalize();
    float       fHeight = 0;
    float       fDieTime = 0, fZVelocity;
    float       fAcceleration = PARTICLE_GRAVITY;

    // blue sparks, man
    CVector move = start;

    float dec = 0.5;
    diff.Multiply( dec );

    if(len)
    {
        vecTraceStart = start;
        vecTraceEnd = vecTraceStart;
        vecTraceEnd.z-=4096;
        fZVelocity = diff.z * len;
        tr = CL_TraceLine(vecTraceStart, vecTraceEnd, 1, MASK_ALL);
        if(tr.fraction < 1.0)
        {
            fHeight = vecTraceStart.z - tr.endpos.z;
            fDieTime = (-fZVelocity - sqrt(fZVelocity * fZVelocity - 2 * fAcceleration * fHeight)) / fAcceleration;
            fDieTime = cl.time + (fDieTime * 1000);
        }
        diff.Multiply( len );
        vecVel = diff;
    }
    //	Nelno:	complex particle sparks
    while ( len > 0 )
    {
        len -= dec;

        if ( !free_comparticles )
        {
		    return;
		}

        if ( (rand()&7) == 0 )
        {
            if ( !(p=CL_SpawnComParticle()) )
            {
			    return;
			}

            p->accel.Zero();
            p->time = cl.time;

            p->type = PARTICLE_BLUE_SPARKS;
            p->alpha =  .3;
            p->alphavel = -(.4+crand()*.2);
//            p->color = 0x48 + (rand()&3);

            p->org.x = move.x + crand()*5;
            p->org.y = move.y + crand()*5;
            p->org.z = move.z + crand()*5;

            p->vel = vecVel;
            p->vel.x += crand()*20;
            p->vel.y += crand()*20;
            p->flags = PF_ADDSPLASH;
            p->die_time = fDieTime;

            p->accel.z = fAcceleration;
			p->color_rgb.Set( 1, 1, 1 );
        }
        move = move + diff;
    }
}

void CL_C4Beep ( entity_s &ent )
{
	V_AddLight (ent.origin, 100, 1,0,0);

	void *blink = re.RegisterModel( "models/global/e_sflred.sp2", RESOURCE_GLOBAL );
	if (!blink)
		return;

	CVector dir = cl.refdef.vieworg - ent.origin;
	dir.Normalize();

	entity_t sprite;
	memset(&sprite,0,sizeof(sprite));
	sprite.model = blink;
	sprite.angles = ent.angles;
	sprite.origin = ent.origin + 6 * dir;
	sprite.render_scale.Set(.8,.8,.8);
	sprite.alpha = .9;
	sprite.flags=SPR_ALPHACHANNEL;

	V_AddEntity(&sprite);
}

void CL_StavrosAmmo ( CVector &start )
{
//	V_AddLight(start,90 + 30*frand(),0.8,0.4,0.2);

	CVector flamecolor(0.8 + 0.1*crand(),0.4 + 0.1*crand(), 0.2 + 0.1*crand());
	CVector vDirection(0,0,1);
	CVector	vAlphaInfo(0.8,1.0,0);
	CVector vGravity(0,0,2);
	CL_GenerateComplexParticle2(start,					// emission origin
								vDirection,						// some direction thinggy
								flamecolor,				// color
								vAlphaInfo,					// alpha (start, depletion, rand)
								3+2*crand(),						// scale
								2,									// count
								30,									// spread
								40,									// velocity
								PARTICLE_FIRE,						// particle type
								vGravity);					// gravity

	flamecolor.Set(0.8 + 0.1*crand(),0.4 + 0.1*crand(), 0.2 + 0.1*crand());
	CL_GenerateComplexParticle2(start,					// emission origin
								vDirection,						// some direction thinggy
								flamecolor,				// color
								vAlphaInfo,					// alpha (start, depletion, rand)
								3+2*crand(),						// scale
								2,									// count
								30,									// spread
								40,									// velocity
								PARTICLE_FIRE,						// particle type
								vGravity);					// gravity

	flamecolor.Set(0.1, 0.1, 0.1);
	vAlphaInfo.Set(0.6,0.5,0);
	vGravity.Set(0, 0, 4);
	CL_GenerateComplexParticle2(start,					// emission origin
								vDirection,						// some direction thinggy
								flamecolor,			// color
								vAlphaInfo,					// alpha (start, depletion, rand)
								1+0.5*crand(),						// scale
								1,									// count
								50,									// spread
								40,									// velocity
								PARTICLE_SMOKE,						// particle type
								vGravity);					// gravity
}

void CL_BoltTrail( CVector &start, CVector &end, centity_t *old)
{
    CVector dir;
    
    dir = end - start;
    dir.Normalize();
    CL_ParticleEffect_Complex(start,dir,CVector(1.0f, 0.4f, 1.0f), 10,PARTICLE_SPARKS );
}

void CL_DiscusSparkles(centity_t *cent)
{
	CVector start = cent->lerp_origin;
	CVector dir = cent->prev.origin - cent->current.origin;
	dir.Normalize();

	CVector ds_color(1,1,1);
	CVector ds_alpha(0.4,6.75,0);
	CVector ds_grav(0,0,0);

	CL_GenerateComplexParticle2(start,						// emission origin
								dir,						// some direction thinggy
								ds_color,					// color
								ds_alpha,					// alpha (start, depletion, rand)
								0.55+0.25*frand(),				// scale
								4,							// count
								15,							// spread
								120,							// velocity
								PARTICLE_SMOKE,			// particle type
								ds_grav);					// gravity
}

void CL_SparkleTrail ( CVector &start )
{
    cparticle_t *p;
	int i;
	//  Noel :  Tone down the 'complex' particle sparks.  Just a weee tad too many there killer.
    for(i=0;i<3;i++)
    {
        if ( !free_comparticles )
        {
		    return;
		}

        if ( (rand()&7) == 0 )
        {
            if ( !(p=CL_SpawnComParticle()) )
            {
			    return;
			}

            p->accel.Zero();
            p->time = cl.time;

            if ( rand () & 1 )
            {
			    p->type = PARTICLE_SPARKLE1;
            }
			else
            {
			    p->type = PARTICLE_SPARKLE2;
			}

            p->alpha = 1.0;
            p->alphavel = -1.0 - (frand()*0.8333);
            //p->color = 0x48 + (rand()&3); Ewe nasty shit...
			//This sets the color to be fucking blue
			p->color_rgb.Set(0.15f, 0.15f, 0.65);
            p->pscale = crand () * 2.0;

            p->org.x = start.x + crand()*5;
            p->org.y = start.y + crand()*5;
            p->org.z = start.z + crand()*5;

            p->vel.x = crand()*20;
            p->vel.y = crand()*20;
            p->vel.z = crand()*20;

            p->accel.z = PARTICLE_GRAVITY;
			p->color_rgb.Set( 1, 1, 1 );
        }
    }
}





//----------------------------------------------------------------------
// void CL_SparkleTrail_OPT ( CVector &start, CVector &end, centity_t *old)
//
//  Description:
//      Sparkly particle effects with options. Hopefully we won't 
//      see another bazillion fx routines that all look the same with
//      a modified cparticle_t struct being the only difference.
//  
//  Parameters:
//
//  Returns:
//      nada y nada
//
//  BUGS:
//
/*
void CL_SparkleTrail_OPT ( CVector &start, CVector &end, centity_t *old, int speed, int type)
{
    cparticle_t *p;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    float dec = 0.5;
    vec = vec * dec;

    while ( len > 0 )
    {
        len -= dec;

        if ( !free_comparticles )
        {
		    return;
		}

        if ( (rand()&7) == 0 )
        {
            if ( !(p=CL_SpawnComParticle()) )
            {
			    return;
			}

            p->accel.Zero();
            p->time = cl.time;
            p->type = type;
            p->alpha = 1;
            // p->alphavel = -1 / (1+frand()*0.2);
            p->alphavel = -1.0 - (frand()*0.8333);
//            p->color = 0x48 + (rand()&3);
			p->color_rgb.Set( 1.0, 1.0, 1.0 );
            p->pscale = crand () * 2.0;

            p->org.x = move.x + crand()*5;
            p->org.y = move.y + crand()*5;
            p->org.z = move.z + crand()*5;

            p->vel.x = crand()*speed;
            p->vel.y = crand()*speed;
            p->vel.z = crand()*speed;

            p->accel.z = 0;//-PARTICLE_GRAVITY;
        }
        move = move + vec;
    }
}
*/
/*
===============
CL_RailTrail

===============
*/
/*
void CL_RailTrail ( CVector &start, CVector &end)
{
    cparticle_t *p;
    float       dec;
    CVector     right, up;
    float       d, c, s;
    CVector     dir;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    MakeNormalVectors (vec, right, up);

    for ( int i = 0; i < len; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->accel.Zero();

        d = i * 0.1;
        c = cos(d);
        s = sin(d);

        dir = right * c;
        VectorMA (dir, up, s, dir);

        p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 - (frand()*0.8333);
        p->color = 0x74 + (rand()&7);

        p->org.x = move.x + dir.x*3;
        p->org.y = move.y + dir.y*3;
        p->org.z = move.z + dir.z*3;

        p->vel.x = dir.x*6;
        p->vel.y = dir.y*6;
        p->vel.z = dir.z*6;

        move = move + vec;
    }

    dec = 0.75;
    vec = vec * dec;
    move = start;

    while ( len > 0 )
    {
        len -= dec;

        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->accel.Zero();

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.6+frand()*0.2);
        p->color = 0x0 + rand()&15;

        p->org.x = move.x + crand()*3;
        p->org.y = move.y + crand()*3;
        p->org.z = move.z + crand()*3;

        p->vel.x = crand()*3;
        p->vel.y = crand()*3;
        p->vel.z = crand()*3;

        p->accel.Zero();

        move = move + vec;
    }
}
*/

/*
===============
CL_PlasmaTrail

===============
*/
/*
void CL_PlasmaTrail ( CVector &start, CVector &end)
{
    cparticle_t *p;
    float		dec;
    CVector		right, up;
    float		d, c, s;
    CVector		dir;
    float		dacnt=1;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    MakeNormalVectors (vec, right, up);

    for ( int i = 0; i < len; i++ )
    {
        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->accel.Zero();

        d = i * 0.1;

        c = sin(d) * sin(d);
        s = cos(d);

        dir = right * c;
        VectorMA (dir, up, s, dir);

        p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 - (frand()*0.8333);
        p->color = 0xd0 + (rand()&7);

        dacnt+=0.1;
        if ( dacnt > 4 )
        {
		    dacnt=1;
		}

        p->org.x = move.x + dir.x*dacnt;
        p->org.y = move.y + dir.x*dacnt;
        p->org.z = move.z + dir.x*dacnt;

        p->vel = dir;

        move = move + vec;
    }

    dec = 0.75;
    vec = vec * dec;
    move = start;

    while ( len > 0 )
    {
        len -= dec;

        if ( crand() > 0.98 && len > 8 )
        {
            if ( !(p=CL_SpawnParticle()) )
            {
			    return;
			}

            p->time = cl.time;
            p->accel.Zero();

            p->alpha = 1.0;
            p->alphavel = -1.0 / (0.6+frand()*0.2);

            p->color = 0xd0 + (rand()&7);

            p->org.x = move.x + crand()*3;
            p->org.y = move.y + crand()*3;
            p->org.z = move.z + crand()*3;

            p->vel.x = crand()*3;
            p->vel.y = crand()*3;
            p->vel.z = crand()*3;

            p->accel.Zero();
        }
        move = move + vec;
    }

}
*/
/*
===============
CL_BoomerTrail

===============
*/
/*
void CL_BoomerTrail ( CVector &start, CVector &end)
{
    cparticle_t *p;
    int     left = 0;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    int dec = 5;
    vec = vec * 5;

    while ( len > 0 )
    {
        len -= dec;

        if ( !(p=CL_SpawnParticle()) )
        {
		    return;
		}

        p->time = cl.time;
        p->alpha = 0.5;
        p->alphavel = -1.0 / (0.3 + frand()*0.2);
        p->color = 0xe4 + (rand()&3);

        p->org = move;
        p->accel.Zero();
        if ( left )
        {
            left = 0;
            p->vel.x = 10;
        }
        else
        {
            left = 1;
            p->vel.x = -10;
        }

        p->vel.y = 0;
        p->vel.z = 0;

        move = move + vec;
    }

}
*/

/*
===============
CL_BubbleTrail

===============
*/
/*
void CL_BubbleTrail ( CVector &start, CVector &end)
{
    cparticle_t *p;

    CVector move = start;
    CVector vec = end - start;
    float len = vec.Normalize();

    float dec = 32;
    vec = vec * dec;

    for ( float i = 0; i < len; i += dec )
    {
        if ( !(p=CL_SpawnComParticle()) )
        {
		    return;
		}

        p->accel.Zero();
        p->time = cl.time;

        p->alpha = 1.0;
        // p->alphavel = -1.0 / (1+frand()*0.2);
        p->alphavel = -1.0 - (frand()*0.8333);
        p->color = 4 + (rand()&7);
		p->color_rgb.Set( 1.0, 1.0, 1.0 );

        p->org.x = move.x + crand()*2;
        p->org.y = move.y + crand()*2;
        p->org.z = move.z + crand()*2;

        p->vel.x = crand()*5;
        p->vel.y = crand()*5;
        p->vel.z = crand()*5;

        p->vel.z += 6;
        p->type = PARTICLE_BUBBLE;

        move = move + vec;
    }
}
*/

/*
===============
CL_FlyParticles
===============
*/

//#define	BEAMLENGTH			16
/*
void CL_FlyParticles ( CVector &origin, int count, int color = 0, float alpha = 1.0f, int beamlength = 16, int ptype = 0);


void CL_FlyParticles ( CVector &origin, int count, int color, float alpha, int beamlength, int ptype)
{
    int         i;
    cparticle_t *p;
    float       angle;
    float       sr, sp, sy, cr, cp, cy;
    CVector     forward;
    float       dist = 6; //64;
    float       ltime;

    if ( count > NUMVERTEXNORMALS )
        count = NUMVERTEXNORMALS;

    if ( !avelocities[0].x )
    {
        for ( i = 0; i < NUMVERTEXNORMALS; i++ )
        {
//            avelocities[0].Set( (rand()&255) * 0.01, (rand()&255) * 0.01, (rand()&255) * 0.01 );
// this is an actual bug!  
            avelocities[i].Set( (rand()&255) * 0.01, (rand()&255) * 0.01, (rand()&255) * 0.01 );

        }
    }

    //ltime = (float)cl.time / 1000.0;
  
    // ltime = (float)cl.time / 256.0;       // lower the time, faster the energy
    ltime = (float)cl.time * 0.00390625;

    for ( i = 0; i < count; i+=2 )
    {
        angle = ltime * avelocities[i].x;
        sy = sin(angle);
        cy = cos(angle);
        angle = ltime * avelocities[i].y;
        sp = sin(angle);
        cp = cos(angle);
        angle = ltime * avelocities[i].z;
        sr = sin(angle);
        cr = cos(angle);

        forward.x = cp*cy;
        forward.y = cp*sy;
        forward.z = -sp;

        //if ( !(p=CL_SpawnParticle()) )
        if ( !(p=CL_SpawnComParticle()) )   // changed for sparkles
        {
		    return;
		}

        p->time = cl.time;
        p->type = ptype;
        
        //dist = sin(ltime + i) * 64;
        dist = sin(ltime + i) * 6;     // determines max range out from origin
       
        p->org.x = origin.x + avertexnormals.GetNormal(i)[0] * dist + forward.x * beamlength;
        p->org.y = origin.y + avertexnormals.GetNormal(i)[1] * dist + forward.y * beamlength;
        p->org.z = origin.z + avertexnormals.GetNormal(i)[2] * (dist * 2) + forward.z * beamlength; // elongate

        p->vel.Zero();
        p->accel.Zero();

        p->color = color;
        p->colorvel = 0;

        p->alpha = alpha;
        p->alphavel = -100;
    }
}
*/

void CL_BoostParticles ( CVector &origin, int count, float alpha, int ptype, CVector& color )
{
    int         i;
    cparticle_t *p;
    float       dist, ltime;

    if (count>NUMVERTEXNORMALS)
        count=NUMVERTEXNORMALS;
    
    ltime = (float)cl.time / 256.0;       // lower the time, faster the energy

    for ( i = 0; i < count; i+=2 )
    {
        if ( !(p=CL_SpawnComParticle()) )   // changed for sparkles
          return;

        p->time = cl.time;
        p->type = ptype;
        
        dist = sin(ltime + i) * 6;     // determines max range out from origin

        p->org.x = origin.x + avertexnormals.GetNormal(i)[0] * dist;
        p->org.y = origin.y + avertexnormals.GetNormal(i)[1] * dist;
        p->org.z = origin.z + avertexnormals.GetNormal(i)[2] * (dist * 2); // elongate

        p->vel.Zero();
        p->accel.Zero();

        p->alpha = alpha;
        p->alphavel = -100;
		p->color_rgb = color;
    }
}


//#define BOOSTFX_STOPTIME (30000.0)
//#define BOOSTFX_KILLTIME (10000.0)
#define BOOSTFX_MAXCOUNT (24)

typedef struct boost_light_s
{
  float r;
  float g;
  float b;
  int size;

} boost_light_t;


void CL_BoostEffects(CVector &origin, int fx_flag)
{
    int     ptype, light_idx;
    CVector center_org, color;
    boost_light_t blight[5] = { 
                                // r    g    b   size
                                { 0.0, 0.0, 1.0, 105 },  // acro
                                { 1.0, 0.0, 0.0, 105 },  // attack
                                { 0.0, 1.0, 0.0, 105 },  // power
                                { 1.0, 1.0, 0.0, 105 },  // speed
                                { 0.8, 0.2, 1.0, 105 },  // vita
                              };

    // try to center particles in middle of boost model
    center_org.x = origin.x;
    center_org.y = origin.y; 
    center_org.z = origin.z + 4.0f;
	light_idx = 0;
    
	// SCG[8/17/99]: Doh. I Broke this, now it's fixed. :P
	if( fx_flag & EF2_BOOSTFX_ACRO )
	{
		light_idx = 0;
	}
	else if( fx_flag & EF2_BOOSTFX_ATTACK )
	{
		light_idx = 1;
	}
	else if( fx_flag & EF2_BOOSTFX_POWER )
	{
		light_idx = 2;
	}
	else if( fx_flag & EF2_BOOSTFX_SPEED )
	{
		light_idx = 3;
	}
	else if( fx_flag & EF2_BOOSTFX_VITA )
	{
		light_idx = 4;
	}

	// SCG[6/22/99]: Set the particle type
	ptype = PARTICLE_SPARKLE1;

	// SCG[6/22/99]: Set the particle color
	color.Set( blight[light_idx].r, blight[light_idx].g, blight[light_idx].b );

    CL_BoostParticles (center_org, BOOSTFX_MAXCOUNT, 0.7f, ptype, color);

    // add little light to add to the 'magical' effect
    V_AddLight(center_org, blight[light_idx].size, blight[light_idx].r, blight[light_idx].g, blight[light_idx].b);
}

/*
===============
CL_TeleportParticles

===============
*/
void CL_TeleportParticles ( CVector &org)
{
    cparticle_t *p;
    float       vel;
    CVector     dir;

    for ( int i = -16; i <= 16; i+=4 )
    {
	    for ( int j = -16; j <= 16; j+=4 )
        {
		    for ( int k = -16; k <= 32; k+=4 )
            {
                if ( !(p=CL_SpawnParticle()) )
                {
				    return;
				}

                p->time = cl.time;
//                p->color = 7 + (rand()&7);
				p->color_rgb.Set( 1.0, 1.0, 1.0 );

                p->alpha = 1.0;
                p->alphavel = -1.0 / (0.3 + (rand()&7) * 0.02);

                p->org.x = org.x + i + (rand()&3);
                p->org.y = org.y + j + (rand()&3);
                p->org.z = org.z + k + (rand()&3);

                dir.x = j*8;
                dir.y = i*8;
                dir.z = k*8;

                dir.Normalize();                        
                vel = 50 + (rand()&63);
				dir.Multiply( vel );
                p->vel = dir;

                p->accel.x = p->accel.y = 0;
                p->accel.z = PARTICLE_GRAVITY;
            }
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SpiralParticlePhysics
//
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    CVector org;
    int     msec_duration;      //	how long this particle lives in milliseconds
    int     msec_frame;         //	inc frame when <= 0
    int     msec_frame_time;    //	milliseconds between frames
    int     scale_counter;
    float   alpha_div;
    float   speed;
	short	dir;				// [cek] control direction!
} spiralHook_t;

#define	RADIAN_DIVISOR (180 / 3.14159265359)

///////////////////////////////////////////////////////////////////////////////
//	CL_SpiralParticlePhysics_XY
//
//	moves particles in a spiral around an origin in the xy plane
//	z origin remains constant
///////////////////////////////////////////////////////////////////////////////
void CL_SpiralParticlePhysics_XY (cparticle_t *p)
{
    CVector         dir_to_org, dir_right;
    spiralHook_t    *hook = (spiralHook_t *)p->pHook;
    float           time, inward_speed, ang;
    int             msec;

    //	find direction to origin without z
    dir_to_org.x = hook->org.x - p->org.x;
    dir_to_org.y = hook->org.y - p->org.y;
    dir_to_org.z = 0;

    //	normalize it
    if ( dir_to_org.Normalize() < 3.0 )
    {
        //	remove particle if close to the center
        p->die_time = cl.time - 100;
        return;
    }

    //	get direction to the right of this
    dir_right.x = dir_to_org[1];
    dir_right.y = - dir_to_org[0];
    dir_right.z = dir_to_org[2];

    inward_speed = hook->speed * 0.25;
    p->vel.x = dir_right.x * hook->speed + dir_to_org.x * inward_speed;
    p->vel.y = dir_right.y * hook->speed + dir_to_org.y * inward_speed;
    p->vel.z = dir_right.z * hook->speed + dir_to_org.z * inward_speed;

    msec = (cl.time - p->time);
    time = msec * 0.001;
    p->time = cl.time;

    hook->msec_frame -= msec;
    if ( hook->msec_frame <= 0 )
    {
        hook->msec_frame = hook->msec_frame_time;
        p->type += 1;
        if ( p->type > PARTICLE_SPARKLE2 )
        {
		    p->type = PARTICLE_SPARKLE1;
		}
	}

    p->org = p->org + p->vel * time;

    //	set alpha based on how much time is left
    time = p->die_time - cl.time;
    //	generate a fake angle based on how long this particle lives
    //	angle will be 0 at birth, 90 and 1/2 life, 180 at death
    ang = (time - hook->msec_duration) / hook->alpha_div;
    p->alpha = sin (ang);

    hook->scale_counter += 5;
    if ( hook->scale_counter > 179 )
    {
	    hook->scale_counter = 0;
    }
	p->pscale = sin (hook->scale_counter / RADIAN_DIVISOR) + 1.0;
	p->color_rgb.Set( 1, 1, 1 );

    hook->speed *= 1.005;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SpirtalParticlePhysics_XYZ
//
//
///////////////////////////////////////////////////////////////////////////////
void CL_SpiralParticlePhysics_XYZ (cparticle_t *p)
{
    CVector         dir_to_org, dir_right;
    spiralHook_t    *hook = (spiralHook_t *)p->pHook;
    float           time, inward_speed, ang;
    int             msec;

    //	find direction to origin without z
    dir_to_org = hook->org - p->org;

    //	normalize it
    if ( dir_to_org.Normalize() < 3.0 )
    {
        //	remove particle if close to the center
        p->die_time = cl.time - 100;
        return;
    }

    //	get direction to the right of this
    dir_right.x = dir_to_org[1];
    dir_right.y = - dir_to_org[0];
    dir_right.z = 0;//dir_to_org [2];

    inward_speed = hook->speed * 0.25;
    p->vel.x = dir_right.x * hook->speed + dir_to_org.x * inward_speed;
    p->vel.y = dir_right.y * hook->speed + dir_to_org.y * inward_speed;
    p->vel.z = dir_right.z * hook->speed + dir_to_org.z * inward_speed;

    msec = (cl.time - p->time);
    time = msec * 0.001;
    p->time = cl.time;

    hook->msec_frame -= msec;
    if ( hook->msec_frame <= 0 )
    {
        hook->msec_frame = hook->msec_frame_time;
        p->type += 1;
        if ( p->type > PARTICLE_SPARKLE2 )
        {
		    p->type = PARTICLE_SPARKLE1;
		}
	}

    p->org = p->org + p->vel * time;

    //	set alpha based on how much time is left
    time = p->die_time - cl.time;
    //	generate a fake angle based on how long this particle lives
    //	angle will be 0 at birth, 90 and 1/2 life, 180 at death
    ang = (time - hook->msec_duration) / hook->alpha_div;
    p->alpha = sin (ang);

    hook->scale_counter += 5;
    if ( hook->scale_counter > 179 )
        hook->scale_counter = 0;
    p->pscale = sin (hook->scale_counter / RADIAN_DIVISOR) + 1.0;
	p->color_rgb.Set( 1, 1, 1 );

    hook->speed *= 1.005;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SpiralParticlePhysics_XY_2
//
// doesn't mess with the color/type of the friggin particle
///////////////////////////////////////////////////////////////////////////////
void CL_SpiralParticlePhysics_XY_2 (cparticle_t *p)
{
    CVector         dir_to_org, dir_right;
    spiralHook_t    *hook = (spiralHook_t *)p->pHook;
    float           time, inward_speed, ang;
    int             msec;

    //	find direction to origin without z
    dir_to_org.x = hook->org.x - p->org.x;
    dir_to_org.y = hook->org.y - p->org.y;
    dir_to_org.z = 0;

    //	normalize it
    if ( dir_to_org.Normalize() < 3.0 )
    {
        //	remove particle if close to the center
        p->die_time = cl.time - 100;
        return;
    }

    //	get direction to the right of this
    dir_right.x = dir_to_org[1];
    dir_right.y = - dir_to_org[0];
    dir_right.z = dir_to_org[2];

    inward_speed = hook->speed * 0.25;
    p->vel.x = dir_right.x * hook->speed + dir_to_org.x * inward_speed;
    p->vel.y = dir_right.y * hook->speed + dir_to_org.y * inward_speed;
    p->vel.z = dir_right.z * hook->speed + dir_to_org.z * inward_speed;

    msec = (cl.time - p->time);
    time = msec * 0.001;
    p->time = cl.time;

    hook->msec_frame -= msec;
    if ( hook->msec_frame <= 0 )
    {
        hook->msec_frame = hook->msec_frame_time;
	}

    p->org = p->org + p->vel * time;

    //	set alpha based on how much time is left
    time = p->die_time - cl.time;
    //	generate a fake angle based on how long this particle lives
    //	angle will be 0 at birth, 90 and 1/2 life, 180 at death
    ang = (time - hook->msec_duration) / hook->alpha_div;
    p->alpha = sin (ang);

    hook->scale_counter += 5;
    if ( hook->scale_counter > 179 )
    {
	    hook->scale_counter = 0;
    }
	p->pscale = sin (hook->scale_counter / RADIAN_DIVISOR) + 1.0;

    hook->speed *= 1.005;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_SpirtalParticlePhysics_XYZ_2
//
//	doesn't mess with the appearance of the friggin particle
///////////////////////////////////////////////////////////////////////////////
void CL_SpiralParticlePhysics_XYZ_2 (cparticle_t *p)
{
    CVector         dir_to_org, dir_right;
    spiralHook_t    *hook = (spiralHook_t *)p->pHook;
    float           time, inward_speed, ang;
    int             msec;

    //	find direction to origin without z
    dir_to_org = hook->org - p->org;

    //	normalize it
    if ( dir_to_org.Normalize() < 3.0 )
    {
        //	remove particle if close to the center
        p->die_time = cl.time - 100;
        return;
    }

    //	get direction to the right of this
	if (hook->dir)
	{
		dir_right.x = dir_to_org[1];
		dir_right.y = - dir_to_org[0];
	}
	else
	{
		dir_right.x = - dir_to_org[1];
		dir_right.y = dir_to_org[0];
	}
	dir_right.z = 0;//dir_to_org [2];

    inward_speed = hook->speed * 0.25;
    p->vel.x = dir_right.x * hook->speed + dir_to_org.x * inward_speed;
    p->vel.y = dir_right.y * hook->speed + dir_to_org.y * inward_speed;
    p->vel.z = dir_right.z * hook->speed + dir_to_org.z * inward_speed;

    msec = (cl.time - p->time);
    time = msec * 0.001;
    p->time = cl.time;

    hook->msec_frame -= msec;
    if ( hook->msec_frame <= 0 )
    {
        hook->msec_frame = hook->msec_frame_time;
	}

    p->org = p->org + p->vel * time;

    //	set alpha based on how much time is left
    time = p->die_time - cl.time;
    //	generate a fake angle based on how long this particle lives
    //	angle will be 0 at birth, 90 and 1/2 life, 180 at death
    ang = (time - hook->msec_duration) / hook->alpha_div;
    p->alpha = sin (ang);

    hook->scale_counter += 5;
    if ( hook->scale_counter > 179 )
        hook->scale_counter = 0;
    p->pscale = sin (hook->scale_counter / RADIAN_DIVISOR) + 1.0;

    hook->speed *= 1.005;
}


///////////////////////////////////////////////////////////////////////////////
//	CL_SpiralParticles
//
//	creates a set of particles spread out randomly within a cube which
//	spriral towards the center and then wink out
///////////////////////////////////////////////////////////////////////////////


#define	MAX_SPIRAL_PARTICLES	128

void    CL_SpiralParticles (CVector &org, CVector &mins, CVector &maxs, int msec)
{
    cparticle_t     *p;
    spiralHook_t    *hook;
    CVector         center_org;

    //	find center of passed mins/maxs
    center_org.x = org.x + mins.x + ((maxs.x - mins.x) / 2);
    center_org.y = org.y + mins.y + ((maxs.y - mins.y) / 2);
    center_org.z = org.z + mins.z + ((maxs.z - mins.z) / 2);

    for ( int i = 0; i < MAX_SPIRAL_PARTICLES; i++ )
    {
        if ( !(p = CL_SpawnComParticle ()) )
        {
		    return;
		}

        p->pFunc = CL_SpiralParticlePhysics_XY;
        p->pHook = X_Malloc(sizeof(spiralHook_t), MEM_TAG_HOOK);
        hook = (spiralHook_t *)p->pHook;

        hook->speed = 20;

        //	find center of passed mins/maxs
        hook->org = center_org;

        p->org.x = org.x + mins.x + (rand() % (int) (maxs.x - mins.x));
        p->org.y = org.y + mins.y + (rand() % (int) (maxs.y - mins.y));
        p->org.z = org.z + mins.z + (rand() % (int) (maxs.z - mins.z));

        p->alpha = 0.0;
        p->type = PARTICLE_SPARKLE1 + ( rand () & 1 );
        hook->msec_frame_time = rand () % 750 + 50;
        hook->msec_frame = rand () % hook->msec_frame_time;
        p->time = cl.time;

        p->alphavel = 0;
        p->vel.Zero();
        p->accel.x = p->accel.y = p->accel.z;

        hook->msec_duration = msec;
        hook->alpha_div = -(msec / 3.14159265359);
        p->die_time = cl.time + msec;
		p->color_rgb.Set( 1, 1, 1 );

        hook->scale_counter = rand () % 180;
    }
}

void CL_SpiralParticles2 (CVector &org, CVector &mins, CVector &maxs, CVector &color, float speed, particle_type_t pt, int msec)
{
    cparticle_t     *p;
    spiralHook_t    *hook;
    CVector         center_org;

    //	find center of passed mins/maxs
    center_org.x = org.x + mins.x + ((maxs.x - mins.x) / 2);
    center_org.y = org.y + mins.y + ((maxs.y - mins.y) / 2);
    center_org.z = org.z + mins.z + ((maxs.z - mins.z) / 2);

    for ( int i = 0; i < MAX_SPIRAL_PARTICLES; i++ )
    {
        if ( !(p = CL_SpawnComParticle ()) )
        {
		    return;
		}

        p->pFunc = CL_SpiralParticlePhysics_XYZ_2;
        p->pHook = X_Malloc(sizeof(spiralHook_t), MEM_TAG_HOOK);
        hook = (spiralHook_t *)p->pHook;

        hook->speed = speed;

        //	find center of passed mins/maxs
        hook->org = center_org;//center_org;
		hook->org.z += mins.z;
		hook->dir = (short)(rand() & 0x01);

        p->org.x = org.x + mins.x + (rand() % (int) (maxs.x - mins.x));
        p->org.y = org.y + mins.y + (rand() % (int) (maxs.y - mins.y));
        p->org.z = org.z + mins.z + (rand() % (int) (maxs.z - mins.z));

        p->alpha = 0.0;
        p->type = pt;
        hook->msec_frame_time = rand () % 750 + 50;
        hook->msec_frame = rand () % hook->msec_frame_time;
        p->time = cl.time;

        p->alphavel = 0;
        p->vel.Zero();
        p->accel.x = p->accel.y = p->accel.z;

        hook->msec_duration = msec;
        hook->alpha_div = -(msec / 3.14159265359);
        p->die_time = cl.time + msec;
		p->color_rgb = color;

        hook->scale_counter = rand () % 180;
    }
}


// ---------------------------------------------------------------------------------------


typedef struct
{
    CVector org;
    int     msec_duration;      //	how long this particle lives in milliseconds
    int     msec_frame;         //	inc frame when <= 0
    int     msec_frame_time;    //	milliseconds between frames
    int     scale_counter;
    float   alpha_div;
    float   speed;
    bool    clockwise;   // rotate direction clockwise?
} doubleHelixHook_t;


//#define	RADIAN_DIVISOR (180 / 3.14159265359)

///////////////////////////////////////////////////////////////////////////////
//	CL_DoubleHelixPhysics
//
//	moves particles around an origin in the xy plane
//	z origin remains constant
///////////////////////////////////////////////////////////////////////////////
void CL_DoubleHelixPhysics(cparticle_t *p)
{
    CVector         dir_to_org, dir_right;
    doubleHelixHook_t *hook = (doubleHelixHook_t *)p->pHook;
    float           time, inward_speed; //, ang;
    int             msec;

    //	find direction to origin without z
    dir_to_org.x = hook->org.x - p->org.x;
    dir_to_org.y = hook->org.y - p->org.y;
    dir_to_org.z = 0;

    dir_to_org.Normalize();      //	normalize it

    if (hook->clockwise)
    {
      //	get direction to the right of this
      dir_right.x = - dir_to_org[1];
      dir_right.y = dir_to_org[0];
      dir_right.z = dir_to_org[2];
    }
    else
    {
      //	get direction to the right of this
      dir_right.x = dir_to_org[1];
      dir_right.y = - dir_to_org[0];
      dir_right.z = dir_to_org[2];
    }

    inward_speed = hook->speed * 0.25;  // causes movement towards the middle
 
    p->vel.x = dir_right.x * hook->speed + dir_to_org.x * inward_speed;
    p->vel.y = dir_right.y * hook->speed + dir_to_org.y * inward_speed;
    //p->vel.z = dir_right.z * hook->speed + dir_to_org.z * inward_speed;
    
    msec = (cl.time - p->time);
    //time = msec * 0.001;

    p->time = cl.time;

    hook->msec_frame -= msec;
    if ( hook->msec_frame <= 0 )
    {
        hook->msec_frame = hook->msec_frame_time;
        //p->type += 1;
        //if ( p->type > PARTICLE_SPARKLE2 )
        //{
		//    p->type = PARTICLE_SPARKLE1;
		//}
	}

    //time = msec * 0.001;
    time = msec * 0.001;
    //time = 0.001;
    time = 0.05f;

    p->org.x = p->org.x + p->vel.x * time;
    p->org.y = p->org.y + p->vel.y * time;
    p->org.z = p->org.z + p->vel.z * time;

    if (p->org.z > (hook->org.z * 2)) // close to the top bounding box?
    {
      p->die_time = cl.time - 100;
    }
                                   
    //	set alpha based on how much time is left
    time = p->die_time - cl.time;
    
}




//#define	MAX_DOUBLEHELIX_PARTICLES	64

void    CL_DoubleHelixParticles (CVector &org, CVector &mins, CVector &maxs, int msec)
{
    cparticle_t     *p;
    doubleHelixHook_t *hook;
    CVector         center_org;
    
    //	find center of passed mins/maxs
    center_org.x = org.x + mins.x + ((maxs.x - mins.x) / 2);
    center_org.y = org.y + mins.y + ((maxs.y - mins.y) / 2);
    center_org.z = org.z + mins.z + ((maxs.z - mins.z) / 2);

    
    for ( int i = 0; i < 2; i++ )
    {
        if ( !(p = CL_SpawnComParticle ()) )
        {
		    return;
		}

        p->pFunc = CL_DoubleHelixPhysics;
        p->pHook = X_Malloc(sizeof (doubleHelixHook_t), MEM_TAG_HOOK);
        hook = (doubleHelixHook_t *)p->pHook;

        hook->speed = 40;
        hook->clockwise = (i & 1);

        //	find center of passed mins/maxs
        hook->org = center_org;

        p->org.x = org.x + mins.x; // starting location of particle
        p->org.y = org.y + mins.y;
        //p->org.z = org.z + mins.z;
        p->org.z = center_org.z;

        p->alpha = 0.0;
        p->type = PARTICLE_SPARKLE1 + ( rand ()& 1 );

        hook->msec_frame_time = 0;  // frame to start showing
        hook->msec_frame = 0;

        p->time = cl.time;

        p->alphavel = 0;
        p->vel.Zero();
        p->vel.z =+ 2;                           // height climb speed
        p->accel.x = p->accel.y = p->accel.z;

        hook->msec_duration = msec;
        hook->alpha_div = -(msec / 3.14159265359);
        p->die_time = cl.time + msec;
		p->color_rgb.Set( 1, 1, 1 );

        hook->scale_counter = rand () % 180;
    }
  
    
}







///////////////////////////////////////////////////////////////////////////////
//	CL_BubbleCheckWater
//
///////////////////////////////////////////////////////////////////////////////

#define	BUBBLE_VELOCITY	50

typedef struct
{
    unsigned int     nextCheck_msec;     //	last time bubble checked its contents
    unsigned int     check_msec;         //	time between checks
} bubbleHook_t;

///////////////////////////////////////////////////////////////////////////////
//	CL_BubbleCheckWater
//
///////////////////////////////////////////////////////////////////////////////
void CL_BubbleCheckWater (cparticle_t *p)
{
    int             contents;
    float           time;
    bubbleHook_t    *pHook = (bubbleHook_t *) p->pHook;

    if ( pHook->nextCheck_msec <= cl.time )
    {
        contents = CM_PointContents (p->org, 0);

        if ( !(contents & MASK_WATER) )
        {
		    p->die_time = cl.time - 100;
        }
		else
        {
		    p->die_time = cl.time + (pHook->check_msec << 1);
		}

        pHook->nextCheck_msec = cl.time + pHook->check_msec;
    }
    else
    {
	    p->die_time = cl.time + (pHook->check_msec << 1);
	}

    time = (cl.time - p->time)*0.001;
    p->time = cl.time;

//	p->org.x = p->org.x + p->vel.x * time;
//	p->org.y = p->org.y + p->vel.y * time;
    p->org.z = p->org.z + p->vel.z * time;
}

///////////////////////////////////////////////////////////////////////////////
//	CL_BubbleParticle
//
//	these particles have actually check the contents at their location every
//	1/10 of a second, so they should be used sparingly!  Definitely not
//	necessary for effects where the top of the water volume is know, but
//	useful for bubbles coming off of players, etc.
///////////////////////////////////////////////////////////////////////////////
void CL_BubbleParticles ( CVector &org,  CVector &mins,  CVector &maxs, int numBubbles)
{
    cparticle_t     *p;
    bubbleHook_t    *hook;
    CVector         center_org;

    //	find center of passed mins/maxs
    center_org.x = org.x + mins.x + ((maxs.x - mins.x) / 2);
    center_org.y = org.y + mins.y + ((maxs.y - mins.y) / 2);
    center_org.z = org.z + mins.z + ((maxs.z - mins.z) / 2);

    for ( int i = 0; i < numBubbles; i++ )
    {
        if ( !(p = CL_SpawnComParticle ()) )
        {
		    return;
		}

        p->pFunc = CL_BubbleCheckWater;
		// cek[11-21-99] this was allocating a spiralHook's worth of memory
        p->pHook = X_Malloc (sizeof (bubbleHook_t), MEM_TAG_HOOK);
        hook = (bubbleHook_t *)p->pHook;

        hook->check_msec = 100;
        hook->nextCheck_msec = cl.time + hook->check_msec;

        // NSS[2/3/00]:Fixing a divide by Zero possibility
		int x,y,z;
		x = maxs.x - mins.x;
		y = maxs.y - mins.y;
		z = maxs.z - mins.z;
		if(x == 0)
			x = 1;
		if(y == 0)
			y = 1;
		if(z == 0)
			z = 1;
		p->org.x = org.x + mins.x + (rand() % (int) (x));
        p->org.y = org.y + mins.y + (rand() % (int) (y));
        p->org.z = org.z + mins.z + (rand() % (int) (z));

		/*p->org.x = org.x + mins.x + (rand() % (int) (maxs.x - mins.x));
        p->org.y = org.y + mins.y + (rand() % (int) (maxs.y - mins.y));
        p->org.z = org.z + mins.z + (rand() % (int) (maxs.z - mins.z));*/

        p->alpha = 0.75;
        p->alphavel = 0;
        p->die_time = cl.time + (hook->check_msec << 1);    //	never die unless out of water content
        p->time = cl.time;
        p->vel.x = p->vel.y = 0;//p->vel [2] = 20;
        p->vel.z = BUBBLE_VELOCITY;
        p->accel.Zero();
        p->type = PARTICLE_BUBBLE;
        p->pscale = frand () * 5.0 + 0.5;
		p->color_rgb.Set( 1.0, 1.0, 1.0 );
    }
}

/*
===============
CL_AddParticles
===============
*/
void CL_AddParticles (void)
{
    cparticle_t     *p, *next;
    float           alpha;
    float           time, time2;
    CVector         org, vColor;
    cparticle_t     *active, *tail;

    active = NULL;
    tail = NULL;

    for ( p = active_particles; p; p = next )
    {
        next = p->next;

        time = (cl.time - p->time)*0.001;
        alpha = p->alpha + time*p->alphavel;
        if ( alpha <= 0 && p->die_time == 0 )
        {
            if ( p->volume )
            {
			    if ( p->volume->pv_active > 0 )
                {
				    p->volume->pv_active--;
				}
			}

                // faded out
            if ( p->pHook )
            {
//                X_Free(p->pHook);
                p->pHook = NULL;
            }
            p->pFunc = NULL;
            p->next = free_particles;
            free_particles = p;

            continue;
        }

        //	Nelno:	check expiration time
        if ( p->die_time > 0 && p->die_time <= cl.time )
        {
            if ( p->volume )
            {
			    if ( p->volume->pv_active > 0 )
				{
				    p->volume->pv_active--;
				}
			}

            if ( p->pHook )
            {
//                X_Free(p->pHook);
                p->pHook = NULL;
            }
            p->pFunc = NULL;
            p->die_time = 0;
            p->next = free_particles;
            free_particles = p;
            continue;
        }

        p->next = NULL;
        if ( !tail )
        {
		    active = tail = p;
        }
		else
        {
            tail->next = p;
            tail = p;
        }

        if ( alpha > 1.0 )
        {
		    alpha = 1;
        }
//		color = p->color;
		vColor = p->color_rgb;

        if ( p->pFunc )
        {
		    p->pFunc (p);
        }
		else
        {
            time2 = time*time;

            org.x = p->org.x + p->vel.x*time + p->accel.x*time2;
            org.y = p->org.y + p->vel.y*time + p->accel.y*time2;
            org.z = p->org.z + p->vel.z*time + p->accel.z*time2;
        }

        //	Nelno:	added type
//        V_AddParticle (org, color, alpha, (particle_type_t)p->type, 0);
        V_AddParticleRGB( org, 0, alpha, (particle_type_t)p->type, vColor, 1.0 );
    }

    active_particles = active;
}

int beam_AddLaser ( CVector &start,  CVector &end,  CVector &rgbColor, float alpha, float radius, float endRadius, int texIndex, unsigned short flags);

////////////////////////////////////////////////////////////////////////
//  CL_SplatParticle(cparticle_t *p)
//
//  Description:
//      Leaves a surface sprite when it touches a surface. Unfortunately
//      the particle type has to be checked here. It would be worse
//      to add another member to the cparticle_t structure. It's 
//      recommended that only one particle in a group over an area have
//      this function set.
//
//  Parameters:
//      cparticle_t *p      particle to check. 
//
//  Return:
//      void
//
/*
===============
CL_AddComParticles
===============
*/
void CL_AddComParticles (void)
{
    CVector end;
    cparticle_t     *p, *next;
    float           alpha, pscale;
    float           time, time2;
    CVector         org,vec, vColor;
    cparticle_t     *active, *tail;

    active = NULL;
    tail = NULL;

    for ( p = active_comparticles; p; p = next )
    {
        next = p->next;

        time = (cl.time - p->time)*0.001;
        alpha = p->alpha + time*p->alphavel;
        if ( alpha <= 0 && p->die_time == 0 )
        {
            if ( p->volume )
            {
			    if ( p->volume->pv_active > 0 )
                {
				    p->volume->pv_active--;
				}
			}

            // faded out
            if ( p->pHook )
            {
//                X_Free(p->pHook);
                p->pHook = NULL;
            }
            p->pFunc = NULL;
            p->next = free_comparticles;
            p->volume = 0;
            free_comparticles = p;

            continue;
        }

        //	Nelno:	check expiration time
        if ( p->die_time > 0 && p->die_time <= cl.time )
        {
            if ( p->volume )
            {
			    if ( p->volume->pv_active > 0 )
				{
			        p->volume->pv_active--;
				}
			}

            if ( p->pHook )
            {
//                X_Free (p->pHook);
                p->pHook = NULL;
            }
            // should this particle leave a splash when it hits the ground?
            if ( p->flags & PF_ADDSPLASH )
            {
                // turn this fucking complex particle into a splash
                CL_AddDripSplash (p);
            }
            else
            {
                p->pFunc = NULL;
                p->die_time = 0;
                p->next = free_comparticles;
                p->volume = 0;
                free_comparticles = p;
                continue;
            }
        } 

        p->next = NULL;
        if ( !tail )
        {
		    active = tail = p;
        }
		else
        {
            tail->next = p;
            tail = p;
        }

        if ( alpha > 1.0 )
        {
		    alpha = 1;
        }

        if ( p->flags & PF_SCALEVEL )
        {
            pscale = p->pscale + time * p->scalevel;
        }
        else
        {
		    pscale = p->pscale;
		}

        if ( p->pFunc )
        {
            p->pFunc (p);

            if ( p->type==PARTICLE_BEAM_SPARKS )
            {
                vec = p->org - p->lastorg;
                VectorMA(p->org, vec, 2, end);
                beam_AddLaser(p->org, end, p->color_rgb, alpha,pscale,pscale*.5,BEAM_TEX_LASER,BEAMFLAG_RGBCOLOR);
                p->lastorg = p->org;
            }
            else
            {
                V_AddComParticle (p->org, 0, alpha, (particle_type_t)p->type, pscale, p->flags, p->color_rgb);
            }
        }
        else
        {
            time2 = time*time;

            org.x = p->org.x + p->vel.x*time + (p->accel.x*time2 * 0.5);
            org.y = p->org.y + p->vel.y*time + (p->accel.y*time2 * 0.5);
            org.z = p->org.z + p->vel.z*time + (p->accel.z*time2 * 0.5);

			if ( p->type==PARTICLE_BEAM_SPARKS )
            {
                vec = p->lastorg - org;
                VectorMA(org, vec, 2, end);
				if(p->lastorg.x == 0 && p->lastorg.y == 0 && p->lastorg.z == 0)
				{
					p->lastorg = org;
					continue;
				}
                beam_AddLaser(org, end, p->color_rgb, alpha, pscale,pscale*0.3,BEAM_TEX_LASERSPARK,BEAMFLAG_RGBCOLOR);
                p->lastorg = org;
            }
            else
                V_AddComParticle (org, 0, alpha, (particle_type_t)p->type, pscale, p->flags, p->color_rgb);
        }

    }

    active_comparticles = active;
}

/*
==============
CL_EntityEvent

An entity has just been parsed that has an event value
==============
*/
extern void *cl_sfx_footsteps[4];
extern void *cl_sfx_footsteps_wood[4];
extern void *cl_sfx_footsteps_metal[4];
extern void *cl_sfx_footsteps_stone[4];
extern void *cl_sfx_footsteps_glass[4];
extern void *cl_sfx_footsteps_ice[4];
extern void *cl_sfx_footsteps_snow[4];
extern void *cl_sfx_footsteps_puddle[4];
extern void *cl_sfx_footsteps_sand[4];
extern void *cl_sfx_footsteps_ladder_wood[3];
extern void *cl_sfx_footsteps_ladder_metal[5];

extern void *cl_sfx_respawn;
extern void *cl_sfx_teleport;
extern void *cl_sfx_fall[3];
extern void *cl_sfx_splash[3];
extern void *cl_sfx_swim[4];

//#define	EV_SPLASH	( EV_FOOTSTEP_PUDDLE + 1 )
void Bubble_Generator(entity_state_t *ent)
{
	entity_state_t	*cl_ent;
	CVector			mins;
	CVector			maxs;
	int				i;

	i = 3;

	if (ent->event == EV_SPLASH_BIG)
		i = 40;
	else if (ent->event == EV_SPLASH_MED)
		i = 25;
	else if (ent->event == EV_SWIM)
		i = 3;

	if ( ent->iflags & IF_CL_REALBBOX )
	{
		cl_ent = &cl_entities [ent->number].current;
		CL_BubbleParticles (ent->origin, cl_ent->mins, cl_ent->maxs, i);
	}
	else
	{
		mins.Set( -16, -16, -16 );
		maxs.Set( 16, 16, 16 );

		CL_BubbleParticles (ent->origin, mins, maxs, i);
	}
}


void CL_EntityEvent (entity_state_t *ent)
{
	CVector			mins;// = {-16, -16, -16};
	CVector			maxs;// = {16, 16, 16};
	CVector			org;
	
	switch (ent->event)
	{
	case EV_ITEM_RESPAWN:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_WEAPON, cl_sfx_respawn, 1, ATTN_IDLE, 0);
		CL_ItemRespawnParticles (ent->origin);
		break;
	case EV_PLAYER_TELEPORT:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_WEAPON, cl_sfx_teleport, 1, ATTN_IDLE, 0);
		CL_TeleportParticles (ent->origin);
		break;
	case EV_FOOTSTEP:
			S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps[rand()&3], 1, ATTN_NORM, 0);
		break;
	case EV_FALLSHORT:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_AUTO, cl_sfx_fall[0], 1, ATTN_NORM, 0);
		break;
	case EV_FALL:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_AUTO, cl_sfx_fall[1], 1, ATTN_NORM, 0);
		break;
	case EV_FALLFAR:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_AUTO, cl_sfx_fall[2], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_WOOD:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_wood [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_METAL:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_metal [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_STONE:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_stone [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_GLASS:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_glass [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_ICE:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_ice [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_SNOW:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_snow [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_PUDDLE:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_puddle [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_SAND:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_sand [(int)cl.time%4], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_LADDER_WOOD:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_ladder_wood[(int)cl.time%3], 1, ATTN_NORM, 0);
		break;
	case EV_FOOTSTEP_LADDER_METAL:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_footsteps_ladder_metal[(int)cl.time%5], 1, ATTN_NORM, 0);
		break;
	case EV_SPLASH_BIG:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_splash [0], 1, ATTN_NORM, 0);
		Bubble_Generator(ent);
		break;
	case EV_SPLASH_MED:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_splash [1], 1, ATTN_NORM, 0);
		Bubble_Generator(ent);
		break;
	case EV_SPLASH_SMALL:
		S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_splash [2], 1, ATTN_NORM, 0);
		break;
	case EV_SWIM:
		if(!(cl.time % 3))
		{
			S_StartSoundQuake2 (CVector(0,0,0), ent->number, CHAN_BODY, cl_sfx_swim[(int)(frand()*3.95)], 0.10f, ATTN_NORM, 0);
		}
		Bubble_Generator(ent);
		break;
	case EV_SUCKITDOWN:
		mins.Set( -16, -16, -16 );
		maxs.Set( 16, 16, 16 );

            org = ent->origin;
            org.z += 32;
            CL_BubbleParticles (org, mins, maxs, 25);
            break;
        case EV_DROWNING:
            mins.Set( -8, -8, -8 );
            maxs.Set( 8, 8, 8 );

            org = ent->origin;
            org.z += 32;
            CL_BubbleParticles (org, mins, maxs, 5);
            break;
        case EV_BUBBLES:
            mins.Set( -4, -4, -4 );
            maxs.Set( 4, 4, 4 );

            org = ent->origin;
            org.z += 32;
            CL_BubbleParticles (org, mins, maxs, 2);
            break;
    }
}


/*
==============
CL_ClearEffects

==============
*/
// SCG[8/10/99]: Not used
//extern void CL_ClearFlares (void);              //  Shawn:  Added for Daikatana
extern void CL_ClearPVolumes(void);             //  Shawn:  Added for Daikatana

void CL_ClearEffects (void)
{
    CL_ClearParticles ();
    CL_ClearDlights ();
    CL_ClearLightStyles ();
// SCG[8/10/99]: Not used
//    CL_ClearFlares ();              //  Shawn:  Added for Daikatana
    CL_ClearPVolumes();             //  Shawn:  Added for Daikatana
    CL_ClearFogInfo();              //  Shawn;  Added for Daiaktana
}


//---------------------------------------------------------------------------
// CL_PoisonSpray()
//---------------------------------------------------------------------------
void CL_PoisonSpray(CVector &start, CVector &end, float speed,short spread)
{
    cparticle_t *p;
    trace_t     tr;
    CVector     pos = start;
    CVector     diff = end - start;
    CVector     vecTraceStart, vecTraceEnd, vecVel;
    float       len = diff.Normalize();
    float       fHeight = 0;
    float       fDieTime = 0, fZVelocity;
    float       fAcceleration = PARTICLE_GRAVITY;

    if(len)
    {
        if(speed < 1.0f)
            speed = len * 10;
        vecTraceStart = start;
        vecTraceEnd = vecTraceStart;
        vecTraceEnd.z-=4096;
        fZVelocity = diff.z * speed;
        tr = CL_TraceLine(vecTraceStart, vecTraceEnd, 1, MASK_ALL);
        if(tr.fraction < 1.0)
        {
            fHeight = vecTraceStart.z - tr.endpos.z;
            fDieTime = (-fZVelocity - sqrt(fZVelocity * fZVelocity - 2 * fAcceleration * fHeight)) / fAcceleration;
            fDieTime = cl.time + (fDieTime * 1000);
        }
        vecVel = diff * speed;
        while ( len > 0 )
        {
            len--;
            if(frand() > 0.8)
            {
                if ( !(p=CL_SpawnComParticle()) )
                {
			        return;
			    }
                p->time = cl.time;
//                p->flags = PF_ADDSPLASH;
                p->type = PARTICLE_SPARKS;
                p->alpha = 0.3;
                p->color_rgb.Set(0.3f, 0.9f, 0.3f);
                p->org.x = pos.x + (crand() * spread);
                p->org.y = pos.y + (crand() * spread);
                p->org.z = pos.z + (crand() * spread);
                p->vel = vecVel;
                p->vel.x += (crand() * spread) * 10;
                p->vel.y += (crand() * spread) * 10;
                p->accel.z = fAcceleration;
                p->pscale = (frand() + 0.5) * 2;
                p->die_time = fDieTime;
            }
            pos = pos + diff;
        }
    }
}

//---------------------------------------------------------------------------
// CL_AddTrails()
//
// - to start trails, ent->s.renderfx = RF_INITTRAILS | RF_TRAILS
// - next frame, think() should clear RF_INITTRAILS
// - to stop trails, just clear RF_TRAILS
// - for now, uses modelindex2 as alternate trail model (if modelindex2 != 0)
//---------------------------------------------------------------------------
void CL_AddTrails(CVector &pos,centity_t *cent)
{
    entity_t ent;
    CVector vec;
    int loop,modelindex;

    // init?
    if ( cent->current.renderfx & RF_INITTRAILS )
    {
        memset(cent->trailPosList,0,sizeof(cent->trailPosList));
        cent->current.renderfx &= ~RF_INITTRAILS;                   // needs to be cleared on server side, too
    }

    // which model?
    modelindex=cent->current.modelindex2;
    if ( !modelindex )
    {
	    modelindex=cent->current.modelindex;
	}

    // if current position is different from last
    if ( memcmp( cent->trailPosList[0], pos, sizeof(CVector) ) )
    {
        // scroll positions through list
        memcpy(cent->trailPosList+1,cent->trailPosList,sizeof(cent->trailPosList)-sizeof(cent->trailPosList[0]));
        cent->trailPosList[0] = cent->current.origin;
        memcpy(cent->trailAngleList+1,cent->trailAngleList,sizeof(cent->trailAngleList)-sizeof(cent->trailAngleList[0]));
        cent->trailAngleList[0] = cent->current.angles;
    }

    // set standard ent members
    memset(&ent,0,sizeof(ent));
    ent.model	= cl.model_draw[modelindex];
    ent.frame	= cent->current.frame;
    ent.skinnum	= cent->current.skinnum;
    ent.render_scale.Set( 1, 1, 1 );
    ent.flags	= RF_TRANSLUCENT;

    for ( loop=0; loop<MAX_TRAILS; loop++ )
    {
        if ( (cent->trailPosList[loop].x || cent->trailPosList[loop].y || cent->trailPosList[loop].z) && 
             (cent->trailPosList[loop+1].x || cent->trailPosList[loop+1].y || cent->trailPosList[loop+1].z) )
        {
            ent.angles = cent->current.angles;
            ent.origin = cent->trailPosList[loop];
            ent.alpha=(MAX_TRAILS-loop)*0.1;
            V_AddEntity(&ent);
        }
    }
}

///////////////////////////////////////////////////////////////////////
// void CL_BeamTrail(CVector &vecPos,centity_t *cent)
//
//  Description:
//      When an entity's state renderfx flags include RF_BEAMTRAIL, a
//      series of beams are rendered in a path behind the entity.
//      Like CL_AddTrails(), the first frame should include RF_INITTRAILS
//      in the flags: ent->s.renderfx = RF_INITTRAILS | RF_BEAMTRAIL.
//      
//      If the entitie's modelindex matches the client enumeration for
//      a valid beam texture, then that index will be used for the
//      beam texture rendered.
//
//  Parameters:
//      CVector vecPos      current render entity's position
//      centity_t *cent     packet entity from previous render pass
//      
//  Return:
//      void
//
//  Usage:
//      see cl_ents.cpp for an example
//      CL_BeamTrail(ent.origin, cent)
// 
//  Todo:
//      The roll for the beam isn't set. It's always parallel to the
//      ground. Angle data is also saved with the trail info in centity_t
//      and should be used.
void CL_BeamTrail(CVector &vecPos,centity_t *cent)
{
    beam_t      *beam;
    beamSeg_t   *beamSeg;
    beamSeg_t   *beamSegPrev;
    int         iLoop, iModelIndex;
    CVector     vecBeamStart, vecBeamEnd, vecBeamLen;

    beamSegPrev = NULL;
    beamSeg = NULL;
    beam = NULL;

    if ( cent->current.renderfx & RF_INITTRAILS )
    {
        memset(cent->trailPosList,0,sizeof(cent->trailPosList));
        cent->current.renderfx &= ~RF_INITTRAILS;                   // needs to be cleared on server side, too
    }
    // if current position is different from last 
    if ( memcmp( cent->trailPosList[0], vecPos, sizeof(CVector) ) )
    {
        // scroll positions through list
        memcpy(cent->trailPosList+1,cent->trailPosList,sizeof(cent->trailPosList)-sizeof(cent->trailPosList[0]));
        cent->trailPosList[0] = vecPos;
        memcpy(cent->trailAngleList+1,cent->trailAngleList,sizeof(cent->trailAngleList)-sizeof(cent->trailAngleList[0]));
        cent->trailAngleList[0] = cent->current.angles;
    }

    if( (beam = beam_AllocBeam()) != NULL)
    {
        
        iModelIndex = cent->current.modelindex2;
        if(!iModelIndex || iModelIndex >= BEAM_TEX_NUMTEXTURES)
            iModelIndex = BEAM_TEX_DISCUSTRAIL;
        beam->alpha = 0.3f;
        beam->startRadius=5.0f;
        beam->endRadius=2.5f;
        beam->texIndex = BEAM_TEX_DISCUSTRAIL;
        beam->flags = BEAMFLAG_ALPHAFADE|BEAMFLAG_RGBCOLOR;
        beam->numSegs = 0;
        beam->beamLength = 0;
        beam->rgbColor.Set(1.0f, 1.0f, 1.0f);
		beam->beamLength = 0;
        
        beam->firstSeg = NULL;
        // beam segments
        for(iLoop = 0;iLoop<MAX_TRAILS-1; iLoop++)
        {
            if ( ((cent->trailPosList[iLoop].x > 0.1f) || (cent->trailPosList[iLoop].y > 0.1f) || (cent->trailPosList[iLoop].z > 0.1f)) && 
                 ((cent->trailPosList[iLoop+1].x > 0.1f) || (cent->trailPosList[iLoop+1].y > 0.1f) || (cent->trailPosList[iLoop+1].z > 0.1f)) )
            {
                if((beamSeg = beam_AllocSeg()) == NULL)
                    break;  // bail out
                beam->numSegs++;
                vecBeamStart = cent->trailPosList[iLoop];
                vecBeamEnd = cent->trailPosList[iLoop+1];
//				vecBeamStart = vecBeamEnd.Interpolate(vecBeamEnd,vecBeamStart,3);	// cek[11-30-99] make the trail a bit longer.
                vecBeamLen = vecBeamStart - vecBeamEnd;
                beam->beamLength += vecBeamLen.Length();
                vecBeamLen.Normalize();
                beamSeg->segNormal = vecBeamLen;
                beamSeg->alpha = 0.3;//(MAX_TRAILS - iLoop) * 0.1;
                beamSeg->segStart = vecBeamStart;
                beamSeg->segEnd = vecBeamEnd;
                if(!beam->firstSeg)
                {
                    beam->beamStart = cent->trailPosList[0];
                    beam->firstSeg = beamSeg;
                }
                else if(beamSegPrev)
                    beamSegPrev->nextSeg = beamSeg;
                beamSeg->nextSeg = NULL;
                beamSegPrev = beamSeg;
            }
        }
    }
}
//
///////////////////////////////////////////////////////////////////////