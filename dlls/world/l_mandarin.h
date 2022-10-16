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

	{"����ť1", K_MOUSE1},
	{"����ť2", K_MOUSE2},
	{"����ť3", K_MOUSE3},

	{"ҡ�˰�ť1", K_JOY1},
	{"ҡ�˰�ť2", K_JOY2},
	{"ҡ�˰�ť3", K_JOY3},
	{"ҡ�˰�ť4", K_JOY4},

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

	{"���������", K_MWHEELUP },
	{"���������", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"�ֺ�", ';'},	// because a raw semicolon seperates commands

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
	"��ȡ��",						// T_PICKUP_WEAPON	weapon pickup
	"����Ҫ",							// T_PICKUP_HEALTH	health pickup
	"�ܵ���������",					// T_PICKUP_ARMOR_GOOD	armor good pickup
	"ȡ����������",					// T_PICKUP_BOOST		boost pickup
	"��Ҫ���汦ʯ",						// T_PICKUP_SAVEGEM	pick up Save Gem
												
	"����",								// T_USEBOOST_POWER	power boost label
	"����",								// T_USEBOOST_ATTACK	attack boost label
	"�ٶ�",								// T_USEBOOST_SPEED	speed boost label
	"����",								// T_USEBOOST_ACRO	jump boost label
	"����",								// T_USEBOOST_VITA	health boost label
												
	"�÷��̻���",							// T_BOMB_EXPLODE			bomb explode
	"������и���ƿ�����",			// T_BOMB_NEED_BOTTLE		need bottle to mix ingredients
	"ȡ�õ���Ʒ��",						// T_BOMB_ITEM_REQUIRED1	ingredient required
	"������һ��ըҩ",					// T_BOMB_CREATED			bomb create
	"���ҵ�",							// T_BOMB_FOUND				bomb found, vowel
	"һ�����",						// T_BOMB_INGREDIENT_1		bomb ingredient 1
	"һ��ú̼",							// T_BOMB_INGREDIENT_2		bomb ingredient 2
	"һƿ��ʯ",						// T_BOMB_INGREDIENT_3		bomb ingredient 3
	"һ����ƿ",						// T_BOMB_BOTTLE			bomb bottle
												
	"���",								// T_BOMB_INGREDIENT_SHORT_1	bomb item, short description
	"ľ̿",								// T_BOMB_INGREDIENT_SHORT_2	bomb item, short description
	"��ʯ",							// T_BOMB_INGREDIENT_SHORT_3	bomb item, short description
												
	"���Ķ���",							// T_BOOK_READ		read book
	"���޷��ڴ�ʹ��",					// T_BOOK_NO_READ		no read book
	"���˵ķ�����",						// T_BOOK_WYNDRAX		Wyndrax spellbook
// SCG[12/5/99]: 	"������а��֮��",			// T_BOOK_EVIL		Satanic book of evil
												
	"��ʰ��",							// T_KEY_PICKUP		key pickup
	"Ĺ��Կ��",							// T_KEY_CRYPT		crypt key name
	"����֮Կ",						// T_KEY_WYNDRAX		Wyndrax key name
	"���ε�Կ�׿�",					// T_KEY_PRISONCELL	Prison cell key

	"��ɫ���ƿ�",						// T_KEY_RED_CARD				
	"��ɫ���ƿ�",					// T_KEY_BLUE_CARD				
	"��ɫ���ƿ�",					// T_KEY_GREEN_CARD				
	"��ɫ���ƿ�",					// T_KEY_YELLOW_CARD			
	"����Կ��ʯ",						// T_KEY_HEX_KEYSTONE			
	"�Ľ�Կ��ʯ",						// T_KEY_QUAD_KEYSTONE			
	"����Կ��ʯ",						// T_KEY_TRIGON_KEYSTONE		
	"��ǿ����",							// T_MEGASHIELD					
	"��������",							// T_SKULL_OF_INVINCIBILITY
	"�ⶾ��",						// T_POISON_ANTIDOTE
	"��������",								// T_DRACHMA
	"����Ž�",						// T_HORN
	"���֮Կ��һ���ݣ�A��",				// T_KEY_AEGIS_A
	"���֮Կ��һ���ݣ�E��",				// T_KEY_AEGIS_E
	"���֮Կ��һ���ݣ�G��",				// T_KEY_AEGIS_G
	"���֮Կ��һ���ݣ�I��",				// T_KEY_AEGIS_I
	"���֮Կ��һ���ݣ�S��",				// T_KEY_AEGIS_S
	"����������Ƭ",					// T_PURIFIER_SHARD
	"��ɫ����",							// T_BLACK_CHEST
	"����֮��",				// T_RING_OF_FIRE
	"����������֮��",			// T_RING_OF_UNDEAD
	"������֮��",			// T_RING_OF_LIGHTNING
	"������",							// T_ENVIROSUIT

	"�ظ�����",							// T_HEALTH_KIT
	"�ƽ����",							// T_GOLDENSOUL
	"�����",							// T_WRAITHORB,

	"���汦ʯ",								// T_SAVEGEM
	"��û�д��汦ʯ",					//cek[12-13-99]
	"����֮ƿ",							// cek[1-3-00]

	// cek[2-2-00]: altered the following *******************txtP************************
	"������",
	"������",
	"����ƿ",
	"����֮��",
	"������",
	"����ⶾ���ǿյġ�",

