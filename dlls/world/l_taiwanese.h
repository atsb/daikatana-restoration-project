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

	{"·Æ¹««ö¶s1", K_MOUSE1},
	{"·Æ¹««ö¶s2", K_MOUSE2},
	{"·Æ¹««ö¶s3", K_MOUSE3},

	{"·n±ì«ö¶s1", K_JOY1},
	{"·n±ì«ö¶s2", K_JOY2},
	{"·n±ì«ö¶s3", K_JOY3},
	{"·n±ì«ö¶s4", K_JOY4},

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

	{"·Æ¹«ºu½ü¤W", K_MWHEELUP },
	{"·Æ¹«ºu½ü¤U", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"¤À¸¹", ';'},	// because a raw semicolon seperates commands

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
	"§A¨ú±o",						// T_PICKUP_WEAPON	weapon pickup
	"§A»Ý­n",							// T_PICKUP_HEALTH	health pickup
	"¨ü¨ì«OÅ@¨Ó¦Û",					// T_PICKUP_ARMOR_GOOD	armor good pickup
	"¨ú±o¯à¶q¨Ó¦Û",					// T_PICKUP_BOOST		boost pickup
	"»Ý­nÀx¦sÄ_¥Û",						// T_PICKUP_SAVEGEM	pick up Save Gem
												
	"¤O¶q",								// T_USEBOOST_POWER	power boost label
	"§ðÀ»",								// T_USEBOOST_ATTACK	attack boost label
	"³t«×",								// T_USEBOOST_SPEED	speed boost label
	"¼u¸õ",								// T_USEBOOST_ACRO	jump boost label
	"Åé¤O",								// T_USEBOOST_VITA	health boost label
												
	"¸Ó©ñ·Ï¤õ¤F",							// T_BOMB_EXPLODE			bomb explode
	"§A¥²¶·¦³­ÓªÅ²~¨Ó²V¦X",			// T_BOMB_NEED_BOTTLE		need bottle to mix ingredients
	"¨ú±oªºª««~¡G",						// T_BOMB_ITEM_REQUIRED1	ingredient required
	"§A§@¤F¤@³U¬µÃÄ",					// T_BOMB_CREATED			bomb create
	"§A§ä¨ì",							// T_BOMB_FOUND				bomb found, vowel
	"¤@³U²¸ÁD",						// T_BOMB_INGREDIENT_1		bomb ingredient 1
	"¤@¶ô·ÑºÒ",							// T_BOMB_INGREDIENT_2		bomb ingredient 2
	"¤@²~µv¥Û",						// T_BOMB_INGREDIENT_3		bomb ingredient 3
	"¤@­ÓªÅ²~",						// T_BOMB_BOTTLE			bomb bottle
												
	"²¸ÁD",								// T_BOMB_INGREDIENT_SHORT_1	bomb item, short description
	"¤ì¬´",								// T_BOMB_INGREDIENT_SHORT_2	bomb item, short description
	"µv¥Û",							// T_BOMB_INGREDIENT_SHORT_3	bomb item, short description
												
	"§A¾\Åª¨ì",							// T_BOOK_READ		read book
	"§AµLªk¦b¦¹¨Ï¥Î",					// T_BOOK_NO_READ		no read book
	"¸U¹F§Jªºªk³N®Ñ",						// T_BOOK_WYNDRAX		Wyndrax spellbook
// SCG[12/5/99]: 	"¼»¥¹ªº¨¸´c¤§®Ñ",			// T_BOOK_EVIL		Satanic book of evil
												
	"§A¬B°_",							// T_KEY_PICKUP		key pickup
	"¹Ó¦aÆ_°Í",							// T_KEY_CRYPT		crypt key name
	"¸U¹F§J¤§Æ_",						// T_KEY_WYNDRAX		Wyndrax key name
	"ºÊ¨cªºÆ_°Í¥d",					// T_KEY_PRISONCELL	Prison cell key

	"¬õ¦â±±¨î¥d",						// T_KEY_RED_CARD				
	"ÂÅ¦â±±¨î¥d",					// T_KEY_BLUE_CARD				
	"ºñ¦â±±¨î¥d",					// T_KEY_GREEN_CARD				
	"¶À¦â±±¨î¥d",					// T_KEY_YELLOW_CARD			
	"¤»¨¤Æ_°Í¥Û",						// T_KEY_HEX_KEYSTONE			
	"¥|¨¤Æ_°Í¥Û",						// T_KEY_QUAD_KEYSTONE			
	"¤T¨¤Æ_°Í¥Û",						// T_KEY_TRIGON_KEYSTONE		
	"¶W±jÅ@¬Þ",							// T_MEGASHIELD					
	"ªk¤O¾uÅ\",							// T_SKULL_OF_INVINCIBILITY
	"¸Ñ¬r¾¯",						// T_POISON_ANTIDOTE
	"¼w©Ô§J°¨",								// T_DRACHMA
	"²î¤Ò¸¹¨¤",						// T_HORN
	"¯«¬Þ¤§Æ_ªº¤@³¡¥÷¡]A¡^",				// T_KEY_AEGIS_A
	"¯«¬Þ¤§Æ_ªº¤@³¡¥÷¡]E¡^",				// T_KEY_AEGIS_E
	"¯«¬Þ¤§Æ_ªº¤@³¡¥÷¡]G¡^",				// T_KEY_AEGIS_G
	"¯«¬Þ¤§Æ_ªº¤@³¡¥÷¡]I¡^",				// T_KEY_AEGIS_I
	"¯«¬Þ¤§Æ_ªº¤@³¡¥÷¡]S¡^",				// T_KEY_AEGIS_S
	"²b¤Æ¾¹ªº¸H¤ù",					// T_PURIFIER_SHARD
	"¶Â¦â½c¤l",							// T_BLACK_CHEST
	"§Ü¤õ¤§§Ù",				// T_RING_OF_FIRE
	"§Ü¤£¦º¥Íª«¤§§Ù",			// T_RING_OF_UNDEAD
	"§Ü°{¹q¤§§Ù",			// T_RING_OF_LIGHTNING
	"Àô¹ÒªA",							// T_ENVIROSUIT

	"¦^´_¤u¨ã",							// T_HEALTH_KIT
	"¶Àª÷ÆF»î",							// T_GOLDENSOUL
	"ÆF»î²y",							// T_WRAITHORB,

	"Àx¦sÄ_¥Û",								// T_SAVEGEM
	"§A¨S¦³Àx¦sÄ_¥Û",					//cek[12-13-99]
	"¥Í©R¤§²~",							// cek[1-3-00]

	// cek[2-2-00]: altered the following *******************txtP************************
	"¥Í©R¥]",
	"¥Í©RÅø",
	"¥Í©R²~",
	"¥Í©R¤§½c",
	"¥Í©R²°",
	"³o­Ó¸Ñ¬r¾¯¬OªÅªº¡C",

