#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

#include "d3d_error.h"
#include "d3dutil.h"

BOOL CheckDirectDrawResult( HRESULT result )
{
	char	ErrorMessage[256];

	switch( result )
	{
	case DDERR_ALREADYINITIALIZED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_ALREADYINITIALIZED );
		break;
	case DDERR_CANNOTATTACHSURFACE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANNOTATTACHSURFACE );
		break;
	case DDERR_CANNOTDETACHSURFACE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANNOTDETACHSURFACE );
		break;
	case DDERR_CURRENTLYNOTAVAIL:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CURRENTLYNOTAVAIL );
		break;
	case DDERR_EXCEPTION:
		sprintf( ErrorMessage, "%s", STRING_DDERR_EXCEPTION );
		break;
	case DDERR_GENERIC:
		sprintf( ErrorMessage, "%s", STRING_DDERR_GENERIC );
		break;
	case DDERR_HEIGHTALIGN:
		sprintf( ErrorMessage, "%s", STRING_DDERR_HEIGHTALIGN );
		break;
	case DDERR_INCOMPATIBLEPRIMARY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INCOMPATIBLEPRIMARY );
		break;
	case DDERR_INVALIDCAPS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDCAPS );
		break;
	case DDERR_INVALIDCLIPLIST:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDCLIPLIST );
		break;
	case DDERR_INVALIDMODE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDMODE );
		break;
	case DDERR_INVALIDOBJECT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDOBJECT );
		break;
	case DDERR_INVALIDPARAMS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDPARAMS );
		break;
	case DDERR_INVALIDPIXELFORMAT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDPIXELFORMAT );
		break;
	case DDERR_INVALIDRECT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDRECT );
		break;
	case DDERR_LOCKEDSURFACES:
		sprintf( ErrorMessage, "%s", STRING_DDERR_LOCKEDSURFACES );
		break;
	case DDERR_NO3D:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NO3D );
		break;
	case DDERR_NOALPHAHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOALPHAHW );
		break;
	case DDERR_NOCLIPLIST:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCLIPLIST );
		break;
	case DDERR_NOCOLORCONVHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCOLORCONVHW );
		break;
	case DDERR_NOCOOPERATIVELEVELSET:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCOOPERATIVELEVELSET );
		break;
	case DDERR_NOCOLORKEY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCOLORKEY );
		break;
	case DDERR_NOCOLORKEYHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCOLORKEYHW );
		break;
	case DDERR_NODIRECTDRAWSUPPORT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCOLORKEYHW );
		break;
	case DDERR_NOEXCLUSIVEMODE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOEXCLUSIVEMODE );
		break;
	case DDERR_NOFLIPHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOFLIPHW );
		break;
	case DDERR_NOGDI:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOGDI );
		break;
	case DDERR_NOMIRRORHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOMIRRORHW );
		break;
	case DDERR_NOTFOUND:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTFOUND );
		break;
	case DDERR_NOOVERLAYHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOOVERLAYHW );
		break;
	case DDERR_OVERLAPPINGRECTS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OVERLAPPINGRECTS );
		break;
	case DDERR_NORASTEROPHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NORASTEROPHW );
		break;
	case DDERR_NOROTATIONHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOROTATIONHW );
		break;
	case DDERR_NOSTRETCHHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOSTRETCHHW );
		break;
	case DDERR_NOT4BITCOLOR:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOT4BITCOLOR );
		break;
	case DDERR_NOT4BITCOLORINDEX:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOT4BITCOLORINDEX );
		break;
	case DDERR_NOT8BITCOLOR:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOT8BITCOLOR );
		break;
	case DDERR_NOTEXTUREHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTEXTUREHW );
		break;
	case DDERR_NOVSYNCHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOVSYNCHW );
		break;
	case DDERR_NOZBUFFERHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOZBUFFERHW );
		break;
	case DDERR_NOZOVERLAYHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOZOVERLAYHW );
		break;
	case DDERR_OUTOFCAPS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OUTOFCAPS );
		break;
	case DDERR_OUTOFMEMORY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OUTOFMEMORY );
		break;
	case DDERR_OUTOFVIDEOMEMORY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OUTOFVIDEOMEMORY );
		break;
	case DDERR_OVERLAYCANTCLIP:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OVERLAYCANTCLIP );
		break;
	case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OVERLAYCOLORKEYONLYONEACTIVE );
		break;
	case DDERR_PALETTEBUSY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_PALETTEBUSY );
		break;
	case DDERR_COLORKEYNOTSET:
		sprintf( ErrorMessage, "%s", STRING_DDERR_COLORKEYNOTSET );
		break;
	case DDERR_SURFACEALREADYATTACHED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACEALREADYATTACHED );
		break;
	case DDERR_SURFACEALREADYDEPENDENT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACEALREADYDEPENDENT );
		break;
	case DDERR_SURFACEBUSY:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACEBUSY );
		break;
	case DDERR_CANTLOCKSURFACE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANTLOCKSURFACE );
		break;
	case DDERR_SURFACEISOBSCURED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACEISOBSCURED );
		break;
	case DDERR_SURFACELOST:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACELOST );
		break;
	case DDERR_SURFACENOTATTACHED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_SURFACENOTATTACHED );
		break;
	case DDERR_TOOBIGHEIGHT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_TOOBIGHEIGHT );
		break;
	case DDERR_TOOBIGSIZE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_TOOBIGSIZE );
		break;
	case DDERR_TOOBIGWIDTH:
		sprintf( ErrorMessage, "%s", STRING_DDERR_TOOBIGWIDTH );
		break;
	case DDERR_UNSUPPORTED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_UNSUPPORTED );
		break;
	case DDERR_UNSUPPORTEDFORMAT:
		sprintf( ErrorMessage, "%s", STRING_DDERR_UNSUPPORTEDFORMAT );
		break;
	case DDERR_UNSUPPORTEDMASK:
		sprintf( ErrorMessage, "%s", STRING_DDERR_UNSUPPORTEDMASK );
		break;
	case DDERR_INVALIDSTREAM:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDSTREAM );
		break;
	case DDERR_VERTICALBLANKINPROGRESS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_VERTICALBLANKINPROGRESS );
		break;
	case DDERR_WASSTILLDRAWING:
		sprintf( ErrorMessage, "%s", STRING_DDERR_WASSTILLDRAWING );
		break;
	case DDERR_XALIGN:
		sprintf( ErrorMessage, "%s", STRING_DDERR_XALIGN );
		break;
	case DDERR_INVALIDDIRECTDRAWGUID:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDDIRECTDRAWGUID );
		break;
	case DDERR_DIRECTDRAWALREADYCREATED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_DIRECTDRAWALREADYCREATED );
		break;
	case DDERR_NODIRECTDRAWHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NODIRECTDRAWHW );
		break;
	case DDERR_PRIMARYSURFACEALREADYEXISTS:
		sprintf( ErrorMessage, "%s", STRING_DDERR_PRIMARYSURFACEALREADYEXISTS );
		break;
	case DDERR_NOEMULATION:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOEMULATION );
		break;
	case DDERR_REGIONTOOSMALL:
		sprintf( ErrorMessage, "%s", STRING_DDERR_REGIONTOOSMALL );
		break;
	case DDERR_CLIPPERISUSINGHWND:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CLIPPERISUSINGHWND );
		break;
	case DDERR_NOCLIPPERATTACHED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOCLIPPERATTACHED );
		break;
	case DDERR_NOHWND:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOHWND );
		break;
	case DDERR_HWNDSUBCLASSED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_HWNDSUBCLASSED );
		break;
	case DDERR_HWNDALREADYSET:
		sprintf( ErrorMessage, "%s", STRING_DDERR_HWNDALREADYSET );
		break;
	case DDERR_NOPALETTEATTACHED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOPALETTEATTACHED );
		break;
	case DDERR_NOPALETTEHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOPALETTEHW );
		break;
	case DDERR_BLTFASTCANTCLIP:
		sprintf( ErrorMessage, "%s", STRING_DDERR_BLTFASTCANTCLIP );
		break;
	case DDERR_NOBLTHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOBLTHW );
		break;
	case DDERR_NODDROPSHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOBLTHW );
		break;
	case DDERR_OVERLAYNOTVISIBLE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_OVERLAYNOTVISIBLE );
		break;
	case DDERR_NOOVERLAYDEST:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOOVERLAYDEST );
		break;
	case DDERR_INVALIDPOSITION:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDPOSITION );
		break;
	case DDERR_NOTAOVERLAYSURFACE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTAOVERLAYSURFACE );
		break;
	case DDERR_EXCLUSIVEMODEALREADYSET:
		sprintf( ErrorMessage, "%s", STRING_DDERR_EXCLUSIVEMODEALREADYSET );
		break;
	case DDERR_NOTFLIPPABLE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTFLIPPABLE );
		break;
	case DDERR_CANTDUPLICATE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANTDUPLICATE );
		break;
	case DDERR_NOTLOCKED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTLOCKED );
		break;
	case DDERR_CANTCREATEDC:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANTCREATEDC );
		break;
	case DDERR_NODC:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NODC );
		break;
	case DDERR_WRONGMODE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_WRONGMODE );
		break;
	case DDERR_IMPLICITLYCREATED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_IMPLICITLYCREATED );
		break;
	case DDERR_NOTPALETTIZED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTPALETTIZED );
		break;
	case DDERR_UNSUPPORTEDMODE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_UNSUPPORTEDMODE );
		break;
	case DDERR_NOMIPMAPHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOMIPMAPHW );
		break;
	case DDERR_INVALIDSURFACETYPE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_INVALIDSURFACETYPE );
		break;
	case DDERR_NOOPTIMIZEHW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOOPTIMIZEHW );
		break;
	case DDERR_NOTLOADED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTLOADED );
		break;
	case DDERR_NOFOCUSWINDOW:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOFOCUSWINDOW );
		break;
	case DDERR_DCALREADYCREATED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_DCALREADYCREATED );
		break;
	case DDERR_NONONLOCALVIDMEM:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NONONLOCALVIDMEM );
		break;
	case DDERR_CANTPAGELOCK:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANTPAGELOCK );
		break;
	case DDERR_CANTPAGEUNLOCK:
		sprintf( ErrorMessage, "%s", STRING_DDERR_CANTPAGEUNLOCK );
		break;
	case DDERR_NOTPAGELOCKED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTPAGELOCKED );
		break;
	case DDERR_MOREDATA:
		sprintf( ErrorMessage, "%s", STRING_DDERR_MOREDATA );
		break;
	case DDERR_EXPIRED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_EXPIRED );
		break;
	case DDERR_VIDEONOTACTIVE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_VIDEONOTACTIVE );
		break;
	case DDERR_DEVICEDOESNTOWNSURFACE:
		sprintf( ErrorMessage, "%s", STRING_DDERR_DEVICEDOESNTOWNSURFACE );
		break;
	case DDERR_NOTINITIALIZED:
		sprintf( ErrorMessage, "%s", STRING_DDERR_NOTINITIALIZED );
		break;
	default:
		sprintf( ErrorMessage, "Unknown Error" );
		break;
	}

