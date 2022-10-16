#pragma pack(push, 1)

#define BMP_SIGNATURE_WORD  0x4d42

typedef struct 
{
    unsigned short    bfType;       // signature - 'BM'
    unsigned long     bfSize;       // file size in bytes
    unsigned short    bfReserved1;  // 0
    unsigned short    bfReserved2;  // 0
    unsigned long     bfOffBits;    // offset to bitmap
} bmphd_t;

typedef struct 
{
    unsigned long     biSize;       // size of this struct
    long              biWidth;      // bmap width in pixels
    long              biHeight;     // bmap height in pixels
    unsigned short    biPlanes;     // num planes - always 1
    unsigned short    biBitCount;   // bits perpixel
    unsigned long     biCompression; // compression flag
    unsigned long     biSizeImage;   // image size in bytes
    long              biXPelsPerMeter; // horz resolution
    long              biYPelsPerMeter; // vert resolution
    unsigned long     biClrUsed;       // 0 -> color table size
    unsigned long     biClrImportant;  // important color count
} binfo_t;

typedef struct 
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
} drgb_t;

typedef struct 
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_t;

typedef struct 
{
	int            bpp;        // bits per pixel
    int            width;
    int            height;
    unsigned char *data;
    rgb_t         *palette;
} bitmap_t;

#pragma pack(pop)
