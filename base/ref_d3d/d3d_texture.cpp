// d3d_texture.cpp: implementatio7n of the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "d3d_local.h"
#include "d3dutil.h"

#include "dk_ref_common.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_ref_pic.h"
#include "dk_ref_wal.h"

#include "d3d_texture.h"



//////////////////////////////////////////////////////////////////////
// TYPEDEFS
//////////////////////////////////////////////////////////////////////
struct TEXTURESEARCHINFO
{
	DWORD dwDesiredBPP; // Input for texture format search
	BOOL  bUseAlpha;
	BOOL  bUsePalette;
	BOOL  bUseFourCC;
	BOOL  bFoundGoodFormat;

	DDPIXELFORMAT* pddpf; // Result of texture format search
};



//////////////////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////////////////

array<CTexture>	loaded_textures;			// list of loaded textures

const CTexture	*texture_notexture;			// default texture

//////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////
static			HRESULT CALLBACK TextureEnumCallback( DDPIXELFORMAT *pddpf, VOID *param );
const CTexture	*LoadTexture( const char *name, resource_t resource_flag, int bpp );



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTexture::CTexture()
{
	bits_per_pixel	= 32;
	resource		= RESOURCE_GLOBAL;
	stage			= 0;
	lpddsSurface	= NULL;
	lpd3dtTexture	= NULL;
}



CTexture::~CTexture()
{
	SAFE_RELEASE( lpddsSurface );
	SAFE_RELEASE( lpd3dtTexture );
}



//-----------------------------------------------------------------------------
// Name: Compare
// Desc: compares the filenames of two textures
//-----------------------------------------------------------------------------
int CTexture::Compare(const CTexture **item1, const CTexture **item2) 
{
    return stricmp((*item1)->FileName(), (*item2)->FileName());
}



//-----------------------------------------------------------------------------
// Name: Find
// Desc: finds a texture
//-----------------------------------------------------------------------------
int CTexture::Find( const CTexture *item, const void *id )
{
    //the id is a character string.
    const char *name = (const char *)id;

    //check if the pic's name is equal to the passed in name.
    return stricmp(name, item->FileName());
}



//-----------------------------------------------------------------------------
// Name: Surface
// Desc: returns a pointer to the direct draw surface
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE4 CTexture::Surface() const
{
	return lpddsSurface;
}



//-----------------------------------------------------------------------------
// Name: Texture
// Desc: returns a texture to the direct3d texture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE2 CTexture::Texture() const
{
	return lpd3dtTexture;
}



//-----------------------------------------------------------------------------
// Name: FileName
// Desc:
//-----------------------------------------------------------------------------
const char *CTexture::FileName() const 
{
    return filename;
}



//-----------------------------------------------------------------------------
// Name: Stage
// Desc: returns the texture stage
//-----------------------------------------------------------------------------
int CTexture::Stage() const 
{
    return stage;
}



//-----------------------------------------------------------------------------
// Name: Stage
// Desc: sets the texture width
//-----------------------------------------------------------------------------
void CTexture::Stage( int stage )
{
    this->stage = stage;
}



//-----------------------------------------------------------------------------
// Name: Width
// Desc: 
//-----------------------------------------------------------------------------
int CTexture::Width() const
{
	return width;
}



//-----------------------------------------------------------------------------
// Name: Height
// Desc: 
//-----------------------------------------------------------------------------
int CTexture::Height() const
{
	return height;
}



