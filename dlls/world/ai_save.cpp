#include "world.h"
//#include "ai_utils.h"
//#include "ai_move.h"
//#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
//#include "MonsterSound.h"
#include "ai.h"
//#include "actorlist.h"

void SEQUENCEMAP_AllocSequenceMap( userEntity_t *self );
char *AIATTRIBUTE_GetModelName( const char *szClassName );

#define	PLAYERHOOK(x) (int)&(((playerHook_t *)0)->x)
field_t player_hook_fields[] =
{
	{"owner",					PLAYERHOOK(owner),						F_EDICT},
	{"base_health",				PLAYERHOOK(base_health),				F_FLOAT},
	{"pain_chance",				PLAYERHOOK(pain_chance),				F_BYTE},
	{"intQuota",				PLAYERHOOK(intQuota),					F_INT},
	{"active_distance",			PLAYERHOOK(active_distance),			F_INT},
	{"back_away_dist",			PLAYERHOOK(back_away_dist),				F_SHORT},
	{"type",					PLAYERHOOK(type),						F_BYTE},
	{"upward_vel",				PLAYERHOOK(upward_vel),					F_FLOAT},
	{"forward_vel",				PLAYERHOOK(forward_vel),				F_FLOAT},
	{"run_speed",				PLAYERHOOK(run_speed),					F_FLOAT},
	{"walk_speed",				PLAYERHOOK(walk_speed),					F_FLOAT},
	{"attack_dist",				PLAYERHOOK(attack_dist),				F_FLOAT},
	{"jump_attack_dist",		PLAYERHOOK(jump_attack_dist),			F_FLOAT},
	{"fov",						PLAYERHOOK(fov),						F_FLOAT},
	{"max_jump_dist",			PLAYERHOOK(max_jump_dist),				F_INT},
	{"jump_chance",				PLAYERHOOK(jump_chance),				F_BYTE},
	{"attack_speed",			PLAYERHOOK(attack_speed),				F_FLOAT},
	{"fly_speed",				PLAYERHOOK(fly_speed),					F_FLOAT},
	{"fly_goal_offset",			PLAYERHOOK(fly_goal_offset),			F_FLOAT},
	{"ID",						PLAYERHOOK(ID),							F_INT},
	{"skill",					PLAYERHOOK(skill),						F_FLOAT},
	{"speak",					PLAYERHOOK(speak),						F_FLOAT},
	{"pain_finished",			PLAYERHOOK(pain_finished),				F_FLOAT},
	{"drown_time",				PLAYERHOOK(drown_time),					F_FLOAT},
	{"attack_finished",			PLAYERHOOK(attack_finished),			F_FLOAT},
	{"jump_time",				PLAYERHOOK(jump_time),					F_FLOAT},
	{"sound_time",				PLAYERHOOK(sound_time),					F_FLOAT},
	{"path_time",				PLAYERHOOK(path_time),					F_FLOAT},
	{"think_time",				PLAYERHOOK(think_time),					F_FLOAT},
	{"shake_time",				PLAYERHOOK(shake_time),					F_FLOAT},
	{"ambient_time",			PLAYERHOOK(ambient_time),				F_FLOAT},
	{"invulnerability_time",	PLAYERHOOK(invulnerability_time),		F_FLOAT},
	{"envirosuit_time",			PLAYERHOOK(envirosuit_time),			F_FLOAT},
	{"wraithorb_time",			PLAYERHOOK(wraithorb_time),				F_FLOAT},
	{"oxylung_time",			PLAYERHOOK(oxylung_time),				F_FLOAT},
	{"poison_time",				PLAYERHOOK(poison_time),				F_FLOAT},
	{"poison_next_damage_time",	PLAYERHOOK(poison_next_damage_time),	F_FLOAT},
	{"poison_damage",			PLAYERHOOK(poison_damage),				F_FLOAT},
	{"poison_interval",			PLAYERHOOK(poison_interval),			F_FLOAT},
	{"base_power",				PLAYERHOOK(base_power),					F_FLOAT},
	{"base_attack",				PLAYERHOOK(base_attack),				F_FLOAT},
	{"base_speed",				PLAYERHOOK(base_speed),					F_FLOAT},
	{"base_acro",				PLAYERHOOK(base_acro),					F_FLOAT},
	{"base_vita",				PLAYERHOOK(base_vita),					F_FLOAT},
	{"power_boost",				PLAYERHOOK(power_boost),				F_FLOAT},
	{"attack_boost",			PLAYERHOOK(attack_boost),				F_FLOAT},
	{"speed_boost",				PLAYERHOOK(speed_boost),				F_FLOAT},
	{"acro_boost",				PLAYERHOOK(acro_boost),					F_FLOAT},
	{"vita_boost",				PLAYERHOOK(vita_boost),					F_FLOAT},
	{"power_boost_time",		PLAYERHOOK(power_boost_time),			F_FLOAT},
	{"attack_boost_time",		PLAYERHOOK(attack_boost_time),			F_FLOAT},
	{"speed_boost_time",		PLAYERHOOK(speed_boost_time),			F_FLOAT},
	{"acro_boost_time",			PLAYERHOOK(acro_boost_time),			F_FLOAT},
	{"vita_boost_time",			PLAYERHOOK(vita_boost_time),			F_FLOAT},
	{"dflags",					PLAYERHOOK(dflags),						F_INT},
	{"items",					PLAYERHOOK(items),						F_INT},
	{"exp_flags",				PLAYERHOOK(exp_flags),					F_INT},
	{"stateFlags",				PLAYERHOOK(stateFlags),					F_INT},
	{"ai_flags",				PLAYERHOOK(ai_flags),					F_INT},
	{"th_state",				PLAYERHOOK(th_state),					F_BYTE},
	{"goalFlags",				PLAYERHOOK(goalFlags),					F_INT},
	{"last_zvel",				PLAYERHOOK(last_zvel),					F_FLOAT},
	{"last_xyvel",				PLAYERHOOK(last_xyvel),					F_FLOAT},
	{"pNodeList",				PLAYERHOOK(pNodeList),					F_RESTORE_AS_NULL},
	{"pPathList",				PLAYERHOOK(pPathList),					F_RESTORE_AS_NULL},
	{"last_origin",				PLAYERHOOK(last_origin),				F_VECTOR},
	{"turn_yaw",				PLAYERHOOK(turn_yaw),					F_FLOAT},
	{"sound1",					PLAYERHOOK(sound1),						F_LSTRING},
	{"sound2",					PLAYERHOOK(sound2),						F_LSTRING},
	{"cur_seq_type",			PLAYERHOOK(cur_seq_type),				F_INT},
	{"cur_seq_num",				PLAYERHOOK(cur_seq_num),				F_INT},
	{"cur_sequence",			PLAYERHOOK(cur_sequence),				F_RESTORE_AS_NULL},
	{"fxFrameNum",				PLAYERHOOK(fxFrameNum),					F_SHORT},
	{"fxFrameFunc",				PLAYERHOOK(fxFrameFunc),				F_FUNC},
	{"last_button0",			PLAYERHOOK(last_button0),				F_INT},
	{"viewmodel_start_frame",	PLAYERHOOK(viewmodel_start_frame),		F_INT},
	{"viewmodel_end_frame",		PLAYERHOOK(viewmodel_end_frame),		F_INT},
	{"viewmodel_frame_time",	PLAYERHOOK(viewmodel_frame_time),		F_FLOAT},
	{"weapon_fired",			PLAYERHOOK(weapon_fired),				F_RESTORE_AS_NULL},
	{"weapon_next",				PLAYERHOOK(weapon_next),				F_FUNC},
	{"killed",					PLAYERHOOK(killed),						F_INT},
	{"died",					PLAYERHOOK(died),						F_INT},
	{"fired",					PLAYERHOOK(fired),						F_INT},
	{"hit",						PLAYERHOOK(hit),						F_INT},
	{"begin_jump",				PLAYERHOOK(begin_jump),					F_FUNC},
	{"begin_stand",				PLAYERHOOK(begin_stand),				F_FUNC},
	{"begin_follow",			PLAYERHOOK(begin_follow),				F_FUNC},
	{"begin_path_follow",		PLAYERHOOK(begin_path_follow),			F_FUNC},
	{"begin_wander",			PLAYERHOOK(begin_wander),				F_FUNC},
	{"begin_attack",			PLAYERHOOK(begin_attack),				F_FUNC},
	{"begin_seek",				PLAYERHOOK(begin_seek),					F_FUNC},
	{"begin_pain",				PLAYERHOOK(begin_pain),					F_FUNC},
	{"wander_sound",			PLAYERHOOK(wander_sound),				F_FUNC},
	{"sight_sound",				PLAYERHOOK(sight_sound),				F_FUNC},
	{"begin_turn",				PLAYERHOOK(begin_turn),					F_FUNC},
	{"begin_path_wander",		PLAYERHOOK(begin_path_wander),			F_FUNC},
	{"begin_action",			PLAYERHOOK(begin_action),				F_FUNC},
	{"continue_think",			PLAYERHOOK(continue_think),				F_FUNC},
	{"special_think",			PLAYERHOOK(special_think),				F_FUNC},
	{"temp_think",				PLAYERHOOK(temp_think),					F_FUNC},
	{"begin_node_wander",		PLAYERHOOK(begin_node_wander),			F_FUNC},
	{"begin_node_retreat",		PLAYERHOOK(begin_node_retreat),			F_FUNC},
	{"sight",					PLAYERHOOK(sight),						F_FUNC},
	{"begin_time_stamp_follow",	PLAYERHOOK(begin_time_stamp_follow),	F_FUNC},
	{"follow_attack",			PLAYERHOOK(follow_attack),				F_FUNC},
	{"begin_follow_attack",		PLAYERHOOK(begin_follow_attack),		F_FUNC},
	{"find_target",				PLAYERHOOK(find_target),				F_FUNC},
	{"begin_transition",		PLAYERHOOK(begin_transition),			F_FUNC},
	{"fnAttackFunc",			PLAYERHOOK(fnAttackFunc),				F_FUNC},
	{"fnStartAttackFunc",		PLAYERHOOK(fnStartAttackFunc),			F_FUNC},
	{"fnTakeCoverFunc",			PLAYERHOOK(fnTakeCoverFunc),			F_FUNC},
	{"fnStartJumpUp",			PLAYERHOOK(fnStartJumpUp),				F_FUNC},
	{"fnJumpUp",				PLAYERHOOK(fnJumpUp),					F_FUNC},		
	{"fnStartJumpForward",		PLAYERHOOK(fnStartJumpForward),			F_FUNC},
	{"fnJumpForward",			PLAYERHOOK(fnJumpForward),				F_FUNC},
	{"fnChasingAttack",			PLAYERHOOK(fnChasingAttack),			F_FUNC},
	{"fnFindTarget",			PLAYERHOOK(fnFindTarget),				F_FUNC},
	{"fnStartCower",			PLAYERHOOK(fnStartCower),				F_FUNC},
	{"fnCower",					PLAYERHOOK(fnCower),					F_FUNC},
	{"fnStartIdle",				PLAYERHOOK(fnStartIdle),				F_FUNC},
	{"fnInAttackRange",			PLAYERHOOK(fnInAttackRange),			F_FUNC},
	{"fatigue",					PLAYERHOOK(fatigue),					F_INT},
	{"force_angles",			PLAYERHOOK(force_angles),				F_VECTOR},
	{"force_rate",				PLAYERHOOK(force_rate),					F_VECTOR},
	{"ideal_pitch",				PLAYERHOOK(ideal_pitch),				F_FLOAT},
	{"angles_last_change",		PLAYERHOOK(angles_last_change),			F_INT},
	{"camera",					PLAYERHOOK(camera),						F_EDICT},
	{"demon",					PLAYERHOOK(demon),						F_EDICT},
	{"strafe_dir",				PLAYERHOOK(strafe_dir),					F_BYTE},
	{"strafe_time",				PLAYERHOOK(strafe_time),				F_FLOAT},
//	{"head",					PLAYERHOOK(head),						F_EDICT},
//	{"torso",					PLAYERHOOK(torso),						F_EDICT},
//	{"legs",					PLAYERHOOK(legs),						F_EDICT},
//	{"weapon",					PLAYERHOOK(weapon),						F_EDICT},
//	{"pGoals",					PLAYERHOOK(pGoals),						F_RESTORE_AS_NULL},
	{"pGoals",					PLAYERHOOK(pGoals),						F_INT},
	{"pScriptGoals",			PLAYERHOOK(pScriptGoals),				F_RESTORE_AS_NULL},
	{"turn_dir",				PLAYERHOOK(turn_dir),					F_BYTE},
	{"ground_org",				PLAYERHOOK(ground_org),					F_VECTOR},
	{"caller",					PLAYERHOOK(caller),						F_RESTORE_AS_NULL},
	{"follow_dist",				PLAYERHOOK(follow_dist),				F_FLOAT},
	{"walk_follow_dist",		PLAYERHOOK(walk_follow_dist),			F_FLOAT},
	{"fov_rate",				PLAYERHOOK(fov_rate),					F_FLOAT},
	{"fov_desired",				PLAYERHOOK(fov_desired),				F_FLOAT},
	{"fov_last_change",			PLAYERHOOK(fov_last_change),			F_INT},
	{"nMoveCounter",			PLAYERHOOK(nMoveCounter),				F_INT},
	{"nTargetCounter",			PLAYERHOOK(nTargetCounter),				F_INT},
	{"nTargetFrequency",		PLAYERHOOK(nTargetFrequency),			F_INT},
	{"startPosition",			PLAYERHOOK(startPosition),				F_VECTOR},
	{"nAttackType",				PLAYERHOOK(nAttackType),				F_INT},
	{"nCurrentAttackType",		PLAYERHOOK(nCurrentAttackType),			F_INT},
	{"nAttackMode",				PLAYERHOOK(nAttackMode),				F_INT},
	{"nSpecificAttackMode",		PLAYERHOOK(nSpecificAttackMode),		F_INT},
	{"savedAttribute",			PLAYERHOOK(savedAttribute),				F_INT},
	{"nMovementState",			PLAYERHOOK(nMovementState),				F_INT},
	{"nWaitCounter",			PLAYERHOOK(nWaitCounter),				F_INT},
	{"nCanUseFlag",				PLAYERHOOK(nCanUseFlag),				F_INT},
	{"boundedBox",				PLAYERHOOK(boundedBox),					F_INT},
	{"nSpawnValue",				PLAYERHOOK(nSpawnValue),				F_INT},
	{"fTenUpdatesDistance",		PLAYERHOOK(fTenUpdatesDistance),		F_FLOAT},
	{"nMovingCounter",			PLAYERHOOK(nMovingCounter),				F_INT},
	{"autoAim",					PLAYERHOOK(autoAim),					F_INT},
	{"fTaskFinishTime",			PLAYERHOOK(fTaskFinishTime),			F_FLOAT},
	{"bOkToAttack",				PLAYERHOOK(bOkToAttack),				F_INT},
	{"fAttackTime",				PLAYERHOOK(fAttackTime),				F_FLOAT},
	{"nOriginalMoveType",		PLAYERHOOK(nOriginalMoveType),			F_INT},
	{"nFollowing",				PLAYERHOOK(nFollowing),					F_INT},
	{"fNextPathTime",			PLAYERHOOK(fNextPathTime),				F_FLOAT},
	{"nFlags",					PLAYERHOOK(nFlags),						F_INT},
	{"nTransitionIndex",		PLAYERHOOK(nTransitionIndex),			F_INT},
	{"fTransitionTime",			PLAYERHOOK(fTransitionTime),			F_FLOAT},
	{"iPlayerClass",			PLAYERHOOK(iPlayerClass),				F_INT},
	{"fLastHealthTick",			PLAYERHOOK(fLastHealthTick),			F_FLOAT},
	{"szScriptName",			PLAYERHOOK(szScriptName),				F_LSTRING},
	{"pEntityToLookAt",			PLAYERHOOK(pEntityToLookAt),			F_EDICT},
	{"fSoundOutDistance",		PLAYERHOOK(fSoundOutDistance),			F_FLOAT},
	{"pWantItem",				PLAYERHOOK(pWantItem),					F_EDICT},
	{"fMinAttenuation",			PLAYERHOOK(fMinAttenuation),			F_FLOAT},
	{"fMaxAttenuation",			PLAYERHOOK(fMaxAttenuation),			F_FLOAT},
//	{"bInScriptMode",			PLAYERHOOK(bInScriptMode),				F_INT},
//	{"bBounded",				PLAYERHOOK(bBounded),					F_INT},
//	{"bMoving",					PLAYERHOOK(bMoving),					F_INT},
//	{"bInTransition",			PLAYERHOOK(bInTransition),				F_INT},
//	{"bDucking",				PLAYERHOOK(bDucking),					F_INT},
//	{"bInAttackMode",			PLAYERHOOK(bInAttackMode),				F_INT},
//	{"bOffGround",				PLAYERHOOK(bOffGround),					F_INT},
//	{"bJumped",					PLAYERHOOK(bJumped),					F_INT},
//	{"bStuck",					PLAYERHOOK(bStuck),						F_INT},
//	{"bAttackPlayer",			PLAYERHOOK(bAttackPlayer),				F_INT},
//	{"bCarryingMikiko",			PLAYERHOOK(bCarryingMikiko),			F_INT},
//	{"bJustFired",				PLAYERHOOK(bJustFired),					F_INT},
//	{"bEmpty",					PLAYERHOOK(bEmpty),						F_INT},
	{"taskTracker",				PLAYERHOOK(taskTracker),				F_INT},
	{NULL, 0, F_INT}
};

