#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "anoxcam.h"


char ErrorBuf[200];
int bErrorOccurred = FALSE;


void MakeScriptError( char *fmt, ... )
{
    va_list ap;

    va_start(ap, fmt);
    vsprintf( ErrorBuf, fmt, ap );
    va_end(ap);

    bErrorOccurred = TRUE;
}


char *GetScriptError()
{
    char *cpRet;

    if( bErrorOccurred )
    {
       cpRet = ErrorBuf;
       bErrorOccurred = FALSE;
    }
    else cpRet = NULL;


    return cpRet;
}

char buf[100];

char *TimeStamp( float secs )
{
    int h,m,s;

    s = int(secs);
    h = s / 3600; s -= h*3600;
    m = s / 60;   s -= m*60;

    sprintf( buf, "%i:%02i:%02g", h, m, secs-h*3600.0f-m*60.0f );
    return buf;
}