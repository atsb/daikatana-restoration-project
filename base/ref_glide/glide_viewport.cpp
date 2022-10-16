#define __MSC__
#include <glide.h>
#include "ref.h"

#include "dk_misc.h"

#include "glide_ref_main.h"
#include "glide_frame_buffer.h"

//the parameters that define the viewport.
float to_screen_scale_x, to_screen_scale_y;
float to_screen_offset_x, to_screen_offset_y;

void ViewportSetup(int32 width, int32 height, int32 left, int32 top, float width_radians, float height_radians) {
    //make sure we are not in 2d mode.
    Leave2dMode();

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

    //compute right and bottom of the clipping window.
    int32 right = left + width;
    int32 bottom = top + height;

    //make sure the boundaries are on the screen.
    bound_min(left, 0);
    bound_min(top, 0);
    bound_max(right, current_video_width);
    bound_max(bottom, current_video_height);

    //tell glide to clip to the given region.
    grClipWindow(left, top, right, bottom);

    //clear the z buffer in this window.

}

