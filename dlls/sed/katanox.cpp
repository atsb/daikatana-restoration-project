#include	<windows.h>
#include	<math.h>

#include	"katanox.h"

//---------------------------------------------------------------------------
//                              DLL stuff
//---------------------------------------------------------------------------

//	Nelno:	gone
/*
int WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
*/

//---------------------------------------------------------------------------
//                               BEZIER
//---------------------------------------------------------------------------


BEZIER& BEZIER::assign( const VEC3& P1, const VEC3& P2, const VEC3& P3, const VEC3& P4 )
{
    v3_ =   -P1 + 3*P2 - 3*P3 + P4;
    v2_ =  3*P1 - 6*P2 + 3*P3;
    v1_ = -3*P1 + 3*P2;
    v0_ =    P1;

    dv2_ = 3 * v3_;
    dv1_ = 2 * v2_;

    return *this;
}


//---------------------------------------------------------------------------
//                       Some more VEC3 functions
//---------------------------------------------------------------------------


VEC3 VEC3::VecToAngle() const
{
    float yaw, pitch, fw;

    if( gmIsZero(x_) && gmIsZero(y_) )
    {
        yaw = 0.0;
        if( z_ > 0.0 ) pitch = 90.0;
        else          pitch = -90.0;
    }
    else
    {
        yaw = atan2( y_, x_ ) * gmRADTODEG;
        if( yaw < 0.0 ) yaw += 360.0;

        fw = sqrt( x_*x_ + y_*y_ );
        pitch = atan2(z_, fw) * gmRADTODEG;
        //if( pitch < 0.0 ) pitch += 360.0;
    }

    return VEC3( pitch, yaw, 0.0 );
}


void  VEC3::AngleVecs( VEC3& fwd, VEC3& right, VEC3& up )
{
    float angle;
    float sr, sp, sy, cr, cp, cy, spcy, spsy;

    angle = yaw_ * gmDEGTORAD;
    sy = sin(angle); cy = cos(angle);

    angle = pitch_ * gmDEGTORAD;
    sp = sin(angle); cp = cos(angle);

    angle = roll_ * gmDEGTORAD;
    sr = sin(angle); cr = cos(angle);

    fwd = VEC3( cp*cy, cp*sy, -sp );
    
    spcy = sp*cy; spsy = sp*sy;

    right = VEC3( -sr*spcy + cr*sy,
                  -sr*spsy - cr*cy,
                  -sr*cp );

    up = VEC3( cr*spcy + sr*sy,
               cr*spsy - sr*cy,
               cr*cp );
}


//---------------------------------------------------------------------------
//                               CUBILERP
//---------------------------------------------------------------------------


CUBILERP::CUBILERP()
{
    t_elapsed = 0; t_eta = 0;
}


CUBILERP::CUBILERP( const VEC3& init, const VEC3& dest, float eta )
{
    assert( eta > 0 );

    final = dest;
    t_elapsed = 0; t_eta = eta;
    s0 = s = init;
    // j = (xf-xi-v0t-a0t^2/2)*6/t^3
    j = ( final - init ) * (6.0 / (eta*eta*eta)); // v and a are 0,0,0
}


CUBILERP::CUBILERP( const VEC3& init_s, const VEC3& final_s, 
                    const VEC3& init_v, const VEC3& init_a, float eta )
{
    assert( eta > 0 );

    final = final_s;
    t_elapsed = 0; t_eta = eta;

    s0 = s = init_s;
    v0 = v = init_v;
    a0 = a = init_a;

    j = ( final - init_s - v*eta - a*(eta*eta*0.5) ) * (6.0/(eta*eta*eta));
}


void CUBILERP::update( const VEC3& dest, float eta )
{
    assert( eta > 0 );

    t_elapsed = 0; t_eta = eta;
    final = dest;

    j = ( dest - s - v*eta - a*(eta*eta*0.5) ) * (6.0/(eta*eta*eta));
    a0 = a;
    v0 = v;
    s0 = s;
    //a0 = -v0/eta - 0.5*j*eta;
}


