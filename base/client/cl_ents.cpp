// cl_ents.c -- entity parsing and management
#include	"client.h"
#include	"dk_gce_main.h"
#include	"dk_beams.h"
//#include	"cl_inventory.h"
#include  "cl_sidekick.h"
//#include	"csv.h"

#include	<crtdbg.h>

//extern	void *cl_mod_powerscreen;
//extern	void *cl_mod_muzzleflash;
extern cvar_t *sv_violence;
cl_TempEventFunction_t   cl_TempEventFunctionList[TEMP_EVENT_COUNT];

foginfo_t	old_fog_state, global_fog_state;
int	last_player_contents;
int backframe[MAX_PARSE_ENTITIES];
int frontframe[MAX_PARSE_ENTITIES];

typedef struct
{
    int frame;
    bool exists;
} nextframe_t;

nextframe_t nextframe[MAX_PARSE_ENTITIES];

// delta angles for the head angles relative to the ent->angles
typedef struct 
{
    CVector headAngles;     // where the head is right now
    int entTarget;          // which entity the head wants to look at
} headdata_t;

headdata_t gHeadData[MAX_PARSE_ENTITIES];

/*
=========================================================================

FRAME PARSING

=========================================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	float	cl_autorotate;
static	int		cl_autoanim;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void	CL_AddTrackEnts(void);
void	CL_AddParticleVolumes(void);
float	CL_FrameUpdate (entity_state_t *estate);
//entity_t *V_AddHrEntity (centity_t *cl_ent, entity_state_t *state, entity_t *ent, int flags);

void CL_DiscusSparkles(centity_t *cent);
void CL_TridentTrail ( CVector &start, CVector &end );
void CL_StavrosAmmo ( CVector &start );
void CL_C4Beep ( entity_s &ent );
void CL_InitClientSideParticleVolume (entity_state_t *ent);
//void CL_IonTrail ( CVector &start, CVector &end);
void CL_SunflareSparks( CVector &origin, float scale, float count, int spread, float maxvel);
int CM_ContentsForLeaf( int leaf );
unsigned short CM_ContentsColor( int leaf );

///////////////////////////////////////////////////////////////////////////////
//	CL_FrameUpdate
//
//	updates an entity's frames
//	TODO:	play sounds when FRSTATE_PLAYSOUNDx is set
///////////////////////////////////////////////////////////////////////////////

float	CL_FrameUpdate (entity_state_t *estate)
{
	float			temp;
	int				add, new_frame;
	centity_t		*cent;
	float			frame_lerp, frameTime;
	frameInfo_t		*frameInfo = &estate->frameInfo;


    cent = &cl_entities[estate->number];

	if (frameInfo->frameFlags == 0 || frameInfo->frameFlags & FRAME_STATIC)
	{
        if (true==nextframe[estate->number].exists)
        {
            estate->frame = nextframe[estate->number].frame;
            nextframe[estate->number].exists = false;
        }
        else
            estate->frame = backframe[estate->number];
   		
        cent->prev.frame = estate->frame;
        backframe[estate->number] = estate->frame;
		return 0.0;
	}

	short orgFrame = estate->frame;
    estate->frame = frontframe[estate->number];

	// increment frames according to length of last frame
    frameTime = cls.frametime;

    // Scale time if we have move speed and frame distance to correct skating
    // And don't do this if we're transitioning to a new animation
    
    // calc the entity speed based on the server data (10 fps data)

    if (cl_antiskate->value)
    {
        float entitySpeed = VectorDistance(cent->current.origin, cent->current.old_origin)/0.1f;

        if ((entitySpeed>=0) && (frameInfo->modelAnimSpeed>0) && (false==nextframe[estate->number].exists))
            frameTime *= (entitySpeed*estate->render_scale.y/frameInfo->modelAnimSpeed);
    }

    frameInfo->next_frameTime = frameInfo->next_frameTime + frameTime;

	frame_lerp = frameInfo->next_frameTime / frameInfo->frameTime;	

	if (frameInfo->next_frameTime > frameInfo->frameTime)
	{
		//frameInfo->frameInc = 1;
		temp = floor (frameInfo->next_frameTime / frameInfo->frameTime);
		add = (int)(frameInfo->frameInc * temp);

		if (estate->number == showsync->value)
			Com_Printf ("frame: %d  next_frameTime: %.2f frameTime: %.2f add: %i\n", estate->frame, frameInfo->next_frameTime, frameInfo->frameTime, add);

		if (add == 0)
			return frame_lerp;

		new_frame = estate->frame + add;
        
        // save off the backframe
        backframe[estate->number] = frontframe[estate->number];

		if ( frameInfo->frameInc > 0 )
		{
            if (true==nextframe[estate->number].exists)
            {
                // new animation
	    		cent->prev.frame = estate->frame;

                estate->frame = nextframe[estate->number].frame;
                nextframe[estate->number].exists = false;
                frame_lerp -= add;
            }
            else
            {
    			///////////////////////////////////////////////////////////////////////
    			//	set/clear FRSTATE_LAST flag if on last frame
    			///////////////////////////////////////////////////////////////////////
    
    			if (frameInfo->endFrame <= new_frame && frameInfo->endFrame > estate->frame)
    			{
    				frameInfo->frameState |= FRSTATE_LAST;
    			}
    
    			///////////////////////////////////////////////////////////////////////
    			//	set FRSTATE_PLAYSOUND flags if correct frame
    			///////////////////////////////////////////////////////////////////////
    
    			//	FIXME:	actually play sounds here??
    			if (frameInfo->sound1Frame >= estate->frame && frameInfo->sound1Frame < new_frame)
    			{
   					frameInfo->frameState |= FRSTATE_PLAYSOUND1;
    			}
    			if (frameInfo->sound2Frame >= estate->frame &&  frameInfo->sound2Frame < new_frame)
    			{
    				frameInfo->frameState |= FRSTATE_PLAYSOUND2;
    			}
    
    			if ( frameInfo->nAttackFrame1 >= estate->frame && frameInfo->nAttackFrame1 < new_frame )
    			{
    				frameInfo->frameState |= FRSTATE_PLAYATTACK1;
    			}
    			if ( frameInfo->nAttackFrame2 >= estate->frame && frameInfo->nAttackFrame2 < new_frame )
    			{
    				frameInfo->frameState |= FRSTATE_PLAYATTACK2;
    			}
                
    			cent->prev.frame = estate->frame;
    
    			frame_lerp -= add;
    			estate->frame += add;
    
    			// are we currently looping within a larger sequence?
    			if (frameInfo->frameFlags & FRAME_REPEAT)
    			{
    				// determine if sequence needs to loop back to loopStart
    				if (estate->frame > frameInfo->endLoop)
    				{
    					estate->frame = frameInfo->startLoop;
    					frameInfo->loopCount--;
    					// if count is <= 0, stop the looping of this partial sequence
    					if (frameInfo->loopCount <= 0)
    					{
    						frameInfo->frameFlags -= FRAME_REPEAT;
    						frameInfo->startLoop = frameInfo->startFrame;
    						frameInfo->endLoop = frameInfo->endFrame;
    					}
    				}
    			}
    			// going through an entire sequence
    			else
    			{
    				// determine if sequence needs to loop back to start
    				if (estate->frame > frameInfo->endFrame)
    				{
    					if (frameInfo->frameFlags & FRAME_ONCE)
    					{
    						// we've gone through once, so set frames to static (no updates)
    						frameInfo->frameState |= FRSTATE_STOPPED;
    						estate->frame = frameInfo->endFrame;
    					}
    					else
    					{
    						//	looping through entire sequence
    						estate->frame = frameInfo->startFrame;
    					}
    				}
    			}
            }
		}
		else
		{
            if (true==nextframe[estate->number].exists)
            {
                // new animation
	    		cent->prev.frame = estate->frame;

                estate->frame = nextframe[estate->number].frame;
                nextframe[estate->number].exists = false;
                frame_lerp += add;
            }
            else
            {
    			if ( new_frame <= frameInfo->endFrame && estate->frame > frameInfo->endFrame )
    			{
    				frameInfo->frameState |= FRSTATE_LAST;
    			}
    
    			cent->prev.frame = estate->frame;
    
    			frame_lerp += add;
    			estate->frame += add;
    
    			// determine if sequence needs to loop back to start
    			if (estate->frame < frameInfo->endFrame)
    			{
    				if (frameInfo->frameFlags & FRAME_ONCE)
    				{
    					// we've gone through once, so set frames to static (no updates)
    					frameInfo->frameState |= FRSTATE_STOPPED;
    					estate->frame = frameInfo->endFrame;
    				}
    				else
    				{
    					//	looping through entire sequence
    					estate->frame = frameInfo->startFrame;
    				}
    			}
            }
		}

        frontframe[estate->number]=estate->frame;
		frameInfo->next_frameTime = frameInfo->next_frameTime - (temp * frameInfo->frameTime);
	}

	if (estate->number == showsync->value)
		Com_Printf ("lerping: %f\n", frame_lerp);

	return	frame_lerp;
}


/*
=================
CL_CalcLookAngles

Calcs the delta angles from entAngles for the vector from srcPos to dstPos
Returns true if the target is within the pitch and yaw limits, and false otherwise
=================
*/


bool CL_CalcLookAngles(CVector& srcPos, CVector& dstPos, CVector& entAngles, CVector& deltaAngles)
{
    CVector angles;
    CVector lookdir = dstPos - srcPos;
    float   yawlimit = 60.0f,
            pitchlimit = 40.0f;
    bool    fInRange = true;

    lookdir.VectorToAngles(angles);
    
    deltaAngles = angles-entAngles;

    // make sure yaw is in the range -180, 180
    while (deltaAngles.yaw>180.0f)
        deltaAngles.yaw-=360.0f;
    while (deltaAngles.yaw<-180.0f)
        deltaAngles.yaw+=360.0f;

    // limit the yaw
    if (deltaAngles.yaw>yawlimit)
    {   
        deltaAngles.yaw = yawlimit;
        fInRange = false;
    }
    else if (deltaAngles.yaw<-yawlimit)
    {
        deltaAngles.yaw = -yawlimit;
        fInRange = false;
    }

    // make sure pitch is in the range -90, 90
    while (deltaAngles.pitch>90.0f)
        deltaAngles.pitch-=360.0f;
    while (deltaAngles.pitch<-90.0f)
        deltaAngles.pitch+=360.0f;
    
    // limit the pitch
    if (deltaAngles.pitch>pitchlimit)
    {
        deltaAngles.pitch = pitchlimit;
        fInRange = false;
    }
    else if (deltaAngles.pitch<-pitchlimit)
    {
        deltaAngles.pitch = -pitchlimit;
        fInRange = false;
    }

    return fInRange;
}

/*
=================
CL_HeadUpdate

Updates the head angles of anybody who wants it
=================
*/

void CL_HeadUpdate(entity_state_t *estate)
{
    CVector targetAngles;
    float dAngle = 200.0f;

    headdata_t &headData = gHeadData[estate->number];
    CVector &headAngles = headData.headAngles;

    if (0==headData.entTarget)
    {
        // not looking at anyone -- target is straight ahead
        targetAngles.Zero();
    }
    else
    {
        // looking at someone
        CVector srcPos, dstPos, entAngles;

        // set up the source pos            
        srcPos = cl_entities[estate->number].current.origin;
        srcPos.z += 32.0f;  // put the source up at the head

        // set up the dest pos
        dstPos = cl_entities[headData.entTarget].current.origin;
        dstPos.z += 32.0f;  // put the target up at the head

        entAngles = cl_entities[estate->number].current.angles;
        
        // get the target delta angles
        if (false==CL_CalcLookAngles(srcPos, dstPos, entAngles, targetAngles))
        {
            targetAngles.Zero();    // target is out of range
        }
    }
    
    // now move our current angles toward the target angles
    if (headAngles.pitch>targetAngles.pitch)
    {
        headAngles.pitch-=dAngle*cls.frametime;
        
        if (headAngles.pitch<targetAngles.pitch) 
            headAngles.pitch = targetAngles.pitch;
    }
    else if (headAngles.pitch<targetAngles.pitch)
    {
        headAngles.pitch+=dAngle*cls.frametime;
        if (headAngles.pitch>targetAngles.pitch) 
            headAngles.pitch = targetAngles.pitch;
    }
    
    if (headAngles.yaw>targetAngles.yaw)
    {
        headAngles.yaw-=dAngle*cls.frametime;
        
        if (headAngles.yaw<targetAngles.yaw) 
            headAngles.yaw = targetAngles.yaw;
    }
    else if (headAngles.yaw<targetAngles.yaw)
    {
        headAngles.yaw+=dAngle*cls.frametime;
        if (headAngles.yaw>targetAngles.yaw) 
            headAngles.yaw = targetAngles.yaw;
    }
    
    if (headAngles.roll>targetAngles.roll)
    {
        headAngles.roll-=dAngle*cls.frametime;
        
        if (headAngles.roll<targetAngles.roll) 
            headAngles.roll = targetAngles.roll;
    }
    else if (headAngles.roll<targetAngles.roll)
    {
        headAngles.roll+=dAngle*cls.frametime;
        if (headAngles.roll>targetAngles.roll) 
            headAngles.roll = targetAngles.roll;
    }
}

