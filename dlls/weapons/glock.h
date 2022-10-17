#ifndef _GLOCK_H
    #define _GLOCK_H


typedef struct
{
    // first two fields must be the same
    weapon_t    weapon;

    int     clip;
} glockWeapon_t;

typedef struct
{

    int     shell_stop;
    long    my_count;
    float   rotate_x;
    float   reload;
    float   remove_time;
    float   last_touch_time;
    float   last_water;
    float   change_time;
    float   rotation_scale;

    CVector last_pos;
    CVector hit_water_pos;
    CVector  forward;
    CVector  right;
    CVector  up;

    short killMuzzleFlash;
} glockHook_t;

extern weaponInfo_t glockWeaponInfo;

DllExport void weapon_glock(userEntity_t *self);
DllExport void ammo_bullets (userEntity_t *self);

short           glock_select (userEntity_t *self);
userInventory_t *glock_give (userEntity_t *self, int ammoCount);
int             glock_reload (userEntity_t *self);
void            glock_ammo(userEntity_t *self);
void            glock_use (userInventory_s *ptr, userEntity_t *self);
void *glock_command (struct userInventory_s *ptr, char *commandStr, void *data);
short glockUseAmmo(userEntity_t *self);
void spawnShell(userEntity_t *self);
void killMuzzleFlash(userEntity_t *self);

int     glock_load_init (void);
void    glock_level_init (void);

#endif