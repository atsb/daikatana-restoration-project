// Texture.h: interface for the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <ddraw.h>
#include <d3d.h>

class CTexture : public object_reference 
{
public:
	// construction
	CTexture();
	virtual ~CTexture();

	HRESULT					Invalidate();
	HRESULT					CreateBitmap( BYTE *data );

	static int				Compare(const CTexture **item1, const CTexture **item2);	// compares the filenames of two textures
	static int				Find( const CTexture *item, const void *id );				// finds a texture
	
	static CTexture *		LoadFromWal( const char *name, resource_t resource_flag, int bpp );
	static CTexture *		LoadFromPic( const char *name, resource_t resource_flag, int bpp );

	//
	// member modification functions
	//
	void					Stage( int stage );											// sets texture stage
	//
	// member access functions
	//
    const char				*FileName() const;											// returns the filename
	int						Stage() const;												// returns texture stage
	int						Width() const;												// returns texture width
	int						Height() const;												// returns texture height
	LPDIRECTDRAWSURFACE4	Surface() const;											// returns a pointer to the direct draw surface
	LPDIRECT3DTEXTURE2		Texture() const;											// returns a pointer to the direct draw palette


private:
	buffer256				filename;			// wal filename
	resource_t				resource;			// resource flag
	CPalette				*palette;
	
	int						bits_per_pixel;		// bits per pixel
	int						stage;				// texture stage
	int						width;
	int						height;
	LPDIRECTDRAWSURFACE4	lpddsSurface;		// surface of the texture
	LPDIRECT3DTEXTURE2		lpd3dtTexture;		// direct3D texture for the texture
    HBITMAP					bitmap;				// image data
};

const CTexture *LoadTexture( const char *name, resource_t resource_flag, int bpp );
