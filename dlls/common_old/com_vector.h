#ifndef _COM_VECTOR_H
#define _COM_VECTOR_H

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define v_len(v) (sqrt(v.x*v.x + v.y*v.y + v.z*v.z))

#define v_dist(v1, v2)                         \
(                                             \
    sqrt( ((v2.x - v1.x) * (v2.x - v1.x)) +   \
	      ((v2.y - v1.y) * (v2.y - v1.y)) +   \
		  ((v2.z - v1.z) * (v2.z - v1.z)) )   \
)

class	Vector
{
protected:
    // for internal converions
	float vec3d[3];

public:
    float x, y, z;    // pitch, yaw, roll

	// constructors
	Vector(void)                         { x = y = z = (float) 0.0; };
	Vector(float x1, float y1, float z1) { x = x1; y = y1; z = z1; };
	Vector(float v[3])                   { x = v[0]; y = v[1]; z = v[2]; };
    ~Vector(void);

	// overloading
	Vector	operator+(const Vector &b) const;
	Vector	operator-(const Vector &b) const;
	Vector	operator-() const;
	Vector	operator*(float n) const;
	int		operator==(const Vector &vec1) const;
	int		operator!=(const Vector &vec1) const;
	float	Vector::operator^(const Vector &v2) const;
	Vector	Vector::operator%(const Vector &v2) const;

	// more functions
	void	AngleVectors(Vector &forward, Vector &right, Vector &up);
	void	Normalize (void);
	float	vlen (void);
	float	vdist (Vector &v2);

	// for angles
	float pitch(void) { return(x); };
	float yaw(void)   { return(y); };
	float roll(void)  { return(z); };

	// conversion into quake vectors
	float *vec(void) { vec3d[0] = x; vec3d[1] = y; vec3d[2] = z;  return((float *) vec3d); };
	void   set(float oldvec[3]) { oldvec[0] = x; oldvec[1] = y; oldvec[2] = z; };


	// friend functions
	friend Vector operator*(float n, const Vector &b);
};

inline Vector operator *( float n, const Vector& b )
{
	return(b * n);
}

#endif