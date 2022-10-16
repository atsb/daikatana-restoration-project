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

	{"滑鼠按钮1", K_MOUSE1},
	{"滑鼠按钮2", K_MOUSE2},
	{"滑鼠按钮3", K_MOUSE3},

	{"摇杆按钮1", K_JOY1},
	{"摇杆按钮2", K_JOY2},
	{"摇杆按钮3", K_JOY3},
	{"摇杆按钮4", K_JOY4},

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

	{"滑鼠滚轮上", K_MWHEELUP },
	{"滑鼠滚轮下", K_MWHEELDOWN },

	{"PAUSE", K_PAUSE},

	{"分号", ';'},	// because a raw semicolon seperates commands

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
	"你取得",						// T_PICKUP_WEAPON	weapon pickup
	"你需要",							// T_PICKUP_HEALTH	health pickup
	"受到保护来自",					// T_PICKUP_ARMOR_GOOD	armor good pickup
	"取得能量来自",					// T_PICKUP_BOOST		boost pickup
	"需要储存宝石",						// T_PICKUP_SAVEGEM	pick up Save Gem
												
	"力量",								// T_USEBOOST_POWER	power boost label
	"攻击",								// T_USEBOOST_ATTACK	attack boost label
	"速度",								// T_USEBOOST_SPEED	speed boost label
	"弹跳",								// T_USEBOOST_ACRO	jump boost label
	"体力",								// T_USEBOOST_VITA	health boost label
												
	"该放烟火了",							// T_BOMB_EXPLODE			bomb explode
	"你必须有个空瓶来混合",			// T_BOMB_NEED_BOTTLE		need bottle to mix ingredients
	"取得的物品　",						// T_BOMB_ITEM_REQUIRED1	ingredient required
	"你作了一袋炸药",					// T_BOMB_CREATED			bomb create
	"你找到",							// T_BOMB_FOUND				bomb found, vowel
	"一袋硫磺",						// T_BOMB_INGREDIENT_1		bomb ingredient 1
	"一块煤碳",							// T_BOMB_INGREDIENT_2		bomb ingredient 2
	"一瓶硝石",						// T_BOMB_INGREDIENT_3		bomb ingredient 3
	"一个空瓶",						// T_BOMB_BOTTLE			bomb bottle
												
	"硫磺",								// T_BOMB_INGREDIENT_SHORT_1	bomb item, short description
	"木炭",								// T_BOMB_INGREDIENT_SHORT_2	bomb item, short description
	"硝石",							// T_BOMB_INGREDIENT_SHORT_3	bomb item, short description
												
	"你阅读到",							// T_BOOK_READ		read book
	"你无法在此使用",					// T_BOOK_NO_READ		no read book
	"万达克的法术书",						// T_BOOK_WYNDRAX		Wyndrax spellbook
// SCG[12/5/99]: 	"撒旦的邪恶之书",			// T_BOOK_EVIL		Satanic book of evil
												
	"你拾起",							// T_KEY_PICKUP		key pickup
	"墓地钥匙",							// T_KEY_CRYPT		crypt key name
	"万达克之钥",						// T_KEY_WYNDRAX		Wyndrax key name
	"监牢的钥匙卡",					// T_KEY_PRISONCELL	Prison cell key

	"红色控制卡",						// T_KEY_RED_CARD				
	"蓝色控制卡",					// T_KEY_BLUE_CARD				
	"绿色控制卡",					// T_KEY_GREEN_CARD				
	"黄色控制卡",					// T_KEY_YELLOW_CARD			
	"六角钥匙石",						// T_KEY_HEX_KEYSTONE			
	"四角钥匙石",						// T_KEY_QUAD_KEYSTONE			
	"三角钥匙石",						// T_KEY_TRIGON_KEYSTONE		
	"超强护盾",							// T_MEGASHIELD					
	"法力骷髅",							// T_SKULL_OF_INVINCIBILITY
	"解毒剂",						// T_POISON_ANTIDOTE
	"德拉克马",								// T_DRACHMA
	"船夫号角",						// T_HORN
	"神盾之钥的一部份（A）",				// T_KEY_AEGIS_A
	"神盾之钥的一部份（E）",				// T_KEY_AEGIS_E
	"神盾之钥的一部份（G）",				// T_KEY_AEGIS_G
	"神盾之钥的一部份（I）",				// T_KEY_AEGIS_I
	"神盾之钥的一部份（S）",				// T_KEY_AEGIS_S
	"净化器的碎片",					// T_PURIFIER_SHARD
	"黑色箱子",							// T_BLACK_CHEST
	"抗火之戒",				// T_RING_OF_FIRE
	"抗不死生物之戒",			// T_RING_OF_UNDEAD
	"抗闪电之戒",			// T_RING_OF_LIGHTNING
	"环境服",							// T_ENVIROSUIT

	"回复工具",							// T_HEALTH_KIT
	"黄金灵魂",							// T_GOLDENSOUL
	"灵魂球",							// T_WRAITHORB,

	"储存宝石",								// T_SAVEGEM
	"你没有储存宝石",					//cek[12-13-99]
	"生命之瓶",							// cek[1-3-00]

	// cek[2-2-00]: altered the following *******************txtP************************
	"生命包",
	"生命罐",
	"生命瓶",
	"生命之箱",
	"生命盒",
	"这个解毒剂是空的　",

