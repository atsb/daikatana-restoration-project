#ifndef KATANOX_H
#define KATANOX_H

#pragma warning( disable : 4244 )

#include	<iostream.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<math.h>

#include	"p_user.h"

//-------------------------------JCL-
// Katana / Anachronox DLL Interface
// October 1997
//-----------------------------------

#define DLL_INVALID     0
#define DLL_VERSION     1
#define DLL_INIT        2
#define DLL_LOAD        3
#define DLL_EXIT        4
#define DLL_LOADLEVEL   5
#define DLL_EXITLEVEL   6

//--------------------------------------------------------------
// Inline Vector Class
// based on libgm++
//--------------------------------------------------------------

double const gm2PI =       6.28318530717958623200;
double const gmDEGTORAD =  0.01745329251994329547;
double const gmE =         2.71828182845904553488;
double const gmEEXPPI =   23.14069263277927390732;
double const gmGOLDEN =    1.61803398874989490253;
double const gmINVPI =     0.31830988618379069122;
double const gmLN10 =      2.30258509299404590109;
double const gmLN2 =       0.69314718055994528623;
double const gmLOG10E =    0.43429448190325187218;
double const gmLOG2E =     1.44269504088896338700;
double const gmPI =        3.14159265358979323846;
double const gmPIDIV2 =    1.57079632679489655800;
double const gmPIDIV4 =    0.78539816339744827900;
double const gmRADTODEG = 57.29577951308232286465;
double const gmSQRT2 =     1.41421356237309514547;
double const gmSQRT2PI=   2.50662827463100024161;
double const gmSQRT3 =     1.73205080756887719318;
double const gmSQRT10 =    3.16227766016837952279;
double const gmSQRTE =     1.64872127070012819416;
double const gmSQRTHALF =  0.70710678118654757274;
double const gmSQRTLN2 =   0.83255461115769768821;
double const gmSQRTPI =    1.77245385090551588192;
double const gmEPSILON =   1.0e-5; // adjust as necessary.  quake precision is low
double const gmGOOGOL =    1.0e50;

//
// Using asserts, NDEBUG is automatically defined when
// compiling the release version under MSVC
//

inline int gmIsZero(float c)
{
    return ( ((c>=0)?c:-c) < gmEPSILON );
}

inline int gmFuzEQ(float f, float g)
{
    return (f <= g) ? (f >= g - gmEPSILON) : (f <= g + gmEPSILON);
}

inline void gmClamp(float &f, float l, float h)
{
  if(f < l) f = l;
  if(f > h) f = h;
}

inline float gmSqr(float f)
{
  return f * f;
}

class VEC3 {
protected:
    union {
        float v_[3];
        float ang_[3]; // pitch, yaw, roll

        struct {
            float x_, y_, z_;
        };

        struct {
            float pitch_, yaw_, roll_;
        };
    };

public:
    VEC3();
    VEC3( const VEC3& );
    VEC3( float, float, float );
    VEC3( float[3] );

    // array access

    float& operator [] (int);
    const float& operator [] (int) const;

    // assignment

    VEC3& assign( float, float, float );
    VEC3& operator = (const VEC3&);

    // math

    VEC3& operator += (const VEC3&);
    VEC3& operator -= (const VEC3&);
    VEC3& operator *= (float);
    VEC3& operator /= (float);

    VEC3 operator + (const VEC3&) const;
    VEC3 operator - (const VEC3&) const;
    VEC3 operator - () const;
    VEC3 operator * (float) const;
    VEC3 operator / (float) const;

    friend VEC3 operator *( float, const VEC3& );

    int operator == (const VEC3&) const;
    int operator != (const VEC3&) const;

    // operations

    VEC3  VecToAngle() const;
    void  AngleVecs( VEC3& fwd, VEC3& right, VEC3& up );

    VEC3& clamp(float, float);
    float length() const;
    float lengthSq() const;
    VEC3& normalize();

    void copyTo(float [3]) const;
    void copyTo(double [3]) const;

    friend VEC3     cross       (const VEC3&, const VEC3&);
    friend float    distance    (const VEC3&, const VEC3&);
    friend float    distanceSq  (const VEC3&, const VEC3&);
    friend float    dot         (const VEC3&, const VEC3&);
    friend VEC3     lerp        (float, VEC3&, VEC3& );
    friend VEC3     elMult      (const VEC3&, const VEC3& ); // multiply elements
    friend VEC3     elDiv       (const VEC3&, const VEC3& ); // divide elements

    // debug

    friend ostream & operator << ( ostream&, const VEC3& );
};


//
// Constructors
//

inline VEC3::VEC3()
{
    v_[0] = v_[1] = v_[2] = 0.0;
}


inline VEC3::VEC3(const VEC3& v)
{
    v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2];
}


inline VEC3::VEC3(float x, float y, float z)
{
    v_[0] = x; v_[1] = y; v_[2] = z;
}


