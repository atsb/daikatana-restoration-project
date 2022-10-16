#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "katanox.h"
#include "anoxcam.h"
#include "glsub.h"


float gl_fCycleTime = 0.0f; // time since program start

//---------------------------------------------------------------------------
//                       Object drawing routines
//---------------------------------------------------------------------------


#define NUM_CAM_VERTS 40

float CamVerts[40][3] = { 
{-2.000000,-2.500000,6.100000},{-2.000000,4.500000,6.100000},
{-2.000000,4.500000,-5.500000},{-2.000000,-2.500000,-5.500000},
{2.000000,-2.500000,6.099999},{2.000000,4.500000,6.099999},
{2.000000,4.500000,-5.500000},{2.000000,-2.500000,-5.500000},
{2.676265,1.208800,-9.749737},{1.352067,3.741058,-9.719719},
{-1.296329,3.741058,-9.659683},{-2.620527,1.208800,-9.629665},
{-1.296329,-1.323458,-9.659683},{1.352067,-1.323458,-9.719719},
{0.014125,1.228000,-6.518251},{0.156265,1.228000,-1.210746},
{-0.569601,10.637692,-1.524439},{-0.569600,10.605190,5.416684},
{-0.569600,10.626856,2.007679},{-0.569601,10.660772,-5.029007},
{-0.569600,7.247909,0.325335},{-0.569601,7.339701,-6.860432},
{-0.569600,4.271886,2.155921},{-0.569601,4.271886,-4.814078},
{-0.569601,4.252015,-1.201496},{-0.569600,4.252015,5.768504},
{-0.569600,7.204144,7.145016},{-0.569600,7.293999,0.354461},
{0.640000,10.637692,-1.524439},{0.640000,10.605190,5.416684},
{0.640000,10.626856,2.007679},{0.640000,10.660772,-5.029007},
{0.640000,7.247909,0.325335},{0.640000,7.339701,-6.860432},
{0.640000,4.271886,2.155921},{0.640000,4.271886,-4.814078},
{0.640000,4.252015,-1.201496},{0.640000,4.252015,5.768504},
{0.640000,7.293999,0.354461},{0.640000,7.204144,7.145016},};

#define NUM_CAM_POLYS 60

int CamPolys[60][3] = {{23,21,19},{24,23,19},{27,24,19},{16,27,19},{19,31,28},
{16,19,28},{21,33,31},{19,21,31},{23,35,33},{21,23,33},{27,38,36},{24,27,36},
{16,28,38},{27,16,38},{33,35,31},{35,36,31},{36,38,31},{38,28,31},{22,20,18},
{25,22,18},{26,25,18},{17,26,18},{18,30,29},{17,18,29},{20,32,30},{18,20,30},
{22,34,32},{20,22,32},{26,39,37},{25,26,37},{17,29,39},{26,17,39},{32,34,30},
{34,37,30},{37,39,30},{39,29,30},{8,9,14},{9,10,14},{10,11,14},{11,12,14},
{12,13,14},{13,8,14},{9,8,15},{10,9,15},{11,10,15},{12,11,15},{13,12,15},
{8,13,15},{0,4,7},{7,3,0},{3,7,6},{6,2,3},{2,6,5},{5,1,2},{1,5,4},{4,0,1},
{0,3,2},{2,1,0},{6,7,4},{4,5,6},};

void DrawCameraObject( const VEC3& pos, const VEC3& ang )
{
    int i;
    int *ptr;
    float f;

    glPushMatrix();
    {
        glTranslatef( pos[0], pos[1], pos[2] );
        glRotatef( ang[1]-90, 0.0, 0.0, 1.0 ); //yaw
        glRotatef( ang[0]+90, 1.0, 0.0, 0.0 ); //pitch
        glRotatef( ang[2], 0.0, 1.0, 0.0 ); //roll

        glBegin( GL_TRIANGLES );
        {
            for( i = 0, ptr = &CamPolys[0][0]; i < 60; i++ )
            {
                f = fmod( float(((i%10)+1)/10.0f) + gl_fCycleTime, 1.0f );

                glColor3f( f,f,f );
                glVertex3fv( &CamVerts[*ptr][0] ); ++ptr;
                glVertex3fv( &CamVerts[*ptr][0] ); ++ptr;
                glVertex3fv( &CamVerts[*ptr][0] ); ++ptr;
            }
        }
        glEnd();
    }
    glPopMatrix();
}


#define X .525731112119133606F
#define Z .850650808352039932F

static GLfloat icodata[12][3]={
    {-X,0.0,Z},{X,0.0,Z},{-X,0.0,-Z},{X,0.0,-Z},
    {0.0,Z,X},{0.0,Z,-X},{0.0,-Z,X},{0.0,-Z,-X},
    {Z,X,0.0},{-Z,X,0.0},{Z,-X,0.0},{-Z,-X,0.0}
};

static GLfloat iconormals[20][3];

