#ifndef	_COM_H_
#define	_COM_H_

#if _MSC_VER
//	for C++ classes that will be used in other DLLs
#ifndef	DllExport_Plus
#define DllExport_Plus __declspec(dllexport)
#endif
//	for functions the Quake engine calls (not in C++)
#ifndef	DllExport
#define DllExport	extern "C"	__declspec( dllexport )
#endif
#else	// for unix
#undef DllExport_Plus
#undef DllExport
#define DLLExport_Plus
#define DllExport extern "C"
#endif


///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

//	com_heap.cpp
//#define	MEM_HUNK		0	// removed each time a level ends
//#define	MEM_MALLOC		1	// freeable, removed each time a level ends
//#define	MEM_PERMANENT	2	// freeable, lasts between levels, removed when DLLs exit

//	com_vector.cpp
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

//	com_sub.cpp
#define	DAMAGE_NONE				0x00000000
#define	DAMAGE_EXPLOSION		0x00000001
#define	DAMAGE_INERTIAL			0x00000002
#define	DAMAGE_SQUISH			0x00000004
#define	DAMAGE_DROWN			0x00000008
#define	DAMAGE_LAVA				0x00000010
#define	DAMAGE_SLIME 			0x00000020
#define	DAMAGE_FALLING			0x00000040
#define	DAMAGE_POISON			0x00000080
#define	DAMAGE_TELEFRAG			0x00000100
#define	DAMAGE_TRIGGER			0x00000200
#define	DAMAGE_SIDEWINDER		0x00000400
#define	DAMAGE_SPARKLES			0x00000800
#define DAMAGE_PAINOVERRIDE     0x00001000
#define DAMAGE_FREEZE           0x00002000
#define DAMAGE_CHECK_INVENTORY	0x00004000		//Checks the inventory for special items and determine the amount of damage to apply
#define DAMAGE_COLD				0x00008000		// overrides the default pain flash with a blue flash

#define  DAMAGE_RADIUS2       0x20000000        // damage, spread over 2x the area
#define  DAMAGE_RADIUS4       0x40000000        // damage, spread over 4x the area
#define  DAMAGE_RADIUS8       0x80000000        // damage, spread over 8x the area

#define	DAMAGE_ONLY_TYPES		0x00FFFFFF	//	mask for all damage types

#define	DAMAGE_NO_VELOCITY		0x01000000	//	Velocity will not be applied
#define	DAMAGE_NO_PROTECTION	0x02000000	//	Nothing will prevent damage, not even llama mode
#define	DAMAGE_DIR_TO_ATTACKER	0x04000000	//	override dir parameter, compute dir from attacker to target
#define	DAMAGE_DIR_TO_INFLICTOR	0x08000000	//	override dir parameter, use velocity of inflictor
#define	DAMAGE_INFLICTOR_VEL	0x10000000	//	override dir parameter, use velocity of inflictor
#define	DAMAGE_NO_BLOOD			0x20000000	//	don't spawn blood particles
#define DAMAGE_TOWARDS_ATTACKER	0x40000000	//  override dir parm.. use velocity projected towards attacker
#define DAMAGE_IGNORE_ARMOR		0x80000000	// SCG[1/4/00]: 

#define	DAMAGE_ONLY_FLAGS		0xFF000000	//	mask for all damage flags

#define	PITCH	0
#define	YAW		1
#define	ROLL	2

//	com_friction.cpp
#define	FR_SPIRAL		0x00000001
#define	FR_SEMIFLOAT	0x00000002
#define	FR_FLOAT		0x00000004

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//	com_sub.cpp

//	com_list.cpp
typedef struct	listNode_s
{
	listNode_s	*prev;
	listNode_s	*next;
	
	char		type;
	
	void		*ptr;	// pointer to this node's data
} listNode_t;

typedef	struct	listHeader_s
{
	listNode_t	*head;
	listNode_t	*tail;
	
	int					count;
} listHeader_t;

//	com_friction.cpp
typedef struct
{
	int  	last_content;	// the content type during the last frame
	float	change_time;	// how often the object is affected by friction
	
	float	size_scale;		// how large the object is, Quake grenade = 1.0
	float	mass_scale;		// how heavy the object is, Quake grenade = 1.0
	
	//CVector	last_pos;		// origin of the object during the last frame
	CVector	liquid_pos;		// location of the object when it first encountered liquid
	
	float	max_vel;		// upper limit on object's velocity
	int		flags;			// various attribute flags
	
	// fields for special movement types
	float	spiral_dir;
	
	// sounds to play when we enter or leave a content
	char	*enter_sound;
	char	*exit_sound;
} frictionHook_t;

