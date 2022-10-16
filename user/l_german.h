// German text t

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
	{"RETURN", K_ENTER},
	{"ESC", K_ESCAPE},
	{"LEERTASTE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"PFEIL HOCH", K_UPARROW},
	{"PFEIL RUNTER", K_DOWNARROW},
	{"PFEIL LINKS", K_LEFTARROW},
	{"PFEIL RECHTS", K_RIGHTARROW},
	
	{"ALT", K_ALT},
	{"STRG", K_CTRL},
	{"UMSCHALT", K_SHIFT},
	
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
	
	{"EINFG", K_INS},
	{"ENTF", K_DEL},
	{"BILD RUNTER", K_PGDN},
	{"BILD HOCH", K_PGUP},
	{"POS 1", K_HOME},
	{"ENDE", K_END},
	
	{"MAUS 1", K_MOUSE1},
	{"MAUS 2", K_MOUSE2},
	{"MAUS 3", K_MOUSE3},
	
	{"JOY 1", K_JOY1},
	{"JOY 2", K_JOY2},
	{"JOY 3", K_JOY3},
	{"JOY 4", K_JOY4},
	
	{"AUX 1", K_AUX1},
	{"AUX 2", K_AUX2},
	{"AUX 3", K_AUX3},
	{"AUX 4", K_AUX4},
	{"AUX 5", K_AUX5},
	{"AUX 6", K_AUX6},
	{"AUX 7", K_AUX7},
	{"AUX 8", K_AUX8},
	{"AUX 9", K_AUX9},
	{"AUX 10", K_AUX10},
	{"AUX 11", K_AUX11},
	{"AUX 12", K_AUX12},
	{"AUX 13", K_AUX13},
	{"AUX 14", K_AUX14},
	{"AUX 15", K_AUX15},
	{"AUX 16", K_AUX16},
	{"AUX 17", K_AUX17},
	{"AUX 18", K_AUX18},
	{"AUX 19", K_AUX19},
	{"AUX 20", K_AUX20},
	{"AUX 21", K_AUX21},
	{"AUX 22", K_AUX22},
	{"AUX 23", K_AUX23},
	{"AUX 24", K_AUX24},
	{"AUX 25", K_AUX25},
	{"AUX 26", K_AUX26},
	{"AUX 27", K_AUX27},
	{"AUX 28", K_AUX28},
	{"AUX 29", K_AUX29},
	{"AUX 30", K_AUX30},
	{"AUX 31", K_AUX31},
	{"AUX 32", K_AUX32},
	
	{"ZB_7",			K_KP_HOME },
	{"ZB_8",		K_KP_UPARROW },
	{"ZB_9",			K_KP_PGUP },
	{"ZB_4",	K_KP_LEFTARROW },
	{"ZB_5",			K_KP_5 },
	{"ZB_6",	K_KP_RIGHTARROW },
	{"ZB_1",			K_KP_END },
	{"ZB_2",	K_KP_DOWNARROW },
	{"ZB_3",			K_KP_PGDN },
	{"ZB_ENTER",		K_KP_ENTER },
	{"ZB_0",			K_KP_INS },
	{"ZB_KOMMA",			K_KP_DEL },
	{"ZB_GETEILT",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },
	
	{"MAUSRAD HOCH", K_MWHEELUP },
	{"MAUSRAD RUNTER", K_MWHEELDOWN },
	
	{"PAUSE", K_PAUSE},
	
	{"SEMIKOLON", ';'},	// because a raw semicolon seperates commands
	
	{NULL,0}
};

