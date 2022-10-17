#ifndef _bodylist_H
#define	_bodylist_H


//#define DllExport	extern "C"	__declspec( dllexport )

void	bodylist_init (userEntity_t *player);
void	bodylist_add (userEntity_t *player, float fadeTime = -1);

#endif