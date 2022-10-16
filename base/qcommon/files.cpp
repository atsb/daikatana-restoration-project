#if _MSC_VER
#include "windows.h"
#include "direct.h"
#endif

#include "qcommon.h"
#include <io.h>
// define this to dissalow any data but the demo pak file
//#define	NO_ADDONS

// if a packfile directory differs from this, it is assumed to be hacked
// Full version

// this checksum is for the mplayer deathmatch demo pak0.pak
//#define	PAK0_CHECKSUM	0x10dc4890

//#define	PAK0_CHECKSUM	0x40e614e0

// Demo
//#define	PAK0_CHECKSUM	0xb2c6d7ea
// OEM
//#define	PAK0_CHECKSUM	0x78e135c

#define DEMOPAK_CHECKSUM_1	0x2d9ac254
#define DEMOPAK_CHECKSUM_2	0xf00c0633
#define DEMOPAK_CHECKSUM_3	0x7d5320be

/*
=============================================================================
QUAKE FILESYSTEM
=============================================================================
*/
#define	BASEDIRNAME	"data"

//
// in memory
//
typedef struct packfile_s
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
	int     compresslen, compresstype;

} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
char	fs_tempdir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_gamedirvar;

typedef struct filelink_s
{
	struct filelink_s	*next;
	char	*from;
	int		fromlength;
	char	*to;
} filelink_t;

filelink_t	*fs_links;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs

static int		files_opened = 0; 
static long		bytes_read = 0;

static int      compress_type   = 0;
static int      compress_length = 0;
static int      from_loadfile = 0;

/*

All of Quake's data access is through a hierchal file system, 
but the contents of the file system can be transparently merged 
from several sources.

The "base directory" is the path to the directory holding the 
quake.exe and all game directories.  The sys_* files pass this to 
host_init in quakeparms_t->basedir.  This can be overridden with 
the "-basedir" command line parm to allow code debugging in a 
different directory.  The base directory is only used during filesystem 
initialization.

The "game directory" is the first tree on the search path and directory 
that all generated files (savegames, screenshots, demos, config files) 
will be saved to.  This can be overridden with the "-game" command line 
parameter.  The game directory can never be changed while quake is executing.  
This is a precacution against having a malicious server instruct clients to 
write files over areas they shouldn't.

*/
#define INSTALLTYPE_FULL		1
#define INSTALLTYPE_STANDARD	2
#define INSTALLTYPE_MINIMUM		3