// SCG[2/8/00]: messages for completed artifacts
	"§A§ä¨ì©Ò¦³¯«¬Þªº²Å¸¹¤F¡I",
	"§A§ä¨ì¥þ³¡¤T­ÓÆ_°Í¤§¥Û¡I",
	"§A²Õ¦X¦n²b¤Æ¾¹"
};


// menu-related
static char *tongue_menu[] =
{
	"«e¶i",                                  // key bind to Move Forward
	"«á°h",                                 // key bind to Move Backward
	"¥ª½ñ",                                // key bind to Step Left
	"¥k½ñ",                               // key bind to Step Right
	"§ðÀ»",                                   // key bind to Attack
	"¸õÅD",                                     // key bind to Jump/Up
	"¨Ï¥Î¡þ¾Þ§@",                              // key bind to Use/Operate object
	"ÃÛ¤U",                                   // key bind to Crouch/Down
	"¤U¤@¶µªZ¾¹",                              // key bind to Next Weapon
	"¤W¤@¶µªZ¾¹",                              // key bind to Prev Weapon
	"¥ªÂà",                                // key bind to Turn Left
	"¥kÂà",                               // key bind to Turn Right
	"¶]",                                      // key bind to Run
	"¾î²¾",                                 // key bind to  Sidestep
	"¦V¤W¬Ý",                                  // key bind to Look Up
	"¦V¤U¬Ý",                                // key bind to Look Down
	"¬Ý¤¤¶¡",                              // key bind to Center View
	"·Æ¹«Æ[¬Ý",                                // key bind to Mouse Look
	"«öÁäÆ[¬Ý",                                  // key bind to Keyboard Look
	"¥i¨£½d³ò¼W¥[",                             // key bind to Screen size up
	"¥i¨£½d³ò¤U­°",                           // key bind to Screen size down
	"Åã¥Ü©Ò¦³¤¶­±",                            // key bind to Show All Huds
	"¶}±Òª««~Äæ",                                // key bind to Inventory Open
	"¨Ï¥Îª««~",                            // key bind to Use Item
	//  "¥á¤Uª««~",                                // key bind to Drop Item
	"¤U¤@¶µª««~",                                // key bind to Next Item
	"¤W¤@¶µª««~",                                // key bind to Prev Item
	"¤Á´«°¼½ð",                            // key bind to Swap Sidekick
	"¤U¤@¶µ«ü¥O",                             // key bind to Command Next
	"¤W¤@¶µ«ü¥O",                             // key bind to Command Prev
	"¨Ï¥Îªº«ü¥O",	                            // key bind to Command Apply				// cek[1-5-00] changed from Apply to Use

	ARROW_LEFT_STRING  "¥ª",                 // left arrow key
	ARROW_UP_STRING    "¤W",                   // up arrow key
	ARROW_DOWN_STRING  "¤U",                 // down arrow key
	ARROW_RIGHT_STRING "¥k",                // right arrow key
	"--",                                       // no bind setting for this key

	"ÂI¿ï©Î«öEnter³]©w¡ADEL§R°£",     // set/delete key
	"ÂI¿ï©Î«öEnter¿ï¾Ü",                 // select key
	"«öÁä³]©w¡AESC¤¤Â_",             // set/abort

	"¹w³]",                                 // default button

	"µL",                                     // no mouse bindings
	"¥ª²¾/¥k²¾",                          // mouse "move left and right" setting
	"¥ªÂà/¥kÂà",                          // mouse "turn left and right" setting
	"¦V¤W/¤U¬Ý",                             // mouse "look up and down" setting
	"«e¶i/«á°h",                           // mouse "move forward and back" setting
	"±NY¶b¤Ï¦V",                           // reverse mouse axis
	"·Æ¹«Æ[¬Ý",                               // Mouse-Look
	"Æ[¬Ý¾î²¾",                              // Mouse-Look
	"X¶b",                                   // x axis
	"Y¶b",                                   // y axis
	"«ö¶s1",                                 // button 1 label
	"«ö¶s2",                                 // button 2 label
	"«ö¶s3",                                 // button 3 label
	"¿ï¾Ü¤@­Ó«ü¥O¡AESC¨ú®ø",          // select a command, ESCape to cancel
	"¥ªÂà/¥kÂà",                                 // turn left and right, abbreviated
	"¥ª/¥k¾î²¾",                               // strafe left and right, abbreviated
	"¦V¤W/¤U¬Ý",                                 // look up and down, abbreviated
	"«e¶i/«á°h",                                 // move forward and back, abbreviated
	"¤ô¥­ÆF±Ó«×",                   // mouse sensitivity for x-axis				//cek[12-13-99]
	"««ª½ÆF±Ó«×",                     // mouse sensitivity for y-axis				//cek[12-13-99]
	"³]©w·Æ¹«¿ï¶µ¡G",                 // configure mouse options

	"¦º¤`ÄvÁÉ",                               // 'deathmatch' game
	"¦º¤`¨ë±þ",                                 // 'deathtag' game
	"¦X§@",                              // 'cooperative' game

	"Â²³æ",                                     // easy difficulty
	"¤¤µ¥",                                   // medium difficulty
	"§xÃø",                                     // hard difficulty

	"·j´Mºô»Úºô¸ô¹CÀ¸",					// SCG[12/1/99]: 
	"·j´M°Ï°ì©Ê¹CÀ¸",						// SCG[12/1/99]: 
	"¶}©l¦h¤H¹CÀ¸",					// SCG[12/1/99]: 

	"°Ï°ìºô¸ô",                                      // local area network game
	"ºô»Úºô¸ô",                                 // internet game

	"¥[¤J",                                     // join server
	"¥D±±",                                     // host new server
	"¼W¥[ " ARROW_DOWN_STRING,                   // add address
	"§ó·s",                                  // refresh network games
	"³]©w",                                    // player setup

	//"ºD¥Î¤â",                               // handedness label
	//"¥ª¤â",                                     // handedness, left
	//"Âù¤â",                                    // handedness, center (ambi-dextrous)
	//"¥k¤â",                                    // handedness, right

	"³s½u³t²v",                             // connection speed label
	"28.8k",                                    // connection type 1
	"56.6k",                                    // connection type 2
	"ISDN",										// connection type 3
	"Æl½u¼Æ¾Ú¾÷/DSL",							    // connection type 4
	"T1/°Ï°ìºô¸ô",                                   // connection type 5
	"¨Ï¥ÎªÌ¦Û­q",                             // connection type 6

	"®É¶¡­­¨î",								// SCG[12/2/99]: 
	"¤Hª«ÃC¦â",							// SCG[12/2/99]: 
	"¶}©l³¹¸`",							// SCG[12/2/99]: 
	"¿ï¾Ü¶¤¥î",								// SCG[12/1/99]: 
	"CTF­­¨î",								// SCG[12/1/99]: 
	"²Ä1¶¤ÃC¦â",								// SCG[12/1/99]: 
	"²Ä2¶¤ÃC¦â",								// SCG[12/1/99]: 
	"¤À¼Æ­­¨î",								// SCG[12/1/99]: 
	"±þ¼Ä­­¨î",								// SCG[12/1/99]: 
	"Ãö¥d­­¨î",								// level limit
	"¶}©l³¹¸`",							// SCG[12/2/99]: 
	"¹CÀ¸¦a¹Ï",									// SCG[12/1/99]: 
	"¹CÀ¸¼Ò¦¡",								// game mode (deathmatch,deathtag,co-op)
	"§Þ¯àµ¥¯Å",								// skill level
	"®É¶¡­­¨î",								// time limit
	"³Ì¦h¤H¼Æ",								// maximum players
	"¤Hª«",								// character
	"°ò¥»ÃC¦â",								// base color
	"¶}©l¡I",									// start multiplayer game
												
	"ªZ¾¹°±¯d",								// weapons remain
	"ª««~¦A¥Í",							// items rematerialize
	"»·¶Z¦A¥Í",								// spawn farthest point from players
	"®e³\Â÷¶}",							// allow exiting of the level
	"¬Û¦P¦a¹Ï",									// same map
	"¤O¶q¦A¥Í",							// force respawn
	"±¼¸¨¨ü¶Ë",							// players take falling damage
	"¥ß§Y±j¤Æ",							// instant power-ups
	"®e³\±j¤Æ",								// allow power-ups
	"®e³\¥Í©R",								// allow health kits
	"®e³\¸Ë¥Ò",								// allow armor
	"¼ußÓµL­­",							// unlimited ammunition
	"µø¨¤­×¥¿",								// fixed field-of-vision
	"²Õ¶¤¹CÀ¸",									// team-play
	"¶Ë®`¶¤¤Í",							// hurt teammates
	"§Ö³t¤Á´«",								// fast weapon switch
	"¸}¨BÁn­µ",								// footsteps audible
	"®e³\§ì¹_",								// allow hook
	"´²¼uºj¤l¼u",							// show shotgun shells?
												
	"¦øªA¾¹IP¦ì§}",						// server internet provider address
	"§ä¤£¨ì°Ï°ì©Ê¹CÀ¸",						// no local games found
	"¶}©l¦a¹Ï",								// starting map
	"¦øªA¾¹¦WºÙ",								// server name
	"ª±ªÌ¦WºÙ",								// player name
	"¶¤¥î¦WºÙ",								// team name
												
	"¶i¦æ¤Á¸¡¡H",						// confirm quit
												
	"¬O",										// yes, acknowledgement
	"§_",										 // no, acknowledgement
	"Àx¦s¹CÀ¸",								// cek[1-3-00] changed from 'save' to 'save game'

	"¸ü¤J³]©w",								//cek[12-6-99]
	"Àx¦s³]©w",								//cek[12-6-99]
	"§R°£³]©w",							//cek[12-6-99]
	"¿ï¾Ü³]©wÀÉ®×",				//cek[12-6-99]
	"§R°£¿ï¾ÜÀÉ®×¡H",						//cek[12-6-99]
												
	"¸ü¤J¹CÀ¸",								//cek[12-6-99]
	"¸Ë¥Ò",									//cek[12-6-99]
	"¥Í©R",									//cek[12-6-99]
	"Ãö¥d",									//cek[12-6-99]
	"©Çª«",									//cek[12-6-99]
	"¯µ±K",									//cek[12-6-99]
	"®É¶¡",										//cek[12-6-99]
	"Á`­p",									//cek[12-6-99]
	"³¹¸`",									//cek[12-6-99]

	"¨Ï¥Î3D­µ³õ",								//cek[12-6-99]

	"¤wÀx¦s",									//cek[12-7-99]
	"¤v¸ü¤J",									//cek[12-7-99]
	"¤v§R°£",									//cek[12-7-99]

	"¬yºZ·Æ¹«",								//cek[12-7-99]
	"ºu½ü¦V¤W",									//cek[12-7-99]

	"ºu½ü¦V¤U",								//cek[12-7-99]

	"Ãö³¬ALT-TAB¥\¯à",						//cek[12-7-99]
	"¦å¸{©M±O±þ",						//cek[12-7-99]
	"ªZ¾¹¸õ°Ê",							//cek[12-7-99]
	"¦Û°Ê¤Á´«ªZ¾¹",						//cek[12-7-99]

	"±j«×",								//cek[12-7-99] cek[12-13-99]
	"ªZ¾¹°{¥ú",							//cek[12-7-99]
	"¼u²ª",								// SCG[1/4/00]: 


	"«e¶i",									//cek[12-9-99]
	"¾î¦V",										//cek[12-9-99]
	"¤W",										//cek[12-9-99]
	"¨¤«×",									//cek[12-9-99]
	"°¾·n",										//cek[12-9-99]
	"¨Ï¥Î·n±ì",								//cek[12-9-99]
	"³]©w¤è¦V¶b",							//cek[12-9-99]
	"³]©w·n±ì¿ï¶µ",				//cek[12-9-99]
	"³]©w¦¹«ö¶s¥\¯à",			//cek[12-9-99]
	"«ö¤U·n±ì«ö¶s§¹¦¨³]©w",		//cek[12-9-99]
	"¬É­­",								//cek[12-9-99]
	"ÆF±Ó«×",								//cek[12-9-99]

	"·mºXÁÉ",							//cek[12-10-99]
	"¦^ªð",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"¦Û°Ê",										//cek[12-10-99]
	"²Ä1¶¤",									//cek[12-10-99]
	"²Ä2¶¤",									//cek[12-10-99]

	"¼½©ñÂ²¤¶°Êµe",							//cek[12-13-99]
	"¨Ï¥Î§Þ¯à¨t²Î",							//cek[12-13-99]
	"·Æ¹«³t«×",								//cek[12-13-99]

	"¹Ù¦ñ¨ú±o",								// cek[1-3-00]
	"¹Ù¦ñ¹L¨Ó",							// cek[1-3-00]
	"¹Ù¦ñ§ðÀ»",							// cek[1-3-00]
	"¹Ù¦ñ«á°h",							// cek[1-3-00]
	"¹Ù¦ñ°±¯d",							// cek[1-3-00]

	"¿ï¾ÜªZ¾¹%d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"¤¶­±¤Á´«",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"¤U¤@¤¶­±",									// cek[1-5-00]  -- similar in meaning.
	"¤W¤@¤¶­±",									// cek[1-5-00]
	"¨Ï¥Î¤¶­±",									// cek[1-5-00]

	"¥æ½Í",										// cek[1-11-00]
	"­p¤ÀªO",								// SCG[1/13/00]: 
	"¤p¶¤¥æ½Í",								// cek[1-22-00]
	"%s¤w¸g¤À¬£¬°%s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"¸m´«¡H",									// cek[1-22-00]

	"«e©¹¤jÆU",								// cek[2-1-00]
	"¤j¤MªÀ¸s",								// cek[2-1-00]
	"Ä~Äò¡H",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"Ä~Äò·|Â÷¶}¤j¤M\n¨Ã±Ò°ÊMplayer¦øªA¾¹ªºÂsÄý¾¹¡C",	// cek[2-1-00]
	"·Q¬Ý·s»D¡B¥æ½Í¡B°Q½×©M­p¤À\n«e©¹www.daikatananews.com¡C\nÄ~Äò¤U¤@¨B·|°õ¦æ§Aªººô­¶ÂsÄý¾¹¡C",			// cek[2-1-00]

	"ªZ¾¹ %d",
	"¨ú®ø",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"«e©¹www.planetdaikatana.com¡C\nÄ~Äò¤U¤@¨B·|°õ¦æ§Aªººô­¶ÂsÄý¾¹¡C",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"¶]¨B¦æ°Ê",                               // always run
	"¼u¸õ¡VÆ[¬Ý",                               // look-spring
	"¾î²¾¡VÆ[¬Ý",                               // look-strafe
	"¦Û°ÊºË·Ç",                           // auto-targeting
	"·Ç¬P",                                // crosshair
	"ªZ¾¹¥i¨£",                           // weapon visible
	"¦r¹õ",                                // subtitles

	"­µ®Ä­µ¶q",                      // volume for sound fx
	"­µ¼Ö­µ¶q",                             // volume for music
	"°Êµe­µ¶q",                         // volume for cinematics

	"³nÅé",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"®M¥ÎÅÜ°Ê­È",                            // apply video mode changes

	"¥þ¿Ã¹õ",                               // fullscreen
// SCG[12/5/99]: 	"Stipple Alpha",                            // stipple alpha
	"¼v¤l",                                  // shadows
	"ÅX°Êµ{¦¡",                                   // video driver
	"¸ÑªR«×",                               // resolution
	"¿Ã¹õ¤j¤p",                              // screen size
	"«G«×",                               // brightness
	"§÷½è«~½è",                          // texture quality
	"¥i¨£ª¬ºA´Î",                    // status bar visibility
	"Ãú/¤ô²Ó½o«×",							// SCG[12/4/99]: 
	"¨B§L",									// difficulty level easy
	"ªZ¤h",									// difficulty level medium
	"±N­x",									// difficulty level hard

	"³·©M«B",							// cek[1-11-00]
	"Ãú"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"§¹¥þ",
	"´î¥b",
	"¥|¤À¤§¤@",
	"¤K¤À¤§¤@"
};


