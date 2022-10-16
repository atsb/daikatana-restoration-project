///////////////////////////////////////////////////////////////////////////////
//	gl_icons.cpp
//
//	Routines for displaying onscreen icons
//
///////////////////////////////////////////////////////////////////////////////

#include	"gl_local.h"
//#include	"gl_protex.h"
#include  "p_user.h"


///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define ICON_SPACING    (36)
#define ICON_TOP        (230)
#define	ICON_WIDTH	    (160)
#define	ICON_HALF_WIDTH	(ICON_WIDTH / 2)

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern	float	ref_laserRotation;
extern	void R_DrawAliasIcon (entity_t *ent);


///////////////////////////////////////////////////////////////////////////////
//	onscreen_icons
///////////////////////////////////////////////////////////////////////////////
void GL_Boost_Icons_Display(void)
{
	int				 i, sx, sy, i2;
	entity_t		 ent;
	onscreen_icons_t icon;
	CVector			 origin;
	int				 numIcons;

	//return;  // TEMPORARY

	if ( r_newrefdef.rdflags & (RDF_LETTERBOX|RDF_NO_ICONS) )
	{
		return;
	}

	numIcons = r_newrefdef.onscreen_icons_info.num_icons;

	if (numIcons <= 0) // any icons visible?
		return;


	qglClear(GL_DEPTH_BUFFER_BIT);  //	clear the zbuffer

	// left edge of screen, full height of screen
	//             x  y       width      height
	qglViewport (  0, 0, ICON_WIDTH, vid.height);

	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix ();
	qglLoadIdentity ();

	// match this with viewport
	//       left       right      bottom  top
	qglOrtho(   0, ICON_WIDTH, vid.height,   0, -9999, 9999);

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix ();
	qglLoadIdentity ();

	sx = ICON_HALF_WIDTH;         // set x center at half viewport width
	sy = vid.height - ICON_TOP;  	// start at TOP and work spacing DOWN

	origin.x = sx;
	origin.y = sy-16.0f;
	origin.z = 0;

	//	write to Z buffer
	GL_SetState( GLSTATE_PRESET1 & ~GLSTATE_CULL_FACE_FRONT );
	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LESS, -1 );

	qglShadeModel (GL_SMOOTH);

	//	draw the icon models
	for (i = 0; i < MAX_ONSCREEN_ICONS; i++)
	{
		icon = r_newrefdef.onscreen_icons_info.icons[i]; 

		// loop through both icons
		for (i2 = 0;i2 < 2;i2++)
		{
			if (icon.models[i2] != NULL)
			{
				if (icon.alpha == 0)
					continue;

				// set up an entity so we can draw the model
				memset (&ent, 0x00, sizeof (entity_t));

				ent.model = icon.models[i2];
				ent.origin = origin;

				// rotation
				if (i2 & 1)
				ent.angles.x += ref_laserRotation * 8; // rotate counter
				else
				ent.angles.x -= ref_laserRotation * 8; // rotate clockwise

				ent.angles.z = -90; // sit up nice and tall, fuckstick

				ent.frame     = 1;
				ent.oldorigin = origin;
				ent.oldframe  = 0;
				ent.backlerp  = 0;
				ent.skinnum   = 0;
				ent.alpha     = icon.alpha;                         // set alpha
//				ent.flags     = ( RF_TRANSLUCENT | RF_MINLIGHT | RF_FULLBRIGHT );
				ent.flags     = RF_FULLBRIGHT;

//				if (i2 & 1)
//				ent.flags |= RF_GLOW;  // glow the internal model

				ent.render_scale.Set(1.0,1.0,1.0);  // scale depends on viewport size

				R_DrawAliasIcon (&ent);
			}
		}

		// work from TOP to bottom
		origin.y += ICON_SPACING;  // make room for the next icon
	}

	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix ();
	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix ();

	qglViewport (0, 0, vid.width, vid.height);  // reset viewport fullscreen

	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LEQUAL, -1 );

	qglCullFace (GL_FRONT);
}

