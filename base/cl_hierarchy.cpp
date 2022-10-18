///////////////////////////////////////////////////////////////////////////////
//	cl_hierarchy.c
//
//	Nelno,	7/9/98
//
//	each entity has a pointer to a list of its children, self->child_list
//		if self->child_list == NULL, entity has no children
//		otherwise, self->child_list points to a cl_childList_t struct
//
//	TODO:	clean out old fields in cl_childInfo_t
//	TODO:	look at vectopyr
///////////////////////////////////////////////////////////////////////////////

#include	<crtdbg.h>

#include	"client.h"
#include	"qcommon.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

cvar_t	*hr_testvert;
cvar_t	*hr_shownormals;

int			clhr_NumSurfNames;
char		clhr_SurfNames [MAX_SURFACE_NAMES][32];
char		clhr_SurfFlags [MAX_SURFACE_NAMES];		//	flags for each surface name 
													//	visible/invisible, etc.

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

/* ---------------------------                ------------------------------ */
/* ++++++++++++++++++++++++++ utility functions ++++++++++++++++++++++++++++ */
/* ---------------------------                ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//	clhr_AddSurfaceName
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_AddSurfaceName  (char *line, int char_count, int is_comment)
{
	int		i;
	char	*comma;
	char	name [128], flags [128];

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
		strcpy (clhr_SurfNames [clhr_NumSurfNames], name);

		//	set flags for this surface name
		clhr_SurfFlags [clhr_NumSurfNames] = atoi (flags);

		clhr_NumSurfNames++;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_ReadSurfaceNames
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_ReadSurfaceNames (void)
{
	FILE	*file;
	int		file_start, file_len;
	char	line [256];
	char	ch;
	int		char_count, is_comment;

	file_len = FS_FOpenFile ("models/hr_names.txt", &file);
	if (file_len < 0)
		Sys_Error ("ERROR: could not find models/hr_names.txt!\n");

	file_start = FS_Tell(file);
	FS_Seek (file, file_start, SEEK_SET);

	char_count = clhr_NumSurfNames = 0;
	is_comment = false;
	memset (line, 0x00, sizeof (line));

	while (file_len > 0)
	{
		ch = FS_Getc (file);
		if (char_count == 0 && ch == '/')
			is_comment = true;

		if (ch != '\r' && ch != '\n')
		{
			//	don't add CR/LF to end
			line [char_count] = ch;
			char_count++;
		}

		if (ch == '\n')
		{
			clhr_AddSurfaceName (line, char_count, is_comment);

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
		clhr_AddSurfaceName (line, char_count, is_comment);
	}

	FS_FCloseFile (file);
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_FlagsForSurfaceIndex
//
///////////////////////////////////////////////////////////////////////////////

int	clhr_FlagsForSurfaceIndex (int index)
{
	if (index < clhr_NumSurfNames)
		return	clhr_SurfFlags [index];
	else
		return	-1;
}


///////////////////////////////////////////////////////////////////////////////
//	clhr_FlagsForSurfaceName
//
///////////////////////////////////////////////////////////////////////////////

int	clhr_FlagsForSurfaceName (char *surfName)
{
	int		i;

	for (i = 0; i < clhr_NumSurfNames; i++)
	{
		if (!stricmp (clhr_SurfNames [i], surfName))
			return	clhr_SurfFlags [i];
	}

	return	-1;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_IndexForSurfaceName
//
///////////////////////////////////////////////////////////////////////////////

int	clhr_IndexForSurfaceName (char *surfName)
{
	int		i;

	for (i = 0; i < clhr_NumSurfNames; i++)
	{
		if (!stricmp (clhr_SurfNames [i], surfName))
			return	i;
	}

	return	-1;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_SurfaceNameForIndex
//
///////////////////////////////////////////////////////////////////////////////

char	*clhr_SurfaceNameForIndex (int index)
{
	if (index >= clhr_NumSurfNames || index < 0)
		Sys_Error ("clhr_IndexForSurfaceName: bad surface index.\n");

	return	clhr_SurfNames [index];
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_VertInfo
//
//	returns the transformed location of a specified vertex in a specified
//	surface for the specified frame of the specified entity, got that?
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
///////////////////////////////////////////////////////////////////////////////

int	clhr_VertInfo (centity_t *ent, int surfIndex, int vertIndex, CVector&vert)
{
	void	*mod;

	mod = cl.model_draw [ent->current.modelindex];

	if(!mod)
		return 0;

	return re.VertInfo (mod, surfIndex, ent->current.frame, vertIndex, 
		ent->lerp_origin, ent->current.old_origin, ent->current.angles, 
		vert);
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_SurfIndex
//
//	returns an index for the passed surface name
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
///////////////////////////////////////////////////////////////////////////////

int	clhr_SurfIndex (int modelIndex, char *surfName)
{
	void	*mod;

	mod = cl.model_draw [modelIndex];

	if(!mod)
		return 0;

	return	re.SurfIndex (mod, surfName);
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_TriVerts
//
//	returns three transformed vertices
//
//	FIXME!!!
//	RELIES ON THE SERVER HAVING ACCESS TO LOCAL CLIENT DATA
//	NEED TO MAKE SURE THIS DATA IS AVAILABLE IN DEDICATED SERVERS, TOO
//////////////////////////////////////////////////////////////////////

int	clhr_TriVerts (centity_t *ent, int surfIndex, int vertIndex, CVector &v1, CVector &v2, CVector &v3)
{
	void	*mod;

	mod = cl.model_draw [ent->current.modelindex];

	if(!mod)
		return 0;

	return	re.TriVerts (mod, surfIndex, ent->current.frame, 
		vertIndex, ent->lerp_origin, ent->current.old_origin, 
		ent->current.angles, v1, v2, v3);
}


//////////////////////////////////////////////////////////////////////////////
//	clhr_VecToAngles
//
//	returns the yaw and pitch equivalent of the passed vector
//////////////////////////////////////////////////////////////////////////////

void clhr_VecToAngles ( CVector &vec, CVector &angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (vec [1] == 0 && vec [0] == 0)
	{
		yaw = 0;
	
		if (vec [2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec [1], vec [0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec [0] * vec [0] + vec [1] * vec [1]);
		pitch = (int) (atan2(vec [2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles.x = -pitch;
	angles.y = yaw;
	angles.z = 0;
}

///////////////////////////////////////////////////////////////////////////////
// clhr_vectoypr
//
//	huh???  FIXME!!  This doesn't make any sense...  a single vector cannot
//	indicate roll
///////////////////////////////////////////////////////////////////////////////

void	clhr_vectopyr ( CVector &vec1, CVector &vec2)
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

cl_childList_t	*hrl_CreateChildList (centity_t *parent, centity_t *child)
{
	if (parent->child_list != NULL)
		Sys_Error ("hrl_CreateChildList: child_list != NULL\n");

	parent->child_list = (cl_childList_t *)X_Malloc (sizeof (cl_childList_t), MEM_TAG_CLIENT);
	
	((cl_childList_t *) parent->child_list)->next = NULL;
	((cl_childList_t *) parent->child_list)->child = child;

	//	set hparent flag on this parent
	parent->flags |= CFL_HPARENT;
	//	set hchild flag on this child
	child->flags |= CFL_HCHILD;

	return	(cl_childList_t *) parent->child_list;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_Normalthink
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_NormalThink (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_ShowNormal
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_ShowNormal (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_AddChildToList
//
//
///////////////////////////////////////////////////////////////////////////////

cl_childList_t	*hrl_AddChildToList (centity_t *parent, centity_t *child)
{
	cl_childList_t	*newlist;

	if (parent->child_list == NULL)
		Sys_Error ("hrl_AddChildToList: Tried to add to NULL child_list.\n");

	//	add to the front of the list
	newlist = (cl_childList_t *)X_Malloc (sizeof (cl_childList_t), MEM_TAG_CLIENT);
	newlist->next = parent->child_list;
	newlist->child = child;

	parent->child_list = newlist;

	//	set hchild flag on this child
	child->flags |= CFL_HCHILD;

	return	newlist;
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_RemoveChildFromList
//
//	removes child from parent's child_list
///////////////////////////////////////////////////////////////////////////////

void	hrl_RemoveChildFromList (centity_t *parent, centity_t *child, int free_child)
{
	cl_childList_t	*clist, *prev;
	int			can_free;

	if (parent->child_list == NULL)
		Sys_Error ("hrl_RemoveChildFromList: Tried to remove from a NULL child_list.\n");

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
//		gi.error ("hrl_RemoveChildFromList: Child is not in child_list.\n");

	if (!prev)
	{
		//	first child in the list
		(cl_childList_t *) parent->child_list = clist->next;

		//	if no more children in parent's list, clear parent flag
		if (!parent->child_list)
			parent->flags -= (parent->flags & CFL_HPARENT);
	}
	else
		prev->next = clist->next;

	//	determine if this child can be freed by this function
	//	if HRF_NOREMOVE is set this function cannot remove the child entity
	can_free = !(clist->info->flags & HRF_NOREMOVE);

	//	free the info structure if it exists
	if (clist->info)
		X_Free(clist->info);

	//	free the list item for this child
	X_Free (clist);

	//	clear child flag
	child->flags &= ~CFL_HCHILD;

	//	FIXME:	how do we free a client entity???
	//	free the child if desired and HRF_NOREMOVE is not set
//	if (free_child && can_free)
//		P_FreeEdict (child);
}

///////////////////////////////////////////////////////////////////////////////
//	hrl_FindChildInList
//
//	returns a cl_childList_t * for child or NULL if child is not in the parent's
//	list
///////////////////////////////////////////////////////////////////////////////

cl_childList_t	*hrl_FindChildInList (centity_t *parent, centity_t *child)
{
	cl_childList_t	*clist;

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

void	hrl_DeleteList (centity_t *parent, int free_children)
{
	if (!parent->child_list)
		return;

	while (parent->child_list)
		hrl_RemoveChildFromList (parent, ((cl_childList_t *) parent->child_list)->child, free_children);
}

/* ---------------------------                ------------------------------ */
/* ++++++++++++++++++++++++ hierarchy functions ++++++++++++++++++++++++++++ */
/* ---------------------------                ------------------------------ */

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

