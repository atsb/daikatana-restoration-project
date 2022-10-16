#pragma once

class CBufferLock565;
class CPic;
class CPalette;

//instantiate a variable of this class at the top of a function to enable direct buffer
//writes in the scope of that function.  While any instances exist, the card will remain in 2d mode.
class CBufferLock565 {
public:
    CBufferLock565();
    ~CBufferLock565();
};

//keeps track of a global lock on the frame buffer.
void Enter2dMode();
void Leave2dMode();

//draws a pic with color 255 as transparent with no stretching.  
//To draw the entire bitmap, pass srcleft and srctop as 0,
//srcright as the width, and srcbottom as the height.
void GlideDrawPic(const CPic *bitmap, int destx, int desty, int srcleft, int srctop, int srcright, int srcbottom);

//stretches a pic, no transparent checking.
void GlideStretchPic(const CPic *bitmap, int destx, int desty, int destw, int desth);

void GlideStretchPicPortion(const CPic *bitmap, int destx, int desty, int dest_width, int dest_height, 
                            int src_left, int src_top, int src_right, int src_bottom);

