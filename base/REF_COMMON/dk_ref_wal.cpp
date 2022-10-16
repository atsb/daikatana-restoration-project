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
#include "dk_ref_wal.h"

CWal::CWal() {
    width = height = 0;

	for( int i = 0; i < 4; i++ ) 
	    data[i] = NULL;

    resource_flag = RESOURCE_GLOBAL;
}

CWal::~CWal() {
    delca(data[0]);
}

int CWal::Compare(const CWal **item1, const CWal **item2) {
    return stricmp((*item1)->FileName(), (*item2)->FileName());
}

int CWal::Find(const CWal *item, const void *id) {
    //the id is a character string.
    const char *name = (const char *)id;

    //check if the pic's name is equal to the passed in name.
    return stricmp(name, item->FileName());
}

const char *CWal::FileName() const {
    return filename;
}

const byte *CWal::Data() const {
    return data[0];
}

const byte *CWal::Data( int miplevel ) const {
    return data[miplevel];
}

int CWal::Height() const {
    return height;
}

int CWal::Width() const {
    return width;
}

const CPalette *CWal::Palette() const {
    return palette;
}

resource_t CWal::Flag() const {
    return resource_flag;
}

void CWal::Flag(resource_t resource_flag) {
    this->resource_flag = resource_flag;
}

//our array of loaded pics.
array<CWal> loaded_wals;

CPalette *GetSharedPalette(const byte *raw_palette);

CWal *CWal::Load(const char *name)
{
	miptex_t		*mt;
	int				ofs;
	unsigned int	filelength;

	CWal *wal = new CWal;

	// load the file
	filelength = ri.FS_LoadFile( name, (void **)&mt );

	// make sure it loaded
	if( !mt )
	{
		ri.Con_Printf (PRINT_ALL, "LoadWal: can't load %s\n", name);
		return NULL;
	}

	// make sure it's the correct format .wal file (for .wal files that don't have a version)
	if(mt->version > 0x20 )
		return NULL;

	/// make sure its the correct version
	if( mt->version != MIPTEX_VERSION )
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)", name, mt->version, MIPTEX_VERSION);

	// set the width
	wal->width = mt->width;

	// set the height
	wal->height = mt->height;

    //get a palette from the raw loaded palette data.
    CPalette *palette = GetSharedPalette(mt->palette);

    if (palette == NULL) 
	{
		// delete the wal
        delete wal;

		// return null
        return NULL;
    }

    //set the pic filename.
    wal->filename = name;

    //put the palette into the wal.
    wal->palette = palette;

	// allocate data for the graphic data
	wal->data[0] = (byte *)memmgr.X_Malloc( filelength - sizeof(miptex_t), MEM_TAG_IMAGE);

	wal->data[1] = wal->data[0] + ((wal->width >> 1) * (wal->height >> 1));
	wal->data[2] = wal->data[1] + ((wal->width >> 2) * (wal->height >> 2));
	wal->data[3] = wal->data[2] + ((wal->width >> 3) * (wal->height >> 3));

	// offset of data
	ofs = (long) mt->offsets[0];

	// copy the data
	memcpy( wal->data[0], (byte *) mt + ofs, filelength - sizeof(miptex_t) );

	// free the loaded file
	ri.FS_FreeFile ((void *)mt);

	return wal;
}

const CWal *LoadWal(const char *name, resource_t resource_flag) {
	// make sure we don't try to load NULL
	if( name == NULL || name[0] == '\0' )
		return NULL;

	CWal *wal = loaded_wals.Item((const char *)name, CWal::Find);

	if( wal != NULL )
	{
		// the wall is already loaded so set the resource flag
		wal->Flag(resource_flag);

		// and return the wal
		return wal;
	}

	// wal does not exist so load it
	wal = CWal::Load( name );
	
	if( wal != NULL )
	{
		// set the resource flag
		wal->Flag( resource_flag );

		// add the wal file
		loaded_wals.Add( wal, CWal::Compare );
	}

	// return the wal
	return wal;
}

