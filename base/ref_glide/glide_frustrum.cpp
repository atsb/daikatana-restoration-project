#define __MSC__
#include <glide.h>
#include <math.h>
#include "ref.h"
#include "dk_misc.h"

#include "dk_point.h"
#include "dk_matrix.h"

#include "glide_plane.h"
#include "glide_frustrum.h"

//the current frustrum parameters.
static float width_radians;
static float height_radians;
static point angles_radians;
static point origin;

//the frustrum normals in view space, facing down positive x, with negative y to the right.
//we use these to do 3d clipping.
static point view_space_frustrum_normal_bottom;
static point view_space_frustrum_normal_top;
static point view_space_frustrum_normal_left;
static point view_space_frustrum_normal_right;

//the frustrum planes in world space.  we use these for bounding box culling.
static plane world_space_frustrum_plane_bottom;
static plane world_space_frustrum_plane_top;
static plane world_space_frustrum_plane_left;
static plane world_space_frustrum_plane_right;

//the world to view transform matrix and rotation matrix.
static matrix world_to_view;

void FrustrumSetup(float width_radians, float height_radians, point &angles_degrees, point &origin) {
    //set our global variables.
    angles_radians.eq(to_radians(angles_degrees.x), to_radians(angles_degrees.y), to_radians(angles_degrees.z));
    ::origin = origin;

    //get the sine and cosine of half the field of view width and height 
    float sw = sin(width_radians / 2.0f);
    float sh = sin(height_radians / 2.0f);

    float cw = cos(width_radians / 2.0f);
    float ch = cos(height_radians / 2.0f);

    //get the forward, up, and right vectors that correspond to the view angles.
    point forward, right, up;
    angles_radians.angle_vectors(forward, right, up);

    //
    //compute the world to view transformation.
    //

    world_to_view.MakeTransformation(forward, right, up, origin);

    //
    //compute the view space frustrum normals.
    //

    //construct the normals of the planes.
    view_space_frustrum_normal_bottom.eq(sh, 0.0f, ch);
    view_space_frustrum_normal_top.eq(sh, 0.0f, -ch);
    view_space_frustrum_normal_left.eq(sw, -cw, 0.0f);
    view_space_frustrum_normal_right.eq(sw, cw, 0.0f);
    
    //
    //compute the world space frustrum planes.
    //

    //combine forward and up to get the normals of the world space frustrum top and bottom planes.
    world_space_frustrum_plane_bottom.eq(up, ch, forward, sh);
    world_space_frustrum_plane_top.eq(up, -ch, forward, sh);

    //combine forward and right to get the normals of the world space frustrum left and right planes.
    world_space_frustrum_plane_left.eq(right, cw, forward, sw);
    world_space_frustrum_plane_right.eq(right, -cw, forward, sw);

    //compute the d's of the 4 planes.
    world_space_frustrum_plane_bottom.compute_d(origin);
    world_space_frustrum_plane_top.compute_d(origin);
    world_space_frustrum_plane_left.compute_d(origin);
    world_space_frustrum_plane_right.compute_d(origin);
}

const matrix &WorldToView() {
    return world_to_view;
}




