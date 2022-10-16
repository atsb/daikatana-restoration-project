#include <windows.h>
#include <gl/gl.h>
#include "sed.h"

extern STATE *gl_pState;

typedef struct camhook_proto
{
    think_t         pre_think;
    think_t         post_think;

    VEC3			pos, ang, oldpos, oldang;
    VEC3			fwdvec, upvec, rightvec;

    int             iType;

    CUBIMOVE       cubicPos, cubicFocus;
} CAMHOOK;

CAMHOOK gl_CamHook;

void CameraSendViewAngle( USERENT *Client, const VEC3 &ang )
{
	vec3_t	angles;

	ang.copyTo (angles);
	angles [0] = -angles [0];

	gl_pState->SetClientAngles (Client, angles);
}


void CamPreThinkDefault( USERENT *CamEnt )
{
}


void CamPreThinkEditPos( USERENT *CamEnt )
{
    float	dx, dy, dz;
    VEC3	posdiff;

    dx = gl_CamHook.ang[1] - CamEnt->owner->client->v_angle[1];
    dy = gl_CamHook.ang[0] - CamEnt->owner->client->v_angle[0];
    dz = 0.0f;

    // hack to prevent angle jumping

    if( fabs(dx) > 90.0f ) dx = 0.0f;
    if( fabs(dy) > 90.0f ) dy = 0.0f;

    posdiff = dx*gl_CamHook.rightvec + dy*gl_CamHook.upvec;

    gl_Editor.EditNodePos( posdiff, TRUE );

    gl_CamHook.ang = VEC3 ( CamEnt->owner->client->v_angle );
}


void CamPreThinkEditPos2( USERENT *CamEnt )
{
    float	dx, dy, dz;
    VEC3	posdiff;

    dx = gl_CamHook.ang[1] - CamEnt->owner->client->v_angle[1];
    dy = gl_CamHook.ang[0] - CamEnt->owner->client->v_angle[0];
    dz = 0.0f;

    // hack to prevent angle jumping

    if( fabs(dx) > 90.0f ) dx = 0.0f;
    if( fabs(dy) > 90.0f ) dy = 0.0f;

    posdiff = dx*gl_CamHook.rightvec + dy*gl_CamHook.fwdvec;

    gl_Editor.EditNodePos( posdiff, TRUE );

    gl_CamHook.ang = VEC3( CamEnt->owner->client->v_angle );
}


void CamPreThinkEditVel( USERENT *CamEnt )
{
    float	phi, theta, rho;
    VEC3	velang, newvel;

    theta = gl_CamHook.ang[1] - CamEnt->owner->client->v_angle[1];
    phi   = gl_CamHook.ang[0] - CamEnt->owner->client->v_angle[0];
    
    // hack to prevent angle jumping

    if( fabs(theta) > 90.0f ) theta = 0.0f;

    velang = gl_Info.NodeVel.VecToAngle();
    phi = (phi+velang[0]-90.0f)*gmDEGTORAD; 
    theta = (-theta+velang[1]-180.0f)*gmDEGTORAD;
    rho = gl_Info.NodeVel.length();
    
    newvel = VEC3( rho * ( sin(phi)* cos(theta) ),
		           rho * ( sin(phi)* sin(theta) ),
		           rho * ( cos(phi) ) );

    gl_Editor.EditNodeVel( newvel, FALSE );

    gl_CamHook.ang = VEC3( CamEnt->owner->client->v_angle );
}


void CamPreThinkEditVel2( USERENT *CamEnt )
{
    float	phi, theta, rho, len;
    VEC3	velang, newvel;

    rho  = gl_CamHook.ang[1] - CamEnt->owner->client->v_angle[1];
    
    // hack to prevent angle jumping

    if( fabs(rho) > 90.0f ) rho = 0.0f;

    velang = gl_Info.NodeVel.VecToAngle();
    phi = (velang[0]-90.0f)*gmDEGTORAD; 
    theta = (velang[1]-180.0f)*gmDEGTORAD;
    len = gl_Info.NodeVel.length();
    
    if( len == 0.0f ) 
    {
        if( rho > 0.2f )
        {
            gl_Editor.EditNodeVel( VEC3( 1.0f, 0.0f, 0.0f ), FALSE );
        }
    }
    else
    {
        rho += len;
        if( rho > 0.0f )
        {
            newvel = VEC3( rho * ( sin(phi)* cos(theta) ),
		                   rho * ( sin(phi)* sin(theta) ),
		                   rho * ( cos(phi) ) );

            gl_Editor.EditNodeVel( newvel, FALSE );
        }
    }


    gl_CamHook.ang = VEC3( CamEnt->owner->client->v_angle );
}


