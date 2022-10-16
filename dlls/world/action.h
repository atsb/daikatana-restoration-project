
#ifndef _ACTION_H
#define _ACTION_H

#include "collect.h"

// ----------------------------------------------------------------------------

#define PARAMETERTYPE_UNDEFINED	0
#define PARAMETERTYPE_VALUE		1
#define PARAMETERTYPE_STRING	2
#define PARAMETERTYPE_TEXT      3


// predefined actions
#define ACTIONTYPE_UNDEFINED			0
#define ACTIONTYPE_ANIMATE				1
#define ACTIONTYPE_ANIMATEPARTIAL		2
#define ACTIONTYPE_SOUND				3
#define ACTIONTYPE_SPAWN				4
#define ACTIONTYPE_MOVETO				5
#define ACTIONTYPE_FACEANGLE			6
#define ACTIONTYPE_USE					7
#define ACTIONTYPE_SENDMESSAGE			8
#define ACTIONTYPE_SETMOVINGANIMATION	9
#define ACTIONTYPE_SETSTATE				10
#define ACTIONTYPE_DIE					11
#define ACTIONTYPE_WHENUSED_SEQUENCE	12
#define ACTIONTYPE_WHENUSED_IDLE		13
#define ACTIONTYPE_CALL					14
#define ACTIONTYPE_RANDOMSCRIPT			15
#define ACTIONTYPE_STREAMSOUND			16
#define ACTIONTYPE_SENDURGENTMESSAGE	17
#define ACTIONTYPE_COMENEAR				18
#define ACTIONTYPE_REMOVE				19
#define ACTIONTYPE_LOOKAT				20
#define ACTIONTYPE_STOPLOOK				21
#define ACTIONTYPE_ATTACK               22
#define ACTIONTYPE_PRINT                23

// script types
#define SCRIPTTYPE_ONCE				0		// executes once
#define SCRIPTTYPE_LOOP				1		// script loops a specified amount

// ----------------------------------------------------------------------------

class CParameter
{
private:
	int nParameterType;

public:
	CParameter();
	CParameter( int nType );
	~CParameter();

//	void* CParameter::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CParameter::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CParameter::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CParameter::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	virtual float GetValue()	{ return 0; }		// pure virtual functions
	virtual char *GetString()	{ return NULL; }
    virtual char *GetText()     { return NULL; }

	void SetParameterType( int nNewType )
	{
		nParameterType = nNewType;
	}
	int GetParameterType()		{ return nParameterType; }
};

// ----------------------------------------------------------------------------

class CValueParameter : public CParameter
{
private:
	float fValue;
		
public:
	CValueParameter();
	CValueParameter( float fNewValue );
	~CValueParameter();

	float GetValue()					{ return fValue; }
	void SetValue( float fNewValue )	{ fValue = fNewValue; }
};

// ----------------------------------------------------------------------------

class CStringParameter : public CParameter
{
private:
	char szString[64];

public:
	CStringParameter();
	CStringParameter( char *newString );
	~CStringParameter();

	char *GetString()		{ return szString; }
	void SetString( char *newString )
	{
		strcpy( szString, newString );
	}
};

// ----------------------------------------------------------------------------

class CTextParameter : public CParameter
{
private:
	char *pszText;

public:
	CTextParameter();
	CTextParameter( char *szNewText );
	~CTextParameter();

	char *GetText()		{ return pszText; }
	void SetText( char *szNewText );
};

// ----------------------------------------------------------------------------

class CAction
{
private:
	int nActionType;
	CPtrList parameterList;

public:
	CAction();
	CAction( int nType );
	~CAction();

//	void* CAction::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CAction::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CAction::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CAction::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void SetActionType( int nType )
	{
		nActionType = nType;
	}
	int GetActionType()			{ return nActionType; }

	void AddParameter( CParameter *pNewParameter );
	CParameter *GetParameter( int nIndex );

	int GetNumParameters()		{ return parameterList.GetCount(); }
};

// ----------------------------------------------------------------------------

class CScriptAction
{
private:
	int	nScriptType;
	int	nLoopCount;
	char szName[32];			// name of this script
	char szUniqueID[32];		// unique identifier for an AI
	CPtrList actionList;

public:
	CScriptAction();
	CScriptAction( char *szNewName, int nType = SCRIPTTYPE_ONCE, int nValue = -1 );
	~CScriptAction();

//	void* CScriptAction::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CScriptAction::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CScriptAction::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CScriptAction::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void Destroy();

	int GetLoopCount()					{ return nLoopCount; }
	void SetLoopCount( int nValue )		{ nLoopCount = nValue; }
	int GetType()						{ return nScriptType; }
	void SetType( int nType )			{ nScriptType = nType; }

	char *GetName()						{ return szName; }
	void SetName( char *szString )		{ strcpy( szName, szString ); }

	char *GetUniqueID()					{ return szUniqueID; }
	void SetUniqueID( char *szString )	{ strcpy( szUniqueID, szString ); }

	void AddAction( CAction *pNewAction );
	CPtrList *GetActionList()			{ return &actionList; }
};

// ----------------------------------------------------------------------------

class CWhenUsedCommand
{
private:
	char szUniqueID[32];		// unique identifier for an AI
    float use_delay;            // ignore next 'use' timer
    float use_time;             // track last 'use' time

	int nNumUsed;
	CPtrList actionList;
	CAction *pIdleAction;

public:
	CWhenUsedCommand();
	~CWhenUsedCommand();

//	void* CWhenUsedCommand::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CWhenUsedCommand::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CWhenUsedCommand::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CWhenUsedCommand::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Destroy();

	void UsedOnce()						{ nNumUsed++; }
	int GetNumUsed()					{ return nNumUsed; }

	char *GetUniqueID()					 { return szUniqueID; }
	void SetUniqueID( char *szString )	 { strcpy( szUniqueID, szString ); }
    void SetUseDelay(float delay_time )  { use_delay = delay_time; }
    void SetUseTime(float engage_time )  { use_time = engage_time; }

	void AddAction( CAction *pNewAction, int nIndex = -1 );
	CAction *GetAction( int nIndex );
	CAction *GetCurrentAction();
	int GetNumActions()					{ return actionList.GetCount(); }
    float GetUseDelay()                 { return use_delay; }
    float GetUseTime()                  { return use_time; }

	void SetIdleAction( CAction *pNewAction )	{ pIdleAction = pNewAction; }
	CAction *GetIdleAction()			{ return pIdleAction; }
};

// ----------------------------------------------------------------------------

CScriptAction *SCRIPTACTION_Lookup( const char *szScriptActionName );

#ifndef CHECK_SYNTAX

userEntity_t *UNIQUEID_Lookup( const char *szUniqueID );
void UNIQUEID_Add( const char *szUniqueID, userEntity_t *pEntity );
void UNIQUEID_Remove( const char *szUniqueID );
void UNIQUEID_RemoveEnt( userEntity_t *self );
int UNIQUEID_Init();
void UNIQUEID_Destroy();

#endif CHECK_SYNTAX

CWhenUsedCommand *WHENUSECOMMAND_Lookup( const char *szUniqueID );

void ACTION_End();
int ACTION_Start();

int ACTION_CheckScriptSyntax( const char *szFileName );
CScriptAction *ACTION_GetLevelStartScript();

#endif _ACTION_H