// vkey conversion table
//==========================================================================
static byte        scantokey[128] = 
                    { 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
    0  ,	27,     '1',    '2',    '3',    '4',    '5',    '6', 
    '7',    '8',    '9',    '0',    'ß',    '´',    K_BACKSPACE, 9, // 0 
    'q',    'w',    'e',    'r',    't',    'z',    'u',    'i', 
    'o',    'p',    'ü',    '+',    13 ,    K_CTRL,'a',  's',      // 1 
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    'ö', 
    'ä',    '`',    K_SHIFT,'<',  'y',    'x',    'c',    'v',      // 2 
    'b',    'n',    'm',    ',',    '.',    '-',    K_SHIFT,'*', 
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
	"Aufgehoben:",              // weapon pickup TX NOTE:+ the items in the weapons section below.
	"Aufgehoben:",			        // health pickup TX NOTE:+ one of the 3 health items below (health kit, Goldenorb, Wraith orb.
	"Aufgehoben:",						  // armor good pickup TX NOTE:+ the items in the armor section below.
	"Aufgehoben:",						  // boost pickup TX NOTE:not used - DON'T TRANSLATE.
	"Aufgehoben: Speicherjuwel",						// T_PICKUP_SAVEGEM		pick up Save Gem
	
	"STÄRKE",                  // use power boost TX NOTE:must match statusbar.xls .
	"ANGRIFF",                  // use attack boost TX NOTE:must match statusbar.xls .
	"TEMPO",                  // use speed boost TX NOTE:must match statusbar.xls .
	"AKROBATIK",                   // use jump boost TX NOTE:must match statusbar.xls .
	"VITALITÄT",                 // use health boost TX NOTE:must match statusbar.xls .
	
	"Gleich wird's heiß!",              // bomb explode TX NOTE:"let's get out of here!.
	"Sie brauchen eine Flasche, um das zu mischen.",  // need bottle to mix ingredients TX NOTE:sentence.
	"Sie brauchen: ",             // ingredient required TX NOTE:+ bomb item, short desc below.
	"Sie mischen Sprengstoff.",         // bomb created TX NOTE:sentence.
	"Sie fanden ",                // bomb found, TX NOTE:+ one of the ingredients or a bottle.
	"einen Beutel mit Schwefel",						// bomb ingredient 1
	"ein Stück Kohle",              // bomb ingredient 2
	"ein Phiole mit Salpeter",           // bomb ingredient 3
	"eine leere Flasche",             // bomb bottle
	
	"Schwefel",                 // bomb item, short description
	"Kohle",                 // bomb item, short description
	"Salpeter",                // bomb item, short description
	
	"Sie lesen das",              // read book TX NOTE:+ one of two books below.
	"Das können Sie hier nicht verwenden.",        // no read book TX NOTE:sentence.
	"Wyndrax-Zauberbuch",          // Wyndrax spellbook
	//	"Satanische Buch des Bösen",         // Satanic book of evil
	
	"Sie nehmen",              // key pickup TX NOTE:one of the 3 following items.
	"den Schlüssel zur Krypta",              // crypt key name
	"den Wyndrax-Schlüssel",             // Wyndrax key name
	"ein Karte für eine Zelle",          // Prison cell key
	
	"die rote Steuerkarte",						// SCG[11/15/99]: red control card name
	"die blaue Steuerkarte",					// SCG[11/6/99]: blue control card name
	"die grüne Steuerkarte",					// SCG[11/6/99]: green control card name
	"die gelbe Steuerkarte",					// SCG[11/6/99]: yellow control card name
	"den Hex-Schlüsselstein",							// SCG[11/6/99]: hex keystone name
	"den Quad-Schlüsselstein",						// SCG[11/6/99]: quad keystone name
	"den Trigon-Schlüsselstein",						// SCG[11/6/99]: trigon keystone name
	"den Megaschild",							// SCG[11/14/99]: Megashield name
	"den Manaschädel",							// SCG[11/14/99]: 
	"das Gegengift",						// SCG[11/14/99]: 
	"eine Drachme",								// SCG[11/14/99]: 
	"das Horn",									// SCG[11/14/99]: 
	"eine Tafel mit dem griechischen Buchstaben a",				// SCG[11/14/99]: A of AEGIS key name
	"eine Tafel mit dem griechischen Buchstaben e",				// SCG[11/14/99]: E of AEGIS key name
	"eine Tafel mit dem griechischen Buchstaben g",				// SCG[11/14/99]: G of AEGIS key name
	"eine Tafel mit dem griechischen Buchstaben i",				// SCG[11/14/99]: I of AEGIS key name
	"eine Tafel mit dem griechischen Buchstaben s",				// SCG[11/14/99]: S of AEGIS key name
	"einen Splitter des Läuterers",					// SCG[11/14/99]: 
	"die schwarze Kiste",							// SCG[11/14/99]: 
	"den Ring des Feuerschutzes",				// SCG[11/14/99]: 
	"den Ring des Schutzes vor Untoten",			// SCG[11/14/99]: 
	"den Ring des Blitzschutzes",			// SCG[11/14/99]: 
	"den Enviroanzug",							// SCG[11/14/99]: 
	
	"das Erste-Hilfe-Set",              // health kit
	"die Goldseele",              // Goldensoul
	"die Gespenstersphäre",							// wraith orb
	
	"ein Speicherjuwel",
	"Sie haben kein Speicherjuwel",					//cek[12-13-99]
	"den Heiltrank",							// cek[1-3-00]

	// cek[2-2-00]: altered the following ***********************************************
	"das Wundpäckchen",
	"die Lebensvase",
	"die Heiltrankflasche",
	"die Verbandskiste",
	"den Verbandskasten",
	"Das Gegengift ist leer.",
// SCG[2/8/00]: messages for completed artifacts
	"Sie haben alle AEGIS-Tafeln gefunden!",
	"Sie haben alle drei Schlüsselsteine gefunden!",
	"Sie haben den Läuterer zusammengesetzt!"
};

