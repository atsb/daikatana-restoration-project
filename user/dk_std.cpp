//////////////////////////////////////////////////////////////////////////////
// dk_std.cpp
// Windows standard library extensions for non-windows platforms.
//////////////////////////////////////////////////////////////////////////////

#if !_MSC_VER

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dk_std.h"

int _stricmp (const char *s, const char *t)
{
	int i;

	for (i = 0; tolower(s[i]) == tolower(t[i]); i++)
		if (s[i] == '\0')
			return 0;

	return s[i] - t[i];
}

int _strnicmp (const char *s, const char *t, int n)
{
	int i;

	if (n <= 0)
		return 0;

	for (i = 0; tolower(s[i]) == tolower(t[i]); i++)
		if ((s[i] == '\0') || (i == n-1))
			return 0;

	return s[i] - t[i];
}

void _splitpath (char *path, char *dev, char *dir, char *name, char *ext)
{
	char *c;
	char *tmp;

	if (!path)
		return;

	if (dev)
		*dev = '\0';

	if (dir)
	{
		c = path;
		while ((tmp = strchr (c, '\\')) != NULL)
			c = tmp + 1;
		while ((tmp = strchr (c, '/')) != NULL)
			c = tmp + 1;
		strncpy (dir, path, c - path);
		dir[c-path] = '\0';
	}

	if (name)
	{
		c = path;
		while ((tmp = strchr (c, '\\')) != NULL)
			c = tmp + 1;
		while ((tmp = strchr (c, '/')) != NULL)
			c = tmp + 1;
		strcpy (name, c);
	}

	if (ext)
	{
		c = path;
		while ((tmp = strchr (c, '.')) != NULL)
			c = tmp + 1;
		strcpy (ext, c - 1);
	}
}

#endif // !_MSC_VER
