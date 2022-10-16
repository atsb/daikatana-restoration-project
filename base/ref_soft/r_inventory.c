///////////////////////////////////////////////////////////////////////////////
//	gl_inventory.c
//
//	Routines for displaying inventory list
//
//	Nelno, 10/13/98
///////////////////////////////////////////////////////////////////////////////

#include	"r_local.h"
#include	"r_protex.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	ICON_HEIGHT	64
#define	ICON_BOX_HEIGHT	(ICON_HEIGHT - 2)
#define	ICON_HALF_HEIGHT	(ICON_HEIGHT / 2)

#define	ICON_WIDTH	64
#define	ICON_BOX_WIDTH	(ICON_WIDTH - 2)
#define	ICON_HALF_WIDTH	(ICON_WIDTH / 2)

#define	ICON_CHAR_WIDTH		6.0
#define	ICON_CHAR_HEIGHT	6.0
#define	ICON_CHARS_PER_ROW	5
#define	ICON_NUM_CHARS		11	//	0 - 9 and "-"

#define	ICON_NUM_X			ICON_HALF_WIDTH / 2
#define	ICON_NUM_Y			ICON_HALF_HEIGHT / 2

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern	float	ref_laserRotation;
//extern	image_t	*r_inventoryTextures [4];
extern	void R_DrawAliasIcon (entity_t *ent);

//extern	image_t	*r_inventoryNumbers;

///////////////////////////////////////////////////////////////////////////////
//	inventory_DrawChar
//
///////////////////////////////////////////////////////////////////////////////

void inventory_DrawChar (int x, int y, char c)
{
	int newx;

	newx = x + vid.width - ICON_WIDTH;

	DrawChar( newx, y, c );
}

///////////////////////////////////////////////////////////////////////////////
//	inventory_DrawNum
//
///////////////////////////////////////////////////////////////////////////////

