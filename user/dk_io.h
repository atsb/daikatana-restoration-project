#ifndef _DKIO_H
#define _DKIO_H

// renamed due to conflicts with windows macro definitions
/*
#define ERROR_NONE			0
#define ERROR_OPENINGFILE	1
#define ERROR_EMPTY			2
#define ERROR_CORRUPT		3
#define ERROR_MEMFAILED		4
*/
#define DKIO_ERROR_NONE			0
#define DKIO_ERROR_OPENINGFILE	1
#define DKIO_ERROR_EMPTY		2
#define DKIO_ERROR_CORRUPT		3
#define DKIO_ERROR_MEMFAILED	4


typedef int		(*FOpenFile_t)	(const char *filename, FILE **file);
typedef void	(*FCloseFile_t)	(FILE *f);
typedef int		(*FFileLength_t)(FILE *f);
typedef void	(*FRead_t)		(void *buffer, int len, FILE *f);
typedef int		(*FSeek_t)		(FILE *f, long offset, int origin);
typedef long	(*FTell_t)		(FILE *f);
typedef int		(*FGetc_t)		(FILE *f);
typedef char*	(*FGets_t)		(char *string, int n, FILE *f);



void DKIO_Init( FOpenFile_t	fOpen, 
				FCloseFile_t	fClose, 
				FFileLength_t	fLength, 
				FRead_t		fRead, 
				FSeek_t		fSeek, 
				FTell_t		fTell, 
				FGetc_t		fGetc, 
				FGets_t		fGets,
				int bUsingPAKFilesFlag = 0 );
int		DKIO_FOpenFile(const char *filename, FILE **file);
void	DKIO_FCloseFile(FILE *f);
int		DKIO_FFileLength(FILE *f);
void	DKIO_FRead(void *buffer, int len, FILE *f);
int		DKIO_FSeek(FILE *f, long offset, int origin);
long	DKIO_FTell(FILE *f);
int		DKIO_FGetc(FILE *f);
char*	DKIO_FGets(char *string, int n, FILE *f);

#endif _DKIO_H