/*
=================
CL_ParseEntityBits

Returns the entity number and the header bits
=================
*/
int	bitcounts[64];	/// just for protocol profiling
int CL_ParseEntityBits (unsigned *bits, unsigned *bits2)
{
	unsigned	b, total, total2 = 0;
	int			i;
	int			number;

	total = MSG_ReadByte (&net_message);
	if (total & U_MOREBITS1)
	{
		b = MSG_ReadByte (&net_message);
		total |= b<<8;
	}
	if (total & U_MOREBITS2)
	{
		b = MSG_ReadByte (&net_message);
		total |= b<<16;
	}
	if (total & U_MOREBITS3)
	{
		b = MSG_ReadByte (&net_message);
		total |= b<<24;
	}

	if (total & U_MOREBITS4)
	{
		//	Nelno:	read the next 4 bit-field bytes, if necessary
		total2 = MSG_ReadByte (&net_message);

		if (total2 & U_MOREBITS5)
		{
			b = MSG_ReadByte (&net_message);
			total2 |= b << 8;
		}
		if (total2 & U_MOREBITS6)
		{
			b = MSG_ReadByte (&net_message);
			total2 |= b << 16;
		}
		if (total2 & U_MOREBITS7)
		{
			b = MSG_ReadByte (&net_message);
			total2 |= b << 24;
		}
	}

	// count the bits for net profiling
	for (i=0 ; i<32 ; i++)
	{
		if (total&(1<<i))
			bitcounts[i]++;

		//	Nelno:	count second dword's bits, too
		if (total2 & (1 << i))
			bitcounts [i + 32]++;
	}

	if (total & U_NUMBER16)
		number = MSG_ReadShort (&net_message);
	else
		number = MSG_ReadByte (&net_message);

	*bits = total;
	*bits2 = total2;

	return number;
}


/*
==================
CL_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
void CL_ParseDelta (entity_state_t *from, entity_state_t *to, int number, int bits, int bits2)
{
	int cluster_bits, cluster_bits2;
//	int	hr_bits;
    int frame_bits = 0, frame_third_byte = 0;

	// set everything to the state we are delta'ing from
	*to = *from;

//	to->old_origin = from->origin;
	to->number = number;

	if (bits & U_MODEL)
		to->modelindex = MSG_ReadShort (&net_message);

	//	Nelno:	animation is now client-side
/*	
	if (bits & U_FRAME8)
		to->frame = MSG_ReadByte (&net_message);
	if (bits & U_FRAME16)
		to->frame = MSG_ReadShort (&net_message);
*/

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		to->skinnum = MSG_ReadLong(&net_message);
	else if (bits & U_SKIN8)
		to->skinnum = MSG_ReadByte(&net_message);
	else if (bits & U_SKIN16)
		to->skinnum = MSG_ReadShort(&net_message);

	if ( (bits & (U_EFFECTS8|U_EFFECTS16)) == (U_EFFECTS8|U_EFFECTS16) )
		to->effects = MSG_ReadLong(&net_message);
	else if (bits & U_EFFECTS8)
		to->effects = MSG_ReadByte(&net_message);
	else if (bits & U_EFFECTS16)
		to->effects = MSG_ReadShort(&net_message);

	if ( (bits2 & (U_EFFECTS2_8|U_EFFECTS2_16)) == (U_EFFECTS2_8|U_EFFECTS2_16) )
		to->effects2 = MSG_ReadLong(&net_message);
	else if (bits2 & U_EFFECTS2_8)
		to->effects2 = MSG_ReadByte(&net_message);
	else if (bits2 & U_EFFECTS2_16)
		to->effects2 = MSG_ReadShort(&net_message);

	if ( (bits2 & (U_FLAGS_8|U_FLAGS_16)) == (U_FLAGS_8|U_FLAGS_16) )
		to->flags = MSG_ReadLong(&net_message);
	else 
	if (bits2 & U_FLAGS_8)
		to->flags = MSG_ReadByte(&net_message);
	else 
	if (bits2 & U_FLAGS_16)
		to->flags = MSG_ReadShort(&net_message);

	if ( (bits & (U_RENDERFX8|U_RENDERFX16)) == (U_RENDERFX8|U_RENDERFX16) )
		to->renderfx = MSG_ReadLong(&net_message);
	else 
	if (bits & U_RENDERFX8)
		to->renderfx = MSG_ReadByte(&net_message);
	else 
	if (bits & U_RENDERFX16)
		to->renderfx = MSG_ReadShort(&net_message);
/* *** NUKE THIS SOON ***
	if (bits & U_ENTITY_ID)
		to->entityID = MSG_ReadShort (&net_message);
*/
	if (bits & U_ORIGIN1)
	{
		to->origin.x = MSG_ReadCoord (&net_message);
	}
	if (bits & U_ORIGIN2)
	{
		to->origin.y = MSG_ReadCoord (&net_message);
	}
	if (bits & U_ORIGIN3)
	{
		to->origin.z = MSG_ReadCoord (&net_message);
	}
		
	if (bits & U_ANGLE1)
	{
		to->angles.x = MSG_ReadFloat(&net_message);
	}
	if (bits & U_ANGLE2)
	{
		to->angles.y = MSG_ReadFloat(&net_message);
	}
	if (bits & U_ANGLE3)
	{
		to->angles.z = MSG_ReadFloat(&net_message);
	}

	if (bits & U_OLDORIGIN)
		MSG_ReadPos (&net_message, to->old_origin);

	if (bits & U_SOUND)
		to->sound = MSG_ReadShort (&net_message);

	// get volume if present
	if (bits2 & U_SOUNDINFO)	
	{
		to->volume = ((float)MSG_ReadByte(&net_message)) * ONE_DIV_255; // Logic[6/24/99]: / 255.0f;
		to->dist_min = ((int)MSG_ReadByte(&net_message))<<3;
		to->dist_max = ((int)MSG_ReadByte(&net_message))<<5;
		to->snd_flags = MSG_ReadByte(&net_message);
	}

	if (bits & U_EVENT)
		to->event = MSG_ReadByte (&net_message);
	else
		to->event = 0;

	if (bits & U_SOLID)
		to->solid = MSG_ReadShort (&net_message);

	if (bits & U_RENDERX)
	{
		to->render_scale.x = MSG_ReadFloat (&net_message);
		to->render_scale.y = MSG_ReadFloat (&net_message);
		to->render_scale.z = MSG_ReadFloat (&net_message);
	}

   if (bits & U_BBOX)
   {
	   to->iflags |= IF_CL_REALBBOX;	//	so clipping knows to use the real bounding box!
	   to->mins.x = -MSG_ReadByte (&net_message);
	   to->mins.y = -MSG_ReadByte (&net_message);
	   to->mins.z = -MSG_ReadByte (&net_message);
	   to->maxs.x = MSG_ReadByte (&net_message);
	   to->maxs.y = MSG_ReadByte (&net_message);
	   to->maxs.z = MSG_ReadByte (&net_message);
   }

	if (bits & U_ALPHA)
	{
		to->alpha = MSG_ReadFloat(&net_message);
	}

	//	this should NEVER be sent except for RF_SNOW and RF_RAIN!
	if (bits2 & U_MINSMAXS)
	{
		to->mins.x = MSG_ReadFloat (&net_message);
		to->mins.y = MSG_ReadFloat (&net_message);
		to->mins.z = MSG_ReadFloat (&net_message);
		to->maxs.x = MSG_ReadFloat (&net_message);
		to->maxs.y = MSG_ReadFloat (&net_message);
		to->maxs.z = MSG_ReadFloat (&net_message);

//		if (to->renderfx & (RF_SNOW|RF_RAIN|RF_DRIP))
		if (to->renderfx & RF_PARTICLEVOLUME)
		{
			//&&& amw
			// add this entity to the client side particle volume list
//			Com_DPrintf ("client: added particle volume entity %d\n", number);
			CL_InitClientSideParticleVolume( to );
		}
	}

	if (bits2 & U_BEAM_DIR)
		MSG_ReadDir (&net_message, to->mins);

	///////////////////////////////////////////////////////////////////////////
	//	read animation info if present
	///////////////////////////////////////////////////////////////////////////

	if (bits & (U_FRAME8 | U_FRAME16))
	{
		if (bits & U_FRAME16)
		{
			frame_bits = MSG_ReadShort (&net_message);

			if (frame_bits & FRSENT_THIRDBYTE)
			{
				frame_third_byte = MSG_ReadByte (&net_message);
				frame_third_byte = frame_third_byte << 16;
//				frame_bits = (frame_third_byte << 16) | frame_bits;	// stupid  frame_bits has all its high bits set
			}
		}
		else
			frame_bits = MSG_ReadByte (&net_message);

		if (frame_bits & FRSENT_SYNC8)
		{
			if (to->number == showsync->value)
				Com_Printf ("client: synced from %i", to->frame);
			to->frame = MSG_ReadByte (&net_message);

			if (to->number == showsync->value)
				Com_Printf (" to %i\n", to->frame);
		}
		else if (frame_bits & FRSENT_SYNC16)
		{
			if (to->number == showsync->value)
				Com_Printf ("client: synced from %i", to->frame);
			to->frame = MSG_ReadShort (&net_message);
			if (to->number == showsync->value)
				Com_Printf (" to %i\n", to->frame);
		}

		if (frame_bits & FRSENT_FLAGS16)
		{
			to->frameInfo.frameFlags = MSG_ReadShort (&net_message);

			//	set correct frame rate based on flags
			if (to->frameInfo.frameFlags & FRAME_FPS10)
				to->frameInfo.frameTime = FRAMETIME_FPS10;
			else if (to->frameInfo.frameFlags & FRAME_FPS30)
				to->frameInfo.frameTime = FRAMETIME_FPS30;
			else	// 20 fps
				to->frameInfo.frameTime = FRAMETIME_FPS20;
		}

		if (frame_bits & FRSENT_STARTFRAME8)
			to->frameInfo.startFrame = MSG_ReadByte (&net_message);
		else if (frame_bits & FRSENT_STARTFRAME16)
			to->frameInfo.startFrame = MSG_ReadShort (&net_message);

		if (frame_bits & FRSENT_ENDFRAME8)
			to->frameInfo.endFrame = MSG_ReadByte (&net_message);
		else if (frame_bits & FRSENT_ENDFRAME16)
			to->frameInfo.endFrame = MSG_ReadShort (&net_message);

		if (frame_bits & FRSENT_SOUND1FRAME8)
			to->frameInfo.sound1Frame = MSG_ReadByte (&net_message);
		else if (frame_bits & FRSENT_SOUND1FRAME16)
			to->frameInfo.sound1Frame = MSG_ReadShort (&net_message);

		if (frame_bits & FRSENT_SOUND2FRAME8)
			to->frameInfo.sound2Frame = MSG_ReadByte (&net_message);
		else if (frame_bits & FRSENT_SOUND2FRAME16)
			to->frameInfo.sound2Frame = MSG_ReadShort (&net_message);

		if (frame_bits & FRSENT_SOUND1INDEX8)
			to->frameInfo.sound1Index = MSG_ReadByte (&net_message);

		if (frame_bits & FRSENT_SOUND2INDEX8)
			to->frameInfo.sound2Index = MSG_ReadByte (&net_message);

		if (frame_bits & FRSENT_FRAMEINC8)
			to->frameInfo.frameInc = MSG_ReadByte (&net_message);

		if (frame_bits & FRSENT_STARTLOOP16)
			to->frameInfo.startLoop = MSG_ReadShort (&net_message);

		if (frame_third_byte & FRSENT_ENDLOOP16)
			to->frameInfo.endLoop = MSG_ReadShort (&net_message);

		if (frame_third_byte & FRSENT_LOOPCOUNT8)
			to->frameInfo.loopCount = MSG_ReadByte (&net_message);

		// amw: can we scale this down and send less data?
		if (frame_third_byte & FRSENT_ANIMSPEED)
			to->frameInfo.modelAnimSpeed = MSG_ReadFloat (&net_message);
	}

	if (bits & U_MODEL2)
		to->modelindex2 = MSG_ReadLong (&net_message);
	if (bits & U_MODEL3)
		to->modelindex3 = MSG_ReadShort (&net_message);
	if (bits & U_MODEL4)
		to->modelindex4 = MSG_ReadShort (&net_message);

	///////////////////////////////////////////////////////////////////////////
	//	read cluster info if present
	///////////////////////////////////////////////////////////////////////////
	if (bits2 & U_NUMCLUSTERS)
	{
		to->numClusters = (int)MSG_ReadShort(&net_message);
//		_ASSERTE(to->numClusters <= MAX_MESH_CLUSTERS);
	}

	if (bits & U_CLUSTERDATA)
	{
		cluster_bits = MSG_ReadByte(&net_message);

		// did the 3rd cluster defined have data that has changed?
		if (cluster_bits & CDSENT_3RDCLUSTER)
			cluster_bits2 = MSG_ReadByte(&net_message);

		// read all info in
		if (cluster_bits & CDSENT_ALL_BITS)
		{
			for (int i=0;i<to->numClusters;i++)
			{
				to->cDef[i].clusterIdx = MSG_ReadByte(&net_message);
				to->cDef[i].angles.x = MSG_ReadAngle(&net_message);
				to->cDef[i].angles.y = MSG_ReadAngle(&net_message);
				to->cDef[i].angles.z = MSG_ReadAngle(&net_message);
			}
		}
		else
		{
			// first cluster
			if (cluster_bits & CDSENT_ANGLES_0_X)
				to->cDef[0].angles.x = MSG_ReadAngle(&net_message);
			if (cluster_bits & CDSENT_ANGLES_0_Y)
				to->cDef[0].angles.y = MSG_ReadAngle(&net_message);
			if (cluster_bits & CDSENT_ANGLES_0_Z)
				to->cDef[0].angles.z = MSG_ReadAngle(&net_message);
			// second cluster
			if (cluster_bits & CDSENT_ANGLES_1_X)
				to->cDef[1].angles.x = MSG_ReadAngle(&net_message); 
			if (cluster_bits & CDSENT_ANGLES_1_Y)
				to->cDef[1].angles.y = MSG_ReadAngle(&net_message); 
			if (cluster_bits & CDSENT_ANGLES_1_Z)
				to->cDef[1].angles.z = MSG_ReadAngle(&net_message); 
			// third cluster
			if (cluster_bits & CDSENT_3RDCLUSTER)
			{
				if (cluster_bits2 & CDSENT_ANGLES_2_X)
					to->cDef[2].angles.x = MSG_ReadAngle(&net_message); 
				if (cluster_bits2 & CDSENT_ANGLES_2_Y)
					to->cDef[2].angles.y = MSG_ReadAngle(&net_message); 
				if (cluster_bits2 & CDSENT_ANGLES_2_Z)
					to->cDef[2].angles.z = MSG_ReadAngle(&net_message); 
			}
		}
	}


