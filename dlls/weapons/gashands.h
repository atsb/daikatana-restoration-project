typedef struct
{
    weapon_t    weapon;                       // weapon_t must be first entry
    float       age;

} gashands_t;

DllExport void weapon_gashands(userEntity_t *self);
extern weaponInfo_t gashandsWeaponInfo;
userInventory_t *gashandsGive (userEntity_t *self, int ammoCount);
short gashandsSelect(userEntity_t *self);
void *gashandsCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void gashandsUse(userInventory_s *ptr, userEntity_t *self);
void gashandsSmackMyBitchUp(userEntity_t *self);
void gashandsThink(userEntity_t *self);
