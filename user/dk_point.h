
#ifndef __CVECTOR_H
#define __CVECTOR_H

//#pragma once

#if _MSC_VER
#include <crtdbg.h>
#endif
#include <math.h>
#include "dk_defines.h"
#include "dk_matrix.h"
#include "memmgrcommon.h"

//unix - _ASSERTE
#if !_MSC_VER
#if !_ASSERTE
#define _ASSERTE(exp)
#endif
#endif

// Logic[5/24/99]: portability, \'s are bad in some environments (gcc)
#define VectorDistance2( v1, v2 ) ( ( ( v2 ).x - ( v1 ).x ) * ( ( v2 ).x - ( v1 ).x ) + ( ( v2 ).y - ( v1 ).y ) * ( ( v2 ).y - ( v1 ).y ) + ( ( v2 ).z - ( v1 ).z ) * ( ( v2 ).z - ( v1 ).z ))
#define VectorDistance(v1, v2) (sqrt( (((v2).x - (v1).x) * ((v2).x - (v1).x)) + (((v2).y - (v1).y) * ((v2).y - (v1).y)) + (((v2).z - (v1).z) * ((v2).z - (v1).z)) ) )
#define VectorXYDistance(v1, v2) (sqrt( (((v2).x - (v1).x) * ((v2).x - (v1).x)) + (((v2).y - (v1).y) * ((v2).y - (v1).y)) )	)
#define VectorZDistance(v1, v2)	( fabs((v1).z - (v2).z)	)
#define VectorYDistance(v1, v2)	( fabs((v1).y - (v2).y) )
#define VectorEqual(a,b)	((a.x == b.x) && (a.y == b.y) && (a.z == b.z))


class CMatrix;

class CVector 
{
public:
//	void* CVector::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CVector::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CVector::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CVector::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
/* PHM
	CVector::CVector( const CVector &v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}
*/
	CVector::CVector(float x, float y, float z) 
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	CVector::CVector(const float *vect) 
	{
		x = vect[0];
		y = vect[1];
		z = vect[2];
	}

	float X() const			{ return x; }
	float Y() const			{ return y; }
	float Z() const			{ return z; }

	void Set( const CVector &v )	{ x = v.X(); y = v.Y(); z = v.Z(); }
	void Set( float fX, float fY, float fZ ) { x = fX; y = fY; z = fZ; }

	void SetX( float fNewX )	{ x = fNewX; }
	void SetY( float fNewY )	{ y = fNewY; }
	void SetZ( float fNewZ )	{ z = fNewZ; }
	void AddX( float fValue )	{ x += fValue; }
	void AddY( float fValue )	{ y += fValue; }
	void AddZ( float fValue )	{ z += fValue; }
	void MultX( float fValue )	{ x *= fValue; }
	void MultY( float fValue )	{ y *= fValue; }
	void MultZ( float fValue )	{ z *= fValue; }

	float Pitch() const					{ return x; }
	float Yaw() const					{ return y; }
	float Roll() const					{ return z; }
	void SetPitch( float fNewPitch )	{ x = fNewPitch; }
	void SetYaw( float fNewYaw )		{ y = fNewYaw; }
	void SetRoll( float fNewRoll )		{ z = fNewRoll; }

	float Distance( CVector &v )
	{
		return (float)sqrt( sqr(x-v.X()) + sqr(y-v.Y()) + sqr(z-v.Z()) );
	}

    float Distance2( CVector &v )
    {
        return (float) (sqr(x-v.X()) + sqr(y-v.Y()) + sqr(z-v.Z()));
    }

#ifdef _DEBUG

    CVector();

    float Normalize();
    float Length() const;
    void Zero();
	void Negate();
    void Negate( const CVector &p );

	void Equal( const CVector &v );
	void Equal(float x, float y, float z);
    void Equal(const CVector &p, float len);
    void Equal(const CVector &p1, float scale1, const CVector &p2, float scale2);

    void Subtract(const CVector &p0, const CVector &p1);