// menu-related TX NOTE:all independent words -- no grammatical inflection.
static char *tongue_menu[] =
{
	"Schritt vor",                 // key bind to Move Forward
	"Schritt zurück",                 // key bind to Move Backward 
	"Seitwärts links",                // key bind to Step Left JG:990802
	"Seitwärts rechts",                // key bind to Step Right JG:990802
	"Angriff",                  // key bind to Attack
	"Springen",                   // key bind to Jump/Up
	"Benutzen",               // key bind to Use/Operate object JG:990802
	"Kriechen",                  // key bind to Crouch/Down
	"Nächste Waffe",               // key bind to Next Weapon
	"Vorherige Waffe",               // key bind to Prev Weapon
	"Drehung links",                // key bind to Turn Left JG:990802
	"Drehung rechts",                // key bind to Turn Right JG:990802
	"Laufen",                   // key bind to Run
	"Seitwärts",                 // key bind to Sidestep JG:990802
	"Blick hoch",                 // key bind to Look Up JG:990802
	"Blick runter",                // key bind to Look Down JG:990802
	"Blick zentrieren",               // key bind to Center View
	"Mausblick",                // key bind to Mouse Look
	"Tastatur-Sicht",                 // key bind to Keyboard Look JG:990802  
	"Vergrößern",                             // key bind to increase screen size
	"Verkleinern",                           // key bind to decrease screen size
	"HUDs zeigen",                            // key bind to Show All Huds
	"Gepäck öffnen",                // key bind to Inventory Open
	"Objekt benutzen",              // key bind to Use Item
	//  "Wegwerfen",                // key bind to Drop Item
	"Nächstes Objekt",                // key bind to Next Item
	"Vorheriges Objekt",                // key bind to Prev Item
	"Partner: tauschen",                            // key bind to Swap Sidekick
	"Nächster Befehl",                             // key bind to Command Next
	"Vorheriger Befehl",                             // key bind to Command Prev
	"Befehl anwenden",                            // key bind to Command Apply
	
	ARROW_LEFT_STRING " Links",         // left arrow key
	ARROW_UP_STRING  " Aufwärts",          // up arrow key
	ARROW_DOWN_STRING " Abwärts",         // down arrow key
	ARROW_RIGHT_STRING " Rechts",        // right arrow key
	"--",                    // no bind setting for this key
	
	"ENTER zum Bestätigen; DEL zum Löschen.",   // set/delete key TX NOTE:sentence.
	"Um auszuwählen, ENTER drücken.",         // select key TX NOTE:sentence. JG:990802
	"Um abzubrechen, ESC drücken.",       // set/abort TX NOTE:sentence. JG:990802
	
	"Standard",                 // default button Standardeinstellungen JG:990802
	
	"Keine",                   // no mouse bindings
	"Links/rechts drehen",             // mouse "move left and right" setting JG:990802
	"Links/rechts bewegen",             // mouse "turn left and right" setting JG:990802
	"Oben/unten sehen",               // mouse "look up and down" setting JG:990802
	"Vorwärts/Zurück",              // mouse "move forward and back" setting
	"Y-Achse umkehren",              // reverse mouse axis
	"Mausblick",                // Mouse-Look
	"Blick/Feuern",               // Mouse-strafe
	"X-Achse",                  // x axis
	"Y-Achse",                  // y axis
	"Taste 1",                 // button 1 label
	"Taste 2",                 // button 2 label
	"Taste 3",                 // button 3 label
	"Befehl auswählen (ESC zum Abbrechen).",     // select a command, ESCape to cancel JG:990802
	"Zur Seite drehen",                 // turn left and right, abbreviated
	"Zur Seite gehen",                // strafe left and right, abbreviated
	"Oben/unten sehen",                 // look up and down, abbreviated
	"Vorwärts/Zurück",                 // move forward and back, abbreviated
	"Empfindlichkeit der X-Achse",              // mouse sensitivity for x-axis
	"Empfindlichkeit der Y-Achse",              // mouse sensitivity for y-axis 
	"Mausoptionen konfigurieren:",         // configure mouse options TX NOTE:sentence.
	
	"Deathmatch",                // 'deathmatch' game
	"Deathtag",                 // 'deathtag' game
	"Kooperativ",               // 'cooperative' game
	
	"Einfach",                   // easy difficulty
	"Mittel",                  // medium difficulty
	"Schwer",                   // hard difficulty
	
	"Internetspiel suchen",					// SCG[12/1/99]: 
	"Netzwerkspiel suchen",						// SCG[12/1/99]: 
	"Mehrspielerrunde starten",					// SCG[12/1/99]: 
	
	"Netzwerk",                                      // local area network game
	"Internet",                                 // internet game
	
	"Teilnehmen",                   // join server
	"Neu anlegen",                   // new server
	"Hinzufügen" ARROW_DOWN_STRING,          // add address
	"Aktualisieren",                 // refresh network games (13 chars--must enlarge button)
	"Setup",                  // player setup 
	
	//"Handedness",                // handedness label TX NOTE:not used. DO NOT TRANSLATE.
	//"Left",                   // handedness, left TX NOTE:not used. DO NOT TRANSLATE.
	//"Ambi-",                  // handedness, center TX NOTE:ambi-dextrous. TX NOTE:not used. DO NOT TRANSLATE.
	//"Right",                  // handedness, right TX NOTE:not used. DO NOT TRANSLATE.
	
	"Übertragungsrate",               // connection speed label
	"28.8 KB",                  // connection type 1
	"56.6 KB",                  // connection type 2
	"ISDN",                 // connection type 3
	"Kabel/DSL",                // connection type 4
	"T1/Netzwerk",                  // connection type 5
	"Benutzerdefiniert",               // connection type 6 TX NOTE:user enters his own connection type.
	
	"Zeitlimit",								// SCG[12/2/99]: 
	"Farbe der Spielfigur",							// SCG[12/2/99]: 
	"Anfangsepisode",							// SCG[12/2/99]: 
	"Mannschaft",								// SCG[12/1/99]: 
	"Flag-Limit",								// SCG[12/1/99]: 
	"Farbe Team 1 ",								// SCG[12/1/99]: 
	"Farbe Team 2 ",								// SCG[12/1/99]: 
	"Punktelimit",								// SCG[12/1/99]: 
	"Abschußlimit",								// SCG[12/1/99]: 
	"Level-Limit",								// frag limit
	"Anfangsepisode",							// SCG[12/2/99]: 
	"Karte",									// SCG[12/1/99]: 
	"Spielmodus",                // game mode (deathmatch,deathtag,co-op)
	"Fähigkeiten",               // skill level
	"Zeitlimit",                // time limit
	"Max. Spieler",               // maximum players
	"Spielfigur",                // character
	"Farbe",                // base color
	"Start!",                  // start multiplayer game
	
	"Waffen bleiben",               // weapons remain with you into the next area of play
	"Neue Gegenstände",              // items rematerialize on the map JG:990802
	"Sichere Position",               // place user's character at the farthest point from other players 
	"Notausgang",              // allow players to exit the level wihtout being killed first 
	"Gleiche Karte",                 // use the same map as last time
	"Wiederbelebung",              // make the respawn command happen now. JG:990802
	"Sturzschaden",              // players take damage from falling down JG:990802
	"Sofort-Powerups",              // make instant power-ups available in the game
	"Powerups",               // allow power-ups to be used in the game JG:990802
	"Heilung",               // allow health kits to be used in the game JG:990802
	"Rüstung",               // allow armor to be used in the game JG:990802
	"Munition",              // unlimited ammunition  JG:990802
	"Sicht beschränken",                // limits the user's field-of-vision JG:990802
	"Mannschaftsspiel",                 // team-play
	"Verletzungen",              // hurt teammates allows players to cause damage to team-mates. JG:990802
	"Schnellwahl",               // allows a player to change weapons faster than normal JG:990802
	"Schritte",                // footsteps become audible or silent 
	"Kletterhaken",                // allow hook  JG:990802 (grappling hook)
	"Munitionshülsen",              // show shotgun shells?
	
	"Server-IP-Adresse",            // server internet provider address
	"Keine Spiele gefunden",           // no local games found TX NOTE:sentence.
	"Karte starten",               // starting map
	"Server ",               // server name
	"Spieler ",               // player name
	"Mannschaft ",                // team name
	
	"Harakiri begehen?",            // confirm quit TX NOTE:Do not translate Harakiri--it's Jpn.
	
	"Ja",										// yes, acknowledgement
	"Nein",										 // no, acknowledgement
	"Speichern",
	
	"Laden",								//cek[12-6-99]
	"Speichern",								//cek[12-6-99]
	"Löschen",							//cek[12-6-99]
	"Konfigurationsdatei aussuchen",				//cek[12-6-99]
	"Ausgewählte Datei löschen?",						//cek[12-6-99]
	
	"Spiel laden",								//cek[12-6-99]
	"Rüst.",									//cek[12-6-99]
	"Ges.",									//cek[12-6-99]
	"Level",									//cek[12-6-99]
	"Ungeheuer",									//cek[12-6-99]
	"Geheimnisse",									//cek[12-6-99]
	"Zeit",										//cek[12-6-99]
	"Gesamt",									//cek[12-6-99]
	"Episode",									//cek[12-6-99]
	
	"3D-Sound",								//cek[12-6-99]
	
	"gespeichert",									//cek[12-7-99]
	"geladen",									//cek[12-7-99]
	"gelöscht",									//cek[12-7-99]
	
	"Weicher Mauslauf",								//cek[12-7-99]
	"Rad hoch",									//cek[12-7-99]
	"Rad runter",								//cek[12-7-99]
	
	"ALT-TAB nicht erlaubt",						//cek[12-7-99]
	"Zerstörung",						//cek[12-7-99]
	"Waffen bewegen sich",							//cek[12-7-99]
	"Auto-Waffentausch",						//cek[12-7-99]
	
	"Modulation",									//cek[12-7-99]
	"Glänzende Waffen",							//cek[12-7-99]
	"Einschußlöcher",								// SCG[1/4/00]: 
	
	"Vorwärts",									//cek[12-9-99]
	"Seitwärts",										//cek[12-9-99]
	"Rauf",										//cek[12-9-99]
	"Steigung",									//cek[12-9-99]
	"Ausweichen",										//cek[12-9-99]
	"Joystick benutzen",								//cek[12-9-99]
	"Achse konfigurieren",							//cek[12-9-99]
	"Joystickoptionen konfigurieren:",				//cek[12-9-99]
	"Eine Funktion für diesen Knopf auswählen",			//cek[12-9-99]
	"Um fortzufahren, Joystickbutton drücken",		//cek[12-9-99]
	"Grenzwert",								//cek[12-9-99]
	"Empfindlichkeit",								//cek[12-9-99]
	
	"Capture the Flag",							//cek[12-10-99]
	"Zurück",										//cek[12-10-99]
	"56,6 KB",									//cek[12-10-99]
	"Auto",										//cek[12-10-99]
	"Team 1",									//cek[12-10-99]
	"Team 2",									//cek[12-10-99]

	"Intro abspielen",								//cek[12-13-99]
	"Handicap-System",							//cek[12-13-99]
	"Maus-Geschwindigkeit",								//cek[12-13-99]

	"Partner: nimm",								// cek[1-3-00]
	"Partner: komm",							// cek[1-3-00]
	"Partner: angriff",							// cek[1-3-00]
	"Partner: rückzug",							// cek[1-3-00]
	"Partner: bleib",							// cek[1-3-00]

	"Wähle Waffe %d",							// cek[1-4-00]  as in "Select Weapon 1" (leave the %d)

	"HUD durchlaufen",								// cek[1-5-00]  -- for these, translate HUD to something short and reasonably
	"HUD nächstes",									// cek[1-5-00]  -- similar in meaning.
	"HUD vorheriges",									// cek[1-5-00]
	"HUD benutzen",									// cek[1-5-00]

	"Nachricht",										// cek[1-11-00]
	"Abschussliste",								// SCG[1/13/00]: 
	"Befehl an Team",								// cek[1-22-00]
	"%s ist bereits %s zugewiesen",				// cek[1-22-00]  as in "F is already assigned to attack" (leave in %s)
	"Ersetzen?",									// cek[1-22-00]

	"Mplayer",								// cek[2-1-00]
	"DK-News",								// cek[2-1-00]
	"Weiter?",								// cek[2-1-00]

	// for the next two, all \n in the strings denotes a string break.  These must remain in place and the translated
	// string should have them in roughly the same place.
	"Das Fortfahren beendet Daikatana \nund startet das Mplayer Serverbrowser.",	// cek[2-1-00]
	"Neuigkeitens, Chat, Foren und Tabellen\nfinden Sie auf www.daikatananews.com. Wenn\nSie weitermachen, startet Ihr Webbrowser.",			// cek[2-1-00]

	"Waffe %d",									// cek[3-6-00]: treat as "weapon 1"
	"Abbrechen",									// cek[3-9-00]
	"www.planetdaikatana.com",													// cek[3-9-00]
	"Zur www.planetdaikatana.com. Wenn\nSie weitermachen, startet Ihr Webbrowser.",			// cek[3-9-00]
};


