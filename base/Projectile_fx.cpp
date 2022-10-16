#include "client.h"

#include "dk_cl_curve.h"
#include "dk_beams.h"
//#include "ClientEntityManager.h"

// all of these funcs need this...
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

#include "Projectile_fx.h"

//---------------------------------------------------------------------------------------------------------------------
// Projectile tracker helper functions
//---------------------------------------------------------------------------------------------------------------------

// implementation over in cl_tent.cpp
void TrackEntFX_Proj_PolyExplosion(CVector &org, CVector &ang, float scale, float lt_size, CVector &lt_color);
void TrackEntFX_AnimExplosion(trackParamData_t *trackData, int pointContents, float scale,float alpha, void* model, short frames, float msdelay);
void CL_GenerateComplexParticle2( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity);
void CL_GenerateComplexParticleRadius( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity,float radius);
void CL_SpriteExplosion(CVector &vecPos, float fScale);
void CL_ExplosionParticles ( CVector &org);
void CL_PolyExplosion( CVector &pos, CVector &angles, void *model, float start_scale, float end_scale, float scale_time, float lsize, CVector &lcolor, int iEntFlags);

extern CVector forward,right,up,zero_vector;
#define GET_TRACKER				te_tracker_t *tracker = trackData->tracker
#define WRAP_VALUE(val,min,max)	( (val <= min) ? (val = max) : ((val >= max) ? (val = min) : (0)) )
//---------------------------------------------------------------------------------------------------------------------
// Kineticore functions
//---------------------------------------------------------------------------------------------------------------------
extern void* cl_mod_kcorehit;				// cl_tent.cpp
void Kcore_Ice(trackParamData_t *trackData)
{
	// make some ice crystals
	CVector org = trackData->track->lerp_origin;
	CVector dir = trackData->tracker->altpos;

	float d;
	cparticle_t *p;
	for ( int i = 0; i < 4; i++ )
	{
		if ( !(p=CL_SpawnComParticle()) )
		{
			return;
		}

		p->type=PARTICLE_ICE;
		p->pscale=.4;
		p->time = cl.time;
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

void Proj_KCore_Launch(trackParamData_t *trackData, int pointContents)
{
	Kcore_Ice(trackData);
	trackData->tracker->numframes &= ~TEF_PROJ_LAUNCH;
}

void Proj_KCore_Fly(trackParamData_t *trackData, int pointContents)
{
	CVector vDir(crand(),crand(),0.0);
	CVector vColor(1.0,1.0,1.0);
	CVector vAlpha(1.0,0.65,0);
	CVector vGravity(0,0,-20);
	int particle = PARTICLE_SNOW;
	int count = 1;
	if ((rand() & 0x03) == 0x03)
	{
		if (pointContents & MASK_WATER)
		{
			vGravity.Set(0,0,20);
			particle = PARTICLE_ICE;
			count = 3;
		}

		CL_GenerateComplexParticle2(trackData->track->lerp_origin,		// emission origin
									vDir,		// some direction thinggy
									vColor,				// color
									vAlpha,				// alpha (start, depletion, rand)
									.5,								// scale
									count,									// count
									40,									// spread
									150,								// velocity
									particle,						// particle type
									vGravity);					// gravity
	}
}

void Proj_KCore_Special(trackParamData_t *trackData, int pointContents)
{
	TrackEntFX_AnimExplosion(trackData, pointContents, 0.5+0.75*frand(),1.0, cl_mod_kcorehit, 5,0);
	Kcore_Ice(trackData);
	trackData->tracker->numframes &= ~TEF_PROJ_SPECIAL;				// kcore sfx is a one-time thinggy
}

void Proj_KCore_Die(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;

	CVector dir = track->current.origin - track->prev.origin;
	dir.Normalize();

	CVector vColor(1,1,1);
	CVector vAlpha(1.0,0.35,0);
	CVector vGravity(0,0,-50);
	CL_GenerateComplexParticle2(track->lerp_origin,		// emission origin
								dir,				// some direction thinggy
								vColor,				// color
								vAlpha,				// alpha (start, depletion, rand)
								.8,								// scale
								20,									// count
								60,									// spread
								100,								// velocity
								PARTICLE_ICE,						// particle type
								vGravity);					// gravity
	
	tracker->fxflags &= ~TEF_PROJ_FX;
}

//---------------------------------------------------------------------------------------------------------------------
// Sidewinder functions
//---------------------------------------------------------------------------------------------------------------------
extern void* cl_sfx_SWwater[];
extern void* cl_sfx_SWwhoosh;

void Proj_Sidewinder_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	// set up our spin
	float angdel = tracker->nextsunsmoke;
	float radius = 10.0 - tracker->ss_start;
	CVector ang = track->current.angles;
	if (angdel < 30)
		angdel = tracker->nextsunsmoke = 30;


	float rot = tracker->ss_angle - angdel;
	if (rot < 0)
	{
		angdel += 5;								// speed up the spin rate
		tracker->nextsunsmoke = angdel;
		if (pointContents & MASK_WATER)
			S_StartSound(track->lerp_origin, 0, 0, cl_sfx_SWwater[(int)(4.95*frand())],0.6,200,1000);
		else
			S_StartSound(track->lerp_origin, 0, 0, cl_sfx_SWwhoosh,0.8,200,1024);
		rot += 360;
	}

	tracker->curframetime -= cls.frametime;			// sync to time
	if(tracker->curframetime < 0)
	{
		tracker->curframetime += 0.05;

		if (radius > 1)
		{
			tracker->ss_start += 0.05;				// decrease the radius
		}
		else
			radius = 1;

		tracker->ss_angle = rot;
	}

	ang.roll += rot;
	AngleToVectors(ang,forward,right,up);
	trackData->track->lerp_origin += radius*up;


	forward = -forward;
	// make a cool smoke trail
	particle_type_t pt;
	float scale, adep, alpha;
	short count;
	CVector gravity(0,0,0);
	if (pointContents & MASK_WATER)
	{
		scale = 2.0+frand();
		pt = PARTICLE_BUBBLE;
		count = 15;
		adep = 0.70;
		alpha = 0.8;
	}
	else
	{
		scale = 1.4+0.4*crand();
		pt = PARTICLE_SMOKE;
		count = 3;
		adep = 0.25;
		alpha = 0.6;
		gravity.Set(0,0,15);
	}
	CVector vColor(0.3,0.3,0.3);
	CVector vAlpha(alpha,adep,0);
	CL_GenerateComplexParticle2(track->lerp_origin,					// emission origin
								forward,							// some direction thinggy
								vColor,				// color
								vAlpha,				// alpha (start, depletion, rand)
								scale,								// scale
								count,								// count
								30,									// spread
								15,									// velocity
								pt,									// particle type
								gravity);							// gravity

	V_AddLight(track->lerp_origin, tracker->lightSize, tracker->lightColor[0], tracker->lightColor[1], tracker->lightColor[2]);
}

void Proj_Sidewinder_Die(trackParamData_t *trackData, int pointContents)
{
	te_tracker_t *tracker = trackData->tracker;
	CVector org = trackData->track->current.origin;
	TrackEntFX_Proj_PolyExplosion(org, tracker->altpos, 1.0, tracker->lightSize, tracker->lightColor);		
	CL_SpriteExplosion(org, 1.0);
	CL_ExplosionParticles(org);
	tracker->fxflags &= ~TEF_PROJ_FX;
}

//---------------------------------------------------------------------------------------------------------------------
// Ballista functions
//---------------------------------------------------------------------------------------------------------------------
void Proj_Ballista_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	CVector lOrg = track->lerp_origin;
	if (pointContents & MASK_WATER)
		return;

	AngleToVectors(track->current.angles,forward);
	V_AddLight(lOrg, tracker->lightSize, tracker->lightColor[0], tracker->lightColor[1], tracker->lightColor[2]);
	CVector vColor(0.5,0.5,1.0);
	CVector vAlpha(0.8,1.0,0);
	CVector vGravity(0,0,0);
	CL_GenerateComplexParticle2(lOrg + 3*tracker->scale*forward,	// emission origin
								forward,							// some direction thinggy
								vColor,				// color
								vAlpha,					// alpha (start, depletion, rand)
								2+1*crand(),						// scale
								2,									// count
								40,									// spread
								60,									// velocity
								PARTICLE_FIRE,						// particle type
								vGravity);					// gravity
	vColor.z = 0.5;
	vAlpha.y = 0.5;
	CL_GenerateComplexParticle2(lOrg + 3*tracker->scale*forward,	// emission origin
								forward,							// some direction thinggy
								vColor,			// color
								vAlpha,					// alpha (start, depletion, rand)
								1+0.5*crand(),						// scale
								3,									// count
								40,									// spread
								60,									// velocity
								PARTICLE_SMOKE,						// particle type
								vGravity);					// gravity

}

