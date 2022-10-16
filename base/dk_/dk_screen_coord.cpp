#include "dk_shared.h"
#include "dk_misc.h"

#include "dk_point.h"
#include "dk_matrix.h"

#include "dk_screen_coord.h"

void FrustrumSetup(float width_radians, float height_radians, CVector &angles_degrees, CVector &origin);
void ViewportSetup(int32 width, int32 height, int32 left, int32 top, float width_radians, float height_radians);

//the world to view transform matrix.
static CMatrix world_to_view;

//the parameters that define the viewport.
static float to_screen_scale_x, to_screen_scale_y;
static float to_screen_offset_x, to_screen_offset_y;

void ComputeScreenCoordinates(int32 screen_width, int32 screen_height, int32 border_left, int32 border_top,
                              float fov_x, float fov_y, const float *v_origin, const float *v_angles, 
                              const float *world_point, int32 *screen_x, int32 *screen_y)
{
    if (v_origin == NULL) return;
    if (v_angles == NULL) return;
    if (world_point == NULL) return;
    if (screen_x == NULL || screen_y == NULL) return;

    //compute the field of view width and height in radians.
    float width_radians = to_radians(fov_x);
    float height_radians = to_radians(fov_y);
    
    //set up the world to view matrix
    CVector view_origin(v_origin);
    CVector frustrum_angles_degrees(v_angles);
    FrustrumSetup(width_radians, height_radians, frustrum_angles_degrees, view_origin);

    //set up the viewport.
    ViewportSetup(screen_width, screen_height, border_left, border_top, width_radians, height_radians);

    //transform the given world point to view space.
    CVector world_space_point(world_point);
    CVector view_space_point;
    view_space_point.Multiply(world_to_view, world_space_point);

    //
    //get screen coordinates for the view point.
    //

    //get the inverse of z.
    float ooz = 1.0f / view_space_point.z;

    //project the view x and y down to the view plane by dividing by z.
    float x, y;
    x = view_space_point.x * ooz;
    y = view_space_point.y * ooz;

    //scale and offset x and y to get real screen coordinates.
    *screen_x = x * to_screen_scale_x + to_screen_offset_x;
    *screen_y = y * to_screen_scale_y + to_screen_offset_y;
}

void ViewportSetup(int32 width, int32 height, int32 left, int32 top, float width_radians, float height_radians) {
    //compute the width of the view 1 unit in front of the camera.
    float view_plane_width = float(2.0f * tan(width_radians * 0.5f));

    //compute the height of the view 1 unit in front of the camera.
    float view_plane_height = float(2.0f * tan(height_radians * 0.5f));

    //the scales are the screen widths over the view plane widths
    to_screen_scale_x = width / view_plane_width;
    to_screen_scale_y = height / view_plane_height;

    //the offsets are half the widths plus the shifts.
    to_screen_offset_x = width * 0.5f + left;
    to_screen_offset_y = height * 0.5f + top;
}

void FrustrumSetup(float width_radians, float height_radians, CVector &angles_degrees, CVector &origin) 
{
    CVector forward, right, up;
	angles_degrees.AngleToVectors( forward, right, up );

    //
    //compute the world to view transformation.
    //

    world_to_view.MakeTransformation(forward, right, up, origin);
}




