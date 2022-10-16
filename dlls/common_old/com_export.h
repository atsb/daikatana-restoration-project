#ifndef	_COM_EXPORT_H_
#define	_COM_EXPORT_H_

//	for C++ classes that will be used in other DLLs
#ifndef	DllExport_Plus
#define DllExport_Plus __declspec(dllexport)
#endif
//	for functions the Quake engine calls (not in C++)
#ifndef	DllExport
#define DllExport	extern "C"	__declspec( dllexport )
#endif

///////////////////////////////////////////////////////////////////////////////
//	function prototypes
///////////////////////////////////////////////////////////////////////////////

//	com_main.cpp
DllExport	BOOL WINAPI dll_CommonEntry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

//	com_heap.cpp
DllExport	void			*com_malloc			(unsigned long size, char mem_type);
DllExport	void			com_free			(void *ptr);
DllExport	void			com_freemost		(void);
DllExport	void			com_freeall			(void);
DllExport	unsigned long	com_count_mem_type	(char mem_type);

//	com_list.cpp
DllExport	listHeader_t	*com_list_init			(listHeader_t *list);
DllExport	void			com_list_kill			(listHeader_t *list);
DllExport	listNode_t		*com_list_new_node		(void);
DllExport	void			com_list_add_node		(void *ptr, listHeader_t *list, char mem_type);
DllExport	void			com_list_delete_node	(listHeader_t *list, listNode_t *node);
DllExport	void			com_list_remove_node	(void *ptr, listHeader_t *list);
DllExport	void			*com_list_ptr			(listNode_t *node);
DllExport	listNode_t		*com_list_node_next		(listNode_t *node);
DllExport	listNode_t		*com_list_node_prev		(listNode_t *node);

DllExport	float	Sub_AngleMod (float ang);
DllExport	float	Sub_VecToYaw (Vector &vec);
DllExport	void	Sub_VecToAngles (Vector &vec1, Vector &vec2);
DllExport	float	Sub_YawDiff (float facing_yaw, float to_targ_yaw);
DllExport	void	Sub_BestDelta (float start_angle, float end_angle, float *best_delta, float *angle_diff);

DllExport	void	Sub_GetMinsMaxs (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs);
DllExport	void	Sub_ChangeYaw (userEntity_t *ent);
DllExport	void	Sub_ChangePitch (userEntity_t *ent);

DllExport	void	Sub_SetMovedir(userEntity_t *self);
DllExport	void	Sub_SpawnMarker (userEntity_t *self, Vector &org, char *model_name, float time);

DllExport	void	Sub_Damage (userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, float damage, int damage_type);
DllExport	int		Sub_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, int damage_type);
DllExport	int		Sub_ValidTouch (userEntity_t *self, userEntity_t *other);
DllExport	void	Sub_UseTargets (userEntity_t *self);
DllExport	void	Sub_Health (userEntity_t *self, float give, float max_health);
DllExport	void	Sub_Respawn (userEntity_t *self);

// routines for vec3_t (Quake engine) vector types
DllExport	void	Sub_ZeroVector (vec3_t v);
DllExport	void	Sub_SetVector (vec3_t v, float f1, float f2, float f3);
DllExport	void	Sub_MultVector (vec3_t v, float f1, vec3_t v2);
DllExport	void	Sub_PrintVector(char *str, Vector &vec);
DllExport	void	Sub_CopyVector (vec3_t v, vec3_t v2);

DllExport	int				Sub_Visible (userEntity_t *self, userEntity_t *ent);
DllExport	userEntity_t	*Sub_FindEntity (char *name);
DllExport	userEntity_t	*Sub_FindClosestEntity (userEntity_t *self, char *name);
DllExport	userEntity_t	*Sub_FindTarget (char *name);
DllExport	userEntity_t	*Sub_DeathmatchSpawnPoint (char *className);
DllExport	userEntity_t	*Sub_SelectSpawnPoint (char *className, char *target);


DllExport	void	Sub_ClientPrint (userEntity_t *self, char *msg);
DllExport	void	Sub_AmbientVolume (userEntity_t *self, Vector &org, int volume);
DllExport	void	Sub_SetRespawn (userEntity_t *self, float time, int reset_ambient, int ambient_vol);

DllExport	void	Sub_CalcBoosts (userEntity_t *self);
DllExport	void	Sub_UpdateClientAmmo (userEntity_t *self);

DllExport	void	Sub_Showbbox (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs);
DllExport	void	Sub_GetFrames (userEntity_t *self, char *seq_name, int *startFrame, int *endFrame);

DllExport	userEntity_t	*Sub_ClientVisible (userEntity_t *self);

DllExport	void	Sub_Warning (char *fmt, ...);
DllExport	void	Sub_Error (char *fmt, ...);

DllExport	void	Sub_SetClientModels (userEntity_t *self, userEntity_t *head, userEntity_t *torso, userEntity_t *legs, userEntity_t *weapon);
DllExport	float	Sub_AngleDiff (float ang1, float ang2);
DllExport	void	Sub_Normal (Vector v1, Vector v2, Vector v3, Vector &out);
DllExport	void	Sub_DrawLine (userEntity_t *self, userEntity_t *target, Vector start, Vector dir);

DllExport	userEntity_t	*Sub_SpawnFakeEntity ();
DllExport	void			Sub_RemoveFakeEntity (userEntity_t *ent);

DllExport	void	Sub_Poison (userEntity_t *target, float damage, float time, float interval);
DllExport	void	Sub_TeleFrag (userEntity_t *self, float damage);
DllExport	int		Sub_GenID (void);

DllExport	float	rnd (void);

#endif