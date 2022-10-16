#ifndef _AIM_H_
#define _AIM_H_

#define	MAX_WEAPON_TRACE	2000.0f

//	used so that ai_check_projectile_attack always checks the exact same direction
//	as ai_fire_projectile will fire
typedef	struct
{
	CVector	org;
	CVector	end;			//	ending position at weapon distance with spread adjustments
	CVector	spread_end;		//	ending position at MAX_WEAPON_TRACE with spread adjustments
	CVector	angles;			//	self->client->v_angle with pitch negated
	CVector	dir;			//	actual direction of attack after spread adjustments
} AIMDATA, *AIMDATA_PTR;

void ai_lead_target( userEntity_t *targ, const CVector &offset, const CVector &org, 
  					 float speed, CVector &predicted_org );


AIMDATA_PTR ai_aim_curWeapon( userEntity_t *self, ai_weapon_t *weapon );
AIMDATA_PTR ai_aim_playerWeapon( userEntity_t *self, ai_weapon_t *weapon, float time );
AIMDATA_PTR ai_aim_straight( userEntity_t *self, ai_weapon_t *weapon );
AIMDATA_PTR ai_wack_aim( userEntity_t *self, ai_weapon_t *weapon );

int W_IsTargetCrouching(userEntity_t *target);

#endif _AIM_H_