// added for Japanese version
#ifdef JPN
#include<windows.h>
#endif // JPN

#include	<io.h>
#include "client.h"

#include "dk_menu.h"
#include "dk_gce_main.h"
#include "l__language.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256

#ifdef JPN
void ImeSetMode2(BOOL mode);
extern HWND        cl_hwnd;
#endif // JPN

char	key_lines[32][MAXCMDLINE];
int		key_linepos;
int		shift_down=false;
qboolean	anykeydown;

int		edit_line=0;
int		history_line=0;

int		key_waiting;
char	*keybindings[256];
qboolean	consolekeys[256];	// if true, can't be rebound while in console
qboolean	menubound[256];	// if true, can't be rebound while in menu
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
qboolean	keydown[256];

int Key_IsDown(int key) {
    if (key < 0 || key >= 256) return 0;
    return keydown[key];
}   

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

void CompleteCommand (void)
{
	char	*cmd, *s;

	s = key_lines[edit_line]+1;
	if (*s == '\\' || *s == '/')
		s++;

	cmd = Cmd_CompleteCommand (s);
	if (!cmd)
		cmd = Cvar_CompleteVariable (s);
	if (cmd)
	{
		key_lines[edit_line][1] = '/';
		strcpy (key_lines[edit_line]+2, cmd);
		key_linepos = strlen(cmd)+2;
		key_lines[edit_line][key_linepos] = ' ';
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
		return;
	}
}

#ifdef JPN
//#ifdef CONSOLE
// Japanese console
void ImeKey_Console (int leadbyte, int trailbyte)
{
	if (leadbyte && key_linepos < MAXCMDLINE-2) // Kanji
	{
			key_lines[edit_line][key_linepos] = leadbyte;
			key_linepos++;
			key_lines[edit_line][key_linepos] = trailbyte;
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
	}
	else if (key_linepos < MAXCMDLINE-1) // Hankaku
	{
		key_lines[edit_line][key_linepos] = trailbyte;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}
}

