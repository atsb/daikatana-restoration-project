#include "weapons.h"
#include "tazerhook.h"

/////////////////////////////////////////////////////////////////////
// prototypes
/////////////////////////////////////////////////////////////////////

void            tazerhook_use(userInventory_s *ptr, userEntity_t *self);
void            tazerhook_select (userEntity_t *self);
userInventory_t *tazerhook_give (userEntity_t *self);
void            tazerhook_reload (userEntity_t *self);
void            tazerhook_ammo(userEntity_t *self);

/////////////////////////////////////////////////////////////////////
/// weapon_tazerhook
/////////////////////////////////////////////////////////////////////

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

static  int readyFirst      [1];
static  int readyLast       [1];

static  int shootFirst      [1];
static  int shootLast       [1];

static  int reloadFirst     [1];
static  int reloadLast      [1];

static  int changeFirst     [1];
static  int changeLast      [1];

/////////////////////////////////
// tazerhook_load_init
//
// should be called from dll_Load
/////////////////////////////////

int tazerhook_load_init (void)
{
    // host_initialized has not been set yet
    // so everything here is perm
    weapon_name = "weapon_tazerhook";
    weapon_netname = tongue_weapons[T_WEAPON_TAZERHOOK],
                     weapon_model = "models/e4/w_tazer.dkm";
    ammo_name = NULL;
    ammo_netname = NULL;
    ammo_model = "models/e4/we_tazerammo.dkm";

    // add spawn command
    gstate->AddCommand ("select_tazerhook", tazerhook_select);
    gstate->AddCommand (weapon_name, (void (*)(struct edict_s *))tazerhook_give);

    return(sizeof(userEntity_t) + sizeof(serverState_t));
}

/////////////////////////////////
// tazerhook_level_init
//
// should be called from dll_LoadLevel
/////////////////////////////////