// SCG[2/8/00]: messages for completed artifacts
	"你找到所有神盾的符号了　",
	"你找到全部三个钥匙之石　",
	"你组合好净化器"
};


// menu-related
static char *tongue_menu[] =
{
	"前进",                                  // key bind to Move Forward
	"後退",                                 // key bind to Move Backward
	"左踏",                                // key bind to Step Left
	"右踏",                               // key bind to Step Right
	"攻击",                                   // key bind to Attack
	"跳跃",                                     // key bind to Jump/Up
	"使用／操作",                              // key bind to Use/Operate object
	"蹲下",                                   // key bind to Crouch/Down
	"下一项武器",                              // key bind to Next Weapon
	"上一项武器",                              // key bind to Prev Weapon
	"左转",                                // key bind to Turn Left
	"右转",                               // key bind to Turn Right
	"跑",                                      // key bind to Run
	"横移",                                 // key bind to  Sidestep
	"向上看",                                  // key bind to Look Up
	"向下看",                                // key bind to Look Down
	"看中间",                              // key bind to Center View
	"滑鼠观看",                                // key bind to Mouse Look
	"按键观看",                                  // key bind to Keyboard Look
	"可见范围增加",                             // key bind to Screen size up
	"可见范围下降",                           // key bind to Screen size down
	"显示所有介面",                            // key bind to Show All Huds
	"开启物品栏",                                // key bind to Inventory Open
	"使用物品",                            // key bind to Use Item
	//  "丢下物品",                                // key bind to Drop Item
	"下一项物品",                                // key bind to Next Item
	"上一项物品",                                // key bind to Prev Item
	"切换侧踢",                            // key bind to Swap Sidekick
	"下一项指令",                             // key bind to Command Next
	"上一项指令",                             // key bind to Command Prev
	"使用的指令",	                            // key bind to Command Apply				// cek[1-5-00] changed from Apply to Use

	ARROW_LEFT_STRING  "左",                 // left arrow key
	ARROW_UP_STRING    "上",                   // up arrow key
	ARROW_DOWN_STRING  "下",                 // down arrow key
	ARROW_RIGHT_STRING "右",                // right arrow key
	"--",                                       // no bind setting for this key

	"点选或按Enter设定　DEL删除",     // set/delete key
	"点选或按Enter选择",                 // select key
	"按键设定　ESC中断",             // set/abort

	"预设",                                 // default button

	"无",                                     // no mouse bindings
	"左移/右移",                          // mouse "move left and right" setting
	"左转/右转",                          // mouse "turn left and right" setting
	"向上/下看",                             // mouse "look up and down" setting
	"前进/後退",                           // mouse "move forward and back" setting
	"将Y轴反向",                           // reverse mouse axis
	"滑鼠观看",                               // Mouse-Look
	"观看横移",                              // Mouse-Look
	"X轴",                                   // x axis
	"Y轴",                                   // y axis
	"按钮1",                                 // button 1 label
	"按钮2",                                 // button 2 label
	"按钮3",                                 // button 3 label
	"选择一个指令　ESC取消",          // select a command, ESCape to cancel
	"左转/右转",                                 // turn left and right, abbreviated
	"左/右横移",                               // strafe left and right, abbreviated
	"向上/下看",                                 // look up and down, abbreviated
	"前进/後退",                                 // move forward and back, abbreviated
	"水平灵敏度",                   // mouse sensitivity for x-axis				//cek[12-13-99]
	"垂直灵敏度",                     // mouse sensitivity for y-axis				//cek[12-13-99]
	"设定滑鼠选项　",                 // configure mouse options

	"死亡竞赛",                               // 'deathmatch' game
	"死亡刺杀",                                 // 'deathtag' game
	"合作",                              // 'cooperative' game

	"简单",                                     // easy difficulty
	"中等",                                   // medium difficulty
	"困难",                                     // hard difficulty

	"搜寻网际网路游戏",					// SCG[12/1/99]: 
	"搜寻区域性游戏",						// SCG[12/1/99]: 
	"开始多人游戏",					// SCG[12/1/99]: 

	"区域网路",                                      // local area network game
	"网际网路",                                 // internet game

	"加入",                                     // join server
	"主控",                                     // host new server
	"增加 " ARROW_DOWN_STRING,                   // add address
	"更新",                                  // refresh network games
	"设定",                                    // player setup

	//"惯用手",                               // handedness label
	//"左手",                                     // handedness, left
	//"双手",                                    // handedness, center (ambi-dextrous)
	//"右手",                                    // handedness, right

	"连线速率",                             // connection speed label
	"28.8k",                                    // connection type 1
	"56.6k",                                    // connection type 2
	"ISDN",										// connection type 3
	"缆线数据机/DSL",							    // connection type 4
	"T1/区域网路",                                   // connection type 5
	"使用者自订",                             // connection type 6

	"时间限制",								// SCG[12/2/99]: 
	"人物颜色",							// SCG[12/2/99]: 
	"开始章节",							// SCG[12/2/99]: 
	"选择队伍",								// SCG[12/1/99]: 
	"CTF限制",								// SCG[12/1/99]: 
	"第1队颜色",								// SCG[12/1/99]: 
	"第2队颜色",								// SCG[12/1/99]: 
	"分数限制",								// SCG[12/1/99]: 
	"杀敌限制",								// SCG[12/1/99]: 
	"关卡限制",								// level limit
	"开始章节",							// SCG[12/2/99]: 
	"游戏地图",									// SCG[12/1/99]: 
	"游戏模式",								// game mode (deathmatch,deathtag,co-op)
	"技能等级",								// skill level
	"时间限制",								// time limit
	"最多人数",								// maximum players
	"人物",								// character
	"基本颜色",								// base color
	"开始　",									// start multiplayer game
												
	"武器停留",								// weapons remain
	"物品再生",							// items rematerialize
	"远距再生",								// spawn farthest point from players
	"容许离开",							// allow exiting of the level
	"相同地图",									// same map
	"力量再生",							// force respawn
	"掉落受伤",							// players take falling damage
	"立即强化",							// instant power-ups
	"容许强化",								// allow power-ups
	"容许生命",								// allow health kits
	"容许装甲",								// allow armor
	"弹药无限",							// unlimited ammunition
	"视角修正",								// fixed field-of-vision
	"组队游戏",									// team-play
	"伤害队友",							// hurt teammates
	"快速切换",								// fast weapon switch
	"脚步声音",								// footsteps audible
	"容许抓钩",								// allow hook
	"散弹枪子弹",							// show shotgun shells?
												
	"伺服器IP位址",						// server internet provider address
	"找不到区域性游戏",						// no local games found
	"开始地图",								// starting map
	"伺服器名称",								// server name
	"玩者名称",								// player name
	"队伍名称",								// team name
												
	"进行切腹　",						// confirm quit
												
	"是",										// yes, acknowledgement
	"否",										 // no, acknowledgement
	"储存游戏",								// cek[1-3-00] changed from 'save' to 'save game'

	"载入设定",								//cek[12-6-99]
	"储存设定",								//cek[12-6-99]
	"删除设定",							//cek[12-6-99]
	"选择设定档案",				//cek[12-6-99]
	"删除选择档案　",						//cek[12-6-99]
												
	"载入游戏",								//cek[12-6-99]
	"装甲",									//cek[12-6-99]
	"生命",									//cek[12-6-99]
	"关卡",									//cek[12-6-99]
	"怪物",									//cek[12-6-99]
	"秘密",									//cek[12-6-99]
	"时间",										//cek[12-6-99]
	"总计",									//cek[12-6-99]
	"章节",									//cek[12-6-99]

	"使用3D音场",								//cek[12-6-99]

	"已储存",									//cek[12-7-99]
	"己载入",									//cek[12-7-99]
	"己删除",									//cek[12-7-99]

	"流畅滑鼠",								//cek[12-7-99]
	"滚轮向上",									//cek[12-7-99]

	"滚轮向下",								//cek[12-7-99]

	"关闭ALT-TAB功能",						//cek[12-7-99]
	"血腥和屠杀",						//cek[12-7-99]
	"武器跳动",							//cek[12-7-99]
	"自动切换武器",						//cek[12-7-99]

	"强度",								//cek[12-7-99] cek[12-13-99]
	"武器闪光",							//cek[12-7-99]
	"弹痕",								// SCG[1/4/00]: 


	"前进",									//cek[12-9-99]
	"横向",										//cek[12-9-99]
	"上",										//cek[12-9-99]
	"角度",									//cek[12-9-99]
	"偏摇",										//cek[12-9-99]
	"使用摇杆",								//cek[12-9-99]
	"设定方向轴",							//cek[12-9-99]
	"设定摇杆选项",				//cek[12-9-99]
	"设定此按钮功能",			//cek[12-9-99]
	"按下摇杆按钮完成设定",		//cek[12-9-99]
	"界限",								//cek[12-9-99]
	"灵敏度",								//cek[12-9-99]

	"抢旗赛",							//cek[12-10-99]
	"回返",										//cek[12-10-99]
	"56.6k",									//cek[12-10-99]
	"自动",										//cek[12-10-99]
	"第1队",									//cek[12-10-99]
	"第2队",									//cek[12-10-99]

	"播放简介动画",							//cek[12-13-99]
	"使用技能系统",							//cek[12-13-99]
	"滑鼠速度",								//cek[12-13-99]

	"夥伴取得",								// cek[1-3-00]
	"夥伴过来",							// cek[1-3-00]
	"夥伴攻击",							// cek[1-3-00]
	"夥伴後退",							// cek[1-3-00]
	"夥伴停留",							// cek[1-3-00]

	"选择武器%d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"介面切换",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"下一介面",									// cek[1-5-00]  -- similar in meaning.
	"上一介面",									// cek[1-5-00]
	"使用介面",									// cek[1-5-00]

	"交谈",										// cek[1-11-00]
	"计分板",								// SCG[1/13/00]: 
	"小队交谈",								// cek[1-22-00]
	"%s已经分派为%s",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"置换　",									// cek[1-22-00]

	"前往大厅",								// cek[2-1-00]
	"大刀社群",								// cek[2-1-00]
	"继续　",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"继续会离开大刀\n并启动Mplayer伺服器的浏览器　",	// cek[2-1-00]
	"想看新闻　交谈　讨论和计分\n前往www.daikatananews.com　\n继续下一步会执行你的网页浏览器　",			// cek[2-1-00]

	"武器 %d",
	"取消",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"前往www.planetdaikatana.com　\n继续下一步会执行你的网页浏览器　",			// cek[3-9-00]
};


