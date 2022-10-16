#include "windows.h"
#include "ref.h"
#include "dk_array.h"
#include "dk_buffer.h"
#include "dk_menup.h"
#include "winquake.h"

array_obj<gl_driver_info_t>	gl_driver_info;
int							gl_driver_info_count = 0;

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

const unsigned char * ( APIENTRY * qglGetString )(unsigned int name);
int   ( WINAPI * qwglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR *);
int   ( WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
BOOL  ( WINAPI * qwglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
HGLRC ( WINAPI * qwglCreateContext)(HDC);
BOOL  ( WINAPI * qwglMakeCurrent)(HDC, HGLRC);
BOOL  ( WINAPI * qwglDeleteContext)(HGLRC);

HDC		g_hDC;
HWND	g_hWnd;
HGLRC   g_hGLRC;

LONG WINAPI gl_driver_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

qboolean CreateTempWindow( HINSTANCE hInstance )
{
	WNDCLASS		wc;
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)gl_driver_wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "gl_driver_window";

    if( !RegisterClass( &wc ) )
	{
		return false;
	}

	g_hWnd = CreateWindowEx (
		 WS_EX_TOPMOST, 
		 "gl_driver_window",
		 "Daikatana",
		 WS_POPUP|WS_VISIBLE,
		 0, 0, 10, 10,
		 NULL,
		 NULL,
		 hInstance,
		 NULL);

	if( g_hWnd == NULL )
	{
		return false;
	}

	ShowWindow( g_hWnd, SW_HIDE );

    g_hDC = GetDC( g_hWnd );
    
	if( g_hDC  == NULL )
	{
		return false;
	}

	return true;
}

void BindGLFunctions( HINSTANCE hInstance )
{
	qglGetString			= ( const unsigned char * (__stdcall * )( unsigned int ) )GetProcAddress( hInstance, "glGetString" );
	qwglChoosePixelFormat	= (int (__stdcall *)(HDC,const struct tagPIXELFORMATDESCRIPTOR *))GetProcAddress( hInstance, "wglChoosePixelFormat" );
	qwglDescribePixelFormat	= (int (__stdcall *)(HDC,int,unsigned int,struct tagPIXELFORMATDESCRIPTOR *))GetProcAddress( hInstance, "wglDescribePixelFormat" );
	qwglSetPixelFormat		= (int (__stdcall *)(HDC,int,const struct tagPIXELFORMATDESCRIPTOR *))GetProcAddress( hInstance, "wglSetPixelFormat" );
	qwglCreateContext		= (HGLRC(__stdcall *)(HDC))GetProcAddress( hInstance, "wglCreateContext" );
	qwglMakeCurrent			= (int (__stdcall *)(HDC,HGLRC))GetProcAddress( hInstance, "wglMakeCurrent" );
	qwglDeleteContext		= (int (__stdcall *)(HGLRC))GetProcAddress( hInstance, "wglDeleteContext" );
}

void ShutDownGL( HINSTANCE hInstance )
{
	if( qwglMakeCurrent )
		qwglMakeCurrent( NULL, NULL );

	if ( g_hGLRC )
	{
		if( qwglDeleteContext )
			qwglDeleteContext( g_hGLRC );
		g_hGLRC = NULL;
	}

	if( g_hDC )
	{
		if( !ReleaseDC( g_hWnd, g_hDC ) )
		g_hDC = NULL;
	}

	if (g_hWnd)
	{
		DestroyWindow( g_hWnd );
		g_hWnd = NULL;
	}

	UnregisterClass( "gl_driver_window", global_hInstance );

	FreeLibrary( hInstance );

	qglGetString			= NULL;
	qwglChoosePixelFormat	= NULL;
	qwglDescribePixelFormat	= NULL;
	qwglSetPixelFormat		= NULL;
	qwglCreateContext		= NULL;
	qwglMakeCurrent			= NULL;
	qwglDeleteContext		= NULL;
}

qboolean InitGL( qboolean minidriver )
{

    PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		24,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		32,								// 32-bit z-buffer	
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };
    int  pixelformat;

	if ( minidriver )
	{
		if ( (pixelformat = qwglChoosePixelFormat( g_hDC, &pfd)) == 0 )
		{
			return false;
		}
		if ( qwglSetPixelFormat( g_hDC, pixelformat, &pfd) == FALSE )
		{
			return false;
		}
		qwglDescribePixelFormat( g_hDC, pixelformat, sizeof( pfd ), &pfd );
	}
	else
	{
		if ( ( pixelformat = ChoosePixelFormat( g_hDC, &pfd)) == 0 )
		{
			return false;
		}
		if ( SetPixelFormat( g_hDC, pixelformat, &pfd) == FALSE )
		{
			return false;
		}
		DescribePixelFormat( g_hDC, pixelformat, sizeof( pfd ), &pfd );
	}

	if ( ( g_hGLRC = qwglCreateContext( g_hDC ) ) == 0 )
	{
		goto fail;
	}

    if ( !qwglMakeCurrent( g_hDC, g_hGLRC ) )
	{
		goto fail;
	}

	return true;

fail:
	if ( g_hGLRC )
	{
		qwglDeleteContext( g_hGLRC );
		g_hGLRC = NULL;
	}

	if ( g_hDC )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}

	return false;
}

