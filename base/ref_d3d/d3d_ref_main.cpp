#include "d3d_local.h"
#include "d3dutil.h"
#include "d3denum.h"

#include "dk_ref_common.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"

#include "dk_ref_pic.h"

#include "d3d_draw.h"

refdef_t	r_newrefdef;		// global refdef
model_t		*r_worldmodel;		// global pointer to the worldmodel
entity_t	*currententity;		// global pointer to the current entity
model_t		*currentmodel;		// global pointer to the curent model

//
// view origin
//
CVector		vup, base_vup;
CVector		vpn, base_vpn;
CVector		vright, base_vright;
CVector		r_origin;

CVector		r_entorigin;		// the currently rendering entity in world
CVector		modelorg;			// modelorg is the viewpoint reletive to
								// the currently rendering entity

int			r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern const CPic		*pic_characters;				// console characters

D3DMATRIX	view_matrix;
D3DMATRIX	projection_matrix;

void Leave2dMode();

extern int	old_win_xpos, old_win_ypos;



//-----------------------------------------------------------------------------
// Name: R_DrawNullModel
// Desc: 
//-----------------------------------------------------------------------------
void R_DrawNullModel()
{
	D3DMATRIX	matrix;
	D3DVERTEX	lpVerts[6];
	HRESULT		hr;
	int			i;

	SetIdentityMatrix( matrix );
	
	CreateWorldMatrix( currententity );
	
    d3d_state.lpd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matrix );
	
	for (i = 0; i <= 4; i++)
	{
		lpVerts[i].x = 16*cos(i*M_PI/2);
		lpVerts[i].y = 16*sin(i*M_PI/2);
		lpVerts[i].z = 0;

		lpVerts[i].nx = 0;
		lpVerts[i].ny = 0;
		lpVerts[i].nz = -1;

		lpVerts[i].tu = 0;
		lpVerts[i].tv = 0;
	}

	if( FAILED( hr = d3d_state.lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_VERTEX, lpVerts, 5, D3DDP_WAIT) ) )
		CheckDirect3DResult( hr );	

	for (i = 4; i >= 0; i--)
	{
		lpVerts[i].x = 16*cos(i*M_PI/2);
		lpVerts[i].y = 16*sin(i*M_PI/2);
		lpVerts[i].z = 0;

		lpVerts[i].nx = 0;
		lpVerts[i].ny = 0;
		lpVerts[i].nz = -1;

		lpVerts[i].tu = 0;
		lpVerts[i].tv = 0;
	}

	if( FAILED( hr = d3d_state.lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_VERTEX, lpVerts, 5, D3DDP_WAIT) ) )
		CheckDirect3DResult( hr );	
}



//-----------------------------------------------------------------------------
// Name: R_DrawSpotlight
// Desc: 
//-----------------------------------------------------------------------------
void R_DrawSpotlight( entity_t *entity )
{
	R_DrawNullModel();
}



//-----------------------------------------------------------------------------
// Name: R_DrawLaser
// Desc: 
//-----------------------------------------------------------------------------
void R_DrawLaser( entity_t *entity, CPic *bitmap )
{
	R_DrawNullModel();
}



