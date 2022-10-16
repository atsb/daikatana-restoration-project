#define __MSC__
#include <glide.h>
#include "ref.h"

#include "glide_main.h"
#include "glide_palettes.h"
#include "glide_frame_buffer.h"


//the palette that is currently downloaded.
glide_palette current_palette = GP_NONE;

//the current interface palette.
byte palette_interface[768];
byte palette_game[768];
byte palette_sky[768];



void PaletteUse(glide_palette palette) {
    Leave2dMode();

    //check if the given palette is the same as the current one.
    if (palette == current_palette) return;

    //set the current palette flag.
    current_palette = palette;

    //decide which palette we will download.
    byte *use = NULL;
    if (current_palette == GP_INTERFACE) {
        use = palette_interface;
    }
    else if (current_palette == GP_GAME) {
        use = palette_game;
    }
    else if (current_palette == GP_SKY) {
        use = palette_sky;
    }

    if (palette == NULL) return;

    //download the palette to each TMU on the card.
    for (int32 i = 0; i < glide_config.SSTs[0].sstBoard.VoodooConfig.nTexelfx; i++) {
        grTexDownloadTable(i, GR_TEXTABLE_PALETTE, use);
    }
}

void SetInterfacePalette(const unsigned char *palette) {
    Leave2dMode();

    //check if we were given a NULL palette.
    if (palette == NULL) {
        //go back to the game palette.
        PaletteUse(GP_GAME);
        return;
    }

    //copy the data over.
    memcpy(palette_interface, palette, 768);

    //make this the current palette.
    PaletteUse(GP_NONE);
    PaletteUse(GP_INTERFACE);
}