void Proj_Ballista_Die(trackParamData_t *trackData, int pointContents)
{
	CVector org = trackData->track->current.origin;
	CL_ExplosionParticles(org);
	trackData->tracker->fxflags &= ~TEF_PROJ_FX;
}

//---------------------------------------------------------------------------------------------------------------------
// Ion blaster functions
//---------------------------------------------------------------------------------------------------------------------
void Proj_Ion_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	// add some always cool complicated particles
	CVector org = track->lerp_origin;
	CVector ang;
	float scale = tracker->scale;
	float size = 8*(scale+frand());
	float alpha = tracker->scale2;
	float mod = 1.0+frand();

	float rot = tracker->ss_angle - 30;
	if (rot < 0) rot += 360;
	tracker->ss_angle = rot;

	// some cool lightning stuff!
	ang = track->current.angles;
	ang.roll += rot;
	ang.pitch -= rot;
	AngleToVectors(ang,forward,right,up);

	CVector back = org-size*forward;
	CVector left = org-size*right;
	CVector down = org-size*up;
	forward = org+size*forward;
	right=org+size*right;
	up=org+size*up;

	// draw the flare
	entity_t ent;
	memset(&ent,0,sizeof(ent));

	ent.model = cl.model_draw[(int)trackData->tracker->length];
	ent.angles = trackData->track->current.angles;
	ent.render_scale.Set(0.8,0.8,0.8);
	ent.alpha = 0.8f;
	ent.flags=SPR_ALPHACHANNEL;
	ent.frame=ent.oldframe=trackData->ent->frame;
	ent.origin = org;
	V_AddEntity(&ent);

	size = scale * 2;
	beam_AddLightning(org, up, tracker->lightColor, alpha, size, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,mod);
	beam_AddLightning(org, back, tracker->lightColor, alpha, size, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,mod);
	beam_AddLightning(org, down, tracker->lightColor, alpha, size, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,mod);
	beam_AddLightning(org, forward, tracker->lightColor, alpha, size, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,mod);
	// a cool green light
	float light = tracker->lightSize * (1.0 + 0.5*crand());
	V_AddLight(track->lerp_origin, light, tracker->lightColor[0], tracker->lightColor[1], tracker->lightColor[2]);

	// a simple particle trail
	CVector dir = track->prev.origin - track->current.origin;
	dir.Normalize();
	CVector vColor(0,1,0);
	CVector vAlpha(0.8,1,0);
	CVector vGravity(0,0,0);
	if (rand() & 1)
	CL_GenerateComplexParticle2(org,								// emission origin
								dir,								// some direction thinggy
								vColor,						// color
								vAlpha,					// alpha (start, depletion, rand)
								0.15+0.15*crand(),					// scale
								4,									// count
								5,									// spread
								450,									// velocity
								PARTICLE_BEAM_SPARKS,					// particle type
								vGravity);					// gravity
}