// weapon names
static char *tongue_weapons[] =
{
	"µL¼ußÓ",
	"©¸¯¥",
	"©¸¯¥¶ê¤ì",
	"¿z¤l",
	"¼u",
	"C4¤ÞÃz¾¹",
	" C4¬µßÓ¥]",
//	"¤ÑÅé",				// SCG[11/28/99]: 
//	"¤ÑÅé¨¾¿mºô",		// SCG[11/28/99]:	
	"¤j¤M",
//	"¤j¤M©ñ³v",			// SCG[11/28/99]: 
//	"¤j¤M¨¾¿mºô",		// SCG[11/28/99]: 
//	"¤j¤M¼C»R",	// SCG[11/28/99]: 
//	"¤j¤Mªñ¾Ô",			// SCG[11/28/99]: 
//	"¤j¤Mªþ¨­",		// SCG[11/28/99]: 
//	"¤j¤MÅÜ¤Æ",		// SCG[11/28/99]: 
	"¤j¤M­P©R",
	"¶ê½L",
	"²V¶ÃªÌ¤â®M",


//	"®¿«Â®Ú¤â¹qµ©",
	"®æ©Ô§J2020",
	"¤@¨Ç¤l¼u",
	"¶Â«Ò´µÅKÂñ",
	"Â÷¤lºj",
	"Â÷¤l¼ußÓ",					// SCG[11/28/99]: 
	"°Ê¤Oºj",
	"°Ê¤O",
	"±j¤Æ·Lªiºj",
	"±j¤Æ·Lªiºj",
//	"¦Ì¹F´µªº¶Àª÷¤§«ü",		// SCG[11/28/99]: 
//	"¶ÀÅKÄq",				// SCG[11/28/99]: 
	"®³º¸ªº´c¹Ú",
	"¤@¨Ç¦r·¤",					// SCG[11/28/99]: 
	"·s¬P¥ú§ô",
	"·s¬P¹q¦À",					// SCG[11/28/99]: 
	"¼¹µõºj",
	"µL·Ïºh¼u",				
	"¾_°Êªi",
	"¾_ªi²y",
	"³s®gºj",
	"³s®gºj¤l¼u",
	"ÅT§À³D­¸¼u",
	"ÅT§À³D­¸¼u",			// SCG[11/28/99]: 
	"»È¤ö",
	"ÄÅ¼uºj",
	"ÄÅ¼u",
	"¥v¶ð¿Õªº¸Ö¸`",
	"º²©¥¥Û",
	"·n¦²ªº¤Ó¶§",
	"·n¦²ªº¤Ó¶§",
//	"¶ð·æ¤§¹_",				// SCG[11/28/99]: 
	"®ü¯«¤T¤e´u",
	"¤T¤e´u¦yºÝ",
	"¬rº¡",
	"²´Ãè³D¬r",					// SCG[11/28/99]: 
	"¸U¹F§Jªº¿Ã¤õÅ]",
	"¿Ã¤õ¤§Å]",
	"©z´µ¤§²´",
	"¯«¯µ¤§²´",
	"²×·¥¥Ë´µ¤â"
};


