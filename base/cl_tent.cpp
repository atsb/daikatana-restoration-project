// cl_tent.c -- client side temporary entities
#include "client.h"

#include "dk_cl_curve.h"
#include "dk_beams.h"
//#include "ClientEntityManager.h"

#define TE_MAX       512

#define MAX_EXP_NODES 20

#define MAX_GLOBAL_POINTS           400         // max global lightning points
#define MAX_LIGHTNING_POINTS        40          // max points per lightning bolt
#define MAX_LIGHTNING_BOLTS         20          // max visible lightning bolts

#define EXP_TS_VELOCITY             400

/*
typedef struct lightningPoint_s {
   CVector position;
   CVector velocity;
} lightningPoint_t;
*/


typedef struct trackParamData_s
{
    entity_t  *ent;
    centity_t *owner,*track,*dstent,*extraent, *o_track;
    te_tracker_t *tracker;
    float     fRandom;          // calculate once each time CL_AddTrackEnts() is called
    CVector     vecMuzzle;     // set by calling re.GetModelHardpoint("hr_muzzle", 0, 0, modelent, v1) in CL_AddTrackEnts()
    char        *muzzle_str[3];    
    CVector     angles, o_angles;
    CVector     origin, o_origin;
    CVector     forward, o_forward, fwd, right, o_right, up, o_up;
    CVector     end;
    CVector     shake;
    short       thirdPersonView;
} trackParamData_t;

typedef struct lightningInfo_s
{
    CVector position[MAX_LIGHTNING_POINTS+1];       // list of positions
    CVector velocity[MAX_LIGHTNING_POINTS+1];       // list of velocities
    char numPoints;                              // -1 == free to use
    float moveofs;                               //
} lightningInfo_t;

// Logic[5/2/99]: More economical kick/effects (smoother on slow connections too!)
kickview_t      g_KickView;
short       thirdPersonView;
CVector     vecBobOffset;       // calculated offset from normal ent origin, run whenever
                                // a 1st person weapon entity is added to the view

// --------------------------------------------------------------------
#include "Projectile_Fx.h"

extern cvar_t *sv_violence;

te_tracker_t trackerList[TE_MAX];
CVector forward,right,up;
CVector zero_vector(0,0,0);

//lightningPoint_t lightningPoints[MAX_GLOBAL_POINTS];
lightningInfo_t lightningInfo[MAX_LIGHTNING_BOLTS];

te_tracker_t *CL_FindTracker(int track_id);
void CL_AddTrackEnts(void);
void CL_TrackEntityMsg(void);
//int doLightning(te_tracker_t *tracker,entity_t *ent);
void addTrailPoint(te_tracker_t *tracker, CVector &pos,short type);
void removeTrailPoint(te_tracker_t *tracker);
void    CL_FreeTrackEntity (int track_entity_index);

typedef enum
{
    ex_free, 
    ex_misc, 
    ex_flash, 
    ex_mflash, 
    ex_poly, 
    ex_sprite,
    ex_scale_sprite,    //	Nelno:	only one frame, scales from large to 0.1
    ex_scale_alpha,
    ex_scale_alpha_size,
    ex_scale_rotate,
    ex_scale_rotate_alpha,     // same as scale_rotate, but fades alpha, too
    ex_explosion,
    ex_explosion_alpha,
    ex_tracer,
    ex_explosion_time,         // looped anim, until time expires, alpha's out during last second
    ex_scale_image,
    ex_anim_rotate,             // looped anim, rotates until time expires, alpha's out during last second
    ex_traveling_sparks
} exptype_t;

typedef struct
{
    exptype_t   type;
    entity_t    ent;

    CVector   pos;
    CVector   end;
    CVector   forward;
    int     frames;
    float       light;
    CVector lightcolor;
    float       start;
    float    killtime;
    int     baseframe;
    float       scale_factor;
    float    start_scale;
    float       end_scale;
    float       alpha_scale;
    float    start_alpha;
    float       end_alpha;
    CVector ofs;
    CVector avelocity;
    short curNode;
    short numNodes;
    CVector node[MAX_EXP_NODES];
    int ent_number;
} explosion_t;

// SCG[10/8/99]: for surfacemarks
enum
{
	SM_BLOOD,
	SM_BULLET,
	SM_SCORCH,
	SM_FIST,
	SM_EARTH,
	SM_CLAW,
	SM_NOVA,
	SM_META,
	SM_MULTIBULLET,
	SM_DK_SLASH,
	SM_COUNT
};

#define	MAX_EXPLOSIONS	96
explosion_t cl_explosions[MAX_EXPLOSIONS];
/*
#define	MAX_BEAMS	32
typedef struct
{
    int		entity;
    struct model_s	*model;
    int		endtime;
    CVector	offset;
    CVector	start, end;
} beam_t;
beam_t		cl_beams[MAX_BEAMS];
*/
#define	MAX_LASERS	32
typedef struct
{
    entity_t    ent;
    int endtime;
    int track_dst;             // pulls dst position from cl_entities[track_index].current
} laser_t;
laser_t     cl_lasers[MAX_LASERS];


//extern  void    clhr_TriCenter ( CVector &v1,  CVector &v2,  CVector &v3, CVector &v_out);

//void CL_BlasterParticles ( CVector &org, CVector &dir);
void CL_ExplosionParticles ( CVector &org);
void CL_BigExplosionParticles ( CVector &org,float scale);
void CL_SmallExplosionParticles ( CVector &org);
//void CL_BFGExplosionParticles ( CVector &org);
void CL_AddLaserEnd( CVector &pos, CVector &ang, int modelIndex, float radius, float alpha, short thirdperson);

//------  quake 2 shit -------
void *cl_sfx_ric1;
void *cl_sfx_ric2;
void *cl_sfx_ric3;
void *cl_sfx_lashit;
void *cl_sfx_spark5;
void *cl_sfx_spark6;
void *cl_sfx_spark7;
void *cl_sfx_railg;
void *cl_sfx_rockexp;
void *cl_sfx_grenexp;
void *cl_sfx_watrexp;
//----------------------------

void *cl_sfx_footsteps[4];
void *cl_sfx_footsteps_wood[4];
void *cl_sfx_footsteps_metal[4];
void *cl_sfx_footsteps_stone[4];
void *cl_sfx_footsteps_glass[4];
void *cl_sfx_footsteps_ice[4];
void *cl_sfx_footsteps_snow[4];
void *cl_sfx_footsteps_puddle[4];
void *cl_sfx_footsteps_sand[4];
void *cl_sfx_footsteps_ladder_wood[3];
void *cl_sfx_footsteps_ladder_metal[5];
void *cl_sfx_swim[4];

void *cl_sfx_respawn;
void *cl_sfx_teleport;
void *cl_sfx_fall[3];
void *cl_sfx_splash[3];
void *cl_sfx_ricochet[8];

void *cl_mod_explode;
void *cl_mod_smoke;
void *cl_mod_flash;
void *cl_mod_parasite_segment;
void *cl_mod_grapple_cable;
void *cl_mod_parasite_tip;
void *cl_mod_explo4;
void *cl_mod_bfg_explo;
void *cl_mod_powerscreen;

//	Nelno:	Daikatana temp entity models
void *cl_sfx_SWwhoosh;
void *cl_sfx_SWwater[5];
void *cl_mod_explosion [3];
void *cl_sfx_explosion [1];
void *cl_sfx_waterexplosion;
void *cl_sfx_shockbounce[1];
void *cl_mod_novahit;
void *cl_sfx_novahit;
void *cl_mod_kcorehit;
void *cl_mod_disctrail;
void *cl_mod_firesmoke;
void *cl_mod_gunflare;
void *cl_mod_metahit;
void *cl_mod_metaexp;

void *cl_mod_muzzleflash;
void *cl_mod_muzzleflash_back;

void *cl_mod_polyexplode;             // center of explosion
void *cl_mod_polyexplode2;            // explosion ring

//void *cl_mod_rainSplash;                    // splash for rain drops

void *cl_mod_surfacemarks[SM_COUNT];

void *cl_sfx_dk;

/*
=================
CL_RegisterTEntSounds
=================
*/
void CL_RegisterTEntSounds (void)
{
	cl_sfx_SWwhoosh				= S_RegisterSound ("global/we_afterburner.wav");
	cl_sfx_SWwater[0]			= S_RegisterSound ("e1/we_sideunbubblesa.wav");
	cl_sfx_SWwater[1]			= S_RegisterSound ("e1/we_sideunbubblesb.wav");
	cl_sfx_SWwater[2]			= S_RegisterSound ("e1/we_sideunbubblesc.wav");
	cl_sfx_SWwater[3]			= S_RegisterSound ("e1/we_sideunbubblesd.wav");
	cl_sfx_SWwater[4]			= S_RegisterSound ("e1/we_sideunbubblese.wav");

    cl_sfx_footsteps[0]			= S_RegisterSound ("global/p_stp1.wav");
    cl_sfx_footsteps[1]			= S_RegisterSound ("global/p_stp2.wav");
    cl_sfx_footsteps[2]			= S_RegisterSound ("global/p_stp3.wav");
    cl_sfx_footsteps[3]			= S_RegisterSound ("global/p_stp4.wav");

    cl_sfx_footsteps_wood[0]	= S_RegisterSound ("global/p_stpwd1.wav");
    cl_sfx_footsteps_wood[1]	= S_RegisterSound ("global/p_stpwd2.wav");
    cl_sfx_footsteps_wood[2]	= S_RegisterSound ("global/p_stpwd3.wav");
    cl_sfx_footsteps_wood[3]	= S_RegisterSound ("global/p_stpwd4.wav");

    cl_sfx_footsteps_metal[0]	= S_RegisterSound ("global/p_stpmt1.wav");
    cl_sfx_footsteps_metal[1]	= S_RegisterSound ("global/p_stpmt2.wav");
    cl_sfx_footsteps_metal[2]	= S_RegisterSound ("global/p_stpmt3.wav");
    cl_sfx_footsteps_metal[3]	= S_RegisterSound ("global/p_stpmt4.wav");

    cl_sfx_footsteps_stone[0]	= S_RegisterSound ("global/p_stpst1.wav");
    cl_sfx_footsteps_stone[1]	= S_RegisterSound ("global/p_stpst2.wav");
    cl_sfx_footsteps_stone[2]	= S_RegisterSound ("global/p_stpst3.wav");
    cl_sfx_footsteps_stone[3]	= S_RegisterSound ("global/p_stpst4.wav");

    cl_sfx_footsteps_ice[0]	= S_RegisterSound ("global/p_stpic1.wav");
    cl_sfx_footsteps_ice[1]	= S_RegisterSound ("global/p_stpic2.wav");
    cl_sfx_footsteps_ice[2]	= S_RegisterSound ("global/p_stpic3.wav");
    cl_sfx_footsteps_ice[3]	= S_RegisterSound ("global/p_stpic4.wav");

    cl_sfx_footsteps_snow[0]	= S_RegisterSound ("global/p_stpsn1.wav");
    cl_sfx_footsteps_snow[1]	= S_RegisterSound ("global/p_stpsn2.wav");
    cl_sfx_footsteps_snow[2]	= S_RegisterSound ("global/p_stpsn3.wav");
    cl_sfx_footsteps_snow[3]	= S_RegisterSound ("global/p_stpsn4.wav");

    cl_sfx_footsteps_puddle[0] = S_RegisterSound ("global/p_stppu1.wav");
    cl_sfx_footsteps_puddle[1] = S_RegisterSound ("global/p_stppu2.wav");
    cl_sfx_footsteps_puddle[2] = S_RegisterSound ("global/p_stppu3.wav");
    cl_sfx_footsteps_puddle[3] = S_RegisterSound ("global/p_stppu4.wav");

    cl_sfx_footsteps_sand[0]	= S_RegisterSound ("global/p_stpsd1.wav");
    cl_sfx_footsteps_sand[1]	= S_RegisterSound ("global/p_stpsd2.wav");
    cl_sfx_footsteps_sand[2]	= S_RegisterSound ("global/p_stpsd3.wav");
    cl_sfx_footsteps_sand[3]	= S_RegisterSound ("global/p_stpsd4.wav");

    cl_sfx_footsteps_ladder_wood[0]	= S_RegisterSound ("global/e_woodcreaka.wav");
    cl_sfx_footsteps_ladder_wood[1]	= S_RegisterSound ("global/e_woodcreakb.wav");
    cl_sfx_footsteps_ladder_wood[2]	= S_RegisterSound ("global/e_woodcreakc.wav");

    cl_sfx_footsteps_ladder_metal[0]	= S_RegisterSound ("global/p_laddera.wav");
    cl_sfx_footsteps_ladder_metal[1]	= S_RegisterSound ("global/p_ladderb.wav");
    cl_sfx_footsteps_ladder_metal[2]	= S_RegisterSound ("global/p_ladderc.wav");
    cl_sfx_footsteps_ladder_metal[3]	= S_RegisterSound ("global/p_ladderd.wav");
    cl_sfx_footsteps_ladder_metal[4]	= S_RegisterSound ("global/p_laddere.wav");

    cl_sfx_explosion [0]		= S_RegisterSound ("global/e_explode1.wav");
    cl_sfx_shockbounce[0]		= S_RegisterSound("global/e_explodeb.wav");
	cl_sfx_waterexplosion		= S_RegisterSound("global/e_wexplodee.wav");
	cl_sfx_splash[0]			= S_RegisterSound ("global/p_stppu1.wav");	// fall short
	cl_sfx_splash[1]			= S_RegisterSound ("global/p_stppu2.wav");	// fall normal
	cl_sfx_splash[2]			= S_RegisterSound ("global/p_stppu3.wav");	// fall far

    cl_sfx_ricochet[0]			= S_RegisterSound("global/e_ricocheta.wav");  // bullethole temp event
    cl_sfx_ricochet[1]			= S_RegisterSound("global/e_ricochetb.wav");  // bullethole temp event
    cl_sfx_ricochet[2]			= S_RegisterSound("global/e_ricochetc.wav");  // bullethole temp event
    cl_sfx_ricochet[3]			= S_RegisterSound("global/e_ricochetd.wav");  // bullethole temp event
    cl_sfx_ricochet[4]			= S_RegisterSound("global/e_ricochete.wav");  // bullethole temp event
    cl_sfx_ricochet[5]			= S_RegisterSound("global/e_ricochetf.wav");  // bullethole temp event
    cl_sfx_ricochet[6]			= S_RegisterSound("global/e_ricochetg.wav");  // bullethole temp event
    cl_sfx_ricochet[7]			= S_RegisterSound("global/e_ricocheth.wav");  // bullethole temp event

	//NSS[11/13/99]:
	cl_sfx_swim[0]				= S_RegisterSound("hiro/swim1.wav");  // Swim
	cl_sfx_swim[1]				= S_RegisterSound("hiro/swim2.wav");  // 
	cl_sfx_swim[2]				= S_RegisterSound("hiro/swim3.wav");  // 
	cl_sfx_swim[3]				= S_RegisterSound("hiro/swim4.wav");  // 

	cl_sfx_dk					= S_RegisterSound("global/we_dk_05.wav");
}   

void Init_weapon_flashes();
/*
=================
CL_RegisterTEntModels
=================
*/
void CL_RegisterTEntModels (void)
{
    //	Nelno:	don't need Quake 2 models any longer
    cl_mod_explode = NULL;
    cl_mod_smoke = NULL;
    cl_mod_flash = NULL;

    //	cache explosion sprite
	cl_mod_explosion[0]				= re.RegisterModel( "models/global/we_expl.sp2", RESOURCE_GLOBAL );
	cl_mod_explosion[1]				= re.RegisterModel( "models/global/we_expla.sp2", RESOURCE_GLOBAL );
	cl_mod_explosion[2]				= re.RegisterModel( "models/e1/we_shockring.sp2", RESOURCE_GLOBAL );		// blue shockwave
	cl_mod_novahit					= re.RegisterModel( "models/e4/we_novahit.sp2", RESOURCE_GLOBAL );
	cl_mod_metahit					= re.RegisterModel( "models/e4/we_mmaserhit.sp2", RESOURCE_GLOBAL );
	cl_mod_metaexp					= re.RegisterModel( "models/e4/we_mmaserexp.dkm", RESOURCE_GLOBAL );
	cl_mod_disctrail				= re.RegisterModel( "models/e2/we_disctr.sp2", RESOURCE_GLOBAL );
	cl_mod_kcorehit					= re.RegisterModel( "models/e4/we_kcorehitb.sp2", RESOURCE_GLOBAL );
	cl_mod_firesmoke				= re.RegisterModel( "models/global/e2_firea.sp2", RESOURCE_GLOBAL );
	cl_mod_gunflare					= re.RegisterModel( "models/global/gunflare.sp2", RESOURCE_GLOBAL );
	cl_mod_muzzleflash				= re.RegisterModel( "models/global/we_mflash.dkm", RESOURCE_GLOBAL );
	cl_mod_polyexplode				= re.RegisterModel( "models/global/we_expball.dkm", RESOURCE_GLOBAL );
	cl_mod_polyexplode2				= re.RegisterModel( "models/global/we_expdisc.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_BLOOD]	= re.RegisterModel( "models/global/we_blood.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_BULLET]	= re.RegisterModel( "models/global/we_bhole.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_SCORCH]	= re.RegisterModel( "models/global/we_scorch.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_FIST]	= re.RegisterModel( "models/global/we_dispunch.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_EARTH]	= re.RegisterModel( "models/e2/we_hammercrack.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_CLAW]	= re.RegisterModel( "models/global/we_clwmark2.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_NOVA]	= cl_mod_novahit;
	cl_mod_surfacemarks[SM_META]	= cl_mod_metahit;
	cl_mod_surfacemarks[SM_MULTIBULLET] = re.RegisterModel( "models/global/we_bhole2.sp2", RESOURCE_GLOBAL );
	cl_mod_surfacemarks[SM_DK_SLASH]= re.RegisterModel( "models/global/we_dkslash.sp2", RESOURCE_GLOBAL );
//	cl_mod_surfacemarks[SM_COUNT]	= NULL;

	Init_weapon_flashes();
}   

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
//	memset (cl_beams, 0, sizeof(cl_beams));
    memset (cl_explosions, 0, sizeof(cl_explosions));
    memset (cl_lasers, 0, sizeof(cl_lasers));
    memset (trackerList,0,sizeof(trackerList));

//	memset(lightningPoints,0,sizeof(lightningPoints));
    memset (lightningInfo,-1,sizeof(lightningInfo));          // make sure numpoints inits to -1
}
/*
=================
CL_AllocExplosion
=================
*/
explosion_t *CL_AllocExplosion (void)
{
    int     i;
    int     time;
    int     index;

    for ( i=0 ; i<MAX_EXPLOSIONS ; i++ )
    {
        if ( cl_explosions[i].type == ex_free )
        {
            memset (&cl_explosions[i], 0, sizeof (cl_explosions[i]));
            return &cl_explosions[i];
        }
    }
// find the oldest explosion
    time = cl.time;
    index = 0;
    for ( i=0 ; i<MAX_EXPLOSIONS ; i++ )
        if ( cl_explosions[i].start < time )
        {
            time = cl_explosions[i].start;
            index = i;
        }
    memset (&cl_explosions[index], 0, sizeof (cl_explosions[index]));
    return &cl_explosions[index];
}

///////////////////////////////////////////////////////////////////////////////
//	CL_NovaHit
//
///////////////////////////////////////////////////////////////////////////////

void    CL_NovaHit ( CVector &org, CVector &angles)
{
    explosion_t *ex;

    ex = CL_AllocExplosion ();

    ex->ent.origin = org;

    ex->ent.angles = angles;

    ex->type = ex_scale_sprite;
    ex->ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = 00;
    ex->lightcolor.x = 0.8f;
    ex->lightcolor.y = 0.4f;
    ex->lightcolor.z = 0.2f;

    ex->ent.render_scale.x = (rand () / (RAND_MAX << 1)) + .8;
    ex->ent.render_scale.y = ex->ent.render_scale.x;
    ex->ent.render_scale.z = ex->ent.render_scale.y;

    ex->ent.alpha = 0.9;
    ex->scale_factor = 0.995;
    ex->end_scale = 0.2;
    ex->alpha_scale = 0.95;
    ex->end_alpha = 0.025;

    ex->ent.model = cl_mod_novahit;

    ex->frames = 1;

    //	Nelno:	start cool hissing sound here
//		S_StartSoundQuake2 (pos, 0, 0, cl_sfx_explosion [0], 1, ATTN_NORM, 0);
}


///////////////////////////////////////////////////////////////////////////////
//	CL_KcoreHit
///////////////////////////////////////////////////////////////////////////////
// Logic[3/29/99]: void    CL_KcoreHit ( CVector &org, CVector &angles)
void    CL_KcoreHit (void)
{
    CVector org;
    CVector angles;
    explosion_t *ex;

#ifdef _DEBUG
	SHOWNET("CL_KcoreHit");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, org);
    //	read facing angle
    angles.x = MSG_ReadAngle16 (&net_message);
    angles.y = MSG_ReadAngle16 (&net_message);
    angles.z = MSG_ReadAngle16 (&net_message);

    ex = CL_AllocExplosion ();

    ex->ent.origin = org;

    ex->ent.angles = angles;

    ex->type = ex_scale_sprite;
    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = 0;
    ex->lightcolor.x = 1.0;
    ex->lightcolor.y = 0.25;
    ex->lightcolor.z = 0.25;

    ex->ent.render_scale.x = (rand () / (RAND_MAX << 1)) + 1.0;
    ex->ent.render_scale.y = ex->ent.render_scale.x;
    ex->ent.render_scale.z = ex->ent.render_scale.x;

    ex->ent.alpha = 1.0;
    ex->scale_factor = 0.98;
    ex->end_scale = 0.05;
    ex->alpha_scale = 0.90;
    ex->end_alpha = 0.05;

    ex->ent.model = cl_mod_kcorehit;

    ex->frames = 1;

    //	Nelno:	start cool hissing sound here
//		S_StartSoundQuake2 (pos, 0, 0, cl_sfx_explosion [0], 1, ATTN_NORM, 0);
}

//---------------------------------------------------------------------------
// CL_Explosion()
//---------------------------------------------------------------------------
// amw - fucking C file so I can't use default parameters

void CL_Explosion( CVector &org, CVector &angles, float scale, int numframes, 
                  int modelindex, short flags, short light, CVector &color, struct model_s *model, float alpha)
{
    explosion_t *ex;

    ex = CL_AllocExplosion ();

    ex->ent.origin = org;

    ex->ent.angles = angles;

    if ( flags & CLE_FADEALPHA )
    {
        ex->type=ex_explosion_alpha;
    }
    else
    {
        if ( alpha > 1.0 )
            alpha = 1.0;
        ex->ent.alpha = alpha;
        ex->type=ex_explosion;
    }

    ex->ent.flags = SPR_ALPHACHANNEL|RF_TRANSLUCENT;
    if ( angles.x >= 0 || angles.y >= 0 || angles.z >= 0 )
	{
        ex->ent.flags |= SPR_ORIENTED;
	}
    ex->light = light;
    ex->lightcolor = color;

    ex->ent.render_scale.x = scale;
    ex->ent.render_scale.y = scale;
    ex->ent.render_scale.z = scale;

    // did we pass in a model pointer or a model index ?
    if ( !model )
        ex->ent.model=cl.model_draw[modelindex];
    else
        ex->ent.model = model;

    ex->frames=numframes;
    ex->baseframe=0;
    ex->start=cl.frame.servertime-CL_FRAME_MILLISECONDS;
}

//---------------------------------------------------------------------------
// CL_ScaleRotate()
//---------------------------------------------------------------------------
// Logic[3/29/99]: void CL_ScaleRotate( CVector &pos,float start_scale,float end_scale,float scale_time,int modelindex)
void CL_ScaleRotate(void)
{
    explosion_t *ex;
    CVector pos;
    float start_scale, end_scale, scale_time;
    int modelindex;
    
#ifdef _DEBUG
	SHOWNET("CL_ScaleRotate");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    start_scale=MSG_ReadFloat(&net_message);           // START scale size
    end_scale=MSG_ReadFloat(&net_message);     // END scale size
    scale_time=MSG_ReadFloat(&net_message);       // period of TIME to scale
    modelindex=MSG_ReadLong(&net_message);                // MODELINDEX


    ex = CL_AllocExplosion ();

    ex->ent.origin = pos;

    ex->ent.angles.x = crand()*90;
    ex->ent.angles.y = crand()*90;
    ex->ent.angles.z = 0;

    ex->type = ex_scale_rotate;
    ex->ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = 0;
    ex->avelocity.x = 7+crand()*27;
    ex->avelocity.y = 7+crand()*27;
	ex->avelocity.z = 7+crand()*27;
    // ex->avelocity.z = 0;

    ex->ent.alpha = .2;
    ex->ent.render_scale.x = start_scale;
    ex->ent.render_scale.y = start_scale;
    ex->ent.render_scale.z = start_scale;

    ex->scale_factor = scale_time*1000;          // length of time to scale
    ex->start_scale = start_scale;               //
    ex->end_scale = end_scale;                   // 

    ex->ent.model=cl.model_draw[modelindex];

    ex->frames = 1;

}

void CL_SpriteExplosion(CVector &vecPos, float fScale)
{
    explosion_t *ex;

	if(ex = CL_AllocExplosion ())
    {
	    ex->ent.origin = vecPos;
	    ex->type = ex_poly;
	    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
	    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
	    ex->light = 200;
	    ex->lightcolor.Set(1.0, 0.5, 0.0);
	    ex->ent.render_scale [0] = ex->ent.render_scale [1] = ex->ent.render_scale [2] = fScale;
	    if (rand () & 1)
	    {
		    ex->ent.model = cl_mod_explosion [1];
		    ex->frames = 8;
	    }
	    else
	    {
		    ex->ent.model = cl_mod_explosion [0];
		    ex->frames = 7;
	    }
    }
}

//---------------------------------------------------------------------------
// CL_ScaleRotateAlpha()
//---------------------------------------------------------------------------
void CL_ScaleRotateAlpha(void)
{
    explosion_t *ex;
    CVector pos,angle,rot;
    float start_scale, end_scale, start_alpha, dietime;
    int modelindex;
    
#ifdef _DEBUG
	SHOWNET("CL_ScaleRotate");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
	angle.x = MSG_ReadFloat(&net_message);
	angle.y = MSG_ReadFloat(&net_message);
	angle.y = MSG_ReadFloat(&net_message);
	rot.x = MSG_ReadFloat(&net_message);
	rot.y = MSG_ReadFloat(&net_message);
	rot.y = MSG_ReadFloat(&net_message);
    start_scale=MSG_ReadFloat(&net_message);   
    end_scale=MSG_ReadFloat(&net_message);     
    modelindex=MSG_ReadLong(&net_message);     
    start_alpha=MSG_ReadFloat(&net_message);   
    dietime=MSG_ReadFloat(&net_message);     

    ex = CL_AllocExplosion ();

    ex->ent.origin = pos;

    ex->ent.angles = angle;

    ex->type = ex_scale_rotate_alpha;
    ex->ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = 0;
    ex->avelocity=rot;

    ex->ent.alpha = start_alpha;
	ex->start_alpha = start_alpha;
    ex->ent.render_scale.x = start_scale;
    ex->ent.render_scale.y = start_scale;
    ex->ent.render_scale.z = start_scale;

    ex->scale_factor = dietime*1000;          // length of time to scale
    ex->start_scale = start_scale;               //
    ex->end_scale = end_scale;                   // 

    ex->ent.model=cl.model_draw[modelindex];

    ex->frames = 1;

}
//---------------------------------------------------------------------------
// CL_PolyExplosion()
//---------------------------------------------------------------------------
void CL_PolyExplosion( CVector &pos, CVector &angles, void *model, float start_scale, float end_scale, float scale_time, float lsize, CVector &lcolor, int iEntFlags)
{
    explosion_t *ex;

    ex = CL_AllocExplosion ();

    ex->ent.origin = pos;
    ex->ent.angles = zero_vector;

    ex->type=ex_scale_rotate_alpha;
    // ex->type = ex_scale_alpha;
    
    ex->ent.flags=RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL;
    ex->ent.flags = iEntFlags;

    ex->start=cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light=lsize;
    ex->lightcolor = lcolor;

    ex->avelocity.Zero();
    
    ex->ent.angles = angles;
    ex->ent.render_scale.x = start_scale;
    ex->ent.render_scale.y = start_scale;
    ex->ent.render_scale.z = start_scale;

    ex->scale_factor=scale_time*1000;
    ex->start_scale=start_scale * 0.01;
    ex->end_scale=end_scale;

    ex->start_alpha=1.10;
    ex->end_alpha=0.01;
//    ex->ent.alpha = 0.25;
    ex->ent.alpha = 1.0;
    ex->ent.model=model;

    ex->frames=5;
}

//---------------------------------------------------------------------------
// CL_ScaleAlpha()
//---------------------------------------------------------------------------
// Logic[4/8/99]: void CL_ScaleAlpha(void), reusing this for more advanced effects now
void CL_ScaleAlpha( CVector &org, CVector &angles,float scale,int numframes,int modelindex, float alpha_scale,float alpha_end,short light, CVector &color)
{
    explosion_t *ex;



    ex = CL_AllocExplosion ();

    ex->ent.origin = org;
    ex->ent.angles = angles;

    ex->type = ex_scale_alpha;
    ex->ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = light;
    ex->lightcolor = color;

    ex->ent.render_scale.x = scale;
    ex->ent.render_scale.y = scale;
    ex->ent.render_scale.z = scale;

    ex->ent.alpha = 1.0;
    ex->alpha_scale = alpha_scale;
    ex->end_alpha = alpha_end;

    ex->ent.model=cl.model_draw[modelindex];

    ex->frames = numframes;
    ex->baseframe=0;
}

