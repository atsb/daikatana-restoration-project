#define __MSC__
#include <glide.h>

#include "ref.h"
#include "dk_misc.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"

#include "dk_ref_pic.h"

#include "glide_frame_buffer.h"
#include "glide_ref_main.h"


//this funciton must be called before any rendering calls are made.
void UnlockBackBuffer();

//this function must be called before any direct writes to the screen take place.
void LockBackBuffer565();

CBufferLock565::CBufferLock565() {
    LockBackBuffer565();
}

CBufferLock565::~CBufferLock565() {
    UnlockBackBuffer();
}

//keeps track of a global lock on the frame buffer.
CBufferLock565 *global_lock = NULL;

void Enter2dMode() {
    //allocate our global lock if we dont have one already.
    if (global_lock != NULL) return;

    global_lock = new CBufferLock565();
}

void Leave2dMode() {
    delc(global_lock);
}


#define MAX_VID_HEIGHT 600
#define MAX_VID_WIDTH 800

//a count to keep track of our lock status.
int32 lock_count = 0;

//information needed to draw to the buffer once it is locked.
int32 scanline_width;
uint16 *scanline_start565[MAX_VID_HEIGHT];

//this funciton must be called before any rendering calls are made.
void UnlockBackBuffer() {
    //decrement the lock count.
    lock_count--;

    if (lock_count < 1) {
        //do the unlock.
        grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);

        lock_count = 0;
    }
}

//this function must be called before any direct writes to the screen take place.
void LockBackBuffer565() {
    //do the actual lock if we have to.
    if (lock_count < 1) {
        GrLfbInfo_t info;

        //is this a spin lock?  hehe
        while (grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT,
                  FXFALSE, &info) != FXTRUE);

        //set the lock count to 0.
        lock_count = 0;

        //get the width of each scanline in memory.
        scanline_width = info.strideInBytes / 2;

        //get the start of the first scanline.
        scanline_start565[0] = (uint16 *)info.lfbPtr;

        //get the start of the remaining scanlines.
        uint16 *next_line = scanline_start565[0] + scanline_width;
        for (int32 i = 1; i < current_video_height; i++) {
            scanline_start565[i] = next_line;
            next_line += scanline_width;
        }
    }

    //increment the lock count.
    lock_count++;
}