static char *tongue_menu_options[] =
{
	"跑步行动",                               // always run
	"弹跳　观看",                               // look-spring
	"横移　观看",                               // look-strafe
	"自动瞄准",                           // auto-targeting
	"准星",                                // crosshair
	"武器可见",                           // weapon visible
	"字幕",                                // subtitles

	"音效音量",                      // volume for sound fx
	"音乐音量",                             // volume for music
	"动画音量",                         // volume for cinematics

	"软体",                                 // video mode, software
	"OpenGL",                                   // video mode, opengl
	"套用变动值",                            // apply video mode changes

	"全萤幕",                               // fullscreen
// SCG[12/5/99]: 	"Stipple Alpha",                            // stipple alpha
	"影子",                                  // shadows
	"驱动程式",                                   // video driver
	"解析度",                               // resolution
	"萤幕大小",                              // screen size
	"亮度",                               // brightness
	"材质品质",                          // texture quality
	"可见状态棒",                    // status bar visibility
	"雾/水细致度",							// SCG[12/4/99]: 
	"步兵",									// difficulty level easy
	"武士",									// difficulty level medium
	"将军",									// difficulty level hard

	"雪和雨",							// cek[1-11-00]
	"雾"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"完全",
	"减半",
	"四分之一",
	"八分之一"
};