static char *tongue_menu_options[] = 
{
	"Immer laufen",                // makes user's characters run everywhere
	"Schauen und springen",                // look + jump
	"Schauen und feuern",                // look + strafe (shoot wildly)
	"Automatisch zielen",              // computer aims at nearest target for the user
	"Fadenkreuz",                // a crosshair is displayed 
	"Waffe sichtbar",              // weapon is visible
	"Untertitel",                   //subtitles
	
	"SFX-Lautstärke",           // volume for sound fx
	"Musiklautstärke",               // volume for music
	"Filmlautstärke",                         // volume for cinematics
	
	"Software",                 // video mode, software
	"OpenGL",                  // video mode, opengl
	"Änderungen übernehmen",              // apply video mode changes TX NOTE:sentence.
	
	"Vollbild",                // make view use the full screen
	//	"Stipple Alpha",              // stipple alpha TX NOTE:this is a type of speed optimization technique. Don't translate.
	"Schatten",                 // allow shadows
	"Treiber",                  // video driver
	"Auflösung",                // choose/set screen display resolution
	"Bildschirmgröße",               // choose/set screen size
	"Helligkeit",                // choose/set brightness
	"Texturen-Qualität",             // choose/set quality of 3D image textures 
	"Statusleiste sichtbar",           // choose/set whether the status bar is visible.
	"Nebel- und Wasserdetails",				// SCG[12/4/99]: 
	"Ronin",									// difficulty level easy
	"Samurai",									// difficulty level medium
	"Shogun",									// difficulty level hard

	"Schnee und Regen",							// cek[1-11-00]
	"Nebel"										// cek[1-11-00]		
};



