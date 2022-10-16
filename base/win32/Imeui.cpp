#ifdef JPN
#include <assert.h>
#include <windows.h>
#include <imm.h>
#include "imefull.h" //4-6-99

#include	"client.h"
#include	"winquake.h"
#include	"daikatana.h"
#include	"dk_beams.h"

#include	"dk_menu.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_menup.h"

/************************************************************************
* Global data
************************************************************************/
IMEUIDATA gImeUIData;               // IMEUI's global data.

extern CInterfaceInputField *CurrentlyEditingField();
extern void ConSaveIme(LPSTR lpCompStr); // DK_CONSOLE
extern void ConAddChat();
extern void ConSetIme(char *lpStr);

extern HWND        cl_hwnd;
extern int lastKey;

/*************************
* ImeInit()
**************************/
void ImeInit(HANDLE hwnd)
{
	// IME initial state.
	gImeUIData.ImeState = 0;
    gImeUIData.fdwProperty = ImmGetProperty( GetKeyboardLayout(0),
					     IGP_PROPERTY );
}

/*************************
* ImeUIComposition()
**************************/
void ImeUIComposition( HWND hwnd, WPARAM wParam, LPARAM CompFlag )
{
    if ( CompFlag & GCS_RESULTSTR )
	{
		// Clear IME Composition string
//#ifdef CONSOLE
		if (cls.key_dest == key_console)
			ConSaveIme(NULL); // DK_CONSOLE
		else
		{
			CInterfaceInputField *edit;
			edit = CurrentlyEditingField();
			if (edit)
				edit->SetImeText(NULL);

		}
/*#else
		CInterfaceInputField *edit;
		edit = CurrentlyEditingField();
		if (edit)
			edit->SetImeText(NULL);
#endif*/
	}
    else
		if ( CompFlag & GCS_COMPSTR )
			 GetCompositionStr( hwnd, CompFlag );
}

/*************************
* GetCompositionStr()
**************************/
void GetCompositionStr( HWND hwnd, LPARAM CompFlag )
{
	 DWORD      dwBufLen;               // Stogare for len. of composition str
	 LPSTR      lpCompStr;              // Pointer to composition str.
	 HIMC       hIMC;                   // Input context handle.
	 HLOCAL     hMem;                   // Memory handle.

	 if ( !( hIMC = ImmGetContext( hwnd ) ) )
		 return;

	 if ( ( dwBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR, 
										 (void FAR*)NULL, 0l ) ) < 0 )
		 goto exit2;

	 if ( !( hMem = LocalAlloc( LPTR, (int)dwBufLen + 1 ) ) )
		 goto exit2;

	 if ( !( lpCompStr = (LPSTR) LocalLock( hMem ) ) )
		 goto exit1;

	 // Reads in the composition string.
	 ImmGetCompositionString( hIMC, GCS_COMPSTR, lpCompStr, dwBufLen );

	 // Null terminated.
	 lpCompStr[ dwBufLen ] = 0;

	 // Display new composition chars.
//#ifdef CONSOLE

 	if(lpCompStr != NULL)	{
		for(int i=0; i<32 && lpCompStr[i]!=0; i++ )	{
			if( IsDBCSLeadByte(*(lpCompStr+i)) )	++i;
		}
		*(lpCompStr+i) = '\0';
	}


	if (cls.key_dest == key_console)
		ConSaveIme(lpCompStr); // DK_CONSOLE
	else // DK_MENU
	{
		CInterfaceInputField *edit;
		edit = CurrentlyEditingField();
		if (edit)
			edit->SetImeText(lpCompStr);
		else
			ConSetIme(lpCompStr);
	}
/*#else
	// DK_MENU
	CInterfaceInputField *edit;
	edit = CurrentlyEditingField();
	if (edit)
		edit->SetImeText(lpCompStr);
#endif*/

	// Keep the length of the composition string for using later.
	 gImeUIData.uCompLen = (UINT)dwBufLen;
	 LocalUnlock( hMem );

	exit1:
	 LocalFree( hMem );

	exit2:
	 ImmReleaseContext( hwnd, hIMC );
}


/*************************
* ImeUIEndComposition()
**************************/
void ImeUIEndComposition()
{
	// Reset the length of composition string to zero.
	gImeUIData.uCompLen = 0;
	gImeUIData.ImeState &= ~IME_IN_COMPOSITION;

	// Clear composition string
//#ifdef CONSOLE
	if ((cls.key_dest == key_console))
		ConSaveIme(NULL); // DK_CONSOLE
	else // DK_MENU
	{
		CInterfaceInputField *edit;
		edit = CurrentlyEditingField();
		if (edit)
			edit->SetImeText(NULL);
		else
		{
			if(lastKey != 0x000e0001)
				ConAddChat();
			ConSetIme(NULL);
		
		}

	}
/*#else
	CInterfaceInputField *edit;
	edit = CurrentlyEditingField();
	if (edit)
		edit->SetImeText(NULL);
#endif*/
}

/*************************
* ImeUINotify)
**************************/
BOOL ImeUINotify( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    switch (wParam )
    {
        case IMN_SETOPENSTATUS:
            ImeUISetOpenStatus( hwnd );
            break;

		default:	    return FALSE;
    }
    return TRUE;
}

/*************************
* ImeUISetOpenStatus()
**************************/
void ImeUISetOpenStatus( HWND hwnd )
{
    HIMC        hIMC;    // Storage for input context handle.

#ifndef JPN // added by yokoyama for Not IME BUILD
ImeSetMode(FALSE);
return;
#endif // JPN

    // If fail to get input context handle then do nothing
    if ( !( hIMC = ImmGetContext( hwnd ) ) )
        return;

    if ( ImmGetOpenStatus( hIMC ) )
	{	
		// IME is actived
//#ifdef CONSOLE
		if (CurrentlyEditingField() || cls.key_dest == key_console || cls.key_dest == key_message)
/*#else
		if (CurrentlyEditingField())
#endif*/
		{	//set IME_ON
			gImeUIData.ImeState |= IME_ON;
		}
		else
			// disable IME
			ImmSetOpenStatus( hIMC, false);
	}
	else
    {
        // Reset IMEUI's global data. 
        gImeUIData.uCompLen = 0;
        gImeUIData.ImeState = 0;
	}
    ImmReleaseContext( hwnd, hIMC );
}

void ImeSetMode(BOOL mode)
{
	HIMC hIMC;

    if ( !( hIMC = ImmGetContext( cl_hwnd ) ) )
        return;

	ImmSetOpenStatus(hIMC, mode);

	ImmReleaseContext(cl_hwnd, hIMC);
}


#endif