inline VEC3::VEC3(float v[3])
{
    v_[0] = v[0]; v_[1] = v[1]; v_[2] = v[2];
}


//
// Array access
//

inline float& VEC3::operator [](int i)
{
    assert( i==0 || i==1 || i==2 );
    return v_[i];
}


inline const float& VEC3::operator [](int i) const
{
    assert( i==0 || i==1 || i==2 );
    return v_[i];
}


//
// Assignemnt
//

inline VEC3& VEC3::assign( float x, float y, float z )
{
    v_[0] = x; v_[1] = y; v_[2] = z;
    return *this;
}


inline VEC3& VEC3::operator =(const VEC3& v)
{
    v_[0] = v[0]; v_[1] = v[1]; v_[2] = v[2];
    return *this;
}


//
// Math
//

inline VEC3& VEC3::operator +=(const VEC3& v)
{
    v_[0] += v[0]; v_[1] += v[1]; v_[2] += v[2];
    return *this;
}


inline VEC3& VEC3::operator -=(const VEC3& v)
{
    v_[0] -= v[0]; v_[1] -= v[1]; v_[2] -= v[2];
    return *this;
}


inline VEC3& VEC3::operator *=(float c)
{
    v_[0] *= c; v_[1] *= c; v_[2] *= c;
    return *this;
}


inline VEC3& VEC3::operator /=(float c)
{
    assert( ! gmIsZero(c) );
    v_[0] /= c; v_[1] /= c; v_[2] /= c;
    return *this;
}


inline VEC3 VEC3::operator +(const VEC3& v) const
{
    return VEC3(v_[0]+v[0], v_[1]+v[1], v_[2]+v[2]);
}


inline VEC3 VEC3::operator -(const VEC3& v) const
{
    return VEC3(v_[0]-v[0], v_[1]-v[1], v_[2]-v[2]);
}


inline VEC3 VEC3::operator -() const
{
    return VEC3(-v_[0], -v_[1], -v_[2]);
}


inline VEC3 VEC3::operator *(float c) const
{
    return VEC3(v_[0]*c, v_[1]*c, v_[2]*c);
}


inline VEC3 VEC3::operator /(float c) const
{
    assert( ! gmIsZero(c) );
    return VEC3( v_[0]/c, v_[1]/c, v_[2]/c );
}


inline VEC3 operator *(float c, const VEC3& v)
{
    return VEC3(c*v[0], c*v[1], c*v[2]);
}


inline int VEC3::operator ==(const VEC3& v) const
{
    return (gmFuzEQ(v_[0], v[0]) && 
            gmFuzEQ(v_[1], v[1]) && 
            gmFuzEQ(v_[2], v[2]));
}


inline int VEC3::operator !=(const VEC3& v) const
{
    return ( ! (*this == v) );
}


//
// Operations
//


inline VEC3& VEC3::clamp(float lo, float hi)
{
    gmClamp(v_[0], lo, hi); gmClamp(v_[1], lo, hi); gmClamp(v_[2], lo, hi);
    return *this;
}


inline float VEC3::length() const
{
    return sqrt(v_[0]*v_[0] + v_[1]*v_[1] + v_[2]*v_[2]);
}


inline float VEC3::lengthSq() const
{
    return v_[0]*v_[0] + v_[1]*v_[1] + v_[2]*v_[2];
}


inline VEC3& VEC3::normalize()
{
    float len = length();

	if (gmIsZero (len))
		return	*this;

//    assert( !gmIsZero(len) );
    *this /= len;
    return *this;
}


inline void VEC3::copyTo(float f[3]) const
{
    f[0] = v_[0]; f[1] = v_[1]; f[2] = v_[2];
}


inline void VEC3::copyTo(double f[3]) const
{
    f[0] = v_[0]; f[1] = v_[1]; f[2] = v_[2];
}


inline VEC3 cross( const VEC3& v1, const VEC3& v2 )
{
    return VEC3( v1[1]*v2[2] - v1[2]*v2[1],
                 v1[2]*v2[0] - v1[0]*v2[2],
                 v1[0]*v2[1] - v1[1]*v2[0] );
}


inline float distance( const VEC3& v1, const VEC3& v2 )
{
    return sqrt( gmSqr(v1[0]-v2[0]) + 
                 gmSqr(v1[1]-v2[1]) + 
                 gmSqr(v1[2]-v2[2]) );
}


inline float distanceSq( const VEC3& v1, const VEC3& v2 )
{
    return gmSqr(v1[0]-v2[0]) + 
           gmSqr(v1[1]-v2[1]) + 
           gmSqr(v1[2]-v2[2]) ;
}


inline float dot( const VEC3& v1, const VEC3& v2 )
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}


inline VEC3 lerp( float f, const VEC3& v1, const VEC3& v2 )
{
    return v1 + ( (v2-v1) * f );
}


//
// Debug
//