// SCG[2/8/00]: messages for completed artifacts
	"���ҵ�������ܵķ����ˡ�",
	"���ҵ�ȫ������Կ��֮ʯ��",
	"����Ϻþ�����"
};


// menu-related
static char *tongue_menu[] =
{
	"ǰ��",                                  // key bind to Move Forward
	"����",                                 // key bind to Move Backward
	"��̤",                                // key bind to Step Left
	"��̤",                               // key bind to Step Right
	"����",                                   // key bind to Attack
	"��Ծ",                                     // key bind to Jump/Up
	"ʹ�ã�����",                              // key bind to Use/Operate object
	"����",                                   // key bind to Crouch/Down
	"��һ������",                              // key bind to Next Weapon
	"��һ������",                              // key bind to Prev Weapon
	"��ת",                                // key bind to Turn Left
	"��ת",                               // key bind to Turn Right
	"��",                                      // key bind to Run
	"����",                                 // key bind to  Sidestep
	"���Ͽ�",                                  // key bind to Look Up
	"���¿�",                                // key bind to Look Down
	"���м�",                              // key bind to Center View
	"����ۿ�",                                // key bind to Mouse Look
	"�����ۿ�",                                  // key bind to Keyboard Look
	"�ɼ���Χ����",                             // key bind to Screen size up
	"�ɼ���Χ�½�",                           // key bind to Screen size down
	"��ʾ���н���",                            // key bind to Show All Huds
	"������Ʒ��",                                // key bind to Inventory Open
	"ʹ����Ʒ",                            // key bind to Use Item
	//  "������Ʒ",                                // key bind to Drop Item
	"��һ����Ʒ",                                // key bind to Next Item
	"��һ����Ʒ",                                // key bind to Prev Item
	"�л�����",                            // key bind to Swap Sidekick
	"��һ��ָ��",                             // key bind to Command Next
	"��һ��ָ��",                             // key bind to Command Prev
	"ʹ�õ�ָ��",	                            // key bind to Command Apply				// cek[1-5-00] changed from Apply to Use

	ARROW_LEFT_STRING  "��",                 // left arrow key
	ARROW_UP_STRING    "��",                   // up arrow key
	ARROW_DOWN_STRING  "��",                 // down arrow key
	ARROW_RIGHT_STRING "��",                // right arrow key
	"--",                                       // no bind setting for this key

	"��ѡ��Enter�趨��DELɾ��",     // set/delete key
	"��ѡ��Enterѡ��",                 // select key
	"�����趨��ESC�ж�",             // set/abort

	"Ԥ��",                                 // default button

	"��",                                     // no mouse bindings
	"����/����",                          // mouse "move left and right" setting
	"��ת/��ת",                          // mouse "turn left and right" setting
	"����/�¿�",                             // mouse "look up and down" setting
	"ǰ��/����",                           // mouse "move forward and back" setting
	"��Y�ᷴ��",                           // reverse mouse axis
	"����ۿ�",                               // Mouse-Look
	"�ۿ�����",                              // Mouse-Look
	"X��",                                   // x axis
	"Y��",                                   // y axis
	"��ť1",                                 // button 1 label
	"��ť2",                                 // button 2 label
	"��ť3",                                 // button 3 label
	"ѡ��һ��ָ�ESCȡ��",          // select a command, ESCape to cancel
	"��ת/��ת",                                 // turn left and right, abbreviated
	"��/�Һ���",                               // strafe left and right, abbreviated
	"����/�¿�",                                 // look up and down, abbreviated
	"ǰ��/����",                                 // move forward and back, abbreviated
	"ˮƽ������",                   // mouse sensitivity for x-axis				//cek[12-13-99]
	"��ֱ������",                     // mouse sensitivity for y-axis				//cek[12-13-99]
	"�趨����ѡ�",                 // configure mouse options

	"��������",                               // 'deathmatch' game
	"������ɱ",                                 // 'deathtag' game
	"����",                              // 'cooperative' game

	"��",                                     // easy difficulty
	"�е�",                                   // medium difficulty
	"����",                                     // hard difficulty

	"��Ѱ������·��Ϸ",					// SCG[12/1/99]: 
	"��Ѱ��������Ϸ",						// SCG[12/1/99]: 
	"��ʼ������Ϸ",					// SCG[12/1/99]: 

	"������·",                                      // local area network game
	"������·",                                 // internet game

	"����",                                     // join server
	"����",                                     // host new server
	"���� " ARROW_DOWN_STRING,                   // add address
	"����",                                  // refresh network games
	"�趨",                                    // player setup

	//"������",                               // handedness label
	//"����",                                     // handedness, left
	//"˫��",                                    // handedness, center (ambi-dextrous)
	//"����",                                    // handedness, right

	"��������",                             // connection speed label
	"28.8k",                                    // connection type 1
	"56.6k",                                    // connection type 2
	"ISDN",										// connection type 3
	"�������ݻ�/DSL",							    // connection type 4
	"T1/������·",                                   // connection type 5
	"ʹ�����Զ�",                             // connection type 6

	"ʱ������",								// SCG[12/2/99]: 
	"������ɫ",							// SCG[12/2/99]: 
	"��ʼ�½�",							// SCG[12/2/99]: 
	"ѡ�����",								// SCG[12/1/99]: 
	"CTF����",								// SCG[12/1/99]: 
	"��1����ɫ",								// SCG[12/1/99]: 
	"��2����ɫ",								// SCG[12/1/99]: 
	"��������",								// SCG[12/1/99]: 
	"ɱ������",								// SCG[12/1/99]: 
	"�ؿ�����",								// level limit
	"��ʼ�½�",							// SCG[12/2/99]: 
	"��Ϸ��ͼ",									// SCG[12/1/99]: 
	"��Ϸģʽ",								// game mode (deathmatch,deathtag,co-op)
	"���ܵȼ�",								// skill level
	"ʱ������",								// time limit
	"�������",								// maximum players
	"����",								// character
	"������ɫ",								// base color
	"��ʼ��",									// start multiplayer game
												
	"����ͣ��",								// weapons remain
	"��Ʒ����",							// items rematerialize
	"Զ������",								// spawn farthest point from players
	"�����뿪",							// allow exiting of the level
	"��ͬ��ͼ",									// same map
	"��������",							// force respawn
	"��������",							// players take falling damage
	"����ǿ��",							// instant power-ups
	"����ǿ��",								// allow power-ups
	"��������",								// allow health kits
	"����װ��",								// allow armor
	"��ҩ����",							// unlimited ammunition
	"�ӽ�����",								// fixed field-of-vision
	"�����Ϸ",									// team-play
	"�˺�����",							// hurt teammates
	"�����л�",								// fast weapon switch
	"�Ų�����",								// footsteps audible
	"����ץ��",								// allow hook
	"ɢ��ǹ�ӵ�",							// show shotgun shells?
												
	"�ŷ���IPλַ",						// server internet provider address
	"�Ҳ�����������Ϸ",						// no local games found
	"��ʼ��ͼ",								// starting map
	"�ŷ�������",								// server name
	"��������",								// player name
	"��������",								// team name
												
	"�����и���",						// confirm quit
												
	"��",										// yes, acknowledgement
	"��",										 // no, acknowledgement
	"������Ϸ",								// cek[1-3-00] changed from 'save' to 'save game'

	"�����趨",								//cek[12-6-99]
	"�����趨",								//cek[12-6-99]
	"ɾ���趨",							//cek[12-6-99]
	"ѡ���趨����",				//cek[12-6-99]
	"ɾ��ѡ�񵵰���",						//cek[12-6-99]
												
	"������Ϸ",								//cek[12-6-99]
	"װ��",									//cek[12-6-99]
	"����",									//cek[12-6-99]
	"�ؿ�",									//cek[12-6-99]
	"����",									//cek[12-6-99]
	"����",									//cek[12-6-99]
	"ʱ��",										//cek[12-6-99]
	"�ܼ�",									//cek[12-6-99]
	"�½�",									//cek[12-6-99]

	"ʹ��3D����",								//cek[12-6-99]

	"�Ѵ���",									//cek[12-7-99]
	"������",									//cek[12-7-99]
	"��ɾ��",									//cek[12-7-99]

	"��������",								//cek[12-7-99]
	"��������",									//cek[12-7-99]

	"��������",								//cek[12-7-99]

	"�ر�ALT-TAB����",						//cek[12-7-99]
	"Ѫ�Ⱥ���ɱ",						//cek[12-7-99]
	"��������",							//cek[12-7-99]
	"�Զ��л�����",						//cek[12-7-99]

	"ǿ��",								//cek[12-7-99] cek[12-13-99]
	"��������",							//cek[12-7-99]
	"����",								// SCG[1/4/00]: 


	"ǰ��",									//cek[12-9-99]
	"����",										//cek[12-9-99]
	"��",										//cek[12-9-99]
	"�Ƕ�",									//cek[12-9-99]
	"ƫҡ",										//cek[12-9-99]
	"ʹ��ҡ��",								//cek[12-9-99]
	"�趨������",							//cek[12-9-99]
	"�趨ҡ��ѡ��",				//cek[12-9-99]
	"�趨�˰�ť����",			//cek[12-9-99]
	"����ҡ�˰�ť����趨",		//cek[12-9-99]
	"����",								//cek[12-9-99]
	"������",								//cek[12-9-99]

	"������",							//cek[12-10-99]
	"�ط�",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"�Զ�",										//cek[12-10-99]
	"��1��",									//cek[12-10-99]
	"��2��",									//cek[12-10-99]

	"���ż�鶯��",							//cek[12-13-99]
	"ʹ�ü���ϵͳ",							//cek[12-13-99]
	"�����ٶ�",								//cek[12-13-99]

	"ⷰ�ȡ��",								// cek[1-3-00]
	"ⷰ����",							// cek[1-3-00]
	"ⷰ鹥��",							// cek[1-3-00]
	"ⷰ�����",							// cek[1-3-00]
	"ⷰ�ͣ��",							// cek[1-3-00]

	"ѡ������%d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"�����л�",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"��һ����",									// cek[1-5-00]  -- similar in meaning.
	"��һ����",									// cek[1-5-00]
	"ʹ�ý���",									// cek[1-5-00]

	"��̸",										// cek[1-11-00]
	"�Ʒְ�",								// SCG[1/13/00]: 
	"С�ӽ�̸",								// cek[1-22-00]
	"%s�Ѿ�����Ϊ%s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"�û���",									// cek[1-22-00]

	"ǰ������",								// cek[2-1-00]
	"����Ⱥ",								// cek[2-1-00]
	"������",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"�������뿪��\n������Mplayer�ŷ������������",	// cek[2-1-00]
	"�뿴���š���̸�����ۺͼƷ�\nǰ��www.daikatananews.com��\n������һ����ִ�������ҳ�������",			// cek[2-1-00]

	"���� %d",
	"ȡ��",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"ǰ��www.planetdaikatana.com��\n������һ����ִ�������ҳ�������",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"�ܲ��ж�",                               // always run
	"�������ۿ�",                               // look-spring
	"���ơ��ۿ�",                               // look-strafe
	"�Զ���׼",                           // auto-targeting
	"׼��",                                // crosshair
	"�����ɼ�",                           // weapon visible
	"��Ļ",                                // subtitles

	"��Ч����",                      // volume for sound fx
	"��������",                             // volume for music
	"��������",                         // volume for cinematics

	"����",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"���ñ䶯ֵ",                            // apply video mode changes

	"ȫөĻ",                               // fullscreen
// SCG[12/5/99]: 	"Stipple Alpha",                            // stipple alpha
	"Ӱ��",                                  // shadows
	"������ʽ",                                   // video driver
	"������",                               // resolution
	"өĻ��С",                              // screen size
	"����",                               // brightness
	"����Ʒ��",                          // texture quality
	"�ɼ�״̬��",                    // status bar visibility
	"��/ˮϸ�¶�",							// SCG[12/4/99]: 
	"����",									// difficulty level easy
	"��ʿ",									// difficulty level medium
	"����",									// difficulty level hard

	"ѩ����",							// cek[1-11-00]
	"��"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"��ȫ",
	"����",
	"�ķ�֮һ",
	"�˷�֮һ"
};


