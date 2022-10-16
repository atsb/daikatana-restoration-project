// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include <memory.h>
#include "dk_point.h"
     
#define TRAP_VECTOR_ERRORS 0
#if TRAP_VECTOR_ERRORS
	#include <float.h>
	#define TRAP_VECTOR_ERROR(v)    {_ASSERTE(!_isnan((v).x));_ASSERTE(!_isnan((v).y));_ASSERTE(!_isnan((v).z));}
#else
	#define TRAP_VECTOR_ERROR(v) 
#endif

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */



#ifdef _DEBUG

CVector::CVector()
{
#if !TRAP_VECTOR_ERRORS
    x = y = z = 0.0f;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//	+ operator
///////////////////////////////////////////////////////////////////////////////
CVector CVector::operator+(const CVector &v) const
{
	CVector sum;

    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	sum.x = x + v.x;
	sum.y = y + v.y;
	sum.z = z + v.z;

	return(sum);
}

///////////////////////////////////////////////////////////////////////////////
//	- operator
///////////////////////////////////////////////////////////////////////////////
CVector CVector::operator-(const CVector &v) const
{
	CVector diff;

    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	diff.x = x - v.x;
	diff.y = y - v.y;
	diff.z = z - v.z;

	return(diff);
}

///////////////////////////////////////////////////////////////////////////////
//	negation operator
///////////////////////////////////////////////////////////////////////////////
CVector CVector::operator-() const
{
	CVector neg;

    TRAP_VECTOR_ERROR(*this)

	neg.x = -x;
	neg.y = -y;
	neg.z = -z;

	return(neg);
}

///////////////////////////////////////////////////////////////////////////////
//	* operator
///////////////////////////////////////////////////////////////////////////////
CVector CVector::operator*(float n) const
{
	CVector mult;
    
    TRAP_VECTOR_ERROR(*this)
	
    mult.x = n * x;
	mult.y = n * y;
	mult.z = n * z;

	return(mult);
}

///////////////////////////////////////////////////////////////////////////////
//	=!
//
//	not freaking equal operator
///////////////////////////////////////////////////////////////////////////////
int CVector::operator!=(const CVector &v) const
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

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
int CVector::operator==(const CVector &v) const
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	if (x == v.x && y == v.y && z == v.z)
	{
		return TRUE;
	}

	return FALSE;
}

int CVector::operator==(int i) const
{
	int CALL_INSOO = FALSE;
	_ASSERTE( CALL_INSOO );

	return FALSE;
}

void CVector::operator+=(const CVector &v)
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	x += v.x;
	y += v.y;
	z += v.z;
}

void CVector::operator-=(const CVector &v)
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	x -= v.x;
	y -= v.y;
	z -= v.z;
}

float &CVector::operator[](int nIndex)
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

void CVector::operator=(const CVector &other) 
{
    x = other.x;
    y = other.y;
    z = other.z;
}

void CVector::operator=(const float *vect) 
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

CVector::operator const float *() const 
{
    TRAP_VECTOR_ERROR(*this)

    return (float *)this;
}


void CVector::Equal( const CVector &v )
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

void CVector::Equal(float x, float y, float z) 
{
    this->x = x;
    this->y = y;
    this->z = z;
}

void CVector::Equal(const CVector &p, float len) 
{
    x = p.x * len;
    y = p.y * len;
    z = p.z * len;
}

void CVector::Equal(const CVector &p1, float scale1, const CVector &p2, float scale2) 
{
    x = p1.x * scale1 + p2.x * scale2;
    y = p1.y * scale1 + p2.y * scale2;
    z = p1.z * scale1 + p2.z * scale2;
}

void CVector::CopyTo(float *vect) const 
{
    vect[0] = x;
    vect[1] = y;
    vect[2] = z;
}

void CVector::Subtract(const CVector &p0, const CVector &p1) 
{
    TRAP_VECTOR_ERROR(p0)
    TRAP_VECTOR_ERROR(p1)

    x = p0.x - p1.x;
    y = p0.y - p1.y;
    z = p0.z - p1.z;
}

