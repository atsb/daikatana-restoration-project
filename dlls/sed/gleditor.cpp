#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "katanox.h"
#include "anoxcam.h"
#include "gleditor.h"
#include "glsub.h"

#define NODERADIUS  3.0f
#define FOCUSRADIUS 3.0f
#define MARKERRADIUS 3.0f

static const char *ShowFlagNames[] = 
{
    "draw camera",
    "draw near path",
    "draw all paths",
    "draw nodes",
    "draw focus info",
    "draw all cameras",
};


GLCAMEDIT::GLCAMEDIT()
{
    Slave = NULL;
    ulShowFlags = 0L;
    ulSavedFlags = 0L;
    fPathDensity = 1.0f;
    fVelColorScale = 400.0f;
}


GLCAMEDIT::~GLCAMEDIT()
{
}


void GLCAMEDIT::Show( DWORD dwFlag )
{
    ulShowFlags |= dwFlag;
}


void GLCAMEDIT::Hide( DWORD dwFlag )
{
    ulShowFlags &= (~dwFlag);
}


void GLCAMEDIT::Toggle( DWORD dwFlag )
{
    if( dwFlag == GLCAM_EVERYTHING )
    {
        if( ulShowFlags == GLCAM_EVERYTHING )
        {
            ulShowFlags = ulSavedFlags;
        }
        else
        {
            ulSavedFlags = ulShowFlags;
            ulShowFlags = GLCAM_EVERYTHING;
        }
    }
    else
    {
        ulShowFlags ^= dwFlag;
    }
}


void GLCAMEDIT::NextCamera()
{
    if( Slave == NULL ) return;

    Slave->iActiveCam = (Slave->iActiveCam + 1) % MAXCAMERAS;
}


void GLCAMEDIT::PrevCamera()
{
    if( Slave == NULL ) return;

    if( Slave->iActiveCam == 0 )
    {
        Slave->iActiveCam = MAXCAMERAS-1;
    }
    else Slave->iActiveCam--;
}


void GLCAMEDIT::NextNode()
{
    if( Slave == NULL ) return;

    Slave->Camera[Slave->iActiveCam].NextNode();
}


void GLCAMEDIT::PrevNode()
{
    if( Slave == NULL ) return;

    Slave->Camera[Slave->iActiveCam].PrevNode();
}


void GLCAMEDIT::FirstNode()
{
    if( Slave == NULL ) return;
    Slave->Camera[Slave->iActiveCam].FirstNode();
}


void GLCAMEDIT::LastNode()
{
    if( Slave == NULL ) return;
    Slave->Camera[Slave->iActiveCam].LastNode();
}


void GLCAMEDIT::InsertNode( float t, DWORD flags, const VEC3& p, const VEC3& v )
{
    if( Slave == NULL ) return;

    Slave->Camera[Slave->iActiveCam].
        InsertNode( new CAMNODE(t,flags,p,v) );
}


void GLCAMEDIT::AppendNode( float t, DWORD flags, const VEC3& p, const VEC3& v )
{
    if( Slave == NULL ) return;

    Slave->Camera[Slave->iActiveCam].
        AppendNode( new CAMNODE(t,flags,p,v) );
}


void GLCAMEDIT::DeleteNode()
{
    if( Slave == NULL ) return;

    Slave->Camera[Slave->iActiveCam].DeleteNode();
}


void GLCAMEDIT::SetInitial( const VEC3& pi, const VEC3& vi )
{
    initpos = pi;
    initvel = vi;
}


void GLCAMEDIT::AttachScript( JSCRIPT *pscript )
{
    Slave = pscript;
}


void GLCAMEDIT::DrawPath( const CAMERA *cam1, const CAMERA *cam2, CAMNODE *endnode, VEC3& col ) // cam must already be initted
{
    static CAMERA TempCam, FocusCam;
    int camstate1, camstate2;
    float v, ofs;
    VEC3 lowcolor, diff;

    TempCam = *cam1;
    if( cam2 ) FocusCam = *cam2;

    ofs = fmod( gl_fCycleTime, fPathDensity );
    
    for( TempCam.Iterate( ofs ), FocusCam.Iterate( ofs ),
         camstate1 = camstate2 = CAMSTATE_NORMAL;
         ((camstate1 != CAMSTATE_IDLE)||(camstate2 != CAMSTATE_IDLE)) && ( TempCam.NodeCurrent != endnode );
         camstate1 = TempCam.Iterate( fPathDensity ), camstate2 = FocusCam.Iterate( fPathDensity ) )
    {
        v = TempCam.vel.length();
        lowcolor[2] = v/fVelColorScale; lowcolor[0] = 1-lowcolor[2];
        DrawMarker( TempCam.pos, MARKERRADIUS, col, lowcolor );

        if( cam2 )
        {
            v = FocusCam.vel.length();
            lowcolor[2] = v/fVelColorScale; lowcolor[0] = 1-lowcolor[2];
            DrawMarker( FocusCam.pos, MARKERRADIUS, VEC3( 0.5f, 1.0f, 0.5f ), lowcolor );

            if( ulShowFlags & GLCAM_SHOWARROWS )
            {
                diff = 15.0f*(FocusCam.pos-TempCam.pos).normalize();
                DrawArrow( TempCam.pos + diff, diff.VecToAngle(), VEC3( 0.0f, 0.85f, 0.06f ) );
            }
        }
    }

    TempCam.NodeHead = TempCam.NodeTail = NULL;
    FocusCam.NodeHead = FocusCam.NodeTail = NULL;
}