void Proj_Ion_Special(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	AngleToVectors(tracker->altangle,forward);
	tracker->altpos.pitch += 90;
	TrackEntFX_AnimExplosion(trackData, pointContents, 0.75+0.5*frand(),0.8, cl.model_draw[tracker->modelindex], 2,0);
	tracker->numframes &= ~TEF_PROJ_SPECIAL;				// sfx is a one-time thinggy
	// did we mention particle effects?
	CVector vAlpha(0.75,1.0,0);
	CVector vGravity(0,0,0);
	CL_GenerateComplexParticle2(trackData->track->current.origin,		// emission origin
								forward,							// some direction thinggy
								tracker->lightColor,				// color
								vAlpha,				// alpha (start, depletion, rand)
								0.25+0.25*frand(),							// scale
								10,									// count
								120,								// spread
								250,								// velocity
								PARTICLE_BEAM_SPARKS,					// particle type
								vGravity);					// gravity
}

void Proj_Ion_Die(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	AngleToVectors(tracker->altangle,forward);
	CVector vTemp(0,1,0);
	CVector vGravity(0,0,0);
	if ((pointContents & MASK_WATER) || tracker->Long2)
	{
		CL_PolyExplosion(track->lerp_origin, track->current.angles, cl.model_draw[tracker->Long2], 0.05, 2, 0.2f, 200, vTemp, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ORIENTED|SPR_ALPHACHANNEL);
		CL_PolyExplosion(track->lerp_origin, zero_vector, cl.model_draw[tracker->Long2], 0.05, 2, 0.2f, 200, vTemp, RF_FULLBRIGHT|RF_TRANSLUCENT|SPR_ALPHACHANNEL);
		// did we mention particle effects?
		vTemp.Set(0.75,0.9,0);
		CL_GenerateComplexParticle2(track->lerp_origin,					// emission origin
									forward,							// some direction thinggy
									tracker->lightColor,				// color
									vTemp,				// alpha (start, depletion, rand)
									0.75+0.5*frand(),							// scale
									25,									// count
									180,								// spread
									450,								// velocity
									PARTICLE_SPARKLE1,					// particle type
									vGravity);					// gravity
	}
	else											
	{
		TrackEntFX_AnimExplosion(trackData, pointContents, 0.25+0.25*frand(),0.1, cl.model_draw[tracker->modelindex], 5,0);
		// did we mention particle effects?
		vTemp.Set(0.75,0.9,0);
		CL_GenerateComplexParticle2(track->lerp_origin,					// emission origin
									forward,							// some direction thinggy
									tracker->lightColor,				// color
									vTemp,				// alpha (start, depletion, rand)
									0.5+0.5*frand(),							// scale
									25,									// count
									180,								// spread
									450,								// velocity
									PARTICLE_SPARKLE1,					// particle type
									vGravity);					// gravity
	}
	tracker->fxflags &= ~TEF_PROJ_FX;
}

