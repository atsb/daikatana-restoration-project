// cl_tent.c -- client side temporary entities
#include "client.h"

#include "dk_cl_curve.h"
#include "dk_beams.h"
//#include "ClientEntityManager.h"

extern cvar_t *sv_violence;

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

void CL_GenerateComplexParticle2( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity);
void CL_GenerateComplexParticleRadius( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,CVector &Gravity,float radius);


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Dragon_Fireball
// Description:Creates Special Fx for the dragon's fireball
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void ArtFx_Dragon_Fireball(trackParamData_t *trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
	//See notes on Complex particle creation to understand this section 
	//Create the fire looking stuff
	Dir = trackData->tracker->dstpos;
	//Dir.Negate();
	
	N_Origin = trackData->owner->lerp_origin + (Dir*35.0f);
	//Set the Color
	Color.Set(0.55f,0.30,0.05f);
	//Color.Set(0.45f,0.85f,0.15f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.85f,0.45f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,0.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,6.25f+(frand()*6.50),4.0f,2.0,400.0f,PARTICLE_CP4,Gravity,10.0f);
	//CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.25f+frand()*1.0,3,180,60.0f,PARTICLE_CP4,Gravity);


	//Create the darker fire looking stuff
	//Set the Color
	Color.Set(0.40,0.15,0.05);
	//Color.Set(0.35f,0.35f,0.05f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.45f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,0.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.75f+(frand()*0.75),2,5,375.0f,PARTICLE_SMOKE,Gravity);
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Dragon_Breath
// Description:Creates Special Fx for Dragon Breath
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void ArtFx_Dragon_Breath(trackParamData_t *trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,vecHardPoint;
	
	entity_t entTemp;
    entTemp.origin = trackData->owner->lerp_origin;
	LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
    entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
	entTemp.frame = entTemp.oldframe = trackData->owner->current.frame;
	entTemp.backlerp = 0.0f;
	entTemp.render_scale = trackData->owner->current.render_scale;
	
	re.GetModelHardpoint("hr_muzzle", trackData->owner->current.frame, trackData->owner->current.frameInfo.endFrame, entTemp,vecHardPoint);

	N_Origin = vecHardPoint;

	
	//See notes on Complex particle creation to understand this section 
	
	
	//Set the initial direction of the smoke
	Dir = trackData->tracker->dstpos;
	
	//Set the Color
	Color = trackData->tracker->altpos;

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.75f,0.35f,0.05f);

	//Gravity settings
	// NSS[2/22/00]:Get the direction to the player
	Gravity = trackData->tracker->dstpos;
	
	//Get the reverse direction
	Gravity.Negate();
	Gravity.z = 0.0f;
	
	//increase the amount of gravity
	Gravity.Multiply(50.0f);
	
	//Make it so... 
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,4.25f+(frand()*6.50),4.0f,2.0,200.0f,PARTICLE_SMOKE,Gravity,1.0f);


	//Set the Color
	Color = trackData->tracker->altpos2;

	// NSS[2/23/00]:Add a bit of fluctuation to this
	Color.x += crand()*0.15f;
	Color.y += crand()*0.15f;

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.65f,0.25f,0.10f);

	Gravity.Zero();
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,3.0f+(frand()*8.0),10,2,250.0f,PARTICLE_CP4,Gravity);
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_FrogSpit
// Description:Creates Special Fx for frogspit
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void ArtFx_FrogSpit(trackParamData_t *trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity;
	N_Origin = trackData->owner->lerp_origin;
	//See notes on Complex particle creation to understand this section 
	//Create the fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color = trackData->tracker->altpos;
	//Color.Set(0.45f,0.85f,0.15f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.85f,0.45f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-400.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.25f+frand()*1.0,3,180,60.0f,PARTICLE_CP4,Gravity);


	//Create the darker fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color = trackData->tracker->altpos2;
	//Color.Set(0.35f,0.35f,0.05f);

	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.35f,0.65f,0.20f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-20.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,1.0f+(frand()*0.50),1,1,2.0f,PARTICLE_SMOKE,Gravity);

}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Fire
// Description:Creates fire stuff for the Fire Ring
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_Fire(trackParamData_t *trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,C1,C2,Angles,Forward;
	te_tracker_t *tracker;
	tracker = trackData->tracker;
	
	
	//Determine if we need to increment the angular offset
	if(!(cl.time % 7))
		tracker->length += 15;
	
	//Create first Circling flame thing
	VectorToAngles(tracker->altpos,Angles);
	Angles.x += tracker->length;
	Angles.y += tracker->length - 90;

	Angles.AngleToForwardVector(Forward);

	Forward = Forward *6;
	
	N_Origin = trackData->owner->current.origin + Forward;

	N_Origin.z +=20.0f;
    
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	//Set the Color
	Color.Set(0.85f,0.15f,0.05f);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.65f,2.35f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,20.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.1f+crand()*2.0,2.0f,90,40.0f,PARTICLE_FIRE,Gravity);

	//Create Second Circling flame thing
	VectorToAngles(tracker->altpos,Angles);
	Angles.x += tracker->length+180;
	Angles.y += tracker->length - 90;
	Angles.AngleToForwardVector(Forward);
	Forward = Forward * 6;
	N_Origin = trackData->owner->current.origin + Forward;
	N_Origin.z +=20.0f;
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticle2( N_Origin, Dir, Color, Alpha,2.1f+crand()*2.0,2.0f,90,40.0f,PARTICLE_FIRE,Gravity);

	C1.Set(0.95,-0.35,-1.0);
	C2.Set(-1.0,-1.0,-1.0);
	
	if(cl.time % 2)
		tracker->lightSize = (cl.time - (cl.time-1))*0.1f;
	Color = Color.Interpolate(C1,C2,tracker->lightSize);
	
	//Add the interpolated light thang!
	 V_AddLight(trackData->owner->current.origin,65, Color.x,Color.y, Color.z);

}



// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Undead
// Description:Creates the undead look thing
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_Undead(trackParamData_t* trackData)
{
	CVector Color,C1,C2;
	te_tracker_t *tracker;
	tracker = trackData->tracker;

	C1.Set(-1.0,0.75,0.15);
	C2.Set(0.35,-1.0,0.75);
	
	if(cl.time % 3)
		tracker->length = (cl.time - (cl.time-1))*0.1f;
	Color = Color.Interpolate(C1,C2,tracker->length);
	
	//Add the interpolated light thang!
	 V_AddLight(trackData->owner->current.origin, 65, Color.x,Color.y, Color.z);
}

#define FRAGTYPE_ROBOTIC			0x00000002   //             robotic gib, versus fleshy gib
#define FRAGTYPE_BONE				0x00000004   //				Bone Gib makes Bone Noises.

// ----------------------------------------------------------------------------
// Creator:<nss>
// Name:		ArtFx_BloodCloud
// Description:Creates the cloud of Blood for exploding Gibs
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_BloodCloud(trackParamData_t* trackData)
{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	return;
#endif

	if (sv_violence->value > 0.0)
		return;

	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,C1,C2,Angles,Forward;
	int CloudType;
	float Scale_Range,Scale_Modify;
	te_tracker_t *tracker;
	tracker = trackData->tracker;

	//The particles
	N_Origin = trackData->tracker->altpos;
	
	N_Origin.x += (crand()*trackData->tracker->altpos2.x)*0.85;
	N_Origin.y += (crand()*trackData->tracker->altpos2.y)*0.85;
	N_Origin.z += crand()*trackData->tracker->altpos2.z*0.50;

	//Gravity settings
	Gravity.Set(0.0,0.0,-100.0);
    
	
	if(trackData->tracker->altpos2.x > 100.0f)
	{
		Scale_Modify = 1.0f;	
	}
	else
	{
		Scale_Modify = trackData->tracker->altpos2.x/85.0f;
	}
	
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(crand(),crand(),crand());
	
	if(trackData->tracker->Long2 & FRAGTYPE_BONE)
	{
		Color.Set(.50f,0.45f,0.45f);
		CloudType = PARTICLE_SMOKE;
		Scale_Range = 90.0f*Scale_Modify;
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.85f,1.25f,0.10f);
		//Get busy Wit' It. Use the wonderful power of ComplexParticle...
		CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.50f+Scale_Range,3.0f,360,100.0f,CloudType,Gravity,10.0f);
	}
	else if(trackData->tracker->Long2 & FRAGTYPE_ROBOTIC)
	{
		Color.Set(-.50f,-0.45f,-0.45f);
		CloudType = PARTICLE_SMOKE;
		Scale_Range = 80.0f*Scale_Modify;
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.85f,1.25f,0.10f);
		CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.50f+Scale_Range,2.0f,360,100.0f,CloudType,Gravity,10.0f);

		Color.Set(.80f,0.55f,0.15f);
		CloudType = PARTICLE_SPARKS;
		Scale_Range = 10.0f*Scale_Modify;
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.65f,0.95f,0.10f);
		CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.50f+Scale_Range,3.0f,360,100.0f,CloudType,Gravity,10.0f);
	}
	else
	{
		Color.Set(0.20f,-0.65f,-0.65f);
		CloudType = PARTICLE_SMOKE;	
		Scale_Range = 1 + 80.0f*Scale_Modify;
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.45f,1.65,0.10f);
		//Get busy Wit' It. Use the wonderful power of ComplexParticle...
		CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.50f+Scale_Range,2.0f,360,100.0f,CloudType,Gravity,5.0f);

		//Color.Set(0.65f,0.55f,0.55f);
		Color.Set(0.20f,-1,-1);
		CloudType = PARTICLE_CP4;	
		Scale_Range = 5 + 80.0f*Scale_Modify;
		//Set the Alpha and Delpetion Rate
		Alpha.Set(0.55f,0.8f,0.10f);
		//Get busy Wit' It. Use the wonderful power of ComplexParticle...
		CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.50f+Scale_Range,3.0f,35,50.0f,CloudType,Gravity,5.0f);

	}
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ART_BUBOID_MELT
// Description:Creates cloud for the Buboid melt
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_Buboid_Melt(trackParamData_t* trackData)
{
	CVector N_Origin,Dir,Alpha,Gravity,Angles,Forward,Color;
	te_tracker_t *tracker;
	tracker = trackData->tracker;

	//The particles
	N_Origin = trackData->owner->current.origin;
	N_Origin.z -= 24.0f;
   
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(0.0f,0.0f,1.0f);
	
	//Set the Color
	Color.Set(0.25f,0.25,0.25f);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.45f,0.35f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-100.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,3.1f+crand()*3.0,2.0f,45,150.0f,PARTICLE_SMOKE,Gravity,5.0f);
}






// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_BlackBox
// Description:Creates the spurt for the Black Box
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_BlackBox(trackParamData_t* trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,C1,C2,Angles,Forward;
	te_tracker_t *tracker;
	tracker = trackData->tracker;

	//The Light
	C1.Set(-1.0,0.75,0.15);
	C2.Set(0.35,-1.0,0.75);
	
	if(!(cl.time % 3))
		tracker->length = (cl.time - (cl.time-1))*0.1f;
	Color = Color.Interpolate(C1,C2,tracker->length);
	
	//Add the interpolated light thang!
	 V_AddLight(trackData->owner->current.origin,165, Color.x,Color.y, Color.z);


	//The particles
	N_Origin = trackData->owner->current.origin;
	N_Origin.z +=10.0f;
    
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(0.0f,0.0f,1.0f);
	
	//Set the Color
	Color.Set(65.0f,-1.0,0.75f);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.65f,0.75f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-100.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,2.1f+crand()*2.0,6.0f,20,100.0f,PARTICLE_CP4,Gravity,5.0f);
}


// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Fountain
// Description:Creates the spurt for the Fountain.
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_Fountain(trackParamData_t* trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,C1,C2,Angles,Forward;
	te_tracker_t *tracker;
	tracker = trackData->tracker;

	//The Light
	C1.Set(0.0,0.0,0.75);
	C2.Set(0.05,0.0,0.25);
	
	if(!(cl.time % 3))
		tracker->length = (cl.time - (cl.time-1))*0.1f;
	Color = Color.Interpolate(C1,C2,tracker->length);
	
	//Add the interpolated light thang!
	 V_AddLight(trackData->owner->current.origin,95, Color.x,Color.y, Color.z);


	//The particles
	N_Origin = trackData->owner->current.origin;
	N_Origin.z +=06.0f;
    
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(0.0f,0.0f,1.0f);
	
	//Set the Color
	Color.Set(0.25f,0.45,0.65f);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.65f,0.45f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-100.0);
	
	//Get busy Wit' It. Use the wonderful power of ComplexParticle... weee.. 
	//Make the water shoot up..
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.4f+crand()*1.0,5.0f,40,70.0f,PARTICLE_CP4,Gravity,7.0f);

	//Add some mist... 
	Dir.Set(crand(),crand(),1.0f);
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,0.25f+crand()*1.0,1.0f,30,30.0f,PARTICLE_SMOKE,Gravity,7.0f);

	//You've got a fountain.
}