//-----------------------------------------------------------------------------
// Name: R_DrawEntitiesOnList
// Desc: 
//-----------------------------------------------------------------------------
void R_DrawEntitiesOnList()
{
	int			i;
	qboolean	translucent_entities = false;

	for( i = 0; i < r_newrefdef.num_entities; i++ )
	{
		currententity  =&r_newrefdef.entities[i];

		if( currententity->flags & RF_TRANSLUCENT )
		{
			translucent_entities = true;
			continue;
		}

		currentmodel = (model_t *) currententity->model;

		if( !currentmodel )
		{
			R_DrawNullModel();
			continue;
		}

		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		switch( currentmodel->type )
		{
		case mod_sprite:
			R_DrawNullModel();
//			R_DrawSprite();
			break;
		case mod_alias:
			R_DrawNullModel();
//			R_DrawAliasModel();
			break;

		default:
			break;
		}
	}

	if( !translucent_entities )
		return;


	for( i = 0; i < r_newrefdef.num_entities; i++ )
	{
		currententity  =&r_newrefdef.entities[i];

		if( !( currententity->flags & RF_TRANSLUCENT ) )
			continue;

		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		if( currententity->flags & RF_SPOTLIGHT )
		{
			R_DrawSpotlight( currententity );
			continue;
		}

		if( currententity->flags & RF_TRACER )
		{
			R_DrawLaser( currententity, NULL );
			continue;
		}

		if( currententity->flags & RF_BEAM | RF_BEAM_MOVING )
		{
			R_DrawLaser( currententity, NULL );
			continue;
		}

		if( currententity->flags & RF_LIGHTNING )
		{
			R_DrawLaser( currententity, NULL );
			continue;
		}

		if( currententity->flags & RF_NOVALASER )
		{
			R_DrawLaser( currententity, NULL );
			continue;
		}

		currentmodel = (model_t *) currententity->model;

		if( !currentmodel )
		{
			R_DrawNullModel();
			continue;
		}

		switch( currentmodel->type )
		{
		case mod_sprite:
			R_DrawNullModel();
//			R_DrawSprite();
			break;
		case mod_alias:
			R_DrawNullModel();
//			R_DrawAliasModel();
			break;

		default:
			break;
		}
	}
}



//-----------------------------------------------------------------------------
// Name: RegisterSkin
// Desc: 
//-----------------------------------------------------------------------------
void *RegisterSkin( const char *name, resource_t resource )
{
	const CPic	*pic = LoadPic( name, resource );

	return (void *) pic;
}



//-----------------------------------------------------------------------------
// Name: RegisterPic
// Desc: 
//-----------------------------------------------------------------------------
void *RegisterPic( const char *name, resource_t resource )
{
    if (name == NULL || name[0] == '\0') return NULL;

    //load the pic.
    const CPic *pic = LoadPic(name, resource);

    //return it.
    return (void *)pic;
}



//-----------------------------------------------------------------------------
// Name: SetSky
// Desc: 
//-----------------------------------------------------------------------------
void SetSky( const char *name, float rotate, CVector &axis )
{
}



//-----------------------------------------------------------------------------
// Name: SetPaletteDir
// Desc: 
//-----------------------------------------------------------------------------
void SetPaletteDir( const char *name )
{
}



//-----------------------------------------------------------------------------
// Name: SetupD3D
// Desc: 
//-----------------------------------------------------------------------------
extern HWND g_hWnd;

void SetupD3D()
{
	LPDIRECT3DMATERIAL3 render_material;
	HRESULT	hr;

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

	// create the view matrix
	view_matrix = CreateViewMatrix();
	if(FAILED( hr = d3d_state.lpd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &view_matrix ) ) )
		DEBUG_MSG( "d3d_state.lpd3dDevice->SetTransform : D3DTRANSFORMSTATE_VIEW failed!" );

	// create the projection matrix
	projection_matrix = CreateProjectionMatrix( 90, 90, 20, 4096 );
    D3DUtil_SetProjectionMatrix( projection_matrix, 0.75f, fAspect, 1.0f, 1000.0f );
	if(FAILED( hr = d3d_state.lpd3dDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &projection_matrix ) ) )
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
// Name: RenderFrame
// Desc: 
//-----------------------------------------------------------------------------
void RenderFrame( refdef_t *fd )
{
	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		ri.Sys_Error (ERR_FATAL,"R_RenderView: NULL worldmodel");

	Leave2dMode();

	// setup matricies
	SetupD3D();

	// do some drawing
	R_DrawEntitiesOnList();

}