//#endif
#endif // JPN

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console (int key)
{
	switch ( key )
	{
		case K_KP_SLASH:
			key = '/';
			break;
		case K_KP_MINUS:
			key = '-';
			break;
		case K_KP_PLUS:
			key = '+';
			break;
/*
		case K_KP_HOME:
			key = '7';
			break;
		case K_KP_UPARROW:
			key = '8';
			break;
		case K_KP_PGUP:
			key = '9';
			break;
		case K_KP_LEFTARROW:
			key = '4';
			break;
		case K_KP_5:
			key = '5';
			break;
		case K_KP_RIGHTARROW:
			key = '6';
			break;
		case K_KP_END:
			key = '1';
			break;
		case K_KP_DOWNARROW:
			key = '2';
			break;
		case K_KP_PGDN:
			key = '3';
			break;
		case K_KP_INS:
			key = '0';
			break;
*/
		case K_KP_DEL:
			key = '.';
			break;
	}

	if ( ( toupper( key ) == 'V' && keydown[K_CTRL] ) ||
		 ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keydown[K_SHIFT] ) )
	{
		char *cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 )
		{
			int i;

			strtok( cbd, "\n\r\b" );

			i = strlen( cbd );
			if ( i + key_linepos >= MAXCMDLINE)
				i= MAXCMDLINE - key_linepos;

			if ( i > 0 )
			{
				cbd[i]=0;
				strcat( key_lines[edit_line], cbd );
				key_linepos += i;
			}
			free( cbd );
		}

		return;
	}

	if( key == 'c' )
	{
		if ( keydown[K_CTRL] )
		{
			Cbuf_AddText ("disconnect\n");
			return;
		}
	}

	if ( key == 'l' ) 
	{
	
		if ( keydown[K_CTRL] )
		{
			Cbuf_AddText ("clear\n");
			return;
		}
	}

	if ( key == K_ENTER || key == K_KP_ENTER )
	{	// backslash text are commands, else chat
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText (key_lines[edit_line]+2);	// skip the >
		else
			Cbuf_AddText (key_lines[edit_line]+1);	// valid command
		Cbuf_AddText ("\n");
		Com_Printf ("%s\n",key_lines[edit_line]);
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = '>';
		key_linepos = 1;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
									// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		CompleteCommand ();
		return;
	}
	
	if ( ( key == K_BACKSPACE ) || ( key == K_LEFTARROW ) || ( key == K_KP_LEFTARROW ) || ( ( key == 'h' ) && ( keydown[K_CTRL] ) ) )
	{
#ifdef JPN
//#ifdef CONSOLE
		char *text;
		text = key_lines[edit_line];
		if (key_linepos > 1)
		{
			int j,prv;
			prv=j=1;
			while ( j < key_linepos)
			{
				prv = j;
				if (IsDBCSLeadByte(text[j]))
					j++;
				j++;
			}
			if (IsDBCSLeadByte(text[prv]))
			{
				//delete the lead-byte
				key_linepos--;
				text[key_linepos] = ' '; // Fixed bug 1498
			}
			// delete the trail-byte
		}
//#endif
#endif // JPN
		if (key_linepos > 1)
			key_linepos--;
		return;
	}

	if ( ( key == K_UPARROW ) || ( key == K_KP_UPARROW ) ||
		 ( ( key == 'p' ) && keydown[K_CTRL] ) )
	{
		do
		{
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line
				&& !key_lines[history_line][1]);
		if (history_line == edit_line)
			history_line = (edit_line+1)&31;
		strcpy(key_lines[edit_line], key_lines[history_line]);
		key_linepos = strlen(key_lines[edit_line]);
		return;
	}

	if ( ( key == K_DOWNARROW ) || ( key == K_KP_DOWNARROW ) ||
		 ( ( key == 'n' ) && keydown[K_CTRL] ) )
	{
		if (history_line == edit_line) return;
		do
		{
			history_line = (history_line + 1) & 31;
		}
		while (history_line != edit_line
			&& !key_lines[history_line][1]);
		if (history_line == edit_line)
		{
		//	Nelno:	changed console edit line character from ]
			key_lines[edit_line][0] = '>';
			key_linepos = 1;
		}
		else
		{
			strcpy(key_lines[edit_line], key_lines[history_line]);
			key_linepos = strlen(key_lines[edit_line]);
		}
		return;
	}
	if (key == K_PGUP || key == K_KP_PGUP )
	{
		con.display -= 2;
		return;
	}
	if (key == K_PGDN || key == K_KP_PGDN ) 
	{
		con.display += 2;
		if (con.display > con.current)
			con.display = con.current;
		return;
	}
	if (key == K_HOME || key == K_KP_HOME )
	{
		con.display = con.current - con.totallines + 10;
		return;
	}
	if (key == K_END || key == K_KP_END )
	{
		con.display = con.current;
		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable
		
	if (key_linepos < MAXCMDLINE-1)
	{
		key_lines[edit_line][key_linepos] = key;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}
}

//============================================================================

qboolean	chat_team;
char		chat_buffer[MAXCMDLINE];
int			chat_bufferlen = 0;