//	com_weapons.cpp
// cek[1-14-99] well, it seems someone decided to mix flags together.  The WFL_* flags are used when a weapon
// is added to the inventory.  the cool thing is that the ITF_* flags overlap these flags.  cool, eh?

#define WFL_LOOPAMBIENT_ANIM	0x0001			// loop AMBIENT anim?
//#define WFL_LOOPAMBIENT_SND		0x0002		  // loop AMBIENT sound?
#define WFL_DK_POWER			0x0004			// Daikatana powers set this flag (stops ready/away animations)
#define WFL_RIPGUN				0x0008			// ripgun weapon set this flag       "    "      "      "
#define WFL_FIRE				0x0010			// track fire to weapon
#define WFL_SPECIAL				0x0020			//	special code required for AI to use this weapon
#define WFL_AI_CANNOT_USE		0x0040			//	ai cannot use this weapon
#define WFL_AI_TRACK_TARG		0x0080			//	weapon should be kept poiting at target until lifetime expires (novabeam)
#define WFL_FORCE_SWITCH		0x0100			// force switch if no ammo?
#define WFL_LOOPBOBWALK_ANIM1	0x0200			// is playe moving and bob anim playing?
#define WFL_NODROP				0x0400			// don't drop this weapon type (taser glove, other "default" inventory items)
#define WFL_CROSSHAIR			0x0800
#define WFL_SELECT_EMPTY		0x1000			// allows this weapon to be selected with no ammo
#define WFL_PLAYER_ONLY			0x2000			// sidekicks cannot pick this up!

/*
#define WFL_LOOPAMBIENT_ANIM	0x00010000			// loop AMBIENT anim?
#define WFL_DK_POWER			0x00040000			// Daikatana powers set this flag (stops ready/away animations)
#define WFL_RIPGUN				0x00080000			// ripgun weapon set this flag       "    "      "      "
#define WFL_FIRE				0x00100000			// track fire to weapon
#define WFL_SPECIAL				0x00200000			//	special code required for AI to use this weapon
#define WFL_AI_CANNOT_USE		0x00400000			//	ai cannot use this weapon
#define WFL_AI_TRACK_TARG		0x00800000			//	weapon should be kept poiting at target until lifetime expires (novabeam)
#define WFL_FORCE_SWITCH		0x01000000			// force switch if no ammo?
#define WFL_LOOPBOBWALK_ANIM1	0x02000000			// is playe moving and bob anim playing?
#define WFL_NODROP				0x04000000			// don't drop this weapon type (taser glove, other "default" inventory items)
#define WFL_CROSSHAIR			0x08000000
#define WFL_SELECT_EMPTY		0X10000000			// allows this weapon to be selected with no ammo
*/
#define WFL_SAME_WEAPON_MASK     (WFL_DK_POWER|WFL_RIPGUN)


typedef userInventory_t *(*weaponGive_t) (userEntity_t *self);
typedef userInventory_t *(*give_t)(userEntity_t *self, int ammoCount);
typedef short (*select_t)(userEntity_t *self);
typedef void *(*command_t)(userInventory_t *inv, char *commandStr, void *data);
typedef void (*usew_t)(userInventory_t *inv,userEntity_t *self);
typedef void (*weapSpawn_t)(struct weaponInfo_s *winfo, userEntity_t *self, float respawn_time, touch_t touch);
typedef	void (*ammoSpawn_t)(struct weaponInfo_s *winfo, userEntity_t *self, int count, float respawn_time, touch_t touch);

typedef struct ammoGiveTake_s
{
	int				count;
	userEntity_t	*owner;
} ammoGiveTake_t;

typedef struct weaponCommand_s {
	char *text;                      // name of command
	void (*func)(struct edict_s *);              // function to run
} weaponCmd_t;

typedef struct weaponFrame_s {
	char model;                      // model num this frame is in
	char *name;                      // name of anim
	int first,last;                  // first/last frame of anim
} weaponFrame_t;

#define WEAPONTYPE_GLOVE		0
#define WEAPONTYPE_PISTOL		1
#define WEAPONTYPE_RIFLE		2
#define WEAPONTYPE_SHOULDER		3