//-----------------------------------------------------------------------------
// Name: DrawGetPicSize
// Desc: 
//-----------------------------------------------------------------------------
void DrawGetPicSize( int *w, int *h, const char *name, resource_t resource )
{
    //set default values.
    *w = 0; 
    *h = 0;

    if (name == NULL || name[0] == '\0') return;

    //get the pic.
    const CPic *pic = LoadPic(name, resource);
    if (pic == NULL) return;

    //get the width and height.
    *w = pic->Width();
    *h = pic->Height();
}



//-----------------------------------------------------------------------------
// Name: DrawChar
// Desc: 
//-----------------------------------------------------------------------------
void DrawChar( int x, int y, int c )
{
	// make sure the pic is there
    if (pic_characters == NULL) 
		return;

	// dont do anything if it's space
	if( (char) c == ' ' )
		return;

    //make sure the given character is 0-255
    c &= 0x000000ff;

    //compute the row and column in the character bitmap that contains the character.
    //there are 16 characters on each row.
    int row = c >> 4;

    //compute the column.  
    int column = c & 0x0f;

    //compute the part of the character bitmap we will draw.
    int src_left = column * 8;
    int src_right = src_left + 8;
    int src_top = row * 8;
    int src_bottom = src_top + 8;

	DrawPicPortion( x, y, src_left, src_top, 8, 8, (void *) pic_characters );
}



//-----------------------------------------------------------------------------
// Name: DrawTileClear
// Desc: 
//-----------------------------------------------------------------------------
void DrawTileClear( int x, int y, int w, int h, const char *name )
{
}



//-----------------------------------------------------------------------------
// Name: DrawFill
// Desc: 
//-----------------------------------------------------------------------------
void DrawFill( int x, int y, int w, int h, int c )
{
}



//-----------------------------------------------------------------------------
// Name: DrawFadeScreen
// Desc: 
//-----------------------------------------------------------------------------
void DrawFadeScreen()
{
}



//-----------------------------------------------------------------------------
// Name: DrawStretchRaw
// Desc: 
//-----------------------------------------------------------------------------
void DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, byte *data )
{
}



//-----------------------------------------------------------------------------
// Name: SetInterfacePalette
// Desc: 
//-----------------------------------------------------------------------------
void SetInterfacePalette( const unsigned char *palette )
{
}



//-----------------------------------------------------------------------------
// Name: CinematicSetPalette
// Desc: 
//-----------------------------------------------------------------------------
void CinematicSetPalette( const unsigned char *palette )
{
}



//-----------------------------------------------------------------------------
// Name: BeginFrame
// Desc: 
//-----------------------------------------------------------------------------
void BeginFrame( float camera_separation )
{
	// check for renerer modifications
	if( d3d_mode->modified || vid_fullscreen->modified )
	{
		cvar_t	*ref = ri.Cvar_Get( "vid_ref", "d3d", 0 );
		ref->modified = true;
	}

	// check for window movement
	vid_xpos = ri.Cvar_Get ("vid_xpos", "0", 0);
	vid_ypos = ri.Cvar_Get ("vid_ypos", "0", 0);

	// check to see if the window was moved
	if( d3d_state.bWindowed )
	{
		if( ( vid_xpos->value != old_win_xpos ) || ( vid_ypos->value != old_win_ypos ) )
		{
			RECT		move_rect;
			move_rect.left		= vid_xpos->value;
			move_rect.top		= vid_ypos->value;
			move_rect.right		= vid_xpos->value + d3d_state.width;
			move_rect.bottom	= vid_ypos->value + d3d_state.height;

			SetRect( &d3d_state.window_rect, move_rect.left, move_rect.top, move_rect.right, move_rect.bottom );

			if( d3d_state.lpddsBack == NULL)
				SetRect( &d3d_state.view_rect, move_rect.left, move_rect.top, move_rect.right, move_rect.bottom );
		}
		
		old_win_xpos = vid_xpos->value;
		old_win_ypos = vid_ypos->value;
	}

	HRESULT	hr;
	D3DRECT	clear_rect;

	clear_rect.x1 = 0;
	clear_rect.y1 = 0;
	clear_rect.x2 = d3d_state.width;
	clear_rect.y2 = d3d_state.height;

	// clear the viewport to black
	if(FAILED( hr = d3d_state.lpd3dViewport->Clear2( 1UL, &clear_rect, D3DCLEAR_TARGET, 0x000000, 0L, 0L ) ) )
		DEBUG_MSG( "d3d_state.lpd3dViewport->Clear2 failed!" );

	d3d_state.lpd3dDevice->BeginScene();

}