//---------------------------------------------------------------------------
// CL_ScaleImage()
//---------------------------------------------------------------------------
// Logic[4/8/99]: void CL_ScaleImage(void)
void CL_ScaleImage( CVector &org, CVector &angles,float scale_start,float scale_end,int modelindex,float alpha,float killtime)
{
    explosion_t *ex;

    ex = CL_AllocExplosion ();

    ex->ent.origin = org;
    ex->ent.angles = angles;

    ex->type = ex_scale_image;
    ex->ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | SPR_ORIENTED;

    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;

    ex->light = 0;                      // no light

    ex->alpha_scale=scale_start;           // starting size of explosion
    ex->end_scale=scale_end-scale_start;   // total scale
    ex->killtime=ex->start+killtime*1000;

    ex->ent.alpha = alpha;

    ex->ent.model=cl.model_draw[modelindex];
    ex->baseframe=0;
}

//---------------------------------------------------------------------------
// CL_TimedFire()
//---------------------------------------------------------------------------
// Logic[3/29/99]: void CL_TimedFire( CVector &org,float burntime,short firstFrame)
void CL_TimedFire(void)
{
    explosion_t *ex;
    CVector org;
    float burntime;
    short firstFrame;

#ifdef _DEBUG
	SHOWNET("CL_TimedFire");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, org);           // POSITION
    burntime=MSG_ReadFloat(&net_message);        // BURN TIME
    firstFrame=MSG_ReadByte(&net_message);         // FIRST FRAME

    ex = CL_AllocExplosion ();

    ex->ent.origin = org;
    ex->ent.flags = RF_TRANSLUCENT;
    ex->ent.alpha=.6;

    ex->type=ex_explosion_time;

    ex->ent.render_scale.x = 1;
    ex->ent.render_scale.y = 1;
    ex->ent.render_scale.z = 1;
    ex->light = 240;
    ex->lightcolor.Set(1.0, 0.6 + (frand() * 0.2), 0.1 + (frand() * 0.2));
    ex->ent.model = cl_mod_firesmoke;
    ex->frames=8;
    ex->baseframe=0;
    ex->start=cl.frame.servertime-CL_FRAME_MILLISECONDS;
    ex->start += firstFrame*CL_FRAME_MILLISECONDS;           // sync start with time where 'firstframe' would be
    ex->killtime = ex->start + burntime*1000;
    ex->end_alpha=ex->ent.alpha;
}

//---------------------------------------------------------------------------
// CL_Smoke2()
//---------------------------------------------------------------------------
void CL_Smoke2( CVector &origin, float scale, float count, int spread, float maxvel)
{
    cparticle_t *p;
    float r;

    while ( count-- )
    {

        if ( !(p=CL_SpawnComParticle()) )
            return;

        p->time = cl.time;
        p->pscale=scale;

        p->type = PARTICLE_SMOKE;
        p->alpha = .5;
        p->alphavel = -(1.4 + .2*frand()); //p->alphavel = -1.0 / (1+frand()*0.2);
//        p->color = 4 + (rand()&7);
		float f = 0.1*crand();
		p->color_rgb.Set( 0.2 + f, 0.2 + f, 0.2  + f);

        r = spread ? (rand() % (spread << 1)) - spread : 0;
        p->org.x = origin.x + r;
        r = spread ? (rand() % (spread << 1)) - spread : 0;
        p->org.y = origin.y + r;
        r = spread ? (rand() % (spread << 1)) - spread : 0;
        p->org.z = origin.z + r;

        p->vel.x = 0;
        p->vel.y = crand()*maxvel;
        p->vel.z = crand()*maxvel;

        p->accel.z = maxvel;
    }
}

void CL_GasHandsSmoke(CVector &vecOrigin, CVector &vecVelocity)
{
    cparticle_t *p;
    int         iCount;

    iCount = rand() & 7;

    while ( iCount-- )
    {
        if ( !free_comparticles )
            return;

        if ( !(p=CL_SpawnComParticle()) )
            return;

        p->time = cl.time;
        p->pscale=0.5;

        p->type = PARTICLE_SMOKE;
        p->alpha = 1.0;
        p->alphavel = -(1.4 + .2*frand()); //p->alphavel = -1.0 / (1+frand()*0.2);
//        p->color = 4 + (rand()&7);
		float f = 0.1*crand();
		p->color_rgb.Set( 0.2 + f, 0.2 + f, 0.2  + f);

        p->org.x = vecOrigin.x + (crand()*0.5);
        p->org.y = vecOrigin.y + (crand()*0.5);
        p->org.z = vecOrigin.z + (crand()*0.5);

        p->vel.x = vecVelocity.x + (crand()*5);
        p->vel.y = vecVelocity.y + (crand()*5);
        p->vel.z = vecVelocity.z + (frand()*5);

        p->accel.z = 5;
    }
}

//---------------------------------------------------------------------------
// CL_Smoke()
//---------------------------------------------------------------------------
void CL_Smoke( CVector &origin, float scale, float count, int spread, float maxvel)
{
    cparticle_t *p;
    float       r;
    while ( count-- )
    {
        if ( !free_comparticles )
            return;

        if ( !(p=CL_SpawnComParticle()) )
            return;

        p->time = cl.time;
        p->pscale=scale;

        p->type = PARTICLE_SMOKE;
        p->alpha = 1.0;
        p->alphavel = -(1.4 + .2*frand()); //p->alphavel = -1.0 / (1+frand()*0.2);
//        p->color = 4 + (rand()&7);
		float f = 0.1*crand();
		p->color_rgb.Set( 0.2 + f, 0.2 + f, 0.2  + f);

        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.x = origin.x + r;
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.y = origin.y + r;
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.z = origin.z + r;

        p->vel.x = 0;
        p->vel.y = frand()*maxvel;
        p->vel.z = frand()*maxvel;

        p->accel.z = maxvel;
    }
}

//---------------------------------------------------------------------------
// CL_SunflareSmoke()
//---------------------------------------------------------------------------
void CL_SunflareSmoke(CVector &origin, float scale, float count, int spread, float maxvel)
{
    cparticle_t *p;
    float r;
	
    while ( count-- )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

//        p->flags=PF_DEPTHHACK;
        p->time = cl.time;
        p->pscale=scale;

        p->type = PARTICLE_SMOKE;
        p->alpha = .5;
        p->alphavel = -(.5 + frand()*.09);      
        r = -(1.4 + .2*frand()); //r = -1.0 / (1+frand()*0.2);
        p->color_rgb.Set(0.1, 0.1, 0.1);
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.x = origin.x + r;
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.y = origin.y + r;
        r = 0;
        p->org.z = origin.z + r;

        p->vel.x = 0;
        p->vel.y = frand()*maxvel - maxvel/2;
        p->vel.z = frand()*maxvel - maxvel/2;

        p->accel.z = maxvel;
    }
}

//---------------------------------------------------------------------------
// CL_SunflareSparks()
//---------------------------------------------------------------------------
void CL_SunflareSparks( CVector &origin, float scale, float count, int spread, float maxvel)
{
    cparticle_t *p;
    float r;

    while ( count-- )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

//        p->flags=PF_DEPTHHACK;
        p->time = cl.time;
        p->pscale=scale;

        p->type = PARTICLE_SPARKS;
        p->alpha = .5;
        p->alphavel = -(.3 + frand()*.1);      
        r = -(1.4 + .2*frand());//r = -1.0 / (1+frand()*0.2);
        
        p->color_rgb.Set(1.0, 0.5, 0.5);
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.x = origin.x + r;
        r = spread ? frand()*spread - ((float)spread/2) : 0;
        p->org.y = origin.y + r;
        r = 0;
        p->org.z = origin.z + r;

        p->vel.x = (frand()*maxvel - maxvel/2) * .25;
        p->vel.y = (frand()*maxvel - maxvel/2) * .25;
        p->vel.z = maxvel*.15 + frand()*maxvel*.25;

        p->accel.z = -maxvel/8;
    }
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		CL_GenerateComplexParticle
// Description:Generates a complex particle stream based off of YOUR input
// Output:<nothing>
// Input:
// Vector Origin---> Origin of particle spawn
// Vector Dir	---> Multiplier for Velocity Vector
// Vector Color ---> Color values (R)x,(G)y,(B)z
// Vector Alpha ---> Alpha level values:
//				x--> Original Alpha level
//				y--> Depletion Rate 
//				z--> Random Limit (0 will set no random depletion rate)
// Gavity ---------> none, up, down (0,1,2)
// ----------------------------------------------------------------------------
void CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,int Gravity)
{
    cparticle_t *p;
    float r;
	//Real fucking simple spread and setting values... Kaduh!
    while ( count-- )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

	    p->time = cl.time;
        p->pscale=scale;

        p->type = Type;
        p->alpha = Alpha.x;
        p->alphavel = -(Alpha.y + frand()*Alpha.z);      
      
		r = -(1.4 + .2*frand());
        
        // Set the color   R        G        B    <nss>
		p->color_rgb.Set(Color.x, Color.y, Color.z);

		//Set the spread
        r = spread ? crand()*spread - ((float)spread/2) : 0;
        p->org.x = origin.x + r;
        r = spread ? crand()*spread - ((float)spread/2) : 0;
        p->org.y = origin.y + r;
        p->org.z = origin.z;
		
        //Set the velocity based off of Direction
		p->vel.x = ((frand()*maxvel - maxvel/2) * .25) * Dir.x;
        p->vel.y = ((frand()*maxvel - maxvel/2) * .25) * Dir.y;
        p->vel.z = (maxvel*.15 + frand()*maxvel*.25) * Dir.z;

        //Set the "deaccelleration" rate
		if(Gravity == 2)
		{
			p->accel.z = -maxvel/8;
		}
		else if(Gravity == 1)
		{
			p->accel.z = maxvel/8;
		}
    }
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		CL_GenerateComplexParticle2
// Description:THIS IS ALMOST like the other but uses a different method for
// pushing out the particles...I know it is "duplicate" code, but there are a
// lot of different functions calling the first one and things look good as is.
// Output:<nothing>
// Input:
// Vector Origin---> Origin of particle spawn
// Vector Dir	---> Multiplier for Velocity Vector
// Vector Color ---> Color values (R)x,(G)y,(B)z
// Vector Alpha ---> Alpha level values:
//				x--> Original Alpha level
//				y--> Depletion Rate 
//				z--> Random Limit (0 will set no random depletion rate)
// Gavity ---------> Vector with amount of Gravity to apply
// spread ---------> 360 degrees of spread
// ----------------------------------------------------------------------------
void CL_GenerateComplexParticle2( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity)
{
    cparticle_t *p;
	CVector target_angles,forward;
    float r,sp;

	sp = spread * 0.18;
	if(sp < 1 || sp > 10)
		sp=1;

	//Real fucking simple spread and setting values... Kaduh!
    while ( count-- )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

		p->time = cl.time;
        p->pscale=scale*3;

		p->type = Type;
        p->alpha = Alpha.x;
        p->alphavel = -(Alpha.y + frand()*Alpha.z);      
      
		r = -(1.4 + .2*frand());
        
        // Set the color   R        G        B    <nss>
		p->color_rgb.Set(Color.x, Color.y, Color.z);

		VectorToAngles(Dir,target_angles);

		target_angles.yaw += crand()*spread;
		target_angles.pitch += crand()*spread;

		target_angles.AngleToForwardVector(forward);
		
		//NSS[11/16/99]:We need to add Conic spread but leave this in for things without a radius.
		//Set the spread
        r = spread ? frand()*sp - ((float)sp/2) : 0;
        p->org.x = origin.x + r;
        r = spread ? frand()*sp - ((float)sp/2) : 0;
        p->org.y = origin.y + r;
        p->org.z = origin.z;
		//NSS[11/10/99]:
		p->lastorg = p->org;
        //Set the velocity based off of Direction

		p->vel = forward * ((maxvel*.55)+ (frand()*(maxvel*.45)));
 
        //Set the "deaccelleration" rate
		p->accel = Gravity;
    }
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//NSS[11/18/99]:
// Name:		CL_GenerateComplexParticleRadius
// Description:THIS IS EXACTLY like 2 but uses a radius for the base spread
// Output:<nothing>
// Input:
// Vector Origin---> Origin of particle spawn
// Vector Dir	---> Multiplier for Velocity Vector
// Vector Color ---> Color values (R)x,(G)y,(B)z
// Vector Alpha ---> Alpha level values:
//				x--> Original Alpha level
//				y--> Depletion Rate 
//				z--> Random Limit (0 will set no random depletion rate)
// Gavity ---------> Vector with amount of Gravity to apply
// spread ---------> 360 degrees of spread
// ----------------------------------------------------------------------------
void CL_GenerateComplexParticleRadius( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity,float radius)
{
    cparticle_t *p;
	CVector target_angles,forward,Radial_Carnage,Next_Dir,my_forward,my_right,my_up,right,up;
    float r,sp;

	sp = 360/count;
	Radial_Carnage.Set(0.0f,0.0f,0.0f);
	
	//NSS[11/18/99]:Arrange our angles based off of our current normalized vector
	Next_Dir.x = Dir.x;
	Next_Dir.y = Dir.z;
	Next_Dir.z = Dir.y;

	//Uh.. oh wow....like this is so fucking wild!
    while ( count > 0)
    {
		if ( !(p=CL_SpawnComParticle()) )
            return;

		p->time = cl.time;
        p->pscale=scale*3;

		p->type = Type;
        p->alpha = Alpha.x;
        p->alphavel = -(Alpha.y + frand()*Alpha.z);      
      
		r = -(1.4 + .2*frand());
        
        // Set the color   R        G        B    <nss>
		p->color_rgb.Set(Color.x, Color.y, Color.z);

		VectorToAngles(Dir,target_angles);

		target_angles.yaw += (crand()*.50f)*spread;
		target_angles.pitch += (crand()*.50f)*spread;

		target_angles.AngleToForwardVector(forward);
		//Set the velocity based off of Direction
		p->vel = forward * ((maxvel*.55)+ (frand()*(maxvel*.45))) ;
		
		//Calculate the radial spread
		Radial_Carnage = Next_Dir * (count*sp);//this is right if you think about it
		
		Radial_Carnage.AngleToVectors(forward, right, up);
		
	
		//right = -right;

		//NSS[11/18/99]:Now let's work some magic.
		// change matrix orientation
		my_forward.x = forward.y; 
		my_forward.y = right.y;  
		my_forward.z = up.y;
		my_right.x = forward.x; 
		my_right.y = right.x; 
		my_right.z = up.x;
		my_up.x = forward.z; 
		my_up.y = right.z; 
		my_up.z = up.z;



		
		//NSS[11/18/99]:Bring all of the values back to the new orientation
		forward.x = DotProduct( Next_Dir, my_forward);
		forward.y = DotProduct( Next_Dir, my_right );
		forward.z = DotProduct( Next_Dir, my_up );
		//NSS[11/18/99]:Normalize this new vector
		forward.Normalize();
			
		//set the spread
		p->org = origin + ( forward * radius );
		//NSS[11/10/99]:
		p->lastorg = p->org;
 
        //Set the "deaccelleration" rate
		p->accel = Gravity;

		count--;
    }
}

//---------------------------------------------------------------------------
// CL_GenerateSmokeComplex()
//---------------------------------------------------------------------------
void CL_GenerateSmokeComplex(CVector &origin, CVector &Dir, CVector &Color,float scale, float count, int spread, float maxvel)
{
    cparticle_t *p;
    float r;
	
    while ( count-- )
    {
        if ( !(p=CL_SpawnComParticle()) )
            return;

		p->time = cl.time;
		p->pscale=scale;

		p->type = PARTICLE_SMOKE;
		
		p->alpha = .5;
		p->alphavel = -(.5 + frand()*.09);      
		
		r = -(1.4 + .2*frand()); 
		
		
		p->color_rgb.Set(Color.x, Color.y, Color.z);

		r = spread ? frand()*spread - ((float)spread/2) : 0;
		p->org.x = origin.x + r;
		r = spread ? frand()*spread - ((float)spread/2) : 0;
		p->org.y = origin.y + r;
		r = 0;
		p->org.z = origin.z + r;

		p->vel.x = 0;
		p->vel.y = frand()*maxvel - maxvel/2;
		p->vel.z = frand()*maxvel - maxvel/2;

		p->accel.z = maxvel;
    }
}














//---------------------------------------------------------------------------
// CL_BeamSparks()
//---------------------------------------------------------------------------
// Logic[3/29/99]: void CL_BeamSparks( CVector &origin,float count,float scale)
void CL_BeamSparks(void)
{
    cparticle_t *p;
    CVector origin;
    float scale;
    int count;

#ifdef _DEBUG
	SHOWNET("CL_BeamSparks");
#endif /* DEBUG */

    MSG_ReadPos (&net_message, origin);
    count=MSG_ReadFloat(&net_message);
    scale=MSG_ReadFloat(&net_message);
    
    while ( count-- )
    {
        if ( !free_comparticles )
            return;

        p = free_comparticles;
        free_comparticles = p->next;
        p->next = active_comparticles;
        active_comparticles = p;
        p->accel.Zero();

        p->time = cl.time;
        p->pscale=scale;

        p->color_rgb.x=.8+crand()*.2;
        p->color_rgb.y=.8+crand()*.2;
        p->color_rgb.z=0;

        p->type = PARTICLE_BEAM_SPARKS;

        p->alpha = 1.0;
        p->alphavel = -(1.4 + .2*frand());         //-1.0 / (1+frand()*0.2);
//        p->color = 4 + (rand()&7);

        p->org = origin;     // + frand()*20-10;
        p->lastorg = p->org;
        p->vel.x = -20 + frand()*30;
        p->vel.y = -20 + frand()*30;
        p->vel.z = -20 + frand()*30;

        p->accel.x = -20 + frand()*30;
        p->accel.y = -20 + frand()*30;
        p->accel.z = -20 + frand()*30;

        p->accel.z = -40 + crand()*-30;
    }

}

/*
=================
CL_SmokeAndFlash
=================
*/
void CL_SmokeAndFlash( CVector &origin)
{
    CL_Smoke(origin, 2.0, 1, 10, 20);
}
/*
=================
CL_ParseParticles
=================
*/
// SCG[8/10/99]: This function is not used.
/*
void CL_ParseParticles (void)
{
    int     color, count;
    CVector pos, dir;
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    color = MSG_ReadByte (&net_message);
    count = MSG_ReadByte (&net_message);
    CL_ParticleEffect (pos, dir, color, count);
}
*/

/*
=================
CL_ParseLaser
=================
*/
// Logic[3/29/99]: void CL_ParseLaser( CVector &start, CVector &end,int track_dst)
void CL_ParseLaser(void)
{
    laser_t *l;
    int     i, track_dst;
    CVector start;
    CVector end;
   
#ifdef _DEBUG
	SHOWNET("CL_ParseLaser");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, start);
    MSG_ReadPos(&net_message, end);
    track_dst=MSG_ReadLong(&net_message);           // index of ent to track... if 0, always uses pos2 as dst origin

    for ( i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++ )
    {
        if ( l->endtime < cl.time )
        {
            l->ent.flags = RF_TRANSLUCENT | RF_BEAM;
            l->ent.origin = start;
            l->ent.oldorigin = end;
            l->ent.alpha = 0.30;
            l->ent.model = NULL;
            l->ent.frame = 4;
            l->endtime = cl.time + CL_FRAME_MILLISECONDS;
            l->track_dst=track_dst;
            return;
        }
    }
}


typedef struct poison_hook_s
{
	float	time;
} poison_hook_t;

void CL_PoisonPhysics(cparticle_t* p)
{
	float time, total_time; 
	poison_hook_t* pHook = (poison_hook_t*)p->pHook;
	
	// calculate time passed
	time = (cl.time - pHook->time)*0.001;
	total_time = (cl.time - p->time)*0.001;

	// update time
	pHook->time = cl.time;

	// move particle
	p->org.x = p->org.x + p->vel.x*time;
	p->org.y = p->org.y + p->vel.y*time;
	p->org.z = p->org.z + p->vel.z*time;

	// modify physics based on current state
	if (total_time > 0.5f)
	{
		p->alphavel = -0.4f;
		p->pscale += 1.0f;
	}
}

void CL_CryoSpray()
{
	int				i;
    CVector			org, ang;
	CVector			x,y,z;
	cparticle_t*	p;
	float			c_ang, c_dir, vel;
	poison_hook_t*	pHook;

    // read origin
#ifdef _DEBUG
	SHOWNET("CL_CryoSpray");
#endif /* DEBUG */
	MSG_ReadPos(&net_message,org);
	// read the direction
	MSG_ReadPos(&net_message,ang);

	AngleToVectors(ang,z,x,y);

	for (i=0;i<35;i++)
	{
		// spawn a new poison particle
		p = CL_SpawnComParticle();

		// warn if we didn't get one - DEBUGGING ONLY
		if (!p)
		{
#ifdef _DEBUG
			Com_Printf("WARNING: No particle available for poison spray.\n");
#endif
			return;
		}
		
		c_ang = frand() * 5.0f;				// 0 to 5
		c_dir = frand() * 359.9f;			// 0 to 360
		vel = (frand() * 75.0f) + 200.0f;	// 200 to 275

		// get hook
		pHook = (poison_hook_t*)X_Malloc(sizeof(poison_hook_t),MEM_TAG_HOOK);
		p->pHook = pHook;
		// start time of the particle
		p->time = cl.time;
		pHook->time = cl.time;
		// green poison particle
		p->type = PARTICLE_CRYOSPRAY;
		// origin should be the same as the muzzle
		p->org = org;
		p->lastorg = org;
		// shoot particles in a random cone
		p->vel = cos(DEG2RAD(c_ang)) * z + 
			sin(DEG2RAD(c_ang)) * cos(DEG2RAD(c_dir)) * x + 
			sin(DEG2RAD(c_ang)) * sin(DEG2RAD(c_dir)) * y;
		// normalize the angle
		p->vel.Normalize();
		// add the velocity
		p->vel.x *= vel;
		p->vel.y *= vel;
		p->vel.z *= vel;
		// full alpha to start
		p->alpha = 0.4f;
		// set scale
		p->pscale = 14.0f + crand()*6.0f;
		// set alpha falloff, none at first, set in physics
		p->alphavel = 0.0f;
		// set particle physics
		p->pFunc = CL_PoisonPhysics;
		p->color_rgb.Set( 0.03, 0.8, 1 );
	}
}

///////////////////////////////////////////////////////////////////////
// Logic[3/26/99]: Support routines to expedite message dispatch in
//  CL_ParseTEnt()
//
// Notes:
//      This approach burns an extra 4 cycles to get the net messages
//      out of the way in deference to just executing these routines
//      directly. It's a LOT better than running through 60 comparisons
//      to try to find the correct function to run.
//
//      The collection of functions below are in place for messages 
//      which reference effects routines called from elsewhere in the 
//      game. Otherwise the effects functions were altered to read the
//      net messages directly. (See CL_ParseLaser, vs the TempEvent_ 
//      functions)
//
///////////////////////////////////////////////////////////////////////
// Logic[3/30/99]: Adding trackData structure. This info is set and
// passed to track functions within the CL_AddTrackEnts() loop through
// the track entities.

//void CL_ParticleEffect_Sparkles ( CVector &org, CVector &dir, int color, int count);



// ----------------------------------------------------------------------------
// Creator:<cek>
//
// Name:		CL_AddLaserEnd
// Description:	Puts a sprite at the end of a laser beam to make it not look like 
//				just a line				
// Input:		pos: where to put it
//				ang: direction of laser beam
//				modelIndex: index of sprite to use
//				radius: base radius of the desired sprite (will be randomized)
//				alpha : alpha of the sprite
// Output:		nada
// Note:		Does not draw the beam!
// ----------------------------------------------------------------------------
void CL_AddLaserEnd( CVector &pos, CVector &ang, int modelIndex, float radius, float alpha, short thirdperson)
{
	float r1 = 1.0 + (0.5*(frand() - 0.5));
	float r2 = 1.0 + (0.5*(frand() - 0.5));
	float r3 = 1.0 + (0.5*(frand() - 0.5));

	entity_t sprite;
	memset(&sprite,0,sizeof(sprite));
	sprite.model = cl.model_draw[modelIndex];
	sprite.angles = ang;
	sprite.angles.y = sprite.angles.y + 90*(frand()-0.5);
	sprite.origin = pos;
	sprite.render_scale.Set(r1*radius,r2*radius,r3*radius);
	sprite.alpha = alpha;
	sprite.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
	if (thirdperson)
		sprite.flags |= RF_DEPTHHACK;

	sprite.frame=sprite.oldframe=0;
	V_AddEntity(&sprite);
}

void TrackEntRFX_Default(trackParamData_t *trackData);

// ----------------------------------------------------------------------------
// Creator:<cek>
//
// Name:		CL_GetWeaponMuzzlePoint
// Description:	Will return the muzzle point of the current weapon in either
//				first or third person view				
// Input:		trackParamData_t *trackData
// Output:		hardPt contains the computed muzzle point
// Note:		trackData must contain the following data:
//					owner(srcent) is the player
//					ent is a track entity (not the 1st person weapon entity!)
//					tracker->modelindex2 is the third person weapon model index
//					extraent is the 1st person weapon
// ----------------------------------------------------------------------------
void CL_GetPlayerHandPoint(trackParamData_t *trackData, char* szHardPt, CVector &hardPt)
{
	entity_t entTemp;
	memset(&entTemp,0,sizeof(entTemp));

	centity_t *owner = trackData->owner;
	// first, get the origin of the 3rd person model (where it's connected to the player)
    entTemp.origin = owner->lerp_origin;
	entTemp.origin = entTemp.origin.Interpolate(owner->current.origin,owner->prev.origin,cl.lerpfrac);
	entTemp.render_scale = owner->current.render_scale;;

	// interpolate the angle!
	LerpAngles(owner->prev.angles,owner->current.angles,entTemp.angles,cl.lerpfrac);
	entTemp.angles.roll *= -1;
    entTemp.model = cl.model_draw[owner->current.modelindex];
    entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
	entTemp.backlerp = 0.0;
		
//	entTemp.oldframe = backframe[trackData->tracker->src_index];
	entTemp.backlerp = 1.0 - owner->fFrameLerp;

	hardPt.Set(0.0,0.0,0.0);
	re.GetModelHardpoint(szHardPt, trackData->owner->current.frame, trackData->owner->prev.frame, entTemp, hardPt);
	if (hardPt.Length() < 0.01)
	{
		hardPt = trackData->owner->lerp_origin;
	}

}
void CL_GetWeaponMuzzlePoint(trackParamData_t *trackData, char* szHardPt, CVector &hardPt)
{
	entity_t entTemp;
	memset(&entTemp,0,sizeof(entTemp));

	if (trackData->thirdPersonView)
	{
		hardPt.Set(0.0,0.0,0.0);
		CL_GetPlayerHandPoint(trackData,"hp_gun",hardPt);
		if (hardPt.Length() < 0.01)
		{
			hardPt = trackData->owner->lerp_origin;
			return;
		}

        entTemp.origin = hardPt;
		LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
		entTemp.render_scale = trackData->owner->current.render_scale;
        entTemp.model = cl.model_draw[trackData->tracker->modelindex2];
        entTemp.frame = entTemp.oldframe = 4;
		entTemp.backlerp = 0.0;

		hardPt.Set(0.0,0.0,0.0);
		re.GetModelHardpoint(szHardPt, 4, 4, entTemp, hardPt);  
		if (hardPt.Length() < 0.01)
			hardPt = entTemp.origin;
	}
	else
	{
		if (!trackData->extraent)
		{
			hardPt = trackData->origin;
			return;
		}

		hardPt.Set(0.0,0.0,0.0);
		entTemp.render_scale = trackData->extraent->current.render_scale;;
        entTemp.origin = trackData->origin;
        entTemp.angles = trackData->angles;
 		entTemp.angles.roll *= -1;
		entTemp.model = cl.model_draw[trackData->extraent->current.modelindex];
//        entTemp.frame = entTemp.oldframe = trackData->tracker->curframe;
		entTemp.frame = trackData->extraent->current.frame;
		entTemp.oldframe = trackData->extraent->prev.frame;
		entTemp.backlerp = 1.0-trackData->extraent->fFrameLerp;//cl.lerpfrac;

//		re.GetModelHardpoint(szHardPt, trackData->track->current.frame, trackData->track->prev.frame, entTemp, hardPt); 
		re.GetModelHardpoint(szHardPt, entTemp.frame, entTemp.oldframe, entTemp, hardPt); 
		if (hardPt.Length() < 0.01)
			hardPt = trackData->origin;
	}
}

int beam_AddNovaLaser( CVector &start, CVector &end, CVector &rgbColor, float alpha, float radius, int texIndex, unsigned short flags);


// ----------------------------------------------------------------------------
// Creator:<nss>
// Name:		TrackEnt_ElectricSword
// Description:Uses CL_GenerateComplexParticle to create fx for Knight2's 
// electric battle Axe.
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_ElectricSword( trackParamData_t *trackData  )
{

	float /*RN,*/Modulation,Alpha,Scale;
    entity_t    /*ent, */entTemp;
//	te_tracker_t    *tracker;	
    CVector         vecHardPoint1,vecHardPoint2,Dir,Color,N_Origin, Origin, Angles;  
    
//	tracker = trackData->tracker;

	//Setup a temp entity and get the hardpoints
	trackData->owner->current.frame;
    entTemp.origin = trackData->owner->lerp_origin;//cek current.origin;
//    entTemp.angles = trackData->owner->current.angles;
	LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
    entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
    entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
	entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
	entTemp.render_scale = trackData->owner->current.render_scale;
//	re.GetModelHardpoint("hr_muzzle1", trackData->owner->current.frame, trackData->owner->current.frameInfo.endFrame, entTemp,vecHardPoint1);
//	re.GetModelHardpoint("hr_muzzle2", trackData->owner->current.frame, trackData->owner->current.frameInfo.endFrame, entTemp,vecHardPoint2);
	re.GetModelHardpoint("hr_muzzle1", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint1);
	re.GetModelHardpoint("hr_muzzle2", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint2);
	
	//Start the Lightning
	for(int i=0;i<=7;i++)
	{
		//Generate Random Factor
//		RN = frand();
		
		//Interpolate the last two points
		if(i>=5)
		{
			Origin		= vecHardPoint2;
			Dir			= vecHardPoint2 - vecHardPoint1;
			Dir.Normalize();
			VectorToAngles(Dir,Angles);
			
			Origin		= vecHardPoint2;
		
			Angles.pitch += crand()*100.0f;
			Angles.yaw	 += crand()*100.0f;
			Angles.AngleToForwardVector(Dir);
			N_Origin	= (Dir * 20.0f) + vecHardPoint2;
			Scale		= 1.4f;
			Modulation	= 0.55f;
			Alpha		= 0.45f;
			
		}
		else if(i>=3 && i <=4)
		{
			Scale		= 2.25f;
			Modulation	= 1.0f;
			Alpha		= 0.45f;
			Origin		= vecHardPoint2;
			N_Origin	= vecHardPoint1.Interpolate(vecHardPoint1,vecHardPoint2,i*0.33);
		}
		else
		{
			//Scale		= 5.35f;
			Scale		= 12.35f;
			Modulation	= 2.0f;
			Alpha		= 0.35f;
			Origin		= vecHardPoint1;
			N_Origin = vecHardPoint1.Interpolate(vecHardPoint1,vecHardPoint2,i*0.33);
		}

		
		
		Color.Set(0.015,0.45,0.65);

		beam_AddLightning ( Origin, N_Origin, Color, Alpha, Scale, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE, Modulation);
	}
}

