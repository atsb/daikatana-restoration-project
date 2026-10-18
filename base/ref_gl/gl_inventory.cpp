///////////////////////////////////////////////////////////////////////////////
//	gl_inventory.c
//
//	Routines for displaying inventory list
//
///////////////////////////////////////////////////////////////////////////////

#include	"gl_local.h"
//#include	"gl_protex.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	ICON_WIDTH	80
//#define	ICON_HALF_WIDTH	(ICON_WIDTH / 2)

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern	float	ref_laserRotation;
extern	void R_DrawAliasIcon(entity_t* ent);
//extern void *scoreboard_font;

#define ICON_ITEM_XOFFSET     54
#define ICON_ITEM_YOFFSET    250
#define ICON_ITEM_HEIGHT      58

#define ICON_WEAPON_XOFFSET   38
#define ICON_WEAPON_YOFFSET  436
#define ICON_WEAPON_HEIGHT		61

static float InventoryHUDScale(void)
{
	cvar_t* hud_scale_cvar;
	float user_scale;

	hud_scale_cvar = ri.Cvar_Get("hud_scale", "1.00", CVAR_ARCHIVE);
	user_scale = hud_scale_cvar ? hud_scale_cvar->value : 1.0f;
	if (user_scale < 1.00f) user_scale = 1.00f;
	if (user_scale > 3.00f) user_scale = 3.00f;

	/* vid is already the physical display size.  HUD scaling here is
	   strictly the user-selected 100..300 percent factor. */
	return user_scale;
}


///////////////////////////////////////////////////////////////////////////////
//	inventory_DrawIcons
//
///////////////////////////////////////////////////////////////////////////////

void	inventory_DrawIcons(void)
// this routine will draw the model icons for weapon and item inventories
{
	int			i, sx, sy;
	entity_t	ent;
	invenIcon_t* icon;
	CVector		origin;
	int			viewportHeight, viewportSY, numIcons, viewportSX, viewportWidth;
	int			icon_height, icon_xoffset;
	float		scale_x, scale_y, scale_z;
	float			hud_scale;

	if (r_newrefdef.rdflags & (RDF_LETTERBOX | RDF_NO_ICONS))
	{
		return;
	}

	if (r_newrefdef.inventory_mode != INVMODE_WEAPON_ICONS && // no weapon icons?
		r_newrefdef.inventory_mode != INVMODE_ITEM_ICONS)			// no icons display mode?
		return;

	// SCG[11/23/99]: Make sure depth writing is enabled
	GL_SetState(GLSTATE_PRESET1);

	//	clear the zbuffer
	qglClear(GL_DEPTH_BUFFER_BIT);

	numIcons = r_newrefdef.numInventoryIcons;

	hud_scale = InventoryHUDScale();

	// set coordinates for specific inventory modes
	if (r_newrefdef.inventory_mode == INVMODE_ITEM_ICONS)
	{
		icon_xoffset = (int)(ICON_ITEM_XOFFSET * hud_scale + 0.5f);
		icon_height = (int)(ICON_ITEM_HEIGHT * hud_scale + 0.5f);
		sy = vid.height - (int)(ICON_ITEM_YOFFSET * hud_scale + 0.5f);
		scale_x = scale_y = scale_z = 1.4f * hud_scale;
	}
	else
	{
		icon_xoffset = (int)(ICON_WEAPON_XOFFSET * hud_scale + 0.5f);
		icon_height = (int)(ICON_WEAPON_HEIGHT * hud_scale + 0.5f);
		sy = vid.height - (int)(ICON_WEAPON_YOFFSET * hud_scale + 0.5f);
		scale_x = scale_y = scale_z = 2.5f * hud_scale;
	}

	viewportWidth = (int)(ICON_WIDTH * hud_scale + 0.5f);
	if (viewportWidth < 1)
		viewportWidth = 1;
	if (viewportWidth > vid.width)
		viewportWidth = vid.width;
	viewportSX = vid.width - viewportWidth;
	viewportSY = 0;
	viewportHeight = vid.height;

	qglViewport(viewportSX,      // x
		viewportSY,      // y
		viewportWidth,   // width
		viewportHeight); // height

	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();

	qglOrtho(0,                         // left
		viewportWidth,             // right
		viewportHeight,            // bottom
		0,                         // top
		-9999, 9999);              // near/far 

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadIdentity();

	sx = icon_xoffset;

	memset(&ent, 0x00, sizeof(entity_t));

	origin.x = sx;
	origin.y = sy;
	origin.z = 0;

	//	draw the icon models
	icon = r_newrefdef.inventoryIcons;

	qglDepthMask(1);
	//	GL_SetState( GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT);
	for (i = 0; i < numIcons; i++, icon++)
	{
		if (icon->model != NULL && r_newrefdef.inventory_alpha != 0)
		{
			//	set up an entity so we can draw the model
			memset(&ent, 0x00, sizeof(entity_t));

			ent.model = icon->model;

			ent.origin = origin;
			ent.angles.x += ref_laserRotation * 2;
			ent.angles.z = -45;

			ent.frame = 2;
			ent.oldorigin = origin;
			ent.oldframe = 2;
			ent.backlerp = 0;
			ent.skinnum = 0;

			//ent.flags = RF_TRANSLUCENT | RF_MINLIGHT;   // models reflect light if RF_MINLIGHT set
			ent.flags = RF_FULLBRIGHT | RF_TRANSLUCENT;
			ent.alpha = r_newrefdef.inventory_alpha;

			ent.render_scale.Set(scale_x, scale_y, scale_y);

			currententity = &ent;
			//			GL_SetState( 0 );
			R_DrawAliasIcon(&ent);
		}

		origin.y += icon_height;
	}

	origin.x = sx;
	origin.y = sy;
	origin.z = 0;

	//qglColor4f (1, 1, 1, r_newrefdef.inventoryAlpha);

	/*
	//	draw the ammo counts
	icon = r_newrefdef.inventoryIcons;

	for (i = 0; i < numIcons; i++, icon++)
	{
	if (icon->model != NULL && icon->count >= 0)
	  inventory_DrawNum (origin.x + ICON_NUM_X, origin.y + ICON_NUM_Y, icon->count);

		origin.y += icon_height;
	}
	*/

	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();

	qglViewport(0, 0, vid.width, vid.height);
	qglShadeModel(GL_FLAT);
	GL_SetState(0);
}

