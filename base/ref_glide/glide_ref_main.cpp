#define __MSC__
#include <glide.h>
#include "ref.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_point.h"

#include "dk_ref_pic.h"
#include "dk_ref_common.h"

#include "dk_model_new.h"

#include "glide_draw.h"
#include "glide_ref_main.h"
#include "glide_frame_buffer.h"
#include "glide_frame_rate.h"
#include "glide_model.h"

//current size of the screen.
int32 current_video_width = 800;
int32 current_video_height = 600;

//our cvars.
cvar_t *r_resourcedir = NULL;
cvar_t *glide_rates = NULL;
cvar_t *glide_clear_buffer = NULL;

//global resources.
const CPic *pic_characters = NULL;
//a void pointer to the base character pic, for use by dk_ref_font
void *default_chars_image = NULL;

bool InitRefresh() {
    //get our cvars.
    r_resourcedir = ri.Cvar_Get("r_resourcedir", "", 0);  
    glide_rates = ri.Cvar_Get("glide_rates", "0", CVAR_ARCHIVE);
    glide_clear_buffer = ri.Cvar_Get("glide_clear_buffer", "0", CVAR_ARCHIVE);

    //make sure we set the resource directory and load the character set.
    SetResourceDir(r_resourcedir->string);

    return true;
}

void ShudownRefresh() {
    //delete all of our loaded pics.
}

void SetResourceDir(char *name) {
    if (name == NULL) return;

    //save the given path into a cvar.
    ri.Cvar_Set("r_resourcedir", name);

    //load the character pic.
    if (r_resourcedir == NULL || r_resourcedir->string == NULL || r_resourcedir->string[0] == '\0') {
        //get the character file out of the default directory.
        pic_characters = LoadPic("dkchars", RESOURCE_GLOBAL);
        default_chars_image = (void *)pic_characters;
    }
    else {
        //make the full path name of the character file.
        buffer256 path("%sdkchars.pcx", r_resourcedir->string);

        //load the pic.
        pic_characters = LoadPic(path, RESOURCE_GLOBAL);
        default_chars_image = (void *)pic_characters;

        //check if we got it.
        if (pic_characters == NULL) {
            //try the default directory.
            pic_characters = LoadPic("dkchars", RESOURCE_GLOBAL);
            default_chars_image = (void *)pic_characters;
        }
    }
}

void BeginFrame(float camera_separation) {
    //
    //clear the depth buffer.
    //

    //disable writes to the color buffer if we do not want to clear it.
    if (glide_clear_buffer != NULL && glide_clear_buffer->value == 0.0f) {
        grColorMask(FALSE, FALSE);
    }

    //clear the buffer.
    grBufferClear(0, 0, GR_WDEPTHVALUE_FARTHEST);

    //enable writes to the color buffer.
    grColorMask(TRUE, FALSE);
}

void EndFrame() {
    //keep track of our frame rate.
    if (glide_rates != NULL && glide_rates->value != 0.0f) {
        static frame_rate rate;
        rate.Frame();

        //draw the framerate.
        buffer16 print("%.1f", rate.Rate());
        GlideDrawString(current_video_width - 40, 200, print);
    }

    //leave 2d drawing mode, if we are currently in it.
    Leave2dMode();

    //flip the back buffer up to the front.
    GlideFlip(0);
}

void DrawTileClear(int x, int y, int w, int h, const char *name) {
    
}

void DrawConsolePic(int x, int y, int w, int h) {
    //enter 2d drawing mode, if we are not already in it.
    Enter2dMode();

    int x_start, x_width;
    float x_scale;

    //  calc where the bigger pic should start
    x_scale = (current_video_width / 320.0);
    x_width = 256 * x_scale;
    x_start = current_video_width - x_width;

    DrawStretchPic(0, y, 64 * x_scale, h, "dkcon2", RESOURCE_GLOBAL);     
    DrawStretchPic(x_start, y, x_width, h, "dkcon", RESOURCE_GLOBAL);     
}