// weapon names
static char *tongue_weapons[] =
{
	"�޵�ҩ",
	"����",
	"����Բľ",
	"ɸ��",
	"��",
	"C4������",
	" C4ըҩ��",
//	"����",				// SCG[11/28/99]: 
//	"���������",		// SCG[11/28/99]:	
	"��",
//	"�󵶷���",			// SCG[11/28/99]: 
//	"�󵶷�����",		// SCG[11/28/99]: 
//	"�󵶽���",	// SCG[11/28/99]: 
//	"�󵶽�ս",			// SCG[11/28/99]: 
//	"�󵶸���",		// SCG[11/28/99]: 
//	"�󵶱仯",		// SCG[11/28/99]: 
	"������",
	"Բ��",
	"����������",


//	"Ų�����ֵ�Ͳ",
	"������2020",
	"һЩ�ӵ�",
	"�ڵ�˹����",
	"����ǹ",
	"���ӵ�ҩ",					// SCG[11/28/99]: 
	"����ǹ",
	"����",
	"ǿ��΢��ǹ",
	"ǿ��΢��ǹ",
//	"�״�˹�Ļƽ�ָ֮",		// SCG[11/28/99]: 
//	"������",				// SCG[11/28/99]: 
	"�ö��Ķ���",
	"һЩ��Ш",					// SCG[11/28/99]: 
	"���ǹ���",
	"���ǵ��",					// SCG[11/28/99]: 
	"˺��ǹ",
	"������",				
	"�𶯲�",
	"����",
	"����ǹ",
	"����ǹ�ӵ�",
	"��β�߷ɵ�",
	"��β�߷ɵ�",			// SCG[11/28/99]: 
	"��צ",
	"����ǹ",
	"����",
	"ʷ��ŵ��ʫ��",
	"����ʯ",
	"ҡҷ��̫��",
	"ҡҷ��̫��",
//	"��ɪ֮��",				// SCG[11/28/99]: 
	"���������",
	"����ꪼ��",
	"����",
	"�۾��߶�",					// SCG[11/28/99]: 
	"���˵�ө��ħ",
	"ө��֮ħ",
	"��˹֮��",
	"����֮��",
	"�ռ���˹��"
};


