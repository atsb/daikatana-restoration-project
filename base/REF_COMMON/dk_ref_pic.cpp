#include "ref.h"
#include "dk_misc.h"

#include "dk_object_reference.h"
#include "dk_shared.h"
#include "dk_ref_common.h"
#include "dk_buffer.h"
#include "dk_pointer.h"
#include "dk_misc.h"
#include "dk_array.h"
#include "dk_ref_common.h"
#include "dk_list.h"

#include "dk_ref_pic.h"

void rgb::Set(byte r, byte g, byte b) {
    this->r = r;
    this->g = g;
    this->b = b;

    color16 = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);

	color32 = (255 << 24) + (r << 0) + (g << 8) + (b << 16);

}

CPalette::CPalette() {

}

CPalette::~CPalette() {

}

//takes 768 bytes of r-g-b triplets.
void CPalette::Init(const byte *raw_data) {
    //set each palette index.
    for (int32 i = 0; i < 256; i++) {
        int32 raw_offset = i * 3;
        colors[i].Set(raw_data[raw_offset], raw_data[raw_offset + 1], raw_data[raw_offset + 2]);
    }
}

CPic::CPic() {
    width = height = 0;
    data = NULL;
    resource_flag = RESOURCE_GLOBAL;
}

CPic::~CPic() {
    delca(data);
}

int CPic::Compare(const CPic **item1, const CPic **item2) {
    return stricmp((*item1)->FileName(), (*item2)->FileName());
}

int CPic::Find(const CPic *item, const void *id) {
    //the id is a character string.
    const char *name = (const char *)id;

    //check if the pic's name is equal to the passed in name.
    return stricmp(name, item->FileName());
}

const char *CPic::FileName() const {
    return filename;
}

const byte *CPic::Data() const {
    return data;
}

int CPic::Height() const {
    return height;
}

int CPic::Width() const {
    return width;
}

const CPalette *CPic::Palette() const {
    return palette;
}

resource_t CPic::Flag() const {
    return resource_flag;
}

void CPic::Flag(resource_t resource_flag) {
    this->resource_flag = resource_flag;
}


typedef enum {
    PFT_NONE,

    PFT_BMP,
    PFT_PCX,
    PFT_TARGA,
} pic_file_type;

pic_file_type PicFileType(const char *name) {
    if (name == NULL || name[0] == '\0') return PFT_NONE;

    //separate the name into its component parts.
    char filename[_MAX_FNAME];
    char extension[_MAX_EXT];

    _splitpath(name, NULL, NULL, filename, extension);

    //look at the extension.
    if (stricmp(extension, ".pcx") == 0) return PFT_PCX;
    if (stricmp(extension, ".bmp") == 0) return PFT_BMP;
    if (stricmp(extension, ".tga") == 0) return PFT_TARGA;

    return PFT_NONE;
}

void PicPathName(const char *name, buffer256 &filename) {
    //if the name does not have a slash at the front, we assume
    //it is in the pics directory and has a pcx extension.
    if (name[0] != '\\' && name[0] != '/') {
		if( strstr( name, ".pcx" ) )
		{
			//add the pics/ at front and the .pcx at the end.
			filename.Set("pics/%s", name);
		}
		else
		{
			//add the pics/ at front and the .pcx at the end.
			filename.Set("pics/%s.pcx", name);
		}
//        //add the pics/ at front and the .pcx at the end.
//        filename.Set("/pics/%s.pcx", name);
    }
    else {
        //assume the given name has the path and extension included.
        filename = name;
    }
}

//our array of loaded pics.
array<CPic> loaded_pics;