void CamPreThinkEditTime( USERENT *CamEnt )
{
    float dt;

    dt = gl_CamHook.ang[1] - CamEnt->owner->client->v_angle[1];
    if( fabs(dt) < 90.0f )
    {
        dt /= 10.0f;
        dt += gl_Info.NodeTime;
        if( dt > 0.0f )
        {
            gl_Editor.SetNodeTime( dt );
        }
    }

    gl_CamHook.ang = VEC3( CamEnt->owner->client->v_angle );
}   


static float playback_oldtime;

void CamPreThinkPlayback( USERENT *CamEnt )
{
	static float cur_time;

	cur_time = gl_pState->time;
    if( gl_Script.Iterate( cur_time - playback_oldtime ) == SCRIPT_STOPPED ) gl_bRestoreCamera = TRUE;
	playback_oldtime = cur_time;

    gl_CamHook.pos = gl_Script.GetPos();
    gl_CamHook.oldang = gl_CamHook.ang = gl_Script.GetAng();

//    gl_pState->SetOrigin( CamEnt, gl_CamHook.pos[0], gl_CamHook.pos[1], gl_CamHook.pos[2] );
	gl_CamHook.pos.copyTo (CamEnt->s.origin);
	gl_pState->LinkEntity (CamEnt);
}


VEC3 ClipCamera( const VEC3 &TargetPosition, const VEC3 &TargetLookAt, USERENT *NoClipEnt )
{
    float f1[3], f2[3], len;
    userTrace_t Trace;
    VEC3		vOfs;

    TargetPosition.copyTo(f1);
    TargetLookAt.copyTo(f2);
    gl_pState->TraceLine( f2, f1, TRUE, NoClipEnt, &Trace );

    //return ( VEC3(Trace.endpos) + 10*((TargetLookAt-TargetPosition).normalize()) );

    vOfs = VEC3(Trace.planeNormal);
    len = vOfs.length();
    if( len > gmEPSILON )
    {
        vOfs *= 10.0f/len;
    }

    return ( VEC3(Trace.endpos) + vOfs );
}

void CamPreThinkMario( USERENT *CamEnt )
{
    VEC3	vPlayer = VEC3 ( CamEnt->owner->s.origin ), vCam, vDiff, vPlayerAng;
    float	fYaw, fTilt, sintilt, fDistToPlayer;

    vPlayerAng = VEC3( CamEnt->owner->s.angles );
    fYaw = gmDEGTORAD * ( vPlayerAng[1] + 180.0f );
    fTilt = gmDEGTORAD * ( vPlayerAng[0] + 70.0f );

    sintilt = sin( fTilt );
    vCam = vPlayer + VEC3( 128.0f*cos(fYaw)*sintilt, 
                           128.0f*sin(fYaw)*sintilt,
                           128.0f*cos(fTilt) );
    vCam = ClipCamera( vCam, vPlayer, CamEnt->owner );
    
    vDiff = vPlayer - gl_CamHook.pos; // vector points to player
    fDistToPlayer = vDiff.length();
//    gl_pState->Con_Printf( "dist = %f\n", vDiff.length() );

#ifdef NEW
    if( vPlayer != gl_CamHook.oldpos )
    {
        // player is moving

        gl_CamHook.oldpos = vPlayer;
    }
    else
    {
        // player is still

        gl_CamHook.cubicPos.update( vCam, 5.0f );
    }
#endif

    if( ( vPlayer != gl_CamHook.oldpos ) || ( vPlayerAng != gl_CamHook.ang ) )
    {
        gl_CamHook.oldpos = vPlayer;
        gl_CamHook.ang = vPlayerAng;

        if( fDistToPlayer > 160.0 )
        {
            gl_CamHook.cubicPos.update( vCam, 0.6f );
            gl_pState->Con_Printf( "update=0.6\n" );
        }
        else
        {
            gl_CamHook.cubicPos.update( vCam, 4.0f );
            gl_pState->Con_Printf( "update=4.0\n" );
        }

    }

    gl_CamHook.cubicPos.iterate( gl_pState->frametime );    
    gl_CamHook.pos = gl_CamHook.cubicPos.getPos();

    gl_pState->SetOrigin( CamEnt, gl_CamHook.pos[0], gl_CamHook.pos[1], gl_CamHook.pos[2] );

    if( fDistToPlayer > 160.0 ) gl_CamHook.cubicFocus.update( vPlayer, 0.15 );
    else gl_CamHook.cubicFocus.update( vPlayer, 0.5 );
    gl_CamHook.cubicFocus.iterate( gl_pState->frametime );

    gl_CamHook.oldang = (gl_CamHook.cubicFocus.getPos() - gl_CamHook.pos ).VecToAngle();
}


