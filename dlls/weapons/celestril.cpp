#include "weapons.h"

#include "weapon_funcs.h"
#include "celestril.h"

////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////

void            barrier_use(userInventory_s *ptr, userEntity_t *self);
void            barrier_select (userEntity_t *self);
userInventory_t *barrier_give (userEntity_t *self);
void            barrier_reload (userEntity_t *self);
void            barrier_ammo(userEntity_t *self);

///////////////////////////////////////////////////////////////////
// standard weapon interface routines
//
// each weapon must have these routines so that they can "plug-in"
// to Daikatana with minimum effort -- modularity is the goal
///////////////////////////////////////////////////////////////////

static  char            *weapon_name;
static  char            *weapon_netname;
static  char            *weapon_model;

static  char            *ammo_name;
static  char            *ammo_netname;
static  char            *ammo_model;

// begin and end frames for each sequence

static  int readyFirst      [1];
static  int readyLast       [1];

static  int shootFirst      [1];
static  int shootLast       [1];

static  int reloadFirst     [1];
static  int reloadLast      [1];

static  int changeFirst     [1];
static  int changeLast      [1];

/////////////////////////////////
// barrier_load_init
//
// should be called from dll_Load
/////////////////////////////////

int barrier_load_init (void)
{
    // host_initialized has not been set yet
    // so everything here is perm
    weapon_name = "weapon_barrier";
    weapon_netname = tongue_weapons[T_WEAPON_CELESTRIL];
    weapon_model = "models/e2/w_barrier.dkm";
    ammo_name = "ammo_barrier";
    ammo_netname = tongue_weapons[T_WEAPON_CELESTRIL_AMMO];

    // add a spawn command for this weapon
    gstate->AddCommand ("select_barrier", barrier_select);
    gstate->AddCommand ("weapon_barrier", (void (*)(struct edict_s *))barrier_give);

    return(sizeof(userEntity_t) + sizeof(serverState_t));
}

/////////////////////////////////
// barrier_level_init
//
// should be called from dll_LoadLevel
/////////////////////////////////

void barrier_level_init (void)
{
    userEntity_t    *temp;

    gstate->ModelIndex (weapon_model);

    ////////////////////////////////////////////////////////////////////
    // spawn a temp entity to pull out the frame sequence data
    ////////////////////////////////////////////////////////////////////

    temp = gstate->SpawnEntity ();
    gstate->SetModel (temp, weapon_model);

    com->GetFrames (temp, "goaway", &changeFirst [0], &changeLast [0]);
    com->GetFrames (temp, "reload", &reloadFirst [0], &reloadLast [0]);
    com->GetFrames (temp, "shoot", &shootFirst [0], &shootLast [0]);
    com->GetFrames (temp, "ready", &readyFirst [0], &readyLast [0]);

    temp->remove (temp);
}

/*
=============
weapon_command
=============
*/
//You can use this to tell the weapon to do just about anything
static  void *weapon_command (struct userInventory_s *ptr, char *commandStr, void *data)
{

    ammo_t  *ammo = (ammo_t *) ptr;

    if ( !stricmp (commandStr, "death_msg") )
    {
        killerInfo_t    *ki = (killerInfo_t *) data;
        char            msg [80];

        if ( rnd () < 0.5 )
            sprintf (msg, "%s takes %s's barrier to the head.\n", ki->victim->netname, ki->attacker->netname);
        else
            sprintf (msg, "%s shoves an barrier up %s's yam! (and he likes it! HEHE) .\n", ki->attacker->netname, ki->victim->netname);
    }
    else if ( !stricmp (commandStr, "rating") )
    {
        // return a value between 0 and 1000
        *(int *) data = 150;
    }
    else if ( !stricmp (commandStr, "change") )
    {
        //	play goaway sequence
        weapon_animate ((userEntity_t *)data, changeFirst [0], changeLast [0], 0.05);
    }
    else if ( !stricmp (commandStr, "change_time") )
    {
        //	return length of time to play putaway sequence
        *(float *) data = (changeLast [0] - changeFirst [0]) * 0.05;
    }

    return NULL;

}

/*
=============
ammo_command

make sure you use standard command names when modifying/adding weapons:
  give_ammo
  take_ammo
  death_msg
=============
*/

static  void    *ammo_command (userInventory_t *ptr, char *commandStr, void *data)
{
    return NULL;
}

/*
=============
ammo_create

creates an inventory item of ammo type
=============
*/

static ammo_t *ammo_create (void)
{
    // use malloc since the inventory is here for good
//    ammo_t  *ammo = new ammo_t;
   // init the inventory struct to 0's
  //  memset (ammo, 0x00, sizeof (ammo_t));
    //ammo->name = strdup (ammo_name);
//    ammo->use  = NULL;
    
	ammo_t* ammo = (ammo_t*)gstate->X_Malloc(sizeof(ammo_t),MEM_TAG_INVENTORY);	
	ammo->command = ammo_command;

    return ammo;
}

/*
=============
barrier_create

creates a inventory item of weapon type
=============
*/

