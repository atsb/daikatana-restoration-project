#include "d3d_local.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"

#include "dk_ref_pic.h"
#include "d3dutil.h"

const CPic		*pic_characters;				// console characters

extern const CTexture	*texture_notexture;

qboolean	in2dmode;

extern	void *char_flare_image = NULL;
extern	float	ref_laserRotation;

void *default_chars_image;



//-----------------------------------------------------------------------------
// Name: Enter2dMode
// Desc: 
//-----------------------------------------------------------------------------
void Enter2dMode(){
	LPDIRECT3DMATERIAL3 render_material;
	D3DVECTOR	vEyePt, vLookatPt, vUpVec;
	D3DMATRIX	matView, matProj;
	HRESULT		hr;

	if( in2dmode )
		return;

	in2dmode = true;

	vEyePt.x    = 0.0f;
	vEyePt.y    = 0.0f;
	vEyePt.z    = -20.0f;

	vLookatPt.x = 0.0f;
	vLookatPt.y = 0.0f;
	vLookatPt.z = 0.0f;

	vUpVec.x    = 0.0f;
	vUpVec.y    = 0.0f;
	vUpVec.z    = 1.0f;

	if( FAILED( d3d_state.lpd3d->CreateMaterial( &render_material, NULL) ) )
		return;

	D3DVIEWPORT2 vp;
	vp.dwSize = sizeof(vp);
	d3d_state.lpd3dViewport->GetViewport2(&vp);
	FLOAT fAspect = ((FLOAT)vp.dwHeight) / vp.dwWidth;

	D3DMATERIAL       mtrl;
	D3DMATERIALHANDLE hmtrl;
	D3DUtil_InitMaterial( mtrl );
	render_material->SetMaterial( &mtrl );
	render_material->GetHandle( d3d_state.lpd3dDevice, &hmtrl );
	d3d_state.lpd3dDevice->SetLightState( D3DLIGHTSTATE_MATERIAL, hmtrl );

	D3DUtil_SetViewMatrix( matView, vEyePt, vLookatPt, vUpVec );
	if(FAILED( hr = d3d_state.lpd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &matView ) ) )
		DEBUG_MSG( "d3d_state.lpd3dDevice->SetTransform : D3DTRANSFORMSTATE_VIEW failed!" );

//	D3DUtil_SetProjectionMatrix( matProj );
	D3DUtil_SetProjectionMatrix( matProj, 0.75f, fAspect, 4.0f, 4096.0f );
	if(FAILED( hr = d3d_state.lpd3dDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj ) ) )
		DEBUG_MSG( "d3d_state.lpd3dDevice->SetTransform : D3DTRANSFORMSTATE_PROJECTION failed!" );

	// Set any appropiate state
	d3d_state.lpd3dDevice->SetLightState(  D3DLIGHTSTATE_AMBIENT,     0xffffffff );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );
	d3d_state.lpd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	d3d_state.lpd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	d3d_state.lpd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
}



//-----------------------------------------------------------------------------
// Name: Draw_InitLocal
// Desc: 
//-----------------------------------------------------------------------------
void Leave2dMode(){
	in2dmode = false;
}



//-----------------------------------------------------------------------------
// Name: Draw_InitLocal
// Desc: 
//-----------------------------------------------------------------------------
void Draw_InitLocal(){
    //load the character pic.
    if (r_resourcedir == NULL || r_resourcedir->string == NULL || r_resourcedir->string[0] == '\0'){
        //get the character file out of the default directory.
        pic_characters = LoadPic("dkchars", RESOURCE_GLOBAL);
    }
    else{
        //make the full path name of the character file.
        buffer256 path("%sdkchars.pcx", r_resourcedir->string);

        //load the pic.
        pic_characters = LoadPic(path, RESOURCE_GLOBAL);

        //check if we got it.
        if (pic_characters == NULL){
            //try the default directory.
            pic_characters = LoadPic("dkchars", RESOURCE_GLOBAL);
        }
    }
	
	texture_notexture = LoadTexture( "textures\\miptest\\wall01.wal", RESOURCE_LEVEL, d3d_state.bpp );
}

//-----------------------------------------------------------------------------
// Name: DrawStretchPic
// Desc: 
//-----------------------------------------------------------------------------
void DrawStretchPic( int x, int y, int w, int h, const char *name, resource_t resource ){
	D3DTLVERTEX		poly[4];
	D3DVIEWPORT2	vp;

	// don't try to load NULL 
	if (name == NULL || name[0] == '\0') 
		return;

	// setup view and projection for 2d
	Enter2dMode();

	// load the texture
	const CTexture *t = LoadTexture( name, resource, d3d_state.bpp);

	// get viewport information
	vp.dwSize = sizeof(vp);
	d3d_state.lpd3dViewport->GetViewport2(&vp);

	// set default info for each vertex
	for( int i = 0; i < 4; i++ ){
		poly[i].sx = x;
		poly[i].sy = y;
		poly[i].sz = 0.5;
		poly[i].rhw = 0.5;
		poly[i].color = -1;
		poly[i].specular = 0;
	}

	// make sure it's valid data
	if( t != NULL ){
		// set vertex information
		poly[2].sx = (FLOAT) x + w;
		poly[3].sx = (FLOAT) x + w;
		poly[0].sy = (FLOAT) y + h;
		poly[2].sy = (FLOAT) y + h;

		// set uv values
		poly[0].tu = poly[1].tu = poly[1].tv = poly[3].tv = 0;
		poly[0].tv = poly[2].tu = poly[2].tv = poly[3].tu = 1;

		// set the current texture
		d3d_state.lpd3dDevice->SetTexture( t->Stage(), t->Texture() );
	}
	else{
		for( i = 0; i < 4; i++ )
			poly[i].tu = poly[i].tv = 0;

		poly[2].sx = x + 8;
		poly[3].sx = x + 8;
		poly[0].sy = y + 8;
		poly[2].sy = y + 8;
	}

	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
	d3d_state.lpd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, poly, 4, 0 );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
}



