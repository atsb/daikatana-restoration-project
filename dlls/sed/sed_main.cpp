#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gl/gl.h>
#include "sed.h"

#include "dk_dll.h"

///////////////////////////////////////////////////////////////////////////////
// exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	*dll_Query		(void);
DllExport	void	dll_Graphics3   ( USERENT * );
DllExport	int		dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

///////////////////////////////////////////////////////////////////////////////
// globals
///////////////////////////////////////////////////////////////////////////////

GLCAMEDIT   gl_Editor;
JSCRIPT     gl_Script;
STATE       *gl_pState = NULL;
SCRIPTINFO  gl_Info;
int         gl_iEdit = EDIT_NOTHING;
char        gl_szLoginName[40];
char        gl_szScriptDir[40];
char        gl_szFileName[_MAX_PATH];
extern      void(*conprintf)( char *fmt, ... ) = NULL;

static		float	default_vel = 320;

char	*dll_Description = "SED.DLL.  Camera script editing functions and interface.\n";

// global flags

int      gl_bSedOn = FALSE;
int      gl_bHello = TRUE;
int      gl_bRestoreCamera = FALSE;
int      gl_bShowInfo = FALSE;

char gl_szStatus[100];

char *gl_szKeyHelp[] =
{
    "F1=scriptor F2=show cam F3=near path F4=allpath F5=show nodes F6=show focus",
    "F7=show arrows F8=show all F9=(none) F10=(none) F11=(none) F12=quit",
    "CTRL=append INS=insert DEL=delete PGUP=previous PGDN=next /=center view",
    "P=change pos V=change vel ESDFAZ=move [=prevcam ]=nextcam +=more -=less"
};

#define NUMHELPLINES (sizeof(gl_szKeyHelp)/sizeof(char *) )

///////////////////////////////////////////////////////////////////////////////

void MakeBold( char *szBuf )
{
    int iLength, iCount;

    if( szBuf )
    {
        iLength = strlen( szBuf );
        for( iCount = 0; iCount < iLength ; iCount++ )
        {
            szBuf[iCount] |= 0x80;
        }
    }
}


void conprintf_b( char *fmt, ... )
{
    static char buf[200];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    MakeBold( buf );
    conprintf( "%s", fmt );
}


//---------------------------------------------------------------------------
//                     console command implementation
//---------------------------------------------------------------------------

void Sed_On( USERENT *ue );


//--------------------
// Sed_Ver
//
//--------------------

void Sed_Ver( USERENT *ue )
{
    gl_pState->Con_Printf( "SED.DLL compiled on " __DATE__ " at " __TIME__ );
}


//--------------------
// Sed_Save
//
//--------------------

void Sed_Save( USERENT *ue )
{
    int argc = gl_pState->GetArgc();

    if( gl_bSedOn && (!gl_iEdit) )
    {
        if( argc > 2 )
        {
            gl_pState->Con_Printf( "Usage: %s [scriptname]\n", gl_pState->GetArgv(0));
            gl_pState->Con_Printf( "If scriptname not provided, one will be generated.\n" );
        }
        else 
        {
            WIN32_FIND_DATA fdata;
            int iVal;
            int iNum;

            if( argc == 2 )
            {
                // save explicit filename

                sprintf( gl_szFileName, "%s%s", gl_szScriptDir, gl_pState->GetArgv(1) );
            }
            else
            {

                for( iNum = 0; iNum < 10000; iNum++ )
                {
                    sprintf( gl_szFileName, "%s%s%03i.script", gl_szScriptDir, gl_szLoginName, iNum );
                    if( FindFirstFile( gl_szFileName, &fdata ) == INVALID_HANDLE_VALUE ) break;
                }
            }

            iVal = gl_Script.SaveScript( gl_szFileName );

            if( iVal < 0 )
            {
                gl_pState->Con_Printf( "Error saving script file \"%s\"\n", gl_szFileName );
            }
            else
            {
                gl_pState->Con_Printf( "Successfully saved %i nodes to file \"%s\"\n", 
                    iVal, gl_szFileName );
            }
        }
    }
}


