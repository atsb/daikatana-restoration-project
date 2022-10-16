///////////////////////////////////////////////////////////////////////////////
//	com_weapons.cpp
//
//	Standard interface to weapons.dll.  Weapons calls some of the routines
//	here to set up lists and register weapons.  Other dlls use these lists
//	to make weapons available to the AI, scriptor, etc.
///////////////////////////////////////////////////////////////////////////////

//	header files in daikatana\user
#include	"p_user.h"
#include	"hooks.h"

#include	"p_global.h"

#define	MAX_WEAPONS	16	//	must be increased if more than 36 weapons exist
						//	per episode

static	weaponInfo_t	*com_WeaponList [MAX_WEAPONS];
static	int				com_WeaponCount;

///////////////////////////////////////////////////////////////////////////////
//	com_RegisterWeapon
//
///////////////////////////////////////////////////////////////////////////////

void	com_RegisterWeapon (weaponInfo_t *winfo)
{
	if (com_WeaponCount < MAX_WEAPONS)
	{
		com_WeaponList [com_WeaponCount] = winfo;
		com_WeaponCount++;
	}
	else
		gi.Con_Printf("com_RegisterWeapons: too many weapons!\n");
}

///////////////////////////////////////////////////////////////////////////////
//	com_FindWeaponInSlot
//
///////////////////////////////////////////////////////////////////////////////

weaponInfo_t *com_FindWeaponInSlot (int display_order)
{
	int		i;

	for (i = 0; i < com_WeaponCount; i++)
	{
		if (com_WeaponList[i]->display_order == display_order)
		{
			return	com_WeaponList [i];
		}
	}

	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	com_FindRegisteredWeapon
//
///////////////////////////////////////////////////////////////////////////////

weaponInfo_t	*com_FindRegisteredWeapon (char *name)
{
	int		i;

	for (i = 0; i < com_WeaponCount; i++)
	{
		if (!stricmp (com_WeaponList [i]->weaponName, name))
		{
			return	com_WeaponList [i];
		}
	}

	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	com_ClearRegisteredWeapons
//
///////////////////////////////////////////////////////////////////////////////

void	com_ClearRegisteredWeapons (void)
{
//	memset (com_WeaponList, 0x00, sizeof (weaponInfo_t) * MAX_WEAPONS);
	com_WeaponCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitWeapons
//
//	set up pointers to functions in com_sub.cpp
///////////////////////////////////////////////////////////////////////////////

void	com_InitWeapons (void)
{
	com.RegisterWeapon = com_RegisterWeapon;
	com.FindRegisteredWeapon = com_FindRegisteredWeapon;
	com.ClearRegisteredWeapons = com_ClearRegisteredWeapons;
}