static char *tongue_armors[] =
{
	"¶ì¿û¸Ë¥Ò",
	"¥ú¾Ç¸Ë¥Ò",
	"¥Õ»È¸Ë¥Ò",
	"¶Àª÷¸Ë¥Ò",
	"ÂêÃì¸Ë¥Ò",
	"¶Âª÷¿û¸Ë¥Ò",
	"³Í¤Ò°Ç¸Ë¥Ò",
	"µw¹³½¦¸Ë¥Ò"
};


// monster names
static char *tongue_monsters[] =
{
	"¾÷±ñ¾Ô°«ºµ",
	"¶Â¥Ç¤H",
	"¥¬ªi±o",
	"¾÷±ñ§¢§B¯S",
	"¦Ê¤Òªø",
	"¤TÀY¤ü",
	"­¸¦æÃì¯¥ªÌ",
	"ÀJ¹³¬W",
	"¾÷±ñ§JÃ¹§J´µ",
// SCG[12/5/99]: 	"¿W²´¥¨¤H",
	"¾÷±ñ¦º¤`²y",
	"¥½¤é½¿½»",
	"Às",
	"¸G¤H",
	"­S¤z®æ",
	"¦h¤ò¤H",
	"»s½bªÌ",
	"¾÷±ñ«Cµì±þ¤â",
	"®æÃ¹´µ¤ý",
	"ª÷³½",
	"®æ§Q­·",
	"³¾¨­¤k§¯",
	"¤Jº»ªÌ",
	"¤T®q¼v",
// NSS[3/6/00]:"¦Ì¿Õ´µ¤ý",
	"¦aº»¤õ¸tÃM¤h",

	"¹q¤O¤kªZ¯«",
	"¹êÅç«ÇµU¤l",
	"§C·Å¬ì§Þ",
	"¾÷±ñ¹p®gºj",
	"¯T¤H",
	"±ö§ù²ï",
// NSS[3/6/00]:	"¤ûÀY¤H",
	"¤T®q¦u½Ã",
	"®³º¸¦ºÆFªk®v",
	"½E¬Ì¦Ñ¹«",
	"¤ôºÞ¦Ñ¹«",
	"ªª®v",
	"¥Ç¤H",
	"¾÷±ñ­ì§Î¨¬",
	"ºë¯«¤ö",
	"¾÷±ñ¼É«ã±M®a5000",
	"¤õ½bÀ°",
	"®ü°\³¡¶¤´²¤õ½b§L",
	"»GÂÎ",
	"¦â¯T",
	"®ü°\³¡¶¤«ü´§©x",
// SCG[12/5/99]: 	"®ü°\¼ç¤ô¤Ò",
	"®ü°\³¡¶¤¶¤ªø",
	"®ü°\³¡¶¤´²¼uºj§L",
	"ÃT³½",
	"±°±þªÌ",

	"¾uÅ\",
	"½G¤u¤H",
	"­D¤u¤H",
	"¾÷±ñ¦Ãª«¹ÆÅo",
	"¤p»jµï",
	"»jµï",
	"¥v¶ð¦òªk®v",
	"­xÂå",
	"¤p°½",
	"¾÷±ñ°{¹q­¸¹v®gÀ»",
// SCG[12/5/99]: 	"¾÷±ñ©w­y§ðÀ»",
	"¯Q¯÷À°",
	"¥Õ¦â«R¸¸",
	"§Å®v",
	"¸U¹F§J§Å®v",
	"®üÅÃ",
	"¿Ã¤õÂÎ",

	"¿Ã¤õÅ]",
	"¬ü¬ö",
	"°­»î",
	"³ÂßÓ³½"
};