static GLuint tindices[20][3] = {
    {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
    {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
    {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
    {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

#undef X
#undef Z

void DrawIcosahedron( const VEC3& v, const VEC3& color, float radius )
{
    int i;
    static float t, scale;
    static int initted = FALSE;
/*
    if( ! initted )
    {
        VEC3 v1, v2, n;

        for( i = 0; i < 20; i++ )
        {
            v1 = VEC3(icodata[tindices[i][1]]) - VEC3(icodata[tindices[i][0]] );
            v2 = VEC3(icodata[tindices[i][2]]) - VEC3(icodata[tindices[i][0]] );
            n = cross( v1,v2 ).normalize();

            iconormals[i][0] = n[0];
            iconormals[i][1] = n[1];
            iconormals[i][2] = n[2];
        }

        initted = TRUE;
    }
*/

    glPushMatrix();

        glTranslatef( v[0], v[1], v[2] );
        glScalef( radius, radius, radius );

        glBegin( GL_TRIANGLES );
            for( i = 0; i < 20; i++ )
            {
                scale = fmod( i/20.0f + gl_fCycleTime, 1.0f );

                glColor3f( color[0]*scale, color[1]*scale, color[2]*scale );
                //glNormal3fv( &iconormals[i][0] );
                glVertex3fv( &icodata[tindices[i][0]][0] );
                glVertex3fv( &icodata[tindices[i][1]][0] );
                glVertex3fv( &icodata[tindices[i][2]][0] );
            }
        glEnd();

    glPopMatrix();
}



#define Y  0.8660254037844
static GLfloat tetradata[4][3] = {
    { 1, 0, 0 }, { -0.5, Y, 0 }, {-0.5, -Y, 0}, {0, 0, 1.2} };
#undef Y

void DrawMarker( const VEC3& v, float radius, const VEC3& c1, const VEC3& c2 )
{
    glPushMatrix();
    {
        glTranslatef( v[0], v[1], v[2] );
        glScalef( radius, radius, radius );

        glBegin( GL_TRIANGLE_FAN );
        {
            glColor3f( c1[0], c1[1], c1[2] );
            glVertex3fv( tetradata[3] );
            glColor3f( c2[0], c2[1], c2[2] );
            glVertex3fv( tetradata[0] );
            glVertex3fv( tetradata[2] );
            glVertex3fv( tetradata[1] );
            glVertex3fv( tetradata[0] );
        }
        glEnd();

        glBegin( GL_TRIANGLES );
        {
            glVertex3fv( tetradata[0] );
            glVertex3fv( tetradata[1] );
            glVertex3fv( tetradata[2] );
        }
        glEnd();
    }
    glPopMatrix();
}


float ArrowVerts[13][3] = { 
    {0.052680,0.038626,-8.980340},{0.916016,-0.881032,-2.361436},
    {-0.933989,0.845639,-2.361436},{0.916016,0.845639,-2.361436},
    {-0.933989,-0.881032,-2.361436},{2.170672,-2.096203,-1.826989},
    {0.916016,-0.881032,8.980340},{-2.170672,2.096203,-1.826989},
    {-0.933989,0.845639,8.980340},{0.916016,0.845639,8.980340},
    {2.170672,2.096203,-1.826989},{-2.170672,-2.096203,-1.826989},
    {-0.933989,-0.881032,8.980340},};

int ArrowPolys[18][3] = {
    {4,1,3},{3,2,4},{1,6,9},{9,3,1},{3,9,8},{8,2,3},{2,8,12},
    {12,4,2},{4,12,6},{6,1,4},{9,6,12},{12,8,9},{10,0,5},{7,0,10},
    {11,0,7},{5,0,11},{10,5,11},{11,7,10},};


void DrawArrow( const VEC3& pos, const VEC3& ang, const VEC3& color1 )
{
    int i, *ptr;
    float scale;
    float f1, f2, f3;

    glPushMatrix();
    {
        glTranslatef( pos[0], pos[1], pos[2] );
        glRotatef( ang[1]-90, 0.0, 0.0, 1.0 ); //yaw
        glRotatef( ang[0]+90, 1.0, 0.0, 0.0 ); //pitch
        glRotatef( ang[2], 0.0, 1.0, 0.0 ); //roll

        glBegin( GL_TRIANGLES );
        {
            f1 = color1[0]; f2 = color1[1]; f3 = color1[2];

            for( i = 0, ptr = &ArrowPolys[0][0]; i < 18; i++ )
            {
                scale = float(i)/18.0f;

                glColor3f( f1*scale, f2*scale, f3*scale );
                
                glVertex3fv( &ArrowVerts[*ptr][0] ); ++ptr;
                glVertex3fv( &ArrowVerts[*ptr][0] ); ++ptr;
                glVertex3fv( &ArrowVerts[*ptr][0] ); ++ptr;
            }
        }
        glEnd();
    }
    glPopMatrix();
}