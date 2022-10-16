// ==========================================================================
//
//  File:		AI_File.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_utils.h"
#include "ai_func.h"
#include "filebuffer.h"
//#include "decode.h"// SCG[1/23/00]: not used
#include "dk_io.h"
#include "csv.h"
#include "collect.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MAX_NUM_MONSTERS	100

class CAIAttributes
{
private:
	char  szName[64];

	char  szModel[64];
    char  szCSVFile[64];
    float health;
	float base_health;
	float elasticity;
	float mass;

	CVector ang_speed;
	CVector mins;
	CVector maxs;
	CVector render_scale;

	float active_distance;
	float attack_dist;
	float jump_attack_dist;

	float upward_vel;
	float run_speed;
	float walk_speed;
	float attack_speed;

	float fov;

	WEAPON_ATTRIBUTE aWeaponAttributes[3];

	CPtrList soundList;

    float fMinAttenuation;
    float fMaxAttenuation;

public:
	CAIAttributes();
	CAIAttributes( char *newName );
	~CAIAttributes();

	void Init();
	void SetName( char *newName );
    void SetModelName( char *szString )     { strcpy( szModel, szString ); }
    void SetCSVFileName( char *szString )   { strcpy( szCSVFile, szString ); }
	void SetHealth( float fNewValue )		{ health = fNewValue; }
	void SetBaseHealth( float fNewValue )	{ base_health = fNewValue; }
	void SetElasticity( float fNewValue )	{ elasticity = fNewValue; }
	void SetMass( float fNewValue )			{ mass = fNewValue; }
	void SetAngleSpeed( float x, float y, float z )	{ ang_speed.Set( x, y, z ); }
	void SetMinX( float x )					{ mins.x = x; }
	void SetMinY( float y )					{ mins.y = y; }
	void SetMinZ( float z )					{ mins.z = z; }
	void SetMaxX( float x )					{ maxs.x = x; }
	void SetMaxY( float y )					{ maxs.y = y; }
	void SetMaxZ( float z )					{ maxs.z = z; }
	void SetRenderScale( float x, float y, float z ) { render_scale.Set( x, y, z ); }

	void SetActiveDistance( float fNewValue )	{ active_distance = fNewValue; }
	void SetAttackDistance( float fNewValue )	{ attack_dist = fNewValue; }
	void SetJumpAttackDistance( float fNewValue )	{ jump_attack_dist = fNewValue; }
	void SetUpwardVelocity( float fNewValue )	{ upward_vel = fNewValue; }
	void SetRunSpeed( float fNewValue )			{ run_speed = fNewValue; }
	void SetWalkSpeed( float fNewValue )		{ walk_speed = fNewValue; }
	void SetAttackSpeed( float fNewValue )		{ attack_speed = fNewValue; }
	void SetFOV( float fNewValue )				{ fov = fNewValue; }

