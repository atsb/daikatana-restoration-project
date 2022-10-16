
#include <stdio.h>
#include <string.h>
#include "dk_io.h"

class DKFileIO
{
public:
	DKFileIO();
	DKFileIO( FOpenFile_t	fOpen, 
			  FCloseFile_t	fClose, 
			  FFileLength_t	fLength, 
			  FRead_t		fRead, 
			  FSeek_t		fSeek, 
			  FTell_t		fTell, 
			  FGetc_t		fGetc, 
			  FGets_t		fGets );

	void Init( FOpenFile_t	fOpen, 
			   FCloseFile_t	fClose, 
			   FFileLength_t	fLength, 
			   FRead_t		fRead, 
			   FSeek_t		fSeek, 
			   FTell_t		fTell, 
			   FGetc_t		fGetc, 
			   FGets_t		fGets );

	int     (*FOpenFile)	(const char *filename, FILE **file);
	void    (*FCloseFile)	(FILE *f);
	int		(*FFileLength)	(FILE *f);
	void	(*FRead)		(void *buffer, int len, FILE *f);
	int		(*FSeek)		(FILE *f, long offset, int origin);
	long	(*FTell)		(FILE *f);
	int		(*FGetc)		(FILE *f);
	char*	(*FGets)		(char *string, int n, FILE *f);
};


DKFileIO dkFileIO;

static int bUsingPAKFile = 0;

// ----------------------------------------------------------------------------


DKFileIO::DKFileIO()
{
	FOpenFile	= NULL;
	FCloseFile	= NULL;
	FFileLength = NULL;
	FRead = NULL;
	FSeek = NULL;
	FTell = NULL;
	FGetc = NULL;
	FGets = NULL;
}

DKFileIO::DKFileIO( FOpenFile_t	fOpen, 
					 FCloseFile_t	fClose, 
					 FFileLength_t	fLength, 
					 FRead_t		fRead, 
					 FSeek_t		fSeek, 
					 FTell_t		fTell, 
					 FGetc_t		fGetc, 
					 FGets_t		fGets )
{
	FOpenFile	= fOpen;
	FCloseFile	= fClose;
	FFileLength = fLength;
	FRead = fRead;
	FSeek = fSeek;
	FTell = fTell;
	FGetc = fGetc;
	FGets = fGets;
}

void DKFileIO::Init( FOpenFile_t	fOpen, 
					 FCloseFile_t	fClose, 
					 FFileLength_t	fLength, 
					 FRead_t		fRead, 
					 FSeek_t		fSeek, 
					 FTell_t		fTell, 
					 FGetc_t		fGetc, 
					 FGets_t		fGets )
{
	FOpenFile	= fOpen;
	FCloseFile	= fClose;
	FFileLength = fLength;
	FRead = fRead;
	FSeek = fSeek;
	FTell = fTell;
	FGetc = fGetc;
	FGets = fGets;
}




void DKIO_Init( FOpenFile_t	fOpen, 
				FCloseFile_t	fClose, 
				FFileLength_t	fLength, 
				FRead_t		fRead, 
				FSeek_t		fSeek, 
				FTell_t		fTell, 
				FGetc_t		fGetc, 
				FGets_t		fGets,
				int bUsingPAKFileFlag )
{
	bUsingPAKFile = bUsingPAKFileFlag;
	dkFileIO.Init( fOpen, fClose, fLength, fRead, fSeek, fTell, fGetc, fGets );
}

int DKIO_FOpenFile(const char *filename, FILE **file)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FOpenFile )
		{
			return dkFileIO.FOpenFile( filename, file );
		}
	}
	else
	{
		char szNetPath[128];
		strcpy( szNetPath, "./data/" );
		strcat( szNetPath, filename );
		*file = fopen( szNetPath, "rb" );
		if ( !(*file) )
		{
			return -1;
		}
		return 1;
	}

	return -1;
}

void DKIO_FCloseFile(FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FCloseFile )
		{
			dkFileIO.FCloseFile( f );
		}
	}
	else
	{
		fclose( f );
	}
}

int	DKIO_FFileLength(FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FFileLength )
		{
			return dkFileIO.FFileLength( f );
		}
	}
	else
	{
		rewind( f );
		int nStartFile = ftell( f );
		fseek( f, 0L, SEEK_END );
		int nFileLength = ftell( f ) - nStartFile;
    
		rewind( f );

		return nFileLength;
	}
			
	return -1;
}

void DKIO_FRead(void *buffer, int len, FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FRead )
		{
			dkFileIO.FRead( buffer, len, f );
		}
	}
	else
	{
		fread( buffer, len, 1, f );
	}
}

int	DKIO_FSeek(FILE *f, long offset, int origin)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FSeek )
		{
			return dkFileIO.FSeek( f, offset, origin );
		}
	}
	else
	{
		fseek( f, offset, origin );
	}

	return -1;
}

long DKIO_FTell(FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FTell )
		{
			return dkFileIO.FTell( f );
		}
	}
	else
	{
		return ftell( f );
	}

	return -1;
}

int	DKIO_FGetc(FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FGetc )
		{
			return dkFileIO.FGetc( f );
		}
	}
	else
	{
		return fgetc( f );
	}

	return -1;
}

char* DKIO_FGets(char *string, int n, FILE *f)
{
	if ( bUsingPAKFile )
	{
		if ( dkFileIO.FGets )
		{
			return dkFileIO.FGets( string, n, f );
		}
	}
	else
	{
		return fgets( string, n, f );
	}

	return NULL;
}
