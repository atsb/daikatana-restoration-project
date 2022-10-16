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
#include "weapons.h"
#include "w_attribute.h"
#include "filebuffer.h"
//#include "decode.h"// SCG[1/24/00]: not used
#include "dk_io.h"
#include "csv.h"

/* ***************************** define types ****************************** */
#define MAX_NUM_PLAYER_WEAPONS		35

/* ***************************** Local Variables *************************** */

static int nNumWeapons = 0;
static CWeaponAttributes aWeaponAttributes[MAX_NUM_PLAYER_WEAPONS];

//static int bUsingFile = TRUE;// SCG[1/24/00]: not used
//static CFileBuffer *pFileBuffer = NULL;
//static FILE *file = NULL;// SCG[1/24/00]: not used

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

#define RATING					0
#define AMMO_MAX				1
#define AMMO_PER_USE			2
#define AMMO_DISPLAY_DIVISOR	3
#define INITIAL_AMMO			4
#define SPEED					5
#define RANGE					6
#define DAMAGE					7
#define LIFETIME				8
#define X1_OFFSET				9
#define Y1_OFFSET				10
#define Z1_OFFSET				11
#define X2_OFFSET				12
#define Y2_OFFSET				13
#define Z2_OFFSET				14
#define X3_OFFSET				15
#define Y3_OFFSET				16
#define Z3_OFFSET				17
#define DISPLAY_ORDER   18

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CWeaponAttributes::CWeaponAttributes()
{
}