	void SetWeaponOffsetX( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].offset.x = fValue;
	}
	void SetWeaponOffsetY( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].offset.y = fValue;
	}
	void SetWeaponOffsetZ( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].offset.z = fValue;
	}
	void SetWeaponBaseDamage( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fBaseDamage = fValue;
	}
	void SetWeaponRandomDamage( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fRandomDamage = fValue;
	}
	void SetWeaponSpreadX( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fSpreadX = fValue;
	}
	void SetWeaponSpreadZ( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fSpreadZ = fValue;
	}
	void SetWeaponSpeed( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fSpeed = fValue;
	}
	void SetWeaponDistance( int nIndex, float fValue )
	{
		aWeaponAttributes[nIndex].fDistance = fValue;
	}
    void SetMinAttenuation( float fNewValue )
    {
        fMinAttenuation = fNewValue;
    }
    void SetMaxAttenuation( float fNewValue )
    {
        fMaxAttenuation = fNewValue;
    }

	char *GetName()			{ return szName; }
    char *GetModelName()    { return szModel; }
    char *GetCSVFileName()  { return szCSVFile; }
	float GetHealth()		{ return health; }
	float GetBaseHealth()	{ return base_health; }
	float GetElasticity()	{ return elasticity; }
	float GetMass()			{ return mass; }
	CVector &GetAngleSpeed(){ return ang_speed; }
	CVector &GetMins()		{ return mins; }
	CVector &GetMaxs()		{ return maxs; }
	CVector &GetRenderScale()	{ return render_scale; }

	float GetActiveDistance()	{ return active_distance; }
	float GetAttackDistance()	{ return attack_dist; }
	float GetJumpAttackDistance()	{ return jump_attack_dist; }
	float GetUpwardVelocity()	{ return upward_vel; }
	float GetRunSpeed()			{ return run_speed; }
	float GetWalkSpeed()		{ return walk_speed; }
	float GetAttackSpeed()		{ return attack_speed; }
	float GetFOV()				{ return fov; }

	CVector &GetWeaponOffset( int nIndex )
	{
		return aWeaponAttributes[nIndex].offset;
	}
	float GetWeaponBaseDamage( int nIndex )
	{
		return aWeaponAttributes[nIndex].fBaseDamage;
	}
	float GetWeaponRandomDamage( int nIndex )
	{
		return aWeaponAttributes[nIndex].fRandomDamage;
	}
	float GetWeaponSpreadX( int nIndex )
	{
		return aWeaponAttributes[nIndex].fSpreadX;
	}
	float GetWeaponSpreadZ( int nIndex )
	{
		return aWeaponAttributes[nIndex].fSpreadZ;
	}
	float GetWeaponSpeed( int nIndex )
	{
		return aWeaponAttributes[nIndex].fSpeed;
	}
	float GetWeaponDistance( int nIndex )
	{
		return aWeaponAttributes[nIndex].fDistance;
	}

    WEAPON_ATTRIBUTE_PTR GetWeaponAttributes()		{ return aWeaponAttributes; }
    
    float GetMinAttenuation()       { return fMinAttenuation; }
    float GetMaxAttenuation()       { return fMaxAttenuation; }

	void AddSound( char *pSound );
	void CacheSounds();
};

/* ***************************** Local Variables *************************** */

static int nNumMonsters = 0;
static CAIAttributes aAIAttributes[MAX_NUM_MONSTERS];

static CMapStringToPtr mapNameToAttributes;

//static int bUsingFile = TRUE;// SCG[1/23/00]: not used
//static CFileBuffer *pFileBuffer = NULL;// SCG[1/23/00]: not used
static FILE *file = NULL;

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
/* ***************************** Local Functions *************************** */


// ----------------------------------------------------------------------------

CAIAttributes::CAIAttributes()
{
	Init();
}

CAIAttributes::CAIAttributes( char *newName )
{
	Init();
	SetName( newName );
}

CAIAttributes::~CAIAttributes()
{
	POSITION pos = soundList.GetHeadPosition();
	while ( pos )
	{
		char *pSound = (char *)soundList.GetNext( pos );
		if ( pSound )
		{
			gstate->X_Free(pSound);
			//delete [] pSound;
		}
	}

	soundList.RemoveAll();
}

void CAIAttributes::Init()
{
	szName[0] = '\0';
	
	health			= 100;
	base_health		= 100;
	elasticity		= 0.5f;
	mass			= 1.0f;

	ang_speed.Set( 90.0f, 135.0f, 90.0f );
	mins.Set( -16.0f, -16.0f, -24.0f );
	maxs.Set( 16.0f, 16.0f, 32.0f );

	active_distance = 2000.0f;
	attack_dist		= 250.0f;
	jump_attack_dist = 250.0f;

	upward_vel		= 270.0f;
	run_speed		= 125.0f;
	walk_speed		= 50.0f;
	attack_speed	= 125.0f;

	fov				= 215.0f;

	for ( int i = 0; i < 3; i++ )
	{
		aWeaponAttributes[i].offset.Zero();
		aWeaponAttributes[i].fBaseDamage	= 0.0f;
		aWeaponAttributes[i].fRandomDamage	= 0.0f;
		aWeaponAttributes[i].fSpreadX		= 0.0f;
		aWeaponAttributes[i].fSpreadZ		= 0.0f;
		aWeaponAttributes[i].fSpeed			= 0.0f;
		aWeaponAttributes[i].fDistance		= 0.0f;
	}

    fMinAttenuation   = ATTN_NORM_MIN;
    fMaxAttenuation   = ATTN_NORM_MAX;

}

// ----------------------------------------------------------------------------
//
// Name:		SetName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAIAttributes::SetName( char *newName )
{
	strcpy( szName, newName );
}