qboolean FS_IsFullInstall()
{
	HKEY	hKey;
// changed by yokoyama for Japanese version // JPN
#ifdef JPN
	#ifndef DAIKATANA_DEMO // If Product
		char	szRegistryPath[]="SOFTWARE\\EIDOS\\Daikatana";
	#else //  if DAIKATANA_DEMO
		char	szRegistryPath[]="SOFTWARE\\Eidos Interactive\\Daikatana";
	#endif
#else
		char	szRegistryPath[]="SOFTWARE\\Eidos Interactive\\Daikatana";
#endif

	char	szValueName[]="Install Type";
	int		nInstallType;
	DWORD	dwType, dwSize;

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegistryPath, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		dwSize = sizeof( nInstallType );
		if( RegQueryValueEx( hKey, szValueName, 0, &dwType, (LPBYTE) &nInstallType, &dwSize ) == ERROR_SUCCESS )
		{
			if( nInstallType != INSTALLTYPE_FULL )
			{
				RegCloseKey( hKey );
				return FALSE;
			}
		}

		RegCloseKey( hKey );
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	FS_Seek
///////////////////////////////////////////////////////////////////////////////

int	FS_Seek (FILE *f, long offset, int origin)
{
	return	fseek (f, offset, origin);
}

///////////////////////////////////////////////////////////////////////////////
//	FS_Tell
///////////////////////////////////////////////////////////////////////////////

long	FS_Tell (FILE *f)
{
	return	ftell (f);
}

///////////////////////////////////////////////////////////////////////////////
//	FS_Getc
///////////////////////////////////////////////////////////////////////////////

int	FS_Getc (FILE *f)
{
	return	getc (f);
}

int FS_Ungetc( int c, FILE *f )
{
	return ungetc( c, f );
}

char *FS_Gets( char *string, int n, FILE *f )
{
	return fgets( string, n, f );
}

///////////////////////////////////////////////////////////////////////////////
//	FS_ScanF
///////////////////////////////////////////////////////////////////////////////

/*
================
FS_filelength
================
*/
int FS_filelength (FILE *f)
{
	int		pos;
	int		end;
	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);
	return end;
}
/*
============
FS_CreatePath
Creates any directories needed to store the given filename
============
*/
void	FS_CreatePath (char *path)
{
	char	*ofs;
	
	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_Mkdir (path);
			*ofs = '/';
		}
	}
}
/*
==============
FS_FCloseFile
For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (FILE *f)
{
	fclose (f);
}
/*
===========
FS_FOpenFile
Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/

//performs a binary search through the pack file entries looking for the given file name.
int FindFileInPack(pack_t *pak, const char *filename) {
    //the boundaries of our search.
    int top, bottom;
//	int	count;

    //set the initial top and bottom.
    top = 0;
    bottom = pak->numfiles;
//	count = 0;

    //search until top and bottom get close.
    while (bottom - top > 5) {
        //the middle element.
        int middle;
        int compare;

        middle = (top + bottom) / 2;

        //check if the middle element is the one we are looking for.
        compare = stricmp(filename, pak->files[middle].name);

//		count++;

        if (compare == 0) {
            //we found our item.
//			Com_DPrintf( "File: %s found in %d searches\n", filename, count );
            return middle;
        }

        //check if it is above the middle.
        if (compare < 0) {
            //the file is above this one.
            bottom = middle - 1;
            continue;
        }
        
        //check if it is below the middle.
        if (compare > 0) {
            //the file is below this one.
            top = middle + 1;
            continue;
        }
    }

    //linear search through the remaining indices.
    for (; top <= bottom; top++) {
        if (stricmp(filename, pak->files[top].name) == 0) {
            //found it.
//			Com_DPrintf( "File: %s found in %d searches\n", filename, count );
            return top;
        }
//		count++;
    }

    //could not find the file
	Com_DPrintf( "File: %s not found\n", filename );
    return -1;
}

#ifdef DAIKATANA_DEMO
int FS_FOpenFile ( const char *filename, FILE **file)
{
    searchpath_t    *search;
    char            netpath[MAX_OSPATH];
    char            hackfilename[MAX_OSPATH];
    pack_t          *pak;
    filelink_t      *link;
    // check for links first

	compress_type   = 0;

	if( filename == NULL || filename[0] == 0 )
	{
		Com_Printf( "Could not open file: NULL\n" );
		return -1;
	}

	if( strstr( filename, ".cfg") || strstr( filename, "decoinfo") || strstr(filename, "save/") || strstr(filename, "shot.tga"))
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_ConfigDir(), filename);	// check for a new config dir (+set configdir "...")

		*file = fopen(netpath,"rb");
		if (!*file)
		{
			// didn't find the file in the config dir...try the gamedir
			Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
			*file = fopen(netpath,"rb");
			if (!*file)
			{
				// still didn't find the file.  if the file was 'daikatana.cfg' or 'default_keys.cfg', keep looking, otherwise, fail out
				if (!(!stricmp(filename,"daikatana.cfg") || !stricmp(filename,"default_keys.cfg")))
					return -1;
			}
		}

		if (*file)
		{
			files_opened++;
			return FS_filelength (*file);
		}
	}

    for (link = fs_links ; link ; link=link->next) 
    {
        if (!strncmp (filename, link->from, link->fromlength))
        {
            Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
            *file = fopen (netpath, "rb");
            if (*file)
            {       
				// track usage
				files_opened++;
                return FS_filelength (*file);
            }
            return -1;
        }
    }

    //
    // search through the path, one element at a time
    //
    for (search = fs_searchpaths ; search ; search = search->next) 
	{
        // is the element a pak file?
        if (search->pack) 
		{
            int index;

            //get the pack_t structure.
            pak = search->pack;

            //get the index that the file is stored at
            index = FindFileInPack(pak, filename);

            if (index != -1) 
			{
                //we found the file.
                // open a new file on the pakfile
                *file = fopen (pak->filename, "rb");
                if (!*file)
                    Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename); 
                fseek (*file, pak->files[index].filepos, SEEK_SET);
                // track usage
				files_opened++;
			
				compress_type = pak->files[index].compresstype;
				compress_length = pak->files[index].compresslen;

				if (compress_type) 
				{
					if (!from_loadfile) 
					{
						_RPTF1(_CRT_ERROR, "## Direct load of compressed file - %s", filename);
					}
				}

				return pak->files[index].filelen;
            }
        }
/*
		if( strstr( filename, ".mp3" ) || strstr( filename, ".csv" ) || strstr( filename, ".vsc" ) )
		{
			// SCG[4/14/00]: Hack because someone made the csv files in the demo vsc files..
			char *ptr = strstr( filename, ".csv" );
			if( ptr != NULL )
			{
				int length = ( ptr - filename ) + 1;
				Com_sprintf( hackfilename, MAX_OSPATH, ( char * ) filename );
				hackfilename[length + 0] = 'v';
				hackfilename[length + 1] = 's';
				hackfilename[length + 2] = 'c';

				// check a file in the directory tree
				Com_sprintf( netpath, sizeof(netpath), "%s/%s",search->filename, hackfilename );
			}
			else
			{
				// check a file in the directory tree
				Com_sprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
			}
            
            *file = fopen (netpath, "rb");
            if (!*file)
                continue;

       		// track usage
			files_opened++;
            return FS_filelength (*file);
        }
*/
    }

    *file = NULL;
    return -1;
