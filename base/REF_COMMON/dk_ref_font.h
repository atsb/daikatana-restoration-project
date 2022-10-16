//
// Our font class.
//

class dk_font {
public:
    dk_font();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //the name of the font.
    buffer256 name;

    //the height of each character.
    int32 height;

    //the width of each character, width of 0 means the character is not in the font.
    byte char_width[256];

    //the position of the character in the image.
    byte char_pos_x[256];
    byte char_pos_y[256];

    //the image that contains the graphics.
    void *image;

    //
    //  sorted array support
    //
    static int Compare(const dk_font **item1, const dk_font **item2);
    static int Find(const dk_font *item, const void *id);
};