    void Add( const float fValue );
	void Add( const CVector &v );
	void Add( const CVector &p0, const CVector &p1 );
    void Add( const CVector &p0, float scale );

    void Multiply(float scale);
    void Multiply(const CMatrix &mat, const CVector &p);

    float DotProduct(const CVector &p) const;
	void CrossProduct( const CVector &v1, const CVector &v2 );

    void CopyTo(float *vect) const;

    // pitch, yaw, and roll, are taken as radians.
    // use this to convert world angles to world direction vectors.
    void AngleToVectors( CVector &forward, CVector &right, CVector &up ) const;
	void VectorToAngles( CVector &angles ) const;
	void AngleToForwardVector( CVector &forward ) const;

	void SetValue( float fValue, int nIndex );
	float GetValue( int nIndex );

    void operator=(const CVector &other);
    void operator=(const float *vect);

	// overloading
	CVector	operator+(const CVector &v) const;
	CVector	operator-(const CVector &v) const;
	CVector	operator-() const;
	CVector	operator*(float n) const;
	int		operator!=(const CVector &v) const;
	int		operator==(const CVector &v) const;
	int		operator==(int v) const;
	void	operator+=(const CVector &v);
	void	operator-=(const CVector &v);

    operator const float *() const;
	float &operator[](int nIndex);

	void Scale(CVector & Source, float scale);
	CVector Interpolate( CVector& v1, CVector& v2, float fPercent );

#else _DEBUG

    CVector::CVector() 
	{
        x = y = z = 0.0f;
    }

    __inline float Normalize();
    __inline float Length() const;
    __inline void Zero();
	__inline void Negate();
    __inline void Negate( const CVector &p );

	__inline void Equal( const CVector &v );
	__inline void Equal(float x, float y, float z);
    __inline void Equal(const CVector &p, float len);
    __inline void Equal(const CVector &p1, float scale1, const CVector &p2, float scale2);

    __inline void Subtract(const CVector &p0, const CVector &p1);

    __inline void Add( const float fValue );
	__inline void Add( const CVector &v );
	__inline void Add( const CVector &p0, const CVector &p1 );
    __inline void Add( const CVector &p0, float scale );

    __inline void Multiply(float scale);
    __inline void Multiply(const CMatrix &mat, const CVector &p);

    __inline float DotProduct(const CVector &p) const;
	__inline void CrossProduct( const CVector &v1, const CVector &v2 );

    __inline void CopyTo(float *vect) const;

    // pitch, yaw, and roll, are taken as radians.
    // use this to convert world angles to world direction vectors.
    __inline void AngleToVectors( CVector &forward, CVector &right, CVector &up ) const;
	__inline void VectorToAngles( CVector &angles ) const;
	__inline void AngleToForwardVector( CVector &forward ) const;

	__inline void SetValue( float fValue, int nIndex );
	__inline float GetValue( int nIndex );

// PHM
//    __inline void operator=(const CVector &other);
    __inline void operator=(const float *vect);

	// overloading
	__inline CVector	operator+(const CVector &v) const;
	__inline CVector	operator-(const CVector &v) const;
	__inline CVector	operator-() const;
	__inline CVector	operator*(float n) const;
	__inline int		operator!=(const CVector &v) const;
	__inline int		operator==(const CVector &v) const;
	__inline int		operator==(int v) const;
	__inline void		operator+=(const CVector &v);
	__inline void		operator-=(const CVector &v);

    __inline operator const float *() const;
	__inline float &operator[](int nIndex);

	__inline void Scale(CVector & Source, float scale);

	__inline CVector Interpolate( CVector& v1, CVector& v2, float fPercent );

#endif _DEBUG

	union
	{
		float x;
		float pitch;
	};
	union
	{
		float y;
		float yaw;
	};
	union
	{
		float z;
		float roll;
	};
};

#ifdef _DEBUG