// ----------------------------------------------------------------------------
// Creator:<nss>
// Name:		TrackEnt_Stavros
// Description:Uses CL_GenerateComplexParticle to create fx for Stavros's meteor
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_Stavros( trackParamData_t *trackData )
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,C1,C2;
	N_Origin = trackData->owner->current.origin - trackData->owner->current.old_origin;
	N_Origin =  trackData->owner->current.old_origin + (N_Origin * 0.650);
	//See notes on Complex particle creation to understand this section 
	//Create the fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color.Set(0.85f,0.15f,0.05f);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.95f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-20.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.0f+frand()*1.0,5.0f,360,150.0f,PARTICLE_FIRE,Gravity);


	//Create the darker fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color.Set(0.85f,0.15f,0.05f);
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.65f,1.45f,0.10f);

	//Gravity settings
	Gravity.Set(crand()*20.0+10.0f,crand()*20.0+10.0f,crand()*20.0+10.0f);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,4.0f+(frand()*2.0),5, 360,200.0f,PARTICLE_CP4,Gravity);
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_ComplexParticle
// Description:Uses CL_GenerateComplexParticle to create a stream of particles
// based on the settings from the map boyZ and girl.
// Input:trackParamData_t *trackData,  entity_t *ent
// Output:<nothing>
// Note:
/*
   float length;	----> Alpha_Level
   CVector lightColor;--> Color
   float lightSize;	----> Delta_Alpha
   int modelindex;  ----> Particle_Type
   short numframes; ----> Spread
   float scale;		----> Scale	
   float frametime; ----> Emit_Freq
   altpos.x   ----> NumParticles
   altpos.y   ----> Velocity
   altpos.z   ----> GType
*/
// ----------------------------------------------------------------------------
void TrackEnt_ComplexParticle(trackParamData_t *trackData, entity_t *ent)
{
	// trackData.track->current.origin should contain the offset of the trackent 
	CVector N_Origin,O_Origin,Dir,Color,Alpha;
	float Scale;
//	float RN;
	int GO = 0;
	int Emit_Freq = (int)trackData->tracker->frametime;
	if(Emit_Freq <= 0 )
		Emit_Freq = 1;
	
	//All of this is timing related bullshit
	//This is where I determine when to actually spurt out particles
	if(trackData->tracker->frametime)
	{
		if(!(cls.framecount % Emit_Freq))
		{
			GO=1;
			if(trackData->tracker->scale2)
			{
				//Let's swap time delay
				float frame = trackData->tracker->frametime;
				if(trackData->tracker->altangle.z)
				{
					trackData->tracker->frametime = trackData->tracker->altangle.y;
				}
				else
				{
					//First Delay is the longest and the full length... from there on out they are random
					if(trackData->tracker->modelindex2)
					{
						//NSS[11/20/99]:THIS IS RIGHT NOEL DON'T CHANGE AGAIN!*self note*
						//trackData->tracker->frametime = (trackData->tracker->altangle.y*0.25)+(frand()*(trackData->tracker->altangle.y*0.75));
						trackData->tracker->frametime = (trackData->tracker->altangle.y*0.25)+(frand()*(trackData->tracker->altangle.y*0.75));
						
						if(trackData->tracker->frametime <= 0.0)
						{
							trackData->tracker->frametime	= trackData->tracker->trailLastVec.x*0.25;
							trackData->tracker->altangle.y	= trackData->tracker->trailLastVec.x;
						}
						else if(trackData->tracker->trailLastVec.x <= 0.0f)
						{
							trackData->tracker->trailLastVec.x = trackData->tracker->altangle.y;
						}

					}
					else
					{
						trackData->tracker->frametime = trackData->tracker->altangle.y;
					}
				}
				trackData->tracker->altangle.y = frame;
				if(trackData->tracker->altangle.z)
				{
					trackData->tracker->altangle.z = 0;
				}
				else
				{
					trackData->tracker->altangle.z = 1;
				}
			}		
		}
	}
	else
	{
		GO = 1;
	}

	if(trackData->tracker->altangle.z)
	{
		GO = 1;
	}

	//This is where the actual generation of particles is done.
	if(GO)
	{
		N_Origin = trackData->owner->lerp_origin;

		//See notes on Complex particle creation to understand this section 
		//CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type, int Gravity)
		//Generate Random Factor
//		RN = frand();
		Scale = ((trackData->tracker->scale*.5)*crand())+trackData->tracker->scale;

		//Setup the Particle Direction let the spread take care of the zaxis
		if(trackData->tracker->altpos2 == trackData->origin)
		{
			if(trackData->tracker->altpos.z == 0)
			{
				Dir.Set(crand(),crand(),0.0);
			}
			else if(trackData->tracker->altpos.z == 1)
			{
				Dir.Set(crand(),crand(),frand());
			}
			else
			{
				Dir.Set(crand(),crand(),frand()*-1);
			}
		}
		else
		{
			Dir = trackData->tracker->altpos2;
		}
		//Set the Color
		Color = trackData->tracker->lightColor;

		//Set the Alpha and Delpetion Rate
		Alpha.Set(trackData->tracker->length,trackData->tracker->lightSize,0);
		
		//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
		//Ok this is a hack but I didn't want to go through all the uses of Complexparticle2 and make changes in case it would screw things up...so sue me.
		//NSS[11/18/99]:
		if(trackData->tracker->altangle.x)
		{
			CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,Scale,trackData->tracker->altpos.x, trackData->tracker->numframes,trackData->tracker->altpos.y,trackData->tracker->modelindex,trackData->tracker->dstpos,trackData->tracker->altangle.x);
		}
		else
		{
			CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,Scale,trackData->tracker->altpos.x, trackData->tracker->numframes,trackData->tracker->altpos.y,trackData->tracker->modelindex,trackData->tracker->dstpos);
		}
	}	
}

// ----------------------------------------------------------------------------
// Creator:			<cek>
//
// Name:			TrackEnt_ShrinkingModel
// Description:		Creates a cool shrinking model for a track entity
// Input:			trackParamData_t *trackData
// Output:			<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_ShrinkingModel(trackParamData_t *trackData )
{
	// get the current scale and make sure it's not too small already
    te_tracker_t *tracker = trackData->tracker;
	CVector curScale = tracker->altpos;
	if (curScale.Length() <= 0.1)
	{
		tracker->fxflags &= ~TEF_SHRINKINGMODEL;
		return;
	}

	if (tracker->modelindex2)
	{
		// get the current scale
		CVector minScale = tracker->dstpos;
		if (tracker->scale != 0.0)				// do fractional scaling
		{
			curScale -= ( curScale * tracker->scale );
		}
		else
		{
			curScale -= tracker->altpos2;
		}

		// see if we should draw the thing
		if ((curScale.x < minScale.x) || (curScale.y < minScale.y) || (curScale.z < minScale.z))
		{
			tracker->altpos.Set(0.0,0.0,0.0);
			tracker->fxflags &= ~TEF_SHRINKINGMODEL;
			return;
		}	
		tracker->altpos = curScale;
	}

	CVector origin = trackData->track->lerp_origin;

	entity_t ent;
	memset(&ent,0,sizeof(entity_t));

	ent.origin = origin;					// origin from srcent
    ent.model  = cl.model_draw[tracker->modelindex];				// modelindex
    ent.alpha = tracker->lightSize;									// alpha
	ent.render_scale = curScale;									// scale
	ent.angles = tracker->altangle;									// angle
    ent.frame=ent.oldframe=tracker->curframe;

	
	V_AddEntity(&ent);
}


// ----------------------------------------------------------------------------
// function defines for Artifact FX
// ----------------------------------------------------------------------------
void ArtFx_Fire(trackParamData_t* trackData);
void ArtFx_Lightning(trackParamData_t* trackData);
void ArtFx_Undead(trackParamData_t* trackData);
void ArtFx_BlackBox(trackParamData_t* trackData);
void ArtFx_Fountain(trackParamData_t* trackData);
void ArtFX_Daikatana(trackParamData_t* trackData);
void ArtFX_StavrosAmmo(trackParamData_t *trackData);
void ArtFx_BloodCloud(trackParamData_t* trackData);
void ArtFx_KageBounce(trackParamData_t* trackData);
void ArtFx_Buboid_Melt(trackParamData_t* trackData);
void ArtFx_FrogSpit(trackParamData_t *trackData);
void ArtFx_Dragon_Breath(trackParamData_t *trackData);
void ArtFx_Dragon_Fireball(trackParamData_t *trackData);
// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEntFX_Artifact
// Description:Creates an FX for artifacts
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: All functions are in Artifact_fx.cpp
// trackData->tracker->Long1  (flags)
// ----------------------------------------------------------------------------
void TrackEntFX_Artifact(trackParamData_t* trackData)
{
	//I purposely made it such that only 1 fx could be used per artifact.
	switch(trackData->tracker->Long1)
	{
		case ART_FIRE:
		{
			ArtFx_Fire(trackData);
			break;
		}
		case ART_LIGHTNING:
		{
			ArtFx_Lightning(trackData);
			break;
		}
		case ART_UNDEAD:
		{
			ArtFx_Undead(trackData);
			break;
		}
		case ART_BLACKBOX:
		{
			ArtFx_BlackBox(trackData);
			break;
		}
		case ART_FOUNTAIN:
		{
			ArtFx_Fountain(trackData);
			break;
		}
		case ART_STAVROS_AMMO:
		{
			ArtFX_StavrosAmmo(trackData);
			break;
		}
		case ART_BLOODCLOUD:
		{
			ArtFx_BloodCloud(trackData);
			break;
		}
		case ART_KAGE_BOUNCE:
		{
			ArtFx_KageBounce(trackData);
			break;
		}
		case ART_BUBOID_MELT:
		{
			ArtFx_Buboid_Melt(trackData);
			break;
		}
		case ART_FROGINATOR_SPIT:
		{
			ArtFx_FrogSpit(trackData);
			break;
		}
		case ART_DRAGON_BREATH:
		{
			ArtFx_Dragon_Breath(trackData);
			break;
		}
		case ART_DRAGON_FIREBALL:
		{
			ArtFx_Dragon_Fireball(trackData);
			break;
		}
		default:// NSS[2/16/00]:For those that use the long1 for other flags
		{
			// NSS[2/16/00]:Not sure if this did this or not.
			if( trackData->tracker->Long1 & ART_DAIKATANA )
				ArtFX_Daikatana(trackData);
			break;
		}
	}
	/*
	if( trackData->tracker->Long1 & ART_FIRE )
		ArtFx_Fire(trackData);
	else if( trackData->tracker->Long1 & ART_LIGHTNING )
		ArtFx_Lightning(trackData);
	else if( trackData->tracker->Long1 & ART_UNDEAD )
		ArtFx_Undead(trackData);
	else if( trackData->tracker->Long1 & ART_BLACKBOX )
		ArtFx_BlackBox(trackData);
	else if( trackData->tracker->Long1 & ART_FOUNTAIN )
		ArtFx_Fountain(trackData);
	else if( trackData->tracker->Long1 & ART_DAIKATANA )
		ArtFX_Daikatana(trackData);
	else if( trackData->tracker->Long1 & ART_STAVROS_AMMO)
		ArtFX_StavrosAmmo(trackData);
	else if( trackData->tracker->Long1 & ART_BLOODCLOUD)
		ArtFx_BloodCloud(trackData);
	else if( trackData->tracker->Long1 & ART_KAGE_BOUNCE)
		ArtFx_KageBounce(trackData);
	else if( trackData->tracker->Long1 & ART_BUBOID_MELT)
		ArtFx_Buboid_Melt(trackData);
	else if( trackData->tracker->Long1 & ART_FROGINATOR_SPIT)
		ArtFx_FrogSpit(trackData);*/

}



// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_SmokeTrailIt
// Description:Creates a SmokeTrail for a Tracked Entity
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_SmokeTrailIt(trackParamData_t *trackData)
{
	// trackData.track->current.origin should contain the offset of the trackent 
	CVector N_Origin,O_Origin,Dir,Color,Alpha;
	float RN;
	
	N_Origin = trackData->owner->lerp_origin;

	//See notes on Complex particle creation to understand this section 
	//CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type)

	
	//Generate Random Factor
	RN = frand();
	//Setup the Smoke
	Dir.Set(0.0f,0.0f,0.0f);
	//Set it to a Yellowish color
	//Color.Set(0.0f,0.0f,0.0f);
	Color.Set(0.85,0.85,0.85);
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.10f,(RN * 0.10)+0.55f,0.05f);
	//Get busy Wit' It.
	//CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (frand() * 1.35f), 1, 6,100.2f,PARTICLE_SMOKE);
	CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (RN * 0.25f)+0.75f, 6, ((1.0f - RN)*3)+2.0f,100.2f,PARTICLE_SMOKE,2);
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_ThunderSkeetFx
// Description:Creates Special Fx for ThunderSkeet
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_ThunderSkeetFx(trackParamData_t *trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
	N_Origin = trackData->owner->lerp_origin;
	//See notes on Complex particle creation to understand this section 
	//Create the fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	//Color.Set(0.45f,0.85f,0.15f);
	Color.Set(0.90f,0.90f,0.20f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.65f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-10.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.0f+frand()*1.0,3,360,50.0f,PARTICLE_CP4,Gravity);


	//Create the darker fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	//Color.Set(0.35f,0.35f,0.05f);
	Color.Set(0.60f,0.60f,0.05f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.35f,0.35f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,30.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.25f+(frand()*1.0),1,360,40.0f,PARTICLE_SMOKE,Gravity);

}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_JetTrail
// Description:Creates a SmokeTrail for a Track Entity
// Input:trackParamData_t *trackData,  entity_t *ent
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_JetTrail(trackParamData_t *trackData, entity_t *ent)
{
	// trackData.track->current.origin should contain the offset of the trackent 
	CVector N_Origin,O_Origin,Dir,Color,Alpha;

	float RN;

	N_Origin = trackData->owner->current.origin;

	ent->angles = trackData->owner->current.angles;
	//See notes on Complex particle creation to understand this section 
	//CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type)

	//Generate Random Factor
	RN = frand();
	//Setup the Smoke
	Dir.Set(0.0f,0.0f,3.0f);
	//Set it to a Yellowish color
	//Color.Set(0.0f,0.0f,0.0f);
	Color.Set(-1.0f,-1.0f,-1.0f);
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.10f,(RN * 0.10)+0.05f,0.05f);
	//Get busy Wit' It.
	//CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (frand() * 1.35f), 1, 6,100.2f,PARTICLE_SMOKE);
	CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (RN * 1.0f)+1.0f, 1, ((1.0f - RN)*3)+2.0f,100.2f,PARTICLE_SMOKE,2);
	
	//Generate Random Factor
	RN = frand();
	//Setup the Bits
	Dir.Set(.55f,.55f,-.75f);
	//Set it to a Grey color
	//Color.Set(0.0f,0.0f,0.0f);
	Color.Set((RN *.20f)+0.35f,0.25f,0.00f);
	//Set the Alpha and Delpetion Rate
	Alpha.Set((RN * 0.15)+0.45f,(RN*.20)+0.35f,0.15f);
	//Get busy Wit' It.
	CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (RN * .65f)+0.65f, 2, ((1.0f - RN)*5)+4.0f,(RN*100)+640.2f,PARTICLE_BLOOD3,2);

	//Generate Random Factor
	RN = frand();
	//Setup the Jet Stream
	Dir.Set(0.65f,0.25f,-.50f);
	//Set it to a redish/orange color
	Color.Set((RN *.20f)+0.35f,0.25f,0.00f);
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.15f,(RN*.20)+0.35f,0.15f);
	//Get busy Wit' It.
	//CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, 8.0f, 4, 12, 240.0f,PARTICLE_SPARKS);
	CL_GenerateComplexParticle( N_Origin, Dir, Color, Alpha, (RN * 3.0f)+7.0f, 4, ((1.0f - RN)*5)+4.0f,(RN*100)+640.0f,PARTICLE_SPARKS,2);
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_Model
// Description:Spawn a model out with and offset calculated from the model's
// hardpoint to the entity being spawned.  Keeps the offset and uses this until 
// the entity becomes un-tracked or dies.
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_Model(trackParamData_t *trackData)
{

//	float RN = frand();
    entity_t    ent, entTemp;
	te_tracker_t    *tracker;	
    CVector         vecHardPoint;  
    
	tracker = trackData->tracker;

	float scale = tracker->scale2;  
    memset(&ent,0,sizeof(ent));
	
    //Are we using 2 models 
	//NSS[11/11/99]:Yes this is a semi hack... 
	if(tracker->fxflags & TEF_MODEL2)
	{
		ent.model = cl.model_draw[(int)tracker->modelindex];
	}
	else
	{
		ent.model = cl.model_draw[(int)tracker->modelindex2];
	}
    
	ent.angles = trackData->angles;
    
	ent.alpha = 1.00f;
    
    ent.frame=ent.oldframe=tracker->curframe;
	//If we do not want to attach the model/sprite and we haven't stored the offset let's do so.. 
	if(tracker->altpos2.x == 0.0f && tracker->altpos2.y == 0.0f && tracker->altpos2.z == 0.0f && !(tracker->fxflags & TEF_ATTACHMODEL))
	{
		entity_t entTemp;
//		trackData->extraent->current.frame;
        entTemp.origin = trackData->extraent->lerp_origin;
		LerpAngles(trackData->extraent->prev.angles,trackData->extraent->current.angles,entTemp.angles,cl.lerpfrac);
        //entTemp.angles = trackData->extraent->current.angles;1
        entTemp.model = cl.model_draw[trackData->extraent->current.modelindex];
        entTemp.frame = entTemp.oldframe = trackData->extraent->current.frame;
		entTemp.backlerp = 0.0f;
		entTemp.render_scale = trackData->extraent->current.render_scale;
		if(tracker->fxflags & TEF_MODEL2)
		{
			re.GetModelHardpoint(tracker->HardPoint, trackData->extraent->current.frame, trackData->extraent->current.frameInfo.endFrame, entTemp,vecHardPoint);
		}
		else
		{
			re.GetModelHardpoint(tracker->HardPoint2, trackData->extraent->current.frame, trackData->extraent->current.frameInfo.endFrame, entTemp,vecHardPoint);
		}
		if(vecHardPoint.x != 0.0f || vecHardPoint.y != 0.0f || vecHardPoint.z != 0.0f)
			tracker->altpos2 = vecHardPoint - trackData->owner->lerp_origin;
		else
			tracker->altpos2 = vecHardPoint;
	}

	//Do we have the offset from the HardPoint
	if (tracker->fxflags & TEF_ATTACHMODEL)
	{
		//Setup a temp entity and get the hardpoints
		//trackData->owner->current.frame;
		//entTemp.origin = trackData->owner->current.origin;
		//entTemp.angles = trackData->owner->current.angles;
	    entTemp.origin = trackData->owner->lerp_origin;
		LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
		entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
		entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
		entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
		entTemp.render_scale = trackData->owner->current.render_scale;
		if(tracker->fxflags & TEF_MODEL2)
		{
			re.GetModelHardpoint(tracker->HardPoint, trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint);
		}
		else
		{
				re.GetModelHardpoint(tracker->HardPoint2, trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint);
		}
		ent.origin = vecHardPoint;
		if(tracker->renderfx & SPR_ALPHACHANNEL)
		{
			ent.alpha = 0.50f;
			ent.flags=SPR_ALPHACHANNEL;
		}
	}
    else
	{
		trackData->owner->lerp_origin += tracker->altpos2;
		//Use that offset for displacement 
		ent.origin = trackData->owner->lerp_origin;
	}

	if(scale == 0.0)
		scale = 1.0;
	ent.render_scale.Set(scale,scale,scale);
	
	V_AddEntity(&ent);
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_Model2
// Description:Spawn a model out with and offset calculated from the model's
// hardpoint to the entity being spawned.  Keeps the offset and uses this until 
// the entity becomes un-tracked or dies.
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_Model2(trackParamData_t *trackData)
{

//	float RN = frand();
    entity_t    ent, entTemp;
	te_tracker_t    *tracker;	
    CVector         vecHardPoint;  
    
	tracker = trackData->tracker;

	float scale = tracker->scale2;  
    memset(&ent,0,sizeof(ent));
	
    //Are we using 2 models 
	//NSS[11/11/99]:Yes this is a semi hack... 
	if(tracker->fxflags & TEF_MODEL)
	{
		ent.model = cl.model_draw[(int)tracker->modelindex2];
	}
	else
	{
		ent.model = cl.model_draw[(int)tracker->modelindex];
	}
    
	ent.angles = trackData->angles;
    
	ent.alpha = 1.00f;
    
    ent.frame=ent.oldframe=tracker->curframe;

	if(tracker->altpos2.x == 0.0f && tracker->altpos2.y == 0.0f && tracker->altpos2.z == 0.0f)
	{
		entity_t entTemp;
		//trackData->extraent->current.frame;
        //entTemp.origin = trackData->extraent->current.origin;
        //entTemp.angles = trackData->extraent->current.angles;
        entTemp.origin = trackData->extraent->lerp_origin;
		LerpAngles(trackData->extraent->prev.angles,trackData->extraent->current.angles,entTemp.angles,cl.lerpfrac);

        entTemp.model = cl.model_draw[trackData->extraent->current.modelindex];
        entTemp.frame = entTemp.oldframe = trackData->extraent->current.frame;
		entTemp.backlerp = 0.0f;
		entTemp.render_scale = trackData->extraent->current.render_scale;
		//!!! Must be HardPoint2
		re.GetModelHardpoint(tracker->HardPoint2, trackData->extraent->current.frame, trackData->extraent->current.frameInfo.endFrame, entTemp,vecHardPoint);
		if(vecHardPoint.x != 0.0f || vecHardPoint.y != 0.0f || vecHardPoint.z != 0.0f)
			tracker->altpos2 = vecHardPoint - trackData->owner->lerp_origin;
		else
			tracker->altpos2 = vecHardPoint;
	}

	//Do we have the offset from the HardPoint
	if (tracker->fxflags & TEF_ATTACHMODEL)
	{
		//Setup a temp entity and get the hardpoints
		//trackData->owner->current.frame;
		//entTemp.origin = trackData->owner->current.origin;
		//entTemp.angles = trackData->owner->current.angles;
	    entTemp.origin = trackData->owner->lerp_origin;
		LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
		entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
		entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
		entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
		entTemp.render_scale = trackData->owner->current.render_scale;
		//!!! Must be HardPoint2
		re.GetModelHardpoint(tracker->HardPoint2, trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint);
		ent.origin = vecHardPoint;
		if(tracker->renderfx & SPR_ALPHACHANNEL)
		{
			ent.alpha = 0.50f;
			ent.flags=SPR_ALPHACHANNEL;
		}
	}
    else
	{
		trackData->owner->lerp_origin += tracker->altpos2;
		//Use that offset for displacement 
		ent.origin = trackData->owner->lerp_origin;
	}

	if(scale == 0.0)
		scale = 1.0;
	ent.render_scale.Set(scale,scale,scale);
	
	V_AddEntity(&ent);
}




// ----------------------------------------------------------------------------
// Creator:<nss>
// NSS[11/4/99]:
// Name:		TrackEnt_FlameSword
// Description:Add's flame FX to the knight1's sword
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_FlameSword(trackParamData_t *trackData)
{

	float RN;
    entity_t    /*ent, */entTemp;
//	te_tracker_t    *tracker;	
    CVector         vecHardPoint1,vecHardPoint2,Dir,Color,C1,C2,Alpha,N_Origin, Gravity;  
    
//	tracker = trackData->tracker;

	//Setup a temp entity and get the hardpoints
	//trackData->owner->current.frame;
    entTemp.origin = trackData->owner->lerp_origin;//cek current.origin;
//    entTemp.angles = trackData->owner->current.angles;
	LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,trackData->owner->fFrameLerp);
    entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
    entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
	entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
	entTemp.render_scale = trackData->owner->current.render_scale;
//	re.GetModelHardpoint("hr_muzzle1", trackData->owner->current.frame, trackData->owner->current.frameInfo.endFrame, entTemp,vecHardPoint1);
//	re.GetModelHardpoint("hr_muzzle2", trackData->owner->current.frame, trackData->owner->current.frameInfo.endFrame, entTemp,vecHardPoint2);
	re.GetModelHardpoint("hr_muzzle1", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint1);
	re.GetModelHardpoint("hr_muzzle2", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint2);


	//Setup the Offset
	Dir = vecHardPoint2 - vecHardPoint1;
	Dir.Normalize();
	
	for(int i=0;i<=2;i++)
	{
		//Generate Random Factor
		RN = frand();
		
		//Interpolate the last two points
		if(i>0)
			N_Origin = vecHardPoint1.Interpolate(vecHardPoint1,vecHardPoint2,i*.33);
		else
			N_Origin = vecHardPoint1;
		
		//FLAME SEGMENT PART I
		//Set the Color
		Color.Set(0.85f,0.15f,0.05f);
		//Set the Gravity
		Gravity.Set(0.0f,0.0f,10.0f);
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.65f,4.85f,0.10f);
		//Get busy Wit' It.
		//CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,5.4f+(RN*2.0),4, 15,220.0f,PARTICLE_CP4,Gravity);
		CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,3.4f+(RN*2.0),4, 15,220.0f,PARTICLE_CP4,Gravity);
		//FLAME SEGMENT PART II
		//Set the Color
		Color.Set(0.85f,0.35f,0.15f);
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.65f,4.85f,0.10f);
		//Gravity settings
		Gravity.Set(0.0,0.0,10.0);
		//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
		//CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,4.7f+RN*1.0,4,15,220.0f,PARTICLE_FIRE,Gravity);
		CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.7f+RN*1.0,4,15,220.0f,PARTICLE_FIRE,Gravity);

		//SMOKE 
		//Set the Color
		Color.Set(-1.0f,-1.0f,-1.0f);
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.20f,0.60f,0.10f);
		//Gravity settings
		Gravity.Set(0.0,0.0,120.0);
		//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
		CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.15,1,35,55.0f,PARTICLE_SMOKE,Gravity);	
	}

}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_Sludge
// Description:This FX is built for the Sludgeminion
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_Sludge( trackParamData_t *trackData)
{
	// trackData.track->current.origin should contain the offset of the trackent 
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
//	float RN;
	N_Origin = trackData->origin;
	//See notes on Complex particle creation to understand this section 
	//CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type)
	//Generate Random Factor
//	RN = frand();
	//SMOKE 
	//Set the Color
	Color.Set(0.25f,0.25f,0.0f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.75f,0.10f);
	//Gravity settings
	Gravity.Set(0.0,0.0,-220.0);
	//Direction set
	Dir.Set(crand(),crand(),crand());
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.25+(crand()*1.75f),4,360,65.0f,PARTICLE_CP4,Gravity);
	//Splitting them in half to get more random sizes in each spurt.
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.25+(crand()*1.75f),4,360,65.0f,PARTICLE_CP4,Gravity);	
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		TrackEnt_Sprite
// Description:This is used to place a sprite on or around an entity(for glow fx)
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void TrackEnt_Sprite(trackParamData_t *trackData)
{

    CVector v1, v2, v4, end;
    CVector rgbcolor(1,1,1);
    entity_t    ent, entTemp;
    te_tracker_t    *tracker;	
    CVector         vecHardPoint;  


    tracker = trackData->tracker;

	
	float scale = tracker->scale;
    memset(&ent,0,sizeof(ent));
	
    if(tracker->fxflags & TEF_ELECTRICSWORD)
	{
		ent.model = cl.model_draw[tracker->modelindex2];
		ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=tracker->scale2;
		ent.alpha = 0.40f;
	}
	else
	{
		ent.model = cl.model_draw[tracker->modelindex];
		ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=tracker->scale;
		ent.alpha = 0.30f;
	}
    ent.angles = trackData->angles;
    ent.origin = trackData->owner->lerp_origin;
    
    ent.flags=SPR_ALPHACHANNEL|(ent.flags & RF_DEPTHHACK);
	ent.frame=ent.oldframe=tracker->curframe;

    tracker->curframetime -= cls.frametime;
    if(tracker->curframetime < 0)
    {
        tracker->curframe++;
        if( tracker->curframe >= tracker->numframes)
            tracker->fxflags &= ~(TEF_ANIMATE);//|TEF_3POINT_ANIM);
        tracker->curframetime += tracker->frametime;
    }
	//NSS[11/7/99]:If we passed in a hardpoint and we are using the TEF_ELECTRICSWORD
	if((tracker->flags & TEF_HARDPOINT) && (tracker->flags & TEF_HARDPOINT2) && (tracker->fxflags & TEF_ELECTRICSWORD))
	{
		//Setup a temp entity and get the hardpoints
//		trackData->owner->current.frame;
		entTemp.origin = trackData->owner->lerp_origin;//current.origin;
//		entTemp.angles = trackData->owner->current.angles;
		LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
		entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
		entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
		entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
		entTemp.render_scale = trackData->owner->current.render_scale;
		re.GetModelHardpoint(tracker->HardPoint, trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,vecHardPoint);
		ent.origin = vecHardPoint;
	}
	
//	float RN = frand();
	if(scale == 0.0)
		scale = 1.0;
	ent.render_scale.Set(scale,scale,scale);
	
	V_AddEntity(&ent);
}