static char *tongue_world[] =
{
	"©|¦³¨ä¥L",                                 // triggers to go until sequence complete
	"¨BÆJ§¹¦¨",                         // trigger sequence completed

	"§A¥²¶·«ù¦³",                              // need a specific key
	"§A¥²¶·«ù¦³¥¿½TªºÆ_°Í¡I \n",               // wrong key in possession
	"¤£¯à±q³o­Ó¤è¦V¥´¶}\n",					  // SCG[11/1/99]: Can't be opened this way

	"§AÄ±±o¬r¯ÀÂ÷¶}¤F§Aªº¨t²Î¡C\n",    // poison expiring
	"§AªÍ¤¤ªº®ñ®ð´X¥G¥ÎºÉ¤F¡C\n",           // oxylung expiring
	"§AªºÀô¹Ò¦ç´X¥G¯Ó¶O¤F¡C\n",                // envirosuit expiring
	"ºñ¦âºëÆF«æ»Ý­¹ª«¡C\n",                 // megashield expiring
	"¼É«ã¤§²y´î®z¤F¡C\n",                   // wraith orb expiring
	"¤O¶q±j¤Æªº®ÄªGºCºCªº®ø¥¢¤F¡C\n",				  // power boost expiring
	"§ðÀ»±j¤Æªº®ÄªGºCºCªº®ø¥¢¤F¡C\n",                // attack boost expiring
	"³t«×±j¤Æªº®ÄªGºCºCªº®ø¥¢¤F¡C\n",                 // speed boost expiring
	"¼u¸õ±j¤Æªº®ÄªGºCºCªº®ø¥¢¤F¡C\n",             // jump boost expiring
	"Åé¤O±j¤Æªº®ÄªGºCºCªº®ø¥¢¤F¡C\n",              // health boost expiring

	"¤w¥[¤J¹CÀ¸",                            // player joined the game
	"Â÷¶}³o¤@Ãö",                           // player exited the level

	"®c¥»¥°",                              // Hiro Miyamoto's full name
	"°ª³t±j´Ë",                           // Superfly's full name
	"®ü¦Ñ­ì¬ü¬ö",                             // Mikiko's full name

	// SCG[10/18/99]: these next 5 had enums, but no strings. 
	"",								// SCG[10/18/99]: T_PLAYER_SELF_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_TIME_LIMIT_HIT
	"",								// SCG[10/18/99]: T_PLAYER_FRAG_LIMIT_HIT
	"",								// SCG[10/18/99]: T_TELL_CMD_UNKNOWN_PLAYER

	"Â÷¶}³o­Ó°Ï°ì¥²¶·¾Ö¦³\n",				// SCG[10/18/99]: no exiting level without sidekicks
	"­nÂ÷¶}§A¥²¶·¨ú±o\n",				// SCG[11/9/99]: No exiting without a key
	"¤¤¬r¡I\n",						// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"§A§ä¨ì¤@­Ó¯µ±K¡I \n",					// cek[1-3-00]
	"ªk¤O¾uÅ\ªº®ÄªGºCºCªº®ø¥¢¤F¡C\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s³Q%sµ¹¤Á¶}¤F¡C\n"
};

