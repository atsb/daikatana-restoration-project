//#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "p_user.h"

#include "vector.h"

Vector::~Vector(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	+ operator
///////////////////////////////////////////////////////////////////////////////

Vector Vector::operator+(const Vector &b) const
{
	float sx, sy, sz;

	sx = x + b.x;
	sy = y + b.y;
	sz = z + b.z;

	Vector sum = Vector(sx, sy, sz);
	return(sum);
}

///////////////////////////////////////////////////////////////////////////////
//	- operator
///////////////////////////////////////////////////////////////////////////////

Vector Vector::operator-(const Vector &b) const
{
	float sx, sy, sz;

	sx = x - b.x;
	sy = y - b.y;
	sz = z - b.z;

	Vector diff = Vector(sx, sy, sz);
	return(diff);
}

///////////////////////////////////////////////////////////////////////////////
//	negation operator
///////////////////////////////////////////////////////////////////////////////

Vector Vector::operator-() const
{
	float nx, ny, nz;

	nx = -x;
	ny = -y;
	nz = -z;

	Vector neg = Vector(nx, ny, nz);
	return(neg);
}

///////////////////////////////////////////////////////////////////////////////
//	* operator
///////////////////////////////////////////////////////////////////////////////

Vector Vector::operator*(float n) const
{
	float mx, my, mz;

	mx = n * x;
	my = n * y;
	mz = n * z;

	Vector mult = Vector(mx, my, mz);
	return(mult);
}

///////////////////////////////////////////////////////////////////////////////
//	^ dot product operator
///////////////////////////////////////////////////////////////////////////////

float	Vector::operator^(const Vector &v2) const
{
	return (x * v2.x + y * v2.y + z * v2.z);
}

///////////////////////////////////////////////////////////////////////////////
//	% 
//
//	cross product operator
///////////////////////////////////////////////////////////////////////////////

Vector	Vector::operator%(const Vector &v2) const
{
	Vector	cross;

	cross.x = y * v2.z - z * v2.y;
	cross.y =  - (x * v2.z - z * v2.x);
	cross.z = x * v2.y - y * v2.x;

	return (cross);
}

///////////////////////////////////////////////////////////////////////////////
//	=!
//
//	not freaking equal operator
///////////////////////////////////////////////////////////////////////////////

int Vector::operator!=(const Vector &vec1) const
{
	if (x != vec1.x || y != vec1.y || z != vec1.z)
		return 1;
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//	==
//
//	compare operator
///////////////////////////////////////////////////////////////////////////////

int Vector::operator==(const Vector &vec1) const
{
	if (x == vec1.x && y == vec1.y && z == vec1.z)
		return 1;
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
//	AngleVectors
//
//	return forward, right and up vectors for a vector storing pitch, yaw and
//	roll in angles
///////////////////////////////////////////////////////////////////////////////

void Vector::AngleVectors(Vector &forward, Vector &right, Vector &up)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = yaw() * (float)(M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = pitch() * (float)(M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = roll() * (float)(M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward.x = cp*cy;
	forward.y = cp*sy;
	forward.z = -sp;

	right.x = (-1*sr*sp*cy+-1*cr*-sy);
	right.y = (-1*sr*sp*sy+-1*cr*cy);
	right.z = -1*sr*cp;

	up.x = (cr*sp*cy+-sr*-sy);
	up.y = (cr*sp*sy+-sr*cy);
	up.z = cr*cp;
}

///////////////////////////////////////////////////////////////////////////////
//	Normalize
//
//	normalize a vector
///////////////////////////////////////////////////////////////////////////////

void	Vector::Normalize (void)
{
	float	newv;

	// find the magnitude of the vector
	newv = x * x + y * y + z * z;
	newv = sqrt (newv);
	
	if (newv == 0)
		x = y = z = (float) 0;
	else
	{
		// divide 1 by magnitude
		newv = 1 / newv;
		x = x * newv;
		y = y * newv;
		z = z * newv;
	}

}

///////////////////////////////////////////////////////////////////////////////
//	vlen
//
//	return the magnitude of a vector
///////////////////////////////////////////////////////////////////////////////

float	Vector::vlen (void)
{
	return (float) sqrt (x * x + y * y + z * z);
}

///////////////////////////////////////////////////////////////////////////////
//	vdist
//
//	return the distance between two points
///////////////////////////////////////////////////////////////////////////////

float	Vector::vdist (Vector &v2)
{
    return (float) sqrt( ((v2.x - x) * (v2.x - x)) + ((v2.y - y) * (v2.y - y)) + ((v2.z - z) * (v2.z - z)) );
}

///////////////////////////////////////////
//	rnd
//
//	returns a random float between 0 and 1
///////////////////////////////////////////

float	rnd (void)
{
	return (float)rand () / (float)RAND_MAX;
}