// ----------------------------------------------------------------------------
// NSS[1/14/00]:
// Name:	ArtFx_KageBounce	
// Description:Creates fire stuff for the Fire Ring
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_KageBounce(trackParamData_t* trackData)
{
	CVector N_Origin,O_Origin,Dir,Color,Alpha,Gravity,Angles,Forward;
	te_tracker_t *tracker;
	tracker = trackData->tracker;
	float Random;
	
	//Add the interpolated light thang!
	 V_AddLight(trackData->owner->current.origin,150, 0.85,0.85, 0.85);

	//The particles
	N_Origin = trackData->owner->lerp_origin;
    
	//See notes on Complex particle creation to understand this section (cl_tent.cpp)
	//Create some fire looking stuff
	Dir.Set(crand(),crand(),1.0f);
	
	//Set the Color
	Random=crand();
	Color.Set(0.85,0.85,0.85);
	
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.55f,0.85f,0.10f);

	//Gravity settings
	Gravity.Set(0.0,0.0,-100.0);
	
	//Create Kage's FX
	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,1.5f+Random*1.0,5.0f,15,200.0f,PARTICLE_CP4,Gravity,5.0f);

	//Add some mist... 
	Dir.Set(crand(),crand(),1.0f);

	//Set the Color
	Random=crand();
	Color.Set(0.65f,0.65f,0.65f);
	//Set the Alpha and Delpetion Rate
	Alpha.Set(0.45f,1.25f,0.10f);

	CL_GenerateComplexParticleRadius( N_Origin, Dir, Color, Alpha,1.0f + Random*1.0,5.0f,35,90.0f,PARTICLE_SMOKE,Gravity,5.0f);
}

// ----------------------------------------------------------------------------
// Creator:<nss>
//
// Name:		ArtFx_Fire
// Description:Creates fire stuff for the Fire Ring
// Input:trackParamData_t *trackData
// Output:<nothing>
// Note: 
// ----------------------------------------------------------------------------
void ArtFx_Lightning(trackParamData_t* trackData)
{
/*	float RN,Modulation,Alpha,Scale;
    entity_t    ent, entTemp;
	te_tracker_t    *tracker;	
    CVector         vecHardPoint1,vecHardPoint2,Dir,Color,N_Origin, Origin, Angles;  
    
	tracker = trackData->tracker;

	//Setup a temp entity and get the hardpoints
	trackData->owner->current.frame;
    entTemp.origin = trackData->owner->current.origin;
    entTemp.angles = trackData->owner->current.angles;
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
		RN = frand();
		
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
*/
}

// in cl_tent.cpp
void DK_Lightning(CVector &start, CVector &end, CVector color, float colDel, float alpha, float alpDel, float scale, float scDel,
				  float mod, float modDel)
{
	color.x += colDel * crand();
	color.y += colDel * crand();
	color.z += colDel * crand();
	color.Normalize();

	alpha += (alpha * alpDel * crand());
	scale += (scale * scDel * crand());
	mod += (mod * modDel * crand());

	beam_AddLightning(start, end, color, alpha, scale, BEAM_TEX_LIGHTNING, BEAMFLAG_ALPHAFADE, mod);
}

void DK_MovingFlare(CVector &start, CVector &end, float &frac, int modelindex, float alpha, float scale, short update)
{
	entity_t ent;
	memset(&ent,0,sizeof(ent));

	float newfrac;

	if (update)
	{
		newfrac = frac + 0.04;
	}
	else
		newfrac = frac;

	if (newfrac < 0)
	{
		if ((newfrac > -0.16) && (newfrac < -0.119))
		{
			DK_MovingFlareSnd(start);
		}

		frac = newfrac;
		return;
	}
	else if (newfrac > 1)
	{
		frac = -1;
		return;
	}

	ent.oldorigin = start.Interpolate(end,start,frac);

	frac = newfrac;

	ent.origin = start.Interpolate(end,start,newfrac);

	ent.model = cl.model_draw[modelindex];
	ent.render_scale.Set(scale,scale,scale);
	ent.alpha = alpha;
//	ent.color = CVector(0,0,1);
	ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;

	ent.angles = CVector(0,0,0);
	ent.angles.roll = 180*frand();
	V_AddEntity(&ent);

	ent.angles.pitch += 90;
	ent.angles.roll = 180*frand();
	V_AddEntity(&ent);

}

