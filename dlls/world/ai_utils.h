#ifndef	_AI_UTILS
#define	_AI_UTILS

#define ON_LEFT     0
#define ON_RIGHT    1
#define COLLINEAR   2

#define CHECK_GROUND_DISTANCE   72.0f

void AI_DisableAI();
void AI_EnableAI();
int AI_IsAIDisabled();


__inline int GetCurrentEpisode()
{
	return (int)sv_episode->value;
}

void AI_Dprintf (char *fmt, ...);
void	ai_debug_info			(userEntity_t *self, char *routine, char* extra);
void	ai_debug_print			(userEntity_t *self, char *fmt, ...);
void	ai_setfov				(userEntity_t *self, float fov_degrees);
int		ai_client_near			(userEntity_t *self);
float	ai_max_height			(userEntity_t *self, float v0);
float	ai_jump_vel				(userEntity_t *self, float distxy, float distz, float up_vel);
float AI_ComputeNeededJumpVelocity(userEntity_t *self, float distxy, float up_vel);
float	ai_max_jump_dist		(float forward_vel, float upward_vel);
void	ai_register_sounds		(userEntity_t* self);

//NSS[12/1/99]:New Collision detection 
int AI_IsInSmallSpace( userEntity_t *self);
void AI_FindNewCourse( userEntity_t *self, CVector *Dir);
void AI_HandleWallCollision(  userEntity_t *self, CVector &forward, CVector &normal, CVector &dir );

int	AI_CheckAirTerrain( userEntity_t *self, const CVector &dir, float dist );
int IsDead( userEntity_t *self );
int AI_CanAttack( userEntity_t *self, userEntity_t *enemy );
int AI_CanAttack( userEntity_t *self );
float AI_ComputeTurningDiameter( userEntity_t *self );

void AI_DoEvasiveAction( userEntity_t *self );

int AI_WillItBeCaughtInLoop( userEntity_t *self, const CVector &destPoint );
float AI_FindTurnRateAdjuster( userEntity_t *self, const CVector &destPoint );

// ISP: 03-15-99	prototyped here because I did not want to create a new file
//					the AI units need this function to activate doors and platforms
void door_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator);


int AI_IsGroundBelowBetween( userEntity_t *self, CVector &point1, CVector &point2, float fCheckGroundDistance = CHECK_GROUND_DISTANCE );
playerHook_t *AI_GetPlayerHook( userEntity_t *self );
int AI_IsAlive( userEntity_t *self );
void AI_SetMovingCounter( playerHook_t *hook, int nValue );
int AI_IsDucking( playerHook_t *hook );
void AI_StartDucking( userEntity_t *self );
void AI_EndDucking( userEntity_t *self );
float AI_GetRunFollowDistance( playerHook_t *hook );
float AI_GetWalkFollowDistance( playerHook_t *hook );
void AI_UpdateCurrentNode( userEntity_t *self );
void AI_ZeroVelocity( userEntity_t *self );
void AI_SetVelocity( userEntity_t *self, CVector &vector, float fSpeed );
int AI_GetStraightMoveDistance( playerHook_t *hook );
int AI_IsOkToMoveStraight( userEntity_t *self, CVector &destPoint, 
						   float fXYDistance, float fZDistance );

void AI_ClearTeam( userEntity_t *self );
void AI_SetOwner( userEntity_t *self, userEntity_t *pOwner );

void AI_InitNodeList( userEntity_t *self );
NODELIST_PTR AI_DeleteNodeList( NODELIST_PTR pNodeList );

bool AI_IsInsideBox( userEntity_t *self, CVector point, trace_t& trace );
bool AI_CheckLineCollision( userEntity_t *self, CVector endPt );

//-----------------------------------------------------------------------------
// inline functions defined below
//-----------------------------------------------------------------------------
__inline void AI_LeftVector( CVector &sourceVector, CVector &outVector )
{
	// left side
	outVector.Set( -sourceVector.y, sourceVector.x, 0.0f );
}

__inline void AI_RightVector( CVector &sourceVector, CVector &outVector ) 
{
	// right side
	outVector.Set( sourceVector.y, -sourceVector.x, 0.0f );
}

void AI_ComputeAbsoluteWeaponOffset( userEntity_t *self, const CVector &weaponOffset, CVector &absoluteOffset );
int AI_IsClearShot( userEntity_t *self, userEntity_t *pEntity );
void AI_GetEntityOrigin( userEntity_t *self, CVector &origin );
void AI_Suicide( userEntity_t *self, int damage );
int AI_IsSidekick( playerHook_t *hook );
int AI_IsSuperfly( playerHook_t *hook );
int AI_IsMikiko( playerHook_t *hook );
int AI_IsPointObstructedByEntity( userEntity_t *self, const CVector &point );
int AI_FindGroundPoint( userEntity_t *self, CVector &testPoint, CVector &groundPoint );


int AI_IsNeedHealth( userEntity_t *self );
int AI_IsNeedWeapon( userEntity_t *self, char *szWeaponName );
int AI_IsNeedAmmo( userEntity_t *self, char *szAmmoName );
userEntity_t *AI_DetermineNeededItem( userEntity_t *self );
userEntity_t *AI_DetermineWantItem( userEntity_t *self );
int AI_IsWeaponAvailable( userEntity_t *self, char *szWeaponName );

float AI_GetDirDot( userEntity_t *self, userEntity_t *attacker );
bool AI_IsFrameInRange( userEntity_t *self, int attackFrame );

userEntity_t *AI_GetTargetedEntity( userEntity_t *self );
int AI_WhichSide( CVector &startPoint, CVector &endPoint, CVector &testPoint );
float AI_ComputeDistanceToPoint( userEntity_t *self, CVector &point );

int AI_CanAttackEnemy( userEntity_t *self );

void AI_SetThinkFunc( userEntity_t *self, think_t fnThinkFunc );
void AI_SetNextThinkTime( userEntity_t *self, const float fTime );

int AI_IsJustFired( playerHook_t *hook );
void AI_SetJustFired( playerHook_t *hook );
void AI_DecreaseJustFired( playerHook_t *hook );

void AI_DoDeathTarget( userEntity_t *self );
void AI_DoSpawnName( userEntity_t *self );

void AI_StopEntity( userEntity_t *self );
int AI_IsRangeAttack( playerHook_t *hook );

int AI_IsEntityBelow( userEntity_t *self, userEntity_t *pEntity );
int AI_IsEntityAbove( userEntity_t *self, userEntity_t *pEntity );

userEntity_t *AI_IsEntityBelow( userEntity_t *self );
userEntity_t *AI_IsEntityAbove( userEntity_t *self );

int AI_FindCollisionFreePoint( userEntity_t *self, CVector &freePoint );
void AI_SubLevelTransition();


#endif _AI_UTILS