inline ostream & operator << ( ostream& os, const VEC3& v )
{
    os << "< " << v[0] << " " << v[1] << " " << v[2] << " >";
    return os;
}


//---------------------------------------------------------------
// Spline Classes (export)
//---------------------------------------------------------------


class BEZIER {
protected:
    VEC3 v0_, v1_, v2_, v3_;
    VEC3 dv1_, dv2_;

public:
    BEZIER();
    BEZIER( const BEZIER& );
    BEZIER( const VEC3&, const VEC3&, const VEC3&, const VEC3& );

    BEZIER& assign( const VEC3&, const VEC3&, const VEC3&, const VEC3& );
    BEZIER& operator = (const BEZIER&);

    VEC3 eval(float) const;         // evaluate spline
    VEC3 evalDer(float) const;      // evaluate first derivative of spline
};


inline BEZIER::BEZIER()
{
}


inline BEZIER::BEZIER( const BEZIER& b )
{
    v3_ = b.v3_; v2_ = b.v2_; v1_ = b.v1_; v0_ = b.v0_;
    dv1_ = b.dv1_; dv2_ = b.dv2_;
}


inline BEZIER::BEZIER( const VEC3& P1, const VEC3& P2, const VEC3& P3, const VEC3& P4 )
{
    assign( P1, P2, P3, P4 );
}


inline BEZIER& BEZIER::operator = ( const BEZIER& b )
{
    v3_ = b.v3_; v2_ = b.v2_; v1_ = b.v1_; v0_ = b.v0_;
    dv2_ = b.dv2_; dv1_ = b.dv1_;
    return *this;
}


inline VEC3 BEZIER::eval(float t) const
{
    return t*t*t*v3_ + t*t*v2_ + t*v1_ + v0_;
}


inline VEC3 BEZIER::evalDer(float t) const
{
    return t*t*dv2_ + t*dv1_ + v1_;
}


inline void ClosestDegrees( float &a1, float &a2 )
{
    float diff;

    a1 = fmod(a1, 360.0F);
    a2 = fmod(a2, 360.0F);

    if( a1 < 0.0 ) a1 += 360.0;
    if( a2 < 0.0 ) a2 += 360.0;

    diff = a1 - a2;
    if( diff < -180.0 ) a1+=360.0;
    else if( diff > 180.0 ) a1-=360.0;
}

//------------------------------------------------------------------------
// Smooth Tracking Camera
//------------------------------------------------------------------------


class CUBILERP { // (JCL) yeah i know its a dumb name suck it down
protected:
    VEC3 s, v, a, j, final;
    VEC3 s0, v0, a0;
    float t_elapsed, t_eta;

public:
    CUBILERP();

    CUBILERP( const VEC3&, // initial position
              const VEC3&, // final position
              float );     // time to take for path

    CUBILERP( const VEC3&, // initial position
              const VEC3&, // final position
              const VEC3&, // initial velocity
              const VEC3&, // initial acceleration
              float );     // time to take for path

    void update( const VEC3&,  // new destination
                 float );    // new time to take for path

    void iterate( float );   // time elapsed since last call

    VEC3 getPos(); // returns position
    VEC3 getVel(); // returns velocity

    VEC3 getVal( float ); // t from 0 to 1
};

/*
class QUADSLOW {
public:
    QUADSLOW();

    QUADSLOW( const VEC3&,
              const VEC3&
};
*/


class CUBIMOVE {
protected:
    VEC3 s, v, j;
    VEC3 s_i, s_f, v_i, v_f, a_i;
    float t, t_f;
    float fWrap;
    int bWrapSet;

    void recalc();

public:
    CUBIMOVE();

    CUBIMOVE( const VEC3&,      // initial position
              const VEC3&,      //   final position
              const VEC3&,      // initial velocity
              const VEC3&,      //   final velocity
              float,            // time allowed for path
              float );          // pass nonzero to wrap (360 for deg, 2pi for rad)

    void update( const VEC3&,   // new final position
                 float );       // new time left for path

    void update( const VEC3&,   // new final position
                 const VEC3&,   // new final velocity
                 float );       // new time left for path

    float iterate( float );     // pass frametime
    float iterateT(float);      // pass t in range [0.0, 1.0]

    VEC3 getPos();
    VEC3 getVel();
};


class FLOATMOVE {

private:
    float   fDensFluid, fDensObject, fViscosity, fRatio;
    VEC3    vGrav, vVel, vAccel;

public:
    FLOATMOVE( float,       // object mass
               float,       // object volume
               float,       // fluid density (1.0 = water)
               float,       // fluid viscocity (ignored for now)
               const VEC3&, // gravity vector < 0, 0, -g >
               const VEC3&);// object initial velocity

    void iterate( float,    // time since last call
                  float );  // percent of object volume submerged

    VEC3 getVel();
    void setVel( const VEC3& ); // for if it has to change direction from 
                                // explosion, etc.
};


#endif