void CAIAttributes::AddSound( char *pSound )
{ 
	if ( pSound && strlen(pSound) > 0 )
	{
		soundList.AddTail( pSound ); 
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CacheSounds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAIAttributes::CacheSounds()
{
	POSITION pos = soundList.GetHeadPosition();
	while ( pos )
	{
		char *pSound = (char *)soundList.GetNext( pos );
		if ( pSound && strlen(pSound) > 0 )
		{
			gstate->SoundIndex( pSound );
		}
	}
}




// ****************************************************************************
// ****************************************************************************


static CAIAttributes *AIATTRIBUTE_Lookup( const char *szClassName )
{
	if ( !szClassName || strlen( szClassName ) == 0 )
	{
		return NULL;
	}

	char *pszTemp = _strlwr( _strdup(szClassName) );
	void *pValue = NULL;
    mapNameToAttributes.Lookup(pszTemp, pValue);
    CAIAttributes *pAttributes = (CAIAttributes*)pValue;
    free( pszTemp );

	return pAttributes;
}

static void AIATTRIBUTE_Add( const char *szClassName, CAIAttributes *pAttributes )
{
	char *pszTemp = _strlwr( _strdup(szClassName) );

    CAIAttributes *pTempAttributes = AIATTRIBUTE_Lookup( pszTemp );
	if ( pTempAttributes )
	{
		return;
	}
	mapNameToAttributes.SetAt( pszTemp, pAttributes );

    free( pszTemp );
}

static void AIATTRIBUTE_Remove( const char *szClassName )
{
	if ( szClassName )
	{
		mapNameToAttributes.RemoveKey( szClassName );
	}
}

static int AIATTRIBUTE_Init()
{
    nNumMonsters = 0;
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        AIATTRIBUTE_Destroy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AIATTRIBUTE_Destroy()
{
	if ( !mapNameToAttributes.IsEmpty() )
    {
        mapNameToAttributes.RemoveAll();
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AIATTRIBUTE_GetNextElement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static CAIAttributes *AIATTRIBUTE_GetNextElement()
{
	_ASSERTE( nNumMonsters < MAX_NUM_MONSTERS );

	CAIAttributes *pAIAttribute = &(aAIAttributes[nNumMonsters]);
	nNumMonsters++;

	return pAIAttribute;
}

// ----------------------------------------------------------------------------
//
// Name:		AIATTRIBUTE_ParseData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define ATTRIBUTE_MODELNAME             0
#define ATTRIBUTE_CSVFILE               1
#define ATTRIBUTE_HEALTH			    2
#define ATTRIBUTE_BASEHEALTH		    3
#define ATTRIBUTE_ELASTICITY		    4
#define ATTRIBUTE_MASS				    5
#define ATTRIBUTE_ANGLESPEED		    6
#define ATTRIBUTE_SIZE1				    7
#define ATTRIBUTE_SIZE2				    8
#define ATTRIBUTE_SIZE3				    9
#define ATTRIBUTE_SIZE4				    10
#define ATTRIBUTE_SIZE5				    11
#define ATTRIBUTE_SIZE6				    12
#define ATTRIBUTE_RENDERSCALE		    13
#define ATTRIBUTE_ACTIVEDIST		    14
#define ATTRIBUTE_ATTACKDIST		    15
#define ATTRIBUTE_JUMPATTACKDIST	    16
#define ATTRIBUTE_UPWARDVELOCITY	    17
#define ATTRIBUTE_RUNSPEED			    18
#define ATTRIBUTE_WALKSPEED			    19
#define ATTRIBUTE_ATTACKSPEED		    20
#define ATTRIBUTE_FOV				    21
#define ATTRIBUTE_WEAPON1_OFFSET_X		22
#define ATTRIBUTE_WEAPON1_OFFSET_Y		23
#define ATTRIBUTE_WEAPON1_OFFSET_Z		24
#define ATTRIBUTE_WEAPON1_BASEDAMAGE	25
#define ATTRIBUTE_WEAPON1_RANDOMDAMAGE	26
#define ATTRIBUTE_WEAPON1_SPREADX		27
#define ATTRIBUTE_WEAPON1_SPREADZ		28
#define ATTRIBUTE_WEAPON1_SPEED			29
#define ATTRIBUTE_WEAPON1_DISTANCE		30
#define ATTRIBUTE_WEAPON2_OFFSET_X		31
#define ATTRIBUTE_WEAPON2_OFFSET_Y		32
#define ATTRIBUTE_WEAPON2_OFFSET_Z		33
#define ATTRIBUTE_WEAPON2_BASEDAMAGE	34
#define ATTRIBUTE_WEAPON2_RANDOMDAMAGE	35
#define ATTRIBUTE_WEAPON2_SPREADX		36
#define ATTRIBUTE_WEAPON2_SPREADZ		37
#define ATTRIBUTE_WEAPON2_SPEED			38
#define ATTRIBUTE_WEAPON2_DISTANCE		39
#define ATTRIBUTE_WEAPON3_OFFSET_X		40
#define ATTRIBUTE_WEAPON3_OFFSET_Y		41
#define ATTRIBUTE_WEAPON3_OFFSET_Z		42
#define ATTRIBUTE_WEAPON3_BASEDAMAGE	43
#define ATTRIBUTE_WEAPON3_RANDOMDAMAGE	44
#define ATTRIBUTE_WEAPON3_SPREADX		45
#define ATTRIBUTE_WEAPON3_SPREADZ		46
#define ATTRIBUTE_WEAPON3_SPEED			47
#define ATTRIBUTE_WEAPON3_DISTANCE		48
#define ATTRIBUTE_MIN_ATTENUATION       49
#define ATTRIBUTE_MAX_ATTENUATION       50


static void AIATTRIBUTE_ParseData( CAIAttributes *pAIAttributes, char *szData, int nCurrentDataIndex )
{
	int nValue = -1;
	float fValue = -1.0f;
    char szString[64];
	CVector Skill_Multiplier;
	//NSS[11/6/99]:
	//Skill 0 = 75%  of normal health/damage settings
	//Skill 1 = 100% of normal health/damage settings
	//Skill 2 = 125% of normal health/damage settings

	//Set values
	Skill_Multiplier.Set(0.75f,1.0,1.25f);
	
	//Get values
	float Multiplier	= AI_GetSkill(Skill_Multiplier);
	float SpeedM,VitalityM,PowerM;

	// NSS[3/6/00]:Skills based off of how many clients in the game.
	PowerM = VitalityM = SpeedM	= 1.0;

	if( PowerM < gstate->game->PowerM )
		PowerM		= gstate->game->PowerM;
	if( VitalityM < gstate->game->VitalityM)
		VitalityM	= gstate->game->VitalityM;
	if( SpeedM < gstate->game->SpeedM)
		SpeedM		= gstate->game->SpeedM;
	
	switch ( nCurrentDataIndex )
	{
        case ATTRIBUTE_MODELNAME:
        {
			int nRetValue = sscanf( szData, "%s", szString );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetModelName( szString );
			break;
        }
        case ATTRIBUTE_CSVFILE:
        {
			int nRetValue = sscanf( szData, "%s", szString );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetCSVFileName( szString );
            break;
        }
		case ATTRIBUTE_HEALTH:
		{
			
			
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:// NSS[3/6/00]:Added the increase in difficulty dependent upon skill level
			pAIAttributes->SetHealth( VitalityM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_BASEHEALTH:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:// NSS[3/6/00]:Added the increase in difficulty dependent upon skill level
			pAIAttributes->SetBaseHealth( VitalityM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_ELASTICITY:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetElasticity( fValue );
			break;
		}
		case ATTRIBUTE_MASS:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMass( fValue );
			break;
		}
		case ATTRIBUTE_ANGLESPEED:
		{
			float fPitch, fYaw, fRoll;
			int nRetValue = sscanf( szData, "%f %f %f", &fPitch, &fYaw, &fRoll );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetAngleSpeed( fPitch, fYaw, fRoll );
			break;
		}
		case ATTRIBUTE_SIZE1:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMinX( fValue );
			break;
		}
		case ATTRIBUTE_SIZE2:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMinY( fValue );
			break;
		}
		case ATTRIBUTE_SIZE3:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMinZ( fValue );
			break;
		}
		case ATTRIBUTE_SIZE4:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMaxX( fValue );
			break;
		}
		case ATTRIBUTE_SIZE5:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMaxY( fValue );
			break;
		}
		case ATTRIBUTE_SIZE6:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetMaxZ( fValue );
			break;
		}
		case ATTRIBUTE_RENDERSCALE:
		{
			float fX, fY, fZ;
			int nRetValue = sscanf( szData, "%f %f %f", &fX, &fY, &fZ );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetRenderScale( fX, fY, fZ );
			break;
		}
		case ATTRIBUTE_ACTIVEDIST:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetActiveDistance( Multiplier*fValue );
			break;
		}
		case ATTRIBUTE_ATTACKDIST:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetAttackDistance(fValue );
			break;
		}
		case ATTRIBUTE_JUMPATTACKDIST:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetJumpAttackDistance(fValue );
			break;
		}
		case ATTRIBUTE_UPWARDVELOCITY:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			
			pAIAttributes->SetUpwardVelocity( fValue );
			break;
		}
		case ATTRIBUTE_RUNSPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetRunSpeed( SpeedM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WALKSPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWalkSpeed( SpeedM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_ATTACKSPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetAttackSpeed( Multiplier*fValue );
			break;
		}
		case ATTRIBUTE_FOV:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetFOV( fValue );
			break;
		}
		
		case ATTRIBUTE_WEAPON1_OFFSET_X:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetX( 0, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_OFFSET_Y:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetY( 0, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_OFFSET_Z:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetZ( 0, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_BASEDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponBaseDamage( 0,PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON1_RANDOMDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponRandomDamage( 0, PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON1_SPREADX:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadX( 0, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_SPREADZ:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadZ( 0, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_SPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponSpeed( 0, Multiplier*fValue );
			break;
		}
		case ATTRIBUTE_WEAPON1_DISTANCE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponDistance( 0, fValue );
			break;
		}

		case ATTRIBUTE_WEAPON2_OFFSET_X:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetX( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_OFFSET_Y:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetY( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_OFFSET_Z:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetZ( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_BASEDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponBaseDamage( 1, PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON2_RANDOMDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponRandomDamage( 1, PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON2_SPREADX:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadX( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_SPREADZ:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadZ( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_SPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpeed( 1, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON2_DISTANCE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponDistance( 1, fValue );
			break;
		}

		case ATTRIBUTE_WEAPON3_OFFSET_X:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetX( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_OFFSET_Y:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetY( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_OFFSET_Z:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponOffsetZ( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_BASEDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponBaseDamage( 2, PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON3_RANDOMDAMAGE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}
			//NSS[11/6/99]:
			pAIAttributes->SetWeaponRandomDamage( 2, PowerM*(Multiplier*fValue) );
			break;
		}
		case ATTRIBUTE_WEAPON3_SPREADX:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadX( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_SPREADZ:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpreadZ( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_SPEED:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponSpeed( 2, fValue );
			break;
		}
		case ATTRIBUTE_WEAPON3_DISTANCE:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			pAIAttributes->SetWeaponDistance( 2, fValue );
			break;
		}
        case ATTRIBUTE_MIN_ATTENUATION:
        {
            int nRetValue = sscanf( szData, "%f", &fValue );
            if ( nRetValue <= 0 )
            {
                return;
            }
            pAIAttributes->SetMinAttenuation( fValue );
            break;
        }
        case ATTRIBUTE_MAX_ATTENUATION:
        {
            int nRetValue = sscanf( szData, "%f", &fValue );
            if ( nRetValue <= 0 )
            {
                return;
            }
            pAIAttributes->SetMaxAttenuation( fValue );
            break;
        }

		default:
			break;
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AIATTRIBUTE_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AIATTRIBUTE_ReadFile( const char *szFileName )
{
    AIATTRIBUTE_Init();

	int bSuccess = TRUE;
	
	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumAttributes = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				CAIAttributes *pAIAttributes = AIATTRIBUTE_GetNextElement();
				if ( pAIAttributes )
				{
					pAIAttributes->SetName( szElement );
                    AIATTRIBUTE_Add( szElement, pAIAttributes );

					int nNumElements = 0;
					while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
					{
						AIATTRIBUTE_ParseData( pAIAttributes, szElement, nNumElements );
						nNumElements++;	
					}
			
					nNumAttributes++;
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
// Name:        AIATTRIBUTE_GetModelName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
char *AIATTRIBUTE_GetModelName( const char *szClassName )
{
	CAIAttributes *pAIAttributes = AIATTRIBUTE_Lookup( szClassName );
	if ( pAIAttributes )
	{
        return pAIAttributes->GetModelName();
    }
    return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        AIATTRIBUTE_GetCSVFileName
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
char *AIATTRIBUTE_GetCSVFileName( const char *szClassName )
{
	CAIAttributes *pAIAttributes = AIATTRIBUTE_Lookup( szClassName );
	if ( pAIAttributes )
	{
        return pAIAttributes->GetCSVFileName();
    }
    return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AIATTRIBUTE_SetInfo
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
WEAPON_ATTRIBUTE_PTR AIATTRIBUTE_SetInfo( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CAIAttributes *pAIAttributes = AIATTRIBUTE_Lookup( self->className );
	if ( pAIAttributes )
	{
		self->health			= pAIAttributes->GetHealth();
		hook->base_health		= pAIAttributes->GetBaseHealth();
		self->elasticity		= pAIAttributes->GetElasticity();
		self->mass				= pAIAttributes->GetMass();

		self->ang_speed			= pAIAttributes->GetAngleSpeed();
		self->s.mins			= pAIAttributes->GetMins();
		self->s.maxs			= pAIAttributes->GetMaxs();
		self->s.render_scale	= pAIAttributes->GetRenderScale();

		hook->active_distance	= pAIAttributes->GetActiveDistance();
		hook->attack_dist		= pAIAttributes->GetAttackDistance();
		hook->jump_attack_dist	= pAIAttributes->GetJumpAttackDistance();

		hook->upward_vel		= pAIAttributes->GetUpwardVelocity();
		hook->run_speed			= pAIAttributes->GetRunSpeed();
		hook->walk_speed		= pAIAttributes->GetWalkSpeed();
		hook->attack_speed		= pAIAttributes->GetAttackSpeed();

		ai_setfov( self, pAIAttributes->GetFOV() );

		hook->max_jump_dist	= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

        hook->fMinAttenuation   = pAIAttributes->GetMinAttenuation();
        hook->fMaxAttenuation   = pAIAttributes->GetMaxAttenuation();

		pAIAttributes->CacheSounds();

        FRAMEDATA_SetModelAnimSpeeds( self );

		//<nss>
		//The speak range defaults to the active_distance value
		hook->speak				= hook->active_distance*0.15f;

		//Added Key Pair values for Active Alert Distance
		//Check to see if we have a key name m_speak
		//if so let's set the range.
		//if not let's go off of the value stored in the AIDATA.CSV file
        if(self->epair)
		{
			for (int i = 0; self->epair [i].key != NULL; i++)
			{
				if (!stricmp (self->epair [i].key, "speak"))
				{
					hook->speak = atof (self->epair [i].value);
				}
				if (!stricmp (self->epair [i].key, "sight"))
				{
					hook->active_distance = atof (self->epair [i].value);
				}
			}

		}

		return pAIAttributes->GetWeaponAttributes();

	}
#ifdef WIN32
	_RPT1( _CRT_WARN, "No AI attributes defined for %s, You can ignore this.", self->className );
#endif			

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        AISOUNDS_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void AISOUNDS_Add( CAIAttributes *pAIAttributes, char *szSoundName )
{
    if (!strlen(szSoundName))  // file name exists?
      return;

	char* pSoundName = (char*)gstate->X_Malloc(sizeof(char)*(strlen(szSoundName)+1),MEM_TAG_AI);
	if ( !pSoundName )
	{
		com->Error( "Mem allocation failed." );
	}

	strcpy( pSoundName, szSoundName );

	pAIAttributes->AddSound( pSoundName );
}

// ----------------------------------------------------------------------------
//
// Name:		AISOUNDS_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AISOUNDS_ReadFile( const char *szFileName )
{
	int bSuccess = TRUE;
	
	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumAttributes = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				CAIAttributes *pAIAttributes = AIATTRIBUTE_Lookup( szElement );
				if ( pAIAttributes )
				{
					while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
					{
						AISOUNDS_Add( pAIAttributes, szElement );
					}
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

#if 0
#endif 0