//-----------------------------------------------------------------------------
// Name: DrawPic
// Desc: 
//-----------------------------------------------------------------------------
void DrawPic( int x, int y, const char *name, resource_t resource ){
	D3DTLVERTEX		poly[4];
	
	// don't try to load NULL 
	if (name == NULL || name[0] == '\0') 
		return;

	// setup view and projection for 2d
	Enter2dMode();

	// load the texture
 	const CTexture *t = LoadTexture( name, resource, d3d_state.bpp );
	
	// set default info for each vertex
	for( int i = 0; i < 4; i++ )
	{
		poly[i].sx = x;
		poly[i].sy = y;
		poly[i].sz = 0.5;
		poly[i].rhw = 0.5;
		poly[i].color = -1;
		poly[i].specular = 0;
	}
	
	// make sure it's valid data
	if( t != NULL ){
		// set vertex information
		poly[2].sx = (FLOAT) x + t->Width();
		poly[3].sx = (FLOAT) x + t->Width();
		poly[0].sy = (FLOAT) y + t->Height();
		poly[2].sy = (FLOAT) y + t->Height();

		// set uv values
		poly[0].tu = poly[1].tu = poly[1].tv = poly[3].tv = 0;
		poly[0].tv = poly[2].tu = poly[2].tv = poly[3].tu = 1;

		// set the current texture
		d3d_state.lpd3dDevice->SetTexture( t->Stage(), t->Texture() );
	}
	else{
		for( i = 0; i < 4; i++ )
			poly[i].tu = poly[i].tv = 0;

		poly[2].sx = x + 8;
		poly[3].sx = x + 8;
		poly[0].sy = y + 8;
		poly[2].sy = y + 8;
	}

	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
	d3d_state.lpd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, poly, 4, 0 );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
}



//-----------------------------------------------------------------------------
// Name: DrawPicPortion
// Desc: 
//-----------------------------------------------------------------------------
void DrawPicPortion( int destx, int desty, int src_left, int src_top, int src_width, int src_height, void *font ){
    //the image we use as texture.
    CPic *pic;
	D3DTLVERTEX		poly[4];

	// set default info for each vertex
	for( int i = 0; i < 4; i++ ){
		poly[i].sx = destx;
		poly[i].sy = desty;
		poly[i].sz = 0.5;
		poly[i].rhw = 0.5;
		poly[i].color = -1;
		poly[i].specular = 0;
	}

    //the floating point texture coordinates used for the poly.
    float tex_left, tex_right, tex_top, tex_bottom;

    //the width and height of the used portion of the texture.
    float tex_width, tex_height;

	// make sure the image is valid
    if (font == NULL) 
		return;

    // get the image.
    pic = (CPic *)font;

	// setup view and projection for 2d
	Enter2dMode();

	// load the texture
	const CTexture *t = LoadTexture( pic->FileName(), RESOURCE_GLOBAL, d3d_state.bpp );

	// make sure it's valid data
	if( t != NULL ){
		//get the width and height of the used part of the texture. 
		tex_width = src_width / (float) (pic->Width() );
		tex_height = src_height / (float) (pic->Height() );

		//compute the texture coordinates.
		tex_left = src_left / (float) ( pic->Width() );
		tex_top = src_top / (float) ( pic->Height() );
		tex_right = tex_left + tex_width;
		tex_bottom = tex_top + tex_height;

		// set vertex information
		poly[2].sx = destx + src_width;
		poly[3].sx = destx + src_width;
		poly[0].sy = desty + src_height;
		poly[2].sy = desty + src_height;

		// set uv values
		poly[0].tu = tex_left;
		poly[0].tv = tex_bottom;
		poly[1].tu = tex_left;
		poly[1].tv = tex_top;
		poly[2].tu = tex_right;
		poly[2].tv = tex_bottom;
		poly[3].tu = tex_right;
		poly[3].tv = tex_top;

		// set the current texture
		d3d_state.lpd3dDevice->SetTexture( t->Stage(), t->Texture() );
	}
	else{
		for( i = 0; i < 4; i++ )
			poly[i].tu = poly[i].tv = 0;

		poly[2].sx = destx + 8;
		poly[3].sx = destx + 8;
		poly[0].sy = desty + 8;
		poly[2].sy = desty + 8;
	}

	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
	d3d_state.lpd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, poly, 4, 0 );
	d3d_state.lpd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
}



//-----------------------------------------------------------------------------
// Name: DrawStretchPicPortion
// Desc: 
//-----------------------------------------------------------------------------
void DrawStretchPicPortion( int destx, int desty, float scale_x, float scale_y, 
    int src_left, int src_top, int src_width, int src_height, void *image ){
}



//-----------------------------------------------------------------------------
// Name: Draw_AlphaPicPortion 
// Desc: 
//-----------------------------------------------------------------------------
void Draw_AlphaPicPortion( int destx, int desty, int src_left, int src_top, int src_width, int src_height, void *font, CVector &rgbBlend, float alpha, int flags ){
}



//-----------------------------------------------------------------------------
// Name: Draw_CharFlare
// Desc: 
//-----------------------------------------------------------------------------
void Draw_CharFlare( int x, int y, int w, int h, void *image, CVector &rgbBlend, float alpha, float scale, float theta ){
}
