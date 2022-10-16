#pragma once

class point;
class dk_model;
class dk_model_frame_decompressed;
class dk_model_triangle;

//
//  Functions that do some actual work on the screen.
//

void GlideClearBuffer();
void GlideFlip(int refresh_waits);

void GlideDrawString(int x, int y, const char *str);

//
// Structures used to pass parameters used for drawing model triangles.
//

typedef struct {
    //location on screen.
    float x, y;
    
    //depth parameter.
    float oow;

    //original 1/z.
    float ooz;

    //light intensity.
    float light_intensity;
} screen_coordinate;

typedef struct {
    //the points in view space.  We include space for 4 points because when the 
    //poly is clipped it could get a 4th point.
    const point *view_space_points[4];

    //the number of points used.
    int32 num_points;

    //the screen coordinates of the points.
    screen_coordinate *screen_coordinates[4];

    //the model
    const dk_model *model;

    //the frame that is being drawn.
    const dk_model_frame_decompressed *frame;

    //the triangle being drawn.
    const dk_model_triangle *triangle;
} model_triangle_draw_params;

void GlideDrawModelPolygon(model_triangle_draw_params &triangle);