/*
    searchpath_t    *search;
    char            netpath[MAX_OSPATH];
    pack_t          *pak;
    filelink_t      *link;


	compress_type   = 0;

    // check for links first
	if( filename == NULL || filename[0] == 0 )
	{
		Com_Printf( "Could not open file: NULL\n" );
		return -1;
	}

	if( strstr( filename, ".cfg") )
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
		
		*file = fopen (netpath, "rb");
		if (!*file)
			return -1;

		// track usage
		files_opened++;
		return FS_filelength (*file);
	}

    for (link = fs_links ; link ; link=link->next) 
    {
        if (!strncmp (filename, link->from, link->fromlength))
        {
            Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
            *file = fopen (netpath, "rb");
            if (*file)
            {       
            	// track usage
				files_opened++;
			    return FS_filelength (*file);
            }
            return -1;
        }
    }

    //
    // search through the path, one element at a time
    //
    for (search = fs_searchpaths ; search ; search = search->next) 
	{
		if( search->pack != NULL )
		{
			break;
		}
	}
	if( search == NULL || search->pack == NULL )
	{
		*file = NULL;
		return -1;
	}

	pak = search->pack;

    //get the index that the file is stored at
    int index = FindFileInPack(pak, filename);

    if (index != -1) {
        //we found the file.
        // open a new file on the pakfile
        *file = fopen (pak->filename, "rb");
        if (!*file)
            Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename); 
        fseek (*file, pak->files[index].filepos, SEEK_SET);
		// track usage
		files_opened++;

        return pak->files[index].filelen;
    }
  
    *file = NULL;
    return -1;
*/
}
#else
#ifndef NO_ADDONS
int FS_FOpenFile ( const char *filename, FILE **file) {
    searchpath_t    *search;
    char            netpath[MAX_OSPATH];
    pack_t          *pak;
    filelink_t      *link;
    // check for links first

	compress_type   = 0;

	if( filename == NULL || filename[0] == 0 )
	{
		Com_Printf( "Could not open file: NULL\n" );
		return -1;
	}

	if( strstr( filename, ".cfg") )
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_ConfigDir(), filename);	// check for a new config dir (+set configdir "...")

		*file = fopen(netpath,"rb");
		if (!*file)
		{
			// didn't find the file in the config dir...try the gamedir
			Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
			*file = fopen(netpath,"rb");
			if (!*file)
			{
				// still didn't find the file.  if the file was 'daikatana.cfg' or 'default_keys.cfg', keep looking, otherwise, fail out
				if (!(!stricmp(filename,"daikatana.cfg") || !stricmp(filename,"default_keys.cfg")))
					return -1;
			}
		}

		if (*file)
		{
			files_opened++;
			return FS_filelength (*file);
		}
	}

    for (link = fs_links ; link ; link=link->next) 
    {
        if (!strncmp (filename, link->from, link->fromlength))
        {
            Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
            *file = fopen (netpath, "rb");
            if (*file)
            {       
				// track usage
				files_opened++;
                return FS_filelength (*file);
            }
            return -1;
        }
    }

    //
    // search through the path, one element at a time
    //
    for (search = fs_searchpaths ; search ; search = search->next) {
        // is the element a pak file?
        if (search->pack) {
            int index;

            //get the pack_t structure.
            pak = search->pack;

            //get the index that the file is stored at
            index = FindFileInPack(pak, filename);

            if (index != -1) {
                //we found the file.
                // open a new file on the pakfile
                *file = fopen (pak->filename, "rb");
                if (!*file)
                    Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename); 
                fseek (*file, pak->files[index].filepos, SEEK_SET);
                // track usage
				files_opened++;
			
				compress_type = pak->files[index].compresstype;
				compress_length = pak->files[index].compresslen;

				if (compress_type) {
					if (!from_loadfile) {
						_RPTF1(_CRT_ERROR, "## Direct load of compressed file - %s", filename);
					}
				}


				return pak->files[index].filelen;
            }
        }
        else {      
            // check a file in the directory tree
            Com_sprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
            
            *file = fopen (netpath, "rb");
            if (!*file)
                continue;

       		// track usage
			files_opened++;
            return FS_filelength (*file);
        }
        
    }

    *file = NULL;
    return -1;
}
#else
// this is just for demos to prevent add on hacking
int FS_FOpenFile ( const char *filename, FILE **file)
{
    searchpath_t    *search;
    char            netpath[MAX_OSPATH];
    pack_t          *pak;
    filelink_t      *link;


	compress_type   = 0;

    // check for links first
	if( filename == NULL || filename[0] == 0 )
	{
		Com_Printf( "Could not open file: NULL\n" );
		return -1;
	}

	if( strstr( filename, ".cfg") )
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
		
		*file = fopen (netpath, "rb");
		if (!*file)
			return -1;

		// track usage
		files_opened++;
		return FS_filelength (*file);
	}

    for (link = fs_links ; link ; link=link->next) 
    {
        if (!strncmp (filename, link->from, link->fromlength))
        {
            Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
            *file = fopen (netpath, "rb");
            if (*file)
            {       
            	// track usage
				files_opened++;
			    return FS_filelength (*file);
            }
            return -1;
        }
    }

    //
    // search through the path, one element at a time
    //
    for (search = fs_searchpaths ; search ; search = search->next) 
	{
		if( search->pack != NULL )
		{
			break;
		}
	}
	if( search == NULL || search->pack == NULL )
	{
		*file = NULL;
		return -1;
	}

	pak = search->pack;

    //get the index that the file is stored at
    int index = FindFileInPack(pak, filename);

    if (index != -1) {
        //we found the file.
        // open a new file on the pakfile
        *file = fopen (pak->filename, "rb");
        if (!*file)
            Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename); 
        fseek (*file, pak->files[index].filepos, SEEK_SET);
		// track usage
		files_opened++;