// weapon names
static char *tongue_weapons[] =
{
	"无弹药",
	"弩炮",
	"弩炮圆木",
	"筛子",
	"弹",
	"C4引爆器",
	" C4炸药包",
//	"天体",				// SCG[11/28/99]: 
//	"天体防御网",		// SCG[11/28/99]:	
	"大刀",
//	"大刀放逐",			// SCG[11/28/99]: 
//	"大刀防御网",		// SCG[11/28/99]: 
//	"大刀剑舞",	// SCG[11/28/99]: 
//	"大刀近战",			// SCG[11/28/99]: 
//	"大刀附身",		// SCG[11/28/99]: 
//	"大刀变化",		// SCG[11/28/99]: 
	"大刀致命",
	"圆盘",
	"混乱者手套",


//	"挪威根手电筒",
	"格拉克2020",
	"一些子弹",
	"黑帝斯铁　",
	"离子枪",
	"离子弹药",					// SCG[11/28/99]: 
	"动力枪",
	"动力",
	"强化微波枪",
	"强化微波枪",
//	"米达斯的黄金之指",		// SCG[11/28/99]: 
//	"黄铁矿",				// SCG[11/28/99]: 
	"拿尔的恶梦",
	"一些字楔",					// SCG[11/28/99]: 
	"新星光束",
	"新星电池",					// SCG[11/28/99]: 
	"撕裂枪",
	"无烟榴弹",				
	"震动波",
	"震波球",
	"连射枪",
	"连射枪子弹",
	"响尾蛇飞弹",
	"响尾蛇飞弹",			// SCG[11/28/99]: 
	"银爪",
	"霰弹枪",
	"霰弹",
	"史塔诺的诗节",
	"熔岩石",
	"摇曳的太阳",
	"摇曳的太阳",
//	"塔瑟之钩",				// SCG[11/28/99]: 
	"海神三叉戟",
	"三叉戟尖端",
	"毒满",
	"眼镜蛇毒",					// SCG[11/28/99]: 
	"万达克的萤火魔",
	"萤火之魔",
	"宙斯之眼",
	"神秘之眼",
	"终极瓦斯手"
};


