#include	"r_local.h"
#include	"dk_ref_common.h"


void SwStretchPicPortion(image_t *bitmap, int destx, int desty, int dest_width, int dest_height, 
    int src_left, int src_top, int src_right, int src_bottom)
{
    if (dest_width < 1) return;
    if (dest_height < 1) return;
    if (bitmap == NULL) return;
    if (src_left >= src_right) return;
    if (src_top >= src_bottom) return;

    //get the bitmap width and height.
    int bitmap_width = bitmap->width;
    int bitmap_height = bitmap->height;

    if (src_left < 0) return;
    if (src_top < 0) return;
    if (src_right > bitmap_width) return;
    if (src_bottom > bitmap_height) return;

    //compute the source width and height. (the source is is rectangle of the bitmap we will actually draw)
    int source_width = src_right - src_left;
    int source_height = src_bottom - src_top;

    //determine if the bitmap is on the screen.
    if (destx >= vid.width) return;               //off the right.
    if (desty >= vid.height) return;              //off the bottom.
    if (destx + dest_width <= 0) return;                  //off the left.
    if (desty + dest_height <= 0) return;                 //off the top.

    //clip the destination top and bottom scan to the screen.
    int32 dest_scan_top = desty;
    if (dest_scan_top < 0) {
        dest_scan_top = 0;
    }

    int32 dest_scan_bottom = desty + dest_height - 1;
    if (dest_scan_bottom >= vid.height) {
        dest_scan_bottom = vid.height - 1;
    }

    //clip the destination left and right to the screen.
    int32 dest_scan_left = destx;
    if (dest_scan_left < 0) {
        dest_scan_left = 0;
    }

    int32 dest_scan_right = destx + dest_width - 1;
    if (dest_scan_right >= vid.width) {
        dest_scan_right = vid.width - 1;
    }

    //compute our "gradient" components for vertical scan.
    int32 vert_numerator = source_height;
    int32 vert_denominator = 2 * dest_height;

    //compute gradient components for horzontal scan.
    int32 horz_numerator = source_width;
    int32 horz_denominator = 2 * dest_width;

    //precomputed source pixel indices for horizontal scan.
    static int32 line_source_pixel[MAXWIDTH];

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
            const byte *src = bitmap->pixels[0] + bitmap_width * vert_source_scanline;
            //get a pointer to the destination scanline.
            uint16 *dest = (uint16 *)vid.buffer + y * vid.pixelwidth;
            uint16 pixel;

            //step through all the horizontal pixels we need to draw.
            for (int32 x = dest_scan_left; x <= dest_scan_right; x++) {
                //get the source pixel.
                pixel = src[line_source_pixel[x]];

                //check for transparency.
                if (pixel == 255) continue;

                //draw the pixel.
                dest[x] = bitmap->palette[pixel];
            }
        }

        //increment our numerator for the next destination scanline.
        cur_vert_numerator += 2 * vert_numerator;
    }
}