//---------------------------------------------------------------------------------------------------------------------
// Venomous functions
//---------------------------------------------------------------------------------------------------------------------
void Proj_Venomous_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	// "strikingly similar" to the thunderskeet.

	if (!tracker->Long2)
	{
		// a nice little flutter
		tracker->curframetime -= cls.frametime;			// sync to time
		if(tracker->curframetime < 0)
		{
			tracker->curframetime += 0.05;
			tracker->ss_angle += 15 + 20*frand();
			if (tracker->ss_angle >= 360)
				tracker->ss_angle -= 360;
		}

		CVector ang = track->current.angles;
		ang.roll = tracker->ss_angle;
		AngleToVectors(ang,forward,right,up);
		trackData->track->lerp_origin += 4*up;
	}


	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
	N_Origin = track->lerp_origin;
	//See notes on Complex particle creation to understand this section 
	//Create the fire looking stuff
	Dir.Set(crand(),crand(),crand());
	Dir.Normalize();

	//Set the Color
	Color.Set(0.45f,0.85f,0.15f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.65f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-20.0);

	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.0f+frand()*1.0,3,360,50.0f,PARTICLE_CP4,Gravity);


	//Create the darker fire looking stuff
	Dir.Set(crand(),crand(),crand());

	//Set the Color
	Color.Set(0.35f,0.55f,0.05f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.35f,0.35f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,30.0);

	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.25f+(frand()*1.0),1,360,40.0f,PARTICLE_SMOKE,Gravity);
}

void Proj_Venomous_Special(trackParamData_t *trackData, int pointContents)
{
	TrackEntFX_AnimExplosion(trackData, pointContents, 0.45+0.45*frand(),0.9, cl.model_draw[trackData->tracker->modelindex], 9,100);
	trackData->tracker->numframes &= ~TEF_PROJ_SPECIAL;				// one time!
}