//--------------------
// GLCAMEDIT::Draw()
//
// Main draw loop for the camera editor.
//--------------------

void GLCAMEDIT::Draw( float frametime )
{
    if( Slave == NULL ) return;

    gl_fCycleTime += frametime;

  int bFocalOn;
  float tempcolor[3];
  CAMNODE *pCurrentNode = Slave->Camera[Slave->iActiveCam].NodeCurrent;
  CAMNODE *nptr;
  static CAMERA TempCamMain, TempCamFocus;
  VEC3 currpos, currvel;

    if( pCurrentNode )
    {
        currpos = pCurrentNode->vFinalPos;
        currvel = pCurrentNode->vFinalVel;
    }

    TempCamMain  = Slave->Camera[Slave->iActiveCam];

    if( ( TempCamMain.iCamFocus >= 0 ) && ( ulShowFlags & GLCAM_FOCUSPOINT ) )
    {
        bFocalOn = TRUE;
        TempCamFocus = Slave->Camera[TempCamMain.iCamFocus];
    }
    else bFocalOn = FALSE;   
    
    //-----------------
    // Draw the camera
    //-----------------

    if( ulShowFlags & GLCAM_CAMERAPOS )
    {
        DrawCameraObject( campos, camang );

        // now draw same for focal point
        if( bFocalOn )
        {
            tempcolor[0] = tempcolor[2] = 0.0f;
            tempcolor[1] = 1.0f;
            DrawIcosahedron( focpos, tempcolor, FOCUSRADIUS );
        }
    }

    //---------------------------
    // Draw paths --
    // If entirepath, then don't 
    //  redraw adjecent path.
    //---------------------------

    if( ulShowFlags & GLCAM_ENTIREPATH )
    {
        // now draw same for focal point
        if( bFocalOn )
        {
            TempCamMain.Init( initpos, initvel );
            TempCamFocus.Init( initpos, initvel );
            DrawPath( &TempCamMain, &TempCamFocus, NULL, VEC3(0.6,1.0,0.6) );            
        }
        else
        {
            TempCamMain.Init( initpos, initvel );
            DrawPath( &TempCamMain, NULL, NULL, VEC3(1.0,1.0,1.0) );
        }


        if( ulShowFlags & GLCAM_NODEPOS )
        {
            // draw all other nodes in gray

            nptr = TempCamMain.NodeHead;
            tempcolor[0] = tempcolor[1] = tempcolor[2] = 0.5f;

            while( nptr )
            {
                if( nptr != pCurrentNode ) DrawIcosahedron( nptr->vFinalPos, tempcolor, NODERADIUS );
                nptr = nptr->next;
            }

            if( bFocalOn )
            {
                nptr = TempCamFocus.NodeHead;
                tempcolor[0] = tempcolor[2] = 0.2;
                tempcolor[1] = 0.6;

                while( nptr )
                {
                    if( nptr != pCurrentNode ) DrawIcosahedron( nptr->vFinalPos, tempcolor, NODERADIUS );
                    nptr = nptr->next;
                }
            }
        }

    }
    else if( ulShowFlags & GLCAM_ADJACENTPATH )
    {
        if( pCurrentNode )
        {
          float c1[3]={1.0,1.0,1.0}, c2[3]={0.0,0.0,0.0};
          CAMNODE *endnode;

            TempCamMain.NodeCurrent = nptr = pCurrentNode;
            nptr->Init();
            if( nptr->next ) endnode = nptr->next->next;
            else endnode = NULL;

            TempCamMain.Init();
            DrawPath( &TempCamMain, NULL, endnode, VEC3( 1.0, 1.0, 1.0 ) );

            // since we don't keep detailed enough data on 
            // the focus camera, then we cannot display
            // a localized part of the focus path.
            // (but we can draw the entire focus path when
            //  GLCAM_ENTIREPATH is set)

        } // if(pCurrentNode)
    }


    //------------------------------------
    // Draw node positions and velocities
    //------------------------------------

    if( pCurrentNode )
    {
        if( ulShowFlags & GLCAM_NODEPOS )
        {
            // draw current node in white

            tempcolor[0] = tempcolor[1] = tempcolor[2] = 1.0f;
            DrawIcosahedron( pCurrentNode->vFinalPos, tempcolor, NODERADIUS );
            tempcolor[0] = 1.0f; tempcolor[1] = 1.0f; tempcolor[2] = 0.0f;
            if( pCurrentNode->vFinalVel != VEC3() )
            {
                DrawArrow( pCurrentNode->vFinalPos + 0.2*pCurrentNode->vFinalVel, 
                    pCurrentNode->vFinalVel.VecToAngle(), tempcolor );
            }
            else
            {
                DrawArrow( pCurrentNode->vFinalPos, VEC3().VecToAngle(), VEC3(0.5f,0.0f,0.0f) );
            }

            if( pCurrentNode->fPathTime == 0.0f )
            {
                if( pCurrentNode->prev )
                {
                    VEC3 arrowpos = 15.0f*((currpos-pCurrentNode->prev->vFinalPos).normalize());
                    DrawArrow( currpos - arrowpos, arrowpos.VecToAngle(), VEC3( 0.26f,0.55f,0.88f ) );
                }
            }
        }
    }


    // do this so the destructor doesn't try to
    // release memory
    
    TempCamMain.NodeHead = TempCamMain.NodeTail = NULL;
    TempCamFocus.NodeHead = TempCamFocus.NodeTail = NULL;
}


