// ==========================================================================
//
//  File:       Action.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#include <ctype.h>
#include <iostream>

#ifdef CHECK_SYNTAX

#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "action.h"
#include "collect.h"

#else CHECK_SYNTAX

#include <stdio.h>
#include <stdlib.h>
//#include "dk_std.h"// SCG[1/23/00]: not used
#include "dk_shared.h"
#include "world.h"
#include "ai_common.h"
#include "ai.h"
#include "ai_func.h"
#include "collect.h"
#include "action.h"
#include "ai_utils.h"

#endif CHECK_SYNTAX

/* ***************************** define types ****************************** */

#ifndef TRUE
#define TRUE 1
#endif TRUE

#ifndef FALSE
#define FALSE 0
#endif FALSE

#define ACTION_ERROR_NONE					0

#define ACTION_ERROR_UNEXPECTED_TOKEN		1
#define ACTION_ERROR_NO_OPEN_PAREN			2
#define ACTION_ERROR_NO_CLOSE_PAREN			3
#define ACTION_ERROR_NO_OPEN_BRACE			4
#define ACTION_ERROR_NO_CLOSE_BRACE			5
#define ACTION_ERROR_NO_END_STATEMENT		6
#define ACTION_ERROR_NO_COMMA				7
#define ACTION_ERROR_EXPECTED_NUMERALS		8
#define ACTION_ERROR_EXPECTED_STRING		9
#define ACTION_ERROR_MEM_FAILED				10
#define ACTION_ERROR_EXPTECTED_TRUE_FALSE	11
#define ACTION_ERROR_EXPECTED_QUOTE         12
#define ACTION_ERROR_EXCEEDED_QUOTE_LIMIT   13

#define ACTION_ERROR_MAX					14

// Logic[5/25/99]: strset is not ANSI, not portable
#ifndef WIN32
#define strset(a, x) memset(a, x, strlen(a))
#endif

class CFileInfo
{
private:
	FILE *file;
	char szCurrentFileName[64];
	int nCurrentLineNumber;

public:
	CFileInfo( FILE *f, const char *szFileName);
	~CFileInfo();

//	void* CFileInfo::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CFileInfo::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CFileInfo::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CFileInfo::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	int GetCurrentLineNumber()		{ return nCurrentLineNumber; }
	void IncrementLineNumber()		{ nCurrentLineNumber++; }

	char *GetFileName()				{ return szCurrentFileName; }
	FILE *GetFile()					{ return file; };
};


typedef struct _actionError
{
	int nActionErrorCode;
	char *szErrorString;
} ACTION_ERROR;

typedef struct _predefinedActions
{
	int	 nActionType;
	char *szAction;
	CAction *(*fnParseAction)( CFileInfo *pFileInfo );
} PREDEFINED_ACTIONS;


/* ***************************** Local Variables *************************** */


ACTION_ERROR actionErrors[] =
{
	{ ACTION_ERROR_NONE,				"No Error." },
	{ ACTION_ERROR_UNEXPECTED_TOKEN,	"Unexpected token" },
	{ ACTION_ERROR_NO_OPEN_PAREN,		"Expected an open parenthesis" },
	{ ACTION_ERROR_NO_CLOSE_PAREN,		"Expected a closed parenthesis" },
	{ ACTION_ERROR_NO_OPEN_BRACE,		"Expected an open brace" },
	{ ACTION_ERROR_NO_CLOSE_BRACE,		"Expected a closed brace" },
	{ ACTION_ERROR_NO_END_STATEMENT,	"Expected a semicolon" },
	{ ACTION_ERROR_NO_COMMA,			"Expected a comma" },
	{ ACTION_ERROR_EXPECTED_NUMERALS,	"Expected a number" },
	{ ACTION_ERROR_EXPECTED_STRING,		"Expected a string" },
	{ ACTION_ERROR_MEM_FAILED,			"Memory allocation failed" },
	{ ACTION_ERROR_EXPTECTED_TRUE_FALSE,"Expected a true or a false" },
    { ACTION_ERROR_EXPECTED_QUOTE,      "Expected a quote" },
    { ACTION_ERROR_EXCEEDED_QUOTE_LIMIT,"Exceeded a quote limit" },
};	


// defined tokens
const char QUOTE[]          = "\"";
const char OPEN_PAREN[]		= "(";
const char CLOSED_PAREN[]	= ")";
const char OPEN_BRACE[]		= "{";
const char CLOSED_BRACE[]	= "}";
const char END_STATEMENT[]	= ";";
const char COMMA[]			= ",";
const char TOKEN_TRUE[]		= "true";
const char TOKEN_FALSE[]	= "false";
const char IDLE[]			= "idle";

const char INCLUDE[]		= "include";
const char SCRIPT[]			= "script";
const char LOOP_SCRIPT[]	= "loop_script";
const char WHEN_USED[]		= "when_used";
const char LEVEL_START[]	= "level_start";
const char LEVEL_EOF[]		= "end_of_script";

