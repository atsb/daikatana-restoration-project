#include "ref.h"
#include "dk_ref_common.h"

int				r_numalphaentities;
sortentity_t	r_alphaentities[MAX_ENTITIES / 2];

extern "C" int V_CompareDistance( const void *ent1, const void *ent2 );
int V_CompareDistance( const void *ent1, const void *ent2 )
{
	float dist1 = ( ( sortentity_t * ) ent1 )->distance;
	float dist2 = ( ( sortentity_t * ) ent2 )->distance;

	// SCG[3/29/99]: Objects furthest away from the camera are drawn first (back to front)
	if( dist1 > dist2 )
	{
		return -1;
	}
	if( dist1 < dist2 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
		return;
=====================
SortAlphaEntities
=====================
*/
void SortAlphaEntities()
{
	qsort( ( void  * ) r_alphaentities, r_numalphaentities, sizeof( sortentity_t ), V_CompareDistance ) ;
}
