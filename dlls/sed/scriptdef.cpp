#include <windows.h>
#include <stdio.h>
#include "katanox.h"
#include "anoxcam.h"
#include "util.h"


//--------------------
// JSCRIPT::JSCRIPT
//
// Allocates the memory needed for the cameras.
//--------------------


JSCRIPT::JSCRIPT()
{
    int i;
    Camera = new CAMERA[MAXCAMERAS];
    name[0] = 0x00;
    iActiveCam = 0;

    for( i = 0; i < MAXCAMERAS; i+= 2 )
    {
        if( i < MAXCAMERAS-1 ) Camera[i].iCamFocus = i+1;
    }
}


//--------------------
// JSCRIPT::JSCRIPT
//
// Deallocates all cameras, and each camera's destructor will also destroy
// each camera's list of nodes.
//--------------------


JSCRIPT::~JSCRIPT()
{
    delete [] Camera;
}


//--------------------
// JSCRIPT::Reset
//
// does same thing as a destructor then a constructor
//--------------------


void JSCRIPT::Reset()
{
    int i;

    delete [] Camera;
    Camera = new CAMERA[MAXCAMERAS];
    name[0] = 0x00;
    iActiveCam = 0;

    for( i = 0; i < MAXCAMERAS; i+= 2 )
    {
        if( i < MAXCAMERAS-1 ) Camera[i].iCamFocus = i+1;
    }

}


//--------------------
// JSCRIPT::Init( VEC3 intial_position, VEC3 initial_velocity )
//
// Initializes all cameras in preparation for script play.
//--------------------

void JSCRIPT::Init( const VEC3& init_pos, const VEC3& init_vel )
{
    int iLoop;

    for( iLoop = 0; iLoop < MAXCAMERAS; iLoop++ )
    {
        // call the init function
        Camera[iLoop].Init( init_pos, init_vel );

        // now find out if it is current view
        if( Camera[iLoop].NodeCurrent )
        {
            if( Camera[iLoop].NodeCurrent->ulFlags & NODEFLAG_MAKECURRENT )
            iActiveCam = iLoop;
        }
    }
}


//--------------------
// JSCRIPT::Iterate( float time_since_last_call )
//
// Iterates all cameras simulataneously, setting its own pos, vel, ang to the
// camera that has the current view.  It stops whenever it receives a
// CAMSTATE_ENDSCRIPT signal from any camera, or when all cameras have become
// CAMSTATE_IDLE.
//
// Returns SCRIPT_STOPPED or SCRIPT_PLAYING.
//--------------------


int JSCRIPT::Iterate( float frametime )
{
    int iLoop;
    int cIdleCams;
    int iRetVal = SCRIPT_PLAYING;
    CAMERA *pCam;

    for( iLoop = 0, cIdleCams = 0; iLoop < MAXCAMERAS; ++iLoop )
    {
        switch( Camera[iLoop].Iterate( frametime ) )
        {
        case CAMSTATE_IDLE:         ++cIdleCams; break;
        case CAMSTATE_ENDSCRIPT:    iRetVal = SCRIPT_STOPPED; break;
        case CAMSTATE_MAKECURRENT:  iActiveCam = iLoop; break;
        case CAMSTATE_NORMAL:       break;
        }
    }

    if( cIdleCams == MAXCAMERAS ) iRetVal = SCRIPT_STOPPED;

    pCam = &Camera[iActiveCam];
    pos = pCam->pos;
    vel = pCam->vel;
    if(( pCam->iCamFocus >= 0 ) && ( pCam->iCamFocus < MAXCAMERAS ))
    {
        //---------------------------
        // get the angle from the 
        // vector between active cam 
        // and active cam's focus
        //---------------------------

        ang = (Camera[Camera[iActiveCam].iCamFocus].pos - pos).VecToAngle();
    }
    else 
    {
        //-------------------------
        // otherwise just make the 
        // camera point "forward" 
        // in direction of travel
        //-------------------------

        ang = vel.VecToAngle();
    }

    return iRetVal;
}


//--------------------
// JSCRIPT::SaveScript( char *filename )
//
// Save the script to a text file.  Returns less than zero on failure.
//--------------------


char *NodeFlagNames[32] = 
{
    "(move relative)", "(current view)", "(end script)", 
    "","","","","",
    "","","","","","","","",
    "","","","","","","","",
    "","","","","","","","",
};