CWeaponAttributes::CWeaponAttributes( char *name )
{
	SetWeaponName( name );
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
CWeaponAttributes::~CWeaponAttributes()
{
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
void CWeaponAttributes::SetWeaponName( char *name )
{
	strcpy( weaponName, name );
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
void WEAPONATTRIBUTE_Init()
{
	nNumWeapons = 0;
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
CWeaponAttributes *WEAPONATTRIBUTE_GetNextElement()
{
	_ASSERTE( nNumWeapons < MAX_NUM_PLAYER_WEAPONS );

	CWeaponAttributes *pWeaponAttribute = &(aWeaponAttributes[nNumWeapons]);
	nNumWeapons++;

	return pWeaponAttribute;
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
CWeaponAttributes *WEAPONATTRIBUTE_Find( char *weaponName )
{
	for ( int i = 0; i < nNumWeapons; i++ )
	{
		CWeaponAttributes *pWeaponAttributes = &(aWeaponAttributes[i]);
		if ( _stricmp( pWeaponAttributes->GetWeaponName(), weaponName ) == 0 )
		{
			return pWeaponAttributes;
		}
	}

	return NULL;
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
static void WEAPONATTRIBUTE_ParseData( CWeaponAttributes *pWeaponAttributes, char *szData, int nCurrentDataIndex )
{
	int nValue = -1;
	float fValue = -1.0f;

	switch ( nCurrentDataIndex )
	{
		case RATING:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetRating( nValue );
			break;
		}
		case AMMO_MAX:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetAmmoMax( nValue );
			break;
		}
		case AMMO_PER_USE:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetAmmoPerUse( nValue );
			break;
		}
		case AMMO_DISPLAY_DIVISOR:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetAmmoDisplayDivisor( nValue );
			break;
		}
		case INITIAL_AMMO:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetInitialAmmo( nValue );
			break;
		}
		case SPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetSpeed( fValue );
			break;
		}
		case RANGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetRange( fValue );
			break;
		}
		case DAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetDamage( fValue );
			break;
		}
		case LIFETIME:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetLifeTime( fValue );
			break;
		}
		case X1_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetX1Offset( fValue );
			break;
		}
		case Y1_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetY1Offset( fValue );
			break;
		}
		case Z1_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetZ1Offset( fValue );
			break;
		}
		case X2_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetX2Offset( fValue );
			break;
		}
		case Y2_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetY2Offset( fValue );
			break;
		}
		case Z2_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetZ2Offset( fValue );
			break;
		}
		case X3_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetX3Offset( fValue );
			break;
		}
		case Y3_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetY3Offset( fValue );
			break;
		}
		case Z3_OFFSET:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetZ3Offset( fValue );
			break;
		}

		case DISPLAY_ORDER:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			pWeaponAttributes->SetDisplayOrder( nValue );
			break;
		}


		default:
		{
			break;
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		WEAPONATTRIBUTE_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int WEAPONATTRIBUTE_ReadFile( const char *szFileName )
{
	WEAPONATTRIBUTE_Init();

    int bSuccess = TRUE;

	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
//	if ( nRetCode == ERROR_NONE ) //SCG:2-26-99
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumWeapons = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF && nNumWeapons < MAX_NUM_PLAYER_WEAPONS )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				CWeaponAttributes *pWeaponAttributes = WEAPONATTRIBUTE_GetNextElement();
				if ( pWeaponAttributes )
				{
					pWeaponAttributes->SetWeaponName( szElement );

					int nNumElements = 0;
					while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
					{
						WEAPONATTRIBUTE_ParseData( pWeaponAttributes, szElement, nNumElements );
						nNumElements++;	
					}
			
					nNumWeapons++;
				}
			}
		}

		CSV_CloseFile( pCsvFile );
	}
	else
	{
		bSuccess = FALSE;
	}		

	return bSuccess;
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
void WEAPONATTRIBUTE_SetInfo( weaponInfo_t *winfo )
{
	CWeaponAttributes *pWeaponAttributes = WEAPONATTRIBUTE_Find( winfo->weaponName );
	if ( pWeaponAttributes )
	{
		winfo->rating				        = pWeaponAttributes->GetRating();
		winfo->ammo_max				      = pWeaponAttributes->GetAmmoMax();
		winfo->ammo_per_use			    = pWeaponAttributes->GetAmmoPerUse();
		winfo->ammo_display_divisor	= pWeaponAttributes->GetAmmoDisplayDivisor();
								
		winfo->initialAmmo			= pWeaponAttributes->GetInitialAmmo();
		                       
		winfo->speed				= pWeaponAttributes->GetSpeed();
		winfo->range				= pWeaponAttributes->GetRange();
		winfo->damage				= pWeaponAttributes->GetDamage();
		winfo->lifetime				= pWeaponAttributes->GetLifeTime();
		winfo->fWeaponOffsetX1		= pWeaponAttributes->GetX1Offset();
		winfo->fWeaponOffsetY1		= pWeaponAttributes->GetY1Offset();
		winfo->fWeaponOffsetZ1		= pWeaponAttributes->GetZ1Offset();
		winfo->fWeaponOffsetX2		= pWeaponAttributes->GetX2Offset();
		winfo->fWeaponOffsetY2		= pWeaponAttributes->GetY2Offset();
		winfo->fWeaponOffsetZ2		= pWeaponAttributes->GetZ2Offset();
		winfo->fWeaponOffsetX3		= pWeaponAttributes->GetX3Offset();
		winfo->fWeaponOffsetY3		= pWeaponAttributes->GetY3Offset();
		winfo->fWeaponOffsetZ3		= pWeaponAttributes->GetZ3Offset();

		winfo->display_order      = pWeaponAttributes->GetDisplayOrder();
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

#if 0
/*
// ----------------------------------------------------------------------------
//
// Name:		WEAPONATTRIBUTE_ParseLine
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void WEAPONATTRIBUTE_ParseLine( CWeaponAttributes *pWeaponAttributes, char *szLine )
{
	int nCurrentDataIndex = 0;

	int nDataCounter = 0;
	char szData[128];

	int nLineCounter = 0;
	while ( szLine[nLineCounter] != '\n' && szLine[nLineCounter] != '\r' )
	{
		if ( szLine[nLineCounter] == ',' )
		{
			szData[nDataCounter] = '\0';
			nDataCounter = 0;

			WEAPONATTRIBUTE_ParseData( pWeaponAttributes, szData, nCurrentDataIndex );

			nCurrentDataIndex++;
		}
		else
		{
			szData[nDataCounter++] = szLine[nLineCounter];
		}

		nLineCounter++;
	}

	// do the last element
	if ( szLine[nLineCounter] == '\n' || szLine[nLineCounter] == '\r' )
	{
		szData[nDataCounter] = '\0';

		WEAPONATTRIBUTE_ParseData( pWeaponAttributes, szData, nCurrentDataIndex );
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
static char *GetLine( char *line, int n )
{
	if ( bUsingFile )
	{
		return gstate->FS_Gets( line, n, file );
	}

	_ASSERTE( pFileBuffer );

	return pFileBuffer->FGetLine( line, n );
}

// ----------------------------------------------------------------------------
//
// Name:		WEAPONATTRIBUTE_ReadAndSetData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void WEAPONATTRIBUTE_ReadAndSetData()
{
	char line[512];
	char string[40];
	int nNumBlankLines = 0;

	while ( GetLine( line, 512 ) != NULL )
	{
		if ( line[0] == '\n' || line[0] == '\r' || nNumWeapons >= MAX_NUM_PLAYER_WEAPONS )
		{
			nNumBlankLines++;
			if ( nNumBlankLines > 3 )
			{
				break;
			}
			else
			{
				nNumBlankLines = 0;
				continue;
			}
		}

		int nCharCounter = 0;
		while ( line[nCharCounter] != ',' )
		{
			if ( line[nCharCounter] != ' ' && line[nCharCounter] != '\t' )
			{
				string[nCharCounter] = line[nCharCounter];
			}
			nCharCounter++;
		}

		string[nCharCounter++] = '\0';

		if ( strlen( string ) > 0 )
		{
			CWeaponAttributes *pWeaponAttributes = WEAPONATTRIBUTE_GetNextElement();
			pWeaponAttributes->SetWeaponName( string );
			
			WEAPONATTRIBUTE_ParseLine( pWeaponAttributes, line+nCharCounter );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		WEAPONATTRIBUTE_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WEAPONATTRIBUTE_ReadFile( char *szFileName )
{
	WEAPONATTRIBUTE_Init();

    int bEncrypted = ENCRYPT_IsFileEncrypted( szFileName );
	if ( bEncrypted )
	{
		bUsingFile = FALSE;
		pFileBuffer = ENCRYPT_DecodeToBuffer( szFileName );
		if ( pFileBuffer )
		{
			WEAPONATTRIBUTE_ReadAndSetData();

			pFileBuffer = FILEBUFFER_Delete( pFileBuffer );
		}
		else
		{
			// error occured
			int nErrorCode = ENCRYPT_GetErrorCode();
		}
	}
	else
	{
		file = NULL;

		//for now, we cannot use the .pak file code.
		int nFileLen = gstate->FS_Open( szFileName, &file );
		if ( file != NULL )
		{
			bUsingFile = TRUE;

			gstate->FS_Seek(file, 0, SEEK_SET);
    
			WEAPONATTRIBUTE_ReadAndSetData();

			// close the file
			gstate->FS_Close(file);
		}
		else
		{
			bUsingFile = FALSE;

			int nStrLength = strlen( szFileName );
			char szNewFileName[64];
			strcpy( szNewFileName, szFileName );
			szNewFileName[nStrLength-1] = '2';
			pFileBuffer = ENCRYPT_DecodeToBuffer( szNewFileName );
			if ( pFileBuffer )
			{
				WEAPONATTRIBUTE_ReadAndSetData();

				pFileBuffer = FILEBUFFER_Delete( pFileBuffer );
			}
			else
			{
				// error occured
				int nErrorCode = ENCRYPT_GetErrorCode();
				gstate->Con_Printf( "[%s] <- weapon file file not found!!!\n", szFileName );
			}
		}
	}
}
*/
#endif 0

