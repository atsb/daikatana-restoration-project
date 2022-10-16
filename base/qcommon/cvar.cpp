// added for Japanese version
#ifdef JPN
#include<windows.h>
#endif // JPN

// cvar.c -- dynamic variable tracking
#include "qcommon.h"
cvar_t	*cvar_vars = NULL;

//short CL_LoadLastConfiguration(void);

/*
============
Cvar_InfoValidate
============
*/
static qboolean Cvar_InfoValidate (const char *s)
{
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
	const char *in = s;
	while (*in)	{
		if( *in == '\\' || *in == '\;' || *in == '\"')		
			return false;
		if(IsDBCSLeadByte(*in))		
			in++;
		in++;
	}
#else	// JPN
	if (strstr (s, "\\"))
		return false;
	if (strstr (s, "\""))
		return false;
	if (strstr (s, ";"))
		return false;
#endif	// JPN
// Encompass MarkMa 040599
	return true;
}

/*
============
Cvar_FindVar
============
*/
static cvar_t *Cvar_FindVar (const char *var_name)
{
	cvar_t	*var;
	
	for (var=cvar_vars ; var ; var=var->next)
	{
		if( var->name )
		{
			if (!strcmp (var_name, var->name))
			{
				return var;
			}
		}
	}
	return NULL;
}
/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (const char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}
/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (const char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->string;
}
/*
============
Cvar_CompleteVariable
============
*/
char *Cvar_CompleteVariable (const char *partial)
{
	cvar_t		*cvar;
	int			len;
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
	// check exact match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!strcmp (partial,cvar->name))
			return cvar->name;
	// check partial match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!strncmp (partial,cvar->name, len))
			return cvar->name;
	return NULL;
}
/*
============
Cvar_Get
If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *Cvar_Get (const char *var_name, const char *var_value, int flags)
{
	cvar_t	*var;
	
	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_name))
		{
			Com_Printf("invalid info cvar name\n");
			return NULL;
		}
	}

	var = Cvar_FindVar (var_name);
	if (var)
	{
		var->flags |= flags;
		return var;
	}
	if (!var_value)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (var_value))
		{
			Com_Printf("invalid info cvar value\n");
			return NULL;
		}
	}

//	var = (cvar_t *)X_Malloc (sizeof(*var),MEM_TAG_CVAR);
//	var->name = CopyString (var_name);
//	var->string = CopyString (var_value);
	var = (cvar_t *)malloc(sizeof(cvar_t));
	if( var == NULL )
	{
		return NULL;
	}

	var->name = strdup(var_name);
	var->string = strdup(var_value);
	var->latched_string = NULL;
	var->modified = true;
	var->value = atof (var->string);
	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;
	var->flags = flags;
	return var;
}
/*
============
Cvar_Set2
============
*/
cvar_t *Cvar_Set2 (const char *var_name, const char *value, qboolean force)
{
	cvar_t	*var;

	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		return Cvar_Get (var_name, value, 0);
	}
	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate (value))
		{
			Com_Printf("invalid info cvar value\n");
			return var;
		}
	}

	if (!force)
	{
		if (var->flags & CVAR_NOSET)
		{
			Com_Printf ("%s is write protected.\n", var_name);
			return var;
		}
		if (var->flags & CVAR_LATCH)
		{
			if (var->latched_string)
			{
				if (strcmp(value, var->latched_string) == 0)
					return var;
//				X_Free (var->latched_string);
				free(var->latched_string);
				var->latched_string = NULL;
			}
			else
			{
				if (strcmp(value, var->string) == 0)
					return var;
			}

			if (Com_ServerState())
			{
				Com_Printf ("%s will be changed for next game.\n", var_name);
//				var->latched_string = CopyString(value);
				var->latched_string = strdup(value);
			}
			else
			{
//				var->string = CopyString(value);
				var->string = strdup(value);
				var->value = atof (var->string);
				if (!strcmp(var->name, "game"))
				{
					FS_SetGamedir (var->string);
					FS_ExecAutoexec ();
					//cek[12-7-99] execute the user's config, too
					//CL_LoadLastConfiguration();
				}
			}
			return var;
		}
	}
	else
	{
		if (var->latched_string)
		{
//			X_Free (var->latched_string);
			free(var->latched_string);
			var->latched_string = NULL;
		}
	}
	if (!strcmp(value, var->string))
		return var;		// not changed
	var->modified = true;
	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true;	// transmit at next oportunity
	
//	X_Free (var->string);	// free the old value string
	free(var->string);	// free the old value string
	
//	var->string = CopyString(value);
	if( strlen( value ) == 2 )
	{
		int foo = 1;
	}
	var->string = strdup(value);
	var->value = atof (var->string);
	return var;
}
/*
============
Cvar_ForceSet
============
*/
cvar_t *Cvar_ForceSet (const char *var_name, const char *value)
{
	return Cvar_Set2 (var_name, value, true);
}
/*
============
Cvar_Set
============
*/
cvar_t *Cvar_Set (const char *var_name, const char *value)
{
	return Cvar_Set2 (var_name, value, false);
}
/*
============
Cvar_FullSet
============
*/
cvar_t *Cvar_FullSet (const char *var_name, const char *value, int flags)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
	{	// create it
		return Cvar_Get (var_name, value, flags);
	}
	var->modified = true;
	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true;	// transmit at next oportunity
	
