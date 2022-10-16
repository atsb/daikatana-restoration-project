// HUDChar.h: interface for the CHUDChar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUDCHAR_H__A38E4E84_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_)
#define AFX_HUDCHAR_H__A38E4E84_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CHUDChar  
{
public:
	// class CVector color_rgb;
	char character;
	void * font;
	int y;
	int x;
	CHUDChar();
	virtual ~CHUDChar();

};

#endif // !defined(AFX_HUDCHAR_H__A38E4E84_BC00_11D2_A7E2_0000C087A6E9__INCLUDED_)