float CVector::Normalize() 
{
    TRAP_VECTOR_ERROR(*this)

    float len = float(sqrt(x * x + y * y + z * z));
    if (len > 0.0001) 
	{
        x /= len;
        y /= len;
        z /= len;
    }

    return len;
}

float CVector::Length() const 
{
    TRAP_VECTOR_ERROR(*this)

    return float(sqrt(x * x + y * y + z * z));
}

void CVector::Negate()
{
    TRAP_VECTOR_ERROR(*this)

	x = -x;
	y = -y;
	z = -z;
}

void CVector::Negate( const CVector &p )
{
    TRAP_VECTOR_ERROR(p)

	x = -p.x;
	y = -p.y;
	z = -p.z;
}

float CVector::DotProduct(const CVector &p) const 
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(p)

    return p.x * x + p.y * y + p.z * z;
}

void CVector::CrossProduct( const CVector &v1, const CVector &v2 )
{
    TRAP_VECTOR_ERROR(v1)
    TRAP_VECTOR_ERROR(v2)

	x = v1.y*v2.z - v1.z*v2.y;
	y = v1.z*v2.x - v1.x*v2.z;
	z = v1.x*v2.y - v1.y*v2.x;
}

void CVector::Add( const float fValue )
{
    TRAP_VECTOR_ERROR(*this)
	
    x += fValue;
	y += fValue;
	z += fValue;
}

void CVector::Add( const CVector &v )
{
    TRAP_VECTOR_ERROR(*this)
    TRAP_VECTOR_ERROR(v)

	x += v.x;
	y += v.y;
	z += v.z;
}

void CVector::Add(const CVector &p0, const CVector &p1) 
{
    TRAP_VECTOR_ERROR(p0)
    TRAP_VECTOR_ERROR(p1)

    x = p0.x + p1.x;
    y = p0.y + p1.y;
    z = p0.z + p1.z;
}

void CVector::Add(const CVector &p0, float scale) 
{
    TRAP_VECTOR_ERROR(p0)

    x += p0.x * scale;
    y += p0.y * scale;
    z += p0.z * scale;
}

void CVector::Zero() 
{
    x = y = z = 0.0f;
}