void DK_TipFlare(CVector &org, CVector &oldOrg, CVector &color, int modelindex, float alpha, float scale)
{
	entity_t ent;
	memset(&ent,0,sizeof(ent));
	ent.model = cl.model_draw[modelindex];
	ent.origin = org;
	ent.oldorigin = oldOrg;
	ent.render_scale.Set(scale,scale,scale);
	ent.alpha = alpha;
	ent.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
	ent.angles = CVector(0,0,0);
	ent.angles.roll = 180*frand();
	ent.color = color;
	V_AddEntity(&ent);

	ent.angles.yaw += 90;
	ent.angles.roll = 180*frand();
	V_AddEntity(&ent);

	oldOrg = org;
}

void DK_BaseFlare(CVector &org, CVector &oldOrg, CVector &ang, int modelindex, float alpha, float scale)
{
	entity_t ent;
	memset(&ent,0,sizeof(ent));
	ent.model = cl.model_draw[modelindex];
	ent.origin = org;
	ent.oldorigin = oldOrg;
	ent.render_scale.Set(scale,scale,scale);
	ent.alpha = alpha;
	ent.flags=SPR_ALPHACHANNEL;
	V_AddEntity(&ent);

	ent.flags |= SPR_ORIENTED;
	ent.angles = ang;
	V_AddEntity(&ent);

	oldOrg = org;
}

#define MK_EYESIZE				0.08
#define DK_MODULATION			0.5
#define DK_MODULATION_DELTA		0.9
#define DK_SCALE				0.8
#define DK_SCALE_DELTA			0.2