// ----------------------------------------------------------------------------
//
// Name: AI_SaveHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SaveHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize )
{
	field_t			*field;
	void			*pHook = gstate->X_Malloc( nSize, MEM_TAG_MISC );

//	gstate->Con_Dprintf( "Saving hook for: %s\n", ent->className );
	
	// SCG[11/21/99]: Allocate temp memory for hook
	memcpy( pHook, ent->userHook, nSize );

	// change the pointers to lengths or indexes
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField1( f, field, ( byte * ) pHook );
	}

	// write the block
	com->AI_SaveHook( f, pHook, nSize );

	// now write any allocated data following the edict
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField2( f, field, ( byte * ) ent->userHook );
	}

	// SCG[11/21/99]: Free temp memory
	gstate->X_Free( pHook );
}

// ----------------------------------------------------------------------------
//
// Name: AI_LoadHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_LoadHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize )
{
	field_t		*field;

	ent->userHook = gstate->X_Malloc( nSize, MEM_TAG_HOOK );

	com->AI_LoadHook( f, ent->userHook, nSize );

	for( field = hook_fields; field->name; field++ )
	{
		com->AI_ReadField (f, field, (byte *)ent->userHook );
	}
}

// ----------------------------------------------------------------------------
//
// Name: AI_SavePlayerHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SavePlayerHook( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, player_hook_fields, sizeof( playerHook_t ) );
}