//--------------------
// Sed_Load
//
//--------------------

void Sed_Load( USERENT *ue )
{
    int iNumArg, iVal;

    if( ! gl_bSedOn )
    {
        Sed_On( ue );
    }

    if( gl_bSedOn && (!gl_iEdit) ) 
    {
        iNumArg = gl_pState->GetArgc();

        if( iNumArg != 2 )
        {
            gl_pState->Con_Printf( "Usage: %s <scriptname.script>\n", gl_pState->GetArgv(0) );
        }
        else
        {
            sprintf( gl_szFileName, "%s%s", gl_szScriptDir, gl_pState->GetArgv(1) );
            iVal = gl_Script.LoadScript( gl_szFileName );

            if( iVal < 0 )
            {
                gl_pState->Con_Printf( "Error loading script file \"%s\"\n", gl_szFileName );
            }
            else
            {
                gl_pState->Con_Printf( "Successfully read script file \"%s\"\n", gl_szFileName );
            }
        }
    }
}


//--------------------
// Sed_AddNode
//
//--------------------

void Sed_AddNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) 
    {
        gl_Editor.AppendNode( 1.0, 0L, VEC3(ue->s.origin), VEC3(ue->velocity) );
    }
}


//--------------------
// Sed_InsNode
//
//--------------------

void Sed_InsNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) 
    {
        gl_Editor.InsertNode( 1.0, 0L, VEC3(ue->s.origin), VEC3(ue->velocity) );
    }
}


//--------------------
// Sed_DelNode
//
//--------------------

void Sed_DelNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.DeleteNode();
}


//--------------------
// Sed_NextCam
//
//--------------------

void Sed_NextCam( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.NextCamera();
}


//--------------------
// Sed_PrevCam
//
//--------------------

void Sed_PrevCam( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.PrevCamera();
}


//--------------------
// Sed_NextNode
//
//--------------------

void Sed_NextNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.NextNode();
}


//--------------------
// Sed_PrevNode
//
//--------------------

void Sed_PrevNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.PrevNode();
}


//--------------------
// Sed_FirstNode
//
//--------------------

void Sed_FirstNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.FirstNode();
}


//--------------------
// Sed_LastNode
//
//--------------------

void Sed_LastNode( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) gl_Editor.LastNode();
}


//--------------------
// Sed_ShowAll
//
//--------------------

void Sed_ShowAll( USERENT *ue )
{ 
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_EVERYTHING ); 
}


//--------------------
// Sed_ShowCam
//
//--------------------

void Sed_ShowCam( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_CAMERAPOS );
}


//--------------------
// Sed_ShowNearPath
//
//--------------------

void Sed_ShowNearPath( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_ADJACENTPATH );
}


//--------------------
// Sed_ShowFarPath
//
//--------------------

void Sed_ShowFarPath( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_ENTIREPATH );
}


//--------------------
// Sed_ShowNodes
//
//--------------------

void Sed_ShowNodes( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_NODEPOS );
}


//--------------------
// Sed_ShowFocus
//
//--------------------

void Sed_ShowFocus( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_FOCUSPOINT );
}


//--------------------
// Sed_ShowAllCam
//
//--------------------

void Sed_ShowAllCam( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_ALLCAMERAS );
}


//--------------------
// Sed_ShowArrows
//
//--------------------

void Sed_ShowArrows( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.Toggle( GLCAM_SHOWARROWS );
}


//--------------------
// Sed_MoreDense
//
//--------------------

void Sed_MoreDense( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.ScaleDensity( 1.0f/1.2f );
}


//--------------------
// Sed_LessDense
//
//--------------------

void Sed_LessDense( USERENT *ue )
{
    if( gl_bSedOn ) gl_Editor.ScaleDensity( 1.2f );
}


//--------------------
// Sed_CenterView
//
//--------------------

