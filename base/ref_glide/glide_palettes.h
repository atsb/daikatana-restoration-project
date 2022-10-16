#pragma once

typedef enum {
    GP_NONE,
    GP_INTERFACE,
    GP_GAME,
    GP_SKY,
} glide_palette;

//call to specify one of the palettes to use during 3-d drawing.
void PaletteUse(glide_palette palette);