//stretches a pic, no transparent checking.
void GlideStretchPic(const CPic *bitmap, int destx, int desty, int dest_width, int dest_height) {
    if (bitmap == NULL) return;
    const CPalette *palette = bitmap->Palette();
    if (palette == NULL) return;

    //get the bitmap width and height.
    int bitmap_width = bitmap->Width();
    int bitmap_height = bitmap->Height();

    //determine if the bitmap is on the screen.
    if (destx >= current_video_width) return;               //off the right.
    if (desty >= current_video_height) return;              //off the bottom.
    if (destx + dest_width <= 0) return;                  //off the left.
    if (desty + dest_height <= 0) return;                 //off the top.

    //we will be drawing some pixels.  lock the back buffer.
    CBufferLock565 lock;

    //compute the starting destination x.
    int32 dest_scan_left;
    if (destx < 0) {
        //clip to the left of the screen.
        dest_scan_left = 0;
    }
    else {
        //start drawing at the given destination.
        dest_scan_left = destx;
    }

    //compute the ending destination x.
    int32 dest_scan_right;
    if (destx + dest_width > current_video_width) {
        //clip to the right of the screen.
        dest_scan_right = current_video_width - 1;
    }
    else {
        //draw all the way to the right of the bitmap.
        dest_scan_right = destx + dest_width - 1;
    }

    //compute ending destination y.
    int32 dest_scan_bottom;
    if (desty + dest_height > current_video_height) {
        //clip to the bottom of the screen.
        dest_scan_bottom = current_video_height - 1;
    }
    else {
        //draw all the way to the bottom of the bitmap.
        dest_scan_bottom = desty + dest_height - 1;
    }

    //compute our "gradient" components for vertical scan.
    int32 vert_numerator = bitmap_height;
    int32 vert_denominator = 2 * dest_height;

    //compute gradient components for horzontal scan.
    int32 horz_numerator = bitmap_width;
    int32 horz_denominator = 2 * dest_width;

    //precomputed source pixel indices for horizontal scan.
    static int32 line_source_pixel[MAX_VID_WIDTH];

    //compute source pixel indices for horizontal scans
    int32 cur_horz_numerator = horz_numerator;
    int32 horz_source_pixel = 0;
    //loop over all destination pixels.  we start at destx instead of dest_scan_left so that
    //if destx is off the side of the screen, we currect behaviour at the edge of the screen.
    for (int32 x = destx; x <= dest_scan_right; x++) {
        //loop until our numerator is less than our denominator
        while (cur_horz_numerator > horz_denominator) {
            //go to next pixel.
            horz_source_pixel++;

            //decrement our numerator.
            cur_horz_numerator -= horz_denominator;
        }

        //save the pixel index if the current destination is on the screen.
        if (x >= 0) {
            line_source_pixel[x] = horz_source_pixel;
        }

        //increment our numerator for the next destination pixel.
        cur_horz_numerator += 2 * horz_numerator;
    }

    //compute source scanline indices for each destination scanline.
    int32 cur_vert_numerator = vert_numerator;
    int32 vert_source_scanline = 0;
    //loop down each destination scanline.
    for (int32 y = desty; y <= dest_scan_bottom; y++) {
        //loop until our numerator is less than our denominator
        while (cur_vert_numerator > vert_denominator) {
            //go to the next pixel.
            vert_source_scanline++;

            //decrement our numerator.
            cur_vert_numerator -= vert_denominator;
        }

        //draw the scanline, if it is on the screen.
        if (y >= 0) {
            //get a pointer to the source bits scanline.
            const byte *src = bitmap->Data() + bitmap_width * vert_source_scanline;
            //get a pointer to the destination scanline.
            uint16 *dest = scanline_start565[y];

            //step through all the horizontal pixels we need to draw.
            for (int32 x = dest_scan_left; x <= dest_scan_right; x++) {
                //get the source pixel.
                byte pixel = src[line_source_pixel[x]];

                //draw the pixel.
                dest[x] = palette->colors[pixel].color16;
            }
        }

        //increment our numerator for the next destination scanline.
        cur_vert_numerator += 2 * vert_numerator;
    }
}

