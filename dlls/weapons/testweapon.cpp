#include "weapons.h"
#include "testweapon.h"

///////////////////////////////////////////////////////////////////////
// exports
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// prototypes
///////////////////////////////////////////////////////////////////////

void			tw_use(userInventory_s *ptr, userEntity_t *self);
void			tw_select (userEntity_t *self);
userInventory_t	*tw_give (userEntity_t *self);
void			tw_listseq (userEntity_t *self);
void			tw_runseq (userEntity_t *self);

///////////////////////////////////////////////////////////////////
// standard weapon interface routines
//
// each weapon must have these routines so that they can "plug-in"
// to Daikatana with minimum effort -- modularity is the goal
///////////////////////////////////////////////////////////////////

static	char			*weapon_name;
static	char			*weapon_netname;
static  char            *weapon_model;

static	int	readyFirst		[1];
static	int	readyLast		[1];

static	int	shootFirst		[1];
static	int	shootLast		[1];

static	int	reloadFirst		[1];
static	int	reloadLast		[1];

static	int	changeFirst		[1];
static	int	changeLast		[1];
			
/////////////////////////////////
// tw_load_init
//
// should be called from dll_Load
/////////////////////////////////

int tw_load_init (void)
{
	// host_initialized has not been set yet
	// so everything here is perm
	weapon_name = "weapon_testweapon";
	weapon_netname = "testweapon";
	weapon_model = "models/global/w_testweapon.dkm";

	
	// add spawn command
	gstate->AddCommand ("select_testweapon", tw_select);
	gstate->AddCommand ("weapon_testweapon", (void (*)(struct edict_s *))tw_give);

	gstate->AddCommand ("tw_give", (void (*)(struct edict_s *))tw_give);
	gstate->AddCommand ("tw_showseq", tw_listseq);
	gstate->AddCommand ("tw_runseq", tw_runseq);
	
	return (sizeof(userEntity_t) + sizeof(serverState_t));
}

/////////////////////////////////
// tw_level_init
//
// should be called from dll_LoadLevel
/////////////////////////////////

void tw_level_init (void)
{
	userEntity_t	*temp;

	gstate->ModelIndex (weapon_model);

	////////////////////////////////////////////////////////////////////
	// spawn a temp entity to pull out the frame sequence data
	////////////////////////////////////////////////////////////////////

	temp = gstate->SpawnEntity ();
	temp->s.modelindex = gstate->ModelIndex (weapon_model);

/*	
	com->GetFrames (temp, "goaway", &changeFirst [0], &changeLast [0]);
	com->GetFrames (temp, "reload", &reloadFirst [0], &reloadLast [0]);
	com->GetFrames (temp, "shoota", &shootFirst [0], &shootLast [0]);
	com->GetFrames (temp, "ready", &readyFirst [0], &readyLast [0]);
*/	

	com->GetFrames (temp, "away", &changeFirst [0], &changeLast [0]);
	com->GetFrames (temp, "reload", &reloadFirst [0], &reloadLast [0]);
	com->GetFrames (temp, "shoota", &shootFirst [0], &shootLast [0]);
	com->GetFrames (temp, "ready", &readyFirst [0], &readyLast [0]);

	temp->remove (temp);
}

/*
=============
weapon_command
=============
*/

void *tw_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
	
	ammo_t	*ammo = (ammo_t *) ptr;
	
	if (!stricmp (commandStr, "death_msg"))
	{
		killerInfo_t	*ki = (killerInfo_t *) data;
		char			msg [80];

		if (rnd () < 0.5)
			sprintf (msg, "%s takes %s's testweapon to the head.\n", ki->victim->netname, ki->attacker->netname);
		else
			sprintf (msg, "%s shoves an testweapon up %s's yam! (and he likes it! HEHE) .\n", ki->attacker->netname, ki->victim->netname);
	}
	else if (!stricmp (commandStr, "rating"))
	{
		// return a value between 0 and 1000
		*(int *) data = 150;
	}
	else if (!stricmp (commandStr, "change"))
	{

      entAnimate((userEntity_t *)data,changeFirst[0],changeLast[0],FRAME_ONCE|FRAME_WEAPON,.05);
		//	play goaway sequence
//weapon_animate ((userEntity_t *)data, changeFirst [0], changeLast [0], 0.05);
	}
	else if (!stricmp (commandStr, "change_time"))
	{
		//	return length of time to play putaway sequence
		*(float *) data = (changeLast [0] - changeFirst [0]) * 0.05;
	}
	else if (!stricmp(commandStr, "winfo"))
	{
		*(void **)data=NULL;
	}

	return NULL;
}

