#define __MSC__
#include <glide.h>
#include "ref.h"

#include "dk_point.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_model_new.h"
#include "dk_matrix.h"

#include "glide_frustrum.h"
#include "glide_viewport.h"
#include "glide_draw.h"
#include "glide_frame_buffer.h"

extern point model_vertex_normals[];
extern float vertex_normal_dots[16][256];
extern float *shadedots;

void RenderScene(const refdef_t *rd);

void RenderFrame(refdef_t *rd) {
    if (rd == NULL) return;
    //compute the field of view width and height in radians.
    float width_radians = to_radians(rd->fov_x);
    float height_radians = to_radians(rd->fov_y);

    //set up the view frustrum based on the given field of view.
    point frustrum_origin(rd->vieworg);
    point frustrum_angles_degrees(rd->viewangles);
    FrustrumSetup(width_radians, height_radians, frustrum_angles_degrees, frustrum_origin);

    //set up the viewport.
    ViewportSetup(rd->width, rd->height, rd->x, rd->y, width_radians, height_radians);

    //render the 3d objects.
    RenderScene(rd);
}

void RenderEntities(int32 num_entities, const entity_t *entities);

void RenderScene(const refdef_t *rd) {
    //leave 2d drawing mode, if we are currently in it.
    Leave2dMode();

    //draw the entities.
    RenderEntities(rd->num_entities, rd->entities);
}

void RenderModelFrame(const dk_model *model, const dk_model_frame_decompressed *frame, const matrix &object_to_world);

void RenderEntities(int32 num_entities, const entity_t *entities) {
    //draw each entity.
    for (int32 i = 0; i < num_entities; i++) {
        //get the entity.
        const entity_t *entity = &entities[i];

        //get the model.
        const dk_model *model = (const dk_model *)entity->model;
        if (model == NULL) continue;

        //
        //set up the transformation matrix from the entity's object space to world space.
        //

        //make the scale matrix.
        matrix matrix_scale;
        point scale(entity->render_scale);
        matrix_scale.MakeScale(scale);

        //make the rotation matrix. 
        matrix matrix_rotation;
        point entity_angles_radians(entity->angles);
        entity_angles_radians.y *= -1;
        entity_angles_radians.mul(DEG2RAD);
        matrix_rotation.MakeRotation(entity_angles_radians);

        //make the translation matrix.
        matrix matrix_translation;
        point entity_origin(entity->origin);
        matrix_translation.MakeTranslation(entity_origin);

        //combine the 3 matrices, translation * rotation * scale.
        matrix rot_scale;
        rot_scale.mul(matrix_rotation, matrix_scale);

        matrix object_to_world;
        object_to_world.mul(matrix_translation, rot_scale);

        //setup the correct precomputed light normal dot product array.
        shadedots = vertex_normal_dots[int(entity->angles[1] * 0.0444f) & 15];

        //
        //get the decompressed frame data from the model.
        //

        const dk_model_frame_decompressed *decompressed_frame = NULL;

        //check if we are lerping frames.
        if (entity->backlerp > 0.0f) {
            //get the interpolated frame between the old and current frames.
            decompressed_frame = model->GetInterpolatedFrame(entity->oldframe, entity->frame, 1.0f - entity->backlerp);
        }
        else {
            //just grab the current frame.
            decompressed_frame = model->GetDecompressedFrame(entity->frame);
        }

        if (decompressed_frame == NULL) continue;

        //draw the frame.
        RenderModelFrame(model, decompressed_frame, object_to_world);
    }
}

void RenderModelTriangle(model_triangle_draw_params &triangle);

//
//  This function rounds numbers to the nearest 1/16th, which needs to be done for glide.
//  It assumes that the floating point unit is in 23 bit precision mode.
//
#define snapper float( 3L << 18 )
__inline void truncate_to_16th_fp(float &num) {
    num += snapper;
    num -= snapper;
}