//////////////////
//
//  CamPostThink
//  ------------
//

void CamPostThink( USERENT *CamEnt )
{
    CameraSendViewAngle( CamEnt->owner, gl_CamHook.oldang );
}


/////////////////
// 
//  CameraSpawn
//  -----------
//

void CameraSpawn( USERENT *Target, int iCamType )
{
    USERENT *CamEnt;
    playerHook_t *PlayerHook;
    VEC3	norm, v1,v2,v3;
    VEC3	newpos;

    CamEnt = gl_pState->SpawnEntity();
    assert( CamEnt != NULL );

    // (NEL) Give the client a pointer to the camera.

    PlayerHook = (playerHook_t *) Target->userHook;
    PlayerHook->camera = CamEnt;
    
    CamEnt->solid        = SOLID_NOT;
    CamEnt->movetype     = MOVETYPE_NOCLIP;
    CamEnt->className    = "camera2";
    CamEnt->flags        = FL_CLIENT;
    CamEnt->owner        = Target;           // (NEL) who started camera
    CamEnt->goalentity   = Target;           // (NEL) entity camera is following
    CamEnt->s.effects    = 0;
/*
    CamEnt->view_ofs[0]  = 0.0f;
    CamEnt->view_ofs[1]  = 0.0f;
    CamEnt->view_ofs[2]  = 0.0f;
*/
	//	Q2FIX
	CamEnt->viewheight = 0.0F;

    gl_pState->SetModel  ( CamEnt, "models/debug/curnode.mdl" );
    gl_pState->SetSize   ( CamEnt, 0, 0, 0, 0, 0, 0 );

    CamEnt->userHook = &gl_CamHook;
    gl_CamHook.iType = iCamType;
    
    switch( iCamType )
    {
    case CAMENT_EDITPOS:
        gl_CamHook.pre_think    = CamPreThinkEditPos;
        break;
    case CAMENT_EDITVEL:
        gl_CamHook.pre_think    = CamPreThinkEditVel;
        break;
    case CAMENT_EDITTIME:
        gl_CamHook.pre_think    = CamPreThinkEditTime;
        break;
    case CAMENT_PLAYBACK:
		playback_oldtime		= gl_pState->time;
        gl_CamHook.pre_think    = CamPreThinkPlayback;
        break;
    case CAMENT_MARIOTIME:
        gl_CamHook.pre_think    = CamPreThinkMario;
        gl_CamHook.cubicPos     = CUBIMOVE( Target->s.origin, Target->s.origin, VEC3(), VEC3(), 1.0, 0.0 );
        gl_CamHook.cubicFocus   = gl_CamHook.cubicPos;
        break;
    default:
        gl_CamHook.pre_think    = CamPreThinkDefault;
    }
    gl_CamHook.post_think   = CamPostThink;
    gl_CamHook.pos          = VEC3( Target->s.origin );

	if (iCamType == CAMENT_EDITVEL)
		gl_CamHook.pos [2] += 22.0F;

    gl_CamHook.oldang = gl_CamHook.ang = VEC3( Target->client->v_angle );
    gl_CamHook.oldang[0] = -gl_CamHook.oldang[0];

    norm[0] = gl_CamHook.ang[0];
    norm[1] = gl_CamHook.ang[1];
    norm[2] = gl_CamHook.ang[2];
    norm.AngleVecs( v1, v2, v3 );
    gl_CamHook.fwdvec = VEC3(&v1[0]).normalize();
    gl_CamHook.rightvec = VEC3(&v2[0]).normalize();
    gl_CamHook.upvec = VEC3(&v3[0]).normalize();

    gl_pState->SetOrigin2 ( CamEnt, &gl_CamHook.pos [0]);

	if (iCamType == CAMENT_EDITVEL)
	{
		if (PlayerHook->weapon)
			PlayerHook->weapon->s.renderfx |= RF_NODRAW;
		if (PlayerHook->head)
			PlayerHook->head->s.renderfx |= RF_NODRAW;
		if (PlayerHook->torso)
			PlayerHook->torso->s.renderfx |= RF_NODRAW;
		if (PlayerHook->legs)
			PlayerHook->legs->s.renderfx |= RF_NODRAW;
	}
	else
	{
		newpos = gl_CamHook.pos - 22.0f * gl_CamHook.fwdvec;

	    gl_pState->SetOrigin( Target, newpos[0], newpos[1], newpos[2] );
	}

	//	Lock view to camera
	gl_pState->SetCameraState (CamEnt->owner, true, CAMFL_LOCK_X + CAMFL_LOCK_Y + CAMFL_LOCK_Z);    

    gl_pState->SetClientViewEntity( Target, CamEnt );
    gl_pState->SetClientInputEntity( Target, CamEnt->owner );
}