cl_childList_t	*clhr_AddChild (centity_t *parent, centity_t *child)
{
	if (parent->child_list == NULL)
		return	hrl_CreateChildList (parent, child);
	else
		return	hrl_AddChildToList (parent, child);
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_Bmodel
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_Bmodel (const centity_t *child, const cl_childInfo_t *cinfo)
{
	Com_Printf ("clhr_BModel:  HR_BMODEL type not implemented.\n");
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_Relative
//
//	movement and rotation of child is based on it's position relative to parent
//	when it is first initialized as a child
///////////////////////////////////////////////////////////////////////////////

void	clhr_Relative (const centity_t *child, const cl_childInfo_t *cinfo)
{
	centity_t		*parent = cinfo->parent;
	CVector			ang_delta, angles, dir, forward;
	entity_t		*parent_render_ent;
	entity_t		*child_render_ent;
	
	parent_render_ent = parent->render_ent;
	child_render_ent = child->render_ent;

	ang_delta = parent_render_ent->angles - cinfo->ang_delta;

	dir = cinfo->dir;
	clhr_vectopyr (dir, angles);
	angles = ang_delta + angles;

	AngleToVectors (angles, forward);
	VectorMA (parent_render_ent->origin, forward, cinfo->dist, child_render_ent->origin);
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_TriNormal
//
//	gets the normal of the plane defined by three verts
///////////////////////////////////////////////////////////////////////////////

void	clhr_TriNormal ( CVector &v1, CVector &v2, CVector &v3, CVector &out)
{
	CVector	a, b;

	a = v3 - v1;
	b = v2 - v1;
	CrossProduct (a, b, out);
	out.Normalize();
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_TriCenter
//
//	returns the center (averaged) of a triangle
///////////////////////////////////////////////////////////////////////////////

void	clhr_TriCenter ( CVector &v1, CVector &v2, CVector &v3, CVector &v_out)
{
	v_out.x = (v1 [0] + v2 [0] + v3 [0]) / 3;
	v_out.y = (v1 [1] + v2 [1] + v3 [1]) / 3;
	v_out.z = (v1 [2] + v2 [2] + v3 [2]) / 3;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_Surface
//
//	takes a centity_t pointer to the child, which in turn points to the 
//	render entity for the child.  The and angles origin are changed directly 
//	in the render entitiy before the renderer draws it
///////////////////////////////////////////////////////////////////////////////

void	clhr_Surface (const centity_t *child, const cl_childInfo_t *cinfo)
{
	centity_t		*parent;
	CVector			f_norm, r_norm, u_norm;
	CVector			f_ang, r_ang, u_ang, t_ang;
	CVector			t_vert [3];
	CVector			nr_vert [3];
	entity_t		*parent_render_ent;
	entity_t		*child_render_ent;

	parent = cinfo->parent;
	parent_render_ent = parent->render_ent;
	child_render_ent = child->render_ent;

	//////////////////////////////////////////////////////////////////////////////
	// amw 11.25.98
	// on hierarchical models.. all 4 pieces are initialized.. the legs, the torso,
	// the head and the weapon.. however, the weapon, which is a child, may not 
	// have a model associated with it.  handle that here.  however, a parent
	// should always have a render entity.. make sure it does..
	//////////////////////////////////////////////////////////////////////////////
	if (child_render_ent == NULL)
	{
		return;
	}
	
	_ASSERTE (parent_render_ent != NULL);

	//////////////////////////////////////////////////////////////
	//	get transformed surface points for parent and child
	//////////////////////////////////////////////////////////////

	//	FIXME: change to VertInfo, only need origin_vertex now
//	clhr_TriVerts (parent, cinfo->surfIndex, 0, t_vert [0], t_vert [1], t_vert [2]);
/*
	re.TriVerts (parent_render_ent->model, cinfo->surfIndex,
		parent_render_ent->frame, 0, parent_render_ent->origin, 
		parent_render_ent->oldorigin, parent_render_ent->angles, 
		t_vert [0], t_vert [1], t_vert [2]);
*/
	re.TriVerts_Lerp (parent_render_ent->model, cinfo->surfIndex,
		parent_render_ent->frame, 0, parent_render_ent->origin, 
		parent_render_ent->oldorigin, parent_render_ent->angles, 
		t_vert [0], t_vert [1], t_vert [2], parent_render_ent->oldframe, parent_render_ent->backlerp);


	//////////////////////////////////////////////////////////////
	//	calculate forward and right normals without model rotation
	//////////////////////////////////////////////////////////////

	t_ang = parent_render_ent->angles;
	parent_render_ent->angles.Zero();

	//clhr_TriVerts (parent, cinfo->surfIndex, 0, nr_vert [0], nr_vert [1], nr_vert [2]);
/*
	re.TriVerts (parent_render_ent->model, cinfo->surfIndex,
		parent_render_ent->frame, 0, parent_render_ent->origin, 
		parent_render_ent->oldorigin, parent_render_ent->angles, 
		nr_vert [0], nr_vert [1], nr_vert [2]);
*/
	re.TriVerts_Lerp (parent_render_ent->model, cinfo->surfIndex,
		parent_render_ent->frame, 0, parent_render_ent->origin, 
		parent_render_ent->oldorigin, parent_render_ent->angles, 
		nr_vert [0], nr_vert [1], nr_vert [2], parent_render_ent->oldframe, parent_render_ent->backlerp);

	//	get forward normal
	f_norm = nr_vert [cinfo->forward_vertex] - nr_vert [cinfo->origin_vertex];
	f_norm.Normalize();

	//	get right vector
	r_norm = nr_vert [cinfo->right_vertex] - nr_vert [cinfo->origin_vertex];
	r_norm.Normalize();

	if (cinfo->flags & HRF_FLIPFORWARD)
	{
		f_norm = f_norm * -1.0;
	}
	if (cinfo->flags & HRF_FLIPRIGHT)
	{
		r_norm = r_norm * -1.0;
	}

	parent_render_ent->angles = t_ang;

	clhr_VecToAngles (f_norm, f_ang);
	clhr_VecToAngles (r_norm, r_ang);

	//////////////////////////////////////////////////////////////
	//	calculate up vector based on forward and right
	//////////////////////////////////////////////////////////////

	//	get up vector based on forward and right
	CrossProduct (f_norm, r_norm, u_norm);
	u_norm = u_norm * -1.0;
	u_norm.Normalize();

	clhr_VecToAngles (u_norm, u_ang);

	if ((u_ang [0] <= -270.0 || u_ang [0] == 0.0))
	{
		f_ang.y = u_ang.y;
	}
	f_ang = f_ang + parent_render_ent->angles;

	//	FIXME: too much error in the roll direction

	if ( f_ang.z > 15 )
	{
		f_ang.z = -r_ang[0] + parent_render_ent->angles[2];
	}
	else
	{
		f_ang.z = 0;
	}

	//////////////////////////////////////////////////////////////
	//	move child's origin to location of parent's vert 0
	//////////////////////////////////////////////////////////////

	child_render_ent->origin = t_vert[cinfo->origin_vertex];
	child_render_ent->oldorigin = t_vert[cinfo->origin_vertex];

	//////////////////////////////////////////////////////////////
	//	check for pitch/yaw flip-flop and correct it
	//////////////////////////////////////////////////////////////

//	if ((f_ang [0] > 80.0f && f_ang [0] < 100.0f) || (f_ang [0] < -80.0f && f_ang [0] > -100.0f))
//		DebugBreak ();

	if (!(cinfo->flags & HRF_NOPITCH))
	{
		child_render_ent->angles.x = f_ang [0] + child->lerp_angles [0];
	}

	if (!(cinfo->flags & HRF_NOYAW))
	{
		child_render_ent->angles.y = f_ang [1] + child->lerp_angles [1];
	}

	if (!(cinfo->flags & HRF_NOROLL))	
	{
		child_render_ent->angles.z = f_ang [2] + child->lerp_angles [2];
	}
}

///////////////////////////////////////////////////////////////////////////////
//	FIXME BIGTIME
///////////////////////////////////////////////////////////////////////////////

void	clhr_MoveChildren (centity_t *parent)
{
	cl_childList_t	*clist;
	cl_childInfo_t	*cinfo;

	if (!parent->child_list)
		return;

	for (clist = parent->child_list; clist; clist = clist->next)
	{
		cinfo = clist->info;

		//	call movement function for this model
		if (cinfo->func)
			cinfo->func (clist->child, cinfo);
		else
			Sys_Error ("clhr_Move:  Unknown hierarchical type.\n");

		//	if this child is also a parent, then move all of its children now
		if (clist->child->flags & CFL_HPARENT)
			clhr_MoveChildren (clist->child);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_Move (frame_t *frame)//centity_t *parent)
{
	cl_childList_t	*clist;
	cl_childInfo_t	*cinfo;
	centity_t		*parent;
	int				i;

	for (i = 0; i < MAX_EDICTS; i++)
	{
		parent = &cl_entities [i];
		
		if (!(parent->flags & CFL_HPARENT) || !parent->child_list)
			continue;

		for (clist = parent->child_list; clist; clist = clist->next)
		{
			cinfo = clist->info;

			//	call movement function for this model
			if (cinfo->func)
				cinfo->func (clist->child, cinfo);
			else
				Sys_Error ("clhr_Move:  Unknown hierarchical type.\n");

			//	if this child is also a parent, then move all of its children now
			if (clist->child->flags & CFL_HPARENT)
				clhr_MoveChildren (clist->child);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_CategorizeVertices
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

void	clhr_CategorizeVertices (centity_t *parent, cl_childInfo_t *cinfo, int surfIndex, int vertIndex)
{
	CVector			v0, v1, v2, temp;
	float			length0to1, length1to2, length2to0;

	//	transform the verts so we can determine which ones are which
	clhr_TriVerts (parent, cinfo->surfIndex, 0, v0, v1, v2);

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

///////////////////////////////////////////////////////////////////////////////
//	clhr_InitSurface
//
///////////////////////////////////////////////////////////////////////////////

void	clhr_InitSurface (centity_t *parent, centity_t *child, int surfIndex, int vertIndex, unsigned short flags)
{
	cl_childList_t		*clist;
	cl_childInfo_t		*cinfo;
	char				*surfName;
	void				*mod;

	clist = clhr_AddChild (parent, child);
	
	cinfo = (cl_childInfo_t *)X_Malloc (sizeof (cl_childInfo_t), MEM_TAG_CLIENT);
	clist->info = cinfo;
	
	cinfo->type = HR_SURFACE;
	cinfo->flags = flags;
	cinfo->parent = parent;

	if (cinfo->surfIndex < 0)
		Sys_Error ("clhr_Init: surface %s doesn't exist in model.", surfIndex);

	//	find the surface name from hr_SurfNames array
	surfName = clhr_SurfaceNameForIndex (surfIndex);

	mod = cl.model_draw [parent->current.modelindex];
	cinfo->surfIndex = re.SurfIndex (mod, surfName);

	//	find the origin, right and forward vertices
	clhr_CategorizeVertices (parent, cinfo, cinfo->surfIndex, vertIndex);

	//	set up movement function 
	cinfo->func = clhr_Surface;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_InitRelative
//
//	init for simple, non-surface hierarchical models
///////////////////////////////////////////////////////////////////////////////

void	clhr_InitRelative (centity_t *parent, centity_t *child, unsigned long int flags)
{
	cl_childInfo_t		*cinfo;
	cl_childList_t		*clist;

	clist = clhr_AddChild (parent, child);

	cinfo = (cl_childInfo_t *)X_Malloc (sizeof (cl_childInfo_t), MEM_TAG_CLIENT);
	clist->info = cinfo;	

	cinfo->type = HR_RELATIVE;
	cinfo->flags = flags;
	cinfo->parent = parent;

	
	cinfo->dir = child->lerp_origin - parent->lerp_origin;

	cinfo->dist = cinfo->dir.Normalize();

	//	Nelno:	ang_delta now holds initial angles of parent
	cinfo->ang_delta = parent->current.angles;

	//	set up movement function 
	cinfo->func = clhr_Relative;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_InitBmodel
//
//	init for bmodels
///////////////////////////////////////////////////////////////////////////////

void	clhr_InitBmodel (centity_t *parent, centity_t *child, unsigned long int flags)
{
	CVector			temp;
	cl_childInfo_t		*cinfo;
	cl_childList_t		*clist;

	clist = clhr_AddChild (parent, child);

	cinfo = (cl_childInfo_t *)X_Malloc (sizeof (cl_childInfo_t), MEM_TAG_CLIENT);
	clist->info = cinfo;	

	cinfo->type = HR_BMODEL;
	cinfo->flags = flags;
	cinfo->parent = parent;

	//	Store the others angles for future reference
	cinfo->last_angle = parent->current.angles;

	//	Store the other's origin for future reference in the movement function
	cinfo->last_origin = parent->lerp_origin;

	//	Store the 3 hyp's lines for 3-D space calc's
	cinfo->hyp_xy = sqrt (((parent->lerp_origin [0] - child->lerp_origin [0]) * 
						(parent->lerp_origin [0] - child->lerp_origin [0])) + 
						((parent->lerp_origin [1] - child->lerp_origin [1]) * 
						(parent->lerp_origin [1] - child->lerp_origin [1])));
	cinfo->hyp_yz = sqrt (((parent->lerp_origin [1] - child->lerp_origin [1]) * 
						(parent->lerp_origin [1] - child->lerp_origin [1])) + 
						((parent->lerp_origin [2] - child->lerp_origin [2]) * 
						(parent->lerp_origin [2] - child->lerp_origin [2])));
	cinfo->hyp_xz = sqrt (((parent->lerp_origin [0] - child->lerp_origin [0]) * 
						(parent->lerp_origin [0] - child->lerp_origin [0])) + 
						((parent->lerp_origin [2] - child->lerp_origin [2]) * 
						(parent->lerp_origin [2] - child->lerp_origin [2])));

	//	Store the Angle Delta for each axis
	temp = parent->lerp_origin - child->lerp_origin;

	//	store the distance from parent to child (.Normalize() normalizes AND returns the magnitude)
	cinfo->dist = temp.Normalize();

	clhr_vectopyr (temp, cinfo->ang_delta);

	cinfo->ang_delta = parent->current.angles - cinfo->ang_delta;

	cinfo->ang_delta.x = anglemod (cinfo->ang_delta[0]);
	cinfo->ang_delta.y = anglemod (cinfo->ang_delta[1]);
	cinfo->ang_delta.z = anglemod (cinfo->ang_delta[2]);

	//	set up movement function 
	cinfo->func = clhr_Bmodel;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_SetFlags
//
//	sets the flags for a hierarchical child
///////////////////////////////////////////////////////////////////////////////

void	clhr_SetFlags (centity_t *parent, centity_t *child, unsigned long flags)
{
	cl_childInfo_t		*cinfo;
	cl_childList_t		*clist;

	clist = hrl_FindChildInList (parent, child);

	if (!clist)
		//	FIXME:	should this error out instead?
		return;

	cinfo = clist->info;
	cinfo->flags = flags;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_GetFlags
//
//	sets the flags for a hierarchical child
///////////////////////////////////////////////////////////////////////////////

unsigned long	clhr_GetFlags (centity_t *parent, centity_t *child)
{
	cl_childInfo_t		*cinfo;
	cl_childList_t		*clist;

	clist = hrl_FindChildInList (parent, child);

	if (!clist)
		//	FIXME:	should this error out instead?
		return	0;

	cinfo = clist->info;
	return	cinfo->flags;
}

///////////////////////////////////////////////////////////////////////////////
//	clhr_RemoveChildFromLists
//
//	removes child from all lists of hierarchical children
///////////////////////////////////////////////////////////////////////////////

void	clhr_RemoveChildFromLists (centity_t *child)
{
	int		i;

	for (i = 0; i < MAX_EDICTS; i++)
	{
		if (cl_entities [i].flags & CFL_HPARENT)
			//	this is a hierarchical parent, so try and remove this child
			//	from its child_list.  If it's not there, it will just be ignored
			hrl_RemoveChildFromList (&cl_entities [i], child, false);
	}
}


///////////////////////////////////////////////////////////////////////////////
//	clhr_Initialize 
//
//	initializes a client entity to be a hierarchical model
///////////////////////////////////////////////////////////////////////////////

void	clhr_Initialize (centity_t *child, entity_state_t *state)
{
	centity_t	*parent = NULL;

	//	find the entity that has a number matching parentNumber
/*
	for (i = 0; i < MAX_EDICTS; i++)
	{
		if (cl_entities [i].current.number == state->hrInfo.parentNumber)
			parent = &cl_entities [i];
	}
*/
	parent = &cl_entities [state->hrInfo.parentNumber];

	if (parent == NULL)
	{
		Sys_Error ("clhr_Initialize: Could not find parent.\n");
		return;
	}

	///////////////////////////////////////////////////////////////////////////
	//	remove child from any previous hierarchical lists
	///////////////////////////////////////////////////////////////////////////

	clhr_RemoveChildFromLists (child);

	if (state->hrInfo.hrFlags & HRF_REMOVE)
	{
		//	entity was removed, so now that it's removed from all lists just exit
		if (state->hrInfo.hrType == HR_SURFACE)
		{
			//	so that CL_ParseDelta can reliably tell when an HR_SURFACE type has had it's
			//	model changed, we have to zero out the prev and baseline hierarchical data
			//	so that when a removed HR_SURFACE entity gets re-used it isn't confused into
			//	thinking it's still an HR_SURFACE model that just had it's model index changed
			//	the .current state is already zeroed out whenever any type of hierarchical model
			//	is removed.
			memset (&child->prev.hrInfo, 0x00, sizeof (hrInfo_t));
			memset (&child->baseline.hrInfo, 0x00, sizeof (hrInfo_t));
		}
		memset (&state->hrInfo, 0x00, sizeof (hrInfo_t));

		if (!(state->hrInfo.hrFlags & HRF_RESET))
			return;
	}

	switch (state->hrInfo.hrType)
	{
		case	HR_SURFACE:
				clhr_InitSurface (parent, child, state->hrInfo.surfaceIndex, 
					state->hrInfo.vertexIndex, state->hrInfo.hrFlags);
				break;
		case	HR_BMODEL:
		case	HR_RELATIVE:
				break;
	}
}
                      
///////////////////////////////////////////////////////////////////////////////
//	clhr_InitForFrame
//
//	initializes all hierarchical models for the current frame
///////////////////////////////////////////////////////////////////////////////

void	clhr_InitForFrame (frame_t *frame)
{
	int		i;

	for (i = 0; i < MAX_EDICTS; i++)
	{
		if (cl_entities [i].current.hrInfo.hrFlags & HRF_RESET)
		{
			cl_entities [i].current.hrInfo.hrFlags -= HRF_RESET;

			clhr_Initialize (&cl_entities [i], &cl_entities [i].current);
		}
	}
}