//	MessageBox( NULL, ErrorMessage, "DirectDraw Error", MB_OK);
	DEBUG_MSG( ErrorMessage );

	return false;
}

BOOL CheckDirect3DResult( HRESULT result )
{
	char	ErrorMessage[256];

	switch( result )
	{
	case D3DERR_BADMAJORVERSION:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_BADMAJORVERSION );
		break;
	case D3DERR_BADMINORVERSION:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_BADMINORVERSION );
		break;
	case D3DERR_INVALID_DEVICE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALID_DEVICE );
		break;
	case D3DERR_INITFAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INITFAILED );
		break;
	case D3DERR_DEVICEAGGREGATED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_DEVICEAGGREGATED );
		break;
	case D3DERR_EXECUTE_CREATE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_CREATE_FAILED );
		break;
	case D3DERR_EXECUTE_DESTROY_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_DESTROY_FAILED );
		break;
	case D3DERR_EXECUTE_LOCK_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_LOCK_FAILED );
		break;
	case D3DERR_EXECUTE_UNLOCK_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_UNLOCK_FAILED );
		break;
	case D3DERR_EXECUTE_LOCKED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_LOCKED );
		break;
	case D3DERR_EXECUTE_NOT_LOCKED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_NOT_LOCKED );
		break;
	case D3DERR_EXECUTE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_FAILED );
		break;
	case D3DERR_EXECUTE_CLIPPED_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_EXECUTE_CLIPPED_FAILED );
		break;
	case D3DERR_TEXTURE_NO_SUPPORT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_NO_SUPPORT );
		break;
	case D3DERR_TEXTURE_CREATE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_CREATE_FAILED );
		break;
	case D3DERR_TEXTURE_DESTROY_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_DESTROY_FAILED );
		break;
	case D3DERR_TEXTURE_LOCK_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_LOCK_FAILED );
		break;
	case D3DERR_TEXTURE_UNLOCK_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_UNLOCK_FAILED );
		break;
	case D3DERR_TEXTURE_LOAD_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_LOAD_FAILED );
		break;
	case D3DERR_TEXTURE_SWAP_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_SWAP_FAILED );
		break;
	case D3DERR_TEXTURE_LOCKED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_LOCKED );
		break;
	case D3DERR_TEXTURE_NOT_LOCKED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_NOT_LOCKED );
		break;
	case D3DERR_TEXTURE_GETSURF_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_GETSURF_FAILED );
		break;
	case D3DERR_MATRIX_CREATE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATRIX_CREATE_FAILED );
		break;
	case D3DERR_MATRIX_DESTROY_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATRIX_DESTROY_FAILED );
		break;
	case D3DERR_MATRIX_SETDATA_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATRIX_SETDATA_FAILED);
		break;
	case D3DERR_MATRIX_GETDATA_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATRIX_GETDATA_FAILED );
		break;
	case D3DERR_SETVIEWPORTDATA_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SETVIEWPORTDATA_FAILED  );
		break;
	case D3DERR_INVALIDCURRENTVIEWPORT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDCURRENTVIEWPORT );
		break;
	case D3DERR_INVALIDPRIMITIVETYPE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDPRIMITIVETYPE );
		break;
	case D3DERR_INVALIDVERTEXTYPE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDVERTEXTYPE );
		break;
	case D3DERR_TEXTURE_BADSIZE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TEXTURE_BADSIZE );
		break;
	case D3DERR_INVALIDRAMPTEXTURE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDRAMPTEXTURE );
		break;
	case D3DERR_MATERIAL_CREATE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATERIAL_CREATE_FAILED );
		break;
	case D3DERR_MATERIAL_DESTROY_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATERIAL_DESTROY_FAILED );
		break;
	case D3DERR_MATERIAL_SETDATA_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATERIAL_SETDATA_FAILED );
		break;
	case D3DERR_MATERIAL_GETDATA_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_MATERIAL_GETDATA_FAILED );
		break;
	case D3DERR_INVALIDPALETTE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDPALETTE );
		break;
	case D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY );
		break;
	case D3DERR_ZBUFF_NEEDS_VIDEOMEMORY:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_ZBUFF_NEEDS_VIDEOMEMORY );
		break;
	case D3DERR_SURFACENOTINVIDMEM:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SURFACENOTINVIDMEM );
		break;
	case D3DERR_LIGHT_SET_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_LIGHT_SET_FAILED );
		break;
	case D3DERR_LIGHTHASVIEWPORT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_LIGHTHASVIEWPORT );
		break;
	case D3DERR_LIGHTNOTINTHISVIEWPORT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_LIGHTNOTINTHISVIEWPORT );
		break;
	case D3DERR_SCENE_IN_SCENE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SCENE_IN_SCENE );
		break;
	case D3DERR_SCENE_NOT_IN_SCENE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SCENE_NOT_IN_SCENE );
		break;
	case D3DERR_SCENE_BEGIN_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SCENE_NOT_IN_SCENE );
		break;
	case D3DERR_SCENE_END_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_SCENE_END_FAILED );
		break;
	case D3DERR_INBEGIN:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INBEGIN );
		break;
	case D3DERR_NOTINBEGIN:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_NOTINBEGIN );
		break;
	case D3DERR_NOVIEWPORTS:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_NOVIEWPORTS );
		break;
	case D3DERR_VIEWPORTDATANOTSET:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_VIEWPORTDATANOTSET );
		break;
	case D3DERR_VIEWPORTHASNODEVICE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_VIEWPORTHASNODEVICE );
		break;
	case D3DERR_NOCURRENTVIEWPORT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_NOCURRENTVIEWPORT );
		break;
	case D3DERR_INVALIDVERTEXFORMAT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDVERTEXFORMAT );
		break;
	case D3DERR_COLORKEYATTACHED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_COLORKEYATTACHED );
		break;
	case D3DERR_VERTEXBUFFEROPTIMIZED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_VERTEXBUFFEROPTIMIZED );
		break;
	case D3DERR_VBUF_CREATE_FAILED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_VBUF_CREATE_FAILED );
		break;
	case D3DERR_VERTEXBUFFERLOCKED:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_VERTEXBUFFERLOCKED );
		break;
	case D3DERR_ZBUFFER_NOTPRESENT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_ZBUFFER_NOTPRESENT );
		break;
	case D3DERR_STENCILBUFFER_NOTPRESENT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_STENCILBUFFER_NOTPRESENT );
		break;
	case D3DERR_WRONGTEXTUREFORMAT:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_WRONGTEXTUREFORMAT );
		break;
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDCOLOROPERATION );
		break;
	case D3DERR_UNSUPPORTEDCOLORARG:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDCOLORARG );
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDALPHAOPERATION );
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDALPHAARG );
		break;
	case D3DERR_TOOMANYOPERATIONS:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TOOMANYOPERATIONS );
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_CONFLICTINGTEXTUREFILTER );
		break;
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDFACTORVALUE );
		break;
	case D3DERR_CONFLICTINGRENDERSTATE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_CONFLICTINGRENDERSTATE );
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_UNSUPPORTEDTEXTUREFILTER );
		break;
	case D3DERR_TOOMANYPRIMITIVES:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TOOMANYPRIMITIVES );
		break;
	case D3DERR_INVALIDMATRIX:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_INVALIDMATRIX );
		break;
	case D3DERR_TOOMANYVERTICES:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_TOOMANYVERTICES );
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		sprintf( ErrorMessage, "%s", STRING_D3DERR_CONFLICTINGTEXTUREPALETTE );
		break;
	default:
		sprintf( ErrorMessage, "Unknown Error" );
		break;
	}

//	MessageBox( NULL, ErrorMessage, "Direct3D Error", MB_OK);
	DEBUG_MSG( ErrorMessage );

	return false;
}
