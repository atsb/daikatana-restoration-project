#ifndef _C4_H
    #define _C4_H
//#include "ClientEntityManager.h"
//#include "TrackLightBlinking.h"

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define C4_MAX_GLOBS       128
    #define C4_BEEP_DELAY      5

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    float killtime;
    float lastframe;
    userEntity_t *controller;
} c4_t;

typedef struct
{
    float           beep_time;					//When should we beep next?
    float           time_to_die;				//How long should we live?
    float           last_warning_beep;
    float           warning_beep_frequency;
    c4_t*			my_master;					//Who launched me into this world?
    userEntity_t*	trigger;					// pointer to my trigger brush
    short			onWorld;					// stuck to world   
    frictionHook_t*	fhook;						// Content friction hook
	userEntity_t	*owner;
//    CTrackLightBlinking *C4Light;
}moduleHook_t;

typedef struct c4trigHook_s
{
	userEntity_t*	c4;
} c4trigHook_t;
	
//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t c4WeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_c4viz (userEntity_t *self);
DllExport void ammo_c4 (userEntity_t *self);

userInventory_t *c4Give(userEntity_t *self, int ammoCount);
short c4Select(userEntity_t *self);
void *c4Command(struct userInventory_s *ptr, char *commandStr, void *data);
void c4Use(userInventory_s *ptr, userEntity_t *self);
void c4Explode(userEntity_t *self);
void c4Detonate(userEntity_t *owner,short staggered);
void c4Think(userEntity_t *self);
void c4Touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void c4Launch(userEntity_t *self);
void c4Use(struct userInventory_s *ptr, userEntity_t *self);
void c4ExitLevel(userEntity_t *self);

#endif