//this macro converts from 1/z to 1/w.
//w should be in the range 1..65535.
//our z value goes out to a maximum of 8K
#define compute_oow(ooz) ((ooz) * 0.125f)

void RenderModelFrame(const dk_model *model, const dk_model_frame_decompressed *frame, const matrix &object_to_world) {
    if (model == NULL) return;
    if (frame == NULL) return;
    if (model->NumPoints() >= 2000) return;
    if (frame->SourceFrame() == NULL) return;

    //a static array of points we use to transform all the points into view space from model space.
    static point view_points[2000];
    //a static array of the screen coordinates derived from the above view points.
    screen_coordinate screen_coordinates[2000];

    //get the number of points in the frame.
    int32 num_points = model->NumPoints();

    //get the array of points from the frame.
    const array_obj<point> &frame_points = frame->Points();

    //get the array of vertex normals for this frame.
    const array_obj<byte> &vertex_normals = frame->SourceFrame()->PointNormals();

    //get the current world to view transformation matrix.
    const matrix &world_to_view = WorldToView();

    //construct the object to view space transform.
    static matrix object_to_view;
    object_to_view.mul(world_to_view, object_to_world);

    //
    //  Process the points of the model.
    //

    for (int32 i = 0; i < num_points; i++) {
        //transform the points of the model to view space.
        view_points[i].mul(object_to_view, frame_points.Item(i));

        //
        //compute the screen coordinate for the point.
        //

        //get the inverse of z.
        float ooz = screen_coordinates[i].ooz = 1.0f / view_points[i].z;

        //set 1/w
        screen_coordinates[i].oow = compute_oow(ooz);

        //divide x and y by z.
        screen_coordinates[i].x = ooz * view_points[i].x;
        screen_coordinates[i].y = ooz * view_points[i].y;

        //scale and offset x and y to get real screen coordinates.
        screen_coordinates[i].x = screen_coordinates[i].x * to_screen_scale_x + to_screen_offset_x;
        screen_coordinates[i].y = screen_coordinates[i].y * to_screen_scale_y + to_screen_offset_y;

        //round the values to 1/16th of a screen pixel.
        truncate_to_16th_fp(screen_coordinates[i].x);
        truncate_to_16th_fp(screen_coordinates[i].y);

        //compute light intensities for this point
        screen_coordinates[i].light_intensity = shadedots[vertex_normals[i]];
    }

    //get the array of triangles.
    const array_obj<dk_model_triangle> &triangles = model->Triangles();

    //get the number of triangles.
    int32 num_triangles = triangles.Num();

    //a structure with the parameters needed to draw the model triangles.
    model_triangle_draw_params draw_tri;
    draw_tri.model = model;
    draw_tri.frame = frame;

    //go through each triangle in the model.
    for (i = 0; i < num_triangles; i++) {
        //get the triangle.
        draw_tri.triangle = &triangles[i];

        //set the number of points.
        draw_tri.num_points = 3;

        //get the points that are being drawn.
        draw_tri.view_space_points[0] = &view_points[draw_tri.triangle->Point0()];
        draw_tri.view_space_points[1] = &view_points[draw_tri.triangle->Point1()];
        draw_tri.view_space_points[2] = &view_points[draw_tri.triangle->Point2()];

        draw_tri.screen_coordinates[0] = &screen_coordinates[draw_tri.triangle->Point0()];
        draw_tri.screen_coordinates[1] = &screen_coordinates[draw_tri.triangle->Point1()];
        draw_tri.screen_coordinates[2] = &screen_coordinates[draw_tri.triangle->Point2()];

        //draw the triangle.
        RenderModelTriangle(draw_tri);
    }
}

bool ClipModelTriangle(model_triangle_draw_params &triangle);

void RenderModelTriangle(model_triangle_draw_params &triangle) {
    //clip the poly to the near clip plane.
    if (ClipModelTriangle(triangle) == false) return;

    //draw the triangle.
    GlideDrawModelPolygon(triangle);
}

bool ClipModelTriangle(model_triangle_draw_params &triangle) {
    return true;
}







