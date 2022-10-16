#define __MSC__
#include <glide.h>
#include <stdlib.h>
#include "ref.h"

#include "dk_shared.h"
#include "dk_point.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_model_new.h"

#include "glide_draw.h"

void GlideClearBuffer() {
    grBufferClear(0, 0, GR_WDEPTHVALUE_FARTHEST); 
}

void GlideFlip(int refresh_waits) {
    grBufferSwap(refresh_waits);
}

void DrawChar(int x, int y, int c);

void GlideDrawString(int x, int y, const char *str) {
    //draw each character.
    for (int pos = 0; str[pos] != '\0'; pos++) {
        DrawChar(x, y, str[pos]);

        x += 8;
    }
}

void GlideDrawModelPolygon(model_triangle_draw_params &triangle) {
    //our array of glide point structures.
    static GrVertex verts[4];

    //copy the vertex data into the vertex structure.
    for (int32 i = 0; i < triangle.num_points; i++) {
        //copy screen coordinates.
        verts[i].x = triangle.screen_coordinates[i]->x;
        verts[i].y = triangle.screen_coordinates[i]->y;
        verts[i].oow = triangle.screen_coordinates[i]->oow;

        //get the flat color of each vertex based on the vertex normals.
        float intensity = triangle.screen_coordinates[i]->light_intensity;
        verts[i].r = byte(127 * intensity);
        verts[i].g = byte(127 * intensity);
        verts[i].b = byte(127 * intensity);
        
    }

    grDrawPolygonVertexList(triangle.num_points, verts);
}



/*

    //get the handle.
    handle = texture->Handle();

    //get the scale used to compute texture coords.
    scale = 256.0f / texture->LargestDim();

    //make sure the texture is loaded into memory.
    texture_data_address = LoadIntoMemory(handle, texture_info);

    //fill in our vertex list that we will pass to glide.
    for (int32 i = 0; i < num_render_pts; i++ ) {
        GrVerts[i].x = render_pts[i]->sx;
        GrVerts[i].y = render_pts[i]->sy;
        //oow should be in the range 1/1 to 1/65535
        GrVerts[i].oow = (1.0f / (1 + render_pts[i]->z * 4));

        truncate_to_16th_fast(&GrVerts[i].x);
        truncate_to_16th_fast(&GrVerts[i].y);

        //fill in the oow in the vertex struct.
        GrVerts[i].tmuvtx[0].oow = GrVerts[i].oow;

        //compute texture coordinates.
        GrVerts[i].tmuvtx[0].sow = render_pts[i]->texture.u * scale * GrVerts[i].oow;
        GrVerts[i].tmuvtx[0].tow = render_pts[i]->texture.v * scale * GrVerts[i].oow;
    }

    grTexSource(GR_TMU0, texture_data_address, GR_MIPMAPLEVELMASK_BOTH, texture_info);

    grDrawPolygonVertexList(num_render_pts, GrVerts);
*/