void Proj_Venomous_Die(trackParamData_t *trackData, int pointContents)
{
	if (pointContents & MASK_WATER)
	{
		CVector vDir(0,0,-1);
		CVector vColor(0.35f,0.55f,0.05f);
		CVector vAlpha(0.45f,0.1f,0.02f);
		CVector vGravity(0,0,-2);
		CL_GenerateComplexParticle2(trackData->track->lerp_origin,		// emission origin
									vDir,						// some direction thinggy
									vColor,			// color
									vAlpha,			// alpha (start, depletion, rand)
									2.25f+(frand()*1.0),				// scale
									75,									// count
									90,								// spread
									30,								// velocity
									PARTICLE_SMOKE,						// particle type
									vGravity);					// gravity
	}
	trackData->tracker->fxflags &= ~TEF_PROJ_FX;
}

//---------------------------------------------------------------------------------------------------------------------
// Trident functions
//---------------------------------------------------------------------------------------------------------------------
extern void *cl_mod_explosion[];
void CL_TridentTrail ( CVector &start, CVector &end );

void Proj_Trident_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	CVector org = track->lerp_origin;
	CVector Dir(crand(),crand(),crand());
	CVector Alpha(0.75f,0.65f,0.20f);
	CVector Gravity(0,0,30);

	float light = tracker->lightSize * (1 + 0.5*crand());
	V_AddLight(track->lerp_origin, light, tracker->lightColor.x, tracker->lightColor.y, tracker->lightColor.z);
	if (!(pointContents & MASK_WATER))
	{
		CL_TridentTrail ( track->lerp_origin, track->prev.origin );
		CL_GenerateComplexParticle2( org, Dir, tracker->lightColor, Alpha,1.25f+(frand()*1.0),1,360,40.0f,PARTICLE_SMOKE,Gravity);
	}
}

void Proj_Trident_Special(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	CVector vDir(0,0,1);
	CVector vColor(0.4,0.4,0.9);
	CVector vAlpha(0.7,0.35,0);
	CVector vGravity(0,0,0);
	CL_GenerateComplexParticle2(trackData->track->lerp_origin,					// emission origin
								vDir,						// some direction thinggy
								vColor,				// color
								vAlpha,				// alpha (start, depletion, rand)
								0.50 + 0.5*frand(),					// scale
								25,									// count
								360,								// spread
								650,								// velocity
								PARTICLE_BEAM_SPARKS,				// particle type
								vGravity);					// gravity

	trackData->tracker->numframes &= ~TEF_PROJ_SPECIAL;				// one time!
	CVector dir = trackData->track->current.origin - trackData->track->prev.origin;
	dir.Normalize();
	VectorToAngles(dir,dir);
	CL_PolyExplosion(trackData->track->lerp_origin, dir, cl_mod_explosion[2], 0.01, 3, 0.9f, 400, tracker->lightColor, SPR_ORIENTED|SPR_ALPHACHANNEL);
	CL_PolyExplosion(trackData->track->lerp_origin, dir, cl_mod_explosion[2], 0.1, 3, 0.8f, 400, tracker->lightColor, SPR_ORIENTED|SPR_ALPHACHANNEL);
}

void Proj_Trident_Die(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	tracker->fxflags &= ~TEF_PROJ_FX;
	CL_PolyExplosion(trackData->track->current.origin, tracker->altpos, cl_mod_explosion[2], 0.1, 1, 0.6f, 200, tracker->lightColor, SPR_ORIENTED|SPR_ALPHACHANNEL);
	CL_PolyExplosion(trackData->track->current.origin, tracker->altpos, cl_mod_explosion[2], 0.01, 0.9, 0.6f, 0, tracker->lightColor, SPR_ORIENTED|SPR_ALPHACHANNEL);
}

//---------------------------------------------------------------------------------------------------------------------
// Wisp functions
//---------------------------------------------------------------------------------------------------------------------

void Proj_Wisp_Launch(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	CVector *zaps[4];
	zaps[0] = &tracker->ss_ofs;
	zaps[1] = &tracker->ss_fwd;
	zaps[2] = &tracker->ss_rt;
	zaps[3] = &tracker->ss_up;
	for (int i = 0; i < 4; i++)
	{
		zaps[i]->pitch = 359*frand();
		zaps[i]->yaw = 359*frand();
	}

	tracker->altangle = trackData->angles;
}