CVector CVector::operator+(const CVector &v) const;
CVector CVector::operator-(const CVector &v) const;
CVector CVector::operator-() const;
CVector CVector::operator*(float n) const;
int CVector::operator!=(const CVector &v) const;
int CVector::operator==(const CVector &v) const;
int CVector::operator==(int i) const;
void CVector::operator+=(const CVector &v);
void CVector::operator-=(const CVector &v);
float &CVector::operator[](int nIndex);
void CVector::operator=(const CVector &other);
void CVector::operator=(const float *vect);
CVector::operator const float *() const;
void CVector::Equal( const CVector &v );
void CVector::Equal(float x, float y, float z);
void CVector::Equal(const CVector &p, float len);
void CVector::Equal(const CVector &p1, float scale1, const CVector &p2, float scale2);
void CVector::CopyTo(float *vect) const;
void CVector::Subtract(const CVector &p0, const CVector &p1);
float CVector::Normalize();
float CVector::Length() const;
void CVector::Negate();
void CVector::Negate( const CVector &p );
float CVector::DotProduct(const CVector &p) const;
void CVector::CrossProduct( const CVector &v1, const CVector &v2 );
void CVector::Add( const float fValue );
void CVector::Add( const CVector &v );
void CVector::Add(const CVector &p0, const CVector &p1);
void CVector::Add(const CVector &p0, float scale);
void CVector::Zero();
void CVector::Multiply( float scale );
void CVector::AngleToVectors( CVector &forward, CVector &right, CVector &up ) const;
void CVector::VectorToAngles( CVector &angles ) const;
void CVector::AngleToForwardVector( CVector &forward ) const;
void CVector::Multiply( const CMatrix &mat, const CVector &p );
void CVector::SetValue( float fValue, int nIndex );
float CVector::GetValue( int nIndex );
CVector operator *( float n, const CVector& b );
void CVector::Scale(CVector & Source, float scale);
CVector CVector::Interpolate( CVector& v1, CVector& v2, float fPercent );

/* ************************************************************************* */
/*						Helper functions and definitions					 */
/* ************************************************************************* */

float DotProduct( const CVector &v1, const CVector &v2 );
float DotProduct2D( const CVector &v1, const CVector &v2 );
void CrossProduct( const CVector &v1, const CVector &v2, CVector &cross );
void AngleToVectors( const CVector &v, CVector &forward, CVector &right, CVector &up );
void VectorToAngles( const CVector &v, CVector &angles );
void AngleToVectors( const CVector &v, CVector &forward );
void YawToVector( float fYaw, CVector &vector );
float VectorToYaw( const CVector &v );
void VectorMA( CVector &v1, CVector &v2, float scale, CVector &out );
float AngleMod (float ang);
float AngleDiff( float ang1, float ang2 );
void Normal( CVector &v1, CVector &v2, CVector &v3, CVector &out );
float LerpAngle( float a2, float a1, float frac );
void LerpAngles( CVector &aPrev, CVector &aCur, CVector &aOut, float frac);

#else _DEBUG


///////////////////////////////////////////////////////////////////////////////
//	+ operator
///////////////////////////////////////////////////////////////////////////////
__inline CVector CVector::operator+(const CVector &v) const
{
	return(CVector(x+v.x, y+v.y, z+v.z));
}

///////////////////////////////////////////////////////////////////////////////
//	- operator
///////////////////////////////////////////////////////////////////////////////
__inline CVector CVector::operator-(const CVector &v) const
{
	return(CVector(x-v.x, y-v.y, z-v.z));
}

///////////////////////////////////////////////////////////////////////////////
//	negation operator
///////////////////////////////////////////////////////////////////////////////
__inline CVector CVector::operator-() const
{
	return(CVector(-x, -y, -z));
}

///////////////////////////////////////////////////////////////////////////////
//	* operator
///////////////////////////////////////////////////////////////////////////////
__inline CVector CVector::operator*(float n) const
{
	return(CVector(n*x, n*y, n*z));
}