typedef struct weaponInfo_s {
	char *weaponName;                // weapon name
	char *weaponNetName;             // weapon net name
	char *ammoName;                  // ammo name
	char *ammoNetName;               // ammo net name
	char *iconName;                  // icon def
	char *models[10];                // model files used by this weapon
	char *sounds[20];                // sound files used by this weapon
	weaponFrame_t frames[20];        // frame info -- weapon "ready" should always be first
	weaponCmd_t command[10];         // commands used by this weapon
	
	give_t		give_func;
	select_t	select_func;
	command_t	cmd_func;
	usew_t		use_func;
	weapSpawn_t	spawn_func;
	ammoSpawn_t	ammoSpawn_func;
	
	short weapon_t_size;            //	size to allocate for weapon_t structure
	short rating;                   //	value between 0 and 1000
	short ammo_max;					//	max amount of ammo this weapon can have
	short ammo_per_use;				//	amount of ammo used each shot
	short ammo_display_divisor;		//	amount to divide ammo count by for display
	//	1 for almost all weapons
	short initialAmmo;				//	ammount of ammo this weapon starts with when picked up
	
	float speed;					// how fast shots from this weapon travel (0 is instant-hit)
	float range;					// how far this weapon can fire
	float damage;					// how much damage a hit from this weapon will do
	float lifetime;
	short flags;				        // entity flags
	short nWeaponType;				// NoWeapon, Pistol, Rifle, Shoulder
	float fWeaponOffsetX1;
	float fWeaponOffsetY1;
	float fWeaponOffsetZ1;
	float fWeaponOffsetX2;
	float fWeaponOffsetY2;
	float fWeaponOffsetZ2;
	float fWeaponOffsetX3;
	float fWeaponOffsetY3;
	float fWeaponOffsetZ3;
	
	char *deathmsg[3];               // [0] victim/attacker [1] attacker/victim
	entity_type_t   EntityType;
	
	short display_order;            //  order in which to display on screen
    think_t     wentThink;
} weaponInfo_t;

typedef struct
{
/*
//	DELETE MY ASS!!
// all these fields need to remain the same for compatibility between weapons
char	*name;
void	(*use)(struct userInventory_s *ptr, userEntity_t *user);
void	*(*command)(struct userInventory_s *ptr, char *commandStr, void *data);
void	*precache;
unsigned long flags;
void	*dummy3;
void	*dummy4;
	*/
	//	!!!! FIELDS BELOW MUST BE IDENTICAL TO userInventory_t!!!!
	char			*name;
	invenUse_t		use;
	invenCommand_t	command;
	int				modelIndex;
	char			modelName[MAX_OSPATH];
	unsigned long	flags;
	mem_type		memType;
	void			*dummy4;
	//	!!!! FIELDS ABOVE MUST BE IDENTICAL TO userInventory_t!!!!
	
	// ammount of ammo left
	int				count;
	weaponInfo_t	*winfo;
} ammo_t;

typedef struct
{
/*
//	DELETE MY ASS!!
// all these fields need to remain the same for compatibility between weapons
char    *name;
void    (*use)(struct userInventory_s *ptr, userEntity_t *user);
void	*(*command)(struct userInventory_s *ptr, char *commandStr, void *data);
int		modelIndex;	//	for inventory display
unsigned long flags;
void	*dummy3;
void	*dummy4;
	*/
	//	!!!! FIELDS BELOW MUST BE IDENTICAL TO userInventory_t!!!!
	char			*name;
	invenUse_t		use;
	invenCommand_t	command;
	int				modelIndex;
	char			modelName[MAX_OSPATH];
	unsigned long	flags;
	mem_type		memType;
	void			*dummy4;
	//	!!!! FIELDS ABOVE MUST BE IDENTICAL TO userInventory_t!!!!
	
	// pointer to the ammo inventory
	ammo_t			*ammo;
	weaponInfo_t	*winfo;
	float lastFired;              // last time weapon was fired
} weapon_t;

///////////////////////////////////////////////////////////////////////////////
//	common export type
///////////////////////////////////////////////////////////////////////////////

