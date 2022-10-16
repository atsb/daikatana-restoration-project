#include "d3d_local.h"



//-----------------------------------------------------------------------------
// Name: SetIdentityMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetIdentityMatrix( D3DMATRIX matrix )
{
	matrix._12 = matrix._13 = matrix._14 = 0;
	matrix._21 = matrix._23 = matrix._24 = 0;
	matrix._31 = matrix._32 = matrix._34 = 0;
	matrix._41 = matrix._42 = matrix._43 = 0;

	matrix._11 = matrix._22 = matrix._33 = matrix._44 = 1.0f;
}



//-----------------------------------------------------------------------------
// Name: SetTranslateMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetTranslateMatrix( D3DMATRIX &matrix, D3DVECTOR &v )
{
	matrix._11 = v.x;
	matrix._22 = v.y;
	matrix._33 = v.z;
}



//-----------------------------------------------------------------------------
// Name: SetTranslateMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetTranslateMatrix( D3DMATRIX &matrix, float x, float y, float z )
{
	matrix._11 = x;
	matrix._22 = y;
	matrix._33 = z;
}



//-----------------------------------------------------------------------------
// Name: SetRotateXMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetRotateXMatrix( D3DMATRIX &matrix, float angle )
{
    double	ra	= angle * PI / 180.f;
	float	s	= sin( ra );
	float	c	= cos( ra );

	matrix._22 = (FLOAT) c;
	matrix._23 = (FLOAT) s;
	matrix._32 = (FLOAT) -s;
	matrix._33 = (FLOAT) c;
}



//-----------------------------------------------------------------------------
// Name: SetRotateYMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetRotateYMatrix( D3DMATRIX &matrix, float angle )
{
    double	ra	= angle * PI / 180.f;
	float	s	= sin( ra );
	float	c	= cos( ra );

	matrix._11 = (FLOAT) c;
	matrix._13 = (FLOAT) -s;
	matrix._31 = (FLOAT) s;
	matrix._33 = (FLOAT) c;
}



//-----------------------------------------------------------------------------
// Name: SetRotateZMatrix
// Desc: 
//-----------------------------------------------------------------------------
void SetRotateZMatrix( D3DMATRIX &matrix, float angle )
{
    double	ra	= angle * PI / 180.f;
	float	s	= sin( ra );
	float	c	= cos( ra );

	matrix._11 = (FLOAT) c;
	matrix._12 = (FLOAT) s;
	matrix._21 = (FLOAT) -s;
	matrix._22 = (FLOAT) c;
}



//-----------------------------------------------------------------------------
// Name: MatrixMuliply
// Desc: Multiplies matrix_a by matrix_b
//-----------------------------------------------------------------------------
void MatrixMultiply( D3DMATRIX c, D3DMATRIX a, D3DMATRIX b )
{
	c._11 = (a._11 * b._11) + (a._12 * b._21) + (a._13 * b._31) + (a._14 + b._41);
	c._12 = (a._11 * b._12) + (a._12 * b._22) + (a._13 * b._32) + (a._14 + b._42);
	c._13 = (a._11 * b._13) + (a._12 * b._23) + (a._13 * b._33) + (a._14 + b._43);
	c._14 = (a._11 * b._14) + (a._12 * b._24) + (a._13 * b._34) + (a._14 + b._44);

	c._21 = (a._21 * b._11) + (a._22 * b._21) + (a._23 * b._31) + (a._24 + b._41);
	c._22 = (a._21 * b._12) + (a._22 * b._22) + (a._23 * b._32) + (a._24 + b._42);
	c._23 = (a._21 * b._13) + (a._22 * b._23) + (a._23 * b._33) + (a._24 + b._43);
	c._24 = (a._21 * b._14) + (a._22 * b._24) + (a._23 * b._34) + (a._24 + b._44);

	c._31 = (a._31 * b._11) + (a._32 * b._21) + (a._33 * b._31) + (a._34 + b._41);
	c._32 = (a._31 * b._12) + (a._32 * b._22) + (a._33 * b._32) + (a._34 + b._42);
	c._33 = (a._31 * b._13) + (a._32 * b._23) + (a._33 * b._33) + (a._34 + b._43);
	c._34 = (a._31 * b._14) + (a._32 * b._24) + (a._33 * b._34) + (a._34 + b._44);

	c._41 = (a._41 * b._11) + (a._42 * b._21) + (a._43 * b._31) + (a._44 + b._41);
	c._42 = (a._41 * b._12) + (a._42 * b._22) + (a._43 * b._32) + (a._44 + b._42);
	c._43 = (a._41 * b._13) + (a._42 * b._23) + (a._43 * b._33) + (a._44 + b._43);
	c._44 = (a._41 * b._14) + (a._42 * b._24) + (a._43 * b._34) + (a._44 + b._44);
}