//descriptions for the texture quality settings.
static char *texture_quality_desc[] =
{
	"Voll",
	"Halb",
	"1/4",
	"1/8"
};


// weapon names TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_weapons[] =
{
	"keine Munition",		//only shows up if you have no ammo for the weapon you're trying to use. TX NOTE:treat like sentence "you have "no ammo".
	"Ballista",
	"Munition für die Ballista",
	"Bolzer",
	"Bolzen",
	"C4-Vitzatergo",
	"C4-Pakete",
	//	"the Celestril",				// SCG[11/28/99]: 
	//	"the Celestril Barrier",		// SCG[11/28/99]:	
	"Daikatana",
	//	"the Daikatana Banish",			// SCG[11/28/99]: 
	//	"the Daikatana Barrier",		// SCG[11/28/99]: 
	//	"the Daikatana Dancing Blade",	// SCG[11/28/99]: 
	//	"the Daikatana Melee",			// SCG[11/28/99]: 
	//	"the Daikatana Possession",		// SCG[11/28/99]: 
	//	"the Daikatana Proteus",		// SCG[11/28/99]: 
	"Diskus von Dädalus",
	"Diskus",
	"Disruptor-Handschuh",
	//	"Norwegisches Blitzlicht",
	"Automark 2020", //TX NOTE:a ficticious automatic pistol--US game currently reads "Glock", but that's a trademark.
	"einige Gewehrkugeln",
	"Hammer",
	"Ionen-Blaster",
	"das Ionenpack",					// SCG[11/28/99]: [accusative=You picked up...]
	"Kineticore",
	"Rail-Ball",
	"Metamaser",
	"Munition für den Metamaser",
	//	"the Midas' Golden Grasp",		// SCG[11/28/99]: 
	//	"the Pyrite Lump",				// SCG[11/28/99]: 
	"Nharres Alptraum",
	"ein bißchen Fleisch",					// SCG[11/28/99]: 
	"Novastrahl",
	"Novapack",								//JK [01/26/00]
	"Ripgun",
	"Cordit-Granaten",
	"Shockwelle",
	"Schock-Sphäre",
	"Shotcycler",
	"Munition",
	"Sidewinder",
	"Sidewinder-Raketen",			// SCG[11/28/99]: 
	"Silberkralle",
	"Slugger",
	"Slugs",
	"Stavros' Stab",
	"Lava-Steine",
	"Sunflare",
	"Sunflare",
	//	"the Tazerhook",				// SCG[11/28/99]: 
	"Poseidons Dreizack",
	"Dreizack-Spitzen",
	"Gifter",
	"Gift einer Kobra",					// SCG[11/28/99]: 
	"Wyndrax Hauch",
	"Hauchladungen",
	"Hand des Zeus",
	"Bolzen",
	"Ultimative Gashände"
};

// armor TX NOTE:"you picked up" +.TX NOTE:translate what you can. "localize" spellings of anything else.
static char *tongue_armors[] =
{
	"Eisenrüstung",
	"Chromrüstung",
	"Silberrüstung",
	"Goldrüstung",
	"Kettenrüstung",
	"Rüstung aus schwarzem Adamant",
	"Kevlar-Rüstung",
	"Ebonit-Rüstung"
};