void Proj_Wisp_Fly(trackParamData_t *trackData, int pointContents)
{
	GET_TRACKER;
	centity_t *track = trackData->track;
	int i;

	// set up some things to make accessing the arrays easier
	CVector *zaps[4];
	zaps[0] = &tracker->ss_ofs;
	zaps[1] = &tracker->ss_fwd;
	zaps[2] = &tracker->ss_rt;
	zaps[3] = &tracker->ss_up;

	tracker->curframetime -= cls.frametime;			// sync to time
	if(tracker->curframetime < 0)
	{
		if (tracker->altangle.y >= 360) tracker->altangle.y -= 360;
		tracker->curframetime += 0.1;
		// jitter the zap positions a bit.
		for (i = 0; i < 4; i++)
		{
			zaps[i]->pitch += 30*crand();
			zaps[i]->yaw += 30*crand();
			WRAP_VALUE(zaps[i]->pitch,0,360);
			WRAP_VALUE(zaps[i]->yaw,0,360);
		}
	}

	// draw the enviro-zaps!
    trace_t trace;
	CVector org = track->lerp_origin;
	for (i = 0; i < 4; i++)
	{
		AngleToVectors(*zaps[i],forward);
		trace = CL_TraceLine(org, org + forward * 100, tracker->src_index, MASK_SOLID);
		beam_AddLightning(track->lerp_origin, trace.endpos, tracker->lightColor, 0.6, 4, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,1);
	}

	// zap the targets
	CVector end;
	for (i = 0; i < 4; i++)
	{
		int entNum = 0;
		switch(i)
		{
		case 0:
			entNum = tracker->dstpos.x;
			break;

		case 1:
			entNum = tracker->dstpos.y;
			break;

		case 2:
			entNum = tracker->dstpos.z;
			break;

		case 3:
			entNum = tracker->Long2;
			break;
		}

		if (entNum)
		{
			if ( !(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE) && (entNum==cl.playernum+1) )
			{
				end = cl.refdef.vieworg;
				end.z -= 5;
			}
			else
			{
				end = cl_entities[entNum].lerp_origin;
			}
			beam_AddLightning(org, end, tracker->lightColor, 0.6, 6, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE,1);
		}

	}

	// add a cool eerie light
	V_AddLight(org, 100 + 40*frand(), tracker->lightColor.x, tracker->lightColor.y, tracker->lightColor.z);

	// draw the flare thingy
	if (tracker->modelindex)
	{
		entity_t ent;
		memset(&ent,0,sizeof(ent));
		ent.model = cl.model_draw[tracker->modelindex];
		ent.color = tracker->lightColor;
		ent.origin = org;
		ent.oldorigin = org;
		ent.render_scale.Set(0.75,0.75,0.75);
		ent.alpha = 0.6;
		ent.flags=SPR_ALPHACHANNEL;
		V_AddEntity(&ent);
	}
}

void Proj_Wisp_Special(trackParamData_t *trackData, int pointContents)
{

}

void Proj_Wisp_Die(trackParamData_t *trackData, int pointContents)
{

}

// the array of functions
projectile_fx_t projectile_fx_funcs[] =
{
	EMPTY_PROJECTILE_FX,
	{Proj_KCore_Launch, Proj_KCore_Fly, Proj_KCore_Special, Proj_KCore_Die},
	{NULL, Proj_Sidewinder_Fly, NULL, Proj_Sidewinder_Die},
	{NULL, Proj_Ballista_Fly, NULL, Proj_Ballista_Die},
	{NULL, Proj_Ion_Fly, Proj_Ion_Special, Proj_Ion_Die},
	{NULL, Proj_Venomous_Fly, Proj_Venomous_Special, Proj_Venomous_Die},
	{NULL, Proj_Trident_Fly, Proj_Trident_Special, Proj_Trident_Die},
	{Proj_Wisp_Launch, Proj_Wisp_Fly, Proj_Wisp_Special, Proj_Wisp_Die}
};

//---------------------------------------------------------------------------------------------------------------------
// Utility functions 
//---------------------------------------------------------------------------------------------------------------------
projectile_fx_s &GetProjectile_Fx(int idx)
{
	return ( (idx > TEF_PROJ_ID_FIRST) && (idx < TEF_PROJ_ID_MAX) ) ? projectile_fx_funcs[idx] : projectile_fx_funcs[0];
}