static char *tongue_armors[] =
{
	"塑钢装甲",
	"光学装甲",
	"白银装甲",
	"黄金装甲",
	"锁链装甲",
	"黑金钢装甲",
	"凯夫勒装甲",
	"硬像胶装甲"
};


// monster names
static char *tongue_monsters[] =
{
	"机械战斗熊",
	"黑犯人",
	"布波得",
	"机械坎伯特",
	"百夫长",
	"三头犬",
	"飞行链炮者",
	"雕像柱",
	"机械克罗克斯",
// SCG[12/5/99]: 	"独眼巨人",
	"机械死亡球",
	"末日蝙蝠",
	"龙",
	"矮人",
	"范干格",
	"多毛人",
	"制箭者",
	"机械青蛙杀手",
	"格罗斯王",
	"金鱼",
	"格利风",
	"鸟身女妖",
	"入狱者",
	"三岛影",
// NSS[3/6/00]:"米诺斯王",
	"地狱火圣骑士",

	"电力女武神",
	"实验室猴子",
	"低温科技",
	"机械雷射枪",
	"狼人",
	"梅杜莎",
// NSS[3/6/00]:	"牛头人",
	"三岛守卫",
	"拿尔死灵法师",
	"瘟疫老鼠",
	"水管老鼠",
	"牧师",
	"犯人",
	"机械原形足",
	"精神爪",
	"机械暴怒专家5000",
	"火箭帮",
	"海豹部队散火箭兵",
	"腐虫",
	"色狼",
	"海豹部队指挥官",
// SCG[12/5/99]: 	"海豹潜水夫",
	"海豹部队队长",
	"海豹部队散弹枪兵",
	"鲨鱼",
	"掠杀者",

	"骷髅",
	"瘦工人",
	"胖工人",
	"机械污物喽罗",
	"小蜘蛛",
	"蜘蛛",
	"史塔佛法师",
	"军医",
	"小偷",
	"机械闪电飞靶射击",
// SCG[12/5/99]: 	"机械定轨攻击",
	"乌兹帮",
	"白色俘虏",
	"巫师",
	"万达克巫师",
	"海鸥",
	"萤火虫",

	"萤火魔",
	"美纪",
	"鬼魂",
	"麻药鱼"
};