//		compress_type = pak->files[index].compresstype;
//		compress_length = pak->files[index].compresslen;

        return pak->files[index].filelen;
    }
  
    *file = NULL;
    return -1;
}
#endif NO_ADDONS
#endif DAIKATANA_DEMO

int FS_FOpenTextFile( const char *filename, FILE **file)
{
    searchpath_t    *search;
    char            netpath[MAX_OSPATH];
    pack_t          *pak;
    filelink_t      *link;
    // check for links first

	if( filename == NULL || filename[0] == 0 )
	{
		Com_Printf( "Could not open file: NULL\n" );
		return -1;
	}

	if( strstr( filename, ".cfg") )
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
		
		*file = fopen (netpath, "rt");
		if (!*file)
			return -1;
		
		// track usage
		files_opened++;
		return FS_filelength (*file);
	}

    for (link = fs_links ; link ; link=link->next) 
    {
        if (!strncmp (filename, link->from, link->fromlength))
        {
            Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
            *file = fopen (netpath, "rt");
            if (*file)
            {       
				        // track usage
				        files_opened++;
                return FS_filelength (*file);
            }
            return -1;
        }
    }

    //
    // search through the path, one element at a time
    //
    for (search = fs_searchpaths ; search ; search = search->next) {
        // is the element a pak file?
        if (search->pack) {
            int index;

            //get the pack_t structure.
            pak = search->pack;

            //get the index that the file is stored at
            index = FindFileInPack(pak, filename);

            if (index != -1) {
                //we found the file.
                // open a new file on the pakfile
                *file = fopen (pak->filename, "rt");
                if (!*file)
                    Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename); 
                fseek (*file, pak->files[index].filepos, SEEK_SET);
                // track usage
				files_opened++;
				return pak->files[index].filelen;
            }
        }
        else {      
            // check a file in the directory tree
            Com_sprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
            
            *file = fopen (netpath, "rt");
            if (!*file)
                continue;

       		// track usage
			files_opened++;
            return FS_filelength (*file);
        }
        
    }

    *file = NULL;
    return -1;
}






/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop(void);
#define	MAX_READ	0x10000		// read in blocks of 64k
void FS_Read (void *buffer, int len, FILE *f)
{
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining)
	{
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, f);
		if (read == 0)
		{
			// we might have been trying to read from a CD
			if (!tries)
			{
				tries = 1;
				CDAudio_Stop();
			}
			else
				Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
		}

		if (read == -1)
			Com_Error (ERR_FATAL, "FS_Read: -1 bytes read");

		bytes_read += read;

		// do some progress bar thing here...

		remaining -= read;
		buf += read;
	}
}
/*
============
FS_LoadFile
Filename are relative to the Daikatana search path
a null buffer will just return the file length without loading
============
*/
char Noel_Test[1024];