// monster names TX NOTE:NOMINITIVE CASE. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters[] =
{
	"die RoboCo-Kampfeber",
	"der Schwarze Gefangene",
	"der Buboid",     // TX NOTE:must match script.
	"der RoboCo-Cambot",
	"der Zentaur",
	"der Cerberus",     // TX NOTE:deleted. don't translate.
	"der Fliegende Schütze",
	"die Caratyd-Säule",
	"das Crox",
	//	"der Zyklop",
	"die RoboCo-Todessphäre",
	"die Fledermaus des Todes",
	"der Drache",     // TX NOTE:must match script.
	"der Zwerg",
	"die Femgang",
	"der Fährmann",
	"der Fletscher",
	"der RoboCo-Froschinator",
	"König Garroth",
	"der Goldfisch",
	"der Griffon",
	"die Harpyie",
	"der Mitgefangene",
	"Kage",
	"King Minos",     // TX NOTE:deleted. don't translate.
	"der erste Ritter",
	"der zweite Ritter",
	"der Laboraffe",
	"der Laborarbeiter",
	"das RoboCo-Lasergat",
	"der Lycanthir",
	"die Medusa",     // TX NOTE:must match script.
	"der Minotaurus",
	"Mishimas Wachen",     // TX NOTE:must match script.
	"Nharre"
	"die Pest-Ratte"
	"die Ratte",
	"der Priester",
	"der Gefangene",
	"der RoboCo-Protopod",
	"die Psyclaw",
	"der RobocoRageMaster 5000",
	"das Raketen-Gang-Mitglied",
	"die Raketen-MP",
	"der Rotwurm",
	"der Satyr",
	"das Seal-Kommando",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	//	"der Seal-Taucher",      // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"der Seal-Captain",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"das Seal-Mädchen",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"der Hai",
	"der Schlachter",
	"das Skelett",
	"der Arbeiter",
	"der fette Arbeiter",
	"der Roboco-Drecksarbeiter",
	"die kleine Spinne",
	"die Spinne",
	"Stavros",     // TX NOTE:must match script.
	"der Chirurg",
	"der Dieb",
	"der Donner-Skeet",
	//	"die Roboco-TrackAttack",
	"das Uzi-Gang-Mitglied",
	"der weiße Gefangene",
	"der Magier",     // TX NOTE:must match script.
	"Wyndrax",     // TX NOTE:must match script.
	"die Möwe",
	"die Feuerfliege",
	"die Wispe",
	"Mikkiko"
};

// monster names ACCUSATIVE CASE. NOT USED IN ENGLISH. translate what you can. "localize" spellings of anything else.
static char *tongue_monsters_acc[] =
{
	"die RoboCo-Kampfeber",
	"den Schwarzen Gefangenen",
	"den Buboiden",     // TX NOTE:must match script.
	"den RoboCo-Cambot",
	"den Zentauren",
	"den Cerberus",     // TX NOTE:deleted. don't translate.
	"den Fliegenden Schützen",
	"die Caratyd-Säule",
	"das Crox",
	//	"den Zyklopen",
	"die RoboCo-Todessphäre",
	"die Fledermaus des Todes",
	"den Drachen",     // TX NOTE:must match script.
	"den Zwerg",
	"die Femgang",
	"den Fährmann",
	"den Fletscher",
	"den RoboCo-Froschinator",
	"König Garroth",
	"den Goldfisch",
	"den Griffon",
	"die Harpyie",
	"den Mitgefangenen",
	"Kage",
	"König Minos",     // TX NOTE:deleted. don't translate.
	"den ersten Ritter",
	"den zweiten Ritter",
	"den Laboraffen",
	"den Laborarbeiter",
	"das RoboCo-Lasergat",
	"den Lycanthir",
	"die Medusa",     // TX NOTE:must match script.
	"den Minotaurus",
	"Mishimas Wachen",     // TX NOTE:must match script.
	"Nharre"
	"die Pest-Ratte",
	"die Ratte",
	"den Priester",
	"den Gefangenen",
	"den RoboCo-Protopod",
	"die Psyclaw",
	"den RobocoRageMaster 5000",
	"das Raketen-Gang-Mitglied",
	"die Raketen-MP",
	"den Rotwurm",
	"den Satyr",
	"das Seal-Kommando",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	//	"den Seal-Taucher",      // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"den Seal-Captain",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"das Seal-Mädchen",     // TX NOTE:U.S. Navy SEAL, not the animal (no pun intended).
	"den Hai",
	"den Schlachter",
	"das Skelett",
	"den Arbeiter",
	"den fetten Arbeiter",
	"den Roboco-Drecksarbeiter",
	"die kleine Spinne",
	"die Spinne",
	"Stavros",     // TX NOTE:must match script.
	"den Chirurg",
	"den Dieb",
	"den Donner-Skeet",
	//	"die Roboco-TrackAttack",
	"das Uzi-Gang-Mitglied",
	"den weißen Gefangenen",
	"den Magier",     // TX NOTE:must match script.
	"Wyndrax",     // TX NOTE:must match script.
	"die Möwe",
	"die Feuerfliege",
	"die Wispe",
	"Mikkiko"
};