static char *tongue_world[] =
{
	"尚有其他",                                 // triggers to go until sequence complete
	"步骤完成",                         // trigger sequence completed

	"你必须持有",                              // need a specific key
	"你必须持有正确的钥匙　 \n",               // wrong key in possession
	"不能从这个方向打开\n",					  // SCG[11/1/99]: Can't be opened this way

	"你觉得毒素离开了你的系统　\n",    // poison expiring
	"你肺中的氧气几乎用尽了　\n",           // oxylung expiring
	"你的环境衣几乎耗费了　\n",                // envirosuit expiring
	"绿色精灵急需食物　\n",                 // megashield expiring
	"暴怒之球减弱了　\n",                   // wraith orb expiring
	"力量强化的效果慢慢的消失了　\n",				  // power boost expiring
	"攻击强化的效果慢慢的消失了　\n",                // attack boost expiring
	"速度强化的效果慢慢的消失了　\n",                 // speed boost expiring
	"弹跳强化的效果慢慢的消失了　\n",             // jump boost expiring
	"体力强化的效果慢慢的消失了　\n",              // health boost expiring

	"已加入游戏",                            // player joined the game
	"离开这一关",                           // player exited the level

	"宫本弘",                              // Hiro Miyamoto's full name
	"高速强森",                           // Superfly's full name
	"海老原美纪",                             // Mikiko's full name

	// SCG[10/18/99]: these next 5 had enums, but no strings. 
	"",								// SCG[10/18/99]: T_PLAYER_SELF_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_SPAM_KICK
	"",								// SCG[10/18/99]: T_PLAYER_TIME_LIMIT_HIT
	"",								// SCG[10/18/99]: T_PLAYER_FRAG_LIMIT_HIT
	"",								// SCG[10/18/99]: T_TELL_CMD_UNKNOWN_PLAYER

	"离开这个区域必须拥有\n",				// SCG[10/18/99]: no exiting level without sidekicks
	"要离开你必须取得\n",				// SCG[11/9/99]: No exiting without a key
	"中毒　\n",						// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"你找到一个秘密　 \n",					// cek[1-3-00]
	"法力骷髅的效果慢慢的消失了　\n"                   // cek[2-9-00]
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s被%s给切开了　\n"
};

static char *tongue_deathmsg_self[] =
{
	"的头骨被切开了　\n",                       // death, fell
	"被吸下去　\n",                             // death, drowned
	"变得易碎　\n",                                // death, lava
	"变苗条了　\n",                                 // death, slimed
	"屈服於毒素下　\n",                    // death, poison
	"不堪生命的折磨　\n",                             // death, killed self
	"传送时爆掉了　\n",                            // death, teleport
	"被压成一块饼　\n",             // death, crushed

	"吞下满肚的C4炸药　\n"                       // death, C4
};

static char *tongue_deathmsg_weapon1[] =
{
	"%s被%s伤了自尊　\n",
	"%s被%s重重打了一掌　已经成为尘土　\n",
	"%s被%s离子化了　\n",
	"%s被%s弄得乱七八糟　\n",
	"%s被%s用双管枪轰到　\n",
	"%s坐到了%s打出来的火箭　\n",
	"%s被%s摇到半空中　\n"
};

static char *tongue_deathmsg_weapon2[] =
{
	"%s把%s当做终极飞盘来丢　\n",
	"%s狠狠的咬了%s一口　\n",
	"%s把%s变成火焰地狱　 \n",
	"%s对%s用铁　重重的打下去　\n",
	"%s把%s打爆了　\n",
	"%s跪在宇斯面前　\n"
};

