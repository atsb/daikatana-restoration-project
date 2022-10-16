//////////////////////////////////////////////////////////////////////////////
// dk_std.h
// Windows standard library extensions for non-windows platforms.
//////////////////////////////////////////////////////////////////////////////

#ifndef _DK_STD_H
#define _DK_STD_H

#if !_MSC_VER

#include "limits.h"

#define _MAX_FNAME	PATH_MAX
#define _MAX_PATH	PATH_MAX
#define _MAX_EXT	PATH_MAX

int _stricmp (const char *s, const char *t);
#define stricmp _stricmp

int _strnicmp (const char *s, const char *t, int n);
#define strnicmp _strnicmp

void _splitpath (char *path, char *dev, char *dir, char *name, char *ext);
#define splitpath _splitpath

template <class T> T __min (T a, T b)
{
	if (a < b)
		return a;
	return b;
}

template <class T> T __max (T a, T b)
{
	if (a > b)
		return a;
	return b;
}

#endif // !_MSC_VER

#endif // _DK_STD_H
