#include <windows.h>
#include "katanox.h"
#include "anoxcam.h"
#include "util.h"


//--------------------
// CAMNODE::CAMNODE()
//
// Default constructor for class CAMNODE.  Initializes all things to zero.
//--------------------

CAMNODE::CAMNODE()
{
    ulFlags   = 0L;
    fPathTime = 0.0f;

    // VEC3's are already set to (0,0,0), so don't need to initialize

    fAccumTime = 0.0f;
    next = NULL;
    prev = NULL;
}


CAMNODE::CAMNODE( float t, unsigned long flags, const VEC3& pos_f, const VEC3& vel_f )
{
    fPathTime = t;
    ulFlags = flags;
    
    if( flags & NODEFLAG_MOVEREL )
    {
        vRelPos = pos_f;
    }
    else
    {
        vFinalPos = pos_f;
    }

    vFinalVel = vel_f;

    fAccumTime = 0.0f;
    next = NULL;
    prev = NULL;
}


//--------------------
// CAMNODE::CAMNODE( const CAMNODE& )
//
// Copy constructor copies all definition information from another node, 
// EVEN ITS LIST LINKS.  This is basically for the purpose of replacing
// nodes in a list during editing.
//
// However, since all the definition variables are public, I don't see why
// anybody would actually need to use this method....
//--------------------

CAMNODE::CAMNODE( const CAMNODE &c )
{
    *this = c;
}


//--------------------
// CAMNODE::FastInit();
//
// Don't use FastInit unless you have called Init(VEC3,VEC3) at least once
// with valid data to initialize the cubic movement and other setup.
//--------------------

void CAMNODE::FastInit()
{
    fAccumTime = 0.0f;
}


//--------------------
// CAMNODE::Init( const VEC3&, const VEC3& );
//
// Init() is slower than FastInit(), but must be called at least once on
// all nodes in the script to make sure that the cubic movement and other
// setup has been performed.
//
// Note that a node's cubic movement is affected only by the node exactly
// before it, where it gets is initial position and velocity, so whenever
// during script editing a node is modified, the Init() function of the
// next node must be called. 
//
// Note also that this must be performed only on the node after the modified
// node, and not any subsequent nodes.
//
// If a node has no previous nodes (it is the first node), then you must
// manually pass Init() the initial position and velocity, otherwise it can
// automatically get the information it requires from the previous node.
//--------------------


void CAMNODE::Init() // use for any nodes after first node
{
    FastInit();

    if( prev )
    {
        Init( prev->vFinalPos, prev->vFinalVel );
    }
}


void CAMNODE::Init( const VEC3& pos_i, const VEC3& vel_i ) // use for first node
{
    FastInit();
    pos = pos_i;
    vel = vel_i;

    if( ulFlags & NODEFLAG_MOVEREL )
    {
        vFinalPos = pos + vRelPos;
    }

    //-------------------------------
    // CUBIMOVE cannot handle a path
    // of time length zero, so catch
    // for this problem.
    //-------------------------------

    if( fPathTime > 0.0f )
    {
        InitCubic();
    }
}


//--------------------
// CAMNODE::Iterate( float );
//
// Returns the time left in the path.  If there is a nonpositive amount of time
// left then pos and vel will stay at vFinalPos and vFinalVel.
//--------------------

float CAMNODE::Iterate( float frametime )
{
    float fTimeLeft;

    fAccumTime += frametime;
    fTimeLeft = fPathTime - fAccumTime;

    if( fTimeLeft > 0.0f )
    {
        IterateCubic();
    }
    else
    {
        pos = vFinalPos;
        vel = vFinalVel;
    }

    return fTimeLeft;
}


//--------------------
// CAMNODE::InitCubic();
//
// Uses current position, velocity, and total pathtime to calculate the 
// coefficients for the cubic spline movement.
//--------------------

void CAMNODE::InitCubic()
{
    float t = fPathTime;

    v3_j = (6.0f/(t*t))*(vFinalVel+vel-(2.0f/t)*(vFinalPos-pos));
    v3_ai = (vFinalVel-vel)/t-0.5f*t*v3_j;
    v3_si = pos;
    v3_vi = vel;
}


//--------------------
// CAMNODE::IterateCubic();
//
//--------------------

void CAMNODE::IterateCubic()
{
    float t, t2;

    t = fAccumTime;
    t2 = 0.5f*t*t;

    pos = v3_si + (v3_vi*t)+(v3_ai*t2)+(v3_j*(t2*t/3.0f));
    vel = v3_vi + (v3_ai*t)+(v3_j*t2);
}


