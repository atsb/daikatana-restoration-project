// mdm 98.02.08 - not portable
// #pragma once

#ifndef _DK_SCREEN_COORD_H_
#define _DK_SCREEN_COORD_H_

void ComputeScreenCoordinates(int32 screen_width, int32 screen_height, int32 border_left, int32 border_top,
    float fov_x, float fov_y, const float *v_origin, const float *v_angles, 
    const float *world_point, int32 *screen_x, int32 *screen_y);

#endif // _DK_SCREEN_COORD_H_