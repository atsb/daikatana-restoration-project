#ifndef	_WEAPONDEFS_H
#define	_WEAPONDEFS_H

//////////////////////////////
// macros
//////////////////////////////

// Logic[4/16/99]: This should not be defined UNLESS we have some special build where it should be
//  FORCED on!
#define WEAPON_UNLIMITED_AMMO    0

// weapon_trace_attack flags
#define WTA_HITMIDDLE      0x0001
#define WTA_SPARKS         0x0002

// poly explosion flags
#define PEF_ANGLES      0x0001                  // 'vec' param is angles
#define PEF_NORMAL      0x0002                  // 'vec' param is normal
#define PEF_SOUND       0x0004                  // play sound

// handedness values
#define RIGHT_HANDED       0
#define LEFT_HANDED        1
#define CENTER_HANDED      2

#define UNICAST_ENT        127
#define WEAPON_ENTITY      ((userEntity_t *)self->client->ps.weapon)

#define winfoModel(num)                         winfo->models[(short)(num)]
#define winfoSound(num)                         winfo->sounds[(short)(num)]
#define winfoAnim(num)                          winfo->frames[(short)(num)].name
//#define winfoAnim(ent,num,delay)                weapon_animate(ent,winfo->frames[(short)(num)].first,winfo->frames[(short)(num)].last,delay)
#define winfoFirstFrame(num)                    (winfo->frames[(short)(num)].first)
#define winfoLastFrame(num)                     (winfo->frames[(short)(num)].last)
#define winfoNumFrames(num)                     (winfoLastFrame(num) - winfoFirstFrame(num) + 1)
#define winfoPlay(ent,num,vol)                  gstate->StartEntitySound(ent,CHAN_AUTO,gstate->SoundIndex(winfo->sounds[(short)(num)]),vol,ATTN_NORM_MIN,ATTN_NORM_MAX)
#define winfoPlayAttn(ent,num,vol, att1, att2)  gstate->StartEntitySound(ent,CHAN_AUTO,gstate->SoundIndex(winfo->sounds[(short)(num)]),vol,att1,att2)
#define winfoSetModel(ent,num)                  {ent->s.modelindex=gstate->ModelIndex(winfoModel(num));}
#define dk2SetModel(ent,str)                    {ent->s.modelindex=gstate->ModelIndex(str);}
#define winfoPlayLooped(ent,num,vol)            {ent->s.sound = gstate->SoundIndex(winfo->sounds[(short)(num)]); ent->s.volume = vol;}//gstate->sound(ent,CHAN_WEAPON,winfo->sounds[(short)(num)],vol,ATTN_NORM)
#define winfoAttackWeap							gstate->attackWeap = winfo->weaponName
#define WP_NUM_EPISODES    4
#define WP_WEAPONS_PER_EP  8

// weaponSelect() return values
#define WEAPON_CHANGED           0           // weapon changes
#define WEAPON_ACTIVE            1           // weapon is already active -- nothing happens
#define WEAPON_UNAVAILABLE       2           // weapon not in inventory -- nothing happens
#define WEAPON_ANIMATING         3           // doing animation -- nothing happens

//////////////////////////////
// typedefs
//////////////////////////////

/*
// crosshair auto-aim info
typedef struct autoAim_s {
   float xofs,yofs;
   struct edict_s *ent;
} autoAim_t;
*/

typedef struct freezeEntity_s {
   think_t old_prethink,old_think;
   pain_t old_pain;
   die_t old_die;
   CVector old_velocity,old_avelocity;
   int old_movetype, old_clipmask;
   edict_t *old_owner;
} freezeEntity_t;

typedef enum weaponStd_t {
   // standard model positions (in winfo list)
   MDL_WEAPON_STD=0,
   MDL_ARTIFACT_STD,
   MDL_AMMO_STD,

   MDL_SPECIAL,                  // start additional models here

   // standard anim positions (in winfo list)
   ANIM_READY_STD=0,
   ANIM_AWAY_STD,
   ANIM_SHOOT_STD,
   ANIM_AMBIENT_STD,
   ANIM_AMBIENT2_STD,
   ANIM_AMBIENT3_STD,
   ANIM_SPECIAL,                 // start additional anims here

   // standard sound positions (in winfo list)
   SND_WEAPON_STD=0,                   // weapon humm
   SND_READY_STD,                      // ready
   SND_AWAY_STD,                       // away
   SND_SHOOT_STD,                      // shoot
   SND_AMBIENT_STD,                    // ambient 1
   SND_AMBIENT2_STD,                   // ambient 2
   SND_AMBIENT3_STD,                   // ambient 3

   SND_SPECIAL,                  // start additional sounds here
} weaponStd_t;

#define MDL_START    MDL_STARTM=MDL_SPECIAL-1
#define SND_START    SND_STARTM=SND_SPECIAL-1
#define ANIM_START   ANIM_STARTM=ANIM_SPECIAL-1

#define SND_NOSOUND     ((char *)-1)
#define MDL_NOMODEL     ((char *)-1)
#define ANIM_NOANIM     ((char *)-1)
#define ANIM_NONE       -1,ANIM_NOANIM,0,0

/*
typedef struct trackInfo_s {
   userEntity_t *ent;                     // entity to be tracked by srcent
   userEntity_t *srcent,*dstent;          // track from src, to dst entity
   userEntity_t *extra;                   // extra entity (special case)
   CVector fru;                            // fwd, rt, up offsets from src (compressed into CVector)
   CVector dstpos;                         // dst position (used if dstent==NULL)
   long flags;                            // uses TEF_xxxx flags
   long renderfx;
   float length;
   CVector lightColor;
   float lightSize;
   int modelindex;
   short numframes;
   float scale;
   float frametime;
   CVector altpos,altpos2;
   CVector altangle;
} trackInfo_t;
*/
typedef struct	weaponListNode_s
{
	weaponListNode_s	*prev;
	weaponListNode_s	*next;

	userEntity_t		*ent;	// the entity that this node refers to
} weaponListNode_t;

typedef	struct	weaponListHeader_s
{
	weaponListNode_t	*head;
	weaponListNode_t	*tail;

	int					count;
} weaponListHeader_t;

typedef	struct	killerInfo_s
{
	userEntity_t	*attacker;
	userEntity_t	*victim;
} killerInfo_t;

//-----------------------------------------------------------------------------
//	touchHook_t
//
//	hook structure for weapon pickup items.  Stores a pointer to the weapon's
//	give function so that it can be called when the player picks it up without
//	stuffing a "weapon_xxx" command.
//-----------------------------------------------------------------------------

typedef	struct	touchHook_s
{
	give_t		give_func;
	select_t	select_func;
} touchHook_t;

typedef	struct
{
	weaponInfo_t	*winfo;		//	pointer to winfo for this weapon
	int				count;		//	how much ammo this ammo pack holds
	int				sound;		//  pickup sound (defaults to i_c4ammo if not set.
} ammoTouchHook_t;

#endif