void TrackEnt_DestIndex(trackParamData_t *trackData)
{
    trackData->ent->oldorigin = trackData->dstent->current.origin;
}

void TrackEntFX_Light(trackParamData_t *trackData)
{
    float fLightSize;
    te_tracker_t *tracker;

    tracker = trackData->tracker;
	//NSS[10/28/99]:Added this for Dynamically Shrinking lights
	if (tracker->fxflags & TEF_LIGHT_SHRINK)
	{
		if(tracker->extra_index != 1)
		{
			tracker->extra_index = 1;
			fLightSize = tracker->lightSize;
		}
		else
		{
			fLightSize = tracker->lightSize - tracker->length;
			tracker->lightSize = fLightSize;
		}
	}
	else
	{
	    fLightSize = tracker->lightSize;
		if(fLightSize < 0)
		{
        
			fLightSize = trackData->fRandom * -fLightSize;
		}
	}
    V_AddLight(trackData->ent->origin, fLightSize, tracker->lightColor[0], tracker->lightColor[1], tracker->lightColor[2]);
}

void TrackEntFX_Tracer(trackParamData_t *trackData)
{
    CVector vecAltStart, vecAltEnd, vecAltForward,Color,Point1,Point2,Offset;
    float FloaterPoint, fDistance;
    te_tracker_t *tracker;

    vecAltStart = trackData->origin;
	tracker = trackData->tracker;   // don't burn instructions dereferencing

   	vecAltEnd = tracker->altpos;

	fDistance = VectorDistance(vecAltEnd, vecAltStart);

	if(fDistance < 164.0f)
		return;
	//Offset = vecAltEnd - trackData->origin;
	Offset = vecAltEnd - vecAltStart;
	Offset.Normalize();

	FloaterPoint = ((fDistance*0.65) * frand())+(fDistance*0.35);
	Point1 = (Offset * FloaterPoint)+vecAltStart;
	Point2 = Offset * (FloaterPoint+16+(frand()*30))+vecAltStart;

	Color.Set(1.0,0.35,0.05);
	beam_AddLaser (Point1, Point2, Color , 0.12 + frand()*0.04, 0.20, 0.35, BEAM_TEX_LASERSPARK, BEAMFLAG_RGBCOLOR);//|BEAMFLAG_ALPHAFADE);
	//tracker->fxflags &= ~TEF_TRACER;
}

void TrackEntFX_Animate(trackParamData_t *trackData)
{
    int iFlashCount, i;
    CVector v1, v2, v4, end;
    CVector rgbcolor(1,1,1);
    entity_t    ent, entTemp;
    te_tracker_t    *tracker;	
    CVector         vecHardPoint;  

    tracker = trackData->tracker;
    if(!trackData->thirdPersonView)
    {
        memset(&ent,0,sizeof(ent));
        ent.model = cl.model_draw[tracker->modelindex];
        ent.angles = trackData->angles;
        ent.oldorigin = trackData->track->current.old_origin;
        ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=tracker->scale;
        ent.alpha = 1;
        ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED|(ent.flags & RF_DEPTHHACK);
        ent.frame=ent.oldframe=tracker->curframe;

//        iFlashCount = tracker->fxflags & TEF_3POINT_ANIM ? 3 : 1;
        iFlashCount = tracker->Long2 & TEF_3POINT_ANIM ? 3 : 1;
        for(i = 0; i < iFlashCount; i++)
        {
            entTemp.origin = trackData->origin;
            entTemp.angles = trackData->angles;
            entTemp.model = cl.model_draw[trackData->track->current.modelindex];
            entTemp.frame = entTemp.oldframe = tracker->curframe;
			entTemp.backlerp = 0.0f;
            re.GetModelHardpoint(trackData->muzzle_str[i], trackData->track->current.frame, trackData->track->current.frame, entTemp, vecHardPoint);
            ent.origin = vecHardPoint;

            if( !(tracker->fxflags & TEF_NOROTATE) )
                ent.angles.z = (trackData->fRandom - 0.5) * 720;
            ent.render_scale.x = ent.render_scale.y = ent.render_scale.z *= 0.9+crand()*0.1;
            V_AddEntity(&ent);
        }
        tracker->curframetime -= cls.frametime;
        if(tracker->curframetime < 0)
        {
            tracker->curframe++;
            if( tracker->curframe >= tracker->numframes)
                tracker->fxflags &= ~(TEF_ANIMATE);//|TEF_3POINT_ANIM);
            tracker->curframetime += tracker->frametime;
        }

 
    }
}

// a cool sprite with control over alpha!  woohoo!
void TrackEnt_AlphaSprite(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;

    CVector v1, v2, v4, end;
    CVector rgbcolor(1,1,1);
    entity_t    ent/*, entTemp*/;
    CVector         vecHardPoint;  

	float scale = tracker->scale;
	if(scale == 0.0)
		scale = 1.0;
	float alpha = tracker->scale2;
    memset(&ent,0,sizeof(ent));
	
	ent.model = cl.model_draw[tracker->modelindex];
	ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=tracker->scale;
	ent.alpha = alpha;

	ent.angles = trackData->angles;
    ent.origin = trackData->owner->lerp_origin;
	ent.origin += tracker->altpos;
    
    ent.flags=SPR_ALPHACHANNEL|(ent.flags & RF_DEPTHHACK);
	ent.frame=ent.oldframe=tracker->curframe;

    tracker->curframetime -= cls.frametime;
    if(tracker->curframetime < 0)
    {
        tracker->curframe++;
        if( tracker->curframe >= tracker->numframes)
            tracker->fxflags &= ~(TEF_ANIMATE);//|TEF_3POINT_ANIM);
        tracker->curframetime += tracker->frametime;
    }
	
	V_AddEntity(&ent);

}

// some cool stuff for muzzle flashes
typedef struct wFlashInfo_s
{
	void			*flashModel;
	short			frames;
	float			frameTime;
	float			scale;
	float			lightsize;
	CVector			lightcolor;
	CVector			offset;
	float			alpha;
	long			flags;
} wFlashInfo_t;

wFlashInfo_t weaponFlashes[MAX_FLASHES];

