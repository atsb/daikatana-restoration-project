// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include <string.h>
#include "memmgrcommon.h"
#include "csv.h"
#include "dk_io.h"
#include "decode.h"
#include "dk_defines.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CCSVFile::CCSVFile()
{
	Init();
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CCSVFile::~CCSVFile()
{
	if ( pFileBuffer )
	{
		pFileBuffer = FILEBUFFER_Delete( pFileBuffer );
	}
	if ( file )
	{
		DKIO_FCloseFile(file);
	}
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CCSVFile::Init()
{
	bUsingFile = TRUE;
	pFileBuffer = NULL;
	file = NULL;

	nLinePosition = 0;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CCSVFile::OpenFile( const char *szFileName )
{
	char szEncFileName[64];
	int  nStrLength;
	int  nErrorCode;

	Init();


    nErrorCode = DKIO_ERROR_NONE;

	// LAB
	// try the file with the .vsc extension first.

	nStrLength = strlen( szFileName );
	strcpy(szEncFileName, szFileName);
	szEncFileName[nStrLength - 3] = 'v';
	szEncFileName[nStrLength - 2] = 's';
	szEncFileName[nStrLength - 1] = 'c';


	int bEncrypted = ENCRYPT_IsFileEncrypted(szEncFileName);
	if (bEncrypted) {

		bUsingFile = FALSE;
		pFileBuffer = ENCRYPT_DecodeToBuffer(szEncFileName);

		if (!pFileBuffer) {
			// error occured
			nErrorCode = ENCRYPT_GetErrorCode();
		}

	} else {

		file = NULL;

		//for now, we cannot use the .pak file code.
		int nFileLen = DKIO_FOpenFile(szFileName, &file);

		if (file) {

			bUsingFile = TRUE;

		} else {

			nErrorCode = DKIO_ERROR_EMPTY;

		}
	}

	return nErrorCode;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CCSVFile::CloseFile()
{
	if ( bUsingFile )
	{
		if ( file )
		{
			DKIO_FCloseFile(file);
		}
	}
	else
	{
		if ( pFileBuffer )
		{
			pFileBuffer = FILEBUFFER_Delete( pFileBuffer );	
		}
	}
	Init();
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CCSVFile::SeekToBeginning()
{
	if ( bUsingFile )
	{
		_ASSERTE( file );
		DKIO_FSeek( file, 0, SEEK_SET );
	}
	else
	{
		_ASSERTE( pFileBuffer );
		pFileBuffer->Init();
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GetLine
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
char *CCSVFile::GetLine( char *line, int n )
{
	if ( bUsingFile )
	{
		return DKIO_FGets( line, n, file );
	}

	_ASSERTE( pFileBuffer );

	return pFileBuffer->FGetLine( line, n );
}

// ----------------------------------------------------------------------------
//
// Name:		GetNextLine
// Description:
// Input:
// Output:
//				EOF			=> end of file stream 
//				otherwise	=> number of chars in the line
// Note:
//
// ----------------------------------------------------------------------------
int CCSVFile::GetNextLine( char *szLine )
{
	while ( 1 )
	{
		if ( GetLine( szLine, 2048 ) == NULL )
		{
			return EOF;
		}

		// skip blank lines
		if ( szLine[0] != '\n' && szLine[0] != '\r' )
		{
			break;
		}
	}

	nLinePosition = 0;

	return strlen( szLine );
}

// ----------------------------------------------------------------------------
//
// Name:		GetFirstElement
// Description:
// Input:
// Output:
//				EOLN	=> end of line
//				0		=> no element
//				> 0		=> number of chars in the element
// Note:
//
// ----------------------------------------------------------------------------
int CCSVFile::GetFirstElement( const char *szLine, char *szElement )
{
	if ( szLine[nLinePosition] == '\n' || szLine[nLinePosition] == '\r' )
	{
		return EOLN;
	}
	if ( szLine[nLinePosition] == ',' )
	{
		nLinePosition++;
	}

	// reset line position at the beginning of file
	nLinePosition = 0;

	int nCharCounter = 0;
	szElement[0] = '\0';
	while ( szLine[nLinePosition] != ',' && szLine[nLinePosition] != '\n' && szLine[nLinePosition] != '\r'  )
	{
		szElement[nCharCounter++] = szLine[nLinePosition++];
	}
	_ASSERTE( nCharCounter < 64 );
	szElement[nCharCounter] = '\0';

	return nCharCounter;
}

// ----------------------------------------------------------------------------
//
// Name:		GetNextElement
// Description:
// Input:
// Output:
//				EOLN	=> end of line
//				0		=> no element
//				> 0		=> number of chars in the element
// Note:
//
// ----------------------------------------------------------------------------
int CCSVFile::GetNextElement( const char *szLine, char *szElement )
{
	if ( szLine[nLinePosition] == '\n' || szLine[nLinePosition] == '\r' )
	{
		return EOLN;
	}
	if ( szLine[nLinePosition] == ',' )
	{
		nLinePosition++;
	}

	int nCharCounter = 0;
	szElement[0] = '\0';
	while ( szLine[nLinePosition] != ',' && szLine[nLinePosition] != '\n' && szLine[nLinePosition] != '\r' )
	{
		szElement[nCharCounter++] = szLine[nLinePosition++];
	}
	_ASSERTE( nCharCounter < 64 );
	szElement[nCharCounter] = '\0';

	return nCharCounter;
}


// ----------------------------------------------------------------------------
//
// Name:		CSV_OpenFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CSV_OpenFile( const char *szFileName, CCSVFile **pCsvFile )
{
	*pCsvFile = new CCSVFile;
	if ( !*pCsvFile )
	{
		return NULL;
	}

	int nRetCode = (*pCsvFile)->OpenFile( szFileName );
//	if ( nRetCode != ERROR_NONE ) //SCG:2-26-99
	if ( nRetCode != DKIO_ERROR_NONE )
	{
		delete *pCsvFile;
		*pCsvFile = NULL;
	}

	return nRetCode;
}

// ----------------------------------------------------------------------------
//
// Name:		CSV_CloseFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CSV_CloseFile( CCSVFile *pCsvFile )
{
	_ASSERTE( pCsvFile );
	pCsvFile->CloseFile();
}

// ----------------------------------------------------------------------------
//
// Name:		CSV_SeekToBeginning
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CSV_SeekToBeginning( CCSVFile *pCsvFile )
{
	_ASSERTE( pCsvFile );
	pCsvFile->SeekToBeginning();
}

// ----------------------------------------------------------------------------
//
// Name:		CSV_GetNextLine
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CSV_GetNextLine( CCSVFile *pCsvFile, char *szLine )
{
	_ASSERTE( pCsvFile );
	
	return pCsvFile->GetNextLine( szLine );
}

// ----------------------------------------------------------------------------
//
// Name:		CSV_GetFirstElement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CSV_GetFirstElement( CCSVFile *pCsvFile, const char *szLine, char *szElement )
{
	_ASSERTE( pCsvFile );

	return pCsvFile->GetFirstElement( szLine, szElement );
}

// ----------------------------------------------------------------------------
//
// Name:		CSV_GetNextElement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CSV_GetNextElement( CCSVFile *pCsvFile, const char *szLine, char *szElement )
{
	_ASSERTE( pCsvFile );
	return pCsvFile->GetNextElement( szLine, szElement );
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