static weapon_t *barrier_create (void)
{
    weapon_t    *weapon;

//    // use malloc since the inventory is here for good
//    weapon = new weapon_t;
//
//    // init the inventory struct to 0's
//    memset (weapon, 0x00, sizeof (weapon_t));
    
	weapon = (weapon_t*)gstate->X_Malloc(sizeof(weapon_t),MEM_TAG_INVENTORY);
	weapon->name = strdup (weapon_name);
    weapon->use  = barrier_use;
    weapon->command = weapon_command;

    return weapon;
}

//////////////////////////////////
// barrier_give
//
// puts a barrier in the inventory
//////////////////////////////////

static  userInventory_t *barrier_give (userEntity_t *self)
{
    weapon_t        *weapon;
    userInventory_t *invItem;

    if ( self->deadflag != DEAD_NO || self->curWeapon == NULL || self->client->ps.weapon == NULL )
        return	NULL;

    ////////////////////////////////
    // add weapon to inventory
    ////////////////////////////////

    // make sure weapon is not already in inventory	
    invItem = gstate->InventoryFindItem (self->inventory, weapon_name);

    if ( invItem )
    {
        gstate->Con_Printf ("You already have a %s.\n", weapon_netname);
        return	invItem;
    }

    // create new inventory item
    if ( (weapon = barrier_create ()) == NULL )
    {
        gstate->Con_Printf("Unable to malloc %s.\n", weapon_netname);
        return	NULL;
    }

    // add the weapon to the player's inventory list
    if ( !gstate->InventoryAddItem (self, self->inventory, (userInventory_t *) weapon) )
    {
        gstate->Con_Printf ("Could not add %s to inventory.\n", weapon_netname);
        return	NULL;
    }

    if ( self->flags & FL_CLIENT )
        gstate->sprint (self, "You got the %s!\n", weapon_netname);

    return(userInventory_t *) weapon;
}

//////////////////////////////////////////////////////////////////////////////
// end of weapon interface routines
//////////////////////////////////////////////////////////////////////////////

/*
================

barrier_use

================
*/

static  void    barrier_use (userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t    *phook;

    phook = (playerHook_t *) self->userHook;
    phook->attack_finished = gstate->time + .75;

    weapon_animate (self, shootFirst [0], shootLast [0], 0.05);
}

/*
=============
barrier_select_finish

set the barrier as the players current weapon if it is in the 
inventory
=============
*/

static  void    barrier_select_finish (userEntity_t *self)
{
    playerHook_t        *hook = (playerHook_t *) self->userHook;
    userInventory_t     *invItem;

    invItem = gstate->InventoryFindItem (self->inventory, "weapon_barrier");

    if ( !invItem )
        return;

    self->curWeapon = invItem;
    // this must be here to stop the sling from screwing up the firing of other weapons
    self->flags = self->flags - (self->flags & FL_ATTACKHOLD);

    weapon_animate (self, readyFirst [0], readyLast [0], 0.05); 

    //place a delay in the next time to attack so the weapon
    //can cycle through its "select" sequence
    hook->attack_finished = gstate->time + 1.30;

    gstate->StartEntitySound(self, CHAN_WEAPON, gstate->SoundIndex("weapons/barrier/barrierpickup.wav"), 0.8f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

///////////////////////////////
//	barrier_select
//
//	set the kineticore as the players current weapon if it is in the 
//	inventory
///////////////////////////////

static  void    barrier_select (userEntity_t *self)
{
    playerHook_t        *hook = (playerHook_t *) self->userHook;
    weapon_t            *weapon;
    userInventory_t     *invItem;
    float               seq_time;
    userEntity_t        *went = (userEntity_t *)self->client->ps.weapon;

    if ( self->deadflag != DEAD_NO || self->flags & FL_INWARP )
        return;

    invItem = gstate->InventoryFindItem (self->inventory, weapon_name);

    if ( !invItem )
        return;
    //	tell the user's current weapon to go away
    weapon = (weapon_t *) self->curWeapon;
    if ( weapon )
    {
        weapon->command (self->curWeapon, "change", self);
        weapon->command (self->curWeapon, "change_time", &seq_time);

        //	make sure the client's frame flags tell us to go to next weapon
        went->s.frameInfo.frameFlags |= FRAME_WEAPONSWITCH;

        //	set client's next weapon to this weapon
        hook->weapon_next = barrier_select_finish;

        //	make sure we can't shoot!
        hook->attack_finished = gstate->time + seq_time;
    }
    else
        barrier_select_finish (self);
}

/*
=================
weapon_barrier
=================
*/

static  void    weapon_barrier (userEntity_t *self)
{
    // FIXME: precache weapon's in-world model here

// touch function doesn't exist
#if 0
    self->touch   = (touch_t)weapon_touch;
#endif
    self->netname = "barrier";
    self->className = "weapon_barrier";

    gstate->SetSize(self, -16.0, -16.0, 0.0, 16.0, 16.0, 56.0);

    self->flags    = FL_ITEM;
    self->solid    = SOLID_TRIGGER;
    self->movetype = MOVETYPE_TOSS;

    // set this so weapon_touch knows what select function to call
    // when the weapon is picked up
    self->use = (use_t)barrier_give;
    self->die = (die_t)barrier_select;
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_weapon_barrier_register_func()
{
	gstate->RegisterFunc("weapon_barrier_give",barrier_give);
	gstate->RegisterFunc("weapon_barrier_select",barrier_select);
}