//-----------------------------------------------------------------------------
// Name: EndFrame
// Desc: 
//-----------------------------------------------------------------------------
void EndFrame()
{
	HRESULT	hr;

	if( FAILED( hr = d3d_state.lpd3dvbVerts->Unlock() ) )
		CheckDirect3DResult( hr );

	if( FAILED( hr = d3d_state.lpd3dDevice->EndScene() ) )
		CheckDirect3DResult( hr );

	if( d3d_state.lpddsBack == NULL )
	{
		if( FAILED( hr = d3d_state.lpddsFront->IsLost() ) )
			CheckDirectDrawResult( hr );

		return;
	}
	else if( d3d_state.bWindowed )
	{
		if( FAILED( hr = d3d_state.lpddsFront->Blt( &d3d_state.window_rect, d3d_state.lpddsBack, &d3d_state.view_rect, DDBLT_WAIT, NULL ) ) )
			CheckDirectDrawResult( hr );

		return;

	}
	else
	{
		if( FAILED( hr = d3d_state.lpddsFront->Flip( NULL, DDFLIP_WAIT ) ) )
			CheckDirectDrawResult( hr );
		return;
	}
}



//-----------------------------------------------------------------------------
// Name: AppActivate
// Desc: 
//-----------------------------------------------------------------------------
void AppActivate( qboolean activate )
{
}



//-----------------------------------------------------------------------------
// Name: SetResourceDir
// Desc: 
//-----------------------------------------------------------------------------
void SetResourceDir( char *name )
{
    if (name == NULL) 
		return;

    //save the given path into a cvar.
    ri.Cvar_Set("r_resourcedir", name);

	Draw_InitLocal();
}



//-----------------------------------------------------------------------------
// Name: DrawConsolePic
// Desc: 
//-----------------------------------------------------------------------------
void DrawConsolePic( int x, int y, int w, int h )
{
	int		x_start, x_width;
	float	x_scale;

	//  calc where the bigger pic should start
	x_scale = d3d_state.width / 320.0;
	x_width = 256 * x_scale;
	x_start = d3d_state.width - x_width;

	DrawStretchPic(0, y, 64 * x_scale, h, "dkcon2", RESOURCE_GLOBAL);
	DrawStretchPic(x_start, y, x_width, h, "dkcon", RESOURCE_GLOBAL);
}



//-----------------------------------------------------------------------------
// Name: GetFrameName
// Desc: 
//-----------------------------------------------------------------------------
void GetFrameName( char *modelname, int frameindex, char *framename )
{
}



//-----------------------------------------------------------------------------
// Name: VertInfo
// Desc: 
//-----------------------------------------------------------------------------
int VertInfo(void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, CVector &oldorigin, CVector &angles, CVector &vert)
{
	return 0;
}



//-----------------------------------------------------------------------------
// Name: SurfIndex
// Desc: 
//-----------------------------------------------------------------------------
int SurfIndex(void *mod, char *name)
{
	return 0;
}



//-----------------------------------------------------------------------------
// Name: TriVerts
// Desc: 
//-----------------------------------------------------------------------------
int TriVerts( void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3 )
{
	return 0;
}



//-----------------------------------------------------------------------------
// Name: RegisterEpisodeSkins
// Desc: 
//-----------------------------------------------------------------------------
void RegisterEpisodeSkins( void *model, int episode, int flags )
{
}