/*
=============
tw_create

creates a inventory item of weapon type
=============
*/

weapon_t *tw_create (void)
{
	weapon_t	*weapon;

//	// use malloc since the inventory is here for good
//	weapon = new weapon_t;
//
//	// init the inventory struct to 0's
//	memset (weapon, 0x00, sizeof (weapon_t));

	weapon = (weapon_t*)gstate->X_Malloc(sizeof(weapon_t),MEM_TAG_HOOK);
	weapon->name = strdup (weapon_name);
	weapon->use  = tw_use;
	weapon->command = tw_command;
	
	return weapon;
}

///////////////////////////////////
// tw_give
//
// puts the testweapon in self's inventory
///////////////////////////////////

userInventory_t	*tw_give (userEntity_t *self)
{
	weapon_t		*weapon;
	userInventory_t	*invItem;

	/////////////////////////////
	// add weapon to inventory
	/////////////////////////////

	// make sure weapon is not already in inventory	
	invItem = gstate->InventoryFindItem (self->inventory, weapon_name);

	if (invItem)
	{
		gstate->Con_Printf ("You already have a %s.\n", weapon_netname);
		return	invItem;
	}

	// create new inventory item
	if ((weapon = tw_create ()) == NULL)
	{
		gstate->Con_Printf("Unable to malloc %s.\n", weapon_netname);
		return	NULL;
	}

	// add the weapon to the player's inventory list
	if (!gstate->InventoryAddItem (self, self->inventory, (userInventory_t *) weapon))
	{
		gstate->Con_Printf ("Could not add %s to inventory.\n", weapon_netname);
		return	NULL;
	}

	if (self->flags & FL_CLIENT)
		gstate->sprint (self, "You picked up the %s!\n", weapon_netname);

	tw_select (self);

	return	(userInventory_t *) weapon;
}

//////////////////////////////////////////////////////////////////////////////
// end of weapon interface routines
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////
//	tw_use
//
///////////////////////////////

void	tw_use (userInventory_s *ptr, userEntity_t *self)
{
	playerHook_t	*phook;
	weapon_t        *testweaponp;
	int				hit;
	
	phook = (playerHook_t *) self->userHook;
	phook->attack_finished = gstate->time + .75;

  entAnimate(self,shootFirst[0],shootLast[0],FRAME_ONCE|FRAME_WEAPON,.05);
//	weapon_animate (self, shootFirst [0], shootLast [0], 0.05);

	testweaponp = (weapon_t *) self->curWeapon;
	
	hit = weapon_trace_attack (self, 25, 1, 62,0);
}

/////////////////////////////
//	tw_select_finish
//
/////////////////////////////

void	tw_select_finish (userEntity_t *self)
{
	playerHook_t		*hook = (playerHook_t *) self->userHook;
	userInventory_t		*invItem;

	invItem = gstate->InventoryFindItem (self->inventory, "weapon_testweapon");

	if (!invItem)
		return;

   if( !selectWeapon(self, weapon_name, weapon_model, 0 ) )
   {
      return;
   }

	self->curWeapon = invItem;
	// this must be here to stop the sling from screwing up the firing of other weapons
	self->flags = self->flags - (self->flags & FL_ATTACKHOLD);

   entAnimate(self,readyFirst[0],readyLast[0],FRAME_ONCE|FRAME_WEAPON,.05);
//	weapon_animate (self, readyFirst [0], readyLast [0], 0.05);	
	
	//place a delay in the next time to attack so the weapon
	//can cycle through its "select" sequence
	hook->attack_finished = gstate->time + 1.30;

   self->client->ps.gunindex = gstate->ModelIndex(weapon_model);
}

///////////////////////////////
//	tw_select
//
//	set the testweapon as the players current weapon if it is in the 
//	inventory
///////////////////////////////