static char *tongue_world[] =
{
	"Es gibt noch mehr zu tun",                 // triggers to go until sequence complete TX NOTE:## +.
	"Sequenz vollständig",             // trigger sequence completed TX NOTE:sentence.
	
	"Sie brauchen",               // need a specific key TX NOTE:followed by the key needed.
	"Sie brauchen den korrekten Schlüssel",        // wrong key in possession TX NOTE:sentence.
	"Kann nicht in diesem Weg geöffnet werden",
	
	"Die Wirkung des Giftes läßt nach",  // poison expiring TX NOTE:sentence.
	"Ihre Oxylungs sind fast verbraucht",      // oxylung expiring TX NOTE:sentence.
	"Ihre Envirosuit wird schwächer",        // envirosuit expiring TX NOTE:sentence.
	"Ihr Megaschild wird schwächer",         // megashield expiring TX NOTE:sentence.
	"Die Gespenstersphäre wird schwächer",          // wraith orb expiring TX NOTE:sentence.
	"Die Extra-Energie ist fast aufgebraucht",				 // power boost expiring TX NOTE:sentence.
	"Die Extra-Angriffsstärke läßt nach",        // attack boost expiring TX NOTE:sentence.
	"Der Geschwindigkeit-Schub wird schwächer",         // speed boost expiring TX NOTE:sentence.
	"Der Akrobatik-Schub ist verbraucht",       // jump boost expiring TX NOTE:sentence.
	"Der Vitalitätsschub ist verbraucht",       // health boost expiring TX NOTE:sentence.
	
	"ist dem Spiel beigetreten",              // player joined the game TX NOTE:NAME +.
	"hat das Level verlassen",              // player exited the level TX NOTE:NAME +.
	
	"Hiro Miyamoto",               // Hiro Miyamoto's full name TX NOTE:same as in script.
	"Superfly Johnson",              // Superfly's full name TX NOTE:same as in script.
	"Mikiko Ebihara",               // Mikiko's full name TX NOTE:same as in script.
	
	// SCG[10/18/99]: these next 5 had enums, but no strings. 
	"",											// SCG[10/18/99]: T_PLAYER_SELF_SPAM_KICK
	"",											// SCG[10/18/99]: T_PLAYER_SPAM_KICK
	"",											// SCG[10/18/99]: T_PLAYER_TIME_LIMIT_HIT
	"",											// SCG[10/18/99]: T_PLAYER_FRAG_LIMIT_HIT
	"",											// SCG[10/18/99]: T_TELL_CMD_UNKNOWN_PLAYER
	
	"Kann die Karte nicht verlassen: Es felh(t/en): \n",					// SCG[10/18/99]: no exiting level without sidekicks 
	"Kann nicht verlassen: Es felh(t/en): \n",					// SCG[10/18/99]: no exiting level without sidekicks 
	"Vergiftet!\n",									// NSS[11/29/99]: Needed a generic and yet global poisoned message

	"Du hast ein Geheimnis entdeckt!\n",						// cek[1-3-00]
	"Der Manaschädel wird schwächer.\n"
};

static char *tongue_deathmsg_daikatana[] = 
{
	"%s wird aufgeschlitzt von %s\n"				//Daikatana
};


static char *tongue_deathmsg_self[] = 
{
	" hat finale Kopfschmerzen\n",             // death, fell TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"hat zuviel Wasser inhaliert\n",               // death, drowned TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"ist schön knusprig.\n",                // death, lava TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"ist weggeschleimt worden\n",                 // death, slimed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"hat sich den Magen verdorben\n",                // death, poison TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"war des Lebens überdrüssig\n",           // death, killed self TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"ist verteleportiert\n",              // death, teleport TX NOTE:character +. TX NOTE:use your own appropriate slang.
	"wurde eingestampft\n",              // death, crushed TX NOTE:character +. TX NOTE:use your own appropriate slang.
	
	"hat sich an C4 verschluckt\n"            // death, C4 explosives TX NOTE:character +. TX NOTE:use your own appropriate slang.
};


static char *tongue_deathmsg_weapon1[] =		// %s = name of attacker/victim
{
	"%s wurde beschämt von %s!\n",					//disruptor glove- NOTE:use your own appropriate slang.
	"%s verpaßte %s den finalen Husten!\n",			//ultimate gashands
	"%s wurde von %s ionisiert!\n",					//Ion blaster
	"%s hat %s gleichmäßig verteilt!\n",					//C4
	"%s hat %s seinen Doppellauf verpaßt!\n",					//Shotcycler
	"%s geht dank %s ab wie eine Rakete!\n",						//Sidewinder
	"%s hat %s geschüttelt, nicht gerührt!\n"							//Shockwave
};

static char *tongue_deathmsg_weapon2[] =		// %s = name of attacker/victim
{
	"%s hat mit %s finales Frisbee gespielt!\n",		//Discus- NOTE:use your own appropriate slang.
	"%s hat %s gebissen!\n",						//venomous
	"%s hat %s schön knusprig gebräunt!\n",		//Sunflare
	"%s hat %s geplättet!\n",						//Hammer
	"%s hat aus %s Dreifach-Schaschlik gemacht!\n",					//trident
	"%s kniete nieder vor Zeus!\n"					//Zeus
};

static char *tongue_deathmsg_weapon3[] =		// %s = name of attacker/victim
{
	"%s wurde von %s tranchiert!\n",			//Silverclaw- NOTE:use your own appropriate slang.
	"%s hat %s den Ast abgeknickt!\n",						//Bolter
	"%s hat %s etwas Holz überreicht!\n",					//Ballista
	"%s wurde von %ss Meteor verkokelt!\n",		//Stave
	"%s steht dank %s tierisch unter Strom!\n",	//Wisp
	"%s hat %ss Dämon kennengelernt!\n"				//Nightmare
};

static char *tongue_deathmsg_weapon4[] =		// %s = name of attacker/victim
{
	"%s hat %s eine Metallfüllung verpaßt!\n",					//Glock- NOTE:use your own appropriate slang.
	"%s bombardiert %s!\n",						//Slugger
	"%s wurde von %s geschnetzelt!\n",				//Ripgun
	"%s hat mit %s eine ziemliche Sauerei veranstaltet!\n",				//Cordite
	"%s wurde von %s vereist!\n",						//Kineticore
	"%s hat %s gelocht und abgeheftet!\n",					//Novebeam
	"%s bekam Masern von %s; Meta-Masern, hihi!\n"						//Metamaser
};

