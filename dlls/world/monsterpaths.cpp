/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	routines for handling monster paths that are hardcoded into levels
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "world.h"
#include "nodelist.h"
//#include "misc.h"// SCG[1/23/00]: not used
#include "ai_func.h"

////////////////////////////////////////////////////////////////////////////////
// exports
////////////////////////////////////////////////////////////////////////////////

DllExport	void	monster_path_corner (userEntity_t *self);

////////////////////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////////////////////
// SCG[11/20/99]: Save game stuff
#define MPATHHOOK(x) (int)&(((mpathHook_t *)0)->x)
field_t mpath_hook_fields[] = 
{
	{"target",			MPATHHOOK(target),			F_LSTRING},
	{"action",			MPATHHOOK(action),			F_LSTRING},
	{"szScriptName",	MPATHHOOK(szScriptName),	F_LSTRING},
	{NULL, 0, F_INT}
};

void mpath_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, mpath_hook_fields, sizeof( mpathHook_t ) );
}

void mpath_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, mpath_hook_fields, sizeof( mpathHook_t ) );
}

void	monster_path_corner (userEntity_t *self)
{
	mpathHook_t	*hook;
	int			i;

	self->userHook = gstate->X_Malloc(sizeof(mpathHook_t), MEM_TAG_HOOK);
	hook = (mpathHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = mpath_hook_save;
	self->load = mpath_hook_load;

	for ( i = 0; self->epair[i].key != NULL; i++ )
	{
		// targets
		if ( !stricmp(self->epair[i].key, "target1") || !stricmp(self->epair[i].key, "target") )
		{
			self->target = self->epair[i].value;
			hook->target[0] = self->epair[i].value;
		}
		else 
		if (!stricmp(self->epair[i].key, "target2"))
		{
			hook->target[1] = self->epair[i].value;
		}
		else 
		if (!stricmp(self->epair[i].key, "target3"))
		{
			hook->target[2] = self->epair[i].value;
		}
		else 
		if (!stricmp(self->epair[i].key, "target4"))
		{
			hook->target[3] = self->epair[i].value;
		}
		// ISP: 3-30
		else
		if ( _stricmp( self->epair[i].key, "aiscript" ) == 0 )
		{
			hook->szScriptName = strdup( self->epair[i].value );
		}
		else 
		if (!stricmp(self->epair[i].key, "targetname"))
		{
			self->targetname = self->epair[i].value;	
		}
		else 
		if (!stricmp(self->epair[i].key, "pathtarget"))
		{
			hook->pathtarget = self->epair[i].value;	
		}
	}		

	self->className = "monster_path_corner";

	// show us the path corner for now
//	gstate->SetModel( self, debug_models[DEBUG_CURNODE] );
	self->movetype  = MOVETYPE_NONE;
	self->solid     = SOLID_NOT;
	gstate->SetSize(self, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2(self, self->s.origin);

}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// SCG[1/23/00]: Why not just a define for these?
const static int nMaxNumVisiblePathCorners       = 30;
const static float fMaxDistanceVisiblePathCorner = 1024.0f;

static int      bShowPathCorners = FALSE;
static userEntity_t	*pPathCornerEnts[nMaxNumVisiblePathCorners];
static int		nNumVisiblePathCorners = 0;
static float	fLastPathCornerUpdate = 0.0f;

static void pathcorner_UpdateVisible( userEntity_t *self, int bCheckTime = TRUE );

#ifdef _DEBUG

// ----------------------------------------------------------------------------
//
// Name:		pathcorner_AddVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void pathcorner_AddVisible( userEntity_t *self, userEntity_t *pPathCorner )
{
	float fDistance = VectorDistance( self->s.origin, pPathCorner->s.origin );
	if ( fDistance <= fMaxDistanceVisiblePathCorner )
	{
		if ( nNumVisiblePathCorners < nMaxNumVisiblePathCorners )
		{
			pPathCornerEnts[nNumVisiblePathCorners] = pPathCorner;
            
            pPathCornerEnts[nNumVisiblePathCorners]->modelName    = debug_models[DEBUG_CURNODE];
            pPathCornerEnts[nNumVisiblePathCorners]->s.modelindex = gstate->ModelIndex(debug_models[DEBUG_CURNODE]);

			gstate->LinkEntity( pPathCornerEnts[nNumVisiblePathCorners] );

            nNumVisiblePathCorners++;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		node_update_visible_nodes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void pathcorner_UpdateVisible( userEntity_t *self, int bCheckTime /* = TRUE */ )
{
	if ( bShowPathCorners &&
         ( (gstate->time - fLastPathCornerUpdate) > 0.5f || bCheckTime == FALSE ) )
	{
		// remove all the current ones
		for ( int i = 0; i < nMaxNumVisiblePathCorners; i++ )
		{
		    if ( pPathCornerEnts[i] )
            {
                pPathCornerEnts[i]->s.modelindex = 0;
                pPathCornerEnts[i]->modelName    = NULL;
            }
		}

		nNumVisiblePathCorners = 0;
        userEntity_t *head = gstate->FirstEntity();
	    while ( head )
	    {
		    if ( _stricmp( head->className, "monster_path_corner" ) == 0 )
            {
                pathcorner_AddVisible( self, head );
            }

		    head = gstate->NextEntity( head );
	    }
	}
}

// ----------------------------------------------------------------------------
//
// Name:        pathcorner_Toggle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pathcorner_Toggle( userEntity_t *self )
{
    if ( bShowPathCorners == TRUE )
    {
	    bShowPathCorners = FALSE;

    }
    else
    {
        bShowPathCorners = TRUE;

		pathcorner_UpdateVisible( self, FALSE );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        pathcorner_Draw
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pathcorner_Draw( userEntity_t *self )
{
    pathcorner_UpdateVisible( self );

    if ( bShowPathCorners )
    {
		for ( int i = 0; i < nNumVisiblePathCorners; i++ )
		{
		    _ASSERTE( pPathCornerEnts[i] );

		    userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCornerEnts[i] );
		    if ( pNextPathCorner )
		    {
                com->DrawLine( self, pPathCornerEnts[i]->s.origin, pNextPathCorner->s.origin, 3 );
            }
		}
    }
}
#endif
// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
