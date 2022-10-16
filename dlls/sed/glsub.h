#ifndef _GLSUB_H
#define _GLSUB_H

void DrawCameraObject( const VEC3& pos, const VEC3& ang );
void DrawIcosahedron( const VEC3& v, const VEC3& color, float radius );
void DrawMarker( const VEC3& v, float radius, const VEC3& c1, const VEC3& c2 );
void DrawArrow( const VEC3& pos, const VEC3& ang, const VEC3& color1 );

extern float gl_fCycleTime;

#endif