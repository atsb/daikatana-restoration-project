#include <ddraw.h>
#include <d3d.h>

#include "d3d_local.h"
#include "d3d_enum.h"
#include "d3d_error.h"

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }



//-----------------------------------------------------------------------------
// Name: DD_CreateDirectDrawObject
// Desc: Creates the direct draw object
//-----------------------------------------------------------------------------
BOOL DD_CreateDirectDrawObject( GUID *pDriverGUID )
{
	HRESULT			hr;
	LPDIRECTDRAW	lpdd;

	if( FAILED( hr = DirectDrawCreate( d3d_state.pDriverGUID, &lpdd, NULL ) ) )
		return CheckDirectDrawResult( hr );

	if( FAILED( hr = lpdd->QueryInterface( IID_IDirectDraw4, (VOID **)&d3d_state.lpdd ) ) )
		return CheckDirectDrawResult( hr );
	
	SAFE_RELEASE( lpdd );

	return true;
}



//-----------------------------------------------------------------------------
// Name: DD_SetCooperativeLevel
// Desc: Sets the cooperative level based on fullscreen or windowed mode
//-----------------------------------------------------------------------------
BOOL DD_SetCooperativeLevel()
{
	HRESULT	hr;
	DWORD	dwCoopFlags = 0;

	dwCoopFlags = DDSCL_FPUSETUP;
	
	if( d3d_state.bWindowed )
		dwCoopFlags = DDSCL_NORMAL;
	else
        dwCoopFlags = DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

	if( FAILED( hr = d3d_state.lpdd->SetCooperativeLevel( d3d_state.hWnd, dwCoopFlags ) ) )
		return CheckDirectDrawResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_GetDirect3DInterface
// Desc: Gets an interface to the direct3d object
//-----------------------------------------------------------------------------
BOOL D3D_GetDirect3DInterface()
{
	HRESULT hr;

	if( FAILED( hr = d3d_state.lpdd->QueryInterface( IID_IDirect3D3, (VOID **)&d3d_state.lpd3d ) ) )
		return CheckDirectDrawResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_FindDevice
// Desc: Finds the 3d device
//-----------------------------------------------------------------------------
BOOL D3D_FindDevice()
{
	HRESULT	hr;
	D3DFINDDEVICERESULT  dr;
	D3DFINDDEVICESEARCH  ds;

	ZeroMemory( &dr, sizeof(D3DFINDDEVICERESULT) );
	ZeroMemory( &ds, sizeof(D3DFINDDEVICESEARCH) );
	dr.dwSize	= sizeof(D3DFINDDEVICERESULT);
	ds.dwSize	= sizeof(D3DFINDDEVICESEARCH);
	ds.dwFlags	= D3DFDS_GUID;
	CopyMemory( &ds.guid, d3d_state.pDeviceGUID, sizeof(GUID) );

	if( d3d_state.bHardware )
	{
		ds.dwFlags |= D3DFDS_HARDWARE;
		ds.bHardware = true;
	}

    if( FAILED( hr = d3d_state.lpd3d->FindDevice( &ds, &dr) ) )
		return CheckDirect3DResult( hr );

	if( dr.ddHwDesc.dwFlags == 0 )
	{
		if( d3d_state.bHardware )
			return false;

		memcpy( &d3d_state.ddDeviceDesc, &dr.ddSwDesc, sizeof(D3DDEVICEDESC) );		
	}
	else
		memcpy( &d3d_state.ddDeviceDesc, &dr.ddHwDesc, sizeof(D3DDEVICEDESC) );		

	return true;
}



//-----------------------------------------------------------------------------
// Name: DD_CreateSurfaces
// Desc: Creates the surfaces and sets the rendering target
//-----------------------------------------------------------------------------
BOOL DD_CreateSurfaces( BOOL bInit3d )
{
	HRESULT	hr;

/*
	if( d3d_state.bWindowed )
	{
		GetClientRect( d3d_state.hWnd, &d3d_state.view_rect );
		GetClientRect( d3d_state.hWnd, &d3d_state.window_rect );
		ClientToScreen( d3d_state.hWnd, (POINT*) &d3d_state.window_rect.left );
		ClientToScreen( d3d_state.hWnd, (POINT*) &d3d_state.window_rect.right );
	}
	else
	{
		SetRect( &d3d_state.view_rect, 0, 0, d3d_state.width, d3d_state.height );
		memcpy( &d3d_state.window_rect, &d3d_state.view_rect, sizeof(RECT) );
		ClientToScreen( d3d_state.hWnd, (POINT*) &d3d_state.window_rect.left );
		ClientToScreen( d3d_state.hWnd, (POINT*) &d3d_state.window_rect.right );
	}
*/
	SetRect( &d3d_state.view_rect, 0, 0, d3d_state.width, d3d_state.height );
	memcpy( &d3d_state.window_rect, &d3d_state.view_rect, sizeof(RECT) );

	GetClientRect( d3d_state.hWnd, &d3d_state.window_rect );
	ClientToScreen( d3d_state.hWnd, (POINT*)&d3d_state.window_rect.left );
	ClientToScreen( d3d_state.hWnd, (POINT*)&d3d_state.window_rect.right );

	if( !d3d_state.bWindowed )
		if( FAILED( hr = d3d_state.lpdd->SetDisplayMode( d3d_state.width, d3d_state.height, d3d_state.bpp, 0, 0 ) ) ) 
			return CheckDirectDrawResult( hr );

	DDSURFACEDESC2	ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize                 = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags                = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps			= DDSCAPS_PRIMARYSURFACE;
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	if( bInit3d )
		ddsd.ddsCaps.dwCaps	|= DDSCAPS_3DDEVICE;

	if( d3d_state.bHardware )
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	if( !d3d_state.bWindowed && d3d_state.buffer_count )
	{
		ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = d3d_state.buffer_count;
	}

	if( FAILED( hr = d3d_state.lpdd->CreateSurface( &ddsd, &d3d_state.lpddsFront, NULL ) ) )
		return CheckDirectDrawResult( hr );

	if( d3d_state.bWindowed )
	{
		LPDIRECTDRAWCLIPPER		lpddClipper;

		if( FAILED( hr = d3d_state.lpdd->CreateClipper( 0, &lpddClipper, NULL ) ) )
			return CheckDirectDrawResult( hr );

		if( FAILED( hr = lpddClipper->SetHWnd( 0, d3d_state.hWnd ) ) ) 
			return CheckDirectDrawResult( hr );

		if( FAILED( hr = d3d_state.lpddsFront->SetClipper( lpddClipper ) ) )
			return CheckDirectDrawResult( hr );
		
		SAFE_RELEASE( lpddClipper );
	}

	if( d3d_state.buffer_count )
	{
		if( d3d_state.bWindowed )
		{
			ddsd.dwFlags		= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwWidth		= d3d_state.width;
			ddsd.dwHeight		= d3d_state.height;
			ddsd.ddsCaps.dwCaps	= DDSCAPS_OFFSCREENPLAIN;

			if( bInit3d )
				ddsd.ddsCaps.dwCaps	|= DDSCAPS_3DDEVICE;

			if( d3d_state.bHardware )
				ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			else
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

			if( FAILED( hr = d3d_state.lpdd->CreateSurface( &ddsd, &d3d_state.lpddsBack, NULL ) ) )
				return CheckDirectDrawResult( hr );

		}
		else
		{
			DDSCAPS2	ddscaps;
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
			if( FAILED( hr = d3d_state.lpddsFront->GetAttachedSurface( &ddscaps, &d3d_state.lpddsBack ) ) )
				return CheckDirectDrawResult( hr );
		}

		d3d_state.lpddsRender = d3d_state.lpddsBack;
		d3d_state.lpddsRender->AddRef();
	}
	else
	{
		d3d_state.lpddsRender = d3d_state.lpddsFront;
		d3d_state.lpddsRender->AddRef();
	}

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_CreateDirect3DDevice
// Desc: Creates the drect 3d d3vice
//-----------------------------------------------------------------------------
BOOL D3D_CreateDirect3DDevice()
{
	HRESULT	hr;

	if( FAILED( hr = d3d_state.lpd3d->CreateDevice( *d3d_state.pDeviceInfo->pGUID, d3d_state.lpddsRender, &d3d_state.lpd3dDevice, NULL ) ) )
		return CheckDirect3DResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_CreateViewport
// Desc: Creates the viewport
//-----------------------------------------------------------------------------
BOOL D3D_CreateViewport()
{
	HRESULT			hr;
	D3DVIEWPORT2	vp;

    ZeroMemory( &vp, sizeof(D3DVIEWPORT2) );
    vp.dwSize		= sizeof(D3DVIEWPORT2);
    vp.dwWidth		= d3d_state.width;
    vp.dwHeight		= d3d_state.height;
    vp.dvMaxZ		= 1.0f;
    vp.dvClipX      = -1.0f;
    vp.dvClipWidth  = 2.0f;
    vp.dvClipY      = 1.0f;
    vp.dvClipHeight = 2.0f;

	if( FAILED( hr = d3d_state.lpd3d->CreateViewport( &d3d_state.lpd3dViewport, NULL ) ) )
		return CheckDirect3DResult( hr );

	if( FAILED( hr = d3d_state.lpd3dDevice->AddViewport( d3d_state.lpd3dViewport ) ) )
		return CheckDirect3DResult( hr );

	if( FAILED( hr = d3d_state.lpd3dViewport->SetViewport2( &vp ) ) )
		return CheckDirect3DResult( hr );

	if( FAILED( hr = d3d_state.lpd3dDevice->SetCurrentViewport( d3d_state.lpd3dViewport ) ) )
		return CheckDirect3DResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_CreateZBuffer
// Desc: Creates the z buffer
//-----------------------------------------------------------------------------
BOOL D3D_CreateZBuffer()
{
	HRESULT	hr;

	if( !d3d_state.bZBuffered )
		return true;

	d3d_state.ddpfZBuffer.dwFlags = DDPF_ZBUFFER;

    d3d_state.lpd3d->EnumZBufferFormats( *d3d_state.pDeviceGUID, EnumZBufferFormatsCallback, (VOID*)&d3d_state.ddpfZBuffer );

    DWORD dwRasterCaps = d3d_state.ddDeviceDesc.dpcTriCaps.dwRasterCaps;
    if( dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )
        return true;

	DDSURFACEDESC2	ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize                 = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags                = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps			= DDSCAPS_ZBUFFER;
    ddsd.dwWidth				= d3d_state.width;
    ddsd.dwHeight				= d3d_state.height;
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    memcpy( &ddsd.ddpfPixelFormat, &d3d_state.ddpfZBuffer, sizeof(DDPIXELFORMAT) );

	if( d3d_state.bHardware )
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    if( FAILED( hr = d3d_state.lpdd->CreateSurface( &ddsd, &d3d_state.lpddsZBuffer, NULL ) ) )
		return CheckDirectDrawResult( hr );

    if( FAILED( hr = d3d_state.lpddsRender->AddAttachedSurface( d3d_state.lpddsZBuffer ) ) )
		return CheckDirectDrawResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_CreateVertexBuffer
// Desc: Creates the vertex buffers
//-----------------------------------------------------------------------------
BOOL D3D_CreateVertexBuffer()
{
	HRESULT hr;
	D3DVERTEXBUFFERDESC vbdesc;

	ZeroMemory(&vbdesc, sizeof(D3DVERTEXBUFFERDESC));
	vbdesc.dwSize			= sizeof(D3DVERTEXBUFFERDESC);
	vbdesc.dwCaps			= 0L;
	vbdesc.dwFVF			= D3DFVF_VERTEX;
	vbdesc.dwNumVertices	= 32768;    

	if( !d3d_state.bHardware )
		vbdesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;    

	if( FAILED( hr = d3d_state.lpd3d->CreateVertexBuffer( &vbdesc, &d3d_state.lpd3dvbVerts, 0L, NULL ) ) )
		return CheckDirect3DResult( hr );

    vbdesc.dwFVF = D3DFVF_TLVERTEX;

	if( FAILED( hr = d3d_state.lpd3d->CreateVertexBuffer( &vbdesc, &d3d_state.lpd3dvbFinalVerts, 0L, NULL ) ) )
		return CheckDirect3DResult( hr );

	if( FAILED( hr = d3d_state.lpd3dvbVerts->Lock( DDLOCK_WAIT, (void**)&d3d_state.pVerts, &d3d_state.dwSizeVerts ) ) )
		return CheckDirect3DResult( hr );

	return true;
}



//-----------------------------------------------------------------------------
// Name: D3D_InitializeAll
// Desc: Initializes all objects necessary for direct3d
//-----------------------------------------------------------------------------
BOOL D3D_InitializeAll()
{
	if( !DD_CreateDirectDrawObject( NULL ) )
		return false;

	if( !DD_SetCooperativeLevel() )
		return false;

	if( !D3D_GetDirect3DInterface() )
		return false;

	if( !D3D_FindDevice() )
		return false;

	if( !DD_CreateSurfaces( true ) )
		return false;

	if( !D3D_CreateDirect3DDevice() )
		return false;

	if( !D3D_CreateViewport() )
		return false;

	if( !D3D_CreateVertexBuffer() )
		return false;

	return true;
}