///////////////////
// 
//  CameraDestroy
//  -------------
//

void CameraDestroy( USERENT *Self )
{
    playerHook_t *PlayerHook = (playerHook_t *) Self->userHook;

    if( PlayerHook->camera )
    {
        USERENT *CamEnt  = PlayerHook->camera;

		gl_pState->SetCameraState (CamEnt->owner, false, 0);

        gl_pState->SetClientViewEntity (Self, CamEnt->owner);
        gl_pState->SetClientInputEntity (Self, CamEnt->owner);

        CameraSendViewAngle ( Self, VEC3 (gl_CamHook.oldang));

		if	(gl_CamHook.iType != CAMENT_EDITVEL)
		{
			gl_pState->SetOrigin( CamEnt->owner, 
				gl_CamHook.pos[0], gl_CamHook.pos[1], gl_CamHook.pos[2] );
		}

        gl_pState->RemoveEntity( CamEnt );
        PlayerHook->camera = NULL;

		if (PlayerHook->weapon)
			PlayerHook->weapon->s.renderfx &= ~RF_NODRAW;
		if (PlayerHook->head)
			PlayerHook->head->s.renderfx &= ~RF_NODRAW;
		if (PlayerHook->torso)
			PlayerHook->torso->s.renderfx &= ~RF_NODRAW;
		if (PlayerHook->legs)
			PlayerHook->legs->s.renderfx &= ~RF_NODRAW;
    }
}


void CameraSwitch()
{
    switch( gl_CamHook.iType )
    {
    case CAMENT_EDITPOS: 
        gl_CamHook.pre_think = CamPreThinkEditPos2; 
        gl_CamHook.iType = CAMENT_EDITPOS2;
        break;
    case CAMENT_EDITPOS2:
        gl_CamHook.pre_think = CamPreThinkEditPos; 
        gl_CamHook.iType = CAMENT_EDITPOS;
        break;
    case CAMENT_EDITVEL:
        gl_CamHook.pre_think = CamPreThinkEditVel2;
        gl_CamHook.iType = CAMENT_EDITVEL2;
        break;
    case CAMENT_EDITVEL2:
        gl_CamHook.pre_think = CamPreThinkEditVel;
        gl_CamHook.iType = CAMENT_EDITVEL;
        break;
    }
}