static CAction *ACTION_ParseAnimate( CFileInfo *pFileInfo );
static CAction *ACTION_ParseAnimatePartial( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSound( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSpawn( CFileInfo *pFileInfo );
static CAction *ACTION_ParseMoveTo( CFileInfo *pFileInfo );
static CAction *ACTION_ParseFaceAngle( CFileInfo *pFileInfo );
static CAction *ACTION_ParseUse( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSendMessage( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSetMovingAnimation( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSetState( CFileInfo *pFileInfo );
static CAction *ACTION_ParseDie( CFileInfo *pFileInfo );
static CAction *ACTION_ParseCall( CFileInfo *pFileInfo );
static CAction *ACTION_ParseRandomScript( CFileInfo *pFileInfo );
static CAction *ACTION_ParseStreamSound( CFileInfo *pFileInfo );
static CAction *ACTION_ParseSendUrgentMessage( CFileInfo *pFileInfo );
static CAction *ACTION_ParseComeNear( CFileInfo *pFileInfo );
static CAction *ACTION_ParseRemove( CFileInfo *pFileInfo );
static CAction *ACTION_ParseLookAt( CFileInfo *pFileInfo );
static CAction *ACTION_ParseStopLook( CFileInfo *pFileInfo );
static CAction *ACTION_ParseAttack( CFileInfo *pFileInfo );
static CAction *ACTION_ParsePrint( CFileInfo *pFileInfo );

PREDEFINED_ACTIONS aPredefinedActions[] =
{
	{ ACTIONTYPE_UNDEFINED,			"undefined",			NULL },
	{ ACTIONTYPE_ANIMATE,			"animate",				ACTION_ParseAnimate },
	{ ACTIONTYPE_ANIMATEPARTIAL,	"animate_partial",		ACTION_ParseAnimatePartial, },
	{ ACTIONTYPE_SOUND,				"sound",				ACTION_ParseSound, },
	{ ACTIONTYPE_SPAWN,				"spawn",				ACTION_ParseSpawn, },
	{ ACTIONTYPE_MOVETO,			"move_to",				ACTION_ParseMoveTo, },
	{ ACTIONTYPE_FACEANGLE,			"face_angle",			ACTION_ParseFaceAngle, },
	{ ACTIONTYPE_USE,				"use",					ACTION_ParseUse, },
	{ ACTIONTYPE_SENDMESSAGE,		"send_message",			ACTION_ParseSendMessage, },
	{ ACTIONTYPE_SETMOVINGANIMATION,"set_moving_animation",	ACTION_ParseSetMovingAnimation, },
	{ ACTIONTYPE_SETSTATE,			"set_state",			ACTION_ParseSetState, },
	{ ACTIONTYPE_DIE,				"die",					ACTION_ParseDie, },
	{ ACTIONTYPE_WHENUSED_SEQUENCE, "",						NULL, },
	{ ACTIONTYPE_WHENUSED_IDLE,		"idle",					NULL, },
	{ ACTIONTYPE_CALL,				"call",					ACTION_ParseCall, },
	{ ACTIONTYPE_RANDOMSCRIPT,		"random_script",		ACTION_ParseRandomScript, },
	{ ACTIONTYPE_STREAMSOUND,		"stream_sound",			ACTION_ParseStreamSound, },
	{ ACTIONTYPE_SENDURGENTMESSAGE, "send_urgent_message",  ACTION_ParseSendUrgentMessage, },
	{ ACTIONTYPE_COMENEAR,			"come_near",			ACTION_ParseComeNear, },
	{ ACTIONTYPE_REMOVE,			"remove",				ACTION_ParseRemove, },
	{ ACTIONTYPE_LOOKAT,			"look_at",				ACTION_ParseLookAt, },
	{ ACTIONTYPE_STOPLOOK,			"stop_look",			ACTION_ParseStopLook, },
	{ ACTIONTYPE_ATTACK,			"attack",			    ACTION_ParseAttack, },
	{ ACTIONTYPE_PRINT,			    "print",			    ACTION_ParsePrint, },

};
const int nNumPredefinedActions = sizeof(aPredefinedActions) / sizeof(aPredefinedActions[0]);

const char szScriptPath[] = "cin/aiscripts/";

const char whiteSpaces[] = " \t\r\n";
const int nNumWhiteSpaces = sizeof(whiteSpaces) / sizeof(whiteSpaces[0]);


static int nTokenLength = 0;
static char szCurrentToken[64] = { "\0" };

const static int nMaxTextLength = 256;
static char szCurrentText[nMaxTextLength] = { "\0" };

static int nLastError = ACTION_ERROR_NONE;

static CMapStringToPtr mapStringToScriptAction;
static CMapStringToPtr mapUniqueIDToEntity;
static CMapStringToPtr mapUniqueIDToWhenUsedCommand;
static CScriptAction *pLevelStartScript = NULL;

/* ***************************** Local Functions *************************** */
static int ACTION_Parse( const char *szFileName );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

// ----------------------------------------------------------------------------

#ifdef CHECK_SYNTAX

	int SCRIPT_fgetc( FILE *stream )
	{
		return fgetc( stream );
	}
	int SCRIPT_ungetc( int c, FILE *stream )
	{
		return ungetc( c, stream );
	}
	FILE *SCRIPT_fopen( const char *filename, const char *mode )
	{
		return fopen( filename, mode );
	}
	int SCRIPT_fclose( FILE *stream )
	{
		return fclose( stream );
	}

#else CHECK_SYNTAX

	int SCRIPT_fgetc( FILE *stream )
	{
		return gstate->FS_Getc( stream );
	}
	int SCRIPT_ungetc( int c, FILE *stream )
	{
		return gstate->FS_Ungetc( c, stream );
	}
	FILE *SCRIPT_fopen( const char *filename, const char *mode )
	{
		FILE *file;
		gstate->FS_Open( filename, &file );

		return file;
	}
	void SCRIPT_fclose( FILE *stream )
	{
		gstate->FS_Close( stream );
	}

#endif CHECK_SYNTAX


//*****************************************************************************
//								class functions
//*****************************************************************************

// ----------------------------------------------------------------------------

CFileInfo::CFileInfo( FILE *f, const char *szFileName )
{
	file = f;
	strcpy( szCurrentFileName, szFileName );
	nCurrentLineNumber = 1;			// line number in a file start with 1
}

CFileInfo::~CFileInfo()
{
}

// ----------------------------------------------------------------------------
//
// Name:        ACTION_SetLastError
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void ACTION_SetLastError( int nNewError )
{
	_ASSERTE( nNewError >= ACTION_ERROR_NONE && nNewError < ACTION_ERROR_MAX );

	nLastError = nNewError;
}

// ----------------------------------------------------------------------------
//
// Name:        ACTION_GetLastError
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_GetLastError()
{
	return nLastError;
}


// ----------------------------------------------------------------------------
//
// Name:        CParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CParameter::CParameter()
{
	nParameterType = PARAMETERTYPE_UNDEFINED;
}

// ----------------------------------------------------------------------------
//
// Name:        CParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CParameter::CParameter( int nType )
{
	nParameterType = nType;
}

CParameter::~CParameter()
{
}

// ----------------------------------------------------------------------------
//
// Name:        CValueParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CValueParameter::CValueParameter() : CParameter( PARAMETERTYPE_VALUE )
{
	fValue = 0.0f;
}

// ----------------------------------------------------------------------------
//
// Name:        CValueParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CValueParameter::CValueParameter( float fNewValue ) : CParameter( PARAMETERTYPE_VALUE )
{
	fValue = fNewValue;
}

CValueParameter::~CValueParameter()
{
}

// ----------------------------------------------------------------------------
//
// Name:        CStringParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CStringParameter::CStringParameter() : CParameter( PARAMETERTYPE_STRING )
{
	strset( szString, 0 );
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
CStringParameter::CStringParameter( char *szNewString ) : CParameter( PARAMETERTYPE_STRING )
{
	strcpy( szString, szNewString );
}

CStringParameter::~CStringParameter()
{
}

// ----------------------------------------------------------------------------
//
// Name:        CTextParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CTextParameter::CTextParameter() : CParameter( PARAMETERTYPE_TEXT )
{
    pszText = NULL;
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
CTextParameter::CTextParameter( char *szNewText ) : CParameter( PARAMETERTYPE_TEXT )
{
    SetText( szNewText );
}

CTextParameter::~CTextParameter()
{
    if ( pszText )
    {
        delete [] pszText;
        pszText = NULL;
    }
}

void CTextParameter::SetText( char *szNewText )
{
    int nStrLength = strlen( szNewText );
    pszText = new char[nStrLength+1];

    strcpy( pszText, szNewText );
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        CAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CAction::CAction()
{
	nActionType = ACTIONTYPE_UNDEFINED;
}

// ----------------------------------------------------------------------------
//
// Name:        CAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CAction::CAction( int nType )
{
	nActionType = nType;
}

// ----------------------------------------------------------------------------
//
// Name:        ~CAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CAction::~CAction()
{
	POSITION pos = parameterList.GetHeadPosition();
	while ( pos )
	{
		CParameter *pParameter = (CParameter*)parameterList.GetNext( pos );
		if ( pParameter )
		{
			delete pParameter;
		}
	}

	parameterList.RemoveAll();
}

// ----------------------------------------------------------------------------
//
// Name:        AddParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAction::AddParameter( CParameter *pNewParameter )
{
	_ASSERTE( pNewParameter );

	parameterList.AddTail( pNewParameter );
}

// ----------------------------------------------------------------------------
//
// Name:        GetParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CParameter *CAction::GetParameter( int nIndex )
{
	_ASSERTE( nIndex >= 0 );

	CParameter *pParameter = NULL;
	POSITION pos = parameterList.FindIndex( nIndex );
	if ( pos != NULL )
	{
		pParameter = (CParameter*)parameterList.GetAt( pos );
	}

	return pParameter;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        CScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CScriptAction::CScriptAction()
{
	nScriptType = SCRIPTTYPE_ONCE;
	nLoopCount = -1;					// -1 => loop forever by default

	strset( szName, 0 );
	strset( szUniqueID, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:        CScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CScriptAction::CScriptAction( char *szNewName, int nType, int nValue )
{
	nScriptType = nType;
	nLoopCount = nValue;

	strcpy( szName, szNewName );
	strset( szUniqueID, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:        ~CScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CScriptAction::~CScriptAction()
{
	Destroy();
}

// ----------------------------------------------------------------------------
//
// Name:        Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CScriptAction::Destroy()
{
	POSITION pos = actionList.GetHeadPosition();
	while ( pos )
	{
		CAction *pAction = (CAction*)actionList.GetNext( pos );
		if ( pAction )
		{
			delete pAction;
		}
	}
			
	actionList.RemoveAll();
}

// ----------------------------------------------------------------------------
//
// Name:        AddAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CScriptAction::AddAction( CAction *pNewAction )
{
	_ASSERTE( pNewAction );

	actionList.AddTail( pNewAction );
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        CWhenUsedCommand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CWhenUsedCommand::CWhenUsedCommand()
{
	nNumUsed = 0;
    use_delay = 1.0f;
    use_time = 0;
	pIdleAction = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        ~CWhenUsedCommand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CWhenUsedCommand::~CWhenUsedCommand()
{
	POSITION pos = actionList.GetHeadPosition();
	while ( pos )
	{
		CAction *pAction = (CAction*)actionList.GetNext( pos );
		if ( pAction )
		{
			delete pAction;
		}
	}
			
	actionList.RemoveAll();

	delete pIdleAction;
}

// ----------------------------------------------------------------------------
//
// Name:        AddAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CWhenUsedCommand::AddAction( CAction *pNewAction, int nIndex )
{
	_ASSERTE( pNewAction );

	if ( nIndex < 0 )
	{
		actionList.AddTail( pNewAction );
	}
	else
	{
		POSITION pos = actionList.FindIndex( nIndex );
		if ( pos != NULL )
		{
			actionList.InsertBefore( pos, pNewAction );
		}
		else
		{
			actionList.AddTail( pNewAction );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        GetAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CAction *CWhenUsedCommand::GetAction( int nIndex )
{
	_ASSERTE( nIndex >= 0 );

	CAction *pAction = NULL;
	POSITION pos = actionList.FindIndex( nIndex );
	if ( pos != NULL )
	{
		pAction = (CAction*)actionList.GetAt( pos );
	}

	return pAction;
}

// ----------------------------------------------------------------------------
//
// Name:        GetCurrentAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CAction *CWhenUsedCommand::GetCurrentAction()
{
	CAction *pAction = NULL;
	POSITION pos = actionList.FindIndex( nNumUsed );
	if ( pos != NULL )
	{
		pAction = (CAction*)actionList.GetAt( pos );
	}

	return pAction;
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        SCRIPTACTION_Verify 
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void SCRIPTACTION_Verify()
{
#ifdef _DEBUG
	char *key;
	void *value;
	POSITION pos = mapStringToScriptAction.GetStartPosition();
	while ( pos )
	{
		mapStringToScriptAction.GetNextAssoc( pos, key, value );
		_ASSERTE( strlen( key ) > 0 );

		CScriptAction *pScriptAction = (CScriptAction*)value;
        _ASSERTE( pScriptAction );
	}
#endif _DEBUG
}

// ----------------------------------------------------------------------------
//
// Name:        SCRIPTACTION_Lookup 
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CScriptAction *SCRIPTACTION_Lookup( const char *szScriptActionName )
{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szScriptActionName) );
#else
	char *pszTemp = _strlwr( strdup(szScriptActionName) );
#endif

	void *pValue = NULL;
    mapStringToScriptAction.Lookup(pszTemp, pValue);
    CScriptAction *pScriptAction = (CScriptAction *)pValue;

    free( pszTemp );

    SCRIPTACTION_Verify();
    return pScriptAction;
}

// ----------------------------------------------------------------------------
//
// Name:        SCRIPTACTION_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void SCRIPTACTION_Add( const char *szScriptActionName, CScriptAction *pScriptAction )
{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szScriptActionName) );
#else
	char *pszTemp = _strlwr( strdup(szScriptActionName) );
#endif

    CScriptAction *pTempScriptAction = SCRIPTACTION_Lookup( pszTemp );
	if ( pTempScriptAction )
	{
        gstate->Con_Dprintf( "Script %s already exists!", szScriptActionName );		
        return;
	}

	_ASSERTE( pScriptAction );
    mapStringToScriptAction.SetAt( pszTemp, pScriptAction );

    free( pszTemp );
}

// ----------------------------------------------------------------------------
//
// Name:        SCRIPTACTION_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int SCRIPTACTION_Init()
{
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SCRIPTACTION_Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void SCRIPTACTION_Destroy()
{
	char *key;
	void *value;
	POSITION pos = mapStringToScriptAction.GetStartPosition();
	while ( pos )
	{
		mapStringToScriptAction.GetNextAssoc( pos, key, value );
		_ASSERTE( strlen( key ) > 0 );

		CScriptAction *pScriptAction = (CScriptAction*)value;
        _ASSERTE( pScriptAction );
		pScriptAction->Destroy();
		delete pScriptAction;
	}

	mapStringToScriptAction.RemoveAll();
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        WHENUSECOMMAND_Lookup
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CWhenUsedCommand *WHENUSECOMMAND_Lookup( const char *szUniqueID )
{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szUniqueID) );
#else
	char *pszTemp = _strlwr( strdup(szUniqueID) );
#endif

	void *pValue = NULL;
    mapUniqueIDToWhenUsedCommand.Lookup(pszTemp, pValue);
    
    CWhenUsedCommand *pWhenUsedCommand = (CWhenUsedCommand *)pValue;

    free( pszTemp );

    return pWhenUsedCommand;
}

// ----------------------------------------------------------------------------
//
// Name:        WHENUSEDCOMMAND_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void WHENUSEDCOMMAND_Add( const char *szUniqueID, CWhenUsedCommand *pWhenUsedCommand )
{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szUniqueID) );
#else
	char *pszTemp = _strlwr( strdup(szUniqueID) );
#endif

    CWhenUsedCommand *pTempCommand = WHENUSECOMMAND_Lookup( pszTemp );
	if ( pTempCommand )
	{
        gstate->Con_Dprintf( "WhenUsedCommand for %s already exists!", szUniqueID );		
        return;
	}

	mapUniqueIDToWhenUsedCommand.SetAt( pszTemp, pWhenUsedCommand );
    free( pszTemp );
}

// ----------------------------------------------------------------------------
//
// Name:        WHENUSEDCOMMAND_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int WHENUSEDCOMMAND_Init()
{
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        WHENUSEDCOMMAND_Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void WHENUSEDCOMMAND_Destroy()
{
	char *key;
	void *value;
	POSITION pos = mapUniqueIDToWhenUsedCommand.GetStartPosition();
	while ( pos )
	{
		mapUniqueIDToWhenUsedCommand.GetNextAssoc( pos, key, value );

		_ASSERTE( strlen( key ) > 0 );
		CWhenUsedCommand *pWhenUsedCommand = (CWhenUsedCommand*)value;
		delete pWhenUsedCommand;
	}

	mapUniqueIDToWhenUsedCommand.RemoveAll();
}

// ----------------------------------------------------------------------------
#ifndef CHECK_SYNTAX

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Lookup
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *UNIQUEID_Lookup( const char *szUniqueID )
{
	if ( !szUniqueID || strlen( szUniqueID ) == 0 )
	{
		return NULL;
	}

#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szUniqueID) );
#else
	char *pszTemp = _strlwr( strdup(szUniqueID) );
#endif

	void *pValue = NULL;
    mapUniqueIDToEntity.Lookup(pszTemp, pValue);
    userEntity_t *pEntity = (userEntity_t*)pValue;

    free( pszTemp );

	if( pEntity == NULL )
	{
		return NULL;
	}

	if ( AI_IsAlive( pEntity ) )
	{
		return pEntity;
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void UNIQUEID_Add( const char *szUniqueID, userEntity_t *pEntity )
{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szUniqueID) );
#else
	char *pszTemp = _strlwr( strdup(szUniqueID) );
#endif

	userEntity_t *pTempEntity = UNIQUEID_Lookup( pszTemp );
#ifdef WIN32
	if ( pTempEntity && pTempEntity != pEntity )
	{
		_RPT3( _CRT_WARN, "Two entities: %s and %s have the same unique id %s\n", 
			   pTempEntity->className, pEntity->className, szUniqueID );
	}
#endif
	mapUniqueIDToEntity.SetAt( pszTemp, pEntity );

    free( pszTemp );

	// set up the command when this entity gets used
	CWhenUsedCommand *pWhenUsedCommand = WHENUSECOMMAND_Lookup( szUniqueID );
	if ( pWhenUsedCommand )
	{
		pEntity->use = AI_WhenUsedByPlayer;

		playerHook_t *hook = AI_GetPlayerHook( pEntity );
		_ASSERTE( hook );
		hook->pWhenUsedCommand = pWhenUsedCommand;
	}

	pEntity->szUniqueID = strdup(szUniqueID);
}

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Remove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void UNIQUEID_Remove( const char *szUniqueID )
{
/*
	if ( szUniqueID )
	{
#ifdef _WIN32
	char *pszTemp = _strlwr( _strdup(szUniqueID) );
#else
	char *pszTemp = _strlwr( strdup(szUniqueID) );
#endif
		mapUniqueIDToEntity.RemoveKey( pszTemp );
	    free( pszTemp );
	}
*/
}

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Remove
// Description:
// Input:		an entity
// Output:
// Note:		uses the szUniqueID tag to remove this entity's id from the list
//				called when an entity is removed.
// ----------------------------------------------------------------------------
void UNIQUEID_RemoveEnt( userEntity_t *self )
{
	if ( self->szUniqueID )
	{
#ifdef _WIN32
		char *pszTemp = _strlwr( _strdup(self->szUniqueID) );
#else
		char *pszTemp = _strlwr( strdup(self->szUniqueID) );
#endif
		mapUniqueIDToEntity.RemoveKey( pszTemp );
	    free( pszTemp );

		free( self->szUniqueID );
		self->szUniqueID = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int UNIQUEID_Init()
{
	// setup a mapping of unique ID to entities
	userEntity_t *head = gstate->FirstEntity();
    while ( head )
    {
		if ( head->epair )
		{
			for ( int i = 0; head->epair[i].key != NULL; i++ )
			{
				if ( _stricmp( head->epair[i].key, "uniqueid") == 0 )
				{
					UNIQUEID_Add( head->epair[i].value, head );
					break;
				}
			}
		}

		head = gstate->NextEntity(head);
    }

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        UNIQUEID_Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void UNIQUEID_Destroy()
{
	mapUniqueIDToEntity.RemoveAll();
}

#endif CHECK_SYNTAX


//*****************************************************************************
//								LOCAL functions
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:        ACTION_IsStartComment
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_IsStartComment( CFileInfo *pFileInfo, char ch )
{
	if ( ch == '/' )
	{
		// check the next char
		if ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
		{
			// push back the last char
			SCRIPT_ungetc( ch, pFileInfo->GetFile() );
			
			if ( ch == '/' )
			{
				return TRUE;
			}
		}

	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_IsWhiteSpace
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_IsWhiteSpace( char ch )
{
	for ( int i = 0; i < nNumWhiteSpaces; i++ )
	{
		if ( whiteSpaces[i] == ch )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_SkipToEndOfLine
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_SkipToEndOfLine( CFileInfo *pFileInfo )
{
	int ch;
	while ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
	{
		if ( ch == '\n' )
		{
			pFileInfo->IncrementLineNumber();
			break;
		}
	}

	return ch;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_SkipComments
// Description:
// Input:
// Output:
//				TRUE	=> comment line detected and the rest of the line is skipped
//				FALSE	=> no commnet line
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_SkipComments( CFileInfo *pFileInfo, int ch )
{
	if ( ch == '/' )
	{
		// check the next char
		if ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
		{
			if ( ch == '/' )
			{
				// comment line, skip to end of line
				ACTION_SkipToEndOfLine( pFileInfo );

				return TRUE;
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_SkipWhiteSpaces
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int ACTION_SkipWhiteSpaces( CFileInfo *pFileInfo )
{
	int ch;
	while ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
	{
		if ( ACTION_SkipComments( pFileInfo, ch ) == FALSE )
		{
			if ( ch == '\n' )
			{
				pFileInfo->IncrementLineNumber();
			}
			
			if ( !ACTION_IsWhiteSpace( ch ) )
			{
				break;
			}
		}
	}

	if ( ch != EOF )
	{
		// push back the last char
		SCRIPT_ungetc( ch, pFileInfo->GetFile() );
	}

	return ch;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_GetNextToken
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_GetNextToken( CFileInfo *pFileInfo )
{
	int ch;
	int nTokenLength = 0;
	
	ACTION_SkipWhiteSpaces( pFileInfo );

    while ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
	{
		if ( ch == OPEN_PAREN[0]	||
			 ch == CLOSED_PAREN[0]	||
			 ch == OPEN_BRACE[0]	||
			 ch == CLOSED_BRACE[0]	||
			 ch == END_STATEMENT[0]	||
			 ch == COMMA[0] )
		{
			if ( nTokenLength == 0 )
			{
				szCurrentToken[nTokenLength++] = ch;

				// get the next character
				ch = SCRIPT_fgetc( pFileInfo->GetFile() );
			}
			break;
		}
		else
		if ( !ACTION_IsWhiteSpace( ch ) && !ACTION_IsStartComment( pFileInfo, ch ) )
		{
			szCurrentToken[nTokenLength++] = ch;
		}
		else
		{
			break;
		}
	}
	
	if ( ch != EOF )
	{
		szCurrentToken[nTokenLength++] = '\0';
		// push back the last char
		SCRIPT_ungetc( ch, pFileInfo->GetFile() );
	}

	return nTokenLength;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_GetText
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_GetText( CFileInfo *pFileInfo )
{
	int ch = 0;
    int nTextLength = 0;
	
	ACTION_SkipWhiteSpaces( pFileInfo );

    // get the first open quote
    int bFirstQuote = FALSE;
    while ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
	{
		if ( ch == QUOTE[0] )
        {
            bFirstQuote = TRUE;
            break;
        }
    }

    if ( bFirstQuote == TRUE )
    {
        while ( (ch = SCRIPT_fgetc( pFileInfo->GetFile() )) != EOF )
	    {
		    if ( ch == QUOTE[0] )
            {
                szCurrentText[nTextLength++] = '\0';
                break;
            }
		    else
            if ( ch == '\n' )
            {
                // got a newline before getting an end quote
                ACTION_SetLastError( ACTION_ERROR_EXPECTED_QUOTE );
                break;
            }
            else
		    {
			    if ( nTextLength > nMaxTextLength )
                {
                    ACTION_SetLastError( ACTION_ERROR_EXCEEDED_QUOTE_LIMIT );
                    break;
                }
                szCurrentText[nTextLength++] = ch;
		    }
	    }
    }
    else
    {
        ACTION_SetLastError( ACTION_ERROR_EXPECTED_QUOTE );
    }
    	
	return nTextLength;
}

// ----------------------------------------------------------------------------
//
// Name:        IsNumeric
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsNumeric( const char *szString )
{
	int nLength = strlen( szString );	
	_ASSERTE( nLength > 0 );

	// handle negative numbers
	int nStartCounter = 0;
	if ( szString[0] == '-' )
	{
		nStartCounter = 1;	
	}
			
	for ( int i = nStartCounter; i < nLength; i++ )
	{
		if ( (szString[i] < '0' || szString[i] > '9') &&
			 szString[i] != '.' )
		{
			return FALSE;
		}		
	}
	
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsInteger
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsInteger( const char *szString )
{
	int nLength = strlen( szString );	
	_ASSERTE( nLength > 0 );

	// handle negative numbers
	int nStartCounter = 0;
	if ( szString[0] == '-' )
	{
		nStartCounter = 1;	
	}
			
	for ( int i = nStartCounter; i < nLength; i++ )
	{
		if ( szString[i] < '0' || szString[i] > '9' )
		{
			return FALSE;
		}		
	}
	
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsString
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsString( const char *szString )
{
	if ( IsNumeric( szString ) )
	{
		return FALSE;
	}

	int nLength = strlen( szString );	
	_ASSERTE( nLength > 0 );

	for ( int i = 0; i < nLength; i++ )
	{
		if ( !isalnum( szString[i] ) &&
			 szString[i] != '_' )
		{
			return FALSE;
		}		
	}
	
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsFileName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsFileName( const char *szString )
{
	int nLength = strlen( szString );	
	_ASSERTE( nLength > 0 );

	for ( int i = 0; i < nLength; i++ )
	{
		if ( !isalnum( szString[i] ) && 
			 szString[i] != '.' &&
			 szString[i] != '_' &&
			 szString[i] != '-' &&
			 szString[i] != '/' &&
			 szString[i] != '\\' )
		{
			return FALSE;
		}		
	}
	
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsOpenParen
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsOpenParen( const char *szString )
{
	if ( _stricmp( szString, OPEN_PAREN ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_OPEN_PAREN );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsClosedParen
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsClosedParen( const char *szString )
{
	if ( _stricmp( szString, CLOSED_PAREN ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_CLOSE_PAREN );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsOpenBrace
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsOpenBrace( const char *szString )
{
	if ( _stricmp( szString, OPEN_BRACE ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_OPEN_BRACE );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsClosedBrace
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsClosedBrace( const char *szString )
{
	if ( _stricmp( szString, CLOSED_BRACE ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_CLOSE_BRACE );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsEndStatement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsEndStatement( const char *szString )
{
	if ( _stricmp( szString, END_STATEMENT ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_END_STATEMENT );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsComma
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsComma( const char *szString )
{
	if ( _stricmp( szString, COMMA ) != 0 )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_COMMA );
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsTrue
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsTrue( const char *szString )
{
	if ( _stricmp( szString, TOKEN_TRUE ) != 0 )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        IsFalse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int IsFalse( const char *szString )
{
	if ( _stricmp( szString, TOKEN_FALSE ) != 0 )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        GetOpenParen
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetOpenParen( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsOpenParen( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetClosedParen
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetClosedParen( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsClosedParen( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetOpenBrace
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetOpenBrace( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsOpenBrace( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetClosedBrace
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetClosedBrace( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsClosedBrace( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetEndStatement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetEndStatement( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsEndStatement( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetComma
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int GetComma( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	return IsComma( szCurrentToken );
}

// ----------------------------------------------------------------------------
//
// Name:        GetTrueOrFalse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static CValueParameter *GetTrueOrFalse( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	if ( !IsString( szCurrentToken ) )
	{
		ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
		return NULL;
	}

	int nTrue = IsTrue( szCurrentToken );
	int nFalse = IsFalse( szCurrentToken );
	if ( !nTrue && !nFalse )
	{
		ACTION_SetLastError( ACTION_ERROR_EXPTECTED_TRUE_FALSE );
		return NULL;
	}
	
	CValueParameter *pValueParameter = NULL;
	if ( nTrue )
	{
		pValueParameter = new CValueParameter( TRUE );
	}
	else
	{
		pValueParameter = new CValueParameter( FALSE );
	}
	if ( !pValueParameter )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	return pValueParameter;
}

// ----------------------------------------------------------------------------
//
// Name:        GetStringParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static CStringParameter *GetStringParameter( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	if ( !IsString( szCurrentToken ) )
	{
		ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
		return NULL;
	}
	CStringParameter *pStringParameter = new CStringParameter( szCurrentToken );
	if ( !pStringParameter )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	return pStringParameter;
}

// ----------------------------------------------------------------------------
//
// Name:        GetFileNameParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static CStringParameter *GetFileNameParameter( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	if ( !IsFileName( szCurrentToken ) )
	{
		ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
		return NULL;
	}
	CStringParameter *pStringParameter = new CStringParameter( szCurrentToken );
	if ( !pStringParameter )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	return pStringParameter;
}

// ----------------------------------------------------------------------------
//
// Name:        GetValueParameter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static CValueParameter *GetValueParameter( CFileInfo *pFileInfo )
{
	ACTION_GetNextToken( pFileInfo );
	if ( !IsNumeric( szCurrentToken ) )
	{
		ACTION_SetLastError( ACTION_ERROR_EXPECTED_NUMERALS );
		return NULL;
	}
	CValueParameter *pValueParameter = new CValueParameter( (float)atof(szCurrentToken) );
	if ( !pValueParameter )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	return pValueParameter;	
}

// ----------------------------------------------------------------------------
//
// Name:        GetTextParameter
// Description:
//              gets a string enclosed in quotations
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static CTextParameter *GetTextParameter( CFileInfo *pFileInfo )
{
	ACTION_GetText( pFileInfo );
    int nLastError = ACTION_GetLastError();
    if ( nLastError == ACTION_ERROR_EXPECTED_QUOTE ||
         nLastError == ACTION_ERROR_EXCEEDED_QUOTE_LIMIT )
    {
        return NULL;
    }

	CTextParameter *pTextParameter = new CTextParameter( szCurrentText );
	if ( !pTextParameter )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	return pTextParameter;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseAnimate
// Description:
// Input:
// Output:
// Note:
//			syntax: animate( arg1 )
//					arg1 => the name of a animation sequence
//					arg2 => optional, defines the number of seconds to play this sequence
//							if left blank, it will play the sequence only once
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseAnimate( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_ANIMATE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	
	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma(szCurrentToken) )
	{
		CValueParameter *pValueParameter = GetValueParameter( pFileInfo );
		if ( !pValueParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pValueParameter );

		// get closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseAnimatePartial
// Description:
// Input:
// Output:
// Note:
//		syntax:	animate_partial( arg1, arg2, arg3 )
//				arg1 => name of animation
//				arg2 => starting index
//				arg3 => ending index (optional, if left blank, plays to the end
//						of sequence
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseAnimatePartial( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_ANIMATEPARTIAL );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	CValueParameter *pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		pValueParameter = GetValueParameter( pFileInfo );
		if ( !pValueParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pValueParameter );

		// get closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		ACTION_SetLastError( ACTION_ERROR_NO_CLOSE_PAREN );
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSound
// Description:
// Input:
// Output:
// Note:
//		syntax: sound( arg1, arg2 );
//				arg1 => name of the sound
//				arg2 => [optional] uniqueid
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseSound( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SOUND );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetFileNameParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

#ifndef CHECK_SYNTAX
	gstate->SoundIndex( pStringParameter->GetString() );  // pre-cache the sound ????? 
#endif CHECK_SYNTAX
	
	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		pStringParameter = GetStringParameter( pFileInfo );
		if ( !pStringParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pStringParameter );

		// get a closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSpawn
// Description:
// Input:
// Output:
// Note:
//		syntax: spawn( arg1, arg2, x, y, z, arg6, arg7, arg8 );
//				arg1 => class name of an AI unit to spawn
//				arg2 => uniqueid
//				arg6 => facingAngle
//				arg7 => (string) flag (true or false) indicating whether or not to respawn after death
//				arg8 => optional, if not specified, the spawned unit goes into idle state,
//						if specified, the specified action should be a script action
//              arg9 => optional, if specified, expects a skin texture
//                      arg8 must be set before setting this arg
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseSpawn( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SPAWN );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// get class name
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// get unique id
	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}
	
	// x
	CValueParameter *pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// y
	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// z
	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// facing angle
	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// true or false
	pValueParameter = GetTrueOrFalse( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		// get script actions
        pStringParameter = GetStringParameter( pFileInfo );
		if ( !pStringParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pStringParameter );

		// get a closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	    if ( IsComma( szCurrentToken ) )
	    {
		    // get skin texture
            pStringParameter = GetStringParameter( pFileInfo );
		    if ( !pStringParameter )
		    {
			    return NULL;
		    }
		    pNewAction->AddParameter( pStringParameter );

		    // get a closed parenthesis
		    ACTION_GetNextToken( pFileInfo );
	    }

	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseMoveTo
// Description:
// Input:
// Output:
// Note:
//		syntax:	move_to( x, y, z );
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseMoveTo( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_MOVETO );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// x
	CValueParameter *pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// y
	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// z
	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseFaceAngle
// Description:
// Input:
// Output:
// Note:
//		syntax:	face_angle( pitch, yaw, roll );
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseFaceAngle( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_FACEANGLE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CValueParameter *pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pValueParameter = GetValueParameter( pFileInfo );
	if ( !pValueParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pValueParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseUse
// Description:
// Input:
// Output:
// Note:
//		syntax:	use( arg1 );
//				arg1 => unique identifier of an AI unit
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseUse( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_USE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	
	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSendMessage
// Description:
// Input:
// Output:
// Note:
//		syntax:	send_message( arg1, arg2 );
//				arg1 => unique identifier
//				arg2 => script action name
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseSendMessage( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SENDMESSAGE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSetMovingAnimation
// Description:
// Input:
// Output:
// Note:
//		syntax:	set_moving_animation( arg1 );
//				arg1 => animation name
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseSetMovingAnimation( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SETMOVINGANIMATION );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSetState
// Description:
// Input:
// Output:
// Note:
//		syntax:	set_state( arg1, arg2, arg3 );
//				arg1 => unique id
//				arg2 => a state, currently the following are recognized
//						- ignore_player
//						- agressive
//						- pathfollow
//				arg3 => optional, if specified expects a target name if arg2
//						is pathfollow
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseSetState( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SETSTATE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}

	if ( _stricmp( pStringParameter->GetString(), "pathfollow" ) != 0 &&
		 _stricmp( pStringParameter->GetString(), "ignore_player" ) != 0 &&
		 _stricmp( pStringParameter->GetString(), "aggressive" ) != 0 )
	{
		printf( "%s is not a recognized state.\n", szCurrentToken );
		ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		pStringParameter = GetStringParameter( pFileInfo );
		if ( !pStringParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pStringParameter );

		// get a closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseDie
// Description:
// Input:
// Output:
// Note:
//		syntax:	die( arg1, arg2 );
//				arg1 => unique id
//				arg2 => animation name to play
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseDie( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_DIE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// unique id
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );
	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	// animation name
	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseWhenUsedSequence
// Description:
// Input:
// Output:
// Note:
//		syntax:	#( arg1 );
//				arg1 => script name
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseWhenUsedSequence( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_WHENUSED_SEQUENCE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// script name
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseWhenUsedIdle
// Description:
// Input:
// Output:
// Note:
//		syntax:	idle( arg1, arg2, arg3, ... );
//				arg1 => script name
//				arg2 => script name
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseWhenUsedIdle( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_WHENUSED_IDLE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// script name
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedParen( szCurrentToken ) )
		{
			if ( !IsString( szCurrentToken ) )
			{
				ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
				return NULL;
			}
			CStringParameter *pStringParameter = new CStringParameter( szCurrentToken );
			if ( !pStringParameter )
			{
				ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
				return NULL;
			}
			pNewAction->AddParameter( pStringParameter );

			if ( !GetComma( pFileInfo ) )
			{
				break;
			}
		}
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseCall
// Description:
// Input:
// Output:
// Note:
//		syntax:	call( arg1 );
//				arg1 => script name
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseCall( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_CALL );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// script name
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseRandomScript
// Description:
// Input:
// Output:
// Note:
//		syntax:	random_script( arg1, arg2, arg3, ... );
//				arg1 => script name
//				arg2 => script name
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseRandomScript( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_RANDOMSCRIPT );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// script name
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedParen( szCurrentToken ) )
		{
			if ( !IsString( szCurrentToken ) )
			{
				ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
				return NULL;
			}
			CStringParameter *pStringParameter = new CStringParameter( szCurrentToken );
			if ( !pStringParameter )
			{
				ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
				return NULL;
			}
			pNewAction->AddParameter( pStringParameter );

			if ( !GetComma( pFileInfo ) )
			{
				break;
			}
		}
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseStreamSound
// Description:
// Input:
// Output:
// Note:
//		syntax: stream_sound( arg1, arg2 );
//				arg1 => name of the sound
//				arg2 => [optional] uniqueid
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseStreamSound( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_STREAMSOUND );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetFileNameParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		pStringParameter = GetStringParameter( pFileInfo );
		if ( !pStringParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pStringParameter );

		// get a closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseSendUrgentMessage
// Description:
// Input:
// Output:
// Note:
//		syntax:	send_urgent_message( arg1, arg2 );
//				arg1 => unique identifier
//				arg2 => script action name
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseSendUrgentMessage( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_SENDURGENTMESSAGE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseComeNear
// Description:
// Input:
// Output:
// Note:
//		syntax:	come_near( arg1, arg2 );
//				arg1 => first entity's unique identifier
//				arg2 => unique id of a second entity which will go near toward the
//						first entity
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseComeNear( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_COMENEAR );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseRemove
// Description:
// Input:
// Output:
// Note:
//		syntax:	remove( arg1 );
//				arg1 => unique id
//
// ----------------------------------------------------------------------------
CAction *ACTION_ParseRemove( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_REMOVE );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	// unique id
	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseLookAt
// Description:
// Input:
// Output:
// Note:
//		syntax:	look_at( arg1, arg2 );
//				arg1 => first entity's unique identifier, this will look at the second
//						entity
//				arg2 => second entity's unique id
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseLookAt( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_LOOKAT );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseStopLook
// Description:
// Input:
// Output:
// Note:
//		syntax:	stop_look( arg1 );
//				arg1 => unique identifier, stops looking
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseStopLook( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_STOPLOOK );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseAttack
// Description:
// Input:
// Output:
// Note:
//		syntax:	attack( arg1, arg2, arg3 );
//				arg1 => entity's unique identifier that will be attacked
//				arg2 => attack animation name
//              arg3 => optional, specifies a weapon if known
//                      if not specified, current weapon will be used whatever
//                      that may be
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParseAttack( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_ATTACK );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CStringParameter *pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	if ( !GetComma( pFileInfo ) )
	{
		return NULL;
	}

	pStringParameter = GetStringParameter( pFileInfo );
	if ( !pStringParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pStringParameter );

	// next token can be either a COMMA or a closed parenthesis
	ACTION_GetNextToken( pFileInfo );
	if ( IsComma( szCurrentToken ) )
	{
		pStringParameter = GetStringParameter( pFileInfo );
		if ( !pStringParameter )
		{
			return NULL;
		}
		pNewAction->AddParameter( pStringParameter );

		// get a closed parenthesis
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsClosedParen( szCurrentToken ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParsePrint
// Description:
// Input:
// Output:
// Note:
//		syntax:	print( "text" );
//				text => text enclosed in quotes
//
// ----------------------------------------------------------------------------
static CAction *ACTION_ParsePrint( CFileInfo *pFileInfo )
{
	CAction *pNewAction = new CAction( ACTIONTYPE_PRINT );
	if ( !pNewAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	if ( !GetOpenParen( pFileInfo ) )
	{
		return NULL;
	}

	CTextParameter *pTextParameter = GetTextParameter( pFileInfo );
	if ( !pTextParameter )
	{
		return NULL;
	}
	pNewAction->AddParameter( pTextParameter );

	if ( !GetClosedParen( pFileInfo ) )
	{
		return NULL;
	}

	if ( !GetEndStatement( pFileInfo ) )
	{
		return NULL;
	}

	return pNewAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_GetEndStatement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void ACTION_GetEndStatement( CFileInfo *pFileInfo )
{
	// eat end of statement char if there is one
	if ( ACTION_SkipWhiteSpaces( pFileInfo ) != EOF )
	{
		int ch = SCRIPT_fgetc( pFileInfo->GetFile() );
		if( ch != END_STATEMENT[0] )
		{
			SCRIPT_ungetc( ch, pFileInfo->GetFile() );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_FindPredefinedAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_FindPredefinedAction( char *szToken )
{
	for ( int i = 0; i < nNumPredefinedActions; i++ )
	{
		if ( _stricmp( szToken, aPredefinedActions[i].szAction ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static CScriptAction *ACTION_ParseScriptAction( CFileInfo *pFileInfo )
{
	// get the script action name
	ACTION_GetNextToken( pFileInfo );
	CScriptAction *pNewScriptAction = new CScriptAction( szCurrentToken, SCRIPTTYPE_ONCE, 1 );
	if ( !pNewScriptAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	ACTION_GetNextToken( pFileInfo );
	if ( !IsOpenBrace( szCurrentToken ) )
	{
		// get the unique identifier
		if ( !IsString( szCurrentToken ) )
		{
			ACTION_SetLastError( ACTION_ERROR_EXPECTED_STRING );
			return NULL;
		}
		pNewScriptAction->SetUniqueID( szCurrentToken );

		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsOpenBrace( szCurrentToken ) )
	{
		return NULL;
	}

	CAction *pAction = NULL;
	while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedBrace( szCurrentToken ) )
	{
		int nPredefinedAction = ACTION_FindPredefinedAction( szCurrentToken );
		if ( nPredefinedAction >= 0 )
		{
			pAction = aPredefinedActions[nPredefinedAction].fnParseAction( pFileInfo );
			if ( !pAction )
			{
				return NULL;
			}
			
			pNewScriptAction->AddAction( pAction );
		}
		else
		{
			ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );
			return NULL;
		}
	}

	if ( !IsClosedBrace( szCurrentToken ) )
	{
		return NULL;
	}

	ACTION_GetEndStatement( pFileInfo );

	return pNewScriptAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseLoopScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static CScriptAction *ACTION_ParseLoopScriptAction( CFileInfo *pFileInfo )
{
	// get the script action name
	ACTION_GetNextToken( pFileInfo );
	CScriptAction *pNewScriptAction = new CScriptAction( szCurrentToken, SCRIPTTYPE_LOOP );
	if ( !pNewScriptAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	ACTION_GetNextToken( pFileInfo );
	if ( !IsOpenBrace( szCurrentToken ) )
	{
		// get the unique identifier
		if ( IsString( szCurrentToken ) )
		{
			pNewScriptAction->SetUniqueID( szCurrentToken );
			ACTION_GetNextToken( pFileInfo );
		}

		if ( !IsOpenBrace( szCurrentToken ) )
		{
			// the number of times to loop
			if ( IsNumeric( szCurrentToken ) )
			{
				pNewScriptAction->SetLoopCount( atoi(szCurrentToken) );
				ACTION_GetNextToken( pFileInfo );
			}
		}
	}

	if ( !IsOpenBrace( szCurrentToken ) )
	{
		return NULL;
	}

	CAction *pAction = NULL;
	while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedBrace( szCurrentToken ) )
	{
		int nPredefinedAction = ACTION_FindPredefinedAction( szCurrentToken );
		if ( nPredefinedAction >= 0 )
		{
			pAction = aPredefinedActions[nPredefinedAction].fnParseAction( pFileInfo );
			if ( !pAction )
			{
				return NULL;
			}
			
			pNewScriptAction->AddAction( pAction );
		}
		else
		{
			ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );
			return NULL;
		}
	}

	if ( !IsClosedBrace( szCurrentToken ) )
	{
		return NULL;
	}

	ACTION_GetEndStatement( pFileInfo );

	return pNewScriptAction;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseWhenUsedCommand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static CWhenUsedCommand *ACTION_ParseWhenUsedCommand( CFileInfo *pFileInfo )
{
	CWhenUsedCommand *pNewWhenUsedCommand = new CWhenUsedCommand;
	if ( !pNewWhenUsedCommand )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	// get the script action name
	ACTION_GetNextToken( pFileInfo );
	pNewWhenUsedCommand->SetUniqueID( szCurrentToken );

    // get use delay time
	ACTION_GetNextToken( pFileInfo );
	if ( IsNumeric( szCurrentToken ) )
	{
		pNewWhenUsedCommand->SetUseDelay( (float)atof(szCurrentToken) );
		ACTION_GetNextToken( pFileInfo );
	}

	if ( !IsOpenBrace( szCurrentToken ) )
	{
		return NULL;
	}

	CAction *pAction = NULL;
	while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedBrace( szCurrentToken ) )
	{
		if ( IsInteger( szCurrentToken ) )
		{
			int nIndex = atoi( szCurrentToken );
			pAction = ACTION_ParseWhenUsedSequence( pFileInfo );
			if ( !pAction )
			{
				return NULL;
			}
			pNewWhenUsedCommand->AddAction( pAction, nIndex );
		}
		else
		if ( _stricmp( szCurrentToken, IDLE ) == 0 )
		{
			pAction = ACTION_ParseWhenUsedIdle( pFileInfo );
			if ( !pAction )
			{
				return NULL;
			}
			pNewWhenUsedCommand->SetIdleAction( pAction );
		}
		else
		{
			ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );
			return NULL;
		}
	}

	if ( !IsClosedBrace( szCurrentToken ) )
	{
		return NULL;
	}

	ACTION_GetEndStatement( pFileInfo );

	return pNewWhenUsedCommand;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_ParseLevelStart
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static CScriptAction *ACTION_ParseLevelStart( CFileInfo *pFileInfo )
{
	CScriptAction *pNewScriptAction = new CScriptAction( "level_start", SCRIPTTYPE_ONCE, 1 );
	if ( !pNewScriptAction )
	{
		ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
		return NULL;
	}

	ACTION_GetNextToken( pFileInfo );
	if ( !IsOpenBrace( szCurrentToken ) )
	{
		return NULL;
	}

	CAction *pAction = NULL;
	while ( ACTION_GetNextToken( pFileInfo ) > 0 && !IsClosedBrace( szCurrentToken ) )
	{
		if ( _stricmp( szCurrentToken, "send_message" ) != 0 )
		{
			ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );
			return NULL;
		}

		pAction = ACTION_ParseSendMessage( pFileInfo );
		if ( !pAction )
		{
			return NULL;
		}
		pNewScriptAction->AddAction( pAction );
	}

	if ( !IsClosedBrace( szCurrentToken ) )
	{
		return NULL;
	}

	ACTION_GetEndStatement( pFileInfo );

	return pNewScriptAction;
}

//*****************************************************************************
//								GLOBAL functions
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		ACTION_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_Init()
{
	nTokenLength = 0;
	strset( szCurrentToken, 0 );

	nLastError = ACTION_ERROR_NONE;
	
	if ( !SCRIPTACTION_Init() )
	{
		return FALSE;
	}

	if ( !WHENUSEDCOMMAND_Init() )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_End
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ACTION_End()
{
	SCRIPTACTION_Destroy();
	WHENUSEDCOMMAND_Destroy();

	if ( pLevelStartScript )
	{
		delete pLevelStartScript;
        pLevelStartScript = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_Parse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_Parse( CFileInfo *pFileInfo )
{
	int bSuccess = TRUE;
	while ( ACTION_GetNextToken( pFileInfo ) > 0 )
	{
		if ( _stricmp( szCurrentToken, INCLUDE ) == 0 )
		{
			ACTION_GetNextToken( pFileInfo );
			bSuccess = ACTION_Parse( szCurrentToken );
			if ( !bSuccess )
			{
				break;
			}
		}
		else
		if ( _stricmp( szCurrentToken, SCRIPT ) == 0 )
		{
			CScriptAction *pScriptAction = ACTION_ParseScriptAction( pFileInfo );
			if ( !pScriptAction )
			{
				bSuccess = FALSE;
				break;
			}

			SCRIPTACTION_Add( pScriptAction->GetName(), pScriptAction );
		}
		else
		if ( _stricmp( szCurrentToken, LOOP_SCRIPT ) == 0 )
		{
			CScriptAction *pScriptAction = ACTION_ParseLoopScriptAction( pFileInfo );
			if ( !pScriptAction )
			{
				bSuccess = FALSE;
				break;
			}

			SCRIPTACTION_Add( pScriptAction->GetName(), pScriptAction );
		}
		else
		if ( _stricmp( szCurrentToken, WHEN_USED ) == 0 )
		{
			CWhenUsedCommand *pWhenUsedCommand = ACTION_ParseWhenUsedCommand( pFileInfo );
			if ( !pWhenUsedCommand )
			{
				bSuccess = FALSE;
				break;
			}
			WHENUSEDCOMMAND_Add( pWhenUsedCommand->GetUniqueID(), pWhenUsedCommand );
		}
		else
		if ( _stricmp( szCurrentToken, LEVEL_START ) == 0 )
		{
			pLevelStartScript = ACTION_ParseLevelStart( pFileInfo );
			if ( !pLevelStartScript )
			{
				bSuccess = FALSE;
				break;
			}
		}
		else if ( _stricmp( szCurrentToken, LEVEL_EOF ) == 0 )
		{
			bSuccess = TRUE;
			break;
		}
		else
		{
			// expected an action token
			ACTION_SetLastError( ACTION_ERROR_UNEXPECTED_TOKEN );

			bSuccess = FALSE;
			break;
		}
	}

	if ( !bSuccess )
	{
		int nError = ACTION_GetLastError();
		int nErrorLineNumber = pFileInfo->GetCurrentLineNumber();

#ifdef CHECK_SYNTAX
		printf( "%s: FAILED Syntax check.\n", pFileInfo->GetFileName() );
		printf( "%s at line #: %d.\n", actionErrors[nError].szErrorString, nErrorLineNumber );
#else
#ifdef WIN32
		_RPT3( _CRT_ASSERT, "%s: %s at line #: %d.", pFileInfo->GetFileName(), actionErrors[nError].szErrorString, nErrorLineNumber );
#endif
#endif CHECK_SYNTAX

	}

	return bSuccess;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_Parse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int ACTION_Parse( const char *szFileName )
{
	int bSuccess = TRUE;;
	
	char szPath[128];
    strcpy( szPath, szScriptPath );
    strcat( szPath, szFileName );

    FILE *file = SCRIPT_fopen( szPath, "rb" );
	if ( file )
	{
		CFileInfo *pFileInfo = new CFileInfo( file, szPath );
		if ( !pFileInfo )
		{
			ACTION_SetLastError( ACTION_ERROR_MEM_FAILED );
			return FALSE;
		}

		bSuccess = ACTION_Parse( pFileInfo );

		delete pFileInfo;

		SCRIPT_fclose( file );

	    SCRIPTACTION_Verify();
    }
	else
	{
		bSuccess = FALSE;
		ACTION_End();

#ifdef CHECK_SYNTAX
		printf( "%s does not exist.\n", szPath );
#else CHECK_SYNTAX
		gstate->Con_Dprintf( "%s does not exist.\n", szPath ); 
#endif CHECK_SYNTAX

	}

	return bSuccess;
}

#ifndef CHECK_SYNTAX

// ----------------------------------------------------------------------------
//
// Name:		ACTION_Start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ACTION_Start()
{
	// get the correct file depending on the current map
	char szScriptActionFileName[64];
	strcpy( szScriptActionFileName, gstate->mapName );
	strcat( szScriptActionFileName, ".sca" );

	if ( !ACTION_Init() )
	{
		return FALSE;
	}

	return ACTION_Parse( szScriptActionFileName );
}

#endif CHECK_SYNTAX

// ----------------------------------------------------------------------------
//
// Name:		ACTION_CheckScriptSyntax
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ACTION_CheckScriptSyntax( const char *szFileName )
{
	if ( !ACTION_Init() )
	{
		return FALSE;
	}

	int bRetValue = ACTION_Parse( szFileName );

	ACTION_End();
	if ( bRetValue )
	{
		printf( "%s: PASSED Syntax check.\n", szFileName );
	}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		ACTION_GetLevelStartScript
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CScriptAction *ACTION_GetLevelStartScript()
{
	return pLevelStartScript;
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