static char *tongue_scoreboard[] =
{
	"Name",
	"Abschüsse",
	"Ping",
	"Zeit",
	"Schüsse",
	"Treffer",
	"Gestorben",
	"Punkte"										// cek[1-22-00]

};


static char *tongue_difficulty[] = // difficulty settings for 'new game' menu
{
	"Schwierigkeitsgrad wählen:",      // difficulty header TX NOTE:using karate skill levels.
	"Ronin",          // newbie
	"Samurai",          // average
	"Shogun"          // expert (or so they think!)
};


static char *tongue_skincolors[] =
{
	"Blau",             // blue
	"Grün",            // green
	"Rot",             // red
	"Gold"             // gold
};


static char *tongue_statbar[] =
{
	"STÄRKE",             // skill 'power' label
	"ANGRIFF",            // skill 'attack' label
	"TEMPO",              // skill 'speed' label
	"AKROBATIK",          // skill 'acrobatic' label
	"VITALITÄT",          // skill 'vitality' label
		
	"RÜSTUNG",              // armor label
	"GESUNDHEIT",					// health label
	"MUNITION",						// ammo label
	"ABSCHÜSSE",					    // kills label
	"ERFAHRUNGSSTUFE", 		// experience points label
		
	"STUFE HOCH!",          // experience level up
	"<+/->   Wählen",     // select skill
	"<Enter> Punkt dazu",  // add point to skill
		
	"Lädt..."                    // loading new level
};


static char *tongue_weapon_misc[] =
{
    "C4-Ladungen werden instabil!"    // C4 modules are de-stabilizing
};


static char *tongue_sidekick[] =
{
	"Heb's auf",	                    // 'get', 'pick up' item command
	"Komm' her",                      // 'come here', 'come to me'
	"Bleib",                          // 'stay' command, 'don't move'
	"Greif' an",                      // 'attack' command
	"Greif' nicht an",                // 'back off', 'move away'
		
	"Gib mir doch mal "                      //  asking to pick up an item: "Can I have the ...?" or "i need the..."
};

static char *tongue_ctf[] =
{
	"Rote Flagge",
	"Blaue Flagge",
	"Unbekanntes Team",

	"%sFlagge",											// when translating, treat '%s Flag' as 'red Flag' and place the %s before or after flag
														// depending on the syntax of the language.  ie if the expression should be 'Flag red' use 'Flag %s'
	"Rot",
	"Blau",
	"Chrom",
	"Metall",
	"Grün",
	"Orange",
	"Lila",
	"Gelb",

	"Spielstand jetzt %s:%d, %s:%d\n",
	"Sie erobern die %s!\n",
	"%s aus Ihrem Team erobert die %s!\n",
	"%s vom Team %s erobert Ihre Flagge!\n",		// when translating, treat '%s team' as 'red team' and place the %s before or after team
															// depending on the syntax of the language.  ie if the expression should be 'team red' use 'team %s'
	"Sie schnappen sich die %s\n",
	"%s aus Ihrem Team schnappt sich die %s\n",
	"%s hat Ihre Fahne gestohlen\n",
	"Die %s wurde zurückgebracht\n",

	"%d ist eine ungültige Teamnummer Verwenden Sie 1 oder 2\n",
	"Sie gehören zu Team %d (%s)\n",
	"Sie gehören schon zu Team %d\n",
	"%s schließt sich Team %s an\n",									// translate same as with the %s team above

	"Team %s hat gewonnen!\nSchlußpunktzahl: %s:%d, %s:%d\n",		// translate same as with the %s team above
	"Das Spiel endet unentschieden!\n",
	"Die Zeit ist abgelaufen!\n",
	"Das Limit wurde erreicht!\n"
};

static char *tongue_deathtag[] =
{
	"Bombe",														// as in backpack
	"%s Bombe",													// as in red pack
	"Punktlimit erreicht!\n",
	"Ihre Zeit wird knapp!\n",
	"Ihre Zeit ist abgelaufen!\n",

	"%s vom Team %s schnappt sich die %s!\n",				// translate using the '%s team' guidelines above

	"Ihr Team erhält einen Punkt!\n",
	"Ihr Team erhält %d Punkte!\n",								// you get 2 points
	"Team %s erhält einen Punkt!\n",								// translate using the '%s team' guidelines above
	"Team %s erhält %d Punkte!\n",							// The red team gets 2 points

	"Punkt!\n",
	"%s aus Ihrem Team hat Punkte gemacht!\n",
	"%s vom Team %s hat Punkte gemacht!\n"
};

// graphics related

static char *tongue_button_skins[] = // texture maps used on front-end menu buttons
{
    "localized/german/ib_newgame.bmp",					// new game
	"localized/german/ib_multi.bmp",						// multiplayer
	"localized/german/ib_loadgame.bmp",         // load game
	"localized/german/ib_savegame.bmp",         // save game
	"localized/german/ib_sound.bmp",            // sound
	"localized/german/ib_video.bmp",            // video
	"localized/german/ib_mouse.bmp",            // mouse
	"localized/german/ib_keyboard.bmp",         // keyboard
	"localized/german/ib_joystick.bmp",         // joystick
	"localized/german/ib_options.bmp",          // options
	"localized/german/ib_advanced.bmp",         // advanced options
	"localized/german/ib_demos.bmp",            // demos
	"localized/german/ib_resume.bmp",           // resume game
	"localized/german/ib_quit.bmp",             // quit
	"localized/german/ib_blank.bmp"             // <blank>
};
