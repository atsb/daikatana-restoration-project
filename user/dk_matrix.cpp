// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include <math.h>
#include "memmgrcommon.h"
#include "dk_point.h"
#include "dk_matrix.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */


// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CMatrix::mul(const CMatrix &left, const CMatrix &right) 
{
    rows[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] + left[0][2] * right[2][0];
    rows[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] + left[0][2] * right[2][1];
    rows[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] + left[0][2] * right[2][2];
    rows[0][3] = left[0][0] * right[0][3] + left[0][1] * right[1][3] + left[0][2] * right[2][3] + left[0][3];

    rows[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] + left[1][2] * right[2][0];
    rows[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] + left[1][2] * right[2][1];
    rows[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] + left[1][2] * right[2][2];
    rows[1][3] = left[1][0] * right[0][3] + left[1][1] * right[1][3] + left[1][2] * right[2][3] + left[1][3];

    rows[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] + left[2][2] * right[2][0];
    rows[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] + left[2][2] * right[2][1];
    rows[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] + left[2][2] * right[2][2];
    rows[2][3] = left[2][0] * right[0][3] + left[2][1] * right[1][3] + left[2][2] * right[2][3] + left[2][3];
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
void CMatrix::MakeTransformation( const CVector &forward, const CVector &right, const CVector &up, const CVector &origin ) 
{
    rows[0][0] = right.X();
    rows[0][1] = right.Y();
    rows[0][2] = right.Z();
    rows[0][3] = -origin.DotProduct(right);

    rows[1][0] = -up.X();
    rows[1][1] = -up.Y();
    rows[1][2] = -up.Z();
    rows[1][3] = origin.DotProduct(up);

    rows[2][0] = forward.X();
    rows[2][1] = forward.Y();
    rows[2][2] = forward.Z();
    rows[2][3] = -origin.DotProduct(forward);
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
void CMatrix::MakeTransformation(const CVector &angle_radians, const CVector &origin) 
{
    //convert the angles to vectors.
    static CVector forward, right, up;
    angle_radians.AngleToVectors(forward, right, up);

    //use the angle vectors to make the transform.
    MakeTransformation(forward, right, up, origin);
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
void CMatrix::MakeScale(const CVector &scale) 
{
    //put the scale factors into the CMatrix.
    rows[0][0] = scale.X();
    rows[0][1] = 0.0f;
    rows[0][2] = 0.0f;
    rows[0][3] = 0.0f;

    rows[1][0] = 0.0f;
    rows[1][1] = scale.Y();
    rows[1][2] = 0.0f;
    rows[1][3] = 0.0f;

    rows[2][0] = 0.0f;
    rows[2][1] = 0.0f;
    rows[2][2] = scale.Z();
    rows[2][3] = 0.0f;
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
void CMatrix::MakeTranslation(const CVector &disp) 
{
    rows[0][0] = 1.0f;
    rows[0][1] = 0.0f;
    rows[0][2] = 0.0f;
    rows[0][3] = disp.X();

    rows[1][0] = 0.0f;
    rows[1][1] = 1.0f;
    rows[1][2] = 0.0f;
    rows[1][3] = disp.Y();

    rows[2][0] = 0.0f;
    rows[2][1] = 0.0f;
    rows[2][2] = 1.0f;
    rows[2][3] = disp.Z();
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
void CMatrix::MakeRotation(const CVector &forward, const CVector &right, const CVector &up) 
{
    rows[0][0] = forward.X();
    rows[0][1] = forward.Y();
    rows[0][2] = forward.Z();
    rows[0][3] = 0.0f;

    rows[1][0] = -right.X();
    rows[1][1] = -right.Y();
    rows[1][2] = -right.Z();
    rows[1][3] = 0.0f;

    rows[2][0] = up.X();
    rows[2][1] = up.Y();
    rows[2][2] = up.Z();
    rows[2][3] = 0.0f;
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
void CMatrix::MakeRotation(const CVector &angle_radians) 
{
    //convert the angles to vectors.
    static CVector forward, right, up;
    angle_radians.AngleToVectors(forward, right, up);

    //use the angle vectors to make the transform.
    MakeRotation(forward, right, up);
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
