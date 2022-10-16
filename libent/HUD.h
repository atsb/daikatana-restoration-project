// HUD.h: interface for the CHUD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUD_H__A38E4E83_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_)
#define AFX_HUD_H__A38E4E83_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ClientEntity.h"
#include "HUDChar.h"	// Added by ClassView
#include "HUDGraphic.h"	// Added by ClassView

#define HUD_STRINGS 24
#define HUD_STRINGWIDTH 80
class CHUD : public CClientEntity  
{
public:
	void * default_font;
	virtual CClientEntity * Alloc();
	CHUDGraphic Graphics[64];
	CHUDChar Text[HUD_STRINGS][HUD_STRINGWIDTH];
	void CL_RunFrame();
	void PrintXY(int x, int y, void * font_name, int flags, char * fmt, ...);
	CHUD();
	virtual ~CHUD();

protected:
	void DrawText();
};

#endif // !defined(AFX_HUD_H__A38E4E83_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_)