void Key_Message (int key)
{
	if ( key == K_ENTER || key == K_KP_ENTER )
	{
		if (chat_team)
			Cbuf_AddText ("say_team \"");
		else
			Cbuf_AddText ("say_all \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

#ifdef JPN
		ImeSetMode2(FALSE);
#endif

		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
#ifdef JPN
		ImeSetMode2(FALSE);
#endif
		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
#ifdef JPN
		if (chat_bufferlen)
		{
			if(IsDBCSLeadByte(chat_buffer[chat_bufferlen -2]) && (chat_bufferlen >= 2))
			{
				chat_bufferlen--;
				chat_buffer[chat_bufferlen] = 0;
				chat_bufferlen--;
				chat_buffer[chat_bufferlen] = 0;
			}
			else
			{
				chat_bufferlen--;
				chat_buffer[chat_bufferlen] = 0;
			}
		}
#else
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
#endif
		return;
	}

	if (chat_bufferlen == sizeof(chat_buffer)-1)
		return; // all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;
	
	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

	for (kn=keynames ; kn->name ; kn++)
	{
		if (!Q_strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;	
	static	char	tinystr[2];
	
	if (keynum == -1)
		return "<KEY NOT FOUND>";

	// SCG[2/18/00]: 
	if( keynum < 0 )
		return "<KEY NOT FOUND>";


	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}
	
	for (kn=keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_KeybindingToKeynum

Returns the number(s) of the key(s) bound to the given keybinding
===================
*/
int *Key_KeybindingToKeynum (char *str)
{
	int i;
    static int kn[2];
    int numfound = 0;

    kn[0] = kn[1] = -1;

	if (str)
	{
		for(i=0;i<256;i++)
		{
			if (keybindings[i])
	        {
		        if (!stricmp(str, keybindings[i]))
			        kn[numfound++] = i;
	        }
		}
	}

    return kn;
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, const char *binding)
{
	char	*newbinding;
	int		l;
			
//	if (keynum == -1)
	if (keynum < 0)
		return;

	// SCG[2/18/00]: 
	if( keynum < 0 )
		return;

// free old bindings
	if (keybindings[keynum])
	{
		// mdm99.03.10 - new memmgr
		//Z_Free (keybindings[keynum]);
		X_Free(keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
			
// allocate memory for new binding
	l = strlen (binding);	
	// mdm99.03.10 - new memmgr
	newbinding = (char*)X_Malloc(l+1,MEM_TAG_KEYBIND);
	strcpy (newbinding, binding);
	newbinding[l] = 0;
	keybindings[keynum] = newbinding;	
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (GetArgc() != 2)
	{
		Com_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum (GetArgv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", GetArgv(1));
		return;
	}

	Key_SetBinding (b, "");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_SetBinding (i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];
	
	c = GetArgc();

	if (c < 2)
	{
		Com_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (GetArgv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", GetArgv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Com_Printf ("\"%s\" = \"%s\"\n", GetArgv(1), keybindings[b] );
		else
			Com_Printf ("\"%s\" is not bound\n", GetArgv(1) );
		return;
	}
	
// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		strcat (cmd, GetArgv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i] && keybindings[i][0])
			Com_Printf ("%s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	for (i=0 ; i<32 ; i++)
	{
		key_lines[i][0] = '>';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;
	
//
// init ascii characters in console mode
//
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_KP_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_KP_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_KP_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_KP_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_KP_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_KP_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_KP_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_KP_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_KP_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_KP_INS] = true;
	consolekeys[K_KP_DEL] = true;
	consolekeys[K_KP_SLASH] = true;
	consolekeys[K_KP_PLUS] = true;
	consolekeys[K_KP_MINUS] = true;
	consolekeys[K_KP_5] = true;

	consolekeys['`'] = false;
	consolekeys['~'] = false;

	for (i=0 ; i<256 ; i++)
		keyshift[i] = i;
	for (i='a' ; i<='z' ; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;

	// cek[2-2-00] so f-keys can be bound in the menu
//	for (i=0 ; i<12 ; i++)
//		menubound[K_F1+i] = true;

//
// register our functions
//
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);
	Cmd_AddCommand ("bindlist",Key_Bindlist_f);


}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/

#define IS_CTRL_C(key)		( (key == 'c') && keydown[K_CTRL] )

extern int			      scr_draw_progress;
void Key_Event (int key, bool down, unsigned time)
{
	char	*kb;
	char	cmd[1024];

#ifdef JPN
	if (!key)
		return;
#endif // JPN

	if( CIN_Running() == 1 )
	{
		if( cls.key_dest != key_console )
		{
			//allow the script editor a chance to take the keydown message
			if (down == true && CIN_GCE_KeyEvent(key, -1, (shift_down != 0) ? 1 : 0) == 1) {
				//the script editor ate up this keystroke.
				return;
			}
			//allow the script editor a chance to take the keyup message
			else if (down != true && CIN_GCE_KeyEvent(-1, key, (shift_down != 0) ? 1 : 0) == 1) {
				//the script editor ate up this keystroke.
				return;
			}
		}
		if( ( key != K_ESCAPE ) && ( key != K_PAUSE ) )
		{
			kb = keybindings[key];
			if (kb && kb[0] == '+')
			{
				Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
				Cbuf_AddText (cmd);
			}
			if (keyshift[key] != key)
			{
				kb = keybindings[keyshift[key]];
				if (kb && kb[0] == '+')
				{
					Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
					Cbuf_AddText (cmd);
				}
			}

			return;
		}
	}

//	if( !( ( key == 'c' ) && ( keydown[K_CTRL] ) ) && ( cls.key_dest != key_menu ) )
	if( (key != K_CTRL) && !(IS_CTRL_C(key)) && ( cls.key_dest != key_menu ) )
	{
		if( (con.mode == CON_LOADING) || ( cls.state == ca_connecting ) || (scr_draw_progress) )
		{
			return;
		}
	}

	// hack for modal presses
	if (key_waiting == -1)
	{
		if (down)
			key_waiting = key;
		return;
	}

	// update auto-repeat status
	if (down)
	{
        //true if we allow repeats.
        bool allow_repeats;

        //if we are in the interface or the console, allow all repeats
        //allow repeats if we are editing some field in the script editor.
        if (DKM_InMenu() == true || 
            cls.key_dest == key_console ||
            (cls.key_dest == key_game && cls.state != ca_active) ||
            CIN_GCE_CurrentlyEditing() == 1
           ) 
        {
            allow_repeats = true;
        }
        else {
            allow_repeats = false;
        }

		key_repeats[key]++;
		if (key != K_BACKSPACE 
			&& key != K_PAUSE 
			&& key != K_PGUP 
			&& key != K_KP_PGUP 
			&& key != K_PGDN
			&& key != K_KP_PGDN
			&& (key_repeats[key] > 1 && allow_repeats == false))
        {
			return;	// ignore the repeat
        }
			
		if (key >= 200 && !keybindings[key])
			Com_Printf ("%s is unbound.\n", Key_KeynumToString (key) );
	}
	else
	{
		key_repeats[key] = 0;
	}

	if (key == K_SHIFT)
		shift_down = down;

	// SCG[8/20/99]: Don't send events to the cinescriptor if we are in console mode.
	if( cls.key_dest != key_console )
	{
		//allow the script editor a chance to take the keydown message
		if (down == true && CIN_GCE_KeyEvent(key, -1, (shift_down != 0) ? 1 : 0) == 1) {
			//the script editor ate up this keystroke.
			return;
		}
		//allow the script editor a chance to take the keyup message
		else if (down != true && CIN_GCE_KeyEvent(-1, key, (shift_down != 0) ? 1 : 0) == 1) {
			//the script editor ate up this keystroke.
			return;
		}
	}

	//	Nelno: added quick console
	// console key is hardcoded, so the user can never unbind it
	if (key == '`')
	{
		if (!down)
			return;

		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return;
		}

		//	Nelno:	disable console if in demo mode
#ifdef	LOCKED_DEMO
		Com_Printf ("Demo Mode:  Press F10 to quit.\n");
		return;
#else
		//	Nelno:	disable console if invulnerable.dat is found
		if (_access ("./data/invulnerable.dat", 0) != -1)
		{
			Com_Printf ("Demo Mode:  Press F10 to quit.\n");
			return;
		}

		// SCG[10/2/99]: No console access during cinematics
		if( CIN_Running() == 1 )
		{
			return;
		}

        if (cls.key_dest != key_menu)  // 5.08  dsn  allow tilde to escape from menu
        {
		  if (!shift_down)
			Con_ToggleQuickConsole_f ();
		  else
			Con_ToggleConsole_f ();
		  return;
        }
#endif
	}

	if( key != K_CTRL && !(IS_CTRL_C(key)) && key != K_ESCAPE && ( cls.key_dest != key_menu ) )
	{
		if( (con.mode == CON_LOADING) || ( cls.state == ca_connecting ) || (scr_draw_progress) )
		{
			return;
		}
	}
	if( key == K_PAUSE )
	{
		if( (con.mode == CON_LOADING) || ( cls.state == ca_connecting ) || ( cl.refresh_prepped == 0 ) || (scr_draw_progress) )
		{
			return;
		}
	}
	//	Nelno:	end Quick Console additions

#ifdef	LOCKED_DEMO
	if (key == K_F10)
	{
		Cbuf_AddText ("quit");
		return;
	}
#endif

	//	any key during the attract mode will bring up the menu
	//	Nelno:	unless the console is down
	if (cl.attractloop && cls.key_dest != key_menu && cls.key_dest != key_console)
		key = K_ESCAPE;

	// menu key is hardcoded, so the user can never unbind it
	if (key == K_ESCAPE)
	{
		if (!down)
			return;
		
		if( CIN_Running() == 1 )
		{
			CIN_Stop( NULL );
			CIN_RemoveCinematicEntities();
			return;
		}

		if( (con.mode == CON_LOADING) || ( cls.state == ca_connecting ))
		{
// SCG[2/22/00]: removed per Romero's request.
// SCG[2/22/00]: 			Cbuf_AddText ("disconnect\n");
			return;
		}

		if (cl.frame.playerstate.stats[STAT_LAYOUTS] && cls.key_dest == key_game)
		{	// put away help computer / inventory
			//Cbuf_AddText ("cmd putaway\n");
			//return;

          // shut down scoreboard and continue                 2.26 dsn
          cl.frame.playerstate.stats[STAT_LAYOUTS] = 0; 
		}

		switch (cls.key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			M_Keydown (key);
			break;
		case key_game:
		case key_console:
			M_Menu_Main_f ();
			break;
		default:
			Com_Error (ERR_FATAL, "Bad cls.key_dest");
		}
		return;
	}

	// track if any key is down for BUTTON_ANY
	keydown[key] = down;
	if (down)
	{
		if (key_repeats[key] == 1)
			anykeydown++;
	}
	else
	{
		anykeydown--;
		if (anykeydown < 0)
			anykeydown = 0;
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
			Cbuf_AddText (cmd);
		}
		if (keyshift[key] != key)
		{
			kb = keybindings[keyshift[key]];
			if (kb && kb[0] == '+')
			{
				Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", kb+1, key, time);
				Cbuf_AddText (cmd);
			}
		}

    //if we are in our menu, send the message to it now.
    if (DKM_InMenu() == true)
      DKM_Key(key, false);

    return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if ( (cls.key_dest == key_menu && menubound[key])
	|| (cls.key_dest == key_console && !consolekeys[key])
	|| (cls.key_dest == key_game && ( cls.state == ca_active || !consolekeys[key] ) ) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum and time as a parm
				Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", kb, key, time);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	if (shift_down)
		key = keyshift[key];

	switch (cls.key_dest)
	{
	case key_message:
		Key_Message (key);
		break;
	case key_menu:
		M_Keydown (key);
		break;

	case key_game:
	case key_console:
		Key_Console (key);
		break;
	default:
		Com_Error (ERR_FATAL, "Bad cls.key_dest");
	}
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	anykeydown = false;

	for (i=0 ; i<256 ; i++)
	{
		if ( keydown[i] || key_repeats[i] )
			Key_Event( i, false, 0 );
		keydown[i] = 0;
		key_repeats[i] = 0;
	}
}


/*
===================
Key_GetKey
===================
*/
int Key_GetKey (void)
{
	key_waiting = -1;

	while (key_waiting == -1)
		Sys_SendKeyEvents ();

	return key_waiting;
}



char Key_MatchCommand(char * command)
{
	int i; // WAW[11/17/99]: This was a "char" go figure - 0 to 255 doesn't work with a character
 
	for (i = 0;i < 256;i++)
	{
		//NSS[11/14/99]: Hey... how about we check the keybinding's content?!
		//if (keybindings[i] && !stricmp(keybindings[i],command))
		if (keybindings[i])
		{
			if(!stricmp(keybindings[i],command))
			{
				return (char)(i);
			}
		}
  
	}

	return (0); // no match found
}

#ifdef JPN
void ImeSetMode2(BOOL mode)
{
	HIMC hIMC;

    if ( !( hIMC = ImmGetContext( cl_hwnd ) ) )
        return;

	ImmSetOpenStatus(hIMC, mode);

	ImmReleaseContext(cl_hwnd, hIMC);
}
#endif