// ----------------------------------------------------------------------------
//
// Name: AI_LoadPlayerHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_LoadPlayerHook( FILE *f, struct edict_s *ent )
{
	AI_LoadHook( f, ent, player_hook_fields, sizeof( playerHook_t ) );
	ent->modelName = AIATTRIBUTE_GetModelName( ent->className );
	ent->s.modelindex = gstate->ModelIndex( ent->modelName );
}

// ----------------------------------------------------------------------------
//
// Name: AI_SaveMonsterPlayerHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SaveMonsterPlayerHook( FILE *f, struct edict_s *ent )
{
	AI_SaveHook( f, ent, player_hook_fields, sizeof( playerHook_t ) );

	// SCG[2/13/00]: Here comes some evil shit.  Save the goals
	playerHook_t	*pHook	= ( playerHook_t * ) ent->userHook;

	GOALTYPE nGoal = GOALTYPE_IDLE;

	if( pHook->pGoals != NULL )
	{
		// SCG[2/14/00]: find pathfollow
		GOAL_PTR	pGoal = pHook->pGoals->pTopOfStack;
		while( pGoal )
		{
			if( pGoal->nGoalType == GOALTYPE_PATHFOLLOW )
			{
				nGoal = pGoal->nGoalType;
				break;
			}

			pGoal = pGoal->pNext;
		}
	}

	com->AI_SaveHook( f, &nGoal, sizeof( GOALTYPE ) );
}