void DrawChar(int x, int y, int c) {
    if (pic_characters == NULL) return;

    //enter 2d drawing mode, if we are not already in it.
    Enter2dMode();

    //make sure the given character is 0-255
    c &= 0x000000ff;

    //compute the row and column in the character bitmap that contains the character.
    //there are 16 characters on each row.
    int row = c >> 4;

    //compute the column.  
    int column = c & 0x0f;

    //compute the part of the character bitmap we will draw.
    int src_left = column * 8;
    int src_right = src_left + 8;
    int src_top = row * 8;
    int src_bottom = src_top + 8;

    //do the drawing.
    GlideDrawPic(pic_characters, x, y, src_left, src_top, src_right, src_bottom);
}

void DrawPic(int x, int y, const char *name, resource_t resource_flag) {
    if (name == NULL || name[0] == '\0') return;

    //get the pic.
    const CPic *pic = LoadPic(name, resource_flag);
    if (pic == NULL) return;

    //enter 2d drawing mode, if we are not already in it.
    Enter2dMode();

    //draw the pic on the screen.
    GlideDrawPic(pic, x, y, 0, 0, pic->Width(), pic->Height());
}

extern "C" void DrawPicPortion(int destx, int desty, int src_left, int src_top, int src_width, int src_height, void *font_image);
void DrawPicPortion(int destx, int desty, int src_left, int src_top, int src_width, int src_height, void *font_image) {
    Enter2dMode();

    GlideDrawPic((CPic *)font_image, destx, desty, src_left, src_top, src_left + src_width, src_top + src_height);
}

void DrawStretchPic(int x, int y, int w, int h, const char *name, resource_t resource) {
    if (name == NULL || name[0] == '\0') return;

    //get the pic.
    const CPic *pic = LoadPic(name, resource);
    if (pic == NULL) return;

    //enter 2d drawing mode, if we are not already in it.
    Enter2dMode();
    //draw the pic.
    GlideStretchPic(pic, x, y, w, h);
}

void DrawGetPicSize(int *w, int *h, const char *name, resource_t resource) {
    //set default values.
    *w = 0; 
    *h = 0;

    if (name == NULL || name[0] == '\0') return;

    //get the pic.
    const CPic *pic = LoadPic(name, resource);
    if (pic == NULL) return;

    //get the width and height.
    *w = pic->Width();
    *h = pic->Height();
}

void *RegisterPic(const char *name, resource_t resource) {
    if (name == NULL || name[0] == '\0') return NULL;

    //load the pic.
    const CPic *pic = LoadPic(name, resource);

    //return it.
    return (void *)pic;
}

void *RegisterModel(const char *name, resource_t resource) {
    if (name == NULL || name[0] == '\0') return NULL;

    //load the pic.
    const dk_model *model = LoadModel(name, resource);

    //return it.
    return (void *)model;
}

//
//functions and variable stubs needed to make dk_ref_font compile.
//

void DrawStretchPicPortion(int destx, int desty, float scale_x, float scale_y, 
    int src_left, int src_top, int src_width, int src_height, void *image)
{
    GlideStretchPicPortion((CPic *)image, destx, desty, int32(src_width * scale_x), int32(src_height * scale_y), 
        src_left, src_top, src_left + src_width, src_top + src_height);
}

void Draw_AlphaPicPortion (int destx, int desty, int src_left, int src_top, 
    int src_width, int src_height, void *image, vec3_t rgbBlend, float alpha, int flags) 
{
    DrawPicPortion(destx, desty, src_left, src_top, src_width, src_height, image);
}

void Draw_CharFlare (int x, int y, int w, int h, void *image, vec3_t rgbBlend, 
    float alpha, float scale, float theta)
{

}

void *char_flare_image = NULL;
float ref_laserRotation = 0.0f;