void CVector::Multiply( float scale )
{
    TRAP_VECTOR_ERROR(*this)

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
void CVector::AngleToVectors( CVector &forward, CVector &right, CVector &up ) const 
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

    fRad = DEG2RAD( y );
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
void CVector::VectorToAngles( CVector &angles ) const
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

	angles.SetPitch( AngleMod(-pitch) );
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
void CVector::AngleToForwardVector( CVector &forward ) const
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
void CVector::Multiply( const CMatrix &mat, const CVector &p ) 
{
    x = mat[0][0] * p.x + mat[0][1] * p.y + mat[0][2] * p.z + mat[0][3];
    y = mat[1][0] * p.x + mat[1][1] * p.y + mat[1][2] * p.z + mat[1][3];
    z = mat[2][0] * p.x + mat[2][1] * p.y + mat[2][2] * p.z + mat[2][3];
}


void CVector::SetValue( float fValue, int nIndex )
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

float CVector::GetValue( int nIndex )
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

CVector operator *( float n, const CVector& b )
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
void CVector::Scale(CVector & Source, float scale)
{

   x = Source.x * scale;
   y = Source.y * scale;
   z = Source.z * scale;
}

CVector CVector::Interpolate( CVector& v1, CVector& v2, float fPercent )
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

float DotProduct( const CVector &v1, const CVector &v2 )
{
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

float DotProduct2D( const CVector &v1, const CVector &v2 )
{
    return (v1.x * v2.x + v1.y * v2.y);
}

void CrossProduct( const CVector &v1, const CVector &v2, CVector &cross )
{
	cross.x = v1.y*v2.z - v1.z*v2.y;
	cross.y = v1.z*v2.x - v1.x*v2.z;
	cross.z = v1.x*v2.y - v1.y*v2.x;
}

void AngleToVectors( const CVector &v, CVector &forward, CVector &right, CVector &up )
{
    float sp, cp, sy, cy, sr, cr, fRad;

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

    fRad = DEG2RAD( v.y );
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

void VectorToAngles( const CVector &v, CVector &angles )
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

	angles.SetPitch( AngleMod(-pitch) );
	angles.SetYaw( (float)yaw );
	angles.SetRoll( 0 );
}

void AngleToVectors( const CVector &v, CVector &forward )
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

    fRad = DEG2RAD( v.y );
	sy = (float)sin( fRad );
	cy = (float)cos( fRad );
    
	forward.Equal( (cp*cy), (cp*sy), (-sp) );
}

void YawToVector( float fYaw, CVector &vector )
{
	float angle = DEG2RAD( fYaw );
	float sy = sin(angle);
	float cy = cos(angle);

	vector.Set( cy, sy, 0 );
}

float VectorToYaw( const CVector &v )
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

void VectorMA( CVector &v1, CVector &v2, float scale, CVector &out )
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
float AngleMod (float ang)
{
	return (360.0 / 65536) * ((int)(ang * (65536 / 360.0)) & 65535);
}

///////////////////////////////////////////////
//	AngleDiff
//
//	returns the difference between passed angles
///////////////////////////////////////////////
float AngleDiff( float ang1, float ang2 )
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
void Normal( CVector &v1, CVector &v2, CVector &v3, CVector &out )
{
	CVector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	CrossProduct( a, b, out );
}

float LerpAngle( float a2, float a1, float frac )
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

void LerpAngles( CVector &aPrev, CVector &aCur, CVector &aOut, float frac)
{
	aOut.x = LerpAngle(aPrev.x,aCur.x,frac);
	aOut.y = LerpAngle(aPrev.y,aCur.y,frac);
	aOut.z = LerpAngle(aPrev.z,aCur.z,frac);	
}

#endif _DEBUG



/* ************************************************************************* */
/*						Helper functions and definitions					 */
/* ************************************************************************* */

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float ComputeAngle2D( const CVector &vector1, const CVector &vector2 )
{
	float cosAngle;
	float tU, tV;

	tU = (float)sqrt( sqr(vector1.x) + sqr(vector1.y) );
	tV = (float)sqrt( sqr(vector2.x) + sqr(vector2.y) );

	cosAngle = DotProduct2D( vector1, vector2 ) / (tU * tV);

	float angleDeg;
	if ( cosAngle <= -1.0f )
	{
		angleDeg = 180.0f;
	}
	else
	if ( cosAngle >= 1.0f )
	{
		angleDeg = 0.0f;
	}
	else
	{
		angleDeg = (((float)acos( cosAngle ) * 180.0f) / M_PI);
	}

	return angleDeg;
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
float ComputeAngle( const CVector &vector1, const CVector &vector2 )
{
	float tU = (float)sqrt( sqr(vector1.x) + sqr(vector1.y) + sqr(vector1.z) );
	float tV = (float)sqrt( sqr(vector2.x) + sqr(vector2.y) + sqr(vector2.z) );

	float cosAngle = DotProduct( vector1, vector2 ) / (tU * tV);

	float angleDeg;
	if ( cosAngle <= -1.0f )
	{
		angleDeg = 180.0f;
	}
	else
	if ( cosAngle >= 1.0f )
	{
		angleDeg = 0.0f;
	}
	else
	{
		angleDeg = (((float)acos( cosAngle ) * 180.0f) / M_PI);
	}

	return angleDeg;
}

// ----------------------------------------------------------------------------
//
// Name:		R_ConcatRotations
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void R_ConcatRotations( float in1[3][3], float in2[3][3], float out[3][3] )
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}


// ----------------------------------------------------------------------------
//
// Name:		R_ConcatTransforms
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void R_ConcatTransforms( float in1[3][4], float in2[3][4], float out[3][4] )
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
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
void ProjectPointOnPlane( const CVector &p, const CVector &normal, CVector &out )
{
	TRAP_VECTOR_ERROR(p)
	TRAP_VECTOR_ERROR(normal)

	CVector n;

	float inv_denom = 1.0F / DotProduct( normal, normal );

	float d = DotProduct( normal, p ) * inv_denom;

	n.Equal( normal, inv_denom );

	out.x = p.x - d * n.x;
	out.y = p.y - d * n.y;
	out.z = p.z - d * n.z;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
//				** assumes "src" is normalized
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PerpendicularVector( const CVector &src, CVector &out )
{
	//** find the smallest magnitude axially aligned vector
	int nPos = 0;
	float minelem = 1.0F;
	if ( fabs( src.x ) < minelem )
	{
		nPos = 0;
		minelem = (float)fabs( src.x );
	}
	if ( fabs( src.y ) < minelem )
	{
		nPos = 1;
		minelem = (float)fabs( src.y );
	}
	if ( fabs( src.z ) < minelem )
	{
		nPos = 2;
		minelem = (float)fabs( src.z );
	}

	CVector tempVec(0,0,0);
	tempVec[nPos] = 1.0f;

	//** project the point onto the plane defined by src
	ProjectPointOnPlane( tempVec, src, out );

	//** normalize the result
	out.Normalize();
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
#ifdef _WIN32
#pragma optimize( "", off )
#endif

void RotatePointAroundVector( const CVector &dir, const CVector &point, float degrees, CVector &out )
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	
	CVector vr, vup, vf;

	vf.Equal( dir );
	PerpendicularVector( dir, vr );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr.x;
	m[1][0] = vr.y;
	m[2][0] = vr.z;

	m[0][1] = vup.x;
	m[1][1] = vup.y;
	m[2][1] = vup.z;

	m[0][2] = vf.x;
	m[1][2] = vf.y;
	m[2][2] = vf.z;

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = (float)cos( DEG2RAD( degrees ) );
	zrot[0][1] = (float)sin( DEG2RAD( degrees ) );
	zrot[1][0] = (float)-sin( DEG2RAD( degrees ) );
	zrot[1][1] = (float)cos( DEG2RAD( degrees ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	out.x = rot[0][0] * point.x + rot[0][1] * point.y + rot[0][2] * point.z;
	out.y = rot[1][0] * point.x + rot[1][1] * point.y + rot[1][2] * point.z;
	out.z = rot[2][0] * point.x + rot[2][1] * point.y + rot[2][2] * point.z;
}

#ifdef _WIN32
#pragma optimize( "", on )
#endif

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

// rotate a vector in counter clock wise direction
//
//          90
//          |
//          |
//          |
//          |
//          |
// 180 -----------> 0
//
void RotateVector2D( CVector &vector, float fDegrees )
{
   float radian = DEG2RAD( fDegrees );
   CVector tempVector;

   // x component
   tempVector.x = (float)(vector.x * cos( radian ) + vector.y * -sin( radian ));
   // y component
   tempVector.y = (float)(vector.x * sin( radian ) + vector.y * cos( radian ));

   vector.x = tempVector.x;
   vector.y = tempVector.y;
}

////////////////////////////////////////
//	BestDelta
//
//	find the best delta (positive or negative) to add to 
//	get from start_angle to end_angle by adding the smallest
//	amount
//	the shortest distance between two angles is stored in angle_diff
//	the best direction is stored in best_delta
////////////////////////////////////////
void BestDelta (float start_angle, float end_angle, float *best_delta, float *angle_diff)
{
//	start_angle = com_AngleMod (start_angle);
//	end_angle = com_AngleMod (end_angle);	

	if (start_angle < 0) 
	{
		start_angle += 360;
	}
	if (end_angle < 0) 
	{
		end_angle += 360;
	}

	// now determine the shortest direction to turn, left or right, to
	// get to the end_angle from start_angle

	if (start_angle < end_angle)
	{
		*angle_diff = end_angle - start_angle;
		if (*angle_diff < 180)
		{
			*best_delta = 1.0;
		}
		else
		{
			*best_delta = -1.0;
			*angle_diff = fabs (*angle_diff - 360.0);
		}
	}
	else
	{
		*angle_diff = start_angle - end_angle;
		if (*angle_diff < 180)
		{
			*best_delta = -1.0;
		}
		else
		{
			*best_delta = 1.0;
			*angle_diff = fabs (*angle_diff - 360.0);
		}
	}
}

// End Scale
///////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