// ----------------------------------------------------------------------------
//
// Name: AI_LoadMonsterPlayerHook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SPAWN_CallInitFunction( userEntity_t *self, const char *szAIClassName );
void SEQUENCEMAP_Remove( int nModelIndex );
void AI_LoadMonsterPlayerHook( FILE *f, struct edict_s *ent )
{
//	gstate->UnlinkEntity( ent );

//	memset (&ent->area, 0, sizeof(ent->area));

	// SCG[11/24/99]: Pre load tasks
	// SCG[11/26/99]: We need to just clear everything out and re-initialize the AI 
	// SCG[11/26/99]: for this entity.

	// SCG[11/24/99]: Load
	AI_LoadHook( f, ent, player_hook_fields, sizeof( playerHook_t ) );

	GOALTYPE nGoal;
	com->AI_LoadHook( f, &nGoal, sizeof( GOALTYPE ) );

	playerHook_t	*pHook	= ( playerHook_t * ) ent->userHook;

	userEntity_t	*pEnemy			= ent->enemy;
	float			fHealth			= ent->health;
	touch_t			touch			= ent->touch;
	think_t			think			= ent->think;
	blocked_t		blocked			= ent->blocked;
	use_t			use				= ent->use;
	pain_t			pain			= ent->pain;
	die_t			die				= ent->die;
	remove_t		remove			= ent->remove;
	int				nFlags			= pHook->nFlags;
	unsigned long	dflags			= pHook->dflags;
	unsigned long	items			= pHook->items;
	unsigned long	exp_flags		= pHook->exp_flags;
	unsigned long	stateFlags		= pHook->stateFlags;
	unsigned long	ai_flags		= pHook->ai_flags;
	char			th_state		= pHook->th_state;
	unsigned long	goalFlags		= pHook->goalFlags;
	char			*szScriptName	= pHook->szScriptName;
	char			*szTarget		= ent->target;
	char			*szTargetname	= ent->targetname;

	SEQUENCEMAP_Remove( ent->s.modelindex );

	SPAWN_CallInitFunction( ent, ent->className );

	pHook				= ( playerHook_t * ) ent->userHook;

	ent->enemy			= pEnemy;
	ent->health			= fHealth;
	ent->touch			= touch;
	ent->think			= think;
	ent->blocked		= blocked;
	ent->use			= use;	
	ent->pain			= pain;	
	ent->die			= die;		
	ent->remove			= remove;
	pHook->nFlags		= nFlags;
	pHook->dflags		= dflags;		
	pHook->items		= items;	
	pHook->exp_flags	= exp_flags;
	pHook->stateFlags	= stateFlags;
	pHook->ai_flags		= ai_flags;
	pHook->th_state		= th_state;	
	pHook->goalFlags	= goalFlags;	
	ent->target			= szTarget;
	ent->targetname		= szTargetname;

	if( szScriptName != NULL )
	{
		pHook->szScriptName	= strdup( szScriptName );
	}

	if( nGoal == GOALTYPE_PATHFOLLOW )
	{
		AI_AddNewGoal( ent, nGoal );
	}
	else if( pHook->szScriptName != NULL )
	{
	    AI_AddScriptActionGoal( ent, pHook->szScriptName );
	}

	ent->modelName = AIATTRIBUTE_GetModelName( ent->className );
	ent->s.modelindex = gstate->ModelIndex( ent->modelName );
}
