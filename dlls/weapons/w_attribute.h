
#ifndef _W_ATTRIBUTE_H
#define _W_ATTRIBUTE_H

class CWeaponAttributes
{
private:
	char weaponName[64];
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
	float fWeaponOffsetX1;
	float fWeaponOffsetY1;
	float fWeaponOffsetZ1;
	float fWeaponOffsetX2;
	float fWeaponOffsetY2;
	float fWeaponOffsetZ2;
	float fWeaponOffsetX3;
	float fWeaponOffsetY3;
	float fWeaponOffsetZ3;

  short display_order;

public:
	
	CWeaponAttributes();
	CWeaponAttributes( char *name );
	~CWeaponAttributes();

//	void* CWeaponAttributes::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CWeaponAttributes::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CWeaponAttributes::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CWeaponAttributes::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void SetWeaponName( char *name );
	void SetRating( short nNewValue )				{ rating = nNewValue; }
	void SetAmmoMax( short nNewValue )				{ ammo_max = nNewValue; }
	void SetAmmoPerUse( short nNewValue )			{ ammo_per_use = nNewValue; }
	void SetAmmoDisplayDivisor( short nNewValue )	{ ammo_display_divisor = nNewValue; }
	void SetInitialAmmo( short nNewValue )			{ initialAmmo = nNewValue; }

	void SetSpeed( float fNewValue )				{ speed = fNewValue; }
	void SetRange( float fNewValue )				{ range = fNewValue; }
	void SetDamage( float fNewValue )				{ damage = fNewValue; }
	void SetLifeTime( float fNewValue )				{ lifetime = fNewValue; }
	void SetX1Offset( float fNewValue )				{ fWeaponOffsetX1 = fNewValue; }
	void SetY1Offset( float fNewValue )				{ fWeaponOffsetY1 = fNewValue; }
	void SetZ1Offset( float fNewValue )				{ fWeaponOffsetZ1 = fNewValue; }
	void SetX2Offset( float fNewValue )				{ fWeaponOffsetX2 = fNewValue; }
	void SetY2Offset( float fNewValue )				{ fWeaponOffsetY2 = fNewValue; }
	void SetZ2Offset( float fNewValue )				{ fWeaponOffsetZ2 = fNewValue; }
	void SetX3Offset( float fNewValue )				{ fWeaponOffsetX3 = fNewValue; }
	void SetY3Offset( float fNewValue )				{ fWeaponOffsetY3 = fNewValue; }
	void SetZ3Offset( float fNewValue )				{ fWeaponOffsetZ3 = fNewValue; }
	void SetDisplayOrder( float fNewValue )   { display_order   = fNewValue; }

	char *GetWeaponName()	{ return weaponName; }
	short GetRating()		{ return rating; }
	short GetAmmoMax()		{ return ammo_max; }
	short GetAmmoPerUse()	{ return ammo_per_use; }
	short GetAmmoDisplayDivisor()	{ return ammo_display_divisor; }
	short GetInitialAmmo()	{ return initialAmmo; }

	float GetSpeed()		{ return speed; }
	float GetRange()		{ return range; }
	float GetDamage()		{ return damage; }
	float GetLifeTime()		{ return lifetime; }
	float GetX1Offset()		{ return fWeaponOffsetX1; }
	float GetY1Offset()		{ return fWeaponOffsetY1; }
	float GetZ1Offset()		{ return fWeaponOffsetZ1; }
	float GetX2Offset()		{ return fWeaponOffsetX2; }
	float GetY2Offset()		{ return fWeaponOffsetY2; }
	float GetZ2Offset()		{ return fWeaponOffsetZ2; }
	float GetX3Offset()		{ return fWeaponOffsetX3; }
	float GetY3Offset()		{ return fWeaponOffsetY3; }
	float GetZ3Offset()		{ return fWeaponOffsetZ3; }

	short GetDisplayOrder() { return display_order; }
};

int WEAPONATTRIBUTE_ReadFile( const char *szFileName );
void WEAPONATTRIBUTE_SetInfo( weaponInfo_t *winfo );

#endif _W_ATTRIBUTE_H