void tazerhook_level_init (void)
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
tazerhook_command
=============
*/
//You can use this to tell the weapon to do just about anything
static  void *tazerhook_command (struct userInventory_s *ptr, char *commandStr, void *data)
{

    ammo_t  *ammo = (ammo_t *) ptr;

    if ( !stricmp (commandStr, "death_msg") )
    {
        killerInfo_t    *ki = (killerInfo_t *) data;
        char            msg [80];

        if ( rnd () < 0.5 )
            sprintf (msg, "%s takes %s's tazerhook to the head.\n", ki->victim->netname, ki->attacker->netname);
        else
            sprintf (msg, "%s shoves an tazerhook up %s's yam! (and he likes it! HEHE) .\n", ki->attacker->netname, ki->victim->netname);
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
  count
=============
*/

static  void *ammo_command (userInventory_t *ptr, char *commandStr, void *data)
{
    ammo_t  *ammo = (ammo_t *) ptr;

    if ( !stricmp (commandStr, "give_ammo") )
    {
        ammo->count = ammo->count + *(int *) data;
        return(void *) 1;
    }
    else if ( !stricmp (commandStr, "take_ammo") )
    {
        if ( ammo->count > 0 )
        {
            ammo->count = ammo->count - *(int *) data;
            return(void *) 1;
        }
        else
            return NULL;
    }
    else if ( !stricmp (commandStr, "count") )
    {
        // returns ammount of ammo in data
        *(int *) data = ammo->count;
        return(void *) 1;
    }

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
    ammo_t  *ammo;

//    // use malloc since the inventory is here for good
//    ammo = new ammo_t;
//
//    // init the inventory struct to 0's
//    memset (ammo, 0x00, sizeof (ammo_t));
//    //ammo->name = strdup (ammo_name);
//    ammo->use  = NULL;

	ammo = (ammo_t*)gstate->X_Malloc(sizeof(ammo_t),MEM_TAG_INVENTORY);
    ammo->command = ammo_command;

    return ammo;
}

/*
=============
weapon_create

creates a inventory item of weapon type
=============
*/

static weapon_t *weapon_create (void)
{
    weapon_t    *weapon;

//    // use malloc since the inventory is here for good
//    weapon = new weapon_t;
//
//    // init the inventory struct to 0's
//    memset (weapon, 0x00, sizeof (weapon_t));

	weapon = (weapon_t*)gstate->X_Malloc(sizeof(weapon_t),MEM_TAG_HOOK);
    weapon->name = strdup (weapon_name);
    weapon->use  = tazerhook_use;
    weapon->command = tazerhook_command;

    return weapon;
}

/*
=============
tazerhook_give

puts a tazerhook in the inventory
=============
*/

static  userInventory_t *tazerhook_give (userEntity_t *self)
{
    weapon_t        *weapon;
    userInventory_t *invItem;

    /////////////////////////////
    // add weapon to inventory
    /////////////////////////////

    invItem = gstate->InventoryFindItem (self->inventory, weapon_name);

    if ( invItem )
    {
        gstate->Con_Printf ("You already have a %s.\n", weapon_netname);
        return	invItem;
    }

    // create new inventory item
    if ( (weapon = weapon_create ()) == NULL )
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

    gstate->sprint (self, "You got the %s!\n", weapon_netname);

    return(userInventory_t *) weapon;
}

//////////////////////////////////////////////////////////////////////////////
// end of weapon interface routines
//////////////////////////////////////////////////////////////////////////////

/*
================
tazerhook_use
================
*/

static  void    tazerhook_use (userInventory_s *ptr, userEntity_t *self)
{
//	weapHook_t      *hook;
    playerHook_t    *phook;
    CVector         dir, org, end;

    phook = (playerHook_t *) self->userHook;
    phook->attack_finished = gstate->time + .75;

    dir = self->s.angles;
    dir.AngleToVectors (forward, right, up);

    org = self->s.origin + forward * 16 + right * 16;
    org.z += 16;
    end = org + forward * 500.0;

    gstate->TraceLine (org, end, FALSE, self, &trace);

    weapon_animate (self, shootFirst [0], shootLast [0], 0.05);
}

/*
=============
tazerhook_select_finish

set the tazerhook as the players current weapon if it is in the 
inventory
=============
*/

static  void    tazerhook_select_finish (userEntity_t *self)
{
    playerHook_t        *hook = (playerHook_t *) self->userHook;
    userInventory_t     *invItem;

    invItem = gstate->InventoryFindItem (self->inventory, "weapon_tazerhook");

    if ( !invItem )
        return;

    self->curWeapon = invItem;
    // this must be here to stop the sling from screwing up the firing of other weapons
    self->flags = self->flags - (self->flags & FL_ATTACKHOLD);

    weapon_animate (self, readyFirst [0], readyLast [0], 0.05); 

    //place a delay in the next time to attack so the weapon
    //can cycle through its "select" sequence
    hook->attack_finished = gstate->time + 1.30;

    gstate->StartEntitySound(self, CHAN_WEAPON, gstate->SoundIndex("weapons/tazerhook/tazerhookpickup.wav"), 0.8f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

///////////////////////////////
//	tazerhook_select
//
//	set the tazerhook as the players current weapon if it is in the 
//	inventory
///////////////////////////////

static  void    tazerhook_select (userEntity_t *self)
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
        hook->weapon_next = tazerhook_select_finish;

        //	make sure we can't shoot!
        hook->attack_finished = gstate->time + seq_time;
    }
    else
        tazerhook_select_finish (self);
}

#if 0
/*
=================
weapon_tazerhook
=================
*/

void    weapon_tazerhook (userEntity_t *self)
{
    //	FIXME:	precache weapon's in-world models here

    self->touch   = (touch_t)weapon_touch;
    self->netname = "tazerhook";
    self->className = "weapon_tazerhook";
    self->client->ps.gunindex = gstate->ModelIndex(weapon_model);

    gstate->SetSize(self, -16.0, -16.0, 0.0, 16.0, 16.0, 56.0);

    self->flags    = FL_ITEM;
    self->solid    = SOLID_TRIGGER;
    self->movetype = MOVETYPE_TOSS;

    // set this so weapon_touch knows what select function to call
    // when the weapon is picked up
    self->use = (use_t)tazerhook_give;
    self->die = (die_t)tazerhook_select;
}
#endif