void Init_weapon_flashes()
{
	wFlashInfo_t	*curflash = &weaponFlashes[FLASH_ION];
	memset(&weaponFlashes,0,MAX_FLASHES * sizeof(wFlashInfo_t));

	// ion blaster
	curflash->flashModel		= re.RegisterModel("models/global/genflashg.dkm", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST;
	curflash->frameTime			= 0.05;
	curflash->scale				= 3.0;
	curflash->lightsize			= 150;
	curflash->lightcolor		= CVector(0,1,0);
	curflash->offset			= CVector(-2,0,0);
	curflash->alpha				= 0.4;
	curflash->flags				= 0;

	// glock
	curflash = &weaponFlashes[FLASH_GLOCK];
	curflash->flashModel		= re.RegisterModel("models/global/we_mflash.dkm", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST|TEF_ANIM2_SMOKE;
	curflash->scale				= 1.0;
	curflash->lightsize			= 150;
	curflash->lightcolor		= CVector(0.8,0.4,0.2);
	curflash->offset			= CVector(0,0,0);
	curflash->alpha				= 0.6;
	curflash->flags				= TEF_NOROTATE;

	// slugger
	curflash = &weaponFlashes[FLASH_SLUGGER];
	curflash->flashModel		= re.RegisterModel("models/global/genflash.dkm", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST|TEF_ANIM2_SMOKE;
	curflash->scale				= 2.3;
	curflash->lightsize			= 175;
	curflash->lightcolor		= CVector(0.8,0.4,0.2);
	curflash->offset			= CVector(0,0,0);
	curflash->alpha				= 0.6;
	curflash->flags				= 0;

	// ripgun
	curflash = &weaponFlashes[FLASH_RIPGUN];
	curflash->flashModel		= re.RegisterModel("models/global/genflash.dkm", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST;
	curflash->scale				= 8.0;
	curflash->lightsize			= 175;
	curflash->lightcolor		= CVector(0.8,0.4,0.2);
	curflash->offset			= CVector(-2,0,0);
	curflash->alpha				= 0.6;
	curflash->flags				= 0;

	// shot cycler
	curflash = &weaponFlashes[FLASH_SHOTCYCLER];
	curflash->flashModel		= re.RegisterModel("models/global/genflash.dkm", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST|TEF_ANIM2_SMOKE;
	curflash->scale				= 3.0;
	curflash->lightsize			= 175;
	curflash->lightcolor		= CVector(0.8,0.4,0.2);
	curflash->offset			= CVector(-2,0,0);
	curflash->alpha				= 0.6;
	curflash->flags				= 0;

	// shockwave
	curflash = &weaponFlashes[FLASH_SHOCKWAVE];
	curflash->flashModel		= re.RegisterModel("models/e1/we_mfswave.sp2", RESOURCE_GLOBAL);
	curflash->frames			= 1 | TEF_ANIM2_THIRD|TEF_ANIM2_FIRST;
	curflash->scale				= 0.285;
	curflash->lightsize			= 150;
	curflash->lightcolor		= CVector(1,1,1);
	curflash->offset			= CVector(0,0,0);
	curflash->alpha				= 0.6;
	curflash->flags				= 0;
}

// ----------------------------------------------------------------------------
// Creator:<cek>
//
// Name:		TrackEntFX_Animate2
// Description:	Causes the specified animation sprite to appear at the current
//				weapon's muzzle point as determined by CL_GetWeaponMuzzlePoint.
//				If set up properly, will work in 1st or 3rd person.				
// Input:		trackParamData_t *trackData
// Output:		hardPt contains the computed muzzle point
// Note:		trackData must contain the following data:
//					owner(srcent) is the player
//					ent is a track entity (not the 1st person weapon entity!)
//					tracker->modelindex2 is the third person weapon model index
//					extraent is the 1st person weapon
//					tracker->modelindex is the animation sprite
//					tracker->numframes high byte is first/third person flag 
//						1 to show first person
//						2 to show third person
//					tracker->numframes low byte is the number of frames in the anim
//					tracker->frametime is the frame delay
//					scale is the sprite's scale value
// ----------------------------------------------------------------------------
void TrackEntFX_Animate2(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;

	void *model;
	// this is a muzzle flash.  extract the info from weaponFlashes.
	bool bMuzzleFlash = false;
	if ((tracker->Long1 > FLASH_NONE) && (tracker->Long1 < MAX_FLASHES))
	{
		wFlashInfo_t	*curflash = &weaponFlashes[tracker->Long1];

		model						= curflash->flashModel;
		tracker->numframes			= curflash->frames;
		tracker->frametime			= curflash->frameTime;
		tracker->scale				= curflash->scale;
		tracker->lightSize			= curflash->lightsize * (1.0 + 0.5 * frand());
		tracker->lightColor			= curflash->lightcolor;
		tracker->altpos				= curflash->offset;
		tracker->scale2				= curflash->alpha;

		if (tracker->numframes & TEF_ANIM2_SMOKE)
		{
			if (CM_PointContents (trackData->owner->current.origin, 0) & MASK_WATER)
				tracker->numframes &= ~TEF_ANIM2_SMOKE;
		}

		bMuzzleFlash = true;
	}
	else 
		model = cl.model_draw[tracker->modelindex];

	short bFirst = tracker->numframes & TEF_ANIM2_FIRST;
	short bThird = tracker->numframes & TEF_ANIM2_THIRD;

	if ((bFirst && !trackData->thirdPersonView) || (bThird && trackData->thirdPersonView) )
	{
		CVector start(0,0,0);
		entity_t ent/*, entTemp*/;
		memset(&ent,0,sizeof(ent));

		ent.model = model;//cl.model_draw[tracker->modelindex];
		CVector angles = trackData->angles;
		ent.oldorigin = trackData->track->current.old_origin;
		ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=tracker->scale;
		ent.alpha = (tracker->scale2 > 0.01) ? tracker->scale2 : 0.8;
		ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;

		// acommodate stuff like the trident.
		int iFlashCount = tracker->Long2& TEF_3POINT_ANIM ? 3 : 1;
		for(int i = 0; i < iFlashCount; i++)
		{
			CL_GetWeaponMuzzlePoint(trackData, trackData->muzzle_str[i], start);
			if (bMuzzleFlash)
			{
				if (tracker->lightSize)
				{
					V_AddLight(start,tracker->lightSize,tracker->lightColor.x,tracker->lightColor.y,tracker->lightColor.z);
				}
			}

			ent.origin = start;
			if (tracker->altpos.Length() > 0.1)
			{
				AngleToVectors(trackData->owner->current.angles,forward,right,up);
				ent.origin += tracker->altpos.x * forward;
				ent.origin += tracker->altpos.y * right;
				ent.origin += tracker->altpos.z * up;
			}
			ent.oldorigin = ent.origin;
			ent.angles = angles;
			if( !(tracker->fxflags & TEF_NOROTATE) )
				ent.angles.z = (trackData->fRandom - 0.5) * 720;
			ent.render_scale.x = ent.render_scale.y = ent.render_scale.z *= 0.9+crand()*0.1;
			V_AddEntity(&ent);
			// add a cool billboard effect (if desired)
			if (tracker->numframes & TEF_ANIM2_BILLBOARD)
			{
				ent.angles.z += 90.0;
				ent.flags &= ~SPR_ORIENTED;
				if( !(tracker->fxflags & TEF_NOROTATE) )
					ent.angles.y = (frand() - 0.5) * 720;
				ent.render_scale.x = ent.render_scale.y = ent.render_scale.z *= 0.9+crand()*0.1;
				V_AddEntity(&ent);
			}

			// do some smoke (if desired!), scaled proportionately to the scale
			if (tracker->numframes & TEF_ANIM2_SMOKE)
			{
				float scale = tracker->scale;
				if (scale > 2)
					scale = 2;
				CL_Smoke (start, scale, scale * 2, 3, 10);
			}
		}

		tracker->curframetime -= cls.frametime;
		if(tracker->curframetime < 0)
		{
			tracker->curframe++;
			if( !(tracker->numframes & TEF_ANIM2_LOOP) && (tracker->curframe >= (tracker->numframes & 0xff)))
			{
				if ((tracker->Long1 > FLASH_NONE) && (tracker->Long1 < MAX_FLASHES))
				{
					tracker->freeMe = TRUE;
				}
				else
				{
					tracker->curframe = 0;
					tracker->fxflags &= ~(TEF_ANIMATE2|TEF_LIGHT);//|TEF_3POINT_ANIM);
				}
			}
			tracker->curframetime += tracker->frametime;
		}
	}
	else
	{
		tracker->fxflags &= ~(TEF_ANIMATE2|TEF_LIGHT);//|TEF_3POINT_ANIM);
	}
}

// ----------------------------------------------------------------------------
// Creator:<cek>
//
// Name:			TrackEntFX_Metabeams
// Description:		Handles almost all effects for the metamaser weapon
//					Allows tracking of up to 12 entities which are laser targets
//				
//				
// Input:			Targets:	index of entity
//						0, 1, 2		tracker->altpos
//						3, 4, 5		tracker->altpos2
//						6, 7, 8		tracker->dstpos
//						9,10,11		tracker->altangle
//					light*:		Color and size of the dlight made when a laser is fired
//					scale:		Radius of the beam
//					scale2:		alpha of the beam
//					modelindex:	shoot flare sprite
//					numframes:	bitflags
//						0-11		specifies whether this item is currently a valid target
//						12			use the dlights
//						13			metamaser is having pain (shut down)
//						14			metamaser is in its die cycle
// Note:		
// ----------------------------------------------------------------------------
#define SET(x)	((unsigned short) 7 << (3*(x)))
#define INDEX(x,y) ((x & SET(y)) >> (3*(y)))

#define TRACKBITS			0X0fff			// bits 0-11
#define LIGHTBITS			0x1000			// bit 12
#define PAINBITS			0X2000			// bit 13
#define DIEBITS				0x4000			// bit 14
#define LASTBIT				0X8000			// bit 15

int ProcessVector(short index, const CVector &vec, trackParamData_t *trackData, const CVector &start);
void ProcessCEnt(short entindex, const centity_t* ent, trackParamData_t *trackData, const CVector &start);

void TrackEntFX_Metabeams(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;

	AngleToVectors(trackData->angles,forward,right,up);
	up.Normalize();

	CVector start = trackData->track->current.origin + 15*up;			// it all starts here!
	short bitflags = tracker->numframes;
	if ((bitflags & TRACKBITS) && !(bitflags & PAINBITS))		// no 'on' flags set or in pain...get outa here
	{
		int count = 0;
		short index;
		if (index=INDEX(bitflags,0))							// 000000000111 (SET 0)
		{
			count += ProcessVector(index, tracker->altpos, trackData, start);
		}
		
		if (index=INDEX(bitflags,1))							// 000000111000 (set 1)
		{
			count += ProcessVector(index, tracker->altpos2, trackData, start);
		}

		if (index=INDEX(bitflags,2))							// 000111000000 (set 2)
		{
			count += ProcessVector(index, tracker->dstpos, trackData, start);
		}

		if (index=INDEX(bitflags,3))							// 111000000000 (set 2)
		{
			count += ProcessVector(index, tracker->altangle, trackData, start);
		}
		// add a flare thingy
		if (count)
		{
			entity_t ent;
			memset(&ent,0,sizeof(ent));
			ent.model = cl.model_draw[tracker->modelindex];
			ent.angles = trackData->angles;
			ent.origin = start;
			ent.render_scale.Set(1,1,1);
			ent.alpha = 0.30f;
			ent.flags=SPR_ALPHACHANNEL;
			ent.frame=ent.oldframe=0;
//			V_AddEntity(&ent);
		}

		if (bitflags & LIGHTBITS)
		{
//			float baselight = tracker->lightSize + 30 * count;
			float baselight = 150 + 30 * count;
			baselight *= (1 + 0.5*(frand() - 0.5));
			CVector color(0,0,1);
			V_AddLight(start, baselight, color.x,color.y,color.z);
		}
	}
	if (bitflags & PAINBITS)
	{
		up.x += 45*(frand() - 0.5);
		up.y += 45*(frand() - 0.5);
		up.z += 45*(frand() - 0.5);
		CL_ParticleEffectSparks(start, up, CVector(0.5,0.6,1), 2);

		explosion_t *ex;

				
		// basically a sprite explosion with less light
		if(ex = CL_AllocExplosion ())
		{
			ex->ent.origin = start;
			ex->type = ex_poly;
			ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
			ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
			ex->light = 100;
			ex->lightcolor.Set(1.0, 0.5, 0.0);
			ex->ent.render_scale [0] = ex->ent.render_scale [1] = ex->ent.render_scale [2] = 0.1;
			if (rand () & 1)
			{
				ex->ent.model = cl_mod_explosion [1];
				ex->frames = 8;
			}
			else
			{
				ex->ent.model = cl_mod_explosion [0];
				ex->frames = 7;
			}
		}
	    CL_SmokeAndFlash(start);
	}
	if (bitflags & DIEBITS)
	{
		// make a random explosion
		if ((rand() & 0x0c) == 0x0c)
		{
			explosion_t *ex;
			if(ex = CL_AllocExplosion ())
			{
				ex->ent.origin = start;
				ex->type = ex_poly;
				ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
				ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
				ex->light = 150;
				ex->lightcolor.Set(1.0, 0.5, 0.0);
				ex->ent.render_scale [0] = ex->ent.render_scale [1] = ex->ent.render_scale [2] = 0.15 + (0.2*frand());;
				if (rand () & 1)
				{
					ex->ent.model = cl_mod_explosion [1];
					ex->frames = 8;
				}
				else
				{
					ex->ent.model = cl_mod_explosion [0];
					ex->frames = 7;
				}
			}
		}
	}

	if (bitflags & LASTBIT)
	{
		tracker->numframes &= ~LASTBIT;
		// center is a model -- model is oriented 90 degrees off
		CVector ang = up;
		ang.x += 90;
		float scale = tracker->frametime * 0.075;
		CVector vDirection(0, 0, 1);
		CL_PolyExplosion(start, ang, cl_mod_metaexp, 0.05f, 3.0 + scale, 0.75f, 350, vDirection, RF_FULLBRIGHT|RF_TRANSLUCENT);
		ang.z += 90;
		CL_PolyExplosion(start, zero_vector, cl_mod_metaexp, 0.05f, 3.0 + scale, 0.5f, 350, vDirection, RF_FULLBRIGHT|RF_TRANSLUCENT);
		tracker->frametime += 1.0;
		if(CM_PointContents (start, 0) & MASK_WATER)
			S_StartSound(start, 0, 0, cl_sfx_waterexplosion,1,512,2048);
		else
			S_StartSound(start,0,0,cl_sfx_explosion[0],1,512,2048);
	}
}
/*		CL_PolyExplosion(start, ang, cl_mod_metaexp, 0.05f, 3.0 + scale, 0.5f, 350, CVector(0,0,1), RF_FULLBRIGHT|RF_TRANSLUCENT);
		ang.z += 90;
		CL_PolyExplosion(start, zero_vector, cl_mod_metaexp, 0.05f, 3.0 + scale, 0.5f, 350, CVector(0,0,1), RF_FULLBRIGHT|RF_TRANSLUCENT);
*/
// ----------------------------------------------------------------------------
// ProcessVector() -- used by metamaser effects
// ----------------------------------------------------------------------------
int ProcessVector(short index, const CVector &vec, trackParamData_t *trackData, const CVector &start)
{
	te_tracker_t *tracker = trackData->tracker;
	int count = 0;
	long entindex;
	if (index & 1)		// x
	{
		entindex = (long)vec.x;
		if (entindex)
		{
			count++;
			ProcessCEnt(entindex, &cl_entities[entindex], trackData, start);
		}
	}

	if (index & 2)		// y
	{
		entindex = (long)vec.y;
		if (entindex)
		{
			count++;
			ProcessCEnt(entindex, &cl_entities[entindex], trackData, start);
		}
	}

	if (index & 4)		// z
	{
		entindex = (long)vec.z;
		if (entindex)
		{
			count++;
			ProcessCEnt(entindex, &cl_entities[entindex], trackData, start);
		}
	}
	return count;
}

// ----------------------------------------------------------------------------
// ProcessCEnt() -- used by metamaser effects
// ----------------------------------------------------------------------------
void ProcessCEnt(short entindex, const centity_t* ent, trackParamData_t *trackData, const CVector &start)
{
	te_tracker_t *tracker = trackData->tracker;
	CVector end;
	CVector s = start;
	CVector color(0,0,1);// = tracker->lightColor;

//	hook->tinfo.lightSize = 150;			// size of the dlight
//	hook->tinfo.lightColor.Set(0,0,1);		// color of the beam/dlight
//	hook->tinfo.scale = 4;					// radius of the beam
//	hook->tinfo.scale2 = 0.4;				// alpha
    if ( !(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE) && (entindex==cl.playernum+1) )
    {
        end = cl.refdef.vieworg;
        end.z -= 5;
    }
    else if (tracker->numframes & DIEBITS)		// don't predict for the death lasers...
	{
		end = ent->current.origin;
	}
	else
    {
		end = ent->lerp_origin;
    }

	color.x += 0.1*(frand()-0.5);
	color.x += 0.1*(frand()-0.5);
	color.x += 0.1*(frand()-0.5);
//	float alpha = tracker->scale2 + 0.2*(frand()-0.7);
//	float radius = tracker->scale * (1 + 0.5*(frand() - 0.5));
	float alpha = 0.4 + 0.2*(frand()-0.7);
	float radius = ((tracker->numframes & DIEBITS) ? 6 : 4 )* (1 + 0.5*(frand() - 0.5));
	beam_AddLaser (s, end, color, alpha, radius, radius, BEAM_TEX_LIGHTNING, RF_DEPTHHACK);
	V_AddLight(end, 100,color.x,color.y,color.z);

	CVector temp, ang;
	temp = end-s;
	temp.Normalize();

	trace_t trace;
	CVector vTraceEnd;
	vTraceEnd = end + ( end - start );
    trace = CL_TraceLine(s-2*temp, vTraceEnd, tracker->src_index, MASK_DEADSOLID);

    VectorToAngles(trace.plane.normal, ang);
    if(trace.ent && (trace.ent == (struct edict_s *)0x00000001) && !(trace.surface->flags & SURF_SKY))//(trace.surface && !(trace.surface->flags & SURF_SKY) && (trace.surface->name[0] != NULL))
	{
		// cut down on the friggin surface sprites!
		if (frand() < 0.1)
		{
			CL_NovaEffect(end, trackData->o_forward, trace.plane.normal);
			re.AddSurfaceSprite( trace.endpos, trace.endpos, trace.plane.normal, cl.model_draw[1], trace.plane.planeIndex, cl_mod_metahit, 0, 0, 1, SSF_FULLBRIGHT|SSF_QUICKFADE);
		}
	}

	entity_t ent2;
	memset(&ent,0,sizeof(ent));
	ent2.model = cl.model_draw[tracker->modelindex];
	temp.VectorToAngles(ent2.angles);
	
	ent2.origin = start;
	ent2.render_scale.Set(1 + 0.25*frand(),1 + 0.25*frand(),1);
	ent2.alpha = 0.30f;
	ent2.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
	ent2.frame=ent2.oldframe=0;
	V_AddEntity(&ent2);

	ent2.angles.y += 90;
	V_AddEntity(&ent2);
}

// ----------------------------------------------------------------------------------------------------------------------
// reaper fx
// ----------------------------------------------------------------------------------------------------------------------
void TrackEntFX_Reaper(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;
	centity_s *reaper = trackData->track;

	CVector vMins(-100,-100,-128);
	CVector vMaxs(100,100,200);
	CVector vColor(0.8,0.2,0.1);
	if (tracker->Long1 & TEF_REAPER_INIT)
	{
		CL_SpiralParticles2 (reaper->current.origin, vMins, vMaxs, vColor, 250, PARTICLE_BEAM_SPARKS, 2000);
		tracker->Long1 &= ~TEF_REAPER_INIT;
	}

	CVector vDir(0,0,1);
	CVector vGravity(0,0,-1);
	CVector vAlpha(0.8,0.45,0);
	vColor.Set(0.5,0.1,0.05);
	if (tracker->Long1 & TEF_REAPER_SMOKE)
	{
		CVector org = reaper->current.origin;
		org.z -= 64;
		CL_GenerateComplexParticle2(org,								// emission origin
									vDir,						// some direction thinggy
									vColor,				// color
									vAlpha,				// alpha (start, depletion, rand)
									2.0,								// scale
									2,									// count
									30,									// spread
									90,									// velocity
									PARTICLE_SMOKE,						// particle type
									vGravity);					// gravity
	}
}

// ----------------------------------------------------------------------------------------------------------------------
// sunflare effects!
// ----------------------------------------------------------------------------------------------------------------------

// returns a vector within radius of org
// dir specifies a 'hint' location.  
CVector randomPos(CVector &org,float radius, CVector &dir)
{
	CVector result = org;

	// preserve z because that's the frame number!
	if (dir.Length() > 0.1)
	{
		result.x -= frand() * fabs(org.x - dir.x);
		result.y -= frand() * fabs(org.y - dir.y);
	}
	else
	{
		result.x += crand() * radius;
		result.y += crand() * radius;
	}

	return result;
}

void doFlame(CVector &pos, CVector &org, te_tracker_t *tracker, short updateframe)
{
	// take the z from the org's z
	pos = randomPos(pos,3,zero_vector);	// move it around a bit
	// set up the position.  remember, pos.z is the frame number!!!
	CVector pos2 = pos;
	pos2.z = org.z;

	// if it's not in water, adjust the origin to touch someting
	if (!(tracker->lightSize > 0))
	{
		CVector start = pos2;
		start.z += 50;
		CVector end = pos2;
		end.z -= 100;

		trace_t trace;
		trace = CL_TraceLine(start, end, tracker->src_index, MASK_DEADSOLID);

		pos2.z = trace.endpos.z;
	}

	entity_t ent;
    memset(&ent,0,sizeof(ent));

	float scale = tracker->scale;//*(1 + 0.5*frand());
	
	ent.model = cl.model_draw[tracker->modelindex];
	ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=scale;
	ent.alpha = tracker->scale2;

	CVector vDir(1,0,0);
	VectorToAngles(vDir,ent.angles);
	ent.angles.yaw += 90*frand();
    ent.origin = pos2;
    
    ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
	ent.frame=ent.oldframe=(short)pos.z;

    pos.z += 1.0;
	if (pos.z > tracker->numframes)
		pos.z = 0;

	V_AddEntity(&ent);
	ent.angles.yaw += 90;
	V_AddEntity(&ent);
	V_AddLight(pos2, 150 + 50*frand(), 0.8, 0.4, 0.2);
	// water!
	vDir.Set(0,0,1);
	CVector vColor(0.2,0.2,0.2);
	CVector vAlpha(0.4,0.15,0);
	CVector vGravity(0,0,0);
	if (!(tracker->lightSize > 0))
	{
		if (rand() & 1)
		CL_GenerateComplexParticle2(pos2,								// emission origin
									vDir,						// some direction thinggy
									vColor,				// color
									vAlpha,				// alpha (start, depletion, rand)
									1.75,								// scale
									1,									// count
									40,									// spread
									80,									// velocity
									PARTICLE_SMOKE,						// particle type
									vGravity);					// gravity

		memset(&ent,0,sizeof(ent));
		ent.model = cl.model_draw[tracker->modelindex2];
		ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=1.5*scale;
		ent.alpha = tracker->scale2 + 0.2;
		
		vDir.Set(0,0,1);
		VectorToAngles(vDir,ent.angles);
		ent.origin = pos2;
    
		ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;//|RF_DEPTHHACK;
		ent.frame=ent.oldframe=0;
		V_AddEntity(&ent);
	}
	
}

void TrackEntFX_SunFlareFX(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;

	CVector org = trackData->track->current.origin;
	float length = tracker->length;

	// zero out all of the vectors we use
	if (tracker->Long2 > 0)
	{
		tracker->altpos.Zero();
		tracker->altpos2.Zero();
		tracker->altangle.Zero();
		tracker->dstpos.Zero();
		tracker->ss_ofs.Zero();
		tracker->ss_fwd.Zero();
		tracker->ss_rt.Zero();
		tracker->ss_up.Zero();
		tracker->trailLastVec.Zero();
	}

	short numframes = tracker->numframes;
	CVector org2 = org;
	org2.z = 0;					// actually contains the current frame!
	switch(tracker->Long2)
	{
	case 9:
		org.z = frand() * numframes;
		tracker->trailLastVec.Set(randomPos(org2,length,zero_vector));
	case 8:
		org.z = frand() * numframes;
		tracker->ss_up.Set(randomPos(org2,length,zero_vector));
	case 7:
		org.z = frand() * numframes;
		tracker->ss_rt.Set(randomPos(org2,length,zero_vector));
	case 6:
		org.z = frand() * numframes;
		tracker->ss_fwd.Set(randomPos(org2,length,zero_vector));
	case 5:
		org.z = frand() * numframes;
		tracker->ss_ofs.Set(randomPos(org2,length,zero_vector));
	case 4:
		org.z = frand() * numframes;
		tracker->dstpos.Set(randomPos(org2,length,zero_vector));
	case 3:
		org.z = frand() * numframes;
		tracker->altangle.Set(randomPos(org2,length,zero_vector));
	case 2:
		org.z = frand() * numframes;
		tracker->altpos2.Set(randomPos(org2,length,zero_vector));
	case 1:
		org.z = frand() * numframes;
		tracker->altpos.Set(randomPos(org2,length,zero_vector));

		tracker->Long1 = tracker->Long2;
		tracker->Long2 = 0;
	default:		// move the flames around and stuff
	case 0:
		{
			short updateframes=0;
			// see if we should update the frames
			tracker->curframetime -= cls.frametime;
			if(tracker->curframetime < 0)
			{
				updateframes = 1;
				tracker->curframetime += tracker->frametime;
			}
			switch(tracker->Long1)
			{
			case 9:
				doFlame(tracker->trailLastVec,org,tracker,updateframes);
			case 8:
				doFlame(tracker->ss_up,org,tracker,updateframes);
			case 7:
				doFlame(tracker->ss_rt,org,tracker,updateframes);
			case 6:
				doFlame(tracker->ss_fwd,org,tracker,updateframes);
			case 5:
				doFlame(tracker->ss_ofs,org,tracker,updateframes);
			case 4:
				doFlame(tracker->dstpos,org,tracker,updateframes);
			case 3:
				doFlame(tracker->altangle,org,tracker,updateframes);
			case 2:
				doFlame(tracker->altpos2,org,tracker,updateframes);
			case 1:
				doFlame(tracker->altpos,org,tracker,updateframes);
			default:
			case 0:
				{
					entity_t ent;
					memset(&ent,0,sizeof(ent));

					float scale = 2.5*tracker->scale;
					ent.model = cl.model_draw[tracker->modelindex];
					ent.render_scale.x=ent.render_scale.y=ent.render_scale.z=scale;
					ent.alpha = tracker->scale2;

					VectorToAngles(CVector(1,0,0),ent.angles);
					ent.angles.yaw += 90*frand();
					ent.origin = org;
    
					ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
					ent.frame=ent.oldframe=(short)tracker->altpos.z;

					V_AddEntity(&ent);
					ent.angles.yaw += 90;
					V_AddEntity(&ent);

					if (updateframes)
					{
						tracker->scale -= 0.015;
						if (tracker->scale < 1.0)
						{
							tracker->scale = 1.0;
							tracker->scale2 -= 0.015;
							if (tracker->scale2 < 0.05)
								tracker->fxflags &= ~TEF_SUNFLARE_FX;
						}
					}
				}
				break;
			};
		}
	};
}
// ----------------------------------------------------------------------------------------------------------------------
// daikatana
// ----------------------------------------------------------------------------------------------------------------------

#define DK_MODULATION			0.5
#define DK_MODULATION_DELTA		0.9
#define DK_SCALE				0.8
#define DK_SCALE_DELTA			0.2
// defined in artifact_fx.cpp
void DK_Lightning(CVector &start, CVector &end, CVector color, float colDel, float alpha, float alpDel, float scale, float scDel,
				  float mod, float modDel);
void DK_MovingFlare(CVector &start, CVector &end, float &frac, int modelindex, float alpha, float scale, short update);
void DK_TipFlare(CVector &org, CVector &oldOrg, CVector &color, int modelindex, float alpha, float scale);
void DK_BaseFlare(CVector &org, CVector &oldOrg, CVector &ang, int modelindex, float alpha, float scale);

void DK_MovingFlareSnd(CVector &pos)
{
	S_StartSound(pos, 0, 0, cl_sfx_dk,0.05,100,300);
}

void TrackEntFX_Daikatana(trackParamData_t *trackData)
{
	// see if this sword became detached from its holder
	qboolean detached = FALSE;
	int modelindex2 = (trackData->owner->current.modelindex2 & 0xffff0000) >> 16;
	if (modelindex2 != trackData->extraent->current.number)
		detached = TRUE;

	if (trackData->extraent->current.modelindex2 != trackData->owner->current.number + 10000)
		detached = TRUE;

    te_tracker_t *tracker = trackData->tracker;

	if (detached)
	{
		tracker->freeMe = TRUE;
		return;
	}

	float alpha = tracker->scale;
	float scale = tracker->scale2;

	if (!trackData->thirdPersonView && ((trackData->extraent->current.renderfx & RF_NODRAW) || (weapon_visible->value == 0)))
		return;
	// set up our frame timer
	short frameUpdate = 0;
	tracker->curframetime -= cls.frametime;
	if(tracker->curframetime < 0)
	{
		frameUpdate = 1;
		tracker->curframetime += 0.05;
	}

	entity_t *sword = trackData->ent;

	// find the gun point if we're doing third person and set up the sword
	short thirdp = 0;
	if (trackData->thirdPersonView)
	{
		CVector hp;
		CL_GetPlayerHandPoint(trackData,"hp_gun",hp);
		trackData->thirdPersonView = 0;
		thirdp = 1;

		// set up the sword, do default actions to draw the model
		trackData->extraent->current.modelindex = tracker->Long2;

		CVector swhardPt;
		entity_t entTemp;
		memset(&entTemp,0,sizeof(entTemp));

        entTemp.origin = zero_vector;// get offset from origintrackData->track->lerp_origin;
		LerpAngles(trackData->track->prev.angles,trackData->track->current.angles,entTemp.angles,1.0-trackData->extraent->fFrameLerp);
		entTemp.render_scale = trackData->track->current.render_scale;
        entTemp.model = cl.model_draw[tracker->Long2];
		entTemp.frame = trackData->extraent->current.frame;
		entTemp.oldframe = trackData->extraent->prev.frame;
		entTemp.backlerp = 1.0-trackData->extraent->fFrameLerp;

		swhardPt.Set(0.0,0.0,0.0);
		re.GetModelHardpoint("hp_hand", entTemp.oldframe, entTemp.frame,  entTemp, swhardPt);  

		sword->origin = hp - swhardPt;
		sword->oldorigin = hp - swhardPt;

	}

	trackData->track = trackData->extraent;

	// so only the first person dk bobs.
	float dobob = cl_bob->value;
	if (thirdp)
		cl_bob->value = 0;

	tracker->renderfx |= RF_WEAPONMODEL;
	TrackEntRFX_Default(trackData);
	sword->flags &= ~RF_DEPTHHACK;

	cl_bob->value = dobob;

	if (!thirdp)
		sword->flags |= ( RF_WEAPONMODEL | RF_ENVMAP );
	// find the hard points and fudge things to fix the inaccurate hard points.
	trackData->origin = sword->origin;//trackData->track->lerp_origin;
	CVector hpHandle,hpTip,dir,angle;
	CL_GetWeaponMuzzlePoint(trackData,"hr_muzzle1",hpHandle);
	CL_GetWeaponMuzzlePoint(trackData,"hr_muzzle2",hpTip);
	dir = hpTip - hpHandle;
	dir.Normalize();
	VectorToAngles(dir,angle);

	if (thirdp)
	{
		sword->origin += dir;
		// move the hp to the right place
		hpHandle -= 1.2*dir;//2.2*dir;
		hpTip += 3.2*dir;//2.2*dir;
	}
	else
	{
		// move the hp to the right place
		hpHandle -= 2.2*dir;
		hpTip += 2.2*dir;
	}

	if (sword->render_scale.Length() < 1)
		sword->render_scale.Set(1,1,1);

	// Actually draw the sword
	if (thirdp)
		V_AddEntity(sword);

	// find the origination points for the lightning
	CVector h1,h2,h3,h4;
	angle.roll = 89.9*frand();
	AngleToVectors(angle,forward,right,h1);
	angle.roll += 90;
	AngleToVectors(angle,forward,right,h2);
	angle.roll += 90;
	AngleToVectors(angle,forward,right,h3);
	angle.roll += 90;
	AngleToVectors(angle,forward,right,h4);

	h1 = hpHandle + frand() * h1;
	h2 = hpHandle + frand() * h2;
	h3 = hpHandle + frand() * h3;
	h4 = hpHandle + frand() * h4;

	// do the power level stuff!

	// alpha levels...we need to customize these because of the wraithorb!
	float lAlpha, ldAlpha,tAlpha,mAlpha,bAlpha;
	if ((sword->flags & (RF_PREDATOR|RF_TRANSLUCENT)) == (RF_PREDATOR|RF_TRANSLUCENT))
	{
		float swAlpha = sword->alpha;
		lAlpha = 0;
		ldAlpha = swAlpha;
		tAlpha = 0.6*swAlpha;
		bAlpha = alpha * swAlpha;
		mAlpha = 0.2*swAlpha;
	}
	else
	{
		lAlpha = 0.1;
		ldAlpha = 0.25;
		tAlpha = 0.6;
		bAlpha = alpha;
		mAlpha = 0.6;
	}

	switch(tracker->numframes) // numframes is the power level
	{
	case 5:
		// negative light technology
		V_AddLight(hpHandle,100+20*frand(),tracker->altpos2.x,tracker->altpos2.y,tracker->altpos2.z);
		V_AddLight(hpTip,100+20*frand(),tracker->altpos2.x,tracker->altpos2.y,tracker->altpos2.z);
		lAlpha += ldAlpha;
	case 4:
		lAlpha += ldAlpha;
		// start,end,color,colordel,alpha,alphadel,scale,scaledel,mod, moddel
		DK_Lightning(h1,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
		DK_Lightning(h2,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
		DK_Lightning(h3,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
		DK_Lightning(h4,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
		DK_TipFlare(hpTip,tracker->ss_fwd,tracker->lightColor,tracker->modelindex,tAlpha,0.075+0.075*frand());				// ss_fwd holds old tip origin
	case 3:
		DK_MovingFlare(hpHandle,hpTip,tracker->length,tracker->modelindex,mAlpha,0.05+0.05*frand(), frameUpdate);
	case 2:
		DK_BaseFlare(hpHandle,tracker->ss_rt,angle,tracker->modelindex,bAlpha,scale);		// ss_rt holds old handle origin
	default:
		break;
	}

	// do special fx
	CVector vMins(-30,-30,-50);
	CVector vMaxs(30,30,50);
	CVector vColor(1,1,1);

	if (frameUpdate)
	{
		CVector pos;
		if (tracker->Long1 & TEF_DK_LEVEL1)			// foe vanquish fx ------------------------------------------------------------
		{
			pos = tracker->altpos;
			if (tracker->nextsunsmoke < 1)
				tracker->nextsunsmoke = 20;

			if (tracker->nextsunsmoke >= 18)
			{
				CL_SpiralParticles2(pos,vMins,vMaxs,vColor,200,PARTICLE_SPARKLE1,1500);
			}
			else if (tracker->nextsunsmoke <= 1)
			{
				tracker->nextsunsmoke = 0;
				tracker->Long1 &= ~TEF_DK_LEVEL1;
			}
			else if (tracker->nextsunsmoke <= 15)
			{
				DK_Lightning(hpTip,pos,vColor,0.3,0.6,0.2,3,0.4,1,0.3);
				DK_Lightning(hpTip,pos,vColor,0.3,0.6,0.2,3,0.4,1,0.3);
			}

			tracker->nextsunsmoke -= 1;
		}

		CVector vDir(0,0,1);
		CVector vAlpha(0.4,0.2,0);
		CVector vColor(0.4,0.4,0.7);
		CVector vGravity(0,0,0);

		if (tracker->Long1 & TEF_DK_LEVEL2)			// level up fx ---------------------------------------------------------------
		{
			// keep track of things..
			if (tracker->trailNextPoint < 1)
				tracker->trailNextPoint = 70;
			
			tracker->trailNextPoint -= 1;
			if (tracker->trailNextPoint <= 1)
			{
				tracker->trailNextPoint = 0;
				tracker->Long1 &= ~TEF_DK_LEVEL2;
				tracker->ss_up.Set(0,0,0);
			}

			// set up our origin (floor)
			pos = trackData->owner->current.origin;
			pos.z += trackData->owner->current.mins.z + 1;
			CL_GenerateComplexParticleRadius(	pos,
												vDir,
												vColor,
												vAlpha,
												0.75,
												15,
												80,
												60,
												PARTICLE_SMOKE,
												vGravity,
												20 * frand());
			// draw the floor sprite
			entity_t ent;
			memset(&ent,0,sizeof(ent));
			ent.model = cl.model_draw[tracker->modelindex];
			ent.origin = pos;
			ent.oldorigin = pos;
			ent.render_scale.Set(3,3,3);
			ent.alpha = 0.8;
			ent.flags=SPR_ALPHACHANNEL;
			ent.flags |= SPR_ORIENTED;
			CVector ang;
			VectorToAngles(vDir,ang);
			ang.yaw = 360*frand();
			ent.angles = ang;
			V_AddEntity(&ent);

			pos = trackData->owner->current.origin;
			V_AddLight(pos,150+50*frand(),-1,-1,1+frand());
			pos.z += trackData->owner->current.maxs.z + 20;
			vAlpha.Set(0.8,0.45,0);
			vGravity.Set(0,0,-20);
			CL_GenerateComplexParticle2(pos,								// emission origin
										vDir,					// some direction thinggy
										vColor,				// color
										vAlpha,				// alpha (start, depletion, rand)
										0.5,								// scale
										20,									// count
										180,								// spread
										100,									// velocity
										PARTICLE_SPARKLE2,					// particle type
										vGravity);					// gravity

		}
	}
}

// ----------------------------------------------------------------------------------------------------------------------
// projectile tracker
// ----------------------------------------------------------------------------------------------------------------------
void TrackEntFX_Proj_PolyExplosion(CVector &org, CVector &ang, float scale, float lt_size, CVector &lt_color)
{
	AngleToVectors(ang,forward,right,up);
	org.x += forward.x * 4;
	org.y += forward.y * 4;
	org.z += forward.z * 4;
    // outer ring is a sprite
    CL_PolyExplosion(org, ang, cl_mod_polyexplode2, scale *0.1, 4 * scale, 0.2f, lt_size, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL);

    // center is a model -- model is oriented 90 degrees off
//    ang.y += 90;
	CL_PolyExplosion(org, up, cl_mod_polyexplode, scale * 0.1f, 8 * scale, 0.3f, lt_size, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL);
}

void TrackEntFX_AnimExplosion(trackParamData_t *trackData, int pointContents, float scale,float alpha, void* model, short frames, float msdelay)
{
	explosion_t *ex;
	ex = CL_AllocExplosion ();
	ex->type = ex_explosion;

	ex->ent.origin = trackData->track->current.origin;
	ex->ent.angles = trackData->tracker->altpos;
	ex->ent.render_scale.Set(scale,scale,scale);
	ex->ent.alpha = alpha;
	ex->ent.model = model;

	ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT | SPR_ORIENTED;
	ex->start = msdelay + cl.frame.servertime - CL_FRAME_MILLISECONDS;
	ex->frames = frames;
}

// called when the projectile is dead
void TrackEntFX_Proj_DieFX(trackParamData_t *trackData, int pointContents)
{
	projectile_fx_func die = GetProjectile_Fx(TEF_INDEX_OF(trackData->tracker->numframes)).die;
	if (die)
		die(trackData,pointContents);

	trackData->tracker->fxflags &= ~TEF_PROJ_FX;
}

// apply special effects to the launch of the projectile
void TrackEntFX_Proj_LaunchFX(trackParamData_t *trackData, int pointContents)
{
    te_tracker_t *tracker = trackData->tracker;
	if (!(tracker->numframes & TEF_PROJ_LAUNCH))
		return;

	// get the launch func and run it!
	projectile_fx_func launch = GetProjectile_Fx(TEF_INDEX_OF(trackData->tracker->numframes)).launch;
	if (launch)
		launch(trackData,pointContents);

	tracker->numframes &= ~TEF_PROJ_LAUNCH;
}

// apply other special effects to the projectile
void TrackEntFX_Proj_SpecialFX(trackParamData_t *trackData, int pointContents)
{
    te_tracker_t *tracker = trackData->tracker;
	if (!(tracker->numframes & TEF_PROJ_SPECIAL))
		return;

	// get the special func and run it!
	projectile_fx_func special = GetProjectile_Fx(TEF_INDEX_OF(trackData->tracker->numframes)).special;
	if (special)
		special(trackData,pointContents);

	tracker->numframes &= ~TEF_PROJ_SPECIAL;
}

void TrackEntFX_Proj_FlyFX(trackParamData_t *trackData, int pointContents)
{
    te_tracker_t *tracker = trackData->tracker;
//	centity_t *track = trackData->track;
	if (tracker->numframes & TEF_PROJ_DIE)
		return;

	// get the fly func and run it!
	projectile_fx_func fly = GetProjectile_Fx(TEF_INDEX_OF(trackData->tracker->numframes)).fly;
	if (fly)
		fly(trackData,pointContents);

	// add any necessary launch effects
	if (tracker->numframes & TEF_PROJ_LAUNCH)				
		TrackEntFX_Proj_LaunchFX(trackData, pointContents);

	// add special effects
	if (tracker->numframes & TEF_PROJ_SPECIAL)
		TrackEntFX_Proj_SpecialFX(trackData, pointContents);
}

// ----------------------------------------------------------------------------
// Creator:<cek>
//
// Name:			TrackEntFX_Proj
// Description:		Handles tracking of a generic 'projectile', allowing
//					effects to be applied to launch, flight and death;
//				
// Input:			numframes:	nonzero to do an impact surface sprite
//					altangle:	normal of hit surface
//					modelindex:	plane num hit
//
// Note:		
// ----------------------------------------------------------------------------

void TrackEntFX_Proj(trackParamData_t *trackData)
{
    te_tracker_t *tracker = trackData->tracker;

	if (!(trackData->track->current.effects2 & EF2_TRACKED_PROJ) || (tracker->numframes == 0))
	{
		tracker->freeMe = TRUE;
		return;
	}
	// update the water flag!
	if ((cl.time % 3 == 0) || (tracker->numframes & TEF_PROJ_LAUNCH))
	{
		tracker->Long1 = CM_PointContents (trackData->track->lerp_origin, 0);
	}

	// if we send the force water flag
	if (tracker->numframes & TEF_PROJ_WATER)
		tracker->Long1 |= MASK_WATER;

	if (tracker->numframes & TEF_PROJ_DIE)										// we're ice now!  shrink!!!!
	{
		TrackEntFX_Proj_DieFX(trackData, tracker->Long1);
		tracker->fxflags &= ~TEF_PROJ_FX;
		tracker->numframes = 0;
	}
	else
	{
		// check some conditions that will require this projectile to be untracked
		CVector rs = trackData->track->current.render_scale;
		if (!(tracker->numframes & TEF_PROJ_FLY) || !(tracker->fxflags & TEF_PROJ_FX) || ((tracker->scale < 0.01) && ( (rs.x < 0.01) || (rs.x < 0.01) || (rs.x < 0.01))))
		{
			tracker->numframes &= ~(TEF_PROJ_FX);
			return;
		}

		// apply the fly effects first so we can mess with the origin if we want.  :)
		if ((tracker->numframes & TEF_PROJ_FLY) && !((tracker->numframes & TEF_PROJ_LAUNCH) && trackData->thirdPersonView))
			TrackEntFX_Proj_FlyFX(trackData, tracker->Long1);				// add some flight effects

		// draw the projectile
		entity_t ent;
		memset(&ent,0,sizeof(ent));

		ent.model = cl.model_draw[trackData->track->current.modelindex];
		ent.angles = trackData->track->current.angles;

		// use scale if set, otherwise use the entity's render_scale
		if (tracker->scale > 0)
			ent.render_scale.Set(tracker->scale,tracker->scale,tracker->scale);
		else 
			ent.render_scale = trackData->track->current.render_scale;
		
		// check our alpha
		if (tracker->scale2 < 0.01)
			ent.alpha = 1.0f;
		else
			ent.alpha = tracker->scale2;

		ent.flags=SPR_ALPHACHANNEL|tracker->renderfx;

		ent.flags &= ~RF_DEPTHHACK;

		ent.frame=ent.oldframe=trackData->ent->frame;
		CVector org = trackData->track->lerp_origin;
//		CVector oldLerp = org;
//		CVector dir = tracker->altpos;

		// make the particle come from the end of the barrel and adjust current and
		// previous origins so that prediction won't mess with the position
		if ((tracker->numframes & TEF_PROJ_LAUNCH) && trackData->thirdPersonView)		
		{
			CL_GetWeaponMuzzlePoint(trackData, "hr_muzzle", org);
			ent.origin = org;
			trackData->track->current.origin = org;
			trackData->track->current.old_origin = org;
			trackData->track->prev.origin = org;
			trackData->track->prev.old_origin = org;
			tracker->numframes &= ~TEF_PROJ_LAUNCH;
		}
		else
		{
			ent.origin = org;
		}	
		V_AddEntity(&ent);
	}
}

// ----------------------------------------------------------------------------------------------------------------------
// End of projectile tracker
// ----------------------------------------------------------------------------------------------------------------------
CVector CL_GetBobOffset();
void TrackEntFX_Fire(trackParamData_t *trackData)
{
	if (!(trackData->track->current.effects2 & EF2_SUNFLARE_FLAME) || !(weapon_visible->value) || (cl.frame.playerstate.rdflags & RDF_LETTERBOX))
		return;

    CVector         vecForward, vecRight, vecUp, vecOrigin, vecHardPoint;
    entity_t        entModel, entTemp;
    te_tracker_t    *tracker;

    memset(&entModel, 0, sizeof(entity_t));
	memset(&entTemp,0, sizeof(entity_t));
    tracker = trackData->tracker;

	if (trackData->thirdPersonView)
	{
		CVector start;
		CL_GetWeaponMuzzlePoint(trackData, "hr_muzzle", start);
		start.z += 3.5;
		entModel.origin = start;
		entModel.model = cl.model_draw[tracker->modelindex];
		entModel.angles = trackData->angles;
		entModel.oldorigin = trackData->track->current.old_origin;
		entModel.frame = ( int ) ( cl.refdef.time * 10 ) % 7;

		//entModel.render_scale.x=entModel.render_scale.y=entModel.render_scale.z=0.15f;
		entModel.render_scale.Set(0.4, 0.2, 0.4);
		entModel.alpha = 1;
//		entModel.flags = RF_DEPTHHACK;
		tracker->nextsunsmoke -= cls.frametime;
		if(tracker->nextsunsmoke <= 0)
		{
			CL_SunflareSmoke(start, 0.9f, 1.0f, 1, 8);
			tracker->nextsunsmoke = 0.25f;
		}
		V_AddEntity(&entModel);
		V_AddLight( start, 160 + (frand() * 10), 0.7, frand() * 0.2 + 0.2, 0.0 );
		return;
	}
	
	// original first person stuff!
    AngleToVectors(trackData->angles, vecForward, vecRight, vecUp);
    entModel.model = cl.model_draw[tracker->modelindex];
    entModel.angles = trackData->angles;
    entModel.oldorigin = trackData->track->current.old_origin;
	entModel.frame = ( int ) ( cl.refdef.time * 10 ) % 7;

    //entModel.render_scale.x=entModel.render_scale.y=entModel.render_scale.z=0.15f;
    entModel.render_scale.Set(0.10, 0.06, 0.1);
    entModel.alpha = 1;
    entModel.flags = RF_DEPTHHACK;

    // get model attitude, find correct origin poly
    entTemp.origin = trackData->origin;
    entTemp.oldorigin = trackData->origin;
    entTemp.angles = trackData->angles;
    entTemp.model = cl.model_draw[trackData->track->current.modelindex];
    entTemp.frame = entTemp.oldframe = tracker->curframe;
	entTemp.backlerp = 1.0-trackData->track->fFrameLerp;

	vecHardPoint.Set(0,0,0);
    re.GetModelHardpoint("fire", trackData->track->current.frame, trackData->track->current.frame, entTemp, vecHardPoint);
    entModel.origin = vecHardPoint;
    vecOrigin = vecHardPoint;
	V_AddLight( vecOrigin, 160 + (frand() * 10), 0.7, frand() * 0.2 + 0.2, 0.0 );


    tracker->nextsunsmoke -= cls.frametime;
    if(tracker->nextsunsmoke <= 0)
    {
        VectorMA(vecOrigin, vecForward, -3.5, vecOrigin);
        VectorMA(vecOrigin, vecRight, -2.3, vecOrigin);
        VectorMA(vecOrigin, vecUp, 2.5, vecOrigin);
        CL_SunflareSmoke(vecOrigin, 0.2f, 1.0f, 1, 2.2f);
//        if(trackData->fRandom > 0.9f)
//            CL_SunflareSparks(vecOrigin, 0.2f, 5.0f, 0, 18.0f);
        tracker->nextsunsmoke = 0.25f;
        vecOrigin = vecHardPoint;
    }
    
    VectorMA(vecOrigin, vecForward, -0.1f, vecOrigin);
    VectorMA(vecOrigin, vecRight, 0.6f, vecOrigin);
    VectorMA(vecOrigin, vecUp, 0.9f, vecOrigin);
    entModel.origin = vecOrigin;
    entModel.origin += CL_GetBobOffset();//vecBobOffset;
    V_AddEntity(&entModel);
}

void TrackEntFX_FlashLight(trackParamData_t *trackData)
{
#ifndef FLASHLIGHT_MAX_RAYS
    #define FLASHLIGHT_MAX_RAYS 5
#endif
    float   fAngleX[FLASHLIGHT_MAX_RAYS] = {0.0f, -4.0f, 4.0f, 0.0f, 0.0f};
    float   fAngleY[FLASHLIGHT_MAX_RAYS] = {0.0f, 0.0f, 0.0f, -4.0f, 4.0f};
    CVector vecStartPosition[FLASHLIGHT_MAX_RAYS], vecEndPosition[FLASHLIGHT_MAX_RAYS];
    int     iEndPositionFlags[FLASHLIGHT_MAX_RAYS], iLoop;
    CVector vecStart, vecAngle, vecFwd, vecEnd;
    float   fLen, fSize, fPercent;
    trace_t trace;
    
    vecFwd = trackData->forward;
    AngleToVectors(trackData->angles, trackData->forward);

    for(iLoop = 0; iLoop < FLASHLIGHT_MAX_RAYS; iLoop++)
    {
        vecAngle.x = trackData->angles.x + fAngleX[iLoop];
        if(vecAngle.x >= 360)
            vecAngle.x -= 360;
        if(vecAngle.x < 0)
            vecAngle.x += 360;
        vecAngle.y = trackData->angles.y + fAngleY[iLoop];
        if(vecAngle.y >= 360)
            vecAngle.y -= 360;
        if(vecAngle.y < 0)
            vecAngle.y += 360;
        vecAngle.z = trackData->angles.z;
        AngleToVectors(vecAngle, trackData->forward);
        
        VectorMA(trackData->ent->origin, trackData->forward, 0, vecStart);
        VectorMA(vecStart, trackData->forward, 2000, vecEnd);
        trace = CL_TraceLine(vecStart, vecEnd, trackData->tracker->src_index, MASK_SHOT);

        vecStartPosition[iLoop] = trackData->ent->origin;
        vecEndPosition[iLoop] = trace.endpos;
        iEndPositionFlags[iLoop]=trace.surface->flags;
    }

    for(iLoop = 0; iLoop < FLASHLIGHT_MAX_RAYS; iLoop++)
    {
        if( !(iEndPositionFlags[iLoop] & SURF_SKY) )
        {
            vecStart = vecEndPosition[iLoop] - vecStartPosition[iLoop];
            fLen = vecStart.Length();
            trackData->forward = vecFwd * fLen;
            vecStart = trackData->ent->origin + trackData->forward;

            fSize = (75.0f+fLen * 0.15f) * trackData->tracker->lightSize;
            if( fSize < 75 )
                fSize = 75;
            fPercent = (1e-6 * fLen * fLen);
            if(fPercent)
                fPercent = 1/fPercent/5;
            if(fPercent > 1.0f)
                fPercent = 1.0f;
            V_AddLight(vecEndPosition[iLoop], fSize, trackData->tracker->lightColor[0]*fPercent, trackData->tracker->lightColor[1]*fPercent, trackData->tracker->lightColor[2]*fPercent);
        }
    }
}

void TrackEntRFX_Shake(trackParamData_t *trackData)
{
    trackData->ent->origin+=trackData->shake;
}

void TrackEntRFX_NovaLaser(trackParamData_t *trackData)
{
    CVector vecAltOrigin, vecAltForward, vecEnd, vecPosition, rgbColor;
    entity_t        *ent, entTemp;
    te_tracker_t    *tracker;
//    centity_t       *track;
    trace_t         trace;
	float fOffset;

    // expedite dereferencing
    tracker = trackData->tracker;
//    track = trackData->track;
	ent = trackData->ent;

	CVector start(0.0,0.0,0.0);
	CVector end = tracker->dstpos;

	memset(&entTemp,0,sizeof(entTemp));

	// figure out where the beam goes
	CL_GetWeaponMuzzlePoint(trackData, "hr_muzzle", start);
	CVector fwd = end-start;
	fwd.Normalize();
	CVector vTraceEnd;
	vTraceEnd = end + ( end - start );
    trace = CL_TraceLine(start-2*fwd, vTraceEnd, tracker->src_index, MASK_DEADSOLID);
	end = trace.endpos;

    VectorToAngles(trace.plane.normal, trackData->angles);
	//trace.ent == (struct edict_s *)0x00000001 || !trace.ent || trace.startsolid
    if(trace.ent && (trace.ent == (struct edict_s *)0x00000001) && !(trace.surface->flags & SURF_SKY))//(trace.surface && !(trace.surface->flags & SURF_SKY) && (trace.surface->name[0] != NULL))
	{
        fOffset = crand()>0.5?crand():-crand(); 
        vecPosition.x = tracker->dstpos.x + fOffset*4;
        fOffset = crand()>0.5?crand():-crand(); 
        vecPosition.y = tracker->dstpos.y + fOffset*4;
        fOffset = crand()>0.5?crand():-crand(); 
        vecPosition.z = tracker->dstpos.z + fOffset*4;
        CL_NovaEffect(end, trackData->o_forward, trace.plane.normal);
		re.AddSurfaceSprite( end, end, trace.plane.normal, cl.model_draw[1], trace.plane.planeIndex, cl_mod_novahit, 0, 0, 1, SSF_FULLBRIGHT|SSF_QUICKFADE);
    }

	float radius = 4;//(ent->frame & 0xff);
	// put in a sprite at the end of the laser to give it 'diameter' when looking at it straight on
	CL_AddLaserEnd( end, fwd, (int)tracker->scale2, 0.075*radius, 1.0,trackData->thirdPersonView);

	// put in some lights
	V_AddLight(start, 200.0f, 0.8f, 0.4f, 0.2f);
    V_AddLight(end, 150.0f, 0.8f, 0.4f, 0.2f);

	// draw the beam
	rgbColor.y = rgbColor.z = 1.0;rgbColor.x = ent->skinnum;
	beam_AddNovaLaser (start, end, rgbColor, ent->alpha, radius, BEAM_TEX_NOVALASER, 0);
}

void TrackEntRFX_SpotLight(trackParamData_t *trackData)
{
    entity_t    *ent;

    ent = trackData->ent;

    ent->frame = trackData->track->current.frame;
    ent->skinnum = (trackData->track->current.skinnum >> ((rand() & 3)*8)) & 0xff;  
    ent->alpha = 0.8f;
    ent->model = NULL;
    trackData->fwd.Multiply(trackData->tracker->length);
    ent->oldorigin = ent->origin + ( trackData->fwd * trackData->tracker->length);
}

void TrackEntRFX_Lightning(trackParamData_t *trackData)
{
	CVector Alpha,Color,Dir,org;

    entity_t    *ent;

    ent = trackData->ent;

    ent->alpha = trackData->track->current.alpha;
    ent->flags |= trackData->tracker->renderfx;
    ent->flags |= RF_LIGHTNING;
    ent->frame = trackData->track->current.frame;
    ent->skinnum = (trackData->track->current.skinnum >> ((rand()& 3)*8)) & 0xff;
    ent->model = NULL;
	CVector rgbColor;
	rgbColor	= trackData->tracker->altangle;
	CVector Gravity;

    if(trackData->tracker->flags & TEF_DSTPOS)
    {
        ent->oldorigin = trackData->tracker->dstpos;
    }
    if( !(trackData->tracker->fxflags & TEF_NOPARTICLES) )
    {
		//New Sparkle function for Lightning
		Dir.Set(crand(),crand(),crand());//Make it go in random directions
		Alpha.Set(0.85,2.0,0.55);//Set the initial alpha, the depletion rate, and the random addition to depletion
		Gravity.Set(0,0,crand());
		Gravity.Multiply( 400 );
		//The sparkle function was randomly assigning one or the other sparkle, this is how it should be done.
		if(frand() > 0.50f)
		{
			//void CL_GenerateComplexParticle2( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity)
			CL_GenerateComplexParticle2(ent->oldorigin , Dir, rgbColor, Alpha, 2.0f+crand(), 2, 360, 200+(200*frand()),PARTICLE_SPARKLE1,Gravity);//Make Da' Sparks
		}
		else
		{
			CL_GenerateComplexParticle2(ent->oldorigin , Dir, rgbColor, Alpha, 2.0f+crand(), 2, 360, 200+(200*frand()),PARTICLE_SPARKLE2,Gravity);//Make Da' Sparks
		}
	}
    
	if (trackData->tracker->lightSize > 0)
	{
		te_tracker_t    *tracker = trackData->tracker;
		V_AddLight(ent->origin, tracker->lightSize, tracker->lightColor.x, tracker->lightColor.y, tracker->lightColor.z);
	}
	float flags = (trackData->tracker->flags & TEF_NOPARTICLES) ? 0 : BEAMFLAG_ALPHAFADE;
	
	if (trackData->tracker->modelindex)
	{
		entity_t entTemp;
		memset(&entTemp,0,sizeof(entity_t));
		entTemp.model = cl.model_draw[trackData->tracker->modelindex];
		entTemp.render_scale.x=entTemp.render_scale.y=entTemp.render_scale.z=trackData->tracker->scale;
		entTemp.alpha = trackData->tracker->scale2;
		entTemp.angles = trackData->angles;
		entTemp.origin = trackData->owner->lerp_origin;
    
		entTemp.flags=SPR_ALPHACHANNEL;
		V_AddEntity(&entTemp);
	}

	beam_AddLightning(ent->origin, ent->oldorigin, rgbColor, 0.75, trackData->tracker->altpos2.x+(frand()*trackData->tracker->altpos2.x), BEAM_TEX_LIGHTNING, flags,trackData->tracker->altpos2.y);
}
float       fBobTime;
float       fGasTime;   // fucking hack

void TrackEntRFX_Default(trackParamData_t *trackData)
{
    entity_t    *ent;
    centity_t   *track;
    float       fBobMove, fXYSpeed, fDistX, fDistY, fBobRadius, fBobRate, fMotorCycle;
    CVector     vecVel, vecDir, vecForward, vecRight, vecUp, vecSmokeOrigin;
    char        cSmoke;
    
    ent = trackData->ent;
    track = trackData->track;

    ent->model = cl.model_draw[track->current.modelindex];

    if(track->current.modelindex != track->prev.modelindex)
    {
        ent->backlerp = 0;
    } 
	else 
	{
        ent->backlerp = track->current.mins.x;  // hack: backlerp placed here by AddPacketEntities()
    }

    ent->frame = track->current.mins.z;         // hack: backframe placed here by AddPacketEntities()
    ent->oldframe = track->current.mins.y;      // hack: backframe placed here by AddPacketEntities()
    ent->oldorigin = ent->origin;
    
    if (!cl_bob->value || (cl.frame.playerstate.pmove.pm_type == PM_FREEZE))
        return;
    
    // Logic[5/5/99]: Programatic weapon bob
    if(trackData->tracker->renderfx & RF_WEAPONMODEL && !trackData->thirdPersonView)
    {
        if(!(trackData->tracker->renderfx & RF_SHAKE))
        {
            cSmoke = 0;
            fGasTime = 0.0f;
            fBobRadius = 0.35;
            fBobRate = 0.004;
            vecVel.Set(cl.frame.playerstate.pmove.velocity[0], cl.frame.playerstate.pmove.velocity[1], 0);
            fXYSpeed = vecVel.Length();
            if(fXYSpeed > 25 && cl.frame.playerstate.pmove.pm_flags & PMF_ON_GROUND)// && I AM A GROUND ENTITY)
            {
                if(fXYSpeed > 3000)
                    fXYSpeed = 3000;
                fBobMove = fXYSpeed * fBobRate;//.003;
                fBobTime += fBobMove * cls.frametime;
                if(fBobTime > M_PI2)
                    fBobTime -= M_PI2;
            } else {
                fBobMove = .125;
                if(fBobTime >= M_PI)
                    fBobTime -= fBobMove;
                else
                    fBobTime += fBobMove;
                if(fBobTime < 0)
                    fBobTime = 0;
            }
        } else {
            // short, fast cycle
            fGasTime+=cls.frametime;
            fMotorCycle = frand();
            fBobRadius = 0.15 + (fMotorCycle * 0.1);
            fBobRate = fMotorCycle * 0.2;
            vecVel.Set(cl.frame.playerstate.pmove.velocity[0], cl.frame.playerstate.pmove.velocity[1], 0);
            fXYSpeed = 250;
            fBobMove = fXYSpeed * fBobRate;
            fBobTime += fBobMove * cls.frametime;
            if(fBobTime > M_PI2)
                fBobTime -= M_PI2;
            cSmoke = 1;
            
        }

    
        fDistX = cos(fBobTime)*(fBobRadius);
        fDistY = fabs(fDistX);
        vecBobOffset = cl.refdef.viewangles;
        vecBobOffset.AngleToVectors(vecForward, vecRight, vecUp);
        vecBobOffset = vecRight * fDistX;
        VectorMA(vecBobOffset, -vecUp, fDistY, vecBobOffset);
        ent->origin += vecBobOffset;
        track->lerp_origin = ent->origin;   
/*        if(cSmoke)
        {
            if(frand()>0.7 && fGasTime > 1.3f)
            {
                vecSmokeOrigin = ent->origin;
                vecSmokeOrigin += vecForward * 4.5;
                vecSmokeOrigin += vecRight * 2;
                vecSmokeOrigin += vecUp * -1;
				if (!(CM_PointContents(vecSmokeOrigin,0) & CONTENTS_WATER))
				{
					vecVel = trackData->owner->current.origin - trackData->owner->prev.origin;
					vecVel.Normalize();
					vecVel = vecVel * 10;
					CL_GasHandsSmoke (vecSmokeOrigin, vecVel);
				}
            }
        }*/
    }
    // end weapon bob
}

///////////////////////////////////////////////////////////////////////
// Track Entity function dispatch structure
typedef struct 
{
    void    (*TrackEntDispatchFunction)(trackParamData_t *trackData);
} trackEntDispatchFunction_t;

trackEntDispatchFunction_t  TEDispatch_flags[32];
trackEntDispatchFunction_t  TEDispatch_fxflags[32];
trackEntDispatchFunction_t  TEDispatch_renderfx[32];


//static byte splash_color[] = {0x00, 0xe0, 0xb0, 0x50, 0xd0, 0xe0, 0xe8};

void TempEvent_BlueSparks(void)
{
    CVector pos;
    
#ifdef _DEBUG
	SHOWNET("TempEvent_BlueSparks");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
	CVector vDir(0,0,0);
	CVector vColor(0.0f, 0.0f, 1.0f);
    CL_ParticleEffect_Complex (pos, vDir, vColor, 16, PARTICLE_BLUE_SPARKS);
}

void TempEvent_ShieldSparks(void)
{
    CVector pos;
    CVector dir;
#ifdef _DEBUG
	SHOWNET("TempEvent_ShieldSparks");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    CL_ParticleEffect (pos, dir, 0xb0, 40);
}


void TempEvent_LaserSparks(void)
{
    int cnt,blah;
    CVector pos,dir,alpha,color,gravity; 

    cnt = MSG_ReadByte (&net_message);
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
	blah = MSG_ReadByte (&net_message);//here just because... blah
	
	
	color.Set(0.85,0.85,0.35);
	gravity.Set(0,0,-15.0);
	if(cnt > 20)
		cnt = 20;
	int cnt1 = cnt * 0.50;
	int cnt2 = cnt * 0.75;
	if(cnt1 > 10)
		cnt1 = 10;
	if(cnt2 > 10)
		cnt2 = 10;
	if(cnt1 <= 0)
		cnt1 = 1;
	if(cnt2 <= 0)
		cnt2 = 1;
	//Generate the beams
	alpha.Set(0.85,0.50+(frand()*0.50),0);
	CL_GenerateComplexParticle2( pos, dir, color, alpha, 0.15, cnt1, 5, 75.0f+(frand()*35),PARTICLE_BEAM_SPARKS,gravity);
	//Generate the fire looking stuff...
	gravity.Set(crand()*35,crand()*35,-35.0);
	alpha.Set(0.85,0.50+(frand()*0.50),0);
	color.Set(0.65,0.35,0.05);
	CL_GenerateComplexParticle2( pos, dir, color, alpha, 1.0, cnt2, 10, 50.0f+(frand()*35),PARTICLE_SIMPLE,gravity);
	//10-19-1999 <nss> Sparks looked dumb... making changes. Oh wow... doesn't even use the fucking color... nice.
	//  CL_ParticleEffect_Complex (pos, dir, CVector(1.0f, 1.0f, 0.5f), cnt, PARTICLE_SPARKS);

}

void TempEvent_BolterSparks(void)
{
    int cnt; 
    CVector color;
    CVector pos;
    CVector dir;

#ifdef _DEBUG
	SHOWNET("TempEvent_BolterSparks");
#endif /* DEBUG */
    cnt = MSG_ReadByte (&net_message);
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    MSG_ReadDir (&net_message, color);
    CL_ParticleEffect_Complex (pos, dir, color, cnt, PARTICLE_SPARKS);
}

void TempEvent_PolyExplosion(void)
{
    short scale_s;
    CVector pos;
    CVector pos2;
    CVector lt_color;
    CVector Zero;    
    long lsize;
    float scale;

#ifdef _DEBUG
	SHOWNET("TempEvent_PolyExplosion");
#endif /* DEBUG */
    Zero.Zero();
    MSG_ReadPos(&net_message,pos);                           // origin
    MSG_ReadPos(&net_message,pos2);                          // angle
    scale_s=MSG_ReadShort(&net_message);
    lsize=MSG_ReadFloat(&net_message);
    if ( lsize )
        MSG_ReadPos(&net_message,lt_color);

    scale=((float)scale_s)/1024.0;                           // convert to float

    // outer ring is a sprite
     // outer ring is a sprite
    CL_PolyExplosion(pos, pos2, cl_mod_polyexplode2, scale *0.1, 4 * scale, 0.2f, lsize, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL);

    // center is a model -- model is oriented 90 degrees off
//    pos2.x += 90;
	// [cek 11-10-99] changed pos2 to up vector cuz it looked better from all orientations.
	AngleToVectors(pos2,forward,right,up);
	CL_PolyExplosion(pos, up, cl_mod_polyexplode, scale * 0.1f, 8 * scale, 0.3f, lsize, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT);
    CL_BigExplosionParticles(pos,scale);
    CL_ExplosionParticles(pos);
}

void TempEvent_PolyExplosionWithSound(void)
{
    short scale_s;
    CVector pos;
    CVector pos2;
    CVector lt_color;
    CVector Zero;    
    long lsize;
    float scale;
    int     iPointContents;

    Zero.Zero();
#ifdef _DEBUG
	SHOWNET("TempEvent_PolyExplosionWithSound");
#endif /* DEBUG */
    MSG_ReadPos(&net_message,pos);                           // origin
    MSG_ReadPos(&net_message,pos2);                          // angle
    scale_s=MSG_ReadShort(&net_message);
    lsize=MSG_ReadFloat(&net_message);
    if ( lsize )
        MSG_ReadPos(&net_message,lt_color);

    scale=((float)scale_s)/1024.0;                           // convert to float

  
    // outer ring is a sprite
    CL_PolyExplosion(pos, pos2, cl_mod_polyexplode2, scale *0.1, 4 * scale, 0.2f, lsize, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL);

    // center is a model -- model is oriented 90 degrees off
//    pos2.x += 90;
	AngleToVectors(pos2,forward,right,up);
	CL_PolyExplosion(pos, up, cl_mod_polyexplode, scale * 0.1f, 8 * scale, 0.3f, lsize, lt_color, RF_FULLBRIGHT|RF_TRANSLUCENT);

    CL_BigExplosionParticles(pos,scale);
    CL_ExplosionParticles(pos);
    if( (iPointContents = CM_PointContents (pos, 0)) & CONTENTS_WATER)
        S_StartSound(pos, 0, 0, cl_sfx_waterexplosion,1,32,2048);
    else
        S_StartSound(pos,0,0,cl_sfx_explosion[0],1,32,2048);
	
}

void TempEvent_Explosion(void)
{
    explosion_t *ex;
    CVector pos;

#ifdef _DEBUG
	SHOWNET("TempEvent_Explosion");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    ex = CL_AllocExplosion ();
    ex->ent.origin = pos;
    ex->type = ex_poly;
    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
    ex->light = 200;
    ex->lightcolor.x = 1.0;
    ex->lightcolor.y = 0.5;
    ex->lightcolor.z = 0.0;

    ex->ent.render_scale.x = ex->ent.render_scale.y = ex->ent.render_scale.z = 1.0;//(rand () / (RAND_MAX << 1)) + 0.75;

    if ( rand () & 1 )
    {
        ex->ent.model = cl_mod_explosion [1];
        ex->frames = 8;
    }
    else
    {
        ex->ent.model = cl_mod_explosion [0];
        ex->frames = 7;
    }

    CL_ExplosionParticles(pos);
}

void TempEvent_ExplosionWithSound(void)
{
    explosion_t *ex;
    CVector pos;

#ifdef _DEBUG
	SHOWNET("TempEvent_ExplosionWithSound");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    ex = CL_AllocExplosion ();
    ex->ent.origin = pos;
    ex->type = ex_poly;
    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
    ex->light = 200;
    ex->lightcolor.x = 1.0;
    ex->lightcolor.y = 0.5;
    ex->lightcolor.z = 0.0;

    ex->ent.render_scale.x = ex->ent.render_scale.y = ex->ent.render_scale.z = 1.0;//(rand () / (RAND_MAX << 1)) + 0.75;

    if ( rand () & 1 )
    {
        ex->ent.model = cl_mod_explosion [1];
        ex->frames = 8;
    }
    else
    {
        ex->ent.model = cl_mod_explosion [0];
        ex->frames = 7;
    }

    CL_ExplosionParticles(pos);
    S_StartSoundQuake2 (pos, 0, 0, cl_sfx_explosion [0], 1, ATTN_NORM, 0);
}

void TempEvent_ExplosionSmall(void)
{
    explosion_t *ex;
    CVector pos;

#ifdef _DEBUG
	SHOWNET("TempEvent_ExplosionSmall");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    ex = CL_AllocExplosion ();
    ex->ent.origin = pos;
    ex->type = ex_poly;
    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
    ex->light = 50;
    ex->lightcolor.x = 1.0;
    ex->lightcolor.y = 0.5;
    ex->lightcolor.z = 0.0;

    ex->ent.render_scale.x = ex->ent.render_scale.y = ex->ent.render_scale.x = (rand () / (RAND_MAX << 1)) + 0.55;

    if ( rand () & 1 )
    {
        ex->ent.model = cl_mod_explosion [1];
        ex->frames = 8;
    }
    else
    {
        ex->ent.model = cl_mod_explosion [0];
        ex->frames = 7;
    }

    CL_SmallExplosionParticles (pos);
}

void TempEvent_ExplosionSmallWithSound(void)
{
    explosion_t *ex;
    CVector pos;

#ifdef _DEBUG
	SHOWNET("TempEvent_ExplosionSmallWithSound");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    ex = CL_AllocExplosion ();
    ex->ent.origin = pos;
    ex->type = ex_poly;
    ex->ent.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;
    ex->start = cl.frame.servertime - CL_FRAME_MILLISECONDS;
    ex->light = 50;
    ex->lightcolor.x = 1.0;
    ex->lightcolor.y = 0.5;
    ex->lightcolor.z = 0.0;

    ex->ent.render_scale.x = ex->ent.render_scale.y = ex->ent.render_scale.x = (rand () / (RAND_MAX << 1)) + 0.55;

    if ( rand () & 1 )
    {
        ex->ent.model = cl_mod_explosion [1];
        ex->frames = 8;
    }
    else
    {
        ex->ent.model = cl_mod_explosion [0];
        ex->frames = 7;
    }

    CL_SmallExplosionParticles (pos);
    S_StartSoundQuake2 (pos, 0, 0, cl_sfx_explosion [0], 1, ATTN_NORM, 0);
}


void TempEvent_Sparks(void)
{
    CVector pos;
    CVector dir;
    CVector vecColor;
    int     cnt;
    int     iDamage;
	byte	use_smoke;

#ifdef _DEBUG
	SHOWNET("TempEvent_Sparks");
#endif /* DEBUG */
    iDamage = MSG_ReadByte(&net_message);
    MSG_ReadPos (&net_message, pos);
    MSG_ReadPos (&net_message, dir);
    MSG_ReadPos (&net_message, vecColor);
	use_smoke = MSG_ReadByte( &net_message );

    // CL_ParticleEffect_Complex (pos, dir, 0x10, 6, PARTICLE_SPARK_1);
    // vecColor.Set(0.8, 0.8, 0.0);
    CL_ParticleEffectSparks(pos, dir, vecColor, iDamage);
	if( use_smoke )
	{
	    CL_SmokeAndFlash(pos);
	}
    // impact sound
    cnt = rand()&15;
    if ( cnt == 1 )
        S_StartSoundQuake2 (pos, 0, 0, cl_sfx_ric1, 1, ATTN_NORM, 0);
    else if ( cnt == 2 )
        S_StartSoundQuake2 (pos, 0, 0, cl_sfx_ric2, 1, ATTN_NORM, 0);
    else if ( cnt == 3 )
        S_StartSoundQuake2 (pos, 0, 0, cl_sfx_ric3, 1, ATTN_NORM, 0);
}

/*
void TempEvent_Gunshot(void)
{
    CVector     vecPos, vecDir;
    CVector     vecColor(1.0f, 1.0f, 1.0f);
    int         i, iCount;
    cparticle_t *p;
    float       d;

	SHOWNET("TempEvent_Gunshot");
    MSG_ReadPos(&net_message, vecPos);
    MSG_ReadDir (&net_message, vecDir);
    CL_SmokeAndFlash(vecPos);

    iCount = 4;
    vecDir = vecDir * 200;
    for( i=0; i<iCount; i++)
    {
        if(! (p = CL_SpawnComParticle()) )
            return;

        d = rand() & 7;
        p->time = cl.time;
        p->color_rgb.Set(1.0f, 1.0f, 0.5f);
        p->org.x = vecPos.x + ((rand() & 7) -4) + d;// * vecDir.x;
        p->org.y = vecPos.y + ((rand() & 7) -4) + d;// * vecDir.y;
        p->org.z = vecPos.z + ((rand() & 7) -4) + d;// * vecDir.z;
        p->lastorg = p->org;
        p->vel.x = crand() * vecDir.x;
        p->vel.y = crand() * vecDir.y;
        p->vel.z = crand() * vecDir.z;
        
//        p->type = iSparkIndex[iPIndex];
        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.Set(p->vel.x * -0.5, p->vel.y * -0.5, PARTICLE_GRAVITY * 0.5);
        //p->accel.z = PARTICLE_GRAVITY;
        p->alpha = 1.0;
        //p->alphavel = -1.0 / (0.5 + frand()*0.3);
        p->scalevel = p->alphavel = -0.5 - (0.10 * d);
        p->pscale = d * 0.05;

    }
}
*/

void TempEvent_Explosion_WTF(void)
{
    CVector pos;
    CVector angles;
    CVector lt_color;
    int     cnt;
    int     r;
    short   flags, light;
    float   scale;

#ifdef _DEBUG
	SHOWNET("TempEvent_Explosion_WTF");
#endif /* DEBUG */

    MSG_ReadPos (&net_message, pos);

    //	read facing angle
    angles.x = MSG_ReadAngle16 (&net_message);
    angles.y = MSG_ReadAngle16 (&net_message);
    angles.z = MSG_ReadAngle16 (&net_message);

    scale=MSG_ReadFloat(&net_message);

    cnt=MSG_ReadByte(&net_message);              // SCALE
    r=MSG_ReadLong(&net_message);                // NUMFRAMES
    flags=MSG_ReadByte(&net_message);            // FLAGS

    light=MSG_ReadShort(&net_message);

    if ( light )
        MSG_ReadPos (&net_message, lt_color);

    CL_Explosion(pos,angles,scale,cnt,r,flags,light,lt_color,NULL, 1.0);
}

void TempEvent_Smoke(void)
{
    CVector pos;
    float scale, alpha_scale, alpha_end;
    int cnt;

    //	read position
#ifdef _DEBUG
	SHOWNET("TempEvent_Smoke");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    scale=MSG_ReadFloat(&net_message);
    cnt=MSG_ReadFloat(&net_message);
    alpha_scale=MSG_ReadFloat(&net_message);
    alpha_end=MSG_ReadFloat(&net_message);
    CL_Smoke (pos, scale, cnt, alpha_scale, alpha_end);
}

#pragma message ("// SCG[1/21/00]: Remove this feature. It does nothing. ")
void TempEvent_Tracer(void)
{
    CVector pos;
    CVector pos2;
    CVector dir;
    float alpha_scale, scale;

#ifdef _DEBUG
	SHOWNET("TempEvent_Tracer");
#endif /* DEBUG */
    MSG_ReadPos(&net_message,pos);               // START
    MSG_ReadPos(&net_message,pos2);              // END
    MSG_ReadPos(&net_message,dir);               // FORWARD vector
    alpha_scale=MSG_ReadFloat(&net_message);     // LENGTH
    scale=MSG_ReadFloat(&net_message);           // killtime

    //CL_Tracer(pos,pos2,dir,alpha_scale,scale);  //Commented out.. tracers never worked anyways.
}

void TempEvent_ParticleFall(void)
{
    CVector pos;
    CVector dir;
	CVector color;
    int speed, cnt, type;

#ifdef _DEBUG
	SHOWNET("TempEvent_ParticleFall");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    speed = MSG_ReadByte (&net_message);
    cnt = MSG_ReadByte (&net_message);
	color.x = MSG_ReadFloat( &net_message );
	color.y = MSG_ReadFloat( &net_message );
	color.z = MSG_ReadFloat( &net_message );
    type = MSG_ReadByte (&net_message);

    CL_ParticleEffect_Complex2 (pos, dir, speed * 10, color, cnt, type, PARTICLE_GRAVITY);
}

void TempEvent_ParticleRise(void)
{
    CVector pos;
    CVector dir;
	CVector color;
    int speed, cnt, type;

#ifdef _DEBUG
	SHOWNET("TempEvent_ParticleRise");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    speed = MSG_ReadByte (&net_message);
    cnt = MSG_ReadByte (&net_message);
	color.x = MSG_ReadFloat( &net_message );
	color.y = MSG_ReadFloat( &net_message );
	color.z = MSG_ReadFloat( &net_message );
    type = MSG_ReadByte (&net_message);

    CL_ParticleEffect_Complex2 (pos, dir, speed * 10, color, cnt, type, -PARTICLE_GRAVITY*0.5);
}

void TempEvent_ParticleFloat(void)
{
    CVector pos;
    CVector dir;
	CVector color;
    int speed, cnt, type;

#ifdef _DEBUG
	SHOWNET("TempEvent_ParticleFloat");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);
    MSG_ReadDir (&net_message, dir);
    speed = MSG_ReadByte (&net_message);
    cnt = MSG_ReadByte (&net_message);
	color.x = MSG_ReadFloat( &net_message );
	color.y = MSG_ReadFloat( &net_message );
	color.z = MSG_ReadFloat( &net_message );
    type = MSG_ReadByte (&net_message);
    CL_ParticleEffect_Complex2 (pos, dir, speed * 10, color, cnt, type, 0);
}

void TempEvent_SurfSprite(void)
{
/*
    CVector pos;
    short planeIndex;
    int type, entNum, cnt;
    void *model;
    void *hitModel;
    float roll, scale;
    entity_state_t  *entState;

    MSG_ReadPos (&net_message, pos);
    planeIndex = MSG_ReadShort (&net_message);  //	plane to tag
    cnt = MSG_ReadShort (&net_message);
    type = MSG_ReadByte (&net_message); //	frame
    model = cl.model_draw [cnt];
    hitModel = cl.model_draw [MSG_ReadShort (&net_message)];
    entNum = MSG_ReadShort (&net_message);

    roll = 0.0;
    scale = 1.0;

    if ( entNum > 0 )
    {
        entState = &cl_entities [entNum].current;
        re.AddSurfaceSprite (pos, entState->origin, entState->angles, hitModel, planeIndex, cl_mod_bulletHole, type, roll, scale, SSF_LIGHTBEAM);
    }
    else
        re.AddSurfaceSprite (pos, CVector(0,0,0), CVector(0,0,0), hitModel, planeIndex, model, type, roll, scale, 0);
*/
}

void TempEvent_BloodSplat(void)
{
    CVector pos;
    short planeIndex;
    int type, entNum;
    void *hitModel;
    float roll, scale;
    entity_state_t  *entState;

    MSG_ReadPos (&net_message, pos);
    planeIndex = MSG_ReadShort (&net_message);  //	plane to tag
    hitModel = cl.model_draw [MSG_ReadShort (&net_message)];
    entNum = MSG_ReadShort (&net_message);

// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	return;
#endif
	if (sv_violence->value > 0.0)
		return;

    type = rand ()& 1; //	2 frames of blood right now + 1 brain splat

    // roll = rand () % 360;
    roll = (frand() * 360);
    scale = crand () * 0.5 + 0.75;

	CVector vZero(0,0,0);
    if ( entNum > 0 )
    {
        entState = &cl_entities [entNum].current;
        re.AddSurfaceSprite (pos, entState->origin, entState->angles, hitModel, planeIndex, cl_mod_surfacemarks[SM_BULLET], type, roll, scale, SSF_LIGHTBEAM);
    }
    else
        re.AddSurfaceSprite (pos, vZero, vZero, hitModel, planeIndex, cl_mod_surfacemarks[SM_BLOOD], type, roll, scale, 0);
}

void TempEvent_BulletHole(void)
{
/*
	CVector			pos, vecDir;
    short			planeIndex;
    int				type, entNum, index, d;
    void			*hitModel;
    float			roll, scale;
    entity_state_t  *entState;
    cparticle_t		*p;

    MSG_ReadPos (&net_message, pos);
    planeIndex = MSG_ReadShort (&net_message);  //	plane to tag
    type = MSG_ReadByte (&net_message); //	frame
    index = MSG_ReadShort (&net_message);
    hitModel = cl.model_draw [index];
    entNum = MSG_ReadShort (&net_message);
    
	CL_SmokeAndFlash(pos);

    if ( type > 0 )
        //	light holes and glass damage right now
        // roll = rand () % 360;
        roll = (frand() * 360);
    else
        roll = 0;
    scale = crand () * 0.25 + 0.75;

    vecDir = pos - cl.refdef.vieworg;
    for( int i=0; i<4; i++)
    {
        if(! (p = CL_SpawnComParticle()) )
            return;

        d = rand() & 7;
        p->time = cl.time;
        p->color_rgb.Set(1.0f, 1.0f, 0.5f);
        p->org.x = pos.x + ((rand() & 7) -4) + d;
        p->org.y = pos.y + ((rand() & 7) -4) + d;
        p->org.z = pos.z + ((rand() & 7) -4) + d;
        p->lastorg = p->org;
        p->vel.x = crand() * vecDir.x;
        p->vel.y = crand() * vecDir.y;
        p->vel.z = crand() * vecDir.z;
        p->type = PARTICLE_BEAM_SPARKS;
        p->accel.Set(p->vel.x * -0.5, p->vel.y * -0.5, PARTICLE_GRAVITY * 0.5);
        p->alpha = 1.0;
        p->scalevel = p->alphavel = -0.5 - (0.10 * d);
        p->pscale = d * 0.05;

    }
*/
}

void TempEvent_Ricochet(void)
{
    int     iSoundIndex;
    CVector vecPos;

#ifdef _DEBUG
	SHOWNET("TempEvent_Ricochet");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, vecPos);
    // make a ricochet sound
    iSoundIndex = (int)(frand() * 7);
    S_StartSoundQuake2(vecPos,0,0,cl_sfx_ricochet[iSoundIndex],1,ATTN_NORM,0);
}

void TempEvent_HeadShot(void)
{
/*
	CVector pos;
    short planeIndex;
    int type, entNum;
    void *hitModel;
    float roll, scale;
    entity_state_t  *entState;

    MSG_ReadPos (&net_message, pos);
    planeIndex = MSG_ReadShort (&net_message);  //	plane to tag
    hitModel = cl.model_draw [MSG_ReadShort (&net_message)];
    entNum = MSG_ReadShort (&net_message);

    type = 2;   //	third frame is bloody brainy mess

    // roll = rand () % 360;
    roll = (frand() * 360);
    scale = crand () * 0.2 + 0.8;
    if ( entNum > 0 )
    {
        entState = &cl_entities [entNum].current;
        re.AddSurfaceSprite (pos, entState->origin, entState->angles, hitModel, planeIndex, cl_mod_bulletHole, type, roll, scale, SSF_LIGHTBEAM);
    }
    else
        re.AddSurfaceSprite (pos, CVector(0,0,0), CVector(0,0,0), hitModel, planeIndex, cl_mod_bloodSplat, type, roll, scale, 0);
*/
}

void TempEvent_SurfaceMark(void)
{
    CVector		vPos, vDir, vOrigin, vAngles,vNorm;
    short		sPlaneIndex;
    int			iEntityNumber, d, i, frame;
    void		*pvHitModel, *pModel;
    float		fRoll, fScale;
	byte		cType;
    cparticle_t	*p;
	
    MSG_ReadPos(&net_message, vPos);
    sPlaneIndex		= MSG_ReadShort(&net_message);
    pvHitModel		= cl.model_draw[MSG_ReadShort(&net_message)];
    iEntityNumber	= MSG_ReadShort(&net_message);
    fScale			= MSG_ReadFloat(&net_message);
    fRoll			= MSG_ReadFloat(&net_message);
    cType			= MSG_ReadByte(&net_message);

	// SCG[1/21/00]: make sure we initialize!
	frame = 0;

    if( iEntityNumber > 0)
    {
		vOrigin = cl_entities[iEntityNumber].current.origin;
		vAngles = cl_entities[iEntityNumber].current.angles;
    }
	else
	{
//		vOrigin.Zero();
		vOrigin = vPos;
		vAngles.Zero();
	}

	pModel = cl_mod_surfacemarks[cType];

	switch( cType )
	{
	case SM_BLOOD:
		fRoll = (frand() * 360);
		fScale = crand () * 0.5 + 0.75;
	    frame = rand ()& 1;
		break;
	case SM_BULLET:
		{
			MSG_ReadPos(&net_message, vNorm);
			fRoll = (frand() * 360);
			int count = (int)fScale;
			fScale = crand () * 0.25 + 0.75;

			vDir = vPos - cl.refdef.vieworg;
			frame = 0;

			CVector rndPos;
			for (i = 0; i < count; i++)
			{
				if (vNorm.Length() > 0.1)
				{
					rndPos = vNorm;
					rndPos.Normalize();
					VectorToAngles(rndPos,rndPos);
					rndPos.roll += 360*frand();
					if (rndPos.roll > 360) rndPos.roll -= 360;
					AngleToVectors(rndPos,forward,right,up);
					rndPos = vPos + 15*frand()*right;
				}
				else
					rndPos = vPos;

				// smoke!
				CL_SmokeAndFlash( vPos );

				// sparks		
				for(int j=0; j<10;j++)
				{
					if(! (p = CL_SpawnComParticle()) )
						return;

					d = rand() & 7;
					p->time = cl.time;
					p->color_rgb.Set(1.0f, 1.0f, 0.5f);
					p->org.x = vPos.x + ((rand() & 7) -4) + d;
					p->org.y = vPos.y + ((rand() & 7) -4) + d;
					p->org.z = vPos.z + ((rand() & 7) -4) + d;
					p->lastorg = p->org;
					p->vel.x = crand() * vDir.x;
					p->vel.y = crand() * vDir.y;
					p->vel.z = crand() * vDir.z;
					p->type = PARTICLE_BEAM_SPARKS;
					p->accel.Set(p->vel.x * -0.5, p->vel.y * -0.5, PARTICLE_GRAVITY * 0.5);
					p->alpha = 1.0;
					p->scalevel = p->alphavel = -0.5 - (0.10 * d);
					p->pscale = d * 0.05;

				}
				// the sprite!
				re.AddSurfaceSprite( rndPos, rndPos, vAngles, pvHitModel, sPlaneIndex, pModel, frame, fRoll, fScale, 0);

				// make a ricochet sound
				int iSoundIndex = rand() & 7;//(int)(frand() * 7);
				S_StartSoundQuake2(vOrigin,0,0,cl_sfx_ricochet[iSoundIndex],0.7,ATTN_IDLE,0);
			}
		}
		return;
	case SM_NOVA:
		CL_NovaEffect(vPos, vAngles, vAngles);		
		fRoll = 0;
		fScale = (frand ()-0.5) * 0.3 + 1.3;
		frame = 0;
		re.AddSurfaceSprite( vPos, vOrigin, vAngles, pvHitModel, sPlaneIndex, pModel, frame, fRoll, fScale, SSF_FULLBRIGHT|SSF_QUICKFADE);
		return;
		break;
	case SM_META:
		fRoll = 0;
		fScale = 0.5 + 0.25*frand();
		frame = 0;
		re.AddSurfaceSprite( vPos, vOrigin, vAngles, pvHitModel, sPlaneIndex, pModel, frame, fRoll, fScale, SSF_FULLBRIGHT|SSF_QUICKFADE);
		return;
		break;
	case SM_MULTIBULLET:
		fRoll = frand () * 360;
		frame = 0;
		fScale = 1.0;
		break;
	case SM_SCORCH:
		fRoll = frand () * 360;
		fScale = crand () * 0.8 + 0.3;
		frame = 0;
		break;
	case SM_FIST:
		fRoll = frand () * 360;
		fScale = 0.25;
		frame = 0;
		break;
	case SM_EARTH:
		frame = 0;
		fRoll = frand () * 360;
		fScale *= 1.0+frand()*0.4;
		break;
	case SM_DK_SLASH:
		frame = 0; 
		fScale = 0.5 + frand() * 0.2;
		re.AddSurfaceSprite( vPos, vOrigin, vAngles, pvHitModel, sPlaneIndex, pModel, frame, fRoll, fScale, SSF_FULLBRIGHT|SSF_QUICKFADE);
		return;
		break;
	case SM_CLAW:
		frame = 0;
		break;
	}

	re.AddSurfaceSprite( vPos, vOrigin, vAngles, pvHitModel, sPlaneIndex, pModel, frame, fRoll, fScale, 0);
}

void TempEvent_SpiralParticles(void)
{
    CVector pos;
    CVector pos2;
    CVector dir;
    int cnt;

#ifdef _DEBUG
	SHOWNET("TempEvent_SpiralParticles");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);             // origin
    MSG_ReadPos (&net_message, pos2);            // mins
    MSG_ReadPos (&net_message, dir);             // maxs
    cnt = MSG_ReadByte (&net_message);          //	seconds
    CL_SpiralParticles (pos, pos2, dir, cnt * 1000);
}

void TempEvent_SpiralParticles_2(void)
{
    CVector pos;
    CVector mins;
    CVector maxs;
	CVector color;
	particle_type_t pt;
    int time;
	float speed;

#ifdef _DEBUG
	SHOWNET("TempEvent_SpiralParticles");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);             // origin
    MSG_ReadPos (&net_message, mins);            // mins
    MSG_ReadPos (&net_message, maxs);             // maxs
    MSG_ReadPos (&net_message, color);             // color
    time = MSG_ReadByte (&net_message);          //	seconds
	pt = (particle_type_t) MSG_ReadByte(&net_message);			// particle type
	speed = MSG_ReadFloat(&net_message);
    CL_SpiralParticles2 (pos, mins, maxs, color, speed, pt, time* 1000);
}
void TempEvent_DoubleHelixParticles(void)
{
    CVector pos;
    CVector pos2;
    CVector dir;
    int cnt;
    
#ifdef _DEBUG
	SHOWNET("TempEvent_DoubleHelixParticles");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, pos);             // origin
    MSG_ReadPos (&net_message, pos2);            // mins
    MSG_ReadPos (&net_message, dir);             // maxs
    cnt = MSG_ReadByte (&net_message);          //	seconds
    CL_DoubleHelixParticles (pos, pos2, dir, cnt * 1000);
}

void TempEvent_ScaleAlpha(void)
{
    CVector org;
    CVector angles;
    CVector color;
    float scale, alpha_scale, alpha_end;
    int numframes, modelindex;
    short light;

#ifdef _DEBUG
	SHOWNET("TempEvent_ScaleAlpha");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, org);                   // position
    angles.x = MSG_ReadAngle16 (&net_message);       // angles
    angles.y = MSG_ReadAngle16 (&net_message);
    angles.z = MSG_ReadAngle16 (&net_message);

    scale=MSG_ReadFloat(&net_message);                 // scale

    numframes=MSG_ReadByte(&net_message);                    // numframes
    modelindex=MSG_ReadLong(&net_message);                      // modelindex

    alpha_scale=MSG_ReadFloat(&net_message);           // alpha_scale
    alpha_end=MSG_ReadFloat(&net_message);             // alpha_end

    light=MSG_ReadShort(&net_message);                 // size of light
    if ( light )
        MSG_ReadPos(&net_message,color);             // light color
    
    CL_ScaleAlpha(org, angles, scale, numframes, modelindex, alpha_scale, alpha_end, light, color);
}

void TempEvent_ScaleImage(void)
{
    CVector org;
    CVector angles;
    float scale_start, scale_end, alpha, killtime;
    int modelindex;

#ifdef _DEBUG
	SHOWNET("TempEvent_ScaleImage");
#endif /* DEBUG */
    MSG_ReadPos (&net_message, org);                   // position
    MSG_ReadPos (&net_message,angles);                 // angles
    scale_start=MSG_ReadFloat(&net_message);                 // scale start
    scale_end=MSG_ReadFloat(&net_message);                // scale end
    modelindex=MSG_ReadShort(&net_message);                   // modelindex
    alpha=MSG_ReadFloat(&net_message);           // alpha
    killtime=MSG_ReadFloat(&net_message);                   // time
    CL_ScaleImage(org, angles, scale_start, scale_end, modelindex, alpha, killtime);
}

void TempEvent_Light(void)
{
    CVector vecOrg;
    CVector vecColor;
    float   fIntensity;
    cdlight_t   *dlight;

#ifdef _DEBUG
	SHOWNET("TempEvent_Light");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, vecOrg);
    MSG_ReadPos(&net_message, vecColor);
    fIntensity = MSG_ReadFloat(&net_message);

    if( (dlight = CL_AllocDlight(TE_LIGHT)) != NULL)
    {
        dlight->color = vecColor;
        dlight->decay = fIntensity * 2;
        dlight->origin = vecOrg;
        dlight->radius = fIntensity;
        dlight->die = cl.time + 150;
    }

}

void TempEvent_ShockOrbBounce(void)
{
    CVector vecOrigin, vecAngles, vecOffSetUp, vecOffSetDown, vecNormal;
    float fScaleStart, fScaleEnd, fAlpha, fKillTime, fRingOffset;
    int iModelIndex, iRing;
    
#ifdef _DEBUG
	SHOWNET("TempEvent_ShockOrbBounce");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, vecOrigin);
    MSG_ReadPos(&net_message, vecAngles);
    iModelIndex = MSG_ReadShort(&net_message);

    // normalize the angles, save them
    vecNormal.x = vecAngles.z;
    vecNormal.y = vecAngles.y;
    vecNormal.z = vecAngles.x;

    vecNormal.Normalize();

    fRingOffset = 20;
    fScaleStart = 1.5;
    fScaleEnd = 60;
    fAlpha = 0.3;
    fKillTime = 0.55;
    vecOffSetUp = vecOrigin;
    vecOffSetDown = vecOrigin;
    // center to top
    CL_ScaleImage(vecOrigin, vecAngles, fScaleStart, fScaleEnd, iModelIndex, fAlpha, fKillTime);
    for(iRing = 0; iRing < 4; iRing++)
    {
        fAlpha -= 0.05;
        fScaleEnd *= 0.6;
        fKillTime *= 1.15;
        vecOffSetUp += ( vecNormal * fRingOffset );
        vecOffSetDown += vecNormal * -fRingOffset;
        CL_ScaleImage(vecOffSetUp, vecAngles, fScaleStart, fScaleEnd, iModelIndex, fAlpha, fKillTime);
        CL_ScaleImage(vecOffSetDown, vecAngles, fScaleStart, fScaleEnd, iModelIndex, fAlpha, fKillTime);
    }
//    S_StartSoundQuake2(vecOrigin,0,0,cl_sfx_shockbounce[0],1,ATTN_NORM,0);
	S_StartSound(vecOrigin,0,0,cl_sfx_shockbounce[0],1.0,2048,4096);
    V_AddLight(vecOrigin, 500, 0, 0, 1);
}

///////////////////////////////////////////////////////////////////////
// Logic[5/2/99]: TempEvent_KickView()
//
//  Description:
//      When the TE_KICKVIEW temporary event message is received by the
//      client, this routine is run. The server sends a vector 
//      describing the extent of the kick angles, an optional FOV,
//      the time it will take to reach the effect extent, and the
//      amount of time it will take to return to a "normal" view state
//
//  Parms:
//      void
//
//  Return:
//      void
//  
//  Bugs/Limitations:
//      Overrides any existing view kicks which may be currently
//      executing.
void TempEvent_KickView(void)
{
    float   fFOV;
    int     iKickVelocityTo;
    int     iKickVelocityReturn;

#ifdef _DEBUG
	SHOWNET("TempEvent_KickView");
#endif /* DEBUG */
    MSG_ReadPos(&net_message, g_KickView.vecKickAngles);
    fFOV = MSG_ReadFloat(&net_message);
    iKickVelocityTo = MSG_ReadShort(&net_message);
    iKickVelocityReturn = MSG_ReadShort(&net_message);
    //if(!thirdPersonView)//NSS[10/25/99]:thirdperson view is always set... commented out until I can figure out why and how to fix it.
    //{
        
        g_KickView.fFOV = fFOV;
        g_KickView.iKickVelocityTo = iKickVelocityTo;
        g_KickView.iKickVelocityReturn = iKickVelocityReturn;
        g_KickView.iStartTime = cl.frame.servertime;
        g_KickView.fToInc = 1.0f/(float)g_KickView.iKickVelocityTo;
        g_KickView.fFromInc = 1.0f/(float)g_KickView.iKickVelocityReturn;
        g_KickView.fBaseFOV = fov->value; 
    //}
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Logic[3/26/99]: Function ptrs
//
//  This is not as clean as it could be --rather than abstracting all
//  of these, prefacing the render calls with the network code (as 
//  above) some are modified and called directly (e.g. CL_BloodSplash).
//
//  First things first, get this code working flawlessly with the
//  existing code base. Features and further design improvements come
//  next.
//
void CL_InitDispatchFunctionList(void)
{
    cl_TempEventFunctionList[TE_BLOODSPLAT].pvfDispatchFunction				= TempEvent_BloodSplat;
    cl_TempEventFunctionList[TE_BLOOD].pvfDispatchFunction					= CL_BloodSplash;
    cl_TempEventFunctionList[TE_BLOOD_FADE].pvfDispatchFunction				= CL_BloodFade;
    cl_TempEventFunctionList[TE_BLUE_SPARKS].pvfDispatchFunction			= TempEvent_BlueSparks;
    cl_TempEventFunctionList[TE_SPARKLES].pvfDispatchFunction				= CL_SparkleSplash;
    cl_TempEventFunctionList[TE_SPARKS].pvfDispatchFunction					= TempEvent_Sparks;
    cl_TempEventFunctionList[TE_SHIELD_SPARKS].pvfDispatchFunction			= TempEvent_ShieldSparks;
    cl_TempEventFunctionList[TE_LASER_SPARKS].pvfDispatchFunction			= TempEvent_LaserSparks;
    cl_TempEventFunctionList[TE_BOLTER_SPARKS].pvfDispatchFunction			= TempEvent_BolterSparks;
    cl_TempEventFunctionList[TE_SUPERTRIDENT].pvfDispatchFunction			= CL_SuperTrident;
    cl_TempEventFunctionList[TE_ICE_CRYSTALS].pvfDispatchFunction			= CL_IceCrystals;
    cl_TempEventFunctionList[TE_POLYEXPLOSION].pvfDispatchFunction			= TempEvent_PolyExplosion;
    cl_TempEventFunctionList[TE_POLYEXPLOSION_SND].pvfDispatchFunction		= TempEvent_PolyExplosionWithSound;
    cl_TempEventFunctionList[TE_EXPLOSION2].pvfDispatchFunction				= TempEvent_ExplosionWithSound;
    cl_TempEventFunctionList[TE_EXPLOSION1].pvfDispatchFunction				= TempEvent_Explosion;
    cl_TempEventFunctionList[TE_EXPLOSION_SMALL1].pvfDispatchFunction		= TempEvent_ExplosionSmallWithSound;
    cl_TempEventFunctionList[TE_EXPLOSION_SMALL2].pvfDispatchFunction		= TempEvent_ExplosionSmall;
    cl_TempEventFunctionList[TE_NOVAHIT].pvfDispatchFunction				= CL_KcoreHit;
    cl_TempEventFunctionList[TE_EXPLOSION].pvfDispatchFunction				= TempEvent_Explosion_WTF;
    cl_TempEventFunctionList[TE_SMOKE].pvfDispatchFunction					= TempEvent_Smoke;
    cl_TempEventFunctionList[TE_BEAM_SPARKS].pvfDispatchFunction			= CL_BeamSparks;
    cl_TempEventFunctionList[TE_FIRESMOKE].pvfDispatchFunction				= CL_TimedFire;
    cl_TempEventFunctionList[TE_SCALEROTATE].pvfDispatchFunction			= CL_ScaleRotate;
    cl_TempEventFunctionList[TE_SCALEIMAGE].pvfDispatchFunction				= TempEvent_ScaleImage;
    cl_TempEventFunctionList[TE_ALPHASCALE].pvfDispatchFunction				= TempEvent_ScaleAlpha;
    cl_TempEventFunctionList[TE_TRACER].pvfDispatchFunction					= TempEvent_Tracer;
    cl_TempEventFunctionList[TE_BFG_LASER].pvfDispatchFunction				= CL_ParseLaser;
    cl_TempEventFunctionList[TE_TRACK_ENTITY].pvfDispatchFunction			= CL_TrackEntityMsg;
    cl_TempEventFunctionList[TE_PARTICLE_FALL].pvfDispatchFunction			= TempEvent_ParticleFall;
    cl_TempEventFunctionList[TE_PARTICLE_RISE].pvfDispatchFunction			= TempEvent_ParticleRise;
    cl_TempEventFunctionList[TE_PARTICLE_FLOAT].pvfDispatchFunction			= TempEvent_ParticleFloat;
    cl_TempEventFunctionList[TE_SURFSPRITE].pvfDispatchFunction				= TempEvent_SurfSprite;
    cl_TempEventFunctionList[TE_SPIRAL_PARTICLES].pvfDispatchFunction		= TempEvent_SpiralParticles;
    cl_TempEventFunctionList[TE_DOUBLEHELIX_PARTICLES].pvfDispatchFunction	= TempEvent_DoubleHelixParticles;
    cl_TempEventFunctionList[TE_SHOCKORB_BOUNCE].pvfDispatchFunction		= TempEvent_ShockOrbBounce;
	cl_TempEventFunctionList[TE_CRYO_SPRAY].pvfDispatchFunction				= CL_CryoSpray;
    cl_TempEventFunctionList[TE_LIGHT].pvfDispatchFunction					= TempEvent_Light;
    cl_TempEventFunctionList[TE_KICKVIEW].pvfDispatchFunction				= TempEvent_KickView;
    cl_TempEventFunctionList[TE_RICOCHET].pvfDispatchFunction				= TempEvent_Ricochet;
    cl_TempEventFunctionList[TE_SCORCHMARK].pvfDispatchFunction				= TempEvent_SurfaceMark;
    cl_TempEventFunctionList[TE_ALPHASCALEROTATE].pvfDispatchFunction		= CL_ScaleRotateAlpha;
    cl_TempEventFunctionList[TE_SPIRAL_PARTICLES2].pvfDispatchFunction		= TempEvent_SpiralParticles_2;

}

/*
=================
CL_ParseTEnt
=================
*/


void CL_ParseTEnt (void)
{
    int         type;
    type = MSG_ReadByte (&net_message);

#ifdef _DEBUG
	char buf[128];
	Com_sprintf(buf,sizeof(buf),"CL_ParseTEnt - type: %d",type);
	SHOWNET(buf);
#endif /* DEBUG */
#ifdef PROFILE_TEMP_EVENTS
    cl_TempEventFunction_t *tef;
#endif
    // Logic[3/26/99]: Removing this switch statement
    // Logic[3/29/99]: DIE DIE DIE DIE DIE!!!!!!!! 800+ lines of Code Constipation *annihilated*
    if(cl_TempEventFunctionList[type].pvfDispatchFunction != NULL)
    {
        // Profiling
        #ifdef PROFILE_TEMP_EVENTS
            tef = &cl_TempEventFunctionList[type];
            tef->iTimeStartFunction = Sys_Milliseconds();   // not terribly accurate. 
        #endif

        cl_TempEventFunctionList[type].pvfDispatchFunction();
        
        #ifdef PROFILE_TEMP_EVENTS
            tef->iTimeCompleteFunction = Sys_Milliseconds();
            tef->iAccumulatedTimeForFunction += tef->iTimeCompleteFunction - tef->iTimeStartFunction;
            if(tef->iFrame == cl.frame.serverframe)
            {
                tef->iRunCountThisFrame++;
            } 
			else 
			{
                tef->iFrame = cl.frame.serverframe;
                tef->iRunCountThisFrame = 0;
            }
            Com_DPrintf("Type: %i, Time: %lu, Total Time: %lu\nFrame: %i, Runcount/Frame: %i\n", type, 
                tef->iTimeCompleteFunction - tef->iTimeStartFunction,
                tef->iAccumulatedTimeForFunction, tef->iFrame, tef->iRunCountThisFrame);
        #endif

    } 
	else 
	{
		Com_Error (ERR_DROP, "CL_ParseTEnt: bad type [%d]\n",type);
    }     
}

float   com_AngleMod (float ang)
{
    return(360.0 / 65536) * ((int)(ang * (65536 / 360.0)) & 65535);
}

float   com_AngleDiff (float ang1, float ang2)
{
    float   diff;

    ang1 = com_AngleMod (ang1);
    ang2 = com_AngleMod (ang2);

    diff = ang1 - ang2;

    if ( diff < -180 )
    {
        diff += 360;
        diff *= -1;
    }
    else if ( diff < 0 )
        diff = fabs (diff);
    else if ( diff > 180 )
        diff = fabs (diff - 360);
    else
        diff *= -1;

    return	diff;
}

//#define VectorMul(vec,val)    vec[0] *= val; vec[1] *= val; vec[2] *= val
//#define VectorDiv(vec,val)    vec[0] /= val; vec[1] /= val; vec[2] /= val

//---------------------------------------------------------------------------
// CL_FindTracker()
//---------------------------------------------------------------------------
te_tracker_t *CL_FindTracker(int track_index)
{
    te_tracker_t *tracker;
    int loop;

    tracker=trackerList;
    for ( loop=0; loop<TE_MAX; loop++,tracker++ )
        if ( tracker->track_index==track_index )
            break;

    if ( loop==TE_MAX )
    {
        // track info wasn't in list, find new track info
        tracker=trackerList;
        for ( loop=0; loop<TE_MAX; loop++,tracker++ )
            if ( !tracker->track_index )
                break;

        if ( loop==TE_MAX )
            return(NULL);

        // and initialize
        memset(tracker,0,sizeof(*tracker));
        tracker->trailHead=-1;
    }

    tracker->track_index=track_index;
    return(tracker);
}




// Logic[4/6/99]: Rebuild the dis-assembled track entity system
///////////////////////////////////////////////////////////////////////
//  CL_AddTrackEnts(void)
//
//  Description:
//      Adds all track entities to the view. The function sets up some
//      common attributes for all track entities, then common attributes
//      for a particular track entity, then decides which functions should
//      be run based on flags passed to the client with the track ent.
//
//  Parameters:
//      none
//
//  Returns:
//      none
//
//  Bugs:
//      *still* not the ideal solution for tracking entities. If someone 
//      happens to think of a more elegant way to handle this, either 
//      with rewriting server-side code, or ideally keeping the current
//      system compatable, feel free to rip it up (or let me know)
//
//  Todo:
//      Kill all the dereferencing that happens with trackData members.
//
void CL_AddTrackEnts(void)
{
    int                 iLoop;
    te_tracker_t        *tracker;
    trackParamData_t    trackData;
//    trace_t             trace;
    float               RF_SHAKE_VAL;
    entity_t            ent;

//    cl.ClientEntityManager->RunFrame();

    // clear out trackData structure
    memset(&trackData, 0, sizeof(trackParamData_t));

    RF_SHAKE_VAL = 0.7f;    // just laziness
    tracker = trackerList;
    trackData.shake.x = (frand() - 0.5f) * RF_SHAKE_VAL * 2 - RF_SHAKE_VAL;
    trackData.shake.y = (frand() - 0.5f) * RF_SHAKE_VAL * 2 - RF_SHAKE_VAL;
    trackData.shake.z = (frand() - 0.5f) * RF_SHAKE_VAL * 2 - RF_SHAKE_VAL;
    trackData.fRandom = frand();

    for(iLoop = 0; iLoop < TE_MAX; iLoop++,tracker++)
    {
        if(!tracker->track_index)
            continue;
        trackData.track = &cl_entities[tracker->track_index];
        if(trackData.track->current.renderfx & RF_NODRAW)
            continue;
		if (trackData.track->current.effects2 & EF2_BLOODY_GIB)
		{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
			continue;
#endif
			if (sv_violence->value > 0.0)
				continue;
		}

        // Logic[4/6/99]: This setup routine is *still* more than 
        //  the track entity system should ideally have. The necessary
        //  changes would require LOTS of code rewrites on the server 
        //  side. This is probably a good compromise.
        trackData.owner = &cl_entities[tracker->src_index];
        trackData.o_track = &cl_entities[tracker->track_index];
        trackData.dstent = &cl_entities[tracker->dst_index];
        trackData.extraent = &cl_entities[tracker->extra_index];
        memset(&ent, 0, sizeof(entity_t));
        trackData.ent = &ent;
        trackData.tracker = tracker;
        ent.flags=trackData.track->current.renderfx;

        trackData.muzzle_str[0] = "hr_muzzle";
        trackData.muzzle_str[1] = "hr_muzzle2";
        trackData.muzzle_str[2] = "hr_muzzle3";
        // third person view
        if ( (tracker->src_index==cl.playernum+1) && (!(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE)) )
        {
            thirdPersonView = trackData.thirdPersonView = 0;
            
            // use refdef in first person view
            trackData.origin = cl.refdef.vieworg;
            trackData.angles = cl.refdef.viewangles;

            trackData.o_origin = trackData.origin;
            trackData.o_angles = trackData.angles;
        } else {
            thirdPersonView = trackData.thirdPersonView = 1;

            
            // no depthhack for 3rd person stuff
            trackData.ent->flags &= ~RF_DEPTHHACK;

            if ( trackData.extraent && trackData.extraent->render_ent )
            {
                trackData.track=trackData.extraent;
                trackData.origin = trackData.track->render_ent->origin;
                trackData.angles = trackData.track->render_ent->angles;
            }
            else
            {
                // interpolate origin
//                trackData.origin.x = trackData.owner->prev.origin.x + cl.lerpfrac * (trackData.owner->current.origin.x - trackData.owner->prev.origin.x);
//				trackData.origin.y = trackData.owner->prev.origin.y + cl.lerpfrac * (trackData.owner->current.origin.y - trackData.owner->prev.origin.y);
//				trackData.origin.z = trackData.owner->prev.origin.z + cl.lerpfrac * (trackData.owner->current.origin.z - trackData.owner->prev.origin.z);
                trackData.origin = 
					trackData.origin.Interpolate( trackData.owner->current.origin, trackData.owner->prev.origin, cl.lerpfrac );

                // get angles, X is 1/3rd actual value
                trackData.angles = trackData.owner->current.angles;
                //NSS[11/10/99]:Don't know why whoever did this did this, but it is wrong
				//trackData.angles.x *= 3;
            }
			/// amw: if this is the old origin, it's not going to lerp
//            trackData.o_origin.x = trackData.owner->prev.origin.x + cl.lerpfrac * (trackData.owner->current.origin.x - trackData.owner->prev.origin.x);
//            trackData.o_origin.y = trackData.owner->prev.origin.y + cl.lerpfrac * (trackData.owner->current.origin.y - trackData.owner->prev.origin.y);
//            trackData.o_origin.z = trackData.owner->prev.origin.z + cl.lerpfrac * (trackData.owner->current.origin.z - trackData.owner->prev.origin.z);
                trackData.o_origin = 
					trackData.o_origin.Interpolate( trackData.owner->current.origin, trackData.owner->prev.origin, cl.lerpfrac );

            trackData.o_angles = trackData.owner->current.angles;
            //NSS[11/10/99]:Don't know why whoever did this did this, but it is wrong
			//trackData.o_angles.x *= 3;
        }

        // set position  (adds forward,right,up to origin)
        AngleToVectors(trackData.angles, trackData.forward, trackData.right,trackData.up);
        
        trackData.o_forward = trackData.forward;
        trackData.o_right = trackData.right;
        trackData.o_up = trackData.up;

        trackData.fwd = trackData.forward;
        trackData.forward.Multiply(trackData.tracker->forward);
        trackData.right.Multiply(trackData.tracker->right);
        trackData.up.Multiply(trackData.tracker->up);
        ent.origin = trackData.origin + trackData.forward;
        ent.origin = ent.origin + trackData.right;
        ent.origin = ent.origin + trackData.up;
        ent.angles = trackData.angles;
        ent.render_scale = trackData.track->current.render_scale;
		ent.alpha = trackData.track->current.alpha;

        // Dispatch to correct effects routines based on bits in 
        // track entity flags. DO NOT put special cases in this 
        // routine!

		//<nss>
		//********** THIS MUST BE CALLED BEFORE ALL OTHER TRACK ENTITIES OTHERWISE THE ORIGIN WILL NOT BE OFFSET ***********
		if( tracker->fxflags & TEF_MODEL )
			TrackEnt_Model( &trackData );
		
		//********** THIS SECOND THING MUST BE CALLED BEFORE ALL OTHER TRACK ENTITIES, BUT THE PREVIOUS TEF_MODEl OTHERWISE THE ORIGIN WILL NOT BE OFFSET ***********
		//Will still work... just if you expect the TE's origin to be in the right 'visual' place these need to be called first. So  don't create new functions above these
		//that's all.
		if( tracker->fxflags & TEF_MODEL2 )
			TrackEnt_Model2( &trackData );
		
		if(tracker->flags & TEF_DSTINDEX)
            TrackEnt_DestIndex(&trackData);

        if(tracker->fxflags & TEF_LIGHT || tracker->fxflags & TEF_LIGHT_SHRINK)
            TrackEntFX_Light(&trackData);

		if(tracker->fxflags & TEF_TRACER)
            TrackEntFX_Tracer(&trackData);

        if(tracker->fxflags & TEF_ANIMATE)
            TrackEntFX_Animate(&trackData);

		if(tracker->fxflags & TEF_ARTIFACT_FX)
            TrackEntFX_Artifact(&trackData);
 
		if(tracker->fxflags & TEF_FIRE)
            TrackEntFX_Fire(&trackData);
      
		if(tracker->fxflags & TEF_FLASHLIGHT)
            TrackEntFX_FlashLight(&trackData);

		//<nss>
		if(tracker->fxflags & TEF_THUNDERSKEET_FX)
			TrackEnt_ThunderSkeetFx( &trackData);
		//<nss>
		if( tracker->fxflags & TEF_JETTRAIL )
			TrackEnt_JetTrail( &trackData, &ent );
		//<nss>
		if( tracker->fxflags & TEF_SMOKETRAIL )
			TrackEnt_SmokeTrailIt(&trackData);
		//<nss>
		if( tracker->fxflags & TEF_STAVROS)
			TrackEnt_Stavros( &trackData );
		//<nss>
		if( tracker->fxflags & TEF_ELECTRICSWORD)
			TrackEnt_ElectricSword( &trackData );
		//<nss>
		if( tracker->fxflags & TEF_FLAMESWORD)
			TrackEnt_FlameSword( &trackData);
		//<nss>
		if( tracker->fxflags & TEF_COMPLEXPARTICLE)
			TrackEnt_ComplexParticle( &trackData, &ent );
		//<nss>
		if( tracker->fxflags & TEF_SPRITE )
			TrackEnt_Sprite( &trackData );
		//<nss>
		if( tracker->fxflags & TEF_SLUDGE_FX)
			TrackEnt_Sludge( &trackData );
		
		if( tracker->fxflags & TEF_ALPHA_SPRITE )
			TrackEnt_AlphaSprite( &trackData );
		
		if (tracker->fxflags & TEF_SHRINKINGMODEL)
			TrackEnt_ShrinkingModel( &trackData );

		if (tracker->fxflags & TEF_METABEAMS)
			TrackEntFX_Metabeams(&trackData);

		if (tracker->fxflags & TEF_PROJ_FX)
			TrackEntFX_Proj(&trackData);

		if (tracker->fxflags & TEF_ANIMATE2)
			TrackEntFX_Animate2(&trackData);

		if (tracker->fxflags & TEF_REAPER)
			TrackEntFX_Reaper(&trackData);

		if (tracker->fxflags & TEF_SUNFLARE_FX)
			TrackEntFX_SunFlareFX(&trackData);		

		if (tracker->fxflags & TEF_DAIKATANA)
			TrackEntFX_Daikatana(&trackData);

        if(tracker->renderfx & RF_SHAKE && !(tracker->renderfx & RF_DEPTHHACK) && !(tracker->renderfx & RF_3P_NODRAW))
            TrackEntRFX_Shake(&trackData);


        if(tracker->renderfx & RF_NOVALASER)
            TrackEntRFX_NovaLaser(&trackData);
        else if(tracker->renderfx & RF_SPOTLIGHT)
            TrackEntRFX_SpotLight(&trackData);
        else if(tracker->renderfx & RF_LIGHTNING)
            TrackEntRFX_Lightning(&trackData);
//        else if(!(tracker->fxflags & TEF_FX_ONLY) && !(tracker->fxflags & TEF_SPRITE) && !(tracker->fxflags & TEF_JETTRAIL) && !(tracker->fxflags & TEF_SMOKETRAIL) && !(tracker->fxflags & TEF_STAVROS) && !(tracker->fxflags & TEF_COMPLEXPARTICLE) && !(tracker->fxflags & TEF_LIGHT_SHRINK) && !(tracker->fxflags & TEF_ANIMATE2))
        else if(!(tracker->fxflags & (TEF_FX_ONLY|TEF_SPRITE|TEF_JETTRAIL|TEF_SMOKETRAIL|TEF_STAVROS|TEF_COMPLEXPARTICLE|TEF_LIGHT_SHRINK|TEF_ANIMATE2)))
			 TrackEntRFX_Default(&trackData);
		//NSS[10/27/99]: This is a temporary hack for a big problem that I am not sure how I want to 
		//deal with.  The problem is that this function will draw the entity being tracked's model here.
		//The dumb thing is that we are already drawing the model... I might add some weird flag that will
		//tell it to draw the entity's model here... not sure yet.

//		if(!(tracker->fxflags & TEF_FX_ONLY) && !(tracker->fxflags & TEF_SPRITE) && !(tracker->fxflags & TEF_JETTRAIL) && !(tracker->fxflags & TEF_SMOKETRAIL) && !(tracker->fxflags & TEF_STAVROS) && !(tracker->fxflags & TEF_COMPLEXPARTICLE) && !(tracker->fxflags & TEF_METABEAMS)  && !(tracker->fxflags & TEF_LIGHT_SHRINK) && !(tracker->fxflags & TEF_ANIMATE2))
		if(!(tracker->fxflags & (TEF_FX_ONLY|TEF_SPRITE|TEF_JETTRAIL|TEF_SMOKETRAIL|TEF_STAVROS|TEF_COMPLEXPARTICLE|TEF_METABEAMS|TEF_LIGHT_SHRINK|TEF_ANIMATE2)))
		{
			// effects have been applied, lightning, beams, novalaser add their own entities
			if(! (tracker->renderfx & (RF_LIGHTNING | RF_BEAM | RF_NOVALASER )) && (! (trackData.thirdPersonView && (tracker->renderfx & RF_3P_NODRAW) ) ) )
         			V_AddEntity(&ent);
		}

		if (tracker->freeMe)
		{
			CL_FreeTrackEntity(tracker->track_index);
		}
    }
}


//---------------------------------------------------------------------------
// CL_TrackEntityMsg()
//---------------------------------------------------------------------------
void CL_TrackEntityMsg(void)
{
    te_tracker_t *tracker;
    CVector fru;
    long track_index;
    unsigned long flags,renderfx,msgflags,fxflags=0;

#ifdef _DEBUG
	SHOWNET("CL_TrackEntityMsg");
#endif /* DEBUG */
    track_index=MSG_ReadLong(&net_message);
    if ( !(tracker=CL_FindTracker(track_index)) )
        return;

	// SCG[1/21/00]: lets initialize things!
	renderfx = 0;

    switch ( MSG_ReadByte(&net_message) )             // read mode
    {
        case 1:                                      // new / update tracking
            msgflags=MSG_ReadLong(&net_message);            // flags
            renderfx=MSG_ReadLong(&net_message);            // render flags

#ifdef _DEBUG
	char buf[128];
	Com_sprintf(buf,sizeof(buf),"CL_TrackEntityMsg: flags=%x",msgflags);
	SHOWNET(buf);
#endif /* DEBUG */
            // read any data needed
            if ( msgflags & TEF_FXFLAGS )
                fxflags=MSG_ReadLong(&net_message);            // flags

            if ( msgflags & TEF_SRCINDEX )
                tracker->src_index=MSG_ReadLong(&net_message);        // src ent index

            if ( msgflags & TEF_FRU )
            {
                MSG_ReadPos(&net_message,fru);                        // forward, right, up offsets
                tracker->forward=fru[0];
                tracker->right=fru[1];
                tracker->up=fru[2];
            }

            if ( msgflags & TEF_DSTINDEX )
                tracker->dst_index=MSG_ReadLong(&net_message);     // dst ent index

            if ( msgflags & TEF_DSTPOS )
                MSG_ReadPos(&net_message,tracker->dstpos);         // dst position

            if ( msgflags & TEF_LENGTH )
                tracker->length=MSG_ReadFloat(&net_message);       // length

            if ( msgflags & TEF_LIGHTCOLOR )
                MSG_ReadPos(&net_message,tracker->lightColor);     // light color

            if ( msgflags & TEF_LIGHTSIZE )
                tracker->lightSize=MSG_ReadFloat(&net_message);    // light size

            if ( msgflags & TEF_MODELINDEX )
                tracker->modelindex=MSG_ReadLong(&net_message);    // model index (for muzzle flash)

            if ( msgflags & TEF_NUMFRAMES )
            {
                tracker->numframes=MSG_ReadShort(&net_message);    // number of frames
                tracker->curframe=0;
            }

            if ( msgflags & TEF_SCALE )
                tracker->scale=MSG_ReadFloat(&net_message);        // scale

            if ( msgflags & TEF_FRAMETIME )
            {
                tracker->frametime=MSG_ReadFloat(&net_message);
                tracker->curframetime=tracker->frametime;
            }

            if ( msgflags & TEF_EXTRAINDEX )
                tracker->extra_index=MSG_ReadLong(&net_message);

            if ( msgflags & TEF_ALTPOS )
                MSG_ReadPos(&net_message,tracker->altpos);

            if ( msgflags & TEF_ALTPOS2 )
                MSG_ReadPos(&net_message,tracker->altpos2);

            if ( msgflags & TEF_ALTANGLE )
            {
                MSG_ReadPos(&net_message,tracker->altangle);
				// SCG[11/1/99]: ReadPos is a short and truncates values
				// SCG[11/1/99]: on the server side.  In order to retain some
				// SCG[11/1/99]: of the precision, we multiply the value by 100 
				// SCG[11/1/99]: on the server side.
				tracker->altangle.Multiply( 0.01 );
            }
			
			if( msgflags & TEF_MODELINDEX2)
			{
				tracker->modelindex2=MSG_ReadLong(&net_message);
			}

			if( msgflags & TEF_SCALE2)
			{
				tracker->scale2 = MSG_ReadFloat(&net_message);
			}
			//NSS[11/7/99]:
			if( msgflags & TEF_LONG1)
			{
				tracker->Long1  = 	MSG_ReadLong(&net_message);
			}
			//NSS[11/7/99]:
			if( msgflags & TEF_LONG2)
			{
				tracker->Long2  = 	MSG_ReadLong(&net_message);
			}
			//NSS[11/7/99]:
			if( msgflags & TEF_HARDPOINT)
			{
//				sprintf(tracker->HardPoint,"%s",MSG_ReadString(&net_message));
				Com_sprintf(tracker->HardPoint,sizeof(tracker->HardPoint), "%s",MSG_ReadString(&net_message));
			}

			if( msgflags & TEF_HARDPOINT2)
			{
// SCG[1/16/00]: 				sprintf(tracker->HardPoint2,"%s",MSG_ReadString(&net_message));
				Com_sprintf(tracker->HardPoint2, sizeof(tracker->HardPoint2), "%s",MSG_ReadString(&net_message));
			}

            // handle flags
            flags=msgflags;
            if ( fxflags & TEF_ADDFX )
            {
                fxflags |= tracker->fxflags;
                renderfx=tracker->renderfx;
            }

            if ( fxflags & TEF_REMOVEFX )
            {
                fxflags &= TEF_EFFECTS;
                fxflags = tracker->fxflags & ~fxflags;
                renderfx=tracker->renderfx;
            }

            fxflags &= ~(TEF_ADDFX|TEF_REMOVEFX);

            tracker->flags=flags;
            tracker->fxflags=fxflags;
            tracker->renderfx=renderfx;
            break;

        case 2:                                      // kill tracking
			CL_FreeTrackEntity(tracker->track_index);
//            tracker->track_index=0;
//			memset(tracker,0,sizeof(tracker));

            // clean up lightning
            lightningInfo[tracker->lightning].numPoints=-1;
            tracker->lightning=-1;
            break;
    }

    // always init to -1
    if ( renderfx & RF_LIGHTNING )
        tracker->lightning = -1;

    if ( tracker->fxflags & TEF_FIRE )
        tracker->nextsunsmoke=0;

 }

///////////////////////////////////////////////////////////////////////////////
//	CL_FreeTrackEntity
//	
//	frees a track entity
//	used when an entity that is a track entity gets removed by the server
///////////////////////////////////////////////////////////////////////////////
void    CL_FreeTrackEntity (int track_entity_index)
{
    te_tracker_t    *tracker = trackerList;
    int             i;

    for ( i = 0; i < TE_MAX; i++, tracker++ )
    {
        if ( tracker->track_index == track_entity_index )
        {
			memset(tracker,0,sizeof(te_tracker_t));
            tracker->track_index = 0;
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//	CL_FreeTrackEntity
//	
//	frees a track entity
//	used when an entity that is a track entity gets removed by the server
///////////////////////////////////////////////////////////////////////////////
void    CL_ClearTrackEntities (void)
{
    te_tracker_t    *tracker = trackerList;
    int             i;
    for ( i = 0; i < TE_MAX; i++, tracker++ )
    {
		memset(tracker,0,sizeof(te_tracker_t));
		tracker->track_index = 0;
    }
}



//---------------------------------------------------------------------------
// addTrailPoint()
//---------------------------------------------------------------------------
void addTrailPoint(te_tracker_t *tracker, CVector &pos,short type)
{
    te_trail_t *trail;
    int i;

    // lets the beam segs know that the entity is dead
    if ( !type )
        tracker->trailNextPoint=cl.time+50;
    else if ( type==2 )
    {
        tracker->numframes=type;
        tracker->trailNextPoint=cl.time+30000;
        return;
    }

    // find empty slot
    for ( i=0; i<TE_MAX_TRAILS; i++ )
        if ( !(tracker->trail[i].flags & TE_TRAIL_INUSE) )
            break;

    if ( i==TE_MAX_TRAILS )
        return;

    // set current head's constant starting point
    // (normally, start is based on position of entity)
    if ( tracker->trailHead != -1 )
    {
        tracker->trail[tracker->trailHead].start = pos;
        tracker->trail[tracker->trailHead].flags |= TE_TRAIL_USESTART;
    }

    // add this as the head trail
    trail=&tracker->trail[i];
    trail->flags |= TE_TRAIL_INUSE|TE_TRAIL_SAVEVEC|TE_TRAIL_WAIT;
    trail->next=tracker->trailHead;

    if ( trail->next==i )
        i=1;

    trail->end = pos;
    tracker->trailHead=i;
    trail->alpha=.75;
}

//---------------------------------------------------------------------------
// removeTrailPoint() - removes last position/trail
//---------------------------------------------------------------------------
void removeTrailPoint(te_tracker_t *tracker)
{
    short last,newlast=-1;

    last=tracker->trailHead;
    while ( tracker->trail[last].next != -1 )
    {
        newlast=last;
        last=tracker->trail[last].next;
    }

    if ( newlast != -1 )
        tracker->trail[newlast].next=-1;

    tracker->trail[last].flags=0;

    if ( last==tracker->trailHead )
        tracker->trailHead=-1;
}

//---------------------------------------------------------------------------
// CL_SpawnTravelingSpark()
//---------------------------------------------------------------------------
void CL_SpawnTravelingSpark(explosion_t *ex,short startNode,short endNode)
{
    cparticle_t *p;
    CVector vec,normal;

    if ( !(p=CL_SpawnComParticle()) )
        return;

    p->time = cl.time;
    p->pscale=1;

    p->type = PARTICLE_BEAM_SPARKS;
    p->alpha = 1.0;
    p->alphavel = -(1.4 + .2*frand());
//    p->color = 4 + (rand()&7);
    p->color_rgb.x=p->color_rgb.y=frand()*.2;
    p->color_rgb.z=1;

    vec = ex->node[endNode] - ex->node[startNode];     // vec = next - cur
    normal = vec;
	normal.Normalize();
    p->vel = normal * EXP_TS_VELOCITY;

    p->lastorg = cl_entities[ex->ent_number].current.origin + ex->node[startNode];
    p->lastorg = p->lastorg + ex->ofs;
    p->org = p->lastorg;

    ex->scale_factor=cl.time + vec.Length()/EXP_TS_VELOCITY * 1000;
    p->die_time=ex->scale_factor;

    CL_ParticleEffect_Complex (p->org, CVector(0,0,0), CVector(1.0f, 1.0f, 1.0f), 3, PARTICLE_BLUE_SPARKS);
}

/*
=================
CL_AddExplosions
=================
*/

void CL_AddExplosions (void)
{
    entity_t    *ent;
    int         i;
    explosion_t *ex;
    float       frac, perc, scale;
    int         f;
    float fFireScale;
    
//    short startNode,endNode;
    CVector ident = vec3_identity;

    for ( i=0, ex=cl_explosions ; i< MAX_EXPLOSIONS ; i++, ex++ )
    {
        if(ex->ent.model == NULL)
            continue;
        if ( ex->type == ex_free)
            continue;
        frac = (cl.time - ex->start) / CL_FRAME_MILLISECONDS;
        f = floor(frac);
        ent = &ex->ent;
        switch ( ex->type )
        {
            case ex_mflash:
                if ( f >= ex->frames-1 )
                    ex->type = ex_free;
                break;
            case ex_misc:
                if ( f >= ex->frames-1 )
                {
                    ex->type = ex_free;
                    break;
                }
                ent->alpha = 1.0 - frac/(ex->frames-1);
                break;
            case ex_flash:
                if ( f >= 1 )
                {
                    ex->type = ex_free;
                    break;
                }
                ent->alpha = 1.0;
                break;
            case ex_poly:
                if ( f >= ex->frames-1 )
                {
                    ex->type = ex_free;
                    break;
                }
                ent->alpha = (16.0 - (float)f)/16.0;
                if ( f < 10 )
                {
                    ent->skinnum = (f>>1);
                    if ( ent->skinnum < 0 )
                        ent->skinnum = 0;
                }
                else
                {
                    ent->flags |= RF_TRANSLUCENT;
                    if ( f < 13 )
                        ent->skinnum = 5;
                    else
                        ent->skinnum = 6;
                }
                break;

            case    ex_sprite:
                ent->alpha = ((ex->frames) - (float)f)/ex->frames;
                if ( f >= ex->frames-1 )
                {
                    ex->type = ex_free;
                    break;
                }
                break;

            case    ex_explosion:
                if ( f >= ex->frames )         // amw - changed to > ... last frame was not being displayed
                {
                    ex->type = ex_free;
                    break;
                }
                break;

            case    ex_explosion_time:
                
                if ( f<0 )                   // my little timeshift (for starting at any frame) produces
                    f=ex->frames+f;         // negative frames until the first time (f >= ex->frames)

                if ( f >= ex->frames )       //&&& amw - FIXME - this really prevents the last from from displaying
                {
                    // but I'm not sure what this fn is supposed to really do so I'm leaving it
                    f=0;
                    ex->start=cl.frame.servertime-CL_FRAME_MILLISECONDS;
                }

                if ( cl.time > ex->killtime-1000 )
                {
                    
                    fFireScale = ex->end_alpha*((ex->killtime-cl.time)*0.001);
                    ent->alpha=fFireScale;
                    ent->render_scale.Set(fFireScale, fFireScale * 2, 1.0);
                }
                else
                {
                    CVector pos;
                    fFireScale = 0.9f + (crand() * 0.05);
                    ent->alpha=fFireScale;
                    ent->render_scale.Set(fFireScale, fFireScale * 2, 1.0);
                    pos.x = ex->ent.origin.x;
                    pos.y = ex->ent.origin.y;
                    pos.z = ex->ent.origin.z+40;
                    CL_Smoke2(pos,3,2,25,20);
                }

                V_AddLight (ent->origin, 70 + (ex->light-70)*ent->alpha*fFireScale,ex->lightcolor[0], ex->lightcolor[1], ex->lightcolor[2]);
                if ( cl.time > ex->killtime )
                {
                    ex->type = ex_free;
                    break;
                }
                break;

            case    ex_explosion_alpha:
                ent->alpha = ((ex->frames) - (float)f)/ex->frames;
                if ( f >= ex->frames-1 )
                {
                    ex->type = ex_free;
                    break;
                }
                break;

            case ex_scale_rotate:         // one frame, scales from 1 to max, lightcolor is used as avelocity
                
                perc = (cl.time-ex->start) / ex->scale_factor;                          // perc=(time-start_time)/scale_time
                scale = ex->start_scale + (ex->end_scale-ex->start_scale) * perc;       // scale=start_scale+(end_scale-start_scale)*perc
                ent->render_scale.x=ent->render_scale.y=ent->render_scale.z=scale;

                if ( cl.time >= ex->start+ex->scale_factor )
                {
                    ex->type = ex_free;
                    break;
                }

                scale = (cl.time-ex->end_alpha) / CL_FRAME_MILLISECONDS;    // scale = ms since last frame / ms per frame
                ent->angles.x += ex->avelocity[0] * scale;
                ent->angles.y += ex->avelocity[1] * scale;
                ent->angles.z += ex->avelocity[2] * scale;

                ex->end_alpha=cl.time;
                f=0;

                break;

            case ex_scale_rotate_alpha:         // one frame, scales from 1 to max, lightcolor is used as avelocity
                perc = (cl.time-ex->start) / ex->scale_factor;                          // perc=(time-start_time)/scale_time
                scale = ex->start_scale + (ex->end_scale-ex->start_scale) * perc;       // scale=start_scale+(end_scale-start_scale)*perc
                ent->render_scale.x=ent->render_scale.y=ent->render_scale.z=scale;

                ent->alpha=ex->start_alpha-((ex->start_alpha)*perc);     // alpha doesn't go lower than .4

                if ( cl.time >= ex->start+ex->scale_factor )
                {
                    ex->type = ex_free;
                    break;
                }

                scale = (cl.time-ex->end_alpha) / CL_FRAME_MILLISECONDS;    // scale = ms since last frame / ms per frame
                ent->angles.x += ex->avelocity[0] * scale;
                ent->angles.y += ex->avelocity[1] * scale;
                ent->angles.z += ex->avelocity[2] * scale;

                ex->end_alpha=cl.time;

                f=0;
                break;

            case    ex_scale_alpha:            // cycles through frames, scales alpha based on settings
                ent->alpha *= ex->alpha_scale;

                if ( f > ex->frames-1 )
                {
                    ex->start=cl.frame.servertime-CL_FRAME_MILLISECONDS;
                    f=0;
                }

                if ( ent->alpha < ex->end_alpha )
                {
                    ex->type = ex_free;
                    break;
                }
                break;

            case ex_scale_image:
                if ( cl.time >= ex->killtime )
                {
                    ex->type=ex_free;
                    break;
                }

                perc=(ex->killtime-cl.time) / (ex->killtime-ex->start);

				ent->render_scale.x = ex->alpha_scale+(1-perc)*ex->end_scale;
				ent->render_scale.y = ent->render_scale.x;
				ent->render_scale.z = ent->render_scale.x;

				ent->frame=0;
                break;

            case ex_tracer:
                // calc laser front
                perc = 1 - ((ex->killtime-cl.time) / (ex->killtime-ex->start));
                if ( perc > 1 )
                    perc=1;
				ent->origin.x = ex->pos.x + (ex->end.x-ex->pos.x) * perc;
				ent->origin.y = ex->pos.y + (ex->end.y-ex->pos.y) * perc;
				ent->origin.z = ex->pos.z + (ex->end.z-ex->pos.z) * perc;


                // calc laser end
                perc = 1 - ((ex->killtime-cl.time) / (ex->killtime-ex->start)) - ex->scale_factor;
                if ( perc < 0 )
                    perc=0;
				ent->oldorigin.x = ex->pos.x + (ex->end.x-ex->pos.x) * perc;
				ent->oldorigin.y = ex->pos.y + (ex->end.y-ex->pos.y) * perc;
				ent->oldorigin.z = ex->pos.z + (ex->end.z-ex->pos.z) * perc;

                if ( perc > 1 )
                    ex->type=ex_free;

				beam_AddLaser (ent->origin, ent->oldorigin, ident, ent->alpha, 1.0, 1.0, BEAM_TEX_TRACER, 0);
                continue;// SCG[1/21/00]: this does nothing different than break....
                break;

            case ex_anim_rotate:
                break;

            case    ex_scale_sprite:           // only one frame, scales from large to 0.1
                ent->render_scale.x *= ex->scale_factor;
                ent->alpha *= ex->alpha_scale;

                if ( ent->render_scale [0] < ex->end_scale || ent->alpha < ex->end_alpha )
                {
                    ex->type = ex_free;
                    break;
                }

                ent->render_scale.y *= ex->scale_factor;
                ent->render_scale.z *= ex->scale_factor;
                break;
        }

        if ( ex->type == ex_free )
            continue;

        if ( ex->light )
            // seems like a light size of 70 is basically not visible, so, treat 70 as the base (0)
            V_AddLight (ent->origin, 70 + (ex->light-70)*ent->alpha,ex->lightcolor[0], ex->lightcolor[1], ex->lightcolor[2]);

        if ( ex->type != ex_tracer )
        {
            ent->oldorigin = ent->origin;

            if ( ex->type != ex_scale_sprite && ex->type != ex_scale_image )
            {
                if ( f < 0 )
                    f = 0;
                ent->frame = ex->baseframe + f;     // + 1; mike:   ???? why add 1??? this is zero based
                ent->oldframe = ex->baseframe + f;
                ent->backlerp = 1.0 - cl.lerpfrac;
            }
            else
            {
                ent->frame = 0;
                ent->oldframe = 0;
                ent->backlerp = 1.0;
            }
        }
        V_AddEntity (ent);
    }
}

/*
=================
CL_AddLasers
=================
*/

void CL_AddLasers (void)
{
    centity_t *dstent;
    laser_t     *l;
    int         i;
	CVector		vColor(0,0,1);

	for ( i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++ )
    {
        if ( l->endtime >= cl.time )
        {
            if ( l->track_dst > 0 )
            {
                dstent=&cl_entities[l->track_dst];

                // use refdef position if ent is client, and not in 3rd person view
                if ( !(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE) && (l->track_dst==cl.playernum+1) )
                {
                    l->ent.oldorigin = cl.refdef.vieworg;
                    l->ent.oldorigin.z -= 16;
                }
                else
                {
                    l->ent.oldorigin.x = dstent->prev.origin.x + cl.lerpfrac * (dstent->current.origin.x - dstent->prev.origin.x);
                    l->ent.oldorigin.y = dstent->prev.origin.y + cl.lerpfrac * (dstent->current.origin.y - dstent->prev.origin.y);
                    l->ent.oldorigin.z = dstent->prev.origin.z + cl.lerpfrac * (dstent->current.origin.z - dstent->prev.origin.z);
                }
            }
			beam_AddLaser (l->ent.origin, l->ent.oldorigin, vColor, 1, 1, 1, BEAM_TEX_LIGHTNING, 0);

//            V_AddEntity (&l->ent);
        }
    }
}


/*
=================
CL_AddTEnts
=================
*/
void CL_AddTEnts (void)
{
    CL_AddExplosions ();
    CL_AddLasers ();
}