static char *tongue_armors[] =
{
	"�ܸ�װ��",
	"��ѧװ��",
	"����װ��",
	"�ƽ�װ��",
	"����װ��",
	"�ڽ��װ��",
	"������װ��",
	"Ӳ��װ��"
};


// monster names
static char *tongue_monsters[] =
{
	"��еս����",
	"�ڷ���",
	"������",
	"��е������",
	"�ٷ�",
	"��ͷȮ",
	"����������",
	"������",
	"��е���޿�˹",
// SCG[12/5/99]: 	"���۾���",
	"��е������",
	"ĩ������",
	"��",
	"����",
	"���ɸ�",
	"��ë��",
	"�Ƽ���",
	"��е����ɱ��",
	"����˹��",
	"����",
	"������",
	"����Ů��",
	"������",
	"����Ӱ",
// NSS[3/6/00]:"��ŵ˹��",
	"������ʥ��ʿ",

	"����Ů����",
	"ʵ���Һ���",
	"���¿Ƽ�",
	"��е����ǹ",
	"����",
	"÷��ɯ",
// NSS[3/6/00]:	"ţͷ��",
	"��������",
	"�ö����鷨ʦ",
	"��������",
	"ˮ������",
	"��ʦ",
	"����",
	"��еԭ����",
	"����צ",
	"��е��ŭר��5000",
	"�����",
	"��������ɢ�����",
	"����",
	"ɫ��",
	"��������ָ�ӹ�",
// SCG[12/5/99]: 	"����Ǳˮ��",
	"�������Ӷӳ�",
	"��������ɢ��ǹ��",
	"����",
	"��ɱ��",

	"����",
	"�ݹ���",
	"�ֹ���",
	"��е�������",
	"С֩��",
	"֩��",
	"ʷ����ʦ",
	"��ҽ",
	"С͵",
	"��е����ɰ����",
// SCG[12/5/99]: 	"��е���칥��",
	"���Ȱ�",
	"��ɫ��²",
	"��ʦ",
	"������ʦ",
	"��Ÿ",
	"ө���",

	"ө��ħ",
	"����",
	"���",
	"��ҩ��"
};