void GlideStretchPicPortion(const CPic *bitmap, int destx, int desty, int dest_width, int dest_height, 
    int src_left, int src_top, int src_right, int src_bottom)
{
    if (dest_width < 1) return;
    if (dest_height < 1) return;
    if (bitmap == NULL) return;
    if (src_left >= src_right) return;
    if (src_top >= src_bottom) return;
    const CPalette *palette = bitmap->Palette();
    if (palette == NULL) return;

    //get the bitmap width and height.
    int bitmap_width = bitmap->Width();
    int bitmap_height = bitmap->Height();

    if (src_left < 0) return;
    if (src_top < 0) return;
    if (src_right > bitmap_width) return;
    if (src_bottom > bitmap_height) return;

    //compute the source width and height. (the source is is rectangle of the bitmap we will actually draw)
    int source_width = src_right - src_left;
    int source_height = src_bottom - src_top;

    //determine if the bitmap is on the screen.
    if (destx >= current_video_width) return;               //off the right.
    if (desty >= current_video_height) return;              //off the bottom.
    if (destx + dest_width <= 0) return;                  //off the left.
    if (desty + dest_height <= 0) return;                 //off the top.

    //we will be drawing some pixels.  lock the back buffer.
    CBufferLock565 lock;

    //clip the destination top and bottom scan to the screen.
    int32 dest_scan_top = desty;
    if (dest_scan_top < 0) {
        dest_scan_top = 0;
    }

    int32 dest_scan_bottom = desty + dest_height - 1;
    if (dest_scan_bottom >= current_video_height) {
        dest_scan_bottom = current_video_height - 1;
    }

    //clip the destination left and right to the screen.
    int32 dest_scan_left = destx;
    if (dest_scan_left < 0) {
        dest_scan_left = 0;
    }

    int32 dest_scan_right = destx + dest_width - 1;
    if (dest_scan_right >= current_video_width) {
        dest_scan_right = current_video_width - 1;
    }

    //compute our "gradient" components for vertical scan.
    int32 vert_numerator = source_height;
    int32 vert_denominator = 2 * dest_height;

    //compute gradient components for horzontal scan.
    int32 horz_numerator = source_width;
    int32 horz_denominator = 2 * dest_width;

    //precomputed source pixel indices for horizontal scan.
    static int32 line_source_pixel[MAX_VID_WIDTH];

    //compute source pixel indices for horizontal scans
    int32 cur_horz_numerator = horz_numerator;
    int32 horz_source_pixel = src_left;
    //loop over all destination pixels.  we start at destx instead of dest_scan_left so that
    //if destx is off the side of the screen, we correct behaviour at the edge of the screen.
    for (int32 x = destx; x <= dest_scan_right; x++) {
        //loop until our numerator is less than our denominator
        while (cur_horz_numerator > horz_denominator) {
            //go to next pixel.
            horz_source_pixel++;

            //decrement our numerator.
            cur_horz_numerator -= horz_denominator;
        }

        //save the pixel index if the current destination is on the screen.
        if (x >= 0) {
            line_source_pixel[x] = horz_source_pixel;
        }

        //increment our numerator for the next destination pixel.
        cur_horz_numerator += 2 * horz_numerator;
    }

    //draw each scanline.  we do a similar loop over vertical scans
    //as we did above across the horizontal scans.
    int32 cur_vert_numerator = vert_numerator;
    int32 vert_source_scanline = src_top;
    for (int32 y = desty; y <= dest_scan_bottom; y++) {
        //loop until our numerator is less than our denominator
        while (cur_vert_numerator > vert_denominator) {
            //go to the next pixel.
            vert_source_scanline++;

            //decrement our numerator.
            cur_vert_numerator -= vert_denominator;
        }

        //draw the scanline, if it is on the screen.
        if (y >= 0) {
            //get a pointer to the source bits scanline.
            const byte *src = bitmap->Data() + bitmap_width * vert_source_scanline;
            //get a pointer to the destination scanline.
            uint16 *dest = scanline_start565[y];

            //step through all the horizontal pixels we need to draw.
            for (int32 x = dest_scan_left; x <= dest_scan_right; x++) {
                //get the source pixel.
                byte pixel = src[line_source_pixel[x]];

                //check for transparency.
                if (pixel == 255) continue;

                //draw the pixel.
                dest[x] = palette->colors[pixel].color16;
            }
        }

        //increment our numerator for the next destination scanline.
        cur_vert_numerator += 2 * vert_numerator;
    }
}

