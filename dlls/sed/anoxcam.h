#ifndef ANOXCAM_H
#define ANOXCAM_H

#include "katanox.h"

//---------------------------------------------------------------------------
// ANOXCAM - camera scriptor for Anachronox (and maybe Daikatana)
// (c) 1997 Ion Storm
//
// Author: Joey Liaw, Halloween 1997
// Query: Why am I working in the office on a holiday?
//
// N.B. : Angle vectors in quake are (pitch,yaw,roll) instead of the normal
//        YPR the rest of the world uses.  Oh well.
//
// N.B. : The only rounding errors I think might ever come into play is from
//        passing frametime every frame instead of absolute time.  However,
//        this way it is easy to facilitate pausing of the script and what
//        not.  I don't think it should be too much of a problem.
//
// Terminology:
//
// create
//     the moment an object gets memory allocated to it, and its
//     constructors (if any) are called
// define
//     the moment the parameters of an object are set and it is
//     ready to be used.  This may not necessarily happen right
//     after creation.
// destroy
//     the moment an object releases all its dependencies' memories and
//     is itself destroyed after its destructors (if any) are called.
// execute / iterate
//     when a node is currently "playing" in a script
// focus
//     i don't mean the camera can focus in and out, the "focal point"
//     or "focus" of a camera is the point in the center of the view.
// unlink
//     when an object is deleted from a list, but its memory is not yet
//     reclaimed.
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//                        Random useful #defines
//---------------------------------------------------------------------------

#define MAXNAMELEN              40 // maximum length of cam/script name


//---------------------------------------------------------------------------
// #define NODEFLAG_* : flags for use in class CAMNODE.ulFlags
//
// NODEFLAG_MOVEREL 
//     (ON)  The specified final position is relative to the initial
//           position specified at script execute time.  The final 
//           position is added to the initial position to find the 
//           absolute coordinates.
//     (OFF) The final position is in absolute coordinates.
//
//     Note: Say you wanted to make the camera just sit there.
//           Just make a node with a positive path time, set 
//           NODEFLAG_MOVEREL to (ON) and set the relative movement 
//           to VEC3(0,0,0).
//
// NODEFLAG_MAKECURRENT 
//     (ON)  Since multiple cameras can be running at the same 
//           time, there has to be some mechanism for switching 
//           the view between cameras.  When a camera encounters 
//           a node with this flag, it notifies the parent script 
//           to change views.
//
// NODEFLAG_ENDSCRIPT 
//      (ON) Forces the script to end abruptly once the node has
//           completed executing.  Normally a script will consider
//           itself "finished" when all of its cameras have come 
//           to the natural end of their node lists.
//
// Note if a node is both ENDSCRIPT and MAKECURRENT and has a time length of
// zero, then it will never become the current view.
//
//---------------------------------------------------------------------------

//!!!!!!!! Any changes to flags must be reflected in NodeFlagNames !!!!!!!!//
//!!!!!!!! in SCRIPTDEF.CPP !!!                                    !!!!!!!!//

#define NODEFLAG_MOVEREL         0x00000001
#define NODEFLAG_MAKECURRENT     0x00000002
#define NODEFLAG_ENDSCRIPT       0x00000004

//---------------------------------------------------------------------------
//                            class CAMNODE
//---------------------------------------------------------------------------


class CAMERA; // forward declaration
class GLCAMEDIT; // forward declaration

class  CAMNODE {

    CAMNODE();
    CAMNODE( float, unsigned long, const VEC3&, const VEC3& );
    CAMNODE( const CAMNODE& ); // copy constructor

private:
    
    void             FastInit();
    void             Init();
    void             Init( const VEC3&, const VEC3& );
    float            Iterate( float );

    friend class CAMERA;
    friend class JSCRIPT;
    friend class GLCAMEDIT;

    // definition vars
    unsigned long    ulFlags;
    float            fPathTime;
    VEC3             vRelPos;
    VEC3             vFinalPos;
    VEC3             vFinalVel;

    // linkage
    CAMNODE         *next;
    CAMNODE         *prev;

    // execute vars
    VEC3             pos, vel;
    float            fAccumTime;
    
    // cubic movement
    void             InitCubic();
    void             IterateCubic();
    VEC3             v3_si, v3_vi, v3_ai, v3_j;

};


//---------------------------------------------------------------------------
//                              class CAMERA
//                          (CAMNODE management)
//
// Note that all handling of angle-related data is handled by the camera.
// CamFocus is a pointer to the camera whose position is always the center
// of the current camera's view.
//---------------------------------------------------------------------------

#define CAMSTATE_IDLE           0 // camera finished all nodes
#define CAMSTATE_ENDSCRIPT      1 // stop the script now!
#define CAMSTATE_MAKECURRENT    2 // make this camera current view
#define CAMSTATE_NORMAL         3 // playing

class  CAMERA {
    
public:
    CAMERA();
    ~CAMERA();

private:

    void            DeleteAllNodes();

    void            AppendNode( CAMNODE * );
    void            InsertNode( CAMNODE * );
    void            DeleteNode();
    void            PrevNode();
    void            NextNode();
    void            FirstNode();
    void            LastNode();

    void            Init();
    void            Init( const VEC3&, const VEC3& );
    int             Iterate( float );

    // data
    char            name[MAXNAMELEN];
    CAMNODE         *NodeHead, *NodeTail, *NodeCurrent;
    int             iCamFocus;

    int             iThisNode, iTotalNodes;

    VEC3            pos, vel;
    CAMNODE         NodeDataCopy;

    friend GLCAMEDIT;
    friend JSCRIPT;
};


//---------------------------------------------------------------------------
//                              class JSCRIPT
//                       (multiple camera management)
//---------------------------------------------------------------------------

#define MAXCAMERAS 20

#define SCRIPT_STOPPED  0
#define SCRIPT_PLAYING  1

class  JSCRIPT {

public:
    JSCRIPT();
    ~JSCRIPT();
    
    void            Reset();

    void            Init( const VEC3&, const VEC3& );
    int             Iterate( float );

    int             SaveScript( char *filename );
    int             LoadScript( char *filename );

    VEC3            GetPos();
    VEC3            GetAng();

private:
    int             ParseLine( int, char * );

    // data
    char            name[MAXNAMELEN];
    CAMERA          *Camera;
    int             iActiveCam;
    VEC3            pos, vel, ang;

    friend GLCAMEDIT;
};


//---------------------------------------------------------------------------
//                       miscellaneous functions
//---------------------------------------------------------------------------

char *GetScriptError();

#endif
