#include	"world.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	BUTTON_HEIGHT	1
#define	MAX_BUTTONS		14

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	userEntity_t	*button [MAX_BUTTONS];
static	int				menu_on = false;

///////////////////////////////////////////////////////////////////////////////
//	menu_quit
///////////////////////////////////////////////////////////////////////////////

void	menu_quit (userEntity_t *self)
{
	int	i;

	if (!menu_on)
		return;

	for (i = 0; i < MAX_BUTTONS; i++)
	{
		gstate->RemoveEntity (button [i]);
	}

	menu_on = false;
}

///////////////////////////////////////////////////////////////////////////////
//	menu_start
//
//	screwed if client has a +/- pitch
///////////////////////////////////////////////////////////////////////////////

void	menu_start (userEntity_t *self)
{
	int				i;
	float			x, y, z;
	Vector			ang, org;

	if (menu_on)
		return;

	ang = Vector (0, self->client->v_angle [1], 0);
	ang.AngleVectors (forward, right, up);

	//	make client face forward
	gstate->SetClientAngles (self, ang.vec ());
	self->flags |= FL_FIXANGLES;

	for (i = 0; i < MAX_BUTTONS; i++)
	{
		org = Vector (self->s.origin) + forward * 12.5;
		org.z = (org.z + 30) - (i * BUTTON_HEIGHT);

		button [i] = gstate->SpawnEntity ();
		button [i]->s.modelindex = gstate->ModelIndex ("models/testbutton/testbutton.dkm");
		button [i]->movetype = MOVETYPE_NONE;
		button [i]->solid = SOLID_NOT;
		button [i]->s.renderfx = RF_MINLIGHT;

		VectorCopy (zero_vector, button [i]->s.angles);
		button [i]->s.angles [1] = self->client->v_angle [1] - 165;
		VectorCopy (zero_vector, button [i]->s.mins);
		VectorCopy (zero_vector, button [i]->s.maxs);
		org.set (button [i]->s.origin);

		gstate->LinkEntity (button [i]);
	}

	menu_on = true;
}