int FS_LoadFile (const char *path, void **buffer)
{
	FILE	*h;
	byte	*buf;
	int		len;
	buf = NULL;	// quiet compiler warning
// look for it in the filesystem or pack files
	sprintf(Noel_Test, path);

	from_loadfile = 1;	
	len = FS_FOpenFile (path, &h);
	from_loadfile = 0;


	if (h == NULL)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}
	
	if (!buffer)
	{
		fclose (h);
		return len;
	}

	if (compress_type) {

		unsigned char  code;
		int  src_pos = 0;
		int  dst_pos = 0;
		int  i;
		char *src_data;
		char *dst_data;
//		int  size = len;

		src_data = (char*)X_Malloc(compress_length, MEM_TAG_FILE);
		dst_data = (char*)X_Malloc(len, MEM_TAG_FILE);

		FS_Read (src_data, compress_length, h);
		fclose (h);

		for (;;) {
			
			code = ((unsigned char*)src_data)[src_pos];
			src_pos++;

			// terminator
			if (code == 255) {
				break;

			} else if (code < 64) {

				// uncompressed block

				for (i = -1; i < (int)code; i++) {

					dst_data[dst_pos] = src_data[src_pos];
					dst_pos++;
					src_pos++;
				}

			} else if (code < 128) {

				// rlz

				for (i = 62; i < (int)code; i++) {

					dst_data[dst_pos] = 0;
					dst_pos++;
				}

			} else if (code < 192) {

				// run length encode

				for (i = 126; i < (int)code; i++) {

					dst_data[dst_pos] = src_data[src_pos];
					dst_pos++;
				}
				src_pos++;

			} else if (code < 254) {

				// reference previous data

				int  offset;

				offset = ((unsigned char*)src_data)[src_pos];
				src_pos++;

				for (i = 190; i < (int)code; i++) {

					dst_data[dst_pos] = dst_data[dst_pos - offset - 2];
					dst_pos++;
				}
			}
		}

		X_Free(src_data);
		*buffer = dst_data;
		return len;

	} else {
		buf = (byte *)X_Malloc(len, MEM_TAG_FILE);
		*buffer = buf;
		FS_Read (buf, len, h);
		fclose (h);
		return len;
	}
}
/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
	X_Free (buffer);
}
/*
=================
FS_LoadPackFile
Takes an explicit (not game tree related) path to a pak file.
Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/

//compare function used to sort the pack file entries based on their names.
dpackfile_t *file_info = NULL;
int pack_file_compare(const void *e1, const void *e2) {
    if (file_info == NULL) return 0;

    //the parameters are pointers to indexes into the file_info array.
    return stricmp(file_info[*((int *)(e1))].name, file_info[*((int *)(e2))].name);
}


pack_t *FS_LoadPackFile (char *packfile) {
    dpackheader_t   header;
    int             i;
    packfile_t      *newfiles;
    int             numpackfiles;
    pack_t          *pack;
    FILE            *packhandle;
//    dpackfile_t     info[MAX_FILES_IN_PACK];
    dpackfile_t     *info;
//    int             sort_table[MAX_FILES_IN_PACK];
    int             *sort_table;

    #ifdef NO_ADDONS
	unsigned        checksum;
    #endif
    #ifdef DAIKATANA_DEMO
	unsigned        checksum;
    #endif

    //open the file.
    packhandle = fopen(packfile, "rb");

    //check if we got the file open correctly
    if (packhandle == NULL) {
        return NULL;
    }

    //read the header.
    fread (&header, 1, sizeof(header), packhandle);

    //check if we go the header correctly.
    if (LittleLong(header.ident) != IDPAKHEADER) {
        Com_Error (ERR_FATAL, "%s is not a packfile", packfile);
    }

    header.dirofs = LittleLong (header.dirofs);
    header.dirlen = LittleLong (header.dirlen);

    //get the number of files in the pack file.
    numpackfiles = header.dirlen / sizeof(dpackfile_t);

    //make sure there is not too many files.
    if (numpackfiles > MAX_FILES_IN_PACK) {
        Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);
    }

    //alloc our array of file descriptors.
    newfiles = (packfile_t *)X_Malloc (numpackfiles * sizeof(packfile_t), MEM_TAG_FILE);

    //seek to the start of the directory info.
    fseek (packhandle, header.dirofs, SEEK_SET);

	// allocate memory for the packfile
	info = (dpackfile_t*) X_Malloc( numpackfiles * sizeof(dpackfile_t), MEM_TAG_FILE);

	sort_table = (int*) X_Malloc( numpackfiles * sizeof(int), MEM_TAG_FILE);

    //read the info on all the files.
    fread (info, 1, header.dirlen, packhandle);

    #ifdef NO_ADDONS
    //crc the directory to check for modifications
    checksum = Com_BlockChecksum ((void *)info, header.dirlen);

    if (checksum != PAK0_CHECKSUM) {
        return NULL;
    }
    #endif

    #ifdef DAIKATANA_DEMO
    //crc the directory to check for modifications
    checksum = Com_BlockChecksum ((void *)info, header.dirlen);

    if( ( checksum != DEMOPAK_CHECKSUM_1 ) && ( checksum != DEMOPAK_CHECKSUM_2 ) && ( checksum != DEMOPAK_CHECKSUM_3 ) )
	{
        return NULL;
    }
    #endif


    //initialize our sort table.
    for (i = 0; i < numpackfiles; i++) {
        sort_table[i] = i;
    }

    //give our compare function thefile info array.
    file_info = &info[0];

    //sort our table of indexes.
    qsort(&sort_table[0], numpackfiles, sizeof(int), pack_file_compare);

    file_info = NULL;

    // parse the directory
    for (i=0 ; i<numpackfiles ; i++)
    {
        strcpy (newfiles[i].name, info[sort_table[i]].name);
        newfiles[i].filepos = LittleLong(info[sort_table[i]].filepos);
        newfiles[i].filelen = LittleLong(info[sort_table[i]].filelen);

        newfiles[i].compresslen  = LittleLong(info[sort_table[i]].compresslen);
        newfiles[i].compresstype = LittleLong(info[sort_table[i]].compresstype);
    }

    pack = (pack_t *)X_Malloc (sizeof (pack_t), MEM_TAG_FILE);
    strcpy (pack->filename, packfile);
    pack->handle = packhandle;
    pack->numfiles = numpackfiles;
    pack->files = newfiles;
    
    Com_DPrintf ("Added packfile %s (%i files)\n", packfile, numpackfiles);

	X_Free( info );
	X_Free( sort_table );
    return pack;
}
/*
================
FS_AddGameDirectory
Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
void FS_AddGameDirectory (char *dir)
{
	int				i;
	searchpath_t	*search;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];
//	strcpy (fs_gamedir, dir);

	//
	// add any pak files in the format pak0.pak pak1.pak, ...
	//
	for (i=0; i<10; i++)
	{
		Com_sprintf (pakfile, sizeof(pakfile), "%s/pak%i.pak", dir, i);
		pak = FS_LoadPackFile (pakfile);
		if (!pak)
			continue;
		search = (searchpath_t *)X_Malloc (sizeof(searchpath_t), MEM_TAG_FILE);
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;		
	}

	//
	// add the directory to the search path
	//
	search = (searchpath_t *)X_Malloc (sizeof(searchpath_t), MEM_TAG_FILE);
	strcpy (search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;

}
/*
============
FS_Gamedir
Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void)
{
	return fs_gamedir;
}

/*
============
FS_ConfigDir
Called to find where to write and read config files
============
*/
char *FS_ConfigDir(void)
{
//	cvar_t *cl = Cvar_Get("configdir","",0);
	char *result = Cvar_VariableString("configdir");
	return (strlen(result) > 0) ? result : FS_Gamedir();
}

