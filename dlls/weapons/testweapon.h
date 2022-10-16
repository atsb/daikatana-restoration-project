#ifndef _TESTWEAPON_H
#define _TESTWEAPON_H

/////////////////////////////////////////////////////////////////////
// prototypes
/////////////////////////////////////////////////////////////////////

int				tw_load_init (void);
void			tw_level_init (void);

void			testweapon_select (userEntity_t *self);
userInventory_t *testweapon_give (userEntity_t *self, int ammoCount);
void			testweapon_use (userInventory_s *ptr, userEntity_t *self);
void			list_sequences (userEntity_t *self);
void			testweapon_runseq (userEntity_t *self);

#endif