void CUBILERP::iterate( float t_new )
{
    t_elapsed += t_new;

    if( t_elapsed > t_eta )
    {
        // whoops we went over
        s = final;
    }
    else
    {
        float t, t2;

        t = t_elapsed; t2 = 0.5*t*t;

        a = a0  +   j * t;
        v = v0  +  a0 * t  +  j * t2 ;
        s = s0  +  v0 * t  +  a0* t2  +  j * (t2*t/3.0);
    }
}


VEC3 CUBILERP::getPos()
{
    return s;
}


VEC3 CUBILERP::getVel()
{
    return v;
}


VEC3 CUBILERP::getVal( float big_t )
{
    float t, t2;

    assert( (big_t>=0.0) && (big_t<=1.0) );
    t = t_eta * big_t;
    t2 = 0.5*t*t;

    return( s0 + v0 * t + a0 * t2 + j * (t2*t/3.0) );
}


//---------------------------------------------------------------------------
//                               CUBIMOVE
//---------------------------------------------------------------------------


void CUBIMOVE::recalc()
{
    j = (6.0/(t_f*t_f))*(v_f+v_i-(2/t_f)*(s_f-s_i));
    a_i = (v_f-v_i)/t_f-0.5*t_f*j;
}


CUBIMOVE::CUBIMOVE()
{
    t_f = 0.0; 
    t = 1.0;   // set time to greater than time allowed,
               // so everything will automatically timeout
    
    s = s_f = VEC3(); 
    v = v_f = VEC3();
}


CUBIMOVE::CUBIMOVE( const VEC3& p1, const VEC3& p2,
                    const VEC3& v1, const VEC3& v2,
                    float t1, float w )
{
    s = s_i = p1; s_f = p2;
    v = v_i = v1; v_f = v2;
    t = 0; t_f = t1;
    
    if( w != 0.0 ) 
    {
        bWrapSet = TRUE;
        fWrap = w;
    }
    else
    {
        bWrapSet = FALSE;
        fWrap = 0.0;
    }

    recalc();
}


void CUBIMOVE::update( const VEC3& s_new, float t_new )
{
    s_f = s_new;
    t_f = t_new;

    t = 0;
    s_i = s;
    v_i = v;

    recalc();
}


void CUBIMOVE::update( const VEC3& s_new, const VEC3& v_new, float t_new )
{
    v_f = v_new;
    update( s_new, t_new );
}


float CUBIMOVE::iterate( float frametime )
{
    float t2;

    t += frametime;

    if( t < t_f )
    {
        t2 = 0.5*t*t;

        s = s_i + (v_i * t) + (a_i * t2) + (j * (t2*t/3.0));
        v = v_i + (a_i * t) + (j * t2);
    }
    else
    {
        s = s_f;
        v = v_f;
    }

    return t_f - t;
}


float CUBIMOVE::iterateT( float tpercent )
{
    t = tpercent * t_f;
    iterate(0);

    return 1.0-tpercent;
}


VEC3 CUBIMOVE::getPos()
{
    return s;
}


VEC3 CUBIMOVE::getVel()
{
    return v;
}


//---------------------------------------------------------------------------
//                               FLOATMOVE
//
// Simulation of stuff floating in a fluid.
// Note that the fluid viscosity parameter is ignored (not yet implemented).
//---------------------------------------------------------------------------


FLOATMOVE::FLOATMOVE( float object_mass, float object_volume, 
                      float fluid_density, float fluid_visc,
                      const VEC3& gravity, const VEC3& init_vel )
{
    fDensObject = object_mass / object_volume;
    fDensFluid  = fluid_density;
    fViscosity  = fluid_visc;
    fRatio      = fDensFluid / fDensObject;

    vGrav = gravity;
    vVel  = init_vel;
    vAccel= VEC3();
}


void FLOATMOVE::iterate( float frametime, float percent_submerged )
{
    if( percent_submerged > 0.0 )
    {
        vVel += vGrav * (frametime*( fRatio * percent_submerged - 1.0 ));
    }
    else
    {
        vVel += vGrav * frametime;
    }
}


VEC3 FLOATMOVE::getVel()
{
    return vVel;
}


void FLOATMOVE::setVel( const VEC3& new_vel )
{
    vVel = new_vel;
}