char *FS_SaveGameDir(void)
{
#ifdef _DEBUG
	char *result = Cvar_VariableString("sv_savegamedir");

	if( (result == NULL) || (result && (strlen(result) == 0)) )
		result = FS_Gamedir();

	return result;
#endif

	return FS_Gamedir();
}
/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void)
{
	char *dir;
	char name [MAX_QPATH];

	dir = Cvar_VariableString("gamedir");
	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir); 
	else
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, BASEDIRNAME); 
	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText ("exec autoexec.cfg\n");
	Sys_FindClose();
}


/*
================
FS_SetGamedir
Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir (char *dir)
{
	searchpath_t	*next;
	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":") )
	{
		Com_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}
	//
	// free up any current game dir info
	//
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose (fs_searchpaths->pack->handle);
			X_Free (fs_searchpaths->pack->files);
			X_Free (fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		X_Free (fs_searchpaths);
		fs_searchpaths = next;
	}
	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText ("vid_restart\nsnd_restart\n");

	Com_sprintf (fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);
	if (!strcmp(dir,BASEDIRNAME) || (*dir == 0))
	{
		Cvar_FullSet ("gamedir", "", CVAR_SERVERINFO|CVAR_NOSET);
		Cvar_FullSet ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	}
	else
	{
		Cvar_FullSet ("gamedir", dir, CVAR_SERVERINFO|CVAR_NOSET);
		if (fs_cddir->string[0])
			FS_AddGameDirectory (va("%s/%s", fs_cddir->string, dir) );
		FS_AddGameDirectory (va("%s/%s", fs_basedir->string, dir) );
	}
}
/*
================
FS_Link_f
Creates a filelink_t
================
*/
void FS_Link_f (void)
{
	filelink_t	*l, **prev;
	if (GetArgc() != 3)
	{
		Com_Printf ("USAGE: link <from> <to>\n");
		return;
	}
	// see if the link already exists
	prev = &fs_links;
	for (l=fs_links ; l ; l=l->next)
	{
		if (!strcmp (l->from, GetArgv(1)))
		{
			X_Free (l->to);
			if (!strlen(GetArgv(2)))
			{	// delete it
				*prev = l->next;
				X_Free (l->from);
				X_Free (l);
				return;
			}
			l->to = CopyString (GetArgv(2));
			return;
		}
		prev = &l->next;
	}
	// create a new link
	l = (filelink_t *)X_Malloc(sizeof(*l), MEM_TAG_FILE);
	l->next = fs_links;
	fs_links = l;
	l->from = CopyString(GetArgv(1));
	l->fromlength = strlen(l->from);
	l->to = CopyString(GetArgv(2));
}