int JSCRIPT::SaveScript( char *filename )
{
    int     iRetVal;
    int     iCount, iBitCount, iNodeCount = 0;
    float   fAccumTime;
    FILE    *fp;
    CAMERA  *pCam;
    CAMNODE *pNode;
    unsigned long ulFlags;


    fp = fopen( filename, "wt" );
    if( fp == NULL )
    {
        iRetVal = -1;
        MakeScriptError( "Unable to open \"%s\" for text output.", filename );
    }
    else
    {
        fprintf( fp, "# Anachronox camera script v5.\n" );
        fprintf( fp, "# This camera script was built " __TIMESTAMP__ ".\n" );
        fprintf( fp, "# Script generator compiled on " __TIMESTAMP__ ".\n" );
        fprintf( fp, "# original file name \"%s\"", filename );
        fprintf( fp, "#\n" );
        fprintf( fp, "# Note that only the values after any FLAGS identifier are\n" );
        fprintf( fp, "# interpreted in C-fashion (leading 0x = hex, leading 0 = octal).\n" );
        fprintf( fp, "# All other values are interpreted as decimal integers.\n" );
        fprintf( fp, "#\n" );
        fprintf( fp, "# flags:\n" );
        
        for( iBitCount = 0, ulFlags = 1; iBitCount < 32; iBitCount++, ulFlags <<= 1 )
        {
            if( NodeFlagNames[iBitCount][0] != '\0' )
            {
                fprintf( fp, "# 0x%08x : %s\n", ulFlags, 
                    NodeFlagNames[iBitCount] );
            }
        }

        fprintf( fp, "\nScriptname=%s\n", name );

        for( iCount = 0; iCount < MAXCAMERAS; iCount++ )
        {
            pCam = &Camera[iCount];
            pNode = pCam->NodeHead;
            fAccumTime = 0.0f;

            if( pNode )
            {
                fprintf( fp, "\n# Note that CameraNum must be first entry for each camera!\n" );
                fprintf( fp, "CameraNum=%i\n", iCount );
                fprintf( fp, "CameraName=%s\n", pCam->name );
                fprintf( fp, "CameraFocus=%i\n", pCam->iCamFocus );

                while( pNode )
                {
                    ++iNodeCount;
                    fAccumTime += pNode->fPathTime;

                    fprintf( fp, "\n# timestamp = %s\n", TimeStamp(fAccumTime) );
                    fprintf( fp, "sor\n" );
                    fprintf( fp, "pathtime=%f\n", pNode->fPathTime );

                    if( pNode->ulFlags != 0L )
                    {
                        // print descriptive flag names
                        fprintf( fp, "# flags: " );
                        for( iBitCount = 0, ulFlags = 1; iBitCount < 32;
                             iBitCount++, ulFlags << 1 )
                        {
                            if( ulFlags & pNode->ulFlags )
                            {
                                fprintf( fp, "%s ", NodeFlagNames[iBitCount] );
                            }
                        }
                        fprintf( fp, "\n" );
                        fprintf( fp, "flags=0x%08x\n", pNode->ulFlags );
                    }

                    if( pNode->ulFlags & NODEFLAG_MOVEREL )
                    {
                        fprintf( fp, "pos=%f %f %f\n", 
                            pNode->vRelPos[0], pNode->vRelPos[1], pNode->vRelPos[2] );
                    }
                    else
                    {
                        fprintf( fp, "pos=%f %f %f\n", 
                            pNode->vFinalPos[0], pNode->vFinalPos[1], pNode->vFinalPos[2] );
                    }
                    fprintf( fp, "vel=%f %f %f\n", 
                        pNode->vFinalVel[0], pNode->vFinalVel[1], pNode->vFinalVel[2] );
                    fprintf( fp, "eor\n" );

                    pNode = pNode->next;
                }

                fprintf( fp, "\n# node count is (%i)\n", iNodeCount );
            }
        }

        fclose( fp );
        iRetVal = iNodeCount;

        if( iNodeCount == 0 )
        {
            MakeScriptError( "Warning: saved empty script \"%s\".", filename );
        }
    }


    return iRetVal;
}


//--------------------
// JSCRIPT::LoadScript
//
// Load script from file.  Returns negative on error.  Returns number of nodes
// read on success.
//--------------------

char gl_szBuf[100];
static int st_bNodeRecord = FALSE;

int JSCRIPT::LoadScript( char *filename )
{
    int iRetVal;
    int iNodeCount = 0;
    int iLineNum;
    int bError;
    FILE *fp;

    fp = fopen( filename, "rt" );
    if( fp == NULL )
    {
        iRetVal = -1;
        MakeScriptError( "Unable to open \"%s\" for text input.", filename );
    }
    else
    {
        //-----------------------------
        // Before reading in the file,
        // we should reset the script.
        //-----------------------------

        Reset();

        //-------------------
        // Now begin parsing
        //-------------------

        st_bNodeRecord = FALSE;
        iLineNum = 0;
        bError = FALSE;
        while( (!feof(fp)) && (!bError) )
        {
            fgets( gl_szBuf, 99, fp );
            ++iLineNum;
            switch( ParseLine( iLineNum, gl_szBuf ) )
            {
            case 1: ++iNodeCount; break;
            case -1: bError = TRUE; break;
            }
            
        }

        fclose( fp );
        iActiveCam = 0;
        iRetVal = iNodeCount;

        if( bError )
        {
            iRetVal = 0;
        }
        else if( iNodeCount == 0 )
        {
            MakeScriptError( "Warning: script \"%s\" was empty.", filename );
        }
    }

    return iRetVal;
}