/*    
    if (bitmap == NULL) return;
    const CPalette *palette = bitmap->Palette();
    if (palette == NULL) return;

    //get the bitmap width and height.
    int bitmap_width = bitmap->Width();
    int bitmap_height = bitmap->Height();

    //determine if the bitmap is on the screen.
    if (destx >= current_video_width) return;               //off the right.
    if (desty >= current_video_height) return;              //off the bottom.
    if (destx + dest_width <= 0) return;                  //off the left.
    if (desty + dest_height <= 0) return;                 //off the top.

    //we will be drawing some pixels.  lock the back buffer.
    CBufferLock565 lock;

    //compute the starting destination x.
    int32 dest_scan_left;
    if (destx < 0) {
        //clip to the left of the screen.
        dest_scan_left = 0;
    }
    else {
        //start drawing at the given destination.
        dest_scan_left = destx;
    }

    //compute the ending destination x.
    int32 dest_scan_right;
    if (destx + dest_width > current_video_width) {
        //clip to the right of the screen.
        dest_scan_right = current_video_width - 1;
    }
    else {
        //draw all the way to the right of the bitmap.
        dest_scan_right = destx + dest_width - 1;
    }

    //compute ending destination y.
    int32 dest_scan_bottom;
    if (desty + dest_height > current_video_height) {
        //clip to the bottom of the screen.
        dest_scan_bottom = current_video_height - 1;
    }
    else {
        //draw all the way to the bottom of the bitmap.
        dest_scan_bottom = desty + dest_height - 1;
    }

    //compute our "gradient" components for vertical scan.
    int32 vert_numerator = bitmap_height;
    int32 vert_denominator = 2 * dest_height;

    //compute gradient components for horzontal scan.
    int32 horz_numerator = bitmap_width;
    int32 horz_denominator = 2 * dest_width;

    //precomputed source pixel indices for horizontal scan.
    static int32 line_source_pixel[MAX_VID_WIDTH];

    //compute source pixel indices for horizontal scans
    int32 cur_horz_numerator = horz_numerator;
    int32 horz_source_pixel = 0;
    //loop over all destination pixels.  we start at destx instead of dest_scan_left so that
    //if destx is off the side of the screen, we currect behaviour at the edge of the screen.
    for (int32 x = destx; x <= dest_scan_right; x++) {
        //loop until our numerator is less than our denominator
        while (cur_horz_numerator > horz_denominator) {
            //go to next pixel.
            horz_source_pixel++;

            //decrement our numerator.
            cur_horz_numerator -= horz_denominator;
        }

        //save the pixel index if the current destination is on the screen.
        if (x >= 0) {
            line_source_pixel[x] = horz_source_pixel;
        }

        //increment our numerator for the next destination pixel.
        cur_horz_numerator += 2 * horz_numerator;
    }

    //compute source scanline indices for each destination scanline.
    int32 cur_vert_numerator = vert_numerator;
    int32 vert_source_scanline = 0;
    //loop down each destination scanline.
    for (int32 y = desty; y <= dest_scan_bottom; y++) {
        //loop until our numerator is less than our denominator
        while (cur_vert_numerator > vert_denominator) {
            //go to the next pixel.
            vert_source_scanline++;

            //decrement our numerator.
            cur_vert_numerator -= vert_denominator;
        }

        //draw the scanline, if it is on the screen.
        if (y >= 0) {
            //get a pointer to the source bits scanline.
            const byte *src = bitmap->Data() + bitmap_width * vert_source_scanline;
            //get a pointer to the destination scanline.
            uint16 *dest = scanline_start565[y];

            //step through all the horizontal pixels we need to draw.
            for (int32 x = dest_scan_left; x <= dest_scan_right; x++) {
                //get the source pixel.
                byte pixel = src[line_source_pixel[x]];

                //draw the pixel.
                dest[x] = palette->colors[pixel].color16;
            }
        }

        //increment our numerator for the next destination scanline.
        cur_vert_numerator += 2 * vert_numerator;
    }
    */