void Sed_CenterView( USERENT *ue )
{
    if( gl_bSedOn && (!gl_iEdit) ) 
    {
        CameraSendViewAngle(ue,(gl_Info.NodePos-VEC3(ue->s.origin)-VEC3(0,0,20.0)).VecToAngle());
    }
}


//--------------------
// Sed_EditPos
//
//--------------------

void Sed_EditPos( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    if( gl_iEdit == EDIT_POS )
    {
        CameraSwitch();
    }
    else if( gl_iEdit == EDIT_NOTHING )
    {
        if( gl_Info.bInfoValid )
        {
            gl_iEdit = EDIT_POS;
            CameraSpawn( ue, CAMENT_EDITPOS );
            strcpy( gl_szStatus, "NODE POSITION: Q=finish, P=change modes" );
        }
    }
}


//--------------------
// Sed_EditVel
//
//--------------------

void Sed_EditVel( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    if( gl_iEdit == EDIT_VEL )
    {
        CameraSwitch();
    }
    else if( gl_iEdit == EDIT_NOTHING )
    {
        if( gl_Info.bInfoValid )
        {
            gl_iEdit = EDIT_VEL;
            CameraSpawn( ue, CAMENT_EDITVEL );
            strcpy( gl_szStatus, "NODE VELOCITY: Q=finish, V=change modes, ZERO=clear" );
        }
    }
}


//--------------------
// Sed_EditTime
//
//--------------------

void Sed_EditTime( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    if( gl_iEdit == EDIT_NOTHING )
    {
        if( gl_Info.bInfoValid )
        {
            gl_iEdit = EDIT_TIME;
            CameraSpawn( ue, CAMENT_EDITTIME );
            strcpy( gl_szStatus, "NODE TIME: Q=finish, ZERO=clear" );
        }
    }
}


//--------------------
// Sed_EditPlay
//
//--------------------

void Sed_EditPlay( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    if( gl_iEdit == EDIT_NOTHING )
    {
        gl_iEdit = EDIT_PLAY;
        gl_Script.Init( ue->s.origin, ue->velocity );
        CameraSpawn( ue, CAMENT_PLAYBACK );
        strcpy( gl_szStatus, "PLAYBACK: Q=stop" );
    }
}


//--------------------
// Sed_EditStop
//
//--------------------

void Sed_EditStop( USERENT *ue )
{
    if( gl_iEdit )
    {
        CameraDestroy( ue );
        gl_iEdit = EDIT_NOTHING;

        gl_szStatus[0] = 0x00;

        gl_pState->SetCvar( "viewmode", "1" );
    }
}


//--------------------
// Sed_EditView
//
//--------------------

void Sed_ExternView( USERENT *ue )
{
    if( gl_iEdit == EDIT_MARIOVW )
    {
        Sed_EditStop(ue);
    }

    if( gl_iEdit == EDIT_NOTHING )
    {
        gl_pState->SetCvar( "viewmode", "0" );
        gl_iEdit = EDIT_MARIOVW;
        CameraSpawn( ue, CAMENT_MARIOTIME );
        strcpy( gl_szStatus, "external view" );
    }
}


//--------------------
// Sed_EditAltPlay
//
//--------------------

void Sed_EditAltPlay( USERENT *ue )
{
    Sed_EditStop(ue);
    Sed_Load(ue);
    Sed_EditPlay(ue);
}


//--------------------
// Sed_EditZero
//
//--------------------

void Sed_EditZero( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    switch( gl_iEdit )
    {
    case EDIT_VEL:
        Sed_EditStop( ue );
        gl_Editor.EditNodeVel( VEC3(), FALSE );
        gl_Editor.GetScriptInfo( &gl_Info );
        break;
    case EDIT_TIME:
        Sed_EditStop( ue );
        gl_Editor.SetNodeTime( 0.0f );
        gl_Editor.GetScriptInfo( &gl_Info );
        break;
    }
}