const CPic *LoadPic(const char *name, resource_t resource_flag) {
    if (name == NULL || name[0] == '\0') return NULL;

    //get name of the file on disk, including full path and extension.
    buffer256 filename;
    PicPathName(name, filename);

    //search for the filename in our array of already-loaded pics.
    CPic *pic = loaded_pics.Item((const char *)filename, CPic::Find);

    if (pic != NULL) {
        //the pic is loaded.
        //set the resource type
        pic->Flag(resource_flag);
        //return the pic.
        return pic;
    }

    //we must try to search for the file on disk.
    //get the type of the file. 
    pic_file_type file_type = PicFileType(filename);

    //load the file.
    switch (file_type) {
      case PFT_PCX:
        //load the pcx file.
        pic = CPic::LoadPCX(filename);
        break;
      case PFT_BMP:
        //load the bitmap file
        pic = CPic::LoadBMP(filename);
        break;
      default:
        return NULL;
    }

    if (pic == NULL) return NULL;

    //put the loaded pic into our array for next time.
    loaded_pics.Add(pic, CPic::Compare);

    //set the resource type
    pic->Flag(resource_flag);

    return pic;
}

//our list of palettes.
static list_pointer<CPalette> loaded_palettes;

CPalette *GetSharedPalette(const byte *raw_palette) {
    if (raw_palette == NULL) return NULL;

    //go through the loaded palettes.
    list_element<pointer<CPalette> > *element = loaded_palettes.First();
    while (element != NULL) {
        //get the palette.
        CPalette *palette = element->Item();
        if (palette != NULL) {
            //compare the elements of the given raw palette with the loaded palette.
            //we compare element 0, 255, 1, 254, 2, 253, ... so we can quickly determine
            //non-identical palettes even if they share a common set of values at the beginning
            for (int32 i = 0; i < 128; i++) {
                //check element i.
                int32 raw_offset = i * 3;
                if (raw_palette[raw_offset + 0] != palette->colors[i].r) break;
                if (raw_palette[raw_offset + 1] != palette->colors[i].g) break;
                if (raw_palette[raw_offset + 2] != palette->colors[i].b) break;

                //check element 255 - i.
                raw_offset = (255 - i) * 3;
                if (raw_palette[raw_offset + 0] != palette->colors[255 - i].r) break;
                if (raw_palette[raw_offset + 1] != palette->colors[255 - i].g) break;
                if (raw_palette[raw_offset + 2] != palette->colors[255 - i].b) break;
            }

            //check if we compared all elements successfully.
            if (i == 128) {
                //the palettes are identical.
                return palette;
            }
        }

        //look at the next palette in the list.
        element = element->Next();
    }

    //make a new palette with the given data.
    CPalette *palette = new CPalette();
    palette->Init(raw_palette);

    //add the palette to the list.
    loaded_palettes.Add(palette);

    return palette;
}

CPic *CPic::LoadBMP(const char *filename) {
    if (filename == NULL || filename[0] == '\0') return NULL;

    //call our common pcx loader
    int width;
    int height;
    byte *data = NULL;
    byte *raw_palette = NULL;

    ::LoadBMP(filename, &data, &raw_palette, &width, &height);

    //check if we got the data successfully.
    if (data == NULL) return NULL;

    //make a pic object to store the data.
    CPic *pic = new CPic();

    //put the data into the pic.
    pic->width = width;
    pic->height = height;
    pic->data = data;
    
    //get a palette from the raw loaded palette data.
    CPalette *palette = GetSharedPalette(raw_palette);

    //delete the palette data returned by ::LoadPCX
    delete raw_palette;

    if (palette == NULL) {
        delete pic;
        return NULL;
    }

    //set the pic filename.
    pic->filename = filename;

    //put the palette into the image.
    pic->palette = palette;

    return pic;
}

CPic *CPic::LoadPCX(const char *filename) {
    if (filename == NULL || filename[0] == '\0') return NULL;

    //call our common pcx loader
    int width;
    int height;
    byte *data = NULL;
    byte *raw_palette = NULL;

    ::LoadPCX(filename, &data, &raw_palette, &width, &height);

    //check if we got the data successfully.
    if (data == NULL) return NULL;

    //make a pic object to store the data.
    CPic *pic = new CPic();

    //put the data into the pic.
    pic->width = width;
    pic->height = height;
    pic->data = data;
    
    //get a palette from the raw loaded palette data.
    CPalette *palette = GetSharedPalette(raw_palette);

    //delete the palette data returned by ::LoadPCX
    delete raw_palette;

    if (palette == NULL) {
        delete pic;
        return NULL;
    }

    //set the pic filename.
    pic->filename = filename;

    //put the palette into the image.
    pic->palette = palette;

    return pic;
}