static char *tongue_world[] =
{
	"��������",                                 // triggers to go until sequence complete
	"�������",                         // trigger sequence completed

	"��������",                              // need a specific key
	"����������ȷ��Կ�ס� \n",               // wrong key in possession
	"���ܴ���������\n",					  // SCG[11/1/99]: Can't be opened this way

	"����ö����뿪�����ϵͳ��\n",    // poison expiring
	"����е����������þ��ˡ�\n",           // oxylung expiring
	"��Ļ����¼����ķ��ˡ�\n",                // envirosuit expiring
	"��ɫ���鼱��ʳ�\n",                 // megashield expiring
	"��ŭ֮������ˡ�\n",                   // wraith orb expiring
	"����ǿ����Ч����������ʧ�ˡ�\n",				  // power boost expiring
	"����ǿ����Ч����������ʧ�ˡ�\n",                // attack boost expiring
	"�ٶ�ǿ����Ч����������ʧ�ˡ�\n",                 // speed boost expiring
	"����ǿ����Ч����������ʧ�ˡ�\n",             // jump boost expiring
	"����ǿ����Ч����������ʧ�ˡ�\n",              // health boost expiring

	"�Ѽ�����Ϸ",                            // player joined the game
	"�뿪��һ��",                           // player exited the level

	"������",                              // Hiro Miyamoto's full name
	"����ǿɭ",                           // Superfly's full name
	"����ԭ����",                             // Mikiko's full name

	// SCG[10/18/99]: these next 5 had enums, but no strings. 
	"",								// SCG[10/18/99]: T_PLAYER_SELF_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_TIME_LIMIT_HIT
	"",								// SCG[10/18/99]: T_PLAYER_FRAG_LIMIT_HIT
	"",								// SCG[10/18/99]: T_TELL_CMD_UNKNOWN_PLAYER

	"�뿪����������ӵ��\n",				// SCG[10/18/99]: no exiting level without sidekicks
	"Ҫ�뿪�����ȡ��\n",				// SCG[11/9/99]: No exiting without a key
	"�ж���\n",						// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"���ҵ�һ�����ܡ� \n",					// cek[1-3-00]
	"�������õ�Ч����������ʧ�ˡ�\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s��%s���п��ˡ�\n"
};