int AddDriver( char *filename )
{
	HINSTANCE			hInstance;
	gl_driver_info_t	driver;
	char				ref_name[] = "ref_gl", drivername[64], *p;
	int					minidriver;

	if( !CreateTempWindow( global_hInstance ) )
	{
		return true;
	}

	hInstance = LoadLibrary( filename );

	if( hInstance == NULL )
	{
		return true;
	}
	
	BindGLFunctions( hInstance );

	if( qglGetString == NULL )
	{
		ShutDownGL( hInstance );

		FreeLibrary( hInstance );

		qglGetString			= NULL;
		qwglChoosePixelFormat	= NULL;
		qwglDescribePixelFormat	= NULL;
		qwglSetPixelFormat		= NULL;
		qwglCreateContext		= NULL;
		qwglMakeCurrent			= NULL;
		qwglDeleteContext		= NULL;
		return true;
	}

	if( strstr( filename, "opengl32.dll" ) )
	{
		minidriver = false;
	}
	else
	{
		minidriver = true;
	}

	InitGL( minidriver );

	// SCG[6/29/99]: Strip the .dll off of the filename
	strncpy( drivername, filename, 64 );
	p = strstr( drivername, ".dll" );
	if( p )
	{
		p[0] = 0x00;
	}

	// SCG[6/29/99]: strip the leading ./ off of file names added by DoDirectoryWork()
	if( strstr( drivername, "./" ) )
	{
		p = &drivername[2];
	}
	else
	{
		p = &drivername[0];
	}

	driver.szFilename.Set( p );
	if( qglGetString( GL_VENDOR ) )
	{
		driver.szVendor.Set( ( const char * ) qglGetString( GL_VENDOR ) );
	}
	else
	{
		ShutDownGL( hInstance );
		return true;
	}

	if( qglGetString( GL_RENDERER ) )
	{
		driver.szRenderer.Set( ( const char * ) qglGetString( GL_RENDERER ) );
	}
	else
	{
		ShutDownGL( hInstance );
		return true;
	}

	if( qglGetString( GL_VERSION ) )
	{
		driver.szVersion.Set( ( const char * ) qglGetString( GL_VERSION ) );
	}
	else
	{
		ShutDownGL( hInstance );
		return true;
	}

	if( qglGetString( GL_EXTENSIONS ) )
	{
		driver.szExtensions.Set( ( const char * ) qglGetString( GL_EXTENSIONS ) );
	}
	else
	{
		ShutDownGL( hInstance );
		return true;
	}

	gl_driver_info.Add( driver );

	gl_driver_info_count++;

	ShutDownGL( hInstance );
	return true;
}

int DoDirectoryWork (char *directory, char *filespec, char *skip, int (*function)(char *))
{
	HANDLE			dirHandle;
	WIN32_FIND_DATA	findData;
	int				found = 1;
	int				lastChar;
	char			temp[128];
	char			filename[1024];
	//FILE			*reject_fp;
	//char			reject_files[128][_MAX_PATH];
	int				num_reject_files; //, test;
	BOOL			skip_file = FALSE;	//amw - needs to be initialized because it's used below!!!

	num_reject_files = 0;

// SCG[1/16/00]: 	sprintf(temp, "%s\\%s", directory, filespec);
	Com_sprintf(temp, sizeof(temp),"%s\\%s", directory, filespec);

	dirHandle = FindFirstFile(temp, &findData);

	if (dirHandle == INVALID_HANDLE_VALUE)
		return 0;

	while (found)
	{
		// if this is a direcory then recuruse
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			lastChar = strlen(findData.cFileName) - 1;
			if ((findData.cFileName[0]) != '.' && (findData.cFileName[lastChar] != '_') && !skip_file)
			{
//			    sprintf(temp, "%s\\%s", directory, findData.cFileName);
			    Com_sprintf(temp, sizeof(temp), "%s\\%s", directory, findData.cFileName);
				DoDirectoryWork(temp, filespec, skip, function);
			}
		}
		else
		{
			strlwr (findData.cFileName);
			strlwr (skip);

			lastChar = strlen(findData.cFileName) - 5;
			
			if( strstr (findData.cFileName, skip ) )
			{
				found = FindNextFile(dirHandle, &findData);
				continue;
			}

			if( findData.cFileName[lastChar] == '_' )
			{
				found = FindNextFile(dirHandle, &findData);
				continue;
			}
			
// SCG[1/16/00]: 			sprintf (filename, "%s/%s", directory, findData.cFileName);
			Com_sprintf (filename, sizeof(filename),"%s/%s", directory, findData.cFileName);

			if (!function (filename ))
				return 0;
		}

		found = FindNextFile(dirHandle, &findData);
	}

	FindClose(dirHandle);

	return 1;
}

extern cvar_t	*fs_basedir;

void DK_EnumerateGLDrivers()
{	
	char skipfile[] = "noskipfile";

	g_hDC = NULL;
	g_hWnd = NULL;
	g_hGLRC = NULL;

	// SCG[6/30/99]: Always add the default opengl32
	AddDriver( "opengl32.dll" );

	// SCG[6/30/99]: Search for other drivers.
	DoDirectoryWork( ".", "*.dll", skipfile, AddDriver );
}