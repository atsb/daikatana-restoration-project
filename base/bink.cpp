#include <windows.h>
#include "client.h"
#include "bink.h"

static	HBINK bink = 0;
static	HBINKBUFFER binkbuf = 0;
static	HWND hBinkWnd;
static	HINSTANCE	hGlobalInstance;
extern	HINSTANCE	global_hInstance;
extern	HWND	cl_hwnd;

LONG WINAPI BinkWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

void NextBinkFrame()
{
	if( cl.cinematictime <= 0 )
	{
		return;
	}
	cl.cinematictime = cls.realtime - cl.cinematicframe*1000/14;

	BinkDoFrame( bink );

	if( BinkBufferLock( binkbuf ) )
	{
		BinkCopyToBuffer( bink, binkbuf->Buffer, binkbuf->BufferPitch, binkbuf->Height, 0, 0, binkbuf->SurfaceType );
		BinkBufferUnlock( binkbuf );
	}

	BinkBufferBlit( binkbuf, bink->FrameRects, BinkGetRects( bink, binkbuf->SurfaceType ) );

	if( bink->FrameNum == bink->Frames - 1 )
	{
		cl.cinematictime = 0;
		BinkBufferClose( binkbuf );
		BinkClose( bink );
	}
	else
	{
		BinkNextFrame( bink );
	}
	cl.cinematicframe = bink->FrameNum;
}

void StopBink()
{
	cl.cinematictime = 0;
	BinkBufferClose( binkbuf );
	BinkClose( bink );
}

int PlayBink( char *filename )
{
	if( Cvar_VariableValue ("dedicated") != 0 )
	{
		return 0;
	}

	BinkSoundUseDirectSound( 0 );

	bink = BinkOpen( filename, 0 );

	if( bink )
	{
/*
		if( Cvar_VariableValue ("vid_fullscreen") != 0 )
		{
			BinkBufferSetResolution( viddef.width, viddef.height, 16 );
			binkbuf = BinkBufferOpen( cl_hwnd, viddef.width, viddef.height, BINKBUFFERRESOLUTION );
		}
		else
*/		{
			binkbuf = BinkBufferOpen( cl_hwnd, viddef.width, viddef.height, 0 );
		}

		if( binkbuf )
		{
			ShowCursor( FALSE );
			BinkBufferSetOffset( binkbuf, 0, 26 );
			cl.cinematictime = Sys_Milliseconds();
			return 1;
		}
		else
		{
			BinkBufferClose( binkbuf );
		}
	}
	else
	{
		BinkClose( bink );
	}

	return 0;
}