void	tw_select (userEntity_t *self)
{
	playerHook_t		*hook = (playerHook_t *) self->userHook;
	weapon_t			*weapon;
	userInventory_t		*invItem;
	float				seq_time;
	userEntity_t		*went = (userEntity_t *)self->client->ps.weapon;

	if (self->deadflag != DEAD_NO || self->flags & FL_INWARP)
		return;

	invItem = gstate->InventoryFindItem (self->inventory, weapon_name);

	if (!invItem)
		return;
	//	tell the user's current weapon to go away
	weapon = (weapon_t *) self->curWeapon;
	if (weapon)
	{
		weapon->command (self->curWeapon, "change", self);
		weapon->command (self->curWeapon, "change_time", &seq_time);

		//	make sure the client's frame flags tell us to go to next weapon
		went->s.frameInfo.frameFlags |= FRAME_WEAPONSWITCH;
	
		//	set client's next weapon to this weapon
		hook->weapon_next = tw_select_finish;

		//	make sure we can't shoot!
		hook->attack_finished = gstate->time + seq_time;
	}
	else
		tw_select_finish (self);
}

////////////////////////////////////////////////////////////////
/// List Sequences
////////////////////////////////////////////////////////////////

typedef struct
{
	char frameName[60];
	int  frameCount;
} listseq_t;


static listseq_t  listseq[100];  // 100 unique frames
static int        listseqCount;

////////////////////////////////////////////////////////////////
/// list_addframename
////////////////////////////////////////////////////////////////

void list_addframename (char *frameName)
{
	int i;

	for (i = 0; i < listseqCount; i++)
	{
		if (!strcmp(listseq[i].frameName, frameName))
		{
			listseq[i].frameCount++;
			return;
		}
	}

	if (i >= 100) return;

	strcpy(listseq[listseqCount].frameName, frameName);
	listseq[listseqCount].frameCount = 1;

	listseqCount++;
}

////////////////////////////////////////////////////////////////
/// list_listsequences
////////////////////////////////////////////////////////////////

void list_printseq ()
{
	int i;

	for (i = 0; i < listseqCount; i++)
	{
		gstate->bprint ("%4d %s\n", 
			listseq[i].frameCount, listseq[i].frameName);
	}
}

////////////////////////////////////////////////////////////////
//	tw_listseq
//
//	list the sequences contained in testweapon.dkm
////////////////////////////////////////////////////////////////

void tw_listseq (userEntity_t *self)
{
	// int ret;
	int i, j, len;
	userFrameInfo_t frameInfo;
	int frameNum;
	char baseName [100];
	userEntity_t	*testweapon;

	// spawn a temp entity to get the frames names from
	testweapon = gstate->SpawnEntity ();
	testweapon->s.modelindex = gstate->ModelIndex ("models/global/w_testweapon.dkm");

	listseqCount = 0;

	// go through all frames and get start and stop info
	for (i = 0;; i++)
	{
	    if (!gstate->FrameInfo(testweapon->s.modelindex, i, &frameInfo)) break;

		len = strlen(frameInfo.frameName);

		// seperate base and frame number
		for (j = 0; j < len; j++)
		{
			if ((frameInfo.frameName[j] >= '0') && (frameInfo.frameName[j] <= '9'))
			{
				break;
			}
		}

		// make sure frame has a name
		if (j >= len) continue;

		strcpy (baseName, frameInfo.frameName);

		baseName[j] = 0x00;
		frameNum    = atoi(&frameInfo.frameName[j]);

		// add frame to list
		list_addframename (baseName);
	}

	// print list seq
	list_printseq ();

	testweapon->remove (testweapon);
}

//////////////////////////////////////////////////////////////////////////
// tw_runseq
//////////////////////////////////////////////////////////////////////////

void	tw_runseq (userEntity_t *self)
{
	int					argc;
	playerHook_t		*hook = (playerHook_t *) self->userHook;
	float				interval;
	userEntity_t		*testweapon;

	argc = gstate->GetArgc();

	if ((argc <= 1) || (argc > 3))
	{
		gstate->bprint ("usage: tw_runseq <frame seq> [interval]\n");
		return;
	}

	if (argc == 3)
	{
		interval = atof(gstate->GetArgv(2));
	}

	// spawn a temp entity to get the frames names from
	testweapon = gstate->SpawnEntity ();
	testweapon->s.modelindex = gstate->ModelIndex ("models/global/w_testweapon.dkm");

	// find beginngin and end sequences
	com->GetFrames (testweapon, gstate->GetArgv (1), &changeFirst [0], &changeLast [0]);

	testweapon->remove (testweapon);

   entAnimate(self,changeFirst[0],changeLast[0],FRAME_ONCE|FRAME_WEAPON,.05);
//	weapon_animate (self, changeFirst [0], changeLast [0], 0.05);
}