static char *tongue_deathmsg_self[] =
{
	"ªºÀY°©³Q¤Á¶}¤F¡C\n",                       // death, fell
	"³Q§l¤U¥h¡C\n",                             // death, drowned
	"ÅÜ±o©ö¸H¡C\n",                                // death, lava
	"ÅÜ­]±ø¤F¡C\n",                                 // death, slimed
	"©}ªA©ó¬r¯À¤U¡C\n",                    // death, poison
	"¤£³ô¥Í©Rªº§é¿i¡C\n",                             // death, killed self
	"¶Ç°e®ÉÃz±¼¤F¡C\n",                            // death, teleport
	"³QÀ£¦¨¤@¶ô»æ¡C\n",             // death, crushed

	"§]¤Uº¡¨{ªºC4¬µßÓ¡C\n"                       // death, C4
};

static char *tongue_deathmsg_weapon1[] =
{
	"%s³Q%s¶Ë¤F¦Û´L¡I\n",
	"%s³Q%s­«­«¥´¤F¤@´x¡A¤w¸g¦¨¬°¹Ð¤g¡I\n",
	"%s³Q%sÂ÷¤l¤Æ¤F¡I\n",
	"%s³Q%s§Ë±o¶Ã¤C¤KÁV¡I\n",
	"%s³Q%s¥ÎÂùºÞºjÅF¨ì¡I\n",
	"%s§¤¨ì¤F%s¥´¥X¨Óªº¤õ½b¡I\n",
	"%s³Q%s·n¨ì¥bªÅ¤¤¡I\n"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s§â%s·í°µ²×·¥­¸½L¨Ó¥á¡I\n",
	"%s¬½¬½ªº«r¤F%s¤@¤f¡I\n",
	"%s§â%sÅÜ¦¨¤õµK¦aº»¡I \n",
	"%s¹ï%s¥ÎÅKÂñ­«­«ªº¥´¤U¥h¡I\n",
	"%s§â%s¥´Ãz¤F¡I\n",
	"%s¸÷¦b¦t´µ­±«e¡I\n"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s³Q%s¹³¤õ»L¤@¼Ë¤Á¶}¡I\n",
	"%s§â%s¥´±¾¤F¡I\n",
	"%s §â%s·í¦aªO½ò¡I \n",
	"%s³Q%s¥Î¬y¬P¯{¨ì¡I\n",
	"%s§ì¨ì¤F%sªº°ªÀ£¹q¡I\n",
	"%s³Q%sªº¤â¬q§ì­Ó¥¿µÛ¡I \n"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s¦b%s­±«eÅS¥X¤@­Ó¸£³U¡I\n",
	"%s³Q%s¥´¦¨¼u¾_¯g¡I\n",
	"%s³Q%s¼¹µõ¡I\n",
	"%s³Q%s¥´¦¨¤@¹Îªd¡I\n",
	"%s³Q%s­á¦¨¦B¶ô¡I\n",
	"%s³Q%s¥´¬ï¤@­Ó¬}¡I\n",
	"%s³Q%s¯N³z³z¡I\n"
};