//-----------------------------------------------------------------------------
// Name: CreateWorldMatrix
// Desc: creates the world matrix for a given entity
//-----------------------------------------------------------------------------
D3DMATRIX CreateWorldMatrix( entity_t *entity )
{
	D3DMATRIX	matrix_a;//, matrix_b, matrix_c;
	D3DVECTOR	position;
	
	// get the entity origin
	position.x = entity->origin[0];
	position.y = entity->origin[1];
	position.z = entity->origin[2];

	// clear the matrix
	SetIdentityMatrix( matrix_a );

	// translate
	SetTranslateMatrix( matrix_a, position );

	// check for rotation
/*
	if( entity->angles[0] || entity->angles[1] || entity->angles[2] )
	{
		D3DVECTOR	angles;

		// get the entity angles
		angles.x = entity->angles[0];
		angles.y = entity->angles[1];
		angles.z = entity->angles[2];

		// x rotation
		SetRotateXMatrix( matrix_a, angles.x );

		// y rotation
		SetRotateYMatrix( matrix_b, angles.y );

		// concatenation
		MatrixMultiply( matrix_c, matrix_b, matrix_a);

		// z rotation
		SetRotateZMatrix( matrix_b, angles.z );

		// concatenation
		MatrixMultiply( matrix_a, matrix_b, matrix_c);
	}
*/

	return matrix_a;
}



//-----------------------------------------------------------------------------
// Name: CreateViewMatrix
// Desc: 
//-----------------------------------------------------------------------------
D3DMATRIX CreateViewMatrix()
{
	D3DMATRIX	matrix;
	D3DVECTOR	right, up, pn;
	D3DVECTOR	origin;

	modelorg = r_newrefdef.vieworg;
	r_origin = r_newrefdef.vieworg;
	AngleToVectors (r_newrefdef.viewangles, vpn, vright, vup);

	SetIdentityMatrix( matrix );

	right.x = vright[0];
	right.y = vright[1];
	right.z = vright[2];

	up.x = vup[0];
	up.y = vup[1];
	up.z = vup[2];

	pn.x = vpn[0];
	pn.y = vpn[1];
	pn.z = vpn[2];

	origin.x = r_origin[0];
	origin.y = r_origin[1];
	origin.z = r_origin[2];

	matrix._11 = right.x;
	matrix._21 = right.y;
	matrix._31 = right.z;

	matrix._12 = up.x;
	matrix._22 = up.y;
	matrix._32 = up.z;

	matrix._13 = pn.x;
	matrix._23 = pn.y;
	matrix._33 = pn.z;

	matrix._41 = -(right.x * origin.x + right.y * origin.y + right.z * origin.z);
	matrix._42 = -(up.x * origin.x + up.y * origin.y + up.z * origin.z);
	matrix._43 = -(pn.x * origin.x + pn.y * origin.y + pn.z * origin.z);

	return matrix;
}



//-----------------------------------------------------------------------------
// Name: CreateProjectionMatrix
// Desc: 
//-----------------------------------------------------------------------------
D3DMATRIX CreateProjectionMatrix( float nplane, float fplane, float hfov, float vfov )
{
	D3DMATRIX	matrix;
	float		h, w, Q;

	SetIdentityMatrix( matrix );

	w = (float)sin( hfov * 0.5 );
    h = (float)cos( vfov * 0.5 );
    Q = fplane/(fplane - nplane); 

	matrix._11 = w;
	matrix._22 = h;
	matrix._33 = Q;
	matrix._43 = -Q * nplane;
	matrix._34 = 1;
    
    return matrix;
}