static char WHITESPACE[]= " \n\r\t=";

int JSCRIPT::ParseLine( int iLineNum, char *szLine )
{
    char *token;
    int iRetVal = 0; // is only 1 when encounter "sor" start-of-record
    int i, slen;
    
    if( szLine == NULL ) return 0;
    slen = strlen( szLine );
    if( slen == 0 ) return 0;

    // kill trailing whitespace

    i = slen-1;
    
    while( i >= 0 )
    {
        if( strchr( WHITESPACE, szLine[i] ) ) break;
        szLine[i] = 0x00;
        --i;
    }

    token = strtok( szLine, WHITESPACE ); 
    
    if( token == NULL ) return 0; // nothin but whitespace

    if( token[0] != '#' ) // then is not a comment, go ahead and parse
    {
        CAMERA  *pCam;

        pCam = &Camera[iActiveCam];

        //---------------------------
        // process non-nodal entries
        //---------------------------

        if( ! st_bNodeRecord )
        {
            if( ! stricmp( token, "scriptname" ) )
            {
                token = strtok( NULL, "" ); // include middle whitespace
                strncpy( name, token, MAXNAMELEN );
                name[ MAXNAMELEN-1 ] = 0x00;
            }
            else if( ! stricmp( token, "cameranum" ) )
            {
                token = strtok( NULL, WHITESPACE );
                iActiveCam = atoi(token);
            }
            else if( ! stricmp( token, "cameraname" ) )
            {
                token = strtok( NULL, "" );
                strncpy( pCam->name, token, MAXNAMELEN );
                name[ MAXNAMELEN-1 ] = 0x00;
            }
            else if( ! stricmp( token, "camerafocus" ) )
            {
                token = strtok( NULL, WHITESPACE );
                pCam->iCamFocus = atoi( token );
            }
            else if( ! stricmp( token, "sor" ) )
            {
                pCam->AppendNode( new CAMNODE );
                st_bNodeRecord = TRUE;
            }
            else
            {
                MakeScriptError( "ERROR: line(%i) had unknown token %s",
                    iLineNum, token );
                iRetVal = -1;
            }
        }
        //-------------------------------
        // the following tokens are all
        // node-related, so only process
        // if have a valid node to modify
        //-------------------------------
        else 
        {
            CAMNODE *pNode;
            pNode = pCam->NodeCurrent;

            if( pNode == NULL )
            {
                MakeScriptError( "ERROR: tried to parse NULL node at line (%i)",
                    iLineNum );
                iRetVal = -1;
            }
            if( ! stricmp( token, "pathtime" ) )
            {
                token = strtok( NULL, WHITESPACE );
                pNode->fPathTime = atof( token );
            }
            else if( ! stricmp( token, "flags" ) )
            {
                token = strtok( NULL, WHITESPACE );
                pNode->ulFlags = strtoul( token, NULL, 0 );
            }
            else if( ! stricmp( token, "pos" ) )
            {
                float f1, f2, f3;

                token = strtok( NULL, "" );
                sscanf( token, "%f %f %f", &f1, &f2, &f3 );
                pNode->vRelPos = pNode->vFinalPos = VEC3( f1, f2, f3 );
            }
            else if( ! stricmp( token, "vel" ) )
            {
                float f1, f2, f3;

                token = strtok( NULL, "" );
                sscanf( token, "%f %f %f", &f1, &f2, &f3 );
                pNode->vFinalVel = VEC3( f1, f2, f3 );
            }
            else if( ! stricmp( token, "eor" ) )
            {
                if( st_bNodeRecord )
                {
                    iRetVal = 1;
                    st_bNodeRecord = FALSE;
                }
                else
                {
                    iRetVal = -1;
                    MakeScriptError( "ERROR: line (%i) had unmatched SOR", iLineNum );
                }
            }
            else
            {
                iRetVal = -1;
                MakeScriptError( "ERROR: line (%i) had unrecognized node token \"%s\"", 
                    iLineNum, token );
            }
        }
    }

    return iRetVal;
}


//--------------------
// JSCRIPT::GetPos
//--------------------

VEC3 JSCRIPT::GetPos()
{
    return pos;
}


//--------------------
// JSCRIPT::GetAng
//--------------------

VEC3 JSCRIPT::GetAng()
{
    return ang;
}