/*
** FS_ListFiles
*/
char **FS_ListFiles( char *findname, int *numfiles, unsigned musthave, unsigned canthave )
{
	char *s;
	int nfiles = 0;
	char **list = 0;

	s = Sys_FindFirst( findname, musthave, canthave );
	while ( s )
	{
		if ( s[strlen(s)-1] != '.' )
			nfiles++;
		s = Sys_FindNext( musthave, canthave );
	}
	Sys_FindClose ();

	if ( !nfiles )
		return NULL;

	nfiles++; // add space for a guard
	*numfiles = nfiles;

	list = (char **)X_Malloc( sizeof( char * ) * nfiles, MEM_TAG_FILE);
	memset( list, 0, sizeof( char * ) * nfiles );

	s = Sys_FindFirst( findname, musthave, canthave );
	nfiles = 0;
	while ( s )
	{
		if ( s[strlen(s)-1] != '.' )
		{
			list[nfiles] = strdup( s );
#ifdef _WIN32
			strlwr( list[nfiles] );
#endif
			nfiles++;
		}
		s = Sys_FindNext( musthave, canthave );
	}
	Sys_FindClose ();

	return list;
}

/*
** FS_Dir_f
*/
void FS_Dir_f( void )
{
	char	*path = NULL;
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if ( GetArgc() != 1 )
	{
		strcpy( wildcard, GetArgv( 1 ) );
	}

	while ( ( path = FS_NextPath( path ) ) != NULL )
	{
		char *tmp = findname;

		Com_sprintf( findname, sizeof(findname), "%s/%s", path, wildcard );

		while ( *tmp != 0 )
		{
			if ( *tmp == '\\' ) 
				*tmp = '/';
			tmp++;
		}
		Com_Printf( "Directory of %s\n", findname );
		Com_Printf( "----\n" );

		if ( ( dirnames = FS_ListFiles( findname, &ndirs, 0, 0 ) ) != 0 )
		{
			int i;

			for ( i = 0; i < ndirs-1; i++ )
			{
				if ( strrchr( dirnames[i], '/' ) )
					Com_Printf( "%s\n", strrchr( dirnames[i], '/' ) + 1 );
				else
					Com_Printf( "%s\n", dirnames[i] );

				X_Free( dirnames[i] );
			}
			X_Free( dirnames );
		}
		Com_Printf( "\n" );
	};
}
/*
============
FS_Path_f
============
*/
void FS_Path_f (void)
{
	searchpath_t	*s;
	filelink_t		*l;
	Com_Printf ("Current search path:\n");
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf ("----------\n");
		if (s->pack)
			Com_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf ("%s\n", s->filename);
	}
	Com_Printf ("\nLinks:\n");
	for (l=fs_links ; l ; l=l->next)
		Com_Printf ("%s : %s\n", l->from, l->to);
}
/*
================
FS_NextPath
Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;
	prev = fs_gamedir;
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s->pack)
			continue;
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}
	return NULL;
}

/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void)
{
	int			i, new_basedir = false;
	char		basedir [MAX_OSPATH];

#ifndef DAIKATANA_DEMO
	char		szDrivePath[] = "?:\\";
	char		szTestCDPath[MAX_OSPATH];
	char		cDriveLetter;
	int			nDriveType;
	qboolean	bFoundCD = FALSE;
	qboolean	bIsFullInstall;
#endif DAIKATANA_DEMO

	// mdm 98.02.11 - only change basedir for windows
#if _MSC_VER
	//	Nelno:	check for basedir override
	i = COM_CheckParm ("-basedir");
    if (i && i < EXEGetArgc() -1)
	{
		new_basedir = true;

		//	get the new base directory from the command line
        strcpy (basedir, EXEGetArgv(i+1));
		
		//	check to see if we can change to that directory
		if (_chdir (basedir))
		{
			Com_Error (ERR_FATAL, "FS_InitFileSystem: %s is not a valid -basedir.\n", basedir);
			//	can't change for some reason, so go to default basedir
			new_basedir = false;
		}
	}

	if (new_basedir)
	{
		_getcwd (basedir, MAX_OSPATH);
		fs_basedir = Cvar_Get ("basedir", basedir, CVAR_NOSET);
	}
	else
#endif
		fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);

	FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_basedir->string) );

	Cmd_AddCommand ("path", FS_Path_f);
	Cmd_AddCommand ("link", FS_Link_f);
	Cmd_AddCommand ("dir", FS_Dir_f );
	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
//	fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);
	//
	// cddir <path>
	// Logically concatenates the cddir after the basedir for 
	// allows the game to run from outside the data tree
	//
	// SCG[3/16/00]: Check registry for installation type and if it's not a full install
	// SCG[3/16/00]: add the cddir
	
	fs_cddir = Cvar_Get( "cddir", "", CVAR_NOSET );

#ifndef DAIKATANA_DEMO
	bIsFullInstall = FS_IsFullInstall();
	if( ( bIsFullInstall == FALSE ) && ( fs_cddir->string[0] == 0 ) )
	{
		for( cDriveLetter = 'D'; cDriveLetter <= 'Z'; cDriveLetter++ )
		{
			szDrivePath[0] = cDriveLetter;
			nDriveType = GetDriveType( szDrivePath );

			if( nDriveType == DRIVE_CDROM )
			{
				sprintf( szTestCDPath, "%c:\\data\\pak2.pak", cDriveLetter );
				if( _access( szTestCDPath, 0 ) == -1 )
				{
					bFoundCD = FALSE;
				}	
				else
				{
					bFoundCD = TRUE;
					break;
				}
			}
		}

		if( bFoundCD == TRUE )
		{
			szDrivePath[2] = 0;
			Cvar_ForceSet( "cddir", szDrivePath );

			// SCG[3/23/00]: Try to load something off of the cd...
			// SCG[3/23/00]: We will check for both .pak files.
			sprintf( szTestCDPath, "%s\\data\\pak3.pak", szDrivePath );
			if( _access( szTestCDPath, 0 ) == -1 )
			{
#ifdef _DEBUG
				Com_DPrintf( "Could not find the Daikatana CD in Drive %s\n", szDrivePath );
#else
				Sys_Error( "Could not find the Daikatana CD in Drive %s", szDrivePath );
#endif _DEBUG
			}

		}
		else
		{
#ifdef _DEBUG
			Com_DPrintf( "Could not locate the Daikatana CD\n" );
#else
			Sys_Error( "Could not locate the Daikatana CD" );
#endif _DEBUG
		}
	}
#endif DAIKATANA_DEMO

	if (fs_cddir->string[0])
		FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_cddir->string) );

	//
	// start up with baseq2 by default
	//
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_basedir->string) );
	strcpy (fs_gamedir, va("%s/"BASEDIRNAME, fs_basedir->string));

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;
	// check for game override
 	fs_gamedirvar = Cvar_Get ("game", "", CVAR_LATCH|CVAR_SERVERINFO);

	if (fs_gamedirvar->string[0])
		FS_SetGamedir (fs_gamedirvar->string);

	cvar_t *fs_configdirvar = Cvar_Get ("configdir", "", 0);
	if( ( fs_configdirvar != NULL ) && ( fs_configdirvar->string != NULL ) )
	{
		FS_AddGameDirectory( fs_configdirvar->string );
	}
}

int FS_GetFilesOpened()
{
	return files_opened;
}

long FS_GetBytesRead()
{
	return bytes_read;
}

///////////////////////////////////////////////////////////////////////////////
//
//  FS_ResetStats
//
///////////////////////////////////////////////////////////////////////////////
void FS_ResetStats()
{
	files_opened = 0; 
	bytes_read = 0;
}