static char *tongue_scoreboard[] =
{
	"©m¦W",
	"±þ¼Ä¼Æ",
	"¦^¶Ç­È",
	"®É¶¡",
	"µo®g¼u¼Æ",
	"À»¤¤¼u¼Æ",
	"¦º¤`¦¸¼Æ",
	"¤À¼Æ"										// cek[1-22-00]
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"“ïˆÕ“x‚Ì‘I‘ð:",//"Select Difficulty:",           // difficulty header
	"¨B§L",	                   // newbie
	"ªZ¤h",                    // average
	"±N­x"                    // expert (or so they think!)
};




static char *tongue_skincolors[] =
{

	"ÂÅ",                         // blue
	"ºñ",                        // green
	"¬õ",                          // red
	"ª÷"                          // gold
};


static char *tongue_statbar[] =
{
	"¤O¶q",                        // skill 'power' label
	"§ðÀ»",                       // skill 'attack' label
	"³t«×",												// skill 'speed' label
	"¸õÅD",													// skill 'acrobatic' label
	"Åé¤O",											// skill 'vitality' label

	"¸Ë¥Ò",                        // armor label
	"¥Í©R",												// health label
	"¼ußÓ",													// ammo label
	"±þ¼Ä",												// kills label
	"Ãö¥d",  											// experience points label

	"ª@¯Å¡I ",                    // experience level up
	"¿ï¾Ü",                       // select skill
	"«ü©w§Þ¯àÂI¼Æ",                    // add point to skill

	"¸ü¤J..."                    // loading new level
};