void inventory_DrawNum (int x, int y, int num)
{
	char	str [16];
	int		i, len;

	itoa (num, str, 10);

	len = strlen (str);
	for (i = 0; i < len; i++)
	{
		inventory_DrawChar( x, y, str [i] );

		x += ICON_CHAR_WIDTH;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	inventory_DrawBox
//
///////////////////////////////////////////////////////////////////////////////

void inventory_DrawBox (int x, int y, int w, int h, float alpha)
{
	DrawPic( x, y, "/pics/statusbar/invenbox.bmp", RESOURCE_GLOBAL );
}

///////////////////////////////////////////////////////////////////////////////
//	inventory_DrawIcons
//
///////////////////////////////////////////////////////////////////////////////

void	inventory_DrawIcons (void)
{
	int				i, j, sx, sy, selected;
//	int				clipTop, clipBottom;
	entity_t		ent;
	invenIcon_t		*icon;
	CVector			origin;
	dmdl_t			*extraData;
	model_t			*model;
	daliasframe_t	*frame;
	float			largestScale, scale;
	int				viewportHeight, viewportSY, numIcons, viewportCenter;
	vrect_t			rect;

	if (r_newrefdef.numInventoryIcons <= 0)
		return;

	numIcons = r_newrefdef.numInventoryIcons;
	if (numIcons > 1)
		numIcons = 3;
	else
		numIcons = 1;

	//	viewport size is dependent on number of unique icons in list
	viewportCenter = vid.height / 2;
	viewportHeight = ICON_HEIGHT * 3;
	viewportSY = (vid.height - viewportHeight) / 2;
 
	// set view origin
	r_newrefdef.vieworg[0] = r_origin[0] = 0;
	r_newrefdef.vieworg[1] = r_origin[1] = 0;
	r_newrefdef.vieworg[2] = r_origin[2] = 0;

	// set the fov
	r_newrefdef.fov_x = 50;
	r_newrefdef.fov_y = 50;

	vpn[1] = vpn[2] = vright[0] = vright[2] = vup[0] = vup[1] = 0;

	vpn[0]		= 1;
	vright[1]	= -1;
	vup[2]		= 1;

	sy = ICON_HALF_HEIGHT;
	if (numIcons == 1)
		sy -= ICON_HEIGHT * 2;
	else
		sy -= ICON_HEIGHT;

	sy -= r_newrefdef.inventoryScroll * ICON_HEIGHT;

	if (r_newrefdef.inventoryScroll < 0)
		sy -= ICON_HEIGHT;
	else
		sy += ICON_HEIGHT;

	sx = ICON_HALF_WIDTH;

	// clear the entity
	memset (&ent, 0x00, sizeof (entity_t));

	sy = viewportSY + ICON_HALF_HEIGHT;

	origin [0] = sx + vid.width - ICON_WIDTH;
	origin [1] = sy + ICON_HEIGHT;
	origin [2] = 0;

	selected = 2;	//	selected item is always in the middle of the 5

	sy = viewportSY - ICON_HEIGHT;

	// clear the zbuffer
	memset( d_pzbuffer, 0xff, vid.width * vid.height * sizeof( d_pzbuffer[0] ) );

	//	draw the background boxes
	if (r_newrefdef.inventoryScroll == 1.0 || r_newrefdef.inventoryScroll == -1.0)
	{
		//	only draw when scrolling is complete
		for (i = 0, icon = r_newrefdef.inventoryIcons; i <= selected; i++, icon++)
		{
			if (icon->model != NULL)
			{
				if (i == selected)
					inventory_DrawBox (origin [0] - ICON_HALF_WIDTH, origin [1] - ICON_HALF_HEIGHT, ICON_BOX_WIDTH, ICON_BOX_HEIGHT, 1.0 * r_newrefdef.inventoryAlpha);
			}
		}
	}

	// software uses a seperate viewport for each icon
	origin [0] = 80;  // x is depth :P
	origin [1] = 0;
	origin [2] = 0;

	//	draw the icon models
	for (i = 0, icon = r_newrefdef.inventoryIcons; i < 5; i++, icon++)
	{
		if( (i == 0 && numIcons > 1) || i == 4 )
			continue;

		if (icon->model != NULL)
		{
			//	set up an entity so we can draw the model
			memset (&ent, 0x00, sizeof (entity_t));
			ent.model = icon->model;
			ent.origin = origin;

			ent.angles = vec3_origin;
			ent.angles [1] += ref_laserRotation * 2;
			ent.angles [2] = -45;

			ent.oldorigin = origin;
			ent.frame		= 1;
			ent.oldframe	= 0;
			ent.backlerp	= 0;
			ent.skinnum		= 0;
			ent.lightstyle	= 0;

			ent.alpha = 1.0 *  r_newrefdef.inventoryAlpha;
			ent.flags = RF_TRANSLUCENT | RF_MINLIGHT | RF_FULLBRIGHT;

			model = icon->model;
			extraData = (dmdl_t *)model->extradata;
			frame = (daliasframe_t *)((byte *)extraData + extraData->ofs_frames 
					+ ent.frame * extraData->framesize);

			largestScale = 0.0;
			for (j = 0; j < 3; j++)
				if (frame->scale [j] > largestScale)
					largestScale = frame->scale [j];

			if (icon->count >= 0)
				scale = 0.18 / largestScale;
			else
				scale = 0.25 / largestScale;

			ent.render_scale [0] = scale;
			ent.render_scale [1] = scale;
			ent.render_scale [2] = scale;

			rect.x		= vid.width - ICON_WIDTH;
			rect.y		= sy + (ICON_HEIGHT * i);
			rect.width	= ICON_WIDTH;
			rect.height	= ICON_HEIGHT;

			R_ViewChanged( &rect );

			R_DrawAliasIcon (&ent);
		}
	}

	origin [0] = sx;
	origin [1] = sy + (ICON_HEIGHT * 2) - ICON_HALF_HEIGHT;
	origin [2] = 0;

	//	draw the ammo counts
	for (i = 0, icon = r_newrefdef.inventoryIcons; i < 5; i++, icon++)
	{
		if( (i == 0 && numIcons > 1) || i == 4 )
			continue;

		if (icon->model != NULL && icon->count >= 0)
		{
			inventory_DrawNum (origin [0] + ICON_NUM_X, origin [1] + ICON_NUM_Y, icon->count);
		}

		origin [1] += ICON_HEIGHT;
	}
}