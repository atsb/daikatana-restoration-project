// ENGLISH text

//
// these are the key numbers that should be passed to Key_Event
//
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32
// normal keys should be passed as lowercased ascii
#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131
#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define	K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define	K_KP_ENTER		169
#define K_KP_INS   		170
#define	K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174
#define K_PAUSE			255
//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202
//
// joystick buttons
//
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206
//
// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
//
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238

#define K_MWHEELDOWN	239
#define K_MWHEELUP		240

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

static keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"???????s1", K_MOUSE1},
	{"???????s2", K_MOUSE2},
	{"???????s3", K_MOUSE3},

	{"?n?????s1", K_JOY1},
	{"?n?????s2", K_JOY2},
	{"?n?????s3", K_JOY3},
	{"?n?????s4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },

	{"?????u???W", K_MWHEELUP },
	{"?????u???U", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"????", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

// vkey conversion table
//==========================================================================
static byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,	27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
    'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
    '\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
    'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
    K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
    K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0  , K_HOME, 
    K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4 
    K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
    K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 

static char *tongue[] =
{
	"?A???o",						// T_PICKUP_WEAPON	weapon pickup
	"?A???n",							// T_PICKUP_HEALTH	health pickup
	"?????O?@????",					// T_PICKUP_ARMOR_GOOD	armor good pickup
	"???o???q????",					// T_PICKUP_BOOST		boost pickup
	"???n?x?s?_??",						// T_PICKUP_SAVEGEM	pick up Save Gem
												
	"?O?q",								// T_USEBOOST_POWER	power boost label
	"????",								// T_USEBOOST_ATTACK	attack boost label
	"?t??",								// T_USEBOOST_SPEED	speed boost label
	"?u??",								// T_USEBOOST_ACRO	jump boost label
	"???O",								// T_USEBOOST_VITA	health boost label
												
	"?????????F",							// T_BOMB_EXPLODE			bomb explode
	"?A???????????~???V?X",			// T_BOMB_NEED_BOTTLE		need bottle to mix ingredients
	"???o?????~?G",						// T_BOMB_ITEM_REQUIRED1	ingredient required
	"?A?@?F?@?U????",					// T_BOMB_CREATED			bomb create
	"?A????",							// T_BOMB_FOUND				bomb found, vowel
	"?@?U???D",						// T_BOMB_INGREDIENT_1		bomb ingredient 1
	"?@??????",							// T_BOMB_INGREDIENT_2		bomb ingredient 2
	"?@?~?v??",						// T_BOMB_INGREDIENT_3		bomb ingredient 3
	"?@?????~",						// T_BOMB_BOTTLE			bomb bottle
												
	"???D",								// T_BOMB_INGREDIENT_SHORT_1	bomb item, short description
	"????",								// T_BOMB_INGREDIENT_SHORT_2	bomb item, short description
	"?v??",							// T_BOMB_INGREDIENT_SHORT_3	bomb item, short description
												
	"?A?\????",							// T_BOOK_READ		read book
	"?A?L?k?b??????",					// T_BOOK_NO_READ		no read book
	"?U?F?J???k?N??",						// T_BOOK_WYNDRAX		Wyndrax spellbook
// SCG[12/5/99]: 	"?????????c????",			// T_BOOK_EVIL		Satanic book of evil
												
	"?A?B?_",							// T_KEY_PICKUP		key pickup
	"???a?_??",							// T_KEY_CRYPT		crypt key name
	"?U?F?J???_",						// T_KEY_WYNDRAX		Wyndrax key name
	"???c???_???d",					// T_KEY_PRISONCELL	Prison cell key

	"?????????d",						// T_KEY_RED_CARD				
	"?????????d",					// T_KEY_BLUE_CARD				
	"?????????d",					// T_KEY_GREEN_CARD				
	"?????????d",					// T_KEY_YELLOW_CARD			
	"?????_????",						// T_KEY_HEX_KEYSTONE			
	"?|???_????",						// T_KEY_QUAD_KEYSTONE			
	"?T???_????",						// T_KEY_TRIGON_KEYSTONE		
	"?W?j?@??",							// T_MEGASHIELD					
	"?k?O?u?\",							// T_SKULL_OF_INVINCIBILITY
	"???r??",						// T_POISON_ANTIDOTE
	"?w???J??",								// T_DRACHMA
	"????????",						// T_HORN
	"???????_???@?????]A?^",				// T_KEY_AEGIS_A
	"???????_???@?????]E?^",				// T_KEY_AEGIS_E
	"???????_???@?????]G?^",				// T_KEY_AEGIS_G
	"???????_???@?????]I?^",				// T_KEY_AEGIS_I
	"???????_???@?????]S?^",				// T_KEY_AEGIS_S
	"?b???????H??",					// T_PURIFIER_SHARD
	"?????c?l",							// T_BLACK_CHEST
	"????????",				// T_RING_OF_FIRE
	"??????????????",			// T_RING_OF_UNDEAD
	"???{?q????",			// T_RING_OF_LIGHTNING
	"?????A",							// T_ENVIROSUIT

	"?^?_?u??",							// T_HEALTH_KIT
	"?????F??",							// T_GOLDENSOUL
	"?F???y",							// T_WRAITHORB,

	"?x?s?_??",								// T_SAVEGEM
	"?A?S???x?s?_??",					//cek[12-13-99]
	"???R???~",							// cek[1-3-00]

	// cek[2-2-00]: altered the following *******************txtP************************
	"???R?]",
	"???R??",
	"???R?~",
	"???R???c",
	"???R??",
	"?o?????r???O?????C",

// SCG[2/8/00]: messages for completed artifacts
	"?A???????????????????F?I",
	"?A?????????T???_???????I",
	"?A???X?n?b????"
};


// menu-related
static char *tongue_menu[] =
{
	"?e?i",                                  // key bind to Move Forward
	"???h",                                 // key bind to Move Backward
	"????",                                // key bind to Step Left
	"?k??",                               // key bind to Step Right
	"????",                                   // key bind to Attack
	"???D",                                     // key bind to Jump/Up
	"?????????@",                              // key bind to Use/Operate object
	"???U",                                   // key bind to Crouch/Down
	"?U?@???Z??",                              // key bind to Next Weapon
	"?W?@???Z??",                              // key bind to Prev Weapon
	"????",                                // key bind to Turn Left
	"?k??",                               // key bind to Turn Right
	"?]",                                      // key bind to Run
	"????",                                 // key bind to  Sidestep
	"?V?W??",                                  // key bind to Look Up
	"?V?U??",                                // key bind to Look Down
	"??????",                              // key bind to Center View
	"?????[??",                                // key bind to Mouse Look
	"?????[??",                                  // key bind to Keyboard Look
	"?i???d???W?[",                             // key bind to Screen size up
	"?i???d???U??",                           // key bind to Screen size down
	"????????????",                            // key bind to Show All Huds
	"?}?????~??",                                // key bind to Inventory Open
	"???????~",                            // key bind to Use Item
	//  "???U???~",                                // key bind to Drop Item
	"?U?@?????~",                                // key bind to Next Item
	"?W?@?????~",                                // key bind to Prev Item
	"????????",                            // key bind to Swap Sidekick
	"?U?@?????O",                             // key bind to Command Next
	"?W?@?????O",                             // key bind to Command Prev
	"?????????O",	                            // key bind to Command Apply				// cek[1-5-00] changed from Apply to Use

	ARROW_LEFT_STRING  "??",                 // left arrow key
	ARROW_UP_STRING    "?W",                   // up arrow key
	ARROW_DOWN_STRING  "?U",                 // down arrow key
	ARROW_RIGHT_STRING "?k",                // right arrow key
	"--",                                       // no bind setting for this key

	"?I??????Enter?]?w?ADEL?R??",     // set/delete key
	"?I??????Enter????",                 // select key
	"?????]?w?AESC???_",             // set/abort

	"?w?]",                                 // default button

	"?L",                                     // no mouse bindings
	"????/?k??",                          // mouse "move left and right" setting
	"????/?k??",                          // mouse "turn left and right" setting
	"?V?W/?U??",                             // mouse "look up and down" setting
	"?e?i/???h",                           // mouse "move forward and back" setting
	"?NY?b???V",                           // reverse mouse axis
	"?????[??",                               // Mouse-Look
	"?[??????",                              // Mouse-Look
	"X?b",                                   // x axis
	"Y?b",                                   // y axis
	"???s1",                                 // button 1 label
	"???s2",                                 // button 2 label
	"???s3",                                 // button 3 label
	"?????@?????O?AESC????",          // select a command, ESCape to cancel
	"????/?k??",                                 // turn left and right, abbreviated
	"??/?k????",                               // strafe left and right, abbreviated
	"?V?W/?U??",                                 // look up and down, abbreviated
	"?e?i/???h",                                 // move forward and back, abbreviated
	"?????F????",                   // mouse sensitivity for x-axis				//cek[12-13-99]
	"?????F????",                     // mouse sensitivity for y-axis				//cek[12-13-99]
	"?]?w?????????G",                 // configure mouse options

	"???`?v??",                               // 'deathmatch' game
	"???`????",                                 // 'deathtag' game
	"?X?@",                              // 'cooperative' game

	"????",                                     // easy difficulty
	"????",                                   // medium difficulty
	"?x??",                                     // hard difficulty

	"?j?M?????????C??",					// SCG[12/1/99]: 
	"?j?M???????C??",						// SCG[12/1/99]: 
	"?}?l?h?H?C??",					// SCG[12/1/99]: 

	"????????",                                      // local area network game
	"????????",                                 // internet game

	"?[?J",                                     // join server
	"?D??",                                     // host new server
	"?W?[ " ARROW_DOWN_STRING,                   // add address
	"???s",                                  // refresh network games
	"?]?w",                                    // player setup

	//"?D????",                               // handedness label
	//"????",                                     // handedness, left
	//"????",                                    // handedness, center (ambi-dextrous)
	//"?k??",                                    // handedness, right

	"?s?u?t?v",                             // connection speed label
	"28.8k",                                    // connection type 1
	"56.6k",                                    // connection type 2
	"ISDN",										// connection type 3
	"?l?u??????/DSL",							    // connection type 4
	"T1/????????",                                   // connection type 5
	"?????????q",                             // connection type 6

	"????????",								// SCG[12/2/99]: 
	"?H???C??",							// SCG[12/2/99]: 
	"?}?l???`",							// SCG[12/2/99]: 
	"????????",								// SCG[12/1/99]: 
	"CTF????",								// SCG[12/1/99]: 
	"??1???C??",								// SCG[12/1/99]: 
	"??2???C??",								// SCG[12/1/99]: 
	"????????",								// SCG[12/1/99]: 
	"????????",								// SCG[12/1/99]: 
	"???d????",								// level limit
	"?}?l???`",							// SCG[12/2/99]: 
	"?C???a??",									// SCG[12/1/99]: 
	"?C??????",								// game mode (deathmatch,deathtag,co-op)
	"????????",								// skill level
	"????????",								// time limit
	"???h?H??",								// maximum players
	"?H??",								// character
	"?????C??",								// base color
	"?}?l?I",									// start multiplayer game
												
	"?Z?????d",								// weapons remain
	"???~?A??",							// items rematerialize
	"???Z?A??",								// spawn farthest point from players
	"?e?\???}",							// allow exiting of the level
	"???P?a??",									// same map
	"?O?q?A??",							// force respawn
	"????????",							// players take falling damage
	"???Y?j??",							// instant power-ups
	"?e?\?j??",								// allow power-ups
	"?e?\???R",								// allow health kits
	"?e?\????",								// allow armor
	"?u???L??",							// unlimited ammunition
	"????????",								// fixed field-of-vision
	"?????C??",									// team-play
	"???`????",							// hurt teammates
	"???t????",								// fast weapon switch
	"?}?B?n??",								// footsteps audible
	"?e?\???_",								// allow hook
	"???u?j?l?u",							// show shotgun shells?
												
	"???A??IP???}",						// server internet provider address
	"?????????????C??",						// no local games found
	"?}?l?a??",								// starting map
	"???A???W??",								// server name
	"?????W??",								// player name
	"?????W??",								// team name
												
	"?i???????H",						// confirm quit
												
	"?O",										// yes, acknowledgement
	"?_",										 // no, acknowledgement
	"?x?s?C??",								// cek[1-3-00] changed from 'save' to 'save game'

	"???J?]?w",								//cek[12-6-99]
	"?x?s?]?w",								//cek[12-6-99]
	"?R???]?w",							//cek[12-6-99]
	"?????]?w????",				//cek[12-6-99]
	"?R???????????H",						//cek[12-6-99]
												
	"???J?C??",								//cek[12-6-99]
	"????",									//cek[12-6-99]
	"???R",									//cek[12-6-99]
	"???d",									//cek[12-6-99]
	"????",									//cek[12-6-99]
	"???K",									//cek[12-6-99]
	"????",										//cek[12-6-99]
	"?`?p",									//cek[12-6-99]
	"???`",									//cek[12-6-99]

	"????3D????",								//cek[12-6-99]

	"?w?x?s",									//cek[12-7-99]
	"?v???J",									//cek[12-7-99]
	"?v?R??",									//cek[12-7-99]

	"?y?Z????",								//cek[12-7-99]
	"?u???V?W",									//cek[12-7-99]

	"?u???V?U",								//cek[12-7-99]

	"????ALT-TAB?\??",						//cek[12-7-99]
	"???{?M?O??",						//cek[12-7-99]
	"?Z??????",							//cek[12-7-99]
	"?????????Z??",						//cek[12-7-99]

	"?j??",								//cek[12-7-99] cek[12-13-99]
	"?Z???{??",							//cek[12-7-99]
	"?u??",								// SCG[1/4/00]: 


	"?e?i",									//cek[12-9-99]
	"???V",										//cek[12-9-99]
	"?W",										//cek[12-9-99]
	"????",									//cek[12-9-99]
	"???n",										//cek[12-9-99]
	"?????n??",								//cek[12-9-99]
	"?]?w???V?b",							//cek[12-9-99]
	"?]?w?n??????",				//cek[12-9-99]
	"?]?w?????s?\??",			//cek[12-9-99]
	"???U?n?????s?????]?w",		//cek[12-9-99]
	"????",								//cek[12-9-99]
	"?F????",								//cek[12-9-99]

	"?m?X??",							//cek[12-10-99]
	"?^??",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"????",										//cek[12-10-99]
	"??1??",									//cek[12-10-99]
	"??2??",									//cek[12-10-99]

	"???????????e",							//cek[12-13-99]
	"?????????t??",							//cek[12-13-99]
	"?????t??",								//cek[12-13-99]

	"???????o",								// cek[1-3-00]
	"?????L??",							// cek[1-3-00]
	"????????",							// cek[1-3-00]
	"???????h",							// cek[1-3-00]
	"???????d",							// cek[1-3-00]

	"?????Z??%d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"????????",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"?U?@????",									// cek[1-5-00]  -- similar in meaning.
	"?W?@????",									// cek[1-5-00]
	"????????",									// cek[1-5-00]

	"????",										// cek[1-11-00]
	"?p???O",								// SCG[1/13/00]: 
	"?p??????",								// cek[1-22-00]
	"%s?w?g??????%s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"?m???H",									// cek[1-22-00]

	"?e???j?U",								// cek[2-1-00]
	"?j?M???s",								// cek[2-1-00]
	"?~???H",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"?~???|???}?j?M\n??????Mplayer???A?????s?????C",	// cek[2-1-00]
	"?Q???s?D?B?????B?Q???M?p??\n?e??www.daikatananews.com?C\n?~???U?@?B?|?????A???????s?????C",			// cek[2-1-00]

	"?Z?? %d",
	"????",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"?e??www.planetdaikatana.com?C\n?~???U?@?B?|?????A???????s?????C",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"?]?B????",                               // always run
	"?u???V?[??",                               // look-spring
	"?????V?[??",                               // look-strafe
	"????????",                           // auto-targeting
	"???P",                                // crosshair
	"?Z???i??",                           // weapon visible
	"?r??",                                // subtitles

	"???????q",                      // volume for sound fx
	"???????q",                             // volume for music
	"???e???q",                         // volume for cinematics

	"?n??",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"?M????????",                            // apply video mode changes

	"??????",                               // fullscreen
// SCG[12/5/99]: 	"Stipple Alpha",                            // stipple alpha
	"?v?l",                                  // shadows
	"?X???{??",                                   // video driver
	"???R??",                               // resolution
	"?????j?p",                              // screen size
	"?G??",                               // brightness
	"?????~??",                          // texture quality
	"?i?????A??",                    // status bar visibility
	"??/?????o??",							// SCG[12/4/99]: 
	"?B?L",									// difficulty level easy
	"?Z?h",									// difficulty level medium
	"?N?x",									// difficulty level hard

	"???M?B",							// cek[1-11-00]
	"??"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"????",
	"???b",
	"?|?????@",
	"?K?????@"
};


// weapon names
static char *tongue_weapons[] =
{
	"?L?u??",
	"????",
	"????????",
	"?z?l",
	"?u",
	"C4???z??",
	" C4?????]",
//	"????",				// SCG[11/28/99]: 
//	"???????m??",		// SCG[11/28/99]:	
	"?j?M",
//	"?j?M???v",			// SCG[11/28/99]: 
//	"?j?M???m??",		// SCG[11/28/99]: 
//	"?j?M?C?R",	// SCG[11/28/99]: 
//	"?j?M????",			// SCG[11/28/99]: 
//	"?j?M????",		// SCG[11/28/99]: 
//	"?j?M????",		// SCG[11/28/99]: 
	"?j?M?P?R",
	"???L",
	"?V???????M",


//	"?????????q??",
	"?????J2020",
	"?@???l?u",
	"???????K??",
	"???l?j",
	"???l?u??",					// SCG[11/28/99]: 
	"???O?j",
	"???O",
	"?j???L?i?j",
	"?j???L?i?j",
//	"???F????????????",		// SCG[11/28/99]: 
//	"???K?q",				// SCG[11/28/99]: 
	"???????c??",
	"?@???r??",					// SCG[11/28/99]: 
	"?s?P????",
	"?s?P?q??",					// SCG[11/28/99]: 
	"?????j",
	"?L???h?u",				
	"?_???i",
	"?_?i?y",
	"?s?g?j",
	"?s?g?j?l?u",
	"?T???D???u",
	"?T???D???u",			// SCG[11/28/99]: 
	"????",
	"???u?j",
	"???u",
	"?v?????????`",
	"??????",
	"?n????????",
	"?n????????",
//	"???????_",				// SCG[11/28/99]: 
	"?????T?e?u",
	"?T?e?u?y??",
	"?r??",
	"?????D?r",					// SCG[11/28/99]: 
	"?U?F?J???????]",
	"???????]",
	"?z??????",
	"????????",
	"??????????"
};


static char *tongue_armors[] =
{
	"????????",
	"????????",
	"????????",
	"????????",
	"????????",
	"??????????",
	"??????????",
	"?w????????"
};


// monster names
static char *tongue_monsters[] =
{
	"??????????",
	"?????H",
	"???i?o",
	"???????B?S",
	"??????",
	"?T?Y??",
	"??????????",
	"?J???W",
	"?????J???J??",
// SCG[12/5/99]: 	"?W?????H",
	"???????`?y",
	"????????",
	"?s",
	"?G?H",
	"?S?z??",
	"?h???H",
	"?s?b??",
	"?????C??????",
	"????????",
	"????",
	"???Q??",
	"?????k??",
	"?J????",
	"?T?q?v",
// NSS[3/6/00]:"????????",
	"?a?????t?M?h",

	"?q?O?k?Z??",
	"???????U?l",
	"?C??????",
	"?????p?g?j",
	"?T?H",
	"??????",
// NSS[3/6/00]:	"???Y?H",
	"?T?q?u??",
	"???????F?k?v",
	"?E??????",
	"????????",
	"???v",
	"???H",
	"??????????",
	"??????",
	"?????????M?a5000",
	"???b??",
	"???\?????????b?L",
	"?G??",
	"???T",
	"???\?????????x",
// SCG[12/5/99]: 	"???\??????",
	"???\????????",
	"???\???????u?j?L",
	"?T??",
	"??????",

	"?u?\",
	"?G?u?H",
	"?D?u?H",
	"???????????o",
	"?p?j??",
	"?j??",
	"?v?????k?v",
	"?x??",
	"?p??",
	"?????{?q???v?g??",
// SCG[12/5/99]: 	"?????w?y????",
	"?Q????",
	"?????R??",
	"???v",
	"?U?F?J???v",
	"????",
	"??????",

	"?????]",
	"????",
	"????",
	"??????"
};


static char *tongue_world[] =
{
	"?|?????L",                                 // triggers to go until sequence complete
	"?B?J????",                         // trigger sequence completed

	"?A????????",                              // need a specific key
	"?A???????????T???_???I \n",               // wrong key in possession
	"?????q?o?????V???}\n",					  // SCG[11/1/99]: Can't be opened this way

	"?A???o?r?????}?F?A???t???C\n",    // poison expiring
	"?A???????????X?G?????F?C\n",           // oxylung expiring
	"?A?????????X?G???O?F?C\n",                // envirosuit expiring
	"???????F?????????C\n",                 // megashield expiring
	"???????y???z?F?C\n",                   // wraith orb expiring
	"?O?q?j???????G?C?C???????F?C\n",				  // power boost expiring
	"?????j???????G?C?C???????F?C\n",                // attack boost expiring
	"?t???j???????G?C?C???????F?C\n",                 // speed boost expiring
	"?u???j???????G?C?C???????F?C\n",             // jump boost expiring
	"???O?j???????G?C?C???????F?C\n",              // health boost expiring

	"?w?[?J?C??",                            // player joined the game
	"???}?o?@??",                           // player exited the level

	"?c????",                              // Hiro Miyamoto's full name
	"???t?j??",                           // Superfly's full name
	"??????????",                             // Mikiko's full name

	// SCG[10/18/99]: these next 5 had enums, but no strings. 
	"",								// SCG[10/18/99]: T_PLAYER_SELF_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_TIME_LIMIT_HIT
	"",								// SCG[10/18/99]: T_PLAYER_FRAG_LIMIT_HIT
	"",								// SCG[10/18/99]: T_TELL_CMD_UNKNOWN_PLAYER

	"???}?o??????????????\n",				// SCG[10/18/99]: no exiting level without sidekicks
	"?n???}?A???????o\n",				// SCG[11/9/99]: No exiting without a key
	"???r?I\n",						// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"?A?????@?????K?I \n",					// cek[1-3-00]
	"?k?O?u?\?????G?C?C???????F?C\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s?Q%s?????}?F?C\n"
};

static char *tongue_deathmsg_self[] =
{
	"???Y???Q???}?F?C\n",                       // death, fell
	"?Q?l?U?h?C\n",                             // death, drowned
	"???o???H?C\n",                                // death, lava
	"???]???F?C\n",                                 // death, slimed
	"?}?A???r???U?C\n",                    // death, poison
	"???????R?????i?C\n",                             // death, killed self
	"???e???z???F?C\n",                            // death, teleport
	"?Q?????@?????C\n",             // death, crushed

	"?]?U???{??C4?????C\n"                       // death, C4
};

static char *tongue_deathmsg_weapon1[] =
{
	"%s?Q%s???F???L?I\n",
	"%s?Q%s???????F?@?x?A?w?g???????g?I\n",
	"%s?Q%s???l???F?I\n",
	"%s?Q%s???o???C?K?V?I\n",
	"%s?Q%s???????j?F???I\n",
	"%s?????F%s???X???????b?I\n",
	"%s?Q%s?n???b?????I\n"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s??%s???????????L?????I\n",
	"%s???????r?F%s?@?f?I\n",
	"%s??%s???????K?a???I \n",
	"%s??%s???K???????????U?h?I\n",
	"%s??%s???z?F?I\n",
	"%s???b?t?????e?I\n"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s?Q%s?????L?@?????}?I\n",
	"%s??%s?????F?I\n",
	"%s ??%s???a?O???I \n",
	"%s?Q%s???y?P?{???I\n",
	"%s?????F%s???????q?I\n",
	"%s?Q%s?????q?????????I \n"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s?b%s???e?S?X?@?????U?I\n",
	"%s?Q%s?????u?_?g?I\n",
	"%s?Q%s?????I\n",
	"%s?Q%s?????@???d?I\n",
	"%s?Q%s?????B???I\n",
	"%s?Q%s?????@???}?I\n",
	"%s?Q%s?N?z?z?I\n"
};

static char *tongue_scoreboard[] =
{
	"?m?W",
	"??????",
	"?^????",
	"????",
	"?o?g?u??",
	"?????u??",
	"???`????",
	"????"										// cek[1-22-00]
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"?????x???I??:",//"Select Difficulty:",           // difficulty header
	"?B?L",	                   // newbie
	"?Z?h",                    // average
	"?N?x"                    // expert (or so they think!)
};




static char *tongue_skincolors[] =
{

	"??",                         // blue
	"??",                        // green
	"??",                          // red
	"??"                          // gold
};


static char *tongue_statbar[] =
{
	"?O?q",                        // skill 'power' label
	"????",                       // skill 'attack' label
	"?t??",												// skill 'speed' label
	"???D",													// skill 'acrobatic' label
	"???O",											// skill 'vitality' label

	"????",                        // armor label
	"???R",												// health label
	"?u??",													// ammo label
	"????",												// kills label
	"???d",  											// experience points label

	"?@???I ",                    // experience level up
	"????",                       // select skill
	"???w?????I??",                    // add point to skill

	"???J..."                    // loading new level
};


static char *tongue_weapon_misc[] =
{
	"C4???????????w???A?I \n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{
	"???o",	                        // 'get' item command
	"?L??",                          // 'come here', 'come to me'
	"???d",                           // 'stay' command, 'don't move'

	"????",                         // 'attack' command
	"???h",                       // 'back off', 'move away'

	"??????"                      //  asking to pick up an item: "Can 			
ve the shotgun?"
};

static char *tongue_ctf[] =
{
	"???X",
	"???X",

	"????????",

	"%s??????",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"??",
	"??",
	"??",
	"???K",
	"??",
	"??",
	"??",
	"??",

	"?{?b?????O%s?G%d?A%s?G%d\n",
	"?A??????%s???I\n",
	"?A????????%s?w?g?????F%s?I\n",
	"%s????%s?????A?a???F?A???X?l?I\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"?A?????F%s\n",
	"?A????????%s?????F%s\n",
	"%s?????F?A???X?l\n",
	"%s?w?g?m?^???F\n",



	"%d?O?@???L???????????r?A????1??2\n",
	"?A?{?b?O%d???]%s?^\n",
	"?A?w?g?O%d??\n",
	"%s?[?J?F%s??\n",									// translate same as with the %s team above

	"%s???w?g???F?I\n?????`???V%s?G%d?A%s?G%d\n",		// translate same as with the %s team above
	"?o???C???????I\n",
	"?????????I\n",
	"???F?m?X?????W???I\n"
};

static char *tongue_deathtag[] =
{
	"???u",														// as in backpack
	"%s???u",													// as in red pack
	"???F?????W???I\n",
	"?A???????????F?I\n",
	"?A???????????????I\n",

	"%s????%s???A???o?F%s?I\n",				// translate using the '%s team' guidelines above

	"?A???????o???@???I\n",
	"?A???????o??%d???I\n",								// you get 2 points
	"%s?????o???@???I\n",								// translate using the '%s team' guidelines above

	"%s?????o??%d???I\n",							// The red team gets 2 points

	"?A?o???F?I\n",
	"?A????????%s?o???????I\n",
	"%s????%s?????A?o???????I\n"
};