static char *tongue_weapon_misc[] =
{
	"C4¼Ò²Õ²æÂ÷Ã­©wª¬ºA¡I \n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{
	"¨ú±o",	                        // 'get' item command
	"¹L¨Ó",                          // 'come here', 'come to me'
	"°±¯d",                           // 'stay' command, 'don't move'

	"§ðÀ»",                         // 'attack' command
	"«á°h",                       // 'back off', 'move away'

	"§Ú¯à®³"                      //  asking to pick up an item: "Can 			
ve the shotgun?"
};

static char *tongue_ctf[] =
{
	"¬õºX",
	"ÂÅºX",

	"¤£©ú¶¤¥î",

	"%s±þ¼Ä¼Æ",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"¬õ",
	"ÂÅ",
	"»Ì",
	"¿ûÅK",
	"ºñ",
	"¾í",
	"µµ",
	"¶À",

	"²{¦b¤À¼Æ¬O%s¡G%d¡A%s¡G%d\n",
	"§A®·®»¨ì%s¦¸¡I\n",
	"§A¶¤¥î¤¤ªº%s¤w¸g®»¨ì¤F%s¡I\n",
	"%s¨Ó¦Û%s¶¤¥î¡A±a¨«¤F§AªººX¤l¡I\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"§A®³¨ì¤F%s\n",
	"§A¶¤¥î¤¤ªº%s®»¨ì¤F%s\n",
	"%s°½¨«¤F§AªººX¤l\n",
	"%s¤w¸g·m¦^¨Ó¤F\n",



	"%d¬O¤@­ÓµL®Äªº¶¤¥î¼Æ¦r¡A½Ð«ö1©Î2\n",
	"§A²{¦b¬O%d¶¤¡]%s¡^\n",
	"§A¤w¸g¬O%d¶¤\n",
	"%s¥[¤J¤F%s¶¤\n",									// translate same as with the %s team above

	"%s¶¤¤w¸gÄ¹¤F¡I\n³Ì«áÁ`¤À¡V%s¡G%d¡A%s¡G%d\n",		// translate same as with the %s team above
	"³o³õ¹CÀ¸¥­¤â¡I\n",
	"®É¶¡¥ÎºÉ¡I\n",
	"¨ì¹F·mºX¦¸¼Æ¤W­­¡I\n"
};

static char *tongue_deathtag[] =
{
	"¬µ¼u",														// as in backpack
	"%s¬µ¼u",													// as in red pack
	"¨ì¹F¤À¼Æ¤W­­¡I\n",
	"§Aªº®É¶¡¥Î§¹¤F¡I\n",
	"§Aªº®É¶¡­è­è¥ÎºÉ¡I\n",

	"%s¨Ó¦Û%s¶¤¡A¨ú±o¤F%s¡I\n",				// translate using the '%s team' guidelines above

	"§Aªº¶¤¥î±o¨ì¤@¤À¡I\n",
	"§Aªº¶¤¥î±o¨ì%d¤À¡I\n",								// you get 2 points
	"%s¶¤¥î±o¨ì¤@¤À¡I\n",								// translate using the '%s team' guidelines above

	"%s¶¤¥î±o¨ì%d¤À¡I\n",							// The red team gets 2 points

	"§A±o¤À¤F¡I\n",
	"§A¶¤¥î¤¤ªº%s±o¨ì¤À¼Æ¡I\n",
	"%s¨Ó¦Û%s¶¤¥î¡A±o¨ì¤À¼Æ¡I\n"
};