//--------------------
// Sed_On
//
// Turns script editing on, and clears all scripts in memory.
//--------------------


static char *SedGreetings[] = {
    "Hello, %s.  Anox script editor is initialized.",
    "Hurry up, %s.  Write some scripts or something.",
    "Anox script editor is ready for %s input.",
    "Get me a coke, %s.  Either that or do some scripts.",
    "%s, don't say that in front of Santa Claus.",
    "Ever wonder, %s, which bored programmer wrote this?",
    "I think you ought to clean your mouse balls, %s.",
    "%s, you owe Joey fifty bucks.",
	"Joey Liaw == assneck.",
    "Go away.  Seriously, %s, I'm too tired for scripting.",
    "Ya know, %s, you could be doing other things...",
    "Did you know that your name is %s?",
    "Please use me %s.  I am your slave for eternity.",
    "I think %s should help Joey get some more monitors.",
    "Konichiwa, %s-sama.  Watashi wa anoksu sculipto desu.",
    "%s, think you could lend Joey a 21\" monitor?",
    "Did you know %s is secretly a movie director?",
    "%s is the greatest movie director, ever.",
};

static char *ExtraGreetings[] = {
    "Early aren't we, %s.  Or was it an all-nighter?",
    "If you didn't have breakfast, %s, go get one now.",
    "Good morning %s.",
    "Hey %s, it's lunchtime, I hope you've eaten.",
    "Good afternoon %s.",
    "Good evening %s.",
    "Go home, %s.  It's getting late.",
    "Fer crying out loud %s go home already!",
};

#define NUMGREETINGS ( sizeof(SedGreetings)/sizeof(char *) )

void Sed_On( USERENT *ue )
{
    int i;

    if( ! gl_bSedOn )
    {
        gl_bSedOn = TRUE;
        gl_Script.Reset();
        Sed_ShowAll(NULL);

        if( gl_bHello )
        {
            gl_bHello = FALSE;

            i = (rand() % (NUMGREETINGS + 1 )) - 1;
            if( i == -1 )
            {
                // get a time-based greeting
                sprintf( gl_szStatus, "Anox script editor ready." );
            }
            else
            {
                // get a random greeting
                sprintf( gl_szStatus, SedGreetings[i], gl_szLoginName );
            }
        }
    }
}


//--------------------
// Sed_Off
//
//--------------------

void Sed_Off( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    gl_bSedOn = FALSE;
    gl_szStatus[0] = 0x00;
    gl_pState->clearxy();
}

void  Sed_Verbose( USERENT *ue )
{
    if( ! gl_bSedOn ) return;

    gl_bShowInfo = ! gl_bShowInfo;
}


void Sed_Help( USERENT *ue )
{
    conprintf_b( "\nAnachronox ScriptEDitor (SED) Additional Help\n" );
    conprintf( "\n" );
    conprintf( "sed_version            - print out version info\n" );
    conprintf( "sed_save [filename]    - save your script, filename optional\n" );
    conprintf( "sed_load <filename>    - load script, filename required\n" );
    conprintf( "\n" );
    conprintf( "sed_on                 - start the scriptor (should be F1)\n" );
    conprintf( "---- end of help\n" );
}

/////////////////////////////////////////////////////////////////////////////
//	Sed_DefaultVel
//
//
/////////////////////////////////////////////////////////////////////////////

void	Sed_DefaultVel (userEntity_t *self)
{
	if (gl_pState->GetArgc () != 2)
	{
		gl_pState->Con_Printf ("USAGE: sed_vel [velocity]\n");
		return;
	}

	default_vel = atof (gl_pState->GetArgv (2));
}

//---------------------------------------------------------------------------
//                         main rendering loop
//                         CALLED FROM CLIENT
//---------------------------------------------------------------------------