//	///////////////////////////////////////////////////////////////////////////
//	//	read hierarchical info if present
//	///////////////////////////////////////////////////////////////////////////
//
//	if (bits2 & U_HIERARCHICAL)
//	{
//		_ASSERTE(0);	// make sure this shit isn't being used
//		hr_bits = MSG_ReadByte (&net_message);
//
//		if (hr_bits & HRSENT_PARENTNUM8)
//			to->hrInfo.parentNumber = MSG_ReadByte (&net_message);
//		else if (hr_bits & HRSENT_PARENTNUM16)
//			to->hrInfo.parentNumber = MSG_ReadShort (&net_message);
//
//		if (hr_bits & HRSENT_HRFLAGS8)
//			to->hrInfo.hrFlags = MSG_ReadByte (&net_message);
//		else if (hr_bits & HRSENT_HRFLAGS16)
//			to->hrInfo.hrFlags = MSG_ReadShort (&net_message);
//
//		if (hr_bits & HRSENT_HRTYPE)
//			to->hrInfo.hrType = MSG_ReadByte (&net_message);
//
//		if (hr_bits & HRSENT_SURFINDEX)
//			to->hrInfo.surfaceIndex = MSG_ReadByte (&net_message);
//
//		if (hr_bits & HRSENT_VERTINDEX8)
//			to->hrInfo.vertexIndex = MSG_ReadByte (&net_message);
//		else if (hr_bits & HRSENT_VERTINDEX16)
//			to->hrInfo.vertexIndex = MSG_ReadShort (&net_message);
//
//		//	any change at all in any of the hierarchical properties will cause
//		//	the current hierarchical state of the entity to be cleared and the
//		//	new state set up
//		to->hrInfo.hrFlags |= HRF_RESET;
//	}
//
//	if ((from->hrInfo.hrType == HR_SURFACE && to->hrInfo.hrType != HR_SURFACE))
//	{
//		_ASSERTE(0);	// make sure this shit isn't being used
//		Com_Printf ("Hierarchical entity being re-used\n");
//		if (to->modelindex != from->modelindex)
//		{
//			//	surface based hierarchical models CANNOT have their model changed!!
//			Sys_Error ("Modelindex changed on HR_SURFACE!\n");
//			to->hrInfo.hrFlags |= HRF_REMOVE | HRF_RESET;
//		}
//	}
///*	 //%%% AMW 12.21.98 - commented this out.. this did nasty things to the hierarchical models ALL the time
//	if (from->entityID != to->entityID)
//	{
//		if (to->hrInfo.hrType != HR_SURFACE)
//			to->hrInfo.hrFlags |= HRF_RESET | HRF_REMOVE;
//	}
//*/
	if (bits2 & U_ANGLEDELTA_X)
	{
		to->angle_delta.x = MSG_ReadAngle (&net_message);
	}
	if (bits2 & U_ANGLEDELTA_Y)
	{
		to->angle_delta.y = MSG_ReadAngle (&net_message);
	}
	if (bits2 & U_ANGLEDELTA_Z)
	{
		to->angle_delta.z = MSG_ReadAngle (&net_message);
	}
	if (bits2 & U_ORIGINHACK)
	{
		to->origin_hack.x = MSG_ReadCoord (&net_message);
		to->origin_hack.y = MSG_ReadCoord (&net_message);
		to->origin_hack.z = MSG_ReadCoord (&net_message);
	}
	if( bits2 & U_COLOR )
	{
		to->color.x = MSG_ReadFloat (&net_message);
		to->color.y = MSG_ReadFloat (&net_message);
		to->color.z = MSG_ReadFloat (&net_message);
	}

    // JAR 990227 - If new animation information received, update anim backlog
    if(frame_bits & FRSENT_SYNC8 || frame_bits & FRSENT_SYNC16)
    {
        nextframe[to->number].frame = to->frame;
        nextframe[to->number].exists = true;
    }
        

}

/*
==================
CL_DeltaEntity

Parses deltas from the given base and adds the resulting entity
to the current frame
==================
*/
void CL_DeltaEntity (frame_t *frame, int newnum, entity_state_t *old, int bits, int bits2)
{
	centity_t		*ent;
	entity_state_t	*state;
    CVector         oldorg;
	
    ent = &cl_entities[newnum];
    
	state = &cl_parse_entities[cl.parse_entities & (MAX_PARSE_ENTITIES-1)];
    oldorg = old->old_origin;
    
	cl.parse_entities++;
	frame->num_entities++;

    CL_ParseDelta (old, state, newnum, bits, bits2);

	// some data changes will force no lerping
	if (state->modelindex != ent->current.modelindex		|| 
//		state->modelindex2 != ent->current.modelindex2		||	// amw: this is not necessary for our code
//		state->modelindex3 != ent->current.modelindex3		||
//		state->modelindex4 != ent->current.modelindex4		|| 
		abs(state->origin.x - ent->current.origin.x) > 512	|| 
		abs(state->origin.y - ent->current.origin.y) > 512	|| 
		abs(state->origin.z - ent->current.origin.z) > 512	|| 
		state->event == EV_PLAYER_TELEPORT					|| 
		state->event == EV_ENTITY_FORCEMOVE)
	{
		ent->serverframe = -99;
	}

    if (ent->serverframe != cl.frame.serverframe - 1)
	{	// wasn't in last update, so initialize some things
        ent->trailcount = 1024;		// for diminishing rocket / grenade trails
		// duplicate the current state so lerping doesn't hurt anything
        ent->prev = *state;
        ent->prev.origin = state->old_origin;
        ent->lerp_origin = state->old_origin;
	}
	else
	{	// shuffle the last state to previous
		ent->prev = ent->current;
	}

	ent->serverframe = cl.frame.serverframe;
	ent->current = *state;
}