static char *tongue_deathmsg_self[] =
{
	"��ͷ�Ǳ��п��ˡ�\n",                       // death, fell
	"������ȥ��\n",                             // death, drowned
	"������顡\n",                                // death, lava
	"�������ˡ�\n",                                 // death, slimed
	"����춶����¡�\n",                    // death, poison
	"������������ĥ��\n",                             // death, killed self
	"����ʱ�����ˡ�\n",                            // death, teleport
	"��ѹ��һ�����\n",             // death, crushed

	"�������ǵ�C4ըҩ��\n"                       // death, C4
};

static char *tongue_deathmsg_weapon1[] =
{
	"%s��%s��������\n",
	"%s��%s���ش���һ�ơ��Ѿ���Ϊ������\n",
	"%s��%s���ӻ��ˡ�\n",
	"%s��%sŪ�����߰��㡡\n",
	"%s��%s��˫��ǹ�䵽��\n",
	"%s������%s������Ļ����\n",
	"%s��%sҡ������С�\n"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s��%s�����ռ�����������\n",
	"%s�ݺݵ�ҧ��%sһ�ڡ�\n",
	"%s��%s��ɻ�������� \n",
	"%s��%s���������صĴ���ȥ��\n",
	"%s��%s���ˡ�\n",
	"%s������˹��ǰ��\n"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s��%s�����һ���п���\n",
	"%s��%s����ˡ�\n",
	"%s ��%s���ذ�ȡ� \n",
	"%s��%s�������ҵ���\n",
	"%sץ����%s�ĸ�ѹ�硡\n",
	"%s��%s���ֶ�ץ�������� \n"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s��%s��ǰ¶��һ���Դ���\n",
	"%s��%s��ɵ���֢��\n",
	"%s��%s˺�ѡ�\n",
	"%s��%s���һ���ࡡ\n",
	"%s��%s���ɱ��顡\n",
	"%s��%s��һ������\n",
	"%s��%s��͸͸��\n"
};