//	X_Free (var->string);	// free the old value string
	free(var->string);	// free the old value string
	
//	var->string = CopyString(value);
	var->string = strdup(value);
	var->value = atof (var->string);
	var->flags = flags;
	return var;
}
/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (const char *var_name, float value)
{
	char	val[32];

	if (value == (int)value)
		Com_sprintf (val, sizeof(val), "%i",(int)value);
	else
		Com_sprintf (val, sizeof(val), "%f",value);
	Cvar_Set (var_name, val);
}
/*
============
Cvar_GetLatchedVars
Any variables with latched values will now be updated
============
*/
void Cvar_GetLatchedVars (void)
{
	cvar_t	*var;
	for (var = cvar_vars ; var ; var = var->next)
	{
		if( !( var->flags & CVAR_LATCH ) )
		{
			continue;
		}

		if (!var->latched_string )
		{
			continue;
		}
//		X_Free (var->string);
		free(var->string);
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = atof(var->string);
		if (!strcmp(var->name, "game"))
		{
			FS_SetGamedir (var->string);
			FS_ExecAutoexec ();
			//cek[12-7-99] execute the user's config, too
			//CL_LoadLastConfiguration();
		}
	}
}
/*
============
Cvar_Command
Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
	cvar_t			*v;
// check variables
	v = Cvar_FindVar (GetArgv(0));
	if (!v)
		return false;
		
// perform a variable print or set
	if (GetArgc() == 1)
	{
		Com_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}
	Cvar_Set (v->name, GetArgv(1));
	return true;
}
/*
============
Cvar_Set_f
Allows setting and defining of arbitrary cvars from console
============
*/
void Cvar_Set_f (void)
{
	int		c;
	int		flags;
	
	c = GetArgc();
	if (c != 3 && c != 4)
	{
		Com_Printf ("usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4)
	{
		if (!strcmp(GetArgv(3), "u"))
			flags = CVAR_USERINFO;
		else if (!strcmp(GetArgv(3), "s"))
			flags = CVAR_SERVERINFO;
		else
		{
			Com_Printf ("flags can only be 'u' or 's'\n");
			return;
		}
		Cvar_FullSet (GetArgv(1), GetArgv(2), flags);
	}
	else
		Cvar_Set (GetArgv(1), GetArgv(2));
}
/*
============
Cvar_WriteVariables
Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (char *path)
{
	cvar_t	*var;
	char	buffer[1024];
	FILE	*f;
	f = fopen (path, "a");
	if( f == NULL )
	{
		return;
	}
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			Com_sprintf (buffer, sizeof(buffer), "set %s \"%s\"\n", var->name, var->string);
			fprintf (f, "%s", buffer);
		}
	}
	fclose (f);
}
/*
============
Cvar_List_f
============
*/
void Cvar_List_f (void)
{
	cvar_t	*var;
	int		i;
	i = 0;
	for (var = cvar_vars ; var ; var = var->next, i++)
	{
		if (var->flags & CVAR_ARCHIVE)
			Com_Printf ("*");
		else
			Com_Printf (" ");
		if (var->flags & CVAR_USERINFO)
			Com_Printf ("U");
		else
			Com_Printf (" ");
		if (var->flags & CVAR_SERVERINFO)
			Com_Printf ("S");
		else
			Com_Printf (" ");
		if (var->flags & CVAR_NOSET)
			Com_Printf ("-");
		else if (var->flags & CVAR_LATCH)
			Com_Printf ("L");
		else
			Com_Printf (" ");
		Com_Printf (" %s \"%s\"\n", var->name, var->string);
	}
	Com_Printf ("%i cvars\n", i);
}
qboolean userinfo_modified;
char	*Cvar_BitInfo (int bit)
{
	static char	info[MAX_INFO_STRING];
	cvar_t	*var;
	info[0] = 0;
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (var->flags & bit)
			Info_SetValueForKey (info, var->name, var->string);
	}
	return info;
}
// returns an info string containing all the CVAR_USERINFO cvars
char	*Cvar_Userinfo (void)
{
	return Cvar_BitInfo (CVAR_USERINFO);
}
// returns an info string containing all the CVAR_SERVERINFO cvars
char	*Cvar_Serverinfo (void)
{
	return Cvar_BitInfo (CVAR_SERVERINFO);
}
/*
============
Cvar_Init
Reads in all archived cvars
============
*/
void Cvar_Init (void)
{
	Cmd_AddCommand ("set", Cvar_Set_f);
	Cmd_AddCommand ("cvarlist", Cvar_List_f);
}