static char *tongue_deathmsg_weapon3[] =
{
	"%s被%s像火腿一样切开　\n",
	"%s把%s打挂了　\n",
	"%s 把%s当地板踩　 \n",
	"%s被%s用流星砸到　\n",
	"%s抓到了%s的高压电　\n",
	"%s被%s的手段抓个正著　 \n"
};

static char *tongue_deathmsg_weapon4[] =
{
	"%s在%s面前露出一个脑袋　\n",
	"%s被%s打成弹震症　\n",
	"%s被%s撕裂　\n",
	"%s被%s打成一团泥　\n",
	"%s被%s冻成冰块　\n",
	"%s被%s打穿一个洞　\n",
	"%s被%s烤透透　\n"
};

static char *tongue_scoreboard[] =
{
	"姓名",
	"杀敌数",
	"回传值",
	"时间",
	"发射弹数",
	"击中弹数",
	"死亡次数",
	"分数"										// cek[1-22-00]
};


static char *tongue_difficulty[] =  // difficulty settings for 'new game' menu
{
	"擄堈搙偺慖戰:",//"Select Difficulty:",           // difficulty header
	"步兵",	                   // newbie
	"武士",                    // average
	"将军"                    // expert (or so they think!)
};




static char *tongue_skincolors[] =
{

	"蓝",                         // blue
	"绿",                        // green
	"红",                          // red
	"金"                          // gold
};


static char *tongue_statbar[] =
{
	"力量",                        // skill 'power' label
	"攻击",                       // skill 'attack' label
	"速度",												// skill 'speed' label
	"跳跃",													// skill 'acrobatic' label
	"体力",											// skill 'vitality' label

	"装甲",                        // armor label
	"生命",												// health label
	"弹药",													// ammo label
	"杀敌",												// kills label
	"关卡",  											// experience points label

	"升级　 ",                    // experience level up
	"选择",                       // select skill
	"指定技能点数",                    // add point to skill

	"载入..."                    // loading new level
};


static char *tongue_weapon_misc[] =
{
	"C4模组脱离稳定状态　 \n"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =    // sidekick commands
{
	"取得",	                        // 'get' item command
	"过来",                          // 'come here', 'come to me'
	"停留",                           // 'stay' command, 'don't move'

	"攻击",                         // 'attack' command
	"後退",                       // 'back off', 'move away'

	"我能拿"                      //  asking to pick up an item: "Can 			
//ve the shotgun?"
};

static char *tongue_ctf[] =
{
	"红旗",
	"蓝旗",

	"不明队伍",

	"%s杀敌数",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"红",
	"蓝",
	"铬",
	"钢铁",
	"绿",
	"橙",
	"紫",
	"黄",

	"现在分数是%s　%d　%s　%d\n",
	"你捕捉到%s次　\n",
	"你队伍中的%s已经捉到了%s　\n",
	"%s来自%s队伍　带走了你的旗子　\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"你拿到了%s\n",
	"你队伍中的%s捉到了%s\n",
	"%s偷走了你的旗子\n",
	"%s已经抢回来了\n",



	"%d是一个无效的队伍数字　请按1或2\n",
	"你现在是%d队（%s）\n",
	"你已经是%d队\n",
	"%s加入了%s队\n",									// translate same as with the %s team above

	"%s队已经赢了　\n最後总分　%s　%d　%s　%d\n",		// translate same as with the %s team above
	"这场游戏平手　\n",
	"时间用尽　\n",
	"到达抢旗次数上限　\n"
};

static char *tongue_deathtag[] =
{
	"炸弹",														// as in backpack
	"%s炸弹",													// as in red pack
	"到达分数上限　\n",
	"你的时间用完了　\n",
	"你的时间刚刚用尽　\n",

	"%s来自%s队　取得了%s　\n",				// translate using the '%s team' guidelines above

	"你的队伍得到一分　\n",
	"你的队伍得到%d分　\n",								// you get 2 points
	"%s队伍得到一分　\n",								// translate using the '%s team' guidelines above

	"%s队伍得到%d分　\n",							// The red team gets 2 points

	"你得分了　\n",
	"你队伍中的%s得到分数　\n",
	"%s来自%s队伍　得到分数　\n"
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