void dll_Graphics3( USERENT *invalid_do_not_use )
{
    if( ( ! gl_bSedOn ) || ( gl_bShowInfo == FALSE ) )
    {
        return;
    };

    int i;

    gl_pState->clearxy();

    glDisable( GL_TEXTURE_2D );

    // no little gl objects while playing script
    if( gl_iEdit != EDIT_PLAY ) 
    {
        gl_Editor.Draw( gl_pState->frametime );
    }

    gl_Editor.GetScriptInfo( &gl_Info );
    
    if( gl_iEdit != EDIT_PLAY ) 
    {
        gl_Editor.Draw( gl_pState->frametime );
        gl_pState->printxy( 0, 40, "NODE @ %f", gl_Info.NodeAccumTime );
        gl_pState->printxy( 0, 48, "  pos = %.4g %.4g %.4g", 
            gl_Info.NodePos[0], gl_Info.NodePos[1], gl_Info.NodePos[2] );
        gl_pState->printxy( 0, 56, "speed = %.4g, ang= %.4g %.4g",
            gl_Info.NodeVelAng[0], gl_Info.NodeVelAng[1], gl_Info.NodeVelAng[2] );
        gl_pState->printxy( 0, 64, " time = %.4g sec", gl_Info.NodeTime );
        gl_pState->printxy( 0, 72, "flags = %s", gl_Info.NodeFlags );

        gl_pState->printxy( 320, 40, "SCRIPT INFO" );
        gl_pState->printxy( 320, 48, "path density = %f", 1.0/gl_Info.NodeDensity );
        gl_pState->printxy( 320, 56, " edit camera = %i --> focus %i", gl_Info.CamNum, gl_Info.CamFocus );

        if( gl_iEdit == EDIT_NOTHING )
        {
            for( i = 0; i < NUMHELPLINES; i++ )
            {
                gl_pState->printxy( 0, 464-(NUMHELPLINES-i)*8, "%.80s", gl_szKeyHelp[i] );
            }
        }
        gl_pState->printxy( 0, 472, "%.80s", gl_szStatus );
    }

    glEnable( GL_TEXTURE_2D );
}


