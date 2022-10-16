#ifdef JPN

#define IME_IN_COMPOSITION      1
#define IME_IN_CHOSECAND        2
#define IME_ON			        4

#define CheckProperty		{				\
    if ( ( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) ||	\
	 !( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) )	\
	goto call_defwinproc;					\
}

typedef struct _IMEUIDATA {
    int        ImeState;   // Current Ime state.
    UINT       uCompLen;   // To save previous composition string length.
    DWORD      fdwProperty;
} IMEUIDATA;

// Prototype declaration
void ImeUIComposition( HWND, WPARAM, LPARAM );
void GetCompositionStr( HWND, LPARAM );
void ImeUIEndComposition();
void ImeUISetOpenStatus( HWND );
void DisplayCompString( HWND, LPSTR, LPSTR );
BOOL ImeUINotify( HWND, WPARAM, LPARAM );

// Frankz
void ImeInit(HANDLE hwnd);

void ImeSetMode(BOOL mode);


// Global data
extern IMEUIDATA gImeUIData;
#endif