static char *tongue_scoreboard[] =
{
	"����",
	"ɱ����",
	"�ش�ֵ",
	"ʱ��",
	"���䵯��",
	"���е���",
	"��������",
	"����"										// cek[1-22-00]
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"��Փx�̑I��:",//"Select Difficulty:",           // difficulty header
	"����",	                   // newbie
	"��ʿ",                    // average
	"����"                    // expert (or so they think!)
};




static char *tongue_skincolors[] =
{

	"��",                         // blue
	"��",                        // green
	"��",                          // red
	"��"                          // gold
};


static char *tongue_statbar[] =
{
	"����",                        // skill 'power' label
	"����",                       // skill 'attack' label
	"�ٶ�",												// skill 'speed' label
	"��Ծ",													// skill 'acrobatic' label
	"����",											// skill 'vitality' label

	"װ��",                        // armor label
	"����",												// health label
	"��ҩ",													// ammo label
	"ɱ��",												// kills label
	"�ؿ�",  											// experience points label

	"������ ",                    // experience level up
	"ѡ��",                       // select skill
	"ָ�����ܵ���",                    // add point to skill

	"����..."                    // loading new level
};


static char *tongue_weapon_misc[] =
{
	"C4ģ�������ȶ�״̬�� \n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{
	"ȡ��",	                        // 'get' item command
	"����",                          // 'come here', 'come to me'
	"ͣ��",                           // 'stay' command, 'don't move'

	"����",                         // 'attack' command
	"����",                       // 'back off', 'move away'

	"������"                      //  asking to pick up an item: "Can 			
//ve the shotgun?"
};

static char *tongue_ctf[] =
{
	"����",
	"����",

	"��������",

	"%sɱ����",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"��",
	"��",
	"��",
	"����",
	"��",
	"��",
	"��",
	"��",

	"���ڷ�����%s��%d��%s��%d\n",
	"�㲶׽��%s�Ρ�\n",
	"������е�%s�Ѿ�׽����%s��\n",
	"%s����%s���顡������������ӡ�\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"���õ���%s\n",
	"������е�%s׽����%s\n",
	"%s͵�����������\n",
	"%s�Ѿ���������\n",



	"%d��һ����Ч�Ķ������֡��밴1��2\n",
	"��������%d�ӣ�%s��\n",
	"���Ѿ���%d��\n",
	"%s������%s��\n",									// translate same as with the %s team above

	"%s���Ѿ�Ӯ�ˡ�\n�����ܷ֡�%s��%d��%s��%d\n",		// translate same as with the %s team above
	"�ⳡ��Ϸƽ�֡�\n",
	"ʱ���þ���\n",
	"��������������ޡ�\n"
};

static char *tongue_deathtag[] =
{
	"ը��",														// as in backpack
	"%sը��",													// as in red pack
	"����������ޡ�\n",
	"���ʱ�������ˡ�\n",
	"���ʱ��ո��þ���\n",

	"%s����%s�ӡ�ȡ����%s��\n",				// translate using the '%s team' guidelines above

	"��Ķ���õ�һ�֡�\n",
	"��Ķ���õ�%d�֡�\n",								// you get 2 points
	"%s����õ�һ�֡�\n",								// translate using the '%s team' guidelines above

	"%s����õ�%d�֡�\n",							// The red team gets 2 points

	"��÷��ˡ�\n",
	"������е�%s�õ�������\n",
	"%s����%s���顡�õ�������\n"
};

// --------------------new-------------------------
static char *tongue_save_error[] =
{
	"You must be running a local game to save!\n",
	"You must be in a game to save!\n",
	"You can not save in deathmatch!\n",
	"You can not save during a cinematic!\n",
	"You can not save while dead!\n",
	"You can not save during intermission!\n",
	"You can not save here!\n"
};
