///////////////////////////////////////////////////////////////////////////////
//	p_hierarchy.c
//
//	Nelno,	4/21/98
//
//	each entity has a pointer to a list of its children, self->child_list
//		if self->child_list == NULL, entity has no children
//		otherwise, self->child_list points to a childList_t struct
//
//	TODO:	move to client
//	TODO:	make a type field instead of keeping types in flags
//	TODO:	clean out old fields in childInfo_t
//	TODO:	look at vectopyr
///////////////////////////////////////////////////////////////////////////////

#include	"p_user.h"
#include	"p_global.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	MAX_SURFACE_NAMES	16

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

cvar_t	*hr_testvert;
cvar_t	*hr_shownormals;

static	edict_t	*forward_normal = NULL;
static	edict_t	*right_normal = NULL;
static	edict_t	*up_normal = NULL;

static	int		hr_NumSurfNames = 0;
char			hr_SurfNames [MAX_SURFACE_NAMES][32];
char			hr_SurfFlags [MAX_SURFACE_NAMES];		//	flags for each surface name 
													//	visible/invisible, etc.

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

/* ---------------------------                ------------------------------ */
/* ++++++++++++++++++++++++++ utility functions ++++++++++++++++++++++++++++ */
/* ---------------------------                ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//	hr_AddSurfaceName
//
///////////////////////////////////////////////////////////////////////////////
/*
void	hr_AddSurfaceName  (char *line, int char_count, int is_comment)
{
	int		i;

	//	strip spaces from the end of the name
	for (i = char_count; i > 0; i--)
	{
		if (line [i] == ' ')
		{
			line [i] = 0x00;
			char_count--;
		}
		else 
			break;
	}

	if (is_comment == false && char_count > 0)
	{
		//	end of line, so copy current line to hr_SurfName
		line [char_count] = 0x00;
		strcpy (hr_SurfNames [hr_NumSurfNames], line);
		hr_NumSurfNames++;
	}

}
*/
void	hr_AddSurfaceName  (char *line, int char_count, int is_comment)
{
	int		i;
	char	*comma;
	// mdm99.02.25 - kick ass andrew thanks for making these too small
	// amw 03.09.99 - whatever bitch, it's not my code ;)
	char	name [256], flags [256];

	memset (name, 0x00, sizeof (name));
	memset (flags, 0x00, sizeof (flags));
	//	find name, which is the first value on the line,
	//	and	the flags, which is the second value on the line
	comma = strstr (line, ",");
	if (!is_comment && comma != NULL)
	{
		//	assumes line has been memset to 0 before new chars were read in
		strncpy (name, line, (int)(comma - line));
		char_count = strlen (name);
		strncpy (flags, comma + 1, (int)(strlen (line) - char_count - 1));
	}
	else
	{
		strcpy (name, line);
		memset (flags, 0x00, sizeof (flags));
	}

	//	strip spaces from the end of the name
	for (i = char_count; i > 0; i--)
	{
		if (name [i] == ' ')
		{
			name [i] = 0x00;
			char_count--;
		}
		else 
			break;
	}

	if (is_comment == false && char_count > 0)
	{
		//	end of line, so copy current line to hr_SurfName
		name [char_count] = 0x00;
		strcpy (hr_SurfNames [hr_NumSurfNames], name);

		//	set flags for this surface name
		hr_SurfFlags [hr_NumSurfNames] = atoi (flags);

		hr_NumSurfNames++;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	hr_ReadSurfaceNames
//
///////////////////////////////////////////////////////////////////////////////

void	hr_ReadSurfaceNames (void)
{
	FILE	*file;
	int		file_start, file_len;
	char	line [256];
	char	ch;
	int		char_count, is_comment;

	file_len = gi.FS_Open ("models/global/hr_names.txt", &file);
	if (file_len < 0)
		Sys_Error ("ERROR: could not find models/global/hr_names.txt!\n");

	file_start = gi.FS_Tell(file);
	gi.FS_Seek (file, file_start, SEEK_SET);

	char_count = hr_NumSurfNames = 0;
	is_comment = false;

	// mdm99.02.25 - init the memory we are about to use
	memset (line, 0x00, sizeof (line));

	while (file_len > 0)
	{
		ch = gi.FS_Getc (file);
		if (char_count == 0 && ch == '/')
			is_comment = true;

		if (ch != '\r' && ch != '\n')
		{
			// mdm99.02.25 - don't let it go out of bounds  (cause it used to)
			if (char_count < 256)
			{
				//	don't add CR/LF to endm
				line [char_count] = ch;
				char_count++;
			}
		}

		if (ch == '\n')
		{
			hr_AddSurfaceName (line, char_count, is_comment);

			char_count = 0;
			is_comment = false;
			memset (line, 0x00, sizeof (line));
		}

		file_len--;
	}

	//	make sure we don't leave out the last line if there is no
	//	carriage return
	if (char_count > 0)
	{
		hr_AddSurfaceName (line, char_count, is_comment);
	}

	gi.FS_Close (file);
}

///////////////////////////////////////////////////////////////////////////////
//	hr_IndexForSurfaceName
//
///////////////////////////////////////////////////////////////////////////////

int	hr_IndexForSurfaceName (char *surfName)
{
	int		i;

	for (i = 0; i < hr_NumSurfNames; i++)
	{
		if (!stricmp (hr_SurfNames [i], surfName))
			return	i;
	}

	return	-1;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_InitInfo
//
//	sets up hrInfo struct for communication to client
///////////////////////////////////////////////////////////////////////////////

void hr_InitInfo (edict_t *parent, edict_t *child, int type, int surfIndex, int vertIndex, unsigned long flags)
{
	//	set hrInfo structure for communication to client
	if (flags & HRF_SERVER_ONLY)
	{
		//	HRF_SERVER_ONLY can cause a hierarhical model to be reset
		//	(if the child is already a hierarchical model) on the 
		//	client, but never cause one to be initialized
		memset (&child->s.hrInfo, 0x00, sizeof (hrInfo_t));
	}
	else
	{
		child->s.hrInfo.parentNumber = parent->s.number;
		child->s.hrInfo.hrFlags = flags;
		child->s.hrInfo.hrType = type;
		child->s.hrInfo.surfaceIndex = surfIndex;
		child->s.hrInfo.vertexIndex = vertIndex;
	}
}

///////////////////////////////////////////////////////////////////////////////
// hr_vectoypr
//
//	huh???  FIXME!!  This doesn't make any sense...  a single vector cannot
//	indicate roll
///////////////////////////////////////////////////////////////////////////////

void	hr_vectopyr (CVector &vec1, CVector &vec2)
{
	float	yaw, pitch, roll;

	if (vec1 [YAW] == 0 && vec1 [PITCH] == 0)
	{
		roll = yaw = 0;

		if (vec1 [ROLL] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec1[YAW], vec1[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		pitch = (int) (atan2(vec1[PITCH], vec1[ROLL]) * 180 /M_PI);
		if(pitch <0)
			pitch += 360;
		
		roll = (int) (atan2(vec1[ROLL], vec1[YAW]) * 180 /M_PI);
		if(roll <0)
			roll += 360;

	}

	vec2.x = -pitch; 
	vec2.y = yaw;
	vec2.z = roll;
}


/* ---------------------------                ------------------------------ */
/* +++++++++++++++++++++++++++ list functions ++++++++++++++++++++++++++++++ */
/* ---------------------------                ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//	hrl_CreateChildList
//
//	creates a new child list for parent
///////////////////////////////////////////////////////////////////////////////

childList_t	*hrl_CreateChildList (edict_t *parent, edict_t *child)
{
	if (parent->child_list != NULL)
		gi.Error ("hrl_CreateChildList: child_list != NULL\n");

	parent->child_list = (childList_t *) gi.X_Malloc(sizeof (childList_t), MEM_TAG_LEVEL);
	
	((childList_t *) parent->child_list)->next = NULL;
	((childList_t *) parent->child_list)->child = child;

	//	set hparent flag on this parent
	parent->flags |= FL_HPARENT;
	//	set hchild flag on this child
	child->flags |= FL_HCHILD;

	return	(childList_t *) parent->child_list;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_Normalthink
//
///////////////////////////////////////////////////////////////////////////////

void	hr_NormalThink (edict_t *ent)
{
	ent->spawnflags |= 0x01;
	VectorMA( ent->s.origin, ent->movedir, 64.0, ent->s.old_origin );

//	ent->nextthink = level.time + 0.05;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_ShowNormal
//
///////////////////////////////////////////////////////////////////////////////

void	hr_ShowNormal (edict_t **ent, CVector &org, CVector &normal)
{
	edict_t	*damn;

	if (!(* ent))
		* ent = P_SpawnEdict ();

	damn = * ent;

	damn->movetype = MOVETYPE_NONE;
	damn->solid = SOLID_NOT;
	
	if (hr_shownormals->value == 2)
		damn->s.renderfx |= RF_SPOTLIGHT | RF_TRANSLUCENT;
	else
	{
		damn->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
		damn->s.frame = 2;
	}
	
	damn->s.modelindex = 1;
	damn->s.skinnum = 0xdcdddedf;
	damn->className = "normal";

	damn->s.origin = org;
	damn->movedir = normal;

	damn->spawnflags = 0x80000001;

//	damn->think = hr_NormalThink;

	damn->s.mins.Set(-8, -8, -8);
	damn->s.maxs.Set(8, 8, 8);
	gi.linkentity (damn);

	hr_NormalThink (damn);
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_AddChildToList
//
//
///////////////////////////////////////////////////////////////////////////////

childList_t	*hrl_AddChildToList (edict_t *parent, edict_t *child)
{
	childList_t	*newlist;

	if (parent->child_list == NULL)
		gi.Error ("hrl_AddChildToList: Tried to add to NULL child_list.\n");

	//	add to the front of the list
	newlist = (childList_t *) gi.X_Malloc(sizeof (childList_t), MEM_TAG_LEVEL);
	newlist->next = parent->child_list;
	newlist->child = child;

	parent->child_list = newlist;

	//	set hchild flag on this child
	child->flags |= FL_HCHILD;

	return	newlist;
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_RemoveChildFromList
//
//	removes child from parent's child_list
///////////////////////////////////////////////////////////////////////////////

void	hrl_RemoveChildFromList (edict_t *parent, edict_t *child, int free_child)
{
	childList_t	*clist, *prev;
	int			can_free;

	if (parent->child_list == NULL)
		gi.Error ("hrl_RemoveChildFromList: Tried to remove from a NULL child_list.\n");

	//	find the list entry with a matching child
	clist = parent->child_list;
	prev = NULL;
	while (clist && clist->child != child)
	{
		prev = clist;
		clist = clist->next;
	}

	if (!clist)
		return;
//		gi.Error ("hrl_RemoveChildFromList: Child is not in child_list.\n");

	if (!prev)
	{
		//	first child in the list
		parent->child_list = clist->next;

		//	if no more children in parent's list, clear parent flag
		if (!parent->child_list)
			parent->flags -= (parent->flags & FL_HPARENT);
	}
	else
		prev->next = clist->next;

	//	determine if this child can be freed by this function
	//	if HRF_NOREMOVE is set this function cannot remove the child entity
	can_free = !(clist->info->flags & HRF_NOREMOVE);

	//	free the info structure if it exists
	if (clist->info)
		gi.X_Free(clist->info);

	gi.X_Free(clist);

	//	clear child flag
	child->flags -= (child->flags & FL_HCHILD);

	//	free the child if desired and HRF_NOREMOVE is not set
	if (free_child && can_free)
		P_FreeEdict (child);
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_FindChildInList
//
//	returns a childList_t * for child or NULL if child is not in the parent's
//	list
///////////////////////////////////////////////////////////////////////////////

childList_t	*hrl_FindChildInList (edict_t *parent, edict_t *child)
{
	childList_t	*clist;

	clist = parent->child_list;

	while (clist && clist->child != child)
		clist = clist->next;

	return	clist;
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_DeleteList 
//
//	frees parent's entire child_list
///////////////////////////////////////////////////////////////////////////////

void	hrl_DeleteList (edict_t *parent, int free_children)
{
	if (!parent->child_list)
		return;

	while (parent->child_list)
		hrl_RemoveChildFromList (parent, ((childList_t *) parent->child_list)->child, free_children);
}

/* ---------------------------                ------------------------------ */
/* ++++++++++++++++++++++++ hierarchy functions ++++++++++++++++++++++++++++ */
/* ---------------------------                ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

childList_t	*hr_AddChild (edict_t *parent, edict_t *child)
{
	if (parent->child_list == NULL)
		return	hrl_CreateChildList (parent, child);
	else
		return	hrl_AddChildToList (parent, child);
}

///////////////////////////////////////////////////////////////////////////////
//	hr_Bmodel
//
///////////////////////////////////////////////////////////////////////////////

void	hr_Bmodel (edict_t *child, childInfo_t *cinfo)
{
	gi.Con_Printf("hr_BModel:  HR_BMODEL type not implemented.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	hr_Relative
//
//	movement and rotation of child is based on it's position relative to parent
//	when it is first initialized as a child
///////////////////////////////////////////////////////////////////////////////

void	hr_Relative_Old (edict_t *child, childInfo_t *cinfo)
{
	edict_t		*parent;
	CVector		new_ang, forward, right, up, p_org, save_ang;
	float		temp;

	parent = cinfo->parent;

	new_ang = parent->s.angles;

	//	Nelno: FIXME:	should we still be negating pitch in Q2???
	new_ang.x = -new_ang.x;

	AngleToVectors (new_ang, forward, right, up);

	p_org = parent->s.origin;

	temp = cinfo->dir [1] * cinfo->dist;
	forward = forward * temp;
	p_org = p_org - forward;

	temp = cinfo->dir [2] * cinfo->dist;
	up = up * temp;
	p_org = p_org + up;

	temp = cinfo->dir [0] * cinfo->dist;
	right = right * temp;
	p_org = p_org + right;

	child->s.origin = p_org;

	//	THIS IS A MAJOR HACK
	//	probably should implement a base_angles field for physics which
	//	is always added to an entity's angles after rotation... or just
	//	use parent's angles if it's a FL_HCHILD?

	//	rotate child to its initial position relative to parent
	if (!(cinfo->flags & HRF_NOYAW))
	{
		child->s.angles = parent->s.angles + cinfo->ang_delta;

		//	save childs angles
		child->s.angles, save_ang = child->s.angles;
		//	let the child move
		P_RunEntity (child, true);
		//	find out how much the childs angles moved
		new_ang = child->s.angles - save_ang;

		//	add the amount of movement to angle_delta
		child->angle_delta = child->angle_delta + new_ang;
		//	add angle_delta to childs angles (== parent's angles + angle_delta)
		child->s.angles = child->s.angles + child->angle_delta;
	}

	gi.linkentity (child);
}

///////////////////////////////////////////////////////////////////////////////
//	hr_Relative
//
//	movement and rotation of child is based on it's position relative to parent
//	when it is first initialized as a child
///////////////////////////////////////////////////////////////////////////////

void	hr_Relative (edict_t *child, childInfo_t *cinfo)
{
	edict_t		*parent = cinfo->parent;
	CVector		ang_delta, angles, forward;
	
	ang_delta = parent->s.angles - cinfo->ang_delta;

	hr_vectopyr (cinfo->dir, angles);
	angles = ang_delta + angles;

	angles.AngleToForwardVector(forward);
	VectorMA( parent->s.origin, forward, cinfo->dist, child->s.origin );

	gi.linkentity (child);
}


//////////////////////////////////////////////////////////////////////////////
//	hr_VecToAngles
//
//	returns the yaw and pitch equivalent of the passed vector
//////////////////////////////////////////////////////////////////////////////

void hr_VecToAngles (CVector &vec, CVector &angles)
{
	float	forward;
	float	yaw, pitch;
	
	if ( vec.y == 0 && vec.x == 0)
	{
		yaw = 0;
	
		if (vec.z > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec.y, vec.x) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec.x * vec.x + vec.y * vec.y);
		pitch = (int) (atan2(vec.z, forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles.x = -pitch;
	angles.y = yaw;
	angles.z = 0;
}
/*
void	hr_VecToAngles (CVector &ec1, CVector &vec2)
{
	float	fw;

	if (vec1 [1] == 0.0F && vec1 [0] == 0.0F)
	{
		vec2 [1] = 0.0F;
		
		if (vec1 [2] > 0.0F)
			vec2 [0] = 90.0F;
		else
			vec2 [0] = 270.0F;
	}
	else
	{
		vec2 [1] = (atan2(vec1 [1], vec1 [0]) * 180.0F / M_PI);

		if (vec2 [1] < 0.0F)
			vec2 [1] += 360.0F;

		// ask Pythagorus
		fw = sqrt (vec1 [0] * vec1 [0] + vec1 [1] * vec1 [1]);
		vec2 [0] = (atan2(vec1 [2], fw) * 180.0F / M_PI);

		if (vec2 [0] < 0.0F)
			vec2 [0] += 360.0F;
	}

	vec2 [2] = 0.0F;
}
*/
///////////////////////////////////////////////////////////////////////////////
//	hr_TriNormal
//
//	gets the normal of the plane defined by three verts
///////////////////////////////////////////////////////////////////////////////

void	hr_TriNormal (CVector &v1, CVector &v2, CVector &v3, CVector &out)
{
	CVector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	CrossProduct (a, b, out);
	out.Normalize();
}

///////////////////////////////////////////////////////////////////////////////
//	hr_TriCenter
//
//	returns the center (averaged) of a triangle
///////////////////////////////////////////////////////////////////////////////

void	hr_TriCenter (CVector &v1, CVector &v2, CVector &v3, CVector &v_out)
{
	v_out.x = (v1.x + v2.x + v3.x) / 3;
	v_out.y = (v1.y + v2.y + v3.y) / 3;
	v_out.z = (v1.z + v2.z + v3.z) / 3;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_Surface
//
//	new, one triangle way
///////////////////////////////////////////////////////////////////////////////

void	hr_Surface (edict_t *child, childInfo_t *cinfo)
{
	userEntity_t	*parent;
	CVector			f_norm, r_norm, u_norm;
	CVector			f_ang, r_ang, u_ang, t_ang;
	CVector			t_vert [3];
	CVector			nr_vert [3];

	parent = cinfo->parent;

	//////////////////////////////////////////////////////////////
	//	get transformed surface points for parent and child
	//////////////////////////////////////////////////////////////

	//	FIXME: change to VertInfo, only need origin_vertex now
	gi.TriVerts (parent, cinfo->surfIndex, 0, t_vert [0], t_vert [1], t_vert [2]);

	//////////////////////////////////////////////////////////////
	//	calculate forward and right normals without model rotation
	//////////////////////////////////////////////////////////////

	t_ang = parent->s.angles;
	parent->s.angles.Zero();

	gi.TriVerts (parent, cinfo->surfIndex, 0, nr_vert [0], nr_vert [1], nr_vert [2]);

	//	get forward normal
	f_norm = nr_vert[cinfo->forward_vertex] - nr_vert [cinfo->origin_vertex];
	f_norm.Normalize();

	//	get right vector
	r_norm = nr_vert[cinfo->right_vertex] - nr_vert[cinfo->origin_vertex];
	r_norm.Normalize();

	if (cinfo->flags & HRF_FLIPFORWARD)
	{
		f_norm = f_norm * -1.0;
	}
	if (cinfo->flags & HRF_FLIPRIGHT)
	{
		r_norm = r_norm * -1.0f;
	}

	parent->s.angles = t_ang;

	hr_VecToAngles (f_norm, f_ang);
	hr_VecToAngles (r_norm, r_ang);

	//////////////////////////////////////////////////////////////
	//	calculate up vector based on forward and right
	//////////////////////////////////////////////////////////////

	//	get up vector based on forward and right
	CrossProduct (f_norm, r_norm, u_norm);
	u_norm = u_norm * -1.0;
	u_norm.Normalize();

	hr_VecToAngles (u_norm, u_ang);
/*
	if (!stricmp (child->className, "vt_weapon"))
	{
		gi.conprintf ("u_ang [0] = %.2f u_ang [1] = %.2f\n", u_ang [0], u_ang [1]);
		gi.conprintf ("f_ang [0] = %.2f f_ang [1] = %.2f\n", f_ang [0], f_ang [1]);
	}
*/
	if ( (u_ang.x <= -270.0 || u_ang.x == 0.0) )
	{
		f_ang.y = u_ang.y;
	}
/*
	else if (u_ang [0] < 0 && u_ang [1] > 180)
	{
		if (!stricmp (child->className, "vt_weapon"))
			gi.conprintf ("adjusting\n");
		//f_ang [0] = 270 - (f_ang [0] - 270);
		f_ang [0] = anglemod (u_ang [0] - 90.0);
		f_ang [1] = u_ang [1] + 90;
	}
*/
	f_ang = f_ang + parent->s.angles;

	//	FIXME: too much error in the roll direction

	if ( f_ang.z > 15 )
	{
		f_ang.z = -r_ang.x + parent->s.angles.z;
	}
	else
	{
		f_ang.z = 0;
	}

/*
	if (f_ang [2] > 15)
		f_ang [2] = -r_ang [0] + parent->angles [2];
	else
		f_ang [2] = 0;
*/

	//////////////////////////////////////////////////////////////
	//	move child's origin to location of parent's vert 0
	//////////////////////////////////////////////////////////////

	child->s.origin = t_vert[cinfo->origin_vertex];

	if (hr_shownormals && hr_shownormals->value && !stricmp (child->className, "vt_weapon"))
	{
/*
		gi.TriVerts (parent, cinfo->surfIndex, 0, nr_vert [0], nr_vert [1], nr_vert [2]);

		//	get forward normal
		f_norm = nr_vert[cinfo->forward_vertex] - nr_vert[cinfo->origin_vertex];
		f_norm.Normalize();

		//	get right vector
		r_norm = nr_vert[cinfo->right_vertex] - nr_vert[cinfo->origin_vertex];
		r_norm.Normalize();

		CrossProduct (f_norm, r_norm, u_norm);
		u_norm = u_norm * -1.0;
		u_norm.Normalize();
*/
		hr_ShowNormal (&forward_normal, child->s.origin, f_norm);
		hr_ShowNormal (&right_normal, child->s.origin, r_norm);
		hr_ShowNormal (&up_normal, child->s.origin, u_norm);
	}

	//////////////////////////////////////////////////////////////
	//	check for pitch/yaw flip-flop and correct it
	//////////////////////////////////////////////////////////////
/*
	if (!stricmp (child->className, "vt_torso"))
	{
		gi.conprintf ("f_ang = %.2f %.2f %.2f\n", f_ang [0], f_ang [1], f_ang [2]);
		gi.conprintf ("ang_d = %.2f %.2f %.2f\n", child->angle_delta [0], child->angle_delta [1], child->angle_delta [2]);
	}
*/
	if ( !(cinfo->flags & HRF_NOPITCH) )
	{
		child->s.angles.x = f_ang.x + child->angle_delta.x;
	}

	if ( !(cinfo->flags & HRF_NOYAW) )
	{
		child->s.angles.y = f_ang.y + child->angle_delta.y;
	}

	if ( !(cinfo->flags & HRF_NOROLL) )
	{
		child->s.angles.z = f_ang.z + child->angle_delta.z;
	}

	gi.linkentity (child);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	hr_Move (edict_t *parent)
{
	childList_t	*clist;
	childInfo_t	*cinfo;

	if (!parent->child_list)
	{
		return;
	}

	for (clist = parent->child_list; clist; clist = clist->next)
	{
		cinfo = clist->info;

		if (cinfo->flags & HRF_CLIENT_ONLY)
		{
			//	just move this entity to the origin of its parent 
			//	to insure that they stay in the same PVS.  The 
			//	client will do the final placement of the entity.
			clist->child->s.origin = parent->s.origin;
			gi.linkentity (clist->child);
		}
		else
		{
			//	call movement function for this entity
			if (cinfo->func)
			{
				cinfo->func (clist->child, cinfo);
			}
			else
			{
				gi.Error ("hr_Move:  Unknown hierarchical type.\n");
			}
		}

		//	if this child is also a parent, then move all of its children now
		if (clist->child->flags & FL_HPARENT)
		{
			hr_Move (clist->child);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	hr_CategorizeVertices
//
//	determines which vertex is the origin, which is the forward and which
//	is the right for single-surface hierarchical models
//
//	this is done by finding the longest side of the triangle, which must
//	be the hypotenuse.  The triangle MUST be a right triangle (ie. one angle
//	equals 90 degrees).
//
//	The vertex opposite the hypotenuse is the origin vertex where the model
//	will be centered.
//
//	The forward vector of the triangle is always considered to be the second
//	longest side, and the right vector is considered to be the shortest side
//	of the triangle.  
//
//	Therefore, for this to work correctly, the reference surface must be a 
//	right triangle with the "forward" pointing side longer than the "right"
//	pointing side, as shown below.
//
//			right side
//				\
//				 \
//				*---------------* <---- origin vertex, where attached child's
//				 \              |		origin will be placed
//				  \             |
//				   \			|
//					\			|  <--- forward side, longer than right side,
//	hypotenuse---->	 \			|		but shorter than the hypotenuse.  
//					  \			|		Child's forward will be oriented along
//					   \		|		this vector.
//						\		|
//						 \		|
//						  \		|
//						   \	|
//							\	|
//							 \	|
//							  \	|
//							   \*
///////////////////////////////////////////////////////////////////////////////

void	hr_CategorizeVertices (edict_t *parent, childInfo_t *cinfo, int surfIndex, int vertIndex)
{
	CVector			v0, v1, v2, temp;
	float			length0to1, length1to2, length2to0;

	//	transform the verts so we can determine which ones are which
	gi.TriVerts (parent, cinfo->surfIndex, 0, v0, v1, v2);

	temp = v0 - v1;
	length0to1 = temp.Length();
	
	temp = v1 - v2;
	length1to2 = temp.Length();

	temp = v2 - v0;
	length2to0 = temp.Length();

	///////////////////////////////////////////////////////////////////////////
	//	longest vertex must be hypotenuse
	///////////////////////////////////////////////////////////////////////////
	
	if (length0to1 > length1to2 && length0to1 > length2to0)
	{
		//	v0 - v1 is hypotenuse, v2 must be origin vertex
		cinfo->origin_vertex = 2;

		if (length1to2 > length2to0)
		{
			//	v1 - v2 is longer than v2 - v0, v1 must be forward vertex
			cinfo->forward_vertex = 1;
			cinfo->right_vertex = 0;
		}
		else
		{
			cinfo->forward_vertex = 0;
			cinfo->right_vertex = 1;
		}
	}
	else if (length1to2 > length2to0)
	{
		cinfo->origin_vertex = 0;

		if (length2to0 > length0to1)
		{
			cinfo->forward_vertex = 2;
			cinfo->right_vertex = 1;
		}
		else
		{
			cinfo->forward_vertex = 1;
			cinfo->right_vertex = 2;
		}
	}
	else
	{
		cinfo->origin_vertex = 1;

		if (length0to1 > length1to2)
		{
			cinfo->forward_vertex = 0;
			cinfo->right_vertex = 2;
		}
		else
		{
			cinfo->forward_vertex = 2;
			cinfo->right_vertex = 0;
		}
	}
}

void	hr_InitSurface (edict_t *parent, edict_t *child, char *f_surf, int f_vIndex, unsigned long flags)
{
	childList_t		*clist;
	childInfo_t		*cinfo;

	clist = hr_AddChild (parent, child);
	
	cinfo = (childInfo_t *) gi.X_Malloc(sizeof (childInfo_t), MEM_TAG_LEVEL);
	clist->info = cinfo;
	
	cinfo->type = HR_SURFACE;
	cinfo->flags = flags | HRF_CLIENT_ONLY;		//	force server to let client do placement!
	cinfo->parent = parent;

	child->owner = parent;	//	do not clip against owner

	//////////////////////////////////////////////////////////////
	//	find the tris on the parent that we should lock to
	//////////////////////////////////////////////////////////////

	cinfo->offset.Zero();

/*
	cinfo->surfIndex = gi.SurfIndex (parent->s.modelindex, f_surf);

	if (cinfo->surfIndex < 0)
		gi.Error ("hr_Init: surface %s doesn't exist in %s.", f_surf, parent->modelName);

	//	never get a surface index here, because we can't be guaranteed that
	//	the model has been loaded, and the server shouldn't really be accessing
	//	that info, anyway.

	//	find the origin, right and forward vertices
	hr_CategorizeVertices (parent, cinfo, cinfo->surfIndex, f_vIndex);

	//	set up movement function 
	cinfo->func = hr_Surface;
*/
	//	get the index for this surface name and send it to the client
	cinfo->surfIndex = hr_IndexForSurfaceName (f_surf);

	//	set up for sending to client
	hr_InitInfo (parent, child, HR_SURFACE, cinfo->surfIndex, f_vIndex, flags);
}

///////////////////////////////////////////////////////////////////////////////
//	hr_InitRelative
//
//	init for simple, non-surface hierarchical models
///////////////////////////////////////////////////////////////////////////////

void	hr_InitRelative (userEntity_t *parent, userEntity_t *child, unsigned long int flags)
{
	childInfo_t		*cinfo;
	childList_t		*clist;

	clist = hr_AddChild (parent, child);

	cinfo = (childInfo_t *) gi.X_Malloc (sizeof (childInfo_t), MEM_TAG_LEVEL);
	clist->info = cinfo;	

	cinfo->type = HR_RELATIVE;
	cinfo->flags = flags;         // | HRF_CLIENT_ONLY;
	cinfo->parent = parent;

	cinfo->dir = child->s.origin - parent->s.origin;

	cinfo->dist = cinfo->dir.Normalize();

	//	Nelno:	ang_delta now holds initial angles of parent
	cinfo->ang_delta = parent->s.angles;
//	cinfo->ang_delta = child->s.angles - parent->s.angles;

	//	make sure we don't clip on parent or children
	if (child->solid != SOLID_BSP && parent->solid != SOLID_BSP)
		child->owner = parent;

	//	set up movement function 
	cinfo->func = hr_Relative;

	//	set up for sending to client
//	hr_InitInfo (parent, child, HR_RELATIVE, 0, 0, flags);
}

///////////////////////////////////////////////////////////////////////////////
//	hr_InitBmodel
//
//	init for bmodels
///////////////////////////////////////////////////////////////////////////////

void	hr_InitBmodel (userEntity_t *parent, userEntity_t *child, unsigned long int flags)
{
	CVector			temp;
	childInfo_t		*cinfo;
	childList_t		*clist;

	clist = hr_AddChild (parent, child);

	cinfo = (childInfo_t *) gi.X_Malloc (sizeof (childInfo_t), MEM_TAG_LEVEL);
	clist->info = cinfo;	

	cinfo->type = HR_BMODEL;
	cinfo->flags = flags | HRF_CLIENT_ONLY;
	cinfo->parent = parent;

	//	Store the others angles for future reference
	cinfo->last_angle = parent->s.angles;

	//	Store the other's origin for future reference in the movement function
	cinfo->last_origin = parent->s.origin;

	//	Store the 3 hyp's lines for 3-D space calc's
	cinfo->hyp_xy = sqrt (((parent->s.origin [0] - child->s.origin [0]) * 
						(parent->s.origin [0] - child->s.origin [0])) + 
						((parent->s.origin [1] - child->s.origin [1]) * 
						(parent->s.origin [1] - child->s.origin [1])));
	cinfo->hyp_yz = sqrt (((parent->s.origin [1] - child->s.origin [1]) * 
						(parent->s.origin [1] - child->s.origin [1])) + 
						((parent->s.origin [2] - child->s.origin [2]) * 
						(parent->s.origin [2] - child->s.origin [2])));
	cinfo->hyp_xz = sqrt (((parent->s.origin [0] - child->s.origin [0]) * 
						(parent->s.origin [0] - child->s.origin [0])) + 
						((parent->s.origin [2] - child->s.origin [2]) * 
						(parent->s.origin [2] - child->s.origin [2])));

	//	Store the Angle Delta for each axis
	temp = parent->s.origin - child->s.origin;

	//	store the distance from parent to child (VectorNormalize normalizes AND returns the magnitude)
	cinfo->dist = temp.Normalize();

	hr_vectopyr (temp, cinfo->ang_delta);

	cinfo->ang_delta = parent->s.angles - cinfo->ang_delta;

	cinfo->ang_delta.x = anglemod (cinfo->ang_delta.x);
	cinfo->ang_delta.y = anglemod (cinfo->ang_delta.y);
	cinfo->ang_delta.z = anglemod (cinfo->ang_delta.z);

	//	make sure we don't clip on parent or children
	child->owner = parent;

	//	set up movement function 
	cinfo->func = hr_Bmodel;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_SetFlags
//
//	sets the flags for a hierarchical child
///////////////////////////////////////////////////////////////////////////////

void	hr_SetFlags (edict_t *parent, edict_t *child, unsigned long flags)
{
	childInfo_t		*cinfo;
	childList_t		*clist;

	clist = hrl_FindChildInList (parent, child);

	if (!clist)
		//	FIXME:	should this error out instead?
		return;

	cinfo = clist->info;
	cinfo->flags = flags;
}

///////////////////////////////////////////////////////////////////////////////
//	hr_GetFlags
//
//	sets the flags for a hierarchical child
///////////////////////////////////////////////////////////////////////////////

unsigned long	hr_GetFlags (edict_t *parent, edict_t *child)
{
	childInfo_t		*cinfo;
	childList_t		*clist;

	clist = hrl_FindChildInList (parent, child);

	if (!clist)
		//	FIXME:	should this error out instead?
		return	0;

	cinfo = clist->info;
	return	cinfo->flags;
}