///////////////////////////////////////////////////////////////////////////////
//	=!
//
//	not freaking equal operator
///////////////////////////////////////////////////////////////////////////////
__inline int CVector::operator!=(const CVector &v) const
{
	if (x != v.x || y != v.y || z != v.z)
	{
		return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
//	==
//
//	compare operator
///////////////////////////////////////////////////////////////////////////////
__inline int CVector::operator==(const CVector &v) const
{
	if (x == v.x && y == v.y && z == v.z)
	{
		return TRUE;
	}

	return FALSE;
}

__inline int CVector::operator==(int i) const
{
	int CALL_INSOO = FALSE;
	_ASSERTE( CALL_INSOO );

	return FALSE;
}

__inline void CVector::operator+=(const CVector &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

__inline void CVector::operator-=(const CVector &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

__inline float &CVector::operator[](int nIndex)
{ 
#if 1

	return (&x)[nIndex];

#else
/*
	float fValue = 0.0f;
	char *thisString = (char*)this;
	int nFloatSize = sizeof(fValue);
	memcpy( &fValue, &thisString[nIndex*nFloatSize], nFloatSize );

	return fValue;
*/
	switch ( nIndex )
	{
		case 0:	return x;
		case 1:	return y;
		case 2:	return z;
			break;

		default:
			// error
			_ASSERTE( FALSE );
			break;
	}

	return x;
#endif
}

/*
__inline void CVector::operator=(const CVector &other) 
{
    x = other.x;
    y = other.y;
    z = other.z;
}
*/
__inline void CVector::operator=(const float *vect) 
{
	if ( vect )
	{
		x = vect[0];
		y = vect[1];
		z = vect[2];
	}
	else
	{
		x = y = z = 0;
	}
}

__inline CVector::operator const float *() const 
{
    return (float *)this;
}


__inline void CVector::Equal( const CVector &v )
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

__inline void CVector::Equal(float x, float y, float z) 
{
    this->x = x;
    this->y = y;
    this->z = z;
}

__inline void CVector::Equal(const CVector &p, float len) 
{
    x = p.x * len;
    y = p.y * len;
    z = p.z * len;
}

__inline void CVector::Equal(const CVector &p1, float scale1, const CVector &p2, float scale2) 
{
    x = p1.x * scale1 + p2.x * scale2;
    y = p1.y * scale1 + p2.y * scale2;
    z = p1.z * scale1 + p2.z * scale2;
}

__inline void CVector::CopyTo(float *vect) const 
{
    vect[0] = x;
    vect[1] = y;
    vect[2] = z;
}

__inline void CVector::Subtract(const CVector &p0, const CVector &p1) 
{
    x = p0.x - p1.x;
    y = p0.y - p1.y;
    z = p0.z - p1.z;
}

__inline float CVector::Normalize() 
{
    float len = float(sqrt(x * x + y * y + z * z));
    if (len > 0.0001) 
	{
        float invLen = 1.0f/len;
        x *= invLen;
        y *= invLen;
        z *= invLen;
    }

    return len;
}

__inline float CVector::Length() const 
{
    return float(sqrt(x * x + y * y + z * z));
}

__inline void CVector::Negate()
{
	x = -x;
	y = -y;
	z = -z;
}

__inline void CVector::Negate( const CVector &p )
{
	x = -p.x;
	y = -p.y;
	z = -p.z;
}

__inline float CVector::DotProduct(const CVector &p) const 
{
    return p.x * x + p.y * y + p.z * z;
}

__inline void CVector::CrossProduct( const CVector &v1, const CVector &v2 )
{
	x = v1.y*v2.z - v1.z*v2.y;
	y = v1.z*v2.x - v1.x*v2.z;
	z = v1.x*v2.y - v1.y*v2.x;
}

__inline void CVector::Add( const float fValue )
{
	x += fValue;
	y += fValue;
	z += fValue;
}

__inline void CVector::Add( const CVector &v )
{
	x += v.x;
	y += v.y;
	z += v.z;
}

__inline void CVector::Add(const CVector &p0, const CVector &p1) 
{
    x = p0.x + p1.x;
    y = p0.y + p1.y;
    z = p0.z + p1.z;
}

__inline void CVector::Add(const CVector &p0, float scale) 
{
    x += p0.x * scale;
    y += p0.y * scale;
    z += p0.z * scale;
}

__inline void CVector::Zero() 
{
    x = y = z = 0.0f;
}

__inline void CVector::Multiply( float scale )
{
    x *= scale;
    y *= scale;
    z *= scale;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
//this->x, y, and z are interpreted as pitch, yaw, and roll, in radians.
__inline void CVector::AngleToVectors( CVector &forward, CVector &right, CVector &up ) const 
{
    float sp, cp, sy, cy, sr, cr, fRad;
    
    // pitch is often 0
    if (pitch) 
    {
        fRad = DEG2RAD( pitch );
        sp = (float)sin( fRad );
        cp = (float)cos( fRad );
    }
    else
    {
        sp = 0;
        cp = 1;
    }

    fRad = DEG2RAD( yaw );
	sy = (float)sin( fRad );
	cy = (float)cos( fRad );
    
    // roll is almost always 0
    if (roll) 
    {
    	fRad = DEG2RAD( roll );
    	sr = (float)sin( fRad );
        cr = (float)cos( fRad );
    }
    else
    {
        sr = 0;
        cr = 1;
    }

    //fill in the return values.  yaw, pitch, and roll of 0 give a forward looking down the positive
    //x axis and an up facing up the positive z axis.  If you want to know how the below formula were
    //derived, figure it out yourself....  hehehe, it's not very hard, if you're smart.
	forward.Equal( cp * cy, cp * sy, -sp );
    right.Equal( -sr * sp * cy + cr * sy, -sr * sp * sy - cr * cy, -sr * cp );
    up.Equal( cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp );
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline void CVector::VectorToAngles( CVector &angles ) const
{
	int yaw, pitch;
	
	if ( y == 0.0f && x == 0.0f )
	{
		yaw = 0;
		if ( z > 0.0f )
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		yaw = (int)( atan2( y, x ) * ONEEIGHTY_OVER_PI );
		if ( yaw < 0 )
		{
			yaw += 360;
		}

		float forward = (float)sqrt( x * x + y * y );
		pitch = (int)( atan2( z, forward ) * ONEEIGHTY_OVER_PI );
		
		if ( pitch < 0 )
		{
			pitch += 360;
		}
	}

	angles.SetPitch( (float)-pitch );
	angles.SetYaw( (float)yaw );
	angles.SetRoll( 0 );
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline void CVector::AngleToForwardVector( CVector &forward ) const
{
    float fRad = DEG2RAD( x );
    float sp = (float)sin( fRad );
    float cp = (float)cos( fRad );

    fRad = DEG2RAD( y );
	float sy = (float)sin( fRad );
	float cy = (float)cos( fRad );
    
	forward.Equal( (cp*cy), (cp*sy), (-sp) );
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline void CVector::Multiply( const CMatrix &mat, const CVector &p ) 
{
    x = mat[0][0] * p.x + mat[0][1] * p.y + mat[0][2] * p.z + mat[0][3];
    y = mat[1][0] * p.x + mat[1][1] * p.y + mat[1][2] * p.z + mat[1][3];
    z = mat[2][0] * p.x + mat[2][1] * p.y + mat[2][2] * p.z + mat[2][3];
}


__inline void CVector::SetValue( float fValue, int nIndex )
{
	switch ( nIndex )
	{
		case 0:	x = fValue;		break;
		case 1:	y = fValue;		break;
		case 2:	z = fValue;		break;

		default:
			// error
			_ASSERTE( FALSE );
			break;
	}
}

__inline float CVector::GetValue( int nIndex )
{
	float fValue = 0.0;;
	switch ( nIndex )
	{
		case 0:	fValue = x;		break;
		case 1:	fValue = y;		break;
		case 2:	fValue = z;		break;
			break;

		default:
			// error
			_ASSERTE( FALSE );
			break;
	}

	return fValue;
}

__inline CVector operator *( float n, const CVector& b )
{
	return(b * n);
}

///////////////////////////////////////////////////////////////////////
// Scale(source_vector, scale)
//
// Description:
//    Equivelant to VectorScale quake/quake2 routine. Scales the 
//    current CVector.
//
// Parms:
//    CVector & Source  The scale source
//    scale             multiplier for scaling
//
// Return:
//    none
//
// Bugs:
__inline void CVector::Scale(CVector & Source, float scale)
{

   x = Source.x * scale;
   y = Source.y * scale;
   z = Source.z * scale;
}

__inline CVector CVector::Interpolate( CVector& v1, CVector& v2, float fPercent )
{
	CVector vTemp;

	vTemp.x = v2.x + fPercent * (v1.x - v2.x);
	vTemp.y = v2.y + fPercent * (v1.y - v2.y);
	vTemp.z = v2.z + fPercent * (v1.z - v2.z);

	return vTemp;
}

/* ************************************************************************* */
/*						Helper functions and definitions					 */
/* ************************************************************************* */

__inline float DotProduct( const CVector &v1, const CVector &v2 )
{
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

__inline float DotProduct2D( const CVector &v1, const CVector &v2 )
{
    return (v1.x * v2.x + v1.y * v2.y);
}

__inline void CrossProduct( const CVector &v1, const CVector &v2, CVector &cross )
{
	cross.x = v1.y*v2.z - v1.z*v2.y;
	cross.y = v1.z*v2.x - v1.x*v2.z;
	cross.z = v1.x*v2.y - v1.y*v2.x;
}

__inline void AngleToVectors( const CVector &v, CVector &forward, CVector &right, CVector &up )
{
    float sp, cp, sy, cy, sr, cr, fRad;

    // pitch is often 0
    if (v.pitch) 
    {
        fRad = DEG2RAD( v.x );
        sp = (float)sin( fRad );
        cp = (float)cos( fRad );
    }
    else
    {
        sp = 0;
        cp = 1;
    }

    fRad = DEG2RAD( v.yaw );
	sy = (float)sin( fRad );
	cy = (float)cos( fRad );
    
    // roll is almost always 0
    if (v.roll) 
    {
    	fRad = DEG2RAD( v.roll );
	    sr = (float)sin( fRad );
        cr = (float)cos( fRad );
    }
    else
    { 
        sr = 0;
        cr = 1;
    }

    //fill in the return values.  yaw, pitch, and roll of 0 give a forward looking down the positive
    //x axis and an up facing up the positive z axis.  If you want to know how the below formula were
    //derived, figure it out yourself....  hehehe, it's not very hard, if you're smart.
	forward.Equal( (cp*cy), (cp*sy), (-sp) );
    right.Equal( (-1*sr*sp*cy+-1*cr*-sy), (-1*sr*sp*sy+-1*cr*cy), (-1*sr*cp) );
    up.Equal( (cr*sp*cy+-sr*-sy), (cr*sp*sy+-sr*cy), (cr*cp) );
}

__inline void VectorToAngles( const CVector &v, CVector &angles )
{
	int yaw, pitch;
	
	if ( v.y == 0.0f && v.x == 0.0f )
	{
		yaw = 0;
		if ( v.z > 0.0f )
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		yaw = (int)( atan2( v.y, v.x ) * ONEEIGHTY_OVER_PI );
		if ( yaw < 0 )
		{
			yaw += 360;
		}

		float forward = (float)sqrt( v.x * v.x + v.y * v.y );
		pitch = (int)( atan2( v.z, forward ) * ONEEIGHTY_OVER_PI );
		
		if ( pitch < 0 )
		{
			pitch += 360;
		}
	}

	angles.SetPitch( (float)-pitch );
	angles.SetYaw( (float)yaw );
	angles.SetRoll( 0 );
}

__inline void AngleToVectors( const CVector &v, CVector &forward )
{
    float sp, cp, sy, cy, fRad;

    // pitch is often 0
    if (v.pitch) 
    {
        fRad = DEG2RAD( v.pitch );
        sp = (float)sin( fRad );
        cp = (float)cos( fRad );
    }
    else
    {
        sp = 0;
        cp = 1;
    }

    fRad = DEG2RAD( v.yaw );
	sy = (float)sin( fRad );
	cy = (float)cos( fRad );
    
	forward.Equal( (cp*cy), (cp*sy), (-sp) );
}

__inline void YawToVector( float fYaw, CVector &vector )
{
	float angle = DEG2RAD( fYaw );
	float sy = sin(angle);
	float cy = cos(angle);

	vector.Set( cy, sy, 0 );
}

__inline float VectorToYaw( const CVector &v )
{
	float yaw;

	if ( v.y == 0 && v.x == 0 )
	{
		yaw = 0;
	}
	else
	{
		yaw = (atan2(v.y, v.x) * ONEEIGHTY_OVER_PI);
		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

__inline void VectorMA( CVector &v1, CVector &v2, float scale, CVector &out )
{
	float fX = v1.x + (v2.x*scale);
	float fY = v1.y + (v2.y*scale);
	float fZ = v1.z + (v2.z*scale);
	out.Set( fX, fY, fZ );
}

////////////////////////////////////////
//	AngleMod
//
//	returns an equivalent angle 0-360
////////////////////////////////////////
__inline float AngleMod (float ang)
{
	return (360.0 / 65536) * ((int)(ang * (65536 / 360.0)) & 65535);
}

///////////////////////////////////////////////
//	AngleDiff
//
//	returns the difference between passed angles
///////////////////////////////////////////////
__inline float AngleDiff( float ang1, float ang2 )
{
	float	diff;

	ang1 = AngleMod(ang1);
	ang2 = AngleMod(ang2);

	diff = ang1 - ang2;

	if (diff < -180)
	{
		diff += 360;
		diff *= -1;
	}
	else 
	if (diff < 0)
	{
		diff = fabs (diff);
	}
	else 
	if (diff > 180)
	{
		diff = fabs (diff - 360);
	}
	else 
	{
		diff *= -1;
	}

	return	diff;
}

///////////////////////////////////////////////
//	Normal
//
//	returns the normal of the plane described by
//	points v1, v2, and v3 in vector out
///////////////////////////////////////////////
__inline void Normal( CVector &v1, CVector &v2, CVector &v3, CVector &out )
{
	CVector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	CrossProduct( a, b, out );
}

__inline float LerpAngle( float a2, float a1, float frac )
{
	if (a1 - a2 > 180)
	{
		a1 -= 360;
	}
	if (a1 - a2 < -180)
	{
		a1 += 360;
	}
	return a2 + frac * (a1 - a2);
}

__inline void LerpAngles( CVector &aPrev, CVector &aCur, CVector &aOut, float frac)
{
	aOut.x = LerpAngle(aPrev.x,aCur.x,frac);
	aOut.y = LerpAngle(aPrev.y,aCur.y,frac);
	aOut.z = LerpAngle(aPrev.z,aCur.z,frac);	
}

#endif _DEBUG

float ComputeAngle2D( const CVector &vector1, const CVector &vector2 );
float ComputeAngle( const CVector &vector1, const CVector &vector2 );

void RotateVector2D( CVector &vector, float fDegrees );

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

void RotatePointAroundVector( const CVector &dir, const CVector &point, float degrees, CVector &out );
void ProjectPointOnPlane( const CVector &p, const CVector &normal, CVector &out );
void PerpendicularVector( const CVector &src, CVector &out );

void BestDelta (float start_angle, float end_angle, float *best_delta, float *angle_diff);

#endif __CVECTOR_H
