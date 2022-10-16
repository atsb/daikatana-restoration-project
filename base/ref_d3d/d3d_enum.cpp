#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf, VOID* pddpfDesired )
{
    if( pddpf == NULL || pddpfDesired == NULL )
        return D3DENUMRET_CANCEL;

    if( pddpf->dwFlags == ((DDPIXELFORMAT*)pddpfDesired)->dwFlags )
    {
        memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}