/*
==================
CL_ParsePacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/

void	CL_FreeTrackEntity (int track_entity_index);

void CL_ParsePacketEntities (frame_t *oldframe, frame_t *newframe)
{
	int			newnum;
	int			bits, bits2;
	entity_state_t	*oldstate;
	int			oldindex, oldnum;

	newframe->parse_entities = cl.parse_entities;
	newframe->num_entities = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	if (!oldframe)
		oldnum = 99999;
	else
	{
		if (oldindex >= oldframe->num_entities)
			oldnum = 99999;
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}

	while (1)
	{
		//	Nelno:	get the number of the current delta entity 
		newnum = CL_ParseEntityBits ((unsigned int *)&bits, (unsigned int *)&bits2);
		if (newnum >= MAX_EDICTS)
			Com_Error (ERR_DROP,"CL_ParsePacketEntities: bad number:%i", newnum);

		if (net_message.readcount > net_message.cursize)
			Com_Error (ERR_DROP,"CL_ParsePacketEntities: end of message");

		if (!newnum)
			break;

		while (oldnum < newnum)
		{	
			// one or more entities from the old packet are unchanged
			//	Nelno:	so skip through client entities until we find the
			//			entity with a number that matches newnum, or we go
			//			past the number of entities in the last frame
			if (cl_shownet->value == 3)
				Com_Printf ("   unchanged: %i\n", oldnum);

			//	copy from old frame directly to new frame
			CL_DeltaEntity (newframe, oldnum, oldstate, 0, 0);
			
			oldindex++;

			if (oldindex >= oldframe->num_entities)
				oldnum = 99999;
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
		}

		if (bits & U_REMOVE)
		{	// the entity present in oldframe is not in the current frame
			if (cl_shownet->value == 3)
				Com_Printf ("   remove: %i\n", newnum);
			if (oldnum != newnum)
				Com_DPrintf ("U_REMOVE: oldnum != newnum\n");

			oldindex++;

			if (oldindex >= oldframe->num_entities)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}

//			//	Nelno:	make sure hierarchical entites get reset when freed
//			if (bits & U_FREED)
//			{
//				//	Nelno:	find a good place to clear this, first
//				//	cl_entities [newnum].flags |= CFL_FREED;
//
//				if (cl_entities [newnum].prev.hrInfo.hrType != 0)
//					cl_entities [newnum].current.hrInfo.hrFlags = HRF_RESET | HRF_REMOVE;
//		
//			}

			//	Nelno:	make sure that this entity gets removed from track entity list!!
			CL_FreeTrackEntity (newnum);

			cl_entities [newnum].flags &= ~CFL_RESPAWN;
			
			continue;
		}

		if (oldnum == newnum)
		{	// delta from previous state
			if (cl_shownet->value == 3)
				Com_Printf ("   delta: %i\n", newnum);
			CL_DeltaEntity (newframe, newnum, oldstate, bits, bits2);

			oldindex++;

			if (oldindex >= oldframe->num_entities)
				oldnum = 99999;
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if (oldnum > newnum)
		{	// delta from baseline
			if (cl_shownet->value == 3)
				Com_Printf ("   baseline: %i\n", newnum);
			CL_DeltaEntity (newframe, newnum, &cl_entities[newnum].baseline, bits, bits2);
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while (oldnum != 99999)
	{	// one or more entities from the old packet are unchanged
		if (cl_shownet->value == 3)
			Com_Printf ("   unchanged: %i\n", oldnum);
		CL_DeltaEntity (newframe, oldnum, oldstate, 0, 0);
		
		oldindex++;

		if (oldindex >= oldframe->num_entities)
			oldnum = 99999;
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}
}



/*
===================
CL_ParsePlayerstate
===================
*/
void CL_ParsePlayerstate (frame_t *oldframe, frame_t *newframe)
{
	int			flags;
	player_state_t	*state;
	int			i;
	int			statbits;

	state = &newframe->playerstate;

	// clear to old value before delta parsing
	if (oldframe)
		*state = oldframe->playerstate;
	else
		memset (state, 0, sizeof(*state));

	flags = MSG_ReadShort (&net_message);

	//
	// parse the pmove_state_t
	//
	if (flags & PS_M_TYPE)
		state->pmove.pm_type = (pmtype_t)MSG_ReadByte (&net_message);

	if (flags & PS_M_ORIGIN)
	{
		state->pmove.origin[0] = MSG_ReadShort (&net_message);
		state->pmove.origin[1] = MSG_ReadShort (&net_message);
		state->pmove.origin[2] = MSG_ReadShort (&net_message);
	}

	if (flags & PS_M_VELOCITY)
	{
		state->pmove.velocity[0] = MSG_ReadShort (&net_message);
		state->pmove.velocity[1] = MSG_ReadShort (&net_message);
		state->pmove.velocity[2] = MSG_ReadShort (&net_message);
	}

	if (flags & PS_M_TIME)
		state->pmove.pm_time = MSG_ReadByte (&net_message);

	if (flags & PS_M_FLAGS)
		state->pmove.pm_flags = MSG_ReadShort (&net_message);

	if (flags & PS_M_GRAVITY)
		state->pmove.gravity = MSG_ReadShort (&net_message);

	if (flags & PS_M_DELTA_ANGLES)
	{
		state->pmove.delta_angles[0] = MSG_ReadShort (&net_message);
		state->pmove.delta_angles[1] = MSG_ReadShort (&net_message);
		state->pmove.delta_angles[2] = MSG_ReadShort (&net_message);
	}

	if (cl.attractloop)
		state->pmove.pm_type = PM_FREEZE;		// demo playback

	//
	// parse the rest of the player_state_t
	//
	if (flags & PS_VIEWOFFSET)
	{
		state->viewoffset.x = MSG_ReadChar (&net_message) * 0.25;
		state->viewoffset.y = MSG_ReadChar (&net_message) * 0.25;
		state->viewoffset.z = MSG_ReadChar (&net_message) * 0.25;
	}

	if (flags & PS_VIEWANGLES)
	{
		state->viewangles.x = MSG_ReadAngle16 (&net_message);
		state->viewangles.y = MSG_ReadAngle16 (&net_message);
		state->viewangles.z = MSG_ReadAngle16 (&net_message);
	}

	if (flags & PS_KICKANGLES)
	{
		state->kick_angles.x = MSG_ReadChar (&net_message) * 0.25;
		state->kick_angles.y = MSG_ReadChar (&net_message) * 0.25;
		state->kick_angles.z = MSG_ReadChar (&net_message) * 0.25;
	}

/*
	if (flags & PS_WEAPONINDEX)
	{
		state->gunindex = MSG_ReadByte (&net_message);
	}

	if (flags & PS_WEAPONFRAME)
	{
		state->gunframe = MSG_ReadByte (&net_message);
		state->gunoffset[0] = MSG_ReadChar (&net_message)*0.25;
		state->gunoffset[1] = MSG_ReadChar (&net_message)*0.25;
		state->gunoffset[2] = MSG_ReadChar (&net_message)*0.25;
		state->gunangles[0] = MSG_ReadChar (&net_message)*0.25;
		state->gunangles[1] = MSG_ReadChar (&net_message)*0.25;
		state->gunangles[2] = MSG_ReadChar (&net_message)*0.25;
	}
*/

	if (flags & PS_BLEND)
	{
		state->blend[0] = MSG_ReadByte (&net_message)/255.0;
		state->blend[1] = MSG_ReadByte (&net_message)/255.0;
		state->blend[2] = MSG_ReadByte (&net_message)/255.0;
		state->blend[3] = MSG_ReadByte (&net_message)/255.0;
	}

	if (flags & PS_FOV)
		state->fov = MSG_ReadByte (&net_message);

	if (flags & PS_RDFLAGS)
		state->rdflags = MSG_ReadShort (&net_message);

	if (flags & PS_VIEWENTITY)
		state->view_entity = MSG_ReadShort (&net_message);

	if (flags & PS_INPUTENTITY)
		state->input_entity = MSG_ReadShort (&net_message);

	// parse stats
	statbits = MSG_ReadLong (&net_message);
	for (i=0 ; i<MAX_STATS ; i++)
		if (statbits & (1<<i) )
			state->stats[i] = MSG_ReadLong(&net_message);

}


/*
==================
CL_FireEntityEvents

==================
*/
void CL_FireEntityEvents (frame_t *frame)
{
	entity_state_t		*s1;
	int					pnum, num;

	for (pnum = 0 ; pnum<frame->num_entities ; pnum++)
	{
		num = (frame->parse_entities + pnum)&(MAX_PARSE_ENTITIES-1);
		s1 = &cl_parse_entities[num];
		if (s1->event)
			CL_EntityEvent (s1);

		// EF_TELEPORTER acts like an event, but is not cleared each frame
		if (s1->effects & EF_TELEPORTER)
			CL_TeleporterParticles (s1);
	}
}


/*
================
CL_ParseFrame
================
*/
void CL_ParseFrame (void)
{
	int			cmd;
	int			len;
	frame_t		*old;

	memset (&cl.frame, 0, sizeof(cl.frame));

	cl.frame.serverframe = MSG_ReadLong (&net_message);
	cl.frame.deltaframe = MSG_ReadLong (&net_message);
	cl.frame.servertime = cl.frame.serverframe * CL_FRAME_MILLISECONDS;

	// BIG HACK to let old demos continue to work
	if (cls.serverProtocol != 26)
		cl.surpressCount = MSG_ReadByte (&net_message);

	if (cl_shownet->value == 3)
		Com_Printf ("   frame:%i  delta:%i\n", cl.frame.serverframe,
		cl.frame.deltaframe);

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	if (cl.frame.deltaframe <= 0)
	{
		cl.frame.valid = true;		// uncompressed frame
		old = NULL;
		cls.demowaiting = false;	// we can start recording now
	}
	else
	{
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];
		if (!old->valid)
		{	// should never happen
			Com_Printf ("Delta from invalid frame (not supposed to happen!).\n");
		}
		if (old->serverframe != cl.frame.deltaframe)
		{	// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Com_DPrintf ("Delta frame too old.\n");
		}
		else if (cl.parse_entities - old->parse_entities > MAX_PARSE_ENTITIES-128)
		{
			Com_DPrintf ("Delta parse_entities too old.\n");
		}
		else
			cl.frame.valid = true;	// valid delta parse
	}

	// clamp time 
	if (cl.time > cl.frame.servertime)
		cl.time = cl.frame.servertime;
	else if (cl.time < cl.frame.servertime - CL_FRAME_MILLISECONDS)
		cl.time = cl.frame.servertime - CL_FRAME_MILLISECONDS;

	// read areabits
	len = MSG_ReadByte (&net_message);
	MSG_ReadData (&net_message, &cl.frame.areabits, len);

	// read playerinfo
	cmd = MSG_ReadByte (&net_message);
#ifdef _DEBUG
	SHOWNET(svc_strings[cmd]);
#endif /* DEBUG */
	if (cmd != svc_playerinfo)
		Com_Error (ERR_DROP, "CL_ParseFrame: not playerinfo");
	CL_ParsePlayerstate (old, &cl.frame);

	// read packet entities
	cmd = MSG_ReadByte (&net_message);
#ifdef _DEBUG
	SHOWNET(svc_strings[cmd]);
#endif /* DEBUG */
	if (cmd != svc_packetentities)
		Com_Error (ERR_DROP, "CL_ParseFrame: not packetentities");
	
    CL_ParsePacketEntities (old, &cl.frame);
    

#if 0  // fuck this shit
	//	Nelno:	now that all entities have been parsed for this frame,
	//			go through and set up hierarhcical data for them
	//			We might be able to do this as each entity is parsed,
	//			with no need for this second loop, however, for the
	//			first implementation we'll leave this here to be sure
	clhr_InitForFrame (&cl.frame);
#endif 

	// save the frame off in the backup array for later delta comparisons
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;

	if (cl.frame.valid)
	{
		// getting a valid frame message ends the connection process
		if (cls.state != ca_active)
		{
			cls.state = ca_active;
			cl.force_refdef = true;
			cl.predicted_origin.x = cl.frame.playerstate.pmove.origin[0] * 0.125;
			cl.predicted_origin.y = cl.frame.playerstate.pmove.origin[1] * 0.125;
			cl.predicted_origin.z = cl.frame.playerstate.pmove.origin[2] * 0.125;
			cl.predicted_angles = cl.frame.playerstate.viewangles;
			if (cls.disable_servercount != cl.servercount
				&& cl.refresh_prepped)
				SCR_EndLoadingPlaque ();	// get rid of loading plaque
		}
		cl.sound_prepped = true;	// can start mixing ambient sounds

		// fire entity events
		CL_FireEntityEvents (&cl.frame);
		CL_CheckPredictionError ();
	}
}