//draws a pic with color 255 as transparent with no stretching.  
//To draw the entire bitmap, pass src_left and src_top as 0,
//src_right as the width, and src_bottom as the height.
void GlideDrawPic(const CPic *bitmap, int destx, int desty, int src_left, int src_top, int src_right, int src_bottom) {
    if (bitmap == NULL) return;
    if (src_left >= src_right) return;
    if (src_top >= src_bottom) return;
    const CPalette *palette = bitmap->Palette();
    if (palette == NULL) return;

    //get the bitmap width and height.
    int bitmap_width = bitmap->Width();
    int bitmap_height = bitmap->Height();

    if (src_left < 0) return;
    if (src_top < 0) return;
    if (src_right > bitmap_width) return;
    if (src_bottom > bitmap_height) return;

    //compute the source width and height. (the source is is rectangle of the bitmap we will actually draw)
    int source_width = src_right - src_left;
    int source_height = src_bottom - src_top;

    //determine if the bitmap is on the screen.
    if (destx >= current_video_width) return;               //off the right.
    if (desty >= current_video_height) return;              //off the bottom.
    if (destx + source_width <= 0) return;                  //off the left.
    if (desty + source_height <= 0) return;                 //off the top.

    //we will be drawing some pixels.  lock the back buffer.
    CBufferLock565 lock;

    //get the top scanline on the bitmap that we will draw, and where we will draw it.
    int32 bmp_scan_top;
    int32 dest_scan_top;
    if (desty < 0) {
        //the top of the source window is off the top of the screen.
        //we start on the first line that is visible on the screen.
        bmp_scan_top = -desty + src_top;
        dest_scan_top = 0;
    }
    else {
        //the top of the source window is on the screen.
        bmp_scan_top = src_top;
        dest_scan_top = desty;
    }

    //get the ending scanline on the bitmap that we will draw, and where we will draw it.
    int32 bmp_scan_bottom;
    int32 dest_scan_bottom;
    if (current_video_height - desty < source_height) {
        //the bottom of the source window is off the bottom of the screen.
        //(current_video_height - desty) is the number of scanlines that we will draw
        bmp_scan_bottom = src_top + (current_video_height - desty) - 1;
        dest_scan_bottom = current_video_height - 1;
    }
    else {
        //the bottom of the source window is on the screen.
        bmp_scan_bottom = src_bottom - 1;
        dest_scan_bottom = desty + source_height - 1;
    }

    //get the starting position to draw on each bitmap scanline.
    int32 bmp_scan_left;
    int32 dest_scan_left;
    if (destx < 0) {
        //the left of the source window is off the left of the screen.
        //we start on the first pixel that is on the screen.
        bmp_scan_left = -destx + src_left;
        dest_scan_left = 0;
    }
    else {
        //the left of the source window is on the screen.
        bmp_scan_left = src_left;
        dest_scan_left = destx;
    }

    //get the ending position to draw on each bitmap scanline.
    int32 bmp_scan_right;
    int32 dest_scan_right;
    if (current_video_width - destx < source_width) {
        //the right of the source window is off the right of the screen.
        //(current_video_width - destx) is the number of scanlines that we will draw.
        bmp_scan_right = src_left + (current_video_width - destx) - 1;
        dest_scan_right = current_video_width - 1;
    }
    else {
        //the right of the source window is on the screen.
        bmp_scan_right = src_right - 1;
        dest_scan_right = destx + source_width - 1;
    }

    //error check.
    if (bmp_scan_right - bmp_scan_left != dest_scan_right - dest_scan_left) return;
    if (bmp_scan_bottom - bmp_scan_top != dest_scan_bottom - dest_scan_top) return;

    //do the actual copy.  
    {
        int32 cur_src_scan = bmp_scan_top;
        int32 cur_dest_scan = dest_scan_top;

        //a pointer to the beginning of the source scanline.
        const byte *src = bitmap->Data() + bitmap_width * cur_src_scan;

        for (;cur_dest_scan <= dest_scan_bottom;) {
            //draw the current scan.
            int32 cur_src_pixel = bmp_scan_left;
            int32 cur_dest_pixel = dest_scan_left;
            uint16 *dest = scanline_start565[cur_dest_scan];
            byte pixel;

            for (;cur_dest_pixel <= dest_scan_right;) {
                //get the pixel.
                pixel = src[cur_src_pixel];
                //check if the pixel is not transparent.
                if (pixel != 255) {
                    //draw the pixel.
                    dest[cur_dest_pixel] = palette->colors[pixel].color16;
                }

                //increment our current pixel.
                cur_src_pixel++;
                cur_dest_pixel++;
            }

            //increment to the next scan.
            cur_src_scan++;
            cur_dest_scan++;

            //get the new source pointer.
            src += bitmap_width;
        }
    }
}