//---------------------------------------------------------------------------
//                   katana-style dll interface functions
//                         CALLED FROM SERVER
//---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// dll_Version
//
// returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version	(int size)
{
	if (size == IONSTORM_DLL_INTERFACE_VERSION)
		return	TRUE;
	else
		return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerLoad
//
//	called when Daikatana first loads this DLL
//
//	NO ROUTINES FROM COMMON ARE AVAILABLE HERE!!
///////////////////////////////////////////////////////////////////////////////

void dll_ServerLoad( STATE *pState )
{
    gl_pState = pState;
    conprintf = gl_pState->Con_Printf;
    gl_Editor.AttachScript( &gl_Script );

    pState->AddCommand( "sed_version", Sed_Ver );
    pState->AddCommand( "sed_on", Sed_On );
    pState->AddCommand( "sed_off", Sed_Off );
    pState->AddCommand( "sed_save", Sed_Save );
    pState->AddCommand( "sed_load", Sed_Load );

    pState->AddCommand( "sed_addnode", Sed_AddNode );
    pState->AddCommand( "sed_insnode", Sed_InsNode );
    pState->AddCommand( "sed_delnode", Sed_DelNode );

    pState->AddCommand( "sed_nextcam", Sed_NextCam );
    pState->AddCommand( "sed_prevcam", Sed_PrevCam );
    pState->AddCommand( "sed_nextnode", Sed_NextNode );
    pState->AddCommand( "sed_prevnode", Sed_PrevNode );
    pState->AddCommand( "sed_firstnode", Sed_FirstNode );
    pState->AddCommand( "sed_lastnode", Sed_LastNode );

    pState->AddCommand( "sed_showall", Sed_ShowAll );
    pState->AddCommand( "sed_showcam", Sed_ShowCam );
    pState->AddCommand( "sed_shownearpath", Sed_ShowNearPath );
    pState->AddCommand( "sed_showfarpath", Sed_ShowFarPath );
    pState->AddCommand( "sed_shownodes", Sed_ShowNodes );
    pState->AddCommand( "sed_showfocus", Sed_ShowFocus );
    pState->AddCommand( "sed_showallcam", Sed_ShowAllCam );
    pState->AddCommand( "sed_showarrows", Sed_ShowArrows );

    pState->AddCommand( "sed_centerview", Sed_CenterView );
    pState->AddCommand( "sed_editpos", Sed_EditPos );
    pState->AddCommand( "sed_editvel", Sed_EditVel );
    pState->AddCommand( "sed_editstop", Sed_EditStop );
    pState->AddCommand( "sed_edittime", Sed_EditTime );
    pState->AddCommand( "sed_editzero", Sed_EditZero );

    pState->AddCommand( "sed_editplay", Sed_EditPlay );

    pState->AddCommand( "sed_moredense", Sed_MoreDense );
    pState->AddCommand( "sed_lessdense", Sed_LessDense );

    pState->AddCommand( "sed_externview", Sed_ExternView );
    pState->AddCommand( "sed_editaltplay", Sed_EditAltPlay );

	pState->AddCommand( "sed_vel", Sed_DefaultVel );

    pState->AddCommand( "sed_help", Sed_Help );
    pState->AddCommand( "sed_verbose", Sed_Verbose );
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerInit
//
//	called after all DLLs have been loaded and their dll_ServerLoad
//	functions have been run.  This allows all dlls to do setup stuff
//	before any other DLLs access their functions.
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerInit(void)
{
    WIN32_FIND_DATA fdata;
    DWORD dwLen = 40L;

	gl_pState->Con_Printf ("* Loaded SED.DLL, %i.\n", sizeof (playerHook_t));

    if( ! GetUserName( gl_szLoginName, &dwLen ) )
    {
        strcpy( gl_szLoginName, "user" );
    }
    else if ( ! stricmp( gl_szLoginName, "user" ) )
    {
        strcpy( gl_szLoginName, "user_" );
    }

    gl_szLoginName[0] = toupper(gl_szLoginName[0]);

    CreateDirectory( "scripts", NULL );

    if( FindFirstFile( "scripts", &fdata ) == INVALID_HANDLE_VALUE )
    {
        strcpy( gl_szScriptDir, ".\\" );
    }
    else
    {
        if( fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            strcpy( gl_szScriptDir, "scripts\\" );
        }
        else
        {
            strcpy( gl_szScriptDir, ".\\" );
        }
    }

    srand((unsigned)time(NULL));
}

///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

void dll_LevelExit(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

void dll_LevelLoad (void)
{
    gl_bSedOn = FALSE;
    gl_Script.Reset();

	gl_pState->Con_Printf ("SED.DLL: dll_LevelLoad\n");
}

///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

void dll_ServerKill (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void *dll_Query(void)
{
	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Entry
//
//
/////////////////////////////////////////////////////////////////////////////////

int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
	int			size;

	switch (dwReasonForCall)
	{
		case	QDLL_VERSION:
			size = *(int *) pvData;

			return	dll_Version (size);
			break;
		
		case 	QDLL_QUERY:
			* (userEpair_t **) pvData = NULL;

			return	TRUE;
			break;

		case	QDLL_DESCRIPTION:
			* (char **) pvData = dll_Description;

			return	TRUE;
			break;

		case	QDLL_SERVER_INIT:
			dll_ServerInit ();

			return	TRUE;
			break;

		case	QDLL_SERVER_KILL:
			dll_ServerKill ();

			return	TRUE;
			break;

		case	QDLL_SERVER_LOAD:
			dll_ServerLoad ((serverState_t *) pvData);

			return	TRUE;
			break;

		case	QDLL_LEVEL_LOAD:
			dll_LevelLoad ();

			return	TRUE;
			break;

		case	QDLL_LEVEL_EXIT:
			dll_LevelExit ();

			return	TRUE;
			break;
	}

	return	FALSE;
}