/*
==========================================================================

INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS

==========================================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	CL_ItemRespawn
//
///////////////////////////////////////////////////////////////////////////////

//#define	EF_RESPAWN	0x01000000

void	CL_ItemRespawn (centity_t *cent, entity_state_t *state)
{
	CVector	maxs(32,32, 8);
	CVector	mins(-32, -32, -24);

	if (!(cent->flags & CFL_RESPAWN))
	{
		//	EF_RESPAWN flags was just sent down to client for this entity
		//	so initialize some data
		cent->flags |= CFL_RESPAWN;
		
		state->renderfx |= RF_TRANSLUCENT;
		
		state->alpha = 0.1;
		cent->current.alpha = state->alpha;		
		cent->prev.alpha = state->alpha;

//		state->render_scale [0] = 0.1;
//		state->render_scale [1] = 0.1;
//		state->render_scale [2] = 0.1;

//		cent->current.render_scale = state->render_scale;
//		cent->prev.render_scale = state->render_scale;

		//	spawn a little vortex here
		//CL_SpiralParticles (state->origin,mins, maxs, 3000);
		CL_SpiralParticles (cent->lerp_origin,mins, maxs, 3000);
	}
	else
	{
		//	entity is still respawning -- when it is done respawning,
		//	we clear the prev.effects CFL_RESPAWN flag
		state->renderfx |= RF_TRANSLUCENT;

		state->alpha = cent->prev.alpha * 1.1;
		cent->current.alpha = state->alpha;

//		state->render_scale = cent->prev.render_scale * 1.1;

		//	update current in case so that it gets copied over to previous after this
//		cent->current.render_scale = state->render_scale;

		if (state->alpha > 1.0 || state->render_scale [0] > 1.0)
		{
			//	Nelno:	restore alpha from baseline so that real translucent entities
			//			don't get their renderfx and alpa screwed up
			state->alpha = cent->baseline.alpha;
			state->renderfx = cent->baseline.renderfx;
			state->render_scale.Set(1.0, 1.0, 1.0);

			state->effects &= ~EF_RESPAWN;
			cent->prev.effects &= ~EF_RESPAWN;
			cent->current.effects &= ~EF_RESPAWN;

			cent->flags &= ~CFL_RESPAWN;
		}
	}
}

extern void CL_BoltTrail( CVector &start, CVector &end, centity_t *old);
extern void	CL_BlueSparkTrail ( CVector &start, CVector &end );
//extern void	CL_SparkleTrail ( CVector &start, CVector &end, centity_t *old);
//extern void CL_SparkleTrail_OPT ( CVector &start, CVector &end, centity_t *old, int speed, int type);
//extern void	CL_NovaHit ( CVector &org, CVector &angles);
//extern void	clhr_TriCenter ( CVector &v1, CVector &v2, CVector &v3, CVector &v_out);
//extern void	clhr_TriNormal ( CVector &v1, CVector &v2, CVector &v3, CVector &out);
//extern void	clhr_VecToAngles ( CVector &vec, CVector &angles);

///////////////////////////////////////////////////////////////////////////////
//	CL_AddMuzzleFlashToSurface
//
///////////////////////////////////////////////////////////////////////////////

void	CL_AddMuzzleFlashToSurface (entity_t *ent, char *surfName, entity_state_t *s1)
{
	entity_t	m_ent;

	//	set up muzzle flash using 32 bit model
	memset (&m_ent, 0, sizeof (m_ent));
	//<nss> we have to get the pointer to the model not just the index
	m_ent.model = cl.model_draw[s1->modelindex3];
	m_ent.angles = ent->angles;
	m_ent.angles.pitch += 180.0f;
	m_ent.oldorigin = ent->oldorigin;
	m_ent.render_scale.Set( 8.0 + crand()*6.0, 13.0 + crand()* 3.0, 10.0 + crand()* 5.0);
	m_ent.alpha = 1.0;
	m_ent.flags = RF_TRANSLUCENT;

	CVector hardPt, oldOrigin;
	hardPt.Set(0,0,0);
	re.GetModelHardpoint(surfName, ent->frame, ent->oldframe, *ent, hardPt);
	m_ent.origin = hardPt;
	m_ent.angles.roll = crand()* 90.0f;
	V_AddEntity(&m_ent);
}

unsigned int	nSuperflyModelindex, nMikikoModelIndex, nMikikoFlyModelIndex;
void CL_GetSidekickModelIndicies()
{
	nMikikoModelIndex = CL_ModelIndex( "models/global/m_mikiko.dkm" );
	nSuperflyModelindex = CL_ModelIndex( "models/global/m_superfly.dkm" );
	nMikikoFlyModelIndex = CL_ModelIndex( "models/global/m_mikikofly.dkm");
}

/*
===============
CL_AddPacketEntities

===============
*/
void CL_AddPacketEntities (frame_t *frame)
{
	entity_t			  ent;
	entity_state_t *s1;
	int					    pnum;
	centity_t			 *cent;
	clientinfo_t	 *ci;
	int					    effects, renderfx,effects2;
	float				    framelerp;
	float				    ang1, ang2;
  int             old_sidekicks_active;

	memset (&ent, 0, sizeof(ent));


  old_sidekicks_active = cl.sidekicks_active; // track previous mode
	cl.sidekicks_active  = 0;                   // reset current

	CL_GetSidekickModelIndicies();
	for (pnum = 0 ; pnum<frame->num_entities ; pnum++)
	{
		s1 = &cl_parse_entities [(frame->parse_entities+pnum)&(MAX_PARSE_ENTITIES-1)];
		cent = &cl_entities [s1->number];
	
		// mdm99.05.10 hack in the origin
		cent->origin_hack = s1->origin_hack;

		//	Nelno:	animate entity
		framelerp = CL_FrameUpdate (s1);
		effects = s1->effects;
        effects2= s1->effects2;


		if (effects & EF_RESPAWN || cent->flags & CFL_RESPAWN)
		{
			CL_ItemRespawn (cent, s1);
			renderfx = s1->renderfx & (~RF_GLOW);
		}
		else
			renderfx = s1->renderfx;

		//	do this here so that no effects associated with a NODRAW entity get sent
		//	to the renderer
		if (renderfx & RF_NODRAW)
			continue;

		if (effects2 & EF2_BLOODY_GIB)
		{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
			continue;
#endif
			if (sv_violence->value > 0.0)
				continue;
		}

		if (!(cl.refdef.rdflags & RDF_COOP))
		{
			if( s1->modelindex )
			{
				if( ( s1->modelindex == nMikikoModelIndex ) && ( nMikikoModelIndex != 0 ) )
				{
					cl.sidekicks_active |= SIDEKICK_MIKIKO;
				}
				else if( (( s1->modelindex == nSuperflyModelindex ) && ( nSuperflyModelindex != 0 )) ||
						 (( s1->modelindex == nMikikoFlyModelIndex ) && ( nMikikoFlyModelIndex != 0 )))
				{
					cl.sidekicks_active |= SIDEKICK_SUPERFLY;
				}
			}
		}

		ent.color = cent->current.color;

		// set frame
		if (effects & EF_ANIM01)
			ent.frame = cl_autoanim & 1;
		else if (effects & EF_ANIM23)
			ent.frame = 2 + (cl_autoanim & 1);
		else if (effects & EF_ANIM_ALL)
			ent.frame = cl_autoanim;
		else if (effects & EF_ANIM_ALLFAST)
			ent.frame = cl.time / CL_FRAME_MILLISECONDS;
		else
			ent.frame = s1->frame;
		
		ent.oldframe = backframe[s1->number];
		ent.backlerp = 1.0 - framelerp;

		cl_entities [s1->number].fFrameLerp = framelerp;

		if (renderfx & RF_TRACKENT)
		{
	         // added so track ents would FUCKING LERP!!!!!!!!!!!!!
			cl_entities [s1->number].current.mins.x = ent.backlerp;      // hack: backlerp put here for AddTrackEnts()
			cl_entities [s1->number].current.mins.y = ent.oldframe;      // hack: backframe put here for AddTrackEnts()
			cl_entities [s1->number].current.mins.z = ent.frame;         // hack: frame put here for AddTrackEnts()
            
			if (renderfx & (RF_FRAMELERP))//|RF_BEAM))
			{	// step origin discretely, because the frames
				// do the animation properly
				ent.origin = cent->current.origin;
				ent.oldorigin = cent->current.old_origin;
			}
			else
			{	// interpolate origin
    			ent.origin.x = cent->prev.origin.x + cl.lerpfrac * (cent->current.origin.x - cent->prev.origin.x);
				ent.oldorigin.x = ent.origin.x;

				ent.origin.y = cent->prev.origin.y + cl.lerpfrac * (cent->current.origin.y - cent->prev.origin.y);
				ent.oldorigin.y = ent.origin.y;

				ent.origin.z = cent->prev.origin.z + cl.lerpfrac * (cent->current.origin.z - cent->prev.origin.z);
				ent.oldorigin.z = ent.origin.z;
			}
			ent.model = cl.model_draw[s1->modelindex];
			ent.origin = s1->origin;
			if (renderfx & RF_MUZZLEFLASH)
			{
				V_AddLight (cent->current.origin, 120, 1.0, 0.75, 0.0);

				//	try to add to all possible surfaces
				CL_AddMuzzleFlashToSurface (&ent, "hr_muzzle", s1);
			}			
			continue;
		}

		if (renderfx & (RF_BEAM | RF_LIGHTRAY))
		{
			float	radius, endRadius;
			//	FIXME:	make RF_BEAM interpolate!  requires some work on the
			//	server side.  Maybe pass end pos via render scale instead?

			//	beginning and end radius are both encoded in frame!
			
#pragma message("TODO Frog: remove radius hack after frame info is fixed")			
			endRadius = 2 >> 8;//s1->frame >> 8;
			radius = 2 & 0xff;// (s1->frame & 0xff);

/*
			if (s1->frame < 0)             // -frame is fractional, 1000 = 1
				radius = -(float)s1->frame / 1000.0;
			else
				radius = s1->frame;  // non-neg is normal little frame ...

			endRadius = s1->render_scale [0];
*/

			CVector ident = vec3_identity;

			if (renderfx & RF_LIGHTRAY)
				beam_AddLaser (cent->current.origin, cent->current.old_origin, ident, s1->alpha, radius, endRadius, BEAM_TEX_NONE, BEAMFLAG_ALPHAFADE);
      		else if (renderfx & RF_TRANSLUCENT)
				beam_AddLaser (cent->current.origin, cent->current.old_origin, ident, s1->alpha, radius, endRadius, BEAM_TEX_LASER, 0);
      		else
				beam_AddLaser (cent->current.origin, cent->current.old_origin, ident, 1.0, radius, endRadius, BEAM_TEX_LASER, 0);

			//	RF_BEAMS no longer get added as entities!
			continue;
		}
    
        if(renderfx & RF_IN_NITRO)
        {
            // scale alpha up
            if(!(cl.frame.serverframe & 3))
            {
                if(s1->alpha < 1.0)
                    s1->alpha+=0.01;
                else
                    s1->alpha = 1.0f;
            }
            ent.alpha = s1->alpha;

        }
        // Logic[5/18/99]: smooth scaling
        if(effects2 & EF2_RENDER_SCALE)
        {
			// cek[11-24-99] causing really crazy problems with hard point calculations.
/*            if(VITA_SCALE < s1->render_scale.x)            {
                // FUCKING FLOATING POINT INACCURATE BULLSHIT!!!
                // when 1.0 + 0.02 = 1.0199999809265 as far as the system is concerned, 
                // hacks like THIS are necessary. this is so wrong!
                if(fabs(VITA_SCALE - s1->render_scale.x) > 0.01)
                {
                    s1->render_scale.x -= 0.02f;
                    s1->render_scale.y -= 0.02f;
                    s1->render_scale.z -= 0.02f;
                    ent.render_scale = s1->render_scale;
                }
            } else if (s1->render_scale.x < VITA_SCALE ) {
                // DITTO!!!!
                if(fabs(VITA_SCALE - s1->render_scale.x) > 0.01)
                {
                    s1->render_scale.x += 0.02f;
                    s1->render_scale.y += 0.02f;
                    s1->render_scale.z += 0.02f;
                    ent.render_scale = s1->render_scale;
                }
            } else {
                s1->render_scale.Set(1.0, 1.0, 1.0);
                ent.render_scale = s1->render_scale;
            }*/
        }

		if (renderfx & (RF_FRAMELERP))//|RF_BEAM))
		{	// step origin discretely, because the frames
			// do the animation properly
			ent.origin = cent->current.origin;
			ent.oldorigin = cent->current.old_origin;
		}
		else
		{	// interpolate origin
    	    ent.origin.x = cent->prev.origin.x + cl.lerpfrac * (cent->current.origin.x - cent->prev.origin.x);
		    ent.oldorigin.x = ent.origin.x;

		    ent.origin.y = cent->prev.origin.y + cl.lerpfrac * (cent->current.origin.y - cent->prev.origin.y);
            ent.oldorigin.y = ent.origin.y;

			ent.origin.z = cent->prev.origin.z + cl.lerpfrac * (cent->current.origin.z - cent->prev.origin.z);
			ent.oldorigin.z = ent.origin.z;
		}
		//&&& rain and snow should be taken out of here...
		//  Shawn:  Added for particle volumes
//		if (renderfx & (RF_SNOW | RF_RAIN | RF_BEAM_MOVING | RF_SPOTLIGHT))
/*
		// provide the renderer with 
		if (renderfx & RF_SPOTLIGHT)
		{
			ent.mins[0] = s1->mins[0];
			ent.mins[1] = s1->mins[1];
			ent.mins[2] = s1->mins[2];
			ent.maxs[0] = s1->maxs[0];
			ent.maxs[1] = s1->maxs[1];
			ent.maxs[2] = s1->maxs[2];
		}
*/
		ent.render_scale = s1->render_scale;

		//	Nelno:	don't process light flares here
		if (renderfx & RF_LIGHTFLARE)
		{
			// go ahead and update the lerp_origin since we've already calculated the new position
			cent->lerp_origin = ent.origin;
			
			if( s1->render_scale.x == 0 
				|| s1->render_scale.y == 0
				|| s1->render_scale.z == 0 )
			{
				ent.render_scale.Set( 1.0, 1.0, 1.0 );
			}
			else
			{
				ent.render_scale = s1->render_scale;
			}

			if (!(renderfx & RF_SPOTLIGHT))
				continue;
		}

		// this is userEntity_t type that is solely a dynamic light
		if (effects & EF_BRIGHTLIGHT)
		{
			// color value is stored in mins[], brightness in maxs[0]
			V_AddLight (ent.origin, s1->maxs.x, s1->mins.x, s1->mins.y, s1->mins.z );
			continue;
		}

		if (renderfx & RF_SPOTLIGHT)
		{
			// lerp render_scale
//			ent.render_scale.x = s1->render_scale.x + cl.lerpfrac * (cent->current.render_scale.x - cent->prev.render_scale.x);
//			ent.render_scale.y = s1->render_scale.y + cl.lerpfrac * (cent->current.render_scale.y - cent->prev.render_scale.y);
//			ent.render_scale.z = s1->render_scale.z + cl.lerpfrac * (cent->current.render_scale.z - cent->prev.render_scale.z);

			ent.render_scale = s1->render_scale.Interpolate(
				cent->current.render_scale,
				cent->prev.render_scale,
				cl.lerpfrac	);

			ent.mins = s1->mins;
			ent.maxs = s1->maxs;

			// lerp the angles in case it's rotating
			ang1 = cent->current.angles.x;
			ang2 = cent->prev.angles.x;
			ent.angles.x = LerpAngle(ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angles.y;
			ang2 = cent->prev.angles.y;
			ent.angles.y = LerpAngle(ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angles.z;
			ang2 = cent->prev.angles.z;
			ent.angles.z = LerpAngle(ang2, ang1, cl.lerpfrac);

			//NSS[11/14/99]:Not being used... WHY LEAVE IT IN!!?
			//float intensity = ( ( ent.render_scale.x - ent.origin.x ) * 0.5f ) + 
			//					( ( ent.render_scale.y - ent.origin.y ) * 0.5f ) +
			//					( ( ent.render_scale.z - ent.origin.z ) * 0.5f );

			V_AddLight( cent->current.render_scale, 75, ent.color.x, ent.color.y, ent.color.z );
		}

//		ent.render_scale = s1->render_scale;

		// tweak the color of beams
		if ( renderfx & RF_BEAM_MOVING)
		{
      		if (renderfx & RF_TRANSLUCENT)
      			ent.alpha = s1->alpha;
      		else
				  ent.alpha = .3;

      	// the four beam colors are encoded in 32 bits of skinnum (hack)
//			ent.skinnum = (s1->skinnum >> ((rand() % 4)*8)) & 0xff;
//			ent.skinnum = 255 << 0 | 255 << 8 | 255 << 16;
			ent.skinnum = s1->skinnum;
			ent.model = NULL;
		}
		else
		{
            ent.skinnum = s1->skinnum;
            ent.skin = NULL;
            ent.model = cl.model_draw[s1->modelindex];

            //DKLOG_Write(LOGTYPE_MISC, 0.0, "s1->skinnum=%d",s1->skinnum);

            if (s1->skinnum == CHARACTER_MODEL_SKINNUM)
            {
                //DKLOG_Write(LOGTYPE_MISC, 0.0, "s1->number = %d",s1->number);
                // use custom player skin
                ent.skinnum = 0;
                ci = &cl.clientinfo[s1->number - 1];
                ent.skin = ci->skin;
                //ent.model = cl.model_draw[ci->modelindex];  // 3.9 dsn
                if(! (ent.model = ci->model))
                    ent.model = cl.model_draw[s1->modelindex];
/*				  if (!ent.skin || !ent.model)
				  {
					// ent.skin = cl.baseclientinfo.skin;
					ent.model = ent.model = cl.model_draw[s1->modelindex];;
				  }    
*/                  
			}
			//amw: 5.3.99 - this skin should have been set up with gstate->ImageIndex()
			else if ((ent.skinnum) && (effects2 & EF2_IMAGESKIN))	
			{
				// this allows applying any skin to any entity
				ent.skin = cl.image_precache[ent.skinnum];
			}
		}

		// only used for black hole model right now, FIXME: do better
		if (renderfx & RF_TRANSLUCENT)
			ent.alpha = s1->alpha;
//			ent.alpha = 0.70;

//		if (renderfx & SPR_ALPHACHANNEL)		//  Shawn:  Added for daikatana
//			ent.alpha = 1;

		// render effects (fullbright, translucent, etc)
		if ((effects & EF_COLOR_SHELL))
			ent.flags = 0;	// renderfx go on color shell entity
		else
			ent.flags = renderfx;

		// calculate angles
		if (effects & EF_ROTATE)
		{	// some bonus items auto-rotate... others rotate based on the speed
			// increments stored in s1->angle_delta
			// also..save the new angles in lerp_angles
            int rotator = (effects2 & EF2_ROTATE_CLOCK) ? -1 : 1; // rotate clockwise?

			if (s1->angle_delta.x || s1->angle_delta.y || s1->angle_delta.z)
			{
				if (s1->angle_delta.x)
				  ent.angles.x = cent->lerp_angles.x = anglemod((cent->lerp_angles.x + ((s1->angle_delta.x-180) * rotator)  ));
				else
				  ent.angles.x = 0;

				if (s1->angle_delta.y)
				  ent.angles.y = cent->lerp_angles.y = anglemod((cent->lerp_angles.y + ((s1->angle_delta.y/30) * rotator)  ));
				else
				  ent.angles.y = 0;

				if (s1->angle_delta.z)
				  ent.angles.z = cent->lerp_angles.z = anglemod((cent->lerp_angles.z + ((s1->angle_delta.z/30) * rotator)  ));
				else
				  ent.angles.z = 0;
			}
			else
			{
				ent.angles.x = 0;
				ent.angles.y = cl_autorotate;
				ent.angles.z = 0;
			}
		}
		else 
		{	// interpolate angles
			ang1 = cent->current.angles.x;
			ang2 = cent->prev.angles.x;
			ent.angles.x = LerpAngle (ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angles.y;
			ang2 = cent->prev.angles.y;
			ent.angles.y = LerpAngle (ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angles.z;
			ang2 = cent->prev.angles.z;
			ent.angles.z = LerpAngle (ang2, ang1, cl.lerpfrac);

			//&&& amw - we don't do this anymore for EF_ROTATE entities
			//	interpolate angle deltas and store them in lerpangles
			ang1 = cent->current.angle_delta.x;
			ang2 = cent->prev.angle_delta.x;
			cent->lerp_angles.x = LerpAngle (ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angle_delta.y;
			ang2 = cent->prev.angle_delta.y;
			cent->lerp_angles.y = LerpAngle (ang2, ang1, cl.lerpfrac);

			ang1 = cent->current.angle_delta.z;
			ang2 = cent->prev.angle_delta.z;
			cent->lerp_angles.z = LerpAngle (ang2, ang1, cl.lerpfrac);
		}
		//	handle muzzle flash

		if (renderfx & RF_MUZZLEFLASH)
		{
			V_AddLight (cent->current.origin, 120, 1.0, 0.75, 0.0);

			//	try to add to all possible surfaces
			CL_AddMuzzleFlashToSurface (&ent, "hr_muzzle", s1);
		}
		//	Nelno:	draw player model when in camera mode
		if (s1->number == cl.playernum+1 && !(frame->playerstate.pmove.pm_flags & PMF_CAMERAMODE))
		{
			ent.flags |= RF_VIEWERMODEL;	// only draw from mirrors
			// FIXME: still pass to refresh

			if (effects & EF_FLAG1)
				V_AddLight (ent.origin, 225, 1.0, 0.1, 0.1);
			else if (effects & EF_FLAG2)
				V_AddLight (ent.origin, 225, 0.1, 0.1, 1.0);

			continue;
		}

		//	Nelno:	don't draw player's body parts unless in camera mode
/*
		if ((s1->number == cl.num_player_legs || s1->number == cl.num_player_torso ||
			s1->number == cl.num_player_head || s1->number == cl.num_player_weapon) &&
			!(frame->playerstate.pmove.pm_flags & PMF_CAMERAMODE))
		{
			ent.flags |= RF_VIEWERMODEL;
			continue;
		}
*/
		// if set to invisible, skip
		if (!s1->modelindex)
			continue;

		if (renderfx & RF_TRAILS)
		{
			CL_AddTrails(ent.origin,cent);
		}
        if(effects & EF_BEAMTRAIL)
		{
            //cek[1-29-00]: sonova....CL_BeamTrail(ent.origin, cent);
			CL_DiscusSparkles(cent);
		}

		/*************** cluster animation ****************/
/*
		if (renderfx & RF_ENVMAP)
		{
//	Com_Printf ("clusters: %d  angles x=%f y=%f z=%f\n",s1->numClusters, ent.angles.x, ent.angles.y, ent.angles.z);
			ent.pDef = s1->cDef;			// point to cluster data
			ent.count = s1->numClusters;	// number of clusters defined
			
            // Ash: animate head and stuff the head angles in the cluster angles
            CL_HeadUpdate(s1);
            ent.pDef[0].angles = gHeadData[s1->number].headAngles;

            // s1->numClusters is the entity to look at (0 is the world -- look straight ahead)
            gHeadData[s1->number].entTarget = s1->numClusters;

			ent.flags |= RF_MINLIGHT;

			V_AddEntity (&ent);
		}
		else
*/		{
			V_AddEntity (&ent);
		}

		// color shells generate a seperate entity for the main model
		if (effects & EF_COLOR_SHELL)
		{
			ent.flags = renderfx | RF_TRANSLUCENT;
			ent.alpha = 0.30;
			V_AddEntity (&ent);
		}

		if (effects2 & EF2_INVULNERABLE)
		{
			V_AddLight (ent.origin, 128, 1.0, 1.0, 0.25);
		}

		//////////////////////////////
		// process linked models now
		//////////////////////////////

		// WAW[11/15/99]: Added for CTF.  HACK!! I hate doing this...!!!!
		if (s1->modelindex4)
		{
			// Save everything I'm going to possibly change.
			entity_t	enew = ent;
			CVector		hardPt;

			hardPt.Set(0,0,0);
			re.GetModelHardpoint("ctf_flag", s1->frame, cent->prev.frame, enew, hardPt);
			enew.origin		= hardPt;			

			enew.model		= cl.model_draw[(s1->modelindex4 & 0x1fff)];
//			enew.skin		= NULL;
//			enew.skinnum	= 0;
			centity_t *flagEnt = &cl_entities [s1->modelindex3];
			enew.skinnum	= flagEnt->current.skinnum;
			enew.skin		= cl.image_precache[enew.skinnum];
			enew.render_scale.Set(flagEnt->current.render_scale);

//			enew.skinnum	= s1->modelindex3;
//			enew.skin		= cl.image_precache[enew.skinnum];
//			enew.render_scale.Set(1,1,1);

			// our player weapon models use the 3rd frame of the model
			enew.frame = enew.oldframe = ((s1->modelindex4 & (0x7 << 13)) >> 13);
			enew.flags |= RF_MINLIGHT;

			V_AddEntity (&enew);
		}
		//==============================

		
		// modelindex2 is reserved for the weapon model
		if (s1->modelindex2)
		{
			if (!(renderfx & RF_WEAPONMODEL))
			{
				int modelindex2 = s1->modelindex2 & 0x0000ffff;
				if (modelindex2)
				{
					CVector hardPt, oldOrigin;
					// retrieve the hardpoint location for the surface, model and frame defined
					hardPt.Set(0,0,0);
					ent.angles.roll *= -1;	// cek[12-2-99] fixes strafe lerp problem
					re.GetModelHardpoint("hp_gun", s1->frame, cent->prev.frame, ent, hardPt);
					ent.angles.roll *= -1;
		//			ent.render_scale.Set( 1.0, 1.0, 1.0 );
					// set up the new origin
					oldOrigin = ent.origin;
					ent.origin = hardPt;
					ent.oldorigin = ent.origin;
					// use the weapon model pointed to by index2
					ent.model = cl.model_draw[modelindex2];
					ent.skin = NULL;
					ent.skinnum = 0;
					// our player weapon models use the 3rd frame of the model

					int cFrame,oFrame;
					cFrame = ent.frame;
					oFrame = ent.oldframe;

					ent.frame = ent.oldframe = 4;

					ent.flags |= RF_MINLIGHT;

					V_AddEntity (&ent);
					
					// reset it for effects etc
					ent.origin = oldOrigin;
					ent.frame = cFrame;
					ent.oldframe = oFrame;
				}
			}
		}

		// certain settings don't carry over to all models
		ent.skin = NULL;		
		ent.skinnum = 0;
		ent.flags = 0;
		ent.alpha = 0;
		
/*
		if (s1->modelindex3)
		{
			ent.model = cl.model_draw[s1->modelindex3];
			V_AddEntity (&ent);
		}
*/


		// add automatic particle trails
		if ( (effects&~EF_ROTATE) )
		{
			if (effects & EF_POISON)
			{
				CVector dir,fwd,rt,up;
				//float spread;

				dir = cent->current.angles;
				AngleToVectors(dir,fwd,rt,up);
				//spread=(1.0-cent->current.alpha);
                /*
				if (spread < .1 || spread > .6)
				{
				   spread=.1;
				}
                */
//                CL_PoisonSpray(cent->lerp_origin,ent.origin,fwd,0,3);
                CL_PoisonSpray(cent->lerp_origin,ent.origin,0,3);
			}
            /*
			else if (effects & EF_ENTITYTRAIL)
			{
                CL_EntityTrail(cent->lerp_origin, ent.origin, cent);
				//CL_RocketTrail (cent->lerp_origin, ent.origin, cent);
				//V_AddLight (ent.origin, 200, 1, 1, 0);
			}
            */
			else if (effects & EF_SMOKETRAIL)
			{

//                CL_SunflareSmoke(cent->lerp_origin,1.0,1,1.5,62.2);
				// SCG[1/21/00]: changed 1.5 to 1 since the 4th parameter is an int, not a float.
                CL_SunflareSmoke(cent->lerp_origin,1.0,1,1,62.2);
                CL_SunflareSparks(cent->lerp_origin, 1.0f, 5.0f, 10, 18.0f);
//            CL_DiminishingTrail (cent->lerp_origin, ent.origin, cent, EF_SMOKETRAIL);
			}
			else if (effects & EF_SLUDGE)
			{
				CL_DiminishingTrail (cent->lerp_origin, ent.origin, cent, effects);
				V_AddLight (ent.origin, -200, 1, 1, 0);
			}
			else if (effects & EF_DIMLIGHT)
			{
				//	Nelno: changed color from pure yellow
				if( ent.color.x || ent.color.y ||  ent.color.z )
					V_AddLight (ent.origin, 100, ent.color.x, ent.color.y, ent.color.z);
				else
					V_AddLight (ent.origin, 100, 1, 1, 0.75);
			}
			else if (effects & EF_GIB)
			{
				CL_DiminishingTrail (cent->lerp_origin, ent.origin, cent, effects);
			}
			else if (effects & EF_MAGIC_ARROW)
			{
				CL_BlueSparkTrail (cent->lerp_origin, ent.origin );
			}
			else if (effects & EF_BOLTTRAIL)
			{
				CL_BoltTrail(cent->lerp_origin, ent.origin, cent);
			}
			else if (effects & EF_METEOR)
			{
				CL_MeteorTrail(cent->lerp_origin, ent.origin, cent, EF_METEOR|EF_FIRE);
			}
			else if (effects & EF_FIRE)
			{
				CL_MeteorTrail(cent->lerp_origin, ent.origin, cent, EF_FIRE);
			}
			else if (effects & EF_GREENGIB)
			{
				CL_DiminishingTrail (cent->lerp_origin, ent.origin, cent, effects);				
			}
			else if (effects & EF_WISP_TRAIL)
			{
				//<nss> Redo of the Wisp_Trail  using new ComplexParticle Function. 
				//We need to go through all of the shit and use this instead of hundreds of lines of the same code with different hard coded values
				CVector Alpha,Color,Dir;
				Dir.Set(1,1,-1);//Make it go in either x or y direction and downwards
				Color.Set(0.25,0.15,0.75);//Set the particle color
				Alpha.Set(0.85,0.45,0.25);//Set the initial alpha, the depletion rate, and the random addition to depletion
				//The sparkle function was randomly assigning one or the other sparkle, this is how it should be done.
				if(frand() > 0.50f)
				{
					CL_GenerateComplexParticle(cent->lerp_origin , Dir, Color, Alpha, 0.75f, 1, 7, 100,PARTICLE_SPARKLE1,2);//Make Da' Sparks
				}
				else
				{
					CL_GenerateComplexParticle(cent->lerp_origin , Dir, Color, Alpha, 0.75f, 1, 7, 100,PARTICLE_SPARKLE2,2);//Make Da' Sparks
				}
				//Dunno about the below function may look into this one later.
				V_AddLight (ent.origin, 100, 0.5, 0.75, 1.0);
			}

		}

		if (effects2 & EF2_TRIDENTTRAIL)
		{
			CL_TridentTrail(cent->lerp_origin, ent.origin );
        }
		if (effects2 & EF2_STAVROS_AMMO)
		{
			if( cent->current.modelindex )
				CL_StavrosAmmo ( cent->lerp_origin );
		}
		if (effects2 & EF2_C4_BEEP)
		{
			CL_C4Beep(ent);
		}

		if (effects2 & EF2_BOOSTFX)
		  CL_BoostEffects(ent.origin, effects2);

		cent->lerp_origin = ent.origin;
	}

	if (!(cl.refdef.rdflags & RDF_COOP))
		sidekicks_active_update(old_sidekicks_active); // compare new status of sidekicks active


}


/*
===============
CL_CalcViewValues

Sets cl.refdef view values
===============
*/

// SCG[1/21/00]: fabs is for floating point. ops->pmove.origin is a short!
//#define player_teleported(ps, ops) (fabs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 || \
//                                    fabs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 || \
//                                    fabs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8)
#define player_teleported(ps, ops) (abs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 || \
                                    abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 || \
                                    abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8)

void CL_ViewSpecial(void)
{
    if(cl.time > g_KickView.iStartTime + g_KickView.iKickVelocityReturn + g_KickView.iKickVelocityTo)
    {
        memset(&g_KickView, 0, sizeof(g_KickView));
    } 
	else 
	{
        if(cl.time < g_KickView.iStartTime + g_KickView.iKickVelocityTo) 
		{
            // en route to effect extent, fMult approaches 1
            g_KickView.fMult = (cl.time - g_KickView.iStartTime) * g_KickView.fToInc;
        } 
		else if (cl.time < g_KickView.iStartTime + g_KickView.iKickVelocityReturn + g_KickView.iKickVelocityTo) 
		{
            g_KickView.fMult = 1- ((cl.time - g_KickView.iStartTime - g_KickView.iKickVelocityTo) * g_KickView.fFromInc);
        }
        g_KickView.vecAngleOffset = g_KickView.vecKickAngles * g_KickView.fMult;
        cl.refdef.viewangles += g_KickView.vecAngleOffset;
        // override ps fov
        if(g_KickView.fFOV)
        {
            g_KickView.fFOVOffset = g_KickView.fBaseFOV + (g_KickView.fFOV * g_KickView.fMult);
            if(g_KickView.fFOVOffset < 1)
                g_KickView.fFOVOffset = 1;
            else if(g_KickView.fFOVOffset > 179)
                g_KickView.fFOVOffset = 179;
            cl.refdef.fov_x = g_KickView.fFOVOffset;
        }
    }
}

float fViewBobTime;
void CL_ViewBob(void)
{
    float   fXYSpeed, fBobRate, fBobMove, fDistX, fBobRadius;
    CVector vecSpeed;

    if (!cl_bob->value || (cl.frame.playerstate.pmove.pm_type == PM_FREEZE))
        return;
    
    vecSpeed.Set(cl.frame.playerstate.pmove.velocity[0], cl.frame.playerstate.pmove.velocity[1], cl.frame.playerstate.pmove.velocity[2]);
    fXYSpeed = vecSpeed.Length();
    fBobRate = 0.008;
    fBobRadius = 1.35;
    if(fXYSpeed > 25 && cl.frame.playerstate.pmove.pm_flags & PMF_ON_GROUND)
    {
        fBobMove = fXYSpeed * fBobRate;
        fViewBobTime += fBobMove * cls.frametime;
    } 
    /*else {
        fBobMove = 0.125f;
        fViewBobTime -= fBobMove * cls.frametime;
    }*/
    fDistX = cos(fViewBobTime) * (fBobRadius);
    cl.refdef.vieworg.z += fDistX;
}

void CL_CalcViewValues (void)
{
	int				i;
	float			lerp, backlerp;
	centity_t		*ent;
	frame_t			*oldframe;
	player_state_t	*ps, *ops;
	float			r, g, b, a;
	CVector			ops_view_offset, ps_view_offset;

	int				player_contents;
	unsigned short	contents_color;

    //get the current player state.
	ps = &cl.frame.playerstate;

	// find the previous frame to interpolate from
	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or invalid

    //get the old player state.
	ops = &oldframe->playerstate;

	//	do not interpolate the view origin or angles if:
	//	player teleported
	//	view entity changed
	//	input entity changed (this may not be necessary...)
	if (player_teleported(ps, ops) ||
		ops->view_entity != ps->view_entity || 
		ops->input_entity != ps->input_entity)
    {
		ops = ps;		// don't interpolate
    }

    //get the player entity
	ent = &cl_entities[cl.playernum+1];
	lerp = cl.lerpfrac;

    ops_view_offset = ops->viewoffset;
    ps_view_offset = ps->viewoffset;

	// calculate the origin
	if ((cl_predict->value) && !(cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION) && 
		!(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE))
	{	// use predicted values
		unsigned	delta;

		backlerp = 1.0 - lerp;

		cl.refdef.vieworg.x = cl.predicted_origin.x + ops_view_offset.x 
							+ cl.lerpfrac * (ps_view_offset.x - ops_view_offset.x)
							- backlerp * cl.prediction_error.x;

		cl.refdef.vieworg.y = cl.predicted_origin.y + ops_view_offset.y 
							+ cl.lerpfrac * (ps_view_offset.y - ops_view_offset.y)
							- backlerp * cl.prediction_error.y;

		cl.refdef.vieworg.z = cl.predicted_origin.z + ops_view_offset.z 
							+ cl.lerpfrac * (ps_view_offset.z - ops_view_offset.z)
							- backlerp * cl.prediction_error.z;

		// smooth out stair climbing
		delta = cls.realtime - cl.predicted_step_time;
		if (delta < 100)
		{
			cl.refdef.vieworg.z -= cl.predicted_step * (100 - delta) * 0.01;
		}
	}
	else
	{	// just use interpolated values
		cl.refdef.vieworg.x = ops->pmove.origin[0]*0.125 + ops_view_offset.x 
								+ lerp * (ps->pmove.origin[0]*0.125 + ps_view_offset.x 
								- (ops->pmove.origin[0]*0.125 + ops_view_offset.x) );

		cl.refdef.vieworg.y = ops->pmove.origin[1]*0.125 + ops_view_offset.y 
								+ lerp * (ps->pmove.origin[1]*0.125 + ps_view_offset.y 
								- (ops->pmove.origin[1]*0.125 + ops_view_offset.y) );

		cl.refdef.vieworg.z = ops->pmove.origin[2]*0.125 + ops_view_offset.z 
								+ lerp * (ps->pmove.origin[2]*0.125 + ps_view_offset.z 
								- (ops->pmove.origin[2]*0.125 + ops_view_offset.z) );

	}

	for (i=0 ; i<4 ; i++)
		cl.refdef.blend[i] = ps->blend[i];

	// check contents for water fog
	int leaf = CM_PointLeafnum( cl.refdef.vieworg );

	player_contents = CM_ContentsForLeaf( leaf );

	if( player_contents != last_player_contents )
	{
		if( player_contents & MASK_WATER )
		{	
			contents_color = CM_ContentsColor( leaf );

			r = ( float )( ( contents_color >> 8 ) & 0xf8 ) / 255.0;
			g = ( float )( ( contents_color >> 3 ) & 0xfc ) / 255.0;
			b = ( float )( ( contents_color << 3 ) & 0xf8 ) / 255.0;
			a = 0.2;

			r_foginfo.active = 1;
			r_foginfo.inc = 0;
			r_foginfo.r = r;
			r_foginfo.g = g;
			r_foginfo.b = b;
			r_foginfo.start = 32;
			r_foginfo.end = 1024;
			r_foginfo.skyend = 4096;

			cl.refdef.blend[0] = r;
			cl.refdef.blend[1] = g;
			cl.refdef.blend[2] = b;
			cl.refdef.blend[3] = a;
		}
		else
		{
			r_foginfo.active	= global_fog_state.active;
			r_foginfo.inc		= global_fog_state.inc;
			r_foginfo.r			= global_fog_state.r;
			r_foginfo.g			= global_fog_state.g;
			r_foginfo.b			= global_fog_state.b;
			r_foginfo.start		= global_fog_state.start;
			r_foginfo.end		= global_fog_state.end;
			r_foginfo.skyend	= global_fog_state.skyend;

			cl.refdef.blend[0] = 0;
			cl.refdef.blend[1] = 0;
			cl.refdef.blend[2] = 0;
			cl.refdef.blend[3] = 0;
		}

		last_player_contents = player_contents;

	}
	
	// if not running a demo or on a locked frame, add the local angle movement
	// NSS[1/7/00]:Special case handlig for view angles in camera mode or no_lerp mode.
	if (cl.frame.playerstate.pmove.pm_flags & (PMF_CAMERAMODE|PMF_CAMERA_NOLERP))
	{
		if(cl.frame.playerstate.pmove.pm_flags & PMF_CAMERA_NOLERP)
		{
			//NSS[10/26/99]:Don't lerp when in stationary camera modes.
			//cl.refdef.viewangles = cl.predicted_angles;	
			cl.refdef.viewangles = ps->viewangles;
		}
		else
		{
			//	interpolate view angles
 			cl.refdef.viewangles.x = LerpAngle(ops->viewangles.x, ps->viewangles.x, lerp);
			cl.refdef.viewangles.y = LerpAngle(ops->viewangles.y, ps->viewangles.y, lerp);
			cl.refdef.viewangles.z = LerpAngle(ops->viewangles.z, ps->viewangles.z, lerp);
		}
	}
	else if ( cl.frame.playerstate.pmove.pm_type < PM_DEAD)
	{	// use predicted values
		cl.refdef.viewangles = cl.predicted_angles;
	}
	else
	{	// just use interpolated values
		cl.refdef.viewangles.x = LerpAngle(ops->viewangles.x, ps->viewangles.x, lerp);
		cl.refdef.viewangles.y = LerpAngle(ops->viewangles.y, ps->viewangles.y, lerp);
		cl.refdef.viewangles.z = LerpAngle(ops->viewangles.z, ps->viewangles.z, lerp);
	}

	cl.refdef.viewangles.x += LerpAngle(ops->kick_angles.x, ps->kick_angles.x, lerp);
	cl.refdef.viewangles.y += LerpAngle(ops->kick_angles.y, ps->kick_angles.y, lerp);
	cl.refdef.viewangles.z += LerpAngle(ops->kick_angles.z, ps->kick_angles.z, lerp);

	// interpolate field of view
	cl.refdef.fov_x = ops->fov + lerp * (ps->fov - ops->fov);

    // Logic[5/2/99]: KickView (angle and fov perturbation with velocities)
    if(g_KickView.iStartTime)
    {
        CL_ViewSpecial();
    }
    CL_ViewBob();
	// don't interpolate blend color
//	for (i=0 ; i<4 ; i++)
//		cl.refdef.blend[i] = ps->blend[i];

    //all this hard work, and now we let the scipt override it all with what it wants.
    GCE_SetCamera();

	AngleToVectors (cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);
}

///////////////////////////////////////////////////////////////////////////////
//	CL_AddLightFlares
///////////////////////////////////////////////////////////////////////////////

void	CL_AddLightFlares (frame_t *frame)
{
	entity_t		flare;
	centity_t		*cent;
	int				pnum;
	entity_state_t	*ent_state;
	CVector			dir;
	trace_t			trace;
	unsigned int	length;
	int				flare_count = 0;

	if (cl_showflares == NULL || cl_showflares->value == 0)
		return;

	memset (&flare, 0, sizeof(flare));

	for (pnum = 0 ; pnum<frame->num_entities ; pnum++)
	{
		ent_state = &cl_parse_entities[(frame->parse_entities+pnum)&(MAX_PARSE_ENTITIES-1)];
		cent = &cl_entities [ent_state->number];

		if (ent_state->renderfx & RF_LIGHTFLARE)
		{
			flare_count++;

			trace = CL_TraceLine(cl.refdef.vieworg, ent_state->origin, cl.playernum + 1, MASK_SOLID);
////			trace = CL_Trace (cl.refdef.vieworg, NULL, NULL, cent->lerp_origin, cl.playernum + 1, MASK_SOLID);

			if (trace.allsolid || trace.startsolid)
				continue;

			//	Calculate vector from flare origin to view origin
			dir = cl.refdef.vieworg - cent->lerp_origin;
			length = (int)dir.Normalize();

			if ((1.0 - trace.fraction) * length > 16.0)
				continue;

			//	now offset flare along this line
			length = length >> 1;
			dir = dir * length;
			flare.origin = cent->lerp_origin + dir;

			flare.frame = 0;
			flare.oldframe = 0;
			flare.alpha = 1 - ( ( float )length / 512.0 );
			flare.render_scale = ent_state->render_scale;
			flare.render_scale.Add( flare.alpha );

			if( flare.render_scale.x == 0.0 
				|| flare.render_scale.y == 0.0 
				|| flare.render_scale.z == 0.0 )
			{
				continue;
			}

			flare.model = cl.model_draw [ent_state->modelindex];

			flare.flags = SPR_ALPHACHANNEL | RF_TRANSLUCENT;

			flare.backlerp = 1.0 - cl.lerpfrac;
			flare.oldorigin = flare.origin;	// don't lerp at all

			V_AddEntity (&flare);
		}
	}

	if (cl_flarecount && cl_flarecount->value)
		Com_Printf ("%i light flares in PVS.\n", flare_count);
}

#if 0	// amw 4.29.99 - not used...

//////////////////////////////////////////////////////////
//
//  Function: CL_ClipAndCullEntity
//
//	Purpose : test the origin of an entity to see if they
//            are all behind the player and within range
//
//////////////////////////////////////////////////////////

int CL_ClipAndCullEntity (CVector entPos)
{
	int			i;
	float		dotProduct = 0.0, distance, clipDistance, minDistance;
	CVector		forward, origin, vecToOrigin;
	qboolean	bRainVolume = FALSE;

	// get the forward facing vector
	forward = cl.v_forward;

	// copy the view origin 
	origin = cl.refdef.vieworg;

	clipDistance = PV_CLIP_DISTANCE;

	// get a vector from the corners to the view origin
	vecToOrigin = entPos - origin;
		
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
			return	false;
		}
	}

	// at this point, this entity is about to be clipped/culled out because it's origin is 
	// behind the player.. or too far away
	
	// clip/cull it out
	return true;
}

#endif

static float       fBobTime;
static float       fGasTime;   // fucking hack
static CVector     vecBobOffset;       // calculated offset from normal ent origin, run whenever

CVector CL_GetBobOffset()
{
	return vecBobOffset;
}

void CL_AddWeaponBob(entity_t *ent, centity_t *cent)
{
    if (!cl_bob->value || (cl.frame.playerstate.pmove.pm_type == PM_FREEZE))
        return;
    
    float       fBobMove, fXYSpeed, fDistX, fDistY, fBobRadius, fBobRate, fMotorCycle;
    CVector     vecVel, vecDir, vecForward, vecRight, vecUp, vecSmokeOrigin;

    if(!(cent->current.renderfx & RF_SHAKE))
    {
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
        } 
		else 
		{
            fBobMove = .125;
            if(fBobTime >= M_PI)
                fBobTime -= fBobMove;
            else
                fBobTime += fBobMove;
            if(fBobTime < 0)
                fBobTime = 0;
        }
    } 
	else 
	{
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
        
    }


    fDistX = cos(fBobTime)*(fBobRadius);
    fDistY = fabs(fDistX);
    vecBobOffset = cl.refdef.viewangles;
    vecBobOffset.AngleToVectors(vecForward, vecRight, vecUp);
    vecBobOffset = vecRight * fDistX;
    VectorMA(vecBobOffset, -vecUp, fDistY, vecBobOffset);
    ent->origin += vecBobOffset;
}

void CL_AddWeaponModels()
{

	centity_t *cent = &cl_entities[0];
	centity_t *player;
	int count = 0;
	if (cl.frame.playerstate.pmove.pm_flags & PMF_CAMERAMODE)
		return;

	for (int i = 0; i < MAX_EDICTS; i++,cent++)
	{
		if (cent->current.renderfx & RF_WEAPONMODEL)
		{
			if (cent->current.renderfx & RF_NODRAW)
				continue;

			player = &cl_entities[cl.playernum+1];
			int modelindex2 = (player->current.modelindex2 & 0xffff0000) >> 16;
			if (modelindex2 != cent->current.number)
				continue;

			if (cent->current.modelindex2 != player->current.number + 10000)
				continue;

			count++;
			entity_t ent;
			memset(&ent,0,sizeof(ent));

			ent.backlerp = cent->current.mins.x;
			ent.frame = cent->current.mins.z;
			ent.oldframe = cent->current.mins.y;
			ent.origin = cl.refdef.vieworg;
			ent.oldorigin = ent.origin;
//			Com_DPrintf("Old: %d -- new %d\n",ent.oldframe,ent.frame);

			CL_AddWeaponBob(&ent, cent);
			ent.angles = cl.refdef.viewangles;
			ent.model = cl.model_draw[cent->current.modelindex];
			ent.alpha = player->current.alpha;
			ent.flags = cent->current.renderfx;
			ent.render_scale = player->current.render_scale;
			V_AddEntity(&ent);

			if (cent->current.frameInfo.frameFlags == 0 || cent->current.frameInfo.frameFlags & FRAME_STATIC)
			{
				backframe[cent->current.number] = ent.oldframe;
				frontframe[cent->current.number] = ent.frame;
			}
		}
	}

	if (count > 1)
	{
		Com_DPrintf("Multiple weapon models drawn!\n");
	}
}

/*
===============
CL_AddEntities

Emits all entities, particles, and lights to the refresh
===============
*/
void CL_AddEntities (void)
{
//    unsigned int starttime, endtime;

	if( cls.disable_screen )
		return;

	if (cls.state != ca_active)
		return;

	if (cl.time > cl.frame.servertime)
	{
		if (cl_showclamp->value)
			Com_Printf ("high clamp %i\n", cl.time - cl.frame.servertime);
		cl.time = cl.frame.servertime;
		cl.lerpfrac = 1.0;
	}
	else if (cl.time < cl.frame.servertime - CL_FRAME_MILLISECONDS)
	{
		if (cl_showclamp->value)
			Com_Printf ("low clamp %i\n", cl.frame.servertime - CL_FRAME_MILLISECONDS - cl.time);
		cl.time = cl.frame.servertime - CL_FRAME_MILLISECONDS;
		cl.lerpfrac = 0;
	}
	else
		cl.lerpfrac = 1.0 - (cl.frame.servertime - cl.time) * 0.01;

	if (cl_timedemo->value)
		cl.lerpfrac = 1.0;

	// bonus items rotate at a fixed rate
	cl_autorotate = anglemod(cl.time * 0.1);

	// brush models can auto animate their frames
	cl_autoanim = 2*cl.time * 0.001;

	CL_AddPacketEntities (&cl.frame);
    
//    starttime = Sys_Milliseconds();
	CL_AddTEnts ();
	CL_AddParticles ();
//	endtime = Sys_Milliseconds();
    CL_AddDLights ();
	CL_AddLightStyles ();
	CL_CalcViewValues ();
	CL_AddParticleVolumes ();				// amw - *this uses values calculated in CL_CalcViewValues()*

// SCG[8/10/99]: Not used
//	CL_AddFlares ();						// Shawn:  Added for daikatana
	CL_AddComParticles ();					// Shawn:  Added for daikatana

	CL_AddLightFlares (&cl.frame);			// Nelno:	add flares attached to light entities

	CIN_GCE_AddEntities();

	CL_AddWeaponModels();
	CL_AddTrackEnts();

}



/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
void CL_GetEntitySoundOrigin (int ent, CVector &org) //, CVector &vecAngles)
{
	centity_t	*cent;

	// validate entity number
	if (ent < 0 || ent >= MAX_EDICTS)
		Com_Error (ERR_DROP, "CL_GetEntitySoundOrigin: bad ent");
	// get entity
	cent = &cl_entities[ent];	
	if (cent->origin_hack.Length() == 0)
	{
		org = cent->lerp_origin;
		if (org.Length() == 0)
		{
			// ack!  origins still aren't set yuck
			org = cent->baseline.origin;
		}
	}
	else
	{
		org = cent->origin_hack;
	}
}