//-----------------------------------------------------------------------------
// Name: Invalidate
// Desc: Invalidates the texture data (used when changing video modes)
//       and recreates the texture
//-----------------------------------------------------------------------------
HRESULT CTexture::Invalidate()
{
    D3DDEVICEDESC		ddHwDesc, ddSwDesc;
    DWORD				dwDeviceCaps;
	DWORD				dwTextureCaps;

    ddHwDesc.dwSize = sizeof(D3DDEVICEDESC);
    ddSwDesc.dwSize = sizeof(D3DDEVICEDESC);
    
	// get the device capability info
	if( FAILED( d3d_state.lpd3dDevice->GetCaps( &ddHwDesc, &ddSwDesc ) ) )
        return E_FAIL;

	// check hardware/software device flags
    if( ddHwDesc.dwFlags ) 
	{
		dwTextureCaps	= ddHwDesc.dpcTriCaps.dwTextureCaps;
		dwDeviceCaps	= ddHwDesc.dwDevCaps;
	}
    else                   
	{
		dwTextureCaps	= ddSwDesc.dpcTriCaps.dwTextureCaps;
		dwDeviceCaps	= ddSwDesc.dwDevCaps;
	}

	// initialize the surface description
    DDSURFACEDESC2	ddsd;
    D3DUtil_InitSurfaceDesc( ddsd );

	// fill out the surface description
	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	ddsd.dwTextureStage  = stage;
	ddsd.dwWidth         = width;
	ddsd.dwHeight        = height;

	// check for nonlocal video memory (agp surfaces)
	if( dwDeviceCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM )
	    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM | DDSCAPS_ALLOCONLOAD;

	// check to see if the hardware requires data in powers of 2
    if( dwTextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for( ddsd.dwWidth = 1;  width > ddsd.dwWidth;   ddsd.dwWidth <<= 1 );
        for( ddsd.dwHeight = 1; height > ddsd.dwHeight; ddsd.dwHeight <<= 1 );
    }

	// check to see if the hardware requires data in a square only
    if( dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
    {
        if( ddsd.dwWidth > ddsd.dwHeight ) 
			ddsd.dwHeight = ddsd.dwWidth;
        else                               
			ddsd.dwWidth  = ddsd.dwHeight;
    }

		// fill out the texture search info structure
	TEXTURESEARCHINFO	tsi;
	tsi.dwDesiredBPP		= bits_per_pixel;
	tsi.bUseAlpha			= false;
	tsi.bUsePalette			= (bits_per_pixel == 8) ? true : false;
	tsi.bUseFourCC			= false;
	tsi.bFoundGoodFormat	= false;
	tsi.pddpf				= &ddsd.ddpfPixelFormat;
	
	// enumerate texture formats
    d3d_state.lpd3dDevice->EnumTextureFormats( TextureEnumCallback, &tsi );

	if( tsi.bFoundGoodFormat == false )
		return E_FAIL;

	// create the texture surface
    if( FAILED( d3d_state.lpdd->CreateSurface( &ddsd, &lpddsSurface, NULL ) ) )
        return E_FAIL;

	// get a pointer to the texture interface
    if( FAILED( lpddsSurface->QueryInterface( IID_IDirect3DTexture2, (VOID**)&lpd3dtTexture ) ) )
        return E_FAIL;

	if( dwDeviceCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM )
		lpd3dtTexture->Load( lpd3dtTexture );

	// init the surface description
    D3DUtil_InitSurfaceDesc( ddsd );
	lpddsSurface->GetSurfaceDesc( &ddsd );
	ddsd.dwFlags			= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps		= DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth			= width;
	ddsd.dwHeight			= height;

	LPDIRECTDRAWSURFACE4	lpddsTempSurface;
	HRESULT	hr;

	if( FAILED( hr = d3d_state.lpdd->CreateSurface( &ddsd, &lpddsTempSurface, NULL ) ) )
		return E_FAIL;

    BITMAP bm;
    GetObject( bitmap, sizeof(BITMAP), &bm );

    // Get a DC for the bitmap
    HDC hdcBitmap = CreateCompatibleDC( NULL );
    if( NULL == hdcBitmap )
    {
        lpddsTempSurface->Release();
        return NULL;
    }
    SelectObject( hdcBitmap, bitmap );

	// copy the bitmap to the surface
    HDC hdcSurface;
    if( SUCCEEDED( lpddsTempSurface->GetDC( &hdcSurface ) ) )
    {
        BitBlt( hdcSurface, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0,
                SRCCOPY );
        lpddsTempSurface->ReleaseDC( hdcSurface );
    }
    DeleteDC( hdcBitmap );

    // Copy the temp surface to the real texture surface
    lpddsSurface->Blt( NULL, lpddsTempSurface, NULL, DDBLT_WAIT, NULL );

	// release the temporary surface
    lpddsTempSurface->Release();

	DDCOLORKEY	ddck;
	DWORD		color;

	switch( bits_per_pixel )
	{
	case 8:
		color = 255;
		break;
	case 16:
		color = palette->colors[255].color16;
		break;
	case 32:
		color = RGB( 255, 0, 255 );
		break;
	default:
		color = 0L;
		break;
	}

	ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue = color;
	lpddsSurface->SetColorKey( DDCKEY_SRCBLT, &ddck );

	return S_OK;
}



//////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: TextureEnumCallback
// Desc: callback function for enumerating textures
//-----------------------------------------------------------------------------
static HRESULT CALLBACK TextureEnumCallback( DDPIXELFORMAT *pddpf, VOID *param )
{
    if( pddpf == NULL || param == NULL)
        return DDENUMRET_OK;

	TEXTURESEARCHINFO* ptsi = (TEXTURESEARCHINFO*)param;

    // Skip any funky modes
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;

	// Check for palettized formats
	if( ptsi->bUsePalette )
	{
		if( !( pddpf->dwFlags & DDPF_PALETTEINDEXED8 ) )
			return DDENUMRET_OK;

		// Accept the first 8-bit palettized format we get
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

	// Else, skip any formats with bpp less than desired bpp
	if( pddpf->dwRGBBitCount < ptsi->dwDesiredBPP )
		return DDENUMRET_OK;

	// Check for FourCC formats
/*
    if( ptsi->bUseFourCC )
	{
		if( pddpf->dwFourCC == 0 )
		    return DDENUMRET_OK;

		return DDENUMRET_CANCEL;
	}
*/
	// Else, skip any FourCC formats
	if( pddpf->dwFourCC != 0 )
		return DDENUMRET_OK;

	// Make sure current alpha format agrees with requested format type
	if( (ptsi->bUseAlpha==TRUE) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;
	if( (ptsi->bUseAlpha==FALSE) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;

    // Check if we found a good match
    if( pddpf->dwRGBBitCount == ptsi->dwDesiredBPP )
    {
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_OK;
}



//-----------------------------------------------------------------------------
// Name: CreateBitmap
// Desc: create a bitmap from the input
//-----------------------------------------------------------------------------
HRESULT CTexture::CreateBitmap( BYTE *data )
{
	BITMAPINFO	bmi;
	LONG		*bits;
    HDC			hdcImage;
	int			i, j;
	HBITMAP		*hbmp = &bitmap;
	
	hdcImage = CreateCompatibleDC(NULL);

	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			= width;
	bmi.bmiHeader.biHeight			= height;
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biBitCount		= bits_per_pixel;
	bmi.bmiHeader.biCompression		= BI_RGB;
	bmi.bmiHeader.biSizeImage		= bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * (bits_per_pixel / 8);
	bmi.bmiHeader.biXPelsPerMeter	= 4000;
	bmi.bmiHeader.biYPelsPerMeter	= 4000;
	bmi.bmiHeader.biClrUsed			= 0;
	bmi.bmiHeader.biClrImportant	= 0;

	(*hbmp) = CreateDIBSection( hdcImage, &bmi, DIB_RGB_COLORS, (VOID**)&bits, NULL, 0 );

	if(*hbmp == NULL )
	{
		DeleteDC( hdcImage );
		return E_FAIL;
	}

	const BYTE	*pSource;
	BYTE *pDest8 = (BYTE *)bits;
	uint16 *pDest16 = (uint16 *)bits;
	
	switch( bits_per_pixel )
	{
	case 8:
		for( i = height - 1; i >= 0; i-- )
		{
			pSource = data + width * i;

			for( j = 0; j < width; j++ )
				*pDest8++ = *pSource++;
		}
		break;
	case 16:
		for( i = height - 1; i >= 0; i-- )
		{
			pSource = data + width * i;

			for( j = 0; j < width; j++ )
				*pDest16++ = palette->colors[*pSource++].color16;
		}
		break;
	case 32:
		for( i = height - 1; i >= 0; i-- )
		{
			pSource = data + width * i;

			for( j = 0; j < width; j++, bits++, pSource++ )
			{
				((BYTE *)bits)[3] = 0;
				((BYTE *)bits)[2] = palette->colors[*pSource].r;
				((BYTE *)bits)[1] = palette->colors[*pSource].g;
				((BYTE *)bits)[0] = palette->colors[*pSource].b;
			}
		}
		break;
	}

	DeleteDC( hdcImage );

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CTexture *CTexture::LoadFromWal( const char *name, resource_t resource_flag, int bpp )
{
	CTexture	*texture = new CTexture;

	// new texture. load the .wal file
	CWal *wal = (CWal *) LoadWal( name, resource_flag );
	
	// make sure it's valid data
	if( wal == NULL )
	{
		// delete the texture object
		delete texture;

		// return null;
		return NULL;
	}

	// set the texture filename
	texture->filename = wal->FileName();

	// set the width, height and bits per pixel (bpp)
	texture->width			= wal->Width();
	texture->height			= wal->Height();
	texture->bits_per_pixel	= bpp;

	// set the texture palette
	texture->palette = (CPalette *) wal->Palette();

	// create the image data
	texture->CreateBitmap( (BYTE *) wal->Data() );

	// return the texture
	return texture;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CTexture *CTexture::LoadFromPic( const char *name, resource_t resource_flag, int bpp )
{
	CTexture	*texture = new CTexture;

	CPic *pic = (CPic *) LoadPic( name, resource_flag );

	// make sure it's valid data
	if( pic == NULL )
	{
		// delete the texture object
		delete texture;

		// return null;
		return NULL;
	}

	// set the texture filename
	texture->filename = pic->FileName();

	// set the width, height and bits per pixel (bpp)
	texture->width			= pic->Width();
	texture->height			= pic->Height();
	texture->bits_per_pixel	= bpp;
		
	// set the texture palette
	texture->palette = (CPalette *) pic->Palette();

	// create the image data
	texture->CreateBitmap( (BYTE *) pic->Data() );

	// return the texture
	return texture;
}


//-----------------------------------------------------------------------------
// Name: TexturePathName
// Desc:
//-----------------------------------------------------------------------------
void TexturePathName(const char *name, buffer256 &filename) 
{
    //if the name does not have a slash at the front, we assume
    //it is in the pics directory and has a pcx extension.
    if (name[0] != '\\' && name[0] != '/') 
	{
		if( strstr( name, ".pcx" ) )
		{
			//add the pics/ at front
			filename.Set("pics/%s", name);
		}
		else
		{
			//add the pics/ at front and the .pcx at the end.
			filename.Set("pics/%s.pcx", name);
		}
    }
    else 
	{
        //assume the given name has the path and extension included.
        filename = name;
    }
}



//-----------------------------------------------------------------------------
// Name: LoadTexture
// Desc: loads the texture data from a .wal file
//-----------------------------------------------------------------------------
const CTexture *LoadTexture( const char *name, resource_t resource_flag, int bpp )
{
	buffer256	filename;
	BOOL		load_from_wal;

	if( !strstr( name, ".wal" ) )
		TexturePathName( name, filename );
	else
		filename.Set( "%s", name );

	//  try to find the texture
	CTexture	*texture = loaded_textures.Item( (const char *) filename, CTexture::Find );

	// see if we load from a .wal or not
	if( strstr( name, ".wal" ) )
		load_from_wal = true;
	else
		load_from_wal = false;

	// if we found it, were done.
	if( texture != NULL )
	{
		if( load_from_wal )
		{
			// try to find the matching wal
			CWal *wal = loaded_wals.Item( (const char *) filename, CWal::Find );
			
			if( wal != NULL)
				wal->Flag( resource_flag );
		}
		else 
		{
			CPic *pic = loaded_pics.Item( (const char *) filename, CPic::Find );

			if( pic != NULL )
				pic->Flag( resource_flag );

		}
		return texture;
	}

	// create a new texture object
	if( load_from_wal )
		texture = CTexture::LoadFromWal( filename, resource_flag, bpp );
	else
		texture = CTexture::LoadFromPic( filename, resource_flag, bpp );

	if( texture == NULL )
		return NULL;

		// add it to the array of loaded textures
	loaded_textures.Add( texture, CTexture::Compare );

	// create the texture surface, etc...
	texture->Invalidate();

	return texture;

}