void GLCAMEDIT::GetScriptInfo( SCRIPTINFO *s )
{
    memset( s, 0, sizeof(SCRIPTINFO) );
    if( Slave == NULL ) return;

  CAMNODE *np = Slave->Camera[Slave->iActiveCam].NodeCurrent, *np2;

    s->NodeFlags[0] = 0x00;
    s->NodeAccumTime = 0.0f;

    if( np == NULL )
    {
        s->bInfoValid = FALSE;
        strcpy( s->NodeFlags, "NULL" );
        s->NodeTime = 0.0f;
    }
    else
    {
        s->bInfoValid = TRUE;

        s->NodeVel = np->vFinalVel;
        s->NodeVelAng = s->NodeVel.VecToAngle();
        s->NodeVelAng[2] = s->NodeVelAng[1];
        s->NodeVelAng[1] = s->NodeVelAng[0];
        s->NodeVelAng[0] = np->vFinalVel.length();
        s->NodeTime = np->fPathTime;

        // fill in flags

        if( np->ulFlags & NODEFLAG_MOVEREL )
        {
            s->NodePos = np->vRelPos;
            strcat( s->NodeFlags, "REL " );
        }
        else
        {
            s->NodePos = np->vFinalPos;
        }

        if( np->ulFlags & NODEFLAG_MAKECURRENT )
        {
            strcat( s->NodeFlags, "VIEW " );
        }

        if( np->ulFlags & NODEFLAG_ENDSCRIPT )
        {
            strcat( s->NodeFlags, "END " );
        }

        // stuff that requires going through the whole list

        np2 = Slave->Camera[Slave->iActiveCam].NodeHead;

        while( np2 )
        {
            s->NodeAccumTime += np2->fPathTime;
            if( np == np2 ) break;
            np2 = np2->next;
        }
    }

    // always fill in the following info

    s->NodeDensity = fPathDensity;
    s->VelRed = 0;
    s->VelBlue = fVelColorScale;
    s->CamNum = Slave->iActiveCam;
    s->CamFocus = Slave->Camera[Slave->iActiveCam].iCamFocus;
}


void GLCAMEDIT::ScaleDensity( float factor )
{
    if( factor > 0.0f )
    {
        fPathDensity *= factor;
    }
}


void GLCAMEDIT::SetNodeTime( float t )
{
    if( Slave == NULL ) return;

  CAMNODE *np = Slave->Camera[Slave->iActiveCam].NodeCurrent;

    if( np ) np->fPathTime = t;
}


void GLCAMEDIT::EditNodePos( const VEC3& p, int bIsRelative )
{
    if( Slave == NULL ) return;
  CAMNODE *np = Slave->Camera[Slave->iActiveCam].NodeCurrent;
    if( np == NULL ) return;

    if( np->ulFlags & NODEFLAG_MOVEREL )
    {
        if( bIsRelative ) np->vRelPos += p;
        else np->vRelPos = p;
    }
    else
    {
        if( bIsRelative ) np->vFinalPos += p;
        else np->vFinalPos = p;
    }

    np->Init();
}


void GLCAMEDIT::EditNodeVel( const VEC3& v, int bIsRelative )
{
    if( Slave == NULL ) return;
  CAMNODE *np = Slave->Camera[Slave->iActiveCam].NodeCurrent;
    if( np == NULL ) return;

    
    if( bIsRelative ) np->vFinalVel += v;
    else np->vFinalVel = v;
}