void ArtFX_Daikatana(trackParamData_t* trackData)
{
    te_tracker_t *tracker = trackData->tracker;

	float alpha = tracker->scale;
	float scale = tracker->scale2;

	// set up our frame timer
	short frameUpdate = 0;
	short doEyes = (tracker->curframe > 50) ? 1 : 0;
	tracker->curframetime -= cls.frametime;
	if(tracker->curframetime < 0)
	{
		if ((tracker->curframe++) > (75 + 50 * frand()))
			tracker->curframe = 0;

		frameUpdate = 1;
		tracker->curframetime += 0.05;
	}

	// get the hard points on the sword
	CVector hpHandle,hpTip,dir,angle;
	entity_t entTemp;
	memset(&entTemp,0,sizeof(entTemp));

    entTemp.origin = trackData->owner->lerp_origin;
	LerpAngles(trackData->owner->prev.angles,trackData->owner->current.angles,entTemp.angles,cl.lerpfrac);
    entTemp.model = cl.model_draw[trackData->owner->current.modelindex];
    entTemp.frame = trackData->owner->current.frame;
	entTemp.oldframe = trackData->owner->prev.frame;
	entTemp.backlerp = 1.0 - trackData->owner->fFrameLerp;
	entTemp.render_scale = trackData->owner->current.render_scale;
	re.GetModelHardpoint("sword2", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,hpTip);
	re.GetModelHardpoint("sword1", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,hpHandle);

	if (doEyes)
	{
		CVector eye1,eye2,forward,ang;
		ang = entTemp.angles;
		re.GetModelHardpoint("eye2", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,eye1);
		re.GetModelHardpoint("eye1", trackData->owner->current.frame, trackData->owner->prev.frame, entTemp,eye2);

		AngleToVectors(ang,forward);
		eye1 += forward;
		eye2 += forward;
		memset(&entTemp,0,sizeof(entTemp));
		entTemp.render_scale.Set(MK_EYESIZE,MK_EYESIZE,MK_EYESIZE);
		entTemp.angles = ang;
		ang.roll = 359*frand();

		entTemp.color = tracker->lightColor;
		entTemp.alpha = 0.6;
		entTemp.flags=SPR_ALPHACHANNEL|SPR_ORIENTED;
		entTemp.model = cl.model_draw[tracker->modelindex2];
		entTemp.origin = eye1;
		entTemp.oldorigin = eye1;
		V_AddEntity(&entTemp);

		entTemp.origin = eye2;
		entTemp.oldorigin = eye2;
		ang.roll = 359*frand();
		V_AddEntity(&entTemp);
	}

	dir = hpTip - hpHandle;
	dir.Normalize();
	VectorToAngles(dir,angle);

	// move the hp to the right place
	hpHandle -= 2.2*dir;
	hpTip += 2.2*dir;

	// find the origination points for the lightning
	CVector forward,right;
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

	float lAlpha,tAlpha,mAlpha,bAlpha;
	lAlpha = 0.6;
	tAlpha = 0.6;
	bAlpha = alpha;
	mAlpha = 0.6;

	// negative light technology
	V_AddLight(hpHandle,100+20*frand(),tracker->altpos2.x,tracker->altpos2.y,tracker->altpos2.z);
	V_AddLight(hpTip,100+20*frand(),tracker->altpos2.x,tracker->altpos2.y,tracker->altpos2.z);
	// start,end,color,colordel,alpha,alphadel,scale,scaledel,mod, moddel
	DK_Lightning(h1,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
	DK_Lightning(h2,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
	DK_Lightning(h3,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
	DK_Lightning(h4,hpTip,tracker->lightColor,0.4,lAlpha,0.2,DK_SCALE,DK_SCALE_DELTA,DK_MODULATION,DK_MODULATION_DELTA);
	DK_TipFlare(hpTip,tracker->ss_fwd,tracker->lightColor,tracker->modelindex,tAlpha,0.075+0.075*frand());				// ss_fwd holds old tip origin
	DK_MovingFlare(hpHandle,hpTip,tracker->length,tracker->modelindex,mAlpha,0.05+0.05*frand(), frameUpdate);
	DK_BaseFlare(hpHandle,tracker->ss_rt,angle,tracker->modelindex,bAlpha,scale);		// ss_rt holds old handle origin
}


void ArtFX_StavrosAmmo(trackParamData_t *trackData)
{
	te_tracker_t *tracker = trackData->tracker;
	if (!trackData->track->current.modelindex)
		return;

	if (tracker->curframe == 0)
	{
		tracker->curframe = 1;
		tracker->lightSize = 90;
	}

	tracker->curframetime -= cls.frametime;
	if(tracker->curframetime < 0)
	{
		tracker->curframetime += 0.05;
		float light = tracker->lightSize;
		light += tracker->curframe;
		if (light > 110)
		{
			tracker->curframe = -1;
		}
		else if (light < 90)
		{
			tracker->curframe = 1;
		}
		tracker->lightSize = light;
	}

	V_AddLight(trackData->origin,tracker->lightSize,1,-0.5,-0.5);

	CVector flamecolor(0.8 + 0.1*crand(),0.4 + 0.1*crand(), 0.2 + 0.1*crand());
	CL_GenerateComplexParticle2(trackData->origin,					// emission origin
								CVector(0,0,1),						// some direction thinggy
								flamecolor,				// color
								CVector(0.8,1.0,0),					// alpha (start, depletion, rand)
								3+2*crand(),						// scale
								2,									// count
								30,									// spread
								40,									// velocity
								PARTICLE_FIRE,						// particle type
								CVector(0,0,2));					// gravity

	flamecolor.Set(0.8 + 0.1*crand(),0.4 + 0.1*crand(), 0.2 + 0.1*crand());
	CL_GenerateComplexParticle2(trackData->origin,					// emission origin
								CVector(0,0,1),						// some direction thinggy
								flamecolor,				// color
								CVector(0.8,1.0,0),					// alpha (start, depletion, rand)
								3+2*crand(),						// scale
								2,									// count
								30,									// spread
								40,									// velocity
								PARTICLE_FIRE,						// particle type
								CVector(0,0,2));					// gravity

	CL_GenerateComplexParticle2(trackData->origin,					// emission origin
								CVector(0,0,1),						// some direction thinggy
								CVector(0.1,0.1,0.1),			// color
								CVector(0.6,0.5,0),					// alpha (start, depletion, rand)
								1+0.5*crand(),						// scale
								1,									// count
								50,									// spread
								40,									// velocity
								PARTICLE_SMOKE,						// particle type
								CVector(0,0,4));					// gravity
}