typedef	struct	comon_export_s
{
#ifdef DEBUG_MEMORY
	void*			(*X_Malloc)						(size_t size, MEM_TAG tag, char* file, int line);
#else
	void*			(*X_Malloc)						(size_t size, MEM_TAG tag);
#endif
	void			(*X_Free)						(void* mem);
	
	//	generic list functions, com_list.cpp
	listHeader_t	*(*list_init)					(listHeader_t *list);
	void			(*list_kill)					(listHeader_t *list);
	listNode_t		*(*list_new_node)				(void);
	void			(*list_add_node)				(void *ptr, listHeader_t *list, char mem_type);
	void			(*list_delete_node)				(listHeader_t *list, listNode_t *node);
	void			(*list_remove_node)				(void *ptr, listHeader_t *list);
	void			*(*list_ptr)					(listNode_t *node);
	listNode_t		*(*list_node_next)				(listNode_t *node);
	listNode_t		*(*list_node_prev)				(listNode_t *node);
													
	float			(*AngleMod)						(float ang);
													
	void			(*GetMinsMaxs)					(userEntity_t *self, CVector &org, CVector &mins, CVector &maxs);
	void			(*ChangeYaw)					(userEntity_t *ent);
	void			(*ChangePitch)					(userEntity_t *ent);
	void			(*ChangeRoll)					(userEntity_t *ent);
    qboolean		(*TeamCheck)					(userEntity_t *self, userEntity_t *attacker);
	void			(*SetMovedir)					(userEntity_t *self);
	void			(*SpawnMarker)					(userEntity_t *self, CVector &org, char *model_name, float time);
    void			(*KickView)						(userEntity_t *entPlayer, CVector vecAngles, float fFOV, short sToVelocity, short sReturnVelocity);
    void			(*Damage)						(userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, 
														CVector &point_of_impact, CVector &damage_vec, float damage, unsigned long damage_flags);
													
	void			(*FlashClient)					(userEntity_t *self, float amt, float r, float g, float b, float vel=0);
	int				(*CanDamage)					(userEntity_t *target, userEntity_t *inflictor);
	int				(*RadiusDamage)					(userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, 
														float damage, unsigned long damage_flags, float fRadius );
													
	int				(*ValidTouch)					(userEntity_t *self, userEntity_t *other);
	int				(*ValidTouch2)					(userEntity_t *self, userEntity_t *other);
													
	void			(*UseTargets)					(userEntity_t *self, userEntity_t *other, userEntity_t *activator);
	void			(*Health)						(userEntity_t *self, float give, float max_health);
	void			(*Respawn)						(userEntity_t *self);
	bool			(*CheckTargetForActiveChildren)	(userEntity_t *self, userEntity_t *other, userEntity_t *activator);	//&&& AMW
					
	char			*(*vtos)						(CVector &v);
	
	int				(*Visible)						(userEntity_t *self, userEntity_t *ent);
	userEntity_t	*(*FindEntity)					(const char *name);
	userEntity_t	*(*FindClosestEntity)			(userEntity_t *self, const char *classname);
	userEntity_t	*(*FindClosestEntityPt)			( CVector &pos, const char *classname, const char *targetname);
	userEntity_t	*(*FindTarget)					(const char *name);
	userEntity_t	*(*FindNodeTarget)				(const char *name);
	userEntity_t	*(*DeathmatchSpawnPoint)		(const char *className);
	userEntity_t	*(*SelectSpawnPoint)			(const char *className, const char *target);
													
	userEntity_t	*(*FindFirstGroupMember)		(char *groupName, char *className = NULL);						//&&& AMW
	userEntity_t	*(*FindNextGroupMember)			(char *groupName, userEntity_t *head, char *className = NULL);	//&&& AMW
	void			(*TransformChildOffset)			( CVector &origin, CVector &offs, CVector &angles, bool bFlipYaw, CVector &out );  //&& AMW
													
	void			(*ClientPrint)					(userEntity_t *self, char *msg);
	void			(*AmbientVolume)				(userEntity_t *self, CVector &org, int volume);
	void			(*SetRespawn)					(userEntity_t *self, float time);
													
	void			(*CalcBoosts)					(userEntity_t *self);
	void			(*Boost_Icons)					(userEntity_t *self, byte icon_action, byte icon_type, int display_time);
	void			(*InformClientDeath)			(userEntity_t *self, byte gibbed);
													
	void			(*Sidekick_Update)				(userEntity_t *self, byte whom, int status, int val); // 5.25
													
	void			(*UpdateClientAmmo)				(userEntity_t *self);
													
	void			(*Showbbox)						(userEntity_t *self, CVector &org, CVector &mins, CVector &maxs);
	int				(*GetFrames)					(userEntity_t *self, char *seq_name, int *startFrame, int *endFrame);
	int				(*GetFrameData)					(userEntity_t *self, frameData_t *pFrameData);
													
	userEntity_t	*(*ClientVisible)				(userEntity_t *self);
													
	void			(*Warning)						(char *fmt, ...);
	void			(*Error)						(char *fmt, ...);
													
	void			(*SetClientModels)				(userEntity_t *self, userEntity_t *head, userEntity_t *torso, 
													userEntity_t *legs, userEntity_t *weapon);
	//float	(*AngleDiff)							(float ang1, float ang2);
	//void	(*Normal)								(CVector &v1, CVector &v2, CVector &v3, CVector &out);
	void	(*DrawLine)								(userEntity_t *self, CVector &org, CVector &end, float lifetime);
	void	(*DrawBoundingBox)						(userEntity_t *self, float lifetime);
													
	void	(*RotateBoundingBox)					(userEntity_t *self);
													
	//	FIXME:	merge these with thinkFuncs.		..
	userEntity_t	*(*SpawnFakeEntity)				(void);
	void			(*RemoveFakeEntity)				(userEntity_t *ent);
													
	void	(*Poison)								(userEntity_t *target, float damage, float time, float interval);
	void	(*TeleFrag)								(userEntity_t *self, float damage);
	int		(*GenID)								(void);
													
	//	com_friction.cpp							
	void			(*friction_apply)				(userEntity_t *self, frictionHook_t *fhook);
	frictionHook_t	*(*friction_init)				(userEntity_t *self, frictionHook_t *fhook);
	void			(*friction_set_sounds)			(userEntity_t *self, frictionHook_t *fhook, char *enter, char *exit);
	void			(*friction_set_physics)			(userEntity_t *self, frictionHook_t *fhook, float size, float mass, 
		float max_vel, int flags);					
	void			(*friction_remove)				(frictionHook_t *fhook);
													
	void			(*FrameUpdate)					(userEntity_t *self);
	void            (*AnimateEntity)				(userEntity_t *ent, short first, short last, unsigned short frameflags, float frametime);
													
	void			(*RegisterWeapon)				(weaponInfo_t *winfo);
	weaponInfo_t	*(*FindRegisteredWeapon)		(char *name);
	void			(*ClearRegisteredWeapons)		(void);
	void			*(*FindSpawnFunction)			(char *funcName);
	userEntity_t	*(*SpawnDynamicEntity)			(userEntity_t *self, char *szClassName, bool bCopyEpairs);
	
	void			(*BloodSplat)					(userEntity_t *impactedEnt, CVector &impactPos, cplane_t *plane);
	
	void			(*trackEntity)					(trackInfo_t *tinfo,short broadcasttype);
	void			(*untrackEntity)				(userEntity_t *ent, userEntity_t *to, short broadcasttype);
	
	userEntity_t	*(*FindRadius)					(userEntity_t *, CVector &, float);
	void			(*EarthCrack)					(CVector vecPos, short sPlaneIndex, short sHitModelIndex, int iEntityNumber, float fScale, float fRoll);
	
	// SCG[11/20/99]: Save game stuff
	void			(*AI_WriteSaveField1)			( FILE *f, field_t *field, byte *base );
	void			(*AI_WriteSaveField2)			( FILE *f, field_t *field, byte *base );
	void			(*AI_ReadField)					( FILE *f, field_t *field, byte *base );
	void			(*AI_SaveHook)					( FILE *f, void *pHook, int nSize );
	void			(*AI_LoadHook)					( FILE *f, void *pHook, int nSize );
	void			(*SavePersInventory)			( userEntity_t *pEnt );
	
	// cek[2-13-00]: using this all over the place
	char			*(*ProperNameOf)				( userEntity_t *self );
	int				(*TransitionType)				( char *mapFrom, char *mapTo, qboolean loadgame );

	void			(*InventorySave)				( void **buf, userEntity_t *self, int aryEntries );
	void			(*InventoryLoad)				( void **buf, userEntity_t *self, int aryEntries );
}	common_export_t;

__inline float rnd()
{
	return (float)rand () / (float)RAND_MAX;
}

#endif